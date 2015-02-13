/*====================================================================*
 *
 *   Copyright (c) 2013 Qualcomm Atheros, Inc.
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or 
 *   without modification, are permitted (subject to the limitations 
 *   in the disclaimer below) provided that the following conditions 
 *   are met:
 *
 *   * Redistributions of source code must retain the above copyright 
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above 
 *     copyright notice, this list of conditions and the following 
 *     disclaimer in the documentation and/or other materials 
 *     provided with the distribution.
 *
 *   * Neither the name of Qualcomm Atheros nor the names of 
 *     its contributors may be used to endorse or promote products 
 *     derived from this software without specific prior written 
 *     permission.
 *
 *   NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE 
 *   GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
 *   COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 *   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 *   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER 
 *   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 *   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *
 *   unsigned hostnics (struct nic list [], unsigned size);
 *
 *   ether.h
 *
 *   encode an external memory region with a packed list of available
 *   nost network interfaces; return the number of interfaces found;
 *   each interface has an index, ethernet address, internet address,
 *   name and description;
 *
 *   this function is implemented for Linux and MacOSX;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef HOSTNICS_SOURCE
#define HOSTNICS_SOURCE

#if defined (__linux__)
#	include <net/if.h>
#	include <net/ethernet.h>
#	include <sys/ioctl.h>
#elif defined (__linux__)
#	include <net/if.h>
#	include <netpacket/packet.h>
#	include <ifaddrs.h>
#elif defined (__APPLE__) || defined (__OpenBSD__)
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <net/if.h>
#	include <net/if_dl.h>
#	include <net/if_types.h>
#	include <ifaddrs.h>
#elif defined (WIN32)
#error "Not implemented for Windows"
#else
#error "Unknown environment"
#endif

#include <unistd.h>
#include <memory.h>
#include <errno.h>

#include "../ether/ether.h"
#include "../tools/error.h"

unsigned hostnics (struct nic nics [], unsigned size)

{

#if defined (__linux__)

/*
 *	native method on Linux systems;
 */

	char buffer [1024];
	struct ifconf ifconf;
	struct ifreq * ifreq;
	unsigned next;
	signed fd;
	memset (nics, 0, size * sizeof (struct nic));
	ifconf.ifc_len = sizeof (buffer);
	ifconf.ifc_buf = buffer;
	if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		error (1, errno, "Can't open socket");
	}
	if (ioctl (fd, SIOCGIFCONF, &ifconf) < 0)
	{
		error (1, errno, "Can't read socket configuration");
	}
	ifreq = ifconf.ifc_req;
	next = ifconf.ifc_len / sizeof (struct ifreq);
	if (next > size)
	{
		next = size;
	}
	if (next < size)
	{
		size = next;
	}
	for (next = 0; next < size; next++)
	{
		struct nic * nic = &nics [next];
		struct sockaddr_in * sockaddr_in = (struct sockaddr_in *)(&ifreq->ifr_addr);
		memcpy (nic->ifname, ifreq->ifr_name, sizeof (nic->ifname));
		memcpy (nic->ifdesc, ifreq->ifr_name, sizeof (nic->ifdesc));
		memcpy (nic->internet, &sockaddr_in->sin_addr, sizeof (nic->internet));
		if (ioctl (fd, SIOCGIFHWADDR, ifreq) == -1)
		{
			error (0, errno, "Can't read hardware address: %s", ifreq->ifr_name);
		}
		memcpy (nic->ethernet, ifreq->ifr_hwaddr.sa_data, sizeof (nic->ethernet));
		if (ioctl (fd, SIOCGIFINDEX, ifreq) == -1)
		{
			error (0, errno, "Can't read interface index: %s", ifreq->ifr_name);
		}
		nic->ifindex = ifreq->ifr_ifindex;
		ifreq++;
	}
	close (fd);

#elif defined (__linux__) || defined (__APPLE__) || defined (__OpenBSD__)

/*
 *	generic (POSIX) method for unix-like systems;
 */

	struct ifaddrs * ifaddrs;
	memset (nics, 0, size * sizeof (struct nic));
	unsigned next = 0;
	if (getifaddrs (&ifaddrs) != -1)
	{
		struct ifaddrs * ifaddr;
		for (ifaddr = ifaddrs; ifaddr && size; ifaddr = ifaddr->ifa_next)
		{
			struct nic * nic = &nics [next];
			struct nic * tmp = nics;
			if (!ifaddr->ifa_addr)
			{
				continue;
			}
			nic->ifindex = if_nametoindex (ifaddr->ifa_name);
			for (tmp = nics; tmp->ifindex != nic->ifindex; tmp++);
			if (tmp == nic)
			{
				next++;
				size--;
			}
			else
			{
				nic = tmp;
			}
			memcpy (nic->ifname, ifaddr->ifa_name, sizeof (nic->ifname));
			memcpy (nic->ifdesc, ifaddr->ifa_name, sizeof (nic->ifdesc));
			if (ifaddr->ifa_addr->sa_family == AF_INET)
			{
				struct sockaddr_in * sockaddr_in = (struct sockaddr_in *) (ifaddr->ifa_addr);
				struct in_addr * in_addr = (struct in_addr *)(&sockaddr_in->sin_addr);
				memcpy (nic->internet, &in_addr->s_addr, sizeof (nic->internet));
			}

#if defined (__linux__)
#define LLADDR(socket) &(socket)->sll_addr

			if (ifaddr->ifa_addr->sa_family == AF_PACKET)
			{
				struct sockaddr_ll * sockaddr_ll = (struct sockaddr_ll *) (ifaddr->ifa_addr);
				memcpy (nic->ethernet, LLADDR (sockaddr_ll), sizeof (nic->ethernet));
			}

#elif defined (__APPLE__) || defined (__OpenBSD__)

			if (ifaddr->ifa_addr->sa_family == AF_LINK)
			{
				struct sockaddr_dl * sockaddr_dl = (struct sockaddr_dl *) (ifaddr->ifa_addr);
				if (sockaddr_dl->sdl_type == IFT_ETHER)
				{

#if 1

					memcpy (nic->ethernet, LLADDR (sockaddr_dl), sizeof (nic->ethernet));

#else

					memcpy (nic->ethernet, LLADDR (sockaddr_dl), sockaddr_dl->sdl_alen);

#endif

				}
			}

#else
#error "Abandon all hope!"
#endif

		}
		freeifaddrs (ifaddrs);
	}

#else
#error "Unknown environment"
#endif

	return (next);
}


#endif

