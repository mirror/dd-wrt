/**
 * @file test_yin.c
 * @author David Sedlák <xsedla1d@stud.fit.vutbr.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for YIN parser and printer
 *
 * Copyright (c) 2015 - 2022 CESNET, z.s.p.o.
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

#include "in.h"
#include "ly_common.h"
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

LY_ERR yin_parse_content(struct lysp_yin_ctx *ctx, struct yin_subelement *subelem_info, size_t subelem_info_size,
        const void *parent, enum ly_stmt parent_stmt, const char **text_content, struct lysp_ext_instance **exts);
LY_ERR yin_validate_value(struct lysp_yin_ctx *ctx, enum yang_arg val_type);
enum ly_stmt yin_match_keyword(struct lysp_yin_ctx *ctx, const char *name, size_t name_len,
        const char *prefix, size_t prefix_len, enum ly_stmt parrent);

LY_ERR yin_parse_extension_instance(struct lysp_yin_ctx *ctx, const void *parent, enum ly_stmt parent_stmt,
        LY_ARRAY_COUNT_TYPE parent_stmt_index, struct lysp_ext_instance **exts);
LY_ERR yin_parse_element_generic(struct lysp_yin_ctx *ctx, enum ly_stmt parent, struct lysp_stmt **element);
LY_ERR yin_parse_mod(struct lysp_yin_ctx *ctx, struct lysp_module *mod);
LY_ERR yin_parse_submod(struct lysp_yin_ctx *ctx, struct lysp_submodule *submod);

/* wrapping element used for mocking has nothing to do with real module structure */
#define ELEMENT_WRAPPER_START "<status xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
#define ELEMENT_WRAPPER_END "</status>"

#define TEST_1_CHECK_LYSP_EXT_INSTANCE(NODE, INSUBSTMT)\
    CHECK_LYSP_EXT_INSTANCE((NODE), NULL, 1, INSUBSTMT, 0, "myext:c-define", LY_VALUE_XML)

struct lysp_yin_ctx *YCTX;
struct lysf_ctx fctx;

static int
setup_ctx(void **state)
{
    struct lysp_module *pmod;

    /* allocate parser context */
    YCTX = calloc(1, sizeof(*YCTX));
    YCTX->main_ctx = (struct lysp_ctx *)YCTX;
    YCTX->format = LYS_IN_YIN;
    ly_set_new(&YCTX->parsed_mods);

    /* allocate new parsed module */
    pmod = calloc(1, sizeof *pmod);
    ly_set_add(YCTX->parsed_mods, pmod, 1, NULL);

    /* allocate new module */
    pmod->mod = calloc(1, sizeof *pmod->mod);
    pmod->mod->ctx = UTEST_LYCTX;
    pmod->mod->parsed = pmod;

    return 0;
}

static int
setup(void **state)
{
    UTEST_SETUP;

    setup_ctx(state);

    fctx.ctx = UTEST_LYCTX;
    fctx.mod = PARSER_CUR_PMOD(YCTX)->mod;

    return 0;
}

static int
teardown_ctx(void **UNUSED(state))
{
    lys_module_free(&fctx, PARSER_CUR_PMOD(YCTX)->mod, 0);
    lysp_yin_ctx_free(YCTX);
    YCTX = NULL;

    return 0;
}

static int
teardown(void **state)
{
    teardown_ctx(state);

    lysf_ctx_erase(&fctx);

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
    const char *value;

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

    ret = yin_parse_content(YCTX, subelems2, 2, NULL, LY_STMT_STATUS, NULL, &exts);
    assert_int_equal(ret, LY_EVALID);
    CHECK_LOG_CTX("Redefinition of \"text\" sub-element in \"status\" element.", NULL, 1);
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

    ret = yin_parse_content(YCTX, subelems3, 2, NULL, LY_STMT_STATUS, NULL, &exts);
    assert_int_equal(ret, LY_EVALID);
    CHECK_LOG_CTX("Sub-element \"text\" of \"status\" element must be defined as it's first sub-element.", NULL, 1);
    lydict_remove(UTEST_LYCTX, prefix_value);
    RESET_STATE;

    /* test mandatory subelem */
    data = ELEMENT_WRAPPER_START ELEMENT_WRAPPER_END;
    struct yin_subelement subelems4[1] = {{LY_STMT_PREFIX, &prefix_value, YIN_SUBELEM_MANDATORY | YIN_SUBELEM_UNIQUE}};

    ly_in_new_memory(data, &UTEST_IN);
    lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx);
    lyxml_ctx_next(YCTX->xmlctx);

    ret = yin_parse_content(YCTX, subelems4, 1, NULL, LY_STMT_STATUS, NULL, &exts);
    assert_int_equal(ret, LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory sub-element \"prefix\" of \"status\" element.", NULL, 1);
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
    CHECK_LOG_CTX("Invalid identifier first character '#' (0x0023).", NULL, 1);

    YCTX->xmlctx->value = "";
    YCTX->xmlctx->value_len = 0;
    assert_int_equal(yin_validate_value(YCTX, Y_STR_ARG), LY_SUCCESS);

    YCTX->xmlctx->value = "pre:b";
    YCTX->xmlctx->value_len = 5;
    assert_int_equal(yin_validate_value(YCTX, Y_IDENTIF_ARG), LY_EVALID);
    CHECK_LOG_CTX("Invalid identifier character ':' (0x003a).", NULL, 1);
    assert_int_equal(yin_validate_value(YCTX, Y_PREF_IDENTIF_ARG), LY_SUCCESS);

    YCTX->xmlctx->value = "pre:pre:b";
    YCTX->xmlctx->value_len = 9;
    assert_int_equal(yin_validate_value(YCTX, Y_PREF_IDENTIF_ARG), LY_EVALID);
    CHECK_LOG_CTX("Invalid identifier character ':' (0x003a).", NULL, 1);
}

