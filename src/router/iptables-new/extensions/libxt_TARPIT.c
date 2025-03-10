/*
 *	"TARPIT" target extension to iptables
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License; either
 *	version 2 of the License, or any later version, as published by the
 *	Free Software Foundation.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <xtables.h>
#include <linux/netfilter/x_tables.h>
#include "xt_TARPIT.h"
#include "compat_user.h"

enum {
	F_TARPIT   = 1 << 0,
	F_HONEYPOT = 1 << 1,
	F_RESET    = 1 << 2,
};

static const struct option tarpit_tg_opts[] = {
	{.name = "tarpit",   .has_arg = false, .val = 't'},
	{.name = "honeypot", .has_arg = false, .val = 'h'},
	{.name = "reset",    .has_arg = false, .val = 'r'},
	{NULL},
};

static void tarpit_tg_help(void)
{
	printf(
		"TARPIT target options:\n"
		"  --tarpit      Enable classic 0-window tarpit (default)\n"
		"  --honeypot    Enable honeypot option\n"
		"  --reset       Enable inline resets\n");
}

static int tarpit_tg_parse(int c, char **argv, int invert, unsigned int *flags,
                           const void *entry, struct xt_entry_target **target)
{
	struct xt_tarpit_tginfo *info = (void *)(*target)->data;

	switch (c) {
	case 't':
		info->variant = XTTARPIT_TARPIT;
		*flags |= F_TARPIT;
		return true;
	case 'h':
		info->variant = XTTARPIT_HONEYPOT;
		*flags |= F_HONEYPOT;
		return true;
	case 'r':
		info->variant = XTTARPIT_RESET;
		*flags |= F_RESET;
		return true;
	}
	return false;
}

static void tarpit_tg_check(unsigned int flags)
{
	if (flags == (F_TARPIT | F_HONEYPOT | F_RESET))
		xtables_error(PARAMETER_PROBLEM,
			"TARPIT: only one action can be used at a time");
}

static void tarpit_tg_save(const void *ip,
    const struct xt_entry_target *target)
{
	const struct xt_tarpit_tginfo *info = (const void *)target->data;

	switch (info->variant) {
	case XTTARPIT_TARPIT:
		printf(" --tarpit ");
		break;
	case XTTARPIT_HONEYPOT:
		printf(" --honeypot ");
		break;
	case XTTARPIT_RESET:
		printf(" --reset ");
		break;
	}
}

static void tarpit_tg_print(const void *ip,
    const struct xt_entry_target *target, int numeric)
{
	printf(" -j TARPIT");
	tarpit_tg_save(ip, target);
}

static struct xtables_target tarpit_tg_reg = {
	.version       = XTABLES_VERSION,
	.name          = "TARPIT",
	.family        = NFPROTO_UNSPEC,
	.size          = XT_ALIGN(sizeof(struct xt_tarpit_tginfo)),
	.userspacesize = XT_ALIGN(sizeof(struct xt_tarpit_tginfo)),
//	.help          = tarpit_tg_help,
	.parse         = tarpit_tg_parse,
	.final_check   = tarpit_tg_check,
	.print         = tarpit_tg_print,
	.save          = tarpit_tg_save,
	.extra_opts    = tarpit_tg_opts,
};

void _init(void)
{
	xtables_register_target(&tarpit_tg_reg);
}
