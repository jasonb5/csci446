/**
 * Jason Boutte
 * Tim Whitaker 
 *
 * CSCI 446
 * Fall 2014
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

void print_usage(void);
int connect_service(char *, char *, struct addrinfo **);

static char prompt[] = "Enter expression: ";

#define BSIZE 2000
#define RSIZE 20

int main(int argc, char **argv) {
    int s;
    socklen_t fromlen;
    char *buffer;
    char *rbuffer;
    struct addrinfo *sinfo;

    if (argv[1] == NULL || argv[2] == NULL) {
        print_usage();
        
        return 1;
    } 

    buffer = malloc(BSIZE * sizeof(char));

    rbuffer = malloc(RSIZE * sizeof(char));

    if ((s = connect_service(argv[1], argv[2], &sinfo)) < 0) {
        return 1;
    }

    while (1) {
        fputs(prompt, stdout);
        
        memset(buffer, 0, BSIZE * sizeof(char));

        fgets(buffer, BSIZE, stdin);

        if (buffer[0] == '\n') {
            break;
        }

        buffer[strlen(buffer)-1] = '\0';

        if (sendto(s, buffer, strlen(buffer)+1, 0, sinfo->ai_addr, sizeof(sinfo->ai_addr)) < 0) {
            fputs("Error sending data\n", stderr);

            break;
        }
 
        fromlen = sizeof(sinfo->ai_addr);
   
        if (recvfrom(s, rbuffer, 128, 0, sinfo->ai_addr, &fromlen) < 0) {
            fputs("Error receiving data\n", stderr);

            break;
        }      
    
        printf("Answer: %s\n", rbuffer);
    }

    close(s);

    freeaddrinfo(sinfo);

    free(rbuffer);

    free(buffer);

    return 0;
}

void print_usage(void) {
    printf("./calc_client <Server IP> <Port>\n");
}

int connect_service(char *server, char *port, struct addrinfo **sinfo) {
    int s;
    struct addrinfo hints;
    
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(server, port, &hints, sinfo)) {
        fputs("Error resolving address\n", stderr);

        return -1;
    }

    if ((s = socket(AF_INET, SOCK_STREAM, 6)) < 0) {
        fputs("Error creating socket\n", stderr);

        return -1;
    }   

    if (connect(s, (*sinfo)->ai_addr, (*sinfo)->ai_addrlen)) {
        fputs("Error connecting to server\n", stderr);        

        return -1;
    }

    return s;
}
