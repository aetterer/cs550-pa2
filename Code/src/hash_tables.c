#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash_tables.h"

// ------------------------------------------------------------------------- //
#define CLIENT_HT_SIZE 10
#define FILE_HT_SIZE 100

// ------------------------------------------------------------------------- //
struct client_info *client_ht[CLIENT_HT_SIZE];
struct file_info *file_ht[FILE_HT_SIZE];

// ------------------------------------------------------------------------- //
unsigned int hash(char *key, int table_size) {
    int length = strlen(key);
    unsigned int hash_value = 0;
    for (int i = 0; i < length; i++) {
        hash_value += key[i];
        hash_value  = (hash_value * key[i]) % table_size;
    }
    return hash_value;
}

// ------------------------------------------------------------------------- //
void init_client_ht() {
    for (int i = 0; i < CLIENT_HT_SIZE; i++) {
        client_ht[i] = NULL;
    }
}

// ------------------------------------------------------------------------- //
void init_file_ht() {
    for (int i = 0; i < FILE_HT_SIZE; i++) {
        file_ht[i] = NULL;
    }
}

// ------------------------------------------------------------------------- //
int client_ht_insert(struct client_info *c) {
    if (c == NULL) return -1;
    int index = hash(c->uid, CLIENT_HT_SIZE);
    for (int i = 0; i < CLIENT_HT_SIZE; i++) {
        int try = (i + index) % CLIENT_HT_SIZE;
        if (client_ht[try] == NULL) {
            client_ht[try] = c;
            return 0;
        }
    }
    return -1;
}

// ------------------------------------------------------------------------- //
int file_ht_insert(struct file_info *f) {
    if (f == NULL) return -1;
    int index = hash(f->filename, FILE_HT_SIZE);
    for (int i = 0; i < FILE_HT_SIZE; i++) {
        int try = (i + index) % FILE_HT_SIZE;
        if (file_ht[try] == NULL) {
            file_ht[try] = f;
            return 0;
        }
    }
    return -1;
}

// ------------------------------------------------------------------------- //
struct file_info * get_file_ht_entry(char *filename) {
    int index = hash(filename, FILE_HT_SIZE);
    for (int i = 0; i < FILE_HT_SIZE; i++) {
        int try = (i + index) % FILE_HT_SIZE;
        if (file_ht[try] != NULL && 
                strncmp(file_ht[try]->filename, filename, MAX_FN_LEN) == 0) {
            return file_ht[try];
        }
    }
    return NULL;
}

// ------------------------------------------------------------------------- //
void print_client_ht() {
    printf("CLIENT HT START\n");
    for (int i = 0; i < CLIENT_HT_SIZE; i++) {
        if (client_ht[i] == NULL) {
            printf("\t%i\t---\n", i);
        } else {
            printf("\t%i\t%s\n", i, client_ht[i]->uid);
            int n = client_ht[i]->num_files;
            for (int j = 0; j < n; j++) {
                printf("\t\t  -> %s\n", client_ht[i]->files[j]);
            }
        }
    }
    printf("CLIENT HT END\n");
}

// ------------------------------------------------------------------------- //
void print_file_ht() {
    printf("FILE HT START\n");
    for (int i = 0; i < FILE_HT_SIZE; i++) {
        if (file_ht[i] == NULL) {
            printf("\t%i\t---\n", i);
        } else {
            printf("\t%i\t%s\n", i, file_ht[i]->filename);
            int n = file_ht[i]->num_hosts;
            for (int j = 0; j < n; j++) {
                printf("\t\t  -> %s\n", file_ht[i]->uids[j]);
            }
        }
    }
    printf("FILE HT END\n");
}

// ------------------------------------------------------------------------- //
