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
 *   int gethwaddr (void volatile * memory, char const * device);
 *
 *   ether.h
 *
 *   encode memory with the hardware address of a named host Ethernet
 *   interface;
 *
 *   there are two ways to obtain the hardware address on Linux; we
 *   use the first because some systems do not support getifaddrs()
 *   and some implementations are inconsistent;
 *
 *
 *   Contributor(s):
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef GETHWADDR_SOURCE
#define GETHWADDR_SOURCE

#include <stdint.h>
#include <unistd.h>
#include <memory.h>

#include "../tools/error.h"
#include "../ether/ether.h"

#ifndef OID_802_3_CURRENT_ADDRESS
#define OID_802_3_CURRENT_ADDRESS 0x01010102
#endif

int gethwaddr (void * memory, char const * device)

{

#if defined (__linux__)

#	include <ifaddrs.h>
#	include <netpacket/packet.h>
#	include <sys/ioctl.h>

	struct ifreq ifreq;
	int fd;
	if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		error (1, errno, "%s: %s", __func__, device);
	}
	memcpy (ifreq.ifr_name, device, sizeof (ifreq.ifr_name));
	if (ioctl (fd, SIOCGIFHWADDR, &ifreq) == -1)
	{
		close (fd);
		return (-1);
	}
	memcpy (memory, ifreq.ifr_ifru.ifru_hwaddr.sa_data, ETHER_ADDR_LEN);
	close (fd);

#elif defined (__linux__)

#include <ifaddrs.h>
#include <net/if_types.h>

	struct ifaddrs *ifaddrs;
	struct ifaddrs *ifaddr;
	if (getifaddrs (&ifaddrs) == -1)
	{
		error (1, errno, "No interfaces available");
	}
	for (ifaddr = ifaddrs; ifaddr; ifaddr = ifaddr->ifa_next)
	{
		if (strcmp (device, ifaddr->ifa_name))
		{
			continue;
		}
		if (!ifaddr->ifa_addr)
		{
			continue;
		}
		if (ifaddr->ifa_addr->sa_family == AF_PACKET)
		{
			struct sockaddr_ll * sockaddr = (struct sockaddr_ll *) (ifaddr->ifa_addr);
			memcpy (memory, sockaddr->sll_addr, ETHER_ADDR_LEN);
			break;
		}
	}
	freeifaddrs (ifaddrs);

#elif defined (__APPLE__)

#include <ifaddrs.h>
#include <net/if_dl.h>

	struct ifaddrs *ifaddrs;
	struct ifaddrs *ifaddr;
	if (getifaddrs (&ifaddrs) == -1)
	{
		error (1, errno, "No interfaces available");
	}
	for (ifaddr = ifaddrs; ifaddr; ifaddr = ifaddr->ifa_next)
	{
		if (strcmp (device, ifaddr->ifa_name))
		{
			continue;
		}
		if (!ifaddr->ifa_addr)
		{
			continue;
		}
		if (ifaddr->ifa_addr->sa_family == AF_LINK)
		{
			struct sockaddr_dl * sockaddr = (struct sockaddr_dl *) (ifaddr->ifa_addr);
			memcpy (memory, LLADDR (sockaddr), ETHER_ADDR_LEN);
			break;
		}
	}
	freeifaddrs (ifaddrs);

#elif defined (__OpenBSD__)

#include <ifaddrs.h>
#include <net/if_dl.h>

	struct ifaddrs *ifaddrs;
	struct ifaddrs *ifaddr;
	if (getifaddrs (&ifaddrs) == -1)
	{
		error (1, errno, "No interfaces available");
	}
	for (ifaddr = ifaddrs; ifaddr; ifaddr = ifaddr->ifa_next)
	{
		if (strcmp (device, ifaddr->ifa_name))
		{
			continue;
		}
		if (!ifaddr->ifa_addr)
		{
			continue;
		}
		if (ifaddr->ifa_addr->sa_family == AF_LINK)
		{
			struct sockaddr_dl * sockaddr = (struct sockaddr_dl *) (ifaddr->ifa_addr);
			memcpy (memory, LLADDR (sockaddr), ETHER_ADDR_LEN);
			break;
		}
	}
	freeifaddrs (ifaddrs);

#elif defined (WINPCAP) || defined (LIBPCAP)

	LPADAPTER adapter = PacketOpenAdapter ((PCHAR)(device));
	PPACKET_OID_DATA data = (PPACKET_OID_DATA)(malloc (ETHER_ADDR_LEN + sizeof (PACKET_OID_DATA)));
	if (!data)
	{
		error (1, errno, "Can't allocate packet: %s", device);
	}
	data->Oid = OID_802_3_CURRENT_ADDRESS;
	data->Length = ETHER_ADDR_LEN;
	if ((adapter == 0) || (adapter->hFile == INVALID_HANDLE_VALUE))
	{
		error (1, errno, "Can't access interface: %s", device);
	}
	if (!PacketRequest (adapter, FALSE, data))
	{
		memset (memory, 0, ETHER_ADDR_LEN);
		PacketCloseAdapter (adapter);
		free (data);
		return (-1);
	}
	memcpy (memory, data->Data, data->Length);
	PacketCloseAdapter (adapter);
	free (data);

#else
#error "Unknown environment"
#endif

	return (0);
}


#endif

