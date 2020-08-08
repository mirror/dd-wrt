/*
 * ebtables.c, v2.0 July 2002
 *
 * Author: Bart De Schuymer
 *
 *  This code was stongly inspired on the iptables code which is
 *  Copyright (C) 1999 Paul `Rusty' Russell & Michael J. Neuling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "config.h"
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <signal.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <iptables.h>
#include <xtables.h>

#include <linux/netfilter_bridge.h>
#include <linux/netfilter/nf_tables.h>
#include <libiptc/libxtc.h>
#include "xshared.h"
#include "nft.h"
#include "nft-bridge.h"

/*
 * From include/ebtables_u.h
 */
#define ebt_check_option2(flags, mask) EBT_CHECK_OPTION(flags, mask)

/*
 * From useful_functions.c
 */

/* 0: default
 * 1: the inverse '!' of the option has already been specified */
int ebt_invert = 0;

unsigned char eb_mac_type_unicast[ETH_ALEN] =   {0,0,0,0,0,0};
unsigned char eb_msk_type_unicast[ETH_ALEN] =   {1,0,0,0,0,0};
unsigned char eb_mac_type_multicast[ETH_ALEN] = {1,0,0,0,0,0};
unsigned char eb_msk_type_multicast[ETH_ALEN] = {1,0,0,0,0,0};
unsigned char eb_mac_type_broadcast[ETH_ALEN] = {255,255,255,255,255,255};
unsigned char eb_msk_type_broadcast[ETH_ALEN] = {255,255,255,255,255,255};
unsigned char eb_mac_type_bridge_group[ETH_ALEN] = {0x01,0x80,0xc2,0,0,0};
unsigned char eb_msk_type_bridge_group[ETH_ALEN] = {255,255,255,255,255,255};

int ebt_get_mac_and_mask(const char *from, unsigned char *to,
  unsigned char *mask)
{
	char *p;
	int i;
	struct ether_addr *addr = NULL;

	if (strcasecmp(from, "Unicast") == 0) {
		memcpy(to, eb_mac_type_unicast, ETH_ALEN);
		memcpy(mask, eb_msk_type_unicast, ETH_ALEN);
		return 0;
	}
	if (strcasecmp(from, "Multicast") == 0) {
		memcpy(to, eb_mac_type_multicast, ETH_ALEN);
		memcpy(mask, eb_msk_type_multicast, ETH_ALEN);
		return 0;
	}
	if (strcasecmp(from, "Broadcast") == 0) {
		memcpy(to, eb_mac_type_broadcast, ETH_ALEN);
		memcpy(mask, eb_msk_type_broadcast, ETH_ALEN);
		return 0;
	}
	if (strcasecmp(from, "BGA") == 0) {
		memcpy(to, eb_mac_type_bridge_group, ETH_ALEN);
		memcpy(mask, eb_msk_type_bridge_group, ETH_ALEN);
		return 0;
	}
	if ( (p = strrchr(from, '/')) != NULL) {
		*p = '\0';
		if (!(addr = ether_aton(p + 1)))
			return -1;
		memcpy(mask, addr, ETH_ALEN);
	} else
		memset(mask, 0xff, ETH_ALEN);
	if (!(addr = ether_aton(from)))
		return -1;
	memcpy(to, addr, ETH_ALEN);
	for (i = 0; i < ETH_ALEN; i++)
		to[i] &= mask[i];
	return 0;
}

static int ebt_check_inverse2(const char option[], int argc, char **argv)
{
	if (!option)
		return ebt_invert;
	if (strcmp(option, "!") == 0) {
		if (ebt_invert == 1)
			xtables_error(PARAMETER_PROBLEM,
				      "Double use of '!' not allowed");
		if (optind >= argc)
			optarg = NULL;
		else
			optarg = argv[optind];
		optind++;
		ebt_invert = 1;
		return 1;
	}
	return ebt_invert;
}

/*
 * Glue code to use libxtables
 */
static int parse_rule_number(const char *rule)
{
	unsigned int rule_nr;

	if (!xtables_strtoui(rule, NULL, &rule_nr, 1, INT_MAX))
		xtables_error(PARAMETER_PROBLEM,
			      "Invalid rule number `%s'", rule);

	return rule_nr;
}

static int
append_entry(struct nft_handle *h,
	     const char *chain,
	     const char *table,
	     struct iptables_command_state *cs,
	     int rule_nr,
	     bool verbose, bool append)
{
	int ret = 1;

	if (append)
		ret = nft_cmd_rule_append(h, chain, table, cs, NULL, verbose);
	else
		ret = nft_cmd_rule_insert(h, chain, table, cs, rule_nr, verbose);

	return ret;
}

static int
delete_entry(struct nft_handle *h,
	     const char *chain,
	     const char *table,
	     struct iptables_command_state *cs,
	     int rule_nr,
	     int rule_nr_end,
	     bool verbose)
{
	int ret = 1;

	if (rule_nr == -1)
		ret = nft_cmd_rule_delete(h, chain, table, cs, verbose);
	else {
		do {
			ret = nft_cmd_rule_delete_num(h, chain, table,
						  rule_nr, verbose);
			rule_nr++;
		} while (rule_nr < rule_nr_end);
	}

	return ret;
}

int ebt_get_current_chain(const char *chain)
{
	if (!chain)
		return -1;

	if (strcmp(chain, "PREROUTING") == 0)
		return NF_BR_PRE_ROUTING;
	else if (strcmp(chain, "INPUT") == 0)
		return NF_BR_LOCAL_IN;
	else if (strcmp(chain, "FORWARD") == 0)
		return NF_BR_FORWARD;
	else if (strcmp(chain, "OUTPUT") == 0)
		return NF_BR_LOCAL_OUT;
	else if (strcmp(chain, "POSTROUTING") == 0)
		return NF_BR_POST_ROUTING;

	/* placeholder for user defined chain */
	return NF_BR_NUMHOOKS;
}

