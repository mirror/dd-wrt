/**
 * @file tree_schema_free.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Freeing functions for schema tree structures.
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdlib.h>

#include "common.h"
#include "compat.h"
#include "dict.h"
#include "log.h"
#include "plugins_exts.h"
#include "plugins_types.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_edit.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xml.h"
#include "xpath.h"

void lysp_grp_free(struct ly_ctx *ctx, struct lysp_node_grp *grp);
static void lysc_node_free_(struct ly_ctx *ctx, struct lysc_node *node);

static void
lysp_stmt_free(struct ly_ctx *ctx, struct lysp_stmt *stmt)
{
    struct lysp_stmt *child, *next;

    lydict_remove(ctx, stmt->stmt);
    lydict_remove(ctx, stmt->arg);
    ly_free_prefix_data(stmt->format, stmt->prefix_data);

    LY_LIST_FOR_SAFE(stmt->child, next, child) {
        lysp_stmt_free(ctx, child);
    }

    free(stmt);
}

void
lysp_ext_instance_free(struct ly_ctx *ctx, struct lysp_ext_instance *ext)
{
    struct lysp_stmt *stmt, *next;
    struct lysp_node *node, *next_node;

    lydict_remove(ctx, ext->name);
    lydict_remove(ctx, ext->argument);
    ly_free_prefix_data(ext->format, ext->prefix_data);
    LY_LIST_FOR_SAFE(ext->parsed, next_node, node) {
        lysp_node_free(ctx, node);
    }

    LY_LIST_FOR_SAFE(ext->child, next, stmt) {
        lysp_stmt_free(ctx, stmt);
    }
}

void
lysp_import_free(struct ly_ctx *ctx, struct lysp_import *import)
{
    /* imported module is freed directly from the context's list */
    lydict_remove(ctx, import->name);
    lydict_remove(ctx, import->prefix);
    lydict_remove(ctx, import->dsc);
    lydict_remove(ctx, import->ref);
    FREE_ARRAY(ctx, import->exts, lysp_ext_instance_free);
}

/**
 * @brief Common function to erase include record in main module and submodule.
 *
 * There is a difference since the main module is expected to have the complete list if the included submodules and
 * the parsed submodule is shared with any include in a submodule. Therefore, the referenced submodules in the include
 * record are freed only from main module's records.
 *
 * @param[in] ctx libyang context
 * @param[in] include The include record to be erased, the record itself is not freed.
 * @param[in] main_module Flag to get know if the include record is placed in main module so also the referenced submodule
 * is supposed to be freed.
 */
static void
lysp_include_free_(struct ly_ctx *ctx, struct lysp_include *include, ly_bool main_module)
{
    if (main_module && include->submodule) {
        lysp_module_free((struct lysp_module *)include->submodule);
    }
    lydict_remove(ctx, include->name);
    lydict_remove(ctx, include->dsc);
    lydict_remove(ctx, include->ref);
    FREE_ARRAY(ctx, include->exts, lysp_ext_instance_free);
}

void
lysp_include_free_submodule(struct ly_ctx *ctx, struct lysp_include *include)
{
    return lysp_include_free_(ctx, include, 0);
}

void
lysp_include_free(struct ly_ctx *ctx, struct lysp_include *include)
{
    return lysp_include_free_(ctx, include, 1);
}

void
lysp_revision_free(struct ly_ctx *ctx, struct lysp_revision *rev)
{
    lydict_remove(ctx, rev->dsc);
    lydict_remove(ctx, rev->ref);
    FREE_ARRAY(ctx, rev->exts, lysp_ext_instance_free);
}

void
lysp_ext_free(struct ly_ctx *ctx, struct lysp_ext *ext)
{
    lydict_remove(ctx, ext->name);
    lydict_remove(ctx, ext->argname);
    lydict_remove(ctx, ext->dsc);
    lydict_remove(ctx, ext->ref);
    FREE_ARRAY(ctx, ext->exts, lysp_ext_instance_free);
    if (ext->compiled) {
        lysc_extension_free(ctx, &ext->compiled);
    }
}

void
lysp_feature_free(struct ly_ctx *ctx, struct lysp_feature *feat)
{
    lydict_remove(ctx, feat->name);
    FREE_ARRAY(ctx, feat->iffeatures, lysp_qname_free);
    FREE_ARRAY(ctx, feat->iffeatures_c, lysc_iffeature_free);
    LY_ARRAY_FREE(feat->depfeatures);
    lydict_remove(ctx, feat->dsc);
    lydict_remove(ctx, feat->ref);
    FREE_ARRAY(ctx, feat->exts, lysp_ext_instance_free);
}

void
lysp_ident_free(struct ly_ctx *ctx, struct lysp_ident *ident)
{
    lydict_remove(ctx, ident->name);
    FREE_ARRAY(ctx, ident->iffeatures, lysp_qname_free);
    FREE_STRINGS(ctx, ident->bases);
    lydict_remove(ctx, ident->dsc);
    lydict_remove(ctx, ident->ref);
    FREE_ARRAY(ctx, ident->exts, lysp_ext_instance_free);
}

