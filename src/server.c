#include <stdio.h>
#include <string.h>
#include <uv.h>

#include "applog.h"
#include "http/http.h"
#include "server.h"

#define DEFAULT_BACKLOG 128
#define DEFAULT_TIMEOUT_MS 3000
#define DEFAULT_MAX_READ_BYTES 2 * 1024 * 1024
#define DEFAULT_BUFFER_SIZE 4096

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char *)malloc(DEFAULT_BUFFER_SIZE);
    buf->len = DEFAULT_BUFFER_SIZE;
}

static void on_close(uv_handle_t *client)
{
    server_conn_free((server_conn_t *)client->data);
    free(client);
}

static void on_walk(uv_handle_t *handle, void *arg)
{
    if (!uv_is_closing(handle))
    {
        uv_close(handle, on_close);
    }
}

static void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    server_conn_t *conn = (server_conn_t *)client->data;
    conn->req->bytes_received += nread;

    if (nread > 0)
    {
        if (!conn->req->header_parsed)
        {

            http_request_parse_headers(conn->req, buf);
        }

        if (conn && conn->server->cb)
        {
            conn->server->cb(conn->req, conn->res, client);
        }
    }
    else if (nread < 0)
    {
        if (nread == UV_EOF)
        {
            _info("Conexão fechada pelo cliente");
        }
        else
        {
            _err("Erro de leitura: %s", uv_strerror(nread));
        }

        uv_close((uv_handle_t *)client, on_close);
    }

    free(buf->base);
}

static void on_new_connection(uv_stream_t *stream, int status)
{
    if (status < 0)
    {
        _err("Erro ao aceitar nova conexão: %s", uv_strerror(status));
        return;
    }

    server_t *server = (server_t *)stream->data;

    uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    if (!client)
    {
        fprintf(stderr, "Falha ao alocar memória para cliente");
        return;
    }

    server_conn_t *conn = (server_conn_t *)malloc(sizeof(server_conn_t));
    if (!conn)
    {
        fprintf(stderr, "Falha ao alocar memória para conexão");
        return;
    }

    conn->server = server;

    int init_err = uv_tcp_init(server->loop, client);
    if (init_err < 0)
    {
        _err("Erro ao inicializar cliente TCP: %s", uv_strerror(init_err));
        server_conn_free(conn);
        free(client);
        return;
    }

    if (!uv_is_active((uv_handle_t *)&server->handle))
    {
        _err("Tentativa de aceitar conexão em um socket inativo");
        return;
    }

    if (uv_accept((uv_stream_t *)&server->handle, (uv_stream_t *)client) == 0)
    {
        conn->req = http_request_create();
        conn->res = http_response_create();

        client->data = conn;

        int r = uv_read_start((uv_stream_t *)client, alloc_buffer, on_read);
        if (r != 0)
        {
            _err("Erro ao aceitar cliente: %s", r);
            server_conn_free(conn);
            uv_close((uv_handle_t *)client, on_close);
            return;
        }
    }
    else
    {
        _err("Erro ao aceitar cliente, fechando conexão");
        uv_close((uv_handle_t *)client, on_close);
    }
}

server_t *server_create(uv_loop_t *loop, server_cb cb)
{
    server_t *server = malloc(sizeof(server_t));

    server->loop = loop;
    server->cb = cb;
    server->handle.data = server;

    uv_tcp_init(server->loop, &server->handle);

    return server;
}

int server_listen(server_t *server, const char *ip, int port)
{
    struct sockaddr_in addr;

    uv_ip4_addr(ip, port, &addr);
    uv_tcp_bind(&server->handle, (const struct sockaddr *)&addr, 0);

    uv_listen((uv_stream_t *)&server->handle, DEFAULT_BACKLOG, on_new_connection);

    return uv_run(server->loop, UV_RUN_DEFAULT);
}

void server_shutdown(server_t *server)
{
    if (!server || !server->loop)
    {
        fprintf(stderr, "Erro: Servidor ou loop inválido.\n");
        return;
    }

    printf("Encerrando o servidor...\n");

    // Fecha todas as conexões e handles registrados no loop
    uv_stop(server->loop);
    uv_walk(server->loop, on_walk, NULL);

    // Aguarda o loop processar os fechamentos
    uv_run(server->loop, UV_RUN_DEFAULT);

    // Libera o loop (se ele não foi alocado estaticamente)
    uv_loop_close(server->loop);
    free(server->loop);

    printf("Servidor encerrado com sucesso.\n");
}

void server_conn_free(server_conn_t *conn)
{
    http_response_free(conn->res);
    http_request_free(conn->req);
    free(conn);
    conn = NULL;
}
