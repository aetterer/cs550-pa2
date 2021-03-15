#include "common.h"
#include "network.h"
#include "read_config.h"
#include "thread_queue.h"
#include "hash_tables.h"
#include "constants.h"

// MACROS ------------------------------------------------------------------ //
#define USAGE "usage: ./index_server <config_file>"
#define BACKLOG 10
//#define MAX_PEER_NODES 10

// GLOBAL VARIABLES AND STRUCTS -------------------------------------------- //
pthread_t thread_pool[MAX_PEER_NODES];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

//struct client_info {
//    char ip_addr[MAX_IP_LEN];
//    char port[MAX_PORT_LEN];
//};

//struct list_node {
//    char *filename;
//    struct client_info client;
//};

// FUNCTION PROTOTYPES ----------------------------------------------------- //
void * thread_function(void *);
void * handle_connection(void *);
void register_client(int);
void client_to_uid(char *, int, char *);

// FUNCTIONS --------------------------------------------------------------- //
int main(int argc, char **argv) {
    char port[MAX_PORT_LEN];
    char wd[MAX_WD_LEN];
    int listener_socket, client_socket; 

    if (argc != 2) {
        printf("%s\n", USAGE);
        exit(1);
    }

    // Get server parameters from config file.
    read_config(argv[1], NULL, port, wd);

    // Set up list of clients.
    //struct client_info *clients = (struct client_info *)malloc(MAX_PEER_NODES * sizeof clients);
    init_client_ht();
    init_file_ht();

    // Initialize the node as a server.
    listener_socket = init_server(port, BACKLOG);

    // Set up threads.
    for (int i = 0; i < MAX_PEER_NODES; i++) {
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);
    }

    printf("waiting for connection...\n");

    while (1) {
        // Accept a connection from a client.
        client_socket = accept(listener_socket, NULL, NULL);

        // Allocate memory for the client socket for the queue.
        int *pclient = malloc(sizeof (int));
        *pclient = client_socket;

        // Enqueue the client socket so a thread can start handling the connection.
        pthread_mutex_lock(&mutex);
        enqueue(pclient);
        pthread_cond_signal(&cv);
        pthread_mutex_unlock(&mutex);

        //close(client_socket);
    }

    //free(clients);
    close(listener_socket);
    return 0;
}

// ------------------------------------------------------------------------- //
// thread_function() function
//      Tries to dequeue from the thread queue. If the queue is empty, 
//      then wait for a something to be enqueued. If the queue isn't empty,
//      then call the handle_connection() function thread with the dequeued
//      socket.
void * thread_function(void *arg) {
    while (1) {
        int *pclient;

        pthread_mutex_lock(&mutex);
        if ((pclient = dequeue()) == NULL) {
            pthread_cond_wait(&cv, &mutex);
            pclient = dequeue();
        }
        pthread_mutex_unlock(&mutex);

        if (pclient != NULL) {
            handle_connection(pclient);
        }
    }
    return NULL;
}

// ------------------------------------------------------------------------- //
// handle_connection() function
//      
void * handle_connection(void *pclient) {
    int client_socket = *(int *)pclient;
    free(pclient);
    printf("handling connection for %d\n", client_socket);

    register_client(client_socket);

    char *fn = NULL;
    fn = recv_packet(client_socket);
    fflush(stdout);
    printf("%s\n", fn);

    struct file_info *f = get_file_ht_entry(fn);
    char uid[UID_LEN];
    strncpy(uid, f->uids[0], UID_LEN);

    send_packet(client_socket, OK, UID_LEN, uid);

    return NULL;
}

// ------------------------------------------------------------------------- //
// register_client() function
//
void register_client(int client_socket) {
    // Get the IP address and port number of the client;
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    getpeername(client_socket, (struct sockaddr *)&addr, &len);
    char *ip = inet_ntoa(addr.sin_addr);

    char *port_str = recv_packet(client_socket);
    int port = atoi(port_str);
    free(port_str);

    //int port = ntohs(addr.sin_port);
    char uid[UID_LEN];
    client_to_uid(ip, port, uid);


    // Get the number of files the client has.
    char *num_files_str = NULL;
    num_files_str = recv_packet(client_socket);
    int num_files = atoi(num_files_str);
    free(num_files_str);

    // Create a client_info struct.
    struct client_info *c = malloc(sizeof (struct client_info));
    strncpy(c->uid, uid, UID_LEN);
    c->num_files = num_files;
    c->files = malloc(num_files * MAX_FN_LEN * sizeof (char));

    // Add the files to the client_info struct.
    char *filename = NULL;
    for (int i = 0; i < num_files; i++) {
        filename = recv_packet(client_socket);
        strncpy(c->files[i], filename, MAX_FN_LEN);
        
        struct file_info *f;
        if ((f = get_file_ht_entry(filename)) == NULL) {
            // This file isn't hosted by any nodes.
            // Create a new entry in the hash table.
            f = malloc(sizeof (struct file_info));
            strncpy(f->filename, filename, MAX_FN_LEN);
            f->num_hosts = 1;
            strncpy(f->uids[0], uid, UID_LEN);
            file_ht_insert(f);
        } else {
            // This file already exists in the table.
            // Add the node's uid to the array.
            strncpy(f->uids[f->num_hosts], uid, UID_LEN);
            f->num_hosts = f->num_hosts + 1;
        }
        //printf("<%s>\n", filename);
    }

    client_ht_insert(c);
    //print_client_ht();
    //print_file_ht();
}

// ------------------------------------------------------------------------- //
// client_to_uid() function
//
void client_to_uid(char *ip, int port, char *buf) {
    char port_str[12];
    char *delimit = ":";
    sprintf(port_str, "%d", port);
    strncpy(buf, ip, MAX_IP_LEN);
    strncat(buf, delimit, 1);
    strncat(buf, port_str, 12);
}


