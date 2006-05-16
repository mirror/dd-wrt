/* Shared library add-on to iptables to add ULOG support.
 * 
 * (C) 2000 by Harald Welte <laforge@gnumonks.org>
 *
 * multipart netlink support based on ideas by Sebastian Zander 
 * 						<zander@fokus.gmd.de>
 *
 * This software is released under the terms of GNU GPL
 * 
 * libipt_ULOG.c,v 1.7 2001/01/30 11:55:02 laforge Exp
 */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <getopt.h>
#include <iptables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_ULOG.h>

#define ULOG_DEFAULT_NLGROUP 1
#define ULOG_DEFAULT_QTHRESHOLD 1


void print_groups(unsigned int gmask)
{
	int b;
	unsigned int test;

	for (b = 31; b >= 0; b--) {
		test = (1 << b);
		if (gmask & test)
			printf("%d ", b + 1);
	}
}

/* Function which prints out usage message. */
static void help(void)
{
	printf("ULOG v%s options:\n"
	       " --ulog-nlgroup nlgroup		NETLINK group used for logging\n"
	       " --ulog-cprange size		Bytes of each packet to be passed\n"
	       " --ulog-qthreshold		Threshold of in-kernel queue\n"
	       " --ulog-prefix prefix		Prefix log messages with this prefix.\n\n",
	       IPTABLES_VERSION);
}

static struct option opts[] = {
	{"ulog-nlgroup", 1, 0, '!'},
	{"ulog-prefix", 1, 0, '#'},
	{"ulog-cprange", 1, 0, 'A'},
	{"ulog-qthreshold", 1, 0, 'B'},
	{0}
};

/* Initialize the target. */
static void init(struct ipt_entry_target *t, unsigned int *nfcache)
{
	struct ipt_ulog_info *loginfo = (struct ipt_ulog_info *) t->data;

	loginfo->nl_group = ULOG_DEFAULT_NLGROUP;
	loginfo->qthreshold = ULOG_DEFAULT_QTHRESHOLD;

	/* Can't cache this */
	*nfcache |= NFC_UNKNOWN;
}

#define IPT_LOG_OPT_NLGROUP 0x01
#define IPT_LOG_OPT_PREFIX 0x02
#define IPT_LOG_OPT_CPRANGE 0x04
#define IPT_LOG_OPT_QTHRESHOLD 0x08

/* Function which parses command options; returns true if it
   ate an option */
static int parse(int c, char **argv, int invert, unsigned int *flags,
		 const struct ipt_entry *entry,
		 struct ipt_entry_target **target)
{
	struct ipt_ulog_info *loginfo =
	    (struct ipt_ulog_info *) (*target)->data;
	int group_d;

	switch (c) {
	case '!':
		if (*flags & IPT_LOG_OPT_NLGROUP)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify --ulog-nlgroup twice");

		if (check_inverse(optarg, &invert, NULL, 0))
			exit_error(PARAMETER_PROBLEM,
				   "Unexpected `!' after --ulog-nlgroup");
		group_d = atoi(optarg);
		if (group_d > 32 || group_d < 1)
			exit_error(PARAMETER_PROBLEM,
				   "--ulog-nlgroup has to be between 1 and 32");

		loginfo->nl_group = (1 << (group_d - 1));

		*flags |= IPT_LOG_OPT_NLGROUP;
		break;

	case '#':
		if (*flags & IPT_LOG_OPT_PREFIX)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify --ulog-prefix twice");

		if (check_inverse(optarg, &invert, NULL, 0))
			exit_error(PARAMETER_PROBLEM,
				   "Unexpected `!' after --ulog-prefix");

		if (strlen(optarg) > sizeof(loginfo->prefix) - 1)
			exit_error(PARAMETER_PROBLEM,
				   "Maximum prefix length %u for --ulog-prefix",
				   sizeof(loginfo->prefix) - 1);

		strcpy(loginfo->prefix, optarg);
		*flags |= IPT_LOG_OPT_PREFIX;
		break;
	case 'A':
		if (*flags & IPT_LOG_OPT_CPRANGE)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify --ulog-cprange twice");
		if (atoi(optarg) < 0)
			exit_error(PARAMETER_PROBLEM,
				   "Negative copy range?");
		loginfo->copy_range = atoi(optarg);
		*flags |= IPT_LOG_OPT_CPRANGE;
		break;
	case 'B':
		if (*flags & IPT_LOG_OPT_QTHRESHOLD)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify --ulog-qthreshold twice");
		if (atoi(optarg) < 1)
			exit_error(PARAMETER_PROBLEM,
				   "Negative or zero queue threshold ?");
		if (atoi(optarg) > ULOG_MAX_QLEN)
			exit_error(PARAMETER_PROBLEM,
				   "Maximum queue length exceeded");
		loginfo->qthreshold = atoi(optarg);
		*flags |= IPT_LOG_OPT_QTHRESHOLD;
		break;
	}
	return 1;
}

/* Final check; nothing. */
static void final_check(unsigned int flags)
{
}

/* Saves the union ipt_targinfo in parsable form to stdout. */
static void save(const struct ipt_ip *ip,
		 const struct ipt_entry_target *target)
{
	const struct ipt_ulog_info *loginfo
	    = (const struct ipt_ulog_info *) target->data;

	if (strcmp(loginfo->prefix, "") != 0)
		printf("--ulog-prefix %s ", loginfo->prefix);

	if (loginfo->nl_group != ULOG_DEFAULT_NLGROUP) {
		printf("--ulog-nlgroup ");
		print_groups(loginfo->nl_group);
		printf("\n");
	}
	if (loginfo->copy_range)
		printf("--ulog-cprange %d ", loginfo->copy_range);

	if (loginfo->qthreshold != ULOG_DEFAULT_QTHRESHOLD)
		printf("--ulog-qthreshold %d ", loginfo->qthreshold);
}

/* Prints out the targinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_target *target, int numeric)
{
	const struct ipt_ulog_info *loginfo
	    = (const struct ipt_ulog_info *) target->data;

	printf("ULOG ");
	printf("copy_range %d nlgroup ", loginfo->copy_range);
	print_groups(loginfo->nl_group);
	if (strcmp(loginfo->prefix, "") != 0)
		printf("prefix `%s' ", loginfo->prefix);
	printf("queue_threshold %d ", loginfo->qthreshold);
}

static
struct iptables_target ulog = { NULL,
	"ULOG",
	IPTABLES_VERSION,
	IPT_ALIGN(sizeof(struct ipt_ulog_info)),
	IPT_ALIGN(sizeof(struct ipt_ulog_info)),
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
	register_target(&ulog);
}
