/**
 * @file Libyang.hpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Class implementation for libyang C header libyang.h.
 *
 * Copyright (c) 2017 Deutsche Telekom AG.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LIBYANG_CPP_H
#define LIBYANG_CPP_H

#include <iostream>
#include <memory>
#include <exception>
#include <functional>
#include <vector>

#include "Internal.hpp"

extern "C" {
#include "libyang.h"
}

namespace libyang {

/**
 * @defgroup classes C++/Python
 * @{
 *
 * Class wrappers for data structures and functions to manipulate and access instance data tree.
 */

/**
 * @brief class for wrapping ly_ctx.
 * @class Context
 */
class Context
{
public:
    /** wrapper for struct ly_ctx, for internal use only */
    Context(struct ly_ctx *ctx, S_Deleter deleter);
    /** wrapper for [ly_ctx_new](@ref ly_ctx_new) */
    explicit Context(const char *search_dir = nullptr, int options = 0);
    /** wrapper for [ly_ctx_new_ylpath](@ref ly_ctx_new_ylpath) */
    Context(const char *search_dir, const char *path, LYD_FORMAT format, int options = 0);
    /** wrapper for [ly_ctx_new_ylmem](@ref ly_ctx_new_ylmem) */
    Context(const char *search_dir, LYD_FORMAT format, const char *data, int options = 0);
    ~Context();
    /** wrapper for [ly_ctx_set_searchdir](@ref ly_ctx_set_searchdir) */
    int set_searchdir(const char *search_dir);
    /** wrapper for [ly_ctx_unset_searchdirs](@ref ly_ctx_unset_searchdirs) */
    void unset_searchdirs(int idx) {return ly_ctx_unset_searchdirs(ctx, idx);};
    /** wrapper for [ly_ctx_get_searchdirs](@ref ly_ctx_get_searchdirs) */
    std::vector<std::string> get_searchdirs();
    /** wrapper for [ly_ctx_set_allimplemented](@ref ly_ctx_set_allimplemented) */
    void set_allimplemented() {return ly_ctx_set_allimplemented(ctx);};
    /** wrapper for [ly_ctx_unset_allimplemented](@ref ly_ctx_unset_allimplemented) */
    void unset_allimplemented() {return ly_ctx_unset_allimplemented(ctx);};
    /** wrapper for [ly_ctx_set_disable_searchdirs](@ref ly_ctx_set_disable_searchdirs) */
    void set_disable_searchdirs() {return ly_ctx_set_disable_searchdirs(ctx);};
    /** wrapper for [ly_ctx_unset_disable_searchdirs](@ref ly_ctx_unset_disable_searchdirs) */
    void unset_disable_searchdirs() {return ly_ctx_unset_disable_searchdirs(ctx);};
    /** wrapper for [ly_ctx_set_disable_searchdir_cwd](@ref ly_ctx_set_disable_searchdir_cwd) */
    void set_disable_searchdir_cwd() {return ly_ctx_set_disable_searchdir_cwd(ctx);};
    /** wrapper for [ly_ctx_unset_disable_searchdir_cwd](@ref ly_ctx_unset_disable_searchdir_cwd) */
    void unset_disable_searchdir_cwd() {return ly_ctx_unset_disable_searchdir_cwd(ctx);};
    /** wrapper for [ly_ctx_set_prefer_searchdirs](@ref ly_ctx_set_disable_searchdirs) */
    void set_prefer_searchdirs() {return ly_ctx_set_prefer_searchdirs(ctx);};
    /** wrapper for [ly_ctx_unset_prefer_searchdirs](@ref ly_ctx_unset_prefer_searchdirs) */
    void unset_prefer_searchdirs() {return ly_ctx_unset_prefer_searchdirs(ctx);};
    /** wrapper for [ly_ctx_info](@ref ly_ctx_info) */
    S_Data_Node info();
    /** wrapper for [ly_ctx_get_module_iter](@ref ly_ctx_get_module_iter) */
    std::vector<S_Module> get_module_iter();
    /** wrapper for [ly_ctx_get_disabled_module_iter](@ref ly_ctx_get_disabled_module_iter) */
    std::vector<S_Module> get_disabled_module_iter();
    /** wrapper for [ly_ctx_get_module](@ref ly_ctx_get_module) */
    S_Module get_module(const char *name, const char *revision = nullptr, int implemented = 0);
    /** wrapper for [ly_ctx_get_module_older](@ref ly_ctx_get_module_older) */
    S_Module get_module_older(S_Module module);
    /** wrapper for [ly_ctx_load_module](@ref ly_ctx_load_module) */
    S_Module load_module(const char *name, const char *revision = nullptr);
    /** wrapper for [ly_ctx_get_module_by_ns](@ref ly_ctx_get_module_by_ns) */
    S_Module get_module_by_ns(const char *ns, const char *revision = nullptr, int implemented = 0);
    /** wrapper for [ly_ctx_get_submodule](@ref ly_ctx_get_submodule) */
    S_Submodule get_submodule(const char *module, const char *revision = nullptr, const char *submodule = nullptr, const char *sub_revision = nullptr);
    /** wrapper for [ly_ctx_get_submodule2](@ref ly_ctx_get_submodule2) */
    S_Submodule get_submodule2(S_Module main_module, const char *submodule = nullptr);
    /** wrapper for [ly_ctx_get_node](@ref ly_ctx_get_node) */
    S_Schema_Node get_node(S_Schema_Node start, const char *data_path, int output = 0);
    /** wrapper for [lys_getnext](@ref lys_getnext) */
    std::vector<S_Schema_Node> data_instantiables(int options);
    /** wrapper for [ly_ctx_find_path](@ref ly_ctx_find_path) */
    S_Set find_path(const char *schema_path);
    /** wrapper for [ly_ctx_clean](@ref ly_ctx_clean) */
    void clean();

