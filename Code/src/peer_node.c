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
    //char ip[MAX_IP_LEN];
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

    printf("sending filename\n");
    char fn[10] = "file1";
    send_packet(index_socket, OK, 10, fn);

    char *uid = recv_packet(index_socket);
    printf("file is at %s\n", uid);

    close(index_socket);

    return 0;
}

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
// client_to_uid(0 function
//
void client_to_uid(char *ip, int port, char *uid) {
    char port_str[12];
    char *delim = ":";
    sprintf(port_str, "%d", port);
    strncpy(uid, ip, MAX_IP_LEN);
    strncat(uid, delim, 1);
    strncat(uid, port_str, 12);
}




