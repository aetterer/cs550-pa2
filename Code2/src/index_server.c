#include "common.h"
#include "network.h"
#include "read_config.h"
#include "thread_queue.h"
#include "hash_tables.h"
#include "constants.h"

// MACROS ------------------------------------------------------------------ //
#define USAGE "usage: ./index_server <config_file>"
#define BACKLOG 10

// GLOBAL VARIABLES AND STRUCTS -------------------------------------------- //
pthread_t thread_pool[MAX_PEER_NODES];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

// FUNCTION PROTOTYPES ----------------------------------------------------- //
void * thread_function(void *);
void * handle_connection(void *);
void register_client(int, char *);
void client_to_uid(char *, char *, char *);

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
    read_config(argv[1], NULL, port, wd, NULL);

    // Set up list of clients
    init_client_ht();
    init_file_ht();
    //thread_queue_t *tq = new_thread_queue();

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
        thread_enqueue(pclient);
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
        if ((pclient = thread_dequeue()) == NULL) {
            pthread_cond_wait(&cv, &mutex);
            pclient = thread_dequeue();
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
    int stat, len;
    free(pclient);

    char uid[UID_LEN];
    register_client(client_socket, uid);

    while (1) {
        // Receive command;
        recv_stat(client_socket, &stat);
        recv_len(client_socket, &len);
        char *cmd = malloc(len);
        memset(cmd, 0, len);
        recv_msg(client_socket, cmd, len);
        if (cmd == NULL) {
            printf("connection lost\n");
            return NULL;
        }
        printf("received command: %s\n", cmd);

        // Handle list command --------------------------------------------- //
        if (strncmp(cmd, "list", len) == 0) {
            // Send the number of files.
            int num_files = get_num_files();
            char num_files_str[12];
            sprintf(num_files_str, "%d", num_files);
            send_stat(client_socket, OK);
            send_len(client_socket, 12);
            send_msg(client_socket, num_files_str, 12);
            
            // Then send the files.
            int files_sent = 0;
            int ht_index = 0;
            while (files_sent < num_files) {
                struct file_info *f = get_file_ht_entry_by_index(ht_index);
                ht_index++;
                if (f == NULL) continue;
                send_stat(client_socket, OK);
                send_len(client_socket, MAX_FN_LEN);
                send_msg(client_socket, f->filename, MAX_FN_LEN);
                files_sent++;
            }
        }

        // Handle download command ----------------------------------------- //
        if (strncmp(cmd, "download", len) == 0) {
            // Receive the filename from the client;
            recv_stat(client_socket, &stat);
            recv_len(client_socket, &len);
            char *filename = malloc(len);
            recv_msg(client_socket, filename, len);

            // Get the file from the hash table and chose a host.
            struct file_info *f = get_file_ht_entry(filename);
            int r = rand() % f->num_hosts;
            struct client_info *c = get_client_ht_entry(f->uids[r]);

            send_stat(client_socket, OK);               //
            send_len(client_socket, MAX_IP_LEN);        //
            send_msg(client_socket, c->ip, MAX_IP_LEN); // --------------------- SEND E

            send_stat(client_socket, OK);                    //
            send_len(client_socket, MAX_PORT_LEN);           //
            send_msg(client_socket, c->sport, MAX_PORT_LEN); // ---------------- SEND F
            
            // Now receive the status of the file.
            recv_stat(client_socket, &stat);

            // Add the client as a host of the file.
            strncpy(f->uids[f->num_hosts], uid, UID_LEN);
            f->num_hosts = f->num_hosts + 1;

            print_file_ht();
        }
    }

    /*
    // Receive filename query.
    recv_stat(client_socket, &stat);          //
    recv_len(client_socket, &len);            //
    char *filename = malloc(len);             //
    recv_msg(client_socket, filename, len);   // ----------------------- RECV D

    trimnl(filename);

    struct file_info *f = get_file_ht_entry(filename);
    printf("found host with uid  %s\n", f->uids[0]);
    struct client_info *c = get_client_ht_entry(f->uids[0]);
    printf("host has ip %s and port %s\n", c->ip, c->sport);

    send_stat(client_socket, OK);               //
    send_len(client_socket, MAX_IP_LEN);        //
    send_msg(client_socket, c->ip, MAX_IP_LEN); // --------------------- SEND E

    send_stat(client_socket, OK);                    //
    send_len(client_socket, MAX_PORT_LEN);           //
    send_msg(client_socket, c->sport, MAX_PORT_LEN); // ---------------- SEND F
    */
    return NULL;
}
 // ------------------------------------------------------------------------ //
 // register_client() function
 //
