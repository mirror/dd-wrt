/**
 * @file test_sec7_13_3.c
 * @author Pavol Vican
 * @brief Cmocka test for RFC 6020 section 7.13.3 (also 7.13.3.1) conformance.
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <cmocka.h>
#include <string.h>
#include <sys/wait.h>

#include "tests/config.h"
#include "libyang.h"

#define TEST_DIR "sec7_13_3"
#define TEST_NAME test_sec7_13_3
#define TEST_SCHEMA_COUNT 1
#define TEST_SCHEMA_LOAD_FAIL 0
#define TEST_DATA_FILE_COUNT 14
#define TEST_DATA_FILE_LOAD_FAIL 1,0,0,0,1,0,0,0,1,1,1,1,1,1
#define TEST_DATA_COMPARE_FAIL 1,0,0,0,0,0,0,0
#define TEST_DATA_COMPARE "ssh", "ssh", "42", "pattern", "", "true", "", ""
#define TEST_RPC_NODE "test5", "test5", "test5", "test4", \
                      "test3", "test3", "test2", "test6", \
                      "test3", "test3", "test", "test6", \
                      "test6", "test6"
struct state {
    struct ly_ctx *ctx;
    struct lyd_node *node;
    char *s;
};

static int
setup_f(void **state)
{
    struct state *st;

    (*state) = st = calloc(1, sizeof *st);
    if (!st) {
        fprintf(stderr, "Memory allocation error");
        return -1;
    }

    /* libyang context */
    st->ctx = ly_ctx_new(TESTS_DIR "/conformance/" TEST_DIR, 0);
    if (!st->ctx) {
        fprintf(stderr, "Failed to create context.\n");
        return -1;
    }
    st->s = NULL;
    return 0;
}

static int
teardown_f(void **state)
{
    struct state *st = (*state);

    lyd_free_withsiblings(st->node);
    ly_ctx_destroy(st->ctx, NULL);
    free(st->s);
    free(st);
    (*state) = NULL;

    return 0;
}

static void
TEST_RPC_OUTPUT(void **state)
{
    struct state *st = (*state);
    const int schemas_fail[] = {TEST_SCHEMA_LOAD_FAIL};
    const int data_files_fail[] = {TEST_DATA_FILE_LOAD_FAIL};
    char buf[1024];
    LYS_INFORMAT schema_format = LYS_IN_YANG;
    const struct lys_module *mod;
    int i, j, ret;
    const int data_compare_fail[] = {TEST_DATA_COMPARE_FAIL};
    const char *data_compare_string[] = {TEST_DATA_COMPARE};
    const char *data_rpc_name[] = {TEST_RPC_NODE};
    struct lyd_node *rpc;

    for (i = 0; i < 2; ++i) {
        for (j = 0; j < TEST_SCHEMA_COUNT; ++j) {
            sprintf(buf, TESTS_DIR "/conformance/" TEST_DIR "/mod%d.%s", j + 1, (schema_format == LYS_IN_YANG ? "yang" : "yin"));
            mod = lys_parse_path(st->ctx, buf, schema_format);
            if (schemas_fail[j]) {
                assert_ptr_equal(mod, NULL);
            } else {
                assert_ptr_not_equal(mod, NULL);
            }
        }

        for (j = 0; j < TEST_DATA_FILE_COUNT; ++j) {
            sprintf(buf, TESTS_DIR "/conformance/" TEST_DIR "/data%d.xml", j + 1);
            rpc = lyd_new(NULL, mod, data_rpc_name[j]);
            st->node = lyd_parse_path(st->ctx, buf, LYD_XML, LYD_OPT_RPCREPLY, rpc, NULL);
            lyd_free(rpc);
            if (data_files_fail[j]) {
                assert_ptr_equal(st->node, NULL);
            } else {
                assert_ptr_not_equal(st->node, NULL);
                lyd_print_mem(&st->s, st->node, LYD_XML, LYP_WITHSIBLINGS | LYP_FORMAT | LYP_WD_ALL);
                if (data_compare_fail[j]) {
                    assert_ptr_equal(strstr(st->s, data_compare_string[j]), NULL);
                } else {
                    assert_ptr_not_equal(strstr(st->s, data_compare_string[j]), NULL);
                }
                free(st->s);
                st->s = NULL;
            }
            lyd_free_withsiblings(st->node);
            st->node = NULL;
        }

        if (schema_format == LYS_IN_YANG) {
            /* convert the modules */
            for (j = 0; j < TEST_SCHEMA_COUNT; ++j) {
                sprintf(buf, BUILD_DIR "/yang2yin "
                             TESTS_DIR "/conformance/" TEST_DIR "/mod%d.yang "
                             TESTS_DIR "/conformance/" TEST_DIR "/mod%d.yin", j + 1, j + 1);
                ret = system(buf);
                if (ret == -1) {
                    fprintf(stderr, "system() failed (%s).\n", strerror(errno));
                    fail();
                } else if (WEXITSTATUS(ret) != 0) {
                    fprintf(stderr, "Executing command \"%s\" finished with %d.\n", buf, WEXITSTATUS(ret));
                    fail();
                }
            }

            schema_format = LYS_IN_YIN;
            ly_ctx_destroy(st->ctx, NULL);
            st->ctx = ly_ctx_new(TESTS_DIR "/conformance/" TEST_DIR, 0);
            if (!st->ctx) {
                fprintf(stderr, "Failed to create context.\n");
                fail();
            }
        } else {
            /* remove the modules */
            for (j = 0; j < TEST_SCHEMA_COUNT; ++j) {
                sprintf(buf, TESTS_DIR "/conformance/" TEST_DIR "/mod%d.yin", j + 1);
                if (unlink(buf)) {
                    fprintf(stderr, "unlink() on \"%s\" failed (%s).\n", buf, strerror(errno));
                }
            }
        }
    }
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(TEST_RPC_OUTPUT, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
