/*
 * useful_functions.c, January 2004
 *
 * Random collection of functions that can be used by extensions.
 *
 * Author: Bart De Schuymer
 *
 *  This code is stongly inspired on the iptables code which is
 *  Copyright (C) 1999 Paul `Rusty' Russell & Michael J. Neuling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "include/ebtables_u.h"
#include "include/ethernetdb.h"
#include <stdio.h>
#include <netinet/ether.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

const unsigned char mac_type_unicast[ETH_ALEN] =   {0,0,0,0,0,0};
const unsigned char msk_type_unicast[ETH_ALEN] =   {1,0,0,0,0,0};
const unsigned char mac_type_multicast[ETH_ALEN] = {1,0,0,0,0,0};
const unsigned char msk_type_multicast[ETH_ALEN] = {1,0,0,0,0,0};
const unsigned char mac_type_broadcast[ETH_ALEN] = {255,255,255,255,255,255};
const unsigned char msk_type_broadcast[ETH_ALEN] = {255,255,255,255,255,255};
const unsigned char mac_type_bridge_group[ETH_ALEN] = {0x01,0x80,0xc2,0,0,0};
const unsigned char msk_type_bridge_group[ETH_ALEN] = {255,255,255,255,255,255};

/* 0: default, print only 2 digits if necessary
 * 2: always print 2 digits, a printed mac address
 * then always has the same length */
int ebt_printstyle_mac;

void ebt_print_mac(const unsigned char *mac)
{
	if (ebt_printstyle_mac == 2) {
		int j;
		for (j = 0; j < ETH_ALEN; j++)
			printf("%02x%s", mac[j],
				(j==ETH_ALEN-1) ? "" : ":");
	} else
		printf("%s", ether_ntoa((struct ether_addr *) mac));
}

