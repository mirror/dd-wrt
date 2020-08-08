/* ebt_vlan
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 * Nick Fedchik <nick@fedchik.org.ua>
 * June, 2002
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <xtables.h>
#include <netinet/if_ether.h>
#include <linux/netfilter_bridge/ebt_vlan.h>
#include <linux/if_ether.h>
#include "iptables/nft.h"
#include "iptables/nft-bridge.h"

#define NAME_VLAN_ID    "id"
#define NAME_VLAN_PRIO  "prio"
#define NAME_VLAN_ENCAP "encap"

#define VLAN_ID    '1'
#define VLAN_PRIO  '2'
#define VLAN_ENCAP '3'

static const struct option brvlan_opts[] = {
	{"vlan-id"   , required_argument, NULL, VLAN_ID},
	{"vlan-prio" , required_argument, NULL, VLAN_PRIO},
	{"vlan-encap", required_argument, NULL, VLAN_ENCAP},
	XT_GETOPT_TABLEEND,
};

/*
 * option inverse flags definition
 */
#define OPT_VLAN_ID     0x01
#define OPT_VLAN_PRIO   0x02
#define OPT_VLAN_ENCAP  0x04
#define OPT_VLAN_FLAGS	(OPT_VLAN_ID | OPT_VLAN_PRIO | OPT_VLAN_ENCAP)

static void brvlan_print_help(void)
{
	printf(
"vlan options:\n"
"--vlan-id [!] id       : vlan-tagged frame identifier, 0,1-4096 (integer)\n"
"--vlan-prio [!] prio   : Priority-tagged frame's user priority, 0-7 (integer)\n"
"--vlan-encap [!] encap : Encapsulated frame protocol (hexadecimal or name)\n");
}

static int
brvlan_parse(int c, char **argv, int invert, unsigned int *flags,
	       const void *entry, struct xt_entry_match **match)
{
	struct ebt_vlan_info *vlaninfo = (struct ebt_vlan_info *) (*match)->data;
	struct xt_ethertypeent *ethent;
	char *end;
	struct ebt_vlan_info local;

	switch (c) {
	case VLAN_ID:
		EBT_CHECK_OPTION(flags, OPT_VLAN_ID);
		if (invert)
			vlaninfo->invflags |= EBT_VLAN_ID;
		local.id = strtoul(optarg, &end, 10);
		if (local.id > 4094 || *end != '\0')
			xtables_error(PARAMETER_PROBLEM, "Invalid --vlan-id range ('%s')", optarg);
		vlaninfo->id = local.id;
		vlaninfo->bitmask |= EBT_VLAN_ID;
		break;
	case VLAN_PRIO:
		EBT_CHECK_OPTION(flags, OPT_VLAN_PRIO);
		if (invert)
			vlaninfo->invflags |= EBT_VLAN_PRIO;
		local.prio = strtoul(optarg, &end, 10);
		if (local.prio >= 8 || *end != '\0')
			xtables_error(PARAMETER_PROBLEM, "Invalid --vlan-prio range ('%s')", optarg);
		vlaninfo->prio = local.prio;
		vlaninfo->bitmask |= EBT_VLAN_PRIO;
		break;
	case VLAN_ENCAP:
		EBT_CHECK_OPTION(flags, OPT_VLAN_ENCAP);
		if (invert)
			vlaninfo->invflags |= EBT_VLAN_ENCAP;
		local.encap = strtoul(optarg, &end, 16);
		if (*end != '\0') {
			ethent = xtables_getethertypebyname(optarg);
			if (ethent == NULL)
				xtables_error(PARAMETER_PROBLEM, "Unknown --vlan-encap value ('%s')", optarg);
			local.encap = ethent->e_ethertype;
		}
		if (local.encap < ETH_ZLEN)
			xtables_error(PARAMETER_PROBLEM, "Invalid --vlan-encap range ('%s')", optarg);
		vlaninfo->encap = htons(local.encap);
		vlaninfo->bitmask |= EBT_VLAN_ENCAP;
		break;
	default:
		return 0;

	}
	return 1;
}

static void brvlan_print(const void *ip, const struct xt_entry_match *match,
			 int numeric)
{
	struct ebt_vlan_info *vlaninfo = (struct ebt_vlan_info *) match->data;

	if (vlaninfo->bitmask & EBT_VLAN_ID) {
		printf("--vlan-id %s%d ", (vlaninfo->invflags & EBT_VLAN_ID) ? "! " : "", vlaninfo->id);
	}
	if (vlaninfo->bitmask & EBT_VLAN_PRIO) {
		printf("--vlan-prio %s%d ", (vlaninfo->invflags & EBT_VLAN_PRIO) ? "! " : "", vlaninfo->prio);
	}
	if (vlaninfo->bitmask & EBT_VLAN_ENCAP) {
		printf("--vlan-encap %s", (vlaninfo->invflags & EBT_VLAN_ENCAP) ? "! " : "");
		printf("%4.4X ", ntohs(vlaninfo->encap));
	}
}

static int brvlan_xlate(struct xt_xlate *xl,
			const struct xt_xlate_mt_params *params)
{
	const struct ebt_vlan_info *vlaninfo = (const void*)params->match->data;

	if (vlaninfo->bitmask & EBT_VLAN_ID)
		xt_xlate_add(xl, "vlan id %s%d ", (vlaninfo->invflags & EBT_VLAN_ID) ? "!= " : "", vlaninfo->id);

	if (vlaninfo->bitmask & EBT_VLAN_PRIO)
		xt_xlate_add(xl, "vlan pcp %s%d ", (vlaninfo->invflags & EBT_VLAN_PRIO) ? "!= " : "", vlaninfo->prio);

	if (vlaninfo->bitmask & EBT_VLAN_ENCAP)
		xt_xlate_add(xl, "vlan type %s0x%4.4x ", (vlaninfo->invflags & EBT_VLAN_ENCAP) ? "!= " : "", ntohs(vlaninfo->encap));

	return 1;
}

static struct xtables_match brvlan_match = {
	.name		= "vlan",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_vlan_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_vlan_info)),
	.help		= brvlan_print_help,
	.parse		= brvlan_parse,
	.print		= brvlan_print,
	.xlate		= brvlan_xlate,
	.extra_opts	= brvlan_opts,
};

void _init(void)
{
	xtables_register_match(&brvlan_match);
}
