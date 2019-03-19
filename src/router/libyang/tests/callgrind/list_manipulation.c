#include <stdlib.h>
#include <valgrind/callgrind.h>

#include "tests/config.h"
#include "libyang.h"

#define SCHEMA TESTS_DIR "/callgrind/files/lists.yang"
#define DATA1 TESTS_DIR "/callgrind/files/lists.xml"
#define DATA2 TESTS_DIR "/callgrind/files/lists2.xml"

int
main(void)
{
    int ret = 0;
    struct ly_ctx *ctx = NULL;
    struct lyd_node *data1 = NULL, *data2 = NULL, *node;
    struct lyd_difflist *diff = NULL;

    ctx = ly_ctx_new(NULL, 0);
    if (!ctx) {
        ret = 1;
        goto finish;
    }

    if (!lys_parse_path(ctx, SCHEMA, LYS_YANG)) {
        ret = 1;
        goto finish;
    }

    data1 = lyd_parse_path(ctx, DATA1, LYD_XML, LYD_OPT_STRICT | LYD_OPT_DATA_NO_YANGLIB);
    if (!data1) {
        ret = 1;
        goto finish;
    }

    data2 = lyd_parse_path(ctx, DATA2, LYD_XML, LYD_OPT_STRICT | LYD_OPT_DATA_NO_YANGLIB);
    if (!data2) {
        ret = 1;
        goto finish;
    }

    CALLGRIND_START_INSTRUMENTATION;
    diff = lyd_diff(data1, data2, 0);
    if (!diff) {
        ret = 1;
        goto finish;
    }

    if (lyd_merge(data1, data2, LYD_OPT_DESTRUCT)) {
        ret = 1;
        goto finish;
    }
    data2 = NULL;

    node = data1->child->prev->prev->prev->prev->prev->prev->prev;
    lyd_unlink(node);

    if (lyd_insert(data1, node)) {
        ret = 1;
        goto finish;
    }

    if (lyd_validate(&data1, LYD_OPT_DATA | LYD_OPT_DATA_NO_YANGLIB, NULL)) {
        ret = 1;
        goto finish;
    }
    CALLGRIND_STOP_INSTRUMENTATION;

finish:
    lyd_free_diff(diff);
    lyd_free_withsiblings(data1);
    lyd_free_withsiblings(data2);
    ly_ctx_destroy(ctx, NULL);
    return ret;
}
