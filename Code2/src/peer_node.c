#include "common.h"
#include "network.h"
#include "read_config.h"
#include "constants.h"
#include "thread_queue.h"
#include "cmd_queue.h"

// MACROS ------------------------------------------------------------------ //
#define USAGE "usage: ./peer_node <config_file> <server_ip> <server_port>"
#define CONFIG_FILE "peernode.cfg"
#define MAX_CLIENTS 10

// GLOBAL VARIABLES AND STRUCTS -------------------------------------------- //
pthread_t thread_pool[MAX_CLIENTS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
char logfile[MAX_WD_LEN];
char wd[MAX_WD_LEN];

// FUNCTION PROTOTYPES ----------------------------------------------------- //
void register_with_index(int, char *);
void client_to_uid(char *, int, char *);
void * thread_function(void *);
void * handle_connection(void *);
void write_to_log(char *);

// ------------------------------------------------------------------------- //
// main() function
//
int main(int argc, char **argv) {
    // Get parameters from config file.
    char port_str[MAX_PORT_LEN];

    if (argc != 4) {
        printf("%s\n", USAGE);
        exit(1);
    }

    read_config(argv[1], NULL, port_str, wd, NULL);
    //printf("log file: %s\n", logfile);


    // Change the directory.
    chdir(wd);

    // Initialize the node as a client of the indexing server.
    int index_socket = init_client(argv[2], argv[3]);
    printf("client: connected to %d\n", index_socket);

    register_with_index(index_socket, port_str);

    // SERVER SECTION ------------------------------------------------------ //
    if (fork() == 0) {
        int listener_socket, client_socket;

        listener_socket = init_server(port_str, 10);
       

        for (int i = 0; i < MAX_CLIENTS; i++) {
            pthread_create(&thread_pool[i], NULL, thread_function, NULL);
        }

        //printf("server: waiting for connection...\n");
        while (1) {
            client_socket = accept(listener_socket, NULL, NULL);
        
            // Get the client's info.
            //struct sockaddr_in addr;
            //socklen_t addr_len = sizeof addr;
            //getpeername(client_socket, (struct sockaddr *)&addr, &addr_len);
            //char *client_ip = inet_ntoa(addr.sin_addr);
            //int client_port = ntohs(addr.sin_port);
            
            //char *logmsg = NULL;
            //sprintf(logmsg, "Server: Received connection from %s:%d", client_ip, client_port);
            //write_to_log("Server: received connection.");
            
            pthread_mutex_lock(&mutex);
            FILE *fp = fopen(logfile, "a");
            char l[100] = "Server: what the actually fuck\n";
            //fprintf(fp, "Server: received connection\n");
            fwrite(l, 100, 1, fp);
            fclose(fp);
            pthread_mutex_unlock(&mutex);

            int *pclient = malloc(sizeof (int));
            *pclient = client_socket;

            pthread_mutex_lock(&mutex);
            thread_enqueue(pclient);
            pthread_cond_signal(&cv);
            pthread_mutex_unlock(&mutex);
        }
        return 0;
    }

    // CLIENT SECTION ------------------------------------------------------ //
    struct timeval start, end;
    double exec_time;
    int stat, len;
    char full_cmd[MAX_CMD_LEN];
    while (1) {
        // Get user input.
        printf("> ");
        fgets(full_cmd, MAX_CMD_LEN, stdin);
        if (full_cmd[0] == '\n') continue;
        trimnl(full_cmd);

        flush_cmd_queue();
        cmd_queue_from_string(full_cmd);

        // Get the command part of the command 
        char cmd_1[MAX_CMD_LEN];
        cmd_dequeue(cmd_1);

        printf("dequeued %s\n", cmd_1);

        if (strncmp(cmd_1, "list", MAX_CMD_LEN) != 0 &&
            strncmp(cmd_1, "download", MAX_CMD_LEN) != 0 &&
            strncmp(cmd_1, "quit", MAX_CMD_LEN) != 0) {
            continue;
        }


        // handle list command --------------------------------------------- //
        if (strncmp(cmd_1, "list", MAX_CMD_LEN) == 0) {
            printf("FILE LIST\n============\n");
            // Start the timer.
            exec_time = 0;
            gettimeofday(&start, NULL);

            // Send the command.
            send_stat(index_socket, OK);
            send_len(index_socket, MAX_CMD_LEN);
            send_msg(index_socket, cmd_1, MAX_CMD_LEN);

            // Receive the number of files.
            recv_stat(index_socket, &stat);
            recv_len(index_socket, &len);
            char *num_files_str = malloc(len);
            recv_msg(index_socket, num_files_str, len);
            int num_files = atoi(num_files_str);
            free(num_files_str);

            // Then receive each filename.
            for (int i = 0; i < num_files; i++) {
                recv_stat(index_socket, &stat);
                recv_len(index_socket, &len);
                char *filename = malloc(len);
                recv_msg(index_socket, filename, len);
                printf("%s\n", filename);
                free(filename);
            }

            // Stop the timer an calculate the request time.
            gettimeofday(&end, NULL);
            exec_time = (double) (end.tv_sec - start.tv_sec) 
                + (double) (end.tv_usec - start.tv_usec) / 1000;
            printf("============\n");
            printf("%lf ms\n", exec_time);
        }

        // handle download command ----------------------------------------- //
        if (strncmp(cmd_1, "download", MAX_CMD_LEN) == 0) {
            // Start the timer.
            exec_time = 0;
            gettimeofday(&start, NULL);

            // Send the command.
            send_stat(index_socket, OK);
            send_len(index_socket, MAX_CMD_LEN);
            send_msg(index_socket, cmd_1, MAX_CMD_LEN);

            // Dequeue the filename.
            char filename[MAX_FN_LEN];
            cmd_dequeue(filename);

            // Send the filename to the indexing server.
            send_stat(index_socket, OK);
            send_stat(index_socket, MAX_FN_LEN);
            send_msg(index_socket, filename, MAX_FN_LEN);

            // Receive the IP address and port of the the host server.
            recv_stat(index_socket, &stat);
            recv_len(index_socket, &len);
            char *host_ip = malloc(len);
            recv_msg(index_socket, host_ip, len);
            
            recv_stat(index_socket, &stat);
            recv_len(index_socket, &len);
            char *host_port = malloc(len);
            recv_msg(index_socket, host_port, len);

            // Then connect to that host.
            int host_socket = init_client(host_ip, host_port);

            // Send the filename.
            send_stat(host_socket, OK);
            send_len(host_socket, MAX_FN_LEN);
            send_msg(host_socket, filename, MAX_FN_LEN);

            // Then receive the file into a buffer.
            recv_stat(host_socket, &stat);
            recv_len(host_socket, &len);
            char *f_buf = malloc(len);
            recv_msg(host_socket, f_buf, len);

            // And write it into a file.
            FILE *fp = fopen(filename, "w");
            fwrite(f_buf, len, 1, fp);
            fclose(fp);

            // Let the indexing server know that the file was received.
            send_stat(index_socket, OK);

            // Stop the timer an calculate the request time.
            gettimeofday(&end, NULL);
            exec_time = (double) (end.tv_sec - start.tv_sec) 
                + (double) (end.tv_usec - start.tv_usec) / 1000;
            printf("%lf ms\n", exec_time);

            free(f_buf);
            free(host_ip);
            free(host_port);

        }

        // Handle quit command --------------------------------------------- //
        if (strncmp(cmd_1, "quit", MAX_CMD_LEN) == 0) {
            // Send the command.
            send_stat(index_socket, OK);
            send_len(index_socket, MAX_CMD_LEN);
            send_msg(index_socket, cmd_1, MAX_CMD_LEN);
            close(index_socket);
            return 0;
        }

        /*
        // Send a filename to the indexing server.
        send_stat(index_socket, OK);               //
        send_len(index_socket, MAX_FN_LEN);        //
        send_msg(index_socket, buf, MAX_FN_LEN);   // ------------------ SEND D

        char *host_ip;
        char *host_port;

        recv_stat(index_socket, &stat);       //
        recv_len(index_socket, &len);         //
        host_ip = malloc(len);                //
        recv_msg(index_socket, host_ip, len); // ----------------------- RECV E

        recv_stat(index_socket, &stat);         //
        recv_len(index_socket, &len);           //
        host_port = malloc(len);                //
        recv_msg(index_socket, host_port, len); // --------------------- RECV F

        printf("%s is on %s (%s)\n", buf, host_ip, host_port);

        int host_socket = init_client(host_ip, host_port);
        char *msg = "hello host";
        send_stat(host_socket, OK);
        send_len(host_socket, strlen(msg));
        send_msg(host_socket, msg, strlen(msg));
        close(host_socket);
        */
    }
    close(index_socket);

    return 0;
}

// ------------------------------------------------------------------------- //
void register_with_index(int index_socket, char *port) {
    // First, send the port the client will be listening on as a server.
    send_stat(index_socket, OK);                  //
    send_len(index_socket, MAX_PORT_LEN);         //
    send_msg(index_socket, port, MAX_PORT_LEN);   // ------------------- SEND A

    // Then call `ls' and read the results into a buffer.
    FILE *ls = popen("ls", "r");
    char buf[4096];
    memset(buf, 0, 4096);
    fread(buf, 4096, 1, ls);

    // Determine the number of files.
    int l = strnlen(buf, 4096);
    int num_files = 0;
    for (int i = 0; i < l; i++) {
        if (buf[i] == '\n') {
            num_files++;
        }
    }

    // Then we send the number of files to the server.
    char num_files_str[12];
    sprintf(num_files_str, "%d", num_files);
    send_stat(index_socket, OK);                 //
    send_len(index_socket, 12);                  //
    send_msg(index_socket, num_files_str, 12);   // -------------------- SEND B

    // Now that the server knows how many `packets' to expect, we send the 
    // filenames one at a time.
    char *token;
    token = strtok(buf, "\n");
    while (token != NULL) {
        printf("Sending %s\n", token);
        send_stat(index_socket, OK);                 //
        send_len(index_socket, MAX_FN_LEN);          //
        send_msg(index_socket, token, MAX_FN_LEN);   // ---------------- SEND C
        token = strtok(NULL, "\n");
    } 
}

// ------------------------------------------------------------------------- //
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
void * handle_connection(void *pclient) {
    int client_socket = *(int *)pclient;
    int stat, len;
    free(pclient);

    // Receive the filename to be sent to the client.
    recv_stat(client_socket, &stat);
    recv_len(client_socket, &len);
    char *filename = malloc(len);
    recv_msg(client_socket, filename, len);

    // Open the requested file and find the file size.
    FILE *fp = fopen(filename, "r");
    fseek(fp, 0L, SEEK_END);
    int f_size = ftell(fp);
    rewind(fp);

    // Read the file into a buffer.
    char *f_buf = malloc(f_size);
    fread(f_buf, f_size, 1, fp);
    fclose(fp);

    // Then send the buffer to the client.
    send_stat(client_socket, OK);
    send_len(client_socket, f_size);
    send_msg(client_socket, f_buf, f_size);

    free(f_buf);

    // Get the client's info.
    //struct sockaddr_in addr;
    //socklen_t addr_len = sizeof addr;
    //getpeername(client_socket, (struct sockaddr *)&addr, &addr_len);
    //char *client_ip = inet_ntoa(addr.sin_addr);
    //int client_port = ntohs(addr.sin_port);

    //char *logmsg = NULL;
    //sprintf(logmsg, "Server: Received download request for %s from %s:%d", 
    //        filename, client_ip, client_port);
    //write_to_log(logmsg);

    return NULL;
}

// ------------------------------------------------------------------------- //
void write_to_log(char *str) {
    //pthread_mutex_lock(&mutex);
    //FILE *fp = fopen(logfile, "a");
    //fprintf(stdout, "%s\n", str);
    //fclose(fp);
    //pthread_mutex_unlock(&mutex);
}

/*
// ------------------------------------------------------------------------- //
// register_with_index() function
//
void register_with_index(int index_socket, char *port_str) {
    // The first thing we need to after connecting to the indexing server is 
    // to send it the list of files in the watched directory.

    //char port_str[MAX_PORT_LEN];
    //sprintf(port_str, "%d", port);
    send_packet(index_socket, OK, 12, port_str);
 
    // We start by calling `ls' and reading the results into a buffer.
    FILE *ls = popen("ls", "r");
    char buf[4096];
    memset(buf, 0, 4096);
    fread(buf, 4096, 1, ls);

    // Then we determine the number of files.
    int l = strnlen(buf, 4096);
    int n = 0;
    for (int i = 0; i < l; i++) {
        if (buf[i] == '\n') {
            n++;
        }
    }

    // Then we send the number of files to the server.
    char num_files[12];
    sprintf(num_files, "%d", n);
    send_packet(index_socket, OK, 12, num_files);

    // Now that the server knows how many `packets' to expect, we send the 
    // filenames one at a time.
    char *token;
    token = strtok(buf, "\n");
    while (token != NULL) {
        printf("Sending %s\n", token);
        send_packet(index_socket, OK, MAX_FN_LEN, token);
        token = strtok(NULL, "\n");
    }
}

// ------------------------------------------------------------------------- //
// client_to_uid() function
//
void client_to_uid(char *ip, int port, char *uid) {
    char port_str[12];
    char *delim = ":";
    sprintf(port_str, "%d", port);
    strncpy(uid, ip, MAX_IP_LEN);
    strncat(uid, delim, 1);
    strncat(uid, port_str, 12);
}
*/



