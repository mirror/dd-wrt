/**
 * @file test_schema.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for schema related functions
 *
 * Copyright (c) 2018 - 2022 CESNET, z.s.p.o.
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

#include "compat.h"
#include "context.h"
#include "log.h"
#include "parser_schema.h"
#include "plugins_exts.h"
#include "set.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"

static LY_ERR
test_imp_clb(const char *UNUSED(mod_name), const char *UNUSED(mod_rev), const char *UNUSED(submod_name),
        const char *UNUSED(sub_rev), void *user_data, LYS_INFORMAT *format,
        const char **module_data, void (**free_module_data)(void *model_data, void *user_data))
{
    *module_data = user_data;
    if ((*module_data)[0] == '<') {
        *format = LYS_IN_YIN;
    } else {
        *format = LYS_IN_YANG;
    }
    *free_module_data = NULL;
    return LY_SUCCESS;
}

#define TEST_YANG_MODULE_10(MOD_NAME, MOD_PREFIX, MOD_NS, CONTENT) \
    "module "MOD_NAME" { namespace "MOD_NS"; prefix "MOD_PREFIX"; "CONTENT"}"

#define TEST_YANG_MODULE_11(MOD_NAME, MOD_PREFIX, MOD_NS, CONTENT) \
    "module "MOD_NAME" {yang-version 1.1; namespace "MOD_NS"; prefix "MOD_PREFIX"; "CONTENT"}"

#define TEST_YIN_MODULE_10(MOD_NAME, MOD_PREFIX, MOD_NS, CONTENT) \
    "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\""MOD_NAME"\">" \
    "<namespace uri=\""MOD_NS"\"/><prefix value=\""MOD_PREFIX"\"/>"CONTENT"</module>"

#define TEST_YIN_MODULE_11(MOD_NAME, MOD_PREFIX, MOD_NS, CONTENT) \
    "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\""MOD_NAME"\"><yang-version value=\"1.1\"/>" \
    "<namespace uri=\""MOD_NS"\"/><prefix value=\""MOD_PREFIX"\"/>"CONTENT"</module>"

#define TEST_SCHEMA_STR(RFC7950, YIN, MOD_NAME, CONTENT, STR) \
    if (YIN) { \
        if (RFC7950) { \
            STR = TEST_YIN_MODULE_11(MOD_NAME, MOD_NAME, "urn:libyang:test:"MOD_NAME, CONTENT); \
        } else { \
            STR = TEST_YIN_MODULE_10(MOD_NAME, MOD_NAME, "urn:libyang:test:"MOD_NAME, CONTENT); \
        } \
    } else { /* YANG */ \
        if (RFC7950) { \
            STR = TEST_YANG_MODULE_11(MOD_NAME, MOD_NAME, "urn:libyang:test:"MOD_NAME, CONTENT); \
        } else { \
            STR = TEST_YANG_MODULE_10(MOD_NAME, MOD_NAME, "urn:libyang:test:"MOD_NAME, CONTENT); \
        } \
    }

#define TEST_SCHEMA_OK(RFC7950, YIN, MOD_NAME, CONTENT, RESULT) \
    { \
    const char *test_str__; \
    TEST_SCHEMA_STR(RFC7950, YIN, MOD_NAME, CONTENT, test_str__) \
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, test_str__, YIN ? LYS_IN_YIN : LYS_IN_YANG, &(RESULT))); \
    }

#define TEST_SCHEMA_ERR(RFC7950, YIN, MOD_NAME, CONTENT, ERRMSG, ERRPATH) \
    { \
    const char *test_str__; \
    TEST_SCHEMA_STR(RFC7950, YIN, MOD_NAME, CONTENT, test_str__) \
    assert_int_not_equal(lys_parse_mem(UTEST_LYCTX, test_str__, YIN ? LYS_IN_YIN : LYS_IN_YANG, NULL), LY_SUCCESS); \
    CHECK_LOG_CTX(ERRMSG, ERRPATH); \
    }

#define TEST_SCHEMA_PARSE_ERR(RFC7950, YIN, MOD_NAME, CONTENT, ERRMSG, ERRPATH) \
    { \
    const char *test_str__; \
    TEST_SCHEMA_STR(RFC7950, YIN, MOD_NAME, CONTENT, test_str__) \
    assert_int_not_equal(lys_parse_mem(UTEST_LYCTX, test_str__, YIN ? LYS_IN_YIN : LYS_IN_YANG, NULL), LY_SUCCESS); \
    CHECK_LOG_CTX("Parsing module \""MOD_NAME"\" failed.", NULL); \
    CHECK_LOG_CTX(ERRMSG, ERRPATH); \
    }

#define TEST_STMT_DUP(RFC7950, YIN, STMT, MEMBER, VALUE1, VALUE2, LINE) \
    if (YIN) { \
        TEST_SCHEMA_PARSE_ERR(RFC7950, YIN, "dup", "", "Duplicate keyword \""MEMBER"\".", "Line number "LINE"."); \
    } else { \
        TEST_SCHEMA_PARSE_ERR(RFC7950, YIN, "dup", STMT"{"MEMBER" "VALUE1";"MEMBER" "VALUE2";}", \
                        "Duplicate keyword \""MEMBER"\".", "Line number "LINE"."); \
    }

#define TEST_STMT_SUBSTM_ERR(RFC7950, STMT, SUBSTMT, VALUE) ;\
        TEST_SCHEMA_PARSE_ERR(RFC7950, 0, "inv", STMT" test {"SUBSTMT" "VALUE";}", \
                        "Invalid keyword \""SUBSTMT"\" as a child of \""STMT"\".", "Line number 1.");

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

static void
test_getnext(void **state)
{
    struct lys_module *mod;
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

static void
test_date(void **state)
{
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, NULL, 0, "date"));
    CHECK_LOG("Invalid argument date (lysp_check_date()).", NULL);
    assert_int_equal(LY_EINVAL, lysp_check_date(NULL, "x", 1, "date"));
    CHECK_LOG("Invalid length 1 of a date.", NULL);
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

static void
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
    assert_string_equal("2018-01-01", revs[0].date);
    assert_string_equal("2018-12-31", revs[1].date);
    /* the order should be fixed, so the newest revision will be the first in the array */
    lysp_sort_revisions(revs);
    assert_string_equal("2018-12-31", revs[0].date);
    assert_string_equal("2018-01-01", revs[1].date);

    LY_ARRAY_FREE(revs);
}

