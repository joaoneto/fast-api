#include <stdlib.h>

#include "http/http_status.h"

const char *http_status_str(http_status_code_t code)
{
    for (size_t i = 0; i < sizeof(http_status_messages) / sizeof(http_status_messages[0]); ++i)
    {
        if (http_status_messages[i].code == code)
        {
            return http_status_messages[i].message;
        }
    }
    return "Unknown Status Code";
}
