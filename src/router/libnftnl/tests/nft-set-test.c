/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2013 by Ana Rey Botello <anarey@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <linux/netfilter/nf_tables.h>

#include <libnftnl/set.h>

static int test_ok = 1;

static void print_err(const char *msg)
{
	test_ok = 0;
	printf("\033[31mERROR:\e[0m %s\n", msg);
}

static void cmp_nftnl_set(struct nftnl_set *a, struct nftnl_set *b)
{
	const uint8_t *data_a, *data_b;
	uint32_t datalen_a, datalen_b;

	if (strcmp(nftnl_set_get_str(a, NFTNL_SET_TABLE),
		   nftnl_set_get_str(b, NFTNL_SET_TABLE)) != 0)
		print_err("Set table mismatches");
	if (strcmp(nftnl_set_get_str(a, NFTNL_SET_NAME),
		   nftnl_set_get_str(b, NFTNL_SET_NAME)) != 0)
		print_err("Set name mismatches");
	if (nftnl_set_get_u32(a, NFTNL_SET_FLAGS) !=
	    nftnl_set_get_u32(b, NFTNL_SET_FLAGS))
		print_err("Set flags mismatches");
	if (nftnl_set_get_u32(a, NFTNL_SET_KEY_TYPE) !=
	    nftnl_set_get_u32(b, NFTNL_SET_KEY_TYPE))
		print_err("Set key-type mismatches");
	if (nftnl_set_get_u32(a, NFTNL_SET_KEY_LEN) !=
	    nftnl_set_get_u32(b, NFTNL_SET_KEY_LEN))
		print_err("Set key-len mismatches");
	if (nftnl_set_get_u32(a, NFTNL_SET_DATA_TYPE) !=
	    nftnl_set_get_u32(b, NFTNL_SET_DATA_TYPE))
		print_err("Set data-type mismatches");
	if (nftnl_set_get_u32(a, NFTNL_SET_DATA_LEN) !=
	    nftnl_set_get_u32(b, NFTNL_SET_DATA_LEN))
		print_err("Set data-len mismatches");
	if (strcmp(nftnl_set_get_str(a, NFTNL_SET_USERDATA),
		   nftnl_set_get_str(b, NFTNL_SET_USERDATA)) != 0)
		print_err("Set userdata mismatches");

	data_a = nftnl_set_get_data(a, NFTNL_SET_DESC_CONCAT, &datalen_a);
	data_b = nftnl_set_get_data(b, NFTNL_SET_DESC_CONCAT, &datalen_b);
	if (datalen_a != datalen_b ||
	    memcmp(data_a, data_b, datalen_a))
		print_err("Set desc concat mismatches");
}

int main(int argc, char *argv[])
{
	struct nftnl_set *a, *b = NULL;
	uint8_t field_lengths[16];
	char buf[4096];
	struct nlmsghdr *nlh;

	a = nftnl_set_alloc();
	b = nftnl_set_alloc();
	if (a == NULL || b == NULL)
		print_err("OOM");

	nftnl_set_set_str(a, NFTNL_SET_TABLE, "test-table");
	nftnl_set_set_str(a, NFTNL_SET_NAME, "test-name");
	nftnl_set_set_u32(a, NFTNL_SET_FLAGS, 0x12345678);
	nftnl_set_set_u32(a, NFTNL_SET_KEY_TYPE, 0x12345678);
	nftnl_set_set_u32(a, NFTNL_SET_KEY_LEN, 0x12345678);
	nftnl_set_set_u32(a, NFTNL_SET_DATA_TYPE, 0x12345678);
	nftnl_set_set_u32(a, NFTNL_SET_DATA_LEN, 0x12345678);
	nftnl_set_set_u32(a, NFTNL_SET_FAMILY, 0x12345678);
	nftnl_set_set_str(a, NFTNL_SET_USERDATA, "testing user data");

	memset(field_lengths, 0xff, sizeof(field_lengths));
	if (!nftnl_set_set_data(a, NFTNL_SET_DESC_CONCAT, field_lengths, 17))
		print_err("oversized NFTNL_SET_DESC_CONCAT data accepted");
	if (nftnl_set_set_data(a, NFTNL_SET_DESC_CONCAT, field_lengths, 16))
		print_err("setting NFTNL_SET_DESC_CONCAT failed");


	/* cmd extracted from include/linux/netfilter/nf_tables.h */
	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_NEWSET, AF_INET, 0, 1234);
	nftnl_set_nlmsg_build_payload(nlh, a);

	if (nftnl_set_nlmsg_parse(nlh, b) < 0)
		print_err("parsing problems");

	cmp_nftnl_set(a,b);

	nftnl_set_free(a); nftnl_set_free(b);

	if (!test_ok)
		exit(EXIT_FAILURE);

	printf("%s: \033[32mOK\e[0m\n", argv[0]);
	return EXIT_SUCCESS;
}
