#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <xtables.h>
#include <iptables.h> /* get_kernel_version */
#include <limits.h> /* INT_MAX in ip_tables.h */
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter/nf_nat.h>

enum {
	O_TO_DEST = 0,
	O_RANDOM,
	O_PERSISTENT,
	O_X_TO_DEST, /* hidden flag */
	F_TO_DEST   = 1 << O_TO_DEST,
	F_RANDOM    = 1 << O_RANDOM,
	F_X_TO_DEST = 1 << O_X_TO_DEST,
};

/* Dest NAT data consists of a multi-range, indicating where to map
   to. */
struct ipt_natinfo
{
	struct xt_entry_target t;
	struct nf_nat_ipv4_multi_range_compat mr;
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

static struct ipt_natinfo *
append_range(struct ipt_natinfo *info, const struct nf_nat_ipv4_range *range)
{
	unsigned int size;

	/* One rangesize already in struct ipt_natinfo */
	size = XT_ALIGN(sizeof(*info) + info->mr.rangesize * sizeof(*range));

	info = realloc(info, size);
	if (!info)
		xtables_error(OTHER_PROBLEM, "Out of memory\n");

	info->t.u.target_size = size;
	info->mr.range[info->mr.rangesize] = *range;
	info->mr.rangesize++;

	return info;
}

/* Ranges expected in network order. */
static struct xt_entry_target *
parse_to(const char *orig_arg, int portok, struct ipt_natinfo *info)
{
	struct nf_nat_ipv4_range range;
	char *arg, *colon, *dash, *error;
	const struct in_addr *ip;

	arg = strdup(orig_arg);
	if (arg == NULL)
		xtables_error(RESOURCE_PROBLEM, "strdup");
	memset(&range, 0, sizeof(range));
	colon = strchr(arg, ':');

	if (colon) {
		int port;

		if (!portok)
			xtables_error(PARAMETER_PROBLEM,
				   "Need TCP, UDP, SCTP or DCCP with port specification");

		range.flags |= NF_NAT_RANGE_PROTO_SPECIFIED;

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
			range.min.tcp.port
				= range.max.tcp.port
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
			range.min.tcp.port = htons(port);
			range.max.tcp.port = htons(maxport);
		}
		/* Starts with a colon? No IP info...*/
		if (colon == arg) {
			free(arg);
			return &(append_range(info, &range)->t);
		}
		*colon = '\0';
	}

	range.flags |= NF_NAT_RANGE_MAP_IPS;
	dash = strchr(arg, '-');
	if (colon && dash && dash > colon)
		dash = NULL;

	if (dash)
		*dash = '\0';

	ip = xtables_numeric_to_ipaddr(arg);
	if (!ip)
		xtables_error(PARAMETER_PROBLEM, "Bad IP address \"%s\"\n",
			   arg);
	range.min_ip = ip->s_addr;
	if (dash) {
		ip = xtables_numeric_to_ipaddr(dash+1);
		if (!ip)
			xtables_error(PARAMETER_PROBLEM, "Bad IP address \"%s\"\n",
				   dash+1);
		range.max_ip = ip->s_addr;
	} else
		range.max_ip = range.min_ip;

	free(arg);
	return &(append_range(info, &range)->t);
}

static void DNAT_parse(struct xt_option_call *cb)
{
	const struct ipt_entry *entry = cb->xt_entry;
	struct ipt_natinfo *info = (void *)(*cb->target);
	int portok;

	if (entry->ip.proto == IPPROTO_TCP
	    || entry->ip.proto == IPPROTO_UDP
	    || entry->ip.proto == IPPROTO_SCTP
	    || entry->ip.proto == IPPROTO_DCCP
	    || entry->ip.proto == IPPROTO_ICMP)
		portok = 1;
	else
		portok = 0;

	xtables_option_parse(cb);
	switch (cb->entry->id) {
	case O_TO_DEST:
		if (cb->xflags & F_X_TO_DEST) {
			if (!kernel_version)
				get_kernel_version();
			if (kernel_version > LINUX_VERSION(2, 6, 10))
				xtables_error(PARAMETER_PROBLEM,
					   "DNAT: Multiple --to-destination not supported");
		}
		*cb->target = parse_to(cb->arg, portok, info);
		cb->xflags |= F_X_TO_DEST;
		break;
	case O_PERSISTENT:
		info->mr.range[0].flags |= NF_NAT_RANGE_PERSISTENT;
		break;
	}
}

static void DNAT_fcheck(struct xt_fcheck_call *cb)
{
	static const unsigned int f = F_TO_DEST | F_RANDOM;
	struct nf_nat_ipv4_multi_range_compat *mr = cb->data;

	if ((cb->xflags & f) == f)
		mr->range[0].flags |= NF_NAT_RANGE_PROTO_RANDOM;
}

