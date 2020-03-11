# Cherokee

## HTTP 1.1 server implementation



### Group
- **aubin_f**
- maux_k
- velley_s
- yilmaz_i

***

### Presentation
Cherokee is an HTTP server implementation project supporting version 1.1.
It is an ETNA project.

cf [rfc2616](https://tools.ietf.org/html/rfc2616)
***

### Architecture

***

#### Handling multiples sockets => Epoll
We choose to use 'epoll' instead of 'select' to handle the file descriptors of users, it pretty much do the same thing but it is an optimized version of select with only listen to his file descriptors instead of having to iterate over all the actives files descriptors.

#### Managing concurrent requests
To be able to manage multiple client at the same time, it is needed to add a system to handle the multiple request, we through of using fork() but it will generate sub-process for each request, which is not resource friendly, so we through of using a worker pool using threads.

###### How it work
after parsing the request into a Request structure, we store it in a fifo list awaiting a thread from the pool to handle the request.

#### Managing services
When the request is handled, the function that will handle the service will be defined by the method and the uri of the request.

currently services are "hard coded" and chained until the 404 fallback is attained.

###### Service Order
* specific service matching uri/method
* index directory matching an existing path of the web root folder
* 404 response

###### Evolution :
  We could add a configuration file to allow custom routing to specific functions

***

### HTTP 1.1 Requirement

#### Supported methods
* GET
* HEAD
* POST
* PUT
* DELETE
#### Supported headers

#### HTTP Response code
- 100 Continue
- 101 Switching Protocols
- 200 OK
- 201 Created
- 202 Accepted
- 203 Non-Authoritative Information
- 204 No Content
- 205 Reset Content
- 206 Partial Content
- 300 Multiple Choices
- 301 Moved Permanently
- 302 Found
- 303 See Other
- 304 Not Modified
- 305 Use Proxy
- 306 (Unused)
- 307 Temporary Redirect
- 400 Bad Request
- 401 Unauthorized
- 402 Payment Required
- 403 Forbidden
- 404 Not Found
- 405 Method Not Allowed
- 406 Not Acceptable
- 407 Proxy Authentication Required
- 408 Request Timeout
- 409 Conflict
- 410 Gone
- 411 Length Required
- 412 Precondition Failed
- 413 Request Entity Too Large
- 414 Request-URI Too Long
- 415 Unsupported Media Type
- 416 Requested Range Not Satisfiable
- 417 Expectation Failed
- 500 Internal Server Error
- 501 Not Implemented
- 502 Bad Gateway
- 503 Service Unavailable
- 504 Gateway Timeout
- 505 HTTP Version Not Supported ( != 1.1 )

***

### Module index directory
Enable the user to navigate through specific server directory
***
