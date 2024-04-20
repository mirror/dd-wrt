/**
 * @file structure.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang extension plugin - structure (RFC 8791)
 *
 * Copyright (c) 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "libyang.h"
#include "plugins_exts.h"

struct lysp_ext_instance_structure {
    struct lysp_restr *musts;
    uint16_t flags;
    const char *dsc;
    const char *ref;
    struct lysp_tpdf *typedefs;
    struct lysp_node_grp *groupings;
    struct lysp_node *child;
};

struct lysc_ext_instance_structure {
    struct lysc_must *musts;
    uint16_t flags;
    const char *dsc;
    const char *ref;
    struct lysc_node *child;
};

struct lysp_ext_instance_augment_structure {
    uint16_t flags;
    const char *dsc;
    const char *ref;
    struct lysp_node *child;
    struct lysp_node_augment *aug;
};

/**
 * @brief Parse structure extension instances.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
structure_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    LY_ERR rc;
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_module *pmod;
    struct lysp_ext_instance_structure *struct_pdata;

    /* structure can appear only at the top level of a YANG module or submodule */
    if ((ext->parent_stmt != LY_STMT_MODULE) && (ext->parent_stmt != LY_STMT_SUBMODULE)) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                "Extension %s must not be used as a non top-level statement in \"%s\" statement.", ext->name,
                lyplg_ext_stmt2str(ext->parent_stmt));
        return LY_EVALID;
    }

    pmod = ext->parent;

    /* check for duplication */
    LY_ARRAY_FOR(pmod->exts, u) {
        if ((&pmod->exts[u] != ext) && (pmod->exts[u].name == ext->name) && !strcmp(pmod->exts[u].argument, ext->argument)) {
            /* duplication of the same structure extension in a single module */
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s is instantiated multiple times.", ext->name);
            return LY_EVALID;
        }
    }

    /* allocate the storage */
    struct_pdata = calloc(1, sizeof *struct_pdata);
    if (!struct_pdata) {
        goto emem;
    }
    ext->parsed = struct_pdata;
    LY_ARRAY_CREATE_GOTO(lyplg_ext_parse_get_cur_pmod(pctx)->mod->ctx, ext->substmts, 14, rc, emem);

    /* parse substatements */
    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[0].stmt = LY_STMT_MUST;
    ext->substmts[0].storage = &struct_pdata->musts;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[1].stmt = LY_STMT_STATUS;
    ext->substmts[1].storage = &struct_pdata->flags;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[2].stmt = LY_STMT_DESCRIPTION;
    ext->substmts[2].storage = &struct_pdata->dsc;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[3].stmt = LY_STMT_REFERENCE;
    ext->substmts[3].storage = &struct_pdata->ref;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[4].stmt = LY_STMT_TYPEDEF;
    ext->substmts[4].storage = &struct_pdata->typedefs;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[5].stmt = LY_STMT_GROUPING;
    ext->substmts[5].storage = &struct_pdata->groupings;

    /* data-def-stmt */
    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[6].stmt = LY_STMT_CONTAINER;
    ext->substmts[6].storage = &struct_pdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[7].stmt = LY_STMT_LEAF;
    ext->substmts[7].storage = &struct_pdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[8].stmt = LY_STMT_LEAF_LIST;
    ext->substmts[8].storage = &struct_pdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[9].stmt = LY_STMT_LIST;
    ext->substmts[9].storage = &struct_pdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[10].stmt = LY_STMT_CHOICE;
    ext->substmts[10].storage = &struct_pdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[11].stmt = LY_STMT_ANYDATA;
    ext->substmts[11].storage = &struct_pdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[12].stmt = LY_STMT_ANYXML;
    ext->substmts[12].storage = &struct_pdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[13].stmt = LY_STMT_USES;
    ext->substmts[13].storage = &struct_pdata->child;

    rc = lyplg_ext_parse_extension_instance(pctx, ext);
    return rc;

emem:
    lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EMEM, "Memory allocation failed (%s()).", __func__);
    return LY_EMEM;
}

/**
 * @brief Compile structure extension instances.
 *
 * Implementation of ::lyplg_ext_compile_clb callback set as lyext_plugin::compile.
 */
