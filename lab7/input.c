#include <sys/select.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv) {
	int ret;
	char buf[1024];
	fd_set fds_read;
	struct timeval tv;

	FD_ZERO(&fds_read);	
	FD_SET(STDIN_FILENO, &fds_read);

	tv.tv_sec = 2;
	tv.tv_usec = 0;

	while (1) {
		ret = select(1, &fds_read, NULL, NULL, &tv);

		if (ret == -1 || ret == 0) {
			break;
		}

		tv.tv_sec = 2;

		fgets(buf, 1024, stdin);

		printf("%s", buf);	
	}	

	return 0;
}
