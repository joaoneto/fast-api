#include <stdlib.h>
#include <uv.h>

#include "applog.h"
#include "http/http.h"
#include "server.h"

#define DEFAULT_SHUTDOWN_TIMEOUT_MS 300

http_response_t *http_response_create()
{
    http_response_t *res = (http_response_t *)malloc(sizeof(http_response_t));
    if (!res)
    {
        _err("Erro ao alocar memória para response");
        return NULL;
    }

    res->status = HTTP_OK;
    res->send = http_response_send;

    return res;
}

static void on_close(uv_handle_t *client)
{
    server_conn_free((server_conn_t *)client->data);
    free(client);
}

void on_timer_close(uv_timer_t *timer)
{
    uv_close((uv_handle_t *)timer->data, on_close);
    free(timer);
}

static void on_shutdown(uv_shutdown_t *shutdown_req, int status)
{
    if (status < 0)
    {
        fprintf(stderr, "Erro no shutdown: %s\n", uv_strerror(status));
    }

    uv_timer_t *timer = (uv_timer_t *)malloc(sizeof(uv_timer_t));
    uv_timer_init(shutdown_req->handle->loop, timer);
    timer->data = shutdown_req->handle;
    uv_timer_start(timer, on_timer_close, DEFAULT_SHUTDOWN_TIMEOUT_MS, 0);

    free(shutdown_req);
}

static void on_write_end(uv_write_t *write_req, int status)
{
    if (status < 0)
    {
        _err("Erro ao escrever: %s", uv_strerror(status));
    }

    uv_shutdown_t *shutdown_req = (uv_shutdown_t *)malloc(sizeof(uv_shutdown_t));
    shutdown_req->data = write_req->handle;
    uv_shutdown(shutdown_req, write_req->handle, on_shutdown);

    free(write_req);
}

int http_response_send(const char *str_body, const char *headers, uv_stream_t *client)
{
    server_conn_t *conn = (server_conn_t *)client->data;
    http_response_t *res = (http_response_t *)conn->res;
    http_status_code_t status = res->status;

    uv_read_stop(client);

    if (!res)
    {
        _err("Res não existe para escrever a resposta");
        return -1;
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

    return 0;
}

void http_response_free(http_response_t *res)
{
    free(res);
}
