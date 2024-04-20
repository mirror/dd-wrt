/**
 * @file bits.c
 * @author Radek Iša <isa@cesnet.cz>
 * @brief test for int8 values
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
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

/* GLOBAL INCLUDE HEADERS */
#include <ctype.h>

/* LOCAL INCLUDE HEADERS */
#include "libyang.h"
#include "path.h"

#define MODULE_CREATE_YIN(MOD_NAME, NODES) \
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" \
    "<module name=\"" MOD_NAME "\"\n" \
    "        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n" \
    "        xmlns:pref=\"urn:tests:" MOD_NAME "\">\n" \
    "  <yang-version value=\"1.1\"/>\n" \
    "  <namespace uri=\"urn:tests:" MOD_NAME "\"/>\n" \
    "  <prefix value=\"pref\"/>\n" \
    NODES \
    "</module>\n"

#define MODULE_CREATE_YANG(MOD_NAME, NODES) \
    "module " MOD_NAME " {\n" \
    "  yang-version 1.1;\n" \
    "  namespace \"urn:tests:" MOD_NAME "\";\n" \
    "  prefix pref;\n" \
    NODES \
    "}\n"

#define TEST_SUCCESS_XML(MOD_NAME, DATA, TYPE, ...) \
    { \
        struct lyd_node *tree; \
        const char *data = "<port xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</port>"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree); \
        CHECK_LYSC_NODE(tree->schema, NULL, 0, 0x5, 1, "port", 0, LYS_LEAF, 0, 0, 0, 0); \
        CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 0, 0, 1, TYPE, __VA_ARGS__); \
        lyd_free_all(tree); \
    }

#define TEST_SUCCESS_JSON(MOD_NAME, DATA, TYPE, ...) \
    { \
        struct lyd_node *tree; \
        const char *data = "{\"" MOD_NAME ":port\":\"" DATA "\"}"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree); \
        CHECK_LYSC_NODE(tree->schema, NULL, 0, 0x5, 1, "port", 0, LYS_LEAF, 0, 0, 0, 0); \
        CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 0, 0, 1, TYPE, __VA_ARGS__); \
        lyd_free_all(tree); \
    }

#define TEST_SUCCESS_LYB(MOD_NAME, NODE_NAME, DATA) \
    { \
        struct lyd_node *tree_1; \
        struct lyd_node *tree_2; \
        char *xml_out, *data; \
        data = "<" NODE_NAME " xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</" NODE_NAME ">"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, LY_SUCCESS, tree_1); \
        assert_int_equal(lyd_print_mem(&xml_out, tree_1, LYD_LYB, LYD_PRINT_WITHSIBLINGS), 0); \
        assert_int_equal(LY_SUCCESS, lyd_parse_data_mem(UTEST_LYCTX, xml_out, LYD_LYB, LYD_PARSE_ONLY | LYD_PARSE_STRICT, 0, &tree_2)); \
        assert_non_null(tree_2); \
        CHECK_LYD(tree_1, tree_2); \
        free(xml_out); \
        lyd_free_all(tree_1); \
        lyd_free_all(tree_2); \
    }

#define TEST_ERROR_XML(MOD_NAME, DATA) \
    {\
        struct lyd_node *tree; \
        const char *data = "<port xmlns=\"urn:tests:" MOD_NAME "\">" DATA "</port>"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree); \
        assert_null(tree); \
    }

#define TEST_ERROR_JSON(MOD_NAME, DATA) \
    { \
        struct lyd_node *tree; \
        const char *data = "{\"" MOD_NAME ":port\":" DATA "}"; \
        CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree); \
        assert_null(tree); \
    }

