/**
 * @file test_yang.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief unit tests for YANG module parser and printer
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

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "in_internal.h"
#include "parser_internal.h"
#include "schema_compile.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_free.h"

/* originally static functions from parser_yang.c and parser_yin.c */
LY_ERR buf_add_char(struct ly_ctx *ctx, struct ly_in *in, size_t len, char **buf, size_t *buf_len, size_t *buf_used);
LY_ERR buf_store_char(struct lysp_yang_ctx *ctx, enum yang_arg arg, char **word_p,
        size_t *word_len, char **word_b, size_t *buf_len, uint8_t need_buf, uint8_t *prefix);
LY_ERR get_keyword(struct lysp_yang_ctx *ctx, enum ly_stmt *kw, char **word_p, size_t *word_len);
LY_ERR get_argument(struct lysp_yang_ctx *ctx, enum yang_arg arg,
        uint16_t *flags, char **word_p, char **word_b, size_t *word_len);
LY_ERR skip_comment(struct lysp_yang_ctx *ctx, uint8_t comment);

LY_ERR parse_action(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node_action **actions);
LY_ERR parse_any(struct lysp_yang_ctx *ctx, enum ly_stmt kw, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_augment(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node_augment **augments);
LY_ERR parse_case(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_container(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_deviate(struct lysp_yang_ctx *ctx, struct lysp_deviate **deviates);
LY_ERR parse_deviation(struct lysp_yang_ctx *ctx, struct lysp_deviation **deviations);
LY_ERR parse_grouping(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node_grp **groupings);
LY_ERR parse_choice(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_leaf(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_leaflist(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_list(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_maxelements(struct lysp_yang_ctx *ctx, uint32_t *max, uint16_t *flags, struct lysp_ext_instance **exts);
LY_ERR parse_minelements(struct lysp_yang_ctx *ctx, uint32_t *min, uint16_t *flags, struct lysp_ext_instance **exts);
LY_ERR parse_module(struct lysp_yang_ctx *ctx, struct lysp_module *mod);
LY_ERR parse_notif(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node_notif **notifs);
LY_ERR parse_submodule(struct lysp_yang_ctx *ctx, struct lysp_submodule *submod);
LY_ERR parse_uses(struct lysp_yang_ctx *ctx, struct lysp_node *parent, struct lysp_node **siblings);
LY_ERR parse_when(struct lysp_yang_ctx *ctx, struct lysp_when **when_p);
LY_ERR parse_type_enum_value_pos(struct lysp_yang_ctx *ctx, enum ly_stmt val_kw, struct lysp_type_enum *enm);

struct lysp_yang_ctx *YCTX;
struct lysf_ctx fctx;

struct ly_in in = {0};

#define YCTX_INIT \
    in.line = 1; \
    YCTX->in = &in; \
    LOG_LOCINIT(UTEST_LYCTX, NULL, NULL, NULL, &in)

static int
setup(void **state)
{
    struct lysp_module *pmod;

    UTEST_SETUP;

    /* allocate parser context */
    YCTX = calloc(1, sizeof(*YCTX));
    YCTX->main_ctx = (struct lysp_ctx *)YCTX;
    YCTX->format = LYS_IN_YANG;
    ly_set_new(&YCTX->parsed_mods);

    /* allocate new parsed module */
    pmod = calloc(1, sizeof *pmod);
    ly_set_add(YCTX->parsed_mods, pmod, 1, NULL);

    /* allocate new module */
    pmod->mod = calloc(1, sizeof *pmod->mod);
    pmod->mod->ctx = UTEST_LYCTX;
    pmod->mod->parsed = pmod;

    /* initilize and use the global easily available and customizable input handler */
    in.line = 1;
    YCTX->in = &in;
    LOG_LOCSET(NULL, NULL, NULL, &in);

    fctx.ctx = PARSER_CTX(YCTX);
    fctx.mod = pmod->mod;

    return 0;
}

static int
teardown(void **state)
{
    lys_module_free(&fctx, PARSER_CUR_PMOD(YCTX)->mod, 0);
    LOG_LOCBACK(0, 0, 0, 1);

    ly_set_free(YCTX->parsed_mods, NULL);
    ly_set_erase(&YCTX->ext_inst, NULL);
    free(YCTX);
    YCTX = NULL;

    lysf_ctx_erase(&fctx);

    UTEST_TEARDOWN;

    return 0;
}

#define TEST_DUP_GENERIC(PREFIX, MEMBER, VALUE1, VALUE2, FUNC, RESULT, LINE, CLEANUP) \
    in.current = PREFIX MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, FUNC(YCTX, RESULT)); \
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", "Line number "LINE".");\
    CLEANUP
static void
test_helpers(void **state)
{
    char *buf, *p;
    size_t len, size;
    uint8_t prefix = 0;

    /* storing into buffer */
    in.current = "abcd";
    buf = NULL;
    size = len = 0;
    assert_int_equal(LY_SUCCESS, buf_add_char(NULL, &in, 2, &buf, &size, &len));
    assert_int_not_equal(0, size);
    assert_int_equal(2, len);
    assert_string_equal("cd", in.current);
    assert_false(strncmp("ab", buf, 2));
    free(buf);
    buf = NULL;

    /* invalid first characters */
    len = 0;
    in.current = "2invalid";
    assert_int_equal(LY_EVALID, buf_store_char(YCTX, Y_IDENTIF_ARG, &p, &len, &buf, &size, 1, &prefix));
    in.current = ".invalid";
    assert_int_equal(LY_EVALID, buf_store_char(YCTX, Y_IDENTIF_ARG, &p, &len, &buf, &size, 1, &prefix));
    in.current = "-invalid";
    assert_int_equal(LY_EVALID, buf_store_char(YCTX, Y_IDENTIF_ARG, &p, &len, &buf, &size, 1, &prefix));
    /* invalid following characters */
    len = 3; /* number of characters read before the str content */
    in.current = "!";
    assert_int_equal(LY_EVALID, buf_store_char(YCTX, Y_IDENTIF_ARG, &p, &len, &buf, &size, 1, &prefix));
    in.current = ":";
    assert_int_equal(LY_EVALID, buf_store_char(YCTX, Y_IDENTIF_ARG, &p, &len, &buf, &size, 1, &prefix));
    UTEST_LOG_CTX_CLEAN;
    /* valid colon for prefixed identifiers */
    len = size = 0;
    p = NULL;
    prefix = 0;
    in.current = "x:id";
    assert_int_equal(LY_SUCCESS, buf_store_char(YCTX, Y_PREF_IDENTIF_ARG, &p, &len, &buf, &size, 0, &prefix));
    assert_int_equal(1, len);
    assert_null(buf);
    assert_string_equal(":id", in.current);
    assert_int_equal('x', p[len - 1]);
    assert_int_equal(LY_SUCCESS, buf_store_char(YCTX, Y_PREF_IDENTIF_ARG, &p, &len, &buf, &size, 1, &prefix));
    assert_int_equal(2, len);
    assert_string_equal("id", in.current);
    assert_int_equal(':', p[len - 1]);
    free(buf);
    prefix = 0;

    /* checking identifiers */
    assert_int_equal(LY_EVALID, lysp_check_identifierchar((struct lysp_ctx *)YCTX, ':', 0, NULL));
    CHECK_LOG_CTX("Invalid identifier character ':' (0x003a).", "Line number 1.");
    assert_int_equal(LY_EVALID, lysp_check_identifierchar((struct lysp_ctx *)YCTX, '#', 1, NULL));
    CHECK_LOG_CTX("Invalid identifier first character '#' (0x0023).", "Line number 1.");

    assert_int_equal(LY_SUCCESS, lysp_check_identifierchar((struct lysp_ctx *)YCTX, 'a', 1, &prefix));
    assert_int_equal(0, prefix);
    assert_int_equal(LY_SUCCESS, lysp_check_identifierchar((struct lysp_ctx *)YCTX, ':', 0, &prefix));
    assert_int_equal(1, prefix);
    assert_int_equal(LY_EVALID, lysp_check_identifierchar((struct lysp_ctx *)YCTX, ':', 0, &prefix));
    CHECK_LOG_CTX("Invalid identifier first character ':' (0x003a).", "Line number 1.");
    assert_int_equal(1, prefix);
    assert_int_equal(LY_SUCCESS, lysp_check_identifierchar((struct lysp_ctx *)YCTX, 'b', 0, &prefix));
    assert_int_equal(2, prefix);
    /* second colon is invalid */
    assert_int_equal(LY_EVALID, lysp_check_identifierchar((struct lysp_ctx *)YCTX, ':', 0, &prefix));
    CHECK_LOG_CTX("Invalid identifier character ':' (0x003a).", "Line number 1.");
}

#define TEST_GET_ARGUMENT_SUCCESS(INPUT_TEXT, CTX, ARG_TYPE, EXPECT_WORD, EXPECT_LEN, EXPECT_CURRENT, EXPECT_LINE)\
    {\
        const char * text = INPUT_TEXT;\
        in.line = 1;\
        in.current = text;\
        assert_int_equal(LY_SUCCESS, get_argument(CTX, Y_MAYBE_STR_ARG, NULL, &word, &buf, &len));\
        assert_string_equal(word, EXPECT_WORD);\
        assert_int_equal(len, EXPECT_LEN);\
        assert_string_equal(EXPECT_CURRENT, in.current);\
        assert_int_equal(EXPECT_LINE, in.line);\
    }

static void
test_comments(void **state)
{
    char *word, *buf;
    size_t len;

    TEST_GET_ARGUMENT_SUCCESS(" // this is a text of / one * line */ comment\nargument;",
            YCTX, Y_STR_ARG, "argument;", 8, ";", 2);
    assert_null(buf);

    TEST_GET_ARGUMENT_SUCCESS("/* this is a \n * text // of / block * comment */\"arg\" + \"ume\" \n + \n \"nt\";",
            YCTX, Y_STR_ARG, "argument", 8, ";", 4);
    assert_ptr_equal(buf, word);
    free(word);

    in.line = 1;
    in.current = " this is one line comment on last line";
    assert_int_equal(LY_SUCCESS, skip_comment(YCTX, 1));
    assert_true(in.current[0] == '\0');

    in.line = 1;
    in.current = " this is a not terminated comment x";
    assert_int_equal(LY_EVALID, skip_comment(YCTX, 2));
    CHECK_LOG_CTX("Unexpected end-of-input, non-terminated comment.", "Line number 1.");
    assert_true(in.current[0] == '\0');
}

static void
test_arg(void **state)
{
    char *word, *buf;
    size_t len;

    /* missing argument */
    in.current = ";";
    assert_int_equal(LY_SUCCESS, get_argument(YCTX, Y_MAYBE_STR_ARG, NULL, &word, &buf, &len));
    assert_null(word);

    in.current = "{";
    assert_int_equal(LY_EVALID, get_argument(YCTX, Y_STR_ARG, NULL, &word, &buf, &len));
    CHECK_LOG_CTX("Invalid character sequence \"{\", expected an argument.", "Line number 1.");

    /* invalid escape sequence */
    in.current = "\"\\s\"";
    assert_int_equal(LY_EVALID, get_argument(YCTX, Y_STR_ARG, NULL, &word, &buf, &len));
    CHECK_LOG_CTX("Double-quoted string unknown special character \'\\s\'.", "Line number 1.");

    TEST_GET_ARGUMENT_SUCCESS("\'\\s\'", YCTX, Y_STR_ARG, "\\s\'", 2, "", 1);

    /* invalid character after the argument */
    in.current = "hello\"";
    assert_int_equal(LY_EVALID, get_argument(YCTX, Y_STR_ARG, NULL, &word, &buf, &len));
    CHECK_LOG_CTX("Invalid character sequence \"\"\", expected unquoted string character, optsep, semicolon or opening brace.", "Line number 1.");

    in.current = "hello}";
    assert_int_equal(LY_EVALID, get_argument(YCTX, Y_STR_ARG, NULL, &word, &buf, &len));
    CHECK_LOG_CTX("Invalid character sequence \"}\", expected unquoted string character, optsep, semicolon or opening brace.", "Line number 1.");
    /* invalid identifier-ref-arg-str */
    in.current = "pre:pre:value";
    assert_int_equal(LY_EVALID, get_argument(YCTX, Y_PREF_IDENTIF_ARG, NULL, &word, &buf, &len));
    CHECK_LOG_CTX("Invalid identifier character ':' (0x003a).", "Line number 1.");

    in.current = "\"\";"; /* empty identifier is not allowed */
    assert_int_equal(LY_EVALID, get_argument(YCTX, Y_IDENTIF_ARG, NULL, &word, &buf, &len));
    CHECK_LOG_CTX("Statement argument is required.", "Line number 1.");

    in.current = "\"\";"; /* empty reference identifier is not allowed */
    assert_int_equal(LY_EVALID, get_argument(YCTX, Y_PREF_IDENTIF_ARG, NULL, &word, &buf, &len));
    CHECK_LOG_CTX("Statement argument is required.", "Line number 1.");

    /* slash is not an invalid character */
    TEST_GET_ARGUMENT_SUCCESS("hello/x\t", YCTX, Y_STR_ARG, "hello/x\t", 7, "\t", 1);
    assert_null(buf);

    /* different quoting */
    TEST_GET_ARGUMENT_SUCCESS("hello/x\t", YCTX, Y_STR_ARG, "hello/x\t", 7, "\t", 1);

    TEST_GET_ARGUMENT_SUCCESS("hello ", YCTX, Y_STR_ARG, "hello ", 5, " ", 1);

    TEST_GET_ARGUMENT_SUCCESS("hello/*comment*/\n", YCTX, Y_STR_ARG, "hello/*comment*/\n", 5, "\n", 1);

    TEST_GET_ARGUMENT_SUCCESS("\"hello\\n\\t\\\"\\\\\";", YCTX, Y_STR_ARG, "hello\n\t\"\\", 9, ";", 1);
    free(buf);

    YCTX->indent = 14;
    /* - space and tabs before newline are stripped out
     * - space and tabs after newline (indentation) are stripped out
     */
    TEST_GET_ARGUMENT_SUCCESS("\"hello \t\n\t\t world!\"", YCTX, Y_STR_ARG, "hello\n  world!", 14, "", 2);
    free(buf);

/* In contrast to previous, the backslash-escaped tabs are expanded after trimming, so they are preserved */
    YCTX->indent = 14;
    TEST_GET_ARGUMENT_SUCCESS("\"hello \\t\n\t\\t world!\"", YCTX, Y_STR_ARG, "hello \t\n\t world!", 16, "", 2);
    assert_ptr_equal(word, buf);
    free(buf);

    /* Do not handle whitespaces after backslash-escaped newline as indentation */
    YCTX->indent = 14;
    TEST_GET_ARGUMENT_SUCCESS("\"hello\\n\t\t world!\"", YCTX, Y_STR_ARG, "hello\n\t\t world!", 15, "", 1);
    assert_ptr_equal(word, buf);
    free(buf);

    YCTX->indent = 14;
    TEST_GET_ARGUMENT_SUCCESS("\"hello\n \tworld!\"", YCTX, Y_STR_ARG, "hello\nworld!", 12, "", 2);
    assert_ptr_equal(word, buf);
    free(buf);

    TEST_GET_ARGUMENT_SUCCESS("\'hello\'", YCTX, Y_STR_ARG, "hello'", 5, "", 1);

    TEST_GET_ARGUMENT_SUCCESS("\"hel\"  +\t\n\"lo\"", YCTX, Y_STR_ARG, "hello", 5, "", 2);
    assert_ptr_equal(word, buf);
    free(buf);

    in.line = 1;
    in.current = "\"hel\"  +\t\nlo"; /* unquoted the second part */
    assert_int_equal(LY_EVALID, get_argument(YCTX, Y_STR_ARG, NULL, &word, &buf, &len));
    CHECK_LOG_CTX("Both string parts divided by '+' must be quoted.", "Line number 2.");

    TEST_GET_ARGUMENT_SUCCESS("\'he\'\t\n+ \"llo\"", YCTX, Y_STR_ARG, "hello", 5, "", 2);
    free(buf);

    TEST_GET_ARGUMENT_SUCCESS(" \t\n\"he\"+\'llo\'", YCTX, Y_STR_ARG, "hello", 5, "", 2);
    free(buf);

    /* missing argument */
    in.line = 1;
    in.current = ";";
    assert_int_equal(LY_EVALID, get_argument(YCTX, Y_STR_ARG, NULL, &word, &buf, &len));
    CHECK_LOG_CTX("Invalid character sequence \";\", expected an argument.", "Line number 1.");
}

#define TEST_STMS_SUCCESS(INPUT_TEXT, CTX, ACTION, EXPECT_WORD)\
                   in.current = INPUT_TEXT;\
                   assert_int_equal(LY_SUCCESS, get_keyword(CTX, &kw, &word, &len));\
                   assert_int_equal(ACTION, kw);\
                   assert_int_equal(strlen(EXPECT_WORD), len);\
                   assert_true(0 == strncmp(EXPECT_WORD, word, len))

static void
test_stmts(void **state)
{
    const char *p;
    enum ly_stmt kw;
    char *word;
    size_t len;

    in.current = "\n// comment\n\tinput\t{";
    assert_int_equal(LY_SUCCESS, get_keyword(YCTX, &kw, &word, &len));
    assert_int_equal(LY_STMT_INPUT, kw);
    assert_int_equal(5, len);
    assert_string_equal("input\t{", word);
    assert_string_equal("\t{", in.current);

    in.current = "\t /* comment */\t output\n\t{";
    assert_int_equal(LY_SUCCESS, get_keyword(YCTX, &kw, &word, &len));
    assert_int_equal(LY_STMT_OUTPUT, kw);
    assert_int_equal(6, len);
    assert_string_equal("output\n\t{", word);
    assert_string_equal("\n\t{", in.current);
    assert_int_equal(LY_SUCCESS, get_keyword(YCTX, &kw, &word, &len));
    assert_int_equal(LY_STMT_SYNTAX_LEFT_BRACE, kw);
    assert_int_equal(1, len);
    assert_string_equal("{", word);
    assert_string_equal("", in.current);

    in.current = "/input { "; /* invalid slash */
    assert_int_equal(LY_EVALID, get_keyword(YCTX, &kw, &word, &len));
    CHECK_LOG_CTX("Invalid identifier first character '/'.", "Line number 4.");

    in.current = "not-a-statement-nor-extension { "; /* invalid identifier */
    assert_int_equal(LY_EVALID, get_keyword(YCTX, &kw, &word, &len));
    CHECK_LOG_CTX("Invalid character sequence \"not-a-statement-nor-extension\", expected a keyword.", "Line number 4.");

    in.current = "path;"; /* missing sep after the keyword */
    assert_int_equal(LY_EVALID, get_keyword(YCTX, &kw, &word, &len));
    CHECK_LOG_CTX("Invalid character sequence \"path;\", expected a keyword followed by a separator.", "Line number 4.");

    TEST_STMS_SUCCESS("action ", YCTX, LY_STMT_ACTION, "action");

    TEST_STMS_SUCCESS("anydata ", YCTX, LY_STMT_ANYDATA, "anydata");
    TEST_STMS_SUCCESS("anyxml ", YCTX, LY_STMT_ANYXML, "anyxml");
    TEST_STMS_SUCCESS("argument ", YCTX, LY_STMT_ARGUMENT, "argument");
    TEST_STMS_SUCCESS("augment ", YCTX, LY_STMT_AUGMENT, "augment");
    TEST_STMS_SUCCESS("base ", YCTX, LY_STMT_BASE, "base");
    TEST_STMS_SUCCESS("belongs-to ", YCTX, LY_STMT_BELONGS_TO, "belongs-to");
    TEST_STMS_SUCCESS("bit ", YCTX, LY_STMT_BIT, "bit");
    TEST_STMS_SUCCESS("case ", YCTX, LY_STMT_CASE, "case");
    TEST_STMS_SUCCESS("choice ", YCTX, LY_STMT_CHOICE, "choice");
    TEST_STMS_SUCCESS("config ", YCTX, LY_STMT_CONFIG, "config");
    TEST_STMS_SUCCESS("contact ", YCTX, LY_STMT_CONTACT, "contact");
    TEST_STMS_SUCCESS("container ", YCTX, LY_STMT_CONTAINER, "container");
    TEST_STMS_SUCCESS("default ", YCTX, LY_STMT_DEFAULT, "default");
    TEST_STMS_SUCCESS("description ", YCTX, LY_STMT_DESCRIPTION, "description");
    TEST_STMS_SUCCESS("deviate ", YCTX, LY_STMT_DEVIATE, "deviate");
    TEST_STMS_SUCCESS("deviation ", YCTX, LY_STMT_DEVIATION, "deviation");
    TEST_STMS_SUCCESS("enum ", YCTX, LY_STMT_ENUM, "enum");
    TEST_STMS_SUCCESS("error-app-tag ", YCTX, LY_STMT_ERROR_APP_TAG, "error-app-tag");
    TEST_STMS_SUCCESS("error-message ", YCTX, LY_STMT_ERROR_MESSAGE, "error-message");
    TEST_STMS_SUCCESS("extension ", YCTX, LY_STMT_EXTENSION, "extension");
    TEST_STMS_SUCCESS("feature ", YCTX, LY_STMT_FEATURE, "feature");
    TEST_STMS_SUCCESS("fraction-digits ", YCTX, LY_STMT_FRACTION_DIGITS, "fraction-digits");
    TEST_STMS_SUCCESS("grouping ", YCTX, LY_STMT_GROUPING, "grouping");
    TEST_STMS_SUCCESS("identity ", YCTX, LY_STMT_IDENTITY, "identity");
    TEST_STMS_SUCCESS("if-feature ", YCTX, LY_STMT_IF_FEATURE, "if-feature");
    TEST_STMS_SUCCESS("import ", YCTX, LY_STMT_IMPORT, "import");
    TEST_STMS_SUCCESS("include ", YCTX, LY_STMT_INCLUDE, "include");
    TEST_STMS_SUCCESS("input{", YCTX, LY_STMT_INPUT, "input");
    TEST_STMS_SUCCESS("key ", YCTX, LY_STMT_KEY, "key");
    TEST_STMS_SUCCESS("leaf ", YCTX, LY_STMT_LEAF, "leaf");
    TEST_STMS_SUCCESS("leaf-list ", YCTX, LY_STMT_LEAF_LIST, "leaf-list");
    TEST_STMS_SUCCESS("length ", YCTX, LY_STMT_LENGTH, "length");
    TEST_STMS_SUCCESS("list ", YCTX, LY_STMT_LIST, "list");
    TEST_STMS_SUCCESS("mandatory ", YCTX, LY_STMT_MANDATORY, "mandatory");
    TEST_STMS_SUCCESS("max-elements ", YCTX, LY_STMT_MAX_ELEMENTS, "max-elements");
    TEST_STMS_SUCCESS("min-elements ", YCTX, LY_STMT_MIN_ELEMENTS, "min-elements");
    TEST_STMS_SUCCESS("modifier ", YCTX, LY_STMT_MODIFIER, "modifier");
    TEST_STMS_SUCCESS("module ", YCTX, LY_STMT_MODULE, "module");
    TEST_STMS_SUCCESS("must ", YCTX, LY_STMT_MUST, "must");
    TEST_STMS_SUCCESS("namespace ", YCTX, LY_STMT_NAMESPACE, "namespace");
    TEST_STMS_SUCCESS("notification ", YCTX, LY_STMT_NOTIFICATION, "notification");
    TEST_STMS_SUCCESS("ordered-by ", YCTX, LY_STMT_ORDERED_BY, "ordered-by");
    TEST_STMS_SUCCESS("organization ", YCTX, LY_STMT_ORGANIZATION, "organization");
    TEST_STMS_SUCCESS("output ", YCTX, LY_STMT_OUTPUT, "output");
    TEST_STMS_SUCCESS("path ", YCTX, LY_STMT_PATH, "path");
    TEST_STMS_SUCCESS("pattern ", YCTX, LY_STMT_PATTERN, "pattern");
    TEST_STMS_SUCCESS("position ", YCTX, LY_STMT_POSITION, "position");
    TEST_STMS_SUCCESS("prefix ", YCTX, LY_STMT_PREFIX, "prefix");
    TEST_STMS_SUCCESS("presence ", YCTX, LY_STMT_PRESENCE, "presence");
    TEST_STMS_SUCCESS("range ", YCTX, LY_STMT_RANGE, "range");
    TEST_STMS_SUCCESS("reference ", YCTX, LY_STMT_REFERENCE, "reference");
    TEST_STMS_SUCCESS("refine ", YCTX, LY_STMT_REFINE, "refine");
    TEST_STMS_SUCCESS("require-instance ", YCTX, LY_STMT_REQUIRE_INSTANCE, "require-instance");
    TEST_STMS_SUCCESS("revision ", YCTX, LY_STMT_REVISION, "revision");
    TEST_STMS_SUCCESS("revision-date ", YCTX, LY_STMT_REVISION_DATE, "revision-date");
    TEST_STMS_SUCCESS("rpc ", YCTX, LY_STMT_RPC, "rpc");
    TEST_STMS_SUCCESS("status ", YCTX, LY_STMT_STATUS, "status");
    TEST_STMS_SUCCESS("submodule ", YCTX, LY_STMT_SUBMODULE, "submodule");
    TEST_STMS_SUCCESS("type ", YCTX, LY_STMT_TYPE, "type");
    TEST_STMS_SUCCESS("typedef ", YCTX, LY_STMT_TYPEDEF, "typedef");
    TEST_STMS_SUCCESS("unique ", YCTX, LY_STMT_UNIQUE, "unique");
    TEST_STMS_SUCCESS("units ", YCTX, LY_STMT_UNITS, "units");
    TEST_STMS_SUCCESS("uses ", YCTX, LY_STMT_USES, "uses");
    TEST_STMS_SUCCESS("value ", YCTX, LY_STMT_VALUE, "value");
    TEST_STMS_SUCCESS("when ", YCTX, LY_STMT_WHEN, "when");
    TEST_STMS_SUCCESS("yang-version ", YCTX, LY_STMT_YANG_VERSION, "yang-version");
    TEST_STMS_SUCCESS("yin-element ", YCTX, LY_STMT_YIN_ELEMENT, "yin-element");
    TEST_STMS_SUCCESS(";config false;", YCTX, LY_STMT_SYNTAX_SEMICOLON, ";");
    assert_string_equal("config false;", in.current);
    TEST_STMS_SUCCESS("{ config false;", YCTX, LY_STMT_SYNTAX_LEFT_BRACE, "{");
    assert_string_equal(" config false;", in.current);
    TEST_STMS_SUCCESS("}", YCTX, LY_STMT_SYNTAX_RIGHT_BRACE, "}");
    assert_string_equal("", in.current);

    /* geenric extension */
    in.current = p = "nacm:default-deny-write;";
    assert_int_equal(LY_SUCCESS, get_keyword(YCTX, &kw, &word, &len));
    assert_int_equal(LY_STMT_EXTENSION_INSTANCE, kw);
    assert_int_equal(23, len);
    assert_ptr_equal(p, word);
}

static void
test_minmax(void **state)
{
    uint16_t flags = 0;
    uint32_t value = 0;
    struct lysp_ext_instance *ext = NULL;

    PARSER_CUR_PMOD(YCTX)->version = 2; /* simulate YANG 1.1 */

    in.current = " 1invalid; ...";
    assert_int_equal(LY_EVALID, parse_minelements(YCTX, &value, &flags, &ext));
    CHECK_LOG_CTX("Invalid value \"1invalid\" of \"min-elements\".", "Line number 1.");

    flags = value = 0;
    in.current = " -1; ...";
    assert_int_equal(LY_EVALID, parse_minelements(YCTX, &value, &flags, &ext));
    CHECK_LOG_CTX("Invalid value \"-1\" of \"min-elements\".", "Line number 1.");

    /* implementation limit */
    flags = value = 0;
    in.current = " 4294967296; ...";
    assert_int_equal(LY_EVALID, parse_minelements(YCTX, &value, &flags, &ext));
    CHECK_LOG_CTX("Value \"4294967296\" is out of \"min-elements\" bounds.", "Line number 1.");

    flags = value = 0;
    in.current = " 1 {config true;} ...";
    assert_int_equal(LY_EVALID, parse_minelements(YCTX, &value, &flags, &ext));
    CHECK_LOG_CTX("Invalid keyword \"config\" as a child of \"min-elements\".", "Line number 1.");

    in.current = " 1invalid; ...";
    assert_int_equal(LY_EVALID, parse_maxelements(YCTX, &value, &flags, &ext));
    CHECK_LOG_CTX("Invalid value \"1invalid\" of \"max-elements\".", "Line number 1.");

    flags = value = 0;
    in.current = " -1; ...";
    assert_int_equal(LY_EVALID, parse_maxelements(YCTX, &value, &flags, &ext));
    CHECK_LOG_CTX("Invalid value \"-1\" of \"max-elements\".", "Line number 1.");

    /* implementation limit */
    flags = value = 0;
    in.current = " 4294967296; ...";
    assert_int_equal(LY_EVALID, parse_maxelements(YCTX, &value, &flags, &ext));
    CHECK_LOG_CTX("Value \"4294967296\" is out of \"max-elements\" bounds.", "Line number 1.");

    flags = value = 0;
    in.current = " 1 {config true;} ...";
    assert_int_equal(LY_EVALID, parse_maxelements(YCTX, &value, &flags, &ext));
    CHECK_LOG_CTX("Invalid keyword \"config\" as a child of \"max-elements\".", "Line number 1.");
}

static void
test_valid_module(void **state)
{
    struct lys_module *mod;
    char *printed;
    const char *links_yang =
            "module links {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:module2\";\n"
            "  prefix mod2;\n"
            "\n"
            "  identity just-another-identity;\n"
            "\n"
            "  leaf one-leaf {\n"
            "    type string;\n"
            "  }\n"
            "\n"
            "  list list-for-augment {\n"
            "    key keyleaf;\n"
            "\n"
            "    leaf keyleaf {\n"
            "      type string;\n"
            "    }\n"
            "\n"
            "    leaf just-leaf {\n"
            "      type int32;\n"
            "    }\n"
            "  }\n"
            "\n"
            "  leaf rleaf {\n"
            "    type string;\n"
            "  }\n"
            "\n"
            "  leaf-list llist {\n"
            "    type string;\n"
            "    min-elements 0;\n"
            "    max-elements 100;\n"
            "    ordered-by user;\n"
            "  }\n"
            "\n"
            "  grouping rgroup {\n"
            "    leaf rg1 {\n"
            "      type string;\n"
            "    }\n"
            "\n"
            "    leaf rg2 {\n"
            "      type string;\n"
            "    }\n"
            "  }\n"
            "}\n";
    const char *statements_yang =
            "module statements {\n"
            "  yang-version 1.1;\n"
            "  namespace \"urn:module\";\n"
            "  prefix mod;\n"
            "\n"
            "  import links {\n"
            "    prefix mod2;\n"
            "  }\n"
            "\n"
            "  extension ext;\n"
            "\n"
            "  identity random-identity {\n"
            "    base mod2:just-another-identity;\n"
            "    base another-identity;\n"
            "  }\n"
            "\n"
            "  identity another-identity {\n"
            "    base mod2:just-another-identity;\n"
            "  }\n"
            "\n"
            "  typedef percent {\n"
            "    type uint8 {\n"
            "      range \"0 .. 100\";\n"
            "    }\n"
            "    units \"percent\";\n"
            "  }\n"
            "\n"
            "  list list1 {\n"
            "    key \"a\";\n"
            "    leaf a {\n"
            "      type string;\n"
            "    }\n"
            "    leaf x {\n"
            "      type string;\n"
            "    }\n"
            "    leaf y {\n"
            "      type string;\n"
            "    }\n"
            "  }\n"
            "  container ice-cream-shop {\n"
            "    container employees {\n"
            "      when \"/list1/x\";\n"
            "      list employee {\n"
            "        key \"id\";\n"
            "        unique \"name\";\n"
            "        config true;\n"
            "        min-elements 0 {\n"
            "          mod:ext;\n"
            "        }\n"
            "        max-elements unbounded;\n"
            "        leaf id {\n"
            "          type uint64;\n"
            "          mandatory true;\n"
            "        }\n"
            "        leaf name {\n"
            "          type string;\n"
            "        }\n"
            "        leaf age {\n"
            "          type uint32;\n"
            "        }\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "  container random {\n"
            "    grouping group {\n"
            "      leaf g1 {\n"
            "        type percent;\n"
            "        mandatory false;\n"
            "      }\n"
            "      leaf g2 {\n"
            "        type string;\n"
            "      }\n"
            "    }\n"
            "    choice switch {\n"
            "      case a {\n"
            "        leaf aleaf {\n"
            "          type string;\n"
            "          default \"aaa\";\n"
            "        }\n"
            "      }\n"
            "      case c {\n"
            "        leaf cleaf {\n"
            "          type string;\n"
            "        }\n"
            "      }\n"
            "    }\n"
            "    anyxml xml-data;\n"
            "    anydata any-data;\n"
            "    leaf-list leaflist {\n"
            "      type string;\n"
            "      min-elements 0;\n"
            "      max-elements 20;\n"
            "    }\n"
            "    uses group;\n"
            "    uses mod2:rgroup;\n"
            "    leaf lref {\n"
            "      type leafref {\n"
            "        path \"/mod2:one-leaf\";\n"
            "      }\n"
            "    }\n"
            "    leaf iref {\n"
            "      type identityref {\n"
            "        base mod2:just-another-identity;\n"
            "      }\n"
            "    }\n"
            "  }\n"
            "\n"
            "  augment \"/random\" {\n"
            "    leaf aug-leaf {\n"
            "      type string;\n"
            "    }\n"
            "  }\n"
            "\n"
            "  notification notif;\n"
            "\n"
            "  deviation \"/mod:ice-cream-shop/mod:employees/mod:employee/mod:age\" {\n"
            "    deviate not-supported {\n"
            "      mod:ext;\n"
            "    }\n"
            "  }\n"
            "  deviation \"/mod:list1\" {\n"
            "    deviate add {\n"
            "      mod:ext;\n"
            "      must \"1\";\n"
            "      must \"2\";\n"
            "      unique \"x\";\n"
            "      unique \"y\";\n"
            "      config true;\n"
            "      min-elements 1;\n"
            "      max-elements 2;\n"
            "    }\n"
            "  }\n"
            "  deviation \"/mod:ice-cream-shop/mod:employees/mod:employee\" {\n"
            "    deviate delete {\n"
            "      unique \"name\";\n"
            "    }\n"
            "  }\n"
            "  deviation \"/mod:random/mod:leaflist\" {\n"
            "    deviate replace {\n"
            "      type uint32;\n"
            "      min-elements 10;\n"
            "      max-elements 15;\n"
            "    }\n"
            "  }\n"
            "}\n";

    UTEST_ADD_MODULE(links_yang, LYS_IN_YANG, NULL, NULL);
    UTEST_ADD_MODULE(statements_yang, LYS_IN_YANG, NULL, &mod);
    lys_print_mem(&printed, mod, LYS_OUT_YANG, 0);
    assert_string_equal(printed, statements_yang);
    free(printed);
}

static struct lysp_module *
mod_renew(struct lysp_yang_ctx *ctx)
{
    struct ly_ctx *ly_ctx = PARSER_CUR_PMOD(ctx)->mod->ctx;
    struct lysp_module *pmod;

    lys_module_free(&fctx, PARSER_CUR_PMOD(ctx)->mod, 0);
    pmod = calloc(1, sizeof *pmod);
    ctx->parsed_mods->objs[0] = pmod;
    pmod->mod = calloc(1, sizeof *pmod->mod);
    pmod->mod->parsed = pmod;
    pmod->mod->ctx = ly_ctx;

    ctx->in->line = 1;
    fctx.mod = pmod->mod;

    return pmod;
}

static struct lysp_submodule *
submod_renew(struct lysp_yang_ctx *ctx)
{
    struct ly_ctx *ly_ctx = PARSER_CUR_PMOD(ctx)->mod->ctx;
    struct lysp_submodule *submod;

    lys_module_free(&fctx, PARSER_CUR_PMOD(ctx)->mod, 0);
    submod = calloc(1, sizeof *submod);
    ctx->parsed_mods->objs[0] = submod;
    submod->mod = calloc(1, sizeof *submod->mod);
    lydict_insert(ly_ctx, "name", 0, &submod->mod->name);
    submod->mod->parsed = (struct lysp_module *)submod;
    submod->mod->ctx = ly_ctx;

    fctx.mod = submod->mod;

    return submod;
}

static LY_ERR
test_imp_clb(const char *UNUSED(mod_name), const char *UNUSED(mod_rev), const char *UNUSED(submod_name),
        const char *UNUSED(sub_rev), void *user_data, LYS_INFORMAT *format,
        const char **module_data, void (**free_module_data)(void *model_data, void *user_data))
{
    *module_data = user_data;
    *format = LYS_IN_YANG;
    *free_module_data = NULL;
    return LY_SUCCESS;
}

static void
test_module(void **state)
{
    struct lysp_module *mod = NULL;
    struct lysp_submodule *submod = NULL;
    struct lys_module *m;
    struct lysp_yang_ctx *ctx_p;

    mod = mod_renew(YCTX);

    /* missing mandatory substatements */
    in.current = " name {}";
    assert_int_equal(LY_EVALID, parse_module(YCTX, mod));
    assert_string_equal("name", mod->mod->name);
    CHECK_LOG_CTX("Missing mandatory keyword \"namespace\" as a child of \"module\".", "Line number 1.");

    mod = mod_renew(YCTX);
    in.current = " name {namespace urn:name;}";
    assert_int_equal(LY_EVALID, parse_module(YCTX, mod));
    assert_string_equal("urn:name", mod->mod->ns);
    CHECK_LOG_CTX("Missing mandatory keyword \"prefix\" as a child of \"module\".", "Line number 1.");
    mod = mod_renew(YCTX);

    in.current = " name {namespace urn:name;prefix \"n\";}";
    assert_int_equal(LY_SUCCESS, parse_module(YCTX, mod));
    assert_string_equal("n", mod->mod->prefix);
    mod = mod_renew(YCTX);

#define SCHEMA_BEGINNING " name {yang-version 1.1;namespace urn:x;prefix \"x\";"
#define SCHEMA_BEGINNING2 " name {namespace urn:x;prefix \"x\";"
#define TEST_NODE(NODETYPE, INPUT, NAME) \
        in.current = SCHEMA_BEGINNING INPUT; \
        assert_int_equal(LY_SUCCESS, parse_module(YCTX, mod)); \
        assert_non_null(mod->data); \
        assert_int_equal(NODETYPE, mod->data->nodetype); \
        assert_string_equal(NAME, mod->data->name); \
        mod = mod_renew(YCTX);
#define TEST_GENERIC(INPUT, TARGET, TEST) \
        in.current = SCHEMA_BEGINNING INPUT; \
        assert_int_equal(LY_SUCCESS, parse_module(YCTX, mod)); \
        assert_non_null(TARGET); \
        TEST; \
        mod = mod_renew(YCTX);
#define TEST_DUP(MEMBER, VALUE1, VALUE2, LINE) \
        TEST_DUP_GENERIC(SCHEMA_BEGINNING, MEMBER, VALUE1, VALUE2, \
                         parse_module, mod, LINE, mod = mod_renew(YCTX))

    /* duplicated namespace, prefix */
    TEST_DUP("namespace", "y", "z", "1");
    TEST_DUP("prefix", "y", "z", "1");
    TEST_DUP("contact", "a", "b", "1");
    TEST_DUP("description", "a", "b", "1");
    TEST_DUP("organization", "a", "b", "1");
    TEST_DUP("reference", "a", "b", "1");

    /* not allowed in module (submodule-specific) */
    in.current = SCHEMA_BEGINNING "belongs-to master {prefix m;}}";
    assert_int_equal(LY_EVALID, parse_module(YCTX, mod));
    CHECK_LOG_CTX("Invalid keyword \"belongs-to\" as a child of \"module\".", "Line number 1.");
    mod = mod_renew(YCTX);

    /* anydata */
    TEST_NODE(LYS_ANYDATA, "anydata test;}", "test");
    /* anyxml */
    TEST_NODE(LYS_ANYXML, "anyxml test;}", "test");
    /* augment */
    TEST_GENERIC("augment /somepath;}", mod->augments,
            assert_string_equal("/somepath", mod->augments[0].nodeid));
    /* choice */
    TEST_NODE(LYS_CHOICE, "choice test;}", "test");
    /* contact 0..1 */
    TEST_GENERIC("contact \"firstname\" + \n\t\" surname\";}", mod->mod->contact,
            assert_string_equal("firstname surname", mod->mod->contact));
    /* container */
    TEST_NODE(LYS_CONTAINER, "container test;}", "test");
    /* description 0..1 */
    TEST_GENERIC("description \'some description\';}", mod->mod->dsc,
            assert_string_equal("some description", mod->mod->dsc));
    /* deviation */
    TEST_GENERIC("deviation /somepath {deviate not-supported;}}", mod->deviations,
            assert_string_equal("/somepath", mod->deviations[0].nodeid));
    /* extension */
    TEST_GENERIC("extension test;}", mod->extensions,
            assert_string_equal("test", mod->extensions[0].name));
    /* feature */
    TEST_GENERIC("feature test;}", mod->features,
            assert_string_equal("test", mod->features[0].name));
    /* grouping */
    TEST_GENERIC("grouping grp;}", mod->groupings,
            assert_string_equal("grp", mod->groupings[0].name));
    /* identity */
    TEST_GENERIC("identity test;}", mod->identities,
            assert_string_equal("test", mod->identities[0].name));
    /* import */
    ly_ctx_set_module_imp_clb(PARSER_CUR_PMOD(YCTX)->mod->ctx, test_imp_clb, "module zzz { namespace urn:zzz; prefix z;}");
    TEST_GENERIC("import zzz {prefix z;}}", mod->imports,
            assert_string_equal("zzz", mod->imports[0].name));

    /* import - prefix collision */
    in.current = SCHEMA_BEGINNING "import zzz {prefix x;}}";
    assert_int_equal(LY_EVALID, parse_module(YCTX, mod));
    CHECK_LOG_CTX("Prefix \"x\" already used as module prefix.", "Line number 1.");
    mod = mod_renew(YCTX);

    in.current = SCHEMA_BEGINNING "import zzz {prefix y;}import zzz {prefix y;}}";
    assert_int_equal(LY_EVALID, parse_module(YCTX, mod));
    CHECK_LOG_CTX("Prefix \"y\" already used to import \"zzz\" module.", "Line number 1.");

    mod = mod_renew(YCTX);
    LOG_LOCBACK(0, 0, 0, 1);

    in.current = "module name10 {yang-version 1.1;namespace urn:name10;prefix \"n10\";import zzz {prefix y;}import zzz {prefix z;}}";
    assert_int_equal(lys_parse_mem(PARSER_CUR_PMOD(YCTX)->mod->ctx, in.current, LYS_IN_YANG, NULL), LY_SUCCESS);
    CHECK_LOG_CTX("Single revision of the module \"zzz\" imported twice.", NULL);

    /* include */
    ly_ctx_set_module_imp_clb(PARSER_CUR_PMOD(YCTX)->mod->ctx, test_imp_clb, "module xxx { namespace urn:xxx; prefix x;}");
    in.current = "module" SCHEMA_BEGINNING "include xxx;}";
    assert_int_equal(lys_parse_mem(PARSER_CUR_PMOD(YCTX)->mod->ctx, in.current, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"name\" failed.", NULL);
    CHECK_LOG_CTX("Including \"xxx\" submodule into \"name\" failed.", NULL);
    CHECK_LOG_CTX("Data model \"xxx\" not found in local searchdirs.", NULL);
    CHECK_LOG_CTX("Parsing submodule failed.", NULL);
    CHECK_LOG_CTX("Input data contains module in situation when a submodule is expected.", NULL);

    ly_ctx_set_module_imp_clb(PARSER_CUR_PMOD(YCTX)->mod->ctx, test_imp_clb, "submodule xxx {belongs-to wrong-name {prefix w;}}");
    in.current = "module" SCHEMA_BEGINNING "include xxx;}";
    assert_int_equal(lys_parse_mem(PARSER_CUR_PMOD(YCTX)->mod->ctx, in.current, LYS_IN_YANG, NULL), LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"name\" failed.", NULL);
    CHECK_LOG_CTX("Including \"xxx\" submodule into \"name\" failed.", NULL);
    UTEST_LOG_CTX_CLEAN;

    ly_ctx_set_module_imp_clb(PARSER_CUR_PMOD(YCTX)->mod->ctx, test_imp_clb, "submodule xxx {belongs-to name {prefix x;}}");
    TEST_GENERIC("include xxx;}", mod->includes,
            assert_string_equal("xxx", mod->includes[0].name));

    /* leaf */
    TEST_NODE(LYS_LEAF, "leaf test {type string;}}", "test");
    /* leaf-list */
    TEST_NODE(LYS_LEAFLIST, "leaf-list test {type string;}}", "test");
    /* list */
    TEST_NODE(LYS_LIST, "list test {key a;leaf a {type string;}}}", "test");
    /* notification */
    TEST_GENERIC("notification test;}", mod->notifs,
            assert_string_equal("test", mod->notifs[0].name));
    /* organization 0..1 */
    TEST_GENERIC("organization \"CESNET a.l.e.\";}", mod->mod->org,
            assert_string_equal("CESNET a.l.e.", mod->mod->org));
    /* reference 0..1 */
    TEST_GENERIC("reference RFC7950;}", mod->mod->ref,
            assert_string_equal("RFC7950", mod->mod->ref));
    /* revision */
    TEST_GENERIC("revision 2018-10-12;}", mod->revs,
            assert_string_equal("2018-10-12", mod->revs[0].date));
    /* rpc */
    TEST_GENERIC("rpc test;}", mod->rpcs,
            assert_string_equal("test", mod->rpcs[0].name));
    /* typedef */
    TEST_GENERIC("typedef test{type string;}}", mod->typedefs,
            assert_string_equal("test", mod->typedefs[0].name));
    /* uses */
    TEST_NODE(LYS_USES, "uses test;}", "test");
    /* yang-version */
    in.current = SCHEMA_BEGINNING2 "\n\tyang-version 10;}";
    assert_int_equal(LY_EVALID, parse_module(YCTX, mod));
    CHECK_LOG_CTX("Invalid value \"10\" of \"yang-version\".", NULL);
    mod = mod_renew(YCTX);
    in.current = SCHEMA_BEGINNING2 "yang-version 1;yang-version 1.1;}";
    assert_int_equal(LY_EVALID, parse_module(YCTX, mod));
    CHECK_LOG_CTX("Duplicate keyword \"yang-version\".", NULL);
    mod = mod_renew(YCTX);
    in.current = SCHEMA_BEGINNING2 "yang-version 1;}";
    assert_int_equal(LY_SUCCESS, parse_module(YCTX, mod));
    assert_int_equal(1, mod->version);
    mod = mod_renew(YCTX);
    in.current = SCHEMA_BEGINNING2 "yang-version \"1.1\";}";
    assert_int_equal(LY_SUCCESS, parse_module(YCTX, mod));
    assert_int_equal(2, mod->version);
    mod = mod_renew(YCTX);

    in.current = "module " SCHEMA_BEGINNING "} module q {namespace urn:q;prefixq;}";
    m = calloc(1, sizeof *m);
    m->ctx = PARSER_CUR_PMOD(YCTX)->mod->ctx;
    assert_int_equal(LY_EVALID, yang_parse_module(&ctx_p, &in, m));
    CHECK_LOG_CTX("Trailing garbage \"module q {names...\" after module, expected end-of-input.", "Line number 1.");
    lysp_yang_ctx_free(ctx_p);
    lys_module_free(&fctx, m, 0);

    in.current = "prefix " SCHEMA_BEGINNING "}";
    m = calloc(1, sizeof *m);
    m->ctx = PARSER_CUR_PMOD(YCTX)->mod->ctx;
    assert_int_equal(LY_EVALID, yang_parse_module(&ctx_p, &in, m));
    CHECK_LOG_CTX("Invalid keyword \"prefix\", expected \"module\" or \"submodule\".", "Line number 1.");
    lysp_yang_ctx_free(ctx_p);
    lys_module_free(&fctx, m, 0);

    in.current = "module " SCHEMA_BEGINNING "leaf enum {type enumeration {enum seven { position 7;}}}}";
    m = calloc(1, sizeof *m);
    m->ctx = PARSER_CUR_PMOD(YCTX)->mod->ctx;
    assert_int_equal(LY_EVALID, yang_parse_module(&ctx_p, &in, m));
    CHECK_LOG_CTX("Invalid keyword \"position\" as a child of \"enum\".", "Line number 1.");
    lysp_yang_ctx_free(ctx_p);
    lys_module_free(&fctx, m, 0);

    /* extensions */
    TEST_GENERIC("prefix:test;}", mod->exts,
            assert_string_equal("prefix:test", mod->exts[0].name);
            assert_int_equal(LY_STMT_MODULE, mod->exts[0].parent_stmt));
    mod = mod_renew(YCTX);

    /* invalid substatement */
    in.current = SCHEMA_BEGINNING "must false;}";
    assert_int_equal(LY_EVALID, parse_module(YCTX, mod));
    CHECK_LOG_CTX("Invalid keyword \"must\" as a child of \"module\".", NULL);

    /* submodule */
    submod = submod_renew(YCTX);

    /* missing mandatory substatements */
    in.current = " subname {}";
    assert_int_equal(LY_EVALID, parse_submodule(YCTX, submod));
    CHECK_LOG_CTX("Missing mandatory keyword \"belongs-to\" as a child of \"submodule\".", NULL);
    assert_string_equal("subname", submod->name);

    submod = submod_renew(YCTX);

    in.current = " subname {belongs-to name {prefix x;}}";
    assert_int_equal(LY_SUCCESS, parse_submodule(YCTX, submod));
    assert_string_equal("name", submod->mod->name);
    submod = submod_renew(YCTX);

#undef SCHEMA_BEGINNING
#define SCHEMA_BEGINNING " subname {belongs-to name {prefix x;}"

    /* duplicated namespace, prefix */
    in.current = " subname {belongs-to name {prefix x;}belongs-to module1;belongs-to module2;} ...";
    assert_int_equal(LY_EVALID, parse_submodule(YCTX, submod));
    CHECK_LOG_CTX("Duplicate keyword \"belongs-to\".", NULL);
    submod = submod_renew(YCTX);

    /* not allowed in submodule (module-specific) */
    in.current = SCHEMA_BEGINNING "namespace \"urn:z\";}";
    assert_int_equal(LY_EVALID, parse_submodule(YCTX, submod));
    CHECK_LOG_CTX("Invalid keyword \"namespace\" as a child of \"submodule\".", NULL);
    submod = submod_renew(YCTX);
    in.current = SCHEMA_BEGINNING "prefix m;}}";
    assert_int_equal(LY_EVALID, parse_submodule(YCTX, submod));
    CHECK_LOG_CTX("Invalid keyword \"prefix\" as a child of \"submodule\".", NULL);
    submod = submod_renew(YCTX);

    in.current = "submodule " SCHEMA_BEGINNING "} module q {namespace urn:q;prefixq;}";
    assert_int_equal(LY_EVALID, yang_parse_submodule(&ctx_p, PARSER_CUR_PMOD(YCTX)->mod->ctx, (struct lysp_ctx *)YCTX, YCTX->in, &submod));
    CHECK_LOG_CTX("Trailing garbage \"module q {names...\" after submodule, expected end-of-input.", "Line number 1.");
    lysp_yang_ctx_free(ctx_p);

    in.current = "prefix " SCHEMA_BEGINNING "}";
    assert_int_equal(LY_EVALID, yang_parse_submodule(&ctx_p, PARSER_CUR_PMOD(YCTX)->mod->ctx, (struct lysp_ctx *)YCTX, YCTX->in, &submod));
    CHECK_LOG_CTX("Invalid keyword \"prefix\", expected \"module\" or \"submodule\".", "Line number 1.");
    lysp_yang_ctx_free(ctx_p);
    submod = submod_renew(YCTX);

#undef TEST_GENERIC
#undef TEST_NODE
#undef TEST_DUP
#undef SCHEMA_BEGINNING
}

static void
test_deviation(void **state)
{
    struct lysp_deviation *d = NULL;

    /* invalid cardinality */
    TEST_DUP_GENERIC(" test {deviate not-supported;", "description", "a", "b", parse_deviation, &d, "1", );
    TEST_DUP_GENERIC(" test {deviate not-supported;", "reference", "a", "b", parse_deviation, &d, "1", );

    /* missing mandatory substatement */
    in.current = " test {description text;}";
    assert_int_equal(LY_EVALID, parse_deviation(YCTX, &d));
    CHECK_LOG_CTX("Missing mandatory keyword \"deviate\" as a child of \"deviation\".", "Line number 1.");

    /* invalid substatement */
    in.current = " test {deviate not-supported; status obsolete;}";
    assert_int_equal(LY_EVALID, parse_deviation(YCTX, &d));
    CHECK_LOG_CTX("Invalid keyword \"status\" as a child of \"deviation\".", "Line number 1.");
}

static void
test_deviate(void **state)
{
    struct lysp_deviate *d = NULL;

    /* invalid cardinality */
    TEST_DUP_GENERIC("add {", "config", "true", "false", parse_deviate, &d, "1", );
    TEST_DUP_GENERIC("add {", "mandatory", "true", "false", parse_deviate, &d, "1", );
    TEST_DUP_GENERIC("add {", "max-elements", "1", "2", parse_deviate, &d, "1", );
    TEST_DUP_GENERIC("add {", "min-elements", "1", "2", parse_deviate, &d, "1", );
    TEST_DUP_GENERIC("add {", "units", "kilometers", "miles", parse_deviate, &d, "1", );

    /* invalid substatements */
#define TEST_NOT_SUP(DEV, STMT, VALUE) \
    in.current = " "DEV" {"STMT" "VALUE";}..."; \
    assert_int_equal(LY_EVALID, parse_deviate(YCTX, &d)); \
    CHECK_LOG_CTX("Deviate \""DEV"\" does not support keyword \""STMT"\".", "Line number 1.");

    TEST_NOT_SUP("not-supported", "units", "meters");
    TEST_NOT_SUP("not-supported", "must", "1");
    TEST_NOT_SUP("not-supported", "unique", "x");
    TEST_NOT_SUP("not-supported", "default", "a");
    TEST_NOT_SUP("not-supported", "config", "true");
    TEST_NOT_SUP("not-supported", "mandatory", "true");
    TEST_NOT_SUP("not-supported", "min-elements", "1");
    TEST_NOT_SUP("not-supported", "max-elements", "2");
    TEST_NOT_SUP("not-supported", "type", "string");
    TEST_NOT_SUP("add", "type", "string");
    TEST_NOT_SUP("delete", "config", "true");
    TEST_NOT_SUP("delete", "mandatory", "true");
    TEST_NOT_SUP("delete", "min-elements", "1");
    TEST_NOT_SUP("delete", "max-elements", "2");
    TEST_NOT_SUP("delete", "type", "string");
    TEST_NOT_SUP("replace", "must", "1");
    TEST_NOT_SUP("replace", "unique", "a");

    in.current = " nonsence; ...";
    assert_int_equal(LY_EVALID, parse_deviate(YCTX, &d));
    CHECK_LOG_CTX("Invalid value \"nonsence\" of \"deviate\".", "Line number 1.");\
    assert_null(d);
#undef TEST_NOT_SUP
}

static void
test_container(void **state)
{
    struct lysp_node_container *c = NULL;

    PARSER_CUR_PMOD(YCTX)->version = 2; /* simulate YANG 1.1 */
    YCTX->main_ctx = (struct lysp_ctx *)YCTX;

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "cont {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_container(YCTX, NULL, (struct lysp_node**)&c)); \
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", "Line number 1."); \
    lysp_node_free(&fctx, (struct lysp_node*)c); c = NULL;

    TEST_DUP("config", "true", "false");
    TEST_DUP("description", "text1", "text2");
    TEST_DUP("presence", "true", "false");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content */
    in.current = "cont {action x;anydata any;anyxml anyxml; choice ch;config false;container c;description test;grouping g;if-feature f; leaf l {type string;}"
            "leaf-list ll {type string;} list li;must 'expr';notification not; presence true; reference test;status current;typedef t {type int8;}uses g;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_container(YCTX, NULL, (struct lysp_node **)&c));
    CHECK_LYSP_NODE(c, "test", 1, LYS_CONFIG_R | LYS_STATUS_CURR, 1, "cont", 0, LYS_CONTAINER, 0, "test", 1);
    assert_non_null(c->actions);
    assert_non_null(c->child);
    assert_non_null(c->groupings);
    assert_non_null(c->musts);
    assert_non_null(c->notifs);
    assert_string_equal("true", c->presence);
    assert_non_null(c->typedefs);
    ly_set_erase(&YCTX->tpdfs_nodes, NULL);
    ly_set_erase(&YCTX->grps_nodes, NULL);
    lysp_node_free(&fctx, (struct lysp_node *)c); c = NULL;

    /* invalid */
    in.current = " cont {augment /root;} ...";
    assert_int_equal(LY_EVALID, parse_container(YCTX, NULL, (struct lysp_node **)&c));
    CHECK_LOG_CTX("Invalid keyword \"augment\" as a child of \"container\".", "Line number 1.");
    lysp_node_free(&fctx, (struct lysp_node *)c); c = NULL;
    in.current = " cont {nonsence true;} ...";
    assert_int_equal(LY_EVALID, parse_container(YCTX, NULL, (struct lysp_node **)&c));
    CHECK_LOG_CTX("Invalid character sequence \"nonsence\", expected a keyword.", "Line number 1.");
    lysp_node_free(&fctx, (struct lysp_node *)c); c = NULL;

    PARSER_CUR_PMOD(YCTX)->version = 1; /* simulate YANG 1.0 */
    in.current = " cont {action x;} ...";
    assert_int_equal(LY_EVALID, parse_container(YCTX, NULL, (struct lysp_node **)&c));
    CHECK_LOG_CTX("Invalid keyword \"action\" as a child of \"container\" - "
            "the statement is allowed only in YANG 1.1 modules.", "Line number 1.");
    lysp_node_free(&fctx, (struct lysp_node *)c); c = NULL;
}

static void
test_leaf(void **state)
{
    struct lysp_node_leaf *l = NULL;

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "l {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_leaf(YCTX, NULL, (struct lysp_node**)&l)); \
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", "Line number 1."); \
    lysp_node_free(&fctx, (struct lysp_node*)l); l = NULL;

    TEST_DUP("config", "true", "false");
    TEST_DUP("default", "x", "y");
    TEST_DUP("description", "text1", "text2");
    TEST_DUP("mandatory", "true", "false");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("type", "int8", "uint8");
    TEST_DUP("units", "text1", "text2");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content - without mandatory which is mutual exclusive with default */
    in.current = "l {config false;default \"xxx\";description test;if-feature f;"
            "must 'expr';reference test;status current;type string; units yyy;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_leaf(YCTX, NULL, (struct lysp_node **)&l));
    CHECK_LYSP_NODE(l, "test", 1, LYS_CONFIG_R | LYS_STATUS_CURR, 1, "l", 0, LYS_LEAF, 0, "test", 1);
    assert_string_equal("xxx", l->dflt.str);
    assert_string_equal("yyy", l->units);
    assert_string_equal("string", l->type.name);
    assert_non_null(l->musts);
    lysp_node_free(&fctx, (struct lysp_node *)l); l = NULL;

    /* full content - now with mandatory */
    in.current = "l {mandatory true; type string;} ...";
    assert_int_equal(LY_SUCCESS, parse_leaf(YCTX, NULL, (struct lysp_node **)&l));
    CHECK_LYSP_NODE(l, NULL, 0, LYS_MAND_TRUE, 0, "l", 0, LYS_LEAF, 0, NULL, 0);
    assert_string_equal("string", l->type.name);
    lysp_node_free(&fctx, (struct lysp_node *)l); l = NULL;

    /* invalid */
    in.current = " l {description \"missing type\";} ...";
    assert_int_equal(LY_EVALID, parse_leaf(YCTX, NULL, (struct lysp_node **)&l));
    CHECK_LOG_CTX("Missing mandatory keyword \"type\" as a child of \"leaf\".", "Line number 1.");
    lysp_node_free(&fctx, (struct lysp_node *)l); l = NULL;

    in.current = "l { type iid { path qpud wrong {";
    assert_int_equal(LY_EVALID, parse_leaf(YCTX, NULL, (struct lysp_node **)&l));
    CHECK_LOG_CTX("Invalid character sequence \"wrong\", expected a keyword.", "Line number 1.");
    lysp_node_free(&fctx, (struct lysp_node *)l); l = NULL;
}

static void
test_leaflist(void **state)
{
    struct lysp_node_leaflist *ll = NULL;

    PARSER_CUR_PMOD(YCTX)->version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "ll {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_leaflist(YCTX, NULL, (struct lysp_node**)&ll)); \
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", "Line number 1."); \
    lysp_node_free(&fctx, (struct lysp_node*)ll); ll = NULL;

    TEST_DUP("config", "true", "false");
    TEST_DUP("description", "text1", "text2");
    TEST_DUP("max-elements", "10", "20");
    TEST_DUP("min-elements", "10", "20");
    TEST_DUP("ordered-by", "user", "system");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("type", "int8", "uint8");
    TEST_DUP("units", "text1", "text2");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content - without min-elements which is mutual exclusive with default */
    in.current = "ll {config false;default \"xxx\"; default \"yyy\";description test;if-feature f;"
            "max-elements 10;must 'expr';ordered-by user;reference test;"
            "status current;type string; units zzz;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_leaflist(YCTX, NULL, (struct lysp_node **)&ll));
    CHECK_LYSP_NODE(ll, "test", 1, 0x446, 1, "ll", 0, LYS_LEAFLIST, 0, "test", 1);
    assert_non_null(ll->dflts);
    assert_int_equal(2, LY_ARRAY_COUNT(ll->dflts));
    assert_string_equal("xxx", ll->dflts[0].str);
    assert_string_equal("yyy", ll->dflts[1].str);
    assert_string_equal("zzz", ll->units);
    assert_int_equal(10, ll->max);
    assert_int_equal(0, ll->min);
    assert_string_equal("string", ll->type.name);
    assert_non_null(ll->musts);
    assert_int_equal(LYS_CONFIG_R | LYS_STATUS_CURR | LYS_ORDBY_USER | LYS_SET_MAX, ll->flags);
    lysp_node_free(&fctx, (struct lysp_node *)ll); ll = NULL;

    /* full content - now with min-elements */
    in.current = "ll {min-elements 10; type string;} ...";
    assert_int_equal(LY_SUCCESS, parse_leaflist(YCTX, NULL, (struct lysp_node **)&ll));
    CHECK_LYSP_NODE(ll, NULL, 0, 0x200, 0, "ll", 0, LYS_LEAFLIST, 0, NULL, 0);
    assert_string_equal("string", ll->type.name);
    assert_int_equal(0, ll->max);
    assert_int_equal(10, ll->min);
    assert_int_equal(LYS_SET_MIN, ll->flags);
    lysp_node_free(&fctx, (struct lysp_node *)ll); ll = NULL;

    /* invalid */
    in.current = " ll {description \"missing type\";} ...";
    assert_int_equal(LY_EVALID, parse_leaflist(YCTX, NULL, (struct lysp_node **)&ll));
    CHECK_LOG_CTX("Missing mandatory keyword \"type\" as a child of \"leaf-list\".", "Line number 1.");
    lysp_node_free(&fctx, (struct lysp_node *)ll); ll = NULL;

    PARSER_CUR_PMOD(YCTX)->version = 1; /* simulate YANG 1.0 - default statement is not allowed */
    in.current = " ll {default xx; type string;} ...";
    assert_int_equal(LY_EVALID, parse_leaflist(YCTX, NULL, (struct lysp_node **)&ll));
    CHECK_LOG_CTX("Invalid keyword \"default\" as a child of \"leaf-list\" - the statement is allowed only in YANG 1.1 modules.", "Line number 1.");
    lysp_node_free(&fctx, (struct lysp_node *)ll); ll = NULL;
}

static void
test_list(void **state)
{
    struct lysp_node_list *l = NULL;

    PARSER_CUR_PMOD(YCTX)->version = 2; /* simulate YANG 1.1 */
    YCTX->main_ctx = (struct lysp_ctx *)YCTX;

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "l {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_list(YCTX, NULL, (struct lysp_node**)&l)); \
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", "Line number 1."); \
    lysp_node_free(&fctx, (struct lysp_node*)l); l = NULL;

    TEST_DUP("config", "true", "false");
    TEST_DUP("description", "text1", "text2");
    TEST_DUP("key", "one", "two");
    TEST_DUP("max-elements", "10", "20");
    TEST_DUP("min-elements", "10", "20");
    TEST_DUP("ordered-by", "user", "system");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content */
    in.current = "l {action x;anydata any;anyxml anyxml; choice ch;config false;container c;description test;grouping g;if-feature f; key l; leaf l {type string;}"
            "leaf-list ll {type string;} list li;max-elements 10; min-elements 1;must 'expr';notification not; ordered-by system; reference test;"
            "status current;typedef t {type int8;}unique xxx;unique yyy;uses g;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_list(YCTX, NULL, (struct lysp_node **)&l));
    CHECK_LYSP_NODE(l, "test", 1, LYS_CONFIG_R | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM | LYS_SET_MAX | LYS_SET_MIN, 1, "l",
            0, LYS_LIST, 0, "test", 1);
    assert_string_equal("l", l->key);
    assert_non_null(l->uniques);
    assert_int_equal(2, LY_ARRAY_COUNT(l->uniques));
    assert_string_equal("xxx", l->uniques[0].str);
    assert_string_equal("yyy", l->uniques[1].str);
    assert_int_equal(10, l->max);
    assert_int_equal(1, l->min);
    assert_non_null(l->musts);
    ly_set_erase(&YCTX->tpdfs_nodes, NULL);
    ly_set_erase(&YCTX->grps_nodes, NULL);
    lysp_node_free(&fctx, (struct lysp_node *)l); l = NULL;

    /* invalid content */
    PARSER_CUR_PMOD(YCTX)->version = 1; /* simulate YANG 1.0 */
    in.current = "l {action x;} ...";
    assert_int_equal(LY_EVALID, parse_list(YCTX, NULL, (struct lysp_node **)&l));
    CHECK_LOG_CTX("Invalid keyword \"action\" as a child of \"list\" - the statement is allowed only in YANG 1.1 modules.", "Line number 1.");
    lysp_node_free(&fctx, (struct lysp_node *)l); l = NULL;
}

static void
test_choice(void **state)
{
    struct lysp_node_choice *ch = NULL;

    PARSER_CUR_PMOD(YCTX)->version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "ch {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_choice(YCTX, NULL, (struct lysp_node**)&ch)); \
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", "Line number 1."); \
    lysp_node_free(&fctx, (struct lysp_node*)ch); ch = NULL;

    TEST_DUP("config", "true", "false");
    TEST_DUP("default", "a", "b");
    TEST_DUP("description", "text1", "text2");
    TEST_DUP("mandatory", "true", "false");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content - without default due to a collision with mandatory */
    in.current = "ch {anydata any;anyxml anyxml; case c;choice ch;config false;container c;description test;if-feature f;leaf l {type string;}"
            "leaf-list ll {type string;} list li;mandatory true;reference test;status current;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_choice(YCTX, NULL, (struct lysp_node **)&ch));
    CHECK_LYSP_NODE(ch, "test", 1, LYS_CONFIG_R | LYS_STATUS_CURR | LYS_MAND_TRUE, 1, "ch", 0, LYS_CHOICE, 0, "test", 1);
    lysp_node_free(&fctx, (struct lysp_node *)ch); ch = NULL;

    /* full content - the default missing from the previous node */
    in.current = "ch {default c;case c;} ...";
    assert_int_equal(LY_SUCCESS, parse_choice(YCTX, NULL, (struct lysp_node **)&ch));
    CHECK_LYSP_NODE(ch, NULL, 0, 0, 0, "ch", 0, LYS_CHOICE, 0, NULL, 0);
    assert_string_equal("c", ch->dflt.str);
    lysp_node_free(&fctx, (struct lysp_node *)ch); ch = NULL;
}

static void
test_case(void **state)
{
    struct lysp_node_case *cs = NULL;

    PARSER_CUR_PMOD(YCTX)->version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "cs {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_case(YCTX, NULL, (struct lysp_node**)&cs)); \
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", "Line number 1."); \
    lysp_node_free(&fctx, (struct lysp_node*)cs); cs = NULL;

    TEST_DUP("description", "text1", "text2");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content */
    in.current = "cs {anydata any;anyxml anyxml; choice ch;container c;description test;if-feature f;leaf l {type string;}"
            "leaf-list ll {type string;} list li;reference test;status current;uses grp;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_case(YCTX, NULL, (struct lysp_node **)&cs));
    CHECK_LYSP_NODE(cs, "test", 1, LYS_STATUS_CURR, 1, "cs", 0, LYS_CASE, 0, "test", 1);
    lysp_node_free(&fctx, (struct lysp_node *)cs); cs = NULL;

    /* invalid content */
    in.current = "cs {config true} ...";
    assert_int_equal(LY_EVALID, parse_case(YCTX, NULL, (struct lysp_node **)&cs));
    CHECK_LOG_CTX("Invalid keyword \"config\" as a child of \"case\".", "Line number 1.");
    lysp_node_free(&fctx, (struct lysp_node *)cs); cs = NULL;
}

static void
test_any(void **state, enum ly_stmt kw)
{
    struct lysp_node_anydata *any = NULL;

    if (kw == LY_STMT_ANYDATA) {
        PARSER_CUR_PMOD(YCTX)->version = 2; /* simulate YANG 1.1 */
    } else {
        PARSER_CUR_PMOD(YCTX)->version = 1; /* simulate YANG 1.0 */
    }

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "l {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_any(YCTX, kw, NULL, (struct lysp_node**)&any)); \
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", "Line number 1."); \
    lysp_node_free(&fctx, (struct lysp_node*)any); any = NULL;

    TEST_DUP("config", "true", "false");
    TEST_DUP("description", "text1", "text2");
    TEST_DUP("mandatory", "true", "false");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content */
    in.current = "any {config true;description test;if-feature f;mandatory true;must 'expr';reference test;status current;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_any(YCTX, kw, NULL, (struct lysp_node **)&any));
    // CHECK_LYSP_NODE(NODE, DSC, EXTS, FLAGS, IFFEATURES, NAME, NEXT, TYPE, PARENT, REF, WHEN)
    uint16_t node_type = kw == LY_STMT_ANYDATA ? LYS_ANYDATA : LYS_ANYXML;

    CHECK_LYSP_NODE(any, "test", 1, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_MAND_TRUE, 1, "any", 0, node_type, 0, "test", 1);
    assert_non_null(any->musts);
    lysp_node_free(&fctx, (struct lysp_node *)any); any = NULL;
}

static void
test_anydata(void **state)
{
    test_any(state, LY_STMT_ANYDATA);
}

static void
test_anyxml(void **state)
{
    test_any(state, LY_STMT_ANYXML);
}

static void
test_grouping(void **state)
{
    struct lysp_node_grp *grp = NULL;

    PARSER_CUR_PMOD(YCTX)->version = 2; /* simulate YANG 1.1 */
    YCTX->main_ctx = (struct lysp_ctx *)YCTX;

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "l {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_grouping(YCTX, NULL, &grp)); \
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", "Line number 1."); \
    lysp_node_free(&fctx, &grp->node); grp = NULL;

    TEST_DUP("description", "text1", "text2");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
#undef TEST_DUP

    /* full content */
    in.current = "grp {action x;anydata any;anyxml anyxml; choice ch;container c;description test;grouping g;leaf l {type string;}"
            "leaf-list ll {type string;} list li;notification not;reference test;status current;typedef t {type int8;}uses g;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_grouping(YCTX, NULL, &grp));
    assert_non_null(grp);
    assert_int_equal(LYS_GROUPING, grp->nodetype);
    assert_string_equal("grp", grp->name);
    assert_string_equal("test", grp->dsc);
    assert_non_null(grp->exts);
    assert_string_equal("test", grp->ref);
    assert_null(grp->parent);
    assert_int_equal(LYS_STATUS_CURR, grp->flags);
    ly_set_erase(&YCTX->tpdfs_nodes, NULL);
    ly_set_erase(&YCTX->grps_nodes, NULL);
    lysp_node_free(&fctx, &grp->node);
    grp = NULL;

    /* invalid content */
    in.current = "grp {config true} ...";
    assert_int_equal(LY_EVALID, parse_grouping(YCTX, NULL, &grp));
    CHECK_LOG_CTX("Invalid keyword \"config\" as a child of \"grouping\".", "Line number 1.");
    lysp_node_free(&fctx, &grp->node);
    grp = NULL;

    in.current = "grp {must 'expr'} ...";
    assert_int_equal(LY_EVALID, parse_grouping(YCTX, NULL, &grp));
    CHECK_LOG_CTX("Invalid keyword \"must\" as a child of \"grouping\".", "Line number 1.");
    lysp_node_free(&fctx, &grp->node);
    grp = NULL;
}

