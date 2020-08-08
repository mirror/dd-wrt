/* Code to take an arptables-style command line and do it. */

/*
 * arptables:
 * Author: Bart De Schuymer <bdschuym@pandora.be>, but
 * almost all code is from the iptables userspace program, which has main
 * authors: Paul.Russell@rustcorp.com.au and mneuling@radlogic.com.au
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

/*
  Currently, only support for specifying hardware addresses for Ethernet
  is available.
  This tool is not luser-proof: you can specify an Ethernet source address
  and set hardware length to something different than 6, f.e.
*/
#include "config.h"
#include <getopt.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <dlfcn.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <iptables.h>
#include <xtables.h>

#include "xshared.h"

#include "nft.h"
#include "nft-arp.h"
#include <linux/netfilter_arp/arp_tables.h>

#define NUMBER_OF_OPT	16
static const char optflags[NUMBER_OF_OPT]
= { 'n', 's', 'd', 2, 3, 7, 8, 4, 5, 6, 'j', 'v', 'i', 'o', '0', 'c'};

static struct option original_opts[] = {
	{ "append", 1, 0, 'A' },
	{ "delete", 1, 0,  'D' },
	{ "insert", 1, 0,  'I' },
	{ "replace", 1, 0,  'R' },
	{ "list", 2, 0,  'L' },
	{ "flush", 2, 0,  'F' },
	{ "zero", 2, 0,  'Z' },
	{ "new-chain", 1, 0,  'N' },
	{ "delete-chain", 2, 0,  'X' },
	{ "rename-chain", 1, 0,  'E' },
	{ "policy", 1, 0,  'P' },
	{ "source-ip", 1, 0, 's' },
	{ "destination-ip", 1, 0,  'd' },
	{ "src-ip", 1, 0,  's' },
	{ "dst-ip", 1, 0,  'd' },
	{ "source-mac", 1, 0, 2},
	{ "destination-mac", 1, 0, 3},
	{ "src-mac", 1, 0, 2},
	{ "dst-mac", 1, 0, 3},
	{ "h-length", 1, 0,  'l' },
	{ "p-length", 1, 0,  8 },
	{ "opcode", 1, 0,  4 },
	{ "h-type", 1, 0,  5 },
	{ "proto-type", 1, 0,  6 },
	{ "in-interface", 1, 0, 'i' },
	{ "jump", 1, 0, 'j' },
	{ "table", 1, 0, 't' },
	{ "match", 1, 0, 'm' },
	{ "numeric", 0, 0, 'n' },
	{ "out-interface", 1, 0, 'o' },
	{ "verbose", 0, 0, 'v' },
	{ "exact", 0, 0, 'x' },
	{ "version", 0, 0, 'V' },
	{ "help", 2, 0, 'h' },
	{ "line-numbers", 0, 0, '0' },
	{ "modprobe", 1, 0, 'M' },
	{ "set-counters", 1, 0, 'c' },
	{ 0 }
};

#define opts xt_params->opts

extern void xtables_exit_error(enum xtables_exittype status, const char *msg, ...) __attribute__((noreturn, format(printf,2,3)));
struct xtables_globals arptables_globals = {
	.option_offset		= 0,
	.program_version	= PACKAGE_VERSION,
	.orig_opts		= original_opts,
	.exit_err		= xtables_exit_error,
	.compat_rev		= nft_compatible_revision,
};

/* index relates to bit of each OPT_* value */
static int inverse_for_options[] =
{
/* -n */ 0,
/* -s */ ARPT_INV_SRCIP,
/* -d */ ARPT_INV_TGTIP,
/* -p */ 0,
/* -j */ 0,
/* -v */ 0,
/* -x */ 0,
/* -i */ ARPT_INV_VIA_IN,
/* -o */ ARPT_INV_VIA_OUT,
/*--line*/ 0,
/* -c */ 0,
/* 2 */ ARPT_INV_SRCDEVADDR,
/* 3 */ ARPT_INV_TGTDEVADDR,
/* -l */ ARPT_INV_ARPHLN,
/* 4 */ ARPT_INV_ARPOP,
/* 5 */ ARPT_INV_ARPHRD,
/* 6 */ ARPT_INV_ARPPRO,
};

