#include "common.h"
#include "network.h"
#include "read_config.h"
#include "constants.h"

#define USAGE "usage: ./peer_node <config_file> <server_ip> <server_port>"
#define CONFIG_FILE "peernode.cfg"

int main(int argc, char **argv) {
    // Get parameters from config file.
    //char ip[MAX_IP_LEN];
    char port[MAX_PORT_LEN];
    char wd[MAX_WD_LEN];

    if (argc != 4) {
        printf("%s\n", USAGE);
        exit(1);
    }

    read_config(argv[1], NULL, port, wd);

    // Change the directory.
    if (chdir(wd) == -1) {
        perror("chdir");
        exit(1);
    }

    // Initialize the node as a client of the indexing server.
    int index_socket = init_client(argv[2], argv[3]);
    printf("client: connected to %d\n", index_socket);

    // The first thing we need to after connecting to the indexing server is 
    // to send it the list of files in the watched directory.

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

    //while (1);
    close(index_socket);

    return 0;
}
