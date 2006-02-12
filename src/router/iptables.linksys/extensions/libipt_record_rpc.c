/* Shared library add-on to iptables for rpc match */
#include <stdio.h>
#include <getopt.h>
#include <iptables.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"record_rpc v%s takes no options\n"
"\n", IPTABLES_VERSION);
}

static struct option opts[] = {
	{0}
};

static void
init(struct ipt_entry_match *m, unsigned int *nfcache)
{
	/* Can't cache this. */
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
	return 0;
}

/* Final check; must have specified --mac. */
static void final_check(unsigned int flags)
{
}

/* Prints out the union ipt_matchinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_match *match,
      int numeric)
{
}

static void save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
{
}

static
struct iptables_match record_rpc
= { NULL,
    "record_rpc",
    IPTABLES_VERSION,
    IPT_ALIGN(0),
    IPT_ALIGN(0),
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
	register_match(&record_rpc);
}