static void
test_collision_typedef(void **state)
{
    const char *str;
    char *submod;
    struct module_clb_list list[3] = {0};

    list[0].name = "asub";
    list[1].name = "bsub";

    /* collision with a built-in type */
    str = "module a {namespace urn:a; prefix a; typedef binary {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"binary\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef bits {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"bits\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef boolean {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"boolean\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef decimal64 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"decimal64\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef empty {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"empty\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef enumeration {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"enumeration\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef int8 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"int8\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef int16 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"int16\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef int32 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"int32\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef int64 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"int64\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef instance-identifier {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"instance-identifier\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef identityref {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"identityref\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef leafref {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"leafref\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef string {type int8;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"string\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef union {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"union\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef uint8 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"uint8\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef uint16 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"uint16\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef uint32 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"uint32\" of typedef statement - name collision with a built-in type.", NULL);
    str = "module a {namespace urn:a; prefix a; typedef uint64 {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"uint64\" of typedef statement - name collision with a built-in type.", NULL);

    str = "module mytypes {namespace urn:types; prefix t; typedef binary_ {type string;} typedef bits_ {type string;} typedef boolean_ {type string;} "
            "typedef decimal64_ {type string;} typedef empty_ {type string;} typedef enumeration_ {type string;} typedef int8_ {type string;} typedef int16_ {type string;}"
            "typedef int32_ {type string;} typedef int64_ {type string;} typedef instance-identifier_ {type string;} typedef identityref_ {type string;}"
            "typedef leafref_ {type string;} typedef string_ {type int8;} typedef union_ {type string;} typedef uint8_ {type string;} typedef uint16_ {type string;}"
            "typedef uint32_ {type string;} typedef uint64_ {type string;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_SUCCESS);

    /* collision in node's scope */
    str = "module a {namespace urn:a; prefix a; container c {typedef y {type int8;} typedef y {type string;}}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"y\" of typedef statement - name collision with sibling type.", NULL);

    /* collision with parent node */
    str = "module a {namespace urn:a; prefix a; container c {container d {typedef y {type int8;}} typedef y {type string;}}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"y\" of typedef statement - name collision with another scoped type.", NULL);

    /* collision with module's top-level */
    str = "module a {namespace urn:a; prefix a; typedef x {type string;} container c {typedef x {type int8;}}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"x\" of typedef statement - scoped type collide with a top-level type.", NULL);

    /* collision of submodule's node with module's top-level */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule b {belongs-to a {prefix a;} container c {typedef x {type string;}}}");
    str = "module a {namespace urn:a; prefix a; include b; typedef x {type int8;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"x\" of typedef statement - scoped type collide with a top-level type.", NULL);

    /* collision of module's node with submodule's top-level */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule b {belongs-to a {prefix a;} typedef x {type int8;}}");
    str = "module a {namespace urn:a; prefix a; include b; container c {typedef x {type string;}}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"x\" of typedef statement - scoped type collide with a top-level type.", NULL);

    /* collision of submodule's node with another submodule's top-level */
    str = "module a {yang-version 1.1; namespace urn:a; prefix a; include asub; include bsub;}";
    list[0].data = "submodule asub {belongs-to a {prefix a;} typedef g {type int;}}";
    list[1].data = "submodule bsub {belongs-to a {prefix a;} container c {typedef g {type int;}}}";
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, module_clb, list);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"g\" of typedef statement - scoped type collide with a top-level type.", NULL);

    /* collision of module's top-levels */
    str = "module a {namespace urn:a; prefix a; typedef test {type string;} typedef test {type int8;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"test\" of typedef statement - name collision with another top-level type.", NULL);

    /* collision of submodule's top-levels */
    submod = "submodule asub {belongs-to a {prefix a;} typedef g {type int;} typedef g {type int;}}";
    str = "module a {yang-version 1.1; namespace urn:a; prefix a; include asub;}";
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, submod);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"g\" of typedef statement - name collision with another top-level type.", NULL);

    /* collision of module's top-level with submodule's top-levels */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule b {belongs-to a {prefix a;} typedef x {type string;}}");
    str = "module a {namespace urn:a; prefix a; include b; typedef x {type int8;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"x\" of typedef statement - name collision with another top-level type.", NULL);

    /* collision of submodule's top-level with another submodule's top-levels */
    str = "module a {yang-version 1.1; namespace urn:a; prefix a; include asub; include bsub;}";
    list[0].data = "submodule asub {belongs-to a {prefix a;} typedef g {type int;}}";
    list[1].data = "submodule bsub {belongs-to a {prefix a;} typedef g {type int;}}";
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, module_clb, list);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"g\" of typedef statement - name collision with another top-level type.", NULL);

    /* error in type-stmt */
    str = "module a {namespace urn:a; prefix a; container c {typedef x {type t{}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Unexpected end-of-input.", "Line number 1.");

    /* no collision if the same names are in different scope */
    str = "module a {yang-version 1.1; namespace urn:a; prefix a;"
            "container c {typedef g {type int;}} container d {typedef g {type int;}}}";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL));
}

