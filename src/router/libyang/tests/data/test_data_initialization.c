/**
 * @file test_data_initialization.c
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Cmocka data test initialization.
 *
 * Copyright (c) 2015 Sartura d.o.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include "tests/config.h"
#include "libyang.h"

struct ly_ctx *ctx = NULL;
struct lyd_node *root = NULL;

int
generic_init(char *config_file, char *yang_file, char *yang_folder)
{
    LYS_INFORMAT yang_format;
    LYD_FORMAT in_format;
    char *schema = NULL;
    char *config = NULL;
    struct stat sb_schema, sb_config;
    int fd = -1;

    if (!config_file || !yang_file || !yang_folder) {
        goto error;
    }

    yang_format = LYS_IN_YIN;
    in_format = LYD_XML;

    ctx = ly_ctx_new(yang_folder, 0);
    if (!ctx) {
        goto error;
    }

    fd = open(yang_file, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb_schema) == -1 || !S_ISREG(sb_schema.st_mode)) {
        goto error;
    }

    schema = mmap(NULL, sb_schema.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    fd = open(config_file, O_RDONLY);
    if (fd == -1 || fstat(fd, &sb_config) == -1 || !S_ISREG(sb_config.st_mode)) {
        goto error;
    }

    config = mmap(NULL, sb_config.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    fd = -1;

    if (!lys_parse_mem(ctx, schema, yang_format)) {
        goto error;
    }

    root = lyd_parse_mem(ctx, config, in_format, LYD_OPT_CONFIG | LYD_OPT_STRICT);
    if (!root) {
        goto error;
    }

    /* cleanup */
    munmap(config, sb_config.st_size);
    munmap(schema, sb_schema.st_size);

    return 0;

error:
    if (schema) {
        munmap(schema, sb_schema.st_size);
    }
    if (config) {
        munmap(config, sb_config.st_size);
    }
    if (fd != -1) {
        close(fd);
    }

    return -1;
}

static int
setup_f(void **state)
{
    (void) state; /* unused */
    char *config_file = TESTS_DIR"/data/files/hello.xml";
    char *yang_file = TESTS_DIR"/data/files/hello@2015-06-08.yin";
    char *yang_folder = TESTS_DIR"/data/files";
    int rc;

    rc = generic_init(config_file, yang_file, yang_folder);
    if (rc) {
        return -1;
    }

    return 0;
}

static int
teardown_f(void **state)
{
    (void) state; /* unused */
    lyd_free_withsiblings(root);
    ly_ctx_destroy(ctx, NULL);

    return 0;
}

static void
test_ctx_new_destroy(void **state)
{
    (void) state; /* unused */
    ctx = ly_ctx_new(NULL, 0);
    if (!ctx) {
        fail();
    }

    ly_ctx_destroy(ctx, NULL);
}

static void
test_container_name(void **state)
{
    (void) state; /* unused */
    struct lyd_node *node;
    const char *result = "";

    node = root;
    result = node->schema->name;

    assert_string_equal("hello", result);
}

static void
test_leaf_name(void **state)
{
    (void) state; /* unused */
    struct lyd_node *node;
    const char *result;

    node = root;
    result = node->schema->child->name;

    assert_string_equal("foo", result);
}

static void
test_leaf_list_parameters(void **state)
{
    (void) state; /* unused */
    struct lyd_node *node;
    struct lyd_node *tmp;
    const char *name_result;
    const char *str_result;
    int int_result;

    node = root;
    tmp = node->child->next;
    name_result = tmp->schema->name;
    int_result = ((struct lyd_node_leaf_list *)tmp)->value.int32;
    str_result = ((struct lyd_node_leaf_list *)tmp)->value_str;

    assert_int_equal(1234, int_result);
    assert_string_equal("1234", str_result);
    assert_string_equal("bar", name_result);
}

static void
test_yanglibrary(void **state)
{
    (void) state; /* unused */
    struct lyd_node *yanglib;
    int rc;

    yanglib = ly_ctx_info(ctx);
    assert_non_null(yanglib);

    rc = lyd_validate(&yanglib, LYD_OPT_DATA, NULL);

    /* cleanup */
    lyd_free_withsiblings(yanglib);

    assert_int_equal(rc, 0);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
                    cmocka_unit_test(test_ctx_new_destroy),
                    cmocka_unit_test_setup_teardown(test_container_name, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_leaf_name, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_leaf_list_parameters, setup_f, teardown_f),
                    cmocka_unit_test_setup_teardown(test_yanglibrary, setup_f, teardown_f), };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
