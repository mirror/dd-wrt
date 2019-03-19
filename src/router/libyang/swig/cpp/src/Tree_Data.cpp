/**
 * @file Tree_Data.cpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Implementation of header Tree_Data.hpp.
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
#include "Libyang.hpp"
#include "Tree_Data.hpp"
#include "Tree_Schema.hpp"

extern "C" {
#include "libyang.h"
#include "tree_data.h"
#include "tree_schema.h"
}

namespace libyang {

Value::Value(lyd_val value, LY_DATA_TYPE* value_type, uint8_t value_flags, S_Deleter deleter):
    value(value),
    type(*value_type),
    flags(value_flags),
    deleter(deleter)
{};
Value::~Value() {};
S_Data_Node Value::instance() {
    if (LY_TYPE_INST != type) {
        return nullptr;
    }
    return value.instance ? std::make_shared<Data_Node>(value.instance, deleter) : nullptr;
}
S_Data_Node Value::leafref() {
    if (LY_TYPE_LEAFREF != type) {
        return nullptr;
    }
    return value.leafref ? std::make_shared<Data_Node>(value.leafref, deleter) : nullptr;
}

Data_Node::Data_Node(struct lyd_node *node, S_Deleter deleter):
    node(node),
    deleter(deleter)
{};
Data_Node::Data_Node(S_Data_Node parent, S_Module module, const char *name) {
    lyd_node *new_node = nullptr;

    if (!module) {
        throw std::invalid_argument("Module can not be empty");
    }

    new_node = lyd_new(parent ? parent->node : nullptr, module->module, name);
    if (!new_node) {
        check_libyang_error(module->module->ctx);
    }

    node = new_node;
    deleter = !parent ? std::make_shared<Deleter>(node, module->deleter) : parent->deleter;
};
Data_Node::Data_Node(S_Data_Node parent, S_Module module, const char *name, const char *val_str) {
    lyd_node *new_node = nullptr;

    if (!module) {
        throw std::invalid_argument("Module can not be empty");
    }

    new_node = lyd_new_leaf(parent ? parent->node : nullptr, module->module, name, val_str);
    if (!new_node) {
        check_libyang_error(module->module->ctx);
    }

    node = new_node;
    deleter = !parent ? std::make_shared<Deleter>(node, module->deleter) : parent->deleter;
};
Data_Node::Data_Node(S_Data_Node parent, S_Module module, const char *name, const char *value, LYD_ANYDATA_VALUETYPE value_type) {
    lyd_node *new_node = nullptr;

    if (!module) {
        throw std::invalid_argument("Module can not be empty");
    }

    new_node = lyd_new_anydata(parent ? parent->node : NULL, module->module, name, (void *) value, value_type);
    if (!new_node) {
        check_libyang_error(module->module->ctx);
    }

    node = new_node;
    deleter = !parent ? std::make_shared<Deleter>(node, module->deleter) : parent->deleter;
};
Data_Node::Data_Node(S_Data_Node parent, S_Module module, const char *name, S_Data_Node value) {
    lyd_node *new_node = nullptr;

    if (!module) {
        throw std::invalid_argument("Module can not be empty");
    }

    new_node = lyd_new_anydata(parent ? parent->node : NULL, module->module, name, (void *) value->node, LYD_ANYDATA_DATATREE);
    if (!new_node) {
        check_libyang_error(module->module->ctx);
    }

    node = new_node;
    deleter = !parent ? std::make_shared<Deleter>(node, module->deleter) : parent->deleter;
};
Data_Node::Data_Node(S_Data_Node parent, S_Module module, const char *name, S_Xml_Elem value) {
    lyd_node *new_node = nullptr;

    if (!module) {
        throw std::invalid_argument("Module can not be empty");
    }

    new_node = lyd_new_anydata(parent ? parent->node : NULL, module->module, name, (void *) value->elem, LYD_ANYDATA_XML);
    if (!new_node) {
        check_libyang_error(module->module->ctx);
    }

    node = new_node;
    deleter = !parent ? std::make_shared<Deleter>(node, module->deleter) : parent->deleter;
}
Data_Node::Data_Node(S_Context context, const char *path, const char *value, LYD_ANYDATA_VALUETYPE value_type, int options) {
    lyd_node *new_node = nullptr;

    if (!context) {
        throw std::invalid_argument("Context can not be empty");
    }
    if (!path) {
        throw std::invalid_argument("Path can not be empty");
    }

    new_node = lyd_new_path(NULL, context->ctx, path, (void *) value, value_type, options);
    if (!new_node) {
        check_libyang_error(context->ctx);
    }

    node = new_node;
    deleter = std::make_shared<Deleter>(node, context->deleter);
}
Data_Node::Data_Node(S_Context context, const char *path, S_Data_Node value, int options) {
    lyd_node *new_node = nullptr;

    if (!context) {
        throw std::invalid_argument("Context can not be empty");
    }
    if (!path) {
        throw std::invalid_argument("Path can not be empty");
    }

    new_node = lyd_new_path(NULL, context->ctx, path, (void *) value->node, LYD_ANYDATA_DATATREE, options);
    if (!new_node) {
        check_libyang_error(context->ctx);
    }

    node = new_node;
    deleter = context->deleter;
}
Data_Node::Data_Node(S_Context context, const char *path, S_Xml_Elem value, int options) {
    lyd_node *new_node = nullptr;

    if (!context) {
        throw std::invalid_argument("Context can not be empty");
    }
    if (!path) {
        throw std::invalid_argument("Path can not be empty");
    }

    new_node = lyd_new_path(NULL, context->ctx, path, (void *) value->elem, LYD_ANYDATA_XML, options);
    if (!new_node) {
        check_libyang_error(context->ctx);
    }

    node = new_node;
    deleter = context->deleter;
}

Data_Node::~Data_Node() {};
S_Attr Data_Node::attr() LY_NEW(node, attr, Attr);
std::string Data_Node::path() {
    char *path = nullptr;

    path = lyd_path(node);
    if (!path) {
        check_libyang_error(node->schema->module->ctx);
        return nullptr;
    }

    std::string s_path = path;
    free(path);
    return s_path;
}
S_Data_Node Data_Node::dup(int recursive) {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_dup(node, recursive);
    if (!new_node) {
        return nullptr;
    }

    S_Deleter new_deleter = std::make_shared<Deleter>(new_node, deleter);
    return std::make_shared<Data_Node>(new_node, new_deleter);
}
S_Data_Node Data_Node::dup_withsiblings(int recursive) {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_dup_withsiblings(node, recursive);
    if (!new_node) {
        return nullptr;
    }

    S_Deleter new_deleter = std::make_shared<Deleter>(new_node, deleter);
    return std::make_shared<Data_Node>(new_node, new_deleter);
}
S_Data_Node Data_Node::dup_to_ctx(int recursive, S_Context context) {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_dup_to_ctx(node, recursive, context->ctx);

    S_Deleter new_deleter = std::make_shared<Deleter>(new_node, context->deleter);
    return new_node ? std::make_shared<Data_Node>(new_node, new_deleter) : nullptr;
}
int Data_Node::merge(S_Data_Node source, int options) {
    int ret = lyd_merge(node, source->node, options);
    if (ret) {
        check_libyang_error(source->node->schema->module->ctx);
    }
    return ret;
}
int Data_Node::merge_to_ctx(S_Data_Node source, int options, S_Context context) {
    int ret = lyd_merge_to_ctx(&node, source->node, options, context->ctx);
    if (ret) {
        check_libyang_error(context->ctx);
    }
    return ret;
}
int Data_Node::insert(S_Data_Node new_node) {
    int ret = lyd_insert(node, new_node->node);
    if (ret) {
        check_libyang_error(node->schema->module->ctx);
    }
    return ret;
}
int Data_Node::insert_sibling(S_Data_Node new_node) {
    /* because of memory handling in C++ the node is duplicated before insertion */
    struct lyd_node *dup_node = lyd_dup(new_node->node, 1);
    if (!dup_node) {
        check_libyang_error(node->schema->module->ctx);
    }

    int ret = lyd_insert_sibling(&node, dup_node);
    if (ret) {
        check_libyang_error(node->schema->module->ctx);
    }
    return ret;
}
int Data_Node::insert_before(S_Data_Node new_node) {
    /* because of memory handling in C++ the node is duplicated before insertion */
    struct lyd_node *dup_node = lyd_dup(new_node->node, 1);
    if (!dup_node) {
        check_libyang_error(node->schema->module->ctx);
    }

    int ret = lyd_insert_before(node, dup_node);
    if (ret) {
        check_libyang_error(node->schema->module->ctx);
    }
    return ret;
}
int Data_Node::insert_after(S_Data_Node new_node) {
    /* because of memory handling in C++ the node is duplicated before insertion */
    struct lyd_node *dup_node = lyd_dup(new_node->node, 1);
    if (!dup_node) {
        check_libyang_error(node->schema->module->ctx);
    }

    int ret = lyd_insert_after(node, dup_node);
    if (ret) {
        check_libyang_error(node->schema->module->ctx);
    }
    return ret;
}
int Data_Node::schema_sort(int recursive) {
    int ret = lyd_schema_sort(node, recursive);
    if (ret) {
        check_libyang_error(node->schema->module->ctx);
    }
    return ret;
}
S_Set Data_Node::find_path(const char *expr) {
    struct ly_set *set = lyd_find_path(node, expr);
    if (!set) {
        check_libyang_error(node->schema->module->ctx);
    }

    return std::make_shared<Set>(set, std::make_shared<Deleter>(set, deleter));
}
S_Set Data_Node::find_instance(S_Schema_Node schema) {
    struct ly_set *set = lyd_find_instance(node, schema->node);
    if (!set) {
        check_libyang_error(node->schema->module->ctx);
    }

    return std::make_shared<Set>(set, std::make_shared<Deleter>(set, deleter));
}
S_Data_Node Data_Node::first_sibling() {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_first_sibling(node);

    return new_node ? std::make_shared<Data_Node>(new_node, deleter) : nullptr;
}
int Data_Node::validate(int options, S_Context var_arg) {
    int ret = lyd_validate(&node, options, (void *) var_arg->ctx);
    if (ret) {
        check_libyang_error(node ? node->schema->module->ctx : var_arg->ctx);
    }
    return ret;
}
int Data_Node::validate(int options, S_Data_Node var_arg) {
    int ret = lyd_validate(&node, options, (void *) var_arg->node);
    if (ret) {
        check_libyang_error(node->schema->module->ctx);
    }
    return ret;
}

