#include <stdlib.h>
#include <uv.h>

#include "http/http.h"

http_request_t *http_request_create(uv_stream_t *client)
{
    http_request_t *req = (http_request_t *)malloc(sizeof(http_request_t));
    if (!req)
    {
        _err("Falha ao alocar memória para request");
        return NULL;
    }

    req->headers = http_headers_create();
    req->body = NULL;
    req->total_read = 0;
    req->content_length = 0;
    req->header_parsed = 0;
    req->json = http_send_json;

    return req;
}

void http_request_parse_line(http_request_t *req, char *line)
{
    char *saveptr;
    req->method = strdup(strtok_r(line, " ", &saveptr));
    req->path = strdup(strtok_r(NULL, " ", &saveptr));
    req->version = strdup(strtok_r(NULL, "\r\n", &saveptr));
}

void http_request_parse_headers(http_request_t *req, char *headers)
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
            http_headers_add(req->headers, key, value);
            req->header_count++;
        }
        line = strtok(NULL, "\r\n");
    }
}

void http_request_free(http_request_t *req)
{
    if (!req)
    {
        return;
    }

    http_headers_free(req->headers);

    if (req->body)
    {
        free(req->body);
        req->body = NULL;
    }

    free(req);
}
