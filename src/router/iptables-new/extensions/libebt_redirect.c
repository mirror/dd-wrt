/* ebt_redirect
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 * April, 2002
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <xtables.h>
#include <linux/netfilter_bridge/ebt_redirect.h>
#include "iptables/nft.h"
#include "iptables/nft-bridge.h"

#define REDIRECT_TARGET '1'
static const struct option brredir_opts[] =
{
	{ "redirect-target", required_argument, 0, REDIRECT_TARGET },
	{ 0 }
};

static void brredir_print_help(void)
{
	printf(
	"redirect option:\n"
	" --redirect-target target   : ACCEPT, DROP, RETURN or CONTINUE\n");
}

static void brredir_init(struct xt_entry_target *target)
{
	struct ebt_redirect_info *redirectinfo =
	   (struct ebt_redirect_info *)target->data;

	redirectinfo->target = EBT_ACCEPT;
}

#define OPT_REDIRECT_TARGET  0x01
static int brredir_parse(int c, char **argv, int invert, unsigned int *flags,
			 const void *entry, struct xt_entry_target **target)
{
	struct ebt_redirect_info *redirectinfo =
	   (struct ebt_redirect_info *)(*target)->data;

	switch (c) {
	case REDIRECT_TARGET:
		EBT_CHECK_OPTION(flags, OPT_REDIRECT_TARGET);
		if (ebt_fill_target(optarg, (unsigned int *)&redirectinfo->target))
			xtables_error(PARAMETER_PROBLEM, "Illegal --redirect-target target");
		break;
	default:
		return 0;
	}
	return 1;
}

static void brredir_print(const void *ip, const struct xt_entry_target *target, int numeric)
{
	struct ebt_redirect_info *redirectinfo =
	   (struct ebt_redirect_info *)target->data;

	if (redirectinfo->target == EBT_ACCEPT)
		return;
	printf("--redirect-target %s", ebt_target_name(redirectinfo->target));
}

static const char* brredir_verdict(int verdict)
{
	switch (verdict) {
	case EBT_ACCEPT: return "accept";
	case EBT_DROP: return "drop";
	case EBT_CONTINUE: return "continue";
	case EBT_RETURN: return "return";
	}

	return "";
}

static int brredir_xlate(struct xt_xlate *xl,
			 const struct xt_xlate_tg_params *params)
{
	const struct ebt_redirect_info *red = (const void*)params->target->data;

	xt_xlate_add(xl, "meta set pkttype host");
	if (red->target != EBT_ACCEPT)
		xt_xlate_add(xl, " %s ", brredir_verdict(red->target));
	return 0;
}

static struct xtables_target brredirect_target = {
	.name		= "redirect",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_redirect_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_redirect_info)),
	.help		= brredir_print_help,
	.init		= brredir_init,
	.parse		= brredir_parse,
	.print		= brredir_print,
	.xlate		= brredir_xlate,
	.extra_opts	= brredir_opts,
};

void _init(void)
{
	xtables_register_target(&brredirect_target);
}
