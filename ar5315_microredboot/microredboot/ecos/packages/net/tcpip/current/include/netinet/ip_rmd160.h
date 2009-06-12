//==========================================================================
//
//      include/netinet/rmd160.h
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


/*	$OpenBSD: ip_rmd160.h,v 1.3 1999/02/17 18:10:08 deraadt Exp $	*/

/*
 *      FILE:     rmd160.h
 *
 *      CONTENTS: Header file for a sample C-implementation of the
 *                RIPEMD-160 hash-function. 
 *      TARGET:   any computer with an ANSI C compiler
 *
 *      AUTHOR:   Antoon Bosselaers, ESAT-COSIC
 *      DATE:     1 March 1996
 *      VERSION:  1.0
 *
 *      Copyright (c) Katholieke Universiteit Leuven
 *      1996, All Rights Reserved
 *
 */

#ifndef  _NETINET_RMD160_H	/* make sure this file is read only once */
#define  _NETINET_RMD160_H

typedef struct {
	u_int32_t state[5];	/* state (ABCDE) */
	u_int32_t length[2];	/* number of bits */
	u_char    bbuffer[64];	/* overflow buffer */
	u_int32_t buflen;	/* number of chars in bbuffer */
} RMD160_CTX;

void RMD160Init __P((RMD160_CTX *context));
void RMD160Transform __P((u_int32_t state[5], const u_int32_t block[16]));
void RMD160Update __P((RMD160_CTX *context, const u_char *data, u_int nbytes));
void RMD160Final __P((u_char digest[20], RMD160_CTX *context));
char *RMD160End __P((RMD160_CTX *, char *));
char *RMD160File __P((char *, char *));
char *RMD160Data __P((const u_char *, size_t, char *));

#endif  /* _NETINET_RMD160_H */
