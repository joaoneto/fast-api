#ifndef HTTP_HEADERS_H
#define HTTP_HEADERS_H

#include "http/http.h"

typedef struct http_header_t
{
    char *key;
    char *value;
    struct http_header_t *next;
} http_header_t;

typedef struct
{
    http_header_t *head;
} http_headers_t;

http_headers_t *http_headers_create();

void http_headers_add(http_headers_t *headers, const char *key, const char *value);

char *http_header_get(http_headers_t *headers, const char *key);

char *http_headers_debug(http_headers_t *headers);

void http_headers_free(http_headers_t *headers);

#endif
