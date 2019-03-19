/**
 * @file Tree_Schema.hpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Class implementation for libyang C header tree_schema.h
 *
 * Copyright (c) 2017 Deutsche Telekom AG.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef TREE_SCHEMA_H
#define TREE_SCHEMA_H

#include <iostream>
#include <memory>
#include <exception>
#include <vector>

#include "Internal.hpp"
#include "Libyang.hpp"

extern "C" {
#include "libyang.h"
#include "tree_schema.h"
}

namespace libyang {

/**
 * @defgroup classes C++/Python
 * @{
 *
 * Class wrappers for data structures and functions to manipulate and access instance data tree.
 */

/**
 * @brief classes for wrapping [lys_module](@ref lys_module).
 * @class Module
 */
class Module
{
public:
    /** wrapper for struct [lys_module](@ref lys_module), for internal use only */
    Module(struct lys_module *module, S_Deleter deleter);
    ~Module();
    /** get name variable from [lys_module](@ref lys_module)*/
    const char *name() {return module->name;};
    /** get prefix variable from [lys_module](@ref lys_module)*/
    const char *prefix() {return module->prefix;};
    /** get dsc variable from [lys_module](@ref lys_module)*/
    const char *dsc() {return module->dsc;};
    /** get ref variable from [lys_module](@ref lys_module)*/
    const char *ref() {return module->ref;};
    /** get org variable from [lys_module](@ref lys_module)*/
    const char *org() {return module->org;};
    /** get contact variable from [lys_module](@ref lys_module)*/
    const char *contact() {return module->contact;};
    /** get filepath variable from [lys_module](@ref lys_module)*/
    const char *filepath() {return module->filepath;};
    /** get type variable from [lys_module](@ref lys_module)*/
    uint8_t type() {return module->type;};
    /** get version variable from [lys_module](@ref lys_module)*/
    uint8_t version() {return module->version;};
    /** get deviated variable from [lys_module](@ref lys_module)*/
    uint8_t deviated() {return module->deviated;};
    /** get disabled variable from [lys_module](@ref lys_module)*/
    uint8_t disabled() {return module->disabled;};
    /** get rev_size variable from [lys_module](@ref lys_module)*/
    uint8_t rev_size() {return module->rev_size;};
    /** get imp_size variable from [lys_module](@ref lys_module)*/
    uint8_t imp_size() {return module->imp_size;};
    /** get inc_size variable from [lys_module](@ref lys_module)*/
    uint8_t inc_size() {return module->inc_size;};
    /** get ident_size variable from [lys_module](@ref lys_module)*/
    uint8_t ident_size() {return module->ident_size;};
    /** get tpdf_size variable from [lys_module](@ref lys_module)*/
    uint8_t tpdf_size() {return module->tpdf_size;};
    /** get features_size variable from [lys_module](@ref lys_module)*/
    uint8_t features_size() {return module->features_size;};
    /** get augment_size variable from [lys_module](@ref lys_module)*/
    uint8_t augment_size() {return module->augment_size;};
    /** get deviation_size variable from [lys_module](@ref lys_module)*/
    uint8_t devaiation_size() {return module->deviation_size;};
    /** get extensions_size variable from [lys_module](@ref lys_module)*/
    uint8_t extensions_size() {return module->extensions_size;};
    /** get ext_size variable from [lys_module](@ref lys_module)*/
    uint8_t ext_size() {return module->ext_size;};
    /** get ns variable from [lys_module](@ref lys_module)*/
    const char *ns() {return module->ns;};
    /** get rev variable from [lys_module](@ref lys_module)*/
    S_Revision rev();
    /** get deviation variable from [lys_module](@ref lys_module)*/
    std::vector<S_Deviation> deviation();
    /** get data variable from [lys_module](@ref lys_module)*/
    S_Schema_Node data() LY_NEW(module, data, Schema_Node);
    /** wrapper for [lys_getnext](@ref lys_getnext) */
    std::vector<S_Schema_Node> data_instantiables(int options);
    /** wrapper for [lys_print_mem](@ref lys_print_mem) */
    std::string print_mem(LYS_OUTFORMAT format, int options);
    std::string print_mem(LYS_OUTFORMAT format, const char *target, int options);

    friend Context;
    friend Data_Node;

private:
    struct lys_module *module;
    S_Deleter deleter;
};

/**
 * @brief classes for wrapping [lys_submodule](@ref lys_submodule).
 * @class Submodule
 */
class Submodule
{
public:
    /** wrapper for struct [lys_submodule](@ref lys_submodule), for internal use only */
    Submodule(struct lys_submodule *submodule, S_Deleter deleter);
    ~Submodule();
    /** get ctx variable from [lys_submodule](@ref lys_submodule)*/
    S_Context ctx() LY_NEW(submodule, ctx, Context);
    /** get name variable from [lys_submodule](@ref lys_submodule)*/
    const char *name() {return submodule->name;};
    /** get prefix variable from [lys_submodule](@ref lys_submodule)*/
    const char *prefix() {return submodule->prefix;};
    /** get dsc variable from [lys_submodule](@ref lys_submodule)*/
    const char *dsc() {return submodule->dsc;};
    /** get ref variable from [lys_submodule](@ref lys_submodule)*/
    const char *ref() {return submodule->ref;};
    /** get org variable from [lys_submodule](@ref lys_submodule)*/
    const char *org() {return submodule->org;};
    /** get contact variable from [lys_submodule](@ref lys_submodule)*/
    const char *contact() {return submodule->contact;};
    /** get filepath variable from [lys_submodule](@ref lys_submodule)*/
    const char *filepath() {return submodule->filepath;};
    /** get type variable from [lys_submodule](@ref lys_submodule)*/
    uint8_t type() {return submodule->type;};
    /** get version variable from [lys_submodule](@ref lys_submodule)*/
    uint8_t version() {return submodule->version;};
    /** get deviated variable from [lys_submodule](@ref lys_submodule)*/
    uint8_t deviated() {return submodule->deviated;};
    /** get disabled variable from [lys_submodule](@ref lys_submodule)*/
    uint8_t disabled() {return submodule->disabled;};
    /** get implemented variable from [lys_submodule](@ref lys_submodule)*/
    uint8_t implemented() {return submodule->implemented;};
    /** get rev_size variable from [lys_submodule](@ref lys_submodule)*/
    uint8_t rev_size() {return submodule->rev_size;};
    /** get imp_size variable from [lys_submodule](@ref lys_submodule)*/
    uint8_t imp_size() {return submodule->imp_size;};
    /** get inc_size variable from [lys_submodule](@ref lys_submodule)*/
    uint8_t inc_size() {return submodule->inc_size;};
    /** get ident_size variable from [lys_submodule](@ref lys_submodule)*/
    uint8_t ident_size() {return submodule->ident_size;};
    /** get tpdf_size variable from [lys_submodule](@ref lys_submodule)*/
    uint8_t tpdf_size() {return submodule->tpdf_size;};
    /** get features_size variable from [lys_submodule](@ref lys_submodule)*/
    uint8_t features_size() {return submodule->features_size;};
    /** get augment_size variable from [lys_submodule](@ref lys_submodule)*/
    uint8_t augment_size() {return submodule->augment_size;};
    /** get deviation_size variable from [lys_submodule](@ref lys_submodule)*/
    uint8_t deviation_size() {return submodule->deviation_size;};
    /** get extensions_size variable from [lys_submodule](@ref lys_submodule)*/
    uint8_t extensions_size() {return submodule->extensions_size;};
    /** get ext_size variable from [lys_submodule](@ref lys_submodule)*/
    uint8_t ext_size() {return submodule->ext_size;};
    /** get rev variable from [lys_submodule](@ref lys_submodule)*/
    S_Revision rev();
    /** get deviation variable from [lys_submodule](@ref lys_submodule)*/
    std::vector<S_Deviation> deviation();
    /** get belongsto variable from [lys_submodule](@ref lys_submodule)*/
    S_Module belongsto() LY_NEW(submodule, belongsto, Module);

private:
    struct lys_submodule *submodule;
    S_Deleter deleter;
};

