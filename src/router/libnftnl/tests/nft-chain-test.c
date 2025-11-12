/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2013 by Ana Rey Botello <anarey@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <linux/netfilter/nf_tables.h>
#include <libnftnl/chain.h>

static int test_ok = 1;

static void print_err(const char *msg)
{
	test_ok = 0;
	printf("\033[31mERROR:\e[0m %s\n", msg);
}

static void cmp_devices(const char * const *adevs,
			const char * const *bdevs)
{
	int i;

	if (!adevs && !bdevs)
		return;
	if (!!adevs ^ !!bdevs)
		print_err("Chain devices mismatches");
	for (i = 0; adevs[i] && bdevs[i]; i++) {
		if (strcmp(adevs[i], bdevs[i]))
			break;
	}
	if (adevs[i] || bdevs[i])
		print_err("Chain devices mismatches");
}

static void cmp_nftnl_chain(struct nftnl_chain *a, struct nftnl_chain *b)
{
	if (strcmp(nftnl_chain_get_str(a, NFTNL_CHAIN_NAME),
		   nftnl_chain_get_str(b, NFTNL_CHAIN_NAME)) != 0)
		print_err("Chain name mismatches");
	if (strcmp(nftnl_chain_get_str(a, NFTNL_CHAIN_TABLE),
		   nftnl_chain_get_str(b, NFTNL_CHAIN_TABLE)) != 0)
		print_err("Chain table mismatches");
	if (nftnl_chain_get_u32(a, NFTNL_CHAIN_FAMILY) !=
	    nftnl_chain_get_u32(b, NFTNL_CHAIN_FAMILY))
		print_err("Chain family mismatches");
	if (nftnl_chain_get_u32(a, NFTNL_CHAIN_POLICY) !=
	    nftnl_chain_get_u32(b, NFTNL_CHAIN_POLICY))
		print_err("Chain policy mismatches");
	if (nftnl_chain_get_u32(a, NFTNL_CHAIN_HOOKNUM) !=
	    nftnl_chain_get_u32(b, NFTNL_CHAIN_HOOKNUM))
		print_err("Chain hooknum mismatches");
	if (nftnl_chain_get_s32(a, NFTNL_CHAIN_PRIO) !=
	    nftnl_chain_get_s32(b, NFTNL_CHAIN_PRIO))
		print_err("Chain Prio mismatches");
	if (nftnl_chain_get_u32(a, NFTNL_CHAIN_USE) !=
	    nftnl_chain_get_u32(b, NFTNL_CHAIN_USE))
		print_err("Chain use mismatches");
	if (nftnl_chain_get_u64(a, NFTNL_CHAIN_PACKETS) !=
	    nftnl_chain_get_u64(b, NFTNL_CHAIN_PACKETS))
		print_err("Chain packets mismatches");
	if (nftnl_chain_get_u64(a, NFTNL_CHAIN_BYTES) !=
	    nftnl_chain_get_u64(b, NFTNL_CHAIN_BYTES))
		print_err("Chain bytes mismatches");
	if (nftnl_chain_get_u64(a, NFTNL_CHAIN_HANDLE) !=
	    nftnl_chain_get_u64(b, NFTNL_CHAIN_HANDLE))
		print_err("Chain handle mismatches");
	if (strcmp(nftnl_chain_get_str(a, NFTNL_CHAIN_TYPE),
		   nftnl_chain_get_str(b, NFTNL_CHAIN_TYPE)) != 0)
		print_err("Chain type mismatches");
	if (nftnl_chain_is_set(a, NFTNL_CHAIN_DEV) &&
	    strcmp(nftnl_chain_get_str(a, NFTNL_CHAIN_DEV),
		   nftnl_chain_get_str(b, NFTNL_CHAIN_DEV)) != 0)
		print_err("Chain device mismatches");
	cmp_devices(nftnl_chain_get_array(a, NFTNL_CHAIN_DEVICES),
		    nftnl_chain_get_array(b, NFTNL_CHAIN_DEVICES));
}

int main(int argc, char *argv[])
{
	const char *devs[] = { "eth0", "eth1", "eth2", NULL };
	struct nftnl_chain *a, *b;
	char buf[4096];
	struct nlmsghdr *nlh;

	a = nftnl_chain_alloc();
	b = nftnl_chain_alloc();
	if (a == NULL || b == NULL)
		print_err("OOM");

	nftnl_chain_set_str(a, NFTNL_CHAIN_NAME, "test");
	nftnl_chain_set_u32(a, NFTNL_CHAIN_FAMILY, AF_INET);
	nftnl_chain_set_str(a, NFTNL_CHAIN_TABLE, "Table");
	nftnl_chain_set_u32(a, NFTNL_CHAIN_POLICY,0x12345678);
	nftnl_chain_set_u32(a, NFTNL_CHAIN_HOOKNUM, 0x34567812);
	nftnl_chain_set_s32(a, NFTNL_CHAIN_PRIO, 0x56781234);
	nftnl_chain_set_u32(a, NFTNL_CHAIN_USE, 0x78123456);
	nftnl_chain_set_u64(a, NFTNL_CHAIN_PACKETS, 0x1234567812345678);
	nftnl_chain_set_u64(a, NFTNL_CHAIN_BYTES, 0x7812345678123456);
	nftnl_chain_set_u64(a, NFTNL_CHAIN_HANDLE, 0x5678123456781234);
	nftnl_chain_set_str(a, NFTNL_CHAIN_TYPE, "Prueba");
	nftnl_chain_set_str(a, NFTNL_CHAIN_DEV, "eth0");

	/* cmd extracted from include/linux/netfilter/nf_tables.h */
	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_NEWCHAIN, AF_INET, 0, 1234);
	nftnl_chain_nlmsg_build_payload(nlh, a);

	if (nftnl_chain_nlmsg_parse(nlh, b) < 0)
		print_err("parsing problems");

	cmp_nftnl_chain(a, b);

	/* repeat test with multiple devices */

	nftnl_chain_unset(a, NFTNL_CHAIN_DEV);
	nftnl_chain_set_array(a, NFTNL_CHAIN_DEVICES, devs);

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_NEWCHAIN, AF_INET, 0, 1234);
	nftnl_chain_nlmsg_build_payload(nlh, a);

	if (nftnl_chain_nlmsg_parse(nlh, b) < 0)
		print_err("parsing problems");

	cmp_nftnl_chain(a, b);

	nftnl_chain_free(a);
	nftnl_chain_free(b);

	if (!test_ok)
		exit(EXIT_FAILURE);

	printf("%s: \033[32mOK\e[0m\n", argv[0]);
	return EXIT_SUCCESS;

}
