/**
 * @file plugins_types.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Built-in types plugins and interface for user types plugins.
 *
 * Copyright (c) 2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* asprintf, strdup */
#include <sys/cdefs.h>

#include "plugins_types.h"

#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "compat.h"
#include "context.h"
#include "dict.h"
#include "path.h"
#include "schema_compile.h"
#include "set.h"
#include "tree.h"
#include "tree_data.h"
#include "tree_data_internal.h"
#include "tree_schema.h"
#include "tree_schema_internal.h"
#include "xml.h"
#include "xpath.h"

/**
 * @brief Find import prefix in imports.
 */
static const struct lys_module *
ly_schema_resolve_prefix(const struct ly_ctx *UNUSED(ctx), const char *prefix, size_t prefix_len, const void *prefix_data)
{
    const struct lysp_module *prefix_mod = prefix_data;
    struct lys_module *m = NULL;
    LY_ARRAY_COUNT_TYPE u;
    const char *local_prefix;

    local_prefix = prefix_mod->is_submod ? ((struct lysp_submodule *)prefix_mod)->prefix : prefix_mod->mod->prefix;
    if (!prefix_len || !ly_strncmp(local_prefix, prefix, prefix_len)) {
        /* it is the prefix of the module itself */
        m = prefix_mod->mod;
    }

    /* search in imports */
    if (!m) {
        LY_ARRAY_FOR(prefix_mod->imports, u) {
            if (!ly_strncmp(prefix_mod->imports[u].prefix, prefix, prefix_len)) {
                m = prefix_mod->imports[u].module;
                break;
            }
        }
    }

    return m;
}

/**
 * @brief Find resolved module for a prefix in prefix - module pairs.
 */
static const struct lys_module *
ly_schema_resolved_resolve_prefix(const struct ly_ctx *UNUSED(ctx), const char *prefix, size_t prefix_len,
        const void *prefix_data)
{
    const struct lysc_prefix *prefixes = prefix_data;
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(prefixes, u) {
        if (!ly_strncmp(prefixes[u].prefix, prefix, prefix_len)) {
            return prefixes[u].mod;
        }
    }

    return NULL;
}

/**
 * @brief Find XML namespace prefix in XML namespaces, which are then mapped to modules.
 */
static const struct lys_module *
ly_xml_resolve_prefix(const struct ly_ctx *ctx, const char *prefix, size_t prefix_len, const void *prefix_data)
{
    const struct lys_module *mod;
    const struct lyxml_ns *ns;
    const struct ly_set *ns_set = prefix_data;

    ns = lyxml_ns_get(ns_set, prefix, prefix_len);
    if (!ns) {
        return NULL;
    }

    mod = ly_ctx_get_module_implemented_ns(ctx, ns->uri);
    if (!mod) {
        /* for YIN extension prefix resolution */
        mod = ly_ctx_get_module_latest_ns(ctx, ns->uri);
    }
    return mod;
}

/**
 * @brief Find module name.
 */
static const struct lys_module *
ly_json_resolve_prefix(const struct ly_ctx *ctx, const char *prefix, size_t prefix_len, const void *UNUSED(prefix_data))
{
    return ly_ctx_get_module_implemented2(ctx, prefix, prefix_len);
}

const struct lys_module *
ly_resolve_prefix(const struct ly_ctx *ctx, const void *prefix, size_t prefix_len, LY_VALUE_FORMAT format,
        const void *prefix_data)
{
    const struct lys_module *mod = NULL;

    LY_CHECK_ARG_RET(ctx, prefix, prefix_len, NULL);

    switch (format) {
    case LY_VALUE_SCHEMA:
        mod = ly_schema_resolve_prefix(ctx, prefix, prefix_len, prefix_data);
        break;
    case LY_VALUE_SCHEMA_RESOLVED:
        mod = ly_schema_resolved_resolve_prefix(ctx, prefix, prefix_len, prefix_data);
        break;
    case LY_VALUE_XML:
        mod = ly_xml_resolve_prefix(ctx, prefix, prefix_len, prefix_data);
        break;
    case LY_VALUE_CANON:
    case LY_VALUE_JSON:
    case LY_VALUE_LYB:
        mod = ly_json_resolve_prefix(ctx, prefix, prefix_len, prefix_data);
        break;
    }

    return mod;
}

