/*
 * Copyright (c) 2011 Patrick McHardy <kaber@trash.net>
 *
 * Based on Rusty Russell's IPv4 DNAT target. Development of IPv6 NAT
 * funded by Astaro.
 */

#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <xtables.h>
#include <iptables.h>
#include <limits.h> /* INT_MAX in ip_tables.h */
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <linux/netfilter/nf_nat.h>

enum {
	O_TO_DEST = 0,
	O_RANDOM,
	O_PERSISTENT,
	O_X_TO_DEST,
	F_TO_DEST   = 1 << O_TO_DEST,
	F_RANDOM   = 1 << O_RANDOM,
	F_X_TO_DEST = 1 << O_X_TO_DEST,
};

static void DNAT_help(void)
{
	printf(
"DNAT target options:\n"
" --to-destination [<ipaddr>[-<ipaddr>]][:port[-port]]\n"
"				Address to map destination to.\n"
"[--random] [--persistent]\n");
}

static void DNAT_help_v2(void)
{
	printf(
"DNAT target options:\n"
" --to-destination [<ipaddr>[-<ipaddr>]][:port[-port[/port]]]\n"
"				Address to map destination to.\n"
"[--random] [--persistent]\n");
}

static const struct xt_option_entry DNAT_opts[] = {
	{.name = "to-destination", .id = O_TO_DEST, .type = XTTYPE_STRING,
	 .flags = XTOPT_MAND | XTOPT_MULTI},
	{.name = "random", .id = O_RANDOM, .type = XTTYPE_NONE},
	{.name = "persistent", .id = O_PERSISTENT, .type = XTTYPE_NONE},
	XTOPT_TABLEEND,
};

/* Ranges expected in network order. */
static void
parse_to(const char *orig_arg, int portok, struct nf_nat_range2 *range, int rev)
{
	char *arg, *start, *end = NULL, *colon = NULL, *dash, *error;
	const struct in6_addr *ip;

	arg = strdup(orig_arg);
	if (arg == NULL)
		xtables_error(RESOURCE_PROBLEM, "strdup");

	start = strchr(arg, '[');
	if (start == NULL) {
		start = arg;
		/* Lets assume one colon is port information. Otherwise its an IPv6 address */
		colon = strchr(arg, ':');
		if (colon && strchr(colon+1, ':'))
			colon = NULL;
	}
	else {
		start++;
		end = strchr(start, ']');
		if (end == NULL)
			xtables_error(PARAMETER_PROBLEM,
				      "Invalid address format");

		*end = '\0';
		colon = strchr(end + 1, ':');
	}

	if (colon) {
		int port;

		if (!portok)
			xtables_error(PARAMETER_PROBLEM,
				   "Need TCP, UDP, SCTP or DCCP with port specification");

		range->flags |= NF_NAT_RANGE_PROTO_SPECIFIED;

		port = atoi(colon+1);
		if (port <= 0 || port > 65535)
			xtables_error(PARAMETER_PROBLEM,
				   "Port `%s' not valid\n", colon+1);

		error = strchr(colon+1, ':');
		if (error)
			xtables_error(PARAMETER_PROBLEM,
				   "Invalid port:port syntax - use dash\n");

		dash = strchr(colon, '-');
		if (!dash) {
			range->min_proto.tcp.port
				= range->max_proto.tcp.port
				= htons(port);
		} else {
			int maxport;

			maxport = atoi(dash + 1);
			if (maxport <= 0 || maxport > 65535)
				xtables_error(PARAMETER_PROBLEM,
					   "Port `%s' not valid\n", dash+1);
			if (maxport < port)
				/* People are stupid. */
				xtables_error(PARAMETER_PROBLEM,
					   "Port range `%s' funky\n", colon+1);
			range->min_proto.tcp.port = htons(port);
			range->max_proto.tcp.port = htons(maxport);

			if (rev >= 2) {
				char *slash = strchr(dash, '/');
				if (slash) {
					int baseport;

					baseport = atoi(slash + 1);
					if (baseport <= 0 || baseport > 65535)
						xtables_error(PARAMETER_PROBLEM,
								 "Port `%s' not valid\n", slash+1);
					range->flags |= NF_NAT_RANGE_PROTO_OFFSET;
					range->base_proto.tcp.port = htons(baseport);
				}
			}
		}
		/* Starts with colon or [] colon? No IP info...*/
		if (colon == arg || colon == arg+2) {
			free(arg);
			return;
		}
		*colon = '\0';
	}

	range->flags |= NF_NAT_RANGE_MAP_IPS;
	dash = strchr(start, '-');
	if (colon && dash && dash > colon)
		dash = NULL;

	if (dash)
		*dash = '\0';

	ip = xtables_numeric_to_ip6addr(start);
	if (!ip)
		xtables_error(PARAMETER_PROBLEM, "Bad IP address \"%s\"\n",
			      start);
	range->min_addr.in6 = *ip;
	if (dash) {
		ip = xtables_numeric_to_ip6addr(dash + 1);
		if (!ip)
			xtables_error(PARAMETER_PROBLEM, "Bad IP address \"%s\"\n",
				      dash+1);
		range->max_addr.in6 = *ip;
	} else
		range->max_addr = range->min_addr;

	free(arg);
	return;
}

