/* Shared library add-on to iptables to add limit support.
 *
 * Jérôme de Vivie   <devivie@info.enserb.u-bordeaux.fr>
 * Hervé Eychenne    <rv@wallfire.org>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <iptables.h>
#include <stddef.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_limit.h>

#define IPT_LIMIT_AVG	"3/hour"
#define IPT_LIMIT_BURST	5

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"limit v%s options:\n"
"--limit avg			max average match rate: default "IPT_LIMIT_AVG"\n"
"                                [Packets per second unless followed by \n"
"                                /sec /minute /hour /day postfixes]\n"
"--limit-burst number		number to match in a burst, default %u\n"
"\n", IPTABLES_VERSION, IPT_LIMIT_BURST);
}

static struct option opts[] = {
	{ "limit", 1, 0, '%' },
	{ "limit-burst", 1, 0, '$' },
	{ 0 }
};

static
int parse_rate(const char *rate, u_int32_t *val)
{
	const char *delim;
	u_int32_t r;
	u_int32_t mult = 1;  /* Seconds by default. */

	delim = strchr(rate, '/');
	if (delim) {
		if (strlen(delim+1) == 0)
			return 0;

		if (strncasecmp(delim+1, "second", strlen(delim+1)) == 0)
			mult = 1;
		else if (strncasecmp(delim+1, "minute", strlen(delim+1)) == 0)
			mult = 60;
		else if (strncasecmp(delim+1, "hour", strlen(delim+1)) == 0)
			mult = 60*60;
		else if (strncasecmp(delim+1, "day", strlen(delim+1)) == 0)
			mult = 24*60*60;
		else
			return 0;
	}
	r = atoi(rate);
	if (!r)
		return 0;

	/* This would get mapped to infinite (1/day is minimum they
           can specify, so we're ok at that end). */
	if (r / mult > IPT_LIMIT_SCALE)
		exit_error(PARAMETER_PROBLEM, "Rate too fast `%s'\n", rate);

	*val = IPT_LIMIT_SCALE * mult / r;
	return 1;
}

/* Initialize the match. */
static void
init(struct ipt_entry_match *m, unsigned int *nfcache)
{
	struct ipt_rateinfo *r = (struct ipt_rateinfo *)m->data;

	parse_rate(IPT_LIMIT_AVG, &r->avg);
	r->burst = IPT_LIMIT_BURST;

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
	struct ipt_rateinfo *r = (struct ipt_rateinfo *)(*match)->data;
	unsigned int num;

	switch(c) {
	case '%':
		if (check_inverse(optarg, &invert, NULL, 0))
			exit_error(PARAMETER_PROBLEM,
				   "Unexpected `!' after --limit");
		if (!parse_rate(optarg, &r->avg))
			exit_error(PARAMETER_PROBLEM,
				   "bad rate `%s'", optarg);
		break;

	case '$':
		if (check_inverse(optarg, &invert, NULL, 0))
			exit_error(PARAMETER_PROBLEM,
				   "Unexpected `!' after --limit-burst");

		if (string_to_number(optarg, 0, 10000, &num) == -1)
			exit_error(PARAMETER_PROBLEM,
				   "bad --limit-burst `%s'", optarg);
		r->burst = num;
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

static struct rates
{
	const char *name;
	u_int32_t mult;
} rates[] = { { "day", IPT_LIMIT_SCALE*24*60*60 },
	      { "hour", IPT_LIMIT_SCALE*60*60 },
	      { "min", IPT_LIMIT_SCALE*60 },
	      { "sec", IPT_LIMIT_SCALE } };

static void print_rate(u_int32_t period)
{
	unsigned int i;

	for (i = 1; i < sizeof(rates)/sizeof(struct rates); i++) {
		if (period > rates[i].mult
            || rates[i].mult/period < rates[i].mult%period)
			break;
	}

	printf("%u/%s ", rates[i-1].mult / period, rates[i-1].name);
}

/* Prints out the matchinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_match *match,
      int numeric)
{
	struct ipt_rateinfo *r = (struct ipt_rateinfo *)match->data;
	printf("limit: avg "); print_rate(r->avg);
	printf("burst %u ", r->burst);
}

static void save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
{
	struct ipt_rateinfo *r = (struct ipt_rateinfo *)match->data;

	printf("--limit "); print_rate(r->avg);
	if (r->burst != IPT_LIMIT_BURST)
		printf("--limit-burst %u ", r->burst);
}

static
struct iptables_match limit
= { NULL,
    "limit",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_rateinfo)),
    offsetof(struct ipt_rateinfo, prev),
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
	register_match(&limit);
}