class Type_Info_Binary
{
public:
    /** wrapper for struct [lys_type_info_binary](@ref lys_type_info_binary), for internal use only */
    Type_Info_Binary(struct lys_type_info_binary *info_binary, S_Deleter deleter);
    ~Type_Info_Binary();
    /** get length variable from [lys_type_info_binary](@ref lys_type_info_binary)*/
    S_Restr length();

private:
    lys_type_info_binary *info_binary;
    S_Deleter deleter;
};

class Type_Bit
{
public:
    /** wrapper for struct [lys_type_bit](@ref lys_type_bit), for internal use only */
    Type_Bit(struct lys_type_bit *info_bit, S_Deleter deleter);
    ~Type_Bit();
    /** get name variable from [lys_type_bit](@ref lys_type_bit)*/
    const char *name() {return info_bit->name;};
    /** get dsc variable from [lys_type_bit](@ref lys_type_bit)*/
    const char *dsc() {return info_bit->dsc;};
    /** get ref variable from [lys_type_bit](@ref lys_type_bit)*/
    const char *ref() {return info_bit->ref;};
    /** get flags variable from [lys_type_bit](@ref lys_type_bit)*/
    uint16_t flags() {return info_bit->flags;};
    /** get ext_size variable from [lys_type_bit](@ref lys_type_bit)*/
    uint8_t ext_size() {return info_bit->ext_size;};
    /** get iffeature_size variable from [lys_type_bit](@ref lys_type_bit)*/
    uint8_t iffeature_size() {return info_bit->iffeature_size;};
    /** get pos variable from [lys_type_bit](@ref lys_type_bit)*/
    uint32_t pos() {return info_bit->pos;};
    /** get ext variable from [lys_type_bit](@ref lys_type_bit)*/
    std::vector<S_Ext_Instance> ext();
    /** get iffeature variable from [lys_type_bit](@ref lys_type_bit)*/
    std::vector<S_Iffeature> iffeature();

private:
    lys_type_bit *info_bit;
    S_Deleter deleter;
};

class Type_Info_Bits
{
public:
    /** wrapper for struct [lys_type_info_bits](@ref lys_type_info_bits), for internal use only */
    Type_Info_Bits(struct lys_type_info_bits *info_bits, S_Deleter deleter);
    ~Type_Info_Bits();
    /** get bit variable from [lys_type_info_bits](@ref lys_type_info_bits)*/
    std::vector<S_Type_Bit> bit();
    /** get count variable from [lys_type_info_bits](@ref lys_type_info_bits)*/
    unsigned int count() {return info_bits->count;};

private:
    lys_type_info_bits *info_bits;
    S_Deleter deleter;
};

class Type_Info_Dec64
{
public:
    /** wrapper for struct [lys_type_info_dec64](@ref lys_type_info_dec64), for internal use only */
    Type_Info_Dec64(struct lys_type_info_dec64 *info_dec64, S_Deleter deleter);
    ~Type_Info_Dec64();
    /** get range variable from [lys_type_info_dec64](@ref lys_type_info_dec64)*/
    S_Restr range();
    /** get dig variable from [lys_type_info_dec64](@ref lys_type_info_dec64)*/
    uint8_t dig() {return info_dec64->dig;}
    /** get div variable from [lys_type_info_dec64](@ref lys_type_info_dec64)*/
    uint8_t div() {return info_dec64->div;}

private:
    lys_type_info_dec64 *info_dec64;
    S_Deleter deleter;
};

class Type_Enum
{
public:
    /** wrapper for struct [lys_type_enum](@ref lys_type_enum), for internal use only */
    Type_Enum(struct lys_type_enum *info_enum, S_Deleter deleter);
    ~Type_Enum();
    /** get name variable from [lys_type_enum](@ref lys_type_enum)*/
    const char *name() {return info_enum->name;};
    /** get dsc variable from [lys_type_enum](@ref lys_type_enum)*/
    const char *dsc() {return info_enum->dsc;};
    /** get ref variable from [lys_type_enum](@ref lys_type_enum)*/
    const char *ref() {return info_enum->ref;};
    /** get flags variable from [lys_type_enum](@ref lys_type_enum)*/
    uint16_t flags() {return info_enum->flags;};
    /** get ext_size variable from [lys_type_enum](@ref lys_type_enum)*/
    uint8_t ext_size() {return info_enum->ext_size;};
    /** get iffeature_size variable from [lys_type_enum](@ref lys_type_enum)*/
    uint8_t iffeature_size() {return info_enum->iffeature_size;};
    /** get value variable from [lys_type_enum](@ref lys_type_enum)*/
    int32_t value() {return info_enum->value;};
    /** get ext variable from [lys_type_enum](@ref lys_type_enum)*/
    std::vector<S_Ext_Instance> ext();
    /** get iffeature variable from [lys_type_enum](@ref lys_type_enum)*/
    std::vector<S_Iffeature> iffeature();

private:
    lys_type_enum *info_enum;
    S_Deleter deleter;
};

class Type_Info_Enums
{
public:
    /** wrapper for struct [lys_type_info_enums](@ref lys_type_info_enums), for internal use only */
    Type_Info_Enums(struct lys_type_info_enums *info_enums, S_Deleter deleter);
    ~Type_Info_Enums();
    /** get enm variable from [lys_type_info_enums](@ref lys_type_info_enums)*/
    std::vector<S_Type_Enum> enm();
    /** get count variable from [lys_type_info_enums](@ref lys_type_info_enums)*/
    unsigned int count() {return info_enums->count;};

private:
    lys_type_info_enums *info_enums;
    S_Deleter deleter;
};

class Type_Info_Ident
{
public:
    /** wrapper for struct [lys_type_info_ident](@ref lys_type_info_ident), for internal use only */
    Type_Info_Ident(struct lys_type_info_ident *info_ident, S_Deleter deleter);
    ~Type_Info_Ident();
    /** get ref variable from [lys_type_info_ident](@ref lys_type_info_ident)*/
    std::vector<S_Ident> ref();
    /** get count variable from [lys_type_info_ident](@ref lys_type_info_ident)*/
    int count() {return info_ident->count;};

private:
    lys_type_info_ident *info_ident;
    S_Deleter deleter;
};

class Type_Info_Inst
{
public:
    /** wrapper for struct [lys_type_info_inst](@ref lys_type_info_inst), for internal use only */
    Type_Info_Inst(struct lys_type_info_inst *info_inst, S_Deleter deleter);
    ~Type_Info_Inst();
    /** get req variable from [lys_type_info_inst](@ref lys_type_info_inst)*/
    int8_t req() {return info_inst->req;};

private:
    lys_type_info_inst *info_inst;
    S_Deleter deleter;
};

class Type_Info_Num
{
public:
    /** wrapper for struct [lys_type_info_num](@ref lys_type_info_num), for internal use only */
    Type_Info_Num(struct lys_type_info_num *info_num, S_Deleter deleter);
    ~Type_Info_Num();
    /** get range variable from [lys_type_info_num](@ref lys_type_info_num)*/
    S_Restr range();

private:
    lys_type_info_num *info_num;
    S_Deleter deleter;
};