static void
test_schema_yang(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lysc_node_leaf *lysc_leaf;
    struct lysp_node_leaf *lysp_leaf;
    struct lysc_type_bits *lysc_type;

    schema = MODULE_CREATE_YANG("T0", "leaf port {type bits { bit zero;\nbit one;"
            " bit ten{position 10;}\tbit \"eleven\"; bit last{position 4294967295; }}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    lysc_type = (struct lysc_type_bits *) lysc_leaf->type;
    CHECK_LYSC_TYPE_BITS(lysc_type, 0, 5);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[0]), 0,  NULL, 0, 0, "zero", NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[1]), 1,  NULL, 0, 0, "one", NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[2]), 10, NULL, 0, 0, "ten", NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[3]), 11, NULL, 0, 0, "eleven", NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[4]), 4294967295, NULL, 0, 0, "last", NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 5, 0, 0, 0, 0x02, 0, 0, "bits", 0, 0, 1, 0, 0, 0);

    schema = MODULE_CREATE_YANG("T1", "leaf port {type bits { bit _ten {position 10;} bit _ten-one;"
            " bit _two {position 2;} bit ten_end...;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    lysc_type = (struct lysc_type_bits *) lysc_leaf->type;
    CHECK_LYSC_TYPE_BITS(lysc_type, 0, 4);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[0]), 2,   NULL, 0, 0, "_two",     NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[1]), 10,  NULL, 0, 0, "_ten",     NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[2]), 11,  NULL, 0, 0, "_ten-one", NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[3]), 12,  NULL, 0, 0, "ten_end...",  NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 4, 0, 0, 0, 0x02, 0, 0, "bits", 0, 0, 1, 0, 0, 0);

    /* TEST MODULE SUBTYPE */
    schema = MODULE_CREATE_YANG("T2", "typedef my_type{type bits {"
            " bit ten {position 10;} bit eleven; bit two {position 2;} bit twelve;}}"
            "leaf port {type my_type;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    lysc_type = (struct lysc_type_bits *) lysc_leaf->type;
    CHECK_LYSC_TYPE_BITS(lysc_type, 0, 4);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[0]), 2,   NULL, 0, 0, "two",     NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[1]), 10,  NULL, 0, 0, "ten",     NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[2]), 11,  NULL, 0, 0, "eleven", NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[3]), 12,  NULL, 0, 0, "twelve",  NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0, 0, 0, "my_type", 0, 0, 1, 0, 0, 0);

    schema = MODULE_CREATE_YANG("T3", "typedef my_type{type bits {"
            "   bit ten {position 10;} bit eleven; bit two {position 2;} bit twelve;}}"
            "leaf port {type my_type {"
            "   bit ten {position 10;} bit two;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    lysc_type = (struct lysc_type_bits *) lysc_leaf->type;
    CHECK_LYSC_TYPE_BITS(lysc_type, 0, 2);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[0]), 2,   NULL, 0, 0, "two",     NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[1]), 10,  NULL, 0, 0, "ten",     NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 2, 0, 0, 0, 0x02, 0, 0, "my_type", 0, 0, 1, 0, 0, 0);

    /*
     * TEST ERROR
     */
    /* test change bit possition */
    schema = MODULE_CREATE_YANG("TERR_0", "typedef my_type{type bits {"
            "   bit ten {position 10;} bit eleven; bit two {position 2;} bit \"twelve\";}}"
            "leaf port {type my_type {"
            "   bit ten {position 11;} bit two;}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid bits - position of the item \"ten\" has changed from 10 to 11 in the derived type.",
            "/TERR_0:port", 0);

    /* add new bit */
    schema = MODULE_CREATE_YANG("TERR_1", "typedef my_type{type bits {"
            "   bit ten {position 10;} bit eleven; bit two {position 2;} bit twelve;}}"
            "leaf port {type my_type {"
            "   bit ten {position 10;} bit two;  bit test;}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid bits - derived type adds new item \"test\".", "/TERR_1:port", 0);

    /* different max value => autoadd index */
    schema = MODULE_CREATE_YANG("TERR_2", "leaf port {type bits {"
            " bit first {position -1;} bit second;"
            "}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"-1\" of \"position\".", NULL, 5);

    /* different max value => autoadd index */
    schema = MODULE_CREATE_YANG("TERR_3", "leaf port {type bits {"
            " bit first {position 4294967295;} bit second;"
            "}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid bits - it is not possible to auto-assign bit position for \"second\" since the highest value is already 4294967295.",
            "/TERR_3:port", 0);

    schema = MODULE_CREATE_YANG("TERR_4", "leaf port {type bits {"
            " bit first {position 10;} bit \"\";"
            "}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_4\" failed.", NULL, 0);
    CHECK_LOG_CTX("Statement argument is required.", NULL, 5);

    /* wrong character */
    schema = MODULE_CREATE_YANG("TERR_5", "leaf port {type bits {"
            " bit first {position 10;} bit abcd^;"
            "}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_5\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid identifier character '^' (0x005e).", NULL, 5);

    schema = MODULE_CREATE_YANG("TERR_6", "leaf port {type bits {"
            " bit hi; bit hi;"
            "}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_6\" failed.", NULL, 0);
    CHECK_LOG_CTX("Duplicate identifier \"hi\" of bit statement.", NULL, 5);

    /* wrong character */
    schema = MODULE_CREATE_YANG("TERR_7", "leaf port {type bits {"
            " bit first {position 10;} bit \"ab&cd\";"
            "}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_7\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid identifier character '&' (0x0026).", NULL, 5);

    schema = MODULE_CREATE_YANG("TERR_8", "leaf port {type bits {"
            " bit first {position 10;} bit \"4abcd\";"
            "}}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_8\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid identifier first character '4' (0x0034).", NULL, 5);

    schema = MODULE_CREATE_YANG("TERR_9", "leaf port {type bits;}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Missing bit substatement for bits type.", "/TERR_9:port", 0);

    /* new features of YANG 1.1 in YANG 1.0 */
    schema = "module TERR_10 {"
            "  namespace \"urn:tests:TERR_10\";"
            "  prefix pref;"
            "  feature f;"
            "  leaf l {type bits {"
            "    bit one {if-feature f;}"
            "  }}"
            "}";
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_10\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid keyword \"if-feature\" as a child of \"bit\" - the statement is allowed only in YANG 1.1 modules.",
            NULL, 1);

    schema = "module TERR_11 {"
            "  namespace \"urn:tests:TERR_10\";"
            "  prefix pref;"
            "  typedef mytype {type bits {bit one;}}"
            "  leaf l {type mytype {bit one;}}"
            "}";
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Bits type can be subtyped only in YANG 1.1 modules.", "/TERR_11:l", 0);

    /* feature is not present */
    schema = MODULE_CREATE_YANG("IF_0", "feature f;"
            "leaf port {type bits { bit zero;\nbit one;"
            " bit ten{if-feature f; position 10;}\tbit eleven;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    assert_int_equal(LY_ENOT, lys_feature_value (mod, "f"));
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    lysc_type = (struct lysc_type_bits *) lysc_leaf->type;
    CHECK_LYSC_TYPE_BITS(lysc_type, 0, 3);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[0]), 0,  NULL, 0, 0, "zero", NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[1]), 1,  NULL, 0, 0, "one", NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[2]), 11, NULL, 0, 0, "eleven", NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 4, 0, 0, 0, 0x02, 0, 0, "bits", 0, 0, 1, 0, 0, 0);

    /* feature is present */
    schema = MODULE_CREATE_YANG("IF_1", "feature f;"
            "leaf port {type bits { bit zero;\nbit one;"
            " bit ten{position 10; if-feature f;}\tbit eleven;}}");
    const char *IF_1_FEATURES[] = {"f", NULL};

    UTEST_ADD_MODULE(schema, LYS_IN_YANG, IF_1_FEATURES, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    lysc_type = (struct lysc_type_bits *) lysc_leaf->type;
    CHECK_LYSC_TYPE_BITS(lysc_type, 0, 4);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[0]), 0,  NULL, 0, 0, "zero", NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[1]), 1,  NULL, 0, 0, "one", NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[2]), 10, NULL, 0, 0, "ten", NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[3]), 11, NULL, 0, 0, "eleven", NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 4, 0, 0, 0, 0x02, 0, 0, "bits", 0, 0, 1, 0, 0, 0);
}

