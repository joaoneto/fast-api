#ifndef HTTP_H
#define HTTP_H

#include <string.h>
#include <uv.h>

#define HTTP_RESPONSE_TEMPLATE \
    "HTTP/1.1 %d %s\r\n"       \
    "%s"                       \
    "Content-Length: %zu\r\n"  \
    "Connection: close\r\n"    \
    "\r\n"                     \
    "%s"

typedef enum
{
    HTTP_CONTINUE = 100,
    HTTP_SWITCHING_PROTOCOL = 101,
    HTTP_PROCESSING = 102,
    HTTP_EARLY_HINTS = 103,
    HTTP_OK = 200,
    HTTP_CREATED = 201,
    HTTP_ACCEPTED = 202,
    HTTP_NON_AUTHORITATIVE_INFORMATION = 203,
    HTTP_NO_CONTENT = 204,
    HTTP_RESET_CONTENT = 205,
    HTTP_PARTIAL_CONTENT = 206,
    HTTP_MULTIPLE_CHOICES = 300,
    HTTP_MOVED_PERMANENTLY = 301,
    HTTP_FOUND = 302,
    HTTP_SEE_OTHER = 303,
    HTTP_NOT_MODIFIED = 304,
    HTTP_USE_PROXY = 305,
    HTTP_TEMPORARY_REDIRECT = 307,
    HTTP_PERMANENT_REDIRECT = 308,
    HTTP_BAD_REQUEST = 400,
    HTTP_UNAUTHORIZED = 401,
    HTTP_PAYMENT_REQUIRED = 402,
    HTTP_FORBIDDEN = 403,
    HTTP_NOT_FOUND = 404,
    HTTP_METHOD_NOT_ALLOWED = 405,
    HTTP_NOT_ACCEPTABLE = 406,
    HTTP_PROXY_AUTHENTICATION_REQUIRED = 407,
    HTTP_REQUEST_TIMEOUT = 408,
    HTTP_CONFLICT = 409,
    HTTP_GONE = 410,
    HTTP_LENGTH_REQUIRED = 411,
    HTTP_PRECONDITION_FAILED = 412,
    HTTP_PAYLOAD_TOO_LARGE = 413,
    HTTP_URI_TOO_LONG = 414,
    HTTP_UNSUPPORTED_MEDIA_TYPE = 415,
    HTTP_RANGE_NOT_SATISFIABLE = 416,
    HTTP_EXPECTATION_FAILED = 417,
    HTTP_IM_A_TEAPOT = 418,
    HTTP_MISDIRECTED_REQUEST = 421,
    HTTP_UNPROCESSABLE_ENTITY = 422,
    HTTP_LOCKED = 423,
    HTTP_FAILED_DEPENDENCY = 424,
    HTTP_TOO_EARLY = 425,
    HTTP_UPGRADE_REQUIRED = 426,
    HTTP_PRECONDITION_REQUIRED = 428,
    HTTP_TOO_MANY_REQUESTS = 429,
    HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    HTTP_UNAVAILABLE_FOR_LEGAL_REASONS = 451,
    HTTP_INTERNAL_SERVER_ERROR = 500,
    HTTP_NOT_IMPLEMENTED = 501,
    HTTP_BAD_GATEWAY = 502,
    HTTP_SERVICE_UNAVAILABLE = 503,
    HTTP_GATEWAY_TIMEOUT = 504,
    HTTP_VERSION_NOT_SUPPORTED = 505,
    HTTP_VARIANT_ALSO_NEGOTIATES = 506,
    HTTP_INSUFFICIENT_STORAGE = 507,
    HTTP_LOOP_DETECTED = 508,
    HTTP_NOT_EXTENDED = 510,
    HTTP_NETWORK_AUTHENTICATION_REQUIRED = 511
} http_status_code;

typedef struct
{
    http_status_code code;
    const char *message;
} http_status_message;

