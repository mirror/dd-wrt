/**
 * @file nacm.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang extension plugin - NACM (RFC 6536)
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
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

#include "libyang.h"
#include "plugins_exts.h"

struct nacm_dfs_arg {
    struct lysc_ext_instance *c_ext;
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
            if (node->exts[u].def == arg->c_ext->def) {
                /* the child already have its own NACM flag, so skip the subtree */
                *dfs_continue = 1;
                return LY_SUCCESS;
            }
        }

        /* duplicate this one to inherit it to the child */
        LY_ARRAY_NEW_GOTO(node->module->ctx, node->exts, inherited, ret, emem);

        inherited->def = lysc_ext_dup(arg->c_ext->def);
        inherited->parent = node;
        inherited->parent_stmt = lys_nodetype2stmt(node->nodetype);
        if (arg->c_ext->argument) {
            LY_ERR ret;

            if ((ret = lydict_insert(node->module->ctx, arg->c_ext->argument, strlen(arg->c_ext->argument),
                    &inherited->argument))) {
                return ret;
            }
        }
        /* TODO duplicate extension instances */
        inherited->data = arg->c_ext->data;
    }

    return LY_SUCCESS;

emem:
    lyplg_ext_log(arg->c_ext, LY_LLERR, LY_EMEM, NULL, "Memory allocation failed (%s()).", __func__);
    return ret;
}

/**
 * @brief Compile NAMC's extension instances.
 *
 * Implementation of ::lyplg_ext_compile_clb callback set as lyext_plugin::compile.
 */
static LY_ERR
nacm_compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *p_ext, struct lysc_ext_instance *c_ext)
{
    LY_ERR ret;
    struct lysc_node *parent = NULL;
    LY_ARRAY_COUNT_TYPE u;
    struct nacm_dfs_arg dfs_arg;

    static const uint8_t nacm_deny_all = 1;
    static const uint8_t nacm_deny_write = 2;

    /* store the NACM flag */
    if (!strcmp(c_ext->def->name, "default-deny-write")) {
        c_ext->data = (void *)&nacm_deny_write;
    } else if (!strcmp(c_ext->def->name, "default-deny-all")) {
        c_ext->data = (void *)&nacm_deny_all;
    } else {
        return LY_EINT;
    }

    /* check that the extension is instantiated at an allowed place - data node */
    if (!LY_STMT_IS_NODE(c_ext->parent_stmt)) {
        lyplg_ext_log(c_ext, LY_LLWRN, 0, lysc_ctx_get_path(cctx),
                "Extension %s is allowed only in a data nodes, but it is placed in \"%s\" statement.",
                p_ext->name, ly_stmt2str(c_ext->parent_stmt));
        return LY_ENOT;
    } else {
        parent = (struct lysc_node *)c_ext->parent;
        if (!(parent->nodetype & (LYS_CONTAINER | LYS_LEAF | LYS_LEAFLIST | LYS_LIST | LYS_CHOICE | LYS_ANYDATA |
                LYS_CASE | LYS_RPC | LYS_ACTION | LYS_NOTIF))) {
            /* note LYS_AUGMENT and LYS_USES is not in the list since they are not present in the compiled tree. Instead, libyang
             * passes all their extensions to their children nodes */
invalid_parent:
            lyplg_ext_log(c_ext, LY_LLWRN, 0, lysc_ctx_get_path(cctx),
                    "Extension %s is not allowed in %s statement.", p_ext->name, lys_nodetype2str(parent->nodetype));
            return LY_ENOT;
        }
        if ((c_ext->data == (void *)&nacm_deny_write) && (parent->nodetype & (LYS_RPC | LYS_ACTION | LYS_NOTIF))) {
            goto invalid_parent;
        }
    }

    /* check for duplication */
    LY_ARRAY_FOR(parent->exts, u) {
        if ((&parent->exts[u] != c_ext) && (parent->exts[u].def->plugin->compile == c_ext->def->plugin->compile)) {
            /* duplication of a NACM extension on a single node
             * We check for all NACM plugins since we want to catch even the situation that there is default-deny-all
             * AND default-deny-write */
            if (parent->exts[u].def == c_ext->def) {
                lyplg_ext_log(c_ext, LY_LLERR, LY_EVALID, lysc_ctx_get_path(cctx),
                        "Extension %s is instantiated multiple times.", p_ext->name);
            } else {
                lyplg_ext_log(c_ext, LY_LLERR, LY_EVALID, lysc_ctx_get_path(cctx),
                        "Extension nacm:default-deny-write is mixed with nacm:default-deny-all.");
            }
            return LY_EVALID;
        }
    }

    /* inherit the extension instance to all the children nodes */
    dfs_arg.c_ext = c_ext;
    dfs_arg.parent = parent;
    ret = lysc_tree_dfs_full(parent, nacm_inherit_clb, &dfs_arg);

    return ret;
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

        .plugin.id = "libyang 2 - NACM, version 1",
        .plugin.compile = &nacm_compile,
        .plugin.validate = NULL,
        .plugin.sprinter = NULL,
        .plugin.free = NULL
    }, {
        .module = "ietf-netconf-acm",
        .revision = "2018-02-14",
        .name = "default-deny-write",

        .plugin.id = "libyang 2 - NACM, version 1",
        .plugin.compile = &nacm_compile,
        .plugin.validate = NULL,
        .plugin.sprinter = NULL,
        .plugin.free = NULL
    }, {
        .module = "ietf-netconf-acm",
        .revision = "2012-02-22",
        .name = "default-deny-all",

        .plugin.id = "libyang 2 - NACM, version 1",
        .plugin.compile = &nacm_compile,
        .plugin.validate = NULL,
        .plugin.sprinter = NULL,
        .plugin.free = NULL
    }, {
        .module = "ietf-netconf-acm",
        .revision = "2018-02-14",
        .name = "default-deny-all",

        .plugin.id = "libyang 2 - NACM, version 1",
        .plugin.compile = &nacm_compile,
        .plugin.validate = NULL,
        .plugin.sprinter = NULL,
        .plugin.free = NULL
    },
    {0} /* terminating zeroed item */
};
