#ifndef SERVER_H
#define SERVER_H

#include <uv.h>

#include "http/http_request.h"
#include "http/http_response.h"

typedef void (*server_cb)(http_request_t *req, http_response_t *res, uv_stream_t *client);

typedef struct
{
    uv_tcp_t handle;
    uv_loop_t *loop;
    server_cb cb;
} server_t;
typedef struct
{
    server_t *server;
    http_request_t *req;
    http_response_t *res;
} server_conn_t;

server_conn_t server_create_conn();

int server_listen(server_t *srv, const char *ip, int port);

server_t *server_create(uv_loop_t *loop, server_cb cb);

void server_shutdown(server_t *server);

void server_conn_free(server_conn_t *conn);

#endif