int Data_Node::validate_value(const char *value) {
    int ret = lyd_validate_value(node->schema, value);
    if (ret != EXIT_SUCCESS) {
        check_libyang_error(node->schema->module->ctx);
    }
    return ret;
}
S_Difflist Data_Node::diff(S_Data_Node second, int options) {
    struct lyd_difflist *diff;

    diff = lyd_diff(node, second->node, options);
    if (!diff) {
        check_libyang_error(node->schema->module->ctx);
    }

    return diff ? std::make_shared<Difflist>(diff, deleter) : nullptr;
}
S_Data_Node Data_Node::new_path(S_Context ctx, const char *path, const char *value, LYD_ANYDATA_VALUETYPE value_type, int options) {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_new_path(node, ctx->ctx, path, (void *)value, value_type, options);
    if (!new_node) {
        check_libyang_error(node->schema->module->ctx);
    }

    return new_node ? std::make_shared<Data_Node>(new_node, deleter) : nullptr;
}
S_Data_Node Data_Node::new_path(S_Context ctx, const char *path, S_Data_Node value, int options) {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_new_path(node, ctx->ctx, path, (void *)value->node, LYD_ANYDATA_DATATREE, options);
    if (!new_node) {
        check_libyang_error(node->schema->module->ctx);
    }

    return new_node ? std::make_shared<Data_Node>(new_node, deleter) : nullptr;
}
S_Data_Node Data_Node::new_path(S_Context ctx, const char *path, S_Xml_Elem value, int options) {
    struct lyd_node *new_node = nullptr;

    new_node = lyd_new_path(node, ctx->ctx, path, (void *)value->elem, LYD_ANYDATA_XML, options);
    if (!new_node) {
        check_libyang_error(node->schema->module->ctx);
    }

    return new_node ? std::make_shared<Data_Node>(new_node, deleter) : nullptr;
}
unsigned int Data_Node::list_pos() {
    unsigned int ret = lyd_list_pos(node);
    if (!ret) {
        check_libyang_error(node->schema->module->ctx);
    }
    return ret;
}
int Data_Node::unlink() {
    int ret = lyd_unlink(node);
    if (ret) {
        check_libyang_error(node->schema->module->ctx);
    }

    /* change C++ memory handling after unlink */
    if (deleter) {
        deleter = std::make_shared<Deleter>(node, nullptr);
    }

    return ret;
}
S_Attr Data_Node::insert_attr(S_Module module, const char *name, const char *value) {
    struct lyd_attr *attr = nullptr;

    attr = lyd_insert_attr(node, module ? module->module : NULL, name, value);
    if (!attr) {
        check_libyang_error(node->schema->module->ctx);
    }

    return attr ? std::make_shared<Attr>(attr, deleter) : nullptr;
}
S_Module Data_Node::node_module() {
    struct lys_module *module = nullptr;

    module = lyd_node_module(node);
    if (!module) {
        check_libyang_error(node->schema->module->ctx);
    }

    return module ? std::make_shared<Module>(module, deleter) : nullptr;
}
std::string Data_Node::print_mem(LYD_FORMAT format, int options) {
    char *strp = nullptr;
    int rc = 0;

    rc = lyd_print_mem(&strp, node, format, options);
    if (rc) {
        check_libyang_error(node->schema->module->ctx);
        return nullptr;
    }

    std::string s_strp = strp;
    free(strp);
    return s_strp;

}
std::vector<S_Data_Node> Data_Node::tree_for() {
    std::vector<S_Data_Node> s_vector;

    struct lyd_node *elem = nullptr;
    LY_TREE_FOR(node, elem) {
        s_vector.push_back(std::make_shared<Data_Node>(elem, deleter));
    }

    return s_vector;
}
std::vector<S_Data_Node> Data_Node::tree_dfs() {
    std::vector<S_Data_Node> s_vector;

    struct lyd_node *elem = nullptr, *next = nullptr;
    LY_TREE_DFS_BEGIN(node, next, elem) {
        s_vector.push_back(std::make_shared<Data_Node>(elem, deleter));
        LY_TREE_DFS_END(node, next, elem)
    }

    return s_vector;
}

