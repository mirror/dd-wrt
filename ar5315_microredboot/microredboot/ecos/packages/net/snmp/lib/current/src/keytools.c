//==========================================================================
//
//      ./lib/current/src/keytools.c
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
 * keytools.c
 */

#include <config.h>

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
#include <stdio.h>
#include <sys/types.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include "asn1.h"
#include "snmp_api.h"
#ifdef USE_OPENSSL
#	include <openssl/hmac.h>
#else 
#ifdef USE_INTERNAL_MD5
#include "md5.h"
#endif
#endif

#include "scapi.h"
#include "keytools.h"
#include "tools.h"
#include "snmp_debug.h"
#include "snmp_logging.h"

#include "transform_oids.h"

/*******************************************************************-o-******
 * generate_Ku
 *
 * Parameters:
 *	*hashtype	MIB OID for the transform type for hashing.
 *	 hashtype_len	Length of OID value.
 *	*P		Pre-allocated bytes of passpharase.
 *	 pplen		Length of passphrase.
 *	*Ku		Buffer to contain Ku.
 *	*kulen		Length of Ku buffer.
 *      
 * Returns:
 *	SNMPERR_SUCCESS			Success.
 *	SNMPERR_GENERR			All errors.
 *
 *
 * Convert a passphrase into a master user key, Ku, according to the
 * algorithm given in RFC 2274 concerning the SNMPv3 User Security Model (USM)
 * as follows:
 *
 * Expand the passphrase to fill the passphrase buffer space, if necessary,
 * concatenation as many duplicates as possible of P to itself.  If P is
 * larger than the buffer space, truncate it to fit.
 *
 * Then hash the result with the given hashtype transform.  Return
 * the result as Ku.
 *
 * If successful, kulen contains the size of the hash written to Ku.
 *
 * NOTE  Passphrases less than USM_LENGTH_P_MIN characters in length
 *	 cause an error to be returned.
 *	 (Punt this check to the cmdline apps?  XXX)
 */
int
generate_Ku(	oid	*hashtype,	u_int  hashtype_len,
		u_char	*P,		size_t  pplen,
		u_char	*Ku,		size_t *kulen)
#if defined(USE_INTERNAL_MD5) || defined(USE_OPENSSL)
{
	int		 rval   = SNMPERR_SUCCESS,
			 nbytes = USM_LENGTH_EXPANDED_PASSPHRASE;

        u_int            i, pindex = 0;

	u_char		 buf[USM_LENGTH_KU_HASHBLOCK],
			*bufp;

#ifdef USE_OPENSSL
	EVP_MD_CTX      *ctx = malloc(sizeof(EVP_MD_CTX));
#else
        MDstruct         MD;
#endif
	/*
	 * Sanity check.
	 */
	if ( !hashtype || !P || !Ku || !kulen
		|| (*kulen<=0)
		|| (hashtype_len != USM_LENGTH_OID_TRANSFORM) )
	{
		QUITFUN(SNMPERR_GENERR, generate_Ku_quit);
	}

        if (pplen < USM_LENGTH_P_MIN) {
#ifdef SNMP_TESTING_CODE
          snmp_log(LOG_WARNING, "Warning: passphrase chosen is below the length requiremnts of the USM.\n");
#else
          snmp_set_detail("Password length too short.");
          QUITFUN(SNMPERR_GENERR, generate_Ku_quit);
#endif
        }


	/*
	 * Setup for the transform type.
	 */
#ifdef USE_OPENSSL
	
	if (ISTRANSFORM(hashtype, HMACMD5Auth))
	  EVP_DigestInit(ctx, EVP_md5());
	else if (ISTRANSFORM(hashtype, HMACSHA1Auth))
	  EVP_DigestInit(ctx, EVP_sha1());
	else  {
	  free(ctx);
	  return (SNMPERR_GENERR);
	}
#else 
        MDbegin(&MD);
#endif /* USE_OPENSSL */

        while (nbytes > 0) {
                bufp = buf;
                for (i = 0; i < USM_LENGTH_KU_HASHBLOCK; i++) {
                        *bufp++ = P[pindex++ % pplen];
                }
#ifdef USE_OPENSSL
		EVP_DigestUpdate(ctx, buf, USM_LENGTH_KU_HASHBLOCK);
#else
                if (MDupdate(&MD, buf, USM_LENGTH_KU_HASHBLOCK*8)) {
                    rval = SNMPERR_USM_ENCRYPTIONERROR;
                    goto md5_fin;
                }
#endif /* USE_OPENSSL */

                nbytes -= USM_LENGTH_KU_HASHBLOCK;
        }

#ifdef USE_OPENSSL
	EVP_DigestFinal(ctx, (unsigned char *) Ku, (unsigned int *) kulen);
	/* what about free() */
#else
        if (MDupdate(&MD, buf, 0)) {
            rval = SNMPERR_USM_ENCRYPTIONERROR;
            goto md5_fin;
        }
        *kulen = sc_get_properlength(hashtype, hashtype_len);
        MDget(&MD, Ku, *kulen);
md5_fin:
        memset(&MD, 0, sizeof(MD));
#endif /* USE_OPENSSL */


#ifdef SNMP_TESTING_CODE
        DEBUGMSGTL(("generate_Ku", "generating Ku (from %s): ", P));
        for(i=0; i < *kulen; i++)
          DEBUGMSG(("generate_Ku", "%02x",Ku[i]));
        DEBUGMSG(("generate_Ku","\n"));
#endif /* SNMP_TESTING_CODE */


generate_Ku_quit:
	memset(buf, 0, sizeof(buf));
#ifdef USE_OPENSSL
	free(ctx);
#endif
	return rval;

}  /* end generate_Ku() */

