#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string.h>
#include <uv.h>

#include "http/http_headers.h"
#include "http/http_status.h"

#define HTTP_RESPONSE_TEMPLATE \
    "HTTP/1.1 %d %s\r\n"       \
    "%s"                       \
    "\r\n"                     \
    "%s"

typedef struct
{
    http_headers_t *headers;
    http_status_code_t status;
    int (*json)(const char *json_body, uv_stream_t *client);
} http_response_t;

http_response_t *http_response_create();

int http_response_json(const char *json_body, uv_stream_t *client);

void http_response_free(http_response_t *res);

#endif
