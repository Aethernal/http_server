//
// Created by florian.aubin on 26/02/2020.
//

#ifndef HTTP_SERVER_MAIN_H
#define HTTP_SERVER_MAIN_H

#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

/* TODO

 * create socket server on selected port
 * create listening loop
 * fork on request reception
 * forked take the socket content to parsing
 * parent continue his loop
*/

/*
 * maximum number of concurrent client
 */
enum {
    max_client = 100,
    max_pending_connection = 10000
};

/*
 * search available ipv4 addr and bind to port
 * then listen for incoming connection
 */
void bind_server_socket(const char *port);

/*
 * main request handler loop
 * dispatch to another threads
 */
void http_serve(const char *port);

#endif //HTTP_SERVER_MAIN_H