static void
test_valid_module(void **state)
{
    struct lys_module *mod;
    char *printed;
    const char *links_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"links\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:mod2=\"urn:module2\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <namespace uri=\"urn:module2\"/>\n"
            "  <prefix value=\"mod2\"/>\n"
            "  <identity name=\"just-another-identity\"/>\n"
            "  <grouping name=\"rgroup\">\n"
            "    <leaf name=\"rg1\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"rg2\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "  </grouping>\n"
            "  <leaf name=\"one-leaf\">\n"
            "    <type name=\"string\"/>\n"
            "  </leaf>\n"
            "  <list name=\"list-for-augment\">\n"
            "    <key value=\"keyleaf\"/>\n"
            "    <leaf name=\"keyleaf\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"just-leaf\">\n"
            "      <type name=\"int32\"/>\n"
            "    </leaf>\n"
            "  </list>\n"
            "  <leaf name=\"rleaf\">\n"
            "    <type name=\"string\"/>\n"
            "  </leaf>\n"
            "  <leaf-list name=\"llist\">\n"
            "    <type name=\"string\"/>\n"
            "    <min-elements value=\"0\"/>\n"
            "    <max-elements value=\"100\"/>\n"
            "    <ordered-by value=\"user\"/>\n"
            "  </leaf-list>\n"
            "</module>\n";
    const char *statements_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"statements\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:mod=\"urn:module\"\n"
            "        xmlns:mod2=\"urn:module2\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <namespace uri=\"urn:module\"/>\n"
            "  <prefix value=\"mod\"/>\n"
            "  <import module=\"links\">\n"
            "    <prefix value=\"mod2\"/>\n"
            "  </import>\n"
            "  <extension name=\"ext\"/>\n"
            "  <identity name=\"random-identity\">\n"
            "    <base name=\"mod2:just-another-identity\"/>\n"
            "    <base name=\"another-identity\"/>\n"
            "  </identity>\n"
            "  <identity name=\"another-identity\">\n"
            "    <base name=\"mod2:just-another-identity\"/>\n"
            "  </identity>\n"
            "  <typedef name=\"percent\">\n"
            "    <type name=\"uint8\">\n"
            "      <range value=\"0 .. 100\"/>\n"
            "    </type>\n"
            "    <units name=\"percent\"/>\n"
            "  </typedef>\n"
            "  <list name=\"list1\">\n"
            "    <key value=\"a\"/>\n"
            "    <leaf name=\"a\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"x\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"y\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "  </list>\n"
            "  <container name=\"ice-cream-shop\">\n"
            "    <container name=\"employees\">\n"
            "      <when condition=\"/list1/x\"/>\n"
            "      <list name=\"employee\">\n"
            "        <key value=\"id\"/>\n"
            "        <unique tag=\"name\"/>\n"
            "        <config value=\"true\"/>\n"
            "        <min-elements value=\"0\">\n"
            "          <mod:ext/>\n"
            "        </min-elements>\n"
            "        <max-elements value=\"unbounded\"/>\n"
            "        <leaf name=\"id\">\n"
            "          <type name=\"uint64\"/>\n"
            "          <mandatory value=\"true\"/>\n"
            "        </leaf>\n"
            "        <leaf name=\"name\">\n"
            "          <type name=\"string\"/>\n"
            "        </leaf>\n"
            "        <leaf name=\"age\">\n"
            "          <type name=\"uint32\"/>\n"
            "        </leaf>\n"
            "      </list>\n"
            "    </container>\n"
            "  </container>\n"
            "  <container name=\"random\">\n"
            "    <grouping name=\"group\">\n"
            "      <leaf name=\"g1\">\n"
            "        <type name=\"percent\"/>\n"
            "        <mandatory value=\"false\"/>\n"
            "      </leaf>\n"
            "      <leaf name=\"g2\">\n"
            "        <type name=\"string\"/>\n"
            "      </leaf>\n"
            "    </grouping>\n"
            "    <choice name=\"switch\">\n"
            "      <case name=\"a\">\n"
            "        <leaf name=\"aleaf\">\n"
            "          <type name=\"string\"/>\n"
            "          <default value=\"aaa\"/>\n"
            "        </leaf>\n"
            "      </case>\n"
            "      <case name=\"c\">\n"
            "        <leaf name=\"cleaf\">\n"
            "          <type name=\"string\"/>\n"
            "        </leaf>\n"
            "      </case>\n"
            "    </choice>\n"
            "    <anyxml name=\"xml-data\"/>\n"
            "    <anydata name=\"any-data\"/>\n"
            "    <leaf-list name=\"leaflist\">\n"
            "      <type name=\"string\"/>\n"
            "      <min-elements value=\"0\"/>\n"
            "      <max-elements value=\"20\"/>\n"
            "    </leaf-list>\n"
            "    <uses name=\"group\"/>\n"
            "    <uses name=\"mod2:rgroup\"/>\n"
            "    <leaf name=\"lref\">\n"
            "      <type name=\"leafref\">\n"
            "        <path value=\"/mod2:one-leaf\"/>\n"
            "      </type>\n"
            "    </leaf>\n"
            "    <leaf name=\"iref\">\n"
            "      <type name=\"identityref\">\n"
            "        <base name=\"mod2:just-another-identity\"/>\n"
            "      </type>\n"
            "    </leaf>\n"
            "  </container>\n"
            "  <augment target-node=\"/random\">\n"
            "    <leaf name=\"aug-leaf\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "  </augment>\n"
            "  <notification name=\"notif\"/>\n"
            "  <deviation target-node=\"/mod:ice-cream-shop/mod:employees/mod:employee/mod:age\">\n"
            "    <deviate value=\"not-supported\">\n"
            "      <mod:ext/>\n"
            "    </deviate>\n"
            "  </deviation>\n"
            "  <deviation target-node=\"/mod:list1\">\n"
            "    <deviate value=\"add\">\n"
            "      <mod:ext/>\n"
            "      <must condition=\"1\"/>\n"
            "      <must condition=\"2\"/>\n"
            "      <unique tag=\"x\"/>\n"
            "      <unique tag=\"y\"/>\n"
            "      <config value=\"true\"/>\n"
            "      <min-elements value=\"1\"/>\n"
            "      <max-elements value=\"2\"/>\n"
            "    </deviate>\n"
            "  </deviation>\n"
            "  <deviation target-node=\"/mod:ice-cream-shop/mod:employees/mod:employee\">\n"
            "    <deviate value=\"delete\">\n"
            "      <unique tag=\"name\"/>\n"
            "    </deviate>\n"
            "  </deviation>\n"
            "  <deviation target-node=\"/mod:random/mod:leaflist\">\n"
            "    <deviate value=\"replace\">\n"
            "      <type name=\"uint32\"/>\n"
            "      <min-elements value=\"10\"/>\n"
            "      <max-elements value=\"15\"/>\n"
            "    </deviate>\n"
            "  </deviation>\n"
            "</module>\n";

    UTEST_ADD_MODULE(links_yin, LYS_IN_YIN, NULL, NULL);
    UTEST_ADD_MODULE(statements_yin, LYS_IN_YIN, NULL, &mod);
    lys_print_mem(&printed, mod, LYS_OUT_YIN, 0);
    assert_string_equal(printed, statements_yin);
    free(printed);
}

