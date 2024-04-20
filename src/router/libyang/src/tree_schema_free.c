/**
 * @file tree_schema_free.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Freeing functions for schema tree structures.
 *
 * Copyright (c) 2019 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "tree_schema_free.h"

#include <assert.h>
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

static void lysc_node_free_(struct lysf_ctx *ctx, struct lysc_node *node);

void
lysp_qname_free(const struct ly_ctx *ctx, struct lysp_qname *qname)
{
    if (qname) {
        lydict_remove(ctx, qname->str);
    }
}

/**
 * @brief Free the parsed generic statement structure.
 *
 * @param[in] ctx libyang context.
 * @param[in] grp Parsed schema statement structure to free. Note that the structure itself is not freed.
 */
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
lysp_ext_instance_free(struct lysf_ctx *ctx, struct lysp_ext_instance *ext)
{
    struct lysp_stmt *stmt, *next;

    lydict_remove(ctx->ctx, ext->name);
    lydict_remove(ctx->ctx, ext->argument);
    ly_free_prefix_data(ext->format, ext->prefix_data);
    if (ext->record && ext->record->plugin.pfree) {
        ext->record->plugin.pfree(ctx->ctx, ext);
    }

    LY_LIST_FOR_SAFE(ext->child, next, stmt) {
        lysp_stmt_free(ctx->ctx, stmt);
    }
}

/**
 * @brief Free the parsed import structure.
 *
 * @param[in] ctx Free context.
 * @param[in] import Parsed schema import structure to free. Note that the structure itself is not freed.
 */
static void
lysp_import_free(struct lysf_ctx *ctx, struct lysp_import *import)
{
    /* imported module is freed directly from the context's list */
    lydict_remove(ctx->ctx, import->name);
    lydict_remove(ctx->ctx, import->prefix);
    lydict_remove(ctx->ctx, import->dsc);
    lydict_remove(ctx->ctx, import->ref);
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
lysp_include_free_(struct lysf_ctx *ctx, struct lysp_include *include, ly_bool main_module)
{
    if (main_module && include->submodule) {
        lysp_module_free(ctx, (struct lysp_module *)include->submodule);
    }
    lydict_remove(ctx->ctx, include->name);
    lydict_remove(ctx->ctx, include->dsc);
    lydict_remove(ctx->ctx, include->ref);
    FREE_ARRAY(ctx, include->exts, lysp_ext_instance_free);
}

/**
 * @brief Free the parsed include structure of a submodule.
 *
 * @param[in] ctx Free context.
 * @param[in] include Parsed schema include structure to free. Note that the structure itself is not freed.
 */
static void
lysp_include_free_submodule(struct lysf_ctx *ctx, struct lysp_include *include)
{
    lysp_include_free_(ctx, include, 0);
}

/**
 * @brief Free the parsed include structure of a module.
 *
 * @param[in] ctx Free context.
 * @param[in] include Parsed schema include structure to free. Note that the structure itself is not freed.
 */
static void
lysp_include_free(struct lysf_ctx *ctx, struct lysp_include *include)
{
    lysp_include_free_(ctx, include, 1);
}

/**
 * @brief Free the parsed revision structure.
 *
 * @param[in] ctx Free context.
 * @param[in] rev Parsed schema revision structure to free. Note that the structure itself is not freed.
 */
static void
lysp_revision_free(struct lysf_ctx *ctx, struct lysp_revision *rev)
{
    lydict_remove(ctx->ctx, rev->dsc);
    lydict_remove(ctx->ctx, rev->ref);
    FREE_ARRAY(ctx, rev->exts, lysp_ext_instance_free);
}

/**
 * @brief Free the compiled extension definition and NULL the provided pointer.
 *
 * @param[in] ctx Free context.
 * @param[in,out] ext Compiled extension definition to be freed.
 */
static void
lysc_extension_free(struct lysf_ctx *ctx, struct lysc_ext **ext)
{
    if (ly_set_contains(&ctx->ext_set, *ext, NULL)) {
        /* already freed and only referenced again in this module */
        return;
    }

    /* remember this extension to be freed, nothing to do on error */
    (void)ly_set_add(&ctx->ext_set, *ext, 0, NULL);

    /* recursive exts free */
    FREE_ARRAY(ctx, (*ext)->exts, lysc_ext_instance_free);

    *ext = NULL;
}

/**
 * @brief Free the parsed ext structure.
 *
 * @param[in] ctx Free context.
 * @param[in] ext Parsed schema ext structure to free. Note that the structure itself is not freed.
 */
static void
lysp_ext_free(struct lysf_ctx *ctx, struct lysp_ext *ext)
{
    lydict_remove(ctx->ctx, ext->name);
    lydict_remove(ctx->ctx, ext->argname);
    lydict_remove(ctx->ctx, ext->dsc);
    lydict_remove(ctx->ctx, ext->ref);
    FREE_ARRAY(ctx, ext->exts, lysp_ext_instance_free);
    if (ext->compiled) {
        lysc_extension_free(ctx, &ext->compiled);
    }
}

/**
 * @brief Free the parsed feature structure.
 *
 * @param[in] ctx Free context.
 * @param[in] feat Parsed schema feature structure to free. Note that the structure itself is not freed.
 */
static void
lysp_feature_free(struct lysf_ctx *ctx, struct lysp_feature *feat)
{
    lydict_remove(ctx->ctx, feat->name);
    FREE_ARRAY(ctx->ctx, feat->iffeatures, lysp_qname_free);
    FREE_ARRAY(ctx, feat->iffeatures_c, lysc_iffeature_free);
    LY_ARRAY_FREE(feat->depfeatures);
    lydict_remove(ctx->ctx, feat->dsc);
    lydict_remove(ctx->ctx, feat->ref);
    FREE_ARRAY(ctx, feat->exts, lysp_ext_instance_free);
}

/**
 * @brief Free the parsed identity structure.
 *
 * @param[in] ctx Free context.
 * @param[in] ident Parsed schema identity structure to free. Note that the structure itself is not freed.
 */
static void
lysp_ident_free(struct lysf_ctx *ctx, struct lysp_ident *ident)
{
    lydict_remove(ctx->ctx, ident->name);
    FREE_ARRAY(ctx->ctx, ident->iffeatures, lysp_qname_free);
    FREE_STRINGS(ctx->ctx, ident->bases);
    lydict_remove(ctx->ctx, ident->dsc);
    lydict_remove(ctx->ctx, ident->ref);
    FREE_ARRAY(ctx, ident->exts, lysp_ext_instance_free);
}

void
lysp_restr_free(struct lysf_ctx *ctx, struct lysp_restr *restr)
{
    lydict_remove(ctx->ctx, restr->arg.str);
    lydict_remove(ctx->ctx, restr->emsg);
    lydict_remove(ctx->ctx, restr->eapptag);
    lydict_remove(ctx->ctx, restr->dsc);
    lydict_remove(ctx->ctx, restr->ref);
    FREE_ARRAY(ctx, restr->exts, lysp_ext_instance_free);
}

/**
 * @brief Free the parsed type enum item.
 *
 * @param[in] ctx Free context.
 * @param[in] item Parsed schema type enum item to free. Note that the structure itself is not freed.
 */
static void
lysp_type_enum_free(struct lysf_ctx *ctx, struct lysp_type_enum *item)
{
    lydict_remove(ctx->ctx, item->name);
    lydict_remove(ctx->ctx, item->dsc);
    lydict_remove(ctx->ctx, item->ref);
    FREE_ARRAY(ctx->ctx, item->iffeatures, lysp_qname_free);
    FREE_ARRAY(ctx, item->exts, lysp_ext_instance_free);
}

void
lysp_type_free(struct lysf_ctx *ctx, struct lysp_type *type)
{
    if (!type) {
        return;
    }

    lydict_remove(ctx->ctx, type->name);
    FREE_MEMBER(ctx, type->range, lysp_restr_free);
    FREE_MEMBER(ctx, type->length, lysp_restr_free);
    FREE_ARRAY(ctx, type->patterns, lysp_restr_free);
    FREE_ARRAY(ctx, type->enums, lysp_type_enum_free);
    FREE_ARRAY(ctx, type->bits, lysp_type_enum_free);
    lyxp_expr_free(ctx->ctx, type->path);
    FREE_STRINGS(ctx->ctx, type->bases);
    FREE_ARRAY(ctx, type->types, lysp_type_free);
    FREE_ARRAY(ctx, type->exts, lysp_ext_instance_free);
    if (type->compiled) {
        lysc_type_free(ctx, type->compiled);
    }
}

/**
 * @brief Free the parsed typedef structure.
 *
 * @param[in] ctx Free context.
 * @param[in] tpdf Parsed schema typedef structure to free. Note that the structure itself is not freed.
 */
static void
lysp_tpdf_free(struct lysf_ctx *ctx, struct lysp_tpdf *tpdf)
{
    lydict_remove(ctx->ctx, tpdf->name);
    lydict_remove(ctx->ctx, tpdf->units);
    lydict_remove(ctx->ctx, tpdf->dflt.str);
    lydict_remove(ctx->ctx, tpdf->dsc);
    lydict_remove(ctx->ctx, tpdf->ref);
    FREE_ARRAY(ctx, tpdf->exts, lysp_ext_instance_free);

    lysp_type_free(ctx, &tpdf->type);

}

/**
 * @brief Free the parsed grouping structure.
 *
 * @param[in] ctx Free context.
 * @param[in] grp Parsed schema grouping structure to free. Note that the structure itself is not freed.
 */
static void
lysp_grp_free(struct lysf_ctx *ctx, struct lysp_node_grp *grp)
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
lysp_when_free(struct lysf_ctx *ctx, struct lysp_when *when)
{
    lydict_remove(ctx->ctx, when->cond);
    lydict_remove(ctx->ctx, when->dsc);
    lydict_remove(ctx->ctx, when->ref);
    FREE_ARRAY(ctx, when->exts, lysp_ext_instance_free);
}

/**
 * @brief Free the parsed augment structure.
 *
 * @param[in] ctx Free context.
 * @param[in] aug Parsed schema augment structure to free. Note that the structure itself is not freed.
 */
static void
lysp_augment_free(struct lysf_ctx *ctx, struct lysp_node_augment *aug)
{
    struct lysp_node *node, *next;

    LY_LIST_FOR_SAFE(aug->child, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysp_node *)aug->actions, next, node) {
        lysp_node_free(ctx, node);
    }
    LY_LIST_FOR_SAFE((struct lysp_node *)aug->notifs, next, node) {
        lysp_node_free(ctx, node);
    }
}

