//==========================================================================
//
//      ./lib/current/include/snmp.h
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
#ifndef SNMP_H
#define SNMP_H

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Definitions for the Simple Network Management Protocol (RFC 1067).
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


#define SNMP_PORT	    161         /* standard UDP port for SNMP agents
                                           to receive requests messages */
#define SNMP_TRAP_PORT	    162         /* standard UDP port for SNMP
                                           managers to receive notificaion
                                          (trap and inform) messages */

#define SNMP_MAX_LEN	    1500        /* typical maximum message size */
#define SNMP_MIN_MAX_LEN    484         /* minimum maximum message size */

/* SNMP versions */
/* There currently exists the following SNMP versions.
 * (Note that only SNMPv1 is in widespread usage, and this code supports
 *  only SNMPv1, SNMPv2c, and SNMPv3.
 *
 *  SNMPv1 - (full) the original version, defined by RFC 1157
 *  SNMPsec - (historic) the first attempt to add strong security
 *             to SNMPv1, defined by RFCs 1351, 1352, and 1353.
 *  SNMPv2p - (historic) party-based SNMP, which was another
 *             attempt to add strong security to SNMP, defined
 *             by RFCs 1441, 1445, 1446, 1448, and 1449.
 *  SNMPv2c - (experimental) community string-based SNMPv2,
 *             which was an attempt to combine the protocol
 *             operations of SNMPv2 with the security of
 *             SNMPv1, defined by RFCs 1901, 1905, and 1906.
 *  SNMPv2u - (experimental) user-based SNMPv2, which provided
 *             security based on user names and protocol
 *             operations of SNMPv2, defined by RFCs 1905,
 *             1909, and 1910.
 *  SNMPv2* (or SNMPv2star) - (experimental) an attempt to add the
 *             best features of SNMPv2p and SNMPv2u, defined
 *             by unpublished documents found at WEB site
 *             owned by SNMP Research (a leading SNMP vendor)
 *  SNMPv3 - the current attempt by the IETF working group to merge
 *             the SNMPv2u and SNMPv2* proposals into a more widly
 *             accepted SNMPv3.  It is defined by not yet published
 *             documents of the IETF SNMPv3 WG.
 *
 * SNMPv1, SNMPv2c, SNMPv2u, and SNMPv3 messages have a common
 * form, which is an ASN.1 sequence containing a message version 
 * field, followed by version dependent fields.
 * SNMPsec, SNMPv2p, and SNMPv2* messages have a common form,
 * which is a tagged ASN.1 context specific sequence containing
 * message dependent fields.
 *
 * In the #defines for the message versions below, the value
 * for SNMPv1, SNMPv2c, SNMPv2u, and SNMPv3 messages is the
 * value of the message version field. Since SNMPsec, SNMPv2p,
 * and SNMPv2* messages do not have a message version field,
 * the value in the defines for them is choosen to be a large
 * arbitrary number.
 *
 * Note that many of the version ID's are defined below purely for
 * documentational purposes.  At this point the only protocol planned
 * for future implementations is SNMP3, as the other v2 protocols will
 * not be supported by the IETF (ie, v2u, v2sec, v2star) or used by
 * the snmp community at large (at the time of this writing).  */

/* versions based on version field */
#define SNMP_VERSION_1	   0
#define SNMP_VERSION_2c    1
#define SNMP_VERSION_2u    2   /* not (will never be) supported by this code */
#define SNMP_VERSION_3     3   

/* versions not based on a version field */
#define SNMP_VERSION_sec   128 /* not (will never be) supported by this code */
#define SNMP_VERSION_2p	   129
#define SNMP_VERSION_2star 130 /* not (will never be) supported by this code */

/* PDU types in SNMPv1, SNMPsec, SNMPv2p, SNMPv2c, SNMPv2u, SNMPv2*, and SNMPv3 */
#define SNMP_MSG_GET	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x0)
#define SNMP_MSG_GETNEXT    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x1)
#define SNMP_MSG_RESPONSE   (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x2)
#define SNMP_MSG_SET	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x3)

/* PDU types in SNMPv1 and SNMPsec */
#define SNMP_MSG_TRAP	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x4)

/* PDU types in SNMPv2p, SNMPv2c, SNMPv2u, SNMPv2*, and SNMPv3 */
#define SNMP_MSG_GETBULK    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x5)
#define SNMP_MSG_INFORM	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x6)
#define SNMP_MSG_TRAP2	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x7)

/* PDU types in SNMPv2u, SNMPv2*, and SNMPv3 */
#define SNMP_MSG_REPORT	    (ASN_CONTEXT | ASN_CONSTRUCTOR | 0x8)

