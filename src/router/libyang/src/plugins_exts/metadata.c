/**
 * @file metadata.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang extension plugin - Metadata (RFC 7952)
 *
 * Copyright (c) 2019 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "metadata.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"
#include "plugins_exts.h"

struct lysp_ext_metadata {
    struct lysp_type *type;            /**< type of the metadata (mandatory) */
    const char *units;                 /**< units of the leaf's type */
    struct lysp_qname *iffeatures;     /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    const char *dsc;                   /**< description */
    const char *ref;                   /**< reference */
    uint16_t flags;                    /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
};

struct lysc_ext_metadata {
    struct lysc_type *type;            /**< type of the metadata (mandatory) */
    const char *units;                 /**< units of the leaf's type */
    const char *dsc;                   /**< description */
    const char *ref;                   /**< reference */
    uint16_t flags;                    /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
};

/**
 * @brief Parse annotation extension instances.
 *
 * Implementation of ::lyplg_ext_parse_clb callback set as lyext_plugin::parse.
 */
static LY_ERR
annotation_parse(struct lysp_ctx *pctx, struct lysp_ext_instance *ext)
{
    LY_ERR r;
    struct lysp_ext_metadata *ann_pdata;
    struct lysp_module *pmod;
    LY_ARRAY_COUNT_TYPE u;

    /* annotations can appear only at the top level of a YANG module or submodule */
    if ((ext->parent_stmt != LY_STMT_MODULE) && (ext->parent_stmt != LY_STMT_SUBMODULE)) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s is allowed only at the top level of a YANG module or "
                "submodule, but it is placed in \"%s\" statement.", ext->name, lyplg_ext_stmt2str(ext->parent_stmt));
        return LY_EVALID;
    }

    pmod = ext->parent;

    /* check for duplication */
    LY_ARRAY_FOR(pmod->exts, u) {
        if ((&pmod->exts[u] != ext) && (pmod->exts[u].name == ext->name) && !strcmp(pmod->exts[u].argument, ext->argument)) {
            /* duplication of the same annotation extension in a single module */
            lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Extension %s is instantiated multiple times.", ext->name);
            return LY_EVALID;
        }
    }

    /* parse annotation substatements */
    ext->parsed = ann_pdata = calloc(1, sizeof *ann_pdata);
    if (!ann_pdata) {
        goto emem;
    }
    LY_ARRAY_CREATE_GOTO(lyplg_ext_parse_get_cur_pmod(pctx)->mod->ctx, ext->substmts, 6, r, emem);

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[0].stmt = LY_STMT_IF_FEATURE;
    ext->substmts[0].storage = &ann_pdata->iffeatures;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[1].stmt = LY_STMT_UNITS;
    ext->substmts[1].storage = &ann_pdata->units;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[2].stmt = LY_STMT_STATUS;
    ext->substmts[2].storage = &ann_pdata->flags;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[3].stmt = LY_STMT_TYPE;
    ext->substmts[3].storage = &ann_pdata->type;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[4].stmt = LY_STMT_DESCRIPTION;
    ext->substmts[4].storage = &ann_pdata->dsc;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[5].stmt = LY_STMT_REFERENCE;
    ext->substmts[5].storage = &ann_pdata->ref;

    if ((r = lyplg_ext_parse_extension_instance(pctx, ext))) {
        return r;
    }

    /* check for mandatory substatements */
    if (!ann_pdata->type) {
        lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EVALID, "Missing mandatory keyword \"type\" as a child of \"%s %s\".",
                ext->name, ext->argument);
        return LY_EVALID;
    }

    return LY_SUCCESS;

emem:
    lyplg_ext_parse_log(pctx, ext, LY_LLERR, LY_EMEM, "Memory allocation failed (%s()).", __func__);
    return LY_EMEM;
}

/**
 * @brief Compile annotation extension instances.
 *
 * Implementation of ::lyplg_ext_compile_clb callback set as lyext_plugin::compile.
 */
static LY_ERR
annotation_compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *extp, struct lysc_ext_instance *ext)
{
    LY_ERR ret;
    struct lysc_ext_metadata *ann_cdata;

    /* compile annotation substatements */
    ext->compiled = ann_cdata = calloc(1, sizeof *ann_cdata);
    if (!ann_cdata) {
        goto emem;
    }
    LY_ARRAY_CREATE_GOTO(lysc_ctx_get_ctx(cctx), ext->substmts, 6, ret, emem);

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[0].stmt = LY_STMT_IF_FEATURE;
    ext->substmts[0].storage = NULL;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[1].stmt = LY_STMT_UNITS;
    ext->substmts[1].storage = &ann_cdata->units;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[2].stmt = LY_STMT_STATUS;
    ext->substmts[2].storage = &ann_cdata->flags;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[3].stmt = LY_STMT_TYPE;
    ext->substmts[3].storage = &ann_cdata->type;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[4].stmt = LY_STMT_DESCRIPTION;
    ext->substmts[4].storage = &ann_cdata->dsc;

    LY_ARRAY_INCREMENT(ext->substmts);
    ext->substmts[5].stmt = LY_STMT_REFERENCE;
    ext->substmts[5].storage = &ann_cdata->ref;

    ret = lyplg_ext_compile_extension_instance(cctx, extp, ext);
    return ret;

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
annotation_printer_info(struct lyspr_ctx *ctx, struct lysc_ext_instance *ext, ly_bool *flag)
{
    lyplg_ext_print_info_extension_instance(ctx, ext, flag);

    return LY_SUCCESS;
}

/**
 * @brief Free parsed annotation extension instance data.
 *
 * Implementation of ::lyplg_ext_parse_free_clb callback set as ::lyext_plugin::pfree.
 */
static void
annotation_pfree(const struct ly_ctx *ctx, struct lysp_ext_instance *ext)
{
    if (!ext->substmts) {
        return;
    }

    lyplg_ext_pfree_instance_substatements(ctx, ext->substmts);
    free(ext->parsed);
}

/**
 * @brief Free compiled annotation extension instance data.
 *
 * Implementation of ::lyplg_ext_compile_free_clb callback set as ::lyext_plugin::cfree.
 */
static void
annotation_cfree(const struct ly_ctx *ctx, struct lysc_ext_instance *ext)
{
    if (!ext->substmts) {
        return;
    }

    lyplg_ext_cfree_instance_substatements(ctx, ext->substmts);
    free(ext->compiled);
}

/**
 * @brief Plugin descriptions for the Metadata's annotation extension
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_EXTENSIONS = {
 */
const struct lyplg_ext_record plugins_metadata[] = {
    {
        .module = "ietf-yang-metadata",
        .revision = "2016-08-05",
        .name = "annotation",

        .plugin.id = "ly2 metadata v1",
        .plugin.parse = annotation_parse,
        .plugin.compile = annotation_compile,
        .plugin.printer_info = annotation_printer_info,
        .plugin.printer_ctree = NULL,
        .plugin.printer_ptree = NULL,
        .plugin.node = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = annotation_pfree,
        .plugin.cfree = annotation_cfree,
    },
    {0}     /* terminating zeroed record */
};
