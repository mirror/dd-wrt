#ifndef _PUD_NETTOOLS_H_
#define _PUD_NETTOOLS_H_

/* Plugin includes */

/* OLSR includes */
#include "olsr_types.h"

/* System includes */
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
#include <net/if.h>
#include <netinet/in.h>


#ifdef __ANDROID__
typedef __in_port_t in_port_t;
#endif /* __ANDROID_API__ */

/**
 Get the port in an OLSR socket address

 @param addr
 A pointer to OLSR socket address
 @param defaultPort
 The default port (in host byte order), returned when addr is NULL
 @return
 The port (in network byte order)
 */
static inline in_port_t getOlsrSockaddrPort(union olsr_sockaddr * addr, in_port_t defaultPort) {
	if (!addr) {
		return htons(defaultPort);
	}

	if (addr->in.sa_family == AF_INET) {
		return addr->in4.sin_port;
	} else {
		return addr->in6.sin6_port;
	}
}

/**
 Set the port in an OLSR socket address

 @param addr
 A pointer to OLSR socket address
 @param port
 The port (in network byte order)
 */
static inline void setOlsrSockaddrPort(union olsr_sockaddr * addr, in_port_t port) {
	if (!addr) {
		return;
	}

	if (addr->in.sa_family == AF_INET) {
		addr->in4.sin_port = port;
	} else {
		addr->in6.sin6_port = port;
	}
}

/**
 Set the IP address in an OLSR socket address

 @param addr
 A pointer to OLSR socket address
 @param addr
 A pointer to the IP address (in network byte order)
 */
static inline void setOlsrSockaddrAddr(union olsr_sockaddr * addr, void * ip) {
	if (!addr) {
		return;
	}

	if (addr->in.sa_family == AF_INET) {
		addr->in4.sin_addr = *((struct in_addr *)ip);
	} else {
		addr->in6.sin6_addr = *((struct in6_addr *)ip);
	}
}

/**
 Determine whether an IP address (v4 or v6) is a multicast address.

 @param addr
 An IP address (v4 or v6)

 @return
 - true when the address is a multicast address
 - false otherwise
 */
static inline bool isMulticast(union olsr_sockaddr *addr) {
	assert(addr != NULL);

	if (addr->in.sa_family == AF_INET) {
		return IN_MULTICAST(ntohl(addr->in4.sin_addr.s_addr));
	}

	return IN6_IS_ADDR_MULTICAST(&addr->in6.sin6_addr);
}


unsigned char * getHardwareAddress(const char * ifName, int family,
		struct ifreq *ifr);

struct in_addr * getIPv4Address(const char * ifName, struct ifreq *ifr);

#endif /* _PUD_NETTOOLS_H_ */
