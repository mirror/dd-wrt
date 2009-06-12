//==========================================================================
//
//      ./lib/current/src/asn1.c
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
 * Abstract Syntax Notation One, ASN.1
 * As defined in ISO/IS 8824 and ISO/IS 8825
 * This implements a subset of the above International Standards that
 * is sufficient to implement SNMP.
 *
 * Encodes abstract data types into a machine independent stream of bytes.
 *
 */
/**********************************************************************
	Copyright 1988, 1989, 1991, 1992 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/
#include <config.h>

#ifdef KINETICS
#include "gw.h"
#endif

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef vms
#include <in.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include "asn1.h"
#include "int64.h"
#include "snmp_debug.h"
#include "mib.h"

#ifndef NULL
#define NULL	0
#endif

#include "snmp_api.h"
#include "snmp_impl.h" /* to define ERROR_MSG */

static
void _asn_size_err(const char *str, size_t wrongsize, size_t rightsize)
{
    char ebuf[128];

    sprintf(ebuf,"%s size %d: s/b %d",str, wrongsize, rightsize);
    ERROR_MSG(ebuf);
}

static
void _asn_length_err(const char *str, size_t wrongsize, size_t rightsize)
{
    char ebuf[128];

    sprintf(ebuf,"%s length %d too large: exceeds %d",str, wrongsize, rightsize);
    ERROR_MSG(ebuf);
}

/*
 * call after asn_parse_length to verify result.
 */
static
int _asn_parse_length_check(const char *str,
                   u_char *bufp, u_char *data,
                   u_long plen, size_t dlen)
{
    char ebuf[128];
    size_t header_len;

    if (bufp == NULL){
	/* error message is set */
	return 1;
    }
    header_len = bufp - data;
    if (((size_t)plen + header_len) > dlen){
	sprintf(ebuf, "%s: message overflow: %d len + %d delta > %d len",
		str, (int)plen, (int)header_len, (int)dlen);
	ERROR_MSG(ebuf);
	return 1;
    }
    return 0;
}

/*
 * call after asn_build_header to verify result.
 */
static
int _asn_build_header_check(const char *str, u_char *data,
                      size_t datalen, size_t typedlen)
{
    char ebuf[128];
    
    if (data == NULL){
	/* error message is set */
	return 1;
    }
    if (datalen < typedlen){
	sprintf(ebuf, "%s: bad header, length too short: %d < %d", str, datalen, typedlen);
	ERROR_MSG(ebuf);
	return 1;
    }
    return 0;
}

/* checks the incoming packet for validity and returns its size or 0 */
int
asn_check_packet (u_char *pkt, size_t len)
{
  u_long asn_length;
  
  if (len < 2)
    return 0;      /* always too short */

  if (*pkt != (u_char)(ASN_SEQUENCE | ASN_CONSTRUCTOR))
    return -1;     /* wrong type */

  if (*(pkt+1) & 0x80) {
    /* long length */
    if ((int)len < (int)(*(pkt+1) & ~0x80)+2)
      return 0;    /* still to short, incomplete length */
    asn_parse_length(pkt+1, &asn_length);
    return (asn_length + 2 + (*(pkt+1) & ~0x80));
  } else {
    /* short length */
    return (*(pkt+1) + 2);
  }
}

static
int _asn_bitstring_check(const char * str, u_long asn_length, u_char datum)
{
    char ebuf[128];

    if (asn_length < 1){
	sprintf(ebuf,"%s: length %d too small", str, (int)asn_length);
	ERROR_MSG(ebuf);
	return 1;
    }
    if (datum > 7){
	sprintf(ebuf,"%s: datum %d >7: too large", str, (int)(datum));
	ERROR_MSG(ebuf);
	return 1;
    }
    return 0;
}

/*
 * asn_parse_int - pulls a long out of an ASN int type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.

  u_char * asn_parse_int(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      u_char     *type         OUT - asn type of object
      long       *intp         IN/OUT - pointer to start of output buffer
      int         intsize      IN - size of output buffer
*/

u_char *
asn_parse_int(u_char *data,
	      size_t *datalength,
	      u_char *type,
	      long *intp,
	      size_t intsize)
{
/*
 * ASN.1 integer ::= 0x02 asnlength byte {byte}*
 */
    static const char *errpre = "parse int";
    register u_char *bufp = data;
    u_long	    asn_length;
    register long   value = 0;

    if (intsize != sizeof (long)){
	_asn_size_err(errpre, intsize, sizeof(long));
	return NULL;
    }
    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (_asn_parse_length_check(errpre, bufp, data, asn_length, *datalength))
	return NULL;

    if ((size_t)asn_length > intsize){
	_asn_length_err(errpre, (size_t)asn_length, intsize);
	return NULL;
    }

    *datalength -= (int)asn_length + (bufp - data);
    if (*bufp & 0x80)
	value = -1; /* integer is negative */

    DEBUGDUMPSETUP("dump_recv", data, bufp - data + asn_length);

    while(asn_length--)
	value = (value << 8) | *bufp++;

    DEBUGMSG(("dump_recv", "  ASN Integer:\t%ld (0x%.2X)\n", value, value));

    *intp = value;
    return bufp;
}


/*
 * asn_parse_unsigned_int - pulls an unsigned long out of an ASN int type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.

  u_char * asn_parse_unsigned_int(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      u_char     *type         OUT - asn type of object
      u_long     *intp         IN/OUT - pointer to start of output buffer
      int         intsize      IN - size of output buffer
 */
u_char *
asn_parse_unsigned_int(u_char *data,
		       size_t *datalength,
		       u_char *type,
		       u_long *intp,
		       size_t intsize)
{
/*
 * ASN.1 integer ::= 0x02 asnlength byte {byte}*
 */
    static const char *errpre = "parse uint";
    register u_char *bufp = data;
    u_long	    asn_length;
    register u_long value = 0;

    if (intsize != sizeof (long)){
	_asn_size_err(errpre, intsize, sizeof(long));
	return NULL;
    }
    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (_asn_parse_length_check(errpre, bufp, data, asn_length, *datalength))
	return NULL;

    if (((int)asn_length > (intsize + 1)) ||
	(((int)asn_length == intsize + 1) && *bufp != 0x00)){
	_asn_length_err(errpre, (size_t)asn_length, intsize);
	return NULL;
    }
    *datalength -= (int)asn_length + (bufp - data);
    if (*bufp & 0x80)
	value = ~value; /* integer is negative */

    DEBUGDUMPSETUP("dump_recv", data, bufp - data + asn_length);

    while(asn_length--)
	value = (value << 8) | *bufp++;

    DEBUGMSG(("dump_recv", "  ASN UInteger:\t%ld (0x%.2X)\n", value, value));

    *intp = value;
    return bufp;
}


