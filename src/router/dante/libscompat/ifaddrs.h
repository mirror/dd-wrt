/* $Id: ifaddrs.h,v 1.1 2012/04/21 15:34:50 karls Exp $ */

/* ifa_dstaddr used for broadcast on broadcast interface, dstaddr on p2p */
#undef ifa_broadaddr
#undef ifa_dstaddr
#define	ifa_broadaddr ifa_dstaddr

struct ifaddrs {
	struct ifaddrs  *ifa_next;
	char		*ifa_name;
	unsigned int	 ifa_flags;
	struct sockaddr	*ifa_addr;
	struct sockaddr	*ifa_netmask;
	struct sockaddr	*ifa_dstaddr;
	void		*ifa_data;
};

int
getifaddrs(struct ifaddrs **ifap);
void
freeifaddrs(struct ifaddrs *ifap);