void
lysp_restr_free(struct ly_ctx *ctx, struct lysp_restr *restr)
{
    lydict_remove(ctx, restr->arg.str);
    lydict_remove(ctx, restr->emsg);
    lydict_remove(ctx, restr->eapptag);
    lydict_remove(ctx, restr->dsc);
    lydict_remove(ctx, restr->ref);
    FREE_ARRAY(ctx, restr->exts, lysp_ext_instance_free);
}

static void
lysp_type_enum_free(struct ly_ctx *ctx, struct lysp_type_enum *item)
{
    lydict_remove(ctx, item->name);
    lydict_remove(ctx, item->dsc);
    lydict_remove(ctx, item->ref);
    FREE_ARRAY(ctx, item->iffeatures, lysp_qname_free);
    FREE_ARRAY(ctx, item->exts, lysp_ext_instance_free);
}

void lysc_type_free(struct ly_ctx *ctx, struct lysc_type *type);

void
lysp_type_free(struct ly_ctx *ctx, struct lysp_type *type)
{
    lydict_remove(ctx, type->name);
    FREE_MEMBER(ctx, type->range, lysp_restr_free);
    FREE_MEMBER(ctx, type->length, lysp_restr_free);
    FREE_ARRAY(ctx, type->patterns, lysp_restr_free);
    FREE_ARRAY(ctx, type->enums, lysp_type_enum_free);
    FREE_ARRAY(ctx, type->bits, lysp_type_enum_free);
    lyxp_expr_free(ctx, type->path);
    FREE_STRINGS(ctx, type->bases);
    FREE_ARRAY(ctx, type->types, lysp_type_free);
    FREE_ARRAY(ctx, type->exts, lysp_ext_instance_free);
    if (type->compiled) {
        lysc_type_free(ctx, type->compiled);
    }
}

void
lysp_tpdf_free(struct ly_ctx *ctx, struct lysp_tpdf *tpdf)
{
    lydict_remove(ctx, tpdf->name);
    lydict_remove(ctx, tpdf->units);
    lydict_remove(ctx, tpdf->dflt.str);
    lydict_remove(ctx, tpdf->dsc);
    lydict_remove(ctx, tpdf->ref);
    FREE_ARRAY(ctx, tpdf->exts, lysp_ext_instance_free);

    lysp_type_free(ctx, &tpdf->type);

}

void
lysp_grp_free(struct ly_ctx *ctx, struct lysp_node_grp *grp)
{
    struct lysp_node *node, *next;

    FREE_ARRAY(ctx, grp->typedefs, lysp_tpdf_free);
    LY_LIST_FOR_SAFE((struct lysp_node *)grp->groupings, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE(grp->child, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysp_node *)grp->actions, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysp_node *)grp->notifs, next, node) {
        lysp_node_free(ctx, node);
    }
}

void
lysp_when_free(struct ly_ctx *ctx, struct lysp_when *when)
{
    lydict_remove(ctx, when->cond);
    lydict_remove(ctx, when->dsc);
    lydict_remove(ctx, when->ref);
    FREE_ARRAY(ctx, when->exts, lysp_ext_instance_free);
}

void
lysp_augment_free(struct ly_ctx *ctx, struct lysp_node_augment *augment)
{
    struct lysp_node *node, *next;

    LY_LIST_FOR_SAFE(augment->child, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysp_node *)augment->actions, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysp_node *)augment->notifs, next, node) {
        lysp_node_free(ctx, node);
    }
}

void
lysp_qname_free(struct ly_ctx *ctx, struct lysp_qname *qname)
{
    if (qname) {
        lydict_remove(ctx, qname->str);
    }
}

void
lysp_deviate_free(struct ly_ctx *ctx, struct lysp_deviate *d)
{
    struct lysp_deviate_add *add = (struct lysp_deviate_add *)d;
    struct lysp_deviate_rpl *rpl = (struct lysp_deviate_rpl *)d;

    FREE_ARRAY(ctx, d->exts, lysp_ext_instance_free);
    switch (d->mod) {
    case LYS_DEV_NOT_SUPPORTED:
        /* nothing to do */
        break;
    case LYS_DEV_ADD:
    case LYS_DEV_DELETE: /* compatible for dynamically allocated data */
        lydict_remove(ctx, add->units);
        FREE_ARRAY(ctx, add->musts, lysp_restr_free);
        FREE_ARRAY(ctx, add->uniques, lysp_qname_free);
        FREE_ARRAY(ctx, add->dflts, lysp_qname_free);
        break;
    case LYS_DEV_REPLACE:
        FREE_MEMBER(ctx, rpl->type, lysp_type_free);
        lydict_remove(ctx, rpl->units);
        lysp_qname_free(ctx, &rpl->dflt);
        break;
    default:
        LOGINT(ctx);
        break;
    }
}