class Type_Info_Lref
{
public:
    /** wrapper for struct [lys_type_info_lref](@ref lys_type_info_lref), for internal use only */
    Type_Info_Lref(struct lys_type_info_lref *info_lref, S_Deleter deleter);
    ~Type_Info_Lref();
    /** get path variable from [lys_type_info_lref](@ref lys_type_info_lref)*/
    const char *path() {return info_lref->path;};
    /** get target variable from [lys_type_info_lref](@ref lys_type_info_lref)*/
    S_Schema_Node_Leaf target();
    /** get req variable from [lys_type_info_lref](@ref lys_type_info_lref)*/
    int8_t req() {return info_lref->req;};

private:
    lys_type_info_lref *info_lref;
    S_Deleter deleter;
};

class Type_Info_Str
{
public:
    /** wrapper for struct [lys_type_info_str](@ref lys_type_info_str), for internal use only */
    Type_Info_Str(struct lys_type_info_str *info_str, S_Deleter deleter);
    ~Type_Info_Str();
    /** get length variable from [lys_type_info_str](@ref lys_type_info_str)*/
    S_Restr length();
    /** get patters variable from [lys_type_info_str](@ref lys_type_info_str)*/
    S_Restr patterns();
    /** get pat_count variable from [lys_type_info_str](@ref lys_type_info_str)*/
    int pat_count() {return info_str->pat_count;};

private:
    lys_type_info_str *info_str;
    S_Deleter deleter;
};

class Type_Info_Union
{
public:
    /** wrapper for struct [lys_type_info_union](@ref lys_type_info_union), for internal use only */
    Type_Info_Union(struct lys_type_info_union *info_union, S_Deleter deleter);
    ~Type_Info_Union();
    /** get types variable from [lys_type_info_union](@ref lys_type_info_union)*/
    std::vector<S_Type> types();
    /** get count variable from [lys_type_info_union](@ref lys_type_info_union)*/
    int count() {return info_union->count;};
    /** get has_ptr_type variable from [lys_type_info_union](@ref lys_type_info_union)*/
    int has_ptr_type() {return info_union->has_ptr_type;};

private:
    lys_type_info_union *info_union;
    S_Deleter deleter;
};

class Type_Info
{
public:
    /** wrapper for struct [lys_type_info](@ref lys_type_info), for internal use only */
    Type_Info(union lys_type_info info, LY_DATA_TYPE *type, uint8_t flags, S_Deleter deleter);
    ~Type_Info();
    /** get binary variable from [lys_type_info](@ref lys_type_info)*/
    S_Type_Info_Binary binary();
    /** get bits variable from [lys_type_info](@ref lys_type_info)*/
    S_Type_Info_Bits bits();
    /** get dec64 variable from [lys_type_info](@ref lys_type_info)*/
    S_Type_Info_Dec64 dec64();
    /** get enums variable from [lys_type_info](@ref lys_type_info)*/
    S_Type_Info_Enums enums();
    /** get ident variable from [lys_type_info](@ref lys_type_info)*/
    S_Type_Info_Ident ident();
    /** get inst variable from [lys_type_info](@ref lys_type_info)*/
    S_Type_Info_Inst inst();
    /** get num variable from [lys_type_info](@ref lys_type_info)*/
    S_Type_Info_Num num();
    /** get lref variable from [lys_type_info](@ref lys_type_info)*/
    S_Type_Info_Lref lref();
    /** get str variable from [lys_type_info](@ref lys_type_info)*/
    S_Type_Info_Str str();
    /** get uni variable from [lys_type_info](@ref lys_type_info)*/
    S_Type_Info_Union uni();

private:
    union lys_type_info info;
    LY_DATA_TYPE type;
    uint8_t flags;
    S_Deleter deleter;
};

class Type
{
public:
    /** wrapper for struct [lys_type](@ref lys_type), for internal use only */
    Type(struct lys_type *type, S_Deleter deleter);
    ~Type();
    /** get base variable from [lys_type](@ref lys_type)*/
    LY_DATA_TYPE base() {return type->base;};
    /** get ext_size variable from [lys_type](@ref lys_type)*/
    uint8_t ext_size() {return type->ext_size;};
    /** get ext variable from [lys_type](@ref lys_type)*/
    std::vector<S_Ext_Instance> ext();
    /** get der variable from [lys_type](@ref lys_type)*/
    S_Tpdf der();
    /** get parent variable from [lys_type](@ref lys_type)*/
    S_Tpdf parent();
    /** get info variable from [lys_type](@ref lys_type)*/
    S_Type_Info info();

private:
    struct lys_type *type;
    S_Deleter deleter;
};

class Iffeature {
public:
    /** wrapper for struct [lys_iffeature](@ref lys_iffeature), for internal use only */
    Iffeature(struct lys_iffeature *iffeature, S_Deleter deleter);
    ~Iffeature();
    /** get expr variable from [lys_iffeature](@ref lys_iffeature)*/
    uint8_t *expr() {return iffeature->expr;};
    /** get ext_size variable from [lys_iffeature](@ref lys_iffeature)*/
    uint8_t ext_size() {return iffeature->ext_size;};
    /** get features variable from [lys_iffeature](@ref lys_iffeature)*/
    std::vector<S_Feature> features();
    /** get ext variable from [lys_iffeature](@ref lys_iffeature)*/
    std::vector<S_Ext_Instance> ext();

private:
    struct lys_iffeature *iffeature;
    S_Deleter deleter;
};

class Ext_Instance
{
public:
    /** wrapper for struct [lys_ext_instance](@ref lys_ext_instance), for internal use only */
    Ext_Instance(lys_ext_instance *ext_instance, S_Deleter deleter);
    ~Ext_Instance();
    //TODO void *parent();
    /** get arg_value variable from [lys_ext_instance](@ref lys_ext_instance)*/
    const char *arg_value() {return ext_instance->arg_value;};
    /** get flags variable from [lys_ext_instance](@ref lys_ext_instance)*/
    uint16_t flags() {return ext_instance->flags;};
    /** get ext_size variable from [lys_ext_instance](@ref lys_ext_instance)*/
    uint8_t ext_size() {return ext_instance->ext_size;};
    /** get insubstmt_index variable from [lys_ext_instance](@ref lys_ext_instance)*/
    uint8_t insubstmt_index() {return ext_instance->insubstmt_index;};
    /** get insubstmt variable from [lys_ext_instance](@ref lys_ext_instance)*/
    uint8_t insubstmt() {return ext_instance->insubstmt;};
    /** get parent_type variable from [lys_ext_instance](@ref lys_ext_instance)*/
    uint8_t parent_type() {return ext_instance->parent_type;};
    /** get ext_type variable from [lys_ext_instance](@ref lys_ext_instance)*/
    uint8_t ext_type() {return ext_instance->ext_type;};
    /** get ext variable from [lys_ext_instance](@ref lys_ext_instance)*/
    std::vector<S_Ext_Instance> ext();
    /** get definition of the instantiated extension from [lys_ext_instance](@ref lys_ext_instance)*/
    S_Ext def() LY_NEW(ext_instance, def, Ext);
    /** get priv variable from [lys_ext_instance](@ref lys_ext_instance)*/
    void *priv() {return ext_instance->priv;};
    /** get module variable from [lys_ext_instance](@ref lys_ext_instance)*/
    S_Module module() LY_NEW(ext_instance, module, Module);
    /** get nodetype variable from [lys_ext_instance](@ref lys_ext_instance)*/
    LYS_NODE nodetype() {return ext_instance->nodetype;};
private:
    struct lys_ext_instance *ext_instance;
    S_Deleter deleter;
};