/*
 * asn_build_int - builds an ASN object containing an integer.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.

  u_char * asn_build_int(
      u_char     *data         IN - pointer to start of output buffer
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      int         type         IN  - asn type of object
      long       *intp         IN - pointer to start of long integer
      int         intsize      IN - size of input buffer
 */
u_char *
asn_build_int(u_char *data,
	      size_t *datalength,
	      u_char type,
	      long *intp,
	      size_t intsize)
{
/*
 * ASN.1 integer ::= 0x02 asnlength byte {byte}*
 */
	static const char *errpre = "build int";
    register long integer;
    register u_long mask;

    if (intsize != sizeof (long)){
	_asn_size_err(errpre, intsize, sizeof(long));
	return NULL;
    }
    integer = *intp;
    /*
     * Truncate "unnecessary" bytes off of the most significant end of this
     * 2's complement integer.  There should be no sequence of 9
     * consecutive 1's or 0's at the most significant end of the
     * integer.
     */
    mask = ((u_long) 0x1FF) << ((8 * (sizeof(long) - 1)) - 1);
    /* mask is 0xFF800000 on a big-endian machine */
    while((((integer & mask) == 0) || ((integer & mask) == mask))
	  && intsize > 1){
	intsize--;
	integer <<= 8;
    }
    data = asn_build_header(data, datalength, type, intsize);
    if (_asn_build_header_check(errpre,data,*datalength,intsize))
	return NULL;

    *datalength -= intsize;
    mask = ((u_long) 0xFF) << (8 * (sizeof(long) - 1));
    /* mask is 0xFF000000 on a big-endian machine */
    while(intsize--){
	*data++ = (u_char)((integer & mask) >> (8 * (sizeof(long) - 1)));
	integer <<= 8;
    }
    return data;
}


/*
 * asn_build_unsigned_int - builds an ASN object containing an integer.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.

  u_char * asn_build_unsigned_int(
      u_char     *data         IN - pointer to start of output buffer
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      u_char      type         IN  - asn type of object
      u_long     *intp         IN - pointer to start of long integer
      int         intsize      IN - size of input buffer
 */
u_char *
asn_build_unsigned_int(u_char *data,
		       size_t *datalength,
		       u_char type,
		       u_long *intp,
		       size_t intsize)
{
/*
 * ASN.1 integer ::= 0x02 asnlength byte {byte}*
 */
	static const char *errpre = "build uint";
    register u_long integer;
    register u_long mask;
    int add_null_byte = 0;

    if (intsize != sizeof (long)){
	_asn_size_err(errpre, intsize, sizeof(long));
	return NULL;
    }
    integer = *intp;
    mask = ((u_long) 0xFF) << (8 * (sizeof(long) - 1));
    /* mask is 0xFF000000 on a big-endian machine */
    if ((u_char)((integer & mask) >> (8 * (sizeof(long) - 1))) & 0x80){
	/* if MSB is set */
	add_null_byte = 1;
	intsize++;
    } else {
	/*
	 * Truncate "unnecessary" bytes off of the most significant end of this 2's complement integer.
	 * There should be no sequence of 9 consecutive 1's or 0's at the most significant end of the
	 * integer.
	 */
	mask = ((u_long) 0x1FF) << ((8 * (sizeof(long) - 1)) - 1);
	/* mask is 0xFF800000 on a big-endian machine */
	while((((integer & mask) == 0) || ((integer & mask) == mask)) && intsize > 1){
	    intsize--;
	    integer <<= 8;
	}
    }
    data = asn_build_header(data, datalength, type, intsize);
    if (_asn_build_header_check(errpre,data,*datalength,intsize))
	return NULL;

    *datalength -= intsize;
    if (add_null_byte == 1){
	*data++ = '\0';
	intsize--;
    }
    mask = ((u_long) 0xFF) << (8 * (sizeof(long) - 1));
    /* mask is 0xFF000000 on a big-endian machine */
    while(intsize--){
	*data++ = (u_char)((integer & mask) >> (8 * (sizeof(long) - 1)));
	integer <<= 8;
    }
    return data;
}


/*
 * asn_parse_string - pulls an octet string out of an ASN octet string type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  "string" is filled with the octet string.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.
 *
 * u_char * asn_parse_string(
 *     u_char     *data         IN - pointer to start of object
 *     int        *datalength   IN/OUT - number of valid bytes left in buffer
 *     u_char     *type         OUT - asn type of object
 *     u_char     *string       IN/OUT - pointer to start of output buffer
 *     int        *strlength    IN/OUT - size of output buffer
 *
 *
 * ASN.1 octet string	::=      primstring | cmpdstring
 * primstring		::= 0x04 asnlength byte {byte}*
 * cmpdstring		::= 0x24 asnlength string {string}*
 */
u_char *
asn_parse_string(u_char *data,
		 size_t *datalength,
		 u_char *type,
		 u_char *string,
		 size_t *strlength)
{
    static const char *errpre = "parse string";
    u_char *bufp = data;
    u_long  asn_length;

    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (_asn_parse_length_check(errpre, bufp, data, asn_length, *datalength))
	return NULL;

    if ((int)asn_length > *strlength){
	_asn_length_err(errpre, (size_t)asn_length, *strlength);
	return NULL;
    }

    DEBUGDUMPSETUP("dump_recv", data, bufp - data + asn_length);

    memmove(string, bufp, asn_length);
    if (*strlength > (int)asn_length)
      string[asn_length] = 0;
    *strlength = (int)asn_length;
    *datalength -= (int)asn_length + (bufp - data);

    DEBUGIF("dump_recv") {
      char *buf = (char *)malloc(1+asn_length);
      sprint_asciistring(buf, string, asn_length);
      DEBUGMSG(("dump_recv", "  ASN String:\t%s\n", buf));
      free (buf);
    }
        
    return bufp + asn_length;
}


/*
 * asn_build_string - Builds an ASN octet string object containing the input string.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.

  u_char * asn_build_string(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      u_char      type         IN - asn type of object
      u_char     *string       IN - pointer to start of input buffer
      int         strlength    IN - size of input buffer
 */
