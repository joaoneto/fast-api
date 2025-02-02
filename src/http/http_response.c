#include <stdlib.h>

#include "applog.h"
#include "http/http.h"

http_response_t *http_response_create()
{
    http_response_t *res = (http_response_t *)malloc(sizeof(http_response_t));
    if (!res)
    {
        _err("Falha ao alocar memória para response");
        return NULL;
    }

    res->headers = http_headers_create();
    res->json = http_send_json;

    return res;
}

char *http_response_json(http_status_code_t status_code, const char *json_body, const char *custom_headers)
{
    if (!json_body)
    {
        json_body = "{}";
    }

    size_t content_length = strlen(json_body);

    // Obtém a mensagem associada ao código de status
    const char *status_message = http_status_str(status_code);

    // Calcula o tamanho necessário para a resposta HTTP
    int needed_size = snprintf(NULL, 0, HTTP_RESPONSE_TEMPLATE, status_code, status_message, custom_headers ? custom_headers : "", content_length, json_body);
    if (needed_size <= 0)
    {
        fprintf(stderr, "Erro ao calcular tamanho da resposta HTTP!\n");
        return NULL;
    }

    size_t response_size = (size_t)needed_size + 1; // +1 para o '\0'
    char *response = malloc(response_size);
    if (!response)
    {
        fprintf(stderr, "Falha ao alocar memória para resposta HTTP!\n");
        return NULL;
    }

    // Preenche o buffer com a resposta formatada
    snprintf(response, response_size, HTTP_RESPONSE_TEMPLATE, status_code, status_message, custom_headers ? custom_headers : "", content_length, json_body);

    return response;
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
