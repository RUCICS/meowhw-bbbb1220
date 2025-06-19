#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#ifdef __linux__
#include <fcntl.h>
#endif

size_t io_blocksize(void) {
#ifdef _WIN32
    // Windows环境下使用固定值
    return 4096;
#else
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        page_size = 4096;
    }
    struct stat st;
    if (stat(".", &st) == 0) {
        size_t block_size = st.st_blksize;
        if (block_size > 0 && (block_size & (block_size - 1)) == 0) {
            return (size_t)(page_size > block_size ? page_size : block_size);
        }
    }
    return (size_t)page_size;
#endif
}

void* align_alloc(size_t size) {
    size_t page_size = io_blocksize();
    void* raw_ptr = malloc(size + page_size + sizeof(void*));
    if (raw_ptr == NULL) {
        return NULL;
    }
    uintptr_t addr = (uintptr_t)raw_ptr + sizeof(void*);
    uintptr_t aligned_addr = (addr + page_size - 1) & ~(page_size - 1);
    void** ptr_to_raw = (void**)(aligned_addr - sizeof(void*));
    *ptr_to_raw = raw_ptr;
    return (void*)aligned_addr;
}

void align_free(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    void** ptr_to_raw = (void**)((uintptr_t)ptr - sizeof(void*));
    void* raw_ptr = *ptr_to_raw;
    free(raw_ptr);
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
#ifdef __linux__
    // 顺序访问建议
    posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
#endif
    size_t base_buf_size = io_blocksize();
    size_t A = 8; // 这里A的值需要根据实验脚本结果调整
    size_t buf_size = base_buf_size * A;
    char *buf = align_alloc(buf_size);
    if (buf == NULL) {
        perror("Error allocating buffer");
        close(fd);
        return 1;
    }
    ssize_t n;
    while ((n = read(fd, buf, buf_size)) > 0) {
        if (write(STDOUT_FILENO, buf, n) != n) {
            perror("Error writing to stdout");
            align_free(buf);
            close(fd);
            return 1;
        }
    }
    if (n == -1) {
        perror("Error reading file");
        align_free(buf);
        close(fd);
        return 1;
    }
    align_free(buf);
    close(fd);
    return 0;
} 