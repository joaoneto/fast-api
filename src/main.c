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
    if (req->total_read >= req->content_length)
    {
        _info("Corpo da requisição completamente lido");

        req->json("{ \"message\": \"Hello world!\" }", client);

        free(req->headers);
        req->headers = NULL;

        uv_read_stop(client);
    }
}

int main()
{
    server *srv = server_create(on_request);

    return server_listen(srv, "0.0.0.0", DEFAULT_PORT);
}
