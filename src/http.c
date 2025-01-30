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

    memset(req, 0, sizeof(http_request));

    req->client = client;
    req->headers = NULL;
    req->body = NULL;
    req->total_read = 0;
    req->content_length = 0;
    req->request_data = NULL;
    req->header_parsed = 0;

    return req;
}

void http_parse_headers(http_request *req, const char *data)
{
    _debug("http_parse_headers");
}

void http_request_append_body(http_request *req, const char *data, size_t len)
{
    _debug("http_request_append_body");
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

int http_respond(const char *response, uv_stream_t *client, const uv_buf_t *buf, uv_write_cb cb)
{
    uv_write_t *w_req = malloc(sizeof(uv_write_t));

    // Falha ao alocar memória para resposta
    if (!w_req)
    {
        return -100;
    }

    uv_buf_t res_buf = uv_buf_init((char *)response, strlen(response));
    uv_write(w_req, client, &res_buf, 1, cb);

    return 0;
}

void http_request_free(http_request *req)
{
    if (!req)
        return;

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