Data_Node_Leaf_List::Data_Node_Leaf_List(S_Data_Node derived):
    Data_Node(derived->node, derived->deleter),
    node(derived->node),
    deleter(derived->deleter)
{
    if (derived->node->schema->nodetype != LYS_LEAFLIST && derived->node->schema->nodetype != LYS_LEAF) {
        throw std::invalid_argument("Type must be LYS_LEAFLIST or LYS_LEAF");
    }
};
Data_Node_Leaf_List::Data_Node_Leaf_List(struct lyd_node *node, S_Deleter deleter):
    Data_Node(node, deleter),
    node(node),
    deleter(deleter)
{};
Data_Node_Leaf_List::~Data_Node_Leaf_List() {};
S_Value Data_Node_Leaf_List::value() {
    struct lyd_node_leaf_list *leaf = (struct lyd_node_leaf_list *) node;
    return std::make_shared<Value>(leaf->value, &leaf->value_type, leaf->value_flags, deleter);
}
int Data_Node_Leaf_List::change_leaf(const char *val_str) {
    int ret = lyd_change_leaf((struct lyd_node_leaf_list *) node, val_str);
    if (ret < 0) {
        check_libyang_error(node->schema->module->ctx);
    }
    return ret;
}
int Data_Node_Leaf_List::wd_default() {
    return lyd_wd_default((struct lyd_node_leaf_list *)node);
}
S_Type Data_Node_Leaf_List::leaf_type() {
    const struct lys_type *type = lyd_leaf_type((const struct lyd_node_leaf_list *) node);
    if (!type) {
        check_libyang_error(node->schema->module->ctx);
    }

    return std::make_shared<Type>((struct lys_type *) type, deleter);
};