API const struct lys_module *
lyplg_type_identity_module(const struct ly_ctx *ctx, const struct lysc_node *ctx_node,
        const char *prefix, size_t prefix_len, LY_VALUE_FORMAT format, const void *prefix_data)
{
    if (prefix_len) {
        return ly_resolve_prefix(ctx, prefix, prefix_len, format, prefix_data);
    } else {
        switch (format) {
        case LY_VALUE_SCHEMA:
        case LY_VALUE_SCHEMA_RESOLVED:
            /* use context node module, handles augments */
            return ctx_node->module;
        case LY_VALUE_CANON:
        case LY_VALUE_JSON:
        case LY_VALUE_LYB:
            /* use context node module (as specified) */
            return ctx_node->module;
        case LY_VALUE_XML:
            /* use the default namespace */
            return ly_xml_resolve_prefix(ctx, NULL, 0, prefix_data);
        }
    }

    return NULL;
}

/**
 * @brief Find module in import prefixes.
 */
static const char *
ly_schema_get_prefix(const struct lys_module *mod, void *prefix_data)
{
    const struct lysp_module *pmod = prefix_data;
    LY_ARRAY_COUNT_TYPE u;

    if (pmod->mod == mod) {
        if (pmod->is_submod) {
            return ((struct lysp_submodule *)pmod)->prefix;
        } else {
            return pmod->mod->prefix;
        }
    }

    LY_ARRAY_FOR(pmod->imports, u) {
        if (pmod->imports[u].module == mod) {
            /* match */
            return pmod->imports[u].prefix;
        }
    }

    return NULL;
}

/**
 * @brief Find prefix in prefix - module pairs.
 */
static const char *
ly_schema_resolved_get_prefix(const struct lys_module *mod, void *prefix_data)
{
    struct lysc_prefix *prefixes = prefix_data;
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(prefixes, u) {
        if (prefixes[u].mod == mod) {
            return prefixes[u].prefix;
        }
    }

    return NULL;
}

/**
 * @brief Simply return module local prefix. Also, store the module in a set.
 */
static const char *
ly_xml_get_prefix(const struct lys_module *mod, void *prefix_data)
{
    struct ly_set *ns_list = prefix_data;

    LY_CHECK_RET(ly_set_add(ns_list, (void *)mod, 0, NULL), NULL);
    return mod->prefix;
}

/**
 * @brief Simply return module name.
 */
static const char *
ly_json_get_prefix(const struct lys_module *mod, void *UNUSED(prefix_data))
{
    return mod->name;
}

const char *
ly_get_prefix(const struct lys_module *mod, LY_VALUE_FORMAT format, void *prefix_data)
{
    const char *prefix = NULL;

    switch (format) {
    case LY_VALUE_SCHEMA:
        prefix = ly_schema_get_prefix(mod, prefix_data);
        break;
    case LY_VALUE_SCHEMA_RESOLVED:
        prefix = ly_schema_resolved_get_prefix(mod, prefix_data);
        break;
    case LY_VALUE_XML:
        prefix = ly_xml_get_prefix(mod, prefix_data);
        break;
    case LY_VALUE_CANON:
    case LY_VALUE_JSON:
    case LY_VALUE_LYB:
        prefix = ly_json_get_prefix(mod, prefix_data);
        break;
    }

    return prefix;
}

API const char *
lyplg_type_get_prefix(const struct lys_module *mod, LY_VALUE_FORMAT format, void *prefix_data)
{
    return ly_get_prefix(mod, format, prefix_data);
}

API LY_ERR
lyplg_type_compare_simple(const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }

    if (val1->_canonical == val2->_canonical) {
        return LY_SUCCESS;
    }

    return LY_ENOT;
}

