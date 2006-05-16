/*
 *  Copyright 2002 Tobias Ringstrom <tobias@ringstrom.mine.nu>
 *  Authentication Copyright 2002 Arcturus Networks Inc.
 *      by Norman Shulman <norm@arcturusnetworks.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#include "ipsecadm.h"

#define MAX(a,b)  ((b) > (a) ? (b) : (a))

struct ipsec_stats
{
	unsigned long	version;

	unsigned long	rx_unknown_sa;
	unsigned long	rx_auth_fail;
	unsigned long	rx_padding_fail;
	unsigned long	rx_non_tunnel;
	unsigned long	rx_no_tunnel;
	unsigned long	rx_mem;
	unsigned long	rx_other;
	unsigned long	rx_ok;

	unsigned long	tx_unknown_sa;
	unsigned long	tx_recursion;
	unsigned long	tx_route;
	unsigned long	tx_mtu;
	unsigned long	tx_mem;
	unsigned long	tx_other;
	unsigned long	tx_ok;
};

static void
stats_usage(void)
{
	fputs("Usage:\n", stderr);
	fputs("    ipsecadm stats help\n", stderr);
	fputs("    ipsecadm stats show\n", stderr);

	exit(1);
}

static int
stats_show(int argc, char *argv[])
{
	struct ipsec_stats stats;
	struct ifreq ifr;
	int fd, st;

	if (argc > 1)
		stats_usage();

	fd = ipsec_tunnel_open(IPSECDEVNAME, &ifr, 0);
	if (fd == -1)
		return -1;

	memset(&stats, 0, sizeof(struct ipsec_stats));
	ifr.ifr_data = (char*)&stats;

	st = ioctl(fd, SIOCIPSEC_GET_STATS, &ifr);
	if (st != 0)
		error("Cannot get statistics! [%s]\n", strerror(errno));

	printf("Accepted incoming packets: %9lu\n", stats.rx_ok);
	printf("Dropped incoming packets:  %9lu\n",
		   stats.rx_unknown_sa + stats.rx_auth_fail +
		   stats.rx_padding_fail + stats.rx_non_tunnel +
		   stats.rx_no_tunnel + stats.rx_mem +
		   stats.rx_other);
	printf("    Unknown SA:            %9lu\n", stats.rx_unknown_sa);
	printf("    Failed authentication: %9lu\n", stats.rx_auth_fail);
	printf("    Bad padding:           %9lu\n", stats.rx_padding_fail);
	printf("    Non-tunnel mode:       %9lu\n", stats.rx_non_tunnel);
	printf("    No matching tunnel:    %9lu\n", stats.rx_no_tunnel);
	printf("    Memory shortage:       %9lu\n", stats.rx_mem);
	printf("    Other errors:          %9lu\n", stats.rx_other);

	printf("Accepted outgoing packets: %9lu\n", stats.tx_ok);
	printf("Dropped outgoing packets:  %9lu\n",
		   stats.tx_unknown_sa + stats.tx_recursion +
		   stats.tx_route + stats.tx_mtu +
		   stats.tx_mem + stats.tx_other);
	printf("    Unknown SA:            %9lu\n", stats.tx_unknown_sa);
	printf("    Recursion:             %9lu\n", stats.tx_recursion);
	printf("    No route:              %9lu\n", stats.tx_route);
	printf("    MTU / packet too big:  %9lu\n", stats.tx_mtu);
	printf("    Memory shortage:       %9lu\n", stats.tx_mem);
	printf("    Other errors:          %9lu\n", stats.tx_other);

	return 0;
}

int
stats_main(int argc, char *argv[])
{
	const char *modes[] = { "help", "usage", "show", NULL };
	int mode;

	if (argc < 2)
		return stats_show(argc - 1, argv + 1);

	mode = find_unambiguous_string(modes, argv[1]);
	switch (mode)
	{
	case 0:
	case 1:
		stats_usage();
		return 0;
	case 2:
		return stats_show(argc - 1, argv + 1);
	}

	stats_usage();

	return 1;
}
