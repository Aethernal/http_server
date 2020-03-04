//
// Created by florian.aubin on 04/03/2020.
//

#ifndef HTTP_SERVER_HTTP_H
#define HTTP_SERVER_HTTP_H

#include <stdlib.h>
#include <string.h>

/*
 * mime example
 * application/pdf
 * application/zip
 * application/octet-stream
 * image/jpeg, image/png, and image/svg+xml.
 */
const char* DEFAULT_MIME = "application/octet-stream";

typedef struct content {
    char* content;
    char* content_type;
    long content_length;
} Content;

typedef struct header {
    char* name;
    char* value;
} Header;

typedef struct query {
    char* name;
    char* value;
} Query;

typedef struct response {
    int clientfd;
    int response_code;
    Header **headers;
    Content content;
} Response;

/*
 * filter version 1.1 or throw 505
 * filter method GET | HEAD | POST | PUT | DELETE or throw 405
 * uri max length 2048 or throw 414
 * headers name max size 1024 or throw 431
 * payload :
 *  content-length not found throw 411
 *  payload too large throw 413
 *  unsupported content-type throw 415
 */
typedef struct request {
    int clientfd;
    char* version;
    char* method;
    char* uri;
    Query *query;
    Header *headers;
    Content payload;
} Request;

/*
 * parse request
 * on failure  http response code is sent by this function
 * return NULL on failure -> abort request processing
 */
Request* http_parse_request(int clientfd);

/*
 * generate http response from Response object
 * "HTTP/{version[1.1]} {response_code} {response_code_message}"
 *	"date: {date}"
 *	"Accept-Ranges: bytes" <- to resume download instead of restart
 *	"Content-type: {content_type}"
 *	"Content-Length: {content_length}"
 *
 *
 *	{content}
 */
void http_send_response(Response *response); // clear response structure after send

/*
 * get specific header from headers list
 * return NULL if not found
 */
Header* http_header_get(const char* name, Header *headers);

#endif //HTTP_SERVER_HTTP_H
