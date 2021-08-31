/**
 * @file test_parser_yin.c
 * @author David Sedlák <xsedla1d@stud.fit.vutbr.cz>
 * @brief unit tests for functions from parser_yin.c
 *
 * Copyright (c) 2015 - 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _UTEST_MAIN_
#include "utests.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "in.h"
#include "parser_internal.h"
#include "schema_compile.h"
#include "tree.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xml.h"
#include "xpath.h"

/* copied from parser_yin.c */
enum yin_argument {
    YIN_ARG_UNKNOWN = 0,   /**< parsed argument can not be matched with any supported yin argument keyword */
    YIN_ARG_NAME,          /**< argument name */
    YIN_ARG_TARGET_NODE,   /**< argument target-node */
    YIN_ARG_MODULE,        /**< argument module */
    YIN_ARG_VALUE,         /**< argument value */
    YIN_ARG_TEXT,          /**< argument text */
    YIN_ARG_CONDITION,     /**< argument condition */
    YIN_ARG_URI,           /**< argument uri */
    YIN_ARG_DATE,          /**< argument data */
    YIN_ARG_TAG,           /**< argument tag */
    YIN_ARG_NONE           /**< empty (special value) */
};

struct yin_subelement {
    enum ly_stmt type;      /**< type of keyword */
    void *dest;             /**< meta infromation passed to responsible function (mostly information about where parsed subelement should be stored) */
    uint16_t flags;         /**< describes constraints of subelement can be set to YIN_SUBELEM_MANDATORY, YIN_SUBELEM_UNIQUE, YIN_SUBELEM_FIRST, YIN_SUBELEM_VER2, and YIN_SUBELEM_DEFAULT_TEXT */
};

struct import_meta {
    const char *prefix;             /**< module prefix. */
    struct lysp_import **imports;   /**< imports to add to. */
};

struct yin_argument_meta {
    uint16_t *flags;        /**< Argument flags */
    const char **argument;  /**< Argument value */
};

struct tree_node_meta {
    struct lysp_node *parent;       /**< parent node */
    struct lysp_node **nodes;    /**< linked list of siblings */
};

struct include_meta {
    const char *name;               /**< Module/submodule name. */
    struct lysp_include **includes; /**< [Sized array](@ref sizedarrays) of parsed includes to add to. */
};

struct inout_meta {
    struct lysp_node *parent;          /**< Parent node. */
    struct lysp_node_action_inout *inout_p; /**< inout_p Input/output pointer to write to. */
};

struct minmax_dev_meta {
    uint32_t *lim;                      /**< min/max value to write to. */
    uint16_t *flags;                    /**< min/max flags to write to. */
    struct lysp_ext_instance **exts;    /**< extension instances to add to. */
};

#define YIN_SUBELEM_MANDATORY   0x01
#define YIN_SUBELEM_UNIQUE      0x02
#define YIN_SUBELEM_FIRST       0x04
#define YIN_SUBELEM_VER2        0x08

#define YIN_SUBELEM_PARSED      0x80

/* prototypes of static functions */
enum yin_argument yin_match_argument_name(const char *name, size_t len);
LY_ERR yin_parse_content(struct lys_yin_parser_ctx *ctx, struct yin_subelement *subelem_info, size_t subelem_info_size,
        enum ly_stmt current_element, const char **text_content, struct lysp_ext_instance **exts);
LY_ERR yin_validate_value(struct lys_yin_parser_ctx *ctx, enum yang_arg val_type);
enum ly_stmt yin_match_keyword(struct lys_yin_parser_ctx *ctx, const char *name, size_t name_len,
        const char *prefix, size_t prefix_len, enum ly_stmt parrent);
LY_ERR yin_parse_extension_instance(struct lys_yin_parser_ctx *ctx, enum ly_stmt subelem, LY_ARRAY_COUNT_TYPE subelem_index,
        struct lysp_ext_instance **exts);
LY_ERR yin_parse_element_generic(struct lys_yin_parser_ctx *ctx, enum ly_stmt parent, struct lysp_stmt **element);
LY_ERR yin_parse_mod(struct lys_yin_parser_ctx *ctx, struct lysp_module *mod);
LY_ERR yin_parse_submod(struct lys_yin_parser_ctx *ctx, struct lysp_submodule *submod);

void lysp_ext_instance_free(struct ly_ctx *ctx, struct lysp_ext_instance *ext);
void lysp_ext_free(struct ly_ctx *ctx, struct lysp_ext *ext);
void lysp_when_free(struct ly_ctx *ctx, struct lysp_when *when);
void lysp_type_free(struct ly_ctx *ctx, struct lysp_type *type);
void lysp_node_free(struct ly_ctx *ctx, struct lysp_node *node);
void lysp_tpdf_free(struct ly_ctx *ctx, struct lysp_tpdf *tpdf);
void lysp_refine_free(struct ly_ctx *ctx, struct lysp_refine *ref);
void lysp_revision_free(struct ly_ctx *ctx, struct lysp_revision *rev);
void lysp_include_free(struct ly_ctx *ctx, struct lysp_include *include);
void lysp_feature_free(struct ly_ctx *ctx, struct lysp_feature *feat);
void lysp_ident_free(struct ly_ctx *ctx, struct lysp_ident *ident);
void lysp_grp_free(struct ly_ctx *ctx, struct lysp_node_grp *grp);
void lysp_augment_free(struct ly_ctx *ctx, struct lysp_node_augment *augment);
void lysp_deviate_free(struct ly_ctx *ctx, struct lysp_deviate *d);
void lysp_deviation_free(struct ly_ctx *ctx, struct lysp_deviation *dev);
void lysp_import_free(struct ly_ctx *ctx, struct lysp_import *import);

/* wrapping element used for mocking has nothing to do with real module structure */
#define ELEMENT_WRAPPER_START "<status xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
#define ELEMENT_WRAPPER_END "</status>"

#define TEST_1_CHECK_LYSP_EXT_INSTANCE(NODE, INSUBSTMT)\
    CHECK_LYSP_EXT_INSTANCE((NODE), NULL, 1, INSUBSTMT, 0, "myext:c-define", LY_VALUE_XML)

struct lys_yin_parser_ctx *YCTX;

static int
setup_ctx(void **state)
{
    /* allocate parser context */
    YCTX = calloc(1, sizeof(*YCTX));
    YCTX->format = LYS_IN_YIN;

    /* allocate new parsed module */
    YCTX->parsed_mod = calloc(1, sizeof *YCTX->parsed_mod);

    /* allocate new module */
    YCTX->parsed_mod->mod = calloc(1, sizeof *YCTX->parsed_mod->mod);
    YCTX->parsed_mod->mod->ctx = UTEST_LYCTX;
    YCTX->parsed_mod->mod->parsed = YCTX->parsed_mod;

    return 0;
}

static int
setup(void **state)
{
    UTEST_SETUP;

    setup_ctx(state);

    return 0;
}

static int
teardown_ctx(void **UNUSED(state))
{
    lyxml_ctx_free(YCTX->xmlctx);
    lys_module_free(YCTX->parsed_mod->mod);
    free(YCTX);
    YCTX = NULL;

    return 0;
}

static int
teardown(void **state)
{
    teardown_ctx(state);

    UTEST_TEARDOWN;

    return 0;
}

#define RESET_STATE \
    ly_in_free(UTEST_IN, 0); \
    UTEST_IN = NULL; \
    teardown_ctx(state); \
    setup_ctx(state)

