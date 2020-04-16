//
// Created by florian.aubin on 16/04/2020.
//


#ifndef HTTP_SERVER_FILE_EXPLORER_SERVICE_H
#define HTTP_SERVER_FILE_EXPLORER_SERVICE_H

#include <stdbool.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>

#include "logger.h"
#include "http.h"

enum pathType {
    isNothing = 0,
    isFile,
    isDirectory
};

extern char* workSpacePath;

char* getFullUri(char* uriPart);
enum pathType getServiceIsAvailable(char* uri);
void getFileContent(char* uri, Response* resp);
char* getDirectoryContent(char* local_path, char* uri);
char* getFileName(char* uri);

#endif //HTTP_SERVER_SERVICES_H
