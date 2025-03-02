/* Physical and virtual interface API */
#ifndef SMCROUTE_IFACE_H_
#define SMCROUTE_IFACE_H_

#include "config.h"

#include <stdint.h>
#include <net/if.h>			/* IFNAMSIZ */
#include <netinet/in.h>			/* struct in_addr */

#include "mroute.h"			/* vifit_t + mifi_t */

#ifndef ALL_MIFS
#define ALL_MIFS (vifi_t)-1
#endif

#define DEFAULT_THRESHOLD 1		/* Packet TTL must be at least 1 to pass */
#define NO_VIF   ALL_VIFS

struct iface {
	TAILQ_ENTRY(iface) link;
	int      unused;		/* set on reload/SIGHUP only */

	char     ifname[IFNAMSIZ];
	struct in_addr inaddr;		/* == 0 for non IP interfaces */
	int      ifindex;		/* Physical interface index   */
	short    flags;
	vifi_t   vif;
	mifi_t   mif;
	uint8_t  mrdisc;		/* Enable multicast router discovery */
	uint8_t  threshold;		/* TTL threshold: 1-255, default: 1 */
};

struct ifmatch {
	struct iface *iface;
	size_t match_count;
};

void          iface_init              (void);
void          iface_exit              (void);
void          iface_update            (void);

struct iface *iface_iterator          (int first);
struct iface *iface_outbound_iterator (struct mroute *route, int first);

struct iface *iface_find              (int ifindex);
struct iface *iface_find_by_name      (const char *ifname);
struct iface *iface_find_by_inbound   (struct mroute *route);

void          iface_match_init        (struct ifmatch *state);
struct iface *iface_match_by_name     (const char *ifname, int reload, struct ifmatch *state);
int           ifname_is_wildcard      (const char *ifname);

vifi_t        iface_get_vif           (int af_family, struct iface *iface);
vifi_t        iface_match_vif_by_name (const char *ifname, struct ifmatch *state, struct iface **found);
mifi_t        iface_match_mif_by_name (const char *ifname, struct ifmatch *state, struct iface **found);

int           iface_show              (int sd, int detail);

/*
 * Check if interface exists, at all, on the system
 */
static inline int iface_exist(char *ifname)
{
	struct ifmatch ifm;

	iface_match_init(&ifm);
	return iface_match_by_name(ifname, 1, &ifm) != NULL;
}

static inline int iface_ifname_maxlen(void)
{
	struct iface *iface;
	int maxlen = 0;
	int first = 1;

	while ((iface = iface_iterator(first))) {
		first = 0;
		if ((int)strlen(iface->ifname) > maxlen)
			maxlen = (int)strlen(iface->ifname);
	}

	return maxlen;
}

#endif /* SMCROUTE_IFACE_H_ */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
