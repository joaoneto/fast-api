#include <stdlib.h>
#include <uv.h>

#include "applog.h"
#include "http/http.h"
#include "server.h"

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
    res->json = http_response_json;
    res->end = http_response_end;

    return res;
}

int http_response_json(const char *json_body, uv_stream_t *client)
{
    server_conn_t *conn = (server_conn_t *)client->data;

    http_response_t *res = (http_response_t *)conn->res;

    http_status_code_t status = res->status;

    if (!conn || !res)
    {
        _err("Erro ao obter conexão da resposta HTTP");
        return 1;
    }

    if (!json_body)
    {
        json_body = "{}";
    }

    // Verifica se existe header Content-Type na request
    if (!http_headers_get(res->headers, "content-type"))
    {
        // Se não existir, usa o mesmo Content-Type da request na response
        char *req_content_type = http_headers_get(conn->req->headers, "content-type");
        http_headers_add(res->headers, "Content-Type", req_content_type ? req_content_type : "application/json");
    }

    // Verifica se existe header Content-Length na request
    if (!http_headers_get(res->headers, "content-length"))
    {
        size_t content_length = strlen(json_body);
        int len = snprintf(NULL, 0, "%zu", content_length);
        if (len < 0)
        {
            _debug("Erro ao calcular o tamanho");
            return 1;
        }

        char *content_length_str = (char *)malloc((len + 1) * sizeof(char)); // +1 para o '\0'

        if (content_length_str != NULL)
        {
            // Converte size_t para string
            snprintf(content_length_str, len + 1, "%zu", content_length);
            http_headers_add(res->headers, "Content-Length", content_length_str);
            free(content_length_str);
        }
    }

    char *headers = http_headers_serialize(res->headers);
    if (!headers)
    {
        headers = strdup("\r\n");
    }

    // Obtém a mensagem associada ao código de status
    const char *status_message = http_status_str(status);

    // Calcula o tamanho necessário para a resposta HTTP
    int needed_size = snprintf(NULL, 0, HTTP_RESPONSE_TEMPLATE, status, status_message, headers, json_body);
    if (needed_size <= 0)
    {
        _err("Erro ao calcular tamanho da resposta HTTP");
        return 1;
    }

    size_t response_size = (size_t)needed_size + 1; // +1 para o '\0'
    char *response = malloc(response_size);
    if (!response)
    {
        _err("Erro ao alocar memória para resposta HTTP");
        return 1;
    }

    // Preenche o buffer com a resposta formatada
    snprintf(response, response_size, HTTP_RESPONSE_TEMPLATE, status, status_message, headers, json_body);

    int result = http_send(response, client);

    free(headers);
    free(response);

    return result;
}

int http_response_end(uv_stream_t *client)
{
    server_conn_t *conn = (server_conn_t *)client->data;

    if (uv_read_stop(client) != 0)
    {
        _err("Erro parando leitura do request");
        uv_close((uv_handle_t *)client, NULL);
        return -1;
    }

    http_request_free(conn->req);
    http_response_free(conn->res);

    uv_close((uv_handle_t *)client, NULL);

    return 0;
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