static LY_ERR
structure_compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *extp, struct lysc_ext_instance *ext)
{
    LY_ERR rc;
    struct lysc_module *mod_c;
    const struct lysc_node *child;
    struct lysc_ext_instance_structure *struct_cdata;
    uint32_t prev_options = *lyplg_ext_compile_get_options(cctx);

    mod_c = ext->parent;

    /* check identifier namespace with the compiled nodes */
    LY_LIST_FOR(mod_c->data, child) {
        if (!strcmp(child->name, ext->argument)) {
            /* identifier collision */
            lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EVALID,  "Extension %s collides with a %s with the same identifier.",
                    extp->name, lys_nodetype2str(child->nodetype));
            return LY_EVALID;
        }
    }

    /* allocate the storage */
    struct_cdata = calloc(1, sizeof *struct_cdata);
    if (!struct_cdata) {
        goto emem;
    }
    ext->compiled = struct_cdata;

    /* compile substatements */
    LY_ARRAY_CREATE_GOTO(cctx->ctx, ext->substmts, 14, rc, emem);
    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[0].stmt = LY_STMT_MUST;
    ext->substmts[0].storage = &struct_cdata->musts;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[1].stmt = LY_STMT_STATUS;
    ext->substmts[1].storage = &struct_cdata->flags;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[2].stmt = LY_STMT_DESCRIPTION;
    ext->substmts[2].storage = &struct_cdata->dsc;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[3].stmt = LY_STMT_REFERENCE;
    ext->substmts[3].storage = &struct_cdata->ref;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[4].stmt = LY_STMT_TYPEDEF;
    ext->substmts[4].storage = NULL;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[5].stmt = LY_STMT_GROUPING;
    ext->substmts[5].storage = NULL;

    /* data-def-stmt */
    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[6].stmt = LY_STMT_CONTAINER;
    ext->substmts[6].storage = &struct_cdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[7].stmt = LY_STMT_LEAF;
    ext->substmts[7].storage = &struct_cdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[8].stmt = LY_STMT_LEAF_LIST;
    ext->substmts[8].storage = &struct_cdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[9].stmt = LY_STMT_LIST;
    ext->substmts[9].storage = &struct_cdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[10].stmt = LY_STMT_CHOICE;
    ext->substmts[10].storage = &struct_cdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[11].stmt = LY_STMT_ANYDATA;
    ext->substmts[11].storage = &struct_cdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[12].stmt = LY_STMT_ANYXML;
    ext->substmts[12].storage = &struct_cdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[13].stmt = LY_STMT_USES;
    ext->substmts[13].storage = &struct_cdata->child;

    *lyplg_ext_compile_get_options(cctx) |= LYS_COMPILE_NO_CONFIG | LYS_COMPILE_NO_DISABLED;
    rc = lyplg_ext_compile_extension_instance(cctx, extp, ext);
    *lyplg_ext_compile_get_options(cctx) = prev_options;
    if (rc) {
        return rc;
    }

    return LY_SUCCESS;

emem:
    lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EMEM, "Memory allocation failed (%s()).", __func__);
    return LY_EMEM;
}

/**
 * @brief Structure schema info printer.
 *
 * Implementation of ::lyplg_ext_sprinter_info_clb set as ::lyext_plugin::printer_info
 */
static LY_ERR
structure_printer_info(struct lyspr_ctx *ctx, struct lysc_ext_instance *ext, ly_bool *flag)
{
    lyplg_ext_print_info_extension_instance(ctx, ext, flag);
    return LY_SUCCESS;
}

/**
 * @brief Free parsed structure extension instance data.
 *
 * Implementation of ::lyplg_clb_parse_free_clb callback set as lyext_plugin::pfree.
 */
static void
structure_pfree(const struct ly_ctx *ctx, struct lysp_ext_instance *ext)
{
    lyplg_ext_pfree_instance_substatements(ctx, ext->substmts);
    free(ext->parsed);
}

/**
 * @brief Free compiled structure extension instance data.
 *
 * Implementation of ::lyplg_clb_compile_free_clb callback set as lyext_plugin::cfree.
 */
static void
structure_cfree(const struct ly_ctx *ctx, struct lysc_ext_instance *ext)
{
    lyplg_ext_cfree_instance_substatements(ctx, ext->substmts);
    free(ext->compiled);
}