    struct mod_missing_cb_return {
        LYS_INFORMAT format;
        const char *data;
    };
    using mod_missing_cb_t = std::function<mod_missing_cb_return(const char *mod_name, const char *mod_rev, const char *submod_name, const char *sub_rev)>;
    using mod_missing_deleter_t = std::function<void(void *)>;
    /** @short Add a missing include or import module callback

    When libyang hits a missing module, the @arg callback will be called. If it can accommodate this request, it should return an
    appropriate mod_missing_cb_return struct. If it is needed to free the actual module "source code", then @arg deleter should be
    provided; it is then responsible for deallocation of the resulting mod_missing_cb_return::data.
    */
    void add_missing_module_callback(const mod_missing_cb_t &callback, const mod_missing_deleter_t &deleter = mod_missing_deleter_t());

    /* functions */
    /** wrapper for [lyd_parse_mem](@ref lyd_parse_mem) */
    S_Data_Node parse_data_mem(const char *data, LYD_FORMAT format, int options = 0);
    /** wrapper for [lyd_parse_fd](@ref lyd_parse_fd) */
    S_Data_Node parse_data_fd(int fd, LYD_FORMAT format, int options = 0);
    /** wrapper for [lyd_parse_path](@ref lyd_parse_path) */
    S_Data_Node parse_data_path(const char *path, LYD_FORMAT format, int options = 0);
    /** wrapper for [lyd_parse_xml](@ref lyd_parse_xml) */
    S_Data_Node parse_data_xml(S_Xml_Elem elem, int options = 0);
    /** wrapper for [lys_parse_mem](@ref lys_parse_mem) */
    S_Module parse_module_mem(const char *data, LYS_INFORMAT format);
    /** wrapper for [lys_parse_fd](@ref lys_parse_fd) */
    S_Module parse_module_fd(int fd, LYS_INFORMAT format);
    /** wrapper for [lys_parse_path](@ref lys_parse_path) */
    S_Module parse_module_path(const char *path, LYS_INFORMAT format);

