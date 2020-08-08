/* Code to take an iptables-style command line and do it. */

/*
 * Author: Paul.Russell@rustcorp.com.au and mneuling@radlogic.com.au
 *
 * (C) 2000-2002 by the netfilter coreteam <coreteam@netfilter.org>:
 *		    Paul 'Rusty' Russell <rusty@rustcorp.com.au>
 *		    Marc Boucher <marc+nf@mbsi.ca>
 *		    James Morris <jmorris@intercode.com.au>
 *		    Harald Welte <laforge@gnumonks.org>
 *		    Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "config.h"
#include <getopt.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
#include <iptables.h>
#include <xtables.h>
#include <fcntl.h>
#include "xshared.h"
#include "nft-shared.h"
#include "nft.h"

#define OPT_FRAGMENT	0x00800U
#define NUMBER_OF_OPT	ARRAY_SIZE(optflags)
static const char optflags[]
= { 'n', 's', 'd', 'p', 'j', 'v', 'x', 'i', 'o', '0', 'c', 'f'};

static struct option original_opts[] = {
	{.name = "append",	  .has_arg = 1, .val = 'A'},
	{.name = "delete",	  .has_arg = 1, .val = 'D'},
	{.name = "check",	  .has_arg = 1, .val = 'C'},
	{.name = "insert",	  .has_arg = 1, .val = 'I'},
	{.name = "replace",	  .has_arg = 1, .val = 'R'},
	{.name = "list",	  .has_arg = 2, .val = 'L'},
	{.name = "list-rules",	  .has_arg = 2, .val = 'S'},
	{.name = "flush",	  .has_arg = 2, .val = 'F'},
	{.name = "zero",	  .has_arg = 2, .val = 'Z'},
	{.name = "new-chain",	  .has_arg = 1, .val = 'N'},
	{.name = "delete-chain",  .has_arg = 2, .val = 'X'},
	{.name = "rename-chain",  .has_arg = 1, .val = 'E'},
	{.name = "policy",	  .has_arg = 1, .val = 'P'},
	{.name = "source",	  .has_arg = 1, .val = 's'},
	{.name = "destination",   .has_arg = 1, .val = 'd'},
	{.name = "src",		  .has_arg = 1, .val = 's'}, /* synonym */
	{.name = "dst",		  .has_arg = 1, .val = 'd'}, /* synonym */
	{.name = "protocol",	  .has_arg = 1, .val = 'p'},
	{.name = "in-interface",  .has_arg = 1, .val = 'i'},
	{.name = "jump",	  .has_arg = 1, .val = 'j'},
	{.name = "table",	  .has_arg = 1, .val = 't'},
	{.name = "match",	  .has_arg = 1, .val = 'm'},
	{.name = "numeric",	  .has_arg = 0, .val = 'n'},
	{.name = "out-interface", .has_arg = 1, .val = 'o'},
	{.name = "verbose",	  .has_arg = 0, .val = 'v'},
	{.name = "wait",	  .has_arg = 2, .val = 'w'},
	{.name = "wait-interval", .has_arg = 2, .val = 'W'},
	{.name = "exact",	  .has_arg = 0, .val = 'x'},
	{.name = "fragments",	  .has_arg = 0, .val = 'f'},
	{.name = "version",	  .has_arg = 0, .val = 'V'},
	{.name = "help",	  .has_arg = 2, .val = 'h'},
	{.name = "line-numbers",  .has_arg = 0, .val = '0'},
	{.name = "modprobe",	  .has_arg = 1, .val = 'M'},
	{.name = "set-counters",  .has_arg = 1, .val = 'c'},
	{.name = "goto",	  .has_arg = 1, .val = 'g'},
	{.name = "ipv4",	  .has_arg = 0, .val = '4'},
	{.name = "ipv6",	  .has_arg = 0, .val = '6'},
	{NULL},
};

void xtables_exit_error(enum xtables_exittype status, const char *msg, ...) __attribute__((noreturn, format(printf,2,3)));

struct xtables_globals xtables_globals = {
	.option_offset = 0,
	.program_version = PACKAGE_VERSION,
	.orig_opts = original_opts,
	.exit_err = xtables_exit_error,
	.compat_rev = nft_compatible_revision,
};

/* Table of legal combinations of commands and options.  If any of the
 * given commands make an option legal, that option is legal (applies to
 * CMD_LIST and CMD_ZERO only).
 * Key:
 *  +  compulsory
 *  x  illegal
 *     optional
 */

