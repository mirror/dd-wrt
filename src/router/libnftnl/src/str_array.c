/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2024 Red Hat GmbH
 * Author: Phil Sutter <phil@nwl.cc>
 */
#include <libmnl/libmnl.h>
#include <linux/netfilter/nf_tables.h>

#include "str_array.h"
#include "utils.h"

void nftnl_str_array_clear(struct nftnl_str_array *sa)
{
	while (sa->len > 0)
		free(sa->array[--sa->len]);
	free(sa->array);
}

int nftnl_str_array_set(struct nftnl_str_array *sa, const char * const *array)
{
	int len = 0;

	while (array[len])
		len++;

	nftnl_str_array_clear(sa);
	sa->array = calloc(len + 1, sizeof(char *));
	if (!sa->array)
		return -1;

	while (sa->len < len) {
		sa->array[sa->len] = strdup(array[sa->len]);
		if (!sa->array[sa->len]) {
			nftnl_str_array_clear(sa);
			return -1;
		}
		sa->len++;
	}
	return 0;
}

int nftnl_parse_devs(struct nftnl_str_array *sa, const struct nlattr *nest)
{
	struct nlattr *attr;
	int len = 0;

	mnl_attr_for_each_nested(attr, nest) {
		if (mnl_attr_get_type(attr) != NFTA_DEVICE_NAME)
			return -1;
		len++;
	}

	nftnl_str_array_clear(sa);
	sa->array = calloc(len + 1, sizeof(char *));
	if (!sa->array)
		return -1;

	mnl_attr_for_each_nested(attr, nest) {
		sa->array[sa->len] = strdup(mnl_attr_get_str(attr));
		if (!sa->array[sa->len]) {
			nftnl_str_array_clear(sa);
			return -1;
		}
		sa->len++;
	}
	return 0;
}
