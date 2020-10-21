/**
 * @file user_inet_types.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief ietf-inet-types typedef conversion to canonical format
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "compat.h"
#include "../user_types.h"

/**
 * @brief Storage for ID used to check plugin API version compatibility.
 */
LYTYPE_VERSION_CHECK

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

static char *
convert_ipv6_addr(const char *ipv6_addr, char **err_msg)
{
    char buf[sizeof(struct in6_addr)], *str;

    str = malloc(INET6_ADDRSTRLEN);
    if (!str) {
        *err_msg = NULL;
        return NULL;
    }

    if (!inet_pton(AF_INET6, ipv6_addr, buf)) {
        if (asprintf(err_msg, "Failed to convert IPv6 address \"%s\".", ipv6_addr) == -1) {
            *err_msg = NULL;
        }
        free(str);
        return NULL;
    }

    if (!inet_ntop(AF_INET6, buf, str, INET6_ADDRSTRLEN)) {
        if (asprintf(err_msg, "Failed to convert IPv6 address (%s).", strerror(errno)) == -1) {
            *err_msg = NULL;
        }
        free(str);
        return NULL;
    }

    return str;
}

static int
ip_store_clb(struct ly_ctx *ctx, const char *UNUSED(type_name), const char **value_str, lyd_val *value, char **err_msg)
{
    char *ptr, *ipv6_addr, *result, *tmp;

    if (!strchr(*value_str, ':')) {
        /* not an IPv6 address */
        return 0;
    }

    if ((ptr = strchr(*value_str, '%'))) {
        /* there is a zone index */
        ipv6_addr = strndup(*value_str, ptr - *value_str);
    } else {
        ipv6_addr = (char *)*value_str;
    }

    /* convert to canonical format */
    result = convert_ipv6_addr(ipv6_addr, err_msg);
    if (ptr) {
        free(ipv6_addr);
    }

    /* failure */
    if (!result) {
        return 1;
    }

    if (strncmp(*value_str, result, strlen(result))) {
        /* some conversion took place, update the value */
        if (ptr) {
            tmp = result;
            if (asprintf(&result, "%s%s", tmp, ptr) == -1) {
                free(tmp);
                *err_msg = NULL;
                return 1;
            }
            free(tmp);
        }

        lydict_remove(ctx, *value_str);
        *value_str = lydict_insert_zc(ctx, result);
        value->string = *value_str;
    } else {
        free(result);
    }

    return 0;
}

static int
ipv4_prefix_store_clb(struct ly_ctx *ctx, const char *UNUSED(type_name), const char **value_str, lyd_val *value, char **err_msg)
{
    char *pref_str, *ptr, *result;
    uint32_t pref, addr_bin, i, mask;

    if (sizeof addr_bin < sizeof(struct in_addr)) {
        if (asprintf(err_msg, "Internal error (buffer too small for an ip address).") == -1) {
            *err_msg = NULL;
        }
        return 1;
    }

    pref_str = strchr(*value_str, '/');
    if (!pref_str) {
        if (asprintf(err_msg, "Invalid IPv4 prefix \"%s\".", *value_str) == -1) {
            *err_msg = NULL;
        }
        return 1;
    }

    /* learn prefix */
    pref = strtoul(pref_str + 1, &ptr, 10);
    if (ptr[0] || (pref > 32)) {
        if (asprintf(err_msg, "Invalid IPv4 prefix \"%s\".", *value_str) == -1) {
            *err_msg = NULL;
        }
        return 1;
    }

    result = malloc(INET_ADDRSTRLEN + 3);
    if (!result) {
        *err_msg = NULL;
        return 1;
    }

    /* copy just the network prefix */
    strncpy(result, *value_str, pref_str - *value_str);
    result[pref_str - *value_str] = '\0';

    /* convert it to binary form */
    if (inet_pton(AF_INET, result, (void *)&addr_bin) != 1) {
        if (asprintf(err_msg, "Failed to convert IPv4 address \"%s\".", result) == -1) {
            *err_msg = NULL;
        }
        free(result);
        return 1;
    }

    /* zero host bits */
    mask = 0;
    for (i = 0; i < 32; ++i) {
        mask <<= 1;
        if (pref > i) {
            mask |= 1;
        }
    }
    mask = htonl(mask);
    addr_bin &= mask;

    /* convert back to string */
    if (!inet_ntop(AF_INET, (void *)&addr_bin, result, INET_ADDRSTRLEN)) {
        if (asprintf(err_msg, "Failed to convert IPv4 address (%s).", strerror(errno)) == -1) {
            *err_msg = NULL;
        }
        free(result);
        return 1;
    }

    /* add the prefix */
    strcat(result, pref_str);

    if (strcmp(result, *value_str)) {
        /* some conversion took place, update the value */
        lydict_remove(ctx, *value_str);
        *value_str = lydict_insert_zc(ctx, result);
        value->string = *value_str;
    } else {
        free(result);
    }

    return 0;
}

