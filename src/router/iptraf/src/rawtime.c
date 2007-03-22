#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argv[1] != NULL) {
        if (strcmp(argv[1], "-v") == 0) {
            printf("rawtime version 2.6.0\n");
            return (1);
        }
    }

    printf("%lu\n", time(NULL));
    return 0;
}