static void
test_collision_grouping(void **state)
{
    const char *str;
    char *submod;
    struct module_clb_list list[3] = {0};

    list[0].name = "asub";
    list[1].name = "bsub";

    /* collision in node's scope */
    str = "module a {namespace urn:a; prefix a; container c {grouping y; grouping y;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"y\" of grouping statement - name collision with sibling grouping.", NULL);

    /* collision with parent node */
    str = "module a {namespace urn:a; prefix a; container c {container d {grouping y;} grouping y;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"y\" of grouping statement - name collision with another scoped grouping.", NULL);

    /* collision with module's top-level */
    str = "module a {namespace urn:a; prefix a; grouping x; container c {grouping x;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"x\" of grouping statement - scoped grouping collide with a top-level grouping.", NULL);

    /* collision of submodule's node with module's top-level */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule b {belongs-to a {prefix a;} container c {grouping x;}}");
    str = "module a {namespace urn:a; prefix a; include b; grouping x;}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"x\" of grouping statement - scoped grouping collide with a top-level grouping.", NULL);

    /* collision of module's node with submodule's top-level */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule b {belongs-to a {prefix a;} grouping x;}");
    str = "module a {namespace urn:a; prefix a; include b; container c {grouping x;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"x\" of grouping statement - scoped grouping collide with a top-level grouping.", NULL);

    /* collision of submodule's node with another submodule's top-level */
    str = "module a {yang-version 1.1; namespace urn:a; prefix a; include asub; include bsub;}";
    list[0].data = "submodule asub {belongs-to a {prefix a;} grouping g;}";
    list[1].data = "submodule bsub {belongs-to a {prefix a;} container c {grouping g;}}";
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, module_clb, list);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"g\" of grouping statement - scoped grouping collide with a top-level grouping.", NULL);

    /* collision of module's top-levels */
    str = "module a {namespace urn:a; prefix a; grouping test; grouping test;}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"test\" of grouping statement - name collision with another top-level grouping.", NULL);

    /* collision of submodule's top-levels */
    submod = "submodule asub {belongs-to a {prefix a;} grouping g; grouping g;}";
    str = "module a {yang-version 1.1; namespace urn:a; prefix a; include asub;}";
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, submod);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"g\" of grouping statement - name collision with another top-level grouping.", NULL);

    /* collision of module's top-level with submodule's top-levels */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule b {belongs-to a {prefix a;} grouping x;}");
    str = "module a {namespace urn:a; prefix a; include b; grouping x;}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"x\" of grouping statement - name collision with another top-level grouping.", NULL);

    /* collision of submodule's top-level with another submodule's top-levels */
    str = "module a {yang-version 1.1; namespace urn:a; prefix a; include asub; include bsub;}";
    list[0].data = "submodule asub {belongs-to a {prefix a;} grouping g;}";
    list[1].data = "submodule bsub {belongs-to a {prefix a;} grouping g;}";
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, module_clb, list);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"g\" of grouping statement - name collision with another top-level grouping.", NULL);

    /* collision in nested groupings, top-level */
    str = "module a {namespace urn:a; prefix a; grouping g {grouping g;}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"g\" of grouping statement - scoped grouping collide with a top-level grouping.", NULL);

    /* collision in nested groupings, in node */
    str = "module a {namespace urn:a; prefix a; container c {grouping g {grouping g;}}}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"g\" of grouping statement - name collision with another scoped grouping.", NULL);

    /* no collision if the same names are in different scope */
    str = "module a {yang-version 1.1; namespace urn:a; prefix a;"
            "container c {grouping g;} container d {grouping g;}}";
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Locally scoped grouping \"g\" not used.", NULL);
    CHECK_LOG_CTX("Locally scoped grouping \"g\" not used.", NULL);
}

static void
test_collision_identity(void **state)
{
    const char *str;
    char *submod;
    struct module_clb_list list[3] = {0};

    list[0].name = "asub";
    list[1].name = "bsub";

    /* collision of module's top-levels */
    str = "module a {yang-version 1.1; namespace urn:a; prefix a; identity g; identity g;}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"g\" of identity statement - name collision with another top-level identity.", NULL);

    /* collision of submodule's top-levels */
    submod = "submodule asub {belongs-to a {prefix a;} identity g; identity g;}";
    str = "module a {yang-version 1.1; namespace urn:a; prefix a; include asub;}";
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, submod);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"g\" of identity statement - name collision with another top-level identity.", NULL);

    /* collision of module's top-level with submodule's top-levels */
    submod = "submodule asub {belongs-to a {prefix a;} identity g;}";
    str = "module a {yang-version 1.1; namespace urn:a; prefix a; include asub; identity g;}";
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, submod);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"g\" of identity statement - name collision with another top-level identity.", NULL);

    /* collision of submodule's top-level with another submodule's top-levels */
    str = "module a {yang-version 1.1; namespace urn:a; prefix a; include asub; include bsub;}";
    list[0].data = "submodule asub {belongs-to a {prefix a;} identity g;}";
    list[1].data = "submodule bsub {belongs-to a {prefix a;} identity g;}";
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, module_clb, list);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"g\" of identity statement - name collision with another top-level identity.", NULL);
}

static void
test_collision_feature(void **state)
{
    const char *str;
    char *submod;
    struct module_clb_list list[3] = {0};

    list[0].name = "asub";
    list[1].name = "bsub";

    /* collision of module's top-levels */
    str = "module a {yang-version 1.1; namespace urn:a; prefix a; feature g; feature g;}";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"g\" of feature statement - name collision with another top-level feature.", NULL);

    /* collision of submodule's top-levels */
    submod = "submodule asub {belongs-to a {prefix a;} feature g; feature g;}";
    str = "module a {yang-version 1.1; namespace urn:a; prefix a; include asub;}";
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, submod);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"g\" of feature statement - name collision with another top-level feature.", NULL);

    /* collision of module's top-level with submodule's top-levels */
    submod = "submodule asub {belongs-to a {prefix a;} feature g;}";
    str = "module a {yang-version 1.1; namespace urn:a; prefix a; include asub; feature g;}";
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, submod);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"g\" of feature statement - name collision with another top-level feature.", NULL);

    /* collision of submodule's top-level with another submodule's top-levels */
    str = "module a {yang-version 1.1; namespace urn:a; prefix a; include asub; include bsub;}";
    list[0].data = "submodule asub {belongs-to a {prefix a;} feature g;}";
    list[1].data = "submodule bsub {belongs-to a {prefix a;} feature g;}";
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, module_clb, list);
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"a\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate identifier \"g\" of feature statement - name collision with another top-level feature.", NULL);
}

static void
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
            "    namespace urn:b;\n"
            "    prefix b;\n"
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
            " (as the leafref does), but it does not.", "Schema location \"/b:cont2/l2\".");

    /* config -> state must */
    str = "module b {\n"
            "    namespace urn:b;\n"
            "    prefix b;\n"
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
    CHECK_LOG_CTX("Schema node \"cont\" for parent \"<config-root>\" not found; in expr \"../../cont\" "
            "with context node \"/b:cont2/l2\".", NULL);

    /* state -> config */
    str = "module c {\n"
            "    namespace urn:c;\n"
            "    prefix c;\n"
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
            "    namespace urn:d;\n"
            "    prefix d;\n"
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
            "    namespace urn:e;\n"
            "    prefix e;\n"
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
            "    namespace urn:f;\n"
            "    prefix f;\n"
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
            "    namespace urn:g;\n"
            "    prefix g;\n"
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
            "    namespace urn:h;\n"
            "    prefix h;\n"
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
    CHECK_LOG_CTX("Not found node \"l\" in path.", "Schema location \"/h:rp/input/l2\".");

    /* rpc input -> rpc output must */
    str = "module h {\n"
            "    namespace urn:h;\n"
            "    prefix h;\n"
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
    CHECK_LOG_CTX("Schema node \"l\" for parent \"/h:rp\" not found; in expr \"../l\" with context node \"/h:rp/input/l2\".", NULL);

    /* rpc input -> notif leafref */
    str = "module i {\n"
            "    namespace urn:i;\n"
            "    prefix i;\n"
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
    CHECK_LOG_CTX("Not found node \"notif\" in path.", "Schema location \"/i:rp/input/l2\".");

    /* rpc input -> notif must */
    str = "module i {\n"
            "    namespace urn:i;\n"
            "    prefix i;\n"
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
    CHECK_LOG_CTX("Schema node \"notif\" for parent \"<root>\" not found; in expr \"/notif\" "
            "with context node \"/i:rp/input/l2\".", NULL);

    /* action output -> state */
    str = "module j {\n"
            "    yang-version 1.1;\n"
            "    namespace urn:j;\n"
            "    prefix j;\n"
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
            "    namespace urn:k;\n"
            "    prefix k;\n"
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
    CHECK_LOG_CTX("Not found node \"l\" in path.", "Schema location \"/k:cont/ll/act/output/l2\".");

    /* action output -> action input must */
    str = "module k {\n"
            "    yang-version 1.1;\n"
            "    namespace urn:k;\n"
            "    prefix k;\n"
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
    CHECK_LOG_CTX("Schema node \"l\" for parent \"/k:cont/ll/act\" not found; in expr \"/cont/ll/act/l\" "
            "with context node \"/k:cont/ll/act/output/l2\".", NULL);
}