static int
ipv6_prefix_store_clb(struct ly_ctx *ctx, const char *UNUSED(type_name), const char **value_str, lyd_val *value, char **err_msg)
{
    char *pref_str, *ptr, *result;
    unsigned long int pref, i, j;
    union {
        struct in6_addr s;
        uint32_t a[4];
    } addr_bin;
    uint32_t mask;

    pref_str = strchr(*value_str, '/');
    if (!pref_str) {
        if (asprintf(err_msg, "Invalid IPv6 prefix \"%s\".", *value_str) == -1) {
            *err_msg = NULL;
        }
        return 1;
    }

    /* learn prefix */
    pref = strtoul(pref_str + 1, &ptr, 10);
    if (ptr[0] || (pref > 128)) {
        if (asprintf(err_msg, "Invalid IPv6 prefix \"%s\".", *value_str) == -1) {
            *err_msg = NULL;
        }
        return 1;
    }

    result = malloc(INET6_ADDRSTRLEN + 4);
    if (!result) {
        *err_msg = NULL;
        return 1;
    }

    /* copy just the network prefix */
    strncpy(result, *value_str, pref_str - *value_str);
    result[pref_str - *value_str] = '\0';

    /* convert it to binary form */
    if (inet_pton(AF_INET6, result, (void *)&addr_bin.s) != 1) {
        if (asprintf(err_msg, "Failed to convert IPv6 address \"%s\".", result) == -1) {
            *err_msg = NULL;
        }
        free(result);
        return 1;
    }

    /* zero host bits */
    for (i = 0; i < 4; ++i) {
        mask = 0;
        for (j = 0; j < 32; ++j) {
            mask <<= 1;
            if (pref > (i * 32) + j) {
                mask |= 1;
            }
        }
        mask = htonl(mask);
        addr_bin.a[i] &= mask;
    }

    /* convert back to string */
    if (!inet_ntop(AF_INET6, (void *)&addr_bin.s, result, INET6_ADDRSTRLEN)) {
        if (asprintf(err_msg, "Failed to convert IPv6 address (%s).", strerror(errno)) == -1) {
            *err_msg = NULL;
        }
        free(result);
        return 1;
    }

    /* add the prefix */
    strcat(result, pref_str);

    if (strcmp(result, *value_str)) {
        /* some conversion took place, update the value */
        lydict_remove(ctx, *value_str);
        *value_str = lydict_insert_zc(ctx, result);
        value->string = *value_str;
    } else {
        free(result);
    }

    return 0;
}

static int
ip_prefix_store_clb(struct ly_ctx *ctx, const char *type_name, const char **value_str, lyd_val *value, char **err_msg)
{
    if (strchr(*value_str, ':')) {
        return ipv6_prefix_store_clb(ctx, type_name, value_str, value, err_msg);
    }
    return ipv4_prefix_store_clb(ctx, type_name, value_str, value, err_msg);
}

/* Name of this array must match the file name! */
struct lytype_plugin_list user_inet_types[] = {
    {"ietf-inet-types", "2013-07-15", "ip-address", ip_store_clb, NULL},
    {"ietf-inet-types", "2013-07-15", "ipv6-address", ip_store_clb, NULL},
    {"ietf-inet-types", "2013-07-15", "ip-address-no-zone", ip_store_clb, NULL},
    {"ietf-inet-types", "2013-07-15", "ipv6-address-no-zone", ip_store_clb, NULL},
    {"ietf-inet-types", "2013-07-15", "ip-prefix", ip_prefix_store_clb, NULL},
    {"ietf-inet-types", "2013-07-15", "ipv4-prefix", ipv4_prefix_store_clb, NULL},
    {"ietf-inet-types", "2013-07-15", "ipv6-prefix", ipv6_prefix_store_clb, NULL},
    {NULL, NULL, NULL, NULL, NULL} /* terminating item */
};
