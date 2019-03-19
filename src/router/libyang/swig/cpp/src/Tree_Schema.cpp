/**
 * @file Tree_Schema.cpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Implementation of header Tree_Schema.hpp.
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

#include "Internal.hpp"
#include "Libyang.hpp"
#include "Tree_Schema.hpp"

extern "C" {
#include "libyang.h"
#include "tree_schema.h"
}

namespace libyang {

Module::Module(struct lys_module *module, S_Deleter deleter):
    module(module),
    deleter(deleter)
{};
Module::~Module() {};
S_Revision Module::rev() LY_NEW(module, rev, Revision);
std::vector<S_Deviation> Module::deviation() LY_NEW_LIST(module, deviation, deviation_size, Deviation);
Submodule::Submodule(struct lys_submodule *submodule, S_Deleter deleter):
    submodule(submodule),
    deleter(deleter)
{};
std::vector<S_Schema_Node> Module::data_instantiables(int options) {
    std::vector<S_Schema_Node> s_vector;
    struct lys_node *iter = NULL;

    while ((iter = (struct lys_node *)lys_getnext(iter, NULL, module, options))) {
        s_vector.push_back(std::make_shared<Schema_Node>(iter, deleter));
    }

    return s_vector;
}
std::string Module::print_mem(LYS_OUTFORMAT format, int options) {
    char *strp = nullptr;
    int rc = 0;

    rc = lys_print_mem(&strp, module, format, NULL, 0, options);
    if (rc) {
        check_libyang_error(module->ctx);
        return nullptr;
    }

    std::string s_strp = strp;
    free(strp);
    return s_strp;
}
std::string Module::print_mem(LYS_OUTFORMAT format, const char *target, int options) {
    char *strp = nullptr;
    int rc = 0;

    rc = lys_print_mem(&strp, module, format, target, 0, options);
    if (rc) {
        check_libyang_error(module->ctx);
        return nullptr;
    }

    std::string s_strp = strp;
    free(strp);
    return s_strp;
}
Submodule::~Submodule() {};
S_Revision Submodule::rev() LY_NEW(submodule, rev, Revision);
std::vector<S_Deviation> Submodule::deviation() LY_NEW_LIST(submodule, deviation, deviation_size, Deviation);

Type_Info_Binary::Type_Info_Binary(struct lys_type_info_binary *info_binary, S_Deleter deleter):
    info_binary(info_binary),
    deleter(deleter)
{};
Type_Info_Binary::~Type_Info_Binary() {};
S_Restr Type_Info_Binary::length() {return info_binary->length ? std::make_shared<Restr>(info_binary->length, deleter) : nullptr;};

Type_Bit::Type_Bit(struct lys_type_bit *info_bit, S_Deleter deleter):
    info_bit(info_bit),
    deleter(deleter)
{};
Type_Bit::~Type_Bit() {};
std::vector<S_Ext_Instance> Type_Bit::ext() LY_NEW_P_LIST(info_bit, ext, ext_size, Ext_Instance);
std::vector<S_Iffeature> Type_Bit::iffeature() LY_NEW_LIST(info_bit, iffeature, iffeature_size, Iffeature);

Type_Info_Bits::Type_Info_Bits(struct lys_type_info_bits *info_bits, S_Deleter deleter):
    info_bits(info_bits),
    deleter(deleter)
{};
Type_Info_Bits::~Type_Info_Bits() {};
std::vector<S_Type_Bit> Type_Info_Bits::bit() LY_NEW_LIST(info_bits, bit, count, Type_Bit);

Type_Info_Dec64::Type_Info_Dec64(struct lys_type_info_dec64 *info_dec64, S_Deleter deleter):
    info_dec64(info_dec64),
    deleter(deleter)
{};
Type_Info_Dec64::~Type_Info_Dec64() {};
S_Restr Type_Info_Dec64::range() {return info_dec64->range ? std::make_shared<Restr>(info_dec64->range, deleter) : nullptr;};

Type_Enum::Type_Enum(struct lys_type_enum *info_enum, S_Deleter deleter):
    info_enum(info_enum),
    deleter(deleter)
{};
Type_Enum::~Type_Enum() {};
std::vector<S_Ext_Instance> Type_Enum::ext() LY_NEW_P_LIST(info_enum, ext, ext_size, Ext_Instance);
std::vector<S_Iffeature> Type_Enum::iffeature() LY_NEW_LIST(info_enum, iffeature, iffeature_size, Iffeature);

Type_Info_Enums::Type_Info_Enums(struct lys_type_info_enums *info_enums, S_Deleter deleter):
    info_enums(info_enums),
    deleter(deleter)
{};
Type_Info_Enums::~Type_Info_Enums() {};
std::vector<S_Type_Enum> Type_Info_Enums::enm() LY_NEW_LIST(info_enums, enm, count, Type_Enum);

Type_Info_Ident::Type_Info_Ident(struct lys_type_info_ident *info_ident, S_Deleter deleter):
    info_ident(info_ident),
    deleter(deleter)
{};
Type_Info_Ident::~Type_Info_Ident() {};
std::vector<S_Ident> Type_Info_Ident::ref() LY_NEW_P_LIST(info_ident, ref, count, Ident);

Type_Info_Inst::Type_Info_Inst(struct lys_type_info_inst *info_inst, S_Deleter deleter):
    info_inst(info_inst),
    deleter(deleter)
{};
Type_Info_Inst::~Type_Info_Inst() {};

Type_Info_Num::Type_Info_Num(struct lys_type_info_num *info_num, S_Deleter deleter):
    info_num(info_num),
    deleter(deleter)
{};
Type_Info_Num::~Type_Info_Num() {};
S_Restr Type_Info_Num::range() {return info_num->range ? std::make_shared<Restr>(info_num->range, deleter) : nullptr;};

Type_Info_Lref::Type_Info_Lref(lys_type_info_lref *info_lref, S_Deleter deleter):
    info_lref(info_lref),
    deleter(deleter)
{};
Type_Info_Lref::~Type_Info_Lref() {};
S_Schema_Node_Leaf Type_Info_Lref::target() {return info_lref->target ? std::make_shared<Schema_Node_Leaf>((struct lys_node *)info_lref->target, deleter) : nullptr;};

Type_Info_Str::Type_Info_Str(lys_type_info_str *info_str, S_Deleter deleter):
    info_str(info_str),
    deleter(deleter)
{};
Type_Info_Str::~Type_Info_Str() {};
S_Restr Type_Info_Str::length() {return info_str->length ? std::make_shared<Restr>(info_str->length, deleter) : nullptr;};
S_Restr Type_Info_Str::patterns() {return info_str->patterns ? std::make_shared<Restr>(info_str->patterns, deleter) : nullptr;};

Type_Info_Union::Type_Info_Union(lys_type_info_union *info_union, S_Deleter deleter):
    info_union(info_union),
    deleter(deleter)
{};
Type_Info_Union::~Type_Info_Union() {};
std::vector<S_Type> Type_Info_Union::types() LY_NEW_LIST(info_union, types, count, Type);

Type_Info::Type_Info(union lys_type_info info, LY_DATA_TYPE *type, uint8_t flags, S_Deleter deleter):
    info(info),
    type(*type),
    flags(flags),
    deleter(deleter)
{};
Type_Info::~Type_Info() {};
S_Type_Info_Binary Type_Info::binary() {return LY_TYPE_BINARY == type ? std::make_shared<Type_Info_Binary>(&info.binary, deleter) : nullptr;};
S_Type_Info_Bits Type_Info::bits() {return LY_TYPE_BITS == type ? std::make_shared<Type_Info_Bits>(&info.bits, deleter) : nullptr;};
S_Type_Info_Dec64 Type_Info::dec64() {return LY_TYPE_DEC64 == type ? std::make_shared<Type_Info_Dec64>(&info.dec64, deleter) : nullptr;};
S_Type_Info_Enums Type_Info::enums() {return LY_TYPE_ENUM == type ? std::make_shared<Type_Info_Enums>(&info.enums, deleter) : nullptr;};
S_Type_Info_Ident Type_Info::ident() {return LY_TYPE_IDENT == type ? std::make_shared<Type_Info_Ident>(&info.ident, deleter) : nullptr;};
S_Type_Info_Inst Type_Info::inst() {return LY_TYPE_INST == type ? std::make_shared<Type_Info_Inst>(&info.inst, deleter) : nullptr;};
S_Type_Info_Num Type_Info::num() {
    if (type >= LY_TYPE_INT8 && type <= LY_TYPE_UINT64) {
        return std::make_shared<Type_Info_Num>(&info.num, deleter);
    } else {
        return nullptr;
    }
};
S_Type_Info_Lref Type_Info::lref() {return LY_TYPE_LEAFREF == type ? std::make_shared<Type_Info_Lref>(&info.lref, deleter) : nullptr;};
S_Type_Info_Str Type_Info::str() {return LY_TYPE_STRING == type ? std::make_shared<Type_Info_Str>(&info.str, deleter) : nullptr;};
S_Type_Info_Union Type_Info::uni() {return LY_TYPE_UNION == type ? std::make_shared<Type_Info_Union>(&info.uni, deleter) : nullptr;};

Type::Type(struct lys_type *type, S_Deleter deleter):
    type(type),
    deleter(deleter)
{};
Type::~Type() {};
std::vector<S_Ext_Instance> Type::ext() LY_NEW_P_LIST(type, ext, ext_size, Ext_Instance);
S_Tpdf Type::der() {return type->der ? std::make_shared<Tpdf>(type->der, deleter) : nullptr;};
S_Tpdf Type::parent() {return type->parent ? std::make_shared<Tpdf>(type->parent, deleter) : nullptr;};
S_Type_Info Type::info() {return std::make_shared<Type_Info>(type->info, &type->base, type->value_flags, deleter);};

Iffeature::Iffeature(struct lys_iffeature *iffeature, S_Deleter deleter):
    iffeature(iffeature),
    deleter(deleter)
{};
Iffeature::~Iffeature() {};
std::vector<S_Feature> Iffeature::features() {
    std::vector<S_Feature> s_vector;

    for (size_t i = 0; i < sizeof(*iffeature->features); i++) {
        s_vector.push_back(std::make_shared<Feature>(iffeature->features[i], deleter));
    }

    return s_vector;
};
std::vector<S_Ext_Instance> Iffeature::ext() LY_NEW_P_LIST(iffeature, ext, ext_size, Ext_Instance);

Ext_Instance::Ext_Instance(lys_ext_instance *ext_instance, S_Deleter deleter):
    ext_instance(ext_instance),
    deleter(deleter)
{};
Ext_Instance::~Ext_Instance() {};
std::vector<S_Ext_Instance> Ext_Instance::ext() LY_NEW_P_LIST(ext_instance, ext, ext_size, Ext_Instance);

Revision::Revision(lys_revision *revision, S_Deleter deleter):
    revision(revision),
    deleter(deleter)
{};
Revision::~Revision() {};

Schema_Node::Schema_Node(struct lys_node *node, S_Deleter deleter):
    node(node),
    deleter(deleter)
{};
Schema_Node::~Schema_Node() {};
std::vector<S_Ext_Instance> Schema_Node::ext() LY_NEW_P_LIST(node, ext, ext_size, Ext_Instance);
S_Module Schema_Node::module() LY_NEW(node, module, Module);
S_Schema_Node Schema_Node::parent() LY_NEW(node, parent, Schema_Node);
S_Schema_Node Schema_Node::child() LY_NEW(node, child, Schema_Node);
S_Schema_Node Schema_Node::next() LY_NEW(node, next, Schema_Node);
S_Schema_Node Schema_Node::prev() LY_NEW(node, prev, Schema_Node);
std::string Schema_Node::path(int options) {
    char *path = nullptr;

    path = lys_path(node, options);
    if (!path) {
        return nullptr;
    }

    std::string s_path = path;
    free(path);
    return s_path;
}
std::vector<S_Schema_Node> Schema_Node::child_instantiables(int options) {
    std::vector<S_Schema_Node> s_vector;
    struct lys_node *iter = NULL;

    while ((iter = (struct lys_node *)lys_getnext(iter, node, node->module, options))) {
        s_vector.push_back(std::make_shared<Schema_Node>(iter, deleter));
    }

    return s_vector;
}
S_Set Schema_Node::find_path(const char *path) {
    struct ly_set *set = lys_find_path(node->module, node, path);
    if (!set) {
        check_libyang_error(node->module->ctx);
        return nullptr;
    }

    S_Deleter new_deleter = std::make_shared<Deleter>(set, deleter);
    return std::make_shared<Set>(set, new_deleter);
}

S_Set Schema_Node::xpath_atomize(enum lyxp_node_type ctx_node_type, const char *expr, int options) {
    struct ly_set *set = lys_xpath_atomize(node, ctx_node_type, expr, options);
    if (!set) {
        check_libyang_error(node->module->ctx);
        return nullptr;
    }

    return std::make_shared<Set>(set, deleter);
}
S_Set Schema_Node::xpath_atomize(int options) {
    struct ly_set *set = lys_node_xpath_atomize(node, options);
    if (!set) {
        check_libyang_error(node->module->ctx);
        return nullptr;
    }

    return std::make_shared<Set>(set, deleter);
}
std::vector<S_Schema_Node> Schema_Node::tree_for() {
    std::vector<S_Schema_Node> s_vector;

    struct lys_node *elem = nullptr;
    LY_TREE_FOR(node, elem) {
        s_vector.push_back(std::make_shared<Schema_Node>(elem, deleter));
    }

    return s_vector;
}
std::vector<S_Schema_Node> Schema_Node::tree_dfs() {
    std::vector<S_Schema_Node> s_vector;

    struct lys_node *elem = nullptr, *next = nullptr;
    LY_TREE_DFS_BEGIN(node, next, elem) {
        s_vector.push_back(std::make_shared<Schema_Node>(elem, deleter));
        LY_TREE_DFS_END(node, next, elem)
    }

    return s_vector;
}

Schema_Node_Container::~Schema_Node_Container() {};
S_When Schema_Node_Container::when() LY_NEW_CASTED(lys_node_container, node, when, When);
S_Restr Schema_Node_Container::must() {
    struct lys_node_container *node_container = (struct lys_node_container *)node;
    return node_container->must ? std::make_shared<Restr>(node_container->must, deleter) : nullptr;
};
S_Tpdf Schema_Node_Container::ptdf() {
    struct lys_node_container *node_container = (struct lys_node_container *)node;
    return node_container->tpdf ? std::make_shared<Tpdf>(node_container->tpdf, deleter) : nullptr;
};

Schema_Node_Choice::~Schema_Node_Choice() {};
S_When Schema_Node_Choice::when() LY_NEW_CASTED(lys_node_choice, node, when, When);
S_Schema_Node Schema_Node_Choice::dflt() {
    struct lys_node_choice *node_choice = (struct lys_node_choice *)node;
    return node_choice->dflt ? std::make_shared<Schema_Node>(node_choice->dflt, deleter) : nullptr;
};

Schema_Node_Leaf::~Schema_Node_Leaf() {};
S_Set Schema_Node_Leaf::backlinks() LY_NEW_CASTED(lys_node_leaf, node, backlinks, Set);
S_When Schema_Node_Leaf::when() LY_NEW_CASTED(lys_node_leaf, node, when, When);
S_Type Schema_Node_Leaf::type() {return std::make_shared<Type>(&((struct lys_node_leaf *)node)->type, deleter);}
S_Schema_Node_List Schema_Node_Leaf::is_key() {
    uint8_t pos;

    auto list = lys_is_key((struct lys_node_leaf *)node, &pos);
    return list ? std::make_shared<Schema_Node_List>((struct lys_node *) list, deleter) : nullptr;
}

Schema_Node_Leaflist::~Schema_Node_Leaflist() {};
S_Set Schema_Node_Leaflist::backlinks() LY_NEW_CASTED(lys_node_leaflist, node, backlinks, Set);
S_When Schema_Node_Leaflist::when() LY_NEW_CASTED(lys_node_leaflist, node, when, When);
std::vector<std::string> Schema_Node_Leaflist::dflt() {
    struct lys_node_leaflist *node_leaflist = (struct lys_node_leaflist *)node;
    LY_NEW_STRING_LIST(node_leaflist, dflt, dflt_size);
}
std::vector<S_Restr> Schema_Node_Leaflist::must() LY_NEW_LIST_CASTED(lys_node_leaflist, node, must, must_size, Restr);
S_Type Schema_Node_Leaflist::type() {return std::make_shared<Type>(&((struct lys_node_leaflist *)node)->type, deleter);}

Schema_Node_List::~Schema_Node_List() {};
S_When Schema_Node_List::when() LY_NEW_CASTED(lys_node_list, node, when, When);
std::vector<S_Restr> Schema_Node_List::must() LY_NEW_LIST_CASTED(lys_node_list, node, must, must_size, Restr);
std::vector<S_Tpdf> Schema_Node_List::tpdf() LY_NEW_LIST_CASTED(lys_node_list, node, tpdf, tpdf_size, Tpdf);
std::vector<S_Schema_Node_Leaf> Schema_Node_List::keys() {
    auto list = (struct lys_node_list *) node;

    std::vector<S_Schema_Node_Leaf> s_vector;

    for (uint8_t i = 0; i < list->keys_size; i++) {
        s_vector.push_back(std::make_shared<Schema_Node_Leaf>((struct lys_node *) list->keys[i], deleter));
    }

    return s_vector;
}
std::vector<S_Unique> Schema_Node_List::unique() LY_NEW_LIST_CASTED(lys_node_list, node, unique, unique_size, Unique);

Schema_Node_Anydata::~Schema_Node_Anydata() {};
S_When Schema_Node_Anydata::when() LY_NEW_CASTED(lys_node_anydata, node, when, When);
std::vector<S_Restr> Schema_Node_Anydata::must() LY_NEW_LIST_CASTED(lys_node_anydata, node, must, must_size, Restr);

Schema_Node_Uses::~Schema_Node_Uses() {};
S_When Schema_Node_Uses::when() LY_NEW_CASTED(lys_node_uses, node, when, When);
std::vector<S_Refine> Schema_Node_Uses::refine() LY_NEW_LIST_CASTED(lys_node_uses, node, refine, refine_size, Refine);
std::vector<S_Schema_Node_Augment> Schema_Node_Uses::augment() {
    auto uses = (struct lys_node_uses *) node;

    std::vector<S_Schema_Node_Augment> s_vector;

    for (uint8_t i = 0; i < uses->augment_size; i++) {
        s_vector.push_back(std::make_shared<Schema_Node_Augment>((struct lys_node *) &uses->augment[i], deleter));
    }

    return s_vector;
}
S_Schema_Node_Grp Schema_Node_Uses::grp() {
    auto uses = (struct lys_node_uses *) node;
    return uses->grp ? std::make_shared<Schema_Node_Grp>(node, deleter) : nullptr;
};

Schema_Node_Grp::~Schema_Node_Grp() {};
std::vector<S_Tpdf> Schema_Node_Grp::tpdf() LY_NEW_LIST_CASTED(lys_node_grp, node, tpdf, tpdf_size, Tpdf);

Schema_Node_Case::~Schema_Node_Case() {};
S_When Schema_Node_Case::when() LY_NEW_CASTED(lys_node_case, node, when, When);

Schema_Node_Inout::~Schema_Node_Inout() {};
std::vector<S_Tpdf> Schema_Node_Inout::tpdf() LY_NEW_LIST_CASTED(lys_node_inout, node, tpdf, tpdf_size, Tpdf);
std::vector<S_Restr> Schema_Node_Inout::must() LY_NEW_LIST_CASTED(lys_node_inout, node, must, must_size, Restr);

Schema_Node_Notif::~Schema_Node_Notif() {};
std::vector<S_Tpdf> Schema_Node_Notif::tpdf() LY_NEW_LIST_CASTED(lys_node_notif, node, tpdf, tpdf_size, Tpdf);
std::vector<S_Restr> Schema_Node_Notif::must() LY_NEW_LIST_CASTED(lys_node_notif, node, must, must_size, Restr);

Schema_Node_Rpc_Action::~Schema_Node_Rpc_Action() {};
std::vector<S_Tpdf> Schema_Node_Rpc_Action::tpdf() LY_NEW_LIST_CASTED(lys_node_rpc_action, node, tpdf, tpdf_size, Tpdf);

Schema_Node_Augment::~Schema_Node_Augment() {};
S_When Schema_Node_Augment::when() LY_NEW_CASTED(lys_node_augment, node, when, When);

When::When(struct lys_when *when, S_Deleter deleter):
    when(when),
    deleter(deleter)
{};
When::~When() {};
std::vector<S_Ext_Instance> When::ext() LY_NEW_P_LIST(when, ext, ext_size, Ext_Instance);

Substmt::Substmt(struct lyext_substmt *substmt, S_Deleter deleter):
    substmt(substmt),
    deleter(deleter)
{};
Substmt::~Substmt() {};

Ext::Ext(struct lys_ext *ext, S_Deleter deleter):
    ext(ext),
    deleter(deleter)
{};
Ext::~Ext() {};
std::vector<S_Ext_Instance> Ext::ext_instance() LY_NEW_P_LIST(ext, ext, ext_size, Ext_Instance);
S_Module Ext::module() LY_NEW(ext, module, Module);

Refine_Mod_List::Refine_Mod_List(struct lys_refine_mod_list *list, S_Deleter deleter):
    list(list),
    deleter(deleter)
{};
Refine_Mod_List::~Refine_Mod_List() {};

Refine_Mod::Refine_Mod(union lys_refine_mod mod, uint16_t target_type, S_Deleter deleter):
    mod(mod),
    target_type(target_type),
    deleter(deleter)
{};
Refine_Mod::~Refine_Mod() {};
//TODO check which type's to accept
S_Refine_Mod_List Refine_Mod::list() {return target_type != LYS_CONTAINER ? std::make_shared<Refine_Mod_List>(&mod.list, deleter) : nullptr;};

Refine::Refine(struct lys_refine *refine, S_Deleter deleter):
    refine(refine),
    deleter(deleter)
{};
Refine::~Refine() {};
std::vector<S_Ext_Instance> Refine::ext() LY_NEW_P_LIST(refine, ext, ext_size, Ext_Instance);
S_Module Refine::module() LY_NEW(refine, module, Module);
std::vector<S_Restr> Refine::must() LY_NEW_LIST(refine, must, must_size, Restr);
S_Refine_Mod Refine::mod() {return std::make_shared<Refine_Mod>(refine->mod, refine->target_type, deleter);};

Deviate::Deviate(struct lys_deviate *deviate, S_Deleter deleter):
    deviate(deviate),
    deleter(deleter)
{};
Deviate::~Deviate() {};
std::vector<S_Ext_Instance> Deviate::ext() LY_NEW_P_LIST(deviate, ext, ext_size, Ext_Instance);
S_Restr Deviate::must() {return deviate->must ? std::make_shared<Restr>(deviate->must, deleter) : nullptr;};
S_Unique Deviate::unique() {return deviate->unique ? std::make_shared<Unique>(deviate->unique, deleter) : nullptr;};
S_Type Deviate::type() {return deviate->type ? std::make_shared<Type>(deviate->type, deleter) : nullptr;}

Deviation::Deviation(struct lys_deviation *deviation, S_Deleter deleter):
    deviation(deviation),
    deleter(deleter)
{};
Deviation::~Deviation() {};
S_Schema_Node Deviation::orig_node() LY_NEW(deviation, orig_node, Schema_Node);
std::vector<S_Deviate> Deviation::deviate() LY_NEW_LIST(deviation, deviate, deviate_size, Deviate);
std::vector<S_Ext_Instance> Deviation::ext() LY_NEW_P_LIST(deviation, ext, ext_size, Ext_Instance);

Import::Import(struct lys_import *import, S_Deleter deleter):
    import(import),
    deleter(deleter)
{};
Import::~Import() {};

Include::Include(struct lys_include *include, S_Deleter deleter):
    include(include),
    deleter(deleter)
{}
Include::~Include() {};

Tpdf::Tpdf(struct lys_tpdf *tpdf, S_Deleter deleter):
    tpdf(tpdf),
    deleter(deleter)
{}
Tpdf::~Tpdf() {};
S_Type Tpdf::type() {return std::make_shared<Type>(&tpdf->type, deleter);}

Unique::Unique(struct lys_unique *unique, S_Deleter deleter):
    unique(unique),
    deleter(deleter)
{};
Unique::~Unique() {};

Feature::Feature(struct lys_feature *feature, S_Deleter deleter):
    feature(feature),
    deleter(deleter)
{};
Feature::~Feature() {};

Restr::Restr(struct lys_restr *restr, S_Deleter deleter):
    restr(restr),
    deleter(deleter)
{};
Restr::~Restr() {};

Ident::Ident(struct lys_ident *ident, S_Deleter deleter):
    ident(ident),
    deleter(deleter)
{};
Ident::~Ident() {};
std::vector<S_Ident> Ident::base() LY_NEW_P_LIST(ident, base, base_size, Ident);

}
