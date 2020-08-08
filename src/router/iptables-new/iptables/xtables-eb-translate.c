#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>
#include <iptables.h>
#include <xtables.h>

#include <netinet/ether.h>

#include <linux/netfilter_bridge.h>
#include <linux/netfilter/nf_tables.h>
#include <libiptc/libxtc.h>

#include "xshared.h"
#include "xtables-multi.h"
#include "nft-bridge.h"
#include "nft.h"
#include "nft-shared.h"
/*
 * From include/ebtables_u.h
 */
#define EXEC_STYLE_PRG    0
#define EXEC_STYLE_DAEMON 1

#define ebt_check_option2(flags, mask) EBT_CHECK_OPTION(flags, mask)

extern int ebt_invert;

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

static int get_current_chain(const char *chain)
{
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

	return -1;
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
#define OPT_COUNT	0x1000 /* This value is also defined in libebtc.c */

/* Default command line options. Do not mess around with the already
 * assigned numbers unless you know what you are doing */
extern struct option ebt_original_options[];
extern struct xtables_globals ebtables_globals;
#define opts ebtables_globals.opts
#define prog_name ebtables_globals.program_name
#define prog_vers ebtables_globals.program_version

static void print_help(void)
{
	fprintf(stderr, "%s: Translate ebtables command to nft syntax\n"
			"no side effects occur, the translated command is written "
			"to standard output.\n"
			"A '#' followed by input means no translation "
			"is available.\n", prog_name);
	exit(0);
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

static void print_ebt_cmd(int argc, char *argv[])
{
	int i;

	printf("# ");
	for (i = 1; i < argc; i++)
		printf("%s ", argv[i]);

	printf("\n");
}

static int nft_rule_eb_xlate_add(struct nft_handle *h, const struct nft_xt_cmd_parse *p,
				 const struct iptables_command_state *cs, bool append)
{
	struct xt_xlate *xl = xt_xlate_alloc(10240);
	int ret;

	if (append) {
		xt_xlate_add(xl, "add rule bridge %s %s ", p->table, p->chain);
	} else {
		xt_xlate_add(xl, "insert rule bridge %s %s ", p->table, p->chain);
	}

	ret = h->ops->xlate(cs, xl);
	if (ret)
		printf("%s\n", xt_xlate_get(xl));

	xt_xlate_free(xl);
	return ret;
}

/* We use exec_style instead of #ifdef's because ebtables.so is a shared object. */
static int do_commandeb_xlate(struct nft_handle *h, int argc, char *argv[], char **table)
{
	char *buffer;
	int c, i;
	int rule_nr = 0;
	int rule_nr_end = 0;
	int ret = 0;
	unsigned int flags = 0;
	struct iptables_command_state cs = {
		.argv		= argv,
		.eb.bitmask	= EBT_NOPROTO,
	};
	char command = 'h';
	const char *chain = NULL;
	int exec_style = EXEC_STYLE_PRG;
	int selected_chain = -1;
	struct xtables_rule_match *xtrm_i;
	struct ebt_match *match;
	struct nft_xt_cmd_parse p = {
		.table          = *table,
        };

	/* prevent getopt to spoil our error reporting */
	opterr = false;

	printf("nft ");
	/* Getopt saves the day */
	while ((c = getopt_long(argc, argv,
	   "-A:D:I:N:E:X::L::Z::F::P:Vhi:o:j:c:p:s:d:t:M:", opts, NULL)) != -1) {
		cs.c = c;
		cs.invert = ebt_invert;
		switch (c) {
		case 'A': /* Add a rule */
		case 'D': /* Delete a rule */
		case 'P': /* Define policy */
		case 'I': /* Insert a rule */
		case 'N': /* Make a user defined chain */
		case 'E': /* Rename chain */
		case 'X': /* Delete chain */
			/* We allow -N chainname -P policy */
			/* XXX: Not in ebtables-compat */
			if (command == 'N' && c == 'P') {
				command = c;
				optind--; /* No table specified */
				break;
			}
			if (OPT_COMMANDS)
				xtables_error(PARAMETER_PROBLEM,
					      "Multiple commands are not allowed");
			command = c;
			chain = optarg;
			selected_chain = get_current_chain(chain);
			p.chain = chain;
			flags |= OPT_COMMAND;

			if (c == 'N') {
				printf("add chain bridge %s %s\n", p.table, p.chain);
				ret = 1;
				break;
			} else if (c == 'X') {
				printf("delete chain bridge %s %s\n", p.table, p.chain);
				ret = 1;
				break;
			}

			if (c == 'E') {
				break;
			} else if (c == 'D' && optind < argc && (argv[optind][0] != '-' || (argv[optind][1] >= '0' && argv[optind][1] <= '9'))) {
				if (optind != argc - 1)
					xtables_error(PARAMETER_PROBLEM,
							 "No extra options allowed with -D start_nr[:end_nr]");
				if (parse_rule_range(argv[optind], &rule_nr, &rule_nr_end))
					xtables_error(PARAMETER_PROBLEM,
							 "Problem with the specified rule number(s) '%s'", argv[optind]);
				optind++;
			} else if (c == 'I') {
				if (optind >= argc || (argv[optind][0] == '-' && (argv[optind][1] < '0' || argv[optind][1] > '9')))
					rule_nr = 1;
				else {
					rule_nr = parse_rule_number(argv[optind]);
					optind++;
				}
				p.rulenum = rule_nr;
			} else if (c == 'P') {
				break;
			}
			break;
		case 'L': /* List */
			printf("list table bridge %s\n", p.table);
			ret = 1;
			break;
		case 'F': /* Flush */
			if (p.chain) {
				printf("flush chain bridge %s %s\n", p.table, p.chain);
			} else {
				printf("flush table bridge %s\n", p.table);
			}
			ret = 1;
			break;
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
			break;
		case 'V': /* Version */
			if (OPT_COMMANDS)
				xtables_error(PARAMETER_PROBLEM,
					      "Multiple commands are not allowed");
			if (exec_style == EXEC_STYLE_DAEMON)
				xtables_error(PARAMETER_PROBLEM,
					      "%s %s\n", prog_name, prog_vers);
			printf("%s %s\n", prog_name, prog_vers);
			exit(0);
		case 'h':
			if (OPT_COMMANDS)
				xtables_error(PARAMETER_PROBLEM,
					      "Multiple commands are not allowed");
			print_help();
			break;
		case 't': /* Table */
			if (OPT_COMMANDS)
				xtables_error(PARAMETER_PROBLEM,
					      "Please put the -t option first");
			ebt_check_option2(&flags, OPT_TABLE);
			if (strlen(optarg) > EBT_TABLE_MAXNAMELEN - 1)
				xtables_error(PARAMETER_PROBLEM,
					      "Table name length cannot exceed %d characters",
					      EBT_TABLE_MAXNAMELEN - 1);
			*table = optarg;
			p.table = optarg;
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
			if (command != 'A' && command != 'D' && command != 'I')
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
				command_jump(&cs, optarg);
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
					      argv[optind - 1]);

			if (command != 'A' && command != 'I' &&
			    command != 'D')
				xtables_error(PARAMETER_PROBLEM,
					      "Extensions only for -A, -I, -D");
		}
		ebt_invert = 0;
	}

	/* Do the final checks */
	if (command == 'A' || command == 'I' || command == 'D') {
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

	cs.eb.ethproto = htons(cs.eb.ethproto);

	if (command == 'P') {
		return 0;
	} else if (command == 'A') {
		ret = nft_rule_eb_xlate_add(h, &p, &cs, true);
		if (!ret)
			print_ebt_cmd(argc, argv);
	} else if (command == 'I') {
		ret = nft_rule_eb_xlate_add(h, &p, &cs, false);
		if (!ret)
			print_ebt_cmd(argc, argv);
	}

	ebt_cs_clean(&cs);
	return ret;
}

static int dummy_compat_rev(const char *name, uint8_t rev, int opt)
{
	return 1;
}

int xtables_eb_xlate_main(int argc, char *argv[])
{
	int ret;
	char *table = "filter";
	struct nft_handle h;

	nft_init_eb(&h, argv[0]);
	ebtables_globals.compat_rev = dummy_compat_rev;

	ret = do_commandeb_xlate(&h, argc, argv, &table);
	if (!ret)
		fprintf(stderr, "Translation not implemented\n");

	exit(!ret);
}