static void
test_action(void **state)
{
    struct lysp_node_action *rpcs = NULL;
    struct lysp_node_container *c = NULL;

    PARSER_CUR_PMOD(YCTX)->version = 2; /* simulate YANG 1.1 */
    YCTX->main_ctx = (struct lysp_ctx *)YCTX;

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "func {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_action(YCTX, NULL, &rpcs)); \
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", "Line number 1."); \
    lysp_node_free(&fctx, (struct lysp_node*)rpcs); rpcs = NULL;

    TEST_DUP("description", "text1", "text2");
    TEST_DUP("input", "{leaf l1 {type empty;}} description a", "{leaf l2 {type empty;}} description a");
    TEST_DUP("output", "{leaf l1 {type empty;}} description a", "{leaf l2 {type empty;}} description a");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
#undef TEST_DUP

    /* full content */
    in.current = "top;";
    assert_int_equal(LY_SUCCESS, parse_container(YCTX, NULL, (struct lysp_node **)&c));
    in.current = "func {description test;grouping grp;if-feature f;reference test;status current;typedef mytype {type int8;} m:ext;"
            "input {anydata a1; anyxml a2; choice ch; container c; grouping grp; leaf l {type int8;} leaf-list ll {type int8;}"
            " list li; must 1; typedef mytypei {type int8;} uses grp; m:ext;}"
            "output {anydata a1; anyxml a2; choice ch; container c; grouping grp; leaf l {type int8;} leaf-list ll {type int8;}"
            " list li; must 1; typedef mytypeo {type int8;} uses grp; m:ext;}} ...";
    assert_int_equal(LY_SUCCESS, parse_action(YCTX, (struct lysp_node *)c, &rpcs));
    assert_non_null(rpcs);
    assert_int_equal(LYS_ACTION, rpcs->nodetype);
    assert_string_equal("func", rpcs->name);
    assert_string_equal("test", rpcs->dsc);
    assert_non_null(rpcs->exts);
    assert_non_null(rpcs->iffeatures);
    assert_string_equal("test", rpcs->ref);
    assert_non_null(rpcs->groupings);
    assert_non_null(rpcs->typedefs);
    assert_int_equal(LYS_STATUS_CURR, rpcs->flags);
    /* input */
    assert_int_equal(rpcs->input.nodetype, LYS_INPUT);
    assert_non_null(rpcs->input.groupings);
    assert_non_null(rpcs->input.exts);
    assert_non_null(rpcs->input.musts);
    assert_non_null(rpcs->input.typedefs);
    assert_non_null(rpcs->input.child);
    /* output */
    assert_int_equal(rpcs->output.nodetype, LYS_OUTPUT);
    assert_non_null(rpcs->output.groupings);
    assert_non_null(rpcs->output.exts);
    assert_non_null(rpcs->output.musts);
    assert_non_null(rpcs->output.typedefs);
    assert_non_null(rpcs->output.child);

    ly_set_erase(&YCTX->tpdfs_nodes, NULL);
    ly_set_erase(&YCTX->grps_nodes, NULL);
    lysp_node_free(&fctx, (struct lysp_node *)rpcs); rpcs = NULL;

    /* invalid content */
    in.current = "func {config true} ...";
    assert_int_equal(LY_EVALID, parse_action(YCTX, NULL, &rpcs));
    CHECK_LOG_CTX("Invalid keyword \"config\" as a child of \"rpc\".", "Line number 1.");
    lysp_node_free(&fctx, (struct lysp_node *)rpcs); rpcs = NULL;

    lysp_node_free(&fctx, (struct lysp_node *)c);
}

