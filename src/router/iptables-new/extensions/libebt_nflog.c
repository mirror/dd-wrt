/* ebt_nflog
 *
 * Authors:
 * Peter Warasin <peter@endian.com>
 *
 *  February, 2008
 *
 * Based on:
 *  ebt_ulog.c, (C) 2004, Bart De Schuymer <bdschuym@pandora.be>
 *  libxt_NFLOG.c
 *
 * Adapted to libxtables for ebtables-compat in 2015 by
 * Arturo Borrero Gonzalez <arturo@debian.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <xtables.h>
#include "iptables/nft.h"
#include "iptables/nft-bridge.h"
#include <linux/netfilter_bridge/ebt_nflog.h>

enum {
	NFLOG_GROUP	= 0x1,
	NFLOG_PREFIX	= 0x2,
	NFLOG_RANGE	= 0x4,
	NFLOG_THRESHOLD	= 0x8,
	NFLOG_NFLOG	= 0x16,
};

static const struct option brnflog_opts[] = {
	{ .name = "nflog-group",     .has_arg = true,  .val = NFLOG_GROUP},
	{ .name = "nflog-prefix",    .has_arg = true,  .val = NFLOG_PREFIX},
	{ .name = "nflog-range",     .has_arg = true,  .val = NFLOG_RANGE},
	{ .name = "nflog-threshold", .has_arg = true,  .val = NFLOG_THRESHOLD},
	{ .name = "nflog",           .has_arg = false, .val = NFLOG_NFLOG},
	XT_GETOPT_TABLEEND,
};

static void brnflog_help(void)
{
	printf("nflog options:\n"
	       "--nflog               : use the default nflog parameters\n"
	       "--nflog-prefix prefix : Prefix string for log message\n"
	       "--nflog-group group   : NETLINK group used for logging\n"
	       "--nflog-range range   : Number of byte to copy\n"
	       "--nflog-threshold     : Message threshold of"
	       "in-kernel queue\n");
}

static void brnflog_init(struct xt_entry_target *t)
{
	struct ebt_nflog_info *info = (struct ebt_nflog_info *)t->data;

	info->prefix[0]	= '\0';
	info->group	= EBT_NFLOG_DEFAULT_GROUP;
	info->threshold = EBT_NFLOG_DEFAULT_THRESHOLD;
}

static int brnflog_parse(int c, char **argv, int invert, unsigned int *flags,
			 const void *entry, struct xt_entry_target **target)
{
	struct ebt_nflog_info *info = (struct ebt_nflog_info *)(*target)->data;
	unsigned int i;

	if (invert)
		xtables_error(PARAMETER_PROBLEM,
			      "The use of '!' makes no sense for the"
			      " nflog watcher");

	switch (c) {
	case NFLOG_PREFIX:
		EBT_CHECK_OPTION(flags, NFLOG_PREFIX);
		if (strlen(optarg) > EBT_NFLOG_PREFIX_SIZE - 1)
			xtables_error(PARAMETER_PROBLEM,
				      "Prefix too long for nflog-prefix");
		strncpy(info->prefix, optarg, EBT_NFLOG_PREFIX_SIZE);
		break;
	case NFLOG_GROUP:
		EBT_CHECK_OPTION(flags, NFLOG_GROUP);
		if (!xtables_strtoui(optarg, NULL, &i, 1, UINT32_MAX))
			xtables_error(PARAMETER_PROBLEM,
				      "--nflog-group must be a number!");
		info->group = i;
		break;
	case NFLOG_RANGE:
		EBT_CHECK_OPTION(flags, NFLOG_RANGE);
		if (!xtables_strtoui(optarg, NULL, &i, 1, UINT32_MAX))
			xtables_error(PARAMETER_PROBLEM,
				      "--nflog-range must be a number!");
		info->len = i;
		break;
	case NFLOG_THRESHOLD:
		EBT_CHECK_OPTION(flags, NFLOG_THRESHOLD);
		if (!xtables_strtoui(optarg, NULL, &i, 1, UINT32_MAX))
			xtables_error(PARAMETER_PROBLEM,
				      "--nflog-threshold must be a number!");
		info->threshold = i;
		break;
	case NFLOG_NFLOG:
		EBT_CHECK_OPTION(flags, NFLOG_NFLOG);
		break;
	default:
		return 0;
	}
	return 1;
}

static void
brnflog_print(const void *ip, const struct xt_entry_target *target,
	      int numeric)
{
	struct ebt_nflog_info *info = (struct ebt_nflog_info *)target->data;

	if (info->prefix[0] != '\0')
		printf("--nflog-prefix \"%s\" ", info->prefix);
	if (info->group)
		printf("--nflog-group %d ", info->group);
	if (info->len)
		printf("--nflog-range %d ", info->len);
	if (info->threshold != EBT_NFLOG_DEFAULT_THRESHOLD)
		printf("--nflog-threshold %d ", info->threshold);
}

static int brnflog_xlate(struct xt_xlate *xl,
			 const struct xt_xlate_tg_params *params)
{
	const struct ebt_nflog_info *info = (void *)params->target->data;

	xt_xlate_add(xl, "log ");
	if (info->prefix[0] != '\0') {
		if (params->escape_quotes)
			xt_xlate_add(xl, "prefix \\\"%s\\\" ", info->prefix);
		else
			xt_xlate_add(xl, "prefix \"%s\" ", info->prefix);
	}

	xt_xlate_add(xl, "group %u ", info->group);

	if (info->len)
		xt_xlate_add(xl, "snaplen %u ", info->len);
	if (info->threshold != EBT_NFLOG_DEFAULT_THRESHOLD)
		xt_xlate_add(xl, "queue-threshold %u ", info->threshold);

	return 1;
}

static struct xtables_target brnflog_watcher = {
	.name		= "nflog",
	.revision	= 0,
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_nflog_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_nflog_info)),
	.init		= brnflog_init,
	.help		= brnflog_help,
	.parse		= brnflog_parse,
	.print		= brnflog_print,
	.xlate		= brnflog_xlate,
	.extra_opts	= brnflog_opts,
};

void _init(void)
{
	xtables_register_target(&brnflog_watcher);
}
