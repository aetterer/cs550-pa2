#ifndef CLIENT_SERVER_H
#define CLIENT_SERVER_H

//#define MAX_IP_LEN 24
//#define MAX_PORT_LEN 24
//#define MAX_WD_LEN 24

int init_client(char *, char *);
int init_server(char *);
char * pack_msg(unsigned int, unsigned int, char *);
int send_all(int, char *, int);
int send_stat(int, int);
int send_len(int, int);
int send_msg(int, char *, int);
void send_packet(int, int, int, char *);
int recv_all(int, char *, int);
int recv_stat(int, int *);
int recv_len(int, int *);
int recv_msg(int, char *, int);
char * recv_packet(int, int *);
void trimnl(char *);

#endif