static void
test_notification(void **state)
{
    struct lysp_node_notif *notifs = NULL;
    struct lysp_node_container *c = NULL;

    PARSER_CUR_PMOD(YCTX)->version = 2; /* simulate YANG 1.1 */
    YCTX->main_ctx = (struct lysp_ctx *)YCTX;

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "func {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_notif(YCTX, NULL, &notifs)); \
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", "Line number 1."); \
    lysp_node_free(&fctx, (struct lysp_node*)notifs); notifs = NULL;

    TEST_DUP("description", "text1", "text2");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
#undef TEST_DUP

    /* full content */
    in.current = "top;";
    assert_int_equal(LY_SUCCESS, parse_container(YCTX, NULL, (struct lysp_node **)&c));
    in.current = "ntf {anydata a1; anyxml a2; choice ch; container c; description test; grouping grp; if-feature f; leaf l {type int8;}"
            "leaf-list ll {type int8;} list li; must 1; reference test; status current; typedef mytype {type int8;} uses grp; m:ext;}";
    assert_int_equal(LY_SUCCESS, parse_notif(YCTX, (struct lysp_node *)c, &notifs));
    assert_non_null(notifs);
    assert_int_equal(LYS_NOTIF, notifs->nodetype);
    assert_string_equal("ntf", notifs->name);
    assert_string_equal("test", notifs->dsc);
    assert_non_null(notifs->exts);
    assert_non_null(notifs->iffeatures);
    assert_string_equal("test", notifs->ref);
    assert_non_null(notifs->groupings);
    assert_non_null(notifs->typedefs);
    assert_non_null(notifs->musts);
    assert_non_null(notifs->child);
    assert_int_equal(LYS_STATUS_CURR, notifs->flags);

    ly_set_erase(&YCTX->tpdfs_nodes, NULL);
    ly_set_erase(&YCTX->grps_nodes, NULL);
    lysp_node_free(&fctx, (struct lysp_node *)notifs); notifs = NULL;

    /* invalid content */
    in.current = "ntf {config true} ...";
    assert_int_equal(LY_EVALID, parse_notif(YCTX, NULL, &notifs));
    CHECK_LOG_CTX("Invalid keyword \"config\" as a child of \"notification\".", "Line number 1.");
    lysp_node_free(&fctx, (struct lysp_node *)notifs); notifs = NULL;

    lysp_node_free(&fctx, (struct lysp_node *)c);
}