static void
test_includes(void **state)
{
    struct lys_module *mod;

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
        CHECK_LOG_CTX("Loading \"main_b\" module failed.", NULL);
        CHECK_LOG_CTX("Data model \"main_b\" not found in local searchdirs.", NULL);
        CHECK_LOG_CTX("Parsing module \"main_b\" failed.", NULL);
        CHECK_LOG_CTX("Including \"sub_b_one\" submodule into \"main_b\" failed.", NULL);
        CHECK_LOG_CTX("Data model \"sub_b_one\" not found in local searchdirs.", NULL);
        CHECK_LOG_CTX("Parsing submodule \"sub_b_one\" failed.", NULL);
        CHECK_LOG_CTX("YANG 1.1 requires all submodules to be included from main module. But submodule \"sub_b_one\" includes "
                "submodule \"sub_b_two\" which is not included by main module \"main_b\".", NULL);
        CHECK_LOG_CTX("YANG version 1.1 expects all includes in main module, includes in submodules (sub_b_one) are not necessary.", NULL);
    }

    {
        /* YANG 1.1 - all includes are in main_c, includes in submodules are not necessary, so expect warning */
        struct module_clb_list list[] = {
            {"main_c", "module main_c { yang-version 1.1; namespace urn:test:main_c; prefix mc; include sub_c_one; include sub_c_two;}"},
            {"sub_c_one", "submodule sub_c_one { yang-version 1.1; belongs-to main_c { prefix mc; } include sub_c_two;}"},
            {"sub_c_two", "submodule sub_c_two { yang-version 1.1; belongs-to main_c { prefix mc; } include sub_c_one;}"},
            {NULL, NULL}
        };

        ly_ctx_set_module_imp_clb(UTEST_LYCTX, module_clb, list);
        mod = ly_ctx_load_module(UTEST_LYCTX, "main_c", NULL, NULL);
        assert_non_null(mod);
        assert_int_equal(2, LY_ARRAY_COUNT(mod->parsed->includes));
        assert_false(mod->parsed->includes[1].injected);
        /* result is ok, but log includes the warning */
        CHECK_LOG_CTX("YANG version 1.1 expects all includes in main module, includes in submodules (sub_c_two) are not necessary.", NULL);
        CHECK_LOG_CTX("YANG version 1.1 expects all includes in main module, includes in submodules (sub_c_one) are not necessary.", NULL);
    }
}

static void
test_key_order(void **state)
{
    struct lys_module *mod;
    const struct lysc_node *node;

    struct module_clb_list list1[] = {
        {
            "a", "module a {"
            "yang-version 1.1;"
            "namespace urn:test:a;"
            "prefix a;"
            "list l {"
            "  key \"k1 k2\";"
            "  leaf k2 {type string;}"
            "  leaf k1 {type string;}"
            "}"
            "}"
        },
        {NULL, NULL}
    };

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, module_clb, list1);
    mod = ly_ctx_load_module(UTEST_LYCTX, "a", NULL, NULL);
    assert_non_null(mod);

    node = lysc_node_child(mod->compiled->data);
    assert_string_equal("k1", node->name);
    node = node->next;
    assert_string_equal("k2", node->name);

    struct module_clb_list list2[] = {
        {
            "b", "module b {"
            "yang-version 1.1;"
            "namespace urn:test:b;"
            "prefix b;"
            "list l {"
            "  key \"k1 k2 k3 k4\";"
            "  leaf k4 {type string;}"
            "  container c {"
            "    leaf l1 {type string;}"
            "  }"
            "  leaf k2 {type string;}"
            "  leaf l2 {type string;}"
            "  leaf k1 {type string;}"
            "  leaf k3 {type string;}"
            "}"
            "}"
        },
        {NULL, NULL}
    };

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, module_clb, list2);
    mod = ly_ctx_load_module(UTEST_LYCTX, "b", NULL, NULL);
    assert_non_null(mod);

    node = lysc_node_child(mod->compiled->data);
    assert_string_equal("k1", node->name);
    node = node->next;
    assert_string_equal("k2", node->name);
    node = node->next;
    assert_string_equal("k3", node->name);
    node = node->next;
    assert_string_equal("k4", node->name);
}