static void
test_print_module(void **state)
{
    struct lys_module *mod;

    char *orig = malloc(8096);

    strcpy(orig,
            "module all {\n"
            "    yang-version 1.1;\n"
            "    namespace \"urn:all\";\n"
            "    prefix all_mod;\n\n"
            "    import ietf-yang-types {\n"
            "        prefix yt;\n"
            "        revision-date 2013-07-15;\n"
            "        description\n"
            "            \"YANG types\";\n"
            "        reference\n"
            "            \"RFC reference\";\n"
            "    }\n\n"
            "    feature feat1 {\n"
            "        if-feature \"feat2\";\n"
            "        status obsolete;\n"
            "    }\n\n"
            "    feature feat2;\n"
            "    feature feat3;\n\n"
            "    identity ident2 {\n"
            "        base ident1;\n"
            "    }\n\n"
            "    identity ident1;\n\n"
            "    typedef tdef1 {\n"
            "        type tdef2 {\n"
            "            length \"3..9 | 30..40\";\n"
            "            pattern \"[ac]*\";\n"
            "        }\n"
            "        units \"none\";\n"
            "        default \"aaa\";\n"
            "    }\n\n"
            "    typedef tdef2 {\n"
            "        type string {\n"
            "            length \"2..10 | 20..50\";\n"
            "            pattern \"[ab]*\";\n"
            "        }\n"
            "    }\n\n"
            "    grouping group1 {\n"
            "        leaf leaf1 {\n"
            "            type int8;\n"
            "        }\n"
            "    }\n\n"
            "    container cont1 {\n"
            "        leaf leaf2 {\n"
            "            if-feature \"feat1\";\n"
            "            type int16;\n"
            "            status obsolete;\n"
            "        }\n\n"
            "        uses group1 {\n"
            "            if-feature \"feat2\";\n"
            "            refine \"leaf1\" {\n"
            "                if-feature \"feat3\";\n"
            "                must \"24 - 4 = number('20')\";\n"
            "                default \"25\";\n"
            "                config true;\n"
            "                mandatory false;\n"
            "                description\n"
            "                    \"dsc\";\n"
            "                reference\n"
            "                    \"none\";\n"
            "            }\n"
            "        }\n\n"
            "        leaf leaf3 {\n"
            "            type int32;\n"
            "        }\n\n"
            "        leaf leaf4 {\n"
            "            type int64 {\n"
            "                range \"1000 .. 50000\" {\n"
            "                    error-message\n"
            "                        \"Special error message.\";\n"
            "                    error-app-tag \"special-tag\";\n"
            "                }\n"
            "            }\n"
            "        }\n\n"
            "        leaf leaf5 {\n"
            "            type uint8;\n"
            "        }\n\n"
            "        leaf leaf6 {\n"
            "            type uint16;\n"
            "        }\n\n"
            "        leaf leaf7 {\n"
            "            type uint32;\n"
            "        }\n\n"
            "        leaf leaf8 {\n"
            "            type uint64;\n"
            "        }\n\n"
            "        choice choic1 {\n"
            "            default \"leaf9b\";\n"
            "            leaf leaf9a {\n"
            "                type decimal64 {\n"
            "                    fraction-digits 9;\n"
            "                }\n"
            "            }\n\n"
            "            leaf leaf9b {\n"
            "                type boolean;\n"
            "                default \"false\";\n"
            "            }\n"
            "        }\n\n"
            "        leaf leaf10 {\n"
            "            type boolean;\n"
            "        }\n\n");
    strcpy(orig + strlen(orig),
            "        leaf leaf11 {\n"
            "            type enumeration {\n"
            "                enum \"one\";\n"
            "                enum \"two\";\n"
            "                enum \"five\" {\n"
            "                    value 5;\n"
            "                }\n"
            "            }\n"
            "        }\n\n"
            "        leaf leaf12 {\n"
            "            type bits {\n"
            "                bit flag0 {\n"
            "                    position 0;\n"
            "                }\n"
            "                bit flag1;\n"
            "                bit flag2 {\n"
            "                    position 2;\n"
            "                }\n"
            "                bit flag3 {\n"
            "                    position 3;\n"
            "                }\n"
            "            }\n"
            "            default \"flag0 flag3\";\n"
            "        }\n\n"
            "        leaf leaf13 {\n"
            "            type binary;\n"
            "        }\n\n"
            "        leaf leaf14 {\n"
            "            type leafref {\n"
            "                path \"/cont1/leaf17\";\n"
            "            }\n"
            "        }\n\n"
            "        leaf leaf15 {\n"
            "            type empty;\n"
            "        }\n\n"
            "        leaf leaf16 {\n"
            "            type union {\n"
            "                type instance-identifier {\n"
            "                    require-instance true;\n"
            "                }\n"
            "                type int8;\n"
            "            }\n"
            "        }\n\n"
            "        list list1 {\n"
            "            key \"leaf18\";\n"
            "            unique \"leaf19\";\n"
            "            min-elements 1;\n"
            "            max-elements 20;\n"
            "            leaf leaf18 {\n"
            "                type string;\n"
            "            }\n\n"
            "            leaf leaf19 {\n"
            "                type uint32;\n"
            "            }\n\n"
            "            anyxml axml1;\n"
            "            anydata adata1;\n\n"
            "            action act1 {\n"
            "                input {\n"
            "                    leaf leaf24 {\n"
            "                        type string;\n"
            "                    }\n"
            "                }\n\n"
            "                output {\n"
            "                    leaf leaf25 {\n"
            "                        type string;\n"
            "                    }\n"
            "                }\n"
            "            }\n\n"
            "            notification notif1 {\n"
            "                leaf leaf26 {\n"
            "                    type string;\n"
            "                }\n"
            "            }\n"
            "        }\n\n"
            "        leaf-list llist1 {\n"
            "            type tdef1;\n"
            "            ordered-by user;\n"
            "        }\n\n"
            "        list list2 {\n"
            "            key \"leaf27 leaf28\";\n"
            "            leaf leaf27 {\n"
            "                type uint8;\n"
            "            }\n\n"
            "            leaf leaf28 {\n"
            "                type uint8;\n"
            "            }\n"
            "        }\n\n"
            "        leaf leaf29 {\n"
            "            type instance-identifier;\n"
            "        }\n\n"
            "        container must-deviations-container {\n"
            "            presence \"Allows deviations on the leaf\";\n"
            "            leaf leaf30 {\n"
            "                type string;\n"
            "            }\n"
            "        }\n\n"
            "        leaf leaf23 {\n"
            "            type empty;\n"
            "        }\n"
            "    }\n\n"
            "    augment \"/cont1\" {\n"
            "        leaf leaf17 {\n"
            "            type string;\n"
            "        }\n"
            "    }\n\n"
            "    rpc rpc1 {\n"
            "        input {\n"
            "            leaf leaf20 {\n"
            "                type tdef1;\n"
            "            }\n"
            "        }\n\n"
            "        output {\n"
            "            container cont2 {\n"
            "                leaf leaf21 {\n"
            "                    type empty;\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "    }\n\n"
            "    container test-when {\n"
            "        leaf when-check {\n"
            "            type boolean;\n"
            "        }\n\n"
            "        leaf gated-data {\n"
            "            when \"../when-check = 'true'\";\n"
            "            type uint16;\n"
            "        }\n"
            "    }\n\n"
            "    extension c-define {\n"
            "        description\n"
            "            \"Takes as an argument a name string.\n"
            "            Makes the code generator use the given name\n"
            "            in the #define.\";\n"
            "        argument \"name\";\n"
            "    }\n"
            "}\n");

    char *ori_res = malloc(8096);

    strcpy(ori_res,
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"all\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:all_mod=\"urn:all\"\n"
            "        xmlns:yt=\"urn:ietf:params:xml:ns:yang:ietf-yang-types\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <namespace uri=\"urn:all\"/>\n"
            "  <prefix value=\"all_mod\"/>\n"
            "  <import module=\"ietf-yang-types\">\n"
            "    <prefix value=\"yt\"/>\n"
            "    <revision-date date=\"2013-07-15\"/>\n"
            "    <description>\n"
            "      <text>YANG types</text>\n"
            "    </description>\n"
            "    <reference>\n"
            "      <text>RFC reference</text>\n"
            "    </reference>\n"
            "  </import>\n"
            "  <extension name=\"c-define\">\n"
            "    <argument name=\"name\"/>\n"
            "    <description>\n"
            "      <text>Takes as an argument a name string.\n"
            "Makes the code generator use the given name\n"
            "in the #define.</text>\n"
            "    </description>\n"
            "  </extension>\n"
            "  <feature name=\"feat1\">\n"
            "    <if-feature name=\"feat2\"/>\n"
            "    <status value=\"obsolete\"/>\n"
            "  </feature>\n"
            "  <feature name=\"feat2\"/>\n"
            "  <feature name=\"feat3\"/>\n"
            "  <identity name=\"ident2\">\n"
            "    <base name=\"ident1\"/>\n"
            "  </identity>\n"
            "  <identity name=\"ident1\"/>\n"
            "  <typedef name=\"tdef1\">\n"
            "    <type name=\"tdef2\">\n"
            "      <length value=\"3..9 | 30..40\"/>\n"
            "      <pattern value=\"[ac]*\"/>\n"
            "    </type>\n"
            "    <units name=\"none\"/>\n"
            "    <default value=\"aaa\"/>\n"
            "  </typedef>\n"
            "  <typedef name=\"tdef2\">\n"
            "    <type name=\"string\">\n"
            "      <length value=\"2..10 | 20..50\"/>\n"
            "      <pattern value=\"[ab]*\"/>\n"
            "    </type>\n"
            "  </typedef>\n"
            "  <grouping name=\"group1\">\n"
            "    <leaf name=\"leaf1\">\n"
            "      <type name=\"int8\"/>\n"
            "    </leaf>\n"
            "  </grouping>\n"
            "  <container name=\"cont1\">\n"
            "    <leaf name=\"leaf2\">\n"
            "      <if-feature name=\"feat1\"/>\n"
            "      <type name=\"int16\"/>\n"
            "      <status value=\"obsolete\"/>\n"
            "    </leaf>\n"
            "    <uses name=\"group1\">\n"
            "      <if-feature name=\"feat2\"/>\n"
            "      <refine target-node=\"leaf1\">\n"
            "        <if-feature name=\"feat3\"/>\n"
            "        <must condition=\"24 - 4 = number('20')\"/>\n"
            "        <default value=\"25\"/>\n"
            "        <config value=\"true\"/>\n"
            "        <mandatory value=\"false\"/>\n"
            "        <description>\n"
            "          <text>dsc</text>\n"
            "        </description>\n"
            "        <reference>\n"
            "          <text>none</text>\n"
            "        </reference>\n"
            "      </refine>\n"
            "    </uses>\n"
            "    <leaf name=\"leaf3\">\n"
            "      <type name=\"int32\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf4\">\n"
            "      <type name=\"int64\">\n"
            "        <range value=\"1000 .. 50000\">\n"
            "          <error-message>\n"
            "            <value>Special error message.</value>\n"
            "          </error-message>\n"
            "          <error-app-tag value=\"special-tag\"/>\n"
            "        </range>\n"
            "      </type>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf5\">\n"
            "      <type name=\"uint8\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf6\">\n"
            "      <type name=\"uint16\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf7\">\n"
            "      <type name=\"uint32\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf8\">\n"
            "      <type name=\"uint64\"/>\n"
            "    </leaf>\n"
            "    <choice name=\"choic1\">\n"
            "      <default value=\"leaf9b\"/>\n"
            "      <leaf name=\"leaf9a\">\n"
            "        <type name=\"decimal64\">\n"
            "          <fraction-digits value=\"9\"/>\n"
            "        </type>\n"
            "      </leaf>\n"
            "      <leaf name=\"leaf9b\">\n"
            "        <type name=\"boolean\"/>\n"
            "        <default value=\"false\"/>\n"
            "      </leaf>\n"
            "    </choice>\n"
            "    <leaf name=\"leaf10\">\n"
            "      <type name=\"boolean\"/>\n"
            "    </leaf>\n");
    strcpy(ori_res + strlen(ori_res),
            "    <leaf name=\"leaf11\">\n"
            "      <type name=\"enumeration\">\n"
            "        <enum name=\"one\"/>\n"
            "        <enum name=\"two\"/>\n"
            "        <enum name=\"five\">\n"
            "          <value value=\"5\"/>\n"
            "        </enum>\n"
            "      </type>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf12\">\n"
            "      <type name=\"bits\">\n"
            "        <bit name=\"flag0\">\n"
            "          <position value=\"0\"/>\n"
            "        </bit>\n"
            "        <bit name=\"flag1\"/>\n"
            "        <bit name=\"flag2\">\n"
            "          <position value=\"2\"/>\n"
            "        </bit>\n"
            "        <bit name=\"flag3\">\n"
            "          <position value=\"3\"/>\n"
            "        </bit>\n"
            "      </type>\n"
            "      <default value=\"flag0 flag3\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf13\">\n"
            "      <type name=\"binary\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf14\">\n"
            "      <type name=\"leafref\">\n"
            "        <path value=\"/cont1/leaf17\"/>\n"
            "      </type>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf15\">\n"
            "      <type name=\"empty\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"leaf16\">\n"
            "      <type name=\"union\">\n"
            "        <type name=\"instance-identifier\">\n"
            "          <require-instance value=\"true\"/>\n"
            "        </type>\n"
            "        <type name=\"int8\"/>\n"
            "      </type>\n"
            "    </leaf>\n"
            "    <list name=\"list1\">\n"
            "      <key value=\"leaf18\"/>\n"
            "      <unique tag=\"leaf19\"/>\n"
            "      <min-elements value=\"1\"/>\n"
            "      <max-elements value=\"20\"/>\n"
            "      <leaf name=\"leaf18\">\n"
            "        <type name=\"string\"/>\n"
            "      </leaf>\n"
            "      <leaf name=\"leaf19\">\n"
            "        <type name=\"uint32\"/>\n"
            "      </leaf>\n"
            "      <anyxml name=\"axml1\"/>\n"
            "      <anydata name=\"adata1\"/>\n"
            "      <action name=\"act1\">\n"
            "        <input>\n"
            "          <leaf name=\"leaf24\">\n"
            "            <type name=\"string\"/>\n"
            "          </leaf>\n"
            "        </input>\n"
            "        <output>\n"
            "          <leaf name=\"leaf25\">\n"
            "            <type name=\"string\"/>\n"
            "          </leaf>\n"
            "        </output>\n"
            "      </action>\n"
            "      <notification name=\"notif1\">\n"
            "        <leaf name=\"leaf26\">\n"
            "          <type name=\"string\"/>\n"
            "        </leaf>\n"
            "      </notification>\n"
            "    </list>\n"
            "    <leaf-list name=\"llist1\">\n"
            "      <type name=\"tdef1\"/>\n"
            "      <ordered-by value=\"user\"/>\n"
            "    </leaf-list>\n"
            "    <list name=\"list2\">\n"
            "      <key value=\"leaf27 leaf28\"/>\n"
            "      <leaf name=\"leaf27\">\n"
            "        <type name=\"uint8\"/>\n"
            "      </leaf>\n"
            "      <leaf name=\"leaf28\">\n"
            "        <type name=\"uint8\"/>\n"
            "      </leaf>\n"
            "    </list>\n"
            "    <leaf name=\"leaf29\">\n"
            "      <type name=\"instance-identifier\"/>\n"
            "    </leaf>\n"
            "    <container name=\"must-deviations-container\">\n"
            "      <presence value=\"Allows deviations on the leaf\"/>\n"
            "      <leaf name=\"leaf30\">\n"
            "        <type name=\"string\"/>\n"
            "      </leaf>\n"
            "    </container>\n"
            "    <leaf name=\"leaf23\">\n"
            "      <type name=\"empty\"/>\n"
            "    </leaf>\n"
            "  </container>\n"
            "  <container name=\"test-when\">\n"
            "    <leaf name=\"when-check\">\n"
            "      <type name=\"boolean\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"gated-data\">\n"
            "      <when condition=\"../when-check = 'true'\"/>\n"
            "      <type name=\"uint16\"/>\n"
            "    </leaf>\n"
            "  </container>\n"
            "  <augment target-node=\"/cont1\">\n"
            "    <leaf name=\"leaf17\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "  </augment>\n"
            "  <rpc name=\"rpc1\">\n"
            "    <input>\n"
            "      <leaf name=\"leaf20\">\n"
            "        <type name=\"tdef1\"/>\n"
            "      </leaf>\n"
            "    </input>\n"
            "    <output>\n"
            "      <container name=\"cont2\">\n"
            "        <leaf name=\"leaf21\">\n"
            "          <type name=\"empty\"/>\n"
            "        </leaf>\n"
            "      </container>\n"
            "    </output>\n"
            "  </rpc>\n"
            "</module>\n");

    char *printed;
    struct ly_out *out;

    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&printed, 0, &out));

    UTEST_ADD_MODULE(orig, LYS_IN_YANG, NULL, &mod);
    assert_int_equal(LY_SUCCESS, lys_print_module(out, mod, LYS_OUT_YIN, 0, 0));
    assert_int_equal(strlen(ori_res), ly_out_printed(out));
    assert_string_equal(printed, ori_res);

    ly_out_free(out, NULL, 1);
    free(orig);
    free(ori_res);
}