/*
 * The original ebtables parser
 */

/* Checks whether a command has already been specified */
#define OPT_COMMANDS (flags & OPT_COMMAND || flags & OPT_ZERO)

#define OPT_COMMAND	0x01
#define OPT_TABLE	0x02
#define OPT_IN		0x04
#define OPT_OUT		0x08
#define OPT_JUMP	0x10
#define OPT_PROTOCOL	0x20
#define OPT_SOURCE	0x40
#define OPT_DEST	0x80
#define OPT_ZERO	0x100
#define OPT_LOGICALIN	0x200
#define OPT_LOGICALOUT	0x400
#define OPT_KERNELDATA	0x800 /* This value is also defined in ebtablesd.c */
#define OPT_COUNT	0x1000 /* This value is also defined in libebtc.c */
#define OPT_CNT_INCR	0x2000 /* This value is also defined in libebtc.c */
#define OPT_CNT_DECR	0x4000 /* This value is also defined in libebtc.c */

/* Default command line options. Do not mess around with the already
 * assigned numbers unless you know what you are doing */
struct option ebt_original_options[] =
{
	{ "append"         , required_argument, 0, 'A' },
	{ "insert"         , required_argument, 0, 'I' },
	{ "delete"         , required_argument, 0, 'D' },
	{ "list"           , optional_argument, 0, 'L' },
	{ "Lc"             , no_argument      , 0, 4   },
	{ "Ln"             , no_argument      , 0, 5   },
	{ "Lx"             , no_argument      , 0, 6   },
	{ "Lmac2"          , no_argument      , 0, 12  },
	{ "zero"           , optional_argument, 0, 'Z' },
	{ "flush"          , optional_argument, 0, 'F' },
	{ "policy"         , required_argument, 0, 'P' },
	{ "in-interface"   , required_argument, 0, 'i' },
	{ "in-if"          , required_argument, 0, 'i' },
	{ "logical-in"     , required_argument, 0, 2   },
	{ "logical-out"    , required_argument, 0, 3   },
	{ "out-interface"  , required_argument, 0, 'o' },
	{ "out-if"         , required_argument, 0, 'o' },
	{ "version"        , no_argument      , 0, 'V' },
	{ "help"           , no_argument      , 0, 'h' },
	{ "jump"           , required_argument, 0, 'j' },
	{ "set-counters"   , required_argument, 0, 'c' },
	{ "change-counters", required_argument, 0, 'C' },
	{ "proto"          , required_argument, 0, 'p' },
	{ "protocol"       , required_argument, 0, 'p' },
	{ "db"             , required_argument, 0, 'b' },
	{ "source"         , required_argument, 0, 's' },
	{ "src"            , required_argument, 0, 's' },
	{ "destination"    , required_argument, 0, 'd' },
	{ "dst"            , required_argument, 0, 'd' },
	{ "table"          , required_argument, 0, 't' },
	{ "modprobe"       , required_argument, 0, 'M' },
	{ "new-chain"      , required_argument, 0, 'N' },
	{ "rename-chain"   , required_argument, 0, 'E' },
	{ "delete-chain"   , optional_argument, 0, 'X' },
	{ "atomic-init"    , no_argument      , 0, 7   },
	{ "atomic-commit"  , no_argument      , 0, 8   },
	{ "atomic-file"    , required_argument, 0, 9   },
	{ "atomic-save"    , no_argument      , 0, 10  },
	{ "init-table"     , no_argument      , 0, 11  },
	{ "concurrent"     , no_argument      , 0, 13  },
	{ 0 }
};

extern void xtables_exit_error(enum xtables_exittype status, const char *msg, ...) __attribute__((noreturn, format(printf,2,3)));
struct xtables_globals ebtables_globals = {
	.option_offset 		= 0,
	.program_version	= PACKAGE_VERSION,
	.orig_opts		= ebt_original_options,
	.exit_err		= xtables_exit_error,
	.compat_rev		= nft_compatible_revision,
};

#define opts ebtables_globals.opts
#define prog_name ebtables_globals.program_name
#define prog_vers ebtables_globals.program_version

/*
 * From libebtc.c
 */

/* Prints all registered extensions */
static void ebt_list_extensions(const struct xtables_target *t,
				const struct xtables_rule_match *m)
{
	printf("%s v%s\n", prog_name, prog_vers);
	printf("Loaded userspace extensions:\n");
	/*printf("\nLoaded tables:\n");
        while (tbl) {
		printf("%s\n", tbl->name);
                tbl = tbl->next;
	}*/
	printf("\nLoaded targets:\n");
        for (t = xtables_targets; t; t = t->next) {
		printf("%s\n", t->name);
	}
	printf("\nLoaded matches:\n");
        for (; m != NULL; m = m->next)
		printf("%s\n", m->match->name);
	/*printf("\nLoaded watchers:\n");
        while (w) {
		printf("%s\n", w->name);
                w = w->next;
	}*/
}

#define OPTION_OFFSET 256
static struct option *merge_options(struct option *oldopts,
				    const struct option *newopts,
				    unsigned int *options_offset)
{
	unsigned int num_old, num_new, i;
	struct option *merge;

	if (!newopts || !oldopts || !options_offset)
		return oldopts;
	for (num_old = 0; oldopts[num_old].name; num_old++);
	for (num_new = 0; newopts[num_new].name; num_new++);

	ebtables_globals.option_offset += OPTION_OFFSET;
	*options_offset = ebtables_globals.option_offset;

	merge = malloc(sizeof(struct option) * (num_new + num_old + 1));
	if (!merge)
		return NULL;
	memcpy(merge, oldopts, num_old * sizeof(struct option));
	for (i = 0; i < num_new; i++) {
		merge[num_old + i] = newopts[i];
		merge[num_old + i].val += *options_offset;
	}
	memset(merge + num_old + num_new, 0, sizeof(struct option));
	/* Only free dynamically allocated stuff */
	if (oldopts != ebt_original_options)
		free(oldopts);

	return merge;
}

