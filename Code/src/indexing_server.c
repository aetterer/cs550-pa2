#include "common.h"
#include "client_server.h"

#define PORT "20769"
#define BACKLOG 10

int main() {
    int listener_socket, client_socket; 

    listener_socket = init_server(PORT, BACKLOG);

    client_socket = accept(listener_socket, NULL, NULL);

    printf("Got connection\n");

    close(client_socket);
    close(listener_socket);
    return 0;
}