static void
test_schema_yin(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lysc_node_leaf *lysc_leaf;
    struct lysp_node_leaf *lysp_leaf;
    struct lysc_type_bits *lysc_type;

    schema = MODULE_CREATE_YIN("T0",
            "<leaf name=\"port\"> <type name=\"bits\">"
            "  <bit name=\"zero\"/> <bit name=\"one\"/>"
            "  <bit name=\"ten\"> <position value=\"10\"/> </bit>"
            "  <bit name=\"eleven\"/>"
            "  <bit name=\"last\"> <position value=\"4294967295\"/> </bit>"
            "</type></leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    lysc_type = (struct lysc_type_bits *) lysc_leaf->type;
    CHECK_LYSC_TYPE_BITS(lysc_type, 0, 5);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[0]), 0,  NULL, 0, 0, "zero", NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[1]), 1,  NULL, 0, 0, "one", NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[2]), 10, NULL, 0, 0, "ten", NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[3]), 11, NULL, 0, 0, "eleven", NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[4]), 4294967295, NULL, 0, 0, "last", NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 5, 0, 0, 0, 0x02, 0, 0, "bits", 0, 0, 1, 0, 0, 0);

    schema = MODULE_CREATE_YIN("T1",
            "<leaf name=\"port\"> <type name=\"bits\">"
            "   <bit name=\"_ten\"> <position value=\"10\"/> </bit> <bit name=\"_ten-one\"/>"
            "   <bit name=\"_two\"> <position value=\"2\"/>  </bit> <bit name=\"ten_end...\"/>"
            "</type></leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    lysc_type = (struct lysc_type_bits *) lysc_leaf->type;
    CHECK_LYSC_TYPE_BITS(lysc_type, 0, 4);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[0]), 2,   NULL, 0, 0, "_two",     NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[1]), 10,  NULL, 0, 0, "_ten",     NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[2]), 11,  NULL, 0, 0, "_ten-one", NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[3]), 12,  NULL, 0, 0, "ten_end...",  NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 4, 0, 0, 0, 0x02, 0, 0, "bits", 0, 0, 1, 0, 0, 0);

    /* TEST MODULE SUBTYPE */
    schema = MODULE_CREATE_YIN("T2",
            "<typedef name=\"my_type\"> <type name=\"bits\">"
            "  <bit name=\"ten\"> <position value=\"10\"/> </bit>"
            "  <bit name=\"eleven\"/> <bit name=\"two\"> <position value=\"2\"/> </bit>"
            "  <bit name=\"twelve\"/>"
            "</type> </typedef>"
            "<leaf name=\"port\"> <type name=\"my_type\"/></leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    lysc_type = (struct lysc_type_bits *) lysc_leaf->type;
    CHECK_LYSC_TYPE_BITS(lysc_type, 0, 4);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[0]), 2,   NULL, 0, 0, "two",     NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[1]), 10,  NULL, 0, 0, "ten",     NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[2]), 11,  NULL, 0, 0, "eleven", NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[3]), 12,  NULL, 0, 0, "twelve",  NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 0, 0, 0, 0, 0, 0, 0, "my_type", 0, 0, 1, 0, 0, 0);

    schema = MODULE_CREATE_YIN("T3", "<typedef name=\"my_type\"> <type name=\"bits\">"
            "   <bit name=\"ten\"> <position value=\"10\"/></bit>"
            "   <bit name=\"eleven\"/> <bit name=\"two\"> <position value=\"2\"/> </bit>"
            "   <bit name=\"twelve\"/>"
            "</type></typedef>"
            "<leaf name=\"port\"> <type name=\"my_type\">"
            "   <bit name=\"ten\"> <position value=\"10\"/> </bit>"
            "   <bit name=\"two\"/>"
            "</type></leaf>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LEAF(lysc_leaf, NULL, 0, 0x5, 1, "port", 0, 0, 0, NULL, 0, 0, NULL, NULL);
    lysc_type = (struct lysc_type_bits *) lysc_leaf->type;
    CHECK_LYSC_TYPE_BITS(lysc_type, 0, 2);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[0]), 2,   NULL, 0, 0, "two",     NULL);
    CHECK_LYSC_TYPE_BITENUM_ITEM(&(lysc_type->bits[1]), 10,  NULL, 0, 0, "ten",     NULL);
    lysp_leaf = (void *)mod->parsed->data;
    CHECK_LYSP_NODE_LEAF(lysp_leaf, NULL, 0, 0x0, 0, "port", 0, 0, NULL, 0, 0, NULL, NULL);
    CHECK_LYSP_TYPE(&(lysp_leaf->type), 0, 2, 0, 0, 0, 0x02, 0, 0, "my_type", 0, 0, 1, 0, 0, 0);

    /*
     * TEST ERROR
     */
    /* test change bit possition */
    schema = MODULE_CREATE_YIN("TERR_0", "<typedef name=\"my_type\"> <type name=\"bits\">"
            "   <bit name=\"ten\"> <position value=\"10\"/> </bit>"
            "   <bit name=\"eleven\"/>"
            "   <bit name=\"two\"> <position value=\"2\"/> </bit>"
            "   <bit name=\"twelve\"/>"
            "</type></typedef>"
            "<leaf name=\"port\"> <type name=\"my_type\">"
            "   <bit name=\"ten\"> <position value=\"11\"/> </bit> <bit name=\"two\"/>"
            "</type></leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid bits - position of the item \"ten\" has changed from 10 to 11 in the derived type.",
            "/TERR_0:port", 0);

    /* add new bit */
    schema = MODULE_CREATE_YIN("TERR_1",
            "<typedef name=\"my_type\"> <type name=\"bits\">"
            "   <bit name=\"ten\">  <position value=\"10\"/> </bit>"
            "   <bit name=\"eleven\"/>"
            "   <bit name=\"two\"> <position value=\"2\"/> </bit>"
            "   <bit name=\"twelve\"/>"
            "</type></typedef>"
            "<leaf name=\"port\"> <type name=\"my_type\">"
            "   <bit name=\"ten\"> <position value=\"10\"/> </bit>"
            "   <bit name=\"two\"/>"
            "   <bit name=\"test\"/>"
            "</type></leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid bits - derived type adds new item \"test\".", "/TERR_1:port", 0);

    /* different max value => autoadd index */
    schema = MODULE_CREATE_YIN("TERR_2",
            "<leaf name=\"port\"> <type name=\"bits\">"
            "   <bit name=\"first\"> <position value=\"-1\"> </bit>"
            "   <bit name=\"second\">"
            "</type></leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_2\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid value \"-1\" of \"value\" attribute in \"position\" element.", NULL, 8);

    /* different max value => autoadd index */
    schema = MODULE_CREATE_YIN("TERR_3",
            "<leaf name=\"port\"> <type name=\"bits\">"
            "  <bit name=\"first\"> <position value=\"4294967295\"/> </bit>"
            "  <bit name=\"second\"/>"
            "</type></leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Invalid bits - it is not possible to auto-assign bit position for \"second\" since the highest value is already 4294967295.",
            "/TERR_3:port", 0);

    schema = MODULE_CREATE_YIN("TERR_4",
            "<leaf name=\"port\"> <type name=\"bits\">"
            "  <bit name=\"  ahoj  \"> <position value=\"20\"/> </bit>"
            "  <bit name=\"second\"/>"
            "</type></leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_4\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid identifier first character ' ' (0x0020).", NULL, 8);

    schema = MODULE_CREATE_YIN("TERR_5",
            "<leaf name=\"port\"> <type name=\"bits\">"
            "  <bit name=\"ah oj\"> <position value=\"20\"/> </bit>"
            "  <bit name=\"second\"/>"
            "</type></leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_5\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid identifier character ' ' (0x0020).", NULL, 8);

    schema = MODULE_CREATE_YIN("TERR_6",
            "<leaf name=\"port\"> <type name=\"bits\">"
            "  <bit name=\"hi\"/> "
            "  <bit name=\"hi\"/>"
            "</type></leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_6\" failed.", NULL, 0);
    CHECK_LOG_CTX("Duplicate identifier \"hi\" of bit statement.", NULL, 8);

    schema = MODULE_CREATE_YIN("TERR_7",
            "<leaf name=\"port\"> <type name=\"bits\">"
            "  <bit name=\"4ahoj\"> <position value=\"20\"/> </bit>"
            "  <bit name=\"second\"/>"
            "</type></leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_7\" failed.", NULL, 0);
    CHECK_LOG_CTX("Invalid identifier first character '4' (0x0034).", NULL, 8);

    /* TEST EMPTY NAME*/
    schema = MODULE_CREATE_YIN("TERR_8",
            "<leaf name=\"port\"> <type name=\"bits\">"
            "  <bit name=\"\"> <position value=\"20\"/> </bit>"
            "  <bit name=\"second\"/>"
            "</type></leaf>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_8\" failed.", NULL, 0);
    CHECK_LOG_CTX("Empty identifier is not allowed.", NULL, 8);
}

