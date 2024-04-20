/**
 * @file plugins_types.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Built-in types plugins and interface for user types plugins.
 *
 * Copyright (c) 2019 - 2022 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* asprintf, strdup */

#include "plugins_types.h"

#include <assert.h>
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
        if ((!prefixes[u].prefix && !prefix_len) || (prefixes[u].prefix && !ly_strncmp(prefixes[u].prefix, prefix, prefix_len))) {
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
    case LY_VALUE_STR_NS:
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

LIBYANG_API_DEF const struct lys_module *
lyplg_type_identity_module(const struct ly_ctx *ctx, const struct lysc_node *ctx_node, const char *prefix,
        size_t prefix_len, LY_VALUE_FORMAT format, const void *prefix_data)
{
    if (prefix_len) {
        return ly_resolve_prefix(ctx, prefix, prefix_len, format, prefix_data);
    } else {
        switch (format) {
        case LY_VALUE_SCHEMA:
            /* use local module */
            return ly_schema_resolve_prefix(ctx, prefix, prefix_len, prefix_data);
        case LY_VALUE_SCHEMA_RESOLVED:
            /* use local module */
            return ly_schema_resolved_resolve_prefix(ctx, prefix, prefix_len, prefix_data);
        case LY_VALUE_CANON:
        case LY_VALUE_JSON:
        case LY_VALUE_LYB:
        case LY_VALUE_STR_NS:
            /* use context node module (as specified) */
            return ctx_node ? ctx_node->module : NULL;
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
 * @brief Get prefix for XML print.
 *
 * @param[in] mod Module whose prefix to get.
 * @param[in,out] prefix_data Set of used modules in the print. If @p mod is found in this set, no string (prefix) is
 * returned.
 * @return Prefix to print, may be NULL if the default namespace should be used.
 */
static const char *
ly_xml_get_prefix(const struct lys_module *mod, void *prefix_data)
{
    struct ly_set *mods = prefix_data;
    uint32_t i;

    /* first is the local module */
    assert(mods->count);
    if (mods->objs[0] == mod) {
        return NULL;
    }

    /* check for duplicates in the rest of the modules and add there */
    for (i = 1; i < mods->count; ++i) {
        if (mods->objs[i] == mod) {
            break;
        }
    }
    if (i == mods->count) {
        LY_CHECK_RET(ly_set_add(mods, (void *)mod, 1, NULL), NULL);
    }

    /* return the prefix */
    return mod->prefix;
}

/**
 * @brief Get prefix for JSON print.
 *
 * @param[in] mod Module whose prefix to get.
 * @param[in] prefix_data Current local module, may be NULL. If it matches @p mod, no string (preifx) is returned.
 * @return Prefix (module name) to print, may be NULL if the default module should be used.
 */
static const char *
ly_json_get_prefix(const struct lys_module *mod, void *prefix_data)
{
    const struct lys_module *local_mod = prefix_data;

    return (local_mod == mod) ? NULL : mod->name;
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
    case LY_VALUE_STR_NS:
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

LIBYANG_API_DEF const char *
lyplg_type_get_prefix(const struct lys_module *mod, LY_VALUE_FORMAT format, void *prefix_data)
{
    return ly_get_prefix(mod, format, prefix_data);
}

LIBYANG_API_DEF LY_ERR
lyplg_type_compare_simple(const struct lyd_value *val1, const struct lyd_value *val2)
{
    if (val1->_canonical == val2->_canonical) {
        return LY_SUCCESS;
    }

    return LY_ENOT;
}

LIBYANG_API_DEF const void *
lyplg_type_print_simple(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *value, LY_VALUE_FORMAT UNUSED(format),
        void *UNUSED(prefix_data), ly_bool *dynamic, size_t *value_len)
{
    if (dynamic) {
        *dynamic = 0;
    }
    if (value_len) {
        *value_len = ly_strlen(value->_canonical);
    }
    return value->_canonical;
}

LIBYANG_API_DEF LY_ERR
lyplg_type_dup_simple(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    memset(dup, 0, sizeof *dup);
    LY_CHECK_RET(lydict_insert(ctx, original->_canonical, 0, &dup->_canonical));
    memcpy(dup->fixed_mem, original->fixed_mem, sizeof dup->fixed_mem);
    dup->realtype = original->realtype;
    return LY_SUCCESS;
}

LIBYANG_API_DEF void
lyplg_type_free_simple(const struct ly_ctx *ctx, struct lyd_value *value)
{
    lydict_remove(ctx, value->_canonical);
    value->_canonical = NULL;
}

LIBYANG_API_DEF LY_ERR
lyplg_type_parse_int(const char *datatype, int base, int64_t min, int64_t max, const char *value, size_t value_len,
        int64_t *ret, struct ly_err_item **err)
{
    LY_CHECK_ARG_RET(NULL, err, datatype, LY_EINVAL);

    *err = NULL;

    /* consume leading whitespaces */
    for ( ; value_len && isspace(*value); ++value, --value_len) {}

    if (!value || !value_len || !value[0]) {
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid type %s empty value.", datatype);
    }

    switch (ly_parse_int(value, value_len, min, max, base, ret)) {
    case LY_EDENIED:
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                "Value \"%.*s\" is out of type %s min/max bounds.", (int)value_len, value, datatype);
    case LY_SUCCESS:
        return LY_SUCCESS;
    default:
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                "Invalid type %s value \"%.*s\".", datatype, (int)value_len, value);
    }
}

LIBYANG_API_DEF LY_ERR
lyplg_type_parse_uint(const char *datatype, int base, uint64_t max, const char *value, size_t value_len, uint64_t *ret,
        struct ly_err_item **err)
{
    LY_CHECK_ARG_RET(NULL, err, datatype, LY_EINVAL);

    *err = NULL;

    /* consume leading whitespaces */
    for ( ; value_len && isspace(*value); ++value, --value_len) {}

    if (!value || !value_len || !value[0]) {
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid type %s empty value.", datatype);
    }

    *err = NULL;
    switch (ly_parse_uint(value, value_len, max, base, ret)) {
    case LY_EDENIED:
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                "Value \"%.*s\" is out of type %s min/max bounds.", (int)value_len, value, datatype);
    case LY_SUCCESS:
        return LY_SUCCESS;
    default:
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL,
                "Invalid type %s value \"%.*s\".", datatype, (int)value_len, value);
    }
}

LIBYANG_API_DEF LY_ERR
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
        return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid %zu. character of decimal64 value \"%.*s\".",
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

LIBYANG_API_DEF LY_ERR
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

            return ly_err_new(err, LY_ESYS, 0, NULL, NULL, "%s", (const char *)pcre2_errmsg);
        } else if (((rc == PCRE2_ERROR_NOMATCH) && !patterns[u]->inverted) ||
                ((rc != PCRE2_ERROR_NOMATCH) && patterns[u]->inverted)) {
            char *eapptag = patterns[u]->eapptag ? strdup(patterns[u]->eapptag) : NULL;

            if (patterns[u]->emsg) {
                return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, eapptag, "%s", patterns[u]->emsg);
            } else {
                const char *inverted = patterns[u]->inverted ? "inverted " : "";

                return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, eapptag,
                        LY_ERRMSG_NOPATTERN, (int)str_len, str, inverted, patterns[u]->expr);
            }
        }
    }
    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
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
                    return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, eapptag, "%s", range->emsg);
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
                    return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, eapptag, "%s", range->emsg);
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
                    return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, eapptag, "%s", range->emsg);
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
                    return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, eapptag, "%s", range->emsg);
                } else {
                    return ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, eapptag, LY_ERRMSG_NORANGE, (int)strval_len, strval);
                }
            }
        }
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyplg_type_prefix_data_new(const struct ly_ctx *ctx, const void *value, size_t value_len, LY_VALUE_FORMAT format,
        const void *prefix_data, LY_VALUE_FORMAT *format_p, void **prefix_data_p)
{
    LY_CHECK_ARG_RET(ctx, value, format_p, prefix_data_p, LY_EINVAL);

    *prefix_data_p = NULL;
    return ly_store_prefix_data(ctx, value, value_len, format, prefix_data, format_p, (void **)prefix_data_p);
}

