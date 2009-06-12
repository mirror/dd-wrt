//==========================================================================
//
//      include/netinet/ip_cast.h
//
//      
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================


/*      $OpenBSD: ip_cast.h,v 1.3 1999/02/17 18:09:55 deraadt Exp $       */

/*
 *	CAST-128 in C
 *	Written by Steve Reid <sreid@sea-to-sky.net>
 *	100% Public Domain - no warranty
 *	Released 1997.10.11
 */

#ifndef _NETINET_IP_CAST_H_
#define _NETINET_IP_CAST_H_

typedef struct {
	u_int32_t	xkey[32];	/* Key, after expansion */
	int		rounds;		/* Number of rounds to use, 12 or 16 */
} cast_key;

void cast_setkey __P((cast_key * key, u_int8_t * rawkey, int keybytes));
void cast_encrypt __P((cast_key * key, u_int8_t * inblock, u_int8_t * outblock));
void cast_decrypt __P((cast_key * key, u_int8_t * inblock, u_int8_t * outblock));

#endif /* ifndef _NETINET_IP_CAST_H_ */