static void
test_disabled_enum(void **state)
{
    const char *str;

    /* no enabled enum */
    str = "module a {"
            "yang-version 1.1;"
            "namespace urn:test:a;"
            "prefix a;"
            "feature f;"
            "leaf l {type enumeration {"
            "  enum e1 {if-feature f;}"
            "  enum e2 {if-feature f;}"
            "}}"
            "}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Node \"l\" without any (or all disabled) valid values.", "Schema location \"/a:l\".");

    /* disabled default value */
    str = "module a {"
            "yang-version 1.1;"
            "namespace urn:test:a;"
            "prefix a;"
            "feature f;"
            "leaf l {"
            "  type enumeration {"
            "    enum e1 {if-feature f;}"
            "    enum e2;"
            "  }"
            "  default e1;"
            "}"
            "}";
    assert_int_equal(lys_parse_mem(UTEST_LYCTX, str, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid default - value does not fit the type (Invalid enumeration value \"e1\".).",
            "Schema location \"/a:l\".");
}

static void
test_identity(void **state)
{
    struct lys_module *mod, *mod_imp;

    /*
     * parsing YANG
     */
    TEST_STMT_DUP(1, 0, "identity id", "description", "a", "b", "1");
    TEST_STMT_DUP(1, 0, "identity id", "reference", "a", "b", "1");
    TEST_STMT_DUP(1, 0, "identity id", "status", "current", "obsolete", "1");

    /* full content */
    TEST_SCHEMA_OK(1, 0, "identityone",
            "identity test {base \"a\";base b; description text;reference \'another text\';status current; if-feature x;if-feature y; identityone:ext;}"
            "identity a; identity b; extension ext; feature x; feature y;", mod);
    assert_non_null(mod->parsed->identities);
    assert_int_equal(3, LY_ARRAY_COUNT(mod->parsed->identities));

    /* invalid substatement */
    TEST_STMT_SUBSTM_ERR(0, "identity", "organization", "XXX");

    /*
     * parsing YIN
     */
    /* max subelems */
    TEST_SCHEMA_OK(1, 1, "identityone-yin", "<identity name=\"ident-name\">"
            "<if-feature name=\"iff\"/>"
            "<base name=\"base-name\"/>"
            "<status value=\"deprecated\"/>"
            "<description><text>desc</text></description>"
            "<reference><text>ref</text></reference>"
            /* TODO yin-extension-prefix-compilation-bug "<myext:ext xmlns:myext=\"urn:libyang:test:identityone-yin\"/>" */
            "</identity><extension name=\"ext\"/><identity name=\"base-name\"/><feature name=\"iff\"/>", mod);
    assert_int_equal(2, LY_ARRAY_COUNT(mod->parsed->identities));
    assert_string_equal(mod->parsed->identities[0].name, "ident-name");
    assert_string_equal(mod->parsed->identities[0].bases[0], "base-name");
    assert_string_equal(mod->parsed->identities[0].iffeatures[0].str, "iff");
    assert_string_equal(mod->parsed->identities[0].dsc, "desc");
    assert_string_equal(mod->parsed->identities[0].ref, "ref");
    assert_true(mod->parsed->identities[0].flags & LYS_STATUS_DEPRC);
    /*assert_string_equal(mod->parsed->identities[0].exts[0].name, "ext");
    assert_non_null(mod->parsed->identities[0].exts[0].compiled);
    assert_int_equal(mod->parsed->identities[0].exts[0].yin, 1);
    assert_int_equal(mod->parsed->identities[0].exts[0].insubstmt_index, 0);
    assert_int_equal(mod->parsed->identities[0].exts[0].insubstmt, LYEXT_SUBSTMT_SELF);*/

    /* min subelems */
    TEST_SCHEMA_OK(1, 1, "identitytwo-yin", "<identity name=\"ident-name\" />", mod);
    assert_int_equal(1, LY_ARRAY_COUNT(mod->parsed->identities));
    assert_string_equal(mod->parsed->identities[0].name, "ident-name");

    /* invalid substatement */
    TEST_SCHEMA_PARSE_ERR(0, 1, "inv", "<identity name=\"ident-name\"><if-feature name=\"iff\"/></identity>",
            "Invalid sub-elemnt \"if-feature\" of \"identity\" element - "
            "this sub-element is allowed only in modules with version 1.1 or newer.", "Line number 1.");

    /*
     * compiling
     */
    TEST_SCHEMA_OK(0, 0, "a", "identity a1;", mod_imp);
    TEST_SCHEMA_OK(1, 0, "b", "import a {prefix a;}"
            "identity b1; identity b2; identity b3 {base b1; base b:b2; base a:a1;}"
            "identity b4 {base b:b1; base b3;}", mod);
    assert_non_null(mod_imp->compiled);
    assert_non_null(mod_imp->identities);
    assert_non_null(mod->identities);
    assert_non_null(mod_imp->identities[0].derived);
    assert_int_equal(1, LY_ARRAY_COUNT(mod_imp->identities[0].derived));
    assert_ptr_equal(mod_imp->identities[0].derived[0], &mod->identities[2]);
    assert_non_null(mod->identities[0].derived);
    assert_int_equal(2, LY_ARRAY_COUNT(mod->identities[0].derived));
    assert_ptr_equal(mod->identities[0].derived[0], &mod->identities[2]);
    assert_ptr_equal(mod->identities[0].derived[1], &mod->identities[3]);
    assert_non_null(mod->identities[1].derived);
    assert_int_equal(1, LY_ARRAY_COUNT(mod->identities[1].derived));
    assert_ptr_equal(mod->identities[1].derived[0], &mod->identities[2]);
    assert_non_null(mod->identities[2].derived);
    assert_int_equal(1, LY_ARRAY_COUNT(mod->identities[2].derived));
    assert_ptr_equal(mod->identities[2].derived[0], &mod->identities[3]);

    TEST_SCHEMA_OK(1, 0, "c", "identity c2 {base c1;} identity c1;", mod);
    assert_int_equal(1, LY_ARRAY_COUNT(mod->identities[1].derived));
    assert_ptr_equal(mod->identities[1].derived[0], &mod->identities[0]);

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule inv_sub {belongs-to inv {prefix inv;} identity i1;}");
    TEST_SCHEMA_ERR(0, 0, "inv", "identity i1 {base i2;}", "Unable to find base (i2) of identity \"i1\".", "Path \"/inv:{identity='i1'}\".");
    TEST_SCHEMA_ERR(0, 0, "inv", "identity i1 {base i1;}", "Identity \"i1\" is derived from itself.", "Path \"/inv:{identity='i1'}\".");
    TEST_SCHEMA_ERR(0, 0, "inv", "identity i1 {base i2;}identity i2 {base i3;}identity i3 {base i1;}",
            "Identity \"i1\" is indirectly derived from itself.", "Path \"/inv:{identity='i3'}\".");

    /* base in non-implemented module */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb,
            "module base {namespace \"urn\"; prefix b; identity i1; identity i2 {base i1;}}");
    TEST_SCHEMA_OK(0, 0, "ident", "import base {prefix b;} identity ii {base b:i1;}", mod);

    /* default value from non-implemented module */
    TEST_SCHEMA_ERR(0, 0, "ident2", "import base {prefix b;} leaf l {type identityref {base b:i1;} default b:i2;}",
            "Invalid default - value does not fit the type (Invalid identityref \"b:i2\" value"
            " - identity found in non-implemented module \"base\".).", "Schema location \"/ident2:l\".");

    /* default value in typedef from non-implemented module */
    TEST_SCHEMA_ERR(0, 0, "ident2", "import base {prefix b;} typedef t1 {type identityref {base b:i1;} default b:i2;}"
            "leaf l {type t1;}", "Invalid default - value does not fit the type (Invalid"
            " identityref \"b:i2\" value - identity found in non-implemented module \"base\".).", "Schema location \"/ident2:l\".");

    /*
     * printing
     */

    /*
     * cleanup
     */
}

