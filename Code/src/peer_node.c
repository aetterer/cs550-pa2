#include "common.h"
#include "client_server.h"
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

    FILE *ls = popen("ls", "r");
    char buf[4088];
    memset(buf, 0, 4088);
    fread(buf, 4088, 1, ls);

    send_packet(index_socket, 0, 4088, buf);

    //while (1);
    close(index_socket);

    return 0;
}
