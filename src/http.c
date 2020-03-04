//
// Created by florian.aubin on 04/03/2020.
//

#include "http.h"

Request* http_parse_request(int clientfd) {
    Request *request = malloc(sizeof(Request));



}

void http_send_response(Response *response);

Header* http_header_get(const char* name, Header *headers) {
    Header* header = headers;
    while(header->name) {
        if (strcmp(header->name, name) == 0)
            return header;
        header++; // increment iterator on table
    }
    return NULL;
}