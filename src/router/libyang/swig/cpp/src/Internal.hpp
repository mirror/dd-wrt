/**
 * @file Internal.hpp
 * @author Mislav Novakovic <mislav.novakovic@sartura.hr>
 * @brief Internal C++ helper class
 *
 * Copyright (c) 2017 Deutsche Telekom AG.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LIBYANG_CPP_INTERNAL_H
#define LIBYANG_CPP_INTERNAL_H

#include <iostream>
#include <memory>
#include <vector>

extern "C" {
#include "libyang.h"
}

namespace libyang {

#define LY_NEW(data, element, class)\
    {\
        return data->element ? std::make_shared<class>(data->element, deleter) : nullptr;\
    }

#define LY_NEW_CASTED(cast, data, element, class)\
    {\
        cast *casted = (struct cast *) data;\
        return casted->element ? std::make_shared<class>(casted->element, deleter) : nullptr;\
    }

#define LY_NEW_LIST(data, element, size, class)\
    {\
        std::vector<S_##class> s_vector;\
        if (0 >= data->size) {\
            return s_vector;\
        }\
        for (uint8_t i = 0; i < data->size; i++) {\
            s_vector.push_back(std::make_shared<class>(&data->element[i], deleter));\
        }\
        return s_vector;\
    }

#define LY_NEW_LIST_CASTED(cast, data, element, size, class)\
    {\
        struct cast *casted = (struct cast *) data;\
        LY_NEW_LIST(casted, element, size, class);\
    }

#define LY_NEW_P_LIST(data, element, size, class)\
    {\
        std::vector<S_##class> s_vector;\
        if (0 >= data->size) {\
            return s_vector;\
        }\
        for (uint8_t i = 0; i < data->size; i++) {\
            s_vector.push_back(std::make_shared<class>(data->element[i], deleter));\
        }\
        return s_vector;\
    }

#define LY_NEW_P_LIST_CASTED(cast, data, element, size, class)\
    {\
        struct cast *casted = (struct cast *) data;\
        LY_NEW_P_LIST(casted, element, size, class);\
    }

#define LY_NEW_STRING_LIST(data, element, size)\
    {\
        std::vector<std::string> s_vector;\
        if (0 >= data->size) {\
            return s_vector;\
        }\
        for (uint8_t i = 0; i < data->size; i++) {\
            s_vector.push_back(std::string(data->element[i]));\
        }\
        return s_vector;\
    }

/* defined */
class Deleter;
using S_Deleter = std::shared_ptr<Deleter>;

/* used */
class Context;
class Error;
class Set;

using S_Context = std::shared_ptr<Context>;
using S_Error = std::shared_ptr<Error>;
using S_Set = std::shared_ptr<Set>;

class Xml_Ns;
class Xml_Attr;
class Xml_Elem;

using S_Xml_Ns = std::shared_ptr<Xml_Ns>;
using S_Xml_Attr = std::shared_ptr<Xml_Attr>;
using S_Xml_Elem = std::shared_ptr<Xml_Elem>;

class Value;
class Data_Node;
class Data_Node_Leaf_List;
class Data_Node_Anydata;
class Attr;
class Difflist;

using S_Value = std::shared_ptr<Value>;
using S_Data_Node = std::shared_ptr<Data_Node>;
using S_Data_Node_Leaf_List = std::shared_ptr<Data_Node_Leaf_List>;
using S_Data_Node_Anydata = std::shared_ptr<Data_Node_Anydata>;
using S_Attr = std::shared_ptr<Attr>;
using S_Difflist = std::shared_ptr<Difflist>;

class Module;
class Submodule;
class Type_Bit;
class Type_Enum;
class Type_Info_Binary;
class Type_Info_Bits;
class Type_Info_Dec64;
class Type_Info_Enum;
class Type_Info_Enums;
class Type_Info_Ident;
class Type_Info_Inst;
class Type_Info_Num;
class Type_Info_Lref;
class Type_Info_Str;
class Type_Info_Union;
class Type_Info;
class Type;
class Iffeature;
class Ext_Instance;
class Schema_Node;
class Schema_Node_Container;
class Schema_Node_Choice;
class Schema_Node_Leaf;
class Schema_Node_Leaflist;
class Schema_Node_List;
class Schema_Node_Anydata;
class Schema_Node_Uses;
class Schema_Node_Grp;
class Schema_Node_Case;
class Schema_Node_Inout;
class Schema_Node_Notif;
class Schema_Node_Action;
class Schema_Node_Augment;
class Schema_Node_Rpc_Action;
class Substmt;
class Ext;
class Refine_Mod_List;
class Refine_Mod;
class Refine;
class Deviate;
class Deviation;
class Import;
class Include;
class Revision;
class Tpdf;
class Unique;
class Feature;
class Restr;
class When;
class Ident;

