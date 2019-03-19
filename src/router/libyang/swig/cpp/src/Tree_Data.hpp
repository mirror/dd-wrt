/**
 * @file Tree_Data.hpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Class implementation for libyang C header tree_data.h.
 *
 * Copyright (c) 2017 Deutsche Telekom AG.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LIBYANG_CPP_TREE_DATA_H
#define LIBYANG_CPP_TREE_DATA_H

#include <iostream>
#include <memory>
#include <exception>
#include <vector>

#include "Internal.hpp"
#include "Tree_Schema.hpp"

extern "C" {
#include "libyang.h"
#include "tree_data.h"
}

namespace libyang {

/**
 * @defgroup classes C++/Python
 * @{
 *
 * Class wrappers for data structures and functions to manipulate and access instance data tree.
 */

/**
 * @brief class for wrapping [lyd_val](@ref lyd_val).
 * @class Value
 */
class Value
{
public:
    /** wrapper for struct [lyd_val](@ref lyd_val), for internal use only */
    Value(lyd_val value, LY_DATA_TYPE* value_type, uint8_t value_flags, S_Deleter deleter);
    ~Value();
    /** get binary variable from [lyd_val](@ref lyd_val)*/
    const char *binary() {return LY_TYPE_BINARY == type ? value.binary : throw "wrong type";};
    //struct lys_type_bit **bit();
    //TODO, check size
    //its size is always the number of defined bits in the schema
    /** get bln variable from [lyd_val](@ref lyd_val)*/
    int8_t bln() {return LY_TYPE_BOOL == type ? value.bln : throw "wrong type";};
    /** get dec64 variable from [lyd_val](@ref lyd_val)*/
    int64_t dec64() {return LY_TYPE_DEC64 == type ? value.dec64 : throw "wrong type";};
    /** get enm variable from [lyd_val](@ref lyd_val)*/
    S_Type_Enum enm() {return LY_TYPE_ENUM == type ? std::make_shared<Type_Enum>(value.enm, deleter) : throw "wrong type";};
    /** get ident variable from [lyd_val](@ref lyd_val)*/
    S_Ident ident() {return LY_TYPE_IDENT == type ? std::make_shared<Ident>(value.ident, deleter) : throw "wrong type";};
    /** get instance variable from [lyd_val](@ref lyd_val)*/
    S_Data_Node instance();
    /** get int8 variable from [lyd_val](@ref lyd_val)*/
    int8_t int8() {return LY_TYPE_INT8 == type ? value.int8 : throw "wrong type";};
    /** get int16 variable from [lyd_val](@ref lyd_val)*/
    int16_t int16() {return LY_TYPE_INT16 == type ? value.int16 : throw "wrong type";};
    /** get int32 variable from [lyd_val](@ref lyd_val)*/
    int32_t int32() {return LY_TYPE_INT32 == type ? value.int32 : throw "wrong type";};
    /** get int64 variable from [lyd_val](@ref lyd_val)*/
    int64_t int64() {return LY_TYPE_INT64 == type ? value.int64 : throw "wrong type";};
    /** get leafref variable from [lyd_val](@ref lyd_val)*/
    S_Data_Node leafref();
    /** get string variable from [lyd_val](@ref lyd_val)*/
    const char *string() {return LY_TYPE_STRING == type ? value.string : throw "wrong type";};
    /** get uint8 variable from [lyd_val](@ref lyd_val)*/
    uint8_t uint8() {return LY_TYPE_UINT8 == type ? value.uint8 : throw "wrong type";};
    /** get uint16 variable from [lyd_val](@ref lyd_val)*/
    uint16_t uint16() {return LY_TYPE_UINT16 == type ? value.uint16 : throw "wrong type";};
    /** get uint32 variable from [lyd_val](@ref lyd_val)*/
    uint32_t uintu32() {return LY_TYPE_UINT32 == type ? value.uint32 : throw "wrong type";};
    /** get uint64 variable from [lyd_val](@ref lyd_val)*/
    uint64_t uint64() {return LY_TYPE_UINT64 == type ? value.uint64 : throw "wrong type";};

private:
    lyd_val value;
    LY_DATA_TYPE type;
    uint8_t flags;
    S_Deleter deleter;
};

