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

static void print_err(const char *test, const char *msg)
{
	test_ok = 0;
	printf("\033[31mERROR:\e[0m [%s] %s\n", test, msg);
}

static void cmp_nftnl_expr_mask_xor(struct nftnl_expr *rule_a,
				    struct nftnl_expr *rule_b)
{
	uint32_t maska, maskb;
	uint32_t xora, xorb;

	if (nftnl_expr_get_u32(rule_a, NFTNL_EXPR_BITWISE_DREG) !=
	    nftnl_expr_get_u32(rule_b, NFTNL_EXPR_BITWISE_DREG))
		print_err("mask & xor", "Expr BITWISE_DREG mismatches");
	if (nftnl_expr_get_u32(rule_a, NFTNL_EXPR_BITWISE_SREG) !=
	    nftnl_expr_get_u32(rule_b, NFTNL_EXPR_BITWISE_SREG))
		print_err("mask & xor", "Expr BITWISE_SREG mismatches");
	if (nftnl_expr_get_u32(rule_a, NFTNL_EXPR_BITWISE_OP) !=
	    nftnl_expr_get_u32(rule_b, NFTNL_EXPR_BITWISE_OP))
		print_err("mask & xor", "Expr BITWISE_OP mismatches");
	if (nftnl_expr_get_u16(rule_a, NFTNL_EXPR_BITWISE_LEN) !=
	    nftnl_expr_get_u16(rule_b, NFTNL_EXPR_BITWISE_LEN))
		print_err("mask & xor", "Expr BITWISE_LEN mismatches");
	nftnl_expr_get(rule_a, NFTNL_EXPR_BITWISE_MASK, &maska);
	nftnl_expr_get(rule_b, NFTNL_EXPR_BITWISE_MASK, &maskb);
	if (maska != maskb)
		print_err("mask & xor", "Size of BITWISE_MASK mismatches");
	nftnl_expr_get(rule_a, NFTNL_EXPR_BITWISE_XOR, &xora);
	nftnl_expr_get(rule_b, NFTNL_EXPR_BITWISE_XOR, &xorb);
	if (xora != xorb)
		print_err("mask & xor", "Size of BITWISE_XOR mismatches");
}

static void test_mask_xor(void)
{
	struct nftnl_rule *a, *b = NULL;
	struct nftnl_expr *ex = NULL;
	struct nlmsghdr *nlh;
	char buf[4096];
	struct nftnl_expr_iter *iter_a, *iter_b = NULL;
	struct nftnl_expr *rule_a, *rule_b = NULL;
	uint32_t mask = 0x01010101;
	uint32_t xor = 0x12345678;

	a = nftnl_rule_alloc();
	b = nftnl_rule_alloc();
	if (a == NULL || b == NULL)
		print_err("mask & xor", "OOM");
	ex = nftnl_expr_alloc("bitwise");
	if (ex == NULL)
		print_err("mask & xor", "OOM");

	nftnl_expr_set_u32(ex, NFTNL_EXPR_BITWISE_SREG, 0x12345678);
	nftnl_expr_set_u32(ex, NFTNL_EXPR_BITWISE_DREG, 0x78123456);
	nftnl_expr_set_u32(ex, NFTNL_EXPR_BITWISE_OP, NFT_BITWISE_BOOL);
	nftnl_expr_set_u32(ex, NFTNL_EXPR_BITWISE_LEN, 0x56781234);
	nftnl_expr_set(ex, NFTNL_EXPR_BITWISE_MASK, &mask, sizeof(mask));
	nftnl_expr_set(ex, NFTNL_EXPR_BITWISE_XOR, &xor, sizeof(xor));

	nftnl_rule_add_expr(a, ex);

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_NEWRULE, AF_INET, 0, 1234);
	nftnl_rule_nlmsg_build_payload(nlh, a);

	if (nftnl_rule_nlmsg_parse(nlh, b) < 0)
		print_err("mask & xor", "parsing problems");

	iter_a = nftnl_expr_iter_create(a);
	iter_b = nftnl_expr_iter_create(b);
	if (iter_a == NULL || iter_b == NULL)
		print_err("mask & xor", "OOM");

	rule_a = nftnl_expr_iter_next(iter_a);
	rule_b = nftnl_expr_iter_next(iter_b);
	if (rule_a == NULL || rule_b == NULL)
		print_err("mask & xor", "OOM");

	if (nftnl_expr_iter_next(iter_a) != NULL ||
	    nftnl_expr_iter_next(iter_b) != NULL)
		print_err("mask & xor", "More 1 expr.");

	nftnl_expr_iter_destroy(iter_a);
	nftnl_expr_iter_destroy(iter_b);

	cmp_nftnl_expr_mask_xor(rule_a,rule_b);

	nftnl_rule_free(a);
	nftnl_rule_free(b);
}

