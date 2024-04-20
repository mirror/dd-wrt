/*
 * @file test_printer_tree.c
 * @author: Adam Piecek <piecek@cesnet.cz>
 * @brief unit tests for functions from printer_tree.c
 *
 * Copyright (c) 2019-2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include "context.h"
#include "ly_common.h"
#include "out.h"
#include "printer_schema.h"
#include "tree_schema.h"

#define TEST_LOCAL_SETUP \
    char *printed; \
    struct lys_module *mod; \
    const char *orig; \
    const char *expect; \
    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&printed, 0, &UTEST_OUT));

#define TEST_LOCAL_PRINT(MOD, LINE_LENGTH) \
    assert_int_equal(LY_SUCCESS, lys_print_module(UTEST_OUT, MOD, LYS_OUT_TREE, LINE_LENGTH, 0));

#define TEST_LOCAL_TEARDOWN \
    ly_out_free(UTEST_OUT, NULL, 1);

static void
base_sections(void **state)
{
    TEST_LOCAL_SETUP;
    struct lys_module *modxx;

    orig =
            "module a01xx {\n"
            "  yang-version 1.1;\n"
            "  namespace \"xx:y\";\n"
            "  prefix xx;\n"
            "  container c;\n"
            "  container d;\n"
            "}\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &modxx);

    /* module with import statement */
    orig =
            "module a01 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "\n"
            "  import a01xx {\n"
            "    prefix xx;\n"
            "  }\n"
            "\n"
            "  grouping g1;\n"
            "\n"
            "  grouping g2;\n"
            "  container g;\n"
            "  augment \"/xx:c\" {\n"
            "    container e;\n"
            "  }\n"
            "  augment \"/xx:d\" {\n"
            "    container f;\n"
            "  }\n"
            "  rpc rpc1;\n"
            "  rpc rpc2;\n"
            "  notification n1;\n"
            "  notification n2;\n"
            "}\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);

    /* from pyang */
    expect =
            "module: a01\n"
            "  +--rw g\n"
            "\n"
            "  augment /xx:c:\n"
            "    +--rw e\n"
            "  augment /xx:d:\n"
            "    +--rw f\n"
            "\n"
            "  rpcs:\n"
            "    +---x rpc1\n"
            "    +---x rpc2\n"
            "\n"
            "  notifications:\n"
            "    +---n n1\n"
            "    +---n n2\n"
            "\n"
            "  grouping g1\n"
            "  grouping g2\n";

    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);

    /* from pyang */
    expect =
            "module: a01\n"
            "  +--rw g\n"
            "\n"
            "  augment /xx:c:\n"
            "    +--rw e\n"
            "  augment /xx:d:\n"
            "    +--rw f\n"
            "\n"
            "  rpcs:\n"
            "    +---x rpc1\n"
            "    +---x rpc2\n"
            "\n"
            "  notifications:\n"
            "    +---n n1\n"
            "    +---n n2\n";

    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);

    /* from pyang */
    expect =
            "module: a01xx\n"
            "  +--rw c\n"
            "  |  +--rw x:e\n"
            "  +--rw d\n"
            "     +--rw x:f\n";

    TEST_LOCAL_PRINT(modxx, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
node_status(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a02 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "  container l {\n"
            "    status current;\n"
            "  }\n"
            "  container m {\n"
            "    status deprecated;\n"
            "  }\n"
            "  container n {\n"
            "    status obsolete;\n"
            "  }\n"
            "}\n";

    /* from pyang */
    expect =
            "module: a02\n"
            "  +--rw l\n"
            "  x--rw m\n"
            "  o--rw n\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
node_config_flags(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a03 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "  container l {\n"
            "    config true;\n"
            "  }\n"
            "  container m {\n"
            "    config false;\n"
            "  }\n"
            "}\n";

    /* from pyang */
    expect =
            "module: a03\n"
            "  +--rw l\n"
            "  +--ro m\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
node_rpcs_flags(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a04 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "  container cont {\n"
            "    action rpc1 {\n"
            "\n"
            "      input {\n"
            "        leaf in {\n"
            "          type string;\n"
            "        }\n"
            "      }\n"
            "\n"
            "      output {\n"
            "        leaf out {\n"
            "          type string;\n"
            "        }\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "}\n";

    /* from pyang */
    expect =
            "module: a04\n"
            "  +--rw cont\n"
            "     +---x rpc1\n"
            "        +---w input\n"
            "        |  +---w in?   string\n"
            "        +--ro output\n"
            "           +--ro out?   string\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
node_grouping_flags(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a05 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "\n"
            "  grouping g {\n"
            "    leaf a {\n"
            "      type string;\n"
            "      config true;\n"
            "    }\n"
            "    leaf b {\n"
            "      type string;\n"
            "      config false;\n"
            "    }\n"
            "    leaf c {\n"
            "      type string;\n"
            "    }\n"
            "    container d {\n"
            "      config false;\n"
            "      leaf e {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "    container f {\n"
            "      leaf g {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "  container d {\n"
            "    uses g;\n"
            "  }\n"
            "}\n";

    /* from yanglint1 */
    expect =
            "module: a05\n"
            "  +--rw d\n"
            "     +---u g\n"
            "\n"
            "  grouping g:\n"
            "    +--rw a?   string\n"
            "    +--ro b?   string\n"
            "    +---- c?   string\n"
            "    +--ro d\n"
            "    |  +--ro e?   string\n"
            "    +---- f\n"
            "       +---- g?   string\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    /* from pyang */
    expect =
            "module: a05\n"
            "  +--rw d\n"
            "     +--rw a?   string\n"
            "     +--ro b?   string\n"
            "     +--rw c?   string\n"
            "     +--ro d\n"
            "     |  +--ro e?   string\n"
            "     +--rw f\n"
            "        +--rw g?   string\n";

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
notif_inside_container(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a06 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "  container c {\n"
            "    notification notif;\n"
            "  }\n"
            "}\n";

    /* from pyang */
    expect =
            "module: a06\n"
            "  +--rw c\n"
            "     +---n notif\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
node_choice(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a07 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "  choice my_choice;\n"
            "}\n";

    /* from pyang */
    expect =
            "module: a07\n"
            "  +--rw (my_choice)?\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
node_case(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a08 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "\n"
            "  feature foo;\n"
            "  choice my_choice {\n"
            "    case my_case;\n"
            "  }\n"
            "  choice shorthand {\n"
            "    container cont1 {\n"
            "      if-feature \"foo\";\n"
            "      status obsolete;\n"
            "    }\n"
            "    container cont2 {\n"
            "      container cont3;\n"
            "    }\n"
            "  }\n"
            "  container top {\n"
            "    choice shorthand1 {\n"
            "      container cont1;\n"
            "    }\n"
            "    choice shorthand2 {\n"
            "      container cont2 {\n"
            "        container cont3;\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "}\n";

    /* from pyang */
    expect =
            "module: a08\n"
            "  +--rw (my_choice)?\n"
            "  |  +--:(my_case)\n"
            "  +--rw (shorthand)?\n"
            "  |  o--:(cont1)\n"
            "  |  |  o--rw cont1 {foo}?\n"
            "  |  +--:(cont2)\n"
            "  |     +--rw cont2\n"
            "  |        +--rw cont3\n"
            "  +--rw top\n"
            "     +--rw (shorthand1)?\n"
            "     |  +--:(cont1)\n"
            "     |     +--rw cont1\n"
            "     +--rw (shorthand2)?\n"
            "        +--:(cont2)\n"
            "           +--rw cont2\n"
            "              +--rw cont3\n";

    const char *feats[] = {"foo", NULL};

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, feats, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
optional_opts(void **state)
{
    TEST_LOCAL_SETUP;
    /* throws libyang warn: Use of anydata to define configuration data is not recommended... */
    orig =
            "module a09 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "  leaf l1 {\n"
            "    type string;\n"
            "    mandatory true;\n"
            "  }\n"
            "  leaf l2 {\n"
            "    type string;\n"
            "    mandatory false;\n"
            "  }\n"
            "  choice c1 {\n"
            "    mandatory true;\n"
            "  }\n"
            "  choice c2 {\n"
            "    mandatory false;\n"
            "  }\n"
            "  anydata a1 {\n"
            "    mandatory true;\n"
            "  }\n"
            "  anydata a2 {\n"
            "    mandatory false;\n"
            "  }\n"
            "  anyxml x1 {\n"
            "    mandatory true;\n"
            "  }\n"
            "  anyxml x2 {\n"
            "    mandatory false;\n"
            "  }\n"
            "}\n";

    /* from yanglint 1 */
    expect =
            "module: a09\n"
            "  +--rw l1      string\n"
            "  +--rw l2?     string\n"
            "  +--rw (c1)\n"
            "  +--rw (c2)?\n"
            "  +--rw a1      anydata\n"
            "  +--rw a2?     anydata\n"
            "  +--rw x1      anyxml\n"
            "  +--rw x2?     anyxml\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
presence_container(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a10 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "  container c;\n"
            "  container d {\n"
            "    presence \"str1\";\n"
            "  }\n"
            "}\n";

    /* from pyang */
    expect =
            "module: a10\n"
            "  +--rw c\n"
            "  +--rw d!\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
node_keys(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a11 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "  list l1 {\n"
            "    key \"a\";\n"
            "    leaf a {\n"
            "      type string;\n"
            "    }\n"
            "  }\n"
            "  list l2 {\n"
            "    key \"a b\";\n"
            "    leaf a {\n"
            "      type string;\n"
            "    }\n"
            "    leaf b {\n"
            "      type string;\n"
            "    }\n"
            "  }\n"
            "  leaf-list ll {\n"
            "    type string;\n"
            "  }\n"
            "}\n";

    /* from pyang */
    expect =
            "module: a11\n"
            "  +--rw l1* [a]\n"
            "  |  +--rw a    string\n"
            "  +--rw l2* [a b]\n"
            "  |  +--rw a    string\n"
            "  |  +--rw b    string\n"
            "  +--rw ll*   string\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
node_type_target(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a12 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "  leaf a {\n"
            "    type leafref {\n"
            "      path \"/x:b\";\n"
            "    }\n"
            "  }\n"
            "  leaf b {\n"
            "    type string;\n"
            "  }\n"
            "}\n";

    /* from yanglint 1 */
    expect =
            "module: a12\n"
            "  +--rw a?   -> /x:b\n"
            "  +--rw b?   string\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
node_type_leafref(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a13 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "  leaf pretty-long-identifier-name-which-should-exceed-the-limit-of-72-characters {\n"
            "    type string;\n"
            "  }\n"
            "  leaf a {\n"
            "    type leafref {\n"
            "      path \"/x:pretty-long-identifier-name-which-should-exceed-the-limit-of-72-characters\";\n"
            "    }\n"
            "  }\n"
            "}\n";

    /* yanglint --tree-no-leafref-target --tree-line-length=72 */
    expect =
            "module: a13\n"
            "  +--rw pretty-long-identifier-name-which-should-exceed-the-limit-of-72-characters?\n"
            "  |       string\n"
            "  +--rw a?\n"
            "          leafref\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
node_iffeatures(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a14 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "\n"
            "  feature foo;\n"
            "\n"
            "  feature bar;\n"
            "  container c {\n"
            "    if-feature \"foo or bar\";\n"
            "  }\n"
            "}\n";

    /* from pyang */
    expect =
            "module: a14\n"
            "  +--rw c {foo or bar}?\n";

    const char *feats[] = {"foo", NULL};

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, feats, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
indent_wrapper(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a15 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "  container a {\n"
            "    container b;\n"
            "  }\n"
            "  container c {\n"
            "    container d {\n"
            "      container e;\n"
            "    }\n"
            "    container f {\n"
            "      container g;\n"
            "    }\n"
            "  }\n"
            "  container h;\n"
            "  container i {\n"
            "    container j;\n"
            "    container k;\n"
            "  }\n"
            "}\n";

    /* from pyang */
    expect =
            "module: a15\n"
            "  +--rw a\n"
            "  |  +--rw b\n"
            "  +--rw c\n"
            "  |  +--rw d\n"
            "  |  |  +--rw e\n"
            "  |  +--rw f\n"
            "  |     +--rw g\n"
            "  +--rw h\n"
            "  +--rw i\n"
            "     +--rw j\n"
            "     +--rw k\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
line_length_twiddling(void **state)
{
    TEST_LOCAL_SETUP;
    /* node_fits_tight */

    orig =
            "module a16 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "\n"
            "  feature f;\n"
            "\n"
            "  typedef some-long-type {\n"
            "    type string;\n"
            "  }\n"
            "  list my-list-name {\n"
            "    key \"key\";\n"
            "    leaf key {\n"
            "      type string;\n"
            "    }\n"
            "    leaf nod-leaf {\n"
            "      if-feature \"f\";\n"
            "      type some-long-type;\n"
            "    }\n"
            "    leaf nos-leaf {\n"
            "      if-feature \"f\";\n"
            "      type int32;\n"
            "    }\n"
            "  }\n"
            "}\n";

    /* pyang --tree-line-length 42 */
    expect =
            "module: a16\n"
            "  +--rw my-list-name* [key]\n"
            "     +--rw key         string\n"
            "     +--rw nod-leaf?   some-long-type {f}?\n"
            "     +--rw nos-leaf?   int32 {f}?\n";

    const char *feats[] = {"f", NULL};

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, feats, &mod);
    TEST_LOCAL_PRINT(mod, 42);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 42);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    ly_out_reset(UTEST_OUT);
    /* break_before_iffeature */

    /* pyang --tree-line-length 41 */
    expect =
            "module: a16\n"
            "  +--rw my-list-name* [key]\n"
            "     +--rw key         string\n"
            "     +--rw nod-leaf?   some-long-type\n"
            "     |       {f}?\n"
            "     +--rw nos-leaf?   int32 {f}?\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, feats, &mod);
    TEST_LOCAL_PRINT(mod, 41);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 41);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    ly_out_reset(UTEST_OUT);
    /* break_before_type */

    /* pyang --tree-line-length 29 */
    expect =
            "module: a16\n"
            "  +--rw my-list-name* [key]\n"
            "     +--rw key         string\n"
            "     +--rw nod-leaf?\n"
            "     |       some-long-type\n"
            "     |       {f}?\n"
            "     +--rw nos-leaf?   int32\n"
            "             {f}?\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, feats, &mod);
    TEST_LOCAL_PRINT(mod, 29);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 29);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    ly_out_reset(UTEST_OUT);
    /* break_before_keys */

    /* pyang --tree-line-length 23 */
    expect =
            "module: a16\n"
            "  +--rw my-list-name*\n"
            "          [key]\n"
            "     +--rw key\n"
            "     |       string\n"
            "     +--rw nod-leaf?\n"
            "     |       some-long-type\n"
            "     |       {f}?\n"
            "     +--rw nos-leaf?\n"
            "             int32 {f}?\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, feats, &mod);
    TEST_LOCAL_PRINT(mod, 23);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 23);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    ly_out_reset(UTEST_OUT);
    /* every_node_name_is_too_long */

    /* pyang --tree-line-length 14 */
    expect =
            "module: a16\n"
            "  +--rw my-list-name*\n"
            "          [key]\n"
            "     +--rw key\n"
            "     |       string\n"
            "     +--rw nod-leaf?\n"
            "     |       some-long-type\n"
            "     |       {f}?\n"
            "     +--rw nos-leaf?\n"
            "             int32\n"
            "             {f}?\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, feats, &mod);
    TEST_LOCAL_PRINT(mod, 14);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 14);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
break_before_leafref(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a17 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "  leaf e {\n"
            "    type string;\n"
            "  }\n"
            "  leaf abcd {\n"
            "    type leafref {\n"
            "      path \"/x:e\";\n"
            "    }\n"
            "  }\n"
            "}\n";

    /* yanglint --tree-line-length 14 */
    expect =
            "module: a17\n"
            "  +--rw e?\n"
            "  |       string\n"
            "  +--rw abcd?\n"
            "          -> /x:e\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 14);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 14);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
break_before_leafref_and_iffeature(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a18 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "\n"
            "  feature f;\n"
            "  leaf some-long-id {\n"
            "    type string;\n"
            "  }\n"
            "  leaf a {\n"
            "    if-feature \"f\";\n"
            "    type leafref {\n"
            "      path \"/x:some-long-id\";\n"
            "    }\n"
            "  }\n"
            "}\n";

    /*  yanglint --tree-no-leafref-target --tree-line-length=20 */
    expect =
            "module: a18\n"
            "  +--rw some-long-id?\n"
            "  |       string\n"
            "  +--rw a?\n"
            "          leafref\n"
            "          {f}?\n";

    const char *feats[] = {"f", NULL};

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, feats, &mod);
    TEST_LOCAL_PRINT(mod, 20);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 20);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
basic_unified_indent_before_type(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a19 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "\n"
            "  typedef longType {\n"
            "    type string;\n"
            "  }\n"
            "  container A {\n"
            "    leaf Bnode {\n"
            "      type int8;\n"
            "    }\n"
            "    leaf Cnode {\n"
            "      type int8;\n"
            "      mandatory true;\n"
            "    }\n"
            "    leaf Dnode {\n"
            "      type int8;\n"
            "      mandatory true;\n"
            "    }\n"
            "    leaf E {\n"
            "      type longType;\n"
            "      mandatory true;\n"
            "    }\n"
            "    leaf G {\n"
            "      type int8;\n"
            "    }\n"
            "  }\n"
            "}\n";

    /* from pyang */
    expect =
            "module: a19\n"
            "  +--rw A\n"
            "     +--rw Bnode?   int8\n"
            "     +--rw Cnode    int8\n"
            "     +--rw Dnode    int8\n"
            "     +--rw E        longType\n"
            "     +--rw G?       int8\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
twiddling_unified_indent_before_type(void **state)
{
    TEST_LOCAL_SETUP;
    /* basic_functionality */

    orig =
            "module a20 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "\n"
            "  typedef longType {\n"
            "    type string;\n"
            "  }\n"
            "  container A {\n"
            "    leaf Bnode {\n"
            "      type int8;\n"
            "    }\n"
            "    leaf CnodeIsBigger {\n"
            "      type int8;\n"
            "      mandatory true;\n"
            "    }\n"
            "    leaf Dnode {\n"
            "      type int8;\n"
            "      mandatory true;\n"
            "    }\n"
            "    leaf E {\n"
            "      type longType;\n"
            "      mandatory true;\n"
            "    }\n"
            "    leaf G {\n"
            "      type int8;\n"
            "    }\n"
            "  }\n"
            "}\n";

    /* pyang --tree-line-length 36 */
    expect =
            "module: a20\n"
            "  +--rw A\n"
            "     +--rw Bnode?           int8\n"
            "     +--rw CnodeIsBigger    int8\n"
            "     +--rw Dnode            int8\n"
            "     +--rw E                longType\n"
            "     +--rw G?               int8\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 36);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 36);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    ly_out_reset(UTEST_OUT);
    /* unified_indent_before_type_long_node_name */

    /* pyang --tree-line-length 32 */
    expect =
            "module: a20\n"
            "  +--rw A\n"
            "     +--rw Bnode?           int8\n"
            "     +--rw CnodeIsBigger    int8\n"
            "     +--rw Dnode            int8\n"
            "     +--rw E\n"
            "     |       longType\n"
            "     +--rw G?               int8\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 32);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 32);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    ly_out_reset(UTEST_OUT);
    /* unified_indent_before_type_long_node_type */

    /* pyang --tree-line-length 31 */
    expect =
            "module: a20\n"
            "  +--rw A\n"
            "     +--rw Bnode?\n"
            "     |       int8\n"
            "     +--rw CnodeIsBigger\n"
            "     |       int8\n"
            "     +--rw Dnode\n"
            "     |       int8\n"
            "     +--rw E\n"
            "     |       longType\n"
            "     +--rw G?\n"
            "             int8\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 31);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 31);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
inheritance_of_config_flag(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a21 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "  container a {\n"
            "    config false;\n"
            "    leaf b {\n"
            "      type string;\n"
            "    }\n"
            "  }\n"
            "}\n";

    /* from pyang */
    expect =
            "module: a21\n"
            "  +--ro a\n"
            "     +--ro b?   string\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
inheritance_of_status_flag(void **state)
{
    TEST_LOCAL_SETUP;
    /* throws libyang warn: Missing explicit "..." status that was already specified in parent, inheriting. */
    orig =
            "module a22 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "  container a {\n"
            "    status current;\n"
            "    container b {\n"
            "      status deprecated;\n"
            "      leaf f {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "    container c {\n"
            "      status obsolete;\n"
            "      leaf g {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "  container d {\n"
            "    status deprecated;\n"
            "    container h {\n"
            "      status obsolete;\n"
            "      leaf e {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "}\n";

    /* from yanglint 1 */
    expect =
            "module: a22\n"
            "  +--rw a\n"
            "  |  x--rw b\n"
            "  |  |  x--rw f?   string\n"
            "  |  o--rw c\n"
            "  |     o--rw g?   string\n"
            "  x--rw d\n"
            "     o--rw h\n"
            "        o--rw e?   string\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
key_leaf_is_always_mandatory_true(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a23 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "  list a {\n"
            "    key \"k1\";\n"
            "    list b {\n"
            "      key \"k2\";\n"
            "      leaf k1 {\n"
            "        type string;\n"
            "      }\n"
            "      leaf k2 {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "    leaf k1 {\n"
            "      type string;\n"
            "    }\n"
            "  }\n"
            "}\n";

    /* from pyang */
    expect =
            "module: a23\n"
            "  +--rw a* [k1]\n"
            "     +--rw b* [k2]\n"
            "     |  +--rw k1?   string\n"
            "     |  +--rw k2    string\n"
            "     +--rw k1    string\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);

    /* from pyang but with some swapped lines */
    expect =
            "module: a23\n"
            "  +--rw a* [k1]\n"
            "     +--rw k1    string\n"
            "     +--rw b* [k2]\n"
            "        +--rw k2    string\n"
            "        +--rw k1?   string\n";

    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
transition_between_rpc_and_notif(void **state)
{
    TEST_LOCAL_SETUP;
    orig =
            "module a24 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "  container top {\n"
            "    leaf g {\n"
            "      type string;\n"
            "    }\n"
            "    action rpc1 {\n"
            "\n"
            "      input {\n"
            "        leaf in {\n"
            "          type string;\n"
            "        }\n"
            "      }\n"
            "    }\n"
            "    action rpc2 {\n"
            "\n"
            "      input {\n"
            "        leaf in {\n"
            "          type string;\n"
            "        }\n"
            "      }\n"
            "\n"
            "      output {\n"
            "        leaf out {\n"
            "          type string;\n"
            "        }\n"
            "      }\n"
            "    }\n"
            "    notification n1;\n"
            "    notification n2;\n"
            "  }\n"
            "}\n";

    /* from pyang */
    expect =
            "module: a24\n"
            "  +--rw top\n"
            "     +--rw g?      string\n"
            "     +---x rpc1\n"
            "     |  +---w input\n"
            "     |     +---w in?   string\n"
            "     +---x rpc2\n"
            "     |  +---w input\n"
            "     |  |  +---w in?   string\n"
            "     |  +--ro output\n"
            "     |     +--ro out?   string\n"
            "     +---n n1\n"
            "     +---n n2\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);
    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
local_augment(void **state)
{
    TEST_LOCAL_SETUP;

    orig =
            "module a25 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "  container g;\n"
            "  augment \"/x:g\" {\n"
            "    container e;\n"
            "  }\n"
            "}\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);

    /* from pyang */
    expect =
            "module: a25\n"
            "  +--rw g\n"
            "     +--rw e\n";

    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
print_compiled_node(void **state)
{
    TEST_LOCAL_SETUP;
    const struct lysc_node *node;

    orig =
            "module a26 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "\n"
            "  container g {\n"
            "    leaf a {\n"
            "      type string;\n"
            "    }\n"
            "    container h {\n"
            "      leaf b {\n"
            "        type string;\n"
            "        mandatory true;\n"
            "      }\n"
            "      leaf c {\n"
            "        type string;\n"
            "      }\n"
            "      list l {\n"
            "        key \"ip\";\n"
            "        leaf ip {\n"
            "          type string;\n"
            "        }\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "}\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);

    /* pyang -f tree --tree-path /g/h/c */
    expect =
            "module: a26\n"
            "  +--rw g\n"
            "     +--rw h\n"
            "        +--rw c?   string\n";

    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    node = lys_find_path(UTEST_LYCTX, NULL, "/a26:g/h/c", 0);
    CHECK_POINTER(node, 1);
    assert_int_equal(LY_SUCCESS, lys_print_node(UTEST_OUT, node, LYS_OUT_TREE, 72, 0));
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);

    /* pyang -f tree --tree-path /g/h/l */
    expect =
            "module: a26\n"
            "  +--rw g\n"
            "     +--rw h\n"
            "        +--rw l* [ip]\n"
            "           +--rw ip    string\n";

    node = lys_find_path(UTEST_LYCTX, NULL, "/a26:g/h/l", 0);
    CHECK_POINTER(node, 1);
    assert_int_equal(LY_SUCCESS, lys_print_node(UTEST_OUT, node, LYS_OUT_TREE, 72, 0));
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);

    /* pyang -f tree --tree-path /g/h */
    expect =
            "module: a26\n"
            "  +--rw g\n"
            "     +--rw h\n"
            "        +--rw b    string\n"
            "        +--rw c?   string\n"
            "        +--rw l* [ip]\n"
            "           +--rw ip    string\n";

    node = lys_find_path(UTEST_LYCTX, NULL, "/a26:g/h", 0);
    CHECK_POINTER(node, 1);
    assert_int_equal(LY_SUCCESS, lys_print_node(UTEST_OUT, node, LYS_OUT_TREE, 72, 0));
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);

    /* pyang whose output is adjusted manually */
    expect =
            "module: a26\n"
            "  +--rw g\n"
            "     +--rw h\n";

    node = lys_find_path(UTEST_LYCTX, NULL, "/a26:g/h", 0);
    CHECK_POINTER(node, 1);
    assert_int_equal(LY_SUCCESS, lys_print_node(UTEST_OUT, node, LYS_OUT_TREE, 72, LYS_PRINT_NO_SUBSTMT));
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
print_compiled_node_augment(void **state)
{
    TEST_LOCAL_SETUP;
    const struct lysc_node *node;

    orig =
            "module b26xx {\n"
            "  yang-version 1.1;\n"
            "  namespace \"xx:y\";\n"
            "  prefix xx;\n"
            "  container c;\n"
            "}\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);

    /* module with import statement */
    orig =
            "module b26 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "\n"
            "  import b26xx {\n"
            "    prefix xx;\n"
            "  }\n"
            "\n"
            "  augment \"/xx:c\" {\n"
            "    container e;\n"
            "  }\n"
            "}\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);

    /* pyang -f tree --tree-path /c/e ... but prefixes modified */
    expect =
            "module: b26\n"
            "  +--rw xx:c\n"
            "     +--rw e\n";

    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    node = lys_find_path(UTEST_LYCTX, NULL, "/b26xx:c/b26:e", 0);
    CHECK_POINTER(node, 1);
    assert_int_equal(LY_SUCCESS, lys_print_node(UTEST_OUT, node, LYS_OUT_TREE, 72, 0));
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_out_reset(UTEST_OUT);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static LY_ERR
local_imp_clb(const char *UNUSED(mod_name), const char *UNUSED(mod_rev), const char *UNUSED(submod_name),
        const char *UNUSED(sub_rev), void *user_data, LYS_INFORMAT *format,
        const char **module_data, void (**free_module_data)(void *model_data, void *user_data))
{
    *module_data = user_data;
    *format = LYS_IN_YANG;
    *free_module_data = NULL;
    return LY_SUCCESS;
}

static void
print_parsed_submodule(void **state)
{
    TEST_LOCAL_SETUP;

    orig = "module a27 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "\n"
            "  include \"a27-sub\";\n"
            "}\n";

    char *submodule =
            "submodule a27-sub {\n"
            "  yang-version 1.1;\n"
            "  belongs-to a27 {\n"
            "    prefix x;\n"
            "  }\n"
            "\n"
            "  container h {\n"
            "    leaf b {\n"
            "      type string;\n"
            "    }\n"
            "  }\n"
            "}\n";

    /* edited pyang output */
    expect =
            "submodule: a27-sub\n"
            "  +--rw h\n"
            "     +--rw b?   string\n";

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, local_imp_clb, submodule);
    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    assert_int_equal(LY_SUCCESS, lys_print_submodule(UTEST_OUT, mod->parsed->includes[0].submodule, LYS_OUT_TREE, 72, 0));
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    TEST_LOCAL_TEARDOWN;
}

static void
yang_data(void **state)
{
    TEST_LOCAL_SETUP;

    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));
    assert_non_null(ly_ctx_load_module(UTEST_LYCTX, "ietf-restconf", "2017-01-26", NULL));

    orig =
            "module a28 {\n"
            "  yang-version 1.1;\n"
            "  namespace \"x:y\";\n"
            "  prefix x;\n"
            "\n"
            "  import ietf-restconf {\n"
            "    prefix rc;\n"
            "    revision-date 2017-01-26;\n"
            "  }\n"
            "\n"
            "  rc:yang-data \"tmp1\" {\n"
            "    container cont1 {\n"
            "      leaf lf {\n"
            "        type string;\n"
            "      }\n"
            "      list l2 {\n"
            "        key\n"
            "          \"a b\";\n"
            "        leaf a {\n"
            "          type string;\n"
            "        }\n"
            "        leaf b {\n"
            "          type string;\n"
            "        }\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "  rc:yang-data \"tmp2\" {\n"
            "    container con2 {\n"
            "      leaf lf {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "  rc:yang-data \"tmp3\" {\n"
            "    uses g1;\n"
            "    uses g2;\n"
            "  }\n"
            "  rc:yang-data \"tmp4\" {\n"
            "    choice x {\n"
            "      case a {\n"
            "        container z;\n"
            "      }\n"
            "      case b {\n"
            "        container y;\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "\n"
            "  grouping g1 {\n"
            "    description\n"
            "      \"Some Text\";\n"
            "  }\n"
            "\n"
            "  grouping g2 {\n"
            "    container cont3;\n"
            "  }\n"
            "  container mont;\n"
            "}\n";

    /* from pyang (--tree-print-yang-data --tree-print-groupings -p "...")
     * but with these adjustments:
     *  - <flags> is always '--' for yang-data nodes
     *  - yang-data tmp3 has two 'uses' nodes
     *  - grouping g2 has ':' character at the end
     */
    expect =
            "module: a28\n"
            "  +--rw mont\n"
            "\n"
            "  grouping g1\n"
            "  grouping g2:\n"
            "    +---- cont3\n"
            "\n"
            "  yang-data tmp1:\n"
            "    +---- cont1\n"
            "       +---- lf?   string\n"
            "       +---- l2* [a b]\n"
            "          +---- a    string\n"
            "          +---- b    string\n"
            "  yang-data tmp2:\n"
            "    +---- con2\n"
            "       +---- lf?   string\n"
            "  yang-data tmp3:\n"
            "    +---u g1\n"
            "    +---u g2\n"
            "  yang-data tmp4:\n"
            "    +---- (x)?\n"
            "       +--:(a)\n"
            "       |  +---- z\n"
            "       +--:(b)\n"
            "          +---- y\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);

    ly_out_reset(UTEST_OUT);

    /* from pyang (--tree-print-yang-data -p "...")
     * but with these adjustments:
     *  <flags> is always '--' for yang-data nodes
     */
    expect =
            "module: a28\n"
            "  +--rw mont\n"
            "\n"
            "  yang-data tmp1:\n"
            "    +---- cont1\n"
            "       +---- lf?   string\n"
            "       +---- l2* [a b]\n"
            "          +---- a    string\n"
            "          +---- b    string\n"
            "  yang-data tmp2:\n"
            "    +---- con2\n"
            "       +---- lf?   string\n"
            "  yang-data tmp3:\n"
            "    +---- cont3\n"
            "  yang-data tmp4:\n"
            "    +---- (x)?\n"
            "       +--:(a)\n"
            "       |  +---- z\n"
            "       +--:(b)\n"
            "          +---- y\n";

    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static LY_ERR
getter(const struct lysc_ext_instance *ext, void *user_data, void **ext_data, ly_bool *ext_data_free)
{
    struct ly_ctx *ctx;
    struct lyd_node *data = NULL;

    ctx = ext->module->ctx;
    if (user_data) {
        assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(ctx, user_data, LYD_XML, 0, LYD_VALIDATE_PRESENT, &data));
    }

    *ext_data = data;
    *ext_data_free = 1;
    return LY_SUCCESS;
}

#define SM_MODNAME_EXT "sm-extension"
#define SM_MOD_EXT_NAMESPACE "urn:sm-ext"
#define SM_PREF "sm"
#define SCHEMA_REF_INLINE "<inline></inline>"
#define SCHEMA_REF_SHARED(REF) "<shared-schema>"REF"</shared-schema>"

#define EXT_DATA(MPMOD_NAME, MODULES, SCHEMA_REF) \
    "<yang-library xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\"\n" \
    "   xmlns:ds=\"urn:ietf:params:xml:ns:yang:ietf-datastores\">\n" \
    "<module-set>\n" \
    "   <name>test-set</name>\n" \
    "   <module>\n" \
    "      <name>"SM_MODNAME_EXT"</name>\n" \
    "      <namespace>"SM_MOD_EXT_NAMESPACE"</namespace>\n" \
    "   </module>\n" \
    MODULES \
    "</module-set>\n" \
    "<content-id>1</content-id>\n" \
    "</yang-library>\n" \
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">\n" \
    "<module-set-id>1</module-set-id>\n" \
    "</modules-state>\n" \
    "<schema-mounts xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-schema-mount\">\n" \
    "<namespace>\n" \
    "   <prefix>"SM_PREF"</prefix>\n" \
    "   <uri>x:"MPMOD_NAME"</uri>\n" \
    "</namespace>\n" \
    "<mount-point>\n" \
    "   <module>"MPMOD_NAME"</module>\n" \
    "   <label>mnt-root</label>\n" \
    SCHEMA_REF \
    "</mount-point>\n" \
    "</schema-mounts>"

#define SM_MOD_MAIN(NAME, BODY) \
    "module "NAME" {\n" \
    "  yang-version 1.1;\n" \
    "  namespace \"x:"NAME"\";\n" \
    "  prefix \"x\";\n" \
    "  import ietf-yang-schema-mount {\n" \
    "    prefix yangmnt;\n" \
    "  }\n" \
    BODY \
    "}"

static void
mount_point(void **state)
{
    char *data;

    TEST_LOCAL_SETUP;
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    /* interested in sm-extension.yang and sm-mod.yang */
    assert_int_equal(LY_SUCCESS, ly_ctx_set_searchdir(UTEST_LYCTX, TESTS_DIR_MODULES_YANG));

    /*
     * 'mp' flag for list and container
     */
    orig = SM_MOD_MAIN("a29",
            "list lt {\n"
            "  key \"name\";\n"
            "  leaf name {\n"
            "    type string;\n"
            "  }\n"
            "  yangmnt:mount-point \"mnt-root\";\n"
            "}\n"
            "container cont {\n"
            "  yangmnt:mount-point \"mnt-root\";\n"
            "}\n");
    expect =
            "module: a29\n"
            "  +--mp lt* [name]\n"
            "  |  +--rw name    string\n"
            "  +--mp cont\n";
    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_out_reset(UTEST_OUT);

    /*
     * mount schema by 'inline' schema-ref
     */
    orig = SM_MOD_MAIN("a30",
            "list lt {\n"
            "  key \"name\";\n"
            "  leaf name {\n"
            "    type string;\n"
            "  }\n"
            "  yangmnt:mount-point \"mnt-root\";\n"
            "}\n");
    expect =
            "module: a30\n"
            "  +--mp lt* [name]\n"
            "     +--rw tlist/* [name]\n"
            "     |  +--rw name    uint32\n"
            "     +--rw tcont/\n"
            "     |  +--rw tleaf?   uint32\n"
            "     +--rw name    string\n";
    data = EXT_DATA("a30", "", SCHEMA_REF_INLINE);
    ly_ctx_set_ext_data_clb(UTEST_LYCTX, getter, data);
    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_out_reset(UTEST_OUT);

    /*
     * mount schema into empty container
     */
    orig = SM_MOD_MAIN("a31",
            "container cont {\n"
            "  yangmnt:mount-point \"mnt-root\";\n"
            "}\n"
            "leaf lf {\n"
            "  type string;\n"
            "}\n");
    expect =
            "module: a31\n"
            "  +--mp cont\n"
            "  |  +--rw tlist/* [name]\n"
            "  |  |  +--rw name    uint32\n"
            "  |  +--rw tcont/\n"
            "  |     +--rw tleaf?   uint32\n"
            "  +--rw lf?     string\n";
    data = EXT_DATA("a31", "", SCHEMA_REF_INLINE);
    ly_ctx_set_ext_data_clb(UTEST_LYCTX, getter, data);
    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_out_reset(UTEST_OUT);

    /*
     * mount schema into non-empty container
     */
    orig = SM_MOD_MAIN("a32",
            "container cont {\n"
            "  leaf lf1 {\n"
            "    type string;\n"
            "  }\n"
            "  yangmnt:mount-point \"mnt-root\";\n"
            "  leaf lf2 {\n"
            "    type string;\n"
            "  }\n"
            "}\n");
    expect =
            "module: a32\n"
            "  +--mp cont\n"
            "     +--rw tlist/* [name]\n"
            "     |  +--rw name    uint32\n"
            "     +--rw tcont/\n"
            "     |  +--rw tleaf?   uint32\n"
            "     +--rw lf1?   string\n"
            "     +--rw lf2?   string\n";
    data = EXT_DATA("a32", "", SCHEMA_REF_INLINE);
    ly_ctx_set_ext_data_clb(UTEST_LYCTX, getter, data);
    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_out_reset(UTEST_OUT);

    /*
     * mounting with parent-reference
     */
    orig = SM_MOD_MAIN("a33",
            "list pr {\n"
            "  key \"name\";\n"
            "  leaf name {\n"
            "    type string;\n"
            "  }\n"
            "  leaf prlf {\n"
            "    type string;\n"
            "  }\n"
            "}\n"
            "leaf lf {\n"
            "  type string;\n"
            "}\n"
            "container cont {\n"
            "  yangmnt:mount-point \"mnt-root\";\n"
            "  list lt {\n"
            "    key \"name\";\n"
            "    leaf name {\n"
            "      type string;\n"
            "    }\n"
            "  }\n"
            "}\n");
    expect =
            "module: a33\n"
            "  +--rw pr* [name]\n"
            "  |  +--rw name    string\n"
            "  |  +--rw prlf?   string\n"
            "  +--rw lf?     string\n"
            "  +--mp cont\n"
            "     +--rw tlist/* [name]\n"
            "     |  +--rw name    uint32\n"
            "     +--rw tcont/\n"
            "     |  +--rw tleaf?   uint32\n"
            "     +--rw pr@* [name]\n"
            "     |  +--rw prlf?   string\n"
            "     +--rw lf@?     string\n"
            "     +--rw lt* [name]\n"
            "        +--rw name    string\n";
    data = EXT_DATA("a33", "", SCHEMA_REF_SHARED(
            "<parent-reference>/"SM_PREF ":pr/"SM_PREF ":prlf</parent-reference>\n"
            "<parent-reference>/"SM_PREF ":lf</parent-reference>\n"));
    ly_ctx_set_ext_data_clb(UTEST_LYCTX, getter, data);
    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_out_reset(UTEST_OUT);

    /*
     * mounting with parent-reference into empty container
     */
    orig = SM_MOD_MAIN("a34",
            "container cont {\n"
            "  yangmnt:mount-point \"mnt-root\";\n"
            "}\n"
            "leaf lf {\n"
            "  type string;\n"
            "}\n");
    expect =
            "module: a34\n"
            "  +--mp cont\n"
            "  |  +--rw tlist/* [name]\n"
            "  |  |  +--rw name    uint32\n"
            "  |  +--rw tcont/\n"
            "  |  |  +--rw tleaf?   uint32\n"
            "  |  +--rw lf@?     string\n"
            "  +--rw lf?     string\n";
    data = EXT_DATA("a34", "",
            SCHEMA_REF_SHARED(
            "<parent-reference>/"SM_PREF ":lf</parent-reference>\n"));
    ly_ctx_set_ext_data_clb(UTEST_LYCTX, getter, data);
    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_out_reset(UTEST_OUT);

    /*
     * mounting module which is only parsed
     */
    orig = SM_MOD_MAIN("a35",
            "import sm-mod {\n"
            "  prefix smm;\n"
            "}\n"
            "container pr {\n"
            "  leaf prlf {\n"
            "    type uint32;\n"
            "  }\n"
            "}\n"
            "list lt {\n"
            "  key \"name\";\n"
            "  yangmnt:mount-point \"mnt-root\";\n"
            "  leaf name {\n"
            "    type string;\n"
            "  }\n"
            "}\n");
    expect =
            "module: a35\n"
            "  +--rw pr\n"
            "  |  +--rw prlf?   uint32\n"
            "  +--mp lt* [name]\n"
            "     +--rw tlist/* [name]\n"
            "     |  +--rw name    uint32\n"
            "     +--rw tcont/\n"
            "     |  +--rw tleaf?   uint32\n"
            "     +--mp ncmp/\n"
            "     +--rw not-compiled/\n"
            "     |  +--rw first?    string\n"
            "     |  +--rw second?   string\n"
            "     +--rw pr@\n"
            "     |  +--rw prlf?   uint32\n"
            "     +--rw name    string\n";
    data = EXT_DATA("a35",
            "<module>\n"
            "   <name>sm-mod</name>\n"
            "   <namespace>urn:sm-mod</namespace>\n"
            "</module>\n",
            SCHEMA_REF_SHARED(
            "<parent-reference>/"SM_PREF ":pr/"SM_PREF ":prlf</parent-reference>\n"));
    ly_ctx_set_ext_data_clb(UTEST_LYCTX, getter, data);
    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_out_reset(UTEST_OUT);

    /*
     * notifications and rpcs in mounted module
     */
    orig = SM_MOD_MAIN("a36",
            "container cont {\n"
            "  yangmnt:mount-point \"mnt-root\";\n"
            "}\n");
    expect =
            "module: a36\n"
            "  +--mp cont\n"
            "     +--rw tlist/* [name]\n"
            "     |  +--rw name    uint32\n"
            "     +--rw tcont/\n"
            "     |  +--rw tleaf?   uint32\n"
            "     +--rw cont/\n"
            "     |  +---x cr\n"
            "     |  |  +---w input\n"
            "     |  |  |  +---w in?   string\n"
            "     |  |  +--ro output\n"
            "     |  |     +--ro out?   string\n"
            "     |  +---n cn\n"
            "     +---x r1/\n"
            "     +---x r2/\n"
            "     +---n n1/\n"
            "     +---n n2/\n";
    data = EXT_DATA("a36",
            "<module>\n"
            "   <name>sm-rpcnotif</name>\n"
            "   <namespace>urn:rpcnotif</namespace>\n"
            "</module>\n",
            SCHEMA_REF_INLINE);
    ly_ctx_set_ext_data_clb(UTEST_LYCTX, getter, data);
    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_out_reset(UTEST_OUT);

    /*
     * parent-ref composes the '@' subtree
     */
    orig = SM_MOD_MAIN("a37",
            "container pr {\n"
            "  leaf ignored_node {\n"
            "    type string;\n"
            "  }\n"
            "  container cont {\n"
            "    leaf ignored_lf {\n"
            "      type uint32;\n"
            "    }\n"
            "  }\n"
            "  container ignored_subtree {\n"
            "    leaf ignored_lf {\n"
            "      type uint32;\n"
            "    }\n"
            "  }\n"
            "  container cont_sibl {\n"
            "    leaf slf {\n"
            "      type string;\n"
            "    }\n"
            "  }\n"
            "  leaf lf {\n"
            "    type uint32;\n"
            "  }\n"
            "}\n"
            "container cont_mount {\n"
            "  yangmnt:mount-point \"mnt-root\";\n"
            "}\n");
    expect =
            "module: a37\n"
            "  +--rw pr\n"
            "  |  +--rw ignored_node?      string\n"
            "  |  +--rw cont\n"
            "  |  |  +--rw ignored_lf?   uint32\n"
            "  |  +--rw ignored_subtree\n"
            "  |  |  +--rw ignored_lf?   uint32\n"
            "  |  +--rw cont_sibl\n"
            "  |  |  +--rw slf?   string\n"
            "  |  +--rw lf?                uint32\n"
            "  +--mp cont_mount\n"
            "     +--rw tlist/* [name]\n"
            "     |  +--rw name    uint32\n"
            "     +--rw tcont/\n"
            "     |  +--rw tleaf?   uint32\n"
            "     +--rw pr@\n"
            "        +--rw cont\n"
            "        +--rw cont_sibl\n"
            "        |  +--rw slf?   string\n"
            "        +--rw lf?          uint32\n";
    data = EXT_DATA("a37", "", SCHEMA_REF_SHARED(
            "<parent-reference>/"SM_PREF ":pr/"SM_PREF ":cont_sibl/slf</parent-reference>\n"
            "<parent-reference>/"SM_PREF ":pr/"SM_PREF ":cont</parent-reference>\n"
            "<parent-reference>/"SM_PREF ":pr/"SM_PREF ":lf</parent-reference>\n"));
    ly_ctx_set_ext_data_clb(UTEST_LYCTX, getter, data);
    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_out_reset(UTEST_OUT);

    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_TEARDOWN;
}

static void
structure(void **state)
{
    TEST_LOCAL_SETUP;

    orig =
            "module example-module {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:example:example-module\";\n"
            "  prefix exm;\n"
            "\n"
            "  import ietf-yang-structure-ext {\n"
            "    prefix sx;\n"
            "  }\n"
            "\n"
            "  sx:structure address-book {\n"
            "    list address {\n"
            "      key \"last first\";\n"
            "      leaf last {\n"
            "        type string;\n"
            "      }\n"
            "      leaf first {\n"
            "        type string;\n"
            "      }\n"
            "      leaf street {\n"
            "        type string;\n"
            "      }\n"
            "      leaf city {\n"
            "        type string;\n"
            "      }\n"
            "      leaf state {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "}\n";

    /* from RFC 8791, Appendix A.1 */
    expect =
            "module: example-module\n"
            "\n"
            "  structure address-book:\n"
            "    +-- address* [last first]\n"
            "       +-- last      string\n"
            "       +-- first     string\n"
            "       +-- street?   string\n"
            "       +-- city?     string\n"
            "       +-- state?    string\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_out_reset(UTEST_OUT);

    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_out_reset(UTEST_OUT);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    orig =
            "module example-module-aug {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:example:example-module-aug\";\n"
            "  prefix exma;\n"
            "\n"
            "  import ietf-yang-structure-ext {\n"
            "    prefix sx;\n"
            "  }\n"
            "  import example-module {\n"
            "    prefix exm;\n"
            "  }\n"
            "\n"
            "  sx:augment-structure \"/exm:address-book/exm:address\" {\n"
            "    leaf county {\n"
            "      type string;\n"
            "    }\n"
            "    leaf zipcode {\n"
            "      type string;\n"
            "    }\n"
            "  }\n"
            "}\n";

    /* from RFC 8791, Appendix A.2 */
    expect =
            "module: example-module-aug\n"
            "\n"
            "  augment-structure /exm:address-book/exm:address:\n"
            "    +-- county?    string\n"
            "    +-- zipcode?   string\n";

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_out_reset(UTEST_OUT);

    /* using lysc tree */
    ly_ctx_set_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_out_reset(UTEST_OUT);
    ly_ctx_unset_options(UTEST_LYCTX, LY_CTX_SET_PRIV_PARSED);

    TEST_LOCAL_TEARDOWN;
}

static void
annotation(void **state)
{
    TEST_LOCAL_SETUP;

    orig =
            "module ann {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:example:ann\";\n"
            "  prefix an;\n"
            "\n"
            "  import ietf-yang-metadata {\n"
            "    prefix md;\n"
            "  }\n"
            "\n"
            "  leaf lf1 {\n"
            "    type string;\n"
            "  }\n"
            "  md:annotation avalue {\n"
            "    type string;\n"
            "  }\n"
            "}\n";

    expect =
            "module: ann\n"
            "  +--rw lf1?   string\n";

    /* annotation is ignored without error message */
    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    TEST_LOCAL_PRINT(mod, 72);
    assert_int_equal(strlen(expect), ly_out_printed(UTEST_OUT));
    assert_string_equal(printed, expect);
    ly_out_reset(UTEST_OUT);

    TEST_LOCAL_TEARDOWN;
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(base_sections),
        UTEST(node_status),
        UTEST(node_config_flags),
        UTEST(node_rpcs_flags),
        UTEST(node_grouping_flags),
        UTEST(notif_inside_container),
        UTEST(node_choice),
        UTEST(node_case),
        UTEST(optional_opts),
        UTEST(presence_container),
        UTEST(node_keys),
        UTEST(node_type_target),
        UTEST(node_type_leafref),
        UTEST(node_iffeatures),
        UTEST(indent_wrapper),
        UTEST(line_length_twiddling),
        UTEST(break_before_leafref),
        UTEST(break_before_leafref_and_iffeature),
        UTEST(basic_unified_indent_before_type),
        UTEST(twiddling_unified_indent_before_type),
        UTEST(inheritance_of_config_flag),
        UTEST(inheritance_of_status_flag),
        UTEST(key_leaf_is_always_mandatory_true),
        UTEST(transition_between_rpc_and_notif),
        UTEST(local_augment),
        UTEST(print_compiled_node),
        UTEST(print_compiled_node_augment),
        UTEST(print_parsed_submodule),
        UTEST(yang_data),
        UTEST(mount_point),
        UTEST(structure),
        UTEST(annotation),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