API const void *
lyplg_type_print_simple(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *value, LY_VALUE_FORMAT UNUSED(format),
        void *UNUSED(prefix_data), ly_bool *dynamic, size_t *value_len)
{
    if (dynamic) {
        *dynamic = 0;
    }
    if (value_len) {
        *value_len = strlen(value->_canonical);
    }
    return value->_canonical;
}

API LY_ERR
lyplg_type_dup_simple(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_CHECK_RET(lydict_insert(ctx, original->_canonical, strlen(original->_canonical), &dup->_canonical));
    memcpy(dup->fixed_mem, original->fixed_mem, sizeof dup->fixed_mem);
    dup->realtype = original->realtype;
    return LY_SUCCESS;
}

API void
lyplg_type_free_simple(const struct ly_ctx *ctx, struct lyd_value *value)
{
    lydict_remove(ctx, value->_canonical);
    value->_canonical = NULL;
}

API LY_ERR
lyplg_type_parse_int(const char *datatype, int base, int64_t min, int64_t max, const char *value, size_t value_len,
        int64_t *ret, struct ly_err_item **err)
{
    LY_CHECK_ARG_RET(NULL, err, datatype, LY_EINVAL);

    *err = NULL;

    /* consume leading whitespaces */
    for ( ; value_len && isspace(*value); ++value, --value_len) {}

    if (!value || !value_len || !value[0]) {
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid empty %s value.", datatype);
    }

    switch (ly_parse_int(value, value_len, min, max, base, ret)) {
    case LY_EDENIED:
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Value is out of %s's min/max bounds.", datatype);
    case LY_SUCCESS:
        return LY_SUCCESS;
    default:
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid %s value \"%.*s\".", datatype, (int)value_len, value);
    }
}

API LY_ERR
lyplg_type_parse_uint(const char *datatype, int base, uint64_t max, const char *value, size_t value_len, uint64_t *ret,
        struct ly_err_item **err)
{
    LY_CHECK_ARG_RET(NULL, err, datatype, LY_EINVAL);

    *err = NULL;

    /* consume leading whitespaces */
    for ( ; value_len && isspace(*value); ++value, --value_len) {}

    if (!value || !value_len || !value[0]) {
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid empty %s value.", datatype);
    }

    *err = NULL;
    switch (ly_parse_uint(value, value_len, max, base, ret)) {
    case LY_EDENIED:
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                "Value \"%.*s\" is out of %s's min/max bounds.", (int)value_len, value, datatype);
    case LY_SUCCESS:
        return LY_SUCCESS;
    default:
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                "Invalid %s value \"%.*s\".", datatype, (int)value_len, value);
    }
}

API LY_ERR
lyplg_type_parse_dec64(uint8_t fraction_digits, const char *value, size_t value_len, int64_t *ret, struct ly_err_item **err)
{
    LY_ERR ret_val;
    char *valcopy = NULL;
    size_t fraction = 0, size, len = 0, trailing_zeros;
    int64_t d;

    *err = NULL;

    /* consume leading whitespaces */
    for ( ; value_len && isspace(*value); ++value, --value_len) {}

    /* parse value */
    if (!value_len) {
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid empty decimal64 value.");
    } else if (!isdigit(value[len]) && (value[len] != '-') && (value[len] != '+')) {
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid %lu. character of decimal64 value \"%.*s\".",
                len + 1, (int)value_len, value);
    }

    if ((value[len] == '-') || (value[len] == '+')) {
        ++len;
    }

    while (len < value_len && isdigit(value[len])) {
        ++len;
    }

    trailing_zeros = 0;
    if ((len < value_len) && ((value[len] != '.') || !isdigit(value[len + 1]))) {
        goto decimal;
    }
    fraction = len;
    ++len;
    while (len < value_len && isdigit(value[len])) {
        if (value[len] == '0') {
            ++trailing_zeros;
        } else {
            trailing_zeros = 0;
        }
        ++len;
    }
    len = len - trailing_zeros;