static void
test_feature(void **state)
{
    struct lys_module *mod;
    const struct lysp_feature *f;

    /*
     * parsing YANG
     */

    TEST_STMT_DUP(1, 0, "feature f", "description", "a", "b", "1");
    TEST_STMT_DUP(1, 0, "feature f", "reference", "a", "b", "1");
    TEST_STMT_DUP(1, 0, "feature f", "status", "current", "obsolete", "1");

    /* full content */
    TEST_SCHEMA_OK(1, 0, "featureone",
            "feature test {description text;reference \'another text\';status current; if-feature x; if-feature y; featureone:ext;}"
            "extension ext; feature x; feature y;", mod);
    assert_non_null(mod->parsed->features);
    assert_int_equal(3, LY_ARRAY_COUNT(mod->parsed->features));

    /* invalid substatement */
    TEST_STMT_SUBSTM_ERR(0, "feature", "organization", "XXX");

    /*
     * parsing YIN
     */
    /* max subelems */
    TEST_SCHEMA_OK(0, 1, "featureone-yin", "<feature name=\"feature-name\">"
            "<if-feature name=\"iff\"/>"
            "<status value=\"deprecated\"/>"
            "<description><text>desc</text></description>"
            "<reference><text>ref</text></reference>"
            /* TODO yin-extension-prefix-compilation-bug "<myext:ext xmlns:myext=\"urn:libyang:test:featureone-yin\"/>" */
            "</feature><extension name=\"ext\"/><feature name=\"iff\"/>", mod);
    assert_int_equal(2, LY_ARRAY_COUNT(mod->parsed->features));
    assert_string_equal(mod->parsed->features[0].name, "feature-name");
    assert_string_equal(mod->parsed->features[0].dsc, "desc");
    assert_true(mod->parsed->features[0].flags & LYS_STATUS_DEPRC);
    assert_string_equal(mod->parsed->features[0].iffeatures[0].str, "iff");
    assert_string_equal(mod->parsed->features[0].ref, "ref");
    /*assert_string_equal(mod->parsed->features[0].exts[0].name, "ext");
    assert_int_equal(mod->parsed->features[0].exts[0].insubstmt_index, 0);
    assert_int_equal(mod->parsed->features[0].exts[0].insubstmt, LYEXT_SUBSTMT_SELF);*/

    /* min subelems */
    TEST_SCHEMA_OK(0, 1, "featuretwo-yin", "<feature name=\"feature-name\"/>", mod)
    assert_int_equal(1, LY_ARRAY_COUNT(mod->parsed->features));
    assert_string_equal(mod->parsed->features[0].name, "feature-name");

    /* invalid substatement */
    TEST_SCHEMA_PARSE_ERR(0, 1, "inv", "<feature name=\"feature-name\"><organization><text>org</text></organization></feature>",
            "Unexpected sub-element \"organization\" of \"feature\" element.", "Line number 1.");

    /*
     * compiling
     */

    TEST_SCHEMA_OK(1, 0, "a", "feature f1 {description test1;reference test2;status current;} feature f2; feature f3;\n"
            "feature orfeature {if-feature \"f1 or f2\";}\n"
            "feature andfeature {if-feature \"f1 and f2\";}\n"
            "feature f6 {if-feature \"not f1\";}\n"
            "feature f7 {if-feature \"(f2 and f3) or (not f1)\";}\n"
            "feature f8 {if-feature \"f1 or f2 or f3 or orfeature or andfeature\";}\n"
            "feature f9 {if-feature \"not not f1\";}", mod);
    assert_non_null(mod->parsed->features);
    assert_int_equal(9, LY_ARRAY_COUNT(mod->parsed->features));

    /* all features are disabled by default */
    LY_ARRAY_FOR(mod->parsed->features, struct lysp_feature, f) {
        assert_false(f->flags & LYS_FENABLED);
    }

    /* some invalid expressions */
    TEST_SCHEMA_PARSE_ERR(1, 0, "inv", "feature f{if-feature f1;}",
            "Invalid value \"f1\" of if-feature - unable to find feature \"f1\".", NULL);
    TEST_SCHEMA_PARSE_ERR(1, 0, "inv", "feature f1; feature f2{if-feature 'f and';}",
            "Invalid value \"f and\" of if-feature - unexpected end of expression.", NULL);
    TEST_SCHEMA_PARSE_ERR(1, 0, "inv", "feature f{if-feature 'or';}",
            "Invalid value \"or\" of if-feature - unexpected end of expression.", NULL);
    TEST_SCHEMA_PARSE_ERR(1, 0, "inv", "feature f1; feature f2{if-feature '(f1';}",
            "Invalid value \"(f1\" of if-feature - non-matching opening and closing parentheses.", NULL);
    TEST_SCHEMA_PARSE_ERR(1, 0, "inv", "feature f1; feature f2{if-feature 'f1)';}",
            "Invalid value \"f1)\" of if-feature - non-matching opening and closing parentheses.", NULL);
    TEST_SCHEMA_PARSE_ERR(1, 0, "inv", "feature f1; feature f2{if-feature ---;}",
            "Invalid value \"---\" of if-feature - unable to find feature \"---\".", NULL);
    TEST_SCHEMA_PARSE_ERR(0, 0, "inv", "feature f1; feature f2{if-feature 'not f1';}",
            "Invalid value \"not f1\" of if-feature - YANG 1.1 expression in YANG 1.0 module.", NULL);

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, "submodule inv_sub {belongs-to inv {prefix inv;} feature f1;}");
    TEST_SCHEMA_PARSE_ERR(0, 0, "inv", "feature f1 {if-feature f2;} feature f2 {if-feature f1;}",
            "Feature \"f1\" is indirectly referenced from itself.", NULL);
    TEST_SCHEMA_PARSE_ERR(0, 0, "inv", "feature f1 {if-feature f1;}",
            "Feature \"f1\" is referenced from itself.", NULL);
    TEST_SCHEMA_PARSE_ERR(1, 0, "inv", "feature f {if-feature ();}",
            "Invalid value \"()\" of if-feature - number of features in expression does not match the required number of operands for the operations.", NULL);
    TEST_SCHEMA_PARSE_ERR(1, 0, "inv", "feature f1; feature f {if-feature 'f1(';}",
            "Invalid value \"f1(\" of if-feature - non-matching opening and closing parentheses.", NULL);
    TEST_SCHEMA_PARSE_ERR(1, 0, "inv", "feature f1; feature f {if-feature 'and f1';}",
            "Invalid value \"and f1\" of if-feature - missing feature/expression before \"and\" operation.", NULL);
    TEST_SCHEMA_PARSE_ERR(1, 0, "inv", "feature f1; feature f {if-feature 'f1 not ';}",
            "Invalid value \"f1 not \" of if-feature - unexpected end of expression.", NULL);
    TEST_SCHEMA_PARSE_ERR(1, 0, "inv", "feature f1; feature f {if-feature 'f1 not not ';}",
            "Invalid value \"f1 not not \" of if-feature - unexpected end of expression.", NULL);
    TEST_SCHEMA_PARSE_ERR(1, 0, "inv", "feature f1; feature f2; feature f {if-feature 'or f1 f2';}",
            "Invalid value \"or f1 f2\" of if-feature - missing feature/expression before \"or\" operation.", NULL);

    /*
     * printing
     */

    /*
     * cleanup
     */
}

