/**
 * @file node_instanceid.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-netconf-acm node-instance-identifier type plugin.
 *
 * Copyright (c) 2019-2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "plugins_types.h"

#include <stdint.h>
#include <stdlib.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "compat.h"
#include "ly_common.h"
#include "path.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */
#include "xpath.h"

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesNodeInstanceIdentifier node-instance-identifier (ietf-netconf-acm)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | string length | yes | `char *` | string JSON format of the instance-identifier |
 */

/**
 * @brief Convert compiled path (node-instance-identifier) or NULL ("/") into string.
 *
 * @param[in] path Compiled path.
 * @param[in] format Value format.
 * @param[in] prefix_data Format-specific data for resolving prefixes.
 * @param[out] str Printed instance-identifier.
 * @return LY_ERR value.
 */
static LY_ERR
node_instanceid_path2str(const struct ly_path *path, LY_VALUE_FORMAT format, void *prefix_data, char **str)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    char *result = NULL, quot;
    const struct lys_module *mod = NULL, *local_mod = NULL;
    struct ly_set *mods;
    ly_bool inherit_prefix = 0, d;
    const char *strval;

    if (!path) {
        /* special path */
        ret = ly_strcat(&result, "/");
        goto cleanup;
    }

    switch (format) {
    case LY_VALUE_XML:
        /* null the local module so that all the prefixes are printed */
        mods = prefix_data;
        local_mod = mods->objs[0];
        mods->objs[0] = NULL;

    /* fallthrough */
    case LY_VALUE_SCHEMA:
    case LY_VALUE_SCHEMA_RESOLVED:
        /* everything is prefixed */
        inherit_prefix = 0;
        break;
    case LY_VALUE_CANON:
    case LY_VALUE_JSON:
    case LY_VALUE_LYB:
    case LY_VALUE_STR_NS:
        /* the same prefix is inherited and skipped */
        inherit_prefix = 1;
        break;
    }

    LY_ARRAY_FOR(path, u) {
        /* new node */
        if (!inherit_prefix || (mod != path[u].node->module)) {
            mod = path[u].node->module;
            ret = ly_strcat(&result, "/%s:%s", lyplg_type_get_prefix(mod, format, prefix_data), path[u].node->name);
        } else {
            ret = ly_strcat(&result, "/%s", path[u].node->name);
        }
        LY_CHECK_GOTO(ret, cleanup);

        /* node predicates */
        LY_ARRAY_FOR(path[u].predicates, v) {
            struct ly_path_predicate *pred = &path[u].predicates[v];

            switch (pred->type) {
            case LY_PATH_PREDTYPE_POSITION:
                /* position predicate */
                ret = ly_strcat(&result, "[%" PRIu64 "]", pred->position);
                break;
            case LY_PATH_PREDTYPE_LIST:
                /* key-predicate */
                strval = pred->value.realtype->plugin->print(path[u].node->module->ctx, &pred->value, format, prefix_data,
                        &d, NULL);

                /* default quote */
                quot = '\'';
                if (strchr(strval, quot)) {
                    quot = '"';
                }
                if (inherit_prefix) {
                    /* always the same prefix as the parent */
                    ret = ly_strcat(&result, "[%s=%c%s%c]", pred->key->name, quot, strval, quot);
                } else {
                    ret = ly_strcat(&result, "[%s:%s=%c%s%c]", lyplg_type_get_prefix(pred->key->module, format, prefix_data),
                            pred->key->name, quot, strval, quot);
                }
                if (d) {
                    free((char *)strval);
                }
                break;
            case LY_PATH_PREDTYPE_LEAFLIST:
                /* leaf-list-predicate */
                strval = pred->value.realtype->plugin->print(path[u].node->module->ctx, &pred->value, format, prefix_data,
                        &d, NULL);

                /* default quote */
                quot = '\'';
                if (strchr(strval, quot)) {
                    quot = '"';
                }
                ret = ly_strcat(&result, "[.=%c%s%c]", quot, strval, quot);
                if (d) {
                    free((char *)strval);
                }
                break;
            case LY_PATH_PREDTYPE_LIST_VAR:
                /* key-predicate with a variable */
                if (inherit_prefix) {
                    /* always the same prefix as the parent */
                    ret = ly_strcat(&result, "[%s=$%s]", pred->key->name, pred->variable);
                } else {
                    ret = ly_strcat(&result, "[%s:%s=$%s]", lyplg_type_get_prefix(pred->key->module, format, prefix_data),
                            pred->key->name, pred->variable);
                }
                break;
            }

            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    if (local_mod) {
        mods->objs[0] = (void *)local_mod;
    }
    if (ret) {
        free(result);
    } else {
        *str = result;
    }
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_store_clb for the node-instance-identifier ietf-netconf-acm type.
 */
static LY_ERR
lyplg_type_store_node_instanceid(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_expr *exp = NULL;
    uint32_t prefix_opt = 0;
    struct ly_path *path = NULL;
    char *canon;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    storage->realtype = type;

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret, cleanup);

    if ((((char *)value)[0] == '/') && (value_len == 1)) {
        /* special path */
        goto store;
    }

    switch (format) {
    case LY_VALUE_SCHEMA:
    case LY_VALUE_SCHEMA_RESOLVED:
    case LY_VALUE_XML:
        prefix_opt = LY_PATH_PREFIX_MANDATORY;
        break;
    case LY_VALUE_CANON:
    case LY_VALUE_LYB:
    case LY_VALUE_JSON:
    case LY_VALUE_STR_NS:
        prefix_opt = LY_PATH_PREFIX_STRICT_INHERIT;
        break;
    }

    /* parse the value */
    ret = ly_path_parse(ctx, ctx_node, value, value_len, 0, LY_PATH_BEGIN_ABSOLUTE, prefix_opt, LY_PATH_PRED_SIMPLE, &exp);
    if (ret) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                "Invalid node-instance-identifier \"%.*s\" value - syntax error.", (int)value_len, (char *)value);
        goto cleanup;
    }

    if (options & LYPLG_TYPE_STORE_IMPLEMENT) {
        /* implement all prefixes */
        LY_CHECK_GOTO(ret = lys_compile_expr_implement(ctx, exp, format, prefix_data, 1, unres, NULL), cleanup);
    }

    /* resolve it on schema tree, use JSON format instead of LYB because for this type they are equal but for some
     * nested types (such as numbers in predicates in the path) LYB would be invalid */
    ret = ly_path_compile(ctx, NULL, ctx_node, NULL, exp, (ctx_node && (ctx_node->flags & LYS_IS_OUTPUT)) ?
            LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT, LY_PATH_TARGET_MANY, 1, (format == LY_VALUE_LYB) ?
            LY_VALUE_JSON : format, prefix_data, &path);
    if (ret) {
        ret = ly_err_new(err, ret, LYVE_DATA, NULL, NULL,
                "Invalid node-instance-identifier \"%.*s\" value - semantic error.", (int)value_len, (char *)value);
        goto cleanup;
    }

