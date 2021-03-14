#include "common.h"
#include "client_server.h"
#include "read_config.h"
#include "thread_queue.h"
#include "constants.h"

// MACROS ------------------------------------------------------------------ //
#define USAGE "usage: ./index_server <config_file>"
#define BACKLOG 10
#define MAX_CLIENTS 10

// GLOBAL VARIABLES AND STRUCTS -------------------------------------------- //
pthread_t thread_pool[MAX_CLIENTS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

struct client_info {
    char ip_addr[MAX_IP_LEN];
    char port[MAX_PORT_LEN];
};

// FUNCTION PROTOTYPES ----------------------------------------------------- //
void * thread_function(void *);
void * handle_connection(void *);
void register_client(int);

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
    //struct client_info *clients = (struct client_info *)malloc(MAX_CLIENTS * sizeof clients);

    // Initialize the node as a server.
    listener_socket = init_server(port, BACKLOG);

    // Set up threads.
    for (int i = 0; i < MAX_CLIENTS; i++) {
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);
    }

    printf("waiting for connection...\n");

    while (1) {
        // Accept a connection from a client.
        client_socket = accept(listener_socket, NULL, NULL);

        // Allocate memory for the client socket for the queue.
        int *pclient = malloc(sizeof pclient);
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

// handle_connection() function
//      
void * handle_connection(void *pclient) {
    int client_socket = *(int *)pclient;
    free(pclient);
    printf("handling connection for %d\n", client_socket);

    register_client(client_socket);

    //while (1);
    return NULL;
}

// register_client() function
//
void register_client(int client_socket) {
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    getpeername(client_socket, (struct sockaddr *)&addr, &len);
    char *ip = inet_ntoa(addr.sin_addr);
    int port = ntohs(addr.sin_port);

    printf("registering client %s:%d\n", ip, port);

    printf("files list:\n");
    char *file_list = NULL;
    file_list = recv_packet(client_socket);
    printf("%s\n", file_list);
}