static void
test_extension_argument(void **state)
{
    struct lys_module *mod;
    const char *mod_def_yang = "module a {\n"
            "  namespace \"urn:a\";\n"
            "  prefix a;\n\n"
            "  extension e {\n"
            "    argument name;\n"
            "  }\n\n"
            "  a:e \"aaa\";\n"
            "}\n";
    const char *mod_def_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"a\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:a\"/>\n"
            "  <prefix value=\"a\"/>\n"
            "  <extension name=\"e\">\n"
            "    <argument name=\"name\"/>\n"
            "  </extension>\n"
            "  <a:e name=\"aaa\"/>\n"
            "</module>\n";
    const char *mod_test_yin, *mod_test_yang;
    char *printed;

    mod_test_yang = "module b {\n"
            "  namespace \"urn:b\";\n"
            "  prefix b;\n\n"
            "  import a {\n"
            "    prefix a;\n"
            "  }\n\n"
            "  a:e \"xxx\";\n"
            "}\n";
    mod_test_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"b\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:b=\"urn:b\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:b\"/>\n"
            "  <prefix value=\"b\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n"
            "  <a:e name=\"xxx\"/>\n"
            "</module>\n";

    /* from YANG */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, (void *)mod_def_yang);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, mod_test_yang, LYS_IN_YANG, &mod));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_test_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_test_yin);
    free(printed);

    assert_non_null(mod = ly_ctx_get_module(UTEST_LYCTX, "a", NULL));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_def_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_def_yin);
    free(printed);

    /* context reset */
    ly_ctx_destroy(UTEST_LYCTX);
    ly_ctx_new(NULL, 0, &UTEST_LYCTX);

    /* from YIN */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, (void *)mod_def_yin);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, &mod));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_test_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_test_yin);
    free(printed);

    assert_non_null(mod = ly_ctx_get_module(UTEST_LYCTX, "a", NULL));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_def_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_def_yin);
    free(printed);
}

static void
test_extension_argument_element(void **state)
{
    struct lys_module *mod;
    const char *mod_def_yang = "module a {\n"
            "  namespace \"urn:a\";\n"
            "  prefix a;\n\n"
            "  extension e {\n"
            "    argument name {\n"
            "      yin-element true;\n"
            "    }\n"
            "  }\n\n"
            "  a:e \"aaa\";\n"
            "}\n";
    const char *mod_def_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"a\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:a\"/>\n"
            "  <prefix value=\"a\"/>\n"
            "  <extension name=\"e\">\n"
            "    <argument name=\"name\">\n"
            "      <yin-element value=\"true\"/>\n"
            "    </argument>\n"
            "  </extension>\n"
            "  <a:e>\n"
            "    <a:name>aaa</a:name>\n"
            "  </a:e>\n"
            "</module>\n";
    const char *mod_test_yin, *mod_test_yang;
    char *printed;

    mod_test_yang = "module b {\n"
            "  namespace \"urn:b\";\n"
            "  prefix b;\n\n"
            "  import a {\n"
            "    prefix a;\n"
            "  }\n\n"
            "  a:e \"xxx\";\n"
            "}\n";
    mod_test_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"b\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:b=\"urn:b\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:b\"/>\n"
            "  <prefix value=\"b\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n"
            "  <a:e>\n"
            "    <a:name>xxx</a:name>\n"
            "  </a:e>\n"
            "</module>\n";

    /* from YANG */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, (void *)mod_def_yang);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, mod_test_yang, LYS_IN_YANG, &mod));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_test_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_test_yin);
    free(printed);

    assert_non_null(mod = ly_ctx_get_module(UTEST_LYCTX, "a", NULL));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_def_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_def_yin);
    free(printed);

    /* context reset */
    ly_ctx_destroy(UTEST_LYCTX);
    ly_ctx_new(NULL, 0, &UTEST_LYCTX);

    /* from YIN */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, (void *)mod_def_yin);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, &mod));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_test_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_test_yin);
    free(printed);

    assert_non_null(mod = ly_ctx_get_module(UTEST_LYCTX, "a", NULL));
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, mod_def_yang);
    free(printed);

    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, mod_def_yin);
    free(printed);

    /* invalid */
    mod_test_yang = "module x { namespace \"urn:x\"; prefix x; import a { prefix a; } a:e; }";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, mod_test_yang, LYS_IN_YANG, NULL));
    CHECK_LOG_CTX("Parsing module \"x\" failed.", NULL);
    CHECK_LOG_CTX("Extension instance \"a:e\" missing argument element \"name\".", NULL);

    mod_test_yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"x\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:x=\"urn:x\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:x\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n\n"
            "  <a:e/>\n"
            "</module>\n";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"x\" failed.", NULL);
    CHECK_LOG_CTX("Extension instance \"a:e\" missing argument element \"name\".", NULL);

    mod_test_yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"x\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:x=\"urn:x\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:x\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n\n"
            "  <a:e name=\"xxx\"/>\n"
            "</module>\n";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"x\" failed.", NULL);
    CHECK_LOG_CTX("Extension instance \"a:e\" missing argument element \"name\".", NULL);

    mod_test_yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"x\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:x=\"urn:x\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:x\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n\n"
            "  <a:e>\n"
            "    <x:name>xxx</x:name>\n"
            "  </a:e>\n"
            "</module>\n";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"x\" failed.", NULL);
    CHECK_LOG_CTX("Extension instance \"a:e\" element and its argument element \"name\" are expected in the same namespace, but they differ.",
            NULL);

    mod_test_yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"x\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:x=\"urn:x\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:x\"/>\n"
            "  <prefix value=\"x\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n\n"
            "  <a:e>\n"
            "    <a:value>xxx</a:value>\n"
            "  </a:e>\n"
            "</module>\n";
    assert_int_equal(LY_EVALID, lys_parse_mem(UTEST_LYCTX, mod_test_yin, LYS_IN_YIN, NULL));
    CHECK_LOG_CTX("Parsing module \"x\" failed.", NULL);
    CHECK_LOG_CTX("Extension instance \"a:e\" expects argument element \"name\" as its first XML child, but \"value\" element found.",
            NULL);

}

