/*
 * @file test_schema_common.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from common.c
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#include "test_schema.h"

#include <string.h>

#include "context.h"
#include "log.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

void
test_getnext(void **state)
{
    const struct lys_module *mod;
    const struct lysc_node *node = NULL, *four;
    const struct lysc_node_container *cont;
    const struct lysc_action *rpc;

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module a {yang-version 1.1; namespace urn:a;prefix a;"
            "container a { container one {presence test;} leaf two {type string;} leaf-list three {type string;}"
            "  list four {config false;} choice x { leaf five {type string;} case y {leaf six {type string;}}}"
            "  anyxml seven; action eight {input {leaf eight-input {type string;}} output {leaf eight-output {type string;}}}"
            "  notification nine {leaf nine-data {type string;}}}"
            "leaf b {type string;} leaf-list c {type string;} list d {config false;}"
            "choice x { case empty-x { choice empty-xc { case nothing;}} leaf e {type string;} case y {leaf f {type string;}}} anyxml g;"
            "rpc h {input {leaf h-input {type string;}} output {leaf h-output {type string;}}}"
            "rpc i;"
            "notification j {leaf i-data {type string;}}"
            "notification k;}", LYS_IN_YANG, &mod));
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("a", node->name);
    cont = (const struct lysc_node_container *)node;
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("b", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("c", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("d", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("e", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("f", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("g", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("h", node->name);
    rpc = (const struct lysc_action *)node;
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("i", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("j", node->name);
    assert_non_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    assert_string_equal("k", node->name);
    assert_null(node = lys_getnext(node, NULL, mod->compiled, 0));
    /* Inside container */
    assert_non_null(node = lys_getnext(node, (const struct lysc_node *)cont, mod->compiled, 0));
    assert_string_equal("one", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node *)cont, mod->compiled, 0));
    assert_string_equal("two", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node *)cont, mod->compiled, 0));
    assert_string_equal("three", node->name);
    assert_non_null(node = four = lys_getnext(node, (const struct lysc_node *)cont, mod->compiled, 0));
    assert_string_equal("four", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node *)cont, mod->compiled, 0));
    assert_string_equal("five", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node *)cont, mod->compiled, 0));
    assert_string_equal("six", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node *)cont, mod->compiled, 0));
    assert_string_equal("seven", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node *)cont, mod->compiled, 0));
    assert_string_equal("eight", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node *)cont, mod->compiled, 0));
    assert_string_equal("nine", node->name);
    assert_null(node = lys_getnext(node, (const struct lysc_node *)cont, mod->compiled, 0));
    /* Inside RPC */
    assert_non_null(node = lys_getnext(node, (const struct lysc_node *)rpc, mod->compiled, 0));
    assert_string_equal("h-input", node->name);
    assert_null(node = lys_getnext(node, (const struct lysc_node *)rpc, mod->compiled, 0));

    /* options */
    assert_non_null(node = lys_getnext(four, (const struct lysc_node *)cont, mod->compiled, LYS_GETNEXT_WITHCHOICE));
    assert_string_equal("x", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node *)cont, mod->compiled, LYS_GETNEXT_WITHCHOICE));
    assert_string_equal("seven", node->name);

    assert_non_null(node = lys_getnext(four, (const struct lysc_node *)cont, mod->compiled, LYS_GETNEXT_NOCHOICE));
    assert_string_equal("seven", node->name);

    assert_non_null(node = lys_getnext(four, (const struct lysc_node *)cont, mod->compiled, LYS_GETNEXT_WITHCASE));
    assert_string_equal("five", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node *)cont, mod->compiled, LYS_GETNEXT_WITHCASE));
    assert_string_equal("y", node->name);
    assert_non_null(node = lys_getnext(node, (const struct lysc_node *)cont, mod->compiled, LYS_GETNEXT_WITHCASE));
    assert_string_equal("seven", node->name);

    assert_non_null(node = lys_getnext(NULL, NULL, mod->compiled, LYS_GETNEXT_INTONPCONT));
    assert_string_equal("one", node->name);

    assert_non_null(node = lys_getnext(NULL, (const struct lysc_node *)rpc, mod->compiled, LYS_GETNEXT_OUTPUT));
    assert_string_equal("h-output", node->name);
    assert_null(node = lys_getnext(node, (const struct lysc_node *)rpc, mod->compiled, LYS_GETNEXT_OUTPUT));

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module c {namespace urn:c;prefix c; rpc c;}", LYS_IN_YANG, &mod));
    assert_non_null(node = lys_getnext(NULL, NULL, mod->compiled, 0));
    assert_string_equal("c", node->name);
    assert_null(node = lys_getnext(node, NULL, mod->compiled, 0));

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module d {namespace urn:d;prefix d; notification d;}", LYS_IN_YANG, &mod));
    assert_non_null(node = lys_getnext(NULL, NULL, mod->compiled, 0));
    assert_string_equal("d", node->name);
    assert_null(node = lys_getnext(node, NULL, mod->compiled, 0));

    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, "module e {namespace urn:e;prefix e; container c {container cc;} leaf a {type string;}}", LYS_IN_YANG, &mod));
    assert_non_null(node = lys_getnext(NULL, NULL, mod->compiled, 0));
    assert_string_equal("c", node->name);
    assert_non_null(node = lys_getnext(NULL, NULL, mod->compiled, LYS_GETNEXT_INTONPCONT));
    assert_string_equal("a", node->name);
}

