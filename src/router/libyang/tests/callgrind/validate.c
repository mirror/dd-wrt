#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <valgrind/callgrind.h>

#include "tests/config.h"
#include "libyang.h"

int
main(int argc, char **argv)
{
    int i;
    char *path;
    struct ly_ctx *ctx;
    struct lyd_node *data;

    if (argc < 3) {
        return 1;
    }

    ctx = ly_ctx_new(NULL, 0);
    if (!ctx) {
        return 1;
    }

    for (i = 1; i < argc - 1; ++i) {
        asprintf(&path, "%s/callgrind/files/%s", TESTS_DIR, argv[i]);
        if (!lys_parse_path(ctx, path, LYS_YANG)) {
            free(path);
            ly_ctx_destroy(ctx, NULL);
            return 1;
        }
        free(path);
    }

    asprintf(&path, "%s/callgrind/files/%s", TESTS_DIR, argv[argc - 1]);

    CALLGRIND_START_INSTRUMENTATION;
    data = lyd_parse_path(ctx, path, LYD_XML, LYD_OPT_STRICT | LYD_OPT_DATA_NO_YANGLIB);
    CALLGRIND_STOP_INSTRUMENTATION;

    free(path);
    if (!data) {
        ly_ctx_destroy(ctx, NULL);
        return 1;
    }

    lyd_free_withsiblings(data);
    ly_ctx_destroy(ctx, NULL);
    return 0;
}
