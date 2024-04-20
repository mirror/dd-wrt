/**
 * @file ipv6_prefix.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-inet-types ipv6-prefix type plugin.
 *
 * Copyright (c) 2019-2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE /* strndup */

#include "plugins_types.h"

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
#  include <arpa/inet.h>
#  if defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
#    include <netinet/in.h>
#    include <sys/socket.h>
#  endif
#endif
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

#include "common.h"
#include "compat.h"

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesIPv6Prefix ipv6-prefix (ietf-inet-types)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 16 | yes | `struct in6_addr *` | IPv6 address in network-byte order |
 * | 1 | yes | `uint8_t *` | prefix length up to 128 |
 */

#define LYB_VALUE_LEN 17

static void lyplg_type_free_ipv6_prefix(const struct ly_ctx *ctx, struct lyd_value *value);

/**
 * @brief Convert IP address with a prefix in string to a binary network-byte order value.
 *
 * @param[in] value String to convert.
 * @param[in] value_len Length of @p value.
 * @param[in,out] addr Allocated address value to fill.
 * @param[out] prefix Prefix length.
 * @param[out] err Error information on error.
 * @return LY_ERR value.
 */
static LY_ERR
ipv6prefix_str2ip(const char *value, size_t value_len, struct in6_addr *addr, uint8_t *prefix, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    const char *pref_str;
    char *mask_str = NULL;

    /* it passed the pattern validation */
    pref_str = ly_strnchr(value, '/', value_len);
    ly_strntou8(pref_str + 1, value_len - (pref_str + 1 - value), prefix);

    /* get just the network prefix */
    mask_str = strndup(value, pref_str - value);
    LY_CHECK_ERR_GOTO(!mask_str, ret = LY_EMEM, cleanup);

    /* convert it to netword-byte order */
    if (inet_pton(AF_INET6, mask_str, addr) != 1) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Failed to convert IPv6 address \"%s\".", mask_str);
        goto cleanup;
    }

cleanup:
    free(mask_str);
    return ret;
}

/**
 * @brief Zero host-portion of the IP address.
 *
 * @param[in,out] addr IP address.
 * @param[in] prefix Prefix length.
 */
static void
ipv6prefix_zero_host(struct in6_addr *addr, uint8_t prefix)
{
    uint32_t i, j, mask;

    /* zero host bits */
    for (i = 0; i < 4; ++i) {
        mask = 0;
        for (j = 0; j < 32; ++j) {
            mask <<= 1;
            if (prefix > (i * 32) + j) {
                mask |= 1;
            }
        }
        mask = htonl(mask);
        ((uint32_t *)addr->s6_addr)[i] &= mask;
    }
}

/**
 * @brief Implementation of ::lyplg_type_store_clb for the ipv6-prefix ietf-inet-types type.
 */
static LY_ERR
lyplg_type_store_ipv6_prefix(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_str *type_str = (struct lysc_type_str *)type;
    struct lyd_value_ipv6_prefix *val;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    storage->realtype = type;

    if (format == LY_VALUE_LYB) {
        /* validation */
        if (value_len != LYB_VALUE_LEN) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB ipv6-prefix value size %zu (expected %d).",
                    value_len, LYB_VALUE_LEN);
            goto cleanup;
        }
        if (((uint8_t *)value)[16] > 128) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB ipv6-prefix prefix length %" PRIu8 ".",
                    ((uint8_t *)value)[16]);
            goto cleanup;
        }

        /* store/allocate value */
        if ((options & LYPLG_TYPE_STORE_DYNAMIC) && LYPLG_TYPE_VAL_IS_DYN(val)) {
            storage->dyn_mem = (void *)value;
            options &= ~LYPLG_TYPE_STORE_DYNAMIC;

            LYD_VALUE_GET(storage, val);
        } else {
            LYPLG_TYPE_VAL_INLINE_PREPARE(storage, val);
            LY_CHECK_ERR_GOTO(!val, ret = LY_EMEM, cleanup);

            memcpy(val, value, value_len);
        }

        /* zero host */
        ipv6prefix_zero_host(&val->addr, val->prefix);

        /* success */
        goto cleanup;
    }

    /* allocate value */
    LYPLG_TYPE_VAL_INLINE_PREPARE(storage, val);
    LY_CHECK_ERR_GOTO(!val, ret = LY_EMEM, cleanup);

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

    /* get the mask in network-byte order */
    ret = ipv6prefix_str2ip(value, value_len, &val->addr, &val->prefix, err);
    LY_CHECK_GOTO(ret, cleanup);

    /* zero host */
    ipv6prefix_zero_host(&val->addr, val->prefix);

    if (format == LY_VALUE_CANON) {
        /* store canonical value */
        if (options & LYPLG_TYPE_STORE_DYNAMIC) {
            ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
            options &= ~LYPLG_TYPE_STORE_DYNAMIC;
            LY_CHECK_GOTO(ret, cleanup);
        } else {
            ret = lydict_insert(ctx, value_len ? value : "", value_len, &storage->_canonical);
            LY_CHECK_GOTO(ret, cleanup);
        }
    }

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_ipv6_prefix(ctx, storage);
    }
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_compare_clb for the ietf-inet-types ipv6-prefix type.
 */
