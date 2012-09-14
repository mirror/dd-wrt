#ifndef IPTRAF_NG_OPTIONS_H
#define IPTRAF_NG_OPTIONS_H

struct OPTIONS {
	unsigned int color:1, logging:1, revlook:1, servnames:1, promisc:1,
	    actmode:1, mac:1, v6inv4asv6:1, dummy:8;
	time_t timeout;
	time_t logspan;
	time_t updrate;
	time_t closedint;
};

#define DEFAULT_UPDATE_DELAY 50000	/* usec screen delay if update rate 0 */

#include "serv.h"

void setoptions(struct OPTIONS *options, struct porttab **ports);
void loadoptions(struct OPTIONS *options);
void saveoptions(struct OPTIONS *options);

#endif	/* IPTRAF_NG_OPTIONS_H */