static const char commands_v_options[NUMBER_OF_CMD][NUMBER_OF_OPT] =
/* Well, it's better than "Re: Linux vs FreeBSD" */
{
	/*     -n  -s  -d  -p  -j  -v  -x  -i  -o --line -c -f */
/*INSERT*/    {'x',' ',' ',' ',' ',' ','x',' ',' ','x',' ',' '},
/*DELETE*/    {'x',' ',' ',' ',' ',' ','x',' ',' ','x','x',' '},
/*DELETE_NUM*/{'x','x','x','x','x',' ','x','x','x','x','x','x'},
/*REPLACE*/   {'x',' ',' ',' ',' ',' ','x',' ',' ','x',' ',' '},
/*APPEND*/    {'x',' ',' ',' ',' ',' ','x',' ',' ','x',' ',' '},
/*LIST*/      {' ','x','x','x','x',' ',' ','x','x',' ','x','x'},
/*FLUSH*/     {'x','x','x','x','x',' ','x','x','x','x','x','x'},
/*ZERO*/      {'x','x','x','x','x',' ','x','x','x','x','x','x'},
/*ZERO_NUM*/  {'x','x','x','x','x',' ','x','x','x','x','x','x'},
/*NEW_CHAIN*/ {'x','x','x','x','x',' ','x','x','x','x','x','x'},
/*DEL_CHAIN*/ {'x','x','x','x','x',' ','x','x','x','x','x','x'},
/*SET_POLICY*/{'x','x','x','x','x',' ','x','x','x','x',' ','x'},
/*RENAME*/    {'x','x','x','x','x',' ','x','x','x','x','x','x'},
/*LIST_RULES*/{'x','x','x','x','x',' ','x','x','x','x','x','x'},
/*CHECK*/     {'x',' ',' ',' ',' ',' ','x',' ',' ','x','x',' '},
};

static const int inverse_for_options[NUMBER_OF_OPT] =
{
/* -n */ 0,
/* -s */ IPT_INV_SRCIP,
/* -d */ IPT_INV_DSTIP,
/* -p */ XT_INV_PROTO,
/* -j */ 0,
/* -v */ 0,
/* -x */ 0,
/* -i */ IPT_INV_VIA_IN,
/* -o */ IPT_INV_VIA_OUT,
/*--line*/ 0,
/* -c */ 0,
/* -f */ IPT_INV_FRAG,
};

#define opts xt_params->opts
#define prog_name xt_params->program_name
#define prog_vers xt_params->program_version

static void __attribute__((noreturn))
exit_tryhelp(int status)
{
	if (line != -1)
		fprintf(stderr, "Error occurred at line: %d\n", line);
	fprintf(stderr, "Try `%s -h' or '%s --help' for more information.\n",
			prog_name, prog_name);
	xtables_free_opts(1);
	exit(status);
}

static void
printhelp(const struct xtables_rule_match *matches)
{
	printf("%s v%s\n\n"
"Usage: %s -[ACD] chain rule-specification [options]\n"
"	%s -I chain [rulenum] rule-specification [options]\n"
"	%s -R chain rulenum rule-specification [options]\n"
"	%s -D chain rulenum [options]\n"
"	%s -[LS] [chain [rulenum]] [options]\n"
"	%s -[FZ] [chain] [options]\n"
"	%s -[NX] chain\n"
"	%s -E old-chain-name new-chain-name\n"
"	%s -P chain target [options]\n"
"	%s -h (print this help information)\n\n",
	       prog_name, prog_vers, prog_name, prog_name,
	       prog_name, prog_name, prog_name, prog_name,
	       prog_name, prog_name, prog_name, prog_name);

	printf(
"Commands:\n"
"Either long or short options are allowed.\n"
"  --append  -A chain		Append to chain\n"
"  --check   -C chain		Check for the existence of a rule\n"
"  --delete  -D chain		Delete matching rule from chain\n"
"  --delete  -D chain rulenum\n"
"				Delete rule rulenum (1 = first) from chain\n"
"  --insert  -I chain [rulenum]\n"
"				Insert in chain as rulenum (default 1=first)\n"
"  --replace -R chain rulenum\n"
"				Replace rule rulenum (1 = first) in chain\n"
"  --list    -L [chain [rulenum]]\n"
"				List the rules in a chain or all chains\n"
"  --list-rules -S [chain [rulenum]]\n"
"				Print the rules in a chain or all chains\n"
"  --flush   -F [chain]		Delete all rules in  chain or all chains\n"
"  --zero    -Z [chain [rulenum]]\n"
"				Zero counters in chain or all chains\n"
"  --new     -N chain		Create a new user-defined chain\n"
"  --delete-chain\n"
"	     -X [chain]		Delete a user-defined chain\n"
"  --policy  -P chain target\n"
"				Change policy on chain to target\n"
"  --rename-chain\n"
"	     -E old-chain new-chain\n"
"				Change chain name, (moving any references)\n"

"Options:\n"
"    --ipv4	-4		Nothing (line is ignored by ip6tables-restore)\n"
"    --ipv6	-6		Error (line is ignored by iptables-restore)\n"
"[!] --proto	-p proto	protocol: by number or name, eg. `tcp'\n"
"[!] --source	-s address[/mask][...]\n"
"				source specification\n"
"[!] --destination -d address[/mask][...]\n"
"				destination specification\n"
"[!] --in-interface -i input name[+]\n"
"				network interface name ([+] for wildcard)\n"
" --jump	-j target\n"
"				target for rule (may load target extension)\n"
#ifdef IPT_F_GOTO
"  --goto      -g chain\n"
"			       jump to chain with no return\n"
#endif
"  --match	-m match\n"
"				extended match (may load extension)\n"
"  --numeric	-n		numeric output of addresses and ports\n"
"[!] --out-interface -o output name[+]\n"
"				network interface name ([+] for wildcard)\n"
"  --table	-t table	table to manipulate (default: `filter')\n"
"  --verbose	-v		verbose mode\n"
"  --wait	-w [seconds]	maximum wait to acquire xtables lock before give up\n"
"  --wait-interval -W [usecs]	wait time to try to acquire xtables lock\n"
"				default is 1 second\n"
"  --line-numbers		print line numbers when listing\n"
"  --exact	-x		expand numbers (display exact values)\n"
"[!] --fragment	-f		match second or further fragments only\n"
"  --modprobe=<command>		try to insert modules using this command\n"
"  --set-counters PKTS BYTES	set the counter during insert/append\n"
"[!] --version	-V		print package version.\n");

	print_extension_helps(xtables_targets, matches);
}