store:
    /* store value */
    storage->target = path;

    /* store canonical value */
    if (format == LY_VALUE_CANON) {
        if (options & LYPLG_TYPE_STORE_DYNAMIC) {
            ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
            options &= ~LYPLG_TYPE_STORE_DYNAMIC;
            LY_CHECK_GOTO(ret, cleanup);
        } else {
            ret = lydict_insert(ctx, value, value_len, &storage->_canonical);
            LY_CHECK_GOTO(ret, cleanup);
        }
    } else {
        /* JSON format with prefix is the canonical one */
        ret = node_instanceid_path2str(path, LY_VALUE_JSON, NULL, &canon);
        LY_CHECK_GOTO(ret, cleanup);

        ret = lydict_insert_zc(ctx, canon, &storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);
    }

cleanup:
    lyxp_expr_free(ctx, exp);
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_instanceid(ctx, storage);
    }
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_print_clb for the node-instance-identifier ietf-netconf-acm type.
 */
static const void *
lyplg_type_print_node_instanceid(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *prefix_data, ly_bool *dynamic, size_t *value_len)
{
    char *ret;

    if ((format == LY_VALUE_CANON) || (format == LY_VALUE_JSON) || (format == LY_VALUE_LYB)) {
        if (dynamic) {
            *dynamic = 0;
        }
        if (value_len) {
            *value_len = strlen(value->_canonical);
        }
        return value->_canonical;
    }

    /* print the value in the specific format */
    if (node_instanceid_path2str(value->target, format, prefix_data, &ret)) {
        return NULL;
    }
    *dynamic = 1;
    if (value_len) {
        *value_len = strlen(ret);
    }
    return ret;
}

/**
 * @brief Plugin information for instance-identifier type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_node_instanceid[] = {
    {
        .module = "ietf-netconf-acm",
        .revision = "2012-02-22",
        .name = "node-instance-identifier",

        .plugin.id = "libyang 2 - node-instance-identifier, version 1",
        .plugin.store = lyplg_type_store_node_instanceid,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_instanceid,
        .plugin.sort = lyplg_type_sort_instanceid,
        .plugin.print = lyplg_type_print_node_instanceid,
        .plugin.duplicate = lyplg_type_dup_instanceid,
        .plugin.free = lyplg_type_free_instanceid,
        .plugin.lyb_data_len = -1,
    },
    {
        .module = "ietf-netconf-acm",
        .revision = "2018-02-14",
        .name = "node-instance-identifier",

        .plugin.id = "libyang 2 - node-instance-identifier, version 1",
        .plugin.store = lyplg_type_store_node_instanceid,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_instanceid,
        .plugin.sort = lyplg_type_sort_instanceid,
        .plugin.print = lyplg_type_print_node_instanceid,
        .plugin.duplicate = lyplg_type_dup_instanceid,
        .plugin.free = lyplg_type_free_instanceid,
        .plugin.lyb_data_len = -1,
    },
    {0}
};
