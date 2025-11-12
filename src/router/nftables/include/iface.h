#ifndef _NFTABLES_IFACE_H_
#define _NFTABLES_IFACE_H_

#include <net/if.h>
#include <list.h>

struct iface {
	struct list_head	list;
	char			name[IFNAMSIZ];
	uint32_t		ifindex;
};

unsigned int nft_if_nametoindex(const char *name);
char *nft_if_indextoname(unsigned int ifindex, char *name);

void iface_cache_update(void);
void iface_cache_release(void);

const struct iface *iface_cache_get_next_entry(const struct iface *prev);
#endif
