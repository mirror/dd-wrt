#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../include/ebtables_u.h"
#include <linux/netfilter_bridge/ebt_log.h>

/*
 * copied from syslog.h
 * used for the LOG target
 */
#define	LOG_EMERG	0 /* system is unusable               */
#define	LOG_ALERT	1 /* action must be taken immediately */
#define	LOG_CRIT	2 /* critical conditions              */
#define	LOG_ERR		3 /* error conditions                 */
#define	LOG_WARNING	4 /* warning conditions               */
#define	LOG_NOTICE	5 /* normal but significant condition */
#define	LOG_INFO	6 /* informational                    */
#define	LOG_DEBUG	7 /* debug-level messages             */

#define LOG_DEFAULT_LEVEL LOG_INFO

typedef struct _code {
	char *c_name;
	int c_val;
} CODE;

static CODE eight_priority[] = {
	{ "emerg", LOG_EMERG },
	{ "alert", LOG_ALERT },
	{ "crit", LOG_CRIT },
	{ "error", LOG_ERR },
	{ "warning", LOG_WARNING },
	{ "notice", LOG_NOTICE },
	{ "info", LOG_INFO },
	{ "debug", LOG_DEBUG }
};

static int name_to_loglevel(char* arg)
{
	int i;
	
	for (i = 0; i < 8; i++)
		if (!strcmp(arg, eight_priority[i].c_name))
			return eight_priority[i].c_val;
	/* return bad loglevel */
	return 9;
}

#define LOG_PREFIX '1'
#define LOG_LEVEL  '2'
#define LOG_ARP    '3'
#define LOG_IP     '4'
#define LOG_LOG    '5'
static struct option opts[] =
{
	{ "log-prefix", required_argument, 0, LOG_PREFIX },
	{ "log-level" , required_argument, 0, LOG_LEVEL  },
	{ "log-arp"   , no_argument      , 0, LOG_ARP    },
	{ "log-ip"    , no_argument      , 0, LOG_IP     },
	{ "log"       , no_argument      , 0, LOG_LOG    },
	{ 0 }
};

static void print_help()
{
	int i;

	printf(
"log options:\n"
"--log               : use this if you're not specifying anything\n"
"--log-level level   : level = [1-8] or a string\n"
"--log-prefix prefix : max. %d chars.\n"
"--log-ip            : put ip info. in the log for ip packets\n"
"--log-arp           : put (r)arp info. in the log for (r)arp packets\n"
	, EBT_LOG_PREFIX_SIZE - 1);
	printf("levels:\n");
	for (i = 0; i < 8; i++)
		printf("%d = %s\n", eight_priority[i].c_val,
		   eight_priority[i].c_name);
}

static void init(struct ebt_entry_watcher *watcher)
{
	struct ebt_log_info *loginfo = (struct ebt_log_info *)watcher->data;

	loginfo->bitmask = 0;
	loginfo->prefix[0] = '\0';
	loginfo->loglevel = LOG_NOTICE;
}

#define OPT_PREFIX 0x01
#define OPT_LEVEL  0x02
#define OPT_ARP    0x04
#define OPT_IP     0x08
#define OPT_LOG    0x10
static int parse(int c, char **argv, int argc, const struct ebt_u_entry *entry,
   unsigned int *flags, struct ebt_entry_watcher **watcher)
{
	struct ebt_log_info *loginfo = (struct ebt_log_info *)(*watcher)->data;
	long int i;
	char *end;

	switch (c) {
	case LOG_PREFIX:
		check_option(flags, OPT_PREFIX);
		if (strlen(optarg) > sizeof(loginfo->prefix) - 1)
			print_error("Prefix too long");
		strcpy(loginfo->prefix, optarg);
		break;

	case LOG_LEVEL:
		check_option(flags, OPT_LEVEL);
		i = strtol(optarg, &end, 16);
		if (*end != '\0' || i < 0 || i > 7)
			loginfo->loglevel = name_to_loglevel(optarg);
		else
			loginfo->loglevel = i;
		if (loginfo->loglevel == 9)
			print_error("Problem with the log-level");
		break;

	case LOG_IP:
		check_option(flags, OPT_IP);
		loginfo->bitmask |= EBT_LOG_IP;
		break;

	case LOG_ARP:
		check_option(flags, OPT_ARP);
		loginfo->bitmask |= EBT_LOG_ARP;
		break;

	case LOG_LOG:
		check_option(flags, OPT_LOG);
		break;
	default:
		return 0;
	}
	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_watcher *watcher, const char *name,
   unsigned int hookmask, unsigned int time)
{
	return;
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_watcher *watcher)
{
	struct ebt_log_info *loginfo = (struct ebt_log_info *)watcher->data;

	printf("--log-level %s --log-prefix \"%s\"",
		eight_priority[loginfo->loglevel].c_name,
		loginfo->prefix);
	if (loginfo->bitmask & EBT_LOG_IP)
		printf(" --log-ip");
	if (loginfo->bitmask & EBT_LOG_ARP)
		printf(" --log-arp");
	printf(" ");
}

static int compare(const struct ebt_entry_watcher *w1,
   const struct ebt_entry_watcher *w2)
{
	struct ebt_log_info *loginfo1 = (struct ebt_log_info *)w1->data;
	struct ebt_log_info *loginfo2 = (struct ebt_log_info *)w2->data;

	if (loginfo1->loglevel != loginfo2->loglevel)
		return 0;
	if (loginfo1->bitmask != loginfo2->bitmask)
		return 0;
	return !strcmp(loginfo1->prefix, loginfo2->prefix);
}

static struct ebt_u_watcher log_watcher =
{
	.name		= EBT_LOG_WATCHER,
	.size		= sizeof(struct ebt_log_info),
	.help		= print_help,
	.init		= init,
	.parse		= parse,
	.final_check	= final_check,
	.print		= print,
	.compare	= compare,
	.extra_ops	= opts,
};

static void _init(void) __attribute__ ((constructor));
static void _init(void)
{
	register_watcher(&log_watcher);
}
