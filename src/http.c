//
// Created by florian.aubin on 04/03/2020.
//


#include "http.h"
#include "logger.h"
#include "utils.h"

Request *http_create_request(int clientfd)
{
    Request *request = malloc(sizeof(Request));

    request->clientfd = clientfd;
    request->buffer = malloc(65535);
    request->version = NULL;
    request->method = NULL;
    request->uri = NULL;
    request->query = NULL;
    request->query_count = 0;
    request->headers = NULL;
    request->header_count = 0;
    request->payload.content_length = 0;
    request->payload.content_type = DEFAULT_MIME;
    request->payload.content = NULL;

    return request;
}

void http_free_request(Request *request)
{
    free(request->buffer); // <- whole Request content
    free(request);
}

Response *http_create_response()
{
    Response *response = malloc(sizeof(Response));

    response->response_code = 0;
    response->headers = NULL;
    response->header_count = 0;
    response->content.content = NULL;
    response->content.content_type = DEFAULT_MIME;
    response->content.content_length = 0;

    return response;
}


void http_free_response(Response *response)
{

    Header *header = response->headers;
    while (header < response->headers + response->header_count)
    {
        free(header);
        header++;
    }

    free(response->content.content);
    free(response);
}

Request *http_parse_request(int clientfd)
{
    long received_length;
    Request *request = http_create_request(clientfd);

    received_length = recv(clientfd, request->buffer, INT_MAX, 0);

    if (received_length < 0)
    {
        logger_error("http - parse_request", "failed to receive client request");
        free(request->buffer);
        return NULL;
    }
    else
        if (received_length == 0)
        {
            logger_error("http - parse_request", "client disconnected");
            free(request->buffer);
            return NULL;
        }
        else
        {
            request->buffer[received_length] = '\0'; // set EOF at the end of the buffer

            char *request_position = request->buffer;

            request->method = strtok_r(request->buffer, " \t\r\n", &request_position);
            request->uri = strtok_r(NULL, " \t", &request_position);
            request->version = strtok_r(NULL, " \t\r\n", &request_position);


            char *query_start;
            if ((query_start = strchr(request->uri, '?')))
            {
                *query_start++ = '\0'; // replace ? by \0 to delimit uri and query and increment by one to get query start

                unsigned int query_count = 5;

                Query *queries = malloc(sizeof(Query) * query_count); // allocate 10 by 10
                Query *query = queries;
                char *queries_position;

                while (query < queries + query_count)
                {
                    char *parameter;
                    char *name = NULL, *value = NULL;
                    char *query_position;

                    parameter = strtok_r(query_start, "\r\n& \t", &queries_position);
                    if (!parameter) // break loop on empty name
                        break;

                    name = strtok_r(parameter, "\r\n= \t", &query_position);
                    value = strtok_r(NULL, "\r\n= \t", &query_position);

                    if (name != NULL && value != NULL)
                    {
                        query->name = name;
                        query->value = value;

                        logger_info("http - parse_request", "query [%s = %s]", name, value);
                    }

                    request->query_count++;
                    query++;
                    if (query == queries + query_count)
                        queries = realloc(queries, request->query_count + query_count);

                    query_start = NULL;
                }

                request->query = queries;

            }
            else
            {
                query_start = NULL;
            }

            logger_info("http - parse_request", "Request [%s] on [%s] using version [%s]", request->method, request->uri,
                        request->version);

            /*
             * parse headers
             */
            char *payload_check = NULL;
            unsigned int header_count = 10;

            Header *headers = malloc(sizeof(Header) * header_count); // allocate 10 by 10
            Header *header = headers;

            while (header < headers + header_count)
            {
                char *name, *value;

                name = strtok_r(NULL, "\r\n: \t", &request_position);
                if (!name) // break loop on empty name
                    break;

                value = strtok_r(NULL, "\r\n", &request_position);
                while (*value && *value == ' ')
                    value++;

                // insert in header struct
                header->name = name;
                header->value = value;

                logger_info("http - parse_request", "header %s = %s", name, value);

                // next header, increase array if needed
                request->header_count++;
                header++;
                if (header == headers + header_count)
                    headers = realloc(headers, request->header_count + header_count);

                /*
                 * check after value to see if it is the last header
                 */
                payload_check = value + 1 + strlen(value);
                if (payload_check[1] == '\r' && payload_check[2] == '\n')
                    break;
            }
            request->headers = headers;

            char *content_length_header_value = http_header_get("Content-Length", request);
            char *content_type_header_value = http_header_get("Content-Type", request);

            logger_info("http - parse_request", "header count [%d]", request->header_count);

            request->payload.content = payload_check + 3;
            request->payload.content_length = content_length_header_value ?
                        (int)atol(content_length_header_value) :
                        (int)(received_length - (request->payload.content - request->buffer));
            request->payload.content_type = content_type_header_value ? content_type_header_value : DEFAULT_MIME;

            logger_info("http - parse_request", "Payload type [%s], length [%d]",
                        request->payload.content_type, request->payload.content_length);
            if (request->payload.content_length > 0)
            {
                logger_info("http - parse_request", "Payload\n%s", request->payload.content);
            }

            return request;
    }
}

