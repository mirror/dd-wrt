/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * mbimcli -- Command line interface to control MBIM devices
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2014 Aleksander Morgado <aleksander@aleksander.es>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <json-c/json.h>

#include "mbimcli-helpers.h"
#include "mbimcli-json.h"

gboolean
mbimcli_read_uint_from_string (const gchar *str,
                               guint *out)
{
    gulong num;

    if (!str || !str[0])
        return FALSE;

    for (num = 0; str[num]; num++) {
        if (!g_ascii_isdigit (str[num]))
            return FALSE;
    }

    errno = 0;
    num = strtoul (str, NULL, 10);
    if (!errno && num <= G_MAXUINT) {
        *out = (guint)num;
        return TRUE;
    }
    return FALSE;
}


gboolean
mbimcli_print_ip_config (MbimDevice *device,
                         MbimMessage *response,
                         GError **error)
{
	MbimIPConfigurationAvailableFlag ipv4_flags;
	MbimIPConfigurationAvailableFlag ipv6_flags;
	guint32 ipv4_cnt;
	MbimIPv4Element **ipv4address;
	guint32 ipv6_cnt;
	MbimIPv6Element **ipv6address;
	const MbimIPv4 *ipv4gateway;
	const MbimIPv6 *ipv6gateway;
	guint32 ipv4_dns_cnt;
	MbimIPv4 *ipv4dnsserver;
	guint32 ipv6_dns_cnt;
	MbimIPv6 *ipv6dnsserver;
	guint32 ipv4mtu;
	guint32 ipv6mtu;
	gchar *str;
	GInetAddress *addr;
	json_object *j_obj, *j_ipv4, *j_ipv6;
	json_object *j_addrs;
	char buf[128];
	guint i;

	if (!mbim_message_ip_configuration_response_parse (
		response,
		NULL, /* sessionid */
		&ipv4_flags,
		&ipv6_flags,
		&ipv4_cnt,
		&ipv4address,
		&ipv6_cnt,
		&ipv6address,
		&ipv4gateway,
		&ipv6gateway,
		&ipv4_dns_cnt,
		&ipv4dnsserver,
		&ipv6_dns_cnt,
		&ipv6dnsserver,
		&ipv4mtu,
		&ipv6mtu,
		error))
		return FALSE;

	j_obj = json_object_new_object();
	j_ipv4 = json_object_new_object();
	j_ipv6 = json_object_new_object();

	if (ipv4_flags & MBIM_IP_CONFIGURATION_AVAILABLE_FLAG_ADDRESS) {

		j_addrs = json_object_new_array();

		for (i = 0; i < ipv4_cnt; i++) {
			const MbimIPv4Element *ip = ipv4address[i];
			const unsigned prefix = ip->on_link_prefix_length;

			addr = g_inet_address_new_from_bytes(
						(guint8 *)&ip->ipv4_address,
						G_SOCKET_FAMILY_IPV4);
			str = g_inet_address_to_string(addr);
			sprintf(buf, "%s/%u", str, prefix);
			json_object_array_add(j_addrs, json_object_new_string(buf));
			g_free(str);
			g_object_unref(addr);
		}
		json_object_object_add(j_ipv4, "ips", j_addrs);
	}

	if (ipv4_flags & MBIM_IP_CONFIGURATION_AVAILABLE_FLAG_GATEWAY) {
		addr = g_inet_address_new_from_bytes((guint8 *)ipv4gateway,
							G_SOCKET_FAMILY_IPV4);
		str = g_inet_address_to_string (addr);
		j_add_str(j_ipv4, "gw", str);
		g_free (str);
		g_object_unref (addr);
	}

	if (ipv4_flags & MBIM_IP_CONFIGURATION_AVAILABLE_FLAG_DNS) {
		j_addrs = json_object_new_array();

		for (i = 0; i < ipv4_dns_cnt; i++) {
			addr = g_inet_address_new_from_bytes(
					(guint8 *)&ipv4dnsserver[i],
					G_SOCKET_FAMILY_IPV4);
			if (!g_inet_address_get_is_any(addr)) {
				str = g_inet_address_to_string (addr);
				json_object_array_add(j_addrs,
					json_object_new_string(str));
				g_free (str);
			}
			g_object_unref(addr);
		}
		json_object_object_add(j_ipv4, "dns", j_addrs);
	}

	if (ipv4_flags & MBIM_IP_CONFIGURATION_AVAILABLE_FLAG_MTU)
		j_add_int(j_ipv4, "mtu", ipv4mtu);

	/* ivp6 */
	if (ipv6_flags & MBIM_IP_CONFIGURATION_AVAILABLE_FLAG_ADDRESS) {

		j_addrs = json_object_new_array();

		for (i = 0; i < ipv6_cnt; i++) {
			const MbimIPv6Element *ip = ipv6address[i];
			const unsigned prefix = ip->on_link_prefix_length;

			addr = g_inet_address_new_from_bytes(
						(guint8 *)&ip->ipv6_address,
						G_SOCKET_FAMILY_IPV6);
			str = g_inet_address_to_string(addr);
			sprintf(buf, "%s/%u", str, prefix);
			json_object_array_add(j_addrs,
					json_object_new_string(buf));
			g_free(str);
			g_object_unref(addr);
		}
		json_object_object_add(j_ipv6, "ips", j_addrs);
	}

	if (ipv6_flags & MBIM_IP_CONFIGURATION_AVAILABLE_FLAG_GATEWAY) {
		addr = g_inet_address_new_from_bytes((guint8 *)ipv6gateway,
							G_SOCKET_FAMILY_IPV6);
		str = g_inet_address_to_string (addr);
		j_add_str(j_ipv6, "gw", str);
		g_free (str);
		g_object_unref (addr);
	}

	if (ipv6_flags & MBIM_IP_CONFIGURATION_AVAILABLE_FLAG_DNS) {
		j_addrs = json_object_new_array();

		for (i = 0; i < ipv6_dns_cnt; i++) {
			addr = g_inet_address_new_from_bytes(
					(guint8 *)&ipv6dnsserver[i],
					G_SOCKET_FAMILY_IPV6);
			if (!g_inet_address_get_is_any(addr)) {
				str = g_inet_address_to_string (addr);
				json_object_array_add(j_addrs,
					json_object_new_string(str));
				g_free (str);
			}
			g_object_unref(addr);
		}
		json_object_object_add(j_ipv6, "dns", j_addrs);
	}

	if (ipv6_flags & MBIM_IP_CONFIGURATION_AVAILABLE_FLAG_MTU)
		j_add_int(j_ipv6, "mtu", ipv6mtu);

	mbim_ipv4_element_array_free(ipv4address);
	mbim_ipv6_element_array_free(ipv6address);
	g_free(ipv4dnsserver);
	g_free(ipv6dnsserver);

	json_object_object_add(j_obj, "ipv4", j_ipv4);
	json_object_object_add(j_obj, "ipv6", j_ipv6);
	j_finish("ipconfig", j_obj);
	return TRUE;
}

