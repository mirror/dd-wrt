#ifndef IPTRAF_NG_DETSTATS_H
#define IPTRAF_NG_DETSTATS_H

#include "options.h"
#include "fltselect.h"

void detstats(char *iface, const struct OPTIONS *options, time_t facilitytime,
	      struct filterstate *ofilter);

#endif /* IPTRAF_NG_DETSTATS_H */