decimal:
    if (fraction && (len - 1 - fraction > fraction_digits)) {
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                "Value \"%.*s\" of decimal64 type exceeds defined number (%u) of fraction digits.",
                (int)len, value, fraction_digits);
    }
    if (fraction) {
        size = len + (fraction_digits - (len - 1 - fraction));
    } else {
        size = len + fraction_digits + 1;
    }

    if (len + trailing_zeros < value_len) {
        /* consume trailing whitespaces to check that there is nothing after it */
        uint64_t u;
        for (u = len + trailing_zeros; u < value_len && isspace(value[u]); ++u) {}
        if (u != value_len) {
            return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                    "Invalid %" PRIu64 ". character of decimal64 value \"%.*s\".", u + 1, (int)value_len, value);
        }
    }

    /* prepare value string without decimal point to easily parse using standard functions */
    valcopy = malloc(size * sizeof *valcopy);
    if (!valcopy) {
        return ly_err_new(err, LY_EMEM, 0, NULL, NULL, LY_EMEM_MSG);
    }

    valcopy[size - 1] = '\0';
    if (fraction) {
        memcpy(&valcopy[0], &value[0], fraction);
        memcpy(&valcopy[fraction], &value[fraction + 1], len - 1 - (fraction));
        /* add trailing zero characters */
        memset(&valcopy[len - 1], '0', fraction_digits - (len - 1 - fraction));
    } else {
        memcpy(&valcopy[0], &value[0], len);
        /* add trailing zero characters */
        memset(&valcopy[len], '0', fraction_digits);
    }

    ret_val = lyplg_type_parse_int("decimal64", LY_BASE_DEC, INT64_C(-9223372036854775807) - INT64_C(1),
            INT64_C(9223372036854775807), valcopy, size - 1, &d, err);
    if (!ret_val && ret) {
        *ret = d;
    }
    free(valcopy);

    return ret_val;
}

API LY_ERR
lyplg_type_validate_patterns(struct lysc_pattern **patterns, const char *str, size_t str_len, struct ly_err_item **err)
{
    int rc, match_opts;
    LY_ARRAY_COUNT_TYPE u;
    pcre2_match_data *match_data = NULL;

    LY_CHECK_ARG_RET(NULL, str, err, LY_EINVAL);

    *err = NULL;

    LY_ARRAY_FOR(patterns, u) {
        /* match_data needs to be allocated each time because of possible multi-threaded evaluation */
        match_data = pcre2_match_data_create_from_pattern(patterns[u]->code, NULL);
        if (!match_data) {
            return ly_err_new(err, LY_EMEM, 0, NULL, NULL, LY_EMEM_MSG);
        }

        match_opts = PCRE2_ANCHORED;
#ifdef PCRE2_ENDANCHORED
        /* PCRE2_ENDANCHORED was added in PCRE2 version 10.30 */
        match_opts |= PCRE2_ENDANCHORED;
#endif
        rc = pcre2_match(patterns[u]->code, (PCRE2_SPTR)str, str_len, 0, match_opts, match_data, NULL);
        pcre2_match_data_free(match_data);

        if ((rc != PCRE2_ERROR_NOMATCH) && (rc < 0)) {
            PCRE2_UCHAR pcre2_errmsg[LY_PCRE2_MSG_LIMIT] = {0};
            pcre2_get_error_message(rc, pcre2_errmsg, LY_PCRE2_MSG_LIMIT);

            return ly_err_new(err, LY_ESYS, 0, NULL, NULL, (const char *)pcre2_errmsg);
        } else if (((rc == PCRE2_ERROR_NOMATCH) && !patterns[u]->inverted) ||
                ((rc != PCRE2_ERROR_NOMATCH) && patterns[u]->inverted)) {
            char *eapptag = patterns[u]->eapptag ? strdup(patterns[u]->eapptag) : NULL;

            if (patterns[u]->emsg) {
                return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, eapptag, patterns[u]->emsg);
            } else {
                const char *inverted = patterns[u]->inverted ? "inverted " : "";
                return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, eapptag,
                        LY_ERRMSG_NOPATTERN, (int)str_len, str, inverted, patterns[u]->expr);
            }
        }
    }
    return LY_SUCCESS;
}

