//
// Created by florian.aubin on 28/02/2020.
//

#include "main.h"
#include "epoll.h"
#include "logger.h"
#include "utils.h"
#include "http.h"
#include "services.h"

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

    /*
     * fork workers and process manager
     */

    int childs[max_worker] = {[0 ... (max_worker-1)] = 0};
    int status = 0;
    int pid;

    manager:
    for(int i = 0; i < max_worker; i++) {

        // slow down manager
        usleep(1000 * 1000);

        if(childs[i] != 0) {
            if(waitpid(childs[i], &status, WNOHANG)) {
                if (status != 0) {
                    logger_error("fork manager", "Worker[%d] with pid [%d] has stopped, restarting", i, childs[i]);
                    childs[i] = 0;
                }
            }
        } else {
            if ((pid = fork()) != 0) {
                logger_info("fork manager", "Worker[%d] process started [%d]", i, pid);
                childs[i] = pid;
            } else {
                goto worker;
            }
        }

    }

    // resume manager loop or exit
    if(*running) {
        goto manager;
    } else {
        // exit for manager to close server fd etc
        return;
    }

    // worker exit the manager loop and start their own
    worker:
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

    while (*running) {

        /*
         * wait for events, with a timeout of 300ms
         */
        event_count = epoll_wait(epollfd, events, max_event, 300);

        /*
         * server failure -> exit
         */
        if (event_count == -1) {
            logger_error("epoll - serve", "failed to wait for event");
            exit(0);
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
                epoll_stdin_event(); //sync
            else if (events[n].data.fd == serverfd) {
                epoll_server_event(n); //sync
            } else {
                epoll_client_event(n);
            }
        }
    }

    epoll_ctl(epollfd, EPOLL_CTL_DEL, serverfd, NULL);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, STDIN, NULL);

    // loop stopped -> closing
    close(epollfd);

    // exit for workers
    exit(0);
}

void epoll_client_event(int eventIndex) {

    struct epoll_event event = events[eventIndex];

    char *buffer[1024] = {[0 ... 1023] '\0'};
    int client = event.data.fd;
    uint32_t eventType = event.events;

    if ((eventType & EPOLLIN) != 0) {

        Request *request = http_parse_request(client);


        if (request != NULL) {

            if (strcmp(request->version, "HTTP/1.1") != 0) {

                Response *resp = http_create_response(client);
                resp->response_code = 505;
                http_send_response(request, resp);
                close(client);

                return;
            }

            const char *allowed_methods[5] = {HTTP_METHOD_GET, HTTP_METHOD_HEAD, HTTP_METHOD_POST, HTTP_METHOD_PUT,
                                              HTTP_METHOD_DELETE};
            int allowed_method = 0;

            for (int i = 0; i < 5; i++)
                if (strcmp(request->method, allowed_methods[i]) == 0)
                    allowed_method = 1;


            if (!allowed_method) {

                Response *resp = http_create_response(client);
                resp->response_code = 405;
                http_send_response(request, resp);
                close(client);

                return;
            }

            route(request);

            return;

        } else {
            close(client);
            return;
        }

    }

        /*
         * client disconnected, remove from epoll
         * cf close -> fd - fclose -> File from fopen()
         */
    else if ((eventType & (EPOLLERR | EPOLLRDHUP | EPOLLHUP)) != 0) {
        epoll_ctl(epollfd, EPOLL_CTL_DEL, client, NULL);
        close(client);
    } else {
        logger_warning("epoll - client_event", "unknown event");
    }

}

int epoll_server_event() {

    // sockaddr_in does not work (Invalid argument), we need to use sockadddr_un
    struct sockaddr_un client_addr;
    socklen_t client_addr_len;

    int client = accept(serverfd, &client_addr, &client_addr_len);

    if (client == -1) {
        logger_error("epoll - server_event", "failed to connect new client");
        return -1;
    }

    if (epoll_setnonblocking(client) == -1) {
        logger_error("epoll - serve", "failed to set client socket to non-blocking");
        return -1;
    }

    // EPOLLET for non blocking behavior
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = client;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client, &event) == -1) {
        logger_warning("epoll - server_event", "failed to add client listener to epoll");
        close(client);
        return -1;
    }

    return client;
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
        *running = 0;
    }

}
