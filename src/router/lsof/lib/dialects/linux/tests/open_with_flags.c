#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>

struct {
    const char *name;
    int flag;
} table[] = {
    {"path", O_PATH},
    {"cx", O_CLOEXEC},
    {"tmpf", O_TMPFILE},
    {"rdwr", O_RDWR},
};

#define TABLELEN sizeof(table) / sizeof(table[0])

static int encode(const char *const s) {
    for (int i = 0; i < TABLELEN; i++)
        if (!strcmp(s, table[i].name))
            return table[i].flag;
    return 0;
}

static void print_usage(FILE *fp, const char *const prog) {
    fprintf(fp, "Usage:\n");
    fprintf(fp, "	%s FILENAME FLAG ...\n", prog);
    fprintf(fp, "Flags:\n");
    for (int i = 0; i < TABLELEN; i++)
        fprintf(fp, "	%s\n", table[i].name);
}

int main(int argc, char **argv) {

    if (argc < 3) {
        fprintf(stderr, "Too few argument\n");
        print_usage(stderr, argv[0]);
        return 1;
    }

    char *fname = argv[1];
    int flags = 0;

    for (int i = 2; i < argc; i++)
        flags |= encode(argv[i]);

    int fd = open(fname, flags, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    printf("%d\n", getpid());
    fflush(stdout);
    pause();
    return 0;
}