void
test_date(void **state)
{
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, NULL, 0, "date"));
    CHECK_LOG("Invalid argument date (lysp_check_date()).", NULL);
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "x", 1, "date"));
    CHECK_LOG("Invalid argument date_len (lysp_check_date()).", NULL);
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "nonsencexx", 10, "date"));
    CHECK_LOG("Invalid value \"nonsencexx\" of \"date\".", NULL);
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "123x-11-11", 10, "date"));
    CHECK_LOG("Invalid value \"123x-11-11\" of \"date\".", NULL);
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018-13-11", 10, "date"));
    CHECK_LOG("Invalid value \"2018-13-11\" of \"date\".", NULL);
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018-11-41", 10, "date"));
    CHECK_LOG("Invalid value \"2018-11-41\" of \"date\".", NULL);
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018-02-29", 10, "date"));
    CHECK_LOG("Invalid value \"2018-02-29\" of \"date\".", NULL);
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018.02-28", 10, "date"));
    CHECK_LOG("Invalid value \"2018.02-28\" of \"date\".", NULL);
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "2018-02.28", 10, "date"));
    CHECK_LOG("Invalid value \"2018-02.28\" of \"date\".", NULL);

    assert_int_equal(LY_SUCCESS, lysp_check_date(NULL, "2018-11-11", 10, "date"));
    assert_int_equal(LY_SUCCESS, lysp_check_date(NULL, "2018-02-28", 10, "date"));
    assert_int_equal(LY_SUCCESS, lysp_check_date(NULL, "2016-02-29", 10, "date"));
}

void
test_revisions(void **state)
{
    struct lysp_revision *revs = NULL, *rev;

    /* no error, it just does nothing */
    lysp_sort_revisions(NULL);
    CHECK_LOG(NULL, NULL);

    /* revisions are stored in wrong order - the newest is the last */
    LY_ARRAY_NEW_RET(NULL, revs, rev, );
    strcpy(rev->date, "2018-01-01");
    LY_ARRAY_NEW_RET(NULL, revs, rev, );
    strcpy(rev->date, "2018-12-31");

    assert_int_equal(2, LY_ARRAY_COUNT(revs));
    assert_string_equal("2018-01-01", &revs[0]);
    assert_string_equal("2018-12-31", &revs[1]);
    /* the order should be fixed, so the newest revision will be the first in the array */
    lysp_sort_revisions(revs);
    assert_string_equal("2018-12-31", &revs[0]);
    assert_string_equal("2018-01-01", &revs[1]);

    LY_ARRAY_FREE(revs);
}