/* Expecting input as:
 *   key1=string,key2=true,key3=false...
 * Strings may also be passed enclosed between double or single quotes, like:
 *   key1="this is a string", key2='and so is this'
 */
gboolean
mbimcli_parse_key_value_string (const gchar *str,
                                GError **error,
                                MbimParseKeyValueForeachFn callback,
                                gpointer user_data)
{
    GError *inner_error = NULL;
    gchar *dupstr, *p, *key, *key_end, *value, *value_end, quote;

    g_return_val_if_fail (callback != NULL, FALSE);
    g_return_val_if_fail (str != NULL, FALSE);

    /* Allow empty strings, we'll just return with success */
    while (g_ascii_isspace (*str))
        str++;
    if (!str[0])
        return TRUE;

    dupstr = g_strdup (str);
    p = dupstr;

    while (TRUE) {
        gboolean keep_iteration = FALSE;

        /* Skip leading spaces */
        while (g_ascii_isspace (*p))
            p++;

        /* Key start */
        key = p;
        if (!g_ascii_isalnum (*key)) {
            inner_error = g_error_new (MBIM_CORE_ERROR,
                                       MBIM_CORE_ERROR_FAILED,
                                       "Key must start with alpha/num, starts with '%c'",
                                       *key);
            break;
        }

        /* Key end */
        while (g_ascii_isalnum (*p) || (*p == '-') || (*p == '_'))
            p++;
        key_end = p;
        if (key_end == key) {
            inner_error = g_error_new (MBIM_CORE_ERROR,
                                       MBIM_CORE_ERROR_FAILED,
                                       "Couldn't find a proper key");
            break;
        }

        /* Skip whitespaces, if any */
        while (g_ascii_isspace (*p))
            p++;

        /* Equal sign must be here */
        if (*p != '=') {
            inner_error = g_error_new (MBIM_CORE_ERROR,
                                       MBIM_CORE_ERROR_FAILED,
                                       "Couldn't find equal sign separator");
            break;
        }
        /* Skip the equal */
        p++;

        /* Skip whitespaces, if any */
        while (g_ascii_isspace (*p))
            p++;

        /* Do we have a quote-enclosed string? */
        if (*p == '\"' || *p == '\'') {
            quote = *p;
            /* Skip the quote */
            p++;
            /* Value start */
            value = p;
            /* Find the closing quote */
            p = strchr (p, quote);
            if (!p) {
                inner_error = g_error_new (MBIM_CORE_ERROR,
                                           MBIM_CORE_ERROR_FAILED,
                                           "Unmatched quotes in string value");
                break;
            }

            /* Value end */
            value_end = p;
            /* Skip the quote */
            p++;
        } else {
            /* Value start */
            value = p;

            /* Value end */
            while ((*p != ',') && (*p != '\0') && !g_ascii_isspace (*p))
                p++;
            value_end = p;
        }

        /* Note that we allow value == value_end here */

        /* Skip whitespaces, if any */
        while (g_ascii_isspace (*p))
            p++;

        /* If a comma is found, we should keep the iteration */
        if (*p == ',') {
            /* skip the comma */
            p++;
            keep_iteration = TRUE;
        }

        /* Got key and value, prepare them and run the callback */
        *value_end = '\0';
        *key_end = '\0';
        if (!callback (key, value, &inner_error, user_data)) {
            /* We were told to abort */
            break;
        }
        g_assert (!inner_error);

        if (keep_iteration)
            continue;

        /* Check if no more key/value pairs expected */
        if (*p == '\0')
            break;

        inner_error = g_error_new (MBIM_CORE_ERROR,
                                   MBIM_CORE_ERROR_FAILED,
                                   "Unexpected content (%s) after value",
                                   p);
        break;
    }

    g_free (dupstr);

    if (inner_error) {
        g_propagate_error (error, inner_error);
        return FALSE;
    }

    return TRUE;
}
