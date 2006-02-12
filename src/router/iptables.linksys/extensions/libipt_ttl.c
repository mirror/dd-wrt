/* Shared library add-on to iptables to add TTL matching support 
 * (C) 2000 by Harald Welte <laforge@gnumonks.org>
 *
 * $Id: libipt_ttl.c,v 1.1.1.4 2003/10/14 08:09:41 sparq Exp $
 *
 * This program is released under the terms of GNU GPL */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <iptables.h>

#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_ttl.h>

static void help(void) 
{
	printf(
"TTL match v%s options:\n"
"  --ttl-eq value	Match time to live value\n"
"  --ttl-lt value	Match TTL < value\n"
"  --ttl-gt value	Match TTL > value\n"
, IPTABLES_VERSION);
}

static void init(struct ipt_entry_match *m, unsigned int *nfcache)
{
	/* caching not yet implemented */
	*nfcache |= NFC_UNKNOWN;
}

static int parse(int c, char **argv, int invert, unsigned int *flags,
		const struct ipt_entry *entry, unsigned int *nfcache,
		struct ipt_entry_match **match)
{
	struct ipt_ttl_info *info = (struct ipt_ttl_info *) (*match)->data;
	u_int8_t value;

	check_inverse(optarg, &invert, &optind, 0);
	value = atoi(argv[optind-1]);

	if (*flags) 
		exit_error(PARAMETER_PROBLEM, 
				"Can't specify TTL option twice");

	if (!optarg)
		exit_error(PARAMETER_PROBLEM,
				"ttl: You must specify a value");
	switch (c) {
		case '2':
			if (invert)
				info->mode = IPT_TTL_NE;
			else
				info->mode = IPT_TTL_EQ;

			/* is 0 allowed? */
			info->ttl = value;
			*flags = 1;

			break;
		case '3':
			if (invert) 
				exit_error(PARAMETER_PROBLEM,
						"ttl: unexpected `!'");

			info->mode = IPT_TTL_LT;
			info->ttl = value;
			*flags = 1;

			break;
		case '4':
			if (invert)
				exit_error(PARAMETER_PROBLEM,
						"ttl: unexpected `!'");

			info->mode = IPT_TTL_GT;
			info->ttl = value;
			*flags = 1;

			break;
		default:
			return 0;

	}

	return 1;
}

static void final_check(unsigned int flags)
{
	if (!flags) 
		exit_error(PARAMETER_PROBLEM,
			"TTL match: You must specify one of "
			"`--ttl-eq', `--ttl-lt', `--ttl-gt");
}

static void print(const struct ipt_ip *ip, 
		const struct ipt_entry_match *match,
		int numeric)
{
	const struct ipt_ttl_info *info = 
		(struct ipt_ttl_info *) match->data;

	printf("TTL match ");
	switch (info->mode) {
		case IPT_TTL_EQ:
			printf("TTL == ");
			break;
		case IPT_TTL_NE:
			printf("TTL != ");
			break;
		case IPT_TTL_LT:
			printf("TTL < ");
			break;
		case IPT_TTL_GT:
			printf("TTL > ");
			break;
	}
	printf("%u ", info->ttl);
}

static void save(const struct ipt_ip *ip, 
		const struct ipt_entry_match *match)
{
	const struct ipt_ttl_info *info =
		(struct ipt_ttl_info *) match->data;

	switch (info->mode) {
		case IPT_TTL_EQ:
			printf("--ttl-eq ");
			break;
		case IPT_TTL_NE:
			printf("! --ttl-eq ");
			break;
		case IPT_TTL_LT:
			printf("--ttl-lt ");
			break;
		case IPT_TTL_GT:
			printf("--ttl-gt ");
			break;
		default:
			/* error */
			break;
	}
	printf("%u ", info->ttl);
}

static struct option opts[] = {
	{ "ttl", 1, 0, '2' },
	{ "ttl-eq", 1, 0, '2'},
	{ "ttl-lt", 1, 0, '3'},
	{ "ttl-gt", 1, 0, '4'},
	{ 0 }
};

static
struct iptables_match ttl = {
	NULL,
	"ttl",
	IPTABLES_VERSION,
	IPT_ALIGN(sizeof(struct ipt_ttl_info)),
	IPT_ALIGN(sizeof(struct ipt_ttl_info)),
	&help,
	&init,
	&parse,
	&final_check,
	&print,
	&save,
	opts
};


void _init(void) 
{
	register_match(&ttl);
}
