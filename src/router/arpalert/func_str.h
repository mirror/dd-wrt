/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: func_str.h 690 2008-03-31 18:36:43Z  $
 *
 */

#ifndef __FUNC_STR_H__
#define __FUNC_STR_H__

#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#ifdef __FreeBSD__
#   define ETHER_ADDR_OCTET octet
#else
#   define ETHER_ADDR_OCTET ether_addr_octet
#endif

// translate binary data mac to string data mac
// void data_tomac(struct ether_addr, char *);
#define MAC_SIZE 18
#define MAC_TO_STR(a, b) \
	snprintf((b), MAC_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x", \
	        (a).ETHER_ADDR_OCTET[0], \
	        (a).ETHER_ADDR_OCTET[1], \
	        (a).ETHER_ADDR_OCTET[2], \
	        (a).ETHER_ADDR_OCTET[3], \
	        (a).ETHER_ADDR_OCTET[4], \
	        (a).ETHER_ADDR_OCTET[5])

// convert hex string to int
//   @param: hexa string terminated by \0
//   @return: hexa int conv
//            -1: error
int strhex_to_int(char *hex);

// translate string data mac to binary data mac
// return -1 if error
int str_to_mac(char *, struct ether_addr *);

#endif
