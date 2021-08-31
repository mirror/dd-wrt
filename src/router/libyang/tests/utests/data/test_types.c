/*
 * @file test_types.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for support of YANG data types
 *
 * Copyright (c) 2019-2020 CESNET, z.s.p.o.
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

#include "libyang.h"
#include "path.h"
#include "plugins_types.h"

const char *schema_a = "module defs {namespace urn:tests:defs;prefix d;yang-version 1.1;"
        "identity crypto-alg; identity interface-type; identity ethernet {base interface-type;} identity fast-ethernet {base ethernet;}"
        "typedef iref {type identityref {base interface-type;}}}";
const char *schema_b = "module types {namespace urn:tests:types;prefix t;yang-version 1.1; import defs {prefix defs;}"
        "feature f; identity gigabit-ethernet { base defs:ethernet;}"
        "typedef tboolean {type boolean;}"
        "typedef tempty {type empty;}"
        "container cont {leaf leaftarget {type empty;}"
        "    list listtarget {key id; max-elements 5;leaf id {type uint8;} leaf value {type string;}}"
        "    leaf-list leaflisttarget {type uint8; max-elements 5;}}"
        "list list {key id; leaf id {type string;} leaf value {type string;} leaf-list targets {type string;}}"
        "list list2 {key \"id value\"; leaf id {type string;} leaf value {type string;}}"
        "list list_inst {key id; leaf id {type instance-identifier {require-instance true;}} leaf value {type string;}}"
        "list list_ident {key id; leaf id {type identityref {base defs:interface-type;}} leaf value {type string;}}"
        "list list_keyless {config \"false\"; leaf id {type string;} leaf value {type string;}}"
        "leaf-list leaflisttarget {type string;}"
        "leaf binary {type binary {length 5 {error-message \"This base64 value must be of length 5.\";}}}"
        "leaf binary-norestr {type binary;}"
        "leaf int8 {type int8 {range 10..20;}}"
        "leaf uint8 {type uint8 {range 150..200;}}"
        "leaf int16 {type int16 {range -20..-10;}}"
        "leaf uint16 {type uint16 {range 150..200;}}"
        "leaf int32 {type int32;}"
        "leaf uint32 {type uint32;}"
        "leaf int64 {type int64;}"
        "leaf uint64 {type uint64;}"
        "leaf bits {type bits {bit zero; bit one {if-feature f;} bit two;}}"
        "leaf enums {type enumeration {enum white; enum yellow {if-feature f;}}}"
        "leaf dec64 {type decimal64 {fraction-digits 1; range 1.5..10;}}"
        "leaf dec64-norestr {type decimal64 {fraction-digits 18;}}"
        "leaf str {type string {length 8..10; pattern '[a-z ]*';}}"
        "leaf str-invert {type string {pattern '[a-z ]*' {modifier invert-match;}}}"
        "leaf str-norestr {type string;}"
        "leaf str-utf8 {type string{length 2..5; pattern '€*';}}"
        "leaf bool {type boolean;}"
        "leaf tbool {type tboolean;}"
        "leaf empty {type empty;}"
        "leaf tempty {type tempty;}"
        "leaf ident {type identityref {base defs:interface-type;}}"
        "leaf iref {type defs:iref;}"
        "leaf inst {type instance-identifier {require-instance true;}}"
        "leaf inst-noreq {type instance-identifier {require-instance false;}}"
        "leaf lref {type leafref {path /leaflisttarget; require-instance true;}}"
        "leaf lref2 {type leafref {path \"../list[id = current()/../str-norestr]/targets\"; require-instance true;}}"
        "leaf un1 {type union {"
        "    type leafref {path /int8; require-instance true;}"
        "    type union { type identityref {base defs:interface-type;} type instance-identifier {require-instance true;} }"
        "    type string {length 1..20;}}}}";

static int
setup(void **state)
{
    UTEST_SETUP;
    UTEST_ADD_MODULE(schema_a, LYS_IN_YANG, NULL, NULL);
    UTEST_ADD_MODULE(schema_b, LYS_IN_YANG, NULL, NULL);

    return 0;
}

#define CHECK_PARSE_LYD(INPUT, MODEL) \
    CHECK_PARSE_LYD_PARAM(INPUT, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, MODEL)

#define TEST_PATTERN_1(INPUT, SCHEMA_NAME, SCHEMA_NEXT, VALUE_TYPE, ...) \
    { \
        struct lyd_node_term *leaf; \
        struct lyd_value value = {0}; \
    /* create model */ \
        CHECK_LYSC_NODE((INPUT)->schema, NULL, 0, 0x5, 1, SCHEMA_NAME, SCHEMA_NEXT, LYS_LEAF, 0, 0, NULL, 0); \
    /* CHECK_LYSC_NODE((INPUT)->schema, LYS_LEAF, SCHEMA_NAME); */ \
        leaf = (struct lyd_node_term*)(INPUT); \
        CHECK_LYD_NODE_TERM(leaf, 0, 0, 0, 0, 1, VALUE_TYPE, ## __VA_ARGS__); \
    /* copy value */ \
        assert_int_equal(LY_SUCCESS, leaf->value.realtype->plugin->duplicate(UTEST_LYCTX, &leaf->value, &value)); \
        CHECK_LYD_VALUE(value, VALUE_TYPE, ## __VA_ARGS__); \
        if (LY_TYPE_INST == LY_TYPE_ ## VALUE_TYPE) { \
            for (LY_ARRAY_COUNT_TYPE it = 0; it < LY_ARRAY_COUNT(leaf->value.target); it++) { \
                assert_ptr_equal(value.target[it].node, leaf->value.target[it].node); \
                LY_ARRAY_COUNT_TYPE arr_size_predicates = 0; \
                if (value.target[it].pred_type == LY_PATH_PREDTYPE_NONE) { \
                    assert_null(value.target[it].predicates); \
                } else { \
                    assert_non_null(value.target[it].predicates); \
                    arr_size_predicates =  LY_ARRAY_COUNT(value.target[it].predicates); \
                    assert_int_equal(LY_ARRAY_COUNT(value.target[it].predicates), LY_ARRAY_COUNT(leaf->value.target[it].predicates)); \
                } \
                for (LY_ARRAY_COUNT_TYPE jt = 0; jt < arr_size_predicates; jt++) { \
                    if (value.target[it].pred_type == LY_PATH_PREDTYPE_POSITION) { \
                        assert_int_equal(value.target[it].predicates[jt].position, leaf->value.target[it].predicates[jt].position); \
                    } else { \
                        assert_true(LY_SUCCESS == value.realtype->plugin->compare(&value, &leaf->value)); \
                    } \
                } \
            } \
        } \
        value.realtype->plugin->free(UTEST_LYCTX, &value); \
    }

#define TEST_TYPE_ERROR(TYPE, VALUE, ERROR_MSG, LINE) \
    { \
        const char *data = "<" TYPE " xmlns=\"urn:tests:types\">" VALUE "</" TYPE">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree); \
        CHECK_LOG_CTX(ERROR_MSG, "Schema location /types:"TYPE", line number "LINE"."); \
    }

#define TEST_PRINTED_VALUE(VALUE, EXPECTED, FORMAT, PREFIX_DATA) \
    { \
        const char *_str; \
        uint8_t _dynamic; \
        assert_non_null(_str = (VALUE)->realtype->plugin->print(UTEST_LYCTX, VALUE, FORMAT, (void *)PREFIX_DATA, &_dynamic, NULL)); \
        assert_string_equal(EXPECTED, _str); \
        if (_dynamic) { \
            free((char *)_str); \
        } \
    }

static void
test_int(void **state)
{
    struct lyd_node *tree;
    const char *data;
    char *error_msg;

#if 0
    /* type int8 is tested in file ./tests/utests/types/int8.c */
    /* valid data */
    data = "<int8 xmlns=\"urn:tests:types\">\n 15 \t\n  </int8>";
    CHECK_PARSE_LYD(data, tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "int8", 1, INT8, "15", 15);
    lyd_free_all(tree);

    /* invalid range */
    error_msg = "Value \"1\" does not satisfy the range constraint.";
    TEST_TYPE_ERROR("int8", "1", error_msg, "1");
#endif

    error_msg = "Unsatisfied range - value \"100\" is out of the allowed range.";
    TEST_TYPE_ERROR("int16", "100", error_msg, "1");

    /* invalid value */
    error_msg = "Invalid int32 value \"0x01\".";
    TEST_TYPE_ERROR("int32", "0x01", error_msg, "1");
    error_msg = "Invalid empty int64 value.";
    TEST_TYPE_ERROR("int64", "", error_msg, "1");
    error_msg = "Invalid empty int64 value.";
    TEST_TYPE_ERROR("int64", "   ", error_msg, "1");
    error_msg = "Invalid int64 value \"-10  xxx\".";
    TEST_TYPE_ERROR("int64", "-10  xxx", error_msg, "1");
}

static void
test_uint(void **state)
{
    struct lyd_node *tree;

    /* valid data */
    CHECK_PARSE_LYD("<uint8 xmlns=\"urn:tests:types\">\n 150 \t\n  </uint8>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "uint8", 1, UINT8, "150", 150);
    lyd_free_all(tree);

    /* invalid range */
    TEST_TYPE_ERROR("uint8", "\n 15 \t\n  ",
            "Unsatisfied range - value \"15\" is out of the allowed range.", "3");
    TEST_TYPE_ERROR("uint16", "\n 1500 \t\n  ",
            "Unsatisfied range - value \"1500\" is out of the allowed range.", "3");

    /* invalid value */
    TEST_TYPE_ERROR("uint32", "-10",
            "Value \"-10\" is out of uint32's min/max bounds.", "1");
    CHECK_PARSE_LYD_PARAM("<uint64 xmlns=\"urn:tests:types\"/>", LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid empty uint64 value.", "Schema location /types:uint64, line number 1.");
    TEST_TYPE_ERROR("uint64", "   ", "Invalid empty uint64 value.", "1");
    TEST_TYPE_ERROR("uint64", "10  xxx", "Invalid uint64 value \"10  xxx\".", "1");
}

static void
test_dec64(void **state)
{
    struct lyd_node *tree;

    /* valid data */
    CHECK_PARSE_LYD("<dec64 xmlns=\"urn:tests:types\">\n +8 \t\n  </dec64>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "dec64", 1, DEC64, "8.0", 80);
    lyd_free_all(tree);

    CHECK_PARSE_LYD("<dec64 xmlns=\"urn:tests:types\">8.00</dec64>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "dec64", 1, DEC64, "8.0", 80);
    lyd_free_all(tree);

    CHECK_PARSE_LYD("<dec64-norestr xmlns=\"urn:tests:types\">-9.223372036854775808</dec64-norestr>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "dec64-norestr", 1, DEC64, "-9.223372036854775808", INT64_C(-9223372036854775807) - INT64_C(1));
    lyd_free_all(tree);

    CHECK_PARSE_LYD("<dec64-norestr xmlns=\"urn:tests:types\">9.223372036854775807</dec64-norestr>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "dec64-norestr", 1, DEC64, "9.223372036854775807", INT64_C(9223372036854775807));
    lyd_free_all(tree);

    /* invalid range */
    TEST_TYPE_ERROR("dec64", "\n 15 \t\n  ", "Unsatisfied range - value \"15.0\" is out of the allowed range.", "3");
    TEST_TYPE_ERROR("dec64", "\n 0 \t\n  ", "Unsatisfied range - value \"0.0\" is out of the allowed range.", "3");

    /* invalid value */
    TEST_TYPE_ERROR("dec64", "xxx", "Invalid 1. character of decimal64 value \"xxx\".", "1");
    CHECK_PARSE_LYD_PARAM("<dec64 xmlns=\"urn:tests:types\"/>", LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid empty decimal64 value.", "Schema location /types:dec64, line number 1.");
    TEST_TYPE_ERROR("dec64", "   ", "Invalid empty decimal64 value.", "1");
    TEST_TYPE_ERROR("dec64", "8.5  xxx", "Invalid 6. character of decimal64 value \"8.5  xxx\".", "1");
    TEST_TYPE_ERROR("dec64", "8.55  xxx", "Value \"8.55\" of decimal64 type exceeds defined number (1) of fraction digits.", "1");
}

/* test string is now in file string.c */
#if 0
static void
test_string(void **state)
{
    struct lyd_node *tree;

    /* valid data */
    CHECK_PARSE_LYD("<str xmlns=\"urn:tests:types\">teststring</str>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "str", 1, STRING, "teststring");
    lyd_free_all(tree);

    /* inverted match */
    CHECK_PARSE_LYD("<str-invert xmlns=\"urn:tests:types\">TESTSTRING</str-invert>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "str-invert", 1, STRING, "TESTSTRING");
    lyd_free_all(tree);

    TEST_TYPE_ERROR("str-invert", "teststring",
            "Unsatisfied pattern - \"teststring\" does not conform to inverted \"[a-z ]*\".", "1");

    /* multibyte characters (€ encodes as 3-byte UTF8 character, length restriction is 2-5) */
    CHECK_PARSE_LYD("<str-utf8 xmlns=\"urn:tests:types\">€€</str-utf8>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "str-utf8", 1, STRING, "€€");
    lyd_free_all(tree);

    /*error */
    TEST_TYPE_ERROR("str-utf8", "€",
            "Unsatisfied length - string length \"1\" is not allowed.", "1");
    TEST_TYPE_ERROR("str-utf8", "€€€€€€",
            "Unsatisfied length - string length \"6\" is not allowed.", "1");
    TEST_TYPE_ERROR("str-utf8", "€€x",
            "Unsatisfied pattern - \"€€x\" does not conform to \"€*\".", "1");

    /* invalid length */
    TEST_TYPE_ERROR("str", "short",
            "Unsatisfied length - string length \"5\" is not allowed.", "1");
    TEST_TYPE_ERROR("str", "tooooo long",
            "Unsatisfied length - string length \"11\" is not allowed.", "1");

    /* invalid pattern */
    TEST_TYPE_ERROR("str", "string15", "Unsatisfied pattern - \"string15\" does not conform to \"[a-z ]*\".", "1");
}

#endif

#if 0
static void
test_bits(void **state)
{
    struct lyd_node *tree;

    /* valid data */
    CHECK_PARSE_LYD("<bits xmlns=\"urn:tests:types\">\n two    \t\nzero\n  </bits>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "bits", 1, BITS, "zero two", "zero", "two");
    lyd_free_all(tree);

    CHECK_PARSE_LYD("<bits xmlns=\"urn:tests:types\">zero  two</bits>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "bits", 1, BITS, "zero two", "zero", "two");
    lyd_free_all(tree);

    /* disabled feature */
    TEST_TYPE_ERROR("bits", " \t one \n\t ", "Invalid bit value \"one\".", "2");

    /* disabled feature */
    TEST_TYPE_ERROR("bits",  "\t one \n\t", "Invalid bit value \"one\".", "2");

    /* multiple instances of the bit */
    TEST_TYPE_ERROR("bits", "one zero one", "Invalid bit value \"one\".", "1");

    /* invalid bit value */
    TEST_TYPE_ERROR("bits", "one xero one", "Invalid bit value \"one\".", "1");
}

#endif

static void
test_enums(void **state)
{
    struct lyd_node *tree;

    /* valid data */
    CHECK_PARSE_LYD("<enums xmlns=\"urn:tests:types\">white</enums>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "enums", 1, ENUM, "white", "white");
    lyd_free_all(tree);

    /* disabled feature */
    TEST_TYPE_ERROR("enums", "yellow", "Invalid enumeration value \"yellow\".", "1");

    /* leading/trailing whitespaces are not valid */
    TEST_TYPE_ERROR("enums", " white", "Invalid enumeration value \" white\".", "1");
    TEST_TYPE_ERROR("enums", "white\n", "Invalid enumeration value \"white\n\".", "2");

    /* invalid enumeration value */
    TEST_TYPE_ERROR("enums", "black", "Invalid enumeration value \"black\".", "1");
}

#if 0
static void
test_binary(void **state)
{
    struct lyd_node *tree;

    /* valid data (hello) */
    CHECK_PARSE_LYD("<binary xmlns=\"urn:tests:types\">\n   aGVs\nbG8=  \t\n  </binary>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "binary", 1, BINARY, "aGVs\nbG8=");
    lyd_free_all(tree);
    CHECK_PARSE_LYD("<binary-norestr xmlns=\"urn:tests:types\">TQ==</binary-norestr>", tree);
    assert_non_null(tree);
    tree = tree->next;
    assert_non_null(tree);
    TEST_PATTERN_1(tree, "binary-norestr", 1, BINARY, "TQ==");
    lyd_free_all(tree);

    /* no data */
    CHECK_PARSE_LYD("<binary-norestr xmlns=\"urn:tests:types\">\n    \t\n  </binary-norestr>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "binary-norestr", 1, BINARY, "");
    lyd_free_all(tree);

    CHECK_PARSE_LYD("<binary-norestr xmlns=\"urn:tests:types\"></binary-norestr>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "binary-norestr", 1, BINARY, "");
    lyd_free_all(tree);

    CHECK_PARSE_LYD("<binary-norestr xmlns=\"urn:tests:types\"/>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "binary-norestr", 1, BINARY, "");
    lyd_free_all(tree);

    /* invalid base64 character */
    TEST_TYPE_ERROR("binary-norestr", "a@bcd=", "Invalid Base64 character (@).", "1");

    /* missing data */
    TEST_TYPE_ERROR("binary-norestr", "aGVsbG8", "Base64 encoded value length must be divisible by 4.", "1");

    TEST_TYPE_ERROR("binary-norestr", "VsbG8=", "Base64 encoded value length must be divisible by 4.", "1");

    /* invalid binary length */
    /* helloworld */
    TEST_TYPE_ERROR("binary", "aGVsbG93b3JsZA==", "This base64 value must be of length 5.", "1");

    /* M */
    TEST_TYPE_ERROR("binary", "TQ==", "This base64 value must be of length 5.", "1");
}

#endif

static void
test_boolean(void **state)
{
    struct lyd_node *tree;

    /* valid data */
    CHECK_PARSE_LYD("<bool xmlns=\"urn:tests:types\">true</bool>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "bool", 1, BOOL, "true", 1);
    lyd_free_all(tree);

    CHECK_PARSE_LYD("<bool xmlns=\"urn:tests:types\">false</bool>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "bool", 1, BOOL, "false", 0);
    lyd_free_all(tree);

    CHECK_PARSE_LYD("<tbool xmlns=\"urn:tests:types\">false</tbool>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "tbool", 1, BOOL, "false", 0);
    lyd_free_all(tree);

    /* invalid value */
    TEST_TYPE_ERROR("bool", "unsure", "Invalid boolean value \"unsure\".", "1");

    TEST_TYPE_ERROR("bool", " true", "Invalid boolean value \" true\".", "1");
}

static void
test_empty(void **state)
{
    struct lyd_node *tree;

    /* valid data */
    CHECK_PARSE_LYD("<empty xmlns=\"urn:tests:types\"></empty>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "empty", 1, EMPTY, "");
    lyd_free_all(tree);

    CHECK_PARSE_LYD("<empty xmlns=\"urn:tests:types\"/>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "empty", 1, EMPTY, "");
    lyd_free_all(tree);

    CHECK_PARSE_LYD("<tempty xmlns=\"urn:tests:types\"/>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "tempty", 1, EMPTY, "");
    lyd_free_all(tree);

    /* invalid value */
    TEST_TYPE_ERROR("empty", "x", "Invalid empty value length 1.", "1");

    TEST_TYPE_ERROR("empty", " ", "Invalid empty value length 1.", "1");
}

static void
test_identityref(void **state)
{
    struct lyd_node *tree;
    struct lyd_node_term *leaf;
    const struct lys_module *mod_types;
    const struct lys_module *mod_defs;

    assert_non_null(mod_types = ly_ctx_get_module_implemented(UTEST_LYCTX, "types"));
    assert_non_null(mod_defs = ly_ctx_get_module_implemented(UTEST_LYCTX, "defs"));

    /* valid data */
    CHECK_PARSE_LYD("<ident xmlns=\"urn:tests:types\">gigabit-ethernet</ident>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "ident", 1, IDENT, "types:gigabit-ethernet", "gigabit-ethernet");
    leaf = (struct lyd_node_term *)tree;
    TEST_PRINTED_VALUE(&leaf->value, "t:gigabit-ethernet", LY_VALUE_SCHEMA, mod_types->parsed);
    lyd_free_all(tree);

    CHECK_PARSE_LYD("<ident xmlns=\"urn:tests:types\" xmlns:x=\"urn:tests:defs\">x:fast-ethernet</ident>", tree);
    assert_non_null(tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "ident", 1, IDENT, "defs:fast-ethernet", "fast-ethernet");
    leaf = (struct lyd_node_term *)tree;
    TEST_PRINTED_VALUE(&leaf->value, "d:fast-ethernet", LY_VALUE_SCHEMA, mod_defs->parsed);
    lyd_free_all(tree);

    /* invalid value */
    TEST_TYPE_ERROR("ident", "fast-ethernet",
            "Invalid identityref \"fast-ethernet\" value - identity not found in module \"types\".", "1");

    CHECK_PARSE_LYD_PARAM("<ident xmlns=\"urn:tests:types\" xmlns:x=\"urn:tests:defs\">x:slow-ethernet</ident>",
            LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid identityref \"x:slow-ethernet\" value - identity not found in module \"defs\".",
            "Schema location /types:ident, line number 1.");

    CHECK_PARSE_LYD_PARAM("<ident xmlns=\"urn:tests:types\" xmlns:x=\"urn:tests:defs\">x:crypto-alg</ident>",
            LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid identityref \"x:crypto-alg\" value - identity not derived from the base \"defs:interface-type\".",
            "Schema location /types:ident, line number 1.");

    CHECK_PARSE_LYD_PARAM("<ident xmlns=\"urn:tests:types\" xmlns:x=\"urn:tests:unknown\">x:fast-ethernet</ident>",
            LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid identityref \"x:fast-ethernet\" value - unable to map prefix to YANG schema.",
            "Schema location /types:ident, line number 1.");
}

/* dummy get_prefix callback for test_instanceid() */
const struct lys_module *
test_instanceid_getprefix(const struct ly_ctx *ctx, const char *prefix, size_t prefix_len, void * private)
{
    (void)ctx;
    (void)prefix;
    (void)prefix_len;

    return private;
}

static void
test_instanceid(void **state)
{
    struct lyd_node *tree;
    const struct lyd_node_term *leaf;
    const struct lys_module *mod_types;
    const char *data;

    assert_non_null(mod_types = ly_ctx_get_module_implemented(UTEST_LYCTX, "types"));

    /* valid data */
    CHECK_PARSE_LYD("<cont xmlns=\"urn:tests:types\"><leaftarget/></cont>"
            "<xdf:inst xmlns:xdf=\"urn:tests:types\">/xdf:cont/xdf:leaftarget</xdf:inst>", tree);
    tree = tree->prev;
    const enum ly_path_pred_type result_1[] = {LY_PATH_PREDTYPE_NONE, LY_PATH_PREDTYPE_NONE};

    TEST_PATTERN_1(tree, "inst", 1, INST, "/types:cont/leaftarget", result_1);
    leaf = (struct lyd_node_term *)tree;
    CHECK_LYSC_NODE(leaf->value.target[0].node, NULL, 0, 0x5, 1, "cont", 1, LYS_CONTAINER, 0, 0, NULL, 0);
    CHECK_LYSC_NODE(leaf->value.target[1].node, NULL, 0, 0x5, 1, "leaftarget", 1, LYS_LEAF, 1, 0, NULL, 0);
    lyd_free_all(tree);

    CHECK_PARSE_LYD("<list xmlns=\"urn:tests:types\"><id>a</id></list><list xmlns=\"urn:tests:types\"><id>b</id></list>"
            "<xdf:inst xmlns:xdf=\"urn:tests:types\">/xdf:list[xdf:id='b']/xdf:id</xdf:inst>", tree);
    tree = tree->prev;
    const enum ly_path_pred_type result_2[] = {LY_PATH_PREDTYPE_LIST, LY_PATH_PREDTYPE_NONE};

    TEST_PATTERN_1(tree, "inst", 1, INST, "/types:list[id='b']/id", result_2);
    leaf = (struct lyd_node_term *)tree;
    assert_int_equal(2, LY_ARRAY_COUNT(leaf->value.target));
    CHECK_LYSC_NODE(leaf->value.target[0].node, NULL, 0,  0x85, 1, "list", 1, LYS_LIST, 0, 0, NULL, 0);
    CHECK_LYSC_NODE(leaf->value.target[1].node, NULL, 0, 0x105, 1,   "id", 1, LYS_LEAF, 1, 0, NULL, 0);
    lyd_free_all(tree);

    CHECK_PARSE_LYD("<leaflisttarget xmlns=\"urn:tests:types\">1</leaflisttarget><leaflisttarget xmlns=\"urn:tests:types\">2</leaflisttarget>"
            "<xdf:inst xmlns:xdf=\"urn:tests:types\">/xdf:leaflisttarget[.='1']</xdf:inst>", tree);
    tree = tree->prev;
    const enum ly_path_pred_type result_3[] = {LY_PATH_PREDTYPE_LEAFLIST};

    TEST_PATTERN_1(tree, "inst", 1, INST, "/types:leaflisttarget[.='1']", result_3);
    leaf = (struct lyd_node_term *)tree;
    CHECK_LYSC_NODE(leaf->value.target[0].node, NULL, 0, 0x85, 1, "leaflisttarget", 1, LYS_LEAFLIST, 0, 0, NULL, 0);
    lyd_free_all(tree);

    data = "<list_inst xmlns=\"urn:tests:types\"><id xmlns:b=\"urn:tests:types\">/b:leaflisttarget[.='a']</id><value>x</value></list_inst>"
            "<list_inst xmlns=\"urn:tests:types\"><id xmlns:b=\"urn:tests:types\">/b:leaflisttarget[.='b']</id><value>y</value></list_inst>"
            "<leaflisttarget xmlns=\"urn:tests:types\">a</leaflisttarget><leaflisttarget xmlns=\"urn:tests:types\">b</leaflisttarget>"
            "<a:inst xmlns:a=\"urn:tests:types\">/a:list_inst[a:id=\"/a:leaflisttarget[.='b']\"]/a:value</a:inst>";
    CHECK_PARSE_LYD(data, tree);
    tree = tree->prev;
    const enum ly_path_pred_type result_4[] = {LY_PATH_PREDTYPE_LIST, LY_PATH_PREDTYPE_NONE};

    TEST_PATTERN_1(tree, "inst", 1, INST, "/types:list_inst[id=\"/types:leaflisttarget[.='b']\"]/value", result_4);
    leaf = (struct lyd_node_term *)tree;
    assert_int_equal(2, LY_ARRAY_COUNT(leaf->value.target));
    CHECK_LYSC_NODE(leaf->value.target[0].node, NULL, 0, 0x85, 1, "list_inst", 1, LYS_LIST, 0, 0, NULL, 0);
    CHECK_LYSC_NODE(leaf->value.target[1].node, NULL, 0,  0x5, 1, "value",     0, LYS_LEAF, 1, 0, NULL, 0);

    assert_int_equal(1, LY_ARRAY_COUNT(leaf->value.target[0].predicates));
    assert_null(leaf->value.target[1].predicates);
    TEST_PRINTED_VALUE(&leaf->value, "/t:list_inst[t:id=\"/t:leaflisttarget[.='b']\"]/t:value", LY_VALUE_SCHEMA, mod_types->parsed);
    TEST_PRINTED_VALUE(&leaf->value, "/types:list_inst[id=\"/types:leaflisttarget[.='b']\"]/value", LY_VALUE_JSON, NULL);
    lyd_free_all(tree);

    data = "<list xmlns=\"urn:tests:types\"><id>a</id></list><list xmlns=\"urn:tests:types\"><id>b</id><value>x</value></list>"
            "<xdf:inst xmlns:xdf=\"urn:tests:types\">/xdf:list[xdf:id='b']/xdf:value</xdf:inst>";
    CHECK_PARSE_LYD(data, tree);
    tree = tree->prev;
    const enum ly_path_pred_type result_5[] = {LY_PATH_PREDTYPE_LIST, LY_PATH_PREDTYPE_NONE};

    TEST_PATTERN_1(tree, "inst", 1, INST, "/types:list[id='b']/value", result_5);
    leaf = (struct lyd_node_term *)tree;
    assert_int_equal(2, LY_ARRAY_COUNT(leaf->value.target));
    CHECK_LYSC_NODE(leaf->value.target[0].node, NULL, 0, 0x85, 1,  "list", 1, LYS_LIST, 0, 0, NULL, 0);
    CHECK_LYSC_NODE(leaf->value.target[1].node, NULL, 0,  0x5, 1, "value", 1, LYS_LEAF, 1, 0, NULL, 0);
    lyd_free_all(tree);

    data = "<list_inst xmlns=\"urn:tests:types\"><id xmlns:b=\"urn:tests:types\">/b:leaflisttarget[.='a']</id><value>x</value></list_inst>"
            "<list_inst xmlns=\"urn:tests:types\"><id xmlns:b=\"urn:tests:types\">/b:leaflisttarget[.='b']</id><value>y</value></list_inst>"
            "<leaflisttarget xmlns=\"urn:tests:types\">a</leaflisttarget><leaflisttarget xmlns=\"urn:tests:types\">b</leaflisttarget>"
            "<a:inst xmlns:a=\"urn:tests:types\">/a:list_inst[a:id=\"/a:leaflisttarget[.='a']\"]/a:value</a:inst>";
    CHECK_PARSE_LYD(data, tree);
    tree = tree->prev;
    const enum ly_path_pred_type result_6[] = {LY_PATH_PREDTYPE_LIST, LY_PATH_PREDTYPE_NONE};

    TEST_PATTERN_1(tree, "inst", 1, INST, "/types:list_inst[id=\"/types:leaflisttarget[.='a']\"]/value", result_6);
    leaf = (struct lyd_node_term *)tree;
    assert_int_equal(2, LY_ARRAY_COUNT(leaf->value.target));
    CHECK_LYSC_NODE(leaf->value.target[0].node, NULL, 0, 0x85, 1, "list_inst", 1, LYS_LIST, 0, 0, NULL, 0);
    CHECK_LYSC_NODE(leaf->value.target[1].node, NULL, 0,  0x5, 1, "value", 0, LYS_LEAF, 1, 0, NULL, 0);
    lyd_free_all(tree);

    data = "<list_ident xmlns=\"urn:tests:types\"><id xmlns:dfs=\"urn:tests:defs\">dfs:ethernet</id><value>x</value></list_ident>"
            "<list_ident xmlns=\"urn:tests:types\"><id xmlns:dfs=\"urn:tests:defs\">dfs:fast-ethernet</id><value>y</value></list_ident>"
            "<a:inst xmlns:a=\"urn:tests:types\" xmlns:d=\"urn:tests:defs\">/a:list_ident[a:id='d:fast-ethernet']/a:value</a:inst>";
    CHECK_PARSE_LYD(data, tree);
    tree = tree->prev;
    const enum ly_path_pred_type result_7[] = {LY_PATH_PREDTYPE_LIST, LY_PATH_PREDTYPE_NONE};

    TEST_PATTERN_1(tree, "inst", 1, INST, "/types:list_ident[id='defs:fast-ethernet']/value", result_7);
    leaf = (struct lyd_node_term *)tree;
    assert_int_equal(2, LY_ARRAY_COUNT(leaf->value.target));
    CHECK_LYSC_NODE(leaf->value.target[0].node, NULL, 0, 0x85, 1, "list_ident", 1, LYS_LIST, 0, 0, NULL, 0);
    CHECK_LYSC_NODE(leaf->value.target[1].node, NULL, 0,  0x5, 1, "value", 0, LYS_LEAF, 1, 0, NULL, 0);
    lyd_free_all(tree);

    data = "<list2 xmlns=\"urn:tests:types\"><id>types:xxx</id><value>x</value></list2>"
            "<list2 xmlns=\"urn:tests:types\"><id>a:xxx</id><value>y</value></list2>"
            "<a:inst xmlns:a=\"urn:tests:types\">/a:list2[a:id='a:xxx'][a:value='y']/a:value</a:inst>";
    CHECK_PARSE_LYD(data, tree);
    tree = tree->prev;
    const enum ly_path_pred_type result_8[] = {LY_PATH_PREDTYPE_LIST, LY_PATH_PREDTYPE_NONE};

    TEST_PATTERN_1(tree, "inst", 1, INST, "/types:list2[id='a:xxx'][value='y']/value", result_8);
    leaf = (struct lyd_node_term *)tree;
    assert_int_equal(2, LY_ARRAY_COUNT(leaf->value.target));
    CHECK_LYSC_NODE(leaf->value.target[0].node, NULL, 0, 0x85, 1, "list2", 1, LYS_LIST, 0, 0, NULL, 0);
    CHECK_LYSC_NODE(leaf->value.target[1].node, NULL, 0, 0x105, 1, "value", 0, LYS_LEAF, 1, 0, NULL, 0);
    lyd_free_all(tree);

    data = "<list xmlns=\"urn:tests:types\"><id>types:xxx</id><value>x</value></list>"
            "<list xmlns=\"urn:tests:types\"><id>a:xxx</id><value>y</value></list>"
            "<a:inst xmlns:a=\"urn:tests:types\">/a:list[a:id='a:xxx']/a:value</a:inst>";
    CHECK_PARSE_LYD(data, tree);
    tree = tree->prev;
    const enum ly_path_pred_type result_9[] = {LY_PATH_PREDTYPE_LIST, LY_PATH_PREDTYPE_NONE};

    TEST_PATTERN_1(tree, "inst", 1, INST, "/types:list[id='a:xxx']/value", result_9);
    leaf = (struct lyd_node_term *)tree;
    assert_int_equal(2, LY_ARRAY_COUNT(leaf->value.target));
    CHECK_LYSC_NODE(leaf->value.target[0].node, NULL, 0, 0x85, 1, "list", 1, LYS_LIST, 0, 0, NULL, 0);
    CHECK_LYSC_NODE(leaf->value.target[1].node, NULL, 0, 0x5, 1, "value", 1, LYS_LEAF, 1, 0, NULL, 0);
    lyd_free_all(tree);

    data = "<list2 xmlns=\"urn:tests:types\"><id>a</id><value>a</value></list2>"
            "<list2 xmlns=\"urn:tests:types\"><id>c</id><value>b</value></list2>"
            "<list2 xmlns=\"urn:tests:types\"><id>a</id><value>b</value></list2>"
            "<a:inst xmlns:a=\"urn:tests:types\">/a:list2[a:id='a'][a:value='b']/a:id</a:inst>";
    CHECK_PARSE_LYD(data, tree);
    tree = tree->prev;
    const enum ly_path_pred_type result_10[] = {LY_PATH_PREDTYPE_LIST, LY_PATH_PREDTYPE_NONE};

    TEST_PATTERN_1(tree, "inst", 1, INST, "/types:list2[id='a'][value='b']/id", result_10);
    leaf = (struct lyd_node_term *)tree;
    assert_int_equal(2, LY_ARRAY_COUNT(leaf->value.target));
    CHECK_LYSC_NODE(leaf->value.target[0].node, NULL, 0, 0x85, 1, "list2", 1, LYS_LIST, 0, 0, NULL, 0);
    CHECK_LYSC_NODE(leaf->value.target[1].node, NULL, 0, 0x105, 1,  "id", 1, LYS_LEAF, 1, 0, NULL, 0);
    assert_non_null(leaf = lyd_target(leaf->value.target, tree));
    assert_string_equal("a", lyd_get_value(&leaf->node));
    assert_string_equal("b", lyd_get_value(leaf->next));
    lyd_free_all(tree);

    /* invalid value */
    data = "<list xmlns=\"urn:tests:types\"><id>a</id></list>"
            "<list xmlns=\"urn:tests:types\"><id>b</id><value>x</value></list>"
            "<xdf:inst xmlns:xdf=\"urn:tests:types\">/xdf:list[2]/xdf:value</xdf:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/xdf:list[2]/xdf:value\" value - semantic error.", "Schema location /types:inst.");

    data = "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:1leaftarget</t:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:cont/t:1leaftarget\" value - syntax error.", "Schema location /types:inst, line number 1.");

    data = "<t:inst xmlns:t=\"urn:tests:types\">/t:cont:t:1leaftarget</t:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:cont:t:1leaftarget\" value - syntax error.", "Schema location /types:inst, line number 1.");

    data = "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:invalid/t:path</t:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:cont/t:invalid/t:path\" value - semantic error.", "Schema location /types:inst.");

    data = "<inst xmlns=\"urn:tests:types\" xmlns:t=\"urn:tests:invalid\">/t:cont/t:leaftarget</inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:cont/t:leaftarget\" value - semantic error.", "Schema location /types:inst.");

    TEST_TYPE_ERROR("inst", "/cont/leaftarget", "Invalid instance-identifier \"/cont/leaftarget\" value - syntax error.", "1");

    /* instance-identifier is here in JSON format because it is already in internal representation without canonical prefixes */
    data = "<cont xmlns=\"urn:tests:types\"/><t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaftarget</t:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_ENOTFOUND, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/types:cont/leaftarget\" value - required instance not found.", "Schema location /types:inst, data location /types:inst.");

    /* instance-identifier is here in JSON format because it is already in internal representation without canonical prefixes */
    data = "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaftarget</t:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_ENOTFOUND, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/types:cont/leaftarget\" value - required instance not found.", "Schema location /types:inst, data location /types:inst.");

    data = "<leaflisttarget xmlns=\"urn:tests:types\">x</leaflisttarget>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:leaflisttarget[1</t:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:leaflisttarget[1\" value - syntax error.", "Schema location /types:inst, line number 1.");

    data = "<cont xmlns=\"urn:tests:types\"/><t:inst xmlns:t=\"urn:tests:types\">/t:cont[1]</t:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:cont[1]\" value - semantic error.", "Schema location /types:inst.");

    data = "<cont xmlns=\"urn:tests:types\"/><t:inst xmlns:t=\"urn:tests:types\">[1]</t:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"[1]\" value - syntax error.", "Schema location /types:inst, line number 1.");

    data = "<cont xmlns=\"urn:tests:types\"><leaflisttarget>1</leaflisttarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[id='1']</t:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:cont/t:leaflisttarget[id='1']\" value - syntax error.", "Schema location /types:inst, line number 1.");

    data = "<cont xmlns=\"urn:tests:types\"><leaflisttarget>1</leaflisttarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[t:id='1']</t:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:cont/t:leaflisttarget[t:id='1']\" value - semantic error.", "Schema location /types:inst.");

    data = "<cont xmlns=\"urn:tests:types\"><leaflisttarget>1</leaflisttarget><leaflisttarget>2</leaflisttarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[4]</t:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:cont/t:leaflisttarget[4]\" value - semantic error.", "Schema location /types:inst.");

    data = "<t:inst-noreq xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[6]</t:inst-noreq>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:cont/t:leaflisttarget[6]\" value - semantic error.", "Schema location /types:inst-noreq.");

    data = "<cont xmlns=\"urn:tests:types\"><listtarget><id>1</id><value>x</value></listtarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[t:value='x']</t:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:cont/t:listtarget[t:value='x']\" value - semantic error.", "Schema location /types:inst.");

    data = "<t:inst-noreq xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[t:value='x']</t:inst-noreq>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:cont/t:listtarget[t:value='x']\" value - semantic error.", "Schema location /types:inst-noreq.");

    data = "<t:inst-noreq xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[t:x='x']</t:inst-noreq>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_ENOTFOUND, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:cont/t:listtarget[t:x='x']\" value - semantic error.", "Schema location /types:inst-noreq.");

    data = "<cont xmlns=\"urn:tests:types\"><listtarget><id>1</id><value>x</value></listtarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[.='x']</t:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:cont/t:listtarget[.='x']\" value - semantic error.", "Schema location /types:inst.");

    /* instance-identifier is here in JSON format because it is already in internal representation without canonical prefixes */
    data = "<cont xmlns=\"urn:tests:types\"><leaflisttarget>1</leaflisttarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[.='2']</t:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_ENOTFOUND, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/types:cont/leaflisttarget[.='2']\" value - required instance not found.", "Schema location /types:inst, data location /types:inst.");

    data = "<cont xmlns=\"urn:tests:types\"><leaflisttarget>1</leaflisttarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:leaflisttarget[.='x']</t:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:cont/t:leaflisttarget[.='x']\" value - semantic error.", "Schema location /types:inst.");

    data = "<cont xmlns=\"urn:tests:types\"><listtarget><id>1</id><value>x</value></listtarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[t:id='x']</t:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/t:cont/t:listtarget[t:id='x']\" value - semantic error.", "Schema location /types:inst.");

    /* instance-identifier is here in JSON format because it is already in internal representation without canonical prefixes */
    data = "<cont xmlns=\"urn:tests:types\"><listtarget><id>1</id><value>x</value></listtarget></cont>"
            "<t:inst xmlns:t=\"urn:tests:types\">/t:cont/t:listtarget[t:id='2']</t:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_ENOTFOUND, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/types:cont/listtarget[id='2']\" value - required instance not found.", "Schema location /types:inst, data location /types:inst.");

    data = "<leaflisttarget xmlns=\"urn:tests:types\">a</leaflisttarget>"
            "<leaflisttarget xmlns=\"urn:tests:types\">b</leaflisttarget>"
            "<a:inst xmlns:a=\"urn:tests:types\">/a:leaflisttarget[1][2]</a:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/a:leaflisttarget[1][2]\" value - syntax error.", "Schema location /types:inst, line number 1.");

    data = "<leaflisttarget xmlns=\"urn:tests:types\">a</leaflisttarget>"
            "<leaflisttarget xmlns=\"urn:tests:types\">b</leaflisttarget>"
            "<a:inst xmlns:a=\"urn:tests:types\">/a:leaflisttarget[.='a'][.='b']</a:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/a:leaflisttarget[.='a'][.='b']\" value - syntax error.", "Schema location /types:inst, line number 1.");

    data = "<list xmlns=\"urn:tests:types\"><id>a</id><value>x</value></list>"
            "<list xmlns=\"urn:tests:types\"><id>b</id><value>y</value></list>"
            "<a:inst xmlns:a=\"urn:tests:types\">/a:list[a:id='a'][a:id='b']/a:value</a:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/a:list[a:id='a'][a:id='b']/a:value\" value - syntax error.", "Schema location /types:inst, line number 1.");

    data = "<list2 xmlns=\"urn:tests:types\"><id>a</id><value>x</value></list2>"
            "<list2 xmlns=\"urn:tests:types\"><id>b</id><value>y</value></list2>"
            "<a:inst xmlns:a=\"urn:tests:types\">/a:list2[a:id='a']/a:value</a:inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/a:list2[a:id='a']/a:value\" value - semantic error.", "Schema location /types:inst.");

    /* check for validting instance-identifier with a complete data tree */
    data = "<list2 xmlns=\"urn:tests:types\"><id>a</id><value>a</value></list2>"
            "<list2 xmlns=\"urn:tests:types\"><id>c</id><value>b</value></list2>"
            "<leaflisttarget xmlns=\"urn:tests:types\">a</leaflisttarget>"
            "<leaflisttarget xmlns=\"urn:tests:types\">b</leaflisttarget>"
            "<a:inst xmlns:a=\"urn:tests:types\">/a:list2[a:id='a'][a:value='a']/a:id</a:inst>";
    CHECK_PARSE_LYD(data, tree);
    /* key-predicate */
    data = "/types:list2[id='a'][value='b']/id";
    assert_int_equal(LY_ENOTFOUND, lyd_value_validate(UTEST_LYCTX, tree->prev->schema, data, strlen(data), tree->prev,
            NULL, NULL));
    CHECK_LOG_CTX("Invalid instance-identifier \"/types:list2[id='a'][value='b']/id\" value - required instance not found.", "Data location /types:inst.");
    /* leaf-list-predicate */
    data = "/types:leaflisttarget[.='c']";
    assert_int_equal(LY_ENOTFOUND, lyd_value_validate(UTEST_LYCTX, tree->prev->schema, data, strlen(data), tree->prev,
            NULL, NULL));
    CHECK_LOG_CTX("Invalid instance-identifier \"/types:leaflisttarget[.='c']\" value - required instance not found.", "Data location /types:inst.");
    /* position predicate */
    data = "/types:list_keyless[4]";
    assert_int_equal(LY_ENOTFOUND, lyd_value_validate(UTEST_LYCTX, tree->prev->schema, data, strlen(data), tree->prev,
            NULL, NULL));
    CHECK_LOG_CTX("Invalid instance-identifier \"/types:list_keyless[4]\" value - required instance not found.", "Data location /types:inst.");

    lyd_free_all(tree);

    data = "<leaflisttarget xmlns=\"urn:tests:types\">b</leaflisttarget>"
            "<inst xmlns=\"urn:tests:types\">/a:leaflisttarget[1]</inst>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid instance-identifier \"/a:leaflisttarget[1]\" value - semantic error.", "Schema location /types:inst.");
}

