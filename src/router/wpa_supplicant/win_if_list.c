/*
 * win_if_list - Display network interfaces with description (for Windows)
 * Copyright (c) 2004-2005, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 *
 * This small tool is for the Windows build to provide an easy way of fetching
 * a list of available network interfaces.
 */

#include "pcap.h"
#include <winsock.h>

static void show_dev(pcap_if_t *dev)
{
	printf("ifname: %s\ndescription: %s\n\n",
	       dev->name, dev->description);
}


int main(int argc, char *argv[])
{
	pcap_if_t *devs, *dev;
	char err[PCAP_ERRBUF_SIZE + 1];

	if (pcap_findalldevs(&devs, err) < 0) {
		fprintf(stderr, "Error - pcap_findalldevs: %s\n", err);
		return -1;
	}

	for (dev = devs; dev; dev = dev->next) {
		show_dev(dev);
	}

	pcap_freealldevs(devs);

	return 0;
}
