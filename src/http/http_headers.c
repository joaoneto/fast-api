#include <stdlib.h>
#include <string.h>
#include "applog.h"
#include "http/http.h"

static http_header_pool_t header_pool = {.index = 0};

static uv_once_t header_pool_once = UV_ONCE_INIT;

static void http_header_pool_init()
{
    uv_mutex_init(&header_pool.lock);
}

http_header_t *http_header_pool_allocate()
{
    uv_mutex_lock(&header_pool.lock);
    http_header_t *header = NULL;

    // Verifique a alocação
    if (header_pool.index < HEADER_POOL_SIZE)
    {
        header = &header_pool.headers[header_pool.index++];
    }
    else
    {
        header = (http_header_t *)malloc(sizeof(http_header_t));
        if (!header)
        {
            _err("Falha ao alocar memória para header");
            uv_mutex_unlock(&header_pool.lock);
            return NULL; // Evita continuar se não alocou memória
        }
    }

    uv_mutex_unlock(&header_pool.lock);
    return header;
}

static void http_header_pool_free(http_header_t *header)
{
    uv_mutex_lock(&header_pool.lock);

    // Verifica se o header pertence ao pool
    if (header >= &header_pool.headers[0] && header < &header_pool.headers[HEADER_POOL_SIZE])
    {
        // O header pertence ao pool, apenas reduzimos o índice
        if (header_pool.index > 0)
        {
            header_pool.index--;
        }
    }
    else
    {
        // Se não pertence ao pool, libera com free
        free(header);
    }

    uv_mutex_unlock(&header_pool.lock);
}

http_headers_t *http_headers_create()
{
    uv_once(&header_pool_once, http_header_pool_init);

    http_headers_t *headers = (http_headers_t *)malloc(sizeof(http_headers_t));
    if (!headers)
    {
        _err("Falha ao alocar memória para headers");
        return NULL;
    }

    headers->head = NULL;
    uv_mutex_init(&headers->lock);

    return headers;
}

void http_headers_add(http_headers_t *headers, const char *key, const char *value)
{
    if (!headers || !key || !value)
    {
        return;
    }

    http_header_t *new_header = http_header_pool_allocate();
    if (!new_header)
    {
        _err("Falha ao alocar memória para header");
        return;
    }

    new_header->key = strdup(key);
    new_header->value = strdup(value);
    new_header->next = NULL;

    if (!new_header->key || !new_header->value)
    {
        free(new_header->key);
        free(new_header->value);
        http_header_pool_free(new_header);
        _err("Falha ao alocar memória para chave/valor do header");
        return;
    }

    uv_mutex_lock(&headers->lock);
    new_header->next = headers->head;
    headers->head = new_header;
    uv_mutex_unlock(&headers->lock);
}

char *http_headers_get(http_headers_t *headers, const char *key)
{
    if (!headers || !key)
    {
        return NULL;
    }

    uv_mutex_lock(&headers->lock);
    http_header_t *current = headers->head;
    char *value = NULL;

    while (current)
    {
        if (strcasecmp(current->key, key) == 0)
        {
            value = strdup(current->value);
            break;
        }
        current = current->next;
    }

    uv_mutex_unlock(&headers->lock);
    return value;
}

char *http_headers_serialize(http_headers_t *headers)
{
    if (!headers)
    {
        return strdup("\r\n");
    }

    size_t buffer_size = 1; // Para o \0 no final
    http_header_t *current = headers->head;

    while (current)
    {
        buffer_size += strlen(current->key) + strlen(current->value) + 4; // ": " + "\r\n"
        current = current->next;
    }

    char *buffer = (char *)malloc(buffer_size);
    if (!buffer)
    {
        _err("Falha ao alocar memória para serialização de headers");
        return NULL;
    }

    buffer[0] = '\0';

    uv_mutex_lock(&headers->lock);
    current = headers->head;
    size_t offset = 0;

    while (current && offset < buffer_size)
    {
        int written = snprintf(buffer + offset, buffer_size - offset, "%s: %s\r\n", current->key, current->value);
        if (written < 0 || (size_t)written >= buffer_size - offset)
        {
            _err("Erro ao serializar headers");
            free(buffer);
            uv_mutex_unlock(&headers->lock);
            return NULL;
        }
        offset += written;
        current = current->next;
    }

    uv_mutex_unlock(&headers->lock);
    return buffer;
}

void http_headers_free(http_headers_t *headers)
{
    if (!headers)
    {
        return;
    }

    uv_mutex_lock(&headers->lock);

    http_header_t *current = headers->head;
    while (current)
    {
        http_header_t *temp = current;
        current = current->next;
        free(temp->key);
        free(temp->value);
        http_header_pool_free(temp);
    }

    headers->head = NULL;
    uv_mutex_unlock(&headers->lock);
    uv_mutex_destroy(&headers->lock);

    free(headers);
}
