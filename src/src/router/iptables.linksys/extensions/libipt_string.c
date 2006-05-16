/* Shared library add-on to iptables to add string matching support. 
 * 
 * Copyright (C) 2000 Emmanuel Roger  <winfield@freegates.be>
 *
 * ChangeLog
 *     27.01.2001: Gianni Tedesco <gianni@ecsc.co.uk>
 *             Changed --tos to --string in save(). Also
 *             updated to work with slightly modified
 *             ipt_string_info.
 */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <iptables.h>
#include <linux/netfilter_ipv4/ipt_string.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"STRING match v%s options:\n"
"--string [!] string             Match a string in a packet\n",
IPTABLES_VERSION);

	fputc('\n', stdout);
}

static struct option opts[] = {
	{ "string", 1, 0, '1' },
	{0}
};

/* Initialize the match. */
static void
init(struct ipt_entry_match *m, unsigned int *nfcache)
{
	*nfcache |= NFC_UNKNOWN;
}

static void
parse_string(const unsigned char *s, struct ipt_string_info *info)
{	
        if (strlen(s) <= BM_MAX_NLEN) strcpy(info->string, s);
	else exit_error(PARAMETER_PROBLEM, "STRING too long `%s'", s);
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      unsigned int *nfcache,
      struct ipt_entry_match **match)
{
	struct ipt_string_info *stringinfo = (struct ipt_string_info *)(*match)->data;

	switch (c) {
	case '1':
		check_inverse(optarg, &invert, &optind, 0);
		parse_string(argv[optind-1], stringinfo);
		if (invert)
			stringinfo->invert = 1;
                stringinfo->len=strlen((char *)&stringinfo->string);
		*flags = 1;
		break;

	default:
		return 0;
	}
	return 1;
}

static void
print_string(char string[], int invert, int numeric)
{

	if (invert)
		fputc('!', stdout);
	printf("%s ",string);
}

/* Final check; must have specified --string. */
static void
final_check(unsigned int flags)
{
	if (!flags)
		exit_error(PARAMETER_PROBLEM,
			   "STRING match: You must specify `--string'");
}

/* Prints out the matchinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_match *match,
      int numeric)
{
	printf("STRING match ");
	print_string(((struct ipt_string_info *)match->data)->string,
		  ((struct ipt_string_info *)match->data)->invert, numeric);
}

/* Saves the union ipt_matchinfo in parsable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
{
	printf("--string ");
	print_string(((struct ipt_string_info *)match->data)->string,
		  ((struct ipt_string_info *)match->data)->invert, 0);
}

static
struct iptables_match string
= { NULL,
    "string",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_string_info)),
    IPT_ALIGN(sizeof(struct ipt_string_info)),
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
	register_match(&string);
}
