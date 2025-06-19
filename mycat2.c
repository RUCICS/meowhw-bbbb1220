#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

size_t io_blocksize(void) {
#ifdef _WIN32
    // Windows环境下使用固定值
    return 4096;
#else
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        // 如果获取页大小失败，使用默认值4KB
        return 4096;
    }
    return (size_t)page_size;
#endif
}

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

    size_t buf_size = io_blocksize();
    char *buf = malloc(buf_size);
    if (buf == NULL) {
        perror("Error allocating buffer");
        close(fd);
        return 1;
    }

    ssize_t n;
    while ((n = read(fd, buf, buf_size)) > 0) {
        if (write(STDOUT_FILENO, buf, n) != n) {
            perror("Error writing to stdout");
            free(buf);
            close(fd);
            return 1;
        }
    }

    if (n == -1) {
        perror("Error reading file");
        free(buf);
        close(fd);
        return 1;
    }

    free(buf);
    close(fd);
    return 0;
} 