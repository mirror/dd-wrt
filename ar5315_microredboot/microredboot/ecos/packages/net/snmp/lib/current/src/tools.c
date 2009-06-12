//==========================================================================
//
//      ./lib/current/src/tools.c
//
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//####UCDSNMPCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from the UCD-SNMP
// project,  <http://ucd-snmp.ucdavis.edu/>  from the University of
// California at Davis, which was originally based on the Carnegie Mellon
// University SNMP implementation.  Portions of this software are therefore
// covered by the appropriate copyright disclaimers included herein.
//
// The release used was version 4.1.2 of May 2000.  "ucd-snmp-4.1.2"
// -------------------------------------------
//
//####UCDSNMPCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt
// Contributors: hmt
// Date:         2000-05-30
// Purpose:      Port of UCD-SNMP distribution to eCos.
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/********************************************************************
       Copyright 1989, 1991, 1992 by Carnegie Mellon University

			  Derivative Work -
Copyright 1996, 1998, 1999, 2000 The Regents of the University of California

			 All Rights Reserved

Permission to use, copy, modify and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU and The Regents of
the University of California not be used in advertising or publicity
pertaining to distribution of the software without specific written
permission.

CMU AND THE REGENTS OF THE UNIVERSITY OF CALIFORNIA DISCLAIM ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL CMU OR
THE REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
FROM THE LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*********************************************************************/
/*
 * tools.c
 */

#include <config.h>

#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h> 
# else
#  include <time.h>
# endif
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include "asn1.h" 
#include "system.h"
#include "snmp_api.h"
#include "snmp_debug.h"
#include "snmp_debug.h"
#include "tools.h"
#include "mib.h"
#include "scapi.h" 


/*******************************************************************-o-******
 * free_zero
 *
 * Parameters:
 *	*buf	Pointer at bytes to free.
 *	size	Number of bytes in buf.
 */
void
free_zero(void *buf, size_t size)
{
	if (buf) {
		memset(buf, 0, size);
		free(buf);
	}

}  /* end free_zero() */




/*******************************************************************-o-******
 * malloc_random
 *
 * Parameters:
 *	size	Number of bytes to malloc() and fill with random bytes.
 *      
 * Returns pointer to allocaed & set buffer on success, size contains
 * number of random bytes filled.
 *
 * buf is NULL and *size set to KMT error value upon failure.
 *
 */
u_char *
malloc_random(size_t *size)
{
	int	 rval	= SNMPERR_SUCCESS;
	u_char	*buf	= (u_char *)calloc (1, *size);

	if (buf) {
		rval = sc_random(buf, size);

		if (rval < 0) {
			free_zero(buf, *size);
			buf = NULL;
		} else {
			*size = rval;
		}
	}

	return buf;

}  /* end malloc_random() */




/*******************************************************************-o-******
 * memdup
 *
 * Parameters:
 *	to       Pointer to allocate and copy memory to.
 *      from     Pointer to copy memory from.
 *      size     Size of the data to be copied.
 *      
 * Returns
 *	SNMPERR_SUCCESS	On success.
 *      SNMPERR_GENERR	On failure.
 */
int
memdup(u_char **to, const u_char *from, size_t size)
{
  if (to == NULL)
    return SNMPERR_GENERR;
  if (from == NULL) {
    *to = NULL;
    return SNMPERR_SUCCESS;
  }
  if ((*to = (u_char *)malloc(size)) == NULL)
    return SNMPERR_GENERR;
  memcpy(*to, from, size);
  return SNMPERR_SUCCESS;

}  /* end memdup() */




/*******************************************************************-o-******
 * binary_to_hex
 *
 * Parameters:
 *	*input		Binary data.
 *	len		Length of binary data.
 *	**output	NULL terminated string equivalent in hex.
 *      
 * Returns:
 *	olen	Length of output string not including NULL terminator.
 *
 * FIX	Is there already one of these in the UCD SNMP codebase?
 *	The old one should be used, or this one should be moved to
 *	snmplib/snmp_api.c.
 */
u_int
binary_to_hex(const u_char *input, size_t len, char **output)
{
	u_int	olen	= (len * 2) + 1;
	char	*s	= (char *) calloc(1,olen),
		*op	= s;
	const u_char *ip	= input;


	while (ip-input < (int)len) {
		*op++ = VAL2HEX( (*ip >> 4) & 0xf );
		*op++ = VAL2HEX( *ip & 0xf );
		ip++;
	}
	*op = '\0';
	
	*output = s;
	return olen;

}  /* end binary_to_hex() */




/*******************************************************************-o-******
 * hex_to_binary2
 *
 * Parameters:
 *	*input		Printable data in base16.
 *	len		Length in bytes of data.
 *	**output	Binary data equivalent to input.
 *      
 * Returns:
 *	SNMPERR_GENERR	Failure.
 *	<len>		Otherwise, Length of allocated string.
 *
 *
 * Input of an odd length is right aligned.
 *
 * FIX	Another version of "hex-to-binary" which takes odd length input
 *	strings.  It also allocates the memory to hold the binary data.
 *	Should be integrated with the official hex_to_binary() function.
 */
