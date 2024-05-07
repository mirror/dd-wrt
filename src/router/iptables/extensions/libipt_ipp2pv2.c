/*
 *	"ipp2p" match extension for iptables
 *	Eicke Friedrich/Klaus Degner <ipp2p@ipp2p.org>, 2005 - 2006
 *	Jan Engelhardt <jengelh [at] medozas de>, 2008 - 2009
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License; either
 *	version 2 of the License, or any later version, as published by the
 *	Free Software Foundation.
 */
#include <stdbool.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <iptables.h>
#include "xt_ipp2p.h"

#define param_act(t, s, f) xtables_param_act((t), "ipp2p", (s), (f))

static void ipp2p_mt_help(void)
{
	printf(
	"ipp2p v%s match options:\n"
	"  --edk    [tcp,udp]  All known eDonkey/eMule/Overnet packets\n"
	"  --dc     [tcp]      All known Direct Connect packets\n"
	"  --kazaa  [tcp,udp]  All known KaZaA packets\n"
	"  --gnu    [tcp,udp]  All known Gnutella packets\n"
	"  --bit    [tcp,udp]  All known BitTorrent packets\n"
	"  --apple  [tcp]      All known AppleJuice packets\n"
	"  --winmx  [tcp]      All known WinMX\n"
	"  --soul   [tcp]      All known SoulSeek\n"
	"  --ares   [tcp]      All known Ares\n\n"
	"EXPERIMENTAL protocols:\n"
	"  --mute   [tcp]      All known Mute packets\n"
	"  --waste  [tcp]      All known Waste packets\n"
	"  --xdcc   [tcp]      All known XDCC packets (only xdcc login)\n\n"
	, IPP2P_VERSION);
}
#define SHORT_HAND_IPP2P 0x1
static const struct option ipp2p_mt_opts[] = {
	{.name = "ipp2p", .has_arg = false, .val = '1'},
	{.name = "edk",   .has_arg = false, .val = '2'},
	{.name = "dc",    .has_arg = false, .val = '7'},
	{.name = "gnu",   .has_arg = false, .val = '9'},
	{.name = "kazaa", .has_arg = false, .val = 'a'},
	{.name = "bit",   .has_arg = false, .val = 'b'},
	{.name = "apple", .has_arg = false, .val = 'c'},
	{.name = "soul",  .has_arg = false, .val = 'd'},
	{.name = "winmx", .has_arg = false, .val = 'e'},
	{.name = "ares",  .has_arg = false, .val = 'f'},
	{.name = "mute",  .has_arg = false, .val = 'g'},
	{.name = "waste", .has_arg = false, .val = 'h'},
	{.name = "xdcc",  .has_arg = false, .val = 'i'},
	{.name = "debug", .has_arg = false, .val = 'j'},
	{NULL},
};

static int ipp2p_mt_parse(int c, char **argv, int invert, unsigned int *flags,
	const struct ipt_entry *entry,
	unsigned int *nfcache,
	struct ipt_entry_match **match)
{
    struct ipt_p2p_info *info = (struct ipt_p2p_info *)(*match)->data;
    