static void
test_uses(void **state)
{
    struct lysp_node_uses *u = NULL;

    PARSER_CUR_PMOD(YCTX)->version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "l {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_uses(YCTX, NULL, (struct lysp_node**)&u)); \
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", "Line number 1."); \
    lysp_node_free(&fctx, (struct lysp_node*)u); u = NULL;

    TEST_DUP("description", "text1", "text2");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content */
    in.current = "grpref {augment some/node;description test;if-feature f;reference test;refine some/other/node;status current;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_uses(YCTX, NULL, (struct lysp_node **)&u));
    CHECK_LYSP_NODE(u, "test", 1, LYS_STATUS_CURR, 1, "grpref", 0, LYS_USES, 0, "test", 1);
    assert_non_null(u->augments);
    assert_non_null(u->refines);
    lysp_node_free(&fctx, (struct lysp_node *)u); u = NULL;
}

static void
test_augment(void **state)
{
    struct lysp_node_augment *a = NULL;

    PARSER_CUR_PMOD(YCTX)->version = 2; /* simulate YANG 1.1 */

    /* invalid cardinality */
#define TEST_DUP(MEMBER, VALUE1, VALUE2) \
    in.current = "l {" MEMBER" "VALUE1";"MEMBER" "VALUE2";} ..."; \
    assert_int_equal(LY_EVALID, parse_augment(YCTX, NULL, &a)); \
    CHECK_LOG_CTX("Duplicate keyword \""MEMBER"\".", "Line number 1."); \
    lysp_node_free(&fctx, (struct lysp_node *)a); a = NULL;

    TEST_DUP("description", "text1", "text2");
    TEST_DUP("reference", "1", "2");
    TEST_DUP("status", "current", "obsolete");
    TEST_DUP("when", "true", "false");
#undef TEST_DUP

    /* full content */
    in.current = "/target/nodeid {action x; anydata any;anyxml anyxml; case cs; choice ch;container c;description test;if-feature f;leaf l {type string;}"
            "leaf-list ll {type string;} list li;notification not;reference test;status current;uses g;when true;m:ext;} ...";
    assert_int_equal(LY_SUCCESS, parse_augment(YCTX, NULL, &a));
    assert_non_null(a);
    assert_int_equal(LYS_AUGMENT, a->nodetype);
    assert_string_equal("/target/nodeid", a->nodeid);
    assert_string_equal("test", a->dsc);
    assert_non_null(a->exts);
    assert_non_null(a->iffeatures);
    assert_string_equal("test", a->ref);
    assert_non_null(a->when);
    assert_null(a->parent);
    assert_int_equal(LYS_STATUS_CURR, a->flags);
    lysp_node_free(&fctx, (struct lysp_node *)a); a = NULL;
}

