/* ebt_arp
 *
 * Authors:
 * Bart De Schuymer <bdschuym@pandora.be>
 * Tim Gardner <timg@tpi.com>
 *
 * April, 2002
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <xtables.h>
#include <netinet/ether.h>

#include <xtables.h>
#include <net/if_arp.h>
#include <linux/netfilter_bridge/ebt_arp.h>
#include "iptables/nft.h"
#include "iptables/nft-bridge.h"

#define ARP_OPCODE '1'
#define ARP_HTYPE  '2'
#define ARP_PTYPE  '3'
#define ARP_IP_S   '4'
#define ARP_IP_D   '5'
#define ARP_MAC_S  '6'
#define ARP_MAC_D  '7'
#define ARP_GRAT   '8'

static const struct option brarp_opts[] = {
	{ "arp-opcode"    , required_argument, 0, ARP_OPCODE },
	{ "arp-op"        , required_argument, 0, ARP_OPCODE },
	{ "arp-htype"     , required_argument, 0, ARP_HTYPE  },
	{ "arp-ptype"     , required_argument, 0, ARP_PTYPE  },
	{ "arp-ip-src"    , required_argument, 0, ARP_IP_S   },
	{ "arp-ip-dst"    , required_argument, 0, ARP_IP_D   },
	{ "arp-mac-src"   , required_argument, 0, ARP_MAC_S  },
	{ "arp-mac-dst"   , required_argument, 0, ARP_MAC_D  },
	{ "arp-gratuitous",       no_argument, 0, ARP_GRAT   },
	XT_GETOPT_TABLEEND,
};

/* a few names */
static char *opcodes[] =
{
	"Request",
	"Reply",
	"Request_Reverse",
	"Reply_Reverse",
	"DRARP_Request",
	"DRARP_Reply",
	"DRARP_Error",
	"InARP_Request",
	"ARP_NAK",
};

static void brarp_print_help(void)
{
	int i;

	printf(
"arp options:\n"
"--arp-opcode  [!] opcode        : ARP opcode (integer or string)\n"
"--arp-htype   [!] type          : ARP hardware type (integer or string)\n"
"--arp-ptype   [!] type          : ARP protocol type (hexadecimal or string)\n"
"--arp-ip-src  [!] address[/mask]: ARP IP source specification\n"
"--arp-ip-dst  [!] address[/mask]: ARP IP target specification\n"
"--arp-mac-src [!] address[/mask]: ARP MAC source specification\n"
"--arp-mac-dst [!] address[/mask]: ARP MAC target specification\n"
"[!] --arp-gratuitous            : ARP gratuitous packet\n"
" opcode strings: \n");
	for (i = 0; i < ARRAY_SIZE(opcodes); i++)
		printf(" %d = %s\n", i + 1, opcodes[i]);
	printf(
" hardware type string: 1 = Ethernet\n"
" protocol type string: see "XT_PATH_ETHERTYPES"\n");
}

#define OPT_OPCODE 0x01
#define OPT_HTYPE  0x02
#define OPT_PTYPE  0x04
#define OPT_IP_S   0x08
#define OPT_IP_D   0x10
#define OPT_MAC_S  0x20
#define OPT_MAC_D  0x40
#define OPT_GRAT   0x80

static int undot_ip(char *ip, unsigned char *ip2)
{
	char *p, *q, *end;
	long int onebyte;
	int i;
	char buf[20];

	strncpy(buf, ip, sizeof(buf) - 1);

	p = buf;
	for (i = 0; i < 3; i++) {
		if ((q = strchr(p, '.')) == NULL)
			return -1;
		*q = '\0';
		onebyte = strtol(p, &end, 10);
		if (*end != '\0' || onebyte > 255 || onebyte < 0)
			return -1;
		ip2[i] = (unsigned char)onebyte;
		p = q + 1;
	}

	onebyte = strtol(p, &end, 10);
	if (*end != '\0' || onebyte > 255 || onebyte < 0)
		return -1;
	ip2[3] = (unsigned char)onebyte;

	return 0;
}

static int ip_mask(char *mask, unsigned char *mask2)
{
	char *end;
	long int bits;
	uint32_t mask22;

	if (undot_ip(mask, mask2)) {
		/* not the /a.b.c.e format, maybe the /x format */
		bits = strtol(mask, &end, 10);
		if (*end != '\0' || bits > 32 || bits < 0)
			return -1;
		if (bits != 0) {
			mask22 = htonl(0xFFFFFFFF << (32 - bits));
			memcpy(mask2, &mask22, 4);
		} else {
			mask22 = 0xFFFFFFFF;
			memcpy(mask2, &mask22, 4);
		}
	}
	return 0;
}

