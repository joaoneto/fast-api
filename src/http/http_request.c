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

    req->headers = http_headers_create();
    req->total_read = 0;
    req->content_length = 0;
    req->header_parsed = 0;

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

    while (line)
    {
        char *sep = strchr(line, ':');
        if (sep)
        {
            *sep = '\0';
            char *key = line;
            char *value = sep + 2;
            http_headers_add(req->headers, key, value);
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

    free(req);
}
