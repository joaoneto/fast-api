#ifndef HTTP_HEADERS_H
#define HTTP_HEADERS_H

#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "applog.h"

#define HEADER_POOL_SIZE 2048

typedef struct http_header_t
{
    char *key;
    char *value;
    struct http_header_t *next;
} http_header_t;

typedef struct
{
    http_header_t *head;
    uv_mutex_t lock;
} http_headers_t;

// Estrutura para o pool de memória de headers
typedef struct
{
    http_header_t headers[HEADER_POOL_SIZE];
    size_t index;
    uv_mutex_t lock;
} http_header_pool_t;

http_headers_t *http_headers_create();

void http_headers_add(http_headers_t *headers, const char *key, const char *value);

char *http_headers_get(http_headers_t *headers, const char *key);

char *http_headers_serialize(http_headers_t *headers);

void http_headers_free(http_headers_t *headers);

#endif
