/**
 * @file test_plugins.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from set.c
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include <stdlib.h>
#include <string.h>

#include "ly_config.h"
#include "plugins.h"
#include "plugins_internal.h"

const char *simple = "module libyang-plugins-simple {"
        "  namespace urn:libyang:tests:plugins:simple;"
        "  prefix s;"
        "  typedef note { type string; }"
        "  extension hint { argument value; }"
        "  leaf test {"
        "    type s:note {length 255;}"
        "    s:hint \"some hint here\";"
        "  }"
        "}";

static void
test_add_invalid(void **state)
{
    (void)state;
    assert_int_equal(LY_ESYS, lyplg_add(TESTS_BIN "/plugins/plugin_does_not_exist" LYPLG_SUFFIX));
}

static void
test_add_simple(void **state)
{
    struct lys_module *mod;
    struct lysc_node_leaf *leaf;
    struct lyplg_ext_record *record_e;
    struct lyplg_type *plugin_t;

    assert_int_equal(LY_SUCCESS, lyplg_add(TESTS_BIN "/plugins/plugin_simple" LYPLG_SUFFIX));

    UTEST_ADD_MODULE(simple, LYS_IN_YANG, NULL, &mod);

    leaf = (struct lysc_node_leaf *)mod->compiled->data;
    assert_int_equal(LYS_LEAF, leaf->nodetype);

    assert_non_null(plugin_t = lyplg_type_plugin_find(NULL, "libyang-plugins-simple", NULL, "note"));
    assert_string_equal("ly2 simple test v1", plugin_t->id);
    assert_ptr_equal(leaf->type->plugin, plugin_t);

    assert_int_equal(1, LY_ARRAY_COUNT(leaf->exts));
    assert_non_null(record_e = lyplg_ext_record_find(NULL, "libyang-plugins-simple", NULL, "hint"));
    assert_string_equal("ly2 simple test v1", record_e->plugin.id);
    assert_ptr_equal(leaf->exts[0].def->plugin, &record_e->plugin);

    /* the second loading of the same plugin - still success */
    assert_int_equal(LY_SUCCESS, lyplg_add(TESTS_BIN "/plugins/plugin_simple" LYPLG_SUFFIX));
}

static void
test_not_implemented(void **state)
{
    struct lys_module *mod;
    struct lyd_node *tree;
    const char *schema = "module libyang-plugins-unknown {"
            "  namespace urn:libyang:tests:plugins:unknown;"
            "  prefix u;"
            "  extension myext;"
            "  typedef mytype { type string;}"
            "  leaf test {"
            "    u:myext;"
            "    type mytype;"
            "  }"
            "}";
    const char *data = "<test xmlns=\"urn:libyang:tests:plugins:unknown\">xxx</test>";
    char *printed = NULL;

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG_COMPILED, 0));
    free(printed);

    assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &tree));
    CHECK_LOG_CTX(NULL, NULL, 0);

    lyd_free_all(tree);
}

static LY_ERR
parse_clb(struct lysp_ctx *UNUSED(pctx), struct lysp_ext_instance *ext)
{
    struct lysp_node_leaf *leaf;

    leaf = (struct lysp_node_leaf *)ext->parent;
    leaf->flags |= LYS_STATUS_OBSLT;
    return LY_SUCCESS;
}

struct lyplg_ext_record memory_recs[] = {
    {
        .module = "libyang-plugins-simple",
        .revision = NULL,
        .name = "hint",

        .plugin.id = "memory-plugin-v1",
        .plugin.parse = parse_clb,
        .plugin.compile = NULL,
        .plugin.printer_info = NULL,
        .plugin.node = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = NULL,
        .plugin.cfree = NULL
    },
    {0} /* terminating zeroed item */
};

static void
test_simple_from_memory(void **state)
{
    struct lys_module *mod;
    struct lysc_node_leaf *leaf;

    lyplg_add_extension_plugin(UTEST_LYCTX, LYPLG_EXT_API_VERSION, memory_recs);
    UTEST_ADD_MODULE(simple, LYS_IN_YANG, NULL, &mod);

    leaf = (struct lysc_node_leaf *)mod->compiled->data;
    assert_int_equal(LYS_LEAF, leaf->nodetype);
    assert_true(leaf->flags & LYS_STATUS_OBSLT);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_add_invalid),
        UTEST(test_add_simple),
        UTEST(test_not_implemented),
        UTEST(test_simple_from_memory),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
