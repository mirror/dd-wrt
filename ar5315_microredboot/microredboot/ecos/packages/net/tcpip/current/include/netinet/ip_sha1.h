//==========================================================================
//
//      include/netinet/sha1.h
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


/*	$OpenBSD: ip_sha1.h,v 1.5 1999/02/17 18:10:24 deraadt Exp $	*/

/*
 * SHA-1 in C
 * By Steve Reid <steve@edmweb.com>
 * 100% Public Domain
 */

#ifndef _NETINET_SHA1_H_
#define _NETINET_SHA1_H_

typedef struct {
	u_int32_t	state[5];
	u_int32_t	count[2];  
	unsigned char	buffer[64];
} SHA1_CTX;
  
void SHA1Transform __P((u_int32_t state[5], unsigned char buffer[64]));
void SHA1Init __P((SHA1_CTX* context));
void SHA1Update __P((SHA1_CTX* context, unsigned char* data, unsigned int len));
void SHA1Final __P((unsigned char digest[20], SHA1_CTX* context));

#endif /* _NETINET_SHA1_H_ */