static void cmp_nftnl_expr_shift(const char *opname,
				 const struct nftnl_expr *rule_a,
				 const struct nftnl_expr *rule_b)
{
	uint32_t data_a, data_b;

	if (nftnl_expr_get_u32(rule_a, NFTNL_EXPR_BITWISE_DREG) !=
	    nftnl_expr_get_u32(rule_b, NFTNL_EXPR_BITWISE_DREG))
		print_err(opname, "Expr BITWISE_DREG mismatches");
	if (nftnl_expr_get_u32(rule_a, NFTNL_EXPR_BITWISE_SREG) !=
	    nftnl_expr_get_u32(rule_b, NFTNL_EXPR_BITWISE_SREG))
		print_err(opname, "Expr BITWISE_SREG mismatches");
	if (nftnl_expr_get_u32(rule_a, NFTNL_EXPR_BITWISE_OP) !=
	    nftnl_expr_get_u32(rule_b, NFTNL_EXPR_BITWISE_OP))
		print_err(opname, "Expr BITWISE_OP mismatches");
	if (nftnl_expr_get_u16(rule_a, NFTNL_EXPR_BITWISE_LEN) !=
	    nftnl_expr_get_u16(rule_b, NFTNL_EXPR_BITWISE_LEN))
		print_err(opname, "Expr BITWISE_LEN mismatches");
	nftnl_expr_get(rule_a, NFTNL_EXPR_BITWISE_DATA, &data_a);
	nftnl_expr_get(rule_b, NFTNL_EXPR_BITWISE_DATA, &data_b);
	if (data_a != data_b)
		print_err(opname, "Expr BITWISE_DATA mismatches");
}

static void test_shift(enum nft_bitwise_ops op)
{
	struct nftnl_rule *a, *b;
	struct nftnl_expr *ex;
	struct nlmsghdr *nlh;
	char buf[4096];
	struct nftnl_expr_iter *iter_a, *iter_b;
	struct nftnl_expr *rule_a, *rule_b;
	const char *opname = op == NFT_BITWISE_LSHIFT ? "lshift" : "rshift";

	a = nftnl_rule_alloc();
	b = nftnl_rule_alloc();
	if (a == NULL || b == NULL)
		print_err(opname, "OOM");
	ex = nftnl_expr_alloc("bitwise");
	if (ex == NULL)
		print_err(opname, "OOM");

	nftnl_expr_set_u32(ex, NFTNL_EXPR_BITWISE_SREG, 0x12345678);
	nftnl_expr_set_u32(ex, NFTNL_EXPR_BITWISE_DREG, 0x78123456);
	nftnl_expr_set_u32(ex, NFTNL_EXPR_BITWISE_OP, op);
	nftnl_expr_set_u32(ex, NFTNL_EXPR_BITWISE_LEN, 0x56781234);
	nftnl_expr_set_u32(ex, NFTNL_EXPR_BITWISE_DATA, 13);

	nftnl_rule_add_expr(a, ex);

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_NEWRULE, AF_INET, 0, 1234);
	nftnl_rule_nlmsg_build_payload(nlh, a);

	if (nftnl_rule_nlmsg_parse(nlh, b) < 0)
		print_err(opname, "parsing problems");

	iter_a = nftnl_expr_iter_create(a);
	iter_b = nftnl_expr_iter_create(b);
	if (iter_a == NULL || iter_b == NULL)
		print_err(opname, "OOM");

	rule_a = nftnl_expr_iter_next(iter_a);
	rule_b = nftnl_expr_iter_next(iter_b);
	if (rule_a == NULL || rule_b == NULL)
		print_err(opname, "OOM");

	if (nftnl_expr_iter_next(iter_a) != NULL ||
	    nftnl_expr_iter_next(iter_b) != NULL)
		print_err(opname, "More 1 expr.");

	nftnl_expr_iter_destroy(iter_a);
	nftnl_expr_iter_destroy(iter_b);

	cmp_nftnl_expr_shift(opname, rule_a, rule_b);

	nftnl_rule_free(a);
	nftnl_rule_free(b);
}

static void test_lshift(void)
{
	test_shift(NFT_BITWISE_LSHIFT);
}

static void test_rshift(void)
{
	test_shift(NFT_BITWISE_RSHIFT);
}