void
xtables_exit_error(enum xtables_exittype status, const char *msg, ...)
{
	va_list args;

	va_start(args, msg);
	fprintf(stderr, "%s v%s (nf_tables): ", prog_name, prog_vers);
	vfprintf(stderr, msg, args);
	va_end(args);
	fprintf(stderr, "\n");
	if (status == PARAMETER_PROBLEM)
		exit_tryhelp(status);
	if (status == VERSION_PROBLEM)
		fprintf(stderr,
			"Perhaps iptables or your kernel needs to be upgraded.\n");
	/* On error paths, make sure that we don't leak memory */
	xtables_free_opts(1);
	exit(status);
}

static void
generic_opt_check(int command, int options)
{
	int i, j, legal = 0;

	/* Check that commands are valid with options.	Complicated by the
	 * fact that if an option is legal with *any* command given, it is
	 * legal overall (ie. -z and -l).
	 */
	for (i = 0; i < NUMBER_OF_OPT; i++) {
		legal = 0; /* -1 => illegal, 1 => legal, 0 => undecided. */

		for (j = 0; j < NUMBER_OF_CMD; j++) {
			if (!(command & (1<<j)))
				continue;

			if (!(options & (1<<i))) {
				if (commands_v_options[j][i] == '+')
					xtables_error(PARAMETER_PROBLEM,
						   "You need to supply the `-%c' "
						   "option for this command\n",
						   optflags[i]);
			} else {
				if (commands_v_options[j][i] != 'x')
					legal = 1;
				else if (legal == 0)
					legal = -1;
			}
		}
		if (legal == -1)
			xtables_error(PARAMETER_PROBLEM,
				   "Illegal option `-%c' with this command\n",
				   optflags[i]);
	}
}

static char
opt2char(int option)
{
	const char *ptr;
	for (ptr = optflags; option > 1; option >>= 1, ptr++);

	return *ptr;
}

/*
 *	All functions starting with "parse" should succeed, otherwise
 *	the program fails.
 *	Most routines return pointers to static data that may change
 *	between calls to the same or other routines with a few exceptions:
 *	"host_to_addr", "parse_hostnetwork", and "parse_hostnetworkmask"
 *	return global static data.
*/

/* Christophe Burki wants `-p 6' to imply `-m tcp'.  */

static void
set_option(unsigned int *options, unsigned int option, uint8_t *invflg,
	   int invert)
{
	if (*options & option)
		xtables_error(PARAMETER_PROBLEM, "multiple -%c flags not allowed",
			   opt2char(option));
	*options |= option;

	if (invert) {
		unsigned int i;
		for (i = 0; 1 << i != option; i++);

		if (!inverse_for_options[i])
			xtables_error(PARAMETER_PROBLEM,
				   "cannot have ! before -%c",
				   opt2char(option));
		*invflg |= inverse_for_options[i];
	}
}

static int
add_entry(const char *chain,
	  const char *table,
	  struct iptables_command_state *cs,
	  int rulenum, int family,
	  const struct addr_mask s,
	  const struct addr_mask d,
	  bool verbose, struct nft_handle *h, bool append)
{
	unsigned int i, j;
	int ret = 1;

	for (i = 0; i < s.naddrs; i++) {
		if (family == AF_INET) {
			cs->fw.ip.src.s_addr = s.addr.v4[i].s_addr;
			cs->fw.ip.smsk.s_addr = s.mask.v4[i].s_addr;
			for (j = 0; j < d.naddrs; j++) {
				cs->fw.ip.dst.s_addr = d.addr.v4[j].s_addr;
				cs->fw.ip.dmsk.s_addr = d.mask.v4[j].s_addr;

				if (append) {
					ret = nft_cmd_rule_append(h, chain, table,
							      cs, NULL,
							      verbose);
				} else {
					ret = nft_cmd_rule_insert(h, chain, table,
							      cs, rulenum,
							      verbose);
				}
			}
		} else if (family == AF_INET6) {
			memcpy(&cs->fw6.ipv6.src,
			       &s.addr.v6[i], sizeof(struct in6_addr));
			memcpy(&cs->fw6.ipv6.smsk,
			       &s.mask.v6[i], sizeof(struct in6_addr));
			for (j = 0; j < d.naddrs; j++) {
				memcpy(&cs->fw6.ipv6.dst,
				       &d.addr.v6[j], sizeof(struct in6_addr));
				memcpy(&cs->fw6.ipv6.dmsk,
				       &d.mask.v6[j], sizeof(struct in6_addr));
				if (append) {
					ret = nft_cmd_rule_append(h, chain, table,
							      cs, NULL,
							      verbose);
				} else {
					ret = nft_cmd_rule_insert(h, chain, table,
							      cs, rulenum,
							      verbose);
				}
			}
		}
	}

	return ret;
}

