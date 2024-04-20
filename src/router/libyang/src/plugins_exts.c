/**
 * @file plugins_exts.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Internally implemented YANG extensions.
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "plugins_exts.h"
#include "plugins_exts_compile.h"
#include "plugins_exts_print.h"

#include <stdint.h>

extern struct lyplg_ext metadata_plugin; /* plugins_exts_metadata.c */
extern struct lyplg_ext nacm_plugin;     /* plugins_exts_nacm.c */
extern struct lyplg_ext yangdata_plugin; /* plugins_exts_yangdata.c */

/* internal libyang headers - do not make them accessible to the extension plugins in plugins_exts_*.c */
#include "common.h"
#include "printer_internal.h"
#include "schema_compile.h"

API struct ly_ctx *
lysc_ctx_get_ctx(const struct lysc_ctx *ctx)
{
    return ctx->ctx;
}

API uint32_t *
lysc_ctx_get_options(const struct lysc_ctx *ctx)
{
    return &((struct lysc_ctx *)ctx)->options;
}

API const char *
lysc_ctx_get_path(const struct lysc_ctx *ctx)
{
    return ctx->path;
}

API struct ly_out **
lys_ypr_ctx_get_out(const struct lyspr_ctx *ctx)
{
    return &((struct lyspr_ctx *)ctx)->out;
}

API uint32_t *
lys_ypr_ctx_get_options(const struct lyspr_ctx *ctx)
{
    return &((struct lyspr_ctx *)ctx)->options;
}

API uint16_t *
lys_ypr_ctx_get_level(const struct lyspr_ctx *ctx)
{
    return &((struct lyspr_ctx *)ctx)->level;
}