void
lysp_deviation_free(struct ly_ctx *ctx, struct lysp_deviation *dev)
{
    struct lysp_deviate *next, *iter;

    lydict_remove(ctx, dev->nodeid);
    lydict_remove(ctx, dev->dsc);
    lydict_remove(ctx, dev->ref);
    LY_LIST_FOR_SAFE(dev->deviates, next, iter) {
        lysp_deviate_free(ctx, iter);
        free(iter);
    }
    FREE_ARRAY(ctx, dev->exts, lysp_ext_instance_free);
}

void
lysp_refine_free(struct ly_ctx *ctx, struct lysp_refine *ref)
{
    lydict_remove(ctx, ref->nodeid);
    lydict_remove(ctx, ref->dsc);
    lydict_remove(ctx, ref->ref);
    FREE_ARRAY(ctx, ref->iffeatures, lysp_qname_free);
    FREE_ARRAY(ctx, ref->musts, lysp_restr_free);
    lydict_remove(ctx, ref->presence);
    FREE_ARRAY(ctx, ref->dflts, lysp_qname_free);
    FREE_ARRAY(ctx, ref->exts, lysp_ext_instance_free);
}

void
lysp_node_free(struct ly_ctx *ctx, struct lysp_node *node)
{
    struct lysp_node *child, *next;
    struct lysp_restr *musts = lysp_node_musts(node);
    struct lysp_when *when = lysp_node_when(node);

    lydict_remove(ctx, node->name);
    lydict_remove(ctx, node->dsc);
    lydict_remove(ctx, node->ref);
    FREE_ARRAY(ctx, node->iffeatures, lysp_qname_free);
    FREE_ARRAY(ctx, node->exts, lysp_ext_instance_free);

    FREE_MEMBER(ctx, when, lysp_when_free);
    FREE_ARRAY(ctx, musts, lysp_restr_free);

    switch (node->nodetype) {
    case LYS_CONTAINER:
        lydict_remove(ctx, ((struct lysp_node_container *)node)->presence);
        FREE_ARRAY(ctx, ((struct lysp_node_container *)node)->typedefs, lysp_tpdf_free);
        LY_LIST_FOR_SAFE(&((struct lysp_node_container *)node)->groupings->node, next, child) {
            lysp_node_free(ctx, child);
        }
        LY_LIST_FOR_SAFE(((struct lysp_node_container *)node)->child, next, child) {
            lysp_node_free(ctx, child);
        }
        LY_LIST_FOR_SAFE(&((struct lysp_node_container *)node)->actions->node, next, child) {
            lysp_node_free(ctx, child);
        }
        LY_LIST_FOR_SAFE(&((struct lysp_node_container *)node)->notifs->node, next, child) {
            lysp_node_free(ctx, child);
        }
        break;
    case LYS_LEAF:
        lysp_type_free(ctx, &((struct lysp_node_leaf *)node)->type);
        lydict_remove(ctx, ((struct lysp_node_leaf *)node)->units);
        lydict_remove(ctx, ((struct lysp_node_leaf *)node)->dflt.str);
        break;
    case LYS_LEAFLIST:
        lysp_type_free(ctx, &((struct lysp_node_leaflist *)node)->type);
        lydict_remove(ctx, ((struct lysp_node_leaflist *)node)->units);
        FREE_ARRAY(ctx, ((struct lysp_node_leaflist *)node)->dflts, lysp_qname_free);
        break;
    case LYS_LIST:
        lydict_remove(ctx, ((struct lysp_node_list *)node)->key);
        FREE_ARRAY(ctx, ((struct lysp_node_list *)node)->typedefs, lysp_tpdf_free);
        LY_LIST_FOR_SAFE(&((struct lysp_node_list *)node)->groupings->node, next, child) {
            lysp_node_free(ctx, child);
        }
        LY_LIST_FOR_SAFE(((struct lysp_node_list *)node)->child, next, child) {
            lysp_node_free(ctx, child);
        }
        LY_LIST_FOR_SAFE(&((struct lysp_node_list *)node)->actions->node, next, child) {
            lysp_node_free(ctx, child);
        }
        LY_LIST_FOR_SAFE(&((struct lysp_node_list *)node)->notifs->node, next, child) {
            lysp_node_free(ctx, child);
        }
        FREE_ARRAY(ctx, ((struct lysp_node_list *)node)->uniques, lysp_qname_free);
        break;
    case LYS_CHOICE:
        LY_LIST_FOR_SAFE(((struct lysp_node_choice *)node)->child, next, child) {
            lysp_node_free(ctx, child);
        }
        lydict_remove(ctx, ((struct lysp_node_choice *)node)->dflt.str);
        break;
    case LYS_CASE:
        LY_LIST_FOR_SAFE(((struct lysp_node_case *)node)->child, next, child) {
            lysp_node_free(ctx, child);
        }
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        /* nothing special to do */
        break;
    case LYS_USES:
        FREE_ARRAY(ctx, ((struct lysp_node_uses *)node)->refines, lysp_refine_free);
        LY_LIST_FOR_SAFE(&((struct lysp_node_uses *)node)->augments->node, next, child) {
            lysp_node_free(ctx, child);
        }
        break;
    case LYS_RPC:
    case LYS_ACTION:
        FREE_ARRAY(ctx, ((struct lysp_node_action *)node)->typedefs, lysp_tpdf_free);
        LY_LIST_FOR_SAFE(&((struct lysp_node_action *)node)->groupings->node, next, child) {
            lysp_node_free(ctx, child);
        }
        if (((struct lysp_node_action *)node)->input.nodetype) {
            lysp_node_free(ctx, &((struct lysp_node_action *)node)->input.node);
        }
        if (((struct lysp_node_action *)node)->output.nodetype) {
            lysp_node_free(ctx, &((struct lysp_node_action *)node)->output.node);
        }
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        FREE_ARRAY(ctx, ((struct lysp_node_action_inout *)node)->typedefs, lysp_tpdf_free);
        LY_LIST_FOR_SAFE(&((struct lysp_node_action_inout *)node)->groupings->node, next, child) {
            lysp_node_free(ctx, child);
        }
        LY_LIST_FOR_SAFE(((struct lysp_node_action_inout *)node)->child, next, child) {
            lysp_node_free(ctx, child);
        }
        /* do not free the node, it is never standalone but part of the action node */
        return;
    case LYS_NOTIF:
        FREE_ARRAY(ctx, ((struct lysp_node_notif *)node)->typedefs, lysp_tpdf_free);
        LY_LIST_FOR_SAFE(&((struct lysp_node_notif *)node)->groupings->node, next, child) {
            lysp_node_free(ctx, child);
        }
        LY_LIST_FOR_SAFE(((struct lysp_node_notif *)node)->child, next, child) {
            lysp_node_free(ctx, child);
        }
        break;
    case LYS_GROUPING:
        lysp_grp_free(ctx, (struct lysp_node_grp *)node);
        break;
    case LYS_AUGMENT:
        lysp_augment_free(ctx, ((struct lysp_node_augment *)node));
        break;
    default:
        LOGINT(ctx);
    }

    free(node);
}

