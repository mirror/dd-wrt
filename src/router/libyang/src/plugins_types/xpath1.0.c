/**
 * @file xpath1.0.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-yang-types xpath1.0 type plugin.
 *
 * Copyright (c) 2021 - 2023 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "plugins_types.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

#include "compat.h"
#include "ly_common.h"
#include "xml.h"
#include "xpath.h"

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesXpath10 xpath1.0 (ietf-yang-types)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | string length | yes | `char *` | string JSON format of the XPath expression |
 */

LIBYANG_API_DEF LY_ERR
lyplg_type_xpath10_print_token(const char *token, uint16_t tok_len, ly_bool is_nametest, const struct lys_module **context_mod,
        const struct ly_ctx *resolve_ctx, LY_VALUE_FORMAT resolve_format, const void *resolve_prefix_data,
        LY_VALUE_FORMAT get_format, void *get_prefix_data, char **token_p, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    const char *str_begin, *str_next, *prefix;
    ly_bool is_prefix, has_prefix = 0;
    char *str = NULL;
    void *mem;
    uint32_t len, str_len = 0, pref_len;
    const struct lys_module *mod;

    str_begin = token;

    while (!(ret = ly_value_prefix_next(str_begin, token + tok_len, &len, &is_prefix, &str_next)) && len) {
        if (!is_prefix) {
            if (!has_prefix && is_nametest && (get_format == LY_VALUE_XML) && *context_mod) {
                /* get the prefix */
                prefix = lyplg_type_get_prefix(*context_mod, get_format, get_prefix_data);
                assert(prefix);

                /* append the nametest and prefix */
                mem = realloc(str, str_len + strlen(prefix) + 1 + len + 1);
                LY_CHECK_ERR_GOTO(!mem, ret = ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory."), cleanup);
                str = mem;
                str_len += sprintf(str + str_len, "%s:%.*s", prefix, (int)len, str_begin);
            } else {
                /* just append the string, we may get the first expression node without a prefix but since this
                 * is not strictly forbidden, allow it */
                mem = realloc(str, str_len + len + 1);
                LY_CHECK_ERR_GOTO(!mem, ret = ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory."), cleanup);
                str = mem;
                str_len += sprintf(str + str_len, "%.*s", (int)len, str_begin);
            }
        } else {
            /* remember there was a prefix found */
            has_prefix = 1;

            /* resolve the module in the original format */
            mod = lyplg_type_identity_module(resolve_ctx, NULL, str_begin, len, resolve_format, resolve_prefix_data);
            if (!mod && is_nametest) {
                ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Failed to resolve prefix \"%.*s\".",
                        (int)len, str_begin);
                goto cleanup;
            }

            if (is_nametest && ((get_format == LY_VALUE_JSON) || (get_format == LY_VALUE_LYB)) && (*context_mod == mod)) {
                /* inherit the prefix and do not print it again */
            } else {
                if (mod) {
                    /* get the prefix in the target format */
                    prefix = lyplg_type_get_prefix(mod, get_format, get_prefix_data);
                    assert(prefix);
                    pref_len = strlen(prefix);
                } else {
                    /* invalid prefix, just copy it */
                    prefix = str_begin;
                    pref_len = len;
                }

                /* append the prefix */
                mem = realloc(str, str_len + pref_len + 2);
                LY_CHECK_ERR_GOTO(!mem, ret = ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory."), cleanup);
                str = mem;
                str_len += sprintf(str + str_len, "%.*s:", (int)pref_len, prefix);
            }

            if (is_nametest) {
                /* update context module */
                *context_mod = mod;
            }
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
 * @brief Print xpath1.0 subexpression in the specific format.
 *
 * @param[in,out] cur_idx Current index of the next token in the expression.
 * @param[in] end_tok End token (including) that finishes this subexpression parsing. If 0, parse until the end.
 * @param[in] context_mod Current context module, some formats (::LY_VALUE_JSON and ::LY_VALUE_LYB) inherit this module
 * instead of printing it again.
 * @param[in] xp_val xpath1.0 value structure.
 * @param[in] format Format to print in.
 * @param[in] prefix_data Format-specific prefix data.
 * @param[in,out] str_value Printed value, appended to.
 * @param[in,out] str_len Length of @p str_value, updated.
 * @param[out] err Error structure on error.
 * @return LY_ERR value.
 */
static LY_ERR
xpath10_print_subexpr_r(uint16_t *cur_idx, enum lyxp_token end_tok, const struct lys_module *context_mod,
        const struct lyd_value_xpath10 *xp_val, LY_VALUE_FORMAT format, void *prefix_data, char **str_value,
        uint32_t *str_len, struct ly_err_item **err)
{
    enum lyxp_token cur_tok, sub_end_tok;
    char *str_tok;
    void *mem;
    const char *cur_exp_ptr;
    ly_bool is_nt;
    const struct lys_module *orig_context_mod = context_mod;

    while (*cur_idx < xp_val->exp->used) {
        cur_tok = xp_val->exp->tokens[*cur_idx];
        cur_exp_ptr = xp_val->exp->expr + xp_val->exp->tok_pos[*cur_idx];

        if ((cur_tok == LYXP_TOKEN_NAMETEST) || (cur_tok == LYXP_TOKEN_LITERAL)) {
            /* tokens that may include prefixes, get them in the target format */
            is_nt = (cur_tok == LYXP_TOKEN_NAMETEST) ? 1 : 0;
            LY_CHECK_RET(lyplg_type_xpath10_print_token(cur_exp_ptr, xp_val->exp->tok_len[*cur_idx], is_nt, &context_mod,
                    xp_val->ctx, xp_val->format, xp_val->prefix_data, format, prefix_data, &str_tok, err));

            /* append the converted token */
            mem = realloc(*str_value, *str_len + strlen(str_tok) + 1);
            LY_CHECK_ERR_GOTO(!mem, free(str_tok), error_mem);
            *str_value = mem;
            *str_len += sprintf(*str_value + *str_len, "%s", str_tok);
            free(str_tok);

            /* token processed */
            ++(*cur_idx);
        } else {
            if ((cur_tok == LYXP_TOKEN_OPER_LOG) || (cur_tok == LYXP_TOKEN_OPER_UNI) || (cur_tok == LYXP_TOKEN_OPER_MATH)) {
                /* copy the token with spaces around */
                mem = realloc(*str_value, *str_len + 1 + xp_val->exp->tok_len[*cur_idx] + 2);
                LY_CHECK_GOTO(!mem, error_mem);
                *str_value = mem;
                *str_len += sprintf(*str_value + *str_len, " %.*s ", (int)xp_val->exp->tok_len[*cur_idx], cur_exp_ptr);

                /* reset context mod */
                context_mod = orig_context_mod;
            } else {
                /* just copy the token */
                mem = realloc(*str_value, *str_len + xp_val->exp->tok_len[*cur_idx] + 1);
                LY_CHECK_GOTO(!mem, error_mem);
                *str_value = mem;
                *str_len += sprintf(*str_value + *str_len, "%.*s", (int)xp_val->exp->tok_len[*cur_idx], cur_exp_ptr);
            }

            /* token processed but keep it in cur_tok */
            ++(*cur_idx);

            if (end_tok && (cur_tok == end_tok)) {
                /* end token found */
                break;
            } else if ((cur_tok == LYXP_TOKEN_BRACK1) || (cur_tok == LYXP_TOKEN_PAR1)) {
                sub_end_tok = (cur_tok == LYXP_TOKEN_BRACK1) ? LYXP_TOKEN_BRACK2 : LYXP_TOKEN_PAR2;

                /* parse the subexpression separately, use the current context mod */
                LY_CHECK_RET(xpath10_print_subexpr_r(cur_idx, sub_end_tok, context_mod, xp_val, format, prefix_data,
                        str_value, str_len, err));
            }
        }
    }

    return LY_SUCCESS;

error_mem:
    return ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, "No memory.");
}

LIBYANG_API_DEF LY_ERR
lyplg_type_print_xpath10_value(const struct lyd_value_xpath10 *xp_val, LY_VALUE_FORMAT format, void *prefix_data,
        char **str_value, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    uint16_t expr_idx = 0;
    uint32_t str_len = 0;
    const struct lys_module *local_mod = NULL;
    struct ly_set *mods;

    *str_value = NULL;
    *err = NULL;

    if (format == LY_VALUE_XML) {
        /* null the local module so that all the prefixes are printed */
        mods = prefix_data;
        local_mod = mods->objs[0];
        mods->objs[0] = NULL;
    }

    /* recursively print the expression */
    ret = xpath10_print_subexpr_r(&expr_idx, 0, NULL, xp_val, format, prefix_data, str_value, &str_len, err);

    if (local_mod) {
        mods->objs[0] = (void *)local_mod;
    }
    if (ret) {
        free(*str_value);
        *str_value = NULL;
    }
    return ret;
}

LIBYANG_API_DEF LY_ERR
lyplg_type_store_xpath10(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *prefix_data, uint32_t hints, const struct lysc_node *ctx_node,
        struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres), struct ly_err_item **err)
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

    /* parse */
    ret = lyxp_expr_parse(ctx, value_len ? value : "", value_len, 1, &val->exp);
    LY_CHECK_GOTO(ret, cleanup);
    val->ctx = ctx;

    if (ctx_node && !strcmp(ctx_node->name, "parent-reference") && !strcmp(ctx_node->module->name, "ietf-yang-schema-mount")) {
        /* special case, this type uses prefix-namespace mapping provided directly in data, keep empty for now */
        val->format = format = LY_VALUE_STR_NS;
        ret = ly_set_new((struct ly_set **)&val->prefix_data);
        LY_CHECK_GOTO(ret, cleanup);
    } else {
        /* store format-specific data and context for later prefix resolution */
        ret = lyplg_type_prefix_data_new(ctx, value, value_len, format, prefix_data, &val->format, &val->prefix_data);
        LY_CHECK_GOTO(ret, cleanup);
    }

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
        ret = lyplg_type_print_xpath10_value(val, LY_VALUE_JSON, NULL, &canon, err);
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
    } else if (val->format == LY_VALUE_STR_NS) {
        /* needs validation */
        return LY_EINCOMPLETE;
    }
    return ret;
}

/**
 * @brief Create a namespace and add it into a set.
 *
 * @param[in] set Set of namespaces to add to.
 * @param[in] pref Namespace prefix.
 * @param[in] uri Namespace URI.
 * @return LY_ERR value.
 */
static LY_ERR
xpath10_add_ns(struct ly_set *set, const char *pref, const char *uri)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyxml_ns *ns = NULL;

    /* create new ns */
    ns = calloc(1, sizeof *ns);
    if (!ns) {
        rc = LY_EMEM;
        goto cleanup;
    }
    ns->prefix = strdup(pref);
    ns->uri = strdup(uri);
    if (!ns->prefix || !ns->uri) {
        rc = LY_EMEM;
        goto cleanup;
    }
    ns->depth = 1;

    /* add into the XML namespace set */
    if ((rc = ly_set_add(set, ns, 1, NULL))) {
        goto cleanup;
    }
    ns = NULL;

cleanup:
    if (ns) {
        free(ns->prefix);
        free(ns->uri);
        free(ns);
    }
    return rc;
}

