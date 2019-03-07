/* Shared library add-on to iptables to add TOS target support. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <ip6tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <linux/netfilter_ipv4/ipt_TOS.h>

struct tosinfo {
	struct ip6t_entry_target t;
	struct ipt_tos_target_info tos;
};

/* TOS names and values. */
static
struct TOS_value
{
	unsigned char TOS;
	const char *name;
} TOS_values[] = {
	{ IPTOS_LOWDELAY,    "Minimize-Delay" },
	{ IPTOS_THROUGHPUT,  "Maximize-Throughput" },
	{ IPTOS_RELIABILITY, "Maximize-Reliability" },
	{ IPTOS_MINCOST,     "Minimize-Cost" },
	{ IPTOS_NORMALSVC,   "Normal-Service" },
};


struct xt_tos_target_info {
	unsigned char tos_value;
	unsigned char tos_mask;
};

/* Function which prints out usage message. */
static void
help(void)
{
	unsigned int i;

	printf(
"TOS target v%s options:\n"
"  --set-tos value                   Set Type of Service field to one of the\n"
"                                following numeric or descriptive values:\n",
IPTABLES_VERSION);

	for (i = 0; i < sizeof(TOS_values)/sizeof(struct TOS_value);i++)
		printf("                                     %s %u (0x%02x)\n",
		       TOS_values[i].name,
                       TOS_values[i].TOS,
                       TOS_values[i].TOS);
	fputc('\n', stdout);
}

static struct option opts[] = {
	{ "set-tos", 1, 0, '1' },
	{ 0 }
};


/* Initialize the target. */
static void
init(struct ip6t_entry_target *t, unsigned int *nfcache)
{
}

static void
parse_tos_v1(const char *s, struct ipt_tos_target_info *info)
{
	unsigned int i, tos;

	if (string_to_number(s, 0, 255, &tos) != -1) {
		if (tos == IPTOS_LOWDELAY
		    || tos == IPTOS_THROUGHPUT
		    || tos == IPTOS_RELIABILITY
		    || tos == IPTOS_MINCOST
		    || tos == IPTOS_NORMALSVC) {
		    	info->tos = (u_int8_t )tos;
		    	return;
		}
	} else {
		for (i = 0; i<sizeof(TOS_values)/sizeof(struct TOS_value); i++)
			if (strcasecmp(s,TOS_values[i].name) == 0) {
				info->tos = TOS_values[i].TOS;
				return;
			}
	}
	exit_error(PARAMETER_PROBLEM, "Bad TOS value `%s'", s);
}

static void
parse_tos_v2(const char *s, struct xt_tos_target_info *info)
{
	unsigned int i, tos;
	info->tos_mask = 0xff;
	if (string_to_number(s, 0, 255, &tos) != -1) {
		info->tos_value = tos;
		return;
	} else {
		for (i = 0; i<sizeof(TOS_values)/sizeof(struct TOS_value); i++)
			if (strcasecmp(s,TOS_values[i].name) == 0) {
				info->tos_value = TOS_values[i].TOS;
				return;
			}
	} 
	exit_error(PARAMETER_PROBLEM, "Bad TOS value `%s'", s);
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse_v2(int c, char **argv, int invert, unsigned int *flags,
      const struct ip6t_entry *entry,
      struct ip6t_entry_target **target)
{
	struct xt_tos_target_info *tosinfo
		= (struct xt_tos_target_info *)(*target)->data;

	switch (c) {
	case '1':
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
			           "TOS target: Cant specify --set-tos twice");
		parse_tos_v2(optarg, tosinfo);
		*flags = 1;
		break;

	default:
		return 0;
	}

	return 1;
}

static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ip6t_entry *entry,
      struct ip6t_entry_target **target)
{
	struct ipt_tos_target_info *tosinfo
		= (struct ipt_tos_target_info *)(*target)->data;

	switch (c) {
	case '1':
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
			           "TOS target: Cant specify --set-tos twice");
		parse_tos_v1(optarg, tosinfo);
		*flags = 1;
		break;

	default:
		return 0;
	}

	return 1;
}

static void
final_check(unsigned int flags)
{
	if (!flags)
		exit_error(PARAMETER_PROBLEM,
		           "TOS target: Parameter --set-tos is required");
}

static void
print_tos(u_int8_t tos, int numeric)
{
	unsigned int i;

	if (!numeric) {
		for (i = 0; i<sizeof(TOS_values)/sizeof(struct TOS_value); i++)
			if (TOS_values[i].TOS == tos) {
				printf("%s ", TOS_values[i].name);
				return;
			}
	}
	printf("0x%02x ", tos);
}

/* Prints out the targinfo. */
static void
print(const struct ip6t_ip6 *ip,
      const struct ip6t_entry_target *target,
      int numeric)
{
	const struct ipt_tos_target_info *tosinfo =
		(const struct ipt_tos_target_info *)target->data;
	printf("TOS set ");
	print_tos(tosinfo->tos, numeric);
}

/* Saves the union ipt_targinfo in parsable form to stdout. */
static void
save(const struct ip6t_ip6 *ip, const struct ip6t_entry_target *target)
{
	const struct ipt_tos_target_info *tosinfo =
		(const struct ipt_tos_target_info *)target->data;

	printf("--set-tos 0x%02x ", tosinfo->tos);
}

static struct ip6tables_target tos_v0 = {
	.next		= NULL,
	.name		= "TOS",
	.version	= IPTABLES_VERSION,
	.revision	= 0,
	.size		= IP6T_ALIGN(sizeof(struct ipt_tos_target_info)),
	.userspacesize	= IP6T_ALIGN(sizeof(struct ipt_tos_target_info)),
//	.help		= &help,
	.init		= &init,
	.parse		= &parse,
	.final_check	= &final_check,
	.print		= &print,
	.save		= &save,
	.extra_opts	= opts
};

static struct ip6tables_target tos_v1 = {
	.next		= NULL,
	.name		= "TOS",
	.version	= IPTABLES_VERSION,
	.revision	= 1,
	.size		= IP6T_ALIGN(sizeof(struct xt_tos_target_info)),
	.userspacesize	= IP6T_ALIGN(sizeof(struct xt_tos_target_info)),
//	.help		= &help,
	.init		= &init,
	.parse		= &parse_v2,
	.final_check	= &final_check,
	.print		= &print,
	.save		= &save,
	.extra_opts	= opts
};

void _init(void)
{
	register_target6(&tos_v0);
	register_target6(&tos_v1);
}
