#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string.h>
#include <uv.h>

#include "http/http_status.h"

#define HTTP_RESPONSE_TEMPLATE \
    "HTTP/1.1 %d %s\r\n"       \
    "%s"                       \
    "\r\n"                     \
    "%s"

typedef struct
{
    char *headers;
    http_status_code_t status;
    int (*header)(uv_stream_t *client, const char *key, const char *value);
    int (*send)(uv_stream_t *client, const char *str_body);
} http_response_t;

typedef struct
{
    uv_write_t req;
    char *response;
} http_response_write_req_t;

http_response_t *http_response_create();

int http_response_set_header(uv_stream_t *client, const char *key, const char *value);

int http_response_send(uv_stream_t *client, const char *str_body);

void http_response_free(http_response_t *res);

#endif
