#ifndef IPTRAF_NG_HOSTMON_H
#define IPTRAF_NG_HOSTMON_H

#include "options.h"
#include "fltselect.h"

void hostmon(const struct OPTIONS *options, time_t facilitytime, char *ifptr,
	     struct filterstate *ofilter);

#endif	/* IPTRAF_NG_HOSTMON_H */
