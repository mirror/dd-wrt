#ifndef IPTRAF_NG_PKTSIZE_H
#define IPTRAF_NG_PKTSIZE_H

#include "options.h"
#include "fltselect.h"

void packet_size_breakdown(struct OPTIONS *options, char *iface,
			   time_t facilitytime, struct filterstate *ofilter);

#endif	/* IPTRAF_NG_PKTSIZE_H */
