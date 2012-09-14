#ifndef IPTRAF_NG_ITRAFMON_H
#define IPTRAF_NG_ITRAFMON_H

#include "options.h"
#include "fltselect.h"

void ipmon(struct OPTIONS *options, struct filterstate *ofilter,
	   time_t facilitytime, char *ifptr);

#endif /* IPTRAF_NG_ITRAFMON_H */
