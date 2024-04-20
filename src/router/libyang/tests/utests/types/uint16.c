/**
 * @file uint16.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief test for uint16 values
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

/* INCLUDE UTEST HEADER */
#define  _UTEST_MAIN_
#include "../utests.h"

/* GLOBAL INCLUDE HEADERS */
#include <ctype.h>

/* LOCAL INCLUDE HEADERS */
#include "libyang.h"
#include "path.h"
#include "plugins_internal.h"

#define MODULE_CREATE_YANG(MOD_NAME, NODES) \
    "module " MOD_NAME " {\n" \
    "  yang-version 1.1;\n" \
    "  namespace \"urn:tests:" MOD_NAME "\";\n" \
    "  prefix pref;\n" \
    NODES \
    "}\n"

#define TEST_SUCCESS_XML(MOD_NAME, DATA, TYPE, ...) \
    { \
        struct lyd_node *tree; \
        const char *data = "<port xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</port>"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree); \
        CHECK_LYSC_NODE(tree->schema, NULL, 0, 0x5, 1, "port", 0, LYS_LEAF, 0, 0, 0, 0); \
        CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 0, 0, 1, TYPE, ## __VA_ARGS__); \
        lyd_free_all(tree); \
    }

#define TEST_ERROR_XML(MOD_NAME, DATA) \
    {\
        struct lyd_node *tree; \
        const char *data = "<port xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</port>"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree); \
        assert_null(tree); \
    }

static void
test_data_xml(void **state)
{
    const char *schema;

    /* xml test */
    schema = MODULE_CREATE_YANG("defs", "leaf port {type uint16 {range 150..200;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    TEST_ERROR_XML("defs", "\n 1500 \t\n  ");
    CHECK_LOG_CTX("Unsatisfied range - value \"1500\" is out of the allowed range.", "/defs:port", 3);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_data_xml),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
