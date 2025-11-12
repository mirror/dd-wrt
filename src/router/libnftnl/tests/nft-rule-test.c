/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2013 by Ana Rey Botello <anarey@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <linux/netfilter/nf_tables.h>
#include <libnftnl/rule.h>
#include <libnftnl/udata.h>

static int test_ok = 1;

static void print_err(const char *msg)
{
	test_ok = 0;
	printf("\033[31mERROR:\e[0m %s\n", msg);
}

static void cmp_nftnl_rule(struct nftnl_rule *a, struct nftnl_rule *b)
{
	const void *udata_a, *udata_b;
	uint32_t len_a, len_b;

	if (nftnl_rule_get_u32(a, NFTNL_RULE_FAMILY) !=
	    nftnl_rule_get_u32(b, NFTNL_RULE_FAMILY))
		print_err("Rule family mismatches");
	if (strcmp(nftnl_rule_get_str(a, NFTNL_RULE_TABLE),
		   nftnl_rule_get_str(b, NFTNL_RULE_TABLE)) != 0)
		print_err("Rule table mismatches");
	if (strcmp(nftnl_rule_get_str(a, NFTNL_RULE_CHAIN),
		   nftnl_rule_get_str(b, NFTNL_RULE_CHAIN)) != 0)
		print_err("Rule table mismatches");
	if (nftnl_rule_get_u64(a, NFTNL_RULE_HANDLE) !=
	    nftnl_rule_get_u64(b, NFTNL_RULE_HANDLE))
		print_err("Rule handle mismatches");
	if (nftnl_rule_get_u32(a, NFTNL_RULE_COMPAT_PROTO) !=
	    nftnl_rule_get_u32(b, NFTNL_RULE_COMPAT_PROTO))
		print_err("Rule compat_proto mismatches");
	if (nftnl_rule_get_u32(a, NFTNL_RULE_COMPAT_FLAGS) !=
	    nftnl_rule_get_u32(b, NFTNL_RULE_COMPAT_FLAGS))
		print_err("Rule compat_flags mismatches");
	if (nftnl_rule_get_u32(a, NFTNL_RULE_ID) !=
            nftnl_rule_get_u32(b, NFTNL_RULE_ID))
                print_err("Rule id mismatches");
	if (nftnl_rule_get_u32(a, NFTNL_RULE_POSITION_ID) !=
            nftnl_rule_get_u32(b, NFTNL_RULE_POSITION_ID))
                print_err("Rule position_id mismatches");
	if (nftnl_rule_get_u64(a, NFTNL_RULE_POSITION) !=
	    nftnl_rule_get_u64(b, NFTNL_RULE_POSITION))
		print_err("Rule compat_position mismatches");

	udata_a = nftnl_rule_get_data(a, NFTNL_RULE_USERDATA, &len_a);
	udata_b = nftnl_rule_get_data(b, NFTNL_RULE_USERDATA, &len_b);

	if (len_a != len_b || memcmp(udata_a, udata_b, len_a) != 0)
		print_err("Rule userdata mismatches");
}

int main(int argc, char *argv[])
{
	struct nftnl_udata_buf *udata;
	struct nftnl_rule *a, *b;
	char buf[4096];
	struct nlmsghdr *nlh;

	a = nftnl_rule_alloc();
	b = nftnl_rule_alloc();
	if (a == NULL || b == NULL)
		print_err("OOM");

	udata = nftnl_udata_buf_alloc(NFT_USERDATA_MAXLEN);
	if (!udata)
		print_err("OOM");

	if (!nftnl_udata_put_strz(udata, 0, "hello world"))
		print_err("User data too big");

	nftnl_rule_set_u32(a, NFTNL_RULE_FAMILY, AF_INET);
	nftnl_rule_set_str(a, NFTNL_RULE_TABLE, "table");
	nftnl_rule_set_str(a, NFTNL_RULE_CHAIN, "chain");
	nftnl_rule_set_u64(a, NFTNL_RULE_HANDLE, 0x1234567812345678);
	nftnl_rule_set_u32(a, NFTNL_RULE_COMPAT_PROTO, 0x12345678);
	nftnl_rule_set_u32(a, NFTNL_RULE_COMPAT_FLAGS, 0x12345678);
	nftnl_rule_set_u32(a, NFTNL_RULE_ID, 0x12345678);
	nftnl_rule_set_u32(a, NFTNL_RULE_POSITION_ID, 0x12345678);
	nftnl_rule_set_u64(a, NFTNL_RULE_POSITION, 0x1234567812345678);
	nftnl_rule_set_data(a, NFTNL_RULE_USERDATA,
			    nftnl_udata_buf_data(udata),
			    nftnl_udata_buf_len(udata));
	nftnl_udata_buf_free(udata);

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_NEWRULE, AF_INET, 0, 1234);
	nftnl_rule_nlmsg_build_payload(nlh, a);

	if (nftnl_rule_nlmsg_parse(nlh, b) < 0)
		print_err("parsing problems");

	cmp_nftnl_rule(a,b);

	nftnl_rule_free(a);
	nftnl_rule_free(b);
	if (!test_ok)
		exit(EXIT_FAILURE);

	printf("%s: \033[32mOK\e[0m\n", argv[0]);
	return EXIT_SUCCESS;
}