static void _DNAT_parse(struct xt_option_call *cb,
		struct nf_nat_range2 *range, int rev)
{
	const struct ip6t_entry *entry = cb->xt_entry;
	int portok;

	if (entry->ipv6.proto == IPPROTO_TCP ||
	    entry->ipv6.proto == IPPROTO_UDP ||
	    entry->ipv6.proto == IPPROTO_SCTP ||
	    entry->ipv6.proto == IPPROTO_DCCP ||
	    entry->ipv6.proto == IPPROTO_ICMP)
		portok = 1;
	else
		portok = 0;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_TO_DEST:
		if (cb->xflags & F_X_TO_DEST) {
			xtables_error(PARAMETER_PROBLEM,
				      "DNAT: Multiple --to-destination not supported");
		}
		parse_to(cb->arg, portok, range, rev);
		cb->xflags |= F_X_TO_DEST;
		break;
	case O_PERSISTENT:
		range->flags |= NF_NAT_RANGE_PERSISTENT;
		break;
	}
}

static void DNAT_parse(struct xt_option_call *cb)
{
	struct nf_nat_range *range_v1 = (void *)cb->data;
	struct nf_nat_range2 range = {};

	memcpy(&range, range_v1, sizeof(*range_v1));
	_DNAT_parse(cb, &range, 1);
	memcpy(range_v1, &range, sizeof(*range_v1));
}

static void DNAT_parse_v2(struct xt_option_call *cb)
{
	_DNAT_parse(cb, (struct nf_nat_range2 *)cb->data, 2);
}

static void _DNAT_fcheck(struct xt_fcheck_call *cb, unsigned int *flags)
{
	static const unsigned int f = F_TO_DEST | F_RANDOM;

	if ((cb->xflags & f) == f)
		*flags |= NF_NAT_RANGE_PROTO_RANDOM;
}

static void DNAT_fcheck(struct xt_fcheck_call *cb)
{
	_DNAT_fcheck(cb, &((struct nf_nat_range *)cb->data)->flags);
}

static void DNAT_fcheck_v2(struct xt_fcheck_call *cb)
{
	_DNAT_fcheck(cb, &((struct nf_nat_range2 *)cb->data)->flags);
}

static void print_range(const struct nf_nat_range2 *range, int rev)
{
	if (range->flags & NF_NAT_RANGE_MAP_IPS) {
		if (range->flags & NF_NAT_RANGE_PROTO_SPECIFIED)
			printf("[");
		printf("%s", xtables_ip6addr_to_numeric(&range->min_addr.in6));
		if (memcmp(&range->min_addr, &range->max_addr,
			   sizeof(range->min_addr)))
			printf("-%s", xtables_ip6addr_to_numeric(&range->max_addr.in6));
		if (range->flags & NF_NAT_RANGE_PROTO_SPECIFIED)
			printf("]");
	}
	if (range->flags & NF_NAT_RANGE_PROTO_SPECIFIED) {
		printf(":");
		printf("%hu", ntohs(range->min_proto.tcp.port));
		if (range->max_proto.tcp.port != range->min_proto.tcp.port)
			printf("-%hu", ntohs(range->max_proto.tcp.port));
		if (rev >= 2 && (range->flags & NF_NAT_RANGE_PROTO_OFFSET))
			printf("/%hu", ntohs(range->base_proto.tcp.port));
	}
}

static void _DNAT_print(const struct nf_nat_range2 *range, int rev)
{
	printf(" to:");
	print_range(range, rev);
	if (range->flags & NF_NAT_RANGE_PROTO_RANDOM)
		printf(" random");
	if (range->flags & NF_NAT_RANGE_PERSISTENT)
		printf(" persistent");
}

static void DNAT_print(const void *ip, const struct xt_entry_target *target,
                       int numeric)
{
	const struct nf_nat_range *range_v1 = (const void *)target->data;
	struct nf_nat_range2 range = {};

	memcpy(&range, range_v1, sizeof(*range_v1));
	_DNAT_print(&range, 1);
}