static LY_ERR
lyplg_type_compare_ipv6_prefix(const struct lyd_value *val1, const struct lyd_value *val2)
{
    struct lyd_value_ipv6_prefix *v1, *v2;

    LYD_VALUE_GET(val1, v1);
    LYD_VALUE_GET(val2, v2);

    if (memcmp(v1, v2, sizeof *v1)) {
        return LY_ENOT;
    }
    return LY_SUCCESS;
}

/**
 * @brief Implementation of ::lyplg_type_compare_clb for the ietf-inet-types ipv6-prefix type.
 */
static const void *
lyplg_type_print_ipv6_prefix(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, size_t *value_len)
{
    struct lyd_value_ipv6_prefix *val;
    char *ret;

    LYD_VALUE_GET(value, val);

    if (format == LY_VALUE_LYB) {
        *dynamic = 0;
        if (value_len) {
            *value_len = LYB_VALUE_LEN;
        }
        return val;
    }

    /* generate canonical value if not already */
    if (!value->_canonical) {
        /* IPv6 mask + '/' + prefix */
        ret = malloc(INET6_ADDRSTRLEN + 4);
        LY_CHECK_RET(!ret, NULL);

        /* convert back to string */
        if (!inet_ntop(AF_INET6, &val->addr, ret, INET6_ADDRSTRLEN)) {
            free(ret);
            return NULL;
        }

        /* add the prefix */
        sprintf(ret + strlen(ret), "/%" PRIu8, val->prefix);

        /* store it */
        if (lydict_insert_zc(ctx, ret, (const char **)&value->_canonical)) {
            LOGMEM(ctx);
            return NULL;
        }
    }

    /* use the cached canonical value */
    if (dynamic) {
        *dynamic = 0;
    }
    if (value_len) {
        *value_len = strlen(value->_canonical);
    }
    return value->_canonical;
}

/**
 * @brief Implementation of ::lyplg_type_dup_clb for the ietf-inet-types ipv6-prefix type.
 */
static LY_ERR
lyplg_type_dup_ipv6_prefix(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ERR ret;
    struct lyd_value_ipv6_prefix *orig_val, *dup_val;

    memset(dup, 0, sizeof *dup);

    ret = lydict_insert(ctx, original->_canonical, 0, &dup->_canonical);
    LY_CHECK_GOTO(ret, error);

    LYPLG_TYPE_VAL_INLINE_PREPARE(dup, dup_val);
    LY_CHECK_ERR_GOTO(!dup_val, ret = LY_EMEM, error);

    LYD_VALUE_GET(original, orig_val);
    memcpy(dup_val, orig_val, sizeof *orig_val);

    dup->realtype = original->realtype;
    return LY_SUCCESS;

error:
    lyplg_type_free_ipv6_prefix(ctx, dup);
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_free_clb for the ietf-inet-types ipv6-prefix type.
 */
static void
lyplg_type_free_ipv6_prefix(const struct ly_ctx *ctx, struct lyd_value *value)
{
    struct lyd_value_ipv6_prefix *val;

    lydict_remove(ctx, value->_canonical);
    value->_canonical = NULL;
    LYD_VALUE_GET(value, val);
    LYPLG_TYPE_VAL_INLINE_DESTROY(val);
}

/**
 * @brief Plugin information for ipv6-prefix type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_ipv6_prefix[] = {
    {
        .module = "ietf-inet-types",
        .revision = "2013-07-15",
        .name = "ipv6-prefix",

        .plugin.id = "libyang 2 - ipv6-prefix, version 1",
        .plugin.store = lyplg_type_store_ipv6_prefix,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_ipv6_prefix,
        .plugin.sort = NULL,
        .plugin.print = lyplg_type_print_ipv6_prefix,
        .plugin.duplicate = lyplg_type_dup_ipv6_prefix,
        .plugin.free = lyplg_type_free_ipv6_prefix,
        .plugin.lyb_data_len = LYB_VALUE_LEN,
    },
    {0}
};
