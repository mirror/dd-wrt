/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2013 by Ana Rey Botello <anarey@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include <linux/netfilter/nf_tables.h>
#include <libnftnl/table.h>

static int test_ok = 1;

static void print_err(const char *msg)
{
	test_ok = 0;
	printf("\033[31mERROR:\e[0m %s\n", msg);
}

static void cmp_nftnl_table(struct nftnl_table *a, struct nftnl_table *b)
{
	if (strcmp(nftnl_table_get_str(a, NFTNL_TABLE_NAME),
		   nftnl_table_get_str(b, NFTNL_TABLE_NAME)) != 0)
		print_err("table name mismatches");
	if (nftnl_table_get_u32(a, NFTNL_TABLE_FLAGS) !=
	    nftnl_table_get_u32(b, NFTNL_TABLE_FLAGS))
		print_err("table flags mismatches");
	if (nftnl_table_get_u32(a, NFTNL_TABLE_FAMILY) !=
	    nftnl_table_get_u32(b, NFTNL_TABLE_FAMILY))
		print_err("table family mismatches");
}

int main(int argc, char *argv[])
{
	char buf[4096];
	struct nlmsghdr *nlh;

	struct nftnl_table *a = NULL;
	struct nftnl_table *b = NULL;
	a = nftnl_table_alloc();
	b = nftnl_table_alloc();

	if (a == NULL || b == NULL)
		print_err("OOM");

	nftnl_table_set_str(a, NFTNL_TABLE_NAME, "test");
	nftnl_table_set_u32(a, NFTNL_TABLE_FAMILY, AF_INET);
	nftnl_table_set_u32(a, NFTNL_TABLE_FLAGS, 0);

	/* cmd extracted from include/linux/netfilter/nf_tables.h */
	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_NEWTABLE, AF_INET, 0, 1234);
	nftnl_table_nlmsg_build_payload(nlh, a);

	if (nftnl_table_nlmsg_parse(nlh, b) < 0)
		print_err("parsing problems");

	cmp_nftnl_table(a,b);

	nftnl_table_free(a);
	nftnl_table_free(b);
	if (!test_ok)
		exit(EXIT_FAILURE);

	printf("%s: \033[32mOK\e[0m\n", argv[0]);
	return EXIT_SUCCESS;
}
