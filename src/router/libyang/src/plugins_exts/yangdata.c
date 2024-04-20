/**
 * @file yangdata.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang extension plugin - yang-data (RFC 8040)
 *
 * Copyright (c) 2021 - 2022 CESNET, z.s.p.o.
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

static void yangdata_cfree(const struct ly_ctx *ctx, struct lysc_ext_instance *ext);

/**
 * @brief Parse yang-data extension instances.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
yangdata_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    LY_ERR ret;
    LY_ARRAY_COUNT_TYPE u;
    struct lysp_module *pmod;

    /* yang-data can appear only at the top level of a YANG module or submodule */
    if ((ext->parent_stmt != LY_STMT_MODULE) && (ext->parent_stmt != LY_STMT_SUBMODULE)) {
        lyplg_ext_parse_log(pctx, ext, LY_LLWRN, 0, "Extension %s is ignored since it appears as a non top-level statement "
                "in \"%s\" statement.", ext->name, lyplg_ext_stmt2str(ext->parent_stmt));
        return LY_ENOT;
    }

    pmod = ext->parent;

    /* check for duplication */
    LY_ARRAY_FOR(pmod->exts, u) {
        if ((&pmod->exts[u] != ext) && (pmod->exts[u].name == ext->name) && !strcmp(pmod->exts[u].argument, ext->argument)) {
            /* duplication of the same yang-data extension in a single module */
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s is instantiated multiple times.", ext->name);
            return LY_EVALID;
        }
    }

    /* parse yang-data substatements */
    LY_ARRAY_CREATE_GOTO(lyplg_ext_parse_get_cur_pmod(pctx)->mod->ctx, ext->substmts, 3, ret, emem);
    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[0].stmt = LY_STMT_CONTAINER;
    ext->substmts[0].storage = &ext->parsed;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[1].stmt = LY_STMT_CHOICE;
    ext->substmts[1].storage = &ext->parsed;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[2].stmt = LY_STMT_USES;
    ext->substmts[2].storage = &ext->parsed;

    if ((ret = lyplg_ext_parse_extension_instance(pctx, ext))) {
        return ret;
    }

    return LY_SUCCESS;

emem:
    lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EMEM, "Memory allocation failed (%s()).", __func__);
    return LY_EMEM;
}

/**
 * @brief Compile yang-data extension instances.
 *
 * Implementation of ::lyplg_ext_compile_clb callback set as lyext_plugin::compile.
 */
static LY_ERR
yangdata_compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *extp, struct lysc_ext_instance *ext)
{
    LY_ERR ret;
    const struct lysc_node *child;
    ly_bool valid = 1;
    uint32_t prev_options = *lyplg_ext_compile_get_options(cctx);

    /* compile yangg-data substatements */
    LY_ARRAY_CREATE_GOTO(cctx->ctx, ext->substmts, 3, ret, emem);
    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[0].stmt = LY_STMT_CONTAINER;
    ext->substmts[0].storage = &ext->compiled;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[1].stmt = LY_STMT_CHOICE;
    ext->substmts[1].storage = &ext->compiled;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[2].stmt = LY_STMT_USES;
    ext->substmts[2].storage = &ext->compiled;

    *lyplg_ext_compile_get_options(cctx) |= LYS_COMPILE_NO_CONFIG | LYS_COMPILE_NO_DISABLED;
    ret = lyplg_ext_compile_extension_instance(cctx, extp, ext);
    *lyplg_ext_compile_get_options(cctx) = prev_options;
    if (ret) {
        return ret;
    }

    /* check that we have really just a single container data definition in the top */
    child = ext->compiled;
    if (!child) {
        valid = 0;
        lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EVALID,
                "Extension %s is instantiated without any top level data node, but exactly one container data node is expected.",
                extp->name);
    } else if (child->next) {
        valid = 0;
        lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EVALID,
                "Extension %s is instantiated with multiple top level data nodes, but only a single container data node is allowed.",
                extp->name);
    } else if (child->nodetype == LYS_CHOICE) {
        /* all the choice's case are expected to result to a single container node */
        struct lysc_module *mod_c = ext->parent;
        const struct lysc_node *snode = NULL;

        while ((snode = lys_getnext(snode, child, mod_c, 0))) {
            if (snode->next) {
                valid = 0;
                lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EVALID,
                        "Extension %s is instantiated with multiple top level data nodes (inside a single choice's case), "
                        "but only a single container data node is allowed.", extp->name);
                break;
            } else if (snode->nodetype != LYS_CONTAINER) {
                valid = 0;
                lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EVALID,
                        "Extension %s is instantiated with %s top level data node (inside a choice), "
                        "but only a single container data node is allowed.", extp->name, lys_nodetype2str(snode->nodetype));
                break;
            }
        }
    } else if (child->nodetype != LYS_CONTAINER) {
        /* via uses */
        valid = 0;
        lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EVALID,
                "Extension %s is instantiated with %s top level data node, but only a single container data node is allowed.",
                extp->name, lys_nodetype2str(child->nodetype));
    }

    if (!valid) {
        yangdata_cfree(lyplg_ext_compile_get_ctx(cctx), ext);
        ext->compiled = ext->substmts = NULL;
        return LY_EVALID;
    }

    return LY_SUCCESS;