static void
test_extension_compile(void **state)
{
    struct lys_module *mod;
    struct lysc_ctx cctx = {0};
    struct lysp_ext_instance ext_p = {0};
    struct lysp_ext_substmt *substmtp;
    struct lysp_stmt child = {0};
    struct lysc_ext_instance ext_c = {0};
    struct lysc_ext_substmt *substmt;
    LY_ERR rc = LY_SUCCESS;

    /* current module, whatever */
    mod = ly_ctx_get_module_implemented(UTEST_LYCTX, "yang");
    assert_true(mod);

    /* compile context */
    cctx.ctx = UTEST_LYCTX;
    cctx.cur_mod = mod;
    cctx.pmod = mod->parsed;
    cctx.path_len = 1;
    cctx.path[0] = '/';

    /* parsed ext instance */
    lydict_insert(UTEST_LYCTX, "pref:my-ext", 0, &ext_p.name);
    ext_p.format = LY_VALUE_JSON;
    ext_p.parent_stmt = LY_STMT_MODULE;

    LY_ARRAY_NEW_GOTO(UTEST_LYCTX, ext_p.substmts, substmtp, rc, cleanup);

    substmtp->stmt = LY_STMT_ERROR_MESSAGE;
    substmtp->storage = &ext_p.parsed;
    /* fake parse */
    lydict_insert(UTEST_LYCTX, "my error", 0, (const char **)&ext_p.parsed);

    /* compiled ext instance */
    ext_c.parent_stmt = ext_p.parent_stmt;
    LY_ARRAY_NEW_GOTO(UTEST_LYCTX, ext_c.substmts, substmt, rc, cleanup);

    substmt->stmt = LY_STMT_ERROR_MESSAGE;
    substmt->storage = &ext_c.compiled;

    /*
     * error-message
     */
    ext_p.child = &child;
    lydict_insert(UTEST_LYCTX, "error-message", 0, &child.stmt);
    lydict_insert(UTEST_LYCTX, "my error", 0, &child.arg);
    child.format = LY_VALUE_JSON;
    child.kw = LY_STMT_ERROR_MESSAGE;

    /* compile */
    assert_int_equal(LY_SUCCESS, lyplg_ext_compile_extension_instance(&cctx, &ext_p, &ext_c));

    /* check */
    assert_string_equal(ext_c.compiled, "my error");

cleanup:
    lydict_remove(UTEST_LYCTX, ext_p.name);
    lydict_remove(UTEST_LYCTX, child.stmt);
    lydict_remove(UTEST_LYCTX, child.arg);
    LY_ARRAY_FREE(ext_p.substmts);
    lydict_remove(UTEST_LYCTX, ext_p.parsed);
    LY_ARRAY_FREE(ext_c.substmts);
    lydict_remove(UTEST_LYCTX, ext_c.compiled);
    if (rc) {
        fail();
    }
}

static void
test_ext_recursive(void **state)
{
    const char *mod_base_yang, *mod_imp_yang, *mod_base_yin, *mod_imp_yin;

    mod_imp_yang = "module b {\n"
            "  namespace \"urn:b\";\n"
            "  prefix b;\n\n"
            "  extension use-in {\n"
            "    argument name {\n"
            "      b:arg-type {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "    b:use-in \"extension\";\n"
            "    b:occurence \"*\";\n"
            "  }\n"
            "\n"
            "  extension substatement {\n"
            "    argument name {\n"
            "      b:arg-type {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "    b:use-in \"extension\";\n"
            "    b:occurence \"*\";\n"
            "    b:substatement \"b:occurence\";\n"
            "  }\n"
            "\n"
            "  extension arg-type {\n"
            "    b:use-in \"argument\";\n"
            "    b:substatement \"type\" {\n"
            "      b:occurence \"1\";\n"
            "    }\n"
            "    b:substatement \"default\";\n"
            "  }\n"
            "\n"
            "  extension occurence {\n"
            "    argument value {\n"
            "      b:arg-type {\n"
            "        type enumeration {\n"
            "          enum \"?\";\n"
            "          enum \"*\";\n"
            "          enum \"+\";\n"
            "          enum \"1\";\n"
            "        }\n"
            "      }\n"
            "    }\n"
            "    b:use-in \"extension\";\n"
            "  }\n"
            "}\n";

    mod_imp_yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"b\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:b=\"urn:b\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:b\"/>\n"
            "  <prefix value=\"b\"/>\n"
            "  <import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "  </import>\n\n"
            "  <a:e name=\"xxx\"/>\n"
            "</module>\n";

    mod_base_yang = "module a {\n"
            "  namespace \"urn:a\";\n"
            "  prefix a;\n\n"
            "  import b {\n"
            "    prefix b;\n"
            "  }\n"
            "\n"
            "  extension abstract {\n"
            "    b:use-in \"identity\";\n"
            "  }\n"
            "\n"
            "  identity mount-id;\n"
            "\n"
            "  identity yang-lib-id {\n"
            "    base mount-id;\n"
            "    a:abstract;\n"
            "  }\n"
            "}\n";

    mod_base_yin = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"a\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:a=\"urn:a\">\n"
            "  <namespace uri=\"urn:a\"/>\n"
            "  <prefix value=\"a\"/>\n\n"
            "  <extension name=\"e\">\n"
            "    <argument name=\"name\"/>\n"
            "  </extension>\n\n"
            "  <a:e name=\"aaa\"/>\n"
            "</module>\n";

    /* from YANG */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, (void *)mod_imp_yang);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, mod_base_yang, LYS_IN_YANG, NULL));

    /* context reset */
    ly_ctx_destroy(UTEST_LYCTX);
    ly_ctx_new(NULL, 0, &UTEST_LYCTX);

    /* from YIN */
    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, (void *)mod_imp_yin);
    assert_int_equal(LY_SUCCESS, lys_parse_mem(UTEST_LYCTX, mod_base_yin, LYS_IN_YIN, NULL));
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_getnext),
        UTEST(test_date),
        UTEST(test_revisions),
        UTEST(test_collision_typedef),
        UTEST(test_collision_grouping),
        UTEST(test_collision_identity),
        UTEST(test_collision_feature),
        UTEST(test_accessible_tree),
        UTEST(test_includes),
        UTEST(test_key_order),
        UTEST(test_disabled_enum),
        UTEST(test_identity),
        UTEST(test_feature),
        UTEST(test_extension_argument),
        UTEST(test_extension_argument_element),
        UTEST(test_extension_compile),
        UTEST(test_ext_recursive),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