static void print_help(const struct xtables_target *t,
		       const struct xtables_rule_match *m, const char *table)
{
	printf("%s %s\n", prog_name, prog_vers);
	printf(
"Usage:\n"
"ebtables -[ADI] chain rule-specification [options]\n"
"ebtables -P chain target\n"
"ebtables -[LFZ] [chain]\n"
"ebtables -[NX] [chain]\n"
"ebtables -E old-chain-name new-chain-name\n\n"
"Commands:\n"
"--append -A chain             : append to chain\n"
"--delete -D chain             : delete matching rule from chain\n"
"--delete -D chain rulenum     : delete rule at position rulenum from chain\n"
"--change-counters -C chain\n"
"          [rulenum] pcnt bcnt : change counters of existing rule\n"
"--insert -I chain rulenum     : insert rule at position rulenum in chain\n"
"--list   -L [chain]           : list the rules in a chain or in all chains\n"
"--flush  -F [chain]           : delete all rules in chain or in all chains\n"
"--init-table                  : replace the kernel table with the initial table\n"
"--zero   -Z [chain]           : put counters on zero in chain or in all chains\n"
"--policy -P chain target      : change policy on chain to target\n"
"--new-chain -N chain          : create a user defined chain\n"
"--rename-chain -E old new     : rename a chain\n"
"--delete-chain -X [chain]     : delete a user defined chain\n"
"--atomic-commit               : update the kernel w/t table contained in <FILE>\n"
"--atomic-init                 : put the initial kernel table into <FILE>\n"
"--atomic-save                 : put the current kernel table into <FILE>\n"
"--atomic-file file            : set <FILE> to file\n\n"
"Options:\n"
"--proto  -p [!] proto         : protocol hexadecimal, by name or LENGTH\n"
"--src    -s [!] address[/mask]: source mac address\n"
"--dst    -d [!] address[/mask]: destination mac address\n"
"--in-if  -i [!] name[+]       : network input interface name\n"
"--out-if -o [!] name[+]       : network output interface name\n"
"--logical-in  [!] name[+]     : logical bridge input interface name\n"
"--logical-out [!] name[+]     : logical bridge output interface name\n"
"--set-counters -c chain\n"
"          pcnt bcnt           : set the counters of the to be added rule\n"
"--modprobe -M program         : try to insert modules using this program\n"
"--concurrent                  : use a file lock to support concurrent scripts\n"
"--version -V                  : print package version\n\n"
"Environment variable:\n"
/*ATOMIC_ENV_VARIABLE "          : if set <FILE> (see above) will equal its value"*/
"\n\n");
	for (; m != NULL; m = m->next) {
		printf("\n");
		m->match->help();
	}
	if (t != NULL) {
		printf("\n");
		t->help();
	}

//	if (table->help)
//		table->help(ebt_hooknames);
}

/* Execute command L */
static int list_rules(struct nft_handle *h, const char *chain, const char *table,
		      int rule_nr, int verbose, int numeric, int expanded,
		      int linenumbers, int counters)
{
	unsigned int format;

	format = FMT_OPTIONS | FMT_C_COUNTS;
	if (verbose)
		format |= FMT_VIA;

	if (numeric)
		format |= FMT_NUMERIC;

	if (!expanded)
		format |= FMT_KILOMEGAGIGA;

	if (linenumbers)
		format |= FMT_LINENUMBERS;

	if (!counters)
		format |= FMT_NOCOUNTS;

	return nft_cmd_rule_list(h, chain, table, rule_nr, format);
}

static int parse_rule_range(const char *argv, int *rule_nr, int *rule_nr_end)
{
	char *colon = strchr(argv, ':'), *buffer;

	if (colon) {
		*colon = '\0';
		if (*(colon + 1) == '\0')
			*rule_nr_end = -1; /* Until the last rule */
		else {
			*rule_nr_end = strtol(colon + 1, &buffer, 10);
			if (*buffer != '\0' || *rule_nr_end == 0)
				return -1;
		}
	}
	if (colon == argv)
		*rule_nr = 1; /* Beginning with the first rule */
	else {
		*rule_nr = strtol(argv, &buffer, 10);
		if (*buffer != '\0' || *rule_nr == 0)
			return -1;
	}
	if (!colon)
		*rule_nr_end = *rule_nr;
	return 0;
}

/* Incrementing or decrementing rules in daemon mode is not supported as the
 * involved code overload is not worth it (too annoying to take the increased
 * counters in the kernel into account). */
static int parse_change_counters_rule(int argc, char **argv, int *rule_nr, int *rule_nr_end, struct iptables_command_state *cs)
{
	char *buffer;
	int ret = 0;

	if (optind + 1 >= argc || argv[optind][0] == '-' || argv[optind + 1][0] == '-')
		xtables_error(PARAMETER_PROBLEM,
			      "The command -C needs at least 2 arguments");
	if (optind + 2 < argc && (argv[optind + 2][0] != '-' || (argv[optind + 2][1] >= '0' && argv[optind + 2][1] <= '9'))) {
		if (optind + 3 != argc)
			xtables_error(PARAMETER_PROBLEM,
				      "No extra options allowed with -C start_nr[:end_nr] pcnt bcnt");
		if (parse_rule_range(argv[optind], rule_nr, rule_nr_end))
			xtables_error(PARAMETER_PROBLEM,
				      "Something is wrong with the rule number specification '%s'", argv[optind]);
		optind++;
	}

	if (argv[optind][0] == '+') {
		ret += 1;
		cs->counters.pcnt = strtoull(argv[optind] + 1, &buffer, 10);
	} else if (argv[optind][0] == '-') {
		ret += 2;
		cs->counters.pcnt = strtoull(argv[optind] + 1, &buffer, 10);
	} else
		cs->counters.pcnt = strtoull(argv[optind], &buffer, 10);

	if (*buffer != '\0')
		goto invalid;
	optind++;
	if (argv[optind][0] == '+') {
		ret += 3;
		cs->counters.bcnt = strtoull(argv[optind] + 1, &buffer, 10);
	} else if (argv[optind][0] == '-') {
		ret += 6;
		cs->counters.bcnt = strtoull(argv[optind] + 1, &buffer, 10);
	} else
		cs->counters.bcnt = strtoull(argv[optind], &buffer, 10);

	if (*buffer != '\0')
		goto invalid;
	optind++;
	return ret;
invalid:
	xtables_error(PARAMETER_PROBLEM,"Packet counter '%s' invalid", argv[optind]);
}

