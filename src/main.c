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
    if (req->total_read >= req->content_length)
    {
        _info("Corpo da requisição completamente lido");

        char *all_headers_debug = http_headers_debug(req->headers);
        _debug("Headers %s", all_headers_debug);
        free(all_headers_debug);

        _debug("REQ Method: %s", req->method);
        _debug("REQ Path: %s", req->path);
        _debug("REQ Version: %s", req->version);

        _debug("Headers count: %d", req->header_count);
        _debug("Content length: %d", req->content_length);
        _debug("Total Read: %d", req->total_read);

        res->json("{ \"message\": \"Hello world!\" }", client);

        // @todo Fazer req->end
        http_request_free(req);
        http_response_free(res);
        uv_read_stop(client);
    }
}

int main()
{
    server_t *server = server_create(on_request);

    return server_listen(server, "0.0.0.0", DEFAULT_PORT);
}
