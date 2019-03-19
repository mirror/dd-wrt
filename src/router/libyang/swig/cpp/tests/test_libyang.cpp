/*
 * @file test_libyang.cpp
 * @author: Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief unit tests for functions from libyang.h header
 *
 * Copyright (C) 2018 Deutsche Telekom AG.
 *
 * Author: Mislav Novakovic <mislav.novakovic@sartura.hr>
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
#include <string.h>

#include "Libyang.hpp"
#include "Tree_Data.hpp"
#include "Tree_Schema.hpp"

#include "../tests/config.h"
#include "libyang.h"

/* include private header to be able to check internal values */
#include "../../../src/context.h"

#include "microtest.h"

TEST(test_ly_ctx_new)
{
    const char *yang_folder1 = TESTS_DIR "/data/files";
    const char *yang_folder2 = TESTS_DIR "/data:" TESTS_DIR "/data/files";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder1);
        ASSERT_FALSE(nullptr == ctx);
        auto list = ctx->get_searchdirs();
        ASSERT_EQ(1, list.size());

        ctx = std::make_shared<libyang::Context>(yang_folder2);
        ASSERT_FALSE(nullptr == ctx);
        list = ctx->get_searchdirs();
        ASSERT_EQ(2, list.size());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_new_invalid)
{
    const char *yang_folder = "INVALID_PATH";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_FALSE("exception not thrown");
    } catch( const std::exception& e ) {
        ASSERT_NOTNULL(strstr(e.what(), "No Context"));
        return;
    }
}

TEST(test_ly_ctx_get_searchdirs)
{
    const char *yang_folder = TESTS_DIR "/data/files";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_FALSE(nullptr == ctx);

        auto list = ctx->get_searchdirs();
        ASSERT_EQ(1, list.size());
        ASSERT_EQ(yang_folder, list.at(0));
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_set_searchdir)
{
    const char *yang_folder = TESTS_DIR "/data/files";
    const char *new_yang_folder = TESTS_DIR "/schema/yin";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_FALSE(nullptr == ctx);

        auto list = ctx->get_searchdirs();
        ASSERT_EQ(1, list.size());
        ASSERT_EQ(yang_folder, list.at(0));

        ctx->set_searchdir(new_yang_folder);
        list = ctx->get_searchdirs();
        ASSERT_EQ(2, list.size());
        ASSERT_EQ(new_yang_folder, list.at(1));

        ctx->unset_searchdirs(0);
        list = ctx->get_searchdirs();
        ASSERT_EQ(1, list.size());
        ASSERT_EQ(new_yang_folder, list.at(0));
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_set_searchdir_invalid)
{
    const char *yang_folder = TESTS_DIR "/data/files";
    const char *new_yang_folder = TESTS_DIR "INVALID_PATH";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_FALSE(nullptr == ctx);

        ctx->set_searchdir(new_yang_folder);
        throw std::runtime_error("exception not thrown");
    } catch( const std::exception& e ) {
        ASSERT_NOTNULL(strstr(e.what(), new_yang_folder));
        return;
    }
}

TEST(test_ly_ctx_info)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_FALSE(nullptr == ctx);

        auto info = ctx->info();
        ASSERT_FALSE(nullptr == info);
        ASSERT_EQ(LYD_VAL_OK, info->validity());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_load_module_invalid)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_FALSE(nullptr == ctx);

        auto module = ctx->load_module("invalid", nullptr);
        throw std::runtime_error("exception not thrown");
    } catch( const std::exception& e ) {
        ASSERT_NOTNULL(strstr(e.what(), "invalid"));
        return;
    }
}

