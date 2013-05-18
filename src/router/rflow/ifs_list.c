#include "headers.h"
#include "ifs_list.h"

#ifdef	IPCAD_IFLIST_USE_GETIFADDRS

slist *
get_interface_names() {
	struct ifaddrs *ifap = NULL;
	struct ifaddrs *ifp;
	slist *sl;

	sl = sinit();
	if(sl == NULL) return NULL;

	if(getifaddrs(&ifap)) {
		sfree(sl);
		return NULL;
	}

	for(ifp = ifap; ifp; ifp = ifp->ifa_next) {
		if(ifp->ifa_addr == 0
		|| ifp->ifa_addr->sa_family != AF_INET)
			continue;
		if(sadd(sl, ifp->ifa_name) == -1) {
			sfree(sl);
			sl = NULL;
			break;
		}
	}

	freeifaddrs(ifap);
	return sl;
}

#else	/* !IPCAD_IFLIST_USE_GETIFADDRS */
#ifndef	IFST_linux
#ifdef	HAVE_PCAP_XALLDEVS

slist *
get_interface_names() {
	slist *sv;
	pcap_if_t *devs = NULL;
	pcap_if_t *dev;
	int ret;

	sv = sinit();
	if(sv == NULL) return NULL;

	ret = pcap_findalldevs(&devs, NULL);
	if(ret) {
		sfree(sv);
		return NULL;
	}

	for(dev = devs; dev; dev = dev->next) {
		if(sadd(sv, dev->name) == -1) {
			sfree(sv);
			sv = NULL;
			break;
		}
	}

	pcap_freealldevs(devs);
	return sv;
}

#else /* ! HAVE_PCAP_XALLDEVS */

#warning Your libpcap(3) library is too old, please upgrade.
#warning pcap_findalldevs() and pcap_freealldevs() functions are required
#warning to provide support for dynamic interfaces.

slist *
get_interface_names() {
	errno = EINVAL;
	return NULL;
}

#endif	/* HAVE_PCAP_XALLDEVS */
#endif	/* !IFST_linux */
#endif	/* IPCAD_IFLIST_USE_GETIFADDRS */