void
lysp_deviate_free(struct lysf_ctx *ctx, struct lysp_deviate *d)
{
    struct lysp_deviate_add *add = (struct lysp_deviate_add *)d;
    struct lysp_deviate_rpl *rpl = (struct lysp_deviate_rpl *)d;

    if (!d) {
        return;
    }

    FREE_ARRAY(ctx, d->exts, lysp_ext_instance_free);
    switch (d->mod) {
    case LYS_DEV_NOT_SUPPORTED:
        /* nothing to do */
        break;
    case LYS_DEV_ADD:
    case LYS_DEV_DELETE: /* compatible for dynamically allocated data */
        lydict_remove(ctx->ctx, add->units);
        FREE_ARRAY(ctx, add->musts, lysp_restr_free);
        FREE_ARRAY(ctx->ctx, add->uniques, lysp_qname_free);
        FREE_ARRAY(ctx->ctx, add->dflts, lysp_qname_free);
        break;
    case LYS_DEV_REPLACE:
        FREE_MEMBER(ctx, rpl->type, lysp_type_free);
        lydict_remove(ctx->ctx, rpl->units);
        lysp_qname_free(ctx->ctx, &rpl->dflt);
        break;
    default:
        LOGINT(ctx->ctx);
        break;
    }
}

void
lysp_deviation_free(struct lysf_ctx *ctx, struct lysp_deviation *dev)
{
    struct lysp_deviate *next, *iter;

    lydict_remove(ctx->ctx, dev->nodeid);
    lydict_remove(ctx->ctx, dev->dsc);
    lydict_remove(ctx->ctx, dev->ref);
    LY_LIST_FOR_SAFE(dev->deviates, next, iter) {
        lysp_deviate_free(ctx, iter);
        free(iter);
    }
    FREE_ARRAY(ctx, dev->exts, lysp_ext_instance_free);
}

