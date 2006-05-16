/* Shared library add-on to iptables to add realm matching support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#if defined(__GLIBC__) && __GLIBC__ == 2
#include <net/ethernet.h>
#else
#include <linux/if_ether.h>
#endif
#include <iptables.h>
#include <linux/netfilter_ipv4/ipt_realm.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"REALM v%s options:\n"
" --realm [!] value[/mask]\n"
"				Match realm\n"
"\n", IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "realm", 1, 0, '1' },
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
	struct ipt_realm_info *realminfo = (struct ipt_realm_info *)(*match)->data;

	switch (c) {
		char *end;
	case '1':
		check_inverse(optarg, &invert, &optind, 0);
		realminfo->id = strtoul(optarg, &end, 0);
		if (*end == '/') {
			realminfo->mask = strtoul(end+1, &end, 0);
		} else
			realminfo->mask = 0xffffffff;
		if (*end != '\0' || end == optarg)
			exit_error(PARAMETER_PROBLEM, "Bad REALM value `%s'", optarg);
		if (invert)
			realminfo->invert = 1;
		*flags = 1;
		break;

	default:
		return 0;
	}
	return 1;
}

static void
print_realm(unsigned long id, unsigned long mask, int invert, int numeric)
{
	if (invert)
		fputc('!', stdout);

	if(mask != 0xffffffff)
		printf("0x%lx/0x%lx ", id, mask);
	else
		printf("0x%lx ", id);
}

/* Prints out the matchinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_match *match,
      int numeric)
{
	printf("REALM match ");
	print_realm(((struct ipt_realm_info *)match->data)->id,
		   ((struct ipt_realm_info *)match->data)->mask,
		   ((struct ipt_realm_info *)match->data)->invert, numeric);
}


/* Saves the union ipt_matchinfo in parsable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
{
	printf("--realm ");
	print_realm(((struct ipt_realm_info *)match->data)->id,
		   ((struct ipt_realm_info *)match->data)->mask,
		   ((struct ipt_realm_info *)match->data)->invert, 0);
}

/* Final check; must have specified --mark. */
static void
final_check(unsigned int flags)
{
	if (!flags)
		exit_error(PARAMETER_PROBLEM,
			   "REALM match: You must specify `--realm'");
}

struct iptables_match realm
= { NULL,
    "realm",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_realm_info)),
    IPT_ALIGN(sizeof(struct ipt_realm_info)),
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
	register_match(&realm);
}