static void cmp_nftnl_expr_bool(const char *opname,
				const struct nftnl_expr *rule_a,
				const struct nftnl_expr *rule_b)
{
	if (nftnl_expr_get_u32(rule_a, NFTNL_EXPR_BITWISE_DREG) !=
	    nftnl_expr_get_u32(rule_b, NFTNL_EXPR_BITWISE_DREG))
		print_err(opname, "Expr BITWISE_DREG mismatches");
	if (nftnl_expr_get_u32(rule_a, NFTNL_EXPR_BITWISE_SREG) !=
	    nftnl_expr_get_u32(rule_b, NFTNL_EXPR_BITWISE_SREG))
		print_err(opname, "Expr BITWISE_SREG mismatches");
	if (nftnl_expr_get_u32(rule_a, NFTNL_EXPR_BITWISE_SREG2) !=
	    nftnl_expr_get_u32(rule_b, NFTNL_EXPR_BITWISE_SREG2))
		print_err(opname, "Expr BITWISE_SREG2 mismatches");
	if (nftnl_expr_get_u32(rule_a, NFTNL_EXPR_BITWISE_OP) !=
	    nftnl_expr_get_u32(rule_b, NFTNL_EXPR_BITWISE_OP))
		print_err(opname, "Expr BITWISE_OP mismatches");
	if (nftnl_expr_get_u16(rule_a, NFTNL_EXPR_BITWISE_LEN) !=
	    nftnl_expr_get_u16(rule_b, NFTNL_EXPR_BITWISE_LEN))
		print_err(opname, "Expr BITWISE_LEN mismatches");
}

static void test_bool(enum nft_bitwise_ops op)
{
	struct nftnl_rule *a, *b;
	struct nftnl_expr *ex;
	struct nlmsghdr *nlh;
	char buf[4096];
	struct nftnl_expr_iter *iter_a, *iter_b;
	struct nftnl_expr *rule_a, *rule_b;
	const char *opname =
		op == NFT_BITWISE_AND ? "and" :
		op == NFT_BITWISE_OR  ? "or"  : "xor";

	a = nftnl_rule_alloc();
	b = nftnl_rule_alloc();
	if (a == NULL || b == NULL)
		print_err(opname, "OOM");
	ex = nftnl_expr_alloc("bitwise");
	if (ex == NULL)
		print_err(opname, "OOM");

	nftnl_expr_set_u32(ex, NFTNL_EXPR_BITWISE_SREG, 0x12345678);
	nftnl_expr_set_u32(ex, NFTNL_EXPR_BITWISE_SREG2, 0x90abcdef);
	nftnl_expr_set_u32(ex, NFTNL_EXPR_BITWISE_DREG, 0x78123456);
	nftnl_expr_set_u32(ex, NFTNL_EXPR_BITWISE_OP, op);
	nftnl_expr_set_u32(ex, NFTNL_EXPR_BITWISE_LEN, 0x56781234);

	nftnl_rule_add_expr(a, ex);

	nlh = nftnl_rule_nlmsg_build_hdr(buf, NFT_MSG_NEWRULE, AF_INET, 0, 1234);
	nftnl_rule_nlmsg_build_payload(nlh, a);

	if (nftnl_rule_nlmsg_parse(nlh, b) < 0)
		print_err(opname, "parsing problems");

	iter_a = nftnl_expr_iter_create(a);
	iter_b = nftnl_expr_iter_create(b);
	if (iter_a == NULL || iter_b == NULL)
		print_err(opname, "OOM");

	rule_a = nftnl_expr_iter_next(iter_a);
	rule_b = nftnl_expr_iter_next(iter_b);
	if (rule_a == NULL || rule_b == NULL)
		print_err(opname, "OOM");

	if (nftnl_expr_iter_next(iter_a) != NULL ||
	    nftnl_expr_iter_next(iter_b) != NULL)
		print_err(opname, "More 1 expr.");

	nftnl_expr_iter_destroy(iter_a);
	nftnl_expr_iter_destroy(iter_b);

	cmp_nftnl_expr_bool(opname, rule_a, rule_b);

	nftnl_rule_free(a);
	nftnl_rule_free(b);
}

static void test_and(void)
{
	test_bool(NFT_BITWISE_AND);
}

static void test_or(void)
{
	test_bool(NFT_BITWISE_OR);
}

static void test_xor(void)
{
	test_bool(NFT_BITWISE_XOR);
}

int main(int argc, char *argv[])
{
	test_mask_xor();
	if (!test_ok)
		exit(EXIT_FAILURE);

	test_lshift();
	if (!test_ok)
		exit(EXIT_FAILURE);

	test_rshift();
	if (!test_ok)
		exit(EXIT_FAILURE);

	test_and();
	if (!test_ok)
		exit(EXIT_FAILURE);

	test_or();
	if (!test_ok)
		exit(EXIT_FAILURE);

	test_xor();
	if (!test_ok)
		exit(EXIT_FAILURE);

	printf("%s: \033[32mOK\e[0m\n", argv[0]);
	return EXIT_SUCCESS;
}