TEST(test_ly_ctx_load_get_module)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *name1 = "a";
    const char *name2 = "b";
    const char *revision = "2016-03-01";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        auto module = ctx->get_module("invalid");
        ASSERT_NULL(module);

        module = ctx->get_module(name1);
        ASSERT_NULL(module);

        module = ctx->load_module(name1);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(name1, module->name());

        module = ctx->load_module(name2, revision);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(name2, module->name());
        ASSERT_STREQ(revision, module->rev()->date());

        module = ctx->get_module(name2, "INVALID_REVISION");
        ASSERT_NULL(module);

        module = ctx->get_module(name1);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(name1, module->name());

        module = ctx->get_module(name2, revision);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(name2, module->name());
        ASSERT_STREQ(revision, module->rev()->date());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_get_module_older)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *name = "b";
    const char *revision = "2016-03-01";
    const char *revision_older = "2015-01-01";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        auto module = ctx->load_module("c");
        ASSERT_NOTNULL(module);
        ASSERT_STREQ("c", module->name());

        module = ctx->load_module(name, revision);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(name, module->name());
        ASSERT_STREQ(revision, module->rev()->date());

        auto module_older = ctx->get_module_older(module);
        ASSERT_NOTNULL(module_older);
        ASSERT_STREQ(name, module_older->name());
        ASSERT_STREQ(revision_older, module_older->rev()->date());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_get_module_by_ns)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *module_name = "a";
    const char *ns = "urn:a";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        auto module = ctx->load_module(module_name);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name, module->name());

        module = ctx->get_module_by_ns(ns);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name, module->name());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_clean)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *module_name = "a";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        auto module = ctx->load_module(module_name);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name, module->name());

        module = ctx->get_module(module_name);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name, module->name());

        ctx->clean();

        module = ctx->get_module(module_name);
        ASSERT_NULL(module);
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_parse_module_path)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *yin_file = TESTS_DIR "/api/files/a.yin";
    const char *yang_file = TESTS_DIR "/api/files/b.yang";
    const char *module_name1 = "a";
    const char *module_name2 = "b";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        auto module = ctx->parse_module_path(yin_file, LYS_IN_YIN);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name1, module->name());

        module = ctx->parse_module_path(yang_file, LYS_IN_YANG);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name2, module->name());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_parse_module_path_invalid)
{
    const char *yang_folder = TESTS_DIR "/api/files";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        auto module = ctx->parse_module_path("INVALID_YANG_FILE", LYS_IN_YANG);
        throw std::logic_error("exception not thrown");
    } catch( const std::exception& e ) {
        ASSERT_NOTNULL(strstr(e.what(), "INVALID_YANG_FILE"));
        return;
    }
}