/**
 * @brief Implementation of ::lyplg_type_validate_clb for the xpath1.0 ietf-yang-types type.
 */
static LY_ERR
lyplg_type_validate_xpath10(const struct ly_ctx *UNUSED(ctx), const struct lysc_type *UNUSED(type),
        const struct lyd_node *ctx_node, const struct lyd_node *UNUSED(tree), struct lyd_value *storage,
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_value_xpath10 *val;
    struct ly_set *set = NULL;
    uint32_t i;
    const char *pref, *uri;
    const struct ly_err_item *eitem;

    *err = NULL;
    LYD_VALUE_GET(storage, val);

    if (val->format != LY_VALUE_STR_NS) {
        /* nothing to validate */
        return LY_SUCCESS;
    }

    /* the XML namespace set must exist */
    assert(val->prefix_data);

    /* special handling of this particular node */
    assert(!strcmp(LYD_NAME(ctx_node), "parent-reference") &&
            !strcmp(ctx_node->schema->module->name, "ietf-yang-schema-mount"));

    /* get all the prefix mappings */
    if ((ret = lyd_find_xpath(ctx_node, "../../../namespace", &set))) {
        goto cleanup;
    }

    for (i = 0; i < set->count; ++i) {
        assert(!strcmp(LYD_NAME(lyd_child(set->dnodes[i])), "prefix"));
        pref = lyd_get_value(lyd_child(set->dnodes[i]));

        if (!lyd_child(set->dnodes[i])->next) {
            /* missing URI - invalid mapping, skip */
            continue;
        }
        assert(!strcmp(LYD_NAME(lyd_child(set->dnodes[i])->next), "uri"));
        uri = lyd_get_value(lyd_child(set->dnodes[i])->next);

        /* new NS */
        if ((ret = xpath10_add_ns(val->prefix_data, pref, uri))) {
            goto cleanup;
        }
    }

cleanup:
    ly_set_free(set, NULL);
    if (ret == LY_EMEM) {
        ly_err_new(err, LY_EMEM, LYVE_DATA, NULL, NULL, LY_EMEM_MSG);
    } else if (ret) {
        eitem = ly_err_last(LYD_CTX(ctx_node));
        ly_err_new(err, ret, LYVE_DATA, eitem->data_path, NULL, "%s", eitem->msg);
    }
    return ret;
}