static void ebt_parse_ip_address(char *address, uint32_t *addr, uint32_t *msk)
{
	char *p;

	/* first the mask */
	if ((p = strrchr(address, '/')) != NULL) {
		*p = '\0';
		if (ip_mask(p + 1, (unsigned char *)msk)) {
			xtables_error(PARAMETER_PROBLEM,
				      "Problem with the IP mask '%s'", p + 1);
			return;
		}
	} else
		*msk = 0xFFFFFFFF;

	if (undot_ip(address, (unsigned char *)addr)) {
		xtables_error(PARAMETER_PROBLEM,
			      "Problem with the IP address '%s'", address);
		return;
	}
	*addr = *addr & *msk;
}

static int brarp_get_mac_and_mask(const char *from, unsigned char *to, unsigned char *mask)
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
brarp_parse(int c, char **argv, int invert, unsigned int *flags,
	    const void *entry, struct xt_entry_match **match)
{
	struct ebt_arp_info *arpinfo = (struct ebt_arp_info *)(*match)->data;
	long int i;
	char *end;
	uint32_t *addr;
	uint32_t *mask;
	unsigned char *maddr;
	unsigned char *mmask;

	switch (c) {
	case ARP_OPCODE:
		EBT_CHECK_OPTION(flags, OPT_OPCODE);
		if (invert)
			arpinfo->invflags |= EBT_ARP_OPCODE;
		i = strtol(optarg, &end, 10);
		if (i < 0 || i >= (0x1 << 16) || *end !='\0') {
			for (i = 0; i < ARRAY_SIZE(opcodes); i++)
				if (!strcasecmp(opcodes[i], optarg))
					break;
			if (i == ARRAY_SIZE(opcodes))
				xtables_error(PARAMETER_PROBLEM, "Problem with specified ARP opcode");
			i++;
		}
		arpinfo->opcode = htons(i);
		arpinfo->bitmask |= EBT_ARP_OPCODE;
		break;

	case ARP_HTYPE:
		EBT_CHECK_OPTION(flags, OPT_HTYPE);
		if (invert)
			arpinfo->invflags |= EBT_ARP_HTYPE;
		i = strtol(optarg, &end, 10);
		if (i < 0 || i >= (0x1 << 16) || *end !='\0') {
			if (!strcasecmp("Ethernet", argv[optind - 1]))
				i = 1;
			else
				xtables_error(PARAMETER_PROBLEM, "Problem with specified ARP hardware type");
		}
		arpinfo->htype = htons(i);
		arpinfo->bitmask |= EBT_ARP_HTYPE;
		break;
	case ARP_PTYPE: {
		uint16_t proto;

		EBT_CHECK_OPTION(flags, OPT_PTYPE);
		if (invert)
			arpinfo->invflags |= EBT_ARP_PTYPE;

		i = strtol(optarg, &end, 16);
		if (i < 0 || i >= (0x1 << 16) || *end !='\0') {
			struct xt_ethertypeent *ent;

			ent = xtables_getethertypebyname(argv[optind - 1]);
			if (!ent)
				xtables_error(PARAMETER_PROBLEM, "Problem with specified ARP "
								 "protocol type");
			proto = ent->e_ethertype;

		} else
			proto = i;
		arpinfo->ptype = htons(proto);
		arpinfo->bitmask |= EBT_ARP_PTYPE;
		break;
	}

	case ARP_IP_S:
	case ARP_IP_D:
		if (c == ARP_IP_S) {
			EBT_CHECK_OPTION(flags, OPT_IP_S);
			addr = &arpinfo->saddr;
			mask = &arpinfo->smsk;
			arpinfo->bitmask |= EBT_ARP_SRC_IP;
		} else {
			EBT_CHECK_OPTION(flags, OPT_IP_D);
			addr = &arpinfo->daddr;
			mask = &arpinfo->dmsk;
			arpinfo->bitmask |= EBT_ARP_DST_IP;
		}
		if (invert) {
			if (c == ARP_IP_S)
				arpinfo->invflags |= EBT_ARP_SRC_IP;
			else
				arpinfo->invflags |= EBT_ARP_DST_IP;
		}
		ebt_parse_ip_address(optarg, addr, mask);
		break;
	case ARP_MAC_S:
	case ARP_MAC_D:
		if (c == ARP_MAC_S) {
			EBT_CHECK_OPTION(flags, OPT_MAC_S);
			maddr = arpinfo->smaddr;
			mmask = arpinfo->smmsk;
			arpinfo->bitmask |= EBT_ARP_SRC_MAC;
		} else {
			EBT_CHECK_OPTION(flags, OPT_MAC_D);
			maddr = arpinfo->dmaddr;
			mmask = arpinfo->dmmsk;
			arpinfo->bitmask |= EBT_ARP_DST_MAC;
		}
		if (invert) {
			if (c == ARP_MAC_S)
				arpinfo->invflags |= EBT_ARP_SRC_MAC;
			else
				arpinfo->invflags |= EBT_ARP_DST_MAC;
		}
		if (brarp_get_mac_and_mask(optarg, maddr, mmask))
			xtables_error(PARAMETER_PROBLEM, "Problem with ARP MAC address argument");
		break;
	case ARP_GRAT:
		EBT_CHECK_OPTION(flags, OPT_GRAT);
		arpinfo->bitmask |= EBT_ARP_GRAT;
		if (invert)
			arpinfo->invflags |= EBT_ARP_GRAT;
		break;
	default:
		return 0;
	}
	return 1;
}

