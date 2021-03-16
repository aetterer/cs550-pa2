
#ifndef HASH_TABLES_H
#define HASH_TABLES_H

#include "constants.h"

struct client_info {
    char uid[UID_LEN];
    char ip[MAX_IP_LEN];
    char cport[MAX_PORT_LEN];
    char sport[MAX_PORT_LEN];
    int num_files;
    char (*files)[MAX_FN_LEN];
};

struct file_info {
    char filename[MAX_FN_LEN];
    int num_hosts;
    char uids[MAX_PEER_NODES][UID_LEN];
};

void init_client_ht();
void init_file_ht();
int client_ht_insert(struct client_info *);
int file_ht_insert(struct file_info *);
struct file_info * get_file_ht_entry(char *);
struct file_info * get_file_ht_entry_by_index(int);
struct client_info * get_client_ht_entry(char *);
void print_client_ht();
void print_file_ht();
int get_num_files(void);

#endif