static void
test_yin_match_keyword(void **state)
{
    const char *prefix;
    size_t prefix_len;

    /* create mock yin namespace in xml context */
    ly_in_new_memory("<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" />", &UTEST_IN);
    lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx);
    prefix = YCTX->xmlctx->prefix;
    prefix_len = YCTX->xmlctx->prefix_len;

    assert_int_equal(yin_match_keyword(YCTX, "anydatax", strlen("anydatax"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_NONE);
    assert_int_equal(yin_match_keyword(YCTX, "asdasd", strlen("asdasd"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_NONE);
    assert_int_equal(yin_match_keyword(YCTX, "", 0, prefix, prefix_len, LY_STMT_NONE), LY_STMT_NONE);
    assert_int_equal(yin_match_keyword(YCTX, "anydata", strlen("anydata"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_ANYDATA);
    assert_int_equal(yin_match_keyword(YCTX, "anyxml", strlen("anyxml"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_ANYXML);
    assert_int_equal(yin_match_keyword(YCTX, "argument", strlen("argument"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_ARGUMENT);
    assert_int_equal(yin_match_keyword(YCTX, "augment", strlen("augment"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_AUGMENT);
    assert_int_equal(yin_match_keyword(YCTX, "base", strlen("base"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_BASE);
    assert_int_equal(yin_match_keyword(YCTX, "belongs-to", strlen("belongs-to"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_BELONGS_TO);
    assert_int_equal(yin_match_keyword(YCTX, "bit", strlen("bit"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_BIT);
    assert_int_equal(yin_match_keyword(YCTX, "case", strlen("case"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_CASE);
    assert_int_equal(yin_match_keyword(YCTX, "choice", strlen("choice"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_CHOICE);
    assert_int_equal(yin_match_keyword(YCTX, "config", strlen("config"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_CONFIG);
    assert_int_equal(yin_match_keyword(YCTX, "contact", strlen("contact"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_CONTACT);
    assert_int_equal(yin_match_keyword(YCTX, "container", strlen("container"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_CONTAINER);
    assert_int_equal(yin_match_keyword(YCTX, "default", strlen("default"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_DEFAULT);
    assert_int_equal(yin_match_keyword(YCTX, "description", strlen("description"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_DESCRIPTION);
    assert_int_equal(yin_match_keyword(YCTX, "deviate", strlen("deviate"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_DEVIATE);
    assert_int_equal(yin_match_keyword(YCTX, "deviation", strlen("deviation"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_DEVIATION);
    assert_int_equal(yin_match_keyword(YCTX, "enum", strlen("enum"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_ENUM);
    assert_int_equal(yin_match_keyword(YCTX, "error-app-tag", strlen("error-app-tag"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_ERROR_APP_TAG);
    assert_int_equal(yin_match_keyword(YCTX, "error-message", strlen("error-message"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_ERROR_MESSAGE);
    assert_int_equal(yin_match_keyword(YCTX, "extension", strlen("extension"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_EXTENSION);
    assert_int_equal(yin_match_keyword(YCTX, "feature", strlen("feature"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_FEATURE);
    assert_int_equal(yin_match_keyword(YCTX, "fraction-digits", strlen("fraction-digits"), prefix,  prefix_len, LY_STMT_NONE), LY_STMT_FRACTION_DIGITS);
    assert_int_equal(yin_match_keyword(YCTX, "grouping", strlen("grouping"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_GROUPING);
    assert_int_equal(yin_match_keyword(YCTX, "identity", strlen("identity"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_IDENTITY);
    assert_int_equal(yin_match_keyword(YCTX, "if-feature", strlen("if-feature"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_IF_FEATURE);
    assert_int_equal(yin_match_keyword(YCTX, "import", strlen("import"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_IMPORT);
    assert_int_equal(yin_match_keyword(YCTX, "include", strlen("include"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_INCLUDE);
    assert_int_equal(yin_match_keyword(YCTX, "input", strlen("input"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_INPUT);
    assert_int_equal(yin_match_keyword(YCTX, "key", strlen("key"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_KEY);
    assert_int_equal(yin_match_keyword(YCTX, "leaf", strlen("leaf"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_LEAF);
    assert_int_equal(yin_match_keyword(YCTX, "leaf-list", strlen("leaf-list"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_LEAF_LIST);
    assert_int_equal(yin_match_keyword(YCTX, "length", strlen("length"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_LENGTH);
    assert_int_equal(yin_match_keyword(YCTX, "list", strlen("list"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_LIST);
    assert_int_equal(yin_match_keyword(YCTX, "mandatory", strlen("mandatory"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_MANDATORY);
    assert_int_equal(yin_match_keyword(YCTX, "max-elements", strlen("max-elements"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_MAX_ELEMENTS);
    assert_int_equal(yin_match_keyword(YCTX, "min-elements", strlen("min-elements"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_MIN_ELEMENTS);
    assert_int_equal(yin_match_keyword(YCTX, "modifier", strlen("modifier"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_MODIFIER);
    assert_int_equal(yin_match_keyword(YCTX, "module", strlen("module"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_MODULE);
    assert_int_equal(yin_match_keyword(YCTX, "must", strlen("must"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_MUST);
    assert_int_equal(yin_match_keyword(YCTX, "namespace", strlen("namespace"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_NAMESPACE);
    assert_int_equal(yin_match_keyword(YCTX, "notification", strlen("notification"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_NOTIFICATION);
    assert_int_equal(yin_match_keyword(YCTX, "ordered-by", strlen("ordered-by"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_ORDERED_BY);
    assert_int_equal(yin_match_keyword(YCTX, "organization", strlen("organization"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_ORGANIZATION);
    assert_int_equal(yin_match_keyword(YCTX, "output", strlen("output"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_OUTPUT);
    assert_int_equal(yin_match_keyword(YCTX, "path", strlen("path"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_PATH);
    assert_int_equal(yin_match_keyword(YCTX, "pattern", strlen("pattern"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_PATTERN);
    assert_int_equal(yin_match_keyword(YCTX, "position", strlen("position"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_POSITION);
    assert_int_equal(yin_match_keyword(YCTX, "prefix", strlen("prefix"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_PREFIX);
    assert_int_equal(yin_match_keyword(YCTX, "presence", strlen("presence"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_PRESENCE);
    assert_int_equal(yin_match_keyword(YCTX, "range", strlen("range"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_RANGE);
    assert_int_equal(yin_match_keyword(YCTX, "reference", strlen("reference"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_REFERENCE);
    assert_int_equal(yin_match_keyword(YCTX, "refine", strlen("refine"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_REFINE);
    assert_int_equal(yin_match_keyword(YCTX, "require-instance", strlen("require-instance"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_REQUIRE_INSTANCE);
    assert_int_equal(yin_match_keyword(YCTX, "revision", strlen("revision"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_REVISION);
    assert_int_equal(yin_match_keyword(YCTX, "revision-date", strlen("revision-date"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_REVISION_DATE);
    assert_int_equal(yin_match_keyword(YCTX, "rpc", strlen("rpc"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_RPC);
    assert_int_equal(yin_match_keyword(YCTX, "status", strlen("status"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_STATUS);
    assert_int_equal(yin_match_keyword(YCTX, "submodule", strlen("submodule"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_SUBMODULE);
    assert_int_equal(yin_match_keyword(YCTX, "type", strlen("type"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_TYPE);
    assert_int_equal(yin_match_keyword(YCTX, "typedef", strlen("typedef"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_TYPEDEF);
    assert_int_equal(yin_match_keyword(YCTX, "unique", strlen("unique"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_UNIQUE);
    assert_int_equal(yin_match_keyword(YCTX, "units", strlen("units"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_UNITS);
    assert_int_equal(yin_match_keyword(YCTX, "uses", strlen("uses"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_USES);
    assert_int_equal(yin_match_keyword(YCTX, "value", strlen("value"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_VALUE);
    assert_int_equal(yin_match_keyword(YCTX, "when", strlen("when"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_WHEN);
    assert_int_equal(yin_match_keyword(YCTX, "yang-version", strlen("yang-version"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_YANG_VERSION);
    assert_int_equal(yin_match_keyword(YCTX, "yin-element", strlen("yin-element"), prefix, prefix_len, LY_STMT_NONE), LY_STMT_YIN_ELEMENT);
}

static void
test_yin_match_argument_name(void **UNUSED(state))
{
    assert_int_equal(yin_match_argument_name("", 5), YIN_ARG_UNKNOWN);
    assert_int_equal(yin_match_argument_name("qwertyasd", 5), YIN_ARG_UNKNOWN);
    assert_int_equal(yin_match_argument_name("conditionasd", 8), YIN_ARG_UNKNOWN);
    assert_int_equal(yin_match_argument_name("condition", 9), YIN_ARG_CONDITION);
    assert_int_equal(yin_match_argument_name("date", 4), YIN_ARG_DATE);
    assert_int_equal(yin_match_argument_name("module", 6), YIN_ARG_MODULE);
    assert_int_equal(yin_match_argument_name("name", 4), YIN_ARG_NAME);
    assert_int_equal(yin_match_argument_name("tag", 3), YIN_ARG_TAG);
    assert_int_equal(yin_match_argument_name("target-node", 11), YIN_ARG_TARGET_NODE);
    assert_int_equal(yin_match_argument_name("text", 4), YIN_ARG_TEXT);
    assert_int_equal(yin_match_argument_name("uri", 3), YIN_ARG_URI);
    assert_int_equal(yin_match_argument_name("value", 5), YIN_ARG_VALUE);
}

static void
test_yin_parse_element_generic(void **state)
{
    struct lysp_ext_instance exts;
    LY_ERR ret;
    const char *arg;
    const char *stmt;
    const char *data;

    memset(&exts, 0, sizeof(exts));

    data = "<myext:elem attr=\"value\" xmlns:myext=\"urn:example:extensions\">text_value</myext:elem>";
    ly_in_new_memory(data, &UTEST_IN);
    lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx);

    ret = yin_parse_element_generic(YCTX, LY_STMT_EXTENSION_INSTANCE, &exts.child);
    assert_int_equal(ret, LY_SUCCESS);
    assert_int_equal(YCTX->xmlctx->status, LYXML_ELEM_CLOSE);
    stmt = "myext:elem";
    arg = "text_value";
    CHECK_LYSP_STMT(exts.child, arg, 1, 0, LY_STMT_EXTENSION_INSTANCE, 0, stmt);
    stmt = "attr";
    arg = "value";
    CHECK_LYSP_STMT(exts.child->child, arg, 0, 0x400, 0, 0, stmt);
    lysp_ext_instance_free(UTEST_LYCTX, &exts);
    RESET_STATE;

    data = "<myext:elem xmlns:myext=\"urn:example:extensions\"></myext:elem>";
    ly_in_new_memory(data, &UTEST_IN);
    lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx);

    ret = yin_parse_element_generic(YCTX, LY_STMT_EXTENSION_INSTANCE, &exts.child);
    assert_int_equal(ret, LY_SUCCESS);
    assert_int_equal(YCTX->xmlctx->status, LYXML_ELEM_CLOSE);
    stmt = "myext:elem";
    CHECK_LYSP_STMT(exts.child, NULL, 0, 0x0, LY_STMT_EXTENSION_INSTANCE, 0, stmt);
    lysp_ext_instance_free(UTEST_LYCTX, &exts);
}

static void
test_yin_parse_extension_instance(void **state)
{
    LY_ERR ret;
    struct lysp_ext_instance *exts = NULL;
    struct lysp_stmt *act_child;
    const char *data = "<myext:ext value1=\"test\" value=\"test2\" xmlns:myext=\"urn:example:extensions\"><myext:subelem>text</myext:subelem></myext:ext>";
    const char *stmt = "value1";
    const char *arg = "test";

    ly_in_new_memory(data, &UTEST_IN);
    lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx);

    ret = yin_parse_extension_instance(YCTX, LY_STMT_CONTACT, 0, &exts);
    assert_int_equal(ret, LY_SUCCESS);
    CHECK_LYSP_EXT_INSTANCE(exts, NULL, 1, LY_STMT_CONTACT, 0, "myext:ext", LY_VALUE_XML);

    CHECK_LYSP_STMT(exts->child, arg, 0, LYS_YIN_ATTR, 0, 1, stmt);
    stmt = "value";
    arg = "test2";
    CHECK_LYSP_STMT(exts->child->next, arg, 0, LYS_YIN_ATTR, 0, 1, stmt);
    stmt = "myext:subelem";
    arg = "text";
    CHECK_LYSP_STMT(exts->child->next->next, arg, 0, 0, LY_STMT_EXTENSION_INSTANCE, 0, stmt);
    lysp_ext_instance_free(UTEST_LYCTX, exts);
    LY_ARRAY_FREE(exts);
    exts = NULL;
    RESET_STATE;

    data = "<myext:extension-elem xmlns:myext=\"urn:example:extensions\" />";
    ly_in_new_memory(data, &UTEST_IN);
    lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx);

    ret = yin_parse_extension_instance(YCTX, LY_STMT_CONTACT, 0, &exts);
    assert_int_equal(ret, LY_SUCCESS);
    CHECK_LYSP_EXT_INSTANCE(exts, NULL, 0, LY_STMT_CONTACT, 0, "myext:extension-elem", LY_VALUE_XML);
    lysp_ext_instance_free(UTEST_LYCTX, exts);
    LY_ARRAY_FREE(exts);
    exts = NULL;
    RESET_STATE;

    data =
            "<myext:ext attr1=\"text1\" attr2=\"text2\" xmlns:myext=\"urn:example:extensions\">\n"
            "     <myext:ext-sub1/>\n"
            "     <myext:ext-sub2 sattr1=\"stext2\">\n"
            "         <myext:ext-sub21>\n"
            "             <myext:ext-sub211 sattr21=\"text21\"/>\n"
            "         </myext:ext-sub21>\n"
            "     </myext:ext-sub2>\n"
            "     <myext:ext-sub3 attr3=\"text3\"></myext:ext-sub3>\n"
            "</myext:ext>";
    ly_in_new_memory(data, &UTEST_IN);
    lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx);

    ret = yin_parse_extension_instance(YCTX, LY_STMT_CONTACT, 0, &exts);
    assert_int_equal(ret, LY_SUCCESS);

    CHECK_LYSP_EXT_INSTANCE(exts, NULL, 1, LY_STMT_CONTACT, 0, "myext:ext", LY_VALUE_XML);

    stmt = "attr1";
    arg = "text1";
    act_child = exts->child;
    CHECK_LYSP_STMT(act_child, arg, NULL, LYS_YIN_ATTR, 0x0, 1, stmt);
    stmt = "attr2";
    arg = "text2";
    act_child = act_child->next;
    CHECK_LYSP_STMT(act_child, arg, NULL, LYS_YIN_ATTR, 0x0, 1, stmt);
    stmt = "myext:ext-sub1";
    arg = NULL;
    act_child = act_child->next;
    CHECK_LYSP_STMT(act_child, arg, NULL, 0, LY_STMT_EXTENSION_INSTANCE, 1, stmt);
    stmt = "myext:ext-sub2";
    arg = NULL;
    act_child = act_child->next;
    CHECK_LYSP_STMT(act_child, arg, 1, 0, LY_STMT_EXTENSION_INSTANCE, 1, stmt);

    stmt = "sattr1";
    arg = "stext2";
    act_child = act_child->child;
    CHECK_LYSP_STMT(act_child, arg, NULL, LYS_YIN_ATTR, 0, 1, stmt);
    stmt = "myext:ext-sub21";
    arg = NULL;
    act_child = act_child->next;
    CHECK_LYSP_STMT(act_child, arg, 1, 0, LY_STMT_EXTENSION_INSTANCE, 0, stmt);

    stmt = "myext:ext-sub211";
    arg = NULL;
    act_child = act_child->child;
    CHECK_LYSP_STMT(act_child, arg, 1, 0, LY_STMT_EXTENSION_INSTANCE, 0, stmt);

    stmt = "sattr21";
    arg = "text21";
    act_child = act_child->child;
    CHECK_LYSP_STMT(act_child, arg, 0, LYS_YIN_ATTR, 0, 0, stmt);

    stmt = "myext:ext-sub3";
    arg = NULL;
    act_child = exts->child->next->next->next->next;
    CHECK_LYSP_STMT(act_child, arg, 1, 0, LY_STMT_EXTENSION_INSTANCE, 0, stmt);
    stmt = "attr3";
    arg = "text3";
    act_child = act_child->child;
    CHECK_LYSP_STMT(act_child, arg, 0, LYS_YIN_ATTR, 0, 0, stmt);

    lysp_ext_instance_free(UTEST_LYCTX, exts);
    LY_ARRAY_FREE(exts);
    exts = NULL;
    RESET_STATE;

    data =
            "<myext:extension-elem xmlns:myext=\"urn:example:extensions\" xmlns:yin=\"urn:ietf:params:xml:ns:yang:yin:1\">\n"
            "     <yin:action name=\"act-name\" pre:prefixed=\"ignored\"/>\n"
            "     <yin:augment target-node=\"target\"/>\n"
            "     <yin:status value=\"value\"/>\n"
            "     <yin:include module=\"mod\"/>\n"
            "     <yin:input />\n"
            "     <yin:must condition=\"cond\"/>\n"
            "     <yin:namespace uri=\"uri\"/>\n"
            "     <yin:revision date=\"data\"/>\n"
            "     <yin:unique tag=\"tag\"/>\n"
            "     <yin:description><yin:text>contact-val</yin:text></yin:description>\n"
            "     <yin:error-message><yin:value>err-msg</yin:value></yin:error-message>\n"
            "</myext:extension-elem>";
    ly_in_new_memory(data, &UTEST_IN);
    lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx);

    ret = yin_parse_extension_instance(YCTX, LY_STMT_CONTACT, 0, &exts);
    assert_int_equal(ret, LY_SUCCESS);
    assert_string_equal(exts->child->arg, "act-name");
    assert_string_equal(exts->child->next->arg, "target");
    assert_string_equal(exts->child->next->next->arg, "value");
    assert_string_equal(exts->child->next->next->next->arg, "mod");
    assert_null(exts->child->next->next->next->next->arg);
    assert_string_equal(exts->child->next->next->next->next->next->arg, "cond");
    assert_string_equal(exts->child->next->next->next->next->next->next->arg, "uri");
    assert_string_equal(exts->child->next->next->next->next->next->next->next->arg, "data");
    assert_string_equal(exts->child->next->next->next->next->next->next->next->next->arg, "tag");
    assert_string_equal(exts->child->next->next->next->next->next->next->next->next->next->arg, "contact-val");
    lysp_ext_instance_free(UTEST_LYCTX, exts);
    LY_ARRAY_FREE(exts);
    exts = NULL;
    RESET_STATE;
}

static void
test_yin_parse_content(void **state)
{
    LY_ERR ret = LY_SUCCESS;
    const char *data =
            "<prefix value=\"a_mod\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">\n"
            "    <myext:custom xmlns:myext=\"urn:example:extensions\">totally amazing extension</myext:custom>\n"
            "    <extension name=\"ext\">\n"
            "        <argument name=\"argname\"></argument>\n"
            "        <description><text>desc</text></description>\n"
            "        <reference><text>ref</text></reference>\n"
            "        <status value=\"deprecated\"></status>\n"
            "    </extension>\n"
            "    <text xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">wsefsdf</text>\n"
            "    <if-feature name=\"foo\"></if-feature>\n"
            "    <when condition=\"condition...\">\n"
            "        <reference><text>when_ref</text></reference>\n"
            "        <description><text>when_desc</text></description>\n"
            "    </when>\n"
            "    <config value=\"true\"/>\n"
            "    <error-message>\n"
            "        <value>error-msg</value>\n"
            "    </error-message>\n"
            "    <error-app-tag value=\"err-app-tag\"/>\n"
            "    <units name=\"radians\"></units>\n"
            "    <default value=\"default-value\"/>\n"
            "    <position value=\"25\"></position>\n"
            "    <value value=\"-5\"/>\n"
            "    <require-instance value=\"true\"></require-instance>\n"
            "    <range value=\"5..10\" />\n"
            "    <length value=\"baf\"/>\n"
            "    <pattern value='pattern'>\n"
            "        <modifier value='invert-match'/>\n"
            "    </pattern>\n"
            "    <enum name=\"yay\">\n"
            "    </enum>\n"
            "</prefix>";
    struct lysp_ext_instance *exts = NULL;
    const char **if_features = NULL;
    const char *value, *error_message, *app_tag, *units;
    struct lysp_qname def = {0};
    struct lysp_ext *ext_def = NULL;
    struct lysp_when *when_p = NULL;
    struct lysp_type_enum pos_enum = {}, val_enum = {};
    struct lysp_type req_type = {}, range_type = {}, len_type = {}, patter_type = {}, enum_type = {};
    uint16_t config = 0;

    ly_in_new_memory(data, &UTEST_IN);
    lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx);
    lyxml_ctx_next(YCTX->xmlctx);
    lyxml_ctx_next(YCTX->xmlctx);
    lyxml_ctx_next(YCTX->xmlctx);

    struct yin_subelement subelems[17] = {
        {LY_STMT_CONFIG, &config, 0},
        {LY_STMT_DEFAULT, &def, YIN_SUBELEM_UNIQUE},
        {LY_STMT_ENUM, &enum_type, 0},
        {LY_STMT_ERROR_APP_TAG, &app_tag, YIN_SUBELEM_UNIQUE},
        {LY_STMT_ERROR_MESSAGE, &error_message, 0},
        {LY_STMT_EXTENSION, &ext_def, 0},
        {LY_STMT_IF_FEATURE, &if_features, 0},
        {LY_STMT_LENGTH, &len_type, 0},
        {LY_STMT_PATTERN, &patter_type, 0},
        {LY_STMT_POSITION, &pos_enum, 0},
        {LY_STMT_RANGE, &range_type, 0},
        {LY_STMT_REQUIRE_INSTANCE, &req_type, 0},
        {LY_STMT_UNITS, &units, YIN_SUBELEM_UNIQUE},
        {LY_STMT_VALUE, &val_enum, 0},
        {LY_STMT_WHEN, &when_p, 0},
        {LY_STMT_EXTENSION_INSTANCE, NULL, 0},
        {LY_STMT_ARG_TEXT, &value, 0}
    };

    ret = yin_parse_content(YCTX, subelems, 17, LY_STMT_PREFIX, NULL, &exts);
    assert_int_equal(ret, LY_SUCCESS);
    /* check parsed values */
    assert_string_equal(def.str, "default-value");
    const char *exts_name = "myext:custom";
    const char *exts_arg = "totally amazing extension";

    CHECK_LYSP_EXT_INSTANCE(exts, exts_arg, 0, LY_STMT_PREFIX, 0, exts_name, LY_VALUE_XML);
    assert_string_equal(value, "wsefsdf");
    assert_string_equal(units, "radians");
    assert_string_equal(when_p->cond, "condition...");
    assert_string_equal(when_p->dsc, "when_desc");
    assert_string_equal(when_p->ref, "when_ref");
    assert_int_equal(config, LYS_CONFIG_W);
    CHECK_LYSP_TYPE_ENUM(&pos_enum, NULL, 0, LYS_SET_VALUE, 0, NULL, NULL, 25);
    CHECK_LYSP_TYPE_ENUM(&val_enum, NULL, 0, LYS_SET_VALUE, 0, NULL, NULL, -5);
    assert_int_equal(req_type.require_instance, 1);
    assert_true(req_type.flags &= LYS_SET_REQINST);
    assert_string_equal(range_type.range->arg.str, "5..10");
    assert_true(range_type.flags & LYS_SET_RANGE);
    assert_string_equal(error_message, "error-msg");
    assert_string_equal(app_tag, "err-app-tag");
    assert_string_equal(enum_type.enums->name, "yay");
    CHECK_LYSP_RESTR(len_type.length, "baf", NULL,
            NULL, NULL, 0, NULL);
    assert_true(len_type.flags & LYS_SET_LENGTH);
    assert_string_equal(patter_type.patterns->arg.str, "\x015pattern");
    assert_true(patter_type.flags & LYS_SET_PATTERN);
    /* cleanup */
    lysp_ext_instance_free(UTEST_LYCTX, exts);
    lysp_when_free(UTEST_LYCTX, when_p);
    lysp_ext_free(UTEST_LYCTX, ext_def);
    lydict_remove(UTEST_LYCTX, *if_features);
    lydict_remove(UTEST_LYCTX, error_message);
    lydict_remove(UTEST_LYCTX, app_tag);
    lydict_remove(UTEST_LYCTX, units);
    lydict_remove(UTEST_LYCTX, patter_type.patterns->arg.str);
    lydict_remove(UTEST_LYCTX, def.str);
    lydict_remove(UTEST_LYCTX, range_type.range->arg.str);
    lydict_remove(UTEST_LYCTX, len_type.length->arg.str);
    lydict_remove(UTEST_LYCTX, enum_type.enums->name);
    lydict_remove(UTEST_LYCTX, value);
    LY_ARRAY_FREE(if_features);
    LY_ARRAY_FREE(exts);
    LY_ARRAY_FREE(ext_def);
    LY_ARRAY_FREE(patter_type.patterns);
    LY_ARRAY_FREE(enum_type.enums);
    free(when_p);
    free(range_type.range);
    free(len_type.length);
    RESET_STATE;

    /* test unique subelem */
    const char *prefix_value;
    struct yin_subelement subelems2[2] = {{LY_STMT_PREFIX, &prefix_value, YIN_SUBELEM_UNIQUE},
        {LY_STMT_ARG_TEXT, &value, YIN_SUBELEM_UNIQUE}};

    data = ELEMENT_WRAPPER_START
            "<prefix value=\"inv_mod\" />"
            "<text xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">wsefsdf</text>"
            "<text xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">wsefsdf</text>"
            ELEMENT_WRAPPER_END;
    ly_in_new_memory(data, &UTEST_IN);
    lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx);
    lyxml_ctx_next(YCTX->xmlctx);

    ret = yin_parse_content(YCTX, subelems2, 2, LY_STMT_STATUS, NULL, &exts);
    assert_int_equal(ret, LY_EVALID);
    CHECK_LOG_CTX("Redefinition of \"text\" sub-element in \"status\" element.", "Line number 1.");
    lydict_remove(UTEST_LYCTX, prefix_value);
    lydict_remove(UTEST_LYCTX, value);
    RESET_STATE;

    /* test first subelem */
    data = ELEMENT_WRAPPER_START
            "<prefix value=\"inv_mod\" />"
            "<text xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">wsefsdf</text>"
            "<text xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">wsefsdf</text>"
            ELEMENT_WRAPPER_END;
    struct yin_subelement subelems3[2] = {{LY_STMT_PREFIX, &prefix_value, YIN_SUBELEM_UNIQUE},
        {LY_STMT_ARG_TEXT, &value, YIN_SUBELEM_FIRST}};

    ly_in_new_memory(data, &UTEST_IN);
    lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx);
    lyxml_ctx_next(YCTX->xmlctx);

    ret = yin_parse_content(YCTX, subelems3, 2, LY_STMT_STATUS, NULL, &exts);
    assert_int_equal(ret, LY_EVALID);
    CHECK_LOG_CTX("Sub-element \"text\" of \"status\" element must be defined as it's first sub-element.", "Line number 1.");
    lydict_remove(UTEST_LYCTX, prefix_value);
    RESET_STATE;

    /* test mandatory subelem */
    data = ELEMENT_WRAPPER_START ELEMENT_WRAPPER_END;
    struct yin_subelement subelems4[1] = {{LY_STMT_PREFIX, &prefix_value, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE}};

    ly_in_new_memory(data, &UTEST_IN);
    lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx);
    lyxml_ctx_next(YCTX->xmlctx);

    ret = yin_parse_content(YCTX, subelems4, 1, LY_STMT_STATUS, NULL, &exts);
    assert_int_equal(ret, LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory sub-element \"prefix\" of \"status\" element.", "Line number 1.");
}

static void
test_validate_value(void **state)
{
    const char *data = ELEMENT_WRAPPER_START ELEMENT_WRAPPER_END;

    /* create some XML context */
    ly_in_new_memory(data, &UTEST_IN);
    lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx);
    YCTX->xmlctx->status = LYXML_ELEM_CONTENT;
    YCTX->xmlctx->dynamic = 0;

    YCTX->xmlctx->value = "#invalid";
    YCTX->xmlctx->value_len = 8;
    assert_int_equal(yin_validate_value(YCTX, Y_IDENTIF_ARG), LY_EVALID);
    CHECK_LOG_CTX("Invalid identifier first character '#' (0x0023).", "Line number 1.");

    YCTX->xmlctx->value = "";
    YCTX->xmlctx->value_len = 0;
    assert_int_equal(yin_validate_value(YCTX, Y_STR_ARG), LY_SUCCESS);

    YCTX->xmlctx->value = "pre:b";
    YCTX->xmlctx->value_len = 5;
    assert_int_equal(yin_validate_value(YCTX, Y_IDENTIF_ARG), LY_EVALID);
    assert_int_equal(yin_validate_value(YCTX, Y_PREF_IDENTIF_ARG), LY_SUCCESS);

    YCTX->xmlctx->value = "pre:pre:b";
    YCTX->xmlctx->value_len = 9;
    assert_int_equal(yin_validate_value(YCTX, Y_PREF_IDENTIF_ARG), LY_EVALID);
}

/* helper function to simplify unit test of each element using parse_content function */
LY_ERR
test_element_helper(void **state, const char *data, void *dest, const char **text, struct lysp_ext_instance **exts)
{
    const char *name, *prefix;
    size_t name_len, prefix_len;
    LY_ERR ret = LY_SUCCESS;
    struct yin_subelement subelems[71] = {
        {LY_STMT_ACTION, dest, 0},
        {LY_STMT_ANYDATA, dest, 0},
        {LY_STMT_ANYXML, dest, 0},
        {LY_STMT_ARGUMENT, dest, 0},
        {LY_STMT_AUGMENT, dest, 0},
        {LY_STMT_BASE, dest, 0},
        {LY_STMT_BELONGS_TO, dest, 0},
        {LY_STMT_BIT, dest, 0},
        {LY_STMT_CASE, dest, 0},
        {LY_STMT_CHOICE, dest, 0},
        {LY_STMT_CONFIG, dest, 0},
        {LY_STMT_CONTACT, dest, 0},
        {LY_STMT_CONTAINER, dest, 0},
        {LY_STMT_DEFAULT, dest, YIN_SUBELEM_UNIQUE},
        {LY_STMT_DESCRIPTION, dest, 0},
        {LY_STMT_DEVIATE, dest, 0},
        {LY_STMT_DEVIATION, dest, 0},
        {LY_STMT_ENUM, dest, 0},
        {LY_STMT_ERROR_APP_TAG, dest, YIN_SUBELEM_UNIQUE},
        {LY_STMT_ERROR_MESSAGE, dest, 0},
        {LY_STMT_EXTENSION, dest, 0},
        {LY_STMT_FEATURE, dest, 0},
        {LY_STMT_FRACTION_DIGITS, dest, 0},
        {LY_STMT_GROUPING, dest, 0},
        {LY_STMT_IDENTITY, dest, 0},
        {LY_STMT_IF_FEATURE, dest, 0},
        {LY_STMT_IMPORT, dest, 0},
        {LY_STMT_INCLUDE, dest, 0},
        {LY_STMT_INPUT, dest, 0},
        {LY_STMT_KEY, dest, YIN_SUBELEM_UNIQUE},
        {LY_STMT_LEAF, dest, 0},
        {LY_STMT_LEAF_LIST, dest, 0},
        {LY_STMT_LENGTH, dest, 0},
        {LY_STMT_LIST, dest, 0},
        {LY_STMT_MANDATORY, dest, 0},
        {LY_STMT_MAX_ELEMENTS, dest, 0},
        {LY_STMT_MIN_ELEMENTS, dest, 0},
        {LY_STMT_MODIFIER, dest, 0},
        {LY_STMT_MODULE, dest, 0},
        {LY_STMT_MUST, dest, 0},
        {LY_STMT_NAMESPACE, dest, YIN_SUBELEM_UNIQUE},
        {LY_STMT_NOTIFICATION, dest, 0},
        {LY_STMT_ORDERED_BY, dest, 0},
        {LY_STMT_ORGANIZATION, dest, 0},
        {LY_STMT_OUTPUT, dest, 0},
        {LY_STMT_PATH, dest, 0},
        {LY_STMT_PATTERN, dest, 0},
        {LY_STMT_POSITION, dest, 0},
        {LY_STMT_PREFIX, dest, YIN_SUBELEM_UNIQUE},
        {LY_STMT_PRESENCE, dest, YIN_SUBELEM_UNIQUE},
        {LY_STMT_RANGE, dest, 0},
        {LY_STMT_REFERENCE, dest, 0},
        {LY_STMT_REFINE, dest, 0},
        {LY_STMT_REQUIRE_INSTANCE, dest, 0},
        {LY_STMT_REVISION, dest, 0},
        {LY_STMT_REVISION_DATE, dest, 0},
        {LY_STMT_RPC, dest, 0},
        {LY_STMT_STATUS, dest, 0},
        {LY_STMT_SUBMODULE, dest, 0},
        {LY_STMT_TYPE, dest, 0},
        {LY_STMT_TYPEDEF, dest, 0},
        {LY_STMT_UNIQUE, dest, 0},
        {LY_STMT_UNITS, dest, YIN_SUBELEM_UNIQUE},
        {LY_STMT_USES, dest, 0},
        {LY_STMT_VALUE, dest, 0},
        {LY_STMT_WHEN, dest, 0},
        {LY_STMT_YANG_VERSION, dest, 0},
        {LY_STMT_YIN_ELEMENT, dest, 0},
        {LY_STMT_EXTENSION_INSTANCE, dest, 0},
        {LY_STMT_ARG_TEXT, dest, 0},
        {LY_STMT_ARG_VALUE, dest, 0}
    };

    ly_in_new_memory(data, &UTEST_IN);
    lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx);
    prefix = YCTX->xmlctx->prefix;
    prefix_len = YCTX->xmlctx->prefix_len;
    name = YCTX->xmlctx->name;
    name_len = YCTX->xmlctx->name_len;
    lyxml_ctx_next(YCTX->xmlctx);

    ret = yin_parse_content(YCTX, subelems, 71, yin_match_keyword(YCTX, name, name_len, prefix, prefix_len, LY_STMT_NONE), text, exts);

    /* free parser and input */
    lyxml_ctx_free(YCTX->xmlctx);
    YCTX->xmlctx = NULL;
    ly_in_free(UTEST_IN, 0);
    UTEST_IN = NULL;
    return ret;
}

#define EXT_SUBELEM "<myext:c-define name=\"MY_MTU\" xmlns:myext=\"urn:example:extensions\"/>"

static void
test_enum_elem(void **state)
{
    struct lysp_type type = {};
    const char *data;

    data = ELEMENT_WRAPPER_START
            "<enum name=\"enum-name\">\n"
            "     <if-feature name=\"feature\" />\n"
            "     <value value=\"55\" />\n"
            "     <status value=\"deprecated\" />\n"
            "     <description><text>desc...</text></description>\n"
            "     <reference><text>ref...</text></reference>\n"
            "     " EXT_SUBELEM "\n"
            "</enum>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    uint16_t flags = LYS_STATUS_DEPRC | LYS_SET_VALUE;

    CHECK_LYSP_TYPE_ENUM(type.enums, "desc...", 1, flags, 1, "enum-name", "ref...", 55);
    assert_string_equal(type.enums->iffeatures[0].str, "feature");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(type.enums->exts, LY_STMT_ENUM);
    lysp_type_free(UTEST_LYCTX, &type);
    memset(&type, 0, sizeof type);

    data = ELEMENT_WRAPPER_START
            "<enum name=\"enum-name\"></enum>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    assert_string_equal(type.enums->name, "enum-name");
    lysp_type_free(UTEST_LYCTX, &type);
    memset(&type, 0, sizeof type);
}

static void
test_bit_elem(void **state)
{
    struct lysp_type type = {};
    const char *data;

    data = ELEMENT_WRAPPER_START
            "<bit name=\"bit-name\">\n"
            "    <if-feature name=\"feature\" />\n"
            "    <position value=\"55\" />\n"
            "    <status value=\"deprecated\" />\n"
            "    <description><text>desc...</text></description>\n"
            "    <reference><text>ref...</text></reference>\n"
            EXT_SUBELEM
            "</bit>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    uint16_t flags = LYS_STATUS_DEPRC | LYS_SET_VALUE;

    CHECK_LYSP_TYPE_ENUM(type.bits, "desc...", 1, flags, 1, "bit-name", "ref...", 55);
    assert_string_equal(type.bits->iffeatures[0].str, "feature");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(type.bits->exts, LY_STMT_BIT);
    lysp_type_free(UTEST_LYCTX, &type);
    memset(&type, 0, sizeof type);

    data = ELEMENT_WRAPPER_START
            "<bit name=\"bit-name\"> </bit>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_TYPE_ENUM(type.bits, NULL, 0, 0, 0, "bit-name", NULL, 0);
    lysp_type_free(UTEST_LYCTX, &type);
    memset(&type, 0, sizeof type);
}

static void
test_meta_elem(void **state)
{
    char *value = NULL;
    const char *data;
    struct lysp_ext_instance *exts = NULL;

    /* organization element */
    data = ELEMENT_WRAPPER_START
            "<organization><text>organization...</text>" EXT_SUBELEM EXT_SUBELEM "</organization>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &value, NULL, &exts), LY_SUCCESS);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_ORGANIZATION);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[1]), LY_STMT_ORGANIZATION);

    assert_string_equal(value, "organization...");
    lydict_remove(UTEST_LYCTX, value);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    value = NULL;
    exts = NULL;

    /* contact element */
    data = ELEMENT_WRAPPER_START
            "<contact><text>contact...</text>" EXT_SUBELEM "</contact>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &value, NULL, &exts), LY_SUCCESS);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_CONTACT);
    assert_string_equal(value, "contact...");
    lydict_remove(UTEST_LYCTX, value);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;
    value = NULL;

    /* description element */
    data = ELEMENT_WRAPPER_START
            "<description><text>description...</text>" EXT_SUBELEM "</description>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &value, NULL, &exts), LY_SUCCESS);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_DESCRIPTION);
    assert_string_equal(value, "description...");
    lydict_remove(UTEST_LYCTX, value);
    value = NULL;
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;

    /* reference element */
    data = ELEMENT_WRAPPER_START
            "<reference><text>reference...</text>" EXT_SUBELEM "</reference>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &value, NULL, &exts), LY_SUCCESS);
    assert_string_equal(value, "reference...");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_REFERENCE);
    lydict_remove(UTEST_LYCTX, value);
    value = NULL;
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;

    /* reference element */
    data = ELEMENT_WRAPPER_START
            "<reference invalid=\"text\"><text>reference...</text>" "</reference>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &value, NULL, &exts), LY_EVALID);
    CHECK_LOG_CTX("Unexpected attribute \"invalid\" of \"reference\" element.", "Line number 1.");
    lydict_remove(UTEST_LYCTX, value);
    value = NULL;
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;

    /* missing text subelement */
    data = ELEMENT_WRAPPER_START
            "<reference>reference...</reference>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &value, NULL, &exts), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory sub-element \"text\" of \"reference\" element.", "Line number 1.");

    /* reference element */
    data = ELEMENT_WRAPPER_START
            "<reference>" EXT_SUBELEM "<text>reference...</text></reference>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &value, NULL, &exts), LY_EVALID);
    CHECK_LOG_CTX("Sub-element \"text\" of \"reference\" element must be defined as it's first sub-element.", "Line number 1.");
    lydict_remove(UTEST_LYCTX, value);
    value = NULL;
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;
}

static void
test_import_elem(void **state)
{
    const char *data;
    struct lysp_import *imports = NULL;
    struct import_meta imp_meta = {"prefix", &imports};

    /* max subelems */
    data = ELEMENT_WRAPPER_START
            "<import module=\"a\">\n"
            EXT_SUBELEM
            "    <prefix value=\"a_mod\"/>\n"
            "    <revision-date date=\"2015-01-01\"></revision-date>\n"
            "    <description><text>import description</text></description>\n"
            "    <reference><text>import reference</text></reference>\n"
            "</import>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &imp_meta, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_IMPORT(imports, "import description", 1, "a",
            "a_mod", "import reference", "2015-01-01");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(imports->exts, LY_STMT_IMPORT);
    FREE_ARRAY(UTEST_LYCTX, imports, lysp_import_free);
    imports = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START
            "<import module=\"a\">\n"
            "    <prefix value=\"a_mod\"/>\n"
            "</import>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &imp_meta, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_IMPORT(imports, NULL, 0, "a",
            "a_mod", NULL, "");
    FREE_ARRAY(UTEST_LYCTX, imports, lysp_import_free);
    imports = NULL;

    /* invalid (missing prefix) */
    data = ELEMENT_WRAPPER_START "<import module=\"a\"></import>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &imp_meta, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory sub-element \"prefix\" of \"import\" element.", "Line number 1.");
    FREE_ARRAY(UTEST_LYCTX, imports, lysp_import_free);
    imports = NULL;

    /* invalid reused prefix */
    data = ELEMENT_WRAPPER_START
            "<import module=\"a\">\n"
            "    <prefix value=\"prefix\"/>\n"
            "</import>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &imp_meta, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Prefix \"prefix\" already used as module prefix.", "Line number 3.");
    FREE_ARRAY(UTEST_LYCTX, imports, lysp_import_free);
    imports = NULL;

    data = ELEMENT_WRAPPER_START
            "<import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "</import>\n"
            "<import module=\"a\">\n"
            "    <prefix value=\"a\"/>\n"
            "</import>\n"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &imp_meta, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Prefix \"a\" already used to import \"a\" module.", "Line number 6.");
    FREE_ARRAY(UTEST_LYCTX, imports, lysp_import_free);
    imports = NULL;
}

static void
test_status_elem(void **state)
{
    const char *data;
    uint16_t flags = 0;
    struct lysp_ext_instance *exts = NULL;

    /* test valid values */
    data = ELEMENT_WRAPPER_START "<status value=\"current\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, NULL), LY_SUCCESS);
    assert_true(flags & LYS_STATUS_CURR);

    data = ELEMENT_WRAPPER_START "<status value=\"deprecated\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, NULL), LY_SUCCESS);
    assert_true(flags & LYS_STATUS_DEPRC);

    data = ELEMENT_WRAPPER_START "<status value=\"obsolete\">"EXT_SUBELEM "</status>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, &exts), LY_SUCCESS);
    assert_true(flags & LYS_STATUS_OBSLT);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_STATUS);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;

    /* test invalid value */
    data = ELEMENT_WRAPPER_START "<status value=\"invalid\"></status>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"invalid\" of \"value\" attribute in \"status\" element. "
            "Valid values are \"current\", \"deprecated\" and \"obsolete\".", "Line number 1.");
}

static void
test_ext_elem(void **state)
{
    const char *data;
    struct lysp_ext *ext = NULL;

    /* max subelems */
    data = ELEMENT_WRAPPER_START
            "<extension name=\"ext_name\">\n"
            "     <argument name=\"arg\"></argument>\n"
            "     <status value=\"current\"/>\n"
            "     <description><text>ext_desc</text></description>\n"
            "     <reference><text>ext_ref</text></reference>\n"
            EXT_SUBELEM
            "</extension>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &ext, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_EXT(ext, "arg", 0, "ext_desc", 1, LYS_STATUS_CURR, "ext_name", "ext_ref");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(ext->exts[0]), LY_STMT_EXTENSION);
    lysp_ext_free(UTEST_LYCTX, ext);
    LY_ARRAY_FREE(ext);
    ext = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<extension name=\"ext_name\"></extension>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &ext, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_EXT(ext, NULL, 0, NULL, 0, 0, "ext_name", NULL);
    lysp_ext_free(UTEST_LYCTX, ext);
    LY_ARRAY_FREE(ext);
    ext = NULL;
}

static void
test_yin_element_elem(void **state)
{
    const char *data;
    uint16_t flags = 0;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<yin-element value=\"true\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, NULL), LY_SUCCESS);
    assert_true(flags & LYS_YINELEM_TRUE);

    data = ELEMENT_WRAPPER_START "<yin-element value=\"false\">" EXT_SUBELEM "</yin-element>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, &exts), LY_SUCCESS);
    assert_true(flags & LYS_YINELEM_TRUE);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_YIN_ELEMENT);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);

    data = ELEMENT_WRAPPER_START "<yin-element value=\"invalid\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, NULL), LY_EVALID);
    assert_true(flags & LYS_YINELEM_TRUE);
    CHECK_LOG_CTX("Invalid value \"invalid\" of \"value\" attribute in \"yin-element\" element. "
            "Valid values are \"true\" and \"false\".", "Line number 1.");
}

static void
test_yangversion_elem(void **state)
{
    const char *data;
    uint8_t version = 0;
    struct lysp_ext_instance *exts = NULL;

    /* valid values */
    data = ELEMENT_WRAPPER_START "<yang-version value=\"1\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &version, NULL, NULL), LY_SUCCESS);
    assert_true(version & LYS_VERSION_1_0);

    data = ELEMENT_WRAPPER_START "<yang-version value=\"1.1\">" EXT_SUBELEM "</yang-version>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &version, NULL, &exts), LY_SUCCESS);
    assert_true(version & LYS_VERSION_1_1);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_YANG_VERSION);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);

    /* invalid value */
    data = ELEMENT_WRAPPER_START "<yang-version value=\"version\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &version, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"version\" of \"value\" attribute in \"yang-version\" element. "
            "Valid values are \"1\" and \"1.1\".", "Line number 1.");
}

static void
test_mandatory_elem(void **state)
{
    const char *data;
    uint16_t man = 0;
    struct lysp_ext_instance *exts = NULL;

    /* valid values */
    data = ELEMENT_WRAPPER_START "<mandatory value=\"true\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &man, NULL, NULL), LY_SUCCESS);
    assert_int_equal(man, LYS_MAND_TRUE);
    man = 0;

    data = ELEMENT_WRAPPER_START "<mandatory value=\"false\">" EXT_SUBELEM "</mandatory>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &man, NULL, &exts), LY_SUCCESS);
    assert_int_equal(man, LYS_MAND_FALSE);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_MANDATORY);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);

    data = ELEMENT_WRAPPER_START "<mandatory value=\"invalid\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &man, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"invalid\" of \"value\" attribute in \"mandatory\" element. "
            "Valid values are \"true\" and \"false\".", "Line number 1.");
}

static void
test_argument_elem(void **state)
{
    const char *data;
    uint16_t flags = 0;
    const char *arg;
    struct yin_argument_meta arg_meta = {&flags, &arg};
    struct lysp_ext_instance *exts = NULL;

    /* max subelems */
    data = ELEMENT_WRAPPER_START
            "<argument name=\"arg-name\">\n"
            "     <yin-element value=\"true\" />\n"
            EXT_SUBELEM
            "</argument>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &arg_meta, NULL, &exts), LY_SUCCESS);
    assert_string_equal(arg, "arg-name");
    assert_true(flags & LYS_YINELEM_TRUE);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_ARGUMENT);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;
    flags = 0;
    lydict_remove(UTEST_LYCTX, arg);
    arg = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START
            "<argument name=\"arg\">"
            "</argument>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &arg_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(arg, "arg");
    assert_true(flags == 0);
    lydict_remove(UTEST_LYCTX, arg);
}

static void
test_base_elem(void **state)
{
    const char *data;
    const char **bases = NULL;
    struct lysp_ext_instance *exts = NULL;
    struct lysp_type type = {};

    /* as identity subelement */
    data = "<identity xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">\n"
            "     <base name=\"base-name\">\n"
            EXT_SUBELEM
            "     </base>\n"
            "</identity>";
    assert_int_equal(test_element_helper(state, data, &bases, NULL, &exts), LY_SUCCESS);
    assert_string_equal(*bases, "base-name");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_BASE);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;
    lydict_remove(UTEST_LYCTX, *bases);
    LY_ARRAY_FREE(bases);

    /* as type subelement */
    data = "<type xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">\n"
            "     <base name=\"base-name\">\n"
            EXT_SUBELEM
            "     </base>\n"
            "</type>";
    assert_int_equal(test_element_helper(state, data, &type, NULL, &exts), LY_SUCCESS);
    assert_string_equal(*type.bases, "base-name");
    assert_true(type.flags & LYS_SET_BASE);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_BASE);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;
    lydict_remove(UTEST_LYCTX, *type.bases);
    LY_ARRAY_FREE(type.bases);
}

static void
test_belongsto_elem(void **state)
{
    const char *data;
    struct lysp_submodule submod;
    struct lysp_ext_instance *exts = NULL;

    lydict_insert(UTEST_LYCTX, "module-name", 0, &YCTX->parsed_mod->mod->name);

    data = ELEMENT_WRAPPER_START
            "<belongs-to module=\"module-name\"><prefix value=\"pref\"/>"EXT_SUBELEM "</belongs-to>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &submod, NULL, &exts), LY_SUCCESS);
    assert_string_equal(submod.prefix, "pref");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_BELONGS_TO);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;
    lydict_remove(UTEST_LYCTX, submod.prefix);

    data = ELEMENT_WRAPPER_START "<belongs-to module=\"module-name\"></belongs-to>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &submod, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory sub-element \"prefix\" of \"belongs-to\" element.", "Line number 1.");
}

static void
test_config_elem(void **state)
{
    const char *data;
    uint16_t flags = 0;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<config value=\"true\">" EXT_SUBELEM "</config>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, &exts), LY_SUCCESS);
    assert_true(flags & LYS_CONFIG_W);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_CONFIG);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;
    flags = 0;

    data = ELEMENT_WRAPPER_START "<config value=\"false\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, NULL), LY_SUCCESS);
    assert_true(flags & LYS_CONFIG_R);
    flags = 0;

    data = ELEMENT_WRAPPER_START "<config value=\"invalid\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"invalid\" of \"value\" attribute in \"config\" element. "
            "Valid values are \"true\" and \"false\".", "Line number 1.");
}

static void
test_default_elem(void **state)
{
    const char *data;
    struct lysp_qname val = {0};
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<default value=\"defaul-value\">"EXT_SUBELEM "</default>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, &exts), LY_SUCCESS);
    assert_string_equal(val.str, "defaul-value");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_DEFAULT);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;
    lydict_remove(UTEST_LYCTX, val.str);
    val.str = NULL;

    data = ELEMENT_WRAPPER_START "<default/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory attribute value of default element.", "Line number 1.");
}

static void
test_err_app_tag_elem(void **state)
{
    const char *data;
    const char *val = NULL;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<error-app-tag value=\"val\">"EXT_SUBELEM "</error-app-tag>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, &exts), LY_SUCCESS);
    assert_string_equal(val, "val");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_ERROR_APP_TAG);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;
    lydict_remove(UTEST_LYCTX, val);
    val = NULL;

    data = ELEMENT_WRAPPER_START "<error-app-tag/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory attribute value of error-app-tag element.", "Line number 1.");
}

static void
test_err_msg_elem(void **state)
{
    const char *data;
    const char *val = NULL;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<error-message><value>val</value>"EXT_SUBELEM "</error-message>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, &exts), LY_SUCCESS);
    assert_string_equal(val, "val");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_ERROR_MESSAGE);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;
    lydict_remove(UTEST_LYCTX, val);

    data = ELEMENT_WRAPPER_START "<error-message></error-message>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory sub-element \"value\" of \"error-message\" element.", "Line number 1.");

    data = ELEMENT_WRAPPER_START "<error-message invalid=\"text\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Unexpected attribute \"invalid\" of \"error-message\" element.", "Line number 1.");
}

static void
test_fracdigits_elem(void **state)
{
    const char *data;
    struct lysp_type type = {};

    /* valid value */
    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"10\">"EXT_SUBELEM "</fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(type.exts[0]), LY_STMT_FRACTION_DIGITS);
    assert_int_equal(type.fraction_digits, 10);
    assert_true(type.flags & LYS_SET_FRDIGITS);
    FREE_ARRAY(UTEST_LYCTX, type.exts, lysp_ext_instance_free);

    /* invalid values */
    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"-1\"></fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"-1\" of \"value\" attribute in \"fraction-digits\" element.", "Line number 1.");

    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"02\"></fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"02\" of \"value\" attribute in \"fraction-digits\" element.", "Line number 1.");

    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"1p\"></fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"1p\" of \"value\" attribute in \"fraction-digits\" element.", "Line number 1.");

    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"19\"></fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"19\" of \"value\" attribute in \"fraction-digits\" element.", "Line number 1.");

    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"999999999999999999\"></fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"999999999999999999\" of \"value\" attribute in \"fraction-digits\" element.", "Line number 1.");
}

static void
test_iffeature_elem(void **state)
{
    const char *data;
    const char **iffeatures = NULL;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<if-feature name=\"local-storage\">"EXT_SUBELEM "</if-feature>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &iffeatures, NULL, &exts), LY_SUCCESS);
    assert_string_equal(*iffeatures, "local-storage");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_IF_FEATURE);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;
    lydict_remove(UTEST_LYCTX, *iffeatures);
    LY_ARRAY_FREE(iffeatures);
    iffeatures = NULL;

    data = ELEMENT_WRAPPER_START "<if-feature/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &iffeatures, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory attribute name of if-feature element.", "Line number 1.");
    LY_ARRAY_FREE(iffeatures);
    iffeatures = NULL;
}

static void
test_length_elem(void **state)
{
    const char *data;
    struct lysp_type type = {};

    /* max subelems */
    data = ELEMENT_WRAPPER_START
            "<length value=\"length-str\">\n"
            "    <error-message><value>err-msg</value></error-message>\n"
            "    <error-app-tag value=\"err-app-tag\"/>\n"
            "    <description><text>desc</text></description>\n"
            "    <reference><text>ref</text></reference>\n"
            EXT_SUBELEM
            "</length>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_RESTR(type.length, "length-str", "desc",
            "err-app-tag", "err-msg", 1, "ref");
    assert_true(type.flags & LYS_SET_LENGTH);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(type.length->exts[0]), LY_STMT_LENGTH);
    lysp_type_free(UTEST_LYCTX, &type);
    memset(&type, 0, sizeof(type));

    /* min subelems */
    data = ELEMENT_WRAPPER_START
            "<length value=\"length-str\">"
            "</length>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_RESTR(type.length, "length-str", NULL,
            NULL, NULL, 0, NULL);
    lysp_type_free(UTEST_LYCTX, &type);
    assert_true(type.flags & LYS_SET_LENGTH);
    memset(&type, 0, sizeof(type));

    data = ELEMENT_WRAPPER_START "<length></length>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory attribute value of length element.", "Line number 1.");
    lysp_type_free(UTEST_LYCTX, &type);
    memset(&type, 0, sizeof(type));
}

static void
test_modifier_elem(void **state)
{
    const char *data;
    const char *pat;
    struct lysp_ext_instance *exts = NULL;

    assert_int_equal(LY_SUCCESS, lydict_insert(UTEST_LYCTX, "\006pattern", 8, &pat));
    data = ELEMENT_WRAPPER_START "<modifier value=\"invert-match\">" EXT_SUBELEM "</modifier>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &pat, NULL, &exts), LY_SUCCESS);
    assert_string_equal(pat, "\x015pattern");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_MODIFIER);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;
    lydict_remove(UTEST_LYCTX, pat);

    assert_int_equal(LY_SUCCESS, lydict_insert(UTEST_LYCTX, "\006pattern", 8, &pat));
    data = ELEMENT_WRAPPER_START "<modifier value=\"invert\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &pat, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"invert\" of \"value\" attribute in \"modifier\" element. "
            "Only valid value is \"invert-match\".", "Line number 1.");
    lydict_remove(UTEST_LYCTX, pat);
}

static void
test_namespace_elem(void **state)
{
    const char *data;
    const char *ns;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<namespace uri=\"ns\">" EXT_SUBELEM "</namespace>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &ns, NULL, &exts), LY_SUCCESS);
    assert_string_equal(ns, "ns");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_NAMESPACE);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;
    lydict_remove(UTEST_LYCTX, ns);

    data = ELEMENT_WRAPPER_START "<namespace/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &ns, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory attribute uri of namespace element.", "Line number 1.");
}

static void
test_pattern_elem(void **state)
{
    const char *data;
    struct lysp_type type = {};

    /* max subelems */
    data = ELEMENT_WRAPPER_START
            "<pattern value=\"super_pattern\">\n"
            "    <modifier value=\"invert-match\"/>\n"
            "    <error-message><value>err-msg-value</value></error-message>\n"
            "    <error-app-tag value=\"err-app-tag-value\"/>\n"
            "    <description><text>&quot;pattern-desc&quot;</text></description>\n"
            "    <reference><text>pattern-ref</text></reference>\n"
            EXT_SUBELEM
            "</pattern>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    assert_true(type.flags & LYS_SET_PATTERN);
    CHECK_LYSP_RESTR(type.patterns, "\x015super_pattern", "\"pattern-desc\"",
            "err-app-tag-value", "err-msg-value", 1, "pattern-ref");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(type.patterns->exts[0]), LY_STMT_PATTERN);
    lysp_type_free(UTEST_LYCTX, &type);
    memset(&type, 0, sizeof(type));

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<pattern value=\"pattern\"> </pattern>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_RESTR(type.patterns, "\x006pattern", NULL, NULL, NULL, 0, NULL);
    lysp_type_free(UTEST_LYCTX, &type);
    memset(&type, 0, sizeof(type));
}

static void
test_value_position_elem(void **state)
{
    const char *data;
    struct lysp_type_enum en = {};

    /* valid values */
    data = ELEMENT_WRAPPER_START "<value value=\"55\">" EXT_SUBELEM "</value>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_TYPE_ENUM(&(en), NULL, 1, LYS_SET_VALUE, 0, NULL, NULL, 55);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(en.exts[0]), LY_STMT_VALUE);
    FREE_ARRAY(UTEST_LYCTX, en.exts, lysp_ext_instance_free);
    memset(&en, 0, sizeof(en));

    data = ELEMENT_WRAPPER_START "<value value=\"-55\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_TYPE_ENUM(&(en), NULL, 0, LYS_SET_VALUE, 0, NULL, NULL, -55);
    memset(&en, 0, sizeof(en));

    data = ELEMENT_WRAPPER_START "<value value=\"0\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_TYPE_ENUM(&(en), NULL, 0, LYS_SET_VALUE, 0, NULL, NULL, 0);
    memset(&en, 0, sizeof(en));

    data = ELEMENT_WRAPPER_START "<value value=\"-0\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_TYPE_ENUM(&(en), NULL, 0, LYS_SET_VALUE, 0, NULL, NULL, 0);
    memset(&en, 0, sizeof(en));

    /* valid positions */
    data = ELEMENT_WRAPPER_START "<position value=\"55\">" EXT_SUBELEM "</position>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_TYPE_ENUM(&(en), NULL, 1, LYS_SET_VALUE, 0, NULL, NULL, 55);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(en.exts[0]), LY_STMT_POSITION);
    FREE_ARRAY(UTEST_LYCTX, en.exts, lysp_ext_instance_free);
    memset(&en, 0, sizeof(en));

    data = ELEMENT_WRAPPER_START "<position value=\"0\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_TYPE_ENUM(&(en), NULL, 0, LYS_SET_VALUE, 0, NULL, NULL, 0);
    memset(&en, 0, sizeof(en));

    /* invalid values */
    data = ELEMENT_WRAPPER_START "<value value=\"99999999999999999999999\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"99999999999999999999999\" of \"value\" attribute in \"value\" element.", "Line number 1.");

    data = ELEMENT_WRAPPER_START "<value value=\"1k\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"1k\" of \"value\" attribute in \"value\" element.", "Line number 1.");

    data = ELEMENT_WRAPPER_START "<value value=\"\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"\" of \"value\" attribute in \"value\" element.", "Line number 1.");

    /*invalid positions */
    data = ELEMENT_WRAPPER_START "<position value=\"-5\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"-5\" of \"value\" attribute in \"position\" element.", "Line number 1.");

    data = ELEMENT_WRAPPER_START "<position value=\"-0\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"-0\" of \"value\" attribute in \"position\" element.", "Line number 1.");

    data = ELEMENT_WRAPPER_START "<position value=\"99999999999999999999\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"99999999999999999999\" of \"value\" attribute in \"position\" element.", "Line number 1.");

    data = ELEMENT_WRAPPER_START "<position value=\"\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"\" of \"value\" attribute in \"position\" element.", "Line number 1.");
}

static void
test_prefix_elem(void **state)
{
    const char *data;
    const char *value = NULL;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<prefix value=\"pref\">" EXT_SUBELEM "</prefix>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &value, NULL, &exts), LY_SUCCESS);
    assert_string_equal(value, "pref");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_PREFIX);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    exts = NULL;
    lydict_remove(UTEST_LYCTX, value);

    data = ELEMENT_WRAPPER_START "<prefix value=\"pref\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &value, NULL, NULL), LY_SUCCESS);
    assert_string_equal(value, "pref");
    lydict_remove(UTEST_LYCTX, value);
}

static void
test_range_elem(void **state)
{
    const char *data;
    struct lysp_type type = {};

    /* max subelems */
    data = ELEMENT_WRAPPER_START
            "<range value=\"range-str\">\n"
            "    <error-message><value>err-msg</value></error-message>\n"
            "    <error-app-tag value=\"err-app-tag\" />\n"
            "    <description><text>desc</text></description>\n"
            "    <reference><text>ref</text></reference>\n"
            EXT_SUBELEM
            "</range>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_RESTR(type.range, "range-str", "desc",
            "err-app-tag", "err-msg", 1, "ref");
    assert_true(type.flags & LYS_SET_RANGE);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(type.range->exts[0]), LY_STMT_RANGE);
    lysp_type_free(UTEST_LYCTX, &type);
    memset(&type, 0, sizeof(type));

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<range value=\"range-str\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_RESTR(type.range, "range-str", NULL,
            NULL, NULL, 0, NULL);
    lysp_type_free(UTEST_LYCTX, &type);
    memset(&type, 0, sizeof(type));
}

static void
test_reqinstance_elem(void **state)
{
    const char *data;
    struct lysp_type type = {};

    data = ELEMENT_WRAPPER_START "<require-instance value=\"true\">" EXT_SUBELEM "</require-instance>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    assert_int_equal(type.require_instance, 1);
    assert_true(type.flags & LYS_SET_REQINST);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(type.exts[0]), LY_STMT_REQUIRE_INSTANCE);
    lysp_type_free(UTEST_LYCTX, &type);
    memset(&type, 0, sizeof(type));

    data = ELEMENT_WRAPPER_START "<require-instance value=\"false\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    assert_int_equal(type.require_instance, 0);
    assert_true(type.flags & LYS_SET_REQINST);
    memset(&type, 0, sizeof(type));

    data = ELEMENT_WRAPPER_START "<require-instance value=\"invalid\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_EVALID);
    memset(&type, 0, sizeof(type));
    CHECK_LOG_CTX("Invalid value \"invalid\" of \"value\" attribute in \"require-instance\" element. "
            "Valid values are \"true\" and \"false\".", "Line number 1.");
}

static void
test_revision_date_elem(void **state)
{
    const char *data;
    char rev[LY_REV_SIZE];
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<revision-date date=\"2000-01-01\">"EXT_SUBELEM "</revision-date>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, rev, NULL, &exts), LY_SUCCESS);
    assert_string_equal(rev, "2000-01-01");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_REVISION_DATE);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);

    data = ELEMENT_WRAPPER_START "<revision-date date=\"2000-01-01\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, rev, NULL, NULL), LY_SUCCESS);
    assert_string_equal(rev, "2000-01-01");

    data = ELEMENT_WRAPPER_START "<revision-date date=\"2000-50-05\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, rev, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"2000-50-05\" of \"revision-date\".", "Line number 1.");
}

static void
test_unique_elem(void **state)
{
    const char *data;
    const char **values = NULL;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<unique tag=\"tag\">"EXT_SUBELEM "</unique>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &values, NULL, &exts), LY_SUCCESS);
    assert_string_equal(*values, "tag");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_UNIQUE);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    lydict_remove(UTEST_LYCTX, *values);
    LY_ARRAY_FREE(values);
    values = NULL;

    data = ELEMENT_WRAPPER_START "<unique tag=\"tag\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &values, NULL, NULL), LY_SUCCESS);
    assert_string_equal(*values, "tag");
    lydict_remove(UTEST_LYCTX, *values);
    LY_ARRAY_FREE(values);
    values = NULL;
}

static void
test_units_elem(void **state)
{
    const char *data;
    const char *values = NULL;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<units name=\"name\">"EXT_SUBELEM "</units>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &values, NULL, &exts), LY_SUCCESS);
    assert_string_equal(values, "name");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_UNITS);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    lydict_remove(UTEST_LYCTX, values);
    values = NULL;

    data = ELEMENT_WRAPPER_START "<units name=\"name\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &values, NULL, NULL), LY_SUCCESS);
    assert_string_equal(values, "name");
    lydict_remove(UTEST_LYCTX, values);
    values = NULL;
}

static void
test_when_elem(void **state)
{
    const char *data;
    struct lysp_when *when = NULL;

    data = ELEMENT_WRAPPER_START
            "<when condition=\"cond\">\n"
            "    <description><text>desc</text></description>\n"
            "    <reference><text>ref</text></reference>\n"
            EXT_SUBELEM
            "</when>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &when, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_WHEN(when, "cond", "desc", 1, "ref");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(when->exts[0]), LY_STMT_WHEN);
    lysp_when_free(UTEST_LYCTX, when);
    free(when);
    when = NULL;

    data = ELEMENT_WRAPPER_START "<when condition=\"cond\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &when, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_WHEN(when, "cond", NULL, 0, NULL);
    lysp_when_free(UTEST_LYCTX, when);
    free(when);
    when = NULL;
}

static void
test_yin_text_value_elem(void **state)
{
    const char *data;
    const char *val;

    data = ELEMENT_WRAPPER_START "<text>text</text>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_SUCCESS);
    assert_string_equal(val, "text");
    lydict_remove(UTEST_LYCTX, val);

    data = "<error-message xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <value>text</value> </error-message>";
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_SUCCESS);
    assert_string_equal(val, "text");
    lydict_remove(UTEST_LYCTX, val);

    data = ELEMENT_WRAPPER_START "<text></text>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_SUCCESS);
    assert_string_equal("", val);
    lydict_remove(UTEST_LYCTX, val);
}

static void
test_type_elem(void **state)
{
    const char *data;
    struct lysp_type type = {};

    /* max subelems */
    data = ELEMENT_WRAPPER_START
            "<type name=\"type-name\">\n"
            "    <base name=\"base-name\"/>\n"
            "    <bit name=\"bit\"/>\n"
            "    <enum name=\"enum\"/>\n"
            "    <fraction-digits value=\"2\"/>\n"
            "    <length value=\"length\"/>\n"
            "    <path value=\"/path\"/>\n"
            "    <pattern value=\"pattern\"/>\n"
            "    <range value=\"range\" />\n"
            "    <require-instance value=\"true\"/>\n"
            "    <type name=\"sub-type-name\"/>\n"
            EXT_SUBELEM
            "</type>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    assert_string_equal(type.name, "type-name");
    assert_string_equal(*type.bases, "base-name");
    assert_string_equal(type.bits->name,  "bit");
    assert_string_equal(type.enums->name,  "enum");
    assert_int_equal(type.fraction_digits, 2);
    CHECK_LYSP_RESTR(type.length, "length", NULL,
            NULL, NULL, 0, NULL);
    assert_string_equal(type.path->expr, "/path");
    CHECK_LYSP_RESTR(type.patterns, "\006pattern", NULL,
            NULL, NULL, 0, NULL);
    CHECK_LYSP_RESTR(type.range, "range", NULL,
            NULL, NULL, 0, NULL);
    assert_int_equal(type.require_instance, 1);
    assert_string_equal(type.types->name, "sub-type-name");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(type.exts[0]), LY_STMT_TYPE);
    assert_true(type.flags & LYS_SET_BASE);
    assert_true(type.flags & LYS_SET_BIT);
    assert_true(type.flags & LYS_SET_ENUM);
    assert_true(type.flags & LYS_SET_FRDIGITS);
    assert_true(type.flags & LYS_SET_LENGTH);
    assert_true(type.flags & LYS_SET_PATH);
    assert_true(type.flags & LYS_SET_PATTERN);
    assert_true(type.flags & LYS_SET_RANGE);
    assert_true(type.flags & LYS_SET_REQINST);
    assert_true(type.flags & LYS_SET_TYPE);
    lysp_type_free(UTEST_LYCTX, &type);
    memset(&type, 0, sizeof(type));

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<type name=\"type-name\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    lysp_type_free(UTEST_LYCTX, &type);
    memset(&type, 0, sizeof(type));
}

static void
test_max_elems_elem(void **state)
{
    const char *data;
    struct lysp_node_list list = {};
    struct lysp_node_leaflist llist = {};
    struct lysp_refine refine = {};

    data = "<refine xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"unbounded\">"EXT_SUBELEM "</max-elements> </refine>";
    assert_int_equal(test_element_helper(state, data, &refine, NULL, NULL), LY_SUCCESS);
    assert_int_equal(refine.max, 0);
    assert_true(refine.flags & LYS_SET_MAX);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(refine.exts[0]), LY_STMT_MAX_ELEMENTS);
    FREE_ARRAY(UTEST_LYCTX, refine.exts, lysp_ext_instance_free);

    data = "<list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"5\">"EXT_SUBELEM "</max-elements> </list>";
    assert_int_equal(test_element_helper(state, data, &list, NULL, NULL), LY_SUCCESS);
    assert_int_equal(list.max, 5);
    CHECK_LYSP_NODE(&list, NULL, 1, LYS_SET_MAX, 0, NULL, 0, LYS_UNKNOWN, NULL, NULL, 0);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(list.exts[0]), LY_STMT_MAX_ELEMENTS);
    FREE_ARRAY(UTEST_LYCTX, list.exts, lysp_ext_instance_free);

    data = "<leaf-list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"85\">"EXT_SUBELEM "</max-elements> </leaf-list>";
    assert_int_equal(test_element_helper(state, data, &llist, NULL, NULL), LY_SUCCESS);
    assert_int_equal(llist.max, 85);
    CHECK_LYSP_NODE(&llist, NULL, 1, LYS_SET_MAX, 0, NULL, 0, LYS_UNKNOWN, NULL, NULL, 0);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(llist.exts[0]), LY_STMT_MAX_ELEMENTS);
    FREE_ARRAY(UTEST_LYCTX, llist.exts, lysp_ext_instance_free);

    data = "<refine xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"10\"/> </refine>";
    assert_int_equal(test_element_helper(state, data, &refine, NULL, NULL), LY_SUCCESS);
    assert_int_equal(refine.max, 10);
    assert_true(refine.flags & LYS_SET_MAX);

    data = "<list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"0\"/> </list>";
    assert_int_equal(test_element_helper(state, data, &list, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"0\" of \"value\" attribute in \"max-elements\" element.", "Line number 1.");

    data = "<list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"-10\"/> </list>";
    assert_int_equal(test_element_helper(state, data, &list, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"-10\" of \"value\" attribute in \"max-elements\" element.", "Line number 1.");

    data = "<list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"k\"/> </list>";
    assert_int_equal(test_element_helper(state, data, &list, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"k\" of \"value\" attribute in \"max-elements\" element.", "Line number 1.");

    data = "<list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"u12\"/> </list>";
    assert_int_equal(test_element_helper(state, data, &list, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"u12\" of \"value\" attribute in \"max-elements\" element.", "Line number 1.");
}

static void
test_min_elems_elem(void **state)
{
    const char *data;
    struct lysp_node_list list = {};
    struct lysp_node_leaflist llist = {};
    struct lysp_refine refine = {};

    data = "<refine xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"0\">"EXT_SUBELEM "</min-elements> </refine>";
    assert_int_equal(test_element_helper(state, data, &refine, NULL, NULL), LY_SUCCESS);
    assert_int_equal(refine.min, 0);
    assert_true(refine.flags & LYS_SET_MIN);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(refine.exts[0]), LY_STMT_MIN_ELEMENTS);
    FREE_ARRAY(UTEST_LYCTX, refine.exts, lysp_ext_instance_free);

    data = "<list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"41\">"EXT_SUBELEM "</min-elements> </list>";
    assert_int_equal(test_element_helper(state, data, &list, NULL, NULL), LY_SUCCESS);
    assert_int_equal(list.min, 41);
    assert_true(list.flags & LYS_SET_MIN);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(list.exts[0]), LY_STMT_MIN_ELEMENTS);
    FREE_ARRAY(UTEST_LYCTX, list.exts, lysp_ext_instance_free);

    data = "<leaf-list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"50\">"EXT_SUBELEM "</min-elements> </leaf-list>";
    assert_int_equal(test_element_helper(state, data, &llist, NULL, NULL), LY_SUCCESS);
    assert_int_equal(llist.min, 50);
    assert_true(llist.flags & LYS_SET_MIN);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(llist.exts[0]), LY_STMT_MIN_ELEMENTS);
    FREE_ARRAY(UTEST_LYCTX, llist.exts, lysp_ext_instance_free);

    data = "<leaf-list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"-5\"/> </leaf-list>";
    assert_int_equal(test_element_helper(state, data, &llist, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Value \"-5\" of \"value\" attribute in \"min-elements\" element is out of bounds.", "Line number 1.");

    data = "<leaf-list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"99999999999999999\"/> </leaf-list>";
    assert_int_equal(test_element_helper(state, data, &llist, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Value \"99999999999999999\" of \"value\" attribute in \"min-elements\" element is out of bounds.", "Line number 1.");

    data = "<leaf-list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"5k\"/> </leaf-list>";
    assert_int_equal(test_element_helper(state, data, &llist, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"5k\" of \"value\" attribute in \"min-elements\" element.", "Line number 1.");

    data = "<leaf-list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"05\"/> </leaf-list>";
    assert_int_equal(test_element_helper(state, data, &llist, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"05\" of \"value\" attribute in \"min-elements\" element.", "Line number 1.");
}

static void
test_ordby_elem(void **state)
{
    const char *data;
    uint16_t flags = 0;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<ordered-by value=\"system\">"EXT_SUBELEM "</ordered-by>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, &exts), LY_SUCCESS);
    assert_true(flags & LYS_ORDBY_SYSTEM);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_ORDERED_BY);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);

    data = ELEMENT_WRAPPER_START "<ordered-by value=\"user\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, NULL), LY_SUCCESS);
    assert_true(flags & LYS_ORDBY_USER);

    data = ELEMENT_WRAPPER_START "<ordered-by value=\"inv\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"inv\" of \"value\" attribute in \"ordered-by\" element. "
            "Valid values are \"system\" and \"user\".", "Line number 1.");
}

static void
test_any_elem(void **state)
{
    const char *data;
    struct lysp_node *siblings = NULL;
    struct tree_node_meta node_meta = {.parent = NULL, .nodes = &siblings};
    struct lysp_node_anydata *parsed = NULL;
    uint16_t flags;

    /* anyxml max subelems */
    data = ELEMENT_WRAPPER_START
            "<anyxml name=\"any-name\">\n"
            "    <config value=\"true\" />\n"
            "    <description><text>desc</text></description>\n"
            "    <if-feature name=\"feature\" />\n"
            "    <mandatory value=\"true\" />\n"
            "    <must condition=\"must-cond\" />\n"
            "    <reference><text>ref</text></reference>\n"
            "    <status value=\"deprecated\"/>\n"
            "    <when condition=\"when-cond\"/>\n"
            EXT_SUBELEM
            "</anyxml>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_anydata *)siblings;
    flags = LYS_CONFIG_W | LYS_MAND_TRUE | LYS_STATUS_DEPRC;
    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1,
            "any-name", 0, LYS_ANYXML, 0, "ref", 1);
    CHECK_LYSP_WHEN(parsed->when, "when-cond", NULL, 0, NULL);
    assert_string_equal(parsed->iffeatures[0].str, "feature");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_ANYXML);
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;

    /* anydata max subelems */
    data = ELEMENT_WRAPPER_START
            "<anydata name=\"any-name\">\n"
            "    <config value=\"true\" />\n"
            "    <description><text>desc</text></description>\n"
            "    <if-feature name=\"feature\" />\n"
            "    <mandatory value=\"true\" />\n"
            "    <must condition=\"must-cond\" />\n"
            "    <reference><text>ref</text></reference>\n"
            "    <status value=\"deprecated\"/>\n"
            "    <when condition=\"when-cond\"/>\n"
            EXT_SUBELEM
            "</anydata>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_anydata *)siblings;
    flags = LYS_CONFIG_W | LYS_MAND_TRUE | LYS_STATUS_DEPRC;
    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1,
            "any-name", 0, LYS_ANYDATA, 0, "ref", 1);
    CHECK_LYSP_WHEN(parsed->when, "when-cond", NULL, 0, NULL);
    assert_string_equal(parsed->iffeatures[0].str, "feature");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_ANYDATA);
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;

    /* min subelems */
    node_meta.parent = (void *)0x10;
    data = ELEMENT_WRAPPER_START "<anydata name=\"any-name\"> </anydata>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_anydata *)siblings;
    assert_ptr_equal(parsed->parent, node_meta.parent);
    CHECK_LYSP_NODE(parsed, NULL, 0, 0, 0,
            "any-name", 0, LYS_ANYDATA, 1, NULL, 0);
    lysp_node_free(UTEST_LYCTX, siblings);
}

static void
test_leaf_elem(void **state)
{
    const char *data;
    struct lysp_node *siblings = NULL;
    struct tree_node_meta node_meta = {.parent = NULL, .nodes = &siblings};
    struct lysp_node_leaf *parsed = NULL;
    uint16_t flags;

    /* max elements */
    data = ELEMENT_WRAPPER_START
            "<leaf name=\"leaf\">\n"
            "    <config value=\"true\" />\n"
            "    <default value=\"def-val\"/>\n"
            "    <description><text>desc</text></description>\n"
            "    <if-feature name=\"feature\" />\n"
            "    <mandatory value=\"true\" />\n"
            "    <must condition=\"must-cond\" />\n"
            "    <reference><text>ref</text></reference>\n"
            "    <status value=\"deprecated\"/>\n"
            "    <type name=\"type\"/>\n"
            "    <units name=\"uni\"/>\n"
            "    <when condition=\"when-cond\"/>\n"
            EXT_SUBELEM
            "</leaf>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_leaf *)siblings;
    flags = LYS_CONFIG_W | LYS_MAND_TRUE | LYS_STATUS_DEPRC;
    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1,
            "leaf", 0, LYS_LEAF, 0, "ref", 1);
    CHECK_LYSP_WHEN(parsed->when, "when-cond", NULL, 0, NULL);
    assert_string_equal(parsed->iffeatures[0].str, "feature");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_LEAF);
    assert_string_equal(parsed->musts->arg.str, "must-cond");
    assert_string_equal(parsed->type.name, "type");
    assert_string_equal(parsed->units, "uni");
    assert_string_equal(parsed->dflt.str, "def-val");
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;

    /* min elements */
    data = ELEMENT_WRAPPER_START "<leaf name=\"leaf\"> <type name=\"type\"/> </leaf>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_leaf *)siblings;
    assert_string_equal(parsed->name, "leaf");
    assert_string_equal(parsed->type.name, "type");
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;
}

static void
test_leaf_list_elem(void **state)
{
    const char *data;
    struct lysp_node *siblings = NULL;
    struct tree_node_meta node_meta = {.parent = NULL, .nodes = &siblings};
    struct lysp_node_leaflist *parsed = NULL;
    uint16_t flags;

    data = ELEMENT_WRAPPER_START
            "<leaf-list name=\"llist\">\n"
            "    <config value=\"true\" />\n"
            "    <default value=\"def-val0\"/>\n"
            "    <default value=\"def-val1\"/>\n"
            "    <description><text>desc</text></description>\n"
            "    <if-feature name=\"feature\"/>\n"
            "    <max-elements value=\"5\"/>\n"
            "    <must condition=\"must-cond\"/>\n"
            "    <ordered-by value=\"user\" />\n"
            "    <reference><text>ref</text></reference>\n"
            "    <status value=\"current\"/>\n"
            "    <type name=\"type\"/>\n"
            "    <units name=\"uni\"/>\n"
            "    <when condition=\"when-cond\"/>\n"
            EXT_SUBELEM
            "</leaf-list>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_leaflist *)siblings;
    flags = LYS_CONFIG_W | LYS_ORDBY_USER | LYS_STATUS_CURR | LYS_SET_MAX;
    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1,
            "llist", 0, LYS_LEAFLIST, 0, "ref", 1);
    CHECK_LYSP_RESTR(parsed->musts, "must-cond", NULL, NULL, NULL, 0, NULL);
    assert_string_equal(parsed->dflts[0].str, "def-val0");
    assert_string_equal(parsed->dflts[1].str, "def-val1");
    assert_string_equal(parsed->iffeatures[0].str, "feature");
    assert_int_equal(parsed->max, 5);
    assert_string_equal(parsed->type.name, "type");
    assert_string_equal(parsed->units, "uni");
    CHECK_LYSP_WHEN(parsed->when, "when-cond", NULL, 0, NULL);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_LEAF_LIST);
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;

    data = ELEMENT_WRAPPER_START
            "<leaf-list name=\"llist\">\n"
            "    <config value=\"true\" />\n"
            "    <description><text>desc</text></description>\n"
            "    <if-feature name=\"feature\"/>\n"
            "    <min-elements value=\"5\"/>\n"
            "    <must condition=\"must-cond\"/>\n"
            "    <ordered-by value=\"user\" />\n"
            "    <reference><text>ref</text></reference>\n"
            "    <status value=\"current\"/>\n"
            "    <type name=\"type\"/>\n"
            "    <units name=\"uni\"/>\n"
            "    <when condition=\"when-cond\"/>\n"
            EXT_SUBELEM
            "</leaf-list>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_leaflist *)siblings;
    flags = LYS_CONFIG_W | LYS_ORDBY_USER | LYS_STATUS_CURR | LYS_SET_MIN;
    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1,
            "llist", 0, LYS_LEAFLIST, 0, "ref", 1);
    CHECK_LYSP_RESTR(parsed->musts, "must-cond", NULL, NULL, NULL, 0, NULL);
    CHECK_LYSP_WHEN(parsed->when, "when-cond", NULL, 0, NULL);
    assert_string_equal(parsed->iffeatures[0].str, "feature");
    assert_int_equal(parsed->min, 5);
    assert_string_equal(parsed->type.name, "type");
    assert_string_equal(parsed->units, "uni");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_LEAF_LIST);
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;

    data = ELEMENT_WRAPPER_START
            "<leaf-list name=\"llist\">\n"
            "    <config value=\"true\" />\n"
            "    <description><text>desc</text></description>\n"
            "    <if-feature name=\"feature\"/>\n"
            "    <max-elements value=\"15\"/>\n"
            "    <min-elements value=\"5\"/>\n"
            "    <must condition=\"must-cond\"/>\n"
            "    <ordered-by value=\"user\" />\n"
            "    <reference><text>ref</text></reference>\n"
            "    <status value=\"current\"/>\n"
            "    <type name=\"type\"/>\n"
            "    <units name=\"uni\"/>\n"
            "    <when condition=\"when-cond\"/>\n"
            "</leaf-list>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_leaflist *)siblings;
    flags = LYS_CONFIG_W | LYS_ORDBY_USER | LYS_STATUS_CURR | LYS_SET_MIN | LYS_SET_MAX;
    CHECK_LYSP_NODE(parsed, "desc", 0, flags, 1,
            "llist", 0, LYS_LEAFLIST, 0, "ref", 1);
    CHECK_LYSP_RESTR(parsed->musts, "must-cond", NULL, NULL, NULL, 0, NULL);
    CHECK_LYSP_WHEN(parsed->when, "when-cond", NULL, 0, NULL);
    assert_string_equal(parsed->iffeatures[0].str, "feature");
    assert_int_equal(parsed->min, 5);
    assert_int_equal(parsed->max, 15);
    assert_string_equal(parsed->type.name, "type");
    assert_string_equal(parsed->units, "uni");
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;

    data = ELEMENT_WRAPPER_START
            "<leaf-list name=\"llist\">\n"
            "    <type name=\"type\"/>\n"
            "</leaf-list>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_leaflist *)siblings;
    assert_string_equal(parsed->name, "llist");
    assert_string_equal(parsed->type.name, "type");
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;

    /* invalid combinations */
    data = ELEMENT_WRAPPER_START
            "<leaf-list name=\"llist\">\n"
            "    <max-elements value=\"5\"/>\n"
            "    <min-elements value=\"15\"/>\n"
            "    <type name=\"type\"/>"
            "</leaf-list>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid combination of min-elements and max-elements: min value 15 is bigger than the max value 5.", "Line number 4.");
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;

    data = ELEMENT_WRAPPER_START
            "<leaf-list name=\"llist\">\n"
            "    <default value=\"def-val1\"/>\n"
            "    <min-elements value=\"15\"/>\n"
            "    <type name=\"type\"/>\n"
            "</leaf-list>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid combination of sub-elemnts \"min-elements\" and \"default\" in \"leaf-list\" element.", "Line number 5.");
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;

    data = ELEMENT_WRAPPER_START
            "<leaf-list name=\"llist\">"
            "</leaf-list>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory sub-element \"type\" of \"leaf-list\" element.", "Line number 1.");
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;
}

static void
test_presence_elem(void **state)
{
    const char *data;
    const char *val;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<presence value=\"presence-val\">"EXT_SUBELEM "</presence>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, &exts), LY_SUCCESS);
    assert_string_equal(val, "presence-val");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_PRESENCE);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    lydict_remove(UTEST_LYCTX, val);

    data = ELEMENT_WRAPPER_START "<presence value=\"presence-val\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_SUCCESS);
    assert_string_equal(val, "presence-val");
    lydict_remove(UTEST_LYCTX, val);

    data = ELEMENT_WRAPPER_START "<presence/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory attribute value of presence element.", "Line number 1.");
}

static void
test_key_elem(void **state)
{
    const char *data;
    const char *val;
    struct lysp_ext_instance *exts = NULL;

    data = ELEMENT_WRAPPER_START "<key value=\"key-value\">"EXT_SUBELEM "</key>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, &exts), LY_SUCCESS);
    assert_string_equal(val, "key-value");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(exts[0]), LY_STMT_KEY);
    FREE_ARRAY(UTEST_LYCTX, exts, lysp_ext_instance_free);
    lydict_remove(UTEST_LYCTX, val);

    data = ELEMENT_WRAPPER_START "<key value=\"key-value\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_SUCCESS);
    assert_string_equal(val, "key-value");
    lydict_remove(UTEST_LYCTX, val);

    data = ELEMENT_WRAPPER_START "<key/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory attribute value of key element.", "Line number 1.");
}

static void
test_typedef_elem(void **state)
{
    const char *data;
    struct lysp_tpdf *tpdfs = NULL;
    struct tree_node_meta typdef_meta = {NULL, (struct lysp_node **)&tpdfs};

    data = ELEMENT_WRAPPER_START
            "<typedef name=\"tpdf-name\">\n"
            "    <default value=\"def-val\"/>\n"
            "    <description><text>desc-text</text></description>\n"
            "    <reference><text>ref-text</text></reference>\n"
            "    <status value=\"current\"/>\n"
            "    <type name=\"type\"/>\n"
            "    <units name=\"uni\"/>\n"
            EXT_SUBELEM
            "</typedef>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &typdef_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(tpdfs[0].dflt.str, "def-val");
    assert_string_equal(tpdfs[0].dsc, "desc-text");
    assert_string_equal(tpdfs[0].name, "tpdf-name");
    assert_string_equal(tpdfs[0].ref, "ref-text");
    assert_string_equal(tpdfs[0].type.name, "type");
    assert_string_equal(tpdfs[0].units, "uni");
    assert_true(tpdfs[0].flags & LYS_STATUS_CURR);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(tpdfs[0].exts[0]), LY_STMT_TYPEDEF);
    FREE_ARRAY(UTEST_LYCTX, tpdfs, lysp_tpdf_free);
    tpdfs = NULL;

    data = ELEMENT_WRAPPER_START
            "<typedef name=\"tpdf-name\">\n"
            "    <type name=\"type\"/>\n"
            "</typedef>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &typdef_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(tpdfs[0].name, "tpdf-name");
    assert_string_equal(tpdfs[0].type.name, "type");
    FREE_ARRAY(UTEST_LYCTX, tpdfs, lysp_tpdf_free);
    tpdfs = NULL;
}

static void
test_refine_elem(void **state)
{
    const char *data;
    struct lysp_refine *refines = NULL;

    /* max subelems */
    data = ELEMENT_WRAPPER_START
            "<refine target-node=\"target\">\n"
            "    <if-feature name=\"feature\" />\n"
            "    <must condition=\"cond\" />\n"
            "    <presence value=\"presence\" />\n"
            "    <default value=\"def\" />\n"
            "    <config value=\"true\" />\n"
            "    <mandatory value=\"true\" />\n"
            "    <min-elements value=\"10\" />\n"
            "    <max-elements value=\"20\" />\n"
            "    <description><text>desc</text></description>\n"
            "    <reference><text>ref</text></reference>\n"
            EXT_SUBELEM
            "</refine>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &refines, NULL, NULL), LY_SUCCESS);
    assert_string_equal(refines->nodeid, "target");
    assert_string_equal(refines->dflts[0].str, "def");
    assert_string_equal(refines->dsc, "desc");
    assert_true(refines->flags & LYS_CONFIG_W);
    assert_true(refines->flags & LYS_MAND_TRUE);
    assert_string_equal(refines->iffeatures[0].str, "feature");
    assert_int_equal(refines->max, 20);
    assert_int_equal(refines->min, 10);
    assert_string_equal(refines->musts->arg.str, "cond");
    assert_string_equal(refines->presence, "presence");
    assert_string_equal(refines->ref, "ref");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(refines->exts[0]), LY_STMT_REFINE);
    FREE_ARRAY(UTEST_LYCTX, refines, lysp_refine_free);
    refines = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<refine target-node=\"target\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &refines, NULL, NULL), LY_SUCCESS);
    assert_string_equal(refines->nodeid, "target");
    FREE_ARRAY(UTEST_LYCTX, refines, lysp_refine_free);
    refines = NULL;
}

static void
test_uses_elem(void **state)
{
    const char *data;
    struct lysp_node *siblings = NULL;
    struct tree_node_meta node_meta = {NULL, &siblings};
    struct lysp_node_uses *parsed = NULL;

    /* max subelems */
    data = ELEMENT_WRAPPER_START
            "<uses name=\"uses-name\">\n"
            "    <when condition=\"cond\" />\n"
            "    <if-feature name=\"feature\" />\n"
            "    <status value=\"obsolete\" />\n"
            "    <description><text>desc</text></description>\n"
            "    <reference><text>ref</text></reference>\n"
            "    <refine target-node=\"target\"/>\n"
            "    <augment target-node=\"target\" />\n"
            EXT_SUBELEM
            "</uses>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_uses *)&siblings[0];
    CHECK_LYSP_NODE(parsed, "desc", 1, LYS_STATUS_OBSLT, 1,
            "uses-name", 0, LYS_USES, 0, "ref", 1);
    CHECK_LYSP_WHEN(parsed->when, "cond", NULL, 0, NULL);
    assert_string_equal(parsed->iffeatures[0].str, "feature");
    assert_string_equal(parsed->refines->nodeid, "target");
    assert_string_equal(parsed->augments->nodeid, "target");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_USES);
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<uses name=\"uses-name\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(siblings[0].name, "uses-name");
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;
}

static void
test_revision_elem(void **state)
{
    const char *data;
    struct lysp_revision *revs = NULL;

    /* max subelems */
    data = ELEMENT_WRAPPER_START
            "<revision date=\"2018-12-25\">\n"
            "    <description><text>desc</text></description>\n"
            "    <reference><text>ref</text></reference>\n"
            EXT_SUBELEM
            "</revision>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &revs, NULL, NULL), LY_SUCCESS);
    assert_string_equal(revs->date, "2018-12-25");
    assert_string_equal(revs->dsc, "desc");
    assert_string_equal(revs->ref, "ref");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(revs->exts[0]), LY_STMT_REVISION);
    FREE_ARRAY(UTEST_LYCTX, revs, lysp_revision_free);
    revs = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<revision date=\"2005-05-05\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &revs, NULL, NULL), LY_SUCCESS);
    assert_string_equal(revs->date, "2005-05-05");
    FREE_ARRAY(UTEST_LYCTX, revs, lysp_revision_free);
    revs = NULL;

    /* invalid value */
    data = ELEMENT_WRAPPER_START "<revision date=\"05-05-2005\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &revs, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"05-05-2005\" of \"revision\".", "Line number 1.");
    FREE_ARRAY(UTEST_LYCTX, revs, lysp_revision_free);
    revs = NULL;
}

static void
test_include_elem(void **state)
{
    const char *data;
    struct lysp_include *includes = NULL;
    struct include_meta inc_meta = {"module-name", &includes};

    /* max subelems */
    YCTX->parsed_mod->version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
            "<include module=\"mod\">\n"
            "    <description><text>desc</text></description>\n"
            "    <reference><text>ref</text></reference>\n"
            "    <revision-date date=\"1999-09-09\"/>\n"
            EXT_SUBELEM
            "</include>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &inc_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(includes->name, "mod");
    assert_string_equal(includes->dsc, "desc");
    assert_string_equal(includes->ref, "ref");
    assert_string_equal(includes->rev, "1999-09-09");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(includes->exts[0]), LY_STMT_INCLUDE);
    FREE_ARRAY(UTEST_LYCTX, includes, lysp_include_free);
    includes = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<include module=\"mod\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &inc_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(includes->name, "mod");
    FREE_ARRAY(UTEST_LYCTX, includes, lysp_include_free);
    includes = NULL;

    /* invalid combinations */
    YCTX->parsed_mod->version = LYS_VERSION_1_0;
    data = ELEMENT_WRAPPER_START
            "<include module=\"mod\">\n"
            "    <description><text>desc</text></description>\n"
            "    <revision-date date=\"1999-09-09\"/>\n"
            "</include>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &inc_meta, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid sub-elemnt \"description\" of \"include\" element - this sub-element is allowed only in modules with version 1.1 or newer.",
            "Line number 2.");
    FREE_ARRAY(UTEST_LYCTX, includes, lysp_include_free);
    includes = NULL;

    YCTX->parsed_mod->version = LYS_VERSION_1_0;
    data = ELEMENT_WRAPPER_START
            "<include module=\"mod\">\n"
            "    <reference><text>ref</text></reference>\n"
            "    <revision-date date=\"1999-09-09\"/>\n"
            "</include>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &inc_meta, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid sub-elemnt \"reference\" of \"include\" element - this sub-element is allowed only in modules with version 1.1 or newer.",
            "Line number 2.");
    FREE_ARRAY(UTEST_LYCTX, includes, lysp_include_free);
    includes = NULL;
}

static void
test_list_elem(void **state)
{
    const char *data;
    struct lysp_node *siblings = NULL;
    struct tree_node_meta node_meta = {NULL, &siblings};
    struct lysp_node_list *parsed = NULL;

    /* max subelems */
    data = ELEMENT_WRAPPER_START
            "<list name=\"list-name\">\n"
            "    <when condition=\"when\"/>\n"
            "    <if-feature name=\"iff\"/>\n"
            "    <must condition=\"must-cond\"/>\n"
            "    <key value=\"key\"/>\n"
            "    <unique tag=\"utag\"/>\n"
            "    <config value=\"true\"/>\n"
            "    <min-elements value=\"10\"/>\n"
            "    <ordered-by value=\"user\"/>\n"
            "    <status value=\"deprecated\"/>\n"
            "    <description><text>desc</text></description>\n"
            "    <reference><text>ref</text></reference>\n"
            "    <anydata name=\"anyd\"/>\n"
            "    <anyxml name=\"anyx\"/>\n"
            "    <container name=\"cont\"/>\n"
            "    <choice name=\"choice\"/>\n"
            "    <action name=\"action\"/>\n"
            "    <grouping name=\"grp\"/>\n"
            "    <notification name=\"notf\"/>\n"
            "    <leaf name=\"leaf\"> <type name=\"type\"/> </leaf>\n"
            "    <leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>\n"
            "    <list name=\"sub-list\"/>\n"
            "    <typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>\n"
            "    <uses name=\"uses-name\"/>\n"
            EXT_SUBELEM
            "</list>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_list *)&siblings[0];
    assert_string_equal(parsed->child->name, "anyd");
    assert_int_equal(parsed->child->nodetype, LYS_ANYDATA);
    assert_string_equal(parsed->child->next->name, "anyx");
    assert_int_equal(parsed->child->next->nodetype, LYS_ANYXML);
    assert_string_equal(parsed->child->next->next->name, "cont");
    assert_int_equal(parsed->child->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(parsed->child->next->next->next->name, "choice");
    assert_int_equal(parsed->child->next->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(parsed->child->next->next->next->next->name, "leaf");
    assert_int_equal(parsed->child->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(parsed->child->next->next->next->next->next->name, "llist");
    assert_int_equal(parsed->child->next->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(parsed->child->next->next->next->next->next->next->name, "sub-list");
    assert_int_equal(parsed->child->next->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(parsed->child->next->next->next->next->next->next->next->name, "uses-name");
    assert_int_equal(parsed->child->next->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_null(parsed->child->next->next->next->next->next->next->next->next);
    uint16_t flags = LYS_ORDBY_USER | LYS_STATUS_DEPRC | LYS_CONFIG_W | LYS_SET_MIN;

    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1,
            "list-name", 0, LYS_LIST, 0, "ref", 1);
    CHECK_LYSP_RESTR(parsed->musts, "must-cond", NULL, NULL, NULL, 0, NULL);
    CHECK_LYSP_WHEN(parsed->when, "when", NULL, 0, NULL);
    assert_string_equal(parsed->groupings->name, "grp");
    assert_string_equal(parsed->actions->name, "action");
    assert_int_equal(parsed->groupings->nodetype, LYS_GROUPING);
    assert_string_equal(parsed->notifs->name, "notf");
    assert_string_equal(parsed->iffeatures[0].str, "iff");
    assert_string_equal(parsed->key, "key");
    assert_int_equal(parsed->min, 10);
    assert_string_equal(parsed->typedefs->name, "tpdf");
    assert_string_equal(parsed->uniques->str, "utag");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_LIST);
    lysp_node_free(UTEST_LYCTX, siblings);
    ly_set_erase(&YCTX->tpdfs_nodes, NULL);
    siblings = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<list name=\"list-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_list *)&siblings[0];
    CHECK_LYSP_NODE(parsed, NULL, 0, 0, 0,
            "list-name", 0, LYS_LIST, 0, NULL, 0);
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;
}

static void
test_notification_elem(void **state)
{
    const char *data;
    struct lysp_node_notif *notifs = NULL;
    struct tree_node_meta notif_meta = {NULL, (struct lysp_node **)&notifs};

    /* max subelems */
    YCTX->parsed_mod->version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
            "<notification name=\"notif-name\">\n"
            "    <anydata name=\"anyd\"/>\n"
            "    <anyxml name=\"anyx\"/>\n"
            "    <description><text>desc</text></description>\n"
            "    <if-feature name=\"iff\"/>\n"
            "    <leaf name=\"leaf\"> <type name=\"type\"/> </leaf>\n"
            "    <leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>\n"
            "    <list name=\"sub-list\"/>\n"
            "    <must condition=\"cond\"/>\n"
            "    <reference><text>ref</text></reference>\n"
            "    <status value=\"deprecated\"/>\n"
            "    <typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>\n"
            "    <uses name=\"uses-name\"/>\n"
            "    <container name=\"cont\"/>\n"
            "    <choice name=\"choice\"/>\n"
            "    <grouping name=\"grp\"/>\n"
            EXT_SUBELEM
            "</notification>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &notif_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(notifs->name, "notif-name");
    assert_string_equal(notifs->child->name, "anyd");
    assert_int_equal(notifs->child->nodetype, LYS_ANYDATA);
    assert_string_equal(notifs->child->next->name, "anyx");
    assert_int_equal(notifs->child->next->nodetype, LYS_ANYXML);
    assert_string_equal(notifs->child->next->next->name, "leaf");
    assert_int_equal(notifs->child->next->next->nodetype, LYS_LEAF);
    assert_string_equal(notifs->child->next->next->next->name, "llist");
    assert_int_equal(notifs->child->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(notifs->child->next->next->next->next->name, "sub-list");
    assert_int_equal(notifs->child->next->next->next->next->nodetype, LYS_LIST);
    assert_true(notifs->flags & LYS_STATUS_DEPRC);
    assert_string_equal(notifs->groupings->name, "grp");
    assert_int_equal(notifs->groupings->nodetype, LYS_GROUPING);
    assert_string_equal(notifs->child->next->next->next->next->next->name, "uses-name");
    assert_int_equal(notifs->child->next->next->next->next->next->nodetype, LYS_USES);
    assert_string_equal(notifs->child->next->next->next->next->next->next->name, "cont");
    assert_int_equal(notifs->child->next->next->next->next->next->next->nodetype, LYS_CONTAINER);
    assert_int_equal(notifs->child->next->next->next->next->next->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(notifs->child->next->next->next->next->next->next->next->name, "choice");
    assert_null(notifs->child->next->next->next->next->next->next->next->next);
    assert_string_equal(notifs->iffeatures[0].str, "iff");
    assert_string_equal(notifs->musts->arg.str, "cond");
    assert_int_equal(notifs->nodetype, LYS_NOTIF);
    assert_null(notifs->parent);
    assert_string_equal(notifs->ref, "ref");
    assert_string_equal(notifs->typedefs->name, "tpdf");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(notifs->exts[0]), LY_STMT_NOTIFICATION);
    lysp_node_free(UTEST_LYCTX, (struct lysp_node *)notifs);
    notifs = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<notification name=\"notif-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &notif_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(notifs->name, "notif-name");
    lysp_node_free(UTEST_LYCTX, (struct lysp_node *)notifs);
    notifs = NULL;
}

static void
test_grouping_elem(void **state)
{
    const char *data;
    struct lysp_node_grp *grps = NULL;
    struct tree_node_meta grp_meta = {NULL, (struct lysp_node **)&grps};

    /* max subelems */
    data = ELEMENT_WRAPPER_START
            "<grouping name=\"grp-name\">\n"
            "    <anydata name=\"anyd\"/>\n"
            "    <anyxml name=\"anyx\"/>\n"
            "    <description><text>desc</text></description>\n"
            "    <grouping name=\"sub-grp\"/>\n"
            "    <leaf name=\"leaf\"> <type name=\"type\"/> </leaf>\n"
            "    <leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>\n"
            "    <list name=\"list\"/>\n"
            "    <notification name=\"notf\"/>\n"
            "    <reference><text>ref</text></reference>\n"
            "    <status value=\"current\"/>\n"
            "    <typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>\n"
            "    <uses name=\"uses-name\"/>\n"
            "    <action name=\"act\"/>\n"
            "    <container name=\"cont\"/>\n"
            "    <choice name=\"choice\"/>\n"
            EXT_SUBELEM
            "</grouping>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &grp_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(grps->name, "grp-name");
    assert_string_equal(grps->child->name, "anyd");
    assert_string_equal(grps->child->next->name, "anyx");
    assert_string_equal(grps->child->next->next->name, "leaf");
    assert_string_equal(grps->child->next->next->next->name, "llist");
    assert_string_equal(grps->child->next->next->next->next->name, "list");
    assert_string_equal(grps->dsc, "desc");
    assert_true(grps->flags & LYS_STATUS_CURR);
    assert_string_equal(grps->groupings->name, "sub-grp");
    assert_int_equal(grps->nodetype, LYS_GROUPING);
    assert_string_equal(grps->notifs->name, "notf");
    assert_null(grps->parent);
    assert_string_equal(grps->ref, "ref");
    assert_string_equal(grps->typedefs->name, "tpdf");
    assert_string_equal(grps->actions->name, "act");
    assert_string_equal(grps->child->next->next->next->next->next->name, "uses-name");
    assert_int_equal(grps->child->next->next->next->next->next->nodetype, LYS_USES);
    assert_string_equal(grps->child->next->next->next->next->next->next->name, "cont");
    assert_int_equal(grps->child->next->next->next->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(grps->child->next->next->next->next->next->next->next->name, "choice");
    assert_int_equal(grps->child->next->next->next->next->next->next->next->nodetype, LYS_CHOICE);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(grps->exts[0]), LY_STMT_GROUPING);
    lysp_node_free(UTEST_LYCTX, &grps->node);
    grps = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<grouping name=\"grp-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &grp_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(grps->name, "grp-name");
    lysp_node_free(UTEST_LYCTX, &grps->node);
    grps = NULL;
}

static void
test_container_elem(void **state)
{
    const char *data;
    struct lysp_node *siblings = NULL;
    struct tree_node_meta node_meta = {NULL, &siblings};
    struct lysp_node_container *parsed = NULL;

    /* max subelems */
    YCTX->parsed_mod->version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
            "<container name=\"cont-name\">\n"
            "    <anydata name=\"anyd\"/>\n"
            "    <anyxml name=\"anyx\"/>\n"
            "    <config value=\"true\"/>\n"
            "    <container name=\"subcont\"/>\n"
            "    <description><text>desc</text></description>\n"
            "    <grouping name=\"sub-grp\"/>\n"
            "    <if-feature name=\"iff\"/>\n"
            "    <leaf name=\"leaf\"> <type name=\"type\"/> </leaf>\n"
            "    <leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>\n"
            "    <list name=\"list\"/>\n"
            "    <must condition=\"cond\"/>\n"
            "    <notification name=\"notf\"/>\n"
            "    <presence value=\"presence\"/>\n"
            "    <reference><text>ref</text></reference>\n"
            "    <status value=\"current\"/>\n"
            "    <typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>\n"
            "    <uses name=\"uses-name\"/>\n"
            "    <when condition=\"when-cond\"/>\n"
            "    <action name=\"act\"/>\n"
            "    <choice name=\"choice\"/>\n"
            EXT_SUBELEM
            "</container>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_container *)siblings;
    uint16_t flags = LYS_CONFIG_W | LYS_STATUS_CURR;

    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1,
            "cont-name", 0, LYS_CONTAINER, 0, "ref", 1);
    CHECK_LYSP_RESTR(parsed->musts, "cond", NULL, NULL, NULL, 0, NULL);
    CHECK_LYSP_WHEN(parsed->when, "when-cond", NULL, 0, NULL);

    assert_string_equal(parsed->iffeatures[0].str, "iff");
    assert_string_equal(parsed->presence, "presence");
    assert_string_equal(parsed->typedefs->name, "tpdf");
    assert_string_equal(parsed->groupings->name, "sub-grp");
    assert_string_equal(parsed->child->name, "anyd");
    assert_int_equal(parsed->child->nodetype, LYS_ANYDATA);
    assert_string_equal(parsed->child->next->name, "anyx");
    assert_int_equal(parsed->child->next->nodetype, LYS_ANYXML);
    assert_string_equal(parsed->child->next->next->name, "subcont");
    assert_int_equal(parsed->child->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(parsed->child->next->next->next->name, "leaf");
    assert_int_equal(parsed->child->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(parsed->child->next->next->next->next->name, "llist");
    assert_int_equal(parsed->child->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(parsed->child->next->next->next->next->next->name, "list");
    assert_int_equal(parsed->child->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(parsed->child->next->next->next->next->next->next->name, "uses-name");
    assert_int_equal(parsed->child->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_string_equal(parsed->child->next->next->next->next->next->next->next->name, "choice");
    assert_int_equal(parsed->child->next->next->next->next->next->next->next->nodetype, LYS_CHOICE);
    assert_null(parsed->child->next->next->next->next->next->next->next->next);
    assert_string_equal(parsed->notifs->name, "notf");
    assert_string_equal(parsed->actions->name, "act");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_CONTAINER);
    lysp_node_free(UTEST_LYCTX, siblings);
    ly_set_erase(&YCTX->tpdfs_nodes, NULL);
    siblings = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<container name=\"cont-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_container *)siblings;
    CHECK_LYSP_NODE(parsed, NULL, 0, 0, 0,
            "cont-name", 0, LYS_CONTAINER, 0, NULL, 0);
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;
}

static void
test_case_elem(void **state)
{
    const char *data;
    struct lysp_node *siblings = NULL;
    struct tree_node_meta node_meta = {NULL, &siblings};
    struct lysp_node_case *parsed = NULL;

    /* max subelems */
    YCTX->parsed_mod->version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
            "<case name=\"case-name\">\n"
            "    <anydata name=\"anyd\"/>\n"
            "    <anyxml name=\"anyx\"/>\n"
            "    <container name=\"subcont\"/>\n"
            "    <description><text>desc</text></description>\n"
            "    <if-feature name=\"iff\"/>\n"
            "    <leaf name=\"leaf\"> <type name=\"type\"/> </leaf>\n"
            "    <leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>\n"
            "    <list name=\"list\"/>\n"
            "    <reference><text>ref</text></reference>\n"
            "    <status value=\"current\"/>\n"
            "    <uses name=\"uses-name\"/>\n"
            "    <when condition=\"when-cond\"/>\n"
            "    <choice name=\"choice\"/>\n"
            EXT_SUBELEM
            "</case>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_case *)siblings;
    uint16_t flags = LYS_STATUS_CURR;

    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1,
            "case-name", 0, LYS_CASE, 0, "ref", 1);
    CHECK_LYSP_WHEN(parsed->when, "when-cond", NULL, 0, NULL);
    assert_string_equal(parsed->iffeatures[0].str, "iff");
    assert_string_equal(parsed->child->name, "anyd");
    assert_int_equal(parsed->child->nodetype, LYS_ANYDATA);
    assert_string_equal(parsed->child->next->name, "anyx");
    assert_int_equal(parsed->child->next->nodetype, LYS_ANYXML);
    assert_string_equal(parsed->child->next->next->name, "subcont");
    assert_int_equal(parsed->child->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(parsed->child->next->next->next->name, "leaf");
    assert_int_equal(parsed->child->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(parsed->child->next->next->next->next->name, "llist");
    assert_int_equal(parsed->child->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(parsed->child->next->next->next->next->next->name, "list");
    assert_int_equal(parsed->child->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(parsed->child->next->next->next->next->next->next->name, "uses-name");
    assert_int_equal(parsed->child->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_string_equal(parsed->child->next->next->next->next->next->next->next->name, "choice");
    assert_int_equal(parsed->child->next->next->next->next->next->next->next->nodetype, LYS_CHOICE);
    assert_null(parsed->child->next->next->next->next->next->next->next->next);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_CASE);
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<case name=\"case-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_case *)siblings;
    CHECK_LYSP_NODE(parsed, NULL, 0, 0, 0,
            "case-name", 0, LYS_CASE, 0, NULL, 0);
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;
}

static void
test_choice_elem(void **state)
{
    const char *data;
    struct lysp_node *siblings = NULL;
    struct tree_node_meta node_meta = {NULL, &siblings};
    struct lysp_node_choice *parsed = NULL;

    /* max subelems */
    YCTX->parsed_mod->version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
            "<choice name=\"choice-name\">\n"
            "    <anydata name=\"anyd\"/>\n"
            "    <anyxml name=\"anyx\"/>\n"
            "    <case name=\"sub-case\"/>\n"
            "    <choice name=\"choice\"/>\n"
            "    <config value=\"true\"/>\n"
            "    <container name=\"subcont\"/>\n"
            "    <default value=\"def\"/>\n"
            "    <description><text>desc</text></description>\n"
            "    <if-feature name=\"iff\"/>\n"
            "    <leaf name=\"leaf\"> <type name=\"type\"/> </leaf>\n"
            "    <leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>\n"
            "    <list name=\"list\"/>\n"
            "    <mandatory value=\"true\" />\n"
            "    <reference><text>ref</text></reference>\n"
            "    <status value=\"current\"/>\n"
            "    <when condition=\"when-cond\"/>\n"
            EXT_SUBELEM
            "</choice>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_choice *)siblings;
    uint16_t flags = LYS_CONFIG_W | LYS_MAND_TRUE | LYS_STATUS_CURR;

    CHECK_LYSP_NODE(parsed, "desc", 1, flags, 1,
            "choice-name", 0, LYS_CHOICE, 0, "ref", 1);
    CHECK_LYSP_WHEN(parsed->when, "when-cond", NULL, 0, NULL);
    assert_string_equal(parsed->iffeatures[0].str, "iff");
    assert_string_equal(parsed->child->name, "anyd");
    assert_int_equal(parsed->child->nodetype, LYS_ANYDATA);
    assert_string_equal(parsed->child->next->name, "anyx");
    assert_int_equal(parsed->child->next->nodetype, LYS_ANYXML);
    assert_string_equal(parsed->child->next->next->name, "sub-case");
    assert_int_equal(parsed->child->next->next->nodetype, LYS_CASE);
    assert_string_equal(parsed->child->next->next->next->name, "choice");
    assert_int_equal(parsed->child->next->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(parsed->child->next->next->next->next->name, "subcont");
    assert_int_equal(parsed->child->next->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(parsed->child->next->next->next->next->next->name, "leaf");
    assert_int_equal(parsed->child->next->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(parsed->child->next->next->next->next->next->next->name, "llist");
    assert_int_equal(parsed->child->next->next->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(parsed->child->next->next->next->next->next->next->next->name, "list");
    assert_int_equal(parsed->child->next->next->next->next->next->next->next->nodetype, LYS_LIST);
    assert_null(parsed->child->next->next->next->next->next->next->next->next);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(parsed->exts[0]), LY_STMT_CHOICE);
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<choice name=\"choice-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_choice *)siblings;
    assert_string_equal(parsed->name, "choice-name");
    CHECK_LYSP_NODE(parsed, NULL, 0, 0, 0,
            "choice-name", 0, LYS_CHOICE, 0, NULL, 0);
    lysp_node_free(UTEST_LYCTX, siblings);
    siblings = NULL;
}

static void
test_inout_elem(void **state)
{
    const char *data;
    struct lysp_node_action_inout inout = {};
    struct inout_meta inout_meta = {NULL, &inout};

    /* max subelements */
    YCTX->parsed_mod->version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
            "<input>\n"
            "    <anydata name=\"anyd\"/>\n"
            "    <anyxml name=\"anyx\"/>\n"
            "    <choice name=\"choice\"/>\n"
            "    <container name=\"subcont\"/>\n"
            "    <grouping name=\"sub-grp\"/>\n"
            "    <leaf name=\"leaf\"> <type name=\"type\"/> </leaf>\n"
            "    <leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>\n"
            "    <list name=\"list\"/>\n"
            "    <must condition=\"cond\"/>\n"
            "    <typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>\n"
            "    <uses name=\"uses-name\"/>\n"
            EXT_SUBELEM
            "</input>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &inout_meta, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_ACTION_INOUT(&(inout), 1, 1, 1, 1, LYS_INPUT, 0, 1);
    CHECK_LYSP_RESTR(inout.musts, "cond", NULL, NULL, NULL, 0, NULL);
    assert_string_equal(inout.typedefs->name, "tpdf");
    assert_string_equal(inout.groupings->name, "sub-grp");
    assert_string_equal(inout.child->name, "anyd");
    assert_int_equal(inout.child->nodetype, LYS_ANYDATA);
    assert_string_equal(inout.child->next->name, "anyx");
    assert_int_equal(inout.child->next->nodetype, LYS_ANYXML);
    assert_string_equal(inout.child->next->next->name, "choice");
    assert_int_equal(inout.child->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(inout.child->next->next->next->name, "subcont");
    assert_int_equal(inout.child->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(inout.child->next->next->next->next->name, "leaf");
    assert_int_equal(inout.child->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(inout.child->next->next->next->next->next->name, "llist");
    assert_int_equal(inout.child->next->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(inout.child->next->next->next->next->next->next->name, "list");
    assert_int_equal(inout.child->next->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(inout.child->next->next->next->next->next->next->next->name, "uses-name");
    assert_int_equal(inout.child->next->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_null(inout.child->next->next->next->next->next->next->next->next);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(inout.exts[0]), LY_STMT_INPUT);
    lysp_node_free(UTEST_LYCTX, (struct lysp_node *)&inout);
    memset(&inout, 0, sizeof inout);

    /* max subelements */
    YCTX->parsed_mod->version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
            "<output>\n"
            "    <anydata name=\"anyd\"/>\n"
            "    <anyxml name=\"anyx\"/>\n"
            "    <choice name=\"choice\"/>\n"
            "    <container name=\"subcont\"/>\n"
            "    <grouping name=\"sub-grp\"/>\n"
            "    <leaf name=\"leaf\"> <type name=\"type\"/> </leaf>\n"
            "    <leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>\n"
            "    <list name=\"list\"/>\n"
            "    <must condition=\"cond\"/>\n"
            "    <typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>\n"
            "    <uses name=\"uses-name\"/>\n"
            EXT_SUBELEM
            "</output>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &inout_meta, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_ACTION_INOUT(&(inout), 1, 1, 1, 1, LYS_OUTPUT, 0, 1);
    assert_string_equal(inout.musts->arg.str, "cond");
    assert_string_equal(inout.typedefs->name, "tpdf");
    assert_string_equal(inout.groupings->name, "sub-grp");
    assert_string_equal(inout.child->name, "anyd");
    assert_int_equal(inout.child->nodetype, LYS_ANYDATA);
    assert_string_equal(inout.child->next->name, "anyx");
    assert_int_equal(inout.child->next->nodetype, LYS_ANYXML);
    assert_string_equal(inout.child->next->next->name, "choice");
    assert_int_equal(inout.child->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(inout.child->next->next->next->name, "subcont");
    assert_int_equal(inout.child->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(inout.child->next->next->next->next->name, "leaf");
    assert_int_equal(inout.child->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(inout.child->next->next->next->next->next->name, "llist");
    assert_int_equal(inout.child->next->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(inout.child->next->next->next->next->next->next->name, "list");
    assert_int_equal(inout.child->next->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(inout.child->next->next->next->next->next->next->next->name, "uses-name");
    assert_int_equal(inout.child->next->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_null(inout.child->next->next->next->next->next->next->next->next);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(inout.exts[0]), LY_STMT_OUTPUT);
    lysp_node_free(UTEST_LYCTX, (struct lysp_node *)&inout);
    memset(&inout, 0, sizeof inout);

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<input><leaf name=\"l\"><type name=\"empty\"/></leaf></input>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &inout_meta, NULL, NULL), LY_SUCCESS);
    lysp_node_free(UTEST_LYCTX, (struct lysp_node *)&inout);
    memset(&inout, 0, sizeof inout);

    data = ELEMENT_WRAPPER_START "<output><leaf name=\"l\"><type name=\"empty\"/></leaf></output>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &inout_meta, NULL, NULL), LY_SUCCESS);
    lysp_node_free(UTEST_LYCTX, (struct lysp_node *)&inout);
    memset(&inout, 0, sizeof inout);

    /* invalid combinations */
    data = ELEMENT_WRAPPER_START "<input name=\"test\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &inout_meta, NULL, NULL), LY_EVALID);
    lysp_node_free(UTEST_LYCTX, (struct lysp_node *)&inout);
    CHECK_LOG_CTX("Unexpected attribute \"name\" of \"input\" element.", "Line number 1.");
    memset(&inout, 0, sizeof inout);
}

static void
test_action_elem(void **state)
{
    const char *data;
    struct lysp_node_action *actions = NULL;
    struct tree_node_meta act_meta = {NULL, (struct lysp_node **)&actions};
    uint16_t flags;

    /* max subelems */
    YCTX->parsed_mod->version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
            "<action name=\"act\">\n"
            "    <description><text>desc</text></description>\n"
            "    <grouping name=\"grouping\"/>\n"
            "    <if-feature name=\"iff\"/>\n"
            "    <input><uses name=\"uses-name\"/></input>\n"
            "    <output><must condition=\"cond\"/><leaf name=\"l\"><type name=\"type\"/></leaf></output>\n"
            "    <reference><text>ref</text></reference>\n"
            "    <status value=\"deprecated\"/>\n"
            "    <typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>\n"
            EXT_SUBELEM
            "</action>"
            ELEMENT_WRAPPER_END;
    /* there must be parent for action */
    act_meta.parent = (void *)1;
    assert_int_equal(test_element_helper(state, data, &act_meta, NULL, NULL), LY_SUCCESS);
    act_meta.parent = NULL;
    flags = LYS_STATUS_DEPRC;
    CHECK_LYSP_ACTION(actions, "desc", 1, flags, 1, 1,\
            1, 0, 0, 0,\
            1, 0,\
            "act", LYS_ACTION, \
            1, 0, 0, 1,\
            1, 0,\
            1, "ref", 1);

    assert_string_equal(actions->iffeatures[0].str, "iff");
    assert_string_equal(actions->typedefs->name, "tpdf");
    assert_string_equal(actions->groupings->name, "grouping");
    assert_string_equal(actions->output.musts->arg.str, "cond");
    assert_string_equal(actions->input.child->name, "uses-name");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(actions->exts[0]), LY_STMT_ACTION);
    lysp_node_free(UTEST_LYCTX, (struct lysp_node *)actions);
    actions = NULL;

    YCTX->parsed_mod->version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
            "<rpc name=\"act\">\n"
            "    <description><text>desc</text></description>\n"
            "    <grouping name=\"grouping\"/>\n"
            "    <if-feature name=\"iff\"/>\n"
            "    <input><uses name=\"uses-name\"/></input>\n"
            "    <output><must condition=\"cond\"/><leaf name=\"l\"><type name=\"type\"/></leaf></output>\n"
            "    <reference><text>ref</text></reference>\n"
            "    <status value=\"deprecated\"/>\n"
            "    <typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>\n"
            EXT_SUBELEM
            "</rpc>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &act_meta, NULL, NULL), LY_SUCCESS);
    flags = LYS_STATUS_DEPRC;
    CHECK_LYSP_ACTION(actions, "desc", 1, flags, 1, 1,\
            1, 0, 0, 0,\
            1, 0,\
            "act", LYS_RPC, \
            1, 0, 0, 1,\
            1, 0,\
            0, "ref", 1);

    assert_string_equal(actions->iffeatures[0].str, "iff");
    assert_string_equal(actions->typedefs->name, "tpdf");
    assert_string_equal(actions->groupings->name, "grouping");
    assert_string_equal(actions->input.child->name, "uses-name");
    assert_string_equal(actions->output.musts->arg.str, "cond");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(actions->exts[0]), LY_STMT_RPC);
    lysp_node_free(UTEST_LYCTX, (struct lysp_node *)actions);
    actions = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<action name=\"act\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &act_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(actions->name, "act");
    lysp_node_free(UTEST_LYCTX, (struct lysp_node *)actions);
    actions = NULL;
}

static void
test_augment_elem(void **state)
{
    const char *data;
    struct lysp_node_augment *augments = NULL;
    struct tree_node_meta aug_meta = {NULL, (struct lysp_node **)&augments};

    YCTX->parsed_mod->version = LYS_VERSION_1_1;
    data = ELEMENT_WRAPPER_START
            "<augment target-node=\"target\">\n"
            "    <action name=\"action\"/>\n"
            "    <anydata name=\"anyd\"/>\n"
            "    <anyxml name=\"anyx\"/>\n"
            "    <case name=\"case\"/>\n"
            "    <choice name=\"choice\"/>\n"
            "    <container name=\"subcont\"/>\n"
            "    <description><text>desc</text></description>\n"
            "    <if-feature name=\"iff\"/>\n"
            "    <leaf name=\"leaf\"> <type name=\"type\"/> </leaf>\n"
            "    <leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>\n"
            "    <list name=\"list\"/>\n"
            "    <notification name=\"notif\"/>\n"
            "    <reference><text>ref</text></reference>\n"
            "    <status value=\"current\"/>\n"
            "    <uses name=\"uses\"/>\n"
            "    <when condition=\"when-cond\"/>\n"
            EXT_SUBELEM
            "</augment>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &aug_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(augments->nodeid, "target");
    assert_null(augments->parent);
    assert_int_equal(augments->nodetype, LYS_AUGMENT);
    assert_true(augments->flags & LYS_STATUS_CURR);
    assert_string_equal(augments->dsc, "desc");
    assert_string_equal(augments->ref, "ref");
    assert_string_equal(augments->when->cond, "when-cond");
    assert_string_equal(augments->iffeatures[0].str, "iff");
    assert_string_equal(augments->child->name, "anyd");
    assert_int_equal(augments->child->nodetype, LYS_ANYDATA);
    assert_string_equal(augments->child->next->name, "anyx");
    assert_int_equal(augments->child->next->nodetype, LYS_ANYXML);
    assert_string_equal(augments->child->next->next->name, "case");
    assert_int_equal(augments->child->next->next->nodetype, LYS_CASE);
    assert_string_equal(augments->child->next->next->next->name, "choice");
    assert_int_equal(augments->child->next->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(augments->child->next->next->next->next->name, "subcont");
    assert_int_equal(augments->child->next->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(augments->child->next->next->next->next->next->name, "leaf");
    assert_int_equal(augments->child->next->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(augments->child->next->next->next->next->next->next->name, "llist");
    assert_int_equal(augments->child->next->next->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(augments->child->next->next->next->next->next->next->next->name, "list");
    assert_int_equal(augments->child->next->next->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(augments->child->next->next->next->next->next->next->next->next->name, "uses");
    assert_int_equal(augments->child->next->next->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_null(augments->child->next->next->next->next->next->next->next->next->next);
    assert_string_equal(augments->actions->name, "action");
    assert_string_equal(augments->notifs->name, "notif");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(augments->exts[0]), LY_STMT_AUGMENT);
    lysp_node_free(YCTX->parsed_mod->mod->ctx, (struct lysp_node *)augments);
    augments = NULL;

    data = ELEMENT_WRAPPER_START "<augment target-node=\"target\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &aug_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(augments->nodeid, "target");
    lysp_node_free(YCTX->parsed_mod->mod->ctx, (struct lysp_node *)augments);
    augments = NULL;
}

static void
test_deviate_elem(void **state)
{
    const char *data;
    struct lysp_deviate *deviates = NULL;
    struct lysp_deviate_add *d_add;
    struct lysp_deviate_rpl *d_rpl;
    struct lysp_deviate_del *d_del;

    /* all valid arguments with min subelems */
    data = ELEMENT_WRAPPER_START "<deviate value=\"not-supported\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_SUCCESS);
    assert_int_equal(deviates->mod, LYS_DEV_NOT_SUPPORTED);
    lysp_deviate_free(UTEST_LYCTX, deviates);
    free(deviates);
    deviates = NULL;

    data = ELEMENT_WRAPPER_START "<deviate value=\"add\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_SUCCESS);
    assert_int_equal(deviates->mod, LYS_DEV_ADD);
    lysp_deviate_free(UTEST_LYCTX, deviates);
    free(deviates);
    deviates = NULL;

    data = ELEMENT_WRAPPER_START "<deviate value=\"replace\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_SUCCESS);
    assert_int_equal(deviates->mod, LYS_DEV_REPLACE);
    lysp_deviate_free(UTEST_LYCTX, deviates);
    free(deviates);
    deviates = NULL;

    data = ELEMENT_WRAPPER_START "<deviate value=\"delete\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_SUCCESS);
    assert_int_equal(deviates->mod, LYS_DEV_DELETE);
    lysp_deviate_free(UTEST_LYCTX, deviates);
    free(deviates);
    deviates = NULL;

    /* max subelems and valid arguments */
    data = ELEMENT_WRAPPER_START
            "<deviate value=\"not-supported\">"
            EXT_SUBELEM
            "</deviate>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_SUCCESS);
    assert_int_equal(deviates->mod, LYS_DEV_NOT_SUPPORTED);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(deviates->exts[0]), LY_STMT_DEVIATE);
    lysp_deviate_free(UTEST_LYCTX, deviates);
    free(deviates);
    deviates = NULL;

    data = ELEMENT_WRAPPER_START
            "<deviate value=\"add\">\n"
            "    <units name=\"units\"/>\n"
            "    <must condition=\"cond\"/>\n"
            "    <unique tag=\"utag\"/>\n"
            "    <default value=\"def\"/>\n"
            "    <config value=\"true\"/>\n"
            "    <mandatory value=\"true\"/>\n"
            "    <min-elements value=\"5\"/>\n"
            "    <max-elements value=\"15\"/>\n"
            EXT_SUBELEM
            "</deviate>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_SUCCESS);
    d_add = (struct lysp_deviate_add *)deviates;
    assert_int_equal(d_add->mod, LYS_DEV_ADD);
    assert_null(d_add->next);
    assert_string_equal(d_add->units, "units");
    assert_string_equal(d_add->musts->arg.str, "cond");
    assert_string_equal(d_add->uniques[0].str, "utag");
    assert_string_equal(d_add->dflts[0].str, "def");
    assert_true((d_add->flags & LYS_MAND_TRUE) && (d_add->flags & LYS_CONFIG_W));
    assert_int_equal(d_add->min, 5);
    assert_int_equal(d_add->max, 15);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(deviates->exts[0]), LY_STMT_DEVIATE);
    lysp_deviate_free(UTEST_LYCTX, deviates);
    free(deviates);
    deviates = NULL;

    data = ELEMENT_WRAPPER_START
            "<deviate value=\"replace\">\n"
            "    <type name=\"newtype\"/>\n"
            "    <units name=\"uni\"/>\n"
            "    <default value=\"def\"/>\n"
            "    <config value=\"true\"/>\n"
            "    <mandatory value=\"true\"/>\n"
            "    <min-elements value=\"5\"/>\n"
            "    <max-elements value=\"15\"/>\n"
            EXT_SUBELEM
            "</deviate>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_SUCCESS);
    d_rpl = (struct lysp_deviate_rpl *)deviates;
    assert_int_equal(d_rpl->mod, LYS_DEV_REPLACE);
    assert_null(d_rpl->next);
    assert_string_equal(d_rpl->type->name, "newtype");
    assert_string_equal(d_rpl->units, "uni");
    assert_string_equal(d_rpl->dflt.str, "def");
    assert_true((d_rpl->flags & LYS_MAND_TRUE) && (d_rpl->flags & LYS_CONFIG_W));
    assert_int_equal(d_rpl->min, 5);
    assert_int_equal(d_rpl->max, 15);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(deviates->exts[0]), LY_STMT_DEVIATE);
    lysp_deviate_free(UTEST_LYCTX, deviates);
    free(deviates);
    deviates = NULL;

    data = ELEMENT_WRAPPER_START
            "<deviate value=\"delete\">\n"
            "    <units name=\"u\"/>\n"
            "    <must condition=\"c\"/>\n"
            "    <unique tag=\"tag\"/>\n"
            "    <default value=\"default\"/>\n"
            EXT_SUBELEM
            "</deviate>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_SUCCESS);
    d_del = (struct lysp_deviate_del *)deviates;
    assert_int_equal(d_del->mod, LYS_DEV_DELETE);
    assert_null(d_del->next);
    assert_string_equal(d_del->units, "u");
    assert_string_equal(d_del->musts->arg.str, "c");
    assert_string_equal(d_del->uniques[0].str, "tag");
    assert_string_equal(d_del->dflts[0].str, "default");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(deviates->exts[0]), LY_STMT_DEVIATE);
    lysp_deviate_free(UTEST_LYCTX, deviates);
    free(deviates);
    deviates = NULL;

    /* invalid arguments */
    data = ELEMENT_WRAPPER_START "<deviate value=\"\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"\" of \"value\" attribute in \"deviate\" element. "
            "Valid values are \"not-supported\", \"add\", \"replace\" and \"delete\".", "Line number 1.");
    deviates = NULL;

    data = ELEMENT_WRAPPER_START "<deviate value=\"invalid\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"invalid\" of \"value\" attribute in \"deviate\" element. "
            "Valid values are \"not-supported\", \"add\", \"replace\" and \"delete\".", "Line number 1.");
    deviates = NULL;

    data = ELEMENT_WRAPPER_START "<deviate value=\"ad\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"ad\" of \"value\" attribute in \"deviate\" element. "
            "Valid values are \"not-supported\", \"add\", \"replace\" and \"delete\".", "Line number 1.");
    deviates = NULL;

    data = ELEMENT_WRAPPER_START "<deviate value=\"adds\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"adds\" of \"value\" attribute in \"deviate\" element. "
            "Valid values are \"not-supported\", \"add\", \"replace\" and \"delete\".", "Line number 1.");
    deviates = NULL;

    data = ELEMENT_WRAPPER_START
            "<deviate value=\"not-supported\">\n"
            "    <must condition=\"c\"/>\n"
            "</deviate>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Deviate of this type doesn't allow \"must\" as it's sub-element.", "Line number 2.");
}

static void
test_deviation_elem(void **state)
{
    const char *data;
    struct lysp_deviation *deviations = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START
            "<deviation target-node=\"target\">\n"
            "    <deviate value=\"not-supported\"/>\n"
            "</deviation>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviations, NULL, NULL), LY_SUCCESS);
    assert_string_equal(deviations->nodeid, "target");
    assert_int_equal(deviations->deviates->mod, LYS_DEV_NOT_SUPPORTED);
    FREE_ARRAY(UTEST_LYCTX, deviations, lysp_deviation_free);
    deviations = NULL;

    /* max subelems */
    data = ELEMENT_WRAPPER_START
            "<deviation target-node=\"target\">\n"
            "    <reference><text>ref</text></reference>\n"
            "    <description><text>desc</text></description>\n"
            "    <deviate value=\"add\"/>\n"
            EXT_SUBELEM
            "</deviation>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviations, NULL, NULL), LY_SUCCESS);
    assert_string_equal(deviations->nodeid, "target");
    assert_int_equal(deviations->deviates->mod, LYS_DEV_ADD);
    assert_string_equal(deviations->ref, "ref");
    assert_string_equal(deviations->dsc, "desc");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(deviations->exts[0]), LY_STMT_DEVIATION);
    FREE_ARRAY(UTEST_LYCTX, deviations, lysp_deviation_free);
    deviations = NULL;

    /* invalid */
    data = ELEMENT_WRAPPER_START "<deviation target-node=\"target\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviations, NULL, NULL), LY_EVALID);
    FREE_ARRAY(UTEST_LYCTX, deviations, lysp_deviation_free);
    deviations = NULL;
    CHECK_LOG_CTX("Missing mandatory sub-element \"deviate\" of \"deviation\" element.", "Line number 1.");
    /* TODO */}

static struct lysp_module *
mod_renew(struct lys_yin_parser_ctx *ctx)
{
    struct ly_ctx *ly_ctx = ctx->parsed_mod->mod->ctx;

    lys_module_free(ctx->parsed_mod->mod);
    ctx->parsed_mod = calloc(1, sizeof *ctx->parsed_mod);
    ctx->parsed_mod->mod = calloc(1, sizeof *ctx->parsed_mod->mod);
    ctx->parsed_mod->mod->parsed = ctx->parsed_mod;
    ctx->parsed_mod->mod->ctx = ly_ctx;

    return ctx->parsed_mod;
}

static void
test_module_elem(void **state)
{
    const char *data;
    struct lysp_module *lysp_mod = mod_renew(YCTX);

    /* max subelems */
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\"mod\">\n"
            "    <yang-version value=\"1.1\"/>\n"
            "    <namespace uri=\"ns\"/>\n"
            "    <prefix value=\"pref\"/>\n"
            "    <include module=\"b-mod\"/>\n"
            "    <import module=\"a-mod\"><prefix value=\"imp-pref\"/></import>\n"
            "    <organization><text>org</text></organization>\n"
            "    <contact><text>contact</text></contact>\n"
            "    <description><text>desc</text></description>\n"
            "    <reference><text>ref</text></reference>\n"
            "    <revision date=\"2019-02-02\"/>\n"
            "    <anydata name=\"anyd\"/>\n"
            "    <anyxml name=\"anyx\"/>\n"
            "    <choice name=\"choice\"/>\n"
            "    <container name=\"cont\"/>\n"
            "    <leaf name=\"leaf\"> <type name=\"type\"/> </leaf>\n"
            "    <leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>\n"
            "    <list name=\"sub-list\"/>\n"
            "    <uses name=\"uses-name\"/>\n"
            "    <augment target-node=\"target\"/>\n"
            "    <deviation target-node=\"target\">\n"
            "        <deviate value=\"not-supported\"/>\n"
            "    </deviation>\n"
            "    <extension name=\"ext\"/>\n"
            "    <feature name=\"feature\"/>\n"
            "    <grouping name=\"grp\"/>\n"
            "    <identity name=\"ident-name\"/>\n"
            "    <notification name=\"notf\"/>\n"
            "    <rpc name=\"rpc-name\"/>\n"
            "    <typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>\n"
            EXT_SUBELEM "\n"
            "</module>\n";
    assert_int_equal(ly_in_new_memory(data, &UTEST_IN), LY_SUCCESS);
    assert_int_equal(lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx), LY_SUCCESS);

    assert_int_equal(yin_parse_mod(YCTX, lysp_mod), LY_SUCCESS);
    assert_string_equal(lysp_mod->mod->name, "mod");
    assert_string_equal(lysp_mod->revs, "2019-02-02");
    assert_string_equal(lysp_mod->mod->ns, "ns");
    assert_string_equal(lysp_mod->mod->prefix, "pref");
    assert_null(lysp_mod->mod->filepath);
    assert_string_equal(lysp_mod->mod->org, "org");
    assert_string_equal(lysp_mod->mod->contact, "contact");
    assert_string_equal(lysp_mod->mod->dsc, "desc");
    assert_string_equal(lysp_mod->mod->ref, "ref");
    assert_int_equal(lysp_mod->version, LYS_VERSION_1_1);
    CHECK_LYSP_IMPORT(lysp_mod->imports, NULL, 0, "a-mod",
            "imp-pref", NULL, "");
    assert_string_equal(lysp_mod->includes->name, "b-mod");
    assert_string_equal(lysp_mod->extensions->name, "ext");
    assert_string_equal(lysp_mod->features->name, "feature");
    assert_string_equal(lysp_mod->identities->name, "ident-name");
    assert_string_equal(lysp_mod->typedefs->name, "tpdf");
    assert_string_equal(lysp_mod->groupings->name, "grp");
    assert_string_equal(lysp_mod->data->name, "anyd");
    assert_int_equal(lysp_mod->data->nodetype, LYS_ANYDATA);
    assert_string_equal(lysp_mod->data->next->name, "anyx");
    assert_int_equal(lysp_mod->data->next->nodetype, LYS_ANYXML);
    assert_string_equal(lysp_mod->data->next->next->name, "choice");
    assert_int_equal(lysp_mod->data->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(lysp_mod->data->next->next->next->name, "cont");
    assert_int_equal(lysp_mod->data->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(lysp_mod->data->next->next->next->next->name, "leaf");
    assert_int_equal(lysp_mod->data->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(lysp_mod->data->next->next->next->next->next->name, "llist");
    assert_int_equal(lysp_mod->data->next->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(lysp_mod->data->next->next->next->next->next->next->name, "sub-list");
    assert_int_equal(lysp_mod->data->next->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(lysp_mod->data->next->next->next->next->next->next->next->name, "uses-name");
    assert_int_equal(lysp_mod->data->next->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_null(lysp_mod->data->next->next->next->next->next->next->next->next);
    assert_string_equal(lysp_mod->augments->nodeid, "target");
    assert_string_equal(lysp_mod->rpcs->name, "rpc-name");
    assert_string_equal(lysp_mod->notifs->name, "notf");
    assert_string_equal(lysp_mod->deviations->nodeid, "target");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(lysp_mod->exts[0]), LY_STMT_MODULE);

    /* min subelems */
    ly_in_free(UTEST_IN, 0);
    lyxml_ctx_free(YCTX->xmlctx);
    lysp_mod = mod_renew(YCTX);
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\"mod\">\n"
            "    <namespace uri=\"ns\"/>\n"
            "    <prefix value=\"pref\"/>\n"
            "    <yang-version value=\"1.1\"/>\n"
            "</module>";
    assert_int_equal(ly_in_new_memory(data, &UTEST_IN), LY_SUCCESS);
    assert_int_equal(lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx), LY_SUCCESS);
    assert_int_equal(yin_parse_mod(YCTX, lysp_mod), LY_SUCCESS);
    assert_string_equal(lysp_mod->mod->name, "mod");

    /* incorrect subelem order */
    ly_in_free(UTEST_IN, 0);
    lyxml_ctx_free(YCTX->xmlctx);
    lysp_mod = mod_renew(YCTX);
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\"mod\">\n"
            "    <feature name=\"feature\"/>\n"
            "    <namespace uri=\"ns\"/>\n"
            "    <prefix value=\"pref\"/>\n"
            "    <yang-version value=\"1.1\"/>\n"
            "</module>";
    assert_int_equal(ly_in_new_memory(data, &UTEST_IN), LY_SUCCESS);
    assert_int_equal(lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx), LY_SUCCESS);
    assert_int_equal(yin_parse_mod(YCTX, lysp_mod), LY_EVALID);
    CHECK_LOG_CTX("Invalid order of module\'s sub-elements \"namespace\" can\'t appear after \"feature\".", "Line number 3.");
}

static struct lysp_submodule *
submod_renew(struct lys_yin_parser_ctx *ctx, const char *belongs_to)
{
    struct ly_ctx *ly_ctx = ctx->parsed_mod->mod->ctx;

    lys_module_free(ctx->parsed_mod->mod);
    ctx->parsed_mod = calloc(1, sizeof(struct lysp_submodule));
    ctx->parsed_mod->mod = calloc(1, sizeof *ctx->parsed_mod->mod);
    lydict_insert(ly_ctx, belongs_to, 0, &ctx->parsed_mod->mod->name);
    ctx->parsed_mod->mod->parsed = ctx->parsed_mod;
    ctx->parsed_mod->mod->ctx = ly_ctx;

    return (struct lysp_submodule *)ctx->parsed_mod;
}

static void
test_submodule_elem(void **state)
{
    const char *data;
    struct lysp_submodule *lysp_submod = submod_renew(YCTX, "module-name");

    /* max subelements */
    data = "<submodule xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\"mod\">\n"
            "    <yang-version value=\"1.1\"/>\n"
            "    <belongs-to module=\"module-name\">\n"
            "        <prefix value=\"pref\"/>\n"
            "    </belongs-to>\n"
            "    <include module=\"b-mod\"/>\n"
            "    <import module=\"a-mod\"><prefix value=\"imp-pref\"/></import>\n"
            "    <organization><text>org</text></organization>\n"
            "    <contact><text>contact</text></contact>\n"
            "    <description><text>desc</text></description>\n"
            "    <reference><text>ref</text></reference>\n"
            "    <revision date=\"2019-02-02\"/>\n"
            "    <anydata name=\"anyd\"/>\n"
            "    <anyxml name=\"anyx\"/>\n"
            "    <choice name=\"choice\"/>\n"
            "    <container name=\"cont\"/>\n"
            "    <leaf name=\"leaf\"> <type name=\"type\"/> </leaf>\n"
            "    <leaf-list name=\"llist\"> <type name=\"type\"/> </leaf-list>\n"
            "    <list name=\"sub-list\"/>\n"
            "    <uses name=\"uses-name\"/>\n"
            "    <augment target-node=\"target\"/>\n"
            "    <deviation target-node=\"target\">\n"
            "        <deviate value=\"not-supported\"/>\n"
            "    </deviation>\n"
            "    <extension name=\"ext\"/>\n"
            "    <feature name=\"feature\"/>\n"
            "    <grouping name=\"grp\"/>\n"
            "    <identity name=\"ident-name\"/>\n"
            "    <notification name=\"notf\"/>\n"
            "    <rpc name=\"rpc-name\"/>\n"
            "    <typedef name=\"tpdf\"> <type name=\"type\"/> </typedef>\n"
            EXT_SUBELEM "\n"
            "</submodule>\n";
    assert_int_equal(ly_in_new_memory(data, &UTEST_IN), LY_SUCCESS);
    assert_int_equal(lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx), LY_SUCCESS);

    assert_int_equal(yin_parse_submod(YCTX, lysp_submod), LY_SUCCESS);
    assert_string_equal(lysp_submod->name, "mod");
    assert_string_equal(lysp_submod->revs, "2019-02-02");
    assert_string_equal(lysp_submod->prefix, "pref");
    assert_null(lysp_submod->filepath);
    assert_string_equal(lysp_submod->org, "org");
    assert_string_equal(lysp_submod->contact, "contact");
    assert_string_equal(lysp_submod->dsc, "desc");
    assert_string_equal(lysp_submod->ref, "ref");
    assert_int_equal(lysp_submod->version, LYS_VERSION_1_1);
    CHECK_LYSP_IMPORT(lysp_submod->imports, NULL, 0, "a-mod",
            "imp-pref", NULL, "");
    assert_string_equal(lysp_submod->includes->name, "b-mod");
    assert_string_equal(lysp_submod->extensions->name, "ext");
    assert_string_equal(lysp_submod->features->name, "feature");
    assert_string_equal(lysp_submod->identities->name, "ident-name");
    assert_string_equal(lysp_submod->typedefs->name, "tpdf");
    assert_string_equal(lysp_submod->groupings->name, "grp");
    assert_string_equal(lysp_submod->data->name, "anyd");
    assert_int_equal(lysp_submod->data->nodetype, LYS_ANYDATA);
    assert_string_equal(lysp_submod->data->next->name, "anyx");
    assert_int_equal(lysp_submod->data->next->nodetype, LYS_ANYXML);
    assert_string_equal(lysp_submod->data->next->next->name, "choice");
    assert_int_equal(lysp_submod->data->next->next->nodetype, LYS_CHOICE);
    assert_string_equal(lysp_submod->data->next->next->next->name, "cont");
    assert_int_equal(lysp_submod->data->next->next->next->nodetype, LYS_CONTAINER);
    assert_string_equal(lysp_submod->data->next->next->next->next->name, "leaf");
    assert_int_equal(lysp_submod->data->next->next->next->next->nodetype, LYS_LEAF);
    assert_string_equal(lysp_submod->data->next->next->next->next->next->name, "llist");
    assert_int_equal(lysp_submod->data->next->next->next->next->next->nodetype, LYS_LEAFLIST);
    assert_string_equal(lysp_submod->data->next->next->next->next->next->next->name, "sub-list");
    assert_int_equal(lysp_submod->data->next->next->next->next->next->next->nodetype, LYS_LIST);
    assert_string_equal(lysp_submod->data->next->next->next->next->next->next->next->name, "uses-name");
    assert_int_equal(lysp_submod->data->next->next->next->next->next->next->next->nodetype, LYS_USES);
    assert_null(lysp_submod->data->next->next->next->next->next->next->next->next);
    assert_string_equal(lysp_submod->augments->nodeid, "target");
    assert_string_equal(lysp_submod->rpcs->name, "rpc-name");
    assert_string_equal(lysp_submod->notifs->name, "notf");
    assert_string_equal(lysp_submod->deviations->nodeid, "target");
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(lysp_submod->exts[0]), LY_STMT_SUBMODULE);

    /* min subelemnts */
    ly_in_free(UTEST_IN, 0);
    lyxml_ctx_free(YCTX->xmlctx);
    lysp_submod = submod_renew(YCTX, "module-name");
    data = "<submodule xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\"submod\">\n"
            "    <yang-version value=\"1\"/>\n"
            "    <belongs-to module=\"module-name\"><prefix value=\"pref\"/></belongs-to>\n"
            "</submodule>";
    assert_int_equal(ly_in_new_memory(data, &UTEST_IN), LY_SUCCESS);
    assert_int_equal(lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx), LY_SUCCESS);
    assert_int_equal(yin_parse_submod(YCTX, lysp_submod), LY_SUCCESS);
    assert_string_equal(lysp_submod->prefix, "pref");
    assert_int_equal(lysp_submod->version, LYS_VERSION_1_0);

    /* incorrect subelem order */
    ly_in_free(UTEST_IN, 0);
    lyxml_ctx_free(YCTX->xmlctx);
    lysp_submod = submod_renew(YCTX, "module-name");
    data = "<submodule xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\"submod\">\n"
            "    <yang-version value=\"1\"/>\n"
            "    <reference><text>ref</text></reference>\n"
            "    <belongs-to module=\"module-name\"><prefix value=\"pref\"/></belongs-to>\n"
            "</submodule>";
    assert_int_equal(ly_in_new_memory(data, &UTEST_IN), LY_SUCCESS);
    assert_int_equal(lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx), LY_SUCCESS);
    assert_int_equal(yin_parse_submod(YCTX, lysp_submod), LY_EVALID);
    CHECK_LOG_CTX("Invalid order of submodule's sub-elements \"belongs-to\" can't appear after \"reference\".", "Line number 4.");
}

static void
test_yin_parse_module(void **state)
{
    const char *data;
    struct lys_module *mod;
    struct lys_yin_parser_ctx *yin_ctx = NULL;
    struct ly_in *in = NULL;
    struct lys_glob_unres unres = {0};

    mod = calloc(1, sizeof *mod);
    mod->ctx = UTEST_LYCTX;
    data = "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\" name=\"a\"> \n"
            "    <yang-version value=\"1.1\"/>\n"
            "    <namespace uri=\"urn:tests:extensions:metadata:a\"/>\n"
            "    <prefix value=\"a\"/>\n"
            "    <import module=\"ietf-yang-metadata\">\n"
            "        <prefix value=\"md\"/>\n"
            "    </import>\n"
            "    <feature name=\"f\"/>\n"
            "    <md:annotation name=\"x\">\n"
            "        <description>\n"
            "            <text>test</text>\n"
            "        </description>\n"
            "        <reference>\n"
            "            <text>test</text>\n"
            "        </reference>\n"
            "        <if-feature name=\"f\"/>\n"
            "        <status value=\"current\"/>\n"
            "        <type name=\"uint8\"/>\n"
            "        <units name=\"meters\"/>\n"
            "    </md:annotation>\n"
            "</module>\n";
    assert_int_equal(ly_in_new_memory(data, &in), LY_SUCCESS);
    assert_int_equal(yin_parse_module(&yin_ctx, in, mod, &unres), LY_SUCCESS);
    assert_null(mod->parsed->exts->child->next->child);
    assert_string_equal(mod->parsed->exts->child->next->arg, "test");
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    lys_module_free(mod);
    yin_parser_ctx_free(yin_ctx);
    ly_in_free(in, 0);
    mod = NULL;
    yin_ctx = NULL;

    mod = calloc(1, sizeof *mod);
    mod->ctx = UTEST_LYCTX;
    data = "<module name=\"example-foo\""
            "    xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\""
            "    xmlns:foo=\"urn:example:foo\""
            "    xmlns:myext=\"urn:example:extensions\">\n"

            "    <yang-version value=\"1\"/>\n"

            "    <namespace uri=\"urn:example:foo\"/>\n"
            "    <prefix value=\"foo\"/>\n"

            "    <import module=\"example-extensions\">\n"
            "        <prefix value=\"myext\"/>\n"
            "    </import>\n"

            "    <list name=\"interface\">\n"
            "        <key value=\"name\"/>\n"
            "        <leaf name=\"name\">\n"
            "            <type name=\"string\"/>\n"
            "        </leaf>\n"
            "        <leaf name=\"mtu\">\n"
            "            <type name=\"uint32\"/>\n"
            "            <description>\n"
            "                <text>The MTU of the interface.</text>\n"
            "            </description>\n"
            "            <myext:c-define name=\"MY_MTU\"/>\n"
            "        </leaf>\n"
            "    </list>\n"
            "</module>\n";
    assert_int_equal(ly_in_new_memory(data, &in), LY_SUCCESS);
    assert_int_equal(yin_parse_module(&yin_ctx, in, mod, &unres), LY_SUCCESS);
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    lys_module_free(mod);
    yin_parser_ctx_free(yin_ctx);
    ly_in_free(in, 0);
    mod = NULL;
    yin_ctx = NULL;

    mod = calloc(1, sizeof *mod);
    mod->ctx = UTEST_LYCTX;
    data = "<module name=\"example-foo\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">\n"
            "    <yang-version value=\"1\"/>\n"
            "    <namespace uri=\"urn:example:foo\"/>\n"
            "    <prefix value=\"foo\"/>\n"
            "</module>\n";
    assert_int_equal(ly_in_new_memory(data, &in), LY_SUCCESS);
    assert_int_equal(yin_parse_module(&yin_ctx, in, mod, &unres), LY_SUCCESS);
    lys_compile_unres_glob_erase(UTEST_LYCTX, &unres);
    lys_module_free(mod);
    yin_parser_ctx_free(yin_ctx);
    ly_in_free(in, 0);
    mod = NULL;
    yin_ctx = NULL;

    mod = calloc(1, sizeof *mod);
    mod->ctx = UTEST_LYCTX;
    data = "<submodule name=\"example-foo\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "</submodule>\n";
    assert_int_equal(ly_in_new_memory(data, &in), LY_SUCCESS);
    assert_int_equal(yin_parse_module(&yin_ctx, in, mod, &unres), LY_EINVAL);
    CHECK_LOG_CTX("Input data contains submodule which cannot be parsed directly without its main module.", NULL);
    lys_module_free(mod);
    yin_parser_ctx_free(yin_ctx);
    ly_in_free(in, 0);

    mod = calloc(1, sizeof *mod);
    mod->ctx = UTEST_LYCTX;
    data = "<module name=\"example-foo\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">\n"
            "    <yang-version value=\"1\"/>\n"
            "    <namespace uri=\"urn:example:foo\"/>\n"
            "    <prefix value=\"foo\"/>\n"
            "</module>\n"
            "<module>";
    assert_int_equal(ly_in_new_memory(data, &in), LY_SUCCESS);
    assert_int_equal(yin_parse_module(&yin_ctx, in, mod, &unres), LY_EVALID);
    CHECK_LOG_CTX("Trailing garbage \"<module>\" after module, expected end-of-input.", "Line number 6.");
    lys_module_free(mod);
    yin_parser_ctx_free(yin_ctx);
    ly_in_free(in, 0);
    mod = NULL;
    yin_ctx = NULL;
}

static void
test_yin_parse_submodule(void **state)
{
    const char *data;
    struct lys_yin_parser_ctx *yin_ctx = NULL;
    struct lysp_submodule *submod = NULL;
    struct ly_in *in;

    lydict_insert(UTEST_LYCTX, "a", 0, &YCTX->parsed_mod->mod->name);

    data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<submodule name=\"asub\""
            "    xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\""
            "    xmlns:a=\"urn:a\">\n"
            "    <yang-version value=\"1\"/>\n"
            "    <belongs-to module=\"a\">\n"
            "        <prefix value=\"a_pref\"/>\n"
            "    </belongs-to>\n"
            "    <include module=\"atop\"/>\n"
            "    <feature name=\"fox\"/>\n"
            "    <notification name=\"bar-notif\">\n"
            "        <if-feature name=\"bar\"/>\n"
            "    </notification>\n"
            "    <notification name=\"fox-notif\">\n"
            "        <if-feature name=\"fox\"/>\n"
            "    </notification>\n"
            "    <augment target-node=\"/a_pref:top\">\n"
            "        <if-feature name=\"bar\"/>\n"
            "        <container name=\"bar-sub\"/>\n"
            "    </augment>\n"
            "    <augment target-node=\"/top\">\n"
            "        <container name=\"bar-sub2\"/>\n"
            "    </augment>\n"
            "</submodule>";
    assert_int_equal(ly_in_new_memory(data, &in), LY_SUCCESS);
    assert_int_equal(yin_parse_submodule(&yin_ctx, UTEST_LYCTX, (struct lys_parser_ctx *)YCTX, in, &submod), LY_SUCCESS);
    lysp_module_free((struct lysp_module *)submod);
    yin_parser_ctx_free(yin_ctx);
    ly_in_free(in, 0);
    yin_ctx = NULL;
    submod = NULL;

    data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<submodule name=\"asub\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">\n"
            "    <yang-version value=\"1\"/>\n"
            "    <belongs-to module=\"a\">\n"
            "        <prefix value=\"a_pref\"/>\n"
            "    </belongs-to>\n"
            "</submodule>";
    assert_int_equal(ly_in_new_memory(data, &in), LY_SUCCESS);
    assert_int_equal(yin_parse_submodule(&yin_ctx, UTEST_LYCTX, (struct lys_parser_ctx *)YCTX, in, &submod), LY_SUCCESS);
    lysp_module_free((struct lysp_module *)submod);
    yin_parser_ctx_free(yin_ctx);
    ly_in_free(in, 0);
    yin_ctx = NULL;
    submod = NULL;

    data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"inval\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "</module>";
    assert_int_equal(ly_in_new_memory(data, &in), LY_SUCCESS);
    assert_int_equal(yin_parse_submodule(&yin_ctx, UTEST_LYCTX, (struct lys_parser_ctx *)YCTX, in, &submod), LY_EINVAL);
    CHECK_LOG_CTX("Input data contains module in situation when a submodule is expected.", NULL);
    lysp_module_free((struct lysp_module *)submod);
    yin_parser_ctx_free(yin_ctx);
    ly_in_free(in, 0);
    yin_ctx = NULL;
    submod = NULL;

    data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<submodule name=\"asub\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">\n"
            "    <yang-version value=\"1\"/>\n"
            "    <belongs-to module=\"a\">\n"
            "        <prefix value=\"a_pref\"/>\n"
            "    </belongs-to>\n"
            "</submodule>\n"
            "<submodule name=\"asub\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">\n"
            "    <yang-version value=\"1\"/>\n"
            "    <belongs-to module=\"a\">\n"
            "        <prefix value=\"a_pref\"/>\n"
            "    </belongs-to>\n"
            "</submodule>";
    assert_int_equal(ly_in_new_memory(data, &in), LY_SUCCESS);
    assert_int_equal(yin_parse_submodule(&yin_ctx, UTEST_LYCTX, (struct lys_parser_ctx *)YCTX, in, &submod), LY_EVALID);
    CHECK_LOG_CTX("Trailing garbage \"<submodule name...\" after submodule, expected end-of-input.", "Line number 8.");
    lysp_module_free((struct lysp_module *)submod);
    yin_parser_ctx_free(yin_ctx);
    ly_in_free(in, 0);
    yin_ctx = NULL;
    submod = NULL;
}

int
main(void)
{

    const struct CMUnitTest tests[] = {
        UTEST(test_yin_match_keyword, setup, teardown),
        UTEST(test_yin_parse_element_generic, setup, teardown),
        UTEST(test_yin_parse_extension_instance, setup, teardown),
        UTEST(test_yin_parse_content, setup, teardown),
        UTEST(test_validate_value, setup, teardown),

        UTEST(test_yin_match_argument_name),
        cmocka_unit_test_setup_teardown(test_enum_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_bit_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_meta_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_import_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_status_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_ext_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_yin_element_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_yangversion_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_mandatory_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_argument_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_base_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_belongsto_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_config_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_default_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_err_app_tag_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_err_msg_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_fracdigits_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_iffeature_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_length_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_modifier_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_namespace_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_pattern_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_value_position_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_prefix_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_range_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_reqinstance_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_revision_date_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_unique_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_units_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_when_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_yin_text_value_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_type_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_max_elems_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_min_elems_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_ordby_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_any_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_leaf_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_leaf_list_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_presence_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_key_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_typedef_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_refine_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_uses_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_revision_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_include_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_list_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_notification_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_grouping_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_container_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_case_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_choice_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_inout_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_action_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_augment_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_deviate_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_deviation_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_module_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_submodule_elem, setup, teardown),

        cmocka_unit_test_setup_teardown(test_yin_parse_module, setup, teardown),
        cmocka_unit_test_setup_teardown(test_yin_parse_submodule, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
