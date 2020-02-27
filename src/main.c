#include "main.h"
#include "logger.h"
#include "utils.h"

/*
 * server and clients files descriptors
 */
static int serverfd, clients[max_client];

/*
 * define if the accept loop shall continue or stop
 */
static int running = 0;

/*
 * parse parameters
 * default port 80
 */
int main(int argc, char **argv) {

    // init logger
    logger_init("server.log");

    // check for param -> port
    char *port = "80";
    if (argc > 1) {
        char *message = malloc(1024);
        sprintf(message, "using port [%s]", argv[1]);
        logger_success("main - main", message);
        free(message);
        port = argv[1];
    } else {
        logger_warning("main - main", "using port [80]");
    }

    // start http server
    http_serve(port);

    // TODO handle server close (free everything correctly) / wait for last requests or kill them ?

    return 0;
}

void http_serve(const char *port) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    int slot = 0;


    /*
     * default value for clients files descriptors
     * free = -1
    */
    int i;
    for (i = 0; i < max_client; i++)
        clients[i] = -1;

    bind_server_socket(port);

    // list of descriptors the select shall handle
    fd_set fds;

    running = 1;
    while(running) {

        FD_ZERO(&fds);

        // to read stdin input
        FD_SET(STDIN, &fds);

        // to accept new request
        FD_SET(serverfd, &fds);

        // to handle current requests
        for(i=0; i < max_client; i++)
            if(clients[i] != -1)
                FD_SET(clients[i], &fds);

        select(FD_SETSIZE, &fds, NULL, NULL, NULL);

        if (FD_ISSET(serverfd, &fds)) {
            computeServerEvent();
        } else if (FD_ISSET(STDIN, &fds)) {
            computeSTDInput();
        } else {
            for (i = 0; i < max_client; i++)
                if (FD_ISSET(clients[i], &fds)) {
                    computeClientEvent(clients[i]);
                    break;
                }
        }

    }

}

void computeServerEvent() {

}

void computeClientEvent(int client) {}

void computeSTDInput() {
    char* cmd = readline();

    // read failed, log already handled in readline
    if(cmd == NULL)
        return;

    // stop server
    if(strcmp(cmd,"exit") == 0) {
        logger_warning("main - computeSTDInput", "stopping server");
        running = 0;
    }

}

void bind_server_socket(const char *port) {
    struct addrinfo socket_options, *addrs, *addr;

    logger_warning("main - bind_server_socket", "starting server");

    /*
     * get bindable socket
     * http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
     * ai_flags = AI_PASSIVE  allow to bind the socket to accept requests
     */
    memset(&socket_options, 0, sizeof(socket_options));
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

    /*
     * for iteration on sockets linked list elements
     * bind sockets to port
     */
    for(addr = addrs; addr != NULL ; addr=addrs->ai_next) {
        int option = 1;
        serverfd = socket(addr->ai_family, addr->ai_socktype, 0); // default protocol
        setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)); // allow ot reuse same port
        if (serverfd == -1) continue; // failed to create socket
        if (bind(serverfd, addr->ai_addr, addr->ai_addrlen) == 0) break; // success
    }

    // check if we could bind a socket
    if (addr == NULL) {
        logger_error("main - bind_server_socket", "failed to bind port");
        exit(1);
    }

    // free addrs linked list
    freeaddrinfo(addrs);

    // start listening for request
    if ( listen (serverfd, max_pending_connection) != 0 ) {
        logger_error("main - bind_server_socket", "failed to listen on serverfd");
        exit(1);
    }

    logger_success("main - bind_server_socket", "server started");

}