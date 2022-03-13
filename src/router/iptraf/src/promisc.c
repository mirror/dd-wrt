/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

promisc.c	- handles the promiscuous mode flag for the Ethernet/FDDI/
              Token Ring interfaces

***/

#include "iptraf-ng-compat.h"

#include "ifaces.h"
#include "error.h"
#include "promisc.h"

struct promisc_list {
	struct list_head list;
	int ifindex;
	char ifname[IFNAMSIZ];
};

static bool promisc_dev_suitable(const char *dev_name)
{
	int flags = dev_get_flags(dev_name);
	if (flags < 0)
		return false;

	if ((flags & (IFF_UP | IFF_PROMISC)) == IFF_UP)
		return true;
	else
		return false;
}

static int sock_change_promisc(int sock, int action, int ifindex)
{
	struct packet_mreq mreq;

	mreq.mr_ifindex = ifindex;
	mreq.mr_type = PACKET_MR_PROMISC;

	return setsockopt(sock, SOL_PACKET, action, &mreq, sizeof(mreq));
}

static int sock_enable_promisc(int sock, int ifindex)
{
	return sock_change_promisc(sock, PACKET_ADD_MEMBERSHIP, ifindex);
}

static int sock_disable_promisc(int sock, int ifindex)
{
	return sock_change_promisc(sock, PACKET_DROP_MEMBERSHIP, ifindex);
}

static void promisc_enable_dev(struct list_head *promisc, int sock, const char *dev)
{
	if (!promisc_dev_suitable(dev))
		return;

	int ifindex = dev_get_ifindex(dev);
	if (ifindex < 0)
		return;

	int r = sock_enable_promisc(sock, ifindex);
	if (r < 0) {
		write_error("Failed to set promiscuous mode on %s", dev);
		return;
	}

	struct promisc_list *new = xmallocz(sizeof(*new));

	new->ifindex = ifindex;
	strcpy(new->ifname, dev);
	list_add_tail(&new->list, promisc);
}

void promisc_enable(int sock, struct list_head *promisc, const char *device_name)
{
	if (device_name) {
		promisc_enable_dev(promisc, sock, device_name);
		return;
	}

	FILE *fp = open_procnetdev();
	if (!fp)
		die_errno("%s: open_procnetdev", __func__);

	char dev_name[IFNAMSIZ];
	while (get_next_iface(fp, dev_name, sizeof(dev_name))) {
		if (!strcmp(dev_name, ""))
			continue;

		promisc_enable_dev(promisc, sock, dev_name);
	}

	fclose(fp);
}

void promisc_disable(int sock, struct list_head *promisc)
{
	struct promisc_list *entry, *tmp;
	list_for_each_entry_safe(entry, tmp, promisc, list) {
		int r = sock_disable_promisc(sock, entry->ifindex);
		if (r < 0)
			write_error("Failed to clear promiscuous mode on %s", entry->ifname);

		list_del(&entry->list);
		free(entry);
	}
}