void
lysp_module_free(struct lysp_module *module)
{
    struct ly_ctx *ctx;
    struct lysp_node *node, *next;

    if (!module) {
        return;
    }
    ctx = module->mod->ctx;

    FREE_ARRAY(ctx, module->imports, lysp_import_free);
    FREE_ARRAY(ctx, module->includes, module->is_submod ? lysp_include_free_submodule : lysp_include_free);

    FREE_ARRAY(ctx, module->revs, lysp_revision_free);
    FREE_ARRAY(ctx, module->extensions, lysp_ext_free);
    FREE_ARRAY(ctx, module->features, lysp_feature_free);
    FREE_ARRAY(ctx, module->identities, lysp_ident_free);
    FREE_ARRAY(ctx, module->typedefs, lysp_tpdf_free);
    LY_LIST_FOR_SAFE((struct lysp_node *)module->groupings, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE(module->data, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysp_node *)module->augments, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysp_node *)module->rpcs, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysp_node *)module->notifs, next, node) {
        lysp_node_free(ctx, node);
    }
    FREE_ARRAY(ctx, module->deviations, lysp_deviation_free);
    FREE_ARRAY(ctx, module->exts, lysp_ext_instance_free);

    if (module->is_submod) {
        struct lysp_submodule *submod = (struct lysp_submodule *)module;

        lydict_remove(ctx, submod->name);
        lydict_remove(ctx, submod->filepath);
        lydict_remove(ctx, submod->prefix);
        lydict_remove(ctx, submod->org);
        lydict_remove(ctx, submod->contact);
        lydict_remove(ctx, submod->dsc);
        lydict_remove(ctx, submod->ref);
    }

    free(module);
}

void
lysc_extension_free(struct ly_ctx *ctx, struct lysc_ext **ext)
{
    if (--(*ext)->refcount) {
        return;
    }
    lydict_remove(ctx, (*ext)->name);
    lydict_remove(ctx, (*ext)->argname);
    FREE_ARRAY(ctx, (*ext)->exts, lysc_ext_instance_free);
    free(*ext);
    *ext = NULL;
}

void
lysc_ext_instance_free(struct ly_ctx *ctx, struct lysc_ext_instance *ext)
{
    if (ext->def && ext->def->plugin && ext->def->plugin->free) {
        ext->def->plugin->free(ctx, ext);
    }
    if (ext->def) {
        lysc_extension_free(ctx, &ext->def);
    }
    lydict_remove(ctx, ext->argument);
    FREE_ARRAY(ctx, ext->exts, lysc_ext_instance_free);
}

