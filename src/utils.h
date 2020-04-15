//
// Created by florian.aubin on 27/02/2020.
//

#ifndef HTTP_SERVER_UTILS_H
#define HTTP_SERVER_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * read a line from STDIN
 * returned char need to be freed after use
 */
char* readline();

/*
 * return current date formated for http response, buffer may need to be freed
 */
void current_date(char* buffer, int size);

#endif //HTTP_SERVER_UTILS_H
