//
// Created by florian.aubin on 05/03/2020.
//


#ifndef HTTP_SERVER_SERVICES_H
#define HTTP_SERVER_SERVICES_H

#include "http.h"
#include "fileExplorerService.h"
#include <zconf.h>

/*
 * redirect a request to a specific service
 * Custom service -> index directory -> 404
 */
void route(Request *request);

#endif //HTTP_SERVER_SERVICES_H
