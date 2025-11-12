/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2013 by Ana Rey Botello <anarey@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <linux/netfilter/nf_tables.h>
#include <libmnl/libmnl.h>
#include <libnftnl/rule.h>
#include <libnftnl/expr.h>

static int test_ok = 1;

static void print_err(const char *msg)
{
	test_ok = 0;
	printf("\033[31mERROR:\e[0m %s\n", msg);
}

static void cmp_nftnl_expr_verdict(struct nftnl_expr *rule_a,
				   struct nftnl_expr *rule_b)
{
	const char *chain_a, *chain_b;
	uint32_t len_a, len_b;

	if (nftnl_expr_get_u32(rule_a, NFTNL_EXPR_IMM_DREG) !=
	    nftnl_expr_get_u32(rule_b, NFTNL_EXPR_IMM_DREG))
		print_err("Expr NFTNL_EXPR_IMM_DREG mismatches");

	if (nftnl_expr_get_u32(rule_a, NFTNL_EXPR_IMM_VERDICT) !=
	    nftnl_expr_get_u32(rule_b, NFTNL_EXPR_IMM_VERDICT))
		print_err("Expr NFTNL_EXPR_IMM_VERDICT mismatches");

	chain_a = nftnl_expr_get(rule_a, NFTNL_EXPR_IMM_CHAIN, &len_a);
	chain_b = nftnl_expr_get(rule_b, NFTNL_EXPR_IMM_CHAIN, &len_b);
	if (len_a != len_b || strncmp(chain_a, chain_b, len_a))
		print_err("Expr NFTNL_EXPR_IMM_CHAIN mismatches");
}

static void cmp_nftnl_expr_value(struct nftnl_expr *rule_a,
				 struct nftnl_expr *rule_b)
{
	const uint32_t *data_a, *data_b;
	uint32_t len_a, len_b;

	if (nftnl_expr_get_u32(rule_a, NFTNL_EXPR_IMM_DREG) !=
	    nftnl_expr_get_u32(rule_b, NFTNL_EXPR_IMM_DREG))
		print_err("Expr NFTNL_EXPR_IMM_DREG mismatches");

	data_a = nftnl_expr_get(rule_a, NFTNL_EXPR_IMM_DATA, &len_a);
	data_b = nftnl_expr_get(rule_b, NFTNL_EXPR_IMM_DATA, &len_b);
	if (len_a != len_b || memcmp(data_a, data_b, len_a))
		print_err("Expr NFTNL_EXPR_IMM_DATA mismatches");
}

int main(int argc, char *argv[])
{
	struct nftnl_rule *a, *b;
	struct nftnl_expr *ex_val, *ex_ver;
	struct nlmsghdr *nlh;
	char buf[4096];
	struct nftnl_expr_iter *iter_a, *iter_b;
	struct nftnl_expr *rule_a, *rule_b;
	const char *chain = "tests_chain01234";
	const char *data = "test_data_01234";

	a = nftnl_rule_alloc();
	b = nftnl_rule_alloc();
	if (a == NULL || b == NULL)
		print_err("OOM");
	ex_val = nftnl_expr_alloc("immediate");
	ex_ver = nftnl_expr_alloc("immediate");
	if (!ex_val || !ex_ver)
		print_err("OOM");

	nftnl_expr_set_u32(ex_val, NFTNL_EXPR_IMM_DREG, 0x1234568);
	nftnl_expr_set(ex_val,     NFTNL_EXPR_IMM_DATA, data, sizeof(data));

	nftnl_expr_set_u32(ex_ver, NFTNL_EXPR_IMM_DREG,    0x1234568);
	nftnl_expr_set_u32(ex_ver, NFTNL_EXPR_IMM_VERDICT, NFT_GOTO);
	nftnl_expr_set(ex_ver,     NFTNL_EXPR_IMM_CHAIN, chain, sizeof(chain));

	nftnl_rule_add_expr(a, ex_val);
	nftnl_rule_add_expr(a, ex_ver);

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_NEWRULE, AF_INET, 0, 1234);
	nftnl_rule_nlmsg_build_payload(nlh, a);

	if (nftnl_rule_nlmsg_parse(nlh, b) < 0)
		print_err("parsing problems");

	iter_a = nftnl_expr_iter_create(a);
	iter_b = nftnl_expr_iter_create(b);
	if (iter_a == NULL || iter_b == NULL)
		print_err("OOM");

	rule_a = nftnl_expr_iter_next(iter_a);
	rule_b = nftnl_expr_iter_next(iter_b);
	if (rule_a == NULL || rule_b == NULL)
		print_err("OOM");

	cmp_nftnl_expr_value(rule_a, rule_b);

	rule_a = nftnl_expr_iter_next(iter_a);
	rule_b = nftnl_expr_iter_next(iter_b);
	if (rule_a == NULL || rule_b == NULL)
		print_err("OOM");

	cmp_nftnl_expr_verdict(rule_a, rule_b);

	if (nftnl_expr_iter_next(iter_a) != NULL ||
	    nftnl_expr_iter_next(iter_b) != NULL)
		print_err("More 2 expr.");

	nftnl_expr_iter_destroy(iter_a);
	nftnl_expr_iter_destroy(iter_b);
	nftnl_rule_free(a);
	nftnl_rule_free(b);

	if (!test_ok)
		exit(EXIT_FAILURE);

	printf("%s: \033[32mOK\e[0m\n", argv[0]);
	return EXIT_SUCCESS;
}