u_char *
asn_build_string(u_char *data,
		 size_t *datalength,
		 u_char	type,
		 const u_char *string,
		 size_t strlength)
{
/*
 * ASN.1 octet string ::= primstring | cmpdstring
 * primstring ::= 0x04 asnlength byte {byte}*
 * cmpdstring ::= 0x24 asnlength string {string}*
 * This code will never send a compound string.
 */
    data = asn_build_header(data, datalength, type, strlength);
    if (_asn_build_header_check("build string", data, *datalength, strlength))
	return NULL;

    if (strlength) {
      if (string == NULL) {
	memset(data, 0, strlength);
      } else {
	memmove(data, string, strlength);
      }
    }
    *datalength -= strlength;
    return data + strlength;
}



/*
 * asn_parse_header - interprets the ID and length of the current object.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   in this object following the id and length.
 *
 *  Returns a pointer to the first byte of the contents of this object.
 *  Returns NULL on any error.

  u_char * asn_parse_header(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      u_char     *type         OUT - asn type of object
 */
u_char *
asn_parse_header(u_char	*data,
		 size_t *datalength,
		 u_char	*type)
{
    register u_char *bufp;
    u_long	    asn_length;

    if (!data || !datalength || !type) {
	ERROR_MSG("parse header: NULL pointer");
	return NULL;
    }
    bufp = data;
    /* this only works on data types < 30, i.e. no extension octets */
    if (IS_EXTENSION_ID(*bufp)){
	ERROR_MSG("can't process ID >= 30");
	return NULL;
    }
    *type = *bufp;
    bufp = asn_parse_length(bufp + 1, &asn_length);
    if (_asn_parse_length_check("parse header", bufp, data, asn_length, *datalength))
	return NULL;

    DEBUGDUMPSETUP("dump_recv", data, (bufp-data));
    DEBUGMSG(("dump_recv", "  ASN Header: 0x%.2X, len = %d (0x%X)\n", *data,
              asn_length, asn_length));

#ifdef OPAQUE_SPECIAL_TYPES

    if ((*type == ASN_OPAQUE) &&
        (*bufp == ASN_OPAQUE_TAG1)) {
      DEBUGINDENTMORE();
      DEBUGDUMPSETUP("dump_recv", data, 1);
      DEBUGMSG(("dump_recv", "Opaque:\t%.2x\n", *bufp));
      DEBUGINDENTLESS();

      /* check if 64-but counter */
      switch(*(bufp+1)) {
        case ASN_OPAQUE_COUNTER64:
        case ASN_OPAQUE_U64:
        case ASN_OPAQUE_FLOAT:
        case ASN_OPAQUE_DOUBLE:
        case ASN_OPAQUE_I64:
          *type = *(bufp+1);
          break;
        
        default:
          /* just an Opaque */
          *datalength = (int)asn_length;
          return bufp;
      }
      /* value is encoded as special format */
      bufp = asn_parse_length(bufp + 2, &asn_length);
      if (_asn_parse_length_check("parse opaque header", bufp, data,
                  asn_length, *datalength))
        return NULL;
    }
#endif /* OPAQUE_SPECIAL_TYPES */

    *datalength = (int)asn_length;

    return bufp;
}

/*
 * same as asn_parse_header with test for expected type.
 */
u_char *
asn_parse_sequence(u_char	*data,
		 size_t *datalength,
		 u_char	*type,
		 u_char	expected_type, /* must be this type */
		 const char *estr)	/* error message prefix */
{
    data = asn_parse_header(data, datalength, type);
    if (data && (*type != expected_type)) {
	char ebuf[128];
	sprintf(ebuf, "%s header type %02X: s/b %02X", estr,
			(u_char)*type, (u_char)expected_type);
	ERROR_MSG(ebuf);
	return NULL;
    }
    return data;
}



/*
 * asn_build_header - builds an ASN header for an object with the ID and
 * length specified.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   in this object following the id and length.
 *
 *  This only works on data types < 30, i.e. no extension octets.
 *  The maximum length is 0xFFFF;
 *
 *  Returns a pointer to the first byte of the contents of this object.
 *  Returns NULL on any error.

  u_char * asn_build_header(
      u_char     *data         IN - pointer to start of object
      size_t     *datalength   IN/OUT - number of valid bytes left in buffer
      u_char      type         IN - asn type of object
      size_t      length       IN - length of object
 */
u_char *
asn_build_header (u_char *data,
		  size_t *datalength,
		  u_char type,
		  size_t length)
{
    char ebuf[128];
    
    if (*datalength < 1){
	sprintf(ebuf, "bad header length < 1 :%d, %d", *datalength, length);
	ERROR_MSG(ebuf);
	return NULL;
    }	    
    *data++ = type;
    (*datalength)--;
    return asn_build_length(data, datalength, length);
}

/*
 * asn_build_sequence - builds an ASN header for a sequence with the ID and
 * length specified.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   in this object following the id and length.
 *
 *  This only works on data types < 30, i.e. no extension octets.
 *  The maximum length is 0xFFFF;
 *
 *  Returns a pointer to the first byte of the contents of this object.
 *  Returns NULL on any error.

  u_char * asn_build_sequence(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      u_char      type         IN - asn type of object
      int         length       IN - length of object
 */
u_char *
asn_build_sequence(u_char *data,
		  size_t *datalength,
		  u_char type,
		  size_t length)
{
    static const char *errpre = "build seq";
    char ebuf[128];
    
    if (*datalength < 4){
	sprintf(ebuf, "%s: length %d < 4: PUNT", errpre, (int)*datalength);
	ERROR_MSG(ebuf);
	return NULL;
    }
    *datalength -= 4;
    *data++ = type;
    *data++ = (u_char)(0x02 | ASN_LONG_LEN);
    *data++ = (u_char)((length >> 8) & 0xFF);
    *data++ = (u_char)(length & 0xFF);
    return data;
}

/*
 * asn_parse_length - interprets the length of the current object.
 *  On exit, length contains the value of this length field.
 *
 *  Returns a pointer to the first byte after this length
 *  field (aka: the start of the data field).
 *  Returns NULL on any error.

  u_char * asn_parse_length(
      u_char     *data         IN - pointer to start of length field
      u_long     *length       OUT - value of length field
 */
u_char *
asn_parse_length(u_char  *data,
		 u_long  *length)
{
    static const char *errpre = "parse length";
    char ebuf[128];
    register u_char lengthbyte;
    
    if (!data || !length) {
	ERROR_MSG("parse length: NULL pointer");
	return NULL;
    }
    lengthbyte = *data;

    if (lengthbyte & ASN_LONG_LEN){
	lengthbyte &= ~ASN_LONG_LEN;	/* turn MSb off */
	if (lengthbyte == 0){
	    sprintf(ebuf, "%s: indefinite length not supported", errpre);
	    ERROR_MSG(ebuf);
	    return NULL;
	}
	if (lengthbyte > sizeof(long)){
	    sprintf(ebuf, "%s: data length %d > %d not supported", errpre,
                 lengthbyte, sizeof(long));
	    ERROR_MSG(ebuf);
	    return NULL;
	}
	data++;
	*length = 0;  /* protect against short lengths */
	while(lengthbyte--) {
		*length <<= 8;
		*length |= *data++;
	}
	return data;
    } else { /* short asnlength */
	*length = (long)lengthbyte;
	return data + 1;
    }
}