static void print_range(const struct nf_nat_ipv4_range *r)
{
	if (r->flags & NF_NAT_RANGE_MAP_IPS) {
		struct in_addr a;

		a.s_addr = r->min_ip;
		printf("%s", xtables_ipaddr_to_numeric(&a));
		if (r->max_ip != r->min_ip) {
			a.s_addr = r->max_ip;
			printf("-%s", xtables_ipaddr_to_numeric(&a));
		}
	}
	if (r->flags & NF_NAT_RANGE_PROTO_SPECIFIED) {
		printf(":");
		printf("%hu", ntohs(r->min.tcp.port));
		if (r->max.tcp.port != r->min.tcp.port)
			printf("-%hu", ntohs(r->max.tcp.port));
	}
}

static void DNAT_print(const void *ip, const struct xt_entry_target *target,
                       int numeric)
{
	const struct ipt_natinfo *info = (const void *)target;
	unsigned int i = 0;

	printf(" to:");
	for (i = 0; i < info->mr.rangesize; i++) {
		print_range(&info->mr.range[i]);
		if (info->mr.range[i].flags & NF_NAT_RANGE_PROTO_RANDOM)
			printf(" random");
		if (info->mr.range[i].flags & NF_NAT_RANGE_PERSISTENT)
			printf(" persistent");
	}
}

static void DNAT_save(const void *ip, const struct xt_entry_target *target)
{
	const struct ipt_natinfo *info = (const void *)target;
	unsigned int i = 0;

	for (i = 0; i < info->mr.rangesize; i++) {
		printf(" --to-destination ");
		print_range(&info->mr.range[i]);
		if (info->mr.range[i].flags & NF_NAT_RANGE_PROTO_RANDOM)
			printf(" --random");
		if (info->mr.range[i].flags & NF_NAT_RANGE_PERSISTENT)
			printf(" --persistent");
	}
}

static void print_range_xlate(const struct nf_nat_ipv4_range *r,
			struct xt_xlate *xl)
{
	if (r->flags & NF_NAT_RANGE_MAP_IPS) {
		struct in_addr a;

		a.s_addr = r->min_ip;
		xt_xlate_add(xl, "%s", xtables_ipaddr_to_numeric(&a));
		if (r->max_ip != r->min_ip) {
			a.s_addr = r->max_ip;
			xt_xlate_add(xl, "-%s", xtables_ipaddr_to_numeric(&a));
		}
	}
	if (r->flags & NF_NAT_RANGE_PROTO_SPECIFIED) {
		xt_xlate_add(xl, ":%hu", ntohs(r->min.tcp.port));
		if (r->max.tcp.port != r->min.tcp.port)
			xt_xlate_add(xl, "-%hu", ntohs(r->max.tcp.port));
	}
}

static int DNAT_xlate(struct xt_xlate *xl,
		      const struct xt_xlate_tg_params *params)
{
	const struct ipt_natinfo *info = (const void *)params->target;
	unsigned int i = 0;
	bool sep_need = false;
	const char *sep = " ";

	for (i = 0; i < info->mr.rangesize; i++) {
		xt_xlate_add(xl, "dnat to ");
		print_range_xlate(&info->mr.range[i], xl);
		if (info->mr.range[i].flags & NF_NAT_RANGE_PROTO_RANDOM) {
			xt_xlate_add(xl, " random");
			sep_need = true;
		}
		if (info->mr.range[i].flags & NF_NAT_RANGE_PERSISTENT) {
			if (sep_need)
				sep = ",";
			xt_xlate_add(xl, "%spersistent", sep);
		}
	}

	return 1;
}

static void
parse_to_v2(const char *orig_arg, int portok, struct nf_nat_range2 *range)
{
	char *arg, *colon, *dash, *error;
	const struct in_addr *ip;

	arg = strdup(orig_arg);
	if (arg == NULL)
		xtables_error(RESOURCE_PROBLEM, "strdup");

	colon = strchr(arg, ':');
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
			char *slash;

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

			slash = strchr(dash, '/');
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
		/* Starts with a colon? No IP info...*/
		if (colon == arg) {
			free(arg);
			return;
		}
		*colon = '\0';
	}

	range->flags |= NF_NAT_RANGE_MAP_IPS;
	dash = strchr(arg, '-');
	if (colon && dash && dash > colon)
		dash = NULL;

	if (dash)
		*dash = '\0';

	ip = xtables_numeric_to_ipaddr(arg);
	if (!ip)
		xtables_error(PARAMETER_PROBLEM, "Bad IP address \"%s\"\n",
			   arg);
	range->min_addr.in = *ip;
	if (dash) {
		ip = xtables_numeric_to_ipaddr(dash+1);
		if (!ip)
			xtables_error(PARAMETER_PROBLEM, "Bad IP address \"%s\"\n",
				   dash+1);
		range->max_addr.in = *ip;
	} else
		range->max_addr = range->min_addr;

	free(arg);
	return;
}

