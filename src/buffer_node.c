#include "applog.h"
#include "buffer_node.h"

buffer_pool_t pool;

void buffer_pool_init()
{
    uv_mutex_init(&pool.lock);

    for (int i = 0; i < BUFFER_POOL_SIZE; i++)
    {
        buffer_node_t *node = (buffer_node_t *)malloc(sizeof(buffer_node_t));
        if (!node)
        {
            _err("Falha ao alocar memória para buffer_node");
            continue;
        }

        node->buffer = (char *)aligned_alloc(16, BUFFER_SIZE);
        if (!node->buffer)
        {
            free(node);
            _err("Falha ao alocar memória para buffer");
            continue;
        }

        node->next = pool.free_list;
        pool.free_list = node;
    }
}

char *buffer_pool_acquire()
{
    uv_mutex_lock(&pool.lock);

    buffer_node_t *node = pool.free_list;
    if (node)
    {
        pool.free_list = node->next;
        uv_mutex_unlock(&pool.lock);
        return node->buffer;
    }

    uv_mutex_unlock(&pool.lock);

    return NULL;
}

void buffer_pool_release(char *buffer)
{
    if (!buffer)
        return;

    uv_mutex_lock(&pool.lock);

    buffer_node_t *node = (buffer_node_t *)malloc(sizeof(buffer_node_t));
    if (!node)
    {
        uv_mutex_unlock(&pool.lock);
        return;
    }

    node->buffer = buffer;
    node->next = pool.free_list;
    pool.free_list = node;

    uv_mutex_unlock(&pool.lock);
}