emem:
    lyplg_ext_compile_log(cctx, ext, LY_LLERR, LY_EMEM, "Memory allocation failed (%s()).", __func__);
    return LY_EMEM;
}

/**
 * @brief INFO printer
 *
 * Implementation of ::lyplg_ext_sprinter_info_clb set as ::lyext_plugin::printer_info
 */
static LY_ERR
yangdata_printer_info(struct lyspr_ctx *ctx, struct lysc_ext_instance *ext, ly_bool *flag)
{
    lyplg_ext_print_info_extension_instance(ctx, ext, flag);
    return LY_SUCCESS;
}

/**
 * @brief Free parsed yang-data extension instance data.
 *
 * Implementation of ::lyplg_clb_parse_free_clb callback set as lyext_plugin::pfree.
 */
static void
yangdata_pfree(const struct ly_ctx *ctx, struct lysp_ext_instance *ext)
{
    lyplg_ext_pfree_instance_substatements(ctx, ext->substmts);
}

/**
 * @brief Free compiled yang-data extension instance data.
 *
 * Implementation of ::lyplg_clb_compile_free_clb callback set as lyext_plugin::cfree.
 */
static void
yangdata_cfree(const struct ly_ctx *ctx, struct lysc_ext_instance *ext)
{
    lyplg_ext_cfree_instance_substatements(ctx, ext->substmts);
}

static void
yangdata_sprinter_node(uint16_t nodetype, const char **flags)
{
    if (nodetype & LYS_USES) {
        *flags = "-u";
    } else {
        *flags = "--";
    }
}

static LY_ERR
yangdata_sprinter_cnode(const struct lysc_node *node, const void *UNUSED(plugin_priv), ly_bool *UNUSED(skip),
        const char **flags, const char **UNUSED(add_opts))
{
    yangdata_sprinter_node(node->nodetype, flags);
    return LY_SUCCESS;
}

static LY_ERR
yangdata_sprinter_pnode(const struct lysp_node *node, const void *UNUSED(plugin_priv), ly_bool *UNUSED(skip),
        const char **flags, const char **UNUSED(add_opts))
{
    yangdata_sprinter_node(node->nodetype, flags);
    return LY_SUCCESS;
}

static LY_ERR
yangdata_sprinter_ctree(struct lysc_ext_instance *ext, const struct lyspr_tree_ctx *ctx,
        const char **UNUSED(flags), const char **UNUSED(add_opts))
{
    LY_ERR rc = LY_SUCCESS;

    assert(ctx);
    rc = lyplg_ext_sprinter_ctree_add_ext_nodes(ctx, ext, yangdata_sprinter_cnode);
    return rc;
}

static LY_ERR
yangdata_sprinter_ptree(struct lysp_ext_instance *ext, const struct lyspr_tree_ctx *ctx,
        const char **UNUSED(flags), const char **UNUSED(add_opts))
{
    LY_ERR rc = LY_SUCCESS;

    assert(ctx);
    rc = lyplg_ext_sprinter_ptree_add_ext_nodes(ctx, ext, yangdata_sprinter_pnode);
    return rc;
}

/**
 * @brief Plugin descriptions for the yang-data extension
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_EXTENSIONS = {
 */
const struct lyplg_ext_record plugins_yangdata[] = {
    {
        .module = "ietf-restconf",
        .revision = "2017-01-26",
        .name = "yang-data",

        .plugin.id = "ly2 yang-data v1",
        .plugin.parse = yangdata_parse,
        .plugin.compile = yangdata_compile,
        .plugin.printer_info = yangdata_printer_info,
        .plugin.printer_ctree = yangdata_sprinter_ctree,
        .plugin.printer_ptree = yangdata_sprinter_ptree,
        .plugin.node = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = yangdata_pfree,
        .plugin.cfree = yangdata_cfree
    },
    {0}     /* terminating zeroed record */
};
