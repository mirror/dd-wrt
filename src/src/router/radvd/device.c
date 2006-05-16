/*
 *   $Id: device-linux.c,v 1.8 2002/07/02 06:49:20 psavola Exp $
 *
 *   Authors:
 *    Lars Fenneberg		<lf@elemental.net>	 
 *
 *   This software is Copyright 1996,1997 by the above mentioned author(s), 
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <lutchann@litech.org>.
 *
 */

#include <config.h>
#include <includes.h>
#include <radvd.h>
#include <defaults.h>
#include <pathnames.h>		/* for PATH_PROC_NET_IF_INET6 */

#ifndef IPV6_ADDR_LINKLOCAL
#define IPV6_ADDR_LINKLOCAL   0x0020U
#endif

/*
 * this function gets the hardware type and address of an interface,
 * determines the link layer token length and checks it against
 * the defined prefixes
 */
int
setup_deviceinfo(int sock, struct Interface *iface)
{
	struct ifreq	ifr;
	struct AdvPrefix *prefix;
	
	strncpy(ifr.ifr_name, iface->Name, IFNAMSIZ-1);
	ifr.ifr_name[IFNAMSIZ-1] = '\0';

	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
	{
		log(LOG_ERR, "ioctl(SIOCGIFHWADDR) failed for %s: %s",
			iface->Name, strerror(errno));
		return (-1);
	}

	dlog(LOG_DEBUG, 3, "hardware type for %s is %d", iface->Name,
		ifr.ifr_hwaddr.sa_family); 

	switch(ifr.ifr_hwaddr.sa_family)
        {
	case ARPHRD_ETHER:
		iface->if_hwaddr_len = 48;
		iface->if_prefix_len = 64;
		iface->if_maxmtu = 1500;
		break;
#ifdef ARPHRD_FDDI
	case ARPHRD_FDDI:
		iface->if_hwaddr_len = 48;
		iface->if_prefix_len = 64;
		iface->if_maxmtu = 4352;
		break;
#endif /* ARPHDR_FDDI */
#ifdef ARPHRD_ARCNET
	case ARPHRD_ARCNET:
		iface->if_hwaddr_len = 8;
		iface->if_prefix_len = -1;
		iface->if_maxmtu = -1;
		break;
#endif /* ARPHDR_ARCNET */
	default:
		iface->if_hwaddr_len = -1;
		iface->if_prefix_len = -1;
		iface->if_maxmtu = -1;
		break;
	}

	dlog(LOG_DEBUG, 3, "link layer token length for %s is %d", iface->Name,
		iface->if_hwaddr_len);

	dlog(LOG_DEBUG, 3, "prefix length for %s is %d", iface->Name,
		iface->if_prefix_len);

	if (iface->if_hwaddr_len != -1)
		memcpy(iface->if_hwaddr, ifr.ifr_hwaddr.sa_data, (iface->if_hwaddr_len + 7) >> 3);

	prefix = iface->AdvPrefixList;
	while (prefix)
	{
		if ((iface->if_prefix_len != -1) &&
		   (iface->if_prefix_len != prefix->PrefixLen))
		{
			log(LOG_WARNING, "prefix length should be %d for %s",
				iface->if_prefix_len, iface->Name);
 		}
 			
 		prefix = prefix->next;
	}
                
	return (0);
}

/*
 * this function extracts the link local address and interface index
 * from PATH_PROC_NET_IF_INET6
 */
int setup_linklocal_addr(int sock, struct Interface *iface)
{
	FILE *fp;
	char str_addr[40];
	unsigned int plen, scope, dad_status, if_idx;
	char devname[IFNAMSIZ];

	if ((fp = fopen(PATH_PROC_NET_IF_INET6, "r")) == NULL)
	{
		log(LOG_ERR, "can't open %s: %s", PATH_PROC_NET_IF_INET6,
			strerror(errno));
		return (-1);	
	}
	
	while (fscanf(fp, "%32s %02x %02x %02x %02x %15s\n",
		      str_addr, &if_idx, &plen, &scope, &dad_status,
		      devname) != EOF)
	{
		if (scope == IPV6_ADDR_LINKLOCAL &&
		    strcmp(devname, iface->Name) == 0)
		{
			struct in6_addr addr;
			unsigned int ap;
			int i;
			
			for (i=0; i<16; i++)
			{
				sscanf(str_addr + i * 2, "%02x", &ap);
				addr.s6_addr[i] = (unsigned char)ap;
			}
			memcpy(&iface->if_addr, &addr, sizeof(addr));

			iface->if_index = if_idx;
			fclose(fp);
			return 0;
		}
	}

	log(LOG_ERR, "no linklocal address configured for %s", iface->Name);
	fclose(fp);
	return (-1);
}

int setup_allrouters_membership(int sock, struct Interface *iface)
{
	struct ipv6_mreq mreq;                  
	
	memset(&mreq, 0, sizeof(mreq));                  
	mreq.ipv6mr_interface = iface->if_index;
	
	/* ipv6-allrouters: ff02::2 */
	mreq.ipv6mr_multiaddr.s6_addr32[0] = htonl(0xFF020000);                                          
	mreq.ipv6mr_multiaddr.s6_addr32[3] = htonl(0x2);     

	if (setsockopt(sock, SOL_IPV6, IPV6_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
	{
		log(LOG_ERR, "can't join ipv6-allrouters on %s", iface->Name);
		return (-1);
	}

	return (0);
}

int check_allrouters_membership(int sock, struct Interface *iface)
{
	#define ALL_ROUTERS_MCAST "ff020000000000000000000000000002"
	
	FILE *fp;
	unsigned int if_idx, allrouters_ok=0;
	char addr[32+1];
	int ret=0;

	if ((fp = fopen(PATH_PROC_NET_IGMP6, "r")) == NULL)
	{
		log(LOG_ERR, "can't open %s: %s", PATH_PROC_NET_IGMP6,
			strerror(errno));
		return (-1);	
	}
	
	while ( (ret=fscanf(fp, "%4u %*s %32[0-9A-Fa-f] %*x %*x %*x\n", &if_idx, addr)) != EOF) {
		if (ret == 2) {
			if (iface->if_index == if_idx) {
				if (strncmp(addr, ALL_ROUTERS_MCAST, sizeof(addr)) == 0)
					allrouters_ok = 1;
			}
		}
	}

	fclose(fp);

	if (!allrouters_ok) {
		log(LOG_WARNING, "resetting ipv6-allrouters membership on %s", iface->Name);
		setup_allrouters_membership(sock, iface);
	}	

	return(0);
}		

int
get_v4addr(const char *ifn, unsigned int *dst)
{
	struct ifreq	ifr;
	struct sockaddr_in *addr;
	int fd;

	if( ( fd = socket(AF_INET,SOCK_DGRAM,0) ) < 0 )
	{
		log(LOG_ERR, "create socket for IPv4 ioctl failed for %s: %s",
			ifn, strerror(errno));
		return (-1);
	}
	
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifn, IFNAMSIZ-1);
	ifr.ifr_name[IFNAMSIZ-1] = '\0';
	ifr.ifr_addr.sa_family = AF_INET;
	
	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
	{
		log(LOG_ERR, "ioctl(SIOCGIFADDR) failed for %s: %s",
			ifn, strerror(errno));
		close( fd );
		return (-1);
	}

	addr = (struct sockaddr_in *)(&ifr.ifr_addr);

	dlog(LOG_DEBUG, 3, "IPv4 address for %s is %s", ifn,
		inet_ntoa( addr->sin_addr ) ); 

	*dst = addr->sin_addr.s_addr;

	close( fd );

	return 0;
}
