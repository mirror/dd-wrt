/* ebt_ip6
 *
 * Authors:
 * Kuo-Lang Tseng <kuo-lang.tseng@intel.com>
 * Manohar Castelino <manohar.castelino@intel.com>
 *
 * Summary:
 * This is just a modification of the IPv4 code written by
 * Bart De Schuymer <bdschuym@pandora.be>
 * with the changes required to support IPv6
 *
 */

#include <errno.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <netdb.h>
#include <xtables.h>
#include <linux/netfilter_bridge/ebt_ip6.h>

#include "libxt_icmp.h"

#define IP_SOURCE '1'
#define IP_DEST   '2'
#define IP_TCLASS '3'
#define IP_PROTO  '4'
#define IP_SPORT  '5'
#define IP_DPORT  '6'
#define IP_ICMP6  '7'

static const struct option brip6_opts[] = {
	{ .name = "ip6-source",		.has_arg = true, .val = IP_SOURCE },
	{ .name = "ip6-src",		.has_arg = true, .val = IP_SOURCE },
	{ .name = "ip6-destination",	.has_arg = true, .val = IP_DEST },
	{ .name = "ip6-dst",		.has_arg = true, .val = IP_DEST },
	{ .name = "ip6-tclass",		.has_arg = true, .val = IP_TCLASS },
	{ .name = "ip6-protocol",	.has_arg = true, .val = IP_PROTO },
	{ .name = "ip6-proto",		.has_arg = true, .val = IP_PROTO },
	{ .name = "ip6-source-port",	.has_arg = true, .val = IP_SPORT },
	{ .name = "ip6-sport",		.has_arg = true, .val = IP_SPORT },
	{ .name = "ip6-destination-port",.has_arg = true,.val = IP_DPORT },
	{ .name = "ip6-dport",		.has_arg = true, .val = IP_DPORT },
	{ .name = "ip6-icmp-type",	.has_arg = true, .val = IP_ICMP6 },
	XT_GETOPT_TABLEEND,
};

static const struct xt_icmp_names icmpv6_codes[] = {
	{ "destination-unreachable", 1, 0, 0xFF },
	{ "no-route", 1, 0, 0 },
	{ "communication-prohibited", 1, 1, 1 },
	{ "address-unreachable", 1, 3, 3 },
	{ "port-unreachable", 1, 4, 4 },

	{ "packet-too-big", 2, 0, 0xFF },

	{ "time-exceeded", 3, 0, 0xFF },
	/* Alias */ { "ttl-exceeded", 3, 0, 0xFF },
	{ "ttl-zero-during-transit", 3, 0, 0 },
	{ "ttl-zero-during-reassembly", 3, 1, 1 },

	{ "parameter-problem", 4, 0, 0xFF },
	{ "bad-header", 4, 0, 0 },
	{ "unknown-header-type", 4, 1, 1 },
	{ "unknown-option", 4, 2, 2 },

	{ "echo-request", 128, 0, 0xFF },
	/* Alias */ { "ping", 128, 0, 0xFF },

	{ "echo-reply", 129, 0, 0xFF },
	/* Alias */ { "pong", 129, 0, 0xFF },

	{ "router-solicitation", 133, 0, 0xFF },

	{ "router-advertisement", 134, 0, 0xFF },

	{ "neighbour-solicitation", 135, 0, 0xFF },
	/* Alias */ { "neighbor-solicitation", 135, 0, 0xFF },

	{ "neighbour-advertisement", 136, 0, 0xFF },
	/* Alias */ { "neighbor-advertisement", 136, 0, 0xFF },

	{ "redirect", 137, 0, 0xFF },
};

static void
parse_port_range(const char *protocol, const char *portstring, uint16_t *ports)
{
	char *buffer;
	char *cp;

	buffer = strdup(portstring);
	if ((cp = strchr(buffer, ':')) == NULL)
		ports[0] = ports[1] = xtables_parse_port(buffer, NULL);
	else {
		*cp = '\0';
		cp++;

		ports[0] = buffer[0] ? xtables_parse_port(buffer, NULL) : 0;
		ports[1] = cp[0] ? xtables_parse_port(cp, NULL) : 0xFFFF;

		if (ports[0] > ports[1])
			xtables_error(PARAMETER_PROBLEM,
				      "invalid portrange (min > max)");
	}
	free(buffer);
}