static void ebtables_parse_interface(const char *arg, char *vianame)
{
	unsigned char mask[IFNAMSIZ];
	char *c;

	xtables_parse_interface(arg, vianame, mask);

	if ((c = strchr(vianame, '+'))) {
		if (*(c + 1) != '\0')
			xtables_error(PARAMETER_PROBLEM,
				      "Spurious characters after '+' wildcard");
	}
}

/* This code is very similar to iptables/xtables.c:command_match() */
static void ebt_load_match(const char *name)
{
	struct xtables_match *m;
	size_t size;

	m = xtables_find_match(name, XTF_TRY_LOAD, NULL);
	if (m == NULL) {
		fprintf(stderr, "Unable to load %s match\n", name);
		return;
	}

	size = XT_ALIGN(sizeof(struct xt_entry_match)) + m->size;
	m->m = xtables_calloc(1, size);
	m->m->u.match_size = size;
	strcpy(m->m->u.user.name, m->name);
	m->m->u.user.revision = m->revision;
	xs_init_match(m);

	opts = merge_options(opts, m->extra_opts, &m->option_offset);
	if (opts == NULL)
		xtables_error(OTHER_PROBLEM, "Can't alloc memory");
}

static void __ebt_load_watcher(const char *name, const char *typename)
{
	struct xtables_target *watcher;
	size_t size;

	watcher = xtables_find_target(name, XTF_TRY_LOAD);
	if (!watcher) {
		fprintf(stderr, "Unable to load %s %s\n", name, typename);
		return;
	}

	size = XT_ALIGN(sizeof(struct xt_entry_target)) + watcher->size;

	watcher->t = xtables_calloc(1, size);
	watcher->t->u.target_size = size;
	snprintf(watcher->t->u.user.name,
		sizeof(watcher->t->u.user.name), "%s", name);
	watcher->t->u.user.name[sizeof(watcher->t->u.user.name)-1] = '\0';
	watcher->t->u.user.revision = watcher->revision;

	xs_init_target(watcher);

	opts = merge_options(opts, watcher->extra_opts,
			     &watcher->option_offset);
	if (opts == NULL)
		xtables_error(OTHER_PROBLEM, "Can't alloc memory");
}

static void ebt_load_watcher(const char *name)
{
	return __ebt_load_watcher(name, "watcher");
}

static void ebt_load_target(const char *name)
{
	return __ebt_load_watcher(name, "target");
}

void ebt_load_match_extensions(void)
{
	opts = ebt_original_options;
	ebt_load_match("802_3");
	ebt_load_match("arp");
	ebt_load_match("ip");
	ebt_load_match("ip6");
	ebt_load_match("mark_m");
	ebt_load_match("limit");
	ebt_load_match("pkttype");
	ebt_load_match("vlan");
	ebt_load_match("stp");
	ebt_load_match("among");

	ebt_load_watcher("log");
	ebt_load_watcher("nflog");

	ebt_load_target("mark");
	ebt_load_target("dnat");
	ebt_load_target("snat");
	ebt_load_target("arpreply");
	ebt_load_target("redirect");
	ebt_load_target("standard");
}

void ebt_add_match(struct xtables_match *m,
		   struct iptables_command_state *cs)
{
	struct xtables_rule_match **rule_matches = &cs->matches;
	struct xtables_match *newm;
	struct ebt_match *newnode, **matchp;
	struct xt_entry_match *m2;

	newm = xtables_find_match(m->name, XTF_LOAD_MUST_SUCCEED, rule_matches);
	if (newm == NULL)
		xtables_error(OTHER_PROBLEM,
			      "Unable to add match %s", m->name);

	m2 = xtables_calloc(1, newm->m->u.match_size);
	memcpy(m2, newm->m, newm->m->u.match_size);
	memset(newm->m->data, 0, newm->size);
	xs_init_match(newm);
	newm->m = m2;

	newm->mflags = m->mflags;
	m->mflags = 0;

	/* glue code for watchers */
	newnode = calloc(1, sizeof(struct ebt_match));
	if (newnode == NULL)
		xtables_error(OTHER_PROBLEM, "Unable to alloc memory");

	newnode->ismatch = true;
	newnode->u.match = newm;

	for (matchp = &cs->match_list; *matchp; matchp = &(*matchp)->next)
		;
	*matchp = newnode;
}

void ebt_add_watcher(struct xtables_target *watcher,
		     struct iptables_command_state *cs)
{
	struct ebt_match *newnode, **matchp;
	struct xtables_target *clone;

	clone = xtables_malloc(sizeof(struct xtables_target));
	memcpy(clone, watcher, sizeof(struct xtables_target));
	clone->udata = NULL;
	clone->tflags = watcher->tflags;
	clone->next = clone;

	clone->t = xtables_calloc(1, watcher->t->u.target_size);
	memcpy(clone->t, watcher->t, watcher->t->u.target_size);

	memset(watcher->t->data, 0, watcher->size);
	xs_init_target(watcher);
	watcher->tflags = 0;


	newnode = calloc(1, sizeof(struct ebt_match));
	if (newnode == NULL)
		xtables_error(OTHER_PROBLEM, "Unable to alloc memory");

