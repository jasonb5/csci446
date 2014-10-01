#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

#define SERVER "farnsworth.ecst.csuchico.edu"
#define PORT "80"
#define PATH "/lab_docs/reset_instructions.pdf"
#define OUTPUT "local_file"

int main(int argc, char **argv) {
	int ret;
	int fd;
	int sock;
	ssize_t len;
	char buffer[2048];
	struct addrinfo hints;
	struct addrinfo *result, *rp;
		
	memset(&hints, 0, sizeof(struct addrinfo));
		
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = AI_PASSIVE;

	if ((ret = getaddrinfo(SERVER, PORT, &hints, &result)) != 0) {
		printf("%s\n", gai_strerror(ret));

		return 1;
	}

	for (rp = result; rp != NULL; rp = result->ai_next) {
		sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if (sock != -1) {
			break;
		}	
	}

	freeaddrinfo(result);

	if (sock == -1) {
		perror("socket");		

		return 1;
	}

	if (connect(sock, rp->ai_addr, rp->ai_addrlen) == -1) {
		perror("connect");

		close(sock);

		return 1;	
	}

	if ((fd = open(OUTPUT, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR)) == -1) {
		perror("open");

		close(sock);

		return 1;
	}

	sprintf(buffer, "GET %s HTTP/1.0\n\n", PATH);

	if (send(sock, buffer, strlen(buffer), 0) == -1) {
		perror("send");

		close(fd);

		close(sock);

		return 1;
	}	

	while ((ret = recv(sock, buffer, 2048, 0)) > 0) {
		printf("%d\n", ret);
		printf("Writing %d\n", write(fd, buffer, ret));
	}

	close(fd);

	close(sock);

	return 0;
}
