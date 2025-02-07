#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string.h>
#include <uv.h>

typedef struct
{
    char *method;          // Método HTTP
    char *path;            // Caminho do request
    char *version;         // Versão HTTP do request
    char *headers;         // Armazena os headers HTTP
    int header_parsed;     // Flag para indicar se o header já foi processado
    size_t bytes_received; // Total de bytes lidos do corpo
    size_t content_length; // Tamanho esperado do conteúdo
} http_request_t;

http_request_t *http_request_create();

void http_request_parse_line(http_request_t *req, const uv_buf_t *buffer);

void http_request_parse_headers(http_request_t *req, const uv_buf_t *buffer);

void http_request_free(http_request_t *req);

#endif