API LY_ERR
lyplg_type_validate_range(LY_DATA_TYPE basetype, struct lysc_range *range, int64_t value, const char *strval,
        size_t strval_len, struct ly_err_item **err)
{
    LY_ARRAY_COUNT_TYPE u;
    ly_bool is_length; /* length or range */

    *err = NULL;
    is_length = (basetype == LY_TYPE_BINARY || basetype == LY_TYPE_STRING) ? 1 : 0;

    LY_ARRAY_FOR(range->parts, u) {
        if (basetype < LY_TYPE_DEC64) {
            /* unsigned */
            if ((uint64_t)value < range->parts[u].min_u64) {
                char *eapptag = range->eapptag ? strdup(range->eapptag) : NULL;
                if (range->emsg) {
                    return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, eapptag, range->emsg);
                } else {
                    return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, eapptag,
                            is_length ? LY_ERRMSG_NOLENGTH : LY_ERRMSG_NORANGE, (int)strval_len, strval);
                }
            } else if ((uint64_t)value <= range->parts[u].max_u64) {
                /* inside the range */
                return LY_SUCCESS;
            } else if (u == LY_ARRAY_COUNT(range->parts) - 1) {
                /* we have the last range part, so the value is out of bounds */
                char *eapptag = range->eapptag ? strdup(range->eapptag) : NULL;
                if (range->emsg) {
                    return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, eapptag, range->emsg);
                } else {
                    return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, eapptag,
                            is_length ? LY_ERRMSG_NOLENGTH : LY_ERRMSG_NORANGE, (int)strval_len, strval);
                }
            }
        } else {
            /* signed */
            if (value < range->parts[u].min_64) {
                char *eapptag = range->eapptag ? strdup(range->eapptag) : NULL;
                if (range->emsg) {
                    return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, eapptag, range->emsg);
                } else {
                    return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, eapptag, LY_ERRMSG_NORANGE, (int)strval_len, strval);
                }
            } else if (value <= range->parts[u].max_64) {
                /* inside the range */
                return LY_SUCCESS;
            } else if (u == LY_ARRAY_COUNT(range->parts) - 1) {
                /* we have the last range part, so the value is out of bounds */
                char *eapptag = range->eapptag ? strdup(range->eapptag) : NULL;
                if (range->emsg) {
                    return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, eapptag, range->emsg);
                } else {
                    return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, eapptag, LY_ERRMSG_NORANGE, (int)strval_len, strval);
                }
            }
        }
    }

    return LY_SUCCESS;
}

API LY_ERR
lyplg_type_prefix_data_new(const struct ly_ctx *ctx, const void *value, size_t value_len, LY_VALUE_FORMAT format,
        const void *prefix_data, LY_VALUE_FORMAT *format_p, void **prefix_data_p)
{
    LY_CHECK_ARG_RET(ctx, value, format_p, prefix_data_p, LY_EINVAL);

    *prefix_data_p = NULL;
    return ly_store_prefix_data(ctx, value, value_len, format, prefix_data, format_p, (void **)prefix_data_p);
}

API LY_ERR
lyplg_type_prefix_data_dup(const struct ly_ctx *ctx, LY_VALUE_FORMAT format, const void *orig, void **dup)
{
    LY_CHECK_ARG_RET(NULL, dup, LY_EINVAL);

    *dup = NULL;
    if (!orig) {
        return LY_SUCCESS;
    }

    return ly_dup_prefix_data(ctx, format, orig, (void **)dup);
}

API void
lyplg_type_prefix_data_free(LY_VALUE_FORMAT format, void *prefix_data)
{
    ly_free_prefix_data(format, prefix_data);
}

static int
type_get_hints_base(uint32_t hints)
{
    /* set allowed base */
    switch (hints & (LYD_VALHINT_DECNUM | LYD_VALHINT_OCTNUM | LYD_VALHINT_HEXNUM)) {
    case LYD_VALHINT_DECNUM:
        return LY_BASE_DEC;
    case LYD_VALHINT_OCTNUM:
        return LY_BASE_OCT;
    case LYD_VALHINT_HEXNUM:
        return LY_BASE_HEX;
    default:
        /* generic base - decimal by default, hexa if prexed by 0x/0X and octal otherwise if prefixed by 0 */
        return 0;
    }
}