void
lysc_iffeature_free(struct ly_ctx *UNUSED(ctx), struct lysc_iffeature *iff)
{
    LY_ARRAY_FREE(iff->features);
    free(iff->expr);
}

static void
lysc_when_free(struct ly_ctx *ctx, struct lysc_when **w)
{
    if (--(*w)->refcount) {
        return;
    }
    lyxp_expr_free(ctx, (*w)->cond);
    ly_free_prefix_data(LY_VALUE_SCHEMA_RESOLVED, (*w)->prefixes);
    lydict_remove(ctx, (*w)->dsc);
    lydict_remove(ctx, (*w)->ref);
    FREE_ARRAY(ctx, (*w)->exts, lysc_ext_instance_free);
    free(*w);
}

void
lysc_must_free(struct ly_ctx *ctx, struct lysc_must *must)
{
    lyxp_expr_free(ctx, must->cond);
    ly_free_prefix_data(LY_VALUE_SCHEMA_RESOLVED, must->prefixes);
    lydict_remove(ctx, must->emsg);
    lydict_remove(ctx, must->eapptag);
    lydict_remove(ctx, must->dsc);
    lydict_remove(ctx, must->ref);
    FREE_ARRAY(ctx, must->exts, lysc_ext_instance_free);
}

void
lysc_ident_free(struct ly_ctx *ctx, struct lysc_ident *ident)
{
    lydict_remove(ctx, ident->name);
    lydict_remove(ctx, ident->dsc);
    lydict_remove(ctx, ident->ref);
    LY_ARRAY_FREE(ident->derived);
    FREE_ARRAY(ctx, ident->exts, lysc_ext_instance_free);
}

static void
lysc_range_free(struct ly_ctx *ctx, struct lysc_range *range)
{
    LY_ARRAY_FREE(range->parts);
    lydict_remove(ctx, range->eapptag);
    lydict_remove(ctx, range->emsg);
    lydict_remove(ctx, range->dsc);
    lydict_remove(ctx, range->ref);
    FREE_ARRAY(ctx, range->exts, lysc_ext_instance_free);
}

static void
lysc_pattern_free(struct ly_ctx *ctx, struct lysc_pattern **pattern)
{
    if (--(*pattern)->refcount) {
        return;
    }
    pcre2_code_free((*pattern)->code);
    lydict_remove(ctx, (*pattern)->expr);
    lydict_remove(ctx, (*pattern)->eapptag);
    lydict_remove(ctx, (*pattern)->emsg);
    lydict_remove(ctx, (*pattern)->dsc);
    lydict_remove(ctx, (*pattern)->ref);
    FREE_ARRAY(ctx, (*pattern)->exts, lysc_ext_instance_free);
    free(*pattern);
}

static void
lysc_enum_item_free(struct ly_ctx *ctx, struct lysc_type_bitenum_item *item)
{
    lydict_remove(ctx, item->name);
    lydict_remove(ctx, item->dsc);
    lydict_remove(ctx, item->ref);
    FREE_ARRAY(ctx, item->exts, lysc_ext_instance_free);
}

static void
lysc_type2_free(struct ly_ctx *ctx, struct lysc_type **type)
{
    lysc_type_free(ctx, *type);
}

void
lysc_type_free(struct ly_ctx *ctx, struct lysc_type *type)
{
    if (--type->refcount) {
        return;
    }

    switch (type->basetype) {
    case LY_TYPE_BINARY:
        FREE_MEMBER(ctx, ((struct lysc_type_bin *)type)->length, lysc_range_free);
        break;
    case LY_TYPE_BITS:
        FREE_ARRAY(ctx, (struct lysc_type_bitenum_item *)((struct lysc_type_bits *)type)->bits, lysc_enum_item_free);
        break;
    case LY_TYPE_DEC64:
        FREE_MEMBER(ctx, ((struct lysc_type_dec *)type)->range, lysc_range_free);
        break;
    case LY_TYPE_STRING:
        FREE_MEMBER(ctx, ((struct lysc_type_str *)type)->length, lysc_range_free);
        FREE_ARRAY(ctx, ((struct lysc_type_str *)type)->patterns, lysc_pattern_free);
        break;
    case LY_TYPE_ENUM:
        FREE_ARRAY(ctx, ((struct lysc_type_enum *)type)->enums, lysc_enum_item_free);
        break;
    case LY_TYPE_INT8:
    case LY_TYPE_UINT8:
    case LY_TYPE_INT16:
    case LY_TYPE_UINT16:
    case LY_TYPE_INT32:
    case LY_TYPE_UINT32:
    case LY_TYPE_INT64:
    case LY_TYPE_UINT64:
        FREE_MEMBER(ctx, ((struct lysc_type_num *)type)->range, lysc_range_free);
        break;
    case LY_TYPE_IDENT:
        LY_ARRAY_FREE(((struct lysc_type_identityref *)type)->bases);
        break;
    case LY_TYPE_UNION:
        FREE_ARRAY(ctx, ((struct lysc_type_union *)type)->types, lysc_type2_free);
        break;
    case LY_TYPE_LEAFREF:
        lyxp_expr_free(ctx, ((struct lysc_type_leafref *)type)->path);
        ly_free_prefix_data(LY_VALUE_SCHEMA_RESOLVED, ((struct lysc_type_leafref *)type)->prefixes);
        break;
    case LY_TYPE_INST:
    case LY_TYPE_BOOL:
    case LY_TYPE_EMPTY:
    case LY_TYPE_UNKNOWN:
        /* nothing to do */
        break;
    }

    FREE_ARRAY(ctx, type->exts, lysc_ext_instance_free);
    free(type);
}

