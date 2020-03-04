//
// Created by florian.aubin on 26/02/2020.
//

#ifndef HTTP_SERVER_MAIN_H
#define HTTP_SERVER_MAIN_H

#include <zconf.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h> // file descriptor close
#include <sys/epoll.h>

/* TODO

 * create socket server on selected port
 * create listening loop
 * fork on request reception
 * forked take the socket content to parsing
 * parent continue his loop
*/

enum {
    max_pending_connection = 10000,
};

extern int serverfd, running;

/*
 * search available ipv4 addr and bind to port
 * then listen for incoming connection
 */
void main_bind_server_socket(const char *interface, const char *port);

/*
 * main request handler loop
 * dispatch to another threads
 */
void main_http_serve(const char *interface, const char *port);

struct headers {
    char* http_version;
    char* accept_ranges;
    int content_length;
};

#endif //HTTP_SERVER_MAIN_H