static char *parse_range(const char *str, unsigned int res[])
{
	char *next;

	if (!xtables_strtoui(str, &next, &res[0], 0, 255))
		return NULL;

	res[1] = res[0];
	if (*next == ':') {
		str = next + 1;
		if (!xtables_strtoui(str, &next, &res[1], 0, 255))
			return NULL;
	}

	return next;
}

static int
parse_icmpv6(const char *icmpv6type, uint8_t type[], uint8_t code[])
{
	static const unsigned int limit = ARRAY_SIZE(icmpv6_codes);
	unsigned int match = limit;
	unsigned int i, number[2];

	for (i = 0; i < limit; i++) {
		if (strncasecmp(icmpv6_codes[i].name, icmpv6type, strlen(icmpv6type)))
			continue;
		if (match != limit)
			xtables_error(PARAMETER_PROBLEM, "Ambiguous ICMPv6 type `%s':"
					" `%s' or `%s'?",
					icmpv6type, icmpv6_codes[match].name,
					icmpv6_codes[i].name);
		match = i;
	}

	if (match < limit) {
		type[0] = type[1] = icmpv6_codes[match].type;
		code[0] = icmpv6_codes[match].code_min;
		code[1] = icmpv6_codes[match].code_max;
	} else {
		char *next = parse_range(icmpv6type, number);
		if (!next) {
			xtables_error(PARAMETER_PROBLEM, "Unknown ICMPv6 type `%s'",
							icmpv6type);
			return -1;
		}
		type[0] = (uint8_t) number[0];
		type[1] = (uint8_t) number[1];
		switch (*next) {
		case 0:
			code[0] = 0;
			code[1] = 255;
			return 0;
		case '/':
			next = parse_range(next+1, number);
			code[0] = (uint8_t) number[0];
			code[1] = (uint8_t) number[1];
			if (next == NULL)
				return -1;
			if (next && *next == 0)
				return 0;
		/* fallthrough */
		default:
			xtables_error(PARAMETER_PROBLEM, "unknown character %c", *next);
			return -1;
		}
	}
	return 0;
}

static void print_port_range(uint16_t *ports)
{
	if (ports[0] == ports[1])
		printf("%d ", ports[0]);
	else
		printf("%d:%d ", ports[0], ports[1]);
}

static void print_icmp_code(uint8_t *code)
{
	if (code[0] == code[1])
		printf("/%"PRIu8 " ", code[0]);
	else
		printf("/%"PRIu8":%"PRIu8 " ", code[0], code[1]);
}

static void print_icmp_type(uint8_t *type, uint8_t *code)
{
	unsigned int i;

	if (type[0] != type[1]) {
		printf("%"PRIu8 ":%" PRIu8, type[0], type[1]);
		print_icmp_code(code);
		return;
	}

	for (i = 0; i < ARRAY_SIZE(icmpv6_codes); i++) {
		if (icmpv6_codes[i].type != type[0])
			continue;

		if (icmpv6_codes[i].code_min == code[0] &&
		    icmpv6_codes[i].code_max == code[1]) {
			printf("%s ", icmpv6_codes[i].name);
			return;
		}
	}
	printf("%"PRIu8, type[0]);
	print_icmp_code(code);
}

static void brip6_print_help(void)
{
	printf(
"ip6 options:\n"
"--ip6-src    [!] address[/mask]: ipv6 source specification\n"
"--ip6-dst    [!] address[/mask]: ipv6 destination specification\n"
"--ip6-tclass [!] tclass        : ipv6 traffic class specification\n"
"--ip6-proto  [!] protocol      : ipv6 protocol specification\n"
"--ip6-sport  [!] port[:port]   : tcp/udp source port or port range\n"
"--ip6-dport  [!] port[:port]   : tcp/udp destination port or port range\n"
"--ip6-icmp-type [!] type[[:type]/code[:code]] : ipv6-icmp type/code or type/code range\n");
	printf("Valid ICMPv6 Types:");
	xt_print_icmp_types(icmpv6_codes, ARRAY_SIZE(icmpv6_codes));
}

static void brip6_init(struct xt_entry_match *match)
{
	struct ebt_ip6_info *ipinfo = (struct ebt_ip6_info *)match->data;

	ipinfo->invflags = 0;
	ipinfo->bitmask = 0;
	memset(ipinfo->saddr.s6_addr, 0, sizeof(ipinfo->saddr.s6_addr));
	memset(ipinfo->smsk.s6_addr, 0, sizeof(ipinfo->smsk.s6_addr));
	memset(ipinfo->daddr.s6_addr, 0, sizeof(ipinfo->daddr.s6_addr));
	memset(ipinfo->dmsk.s6_addr, 0, sizeof(ipinfo->dmsk.s6_addr));
}