void http_send_response(Request *request, Response *response)
{
    const char *header_structure = "%s: %s\r\n";
    const char response_structure[] = "HTTP/1.1 %d %s\r\n" \
                "Date: %s\r\n" \
                "Content-Type: %s\r\n" \
                "Content-Length: %d";

    char *response_code_info = "Unknown";

    /*
     * TODO use a referential
    */
    if (response->response_code >= 500)
    {
        response_code_info = "Server Error";
    }
    else
        if (response->response_code >= 400)
        {
            response_code_info = "Client Error";
        }
        else
            if (response->response_code >= 300)
            {
                response_code_info = "Redirection";
            }
            else
                if (response->response_code >= 200)
                {
                    response_code_info = "Success";
                }
                else
                    if (response->response_code >= 100)
                    {
                        response_code_info = "Informational";
                    }

    // get current date
    char date[32] = {[0 ... 31] = '\0'};
    current_date(date, 32);

    // generate additional header response_structure
    unsigned int total_char = 0;
    int n = 0;
    char *headers_buffer = NULL;
    char *header_buffer = NULL;
    Header *header = response->headers;


    while (header < response->headers + response->header_count)
    {
        n = snprintf(NULL, 0, header_structure, header->name, header->value);
        if(n > 0)
            total_char += (unsigned int)n;

        headers_buffer = realloc(headers_buffer, total_char);
        header_buffer = malloc((unsigned int)n);

        sprintf(header_buffer, header_structure, header->name, header->value);
        strcat(header_buffer, header_buffer);
        free(header_buffer);

        header++;
    }


    // calculate response length
    long response_size = snprintf(NULL, 0, response_structure, response->response_code, response_code_info, date,
                                  response->content.content_type, response->content.content_length);

    // allocate memory and write response content in buffer
    char *response_content = malloc((unsigned long)response_size);
    sprintf(response_content, response_structure, response->response_code, response_code_info, date,
            response->content.content_type, response->content.content_length);

    if (headers_buffer != NULL && response->header_count > 0)
    {
        response_size = snprintf(NULL, 0, "%s\r\n%s", response_content, headers_buffer);
        response_content = realloc(response_content, (unsigned long)response_size);
        sprintf(response_content, "%s\r\n%s", response_content, headers_buffer);
    }


    /*
     * add content to response_content
     * or print CRLF only because it is needed by HTTP
     * HEAD request should have empty content but Content-Length
     */
    if (response->content.content != NULL && response->content.content_length > 0 && strcmp(HTTP_METHOD_HEAD, request->method) != 0)
    {
        response_size = snprintf(NULL, 0, "%s\r\n\r\n%s", response_content, response->content.content);
        response_content = realloc(response_content, (unsigned long)response_size);
        sprintf(response_content, "%s\r\n\r\n%s", response_content, response->content.content);
    }
    else
    {
        response_size = snprintf(NULL, 0, "%s\r\n\r\n", response_content);
        response_content = realloc(response_content, (unsigned long)response_size);
        sprintf(response_content, "%s\r\n\r\n", response_content);
    }

    // send response to client
    if (send(request->clientfd, response_content, (unsigned long)response_size, 0) == -1)
    {
        logger_error("http - send_response", "failed to send response to client");
    }
    else
    {
        logger_info("http - send_response", "response length : %d content:\n%s", response_size, response_content);
    }

    free(headers_buffer);
    free(response_content);
    http_free_request(request);
    http_free_response(response);
}

char *http_header_get(const char *name, Request *request) {
    Header *header = request->headers;

    while (header < request->headers + request->header_count && header->name != NULL) {
        if (strcmp(header->name, name) == 0)
            return header->value;
        header++; // increment iterator on table
    }
    return NULL;
}