static int
replace_entry(const char *chain, const char *table,
	      struct iptables_command_state *cs,
	      unsigned int rulenum,
	      int family,
	      const struct addr_mask s,
	      const struct addr_mask d,
	      bool verbose, struct nft_handle *h)
{
	if (family == AF_INET) {
		cs->fw.ip.src.s_addr = s.addr.v4->s_addr;
		cs->fw.ip.dst.s_addr = d.addr.v4->s_addr;
		cs->fw.ip.smsk.s_addr = s.mask.v4->s_addr;
		cs->fw.ip.dmsk.s_addr = d.mask.v4->s_addr;
	} else if (family == AF_INET6) {
		memcpy(&cs->fw6.ipv6.src, s.addr.v6, sizeof(struct in6_addr));
		memcpy(&cs->fw6.ipv6.dst, d.addr.v6, sizeof(struct in6_addr));
		memcpy(&cs->fw6.ipv6.smsk, s.mask.v6, sizeof(struct in6_addr));
		memcpy(&cs->fw6.ipv6.dmsk, d.mask.v6, sizeof(struct in6_addr));
	} else
		return 1;

	return nft_cmd_rule_replace(h, chain, table, cs, rulenum, verbose);
}

static int
delete_entry(const char *chain, const char *table,
	     struct iptables_command_state *cs,
	     int family,
	     const struct addr_mask s,
	     const struct addr_mask d,
	     bool verbose,
	     struct nft_handle *h)
{
	unsigned int i, j;
	int ret = 1;

	for (i = 0; i < s.naddrs; i++) {
		if (family == AF_INET) {
			cs->fw.ip.src.s_addr = s.addr.v4[i].s_addr;
			cs->fw.ip.smsk.s_addr = s.mask.v4[i].s_addr;
			for (j = 0; j < d.naddrs; j++) {
				cs->fw.ip.dst.s_addr = d.addr.v4[j].s_addr;
				cs->fw.ip.dmsk.s_addr = d.mask.v4[j].s_addr;
				ret = nft_cmd_rule_delete(h, chain,
						      table, cs, verbose);
			}
		} else if (family == AF_INET6) {
			memcpy(&cs->fw6.ipv6.src,
			       &s.addr.v6[i], sizeof(struct in6_addr));
			memcpy(&cs->fw6.ipv6.smsk,
			       &s.mask.v6[i], sizeof(struct in6_addr));
			for (j = 0; j < d.naddrs; j++) {
				memcpy(&cs->fw6.ipv6.dst,
				       &d.addr.v6[j], sizeof(struct in6_addr));
				memcpy(&cs->fw6.ipv6.dmsk,
				       &d.mask.v6[j], sizeof(struct in6_addr));
				ret = nft_cmd_rule_delete(h, chain,
						      table, cs, verbose);
			}
		}
	}

	return ret;
}

static int
check_entry(const char *chain, const char *table,
	    struct iptables_command_state *cs,
	    int family,
	    const struct addr_mask s,
	    const struct addr_mask d,
	    bool verbose, struct nft_handle *h)
{
	unsigned int i, j;
	int ret = 1;

	for (i = 0; i < s.naddrs; i++) {
		if (family == AF_INET) {
			cs->fw.ip.src.s_addr = s.addr.v4[i].s_addr;
			cs->fw.ip.smsk.s_addr = s.mask.v4[i].s_addr;
			for (j = 0; j < d.naddrs; j++) {
				cs->fw.ip.dst.s_addr = d.addr.v4[j].s_addr;
				cs->fw.ip.dmsk.s_addr = d.mask.v4[j].s_addr;
				ret = nft_cmd_rule_check(h, chain,
						     table, cs, verbose);
			}
		} else if (family == AF_INET6) {
			memcpy(&cs->fw6.ipv6.src,
			       &s.addr.v6[i], sizeof(struct in6_addr));
			memcpy(&cs->fw6.ipv6.smsk,
			       &s.mask.v6[i], sizeof(struct in6_addr));
			for (j = 0; j < d.naddrs; j++) {
				memcpy(&cs->fw6.ipv6.dst,
				       &d.addr.v6[j], sizeof(struct in6_addr));
				memcpy(&cs->fw6.ipv6.dmsk,
				       &d.mask.v6[j], sizeof(struct in6_addr));
				ret = nft_cmd_rule_check(h, chain,
						     table, cs, verbose);
			}
		}
	}

	return ret;
}

static int
list_entries(struct nft_handle *h, const char *chain, const char *table,
	     int rulenum, int verbose, int numeric, int expanded,
	     int linenumbers)
{
	unsigned int format;

	format = FMT_OPTIONS;
	if (!verbose)
		format |= FMT_NOCOUNTS;
	else
		format |= FMT_VIA;

	if (numeric)
		format |= FMT_NUMERIC;

	if (!expanded)
		format |= FMT_KILOMEGAGIGA;

	if (linenumbers)
		format |= FMT_LINENUMBERS;

	return nft_cmd_rule_list(h, chain, table, rulenum, format);
}

