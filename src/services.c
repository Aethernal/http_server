//
// Created by florian.aubin on 05/03/2020.
//


#include "services.h"
#include <errno.h>

extern int errno ;

void route(Request *request)
{

    if(strcmp(request->uri, "/home") == 0 )
    {
        Response *resp = http_create_response(request->clientfd);
        resp->response_code = 200;

        const char* hello = "Hello World ! ";
        resp->content.content = malloc(strlen(hello));
        resp->content.content_length = (int)strlen(hello);
        strcpy(resp->content.content, hello);

        http_send_response(request, resp);

        return;
    }

    char* uri = getFullUri(request->uri);
    enum pathType type = getServiceIsAvailable(uri);
    logger_info("TRUC", "%d", type);
    logger_info("URI", "%s", uri);
    logger_info("URI2", "%s", request->uri);
    logger_info("WS", "%s", workSpacePath);

    if(type != isNothing)
    {

        Response *resp = http_create_response(request->clientfd);
        resp->response_code = 200;

        char* buffer = NULL;

        switch (type)
        {
        case isFile:
            buffer = getFileContent(uri);
            resp->content.content = malloc(strlen(buffer));
            resp->content.content_length = (int)strlen(buffer);
            strcpy(resp->content.content, buffer);
            break;
        case isDirectory:
            buffer = getDirectoryContent(uri, request->uri);
            resp->content.content = malloc(strlen(buffer));
            resp->content.content_length = (int)strlen(buffer);
            strcpy(resp->content.content, buffer);
            break;
        }

        free(uri);
        free(buffer);

        http_send_response(request, resp);

        return;
    }

    free(uri);
    // last fallback
    Response *resp = http_create_response(request->clientfd);
    resp->response_code = 404;

    const char* notfound = "404 Page Not Found";
    resp->content.content = malloc(strlen(notfound));
    resp->content.content_length = (int)strlen(notfound);
    strcpy(resp->content.content, notfound);

    http_send_response(request, resp);

    return;
}