API LY_ERR
lyplg_type_check_hints(uint32_t hints, const char *value, size_t value_len, LY_DATA_TYPE type, int *base, struct ly_err_item **err)
{
    LY_CHECK_ARG_RET(NULL, value || !value_len, err, LY_EINVAL);

    *err = NULL;
    if (!value) {
        value = "";
    }

    switch (type) {
    case LY_TYPE_UINT8:
    case LY_TYPE_UINT16:
    case LY_TYPE_UINT32:
    case LY_TYPE_INT8:
    case LY_TYPE_INT16:
    case LY_TYPE_INT32:
        LY_CHECK_ARG_RET(NULL, base, LY_EINVAL);

        if (!(hints & (LYD_VALHINT_DECNUM | LYD_VALHINT_OCTNUM | LYD_VALHINT_HEXNUM))) {
            return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid non-number-encoded %s value \"%.*s\".",
                    lys_datatype2str(type), (int)value_len, value);
        }
        *base = type_get_hints_base(hints);
        break;
    case LY_TYPE_UINT64:
    case LY_TYPE_INT64:
        LY_CHECK_ARG_RET(NULL, base, LY_EINVAL);

        if (!(hints & LYD_VALHINT_NUM64)) {
            return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid non-num64-encoded %s value \"%.*s\".",
                    lys_datatype2str(type), (int)value_len, value);
        }
        *base = type_get_hints_base(hints);
        break;
    case LY_TYPE_STRING:
    case LY_TYPE_DEC64:
    case LY_TYPE_ENUM:
    case LY_TYPE_BITS:
    case LY_TYPE_BINARY:
    case LY_TYPE_IDENT:
    case LY_TYPE_INST:
        if (!(hints & LYD_VALHINT_STRING)) {
            return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid non-string-encoded %s value \"%.*s\".",
                    lys_datatype2str(type), (int)value_len, value);
        }
        break;
    case LY_TYPE_BOOL:
        if (!(hints & LYD_VALHINT_BOOLEAN)) {
            return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid non-boolean-encoded %s value \"%.*s\".",
                    lys_datatype2str(type), (int)value_len, value);
        }
        break;
    case LY_TYPE_EMPTY:
        if (!(hints & LYD_VALHINT_EMPTY)) {
            return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid non-empty-encoded %s value \"%.*s\".",
                    lys_datatype2str(type), (int)value_len, value);
        }
        break;
    case LY_TYPE_UNKNOWN:
    case LY_TYPE_LEAFREF:
    case LY_TYPE_UNION:
        LOGINT_RET(NULL);
    }

    return LY_SUCCESS;
}

API LY_ERR
lyplg_type_lypath_new(const struct ly_ctx *ctx, const char *value, size_t value_len, uint32_t options,
        LY_VALUE_FORMAT format, void *prefix_data, const struct lysc_node *ctx_node, struct lys_glob_unres *unres,
        struct ly_path **path, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_expr *exp = NULL;
    uint32_t prefix_opt = 0;

    LY_CHECK_ARG_RET(ctx, ctx, value, ctx_node, path, err, LY_EINVAL);

    *path = NULL;
    *err = NULL;

    switch (format) {
    case LY_VALUE_SCHEMA:
    case LY_VALUE_SCHEMA_RESOLVED:
    case LY_VALUE_XML:
        prefix_opt = LY_PATH_PREFIX_MANDATORY;
        break;
    case LY_VALUE_CANON:
    case LY_VALUE_LYB:
    case LY_VALUE_JSON:
        prefix_opt = LY_PATH_PREFIX_STRICT_INHERIT;
        break;
    }

    /* parse the value */
    ret = ly_path_parse(ctx, ctx_node, value, value_len, LY_PATH_BEGIN_ABSOLUTE, LY_PATH_LREF_FALSE,
            prefix_opt, LY_PATH_PRED_SIMPLE, &exp);
    if (ret) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                "Invalid instance-identifier \"%.*s\" value - syntax error.", (int)value_len, value);
        goto cleanup;
    }

    if (options & LYPLG_TYPE_STORE_IMPLEMENT) {
        /* implement all prefixes */
        LY_CHECK_GOTO(ret = lys_compile_expr_implement(ctx, exp, format, prefix_data, 1, unres, NULL), cleanup);
    }

    /* resolve it on schema tree */
    ret = ly_path_compile(ctx, NULL, ctx_node, NULL, exp, LY_PATH_LREF_FALSE, (ctx_node->flags & LYS_IS_OUTPUT) ?
            LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT, LY_PATH_TARGET_SINGLE, format, prefix_data, unres, path);
    if (ret) {
        ret = ly_err_new(err, ret, LYVE_DATA, NULL, NULL,
                "Invalid instance-identifier \"%.*s\" value - semantic error.", (int)value_len, value);
        goto cleanup;
    }