static void
test_leafref(void **state)
{
    struct lyd_node *tree;
    struct lyd_node_term *leaf;
    const char *data;

    /* types:lref: /leaflisttarget */
    /* types:lref2: ../list[id = current()/../str-norestr]/targets */

    const char *schema = "module leafrefs {yang-version 1.1; namespace urn:tests:leafrefs; prefix lr; import types {prefix t;}"
            "container c { container x {leaf x {type string;}} list l {key \"id value\"; leaf id {type string;} leaf value {type string;}"
            "leaf lr1 {type leafref {path \"../../../t:str-norestr\"; require-instance true;}}"
            "leaf lr2 {type leafref {path \"../../l[id=current()/../../../t:str-norestr][value=current()/../../../t:str-norestr]/value\"; require-instance true;}}"
            "leaf lr3 {type leafref {path \"/t:list[t:id=current ( )/../../x/x]/t:targets\";}}"
            "}}}";

    /* additional schema */
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    /* valid data */
    data = "<leaflisttarget xmlns=\"urn:tests:types\">x</leaflisttarget>"
            "<leaflisttarget xmlns=\"urn:tests:types\">y</leaflisttarget>"
            "<lref xmlns=\"urn:tests:types\">y</lref>";
    CHECK_PARSE_LYD(data, tree);
    tree = tree->prev;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, 0x5, 1, "lref", 1, LYS_LEAF, 0, 0, NULL, 0);
    leaf = (struct lyd_node_term *)tree;
    CHECK_LYD_NODE_TERM(leaf, 0, 0, 0, 0, 1, STRING, "y");
    lyd_free_all(tree);

    data = "<list xmlns=\"urn:tests:types\"><id>x</id><targets>a</targets><targets>b</targets></list>"
            "<list xmlns=\"urn:tests:types\"><id>y</id><targets>x</targets><targets>y</targets></list>"
            "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr><lref2 xmlns=\"urn:tests:types\">y</lref2>";
    CHECK_PARSE_LYD(data, tree);
    tree = tree->prev;
    CHECK_LYSC_NODE(tree->schema, NULL, 0, 0x5, 1, "lref2", 1, LYS_LEAF, 0, 0, NULL, 0);
    leaf = (struct lyd_node_term *)tree;
    CHECK_LYD_NODE_TERM(leaf, 0, 0, 0, 0, 1, STRING, "y");
    lyd_free_all(tree);

    data = "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr>"
            "<c xmlns=\"urn:tests:leafrefs\"><l><id>x</id><value>x</value><lr1>y</lr1></l></c>";
    CHECK_PARSE_LYD(data, tree);
    CHECK_LYSC_NODE(tree->schema, NULL, 0, 0x5, 1, "c", 0, LYS_CONTAINER, 0, 0, NULL, 0);
    leaf = (struct lyd_node_term *)(lyd_child(lyd_child(tree)->next)->prev);
    CHECK_LYSC_NODE(leaf->schema, NULL, 0, 0x5, 1, "lr1", 1, LYS_LEAF, 1, 0, NULL, 0);
    CHECK_LYD_NODE_TERM(leaf, 0, 0, 0, 1, 1, STRING, "y");
    lyd_free_all(tree);

    data = "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr>"
            "<c xmlns=\"urn:tests:leafrefs\"><l><id>y</id><value>y</value></l>"
            "<l><id>x</id><value>x</value><lr2>y</lr2></l></c>";
    CHECK_PARSE_LYD(data, tree);
    CHECK_LYSC_NODE(tree->schema, NULL, 0, 0x5, 1, "c", 0, LYS_CONTAINER, 0, 0, NULL, 0);
    leaf = (struct lyd_node_term *)(lyd_child(lyd_child(tree)->prev)->prev);
    CHECK_LYSC_NODE(leaf->schema, NULL, 0, 0x5, 1, "lr2", 1, LYS_LEAF, 1, 0, NULL, 0);
    CHECK_LYD_NODE_TERM(leaf, 0, 0, 0, 1, 1, STRING, "y");
    lyd_free_all(tree);

    data = "<list xmlns=\"urn:tests:types\"><id>x</id><targets>a</targets><targets>b</targets></list>"
            "<list xmlns=\"urn:tests:types\"><id>y</id><targets>c</targets><targets>d</targets></list>"
            "<c xmlns=\"urn:tests:leafrefs\"><x><x>y</x></x>"
            "<l><id>x</id><value>x</value><lr3>c</lr3></l></c>";
    CHECK_PARSE_LYD(data, tree);
    CHECK_LYSC_NODE(tree->schema, NULL, 0, 0x5, 1, "c", 0, LYS_CONTAINER, 0, 0, NULL, 0);
    leaf = (struct lyd_node_term *)(lyd_child(lyd_child(tree)->prev)->prev);
    CHECK_LYSC_NODE(leaf->schema, NULL, 0, 0x5, 1, "lr3", 0, LYS_LEAF, 1, 0, NULL, 0);
    CHECK_LYD_NODE_TERM(leaf, 0, 0, 0, 1, 1, STRING, "c");
    lyd_free_all(tree);

    /* invalid value */
    data = "<leaflisttarget xmlns=\"urn:tests:types\">x</leaflisttarget><lref xmlns=\"urn:tests:types\">y</lref>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid leafref value \"y\" - "
            "no target instance \"/leaflisttarget\" with the same value.",
            "Schema location /types:lref, data location /types:lref.");

    data = "<list xmlns=\"urn:tests:types\"><id>x</id><targets>a</targets><targets>b</targets></list>"
            "<list xmlns=\"urn:tests:types\"><id>y</id><targets>x</targets><targets>y</targets></list>"
            "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr><lref2 xmlns=\"urn:tests:types\">b</lref2>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid leafref value \"b\" - "
            "no target instance \"../list[id = current()/../str-norestr]/targets\" with the same value.",
            "Schema location /types:lref2, data location /types:lref2.");

    data = "<list xmlns=\"urn:tests:types\"><id>x</id><targets>a</targets><targets>b</targets></list>"
            "<list xmlns=\"urn:tests:types\"><id>y</id><targets>x</targets><targets>y</targets></list>"
            "<lref2 xmlns=\"urn:tests:types\">b</lref2>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid leafref value \"b\" - "
            "no existing target instance \"../list[id = current()/../str-norestr]/targets\".",
            "Schema location /types:lref2, data location /types:lref2.");

    data = "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr><lref2 xmlns=\"urn:tests:types\">b</lref2>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid leafref value \"b\" - "
            "no existing target instance \"../list[id = current()/../str-norestr]/targets\".",
            "Schema location /types:lref2, data location /types:lref2.");

    data = "<str-norestr xmlns=\"urn:tests:types\">y</str-norestr>"
            "<c xmlns=\"urn:tests:leafrefs\"><l><id>x</id><value>x</value><lr1>a</lr1></l></c>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid leafref value \"a\" - no target instance \"../../../t:str-norestr\" with the same value.",
            "Schema location /leafrefs:c/l/lr1, data location /leafrefs:c/l[id='x'][value='x']/lr1.");

    data = "<str-norestr xmlns=\"urn:tests:types\">z</str-norestr>"
            "<c xmlns=\"urn:tests:leafrefs\"><l><id>y</id><value>y</value></l>"
            "<l><id>x</id><value>x</value><lr2>z</lr2></l></c>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    CHECK_LOG_CTX("Invalid leafref value \"z\" - no existing target instance \"../../l[id=current()/../../../t:str-norestr]"
            "[value=current()/../../../t:str-norestr]/value\".",
            "Schema location /leafrefs:c/l/lr2, data location /leafrefs:c/l[id='x'][value='x']/lr2.");
}

