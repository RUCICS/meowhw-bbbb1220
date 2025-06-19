#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    char c;
    ssize_t n;
    while ((n = read(fd, &c, 1)) > 0) {
        if (write(STDOUT_FILENO, &c, 1) != 1) {
            perror("Error writing to stdout");
            close(fd);
            return 1;
        }
    }

    if (n == -1) {
        perror("Error reading file");
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
} 