cleanup:
    lyxp_expr_free(ctx, exp);
    if (ret) {
        ly_path_free(ctx, *path);
        *path = NULL;
    }

    return ret;
}

API void
lyplg_type_lypath_free(const struct ly_ctx *ctx, struct ly_path *path)
{
    ly_path_free(ctx, path);
}

API LY_ERR
lyplg_type_make_implemented(struct lys_module *mod, const char **features, struct lys_glob_unres *unres)
{
    LY_CHECK_RET(lys_set_implemented_r(mod, features, unres));

    if (unres->recompile) {
        return LY_ERECOMPILE;
    }

    return LY_SUCCESS;
}

API LY_ERR
lyplg_type_identity_isderived(const struct lysc_ident *base, const struct lysc_ident *der)
{
    LY_ARRAY_COUNT_TYPE u;

    LY_ARRAY_FOR(base->derived, u) {
        if (der == base->derived[u]) {
            return LY_SUCCESS;
        }
        if (!lyplg_type_identity_isderived(base->derived[u], der)) {
            return LY_SUCCESS;
        }
    }
    return LY_ENOTFOUND;
}

API LY_ERR
lyplg_type_resolve_leafref(const struct lysc_type_leafref *lref, const struct lyd_node *node, struct lyd_value *value,
        const struct lyd_node *tree, struct lyd_node **target, char **errmsg)
{
    LY_ERR ret;
    struct lyxp_set set = {0};
    const char *val_str;
    uint32_t i;
    int rc;

    /* find all target data instances */
    ret = lyxp_eval(lref->cur_mod->ctx, lref->path, lref->cur_mod, LY_VALUE_SCHEMA_RESOLVED, lref->prefixes, node, tree,
            &set, 0);
    if (ret) {
        ret = LY_ENOTFOUND;
        val_str = lref->plugin->print(lref->cur_mod->ctx, value, LY_VALUE_CANON, NULL, NULL, NULL);
        if (asprintf(errmsg, "Invalid leafref value \"%s\" - XPath evaluation error.", val_str) == -1) {
            *errmsg = NULL;
            ret = LY_EMEM;
        }
        goto error;
    }

    /* check whether any matches */
    for (i = 0; i < set.used; ++i) {
        if (set.val.nodes[i].type != LYXP_NODE_ELEM) {
            continue;
        }

        if (!lref->plugin->compare(&((struct lyd_node_term *)set.val.nodes[i].node)->value, value)) {
            break;
        }
    }
    if (i == set.used) {
        ret = LY_ENOTFOUND;
        val_str = lref->plugin->print(lref->cur_mod->ctx, value, LY_VALUE_CANON, NULL, NULL, NULL);
        if (set.used) {
            rc = asprintf(errmsg, LY_ERRMSG_NOLREF_VAL, val_str, lref->path->expr);
        } else {
            rc = asprintf(errmsg, LY_ERRMSG_NOLREF_INST, val_str, lref->path->expr);
        }
        if (rc == -1) {
            *errmsg = NULL;
            ret = LY_EMEM;
        }
        goto error;
    }

    if (target) {
        *target = set.val.nodes[i].node;
    }

    lyxp_set_free_content(&set);
    return LY_SUCCESS;

error:
    lyxp_set_free_content(&set);
    return ret;
}
