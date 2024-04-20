/**
 * @file instanceid_keys.c
 * @author Michal Basko <mvasko@cesnet.cz>
 * @brief ietf-netconf edit-config key metadata instance-identifier keys predicate type plugin.
 *
 * Copyright (c) 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#define _GNU_SOURCE /* strdup */

#include "plugins_types.h"

#include <stdint.h>
#include <stdlib.h>

#include "libyang.h"

/* additional internal headers for some useful simple macros */
#include "common.h"
#include "compat.h"
#include "path.h"
#include "plugins_internal.h" /* LY_TYPE_*_STR */
#include "xpath.h"

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesInstanceIdentifierKeys instance-identifier-keys (yang)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | string length | yes | `char *` | string JSON format of the instance-identifier keys predicate |
 */

/**
 * @brief Special lyd_value structure for yang instance-identifier-keys values.
 */
struct lyd_value_instance_identifier_keys {
    struct lyxp_expr *keys;
    const struct ly_ctx *ctx;
    void *prefix_data;
    LY_VALUE_FORMAT format;
};

/**
 * @brief Print instance-id-keys value in the specific format.
 *
 * @param[in] val instance-id-keys value structure.
 * @param[in] format Format to print in.
 * @param[in] prefix_data Format-specific prefix data.
 * @param[out] str_value Printed value.
 * @param[out] err Error structure on error.
 * @return LY_ERR value.
 */
static LY_ERR
instanceid_keys_print_value(const struct lyd_value_instance_identifier_keys *val, LY_VALUE_FORMAT format, void *prefix_data,
        char **str_value, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    uint32_t cur_idx = 0, str_len = 0;
    enum lyxp_token cur_tok;
    char *str_tok;
    void *mem;
    const char *cur_exp_ptr;
    ly_bool is_nt;
    const struct lys_module *context_mod = NULL, *local_mod = NULL;
    struct ly_set *mods;

    *str_value = NULL;

    if (format == LY_VALUE_XML) {
        /* null the local module so that all the prefixes are printed */
        mods = prefix_data;
        local_mod = mods->objs[0];
        mods->objs[0] = NULL;
    }

    while (cur_idx < val->keys->used) {
        cur_tok = val->keys->tokens[cur_idx];
        cur_exp_ptr = val->keys->expr + val->keys->tok_pos[cur_idx];

        if ((cur_tok == LYXP_TOKEN_NAMETEST) || (cur_tok == LYXP_TOKEN_LITERAL)) {
            /* tokens that may include prefixes, get them in the target format */
            is_nt = (cur_tok == LYXP_TOKEN_NAMETEST) ? 1 : 0;
            LY_CHECK_GOTO(ret = lyplg_type_xpath10_print_token(cur_exp_ptr, val->keys->tok_len[cur_idx], is_nt, &context_mod,
                    val->ctx, val->format, val->prefix_data, format, prefix_data, &str_tok, err), cleanup);

            /* append the converted token */
            mem = realloc(*str_value, str_len + strlen(str_tok) + 1);
            if (!mem) {
                free(str_tok);
                ret = ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory.");
                goto cleanup;
            }
            *str_value = mem;
            str_len += sprintf(*str_value + str_len, "%s", str_tok);
            free(str_tok);

            /* token processed */
            ++cur_idx;
        } else {
            /* just copy the token */
            mem = realloc(*str_value, str_len + val->keys->tok_len[cur_idx] + 1);
            if (!mem) {
                ret = ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory.");
                goto cleanup;
            }
            *str_value = mem;
            str_len += sprintf(*str_value + str_len, "%.*s", (int)val->keys->tok_len[cur_idx], cur_exp_ptr);

            /* token processed */
            ++cur_idx;
        }
    }

cleanup:
    if (local_mod) {
        mods->objs[0] = (void *)local_mod;
    }
    if (ret) {
        free(*str_value);
        *str_value = NULL;
    }
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_store_clb for the instance-identifier-keys yang type.
 */
static LY_ERR
lyplg_type_store_instanceid_keys(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *UNUSED(ctx_node),
        struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres), struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_str *type_str = (struct lysc_type_str *)type;
    struct lyd_value_instance_identifier_keys *val;
    uint32_t log_opts = LY_LOSTORE;
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

    /* parse instance-identifier keys, with optional prefix even though it should be mandatory */
    if (value_len && (((char *)value)[0] != '[')) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid first character '%c', list key predicates expected.",
                ((char *)value)[0]);
        goto cleanup;
    }

    /* do not log */
    ly_temp_log_options(&log_opts);
    ret = ly_path_parse_predicate(ctx, NULL, value_len ? value : "", value_len, LY_PATH_PREFIX_OPTIONAL,
            LY_PATH_PRED_KEYS, &val->keys);
    ly_temp_log_options(NULL);
    if (ret) {
        ret = ly_err_new(err, ret, LYVE_DATA, NULL, NULL, "%s", ly_errmsg(ctx));
        ly_err_clean((struct ly_ctx *)ctx, NULL);
        goto cleanup;
    }
    val->ctx = ctx;

    /* store format-specific data and context for later prefix resolution */
    ret = lyplg_type_prefix_data_new(ctx, value, value_len, format, prefix_data, &val->format, &val->prefix_data);
    LY_CHECK_GOTO(ret, cleanup);

    switch (format) {
    case LY_VALUE_CANON:
    case LY_VALUE_JSON:
    case LY_VALUE_LYB:
    case LY_VALUE_STR_NS:
        /* store canonical value */
        if (options & LYPLG_TYPE_STORE_DYNAMIC) {
            ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
            options &= ~LYPLG_TYPE_STORE_DYNAMIC;
            LY_CHECK_GOTO(ret, cleanup);
        } else {
            ret = lydict_insert(ctx, value_len ? value : "", value_len, &storage->_canonical);
            LY_CHECK_GOTO(ret, cleanup);
        }
        break;
    case LY_VALUE_SCHEMA:
    case LY_VALUE_SCHEMA_RESOLVED:
    case LY_VALUE_XML:
        /* JSON format with prefix is the canonical one */
        ret = instanceid_keys_print_value(val, LY_VALUE_JSON, NULL, &canon, err);
        LY_CHECK_GOTO(ret, cleanup);

        ret = lydict_insert_zc(ctx, canon, &storage->_canonical);
        LY_CHECK_GOTO(ret, cleanup);
        break;
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

/**
 * @brief Plugin information for instance-identifier type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_instanceid_keys[] = {
    {
        .module = "yang",
        .revision = NULL,
        .name = "instance-identifier-keys",

        .plugin.id = "libyang 2 - instance-identifier-keys, version 1",
        .plugin.store = lyplg_type_store_instanceid_keys,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.sort = NULL,
        .plugin.print = lyplg_type_print_xpath10,
        .plugin.duplicate = lyplg_type_dup_xpath10,
        .plugin.free = lyplg_type_free_xpath10,
        .plugin.lyb_data_len = -1,
    },
    {0}
};
