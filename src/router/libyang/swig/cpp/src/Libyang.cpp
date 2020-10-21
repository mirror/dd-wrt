/**
 * @file Libyang.cpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Implementation of header Libyang.hpp
 *
 * Copyright (c) 2017 Deutsche Telekom AG.
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

#include "Xml.hpp"
#include "Internal.hpp"
#include "Libyang.hpp"
#include "Tree_Data.hpp"
#include "Tree_Schema.hpp"

extern "C" {
#include "libyang.h"
#include "tree_data.h"
#include "tree_schema.h"
#include "context.h"
}

namespace libyang {

Context::Context(ly_ctx *ctx, S_Deleter deleter):
    ctx(ctx),
    deleter(deleter)
{};
Context::Context(const char *search_dir, int options) {
    ctx = ly_ctx_new(search_dir, options);
    if (!ctx) {
        check_libyang_error(nullptr);
    }
    deleter = std::make_shared<Deleter>(ctx);
}
Context::Context(const char *search_dir, const char *path, LYD_FORMAT format, int options) {
    ctx = ly_ctx_new_ylpath(search_dir, path, format, options);
    if (!ctx) {
        check_libyang_error(nullptr);
    }
    deleter = std::make_shared<Deleter>(ctx);
}
Context::Context(const char *search_dir, LYD_FORMAT format, const char *data, int options) {
    ctx = ly_ctx_new_ylmem(search_dir, data, format, options);
    if (!ctx) {
        check_libyang_error(nullptr);
    }
    deleter = std::make_shared<Deleter>(ctx);
}
Context::~Context() {}
int Context::set_searchdir(const char *search_dir) {
    int ret = ly_ctx_set_searchdir(ctx, search_dir);
    if (ret) {
        check_libyang_error(ctx);
    }
    return ret;
}
S_Data_Node Context::info() {
    struct lyd_node *new_node = ly_ctx_info(ctx);
    if (!new_node) {
        check_libyang_error(ctx);
        return nullptr;
    }

    S_Deleter new_deleter = std::make_shared<Deleter>(new_node, deleter);
    return std::make_shared<Data_Node>(new_node, new_deleter);
}
S_Module Context::get_module(const char *name, const char *revision, int implemented) {
    const struct lys_module *module = ly_ctx_get_module(ctx, name, revision, implemented);
    return module ? std::make_shared<Module>((lys_module *) module, deleter) : nullptr;
}
S_Module Context::get_module_older(S_Module module) {
    const struct lys_module *new_module = ly_ctx_get_module_older(ctx, module->module);
    return new_module ? std::make_shared<Module>((lys_module *) new_module, deleter) : nullptr;
}
S_Module Context::load_module(const char *name, const char *revision) {
    const struct lys_module *module = ly_ctx_load_module(ctx, name, revision);
    if (!module) {
        check_libyang_error(ctx);
    }
    return module ? std::make_shared<Module>((lys_module *) module, deleter) : nullptr;
}
S_Module Context::get_module_by_ns(const char *ns, const char *revision, int implemented) {
    const struct lys_module *module = ly_ctx_get_module_by_ns(ctx, ns, revision, implemented);
    return module ? std::make_shared<Module>((lys_module *) module, deleter) : nullptr;
}
std::vector<S_Module> Context::get_module_iter() {
    const struct lys_module *mod = nullptr;
    uint32_t i = 0;

    std::vector<S_Module> s_vector;

    while ((mod = ly_ctx_get_module_iter(ctx, &i))) {
        if (mod == nullptr) {
            break;
        }
        s_vector.push_back(std::make_shared<Module>((lys_module *) mod, deleter));
    }

    return s_vector;
}
std::vector<S_Module> Context::get_disabled_module_iter() {
    const struct lys_module *mod = nullptr;
    uint32_t i = 0;

    std::vector<S_Module> s_vector;

    while ((mod = ly_ctx_get_disabled_module_iter(ctx, &i))) {
        if (mod == nullptr) {
            break;
        }
        s_vector.push_back(std::make_shared<Module>((lys_module *) mod, deleter));
    }

    return s_vector;
}
void Context::clean() {
    return ly_ctx_clean(ctx, nullptr);
}
std::vector<std::string> Context::get_searchdirs() {
    std::vector<std::string> s_vector;
    const char * const *data = ly_ctx_get_searchdirs(ctx);
    if (!data) {
        return s_vector;
    }

    int size = 0;
    while (true) {
        if (data[size] == nullptr) {
            break;
        }
        s_vector.push_back(std::string(data[size]));
        size++;
    }

    return s_vector;
};
S_Submodule Context::get_submodule(const char *module, const char *revision, const char *submodule, const char *sub_revision) {
    const struct lys_submodule *tmp_submodule = nullptr;

    tmp_submodule = ly_ctx_get_submodule(ctx, module, revision, submodule, sub_revision);

    return tmp_submodule ? std::make_shared<Submodule>((struct lys_submodule *) tmp_submodule, deleter) : nullptr;
}
S_Submodule Context::get_submodule2(S_Module main_module, const char *submodule) {
    const struct lys_submodule *tmp_submodule = nullptr;

    tmp_submodule = ly_ctx_get_submodule2(main_module->module, submodule);

    return tmp_submodule ? std::make_shared<Submodule>((struct lys_submodule *) tmp_submodule, deleter) : nullptr;
}
S_Schema_Node Context::get_node(S_Schema_Node start, const char *data_path, int output) {
    const struct lys_node *node = nullptr;

    node = ly_ctx_get_node(ctx, start ? start->node : NULL, data_path, output);

    return node ? std::make_shared<Schema_Node>((struct lys_node *) node, deleter) : nullptr;
}
S_Set Context::find_path(const char *schema_path) {
    struct ly_set *set = ly_ctx_find_path(ctx, schema_path);
    if (!set) {
        return nullptr;
    }

    S_Deleter new_deleter = std::make_shared<Deleter>(set, deleter);
    return std::make_shared<Set>(set, new_deleter);
}
std::vector<S_Schema_Node> Context::data_instantiables(int options) {
    std::vector<S_Schema_Node> s_vector;
    struct lys_node *iter = NULL;
    int i;

    for (i = 0; i < ctx->models.used; i++) {
        while ((iter = (struct lys_node *)lys_getnext(iter, NULL, ctx->models.list[i], options))) {
            s_vector.push_back(std::make_shared<Schema_Node>(iter, deleter));
        }
    }

    return s_vector;
}
S_Data_Node Context::parse_data_mem(const char *data, LYD_FORMAT format, int options) {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_parse_mem(ctx, data, format, options, NULL);
    if (!new_node) {
        check_libyang_error(ctx);
        return nullptr;
    }

    S_Deleter new_deleter = std::make_shared<Deleter>(new_node, deleter);
    return std::make_shared<Data_Node>(new_node, new_deleter);
}
S_Data_Node Context::parse_data_fd(int fd, LYD_FORMAT format, int options) {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_parse_fd(ctx, fd, format, options, NULL);
    if (!new_node) {
        check_libyang_error(ctx);
        return nullptr;
    }

    S_Deleter new_deleter = std::make_shared<Deleter>(new_node, deleter);
    return std::make_shared<Data_Node>(new_node, new_deleter);
}

S_Module Context::parse_module_mem(const char *data, LYS_INFORMAT format) {
    struct lys_module *module = nullptr;

    module = (struct lys_module *) lys_parse_mem(ctx, data, format);
    if (!module) {
        check_libyang_error(ctx);
        return nullptr;
    }

    S_Deleter new_deleter = std::make_shared<Deleter>(module, deleter);
    return std::make_shared<Module>(module, new_deleter);
}
S_Module Context::parse_module_fd(int fd, LYS_INFORMAT format) {
    struct lys_module *module = nullptr;

    module = (struct lys_module *) lys_parse_fd(ctx, fd, format);
    if (!module) {
        check_libyang_error(ctx);
        return nullptr;
    }

    S_Deleter new_deleter = std::make_shared<Deleter>(module, deleter);
    return std::make_shared<Module>(module, new_deleter);
}
S_Module Context::parse_module_path(const char *path, LYS_INFORMAT format) {
    struct lys_module *module = nullptr;

    module = (struct lys_module *) lys_parse_path(ctx, path, format);
    if (!module) {
        check_libyang_error(ctx);
        return nullptr;
    }

    S_Deleter new_deleter = std::make_shared<Deleter>(module, deleter);
    return std::make_shared<Module>(module, new_deleter);
}
S_Data_Node Context::parse_data_path(const char *path, LYD_FORMAT format, int options) {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_parse_path(ctx, path, format, options, NULL);
    if (!new_node) {
        check_libyang_error(ctx);
        return nullptr;
    }

    S_Deleter new_deleter = std::make_shared<Deleter>(new_node, deleter);
    return std::make_shared<Data_Node>(new_node, new_deleter);
}
S_Data_Node Context::parse_data_xml(S_Xml_Elem elem, int options) {
    struct lyd_node *new_node = nullptr;

    if (!elem) {
        throw std::invalid_argument("Elem can not be empty");
    }

    new_node = lyd_parse_xml(ctx, &elem->elem, options, NULL);
    if (!new_node) {
        check_libyang_error(ctx);
        return nullptr;
    }

    S_Deleter new_deleter = std::make_shared<Deleter>(new_node, deleter);
    return std::make_shared<Data_Node>(new_node, new_deleter);
}

void Context::add_missing_module_callback(const mod_missing_cb_t &callback, const mod_missing_deleter_t &deleter)
{
    if (mod_missing_cb.empty()) {
        ly_ctx_set_module_imp_clb(ctx, Context::cpp_mod_missing_cb, this);
    }
    mod_missing_cb.emplace_back(std::move(callback), std::move(deleter));
}

const char* Context::cpp_mod_missing_cb(const char *mod_name, const char *mod_rev, const char *submod_name, const char *sub_rev, void *user_data, LYS_INFORMAT *format, void (**free_module_data)(void*, void*))
{
    Context *ctx = static_cast<Context*>(user_data);
    for (const auto &x : ctx->mod_missing_cb) {
        const auto &cb = x.first;
        auto ret = cb(mod_name, mod_rev, submod_name, sub_rev);
        if (ret.data) {
            *format = ret.format;
            if (x.second) {
                ctx->mod_missing_deleter.push_back(&x.second);
                *free_module_data = Context::cpp_mod_missing_deleter;
            }
            return ret.data;
        }
        if (ly_errno != LY_SUCCESS) {
            // The C API docs say that we should not try any more callbacks
            return nullptr;
        }
    }
    return nullptr;
}

void Context::cpp_mod_missing_deleter(void *data, void *user_data)
{
    Context *ctx = static_cast<Context*>(user_data);
    (*ctx->mod_missing_deleter.back())(data);
    ctx->mod_missing_deleter.pop_back();
}


Error::Error(struct ly_err_item *eitem):
	eitem(eitem)
{};

std::vector<S_Error> get_ly_errors(S_Context context)
{
    std::vector<S_Error> s_vector;
    if (!context) {
        return s_vector;
    }

    struct ly_err_item *first_eitem = ly_err_first(context->ctx);
    if (!first_eitem) {
        return s_vector;
    }

    struct ly_err_item *eitem = first_eitem;
    while (eitem) {
        s_vector.push_back(std::make_shared<Error>(eitem));
        eitem = eitem->next;
    }

    return s_vector;
}

int set_log_options(int options)
{
    return ly_log_options(options);
}

LY_LOG_LEVEL set_log_verbosity(LY_LOG_LEVEL level)
{
    return ly_verb(level);
}

Set::Set() {
    struct ly_set *set_new = ly_set_new();
    if (!set_new) {
        check_libyang_error(nullptr);
    }

    set = set_new;
    deleter = std::make_shared<Deleter>(set_new);
}
Set::Set(struct ly_set *set, S_Deleter deleter):
    set(set),
    deleter(deleter)
{};
Set::~Set() {}
std::vector<S_Data_Node> Set::data() {
    std::vector<S_Data_Node> s_vector;

    unsigned int i;
    for (i = 0; i < set->number; i++){
        s_vector.push_back(std::make_shared<Data_Node>(set->set.d[i], deleter));
    }

    return s_vector;
};
std::vector<S_Schema_Node> Set::schema() {
    std::vector<S_Schema_Node> s_vector;

    unsigned int i;
    for (i = 0; i < set->number; i++){
        s_vector.push_back(std::make_shared<Schema_Node>(set->set.s[i], deleter));
    }

    return s_vector;
};
S_Set Set::dup() {
    ly_set *new_set = ly_set_dup(set);
    if (!new_set) {
        return nullptr;
    }

    auto deleter = std::make_shared<Deleter>(new_set);
    return std::make_shared<Set>(new_set, deleter);
}
int Set::add(S_Data_Node node, int options) {
    if (!node) {
        throw std::invalid_argument("Node can not be empty");
    }
    return ly_set_add(set, (void *) node->node, options);
}
int Set::add(S_Schema_Node node, int options) {
    if (!node) {
        throw std::invalid_argument("Node can not be empty");
    }
    return ly_set_add(set, (void *) node->node, options);
}
int Set::contains(S_Data_Node node) {
    if (!node) {
        return -1;
    }
    return ly_set_contains(set, (void *) node->node);
}
int Set::contains(S_Schema_Node node) {
    if (!node) {
        return -1;
    }
    return ly_set_contains(set, (void *) node->node);
}
int Set::clean() {
    return ly_set_clean(set);
}
int Set::rm(S_Data_Node node) {
    if (!node) {
        throw std::invalid_argument("Node can not be empty");
    }
    return ly_set_rm(set, (void *) node->node);
}
int Set::rm(S_Schema_Node node) {
    if (!node) {
        throw std::invalid_argument("Node can not be empty");
    }
    return ly_set_rm(set, (void *) node->node);
}
int Set::rm_index(unsigned int index) {
    return ly_set_rm_index(set, index);
}

/* API for wrapping struct ly_ctx from libnetconf2 python bindings */
S_Context create_new_Context(struct ly_ctx *new_ctx) {
    return new_ctx ? std::make_shared<Context>(new_ctx, nullptr) : nullptr;
}

}