/**
 * @brief classes for wrapping [lyd_node](@ref lyd_node).
 * @class Data_Node
 */
class Data_Node
{
public:
    /** wrapper for struct [lyd_node](@ref lyd_node), for internal use only */
    Data_Node(struct lyd_node *node, S_Deleter deleter = nullptr);
    /** wrapper for [lyd_new](@ref lyd_new) */
    Data_Node(S_Data_Node parent, S_Module module, const char *name);
    /** wrapper for [lyd_new_leaf](@ref lyd_new_leaf) */
    Data_Node(S_Data_Node parent, S_Module module, const char *name, const char *val_str);
    /** wrapper for [lyd_new_anydata](@ref lyd_new_anydata) */
    Data_Node(S_Data_Node parent, S_Module module, const char *name, const char *value, LYD_ANYDATA_VALUETYPE value_type);
    /** wrapper for [lyd_new_anydata](@ref lyd_new_anydata) */
    Data_Node(S_Data_Node parent, S_Module module, const char *name, S_Data_Node value);
    /** wrapper for [lyd_new_anydata](@ref lyd_new_anydata) */
    Data_Node(S_Data_Node parent, S_Module module, const char *name, S_Xml_Elem value);
    /** wrapper for [lyd_new_path](@ref lyd_new_path) */
    Data_Node(S_Context context, const char *path, const char *value, LYD_ANYDATA_VALUETYPE value_type, int options);
    /** wrapper for [lyd_new_path](@ref lyd_new_path) */
    Data_Node(S_Context context, const char *path, S_Data_Node value, int options);
    /** wrapper for [lyd_new_path](@ref lyd_new_path) */
    Data_Node(S_Context context, const char *path, S_Xml_Elem value, int options);
    //TODO
    //struct lyd_node *lyd_new_output(struct lyd_node *parent, const struct lys_module *module, const char *name);
    //struct lyd_node *lyd_new_output_leaf(struct lyd_node *parent, const struct lys_module *module, const char *name,
    //                                     const char *val_str);
    //struct lyd_node *lyd_new_output_leaf(struct lyd_node *parent, const struct lys_module *module, const char *name,
    //                                     void *value, LYD_ANYDATA_VALUETYPE value_type);
    ~Data_Node();
    /** get schema variable from [lyd_node](@ref lyd_node)*/
    S_Schema_Node schema() LY_NEW(node, schema, Schema_Node);
    /** get validity variable from [lyd_node](@ref lyd_node)*/
    uint8_t validity() {return node->validity;};
    /** get dflt variable from [lyd_node](@ref lyd_node)*/
    uint8_t dflt() {return node->dflt;};
    /** get when_status variable from [lyd_node](@ref lyd_node)*/
    uint8_t when_status() {return node->when_status;};
    /** get attr variable from [lyd_node](@ref lyd_node)*/
    S_Attr attr();
    /** get next variable from [lyd_node](@ref lyd_node)*/
    S_Data_Node next() LY_NEW(node, next, Data_Node);
    /** get prev variable from [lyd_node](@ref lyd_node)*/
    S_Data_Node prev() LY_NEW(node, prev, Data_Node);
    /** get parent variable from [lyd_node](@ref lyd_node)*/
    S_Data_Node parent() LY_NEW(node, parent, Data_Node);
    /** get child variable from [lyd_node](@ref lyd_node)*/
    virtual S_Data_Node child() LY_NEW(node, child, Data_Node);

