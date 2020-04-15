#include "main.h"
#include "logger.h"
#include "utils.h"


/*
 * containers for file descriptors
 */
int serverfd = -1;

int *running = NULL;

/*
 * parse parameters
 * default port 80
 */

int main(int argc, char **argv) {

    // parameters //
    int logger = 0;
    char* port = "25565";
    char* logger_file = "out.log";
    char* default_directory = ".";
    char* interface = "0.0.0.0";

    // parameters iterator
    int opt;

    while( (opt = getopt(argc, argv, "o:d:p:l")) != -1 ) {
        switch (opt) {
            case 'o':
                logger_file = optarg;
                break;
            case 'd':
                default_directory = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            case 'l':
                logger = 1;
                break;
        }
    }

    // init logger
    if(logger) {
        logger_init(logger_file);
    } else {
        logger_info("main","printing logs to STDOUT");
    }


    // start http server
    main_http_serve(interface, port);

    if (serverfd != -1)
        close(serverfd);

    logger_success("main", "server stopped correctly");

    return 0;
}

void main_http_serve(const char *interface, const char *port)
{
    clientEvents = mmap(NULL, sizeof(clientEvents) * max_client_event, PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    for(int i = 0; i < max_client_event; i++)
    {
        clientEvents[i].status = Finish;
    }

    running = mmap(NULL, sizeof(running), PROT_READ | PROT_WRITE,
               MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *running = 1;

    mutex = mmap(NULL, sizeof(mutex), PROT_READ | PROT_WRITE,
                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &attr);

    epoll_serve(interface, port);

    munmap(clientEvents, sizeof(clientEvents) * max_client_event);
    munmap(running, sizeof(running));
    munmap(mutex, sizeof(mutex));
}

void main_bind_server_socket(const char *interface, const char *port) {

    struct addrinfo socket_options = {}, *addrs, *addr;

    logger_info("main - bind_server_socket", "starting server");

    /*
     * get bindable socket
     * http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
     * ai_flags = AI_PASSIVE  allow to bind the socket to accept requests
     */
    socket_options.ai_family = AF_INET;
    socket_options.ai_socktype = SOCK_STREAM;
    socket_options.ai_flags = AI_PASSIVE;

    /*
     * fill addrs with all available ipv4 addr
     */
    if (getaddrinfo(NULL, port, &socket_options, &addrs) != 0) {
        logger_error("main - bind_server_socket", "failed to get addrs");
        exit(1);
    }

    logger_info("main - bind_server_socket", "binding to [%s]", port);

    /*
     * for iteration on sockets linked list elements
     * bind sockets to port
     */
    for (addr = addrs; addr != NULL; addr = addrs->ai_next) {
        int option = 1; // boolean value to enable SO_REUSEADDR

        serverfd = socket(addr->ai_family, addr->ai_socktype, 0); // default protocol

        setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)); // allow to reuse same port
        if (serverfd == -1)
            continue; // failed to create socket
        if (bind(serverfd, addr->ai_addr, addr->ai_addrlen) == 0)
            break; // success
    }

    // check if we could bind a socket
    if (addr == NULL) {
        logger_error("main - bind_server_socket", "failed to bind port");
        exit(1);
    } else {
        logger_success("main - bind_server_socket", "binded to [%s][%s]", inet_ntoa(*((struct in_addr*)addr)), port);
    }

    // free addrs linked list
    freeaddrinfo(addrs);

    // start listening for request
    if (listen(serverfd, max_pending_connection) != 0) {
        logger_error("main - bind_server_socket", "failed to listen on serverfd");
        exit(1);
    }

    logger_success("main - bind_server_socket", "server started");

}