void
lysc_node_action_inout_free(struct ly_ctx *ctx, struct lysc_node_action_inout *inout)
{
    struct lysc_node *child, *child_next;

    FREE_ARRAY(ctx, inout->musts, lysc_must_free);
    LY_LIST_FOR_SAFE(inout->child, child_next, child) {
        lysc_node_free_(ctx, child);
    }
}

void
lysc_node_action_free(struct ly_ctx *ctx, struct lysc_node_action *action)
{
    FREE_ARRAY(ctx, action->when, lysc_when_free);
    if (action->input.nodetype) {
        lysc_node_free_(ctx, &action->input.node);
    }
    if (action->output.nodetype) {
        lysc_node_free_(ctx, &action->output.node);
    }
}

void
lysc_node_notif_free(struct ly_ctx *ctx, struct lysc_node_notif *notif)
{
    struct lysc_node *child, *child_next;

    FREE_ARRAY(ctx, notif->when, lysc_when_free);
    FREE_ARRAY(ctx, notif->musts, lysc_must_free);
    LY_LIST_FOR_SAFE(notif->child, child_next, child) {
        lysc_node_free_(ctx, child);
    }
}

void
lysc_node_container_free(struct ly_ctx *ctx, struct lysc_node_container *node)
{
    struct lysc_node *child, *child_next;

    LY_LIST_FOR_SAFE(node->child, child_next, child) {
        lysc_node_free_(ctx, child);
    }
    LY_LIST_FOR_SAFE((struct lysc_node *)node->actions, child_next, child) {
        lysc_node_free_(ctx, child);
    }
    LY_LIST_FOR_SAFE((struct lysc_node *)node->notifs, child_next, child) {
        lysc_node_free_(ctx, child);
    }
    FREE_ARRAY(ctx, node->when, lysc_when_free);
    FREE_ARRAY(ctx, node->musts, lysc_must_free);
}

static void
lysc_node_leaf_free(struct ly_ctx *ctx, struct lysc_node_leaf *node)
{
    FREE_ARRAY(ctx, node->when, lysc_when_free);
    FREE_ARRAY(ctx, node->musts, lysc_must_free);
    if (node->type) {
        lysc_type_free(ctx, node->type);
    }
    lydict_remove(ctx, node->units);
    if (node->dflt) {
        node->dflt->realtype->plugin->free(ctx, node->dflt);
        lysc_type_free(ctx, (struct lysc_type *)node->dflt->realtype);
        free(node->dflt);
    }
}

static void
lysc_node_leaflist_free(struct ly_ctx *ctx, struct lysc_node_leaflist *node)
{
    LY_ARRAY_COUNT_TYPE u;

    FREE_ARRAY(ctx, node->when, lysc_when_free);
    FREE_ARRAY(ctx, node->musts, lysc_must_free);
    if (node->type) {
        lysc_type_free(ctx, node->type);
    }
    lydict_remove(ctx, node->units);
    LY_ARRAY_FOR(node->dflts, u) {
        node->dflts[u]->realtype->plugin->free(ctx, node->dflts[u]);
        lysc_type_free(ctx, (struct lysc_type *)node->dflts[u]->realtype);
        free(node->dflts[u]);
    }
    LY_ARRAY_FREE(node->dflts);
}

static void
lysc_node_list_free(struct ly_ctx *ctx, struct lysc_node_list *node)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_node *child, *child_next;

    LY_LIST_FOR_SAFE(node->child, child_next, child) {
        lysc_node_free_(ctx, child);
    }
    FREE_ARRAY(ctx, node->when, lysc_when_free);
    FREE_ARRAY(ctx, node->musts, lysc_must_free);

    LY_ARRAY_FOR(node->uniques, u) {
        LY_ARRAY_FREE(node->uniques[u]);
    }
    LY_ARRAY_FREE(node->uniques);

    LY_LIST_FOR_SAFE((struct lysc_node *)node->actions, child_next, child) {
        lysc_node_free_(ctx, child);
    }
    LY_LIST_FOR_SAFE((struct lysc_node *)node->notifs, child_next, child) {
        lysc_node_free_(ctx, child);
    }
}