static LY_ERR
test_imp_clb(const char *UNUSED(mod_name), const char *UNUSED(mod_rev), const char *UNUSED(submod_name),
        const char *UNUSED(sub_rev), void *user_data, LYS_INFORMAT *format,
        const char **module_data, void (**free_module_data)(void *model_data, void *user_data))
{
    *module_data = user_data;
    *format = LYS_IN_YIN;
    *free_module_data = NULL;
    return LY_SUCCESS;
}

static void
test_print_submodule(void **state)
{
    struct lys_module *mod;

    const char *mod_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"a\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:a_mod=\"urn:a\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <namespace uri=\"urn:a\"/>\n"
            "  <prefix value=\"a_mod\"/>\n"
            "  <include module=\"a-sub\"/>\n"
            "</module>\n";

    char *submod_yin =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<submodule name=\"a-sub\"\n"
            "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n"
            "        xmlns:a_mod=\"urn:a\"\n"
            "        xmlns:yt=\"urn:ietf:params:xml:ns:yang:ietf-yang-types\">\n"
            "  <yang-version value=\"1.1\"/>\n"
            "  <belongs-to module=\"a\">\n"
            "    <prefix value=\"a_mod\"/>\n"
            "  </belongs-to>\n"
            "  <import module=\"ietf-yang-types\">\n"
            "    <prefix value=\"yt\"/>\n"
            "    <revision-date date=\"2013-07-15\"/>\n"
            "  </import>\n\n"
            "  <description>\n"
            "    <text>YANG types</text>\n"
            "  </description>\n"
            "  <reference>\n"
            "    <text>RFC reference</text>\n"
            "  </reference>\n"
            "</submodule>\n";

    char *printed;
    struct ly_out *out;

    assert_int_equal(LY_SUCCESS, ly_out_new_memory(&printed, 0, &out));

    ly_ctx_set_module_imp_clb(UTEST_LYCTX, test_imp_clb, submod_yin);

    UTEST_ADD_MODULE(mod_yin, LYS_IN_YIN, NULL, &mod);
    assert_int_equal(LY_SUCCESS, lys_print_submodule(out, mod->parsed->includes[0].submodule, LYS_OUT_YIN, 0, 0));
    assert_int_equal(strlen(submod_yin), ly_out_printed(out));
    assert_string_equal(printed, submod_yin);

    ly_out_free(out, NULL, 1);
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

    YCTX->main_ctx = (struct lysp_ctx *)YCTX;
    ly_in_new_memory(data, &UTEST_IN);
    lyxml_ctx_new(UTEST_LYCTX, UTEST_IN, &YCTX->xmlctx);
    prefix = YCTX->xmlctx->prefix;
    prefix_len = YCTX->xmlctx->prefix_len;
    name = YCTX->xmlctx->name;
    name_len = YCTX->xmlctx->name_len;
    lyxml_ctx_next(YCTX->xmlctx);

    ret = yin_parse_content(YCTX, subelems, 71, NULL,
            yin_match_keyword(YCTX, name, name_len, prefix, prefix_len, LY_STMT_NONE), text, exts);

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
    struct lysp_type type = {0};
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
    lysp_type_free(&fctx, &type);
    memset(&type, 0, sizeof type);

    data = ELEMENT_WRAPPER_START
            "<enum name=\"enum-name\"></enum>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    assert_string_equal(type.enums->name, "enum-name");
    lysp_type_free(&fctx, &type);
    memset(&type, 0, sizeof type);
}