/* test for member of Confirmed Class i.e., reportable */
#define SNMP_CMD_CONFIRMED(c) (c == SNMP_MSG_INFORM || c == SNMP_MSG_GETBULK ||\
                               c == SNMP_MSG_GETNEXT || c == SNMP_MSG_GET || \
                               c == SNMP_MSG_SET)

/* Exception values for SNMPv2p, SNMPv2c, SNMPv2u, SNMPv2*, and SNMPv3 */
#define SNMP_NOSUCHOBJECT    (ASN_CONTEXT | ASN_PRIMITIVE | 0x0)
#define SNMP_NOSUCHINSTANCE  (ASN_CONTEXT | ASN_PRIMITIVE | 0x1)
#define SNMP_ENDOFMIBVIEW    (ASN_CONTEXT | ASN_PRIMITIVE | 0x2)

/* Error codes (the value of the field error-status in PDUs) */

/* in SNMPv1, SNMPsec, SNMPv2p, SNMPv2c, SNMPv2u, SNMPv2*, and SNMPv3 PDUs */
#define SNMP_ERR_NOERROR                (0)	/* XXX  Used only for PDUs? */
#define SNMP_ERR_TOOBIG	                (1)
#define SNMP_ERR_NOSUCHNAME             (2)
#define SNMP_ERR_BADVALUE               (3)
#define SNMP_ERR_READONLY               (4)
#define SNMP_ERR_GENERR	                (5)

/* in SNMPv2p, SNMPv2c, SNMPv2u, SNMPv2*, and SNMPv3 PDUs */
#define SNMP_ERR_NOACCESS		(6)
#define SNMP_ERR_WRONGTYPE		(7)
#define SNMP_ERR_WRONGLENGTH		(8)
#define SNMP_ERR_WRONGENCODING		(9)
#define SNMP_ERR_WRONGVALUE		(10)
#define SNMP_ERR_NOCREATION		(11)
#define SNMP_ERR_INCONSISTENTVALUE	(12)
#define SNMP_ERR_RESOURCEUNAVAILABLE	(13)
#define SNMP_ERR_COMMITFAILED		(14)
#define SNMP_ERR_UNDOFAILED		(15)
#define SNMP_ERR_AUTHORIZATIONERROR	(16)
#define SNMP_ERR_NOTWRITABLE		(17)

/* in SNMPv2c, SNMPv2u, SNMPv2*, and SNMPv3 PDUs */
#define SNMP_ERR_INCONSISTENTNAME	(18)

#define MAX_SNMP_ERR	18


/* values of the generic-trap field in trap PDUs */
#define SNMP_TRAP_COLDSTART		(0)
#define SNMP_TRAP_WARMSTART		(1)
#define SNMP_TRAP_LINKDOWN		(2)
#define SNMP_TRAP_LINKUP		(3)
#define SNMP_TRAP_AUTHFAIL		(4)
#define SNMP_TRAP_EGPNEIGHBORLOSS	(5)
#define SNMP_TRAP_ENTERPRISESPECIFIC	(6)

/* row status values */
#define SNMP_ROW_NONEXISTENT		0
#define SNMP_ROW_ACTIVE			1
#define SNMP_ROW_NOTINSERVICE		2
#define SNMP_ROW_NOTREADY		3
#define SNMP_ROW_CREATEANDGO		4
#define SNMP_ROW_CREATEANDWAIT		5
#define SNMP_ROW_DESTROY		6

/* row storage values */
#define SNMP_STORAGE_OTHER		1
#define SNMP_STORAGE_VOLATILE		2
#define SNMP_STORAGE_NONVOLATILE	3
#define SNMP_STORAGE_PERMANENT		4
#define SNMP_STORAGE_READONLY		5

/* message processing models */
#define SNMP_MP_MODEL_SNMPv1		0
#define SNMP_MP_MODEL_SNMPv2c		1
#define SNMP_MP_MODEL_SNMPv2u		2
#define SNMP_MP_MODEL_SNMPv3		3
#define SNMP_MP_MODEL_SNMPv2p		256

/* security values */
#define SNMP_SEC_MODEL_ANY		0
#define SNMP_SEC_MODEL_SNMPv1		1
#define SNMP_SEC_MODEL_SNMPv2c		2
#define SNMP_SEC_MODEL_USM		3
#define SNMP_SEC_MODEL_SNMPv2p		256

#define SNMP_SEC_LEVEL_NOAUTH		1
#define SNMP_SEC_LEVEL_AUTHNOPRIV	2
#define SNMP_SEC_LEVEL_AUTHPRIV		3