TEST(test_ly_ctx_get_submodule)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *yin_file = TESTS_DIR "/api/files/a.yin";
    const char *module_name = "a";
    const char *sub_name = "asub";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_path(yin_file, LYS_IN_YIN);

        auto submodule = ctx->get_submodule(module_name, nullptr, sub_name, nullptr);
        ASSERT_NOTNULL(submodule);
        ASSERT_STREQ(sub_name, submodule->name());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_get_submodule2)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *yin_file = TESTS_DIR "/api/files/a.yin";
    const char *config_file = TESTS_DIR "/api/files/a.xml";
    const char *sub_name = "asub";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_path(yin_file, LYS_IN_YIN);

        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);
        ASSERT_NOTNULL(root->schema()->module());

        auto submodule = ctx->get_submodule2(root->schema()->module(), sub_name);
        ASSERT_NOTNULL(submodule);
        ASSERT_STREQ(sub_name, submodule->name());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_find_path)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *yin_file = TESTS_DIR "/api/files/a.yin";
    const char *yang_file = TESTS_DIR "/api/files/b.yang";
    const char *schema_path1 = "/b:x/b:bubba";
    const char *schema_path2 = "/a:x/a:bubba";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        ctx->parse_module_path(yang_file, LYS_IN_YANG);
        auto set = ctx->find_path(schema_path1);
        ASSERT_NOTNULL(set);

        ctx->parse_module_path(yin_file, LYS_IN_YIN);
        set = ctx->find_path(schema_path2);
        ASSERT_NOTNULL(set);
        std::make_shared<libyang::Set>();
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_set)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *yin_file = TESTS_DIR "/api/files/a.yin";
    const char *config_file = TESTS_DIR "/api/files/a.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);
        ctx->parse_module_path(yin_file, LYS_IN_YIN);
        auto root = ctx->parse_data_path(config_file, LYD_XML, LYD_OPT_CONFIG | LYD_OPT_STRICT);
        ASSERT_NOTNULL(root);

        auto set = std::make_shared<libyang::Set>();
        ASSERT_NOTNULL(set);
        ASSERT_EQ(0, set->number());

        set->add(root->child()->schema());
        ASSERT_EQ(1, set->number());

        set->add(root->schema());
        ASSERT_EQ(2, set->number());

        set->rm(root->schema());
        ASSERT_EQ(1, set->number());

        set->add(root->schema());
        ASSERT_EQ(2, set->number());

        set->rm_index(1);
        ASSERT_EQ(1, set->number());

        set->clean();
        ASSERT_EQ(0, set->number());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_new_ylpath)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *path = TESTS_DIR "/api/files/ylpath.xml";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder, path, LYD_XML, 0);
        ASSERT_FALSE(nullptr == ctx);
        auto list = std::make_shared<std::vector<std::string>>(ctx->get_searchdirs());
        ASSERT_FALSE(nullptr == list);
        ASSERT_EQ(1, list->size());

    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_new_ylmem)
{
    const char *yang_folder = TESTS_DIR "/api/files";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_FALSE(nullptr == ctx);
        auto info = ctx->info();
        ASSERT_FALSE(nullptr == info);
        ASSERT_EQ(LYD_VAL_OK, info->validity());

        auto mem = info->print_mem(LYD_XML, 0);
        auto new_ctx = std::make_shared<libyang::Context>(yang_folder, LYD_XML, mem.data(), 0);
        ASSERT_FALSE(nullptr == new_ctx);
        auto list = std::make_shared<std::vector<std::string>>(new_ctx->get_searchdirs());
        ASSERT_FALSE(nullptr == list);
        ASSERT_EQ(1, list->size());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_get_module_iter)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *module_name = "a";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        auto module = ctx->load_module(module_name);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name, module->name());
        auto list = std::make_shared<std::vector<libyang::S_Module>>(ctx->get_module_iter());
        ASSERT_NOTNULL(list);
        ASSERT_EQ(7, list->size());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_get_disabled_module_iter)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *module_name = "x";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        auto module = ctx->load_module(module_name);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name, module->name());
        // FIXME no way to disable module from here

        auto list = std::make_shared<std::vector<libyang::S_Module>>(ctx->get_disabled_module_iter());
        ASSERT_NOTNULL(list);
        ASSERT_EQ(0, list->size());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_data_instantiables)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *module_name = "b";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        auto module = ctx->load_module(module_name);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name, module->name());

        auto list = std::make_shared<std::vector<libyang::S_Schema_Node>>(ctx->data_instantiables(0));
        ASSERT_NOTNULL(list);
        ASSERT_EQ(5, list->size());
    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_ly_ctx_get_node)
{
    const char *yang_folder = TESTS_DIR "/api/files";
    const char *module_name = "b";

    try {
        auto ctx = std::make_shared<libyang::Context>(yang_folder);
        ASSERT_NOTNULL(ctx);

        auto module = ctx->load_module(module_name);
        ASSERT_NOTNULL(module);
        ASSERT_STREQ(module_name, module->name());

        auto list = std::make_shared<std::vector<libyang::S_Schema_Node>>(ctx->data_instantiables(0));
        ASSERT_NOTNULL(list);
        ASSERT_EQ(5, list->size());

        auto schema = list->back();
        ASSERT_NOTNULL(schema);
        auto node = ctx->get_node(schema, "/b:x/b:bubba", 0);
        ASSERT_NOTNULL(node);

    } catch( const std::exception& e ) {
        mt::printFailed(e.what(), stdout);
        throw;
    }
}