void
test_typedef(void **state)
{
    const char *str;

    str = "module a {namespace urn:a; prefix a; typedef binary {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"binary\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef bits {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"bits\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef boolean {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"boolean\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef decimal64 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"decimal64\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef empty {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"empty\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef enumeration {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"enumeration\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef int8 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"int8\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef int16 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"int16\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef int32 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"int32\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef int64 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"int64\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef instance-identifier {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"instance-identifier\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef identityref {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"identityref\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef leafref {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"leafref\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef string {type int8;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"string\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef union {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"union\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef uint8 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"uint8\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef uint16 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"uint16\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef uint32 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"uint32\" of typedef - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef uint64 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"uint64\" of typedef - name collision with a built-in type.", NULL);

    str = "module mytypes {namespace urn:types; prefix t; typedef binary_ {type string;} typedef bits_ {type string;} typedef boolean_ {type string;} "
            "typedef decimal64_ {type string;} typedef empty_ {type string;} typedef enumeration_ {type string;} typedef int8_ {type string;} typedef int16_ {type string;}"
            "typedef int32_ {type string;} typedef int64_ {type string;} typedef instance-identifier_ {type string;} typedef identityref_ {type string;}"
            "typedef leafref_ {type string;} typedef string_ {type int8;} typedef union_ {type string;} typedef uint8_ {type string;} typedef uint16_ {type string;}"
            "typedef uint32_ {type string;} typedef uint64_ {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_SUCCESS);

    str = "module a {namespace urn:a; prefix a; typedef test {type string;} typedef test {type int8;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"test\" of typedef - name collision with another top-level type.", NULL);

    str = "module a {namespace urn:a; prefix a; typedef x {type string;} container c {typedef x {type int8;}}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"x\" of typedef - scoped type collide with a top-level type.", NULL);

    str = "module a {namespace urn:a; prefix a; container c {container d {typedef y {type int8;}} typedef y {type string;}}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"y\" of typedef - name collision with another scoped type.", NULL);

    str = "module a {namespace urn:a; prefix a; container c {typedef y {type int8;} typedef y {type string;}}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"y\" of typedef - name collision with sibling type.", NULL);

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule b {belongs-to a {prefix a;} typedef x {type string;}}");
    str = "module a {namespace urn:a; prefix a; include b; typedef x {type int8;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"x\" of typedef - name collision with another top-level type.", NULL);

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule b {belongs-to a {prefix a;} container c {typedef x {type string;}}}");
    str = "module a {namespace urn:a; prefix a; include b; typedef x {type int8;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"x\" of typedef - scoped type collide with a top-level type.", NULL);

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule b {belongs-to a {prefix a;} typedef x {type int8;}}");
    str = "module a {namespace urn:a; prefix a; include b; container c {typedef x {type string;}}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EEXIST);
    CHECK_LOG("Invalid name \"x\" of typedef - scoped type collide with a top-level type.", NULL);
}