static void
test_schema_print(void **state)
{
    const char *schema_yang, *schema_yin;
    char *printed;
    struct lys_module *mod;

    /* test print yang to yin */
    schema_yang = MODULE_CREATE_YANG("PRINT0",
            "typedef my_type{type bits {"
            "   bit ten {position 10;} bit eleven; bit two {position 2;} bit twelve;}}"
            "leaf port {type my_type {"
            "   bit ten {position 10;} bit two;}}");

    schema_yin = MODULE_CREATE_YIN("PRINT0",
            "  <typedef name=\"my_type\">\n"
            "    <type name=\"bits\">\n"
            "      <bit name=\"ten\">\n"
            "        <position value=\"10\"/>\n"
            "      </bit>\n"
            "      <bit name=\"eleven\"/>\n"
            "      <bit name=\"two\">\n"
            "        <position value=\"2\"/>\n"
            "      </bit>\n"
            "      <bit name=\"twelve\"/>\n"
            "    </type>\n"
            "  </typedef>\n"
            "  <leaf name=\"port\">\n"
            "    <type name=\"my_type\">\n"
            "      <bit name=\"ten\">\n"
            "        <position value=\"10\"/>\n"
            "      </bit>\n"
            "      <bit name=\"two\"/>\n"
            "    </type>\n"
            "  </leaf>\n");

    UTEST_ADD_MODULE(schema_yang, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, schema_yin);
    free(printed);

    /* test print yin to yang */
    schema_yang = MODULE_CREATE_YANG("PRINT1",
            "\n"
            "  typedef my_type {\n"
            "    type bits {\n"
            "      bit ten {\n"
            "        position 10;\n"
            "      }\n"
            "      bit eleven;\n"
            "      bit two {\n"
            "        position 2;\n"
            "      }\n"
            "      bit twelve;\n"
            "    }\n"
            "  }\n\n"
            "  leaf port {\n"
            "    type my_type {\n"
            "      bit ten {\n"
            "        position 10;\n"
            "      }\n"
            "      bit two;\n"
            "    }\n"
            "  }\n");

    schema_yin = MODULE_CREATE_YIN("PRINT1",
            "  <typedef name=\"my_type\">\n"
            "    <type name=\"bits\">\n"
            "      <bit name=\"ten\">\n"
            "        <position value=\"10\"/>\n"
            "      </bit>\n"
            "      <bit name=\"eleven\"/>\n"
            "      <bit name=\"two\">\n"
            "        <position value=\"2\"/>\n"
            "      </bit>\n"
            "      <bit name=\"twelve\"/>\n"
            "    </type>\n"
            "  </typedef>\n"
            "  <leaf name=\"port\">\n"
            "    <type name=\"my_type\">\n"
            "      <bit name=\"ten\">\n"
            "        <position value=\"10\"/>\n"
            "      </bit>\n"
            "      <bit name=\"two\"/>\n"
            "    </type>\n"
            "  </leaf>\n");

    UTEST_ADD_MODULE(schema_yin, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, schema_yang);
    free(printed);
}

