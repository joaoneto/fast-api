#include <stdio.h>
#include <string.h>
#include <uv.h>

#include "applog.h"
#include "http.h"
#include "server.h"

#define DEFAULT_BACKLOG 128

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = malloc(suggested_size);
    if (!buf->base)
    {
        _err("Falha ao alocar memória para buffer");
        buf->len = 0;
    }
    else
    {
        buf->len = suggested_size;
    }
}

static void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    server_conn *conn = (server_conn *)client->data;

    if (!conn->req)
    {
        _err("Erro: http_request não inicializado");
        if (buf->base)
        {
            free(buf->base);
        }
        return;
    }

    if (nread > 0)
    {
        if (!conn->req->header_parsed)
        {
            char *header_end = strstr(buf->base, "\r\n\r\n");
            if (header_end)
            {
                size_t header_len = (header_end - buf->base) + 4;
                char *headers_str = strndup(buf->base, header_len);

                char *first_line = strtok(headers_str, "\r\n");
                char *first_line_copy = strdup(first_line);
                http_parse_request_line(conn->req, first_line_copy);
                free(first_line_copy);
                http_parse_headers(conn->req, headers_str + strlen(first_line) + 2);

                free(headers_str);

                char *content_length_pos = strcasestr(buf->base, "content-length: ");
                if (content_length_pos)
                {
                    content_length_pos += 16;
                    conn->req->content_length = strtol(content_length_pos, NULL, 10);
                }

                conn->req->header_parsed = 1;
            }
        }

        if (conn->req->header_parsed)
        {
            conn->req->total_read += nread;

            if (conn && conn->srv->cb)
            {
                conn->srv->cb(conn->req, client);
            }

            // @todo Fazer req->end
            // http_request_free(conn->req);
            // uv_read_stop(client);
            // uv_close((uv_handle_t *)client, NULL);
            // return;
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
            _err("Erro: %s", uv_strerror(nread));
        }

        http_request_free(conn->req);
        uv_close((uv_handle_t *)client, NULL);
    }

    if (buf->base)
    {
        free(buf->base);
    }
}

static void on_new_connection(uv_stream_t *stream, int status)
{
    if (status < 0)
    {
        _err("Erro ao aceitar nova conexão: %s", uv_strerror(status));
        return;
    }

    server *srv = (server *)stream->data;

    uv_tcp_t *client = malloc(sizeof(uv_tcp_t));
    if (!client)
    {
        _err("Falha ao alocar memória para cliente");
        return;
    }

    server_conn *conn = malloc(sizeof(server_conn));
    if (!conn)
    {
        _err("Falha ao alocar memória para connexão");
        return;
    }
    conn->req = NULL;
    conn->srv = NULL;

    uv_tcp_init(srv->loop, client);

    if (uv_accept(stream, (uv_stream_t *)client) == 0)
    {
        conn->req = http_request_create((uv_stream_t *)client);
        conn->srv = srv;

        if (!conn->req)
        {
            _err("Falha ao alocar memória para request");
            free(client);
            return;
        }

        client->data = conn;
        uv_read_start((uv_stream_t *)client, alloc_buffer, on_read);
    }
    else
    {
        _err("Erro ao aceitar conexão");
        free(client);
    }
}

server *server_create(server_cb cb)
{
    server *server = malloc(sizeof(*server));

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

int server_listen(server *srv, const char *ip, int port)
{
    struct sockaddr_in addr;

    uv_ip4_addr(ip, port, &addr);
    uv_tcp_bind(&srv->socket, (const struct sockaddr *)&addr, 0);

    uv_listen((uv_stream_t *)&srv->socket, DEFAULT_BACKLOG, on_new_connection);

    return uv_run(srv->loop, UV_RUN_DEFAULT);
}