void
test_accessible_tree(void **state)
{
    const char *str;

    /* config -> config */
    str = "module a {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    container cont {\n"
            "        leaf l {\n"
            "            type empty;\n"
            "        }\n"
            "    }\n"
            "    container cont2 {\n"
            "        leaf l2 {\n"
            "            must ../../cont/l;\n"
            "            type leafref {\n"
            "                path /cont/l;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    CHECK_LOG_CTX(NULL, NULL);

    /* config -> state leafref */
    str = "module b {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    container cont {\n"
            "        config false;\n"
            "        leaf l {\n"
            "            type empty;\n"
            "        }\n"
            "    }\n"
            "    container cont2 {\n"
            "        leaf l2 {\n"
            "            type leafref {\n"
            "                path /cont/l;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid leafref path \"/cont/l\" - target is supposed to represent configuration data"
            " (as the leafref does), but it does not.", "Schema location /b:cont2/l2.");

    /* config -> state must */
    str = "module b {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    container cont {\n"
            "        config false;\n"
            "        leaf l {\n"
            "            type empty;\n"
            "        }\n"
            "    }\n"
            "    container cont2 {\n"
            "        leaf l2 {\n"
            "            must ../../cont/l;\n"
            "            type empty;\n"
            "        }\n"
            "    }\n"
            "}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    CHECK_LOG_CTX("Schema node \"l\" not found (\"../../cont/l\") with context node \"/b:cont2/l2\".", NULL);

    /* state -> config */
    str = "module c {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    container cont {\n"
            "        leaf l {\n"
            "            type empty;\n"
            "        }\n"
            "    }\n"
            "    container cont2 {\n"
            "        config false;\n"
            "        leaf l2 {\n"
            "            must ../../cont/l;\n"
            "            type leafref {\n"
            "                path /cont/l;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    CHECK_LOG_CTX(NULL, NULL);

    /* notif -> state */
    str = "module d {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    container cont {\n"
            "        config false;\n"
            "        leaf l {\n"
            "            type empty;\n"
            "        }\n"
            "    }\n"
            "    notification notif {\n"
            "        leaf l2 {\n"
            "            must ../../cont/l;\n"
            "            type leafref {\n"
            "                path /cont/l;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    CHECK_LOG_CTX(NULL, NULL);

    /* notif -> notif */
    str = "module e {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    notification notif {\n"
            "        leaf l {\n"
            "            type empty;\n"
            "        }\n"
            "        leaf l2 {\n"
            "            must ../../notif/l;\n"
            "            type leafref {\n"
            "                path /notif/l;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    CHECK_LOG_CTX(NULL, NULL);

    /* rpc input -> state */
    str = "module f {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    container cont {\n"
            "        config false;\n"
            "        leaf l {\n"
            "            type empty;\n"
            "        }\n"
            "    }\n"
            "    rpc rp {\n"
            "        input {\n"
            "            leaf l2 {\n"
            "                must ../../cont/l;\n"
            "                type leafref {\n"
            "                    path /cont/l;\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    CHECK_LOG_CTX(NULL, NULL);

    /* rpc input -> rpc input */
    str = "module g {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    rpc rp {\n"
            "        input {\n"
            "            leaf l {\n"
            "                type empty;\n"
            "            }\n"
            "            leaf l2 {\n"
            "                must ../l;\n"
            "                type leafref {\n"
            "                    path /rp/l;\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    CHECK_LOG_CTX(NULL, NULL);

    /* rpc input -> rpc output leafref */
    str = "module h {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    rpc rp {\n"
            "        input {\n"
            "            leaf l2 {\n"
            "                type leafref {\n"
            "                    path /rp/l;\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "        output {\n"
            "            leaf l {\n"
            "                type empty;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Not found node \"l\" in path.", "Schema location /h:rp/input/l2.");

    /* rpc input -> rpc output must */
    str = "module h {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    rpc rp {\n"
            "        input {\n"
            "            leaf l2 {\n"
            "                must ../l;\n"
            "                type empty;\n"
            "            }\n"
            "        }\n"
            "        output {\n"
            "            leaf l {\n"
            "                type empty;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    CHECK_LOG_CTX("Schema node \"l\" not found (\"../l\") with context node \"/h:rp/input/l2\".", NULL);

    /* rpc input -> notif leafref */
    str = "module i {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    rpc rp {\n"
            "        input {\n"
            "            leaf l2 {\n"
            "                type leafref {\n"
            "                    path ../../notif/l;\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    notification notif {\n"
            "        leaf l {\n"
            "            type empty;\n"
            "        }\n"
            "    }\n"
            "}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Not found node \"notif\" in path.", "Schema location /i:rp/input/l2.");

    /* rpc input -> notif must */
    str = "module i {\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    rpc rp {\n"
            "        input {\n"
            "            leaf l2 {\n"
            "                must /notif/l;\n"
            "                type empty;\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "    notification notif {\n"
            "        leaf l {\n"
            "            type empty;\n"
            "        }\n"
            "    }\n"
            "}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    CHECK_LOG_CTX("Schema node \"l\" not found (\"/notif/l\") with context node \"/i:rp/input/l2\".", NULL);

    /* action output -> state */
    str = "module j {\n"
            "    yang-version 1.1;\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    container cont {\n"
            "        list ll {\n"
            "            key k;\n"
            "            leaf k {\n"
            "                type string;\n"
            "            }\n"
            "            action act {\n"
            "                output {\n"
            "                    leaf l2 {\n"
            "                        must /cont/l;\n"
            "                        type leafref {\n"
            "                            path ../../../l;\n"
            "                        }\n"
            "                    }\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "        leaf l {\n"
            "            config false;\n"
            "            type empty;\n"
            "        }\n"
            "    }\n"
            "}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    CHECK_LOG_CTX(NULL, NULL);

    /* action output -> action input leafref */
    str = "module k {\n"
            "    yang-version 1.1;\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    container cont {\n"
            "        list ll {\n"
            "            key k;\n"
            "            leaf k {\n"
            "                type string;\n"
            "            }\n"
            "            action act {\n"
            "                input {\n"
            "                    leaf l {\n"
            "                        type empty;\n"
            "                    }\n"
            "                }\n"
            "                output {\n"
            "                    leaf l2 {\n"
            "                        type leafref {\n"
            "                            path ../l;\n"
            "                        }\n"
            "                    }\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Not found node \"l\" in path.", "Schema location /k:cont/ll/act/output/l2.");

    /* action output -> action input must */
    str = "module k {\n"
            "    yang-version 1.1;\n"
            "    namespace urn:a;\n"
            "    prefix a;\n"
            "    container cont {\n"
            "        list ll {\n"
            "            key k;\n"
            "            leaf k {\n"
            "                type string;\n"
            "            }\n"
            "            action act {\n"
            "                input {\n"
            "                    leaf l {\n"
            "                        type empty;\n"
            "                    }\n"
            "                }\n"
            "                output {\n"
            "                    leaf l2 {\n"
            "                        must /cont/ll/act/l;\n"
            "                        type empty;\n"
            "                    }\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_SUCCESS);
    CHECK_LOG_CTX("Schema node \"l\" not found (\"/cont/ll/act/l\") with context node \"/k:cont/ll/act/output/l2\".", NULL);
}

