//
// Created by florian.aubin on 27/02/2020.
//

#include "utils.h"
#include "logger.h"

char* readline()
{
    char* buffer = malloc(1024);
    if(buffer == NULL)
    {
        logger_error("utils - readline", "failed to malloc");
        return NULL;
    }

    fgets(buffer, 256, stdin);
    unsigned long length = strlen(buffer);
    buffer[length - 1] = '\0';
    return buffer;
}

void current_date(char* buffer, unsigned int size)
{
    time_t current_time;
    time(&current_time);
    strftime(buffer, size - 1, "%a, %d %b %Y %T", gmtime(&current_time));
}
