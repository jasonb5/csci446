/* This code is an updated version of the sample code from "Computer Networks: A Systems
 * Approach," 5th Edition by Larry L. Peterson and Bruce S. Davis. Some code comes from
 * man pages, mostly getaddrinfo(3). */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

#define SERVER_PORT "5432"
#define MAX_LINE 256
#define TIMEOUT_SEC 2
#define TIMEOUT_USEC 0

int
main(int argc, char *argv[])
{
	struct addrinfo hints;
	struct addrinfo *rp, *result;
	char *host;
	char buf[MAX_LINE];
	int s;
	int len;
	struct timeval tv;	
	fd_set fds_read;

	if (argc==2)
	{
		host = argv[1];
	}
	else
	{
		fprintf(stderr, "usage: %s host\n", argv[0]);
		exit(1);
	}

	/* Translate host name into peer's IP address */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	if ((s = getaddrinfo(host, SERVER_PORT, &hints, &result)) != 0 )
	{
		fprintf(stderr, "%s: getaddrinfo: %s\n", argv[0], gai_strerror(s));
		exit(1);
	}

	/* Iterate through the address list and try to connect */
	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		if ((s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1 )
		{
			continue;
		}

		if (connect(s, rp->ai_addr, rp->ai_addrlen) != -1)
		{
			break;
		}

		close(s);
	}
	if (rp == NULL)
	{
		perror("stream-talk-client: connect");
		exit(1);
	}
	freeaddrinfo(result);

	FD_ZERO(&fds_read);
	FD_SET(STDIN_FILENO, &fds_read);	

	tv.tv_sec = TIMEOUT_SEC;
	tv.tv_usec = TIMEOUT_USEC;

	// Detect timeout on stdin
	if (select(STDIN_FILENO+1, &fds_read, NULL, NULL, &tv) <= 0) {
		close(s);

		return 0;
	}

	/* Main loop: get and send lines of text */
	while (fgets(buf, sizeof(buf), stdin))
	{
		// Reset timeout values
		tv.tv_sec = TIMEOUT_SEC;
		tv.tv_usec = TIMEOUT_USEC;	
			
		buf[MAX_LINE-1] = '\0';
		len = strlen(buf) + 1;
		send(s, buf, len, 0);

		// Detect timeout on stdin
		if (select(STDIN_FILENO+1, &fds_read, NULL, NULL, &tv) <= 0) {
			break;
		}	
	}

	close(s);

	return 0;
}
