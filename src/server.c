#include <stdio.h>
#include <string.h>
#include <uv.h>

#include "applog.h"
#include "http.h"
#include "server.h"

#define DEFAULT_BACKLOG 128

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
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
    server *srv = (server *)client->data;

    if (!srv->req)
    {
        _err("Erro: http_request não inicializado");
        if (buf->base)
            free(buf->base);
        return;
    }

    if (nread > 0)
    {
        if (!srv->req->header_parsed)
        {
            char *header_end = strstr(buf->base, "\r\n\r\n");
            if (header_end)
            {
                char *content_length_str = strstr(buf->base, "Content-Length: ");
                if (content_length_str)
                {
                    content_length_str += strlen("Content-Length: ");
                    srv->req->content_length = strtol(content_length_str, NULL, 10);
                }

                srv->req->headers = malloc(nread + 1);
                if (!srv->req->headers)
                {
                    _err("Falha ao alocar memória para headers");
                    http_request_free(srv->req);
                    free(buf->base);
                    return;
                }

                memcpy(srv->req->headers, buf->base, nread - strlen(header_end) + 1);
                srv->req->headers[nread] = '\0';
                srv->req->header_parsed = 1;
            }
        }

        if (srv->req->header_parsed)
        {
            srv->req->total_read += nread;

            if (srv && srv->cb)
            {
                srv->cb(srv->req, client);
            }

            free(srv->req->headers);
            srv->req->headers = NULL;

            uv_read_stop(client);
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

        http_request_free(srv->req);
        uv_close((uv_handle_t *)client, NULL);
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

    server *srv = (server *)stream->data;

    uv_tcp_t *client = malloc(sizeof(uv_tcp_t));
    if (!client)
    {
        _err("Falha ao alocar memória para cliente");
        return;
    }

    uv_tcp_init(srv->loop, client);

    if (uv_accept(stream, (uv_stream_t *)client) == 0)
    {
        srv->req = http_request_create((uv_stream_t *)client);

        if (!srv->req)
        {
            _err("Falha ao alocar memória para request");
            free(client);
            return;
        }

        client->data = srv;
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
    server *server = malloc(sizeof(server));

    if (!server)
    {
        _err("Erro ao alocar servidor");
        return NULL;
    }

    server->loop = uv_default_loop();
    server->cb = cb;
    server->req = NULL;
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
