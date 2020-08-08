/* ebt_stp
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 * July, 2003
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <netinet/ether.h>
#include <linux/netfilter_bridge/ebt_stp.h>
#include <xtables.h>

#include "iptables/nft.h"
#include "iptables/nft-bridge.h"

#define STP_TYPE	'a'
#define STP_FLAGS	'b'
#define STP_ROOTPRIO	'c'
#define STP_ROOTADDR	'd'
#define STP_ROOTCOST	'e'
#define STP_SENDERPRIO	'f'
#define STP_SENDERADDR	'g'
#define STP_PORT	'h'
#define STP_MSGAGE	'i'
#define STP_MAXAGE	'j'
#define STP_HELLOTIME	'k'
#define STP_FWDD	'l'
#define STP_NUMOPS 12

static const struct option brstp_opts[] =
{
	{ "stp-type"         , required_argument, 0, STP_TYPE},
	{ "stp-flags"        , required_argument, 0, STP_FLAGS},
	{ "stp-root-prio"    , required_argument, 0, STP_ROOTPRIO},
	{ "stp-root-addr"    , required_argument, 0, STP_ROOTADDR},
	{ "stp-root-cost"    , required_argument, 0, STP_ROOTCOST},
	{ "stp-sender-prio"  , required_argument, 0, STP_SENDERPRIO},
	{ "stp-sender-addr"  , required_argument, 0, STP_SENDERADDR},
	{ "stp-port"         , required_argument, 0, STP_PORT},
	{ "stp-msg-age"      , required_argument, 0, STP_MSGAGE},
	{ "stp-max-age"      , required_argument, 0, STP_MAXAGE},
	{ "stp-hello-time"   , required_argument, 0, STP_HELLOTIME},
	{ "stp-forward-delay", required_argument, 0, STP_FWDD},
	{ 0 }
};

#define BPDU_TYPE_CONFIG 0
#define BPDU_TYPE_TCN 0x80
#define BPDU_TYPE_CONFIG_STRING "config"
#define BPDU_TYPE_TCN_STRING "tcn"

#define FLAG_TC 0x01
#define FLAG_TC_ACK 0x80
#define FLAG_TC_STRING "topology-change"
#define FLAG_TC_ACK_STRING "topology-change-ack"

static void brstp_print_help(void)
{
	printf(
"stp options:\n"
"--stp-type type                  : BPDU type\n"
"--stp-flags flag                 : control flag\n"
"--stp-root-prio prio[:prio]      : root priority (16-bit) range\n"
"--stp-root-addr address[/mask]   : MAC address of root\n"
"--stp-root-cost cost[:cost]      : root cost (32-bit) range\n"
"--stp-sender-prio prio[:prio]    : sender priority (16-bit) range\n"
"--stp-sender-addr address[/mask] : MAC address of sender\n"
"--stp-port port[:port]           : port id (16-bit) range\n"
"--stp-msg-age age[:age]          : message age timer (16-bit) range\n"
"--stp-max-age age[:age]          : maximum age timer (16-bit) range\n"
"--stp-hello-time time[:time]     : hello time timer (16-bit) range\n"
"--stp-forward-delay delay[:delay]: forward delay timer (16-bit) range\n"
" Recognized BPDU type strings:\n"
"   \"config\": configuration BPDU (=0)\n"
"   \"tcn\"   : topology change notification BPDU (=0x80)\n"
" Recognized control flag strings:\n"
"   \"topology-change\"    : topology change flag (0x01)\n"
"   \"topology-change-ack\": topology change acknowledgement flag (0x80)");
}

static int parse_range(const char *portstring, void *lower, void *upper,
   int bits, uint32_t min, uint32_t max)
{
	char *buffer;
	char *cp, *end;
	uint32_t low_nr, upp_nr;
	int ret = 0;

	buffer = strdup(portstring);
	if ((cp = strchr(buffer, ':')) == NULL) {
		low_nr = strtoul(buffer, &end, 10);
		if (*end || low_nr < min || low_nr > max) {
			ret = -1;
			goto out;
		}
		if (bits == 2) {
			*(uint16_t *)lower =  low_nr;
			*(uint16_t *)upper =  low_nr;
		} else {
			*(uint32_t *)lower =  low_nr;
			*(uint32_t *)upper =  low_nr;
		}
	} else {
		*cp = '\0';
		cp++;
		if (!*buffer)
			low_nr = min;
		else {
			low_nr = strtoul(buffer, &end, 10);
			if (*end || low_nr < min) {
				ret = -1;
				goto out;
			}
		}
		if (!*cp)
			upp_nr = max;
		else {
			upp_nr = strtoul(cp, &end, 10);
			if (*end || upp_nr > max) {
				ret = -1;
				goto out;
			}
		}
		if (upp_nr < low_nr) {
			ret = -1;
			goto out;
		}
		if (bits == 2) {
			*(uint16_t *)lower = low_nr;
			*(uint16_t *)upper = upp_nr;
		} else {
			*(uint32_t *)lower = low_nr;
			*(uint32_t *)upper = upp_nr;
		}
	}
out:
	free(buffer);
	return ret;
}

static void print_range(unsigned int l, unsigned int u)
{
	if (l == u)
		printf("%u ", l);
	else
		printf("%u:%u ", l, u);
}

static int brstp_get_mac_and_mask(const char *from, unsigned char *to, unsigned char *mask)
{
	char *p;
	int i;
	struct ether_addr *addr = NULL;

	static const unsigned char mac_type_unicast[ETH_ALEN];
	static const unsigned char msk_type_unicast[ETH_ALEN] =   {1,0,0,0,0,0};
	static const unsigned char mac_type_multicast[ETH_ALEN] = {1,0,0,0,0,0};
	static const unsigned char mac_type_broadcast[ETH_ALEN] = {255,255,255,255,255,255};
	static const unsigned char mac_type_bridge_group[ETH_ALEN] = {0x01,0x80,0xc2,0,0,0};
	static const unsigned char msk_type_bridge_group[ETH_ALEN] = {255,255,255,255,255,255};

	if (strcasecmp(from, "Unicast") == 0) {
		memcpy(to, mac_type_unicast, ETH_ALEN);
		memcpy(mask, msk_type_unicast, ETH_ALEN);
		return 0;
	}
	if (strcasecmp(from, "Multicast") == 0) {
		memcpy(to, mac_type_multicast, ETH_ALEN);
		memcpy(mask, mac_type_multicast, ETH_ALEN);
		return 0;
	}
	if (strcasecmp(from, "Broadcast") == 0) {
		memcpy(to, mac_type_broadcast, ETH_ALEN);
		memcpy(mask, mac_type_broadcast, ETH_ALEN);
		return 0;
	}
	if (strcasecmp(from, "BGA") == 0) {
		memcpy(to, mac_type_bridge_group, ETH_ALEN);
		memcpy(mask, msk_type_bridge_group, ETH_ALEN);
		return 0;
	}
	if ( (p = strrchr(from, '/')) != NULL) {
		*p = '\0';
		if (!(addr = ether_aton(p + 1)))
			return -1;
		memcpy(mask, addr, ETH_ALEN);
	} else
		memset(mask, 0xff, ETH_ALEN);
	if (!(addr = ether_aton(from)))
		return -1;
	memcpy(to, addr, ETH_ALEN);
	for (i = 0; i < ETH_ALEN; i++)
		to[i] &= mask[i];
	return 0;
}

static int
brstp_parse(int c, char **argv, int invert, unsigned int *flags,
	    const void *entry, struct xt_entry_match **match)
{
	struct ebt_stp_info *stpinfo = (struct ebt_stp_info *)(*match)->data;
	unsigned int flag;
	long int i;
	char *end = NULL;

	if (c < 'a' || c > ('a' + STP_NUMOPS - 1))
		return 0;
	flag = 1 << (c - 'a');
	EBT_CHECK_OPTION(flags, flag);
	if (invert)
		stpinfo->invflags |= flag;
	stpinfo->bitmask |= flag;
	switch (flag) {
	case EBT_STP_TYPE:
		i = strtol(optarg, &end, 0);
		if (i < 0 || i > 255 || *end != '\0') {
			if (!strcasecmp(optarg, BPDU_TYPE_CONFIG_STRING))
				stpinfo->type = BPDU_TYPE_CONFIG;
			else if (!strcasecmp(optarg, BPDU_TYPE_TCN_STRING))
				stpinfo->type = BPDU_TYPE_TCN;
			else
				xtables_error(PARAMETER_PROBLEM, "Bad --stp-type argument");
		} else
			stpinfo->type = i;
		break;
	case EBT_STP_FLAGS:
		i = strtol(optarg, &end, 0);
		if (i < 0 || i > 255 || *end != '\0') {
			if (!strcasecmp(optarg, FLAG_TC_STRING))
				stpinfo->config.flags = FLAG_TC;
			else if (!strcasecmp(optarg, FLAG_TC_ACK_STRING))
				stpinfo->config.flags = FLAG_TC_ACK;
			else
				xtables_error(PARAMETER_PROBLEM, "Bad --stp-flags argument");
		} else
			stpinfo->config.flags = i;
		break;
	case EBT_STP_ROOTPRIO:
		if (parse_range(argv[optind-1], &(stpinfo->config.root_priol),
		    &(stpinfo->config.root_priou), 2, 0, 0xffff))
			xtables_error(PARAMETER_PROBLEM, "Bad --stp-root-prio range");
		break;
	case EBT_STP_ROOTCOST:
		if (parse_range(argv[optind-1], &(stpinfo->config.root_costl),
		    &(stpinfo->config.root_costu), 4, 0, 0xffffffff))
			xtables_error(PARAMETER_PROBLEM, "Bad --stp-root-cost range");
		break;
	case EBT_STP_SENDERPRIO:
		if (parse_range(argv[optind-1], &(stpinfo->config.sender_priol),
		    &(stpinfo->config.sender_priou), 2, 0, 0xffff))
			xtables_error(PARAMETER_PROBLEM, "Bad --stp-sender-prio range");
		break;
	case EBT_STP_PORT:
		if (parse_range(argv[optind-1], &(stpinfo->config.portl),
		    &(stpinfo->config.portu), 2, 0, 0xffff))
			xtables_error(PARAMETER_PROBLEM, "Bad --stp-port-range");
		break;
	case EBT_STP_MSGAGE:
		if (parse_range(argv[optind-1], &(stpinfo->config.msg_agel),
		    &(stpinfo->config.msg_ageu), 2, 0, 0xffff))
			xtables_error(PARAMETER_PROBLEM, "Bad --stp-msg-age range");
		break;
	case EBT_STP_MAXAGE:
		if (parse_range(argv[optind-1], &(stpinfo->config.max_agel),
		    &(stpinfo->config.max_ageu), 2, 0, 0xffff))
			xtables_error(PARAMETER_PROBLEM, "Bad --stp-max-age range");
		break;
	case EBT_STP_HELLOTIME:
		if (parse_range(argv[optind-1], &(stpinfo->config.hello_timel),
		    &(stpinfo->config.hello_timeu), 2, 0, 0xffff))
			xtables_error(PARAMETER_PROBLEM, "Bad --stp-hello-time range");
		break;
	case EBT_STP_FWDD:
		if (parse_range(argv[optind-1], &(stpinfo->config.forward_delayl),
		    &(stpinfo->config.forward_delayu), 2, 0, 0xffff))
			xtables_error(PARAMETER_PROBLEM, "Bad --stp-forward-delay range");
		break;
	case EBT_STP_ROOTADDR:
		if (brstp_get_mac_and_mask(argv[optind-1],
		    (unsigned char *)stpinfo->config.root_addr,
		    (unsigned char *)stpinfo->config.root_addrmsk))
			xtables_error(PARAMETER_PROBLEM, "Bad --stp-root-addr address");
		break;
	case EBT_STP_SENDERADDR:
		if (brstp_get_mac_and_mask(argv[optind-1],
		    (unsigned char *)stpinfo->config.sender_addr,
		    (unsigned char *)stpinfo->config.sender_addrmsk))
			xtables_error(PARAMETER_PROBLEM, "Bad --stp-sender-addr address");
		break;
	default:
		xtables_error(PARAMETER_PROBLEM, "Unknown stp option");
	}
	return 1;
}

static void brstp_print(const void *ip, const struct xt_entry_match *match,
			 int numeric)
{
	const struct ebt_stp_info *stpinfo = (struct ebt_stp_info *)match->data;
	const struct ebt_stp_config_info *c = &(stpinfo->config);
	int i;

	for (i = 0; i < STP_NUMOPS; i++) {
		if (!(stpinfo->bitmask & (1 << i)))
			continue;
		printf("--%s %s", brstp_opts[i].name,
		       (stpinfo->invflags & (1 << i)) ? "! " : "");
		if (EBT_STP_TYPE == (1 << i)) {
			if (stpinfo->type == BPDU_TYPE_CONFIG)
				printf("%s", BPDU_TYPE_CONFIG_STRING);
			else if (stpinfo->type == BPDU_TYPE_TCN)
				printf("%s", BPDU_TYPE_TCN_STRING);
			else
				printf("%d", stpinfo->type);
		} else if (EBT_STP_FLAGS == (1 << i)) {
			if (c->flags == FLAG_TC)
				printf("%s", FLAG_TC_STRING);
			else if (c->flags == FLAG_TC_ACK)
				printf("%s", FLAG_TC_ACK_STRING);
			else
				printf("%d", c->flags);
		} else if (EBT_STP_ROOTPRIO == (1 << i))
			print_range(c->root_priol, c->root_priou);
		else if (EBT_STP_ROOTADDR == (1 << i))
			xtables_print_mac_and_mask((unsigned char *)c->root_addr,
			   (unsigned char*)c->root_addrmsk);
		else if (EBT_STP_ROOTCOST == (1 << i))
			print_range(c->root_costl, c->root_costu);
		else if (EBT_STP_SENDERPRIO == (1 << i))
			print_range(c->sender_priol, c->sender_priou);
		else if (EBT_STP_SENDERADDR == (1 << i))
			xtables_print_mac_and_mask((unsigned char *)c->sender_addr,
			   (unsigned char *)c->sender_addrmsk);
		else if (EBT_STP_PORT == (1 << i))
			print_range(c->portl, c->portu);
		else if (EBT_STP_MSGAGE == (1 << i))
			print_range(c->msg_agel, c->msg_ageu);
		else if (EBT_STP_MAXAGE == (1 << i))
			print_range(c->max_agel, c->max_ageu);
		else if (EBT_STP_HELLOTIME == (1 << i))
			print_range(c->hello_timel, c->hello_timeu);
		else if (EBT_STP_FWDD == (1 << i))
			print_range(c->forward_delayl, c->forward_delayu);
		printf(" ");
	}
}

static struct xtables_match brstp_match = {
	.name		= "stp",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= sizeof(struct ebt_stp_info),
	.help		= brstp_print_help,
	.parse		= brstp_parse,
	.print		= brstp_print,
	.extra_opts	= brstp_opts,
};

void _init(void)
{
	xtables_register_match(&brstp_match);
}