void register_client(int client_socket, char *uid) {
    int stat;
    int len;

    // First, get the ip address and `connection' port of the client to make
    // a uid to key into the hash table.
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof addr;
    getpeername(client_socket, (struct sockaddr *)&addr, &addr_len);
    char *client_ip = inet_ntoa(addr.sin_addr);
    int ccp = ntohs(addr.sin_port);
    char client_cport[MAX_PORT_LEN];
    sprintf(client_cport, "%d", ccp);
    //char uid[UID_LEN]; 
    client_to_uid(client_ip, client_cport, uid);

    printf("client uid: %s\n", uid);

    // Then, receive the port the client will be listening on as a server.
    recv_stat(client_socket, &stat);              //
    recv_len(client_socket, &len);                //
    char *client_sport = malloc(len);             //
    recv_msg(client_socket, client_sport, len);   // ------------------- RECV A
    printf("received message: %s\n", client_sport);

    // Now create the client_info entry for the hash table and fill in the uid,
    // ip, cport, and sport fields.
    struct client_info *c = malloc(sizeof (struct client_info));
    strncpy(c->uid, uid, UID_LEN);
    strncpy(c->ip, client_ip, MAX_IP_LEN);
    strncpy(c->cport, client_cport, MAX_PORT_LEN);
    strncpy(c->sport, client_sport, MAX_PORT_LEN);

    // Receive the number of files.
    char *num_files_str;
    recv_stat(client_socket, &stat);               //
    recv_len(client_socket, &len);                 //
    num_files_str = malloc(len);                   //
    recv_msg(client_socket, num_files_str, len);   // ------------------ RECV B
    int num_files = atoi(num_files_str);
    free(num_files_str);

    // Set the num_files field and allocate memory for the list of files.
    c->num_files = num_files;
    c->files = malloc(num_files * MAX_FN_LEN * sizeof (char));

    // Now receive the client's list of files. For each file, we add it to the
    // hash table entry's list of files and create a file hash table entry and 
    // add it the table.
    char *filename;
    struct file_info *f;
    for (int i = 0; i < num_files; i++) {
        recv_stat(client_socket, &stat);          //
        recv_len(client_socket, &len);            //
        filename = malloc(len);                   //
        recv_msg(client_socket, filename, len);   // ------------------- RECV C
        
        strncpy(c->files[i], filename, MAX_FN_LEN);
        
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
    }
    client_ht_insert(c);
    //print_client_ht();
    //print_file_ht();
}

// ------------------------------------------------------------------------- //
// register_client() function
//
/*
void register_client(int client_socket) {
    // Get the IP address and port number of the client;
    struct sockaddr_in addr;
    socklen_t len = sizeof addr;
    getpeername(client_socket, (struct sockaddr *)&addr, &len);
    char *ip = inet_ntoa(addr.sin_addr);
    int cport = ntohs(addr.sin_port);
    char cport_str[MAX_PORT_LEN];
    sprintf(cport_str, "%d", cport);

    int stat;

    // Receive the client's listening port.
    char *sport_str = recv_packet(client_socket, &stat);

    //int port = ntohs(addr.sin_port);
    char uid[UID_LEN];
    client_to_uid(ip, cport, uid);

    // Get the number of files the client has.
    char *num_files_str = NULL;
    num_files_str = recv_packet(client_socket, &stat);
    int num_files = atoi(num_files_str);
    free(num_files_str);

    // Create a client_info struct.
    struct client_info *c = malloc(sizeof (struct client_info));
    strncpy(c->uid, uid, UID_LEN);
    strncpy(c->ip, ip, MAX_IP_LEN);
    strncpy(c->cport, cport_str, MAX_PORT_LEN);
    strncpy(c->sport, sport_str, MAX_PORT_LEN);
    c->num_files = num_files;
    c->files = malloc(num_files * MAX_FN_LEN * sizeof (char));

    // Add the files to the client_info struct.
    char *filename = NULL;
    for (int i = 0; i < num_files; i++) {
        filename = recv_packet(client_socket, &stat);
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
    }

    client_ht_insert(c);
    //print_client_ht();
    //print_file_ht();
}
*/

// ------------------------------------------------------------------------- //
// client_to_uid() function
//
void client_to_uid(char *ip, char *port, char *buf) {
    //char port_str[12];
    char *delimit = ":";
    //sprintf(port_str, "%d", port);
    strncpy(buf, ip, MAX_IP_LEN);
    strncat(buf, delimit, 1);
    strncat(buf, port, MAX_PORT_LEN);
}


