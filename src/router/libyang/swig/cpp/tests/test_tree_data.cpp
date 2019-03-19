/*
 * @file test_tree_data.cpp
 * @author: Hrvoje Varga <hrvoje.varga@sartura.hr>
 * @brief unit tests for functions from tree_data.h header
 *
 * Copyright (C) 2018 Deutsche Telekom AG.
 *
 * Author: Hrvoje Varga <hrvoje.varga@sartura.hr>
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "Libyang.hpp"
#include "Tree_Data.hpp"

#include "../tests/config.h"
#include "microtest.h"
#include <string.h>

const char *lys_module_a = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>           \
<module name=\"a\"                                    \
        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"   \
        xmlns:md=\"urn:ietf:params:xml:ns:yang:ietf-yang-metadata\"\
        xmlns:a=\"urn:a\">                            \
  <namespace uri=\"urn:a\"/>                          \
  <prefix value=\"a_mod\"/>                           \
  <include module=\"asub\"/>                          \
  <include module=\"atop\"/>                          \
  <import module=\"ietf-yang-metadata\">              \
    <prefix value=\"md\"/>                            \
  </import>                                           \
  <feature name=\"foo\"/>                             \
  <grouping name=\"gg\">                              \
    <leaf name=\"bar-gggg\">                          \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </grouping>                                         \
  <md:annotation name=\"test\">                       \
    <type name=\"string\"/>                           \
  </md:annotation>                                    \
  <container name=\"x\">                              \
    <leaf name=\"bar-leaf\">                          \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <uses name=\"gg\">                                \
    </uses>                                           \
    <leaf name=\"baz\">                               \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <leaf name=\"bubba\">                             \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <leaf name=\"number32\">                          \
      <type name=\"int32\"/>                          \
    </leaf>                                           \
    <leaf name=\"number64\">                          \
      <type name=\"int64\"/>                          \
    </leaf>                                           \
    <leaf name=\"def-leaf\">                          \
      <type name=\"string\"/>                         \
      <default value=\"def\"/>                        \
    </leaf>                                           \
    <anydata name=\"any-data\"/>                              \
  </container>                                        \
  <leaf name=\"y\"><type name=\"string\"/></leaf>     \
  <anyxml name=\"any\"/>                              \
  <augment target-node=\"/x\">                        \
    <container name=\"bar-y\"/>                       \
  </augment>                                          \
  <rpc name=\"bar-rpc\">                              \
  </rpc>                                              \
  <rpc name=\"foo-rpc\">                              \
  </rpc>                                              \
  <rpc name=\"rpc1\">                                 \
    <input>                                           \
      <leaf name=\"input-leaf1\">                     \
        <type name=\"string\"/>                       \
      </leaf>                                         \
      <container name=\"x\">                          \
        <leaf name=\"input-leaf2\">                   \
          <type name=\"string\"/>                     \
        </leaf>                                       \
      </container>                                    \
    </input>                                          \
    <output>                                          \
      <leaf name=\"output-leaf1\">                    \
        <type name=\"string\"/>                       \
      </leaf>                                         \
      <leaf name=\"output-leaf2\">                    \
        <type name=\"string\"/>                       \
      </leaf>                                         \
      <container name=\"rpc-container\">              \
        <leaf name=\"output-leaf3\">                  \
          <type name=\"string\"/>                     \
        </leaf>                                       \
      </container>                                    \
    </output>                                         \
  </rpc>                                              \
  <list name=\"l\">                                   \
    <key value=\"key1 key2\"/>                        \
    <leaf name=\"key1\">                              \
      <type name=\"uint8\"/>                          \
    </leaf>                                           \
    <leaf name=\"key2\">                              \
      <type name=\"uint8\"/>                          \
    </leaf>                                           \
    <leaf name=\"value\">                             \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </list>                                             \
</module>                                             \
";

const char *result_xml = "<x xmlns=\"urn:a\"><bubba>test</bubba></x>";
const char *result_xml_format ="\
<x xmlns=\"urn:a\">\n\
  <bubba>test</bubba>\n\
</x>\n\
";

const char *result_json = "\
{\n\
  \"a:x\": {\n\
    \"bubba\": \"test\"\n\
  }\n\
}\n\
";

TEST(test_ly_ctx_parse_data_mem)
{
    const char *a_data_xml = "\
                    <x xmlns=\"urn:a\">\n\
                        <bubba>test</bubba>\n\
                    </x>\n";

    const char *yang_folder = TESTS_DIR "/api/files";
    const char *yin_file = TESTS_DIR "/api/files/a.yin";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_path(yin_file, LYS_IN_YIN);

        auto root = ctx->parse_data_mem(a_data_xml, LYD_XML, LYD_OPT_NOSIBLINGS | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);
        ASSERT_STREQ("x", root->schema()->name());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_parse_data_fd)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *yin_file = TESTS_DIR "/api/files/a.yin";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_path(yin_file, LYS_IN_YIN);

        FILE *f = fopen(config_file, "r");
        auto fd = fileno(f);
        auto root = ctx->parse_data_fd(fd, LYD_XML, LYD_OPT_NOSIBLINGS | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);
        ASSERT_STREQ("x", root->schema()->name());
        fclose(f);
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_parse_data_path)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *yin_file = TESTS_DIR "/api/files/a.yin";
    const char *config_file = TESTS_DIR "/api/files/a.xml";
    const char *module_name = "a";
    const char *schema_name = "x";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        auto module = ctx->parse_module_path(yin_file, LYS_IN_YIN);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name, module->name());

        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);
        ASSERT_STREQ(schema_name, root->schema()->name());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_parse_data_path_invalid)
{
    const char *yang_folder = TESTS_DIR "/api/files";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        auto root = ctx->parse_data_path("INVALID_PATH", LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        throw std::runtime_error("exception not thrown");
    } catch( const std::exception& e ) {
        ASSERT_NOTNULL(strstr(e.what(), "INVALID_PATH"));
        return;
    }
}

TEST(test_ly_data_node)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        auto new_node = std::make_shared<libyang::Data_Node>(root, root->child()->schema()->module(), "bar-y");
        ASSERT_NOTNULL(new_node);
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_new_path)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        auto mod = ctx->get_module("a", nullptr, 1);
        ASSERT_NOTNULL(mod);

        auto root = std::make_shared<libyang::Data_Node>(ctx, "/a:x/bar-gggg", "a", LYD_ANYDATA_CONSTSTRING, 0);
        ASSERT_NOTNULL(root);
        ASSERT_STREQ("x", root->schema()->name());
        ASSERT_STREQ("bar-gggg", root->child()->schema()->name());

        auto node = root->new_path(ctx, "def-leaf", "def", LYD_ANYDATA_CONSTSTRING, LYD_PATH_OPT_DFLT);
        ASSERT_NOTNULL(node);
        ASSERT_STREQ("def-leaf", node->schema()->name());
        ASSERT_EQ(1, node->dflt());

        node = root->new_path(ctx, "def-leaf", "def", LYD_ANYDATA_CONSTSTRING, 0);
        ASSERT_NOTNULL(node);
        ASSERT_STREQ("def-leaf", node->schema()->name());
        ASSERT_EQ(0, node->dflt());

        node = root->new_path(ctx, "bubba", "b", LYD_ANYDATA_CONSTSTRING, 0);
        ASSERT_NOTNULL(node);
        ASSERT_STREQ("bubba", node->schema()->name());

        node = root->new_path(ctx, "/a:x/number32", "3", LYD_ANYDATA_CONSTSTRING, 0);
        ASSERT_NOTNULL(node);
        ASSERT_STREQ("number32", node->schema()->name());

        node = root->new_path(ctx, "/a:l[key1='1'][key2='2']/value", nullptr, LYD_ANYDATA_CONSTSTRING, 0);
        ASSERT_NOTNULL(node);
        ASSERT_STREQ("l", node->schema()->name());
        ASSERT_STREQ("key1", node->child()->schema()->name());
        ASSERT_STREQ("key2", node->child()->next()->schema()->name());
        ASSERT_STREQ("value", node->child()->next()->next()->schema()->name());
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_insert)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);
        auto new_node = std::make_shared<libyang::Data_Node>(root, root->schema()->module(), "number32", "200");
        ASSERT_NOTNULL(new_node);
        auto rc = root->insert(new_node);
        ASSERT_EQ(0, rc);
        ASSERT_STREQ("number32", root->child()->prev()->schema()->name());
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_insert_sibling)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        auto last = root->prev();
        auto new_node = std::make_shared<libyang::Data_Node>(nullptr, root->schema()->module(), "y", "test");
        ASSERT_NOTNULL(new_node);
        auto rc = root->insert_sibling(new_node);
        ASSERT_EQ(0, rc);
        ASSERT_STRNEQ(last->schema()->name(), root->prev()->schema()->name());
        ASSERT_STREQ("y", root->prev()->schema()->name());
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_insert_before)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        auto last = root->prev();
        auto new_node = std::make_shared<libyang::Data_Node>(nullptr, root->schema()->module(), "y", "test");
        ASSERT_NOTNULL(new_node);
        auto rc = root->insert_before(new_node);
        ASSERT_EQ(0, rc);
        ASSERT_STRNEQ(last->schema()->name(), root->prev()->schema()->name());
        ASSERT_STREQ("y", root->prev()->schema()->name());
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_insert_after)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        auto last = root->next();
        auto new_node = std::make_shared<libyang::Data_Node>(nullptr, root->schema()->module(), "y", "test");
        ASSERT_NOTNULL(new_node);
        auto rc = root->insert_after(new_node);
        ASSERT_EQ(0, rc);
        ASSERT_STRNEQ(last->schema()->name(), root->next()->schema()->name());
        ASSERT_STREQ("y", root->next()->schema()->name());
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_schema_sort)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        auto mod = ctx->get_module("a", nullptr, 1);
        ASSERT_NOTNULL(mod);

        auto root = std::make_shared<libyang::Data_Node>(nullptr, mod, "l");
        ASSERT_NOTNULL(root);
        auto node = std::make_shared<libyang::Data_Node>(root, mod, "key1", "1");
        ASSERT_NOTNULL(node);
        node = std::make_shared<libyang::Data_Node>(root, mod, "key2", "2");
        ASSERT_NOTNULL(node);

        node = std::make_shared<libyang::Data_Node>(nullptr, mod, "x");
        ASSERT_NOTNULL(node);
        auto rc = root->insert_after(node);
        ASSERT_EQ(0, rc);
        node = root->next();

        auto node2 = std::make_shared<libyang::Data_Node>(node, mod, "bubba", "a");
        ASSERT_NOTNULL(node2);
        node2 = std::make_shared<libyang::Data_Node>(node, mod, "bar-gggg", "b");
        ASSERT_NOTNULL(node2);
        node2 = std::make_shared<libyang::Data_Node>(node, mod, "number64", "64");
        ASSERT_NOTNULL(node2);
        node2 = std::make_shared<libyang::Data_Node>(node, mod, "number32", "32");
        ASSERT_NOTNULL(node2);

        rc = root->schema_sort(1);
        ASSERT_EQ(0, rc);

        root = node;
        ASSERT_STREQ("x", root->schema()->name());
        ASSERT_STREQ("l", root->next()->schema()->name());

        ASSERT_STREQ("bar-gggg", root->child()->schema()->name());
        ASSERT_STREQ("bubba", root->child()->next()->schema()->name());
        ASSERT_STREQ("number32", root->child()->next()->next()->schema()->name());
        ASSERT_STREQ("number64", root->child()->next()->next()->next()->schema()->name());
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_find_path)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        auto node = root->child();
        ASSERT_NOTNULL(node);
        auto set = node->find_path("/a:x/bubba");
        ASSERT_NOTNULL(set);
        ASSERT_EQ(1, set->number());
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_find_instance)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        auto node = root->child();
        ASSERT_NOTNULL(node);
        auto set = node->find_instance(node->schema());
        ASSERT_NOTNULL(set);
        ASSERT_EQ(1, set->number());
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_validate_ctx)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        auto rc = root->validate(LYD_OPT_CONFIG, ctx);
        ASSERT_EQ(0, rc);
        auto new_node = std::make_shared<libyang::Data_Node>(root, root->schema()->module(), "number32", "1");
        ASSERT_NOTNULL(new_node);
        rc = root->insert(new_node);
        ASSERT_EQ(0, rc);
        rc = root->validate(LYD_OPT_CONFIG, ctx);
        ASSERT_EQ(0, rc);
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_unlink)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        auto node = root->child();
        auto new_node = std::make_shared<libyang::Data_Node>(root, root->schema()->module(), "number32", "1");
        ASSERT_NOTNULL(new_node);
        auto rc = root->insert(new_node);
        ASSERT_EQ(0, rc);

        ASSERT_STREQ("number32", node->prev()->schema()->name());

        rc = node->prev()->unlink();
        ASSERT_EQ(0, rc);

        ASSERT_STRNEQ("number32", node->prev()->schema()->name());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_print_mem_xml)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        auto result = root->print_mem(LYD_XML, 0);
        ASSERT_STREQ(result_xml, result);
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_print_mem_xml_format)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        auto result = root->print_mem(LYD_XML, LYP_FORMAT);
        ASSERT_STREQ(result_xml_format, result);
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_print_mem_json)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        auto result = root->print_mem(LYD_JSON, LYP_FORMAT);
        ASSERT_STREQ(result_json, result);
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_path)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        std::string path = root->path();
        ASSERT_NOTNULL(path.c_str());
        ASSERT_STREQ("/a:x", path.c_str());
        path = root->child()->path();
        ASSERT_NOTNULL(path.c_str());
        ASSERT_STREQ("/a:x/bubba", path.c_str());
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_leaf)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        auto new_node = std::make_shared<libyang::Data_Node>(root, root->schema()->module(), "number32", "100");
        ASSERT_NOTNULL(new_node);
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_anydata)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);
        auto mod = ctx->get_module("a", nullptr, 1);

        auto new_node = std::make_shared<libyang::Data_Node>(root, mod, "any-data", "100", LYD_ANYDATA_CONSTSTRING);
        ASSERT_NOTNULL(new_node);
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_dup)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        auto new_node = std::make_shared<libyang::Data_Node>(root, root->child()->schema()->module(), "bar-y");
        ASSERT_NOTNULL(new_node);
        auto dup_node = new_node->dup(0);
        ASSERT_NOTNULL(dup_node);
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_dup_to_ctx)
{
    const char *sch = "module x {"
                    "  namespace urn:x;"
                    "  prefix x;"
                    "  leaf x { type string; }}";
    const char *data = "<x xmlns=\"urn:x\">hello</x>";

    try {
        auto ctx1 = std::make_shared<libyang::Context>(nullptr);
        ASSERT_NOTNULL(ctx1);
        ctx1->parse_module_mem(sch, LYS_IN_YANG);
        auto data1 = ctx1->parse_data_mem(data, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(data1);

        auto ctx2 = std::make_shared<libyang::Context>(nullptr);
        ASSERT_NOTNULL(ctx2);
        // we expect NULL due to missing schema in the second ctx
        auto dup_node = data1->dup_to_ctx(1, ctx2);
        ASSERT_NULL(dup_node);

        ctx2->parse_module_mem(sch, LYS_IN_YANG);
        // now we expect success due to schema being added to the second ctx
        dup_node = data1->dup_to_ctx(0, ctx2);
        ASSERT_NOTNULL(dup_node);
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_validate_node)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        auto rc = root->validate(LYD_OPT_CONFIG, ctx);
        ASSERT_EQ(0, rc);
        auto new_node = std::make_shared<libyang::Data_Node>(root, root->schema()->module(), "number32", "1");
        ASSERT_NOTNULL(new_node);
        rc = root->validate(LYD_OPT_CONFIG, new_node);
        ASSERT_EQ(0, rc);
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_data_node_validate_value)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        auto rc = root->validate(LYD_OPT_CONFIG, ctx);
        ASSERT_EQ(0, rc);
        auto new_node = std::make_shared<libyang::Data_Node>(root, root->schema()->module(), "number32", "1");
        ASSERT_NOTNULL(new_node);
        ASSERT_EQ(new_node->validate_value("1"), 0);
        ASSERT_EQ(new_node->validate_value("100"), 0);
        ASSERT_EQ(new_node->validate_value("110000000"), 0);
    } catch (const std::exception &e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST_MAIN();
