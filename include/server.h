#ifndef SERVER_H
#define SERVER_H

#include <uv.h>

#include "http/http.h"

typedef void (*server_cb)(http_request_t *req, uv_stream_t *client);

typedef struct
{
    uv_tcp_t socket;
    uv_loop_t *loop;
    server_cb cb;
} server;

typedef struct
{
    server *srv;
    http_request_t *req;
} server_conn;

int server_listen(server *srv, const char *ip, int port);

server *server_create(server_cb cb);

#endif
