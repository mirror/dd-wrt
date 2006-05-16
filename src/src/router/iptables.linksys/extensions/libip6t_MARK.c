/* Shared library add-on to iptables to add MARK target support. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <ip6tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <linux/netfilter_ipv6/ip6t_MARK.h>

struct markinfo {
	struct ip6t_entry_target t;
	struct ip6t_mark_target_info mark;
};

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"MARK target v%s options:\n"
"  --set-mark value                   Set nfmark value\n"
"\n",
IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "set-mark", 1, 0, '1' },
	{ 0 }
};

/* Initialize the target. */
static void
init(struct ip6t_entry_target *t, unsigned int *nfcache)
{
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ip6t_entry *entry,
      struct ip6t_entry_target **target)
{
	struct ip6t_mark_target_info *markinfo
		= (struct ip6t_mark_target_info *)(*target)->data;

	switch (c) {
		char *end;
	case '1':
		markinfo->mark = strtoul(optarg, &end, 0);
		if (*end != '\0' || end == optarg)
			exit_error(PARAMETER_PROBLEM, "Bad MARK value `%s'", optarg);
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
			           "MARK target: Can't specify --set-mark twice");
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
		           "MARK target: Parameter --set-mark is required");
}

static void
print_mark(unsigned long mark, int numeric)
{
	printf("0x%lx ", mark);
}

/* Prints out the targinfo. */
static void
print(const struct ip6t_ip6 *ip,
      const struct ip6t_entry_target *target,
      int numeric)
{
	const struct ip6t_mark_target_info *markinfo =
		(const struct ip6t_mark_target_info *)target->data;
	printf("MARK set ");
	print_mark(markinfo->mark, numeric);
}

/* Saves the union ipt_targinfo in parsable form to stdout. */
static void
save(const struct ip6t_ip6 *ip, const struct ip6t_entry_target *target)
{
	const struct ip6t_mark_target_info *markinfo =
		(const struct ip6t_mark_target_info *)target->data;

	printf("--set-mark 0x%lx ", markinfo->mark);
}

static
struct ip6tables_target mark
= { NULL,
    "MARK",
    IPTABLES_VERSION,
    IP6T_ALIGN(sizeof(struct ip6t_mark_target_info)),
    IP6T_ALIGN(sizeof(struct ip6t_mark_target_info)),
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
	register_target6(&mark);
}
