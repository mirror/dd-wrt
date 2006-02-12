/* Shared library add-on to iptables to add state tracking support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <getopt.h>
#include <iptables.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ipt_iplimit.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"iplimit v%s options:\n"
"[!] --iplimit-above n		match if the number of existing tcp connections is (not) above n\n"
" --iplimit-mask n		group hosts using mask\n"
"\n", IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "iplimit-above", 1, 0, '1' },
	{ "iplimit-mask",  1, 0, '2' },
	{0}
};

/* Initialize the match. */
static void
init(struct ipt_entry_match *m, unsigned int *nfcache)
{
	/* Can't cache this */
	*nfcache |= NFC_UNKNOWN;
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      unsigned int *nfcache,
      struct ipt_entry_match **match)
{
	struct ipt_iplimit_info *info = (struct ipt_iplimit_info*)(*match)->data;

	if (0 == (*flags & 2)) {
		/* set default mask unless we've already seen a mask option */
		info->mask = htonl(0xFFFFFFFF);
	}

	switch (c) {
	case '1':
		check_inverse(optarg, &invert, &optind, 0);
		info->limit = atoi(argv[optind-1]);
		info->inverse = invert;
		*flags |= 1;
		break;

	case '2':
		info->mask = htonl(0xFFFFFFFF << (32 - atoi(argv[optind-1])));
		*flags |= 2;
		break;

	default:
		return 0;
	}

	return 1;
}

/* Final check */
static void final_check(unsigned int flags)
{
	if (!flags & 1)
		exit_error(PARAMETER_PROBLEM, "You must specify `--iplimit-above'");
}

static int
count_bits(u_int32_t mask)
{
	int i, bits;

	for (bits = 0, i = 31; i >= 0; i--) {
		if (mask & htonl((u_int32_t)1 << i)) {
			bits++;
			continue;
		}
		break;
	}
	return bits;
}

/* Prints out the matchinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_match *match,
      int numeric)
{
	struct ipt_iplimit_info *info = (struct ipt_iplimit_info*)match->data;

	printf("#conn/%d %s %d ", count_bits(info->mask),
	       info->inverse ? "<" : ">", info->limit);
}

/* Saves the matchinfo in parsable form to stdout. */
static void save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
{
	struct ipt_iplimit_info *info = (struct ipt_iplimit_info*)match->data;

	printf("%s--iplimit-above %d ",info->inverse ? "! " : "",info->limit);
	printf("--iplimit-mask %d ",count_bits(info->mask));
}

static struct iptables_match iplimit = {
	name:		"iplimit",
	version:	IPTABLES_VERSION,
	size:		IPT_ALIGN(sizeof(struct ipt_iplimit_info)),
	userspacesize:	offsetof(struct ipt_iplimit_info,data),
	help:		help,
	init:		init,
	parse:		parse,
	final_check:	final_check,
	print:		print,
	save: 		save,
	extra_opts:	opts
};

void _init(void)
{
	register_match(&iplimit);
}
