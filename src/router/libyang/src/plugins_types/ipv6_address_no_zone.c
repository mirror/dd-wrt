/**
 * @file ipv6_address_no_zone.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-inet-types ipv6-address-no-zone type plugin.
 *
 * Copyright (c) 2019-2021 CESNET, z.s.p.o.
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

#include <arpa/inet.h>
#if defined (__FreeBSD__) || defined (__NetBSD__) || defined (__OpenBSD__)
#include <netinet/in.h>
#include <sys/socket.h>
#endif
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

#include "common.h"
#include "compat.h"

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesIPv6AddressNoZone ipv6-address-no-zone (ietf-inet-types)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 16       | yes       | `struct in6_addr *` | IPv6 address in network-byte order |
 */

static void lyplg_type_free_ipv6_address_no_zone(const struct ly_ctx *ctx, struct lyd_value *value);

/**
 * @brief Convert IP address to network-byte order.
 *
 * @param[in] value Value to convert.
 * @param[in] value_len Length of @p value.
 * @param[in] options Type store callback options.
 * @param[in,out] addr Allocated value for the address.
 * @param[out] err Error information on error.
 * @return LY_ERR value.
 */
static LY_ERR
ipv6addressnozone_str2ip(const char *value, size_t value_len, uint32_t options, struct in6_addr *addr, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    const char *addr_str;
    char *addr_dyn = NULL;

    /* get the IP terminated with zero */
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        /* we can use the value directly */
        addr_str = value;
    } else {
        addr_dyn = strndup(value, value_len);
        addr_str = addr_dyn;
    }

    /* store the IPv6 address in network-byte order */
    if (!inet_pton(AF_INET6, addr_str, addr)) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Failed to convert IPv6 address \"%s\".", addr_str);
        goto cleanup;
    }

cleanup:
    free(addr_dyn);
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_store_clb for the ipv6-address-no-zone ietf-inet-types type.
 */
static LY_ERR
lyplg_type_store_ipv6_address_no_zone(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value,
        size_t value_len, uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_str *type_str = (struct lysc_type_str *)type;
    struct lyd_value_ipv6_address_no_zone *val;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    storage->realtype = type;

    if (format == LY_VALUE_LYB) {
        /* validation */
        if (value_len != 16) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB ipv6-address-no-zone value size %zu "
                    "(expected 16).", value_len);
            goto cleanup;
        }

        if ((options & LYPLG_TYPE_STORE_DYNAMIC) && LYPLG_TYPE_VAL_IS_DYN(val)) {
            /* use the value directly */
            storage->dyn_mem = (void *)value;
            options &= ~LYPLG_TYPE_STORE_DYNAMIC;
        } else {
            /* allocate value */
            LYPLG_TYPE_VAL_INLINE_PREPARE(storage, val);
            LY_CHECK_ERR_GOTO(!val, ret = LY_EMEM, cleanup);

            /* store IP address */
            memcpy(&val->addr, value, sizeof val->addr);
        }

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

    /* get the network-byte order address */
    ret = ipv6addressnozone_str2ip(value, value_len, options, &val->addr, err);
    LY_CHECK_GOTO(ret, cleanup);

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
        lyplg_type_free_ipv6_address_no_zone(ctx, storage);
    }
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_compare_clb for the ipv6-address-no-zone ietf-inet-types type.
 */
static LY_ERR
lyplg_type_compare_ipv6_address_no_zone(const struct lyd_value *val1, const struct lyd_value *val2)
{
    struct lyd_value_ipv6_address_no_zone *v1, *v2;

    if (val1->realtype != val2->realtype) {
        return LY_ENOT;
    }

    LYD_VALUE_GET(val1, v1);
    LYD_VALUE_GET(val2, v2);

    if (memcmp(&v1->addr, &v2->addr, sizeof v1->addr)) {
        return LY_ENOT;
    }
    return LY_SUCCESS;
}

/**
 * @brief Implementation of ::lyplg_type_print_clb for the ipv6-address-no-zone ietf-inet-types type.
 */
static const void *
lyplg_type_print_ipv6_address_no_zone(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, size_t *value_len)
{
    struct lyd_value_ipv6_address_no_zone *val;
    char *ret;

    LYD_VALUE_GET(value, val);

    if (format == LY_VALUE_LYB) {
        *dynamic = 0;
        if (value_len) {
            *value_len = sizeof val->addr;
        }
        return &val->addr;
    }

    /* generate canonical value if not already */
    if (!value->_canonical) {
        /* '%' + zone */
        ret = malloc(INET6_ADDRSTRLEN);
        LY_CHECK_RET(!ret, NULL);

        /* get the address in string */
        if (!inet_ntop(AF_INET6, &val->addr, ret, INET6_ADDRSTRLEN)) {
            free(ret);
            LOGERR(ctx, LY_EVALID, "Failed to get IPv6 address in string (%s).", strerror(errno));
            return NULL;
        }

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
 * @brief Implementation of ::lyplg_type_dup_clb for the ipv6-address-no-zone ietf-inet-types type.
 */
static LY_ERR
lyplg_type_dup_ipv6_address_no_zone(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ERR ret;
    struct lyd_value_ipv6_address_no_zone *orig_val, *dup_val;

    ret = lydict_insert(ctx, original->_canonical, ly_strlen(original->_canonical), &dup->_canonical);
    LY_CHECK_RET(ret);

    LYPLG_TYPE_VAL_INLINE_PREPARE(dup, dup_val);
    if (!dup_val) {
        lydict_remove(ctx, dup->_canonical);
        return LY_EMEM;
    }

    LYD_VALUE_GET(original, orig_val);
    memcpy(&dup_val->addr, &orig_val->addr, sizeof orig_val->addr);

    dup->realtype = original->realtype;
    return LY_SUCCESS;
}

/**
 * @brief Implementation of ::lyplg_type_free_clb for the ipv6-address-no-zone ietf-inet-types type.
 */
static void
lyplg_type_free_ipv6_address_no_zone(const struct ly_ctx *ctx, struct lyd_value *value)
{
    struct lyd_value_ipv6_address_no_zone *val;

    lydict_remove(ctx, value->_canonical);
    LYD_VALUE_GET(value, val);
    LYPLG_TYPE_VAL_INLINE_DESTROY(val);
}

/**
 * @brief Plugin information for ipv6-address-no-zone type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_ipv6_address_no_zone[] = {
    {
        .module = "ietf-inet-types",
        .revision = "2013-07-15",
        .name = "ipv6-address-no-zone",

        .plugin.id = "libyang 2 - ipv6-address-no-zone, version 1",
        .plugin.store = lyplg_type_store_ipv6_address_no_zone,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_ipv6_address_no_zone,
        .plugin.sort = NULL,
        .plugin.print = lyplg_type_print_ipv6_address_no_zone,
        .plugin.duplicate = lyplg_type_dup_ipv6_address_no_zone,
        .plugin.free = lyplg_type_free_ipv6_address_no_zone
    },
    {0}
};
