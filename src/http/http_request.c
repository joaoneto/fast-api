#include <stdlib.h>

#include "applog.h"
#include "http/http.h"

#define DEFAULT_HEADERS_BYTES 128
#define CONTENT_LENGTH_HEADER "content-length: "
#define CONTENT_LENGTH_HEADER_LEN (sizeof(CONTENT_LENGTH_HEADER) - 1)

http_request_t *http_request_create()
{
    http_request_t *req = malloc(sizeof(http_request_t));
    if (!req)
    {
        _err("Falha ao alocar memória para request");
        return NULL;
    }

    req->bytes_received = 0;
    req->content_length = 0;
    req->header_parsed = 0;

    return req;
}

void http_request_parse_headers(http_request_t *req, const uv_buf_t *buffer)
{
    if (!req || !buffer || !buffer->base)
    {
        return;
    }

    // Busca o fim dos headers (duas quebras de linha consecutivas)
    char *header_end = strstr(buffer->base, "\r\n\r\n");
    if (!header_end)
    {
        return;
    }

    // Calcula o tamanho dos headers e faz uma cópia para manipulação
    size_t header_len = header_end - buffer->base;
    char *headers_str = strndup(buffer->base, header_len);
    if (!headers_str)
    {
        return;
    }

    // Procura pelo header "content-length" de forma case-insensitive
    char *content_length_pos = strcasestr(headers_str, CONTENT_LENGTH_HEADER);
    if (content_length_pos)
    {
        // Avança o ponteiro para o início do valor
        content_length_pos += CONTENT_LENGTH_HEADER_LEN;
        req->content_length = strtol(content_length_pos, NULL, 10);
    }

    // Separa a primeira linha (linha de requisição)
    char *first_line = strtok(headers_str, "\r\n");
    if (!first_line)
    {
        free(headers_str);
        return;
    }

    // Faz uma cópia da primeira linha para usar com strtok_r (que modifica a string)
    char *first_line_copy = strdup(first_line);
    if (!first_line_copy)
    {
        free(headers_str);
        return;
    }

    char *saveptr = NULL;
    // Obtém o método HTTP
    char *token = strtok_r(first_line_copy, " ", &saveptr);
    if (token)
    {
        req->method = strdup(token);
    }
    else
    {
        req->method = NULL;
    }

    // Obtém o caminho HTTP
    token = strtok_r(NULL, " ", &saveptr);
    if (token)
    {
        req->path = strdup(token);
    }
    else
    {
        req->path = NULL;
    }

    // Obtém a versão do protocolo HTTP
    token = strtok_r(NULL, "\r\n", &saveptr);
    if (token)
    {
        req->version = strdup(token);
    }
    else
    {
        req->version = NULL;
    }

    // Recupera os headers restantes (depois da primeira linha)
    size_t first_line_len = strlen(first_line);
    if (header_len > first_line_len + 2)
    {
        req->headers = strdup(headers_str + first_line_len + 2);
    }
    else
    {
        req->headers = strdup("");
    }

    // Libera as cópias temporárias
    free(first_line_copy);
    free(headers_str);

    req->header_parsed = 1;
}

void http_request_free(http_request_t *req)
{
    if (!req)
        return;
    free(req->method);
    free(req->path);
    free(req->version);
    free(req->headers);
    free(req);
}
