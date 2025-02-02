#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string.h>
#include <uv.h>

#include "http/http_headers.h"
#include "http/http_request.h"
#include "http/http_response.h"

typedef struct
{
    uv_stream_t *client; // Stream do cliente
    char *method;
    char *path;
    char *version;
    size_t header_count;
    http_headers_t *headers; // Armazena os headers HTTP
    int header_parsed;       // Flag para indicar se o header já foi processado
    size_t total_read;       // Total de bytes lidos do corpo
    size_t content_length;   // Tamanho esperado do conteúdo
    char *body;              // Buffer para armazenar o corpo da requisição
    int (*json)(const char *req, uv_stream_t *client);
} http_request_t;

http_request_t *http_request_create(uv_stream_t *client);

void http_request_parse_line(http_request_t *req, char *line);

void http_request_parse_headers(http_request_t *req, char *headers);

void http_request_free(http_request_t *req);

#endif