/**
 * @brief Parse augment-structure extension instances.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
structure_aug_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    LY_ERR rc;
    struct lysp_stmt *stmt;
    struct lysp_ext_instance_augment_structure *aug_pdata;
    const struct ly_ctx *ctx = lyplg_ext_parse_get_cur_pmod(pctx)->mod->ctx;

    /* augment-structure can appear only at the top level of a YANG module or submodule */
    if ((ext->parent_stmt != LY_STMT_MODULE) && (ext->parent_stmt != LY_STMT_SUBMODULE)) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                "Extension %s must not be used as a non top-level statement in \"%s\" statement.", ext->name,
                lyplg_ext_stmt2str(ext->parent_stmt));
        return LY_EVALID;
    }

    /* augment-structure must define some data-def-stmt */
    LY_LIST_FOR(ext->child, stmt) {
        if (stmt->kw & LY_STMT_DATA_NODE_MASK) {
            break;
        }
    }
    if (!stmt) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s does not define any data-def-stmt statements.",
                ext->name);
        return LY_EVALID;
    }

    /* allocate the storage */
    aug_pdata = calloc(1, sizeof *aug_pdata);
    if (!aug_pdata) {
        goto emem;
    }
    ext->parsed = aug_pdata;
    LY_ARRAY_CREATE_GOTO(ctx, ext->substmts, 13, rc, emem);

    /* parse substatements */
    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[0].stmt = LY_STMT_STATUS;
    ext->substmts[0].storage = &aug_pdata->flags;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[1].stmt = LY_STMT_DESCRIPTION;
    ext->substmts[1].storage = &aug_pdata->dsc;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[2].stmt = LY_STMT_REFERENCE;
    ext->substmts[2].storage = &aug_pdata->ref;

    /* data-def-stmt */
    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[3].stmt = LY_STMT_CONTAINER;
    ext->substmts[3].storage = &aug_pdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[4].stmt = LY_STMT_LEAF;
    ext->substmts[4].storage = &aug_pdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[5].stmt = LY_STMT_LEAF_LIST;
    ext->substmts[5].storage = &aug_pdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[6].stmt = LY_STMT_LIST;
    ext->substmts[6].storage = &aug_pdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[7].stmt = LY_STMT_CHOICE;
    ext->substmts[7].storage = &aug_pdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[8].stmt = LY_STMT_ANYDATA;
    ext->substmts[8].storage = &aug_pdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[9].stmt = LY_STMT_ANYXML;
    ext->substmts[9].storage = &aug_pdata->child;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[10].stmt = LY_STMT_USES;
    ext->substmts[10].storage = &aug_pdata->child;

    /* case */
    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[11].stmt = LY_STMT_CASE;
    ext->substmts[11].storage = &aug_pdata->child;

    if ((rc = lyplg_ext_parse_extension_instance(pctx, ext))) {
        return rc;
    }

    /* add fake parsed augment node */
    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[12].stmt = LY_STMT_AUGMENT;
    ext->substmts[12].storage = &aug_pdata->aug;

    aug_pdata->aug = calloc(1, sizeof *aug_pdata->aug);
    if (!aug_pdata->aug) {
        goto emem;
    }
    aug_pdata->aug->nodetype = LYS_AUGMENT;
    aug_pdata->aug->flags = aug_pdata->flags;
    if (lydict_insert(ctx, ext->argument, 0, &aug_pdata->aug->nodeid)) {
        goto emem;
    }
    aug_pdata->aug->child = aug_pdata->child;
    /* avoid double free */
    aug_pdata->child = NULL;

    return LY_SUCCESS;

emem:
    lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EMEM, "Memory allocation failed (%s()).", __func__);
    return LY_EMEM;
}

static LY_ERR
structure_sprinter_pnode(const struct lysp_node *UNUSED(node), const void *UNUSED(plugin_priv),
        ly_bool *UNUSED(skip), const char **flags, const char **UNUSED(add_opts))
{
    *flags = "";
    return LY_SUCCESS;
}

static LY_ERR
structure_sprinter_cnode(const struct lysc_node *UNUSED(node), const void *UNUSED(plugin_priv),
        ly_bool *UNUSED(skip), const char **flags, const char **UNUSED(add_opts))
{
    *flags = "";
    return LY_SUCCESS;
}