static struct in6_addr *numeric_to_addr(const char *num)
{
	static struct in6_addr ap;
	int err;

	if ((err=inet_pton(AF_INET6, num, &ap)) == 1)
		return &ap;
	return (struct in6_addr *)NULL;
}

static struct in6_addr *parse_ip6_mask(char *mask)
{
	static struct in6_addr maskaddr;
	struct in6_addr *addrp;
	unsigned int bits;

	if (mask == NULL) {
		/* no mask at all defaults to 128 bits */
		memset(&maskaddr, 0xff, sizeof maskaddr);
		return &maskaddr;
	}
	if ((addrp = numeric_to_addr(mask)) != NULL)
		return addrp;
	if (!xtables_strtoui(mask, NULL, &bits, 0, 128))
		xtables_error(PARAMETER_PROBLEM, "Invalid IPv6 Mask '%s' specified", mask);
	if (bits != 0) {
		char *p = (char *)&maskaddr;
		memset(p, 0xff, bits / 8);
		memset(p + (bits / 8) + 1, 0, (128 - bits) / 8);
		p[bits / 8] = 0xff << (8 - (bits & 7));
		return &maskaddr;
	}

	memset(&maskaddr, 0, sizeof maskaddr);
	return &maskaddr;
}

/* Set the ipv6 mask and address. Callers should check ebt_errormsg[0].
 * The string pointed to by address can be altered. */
static void ebt_parse_ip6_address(char *address, struct in6_addr *addr, struct in6_addr *msk)
{
	struct in6_addr *tmp_addr;
	char buf[256];
	char *p;
	int i;
	int err;

	strncpy(buf, address, sizeof(buf) - 1);
	/* first the mask */
	buf[sizeof(buf) - 1] = '\0';
	if ((p = strrchr(buf, '/')) != NULL) {
		*p = '\0';
		tmp_addr = parse_ip6_mask(p + 1);
	} else
		tmp_addr = parse_ip6_mask(NULL);

	*msk = *tmp_addr;

	/* if a null mask is given, the name is ignored, like in "any/0" */
	if (!memcmp(msk, &in6addr_any, sizeof(in6addr_any)))
		strcpy(buf, "::");

	if ((err=inet_pton(AF_INET6, buf, addr)) < 1) {
		xtables_error(PARAMETER_PROBLEM, "Invalid IPv6 Address '%s' specified", buf);
		return;
	}

	for (i = 0; i < 4; i++)
		addr->s6_addr32[i] &= msk->s6_addr32[i];
}

#define OPT_SOURCE 0x01
#define OPT_DEST   0x02
#define OPT_TCLASS 0x04
#define OPT_PROTO  0x08
#define OPT_SPORT  0x10
#define OPT_DPORT  0x20
static int
brip6_parse(int c, char **argv, int invert, unsigned int *flags,
	   const void *entry, struct xt_entry_match **match)
{
	struct ebt_ip6_info *info = (struct ebt_ip6_info *)(*match)->data;
	unsigned int i;
	char *end;

	switch (c) {
	case IP_SOURCE:
		if (invert)
			info->invflags |= EBT_IP6_SOURCE;
		ebt_parse_ip6_address(optarg, &info->saddr, &info->smsk);
		info->bitmask |= EBT_IP6_SOURCE;
		break;
	case IP_DEST:
		if (invert)
			info->invflags |= EBT_IP6_DEST;
		ebt_parse_ip6_address(optarg, &info->daddr, &info->dmsk);
		info->bitmask |= EBT_IP6_DEST;
		break;
	case IP_SPORT:
		if (invert)
			info->invflags |= EBT_IP6_SPORT;
		parse_port_range(NULL, optarg, info->sport);
		info->bitmask |= EBT_IP6_SPORT;
		break;
	case IP_DPORT:
		if (invert)
			info->invflags |= EBT_IP6_DPORT;
		parse_port_range(NULL, optarg, info->dport);
		info->bitmask |= EBT_IP6_DPORT;
		break;
	case IP_ICMP6:
		if (invert)
			info->invflags |= EBT_IP6_ICMP6;
		if (parse_icmpv6(optarg, info->icmpv6_type, info->icmpv6_code))
			return 0;
		info->bitmask |= EBT_IP6_ICMP6;
		break;
	case IP_TCLASS:
		if (invert)
			info->invflags |= EBT_IP6_TCLASS;
		if (!xtables_strtoui(optarg, &end, &i, 0, 255))
			xtables_error(PARAMETER_PROBLEM, "Problem with specified IPv6 traffic class '%s'", optarg);
		info->tclass = i;
		info->bitmask |= EBT_IP6_TCLASS;
		break;
	case IP_PROTO:
		if (invert)
			info->invflags |= EBT_IP6_PROTO;
		info->protocol = xtables_parse_protocol(optarg);
		info->bitmask |= EBT_IP6_PROTO;
		break;
	default:
		return 0;
	}

	*flags |= info->bitmask;
	return 1;
}