LIBYANG_API_DEF const void *
lyplg_type_print_xpath10(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *prefix_data, ly_bool *dynamic, size_t *value_len)
{
    struct lyd_value_xpath10 *val;
    char *ret;
    struct ly_err_item *err = NULL;

    LYD_VALUE_GET(value, val);

    /* LY_VALUE_STR_NS should never be transformed */
    if ((val->format == LY_VALUE_STR_NS) || (format == LY_VALUE_CANON) || (format == LY_VALUE_JSON) ||
            (format == LY_VALUE_LYB)) {
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
    if (lyplg_type_print_xpath10_value(val, format, prefix_data, &ret, &err)) {
        if (err) {
            ly_err_print(ctx, err);
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

LIBYANG_API_DEF LY_ERR
lyplg_type_dup_xpath10(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_value_xpath10 *orig_val, *dup_val;

    /* init dup value */
    memset(dup, 0, sizeof *dup);
    dup->realtype = original->realtype;

    ret = lydict_insert(ctx, original->_canonical, 0, &dup->_canonical);
    LY_CHECK_GOTO(ret, cleanup);

    LYPLG_TYPE_VAL_INLINE_PREPARE(dup, dup_val);
    LY_CHECK_ERR_GOTO(!dup_val, LOGMEM(ctx); ret = LY_EMEM, cleanup);
    dup_val->ctx = ctx;

    LYD_VALUE_GET(original, orig_val);
    ret = lyxp_expr_dup(ctx, orig_val->exp, 0, 0, &dup_val->exp);
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

LIBYANG_API_DEF void
lyplg_type_free_xpath10(const struct ly_ctx *ctx, struct lyd_value *value)
{
    struct lyd_value_xpath10 *val;

    lydict_remove(ctx, value->_canonical);
    value->_canonical = NULL;
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
        .plugin.validate = lyplg_type_validate_xpath10,
        .plugin.compare = lyplg_type_compare_simple,
        .plugin.sort = lyplg_type_sort_simple,
        .plugin.print = lyplg_type_print_xpath10,
        .plugin.duplicate = lyplg_type_dup_xpath10,
        .plugin.free = lyplg_type_free_xpath10,
        .plugin.lyb_data_len = -1,
    },
    {0}
};
