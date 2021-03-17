#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "network.h"
#include "constants.h"

// ------------------------------------------------------------------------- //
int init_client(char *ip, char *port) {
    struct addrinfo hints, *info;
    int server_socket;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(ip, port, &hints, &info);

    if ((server_socket = 
                socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == -1) {
        perror("init_client() socket");
    }
    if (connect(server_socket, info->ai_addr, info->ai_addrlen) == -1) {
        perror("connect");
    }
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

    if ((listener_socket = 
                socket(info->ai_family, info->ai_socktype, info->ai_protocol)) == -1) {
        perror("init_server() socket");
    }
    setsockopt(listener_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
    if (bind(listener_socket, info->ai_addr, info->ai_addrlen) == -1) {
        perror("bind");
    }
    freeaddrinfo(info);

    if (listen(listener_socket, backlog) == -1) {
        perror("listen");
    }

    return listener_socket;
 }

// ------------------------------------------------------------------------- //
char * pack_msg(unsigned int stat, unsigned int len, char *msg) {
    unsigned int total_msg_len = sizeof (char) + sizeof (int) + len;
    char *buf = (char *)malloc(total_msg_len * sizeof (char));

    memcpy(buf, (char *)&stat, 4);
    memcpy(buf, (char *)&len, 4);
    strncpy(buf, msg, len);

    return buf;
}

// ------------------------------------------------------------------------- //
// send_all() function
//     source: https://beej.us/guide/bgnet/html/#cb22-36
int send_all(int socket, char *buf, int len) {
    int total = 0;
    int bytes_left = len;
    int n;

    while (total < len) {
        n = send(socket, buf + total, bytes_left, 0);
        if (n == -1)
            break;
        total += n;
        bytes_left -= n;
    }

    //len = total;
    return n == -1?-1:0;
}

int send_stat(int socket, int stat) {
    stat = htonl(stat);
    char stat_as_bytes[MSG_STAT_LEN];
    memcpy(stat_as_bytes, (char *)&stat, MSG_STAT_LEN);
    int ret = send_all(socket, stat_as_bytes, MSG_STAT_LEN);
    return ret;
}

int send_len(int socket, int len) {
    len = htonl(len);
    char len_as_bytes[MSG_LEN_LEN];
    memcpy(len_as_bytes, (char *)&len, MSG_LEN_LEN);
    int ret = send_all(socket, len_as_bytes, MSG_LEN_LEN);
    return ret;
}

int send_msg(int socket, char *msg, int len) {
    int ret = send_all(socket, msg, len);
    return ret;
}

// ------------------------------------------------------------------------- //
void send_packet(int socket, int stat, int len, char *msg) {
    // First, send the status.
    stat = htonl(stat);
    char stat_as_bytes[MSG_STAT_LEN];
    memcpy(stat_as_bytes, (char *)&stat, MSG_STAT_LEN);
    send_all(socket, stat_as_bytes, MSG_STAT_LEN);

    // Then send the length of the message.
    int nlen = htonl(len);
    char len_as_bytes[MSG_LEN_LEN];
    memcpy(len_as_bytes, (char *)&nlen, MSG_LEN_LEN);
    send_all(socket, len_as_bytes, MSG_LEN_LEN);

    // Finally, send the message.
    send_all(socket, msg, len);

}

// ------------------------------------------------------------------------- //
int recv_all(int socket, char *buf, int len) {
    int total = 0;
    int bytes_left = len;
    int n;

    while (total < len) {
        n = recv(socket, buf + total, bytes_left, 0);
        if (n == 0) 
            return 0;
        if (n == -1)
            break;
        total += n;
        bytes_left -= n;
    }

    //len = total;
    return n == -1?-1:0;
}

int recv_stat(int socket, int *stat) {
    char stat_as_bytes[MSG_STAT_LEN];
    int ret = recv_all(socket, stat_as_bytes, MSG_STAT_LEN);
    int s;
    memcpy(&s, (int *)stat_as_bytes, MSG_STAT_LEN);
    s = ntohl(s);
    *stat = s;
    return ret;
}

int recv_len(int socket, int *len) {
    char len_as_bytes[MSG_LEN_LEN];
    int ret = recv_all(socket, len_as_bytes, MSG_LEN_LEN);
    int l;
    memcpy(&l, (int *)len_as_bytes, MSG_LEN_LEN);
    l = ntohl(l);
    *len = l;
    return ret;
}

int recv_msg(int socket, char *msg, int len) {
    int ret = recv_all(socket, msg, len);
    return ret;
}

// ------------------------------------------------------------------------- //
char * recv_packet(int socket, int *stat) {
    //printf("\t### recv_packet() debug ###\n");
    // First, handle the status.
    char stat_as_bytes[MSG_STAT_LEN];
    recv_all(socket, stat_as_bytes, MSG_STAT_LEN);
    int s;
    memcpy(&s, (int *)stat_as_bytes, MSG_STAT_LEN);
    s = ntohl(s);
    *stat = s;
    //printf("\tmsg stat: %d\n", stat);

    // Then handle the length of the message.
    char len_as_bytes[MSG_LEN_LEN];
    recv_all(socket, len_as_bytes, MSG_LEN_LEN);
    int len;
    memcpy(&len, (int *)len_as_bytes, MSG_LEN_LEN);
    len = ntohl(len);
    //printf("\tmsg len: %d\n", len);

    // Finally, receive the message.
    char *msg = (char *)malloc(len * sizeof (char));
    memset(msg, 0, len);
    recv_all(socket, msg, len);
    //printf("\tmsg: %s\n", msg);
    //printf("\t###########################\n");
    return msg;
}

// ------------------------------------------------------------------------- //
void trimnl(char *s) {
    int i = 0;
    while (s[i] != '\n') {
        i++;
    }
    if (s[i] == '\n')
        s[i] = '\0';
}