#define SNMP_MSG_FLAG_AUTH_BIT          0x01
#define SNMP_MSG_FLAG_PRIV_BIT          0x02
#define SNMP_MSG_FLAG_RPRT_BIT          0x04

	/* control PDU handling characteristics */
#define UCD_MSG_FLAG_RESPONSE_PDU            0x100
#define UCD_MSG_FLAG_EXPECT_RESPONSE         0x200
#define UCD_MSG_FLAG_FORCE_PDU_COPY          0x400
#define UCD_MSG_FLAG_ALWAYS_IN_VIEW          0x800

/* view status */
#define SNMP_VIEW_INCLUDED		1
#define SNMP_VIEW_EXCLUDED		2

/* basic oid values */
#define SNMP_OID_INTERNET		1, 3, 6, 1
#define SNMP_OID_ENTERPRISES		SNMP_OID_INTERNET, 4, 1
#define SNMP_OID_MIB2			SNMP_OID_INTERNET, 2, 1
#define SNMP_OID_SNMPV2			SNMP_OID_INTERNET, 6
#define SNMP_OID_SNMPMODULES		SNMP_OID_SNMPV2, 3

/* lengths as defined by TCs */
#define SNMPADMINLENGTH 255


#ifdef CMU_COMPATIBLE
/* PDU types in SNMPv1, SNMPsec, SNMPv2p, SNMPv2c, SNMPv2u, SNMPv2*, and SNMPv3 */
#define GET_REQ_MSG	    SNMP_MSG_GET
#define GETNEXT_REQ_MSG	    SNMP_MSG_GETNEXT
#define GET_RSP_MSG	    SNMP_MSG_RESPONSE
#define SET_REQ_MSG	    SNMP_MSG_SET

/* PDU types in SNMPv1 and SNMPsec */
#define TRP_REQ_MSG	    SNMP_MSG_TRAP

/* PDU types in SNMPv2p, SNMPv2c, SNMPv2u, SNMPv2*, and SNMPv3 */
#define BULK_REQ_MSG	    SNMP_MSG_GETBULK
#define INFORM_REQ_MSG	    SNMP_MSG_INFORM
#define TRP2_REQ_MSG	    SNMP_MSG_TRAP2

/* PDU types in SNMPv2u, SNMPv2*, and SNMPv3 */
#define REPORT_RSP_MSG	    SNMP_MSG_REPORT

/* since CMU V1.5 */

#define SNMP_PDU_GET	    SNMP_MSG_GET
#define SNMP_PDU_GETNEXT    SNMP_MSG_GETNEXT
#define SNMP_PDU_RESPONSE   SNMP_MSG_RESPONSE
#define SNMP_PDU_SET        SNMP_MSG_SET
#define SNMP_PDU_GETBULK    SNMP_MSG_GETBULK
#define SNMP_PDU_INFORM     SNMP_MSG_INFORM
#define SNMP_PDU_V2TRAP     SNMP_MSG_TRAP2
#define SNMP_PDU_REPORT     SNMP_MSG_REPORT

#define SNMP_TRAP_AUTHENTICATIONFAILURE SNMP_TRAP_AUTHFAIL

#define SMI_INTEGER     ASN_INTEGER
#define SMI_STRING      ASN_OCTET_STR
#define SMI_OBJID       ASN_OBJECT_ID
#define SMI_NULLOBJ     ASN_NULL
#define SMI_IPADDRESS   ASN_IPADDRESS
#define SMI_COUNTER32	    ASN_COUNTER
#define SMI_GAUGE32	    ASN_GAUGE
#define SMI_UNSIGNED32 SMI_GAUGE32
#define SMI_TIMETICKS   ASN_TIMETICKS
#define SMI_OPAQUE ASN_OPAQUE
#define SMI_COUNTER64   ASN_COUNTER64

int mib_TxtToOid (char *, oid **, size_t *);
int mib_OidToTxt (oid *, size_t , char *, size_t );

struct snmp_pdu;
char *snmp_pdu_type (struct snmp_pdu *);

struct snmp_session;
u_char * cmu_snmp_parse (struct snmp_session *session,
    struct snmp_pdu *pdu,
    u_char *data,
    size_t length);

#endif /* CMU_COMPATIBLE */

char *uptime_string (u_long, char *);
void xdump (const u_char *, size_t, const char *);
u_char *snmp_parse_var_op (u_char *, oid *, size_t *, u_char *, size_t *,
                               u_char **, size_t *);
u_char *snmp_build_var_op (u_char *, oid *, size_t *, u_char, size_t, u_char *,
                               size_t *);
#ifdef __cplusplus
}
#endif

#endif /* SNMP_H */
