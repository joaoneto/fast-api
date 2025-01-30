#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <uv.h>

#include "applog.h"
#include "http.h"

#define DEFAULT_PORT 3000
#define DEFAULT_BACKLOG 128

uv_loop_t *loop;

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

void on_write(uv_write_t *req, int status)
{
    if (status < 0)
    {
        _err("Erro ao escrever: %s", uv_strerror(status));
        uv_close((uv_handle_t *)req->handle, NULL);
    }

    free(req);
}

void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    http_request *req = (http_request *)client->data;

    if (!req)
    {
        _err("Erro: http_request não inicializado");
        if (buf->base)
            free(buf->base);
        return;
    }

    if (nread > 0)
    {
        if (!req->header_parsed)
        {
            char *header_end = strstr(buf->base, "\r\n\r\n");
            if (header_end)
            {
                char *content_length_str = strstr(buf->base, "Content-Length: ");
                if (content_length_str)
                {
                    content_length_str += strlen("Content-Length: ");
                    req->content_length = strtol(content_length_str, NULL, 10);
                }

                req->headers = malloc(nread + 1);
                if (!req->headers)
                {
                    _err("Falha ao alocar memória para headers");
                    http_request_free(req);
                    free(buf->base);
                    return;
                }

                memcpy(req->headers, buf->base, nread - strlen(header_end) + 1);
                req->headers[nread] = '\0';
                req->header_parsed = 1;
            }
        }

        if (req->header_parsed)
        {
            req->total_read += nread;

            if (req->total_read >= req->content_length)
            {
                _info("Corpo da requisição completamente lido");

                const char *res = http_json_response("{\"message\": \"Hello, world!\"}");

                int r = http_respond(
                    res,
                    client,
                    buf,
                    on_write);

                if (r < 0)
                {
                    _err("Erro: Código %d", r);
                }

                free(req->headers);
                req->headers = NULL;

                uv_read_stop(client);
            }
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

        http_request_free(req);
        uv_close((uv_handle_t *)client, NULL);
    }

    free(buf->base);
}

void on_new_connection(uv_stream_t *server, int status)
{
    if (status < 0)
    {
        _err("Erro ao aceitar nova conexão: %s", uv_strerror(status));
        return;
    }

    uv_tcp_t *client = malloc(sizeof(uv_tcp_t));
    if (!client)
    {
        _err("Falha ao alocar memória para cliente");
        return;
    }

    uv_tcp_init(loop, client);

    if (uv_accept(server, (uv_stream_t *)client) == 0)
    {
        http_request *req = http_request_create((uv_stream_t *)client);
        if (!req)
        {
            _err("Falha ao alocar memória para request");
            free(client);
            return;
        }

        client->data = req;
        uv_read_start((uv_stream_t *)client, alloc_buffer, on_read);
    }
    else
    {
        _err("Erro ao aceitar conexão");
        free(client);
    }
}

int main()
{
    loop = uv_default_loop();

    uv_tcp_t server;
    uv_tcp_init(loop, &server);
    struct sockaddr_in addr;

    uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);

    uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);
    int r = uv_listen((uv_stream_t *)&server, DEFAULT_BACKLOG, on_new_connection);
    if (r)
    {
        _err("Erro: %s", uv_strerror(r));
        return 1;
    }

    return uv_run(loop, UV_RUN_DEFAULT);
}
