/**
 * @file printer_schema.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Generic schema printers functions.
 *
 * Copyright (c) 2015 - 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "printer_schema.h"

#include <stdio.h>

#include "compat.h"
#include "log.h"
#include "ly_common.h"
#include "out_internal.h"
#include "printer_internal.h"
#include "tree_schema.h"

LIBYANG_API_DEF LY_ERR
lys_print_module(struct ly_out *out, const struct lys_module *module, LYS_OUTFORMAT format, size_t line_length,
        uint32_t options)
{
    LY_ERR ret;

    LY_CHECK_ARG_RET(NULL, out, module, LY_EINVAL);

    /* reset number of printed bytes */
    out->func_printed = 0;

    switch (format) {
    case LYS_OUT_YANG:
        if (!module->parsed) {
            LOGERR(module->ctx, LY_EINVAL, "Module \"%s\" parsed module missing.", module->name);
            ret = LY_EINVAL;
            break;
        }

        ret = yang_print_parsed_module(out, module->parsed, options);
        break;
    case LYS_OUT_YANG_COMPILED:
        if (!module->compiled) {
            LOGERR(module->ctx, LY_EINVAL, "Module \"%s\" compiled module missing.", module->name);
            ret = LY_EINVAL;
            break;
        }

        ret = yang_print_compiled(out, module, options);
        break;
    case LYS_OUT_YIN:
        if (!module->parsed) {
            LOGERR(module->ctx, LY_EINVAL, "Module \"%s\" parsed module missing.", module->name);
            ret = LY_EINVAL;
            break;
        }

        ret = yin_print_parsed_module(out, module->parsed, options);
        break;
    case LYS_OUT_TREE:
        if (!module->parsed) {
            LOGERR(module->ctx, LY_EINVAL, "Module \"%s\" parsed module missing.", module->name);
            ret = LY_EINVAL;
            break;
        }
        ret = tree_print_module(out, module, options, line_length);
        break;
    default:
        LOGERR(module->ctx, LY_EINVAL, "Unsupported output format.");
        ret = LY_EINVAL;
        break;
    }

    return ret;
}

LIBYANG_API_DEF LY_ERR
lys_print_submodule(struct ly_out *out, const struct lysp_submodule *submodule, LYS_OUTFORMAT format,
        size_t line_length, uint32_t options)
{
    LY_ERR ret;

    LY_CHECK_ARG_RET(NULL, out, submodule, LY_EINVAL);

    /* reset number of printed bytes */
    out->func_printed = 0;

    switch (format) {
    case LYS_OUT_YANG:
        ret = yang_print_parsed_submodule(out, submodule, options);
        break;
    case LYS_OUT_YIN:
        ret = yin_print_parsed_submodule(out, submodule, options);
        break;
    case LYS_OUT_TREE:
        ret = tree_print_parsed_submodule(out, submodule, options, line_length);
        break;
    default:
        LOGERR(submodule->mod->ctx, LY_EINVAL, "Unsupported output format.");
        ret = LY_EINVAL;
        break;
    }

    return ret;
}

static LY_ERR
lys_print_(struct ly_out *out, const struct lys_module *module, LYS_OUTFORMAT format, uint32_t options)
{
    LY_ERR ret;

    LY_CHECK_ARG_RET(NULL, out, LY_EINVAL);

    ret = lys_print_module(out, module, format, 0, options);

    ly_out_free(out, NULL, 0);
    return ret;
}

LIBYANG_API_DEF LY_ERR
lys_print_mem(char **strp, const struct lys_module *module, LYS_OUTFORMAT format, uint32_t options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, strp, module, LY_EINVAL);

    /* init */
    *strp = NULL;

    LY_CHECK_RET(ly_out_new_memory(strp, 0, &out));
    return lys_print_(out, module, format, options);
}

LIBYANG_API_DEF LY_ERR
lys_print_fd(int fd, const struct lys_module *module, LYS_OUTFORMAT format, uint32_t options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, fd != -1, module, LY_EINVAL);

    LY_CHECK_RET(ly_out_new_fd(fd, &out));
    return lys_print_(out, module, format, options);
}

LIBYANG_API_DEF LY_ERR
lys_print_file(FILE *f, const struct lys_module *module, LYS_OUTFORMAT format, uint32_t options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, f, module, LY_EINVAL);

    LY_CHECK_RET(ly_out_new_file(f, &out));
    return lys_print_(out, module, format, options);
}

LIBYANG_API_DEF LY_ERR
lys_print_path(const char *path, const struct lys_module *module, LYS_OUTFORMAT format, uint32_t options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, path, module, LY_EINVAL);

    LY_CHECK_RET(ly_out_new_filepath(path, &out));
    return lys_print_(out, module, format, options);
}

LIBYANG_API_DEF LY_ERR
lys_print_clb(ly_write_clb writeclb, void *user_data, const struct lys_module *module, LYS_OUTFORMAT format, uint32_t options)
{
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, writeclb, module, LY_EINVAL);

    LY_CHECK_RET(ly_out_new_clb(writeclb, user_data, &out));
    return lys_print_(out, module, format, options);
}

LIBYANG_API_DEF LY_ERR
lys_print_node(struct ly_out *out, const struct lysc_node *node, LYS_OUTFORMAT format, size_t line_length, uint32_t options)
{
    LY_ERR ret;

    LY_CHECK_ARG_RET(NULL, out, node, LY_EINVAL);

    /* reset number of printed bytes */
    out->func_printed = 0;

    switch (format) {
    case LYS_OUT_YANG_COMPILED:
        ret = yang_print_compiled_node(out, node, options);
        break;
    case LYS_OUT_TREE:
        ret = tree_print_compiled_node(out, node, options, line_length);
        break;
    default:
        LOGERR(NULL, LY_EINVAL, "Unsupported output format.");
        ret = LY_EINVAL;
        break;
    }

    return ret;
}
