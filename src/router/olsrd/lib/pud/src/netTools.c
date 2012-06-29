#include "netTools.h"

/* Plugin includes */
#include "pud.h"

/* OLSR includes */

/* System includes */
#include <assert.h>
#include <sys/ioctl.h>
#include <unistd.h>

/**
 Determine whether an IP address (v4 or v6) is a multicast address.

 @param addressFamily
 The address family (AF_INET or AF_INET6)
 @param addr
 An IP address (v4 or v6)

 @return
 - true when the address is a multicast address
 - false otherwise
 */
bool isMulticast(int addressFamily, union olsr_sockaddr *addr) {
	assert(addr != NULL);
	assert((addressFamily == AF_INET) || (addressFamily == AF_INET6));

	if (addressFamily == AF_INET) {
		return IN_MULTICAST(ntohl(addr->in4.sin_addr.s_addr));
	}

	return IN6_IS_ADDR_MULTICAST(&addr->in6.sin6_addr);
}

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
	int cpySize;

	assert(ifName != NULL);
	assert(strlen(ifName) <= IFNAMSIZ);
	assert((family == AF_INET) || (family == AF_INET6));
	assert(ifr != NULL);

	fd = socket(family, SOCK_DGRAM, 0);

	ifr->ifr_addr.sa_family = family;
	memset(ifr->ifr_name, 0, sizeof(ifr->ifr_name));
	cpySize = (strlen(ifName) < sizeof(ifr->ifr_name)) ? strlen(ifName)
			: sizeof(ifr->ifr_name);
	strncpy(ifr->ifr_name, ifName, cpySize);

	errno = 0;
	if (ioctl(fd, SIOCGIFHWADDR, ifr) < 0) {
		pudError(true, "%s@%u: ioctl(SIOCGIFHWADDR) error", __FILE__, __LINE__);
		close(fd);
		return NULL;
	}

	close(fd);

	return (unsigned char *) &ifr->ifr_hwaddr.sa_data[0];
}