static void
test_data_xml(void **state)
{

    const char *schema;
    struct lyd_node *tree;
    const char *data;

    /* xml test */
    schema = MODULE_CREATE_YANG("T0", "typedef my_type{type bits {"
            "  bit ten {position 10;} bit eleven; bit two {position 2;} bit twelve;"
            "  bit _test-end...;}}"
            "leaf port {type my_type;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    TEST_SUCCESS_XML("T0", "ten two twelve", BITS, "two ten twelve", "two", "ten", "twelve");
    TEST_SUCCESS_XML("T0", "ten\ntwo\ttwelve", BITS, "two ten twelve", "two", "ten", "twelve");
    TEST_SUCCESS_XML("T0", "ten two", BITS, "two ten", "two", "ten");
    TEST_SUCCESS_XML("T0", "_test-end...", BITS, "_test-end...", "_test-end...");
    TEST_SUCCESS_XML("T0", "twelve\nten\ttwo  \n eleven", BITS, "two ten eleven twelve",
            "two", "ten", "eleven", "twelve");
    TEST_SUCCESS_XML("T0", "", BITS, "");
    TEST_SUCCESS_XML("T0", "\n\t", BITS, "");

    TEST_ERROR_XML("T0", "twelvea");
    CHECK_LOG_CTX("Invalid bit \"twelvea\".", "/T0:port", 1);
    TEST_ERROR_XML("T0", "twelve t");
    CHECK_LOG_CTX("Invalid bit \"t\".", "/T0:port", 1);
    TEST_ERROR_XML("T0", "ELEVEN");
    CHECK_LOG_CTX("Invalid bit \"ELEVEN\".", "/T0:port", 1);

    /* empty value  */
    data = "<port xmlns=\"urn:tests:T0\"/>"; \
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    CHECK_LYSC_NODE(tree->schema, NULL, 0, 0x5, 1, "port", 0, LYS_LEAF, 0, 0, 0, 0);
    CHECK_LYD_NODE_TERM((struct lyd_node_term *)tree, 0, 0, 0, 0, 1, BITS, "");
    lyd_free_all(tree);

}

static void
test_data_json(void **state)
{
    const char *schema;

    /* variable for default value test */

    /* xml test */
    schema = MODULE_CREATE_YANG("T0", "typedef my_type{type bits {"
            "  bit ten {position 10;} bit eleven; bit two {position 2;} bit twelve;"
            "  bit _test-end...;}}"
            "leaf port {type my_type;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    TEST_SUCCESS_JSON("T0", "ten two twelve", BITS, "two ten twelve", "two", "ten", "twelve");
    TEST_SUCCESS_JSON("T0", "ten\\ntwo\\ttwelve", BITS, "two ten twelve", "two", "ten", "twelve");
    TEST_SUCCESS_JSON("T0", "ten two", BITS, "two ten", "two", "ten");
    TEST_SUCCESS_JSON("T0", "_test-end...", BITS, "_test-end...", "_test-end...");
    TEST_SUCCESS_JSON("T0", "twelve\\nten\\ttwo  \\n eleven", BITS, "two ten eleven twelve",
            "two", "ten", "eleven", "twelve");
    TEST_SUCCESS_JSON("T0", "", BITS, "");
    TEST_SUCCESS_JSON("T0", "\\n\\t", BITS, "");

    TEST_ERROR_JSON("T0", "twelvea");
    CHECK_LOG_CTX("Invalid character sequence \"twelvea}\", expected a JSON value.", NULL, 1);
    TEST_ERROR_JSON("T0", "twelve t");
    CHECK_LOG_CTX("Invalid character sequence \"twelve t}\", expected a JSON value.", NULL, 1);
    TEST_ERROR_JSON("T0", "ELEVEN");
    CHECK_LOG_CTX("Invalid character sequence \"ELEVEN}\", expected a JSON value.", NULL, 1);
}

static void
test_data_lyb(void **state)
{
    const char *schema;

    schema = MODULE_CREATE_YANG("lyb", "typedef my_type{type bits {"
            "  bit ten {position 10;} bit eleven; bit two {position 2;} bit twelve;"
            "  bit _test-end...;}}"
            "leaf port {type my_type;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    TEST_SUCCESS_LYB("lyb", "port", "ten twelve");
    TEST_SUCCESS_LYB("lyb", "port", "two");
    TEST_SUCCESS_LYB("lyb", "port", "");
}

static void
test_diff(void **state)
{
    const char *schema;
    struct lyd_node *model_1, *model_2;
    struct lyd_node *diff;
    const char *expected_string;
    const char *data_1;
    const char *data_2;
    const char *diff_expected;

    schema = MODULE_CREATE_YANG("T0", "leaf port {type bits { bit zero; bit one;"
            " bit two; bit three;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    data_1 = "<port xmlns=\"urn:tests:T0\"> two three </port>";
    data_2 = "<port xmlns=\"urn:tests:T0\"> one</port>";
    diff_expected = "<port xmlns=\"urn:tests:T0\""
            " xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"replace\""
            " yang:orig-default=\"false\" yang:orig-value=\"two three\">one</port>";
    CHECK_PARSE_LYD_PARAM(data_1, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_1)
    CHECK_PARSE_LYD_PARAM(data_2, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_2)
    assert_int_equal(LY_SUCCESS, lyd_diff_siblings(model_1, model_2, 0, &diff));
    assert_non_null(diff);
    CHECK_LYD_STRING_PARAM(diff, diff_expected, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_int_equal(LY_SUCCESS, lyd_diff_apply_all(&model_1, diff));
    CHECK_LYD(model_1, model_2);
    lyd_free_all(model_1);
    lyd_free_all(model_2);
    lyd_free_all(diff);

    /* create data from diff */
    diff_expected = "<port xmlns=\"urn:tests:T0\""
            " xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"replace\""
            " yang:orig-default=\"false\" yang:orig-value=\"two three \"></port>";
    CHECK_PARSE_LYD_PARAM(diff_expected, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, diff)
    data_1 = "<port xmlns=\"urn:tests:T0\"> two three </port>";
    CHECK_PARSE_LYD_PARAM(data_1, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_1)
    assert_int_equal(LY_SUCCESS, lyd_diff_apply_all(&model_1, diff));
    expected_string = "<port xmlns=\"urn:tests:T0\"/>";

    CHECK_LYD_STRING_PARAM(model_1, expected_string, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    lyd_free_all(model_1);
    lyd_free_all(diff);

    /* create data from diff */
    diff_expected = "<port xmlns=\"urn:tests:T0\""
            " xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\" yang:operation=\"replace\""
            " yang:orig-default=\"false\" yang:orig-value=\"two three\"> one </port>";
    CHECK_PARSE_LYD_PARAM(diff_expected, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, diff)
    data_1 = "<port xmlns=\"urn:tests:T0\"> two three </port>";
    CHECK_PARSE_LYD_PARAM(data_1, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_1)
    assert_int_equal(LY_SUCCESS, lyd_diff_apply_all(&model_1, diff));
    expected_string = "<port xmlns=\"urn:tests:T0\">one</port>";

    CHECK_LYD_STRING_PARAM(model_1, expected_string, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    lyd_free_all(model_1);
    lyd_free_all(diff);

}

static void
test_print(void **state)
{
    const char *schema;
    const char *expected_string;
    struct lyd_node *model_1;
    const char *data;

    schema = MODULE_CREATE_YANG("T0", "leaf port {type bits { bit zero; bit one;"
            " bit two; bit three;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    /* print zero bits */
    data = "<port xmlns=\"urn:tests:T0\"></port>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_1)
    /* XML */
    expected_string = "<port xmlns=\"urn:tests:T0\"/>";
    CHECK_LYD_STRING_PARAM(model_1, expected_string, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    /* JSON */
    expected_string = "{\"T0:port\":\"\"}";
    CHECK_LYD_STRING_PARAM(model_1, expected_string, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    /* free */
    lyd_free_all(model_1);

    /* print one bit */
    data = "<port xmlns=\"urn:tests:T0\"> two </port>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_1)
    /* XML */
    expected_string = "<port xmlns=\"urn:tests:T0\">two</port>";
    CHECK_LYD_STRING_PARAM(model_1, expected_string, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    /* JSON */
    expected_string = "{\"T0:port\":\"two\"}";
    CHECK_LYD_STRING_PARAM(model_1, expected_string, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    /* free */
    lyd_free_all(model_1);

    /* print two bits */
    data = "<port xmlns=\"urn:tests:T0\">three two </port>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_1)
    /* XML */
    expected_string = "<port xmlns=\"urn:tests:T0\">two three</port>";
    CHECK_LYD_STRING_PARAM(model_1, expected_string, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    /* JSON */
    expected_string = "{\"T0:port\":\"two three\"}";
    CHECK_LYD_STRING_PARAM(model_1, expected_string, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    /* free */
    lyd_free_all(model_1);
}

static void
test_plugin_store(void **state)
{
    const char *val_text = NULL;
    struct ly_err_item *err = NULL;
    struct lys_module *mod;
    struct lyd_value value = {0};
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_BITS]);
    struct lysc_type *lysc_type;
    struct lysc_type lysc_type_test;
    LY_ERR ly_ret;
    char *alloc;
    const char *schema;

    /* create schema. Prepare common used variables */
    schema = MODULE_CREATE_YANG("T0", "leaf port {type bits { bit zero; bit one;"
            " bit two; bit three;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *)mod->compiled->data)->type;

    /* check proper type */
    assert_string_equal("libyang 2 - bits, version 1", type->id);

    /* check store
     */
    val_text = "";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, BITS, "");
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    val_text = "zero one two";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, BITS, "zero one two", "zero", "one", "two");
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    val_text = "zero two";
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err));
    CHECK_LYD_VALUE(value, BITS, "zero two", "zero", "two");
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    val_text = "\n ";
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err);
    assert_int_equal(LY_SUCCESS, ly_ret);
    CHECK_LYD_VALUE(value, BITS, "");
    assert_ptr_equal(value.realtype, lysc_type);
    type->free(UTEST_LYCTX, &value);

    /*
     * minor tests
     * dynamic alocated input text
     */
    val_text = "two";
    alloc = (char *)malloc(strlen(val_text) + 1);
    memcpy(alloc, val_text, strlen(val_text) + 1);
    ly_ret = type->store(UTEST_LYCTX, lysc_type, alloc, strlen(val_text),
            LYPLG_TYPE_STORE_DYNAMIC, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err);
    alloc = NULL;
    assert_int_equal(LY_SUCCESS, ly_ret);
    CHECK_LYD_VALUE(value, BITS, "two", "two");
    type->free(UTEST_LYCTX, &value);

    /* wrong lysc_type of value */
    lysc_type_test = *lysc_type;
    lysc_type_test.basetype = LY_TYPE_INT8;
    val_text = "two";
    ly_ret = type->store(UTEST_LYCTX, &lysc_type_test, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err);
    assert_int_equal(LY_EINVAL, ly_ret);
    ly_err_free(err);

    /*
     * ERROR TESTS
     */
    val_text = "two";
    err = NULL;
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_HEXNUM, NULL, &value, NULL, &err);
    assert_int_equal(LY_EVALID, ly_ret);
    ly_err_free(err);

    val_text = "two two";
    err = NULL;
    ly_ret = type->store(UTEST_LYCTX, lysc_type, val_text, strlen(val_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &value, NULL, &err);
    assert_int_equal(LY_EVALID, ly_ret);
    ly_err_free(err);
}

static void
test_plugin_compare(void **state)
{
    struct ly_err_item *err = NULL;
    struct lys_module *mod;
    struct lyd_value values[10];
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_BITS]);
    struct lysc_type *lysc_type;
    LY_ERR ly_ret;
    const char *schema;
    /* different type */
    const char *diff_type_text = "20";
    struct lyd_value  diff_type_val;
    struct lysc_type *diff_type;
    /* Value which are going to be created to tested */
    const char *val_init[] = {"", "two zero", "three", "zero two", "zero", "three"};

    /* create schema. Prepare common used variables */
    schema = MODULE_CREATE_YANG("T0", "typedef my_int_type { type bits { bit zero; bit one;"
            " bit two; bit three;}}"
            "leaf p1 {type my_int_type;}"
            "leaf p2 {type my_int_type;}"
            "leaf p3 {type my_int_type{bit three; bit zero;}}"
            "leaf p4 {type string;}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *)mod->compiled->data)->type;

    /* CREATE VALUES */
    for (unsigned int it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
        ly_ret = type->store(UTEST_LYCTX, lysc_type, val_init[it], strlen(val_init[it]),
                0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &(values[it]), NULL, &err);
        assert_int_equal(LY_SUCCESS, ly_ret);
    }

    /*
     * BASIC TEST;
     */
    assert_int_equal(LY_SUCCESS, type->compare(UTEST_LYCTX, &(values[0]), &(values[0])));
    assert_int_equal(LY_SUCCESS, type->compare(UTEST_LYCTX, &(values[1]), &(values[3])));
    assert_int_equal(LY_ENOT, type->compare(UTEST_LYCTX, &(values[0]), &(values[1])));
    assert_int_equal(LY_ENOT, type->compare(UTEST_LYCTX, &(values[3]), &(values[4])));
    assert_int_equal(LY_ENOT, type->compare(UTEST_LYCTX, &(values[1]), &(values[0])));
    assert_int_equal(LY_ENOT, type->compare(UTEST_LYCTX, &(values[1]), &(values[2])));
    assert_int_equal(LY_SUCCESS, type->compare(UTEST_LYCTX, &(values[2]), &(values[5])));

    /*
     * SAME TYPE but different node
     */
    diff_type_text = val_init[2];
    diff_type = ((struct lysc_node_leaf *)mod->compiled->data->next)->type;
    ly_ret = diff_type->plugin->store(UTEST_LYCTX, diff_type, diff_type_text, strlen(diff_type_text),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &diff_type_val, NULL, &err);
    assert_int_equal(LY_SUCCESS, ly_ret);
    assert_int_equal(LY_SUCCESS, type->compare(UTEST_LYCTX, &diff_type_val, &(values[2])));
    assert_int_equal(LY_ENOT,    type->compare(UTEST_LYCTX, &diff_type_val, &(values[1])));
    type->free(UTEST_LYCTX, &(diff_type_val));

    /* delete values */
    for (unsigned int it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
        type->free(UTEST_LYCTX, &(values[it]));
    }
}

static void
test_plugin_sort(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_BITS]);
    struct lysc_type *lysc_type;
    struct ly_err_item *err = NULL;
    struct lyd_value val1 = {0}, val2 = {0};

    schema = MODULE_CREATE_YANG("T0", "leaf-list ll { type bits { bit zero; bit one; bit two; bit three;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *)mod->compiled->data)->type;

    /* 1000 < 1001 */
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, "three", strlen("three"),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, "three one", strlen("three one"),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_true(0 > type->sort(UTEST_LYCTX, &val1, &val2));
    assert_true(0 < type->sort(UTEST_LYCTX, &val2, &val1));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val1, &val1));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);

    /* 0011 == 0011 */
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, "zero one", strlen("zero one"),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val1, NULL, &err));
    assert_int_equal(LY_SUCCESS, type->store(UTEST_LYCTX, lysc_type, "one zero", strlen("one zero"),
            0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &val2, NULL, &err));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val1, &val2));
    assert_int_equal(0, type->sort(UTEST_LYCTX, &val2, &val1));
    type->free(UTEST_LYCTX, &val1);
    type->free(UTEST_LYCTX, &val2);
}

static void
test_plugin_print(void **state)
{
    struct ly_err_item *err = NULL;
    struct lys_module *mod;
    struct lyd_value values[10];
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_BITS]);
    struct lysc_type *lysc_type;
    LY_ERR ly_ret;
    const char *schema;
    /* Value which are going to be created to tested */
    const char *val_init[] = {"", "two zero", "three", "zero two", "zero", "three"};

    /* create schema. Prepare common used variables */
    schema = MODULE_CREATE_YANG("T0",
            "leaf p1 { type bits { bit zero; bit one;"
            " bit two; bit three;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *)mod->compiled->data)->type;

    /* CREATE VALUES */
    for (unsigned int it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
        ly_ret = type->store(UTEST_LYCTX, lysc_type, val_init[it], strlen(val_init[it]),
                0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &(values[it]), NULL, &err);
        assert_int_equal(LY_SUCCESS, ly_ret);
    }

    /* print value */
    ly_bool dynamic = 0;

    assert_string_equal("", type->print(UTEST_LYCTX, &(values[0]), LY_VALUE_XML, NULL, &dynamic, NULL));
    assert_string_equal("zero two", type->print(UTEST_LYCTX, &(values[1]), LY_VALUE_XML, NULL, &dynamic, NULL));
    assert_string_equal("three", type->print(UTEST_LYCTX, &(values[2]), LY_VALUE_XML, NULL, &dynamic, NULL));
    assert_string_equal("zero two", type->print(UTEST_LYCTX, &(values[3]), LY_VALUE_XML, NULL, &dynamic, NULL));
    assert_string_equal("zero", type->print(UTEST_LYCTX, &(values[4]), LY_VALUE_XML, NULL, &dynamic, NULL));
    assert_string_equal("three", type->print(UTEST_LYCTX, &(values[5]), LY_VALUE_XML, NULL, &dynamic, NULL));

    for (unsigned int it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
        type->free(UTEST_LYCTX, &(values[it]));
    }
}