    friend std::vector<S_Error> get_ly_errors(S_Context context);
    friend Data_Node;
    friend Deleter;
    friend Error;

    std::vector<std::pair<mod_missing_cb_t, mod_missing_deleter_t>> mod_missing_cb;
    std::vector<const mod_missing_deleter_t *> mod_missing_deleter;
    static const char* cpp_mod_missing_cb(const char *mod_name, const char *mod_rev, const char *submod_name, const char *sub_rev, void *user_data, LYS_INFORMAT *format, void(**free_module_data)(void *model_data, void *user_data));
    static void cpp_mod_missing_deleter(void *data, void *user_data);

    /* SWIG specific */
    struct ly_ctx *swig_ctx() {return ctx;};
    std::vector<void*> wrap_cb_l;
private:
    struct ly_ctx *ctx;
    S_Deleter deleter;
};

S_Context create_new_Context(struct ly_ctx *ctx);

/**
 * @brief class for wrapping [ly_err_item](@ref ly_err_item).
 * @class Error
 */
class Error
{
public:
    /** wrapper for [ly_err_item](@ref ly_err_item) */
    Error(struct ly_err_item *eitem);
    ~Error() {};
    /** get no variable from [ly_err_item](@ref ly_err_item) */
    LY_ERR err() throw() {return eitem->no;};
    /** get vecode variable from [ly_err_item](@ref ly_err_item)*/
    LY_VECODE vecode() throw() {return eitem->vecode;};
    /** get errmsg variable from [ly_err_item](@ref ly_err_item)*/
    const char *errmsg() const throw() {return eitem->msg ? eitem->msg : "";};
    /** get errpath variable from [ly_err_item](@ref ly_err_item)*/
    const char *errpath() const throw() {return eitem->path ? eitem->path : "";};
    /** get errapptag variable from [ly_err_item](@ref ly_err_item)*/
    const char *errapptag() const throw() {return eitem->apptag ? eitem->path : "";};
private:
	struct ly_err_item *eitem;
};

std::vector<S_Error> get_ly_errors(S_Context context);
int set_log_options(int options);
LY_LOG_LEVEL set_log_verbosity(LY_LOG_LEVEL level);

/**
 * @brief class for wrapping [ly_set](@ref ly_set).
 * @class Set
 */
class Set
{
public:
    /** wrapper for [ly_set_new](@ref ly_set_new) */
    Set(struct ly_set *set, S_Deleter);
    Set();
    ~Set();
    /** get size variable from [ly_set](@ref ly_set)*/
    unsigned int size() {return set->size;};
    /** get number variable from [ly_set](@ref ly_set)*/
    unsigned int number() {return set->number;};
    /** get d variable from [ly_set_set](@ref ly_set_set)*/
    std::vector<S_Data_Node> data();
    /** get s variable from [ly_set_set](@ref ly_set_set)*/
    std::vector<S_Schema_Node> schema();

    /* functions */
    /** wrapper for [ly_set_dup](@ref ly_set_dup) */
    S_Set dup();
    /** wrapper for [ly_set_add](@ref ly_set_add) */
    int add(S_Data_Node node, int options = 0);
    /** wrapper for [ly_set_add](@ref ly_set_add) */
    int add(S_Schema_Node node, int options = 0);
    /** wrapper for [ly_set_contains](@ref ly_set_contains) */
    int contains(S_Data_Node node);
    /** wrapper for [ly_set_contains](@ref ly_set_contains) */
    int contains(S_Schema_Node node);
    /** wrapper for [ly_set_clean](@ref ly_set_clean) */
    int clean();
    /** wrapper for [ly_set_rm](@ref ly_set_rm) */
    int rm(S_Data_Node node);
    /** wrapper for [ly_set_rm](@ref ly_set_rm) */
    int rm(S_Schema_Node node);
    /** wrapper for [ly_set_rm_index](@ref ly_set_rm_index) */
    int rm_index(unsigned int index);

private:
    struct ly_set *set;
    S_Deleter deleter;
};

/**@} */

}

#endif