struct module_clb_list {
    const char *name;
    const char *data;
};

static LY_ERR
module_clb(const char *mod_name, const char *UNUSED(mod_rev), const char *submod_name,
        const char *UNUSED(sub_rev), void *user_data, LYS_INFORMAT *format,
        const char **module_data, void (**free_module_data)(void *model_data, void *user_data))
{
    struct module_clb_list *list = (struct module_clb_list *)user_data;

    for (unsigned int i = 0; list[i].data; i++) {

        if ((submod_name && !strcmp(list[i].name, submod_name)) ||
                (!submod_name && mod_name && !strcmp(list[i].name, mod_name))) {
            *module_data = list[i].data;
            *format = LYS_IN_YANG;
            *free_module_data = NULL;
            return LY_SUCCESS;
        }
    }
    return LY_EINVAL;
}

void
test_includes(void **state)
{
    const struct lys_module *mod;

    {
        /* YANG 1.0 - the missing include sub_a_two in main_a will be injected from sub_a_one */
        struct module_clb_list list[] = {
            {"main_a", "module main_a { namespace urn:test:main_a; prefix ma; include sub_a_one;}"},
            {"sub_a_one", "submodule sub_a_one { belongs-to main_a { prefix ma; } include sub_a_two;}"},
            {"sub_a_two", "submodule sub_a_two { belongs-to main_a { prefix ma; } }"},
            {NULL, NULL}
        };
        ly_ctx_set_module_imp_clb(UTEST_LYCTX, module_clb, list);
        mod = ly_ctx_load_module(UTEST_LYCTX, "main_a", NULL, NULL);
        assert_non_null(mod);
        assert_int_equal(2, LY_ARRAY_COUNT(mod->parsed->includes));
        assert_true(mod->parsed->includes[1].injected);
    }

    {
        /* YANG 1.1 - the missing include sub_b_two in main_b is error */
        struct module_clb_list list[] = {
            {"main_b", "module main_b { yang-version 1.1; namespace urn:test:main_b; prefix mb; include sub_b_one;}"},
            {"sub_b_one", "submodule sub_b_one { yang-version 1.1; belongs-to main_b { prefix mb; } include sub_b_two;}"},
            {"sub_b_two", "submodule sub_b_two { yang-version 1.1; belongs-to main_b { prefix mb; } }"},
            {NULL, NULL}
        };
        ly_ctx_set_module_imp_clb(UTEST_LYCTX, module_clb, list);
        mod = ly_ctx_load_module(UTEST_LYCTX, "main_b", NULL, NULL);
        assert_null(mod);
        CHECK_LOG_CTX("Loading \"main_b\" module failed.", NULL,
                "Data model \"main_b\" not found in local searchdirs.", NULL,
                "Including \"sub_b_one\" submodule into \"main_b\" failed.", NULL,
                "Data model \"sub_b_one\" not found in local searchdirs.", NULL,
                "YANG 1.1 requires all submodules to be included from main module. But submodule \"sub_b_one\" includes "
                "submodule \"sub_b_two\" which is not included by main module \"main_b\".", NULL);
    }

    {
        /* YANG 1.1 - all includes are in main_c, includes in submodules are not necessary, so expect warning */
        struct module_clb_list list[] = {
            {"main_c", "module main_c { yang-version 1.1; namespace urn:test:main_c; prefix mc; include sub_c_one; include sub_c_two;}"},
            {"sub_c_one", "submodule sub_c_one { yang-version 1.1; belongs-to main_c { prefix mc; } include sub_c_two;}"},
            {"sub_c_two", "submodule sub_c_two { yang-version 1.1; belongs-to main_c { prefix mc; } }"},
            {NULL, NULL}
        };
        ly_ctx_set_module_imp_clb(UTEST_LYCTX, module_clb, list);
        mod = ly_ctx_load_module(UTEST_LYCTX, "main_c", NULL, NULL);
        assert_non_null(mod);
        assert_int_equal(2, LY_ARRAY_COUNT(mod->parsed->includes));
        assert_false(mod->parsed->includes[1].injected);
        /* result is ok, but log includes the warning */
        CHECK_LOG_CTX("YANG version 1.1 expects all includes in main module, includes in submodules (sub_c_one) are not necessary.", NULL);
    }
}
