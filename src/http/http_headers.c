#include <stdlib.h>

#include "applog.h"
#include "http/http.h"

http_headers_t *http_headers_create()
{
    http_headers_t *headers = (http_headers_t *)malloc(sizeof(http_headers_t));
    if (!headers)
    {
        _err("Falha ao alocar memória para headers");
        return NULL;
    }

    headers->head = NULL;

    return headers;
}

void http_headers_add(http_headers_t *headers, const char *key, const char *value)
{
    if (!headers || !key || !value)
    {
        return;
    }

    http_header_t *new_header = (http_header_t *)malloc(sizeof(http_header_t));
    if (!new_header)
    {
        _err("Falha ao alocar memória para header");
        return;
    }

    new_header->key = strdup(key);
    new_header->value = strdup(value);
    new_header->next = headers->head;
    headers->head = new_header;
}

char *http_header_get(http_headers_t *headers, const char *key)
{
    if (!headers || !key)
    {
        return NULL;
    }

    http_header_t *current = headers->head;
    while (current)
    {
        if (strcasecmp(current->key, key) == 0)
        {
            return current->value;
        }
        current = current->next;
    }
    return NULL;
}

char *http_headers_debug(http_headers_t *headers)
{
    if (!headers || !headers->head)
    {
        return strdup("{}");
    }

    // Espaço para "{\n"
    // 2 caracteres + 1 para '\0'
    size_t total_len = strlen("{\n");

    http_header_t *current = headers->head;
    while (current)
    {
        // Para cada header, usamos o formato:
        // "  \"%s\": \"%s\", \n"
        // 2 (espaços) + 1 (aspas) + 4 (": ") + 1 (aspas) + 4 (", \n") = 12.
        total_len += 12 + strlen(current->key) + strlen(current->value);
        current = current->next;
    }
    // Remover os 2 caracteres extras (", " e "\n") do final (-2)
    // e adicionar espaço para o fechamento "\n}" (2 caracteres) e o caractere nulo. (+3)
    // total_len -= 2;
    // total_len += 3;
    total_len += 1;

    // Alocar buffer suficiente
    char *result = (char *)malloc(total_len);
    if (!result)
    {
        return NULL;
    }

    strcpy(result, "{\n");
    size_t pos = strlen(result);

    // Preenche com cada header
    current = headers->head;
    while (current)
    {
        // Sprintf para escrever diretamente na posição atual
        int written = sprintf(result + pos, "  \"%s\": \"%s\", \n", current->key, current->value);
        pos += written;
        current = current->next;
    }

    // Remove os últimos ", \n" adicionados por último
    if (pos >= 3)
    {
        pos -= 3;
    }

    sprintf(result + pos, "\n}");

    return result;
}

void http_headers_free(http_headers_t *headers)
{
    if (!headers)
    {
        return;
    }

    http_header_t *current = headers->head;
    while (current)
    {
        http_header_t *temp = current;
        current = current->next;
        free(temp->key);
        free(temp->value);
        free(temp);
    }

    free(headers);
}
