#ifndef UDPFROMTO_H
#define UDPFROMTO_H
/*
 * Version:	$Id: 525df38c64cceff0c48850c49bb4ca5c98c58549 $
 *
 */

#include <freeradius-devel/ident.h>
RCSIDH(udpfromtoh, "$Id: 525df38c64cceff0c48850c49bb4ca5c98c58549 $")

#include <freeradius-devel/autoconf.h>
#include <freeradius-devel/libradius.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WITH_UDPFROMTO
int udpfromto_init(int s);
int recvfromto(int s, void *buf, size_t len, int flags,
	       struct sockaddr *from, socklen_t *fromlen,
	       struct sockaddr *to, socklen_t *tolen);
int sendfromto(int s, void *buf, size_t len, int flags,
	       struct sockaddr *from, socklen_t fromlen,
	       struct sockaddr *to, socklen_t tolen);
#endif

#ifdef __cplusplus
}
#endif

#endif
