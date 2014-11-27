/*
 * Copyright (c) 2005-2010 Thierry FOURNIER
 * $Id: func_str.c 690 2008-03-31 18:36:43Z  $
 *
 */

#include "config.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#if (__NetBSD__ || __FreeBSD__ || __OpenBSD__)
#   include <net/if_dl.h>
#endif


#ifdef __FreeBSD__
#   define ETHER_ADDR_OCTET octet
#else
#   define ETHER_ADDR_OCTET ether_addr_octet
#endif

// conversion hexa -> bin
const u_char hex_conv[103] = {
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /*  9 */
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 19 */
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 29 */
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 39 */
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  1, /* 49 */
	 2,  3,  4,  5,  6,  7,  8,  9,  0,  0, /* 59 */
	 0,  0,  0,  0,  0, 10, 11, 12, 13, 14, /* 69 */
	15,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 79 */
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, /* 89 */
	 0,  0,  0,  0,  0,  0,  0, 10, 11, 12, /* 99 */
	13, 14, 15                              /* 102 */
};

// convert hex string to int
int strhex_to_int(char *hex){
	int ret = 0;
	
	while(*hex != 0){
		if((*hex < 'a' || *hex > 'f' ) &&
		   (*hex < 'A' || *hex > 'F' ) &&
		   (*hex < '0' || *hex > '9')){
			return -1;
		}
		ret *= 16;
		ret += hex_conv[(int)*hex];
		hex++;
	}

	return ret;
}

// translate string mac to binary mac
int str_to_mac(char *macaddr, struct ether_addr *to_mac){
	int i;
	
	// format verification
	i = 0;
	while(i <= 19) {
		switch(i){
			case  0: case  1:
			case  3: case  4:
			case  6: case  7:
			case  9: case 10:
			case 12: case 13:
			case 15: case 16:
				if((macaddr[i] < 'a' || macaddr[i] > 'f' ) &&
				   (macaddr[i] < 'A' || macaddr[i] > 'F' ) &&
				   (macaddr[i] < '0' || macaddr[i] > '9')){
					return -1;
				}
				break;

			case  2:
			case  5:
			case  8:
			case 11:
			case 14:
				if(macaddr[i] != ':'){
					return -1;
				}
				break;
		}
		i++;
	}

	to_mac->ETHER_ADDR_OCTET[0] =  hex_conv[(u_char)macaddr[1]];
	to_mac->ETHER_ADDR_OCTET[0] += hex_conv[(u_char)macaddr[0]] * 16;
	to_mac->ETHER_ADDR_OCTET[1] =  hex_conv[(u_char)macaddr[4]];
	to_mac->ETHER_ADDR_OCTET[1] += hex_conv[(u_char)macaddr[3]] * 16;
	to_mac->ETHER_ADDR_OCTET[2] =  hex_conv[(u_char)macaddr[7]];
	to_mac->ETHER_ADDR_OCTET[2] += hex_conv[(u_char)macaddr[6]] * 16;
	to_mac->ETHER_ADDR_OCTET[3] =  hex_conv[(u_char)macaddr[10]];
	to_mac->ETHER_ADDR_OCTET[3] += hex_conv[(u_char)macaddr[9]] * 16;
	to_mac->ETHER_ADDR_OCTET[4] =  hex_conv[(u_char)macaddr[13]];
	to_mac->ETHER_ADDR_OCTET[4] += hex_conv[(u_char)macaddr[12]] * 16;
	to_mac->ETHER_ADDR_OCTET[5] =  hex_conv[(u_char)macaddr[16]];
	to_mac->ETHER_ADDR_OCTET[5] += hex_conv[(u_char)macaddr[15]] * 16;
	return 0;
}

