/**
 * @file yangdata.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang extension plugin - yang-data (RFC 8040)
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
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

/**
 * @brief Free yang-data extension instances' data.
 *
 * Implementation of ::lyplg_clb_free_clb callback set as lyext_plugin::free.
 */
static void
yangdata_free(struct ly_ctx *ctx, struct lysc_ext_instance *ext)
{
    lyplg_ext_instance_substatements_free(ctx, ext->substmts);
}

/**
 * @brief Compile yang-data extension instances.
 *
 * Implementation of ::lyplg_ext_compile_clb callback set as lyext_plugin::compile.
 */
static LY_ERR
yangdata_compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *p_ext, struct lysc_ext_instance *c_ext)
{
    LY_ERR ret;
    LY_ARRAY_COUNT_TYPE u;
    struct lysc_module *mod_c;
    const struct lysc_node *child;
    ly_bool valid = 1;
    uint32_t prev_options = *lysc_ctx_get_options(cctx);

    /* yang-data can appear only at the top level of a YANG module or submodule */
    if ((c_ext->parent_stmt != LY_STMT_MODULE) && (c_ext->parent_stmt != LY_STMT_SUBMODULE)) {
        lyplg_ext_log(c_ext, LY_LLWRN, 0, lysc_ctx_get_path(cctx),
                "Extension %s is ignored since it appears as a non top-level statement in \"%s\" statement.",
                p_ext->name, ly_stmt2str(c_ext->parent_stmt));
        return LY_ENOT;
    }

    mod_c = (struct lysc_module *)c_ext->parent;

    /* check for duplication */
    LY_ARRAY_FOR(mod_c->exts, u) {
        if ((&mod_c->exts[u] != c_ext) && (mod_c->exts[u].def == c_ext->def) && !strcmp(mod_c->exts[u].argument, c_ext->argument)) {
            /* duplication of the same yang-data extension in a single module */
            lyplg_ext_log(c_ext, LY_LLERR, LY_EVALID, lysc_ctx_get_path(cctx), "Extension %s is instantiated multiple times.", p_ext->name);
            return LY_EVALID;
        }
    }

    /* compile annotation substatements
     * To let the compilation accept different statements possibly leading to the container top-level node, there are 3
     * allowed substatements pointing to a single storage. But when compiled, the substaments list is compressed just to
     * a single item providing the schema tree. */
    LY_ARRAY_CREATE_GOTO(cctx->ctx, c_ext->substmts, 3, ret, emem);
    LY_ARRAY_INCREMENT(c_ext->substmts);
    c_ext->substmts[0].stmt = LY_STMT_CONTAINER;
    c_ext->substmts[0].cardinality = LY_STMT_CARD_OPT;
    c_ext->substmts[0].storage = &c_ext->data;

    LY_ARRAY_INCREMENT(c_ext->substmts);
    c_ext->substmts[1].stmt = LY_STMT_CHOICE;
    c_ext->substmts[1].cardinality = LY_STMT_CARD_OPT;
    c_ext->substmts[1].storage = &c_ext->data;

    LY_ARRAY_INCREMENT(c_ext->substmts);
    c_ext->substmts[2].stmt = LY_STMT_USES;
    c_ext->substmts[2].cardinality = LY_STMT_CARD_OPT;
    c_ext->substmts[2].storage = &c_ext->data;

    *lysc_ctx_get_options(cctx) |= LYS_COMPILE_NO_CONFIG | LYS_COMPILE_NO_DISABLED;
    ret = lys_compile_extension_instance(cctx, p_ext, c_ext);
    *lysc_ctx_get_options(cctx) = prev_options;
    LY_ARRAY_DECREMENT(c_ext->substmts);
    LY_ARRAY_DECREMENT(c_ext->substmts);
    if (ret) {
        return ret;
    }

    /* check that we have really just a single container data definition in the top */
    child = *(struct lysc_node **)c_ext->substmts[0].storage;
    if (!child) {
        valid = 0;
        lyplg_ext_log(c_ext, LY_LLERR, LY_EVALID, lysc_ctx_get_path(cctx),
                "Extension %s is instantiated without any top level data node, but exactly one container data node is expected.",
                p_ext->name);
    } else if (child->next) {
        valid = 0;
        lyplg_ext_log(c_ext, LY_LLERR, LY_EVALID, lysc_ctx_get_path(cctx),
                "Extension %s is instantiated with multiple top level data nodes, but only a single container data node is allowed.",
                p_ext->name);
    } else if (child->nodetype == LYS_CHOICE) {
        /* all the choice's case are expected to result to a single container node */
        const struct lysc_node *snode = NULL;

        while ((snode = lys_getnext(snode, child, mod_c, 0))) {
            if (snode->next) {
                valid = 0;
                lyplg_ext_log(c_ext, LY_LLERR, LY_EVALID, lysc_ctx_get_path(cctx),
                        "Extension %s is instantiated with multiple top level data nodes (inside a single choice's case), "
                        "but only a single container data node is allowed.", p_ext->name);
                break;
            } else if (snode->nodetype != LYS_CONTAINER) {
                valid = 0;
                lyplg_ext_log(c_ext, LY_LLERR, LY_EVALID, lysc_ctx_get_path(cctx),
                        "Extension %s is instantiated with %s top level data node (inside a choice), "
                        "but only a single container data node is allowed.", p_ext->name, lys_nodetype2str(snode->nodetype));
                break;
            }
        }
    } else if (child->nodetype != LYS_CONTAINER) {
        /* via uses */
        valid = 0;
        lyplg_ext_log(c_ext, LY_LLERR, LY_EVALID, lysc_ctx_get_path(cctx),
                "Extension %s is instantiated with %s top level data node, but only a single container data node is allowed.",
                p_ext->name, lys_nodetype2str(child->nodetype));
    }

    if (!valid) {
        yangdata_free(lysc_ctx_get_ctx(cctx), c_ext);
        c_ext->data = c_ext->substmts = NULL;
        return LY_EVALID;
    }

    return LY_SUCCESS;

emem:
    lyplg_ext_log(c_ext, LY_LLERR, LY_EMEM, lysc_ctx_get_path(cctx), "Memory allocation failed (%s()).", __func__);
    return LY_EMEM;
}

/**
 * @brief INFO printer
 *
 * Implementation of ::lyplg_ext_schema_printer_clb set as ::lyext_plugin::sprinter
 */
static LY_ERR
yangdata_schema_printer(struct lyspr_ctx *ctx, struct lysc_ext_instance *ext, ly_bool *flag)
{
    lysc_print_extension_instance(ctx, ext, flag);
    return LY_SUCCESS;
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

        .plugin.id = "libyang 2 - yang-data, version 1",
        .plugin.compile = &yangdata_compile,
        .plugin.validate = NULL,
        .plugin.sprinter = &yangdata_schema_printer,
        .plugin.free = yangdata_free
    },
    {0}     /* terminating zeroed record */
};
