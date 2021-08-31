/**
 * @file xpath1.0.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-yang-types xpath1.0 type plugin.
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "plugins_types.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

#include "common.h"
#include "compat.h"

/* internal header */
#include "xpath.h"

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesXpath10 xpath1.0 (ietf-yang-types)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | string length | yes | `char *` | string JSON format of the XPath expression |
 */

/**
 * @brief Stored value structure for xpath1.0
 */
struct lyd_value_xpath10 {
    struct lyxp_expr *exp;
    const struct ly_ctx *ctx;
    void *prefix_data;
    LY_VALUE_FORMAT format;
};

/**
 * @brief Print xpath1.0 token in the specific format.
 *
 * @param[in] token Token to transform.
 * @param[in] tok_len Lenghth of @p token.
 * @param[in] is_nametest Whether the token is a nametest, it then always requires a prefix in XML @p get_format.
 * @param[in] resolve_ctx Context to use for resolving prefixes.
 * @param[in] resolve_format Format of the resolved prefixes.
 * @param[in] resolve_prefix_data Resolved prefixes prefix data.
 * @param[in] get_format Format of the output prefixes.
 * @param[in] get_prefix_data Format-specific prefix data for the output.
 * @param[in,out] prev_prefix Prefix of a previous nametest.
 * @param[out] token_p Printed token.
 * @param[out] err Error structure on error.
 * @return LY_ERR value.
 */
