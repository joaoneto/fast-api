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
    char *key;
    char *value;
} http_header;

typedef struct
{
    uv_stream_t *client; // Stream do cliente
    char *method;
    char *path;
    char *version;
    size_t header_count;
    http_header *headers;  // Armazena os headers HTTP
    int header_parsed;     // Flag para indicar se o header já foi processado
    size_t total_read;     // Total de bytes lidos do corpo
    size_t content_length; // Tamanho esperado do conteúdo
    char *body;            // Buffer para armazenar o corpo da requisição
    int (*json)(const char *req, uv_stream_t *client);
} http_request;

char *http_get_header(http_request *req, const char *key);

http_request *http_request_create(uv_stream_t *client);

char *http_json_response(const char *json_body);

int http_send(const char *res, uv_stream_t *client);

int http_send_json(const char *res, uv_stream_t *client);

void http_parse_request_line(http_request *req, char *line);

void http_parse_headers(http_request *req, char *headers);

void http_request_free(http_request *req);

#endif
