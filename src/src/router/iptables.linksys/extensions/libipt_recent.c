/* Shared library add-on to iptables to add recent matching support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <iptables.h>
#include <linux/netfilter_ipv4/ipt_recent.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"recent v%s options:\n"
"[!] --set                       Add source address to list, always matches.\n"
"[!] --rcheck                    Match if source address in list.\n"
"[!] --update                    Match if source address in list, also update last-seen time.\n"
"[!] --remove                    Match if source address in list, also removes that address from list.\n"
"    --seconds seconds           For check and update commands above.\n"
"                                Specifies that the match will only occur if source address last seen within\n"
"                                the last 'seconds' seconds.\n"
"    --hitcount hits             For check and update commands above.\n"
"                                Specifies that the match will only occur if source address seen hits times.\n"
"                                May be used in conjunction with the seconds option.\n"
"    --rttl                      For check and update commands above.\n"
"                                Specifies that the match will only occur if the source address and the TTL\n"
"                                match between this packet and the one which was set.\n"
"                                Useful if you have problems with people spoofing their source address in order\n"
"                                to DoS you via this module.\n"
"    --name name                 Name of the recent list to be used.  DEFAULT used if none given.\n"
"    --rsource                   Save the source address of each packet in the recent list table (default).\n"
"    --rdest                     Save the destination address of each packet in the recent list table.\n"
,
IPTABLES_VERSION);

}
  
static struct option opts[] = {
	{ "set", 0, 0, 201 }, 
	{ "rcheck", 0, 0, 202 }, 
	{ "update", 0, 0, 203 },
	{ "seconds", 1, 0, 204 }, 
	{ "hitcount", 1, 0, 205 },
	{ "remove",0, 0, 206 },
	{ "rttl",0, 0, 207},
	{ "name", 1, 0, 208},
	{ "rsource", 0, 0, 209},
	{ "rdest", 0, 0, 210},
	{0}
};

/* Initialize the match. */
static void
init(struct ipt_entry_match *match, unsigned int *nfcache)
{
	struct ipt_recent_info *info = (struct ipt_recent_info *)(match)->data;

	*nfcache |= NFC_UNKNOWN;

	strncpy(info->name,"DEFAULT",200);
	info->side = IPT_RECENT_SOURCE;
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      unsigned int *nfcache,
      struct ipt_entry_match **match)
{
	struct ipt_recent_info *info = (struct ipt_recent_info *)(*match)->data;
	switch (c) {
		case 201:
			if (*flags) exit_error(PARAMETER_PROBLEM,
					"recent: only one of `--set', `--check' "
					"`--update' or `--remove' may be set");
			check_inverse(optarg, &invert, &optind, 0);
			info->check_set |= IPT_RECENT_SET;
			if (invert) info->invert = 1;
			*flags = 1;
			break;
			
		case 202:
			if (*flags) exit_error(PARAMETER_PROBLEM,
					"recent: only one of `--set', `--check' "
					"`--update' or `--remove' may be set");
			check_inverse(optarg, &invert, &optind, 0);
			info->check_set |= IPT_RECENT_CHECK;
			if(invert) info->invert = 1;
			*flags = 1;
			break;

		case 203:
			if (*flags) exit_error(PARAMETER_PROBLEM,
					"recent: only one of `--set', `--check' "
					"`--update' or `--remove' may be set");
			check_inverse(optarg, &invert, &optind, 0);
			info->check_set |= IPT_RECENT_UPDATE;
			if (invert) info->invert = 1;
			*flags = 1;
			break;

		case 206:
			if (*flags) exit_error(PARAMETER_PROBLEM,
					"recent: only one of `--set', `--check' "
					"`--update' or `--remove' may be set");
			check_inverse(optarg, &invert, &optind, 0);
			info->check_set |= IPT_RECENT_REMOVE;
			if (invert) info->invert = 1;
			*flags = 1;
			break;

		case 204:
			info->seconds = atoi(optarg);
			break;

		case 205:
			info->hit_count = atoi(optarg);
			break;

		case 207:
			info->check_set |= IPT_RECENT_TTL;
			break;

		case 208:
			strncpy(info->name,optarg,200);
			break;

		case 209:
			info->side = IPT_RECENT_SOURCE;
			break;

		case 210:
			info->side = IPT_RECENT_DEST;
			break;

		default:
			return 0;
	}

	return 1;
}

/* Final check; must have specified a specific option. */
static void
final_check(unsigned int flags)
{

	if (!flags)
		exit_error(PARAMETER_PROBLEM,
			"recent: you must specify one of `--set', `--check' "
			"`--update' or `--remove'");
}

/* Prints out the matchinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_match *match,
      int numeric)
{
	struct ipt_recent_info *info = (struct ipt_recent_info *)match->data;

	if (info->invert) fputc('!', stdout);

	printf("recent: ");
	if(info->check_set & IPT_RECENT_SET) printf("SET ");
	if(info->check_set & IPT_RECENT_CHECK) printf("CHECK ");
	if(info->check_set & IPT_RECENT_UPDATE) printf("UPDATE ");
	if(info->check_set & IPT_RECENT_REMOVE) printf("REMOVE ");
	if(info->seconds) printf("seconds: %d ",info->seconds);
	if(info->hit_count) printf("hit_count: %d ",info->hit_count);
	if(info->check_set & IPT_RECENT_TTL) printf("TTL-Match ");
	if(info->name) printf("name: %s ",info->name);
	if(info->side == IPT_RECENT_SOURCE) printf("side: source ");
	if(info->side == IPT_RECENT_DEST) printf("side: dest");
}

/* Saves the union ipt_matchinfo in parsable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
{
	struct ipt_recent_info *info = (struct ipt_recent_info *)match;

	if (info->invert) fputc('!', stdout);

	printf("recent: ");
	if(info->check_set & IPT_RECENT_SET) printf("SET ");
	if(info->check_set & IPT_RECENT_CHECK) printf("CHECK ");
	if(info->check_set & IPT_RECENT_UPDATE) printf("UPDATE ");
	if(info->check_set & IPT_RECENT_REMOVE) printf("REMOVE ");
	if(info->seconds) printf("seconds: %d ",info->seconds);
	if(info->hit_count) printf("hit_count: %d ",info->hit_count);
	if(info->check_set & IPT_RECENT_TTL) printf("TTL-Match ");
	if(info->name) printf("name: %s ",info->name);
	if(info->side == IPT_RECENT_SOURCE) printf("side: source ");
	if(info->side == IPT_RECENT_DEST) printf("side: dest");
}

static
struct iptables_match recent
= { NULL,
    "recent",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_recent_info)),
    IPT_ALIGN(sizeof(struct ipt_recent_info)),
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
	register_match(&recent);
}
