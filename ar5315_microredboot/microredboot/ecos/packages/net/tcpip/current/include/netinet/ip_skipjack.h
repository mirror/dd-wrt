//==========================================================================
//
//      include/netinet/ip_skipjack.h
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


/* 
 * Further optimized test implementation of SKIPJACK algorithm 
 * Mark Tillotson <markt@chaos.org.uk>, 25 June 98
 * Optimizations suit RISC (lots of registers) machine best.
 *
 * based on unoptimized implementation of
 * Panu Rissanen <bande@lut.fi> 960624
 *
 * SKIPJACK and KEA Algorithm Specifications 
 * Version 2.0 
 * 29 May 1998
*/

#ifndef _NETINET_IP_SKIPJACK_H_
#define _NETINET_IP_SKIPJACK_H_

extern void skipjack_forwards __P((u_int8_t *plain, u_int8_t *cipher, u_int8_t **key));
extern void skipjack_backwards __P((u_int8_t *cipher, u_int8_t *plain, u_int8_t **key));
extern void subkey_table_gen __P((u_int8_t *key, u_int8_t **key_tables));

#endif // _NETINET_IP_SKIPJACK_H_
