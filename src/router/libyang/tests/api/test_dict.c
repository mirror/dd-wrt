/*
 * @file test_dict.c
 * @author: Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief unit tests for functions from dict.h header
 *
 * Copyright (C) 2016 Deutsche Telekom AG.
 *
 * Author: Mislav Novakovic <mislav.novakovic@sartura.hr>
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

const char *a_data_xml = "\
<x xmlns=\"urn:a\">\n\
  <bubba>test</bubba>\n\
  </x>\n";

int
generic_init(char *yang_file, char *yang_folder)
{
    LYS_INFORMAT yang_format;
    char *schema = NULL;
    struct stat sb_schema;
    int fd = -1;

    if (!yang_file || !yang_folder) {
        goto error;
    }

    yang_format = LYS_IN_YIN;

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

    if (!lys_parse_mem(ctx, schema, yang_format)) {
        goto error;
    }

    /* cleanup */
    munmap(schema, sb_schema.st_size);

    return 0;

error:
    if (schema) {
        munmap(schema, sb_schema.st_size);
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
    char *yang_file = TESTS_DIR"/api/files/a.yin";
    char *yang_folder = TESTS_DIR"/api/files";
    int rc;

    rc = generic_init(yang_file, yang_folder);

    if (rc) {
        return -1;
    }

    return 0;
}

static int
teardown_f(void **state)
{
    (void) state; /* unused */
    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    return 0;
}

static void
test_lydict_insert(void **state)
{
    (void) state; /* unused */
    const char *value = "x";
    const char *string;
    size_t len = 1;

    string = lydict_insert(ctx, value, len);
    if (!string) {
        fail();
    }

    assert_string_equal(value, string);
    value = "bubba";
    len = 5;

    string = lydict_insert(ctx, value, len);
    if (!string) {
        fail();
    }

    assert_string_equal(value, string);
    lydict_remove(ctx, "bubba");
    lydict_remove(ctx, "x");
}

static void
test_lydict_insert_zc(void **state)
{
    (void) state; /* unused */
    char *value = NULL;

    value = strdup("x");
    if (!value) {
        fail();
    }
    const char *string;
    string = lydict_insert_zc(ctx, value);
    if (!string) {
        free(value);
        fail();
    }

    assert_string_equal("x", string);

    value = strdup("bubba");
    if (!value) {
        fail();
    }

    string = lydict_insert_zc(ctx, value);
    if (!string) {
        free(value);
        fail();
    }

    assert_string_equal("bubba", string);
    lydict_remove(ctx, "bubba");
    lydict_remove(ctx, "x");
}

static void
test_lydict_remove(void **state)
{
    (void) state; /* unused */
    char *value = NULL, *value2;
    const char *str;

    value = strdup("new_name");
    if (!value) {
        fail();
    }
    value2 = strdup("new_name");
    if (!value2) {
        free(value);
        fail();
    }

    const char *string;
    string = lydict_insert_zc(ctx, value); /* 1st instance */
    if (!string) {
        free(value);
        free(value2);
        fail();
    }

    assert_string_equal("new_name", string);
    str = lydict_insert(ctx, "new_name", 0); /* 2nd instance */
    assert_ptr_equal(str, string);
    lydict_remove(ctx, string); /* remove 2nd instance */
    lydict_remove(ctx, string); /* remove 1st instance */
    /* string content is supposed to be invalid since now! */
    str = lydict_insert_zc(ctx, value2);
    assert_ptr_not_equal(str, NULL);
    assert_ptr_not_equal(str, string);
    lydict_remove(ctx, str);
}

static void
test_similar_strings(void **state) {
    (void) state; /* unused */

    const char *ret = NULL;

    ret = lydict_insert(ctx, "aaab", 4);
    if (!ret) {
        fail();
    }
    assert_string_equal(ret, "aaab");

    ret = lydict_insert(ctx, "aaa", 3);
    if (!ret) {
        fail();
    }
    assert_string_equal(ret, "aaa");

    ret = lydict_insert(ctx, "bbb", 3);
    if (!ret) {
        fail();
    }
    assert_string_equal(ret, "bbb");

    ret = lydict_insert(ctx, "bbba", 4);
    if (!ret) {
        fail();
    }
    assert_string_equal(ret, "bbba");

    lydict_remove(ctx, "aaa");
    lydict_remove(ctx, "aaab");
    lydict_remove(ctx, "bbb");
    lydict_remove(ctx, "bbba");
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_lydict_insert, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lydict_insert_zc, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_lydict_remove, setup_f, teardown_f),
        cmocka_unit_test_setup_teardown(test_similar_strings, setup_f, teardown_f),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
