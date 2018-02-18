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

#include "netTools.h"

/* Plugin includes */
#include "pud.h"

/* OLSR includes */

/* System includes */
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

/**
 Get the hardware address (MAC) of an interface

 @param ifName
 the name of the interface
 @param family
 the protocol family (AF_INET or AF_INET6)
 @param ifr
 the buffer in which to write the hardware address (MAC)

 @return
 - the pointer to the hardware address (inside ifr)
 - NULL on failure
 */
unsigned char * getHardwareAddress(const char * ifName, int family,
		struct ifreq *ifr) {
	int fd;

	assert(ifName != NULL);
	assert(strlen(ifName) <= sizeof(ifr->ifr_name));
	assert((family == AF_INET) || (family == AF_INET6));
	assert(ifr != NULL);

	fd = socket(family, SOCK_DGRAM, 0);
	if (fd < 0) {
		pudError(true, "%s@%u: socket error: %s", __FILE__, __LINE__, strerror(errno));
		return NULL;
	}

	ifr->ifr_addr.sa_family = family;
	memset(ifr->ifr_name, 0, sizeof(ifr->ifr_name));
	strncpy(ifr->ifr_name, ifName, sizeof(ifr->ifr_name));

	errno = 0;
	if (ioctl(fd, SIOCGIFHWADDR, ifr) < 0) {
		pudError(true, "%s@%u: ioctl(SIOCGIFHWADDR) error", __FILE__, __LINE__);
		close(fd);
		return NULL;
	}

	close(fd);

	return (unsigned char *) &ifr->ifr_hwaddr.sa_data[0];
}

/**
 Get the IPv4 address of an interface

 @param ifName
 the name of the interface
 @param ifr
 the buffer in which to write the IPv4 address

 @return
 - the pointer to the IPv4 address (inside ifr)
 - NULL on failure
 */
struct in_addr * getIPv4Address(const char * ifName, struct ifreq *ifr) {
	int fd;

	assert(ifName != NULL);
	assert(strlen(ifName) <= sizeof(ifr->ifr_name));
	assert(ifr != NULL);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		pudError(true, "%s@%u: socket error: %s", __FILE__, __LINE__, strerror(errno));
		return NULL;
	}

	ifr->ifr_addr.sa_family = AF_INET;
	memset(ifr->ifr_name, 0, sizeof(ifr->ifr_name));
	strncpy(ifr->ifr_name, ifName, sizeof(ifr->ifr_name));

	errno = 0;
	if (ioctl(fd, SIOCGIFADDR, ifr) < 0) {
		pudError(true, "%s@%u: ioctl(SIOCGIFADDR) error", __FILE__, __LINE__);
		close(fd);
		return NULL;
	}

	close(fd);

	{
	  struct sockaddr* ifra = &ifr->ifr_addr;
	  return &((struct sockaddr_in *)(void *) ifra)->sin_addr;
	}
}
