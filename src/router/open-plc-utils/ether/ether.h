/*====================================================================*
 *
 *   ether.h - Ethernet definitions and declarations;
 *
 *   include or exclude various ethernet related definitions based
 *   platform and environment;
 *
 *.  Qualcomm Atheros HomePlug AV Powerline Toolkit
 *:  Copyright (c) 2009-2013 by Qualcomm Atheros Inc. ALL RIGHTS RESERVED;
 *;  For demonstration and evaluation only; Not for production use.
 *
 *--------------------------------------------------------------------*/

#ifndef ETHER_HEADER
#define ETHER_HEADER

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdint.h>
#if defined (__linux__)
#       include <net/if.h>
#       include <net/ethernet.h>
#       include <arpa/inet.h>
#elif defined (__APPLE__)
#       include <sys/types.h>
#       include <sys/socket.h>
#       include <net/if.h>
#       include <net/ethernet.h>
#       include <arpa/inet.h>
#       include <net/bpf.h>
#elif defined (__OpenBSD__)
#       include <sys/ioctl.h>
#       include <sys/types.h>
#       include <sys/socket.h>
#       include <net/if.h>
#       include <netinet/in.h>
#       include <netinet/if_ether.h>
#       include <net/bpf.h>
#       include <fcntl.h>
#elif defined (WIN32)
#       if defined (WINPCAP)
#               include <pcap.h>
#               include <Packet32.h>
#       endif
#       include <net/ethernet.h>
#       include <net/if.h>
#elif defined (__CYGWIN__)
#	error "Cygwin in unsupported!"
#else
#	error "Unknown environment!"
#endif

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/types.h"

/*====================================================================*
 *   variables;
 *--------------------------------------------------------------------*/

typedef struct nic

{
	unsigned ifindex;
	byte ethernet [ETHER_ADDR_LEN];
	byte internet [4];
	char ifname [IF_NAMESIZE];
	char ifdesc [255];
}

NIC;

/*====================================================================*
 *   ethertypes;
 *--------------------------------------------------------------------*/

#define ETH_P_HP10 0x887B
#define ETH_P_HCP  0x88B7
#define ETH_P_LLDP 0x88CC
#define ETH_P_HPAV 0x88E1
#define ETH_P_1905 0x893A

/*====================================================================*
 *   basic 802.2 Ethernet frame structure;
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct ethernet_frame

{
	byte frame_dhost [ETHER_ADDR_LEN];
	byte frame_shost [ETHER_ADDR_LEN];
	uint16_t frame_type;
	byte frame_data [ETHERMTU];
}

FRAME;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   functions;
 *--------------------------------------------------------------------*/

char * getifname (signed number);
signed gethwaddr (void * memory, char const * device);
signed anynic (char buffer [], unsigned length);
unsigned hostnics (struct nic list [], unsigned size);

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif

