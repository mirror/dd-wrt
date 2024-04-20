/**
 * @file list.c
 * @author Radek Iša <isa@cesnet.cz>
 * @brief test for list node
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

#define LYD_TREE_CREATE(INPUT, MODEL) \
    CHECK_PARSE_LYD_PARAM(INPUT, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, MODEL)

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

static void
test_schema_yang(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lysc_node_list *lysc_leaf;
    struct lysc_node *lysc_node;

    schema = MODULE_CREATE_YANG("T0", "list user {"
            "key uid;"
            "unique name;"
            "leaf uid{type int32;}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "user", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 1, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "uid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_UNIQUE, 1, "name", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "group", 0, LYS_LEAF, 1, 0, 0, 0);

    schema = MODULE_CREATE_YANG("T1", "list user {"
            "key uid;"
            "container name{"
            "   leaf fist  {type string;}"
            "   container second{leaf sub { type int32;}}"
            "}"
            "leaf uid{type int32;}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "user", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 0, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "uid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "name", 0, LYS_CONTAINER, 1, 0, 0, 0);

    schema = MODULE_CREATE_YANG("T2", "list grup {"
            "key \"guid\";"
            "leaf guid{type int32;}"
            "list users{ key name; leaf name {type string;}}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "grup", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 0, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "guid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "users", 0, LYS_LIST, 1, 0, 0, 0);

    /* restriction */
    schema = MODULE_CREATE_YANG("T3", "list grup {"
            "key guid;"
            "min-elements 10;"
            "max-elements 20;"
            "leaf guid{type int32;}"
            "list users{ key name; leaf name {type string;}}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_MAND_TRUE | LYS_ORDBY_SYSTEM, 1, \
            "grup", 0, 0, 0, 0, 0, 1, 20, 10, 0, 0, 0, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "guid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "users", 0, LYS_LIST, 1, 0, 0, 0);

    schema = MODULE_CREATE_YANG("T4", "list user {"
            "key \"uid name\";"
            "unique name;"
            "leaf uid{type int32;}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "user", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 1, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "uid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_UNIQUE | LYS_KEY, 1, "name", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "group", 0, LYS_LEAF, 1, 0, 0, 0);

    schema = MODULE_CREATE_YANG("T5", "list rule {"
            "key \"id\";"
            "unique \"name\";"
            "unique \"ip port\";"
            "leaf id{type int32;}"
            "leaf name{type string;}"
            "leaf ip{type string;}"
            "leaf port{type int16;}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "rule", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 2, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "id", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_UNIQUE, 1, "name", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_UNIQUE, 1, "ip", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_UNIQUE, 1, "port", 0, LYS_LEAF, 1, 0, 0, 0);

    /* test error */
    schema = MODULE_CREATE_YANG("TERR_0", "list user {"
            "key uid;"
            "min-elements 10;"
            "max-elements -1;"
            "leaf uid{type int32;}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_0\" failed.", NULL);
    CHECK_LOG_CTX("Invalid value \"-1\" of \"max-elements\".", "Line number 5.");

    schema = MODULE_CREATE_YANG("TERR_0", "list user {"
            "key uid;"
            "min-elements 10;"
            "max-elements 4294967298;"
            "leaf uid{type int32;}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_0\" failed.", NULL);
    CHECK_LOG_CTX("Value \"4294967298\" is out of \"max-elements\" bounds.", "Line number 5.");

    schema = MODULE_CREATE_YANG("TERR_0", "list user {"
            "key uid;"
            "min-elements 20;"
            "max-elements 10;"
            "leaf uid{type int32;}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("List min-elements 20 is bigger than max-elements 10.", "Path \"/TERR_0:user\".");

    schema = MODULE_CREATE_YANG("TERR_0", "list user {"
            "key uid;"
            "min-elements -1;"
            "max-elements 20;"
            "leaf uid{type int32;}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_0\" failed.", NULL);
    CHECK_LOG_CTX("Invalid value \"-1\" of \"min-elements\".", "Line number 5.");

    schema = MODULE_CREATE_YANG("TERR_0", "list user {"
            "key uid;"
            "key name;"
            "leaf uid{type int32;}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_0\" failed.", NULL);
    CHECK_LOG_CTX("Duplicate keyword \"key\".", "Line number 5.");

    schema = MODULE_CREATE_YANG("T6", "list user {"
            "config false;"
            "leaf uid{type int32;}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_R | LYS_STATUS_CURR | LYS_ORDBY_USER | LYS_KEYLESS | LYS_SET_CONFIG, \
            1, "user", 0, 0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 0, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_R | LYS_STATUS_CURR, 1, "uid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_R | LYS_STATUS_CURR, 1, "name", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_R | LYS_STATUS_CURR, 1, "group", 0, LYS_LEAF, 1, 0, 0, 0);

    schema = MODULE_CREATE_YANG("T7", "list user {"
            "key uid;"
            "unique name;"
            "ordered-by user;"
            "leaf uid{type int32;}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_USER, 1, "user", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 1, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "uid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_UNIQUE, 1, "name", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "group", 0, LYS_LEAF, 1, 0, 0, 0);

    schema = MODULE_CREATE_YANG("T8", "list user {"
            "key uid;"
            "unique name;"
            "ordered-by system;"
            "leaf uid{type int32;}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "user", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 1, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "uid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_UNIQUE, 1, "name", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "group", 0, LYS_LEAF, 1, 0, 0, 0);

    schema = MODULE_CREATE_YANG("TERROR0", "list user {"
            "key uid;"
            "unique name;"
            "ordered-by systeme;"
            "leaf uid{type int32;}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERROR0\" failed.", NULL);
    CHECK_LOG_CTX("Invalid value \"systeme\" of \"ordered-by\".", "Line number 5.");

    schema = MODULE_CREATE_YANG("TERROR0", "list \"\" {"
            "key uid;"
            "unique name;"
            "ordered-by system;"
            "leaf uid{type int32;}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_INVALID_MODULE(schema, LYS_IN_YANG, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERROR0\" failed.", NULL);
    CHECK_LOG_CTX("Statement argument is required.", "Line number 5.");

    schema = MODULE_CREATE_YANG("T9", "list user {"
            "key uid;"
            "unique name;"
            "ordered-by system;"
            "leaf uid{type int32;}"
            "leaf name{type string;}"
            "leaf group{type string; default \"abcd\";}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "user", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 1, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "uid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_UNIQUE, 1, "name", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_SET_DFLT, 1, "group", 0, LYS_LEAF, 1, 0, 0, 0);

    schema = MODULE_CREATE_YANG("T10", "list user {"
            "key uid;"
            "leaf uid{type int32; default \"25\";}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "user", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 0, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY | LYS_SET_DFLT, 1, "uid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "name", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "group", 0, LYS_LEAF, 1, 0, 0, 0);

    schema = MODULE_CREATE_YANG("T11",
            "typedef my_type {"
            "    type int8; default \"25\";"
            "}"
            "list user {"
            "   key uid;"
            "   leaf uid{type my_type;}"
            "   leaf name{type string;}"
            "   leaf group{type string;}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "user", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 0, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "uid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "name", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "group", 0, LYS_LEAF, 1, 0, 0, 0);

}

static void
test_schema_yin(void **state)
{
    const char *schema;
    struct lys_module *mod;
    struct lysc_node_list *lysc_leaf;
    struct lysc_node *lysc_node;

    schema = MODULE_CREATE_YIN("T0", "<list name=\"user\">"
            "   <key value=\"uid\"/>"
            "   <unique tag=\"name\"/>"
            "   <leaf name=\"uid\"><type name=\"int32\"/></leaf>"
            "   <leaf name=\"name\"><type name=\"string\"/></leaf>"
            "   <leaf name=\"group\"><type name=\"string\"/></leaf>"
            "</list>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "user", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 1, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "uid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_UNIQUE, 1, "name", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "group", 0, LYS_LEAF, 1, 0, 0, 0);

    schema = MODULE_CREATE_YIN("T00", "<list name=\"user\">"
            "   <key value=\"u&lt;id\"/>"
            "   <leaf name=\"uid\"><type name=\"int32\"/></leaf>"
            "   <leaf name=\"name\"><type name=\"string\"/></leaf>"
            "   <leaf name=\"group\"><type name=\"string\"/></leaf>"
            "</list>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("The list's key \"u<id\" not found.", "Path \"/T00:user\".");

    schema = MODULE_CREATE_YIN("T1", "<list name=\"user\"> "
            "   <key value=\"uid\"/>"
            "   <container name=\"name\">"
            "      <leaf name=\"fist\">  <type name=\"string\"/> </leaf>"
            "      <container name=\"second\">"
            "           <leaf name=\"sub\"> <type name=\"int32\"/></leaf>"
            "      </container>"
            "   </container>"
            "   <leaf name=\"uid\"> <type name=\"int32\"/></leaf>"
            "</list>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "user", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 0, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "uid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "name", 0, LYS_CONTAINER, 1, 0, 0, 0);

    schema = MODULE_CREATE_YIN("T2", "<list name=\"grup\">"
            "<key value=\"guid\"/>"
            "<leaf name=\"guid\"> <type name=\"int32\"/> </leaf>"
            "<list name=\"users\">"
            "   <key value=\"name\"/>"
            "   <leaf name=\"name\"> <type name=\"string\"/> </leaf>"
            "</list>"
            "</list>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "grup", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 0, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "guid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "users", 0, LYS_LIST, 1, 0, 0, 0);

    /* restriction */
    schema = MODULE_CREATE_YIN("T3",
            "<list name = \"grup\">"
            "   <key value=\"guid\"/>"
            "   <min-elements value=\"10\"/>"
            "   <max-elements value=\"20\"/>"
            "   <leaf name=\"guid\"> <type name=\"int32\"/> </leaf>"
            "   <list name=\"users\"> <key value=\"name\"/>"
            "       <leaf name=\"name\"> <type name=\"string\"/> </leaf>"
            "   </list>"
            "</list>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_MAND_TRUE | LYS_ORDBY_SYSTEM, 1, "grup", \
            0, 0, 0, 0, 0, 1, 20, 10, 0, 0, 0, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "guid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM,  1, "users", 0, LYS_LIST, 1, 0, 0, 0);

    schema = MODULE_CREATE_YIN("T4",
            "<list name=\"user\">"
            "   <key value=\"uid name\"/>"
            "   <unique tag=\"name\"/>"
            "   <leaf name=\"uid\"> <type name=\"int32\"/> </leaf>"
            "   <leaf name=\"name\"> <type name=\"string\"/> </leaf>"
            "   <leaf name=\"group\"> <type name=\"string\"/> </leaf>"
            "</list>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "user", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 1, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "uid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_UNIQUE | LYS_KEY, 1, "name", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "group", 0, LYS_LEAF, 1, 0, 0, 0);

    schema = MODULE_CREATE_YIN("T5",
            "<list name=\"rule\">"
            "   <key value=\"id\"/>"
            "   <unique tag=\"name\"/>"
            "   <unique tag=\"ip port\"/>"
            "   <leaf name=\"id\"> <type name=\"int32\"/></leaf>"
            "   <leaf name=\"name\"> <type name=\"string\"/> </leaf>"
            "   <leaf name=\"ip\"> <type name=\"string\"/> </leaf>"
            "   <leaf name=\"port\"> <type name=\"int16\"/> </leaf>"
            "</list>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "rule", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 2, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "id", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_UNIQUE, 1, "name", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_UNIQUE, 1, "ip", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_UNIQUE, 1, "port", 0, LYS_LEAF, 1, 0, 0, 0);

    /* test error */
    schema = MODULE_CREATE_YIN("TERR_0",
            "<list name=\"user\">"
            "   <key value=\"uid\"/>"
            "   <min-elements value=\"10\"/>"
            "   <max-elements value=\"-1\"/>"
            "   <leaf name=\"uid\"> <type name=\"int32\"> </leaf>"
            "</list>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_0\" failed.", NULL);
    CHECK_LOG_CTX("Invalid value \"-1\" of \"value\" attribute in \"max-elements\" element.", "Line number 8.");

    schema = MODULE_CREATE_YIN("TERR_0",
            "<list name=\"user\">"
            "   <key value=\"uid\"/>"
            "   <min-elements value=\"10\"/>"
            "   <max-elements value=\"4294967298\"/>"
            "   <leaf name=\"uid\">   <type name=\"int32\"/>   </leaf>"
            "   <leaf name=\"name\">  <type name=\"string\"/>  </leaf>"
            "   <leaf name=\"group\"> <type name=\"string\"/>  </leaf>"
            "</list>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_0\" failed.", NULL);
    CHECK_LOG_CTX("Value \"4294967298\" of \"value\" attribute in \"max-elements\" element is out of bounds.", "Line number 8.");

    schema = MODULE_CREATE_YIN("TERR_0",
            "<list name=\"user\">"
            "   <key value=\"uid\"/>"
            "   <min-elements value=\"20\"/>"
            "   <max-elements value=\"10\"/>"
            "   <leaf name=\"uid\">   <type name=\"int32\"/>  </leaf>"
            "   <leaf name=\"name\">  <type name=\"string\"/> </leaf>"
            "   <leaf name=\"group\"> <type name=\"string\"/> </leaf>"
            "</list>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_0\" failed.", NULL);
    CHECK_LOG_CTX("Invalid combination of min-elements and max-elements: min value 20 is bigger than the max value 10.",
            "Line number 8.");

    schema = MODULE_CREATE_YIN("TERR_0",
            "<list name=\"user\">"
            "   <key value=\"uid\"/>"
            "   <min-elements value=\"-1\"/>"
            "   <max-elements value=\"20\"/>"
            "   <leaf name=\"uid\">   <type name=\"int32\"/>  </leaf>"
            "   <leaf name=\"name\">  <type name=\"string\"/> </leaf>"
            "   <leaf name=\"group\"> <type name=\"string\"/> </leaf>"
            "</list>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_0\" failed.", NULL);
    CHECK_LOG_CTX("Value \"-1\" of \"value\" attribute in \"min-elements\" element is out of bounds.", "Line number 8.");

    schema = MODULE_CREATE_YIN("TERR_0",
            "<list name=\"user\">"
            "   <key  value=\"uid\"/>"
            "   <key  value=\"name\"/>"
            "   <leaf name=\"uid\">   <type name=\"int32\"/>  </leaf>"
            "   <leaf name=\"name\">  <type name=\"string\"/> </leaf>"
            "   <leaf name=\"group\"> <type name=\"string\"/> </leaf>"
            "</list>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERR_0\" failed.", NULL);
    CHECK_LOG_CTX("Redefinition of \"key\" sub-element in \"list\" element.", "Line number 8.");

    schema = MODULE_CREATE_YIN("T6",
            "<list name=\"user\">"
            "   <config value=\"false\"/>"
            "   <leaf name=\"uid\">  <type name=\"int32\"/>  </leaf>"
            "   <leaf name=\"name\"> <type name=\"string\"/> </leaf>"
            "   <leaf name=\"group\"><type name=\"string\"/> </leaf>"
            "</list>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_R | LYS_STATUS_CURR | LYS_ORDBY_USER | LYS_KEYLESS | LYS_SET_CONFIG, \
            1, "user", 0, 0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 0, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_R | LYS_STATUS_CURR, 1, "uid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_R | LYS_STATUS_CURR,  1, "name", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_R | LYS_STATUS_CURR, 1, "group", 0, LYS_LEAF, 1, 0, 0, 0);

    schema = MODULE_CREATE_YIN("T7",
            "<list name=\"user\">"
            "   <key value=\"uid\"/>"
            "   <unique tag=\"name\"/>"
            "   <ordered-by value=\"user\"/>"
            "   <leaf name=\"uid\">  <type name=\"int32\"/>  </leaf>"
            "   <leaf name=\"name\"> <type name=\"string\"/> </leaf>"
            "   <leaf name=\"group\"><type name=\"string\"/> </leaf>"
            "</list>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_USER, 1, "user", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 1, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "uid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_UNIQUE, 1, "name", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "group", 0, LYS_LEAF, 1, 0, 0, 0);

    schema = MODULE_CREATE_YIN("T8",
            "<list name=\"user\">"
            "   <key value=\"uid\"/>"
            "   <unique tag=\"name\"/>"
            "   <ordered-by value=\"system\"/>"
            "   <leaf name=\"uid\">  <type name=\"int32\"/>  </leaf>"
            "   <leaf name=\"name\"> <type name=\"string\"/> </leaf>"
            "   <leaf name=\"group\"><type name=\"string\"/> </leaf>"
            "</list>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "user", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 1, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "uid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_UNIQUE, 1, "name", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR, 1, "group", 0, LYS_LEAF, 1, 0, 0, 0);

    schema = MODULE_CREATE_YIN("TERROR0",
            "<list name=\"user\">"
            "   <key value=\"uid\"/>"
            "   <unique tag=\"name\"/>"
            "   <ordered-by value=\"systeme\"/>"
            "   <leaf name=\"uid\">  <type name=\"int32\"/>  </leaf>"
            "   <leaf name=\"name\"> <type name=\"string\"/> </leaf>"
            "   <leaf name=\"group\"><type name=\"string\"/> </leaf>"
            "</list>");
    UTEST_INVALID_MODULE(schema, LYS_IN_YIN, NULL, LY_EVALID);
    CHECK_LOG_CTX("Parsing module \"TERROR0\" failed.", NULL);
    CHECK_LOG_CTX("Invalid value \"systeme\" of \"value\" attribute in \"ordered-by\" element. Valid values are \"system\" and \"user\".",
            "Line number 8.");

    schema = MODULE_CREATE_YIN("T_DEFS1",
            "<list name=\"user\">"
            "   <key value=\"uid\"/>"
            "   <unique tag=\"name\"/>"
            "   <ordered-by value=\"system\"/>"
            "   <leaf name=\"uid\">  <type name=\"int32\"/> </leaf>"
            "   <leaf name=\"name\"> <type name=\"string\"/> </leaf>"
            "   <leaf name=\"group\"><type name=\"string\"/> <default value=\"ath\"/> </leaf>"
            "</list>");
    UTEST_ADD_MODULE(schema, LYS_IN_YIN, NULL, &mod);
    lysc_leaf = (void *)mod->compiled->data;
    CHECK_LYSC_NODE_LIST(lysc_leaf, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_ORDBY_SYSTEM, 1, "user", 0, \
            0, 0, 0, 0, 1, 0xffffffff, 0, 0, 0, 1, 0);
    lysc_node = lysc_leaf->child;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_KEY, 1, "uid", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_UNIQUE, 1, "name", 1, LYS_LEAF, 1, 0, 0, 0);
    lysc_node = lysc_node->next;
    CHECK_LYSC_NODE(lysc_node, 0, 0, LYS_CONFIG_W | LYS_STATUS_CURR | LYS_SET_DFLT, 1, "group", 0, LYS_LEAF, 1, 0, 0, 0);

}

static void
test_schema_print(void **state)
{
    const char *schema_yang, *schema_yin;
    char *printed;
    struct lys_module *mod;

    /* test print yang to yin */
    schema_yang = MODULE_CREATE_YANG("PRINT0",
            "list user {"
            "   min-elements 10;"
            "   max-elements 20;"
            "   key \"uid name\";"
            "   unique name;"
            "   leaf uid{type int32;}"
            "   leaf name{type string;}"
            "   leaf group{type string;}"
            "}");
    schema_yin = MODULE_CREATE_YIN("PRINT0",
            "  <list name=\"user\">\n"
            "    <key value=\"uid name\"/>\n"
            "    <unique tag=\"name\"/>\n"
            "    <min-elements value=\"10\"/>\n"
            "    <max-elements value=\"20\"/>\n"
            "    <leaf name=\"uid\">\n"
            "      <type name=\"int32\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"name\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"group\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "  </list>\n");

    UTEST_ADD_MODULE(schema_yang, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    assert_string_equal(printed, schema_yin);
    free(printed);

    /* test print yin to yang */
    schema_yang = MODULE_CREATE_YANG("PRINT1",
            "\n"
            "  list user {\n"
            "    key \"uid name\";\n"
            "    unique \"name\";\n"
            "    min-elements 10;\n"
            "    max-elements 20;\n"
            "    leaf uid {\n"
            "      type int32;\n"
            "    }\n"
            "    leaf name {\n"
            "      type string;\n"
            "    }\n"
            "    leaf group {\n"
            "      type string;\n"
            "    }\n"
            "  }\n");
    schema_yin = MODULE_CREATE_YIN("PRINT1",
            "  <list name=\"user\">\n"
            "    <key value=\"uid name\"/>\n"
            "    <unique tag=\"name\"/>\n"
            "    <min-elements value=\"10\"/>\n"
            "    <max-elements value=\"20\"/>\n"
            "    <leaf name=\"uid\">\n"
            "      <type name=\"int32\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"name\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "    <leaf name=\"group\">\n"
            "      <type name=\"string\"/>\n"
            "    </leaf>\n"
            "  </list>\n");

    UTEST_ADD_MODULE(schema_yin, LYS_IN_YIN, NULL, &mod);
    assert_non_null(mod);
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YANG, 0));
    assert_string_equal(printed, schema_yang);
    free(printed);

    /* test no segmentation fold due ignoring default value */
    schema_yang = MODULE_CREATE_YANG("PRINT2", "list user {"
            "key uid;"
            "leaf uid{type int32; default \"25\";}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");

    UTEST_ADD_MODULE(schema_yang, LYS_IN_YANG, NULL, &mod);
    assert_non_null(mod);
    assert_int_equal(LY_SUCCESS, lys_print_mem(&printed, mod, LYS_OUT_YIN, 0));
    free(printed);
}

static void
test_xml(void **state)
{
    struct lyd_node *tree;
    const char *data, *schema;
    struct lyd_node_inner *list_tree;
    struct lyd_node_term *list_leaf;

    schema = MODULE_CREATE_YANG("T0", "list user {"
            "key uid;"
            "unique \"name group\";"
            "leaf uid{type uint32;}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    /* add data */
    data =
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>1</uid>"
            "   <name>Martin Novák</name>"
            "   <group>User</group>"
            "</user>";
    /* check first item */
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    list_tree = (void *)tree;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "0", 0);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "Tomáš Novák");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "User");
    /* check second item */
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 0, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "1", 1);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "Martin Novák");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "User");
    lyd_free_all(tree);

    /* add data */
    data =
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>1</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>Admin</group>"
            "</user>";
    /* check first item */
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    list_tree = (void *)tree;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "0", 0);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "Tomáš Novák");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "User");
    /* check second item */
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 0, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "1", 1);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "Tomáš Novák");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "Admin");
    lyd_free_all(tree);

    data =
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>Admin</group>"
            "</user>";
    /* check first item */
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    assert_null(tree);
    CHECK_LOG_CTX("Duplicate instance of \"user\".",
            "Data location \"/T0:user[uid='0']\".");

    data =
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>1</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>";
    /* check first item */
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    assert_null(tree);
    CHECK_LOG_CTX("Unique data leaf(s) \"name group\" not satisfied in \"/T0:user[uid='0']\" and \"/T0:user[uid='1']\".",
            "Data location \"/T0:user[uid='1']\".");

    /* double key */
    schema = MODULE_CREATE_YANG("T1", "list user {"
            "key \"uid group\";"
            "leaf uid{type uint32;}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    data =
            "<user xmlns=\"urn:tests:T1\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T1\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>Admin</group>"
            "</user>";
    /* check first item */
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    list_tree = (void *)tree;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "0", 0);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "User");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "Tomáš Novák");
    /* check second item */
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 0, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "0", 0);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "Admin");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "Tomáš Novák");
    lyd_free_all(tree);

    data =
            "<user xmlns=\"urn:tests:T1\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T1\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>";
    /* check first item */
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    assert_null(tree);
    CHECK_LOG_CTX("Duplicate instance of \"user\".",
            "Data location \"/T1:user[uid='0'][group='User']\".");

    /* min elements max elements */
    schema = MODULE_CREATE_YANG("T2",
            "list user {"
            "   key uid;"
            "   min-elements 3;"
            "   max-elements 5;"
            "   leaf uid{type uint32;}"
            "   leaf name{type string;}"
            "   leaf group{type string;}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    data =
            "<user xmlns=\"urn:tests:T2\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T2\">"
            "   <uid>1</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>Admin</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T2\">"
            "   <uid>2</uid>"
            "   <name>Tomáš Jak</name>"
            "   <group>Admin</group>"
            "</user>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    list_tree = (void *)tree;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "0", 0);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "Tomáš Novák");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "User");
    /* check second item */
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "1", 1);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "Tomáš Novák");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "Admin");
    /* check third item */
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 0, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "2", 2);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "Tomáš Jak");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "Admin");
    lyd_free_all(tree);

    data =
            "<user xmlns=\"urn:tests:T2\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T2\">"
            "   <uid>1</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>Admin</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T2\">"
            "   <uid>2</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>Admin</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T2\">"
            "   <uid>3</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>Admin</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T2\">"
            "   <uid>4</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>Admin</group>"
            "</user>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    list_tree = (void *)tree;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 0, 0, 0, 1);
    lyd_free_all(tree);

    /* check wrong number of items */
    data =
            "<user xmlns=\"urn:tests:T2\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T2\">"
            "   <uid>1</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>Admin</group>"
            "</user>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    assert_null(tree);
    CHECK_LOG_CTX("Too few \"user\" instances.",
            "Schema location \"/T2:user\".");

    data =
            "<user xmlns=\"urn:tests:T2\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T2\">"
            "   <uid>1</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>Admin</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T2\">"
            "   <uid>2</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>Admin</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T2\">"
            "   <uid>3</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>Admin</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T2\">"
            "   <uid>4</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>Admin</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T2\">"
            "   <uid>5</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>Admin</group>"
            "</user>";
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    assert_null(tree);
    CHECK_LOG_CTX("Too many \"user\" instances.",
            "Data location \"/T2:user[uid='5']\".");

    /* empty list */
    schema = MODULE_CREATE_YANG("T_EMPTY_LIST",
            "container user_list {"
            "   list user {"
            "   key uid;"
            "   unique \"name group\";"
            "   leaf uid{type uint32;}"
            "   leaf name{type string;}"
            "   leaf group{type string;}"
            "}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    /* empty list */
    data = "<user_list xmlns=\"urn:tests:T_EMPTY_LIST\"/>";
    /* check first item */
    CHECK_PARSE_LYD_PARAM(data, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    list_tree = (void *)tree;
    CHECK_LYD_NODE_INNER(list_tree, 0, 0, 0, 1, 0, 0, 0, 1);
    lyd_free_all(tree);
}

static void
test_json(void **state)
{
    struct lyd_node *tree;
    const char *data, *schema;
    struct lyd_node_inner *list_tree;
    struct lyd_node_term *list_leaf;

    schema = MODULE_CREATE_YANG("T0", "list user {"
            "key uid;"
            "unique \"name group\";"
            "leaf uid{type uint32;}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    /* add data */
    data =
            "{\"T0:user\": ["
            "   {\"uid\":0, \"name\":\"Jan Kuba\", \"group\":\"User\"},"
            "   {\"uid\":1, \"name\":\"Martin Novák\", \"group\":\"User\"}"
            "]}";

    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    list_tree = (void *)tree;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "0", 0);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "Jan Kuba");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "User");
    /* check second item */
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 0, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "1", 1);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "Martin Novák");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "User");
    lyd_free_all(tree);

    /* Unique */
    data =
            "{\"T0:user\": ["
            "   {\"uid\":0, \"name\":\"Jan Kuba\", \"group\":\"User\"},"
            "   {\"uid\":1, \"name\":\"Jan Kuba\", \"group\":\"Admin\"}"
            "]}";

    /* check first item */
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    list_tree = (void *)tree;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "0", 0);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "Jan Kuba");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "User");
    /* check second item */
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 0, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "1", 1);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "Jan Kuba");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "Admin");
    lyd_free_all(tree);

    data =
            "{\"T0:user\": ["
            "   {\"uid\":0, \"name\":\"Jan Kuba\", \"group\":\"User\"},"
            "   {\"uid\":0, \"name\":\"Jan Kuba\", \"group\":\"Admin\"}"
            "]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    assert_null(tree);
    CHECK_LOG_CTX("Duplicate instance of \"user\".",
            "Data location \"/T0:user[uid='0']\".");

    data =
            "{\"T0:user\": ["
            "   {\"uid\":0, \"name\":\"Jan Kuba\", \"group\":\"User\"},"
            "   {\"uid\":1, \"name\":\"Jan Kuba\", \"group\":\"User\"}"
            "]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    assert_null(tree);
    CHECK_LOG_CTX("Unique data leaf(s) \"name group\" not satisfied in \"/T0:user[uid='0']\" and \"/T0:user[uid='1']\".",
            "Data location \"/T0:user[uid='1']\".");

    /* double key */
    schema = MODULE_CREATE_YANG("T1", "list user {"
            "key \"uid group\";"
            "leaf uid{type uint32;}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    data =
            "{\"T1:user\": ["
            "   {\"uid\":0, \"name\":\"Jan Kuba\", \"group\":\"User\"},"
            "   {\"uid\":0, \"name\":\"Jan Kuba\", \"group\":\"Admin\"}"
            "]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    list_tree = (void *)tree;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "0", 0);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "User");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "Jan Kuba");
    /* check second item */
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 0, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "0", 0);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "Admin");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "Jan Kuba");
    lyd_free_all(tree);

    data =
            "{\"T1:user\": ["
            "   {\"uid\":0, \"name\":\"Jan Kuba\", \"group\":\"User\"},"
            "   {\"uid\":0, \"name\":\"Jan Kuba\", \"group\":\"User\"}"
            "]}";
    /* check first item */
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    assert_null(tree);
    CHECK_LOG_CTX("Duplicate instance of \"user\".",
            "Data location \"/T1:user[uid='0'][group='User']\".");

    /* min elements max elements */
    schema = MODULE_CREATE_YANG("T2",
            "list user {"
            "   key uid;"
            "   min-elements 3;"
            "   max-elements 5;"
            "   leaf uid{type uint32;}"
            "   leaf name{type string;}"
            "   leaf group{type string;}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);
    data =
            "{\"T2:user\": ["
            "   {\"uid\":0, \"name\":\"Jan Kuba\", \"group\":\"User\"},"
            "   {\"uid\":1, \"name\":\"Antonín Kuba\", \"group\":\"User\"},"
            "   {\"uid\":2, \"name\":\"Tomáš Novák\", \"group\":\"Admin\"}"
            "]}";

    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    list_tree = (void *)tree;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "0", 0);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "Jan Kuba");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "User");
    /* check second item */
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "1", 1);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "Antonín Kuba");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "User");
    /* check third item */
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 0, 0, 0, 1);
    list_leaf = (void *) list_tree->child;
    assert_string_equal(list_leaf->schema->name, "uid");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, UINT32, "2", 2);
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "name");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 1, 1, 1, STRING, "Tomáš Novák");
    list_leaf = (void *) list_leaf->next;
    assert_string_equal(list_leaf->schema->name, "group");
    CHECK_LYD_NODE_TERM(list_leaf, 0, 0, 0, 1, 1, STRING, "Admin");
    lyd_free_all(tree);

    data =
            "{\"T2:user\": ["
            "   {\"uid\":0, \"name\":\"Jan Kuba\", \"group\":\"User\"},"
            "   {\"uid\":1, \"name\":\"Antonín Kuba\", \"group\":\"User\"},"
            "   {\"uid\":2, \"name\":\"Antonín Kuba\", \"group\":\"User\"},"
            "   {\"uid\":3, \"name\":\"Antonín Kuba\", \"group\":\"User\"},"
            "   {\"uid\":4, \"name\":\"Tomáš Novák\", \"group\":\"Admin\"}"
            "]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    list_tree = (void *)tree;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 1, 0, 0, 1);
    list_tree = (void *) list_tree->next;
    CHECK_LYD_NODE_INNER(list_tree, 1, 0, 0, 0, 0, 0, 0, 1);
    lyd_free_all(tree);

    /* check wrong number of items */
    data =
            "{\"T2:user\": ["
            "   {\"uid\":0, \"name\":\"Jan Kuba\", \"group\":\"User\"},"
            "   {\"uid\":4, \"name\":\"Tomáš Novák\", \"group\":\"Admin\"}"
            "]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    assert_null(tree);
    CHECK_LOG_CTX("Too few \"user\" instances.",
            "Schema location \"/T2:user\".");

    data =
            "{\"T2:user\": ["
            "   {\"uid\":0, \"name\":\"Jan Kuba\", \"group\":\"User\"},"
            "   {\"uid\":1, \"name\":\"Antonín Kuba\", \"group\":\"User\"},"
            "   {\"uid\":2, \"name\":\"Antonín Kuba\", \"group\":\"User\"},"
            "   {\"uid\":3, \"name\":\"Antonín Kuba\", \"group\":\"User\"},"
            "   {\"uid\":4, \"name\":\"Tomáš Novák\", \"group\":\"Admin\"},"
            "   {\"uid\":5, \"name\":\"Tomáš Novák\", \"group\":\"Admin\"}"
            "]}";
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_EVALID, tree);
    assert_null(tree);
    CHECK_LOG_CTX("Too many \"user\" instances.",
            "Data location \"/T2:user[uid='5']\".");

    schema = MODULE_CREATE_YANG("T_EMPTY_LIST",
            "container user_list {"
            "   list user {"
            "   key uid;"
            "   unique \"name group\";"
            "   leaf uid{type uint32;}"
            "   leaf name{type string;}"
            "   leaf group{type string;}"
            "}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    /* empty list */
    data =
            "{\"T_EMPTY_LIST:user_list\": {}"
            "}";

    /* check first item */
    CHECK_PARSE_LYD_PARAM(data, LYD_JSON, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, tree);
    list_tree = (void *)tree;
    CHECK_LYD_NODE_INNER(list_tree, 0, 0, 0, 1, 0, 0, 0, 1);
    lyd_free_all(tree);
}

static void
test_diff(void **state)
{
    const char *schema;
    struct lyd_node *model_1, *model_2;
    struct lyd_node *diff;
    const char *data_1;
    const char *data_2;
    const char *diff_expected;

    schema = MODULE_CREATE_YANG("T0", "list user {"
            "key uid;"
            "unique \"name group\";"
            "leaf uid{type uint32;}"
            "leaf name{type string;}"
            "leaf group{type string;}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    /* delete item */
    data_1 =
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>1</uid>"
            "   <name>Martin Novák</name>"
            "   <group>User</group>"
            "</user>";

    data_2 =
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>";

    diff_expected =
            "<user xmlns=\"urn:tests:T0\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\""
            " yang:operation=\"delete\">"
            "<uid>1</uid>"
            "<name>Martin Novák</name>"
            "<group>User</group>"
            "</user>";
    CHECK_PARSE_LYD_PARAM(data_1, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_1);
    CHECK_PARSE_LYD_PARAM(data_2, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_2);
    assert_int_equal(LY_SUCCESS, lyd_diff_siblings(model_1, model_2, 0, &diff));
    assert_non_null(diff);
    CHECK_LYD_STRING_PARAM(diff, diff_expected, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_int_equal(LY_SUCCESS, lyd_diff_apply_all(&model_1, diff));
    CHECK_LYD(model_1, model_2);
    lyd_free_all(model_1);
    lyd_free_all(model_2);
    lyd_free_all(diff);

    /* add item */
    data_1 =
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>";

    data_2 =
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>1</uid>"
            "   <name>Martin Novák</name>"
            "   <group>User</group>"
            "</user>";

    diff_expected =
            "<user xmlns=\"urn:tests:T0\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\""
            " yang:operation=\"create\">"
            "<uid>1</uid><name>Martin Novák</name><group>User</group>"
            "</user>";
    CHECK_PARSE_LYD_PARAM(data_1, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_1);
    CHECK_PARSE_LYD_PARAM(data_2, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_2);
    assert_int_equal(LY_SUCCESS, lyd_diff_siblings(model_1, model_2, 0, &diff));
    assert_non_null(diff);
    CHECK_LYD_STRING_PARAM(diff, diff_expected, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_int_equal(LY_SUCCESS, lyd_diff_apply_all(&model_1, diff));
    CHECK_LYD(model_1, model_2);
    lyd_free_all(model_1);
    lyd_free_all(model_2);
    lyd_free_all(diff);

    /* diff one  */
    data_1 =
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>1</uid>"
            "   <name>Martin Novák</name>"
            "   <group>User</group>"
            "</user>";

    data_2 =
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>Admin</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>1</uid>"
            "   <name>Martin Novák</name>"
            "   <group>User</group>"
            "</user>";

    diff_expected =
            "<user xmlns=\"urn:tests:T0\" xmlns:yang=\"urn:ietf:params:xml:ns:yang:1\""
            " yang:operation=\"none\">"
            "<uid>0</uid>"
            "<group yang:operation=\"replace\" yang:orig-default=\"false\""
            " yang:orig-value=\"User\">Admin</group></user>";
    CHECK_PARSE_LYD_PARAM(data_1, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_1);
    CHECK_PARSE_LYD_PARAM(data_2, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_2);
    assert_int_equal(LY_SUCCESS, lyd_diff_siblings(model_1, model_2, 0, &diff));
    assert_non_null(diff);
    CHECK_LYD_STRING_PARAM(diff, diff_expected, LYD_XML, LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);
    assert_int_equal(LY_SUCCESS, lyd_diff_apply_all(&model_1, diff));
    CHECK_LYD(model_1, model_2);
    lyd_free_all(model_1);
    lyd_free_all(model_2);
    lyd_free_all(diff);

    /* diff same */
    data_1 =
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>1</uid>"
            "   <name>Martin Novák</name>"
            "   <group>User</group>"
            "</user>";

    data_2 =
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "   <group>User</group>"
            "</user>"
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>1</uid>"
            "   <name>Martin Novák</name>"
            "   <group>User</group>"
            "</user>";

    CHECK_PARSE_LYD_PARAM(data_1, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_1);
    CHECK_PARSE_LYD_PARAM(data_2, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, model_2);
    assert_int_equal(LY_SUCCESS, lyd_diff_siblings(model_1, model_2, 0, &diff));
    assert_null(diff);
    assert_int_equal(LY_SUCCESS, lyd_diff_apply_all(&model_1, diff));
    CHECK_LYD(model_1, model_2);
    lyd_free_all(model_1);
    lyd_free_all(model_2);
    lyd_free_all(diff);
}

static void
test_print(void **state)
{

    const char *schema;
    const char *expected_string;

    schema = MODULE_CREATE_YANG("T0",
            "list user {"
            "key uid;"
            "leaf uid{type uint32;}"
            "leaf name{type string;}"
            "leaf group{type string; default \"User\";}"
            "}");
    UTEST_ADD_MODULE(schema, LYS_IN_YANG, NULL, NULL);

    struct lyd_node *model_1;
    const char *data_1 =
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>0</uid>"
            "   <name>Tomáš Novák</name>"
            "</user>"
            "<user xmlns=\"urn:tests:T0\">"
            "   <uid>1</uid>"
            "   <name>Martin Novák</name>"
            "   <group>Admin</group>"
            "</user>";

    LYD_TREE_CREATE(data_1, model_1);

    /* XML */
    expected_string =
            "<user xmlns=\"urn:tests:T0\">"
            "<uid>0</uid><name>Tomáš Novák</name>"
            "</user>"
            "<user xmlns=\"urn:tests:T0\">"
            "<uid>1</uid><name>Martin Novák</name><group>Admin</group>"
            "</user>";
    CHECK_LYD_STRING_PARAM(model_1, expected_string, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);

    /* JSON */
    expected_string = "{\"T0:user\":["
            "{\"uid\":0,\"name\":\"Tomáš Novák\"},"
            "{\"uid\":1,\"name\":\"Martin Novák\",\"group\":\"Admin\"}"
            "]}";
    CHECK_LYD_STRING_PARAM(model_1, expected_string, LYD_JSON, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK);

    lyd_free_all(model_1);
}

int
main(void)
{
    const struct CMUnitTest tests[] = {
        UTEST(test_schema_yang),
        UTEST(test_schema_yin),
        UTEST(test_schema_print),

        UTEST(test_xml),
        UTEST(test_json),
        UTEST(test_diff),
        UTEST(test_print),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
