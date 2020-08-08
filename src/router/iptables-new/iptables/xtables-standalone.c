/*
 * Author: Paul.Russell@rustcorp.com.au and mneuling@radlogic.com.au
 *
 * Based on the ipchains code by Paul Russell and Michael Neuling
 *
 * (C) 2000-2002 by the netfilter coreteam <coreteam@netfilter.org>:
 * 		    Paul 'Rusty' Russell <rusty@rustcorp.com.au>
 * 		    Marc Boucher <marc+nf@mbsi.ca>
 * 		    James Morris <jmorris@intercode.com.au>
 * 		    Harald Welte <laforge@gnumonks.org>
 * 		    Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 *
 *	iptables -- IP firewall administration for kernels with
 *	firewall table (aimed for the 2.3 kernels)
 *
 *	See the accompanying manual page iptables(8) for information
 *	about proper usage of this program.
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <iptables.h>
#include "xtables-multi.h"
#include "nft.h"

static int
xtables_main(int family, const char *progname, int argc, char *argv[])
{
	int ret;
	char *table = "filter";
	struct nft_handle h;

	xtables_globals.program_name = progname;
	ret = xtables_init_all(&xtables_globals, family);
	if (ret < 0) {
		fprintf(stderr, "%s/%s Failed to initialize xtables\n",
				xtables_globals.program_name,
				xtables_globals.program_version);
				exit(1);
	}
#if defined(ALL_INCLUSIVE) || defined(NO_SHARED_LIBS)
	init_extensions();
	init_extensions4();
#endif

	if (nft_init(&h, family, xtables_ipv4) < 0) {
		fprintf(stderr, "%s/%s Failed to initialize nft: %s\n",
				xtables_globals.program_name,
				xtables_globals.program_version,
				strerror(errno));
		exit(EXIT_FAILURE);
	}

	ret = do_commandx(&h, argc, argv, &table, false);
	if (ret)
		ret = nft_commit(&h);

	nft_fini(&h);
	xtables_fini();

	if (!ret) {
		if (errno == EINVAL) {
			fprintf(stderr, "iptables: %s. "
					"Run `dmesg' for more information.\n",
				nft_strerror(errno));
		} else {
			fprintf(stderr, "iptables: %s.\n",
				nft_strerror(errno));
		}
		if (errno == EAGAIN)
			exit(RESOURCE_PROBLEM);
	}

	exit(!ret);
}

int xtables_ip4_main(int argc, char *argv[])
{
	return xtables_main(NFPROTO_IPV4, "iptables", argc, argv);
}

int xtables_ip6_main(int argc, char *argv[])
{
	return xtables_main(NFPROTO_IPV6, "ip6tables", argc, argv);
}