/*

  u_char * asn_build_length(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      int         length       IN - length of object
 */
u_char *
asn_build_length(u_char *data,
		 size_t *datalength,
		 size_t length)
{
    static const char *errpre = "build length";
    char ebuf[128];
    
    u_char    *start_data = data;

    /* no indefinite lengths sent */
    if (length < 0x80){
	if (*datalength < 1){
	    sprintf(ebuf, "%s: bad length < 1 :%d, %d",errpre,*datalength,length);
	    ERROR_MSG(ebuf);
	    return NULL;
	}	    
	*data++ = (u_char)length;
    } else if (length <= 0xFF){
	if (*datalength < 2){
	    sprintf(ebuf, "%s: bad length < 2 :%d, %d",errpre,*datalength,length);
	    ERROR_MSG(ebuf);
	    return NULL;
	}	    
	*data++ = (u_char)(0x01 | ASN_LONG_LEN);
	*data++ = (u_char)length;
    } else { /* 0xFF < length <= 0xFFFF */
	if (*datalength < 3){
	    sprintf(ebuf, "%s: bad length < 3 :%d, %d",errpre,*datalength,length);
	    ERROR_MSG(ebuf);
	    return NULL;
	}	    
	*data++ = (u_char)(0x02 | ASN_LONG_LEN);
	*data++ = (u_char)((length >> 8) & 0xFF);
	*data++ = (u_char)(length & 0xFF);
    }
    *datalength -= (data - start_data);
    return data;

}

/*
 * asn_parse_objid - pulls an object indentifier out of an ASN object identifier type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  "objid" is filled with the object identifier.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.

  u_char * asn_parse_objid(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      u_char     *type         OUT - asn type of object
      oid        *objid        IN/OUT - pointer to start of output buffer
      int        *objidlength  IN/OUT - number of sub-id's in objid
 */
u_char *
asn_parse_objid(u_char *data,
		size_t *datalength,
		u_char *type,	
		oid *objid,
		size_t *objidlength)
{
/*
 * ASN.1 objid ::= 0x06 asnlength subidentifier {subidentifier}*
 * subidentifier ::= {leadingbyte}* lastbyte
 * leadingbyte ::= 1 7bitvalue
 * lastbyte ::= 0 7bitvalue
 */
    register u_char *bufp = data;
    register oid *oidp = objid + 1;
    register u_long subidentifier;
    register long   length;
    u_long	    asn_length;

    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (_asn_parse_length_check("parse objid", bufp, data,
                    asn_length, *datalength))
	return NULL;

    *datalength -= (int)asn_length + (bufp - data);

    DEBUGDUMPSETUP("dump_recv", data, bufp - data + asn_length);

    /* Handle invalid object identifier encodings of the form 06 00 robustly */
    if (asn_length == 0)
	objid[0] = objid[1] = 0;

    length = asn_length;
    (*objidlength)--;	/* account for expansion of first byte */

    while (length > 0 && (*objidlength)-- > 0){
	subidentifier = 0;
	do {	/* shift and add in low order 7 bits */
	    subidentifier = (subidentifier << 7) + (*(u_char *)bufp & ~ASN_BIT8);
	    length--;
	} while (*(u_char *)bufp++ & ASN_BIT8);	/* last byte has high bit clear */
/*?? note, this test will never be true, since the largest value
     of subidentifier is the value of MAX_SUBID! */
	if (subidentifier > (u_long)MAX_SUBID){
	    ERROR_MSG("subidentifier too large");
	    return NULL;
	}
	*oidp++ = (oid)subidentifier;
    }

    /*
     * The first two subidentifiers are encoded into the first component
     * with the value (X * 40) + Y, where:
     *	X is the value of the first subidentifier.
     *  Y is the value of the second subidentifier.
     */
    subidentifier = (u_long)objid[1];
    if (subidentifier == 0x2B){
	objid[0] = 1;
	objid[1] = 3;
    } else {
        if (subidentifier < 40) {
            objid[0] = 0;
	    objid[1] = subidentifier;
        } else if (subidentifier < 80) {
            objid[0] = 1;
            objid[1] = subidentifier - 40;
        } else if (subidentifier < 120) {
            objid[0] = 2;
            objid[1] = subidentifier - 80;
        } else {
	    objid[1] = (subidentifier % 40);
	    objid[0] = ((subidentifier - objid[1]) / 40);
        }
    }

    *objidlength = (int)(oidp - objid);

    DEBUGMSG(("dump_recv", "  ASN ObjID: "));
    DEBUGMSGOID(("dump_recv", objid, *objidlength));
    DEBUGMSG(("dump_recv", "\n"));
    return bufp;
}

/*
 * asn_build_objid - Builds an ASN object identifier object containing the
 * input string.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.

  u_char * asn_build_objid(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      int        type         IN - asn type of object
      oid        *objid        IN - pointer to start of input buffer
      int         objidlength  IN - number of sub-id's in objid
 */
