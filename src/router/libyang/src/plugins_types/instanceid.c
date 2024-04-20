/**
 * @file instanceid.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in instance-identifier type plugin.
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

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"
#include "path.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesInstanceIdentifier instance-identifier (built-in)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | string length | yes | `char *` | string JSON format of the instance-identifier |
 */

/**
 * @brief Convert compiled path (instance-identifier) into string.
 *
 * @param[in] path Compiled path.
 * @param[in] format Value format.
 * @param[in] prefix_data Format-specific data for resolving prefixes.
 * @param[out] str Printed instance-identifier.
 * @return LY_ERR value.
 */
static LY_ERR
instanceid_path2str(const struct ly_path *path, LY_VALUE_FORMAT format, void *prefix_data, char **str)
{
    LY_ERR ret = LY_SUCCESS;
    LY_ARRAY_COUNT_TYPE u, v;
    char *result = NULL, quot;
    const struct lys_module *mod = NULL;
    ly_bool inherit_prefix, d;
    const char *strval;

    switch (format) {
    case LY_VALUE_XML:
    case LY_VALUE_SCHEMA:
    case LY_VALUE_SCHEMA_RESOLVED:
        /* everything is prefixed */
        inherit_prefix = 0;
        break;
    case LY_VALUE_CANON:
    case LY_VALUE_JSON:
    case LY_VALUE_LYB:
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

            switch (path[u].pred_type) {
            case LY_PATH_PREDTYPE_NONE:
                break;
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
            }

            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    if (ret) {
        free(result);
    } else {
        *str = result;
    }
    return ret;
}

API LY_ERR
lyplg_type_store_instanceid(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *unres, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_instanceid *type_inst = (struct lysc_type_instanceid *)type;
    struct ly_path *path;
    char *canon;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    storage->realtype = type;

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* compile instance-identifier into path */
    ret = lyplg_type_lypath_new(ctx, value, value_len, options, format, prefix_data, ctx_node,
            unres, &path, err);
    LY_CHECK_GOTO(ret, cleanup);

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
        ret = instanceid_path2str(path, LY_VALUE_JSON, NULL, &canon);
        LY_CHECK_GOTO(ret, cleanup);

        ret = lydict_insert_zc(ctx, canon, &storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);
    }

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_instanceid(ctx, storage);
    }
    if (!ret && type_inst->require_instance) {
        /* needs to be resolved */
        return LY_EINCOMPLETE;
    } else {
        return ret;
    }
}

API LY_ERR
lyplg_type_validate_instanceid(const struct ly_ctx *ctx, const struct lysc_type *UNUSED(type),
        const struct lyd_node *UNUSED(ctx_node), const struct lyd_node *tree, struct lyd_value *storage,
        struct ly_err_item **err)
{
    struct lysc_type_instanceid *type_inst = (struct lysc_type_instanceid *)storage->realtype;
    LY_ERR ret = LY_SUCCESS;

    *err = NULL;

    if (!type_inst->require_instance) {
        /* redundant to resolve */
        return LY_SUCCESS;
    }

    /* find the target in data */
    if ((ret = ly_path_eval(storage->target, tree, NULL))) {
        const char *value = lyplg_type_print_instanceid(ctx, storage, LY_VALUE_CANON, NULL, NULL, NULL);
        return ly_err_new(err, ret, LYVE_DATA, NULL, NULL, LY_ERRMSG_NOINST, value);
    }

    return LY_SUCCESS;
}

API LY_ERR
lyplg_type_compare_instanceid(const struct lyd_value *val1, const struct lyd_value *val2)
{
    LY_ARRAY_COUNT_TYPE u, v;

    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }

    if (val1 == val2) {
        return LY_SUCCESS;
    } else if (LY_ARRAY_COUNT(val1->target) != LY_ARRAY_COUNT(val2->target)) {
        return LY_ENOT;
    }

    LY_ARRAY_FOR(val1->target, u) {
        struct ly_path *s1 = &val1->target[u];
        struct ly_path *s2 = &val2->target[u];

        if ((s1->node != s2->node) || (s1->pred_type != s2->pred_type) ||
                (s1->predicates && (LY_ARRAY_COUNT(s1->predicates) != LY_ARRAY_COUNT(s2->predicates)))) {
            return LY_ENOT;
        }
        if (s1->predicates) {
            LY_ARRAY_FOR(s1->predicates, v) {
                struct ly_path_predicate *pred1 = &s1->predicates[v];
                struct ly_path_predicate *pred2 = &s2->predicates[v];

                switch (s1->pred_type) {
                case LY_PATH_PREDTYPE_NONE:
                    break;
                case LY_PATH_PREDTYPE_POSITION:
                    /* position predicate */
                    if (pred1->position != pred2->position) {
                        return LY_ENOT;
                    }
                    break;
                case LY_PATH_PREDTYPE_LIST:
                    /* key-predicate */
                    if ((pred1->key != pred2->key) ||
                            ((struct lysc_node_leaf *)pred1->key)->type->plugin->compare(&pred1->value, &pred2->value)) {
                        return LY_ENOT;
                    }
                    break;
                case LY_PATH_PREDTYPE_LEAFLIST:
                    /* leaf-list predicate */
                    if (((struct lysc_node_leaflist *)s1->node)->type->plugin->compare(&pred1->value, &pred2->value)) {
                        return LY_ENOT;
                    }
                }
            }
        }
    }

    return LY_SUCCESS;
}

API const void *
lyplg_type_print_instanceid(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *value, LY_VALUE_FORMAT format,
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
    if (instanceid_path2str(value->target, format, prefix_data, &ret)) {
        return NULL;
    }
    *dynamic = 1;
    if (value_len) {
        *value_len = strlen(ret);
    }
    return ret;
}

API LY_ERR
lyplg_type_dup_instanceid(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ERR ret;

    memset(dup, 0, sizeof *dup);

    /* canonical value */
    ret = lydict_insert(ctx, original->_canonical, ly_strlen(original->_canonical), &dup->_canonical);
    LY_CHECK_GOTO(ret, error);

    /* copy path */
    ret = ly_path_dup(ctx, original->target, &dup->target);
    LY_CHECK_GOTO(ret, error);

    dup->realtype = original->realtype;
    return LY_SUCCESS;

error:
    lyplg_type_free_instanceid(ctx, dup);
    return ret;
}

API void
lyplg_type_free_instanceid(const struct ly_ctx *ctx, struct lyd_value *value)
{
    lydict_remove(ctx, value->_canonical);
    ly_path_free(ctx, value->target);
}

/**
 * @brief Plugin information for instance-identifier type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_instanceid[] = {
    {
        .module = "",
        .revision = NULL,
        .name = LY_TYPE_INST_STR,

        .plugin.id = "libyang 2 - instance-identifier, version 1",
        .plugin.store = lyplg_type_store_instanceid,
        .plugin.validate = lyplg_type_validate_instanceid,
        .plugin.compare = lyplg_type_compare_instanceid,
        .plugin.sort = NULL,
        .plugin.print = lyplg_type_print_instanceid,
        .plugin.duplicate = lyplg_type_dup_instanceid,
        .plugin.free = lyplg_type_free_instanceid
    },
    {0}
};
