/*
 * @file test_tree_schema.cpp
 * @author: Hrvoje Varga <hrvoje.varga@sartura.hr>
 * @brief unit tests for functions from tree_schema.h header
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
#include "Tree_Schema.hpp"
#include "microtest.h"
#include "../tests/config.h"

#include <cstdio>

const char *lys_module_a = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>           \
<module name=\"a\"                                    \
        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"   \
        xmlns:a=\"urn:a\">                            \
  <namespace uri=\"urn:a\"/>                          \
  <prefix value=\"a_mod\"/>                           \
  <include module=\"asub\"/>                          \
  <include module=\"atop\"/>                          \
  <revision date=\"2015-01-01\">                      \
    <description>                                     \
      <text>version 1</text>                          \
    </description>                                    \
    <reference>                                       \
      <text>RFC XXXX</text>                           \
    </reference>                                      \
  </revision>                                         \
  <feature name=\"foo\"/>                             \
  <grouping name=\"gg\">                              \
    <leaf name=\"bar-gggg\">                          \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </grouping>                                         \
  <container name=\"x\">                              \
    <leaf name=\"bar-leaf\">                          \
      <if-feature name=\"bar\"/>                      \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <uses name=\"gg\">                                \
      <if-feature name=\"bar\"/>                      \
    </uses>                                           \
    <leaf name=\"baz\">                               \
      <if-feature name=\"foo\"/>                      \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <leaf name=\"bubba\">                             \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </container>                                        \
  <augment target-node=\"/x\">                        \
    <if-feature name=\"bar\"/>                        \
    <container name=\"bar-y\">                        \
      <leaf name=\"ll\">                              \
        <type name=\"string\"/>                       \
      </leaf>                                         \
    </container>                                      \
  </augment>                                          \
  <rpc name=\"bar-rpc\">                              \
    <if-feature name=\"bar\"/>                        \
  </rpc>                                              \
  <rpc name=\"foo-rpc\">                              \
    <if-feature name=\"foo\"/>                        \
  </rpc>                                              \
</module>                                             \
";

const char *lys_module_b = \
"module b {\
    namespace \"urn:b\";\
    prefix b_mod;\
    include bsub;\
    include btop;\
    feature foo;\
    grouping gg {\
        leaf bar-gggg {\
            type string;\
        }\
    }\
    container x {\
        leaf bar-leaf {\
            if-feature \"bar\";\
            type string;\
        }\
        uses gg {\
            if-feature \"bar\";\
        }\
        leaf baz {\
            if-feature \"foo\";\
            type string;\
        }\
        leaf bubba {\
            type string;\
        }\
    }\
    augment \"/x\" {\
        if-feature \"bar\";\
        container bar-y;\
    }\
    rpc bar-rpc {\
        if-feature \"bar\";\
    }\
    rpc foo-rpc {\
        if-feature \"foo\";\
    }\
}";

const char *lys_module_a_with_typo = \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>           \
<module_typo name=\"a\"                                    \
        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"   \
        xmlns:a=\"urn:a\">                            \
  <namespace uri=\"urn:a\"/>                          \
  <prefix value=\"a_mod\"/>                           \
  <include module=\"asub\"/>                          \
  <include module=\"atop\"/>                          \
  <feature name=\"foo\"/>                             \
  <grouping name=\"gg\">                              \
    <leaf name=\"bar-gggg\">                          \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </grouping>                                         \
  <container name=\"x\">                              \
    <leaf name=\"bar-leaf\">                          \
      <if-feature name=\"bar\"/>                      \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <uses name=\"gg\">                                \
      <if-feature name=\"bar\"/>                      \
    </uses>                                           \
    <leaf name=\"baz\">                               \
      <if-feature name=\"foo\"/>                      \
      <type name=\"string\"/>                         \
    </leaf>                                           \
    <leaf name=\"bubba\">                             \
      <type name=\"string\"/>                         \
    </leaf>                                           \
  </container>                                        \
  <augment target-node=\"/x\">                        \
    <if-feature name=\"bar\"/>                        \
    <container name=\"bar-y\">                        \
      <leaf name=\"ll\">                              \
        <type name=\"string\"/>                       \
      </leaf>                                         \
    </container>                                      \
  </augment>                                          \
  <rpc name=\"bar-rpc\">                              \
    <if-feature name=\"bar\"/>                        \
  </rpc>                                              \
  <rpc name=\"foo-rpc\">                              \
    <if-feature name=\"foo\"/>                        \
  </rpc>                                              \
</module>                                             \
";

const char *result_tree = "\
module: a\n\
  +--rw top\n\
  |  +--rw bar-sub2\n\
  +--rw x\n\
     +--rw bubba?      string\n";

const char *result_yang = "\
module a {\n\
  namespace \"urn:a\";\n\
  prefix a_mod;\n\
\n\
  include \"asub\";\n\
\n\
  include \"atop\";\n\
\n\
  revision 2015-01-01 {\n\
    description\n\
      \"version 1\";\n\
    reference\n\
      \"RFC XXXX\";\n\
  }\n\
\n\
  feature foo;\n\
\n\
  grouping gg {\n\
    leaf bar-gggg {\n\
      type string;\n\
    }\n\
  }\n\
\n\
  container x {\n\
    leaf bar-leaf {\n\
      if-feature \"bar\";\n\
      type string;\n\
    }\n\n\
    uses gg {\n\
      if-feature \"bar\";\n\
    }\n\n\
    leaf baz {\n\
      if-feature \"foo\";\n\
      type string;\n\
    }\n\n\
    leaf bubba {\n\
      type string;\n\
    }\n\
  }\n\
\n\
  augment \"/x\" {\n\
    if-feature \"bar\";\n\
    container bar-y {\n\
      leaf ll {\n\
        type string;\n\
      }\n\
    }\n\
  }\n\
\n\
  rpc bar-rpc {\n\
    if-feature \"bar\";\n\
  }\n\
\n\
  rpc foo-rpc {\n\
    if-feature \"foo\";\n\
  }\n\
}\n";

const char *result_yin = "\
<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<module name=\"a\"\n\
        xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\"\n\
        xmlns:a_mod=\"urn:a\">\n\
  <namespace uri=\"urn:a\"/>\n\
  <prefix value=\"a_mod\"/>\n\
  <include module=\"asub\"/>\n\
  <include module=\"atop\"/>\n\
  <revision date=\"2015-01-01\">\n\
    <description>\n\
      <text>version 1</text>\n\
    </description>\n\
    <reference>\n\
      <text>RFC XXXX</text>\n\
    </reference>\n\
  </revision>\n\
  <feature name=\"foo\"/>\n\
  <grouping name=\"gg\">\n\
    <leaf name=\"bar-gggg\">\n\
      <type name=\"string\"/>\n\
    </leaf>\n\
  </grouping>\n\
  <container name=\"x\">\n\
    <leaf name=\"bar-leaf\">\n\
      <if-feature name=\"bar\"/>\n\
      <type name=\"string\"/>\n\
    </leaf>\n\
    <uses name=\"gg\">\n\
      <if-feature name=\"bar\"/>\n\
    </uses>\n\
    <leaf name=\"baz\">\n\
      <if-feature name=\"foo\"/>\n\
      <type name=\"string\"/>\n\
    </leaf>\n\
    <leaf name=\"bubba\">\n\
      <type name=\"string\"/>\n\
    </leaf>\n\
  </container>\n\
  <augment target-node=\"/x\">\n\
    <if-feature name=\"bar\"/>\n\
    <container name=\"bar-y\">\n\
      <leaf name=\"ll\">\n\
        <type name=\"string\"/>\n\
      </leaf>\n\
    </container>\n\
  </augment>\n\
  <rpc name=\"bar-rpc\">\n\
    <if-feature name=\"bar\"/>\n\
  </rpc>\n\
  <rpc name=\"foo-rpc\">\n\
    <if-feature name=\"foo\"/>\n\
  </rpc>\n\
</module>\n";

TEST(test_ly_ctx_parse_module_mem)
{
    const char *yang_folder = TESTS_DIR "/api/files";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        auto module = ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ("a", module->name());

        module = ctx->parse_module_mem(lys_module_b, LYS_IN_YANG);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ("b", module->name());
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_parse_module_mem_invalid)
{
    const char *yang_folder = TESTS_DIR "/api/files";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        ctx->parse_module_mem(lys_module_a_with_typo, LYS_IN_YIN);
        throw std::runtime_error("exception not thrown");
    } catch (const std::exception& e) {
        ASSERT_STREQ("Module parsing failed.", e.what());
        return;
    }
}

TEST(test_ly_ctx_parse_module_fd)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *yin_file = TESTS_DIR "/api/files/a.yin";
    const char *yang_file = TESTS_DIR "/api/files/b.yang";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        FILE *f = fopen(yin_file, "r");
        auto fd = fileno(f);
        auto module = ctx->parse_module_fd(fd, LYS_IN_YIN);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ("a", module->name());
        fclose(f);
        f = fopen(yang_file, "r");
        fd = fileno(f);
        module = ctx->parse_module_fd(fd, LYS_IN_YANG);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ("b", module->name());
        fclose(f);
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_parse_module_fd_invalid)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *yin_file = TESTS_DIR "/api/files/a.yin";
    FILE *f;

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        f = fopen(yin_file, "r");
        auto fd = fileno(f);
        auto module = ctx->parse_module_fd(fd, LYS_IN_YANG);
        throw std::runtime_error("exception not thrown");
    } catch( const std::exception& e ) {
        ASSERT_STREQ("Module parsing failed.", e.what());
        fclose(f);
        return;
    }
}

TEST(test_ly_ctx_parse_module_path)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *yin_file = TESTS_DIR "/api/files/a.yin";
    const char *yang_file = TESTS_DIR "/api/files/b.yang";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        auto module = ctx->parse_module_path(yin_file, LYS_IN_YIN);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ("a", module->name());

        module = ctx->parse_module_path(yang_file, LYS_IN_YANG);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ("b", module->name());
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_parse_module_path_invalid)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *yin_file = TESTS_DIR "/api/files/a.yin";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        auto module = ctx->parse_module_path(yin_file, LYS_IN_YANG);
        throw std::runtime_error("exception not thrown");
    } catch( const std::exception& e ) {
        ASSERT_STREQ("Module parsing failed.", e.what());
        return;
    }
}

TEST(test_ly_module_print_mem_tree)
{
    const char *yang_folder = TESTS_DIR "/api/files";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        auto module = ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        ASSERT_NOTNULL(module);

        auto result = module->print_mem(LYS_OUT_TREE, 0);
        ASSERT_STREQ(result_tree, result);
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_module_print_mem_yang)
{
    const char *yang_folder = TESTS_DIR "/api/files";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        auto module = ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        ASSERT_NOTNULL(module);

        auto result = module->print_mem(LYS_OUT_YANG, 0);
        ASSERT_STREQ(result_yang, result);
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_module_print_mem_yin)
{
    const char *yang_folder = TESTS_DIR "/api/files";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        auto module = ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        ASSERT_NOTNULL(module);

        auto result = module->print_mem(LYS_OUT_YIN, 0);
        ASSERT_STREQ(result_yin, result);
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_schema_node_find_path)
{
    const char *yang_folder = TESTS_DIR "/api/files";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        auto module = ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        ASSERT_NOTNULL(module);
        auto schema_node = module->data();
        ASSERT_NOTNULL(schema_node);

        auto set = schema_node->find_path("/a:x/*");
        ASSERT_NOTNULL(set);
        ASSERT_EQ(5, set->number());
        set = schema_node->find_path("/a:x//*");
        ASSERT_NOTNULL(set);
        ASSERT_EQ(6, set->number());
        set = schema_node->find_path("/a:x//.");
        ASSERT_NOTNULL(set);
        ASSERT_EQ(7, set->number());
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_schema_node_path)
{
    const char *yang_folder = TESTS_DIR "/api/files";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        auto module = ctx->parse_module_mem(lys_module_a, LYS_IN_YIN);
        ASSERT_NOTNULL(module);
        auto schema_node = module->data();
        ASSERT_NOTNULL(schema_node);

        const char *path_template = "/a:x/a:bar-gggg";
        auto set = schema_node->find_path(path_template);
        ASSERT_NOTNULL(set);

        auto schemas = set->schema();
        auto schema = schemas.at(0);
        auto path = schema->path(0);
        ASSERT_STREQ(path_template, path);
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_module_data_instatiables)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *module_name1 = "b";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        auto module = ctx->load_module(module_name1);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name1, module->name());

        auto list = std::make_shared<std::vector<libyang::S_Schema_Node>>(module->data_instantiables(0));
        ASSERT_NOTNULL(list);
        ASSERT_EQ(1, list->size());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_schema_child_instatiables)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *module_name = "b";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        auto module = ctx->load_module(module_name);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name, module->name());

        auto list = std::make_shared<std::vector<libyang::S_Schema_Node>>(module->data_instantiables(0));
        ASSERT_NOTNULL(list);
        ASSERT_EQ(1, list->size());
        auto child_list = std::make_shared<std::vector<libyang::S_Schema_Node>>(list->front()->child_instantiables(0));
        ASSERT_NOTNULL(child_list);
        ASSERT_EQ(3, child_list->size());

    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_iffeature)
{
    const char *yang_folder = TESTS_DIR "/api/files";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        auto module = ctx->parse_module_mem(lys_module_b, LYS_IN_YANG);
        ASSERT_NOTNULL(module);

        ASSERT_EQ(module->feature_enable("foo"), 0);

        auto cont = ctx->get_node(nullptr, "/b:x");
        ASSERT_NOTNULL(cont);

        auto snode = cont->child();
        ASSERT_NOTNULL(snode);
        ASSERT_STREQ(snode->name(), "bar-leaf");

        auto iffeatures = snode->iffeature();
        ASSERT_EQ(iffeatures.size(), 1);

        ASSERT_EQ(iffeatures[0]->value(), 0);

        snode = snode->next()->next();
        ASSERT_NOTNULL(snode);
        ASSERT_STREQ(snode->name(), "baz");

        iffeatures = snode->iffeature();
        ASSERT_EQ(iffeatures.size(), 1);

        ASSERT_EQ(iffeatures[0]->value(), 1);
    } catch (const std::exception& e) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST_MAIN();