u_char *
asn_build_objid(u_char *data,
		size_t *datalength,
		u_char type,
		oid *objid,
		size_t objidlength)
{
/*
 * ASN.1 objid ::= 0x06 asnlength subidentifier {subidentifier}*
 * subidentifier ::= {leadingbyte}* lastbyte
 * leadingbyte ::= 1 7bitvalue
 * lastbyte ::= 0 7bitvalue
 */
    size_t asnlength;
    register oid *op = objid;
    u_char objid_size[MAX_OID_LEN];
    register u_long objid_val;
    u_long first_objid_val;
    register int i;

    /* check if there are at least 2 sub-identifiers */
    if (objidlength == 0){
        /* there are not, so make OID have two with value of zero */
        objid_val = 0;
	objidlength = 2;
    } else if (objidlength == 1){
        /* encode the first value */
	objid_val = (op[0] * 40);
	objidlength = 2;
	op++;
    } else {
        /* combine the first two values */
	if ( op[1] > 40 ) {
	    ERROR_MSG("build objid: bad second subidentifier");
	    return NULL;
	}
	objid_val = (op[0] * 40) + op[1];
	op += 2;
    }
    first_objid_val = objid_val;

    /* calculate the number of bytes needed to store the encoded value */
    for (i = 1, asnlength = 0;;) {
        if (objid_val < (unsigned)0x80) {
            objid_size[i] = 1;
            asnlength += 1;
        } else if (objid_val < (unsigned)0x4000) {
            objid_size[i] = 2;
            asnlength += 2;
        } else if (objid_val < (unsigned)0x200000) {
            objid_size[i] = 3;
            asnlength += 3;
        } else if (objid_val < (unsigned)0x10000000) {
            objid_size[i] = 4;
            asnlength += 4;
        } else {
            objid_size[i] = 5;
            asnlength += 5;
        }
        i++;
        if (i >= (int)objidlength)
            break;
        objid_val = *op++;
    } 

    /* store the ASN.1 tag and length */
    data = asn_build_header(data, datalength, type, asnlength);
    if (_asn_build_header_check("build objid", data, *datalength, asnlength))
	return NULL;

    /* store the encoded OID value */
    for (i = 1, objid_val = first_objid_val, op = objid+2;
				i < (int)objidlength;
                i++) {
      if (i != 1) objid_val = *op++;
        switch (objid_size[i]) {
        case 1:
            *data++ = (u_char)objid_val;
            break;

        case 2:
            *data++ = (u_char)((objid_val>>7) | 0x80);
            *data++ = (u_char)(objid_val & 0x07f);
            break;

        case 3:
            *data++ = (u_char)((objid_val>>14) | 0x80);
            *data++ = (u_char)((objid_val>>7 & 0x7f) | 0x80);
            *data++ = (u_char)(objid_val & 0x07f);
            break;

        case 4:
            *data++ = (u_char)((objid_val>>21) | 0x80);
            *data++ = (u_char)((objid_val>>14 & 0x7f) | 0x80);
            *data++ = (u_char)((objid_val>>7 & 0x7f) | 0x80);
            *data++ = (u_char)(objid_val & 0x07f);
            break;

        case 5:
            *data++ = (u_char)((objid_val>>28) | 0x80);
            *data++ = (u_char)((objid_val>>21 & 0x7f) | 0x80);
            *data++ = (u_char)((objid_val>>14 & 0x7f) | 0x80);
            *data++ = (u_char)((objid_val>>7 & 0x7f) | 0x80);
            *data++ = (u_char)(objid_val & 0x07f);
            break;
        }
    }

    /* return the length and data ptr */
    *datalength -= asnlength;
    return data;
}

/*
 * asn_parse_null - Interprets an ASN null type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.

  u_char * asn_parse_null(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      u_char     *type         OUT - asn type of object
 */
u_char *
asn_parse_null(u_char *data,
	       size_t *datalength,
	       u_char *type)
{
/*
 * ASN.1 null ::= 0x05 0x00
 */
    register u_char   *bufp = data;
    u_long	    asn_length;

    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (bufp == NULL){
	ERROR_MSG("parse null: bad length");
	return NULL;
    }
    if (asn_length != 0){
	ERROR_MSG("parse null: malformed ASN.1 null");
	return NULL;
    }

    *datalength -= (bufp - data);

    DEBUGDUMPSETUP("dump_recv", data, bufp - data);
    DEBUGMSG(("dump_recv", "  ASN NULL\n"));

    return bufp + asn_length;
}


/*
 * asn_build_null - Builds an ASN null object.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.

  u_char * asn_build_null(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      u_char      type         IN - asn type of object
 */
u_char *
asn_build_null(u_char *data,
	       size_t *datalength,
	       u_char type)
{
/*
 * ASN.1 null ::= 0x05 0x00
 */
    return asn_build_header(data, datalength, type, 0);
}

/*
 * asn_parse_bitstring - pulls a bitstring out of an ASN bitstring type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  "string" is filled with the bit string.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.

  u_char * asn_parse_bitstring(
      u_char     *data         IN - pointer to start of object
      size_t     *datalength   IN/OUT - number of valid bytes left in buffer
      u_char     *type         OUT - asn type of object
      u_char     *string       IN/OUT - pointer to start of output buffer
      size_t     *strlength    IN/OUT - size of output buffer
 */
u_char *
asn_parse_bitstring(u_char *data,
		    size_t *datalength,
		    u_char *type,
		    u_char *string,
		    size_t *strlength)
{
/*
 * bitstring ::= 0x03 asnlength unused {byte}*
 */
    static const char *errpre = "parse bitstring";
    register u_char *bufp = data;
    u_long	    asn_length;

    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (_asn_parse_length_check(errpre, bufp, data,
               asn_length, *datalength))
	return NULL;

    if ((size_t)asn_length > *strlength){
	_asn_length_err(errpre, (size_t)asn_length, *strlength);
	return NULL;
    }
    if (_asn_bitstring_check(errpre, asn_length, *bufp))
	return NULL;

    DEBUGDUMPSETUP("dump_recv", data, bufp - data);
    DEBUGMSG(("dump_recv", "  ASN Bitstring: "));
    DEBUGMSGHEX(("dump_recv", data, asn_length));

    memmove(string, bufp, asn_length);
    *strlength = (int)asn_length;
    *datalength -= (int)asn_length + (bufp - data);
    return bufp + asn_length;
}


/*
 * asn_build_bitstring - Builds an ASN bit string object containing the
 * input string.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the beginning of the next object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.

  u_char * asn_build_bitstring(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      u_char      type         IN - asn type of object
      u_char     *string       IN - pointer to start of input buffer
      int         strlength    IN - size of input buffer
 */
u_char *
asn_build_bitstring(u_char *data,
		    size_t *datalength,
		    u_char type,
		    u_char *string,
		    size_t strlength)
{
/*
 * ASN.1 bit string ::= 0x03 asnlength unused {byte}*
 */
    static const char *errpre = "build bitstring";
    if (_asn_bitstring_check(errpre, strlength, *string))
	return NULL;

    data = asn_build_header(data, datalength, type, strlength);
    if (_asn_build_header_check(errpre,data,*datalength,strlength))
	return NULL;

    memmove(data, string, strlength);
    *datalength -= strlength;
    return data + strlength;
}


/*
 * asn_parse_unsigned_int64 - pulls a 64 bit unsigned long out of an ASN int
 * type.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.

  u_char * asn_parse_unsigned_int64(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      u_char     *type         OUT - asn type of object
      struct counter64 *cp     IN/OUT - pointer to counter struct
      int         countersize  IN - size of output buffer
 */
u_char *
asn_parse_unsigned_int64(u_char *data,
			 size_t *datalength,
			 u_char *type, 
			 struct counter64 *cp,
			 size_t countersize)
{
/*
 * ASN.1 integer ::= 0x02 asnlength byte {byte}*
 */
    static const char *errpre = "parse uint64";
    const int uint64sizelimit = (4 * 2) + 1;
    register u_char *bufp = data;
    u_long	    asn_length;
    register u_long low = 0, high = 0;
    
    if (countersize != sizeof(struct counter64)){
	_asn_size_err(errpre, countersize, sizeof(struct counter64));
	return NULL;
    }
    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (_asn_parse_length_check(errpre, bufp, data, asn_length, *datalength))
        return NULL;

    DEBUGDUMPSETUP("dump_recv", data, bufp - data);
#ifdef OPAQUE_SPECIAL_TYPES
/* 64 bit counters as opaque */
    if ((*type == ASN_OPAQUE) &&
            (asn_length <= ASN_OPAQUE_COUNTER64_MX_BER_LEN) &&
	    (*bufp == ASN_OPAQUE_TAG1) &&
	    ((*(bufp+1) == ASN_OPAQUE_COUNTER64) ||
             (*(bufp+1) == ASN_OPAQUE_U64))) {
        DEBUGMSG(("dump_recv", "Opaque %.2x %.2x: ", *bufp, *(bufp+1)));

	/* change type to Counter64 or U64 */
        *type = *(bufp+1);
        /* value is encoded as special format */
	bufp = asn_parse_length(bufp + 2, &asn_length);
	if (_asn_parse_length_check("parse opaque uint64", bufp, data,
                  asn_length, *datalength))
        return NULL;
    }
#endif /* OPAQUE_SPECIAL_TYPES */
    if (((int)asn_length > uint64sizelimit) ||
	(((int)asn_length == uint64sizelimit) && *bufp != 0x00)){
	_asn_length_err(errpre, (size_t)asn_length, uint64sizelimit);
	return NULL;
    }
    *datalength -= (int)asn_length + (bufp - data);
    if (*bufp & 0x80){
	low = ~low; /* integer is negative */
	high = ~high;
    }

    while(asn_length--){
	high = (high << 8) | ((low & 0xFF000000) >> 24);
	low = (low << 8) | *bufp++;
    }

    cp->low = low;
    cp->high = high;

    DEBUGIF("dump_recv") {
      char i64buf[I64CHARSZ+1];
      printU64(i64buf, cp);
    }

    return bufp;
}


/*
 * asn_build_unsigned_int64 - builds an ASN object containing a 64 bit integer.
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.

  u_char * asn_build_unsigned_int64(
      u_char     *data         IN - pointer to start of output buffer
      size_t     *datalength   IN/OUT - number of valid bytes left in buffer
      u_char      type         IN  - asn type of object
      struct counter64 *cp     IN - pointer to counter struct
      size_t      countersize  IN - size of input buffer
 */
u_char *
asn_build_unsigned_int64(u_char *data,
			 size_t *datalength,
			 u_char type,
			 struct counter64 *cp,
			 size_t countersize)
{
/*
 * ASN.1 integer ::= 0x02 asnlength byte {byte}*
 */

    register u_long low, high;
    register u_long mask, mask2;
    int add_null_byte = 0;
    size_t intsize;

  if (countersize != sizeof(struct counter64)){
    _asn_size_err("build uint64", countersize, sizeof(struct counter64));
    return NULL;
  }
    intsize = 8;
    low = cp->low;
    high = cp->high;
    mask = ((u_long) 0xFF) << (8 * (sizeof(long) - 1));
    /* mask is 0xFF000000 on a big-endian machine */
    if ((u_char)((high & mask) >> (8 * (sizeof(long) - 1))) & 0x80){
	/* if MSB is set */
	add_null_byte = 1;
	intsize++;
    } else {
	/*
	 * Truncate "unnecessary" bytes off of the most significant end of this 2's
	 * complement integer.
	 * There should be no sequence of 9 consecutive 1's or 0's at the most
	 * significant end of the integer.
	 */
	mask2 = ((u_long) 0x1FF) << ((8 * (sizeof(long) - 1)) - 1);
	/* mask2 is 0xFF800000 on a big-endian machine */
	while((((high & mask2) == 0) || ((high & mask2) == mask2)) && intsize > 1){
	    intsize--;
	    high = (high << 8)
		| ((low & mask) >> (8 * (sizeof(long) - 1)));
	    low <<= 8;
	}
    }
#ifdef OPAQUE_SPECIAL_TYPES
/* encode a Counter64 as an opaque (it also works in SNMPv1) */
    /* turn into Opaque holding special tagged value */
    if (type == ASN_OPAQUE_COUNTER64) {
        /* put the tag and length for the Opaque wrapper */
        data = asn_build_header(data, datalength, ASN_OPAQUE, intsize+3);
    if (_asn_build_header_check("build counter u64", data, *datalength, intsize+3))
	return NULL;

	/* put the special tag and length */
	*data++ = ASN_OPAQUE_TAG1;
	*data++ = ASN_OPAQUE_COUNTER64;
	*data++ = (u_char)intsize;
	*datalength = *datalength - 3;
    }
    else
/* Encode the Unsigned int64 in an opaque */
    /* turn into Opaque holding special tagged value */
    if (type == ASN_OPAQUE_U64) {
        /* put the tag and length for the Opaque wrapper */
        data = asn_build_header(data, datalength, ASN_OPAQUE, intsize+3);
    if (_asn_build_header_check("build opaque u64", data, *datalength, intsize+3))
	return NULL;

	/* put the special tag and length */
	*data++ = ASN_OPAQUE_TAG1;
	*data++ = ASN_OPAQUE_U64;
	*data++ = (u_char)intsize;
	*datalength = *datalength - 3;
    }
    else
    {
#endif /* OPAQUE_SPECIAL_TYPES */
    data = asn_build_header(data, datalength, type, intsize);
    if (_asn_build_header_check("build uint64", data, *datalength, intsize))
	return NULL;

#ifdef OPAQUE_SPECIAL_TYPES
    }
#endif /* OPAQUE_SPECIAL_TYPES */
    *datalength -= intsize;
    if (add_null_byte == 1){
	*data++ = '\0';
	intsize--;
    }
    while(intsize--){
	*data++ = (u_char)((high & mask) >> (8 * (sizeof(long) - 1)));
	high = (high << 8)
	    | ((low & mask) >> (8 * (sizeof(long) - 1)));
	low <<= 8;
	
    }
    return data;
}

