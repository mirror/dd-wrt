/**
 * @file ipv6_address.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-inet-types ipv6-address type plugin.
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

#include "plugins_internal.h"
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
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

#include "compat.h"
#include "ly_common.h"

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesIPv6Address ipv6-address (ietf-inet-types)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 16       | yes       | `struct in6_addr *` | IPv6 address in network-byte order |
 * | string length | no        | `char *` | IPv6 address zone string |
 */

static void lyplg_type_free_ipv6_address(const struct ly_ctx *ctx, struct lyd_value *value);

/**
 * @brief Convert IP address with optional zone to network-byte order.
 *
 * @param[in] value Value to convert.
 * @param[in] value_len Length of @p value.
 * @param[in] options Type store callback options.
 * @param[in] ctx libyang context with dictionary.
 * @param[in,out] addr Allocated value for the address.
 * @param[out] zone Ipv6 address zone in dictionary.
 * @param[out] err Error information on error.
 * @return LY_ERR value.
 */
static LY_ERR
ipv6address_str2ip(const char *value, size_t value_len, uint32_t options, const struct ly_ctx *ctx,
        struct in6_addr *addr, const char **zone, struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    const char *addr_no_zone;
    char *zone_ptr = NULL, *addr_dyn = NULL;
    size_t zone_len;

    /* store zone and get the string IPv6 address without it */
    if ((zone_ptr = ly_strnchr(value, '%', value_len))) {
        /* there is a zone index */
        zone_len = value_len - (zone_ptr - value) - 1;
        ret = lydict_insert(ctx, zone_ptr + 1, zone_len, zone);
        LY_CHECK_GOTO(ret, cleanup);

        /* get the IP without it */
        if (options & LYPLG_TYPE_STORE_DYNAMIC) {
            *zone_ptr = '\0';
            addr_no_zone = value;
        } else {
            addr_dyn = strndup(value, zone_ptr - value);
            addr_no_zone = addr_dyn;
        }
    } else {
        /* no zone */
        *zone = NULL;

        /* get the IP terminated with zero */
        if (options & LYPLG_TYPE_STORE_DYNAMIC) {
            /* we can use the value directly */
            addr_no_zone = value;
        } else {
            addr_dyn = strndup(value, value_len);
            addr_no_zone = addr_dyn;
        }
    }

    /* store the IPv6 address in network-byte order */
    if (!inet_pton(AF_INET6, addr_no_zone, addr)) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Failed to convert IPv6 address \"%s\".", addr_no_zone);
        goto cleanup;
    }

    /* restore the value */
    if ((options & LYPLG_TYPE_STORE_DYNAMIC) && zone_ptr) {
        *zone_ptr = '%';
    }

cleanup:
    free(addr_dyn);
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_store_clb for the ipv6-address ietf-inet-types type.
 */
static LY_ERR
lyplg_type_store_ipv6_address(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value, size_t value_len,
        uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    const char *value_str = value;
    struct lysc_type_str *type_str = (struct lysc_type_str *)type;
    struct lyd_value_ipv6_address *val;
    size_t i;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    LYPLG_TYPE_VAL_INLINE_PREPARE(storage, val);
    LY_CHECK_ERR_GOTO(!val, ret = LY_EMEM, cleanup);
    storage->realtype = type;

    if (format == LY_VALUE_LYB) {
        /* validation */
        if (value_len < 16) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB ipv6-address value size %zu "
                    "(expected at least 16).", value_len);
            goto cleanup;
        }
        for (i = 16; i < value_len; ++i) {
            if (!isalnum(value_str[i])) {
                ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB ipv6-address zone character 0x%x.",
                        value_str[i]);
                goto cleanup;
            }
        }

        /* store IP address */
        memcpy(&val->addr, value, sizeof val->addr);

        /* store zone, if any */
        if (value_len > 16) {
            ret = lydict_insert(ctx, value_str + 16, value_len - 16, &val->zone);
            LY_CHECK_GOTO(ret, cleanup);
        } else {
            val->zone = NULL;
        }

        /* success */
        goto cleanup;
    }

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
    ret = ipv6address_str2ip(value, value_len, options, ctx, &val->addr, &val->zone, err);
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
        lyplg_type_free_ipv6_address(ctx, storage);
    }
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_compare_clb for the ipv6-address ietf-inet-types type.
 */
static LY_ERR
lyplg_type_compare_ipv6_address(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1,
        const struct lyd_value *val2)
{
    struct lyd_value_ipv6_address *v1, *v2;

    LYD_VALUE_GET(val1, v1);
    LYD_VALUE_GET(val2, v2);

    /* zones are NULL or in the dictionary */
    if (memcmp(&v1->addr, &v2->addr, sizeof v1->addr) || (v1->zone != v2->zone)) {
        return LY_ENOT;
    }
    return LY_SUCCESS;
}

/**
 * @brief Implementation of ::lyplg_type_sort_clb for the ipv6-address ietf-inet-types type.
 */
