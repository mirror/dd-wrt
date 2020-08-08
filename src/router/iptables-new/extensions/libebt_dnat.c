/* ebt_nat
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 * June, 2002
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <netinet/ether.h>
#include <xtables.h>
#include <linux/netfilter_bridge/ebt_nat.h>
#include "iptables/nft.h"
#include "iptables/nft-bridge.h"

#define NAT_D '1'
#define NAT_D_TARGET '2'
static const struct option brdnat_opts[] =
{
	{ "to-destination", required_argument, 0, NAT_D },
	{ "to-dst"        , required_argument, 0, NAT_D },
	{ "dnat-target"   , required_argument, 0, NAT_D_TARGET },
	{ 0 }
};

static void brdnat_print_help(void)
{
	printf(
	"dnat options:\n"
	" --to-dst address       : MAC address to map destination to\n"
	" --dnat-target target   : ACCEPT, DROP, RETURN or CONTINUE\n");
}

static void brdnat_init(struct xt_entry_target *target)
{
	struct ebt_nat_info *natinfo = (struct ebt_nat_info *)target->data;

	natinfo->target = EBT_ACCEPT;
}

#define OPT_DNAT        0x01
#define OPT_DNAT_TARGET 0x02
static int brdnat_parse(int c, char **argv, int invert, unsigned int *flags,
			 const void *entry, struct xt_entry_target **target)
{
	struct ebt_nat_info *natinfo = (struct ebt_nat_info *)(*target)->data;
	struct ether_addr *addr;

	switch (c) {
	case NAT_D:
		EBT_CHECK_OPTION(flags, OPT_DNAT);
		if (!(addr = ether_aton(optarg)))
			xtables_error(PARAMETER_PROBLEM, "Problem with specified --to-destination mac");
		memcpy(natinfo->mac, addr, ETH_ALEN);
		break;
	case NAT_D_TARGET:
		EBT_CHECK_OPTION(flags, OPT_DNAT_TARGET);
		if (ebt_fill_target(optarg, (unsigned int *)&natinfo->target))
			xtables_error(PARAMETER_PROBLEM, "Illegal --dnat-target target");
		break;
	default:
		return 0;
	}
	return 1;
}

static void brdnat_final_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			      "You must specify proper arguments");
}

static void brdnat_print(const void *ip, const struct xt_entry_target *target, int numeric)
{
	struct ebt_nat_info *natinfo = (struct ebt_nat_info *)target->data;

	printf("--to-dst ");
	xtables_print_mac(natinfo->mac);
	printf(" --dnat-target %s", ebt_target_name(natinfo->target));
}

static const char* brdnat_verdict(int verdict)
{
	switch (verdict) {
	case EBT_ACCEPT: return "accept";
	case EBT_DROP: return "drop";
	case EBT_CONTINUE: return "continue";
	case EBT_RETURN: return "return";
	}

	return "";
}

static int brdnat_xlate(struct xt_xlate *xl,
			 const struct xt_xlate_tg_params *params)
{
	const struct ebt_nat_info *natinfo = (const void*)params->target->data;

	xt_xlate_add(xl, "ether daddr set %s %s ",
		     ether_ntoa((struct ether_addr *)natinfo->mac),
		     brdnat_verdict(natinfo->target));

	return 1;
}

static struct xtables_target brdnat_target =
{
	.name		= "dnat",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size           = XT_ALIGN(sizeof(struct ebt_nat_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_nat_info)),
	.help		= brdnat_print_help,
	.init		= brdnat_init,
	.parse		= brdnat_parse,
	.final_check	= brdnat_final_check,
	.print		= brdnat_print,
	.xlate		= brdnat_xlate,
	.extra_opts	= brdnat_opts,
};

void _init(void)
{
	xtables_register_target(&brdnat_target);
}