/***********************************************/
/* ARPTABLES SPECIFIC NEW FUNCTIONS ADDED HERE */
/***********************************************/

static unsigned char mac_type_unicast[ETH_ALEN] =   {0,0,0,0,0,0};
static unsigned char msk_type_unicast[ETH_ALEN] =   {1,0,0,0,0,0};
static unsigned char mac_type_multicast[ETH_ALEN] = {1,0,0,0,0,0};
static unsigned char msk_type_multicast[ETH_ALEN] = {1,0,0,0,0,0};
static unsigned char mac_type_broadcast[ETH_ALEN] = {255,255,255,255,255,255};
static unsigned char msk_type_broadcast[ETH_ALEN] = {255,255,255,255,255,255};

/*
 * put the mac address into 6 (ETH_ALEN) bytes
 */
static int getmac_and_mask(char *from, char *to, char *mask)
{
	char *p;
	int i;
	struct ether_addr *addr;

	if (strcasecmp(from, "Unicast") == 0) {
		memcpy(to, mac_type_unicast, ETH_ALEN);
		memcpy(mask, msk_type_unicast, ETH_ALEN);
		return 0;
	}
	if (strcasecmp(from, "Multicast") == 0) {
		memcpy(to, mac_type_multicast, ETH_ALEN);
		memcpy(mask, msk_type_multicast, ETH_ALEN);
		return 0;
	}
	if (strcasecmp(from, "Broadcast") == 0) {
		memcpy(to, mac_type_broadcast, ETH_ALEN);
		memcpy(mask, msk_type_broadcast, ETH_ALEN);
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

static int getlength_and_mask(char *from, uint8_t *to, uint8_t *mask)
{
	char *p, *buffer;
	int i;

	if ( (p = strrchr(from, '/')) != NULL) {
		*p = '\0';
		i = strtol(p+1, &buffer, 10);
		if (*buffer != '\0' || i < 0 || i > 255)
			return -1;
		*mask = (uint8_t)i;
	} else
		*mask = 255;
	i = strtol(from, &buffer, 10);
	if (*buffer != '\0' || i < 0 || i > 255)
		return -1;
	*to = (uint8_t)i;
	return 0;
}

static int get16_and_mask(char *from, uint16_t *to, uint16_t *mask, int base)
{
	char *p, *buffer;
	int i;

	if ( (p = strrchr(from, '/')) != NULL) {
		*p = '\0';
		i = strtol(p+1, &buffer, base);
		if (*buffer != '\0' || i < 0 || i > 65535)
			return -1;
		*mask = htons((uint16_t)i);
	} else
		*mask = 65535;
	i = strtol(from, &buffer, base);
	if (*buffer != '\0' || i < 0 || i > 65535)
		return -1;
	*to = htons((uint16_t)i);
	return 0;
}

/*********************************************/
/* ARPTABLES SPECIFIC NEW FUNCTIONS END HERE */
/*********************************************/

static void
exit_tryhelp(int status)
{
	fprintf(stderr, "Try `%s -h' or '%s --help' for more information.\n",
		arptables_globals.program_name,
		arptables_globals.program_version);
	exit(status);
}

static void
printhelp(void)
{
	struct xtables_target *t = NULL;
	int i;

	printf("%s v%s\n\n"
"Usage: %s -[AD] chain rule-specification [options]\n"
"       %s -[RI] chain rulenum rule-specification [options]\n"
"       %s -D chain rulenum [options]\n"
"       %s -[LFZ] [chain] [options]\n"
"       %s -[NX] chain\n"
"       %s -E old-chain-name new-chain-name\n"
"       %s -P chain target [options]\n"
"       %s -h (print this help information)\n\n",
	       arptables_globals.program_name,
	       arptables_globals.program_version,
	       arptables_globals.program_name,
	       arptables_globals.program_name,
	       arptables_globals.program_name,
	       arptables_globals.program_name,
	       arptables_globals.program_name,
	       arptables_globals.program_name,
	       arptables_globals.program_name,
	       arptables_globals.program_name);
	printf(
"Commands:\n"
"Either long or short options are allowed.\n"
"  --append  -A chain		Append to chain\n"
"  --delete  -D chain		Delete matching rule from chain\n"
"  --delete  -D chain rulenum\n"
"				Delete rule rulenum (1 = first) from chain\n"
"  --insert  -I chain [rulenum]\n"
"				Insert in chain as rulenum (default 1=first)\n"
"  --replace -R chain rulenum\n"
"				Replace rule rulenum (1 = first) in chain\n"
"  --list    -L [chain]		List the rules in a chain or all chains\n"
"  --flush   -F [chain]		Delete all rules in  chain or all chains\n"
"  --zero    -Z [chain]		Zero counters in chain or all chains\n"
"  --new     -N chain		Create a new user-defined chain\n"
"  --delete-chain\n"
"            -X [chain]		Delete a user-defined chain\n"
"  --policy  -P chain target\n"
"				Change policy on chain to target\n"
"  --rename-chain\n"
"            -E old-chain new-chain\n"
"				Change chain name, (moving any references)\n"

"Options:\n"
"  --source-ip	-s [!] address[/mask]\n"
"				source specification\n"
"  --destination-ip -d [!] address[/mask]\n"
"				destination specification\n"
"  --source-mac [!] address[/mask]\n"
"  --destination-mac [!] address[/mask]\n"
"  --h-length   -l   length[/mask] hardware length (nr of bytes)\n"
"  --opcode code[/mask] operation code (2 bytes)\n"
"  --h-type   type[/mask]  hardware type (2 bytes, hexadecimal)\n"
"  --proto-type   type[/mask]  protocol type (2 bytes)\n"
"  --in-interface -i [!] input name[+]\n"
"				network interface name ([+] for wildcard)\n"
"  --out-interface -o [!] output name[+]\n"
"				network interface name ([+] for wildcard)\n"
"  --jump	-j target\n"
"				target for rule (may load target extension)\n"
"  --match	-m match\n"
"				extended match (may load extension)\n"
"  --numeric	-n		numeric output of addresses and ports\n"
"  --table	-t table	table to manipulate (default: `filter')\n"
"  --verbose	-v		verbose mode\n"
"  --line-numbers		print line numbers when listing\n"
"  --exact	-x		expand numbers (display exact values)\n"
"  --modprobe=<command>		try to insert modules using this command\n"
"  --set-counters -c PKTS BYTES	set the counter during insert/append\n"
"[!] --version	-V		print package version.\n");
	printf(" opcode strings: \n");
        for (i = 0; i < NUMOPCODES; i++)
                printf(" %d = %s\n", i + 1, arp_opcodes[i]);
        printf(
" hardware type string: 1 = Ethernet\n"
" protocol type string: 0x800 = IPv4\n");

	/* Print out any special helps. A user might like to be able
		to add a --help to the commandline, and see expected
		results. So we call help for all matches & targets */
	for (t = xtables_targets; t; t = t->next) {
		if (strcmp(t->name, "CLASSIFY") && strcmp(t->name, "mangle"))
			continue;
		printf("\n");
		t->help();
	}
}

static char
opt2char(int option)
{
	const char *ptr;
	for (ptr = optflags; option > 1; option >>= 1, ptr++);

	return *ptr;
}

static int
check_inverse(const char option[], int *invert, int *optidx, int argc)
{
	if (option && strcmp(option, "!") == 0) {
		if (*invert)
			xtables_error(PARAMETER_PROBLEM,
				      "Multiple `!' flags not allowed");
		*invert = true;
		if (optidx) {
			*optidx = *optidx+1;
			if (argc && *optidx > argc)
				xtables_error(PARAMETER_PROBLEM,
					      "no argument following `!'");
		}

		return true;
	}
	return false;
}

static void
set_option(unsigned int *options, unsigned int option, u_int16_t *invflg,
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
append_entry(struct nft_handle *h,
	     const char *chain,
	     const char *table,
	     struct iptables_command_state *cs,
	     int rulenum,
	     unsigned int nsaddrs,
	     const struct in_addr saddrs[],
	     const struct in_addr smasks[],
	     unsigned int ndaddrs,
	     const struct in_addr daddrs[],
	     const struct in_addr dmasks[],
	     bool verbose, bool append)
{
	unsigned int i, j;
	int ret = 1;

	for (i = 0; i < nsaddrs; i++) {
		cs->arp.arp.src.s_addr = saddrs[i].s_addr;
		cs->arp.arp.smsk.s_addr = smasks[i].s_addr;
		for (j = 0; j < ndaddrs; j++) {
			cs->arp.arp.tgt.s_addr = daddrs[j].s_addr;
			cs->arp.arp.tmsk.s_addr = dmasks[j].s_addr;
			if (append) {
				ret = nft_cmd_rule_append(h, chain, table, cs, NULL,
						      verbose);
			} else {
				ret = nft_cmd_rule_insert(h, chain, table, cs,
						      rulenum, verbose);
			}
		}
	}

	return ret;
}

static int
replace_entry(const char *chain,
	      const char *table,
	      struct iptables_command_state *cs,
	      unsigned int rulenum,
	      const struct in_addr *saddr,
	      const struct in_addr *smask,
	      const struct in_addr *daddr,
	      const struct in_addr *dmask,
	      bool verbose, struct nft_handle *h)
{
	cs->arp.arp.src.s_addr = saddr->s_addr;
	cs->arp.arp.tgt.s_addr = daddr->s_addr;
	cs->arp.arp.smsk.s_addr = smask->s_addr;
	cs->arp.arp.tmsk.s_addr = dmask->s_addr;

	return nft_cmd_rule_replace(h, chain, table, cs, rulenum, verbose);
}

static int
delete_entry(const char *chain,
	     const char *table,
	     struct iptables_command_state *cs,
	     unsigned int nsaddrs,
	     const struct in_addr saddrs[],
	     const struct in_addr smasks[],
	     unsigned int ndaddrs,
	     const struct in_addr daddrs[],
	     const struct in_addr dmasks[],
	     bool verbose, struct nft_handle *h)
{
	unsigned int i, j;
	int ret = 1;

	for (i = 0; i < nsaddrs; i++) {
		cs->arp.arp.src.s_addr = saddrs[i].s_addr;
		cs->arp.arp.smsk.s_addr = smasks[i].s_addr;
		for (j = 0; j < ndaddrs; j++) {
			cs->arp.arp.tgt.s_addr = daddrs[j].s_addr;
			cs->arp.arp.tmsk.s_addr = dmasks[j].s_addr;
			ret = nft_cmd_rule_delete(h, chain, table, cs, verbose);
		}
	}

	return ret;
}

int nft_init_arp(struct nft_handle *h, const char *pname)
{
	arptables_globals.program_name = pname;
	if (xtables_init_all(&arptables_globals, NFPROTO_ARP) < 0) {
		fprintf(stderr, "%s/%s Failed to initialize arptables-compat\n",
			arptables_globals.program_name,
			arptables_globals.program_version);
		exit(1);
	}

#if defined(ALL_INCLUSIVE) || defined(NO_SHARED_LIBS)
	init_extensionsa();
#endif

	if (nft_init(h, NFPROTO_ARP, xtables_arp) < 0)
		xtables_error(OTHER_PROBLEM,
			      "Could not initialize nftables layer.");

	return 0;
}

int do_commandarp(struct nft_handle *h, int argc, char *argv[], char **table,
		  bool restore)
{
	struct iptables_command_state cs = {
		.jumpto = "",
		.arp.arp = {
			.arhln = 6,
			.arhln_mask = 255,
			.arhrd = htons(ARPHRD_ETHER),
			.arhrd_mask = 65535,
		},
	};
	int invert = 0;
	unsigned int nsaddrs = 0, ndaddrs = 0;
	struct in_addr *saddrs = NULL, *smasks = NULL;
	struct in_addr *daddrs = NULL, *dmasks = NULL;

	int c, verbose = 0;
	const char *chain = NULL;
	const char *shostnetworkmask = NULL, *dhostnetworkmask = NULL;
	const char *policy = NULL, *newname = NULL;
	unsigned int rulenum = 0, options = 0, command = 0;
	const char *pcnt = NULL, *bcnt = NULL;
	int ret = 1;
	struct xtables_target *t;

	/* re-set optind to 0 in case do_command gets called
	 * a second time */
	optind = 0;

	for (t = xtables_targets; t; t = t->next) {
		t->tflags = 0;
		t->used = 0;
	}

	/* Suppress error messages: we may add new options if we
	    demand-load a protocol. */
	opterr = 0;

	opts = xt_params->orig_opts;
	while ((c = getopt_long(argc, argv,
	   "-A:D:R:I:L::M:F::Z::N:X::E:P:Vh::o:p:s:d:j:l:i:vnt:m:c:",
					   opts, NULL)) != -1) {
		switch (c) {
			/*
			 * Command selection
			 */
		case 'A':
			add_command(&command, CMD_APPEND, CMD_NONE,
				    invert);
			chain = optarg;
			break;

		case 'D':
			add_command(&command, CMD_DELETE, CMD_NONE,
				    invert);
			chain = optarg;
			if (xs_has_arg(argc, argv)) {
				rulenum = parse_rulenumber(argv[optind++]);
				command = CMD_DELETE_NUM;
			}
			break;

		case 'R':
			add_command(&command, CMD_REPLACE, CMD_NONE,
				    invert);
			chain = optarg;
			if (xs_has_arg(argc, argv))
				rulenum = parse_rulenumber(argv[optind++]);
			else
				xtables_error(PARAMETER_PROBLEM,
					      "-%c requires a rule number",
					      cmd2char(CMD_REPLACE));
			break;

		case 'I':
			add_command(&command, CMD_INSERT, CMD_NONE,
				    invert);
			chain = optarg;
			if (xs_has_arg(argc, argv))
				rulenum = parse_rulenumber(argv[optind++]);
			else rulenum = 1;
			break;

		case 'L':
			add_command(&command, CMD_LIST, CMD_ZERO,
				    invert);
			if (optarg) chain = optarg;
			else if (xs_has_arg(argc, argv))
				chain = argv[optind++];
			break;

		case 'F':
			add_command(&command, CMD_FLUSH, CMD_NONE,
				    invert);
			if (optarg) chain = optarg;
			else if (xs_has_arg(argc, argv))
				chain = argv[optind++];
			break;

		case 'Z':
			add_command(&command, CMD_ZERO, CMD_LIST,
				    invert);
			if (optarg) chain = optarg;
			else if (xs_has_arg(argc, argv))
				chain = argv[optind++];
			break;

		case 'N':
			if (optarg && *optarg == '-')
				xtables_error(PARAMETER_PROBLEM,
					      "chain name not allowed to start "
					      "with `-'\n");
			if (xtables_find_target(optarg, XTF_TRY_LOAD))
				xtables_error(PARAMETER_PROBLEM,
						"chain name may not clash "
						"with target name\n");
			add_command(&command, CMD_NEW_CHAIN, CMD_NONE,
				    invert);
			chain = optarg;
			break;

		case 'X':
			add_command(&command, CMD_DELETE_CHAIN, CMD_NONE,
				    invert);
			if (optarg) chain = optarg;
			else if (xs_has_arg(argc, argv))
				chain = argv[optind++];
			break;

		case 'E':
			add_command(&command, CMD_RENAME_CHAIN, CMD_NONE,
				    invert);
			chain = optarg;
			if (xs_has_arg(argc, argv))
				newname = argv[optind++];
			else
				xtables_error(PARAMETER_PROBLEM,
					      "-%c requires old-chain-name and "
					      "new-chain-name",
					      cmd2char(CMD_RENAME_CHAIN));
			break;

		case 'P':
			add_command(&command, CMD_SET_POLICY, CMD_NONE,
				    invert);
			chain = optarg;
			if (xs_has_arg(argc, argv))
				policy = argv[optind++];
			else
				xtables_error(PARAMETER_PROBLEM,
					      "-%c requires a chain and a policy",
					      cmd2char(CMD_SET_POLICY));
			break;

		case 'h':
			if (!optarg)
				optarg = argv[optind];

			printhelp();
			command = CMD_NONE;
			break;
		case 's':
			check_inverse(optarg, &invert, &optind, argc);
			set_option(&options, OPT_SOURCE, &cs.arp.arp.invflags,
				   invert);
			shostnetworkmask = argv[optind-1];
			break;

		case 'd':
			check_inverse(optarg, &invert, &optind, argc);
			set_option(&options, OPT_DESTINATION, &cs.arp.arp.invflags,
				   invert);
			dhostnetworkmask = argv[optind-1];
			break;

		case 2:/* src-mac */
			check_inverse(optarg, &invert, &optind, argc);
			set_option(&options, OPT_S_MAC, &cs.arp.arp.invflags,
				   invert);
			if (getmac_and_mask(argv[optind - 1],
			    cs.arp.arp.src_devaddr.addr, cs.arp.arp.src_devaddr.mask))
				xtables_error(PARAMETER_PROBLEM, "Problem with specified "
						"source mac");
			break;

		case 3:/* dst-mac */
			check_inverse(optarg, &invert, &optind, argc);
			set_option(&options, OPT_D_MAC, &cs.arp.arp.invflags,
				   invert);

			if (getmac_and_mask(argv[optind - 1],
			    cs.arp.arp.tgt_devaddr.addr, cs.arp.arp.tgt_devaddr.mask))
				xtables_error(PARAMETER_PROBLEM, "Problem with specified "
						"destination mac");
			break;

		case 'l':/* hardware length */
			check_inverse(optarg, &invert, &optind, argc);
			set_option(&options, OPT_H_LENGTH, &cs.arp.arp.invflags,
				   invert);
			getlength_and_mask(argv[optind - 1], &cs.arp.arp.arhln,
					   &cs.arp.arp.arhln_mask);

			if (cs.arp.arp.arhln != 6) {
				xtables_error(PARAMETER_PROBLEM,
					      "Only harware address length of"
					      " 6 is supported currently.");
			}

			break;

		case 8: /* was never supported, not even in arptables-legacy */
			xtables_error(PARAMETER_PROBLEM, "not supported");
		case 4:/* opcode */
			check_inverse(optarg, &invert, &optind, argc);
			set_option(&options, OPT_OPCODE, &cs.arp.arp.invflags,
				   invert);
			if (get16_and_mask(argv[optind - 1], &cs.arp.arp.arpop,
					   &cs.arp.arp.arpop_mask, 10)) {
				int i;

				for (i = 0; i < NUMOPCODES; i++)
					if (!strcasecmp(arp_opcodes[i], optarg))
						break;
				if (i == NUMOPCODES)
					xtables_error(PARAMETER_PROBLEM, "Problem with specified opcode");
				cs.arp.arp.arpop = htons(i+1);
			}
			break;

		case 5:/* h-type */
			check_inverse(optarg, &invert, &optind, argc);
			set_option(&options, OPT_H_TYPE, &cs.arp.arp.invflags,
				   invert);
			if (get16_and_mask(argv[optind - 1], &cs.arp.arp.arhrd,
					   &cs.arp.arp.arhrd_mask, 16)) {
				if (strcasecmp(argv[optind-1], "Ethernet"))
					xtables_error(PARAMETER_PROBLEM, "Problem with specified hardware type");
				cs.arp.arp.arhrd = htons(1);
			}
			break;

		case 6:/* proto-type */
			check_inverse(optarg, &invert, &optind, argc);
			set_option(&options, OPT_P_TYPE, &cs.arp.arp.invflags,
				   invert);
			if (get16_and_mask(argv[optind - 1], &cs.arp.arp.arpro,
					   &cs.arp.arp.arpro_mask, 0)) {
				if (strcasecmp(argv[optind-1], "ipv4"))
					xtables_error(PARAMETER_PROBLEM, "Problem with specified protocol type");
				cs.arp.arp.arpro = htons(0x800);
			}
			break;

		case 'j':
			set_option(&options, OPT_JUMP, &cs.arp.arp.invflags,
				   invert);
			command_jump(&cs, optarg);
			break;

		case 'i':
			check_inverse(optarg, &invert, &optind, argc);
			set_option(&options, OPT_VIANAMEIN, &cs.arp.arp.invflags,
				   invert);
			xtables_parse_interface(argv[optind-1],
						cs.arp.arp.iniface,
						cs.arp.arp.iniface_mask);
			break;

		case 'o':
			check_inverse(optarg, &invert, &optind, argc);
			set_option(&options, OPT_VIANAMEOUT, &cs.arp.arp.invflags,
				   invert);
			xtables_parse_interface(argv[optind-1],
						cs.arp.arp.outiface,
						cs.arp.arp.outiface_mask);
			break;

		case 'v':
			if (!verbose)
				set_option(&options, OPT_VERBOSE,
					   &cs.arp.arp.invflags, invert);
			verbose++;
			break;

		case 'm': /* ignored by arptables-legacy */
			break;
		case 'n':
			set_option(&options, OPT_NUMERIC, &cs.arp.arp.invflags,
				   invert);
			break;

		case 't':
			if (invert)
				xtables_error(PARAMETER_PROBLEM,
					      "unexpected ! flag before --table");
			/* ignore this option.
			 * arptables-legacy parses it, but libarptc doesn't use it.
			 * arptables only has a 'filter' table anyway.
			 */
			break;

		case 'V':
			if (invert)
				printf("Not %s ;-)\n", arptables_globals.program_version);
			else
				printf("%s v%s (nf_tables)\n",
				       arptables_globals.program_name,
				       arptables_globals.program_version);
			exit(0);

		case '0':
			set_option(&options, OPT_LINENUMBERS, &cs.arp.arp.invflags,
				   invert);
			break;

		case 'M':
			//modprobe = optarg;
			break;

		case 'c':

			set_option(&options, OPT_COUNTERS, &cs.arp.arp.invflags,
				   invert);
			pcnt = optarg;
			if (xs_has_arg(argc, argv))
				bcnt = argv[optind++];
			else
				xtables_error(PARAMETER_PROBLEM,
					      "-%c requires packet and byte counter",
					      opt2char(OPT_COUNTERS));

			if (sscanf(pcnt, "%llu", &cs.arp.counters.pcnt) != 1)
			xtables_error(PARAMETER_PROBLEM,
				"-%c packet counter not numeric",
				opt2char(OPT_COUNTERS));

			if (sscanf(bcnt, "%llu", &cs.arp.counters.bcnt) != 1)
				xtables_error(PARAMETER_PROBLEM,
					      "-%c byte counter not numeric",
					      opt2char(OPT_COUNTERS));

			break;


		case 1: /* non option */
			if (optarg[0] == '!' && optarg[1] == '\0') {
				if (invert)
					xtables_error(PARAMETER_PROBLEM,
						      "multiple consecutive ! not"
						      " allowed");
				invert = true;
				optarg[0] = '\0';
				continue;
			}
			printf("Bad argument `%s'\n", optarg);
			exit_tryhelp(2);

		default:
			if (cs.target) {
				xtables_option_tpcall(c, argv,
						      invert, cs.target, &cs.arp);
			}
			break;
		}
		invert = false;
	}

	if (cs.target)
		xtables_option_tfcall(cs.target);

	if (optind < argc)
		xtables_error(PARAMETER_PROBLEM,
			      "unknown arguments found on commandline");
	if (invert)
		xtables_error(PARAMETER_PROBLEM,
			      "nothing appropriate following !");

	if (command & (CMD_REPLACE | CMD_INSERT | CMD_DELETE | CMD_APPEND)) {
		if (!(options & OPT_DESTINATION))
			dhostnetworkmask = "0.0.0.0/0";
		if (!(options & OPT_SOURCE))
			shostnetworkmask = "0.0.0.0/0";
	}

	if (shostnetworkmask)
		xtables_ipparse_multiple(shostnetworkmask, &saddrs,
					 &smasks, &nsaddrs);

	if (dhostnetworkmask)
		xtables_ipparse_multiple(dhostnetworkmask, &daddrs,
					 &dmasks, &ndaddrs);

	if ((nsaddrs > 1 || ndaddrs > 1) &&
	    (cs.arp.arp.invflags & (ARPT_INV_SRCIP | ARPT_INV_TGTIP)))
		xtables_error(PARAMETER_PROBLEM, "! not allowed with multiple"
				" source or destination IP addresses");

	if (command == CMD_REPLACE && (nsaddrs != 1 || ndaddrs != 1))
		xtables_error(PARAMETER_PROBLEM, "Replacement rule does not "
						 "specify a unique address");

	if (chain && strlen(chain) > ARPT_FUNCTION_MAXNAMELEN)
		xtables_error(PARAMETER_PROBLEM,
				"chain name `%s' too long (must be under %i chars)",
				chain, ARPT_FUNCTION_MAXNAMELEN);

	if (command == CMD_APPEND
	    || command == CMD_DELETE
	    || command == CMD_INSERT
	    || command == CMD_REPLACE) {
		if (strcmp(chain, "PREROUTING") == 0
		    || strcmp(chain, "INPUT") == 0) {
			/* -o not valid with incoming packets. */
			if (options & OPT_VIANAMEOUT)
				xtables_error(PARAMETER_PROBLEM,
					      "Can't use -%c with %s\n",
					      opt2char(OPT_VIANAMEOUT),
					      chain);
		}

		if (strcmp(chain, "POSTROUTING") == 0
		    || strcmp(chain, "OUTPUT") == 0) {
			/* -i not valid with outgoing packets */
			if (options & OPT_VIANAMEIN)
				xtables_error(PARAMETER_PROBLEM,
						"Can't use -%c with %s\n",
						opt2char(OPT_VIANAMEIN),
						chain);
		}
	}

	switch (command) {
	case CMD_APPEND:
		ret = append_entry(h, chain, *table, &cs, 0,
				   nsaddrs, saddrs, smasks,
				   ndaddrs, daddrs, dmasks,
				   options&OPT_VERBOSE, true);
		break;
	case CMD_DELETE:
		ret = delete_entry(chain, *table, &cs,
				   nsaddrs, saddrs, smasks,
				   ndaddrs, daddrs, dmasks,
				   options&OPT_VERBOSE, h);
		break;
	case CMD_DELETE_NUM:
		ret = nft_cmd_rule_delete_num(h, chain, *table, rulenum - 1, verbose);
		break;
	case CMD_REPLACE:
		ret = replace_entry(chain, *table, &cs, rulenum - 1,
				    saddrs, smasks, daddrs, dmasks,
				    options&OPT_VERBOSE, h);
		break;
	case CMD_INSERT:
		ret = append_entry(h, chain, *table, &cs, rulenum - 1,
				   nsaddrs, saddrs, smasks,
				   ndaddrs, daddrs, dmasks,
				   options&OPT_VERBOSE, false);
		break;
	case CMD_LIST:
		ret = list_entries(h, chain, *table,
				   rulenum,
				   options&OPT_VERBOSE,
				   options&OPT_NUMERIC,
				   /*options&OPT_EXPANDED*/0,
				   options&OPT_LINENUMBERS);
		break;
	case CMD_FLUSH:
		ret = nft_cmd_rule_flush(h, chain, *table, options & OPT_VERBOSE);
		break;
	case CMD_ZERO:
		ret = nft_cmd_chain_zero_counters(h, chain, *table,
					      options & OPT_VERBOSE);
		break;
	case CMD_LIST|CMD_ZERO:
		ret = list_entries(h, chain, *table, rulenum,
				   options&OPT_VERBOSE,
				   options&OPT_NUMERIC,
				   /*options&OPT_EXPANDED*/0,
				   options&OPT_LINENUMBERS);
		if (ret)
			ret = nft_cmd_chain_zero_counters(h, chain, *table,
						      options & OPT_VERBOSE);
		break;
	case CMD_NEW_CHAIN:
		ret = nft_cmd_chain_user_add(h, chain, *table);
		break;
	case CMD_DELETE_CHAIN:
		ret = nft_cmd_chain_user_del(h, chain, *table,
					 options & OPT_VERBOSE);
		break;
	case CMD_RENAME_CHAIN:
		ret = nft_cmd_chain_user_rename(h, chain, *table, newname);
		break;
	case CMD_SET_POLICY:
		ret = nft_cmd_chain_set(h, *table, chain, policy, NULL);
		if (ret < 0)
			xtables_error(PARAMETER_PROBLEM, "Wrong policy `%s'\n",
				      policy);
		break;
	case CMD_NONE:
		break;
	default:
		/* We should never reach this... */
		exit_tryhelp(2);
	}

	free(saddrs);
	free(smasks);
	free(daddrs);
	free(dmasks);

	nft_clear_iptables_command_state(&cs);
	xtables_free_opts(1);

/*	if (verbose > 1)
		dump_entries(*handle);*/

	return ret;
}
