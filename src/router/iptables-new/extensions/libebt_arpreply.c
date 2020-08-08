/* ebt_arpreply
 *
 * Authors:
 * Grzegorz Borowiak <grzes@gnu.univ.gda.pl>
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 *  August, 2003
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <xtables.h>
#include <netinet/ether.h>
#include <linux/netfilter_bridge/ebt_arpreply.h>
#include "iptables/nft.h"
#include "iptables/nft-bridge.h"

#define OPT_REPLY_MAC     0x01
#define OPT_REPLY_TARGET  0x02

#define REPLY_MAC '1'
#define REPLY_TARGET '2'
static const struct option brarpreply_opts[] = {
	{ "arpreply-mac" ,    required_argument, 0, REPLY_MAC    },
	{ "arpreply-target" , required_argument, 0, REPLY_TARGET },
	XT_GETOPT_TABLEEND,
};

static void brarpreply_print_help(void)
{
	printf(
	"arpreply target options:\n"
	" --arpreply-mac address           : source MAC of generated reply\n"
	" --arpreply-target target         : ACCEPT, DROP, RETURN or CONTINUE\n"
	"                                    (standard target is DROP)\n");
}

static void brarpreply_init(struct xt_entry_target *target)
{
	struct ebt_arpreply_info *replyinfo = (void *)target->data;

	replyinfo->target = EBT_DROP;
}

static int
brarpreply_parse(int c, char **argv, int invert, unsigned int *flags,
	    const void *entry, struct xt_entry_target **tg)

{
	struct ebt_arpreply_info *replyinfo = (void *)(*tg)->data;
	struct ether_addr *addr;

	switch (c) {
	case REPLY_MAC:
		EBT_CHECK_OPTION(flags, OPT_REPLY_MAC);
		if (!(addr = ether_aton(optarg)))
			xtables_error(PARAMETER_PROBLEM, "Problem with specified --arpreply-mac mac");
		memcpy(replyinfo->mac, addr, ETH_ALEN);
		break;
	case REPLY_TARGET:
		EBT_CHECK_OPTION(flags, OPT_REPLY_TARGET);
		if (ebt_fill_target(optarg, (unsigned int *)&replyinfo->target))
			xtables_error(PARAMETER_PROBLEM, "Illegal --arpreply-target target");
		break;

	default:
		return 0;
	}
	return 1;
}

static void brarpreply_print(const void *ip, const struct xt_entry_target *t, int numeric)
{
	struct ebt_arpreply_info *replyinfo = (void *)t->data;

	printf("--arpreply-mac ");
	xtables_print_mac(replyinfo->mac);
	if (replyinfo->target == EBT_DROP)
		return;
	printf(" --arpreply-target %s", ebt_target_name(replyinfo->target));
}

static struct xtables_target arpreply_target = {
	.name		= "arpreply",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.init		= brarpreply_init,
	.size		= XT_ALIGN(sizeof(struct ebt_arpreply_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_arpreply_info)),
	.help		= brarpreply_print_help,
	.parse		= brarpreply_parse,
	.print		= brarpreply_print,
	.extra_opts	= brarpreply_opts,
};

void _init(void)
{
	xtables_register_target(&arpreply_target);
}
