/**
 * @file validate.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Simple testing plugins implementation
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdlib.h>

#include "libyang.h"
#include "plugins_exts.h"

/**
 * @brief Compile NAMC's extension instances.
 *
 * Implementation of ::lyplg_ext_compile_clb callback set as ::lyext_plugin.compile.
 */
static LY_ERR
compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *p_ext, struct lysc_ext_instance *c_ext)
{
    /* check that the extension is instantiated at an allowed place - data node */
    if (!LY_STMT_IS_DATA_NODE(c_ext->parent_stmt) && (c_ext->parent_stmt != LY_STMT_TYPE)) {
        lyplg_ext_log(c_ext, LY_LLWRN, 0, lysc_ctx_get_path(cctx),
                "Extension %s is allowed only in a data nodes, but it is placed in \"%s\" statement.",
                p_ext->name, ly_stmt2str(c_ext->parent_stmt));
        return LY_ENOT;
    }

    return LY_SUCCESS;
}

/**
 * @brief Compile NAMC's extension instances.
 *
 * Implementation of ::lyplg_ext_data_validation_clb callback set as ::lyext_plugin.validate.
 */
static LY_ERR
validate(struct lysc_ext_instance *ext, struct lyd_node *node)
{
    lyplg_ext_log(ext, LY_LLWRN, LY_SUCCESS, NULL, "extra validation callback invoked on %s", node->schema->name);
    return LY_SUCCESS;
}

/**
 * @brief Plugin descriptions for the test extensions
 */
LYPLG_EXTENSIONS = {
    {
        .module = "libyang-plugins-validate",
        .revision = NULL,
        .name = "extra-validation",

        .plugin.id = "libyang 2 - validation test, version 1",
        .plugin.compile = &compile,
        .plugin.validate = &validate,
        .plugin.sprinter = NULL,
        .plugin.free = NULL
    },
    {0} /* terminating zeroed item */
};
