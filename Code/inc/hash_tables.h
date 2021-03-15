
#ifndef HASH_TABLES_H
#define HASH_TABLES_H

#include "constants.h"

struct client_info {
    char uid[UID_LEN];
    int num_files;
    char (*files)[MAX_FN_LEN];
};

struct file_info {
    char filename[MAX_FN_LEN];
};

void init_client_ht();
void init_file_ht();
int client_ht_insert(struct client_info *);
int file_ht_insert(struct file_info *);
void print_client_ht();
void print_file_ht();

#endif