static int
list_rules(struct nft_handle *h, const char *chain, const char *table,
	   int rulenum, int counters)
{
	if (counters)
	    counters = -1;		/* iptables -c format */

	return nft_cmd_rule_list_save(h, chain, table, rulenum, counters);
}

void do_parse(struct nft_handle *h, int argc, char *argv[],
	      struct nft_xt_cmd_parse *p, struct iptables_command_state *cs,
	      struct xtables_args *args)
{
	struct xtables_match *m;
	struct xtables_rule_match *matchp;
	bool wait_interval_set = false;
	struct timeval wait_interval;
	struct xtables_target *t;
	bool table_set = false;
	int wait = 0;

	memset(cs, 0, sizeof(*cs));
	cs->jumpto = "";
	cs->argv = argv;

	/* re-set optind to 0 in case do_command4 gets called
	 * a second time */
	optind = 0;

	/* clear mflags in case do_command4 gets called a second time
	 * (we clear the global list of all matches for security)*/
	for (m = xtables_matches; m; m = m->next)
		m->mflags = 0;

	for (t = xtables_targets; t; t = t->next) {
		t->tflags = 0;
		t->used = 0;
	}

	/* Suppress error messages: we may add new options if we
	   demand-load a protocol. */
	opterr = 0;

	opts = xt_params->orig_opts;
	while ((cs->c = getopt_long(argc, argv,
	   "-:A:C:D:R:I:L::S::M:F::Z::N:X::E:P:Vh::o:p:s:d:j:i:fbvw::W::nt:m:xc:g:46",
					   opts, NULL)) != -1) {
		switch (cs->c) {
			/*
			 * Command selection
			 */
		case 'A':
			add_command(&p->command, CMD_APPEND, CMD_NONE,
				    cs->invert);
			p->chain = optarg;
			break;

		case 'C':
			add_command(&p->command, CMD_CHECK, CMD_NONE,
				    cs->invert);
			p->chain = optarg;
			break;

		case 'D':
			add_command(&p->command, CMD_DELETE, CMD_NONE,
				    cs->invert);
			p->chain = optarg;
			if (xs_has_arg(argc, argv)) {
				p->rulenum = parse_rulenumber(argv[optind++]);
				p->command = CMD_DELETE_NUM;
			}
			break;

		case 'R':
			add_command(&p->command, CMD_REPLACE, CMD_NONE,
				    cs->invert);
			p->chain = optarg;
			if (xs_has_arg(argc, argv))
				p->rulenum = parse_rulenumber(argv[optind++]);
			else
				xtables_error(PARAMETER_PROBLEM,
					   "-%c requires a rule number",
					   cmd2char(CMD_REPLACE));
			break;

		case 'I':
			add_command(&p->command, CMD_INSERT, CMD_NONE,
				    cs->invert);
			p->chain = optarg;
			if (xs_has_arg(argc, argv))
				p->rulenum = parse_rulenumber(argv[optind++]);
			else
				p->rulenum = 1;
			break;

		case 'L':
			add_command(&p->command, CMD_LIST,
				    CMD_ZERO | CMD_ZERO_NUM, cs->invert);
			if (optarg)
				p->chain = optarg;
			else if (xs_has_arg(argc, argv))
				p->chain = argv[optind++];
			if (xs_has_arg(argc, argv))
				p->rulenum = parse_rulenumber(argv[optind++]);
			break;

		case 'S':
			add_command(&p->command, CMD_LIST_RULES,
				    CMD_ZERO|CMD_ZERO_NUM, cs->invert);
			if (optarg)
				p->chain = optarg;
			else if (xs_has_arg(argc, argv))
				p->chain = argv[optind++];
			if (xs_has_arg(argc, argv))
				p->rulenum = parse_rulenumber(argv[optind++]);
			break;

		case 'F':
			add_command(&p->command, CMD_FLUSH, CMD_NONE,
				    cs->invert);
			if (optarg)
				p->chain = optarg;
			else if (xs_has_arg(argc, argv))
				p->chain = argv[optind++];
			break;

		case 'Z':
			add_command(&p->command, CMD_ZERO,
				    CMD_LIST|CMD_LIST_RULES, cs->invert);
			if (optarg)
				p->chain = optarg;
			else if (xs_has_arg(argc, argv))
				p->chain = argv[optind++];
			if (xs_has_arg(argc, argv)) {
				p->rulenum = parse_rulenumber(argv[optind++]);
				p->command = CMD_ZERO_NUM;
			}
			break;

		case 'N':
			if (optarg && (*optarg == '-' || *optarg == '!'))
				xtables_error(PARAMETER_PROBLEM,
					   "chain name not allowed to start "
					   "with `%c'\n", *optarg);
			if (xtables_find_target(optarg, XTF_TRY_LOAD))
				xtables_error(PARAMETER_PROBLEM,
					   "chain name may not clash "
					   "with target name\n");
			add_command(&p->command, CMD_NEW_CHAIN, CMD_NONE,
				    cs->invert);
			p->chain = optarg;
			break;

		case 'X':
			add_command(&p->command, CMD_DELETE_CHAIN, CMD_NONE,
				    cs->invert);
			if (optarg)
				p->chain = optarg;
			else if (xs_has_arg(argc, argv))
				p->chain = argv[optind++];
			break;

		case 'E':
			add_command(&p->command, CMD_RENAME_CHAIN, CMD_NONE,
				    cs->invert);
			p->chain = optarg;
			if (xs_has_arg(argc, argv))
				p->newname = argv[optind++];
			else
				xtables_error(PARAMETER_PROBLEM,
					   "-%c requires old-chain-name and "
					   "new-chain-name",
					    cmd2char(CMD_RENAME_CHAIN));
			break;

		case 'P':
			add_command(&p->command, CMD_SET_POLICY, CMD_NONE,
				    cs->invert);
			p->chain = optarg;
			if (xs_has_arg(argc, argv))
				p->policy = argv[optind++];
			else
				xtables_error(PARAMETER_PROBLEM,
					   "-%c requires a chain and a policy",
					   cmd2char(CMD_SET_POLICY));
			break;

		case 'h':
			if (!optarg)
				optarg = argv[optind];

			/* iptables -p icmp -h */
			if (!cs->matches && cs->protocol)
				xtables_find_match(cs->protocol,
					XTF_TRY_LOAD, &cs->matches);

			printhelp(cs->matches);
			p->command = CMD_NONE;
			return;

			/*
			 * Option selection
			 */
		case 'p':
			set_option(&cs->options, OPT_PROTOCOL,
				   &args->invflags, cs->invert);

			/* Canonicalize into lower case */
			for (cs->protocol = optarg; *cs->protocol; cs->protocol++)
				*cs->protocol = tolower(*cs->protocol);

			cs->protocol = optarg;
			args->proto = xtables_parse_protocol(cs->protocol);

			if (args->proto == 0 &&
			    (args->invflags & XT_INV_PROTO))
				xtables_error(PARAMETER_PROBLEM,
					   "rule would never match protocol");

			/* This needs to happen here to parse extensions */
			h->ops->proto_parse(cs, args);
			break;

		case 's':
			set_option(&cs->options, OPT_SOURCE,
				   &args->invflags, cs->invert);
			args->shostnetworkmask = optarg;
			break;

		case 'd':
			set_option(&cs->options, OPT_DESTINATION,
				   &args->invflags, cs->invert);
			args->dhostnetworkmask = optarg;
			break;

#ifdef IPT_F_GOTO
		case 'g':
			set_option(&cs->options, OPT_JUMP, &args->invflags,
				   cs->invert);
			args->goto_set = true;
			cs->jumpto = xt_parse_target(optarg);
			break;
#endif

		case 'j':
			set_option(&cs->options, OPT_JUMP, &cs->fw.ip.invflags,
				   cs->invert);
			command_jump(cs, optarg);
			break;


		case 'i':
			if (*optarg == '\0')
				xtables_error(PARAMETER_PROBLEM,
					"Empty interface is likely to be "
					"undesired");
			set_option(&cs->options, OPT_VIANAMEIN,
				   &args->invflags, cs->invert);
			xtables_parse_interface(optarg,
						args->iniface,
						args->iniface_mask);
			break;

		case 'o':
			if (*optarg == '\0')
				xtables_error(PARAMETER_PROBLEM,
					"Empty interface is likely to be "
					"undesired");
			set_option(&cs->options, OPT_VIANAMEOUT,
				   &args->invflags, cs->invert);
			xtables_parse_interface(optarg,
						args->outiface,
						args->outiface_mask);
			break;

		case 'f':
			if (args->family == AF_INET6) {
				xtables_error(PARAMETER_PROBLEM,
					"`-f' is not supported in IPv6, "
					"use -m frag instead");
			}
			set_option(&cs->options, OPT_FRAGMENT, &args->invflags,
				   cs->invert);
			args->flags |= IPT_F_FRAG;
			break;

		case 'v':
			if (!p->verbose)
				set_option(&cs->options, OPT_VERBOSE,
					   &args->invflags, cs->invert);
			p->verbose++;
			break;

		case 'm':
			command_match(cs);
			break;

		case 'n':
			set_option(&cs->options, OPT_NUMERIC, &args->invflags,
				   cs->invert);
			break;

		case 't':
			if (cs->invert)
				xtables_error(PARAMETER_PROBLEM,
					   "unexpected ! flag before --table");
			if (p->restore && table_set)
				xtables_error(PARAMETER_PROBLEM,
					      "The -t option (seen in line %u) cannot be used in %s.\n",
					      line, xt_params->program_name);
			if (!nft_table_builtin_find(h, optarg))
				xtables_error(VERSION_PROBLEM,
					      "table '%s' does not exist",
					      optarg);
			p->table = optarg;
			table_set = true;
			break;

		case 'x':
			set_option(&cs->options, OPT_EXPANDED, &args->invflags,
				   cs->invert);
			break;

		case 'V':
			if (cs->invert)
				printf("Not %s ;-)\n", prog_vers);
			else
				printf("%s v%s (nf_tables)\n",
				       prog_name, prog_vers);
			exit(0);

		case 'w':
			if (p->restore) {
				xtables_error(PARAMETER_PROBLEM,
					      "You cannot use `-w' from "
					      "iptables-restore");
			}

			wait = parse_wait_time(argc, argv);
			break;

		case 'W':
			if (p->restore) {
				xtables_error(PARAMETER_PROBLEM,
					      "You cannot use `-W' from "
					      "iptables-restore");
			}

			parse_wait_interval(argc, argv, &wait_interval);
			wait_interval_set = true;
			break;

		case '0':
			set_option(&cs->options, OPT_LINENUMBERS,
				   &args->invflags, cs->invert);
			break;

		case 'M':
			xtables_modprobe_program = optarg;
			break;

		case 'c':
			set_option(&cs->options, OPT_COUNTERS, &args->invflags,
				   cs->invert);
			args->pcnt = optarg;
			args->bcnt = strchr(args->pcnt + 1, ',');
			if (args->bcnt)
			    args->bcnt++;
			if (!args->bcnt && xs_has_arg(argc, argv))
				args->bcnt = argv[optind++];
			if (!args->bcnt)
				xtables_error(PARAMETER_PROBLEM,
					"-%c requires packet and byte counter",
					opt2char(OPT_COUNTERS));

			if (sscanf(args->pcnt, "%llu", &args->pcnt_cnt) != 1)
				xtables_error(PARAMETER_PROBLEM,
					"-%c packet counter not numeric",
					opt2char(OPT_COUNTERS));

			if (sscanf(args->bcnt, "%llu", &args->bcnt_cnt) != 1)
				xtables_error(PARAMETER_PROBLEM,
					"-%c byte counter not numeric",
					opt2char(OPT_COUNTERS));
			break;

		case '4':
			if (args->family == AF_INET)
				break;

			if (p->restore && args->family == AF_INET6)
				return;

			exit_tryhelp(2);

		case '6':
			if (args->family == AF_INET6)
				break;

			if (p->restore && args->family == AF_INET)
				return;

			exit_tryhelp(2);

		case 1: /* non option */
			if (optarg[0] == '!' && optarg[1] == '\0') {
				if (cs->invert)
					xtables_error(PARAMETER_PROBLEM,
						   "multiple consecutive ! not"
						   " allowed");
				cs->invert = true;
				optarg[0] = '\0';
				continue;
			}
			fprintf(stderr, "Bad argument `%s'\n", optarg);
			exit_tryhelp(2);

		default:
			if (command_default(cs, &xtables_globals) == 1)
				/* cf. ip6tables.c */
				continue;
			break;
		}
		cs->invert = false;
	}

