#include "common.h"
#include "network.h"
#include "read_config.h"
#include "constants.h"

#define USAGE "usage: ./peer_node <config_file> <server_ip> <server_port>"
#define CONFIG_FILE "peernode.cfg"

// ------------------------------------------------------------------------- //
void register_with_index(int, char *);
void client_to_uid(char *, int, char *);

// ------------------------------------------------------------------------- //
// main() function
//
int main(int argc, char **argv) {
    // Get parameters from config file.
    char port_str[MAX_PORT_LEN];
    char wd[MAX_WD_LEN];

    if (argc != 4) {
        printf("%s\n", USAGE);
        exit(1);
    }

    read_config(argv[1], NULL, port_str, wd);

    // Change the directory.
    if (chdir(wd) == -1) {
        perror("chdir");
        exit(1);
    }

    // Initialize the node as a client of the indexing server.
    int index_socket = init_client(argv[2], argv[3]);
    printf("client: connected to %d\n", index_socket);

    register_with_index(index_socket, port_str);

    // SERVER SECTION ------------------------------------------------------ //
    if (fork() == 0) {
        int listener_socket, client_socket;

        listener_socket = init_server(port_str, 10);
        //printf("server: waiting for connection...\n");
        while (1) {
            //printf("here we go...\n");
            client_socket = accept(listener_socket, NULL, NULL);
            //printf("..........\n");
            int stat, len;
            recv_stat(client_socket, &stat);
            recv_len(client_socket, &len);
            char *msg = malloc(len);
            recv_msg(client_socket, msg, len);
            printf("received: %s\n", msg);
            free(msg);
        }
        return 0;
    }

    // CLIENT SECTION ------------------------------------------------------ //
    while (1) {
        int stat, len;
        char cmd[MAX_FN_LEN];
        printf("> ");
        fgets(cmd, MAX_FN_LEN, stdin);
        trimnl(cmd);

        if (strncmp(cmd, "list", MAX_FN_LEN) != 0) {
            continue;
        }

        send_stat(index_socket, OK);
        send_len(index_socket, MAX_FN_LEN);
        send_msg(index_socket, cmd, MAX_FN_LEN);

        if (strncmp(cmd, "list", MAX_FN_LEN) == 0) {
            recv_stat(index_socket, &stat);
            recv_len(index_socket, &len);
            char *num_files_str = malloc(len);
            recv_msg(index_socket, num_files_str, len);
            int num_files = atoi(num_files_str);
            printf("%d files\n", num_files);
            free(num_files_str);

            for (int i = 0; i < num_files; i++) {
                recv_stat(index_socket, &stat);
                recv_len(index_socket, &len);
                char *filename = malloc(len);
                recv_msg(index_socket, filename, len);
                printf("%s\n", filename);
                free(filename);
            }
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



