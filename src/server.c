#include <stdio.h>
#include <string.h>
#include <uv.h>

#include "applog.h"
#include "buffer_node.h"
#include "http/http.h"
#include "server.h"

#define DEFAULT_BACKLOG 128
#define DEFAULT_TIMEOUT_MS 3000
#define DEFAULT_MAX_READ_BYTES 2 * 1024 * 1024

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    server_conn_t *conn = (server_conn_t *)handle->data;

    if (!conn)
    {
        _err("Falha ao acessar a estrutura da conexão");
        buf->base = NULL;
        buf->len = 0;
        return;
    }

    // Pega um buffer do pool ou aloca novo se necessário
    conn->buffer = buffer_pool_acquire();
    if (!conn->buffer)
    {
        conn->buffer = aligned_alloc(16, suggested_size);
        if (!conn->buffer)
        {
            _err("Falha ao alocar buffer");
            buf->base = NULL;
            buf->len = 0;
            return;
        }
    }

    buf->base = conn->buffer;
    buf->len = BUFFER_SIZE;
}

static void on_close(uv_handle_t *client)
{
    server_conn_free((server_conn_t *)client->data);
    free(client);
}

static void on_timeout(uv_timer_t *timer)
{
    uv_stream_t *client = (uv_stream_t *)timer->data;
    server_conn_t *conn = (server_conn_t *)client->data;
    http_response_t *res = (http_response_t *)conn->res;

    _err("Tempo limite da requisição atingido, fechando conexão");

    uv_read_stop((uv_stream_t *)client);
    uv_timer_stop((uv_timer_t *)conn->timeout);

    res->status = HTTP_REQUEST_TIMEOUT;
    res->json("{ \"message\": \"Tempo limite da requisição atingido, fechando conexão\" }", client);
}

static void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    server_conn_t *conn = (server_conn_t *)client->data;
    conn->req->total_read += nread;

    if (!conn->req || !conn->res)
    {
        _err("Conexão não inicializado com Req ou Res");
        free(buf->base);
        return;
    }

    if (nread > 0)
    {
        uv_mutex_lock(&conn->lock);

        if (!conn->req->header_parsed)
        {
            char *header_end = strstr(buf->base, "\r\n\r\n");
            if (header_end)
            {
                // size_t header_len = (header_end - buf->base) + 4; // \r\n\r\n

                // Trabalhando diretamente no buffer para separar a primeira linha
                char *headers_str = buf->base;

                char *content_length_pos = strcasestr(headers_str, "content-length: ");
                if (content_length_pos)
                {
                    content_length_pos += 16;
                    conn->req->content_length = strtol(content_length_pos, NULL, 10);
                }

                // Encontrando o fim da primeira linha
                char *first_line_end = strstr(headers_str, "\r\n");
                if (first_line_end)
                {
                    *first_line_end = '\0'; // Finaliza a primeira linha

                    // Parse a primeira linha do HTTP diretamente
                    http_request_parse_line(conn->req, headers_str);

                    // Parse os headers restantes, começando após a primeira linha
                    http_request_parse_headers(conn->req, first_line_end + 2);
                }

                conn->req->header_parsed = 1;
            }
        }

        _debug("> Quantidade lida: %zu", conn->req->total_read);
        _debug("> Tamanho do payload: %zu", conn->req->content_length);

        // Não funciona, tem vazameto de memória
        if (conn->req->total_read > DEFAULT_MAX_READ_BYTES || conn->req->content_length > DEFAULT_MAX_READ_BYTES)
        {
            _err("Requisição excedeu o limite de 2MB, encerrando conexão");

            conn->res->status = HTTP_PAYLOAD_TOO_LARGE;
            conn->res->json("{ \"message\": \"Requisição excedeu o limite de 2MB, encerrando conexão\" }", client);

            uv_mutex_unlock(&conn->lock);
            return;
        }

        if (conn && conn->server->cb)
        {
            conn->server->cb(conn->req, conn->res, client);
        }

        uv_mutex_unlock(&conn->lock);
    }
    else if (nread < 0)
    {
        if (nread == UV_EOF)
        {
            _info("Conexão fechada pelo cliente");
        }
        else
        {
            _err("Erro: %s", uv_strerror(nread));
        }

        uv_close((uv_handle_t *)client, on_close);
    }
}

static void on_new_connection(uv_stream_t *stream, int status)
{
    if (status < 0)
    {
        _err("Erro ao aceitar nova conexão: %s", uv_strerror(status));
        return;
    }

    server_t *server = (server_t *)stream->data;

    uv_tcp_t *client = malloc(sizeof(uv_tcp_t));
    if (!client)
    {
        _err("Falha ao alocar memória para cliente");
        return;
    }

    server_conn_t *conn = malloc(sizeof(server_conn_t));
    if (!conn)
    {
        _err("Falha ao alocar memória para conexão");
        return;
    }

    uv_tcp_init(server->loop, client);

    if (uv_accept(stream, (uv_stream_t *)client) == 0)
    {

        conn->req = http_request_create();
        conn->res = http_response_create();
        uv_mutex_init(&conn->lock);
        conn->server = server;
        conn->buffer = NULL;

        if (!conn->req || !conn->res)
        {
            free(client);
            return;
        }

        client->data = conn;

        // Inicia o timer para controlar o timeout da requisição
        conn->timeout = malloc(sizeof(uv_timer_t));
        if (!conn->timeout)
        {
            _err("Falha ao alocar timer");
            return;
        }
        conn->timeout->data = client;
        uv_timer_init(conn->server->loop, conn->timeout);
        uv_timer_start(conn->timeout, on_timeout, DEFAULT_TIMEOUT_MS, 0);

        // Inicia leitura da requisição
        uv_read_start((uv_stream_t *)client, alloc_buffer, on_read);
    }
    else
    {
        _err("Erro ao aceitar conexão");
        free(client);
    }
}

server_t *server_create(server_cb cb)
{
    server_t *server = malloc(sizeof(*server));

    if (!server)
    {
        _err("Erro ao alocar servidor");
        return NULL;
    }

    server->loop = uv_default_loop();
    server->cb = cb;
    server->socket.data = server;

    uv_tcp_init(server->loop, &server->socket);

    return server;
}

int server_listen(server_t *server, const char *ip, int port)
{
    struct sockaddr_in addr;

    uv_ip4_addr(ip, port, &addr);
    uv_tcp_bind(&server->socket, (const struct sockaddr *)&addr, 0);

    uv_listen((uv_stream_t *)&server->socket, DEFAULT_BACKLOG, on_new_connection);

    return uv_run(server->loop, UV_RUN_DEFAULT);
}

void server_conn_free(server_conn_t *conn)
{
    if (!conn)
    {
        return;
    }

    uv_mutex_lock(&conn->lock);

    http_request_free(conn->req);
    http_response_free(conn->res);

    if (conn->timeout)
    {
        free(conn->timeout);
        conn->timeout = NULL;
    }

    if (conn->buffer)
    {
        buffer_pool_release(conn->buffer);
        conn->buffer = NULL;
    }

    uv_mutex_unlock(&conn->lock);
    uv_mutex_destroy(&conn->lock);

    free(conn);
    conn = NULL;
}
