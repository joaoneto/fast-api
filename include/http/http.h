#ifndef HTTP_H
#define HTTP_H

#include <string.h>
#include <uv.h>

#include "http/http_headers.h"
#include "http/http_request.h"
#include "http/http_response.h"
#include "http/http_status.h"

int http_send(const char *res, uv_stream_t *client);

#endif
