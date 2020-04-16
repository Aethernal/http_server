//
// Created by florian.aubin on 27/02/2020.
//

#ifndef HTTP_SERVER_LOGGER_H
#define HTTP_SERVER_LOGGER_H

#include <bits/types/FILE.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#define logger_color_red "\033[0;31m"
#define logger_color_bold_red "\033[1;31m"
#define logger_color_green "\033[0;32m"
#define logger_color_bold_green "\033[1;32m"
#define logger_color_yellow "\033[0;33m"
#define logger_color_bold_yellow "\033[01;33m"
#define logger_color_blue "\033[0;34m"
#define logger_color_bold_blue "\033[1;34m"
#define logger_color_magenta "\033[0;35m"
#define logger_color_bold_magenta "\033[1;35m"
#define logger_color_cyan "\033[0;36m"
#define logger_color_bold_cyan "\033[1;36m"
#define logger_color_reset "\033[0m"

/*
 * try to create / open the file on write append mode
 */
void logger_init(const char* file);

/*
 * close the log file
 */
void logger_exit();

/*
 * log a message with green tag
 */
void logger_success(const char* tag, const char* format, ...);

/*
 * log a message with red tag
 */
void logger_error(const char* tag, const char* format, ...);

/*
 * log a message with yellow tag
 */
void logger_warning(const char* tag, const char* format, ...);

/*
 * log e message with cyan tag
 */
void logger_info(const char* tag, const char* format, ...);

/*
 * log e message with magenta tag
 */
void logger_content(const char* tag, const char* format, ...);

/*
 * log a message with the defined tag color (generic)
 * if log file wasn't opened correctly, log to stdio
 */
void logger_log(const char* tag, const char* format, const char* color, va_list args);

#endif //HTTP_SERVER_LOGGER_H
