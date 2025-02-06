#include <stdlib.h>
#include <uv.h>

#include "applog.h"
#include "http/http.h"
#include "server.h"

#define CONTENT_TYPE ((http_header_t){"Content-Type", "application/json"})
#define CONTENT_LENGTH ((http_header_t){"Content-Length", NULL})

http_response_t *http_response_create()
{
    http_response_t *res = (http_response_t *)malloc(sizeof(http_response_t));
    if (!res)
    {
        _err("Erro ao alocar memória para response");
        return NULL;
    }

    res->headers = http_headers_create();
    res->status = HTTP_OK;
    res->send = http_response_send;
    res->json = http_response_json;

    return res;
}

static void on_close(uv_handle_t *client)
{
    server_conn_free((server_conn_t *)client->data);
    free(client);
}

static void on_shutdown(uv_shutdown_t *req, int status)
{
    if (status < 0)
    {
        fprintf(stderr, "Erro no shutdown: %s\n", uv_strerror(status));
    }

    // Atrasando para fechar a conexão evitando problemas de liberação de memória
    // @todo Melhorar o tratamento do fechamento da conexão para evitar EPIPE/ECONNRESET e `corrupted double-linked list`
    uv_sleep(1);

    uv_handle_t *client = (uv_handle_t *)req->data;

    if (!uv_is_closing(client))
    {
        uv_close(client, on_close);
    }

    free(req);
}

static void on_write_end(uv_write_t *write_req, int status)
{
    if (status < 0)
    {
        _err("Erro ao escrever: %s", uv_strerror(status));
    }

    uv_shutdown_t *shutdown_req = (uv_shutdown_t *)malloc(sizeof(uv_shutdown_t));
    if (shutdown_req)
    {
        shutdown_req->data = write_req->handle;
        uv_shutdown(shutdown_req, write_req->handle, on_shutdown);
    }

    free(write_req);
}

int http_response_send(const char *str_body, uv_stream_t *client)
{
    server_conn_t *conn = (server_conn_t *)client->data;
    http_response_t *res = (http_response_t *)conn->res;
    http_status_code_t status = res->status;

    uv_read_stop(client);
    uv_timer_stop((uv_timer_t *)conn->timeout);

    if (!res)
    {
        _err("Res não existe para escrever a resposta");
        return -1;
    }

    // Verifica se existe header Content-Length na request
    if (!http_headers_get(res->headers, CONTENT_LENGTH.key))
    {
        size_t content_length = strlen(str_body);

        // Calcula o tamanho da string que representará o Content-Length
        int len = snprintf(NULL, 0, "%zu", content_length);
        if (len < 0)
        {
            _debug("Erro ao calcular o tamanho do %s", CONTENT_LENGTH.key);
            return -1;
        }

        // Aloca um buffer de tamanho fixo
        char content_length_str[32]; // Aloca um buffer de tamanho fixo
        snprintf(content_length_str, sizeof(content_length_str), "%zu", content_length);
        http_headers_add(res->headers, CONTENT_LENGTH.key, content_length_str);
    }

    char *headers = http_headers_serialize(res->headers);
    if (!headers)
    {
        headers = strdup("\r\n");
    }

    // Obtém a mensagem associada ao código de status
    const char *status_message = http_status_str(status);

    // Calcula o tamanho necessário para a resposta HTTP
    int needed_size = snprintf(NULL, 0, HTTP_RESPONSE_TEMPLATE, status, status_message, headers, str_body);
    if (needed_size <= 0)
    {
        _err("Erro ao calcular tamanho da resposta HTTP");
        return -1;
    }

    // +1 para o '\0'
    size_t response_size = (size_t)needed_size + 1;
    char *response = malloc(response_size);
    if (!response)
    {
        _err("Erro ao alocar memória para resposta HTTP");
        return -1;
    }

    // Preenche o buffer com a resposta formatada
    snprintf(response, response_size, HTTP_RESPONSE_TEMPLATE, status, status_message, headers, str_body);

    _debug("Enviando resposta para o client:\n%s", response);

    uv_write_t *write_req = malloc(sizeof(uv_write_t));
    uv_buf_t buf = uv_buf_init((char *)response, strlen(response));
    uv_write(write_req, client, &buf, 1, on_write_end);

    // free(response);

    return 0;
}

int http_response_json(const char *str_json_body, uv_stream_t *client)
{
    server_conn_t *conn = (server_conn_t *)client->data;
    http_response_t *res = (http_response_t *)conn->res;

    if (!str_json_body)
    {
        str_json_body = "{}";
    }

    if (!http_headers_get(res->headers, CONTENT_TYPE.key))
    {
        http_headers_add(res->headers, CONTENT_TYPE.key, CONTENT_TYPE.value);
    }

    return http_response_send(str_json_body, client);
}

void http_response_free(http_response_t *res)
{
    if (!res)
    {
        return;
    }

    http_headers_free(res->headers);

    free(res);
}