/**
 * @brief Structure schema compiled tree printer.
 *
 * Implementation of ::lyplg_ext_sprinter_ctree_clb callback set as lyext_plugin::printer_ctree.
 */
static LY_ERR
structure_sprinter_ctree(struct lysc_ext_instance *ext, const struct lyspr_tree_ctx *ctx,
        const char **UNUSED(flags), const char **UNUSED(add_opts))
{
    LY_ERR rc;

    rc = lyplg_ext_sprinter_ctree_add_ext_nodes(ctx, ext, structure_sprinter_cnode);
    return rc;
}

/**
 * @brief Structure schema parsed tree printer.
 *
 * Implementation of ::lyplg_ext_sprinter_ptree_clb callback set as lyext_plugin::printer_ptree.
 */
static LY_ERR
structure_sprinter_ptree(struct lysp_ext_instance *ext, const struct lyspr_tree_ctx *ctx,
        const char **UNUSED(flags), const char **UNUSED(add_opts))
{
    LY_ERR rc;

    rc = lyplg_ext_sprinter_ptree_add_ext_nodes(ctx, ext, structure_sprinter_pnode);
    return rc;
}

/**
 * @brief Augment structure schema parsed tree printer.
 *
 * Implementation of ::lyplg_ext_sprinter_ptree_clb callback set as lyext_plugin::printer_ptree.
 */
static LY_ERR
structure_aug_sprinter_ptree(struct lysp_ext_instance *ext, const struct lyspr_tree_ctx *ctx,
        const char **UNUSED(flags), const char **UNUSED(add_opts))
{
    LY_ERR rc = LY_SUCCESS;
    struct lysp_node_augment **aug;

    assert(ctx);

    aug = ext->substmts[12].storage;
    rc = lyplg_ext_sprinter_ptree_add_nodes(ctx, (*aug)->child, structure_sprinter_pnode);

    return rc;
}

/**
 * @brief Augment structure schema compiled tree printer.
 *
 * Implementation of ::lyplg_ext_sprinter_ctree_clb callback set as lyext_plugin::printer_ctree.
 */
static LY_ERR
structure_aug_sprinter_ctree(struct lysc_ext_instance *ext, const struct lyspr_tree_ctx *ctx, const char **flags,
        const char **add_opts)
{
    LY_ERR rc = LY_SUCCESS;

    LY_ARRAY_COUNT_TYPE i;
    struct lysp_ext_instance *parsed_ext;

    assert(ctx);

    /* find the parsed ext structure */
    parsed_ext = ext->module->parsed->exts;
    LY_ARRAY_FOR(parsed_ext, i) {
        if (!strcmp(parsed_ext[i].name, "sx:augment-structure") && !strcmp(parsed_ext[i].argument, ext->argument)) {
            break;
        }
    }
    assert(i < LY_ARRAY_COUNT(parsed_ext));

    /* for augments print the parsed tree */
    rc = structure_aug_sprinter_ptree(parsed_ext, ctx, flags, add_opts);
    return rc;
}

/**
 * @brief Plugin descriptions for the structure extension
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_EXTENSIONS = {
 */
const struct lyplg_ext_record plugins_structure[] = {
    {
        .module = "ietf-yang-structure-ext",
        .revision = "2020-06-17",
        .name = "structure",

        .plugin.id = "ly2 structure v1",
        .plugin.parse = structure_parse,
        .plugin.compile = structure_compile,
        .plugin.printer_info = structure_printer_info,
        .plugin.printer_ctree = structure_sprinter_ctree,
        .plugin.printer_ptree = structure_sprinter_ptree,
        .plugin.node = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = structure_pfree,
        .plugin.cfree = structure_cfree
    },
    {
        .module = "ietf-yang-structure-ext",
        .revision = "2020-06-17",
        .name = "augment-structure",

        .plugin.id = "ly2 structure v1",
        .plugin.parse = structure_aug_parse,
        .plugin.compile = NULL,
        .plugin.printer_info = NULL,
        .plugin.printer_ctree = structure_aug_sprinter_ctree,
        .plugin.printer_ptree = structure_aug_sprinter_ptree,
        .plugin.node = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = structure_pfree,
        .plugin.cfree = NULL
    },
    {0}     /* terminating zeroed record */
};
