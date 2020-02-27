//
// Created by florian.aubin on 27/02/2020.
//

#include "logger.h"

static FILE *logFile;

void logger_init(const char* file) {
    logFile = fopen(file, "a+"); // open file & append
    if( logFile == NULL ) {
        logger_error("log - init}", "failed to open log file");
    }
    logger_warning("logger - init","------------------");
    logger_success("logger - init", "logger initialised");
    logger_warning("logger - init","------------------");
}

void logger_exit() {
    if (logFile != NULL ) {
        fclose(logFile);
    }
}

void logger_error(const char* tag, const char* message) {
    logger_log(tag,message,logger_color_red);
}

void logger_success(const char* tag, const char* message) {
    logger_log(tag,message,logger_color_green);
}

void logger_warning(const char* tag, const char* message) {
    logger_log(tag,message,logger_color_yellow);
}

void logger_log(const char* tag, const char* message, const char* color) {
    if (logFile == NULL ) {
        printf("%s{%s}%s %s\n", color, tag, logger_color_reset, message);
    } else {
        fprintf( logFile,"%s{%s}%s %s\n", color, tag, logger_color_reset, message);
        fflush(logFile);
    }
}