static void
test_union(void **state)
{
    struct lyd_node *tree;
    struct lyd_node_term *leaf;
    const char *data;
    const struct lys_module *mod_defs;

    assert_non_null(mod_defs = ly_ctx_get_module_implemented(UTEST_LYCTX, "defs"));

    /*
     * leaf un1 {type union {
     *             type leafref {path /int8; require-instance true;}
     *             type union {
     *               type identityref {base defs:interface-type;}
     *               type instance-identifier {require-instance true;}
     *             }
     *             type string {range 1..20;};
     *           }
     * }
     */

    /* valid data */
    data = "<int8 xmlns=\"urn:tests:types\">12</int8><un1 xmlns=\"urn:tests:types\">12</un1>";
    CHECK_PARSE_LYD(data, tree);
    tree = tree->next->next;
    TEST_PATTERN_1(tree, "un1", 0, UNION, "12", INT8, "12", 12);
    leaf = (struct lyd_node_term *)tree;
    assert_int_equal(((struct ly_set *)leaf->value.subvalue->prefix_data)->count, 0);
    TEST_PRINTED_VALUE(&leaf->value, "12", LY_VALUE_SCHEMA, NULL);
    lyd_free_all(tree);

    data = "<int8 xmlns=\"urn:tests:types\">12</int8><un1 xmlns=\"urn:tests:types\">2</un1>";
    CHECK_PARSE_LYD(data, tree);
    tree = tree->next->next;
    TEST_PATTERN_1(tree, "un1", 0, UNION, "2", STRING, "2");
    leaf = (struct lyd_node_term *)tree;
    assert_int_equal(((struct ly_set *)leaf->value.subvalue->prefix_data)->count, 0);
    TEST_PRINTED_VALUE(&leaf->value, "2", LY_VALUE_SCHEMA, NULL);
    lyd_free_all(tree);

    data = "<un1 xmlns=\"urn:tests:types\" xmlns:x=\"urn:tests:defs\">x:fast-ethernet</un1>";
    CHECK_PARSE_LYD(data, tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "un1", 0, UNION, "defs:fast-ethernet", IDENT, "defs:fast-ethernet", "fast-ethernet");
    leaf = (struct lyd_node_term *)tree;
    assert_int_equal(((struct ly_set *)leaf->value.subvalue->prefix_data)->count, 1);
    TEST_PRINTED_VALUE(&leaf->value, "d:fast-ethernet", LY_VALUE_SCHEMA, mod_defs->parsed);
    TEST_PRINTED_VALUE(&leaf->value.subvalue->value, "d:fast-ethernet", LY_VALUE_SCHEMA, mod_defs->parsed);
    lyd_free_all(tree);

    data = "<un1 xmlns=\"urn:tests:types\" xmlns:d=\"urn:tests:defs\">d:superfast-ethernet</un1>";
    CHECK_PARSE_LYD(data, tree);
    tree = tree->next;
    TEST_PATTERN_1(tree, "un1", 0, UNION, "d:superfast-ethernet", STRING, "d:superfast-ethernet");
    leaf = (struct lyd_node_term *)tree;
    assert_int_equal(((struct ly_set *)leaf->value.subvalue->prefix_data)->count, 1);
    lyd_free_all(tree);

    data = "<leaflisttarget xmlns=\"urn:tests:types\">x</leaflisttarget><leaflisttarget xmlns=\"urn:tests:types\">y</leaflisttarget>"
            "<un1 xmlns=\"urn:tests:types\" xmlns:a=\"urn:tests:types\">/a:leaflisttarget[.='y']</un1>";
    CHECK_PARSE_LYD(data, tree);
    tree = tree->prev;
    const enum ly_path_pred_type result_1[] = {LY_PATH_PREDTYPE_LEAFLIST};

    TEST_PATTERN_1(tree, "un1", 0, UNION, "/types:leaflisttarget[.='y']", INST, "/types:leaflisttarget[.='y']", result_1);
    leaf = (struct lyd_node_term *)tree;

    assert_int_equal(((struct ly_set *)leaf->value.subvalue->prefix_data)->count, 1);
    lyd_free_all(tree);

    data = "<leaflisttarget xmlns=\"urn:tests:types\">x</leaflisttarget><leaflisttarget xmlns=\"urn:tests:types\">y</leaflisttarget>"
            "<un1 xmlns=\"urn:tests:types\" xmlns:a=\"urn:tests:types\">/a:leaflisttarget[3]</un1>";
    CHECK_PARSE_LYD(data, tree);
    tree = tree->prev;
    TEST_PATTERN_1(tree, "un1", 0, UNION, "/a:leaflisttarget[3]", STRING, "/a:leaflisttarget[3]");
    leaf = (struct lyd_node_term *)tree;
    assert_int_equal(((struct ly_set *)leaf->value.subvalue->prefix_data)->count, 1);
    lyd_free_all(tree);

    TEST_TYPE_ERROR("un1", "123456789012345678901",
            "Invalid union value \"123456789012345678901\" - no matching subtype found.", "1");
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_int, setup),
        UTEST(test_uint, setup),
        UTEST(test_dec64, setup),
        /* UTEST(test_string, setup),*/
        /* UTEST(test_bits, setup), */
        UTEST(test_enums, setup),
        /* UTEST(test_binary, setup), */
        UTEST(test_boolean, setup),
        UTEST(test_empty, setup),
        UTEST(test_identityref, setup),
        UTEST(test_instanceid, setup),
        UTEST(test_leafref, setup),
        UTEST(test_union, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
