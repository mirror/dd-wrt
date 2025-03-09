/*
 *	LSCAN match extension for iptables
 *	Copyright © Jan Engelhardt, 2006 - 2009
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License; either
 *	version 2 of the License, or any later version, as published by the
 *	Free Software Foundation.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <xtables.h>
#include <linux/netfilter/x_tables.h>
#include "xt_lscan.h"
#include "compat_user.h"

static const struct option lscan_mt_opts[] = {
	{.name = "stealth", .has_arg = false, .val = 'x'},
	{.name = "synscan", .has_arg = false, .val = 's'},
	{.name = "cnscan",  .has_arg = false, .val = 'c'},
	{.name = "grscan",  .has_arg = false, .val = 'g'},
	{.name = "mirai",   .has_arg = false, .val = 'm'},
	{NULL},
};

static void lscan_mt_help(void)
{
	printf(
		"lscan match options:\n"
		"(Combining them will make them match by OR-logic)\n"
		"  --stealth    Match TCP Stealth packets\n"
		"  --synscan    Match TCP SYN scans\n"
		"  --cnscan     Match TCP Connect scans\n"
		"  --grscan     Match Banner Grabbing scans\n"
		"  --mirai      Match TCP scan with ISN = dest. IP\n");
}

static int lscan_mt_parse(int c, char **argv, int invert,
    unsigned int *flags, const void *entry, struct xt_entry_match **match)
{
	struct xt_lscan_mtinfo *info = (void *)((*match)->data);

	switch (c) {
	case 'c':
		info->match_fl3 |= LSCAN_FL3_CN;
		return true;
	case 'g':
		info->match_fl4 |= LSCAN_FL4_GR;
		return true;
	case 'm':
		info->match_fl1 |= LSCAN_FL1_MIRAI;
		return true;
	case 's':
		info->match_fl2 |= LSCAN_FL2_SYN;
		return true;
	case 'x':
		info->match_fl1 |= LSCAN_FL1_STEALTH;
		return true;
	}
	return false;
}

static void lscan_mt_check(unsigned int flags)
{
}

static void lscan_mt_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_lscan_mtinfo *info = (const void *)(match->data);

	if (info->match_fl1 & LSCAN_FL1_STEALTH)
		printf(" --stealth ");
	if (info->match_fl2 & LSCAN_FL2_SYN)
		printf(" --synscan ");
	if (info->match_fl3 & LSCAN_FL3_CN)
		printf(" --cnscan ");
	if (info->match_fl4 & LSCAN_FL4_GR)
		printf(" --grscan ");
	if (info->match_fl1 & LSCAN_FL1_MIRAI)
		printf(" --mirai ");
}

static void lscan_mt_print(const void *ip,
    const struct xt_entry_match *match, int numeric)
{
	printf(" -m lscan");
	lscan_mt_save(ip, match);
}

static struct xtables_match lscan_mt_reg = {
	.version       = XTABLES_VERSION,
	.name          = "lscan",
	.revision      = 0,
	.family        = NFPROTO_IPV4,
	.size          = XT_ALIGN(sizeof(struct xt_lscan_mtinfo)),
	.userspacesize = XT_ALIGN(sizeof(struct xt_lscan_mtinfo)),
//	.help          = lscan_mt_help,
	.parse         = lscan_mt_parse,
	.final_check   = lscan_mt_check,
	.print         = lscan_mt_print,
	.save          = lscan_mt_save,
	.extra_opts    = lscan_mt_opts,
};

void _init(void)
{
	xtables_register_match(&lscan_mt_reg);
}
