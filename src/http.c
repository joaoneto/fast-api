#include <stdlib.h>
#include <uv.h>

#include "applog.h"
#include "http.h"

char *http_get_header(http_request *req, const char *key)
{
    for (size_t i = 0; i < req->header_count; i++)
    {
        if (strcasecmp(req->headers[i].key, key) == 0)
        {
            return req->headers[i].value;
        }
    }
    return NULL;
}

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

void http_parse_request_line(http_request *req, char *line)
{
    char *saveptr;
    req->method = strdup(strtok_r(line, " ", &saveptr));
    req->path = strdup(strtok_r(NULL, " ", &saveptr));
    req->version = strdup(strtok_r(NULL, "\r\n", &saveptr));
}

void http_parse_headers(http_request *req, char *headers)
{
    char *line = strtok(headers, "\r\n");
    req->header_count = 0;

    while (line)
    {
        char *sep = strchr(line, ':');
        if (sep)
        {
            *sep = '\0';
            char *key = line;
            char *value = sep + 2;
            req->headers = realloc(req->headers, (req->header_count + 1) * sizeof(http_header));
            req->headers[req->header_count].key = strdup(key);
            req->headers[req->header_count].value = strdup(value);
            req->header_count++;
        }
        line = strtok(NULL, "\r\n");
    }
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
