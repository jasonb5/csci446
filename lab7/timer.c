#include <sys/select.h>
#include <stdio.h>

int main(int argc, char **argv) {
	int ret;
	struct timeval tm;

	tm.tv_sec = 1;
	tm.tv_usec = 0;

	while (1) {
		ret = select(0, NULL, NULL, NULL, &tm);

		tm.tv_sec = 1;

		if (ret == -1) {
			break;
		}

		printf("Message\n");
	}
	
	return 0;
}
