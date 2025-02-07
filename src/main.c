#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <uv.h>

#include "applog.h"
#include "http/http.h"
#include "server.h"

#define DEFAULT_PORT 3000
#define DEFAULT_BACKLOG 128

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

        res->status = HTTP_NOT_FOUND;
        res->send("{ \"message\": \"Hello world!\" }", "content-type: application/json\r\n", client);

        // res->status = HTTP_GATEWAY_TIMEOUT;
        // res->status = HTTP_REQUEST_TIMEOUT;
        // res->send("Atingiu o tempo limite da requisição!", client);
    }
}

int main()
{
    server_t *server = server_create(on_request);

    return server_listen(server, "0.0.0.0", DEFAULT_PORT);
}
