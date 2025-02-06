#ifndef BUFFER_NODE_H
#define BUFFER_NODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#define BUFFER_POOL_SIZE 1024 // Número de buffers no pool
#define BUFFER_SIZE 4096      // Tamanho de cada buffer

typedef struct buffer_node
{
    char *buffer;
    struct buffer_node *next;
} buffer_node_t;

typedef struct
{
    buffer_node_t *free_list;
    uv_mutex_t lock;
} buffer_pool_t;

void buffer_pool_init();

char *buffer_pool_acquire();

void buffer_pool_release(char *buffer);

#endif
