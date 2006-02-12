/* Shared library add-on to iptables to add LOG support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <getopt.h>
#include <iptables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_LOG.h>

#define LOG_DEFAULT_LEVEL LOG_WARNING

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"LOG v%s options:\n"
" --log-level level		Level of logging (numeric or see syslog.conf)\n"
" --log-prefix prefix		Prefix log messages with this prefix.\n\n"
" --log-tcp-sequence		Log TCP sequence numbers.\n\n"
" --log-tcp-options		Log TCP options.\n\n"
" --log-ip-options		Log IP options.\n\n",
IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "log-level", 1, 0, '!' },
	{ "log-prefix", 1, 0, '#' },
	{ "log-tcp-sequence", 0, 0, '1' },
	{ "log-tcp-options", 0, 0, '2' },
	{ "log-ip-options", 0, 0, '3' },
	{ 0 }
};

/* Initialize the target. */
static void
init(struct ipt_entry_target *t, unsigned int *nfcache)
{
	struct ipt_log_info *loginfo = (struct ipt_log_info *)t->data;

	loginfo->level = LOG_DEFAULT_LEVEL;

	/* Can't cache this */
	*nfcache |= NFC_UNKNOWN;
}

struct ipt_log_names {
	const char *name;
	unsigned int level;
};

static struct ipt_log_names ipt_log_names[]
= { { "alert", LOG_ALERT },
    { "crit", LOG_CRIT },
    { "debug", LOG_DEBUG },
    { "emerg", LOG_EMERG },
    { "error", LOG_ERR },		/* DEPRECATED */
    { "info", LOG_INFO },
    { "notice", LOG_NOTICE },
    { "panic", LOG_EMERG },		/* DEPRECATED */
    { "warning", LOG_WARNING }
};

static u_int8_t
parse_level(const char *level)
{
	unsigned int lev = -1;
	unsigned int set = 0;

	if (string_to_number(level, 0, 7, &lev) == -1) {
		unsigned int i = 0;

		for (i = 0;
		     i < sizeof(ipt_log_names) / sizeof(struct ipt_log_names);
		     i++) {
			if (strncasecmp(level, ipt_log_names[i].name,
					strlen(level)) == 0) {
				if (set++)
					exit_error(PARAMETER_PROBLEM,
						   "log-level `%s' ambiguous",
						   level);
				lev = ipt_log_names[i].level;
			}
		}

		if (!set)
			exit_error(PARAMETER_PROBLEM,
				   "log-level `%s' unknown", level);
	}

	return (u_int8_t)lev;
}

#define IPT_LOG_OPT_LEVEL 0x01
#define IPT_LOG_OPT_PREFIX 0x02
#define IPT_LOG_OPT_TCPSEQ 0x04
#define IPT_LOG_OPT_TCPOPT 0x08
#define IPT_LOG_OPT_IPOPT 0x10

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      struct ipt_entry_target **target)
{
	struct ipt_log_info *loginfo = (struct ipt_log_info *)(*target)->data;

	switch (c) {
	case '!':
		if (*flags & IPT_LOG_OPT_LEVEL)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify --log-level twice");

		if (check_inverse(optarg, &invert, NULL, 0))
			exit_error(PARAMETER_PROBLEM,
				   "Unexpected `!' after --log-level");

		loginfo->level = parse_level(optarg);
		*flags |= IPT_LOG_OPT_LEVEL;
		break;

	case '#':
		if (*flags & IPT_LOG_OPT_PREFIX)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify --log-prefix twice");

		if (check_inverse(optarg, &invert, NULL, 0))
			exit_error(PARAMETER_PROBLEM,
				   "Unexpected `!' after --log-prefix");

		if (strlen(optarg) > sizeof(loginfo->prefix) - 1)
			exit_error(PARAMETER_PROBLEM,
				   "Maximum prefix length %u for --log-prefix",
				   sizeof(loginfo->prefix) - 1);

		strcpy(loginfo->prefix, optarg);
		*flags |= IPT_LOG_OPT_PREFIX;
		break;

	case '1':
		if (*flags & IPT_LOG_OPT_TCPSEQ)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify --log-tcp-sequence "
				   "twice");

		loginfo->logflags |= IPT_LOG_TCPSEQ;
		*flags |= IPT_LOG_OPT_TCPSEQ;
		break;

	case '2':
		if (*flags & IPT_LOG_OPT_TCPOPT)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify --log-tcp-options twice");

		loginfo->logflags |= IPT_LOG_TCPOPT;
		*flags |= IPT_LOG_OPT_TCPOPT;
		break;

	case '3':
		if (*flags & IPT_LOG_OPT_IPOPT)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify --log-ip-options twice");

		loginfo->logflags |= IPT_LOG_IPOPT;
		*flags |= IPT_LOG_OPT_IPOPT;
		break;

	default:
		return 0;
	}

	return 1;
}

/* Final check; nothing. */
static void final_check(unsigned int flags)
{
}

/* Prints out the targinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_target *target,
      int numeric)
{
	const struct ipt_log_info *loginfo
		= (const struct ipt_log_info *)target->data;
	unsigned int i = 0;

	printf("LOG ");
	if (numeric)
		printf("flags %u level %u ",
		       loginfo->logflags, loginfo->level);
	else {
		for (i = 0;
		     i < sizeof(ipt_log_names) / sizeof(struct ipt_log_names);
		     i++) {
			if (loginfo->level == ipt_log_names[i].level) {
				printf("level %s ", ipt_log_names[i].name);
				break;
			}
		}
		if (i == sizeof(ipt_log_names) / sizeof(struct ipt_log_names))
			printf("UNKNOWN level %u ", loginfo->level);
		if (loginfo->logflags & IPT_LOG_TCPSEQ)
			printf("tcp-sequence ");
		if (loginfo->logflags & IPT_LOG_TCPOPT)
			printf("tcp-options ");
		if (loginfo->logflags & IPT_LOG_IPOPT)
			printf("ip-options ");
		if (loginfo->logflags & ~(IPT_LOG_MASK))
			printf("unknown-flags ");
	}

	if (strcmp(loginfo->prefix, "") != 0)
		printf("prefix `%s' ", loginfo->prefix);
}

/* Saves the union ipt_targinfo in parsable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_target *target)
{
	const struct ipt_log_info *loginfo
		= (const struct ipt_log_info *)target->data;

	if (strcmp(loginfo->prefix, "") != 0)
		printf("--log-prefix \"%s\" ", loginfo->prefix);

	if (loginfo->level != LOG_DEFAULT_LEVEL)
		printf("--log-level %u ", loginfo->level);

	if (loginfo->logflags & IPT_LOG_TCPSEQ)
		printf("--log-tcp-sequence ");
	if (loginfo->logflags & IPT_LOG_TCPOPT)
		printf("--log-tcp-options ");
	if (loginfo->logflags & IPT_LOG_IPOPT)
		printf("--log-ip-options ");
}

static
struct iptables_target log
= { NULL,
    "LOG",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_log_info)),
    IPT_ALIGN(sizeof(struct ipt_log_info)),
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
	register_target(&log);
}
