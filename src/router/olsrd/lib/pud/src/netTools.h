#ifndef _PUD_NETTOOLS_H_
#define _PUD_NETTOOLS_H_

/* Plugin includes */

/* OLSR includes */
#include "olsr_types.h"

/* System includes */
#include <stdbool.h>
#include <net/if.h>

bool isMulticast(int addressFamily, union olsr_sockaddr *addr);

unsigned char * getHardwareAddress(const char * ifName, int family,
		struct ifreq *ifr);

#endif /* _PUD_NETTOOLS_H_ */
