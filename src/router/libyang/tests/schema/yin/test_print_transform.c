/**
 * \file test_print_transform.c
 * \author Michal Vasko <mvasko@cesnet.cz>
 * \brief libyang tests - transforming node-ids schema -> JSON -> schema and printing in both YANG and YIN
 *
 * Copyright (c) 2016 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cmocka.h>

#include "libyang.h"
#include "../tests/config.h"

#define SCHEMA_FOLDER TESTS_DIR"/schema/yin/files"

static int
diff(const char *data, FILE *model)
{
    char *data2;
    unsigned int length;
    size_t read;

    length = strlen(data);

    data2 = malloc(length);

    read = fread(data2, 1, length, model);
    if (read != length) {
        goto fail;
    }

    if (strncmp(data, data2, length)) {
        goto fail;
    }

    free(data2);
    return 0;

fail:
    fprintf(stderr, "diff failed on:\n\"%s\"\nand\n\"%s\"\n", data, data2);
    free(data2);
    return 1;
}

static int
setup_ctx(void **state)
{
    //ly_verb(LY_LLVRB);
    (*state) = ly_ctx_new(SCHEMA_FOLDER, 0);
    if (!(*state)) {
        return -1;
    }

    return 0;
}

static int
teardown_ctx(void **state)
{
    ly_ctx_destroy((struct ly_ctx *)(*state), NULL);
    (*state) = NULL;

    return 0;
}

static void
execute_test_with_filenames(void **state,
                            char *module_name,
                            char *yang_file,
                            char *yin_file)
{
    struct ly_ctx *ctx = *state;
    const struct lys_module *module;
    char *new;
    FILE *file;
    int ret;

    module = ly_ctx_load_module(ctx, module_name, NULL);
    assert_non_null(module);

    /* YANG */
    ret = lys_print_mem(&new, module, LYS_OUT_YANG, NULL, 0, 0);
    assert_int_equal(ret, 0);

    file = fopen(yang_file, "r");
    assert_non_null(file);

    ret = diff(new, file);
    free(new);
    fclose(file);
    assert_int_equal(ret, 0);

    /* YIN */
    ret = lys_print_mem(&new, module, LYS_OUT_YIN, NULL, 0, 0);
    assert_int_equal(ret, 0);

    file = fopen(yin_file, "r");
    assert_non_null(file);

    ret = diff(new, file);
    free(new);
    fclose(file);
    assert_int_equal(ret, 0);
}

static void
test_modules(void **state)
{
execute_test_with_filenames(state,
                            "d2",
                            SCHEMA_FOLDER"/d2_output.yang",
                            SCHEMA_FOLDER"/d2_output.yin");
}

static void
test_modules_2(void **state)
{
execute_test_with_filenames(state,
                            "compname-int-unit-test",
                            SCHEMA_FOLDER"/compname-int-unit-test-output.yang",
                            SCHEMA_FOLDER"/compname-int-unit-test-output.yin");
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_modules, setup_ctx, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_modules_2, setup_ctx, teardown_ctx)
    };

    return cmocka_run_group_tests(cmut, NULL, NULL);
}