	if (strcmp(p->table, "nat") == 0 &&
	    ((p->policy != NULL && strcmp(p->policy, "DROP") == 0) ||
	    (cs->jumpto != NULL && strcmp(cs->jumpto, "DROP") == 0)))
		xtables_error(PARAMETER_PROBLEM,
			"\nThe \"nat\" table is not intended for filtering, "
			"the use of DROP is therefore inhibited.\n\n");

	if (!wait && wait_interval_set)
		xtables_error(PARAMETER_PROBLEM,
			      "--wait-interval only makes sense with --wait\n");

	for (matchp = cs->matches; matchp; matchp = matchp->next)
		xtables_option_mfcall(matchp->match);
	if (cs->target != NULL)
		xtables_option_tfcall(cs->target);

	/* Fix me: must put inverse options checking here --MN */

	if (optind < argc)
		xtables_error(PARAMETER_PROBLEM,
			   "unknown arguments found on commandline");
	if (!p->command)
		xtables_error(PARAMETER_PROBLEM, "no command specified");
	if (cs->invert)
		xtables_error(PARAMETER_PROBLEM,
			   "nothing appropriate following !");

	/* Set only if required, needed by xtables-restore */
	if (h->family == AF_UNSPEC)
		h->family = args->family;

	h->ops->post_parse(p->command, cs, args);