static void
test_bit_elem(void **state)
{
    struct lysp_type type = {0};
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
    lysp_type_free(&fctx, &type);
    memset(&type, 0, sizeof type);

    data = ELEMENT_WRAPPER_START
            "<bit name=\"bit-name\"> </bit>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_TYPE_ENUM(type.bits, NULL, 0, 0, 0, "bit-name", NULL, 0);
    lysp_type_free(&fctx, &type);
    memset(&type, 0, sizeof type);
}

static void
test_status_elem(void **state)
{
    const char *data;
    uint16_t flags = 0;

    /* test invalid value */
    data = ELEMENT_WRAPPER_START "<status value=\"invalid\"></status>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"invalid\" of \"value\" attribute in \"status\" element. "
            "Valid values are \"current\", \"deprecated\" and \"obsolete\".", NULL, 1);
}

static void
test_yin_element_elem(void **state)
{
    const char *data;
    uint16_t flags = 0;

    data = ELEMENT_WRAPPER_START "<yin-element value=\"invalid\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"invalid\" of \"value\" attribute in \"yin-element\" element. "
            "Valid values are \"true\" and \"false\".", NULL, 1);
}

static void
test_yangversion_elem(void **state)
{
    const char *data;
    uint8_t version = 0;

    /* invalid value */
    data = ELEMENT_WRAPPER_START "<yang-version value=\"version\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &version, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"version\" of \"value\" attribute in \"yang-version\" element. "
            "Valid values are \"1\" and \"1.1\".", NULL, 1);
}

static void
test_argument_elem(void **state)
{
    const char *data;
    uint16_t flags = 0;
    const char *arg;
    struct yin_argument_meta arg_meta = {&flags, &arg};

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
test_belongsto_elem(void **state)
{
    const char *data;
    struct lysp_submodule submod;

    lydict_insert(UTEST_LYCTX, "module-name", 0, &PARSER_CUR_PMOD(YCTX)->mod->name);

    data = ELEMENT_WRAPPER_START "<belongs-to module=\"module-name\"></belongs-to>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &submod, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory sub-element \"prefix\" of \"belongs-to\" element.", NULL, 1);
}

static void
test_config_elem(void **state)
{
    const char *data;
    uint16_t flags = 0;

    data = ELEMENT_WRAPPER_START "<config value=\"false\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, NULL), LY_SUCCESS);
    assert_true(flags & LYS_CONFIG_R);
    flags = 0;

    data = ELEMENT_WRAPPER_START "<config value=\"invalid\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"invalid\" of \"value\" attribute in \"config\" element. "
            "Valid values are \"true\" and \"false\".", NULL, 1);
}

static void
test_default_elem(void **state)
{
    const char *data;
    struct lysp_qname val = {0};

    data = ELEMENT_WRAPPER_START "<default/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory attribute value of default element.", NULL, 1);
}

static void
test_err_app_tag_elem(void **state)
{
    const char *data;
    const char *val = NULL;

    data = ELEMENT_WRAPPER_START "<error-app-tag/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory attribute value of error-app-tag element.", NULL, 1);
}

static void
test_err_msg_elem(void **state)
{
    const char *data;
    const char *val = NULL;

    data = ELEMENT_WRAPPER_START "<error-message></error-message>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory sub-element \"value\" of \"error-message\" element.", NULL, 1);

    data = ELEMENT_WRAPPER_START "<error-message invalid=\"text\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Unexpected attribute \"invalid\" of \"error-message\" element.", NULL, 1);
}

static void
test_fracdigits_elem(void **state)
{
    const char *data;
    struct lysp_type type = {0};

    /* invalid values */
    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"-1\"></fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"-1\" of \"value\" attribute in \"fraction-digits\" element.", NULL, 1);

    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"02\"></fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"02\" of \"value\" attribute in \"fraction-digits\" element.", NULL, 1);

    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"1p\"></fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"1p\" of \"value\" attribute in \"fraction-digits\" element.", NULL, 1);

    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"19\"></fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"19\" of \"value\" attribute in \"fraction-digits\" element.", NULL, 1);

    data = ELEMENT_WRAPPER_START "<fraction-digits value=\"999999999999999999\"></fraction-digits>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"999999999999999999\" of \"value\" attribute in \"fraction-digits\" element.", NULL, 1);
}

static void
test_iffeature_elem(void **state)
{
    const char *data;
    const char **iffeatures = NULL;

    data = ELEMENT_WRAPPER_START "<if-feature/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &iffeatures, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory attribute name of if-feature element.", NULL, 1);
    LY_ARRAY_FREE(iffeatures);
    iffeatures = NULL;
}

static void
test_length_elem(void **state)
{
    const char *data;
    struct lysp_type type = {0};

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
    lysp_type_free(&fctx, &type);
    memset(&type, 0, sizeof(type));

    /* min subelems */
    data = ELEMENT_WRAPPER_START
            "<length value=\"length-str\">"
            "</length>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_RESTR(type.length, "length-str", NULL,
            NULL, NULL, 0, NULL);
    lysp_type_free(&fctx, &type);
    assert_true(type.flags & LYS_SET_LENGTH);
    memset(&type, 0, sizeof(type));

    data = ELEMENT_WRAPPER_START "<length></length>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory attribute value of length element.", NULL, 1);
    lysp_type_free(&fctx, &type);
    memset(&type, 0, sizeof(type));
}

static void
test_modifier_elem(void **state)
{
    const char *data;
    const char *pat;

    assert_int_equal(LY_SUCCESS, lydict_insert(UTEST_LYCTX, "\006pattern", 8, &pat));
    data = ELEMENT_WRAPPER_START "<modifier value=\"invert\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &pat, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"invert\" of \"value\" attribute in \"modifier\" element. "
            "Only valid value is \"invert-match\".", NULL, 1);
    lydict_remove(UTEST_LYCTX, pat);
}

static void
test_namespace_elem(void **state)
{
    const char *data;
    const char *ns;

    data = ELEMENT_WRAPPER_START "<namespace/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &ns, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory attribute uri of namespace element.", NULL, 1);
}

static void
test_pattern_elem(void **state)
{
    const char *data;
    struct lysp_type type = {0};

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
    lysp_type_free(&fctx, &type);
    memset(&type, 0, sizeof(type));

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<pattern value=\"pattern\"> </pattern>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_RESTR(type.patterns, "\x006pattern", NULL, NULL, NULL, 0, NULL);
    lysp_type_free(&fctx, &type);
    memset(&type, 0, sizeof(type));
}