static void
lysc_node_choice_free(struct ly_ctx *ctx, struct lysc_node_choice *node)
{
    struct lysc_node *child, *child_next;

    FREE_ARRAY(ctx, node->when, lysc_when_free);
    LY_LIST_FOR_SAFE((struct lysc_node *)node->cases, child_next, child) {
        lysc_node_free_(ctx, child);
    }
}

static void
lysc_node_case_free(struct ly_ctx *ctx, struct lysc_node_case *node)
{
    struct lysc_node *child, *child_next;

    FREE_ARRAY(ctx, node->when, lysc_when_free);
    LY_LIST_FOR_SAFE(node->child, child_next, child) {
        lysc_node_free_(ctx, child);
    }
}

static void
lysc_node_anydata_free(struct ly_ctx *ctx, struct lysc_node_anydata *node)
{
    FREE_ARRAY(ctx, node->when, lysc_when_free);
    FREE_ARRAY(ctx, node->musts, lysc_must_free);
}

static void
lysc_node_free_(struct ly_ctx *ctx, struct lysc_node *node)
{
    ly_bool inout = 0;

    /* common part */
    lydict_remove(ctx, node->name);
    lydict_remove(ctx, node->dsc);
    lydict_remove(ctx, node->ref);

    /* nodetype-specific part */
    switch (node->nodetype) {
    case LYS_CONTAINER:
        lysc_node_container_free(ctx, (struct lysc_node_container *)node);
        break;
    case LYS_LEAF:
        lysc_node_leaf_free(ctx, (struct lysc_node_leaf *)node);
        break;
    case LYS_LEAFLIST:
        lysc_node_leaflist_free(ctx, (struct lysc_node_leaflist *)node);
        break;
    case LYS_LIST:
        lysc_node_list_free(ctx, (struct lysc_node_list *)node);
        break;
    case LYS_CHOICE:
        lysc_node_choice_free(ctx, (struct lysc_node_choice *)node);
        break;
    case LYS_CASE:
        lysc_node_case_free(ctx, (struct lysc_node_case *)node);
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        lysc_node_anydata_free(ctx, (struct lysc_node_anydata *)node);
        break;
    case LYS_RPC:
    case LYS_ACTION:
        lysc_node_action_free(ctx, (struct lysc_node_action *)node);
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        lysc_node_action_inout_free(ctx, (struct lysc_node_action_inout *)node);
        inout = 1;
        break;
    case LYS_NOTIF:
        lysc_node_notif_free(ctx, (struct lysc_node_notif *)node);
        break;
    default:
        LOGINT(ctx);
    }

    FREE_ARRAY(ctx, node->exts, lysc_ext_instance_free);

    if (!inout) {
        free(node);
    }
}

void
lysc_node_free(struct ly_ctx *ctx, struct lysc_node *node, ly_bool unlink)
{
    struct lysc_node *iter, **child_p;

    if (node->nodetype & (LYS_INPUT | LYS_OUTPUT)) {
        /* nothing to do - inouts are part of actions and cannot be unlinked/freed separately */
        return;
    }

    if (unlink) {
        /* unlink from siblings */
        if (node->prev->next) {
            node->prev->next = node->next;
        }
        if (node->next) {
            node->next->prev = node->prev;
        } else {
            /* unlinking the last node */
            if (node->parent) {
                if (node->nodetype == LYS_ACTION) {
                    iter = (struct lysc_node *)lysc_node_actions(node->parent);
                } else if (node->nodetype == LYS_NOTIF) {
                    iter = (struct lysc_node *)lysc_node_notifs(node->parent);
                } else {
                    iter = (struct lysc_node *)lysc_node_child(node->parent);
                }
                LY_CHECK_ERR_RET(!iter, LOGINT(ctx), );
            } else if (node->nodetype == LYS_RPC) {
                iter = (struct lysc_node *)node->module->compiled->rpcs;
            } else if (node->nodetype == LYS_NOTIF) {
                iter = (struct lysc_node *)node->module->compiled->notifs;
            } else {
                iter = node->module->compiled->data;
            }
            /* update the "last" pointer from the first node */
            iter->prev = node->prev;
        }

        /* unlink from parent */
        if (node->parent) {
            if (node->nodetype == LYS_ACTION) {
                child_p = (struct lysc_node **)lysc_node_actions_p(node->parent);
            } else if (node->nodetype == LYS_NOTIF) {
                child_p = (struct lysc_node **)lysc_node_notifs_p(node->parent);
            } else {
                child_p = lysc_node_child_p(node->parent);
            }
        } else if (node->nodetype == LYS_RPC) {
            child_p = (struct lysc_node **)&node->module->compiled->rpcs;
        } else if (node->nodetype == LYS_NOTIF) {
            child_p = (struct lysc_node **)&node->module->compiled->notifs;
        } else {
            child_p = &node->module->compiled->data;
        }
        if (child_p && (*child_p == node)) {
            /* the node is the first child */
            *child_p = node->next;
        }
    }

    lysc_node_free_(ctx, node);
}

