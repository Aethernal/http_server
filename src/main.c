#include <stdio.h>
#include <netinet/in.h>

/* TODO

 * create socket server on selected port
 * create listening loop
 * fork on request reception
 * forked take the socket content to parsing
 * parent continue his loop
*/

/*
 * maximum number of concurrent client
 */
static const int max_client = 100;

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

    return 0;
}

/*
 * main request handler loop
 * dispatch to another threads
 */
void http_serve(const char *PORT) {
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

    /*
     * TODO Create socket and fill serverfd variable
     */

    while(running) {
        // TODO select() or fork() requests
    }


}