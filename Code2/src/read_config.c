#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "read_config.h"
#include "constants.h"

// ------------------------------------------------------------------------- //
void trim(char *str) {
    int i = 0;
    char current = str[0];
    while (current !='\0') {
        i++;
        current = str[i];
    }
    str[i - 1] = '\0';
}

// ------------------------------------------------------------------------- //
void read_config(char *config, char *ip, char *port, char *wd) {
    FILE *fp = fopen(config, "r");
    
    if (ip != NULL) {
        fgets(ip, MAX_IP_LEN, fp);
        trim(ip);
    }
    if (port != NULL) {
        fgets(port, MAX_PORT_LEN, fp);
        trim(port);
    }
    if (wd != NULL) {
        fgets(wd, MAX_WD_LEN, fp);
        trim(wd);
    }

    fclose(fp);
}

// ------------------------------------------------------------------------- //