static const http_status_message http_status_messages[] = {
    {HTTP_CONTINUE, "Continue"},
    {HTTP_SWITCHING_PROTOCOL, "Switching Protocols"},
    {HTTP_PROCESSING, "Processing"},
    {HTTP_EARLY_HINTS, "Early Hints"},
    {HTTP_OK, "OK"},
    {HTTP_CREATED, "Created"},
    {HTTP_ACCEPTED, "Accepted"},
    {HTTP_NON_AUTHORITATIVE_INFORMATION, "Non-Authoritative Information"},
    {HTTP_NO_CONTENT, "No Content"},
    {HTTP_RESET_CONTENT, "Reset Content"},
    {HTTP_PARTIAL_CONTENT, "Partial Content"},
    {HTTP_MULTIPLE_CHOICES, "Multiple Choices"},
    {HTTP_MOVED_PERMANENTLY, "Moved Permanently"},
    {HTTP_FOUND, "Found"},
    {HTTP_SEE_OTHER, "See Other"},
    {HTTP_NOT_MODIFIED, "Not Modified"},
    {HTTP_USE_PROXY, "Use Proxy"},
    {HTTP_TEMPORARY_REDIRECT, "Temporary Redirect"},
    {HTTP_PERMANENT_REDIRECT, "Permanent Redirect"},
    {HTTP_BAD_REQUEST, "Bad Request"},
    {HTTP_UNAUTHORIZED, "Unauthorized"},
    {HTTP_PAYMENT_REQUIRED, "Payment Required"},
    {HTTP_FORBIDDEN, "Forbidden"},
    {HTTP_NOT_FOUND, "Not Found"},
    {HTTP_METHOD_NOT_ALLOWED, "Method Not Allowed"},
    {HTTP_NOT_ACCEPTABLE, "Not Acceptable"},
    {HTTP_PROXY_AUTHENTICATION_REQUIRED, "Proxy Authentication Required"},
    {HTTP_REQUEST_TIMEOUT, "Request Timeout"},
    {HTTP_CONFLICT, "Conflict"},
    {HTTP_GONE, "Gone"},
    {HTTP_LENGTH_REQUIRED, "Length Required"},
    {HTTP_PRECONDITION_FAILED, "Precondition Failed"},
    {HTTP_PAYLOAD_TOO_LARGE, "Payload Too Large"},
    {HTTP_URI_TOO_LONG, "URI Too Long"},
    {HTTP_UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"},
    {HTTP_RANGE_NOT_SATISFIABLE, "Range Not Satisfiable"},
    {HTTP_EXPECTATION_FAILED, "Expectation Failed"},
    {HTTP_IM_A_TEAPOT, "I'm a teapot (418)"},
    {HTTP_MISDIRECTED_REQUEST, "Misdirected Request"},
    {HTTP_UNPROCESSABLE_ENTITY, "Unprocessable Entity"},
    {HTTP_LOCKED, "Locked"},
    {HTTP_FAILED_DEPENDENCY, "Failed Dependency"},
    {HTTP_TOO_EARLY, "Too Early"},
    {HTTP_UPGRADE_REQUIRED, "Upgrade Required"},
    {HTTP_PRECONDITION_REQUIRED, "Precondition Required"},
    {HTTP_TOO_MANY_REQUESTS, "Too Many Requests"},
    {HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE, "Request Header Fields Too Large"},
    {HTTP_UNAVAILABLE_FOR_LEGAL_REASONS, "Unavailable For Legal Reasons"},
    {HTTP_INTERNAL_SERVER_ERROR, "Internal Server Error"},
    {HTTP_NOT_IMPLEMENTED, "Not Implemented"},
    {HTTP_BAD_GATEWAY, "Bad Gateway"},
    {HTTP_SERVICE_UNAVAILABLE, "Service Unavailable"},
    {HTTP_GATEWAY_TIMEOUT, "Gateway Timeout"},
    {HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported"},
    {HTTP_VARIANT_ALSO_NEGOTIATES, "Variant Also Negotiates"},
    {HTTP_INSUFFICIENT_STORAGE, "Insufficient Storage"},
    {HTTP_LOOP_DETECTED, "Loop Detected"},
    {HTTP_NOT_EXTENDED, "Not Extended"},
    {HTTP_NETWORK_AUTHENTICATION_REQUIRED, "Network Authentication Required"}};

typedef struct
{
    char *key;
    char *value;
} http_header;

typedef struct
{
    uv_stream_t *client; // Stream do cliente
    char *method;
    char *path;
    char *version;
    size_t header_count;
    http_header *headers;  // Armazena os headers HTTP
    int header_parsed;     // Flag para indicar se o header já foi processado
    size_t total_read;     // Total de bytes lidos do corpo
    size_t content_length; // Tamanho esperado do conteúdo
    char *body;            // Buffer para armazenar o corpo da requisição
    int (*json)(const char *req, uv_stream_t *client);
} http_request;

const char *http_status_str(http_status_code code);

char *http_get_header(http_request *req, const char *key);

http_request *http_request_create(uv_stream_t *client);

char *http_json_response(http_status_code status_code, const char *json_body, const char *custom_headers);

int http_send(const char *res, uv_stream_t *client);

int http_send_json(const char *res, uv_stream_t *client);

void http_parse_request_line(http_request *req, char *line);

void http_parse_headers(http_request *req, char *headers);

void http_request_free(http_request *req);

#endif
