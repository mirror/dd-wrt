/* Shared library add-on to iptables to add customized REJECT support.
 *
 * (C) 2000 Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <iptables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_REJECT.h>

struct reject_names {
	const char *name;
	const char *alias;
	enum ipt_reject_with with;
	const char *desc;
};

static const struct reject_names reject_table[] = {
	{"icmp-net-unreachable", "net-unreach",
		IPT_ICMP_NET_UNREACHABLE, "ICMP network unreachable"},
	{"icmp-host-unreachable", "host-unreach",
		IPT_ICMP_HOST_UNREACHABLE, "ICMP host unreachable"},
	{"icmp-proto-unreachable", "proto-unreach",
		IPT_ICMP_PROT_UNREACHABLE, "ICMP protocol unreachable"},
	{"icmp-port-unreachable", "port-unreach",
		IPT_ICMP_PORT_UNREACHABLE, "ICMP port unreachable (default)"},
	{"icmp-net-prohibited", "net-prohib",
	 IPT_ICMP_NET_PROHIBITED, "ICMP network prohibited"},
	{"icmp-host-prohibited", "host-prohib",
	 IPT_ICMP_HOST_PROHIBITED, "ICMP host prohibited"},
	{"tcp-reset", "tcp-reset",
	 IPT_TCP_RESET, "TCP RST packet"}
};

static void
print_reject_types()
{
	unsigned int i;

	printf("Valid reject types:\n");

	for (i = 0; i < sizeof(reject_table)/sizeof(struct reject_names); i++) {
		printf("    %-25s\t%s\n", reject_table[i].name, reject_table[i].desc);
		printf("    %-25s\talias\n", reject_table[i].alias);
	}
	printf("\n");
}

/* Saves the union ipt_targinfo in parsable form to stdout. */

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"REJECT options:\n"
"--reject-with type              drop input packet and send back\n"
"                                a reply packet according to type:\n");

	print_reject_types();
}

static struct option opts[] = {
	{ "reject-with", 1, 0, '1' },
	{ 0 }
};

/* Allocate and initialize the target. */
static void
init(struct ipt_entry_target *t, unsigned int *nfcache)
{
	struct ipt_reject_info *reject = (struct ipt_reject_info *)t->data;

	/* default */
	reject->with = IPT_ICMP_PORT_UNREACHABLE;

	/* Can't cache this */
	*nfcache |= NFC_UNKNOWN;
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      struct ipt_entry_target **target)
{
	struct ipt_reject_info *reject = (struct ipt_reject_info *)(*target)->data;
	unsigned int limit = sizeof(reject_table)/sizeof(struct reject_names);
	unsigned int i;

	switch(c) {
	case '1':
		if (check_inverse(optarg, &invert, NULL, 0))
			exit_error(PARAMETER_PROBLEM,
				   "Unexpected `!' after --reject-with");
		for (i = 0; i < limit; i++) {
			if ((strncasecmp(reject_table[i].name, optarg, strlen(optarg)) == 0)
			    || (strncasecmp(reject_table[i].alias, optarg, strlen(optarg)) == 0)) {
				reject->with = reject_table[i].with;
				return 1;
			}
		}
		/* This due to be dropped late in 2.4 pre-release cycle --RR */
		if (strncasecmp("echo-reply", optarg, strlen(optarg)) == 0
		    || strncasecmp("echoreply", optarg, strlen(optarg)) == 0)
			fprintf(stderr, "--reject-with echo-reply no longer"
				" supported\n");
		exit_error(PARAMETER_PROBLEM, "unknown reject type `%s'",optarg);
	default:
		/* Fall through */
		break;
	}
	return 0;
}

/* Final check; nothing. */
static void final_check(unsigned int flags)
{
}

/* Prints out ipt_reject_info. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_target *target,
      int numeric)
{
	const struct ipt_reject_info *reject
		= (const struct ipt_reject_info *)target->data;
	unsigned int i;

	for (i = 0; i < sizeof(reject_table)/sizeof(struct reject_names); i++) {
		if (reject_table[i].with == reject->with)
			break;
	}
	printf("reject-with %s ", reject_table[i].name);
}

/* Saves ipt_reject in parsable form to stdout. */
static void save(const struct ipt_ip *ip, const struct ipt_entry_target *target)
{
	const struct ipt_reject_info *reject
		= (const struct ipt_reject_info *)target->data;
	unsigned int i;

	for (i = 0; i < sizeof(reject_table)/sizeof(struct reject_names); i++)
		if (reject_table[i].with == reject->with)
			break;

	printf("--reject-with %s ", reject_table[i].name);
}

static
struct iptables_target reject
= { NULL,
    "REJECT",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_reject_info)),
    IPT_ALIGN(sizeof(struct ipt_reject_info)),
    &help,
    &init,
    &parse,
    &final_check,
    &print,
    &save,
    opts
};

void _init(void)
{
	register_target(&reject);
}
