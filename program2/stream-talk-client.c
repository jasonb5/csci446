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
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_LINE 256

int main(int argc, char *argv[]) {
	struct addrinfo hints;
	struct addrinfo *rp, *result;
	char *host;
	char *port;
	char *file;	
	char buf[MAX_LINE];
	int s;
	int len;
	int fd;

	// Check for correct number of parameters
	if (argc == 4) {
		host = argv[1]; 

		port = argv[2];

		file = argv[3];
	} else {
		fprintf(stderr, "usage: %s host port file\n", argv[0]);
		exit(1);
	}

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

	// Send server file being requested
	if (send(s, file, strlen(file), 0) == -1) {
		perror("stream-talk-client: send");
		close(s);
		exit(1);
	}

	// Wait for server response
	if (recv(s, buf, sizeof(buf), 0) == -1) {
		perror("stream-talk-client: recv");
		close(s);
		exit(1);
	}

	// Check for server error
	if (atoi(buf) == 1) {
		fprintf(stderr, "Server Error: Unable to access file '%s'\n", file);
		close(s);
		exit(1);
	}

	sprintf(buf, "%s", file);
	
	// Open local file. Create if doesn't exist, overwrite if
	// it exists
	if ((fd = open(buf, O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR | S_IRUSR)) == -1) {
		perror("stream-talk-client: open");
		close(s);
		exit(1);
	}

	sprintf(buf, "0");

	// Notify server we're ready to receive data
	if (send(s, buf, strlen(buf), 0) == -1) {
		perror("stream-talk-client: send");
		close(fd);
		close(s);
		exit(1);
	}

	// Receive file from server
	while ((len = recv(s, buf, sizeof(buf), 0)) > 0) {
		// Write contents of buf	
		if (write(fd, buf, len) == -1) {
			perror("stream-talk-client: write");
			close(fd);
			close(s);
			exit(1);
		}
	}

	// Shutdown socket
	shutdown(s, SHUT_RDWR);

	// Cleanup
	close(fd);

	close(s);

	return 0;
}