static int
lyplg_type_sort_ipv6_address(const struct ly_ctx *UNUSED(ctx), const struct lyd_value *val1,
        const struct lyd_value *val2)
{
    struct lyd_value_ipv6_address *v1, *v2;
    int cmp;

    LYD_VALUE_GET(val1, v1);
    LYD_VALUE_GET(val2, v2);

    cmp = memcmp(&v1->addr, &v2->addr, sizeof v1->addr);
    if (cmp != 0) {
        return cmp;
    }

    if (!v1->zone && v2->zone) {
        return -1;
    } else if (v1->zone && !v2->zone) {
        return 1;
    } else if (v1->zone && v2->zone) {
        return strcmp(v1->zone, v2->zone);
    }

    return 0;
}

/**
 * @brief Implementation of ::lyplg_type_print_clb for the ipv6-address ietf-inet-types type.
 */
static const void *
lyplg_type_print_ipv6_address(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, size_t *value_len)
{
    struct lyd_value_ipv6_address *val;
    size_t zone_len;
    char *ret;

    LYD_VALUE_GET(value, val);

    if (format == LY_VALUE_LYB) {
        if (!val->zone) {
            /* address-only, const */
            *dynamic = 0;
            if (value_len) {
                *value_len = sizeof val->addr;
            }
            return &val->addr;
        }

        /* dynamic */
        zone_len = strlen(val->zone);
        ret = malloc(sizeof val->addr + zone_len);
        LY_CHECK_RET(!ret, NULL);

        memcpy(ret, &val->addr, sizeof val->addr);
        memcpy(ret + sizeof val->addr, val->zone, zone_len);

        *dynamic = 1;
        if (value_len) {
            *value_len = sizeof val->addr + zone_len;
        }
        return ret;
    }

    /* generate canonical value if not already */
    if (!value->_canonical) {
        /* '%' + zone */
        zone_len = val->zone ? strlen(val->zone) + 1 : 0;
        ret = malloc(INET6_ADDRSTRLEN + zone_len);
        LY_CHECK_RET(!ret, NULL);

        /* get the address in string */
        if (!inet_ntop(AF_INET6, &val->addr, ret, INET6_ADDRSTRLEN)) {
            free(ret);
            LOGERR(ctx, LY_ESYS, "Failed to get IPv6 address in string (%s).", strerror(errno));
            return NULL;
        }

        /* add zone */
        if (zone_len) {
            sprintf(ret + strlen(ret), "%%%s", val->zone);
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
 * @brief Implementation of ::lyplg_type_dup_clb for the ipv6-address ietf-inet-types type.
 */
static LY_ERR
lyplg_type_dup_ipv6_address(const struct ly_ctx *ctx, const struct lyd_value *original, struct lyd_value *dup)
{
    LY_ERR ret;
    struct lyd_value_ipv6_address *orig_val, *dup_val;

    memset(dup, 0, sizeof *dup);

    ret = lydict_insert(ctx, original->_canonical, 0, &dup->_canonical);
    LY_CHECK_GOTO(ret, error);

    LYPLG_TYPE_VAL_INLINE_PREPARE(dup, dup_val);
    LY_CHECK_ERR_GOTO(!dup_val, ret = LY_EMEM, error);

    LYD_VALUE_GET(original, orig_val);
    memcpy(&dup_val->addr, &orig_val->addr, sizeof orig_val->addr);
    ret = lydict_insert(ctx, orig_val->zone, 0, &dup_val->zone);
    LY_CHECK_GOTO(ret, error);

    dup->realtype = original->realtype;
    return LY_SUCCESS;

error:
    lyplg_type_free_ipv6_address(ctx, dup);
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_free_clb for the ipv6-address ietf-inet-types type.
 */
static void
lyplg_type_free_ipv6_address(const struct ly_ctx *ctx, struct lyd_value *value)
{
    struct lyd_value_ipv6_address *val;

    lydict_remove(ctx, value->_canonical);
    value->_canonical = NULL;
    LYD_VALUE_GET(value, val);
    if (val) {
        lydict_remove(ctx, val->zone);
        LYPLG_TYPE_VAL_INLINE_DESTROY(val);
    }
}

/**
 * @brief Plugin information for ipv6-address type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_ipv6_address[] = {
    {
        .module = "ietf-inet-types",
        .revision = "2013-07-15",
        .name = "ipv6-address",

        .plugin.id = "libyang 2 - ipv6-address, version 1",
        .plugin.store = lyplg_type_store_ipv6_address,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_ipv6_address,
        .plugin.sort = lyplg_type_sort_ipv6_address,
        .plugin.print = lyplg_type_print_ipv6_address,
        .plugin.duplicate = lyplg_type_dup_ipv6_address,
        .plugin.free = lyplg_type_free_ipv6_address,
        .plugin.lyb_data_len = -1,
    },
    {0}
};
