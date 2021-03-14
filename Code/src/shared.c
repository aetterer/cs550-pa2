#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "shared.h"

// ------------------------------------------------------------------------- //
int init_client(char *ip, char *port) {
    struct addrinfo hints, *info;
    int server_socket;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(ip, port, &hints, &info);

    server_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    connect(server_socket, info->ai_addr, info->ai_addrlen);
    freeaddrinfo(info);

    return server_socket;
}

// ------------------------------------------------------------------------- //
int init_server(char *port, int backlog) {
    struct addrinfo hints, *info;
    int listener_socket;
    int yes = 1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, port, &hints, &info);

    listener_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
    setsockopt(listener_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
    bind(listener_socket, info->ai_addr, info->ai_addrlen);
    freeaddrinfo(info);

    listen(listener_socket, backlog);

    return listener_socket;
 }

// ------------------------------------------------------------------------- //
void trim(char *str) {
    int i = 0;
    char current = str[0];
    while (current !='\0') {
        i++;
        current = str[i];
    }
    str[i - 1] = '\0';
}

// ------------------------------------------------------------------------- //
void read_config(char *config, char *ip, char *port, char *wd) {
    FILE *fp = fopen(config, "r");
    
    if (ip != NULL) {
        fgets(ip, MAX_IP_LEN, fp);
        trim(ip);
    }
    if (port != NULL) {
        fgets(port, MAX_PORT_LEN, fp);
        trim(port);
    }
    if (wd != NULL) {
        fgets(wd, MAX_WD_LEN, fp);
        trim(wd);
    }

    fclose(fp);
}

// ------------------------------------------------------------------------- //

