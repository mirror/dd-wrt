/**
 * @file test_yanglib.c
 * @author: Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for ietf-yang-library data
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include <string.h>

#include "context.h"
#include "in.h"
#include "log.h"
#include "set.h"
#include "tests_config.h"
#include "tree_data.h"
#include "tree_schema.h"

const char *schema_a =
        "module a {\n"
        "  namespace urn:tests:a;\n"
        "  prefix a;\n"
        "  yang-version 1.1;\n"
        "\n"
        "  include a_sub;\n"
        "\n"
        "  list l2 {\n"
        "    key \"a\";\n"
        "    leaf a {\n"
        "      type uint16;\n"
        "    }\n"
        "    leaf b {\n"
        "      type uint16;\n"
        "    }\n"
        "  }\n"
        "}";
const char *schema_b =
        "module b {\n"
        "  namespace urn:tests:b;\n"
        "  prefix b;\n"
        "  yang-version 1.1;\n"
        "\n"
        "  import a {\n"
        "    prefix a;\n"
        "  }\n"
        "\n"
        "  deviation /a:l2 {\n"
        "    deviate add {\n"
        "      max-elements 40;\n"
        "    }\n"
        "  }\n"
        "\n"
        "  leaf foo {\n"
        "    type string;\n"
        "  }\n"
        "}";

static LY_ERR
test_imp_clb(const char *mod_name, const char *mod_rev, const char *submod_name, const char *sub_rev, void *user_data,
        LYS_INFORMAT *format, const char **module_data, void (**free_module_data)(void *model_data, void *user_data))
{
    const char *schema_a_sub =
            "submodule a_sub {\n"
            "    belongs-to a {\n"
            "        prefix a;\n"
            "    }\n"
            "    yang-version 1.1;\n"
            "\n"
            "    feature feat1;\n"
            "\n"
            "    list l3 {\n"
            "        key \"a\";\n"
            "        leaf a {\n"
            "            type uint16;\n"
            "        }\n"
            "        leaf b {\n"
            "            type uint16;\n"
            "        }\n"
            "    }\n"
            "}\n";

    assert_string_equal(mod_name, "a");
    assert_null(mod_rev);
    if (!submod_name) {
        return LY_ENOTFOUND;
    }
    assert_string_equal(submod_name, "a_sub");
    assert_null(sub_rev);
    assert_null(user_data);

    *format = LYS_IN_YANG;
    *module_data = schema_a_sub;
    *free_module_data = NULL;
    return LY_SUCCESS;
}

static void
test_yanglib(void **state)
{
    const char *feats[] = {"feat1", NULL};
    struct lyd_node *tree;
    struct ly_set *set;
    LY_ERR ret;

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, NULL);
    UTEST_ADD_MODULE(schema_a, LYS_IN_YANG, feats, NULL);
    UTEST_ADD_MODULE(schema_b, LYS_IN_YANG, NULL, NULL);

    assert_int_equal(LY_SUCCESS, ly_ctx_get_yanglib_data(UTEST_LYCTX, &tree, "<<%u>>", ly_ctx_get_change_count(UTEST_LYCTX)));
    lyd_free_all(tree);
    assert_int_equal(LY_SUCCESS, ly_ctx_get_yanglib_data(UTEST_LYCTX, &tree, "%u", -10));
    lyd_free_all(tree);
    assert_int_equal(LY_SUCCESS, ly_ctx_get_yanglib_data(UTEST_LYCTX, &tree, ""));
    lyd_free_all(tree);
    assert_int_equal(LY_SUCCESS, ly_ctx_get_yanglib_data(UTEST_LYCTX, &tree, "%u", ly_ctx_get_change_count(UTEST_LYCTX)));

    /* make sure there is "a" with a submodule and deviation */
    ret = lyd_find_xpath(tree, "/ietf-yang-library:yang-library/module-set/module[name='a'][submodule/name='a_sub']"
            "[feature='feat1'][deviation='b']", &set);
    assert_int_equal(ret, LY_SUCCESS);

    assert_int_equal(set->count, 1);
    ly_set_free(set, NULL);

    lyd_free_all(tree);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_yanglib),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
