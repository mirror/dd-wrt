/**
 * \file test_ietf.c
 * \author Radek Krejci <rkrejci@cesnet.cz>
 * \brief libyang tests - loading standard IETF modules
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
#include <limits.h>

#include <cmocka.h>

#include "libyang.h"
#include "../../src/context.h"
#include "tests/config.h"

#define SCHEMA_COUNT 17

#define SCHEMA_FOLDER_YIN TESTS_DIR"/schema/yin/ietf"
#define SCHEMA_FOLDER_YANG TESTS_DIR"/schema/yang/ietf"

char files[SCHEMA_COUNT][32] = { "iana-crypt-hash", "iana-if-type",
                           "ietf-inet-types", "ietf-interfaces", "ietf-ipfix-psamp", "ietf-ip",
                           "ietf-netconf-acm", "ietf-netconf-monitoring", "ietf-netconf-notifications",
                           "ietf-netconf-partial-lock", "ietf-netconf-with-defaults", "ietf-netconf",
                           "ietf-snmp", "ietf-system", "ietf-x509-cert-to-name", "ietf-yang-smiv2",
                           "ietf-yang-types"
                         };
char yang_files[34][50] = {};
char yin_files[34][50] = {};

static int
setup_ctx(void **state, int format, int flags)
{
    //ly_verb(LY_LLVRB);
    if (format == LYS_IN_YANG){
        (*state) = ly_ctx_new(SCHEMA_FOLDER_YANG, flags);
    } else {
        (*state) = ly_ctx_new(SCHEMA_FOLDER_YIN, flags);
    }
    if (!(*state)) {
        return -1;
    }

    return 0;
}

static int
setup_ctx_yin(void **state)
{
    return setup_ctx(state, LYS_IN_YIN, 0);
}

static int
setup_ctx_yang(void **state)
{
    return setup_ctx(state, LYS_IN_YANG, 0);
}

static int
setup_ctx_yin_trusted(void **state)
{
    return setup_ctx(state, LYS_IN_YIN, LY_CTX_TRUSTED);
}

static int
setup_ctx_yang_trusted(void **state)
{
    return setup_ctx(state, LYS_IN_YANG, LY_CTX_TRUSTED);
}

static int
teardown_ctx(void **state)
{
    ly_ctx_destroy((struct ly_ctx *)(*state), NULL);
    (*state) = NULL;

    return 0;
}

static void
write_file(char *filename, const char *name, const struct lys_module *module, LYS_OUTFORMAT format)
{
    FILE *f;

    strcpy(filename, name);
    strcat(filename, "_");
    strcat(filename, module->name);
    f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "unable to open \"%s\" file.\n", filename);
        fail();
    }
    lys_print_file(f, module, format, NULL, 0, 0);
    fclose(f);
}

static void
test_modules(void **state)
{
    struct ly_ctx *ctx = *state;
    char *extension, path[PATH_MAX];
    const struct lys_module *module;
    int i, format;

    if (!strcmp(ctx->models.search_paths[0], realpath(SCHEMA_FOLDER_YIN, path))) {
        extension = ".yin";
        format = LYS_IN_YIN;
    }  else {
        extension = ".yang";
        format = LYS_IN_YANG;
    }

    if (chdir(ctx->models.search_paths[0])) {
        fprintf(stderr, "unable to open \"%s\" folder.\n", ctx->models.search_paths[0]);
        fail();
    }

    for (i = 0; i < SCHEMA_COUNT; i++) {
        sprintf(path, "%s%s", files[i], extension);
        fprintf(stdout, "Loading \"%s\" module ... ", path);
        if (!(module = lys_parse_path(ctx, path, format))) {
            fprintf(stdout, "failed\n");
            fail();
        }
        fprintf(stdout, "ok\n");
        if (format == LYS_IN_YIN) {
            write_file(yang_files[i], "tmp1", module, LYS_OUT_YANG);
            write_file(yin_files[i], "tmp3", module, LYS_OUT_YIN);
        } else {
            write_file(yang_files[i + SCHEMA_COUNT], "tmp2", module, LYS_OUT_YANG);
            write_file(yin_files[i + SCHEMA_COUNT], "tmp4", module, LYS_OUT_YIN);
        }
    }
}

static void
compare_modules(void **state)
{
    int i, ch1, ch2;
    FILE *f1, *f2;
    char filename[1024];
    char (*files)[2 * SCHEMA_COUNT][50] = *state;

    for (i = 0; i < SCHEMA_COUNT; ++i) {
        if (!(*files)[i]) {
            fprintf(stderr, "missing file name.\n");
            fail();
        }
        fprintf(stdout, "Compare \"%s\" module ... ", &(*files)[i][5]);
        strcpy(filename, SCHEMA_FOLDER_YIN);
        strcat(filename, "/");
        strcat(filename, (*files)[i]);
        f1 = fopen(filename, "r");
        if (!f1) {
            fprintf(stdout, "failed\n");
            fprintf(stderr, "unable to open \"%s\" file.\n", filename);
            fail();
        }
        strcpy(filename, SCHEMA_FOLDER_YANG);
        strcat(filename, "/");
        strcat(filename, (*files)[i + SCHEMA_COUNT]);
        f2 = fopen(filename, "r");
        if (!f2) {
            fclose(f1);
            fprintf(stdout, "failed\n");
            fprintf(stderr, "unable to open \"%s\" file.\n", (*files)[i + SCHEMA_COUNT]);
            fail();
        }

        ch1 = getc(f1);
        ch2 = getc(f2);
        while ((ch1 != EOF) && (ch2 != EOF) && (ch1 == ch2)) {
            ch1 = getc(f1);
            ch2 = getc(f2);
        }
        fclose(f1);
        fclose(f2);
        if (ch1 == ch2) {
            fprintf(stdout, "ok\n");
        } else {
            fprintf(stdout, "failed\n");
            fail();
        }
    }
}

static int
setup_files_yin(void **state)
{
    *state = &yin_files;
    return 0;
}

static int
setup_files_yang(void **state)
{
    *state = &yang_files;
    return 0;
}

static int
teardown_files(void **state)
{
    int i;

    (void)state; /* unused */
    chdir(SCHEMA_FOLDER_YIN);
    for (i = 0; i < SCHEMA_COUNT; ++i) {
        remove(yang_files[i]);
        remove(yin_files[i]);
    }
    chdir(SCHEMA_FOLDER_YANG);
    for (i = SCHEMA_COUNT; i < 2 * SCHEMA_COUNT; ++i) {
        remove(yang_files[i]);
        remove(yin_files[i]);
    }
    return 0;
}

int
main(void)
{
    const struct CMUnitTest cmut[] = {
        cmocka_unit_test_setup_teardown(test_modules, setup_ctx_yin, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_modules, setup_ctx_yang, teardown_ctx),
        cmocka_unit_test_setup_teardown(compare_modules, setup_files_yang, NULL),
        cmocka_unit_test_setup_teardown(compare_modules, setup_files_yin, NULL),
        cmocka_unit_test_setup_teardown(test_modules, setup_ctx_yin_trusted, teardown_ctx),
        cmocka_unit_test_setup_teardown(test_modules, setup_ctx_yang_trusted, teardown_ctx),
        cmocka_unit_test_setup_teardown(compare_modules, setup_files_yang, NULL),
        cmocka_unit_test_setup_teardown(compare_modules, setup_files_yin, NULL)
    };

    return cmocka_run_group_tests(cmut, NULL, teardown_files);
}