Data_Node_Anydata::Data_Node_Anydata(S_Data_Node derived):
    Data_Node(derived->node, derived->deleter),
    node(derived->node),
    deleter(derived->deleter)
{
    if (derived->node->schema->nodetype != LYS_ANYDATA && derived->node->schema->nodetype != LYS_ANYXML) {
        throw std::invalid_argument("Type must be LYS_ANYDATA or LYS_ANYXML");
    }
};
Data_Node_Anydata::Data_Node_Anydata(struct lyd_node *node, S_Deleter deleter):
    Data_Node(node, deleter),
    node(node),
    deleter(deleter)
{};
Data_Node_Anydata::~Data_Node_Anydata() {};

Attr::Attr(struct lyd_attr *attr, S_Deleter deleter):
    attr(attr),
    deleter(deleter)
{};
Attr::~Attr() {};
S_Value Attr::value() {
    struct lyd_node_leaf_list *leaf = (struct lyd_node_leaf_list *) attr;
    return std::make_shared<Value>(leaf->value, &leaf->value_type, leaf->value_flags, deleter);
}
S_Attr Attr::next() LY_NEW(attr, next, Attr);

Difflist::Difflist(struct lyd_difflist *diff, S_Deleter deleter) {
    diff = diff;
    deleter = std::make_shared<Deleter>(diff, deleter);
}
Difflist::~Difflist() {};
std::vector<S_Data_Node> Difflist::first() {
    std::vector<S_Data_Node> s_vector;
    unsigned int i = 0;

    if (!*diff->first) {
        return s_vector;
    }

    for(i = 0; i < sizeof(*diff->first); i++) {
        s_vector.push_back(std::make_shared<Data_Node>(*diff->first, deleter));
    }

    return s_vector;
}
std::vector<S_Data_Node> Difflist::second() {
    std::vector<S_Data_Node> s_vector;
    unsigned int i = 0;

    if (!*diff->second) {
        return s_vector;
    }

    for(i = 0; i < sizeof(*diff->second); i++) {
        s_vector.push_back(std::make_shared<Data_Node>(*diff->second, deleter));
    }

    return s_vector;
}

S_Data_Node create_new_Data_Node(struct lyd_node *new_node) {
    return new_node ? std::make_shared<Data_Node>(new_node, nullptr) : nullptr;
}

}
