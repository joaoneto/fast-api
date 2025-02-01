#ifndef HTTP_H
#define HTTP_H

#include <string.h>
#include <uv.h>

#define HTTP_RESPONSE_TEMPLATE                          \
    "HTTP/1.1 200 OK\r\n"                               \
    "Content-Type: application/json; charset=utf-8\r\n" \
    "Content-Length: %zu\r\n"                           \
    "Connection: close\r\n"                             \
    "\r\n"                                              \
    "%s"

typedef struct
{
    uv_stream_t *client;   // Stream do cliente
    char *headers;         // Armazena os headers HTTP
    char *body;            // Buffer para armazenar o corpo da requisição
    int header_parsed;     // Flag para indicar se o header já foi processado
    size_t total_read;     // Total de bytes lidos do corpo
    size_t content_length; // Tamanho esperado do conteúdo
    int (*json)(const char *req, uv_stream_t *client);
} http_request;

http_request *http_request_create(uv_stream_t *client);

char *http_json_response(const char *json_body);

int http_send(const char *res, uv_stream_t *client);

int http_send_json(const char *res, uv_stream_t *client);

void http_request_free(http_request *req);

#endif
