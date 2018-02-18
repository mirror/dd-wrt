/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#ifndef _PUD_NETTOOLS_H_
#define _PUD_NETTOOLS_H_

/* Plugin includes */

/* OLSR includes */
#include "olsr_types.h"
#include "defs.h"

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
static INLINE in_port_t getOlsrSockaddrPort(union olsr_sockaddr * addr, in_port_t defaultPort) {
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
static INLINE void setOlsrSockaddrPort(union olsr_sockaddr * addr, in_port_t port) {
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
static INLINE void setOlsrSockaddrAddr(union olsr_sockaddr * addr, union olsr_sockaddr * ip) {
	if (!addr) {
		return;
	}

	if (addr->in.sa_family == AF_INET) {
	  addr->in4.sin_addr = ip->in4.sin_addr;
	} else {
	  addr->in6.sin6_addr = ip->in6.sin6_addr;
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
static INLINE bool isMulticast(union olsr_sockaddr *addr) {
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
