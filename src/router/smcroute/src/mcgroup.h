/* IGMP/MLD group subscription API */
#ifndef SMCROUTE_MCGROUP_H_
#define SMCROUTE_MCGROUP_H_

#include "inet.h"
#include "queue.h"

struct mcgroup {
	TAILQ_ENTRY(mcgroup) link;
	int            unused;

	char           ifname[IFNAMSIZ];
	struct iface  *iface;
	inet_addr_t    source;
	uint8_t        src_len;
	inet_addr_t    group;
	uint8_t        len;
	int            sd;
};

void mcgroup_reload_beg(void);
void mcgroup_reload_end(void);
void mcgroup_prune     (char *ifname);

void mcgroup_init      (void);
void mcgroup_exit      (void);

int  mcgroup_action    (int cmd, const char *ifname, inet_addr_t *source, int src_len, inet_addr_t *group, int len);

int  mcgroup_show      (int sd, int detail);

#endif /* SMCROUTE_MCGROUP_H_ */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
