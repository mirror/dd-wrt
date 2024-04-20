/**
 * @file nacm.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang extension plugin - NACM (RFC 6536)
 *
 * Copyright (c) 2019 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "libyang.h"
#include "plugins_exts.h"

struct nacm_dfs_arg {
    struct lysc_ext_instance *ext;
    struct lysc_node *parent;
};

/**
 * @brief DFS callback implementation for inheriting the NACM extension.
 */
static LY_ERR
nacm_inherit_clb(struct lysc_node *node, void *data, ly_bool *dfs_continue)
{
    LY_ERR ret;
    struct nacm_dfs_arg *arg = data;
    struct lysc_ext_instance *inherited;
    LY_ARRAY_COUNT_TYPE u;

    /* ignore the parent from which we inherit and input/output nodes */
    if ((node != arg->parent) && !(node->nodetype & (LYS_INPUT | LYS_OUTPUT))) {
        /* check that the node does not have its own NACM extension instance */
        LY_ARRAY_FOR(node->exts, u) {
            if (node->exts[u].def == arg->ext->def) {
                /* the child already have its own NACM flag, so skip the subtree */
                *dfs_continue = 1;
                return LY_SUCCESS;
            }
        }

        /* duplicate this one to inherit it to the child */
        LY_ARRAY_NEW_GOTO(node->module->ctx, node->exts, inherited, ret, emem);

        inherited->def = arg->ext->def;
        inherited->parent = node;
        inherited->parent_stmt = lyplg_ext_nodetype2stmt(node->nodetype);
        if (arg->ext->argument) {
            if ((ret = lydict_insert(node->module->ctx, arg->ext->argument, 0, &inherited->argument))) {
                return ret;
            }
        }
        /* copy the pointer to the static variables */
        inherited->compiled = arg->ext->compiled;
    }

    return LY_SUCCESS;

emem:
    lyplg_ext_compile_log(NULL, arg->ext, LY_LLERR, LY_EMEM, "Memory allocation failed (%s()).", __func__);
    return ret;
}

/**
 * @brief Parse NACM extension instances.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
nacm_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    struct lysp_node *parent = NULL;
    LY_ARRAY_COUNT_TYPE u;

    /* check that the extension is instantiated at an allowed place - data node */
    if (!(ext->parent_stmt & LY_STMT_NODE_MASK)) {
        lyplg_ext_parse_log(pctx, ext, LY_LLWRN, 0, "Extension %s is allowed only in a data nodes, but it is placed in "
                "\"%s\" statement.", ext->name, lyplg_ext_stmt2str(ext->parent_stmt));
        return LY_ENOT;
    }

    parent = ext->parent;
    if (!(parent->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_CHOICE | LYS_ANYDATA |
            LYS_CASE | LYS_RPC | LYS_ACTION | LYS_NOTIF)) || (!strcmp(strchr(ext->name, ':') + 1, "default-deny-write") &&
            (parent->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF)))) {
        /* note LYS_AUGMENT and LYS_USES is not in the list since they are not present in the compiled tree. Instead, libyang
         * passes all their extensions to their children nodes */
        lyplg_ext_parse_log(pctx, ext, LY_LLWRN, 0, "Extension %s is not allowed in %s statement.", ext->name,
                lys_nodetype2str(parent->nodetype));
        return LY_ENOT;
    }

    /* check for duplication */
    LY_ARRAY_FOR(parent->exts, u) {
        if ((&parent->exts[u] != ext) && parent->exts[u].record && !strcmp(parent->exts[u].record->plugin.id, ext->record->plugin.id)) {
            /* duplication of a NACM extension on a single node
             * We check for all NACM plugins since we want to catch even the situation that there is default-deny-all
             * AND default-deny-write */
            if (parent->exts[u].name == ext->name) {
                lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s is instantiated multiple times.", ext->name);
            } else {
                lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID,
                        "Extension nacm:default-deny-write is mixed with nacm:default-deny-all.");
            }
            return LY_EVALID;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Compile NACM extension instances.
 *
 * Implementation of ::lyplg_ext_compile_clb callback set as lyext_plugin::compile.
 */
static LY_ERR
nacm_compile(struct lysc_ctx *UNUSED(cctx), const struct lysp_ext_instance *UNUSED(extp), struct lysc_ext_instance *ext)
{
    struct nacm_dfs_arg dfs_arg;

    static const uint8_t nacm_deny_all = 1;
    static const uint8_t nacm_deny_write = 2;

    /* store the NACM flag */
    if (!strcmp(ext->def->name, "default-deny-write")) {
        ext->compiled = (void *)&nacm_deny_write;
    } else if (!strcmp(ext->def->name, "default-deny-all")) {
        ext->compiled = (void *)&nacm_deny_all;
    } else {
        return LY_EINT;
    }

    /* inherit the extension instance to all the children nodes */
    dfs_arg.ext = ext;
    dfs_arg.parent = ext->parent;
    return lysc_tree_dfs_full(ext->parent, nacm_inherit_clb, &dfs_arg);
}

/**
 * @brief Plugin descriptions for the NACM's default-deny-write and default-deny-all extensions
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_EXTENSIONS = {
 */
const struct lyplg_ext_record plugins_nacm[] = {
    {
        .module = "ietf-netconf-acm",
        .revision = "2012-02-22",
        .name = "default-deny-write",

        .plugin.id = "ly2 NACM v1",
        .plugin.parse = nacm_parse,
        .plugin.compile = nacm_compile,
        .plugin.printer_info = NULL,
        .plugin.printer_ctree = NULL,
        .plugin.printer_ptree = NULL,
        .plugin.node = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = NULL,
        .plugin.cfree = NULL
    }, {
        .module = "ietf-netconf-acm",
        .revision = "2018-02-14",
        .name = "default-deny-write",

        .plugin.id = "ly2 NACM v1",
        .plugin.parse = nacm_parse,
        .plugin.compile = nacm_compile,
        .plugin.printer_info = NULL,
        .plugin.printer_ctree = NULL,
        .plugin.printer_ptree = NULL,
        .plugin.node = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = NULL,
        .plugin.cfree = NULL
    }, {
        .module = "ietf-netconf-acm",
        .revision = "2012-02-22",
        .name = "default-deny-all",

        .plugin.id = "ly2 NACM v1",
        .plugin.parse = nacm_parse,
        .plugin.compile = nacm_compile,
        .plugin.printer_info = NULL,
        .plugin.printer_ctree = NULL,
        .plugin.printer_ptree = NULL,
        .plugin.node = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = NULL,
        .plugin.cfree = NULL
    }, {
        .module = "ietf-netconf-acm",
        .revision = "2018-02-14",
        .name = "default-deny-all",

        .plugin.id = "ly2 NACM v1",
        .plugin.parse = nacm_parse,
        .plugin.compile = nacm_compile,
        .plugin.printer_info = NULL,
        .plugin.printer_ctree = NULL,
        .plugin.printer_ptree = NULL,
        .plugin.node = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = NULL,
        .plugin.cfree = NULL
    },
    {0} /* terminating zeroed item */
};