    /* functions */
    /** wrapper for [lyd_path](@ref lyd_path) */
    std::string path();
    /** wrapper for [lyd_dup](@ref lyd_dup) */
    S_Data_Node dup(int recursive);
    /** wrapper for [lyd_dup_withsiblings](@ref lyd_dup_withsiblings) */
    S_Data_Node dup_withsiblings(int recursive);
    /** wrapper for [lyd_dup_to_ctx](@ref lyd_dup_to_ctx) */
    S_Data_Node dup_to_ctx(int recursive, S_Context context);
    /** wrapper for [lyd_merge](@ref lyd_merge) */
    int merge(S_Data_Node source, int options);
    /** wrapper for [lyd_merge_to_ctx](@ref lyd_merge_to_ctx) */
    int merge_to_ctx(S_Data_Node source, int options, S_Context context);
    /** wrapper for [lyd_insert](@ref lyd_insert) */
    int insert(S_Data_Node new_node);
    /** wrapper for [lyd_insert_sibling](@ref lyd_insert_sibling) */
    int insert_sibling(S_Data_Node new_node);
    /** wrapper for [lyd_insert_before](@ref lyd_insert_before) */
    int insert_before(S_Data_Node new_node);
    /** wrapper for [lyd_insert_after](@ref lyd_insert_after) */
    int insert_after(S_Data_Node new_node);
    /** wrapper for [lyd_schema_sort](@ref lyd_schema_sort) */
    int schema_sort(int recursive);
    /** wrapper for [lyd_find_path](@ref lyd_find_path) */
    S_Set find_path(const char *expr);
    /** wrapper for [lyd_find_instance](@ref lyd_find_instance) */
    S_Set find_instance(S_Schema_Node schema);
    /** wrapper for [lyd_first_sibling](@ref lyd_first_sibling) */
    S_Data_Node first_sibling();
    /** wrapper for [lyd_validate](@ref lyd_validate) */
    int validate(int options, S_Context var_arg);
    /** wrapper for [lyd_validate](@ref lyd_validate) */
    int validate(int options, S_Data_Node var_arg);
    /** wrapper for [lyd_validate_value](@ref lyd_validate_value) */
    int validate_value(const char *value);
    /** wrapper for [lyd_diff](@ref lyd_diff) */
    S_Difflist diff(S_Data_Node second, int options);
    /** wrapper for [lyd_new_path](@ref lyd_new_path) */
    S_Data_Node new_path(S_Context ctx, const char *path, const char *value, LYD_ANYDATA_VALUETYPE value_type, int options);
    /** wrapper for [lyd_new_path](@ref lyd_new_path) */
    S_Data_Node new_path(S_Context ctx, const char *path, S_Data_Node value, int options);
    /** wrapper for [lyd_new_path](@ref lyd_new_path) */
    S_Data_Node new_path(S_Context ctx, const char *path, S_Xml_Elem value, int options);
    /** wrapper for [lyd_list_pos](@ref lyd_list_pos) */
    unsigned int list_pos();
    /** wrapper for [lyd_unlink](@ref lyd_unlink) */
    int unlink();
    /** wrapper for [lyd_insert_attr](@ref lyd_insert_attr) */
    S_Attr insert_attr(S_Module module, const char *name, const char *value);
    /** wrapper for [lyd_node_module](@ref lyd_node_module) */
    S_Module node_module();
    /** wrapper for [lyd_print_mem](@ref lyd_print_mem) */
    std::string print_mem(LYD_FORMAT format, int options);

    /* emulate TREE macro's */
    /** wrapper for macro [LY_TREE_FOR](@ref LY_TREE_FOR) */
    std::vector<S_Data_Node> tree_for();
    /** wrapper for macro [LY_TREE_DFS_BEGIN](@ref LY_TREE_DFS_BEGIN) and [LY_TREE_DFS_END](@ref LY_TREE_DFS_END) */
    std::vector<S_Data_Node> tree_dfs();

    /** SWIG related wrappers, for internal use only */
    struct lyd_node *swig_node() {return node;};
    /** SWIG related wrappers, for internal use only */
    S_Deleter swig_deleter() {return deleter;};

    friend Set;
    friend Data_Node_Anydata;
    friend Data_Node_Leaf_List;

    /** libnetconf2 related wrappers, for internal use only */
    struct lyd_node *C_lyd_node() {return node;};

private:
    struct lyd_node *node;
    S_Deleter deleter;
};

S_Data_Node create_new_Data_Node(struct lyd_node *node);

/**
 * @brief class for wrapping [lyd_node_leaf_list](@ref lyd_node_leaf_list).
 * @class Data_Node_Leaf_List
 */
