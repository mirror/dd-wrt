#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define handle_error(msg)                                                      \
    do {                                                                       \
        perror(msg);                                                           \
        exit(EXIT_FAILURE);                                                    \
    } while (0)

int main(int argc, char *argv[]) {
    char *addr;
    int fd;
    struct stat sb;

    if (argc != 2)
        exit(EXIT_FAILURE);

    fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        handle_error("open");

    if (fstat(fd, &sb) == -1)
        handle_error("fstat");

    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED)
        handle_error("mmap");

    pause();
    return 0;
}
