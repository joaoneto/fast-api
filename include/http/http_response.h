#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string.h>
#include <uv.h>

#include "applog.h"
#include "http/http_status.h"

#define HTTP_RESPONSE_TEMPLATE \
    "HTTP/1.1 %d %s\r\n"       \
    "%s"                       \
    "Content-Length: %zu\r\n"  \
    "Connection: close\r\n"    \
    "\r\n"                     \
    "%s"

char *http_response_json(http_status_code_t status_code, const char *json_body, const char *custom_headers);

#endif