class Schema_Node
{
public:
    /** wrapper for struct [lys_node](@ref lys_node), for internal use only */
    Schema_Node(lys_node *node, S_Deleter deleter);
    virtual ~Schema_Node();
    /** get name variable from [lys_node](@ref lys_node)*/
    const char *name() {return node->name;};
    /** get dsc variable from [lys_node](@ref lys_node)*/
    const char *dsc() {return node->dsc;};
    /** get ref variable from [lys_node](@ref lys_node)*/
    const char *ref() {return node->ref;};
    /** get flags variable from [lys_node](@ref lys_node)*/
    uint16_t flags() {return node->flags;};
    /** get ext_size variable from [lys_node](@ref lys_node)*/
    uint8_t ext_size() {return node->ext_size;};
    /** get iffeature_size variable from [lys_node](@ref lys_node)*/
    uint8_t iffeature_size() {return node->iffeature_size;};
    /** get ext variable from [lys_node](@ref lys_node)*/
    std::vector<S_Ext_Instance> ext();
    /** get iffeature variable from [lys_node](@ref lys_node)*/
    std::vector<S_Iffeature> iffeature() LY_NEW_LIST(node, iffeature, iffeature_size, Iffeature);
    /** get module variable from [lys_node](@ref lys_node)*/
    S_Module module();
    /** get nodetype variable from [lys_node](@ref lys_node)*/
    LYS_NODE nodetype() {return node->nodetype;};
    /** get parent variable from [lys_node](@ref lys_node)*/
    virtual S_Schema_Node parent();
    /** get child variable from [lys_node](@ref lys_node)*/
    virtual S_Schema_Node child();
    /** get next variable from [lys_node](@ref lys_node)*/
    virtual S_Schema_Node next();
    /** get prev variable from [lys_node](@ref lys_node)*/
    virtual S_Schema_Node prev();

    /** wrapper for [lys_path](@ref lys_path) */
    std::string path(int options = 0);
    /** wrapper for [lyd_validate_value](@ref lyd_validate_value) */
    int validate_value(const char *value) {return lyd_validate_value(node, value);};
    /** wrapper for [lys_getnext](@ref lys_getnext) */
    std::vector<S_Schema_Node> child_instantiables(int options);
    /** wrapper for [lys_find_path](@ref lys_find_path) */
    S_Set find_path(const char *path);
    /** wrapper for [lys_xpath_atomize](@ref lys_xpath_atomize) */
    S_Set xpath_atomize(enum lyxp_node_type ctx_node_type, const char *expr, int options);
    /** wrapper for [lys_xpath_atomize](@ref lys_xpath_atomize) */
    S_Set xpath_atomize(int options);
    // void *priv;

    /* emulate TREE macro's */
    /** wrapper for macro [LY_TREE_FOR](@ref LY_TREE_FOR) */
    std::vector<S_Schema_Node> tree_for();
    /** wrapper for macro [LY_TREE_DFS_BEGIN](@ref LY_TREE_DFS_BEGIN) and [LY_TREE_DFS_END](@ref LY_TREE_DFS_END) */
    std::vector<S_Schema_Node> tree_dfs();

    /* SWIG can not access private variables so it needs public getters */
    struct lys_node *swig_node() {return node;};
    S_Deleter swig_deleter() {return deleter;};

