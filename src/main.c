#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <uv.h>

#include "applog.h"
#include "http.h"
#include "server.h"

#define DEFAULT_PORT 3000
#define DEFAULT_BACKLOG 128

void on_request(http_request *req, uv_stream_t *client)
{
    _debug("Headers Content_Type: %s", http_get_header(req, "Content-type"));
    _debug("Headers User-Agent: %s", http_get_header(req, "user-agent"));
    _debug("Headers Host: %s", http_get_header(req, "HosT"));

    _debug("REQ Method: %s", req->method);
    _debug("REQ Path: %s", req->path);
    _debug("REQ Version: %s", req->version);

    _debug("Headers count: %zu", req->header_count);
    _debug("Content length: %d", req->content_length);
    _debug("Total Read: %zu", req->total_read);

    if (req->total_read >= req->content_length)
    {
        _info("Corpo da requisição completamente lido");

        req->json("{ \"message\": \"Hello world!\" }", client);

        // @todo Fazer req->end
        http_request_free(req);
        uv_read_stop(client);
    }
}

int main()
{
    server *srv = server_create(on_request);

    return server_listen(srv, "0.0.0.0", DEFAULT_PORT);
}
