/*
 * Bart De Schuymer <bdschuym@pandora.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Giuseppe Longo <giuseppelng@gmail.com> adapted the original code to the
 * xtables-compat environment in 2015.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <getopt.h>
#include <xtables.h>
#include <linux/netfilter_bridge/ebt_log.h>

#define LOG_DEFAULT_LEVEL LOG_INFO

#define LOG_PREFIX '1'
#define LOG_LEVEL  '2'
#define LOG_ARP    '3'
#define LOG_IP     '4'
#define LOG_LOG    '5'
#define LOG_IP6    '6'

struct code {
	char *c_name;
	int c_val;
};

static struct code eight_priority[] = {
	{ "emerg", LOG_EMERG },
	{ "alert", LOG_ALERT },
	{ "crit", LOG_CRIT },
	{ "error", LOG_ERR },
	{ "warning", LOG_WARNING },
	{ "notice", LOG_NOTICE },
	{ "info", LOG_INFO },
	{ "debug", LOG_DEBUG }
};

static int name_to_loglevel(const char *arg)
{
	int i;

	for (i = 0; i < 8; i++)
		if (!strcmp(arg, eight_priority[i].c_name))
			return eight_priority[i].c_val;

	/* return bad loglevel */
	return 9;
}

static const struct option brlog_opts[] = {
	{ .name = "log-prefix",		.has_arg = true,  .val = LOG_PREFIX },
	{ .name = "log-level",		.has_arg = true,  .val = LOG_LEVEL  },
	{ .name = "log-arp",		.has_arg = false, .val = LOG_ARP    },
	{ .name = "log-ip",		.has_arg = false, .val = LOG_IP     },
	{ .name = "log",		.has_arg = false, .val = LOG_LOG    },
	{ .name = "log-ip6",		.has_arg = false, .val = LOG_IP6    },
	XT_GETOPT_TABLEEND,
};

static void brlog_help(void)
{
	int i;

	printf(
"log options:\n"
"--log               : use this if you're not specifying anything\n"
"--log-level level   : level = [1-8] or a string\n"
"--log-prefix prefix : max. %d chars.\n"
"--log-ip            : put ip info. in the log for ip packets\n"
"--log-arp           : put (r)arp info. in the log for (r)arp packets\n"
"--log-ip6           : put ip6 info. in the log for ip6 packets\n"
	, EBT_LOG_PREFIX_SIZE - 1);
	for (i = 0; i < 8; i++)
		printf("%d = %s\n", eight_priority[i].c_val,
				    eight_priority[i].c_name);
}

static void brlog_init(struct xt_entry_target *t)
{
	struct ebt_log_info *loginfo = (struct ebt_log_info *)t->data;

	loginfo->bitmask = 0;
	loginfo->prefix[0] = '\0';
	loginfo->loglevel = LOG_NOTICE;
}

static unsigned int log_chk_inv(int inv, unsigned int bit, const char *suffix)
{
	if (inv)
		xtables_error(PARAMETER_PROBLEM,
			      "Unexpected `!' after --log%s", suffix);
	return bit;
}

static int brlog_parse(int c, char **argv, int invert, unsigned int *flags,
		       const void *entry, struct xt_entry_target **target)
{
	struct ebt_log_info *loginfo = (struct ebt_log_info *)(*target)->data;
	long int i;
	char *end;

	switch (c) {
	case LOG_PREFIX:
		if (invert)
			xtables_error(PARAMETER_PROBLEM,
				      "Unexpected `!` after --log-prefix");
		if (strlen(optarg) > sizeof(loginfo->prefix) - 1)
			xtables_error(PARAMETER_PROBLEM,
				      "Prefix too long");
		if (strchr(optarg, '\"'))
			xtables_error(PARAMETER_PROBLEM,
				      "Use of \\\" is not allowed"
				      " in the prefix");
		strcpy((char *)loginfo->prefix, (char *)optarg);
		break;
	case LOG_LEVEL:
		i = strtol(optarg, &end, 16);
		if (*end != '\0' || i < 0 || i > 7)
			loginfo->loglevel = name_to_loglevel(optarg);
		else
			loginfo->loglevel = i;

		if (loginfo->loglevel == 9)
			xtables_error(PARAMETER_PROBLEM,
				      "Problem with the log-level");
		break;
	case LOG_IP:
		loginfo->bitmask |= log_chk_inv(invert, EBT_LOG_IP, "-ip");
		break;
	case LOG_ARP:
		loginfo->bitmask |= log_chk_inv(invert, EBT_LOG_ARP, "-arp");
		break;
	case LOG_LOG:
		loginfo->bitmask |= log_chk_inv(invert, 0, "");
		break;
	case LOG_IP6:
		loginfo->bitmask |= log_chk_inv(invert, EBT_LOG_IP6, "-ip6");
		break;
	default:
		return 0;
	}

	*flags |= loginfo->bitmask;
	return 1;
}

static void brlog_final_check(unsigned int flags)
{
}

static void brlog_print(const void *ip, const struct xt_entry_target *target,
			int numeric)
{
	struct ebt_log_info *loginfo = (struct ebt_log_info *)target->data;

	printf("--log-level %s --log-prefix \"%s\"",
		eight_priority[loginfo->loglevel].c_name,
		loginfo->prefix);

	if (loginfo->bitmask & EBT_LOG_IP)
		printf(" --log-ip");
	if (loginfo->bitmask & EBT_LOG_ARP)
		printf(" --log-arp");
	if (loginfo->bitmask & EBT_LOG_IP6)
		printf(" --log-ip6");
	printf(" ");
}

static int brlog_xlate(struct xt_xlate *xl,
		       const struct xt_xlate_tg_params *params)
{
	const struct ebt_log_info *loginfo = (const void *)params->target->data;

	xt_xlate_add(xl, "log");
	if (loginfo->prefix[0]) {
		if (params->escape_quotes)
			xt_xlate_add(xl, " prefix \\\"%s\\\"", loginfo->prefix);
		else
			xt_xlate_add(xl, " prefix \"%s\"", loginfo->prefix);
	}

	if (loginfo->loglevel != LOG_DEFAULT_LEVEL)
		xt_xlate_add(xl, " level %s", eight_priority[loginfo->loglevel].c_name);

	xt_xlate_add(xl, " flags ether ");

	return 1;
}

static struct xtables_target brlog_target = {
	.name		= "log",
	.revision	= 0,
	.version	= XTABLES_VERSION,
	.family		= NFPROTO_BRIDGE,
	.size		= XT_ALIGN(sizeof(struct ebt_log_info)),
	.userspacesize	= XT_ALIGN(sizeof(struct ebt_log_info)),
	.init		= brlog_init,
	.help		= brlog_help,
	.parse		= brlog_parse,
	.final_check	= brlog_final_check,
	.print		= brlog_print,
	.xlate		= brlog_xlate,
	.extra_opts	= brlog_opts,
};

void _init(void)
{
	xtables_register_target(&brlog_target);
}