#else
_KEYTOOLS_NOT_AVAILABLE
#endif						/* internal or openssl */




/*******************************************************************-o-******
 * generate_kul
 *
 * Parameters:
 *	*hashtype
 *	 hashtype_len
 *	*engineID
 *	 engineID_len
 *	*Ku		Master key for a given user.
 *	 ku_len		Length of Ku in bytes.
 *	*Kul		Localized key for a given user at engineID.
 *	*kul_len	Length of Kul buffer (IN); Length of Kul key (OUT).
 *      
 * Returns:
 *	SNMPERR_SUCCESS			Success.
 *	SNMPERR_GENERR			All errors.
 *
 *
 * Ku MUST be the proper length (currently fixed) for the given hashtype.
 *
 * Upon successful return, Kul contains the localized form of Ku at
 * engineID, and the length of the key is stored in kul_len.
 *
 * The localized key method is defined in RFC2274, Sections 2.6 and A.2, and
 * originally documented in:
 *  	U. Blumenthal, N. C. Hien, B. Wijnen,
 *     	"Key Derivation for Network Management Applications",
 *	IEEE Network Magazine, April/May issue, 1997.
 *
 *
 * ASSUMES  SNMP_MAXBUF >= sizeof(Ku + engineID + Ku).
 *
 * NOTE  Localized keys for privacy transforms are generated via
 *	 the authentication transform held by the same usmUser.
 *
 * XXX	An engineID of any length is accepted, even if larger than
 *	what is spec'ed for the textual convention.
 */
int
generate_kul(	oid	*hashtype,	u_int  hashtype_len,
		u_char	*engineID,	size_t  engineID_len,
		u_char	*Ku,		size_t  ku_len,
		u_char	*Kul,		size_t *kul_len)
#if defined(USE_OPENSSL) || defined(USE_INTERNAL_MD5)
{
	int		 rval    = SNMPERR_SUCCESS;
	u_int		 nbytes  = 0;
        size_t           properlength;

	u_char		 buf[SNMP_MAXBUF];
	void		*context = NULL;
#ifdef SNMP_TESTING_CODE
        int		 i;
#endif


	/*
	 * Sanity check.
	 */
	if ( !hashtype || !engineID || !Ku || !Kul || !kul_len
		|| (engineID_len<=0) || (ku_len<=0) || (*kul_len<=0)
		|| (hashtype_len != USM_LENGTH_OID_TRANSFORM) )
	{
		QUITFUN(SNMPERR_GENERR, generate_kul_quit);
	}


        properlength = sc_get_properlength(hashtype, hashtype_len);
        if (properlength == SNMPERR_GENERR)
          QUITFUN(SNMPERR_GENERR, generate_kul_quit);
       

	if (((int)*kul_len < properlength) || ((int)ku_len < properlength) ) {
		QUITFUN(SNMPERR_GENERR, generate_kul_quit);
	}

	/*
	 * Concatenate Ku and engineID properly, then hash the result.
	 * Store it in Kul.
	 */
	nbytes = 0;
	memcpy(buf,	   Ku,		properlength); nbytes += properlength;
	memcpy(buf+nbytes, engineID,	engineID_len); nbytes += engineID_len;
	memcpy(buf+nbytes, Ku,		properlength); nbytes += properlength;

	rval = sc_hash(hashtype, hashtype_len, buf, nbytes, Kul, kul_len);

#ifdef SNMP_TESTING_CODE
        DEBUGMSGTL(("generate_kul", "generating Kul (from Ku): "));
        for(i=0; i < *kul_len; i++)
          DEBUGMSG(("generate_kul", "%02x",Kul[i]));
        DEBUGMSG(("generate_kul", "keytools\n"));
#endif /* SNMP_TESTING_CODE */

	QUITFUN(rval, generate_kul_quit);
		

generate_kul_quit:
	SNMP_FREE(context);
	return rval;

}  /* end generate_kul() */

