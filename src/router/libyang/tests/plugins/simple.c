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
 * @brief Compile simple extension instances.
 *
 * Implementation of ::lyplg_ext_compile_clb callback set as lyext_plugin::compile.
 */
static LY_ERR
hint_compile(struct lysc_ctx *cctx, const struct lysp_ext_instance *extp, struct lysc_ext_instance *ext)
{
    /* check that the extension is instantiated at an allowed place - data node */
    if (!(ext->parent_stmt & LY_STMT_DATA_NODE_MASK)) {
        lyplg_ext_compile_log(cctx, ext, LY_LLWRN, 0,
                "Extension %s is allowed only in a data nodes, but it is placed in \"%s\" statement.",
                extp->name, lyplg_ext_stmt2str(ext->parent_stmt));
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

        .plugin.id = "ly2 simple test v1",
        .plugin.parse = NULL,
        .plugin.compile = hint_compile,
        .plugin.printer_info = NULL,
        .plugin.node = NULL,
        .plugin.snode = NULL,
        .plugin.validate = NULL,
        .plugin.pfree = NULL,
        .plugin.cfree = NULL
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

        .plugin.id = "ly2 simple test v1",
        .plugin.store = lyplg_type_store_string,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.sort = NULL,
        .plugin.print = lyplg_type_print_simple,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
        .plugin.lyb_data_len = 0
    },
    {0}
};
