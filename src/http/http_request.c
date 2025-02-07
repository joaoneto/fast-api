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

void http_request_parse_line(http_request_t *req, const uv_buf_t *buffer)
{
    char *saveptr;

    // Split a primeira linha diretamente no buffer
    req->method = strtok_r(buffer->base, " ", &saveptr);
    req->path = strtok_r(NULL, " ", &saveptr);
    req->version = strtok_r(NULL, "\r\n", &saveptr);

    if (!req->method || !req->path || !req->version)
    {
        _err("Falha ao parsear a linha HTTP");
        return;
    }
}

void http_request_parse_headers(http_request_t *req, const uv_buf_t *buffer)
{
    char *header_end = strstr(buffer->base, "\r\n\r\n");

    if (header_end)
    {
        size_t header_len = (header_end - buffer->base) + 4;

        req->content_length = 0;

        req->headers = (char *)malloc(header_len + 1);
        if (req->headers)
        {
            strncpy(req->headers, buffer->base, header_len);
            req->headers[header_len] = '\0';
        }

        char *content_length_pos = strcasestr(buffer->base, "content-length: ");
        if (content_length_pos)
        {
            content_length_pos += 16;
            req->content_length = strtol(content_length_pos, NULL, 10);
        }

        req->header_parsed = 1;
    }
}

void http_request_free(http_request_t *req)
{
    free(req->headers);
    free(req);
}
