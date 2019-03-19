/**
 * @file Xml.hpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Class implementation for libyang C header xml.h.
 *
 * Copyright (c) 2017 Deutsche Telekom AG.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef XML_H
#define XML_H

#include <iostream>
#include <memory>
#include <exception>
#include <vector>

#include "Internal.hpp"

extern "C" {
#include "libyang.h"
#include "xml.h"
}

namespace libyang {

/**
 * @defgroup classes C++/Python
 * @{
 *
 * Class wrappers for data structures and functions to manipulate and access instance data tree.
 */

/**
 * @brief class for wrapping [lyxml_ns](@ref lyxml_ns).
 * @class Xml_Ns
 */
class Xml_Ns
{
public:
    /** wrapper for struct [lyxml_ns](@ref lyxml_ns), for internal use only */
    Xml_Ns(const struct lyxml_ns *ns, S_Deleter deleter);
    ~Xml_Ns();
    /** get type variable from [lyxml_ns](@ref lyxml_ns)*/
    LYXML_ATTR_TYPE type() {return ns->type;};
    /** get next variable from [lyxml_ns](@ref lyxml_ns)*/
    S_Xml_Ns next();
    //struct lyxml_elem *parent;       /**< parent node of the attribute */
    /** get prefix variable from [lyxml_ns](@ref lyxml_ns)*/
    const char *prefix() {return ns->prefix;};
    /** get value variable from [lyxml_ns](@ref lyxml_ns)*/
    const char *value() {return ns->value;};

private:
    struct lyxml_ns *ns;
    S_Deleter deleter;
};

class Xml_Attr
{
public:
    /** wrapper for struct [lyxml_attr](@ref lyxml_attr), for internal use only */
    Xml_Attr(struct lyxml_attr *attr, S_Deleter deleter);
    ~Xml_Attr();
    /** get type variable from [lyxml_attr](@ref lyxml_attr)*/
    LYXML_ATTR_TYPE type() {return attr->type;};
    /** get next variable from [lyxml_attr](@ref lyxml_attr)*/
    S_Xml_Attr next();
    /** get ns variable from [lyxml_attr](@ref lyxml_attr)*/
    S_Xml_Ns ns();
    /** get name variable from [lyxml_attr](@ref lyxml_attr)*/
    const char *name() {return attr->name;};
    /** get value variable from [lyxml_attr](@ref lyxml_attr)*/
    const char *value() {return attr->value;};

private:
    struct lyxml_attr *attr;
    S_Deleter deleter;
};

class Xml_Elem
{
public:
    /** wrapper for struct [lyxml_elem](@ref lyxml_elem), for internal use only */
    Xml_Elem(S_Context context, struct lyxml_elem *elem, S_Deleter deleter);
    ~Xml_Elem();
    /** get flags variable from [lyxml_elem](@ref lyxml_elem)*/
    char flags() {return elem->flags;};
    /** get parent variable from [lyxml_elem](@ref lyxml_elem)*/
    S_Xml_Elem parent();
    /** get attr variable from [lyxml_elem](@ref lyxml_elem)*/
    S_Xml_Attr attr();
    /** get child variable from [lyxml_elem](@ref lyxml_elem)*/
    S_Xml_Elem child();
    /** get next variable from [lyxml_elem](@ref lyxml_elem)*/
    S_Xml_Elem next();
    /** get prev variable from [lyxml_elem](@ref lyxml_elem)*/
    S_Xml_Elem prev();
    /** get name variable from [lyxml_elem](@ref lyxml_elem)*/
    const char *name() {return elem->name;};
    /** get ns variable from [lyxml_elem](@ref lyxml_elem)*/
    S_Xml_Ns ns();
    /** get content variable from [lyxml_elem](@ref lyxml_elem)*/
    const char *content() {return elem->content;};

    /* methods */
    /** wrapper for [lyxml_get_attr](@ref lyxml_get_attr) */
    const char *get_attr(const char *name, const char *ns = nullptr);
    /** wrapper for [lyxml_get_ns](@ref lyxml_get_ns) */
    S_Xml_Ns get_ns(const char *prefix);
    /** wrapper for [lyxml_print_mem](@ref lyxml_print_mem) */
    std::string print_mem(int options);
    //int lyxml_print_fd(int fd, const struct lyxml_elem *elem, int options);
    //int lyxml_print_file(FILE * stream, const struct lyxml_elem *elem, int options);

    /* emulate TREE macro's */
    /** wrapper for macro [LY_TREE_FOR](@ref LY_TREE_FOR) */
    std::vector<S_Xml_Elem> tree_for();
    /** wrapper for macro [LY_TREE_DFS_BEGIN](@ref LY_TREE_DFS_BEGIN) and [LY_TREE_DFS_END](@ref LY_TREE_DFS_END) */
    std::vector<S_Xml_Elem> tree_dfs();

    /* TODO
    struct lyxml_elem *lyxml_dup(struct ly_ctx *ctx, struct lyxml_elem *root);
    struct lyxml_elem *lyxml_parse_mem(struct ly_ctx *ctx, const char *data, int options);
    struct lyxml_elem *lyxml_parse_path(struct ly_ctx *ctx, const char *filename, int options);
    */

    friend Data_Node;
    friend Context;

private:
    S_Context context;
    struct lyxml_elem *elem;
    S_Deleter deleter;
};

/**@} */

}

#endif