int
hex_to_binary2(const u_char *input, size_t len, char **output)
{
	u_int	olen	= (len/2) + (len%2);
	char	*s	= (char *)calloc (1,olen),
		*op	= s;
	const u_char *ip	= input;


	*output = NULL;
	*op = 0;
	if (len%2) {
		if(!isxdigit(*ip)) goto hex_to_binary2_quit;
		*op++ = HEX2VAL( *ip );		ip++;
	}

	while (ip-input < (int)len) {
		if(!isxdigit(*ip)) goto hex_to_binary2_quit;
		*op = HEX2VAL( *ip ) << 4;	ip++;

		if(!isxdigit(*ip)) goto hex_to_binary2_quit;
		*op++ += HEX2VAL( *ip );	ip++;
	}

	*output = s;	
	return olen;

hex_to_binary2_quit:
	free_zero(s, olen);
	return -1;

}  /* end hex_to_binary2() */




/*******************************************************************-o-******
 * dump_chunk
 *
 * Parameters:
 *	*title	(May be NULL.)
 *	*buf
 *	 size
 */
void
dump_chunk(const char *debugtoken, const char *title, const u_char *buf, int size)
{
	u_int		printunit = 64;		/* XXX  Make global. */
	char		chunk[SNMP_MAXBUF],
			*s, *sp;

	if ( title && (*title != '\0') ) {
          DEBUGMSGTL((debugtoken, "%s\n", title));
	}


	memset(chunk, 0, SNMP_MAXBUF);
	size = binary_to_hex(buf, size, &s);
	sp   = s;

	while (size > 0)
	{
		if (size > (int)printunit) {
			strncpy(chunk, sp, printunit);	
			chunk[printunit] = '\0';
			DEBUGMSGTL((debugtoken, "\t%s\n", chunk));
		} else {
			DEBUGMSGTL((debugtoken, "\t%s\n", sp));
		}

		sp	+= printunit;
		size	-= printunit;
	}


	SNMP_FREE(s);

}  /* end dump_chunk() */




