#ifndef SHARED_H
#define SHARED_H

#define MAX_IP_LEN 24
#define MAX_PORT_LEN 24
#define MAX_WD_LEN 24

int init_client(char *, char *);
int init_server(char *, int);
void read_config(char *, char *, char *, char *);

#endif