TEST(test_module_impl_callback)
{
    std::string mod_a{"module a {namespace urn:a; prefix a; import b { prefix b; } import c { prefix c; } import d { prefix d; } leaf a { type b:mytype; } leaf a1 { type c:mytype; } }"};
    std::string mod_b{"module b {namespace urn:b; prefix b; import b_1 { prefix b_1; } typedef mytype { type string; }}"};
    std::string mod_b_1{"module b_1 {namespace urn:b_1; prefix b_1; typedef mytype { type string; }}"};
    std::string mod_c{"module c {namespace urn:c; prefix c; typedef mytype { type string; }}"};
    std::string mod_d{"module d {namespace urn:d; prefix d; typedef mytype { type string; }}"};
    int b_allocated = 0, b_freed = 0, b_1_allocated = 0, b_1_freed = 0, c_allocated = 0, c_freed = 0, d_allocated = 0;
    auto mod_b_cb = [mod_b, &b_allocated](const char *mod_name, const char *, const char *, const char *) -> libyang::Context::mod_missing_cb_return {
        if (mod_name == std::string("b")) {
            ASSERT_EQ(0, b_allocated);
            ++b_allocated;
            return {LYS_IN_YANG, strdup(mod_b.c_str())};
        }
        return {LYS_IN_UNKNOWN, nullptr};
    };
    auto mod_b_free = [&b_allocated, &b_freed](void *data) {
        ASSERT_EQ(1, b_allocated);
        ++b_freed;
        free(data);
    };
    auto mod_b_1_cb = [mod_b_1, &b_1_allocated](const char *mod_name, const char *, const char *, const char *) -> libyang::Context::mod_missing_cb_return {
        if (mod_name == std::string("b_1")) {
            ASSERT_EQ(0, b_1_allocated);
            ++b_1_allocated;
            return {LYS_IN_YANG, strdup(mod_b_1.c_str())};
        }
        return {LYS_IN_UNKNOWN, nullptr};
    };
    auto mod_b_1_free = [&b_1_allocated, &b_1_freed](void *data) {
        ASSERT_EQ(1, b_1_allocated);
        ++b_1_freed;
        free(data);
    };
    auto mod_c_cb = [mod_c, &c_allocated](const char *mod_name, const char *, const char *, const char *) -> libyang::Context::mod_missing_cb_return {
        if (mod_name == std::string("c")) {
            ASSERT_EQ(0, c_allocated);
            ++c_allocated;
            // no actual allocation here
            return {LYS_IN_YANG, mod_c.c_str()};
        }
        return {LYS_IN_UNKNOWN, nullptr};
    };
    auto mod_c_free = [&c_allocated, &c_freed](void *) {
        ASSERT_EQ(1, c_allocated);
        ++c_freed;
        // no actual deallocation
    };
    auto mod_d_cb = [mod_d, &d_allocated](const char *mod_name, const char *, const char *, const char *) -> libyang::Context::mod_missing_cb_return {
        if (mod_name == std::string("d")) {
            ASSERT_EQ(0, d_allocated);
            ++d_allocated;
            // no allocation because we are testing behavior with no deleter
            return {LYS_IN_YANG, mod_d.c_str()};
        }
        return {LYS_IN_UNKNOWN, nullptr};
    };

    auto ctx = std::make_shared<libyang::Context>();
    ctx->add_missing_module_callback(mod_b_cb, mod_b_free);
    ctx->add_missing_module_callback(mod_b_1_cb, mod_b_1_free);
    ctx->add_missing_module_callback(mod_c_cb, mod_c_free);
    ctx->add_missing_module_callback(mod_d_cb);
    ctx->parse_module_mem(mod_a.c_str(), LYS_IN_YANG);
    ASSERT_EQ(1, b_allocated);
    ASSERT_EQ(1, b_freed);
    ASSERT_EQ(1, b_1_allocated);
    ASSERT_EQ(1, b_1_freed);
    ASSERT_EQ(1, c_allocated);
    ASSERT_EQ(1, c_freed);
    ASSERT_EQ(1, d_allocated);
}

TEST_MAIN();