static void DNAT_parse_v2(struct xt_option_call *cb)
{
	const struct ipt_entry *entry = cb->xt_entry;
	struct nf_nat_range2 *range = cb->data;
	int portok;

	if (entry->ip.proto == IPPROTO_TCP
	    || entry->ip.proto == IPPROTO_UDP
	    || entry->ip.proto == IPPROTO_SCTP
	    || entry->ip.proto == IPPROTO_DCCP
	    || entry->ip.proto == IPPROTO_ICMP)
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
		parse_to_v2(cb->arg, portok, range);
		cb->xflags |= F_X_TO_DEST;
		break;
	case O_PERSISTENT:
		range->flags |= NF_NAT_RANGE_PERSISTENT;
		break;
	}
}

static void DNAT_fcheck_v2(struct xt_fcheck_call *cb)
{
	static const unsigned int f = F_TO_DEST | F_RANDOM;
	struct nf_nat_range2 *range = cb->data;

	if ((cb->xflags & f) == f)
		range->flags |= NF_NAT_RANGE_PROTO_RANDOM;
}

static void print_range_v2(const struct nf_nat_range2 *range)
{
	if (range->flags & NF_NAT_RANGE_MAP_IPS) {
		printf("%s", xtables_ipaddr_to_numeric(&range->min_addr.in));
		if (memcmp(&range->min_addr, &range->max_addr,
			   sizeof(range->min_addr)))
			printf("-%s", xtables_ipaddr_to_numeric(&range->max_addr.in));
	}
	if (range->flags & NF_NAT_RANGE_PROTO_SPECIFIED) {
		printf(":");
		printf("%hu", ntohs(range->min_proto.tcp.port));
		if (range->max_proto.tcp.port != range->min_proto.tcp.port)
			printf("-%hu", ntohs(range->max_proto.tcp.port));
		if (range->flags & NF_NAT_RANGE_PROTO_OFFSET)
			printf("/%hu", ntohs(range->base_proto.tcp.port));
	}
}

static void DNAT_print_v2(const void *ip, const struct xt_entry_target *target,
                       int numeric)
{
	const struct nf_nat_range2 *range = (const void *)target->data;

	printf(" to:");
	print_range_v2(range);
	if (range->flags & NF_NAT_RANGE_PROTO_RANDOM)
		printf(" random");
	if (range->flags & NF_NAT_RANGE_PERSISTENT)
		printf(" persistent");
}

static void DNAT_save_v2(const void *ip, const struct xt_entry_target *target)
{
	const struct nf_nat_range2 *range = (const void *)target->data;

	printf(" --to-destination ");
	print_range_v2(range);
	if (range->flags & NF_NAT_RANGE_PROTO_RANDOM)
		printf(" --random");
	if (range->flags & NF_NAT_RANGE_PERSISTENT)
		printf(" --persistent");
}

static void print_range_xlate_v2(const struct nf_nat_range2 *range,
			      struct xt_xlate *xl)
{
	if (range->flags & NF_NAT_RANGE_MAP_IPS) {
		xt_xlate_add(xl, "%s", xtables_ipaddr_to_numeric(&range->min_addr.in));
		if (memcmp(&range->min_addr, &range->max_addr,
			   sizeof(range->min_addr))) {
			xt_xlate_add(xl, "-%s", xtables_ipaddr_to_numeric(&range->max_addr.in));
		}
	}
	if (range->flags & NF_NAT_RANGE_PROTO_SPECIFIED) {
		xt_xlate_add(xl, ":%hu", ntohs(range->min_proto.tcp.port));
		if (range->max_proto.tcp.port != range->min_proto.tcp.port)
			xt_xlate_add(xl, "-%hu", ntohs(range->max_proto.tcp.port));
		if (range->flags & NF_NAT_RANGE_PROTO_OFFSET)
			xt_xlate_add(xl, ";%hu", ntohs(range->base_proto.tcp.port));
	}
}

static int DNAT_xlate_v2(struct xt_xlate *xl,
		      const struct xt_xlate_tg_params *params)
{
	const struct nf_nat_range2 *range = (const void *)params->target->data;
	bool sep_need = false;
	const char *sep = " ";

	xt_xlate_add(xl, "dnat to ");
	print_range_xlate_v2(range, xl);
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

static struct xtables_target dnat_tg_reg[] = {
	{
		.name		= "DNAT",
		.version	= XTABLES_VERSION,
		.family		= NFPROTO_IPV4,
		.revision	= 0,
		.size		= XT_ALIGN(sizeof(struct nf_nat_ipv4_multi_range_compat)),
		.userspacesize	= XT_ALIGN(sizeof(struct nf_nat_ipv4_multi_range_compat)),
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
		.family		= NFPROTO_IPV4,
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
