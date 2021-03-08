#include "common.h"
#include "client_server.h"

#define IP "192.168.0.7"
#define PORT "20769"

int main() {
    int server_socket = init_client(IP, PORT);

    printf("Got connection\n");

    close(server_socket);
    return 0;
}
