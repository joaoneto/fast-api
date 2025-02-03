#include <stdlib.h>
#include <uv.h>

#include "applog.h"
#include "http/http.h"

static void on_write(uv_write_t *req, int status)
{
    if (status < 0)
    {
        _err("Erro ao escrever: %s", uv_strerror(status));
        uv_close((uv_handle_t *)req->handle, NULL);
    }

    free(req);
}

int http_send(const char *res, uv_stream_t *client)
{
    uv_write_t *r = malloc(sizeof(uv_write_t));
    uv_buf_t buf = uv_buf_init((char *)res, strlen(res));
    uv_write(r, client, &buf, 1, on_write);

    return 0;
}