/**
 * @brief Free the parsed refine structure.
 *
 * @param[in] ctx Free context.
 * @param[in] ref Parsed schema refine structure to free. Note that the structure itself is not freed.
 */
static void
lysp_refine_free(struct lysf_ctx *ctx, struct lysp_refine *ref)
{
    lydict_remove(ctx->ctx, ref->nodeid);
    lydict_remove(ctx->ctx, ref->dsc);
    lydict_remove(ctx->ctx, ref->ref);
    FREE_ARRAY(ctx->ctx, ref->iffeatures, lysp_qname_free);
    FREE_ARRAY(ctx, ref->musts, lysp_restr_free);
    lydict_remove(ctx->ctx, ref->presence);
    FREE_ARRAY(ctx->ctx, ref->dflts, lysp_qname_free);
    FREE_ARRAY(ctx, ref->exts, lysp_ext_instance_free);
}

void
lysp_node_free(struct lysf_ctx *ctx, struct lysp_node *node)
{
    struct lysp_node *child, *next;
    struct lysp_node_container *cont;
    struct lysp_node_leaf *leaf;
    struct lysp_node_leaflist *llist;
    struct lysp_node_list *list;
    struct lysp_node_choice *choice;
    struct lysp_node_case *cas;
    struct lysp_node_uses *uses;
    struct lysp_node_action *act;
    struct lysp_node_action_inout *inout;
    struct lysp_node_notif *notif;
    struct lysp_restr *musts = lysp_node_musts(node);
    struct lysp_when *when = lysp_node_when(node);

    lydict_remove(ctx->ctx, node->name);
    lydict_remove(ctx->ctx, node->dsc);
    lydict_remove(ctx->ctx, node->ref);
    FREE_ARRAY(ctx->ctx, node->iffeatures, lysp_qname_free);
    FREE_ARRAY(ctx, node->exts, lysp_ext_instance_free);

    FREE_MEMBER(ctx, when, lysp_when_free);
    FREE_ARRAY(ctx, musts, lysp_restr_free);

    switch (node->nodetype) {
    case LYS_CONTAINER:
        cont = (struct lysp_node_container *)node;

        lydict_remove(ctx->ctx, cont->presence);
        FREE_ARRAY(ctx, cont->typedefs, lysp_tpdf_free);
        if (cont->groupings) {
            LY_LIST_FOR_SAFE(&cont->groupings->node, next, child) {
                lysp_node_free(ctx, child);
            }
        }
        LY_LIST_FOR_SAFE(cont->child, next, child) {
            lysp_node_free(ctx, child);
        }
        if (cont->actions) {
            LY_LIST_FOR_SAFE(&cont->actions->node, next, child) {
                lysp_node_free(ctx, child);
            }
        }
        if (cont->notifs) {
            LY_LIST_FOR_SAFE(&cont->notifs->node, next, child) {
                lysp_node_free(ctx, child);
            }
        }
        break;
    case LYS_LEAF:
        leaf = (struct lysp_node_leaf *)node;

        lysp_type_free(ctx, &leaf->type);
        lydict_remove(ctx->ctx, leaf->units);
        lydict_remove(ctx->ctx, leaf->dflt.str);
        break;
    case LYS_LEAFLIST:
        llist = (struct lysp_node_leaflist *)node;

        lysp_type_free(ctx, &llist->type);
        lydict_remove(ctx->ctx, llist->units);
        FREE_ARRAY(ctx->ctx, llist->dflts, lysp_qname_free);
        break;
    case LYS_LIST:
        list = (struct lysp_node_list *)node;

        lydict_remove(ctx->ctx, list->key);
        FREE_ARRAY(ctx, list->typedefs, lysp_tpdf_free);
        if (list->groupings) {
            LY_LIST_FOR_SAFE(&list->groupings->node, next, child) {
                lysp_node_free(ctx, child);
            }
        }
        LY_LIST_FOR_SAFE(list->child, next, child) {
            lysp_node_free(ctx, child);
        }
        if (list->actions) {
            LY_LIST_FOR_SAFE(&list->actions->node, next, child) {
                lysp_node_free(ctx, child);
            }
        }
        if (list->notifs) {
            LY_LIST_FOR_SAFE(&list->notifs->node, next, child) {
                lysp_node_free(ctx, child);
            }
        }
        FREE_ARRAY(ctx->ctx, list->uniques, lysp_qname_free);
        break;
    case LYS_CHOICE:
        choice = (struct lysp_node_choice *)node;

        LY_LIST_FOR_SAFE(choice->child, next, child) {
            lysp_node_free(ctx, child);
        }
        lydict_remove(ctx->ctx, choice->dflt.str);
        break;
    case LYS_CASE:
        cas = (struct lysp_node_case *)node;

        LY_LIST_FOR_SAFE(cas->child, next, child) {
            lysp_node_free(ctx, child);
        }
        break;
    case LYS_ANYDATA:
    case LYS_ANYXML:
        /* nothing special to do */
        break;
    case LYS_USES:
        uses = (struct lysp_node_uses *)node;

        FREE_ARRAY(ctx, uses->refines, lysp_refine_free);
        if (uses->augments) {
            LY_LIST_FOR_SAFE(&uses->augments->node, next, child) {
                lysp_node_free(ctx, child);
            }
        }
        break;
    case LYS_RPC:
    case LYS_ACTION:
        act = (struct lysp_node_action *)node;

        FREE_ARRAY(ctx, act->typedefs, lysp_tpdf_free);
        if (act->groupings) {
            LY_LIST_FOR_SAFE(&act->groupings->node, next, child) {
                lysp_node_free(ctx, child);
            }
        }
        if (act->input.nodetype) {
            lysp_node_free(ctx, &act->input.node);
        }
        if (act->output.nodetype) {
            lysp_node_free(ctx, &act->output.node);
        }
        break;
    case LYS_INPUT:
    case LYS_OUTPUT:
        inout = (struct lysp_node_action_inout *)node;

        FREE_ARRAY(ctx, inout->typedefs, lysp_tpdf_free);
        if (inout->groupings) {
            LY_LIST_FOR_SAFE(&inout->groupings->node, next, child) {
                lysp_node_free(ctx, child);
            }
        }
        LY_LIST_FOR_SAFE(inout->child, next, child) {
            lysp_node_free(ctx, child);
        }
        /* do not free the node, it is never standalone but part of the action node */
        return;
    case LYS_NOTIF:
        notif = (struct lysp_node_notif *)node;

        FREE_ARRAY(ctx, notif->typedefs, lysp_tpdf_free);
        if (notif->groupings) {
            LY_LIST_FOR_SAFE(&notif->groupings->node, next, child) {
                lysp_node_free(ctx, child);
            }
        }
        LY_LIST_FOR_SAFE(notif->child, next, child) {
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
        LOGINT(ctx->ctx);
    }

    free(node);
}

void
lysp_module_free(struct lysf_ctx *ctx, struct lysp_module *module)
{
    struct lysp_node *node, *next;

    if (!module) {
        return;
    }

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

        lydict_remove(ctx->ctx, submod->name);
        lydict_remove(ctx->ctx, submod->filepath);
        lydict_remove(ctx->ctx, submod->prefix);
        lydict_remove(ctx->ctx, submod->org);
        lydict_remove(ctx->ctx, submod->contact);
        lydict_remove(ctx->ctx, submod->dsc);
        lydict_remove(ctx->ctx, submod->ref);
    }

    free(module);
}