static void DNAT_print_v2(const void *ip, const struct xt_entry_target *target,
                          int numeric)
{
	_DNAT_print((const struct nf_nat_range2 *)target->data, 2);
}

static void _DNAT_save(const struct nf_nat_range2 *range, int rev)
{
	printf(" --to-destination ");
	print_range(range, rev);
	if (range->flags & NF_NAT_RANGE_PROTO_RANDOM)
		printf(" --random");
	if (range->flags & NF_NAT_RANGE_PERSISTENT)
		printf(" --persistent");
}

static void DNAT_save(const void *ip, const struct xt_entry_target *target)
{
	const struct nf_nat_range *range_v1 = (const void *)target->data;
	struct nf_nat_range2 range = {};

	memcpy(&range, range_v1, sizeof(*range_v1));
	_DNAT_save(&range, 1);
}

static void DNAT_save_v2(const void *ip, const struct xt_entry_target *target)
{
	_DNAT_save((const struct nf_nat_range2 *)target->data, 2);
}

static void print_range_xlate(const struct nf_nat_range2 *range,
			      struct xt_xlate *xl, int rev)
{
	bool proto_specified = range->flags & NF_NAT_RANGE_PROTO_SPECIFIED;

	if (range->flags & NF_NAT_RANGE_MAP_IPS) {
		xt_xlate_add(xl, "%s%s%s",
			     proto_specified ? "[" : "",
			     xtables_ip6addr_to_numeric(&range->min_addr.in6),
			     proto_specified ? "]" : "");

		if (memcmp(&range->min_addr, &range->max_addr,
			   sizeof(range->min_addr))) {
			xt_xlate_add(xl, "-%s%s%s",
				     proto_specified ? "[" : "",
				     xtables_ip6addr_to_numeric(&range->max_addr.in6),
				     proto_specified ? "]" : "");
		}
	}
	if (proto_specified) {
		xt_xlate_add(xl, ":%hu", ntohs(range->min_proto.tcp.port));

		if (range->max_proto.tcp.port != range->min_proto.tcp.port)
			xt_xlate_add(xl, "-%hu",
				   ntohs(range->max_proto.tcp.port));
	}
}

static int _DNAT_xlate(struct xt_xlate *xl,
		      const struct nf_nat_range2 *range, int rev)
{
	bool sep_need = false;
	const char *sep = " ";

	xt_xlate_add(xl, "dnat to ");
	print_range_xlate(range, xl, rev);
	if (range->flags & NF_NAT_RANGE_PROTO_RANDOM) {
		xt_xlate_add(xl, " random");
		sep_need = true;
	}
	if (range->flags & NF_NAT_RANGE_PERSISTENT) {
		if (sep_need)
			sep = ",";
		xt_xlate_add(xl, "%spersistent", sep);
	}

	return 1;
}

static int DNAT_xlate(struct xt_xlate *xl,
		      const struct xt_xlate_tg_params *params)
{
	const struct nf_nat_range *range_v1 = (const void *)params->target->data;
	struct nf_nat_range2 range = {};

	memcpy(&range, range_v1, sizeof(*range_v1));
	_DNAT_xlate(xl, &range, 1);

	return 1;
}

static int DNAT_xlate_v2(struct xt_xlate *xl,
		      const struct xt_xlate_tg_params *params)
{
	_DNAT_xlate(xl, (const struct nf_nat_range2 *)params->target->data, 2);

	return 1;
}

static struct xtables_target dnat_tg_reg[] = {
	{
		.name		= "DNAT",
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_IPV6,
		.revision	= 1,
		.size		= XT_ALIGN(sizeof(struct nf_nat_range)),
		.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_range)),
		.help		= DNAT_help,
		.print		= DNAT_print,
		.save		= DNAT_save,
		.x6_parse	= DNAT_parse,
		.x6_fcheck	= DNAT_fcheck,
		.x6_options	= DNAT_opts,
		.xlate		= DNAT_xlate,
	},
	{
		.name		= "DNAT",
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_IPV6,
		.revision	= 2,
		.size		= XT_ALIGN(sizeof(struct nf_nat_range2)),
		.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_range2)),
		.help		= DNAT_help_v2,
		.print		= DNAT_print_v2,
		.save		= DNAT_save_v2,
		.x6_parse	= DNAT_parse_v2,
		.x6_fcheck	= DNAT_fcheck_v2,
		.x6_options	= DNAT_opts,
		.xlate		= DNAT_xlate_v2,
	},
};

void _init(void)
{
	xtables_register_targets(dnat_tg_reg, ARRAY_SIZE(dnat_tg_reg));
}