/*******************************************************************-o-******
 * dump_snmpEngineID
 *
 * Parameters:
 *	*estring
 *	*estring_len
 *      
 * Returns:
 *	Allocated memory pointing to a string of buflen char representing
 *	a printf'able form of the snmpEngineID.
 *
 *	-OR- NULL on error.
 *
 *
 * Translates the snmpEngineID TC into a printable string.  From RFC 2271,
 * Section 5 (pp. 36-37):
 *
 * First bit:	0	Bit string structured by means non-SNMPv3.
 *  		1	Structure described by SNMPv3 SnmpEngineID TC.
 *  
 * Bytes 1-4:		Enterprise ID.  (High bit of first byte is ignored.)
 *  
 * Byte 5:	0	(RESERVED by IANA.)
 *  		1	IPv4 address.		(   4 octets)
 *  		2	IPv6 address.		(  16 octets)
 *  		3	MAC address.		(   6 octets)
 *  		4	Locally defined text.	(0-27 octets)
 *  		5	Locally defined octets.	(0-27 octets)
 *  		6-127	(RESERVED for enterprise.)
 *  
 * Bytes 6-32:		(Determined by byte 5.)
 *  
 *
 * Non-printable characters are given in hex.  Text is given in quotes.
 * IP and MAC addresses are given in standard (UN*X) conventions.  Sections
 * are comma separated.
 *
 * esp, remaining_len and s trace the state of the constructed buffer.
 * s will be defined if there is something to return, and it will point
 * to the end of the constructed buffer.
 *
 *
 * ASSUME  "Text" means printable characters.
 *
 * XXX	Must the snmpEngineID always have a minimum length of 12?
 *	(Cf. part 2 of the TC definition.)
 * XXX	Does not enforce upper-bound of 32 bytes.
 * XXX	Need a switch to decide whether to use DNS name instead of a simple
 *	IP address.
 *
 * FIX	Use something other than sprint_hexstring which doesn't add 
 *	trailing spaces and (sometimes embedded) newlines...
 */
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
#ifdef SNMP_TESTING_CODE
char *
dump_snmpEngineID(const u_char *estring, size_t *estring_len)
{
#define eb(b)	( *(esp+b) & 0xff )

	int		 rval		= SNMPERR_SUCCESS,
                         gotviolation	= 0,
                         slen           = 0;
	u_int	 	 remaining_len;

	char	 	 buf[SNMP_MAXBUF],
			*s = NULL,
			*t;
        const u_char    *esp = estring;

	struct	in_addr	 iaddr;



	/*
	 * Sanity check.
	 */
	if ( !estring || (*estring_len <= 0) ) {
		QUITFUN(SNMPERR_GENERR, dump_snmpEngineID_quit);
	}
	remaining_len = *estring_len;
	memset(buf, 0, SNMP_MAXBUF);



	/*
	 * Test first bit.  Return immediately with a hex string, or
	 * begin by formatting the enterprise ID.
	 */
	if ( !(*esp & 0x80) ) {
		sprint_hexstring(buf, esp, remaining_len);
		s  = strchr(buf, '\0');
		s -= 1;
		goto dump_snmpEngineID_quit;
	}

	s = buf;
	s += sprintf(s, "enterprise %d, ",	((*(esp+0)&0x7f) << 24) |
						((*(esp+1)&0xff) << 16) |
						((*(esp+2)&0xff) <<  8) |
						((*(esp+3)&0xff)) );
						/* XXX  Ick. */

	if (remaining_len < 5) {	/* XXX	Violating string. */
		goto dump_snmpEngineID_quit;
	}

	esp += 4;		/* Incremented one more in the switch below. */
	remaining_len -= 5;



	/*
	 * Act on the fifth byte.
	 */
	switch ((int) *esp++) {
	case 1:					/* IPv4 address. */

		if (remaining_len < 4) goto dump_snmpEngineID_violation;
		memcpy(&iaddr.s_addr, esp, 4);

		if ( !(t = inet_ntoa(iaddr)) ) goto dump_snmpEngineID_violation;
		s += sprintf(s, "%s", t);

		esp += 4;
		remaining_len -= 4;
		break;

	case 2:					/* IPv6 address. */

		if (remaining_len < 16) goto dump_snmpEngineID_violation;

		s += sprintf(	s,
				"%02X%02X %02X%02X %02X%02X %02X%02X::"
				"%02X%02X %02X%02X %02X%02X %02X%02X",
					eb(0),  eb(1),  eb(2),  eb(3),
					eb(4),  eb(5),  eb(6),  eb(7),
					eb(8),  eb(9),  eb(10), eb(11),
					eb(12), eb(13), eb(14), eb(15) );

		esp += 16;
		remaining_len -= 16;
		break;

	case 3:					/* MAC address. */

		if (remaining_len < 6) goto dump_snmpEngineID_violation;

		s += sprintf( s, "%02X:%02X:%02X:%02X:%02X:%02X",
			eb(0), eb(1), eb(2), eb(3), eb(4), eb(5) );

		esp += 6;
		remaining_len -= 6;
		break;

	case 4:					/* Text. */

                /* Doesn't exist on all (many) architectures */
                /* s += snprintf(s, remaining_len+3, "\"%s\"", esp); */
		s += sprintf(s, "\"%s\"", esp);
		goto dump_snmpEngineID_quit;
		break;	/*NOTREACHED*/

	case 5:					/* Octets. */

		sprint_hexstring(s, esp, remaining_len);
		s  = strchr(buf, '\0');
		s -= 1;
		goto dump_snmpEngineID_quit;
		break;	/*NOTREACHED*/


dump_snmpEngineID_violation:
	case 0:					/* Violation of RESERVED, 
						 *   -OR- of expected length.
						 */
		gotviolation = 1;
		s += sprintf(s, "!!! ");

	default:				/* Unknown encoding. */

		if ( !gotviolation ) {
			s += sprintf(s, "??? ");
		}
		sprint_hexstring(s, esp, remaining_len);
		s  = strchr(buf, '\0');
		s -= 1;

		goto dump_snmpEngineID_quit;

	}  /* endswitch */



	/*
	 * Cases 1-3 (IP and MAC addresses) should not have trailing
	 * octets, but perhaps they do.  Throw them in too.  XXX
	 */
	if (remaining_len > 0) {
		s += sprintf(s, " (??? ");

		sprint_hexstring(s, esp, remaining_len);
		s  = strchr(buf, '\0');
		s -= 1;

		s += sprintf(s, ")");
	}



dump_snmpEngineID_quit:
	if (s) {
                slen = s-buf+1;
		s = calloc(1,slen);
		memcpy(s, buf, (slen)-1);
	}

	memset(buf, 0, SNMP_MAXBUF);	/* XXX -- Overkill? XXX: Yes! */

	return s;

#undef eb
}  /* end dump_snmpEngineID() */
#endif /* SNMP_TESTING_CODE */
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */


/*
 * create a new time marker.
 * NOTE: Caller must free time marker when no longer needed.
 */
marker_t atime_newMarker(void)
{
  marker_t pm = (marker_t)calloc(1,sizeof(struct timeval));
  gettimeofday((struct timeval *)pm, 0);
  return pm;
}

/*
 * set a time marker.
 */
void atime_setMarker(marker_t pm)
{
  if (! pm) return;

  gettimeofday((struct timeval *)pm, 0);
}

/*
 * Test: Has (marked time plus delta) exceeded current time ?
 * Returns 0 if test fails or cannot be tested (no marker).
 */
int atime_ready( marker_t pm, int deltaT)
{
  struct timeval txdelta, txnow;
  if (! pm) return 0;

  memcpy((void *)&txdelta, pm, sizeof(txdelta));
  while (deltaT > 1000) {
     txdelta.tv_sec ++;
     deltaT -= 1000;
  }
  txdelta.tv_usec = (deltaT * 1000) + txdelta.tv_usec;

  gettimeofday(&txnow, 0);
  if (timercmp(&txnow, &txdelta, <))
        return 0;

  return 1;
}
