/*====================================================================*
 *   
 *   Copyright (c) 2011 by Qualcomm Atheros.
 *   
 *   Permission to use, copy, modify, and/or distribute this software 
 *   for any purpose with or without fee is hereby granted, provided 
 *   that the above copyright notice and this permission notice appear 
 *   in all copies.
 *   
 *   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL 
 *   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED 
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL  
 *   THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR 
 *   CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 *   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, 
 *   NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 *   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *   
 *--------------------------------------------------------------------*/

/*====================================================================*
 *
 *   ethernet.h - substitute net/ethernet.h for systems without one;
 *
 *.  Intellon INT6000 Linux Toolkit for HomePlug AV;
 *:  Published 2006-2009 by Intellon Corp. ALL RIGHTS RESERVED;
 *;  For demonstration; Not for production use;
 *
 *--------------------------------------------------------------------*/

#ifndef ETHERNET_HEADER
#define ETHERNET_HEADER

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdint.h>

/*====================================================================*
 *   Ethernet frame lengths;
 *--------------------------------------------------------------------*/

#define	ETHER_ADDR_LEN		6
#define	ETHER_TYPE_LEN		2
#define	ETHER_CRC_LEN		4
#define	ETHER_HDR_LEN		(ETHER_ADDR_LEN + ETHER_ADDR_LEN + ETHER_TYPE_LEN)
#define	ETHER_MIN_LEN		64
#define	ETHER_MAX_LEN		1518
#define	ETHERMTU		1500
#define ETHERMIN		(ETHER_MIN_LEN - ETHER_HDR_LEN - ETHER_CRC_LEN)

/*====================================================================*
 *   Ethernet type/length field values;
 *--------------------------------------------------------------------*/

#define	ETHERTYPE_PUP		0x0200	
#define	ETHERTYPE_IP		0x0800	
#define ETHERTYPE_ARP		0x0806	
#define ETHERTYPE_REVARP	0x8035	
#define	ETHERTYPE_VLAN		0x8100	
#define ETHERTYPE_IPV6		0x86dd	
#define	ETHERTYPE_LOOPBACK	0x9000	
#define ETHERTYPE_HP10		0x887B  
#define ETHERTYPE_HPAV		0x88E1  

/*====================================================================*
 *  Ethernet frame structure;
 *--------------------------------------------------------------------*/

#pragma pack (push, 1)

struct ether_header 

{
	uint8_t ether_dhost [ETHER_ADDR_LEN];
	uint8_t ether_shost [ETHER_ADDR_LEN];
	uint16_t ether_type;
};

#pragma pack (pop)

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif 