static void
test_when(void **state)
{
    struct lysp_when *w = NULL;

    PARSER_CUR_PMOD(YCTX)->version = 2; /* simulate YANG 1.1 */

    in.current = "l { description text1;description text2;} ...";
    assert_int_equal(LY_EVALID, parse_when(YCTX, &w));
    assert_null(w);
    CHECK_LOG_CTX("Duplicate keyword \"description\".", "Line number 1.");

    in.current = "l { reference 1;reference 2;} ...";
    assert_int_equal(LY_EVALID, parse_when(YCTX, &w));
    assert_null(w);
    CHECK_LOG_CTX("Duplicate keyword \"reference\".", "Line number 1.");
}

static void
test_value(void **state)
{
    struct lysp_type_enum enm;

    in.current = "-0;";
    memset(&enm, 0, sizeof enm);
    assert_int_equal(parse_type_enum_value_pos(YCTX, LY_STMT_VALUE, &enm), LY_SUCCESS);

    in.current = "-0;";
    memset(&enm, 0, sizeof enm);
    assert_int_equal(parse_type_enum_value_pos(YCTX, LY_STMT_POSITION, &enm), LY_EVALID);
    CHECK_LOG_CTX("Invalid value \"-0\" of \"position\".", "Line number 1.");
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_helpers, setup, teardown),
        UTEST(test_comments, setup, teardown),
        UTEST(test_arg, setup, teardown),
        UTEST(test_stmts, setup, teardown),
        UTEST(test_minmax, setup, teardown),
        UTEST(test_valid_module, setup, teardown),
        UTEST(test_module, setup, teardown),
        UTEST(test_deviation, setup, teardown),
        UTEST(test_deviate, setup, teardown),
        UTEST(test_container, setup, teardown),
        UTEST(test_leaf, setup, teardown),
        UTEST(test_leaflist, setup, teardown),
        UTEST(test_list, setup, teardown),
        UTEST(test_choice, setup, teardown),
        UTEST(test_case, setup, teardown),
        UTEST(test_anydata, setup, teardown),
        UTEST(test_anyxml, setup, teardown),
        UTEST(test_action, setup, teardown),
        UTEST(test_notification, setup, teardown),
        UTEST(test_grouping, setup, teardown),
        UTEST(test_uses, setup, teardown),
        UTEST(test_augment, setup, teardown),
        UTEST(test_when, setup, teardown),
        UTEST(test_value, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
