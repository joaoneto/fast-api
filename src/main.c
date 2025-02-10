#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <uv.h>

#include "applog.h"
#include "http/http.h"
#include "server.h"

#define DEFAULT_PORT 3000

void on_request(http_request_t *req, http_response_t *res, uv_stream_t *client)
{
    if (req->bytes_received >= req->content_length)
    {
        _info("Corpo da requisição completamente lido");

        _debug("Headers\n%s", req->headers);

        _debug("REQ Method: %s", req->method);
        _debug("REQ Path: %s", req->path);
        _debug("REQ Version: %s", req->version);

        _debug("Content length: %d", req->content_length);
        _debug("Total Read: %d", req->bytes_received);

        res->status = HTTP_OK;
        res->header(client, "content-type", "application/json");
        res->send(client, "{ \"message\": \"Hello World!\" }");

        // http_response_set_header(client, "content-type", "application/json");
        // http_response_set_header(client, "connection", "close");

        // http_response_send(client, "{ \"message\": \"Hello World!\" }");
    }
}

int main()
{
    uv_loop_t *loop = uv_default_loop();

    server_t *server = server_create(loop, on_request);

    int result = server_listen(server, "0.0.0.0", DEFAULT_PORT);

    if (result != 0)
    {
        _err("Erro: %d", result);

        server_shutdown(server);

        free(loop);
        free(server);
    }

    return result;
}
