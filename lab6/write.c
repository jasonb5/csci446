#include <fcntl.h>
#include <stdio.h>

#define BSIZE 1024

int main(int argc, char **argv) {
    int fd_in;
    int fd_out;
    int in;
    int i;
    char b[BSIZE];
    char o_name[256];

    if (argv[1] == NULL) {
        printf("Missing input file\n");   
 
        return 1;
    }

    fd_in = open(argv[1], O_RDONLY);

    if (fd_in < 0) {
        perror("open_input");

        return 1;
    }

    sprintf(o_name, "%s.up", argv[1]);

    fd_out = open(o_name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd_out < 0) {
        perror("open_output");

        close(fd_in);

        return 1;
    }

    while ((in = read(fd_in, &b, BSIZE)) > 0) {
        for (i = 0; i < in; ++i) {
            b[i] = toupper(b[i]);
        }   

        write(fd_out, &b, in);
    }

    close(fd_in);

    close(fd_out);

    return 0;
}