#ifdef OPAQUE_SPECIAL_TYPES

/*

  u_char * asn_parse_signed_int64(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      u_char     *type         OUT - asn type of object
      struct counter64 *cp     IN/OUT - pointer to counter struct
      int         countersize  IN - size of output buffer
 */

u_char *
asn_parse_signed_int64(u_char *data,
		       size_t *datalength,
		       u_char *type,
		       struct counter64 *cp,
		       size_t countersize)
{
  static const char *errpre = "parse int64";
  const int int64sizelimit = (4 * 2) + 1;
  char ebuf[128];
  register u_char *bufp = data;
  u_long	    asn_length;
  register u_int low = 0, high = 0;
    
  if (countersize != sizeof(struct counter64)){
    _asn_size_err(errpre, countersize, sizeof(struct counter64));
    return NULL;
  }
  *type = *bufp++;
  bufp = asn_parse_length(bufp, &asn_length);
  if (_asn_parse_length_check(errpre, bufp, data, asn_length, *datalength))
        return NULL;

  DEBUGDUMPSETUP("dump_recv", data, bufp - data);
  if ((*type == ASN_OPAQUE) &&
      (asn_length <= ASN_OPAQUE_COUNTER64_MX_BER_LEN) &&
      (*bufp == ASN_OPAQUE_TAG1) &&
       (*(bufp+1) == ASN_OPAQUE_I64)) {
      DEBUGMSG(("dump_recv", "Opaque %.2x %.2x: ", *bufp, *(bufp+1)));
    /* change type to Int64 */
    *type = *(bufp+1);
    /* value is encoded as special format */
    bufp = asn_parse_length(bufp + 2, &asn_length);
    if (_asn_parse_length_check("parse opaque int64", bufp, data,
                  asn_length, *datalength))
        return NULL;
  }
  /* this should always have been true until snmp gets int64 PDU types */
  else {
    sprintf(ebuf, "%s: wrong type: %d, len %d, buf bytes (%02X,%02X)",
             errpre, *type, (int)asn_length, *bufp, *(bufp+1));
    ERROR_MSG(ebuf);
    return NULL;
  }
  if (((int)asn_length > int64sizelimit) ||
      (((int)asn_length == int64sizelimit) && *bufp != 0x00)){
    _asn_length_err(errpre, (size_t)asn_length, int64sizelimit);
    return NULL;
  }
  *datalength -= (int)asn_length + (bufp - data);
  if (*bufp & 0x80){
    low = ~low; /* integer is negative */
    high = ~high;
  }

  while(asn_length--){
    high = (high << 8) | ((low & 0xFF000000) >> 24);
    low = (low << 8) | *bufp++;
  }

  cp->low = low;
  cp->high = high;

  DEBUGIF("dump_recv") {
    char i64buf[I64CHARSZ+1];
    printI64(i64buf, cp);
  }

  return bufp;
}


/*

  u_char * asn_build_signed_int64(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      u_char      type         IN - asn type of object
      struct counter64 *cp     IN - pointer to counter struct
      int         countersize  IN - size of input buffer
 */
u_char *
asn_build_signed_int64(u_char *data,
		       size_t *datalength,
		       u_char type,
		       struct counter64 *cp,
		       size_t countersize)
{
/*
 * ASN.1 integer ::= 0x02 asnlength byte {byte}*
 */

    struct counter64 c64;
    register u_int mask, mask2;
    u_long low, high;
    size_t intsize;

  if (countersize != sizeof(struct counter64)){
    _asn_size_err("build int64", countersize, sizeof(struct counter64));
    return NULL;
  }
    intsize = 8;
    memcpy(&c64, cp, sizeof(struct counter64));  /* we're may modify it */
    low = c64.low;
    high = c64.high;
    
    /*
     * Truncate "unnecessary" bytes off of the most significant end of this
     * 2's complement integer.  There should be no sequence of 9
     * consecutive 1's or 0's at the most significant end of the
     * integer.
     */
    mask = ((u_int) 0xFF) << (8 * (sizeof(u_int) - 1));
    mask2 = ((u_int) 0x1FF) << ((8 * (sizeof(u_int) - 1)) - 1);
    /* mask is 0xFF800000 on a big-endian machine */
    while((((high & mask2) == 0) || ((high & mask2) == mask2)) && intsize > 1){
      intsize--;
      high = (high << 8)
        | ((low & mask) >> (8 * (sizeof(u_int) - 1)));
      low <<= 8;
    }
    /* until a real int64 gets incorperated into SNMP, we are going to
       encode it as an opaque instead.  First, we build the opaque
       header and then the int64 tag type we use to mark it as an
       int64 in the opaque string. */
    data = asn_build_header(data, datalength, ASN_OPAQUE, intsize+3);
    if (_asn_build_header_check("build int64", data, *datalength, intsize+3))
	return NULL;

    *data++ = ASN_OPAQUE_TAG1;
    *data++ = ASN_OPAQUE_I64;
    *data++ = (u_char)intsize;
    *datalength -= (3 + intsize);
    
    while(intsize--){
	*data++ = (u_char)((high & mask) >> (8 * (sizeof(u_int) - 1)));
	high = (high << 8)
	    | ((low & mask) >> (8 * (sizeof(u_int) - 1)));
	low <<= 8;
    }
    return data;
}

/*
 * asn_parse_float - pulls a single precision floating-point out of an opaque type.
 *
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.

  u_char * asn_parse_float(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      u_char     *type         OUT - asn type of object
      float      *floatp       IN/OUT - pointer to float
      int         floatsize    IN - size of output buffer
 */
u_char *
asn_parse_float(u_char *data,
		size_t *datalength,
		u_char *type,
		float *floatp,
		size_t floatsize)
{
    register u_char *bufp = data;
    u_long	    asn_length;
    union {
        float  floatVal;
	long   longVal;
	u_char c[sizeof(float)];
    } fu;

    if (floatsize != sizeof(float)){
	_asn_size_err("parse float", floatsize, sizeof(float));
	return NULL;
    }
    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (_asn_parse_length_check("parse float", bufp, data,
                  asn_length, *datalength))
        return NULL;

    DEBUGDUMPSETUP("dump_recv", data, bufp - data + asn_length);
/* the float is encoded as an opaque */
    if ((*type == ASN_OPAQUE) &&
            (asn_length == ASN_OPAQUE_FLOAT_BER_LEN) &&
	    (*bufp == ASN_OPAQUE_TAG1) &&
	    (*(bufp+1) == ASN_OPAQUE_FLOAT)) {
      DEBUGMSG(("dump_recv", "Opaque %.2x %.2x: ", *bufp, *(bufp+1)));

        /* value is encoded as special format */
	bufp = asn_parse_length(bufp + 2, &asn_length);
	if (_asn_parse_length_check("parse opaque float", bufp, data,
                  asn_length, *datalength))
        return NULL;

	/* change type to Float */
	*type = ASN_OPAQUE_FLOAT;
    }

    if (asn_length != sizeof(float)) {
	_asn_size_err("parse seq float", asn_length, sizeof(float));
	return NULL;
    }

    *datalength -= (int)asn_length + (bufp - data);
    memcpy(&fu.c[0], bufp, asn_length);

   /* correct for endian differences */
    fu.longVal = ntohl(fu.longVal);	

    *floatp =  fu.floatVal;

    DEBUGMSG(("dump_recv", "%f",*floatp));
    return bufp;
}

