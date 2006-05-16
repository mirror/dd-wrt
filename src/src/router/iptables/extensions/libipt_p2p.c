/*
 * $Id: libipt_p2p.c,v 1.16 2004/02/29 00:53:13 liquidk Exp $
 *
 * Iptables match extension for matching commonly used peer-to-peer
 * services.  See the kernel module for implementation details.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <iptables.h>
#include <linux/netfilter_ipv4/ipt_p2p.h>


/*****************************************************************************/

/* Translates a named protocol option to an integer proto value; See
   arg_to_proto for usage. */
static struct
{
	const char *name;
	int         proto;
}

protomap[] =
{
	{ "all",        IPT_P2P_PROTO_ALL            },
	{ "fasttrack",  IPT_P2P_PROTO_FASTTRACK      },
	{ "gnutella",   IPT_P2P_PROTO_GNUTELLA       },
	{ "edonkey",    IPT_P2P_PROTO_EDONKEY        },
	{ "dc",         IPT_P2P_PROTO_DIRECT_CONNECT },
	{ "bittorrent", IPT_P2P_PROTO_BITTORRENT     },
	{ "openft",     IPT_P2P_PROTO_OPENFT         }
};

#define PROTOMAP_NELEM (sizeof(protomap) / (sizeof(protomap[0])))

/*****************************************************************************/

static void help(void);
static void init(struct ipt_entry_match *m, unsigned int *nfcache);
static int parse(int c, char **argv, int invert, unsigned int *flags,
                 const struct ipt_entry *entry, unsigned int *nfcache,
                 struct ipt_entry_match **match);
static void final_check(unsigned int flags);
static void print(const struct ipt_ip *ip, const struct ipt_entry_match *m,
                  int numeric);
static void save(const struct ipt_ip *ip, const struct ipt_entry_match *m);

static struct option opts[] =
{
	{ "p2p-protocol", 1, NULL, 'P' },
	{ "p2p",          1, NULL, 'P' },  /* Synonym. */
	{ NULL }
};
static struct iptables_match p2p = { 
	.next		= NULL,
	.name		= "p2p",
	.version	= IPTABLES_VERSION,
	.size		= IPT_ALIGN(sizeof(struct ipt_p2p_info)),
	.userspacesize	= IPT_ALIGN(sizeof(struct ipt_p2p_info)),
//	.help		= &help,
	.init		= &init,
	.parse		= &parse,
	.final_check	= &final_check,
	.print		= &print,
	.save		= &save,
	.extra_opts	= opts
};

/*****************************************************************************/

/* Access what I can only assume is user data that will be given to our
   kernel module companion as is. */
#define IPT_P2P_INFO(match) ((struct ipt_p2p_info *)(match)->data)
#define IPT_P2P_INFO_const(match) ((const struct ipt_p2p_info *)(match)->data)

/*****************************************************************************/

/* Prints usage. */
static void help(void)
{
	int i;

	printf(
"P2P match v%s options:\n"
"  --p2p-protocol [!] protocol[,...]\n"
"  --p2p ...\n"
"                                match application-layer protocol\n",
	IPT_P2P_VERSION);

	printf("Valid p2p protocols:\n");

	for (i = 1; i < PROTOMAP_NELEM; i++)
		printf("\t%s\n", protomap[i].name);
}

/* Initialize the match. */
static void init(struct ipt_entry_match *m, unsigned int *nfcache)
{
	struct ipt_p2p_info *pinfo = IPT_P2P_INFO(m);

	/* Can't cache this. */
	*nfcache |= NFC_UNKNOWN;

	/* Initialize default match options. */
	pinfo->proto = IPT_P2P_PROTO_ALL;
}

static int arg_to_proto(const char *arg)
{
	unsigned int i;
	unsigned long constant;
	char *argend;

	/* First check if they specified an integer constant. */
	constant = strtoul(arg, &argend, 0);

	/* Require that strtoul() slurp up the entire argument to succeed. */
	if (argend[0] == '\0')
	{
		/* Successfully matched an integer constant. */
		if (constant > 0 && constant <= IPT_P2P_PROTO_ALL)
			return constant;
	}
	else
	{
		/* Try to match a protocol literal. */
		for (i = 0; i < PROTOMAP_NELEM; i++)
		{
			if (strcasecmp(protomap[i].name, arg) == 0)
				return protomap[i].proto;
		}
	}

	/* No match, croak. */
	exit_error(PARAMETER_PROBLEM,
	           "P2P match: Unknown protocol `%s'", arg);

	/* Just to satisfy the compiler. */
	return 0;
}

