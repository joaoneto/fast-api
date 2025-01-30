#ifndef REQUEST_H
#define REQUEST_H

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
    char *request_data;    // Buffer para armazenar os dados

} http_request;

http_request *http_request_create(uv_stream_t *client);

void http_parse_headers(http_request *req, const char *data);

void http_request_append_body(http_request *req, const char *data, size_t len);

char *http_json_response(const char *json_body);

int http_respond(const char *response, uv_stream_t *client, const uv_buf_t *buf, uv_write_cb cb);

void http_request_free(http_request *req);

#endif