#else
_KEYTOOLS_NOT_AVAILABLE
#endif						/* internal or openssl */




/*******************************************************************-o-******
 * encode_keychange
 *
 * Parameters:
 *	*hashtype	MIB OID for the hash transform type.
 *	 hashtype_len	Length of the MIB OID hash transform type.
 *	*oldkey		Old key that is used to encodes the new key.
 *	 oldkey_len	Length of oldkey in bytes.
 *	*newkey		New key that is encoded using the old key.
 *	 newkey_len	Length of new key in bytes.
 *	*kcstring	Buffer to contain the KeyChange TC string.
 *	*kcstring_len	Length of kcstring buffer.
 *      
 * Returns:
 *	SNMPERR_SUCCESS			Success.
 *	SNMPERR_GENERR			All errors.
 *
 *
 * Uses oldkey and acquired random bytes to encode newkey into kcstring
 * according to the rules of the KeyChange TC described in RFC 2274, Section 5.
 *
 * Upon successful return, *kcstring_len contains the length of the
 * encoded string.
 *
 * ASSUMES	Old and new key are always equal to each other, although
 *		this may be less than the transform type hash output
 * 		output length (eg, using KeyChange for a DESPriv key when
 *		the user also uses SHA1Auth).  This also implies that the
 *		hash placed in the second 1/2 of the key change string
 *		will be truncated before the XOR'ing when the hash output is 
 *		larger than that 1/2 of the key change string.
 *
 *		*kcstring_len will be returned as exactly twice that same
 *		length though the input buffer may be larger.
 *
 * XXX FIX:     Does not handle varibable length keys.
 * XXX FIX:     Does not handle keys larger than the hash algorithm used.
 */
int
encode_keychange(	oid	*hashtype,	u_int  hashtype_len,
			u_char	*oldkey,	size_t  oldkey_len,
			u_char	*newkey,	size_t  newkey_len,
			u_char	*kcstring,	size_t *kcstring_len)
#if defined(USE_OPENSSL) || defined(USE_INTERNAL_MD5)
{
	int		 rval    = SNMPERR_SUCCESS;
	size_t		 properlength;
        size_t            nbytes  = 0;

        u_char          *tmpbuf = NULL;
	void		*context = NULL;


	/*
	 * Sanity check.
	 */
	if ( !hashtype || !oldkey || !newkey || !kcstring || !kcstring_len
		|| (oldkey_len<=0) || (newkey_len<=0) || (*kcstring_len<=0)
		|| (hashtype_len != USM_LENGTH_OID_TRANSFORM) )
	{
		QUITFUN(SNMPERR_GENERR, encode_keychange_quit);
	}

	/*
	 * Setup for the transform type.
	 */
        properlength = sc_get_properlength(hashtype, hashtype_len);
        if (properlength == SNMPERR_GENERR)
          QUITFUN(SNMPERR_GENERR, encode_keychange_quit);

	if ( (oldkey_len != newkey_len) || (*kcstring_len < (2*oldkey_len)) )
	{
		QUITFUN(SNMPERR_GENERR, encode_keychange_quit);
	}

	properlength = SNMP_MIN((int)oldkey_len, properlength);

	/*
	 * Use the old key and some random bytes to encode the new key
	 * in the KeyChange TC format:
	 *	. Get random bytes (store in first half of kcstring),
	 *	. Hash (oldkey | random_bytes) (into second half of kcstring),
	 *	. XOR hash and newkey (into second half of kcstring).
	 *
	 * Getting the wrong number of random bytes is considered an error.
	 */
	nbytes = properlength;

#if defined(SNMP_TESTING_CODE) && defined(RANDOMZEROS)
		memset(kcstring, 0, nbytes);
		DEBUGMSG(("encode_keychange",
                          "** Using all zero bits for \"random\" delta of )"
                          "the keychange string! **\n"));
#else /* !SNMP_TESTING_CODE */
		rval = sc_random(kcstring, &nbytes);
		QUITFUN(rval, encode_keychange_quit);
		if ((int)nbytes != properlength) {
			QUITFUN(SNMPERR_GENERR, encode_keychange_quit);
		}
#endif /* !SNMP_TESTING_CODE */

        tmpbuf = (u_char *)malloc(properlength*2);
        if (tmpbuf) {
            memcpy(tmpbuf, oldkey, properlength);
            memcpy(tmpbuf+properlength, kcstring, properlength);
    
            *kcstring_len -= properlength;
            rval = sc_hash(hashtype, hashtype_len, tmpbuf, properlength*2,
                           kcstring+properlength, kcstring_len);
            
            QUITFUN(rval, encode_keychange_quit);
    
            *kcstring_len = (properlength*2);
    
            kcstring += properlength;
            nbytes    = 0;
            while ((int)(nbytes++) < properlength) {
                u_char kcs = *kcstring;
            	*kcstring++ = kcs ^ *newkey++;
            }
        }

encode_keychange_quit:
	if (rval != SNMPERR_SUCCESS) memset(kcstring, 0, *kcstring_len);
        SNMP_FREE(tmpbuf);
	SNMP_FREE(context);

	return rval;

}  /* end encode_keychange() */

