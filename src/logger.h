//
// Created by florian.aubin on 27/02/2020.
//

#ifndef HTTP_SERVER_LOGGER_H
#define HTTP_SERVER_LOGGER_H

#include <bits/types/FILE.h>
#include <libio.h>
#include <stdio.h>
#include <stdlib.h>


#define logger_color_red "\e[0;31m"
#define logger_color_bold_red "\e[1;31m"
#define logger_color_green "\e[0;32m"
#define logger_color_bold_green "\e[1;32m"
#define logger_color_yellow "\e[0;33m"
#define logger_color_bold_yellow "\e[01;33m"
#define logger_color_blue "\e[0;34m"
#define logger_color_bold_blue "\e[1;34m"
#define logger_color_magenta "\e[0;35m"
#define logger_color_bold_magenta "\e[1;35m"
#define logger_color_cyan "\e[0;36m"
#define logger_color_bold_cyan "\e[1;36m"
#define logger_color_reset "\e[0m"

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
void logger_success(const char* tag, const char* message);

/*
 * log a message with red tag
 */
void logger_error(const char* tag, const char* message);

/*
 * log a message with yellow tag
 */
void logger_warning(const char* tag, const char* message);

/*
 * log a message with the defined tag color (generic)
 * if log file wasn't opened correctly, log to stdio
 */
void logger_log(const char* tag, const char* message, const char* color);

#endif //HTTP_SERVER_LOGGER_H
