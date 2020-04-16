//
// Created by florian.aubin on 16/04/2020.
//


#ifndef HTTP_SERVER_FILE_EXPLORER_SERVICE_H
#define HTTP_SERVER_FILE_EXPLORER_SERVICE_H

#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "logger.h"

enum pathType {
    isNothing = -1,
    isFile =0,
    isDirectory
};

extern char* workSpacePath;

enum pathType getServiceIsAvailable(char* uri);

#endif //HTTP_SERVER_SERVICES_H
