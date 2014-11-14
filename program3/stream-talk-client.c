/* Jason Boutte
 * Tyler Parks
 *
 * CSCI 446 
 * Fall 2014
 *
 * This code is an updated version of the sample code from "Computer Networks: A Systems
 * Approach," 5th Edition by Larry L. Peterson and Bruce S. Davis. Some code comes from
 * man pages, mostly getaddrinfo(3). */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "packetErrorSend.h"

#define debug(M, ...) fprintf(stdout, "%s:%d:" M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define MAX_LINE 256
#define OUTPUT_PATTERN "%s_new"
#define TIMEOUT_SEC 5
#define TIMEOUT_USEC 0

int main(int argc, char *argv[]) {
	int s;
	int fd;
	int ret;
	int len;
	char *host;
	char *port;
	char *file;	
	int alt_bit;
	fd_set read_set;
	char buf[MAX_LINE];
	struct addrinfo hints;
	struct timeval timeout;
	struct addrinfo *rp, *result;

	// Check for correct number of parameters
	if (argc == 4) {
		host = argv[1]; 

		port = argv[2];

		file = argv[3];
	} else {
		fprintf(stderr, "usage: %s host port file\n", argv[0]);
		exit(1);
	}

	alt_bit = 1;

	FD_ZERO(&read_set);

	/* Translate host name into peer's IP address */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	if ((s = getaddrinfo(host, port, &hints, &result)) != 0 ) {
		fprintf(stderr, "%s: getaddrinfo: %s\n", argv[0], gai_strerror(s));
		exit(1);
	}

	/* Iterate through the address list and try to connect */
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		if ((s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1 ) {
			continue;
		}

		if (connect(s, rp->ai_addr, rp->ai_addrlen) != -1) {
			break;
		}

		close(s);
	}

	if (rp == NULL)	{
		perror("stream-talk-client: connect");
		exit(1);
	}

	freeaddrinfo(result);

	FD_SET(s, &read_set);

	alt_bit = 0;

	sprintf(buf, "%d%s", alt_bit, file); 	

	debug("Requesting '%s' sending %s length %d", file, buf, (int)strlen(buf));

	// Send server requested filename
	do {
		if ((ret = packetErrorSend(s, buf, strlen(buf), 0)) == -1) {
			break;
		}

		timeout.tv_sec = TIMEOUT_SEC;
		timeout.tv_usec = TIMEOUT_USEC;

		ret = select(s+1, &read_set, NULL, NULL, &timeout);

		if (ret == 0) {
			continue;
		} else if (ret == -1) {
			break;
		}

		if ((ret = recv(s, buf, sizeof(buf), 0)) == -1) {
			break;
		}
		
		buf[ret] = '\0';

		debug("Received ACK %s length %d", buf, ret);

		if (alt_bit == atoi(&buf[0])) {
			debug("Matching alt bit");
			break;
		}
	} while (1);

	if (ret == -1) {
		close(s);
		exit(1);
	}	

	debug("Waiting for server status");

	// Wait for server status 
	if ((recv(s, buf, sizeof(buf), 0)) == -1) {
		close(s);
		exit(1);
	}	

	alt_bit = atoi(&buf[0]);

	if (buf[1] == 1) {
		fprintf(stdout, "Server Error opening file\n");
		// Server failed to open file
		close(s);
		exit(1);
	}

	sprintf(buf, "%d", alt_bit);

	// Acknowledg 
	if (packetErrorSend(s, buf, strlen(buf), 0) == -1) {
		close(s);
		exit(1);
	}


	close(s);

	return 0;
}
