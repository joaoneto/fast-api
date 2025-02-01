#include <stdlib.h>
#include <uv.h>

#include "applog.h"
#include "http.h"

http_request *http_request_create(uv_stream_t *client)
{
    http_request *req = (http_request *)malloc(sizeof(http_request));
    if (!req)
    {
        _err("Falha ao alocar memória para request");
        return NULL;
    }

    req->headers = NULL;
    req->body = NULL;
    req->total_read = 0;
    req->content_length = 0;
    req->header_parsed = 0;
    req->json = http_send_json;

    return req;
}

char *http_json_response(const char *json_body)
{
    if (!json_body)
    {
        json_body = "{}";
    }

    size_t content_length = strlen(json_body);

    // Calcula o tamanho necessário para a resposta HTTP
    int needed_size = snprintf(NULL, 0, HTTP_RESPONSE_TEMPLATE, content_length, json_body);
    if (needed_size <= 0)
    {
        _err("Erro ao calcular tamanho da resposta HTTP!");
        return NULL;
    }

    size_t response_size = (size_t)needed_size + 1; // +1 para o '\0'
    char *response = malloc(response_size);
    if (!response)
    {
        _err("Falha ao alocar memória para resposta HTTP!");
        return NULL;
    }

    // Preenche o buffer com a resposta formatada
    snprintf(response, response_size, HTTP_RESPONSE_TEMPLATE, content_length, json_body);

    return response;
}

static void on_write(uv_write_t *req, int status)
{
    if (status < 0)
    {
        _err("Erro ao escrever: %s", uv_strerror(status));
        uv_close((uv_handle_t *)req->handle, NULL);
    }

    free(req);
}

int http_send(const char *res, uv_stream_t *client)
{
    uv_write_t *r = malloc(sizeof(uv_write_t));
    uv_buf_t buf = uv_buf_init((char *)res, strlen(res));
    uv_write(r, client, &buf, 1, on_write);

    return 0;
}

int http_send_json(const char *res, uv_stream_t *client)
{
    const char *r = http_json_response(res);

    return http_send(r, client);
}

void http_request_free(http_request *req)
{
    if (!req)
    {
        return;
    }

    if (req->headers)
    {
        free(req->headers);
        req->headers = NULL;
    }

    if (req->body)
    {
        free(req->body);
        req->body = NULL;
    }

    free(req);
}
