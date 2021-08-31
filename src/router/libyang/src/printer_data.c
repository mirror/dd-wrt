/**
 * @file printer_data.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Generic data printers functions.
 *
 * Copyright (c) 2015 - 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "printer_data.h"

#include <stdio.h>

#include "common.h"
#include "log.h"
#include "out.h"
#include "out_internal.h"
#include "printer_internal.h"
#include "tree_data.h"

static LY_ERR
lyd_print_(struct ly_out *out, const struct lyd_node *root, LYD_FORMAT format, uint32_t options)
{
    LY_ERR ret = LY_SUCCESS;

    switch (format) {
    case LYD_XML:
        ret = xml_print_data(out, root, options);
        break;
    case LYD_JSON:
        ret = json_print_data(out, root, options);
        break;
    case LYD_LYB:
        ret = lyb_print_data(out, root, options);
        break;
    case LYD_UNKNOWN:
        LOGINT(root ? LYD_CTX(root) : NULL);
        ret = LY_EINT;
        break;
    }

    return ret;
}

API LY_ERR
lyd_print_all(struct ly_out *out, const struct lyd_node *root, LYD_FORMAT format, uint32_t options)
{
    LY_CHECK_ARG_RET(NULL, out, !(options & LYD_PRINT_WITHSIBLINGS), LY_EINVAL);

    /* reset the number of printed bytes */
    out->func_printed = 0;

    if (root) {
        /* get first top-level sibling */
        while (root->parent) {
            root = lyd_parent(root);
        }
        while (root->prev->next) {
            root = root->prev;
        }
    }

    /* print each top-level sibling */
    LY_CHECK_RET(lyd_print_(out, root, format, options | LYD_PRINT_WITHSIBLINGS));

    return LY_SUCCESS;
}

API LY_ERR
lyd_print_tree(struct ly_out *out, const struct lyd_node *root, LYD_FORMAT format, uint32_t options)
{
    LY_CHECK_ARG_RET(NULL, out, !(options & LYD_PRINT_WITHSIBLINGS), LY_EINVAL);

    /* reset the number of printed bytes */
    out->func_printed = 0;

    /* print the subtree */
    LY_CHECK_RET(lyd_print_(out, root, format, options));

    return LY_SUCCESS;
}

API LY_ERR
lyd_print_mem(char **strp, const struct lyd_node *root, LYD_FORMAT format, uint32_t options)
{
    LY_ERR ret;
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, strp, LY_EINVAL);

    /* init */
    *strp = NULL;

    LY_CHECK_RET(ly_out_new_memory(strp, 0, &out));
    ret = lyd_print_(out, root, format, options);
    ly_out_free(out, NULL, 0);
    return ret;
}

API LY_ERR
lyd_print_fd(int fd, const struct lyd_node *root, LYD_FORMAT format, uint32_t options)
{
    LY_ERR ret;
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, fd != -1, LY_EINVAL);

    LY_CHECK_RET(ly_out_new_fd(fd, &out));
    ret = lyd_print_(out, root, format, options);
    ly_out_free(out, NULL, 0);
    return ret;
}

API LY_ERR
lyd_print_file(FILE *f, const struct lyd_node *root, LYD_FORMAT format, uint32_t options)
{
    LY_ERR ret;
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, f, LY_EINVAL);

    LY_CHECK_RET(ly_out_new_file(f, &out));
    ret = lyd_print_(out, root, format, options);
    ly_out_free(out, NULL, 0);
    return ret;
}

API LY_ERR
lyd_print_path(const char *path, const struct lyd_node *root, LYD_FORMAT format, uint32_t options)
{
    LY_ERR ret;
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, path, LY_EINVAL);

    LY_CHECK_RET(ly_out_new_filepath(path, &out));
    ret = lyd_print_(out, root, format, options);
    ly_out_free(out, NULL, 0);
    return ret;
}

API LY_ERR
lyd_print_clb(ly_write_clb writeclb, void *user_data, const struct lyd_node *root, LYD_FORMAT format, uint32_t options)
{
    LY_ERR ret;
    struct ly_out *out;

    LY_CHECK_ARG_RET(NULL, writeclb, LY_EINVAL);

    LY_CHECK_RET(ly_out_new_clb(writeclb, user_data, &out));
    ret = lyd_print_(out, root, format, options);
    ly_out_free(out, NULL, 0);
    return ret;
}