void ebt_print_mac_and_mask(const unsigned char *mac, const unsigned char *mask)
{
	char hlpmsk[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	if (!memcmp(mac, mac_type_unicast, 6) &&
	    !memcmp(mask, msk_type_unicast, 6))
		printf("Unicast");
	else if (!memcmp(mac, mac_type_multicast, 6) &&
	         !memcmp(mask, msk_type_multicast, 6))
		printf("Multicast");
	else if (!memcmp(mac, mac_type_broadcast, 6) &&
	         !memcmp(mask, msk_type_broadcast, 6))
		printf("Broadcast");
	else if (!memcmp(mac, mac_type_bridge_group, 6) &&
	         !memcmp(mask, msk_type_bridge_group, 6))
		printf("BGA");
	else {
		ebt_print_mac(mac);
		if (memcmp(mask, hlpmsk, 6)) {
			printf("/");
			ebt_print_mac(mask);
		}
	}
}

/* Checks the type for validity and calls getethertypebynumber(). */
struct ethertypeent *parseethertypebynumber(int type)
{
	if (type < 1536)
		ebt_print_error("Ethernet protocols have values >= 0x0600");
	if (type > 0xffff)
		ebt_print_error("Ethernet protocols have values <= 0xffff");
	return getethertypebynumber(type);
}

/* Put the mac address into 6 (ETH_ALEN) bytes returns 0 on success. */
int ebt_get_mac_and_mask(const char *from, unsigned char *to,
  unsigned char *mask)
{
	char *p;
	int i;
	struct ether_addr *addr;

	if (strcasecmp(from, "Unicast") == 0) {
		memcpy(to, mac_type_unicast, ETH_ALEN);
		memcpy(mask, msk_type_unicast, ETH_ALEN);
		return 0;
	}
	if (strcasecmp(from, "Multicast") == 0) {
		memcpy(to, mac_type_multicast, ETH_ALEN);
		memcpy(mask, msk_type_multicast, ETH_ALEN);
		return 0;
	}
	if (strcasecmp(from, "Broadcast") == 0) {
		memcpy(to, mac_type_broadcast, ETH_ALEN);
		memcpy(mask, msk_type_broadcast, ETH_ALEN);
		return 0;
	}
	if (strcasecmp(from, "BGA") == 0) {
		memcpy(to, mac_type_bridge_group, ETH_ALEN);
		memcpy(mask, msk_type_bridge_group, ETH_ALEN);
		return 0;
	}
	if ( (p = strrchr(from, '/')) != NULL) {
		*p = '\0';
		if (!(addr = ether_aton(p + 1)))
			return -1;
		memcpy(mask, addr, ETH_ALEN);
	} else
		memset(mask, 0xff, ETH_ALEN);
	if (!(addr = ether_aton(from)))
		return -1;
	memcpy(to, addr, ETH_ALEN);
	for (i = 0; i < ETH_ALEN; i++)
		to[i] &= mask[i];
	return 0;
}

/* 0: default
 * 1: the inverse '!' of the option has already been specified */
int ebt_invert = 0;

/*
 * Check if the inverse of the option is specified. This is used
 * in the parse functions of the extensions and ebtables.c
 */
int _ebt_check_inverse(const char option[], int argc, char **argv)
{
	if (!option)
		return ebt_invert;
	if (strcmp(option, "!") == 0) {
		if (ebt_invert == 1)
			ebt_print_error("Double use of '!' not allowed");
		if (optind >= argc)
			optarg = NULL;
		else
			optarg = argv[optind];
		optind++;
		ebt_invert = 1;
		return 1;
	}
	return ebt_invert;
}

/* Make sure the same option wasn't specified twice. This is used
 * in the parse functions of the extensions and ebtables.c */
void ebt_check_option(unsigned int *flags, unsigned int mask)
{
	if (*flags & mask)
		ebt_print_error("Multiple use of same option not allowed");
	*flags |= mask;
}

/* Put the ip string into 4 bytes. */
static int undot_ip(char *ip, unsigned char *ip2)
{
	char *p, *q, *end;
	long int onebyte;
	int i;
	char buf[20];

	strncpy(buf, ip, sizeof(buf) - 1);

	p = buf;
	for (i = 0; i < 3; i++) {
		if ((q = strchr(p, '.')) == NULL)
			return -1;
		*q = '\0';
		onebyte = strtol(p, &end, 10);
		if (*end != '\0' || onebyte > 255 || onebyte < 0)   
			return -1;
		ip2[i] = (unsigned char)onebyte;
		p = q + 1;
	}

	onebyte = strtol(p, &end, 10);
	if (*end != '\0' || onebyte > 255 || onebyte < 0)
		return -1;
	ip2[3] = (unsigned char)onebyte;

	return 0;
}

/* Put the mask into 4 bytes. */
static int ip_mask(char *mask, unsigned char *mask2)
{
	char *end;
	long int bits;
	uint32_t mask22;

	if (undot_ip(mask, mask2)) {
		/* not the /a.b.c.e format, maybe the /x format */
		bits = strtol(mask, &end, 10);
		if (*end != '\0' || bits > 32 || bits < 0)
			return -1;
		if (bits != 0) {
			mask22 = htonl(0xFFFFFFFF << (32 - bits));
			memcpy(mask2, &mask22, 4);
		} else {
			mask22 = 0xFFFFFFFF;
			memcpy(mask2, &mask22, 4);
		}
	}
	return 0;
}

/* Set the ip mask and ip address. Callers should check ebt_errormsg[0].
 * The string pointed to by address can be altered. */
void ebt_parse_ip_address(char *address, uint32_t *addr, uint32_t *msk)
{
	char *p;

	/* first the mask */
	if ((p = strrchr(address, '/')) != NULL) {
		*p = '\0';
		if (ip_mask(p + 1, (unsigned char *)msk)) {
			ebt_print_error("Problem with the IP mask '%s'", p + 1);
			return;
		}
	} else
		*msk = 0xFFFFFFFF;

	if (undot_ip(address, (unsigned char *)addr)) {
		ebt_print_error("Problem with the IP address '%s'", address);
		return;
	}
	*addr = *addr & *msk;
}

/* Transform the ip mask into a string ready for output. */
char *ebt_mask_to_dotted(uint32_t mask)
{
	int i;
	static char buf[20];
	uint32_t maskaddr, bits;

	maskaddr = ntohl(mask);

	/* don't print /32 */
	if (mask == 0xFFFFFFFFL) {
		*buf = '\0';
		return buf;
	}

	i = 32;
	bits = 0xFFFFFFFEL; /* Case 0xFFFFFFFF has just been dealt with */
	while (--i >= 0 && maskaddr != bits)
		bits <<= 1;

	if (i > 0)
		sprintf(buf, "/%d", i);
	else if (!i)
		*buf = '\0';
	else
		/* Mask was not a decent combination of 1's and 0's */
		sprintf(buf, "/%d.%d.%d.%d", ((unsigned char *)&mask)[0], 
		   ((unsigned char *)&mask)[1], ((unsigned char *)&mask)[2],
		   ((unsigned char *)&mask)[3]);

	return buf;
}