	newnode->u.watcher = clone;

	for (matchp = &cs->match_list; *matchp; matchp = &(*matchp)->next)
		;
	*matchp = newnode;
}

int ebt_command_default(struct iptables_command_state *cs)
{
	struct xtables_target *t = cs->target;
	struct xtables_match *m;
	struct ebt_match *matchp;

	/* Is it a target option? */
	if (t && t->parse) {
		if (t->parse(cs->c - t->option_offset, cs->argv,
			     ebt_invert, &t->tflags, NULL, &t->t))
			return 0;
	}

	/* check previously added matches/watchers to this rule first */
	for (matchp = cs->match_list; matchp; matchp = matchp->next) {
		if (matchp->ismatch) {
			m = matchp->u.match;
			if (m->parse &&
			    m->parse(cs->c - m->option_offset, cs->argv,
				     ebt_invert, &m->mflags, NULL, &m->m))
				return 0;
		} else {
			t = matchp->u.watcher;
			if (t->parse &&
			    t->parse(cs->c - t->option_offset, cs->argv,
				     ebt_invert, &t->tflags, NULL, &t->t))
				return 0;
		}
	}

	/* Is it a match_option? */
	for (m = xtables_matches; m; m = m->next) {
		if (m->parse &&
		    m->parse(cs->c - m->option_offset, cs->argv,
			     ebt_invert, &m->mflags, NULL, &m->m)) {
			ebt_add_match(m, cs);
			return 0;
		}
	}

	/* Is it a watcher option? */
	for (t = xtables_targets; t; t = t->next) {
		if (t->parse &&
		    t->parse(cs->c - t->option_offset, cs->argv,
			     ebt_invert, &t->tflags, NULL, &t->t)) {
			ebt_add_watcher(t, cs);
			return 0;
		}
	}
	return 1;
}

int nft_init_eb(struct nft_handle *h, const char *pname)
{
	ebtables_globals.program_name = pname;
	if (xtables_init_all(&ebtables_globals, NFPROTO_BRIDGE) < 0) {
		fprintf(stderr, "%s/%s Failed to initialize ebtables-compat\n",
			ebtables_globals.program_name,
			ebtables_globals.program_version);
		exit(1);
	}

#if defined(ALL_INCLUSIVE) || defined(NO_SHARED_LIBS)
	init_extensionsb();
#endif

	if (nft_init(h, NFPROTO_BRIDGE, xtables_bridge) < 0)
		xtables_error(OTHER_PROBLEM,
			      "Could not initialize nftables layer.");

	/* manually registering ebt matches, given the original ebtables parser
	 * don't use '-m matchname' and the match can't be loaded dynamically when
	 * the user calls it.
	 */
	ebt_load_match_extensions();

	return 0;
}

void nft_fini_eb(struct nft_handle *h)
{
	struct xtables_match *match;
	struct xtables_target *target;

	for (match = xtables_matches; match; match = match->next) {
		free(match->m);
	}
	for (target = xtables_targets; target; target = target->next) {
		free(target->t);
	}

	free(opts);

	nft_fini(h);
	xtables_fini();
}

