#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string.h>
#include <uv.h>

#include "http/http_status.h"

#define HTTP_RESPONSE_TEMPLATE \
    "HTTP/1.1 %d %s\r\n"       \
    "%s"                       \
    "Content-Length: %zu\r\n"  \
    "Connection: close\r\n"    \
    "\r\n"                     \
    "%s"

typedef struct
{
    http_headers_t *headers; // Armazena os headers HTTP
    int (*json)(const char *req, uv_stream_t *client);
} http_response_t;

http_response_t *http_response_create();

char *http_response_json(http_status_code_t status_code, const char *json_body, const char *custom_headers);

void http_response_free(http_response_t *res);

#endif
