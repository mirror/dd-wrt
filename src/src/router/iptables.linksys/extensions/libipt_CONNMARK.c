/* Shared library add-on to iptables to add CONNMARK target support. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <iptables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_CONNMARK.h>


/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"CONNMARK target v%s options:\n"
"  --set-mark value              Set conntrack mark value\n"
"  --save-mark                   Save the packet nfmark on the connection\n"
"  --restore-mark                Restore saved nfmark value\n"
"\n",
IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "set-mark", 1, 0, '1' },
	{ "save-mark", 0, 0, '2' },
	{ "restore-mark", 0, 0, '3' },
	{ 0 }
};

/* Initialize the target. */
static void
init(struct ipt_entry_target *t, unsigned int *nfcache)
{
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      struct ipt_entry_target **target)
{
	struct ipt_connmark_target_info *markinfo
		= (struct ipt_connmark_target_info *)(*target)->data;

	switch (c) {
		char *end;
	case '1':
		markinfo->mode = IPT_CONNMARK_SET;
		markinfo->mark = strtoul(optarg, &end, 0);
		if (*end != '\0' || end == optarg)
			exit_error(PARAMETER_PROBLEM, "Bad MARK value `%s'", optarg);
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
			           "CONNMARK target: Can't specify --set-mark twice");
		*flags = 1;
		break;
	case '2':
		markinfo->mode = IPT_CONNMARK_SAVE;
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
			           "CONNMARK target: Can't specify --save-mark twice");
		*flags = 1;
		break;
	case '3':
		markinfo->mode = IPT_CONNMARK_RESTORE;
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
			           "CONNMARK target: Can't specify --restore-mark twice");
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
		           "CONNMARK target: Parameter --set-mark is required");
}

static void
print_mark(unsigned long mark, int numeric)
{
	printf("0x%lx ", mark);
}

/* Prints out the targinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_target *target,
      int numeric)
{
	const struct ipt_connmark_target_info *markinfo =
		(const struct ipt_connmark_target_info *)target->data;
	switch (markinfo->mode) {
	case IPT_CONNMARK_SET:
	    printf("CONNMARK set ");
	    print_mark(markinfo->mark, numeric);
	    break;
	case IPT_CONNMARK_SAVE:
	    printf("CONNMARK save ");
	    break;
	case IPT_CONNMARK_RESTORE:
	    printf("CONNMARK restore ");
	    break;
	default:
	    printf("ERROR: UNKNOWN CONNMARK MODE ");
	    break;
	}
}

/* Saves the union ipt_targinfo in parsable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_target *target)
{
	const struct ipt_connmark_target_info *markinfo =
		(const struct ipt_connmark_target_info *)target->data;

	switch (markinfo->mode) {
	case IPT_CONNMARK_SET:
	    printf("--set-mark 0x%lx ", markinfo->mark);
	    break;
	case IPT_CONNMARK_SAVE:
	    printf("--save-mark ");
	    break;
	case IPT_CONNMARK_RESTORE:
	    printf("--restore-mark ");
	    break;
	default:
	    printf("ERROR: UNKNOWN CONNMARK MODE ");
	    break;
	}
}

static
struct iptables_target mark
= { NULL,
    "CONNMARK",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_connmark_target_info)),
    IPT_ALIGN(sizeof(struct ipt_connmark_target_info)),
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
	register_target(&mark);
}