static void brip6_final_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			      "You must specify proper arguments");
}

static void brip6_print(const void *ip, const struct xt_entry_match *match,
		       int numeric)
{
	struct ebt_ip6_info *ipinfo = (struct ebt_ip6_info *)match->data;

	if (ipinfo->bitmask & EBT_IP6_SOURCE) {
		printf("--ip6-src ");
		if (ipinfo->invflags & EBT_IP6_SOURCE)
			printf("! ");
		printf("%s", xtables_ip6addr_to_numeric(&ipinfo->saddr));
		printf("%s ", xtables_ip6mask_to_numeric(&ipinfo->smsk));
	}
	if (ipinfo->bitmask & EBT_IP6_DEST) {
		printf("--ip6-dst ");
		if (ipinfo->invflags & EBT_IP6_DEST)
			printf("! ");
		printf("%s", xtables_ip6addr_to_numeric(&ipinfo->daddr));
		printf("%s ", xtables_ip6mask_to_numeric(&ipinfo->dmsk));
	}
	if (ipinfo->bitmask & EBT_IP6_TCLASS) {
		printf("--ip6-tclass ");
		if (ipinfo->invflags & EBT_IP6_TCLASS)
			printf("! ");
		printf("0x%02X ", ipinfo->tclass);
	}
	if (ipinfo->bitmask & EBT_IP6_PROTO) {
		struct protoent *pe;

		printf("--ip6-proto ");
		if (ipinfo->invflags & EBT_IP6_PROTO)
			printf("! ");
		pe = getprotobynumber(ipinfo->protocol);
		if (pe == NULL) {
			printf("%d ", ipinfo->protocol);
		} else {
			printf("%s ", pe->p_name);
		}
	}
	if (ipinfo->bitmask & EBT_IP6_SPORT) {
		printf("--ip6-sport ");
		if (ipinfo->invflags & EBT_IP6_SPORT)
			printf("! ");
		print_port_range(ipinfo->sport);
	}
	if (ipinfo->bitmask & EBT_IP6_DPORT) {
		printf("--ip6-dport ");
		if (ipinfo->invflags & EBT_IP6_DPORT)
			printf("! ");
		print_port_range(ipinfo->dport);
	}
	if (ipinfo->bitmask & EBT_IP6_ICMP6) {
		printf("--ip6-icmp-type ");
		if (ipinfo->invflags & EBT_IP6_ICMP6)
			printf("! ");
		print_icmp_type(ipinfo->icmpv6_type, ipinfo->icmpv6_code);
	}
}

static void brip_xlate_th(struct xt_xlate *xl,
			  const struct ebt_ip6_info *info, int bit,
			  const char *pname)
{
	const uint16_t *ports;

	if ((info->bitmask & bit) == 0)
		return;

	switch (bit) {
	case EBT_IP6_SPORT:
		if (pname)
			xt_xlate_add(xl, "%s sport ", pname);
		else
			xt_xlate_add(xl, "@th,0,16 ");

		ports = info->sport;
		break;
	case EBT_IP6_DPORT:
		if (pname)
			xt_xlate_add(xl, "%s dport ", pname);
		else
			xt_xlate_add(xl, "@th,16,16 ");

		ports = info->dport;
		break;
	default:
		return;
	}

	if (info->invflags & bit)
		xt_xlate_add(xl, "!= ");

	if (ports[0] == ports[1])
		xt_xlate_add(xl, "%d ", ports[0]);
	else
		xt_xlate_add(xl, "%d-%d ", ports[0], ports[1]);
}