void
lysc_module_free(struct lysc_module *module)
{
    struct ly_ctx *ctx;
    struct lysc_node *node, *node_next;

    if (!module) {
        return;
    }

    ctx = module->mod->ctx;

    LY_LIST_FOR_SAFE(module->data, node_next, node) {
        lysc_node_free_(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysc_node *)module->rpcs, node_next, node) {
        lysc_node_free_(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysc_node *)module->notifs, node_next, node) {
        lysc_node_free_(ctx, node);
    }
    FREE_ARRAY(ctx, module->exts, lysc_ext_instance_free);

    free(module);
}

void
lys_module_free(struct lys_module *module)
{
    if (!module) {
        return;
    }

    lysc_module_free(module->compiled);
    FREE_ARRAY(module->ctx, module->identities, lysc_ident_free);
    lysp_module_free(module->parsed);

    LY_ARRAY_FREE(module->augmented_by);
    LY_ARRAY_FREE(module->deviated_by);

    lydict_remove(module->ctx, module->name);
    lydict_remove(module->ctx, module->revision);
    lydict_remove(module->ctx, module->ns);
    lydict_remove(module->ctx, module->prefix);
    lydict_remove(module->ctx, module->filepath);
    lydict_remove(module->ctx, module->org);
    lydict_remove(module->ctx, module->contact);
    lydict_remove(module->ctx, module->dsc);
    lydict_remove(module->ctx, module->ref);

    free(module);
}

API void
lyplg_ext_instance_substatements_free(struct ly_ctx *ctx, struct lysc_ext_substmt *substmts)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(substmts, u) {
        if (!substmts[u].storage) {
            continue;
        }

        switch (substmts[u].stmt) {
        case LY_STMT_ACTION:
        case LY_STMT_ANYDATA:
        case LY_STMT_ANYXML:
        case LY_STMT_CONTAINER:
        case LY_STMT_CHOICE:
        case LY_STMT_LEAF:
        case LY_STMT_LEAF_LIST:
        case LY_STMT_LIST:
        case LY_STMT_NOTIFICATION:
        case LY_STMT_RPC:
        case LY_STMT_USES: {
            struct lysc_node *child, *child_next;

            LY_LIST_FOR_SAFE(*((struct lysc_node **)substmts[u].storage), child_next, child) {
                lysc_node_free_(ctx, child);
            }
            break;
        }
        case LY_STMT_CONFIG:
        case LY_STMT_STATUS:
            /* nothing to do */
            break;
        case LY_STMT_DESCRIPTION:
        case LY_STMT_REFERENCE:
        case LY_STMT_UNITS:
            if (substmts[u].cardinality < LY_STMT_CARD_SOME) {
                /* single item */
                const char *str = *((const char **)substmts[u].storage);
                if (!str) {
                    break;
                }
                lydict_remove(ctx, str);
            } else {
                /* multiple items */
                const char **strs = *((const char ***)substmts[u].storage);
                if (!strs) {
                    break;
                }
                FREE_STRINGS(ctx, strs);
            }
            break;
        case LY_STMT_IF_FEATURE: {
            struct lysc_iffeature *iff = *((struct lysc_iffeature **)substmts[u].storage);
            if (!iff) {
                break;
            }
            if (substmts[u].cardinality < LY_STMT_CARD_SOME) {
                /* single item */
                lysc_iffeature_free(ctx, iff);
                free(iff);
            } else {
                /* multiple items */
                FREE_ARRAY(ctx, iff, lysc_iffeature_free);
            }
            break;
        case LY_STMT_TYPE:
            if (substmts[u].cardinality < LY_STMT_CARD_SOME) {
                /* single item */
                struct lysc_type *type = *((struct lysc_type **)substmts[u].storage);
                if (!type) {
                    break;
                }
                lysc_type_free(ctx, type);
            } else {
                /* multiple items */
                struct lysc_type **types = *((struct lysc_type ***)substmts[u].storage);
                if (!types) {
                    break;
                }
                FREE_ARRAY(ctx, types, lysc_type2_free);
            }
            break;
        }

        /* TODO other statements */
        default:
            LOGINT(ctx);
        }
    }

    LY_ARRAY_FREE(substmts);
}

void
yang_parser_ctx_free(struct lys_yang_parser_ctx *ctx)
{
    if (ctx) {
        free(ctx);
    }
}

void
yin_parser_ctx_free(struct lys_yin_parser_ctx *ctx)
{
    if (ctx) {
        lyxml_ctx_free(ctx->xmlctx);
        free(ctx);
    }
}