#else
_KEYTOOLS_NOT_AVAILABLE
#endif						/* internal or openssl */




/*******************************************************************-o-******
 * decode_keychange
 *
 * Parameters:
 *	*hashtype	MIB OID of the hash transform to use.
 *	 hashtype_len	Length of the hash transform MIB OID.
 *	*oldkey		Old key that is used to encode the new key.
 *	 oldkey_len	Length of oldkey in bytes.
 *	*kcstring	Encoded KeyString buffer containing the new key.
 *	 kcstring_len	Length of kcstring in bytes.
 *	*newkey		Buffer to hold the extracted new key.
 *	*newkey_len	Length of newkey in bytes.
 *      
 * Returns:
 *	SNMPERR_SUCCESS			Success.
 *	SNMPERR_GENERR			All errors.
 *
 *
 * Decodes a string of bits encoded according to the KeyChange TC described
 * in RFC 2274, Section 5.  The new key is extracted from *kcstring with
 * the aid of the old key.
 *
 * Upon successful return, *newkey_len contains the length of the new key.
 *
 *
 * ASSUMES	Old key is exactly 1/2 the length of the KeyChange buffer,
 *		although this length may be less than the hash transform
 *		output.  Thus the new key length will be equal to the old
 *		key length.
 */

/* XXX:  if the newkey is not long enough, it should be freed and remalloced */
int
decode_keychange(	oid	*hashtype,	u_int  hashtype_len,
			u_char	*oldkey,	size_t  oldkey_len,
			u_char	*kcstring,	size_t  kcstring_len,
			u_char	*newkey,	size_t *newkey_len)
#if defined(USE_OPENSSL) || defined(USE_INTERNAL_MD5)
{
	int		 rval    = SNMPERR_SUCCESS;
	size_t		 properlength = 0;
	u_int		 nbytes  = 0;

	u_char		*bufp,
			 tmp_buf[SNMP_MAXBUF];
        size_t           tmp_buf_len = SNMP_MAXBUF;
	void		*context = NULL;
        u_char          *tmpbuf = NULL;



	/*
	 * Sanity check.
	 */
	if ( !hashtype || !oldkey || !kcstring || !newkey || !newkey_len
		|| (oldkey_len<=0) || (kcstring_len<=0) || (*newkey_len<=0)
		|| (hashtype_len != USM_LENGTH_OID_TRANSFORM) )
	{
		QUITFUN(SNMPERR_GENERR, decode_keychange_quit);
	}


	/*
	 * Setup for the transform type.
	 */
        properlength = sc_get_properlength(hashtype, hashtype_len);
        if (properlength == SNMPERR_GENERR)
          QUITFUN(SNMPERR_GENERR, decode_keychange_quit);


	if ( ((oldkey_len*2) != kcstring_len) || (*newkey_len < oldkey_len) )
	{
		QUITFUN(SNMPERR_GENERR, decode_keychange_quit);
	}

	properlength = oldkey_len;
        *newkey_len = properlength;

	/*
	 * Use the old key and the given KeyChange TC string to recover
	 * the new key:
	 *	. Hash (oldkey | random_bytes) (into newkey),
	 *	. XOR hash and encoded (second) half of kcstring (into newkey).
	 */
        tmpbuf = (u_char *)malloc(properlength*2);
        if (tmpbuf) {
            memcpy(tmpbuf, oldkey, properlength);
            memcpy(tmpbuf+properlength, kcstring, properlength);
    
            rval = sc_hash(hashtype, hashtype_len, tmpbuf, properlength*2,
                           tmp_buf, &tmp_buf_len);
            QUITFUN(rval, decode_keychange_quit);
    
            memcpy(newkey, tmp_buf, properlength);
            bufp   = kcstring+properlength;
            nbytes = 0;
            while ((int)(nbytes++) < properlength) {
                    u_char nk = *newkey;
                    *newkey++ = nk ^ *bufp++;
            }
        }

decode_keychange_quit:
	if (rval != SNMPERR_SUCCESS) {
		memset(newkey, 0, properlength);
	}
	memset(tmp_buf, 0, SNMP_MAXBUF);
	SNMP_FREE(context);
        if (tmpbuf != NULL) SNMP_FREE(tmpbuf);

	return rval;

}  /* end decode_keychange() */

#else
_KEYTOOLS_NOT_AVAILABLE
#endif						/* internal or openssl */

#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */
