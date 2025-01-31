#ifndef SERVER_H
#define SERVER_H

#include <uv.h>

#include "http.h"

typedef void (*server_cb)(http_request *req, uv_stream_t *client);

typedef struct
{
    uv_tcp_t socket;
    uv_loop_t *loop;
    server_cb cb;
    http_request *req;
} server;

int server_listen(server *srv, const char *ip, int port);

server *server_create(server_cb cb);

#endif