void
lysc_ext_instance_free(struct lysf_ctx *ctx, struct lysc_ext_instance *ext)
{
    if (ext->def && ext->def->plugin && ext->def->plugin->cfree) {
        ext->def->plugin->cfree(ctx->ctx, ext);
    }
    lydict_remove(ctx->ctx, ext->argument);
    FREE_ARRAY(ctx, ext->exts, lysc_ext_instance_free);
}

void
lysc_iffeature_free(struct lysf_ctx *UNUSED(ctx), struct lysc_iffeature *iff)
{
    LY_ARRAY_FREE(iff->features);
    free(iff->expr);
}

/**
 * @brief Free the compiled when structure (decrease refcount) and NULL the provided pointer.
 *
 * @param[in] ctx Free context.
 * @param[in] grp Parsed schema grouping structure to free. Note that the structure itself is not freed.
 */
static void
lysc_when_free(struct lysf_ctx *ctx, struct lysc_when **w)
{
    if (--(*w)->refcount) {
        return;
    }
    lyxp_expr_free(ctx->ctx, (*w)->cond);
    ly_free_prefix_data(LY_VALUE_SCHEMA_RESOLVED, (*w)->prefixes);
    lydict_remove(ctx->ctx, (*w)->dsc);
    lydict_remove(ctx->ctx, (*w)->ref);
    FREE_ARRAY(ctx, (*w)->exts, lysc_ext_instance_free);
    free(*w);
}

/**
 * @brief Free the compiled must structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] must Compiled must structure to be freed.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
static void
lysc_must_free(struct lysf_ctx *ctx, struct lysc_must *must)
{
    if (!must) {
        return;
    }

    lyxp_expr_free(ctx->ctx, must->cond);
    ly_free_prefix_data(LY_VALUE_SCHEMA_RESOLVED, must->prefixes);
    lydict_remove(ctx->ctx, must->emsg);
    lydict_remove(ctx->ctx, must->eapptag);
    lydict_remove(ctx->ctx, must->dsc);
    lydict_remove(ctx->ctx, must->ref);
    FREE_ARRAY(ctx, must->exts, lysc_ext_instance_free);
}

/**
 * @brief Unlink the identity from all derived identity arrays.
 *
 * @param[in] ident Identity to unlink.
 */