static void brip_xlate_nh(struct xt_xlate *xl,
			  const struct ebt_ip6_info *info, int bit)
{
	struct in6_addr *addrp, *maskp;

	if ((info->bitmask & bit) == 0)
		return;

	switch (bit) {
	case EBT_IP6_SOURCE:
		xt_xlate_add(xl, "ip6 saddr ");
		addrp = (struct in6_addr *)&info->saddr;
		maskp = (struct in6_addr *)&info->smsk;
		break;
	case EBT_IP6_DEST:
		xt_xlate_add(xl, "ip6 daddr ");
		addrp = (struct in6_addr *)&info->daddr;
		maskp = (struct in6_addr *)&info->dmsk;
		break;
	default:
		return;
	}

	if (info->invflags & bit)
		xt_xlate_add(xl, "!= ");

	xt_xlate_add(xl, "%s%s ", xtables_ip6addr_to_numeric(addrp),
				  xtables_ip6mask_to_numeric(maskp));
}

static const char *brip6_xlate_proto_to_name(uint8_t proto)
{
	switch (proto) {
	case IPPROTO_TCP:
		return "tcp";
	case IPPROTO_UDP:
		return "udp";
	case IPPROTO_UDPLITE:
		return "udplite";
	case IPPROTO_SCTP:
		return "sctp";
	case IPPROTO_DCCP:
		return "dccp";
	default:
		return NULL;
	}
}

static int brip6_xlate(struct xt_xlate *xl,
		      const struct xt_xlate_mt_params *params)
{
	const struct ebt_ip6_info *info = (const void *)params->match->data;
	const char *pname = NULL;

	if ((info->bitmask & (EBT_IP6_SOURCE|EBT_IP6_DEST|EBT_IP6_ICMP6|EBT_IP6_TCLASS)) == 0)
		xt_xlate_add(xl, "ether type ip6 ");

	brip_xlate_nh(xl, info, EBT_IP6_SOURCE);
	brip_xlate_nh(xl, info, EBT_IP6_DEST);

	if (info->bitmask & EBT_IP6_TCLASS) {
		xt_xlate_add(xl, "ip6 dscp ");
		if (info->invflags & EBT_IP6_TCLASS)
			xt_xlate_add(xl, "!= ");
		xt_xlate_add(xl, "0x%02x ", info->tclass & 0x3f); /* remove ECN bits */
	}

	if (info->bitmask & EBT_IP6_PROTO) {
		struct protoent *pe;

		if (info->bitmask & (EBT_IP6_SPORT|EBT_IP6_DPORT|EBT_IP6_ICMP6) &&
		    (info->invflags & EBT_IP6_PROTO) == 0) {
			/* port number given and not inverted, no need to
			 * add explicit 'meta l4proto'.
			 */
			pname = brip6_xlate_proto_to_name(info->protocol);
		} else {
			xt_xlate_add(xl, "meta l4proto ");
			if (info->invflags & EBT_IP6_PROTO)
				xt_xlate_add(xl, "!= ");
			pe = getprotobynumber(info->protocol);
			if (pe == NULL)
				xt_xlate_add(xl, "%d ", info->protocol);
			else
				xt_xlate_add(xl, "%s ", pe->p_name);
		}
	}

	brip_xlate_th(xl, info, EBT_IP6_SPORT, pname);
	brip_xlate_th(xl, info, EBT_IP6_DPORT, pname);

	if (info->bitmask & EBT_IP6_ICMP6) {
		xt_xlate_add(xl, "icmpv6 type ");
		if (info->invflags & EBT_IP6_ICMP6)
			xt_xlate_add(xl, "!= ");

		if (info->icmpv6_type[0] == info->icmpv6_type[1])
			xt_xlate_add(xl, "%d ", info->icmpv6_type[0]);
		else
			xt_xlate_add(xl, "%d-%d ", info->icmpv6_type[0],
						   info->icmpv6_type[1]);

		if (info->icmpv6_code[0] == 0 &&
		    info->icmpv6_code[1] == 0xff)
			return 1;

		xt_xlate_add(xl, "icmpv6 code ");
		if (info->invflags & EBT_IP6_ICMP6)
			xt_xlate_add(xl, "!= ");

		if (info->icmpv6_code[0] == info->icmpv6_code[1])
			xt_xlate_add(xl, "%d ", info->icmpv6_code[0]);
		else
			xt_xlate_add(xl, "%d-%d ", info->icmpv6_code[0],
						   info->icmpv6_code[1]);
	}

	return 1;
}

static struct xtables_match brip6_match = {
	.name		= "ip6",
	.revision	= 0,
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_ip6_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_ip6_info)),
	.init		= brip6_init,
	.help		= brip6_print_help,
	.parse		= brip6_parse,
	.final_check	= brip6_final_check,
	.print		= brip6_print,
	.xlate		= brip6_xlate,
	.extra_opts	= brip6_opts,
};

void _init(void)
{
	xtables_register_match(&brip6_match);
}