class Data_Node_Leaf_List : public Data_Node
{
public:
    /** wrapper for [Data_Node_Leaf_List](@ref Data_Node_Leaf_List) */
    Data_Node_Leaf_List(S_Data_Node derived);
    /** wrapper for struct [lyd_node](@ref lyd_node), for internal use only */
    Data_Node_Leaf_List(struct lyd_node *node, S_Deleter deleter = nullptr);
    ~Data_Node_Leaf_List();
    /** get value_str variable from [lyd_node_leaf_list](@ref lyd_node_leaf_list)*/
    const char *value_str() {return ((struct lyd_node_leaf_list *) node)->value_str;};
    /** get value variable from [lyd_node_leaf_list](@ref lyd_node_leaf_list)*/
    S_Value value();
    /** get value_type variable from [lyd_node_leaf_list](@ref lyd_node_leaf_list)*/
    uint16_t value_type() {return ((struct lyd_node_leaf_list *) node)->value_type;};
    /** get child variable from [lyd_node_leaf_list](@ref lyd_node_leaf_list)*/
    S_Data_Node child() {return nullptr;};

    /* functions */
    /** wrapper for [lyd_change_leaf](@ref lyd_change_leaf) */
    int change_leaf(const char *val_str);
    /** wrapper for [lyd_wd_default](@ref lyd_wd_default) */
    int wd_default();
    /** wrapper for [lyd_leaf_type](@ref lyd_leaf_type) */
    S_Type leaf_type();

private:
    struct lyd_node *node;
    S_Deleter deleter;
};

/**
 * @brief class for wrapping [lyd_node_anydata](@ref lyd_node_anydata).
 * @class Data_Node_Anydata
 */
class Data_Node_Anydata : public Data_Node
{
public:
    /** wrapper for [Data_Node_Anydata](@ref Data_Node_Anydata) */
    Data_Node_Anydata(S_Data_Node derived);
    /** wrapper for struct [lyd_node](@ref lyd_node), for internal use only */
    Data_Node_Anydata(struct lyd_node *node, S_Deleter deleter = nullptr);
    ~Data_Node_Anydata();
    /** get value_type variable from [lyd_node_anydata](@ref lyd_node_anydata)*/
    LYD_ANYDATA_VALUETYPE value_type() {return ((struct lyd_node_anydata *) node)->value_type;};
    //union value
    /** get child variable from [lyd_node_anydata](@ref lyd_node_anydata)*/
    S_Data_Node child() {return nullptr;};

private:
    struct lyd_node *node;
    S_Deleter deleter;
};

/**
 * @brief class for wrapping [lyd_attr](@ref lyd_attr).
 * @class Attr
 */
class Attr
{
public:
    /** wrapper for struct [lyd_attr](@ref lyd_attr), for internal use only */
    Attr(struct lyd_attr *attr, S_Deleter deleter = nullptr);
    ~Attr();
    /** get parent variable from [lyd_attr](@ref lyd_attr)*/
    S_Data_Node parent() LY_NEW(attr, parent, Data_Node);
    /** get next variable from [lyd_attr](@ref lyd_attr)*/
    S_Attr next();
    //struct lys_ext_instance_complex *annotation
    /** get name variable from [lyd_attr](@ref lyd_attr)*/
    const char *name() {return attr->name;};
    /** get value_str variable from [lyd_attr](@ref lyd_attr)*/
    const char *value_str() {return attr->value_str;};
    /** get value variable from [lyd_attr](@ref lyd_attr)*/
    S_Value value();
    /** get value_type variable from [lyd_attr](@ref lyd_attr)*/
    uint16_t value_type() {return attr->value_type;};
private:
    struct lyd_attr *attr;
    S_Deleter deleter;
};

/**
 * @brief class for wrapping [lyd_difflist](@ref lyd_difflist).
 * @class Difflist
 */
class Difflist
{
public:
    /** wrapper for struct [lyd_difflist](@ref lyd_difflist), for internal use only */
    Difflist(struct lyd_difflist *diff, S_Deleter deleter);
    ~Difflist();
    /** get type variable from [lyd_difflist](@ref lyd_difflist)*/
    LYD_DIFFTYPE *type() {return diff->type;};
    /** get first variable from [lyd_difflist](@ref lyd_difflist)*/
    std::vector<S_Data_Node> first();
    /** get second variable from [lyd_difflist](@ref lyd_difflist)*/
    std::vector<S_Data_Node> second();

private:
    struct lyd_difflist *diff;
    S_Deleter deleter;
};

/**@} */

}

#endif
