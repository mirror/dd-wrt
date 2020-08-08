/* ebt_mark_m
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 * July, 2002
 *
 * Adapted by Arturo Borrero Gonzalez <arturo@debian.org>
 * to use libxtables for ebtables-compat in 2015.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <xtables.h>
#include <linux/netfilter_bridge/ebt_mark_m.h>

#define MARK '1'

static const struct option brmark_m_opts[] = {
	{ .name = "mark",	.has_arg = true, .val = MARK },
	XT_GETOPT_TABLEEND,
};

static void brmark_m_print_help(void)
{
	printf(
"mark option:\n"
"--mark    [!] [value][/mask]: Match nfmask value (see man page)\n");
}

static void brmark_m_init(struct xt_entry_match *match)
{
	struct ebt_mark_m_info *info = (struct ebt_mark_m_info *)match->data;

	info->mark = 0;
	info->mask = 0;
	info->invert = 0;
	info->bitmask = 0;
}

#define OPT_MARK 0x01
static int
brmark_m_parse(int c, char **argv, int invert, unsigned int *flags,
	       const void *entry, struct xt_entry_match **match)
{
	struct ebt_mark_m_info *info = (struct ebt_mark_m_info *)
				       (*match)->data;
	char *end;

	switch (c) {
	case MARK:
		if (invert)
			info->invert = 1;
		info->mark = strtoul(optarg, &end, 0);
		info->bitmask = EBT_MARK_AND;
		if (*end == '/') {
			if (end == optarg)
				info->bitmask = EBT_MARK_OR;
			info->mask = strtoul(end+1, &end, 0);
		} else {
			info->mask = 0xffffffff;
		}
		if (*end != '\0' || end == optarg)
			xtables_error(PARAMETER_PROBLEM, "Bad mark value '%s'",
				      optarg);
		break;
	default:
		return 0;
	}

	*flags |= info->bitmask;
	return 1;
}

static void brmark_m_final_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			      "You must specify proper arguments");
}

static void brmark_m_print(const void *ip, const struct xt_entry_match *match,
			   int numeric)
{
	struct ebt_mark_m_info *info = (struct ebt_mark_m_info *)match->data;

	printf("--mark ");
	if (info->invert)
		printf("! ");
	if (info->bitmask == EBT_MARK_OR)
		printf("/0x%lx ", info->mask);
	else if (info->mask != 0xffffffff)
		printf("0x%lx/0x%lx ", info->mark, info->mask);
	else
		printf("0x%lx ", info->mark);
}

static int brmark_m_xlate(struct xt_xlate *xl,
			  const struct xt_xlate_mt_params *params)
{
	const struct ebt_mark_m_info *info = (const void*)params->match->data;
	enum xt_op op = XT_OP_EQ;

	if (info->invert)
		op = XT_OP_NEQ;

	xt_xlate_add(xl, "meta mark ");

	if (info->bitmask == EBT_MARK_OR) {
		xt_xlate_add(xl, "and 0x%x %s0 ", (uint32_t)info->mask,
			     info->invert ? "" : "!= ");
	} else if (info->mask != 0xffffffffU) {
		xt_xlate_add(xl, "and 0x%x %s0x%x ", (uint32_t)info->mask,
			   op == XT_OP_EQ ? "" : "!= ", (uint32_t)info->mark);
	} else {
		xt_xlate_add(xl, "%s0x%x ",
			   op == XT_OP_EQ ? "" : "!= ", (uint32_t)info->mark);
	}

	return 1;
}
static struct xtables_match brmark_m_match = {
	.name		= "mark_m",
	.revision	= 0,
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_mark_m_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_mark_m_info)),
	.init		= brmark_m_init,
	.help		= brmark_m_print_help,
	.parse		= brmark_m_parse,
	.final_check	= brmark_m_final_check,
	.print		= brmark_m_print,
	.xlate		= brmark_m_xlate,
	.extra_opts	= brmark_m_opts,
};

void _init(void)
{
	xtables_register_match(&brmark_m_match);
}