	if (p->command == CMD_REPLACE &&
	    (args->s.naddrs != 1 || args->d.naddrs != 1))
		xtables_error(PARAMETER_PROBLEM, "Replacement rule does not "
			   "specify a unique address");

	generic_opt_check(p->command, cs->options);

	if (p->chain != NULL && strlen(p->chain) >= XT_EXTENSION_MAXNAMELEN)
		xtables_error(PARAMETER_PROBLEM,
			   "chain name `%s' too long (must be under %u chars)",
			   p->chain, XT_EXTENSION_MAXNAMELEN);

	if (p->command == CMD_APPEND ||
	    p->command == CMD_DELETE ||
	    p->command == CMD_DELETE_NUM ||
	    p->command == CMD_CHECK ||
	    p->command == CMD_INSERT ||
	    p->command == CMD_REPLACE) {
		if (strcmp(p->chain, "PREROUTING") == 0
		    || strcmp(p->chain, "INPUT") == 0) {
			/* -o not valid with incoming packets. */
			if (cs->options & OPT_VIANAMEOUT)
				xtables_error(PARAMETER_PROBLEM,
					   "Can't use -%c with %s\n",
					   opt2char(OPT_VIANAMEOUT),
					   p->chain);
		}

		if (strcmp(p->chain, "POSTROUTING") == 0
		    || strcmp(p->chain, "OUTPUT") == 0) {
			/* -i not valid with outgoing packets */
			if (cs->options & OPT_VIANAMEIN)
				xtables_error(PARAMETER_PROBLEM,
					   "Can't use -%c with %s\n",
					   opt2char(OPT_VIANAMEIN),
					   p->chain);
		}
	}
}

