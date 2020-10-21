#include <stdio.h>
#include <stdlib.h>

#include "libyang.h"

int main(int argc, char **argv) {
    struct ly_ctx *ctx = NULL;

    if (argc != 2) {
        fprintf(stderr, "invalid usage\n");
        exit(EXIT_FAILURE);
    }

    ctx = ly_ctx_new(NULL, 0);
    if (!ctx) {
        fprintf(stderr, "failed to create context.\n");
        return 1;
    }

    while (__AFL_LOOP(100)) {
        lys_parse_path(ctx, argv[1], LYS_IN_YANG);

        ly_ctx_clean(ctx, NULL);
    }

    ly_ctx_destroy(ctx, NULL);
}