static void
test_plugin_dup(void **state)
{
    struct ly_err_item *err = NULL;
    struct lys_module *mod;
    struct lyd_value values[10];
    struct lyplg_type *type = lyplg_type_plugin_find(NULL, "", NULL, ly_data_type2str[LY_TYPE_BITS]);
    struct lysc_type *lysc_type;
    const char *schema;
    LY_ERR ly_ret;
    /* Value which are going to be tested */
    const char *val_init[] = {"", "two zero", "three", "zero two", "zero", "three one two zero"};

    /* create schema. Prepare common used variables */
    schema = MODULE_CREATE_YANG("T0",
            "leaf p1 { type bits { bit zero; bit one;"
            " bit two; bit three;}}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_type = ((struct lysc_node_leaf *)mod->compiled->data)->type;

    /* CREATE VALUES */
    for (unsigned int it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
        ly_ret = type->store(UTEST_LYCTX, lysc_type, val_init[it], strlen(val_init[it]),
                0, LY_VALUE_XML, NULL, LYD_VALHINT_STRING, NULL, &(values[it]), NULL, &err);
        assert_int_equal(LY_SUCCESS, ly_ret);
    }

    /* print duplicate value */
    struct lyd_value dup_value;

    assert_int_equal(LY_SUCCESS, type->duplicate(UTEST_LYCTX, &(values[0]), &dup_value));
    CHECK_LYD_VALUE(dup_value, BITS, "");
    assert_ptr_equal(dup_value.realtype, values[0].realtype);
    type->free(UTEST_LYCTX, &dup_value);

    assert_int_equal(LY_SUCCESS, type->duplicate(UTEST_LYCTX, &(values[1]), &dup_value));
    CHECK_LYD_VALUE(dup_value, BITS, "zero two", "zero", "two");
    assert_ptr_equal(dup_value.realtype, values[1].realtype);
    type->free(UTEST_LYCTX, &dup_value);

    assert_int_equal(LY_SUCCESS, type->duplicate(UTEST_LYCTX, &(values[2]), &dup_value));
    CHECK_LYD_VALUE(dup_value, BITS, "three", "three");
    assert_ptr_equal(dup_value.realtype, values[2].realtype);
    type->free(UTEST_LYCTX, &dup_value);

    assert_int_equal(LY_SUCCESS, type->duplicate(UTEST_LYCTX, &(values[3]), &dup_value));
    CHECK_LYD_VALUE(dup_value, BITS, "zero two", "zero", "two");
    assert_ptr_equal(dup_value.realtype, values[3].realtype);
    type->free(UTEST_LYCTX, &dup_value);

    assert_int_equal(LY_SUCCESS, type->duplicate(UTEST_LYCTX, &(values[4]), &dup_value));
    CHECK_LYD_VALUE(dup_value, BITS, "zero", "zero");
    assert_ptr_equal(dup_value.realtype, values[4].realtype);
    type->free(UTEST_LYCTX, &dup_value);

    assert_int_equal(LY_SUCCESS, type->duplicate(UTEST_LYCTX, &(values[5]), &dup_value));
    CHECK_LYD_VALUE(dup_value, BITS, "zero one two three", "zero", "one", "two", "three");
    assert_ptr_equal(dup_value.realtype, values[5].realtype);
    type->free(UTEST_LYCTX, &dup_value);

    /* error tests */
    assert_int_equal(LY_EINVAL, type->duplicate(NULL, &(values[0]), &dup_value));

    for (unsigned int it = 0; it < sizeof(val_init) / sizeof(val_init[0]); it++) {
        type->free(UTEST_LYCTX, &(values[it]));
    }
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_schema_yang),
        UTEST(test_schema_yin),
        UTEST(test_schema_print),
        UTEST(test_data_xml),
        UTEST(test_data_json),
        UTEST(test_data_lyb),
        UTEST(test_diff),
        UTEST(test_print),

        UTEST(test_plugin_store),
        UTEST(test_plugin_compare),
        UTEST(test_plugin_sort),
        UTEST(test_plugin_print),
        UTEST(test_plugin_dup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
