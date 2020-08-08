/* ebt_pkttype
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 * April, 2003
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <netdb.h>
#include <xtables.h>
#include <linux/if_packet.h>
#include <linux/netfilter_bridge/ebt_pkttype.h>

static const char *classes[] = {
	"host",
	"broadcast",
	"multicast",
	"otherhost",
	"outgoing",
	"loopback",
	"fastroute",
};

static const struct option brpkttype_opts[] =
{
	{ "pkttype-type"        , required_argument, 0, '1' },
	{ 0 }
};

static void brpkttype_print_help(void)
{
	printf(
"pkttype options:\n"
"--pkttype-type    [!] type: class the packet belongs to\n"
"Possible values: broadcast, multicast, host, otherhost, or any other byte value (which would be pretty useless).\n");
}


static int brpkttype_parse(int c, char **argv, int invert, unsigned int *flags,
			   const void *entry, struct xt_entry_match **match)
{
	struct ebt_pkttype_info *ptinfo = (struct ebt_pkttype_info *)(*match)->data;
	char *end;
	long int i;

	switch (c) {
	case '1':
		if (invert)
			ptinfo->invert = 1;
		i = strtol(optarg, &end, 16);
		if (*end != '\0') {
			for (i = 0; i < ARRAY_SIZE(classes); i++) {
				if (!strcasecmp(optarg, classes[i]))
					break;
			}
			if (i >= ARRAY_SIZE(classes))
				xtables_error(PARAMETER_PROBLEM, "Could not parse class '%s'", optarg);
		}
		if (i < 0 || i > 255)
			xtables_error(PARAMETER_PROBLEM, "Problem with specified pkttype class");
		ptinfo->pkt_type = (uint8_t)i;
		break;
	default:
		return 0;
	}
	return 1;
}


static void brpkttype_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	struct ebt_pkttype_info *pt = (struct ebt_pkttype_info *)match->data;

	printf("--pkttype-type %s", pt->invert ? "! " : "");

	if (pt->pkt_type < ARRAY_SIZE(classes))
		printf("%s ", classes[pt->pkt_type]);
	else
		printf("%d ", pt->pkt_type);
}

static int brpkttype_xlate(struct xt_xlate *xl,
			  const struct xt_xlate_mt_params *params)
{
	const struct ebt_pkttype_info *info = (const void*)params->match->data;

	xt_xlate_add(xl, "meta pkttype %s", info->invert ? "!= " : "");

	if (info->pkt_type < 3)
		xt_xlate_add(xl, "%s ", classes[info->pkt_type]);
	else if (info->pkt_type == 3)
		xt_xlate_add(xl, "other ");
	else
		xt_xlate_add(xl, "%d ", info->pkt_type);

	return 1;
}

static struct xtables_match brpkttype_match = {
	.name		= "pkttype",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_pkttype_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_pkttype_info)),
	.help		= brpkttype_print_help,
	.parse		= brpkttype_parse,
	.print		= brpkttype_print,
	.xlate		= brpkttype_xlate,
	.extra_opts	= brpkttype_opts,
};

void _init(void)
{
	xtables_register_match(&brpkttype_match);
}