static void brarp_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct ebt_arp_info *arpinfo = (struct ebt_arp_info *)match->data;

	if (arpinfo->bitmask & EBT_ARP_OPCODE) {
		int opcode = ntohs(arpinfo->opcode);
		printf("--arp-op ");
		if (arpinfo->invflags & EBT_ARP_OPCODE)
			printf("! ");
		if (opcode > 0 && opcode <= ARRAY_SIZE(opcodes))
			printf("%s ", opcodes[opcode - 1]);
		else
			printf("%d ", opcode);
	}
	if (arpinfo->bitmask & EBT_ARP_HTYPE) {
		printf("--arp-htype ");
		if (arpinfo->invflags & EBT_ARP_HTYPE)
			printf("! ");
		printf("%d ", ntohs(arpinfo->htype));
	}
	if (arpinfo->bitmask & EBT_ARP_PTYPE) {
		printf("--arp-ptype ");
		if (arpinfo->invflags & EBT_ARP_PTYPE)
			printf("! ");
		printf("0x%x ", ntohs(arpinfo->ptype));
	}
	if (arpinfo->bitmask & EBT_ARP_SRC_IP) {
		printf("--arp-ip-src ");
		if (arpinfo->invflags & EBT_ARP_SRC_IP)
			printf("! ");
		printf("%s%s ", xtables_ipaddr_to_numeric((const struct in_addr*) &arpinfo->saddr),
		       xtables_ipmask_to_numeric((const struct in_addr*)&arpinfo->smsk));
	}
	if (arpinfo->bitmask & EBT_ARP_DST_IP) {
		printf("--arp-ip-dst ");
		if (arpinfo->invflags & EBT_ARP_DST_IP)
			printf("! ");
		printf("%s%s ", xtables_ipaddr_to_numeric((const struct in_addr*) &arpinfo->daddr),
		       xtables_ipmask_to_numeric((const struct in_addr*)&arpinfo->dmsk));
	}
	if (arpinfo->bitmask & EBT_ARP_SRC_MAC) {
		printf("--arp-mac-src ");
		if (arpinfo->invflags & EBT_ARP_SRC_MAC)
			printf("! ");
		xtables_print_mac_and_mask(arpinfo->smaddr, arpinfo->smmsk);
		printf(" ");
	}
	if (arpinfo->bitmask & EBT_ARP_DST_MAC) {
		printf("--arp-mac-dst ");
		if (arpinfo->invflags & EBT_ARP_DST_MAC)
			printf("! ");
		xtables_print_mac_and_mask(arpinfo->dmaddr, arpinfo->dmmsk);
		printf(" ");
	}
	if (arpinfo->bitmask & EBT_ARP_GRAT) {
		if (arpinfo->invflags & EBT_ARP_GRAT)
			printf("! ");
		printf("--arp-gratuitous ");
	}
}

static struct xtables_match brarp_match = {
	.name		= "arp",
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_arp_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_arp_info)),
	.help		= brarp_print_help,
	.parse		= brarp_parse,
	.print		= brarp_print,
	.extra_opts	= brarp_opts,
};

void _init(void)
{
	xtables_register_match(&brarp_match);
}
