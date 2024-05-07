/* Shared library add-on to iptables to add NFMARK matching support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <ip6tables.h>
/* For 64bit kernel / 32bit userspace */
#include "../include/linux/netfilter_ipv4/ipt_mark.h"

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"MARK match v%s options:\n"
"[!] --mark value[/mask]         Match nfmark value with optional mask\n"
"\n",
IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "mark", 1, 0, '1' },
	{0}
};

#define ipt_entry ip6t_entry
#define ipt_entry_match ip6t_entry_match
#define ipt_ip ip6t_ip6
#define iptables_match ip6tables_match
#define register_match register_match6
#define IPT_ALIGN IP6T_ALIGN

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      unsigned int *nfcache,
      struct ipt_entry_match **match)
{
	struct ipt_mark_info *markinfo = (struct ipt_mark_info *)(*match)->data;

	switch (c) {
		char *end;
	case '1':
		check_inverse(optarg, &invert, &optind, 0);
#ifdef KERNEL_64_USERSPACE_32
		markinfo->mark = strtoull(optarg, &end, 0);
		if (*end == '/') {
			markinfo->mask = strtoull(end+1, &end, 0);
		} else
			markinfo->mask = 0xffffffffffffffffULL;
#else
		markinfo->mark = strtoul(optarg, &end, 0);
		if (*end == '/') {
			markinfo->mask = strtoul(end+1, &end, 0);
		} else
			markinfo->mask = 0xffffffff;
#endif
		if (*end != '\0' || end == optarg)
			exit_error(PARAMETER_PROBLEM, "Bad MARK value `%s'", optarg);
		if (invert)
			markinfo->invert = 1;
		*flags = 1;
		break;

	default:
		return 0;
	}
	return 1;
}

static int
parse_v1(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      unsigned int *nfcache,
      struct ipt_entry_match **match)
{
	struct xt_mark_mtinfo1 *markinfo = (struct xt_mark_mtinfo1 *)(*match)->data;

	switch (c) {
		char *end;
	case '1':
		check_inverse(optarg, &invert, &optind, 0);
#ifdef KERNEL_64_USERSPACE_32
		markinfo->mark = strtoull(optarg, &end, 0);
		if (*end == '/') {
			markinfo->mask = strtoull(end+1, &end, 0);
		} else
			markinfo->mask = 0xffffffffffffffffULL;
#else
		markinfo->mark = strtoul(optarg, &end, 0);
		if (*end == '/') {
			markinfo->mask = strtoul(end+1, &end, 0);
		} else
			markinfo->mask = 0xffffffff;
#endif
		if (*end != '\0' || end == optarg)
			exit_error(PARAMETER_PROBLEM, "Bad MARK value `%s'", optarg);
		if (invert)
			markinfo->invert = 1;
		*flags = 1;
		break;

	default:
		return 0;
	}
	return 1;
}

#ifdef KERNEL_64_USERSPACE_32
static void
print_mark(unsigned long long mark, unsigned long long mask, int numeric)
{
	if(mask != 0xffffffffffffffffULL)
		printf("0x%llx/0x%llx ", mark, mask);
	else
		printf("0x%llx ", mark);
}
#else
static void
print_mark(unsigned long mark, unsigned long mask, int numeric)
{
	if(mask != 0xffffffff)
		printf("0x%lx/0x%lx ", mark, mask);
	else
		printf("0x%lx ", mark);
}
#endif

/* Final check; must have specified --mark. */
static void
final_check(unsigned int flags)
{
	if (!flags)
		exit_error(PARAMETER_PROBLEM,
			   "MARK match: You must specify `--mark'");
}

/* Prints out the matchinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_match *match,
      int numeric)
{
	struct ipt_mark_info *info = (struct ipt_mark_info *)match->data;

	printf("MARK match ");

	if (info->invert)
		printf("!");
	
	print_mark(info->mark, info->mask, numeric);
}

static void
print_v1(const struct ipt_ip *ip,
      const struct ipt_entry_match *match,
      int numeric)
{
	const struct xt_mark_mtinfo1 *info = (const struct xt_mark_mtinfo1 *)match->data;

	printf("mark match ");

	if (info->invert)
		printf("!");
	
	print_mark(info->mark, info->mask, numeric);
}

/* Saves the union ipt_matchinfo in parsable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
{
	struct ipt_mark_info *info = (struct ipt_mark_info *)match->data;

	if (info->invert)
		printf("! ");
	
	printf("--mark ");
	print_mark(info->mark, info->mask, 0);
}

static void
save_v1(const struct ipt_ip *ip, const struct ipt_entry_match *match)
{
	const struct xt_mark_mtinfo1 *info = (const struct xt_mark_mtinfo1 *)match->data;

	if (info->invert)
		printf(" !");

	printf(" --mark");
	print_mark(info->mark, info->mask, 0);
}



static struct iptables_match mark_v0 = { 
	.next		= NULL,
	.name		= "mark",
	.revision	= 0,
	.version	= IPTABLES_VERSION,
	.size		= IPT_ALIGN(sizeof(struct ipt_mark_info)),
	.userspacesize	= IPT_ALIGN(sizeof(struct ipt_mark_info)),
//	.help		= &help,
	.parse		= &parse,
	.final_check	= &final_check,
	.print		= &print,
	.save		= &save,
	.extra_opts	= opts
};

static struct iptables_match mark_v1 = { 
	.next		= NULL,
	.name		= "mark",
	.revision	= 1,
	.version	= IPTABLES_VERSION,
	.size		= IPT_ALIGN(sizeof(struct xt_mark_mtinfo1)),
	.userspacesize	= IPT_ALIGN(sizeof(struct xt_mark_mtinfo1)),
//	.help		= &help,
	.parse		= &parse_v1,
	.final_check	= &final_check,
	.print		= &print_v1,
	.save		= &save_v1,
	.extra_opts	= opts
};

void _init(void)
{
	register_match(&mark_v0);
	register_match(&mark_v1);
}
