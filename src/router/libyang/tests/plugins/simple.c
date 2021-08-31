/**
 * @file simple.c
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
#include "plugins_types.h"

/*
 * EXTENSION PLUGIN
 */

/**
 * @brief Compile NAMC's extension instances.
 *
 * Implementation of ::lyplg_ext_compile_clb callback set as lyext_plugin::compile.
 */
static LY_ERR
hint_compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *p_ext, struct lysc_ext_instance *c_ext)
{
    /* check that the extension is instantiated at an allowed place - data node */
    if (!LY_STMT_IS_DATA_NODE(c_ext->parent_stmt)) {
        lyplg_ext_log(c_ext, LY_LLWRN, 0, lysc_ctx_get_path(cctx),
                "Extension %s is allowed only in a data nodes, but it is placed in \"%s\" statement.",
                p_ext->name, ly_stmt2str(c_ext->parent_stmt));
        return LY_ENOT;
    }

    return LY_SUCCESS;
}

/**
 * @brief Plugin descriptions for the test extensions
 */
LYPLG_EXTENSIONS = {
    {
        .module = "libyang-plugins-simple",
        .revision = NULL,
        .name = "hint",

        .plugin.id = "libyang 2 - simple test, version 1",
        .plugin.compile = &hint_compile,
        .plugin.validate = NULL,
        .plugin.sprinter = NULL,
        .plugin.free = NULL
    },
    {0} /* terminating zeroed item */
};

/*
 * TYPE PLUGIN
 */

/**
 * @brief Plugin information for note (string) type implementation.
 *
 * Everything is just the same as for built-in string.
 */
LYPLG_TYPES = {
    {
        .module = "libyang-plugins-simple",
        .revision = NULL,
        .name = "note",

        .plugin.id = "libyang 2 - simple test, version 1",
        .plugin.store = lyplg_type_store_string,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.print = lyplg_type_print_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple
    },
    {0}
};