/*
 * asn_build_float - builds an ASN object containing a single precision floating-point
 *                    number in an Opaque value.
 *
 *  On entry, datalength is input as the number of valid bytes following
 *   "data".  On exit, it is returned as the number of valid bytes
 *   following the end of this object.
 *
 *  Returns a pointer to the first byte past the end
 *   of this object (i.e. the start of the next object).
 *  Returns NULL on any error.

  u_char * asn_build_float(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      u_char      type         IN - asn type of object
      float      *floatp       IN - pointer to float
      int         floatsize    IN - size of input buffer
 */
u_char *
asn_build_float(u_char *data,
		size_t *datalength,
		u_char type,
		float *floatp,
		size_t floatsize)
{
    union {
        float  floatVal;
        int    intVal;
	u_char c[sizeof(float)];
    } fu;

    if (floatsize != sizeof (float)) {
	_asn_size_err("build float", floatsize, sizeof(float));
	return NULL;
    }
/* encode the float as an opaque */
    /* turn into Opaque holding special tagged value */

    /* put the tag and length for the Opaque wrapper */
    data = asn_build_header(data, datalength, ASN_OPAQUE, floatsize+3);
    if (_asn_build_header_check("build float", data, *datalength, (floatsize+3)))
	return NULL;

    /* put the special tag and length */
    *data++ = ASN_OPAQUE_TAG1;
    *data++ = ASN_OPAQUE_FLOAT;
    *data++ = (u_char)floatsize;
    *datalength = *datalength - 3;

    fu.floatVal = *floatp;
    /* correct for endian differences */
    fu.intVal = htonl(fu.intVal);	

    *datalength -= floatsize;
    memcpy(data, &fu.c[0], floatsize);

    data += floatsize;
    return data;
}

/*

  u_char * asn_parse_double(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      u_char     *type         OUT - asn type of object
      double     *doublep      IN/OUT - pointer to double
      int         doublesize   IN - size of output buffer
 */
u_char *
asn_parse_double(u_char *data,
		 size_t *datalength,
		 u_char *type,
		 double *doublep,
		 size_t doublesize)
{
    register u_char *bufp = data;
    u_long	    asn_length;
    long            tmp;
    union {
        double doubleVal;
        int    intVal[2];
	u_char c[sizeof(double)];
    } fu;
  

    if (doublesize != sizeof(double)){
	_asn_size_err("parse double", doublesize, sizeof(double));
	return NULL;
    }
    *type = *bufp++;
    bufp = asn_parse_length(bufp, &asn_length);
    if (_asn_parse_length_check("parse double", bufp, data,
                  asn_length, *datalength))
        return NULL;

    DEBUGDUMPSETUP("dump_recv", data, bufp - data + asn_length);
/* the double is encoded as an opaque */
    if ((*type == ASN_OPAQUE) &&
            (asn_length == ASN_OPAQUE_DOUBLE_BER_LEN) &&
	    (*bufp == ASN_OPAQUE_TAG1) &&
	    (*(bufp+1) == ASN_OPAQUE_DOUBLE)) {
      DEBUGMSG(("dump_recv", "Opaque %.2x %.2x: ", *bufp, *(bufp+1)));

        /* value is encoded as special format */
	bufp = asn_parse_length(bufp + 2, &asn_length);
	if (_asn_parse_length_check("parse opaque double", bufp, data,
                  asn_length, *datalength))
        return NULL;

	/* change type to Double */
	*type = ASN_OPAQUE_DOUBLE;
    }

    if (asn_length != sizeof(double)) {
	_asn_size_err("parse seq double", asn_length, sizeof(double));
	return NULL;
    }
    *datalength -= (int)asn_length + (bufp - data);
    memcpy(&fu.c[0], bufp, asn_length);

   /* correct for endian differences */

    tmp = ntohl(fu.intVal[0]);
    fu.intVal[0] = ntohl(fu.intVal[1]);
    fu.intVal[1] = tmp;
    	
    *doublep =  fu.doubleVal;
    DEBUGMSG(("dump_recv", "%d",*doublep));

    return bufp;
}

/*

  u_char * asn_build_double(
      u_char     *data         IN - pointer to start of object
      int        *datalength   IN/OUT - number of valid bytes left in buffer
      u_char      type         IN - asn type of object
      double     *doublep      IN - pointer to double
      int         doublesize   IN - size of input buffer
 */
u_char *
asn_build_double(u_char *data,
		 size_t *datalength,
		 u_char type,
		 double* doublep,
		 size_t doublesize)
{
    long  tmp;
    union {
        double doubleVal;
	int    intVal[2];
	u_char c[sizeof(double)];
    } fu;

    if (doublesize != sizeof(double)){
	_asn_size_err("build double", doublesize, sizeof(double));
	return NULL;
    }

/* encode the double as an opaque */
    /* turn into Opaque holding special tagged value */

    /* put the tag and length for the Opaque wrapper */
    data = asn_build_header(data, datalength, ASN_OPAQUE, doublesize+3);
    if (_asn_build_header_check("build double", data, *datalength, doublesize+3))
	return NULL;

    /* put the special tag and length */
    *data++ = ASN_OPAQUE_TAG1;
    *data++ = ASN_OPAQUE_DOUBLE;
    *data++ = (u_char)doublesize;
    *datalength = *datalength - 3;

    fu.doubleVal = *doublep;
    /* correct for endian differences */
    tmp = htonl(fu.intVal[0]);
    fu.intVal[0] = htonl(fu.intVal[1]);	
    fu.intVal[1] = tmp;
    *datalength -= doublesize;
    memcpy(data, &fu.c[0], doublesize);

    data += doublesize;
    return data;
}

#endif /* OPAQUE_SPECIAL_TYPES */

