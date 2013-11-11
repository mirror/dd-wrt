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

extern struct OPTIONS options;

void setoptions(void);
void loadoptions(void);
void saveoptions(void);

#endif	/* IPTRAF_NG_OPTIONS_H */
