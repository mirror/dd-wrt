/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2013 by Ana Rey Botello <anarey@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include <linux/netfilter/nf_tables.h>
#include <libnftnl/object.h>

static int test_ok = 1;

static void print_err(const char *msg)
{
	test_ok = 0;
	printf("\033[31mERROR:\e[0m %s\n", msg);
}

static void cmp_nftnl_obj(struct nftnl_obj *a, struct nftnl_obj *b)
{
	if (strcmp(nftnl_obj_get_str(a, NFTNL_OBJ_TABLE),
		   nftnl_obj_get_str(b, NFTNL_OBJ_TABLE)) != 0)
		print_err("table name mismatches");
	if (strcmp(nftnl_obj_get_str(a, NFTNL_OBJ_NAME),
		   nftnl_obj_get_str(b, NFTNL_OBJ_NAME)) != 0)
		print_err("name mismatches");
	if (nftnl_obj_get_u32(a, NFTNL_OBJ_FAMILY) !=
	    nftnl_obj_get_u32(b, NFTNL_OBJ_FAMILY))
		print_err("family mismatches");
	if (nftnl_obj_get_u32(a, NFTNL_OBJ_TYPE) !=
	    nftnl_obj_get_u32(b, NFTNL_OBJ_TYPE))
		print_err("type mismatches");
}

int main(int argc, char *argv[])
{
	char buf[4096];
	struct nlmsghdr *nlh;
	struct nftnl_obj *a;
	struct nftnl_obj *b;

	a = nftnl_obj_alloc();
	b = nftnl_obj_alloc();
	if (a == NULL || b == NULL)
		print_err("OOM");

	nftnl_obj_set_str(a, NFTNL_OBJ_TABLE, "test");
	nftnl_obj_set_str(a, NFTNL_OBJ_NAME, "test");
	nftnl_obj_set_u32(a, NFTNL_OBJ_FAMILY, AF_INET);
	nftnl_obj_set_u32(a, NFTNL_OBJ_USE, 1);
	nftnl_obj_set_u64(a, NFTNL_OBJ_CTR_BYTES, 0x12345678abcd);
	nftnl_obj_set_u64(a, NFTNL_OBJ_CTR_PKTS, 0xcd12345678ab);

	/* cmd extracted from include/linux/netfilter/nf_tables.h */
	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_NEWOBJ, AF_INET, 0, 1234);
	nftnl_obj_nlmsg_build_payload(nlh, a);

	if (nftnl_obj_nlmsg_parse(nlh, b) < 0)
		print_err("parsing problems");

	cmp_nftnl_obj(a, b);

	nftnl_obj_free(a);
	nftnl_obj_free(b);
	if (!test_ok)
		exit(EXIT_FAILURE);

	printf("%s: \033[32mOK\e[0m\n", argv[0]);
	return EXIT_SUCCESS;
}
