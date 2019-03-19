/**
 * @file user_ipv4.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Example implementation of an ipv4-address as a user type
 *
 * Copyright (c) 2018 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../user_types.h"

static int
ipv4_store_clb(const char *type_name, const char *value_str, lyd_val *value, char **err_msg)
{
    value->ptr = malloc(sizeof(struct in_addr));
    if (!value->ptr) {
        return 1;
    }

    if (inet_pton(AF_INET, value_str, value->ptr) != 1) {
        free(value->ptr);
        return 1;
    }
    return 0;
}

/* Name of this array must match the file name! */
struct lytype_plugin_list user_ipv4[] = {
    {"ietf-inet-types", "2013-07-15", "ipv4-address", ipv4_store_clb, free},
    {"ietf-inet-types", "2013-07-15", "ipv4-address-no-zone", ipv4_store_clb, free},
    {NULL, NULL, NULL, NULL, NULL} /* terminating item */
};
