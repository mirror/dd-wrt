/**
 * @file instanceid_keys.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief test for yang instance-identifier-keys type
 *
 * Copyright (c) 2022 CESNET, z.s.p.o.
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

/* LOCAL INCLUDE HEADERS */
#include "libyang.h"

#define MODULE_CREATE_YANG(MOD_NAME, NODES) \
    "module " MOD_NAME " {\n" \
    "  yang-version 1.1;\n" \
    "  namespace \"urn:tests:" MOD_NAME "\";\n" \
    "  prefix pref;\n" \
    NODES \
    "}\n"

#define TEST_SUCCESS_XML_NS1(MOD_NAME, NODE_NAME, PREFIX, NS, DATA, TYPE, ...) \
    { \
        struct lyd_node *tree; \
        const char *data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\" xmlns:" PREFIX "=\"" NS "\">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree); \
        CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 0, 0, 1, TYPE, __VA_ARGS__); \
        lyd_free_all(tree); \
    }

#define TEST_ERROR_XML(MOD_NAME, NODE_NAME, DATA) \
    {\
        struct lyd_node *tree; \
        const char *data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree); \
        assert_null(tree); \
    }

static void
test_data_xml(void **state)
{
    const char *schema;

    /* xml test */
    schema = MODULE_CREATE_YANG("defs", "import yang {prefix y;} leaf l1 {type y:instance-identifier-keys;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    TEST_SUCCESS_XML_NS1("defs", "l1", "px", "urn:tests:defs", "[px:key='val']", STRING, "[defs:key='val']");

    TEST_ERROR_XML("defs", "l1", "black");
    CHECK_LOG_CTX("Invalid first character 'b', list key predicates expected.", "Schema location \"/defs:l1\", line number 1.");

    TEST_ERROR_XML("defs", "l1", "[this is not a valid xpath]");
    CHECK_LOG_CTX("Invalid character 0x69 ('i'), perhaps \"this\" is supposed to be a function call.",
            "Schema location \"/defs:l1\", line number 1.");

    TEST_ERROR_XML("defs", "l1", "[px:key='val']");
    CHECK_LOG_CTX("Failed to resolve prefix \"px\".", "Schema location \"/defs:l1\", line number 1.");
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_data_xml),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
