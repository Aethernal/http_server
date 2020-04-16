//
// Created by florian.aubin on 05/03/2020.
//


#include "services.h"

void route(Request *request) {

    if(strcmp(request->uri, "/home") == 0 ) {

        Response *resp = http_create_response(request->clientfd);
        resp->response_code = 200;

        const char* hello = "Hello World ! ";
        resp->content.content = malloc(strlen(hello));
        resp->content.content_length = strlen(hello);
        strcpy(resp->content.content, hello);

        http_send_response(request, resp);
        close(request->clientfd);

        return;
    }

    // last fallback
    Response *resp = http_create_response(request->clientfd);
    resp->response_code = 404;
    
    const char* notfound = "404 Page Not Found";
    resp->content.content = malloc(strlen(notfound));
    resp->content.content_length = strlen(notfound);
    strcpy(resp->content.content, notfound);

    http_send_response(request, resp);
    close(request->clientfd);

    return;
}