static void
test_value_position_elem(void **state)
{
    const char *data;
    struct lysp_type_enum en = {0};

    /* valid values */
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
    data = ELEMENT_WRAPPER_START "<position value=\"0\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_TYPE_ENUM(&(en), NULL, 0, LYS_SET_VALUE, 0, NULL, NULL, 0);
    memset(&en, 0, sizeof(en));

    /* invalid values */
    data = ELEMENT_WRAPPER_START "<value value=\"99999999999999999999999\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"99999999999999999999999\" of \"value\" attribute in \"value\" element.", NULL, 1);

    data = ELEMENT_WRAPPER_START "<value value=\"1k\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"1k\" of \"value\" attribute in \"value\" element.", NULL, 1);

    data = ELEMENT_WRAPPER_START "<value value=\"\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"\" of \"value\" attribute in \"value\" element.", NULL, 1);

    /*invalid positions */
    data = ELEMENT_WRAPPER_START "<position value=\"-5\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"-5\" of \"value\" attribute in \"position\" element.", NULL, 1);

    data = ELEMENT_WRAPPER_START "<position value=\"-0\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"-0\" of \"value\" attribute in \"position\" element.", NULL, 1);

    data = ELEMENT_WRAPPER_START "<position value=\"99999999999999999999\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"99999999999999999999\" of \"value\" attribute in \"position\" element.", NULL, 1);

    data = ELEMENT_WRAPPER_START "<position value=\"\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &en, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"\" of \"value\" attribute in \"position\" element.", NULL, 1);
}

static void
test_prefix_elem(void **state)
{
    const char *data;
    const char *value = NULL;

    data = ELEMENT_WRAPPER_START "<prefix value=\"pref\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &value, NULL, NULL), LY_SUCCESS);
    assert_string_equal(value, "pref");
    lydict_remove(UTEST_LYCTX, value);
}

static void
test_range_elem(void **state)
{
    const char *data;
    struct lysp_type type = {0};

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
    lysp_type_free(&fctx, &type);
    memset(&type, 0, sizeof(type));

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<range value=\"range-str\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    CHECK_LYSP_RESTR(type.range, "range-str", NULL,
            NULL, NULL, 0, NULL);
    lysp_type_free(&fctx, &type);
    memset(&type, 0, sizeof(type));
}

static void
test_reqinstance_elem(void **state)
{
    const char *data;
    struct lysp_type type = {0};

    data = ELEMENT_WRAPPER_START "<require-instance value=\"true\">" EXT_SUBELEM "</require-instance>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    assert_int_equal(type.require_instance, 1);
    assert_true(type.flags & LYS_SET_REQINST);
    TEST_1_CHECK_LYSP_EXT_INSTANCE(&(type.exts[0]), LY_STMT_REQUIRE_INSTANCE);
    lysp_type_free(&fctx, &type);
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
            "Valid values are \"true\" and \"false\".", NULL, 1);
}

static void
test_revision_date_elem(void **state)
{
    const char *data;
    char rev[LY_REV_SIZE];

    data = ELEMENT_WRAPPER_START "<revision-date date=\"2000-01-01\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, rev, NULL, NULL), LY_SUCCESS);
    assert_string_equal(rev, "2000-01-01");

    data = ELEMENT_WRAPPER_START "<revision-date date=\"2000-50-05\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, rev, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"2000-50-05\" of \"revision-date\".", NULL, 1);
}

static void
test_unique_elem(void **state)
{
    const char *data;
    const char **values = NULL;

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

    data = ELEMENT_WRAPPER_START "<units name=\"name\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &values, NULL, NULL), LY_SUCCESS);
    assert_string_equal(values, "name");
    lydict_remove(UTEST_LYCTX, values);
    values = NULL;
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
    struct lysp_type type = {0};

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
    lysp_type_free(&fctx, &type);
    memset(&type, 0, sizeof(type));

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<type name=\"type-name\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &type, NULL, NULL), LY_SUCCESS);
    lysp_type_free(&fctx, &type);
    memset(&type, 0, sizeof(type));
}

