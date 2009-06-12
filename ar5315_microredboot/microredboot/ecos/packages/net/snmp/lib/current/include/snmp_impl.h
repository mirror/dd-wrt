//==========================================================================
//
//      ./lib/current/include/snmp_impl.h
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
#ifndef SNMP_IMPL_H
#define SNMP_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif
/*
* file: snmp_impl.h
*/

/*
 * Definitions for SNMP implementation.
 *
 *
 */
/***********************************************************
	Copyright 1988, 1989 by Carnegie Mellon University

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

#include<stdio.h>

#define COMMUNITY_MAX_LEN	256

/* Space for character representation of an object identifier */
#define SPRINT_MAX_LEN		2560


#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

#define READ	    1
#define WRITE	    0

#define RESERVE1    0
#define RESERVE2    1
#define ACTION	    2
#define COMMIT      3
#define FREE        4
#define UNDO        5
#define FINISHED_SUCCESS        9
#define FINISHED_FAILURE	10

/* Access control statements for the agent */
#define RONLY	0x1	/* read access only */
#define RWRITE	0x2	/* read and write access (must have 0x2 bit set) */

#define NOACCESS 0x0000	/* no access for anybody */

/* defined types (from the SMI, RFC 1157) */
#define ASN_IPADDRESS   (ASN_APPLICATION | 0)
#define ASN_COUNTER	(ASN_APPLICATION | 1)
#define ASN_GAUGE	(ASN_APPLICATION | 2)
#define ASN_UNSIGNED    (ASN_APPLICATION | 2)  /* RFC 1902 - same as GAUGE */
#define ASN_TIMETICKS   (ASN_APPLICATION | 3)
#define ASN_OPAQUE	(ASN_APPLICATION | 4)  /* changed so no conflict with other includes */

/* defined types (from the SMI, RFC 1442) */
#define ASN_NSAP	(ASN_APPLICATION | 5)  /* historic - don't use */
#define ASN_COUNTER64   (ASN_APPLICATION | 6)
#define ASN_UINTEGER    (ASN_APPLICATION | 7)  /* historic - don't use */

#ifdef OPAQUE_SPECIAL_TYPES
/* defined types from draft-perkins-opaque-01.txt */
#define ASN_FLOAT	    (ASN_APPLICATION | 8)
#define ASN_DOUBLE	    (ASN_APPLICATION | 9)
#define ASN_INTEGER64        (ASN_APPLICATION | 10)
#define ASN_UNSIGNED64       (ASN_APPLICATION | 11)
#endif /* OPAQUE_SPECIAL_TYPES */

/* #define CMU_COMPATIBLE */
#ifdef CMU_COMPATIBLE
#define INTEGER	    ASN_INTEGER
#define STRING	    ASN_OCTET_STR
#define OBJID	    ASN_OBJECT_ID
#define NULLOBJ	    ASN_NULL
#define BITSTRING   ASN_BIT_STR  /* HISTORIC - don't use */
#define IPADDRESS   ASN_IPADDRESS
#define COUNTER	    ASN_COUNTER
#define GAUGE	    ASN_GAUGE
#define UNSIGNED    ASN_UNSIGNED
#define TIMETICKS   ASN_TIMETICKS
#define ASNT_OPAQUE ASN_OPAQUE
#define NSAP	    ASN_NSAP
#define COUNTER64   ASN_COUNTER64
#define UINTEGER    ASN_UINTEGER
#endif /* CMU_COMPATIBLE */

/* changed to ERROR_MSG to eliminate conflict with other includes */
#define ERROR_MSG(string)	snmp_set_detail(string)

/* from snmp.c */
extern u_char	sid[];	/* size SID_MAX_LEN */
extern int snmp_errno;


/*
 * For calling secauth_build, FIRST_PASS is an indication that a new nonce
 * and lastTimeStamp should be recorded.  LAST_PASS is an indication that
 * the packet should be checksummed and encrypted if applicable, in
 * preparation for transmission.
 * 0 means do neither, FIRST_PASS | LAST_PASS means do both.
 * For secauth_parse, FIRST_PASS means decrypt the packet, otherwise leave it
 * alone.  LAST_PASS is ignored.
 */
#define FIRST_PASS	1
#define	LAST_PASS	2
u_char	*snmp_comstr_parse(u_char *, size_t *, u_char *, size_t *, long *);
u_char	*snmp_comstr_build (u_char *, size_t *, u_char *, size_t *, long *, size_t);

int has_access (u_char, int, int, int);
#ifdef __cplusplus
}
#endif

#endif /* SNMP_IMPL_H */
