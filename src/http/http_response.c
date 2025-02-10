#include <stdlib.h>
#include <uv.h>

#include "applog.h"
#include "http/http.h"
#include "server.h"

#define DEFAULT_SHUTDOWN_TIMEOUT_MS 100
#define DEFAULT_HEADERS_BYTES 128

http_response_t *http_response_create()
{
    http_response_t *res = malloc(sizeof(http_response_t));
    if (!res)
    {
        _err("Erro ao alocar memória para response");
        return NULL;
    }

    res->headers = (char *)malloc(DEFAULT_HEADERS_BYTES);
    if (!res->headers)
    {
        free(res);
        return NULL;
    }
    res->headers[0] = '\0';

    res->status = HTTP_OK;
    res->header = http_response_set_header;
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
    uv_handle_t *hande = (uv_handle_t *)timer->data;

    if (hande && uv_is_active(hande))
    {
        uv_close(hande, on_close);
    }

    free(timer);
}

static void on_shutdown(uv_shutdown_t *shutdown_req, int status)
{
    if (status < 0)
    {
        fprintf(stderr, "Erro no shutdown: %s\n", uv_strerror(status));
    }

    uv_loop_t *loop = shutdown_req->handle->loop;

    /* Aloca o timer para fechar o handle após um pequeno delay */
    uv_timer_t *timer = (uv_timer_t *)malloc(sizeof(uv_timer_t));
    if (!timer)
    {
        _err("Erro ao alocar memória para uv_timer_t");
        /* Se não conseguir alocar o timer, fecha imediatamente o handle */
        uv_close((uv_handle_t *)shutdown_req->handle, on_close);
        free(shutdown_req);
        return;
    }

    /* Inicializa o timer e verifica o retorno */
    int ret = uv_timer_init(loop, timer);
    if (ret < 0)
    {
        _err("Erro ao inicializar timer: %s", uv_strerror(ret));
        free(timer);
        uv_close((uv_handle_t *)shutdown_req->handle, on_close);
        free(shutdown_req);
        return;
    }

    /* Associa o timer ao handle, caso necessário, ou utilize o data conforme sua lógica */
    timer->data = shutdown_req->handle;

    /* Inicia o timer com um delay definido */
    ret = uv_timer_start(timer, on_timer_close, DEFAULT_SHUTDOWN_TIMEOUT_MS, 0);
    if (ret < 0)
    {
        _err("Erro ao iniciar timer: %s", uv_strerror(ret));
        uv_close((uv_handle_t *)shutdown_req->handle, on_close);
        free(timer);
        free(shutdown_req);
        return;
    }

    /* Libera o shutdown_req pois já não será mais utilizado */
    free(shutdown_req);
}

static void on_write_end(uv_write_t *req, int status)
{
    http_response_write_req_t *wr = (http_response_write_req_t *)req;

    if (status < 0)
    {
        _err("Erro ao escrever: %s", uv_strerror(status));
    }

    _debug("Escrita finalizada, recurso liberado");

    server_conn_t *conn = (server_conn_t *)req->handle->data;

    if (conn->req->bytes_received >= conn->req->content_length)
    {
        _debug("Fechando cliente (close)");
        uv_close((uv_handle_t *)req->handle, on_close);
    }
    else
    {
        _debug("Fechando cliente (shutdown)");
        uv_shutdown_t *shutdown_req = (uv_shutdown_t *)malloc(sizeof(uv_shutdown_t));
        if (shutdown_req)
        {
            shutdown_req->data = req->handle;
            int shutdown_ret = uv_shutdown(shutdown_req, req->handle, on_shutdown);
            if (shutdown_ret)
            {
                _err("Erro ao iniciar uv_shutdown: %s", uv_strerror(shutdown_ret));
                free(shutdown_req);
            }
        }
    }

    if (wr->response)
    {
        free(wr->response);
        wr->response = NULL;
    }

    if (wr->req.bufs)
    {
        free(wr->req.bufs);
        wr->req.bufs = NULL;
    }

    free(wr);
}

int http_response_set_header(uv_stream_t *client, const char *key, const char *value)
{
    server_conn_t *conn = (server_conn_t *)client->data;
    if (!conn->res || !conn->res->headers || !key || !value)
    {
        return -1;
    }
    size_t current_len = strlen(conn->res->headers);
    size_t key_len = strlen(key);
    size_t value_len = strlen(value);
    size_t required_size = current_len + key_len + value_len + 4; // ": " + "\r\n" + '\0'

    // Expande se necessário
    if (required_size >= DEFAULT_HEADERS_BYTES)
    {
        size_t new_size = required_size + DEFAULT_HEADERS_BYTES;
        char *new_headers = (char *)realloc(conn->res->headers, new_size);
        if (!new_headers)
        {
            return -1;
        }
        conn->res->headers = new_headers;
    }

    // Adiciona o cabeçalho de forma segura
    snprintf(conn->res->headers + current_len, required_size - current_len + 2, "%s: %s\r\n", key, value);

    return 0;
}

int http_response_send(uv_stream_t *client, const char *str_body)
{
    server_conn_t *conn = (server_conn_t *)client->data;
    http_status_code_t status = conn->res->status;

    uv_read_stop(client);

    // Obtém a mensagem associada ao código de status
    const char *status_message = http_status_str(status);

    // Calcula o tamanho necessário para a resposta HTTP
    int needed_size = snprintf(NULL, 0, HTTP_RESPONSE_TEMPLATE, status, status_message, conn->res->headers, str_body);
    if (needed_size <= 0)
    {
        _err("Erro ao calcular tamanho da resposta HTTP");
        return -1;
    }

    // +1 para o '\0'
    size_t response_size = (size_t)needed_size + 1;
    char *response = (char *)malloc(response_size);
    if (!response)
    {
        _err("Erro ao alocar memória para resposta HTTP");
        return -1;
    }

    // Preenche o buffer com a resposta formatada
    snprintf(response, response_size, HTTP_RESPONSE_TEMPLATE, status, status_message, conn->res->headers, str_body);

    _debug("Enviando resposta para o client:\n%s", response);
    http_response_write_req_t *write_req = (http_response_write_req_t *)malloc(sizeof(http_response_write_req_t));
    if (!write_req)
    {
        _err("Erro ao alocar memória para write_req_t");
        free(response);
        return -1;
    }
    write_req->response = response;

    uv_buf_t buf = uv_buf_init((char *)response, strlen(response));
    int r = uv_write((uv_write_t *)write_req, client, &buf, 1, on_write_end);
    if (r != 0)
    {
        _err("Erro escrevendo resposta para o cliente: %s", uv_strerror(r));
        free(write_req->response);
        free(write_req);
        return r;
    }

    return 0;
}

void http_response_free(http_response_t *res)
{
    free(res->headers);
    free(res);
}