static void
test_max_elems_elem(void **state)
{
    const char *data;
    struct lysp_node_list list = {0};
    struct lysp_refine refine = {0};

    data = "<refine xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"10\"/> </refine>";
    assert_int_equal(test_element_helper(state, data, &refine, NULL, NULL), LY_SUCCESS);
    assert_int_equal(refine.max, 10);
    assert_true(refine.flags & LYS_SET_MAX);

    data = "<list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"0\"/> </list>";
    assert_int_equal(test_element_helper(state, data, &list, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"0\" of \"value\" attribute in \"max-elements\" element.", NULL, 1);

    data = "<list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"-10\"/> </list>";
    assert_int_equal(test_element_helper(state, data, &list, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"-10\" of \"value\" attribute in \"max-elements\" element.", NULL, 1);

    data = "<list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"k\"/> </list>";
    assert_int_equal(test_element_helper(state, data, &list, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"k\" of \"value\" attribute in \"max-elements\" element.", NULL, 1);

    data = "<list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <max-elements value=\"u12\"/> </list>";
    assert_int_equal(test_element_helper(state, data, &list, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"u12\" of \"value\" attribute in \"max-elements\" element.", NULL, 1);
}

static void
test_min_elems_elem(void **state)
{
    const char *data;
    struct lysp_node_leaflist llist = {0};

    data = "<leaf-list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"-5\"/> </leaf-list>";
    assert_int_equal(test_element_helper(state, data, &llist, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Value \"-5\" of \"value\" attribute in \"min-elements\" element is out of bounds.", NULL, 1);

    data = "<leaf-list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"99999999999999999\"/> </leaf-list>";
    assert_int_equal(test_element_helper(state, data, &llist, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Value \"99999999999999999\" of \"value\" attribute in \"min-elements\" element is out of bounds.", NULL, 1);

    data = "<leaf-list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"5k\"/> </leaf-list>";
    assert_int_equal(test_element_helper(state, data, &llist, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"5k\" of \"value\" attribute in \"min-elements\" element.", NULL, 1);

    data = "<leaf-list xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"> <min-elements value=\"05\"/> </leaf-list>";
    assert_int_equal(test_element_helper(state, data, &llist, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"05\" of \"value\" attribute in \"min-elements\" element.", NULL, 1);
}

static void
test_ordby_elem(void **state)
{
    const char *data;
    uint16_t flags = 0;

    data = ELEMENT_WRAPPER_START "<ordered-by value=\"user\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, NULL), LY_SUCCESS);
    assert_true(flags & LYS_ORDBY_USER);

    data = ELEMENT_WRAPPER_START "<ordered-by value=\"inv\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &flags, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"inv\" of \"value\" attribute in \"ordered-by\" element. "
            "Valid values are \"system\" and \"user\".", NULL, 1);
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
    lysp_node_free(&fctx, siblings);
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
    lysp_node_free(&fctx, siblings);
    siblings = NULL;

    /* min subelems */
    node_meta.parent = (void *)0x10;
    data = ELEMENT_WRAPPER_START "<anydata name=\"any-name\"> </anydata>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_anydata *)siblings;
    assert_ptr_equal(parsed->parent, node_meta.parent);
    CHECK_LYSP_NODE(parsed, NULL, 0, 0, 0,
            "any-name", 0, LYS_ANYDATA, 1, NULL, 0);
    lysp_node_free(&fctx, siblings);
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
    lysp_node_free(&fctx, siblings);
    siblings = NULL;

    /* min elements */
    data = ELEMENT_WRAPPER_START "<leaf name=\"leaf\"> <type name=\"type\"/> </leaf>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_leaf *)siblings;
    assert_string_equal(parsed->name, "leaf");
    assert_string_equal(parsed->type.name, "type");
    lysp_node_free(&fctx, siblings);
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
    lysp_node_free(&fctx, siblings);
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
    lysp_node_free(&fctx, siblings);
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
    lysp_node_free(&fctx, siblings);
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
    lysp_node_free(&fctx, siblings);
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
    CHECK_LOG_CTX("Invalid combination of min-elements and max-elements: min value 15 is bigger than the max value 5.",
            NULL, 4);
    lysp_node_free(&fctx, siblings);
    siblings = NULL;

    data = ELEMENT_WRAPPER_START
            "<leaf-list name=\"llist\">\n"
            "    <default value=\"def-val1\"/>\n"
            "    <min-elements value=\"15\"/>\n"
            "    <type name=\"type\"/>\n"
            "</leaf-list>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid combination of sub-elemnts \"min-elements\" and \"default\" in \"leaf-list\" element.", NULL, 5);
    lysp_node_free(&fctx, siblings);
    siblings = NULL;

    data = ELEMENT_WRAPPER_START
            "<leaf-list name=\"llist\">"
            "</leaf-list>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory sub-element \"type\" of \"leaf-list\" element.", NULL, 1);
    lysp_node_free(&fctx, siblings);
    siblings = NULL;
}

static void
test_presence_elem(void **state)
{
    const char *data;
    const char *val;

    data = ELEMENT_WRAPPER_START "<presence value=\"presence-val\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_SUCCESS);
    assert_string_equal(val, "presence-val");
    lydict_remove(UTEST_LYCTX, val);

    data = ELEMENT_WRAPPER_START "<presence/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory attribute value of presence element.", NULL, 1);
}

static void
test_key_elem(void **state)
{
    const char *data;
    const char *val;

    data = ELEMENT_WRAPPER_START "<key value=\"key-value\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_SUCCESS);
    assert_string_equal(val, "key-value");
    lydict_remove(UTEST_LYCTX, val);

    data = ELEMENT_WRAPPER_START "<key/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &val, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory attribute value of key element.", NULL, 1);
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
    lysp_node_free(&fctx, siblings);
    siblings = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<uses name=\"uses-name\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(siblings[0].name, "uses-name");
    lysp_node_free(&fctx, siblings);
    siblings = NULL;
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
    lysp_node_free(&fctx, siblings);
    ly_set_erase(&YCTX->tpdfs_nodes, NULL);
    siblings = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<list name=\"list-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_list *)&siblings[0];
    CHECK_LYSP_NODE(parsed, NULL, 0, 0, 0,
            "list-name", 0, LYS_LIST, 0, NULL, 0);
    lysp_node_free(&fctx, siblings);
    siblings = NULL;
}

static void
test_notification_elem(void **state)
{
    const char *data;
    struct lysp_node_notif *notifs = NULL;
    struct tree_node_meta notif_meta = {NULL, (struct lysp_node **)&notifs};

    /* max subelems */
    PARSER_CUR_PMOD(YCTX)->version = LYS_VERSION_1_1;
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
    lysp_node_free(&fctx, (struct lysp_node *)notifs);
    notifs = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<notification name=\"notif-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &notif_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(notifs->name, "notif-name");
    lysp_node_free(&fctx, (struct lysp_node *)notifs);
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
    lysp_node_free(&fctx, &grps->node);
    grps = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<grouping name=\"grp-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &grp_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(grps->name, "grp-name");
    lysp_node_free(&fctx, &grps->node);
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
    PARSER_CUR_PMOD(YCTX)->version = LYS_VERSION_1_1;
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
    lysp_node_free(&fctx, siblings);
    ly_set_erase(&YCTX->tpdfs_nodes, NULL);
    siblings = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<container name=\"cont-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_container *)siblings;
    CHECK_LYSP_NODE(parsed, NULL, 0, 0, 0,
            "cont-name", 0, LYS_CONTAINER, 0, NULL, 0);
    lysp_node_free(&fctx, siblings);
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
    PARSER_CUR_PMOD(YCTX)->version = LYS_VERSION_1_1;
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
    lysp_node_free(&fctx, siblings);
    siblings = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<case name=\"case-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_case *)siblings;
    CHECK_LYSP_NODE(parsed, NULL, 0, 0, 0,
            "case-name", 0, LYS_CASE, 0, NULL, 0);
    lysp_node_free(&fctx, siblings);
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
    PARSER_CUR_PMOD(YCTX)->version = LYS_VERSION_1_1;
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
    lysp_node_free(&fctx, siblings);
    siblings = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<choice name=\"choice-name\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &node_meta, NULL, NULL), LY_SUCCESS);
    parsed = (struct lysp_node_choice *)siblings;
    assert_string_equal(parsed->name, "choice-name");
    CHECK_LYSP_NODE(parsed, NULL, 0, 0, 0,
            "choice-name", 0, LYS_CHOICE, 0, NULL, 0);
    lysp_node_free(&fctx, siblings);
    siblings = NULL;
}

static void
test_inout_elem(void **state)
{
    const char *data;
    struct lysp_node_action_inout inout = {0};
    struct inout_meta inout_meta = {NULL, &inout};

    /* max subelements */
    PARSER_CUR_PMOD(YCTX)->version = LYS_VERSION_1_1;
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
    lysp_node_free(&fctx, (struct lysp_node *)&inout);
    memset(&inout, 0, sizeof inout);

    /* max subelements */
    PARSER_CUR_PMOD(YCTX)->version = LYS_VERSION_1_1;
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
    lysp_node_free(&fctx, (struct lysp_node *)&inout);
    memset(&inout, 0, sizeof inout);

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<input><leaf name=\"l\"><type name=\"empty\"/></leaf></input>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &inout_meta, NULL, NULL), LY_SUCCESS);
    lysp_node_free(&fctx, (struct lysp_node *)&inout);
    memset(&inout, 0, sizeof inout);

    data = ELEMENT_WRAPPER_START "<output><leaf name=\"l\"><type name=\"empty\"/></leaf></output>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &inout_meta, NULL, NULL), LY_SUCCESS);
    lysp_node_free(&fctx, (struct lysp_node *)&inout);
    memset(&inout, 0, sizeof inout);

    /* invalid combinations */
    data = ELEMENT_WRAPPER_START "<input name=\"test\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &inout_meta, NULL, NULL), LY_EVALID);
    lysp_node_free(&fctx, (struct lysp_node *)&inout);
    CHECK_LOG_CTX("Unexpected attribute \"name\" of \"input\" element.", NULL, 1);
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
    PARSER_CUR_PMOD(YCTX)->version = LYS_VERSION_1_1;
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
    lysp_node_free(&fctx, (struct lysp_node *)actions);
    actions = NULL;

    PARSER_CUR_PMOD(YCTX)->version = LYS_VERSION_1_1;
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
    lysp_node_free(&fctx, (struct lysp_node *)actions);
    actions = NULL;

    /* min subelems */
    data = ELEMENT_WRAPPER_START "<action name=\"act\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &act_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(actions->name, "act");
    lysp_node_free(&fctx, (struct lysp_node *)actions);
    actions = NULL;
}

static void
test_augment_elem(void **state)
{
    const char *data;
    struct lysp_node_augment *augments = NULL;
    struct tree_node_meta aug_meta = {NULL, (struct lysp_node **)&augments};

    PARSER_CUR_PMOD(YCTX)->version = LYS_VERSION_1_1;
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
    lysp_node_free(&fctx, (struct lysp_node *)augments);
    augments = NULL;

    data = ELEMENT_WRAPPER_START "<augment target-node=\"target\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &aug_meta, NULL, NULL), LY_SUCCESS);
    assert_string_equal(augments->nodeid, "target");
    lysp_node_free(&fctx, (struct lysp_node *)augments);
    augments = NULL;
}

static void
test_deviate_elem(void **state)
{
    const char *data;
    struct lysp_deviate *deviates = NULL;

    /* invalid arguments */
    data = ELEMENT_WRAPPER_START "<deviate value=\"\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"\" of \"value\" attribute in \"deviate\" element. "
            "Valid values are \"not-supported\", \"add\", \"replace\" and \"delete\".", NULL, 1);
    deviates = NULL;

    data = ELEMENT_WRAPPER_START "<deviate value=\"invalid\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"invalid\" of \"value\" attribute in \"deviate\" element. "
            "Valid values are \"not-supported\", \"add\", \"replace\" and \"delete\".", NULL, 1);
    deviates = NULL;

    data = ELEMENT_WRAPPER_START "<deviate value=\"ad\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"ad\" of \"value\" attribute in \"deviate\" element. "
            "Valid values are \"not-supported\", \"add\", \"replace\" and \"delete\".", NULL, 1);
    deviates = NULL;

    data = ELEMENT_WRAPPER_START "<deviate value=\"adds\" />" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"adds\" of \"value\" attribute in \"deviate\" element. "
            "Valid values are \"not-supported\", \"add\", \"replace\" and \"delete\".", NULL, 1);
    deviates = NULL;

    data = ELEMENT_WRAPPER_START
            "<deviate value=\"not-supported\">\n"
            "    <must condition=\"c\"/>\n"
            "</deviate>"
            ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviates, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Deviate of this type doesn't allow \"must\" as it's sub-element.", NULL, 2);
}

