#ifndef IPTRAF_NG_LANDESC_H
#define IPTRAF_NG_LANDESC_H

/***

ethdesc.c	- Ethernet host description management module

***/

#include "list.h"

#define WITHETCETHERS 1
#define WITHOUTETCETHERS 0

struct eth_desc {
	struct list_head hd_list;
	char hd_mac[18];
	char *hd_desc;
};

struct eth_desc *load_eth_desc(unsigned link_type);

void free_eth_desc(struct eth_desc *hd);

void manage_eth_desc(unsigned int linktype);

#endif	/* IPTRAF_NG_LANDESC_H */
