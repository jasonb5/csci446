/* Jason Boutte
 * Tyler Parks
 *
 * CSCI 446 
 * Fall 2014
 *
 * This code is an updated version of the sample code from "Computer Networks: A Systems
 * Approach," 5th Edition by Larry L. Peterson and Bruce S. Davis. Some code comes from
 * man pages, mostly getaddrinfo(3). */#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_LINE 256
#define MAX_PENDING 5

int main(int argc, char *argv[]) {
	struct addrinfo hints;
	struct addrinfo *rp, *result;
	char buf[MAX_LINE];
	int fd, s, new_s;
	int len;
	char *port;
	
	// Check for appropriate arguments
	if (argc == 2) {
		port = argv[1];
	} else {
		fprintf(stderr, "usage: %s port\n", argv[0]);

		exit(1);
	}

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

	/* Iterate through the address list and try to perform passive open */
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		if ((s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1 ) {
			continue;
		}

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

	
	// Wait for new connections
	while(1) {
		if ((new_s = accept(s, rp->ai_addr, &(rp->ai_addrlen))) < 0) {
			perror("stream-talk-server: accept");
			close(s);
			exit(1);
		}

		// Get requested filename from client
		if ((len = recv(new_s, buf, sizeof(buf), 0)) == -1) {
			perror("stream-talk-server: recv");
			close(new_s);
			continue;
		}

	  // Try to open file and prepare client response
		// 1 on error, 0 on success	
		if ((fd = open(buf, O_RDONLY)) == -1) {
			sprintf(buf, "%d", 1);	
		} else {
			sprintf(buf, "%d", 0);
		} 

		// Send server response to client
		if (send(new_s, buf, strlen(buf), 0) == -1) {
			perror("stream-talk-server: send");
			close(fd);
			close(new_s);
			continue;
		}
		
		// If open failed we cleanup and continue 
		// listening for new connections
		if (fd == -1) {
			close(fd);
			close(new_s);
			continue;
		}

		// Wait for client to be ready
		if (recv(new_s, buf, sizeof(buf), 0) == -1) {
			perror("stream-talk-server: recv");
			close(fd);
			close(new_s);
			continue;	
		}
	
		// Check if client is read to receive
		if (atoi(buf) != 0) {
			close(fd);
			close(new_s);
			continue;
		}

		// Read contents of file	
		while ((len = read(fd, buf, sizeof(buf))) > 0) {
			// Send contents of buf, on failure we break read loop	
			if (send(new_s, buf, len, 0) == -1) {
				break;
			}
		}	

		// Cleanup
		close(fd);

		close(new_s);
	}

	close(s);

	return 0;
}