using S_Module = std::shared_ptr<Module>;
using S_Submodule = std::shared_ptr<Submodule>;
using S_Type_Info_Binary = std::shared_ptr<Type_Info_Binary>;
using S_Type_Bit = std::shared_ptr<Type_Bit>;
using S_Type_Info_Bits = std::shared_ptr<Type_Info_Bits>;
using S_Type_Info_Dec64 = std::shared_ptr<Type_Info_Dec64>;
using S_Type_Enum = std::shared_ptr<Type_Enum>;
using S_Type_Info_Enums = std::shared_ptr<Type_Info_Enums>;
using S_Type_Info_Ident = std::shared_ptr<Type_Info_Ident>;
using S_Type_Info_Inst = std::shared_ptr<Type_Info_Inst>;
using S_Type_Info_Num = std::shared_ptr<Type_Info_Num>;
using S_Type_Info_Num = std::shared_ptr<Type_Info_Num>;
using S_Type_Info_Lref = std::shared_ptr<Type_Info_Lref>;
using S_Type_Info_Str = std::shared_ptr<Type_Info_Str>;
using S_Type_Info_Union = std::shared_ptr<Type_Info_Union>;
using S_Type_Info = std::shared_ptr<Type_Info>;
using S_Type = std::shared_ptr<Type>;
using S_Iffeature = std::shared_ptr<Iffeature>;
using S_Ext_Instance = std::shared_ptr<Ext_Instance>;
using S_Revision = std::shared_ptr<Revision>;
using S_Schema_Node = std::shared_ptr<Schema_Node>;
using S_Schema_Node_Container = std::shared_ptr<Schema_Node_Container>;
using S_Schema_Node_Choice = std::shared_ptr<Schema_Node_Choice>;
using S_Schema_Node_Leaf = std::shared_ptr<Schema_Node_Leaf>;
using S_Schema_Node_Leaflist = std::shared_ptr<Schema_Node_Leaflist>;
using S_Schema_Node_List = std::shared_ptr<Schema_Node_List>;
using S_Schema_Node_Anydata = std::shared_ptr<Schema_Node_Anydata>;
using S_Schema_Node_Uses = std::shared_ptr<Schema_Node_Uses>;
using S_Schema_Node_Grp = std::shared_ptr<Schema_Node_Grp>;
using S_Schema_Node_Case = std::shared_ptr<Schema_Node_Case>;
using S_Schema_Node_Inout = std::shared_ptr<Schema_Node_Inout>;
using S_Schema_Node_Notif = std::shared_ptr<Schema_Node_Notif>;
using S_Schema_Node_Action = std::shared_ptr<Schema_Node_Action>;
using S_Schema_Node_Augment = std::shared_ptr<Schema_Node_Augment>;
using S_When = std::shared_ptr<When>;
using S_Substmt = std::shared_ptr<Substmt>;
using S_Ext = std::shared_ptr<Ext>;
using S_Refine_Mod_List = std::shared_ptr<Refine_Mod_List>;
using S_Refine_Mod = std::shared_ptr<Refine_Mod>;
using S_Refine = std::shared_ptr<Refine>;
using S_Deviate = std::shared_ptr<Deviate>;
using S_Deviation = std::shared_ptr<Deviation>;
using S_Import = std::shared_ptr<Import>;
using S_Include = std::shared_ptr<Include>;
using S_Tpdf = std::shared_ptr<Tpdf>;
using S_Unique = std::shared_ptr<Unique>;
using S_Feature = std::shared_ptr<Feature>;
using S_Restr = std::shared_ptr<Restr>;
using S_Ident = std::shared_ptr<Ident>;


void check_libyang_error(ly_ctx *ctx);

enum class Free_Type {
    CONTEXT,
    DATA_NODE,
    //TODO DATA_NODE_WITHSIBLINGS,
    SCHEMA_NODE,
    MODULE,
    SUBMODULE,
    XML,
    SET,
    DIFFLIST,
};

typedef union value_e {
    struct ly_ctx *ctx;
    struct lyd_node *data;
    struct lys_node *schema;
    struct lys_module *module;
    struct lys_submodule *submodule;
    struct lyxml_elem *elem;
    struct ly_set *set;
    struct lyd_difflist *diff;
} value_t;

class Deleter
{
public:
    Deleter(ly_ctx *ctx, S_Deleter parent = nullptr);
    Deleter(struct lyd_node *data, S_Deleter parent = nullptr);
    Deleter(struct lys_node *schema, S_Deleter parent = nullptr);
    Deleter(struct lys_module *module, S_Deleter parent = nullptr);
    Deleter(struct lys_submodule *submodule, S_Deleter parent = nullptr);
    Deleter(S_Context context, struct lyxml_elem *elem, S_Deleter parent = nullptr);
    Deleter(struct ly_set *set, S_Deleter parent = nullptr);
    Deleter(struct lyd_difflist *diff, S_Deleter parent = nullptr);
    ~Deleter();

private:
    S_Context context;
    value_t v;
    Free_Type t;
    S_Deleter parent;
};

}

#endif