LIBYANG_API_DEF LY_ERR
lyplg_type_prefix_data_dup(const struct ly_ctx *ctx, LY_VALUE_FORMAT format, const void *orig, void **dup)
{
    LY_CHECK_ARG_RET(NULL, dup, LY_EINVAL);

    *dup = NULL;
    if (!orig) {
        return LY_SUCCESS;
    }

    return ly_dup_prefix_data(ctx, format, orig, (void **)dup);
}

LIBYANG_API_DEF void
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

LIBYANG_API_DEF LY_ERR
lyplg_type_check_hints(uint32_t hints, const char *value, size_t value_len, LY_DATA_TYPE type, int *base,
        struct ly_err_item **err)
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

LIBYANG_API_DEF LY_ERR
lyplg_type_check_status(const struct lysc_node *ctx_node, uint16_t val_flags, LY_VALUE_FORMAT format, void *prefix_data,
        const char *val_name, struct ly_err_item **err)
{
    LY_ERR ret;
    const struct lys_module *mod2;
    uint16_t flg1, flg2;

    if (format != LY_VALUE_SCHEMA) {
        /* nothing/unable to check */
        return LY_SUCCESS;
    }

    mod2 = ((struct lysp_module *)prefix_data)->mod;

    if (mod2 == ctx_node->module) {
        /* use flags of the context node since the definition is local */
        flg1 = (ctx_node->flags & LYS_STATUS_MASK) ? (ctx_node->flags & LYS_STATUS_MASK) : LYS_STATUS_CURR;
    } else {
        /* definition is foreign (deviation, refine), always current */
        flg1 = LYS_STATUS_CURR;
    }
    flg2 = (val_flags & LYS_STATUS_MASK) ? (val_flags & LYS_STATUS_MASK) : LYS_STATUS_CURR;

    if ((flg1 < flg2) && (ctx_node->module == mod2)) {
        ret = ly_err_new(err, LY_EVALID, LYVE_REFERENCE, NULL, NULL,
                "A %s definition \"%s\" is not allowed to reference %s value \"%s\".",
                flg1 == LYS_STATUS_CURR ? "current" : "deprecated", ctx_node->name,
                flg2 == LYS_STATUS_OBSLT ? "obsolete" : "deprecated", val_name);
        return ret;
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyplg_type_lypath_check_status(const struct lysc_node *ctx_node, const struct ly_path *path, LY_VALUE_FORMAT format,
        void *prefix_data, struct ly_err_item **err)
{
    LY_ERR ret;
    LY_ARRAY_COUNT_TYPE u;
    const struct lys_module *val_mod;
    const struct lysc_node *node;
    uint16_t flg1, flg2;

    if (format != LY_VALUE_SCHEMA) {
        /* nothing to check */
        return LY_SUCCESS;
    }

    val_mod = ((struct lysp_module *)prefix_data)->mod;
    if (val_mod == ctx_node->module) {
        /* use flags of the context node since the definition is local */
        flg1 = (ctx_node->flags & LYS_STATUS_MASK) ? (ctx_node->flags & LYS_STATUS_MASK) : LYS_STATUS_CURR;
    } else {
        /* definition is foreign (deviation, refine), always current */
        flg1 = LYS_STATUS_CURR;
    }

    LY_ARRAY_FOR(path, u) {
        node = path[u].node;

        flg2 = (node->flags & LYS_STATUS_MASK) ? (node->flags & LYS_STATUS_MASK) : LYS_STATUS_CURR;
        if ((flg1 < flg2) && (val_mod == node->module)) {
            ret = ly_err_new(err, LY_EVALID, LYVE_REFERENCE, NULL, NULL,
                    "A %s definition \"%s\" is not allowed to reference %s value \"%s\".",
                    flg1 == LYS_STATUS_CURR ? "current" : "deprecated", ctx_node->name,
                    flg2 == LYS_STATUS_OBSLT ? "obsolete" : "deprecated", node->name);
            return ret;
        }
    }

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyplg_type_lypath_new(const struct ly_ctx *ctx, const char *value, size_t value_len, uint32_t options,
        LY_VALUE_FORMAT format, void *prefix_data, const struct lysc_node *ctx_node, struct lys_glob_unres *unres,
        struct ly_path **path, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyxp_expr *exp = NULL;
    uint32_t prefix_opt = 0;
    struct ly_err_item *e;
    const char *err_fmt = NULL;

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
    case LY_VALUE_STR_NS:
        prefix_opt = LY_PATH_PREFIX_STRICT_INHERIT;
        break;
    }

    /* remember the current last error */
    e = ly_err_last(ctx);

    /* parse the value */
    ret = ly_path_parse(ctx, ctx_node, value, value_len, 0, LY_PATH_BEGIN_ABSOLUTE, prefix_opt, LY_PATH_PRED_SIMPLE, &exp);
    if (ret) {
        err_fmt = "Invalid instance-identifier \"%.*s\" value - syntax error%s%s";
        goto cleanup;
    }

    if (options & LYPLG_TYPE_STORE_IMPLEMENT) {
        /* implement all prefixes */
        ret = lys_compile_expr_implement(ctx, exp, format, prefix_data, 1, unres, NULL);
        if (ret) {
            err_fmt = "Failed to implement a module referenced by instance-identifier \"%.*s\"%s%s";
            goto cleanup;
        }
    }

    /* resolve it on schema tree */
    ret = ly_path_compile(ctx, NULL, ctx_node, NULL, exp, (ctx_node->flags & LYS_IS_OUTPUT) ?
            LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT, LY_PATH_TARGET_SINGLE, 1, format, prefix_data, path);
    if (ret) {
        err_fmt = "Invalid instance-identifier \"%.*s\" value - semantic error%s%s";
        goto cleanup;
    }

cleanup:
    lyxp_expr_free(ctx, exp);
    if (ret) {
        /* generate error, spend the context error, if any */
        e = e ? e->next : ly_err_last(ctx);
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, err_fmt, (int)value_len, value, e ? ": " : ".", e ? e->msg : "");
        if (e) {
            ly_err_clean((struct ly_ctx *)ctx, e);
        }

        ly_path_free(ctx, *path);
        *path = NULL;
    }

    return ret;
}

LIBYANG_API_DEF void
lyplg_type_lypath_free(const struct ly_ctx *ctx, struct ly_path *path)
{
    ly_path_free(ctx, path);
}

LIBYANG_API_DEF LY_ERR
lyplg_type_make_implemented(struct lys_module *mod, const char **features, struct lys_glob_unres *unres)
{
    if (mod->implemented) {
        return LY_SUCCESS;
    }

    LY_CHECK_RET(lys_implement(mod, features, unres));
    LY_CHECK_RET(lys_compile(mod, &unres->ds_unres));

    return LY_SUCCESS;
}

LIBYANG_API_DEF LY_ERR
lyplg_type_identity_isderived(const struct lysc_ident *base, const struct lysc_ident *der)
{
    LY_ARRAY_COUNT_TYPE u;

    assert(base->module->ctx == der->module->ctx);

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

/**
 * @brief Try to generate a path to the leafref target with its value to enable the use of hash-based search.
 *
 * @param[in] path Leafref path.
 * @param[in] ctx_node Leafref context node.
 * @param[in] format Format of @p path.
 * @param[in] prefix_data Prefix data of @p path.
 * @param[in] target_val Leafref target value.
 * @param[out] target_path Generated path with the target value.
 * @return LY_SUCCESS on success.
 * @return LY_ENOT if no matching target exists.
 * @return LY_ERR on error.
 */
static LY_ERR
lyplg_type_resolve_leafref_get_target_path(const struct lyxp_expr *path, const struct lysc_node *ctx_node,
        LY_VALUE_FORMAT format, void *prefix_data, const char *target_val, struct lyxp_expr **target_path)
{
    LY_ERR rc = LY_SUCCESS;
    uint8_t oper;
    struct ly_path *p = NULL;
    char *str_path = NULL, quot;
    int len;
    ly_bool list_key = 0;

    *target_path = NULL;

    /* compile, has already been so it must succeed */
    oper = (ctx_node->flags & LYS_IS_OUTPUT) ? LY_PATH_OPER_OUTPUT : LY_PATH_OPER_INPUT;
    if (ly_path_compile_leafref(ctx_node->module->ctx, ctx_node, NULL, path, oper, LY_PATH_TARGET_MANY, format,
            prefix_data, &p)) {
        /* the target was found before but is disabled so it was removed */
        return LY_ENOT;
    }

    /* check whether we can search for a list instance with a specific key value */
    if (lysc_is_key(p[LY_ARRAY_COUNT(p) - 1].node)) {
        if ((LY_ARRAY_COUNT(p) >= 2) && (p[LY_ARRAY_COUNT(p) - 2].node->nodetype == LYS_LIST)) {
            if ((path->tokens[path->used - 1] == LYXP_TOKEN_NAMETEST) &&
                    (path->tokens[path->used - 2] == LYXP_TOKEN_OPER_PATH) &&
                    (path->tokens[path->used - 3] == LYXP_TOKEN_NAMETEST)) {
                list_key = 1;
            } /* else again, should be possible but does not make sense */
        } /* else allowed despite not making sense */
    }

    if (list_key) {
        /* get the length of the orig expression without the last "/" and the key node */
        len = path->tok_pos[path->used - 3] + path->tok_len[path->used - 3];

        /* generate the string path evaluated using hashes */
        quot = strchr(target_val, '\'') ? '\"' : '\'';
        if (asprintf(&str_path, "%.*s[%s=%c%s%c]/%s", len, path->expr, path->expr + path->tok_pos[path->used - 1],
                quot, target_val, quot, path->expr + path->tok_pos[path->used - 1]) == -1) {
            LOGMEM(ctx_node->module->ctx);
            rc = LY_EMEM;
            goto cleanup;
        }

    } else {
        /* leaf will not be found using hashes, but generate the path just to unify it */
        assert(p[LY_ARRAY_COUNT(p) - 1].node->nodetype & LYD_NODE_TERM);

        /* generate the string path evaluated using hashes */
        quot = strchr(target_val, '\'') ? '\"' : '\'';
        if (asprintf(&str_path, "%s[.=%c%s%c]", path->expr, quot, target_val, quot) == -1) {
            LOGMEM(ctx_node->module->ctx);
            rc = LY_EMEM;
            goto cleanup;
        }
    }

    /* parse into an expression */
    LY_CHECK_GOTO(lyxp_expr_parse(ctx_node->module->ctx, str_path, 0, 1, target_path), cleanup);

cleanup:
    ly_path_free(ctx_node->module->ctx, p);
    free(str_path);
    return rc;
}

LIBYANG_API_DEF LY_ERR
lyplg_type_resolve_leafref(const struct lysc_type_leafref *lref, const struct lyd_node *node, struct lyd_value *value,
        const struct lyd_node *tree, struct lyd_node **target, char **errmsg)
{
    LY_ERR rc = LY_SUCCESS;
    struct lyxp_expr *target_path = NULL;
    struct lyxp_set set = {0};
    const char *val_str, *xp_err_msg;
    uint32_t i;
    int r;

    LY_CHECK_ARG_RET(NULL, lref, node, value, errmsg, LY_EINVAL);

    if (target) {
        *target = NULL;
    }

    /* get the canonical value */
    val_str = lyd_value_get_canonical(LYD_CTX(node), value);

    if (!strchr(val_str, '\"') || !strchr(val_str, '\'')) {
        /* get the path with the value */
        r = lyplg_type_resolve_leafref_get_target_path(lref->path, node->schema, LY_VALUE_SCHEMA_RESOLVED, lref->prefixes,
                val_str, &target_path);
        if (r == LY_ENOT) {
            goto cleanup;
        } else if (r) {
            rc = r;
            goto cleanup;
        }
    } /* else value with both ' and ", XPath does not support that */

    /* find the target data instance(s) */
    rc = lyxp_eval(LYD_CTX(node), target_path ? target_path : lref->path, node->schema->module,
            LY_VALUE_SCHEMA_RESOLVED, lref->prefixes, node, node, tree, NULL, &set, LYXP_IGNORE_WHEN);
    if (rc) {
        if (ly_errcode(LYD_CTX(node)) == rc) {
            xp_err_msg = ly_errmsg(LYD_CTX(node));
        } else {
            xp_err_msg = NULL;
        }

        if (xp_err_msg) {
            r = asprintf(errmsg, "Invalid leafref value \"%s\" - XPath evaluation error (%s).", val_str, xp_err_msg);
        } else {
            r = asprintf(errmsg, "Invalid leafref value \"%s\" - XPath evaluation error.", val_str);
        }
        if (r == -1) {
            *errmsg = NULL;
            rc = LY_EMEM;
        }
        goto cleanup;
    }

    /* check the result */
    if (target_path) {
        /* no or exact match(es) */
        i = 0;
    } else {
        /* check whether any matches */
        for (i = 0; i < set.used; ++i) {
            if (set.val.nodes[i].type != LYXP_NODE_ELEM) {
                continue;
            }
            if (((struct lyd_node_term *)set.val.nodes[i].node)->value.realtype != value->realtype) {
                continue;
            }

            if (!lref->plugin->compare(&((struct lyd_node_term *)set.val.nodes[i].node)->value, value)) {
                break;
            }
        }
    }

    if (i == set.used) {
        /* no match found */
        rc = LY_ENOTFOUND;
        if (asprintf(errmsg, LY_ERRMSG_NOLREF_VAL, val_str, lref->path->expr) == -1) {
            *errmsg = NULL;
            rc = LY_EMEM;
        }
        goto cleanup;
    }
    if (target) {
        *target = set.val.nodes[i].node;
    }

cleanup:
    lyxp_expr_free(LYD_CTX(node), target_path);
    lyxp_set_free_content(&set);
    return rc;
}
