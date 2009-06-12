//==========================================================================
//
//      ./lib/current/include/scapi.h
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
 * scapi.h
 */

#ifndef _SCAPI_H
#define _SCAPI_H

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * Authentication/privacy transform bitlengths.
 */
#define SNMP_TRANS_AUTHLEN_HMACMD5	128
#define SNMP_TRANS_AUTHLEN_HMACSHA1	160

#define SNMP_TRANS_AUTHLEN_HMAC96	96

#define SNMP_TRANS_PRIVLEN_1DES		64
#define SNMP_TRANS_PRIVLEN_1DES_IV	64



/*
 * Prototypes.
 */
int sc_get_properlength(oid *hashtype, u_int hashtype_len);

int	sc_init (void);
int	sc_shutdown (int majorID, int minorID, void *serverarg,
                     void *clientarg);

int	sc_random (	u_char *buf, size_t *buflen);

int	sc_generate_keyed_hash (
		oid    *authtype,	size_t   authtypelen,
		u_char *key,		u_int keylen,
		u_char *message,	u_int msglen,
		u_char *MAC,		size_t *maclen);

int	sc_check_keyed_hash (
		oid    *authtype,	size_t   authtypelen,
		u_char *key,		u_int keylen,
		u_char *message,	u_int msglen,
		u_char *MAC,		u_int maclen);

int	sc_encrypt (	oid    *privtype,	size_t   privtypelen,
				u_char *key,		u_int keylen,
				u_char *iv,		u_int ivlen,
				u_char *plaintext,	u_int ptlen,
				u_char *ciphertext,	size_t *ctlen);

int	sc_decrypt (	oid    *privtype,	size_t   privtypelen,
				u_char *key,		u_int keylen,
				u_char *iv,		u_int ivlen,
				u_char *ciphertext,	u_int ctlen,
				u_char *plaintext,	size_t *ptlen);

int     sc_hash(oid *hashtype, size_t hashtypelen, u_char *buf, size_t buf_len,
                u_char *MAC, size_t *MAC_len);

int     sc_get_transform_type(oid *hashtype, u_int hashtype_len,
                              int (**hash_fn)(
                                const int	  mode,	  void 	   **context,
                                const u_char	 *data,	  const int  data_len,
                                u_char		**digest, size_t    *digest_len));
  

/*
 * All functions devolve to the following block if we can't do cryptography
 */
#define	_SCAPI_NOT_CONFIGURED			\
{						\
        DEBUGPL(("SCAPI not configured");      \
	return SNMPERR_SC_NOT_CONFIGURED;	\
}

/* define a transform type if we're using the internal md5 support */
#ifdef USE_INTERNAL_MD5
#define INTERNAL_MD5 1
#endif

#ifdef __cplusplus
}
#endif

#endif	/* _SCAPI_H */
