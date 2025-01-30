#ifndef APPLOG_H
#define APPLOG_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @def APPLOG_LEVEL
 * Definindo o nível de log para exibição.
 * O valor padrão é APPLOG_ERROR. Se desejar, pode definir APPLOG_LEVEL
 * em outro lugar do código para um nível diferente.
 */
#ifdef APPLOG_LEVEL
#else
#define APPLOG_LEVEL APPLOG_ERROR
#endif

#define APPLOG_COLOR_RESET "\033[0m"    /** Cor em ANSI code RESET */
#define APPLOG_COLOR_DEBUG "\033[34m"   /** Cor em ANSI code DEBUG */
#define APPLOG_COLOR_INFO "\033[32m"    /** Cor em ANSI code INFO */
#define APPLOG_COLOR_WARNING "\033[33m" /** Cor em ANSI code WARNING */
#define APPLOG_COLOR_ERROR "\033[31m"   /** Cor em ANSI code ERROR */

/**
 * @enum applog_level
 * Enum dos diferentes níveis de log suportados.
 * - APPLOG_DEBUG: Usado para mensagens de depuração.
 * - APPLOG_INFO: Usado para mensagens informativas.
 * - APPLOG_WARNING: Usado para mensagens de aviso.
 * - APPLOG_ERROR: Usado para mensagens de erro.
 */
typedef enum
{
    APPLOG_DEBUG = 0, /**< Nível de log para debug */
    APPLOG_INFO,      /**< Nível de log para informação */
    APPLOG_WARNING,   /**< Nível de log para alerta */
    APPLOG_ERROR      /**< Nível de log para erro */
} applog_level;

/**
 * @brief Converte o nível de log para um código de cor ANSI.
 *
 * Esta função converte o valor de um nível de log para sua representação em código de cor ANSI.
 * Por exemplo, APPLOG_INFO será convertido para "\033[32m".
 *
 * @param level O nível de log a ser convertido.
 * @return A string correspondente ao nível de log.
 */
const char *applog_levelcolor(applog_level level);

/**
 * @brief Converte o nível de log para uma string.
 *
 * Esta função converte o valor de um nível de log para sua representação em string.
 * Por exemplo, APPLOG_INFO será convertido para "INFO".
 *
 * @param level O nível de log a ser convertido.
 * @return A string correspondente ao nível de log.
 */
const char *applog_levelstr(applog_level level);

/**
 * @brief Função que exibe as mensagens de log de DEBUG.
 */
void _debug(const char *fmt, ...);

/**
 * @brief Função que exibe as mensagens de log de INFO.
 */
void _info(const char *fmt, ...);

/**
 * @brief Função que exibe as mensagens de log de WARNING.
 */
void _warn(const char *fmt, ...);

/**
 * @brief Função que exibe as mensagens de log de ERROR.
 */
void _err(const char *fmt, ...);

#endif
