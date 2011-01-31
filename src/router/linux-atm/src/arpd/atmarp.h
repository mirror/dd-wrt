/* atmarp.h - ATM ARP protocol definitions */
 
/* Written 1995-1998 by Werner Almesberger, EPFL LRC/ICA */
 

#ifndef _ATMARP_H
#define _ATMARP_H

#include <stdint.h>


/* RFC 1577 ATM ARP header */

struct atmarphdr {
	uint16_t	ar_hrd;	/* Hardware type */
	uint16_t	ar_pro;	/* Protocol type */
	uint8_t		ar_shtl;/* Type & length of source ATM number (q) */
	uint8_t		ar_sstl;/* Type & length of source ATM subaddress (r) */
	uint16_t	ar_op;	/* Operation code (request, reply, or NAK) */
	uint8_t		ar_spln;/* Length of source protocol address (s) */
	uint8_t		ar_thtl;/* Type & length of target ATM number (x) */
	uint8_t		ar_tstl;/* Type & length of target ATM subaddress (y) */
	uint8_t		ar_tpln;/* Length of target protocol address (z) */
	/* ar_sha, at_ssa, ar_spa, ar_tha, ar_tsa, ar_tpa */
	unsigned char data[1];
};

#define	TL_LEN	0x3f	/* ATMARP Type/Length field structure */
#define	TL_E164	0x40


#define MAX_ATMARP_SIZE (sizeof(struct atmarphdr)-1+2*(ATM_E164_LEN+ \
			ATM_ESA_LEN+4))

#endif