static void
test_deviation_elem(void **state)
{
    const char *data;
    struct lysp_deviation *deviations = NULL;

    /* invalid */
    data = ELEMENT_WRAPPER_START "<deviation target-node=\"target\"/>" ELEMENT_WRAPPER_END;
    assert_int_equal(test_element_helper(state, data, &deviations, NULL, NULL), LY_EVALID);
    CHECK_LOG_CTX("Missing mandatory sub-element \"deviate\" of \"deviation\" element.", NULL, 1);
}

static struct lysp_module *
mod_renew(struct lysp_yin_ctx *ctx)
{
    struct ly_ctx *ly_ctx = PARSER_CUR_PMOD(ctx)->mod->ctx;
    struct lysp_module *pmod;

    lys_module_free(&fctx, PARSER_CUR_PMOD(ctx)->mod, 0);
    pmod = calloc(1, sizeof *pmod);
    ctx->parsed_mods->objs[0] = pmod;
    pmod->mod = calloc(1, sizeof *pmod->mod);
    pmod->mod->parsed = pmod;
    pmod->mod->ctx = ly_ctx;

    fctx.mod = pmod->mod;

    return pmod;
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
    assert_string_equal(lysp_mod->revs[0].date, "2019-02-02");
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
    CHECK_LOG_CTX("Invalid order of module\'s sub-elements \"namespace\" can\'t appear after \"feature\".", NULL, 3);
}

static struct lysp_submodule *
submod_renew(struct lysp_yin_ctx *ctx, const char *belongs_to)
{
    struct ly_ctx *ly_ctx = PARSER_CUR_PMOD(ctx)->mod->ctx;
    struct lysp_submodule *submod;

    lys_module_free(&fctx, PARSER_CUR_PMOD(ctx)->mod, 0);
    submod = calloc(1, sizeof *submod);
    ctx->parsed_mods->objs[0] = submod;
    submod->mod = calloc(1, sizeof *submod->mod);
    lydict_insert(ly_ctx, belongs_to, 0, &submod->mod->name);
    submod->mod->parsed = (struct lysp_module *)submod;
    submod->mod->ctx = ly_ctx;

    fctx.mod = submod->mod;

    return submod;
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
    CHECK_LOG_CTX("YANG version 1.1 expects all includes in main module, includes in submodules (mod) are not necessary.",
            NULL, 0);
    assert_string_equal(lysp_submod->name, "mod");
    assert_string_equal(lysp_submod->revs[0].date, "2019-02-02");
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
    CHECK_LOG_CTX("Invalid order of submodule's sub-elements \"belongs-to\" can't appear after \"reference\".", NULL, 4);
}

static void
test_yin_parse_module(void **state)
{
    const char *data;
    struct lys_module *mod;
    struct lysp_yin_ctx *yin_ctx = NULL;
    struct ly_in *in = NULL;

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
    assert_int_equal(yin_parse_module(&yin_ctx, in, mod), LY_SUCCESS);
    assert_null(mod->parsed->exts->child->next->child);
    assert_string_equal(mod->parsed->exts->child->next->arg, "test");
    lys_module_free(&fctx, mod, 0);
    lysp_yin_ctx_free(yin_ctx);
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
    assert_int_equal(yin_parse_module(&yin_ctx, in, mod), LY_SUCCESS);
    lys_module_free(&fctx, mod, 0);
    lysp_yin_ctx_free(yin_ctx);
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
    assert_int_equal(yin_parse_module(&yin_ctx, in, mod), LY_SUCCESS);
    lys_module_free(&fctx, mod, 0);
    lysp_yin_ctx_free(yin_ctx);
    ly_in_free(in, 0);
    mod = NULL;
    yin_ctx = NULL;

    mod = calloc(1, sizeof *mod);
    mod->ctx = UTEST_LYCTX;
    data = "<submodule name=\"example-foo\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "</submodule>\n";
    assert_int_equal(ly_in_new_memory(data, &in), LY_SUCCESS);
    assert_int_equal(yin_parse_module(&yin_ctx, in, mod), LY_EINVAL);
    CHECK_LOG_CTX("Input data contains submodule which cannot be parsed directly without its main module.", NULL, 0);
    lys_module_free(&fctx, mod, 0);
    lysp_yin_ctx_free(yin_ctx);
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
    assert_int_equal(yin_parse_module(&yin_ctx, in, mod), LY_EVALID);
    CHECK_LOG_CTX("Trailing garbage \"<module>\" after module, expected end-of-input.", NULL, 6);
    lys_module_free(&fctx, mod, 0);
    lysp_yin_ctx_free(yin_ctx);
    ly_in_free(in, 0);
    mod = NULL;
    yin_ctx = NULL;
}

static void
test_yin_parse_submodule(void **state)
{
    const char *data;
    struct lysp_yin_ctx *yin_ctx = NULL;
    struct lysp_submodule *submod = NULL;
    struct ly_in *in;

    lydict_insert(UTEST_LYCTX, "a", 0, &PARSER_CUR_PMOD(YCTX)->mod->name);

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
    assert_int_equal(yin_parse_submodule(&yin_ctx, UTEST_LYCTX, (struct lysp_ctx *)YCTX, in, &submod), LY_SUCCESS);
    lysp_module_free(&fctx, (struct lysp_module *)submod);
    lysp_yin_ctx_free(yin_ctx);
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
    assert_int_equal(yin_parse_submodule(&yin_ctx, UTEST_LYCTX, (struct lysp_ctx *)YCTX, in, &submod), LY_SUCCESS);
    lysp_module_free(&fctx, (struct lysp_module *)submod);
    lysp_yin_ctx_free(yin_ctx);
    ly_in_free(in, 0);
    yin_ctx = NULL;
    submod = NULL;

    data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<module name=\"inval\" xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\">"
            "</module>";
    assert_int_equal(ly_in_new_memory(data, &in), LY_SUCCESS);
    assert_int_equal(yin_parse_submodule(&yin_ctx, UTEST_LYCTX, (struct lysp_ctx *)YCTX, in, &submod), LY_EINVAL);
    CHECK_LOG_CTX("Input data contains module when a submodule is expected.", NULL, 0);
    lysp_module_free(&fctx, (struct lysp_module *)submod);
    lysp_yin_ctx_free(yin_ctx);
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
    assert_int_equal(yin_parse_submodule(&yin_ctx, UTEST_LYCTX, (struct lysp_ctx *)YCTX, in, &submod), LY_EVALID);
    CHECK_LOG_CTX("Trailing garbage \"<submodule name...\" after submodule, expected end-of-input.", NULL, 8);
    lysp_module_free(&fctx, (struct lysp_module *)submod);
    lysp_yin_ctx_free(yin_ctx);
    ly_in_free(in, 0);
    yin_ctx = NULL;
    submod = NULL;
}

int
main(void)
{

    const struct CMUnitTest tests[] = {
        UTEST(test_yin_match_keyword, setup, teardown),
        UTEST(test_yin_parse_content, setup, teardown),
        UTEST(test_validate_value, setup, teardown),
        UTEST(test_valid_module),
        UTEST(test_print_module),
        UTEST(test_print_submodule),

        UTEST(test_yin_match_argument_name),
        cmocka_unit_test_setup_teardown(test_enum_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_bit_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_status_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_yin_element_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_yangversion_elem, setup, teardown),
        cmocka_unit_test_setup_teardown(test_argument_elem, setup, teardown),
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
        cmocka_unit_test_setup_teardown(test_uses_elem, setup, teardown),
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