static LY_ERR
xpath10_print_token(const char *token, uint16_t tok_len, ly_bool is_nametest, const struct ly_ctx *resolve_ctx,
        LY_VALUE_FORMAT resolve_format, const void *resolve_prefix_data, LY_VALUE_FORMAT get_format,
        void *get_prefix_data, const char **prev_prefix, char **token_p, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    const char *str_begin, *str_next, *prefix;
    ly_bool is_prefix;
    uint32_t len;
    char *str = NULL;
    void *mem;
    uint32_t str_len = 0;
    const struct lys_module *mod;

    str_begin = token;

    while (!(ret = ly_value_prefix_next(str_begin, token + tok_len, &len, &is_prefix, &str_next)) && len) {

        if (is_prefix) {
            /* resolve the module in the original format */
            mod = lyplg_type_identity_module(resolve_ctx, NULL, str_begin, len, resolve_format, resolve_prefix_data);
            if (!mod) {
                ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Failed to resolve prefix \"%.*s\".", len, str_begin);
                goto cleanup;
            }

            /* get the prefix in the target format */
            prefix = lyplg_type_get_prefix(mod, get_format, get_prefix_data);
            if (!prefix) {
                ret = ly_err_new(err, LY_EINT, LYVE_DATA, NULL, NULL, "Internal error.");
                goto cleanup;
            }

            /* append the prefix */
            mem = realloc(str, str_len + strlen(prefix) + 2);
            LY_CHECK_ERR_GOTO(!mem, ret = ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory."), cleanup);
            str = mem;
            sprintf(str + str_len, "%s:", prefix);
            str_len += strlen(prefix) + 1;

            if (is_nametest) {
                /* remember prefix of a nametest */
                *prev_prefix = prefix;
            }
        } else if (is_nametest && (get_format == LY_VALUE_XML) && (len == tok_len) && *prev_prefix) {
            /* nametest without a prefix, we must add it */
            mem = realloc(str, str_len + strlen(*prev_prefix) + 1 + len + 1);
            LY_CHECK_ERR_GOTO(!mem, ret = ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory."), cleanup);
            str = mem;
            sprintf(str + str_len, "%s:%.*s", *prev_prefix, len, str_begin);
            str_len += strlen(*prev_prefix) + 1 + len;
        } else {
            /* just append the string */
            mem = realloc(str, str_len + len + 1);
            LY_CHECK_ERR_GOTO(!mem, ret = ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory."), cleanup);
            str = mem;
            sprintf(str + str_len, "%.*s", len, str_begin);
            str_len += len;
        }

        str_begin = str_next;
    }

cleanup:
    if (ret) {
        free(str);
    } else {
        *token_p = str;
    }
    return ret;
}

/**
 * @brief Print xpath1.0 value in the specific format.
 *
 * @param[in] xp_val xpath1.0 value structure.
 * @param[in] format Format to print in.
 * @param[in] prefix_data Format-specific prefix data.
 * @param[out] str_value Printed value.
 * @param[out] err Error structure on error.
 * @return LY_ERR value.
 */
static LY_ERR
xpath10_print_value(const struct lyd_value_xpath10 *xp_val, LY_VALUE_FORMAT format, void *prefix_data,
        char **str_value, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    char *str = NULL, *token;
    void *mem;
    const char *str_exp_ptr = xp_val->exp->expr, *cur_exp_ptr, *prev_prefix = NULL;
    uint32_t str_len = 0, len;
    uint16_t idx;
    ly_bool is_nt;

    for (idx = 0; idx < xp_val->exp->used; ++idx) {
        cur_exp_ptr = xp_val->exp->expr + xp_val->exp->tok_pos[idx];

        /* only these tokens may include prefixes */
        if ((xp_val->exp->tokens[idx] == LYXP_TOKEN_NAMETEST) || (xp_val->exp->tokens[idx] == LYXP_TOKEN_LITERAL)) {
            /* append preceding expression */
            len = cur_exp_ptr - str_exp_ptr;
            mem = realloc(str, str_len + len + 1);
            LY_CHECK_ERR_GOTO(!mem, ret = ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory."), cleanup);
            str = mem;
            sprintf(str + str_len, "%.*s", len, str_exp_ptr);
            str_len += len;
            str_exp_ptr = cur_exp_ptr;

            /* get the token in the target format */
            is_nt = (xp_val->exp->tokens[idx] == LYXP_TOKEN_NAMETEST) ? 1 : 0;
            ret = xpath10_print_token(cur_exp_ptr, xp_val->exp->tok_len[idx], is_nt, xp_val->ctx, xp_val->format,
                    xp_val->prefix_data, format, prefix_data, &prev_prefix, &token, err);
            LY_CHECK_GOTO(ret, cleanup);

            /* append the converted token */
            mem = realloc(str, str_len + strlen(token) + 1);
            LY_CHECK_ERR_GOTO(!mem, free(token); ret = ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory."), cleanup);
            str = mem;
            sprintf(str + str_len, "%s", token);
            str_len += strlen(token);
            str_exp_ptr += xp_val->exp->tok_len[idx];
            free(token);
        }
    }

    /* append the rest of the expression */
    if (str_exp_ptr[0]) {
        mem = realloc(str, str_len + strlen(str_exp_ptr) + 1);
        LY_CHECK_ERR_GOTO(!mem, ret = ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory."), cleanup);
        str = mem;
        sprintf(str + str_len, "%s", str_exp_ptr);
    }

cleanup:
    if (ret) {
        free(str);
    } else {
        *str_value = str;
    }
    return ret;
}

API LY_ERR
lyplg_type_store_xpath10(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_str *type_str = (struct lysc_type_str *)type;
    struct lyd_value_xpath10 *val;
    char *canon;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    LYPLG_TYPE_VAL_INLINE_PREPARE(storage, val);
    LY_CHECK_ERR_GOTO(!val, ret = LY_EMEM, cleanup);
    storage->realtype = type;

    /* check hints */
    ret = lyplg_type_check_hints(hints, value, value_len, type->basetype, NULL, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* length restriction of the string */
    if (type_str->length) {
        /* value_len is in bytes, but we need number of characters here */
        ret = lyplg_type_validate_range(LY_TYPE_STRING, type_str->length, ly_utf8len(value, value_len), value, value_len, err);
        LY_CHECK_GOTO(ret, cleanup);
    }

    /* pattern restrictions */
    ret = lyplg_type_validate_patterns(type_str->patterns, value, value_len, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* store format-specific data and context for later prefix resolution */
    ret = lyplg_type_prefix_data_new(ctx, value, value_len, format, prefix_data, &val->format, &val->prefix_data);
    LY_CHECK_GOTO(ret, cleanup);
    val->ctx = ctx;

    /* parse */
    ret = lyxp_expr_parse(ctx, value, value_len, 1, &val->exp);
    LY_CHECK_GOTO(ret, cleanup);

    /* store canonical value */
    if ((format == LY_VALUE_CANON) || (format == LY_VALUE_JSON) || (format == LY_VALUE_LYB)) {
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
        ret = xpath10_print_value(val, LY_VALUE_JSON, NULL, &canon, err);
        LY_CHECK_GOTO(ret, cleanup);

        ret = lydict_insert_zc(ctx, canon, &storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);
    }

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_xpath10(ctx, storage);
    }
    return ret;
}

API const void *
lyplg_type_print_xpath10(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *prefix_data, ly_bool *dynamic, size_t *value_len)
{
    struct lyd_value_xpath10 *val;
    char *ret;
    struct ly_err_item *err = NULL;

    LYD_VALUE_GET(value, val);

    if ((format == LY_VALUE_CANON) || (format == LY_VALUE_JSON) || (format == LY_VALUE_LYB)) {
        /* canonical */
        if (dynamic) {
            *dynamic = 0;
        }
        if (value_len) {
            *value_len = strlen(value->_canonical);
        }
        return value->_canonical;
    }

    /* print in the specific format */
    if (xpath10_print_value(val, format, prefix_data, &ret, &err)) {
        if (err) {
            LOGVAL_ERRITEM(ctx, err);
            ly_err_free(err);
        }
        return NULL;
    }

    *dynamic = 1;
    if (value_len) {
        *value_len = strlen(ret);
    }
    return ret;
}

API LY_ERR
lyplg_type_dup_xpath10(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_value_xpath10 *orig_val, *dup_val;

    /* init dup value */
    memset(dup, 0, sizeof *dup);
    dup->realtype = original->realtype;

    ret = lydict_insert(ctx, original->_canonical, ly_strlen(original->_canonical), &dup->_canonical);
    LY_CHECK_GOTO(ret, cleanup);

    LYPLG_TYPE_VAL_INLINE_PREPARE(dup, dup_val);
    LY_CHECK_ERR_GOTO(!dup_val, LOGMEM(ctx); ret = LY_EMEM, cleanup);
    dup_val->ctx = ctx;

    LYD_VALUE_GET(original, orig_val);
    ret = lyxp_expr_dup(ctx, orig_val->exp, &dup_val->exp);
    LY_CHECK_GOTO(ret, cleanup);

    ret = lyplg_type_prefix_data_dup(ctx, orig_val->format, orig_val->prefix_data, &dup_val->prefix_data);
    LY_CHECK_GOTO(ret, cleanup);
    dup_val->format = orig_val->format;

cleanup:
    if (ret) {
        lyplg_type_free_xpath10(ctx, dup);
    }
    return ret;
}

API void
lyplg_type_free_xpath10(const struct ly_ctx *ctx, struct lyd_value *value)
{
    struct lyd_value_xpath10 *val;

    lydict_remove(ctx, value->_canonical);
    LYD_VALUE_GET(value, val);
    if (val) {
        lyxp_expr_free(ctx, val->exp);
        lyplg_type_prefix_data_free(val->format, val->prefix_data);

        LYPLG_TYPE_VAL_INLINE_DESTROY(val);
    }
}

/**
 * @brief Plugin information for xpath1.0 type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_xpath10[] = {
    {
        .module = "ietf-yang-types",
        .revision = "2013-07-15",
        .name = "xpath1.0",

        .plugin.id = "libyang 2 - xpath1.0, version 1",
        .plugin.store = lyplg_type_store_xpath10,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.sort = NULL,
        .plugin.print = lyplg_type_print_xpath10,
        .plugin.duplicate = lyplg_type_dup_xpath10,
        .plugin.free = lyplg_type_free_xpath10
    },
    {0}
};
