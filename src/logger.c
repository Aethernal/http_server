//
// Created by florian.aubin on 27/02/2020.
//

#include "logger.h"

static FILE *logFile;

void logger_init(const char* file)
{
    logFile = fopen(file, "a+"); // open file & append
    if( logFile == NULL )
    {
        logger_error("log - init}", "failed to open log file");
        return;
    }
    logger_success("logger - init", "logger initialised");
    logger_info("logger - init", "output in [%s]", file);
}

void logger_exit()
{
    if (logFile != NULL )
    {
        fclose(logFile);
    }
}

void logger_error(const char* tag, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    logger_log(tag, format, logger_color_red, args);
    va_end(args);
}

void logger_success(const char* tag, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    logger_log(tag, format, logger_color_green, args);
    va_end(args);
}

void logger_warning(const char* tag, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    logger_log(tag, format, logger_color_yellow, args);
    va_end(args);
}

void logger_info(const char* tag, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    logger_log(tag, format, logger_color_cyan, args);
    va_end(args);
}

void logger_content(const char* tag, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    logger_log(tag, format, logger_color_magenta, args);
    va_end(args);
}

void logger_log(const char* tag, const char* format, const char* color, va_list args)
{

    char date [32] = {[0 ... 31] = '\0'};
    time_t current_time;

    time(&current_time);
    strftime(date, sizeof(date)-1, "[%Y-%m-%d %H:%M:%S] -", gmtime(&current_time));

    char* log_format = "%s %s{%s}%s %s\n";
    size_t needed = snprintf(NULL, 0, log_format, date, color, tag, logger_color_reset, format);

    // size for tag format
    char* buffer = malloc(needed);
    sprintf(buffer, log_format, date, color, tag, logger_color_reset, format);

    // make a copy of args or else vsnprintf consume them
    va_list args_dup;
    va_copy(args_dup, args);

    // update size for content format
    int new_needed = vsnprintf(NULL, 0 , buffer, args_dup);

    // realloc only if different size
    buffer = realloc(buffer, (unsigned int)new_needed);

    if (logFile == NULL )
    {
        vprintf( buffer, args);
    }
    else
    {
        vfprintf(logFile, buffer, args);
        fflush(logFile);
    }

}
