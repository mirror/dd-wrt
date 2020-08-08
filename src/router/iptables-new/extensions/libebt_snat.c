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

#define NAT_S '1'
#define NAT_S_TARGET '2'
#define NAT_S_ARP '3'
static const struct option brsnat_opts[] =
{
	{ "to-source"     , required_argument, 0, NAT_S },
	{ "to-src"        , required_argument, 0, NAT_S },
	{ "snat-target"   , required_argument, 0, NAT_S_TARGET },
	{ "snat-arp"      ,       no_argument, 0, NAT_S_ARP },
	{ 0 }
};

static void brsnat_print_help(void)
{
	printf(
	"snat options:\n"
	" --to-src address       : MAC address to map source to\n"
	" --snat-target target   : ACCEPT, DROP, RETURN or CONTINUE\n"
	" --snat-arp             : also change src address in arp msg\n");
}

static void brsnat_init(struct xt_entry_target *target)
{
	struct ebt_nat_info *natinfo = (struct ebt_nat_info *)target->data;

	natinfo->target = EBT_ACCEPT;
}

#define OPT_SNAT         0x01
#define OPT_SNAT_TARGET  0x02
#define OPT_SNAT_ARP     0x04
static int brsnat_parse(int c, char **argv, int invert, unsigned int *flags,
			 const void *entry, struct xt_entry_target **target)
{
	struct ebt_nat_info *natinfo = (struct ebt_nat_info *)(*target)->data;
	struct ether_addr *addr;

	switch (c) {
	case NAT_S:
		EBT_CHECK_OPTION(flags, OPT_SNAT);
		if (!(addr = ether_aton(optarg)))
			xtables_error(PARAMETER_PROBLEM, "Problem with specified --to-source mac");
		memcpy(natinfo->mac, addr, ETH_ALEN);
		break;
	case NAT_S_TARGET:
		{ unsigned int tmp;
		EBT_CHECK_OPTION(flags, OPT_SNAT_TARGET);
		if (ebt_fill_target(optarg, &tmp))
			xtables_error(PARAMETER_PROBLEM, "Illegal --snat-target target");
		natinfo->target = (natinfo->target & ~EBT_VERDICT_BITS) | (tmp & EBT_VERDICT_BITS);
		}
		break;
	case NAT_S_ARP:
		EBT_CHECK_OPTION(flags, OPT_SNAT_ARP);
		natinfo->target ^= NAT_ARP_BIT;
		break;
	default:
		return 0;
	}
	return 1;
}

static void brsnat_final_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			      "You must specify proper arguments");
}

static void brsnat_print(const void *ip, const struct xt_entry_target *target, int numeric)
{
	struct ebt_nat_info *natinfo = (struct ebt_nat_info *)target->data;

	printf("--to-src ");
	xtables_print_mac(natinfo->mac);
	if (!(natinfo->target&NAT_ARP_BIT))
		printf(" --snat-arp");
	printf(" --snat-target %s", ebt_target_name((natinfo->target|~EBT_VERDICT_BITS)));
}

static const char* brsnat_verdict(int verdict)
{
	switch (verdict) {
	case EBT_ACCEPT: return "accept";
	case EBT_DROP: return "drop";
	case EBT_CONTINUE: return "continue";
	case EBT_RETURN: return "return";
	}

	return "";
}

static int brsnat_xlate(struct xt_xlate *xl,
			 const struct xt_xlate_tg_params *params)
{
	const struct ebt_nat_info *natinfo = (const void*)params->target->data;

	xt_xlate_add(xl, "ether saddr set %s ",
		     ether_ntoa((struct ether_addr *)natinfo->mac));

	/* NAT_ARP_BIT set -> no arp mangling, not set -> arp mangling (yes, its inverted) */
	if (!(natinfo->target&NAT_ARP_BIT))
		return 0;

	xt_xlate_add(xl, "%s ", brsnat_verdict(natinfo->target | ~EBT_VERDICT_BITS));
	return 1;
}

static struct xtables_target brsnat_target =
{
	.name		= "snat",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size           = XT_ALIGN(sizeof(struct ebt_nat_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_nat_info)),
	.help		= brsnat_print_help,
	.init		= brsnat_init,
	.parse		= brsnat_parse,
	.final_check	= brsnat_final_check,
	.print		= brsnat_print,
	.xlate		= brsnat_xlate,
	.extra_opts	= brsnat_opts,
};

void _init(void)
{
	xtables_register_target(&brsnat_target);
}
