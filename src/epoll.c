//
// Created by florian.aubin on 28/02/2020.
//

#include "main.h"
#include "epoll.h"
#include "logger.h"
#include "utils.h"

/*
 * structure to access dispatched event
 */
static struct epoll_event event, events[max_event];
static int epollfd;

void epoll_serve(const char *interface, const char *port) {

    int n, event_count = -1;

    main_bind_server_socket(interface, port);

    if (epoll_setnonblocking(serverfd) == -1) {
        logger_error("epoll - serve", "failed to set server socket to non-blocking");
        perror("nonblocking");
        return;
    }

    // create epoll and get fd for configuration
    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        logger_error("epoll - serve", "failed to create epoll");
        return;
    }

    // configure serverfd as input
    event.events = EPOLLIN;
    event.data.fd = serverfd;

    // add listener on serverfd to epoll
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serverfd, &event) == -1) {
        logger_error("epoll - serve", "failed to add server event listener");
        close(epollfd);
        return;
    }

    // configure stdin as input
    event.events = EPOLLIN;
    event.data.fd = STDIN;

    // add listener on STDIN to epoll
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN, &event) == -1) {
        logger_error("epoll - serve", "failed to add stdin event listener");
        close(epollfd);
        return;
    }

    running = 1;
    while (running) {

        /*
         * wait for events, with a timeout of 300ms
         */
        event_count = epoll_wait(epollfd, events, max_event, 300);

        /*
         * server failure -> exit
         */
        if (event_count == -1) {
            logger_error("epoll - serve", "failed to wait for event");
            exit(1);
        }

        /*
         * no event -> epoll_wait
         */
        if (event_count == 0)
            continue;

        /*
         * send each event to correct handler
         */
        for (n = 0; n < event_count; n++) {
            if (events[n].data.fd == STDIN)
                epoll_stdin_event();
            else if (events[n].data.fd == serverfd)
                epoll_server_event(n);
            else
                epoll_client_event(n);

        }

    }

    epoll_ctl(epollfd, EPOLL_CTL_DEL, serverfd, NULL);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, STDIN, NULL);

    // loop stopped -> closing
    close(epollfd);

}

void epoll_client_event(int eventIndex) {

    char* buffer [1024] = {[0 ... 1023] '\0'};
    int client = events[eventIndex].data.fd;
    int event = events[eventIndex].events;



    // TODO REMOVE
    const char headers [] = "HTTP/1.1 %d OK\n" \
				"date:%s\n" \
				"Accept-Ranges: \n" \
				"Content-type: text/html\n" \
				"Content-Length: %d" \
                "\n\n%s";

    const char content [] = "<HTML>\n" \
                                "\t<HEAD>\n" \
                                    "\t\t<TITLE>200 OK</TITLE>\n " \
                                "\t</HEAD>\n" \
                                "\t<BODY>\n" \
                                    "\t\t<H1>Welcome</H1>\n" \
                                "\t</BODY>\n" \
                            "</HTML>\n";
    // TODO REMOVE END

    char date [32] = {[0 ... 31] '\0'};
    char response [65565] = {[0 ... 65564] '\0'};
    char *http = NULL;

    if ((event & EPOLLIN) != 0) {

        memset(buffer, sizeof(buffer)-1, '\0');
        read(client, buffer, sizeof(buffer)-1);
        logger_content("epoll - client_event", "\n%s", buffer);

        // http request ?
        if (strstr(buffer, "HTTP/") != NULL) {

            current_date(&date);

            snprintf(response, sizeof(response) - 1, headers, 200, date,
                     strlen(content), content);

            logger_content("epoll - client_event", "\n%s", response);

            write(client, response, strlen(response));
            close(client);
        }

    }

    /*
     * client disconnected, remove from epoll
     * cf close -> fd - fclose -> File from fopen()
     */
    else if ((event & (EPOLLERR | EPOLLRDHUP | EPOLLHUP)) != 0) {
        epoll_ctl(epollfd, EPOLL_CTL_DEL, client, NULL);
        close(client);
    } else {
        logger_warning("epoll - client_event", "unknown event");
    }

}

void epoll_server_event() {

    // sockaddr_in does not work (Invalid argument), we need to use sockadddr_un
    struct sockaddr_un client_addr;
    socklen_t client_addr_len;

    int client = accept(serverfd, &client_addr, &client_addr_len);
    if(client == -1) {
        logger_error("epoll - server_event", "failed to connect new client");
        return;
    }

    if (epoll_setnonblocking(client) == -1) {
        logger_error("epoll - serve", "failed to set client socket to non-blocking");
        return;
    }

    // EPOLLET for non blocking behavior
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = client;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client, &event) == -1) {
        logger_warning("epoll - server_event", "failed to add client listener to epoll");
        close(client);
        return;
    }

}

int epoll_setnonblocking(int fd) {

    int flags = 0;

    // get current flags value
    if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
        flags = 0;

    // update with old flags + o_nonblock
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);

}

void epoll_stdin_event() {

    char *cmd = readline();

    logger_info("epoll - stdin_event", "terminal input [%s]", cmd);

    // read failed, log already handled in readline
    if (cmd == NULL)
        return;

    // stop server
    if (strcmp(cmd, "exit") == 0) {
        logger_warning("epoll - stdin_event", "stopping server");
        running = 0;
    }

}