/**
 * @file metadata.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief libyang extension plugin - Metadata (RFC 7952)
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
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

/**
 * @brief Representation of the compiled metadata substatements - simplify storage for the items available via
 * ::lysc_ext_substmt.
 */
struct lyext_metadata {
    struct lysc_type *type;            /**< type of the metadata (mandatory) */
    const char *units;                 /**< units of the leaf's type */
    struct lysc_iffeature *iffeatures; /**< list of if-feature expressions ([sized array](@ref sizedarrays)) */
    const char *dsc;                   /**< description */
    const char *ref;                   /**< reference */
    uint16_t flags;                    /**< [schema node flags](@ref snodeflags) - only LYS_STATUS_* values are allowed */
};

/**
 * @brief Compile annotation extension instances.
 *
 * Implementation of ::lyplg_ext_compile_clb callback set as lyext_plugin::compile.
 */
static LY_ERR
annotation_compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *p_ext, struct lysc_ext_instance *c_ext)
{
    LY_ERR ret;
    struct lyext_metadata *annotation;
    struct lysc_module *mod_c;
    LY_ARRAY_COUNT_TYPE u;

    /* annotations can appear only at the top level of a YANG module or submodule */
    if ((c_ext->parent_stmt != LY_STMT_MODULE) && (c_ext->parent_stmt != LY_STMT_SUBMODULE)) {
        lyplg_ext_log(c_ext, LY_LLERR, LY_EVALID, lysc_ctx_get_path(cctx),
                "Extension %s is allowed only at the top level of a YANG module or submodule, but it is placed in \"%s\" statement.",
                p_ext->name, ly_stmt2str(c_ext->parent_stmt));
        return LY_EVALID;
    }
    /* check mandatory argument */
    if (!c_ext->argument) {
        lyplg_ext_log(c_ext, LY_LLERR, LY_EVALID, lysc_ctx_get_path(cctx),
                "Extension %s is instantiated without mandatory argument representing metadata name.", p_ext->name);
        return LY_EVALID;
    }

    mod_c = (struct lysc_module *)c_ext->parent;

    /* check for duplication */
    LY_ARRAY_FOR(mod_c->exts, u) {
        if ((&mod_c->exts[u] != c_ext) && (mod_c->exts[u].def == c_ext->def) && !strcmp(mod_c->exts[u].argument, c_ext->argument)) {
            /* duplication of the same annotation extension in a single module */
            lyplg_ext_log(c_ext, LY_LLERR, LY_EVALID, lysc_ctx_get_path(cctx), "Extension %s is instantiated multiple times.", p_ext->name);
            return LY_EVALID;
        }
    }

    /* compile annotation substatements */
    c_ext->data = annotation = calloc(1, sizeof *annotation);
    if (!annotation) {
        goto emem;
    }
    LY_ARRAY_CREATE_GOTO(lysc_ctx_get_ctx(cctx), c_ext->substmts, 6, ret, emem);

    LY_ARRAY_INCREMENT(c_ext->substmts);
    c_ext->substmts[ANNOTATION_SUBSTMT_IFF].stmt = LY_STMT_IF_FEATURE;
    c_ext->substmts[ANNOTATION_SUBSTMT_IFF].cardinality = LY_STMT_CARD_ANY;
    c_ext->substmts[ANNOTATION_SUBSTMT_IFF].storage = &annotation->iffeatures;

    LY_ARRAY_INCREMENT(c_ext->substmts);
    c_ext->substmts[ANNOTATION_SUBSTMT_UNITS].stmt = LY_STMT_UNITS;
    c_ext->substmts[ANNOTATION_SUBSTMT_UNITS].cardinality = LY_STMT_CARD_OPT;
    c_ext->substmts[ANNOTATION_SUBSTMT_UNITS].storage = &annotation->units;

    LY_ARRAY_INCREMENT(c_ext->substmts);
    c_ext->substmts[ANNOTATION_SUBSTMT_STATUS].stmt = LY_STMT_STATUS;
    c_ext->substmts[ANNOTATION_SUBSTMT_STATUS].cardinality = LY_STMT_CARD_OPT;
    c_ext->substmts[ANNOTATION_SUBSTMT_STATUS].storage = &annotation->flags;

    LY_ARRAY_INCREMENT(c_ext->substmts);
    c_ext->substmts[ANNOTATION_SUBSTMT_TYPE].stmt = LY_STMT_TYPE;
    c_ext->substmts[ANNOTATION_SUBSTMT_TYPE].cardinality = LY_STMT_CARD_MAND;
    c_ext->substmts[ANNOTATION_SUBSTMT_TYPE].storage = &annotation->type;

    LY_ARRAY_INCREMENT(c_ext->substmts);
    c_ext->substmts[ANNOTATION_SUBSTMT_DSC].stmt = LY_STMT_DESCRIPTION;
    c_ext->substmts[ANNOTATION_SUBSTMT_DSC].cardinality = LY_STMT_CARD_OPT;
    c_ext->substmts[ANNOTATION_SUBSTMT_DSC].storage = &annotation->dsc;

    LY_ARRAY_INCREMENT(c_ext->substmts);
    c_ext->substmts[ANNOTATION_SUBSTMT_REF].stmt = LY_STMT_REFERENCE;
    c_ext->substmts[ANNOTATION_SUBSTMT_REF].cardinality = LY_STMT_CARD_OPT;
    c_ext->substmts[ANNOTATION_SUBSTMT_REF].storage = &annotation->ref;

    ret = lys_compile_extension_instance(cctx, p_ext, c_ext);
    return ret;

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
annotation_schema_printer(struct lyspr_ctx *ctx, struct lysc_ext_instance *ext, ly_bool *flag)
{
    lysc_print_extension_instance(ctx, ext, flag);

    return LY_SUCCESS;
}

/**
 * @brief Free annotation extension instances' data.
 *
 * Implementation of ::lyplg_ext_free_clb callback set as ::lyext_plugin::free.
 */
static void
annotation_free(struct ly_ctx *ctx, struct lysc_ext_instance *ext)
{
    if (!ext->substmts) {
        return;
    }

    lyplg_ext_instance_substatements_free(ctx, ext->substmts);
    free(ext->data);
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

        .plugin.id = "libyang 2 - metadata, version 1",
        .plugin.compile = &annotation_compile,
        .plugin.validate = NULL,
        .plugin.sprinter = &annotation_schema_printer,
        .plugin.free = annotation_free
    },
    {0}     /* terminating zeroed record */
};
