// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Andreas Langer <an.langer@gmx.de>, Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "functions.h"
#include "bat-hosts.h"


static void translate_usage(void)
{
	fprintf(stderr, "Usage: batctl [options] translate mac|bat-host|host_name|IPv4_address\n");
}

static int translate(struct state *state, int argc, char **argv)
{
	struct ether_addr *dst_mac = NULL;
	struct bat_host *bat_host;
	int ret = EXIT_FAILURE;
	char *dst_string, *mac_string;

	if (argc <= 1) {
		fprintf(stderr, "Error - destination not specified\n");
		translate_usage();
		return EXIT_FAILURE;
	}

	check_root_or_die("batctl translate");

	dst_string = argv[1];
	bat_hosts_init(0);
	bat_host = bat_hosts_find_by_name(dst_string);

	if (bat_host)
		dst_mac = &bat_host->mac_addr;

	if (!dst_mac) {
		dst_mac = resolve_mac(dst_string);

		if (!dst_mac) {
			fprintf(stderr, "Error - mac address of the ping destination could not be resolved and is not a bat-host name: %s\n", dst_string);
			goto out;
		}
	}

	dst_mac = translate_mac(state, dst_mac);
	if (dst_mac) {
		mac_string = ether_ntoa_long(dst_mac);
		printf("%s\n", mac_string);
		ret = EXIT_SUCCESS;
	} else {
		ret = EXIT_NOSUCCESS;
	}

out:
	bat_hosts_free();
	return ret;
}

COMMAND(SUBCOMMAND_MIF, translate, "t",
	COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK, NULL,
	"<destination>     \ttranslate a destination to the originator responsible for it");
