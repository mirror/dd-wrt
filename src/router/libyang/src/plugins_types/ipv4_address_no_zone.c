/**
 * @file ipv4_address_no_zone.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-inet-types ipv4-address-no-zone type plugin.
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
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "libyang.h"

#include "common.h"
#include "compat.h"

/**
 * @page howtoDataLYB LYB Binary Format
 * @subsection howtoDataLYBTypesIPv4AddressNoZone ipv4-address-no-zone (ietf-inet-types)
 *
 * | Size (B) | Mandatory | Type | Meaning |
 * | :------  | :-------: | :--: | :-----: |
 * | 4       | yes       | `struct in_addr *` | IPv4 address in network-byte order |
 */

/**
 * @brief Implementation of ::lyplg_type_store_clb for the ipv4-address-no-zone ietf-inet-types type.
 */
static LY_ERR
lyplg_type_store_ipv4_address_no_zone(const struct ly_ctx *ctx, const struct lysc_type *type, const void *value,
        size_t value_len, uint32_t options, LY_VALUE_FORMAT format, void *UNUSED(prefix_data), uint32_t hints,
        const struct lysc_node *UNUSED(ctx_node), struct lyd_value *storage, struct lys_glob_unres *UNUSED(unres),
        struct ly_err_item **err)
{
    LY_ERR ret = LY_SUCCESS;
    struct lysc_type_str *type_str = (struct lysc_type_str *)type;
    struct lyd_value_ipv4_address_no_zone *val;

    /* init storage */
    memset(storage, 0, sizeof *storage);
    LYPLG_TYPE_VAL_INLINE_PREPARE(storage, val);
    LY_CHECK_ERR_GOTO(!val, ret = LY_EMEM, cleanup);
    storage->realtype = type;

    if (format == LY_VALUE_LYB) {
        /* validation */
        if (value_len != 4) {
            ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Invalid LYB ipv4-address-no-zone value size %zu "
                    "(expected 4).", value_len);
            goto cleanup;
        }

        /* store IP address */
        memcpy(&val->addr, value, 4);

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

    /* we always need a dynamic value */
    if (!(options & LYPLG_TYPE_STORE_DYNAMIC)) {
        value = strndup(value, value_len);
        LY_CHECK_ERR_GOTO(!value, ret = LY_EMEM, cleanup);

        options |= LYPLG_TYPE_STORE_DYNAMIC;
    }

    /* get the network-byte order address */
    if (!inet_pton(AF_INET, value, &val->addr)) {
        ret = ly_err_new(err, LY_EVALID, LYVE_DATA, NULL, NULL, "Failed to convert IPv4 address \"%s\".", (char *)value);
        goto cleanup;
    }

    /* store canonical value */
    ret = lydict_insert_zc(ctx, (char *)value, &storage->_canonical);
    options &= ~LYPLG_TYPE_STORE_DYNAMIC;
    LY_CHECK_GOTO(ret, cleanup);

cleanup:
    if (options & LYPLG_TYPE_STORE_DYNAMIC) {
        free((void *)value);
    }

    if (ret) {
        lyplg_type_free_simple(ctx, storage);
    }
    return ret;
}

/**
 * @brief Implementation of ::lyplg_type_compare_clb for the ipv4-address-no-zone ietf-inet-types type.
 */
static LY_ERR
lyplg_type_compare_ipv4_address_no_zone(const struct lyd_value *val1, const struct lyd_value *val2)
{
    struct lyd_value_ipv4_address_no_zone *v1, *v2;

    LYD_VALUE_GET(val1, v1);
    LYD_VALUE_GET(val2, v2);

    if (memcmp(&v1->addr, &v2->addr, sizeof v1->addr)) {
        return LY_ENOT;
    }
    return LY_SUCCESS;
}

/**
 * @brief Implementation of ::lyplg_type_print_clb for the ipv4-address-no-zone ietf-inet-types type.
 */
static const void *
lyplg_type_print_ipv4_address_no_zone(const struct ly_ctx *ctx, const struct lyd_value *value, LY_VALUE_FORMAT format,
        void *UNUSED(prefix_data), ly_bool *dynamic, size_t *value_len)
{
    struct lyd_value_ipv4_address_no_zone *val;
    char *ret;

    LYD_VALUE_GET(value, val);

    if (format == LY_VALUE_LYB) {
        *dynamic = 0;
        if (value_len) {
            *value_len = 4;
        }
        return &val->addr;
    }

    /* generate canonical value if not already (loaded from LYB) */
    if (!value->_canonical) {
        ret = malloc(INET_ADDRSTRLEN);
        LY_CHECK_RET(!ret, NULL);

        /* get the address in string */
        if (!inet_ntop(AF_INET, &val->addr, ret, INET_ADDRSTRLEN)) {
            free(ret);
            LOGERR(ctx, LY_EVALID, "Failed to get IPv4 address in string (%s).", strerror(errno));
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
 * @brief Plugin information for ipv4-address-no-zone type implementation.
 *
 * Note that external plugins are supposed to use:
 *
 *   LYPLG_TYPES = {
 */
const struct lyplg_type_record plugins_ipv4_address_no_zone[] = {
    {
        .module = "ietf-inet-types",
        .revision = "2013-07-15",
        .name = "ipv4-address-no-zone",

        .plugin.id = "libyang 2 - ipv4-address-no-zone, version 1",
        .plugin.store = lyplg_type_store_ipv4_address_no_zone,
        .plugin.validate = NULL,
        .plugin.compare = lyplg_type_compare_ipv4_address_no_zone,
        .plugin.sort = NULL,
        .plugin.print = lyplg_type_print_ipv4_address_no_zone,
        .plugin.duplicate = lyplg_type_dup_simple,
        .plugin.free = lyplg_type_free_simple,
        .plugin.lyb_data_len = 4,
    },
    {0}
};
