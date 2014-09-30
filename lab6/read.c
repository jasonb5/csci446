#include <fcntl.h>
#include <stdio.h>

#define BSIZE 1024

int main(int argc, char **argv) {
    int fd;
    char b[BSIZE];

    if (argv[1] == NULL) {
        printf("Missing input parameter\n");

        return 1;
    }

    fd = open(argv[1], O_RDONLY);

    if (fd < 0) {
        perror("open");

        return 1;
    }

    while (read(fd, &b, BSIZE) > 0) {
        printf("%s", b);
    }
    
    close(fd);

    return 0;
}
