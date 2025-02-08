#include <stdlib.h>

#include "applog.h"
#include "http/http.h"

http_request_t *http_request_create()
{
    http_request_t *req = (http_request_t *)malloc(sizeof(http_request_t));
    if (!req)
    {
        _err("Falha ao alocar memória para request");
        return NULL;
    }

    req->bytes_received = 0;
    req->content_length = 0;
    req->header_parsed = 0;

    return req;
}

void http_request_parse_headers(http_request_t *req, const uv_buf_t *buffer)
{
    char *header_end = strstr(buffer->base, "\r\n\r\n");

    if (!header_end)
    {
        return;
    }

    size_t header_len = (header_end - buffer->base);

    char *headers_str = strndup(buffer->base, header_len);

    char *content_length_pos = strcasestr(headers_str, "content-length: ");
    if (content_length_pos)
    {
        content_length_pos += 16;
        req->content_length = strtol(content_length_pos, NULL, 10);
    }

    char *first_line = strtok(headers_str, "\r\n");
    char *first_line_copy = strdup(first_line);
    char *saveptr;
    req->method = strdup(strtok_r(first_line_copy, " ", &saveptr));
    req->path = strdup(strtok_r(NULL, " ", &saveptr));
    req->version = strdup(strtok_r(NULL, "\r\n", &saveptr));

    req->headers = strdup(headers_str + strlen(first_line) + 2);

    free(first_line_copy);
    free(headers_str);

    req->header_parsed = 1;
}

void http_request_free(http_request_t *req)
{
    if (!req)
        return;
    free(req->method);
    free(req->path);
    free(req->version);
    free(req->headers);
    free(req);
}
