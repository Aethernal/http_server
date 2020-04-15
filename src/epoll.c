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
int nextProcessFD[max_event];
static int epollfd;

clientEvents_t* clientEvents = NULL;
pthread_mutex_t* mutex = NULL;

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

        bool newUser = false;

        for (int i = 0; i < max_event; i++)
            nextProcessFD[i] = -1;

        for (n = 0; n < event_count; n++)
        {
            if (events[n].data.fd == STDIN)
                epoll_stdin_event(); //sync
            else if (events[n].data.fd == serverfd)
            {
                newUser = true;
                int client = epoll_server_event(n); //sync
                nextProcessFD[n] = client;
            }
            else
            {
                pthread_mutex_lock(mutex);

                int i = 0;
                while(clientEvents[i].status != Finish && i < max_client_event)
                    i++;
                if(i < max_client_event)
                {
                    clientEvents[i].pollEvent = events[n];
                    clientEvents[i].status = New;
                }
                pthread_mutex_unlock(mutex);
            }
        }
        if(newUser)
        {
            int pid = fork();
            if(pid  != 0)
            {
                epoll_slave();
                kill(pid, SIGKILL);
            }
        }
    }

    epoll_ctl(epollfd, EPOLL_CTL_DEL, serverfd, NULL);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, STDIN, NULL);

    // loop stopped -> closing
    close(epollfd);
}

void epoll_slave()
{
    while(*running)
    {
        pthread_mutex_lock(mutex);

        int i = 0;
        int j = 0;
        bool find = false;
        while(i < max_client_event && !find)
        {
            for (j = 0; j < max_event; j++)
            {
                if(clientEvents[i].pollEvent.data.fd == nextProcessFD[j])
                {
                    find = true;
                    break;
                }
            }
            if(!find)
                i++;
        }
        if(find)
        {
            clientEvents[i].status = InProgress;
            pthread_mutex_unlock(mutex);
            epoll_client_event(i);

            pthread_mutex_lock(mutex);
            nextProcessFD[j] = -1;
            clientEvents[i].status = Finish;
            pthread_mutex_unlock(mutex);
        }
        else
        {
            pthread_mutex_unlock(mutex);
        }

        find = false;
        for (j = 0; j < max_event; j++)
        {
            if(nextProcessFD[j] != -1)
            {
                find = true;
                break;
            }
        }
        if(!find)
        {
            return;
        }

        usleep(20000); //20ms
    }
}

void epoll_client_event(int eventIndex) {

    struct epoll_event event = clientEvents[eventIndex].pollEvent;

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

            const char *allowed_methods[5] = {HTTP_METHOD_GET, HTTP_METHOD_HEAD, HTTP_METHOD_POST, HTTP_METHOD_PUT, HTTP_METHOD_DELETE};
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

    return  client;
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
