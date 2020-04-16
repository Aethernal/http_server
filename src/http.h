//
// Created by florian.aubin on 04/03/2020.
//

#ifndef HTTP_SERVER_HTTP_H
#define HTTP_SERVER_HTTP_H

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/socket.h>

/*
 * mime example
 * application/pdf
 * application/zip
 * application/octet-stream
 * image/jpeg, image/png, and image/svg+xml.
 */
static unsigned char* DEFAULT_MIME = "text/html";
static unsigned char* MIME_JPG = "image/jpeg";
static unsigned char* MIME_PNG = "image/png";

static const unsigned char* HTTP_METHOD_GET = "GET";
static const unsigned char* HTTP_METHOD_HEAD = "HEAD";
static const unsigned char* HTTP_METHOD_POST = "POST";
static const unsigned char* HTTP_METHOD_PUT = "PUT";
static const unsigned char* HTTP_METHOD_DELETE = "DELETE";

typedef struct content {
    unsigned char* content;
    unsigned char* content_type;
    unsigned int content_length;
} Content;

typedef struct header {
    unsigned char* name;
    unsigned char* value;
} Header;

typedef struct query {
    unsigned char* name;
    unsigned char* value;
} Query;

typedef struct response {
    unsigned int response_code;
    Header *headers;
    unsigned int header_count;
    Content content;
} Response;

/*
 * filter version HTTP/1.1 or throw 505
 * filter method GET | HEAD | POST | PUT | DELETE or throw 405
 * uri max length 2048 or throw 414
 * headers name max size 1024 or throw 431
 * payload :
 *  content-length not found throw 411
 *  payload too large throw 413
 *  unsupported content-type throw 415
 */
typedef struct request {
    unsigned int clientfd;
    unsigned char* buffer;
    unsigned char* version;
    unsigned char* method;
    unsigned char* uri;
    Query *query;
    unsigned int query_count;
    Header *headers;
    unsigned int header_count;
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
void http_send_response(Request *request, Response *response); // clear response structure after send

/*
 * get specific header from headers list
 * return NULL if not found
 */
unsigned char* http_header_get(const char* name, Request *request);

/*
 * create empty Request structure for a specific fd
 */
Request* http_create_request(int clientfd);

/*
 * free request and free every content
 */
void http_free_request(Request *request);

/*
 * create empty Response for a specific fd
 */
Response* http_create_response();

/*
 * free response and free every content;
 * don't free content type = const char*
 */
void http_free_response(Response *response);

#endif //HTTP_SERVER_HTTP_H