    switch (c) {
	case '1':		/*cmd: ipp2p*/
	    if ((*flags & 1) == 1)
		    exit_error(PARAMETER_PROBLEM,
				"ipp2p: `--ipp2p' may only be "
				"specified once!");
/*	    if ((*flags & SHORT_HAND_DATA) == SHORT_HAND_DATA)
		    exit_error(PARAMETER_PROBLEM,
				"ipp2p: `--ipp2p-data' may only be "
				"specified alone!");*/
	    if ((*flags) != 0)
		    exit_error(PARAMETER_PROBLEM,
				"ipp2p: `--ipp2p' may only be "
				"specified alone!");
	    if (invert) exit_error(PARAMETER_PROBLEM, "ipp2p: invert [!] is not allowed!");
	    *flags = IPP2P_EDK;
	    *flags += IPP2P_DC;
	    *flags += IPP2P_GNU;
	    *flags += IPP2P_KAZAA;
	    *flags += IPP2P_BIT;
	    *flags += IPP2P_APPLE;
	    *flags += IPP2P_SOUL;
	    *flags += IPP2P_WINMX;
	    *flags += IPP2P_ARES;
	    info->cmd = *flags;
	    *flags = 1;
	    break;
	    
	case '2':		/*cmd: edk*/
	    if ((*flags & IPP2P_EDK) == IPP2P_EDK)
		    exit_error(PARAMETER_PROBLEM,
				"ipp2p: `--edk' may only be "
				"specified once");
	    if ((*flags & SHORT_HAND_IPP2P) == SHORT_HAND_IPP2P)
		    exit_error(PARAMETER_PROBLEM,
				"ipp2p: `--ipp2p' may only be "
				"specified alone!");
/*	    if ((*flags & SHORT_HAND_DATA) == SHORT_HAND_DATA)
		    exit_error(PARAMETER_PROBLEM,
				"ipp2p: `--ipp2p-data' may only be "
				"specified alone!");*/
            if ((*flags & IPP2P_DATA_EDK) == IPP2P_DATA_EDK)
            exit_error(PARAMETER_PROBLEM,
                                "ipp2p: use `--edk' OR `--edk-data' but not both of them!");
	    if (invert) exit_error(PARAMETER_PROBLEM, "ipp2p: invert [!] is not allowed!");
	    *flags += IPP2P_EDK;
	    info->cmd = *flags;	    
	    break;


	case '7':		/*cmd: dc*/
            if ((*flags & IPP2P_DC) == IPP2P_DC)
            exit_error(PARAMETER_PROBLEM,
                                "ipp2p: `--dc' may only be "
                                "specified once!");
	    if ((*flags & SHORT_HAND_IPP2P) == SHORT_HAND_IPP2P)
		    exit_error(PARAMETER_PROBLEM,
				"ipp2p: `--ipp2p' may only be "
				"specified alone!");
/*	    if ((*flags & SHORT_HAND_DATA) == SHORT_HAND_DATA)
		    exit_error(PARAMETER_PROBLEM,
				"ipp2p: `--ipp2p-data' may only be "
				"specified alone!");*/
            if ((*flags & IPP2P_DATA_DC) == IPP2P_DATA_DC)
            exit_error(PARAMETER_PROBLEM,
                                "ipp2p: use `--dc' OR `--dc-data' but not both of them!");
	    if (invert) exit_error(PARAMETER_PROBLEM, "ipp2p: invert [!] is not allowed!");
            *flags += IPP2P_DC;
	    info->cmd = *flags;
	    break;


	case '9':		/*cmd: gnu*/
            if ((*flags & IPP2P_GNU) == IPP2P_GNU)
            exit_error(PARAMETER_PROBLEM,
                                "ipp2p: `--gnu' may only be "
                                "specified once!");
/*	    if ((*flags & SHORT_HAND_DATA) == SHORT_HAND_DATA)
		    exit_error(PARAMETER_PROBLEM,
				"ipp2p: `--ipp2p-data' may only be "
				"specified alone!");*/
	    if ((*flags & SHORT_HAND_IPP2P) == SHORT_HAND_IPP2P)
		    exit_error(PARAMETER_PROBLEM,
				"ipp2p: `--ipp2p' may only be "
				"specified alone!");
            if ((*flags & IPP2P_DATA_GNU) == IPP2P_DATA_GNU)
            exit_error(PARAMETER_PROBLEM,
                                "ipp2p: use `--gnu' OR `--gnu-data' but not both of them!");
	    if (invert) exit_error(PARAMETER_PROBLEM, "ipp2p: invert [!] is not allowed!");
            *flags += IPP2P_GNU;
	    info->cmd = *flags;
	    break;

	case 'a':		/*cmd: kazaa*/
            if ((*flags & IPP2P_KAZAA) == IPP2P_KAZAA)
            exit_error(PARAMETER_PROBLEM,
                                "ipp2p: `--kazaa' may only be "
                                "specified once!");
/*	    if ((*flags & SHORT_HAND_DATA) == SHORT_HAND_DATA)
		    exit_error(PARAMETER_PROBLEM,
				"ipp2p: `--ipp2p-data' may only be "
				"specified alone!");*/
	    if ((*flags & SHORT_HAND_IPP2P) == SHORT_HAND_IPP2P)
		    exit_error(PARAMETER_PROBLEM,
				"ipp2p: `--ipp2p' may only be "
				"specified alone!");
            if ((*flags & IPP2P_DATA_KAZAA) == IPP2P_DATA_KAZAA)
            exit_error(PARAMETER_PROBLEM,
                                "ipp2p: use `--kazaa' OR `--kazaa-data' but not both of them!");
	    if (invert) exit_error(PARAMETER_PROBLEM, "ipp2p: invert [!] is not allowed!");
            *flags += IPP2P_KAZAA;
	    info->cmd = *flags;
	    break;																											

	case 'b':		/*cmd: bit*/
            if ((*flags & IPP2P_BIT) == IPP2P_BIT)
            exit_error(PARAMETER_PROBLEM,
                                "ipp2p: `--bit' may only be "
                                "specified once!");
	    if ((*flags & SHORT_HAND_IPP2P) == SHORT_HAND_IPP2P)
		    exit_error(PARAMETER_PROBLEM,
				"ipp2p: `--ipp2p' may only be "
				"specified alone!");
	    if (invert) exit_error(PARAMETER_PROBLEM, "ipp2p: invert [!] is not allowed!");
            *flags += IPP2P_BIT;
	    info->cmd = *flags;
	    break;																											

	case 'c':		/*cmd: apple*/
            if ((*flags & IPP2P_APPLE) == IPP2P_APPLE)
            exit_error(PARAMETER_PROBLEM,
                                "ipp2p: `--apple' may only be "
                                "specified once!");
	    if ((*flags & SHORT_HAND_IPP2P) == SHORT_HAND_IPP2P)
		    exit_error(PARAMETER_PROBLEM,
				"ipp2p: `--ipp2p' may only be "
				"specified alone!");
	    if (invert) exit_error(PARAMETER_PROBLEM, "ipp2p: invert [!] is not allowed!");
            *flags += IPP2P_APPLE;
	    info->cmd = *flags;
	    break;																											


	case 'd':		/*cmd: soul*/
            if ((*flags & IPP2P_SOUL) == IPP2P_SOUL)
            exit_error(PARAMETER_PROBLEM,
                                "ipp2p: `--soul' may only be "
                                "specified once!");
	    if ((*flags & SHORT_HAND_IPP2P) == SHORT_HAND_IPP2P)
		    exit_error(PARAMETER_PROBLEM,
				"ipp2p: `--ipp2p' may only be "
				"specified alone!");
	    if (invert) exit_error(PARAMETER_PROBLEM, "ipp2p: invert [!] is not allowed!");
            *flags += IPP2P_SOUL;
	    info->cmd = *flags;
	    break;																											


	case 'e':		/*cmd: winmx*/
            if ((*flags & IPP2P_WINMX) == IPP2P_WINMX)
            exit_error(PARAMETER_PROBLEM,
                                "ipp2p: `--winmx' may only be "
                                "specified once!");
	    if ((*flags & SHORT_HAND_IPP2P) == SHORT_HAND_IPP2P)
		    exit_error(PARAMETER_PROBLEM,
				"ipp2p: `--ipp2p' may only be "
				"specified alone!");
	    if (invert) exit_error(PARAMETER_PROBLEM, "ipp2p: invert [!] is not allowed!");
            *flags += IPP2P_WINMX;
	    info->cmd = *flags;
	    break;																											

	case 'f':		/*cmd: ares*/
            if ((*flags & IPP2P_ARES) == IPP2P_ARES)
            exit_error(PARAMETER_PROBLEM,
                                "ipp2p: `--ares' may only be "
                                "specified once!");
	    if ((*flags & SHORT_HAND_IPP2P) == SHORT_HAND_IPP2P)
		    exit_error(PARAMETER_PROBLEM,
				"ipp2p: `--ipp2p' may only be "
				"specified alone!");
	    if (invert) exit_error(PARAMETER_PROBLEM, "ipp2p: invert [!] is not allowed!");
            *flags += IPP2P_ARES;
	    info->cmd = *flags;
	    break;																											
	
	case 'g':		/*cmd: mute*/
            if ((*flags & IPP2P_MUTE) == IPP2P_MUTE)
            exit_error(PARAMETER_PROBLEM,
                                "ipp2p: `--mute' may only be "
                                "specified once!");
	    if (invert) exit_error(PARAMETER_PROBLEM, "ipp2p: invert [!] is not allowed!");
            *flags += IPP2P_MUTE;
	    info->cmd = *flags;
	    break;																											
	case 'h':		/*cmd: waste*/
            if ((*flags & IPP2P_WASTE) == IPP2P_WASTE)
            exit_error(PARAMETER_PROBLEM,
                                "ipp2p: `--waste' may only be "
                                "specified once!");
	    if (invert) exit_error(PARAMETER_PROBLEM, "ipp2p: invert [!] is not allowed!");
            *flags += IPP2P_WASTE;
	    info->cmd = *flags;
	    break;																											
	case 'i':		/*cmd: xdcc*/
            if ((*flags & IPP2P_XDCC) == IPP2P_XDCC)
            exit_error(PARAMETER_PROBLEM,
                                "ipp2p: `--ares' may only be "
                                "specified once!");
	    if (invert) exit_error(PARAMETER_PROBLEM, "ipp2p: invert [!] is not allowed!");
            *flags += IPP2P_XDCC;
	    info->cmd = *flags;
	    break;																											

	case 'j':		/*cmd: debug*/
	    if (invert) exit_error(PARAMETER_PROBLEM, "ipp2p: invert [!] is not allowed!");
	    info->debug = 1;
	    break;																											

	default:
//            exit_error(PARAMETER_PROBLEM,
//	    "\nipp2p-parameter problem: for ipp2p usage type: iptables -m ipp2p --help\n");
	    return 0;
    }
    return 1;
}