/*
 * Arg may be in the following forms:
 *
 *   fasttrack,gnutella     = IPT_P2P_PROTO_FASTTRACK | IPT_P2P_PROTO_GNUTELLA
 *   0x6                    = IPT_P2P_PROTO_GNUTELLA  | IPT_P2P_PROTO_EDONKEY
 *   edonkey,16             = IPT_P2P_PROTO_EDONKEY   | IPT_P2P_PROTO_BITTORRENT
 */
static int arglist_to_proto(const char *arg, int invert)
{
	char buf[32];                      /* Large enough to store proto name. */
	size_t protolen;
	int protoret;

	if (invert)
		protoret = IPT_P2P_PROTO_ALL;
	else
		protoret = 0;

	while (1)
	{
		if ((protolen = strcspn(arg, ",")) == 0)
			continue;

		if (protolen + 1 > sizeof(buf))
			continue;

		strncpy(buf, arg, protolen);
		buf[protolen] = '\0';

		/* Handle a single parameter. */
		if (invert)
			protoret &= ~(arg_to_proto(buf));
		else
			protoret |= arg_to_proto(buf);

		arg += protolen;

		/* We reached the end of the list gracefully. */
		if (arg[0] == '\0')
			break;

		/* Move beyond the ','. */
		arg++;
	}

	return protoret;
}

static char *proto_to_arg(int proto)
{
	static char buf[256];
	size_t buflen;
	int i;

	buflen = 0;
	buf[buflen] = '\0';

    if (proto == IPT_P2P_PROTO_ALL) {
        strcpy(buf, "all");
        return buf;
    }

	for (i = 0; i < PROTOMAP_NELEM; i++)
	{
		if ((proto & protomap[i].proto) == protomap[i].proto)
		{
			size_t namelen;

			namelen = strlen(protomap[i].name);

			if (buflen + namelen + 2 > sizeof (buf))
				break;

			strcpy(buf + buflen, protomap[i].name);
			buflen += namelen;

			strcpy(buf + buflen, ",");
			buflen += 1;
		}
	}

	/* rewind the last ", " appended to the buffer */
	if (buflen > 1)
		buf[buflen - 1] = '\0';

	return buf;
}

/* Parses command options; returns true if it ate an option. */
static int parse(int c, char **argv, int invert, unsigned int *flags,
                 const struct ipt_entry *entry,
                 unsigned int *nfcache,
                 struct ipt_entry_match **match)
{
	struct ipt_p2p_info *pinfo = IPT_P2P_INFO(*match);

	switch (c)
	{
	 /* TODO: Croak if -P is specified multiple times. */
	 case 'P':
		/* Where the hell is check_inverse() defined, and what the hell
		   does it do? */
		check_inverse(optarg, &invert, &optind, 0);
		pinfo->proto = arglist_to_proto(argv[optind-1], invert);

		if (pinfo->proto == 0)
		{
			exit_error(PARAMETER_PROBLEM,
			           "P2P match: May not specify inverted 'all'");
		}

		break;

	 default:
		return 0;
	}

	return 1;
}

/* Final check; must have specified --mac. */
static void final_check(unsigned int flags)
{
	/* Do nothing. */
}

static void print(const struct ipt_ip *ip, const struct ipt_entry_match *m,
                  int numeric)
{
	const struct ipt_p2p_info *pinfo = IPT_P2P_INFO_const(m);

	printf("P2P match %s ", proto_to_arg(pinfo->proto));
}

static void save(const struct ipt_ip *ip, const struct ipt_entry_match *m)
{
	const struct ipt_p2p_info *pinfo = IPT_P2P_INFO_const(m);

	printf("--p2p-protocol %s ", proto_to_arg(pinfo->proto));
}

void _init(void)
{
	register_match(&p2p);
}
