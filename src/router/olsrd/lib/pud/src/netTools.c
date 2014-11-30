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

	return &((struct sockaddr_in *)(void *) &ifr->ifr_addr)->sin_addr;
}
