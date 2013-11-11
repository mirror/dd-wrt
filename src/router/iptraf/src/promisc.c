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

static void promisc_add_dev(struct list_head *promisc, const char *dev_name)
{
	struct promisc_list *p = xmallocz(sizeof(*p));
	strcpy(p->ifname, dev_name);
	INIT_LIST_HEAD(&p->list);

	list_add_tail(&p->list, promisc);
}

void promisc_init(struct list_head *promisc, const char *device_name)
{
	if (device_name) {
		int flags = dev_promisc_flag(device_name);
		if (flags < 0)
			return;

		promisc_add_dev(promisc, device_name);

		return;
	}

	FILE *fp = open_procnetdev();
	if (!fp)
		die_errno("%s: open_procnetdev", __func__);

	char dev_name[IFNAMSIZ];
	while (get_next_iface(fp, dev_name, sizeof(dev_name))) {
		if (!strcmp(dev_name, ""))
			continue;

		int flags = dev_promisc_flag(dev_name);
		if (flags < 0)
			continue;

		promisc_add_dev(promisc, dev_name);
	}

	fclose(fp);
}

void promisc_set_list(struct list_head *promisc)
{
	struct promisc_list *entry = NULL;
	list_for_each_entry(entry, promisc, list) {
		int r = dev_set_promisc(entry->ifname);
		if (r < 0)
			write_error("Failed to set promiscuous mode on %s", entry->ifname);
	}
}

void promisc_restore_list(struct list_head *promisc)
{
	struct promisc_list *entry = NULL;
	list_for_each_entry(entry, promisc, list) {
		int r = dev_clr_promisc(entry->ifname);
		if (r < 0)
			write_error("Failed to clear promiscuous mode on %s", entry->ifname);
	}
}

void promisc_destroy(struct list_head *promisc)
{
	struct promisc_list *entry, *tmp;
	list_for_each_entry_safe(entry, tmp, promisc, list) {
		list_del(&entry->list);
		free(entry);
	}
}
