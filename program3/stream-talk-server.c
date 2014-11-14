/* Jason Boutte
 * Tyler Parks
 *
 * CSCI 446 
 * Fall 2014
 *
 * This code is an updated version of the sample code from "Computer Networks: A Systems
 * Approach," 5th Edition by Larry L. Peterson and Bruce S. Davis. Some code comes from
 * man pages, mostly getaddrinfo(3). */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "packetErrorSend.h"

#define debug(M, ...) fprintf(stdout, "%s:%d:" M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define MAX_LINE 256
#define MAX_FILENAME 1024
#define MAX_PENDING 5
#define TIMEOUT_SEC 5
#define TIMEOUT_USEC 0

int main(int argc, char *argv[]) {
	int fd;
	int len;
	int ret;
	char *port;
	int alt_bit;
	int s, new_s;
	fd_set read_set;
	char buf[MAX_LINE];
	struct addrinfo hints;
	struct timeval timeout;
	char filename[MAX_FILENAME];
	struct addrinfo *rp, *result;
	
	// Check for appropriate arguments
	if (argc == 2) {
		port = argv[1];
	} else {
		fprintf(stderr, "usage: %s port\n", argv[0]);

		exit(1);
	}

	// Initialize alternating bit to one
	alt_bit = 1;

	// Zero out the fd_set
	FD_ZERO(&read_set);

	/* Build address data structure */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	/* Get local address info */
	if ((s = getaddrinfo(NULL, port, &hints, &result)) != 0 ) {
		fprintf(stderr, "%s: getaddrinfo: %s\n", argv[0], gai_strerror(s));
		exit(1);
	}

#warning "Remove this as well"
	int optval = 1;

	/* Iterate through the address list and try to perform passive open */
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		if ((s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1 ) {
			continue;
		}

#warning "Remove this on release"
		setsockopt(s, IPPROTO_TCP, SO_REUSEADDR, &optval, sizeof optval);

		// Bind socket to address
		if (!bind(s, rp->ai_addr, rp->ai_addrlen)) {
			break;
		}

		close(s);
	}
	
	if (rp == NULL)	{
		perror("stream-talk-server: bind");
		exit(1);
	}
		
	freeaddrinfo(result);

	// Put socket into listen mode
	if (listen(s, MAX_PENDING) == -1) {
		perror("stream-talk-server: listen");
		close(s);
		exit(1);
	}

	debug("Listening for connections");
	
	// Wait for new connections
	while(1) {
		if ((new_s = accept(s, rp->ai_addr, &(rp->ai_addrlen))) < 0) {
			perror("stream-talk-server: accept");
			close(s);
			exit(1);
		}

		FD_SET(new_s, &read_set);
	
		// Wait for filename from client
		if ((len = recv(new_s, buf, sizeof(buf), 0)) == -1) {
			close(new_s);
			break;
		}	

		debug("Requested '%s' length %d", buf, (int)strlen(buf));

		alt_bit = atoi(&buf[0]);

		memcpy(&filename, &buf[1], strlen(buf)-1);

		sprintf(buf, "%d", alt_bit);

		// Acknowledge 
		if (packetErrorSend(new_s, buf, strlen(buf), 0) == -1) {
			close(new_s);
			break;
		}	

		debug("Acknowleged");

		// Attempt to open file for reading
		if ((fd = open(filename, O_RDONLY)) == -1) {
			ret = 1;
    } else {
			ret = 0;
		}

		sprintf(buf, "%d%d", alt_bit, ret);

		debug("Sending server status");
		do {
			if ((ret = packetErrorSend(s, buf, strlen(buf), 0)) == -1) {
				debug("Failed on send");
				break;
			}
		
			timeout.tv_sec = TIMEOUT_SEC;
			timeout.tv_usec = TIMEOUT_USEC;

			ret = select(new_s+1, &read_set, NULL, NULL, &timeout);

			if (ret == 0) {
				debug("Timeout");
				continue;
			} else if (ret == -1) {
				debug("Failed on select");
				break;
			}

			if ((ret = recv(new_s, buf, sizeof(buf), 0)) == -1) {
				break;
			}

			buf[ret] = '\0';

			debug("Received ACK %s length %d", buf, ret);

			if (alt_bit == atoi(&buf[0])) {
				break;
			}
		} while (1);	
			
		if (ret == -1) {
			debug("Something went wrong in status");
			close(new_s);
			break;
		}			

		debug("Read to send file");

		close(new_s);

#warning "Remove this debug break"
		break;
	}

	close(s);

	return 0;
}