int do_commandeb(struct nft_handle *h, int argc, char *argv[], char **table,
		 bool restore)
{
	char *buffer;
	int c, i;
	int chcounter = 0; /* Needed for -C */
	int rule_nr = 0;
	int rule_nr_end = 0;
	int ret = 0;
	unsigned int flags = 0;
	struct xtables_target *t;
	struct iptables_command_state cs = {
		.argv = argv,
		.jumpto	= "",
		.eb.bitmask = EBT_NOPROTO,
	};
	char command = 'h';
	const char *chain = NULL;
	const char *policy = NULL;
	int selected_chain = -1;
	struct xtables_rule_match *xtrm_i;
	struct ebt_match *match;
	bool table_set = false;

	/* prevent getopt to spoil our error reporting */
	optind = 0;
	opterr = false;

	/* Getopt saves the day */
	while ((c = getopt_long(argc, argv,
	   "-A:D:C:I:N:E:X::L::Z::F::P:Vhi:o:j:c:p:s:d:t:M:", opts, NULL)) != -1) {
		cs.c = c;
		cs.invert = ebt_invert;
		switch (c) {

		case 'A': /* Add a rule */
		case 'D': /* Delete a rule */
		case 'C': /* Change counters */
		case 'P': /* Define policy */
		case 'I': /* Insert a rule */
		case 'N': /* Make a user defined chain */
		case 'E': /* Rename chain */
		case 'X': /* Delete chain */
			/* We allow -N chainname -P policy */
			if (command == 'N' && c == 'P') {
				command = c;
				optind--; /* No table specified */
				goto handle_P;
			}
			if (OPT_COMMANDS)
				xtables_error(PARAMETER_PROBLEM,
					      "Multiple commands are not allowed");

			command = c;
			if (optarg && (optarg[0] == '-' || !strcmp(optarg, "!")))
				xtables_error(PARAMETER_PROBLEM, "No chain name specified");
			chain = optarg;
			selected_chain = ebt_get_current_chain(chain);
			flags |= OPT_COMMAND;

			if (c == 'N') {
				ret = nft_cmd_chain_user_add(h, chain, *table);
				break;
			} else if (c == 'X') {
				/* X arg is optional, optarg is NULL */
				if (!chain && optind < argc && argv[optind][0] != '-') {
					chain = argv[optind];
					optind++;
				}
				ret = nft_cmd_chain_user_del(h, chain, *table, 0);
				break;
			}

			if (c == 'E') {
				if (optind >= argc)
					xtables_error(PARAMETER_PROBLEM, "No new chain name specified");
				else if (optind < argc - 1)
					xtables_error(PARAMETER_PROBLEM, "No extra options allowed with -E");
				else if (strlen(argv[optind]) >= NFT_CHAIN_MAXNAMELEN)
					xtables_error(PARAMETER_PROBLEM, "Chain name length can't exceed %d"" characters", NFT_CHAIN_MAXNAMELEN - 1);
				else if (strchr(argv[optind], ' ') != NULL)
					xtables_error(PARAMETER_PROBLEM, "Use of ' ' not allowed in chain names");

				ret = nft_cmd_chain_user_rename(h, chain, *table,
							    argv[optind]);
				if (ret != 0 && errno == ENOENT)
					xtables_error(PARAMETER_PROBLEM, "Chain '%s' doesn't exists", chain);

				optind++;
				break;
			} else if (c == 'D' && optind < argc && (argv[optind][0] != '-' || (argv[optind][1] >= '0' && argv[optind][1] <= '9'))) {
				if (optind != argc - 1)
					xtables_error(PARAMETER_PROBLEM,
							 "No extra options allowed with -D start_nr[:end_nr]");
				if (parse_rule_range(argv[optind], &rule_nr, &rule_nr_end))
					xtables_error(PARAMETER_PROBLEM,
							 "Problem with the specified rule number(s) '%s'", argv[optind]);
				optind++;
			} else if (c == 'C') {
				if ((chcounter = parse_change_counters_rule(argc, argv, &rule_nr, &rule_nr_end, &cs)) == -1)
					return -1;
			} else if (c == 'I') {
				if (optind >= argc || (argv[optind][0] == '-' && (argv[optind][1] < '0' || argv[optind][1] > '9')))
					rule_nr = 1;
				else {
					rule_nr = parse_rule_number(argv[optind]);
					optind++;
				}
			} else if (c == 'P') {
handle_P:
				if (optind >= argc)
					xtables_error(PARAMETER_PROBLEM,
						      "No policy specified");
				for (i = 0; i < NUM_STANDARD_TARGETS; i++)
					if (!strcmp(argv[optind], nft_ebt_standard_target(i))) {
						policy = argv[optind];
						if (-i-1 == EBT_CONTINUE)
							xtables_error(PARAMETER_PROBLEM,
								      "Wrong policy '%s'",
								      argv[optind]);
						break;
					}
				if (i == NUM_STANDARD_TARGETS)
					xtables_error(PARAMETER_PROBLEM,
						      "Unknown policy '%s'", argv[optind]);
				optind++;
			}
			break;
		case 'L': /* List */
		case 'F': /* Flush */
		case 'Z': /* Zero counters */
			if (c == 'Z') {
				if ((flags & OPT_ZERO) || (flags & OPT_COMMAND && command != 'L'))
print_zero:
					xtables_error(PARAMETER_PROBLEM,
						      "Command -Z only allowed together with command -L");
				flags |= OPT_ZERO;
			} else {
				if (flags & OPT_COMMAND)
					xtables_error(PARAMETER_PROBLEM,
						      "Multiple commands are not allowed");
				command = c;
				flags |= OPT_COMMAND;
				if (flags & OPT_ZERO && c != 'L')
					goto print_zero;
			}

			if (optind < argc && argv[optind][0] != '-') {
				chain = argv[optind];
				optind++;
			}
			break;
		case 'V': /* Version */
			if (OPT_COMMANDS)
				xtables_error(PARAMETER_PROBLEM,
					      "Multiple commands are not allowed");
			printf("%s %s (nf_tables)\n", prog_name, prog_vers);
			exit(0);
		case 'h': /* Help */
			if (OPT_COMMANDS)
				xtables_error(PARAMETER_PROBLEM,
					      "Multiple commands are not allowed");
			command = 'h';

			/* All other arguments should be extension names */
			while (optind < argc) {
				/*struct ebt_u_match *m;
				struct ebt_u_watcher *w;*/

				if (!strcasecmp("list_extensions", argv[optind])) {
					ebt_list_extensions(xtables_targets, cs.matches);
					exit(0);
				}
				/*if ((m = ebt_find_match(argv[optind])))
					ebt_add_match(new_entry, m);
				else if ((w = ebt_find_watcher(argv[optind])))
					ebt_add_watcher(new_entry, w);
				else {*/
					if (!(t = xtables_find_target(argv[optind], XTF_TRY_LOAD)))
						xtables_error(PARAMETER_PROBLEM,"Extension '%s' not found", argv[optind]);
					if (flags & OPT_JUMP)
						xtables_error(PARAMETER_PROBLEM,"Sorry, you can only see help for one target extension at a time");
					flags |= OPT_JUMP;
					cs.target = t;
				//}
				optind++;
			}
			break;
		case 't': /* Table */
			ebt_check_option2(&flags, OPT_TABLE);
			if (restore && table_set)
				xtables_error(PARAMETER_PROBLEM,
					      "The -t option (seen in line %u) cannot be used in %s.\n",
					      line, xt_params->program_name);
			if (strlen(optarg) > EBT_TABLE_MAXNAMELEN - 1)
				xtables_error(PARAMETER_PROBLEM,
					      "Table name length cannot exceed %d characters",
					      EBT_TABLE_MAXNAMELEN - 1);
			*table = optarg;
			table_set = true;
			break;
		case 'i': /* Input interface */
		case 2  : /* Logical input interface */
		case 'o': /* Output interface */
		case 3  : /* Logical output interface */
		case 'j': /* Target */
		case 'p': /* Net family protocol */
		case 's': /* Source mac */
		case 'd': /* Destination mac */
		case 'c': /* Set counters */
			if (!OPT_COMMANDS)
				xtables_error(PARAMETER_PROBLEM,
					      "No command specified");
			if (command != 'A' && command != 'D' && command != 'I' && command != 'C')
				xtables_error(PARAMETER_PROBLEM,
					      "Command and option do not match");
			if (c == 'i') {
				ebt_check_option2(&flags, OPT_IN);
				if (selected_chain > 2 && selected_chain < NF_BR_BROUTING)
					xtables_error(PARAMETER_PROBLEM,
						      "Use -i only in INPUT, FORWARD, PREROUTING and BROUTING chains");
				if (ebt_check_inverse2(optarg, argc, argv))
					cs.eb.invflags |= EBT_IIN;

				ebtables_parse_interface(optarg, cs.eb.in);
				break;
			} else if (c == 2) {
				ebt_check_option2(&flags, OPT_LOGICALIN);
				if (selected_chain > 2 && selected_chain < NF_BR_BROUTING)
					xtables_error(PARAMETER_PROBLEM,
						      "Use --logical-in only in INPUT, FORWARD, PREROUTING and BROUTING chains");
				if (ebt_check_inverse2(optarg, argc, argv))
					cs.eb.invflags |= EBT_ILOGICALIN;

				ebtables_parse_interface(optarg, cs.eb.logical_in);
				break;
			} else if (c == 'o') {
				ebt_check_option2(&flags, OPT_OUT);
				if (selected_chain < 2 || selected_chain == NF_BR_BROUTING)
					xtables_error(PARAMETER_PROBLEM,
						      "Use -o only in OUTPUT, FORWARD and POSTROUTING chains");
				if (ebt_check_inverse2(optarg, argc, argv))
					cs.eb.invflags |= EBT_IOUT;

				ebtables_parse_interface(optarg, cs.eb.out);
				break;
			} else if (c == 3) {
				ebt_check_option2(&flags, OPT_LOGICALOUT);
				if (selected_chain < 2 || selected_chain == NF_BR_BROUTING)
					xtables_error(PARAMETER_PROBLEM,
						      "Use --logical-out only in OUTPUT, FORWARD and POSTROUTING chains");
				if (ebt_check_inverse2(optarg, argc, argv))
					cs.eb.invflags |= EBT_ILOGICALOUT;

				ebtables_parse_interface(optarg, cs.eb.logical_out);
				break;
			} else if (c == 'j') {
				ebt_check_option2(&flags, OPT_JUMP);
				if (strcmp(optarg, "CONTINUE") != 0) {
					command_jump(&cs, optarg);
				}
				break;
			} else if (c == 's') {
				ebt_check_option2(&flags, OPT_SOURCE);
				if (ebt_check_inverse2(optarg, argc, argv))
					cs.eb.invflags |= EBT_ISOURCE;

				if (ebt_get_mac_and_mask(optarg, cs.eb.sourcemac, cs.eb.sourcemsk))
					xtables_error(PARAMETER_PROBLEM, "Problem with specified source mac '%s'", optarg);
				cs.eb.bitmask |= EBT_SOURCEMAC;
				break;
			} else if (c == 'd') {
				ebt_check_option2(&flags, OPT_DEST);
				if (ebt_check_inverse2(optarg, argc, argv))
					cs.eb.invflags |= EBT_IDEST;

				if (ebt_get_mac_and_mask(optarg, cs.eb.destmac, cs.eb.destmsk))
					xtables_error(PARAMETER_PROBLEM, "Problem with specified destination mac '%s'", optarg);
				cs.eb.bitmask |= EBT_DESTMAC;
				break;
			} else if (c == 'c') {
				ebt_check_option2(&flags, OPT_COUNT);
				if (ebt_check_inverse2(optarg, argc, argv))
					xtables_error(PARAMETER_PROBLEM,
						      "Unexpected '!' after -c");
				if (optind >= argc || optarg[0] == '-' || argv[optind][0] == '-')
					xtables_error(PARAMETER_PROBLEM,
						      "Option -c needs 2 arguments");

				cs.counters.pcnt = strtoull(optarg, &buffer, 10);
				if (*buffer != '\0')
					xtables_error(PARAMETER_PROBLEM,
						      "Packet counter '%s' invalid",
						      optarg);
				cs.counters.bcnt = strtoull(argv[optind], &buffer, 10);
				if (*buffer != '\0')
					xtables_error(PARAMETER_PROBLEM,
						      "Packet counter '%s' invalid",
						      argv[optind]);
				optind++;
				break;
			}
			ebt_check_option2(&flags, OPT_PROTOCOL);
			if (ebt_check_inverse2(optarg, argc, argv))
				cs.eb.invflags |= EBT_IPROTO;

			cs.eb.bitmask &= ~((unsigned int)EBT_NOPROTO);
			i = strtol(optarg, &buffer, 16);
			if (*buffer == '\0' && (i < 0 || i > 0xFFFF))
				xtables_error(PARAMETER_PROBLEM,
					      "Problem with the specified protocol");
			if (*buffer != '\0') {
				struct xt_ethertypeent *ent;

				if (!strcasecmp(optarg, "LENGTH")) {
					cs.eb.bitmask |= EBT_802_3;
					break;
				}
				ent = xtables_getethertypebyname(optarg);
				if (!ent)
					xtables_error(PARAMETER_PROBLEM,
						      "Problem with the specified Ethernet protocol '%s', perhaps "XT_PATH_ETHERTYPES " is missing", optarg);
				cs.eb.ethproto = ent->e_ethertype;
			} else
				cs.eb.ethproto = i;

			if (cs.eb.ethproto < 0x0600)
				xtables_error(PARAMETER_PROBLEM,
					      "Sorry, protocols have values above or equal to 0x0600");
			break;
		case 4  : /* Lc */
			ebt_check_option2(&flags, LIST_C);
			if (command != 'L')
				xtables_error(PARAMETER_PROBLEM,
					      "Use --Lc with -L");
			flags |= LIST_C;
			break;
		case 5  : /* Ln */
			ebt_check_option2(&flags, LIST_N);
			if (command != 'L')
				xtables_error(PARAMETER_PROBLEM,
					      "Use --Ln with -L");
			if (flags & LIST_X)
				xtables_error(PARAMETER_PROBLEM,
					      "--Lx is not compatible with --Ln");
			flags |= LIST_N;
			break;
		case 6  : /* Lx */
			ebt_check_option2(&flags, LIST_X);
			if (command != 'L')
				xtables_error(PARAMETER_PROBLEM,
					      "Use --Lx with -L");
			if (flags & LIST_N)
				xtables_error(PARAMETER_PROBLEM,
					      "--Lx is not compatible with --Ln");
			flags |= LIST_X;
			break;
		case 12 : /* Lmac2 */
			ebt_check_option2(&flags, LIST_MAC2);
			if (command != 'L')
				xtables_error(PARAMETER_PROBLEM,
					       "Use --Lmac2 with -L");
			flags |= LIST_MAC2;
			break;
		case 8 : /* atomic-commit */
/*
			replace->command = c;
			if (OPT_COMMANDS)
				ebt_print_error2("Multiple commands are not allowed");
			replace->flags |= OPT_COMMAND;
			if (!replace->filename)
				ebt_print_error2("No atomic file specified");*/
			/* Get the information from the file */
			/*ebt_get_table(replace, 0);*/
			/* We don't want the kernel giving us its counters,
			 * they would overwrite the counters extracted from
			 * the file */
			/*replace->num_counters = 0;*/
			/* Make sure the table will be written to the kernel */
			/*free(replace->filename);
			replace->filename = NULL;
			break;*/
		/*case 7 :*/ /* atomic-init */
		/*case 10:*/ /* atomic-save */
		case 11: /* init-table */
			nft_cmd_table_flush(h, *table);
			return 1;
		/*
			replace->command = c;
			if (OPT_COMMANDS)
				ebt_print_error2("Multiple commands are not allowed");
			if (c != 11 && !replace->filename)
				ebt_print_error2("No atomic file specified");
			replace->flags |= OPT_COMMAND;
			{
				char *tmp = replace->filename;*/

				/* Get the kernel table */
				/*replace->filename = NULL;
				ebt_get_kernel_table(replace, c == 10 ? 0 : 1);
				replace->filename = tmp;
			}
			break;
		case 9 :*/ /* atomic */
			/*
			if (OPT_COMMANDS)
				ebt_print_error2("--atomic has to come before the command");*/
			/* A possible memory leak here, but this is not
			 * executed in daemon mode */
			/*replace->filename = (char *)malloc(strlen(optarg) + 1);
			strcpy(replace->filename, optarg);
			break; */
		case 13 :
			break;
		case 1 :
			if (!strcmp(optarg, "!"))
				ebt_check_inverse2(optarg, argc, argv);
			else
				xtables_error(PARAMETER_PROBLEM,
					      "Bad argument : '%s'", optarg);
			/* ebt_ebt_check_inverse2() did optind++ */
			optind--;
			continue;
		default:
			ebt_check_inverse2(optarg, argc, argv);

			if (ebt_command_default(&cs))
				xtables_error(PARAMETER_PROBLEM,
					      "Unknown argument: '%s'",
					      argv[optind]);

			if (command != 'A' && command != 'I' &&
			    command != 'D' && command != 'C')
				xtables_error(PARAMETER_PROBLEM,
					      "Extensions only for -A, -I, -D and -C");
		}
		ebt_invert = 0;
	}

	/* Just in case we didn't catch an error */
	/*if (ebt_errormsg[0] != '\0')
		return -1;

	if (!(table = ebt_find_table(replace->name)))
		ebt_print_error2("Bad table name");*/

	if (command == 'h' && !(flags & OPT_ZERO)) {
		print_help(cs.target, cs.matches, *table);
		ret = 1;
	}

	/* Do the final checks */
	if (command == 'A' || command == 'I' ||
	    command == 'D' || command == 'C') {
		for (xtrm_i = cs.matches; xtrm_i; xtrm_i = xtrm_i->next)
			xtables_option_mfcall(xtrm_i->match);

		for (match = cs.match_list; match; match = match->next) {
			if (match->ismatch)
				continue;

			xtables_option_tfcall(match->u.watcher);
		}

		if (cs.target != NULL)
			xtables_option_tfcall(cs.target);
	}
	/* So, the extensions can work with the host endian.
	 * The kernel does not have to do this of course */
	cs.eb.ethproto = htons(cs.eb.ethproto);

	if (command == 'P') {
		if (selected_chain >= NF_BR_NUMHOOKS) {
			ret = ebt_cmd_user_chain_policy(h, *table, chain, policy);
		} else {
			if (strcmp(policy, "RETURN") == 0) {
				xtables_error(PARAMETER_PROBLEM,
					      "Policy RETURN only allowed for user defined chains");
			}
			ret = nft_cmd_chain_set(h, *table, chain, policy, NULL);
			if (ret < 0)
				xtables_error(PARAMETER_PROBLEM, "Wrong policy");
		}
	} else if (command == 'L') {
		ret = list_rules(h, chain, *table, rule_nr,
				 0,
				 0,
				 /*flags&OPT_EXPANDED*/0,
				 flags&LIST_N,
				 flags&LIST_C);
	}
	if (flags & OPT_ZERO) {
		ret = nft_cmd_chain_zero_counters(h, chain, *table, 0);
	} else if (command == 'F') {
		ret = nft_cmd_rule_flush(h, chain, *table, 0);
	} else if (command == 'A') {
		ret = append_entry(h, chain, *table, &cs, 0, 0, true);
	} else if (command == 'I') {
		ret = append_entry(h, chain, *table, &cs, rule_nr - 1,
				   0, false);
	} else if (command == 'D') {
		ret = delete_entry(h, chain, *table, &cs, rule_nr - 1,
				   rule_nr_end, 0);
	} /*else if (replace->command == 'C') {
		ebt_change_counters(replace, new_entry, rule_nr, rule_nr_end, &(new_entry->cnt_surplus), chcounter);
		if (ebt_errormsg[0] != '\0')
			return -1;
	}*/

	ebt_cs_clean(&cs);
	return ret;
}