static void
lysc_ident_derived_unlink(const struct lysc_ident *ident)
{
    LY_ARRAY_COUNT_TYPE u, v, w;
    const struct lysp_submodule *submod;
    const struct lysp_module *base_pmod = NULL;
    const struct lysp_ident *identp = NULL;
    const struct lys_module *mod, *iter;
    const char *base_name;
    uint32_t i;

    /* find the parsed identity */
    LY_ARRAY_FOR(ident->module->parsed->identities, u) {
        if (ident->module->parsed->identities[u].name == ident->name) {
            identp = &ident->module->parsed->identities[u];
            base_pmod = ident->module->parsed;
            break;
        }
    }
    if (!identp) {
        LY_ARRAY_FOR(ident->module->parsed->includes, v) {
            submod = ident->module->parsed->includes[v].submodule;
            LY_ARRAY_FOR(submod->identities, u) {
                if (submod->identities[u].name == ident->name) {
                    identp = &submod->identities[u];
                    base_pmod = (struct lysp_module *)submod;
                    break;
                }
            }
        }
    }
    assert(identp);

    /* remove link from all the foreign bases, it may not be there if identity compilation failed */
    LY_ARRAY_FOR(identp->bases, u) {
        base_name = strchr(identp->bases[u], ':');
        if (!base_name) {
            continue;
        }

        /* prefixed identity */
        mod = ly_resolve_prefix(ident->module->ctx, identp->bases[u], base_name - identp->bases[u], LY_VALUE_SCHEMA,
                (void *)base_pmod);
        if (!mod) {
            continue;
        }
        ++base_name;

        i = 0;
        while ((iter = ly_ctx_get_module_iter(ident->module->ctx, &i))) {
            if (iter == mod) {
                break;
            }
        }
        if (!iter) {
            /* target module was freed already */
            continue;
        }

        /* find the compiled base */
        LY_ARRAY_FOR(mod->identities, v) {
            if (!strcmp(mod->identities[v].name, base_name)) {
                /* find the derived link */
                LY_ARRAY_FOR(mod->identities[v].derived, w) {
                    if (mod->identities[v].derived[w] == ident) {
                        /* remove the link */
                        LY_ARRAY_DECREMENT(mod->identities[v].derived);
                        if (!LY_ARRAY_COUNT(mod->identities[v].derived)) {
                            LY_ARRAY_FREE(mod->identities[v].derived);
                            mod->identities[v].derived = NULL;
                        } else if (w < LY_ARRAY_COUNT(mod->identities[v].derived)) {
                            memmove(mod->identities[v].derived + w, mod->identities[v].derived + w + 1,
                                    (LY_ARRAY_COUNT(mod->identities[v].derived) - w) * sizeof ident);
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
}

/**
 * @brief Free the compiled identity structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] ident Compiled identity structure to be freed.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
static void
lysc_ident_free(struct lysf_ctx *ctx, struct lysc_ident *ident)
{
    lydict_remove(ctx->ctx, ident->name);
    lydict_remove(ctx->ctx, ident->dsc);
    lydict_remove(ctx->ctx, ident->ref);
    LY_ARRAY_FREE(ident->derived);
    FREE_ARRAY(ctx, ident->exts, lysc_ext_instance_free);
}

/**
 * @brief Free the compiled range structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] range Compiled range structure to be freed.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
static void
lysc_range_free(struct lysf_ctx *ctx, struct lysc_range *range)
{
    LY_ARRAY_FREE(range->parts);
    lydict_remove(ctx->ctx, range->eapptag);
    lydict_remove(ctx->ctx, range->emsg);
    lydict_remove(ctx->ctx, range->dsc);
    lydict_remove(ctx->ctx, range->ref);
    FREE_ARRAY(ctx, range->exts, lysc_ext_instance_free);
}

void
lysc_pattern_free(struct lysf_ctx *ctx, struct lysc_pattern **pattern)
{
    if (--(*pattern)->refcount) {
        return;
    }
    pcre2_code_free((*pattern)->code);
    lydict_remove(ctx->ctx, (*pattern)->expr);
    lydict_remove(ctx->ctx, (*pattern)->eapptag);
    lydict_remove(ctx->ctx, (*pattern)->emsg);
    lydict_remove(ctx->ctx, (*pattern)->dsc);
    lydict_remove(ctx->ctx, (*pattern)->ref);
    FREE_ARRAY(ctx, (*pattern)->exts, lysc_ext_instance_free);
    free(*pattern);
}

void
lysc_enum_item_free(struct lysf_ctx *ctx, struct lysc_type_bitenum_item *item)
{
    lydict_remove(ctx->ctx, item->name);
    lydict_remove(ctx->ctx, item->dsc);
    lydict_remove(ctx->ctx, item->ref);
    FREE_ARRAY(ctx, item->exts, lysc_ext_instance_free);
}

/**
 * @brief Free the compiled type structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] type Pointer to compiled type structure to be freed.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
static void
lysc_type2_free(struct lysf_ctx *ctx, struct lysc_type **type)
{
    lysc_type_free(ctx, *type);
}

void
lysc_type_free(struct lysf_ctx *ctx, struct lysc_type *type)
{
    if (!type || (LY_ATOMIC_DEC_BARRIER(type->refcount) > 1)) {
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
        lyxp_expr_free(ctx->ctx, ((struct lysc_type_leafref *)type)->path);
        ly_free_prefix_data(LY_VALUE_SCHEMA_RESOLVED, ((struct lysc_type_leafref *)type)->prefixes);
        lysc_type_free(ctx, ((struct lysc_type_leafref *)type)->realtype);
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

/**
 * @brief Free the compiled input/output structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] inout Compiled inout structure to be freed.
 * Since the structure is part of the RPC/action structure, it is not freed itself.
 */
static void
lysc_node_action_inout_free(struct lysf_ctx *ctx, struct lysc_node_action_inout *inout)
{
    struct lysc_node *child, *child_next;

    FREE_ARRAY(ctx, inout->musts, lysc_must_free);
    LY_LIST_FOR_SAFE(inout->child, child_next, child) {
        lysc_node_free_(ctx, child);
    }
}

/**
 * @brief Free the compiled RPC/action structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] action Compiled RPC/action structure to be freed.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
static void
lysc_node_action_free(struct lysf_ctx *ctx, struct lysc_node_action *action)
{
    FREE_ARRAY(ctx, action->when, lysc_when_free);
    if (action->input.nodetype) {
        lysc_node_free_(ctx, &action->input.node);
    }
    if (action->output.nodetype) {
        lysc_node_free_(ctx, &action->output.node);
    }
}

/**
 * @brief Free the compiled notification structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] notif Compiled notification structure to be freed.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
static void
lysc_node_notif_free(struct lysf_ctx *ctx, struct lysc_node_notif *notif)
{
    struct lysc_node *child, *child_next;

    FREE_ARRAY(ctx, notif->when, lysc_when_free);
    FREE_ARRAY(ctx, notif->musts, lysc_must_free);
    LY_LIST_FOR_SAFE(notif->child, child_next, child) {
        lysc_node_free_(ctx, child);
    }
}

void
lysc_node_container_free(struct lysf_ctx *ctx, struct lysc_node_container *node)
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

/**
 * @brief Free the compiled leaf structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] node Compiled leaf structure to be freed.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
static void
lysc_node_leaf_free(struct lysf_ctx *ctx, struct lysc_node_leaf *node)
{
    FREE_ARRAY(ctx, node->when, lysc_when_free);
    FREE_ARRAY(ctx, node->musts, lysc_must_free);
    if (node->type) {
        lysc_type_free(ctx, node->type);
    }
    lydict_remove(ctx->ctx, node->units);
    if (node->dflt) {
        node->dflt->realtype->plugin->free(ctx->ctx, node->dflt);
        lysc_type_free(ctx, (struct lysc_type *)node->dflt->realtype);
        free(node->dflt);
    }
}

/**
 * @brief Free the compiled leaflist structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] node Compiled leaflist structure to be freed.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
static void
lysc_node_leaflist_free(struct lysf_ctx *ctx, struct lysc_node_leaflist *node)
{
    LY_ARRAY_COUNT_TYPE u;

    FREE_ARRAY(ctx, node->when, lysc_when_free);
    FREE_ARRAY(ctx, node->musts, lysc_must_free);
    if (node->type) {
        lysc_type_free(ctx, node->type);
    }
    lydict_remove(ctx->ctx, node->units);
    LY_ARRAY_FOR(node->dflts, u) {
        node->dflts[u]->realtype->plugin->free(ctx->ctx, node->dflts[u]);
        lysc_type_free(ctx, (struct lysc_type *)node->dflts[u]->realtype);
        free(node->dflts[u]);
    }
    LY_ARRAY_FREE(node->dflts);
}

/**
 * @brief Free the compiled list structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] node Compiled list structure to be freed.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
static void
lysc_node_list_free(struct lysf_ctx *ctx, struct lysc_node_list *node)
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

/**
 * @brief Free the compiled choice structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] node Compiled choice structure to be freed.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
static void
lysc_node_choice_free(struct lysf_ctx *ctx, struct lysc_node_choice *node)
{
    struct lysc_node *child, *child_next;

    FREE_ARRAY(ctx, node->when, lysc_when_free);
    LY_LIST_FOR_SAFE((struct lysc_node *)node->cases, child_next, child) {
        lysc_node_free_(ctx, child);
    }
}

/**
 * @brief Free the compiled case structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] node Compiled case structure to be freed.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
static void
lysc_node_case_free(struct lysf_ctx *ctx, struct lysc_node_case *node)
{
    struct lysc_node *child, *child_next;

    FREE_ARRAY(ctx, node->when, lysc_when_free);
    LY_LIST_FOR_SAFE(node->child, child_next, child) {
        lysc_node_free_(ctx, child);
    }
}

/**
 * @brief Free the compiled anyxml/anydata structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] node Compiled anyxml/anydata structure to be freed.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
static void
lysc_node_anydata_free(struct lysf_ctx *ctx, struct lysc_node_anydata *node)
{
    FREE_ARRAY(ctx, node->when, lysc_when_free);
    FREE_ARRAY(ctx, node->musts, lysc_must_free);
}

/**
 * @brief Free the compiled node structure.
 *
 * @param[in] ctx Free context.
 * @param[in,out] node Compiled node structure to be freed.
 * Since the structure is typically part of the sized array, the structure itself is not freed.
 */
static void
lysc_node_free_(struct lysf_ctx *ctx, struct lysc_node *node)
{
    ly_bool inout = 0;

    /* common part */
    lydict_remove(ctx->ctx, node->name);
    lydict_remove(ctx->ctx, node->dsc);
    lydict_remove(ctx->ctx, node->ref);

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
        LOGINT(ctx->ctx);
    }

    FREE_ARRAY(ctx, node->exts, lysc_ext_instance_free);

    if (!inout) {
        free(node);
    }
}

void
lysc_node_free(struct lysf_ctx *ctx, struct lysc_node *node, ly_bool unlink)
{
    struct lysc_node *next, *iter, **child_p;

    if (node->nodetype & (LYS_INPUT | LYS_OUTPUT)) {
        /* inouts are part of actions and cannot be unlinked/freed separately, we can only free all the children */
        struct lysc_node_action_inout *inout = (struct lysc_node_action_inout *)node;

        LY_LIST_FOR_SAFE(inout->child, next, iter) {
            lysc_node_free_(ctx, iter);
        }
        inout->child = NULL;
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
                LY_CHECK_ERR_RET(!iter, LOGINT(ctx->ctx), );
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
lysc_module_free(struct lysf_ctx *ctx, struct lysc_module *module)
{
    struct lysc_node *node, *node_next;

    if (!module) {
        return;
    }

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
lys_module_free(struct lysf_ctx *ctx, struct lys_module *module, ly_bool remove_links)
{
    LY_ARRAY_COUNT_TYPE u;

    if (!module) {
        return;
    }

    assert(!module->implemented);
    assert(!module->compiled);

    if (remove_links) {
        /* remove derived identity links */
        LY_ARRAY_FOR(module->identities, u) {
            lysc_ident_derived_unlink(&module->identities[u]);
        }
    }
    FREE_ARRAY(ctx, module->identities, lysc_ident_free);
    lysp_module_free(ctx, module->parsed);

    LY_ARRAY_FREE(module->augmented_by);
    LY_ARRAY_FREE(module->deviated_by);

    lydict_remove(ctx->ctx, module->name);
    lydict_remove(ctx->ctx, module->revision);
    lydict_remove(ctx->ctx, module->ns);
    lydict_remove(ctx->ctx, module->prefix);
    lydict_remove(ctx->ctx, module->filepath);
    lydict_remove(ctx->ctx, module->org);
    lydict_remove(ctx->ctx, module->contact);
    lydict_remove(ctx->ctx, module->dsc);
    lydict_remove(ctx->ctx, module->ref);

    free(module);
}

void
lysf_ctx_erase(struct lysf_ctx *ctx)
{
    struct lysc_ext *ext;
    uint32_t i;

    for (i = 0; i < ctx->ext_set.count; ++i) {
        ext = ctx->ext_set.objs[i];

        lydict_remove(ctx->ctx, ext->name);
        lydict_remove(ctx->ctx, ext->argname);
        free(ext);
    }
    ly_set_erase(&ctx->ext_set, NULL);
}

LIBYANG_API_DEF void
lyplg_ext_pfree_instance_substatements(const struct ly_ctx *ctx, struct lysp_ext_substmt *substmts)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysf_ctx fctx = {.ctx = (struct ly_ctx *)ctx};
    ly_bool node_free;

    LY_ARRAY_FOR(substmts, u) {
        if (!substmts[u].storage) {
            continue;
        }

        switch (substmts[u].stmt) {
        case LY_STMT_NOTIFICATION:
        case LY_STMT_INPUT:
        case LY_STMT_OUTPUT:
        case LY_STMT_ACTION:
        case LY_STMT_RPC:
        case LY_STMT_ANYDATA:
        case LY_STMT_ANYXML:
        case LY_STMT_AUGMENT:
        case LY_STMT_CASE:
        case LY_STMT_CHOICE:
        case LY_STMT_CONTAINER:
        case LY_STMT_GROUPING:
        case LY_STMT_LEAF:
        case LY_STMT_LEAF_LIST:
        case LY_STMT_LIST:
        case LY_STMT_USES: {
            struct lysp_node *child, *child_next;

            LY_LIST_FOR_SAFE(*((struct lysp_node **)substmts[u].storage), child_next, child) {
                node_free = (child->nodetype & (LYS_INPUT | LYS_OUTPUT)) ? 1 : 0;
                lysp_node_free(&fctx, child);
                if (node_free) {
                    free(child);
                }
            }
            *((struct lysc_node **)substmts[u].storage) = NULL;
            break;
        }
        case LY_STMT_BASE:
            /* multiple strings */
            FREE_ARRAY(ctx, **(const char ***)substmts[u].storage, lydict_remove);
            break;

        case LY_STMT_BIT:
        case LY_STMT_ENUM:
            /* single enum */
            lysp_type_enum_free(&fctx, *(struct lysp_type_enum **)substmts[u].storage);
            break;

        case LY_STMT_DEVIATE:
            /* single deviate */
            lysp_deviate_free(&fctx, *(struct lysp_deviate **)substmts[u].storage);
            break;

        case LY_STMT_DEVIATION:
            /* single deviation */
            lysp_deviation_free(&fctx, *(struct lysp_deviation **)substmts[u].storage);
            break;

        case LY_STMT_EXTENSION:
            /* single extension */
            lysp_ext_free(&fctx, *(struct lysp_ext **)substmts[u].storage);
            break;

        case LY_STMT_EXTENSION_INSTANCE:
            /* multiple extension instances */
            FREE_ARRAY(&fctx, *(struct lysp_ext_instance **)substmts[u].storage, lysp_ext_instance_free);
            break;

        case LY_STMT_FEATURE:
            /* multiple features */
            FREE_ARRAY(&fctx, *(struct lysp_feature **)substmts[u].storage, lysp_feature_free);
            break;

        case LY_STMT_IDENTITY:
            /* multiple identities */
            FREE_ARRAY(&fctx, *(struct lysp_ident **)substmts[u].storage, lysp_ident_free);
            break;

        case LY_STMT_IMPORT:
            /* multiple imports */
            FREE_ARRAY(&fctx, *(struct lysp_import **)substmts[u].storage, lysp_import_free);
            break;

        case LY_STMT_INCLUDE:
            /* multiple includes */
            FREE_ARRAY(&fctx, *(struct lysp_include **)substmts[u].storage, lysp_include_free);
            break;

        case LY_STMT_REFINE:
            /* multiple refines */
            FREE_ARRAY(&fctx, *(struct lysp_refine **)substmts[u].storage, lysp_refine_free);
            break;

        case LY_STMT_REVISION:
            /* multiple revisions */
            FREE_ARRAY(&fctx, *(struct lysp_revision **)substmts[u].storage, lysp_revision_free);
            break;

        case LY_STMT_CONFIG:
        case LY_STMT_FRACTION_DIGITS:
        case LY_STMT_MANDATORY:
        case LY_STMT_MAX_ELEMENTS:
        case LY_STMT_MIN_ELEMENTS:
        case LY_STMT_ORDERED_BY:
        case LY_STMT_POSITION:
        case LY_STMT_REQUIRE_INSTANCE:
        case LY_STMT_STATUS:
        case LY_STMT_VALUE:
        case LY_STMT_YANG_VERSION:
        case LY_STMT_YIN_ELEMENT:
            /* nothing to do */
            break;

        case LY_STMT_ARGUMENT:
        case LY_STMT_BELONGS_TO:
        case LY_STMT_CONTACT:
        case LY_STMT_DESCRIPTION:
        case LY_STMT_ERROR_APP_TAG:
        case LY_STMT_ERROR_MESSAGE:
        case LY_STMT_KEY:
        case LY_STMT_MODIFIER:
        case LY_STMT_NAMESPACE:
        case LY_STMT_ORGANIZATION:
        case LY_STMT_PREFIX:
        case LY_STMT_PRESENCE:
        case LY_STMT_REFERENCE:
        case LY_STMT_REVISION_DATE:
        case LY_STMT_UNITS:
            /* single string */
            lydict_remove(ctx, *(const char **)substmts[u].storage);
            break;

        case LY_STMT_LENGTH:
        case LY_STMT_MUST:
        case LY_STMT_PATTERN:
        case LY_STMT_RANGE:
            /* multiple restrictions */
            FREE_ARRAY(&fctx, *(struct lysp_restr **)substmts[u].storage, lysp_restr_free);
            break;

        case LY_STMT_WHEN:
            /* multiple whens */
            FREE_ARRAY(&fctx, *(struct lysp_when **)substmts[u].storage, lysp_when_free);
            break;

        case LY_STMT_PATH:
            /* single expression */
            lyxp_expr_free(ctx, *(struct lyxp_expr **)substmts[u].storage);
            break;

        case LY_STMT_DEFAULT:
        case LY_STMT_IF_FEATURE:
        case LY_STMT_UNIQUE:
            /* multiple qnames */
            FREE_ARRAY(ctx, *(struct lysp_qname **)substmts[u].storage, lysp_qname_free);
            break;

        case LY_STMT_TYPEDEF:
            /* multiple typedefs */
            FREE_ARRAY(&fctx, *(struct lysp_tpdf **)substmts[u].storage, lysp_tpdf_free);
            break;

        case LY_STMT_TYPE: {
            /* single type */
            struct lysp_type **type_p = substmts[u].storage;

            lysp_type_free(&fctx, *type_p);
            free(*type_p);
            break;
        }
        case LY_STMT_MODULE:
        case LY_STMT_SUBMODULE:
            /* single (sub)module */
            lysp_module_free(&fctx, *(struct lysp_module **)substmts[u].storage);
            break;

        default:
            LOGINT(ctx);
        }
    }

    LY_ARRAY_FREE(substmts);
}

LIBYANG_API_DEF void
lyplg_ext_cfree_instance_substatements(const struct ly_ctx *ctx, struct lysc_ext_substmt *substmts)
{
    LY_ARRAY_COUNT_TYPE u;
    struct lysf_ctx fctx = {.ctx = (struct ly_ctx *)ctx};
    ly_bool node_free;

    LY_ARRAY_FOR(substmts, u) {
        if (!substmts[u].storage) {
            continue;
        }

        switch (substmts[u].stmt) {
        case LY_STMT_NOTIFICATION:
        case LY_STMT_INPUT:
        case LY_STMT_OUTPUT:
        case LY_STMT_ACTION:
        case LY_STMT_RPC:
        case LY_STMT_ANYDATA:
        case LY_STMT_ANYXML:
        case LY_STMT_CASE:
        case LY_STMT_CHOICE:
        case LY_STMT_CONTAINER:
        case LY_STMT_LEAF:
        case LY_STMT_LEAF_LIST:
        case LY_STMT_LIST: {
            struct lysc_node *child, *child_next;

            LY_LIST_FOR_SAFE(*((struct lysc_node **)substmts[u].storage), child_next, child) {
                node_free = (child->nodetype & (LYS_INPUT | LYS_OUTPUT)) ? 1 : 0;
                lysc_node_free_(&fctx, child);
                if (node_free) {
                    free(child);
                }
            }
            *((struct lysc_node **)substmts[u].storage) = NULL;
            break;
        }
        case LY_STMT_USES:
        case LY_STMT_CONFIG:
        case LY_STMT_FRACTION_DIGITS:
        case LY_STMT_MANDATORY:
        case LY_STMT_MAX_ELEMENTS:
        case LY_STMT_MIN_ELEMENTS:
        case LY_STMT_ORDERED_BY:
        case LY_STMT_POSITION:
        case LY_STMT_REQUIRE_INSTANCE:
        case LY_STMT_STATUS:
        case LY_STMT_VALUE:
            /* nothing to do */
            break;

        case LY_STMT_ARGUMENT:
        case LY_STMT_CONTACT:
        case LY_STMT_DESCRIPTION:
        case LY_STMT_ERROR_APP_TAG:
        case LY_STMT_ERROR_MESSAGE:
        case LY_STMT_KEY:
        case LY_STMT_MODIFIER:
        case LY_STMT_NAMESPACE:
        case LY_STMT_ORGANIZATION:
        case LY_STMT_PRESENCE:
        case LY_STMT_REFERENCE:
        case LY_STMT_UNITS: {
            /* single item */
            const char *str = *((const char **)substmts[u].storage);

            lydict_remove(ctx, str);
            break;
        }
        case LY_STMT_BIT:
        case LY_STMT_ENUM: {
            /* sized array */
            struct lysc_type_bitenum_item *items = *((struct lysc_type_bitenum_item **)substmts[u].storage);

            FREE_ARRAY(&fctx, items, lysc_enum_item_free);
            break;
        }
        case LY_STMT_LENGTH:
        case LY_STMT_RANGE: {
            /* single item */
            struct lysc_range *range = *((struct lysc_range **)substmts[u].storage);

            lysc_range_free(&fctx, range);
            break;
        }
        case LY_STMT_MUST: {
            /* sized array */
            struct lysc_must *musts = *((struct lysc_must **)substmts[u].storage);

            FREE_ARRAY(&fctx, musts, lysc_must_free);
            break;
        }
        case LY_STMT_WHEN:
            /* single item, expects a pointer */
            lysc_when_free(&fctx, substmts[u].storage);
            break;

        case LY_STMT_PATTERN: {
            /* sized array of pointers */
            struct lysc_pattern **patterns = *((struct lysc_pattern ***)substmts[u].storage);

            FREE_ARRAY(&fctx, patterns, lysc_pattern_free);
            break;
        }
        case LY_STMT_TYPE: {
            /* single item */
            struct lysc_type *type = *((struct lysc_type **)substmts[u].storage);

            lysc_type_free(&fctx, type);
            break;
        }
        case LY_STMT_IDENTITY: {
            /* sized array */
            struct lysc_ident *idents = *((struct lysc_ident **)substmts[u].storage);

            FREE_ARRAY(&fctx, idents, lysc_ident_free);
            break;
        }
        case LY_STMT_EXTENSION_INSTANCE: {
            /* sized array */
            struct lysc_ext_instance *exts = *((struct lysc_ext_instance **)substmts[u].storage);

            FREE_ARRAY(&fctx, exts, lysc_ext_instance_free);
            break;
        }
        case LY_STMT_AUGMENT:
        case LY_STMT_BASE:
        case LY_STMT_BELONGS_TO:
        case LY_STMT_DEFAULT:
        case LY_STMT_DEVIATE:
        case LY_STMT_DEVIATION:
        case LY_STMT_EXTENSION:
        case LY_STMT_FEATURE:
        case LY_STMT_GROUPING:
        case LY_STMT_IF_FEATURE:
        case LY_STMT_IMPORT:
        case LY_STMT_INCLUDE:
        case LY_STMT_MODULE:
        case LY_STMT_PATH:
        case LY_STMT_PREFIX:
        case LY_STMT_REFINE:
        case LY_STMT_REVISION:
        case LY_STMT_REVISION_DATE:
        case LY_STMT_SUBMODULE:
        case LY_STMT_TYPEDEF:
        case LY_STMT_UNIQUE:
        case LY_STMT_YANG_VERSION:
        case LY_STMT_YIN_ELEMENT:
            /* it is not possible to compile these statements */
            break;

        default:
            LOGINT(ctx);
        }
    }

    LY_ARRAY_FREE(substmts);
}

void
lysp_yang_ctx_free(struct lysp_yang_ctx *ctx)
{
    if (ctx) {
        if (ctx->main_ctx == (struct lysp_ctx *)ctx) {
            ly_set_erase(&ctx->tpdfs_nodes, NULL);
            ly_set_erase(&ctx->grps_nodes, NULL);
        }
        assert(!ctx->tpdfs_nodes.count && !ctx->grps_nodes.count);
        ly_set_erase(&ctx->ext_inst, NULL);
        ly_set_rm_index(ctx->parsed_mods, ctx->parsed_mods->count - 1, NULL);
        if (!ctx->parsed_mods->count) {
            ly_set_free(ctx->parsed_mods, NULL);
        }
        free(ctx);
    }
}

void
lysp_yin_ctx_free(struct lysp_yin_ctx *ctx)
{
    if (ctx) {
        if (ctx->main_ctx == (struct lysp_ctx *)ctx) {
            ly_set_erase(&ctx->tpdfs_nodes, NULL);
            ly_set_erase(&ctx->grps_nodes, NULL);
        }
        assert(!ctx->tpdfs_nodes.count && !ctx->grps_nodes.count);
        ly_set_erase(&ctx->ext_inst, NULL);
        ly_set_rm_index(ctx->parsed_mods, ctx->parsed_mods->count - 1, NULL);
        if (!ctx->parsed_mods->count) {
            ly_set_free(ctx->parsed_mods, NULL);
        }
        lyxml_ctx_free(ctx->xmlctx);
        free(ctx);
    }
}