    friend Set;
    friend Data_Node;
    friend Context;
    friend Schema_Node_Container;
    friend Schema_Node_Choice;
    friend Schema_Node_Leaf;
    friend Schema_Node_Leaflist;
    friend Schema_Node_List;
    friend Schema_Node_Anydata;
    friend Schema_Node_Uses;
    friend Schema_Node_Grp;
    friend Schema_Node_Case;
    friend Schema_Node_Inout;
    friend Schema_Node_Notif;
    friend Schema_Node_Action;
    friend Schema_Node_Augment;
    friend Schema_Node_Rpc_Action;

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Container : public Schema_Node
{
public:
    Schema_Node_Container(S_Schema_Node derived):
        Schema_Node(derived->node, derived->deleter),
        node(derived->node),
        deleter(derived->deleter)
    {
        if (derived->node->nodetype != LYS_CONTAINER) {
            throw std::invalid_argument("Type must be LYS_CONTAINER");
        }
    };
    /** wrapper for struct [lys_node_container](@ref lys_node_container), for internal use only */
    Schema_Node_Container(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Container();
    /** get when variable from [lys_node_container](@ref lys_node_container)*/
    S_When when();
    /** get must variable from [lys_node_container](@ref lys_node_container)*/
    S_Restr must();
    /** get ptdf variable from [lys_node_container](@ref lys_node_container)*/
    S_Tpdf ptdf();
    /** get presence variable from [lys_node_container](@ref lys_node_container)*/
    const char *presence() {return ((struct lys_node_container *) node)->presence;};

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Choice : public Schema_Node
{
public:
    Schema_Node_Choice(S_Schema_Node derived):
        Schema_Node(derived->node, derived->deleter),
        node(derived->node),
        deleter(derived->deleter)
    {
        if (derived->node->nodetype != LYS_CHOICE) {
            throw std::invalid_argument("Type must be LYS_CHOICE");
        }
    };
    /** wrapper for struct [lys_node_choice](@ref lys_node_choice), for internal use only */
    Schema_Node_Choice(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Choice();
    /** get when variable from [lys_node_choice](@ref lys_node_choice)*/
    S_When when();
    /** get dflt variable from [lys_node_choice](@ref lys_node_choice)*/
    S_Schema_Node dflt();

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Leaf : public Schema_Node
{
public:
    Schema_Node_Leaf(S_Schema_Node derived):
        Schema_Node(derived->node, derived->deleter),
        node(derived->node),
        deleter(derived->deleter)
    {
        if (derived->node->nodetype != LYS_LEAF) {
            throw std::invalid_argument("Type must be LYS_LEAF");
        }
    };
    /** wrapper for struct [lys_node_leaf](@ref lys_node_leaf), for internal use only */
    Schema_Node_Leaf(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Leaf();
    /** get backlinks variable from [lys_node_leaf](@ref lys_node_leaf)*/
    S_Set backlinks();
    /** get when variable from [lys_node_leaf](@ref lys_node_leaf)*/
    S_When when();
    /** get type variable from [lys_node_leaf](@ref lys_node_leaf)*/
    S_Type type();
    /** get units variable from [lys_node_leaf](@ref lys_node_leaf)*/
    const char *units() {return ((struct lys_node_leaf *)node)->units;};
    /** get dflt variable from [lys_node_leaf](@ref lys_node_leaf)*/
    const char *dflt() {return ((struct lys_node_leaf *)node)->dflt;};
    S_Schema_Node child() override {return nullptr;};
    /** wrapper for [lys_is_key](@ref lys_is_key) */
    S_Schema_Node_List is_key();

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Leaflist : public Schema_Node
{
public:
    Schema_Node_Leaflist(S_Schema_Node derived):
        Schema_Node(derived->node, derived->deleter),
        node(derived->node),
        deleter(derived->deleter)
    {
        if (derived->node->nodetype != LYS_LEAFLIST) {
            throw std::invalid_argument("Type must be LYS_LEAFLIST");
        }
    };
    /** wrapper for struct [lys_node_leaflist](@ref lys_node_leaflist), for internal use only */
    Schema_Node_Leaflist(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Leaflist();
    /** get dflt_size variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    uint8_t dflt_size() {return ((struct lys_node_leaflist *)node)->dflt_size;};
    /** get must_size variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    uint8_t must_size() {return ((struct lys_node_leaflist *)node)->must_size;};
    /** get when variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    S_When when();
    /** get backlinks variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    S_Set backlinks();
    /** get must variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    std::vector<S_Restr> must();
    /** get type variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    S_Type type();
    /** get units variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    const char *units() {return ((struct lys_node_leaflist *)node)->units;};
    /** get dflt variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    std::vector<std::string> dflt();
    /** get min variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    uint32_t min() {return ((struct lys_node_leaflist *)node)->min;};
    /** get max variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    uint32_t max() {return ((struct lys_node_leaflist *)node)->max;};
    S_Schema_Node child() override {return nullptr;};

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_List : public Schema_Node
{
public:
    Schema_Node_List(S_Schema_Node derived):
        Schema_Node(derived->node, derived->deleter),
        node(derived->node),
        deleter(derived->deleter)
    {
        if (derived->node->nodetype != LYS_LIST) {
            throw std::invalid_argument("Type must be LYS_LIST");
        }
    };
    /** wrapper for struct [lys_node_list](@ref lys_node_list), for internal use only */
    Schema_Node_List(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_List();
    /** get must_size variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    uint8_t must_size() {return ((struct lys_node_list *)node)->must_size;};
    /** get tpdf_size variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    uint8_t tpdf_size() {return ((struct lys_node_list *)node)->tpdf_size;};
    /** get keys_size variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    uint8_t keys_size() {return ((struct lys_node_list *)node)->keys_size;};
    /** get unique_size variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    uint8_t unique_size() {return ((struct lys_node_list *)node)->unique_size;};
    /** get when variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    S_When when();
    /** get must variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    std::vector<S_Restr> must();
    /** get tpdf variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    std::vector<S_Tpdf> tpdf();
    /** get keys variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    std::vector<S_Schema_Node_Leaf> keys();
    /** get unique variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    std::vector<S_Unique> unique();
    /** get min variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    uint32_t min() {return ((struct lys_node_list *)node)->min;};
    /** get max variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    uint32_t max() {return ((struct lys_node_list *)node)->max;};
    /** get keys_str variable from [lys_node_leaflist](@ref lys_node_leaflist)*/
    const char *keys_str() {return ((struct lys_node_list *)node)->keys_str;};

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Anydata : public Schema_Node
{
public:
    Schema_Node_Anydata(S_Schema_Node derived):
        Schema_Node(derived->node, derived->deleter),
        node(derived->node),
        deleter(derived->deleter)
    {
        if (derived->node->nodetype != LYS_ANYDATA && derived->node->nodetype != LYS_ANYXML) {
            throw std::invalid_argument("Type must be LYS_ANYDATA or LYS_ANYXML");
        }
    };
    /** wrapper for struct [lys_node_anydata](@ref lys_node_anydata), for internal use only */
    Schema_Node_Anydata(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Anydata();
    /** get must_size variable from [lys_node_anydata](@ref lys_node_anydata)*/
    uint8_t must_size() {return ((struct lys_node_list *)node)->must_size;};
    /** get when variable from [lys_node_anydata](@ref lys_node_anydata)*/
    S_When when();
    /** get must variable from [lys_node_anydata](@ref lys_node_anydata)*/
    std::vector<S_Restr> must();

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Uses : public Schema_Node
{
public:
    Schema_Node_Uses(S_Schema_Node derived):
        Schema_Node(derived->node, derived->deleter),
        node(derived->node),
        deleter(derived->deleter)
    {
        if (derived->node->nodetype != LYS_USES) {
            throw std::invalid_argument("Type must be LYS_USES");
        }
    };
    /** wrapper for struct [lys_node_uses](@ref lys_node_uses), for internal use only */
    Schema_Node_Uses(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Uses();
    /** get augment_size variable from [lys_node_uses](@ref lys_node_uses)*/
    uint8_t augment_size() {return ((struct lys_node_uses *)node)->augment_size;};
    /** get when variable from [lys_node_uses](@ref lys_node_uses)*/
    S_When when();
    /** get refine variable from [lys_node_uses](@ref lys_node_uses)*/
    std::vector<S_Refine> refine();
    /** get augment variable from [lys_node_uses](@ref lys_node_uses)*/
    std::vector<S_Schema_Node_Augment> augment();
    /** get grp variable from [lys_node_uses](@ref lys_node_uses)*/
    S_Schema_Node_Grp grp();

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Grp : public Schema_Node
{
public:
    Schema_Node_Grp(S_Schema_Node derived):
        Schema_Node(derived->node, derived->deleter),
        node(derived->node),
        deleter(derived->deleter)
    {
        if (derived->node->nodetype != LYS_GROUPING) {
            throw std::invalid_argument("Type must be LYS_GROUPING");
        }
    };
    /** wrapper for struct [lys_node_grp](@ref lys_node_grp), for internal use only */
    Schema_Node_Grp(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Grp();
    /** get tpdf_size variable from [lys_node_grp](@ref lys_node_grp)*/
    uint8_t tpdf_size() {return ((struct lys_node_grp *)node)->tpdf_size;};
    /** get tpdf variable from [lys_node_grp](@ref lys_node_grp)*/
    std::vector<S_Tpdf> tpdf();

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Case : public Schema_Node
{
public:
    Schema_Node_Case(S_Schema_Node derived):
        Schema_Node(derived->node, derived->deleter),
        node(derived->node),
        deleter(derived->deleter)
    {
        if (derived->node->nodetype != LYS_CASE) {
            throw std::invalid_argument("Type must be LYS_CASE");
        }
    };
    /** wrapper for struct [lys_node_case](@ref lys_node_case), for internal use only */
    Schema_Node_Case(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Case();
    /** get when variable from [lys_node_case](@ref lys_node_case)*/
    S_When when();

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Inout : public Schema_Node
{
public:
    Schema_Node_Inout(S_Schema_Node derived):
        Schema_Node(derived->node, derived->deleter),
        node(derived->node),
        deleter(derived->deleter)
    {
        if (derived->node->nodetype != LYS_INPUT && derived->node->nodetype != LYS_OUTPUT) {
            throw std::invalid_argument("Type must be LYS_INOUT or LYS_OUTPUT");
        }
    };
    /** wrapper for struct [lys_node_inout](@ref lys_node_inout), for internal use only */
    Schema_Node_Inout(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Inout();
    /** get tpdf_size variable from [lys_node_inout](@ref lys_node_inout)*/
    uint8_t tpdf_size() {return ((struct lys_node_inout *)node)->tpdf_size;};
    /** get must_size variable from [lys_node_inout](@ref lys_node_inout)*/
    uint8_t must_size() {return ((struct lys_node_inout *)node)->must_size;};
    /** get tpdf variable from [lys_node_inout](@ref lys_node_inout)*/
    std::vector<S_Tpdf> tpdf();
    /** get must variable from [lys_node_inout](@ref lys_node_inout)*/
    std::vector<S_Restr> must();

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Notif : public Schema_Node
{
public:
    Schema_Node_Notif(S_Schema_Node derived):
        Schema_Node(derived->node, derived->deleter),
        node(derived->node),
        deleter(derived->deleter)
    {
        if (derived->node->nodetype != LYS_NOTIF) {
            throw std::invalid_argument("Type must be LYS_NOTIF");
        }
    };
    /** wrapper for struct [lys_node_notif](@ref lys_node_notif), for internal use only */
    Schema_Node_Notif(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Notif();
    /** get tpdf_size variable from [lys_node_notif](@ref lys_node_notif)*/
    uint8_t tpdf_size() {return ((struct lys_node_notif *)node)->tpdf_size;};
    /** get must_size variable from [lys_node_notif](@ref lys_node_notif)*/
    uint8_t must_size() {return ((struct lys_node_notif *)node)->must_size;};
    /** get tpdf variable from [lys_node_notif](@ref lys_node_notif)*/
    std::vector<S_Tpdf> tpdf();
    /** get must variable from [lys_node_notif](@ref lys_node_notif)*/
    std::vector<S_Restr> must();

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Rpc_Action : public Schema_Node
{
public:
    Schema_Node_Rpc_Action(S_Schema_Node derived):
        Schema_Node(derived->node, derived->deleter),
        node(derived->node),
        deleter(derived->deleter)
    {
        if (derived->node->nodetype != LYS_ACTION && derived->node->nodetype != LYS_RPC) {
            throw std::invalid_argument("Type must be LYS_ACTION or LYS_RPC");
        }
    };
    /** wrapper for struct [lys_node_rpc_action](@ref lys_node_rpc_action), for internal use only */
    Schema_Node_Rpc_Action(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Rpc_Action();
    /** get tpdf_size variable from [lys_node_rpc_action](@ref lys_node_rpc_action)*/
    uint8_t tpdf_size() {return ((struct lys_node_rpc_action *)node)->tpdf_size;};
    /** get tpdf variable from [lys_node_rpc_action](@ref lys_node_rpc_action)*/
    std::vector<S_Tpdf> tpdf();

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Schema_Node_Augment : public Schema_Node
{
public:
    Schema_Node_Augment(S_Schema_Node derived):
        Schema_Node(derived->node, derived->deleter),
        node(derived->node),
        deleter(derived->deleter)
    {
        if (derived->node->nodetype != LYS_AUGMENT) {
            throw std::invalid_argument("Type must be LYS_AUGMENT");
        }
    };
    /** wrapper for struct [lys_node_augment](@ref lys_node_augment), for internal use only */
    Schema_Node_Augment(struct lys_node *node, S_Deleter deleter):
        Schema_Node(node, deleter),
        node(node),
        deleter(deleter)
    {};
    ~Schema_Node_Augment();
    /** get when variable from [lys_node_augment](@ref lys_node_augment)*/
    S_When when();
    /** get target variable from [lys_node_augment](@ref lys_node_augment)*/
    S_Schema_Node target() LY_NEW_CASTED(lys_node_augment, node, target, Schema_Node);

private:
    struct lys_node *node;
    S_Deleter deleter;
};

class Substmt
{
public:
    /** wrapper for struct [lyext_substmt](@ref lyext_substmt), for internal use only */
    Substmt(struct lyext_substmt *substmt, S_Deleter deleter);
    ~Substmt();
    /** get stmt variable from [lyext_substmt](@ref lyext_substmt)*/
    LY_STMT stmt() {return substmt->stmt;};
    /** get offset variable from [lyext_substmt](@ref lyext_substmt)*/
    size_t offset() {return substmt->offset;};
    /** get cardinality variable from [lyext_substmt](@ref lyext_substmt)*/
    LY_STMT_CARD cardinality() {return substmt->cardinality;};
private:
    struct lyext_substmt *substmt;
    S_Deleter deleter;
};

class Ext
{
public:
    /** wrapper for struct [lys_ext](@ref lys_ext), for internal use only */
    Ext(struct lys_ext *ext, S_Deleter deleter);
    ~Ext();
    /** get name variable from [lys_ext](@ref lys_ext)*/
    const char *name() {return ext->name;};
    /** get dsc variable from [lys_ext](@ref lys_ext)*/
    const char *dsc() {return ext->dsc;};
    /** get ref variable from [lys_ext](@ref lys_ext)*/
    const char *ref() {return ext->ref;};
    /** get flags variable from [lys_ext](@ref lys_ext)*/
    uint16_t flags() {return ext->flags;};
    /** get ext_size variable from [lys_ext](@ref lys_ext)*/
    uint8_t ext_size() {return ext->ext_size;};
    /** get ext_instance variable from [lys_ext](@ref lys_ext)*/
    std::vector<S_Ext_Instance> ext_instance();
    /** get argument variable from [lys_ext](@ref lys_ext)*/
    const char *argument() {return ext->argument;};
    /** get module variable from [lys_ext](@ref lys_ext)*/
    S_Module module();
    //struct lyext_plugin *plugin;
private:
    struct lys_ext *ext;
    S_Deleter deleter;
};

class Refine_Mod_List
{
public:
    /** wrapper for struct [lys_refine_mod_list](@ref lys_refine_mod_list), for internal use only */
    Refine_Mod_List(struct lys_refine_mod_list *list, S_Deleter deleter);
    ~Refine_Mod_List();
    /** get min variable from [lys_refine_mod_list](@ref lys_refine_mod_list)*/
    uint32_t min() {return list->min;};
    /** get max variable from [lys_refine_mod_list](@ref lys_refine_mod_list)*/
    uint32_t max() {return list->max;};

private:
    struct lys_refine_mod_list *list;
    S_Deleter deleter;
};

class Refine_Mod
{
public:
    /** wrapper for struct [lys_refine_mod](@ref lys_refine_mod), for internal use only */
    Refine_Mod(union lys_refine_mod mod, uint16_t target_type, S_Deleter deleter);
    ~Refine_Mod();
    /** get presence variable from [lys_refine_mod](@ref lys_refine_mod)*/
    const char *presence() {return target_type == LYS_CONTAINER ? mod.presence : nullptr;};
    /** get list variable from [lys_refine_mod](@ref lys_refine_mod)*/
    S_Refine_Mod_List list();

private:
    union lys_refine_mod mod;
    uint16_t target_type;
    S_Deleter deleter;
};

class Refine
{
public:
    /** wrapper for struct [lys_refine](@ref lys_refine), for internal use only */
    Refine(struct lys_refine *refine, S_Deleter deleter);
    ~Refine();
    /** get target_name variable from [lys_refine](@ref lys_refine)*/
    const char *target_name() {return refine->target_name;};
    /** get dsc variable from [lys_refine](@ref lys_refine)*/
    const char *dsc() {return refine->dsc;};
    /** get ref variable from [lys_refine](@ref lys_refine)*/
    const char *ref() {return refine->ref;};
    /** get flags variable from [lys_refine](@ref lys_refine)*/
    uint16_t flags() {return refine->flags;};
    /** get ext_size variable from [lys_refine](@ref lys_refine)*/
    uint8_t ext_size() {return refine->ext_size;};
    /** get iffeature_size variable from [lys_refine](@ref lys_refine)*/
    uint8_t iffeature_size() {return refine->iffeature_size;};
    /** get target_type variable from [lys_refine](@ref lys_refine)*/
    uint16_t target_type() {return refine->target_type;};
    /** get must_size variable from [lys_refine](@ref lys_refine)*/
    uint8_t must_size() {return refine->must_size;};
    /** get dflt_size variable from [lys_refine](@ref lys_refine)*/
    uint8_t dflt_size() {return refine->dflt_size;};
    /** get ext variable from [lys_refine](@ref lys_refine)*/
    std::vector<S_Ext_Instance> ext();
    /** get iffeature variable from [lys_refine](@ref lys_refine)*/
    std::vector<S_Iffeature> iffeature() LY_NEW_LIST(refine, iffeature, iffeature_size, Iffeature);
    /** get module variable from [lys_refine](@ref lys_refine)*/
    S_Module module();
    /** get must variable from [lys_refine](@ref lys_refine)*/
    std::vector<S_Restr> must();
    /** get dflt variable from [lys_refine](@ref lys_refine)*/
    std::vector<std::string> dflt() LY_NEW_STRING_LIST(refine, dflt, dflt_size);
    /** get mod variable from [lys_refine](@ref lys_refine)*/
    S_Refine_Mod mod();

private:
    struct lys_refine *refine;
    S_Deleter deleter;
};

class Deviate
{
public:
    /** wrapper for struct [lys_deviate](@ref lys_deviate), for internal use only */
    Deviate(struct lys_deviate *deviate, S_Deleter deleter);
    ~Deviate();
    /** get mod variable from [lys_deviate](@ref lys_deviate)*/
    LYS_DEVIATE_TYPE mod() {return deviate->mod;};
    /** get flags variable from [lys_deviate](@ref lys_deviate)*/
    uint8_t flags() {return deviate->flags;};
    /** get dflt_size variable from [lys_deviate](@ref lys_deviate)*/
    uint8_t dflt_size() {return deviate->dflt_size;};
    /** get ext_size variable from [lys_deviate](@ref lys_deviate)*/
    uint8_t ext_size() {return deviate->ext_size;};
    /** get min_set variable from [lys_deviate](@ref lys_deviate)*/
    uint8_t min_set() {return deviate->min_set;};
    /** get max_set variable from [lys_deviate](@ref lys_deviate)*/
    uint8_t max_set() {return deviate->max_set;};
    /** get must_size variable from [lys_deviate](@ref lys_deviate)*/
    uint8_t must_size() {return deviate->must_size;};
    /** get unique_size variable from [lys_deviate](@ref lys_deviate)*/
    uint8_t unique_size() {return deviate->unique_size;};
    /** get min variable from [lys_deviate](@ref lys_deviate)*/
    uint32_t min() {return deviate->min;};
    /** get max variable from [lys_deviate](@ref lys_deviate)*/
    uint32_t max() {return deviate->max;};
    /** get must variable from [lys_deviate](@ref lys_deviate)*/
    S_Restr must();
    /** get unique variable from [lys_deviate](@ref lys_deviate)*/
    S_Unique unique();
    /** get type variable from [lys_deviate](@ref lys_deviate)*/
    S_Type type();
    /** get units variable from [lys_deviate](@ref lys_deviate)*/
    const char *units() {return deviate->units;};
    /** get dflt variable from [lys_deviate](@ref lys_deviate)*/
    std::vector<std::string> dflt() LY_NEW_STRING_LIST(deviate, dflt, dflt_size);
    /** get ext variable from [lys_deviate](@ref lys_deviate)*/
    std::vector<S_Ext_Instance> ext();

private:
    struct lys_deviate *deviate;
    S_Deleter deleter;
};

class Deviation
{
public:
    /** wrapper for struct [lys_deviation](@ref lys_deviation), for internal use only */
    Deviation(struct lys_deviation *deviation, S_Deleter deleter);
    ~Deviation();
    /** get target_name variable from [lys_deviation](@ref lys_deviation)*/
    const char *target_name() {return deviation->target_name;};
    /** get dsc variable from [lys_deviation](@ref lys_deviation)*/
    const char *dsc() {return deviation->dsc;};
    /** get ref variable from [lys_deviation](@ref lys_deviation)*/
    const char *ref() {return deviation->ref;};
    /** get orig_node variable from [lys_deviation](@ref lys_deviation)*/
    S_Schema_Node orig_node();
    /** get deviate_size variable from [lys_deviation](@ref lys_deviation)*/
    uint8_t deviate_size() {return deviation->deviate_size;};
    /** get ext_size variable from [lys_deviation](@ref lys_deviation)*/
    uint8_t ext_size() {return deviation->ext_size;};
    /** get deviate variable from [lys_deviation](@ref lys_deviation)*/
    std::vector<S_Deviate> deviate();
    /** get ext variable from [lys_deviation](@ref lys_deviation)*/
    std::vector<S_Ext_Instance> ext();

private:
    struct lys_deviation *deviation;
    S_Deleter deleter;
};

class Import
{
public:
    /** wrapper for struct [lys_import](@ref lys_import), for internal use only */
    Import(struct lys_import *import, S_Deleter deleter);
    ~Import();
    /** get module variable from [lys_import](@ref lys_import)*/
    S_Module module() LY_NEW(import, module, Module);
    /** get prefix variable from [lys_import](@ref lys_import)*/
    const char *prefix() {return import->prefix;};
    /** get rev variable from [lys_import](@ref lys_import)*/
    char *rev() {return &import->rev[0];};
    /** get ext_size variable from [lys_import](@ref lys_import)*/
    uint8_t ext_size() {return import->ext_size;};
    /** get ext variable from [lys_import](@ref lys_import)*/
    std::vector<S_Ext_Instance> ext() LY_NEW_P_LIST(import, ext, ext_size, Ext_Instance);
    /** get dsc variable from [lys_import](@ref lys_import)*/
    const char *dsc() {return import->dsc;};
    /** get ref variable from [lys_import](@ref lys_import)*/
    const char *ref() {return import->ref;};

private:
    struct lys_import *import;
    S_Deleter deleter;
};

class Include
{
public:
    /** wrapper for struct [lys_include](@ref lys_include), for internal use only */
    Include(struct lys_include *include, S_Deleter deleter);
    ~Include();
    /** get submodule variable from [lys_include](@ref lys_include)*/
    S_Submodule submodule() LY_NEW(include, submodule, Submodule);
    /** get rev variable from [lys_include](@ref lys_include)*/
    char *rev() {return &include->rev[0];};
    /** get ext_size variable from [lys_include](@ref lys_include)*/
    uint8_t ext_size() {return include->ext_size;};
    /** get ext variable from [lys_include](@ref lys_include)*/
    std::vector<S_Ext_Instance> ext() LY_NEW_P_LIST(include, ext, ext_size, Ext_Instance);
    /** get dsc variable from [lys_include](@ref lys_include)*/
    const char *dsc() {return include->dsc;};
    /** get ref variable from [lys_include](@ref lys_include)*/
    const char *ref() {return include->ref;};

private:
    struct lys_include *include;
    S_Deleter deleter;
};

class Revision
{
public:
    /** wrapper for struct [lys_revision](@ref lys_revision), for internal use only */
    Revision(lys_revision *revision, S_Deleter deleter);
    ~Revision();
    /** get date variable from [lys_revision](@ref lys_revision)*/
    char *date() {return &revision->date[0];};
    /** get ext_size variable from [lys_revision](@ref lys_revision)*/
    uint8_t ext_size() {return revision->ext_size;};
    /** get dsc variable from [lys_revision](@ref lys_revision)*/
    const char *dsc() {return revision->dsc;};
    /** get ref variable from [lys_revision](@ref lys_revision)*/
    const char *ref() {return revision->ref;};

private:
    struct lys_revision *revision;
    S_Deleter deleter;
};

class Tpdf
{
public:
    /** wrapper for struct [lys_tpdf](@ref lys_tpdf), for internal use only */
    Tpdf(struct lys_tpdf *tpdf, S_Deleter deleter);
    ~Tpdf();
    /** get name variable from [lys_tpdf](@ref lys_tpdf)*/
    const char *name() {return tpdf->name;};
    /** get dsc variable from [lys_tpdf](@ref lys_tpdf)*/
    const char *dsc() {return tpdf->dsc;};
    /** get ref variable from [lys_tpdf](@ref lys_tpdf)*/
    const char *ref() {return tpdf->ref;};
    /** get flags variable from [lys_tpdf](@ref lys_tpdf)*/
    uint16_t flags() {return tpdf->flags;};
    /** get ext_size variable from [lys_tpdf](@ref lys_tpdf)*/
    uint8_t ext_size() {return tpdf->ext_size;};
    /** get padding_iffsize variable from [lys_tpdf](@ref lys_tpdf)*/
    uint8_t padding_iffsize() {return tpdf->padding_iffsize;};
    /** get has_union_leafref variable from [lys_tpdf](@ref lys_tpdf)*/
    uint8_t has_union_leafref() {return tpdf->has_union_leafref;};
    /** get ext variable from [lys_tpdf](@ref lys_tpdf)*/
    std::vector<S_Ext_Instance> ext() LY_NEW_P_LIST(tpdf, ext, ext_size, Ext_Instance);
    /** get units variable from [lys_tpdf](@ref lys_tpdf)*/
    const char *units() {return tpdf->units;};
    /** get module variable from [lys_tpdf](@ref lys_tpdf)*/
    S_Module module() LY_NEW(tpdf, module, Module);
    /** get type variable from [lys_tpdf](@ref lys_tpdf)*/
    S_Type type();
    /** get dflt variable from [lys_tpdf](@ref lys_tpdf)*/
    const char *dflt() {return tpdf->dflt;};

private:
    struct lys_tpdf *tpdf;
    S_Deleter deleter;
};

class Unique
{
public:
    /** wrapper for struct [lys_unique](@ref lys_unique), for internal use only */
    Unique(struct lys_unique *unique, S_Deleter deleter);
    ~Unique();
    /** get expr variable from [lys_unique](@ref lys_unique)*/
    std::vector<std::string> expr() LY_NEW_STRING_LIST(unique, expr, expr_size);
    /** get expr_size variable from [lys_unique](@ref lys_unique)*/
    uint8_t expr_size() {return unique->expr_size;};
    /** get trg_type variable from [lys_unique](@ref lys_unique)*/
    uint8_t trg_type() {return unique->trg_type;};

private:
    struct lys_unique *unique;
    S_Deleter deleter;
};

class Feature
{
public:
    /** wrapper for struct [lys_feature](@ref lys_feature), for internal use only */
    Feature(struct lys_feature *feature, S_Deleter);
    ~Feature();
    /** get name variable from [lys_feature](@ref lys_feature)*/
    const char *name() {return feature->name;};
    /** get dsc variable from [lys_feature](@ref lys_feature)*/
    const char *dsc() {return feature->dsc;};
    /** get ref variable from [lys_feature](@ref lys_feature)*/
    const char *ref() {return feature->ref;};
    /** get flags variable from [lys_feature](@ref lys_feature)*/
    uint16_t flags() {return feature->flags;};
    /** get ext_size variable from [lys_feature](@ref lys_feature)*/
    uint8_t ext_size() {return feature->ext_size;};
    /** get iffeature_size variable from [lys_feature](@ref lys_feature)*/
    uint8_t iffeature_size() {return feature->iffeature_size;};
    /** get ext variable from [lys_feature](@ref lys_feature)*/
    std::vector<S_Ext_Instance> ext() LY_NEW_P_LIST(feature, ext, ext_size, Ext_Instance);
    /** get iffeature variable from [lys_feature](@ref lys_feature)*/
    std::vector<S_Iffeature> iffeature() LY_NEW_LIST(feature, iffeature, iffeature_size, Iffeature);
    /** get module variable from [lys_feature](@ref lys_feature)*/
    S_Module module() LY_NEW(feature, module, Module);
    /** get depfeatures variable from [lys_feature](@ref lys_feature)*/
    S_Set depfeatures() LY_NEW(feature, depfeatures, Set);

private:
    struct lys_feature *feature;
    S_Deleter deleter;
};

class Restr
{
public:
    /** wrapper for struct [lys_restr](@ref lys_restr), for internal use only */
    Restr(struct lys_restr *restr, S_Deleter deleter);
    ~Restr();
    /** get expr variable from [lys_restr](@ref lys_restr)*/
    const char *expr() {return restr->expr;};
    /** get dsc variable from [lys_restr](@ref lys_restr)*/
    const char *dsc() {return restr->dsc;};
    /** get ref variable from [lys_restr](@ref lys_restr)*/
    const char *ref() {return restr->ref;};
    /** get eapptag variable from [lys_restr](@ref lys_restr)*/
    const char *eapptag() {return restr->eapptag;};
    /** get emsg variable from [lys_restr](@ref lys_restr)*/
    const char *emsg() {return restr->emsg;};
    /** get ext variable from [lys_restr](@ref lys_restr)*/
    std::vector<S_Ext_Instance> ext() LY_NEW_P_LIST(restr, ext, ext_size, Ext_Instance);
    /** get ext_size variable from [lys_restr](@ref lys_restr)*/
    uint8_t ext_size() {return restr->ext_size;};

private:
    struct lys_restr *restr;
    S_Deleter deleter;
};

class When
{
public:
    /** wrapper for struct [lys_when](@ref lys_when), for internal use only */
    When(struct lys_when *when, S_Deleter deleter = nullptr);
    ~When();
    /** get cond variable from [lys_when](@ref lys_when)*/
    const char *cond() {return when->cond;};
    /** get dsc variable from [lys_when](@ref lys_when)*/
    const char *dsc() {return when->dsc;};
    /** get ref variable from [lys_when](@ref lys_when)*/
    const char *ref() {return when->ref;};
    /** get ext variable from [lys_when](@ref lys_when)*/
    std::vector<S_Ext_Instance> ext();
    /** get ext_size variable from [lys_when](@ref lys_when)*/
    uint8_t ext_size() {return when->ext_size;};

private:
    struct lys_when *when;
    S_Deleter deleter;
};

class Ident
{
public:
    /** wrapper for struct [lys_ident](@ref lys_ident), for internal use only */
    Ident(struct lys_ident *ident, S_Deleter deleter);
    ~Ident();
    /** get name variable from [lys_ident](@ref lys_ident)*/
    const char *name() {return ident->name;};
    /** get dsc variable from [lys_ident](@ref lys_ident)*/
    const char *dsc() {return ident->dsc;};
    /** get ref variable from [lys_ident](@ref lys_ident)*/
    const char *ref() {return ident->ref;};
    /** get flags variable from [lys_ident](@ref lys_ident)*/
    uint16_t flags() {return ident->flags;};
    /** get ext_size variable from [lys_ident](@ref lys_ident)*/
    uint8_t ext_size() {return ident->ext_size;};
    /** get iffeature_size variable from [lys_ident](@ref lys_ident)*/
    uint8_t iffeature_size() {return ident->iffeature_size;};
    /** get base_size variable from [lys_ident](@ref lys_ident)*/
    uint8_t base_size() {return ident->base_size;};
    /** get ext variable from [lys_ident](@ref lys_ident)*/
    std::vector<S_Ext_Instance> ext() LY_NEW_P_LIST(ident, ext, ext_size, Ext_Instance);
    /** get iffeature variable from [lys_ident](@ref lys_ident)*/
    std::vector<S_Iffeature> iffeature() LY_NEW_LIST(ident, iffeature, iffeature_size, Iffeature);
    /** get module variable from [lys_ident](@ref lys_ident)*/
    S_Module module() LY_NEW(ident, module, Module);
    /** get base variable from [lys_ident](@ref lys_ident)*/
    std::vector<S_Ident> base();
    /** get der variable from [lys_ident](@ref lys_ident)*/
    S_Set der() LY_NEW(ident, der, Set);

private:
   struct lys_ident *ident;
    S_Deleter deleter;
};

/**@} */

}

#endif
