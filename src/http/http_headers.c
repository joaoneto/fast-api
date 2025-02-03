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

char *http_headers_get(http_headers_t *headers, const char *key)
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

char *http_headers_serialize(http_headers_t *headers)
{
    if (!headers)
    {
        return strdup("");
    }

    size_t buffer_size = 256;
    char *buffer = (char *)malloc(buffer_size);
    if (!buffer)
    {
        _err("Falha ao alocar memória para serialização de headers");
        return NULL;
    }
    buffer[0] = '\0';

    http_header_t *current = headers->head;
    while (current)
    {
        size_t required_size = strlen(current->key) + strlen(current->value) + 4;
        if (strlen(buffer) + required_size >= buffer_size)
        {
            buffer_size *= 2;
            buffer = (char *)realloc(buffer, buffer_size);
            if (!buffer)
            {
                _err("Falha ao realocar memória para headers");
                return NULL;
            }
        }
        strcat(buffer, current->key);
        strcat(buffer, ": ");
        strcat(buffer, current->value);
        strcat(buffer, "\r\n");
        current = current->next;
    }
    return buffer;
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