static void ipp2p_mt_check(unsigned int flags)
{
    if (!flags)
            exit_error(PARAMETER_PROBLEM,
	    "\nipp2p-parameter problem: for ipp2p usage type: iptables -m ipp2p --help\n");
}

static const char *const ipp2p_cmds[] = {
	[IPP2N_EDK]        = "--edk",
	[IPP2N_DATA_KAZAA] = "--kazaa-data",
	[IPP2N_DATA_EDK]   = "--edk-data",
	[IPP2N_DATA_DC]    = "--dc-data",
	[IPP2N_DC]         = "--dc",
	[IPP2N_DATA_GNU]   = "--gnu-data",
	[IPP2N_GNU]        = "--gnu",
	[IPP2N_KAZAA]      = "--kazaa",
	[IPP2N_BIT]        = "--bit",
	[IPP2N_APPLE]      = "--apple",
	[IPP2N_SOUL]       = "--soul",
	[IPP2N_WINMX]      = "--winmx",
	[IPP2N_ARES]       = "--ares",
	[IPP2N_MUTE]       = "--mute",
	[IPP2N_WASTE]      = "--waste",
	[IPP2N_XDCC]       = "--xdcc",
};

static void
ipp2p_mt_print1(const struct ipt_ip *ip, const struct ipt_entry_match *match,
               int numeric)
{
	const struct ipt_p2p_info *info = (const void *)match->data;
	unsigned int i;

	for (i = IPP2N_EDK; i <= IPP2N_XDCC; ++i)
		if (info->cmd & (1 << i))
			printf(" %s ", ipp2p_cmds[i]);

	if (info->debug != 0)
		printf(" --debug ");
}

static void
ipp2p_mt_print(const struct ipt_ip *ip,
        const struct ipt_entry_match *match,
	int numeric)
{
	printf(" ipp2p ");
	ipp2p_mt_print1(ip, match, true);
}

static void ipp2p_mt_save(const struct ipt_ip *entry, const struct xt_entry_match *match)
{
	ipp2p_mt_print1(entry, match, true);
}

static struct iptables_match ipp2p_mt_reg = {
	.version       = IPTABLES_VERSION,
	.name          = "ipp2p",
	.revision      = 1,
	.size          = IPT_ALIGN(sizeof(struct ipt_p2p_info)),
	.userspacesize = IPT_ALIGN(sizeof(struct ipt_p2p_info)),
//	.help          = ipp2p_mt_help,
	.parse         = ipp2p_mt_parse,
	.final_check   = ipp2p_mt_check,
	.print         = ipp2p_mt_print,
	.save          = ipp2p_mt_save,
	.extra_opts    = ipp2p_mt_opts,
};

void _init(void)
{
    register_match(&ipp2p_mt_reg);
}
