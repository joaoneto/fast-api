#include <stdarg.h>
#include "applog.h"

const char *applog_levelcolor(applog_level level)
{
    switch (level)
    {
    case APPLOG_DEBUG:
        return APPLOG_COLOR_DEBUG;
    case APPLOG_INFO:
        return APPLOG_COLOR_INFO;
    case APPLOG_WARNING:
        return APPLOG_COLOR_WARNING;
    case APPLOG_ERROR:
        return APPLOG_COLOR_ERROR;
    default:
        return APPLOG_COLOR_RESET;
    }
}

const char *applog_levelstr(applog_level level)
{
    switch (level)
    {
    case APPLOG_DEBUG:
        return "DEBUG";
    case APPLOG_INFO:
        return "INFO";
    case APPLOG_WARNING:
        return "WARNING";
    case APPLOG_ERROR:
        return "ERROR";
    default:
        return "INFO";
    }
}

static void applog(applog_level level, const char *fmt, va_list args)
{
    if (level < APPLOG_LEVEL)
        return;

    time_t tempo_atual = time(NULL);
    struct tm *tm_info = localtime(&tempo_atual);

    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    // Verificando se é nível de erro (ERROR), então usar stderr
    FILE *output = (level == APPLOG_ERROR) ? stderr : stdout;

    // Exibe o log no formato [timestamp] [level] com a cor apropriada
    fprintf(output, "%s[%s] [%s] %s", applog_levelcolor(level), timestamp, applog_levelstr(level), APPLOG_COLOR_RESET);

    // Imprime a mensagem formatada
    vfprintf(output, fmt, args);
    fprintf(output, "\n");
}

void _debug(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    applog(APPLOG_DEBUG, fmt, args);
    va_end(args);
}

void _info(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    applog(APPLOG_INFO, fmt, args);
    va_end(args);
}

void _warn(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    applog(APPLOG_WARNING, fmt, args);
    va_end(args);
}

void _err(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    applog(APPLOG_ERROR, fmt, args);
    va_end(args);
}