int do_commandx(struct nft_handle *h, int argc, char *argv[], char **table,
		bool restore)
{
	int ret = 1;
	struct nft_xt_cmd_parse p = {
		.table		= *table,
		.restore	= restore,
	};
	struct iptables_command_state cs;
	struct xtables_args args = {
		.family = h->family,
	};

	do_parse(h, argc, argv, &p, &cs, &args);

	switch (p.command) {
	case CMD_APPEND:
		ret = add_entry(p.chain, p.table, &cs, 0, h->family,
				args.s, args.d,
				cs.options & OPT_VERBOSE, h, true);
		break;
	case CMD_DELETE:
		ret = delete_entry(p.chain, p.table, &cs, h->family,
				   args.s, args.d,
				   cs.options & OPT_VERBOSE, h);
		break;
	case CMD_DELETE_NUM:
		ret = nft_cmd_rule_delete_num(h, p.chain, p.table,
					      p.rulenum - 1, p.verbose);
		break;
	case CMD_CHECK:
		ret = check_entry(p.chain, p.table, &cs, h->family,
				  args.s, args.d,
				  cs.options & OPT_VERBOSE, h);
		break;
	case CMD_REPLACE:
		ret = replace_entry(p.chain, p.table, &cs, p.rulenum - 1,
				    h->family, args.s, args.d,
				    cs.options & OPT_VERBOSE, h);
		break;
	case CMD_INSERT:
		ret = add_entry(p.chain, p.table, &cs, p.rulenum - 1,
				h->family, args.s, args.d,
				cs.options&OPT_VERBOSE, h, false);
		break;
	case CMD_FLUSH:
		ret = nft_cmd_rule_flush(h, p.chain, p.table,
					 cs.options & OPT_VERBOSE);
		break;
	case CMD_ZERO:
		ret = nft_cmd_chain_zero_counters(h, p.chain, p.table,
						  cs.options & OPT_VERBOSE);
		break;
	case CMD_ZERO_NUM:
		ret = nft_cmd_rule_zero_counters(h, p.chain, p.table,
					     p.rulenum - 1);
		break;
	case CMD_LIST:
	case CMD_LIST|CMD_ZERO:
	case CMD_LIST|CMD_ZERO_NUM:
		ret = list_entries(h, p.chain, p.table, p.rulenum,
				   cs.options & OPT_VERBOSE,
				   cs.options & OPT_NUMERIC,
				   cs.options & OPT_EXPANDED,
				   cs.options & OPT_LINENUMBERS);
		if (ret && (p.command & CMD_ZERO)) {
			ret = nft_cmd_chain_zero_counters(h, p.chain, p.table,
						      cs.options & OPT_VERBOSE);
		}
		if (ret && (p.command & CMD_ZERO_NUM)) {
			ret = nft_cmd_rule_zero_counters(h, p.chain, p.table,
						     p.rulenum - 1);
		}
		nft_check_xt_legacy(h->family, false);
		break;
	case CMD_LIST_RULES:
	case CMD_LIST_RULES|CMD_ZERO:
	case CMD_LIST_RULES|CMD_ZERO_NUM:
		ret = list_rules(h, p.chain, p.table, p.rulenum,
				 cs.options & OPT_VERBOSE);
		if (ret && (p.command & CMD_ZERO)) {
			ret = nft_cmd_chain_zero_counters(h, p.chain, p.table,
						      cs.options & OPT_VERBOSE);
		}
		if (ret && (p.command & CMD_ZERO_NUM)) {
			ret = nft_cmd_rule_zero_counters(h, p.chain, p.table,
						     p.rulenum - 1);
		}
		nft_check_xt_legacy(h->family, false);
		break;
	case CMD_NEW_CHAIN:
		ret = nft_cmd_chain_user_add(h, p.chain, p.table);
		break;
	case CMD_DELETE_CHAIN:
		ret = nft_cmd_chain_user_del(h, p.chain, p.table,
					 cs.options & OPT_VERBOSE);
		break;
	case CMD_RENAME_CHAIN:
		ret = nft_cmd_chain_user_rename(h, p.chain, p.table, p.newname);
		break;
	case CMD_SET_POLICY:
		ret = nft_cmd_chain_set(h, p.table, p.chain, p.policy, NULL);
		break;
	case CMD_NONE:
	/* do_parse ignored the line (eg: -4 with ip6tables-restore) */
		break;
	default:
		/* We should never reach this... */
		exit_tryhelp(2);
	}

	*table = p.table;

	nft_clear_iptables_command_state(&cs);

	if (h->family == AF_INET) {
		free(args.s.addr.v4);
		free(args.s.mask.v4);
		free(args.d.addr.v4);
		free(args.d.mask.v4);
	} else if (h->family == AF_INET6) {
		free(args.s.addr.v6);
		free(args.s.mask.v6);
		free(args.d.addr.v6);
		free(args.d.mask.v6);
	}
	xtables_free_opts(1);

	return ret;
}
