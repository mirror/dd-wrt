//==========================================================================
//
//      ./lib/current/src/snmp_api.c
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
/******************************************************************
	Copyright 1989, 1991, 1992 by Carnegie Mellon University

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
/*
 * snmp_api.c - API for access to snmp.
 */
#include <config.h>

#include <stdio.h>
#include <ctype.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
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
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>

#ifdef __ECOS
# ifdef AF_UNIX
#  undef AF_UNIX // eCos does not support local sockets
# endif // AF_UNIX
#endif // __ECOS

#endif
#if HAVE_SYS_UN_H
#include <sys/un.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_NET_IF_DL_H
#include <net/if_dl.h>
#endif
#include <errno.h>

#if HAVE_LOCALE_H
#include <locale.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include "asn1.h"
#include "snmp.h"
#define SNMP_NEED_REQUEST_LIST
#include "snmp_api.h"
#include "snmp_client.h"
#include "snmp_impl.h"
#include "parse.h"
#include "mib.h"
#include "system.h"
#include "int64.h"
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
#include "snmpv3.h"
#endif
#include "read_config.h"
#include "snmp_debug.h"
#include "callback.h"
#include "tools.h"
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
#include "snmpusm.h"
#include "keytools.h"
#include "lcd_time.h"
#endif
#include "snmp_alarm.h"
#include "snmp_logging.h"
#include "default_store.h"
#include "mt_support.h"

static void _init_snmp (void);

#include "transform_oids.h"
#ifndef timercmp
#define	timercmp(tvp, uvp, cmp) \
	/* CSTYLED */ \
	((tvp)->tv_sec cmp (uvp)->tv_sec || \
	((tvp)->tv_sec == (uvp)->tv_sec && \
	/* CSTYLED */ \
	(tvp)->tv_usec cmp (uvp)->tv_usec))
#endif
#ifndef timerclear
#define	timerclear(tvp)		(tvp)->tv_sec = (tvp)->tv_usec = 0
#endif

/*
 * Globals.
 */
#define PACKET_LENGTH	(8 * 1024)
#define MAX_PACKET_LENGTH	(32768)
#ifndef SNMP_STREAM_QUEUE_LEN
#define SNMP_STREAM_QUEUE_LEN  5
#endif

#ifndef BSD4_3
#define BSD4_2
#endif

#ifndef FD_SET

typedef long	fd_mask;
#define NFDBITS	(sizeof(fd_mask) * NBBY)	/* bits per mask */

#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	memset((p), 0, sizeof(*(p)))
#endif

static oid default_enterprise[] = {1, 3, 6, 1, 4, 1, 3, 1, 1};
/* enterprises.cmu.systems.cmuSNMP */

#define DEFAULT_COMMUNITY   "public"
#define DEFAULT_RETRIES	    5
#define DEFAULT_TIMEOUT	    1000000L
#define DEFAULT_REMPORT	    SNMP_PORT
#define DEFAULT_ENTERPRISE  default_enterprise
#define DEFAULT_TIME	    0

/*
 * Internal information about the state of the snmp session.
 */
struct snmp_internal_session {
    int	    sd;		/* socket descriptor for this connection */
    snmp_ipaddr  addr;	/* address of connected peer */
    snmp_ipaddr  me;	/* address of local socket */
    struct request_list *requests;/* Info about outstanding requests */
    struct request_list *requestsEnd; /* ptr to end of list */
    int (*hook_pre)  ( struct snmp_session*, snmp_ipaddr);
    int (*hook_parse)( struct snmp_session *, struct snmp_pdu *, u_char *, size_t);
    int (*hook_post) ( struct snmp_session*, struct snmp_pdu*, int );
    int (*hook_build)( struct snmp_session *, struct snmp_pdu *, u_char *, size_t *);
    int (*check_packet) ( u_char *, size_t );
    u_char *packet;
    long packet_len, proper_len;
    size_t packet_size;
    char newpkt;
};

/*
 * The list of active/open sessions.
 */
struct session_list {
    struct session_list *next;
    struct snmp_session *session;
    struct snmp_internal_session *internal;
};



static const char *api_errors[-SNMPERR_MAX+1] = {
    "No error",                            /* SNMPERR_SUCCESS */
    "Generic error",                       /* SNMPERR_GENERR */
    "Invalid local port",                  /* SNMPERR_BAD_LOCPORT */
    "Unknown host",                        /* SNMPERR_BAD_ADDRESS */
    "Unknown session",                     /* SNMPERR_BAD_SESSION */
    "Too long",                            /* SNMPERR_TOO_LONG */
    "No socket",                           /* SNMPERR_NO_SOCKET */
    "Cannot send V2 PDU on V1 session",    /* SNMPERR_V2_IN_V1 */
    "Cannot send V1 PDU on V2 session",    /* SNMPERR_V1_IN_V2 */
    "Bad value for non-repeaters",         /* SNMPERR_BAD_REPEATERS */
    "Bad value for max-repetitions",       /* SNMPERR_BAD_REPETITIONS */
    "Error building ASN.1 representation", /* SNMPERR_BAD_ASN1_BUILD */
    "Failure in sendto",                   /* SNMPERR_BAD_SENDTO */
    "Bad parse of ASN.1 type",             /* SNMPERR_BAD_PARSE */
    "Bad version specified",               /* SNMPERR_BAD_VERSION */
    "Bad source party specified",          /* SNMPERR_BAD_SRC_PARTY */
    "Bad destination party specified",     /* SNMPERR_BAD_DST_PARTY */
    "Bad context specified",               /* SNMPERR_BAD_CONTEXT */
    "Bad community specified",             /* SNMPERR_BAD_COMMUNITY */
    "Cannot send noAuth/desPriv",          /* SNMPERR_NOAUTH_DESPRIV */
    "Bad ACL definition",                  /* SNMPERR_BAD_ACL */
    "Bad Party definition",                /* SNMPERR_BAD_PARTY */
    "Session abort failure",               /* SNMPERR_ABORT */
    "Unknown PDU type",                    /* SNMPERR_UNKNOWN_PDU */
    "Timeout",                             /* SNMPERR_TIMEOUT */
    "Failure in recvfrom",                 /* SNMPERR_BAD_RECVFROM */
    "Unable to determine contextEngineID", /* SNMPERR_BAD_ENG_ID */
    "Unable to determine securityName",    /* SNMPERR_BAD_SEC_NAME */
    "Unable to determine securityLevel",   /* SNMPERR_BAD_SEC_LEVEL  */
    "ASN.1 parse error in message",        /* SNMPERR_ASN_PARSE_ERR */
    "Unknown security model in message",   /* SNMPERR_UNKNOWN_SEC_MODEL */
    "Invalid message (e.g. msgFlags)",     /* SNMPERR_INVALID_MSG */
    "Unknown engine ID",                   /* SNMPERR_UNKNOWN_ENG_ID */
    "Unknown user name",                   /* SNMPERR_UNKNOWN_USER_NAME */
    "Unsupported security level",          /* SNMPERR_UNSUPPORTED_SEC_LEVEL */
    "Authentication failure",              /* SNMPERR_AUTHENTICATION_FAILURE */
    "Not in time window",                  /* SNMPERR_NOT_IN_TIME_WINDOW */
    "Decryption error",                    /* SNMPERR_DECRYPTION_ERR */
    "SCAPI general failure",		   /* SNMPERR_SC_GENERAL_FAILURE */
    "SCAPI sub-system not configured",	   /* SNMPERR_SC_NOT_CONFIGURED */
    "Key tools not available",		   /* SNMPERR_KT_NOT_AVAILABLE */
    "Unknown Report message",	           /* SNMPERR_UNKNOWN_REPORT */
    "USM generic error",	           /* SNMPERR_USM_GENERICERROR */
    "USM unknown security name",           /* SNMPERR_USM_UNKNOWNSECURITYNAME */
    "USM unsupported security level",      /* SNMPERR_USM_UNSUPPORTEDSECURITYLEVEL */
    "USM encryption error",	           /* SNMPERR_USM_ENCRYPTIONERROR */
    "USM authentication failure",          /* SNMPERR_USM_AUTHENTICATIONFAILURE */
    "USM parse error",		           /* SNMPERR_USM_PARSEERROR */
    "USM unknown engineID",	           /* SNMPERR_USM_UNKNOWNENGINEID */
    "USM not in time window",	           /* SNMPERR_USM_NOTINTIMEWINDOW */
    "USM decryption error",	           /* SNMPERR_USM_DECRYPTIONERROR */
    "MIB not initialized",		   /* SNMPERR_NOMIB */
    "Value out of range",		   /* SNMPERR_RANGE */
    "Sub-id out of range",		   /* SNMPERR_MAX_SUBID */
    "Bad sub-id in object identifier",	   /* SNMPERR_BAD_SUBID */
    "Object identifier too long",	   /* SNMPERR_LONG_OID */
    "Bad value name",			   /* SNMPERR_BAD_NAME */
    "Bad value notation",		   /* SNMPERR_VALUE */
    "Unknown Object Identifier",	   /* SNMPERR_UNKNOWN_OBJID */
    "No PDU in snmp_send",		   /* SNMPERR_NULL_PDU */
    "Missing variables in PDU",		   /* SNMPERR_NO_VARS */
    "Bad variable type",		   /* SNMPERR_VAR_TYPE */
    "Out of memory (malloc failure)",	   /* SNMPERR_MALLOC */
};

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
static const char * usmSecLevelName[] =
	{
		"BAD_SEC_LEVEL",
		"noAuthNoPriv",
		"authNoPriv",
		"authPriv"
	};
#endif

/*
 * Multiple threads may changes these variables.
 * Suggest using the Single API, which does not use Sessions.
 *
 * Reqid may need to be protected. Time will tell...
 *
 */
/*MTCRITICAL_RESOURCE*/
/* use token in comments to individually protect these resources */
struct session_list	*Sessions	 = NULL; /* MT_LIB_SESSION */
static long		 Reqid		 = 0;    /* MT_LIB_REQUESTID */
static long		 Msgid		 = 0;    /* MT_LIB_MESSAGEID */
static long		 Sessid		 = 0;    /* MT_LIB_SESSIONID */
static long		 Transid	 = 0;    /* MT_LIB_TRANSID */
int              snmp_errno  = 0;
/*END MTCRITICAL_RESOURCE*/

/*struct timeval Now;*/

/*
 * global error detail storage
 */
static char snmp_detail[192];
static int  snmp_detail_f  = 0;

/*
 * Prototypes.
 */
int snmp_build (struct snmp_session *, struct snmp_pdu *, u_char *, size_t *);
static int snmp_parse (void *, struct snmp_session *, struct snmp_pdu *, u_char *, size_t);
static void * snmp_sess_pointer (struct snmp_session *);

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
static void snmpv3_calc_msg_flags (int, int, u_char *);
static int snmpv3_verify_msg (struct request_list *, struct snmp_pdu *);
static int snmpv3_build_probe_pdu (struct snmp_pdu **);
static int snmpv3_build (struct snmp_session *, struct snmp_pdu *, 
			     u_char *, size_t *);
#endif

static int snmp_parse_version (u_char *, size_t);
static int snmp_resend_request (struct session_list *slp, 
				struct request_list *rp, 
				int incr_retries);

#ifndef HAVE_STRERROR
const char *strerror(int err)
{
  extern const char *sys_errlist[];
  extern int sys_nerr;

  if (err < 0 || err >= sys_nerr) return "Unknown error";
  return sys_errlist[err];
}
#endif


long
snmp_get_next_reqid (void)
{ 
    long retVal;
    snmp_res_lock(MT_LIBRARY_ID, MT_LIB_REQUESTID);
    retVal = 1 + Reqid; /*MTCRITICAL_RESOURCE*/
    if (!retVal) retVal = 2;
    Reqid = retVal;
    snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_REQUESTID);
    return retVal;
}

long
snmp_get_next_msgid (void)
{
    long retVal;
    snmp_res_lock(MT_LIBRARY_ID, MT_LIB_MESSAGEID);
    retVal = 1 + Msgid; /*MTCRITICAL_RESOURCE*/
    if (!retVal) retVal = 2;
    Msgid = retVal;
    snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_MESSAGEID);
    return retVal;
}

long
snmp_get_next_sessid (void)
{ 
    long retVal;
    snmp_res_lock(MT_LIBRARY_ID, MT_LIB_SESSIONID);
    retVal = 1 + Sessid; /*MTCRITICAL_RESOURCE*/
    if (!retVal) retVal = 2;
    Sessid = retVal;
    snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_SESSIONID);
    return retVal;
}

long
snmp_get_next_transid (void)
{
    long retVal;
    snmp_res_lock(MT_LIBRARY_ID, MT_LIB_TRANSID);
    retVal = 1 + Transid; /*MTCRITICAL_RESOURCE*/
    if (!retVal) retVal = 2;
    Transid = retVal;
    snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_TRANSID);
    return retVal;
}

void
snmp_perror(const char *prog_string)
{
    const char *str;
    int xerr;
    xerr = snmp_errno; /*MTCRITICAL_RESOURCE*/
    str = snmp_api_errstring(xerr);
    snmp_log(LOG_ERR,"%s: %s\n",prog_string, str);
}

void
snmp_set_detail(const char *detail_string)
{
  if (detail_string != NULL) {
    strncpy((char *)snmp_detail, detail_string, sizeof(snmp_detail));
    snmp_detail[sizeof(snmp_detail)-1] = '\0';
    snmp_detail_f = 1;
  }
}

/* returns pointer to static data */
/* results not guaranteed in multi-threaded use */
const char *
snmp_api_errstring(int snmp_errnumber)
{
    const char *msg = "";
    static char msg_buf [256];
    if (snmp_errnumber >= SNMPERR_MAX && snmp_errnumber <= SNMPERR_GENERR){
        msg = api_errors[-snmp_errnumber];
    } else if (snmp_errnumber != SNMPERR_SUCCESS) {
        msg = "Unknown Error";
    }
    if (snmp_detail_f) {
        sprintf (msg_buf, "%s (%s)", msg, snmp_detail);
        snmp_detail_f = 0;
    }
    else
        strcpy(msg_buf,msg);

    return (msg_buf);
}

/*
 * snmp_error - return error data
 * Inputs :  address of errno, address of snmp_errno, address of string
 * Caller must free the string returned after use.
 */
void
snmp_error(struct snmp_session *psess,
	   int *p_errno,
	   int *p_snmp_errno,
	   char **p_str)
{
    char buf[SPRINT_MAX_LEN];
    int snmp_errnumber;

    if (p_errno) *p_errno = psess->s_errno;
    if (p_snmp_errno) *p_snmp_errno = psess->s_snmp_errno;
    if (p_str == NULL) return;

    strcpy(buf, "");
    snmp_errnumber = psess->s_snmp_errno;
    if (snmp_errnumber >= SNMPERR_MAX && snmp_errnumber <= SNMPERR_GENERR){
	strcpy(buf, api_errors[-snmp_errnumber]);
    } else {
	if (snmp_errnumber)
	sprintf(buf, "Unknown Error %d", snmp_errnumber);
    }

    /* append a useful system errno interpretation. */
    if (psess->s_errno)
        sprintf (&buf[strlen(buf)], " (%s)", strerror(psess->s_errno));
    *p_str = strdup(buf);
}

/*
 * snmp_sess_error - same as snmp_error for single session API use.
 */
void
snmp_sess_error(void *sessp,
		int *p_errno,
		int *p_snmp_errno,
		char **p_str)
{
    struct session_list *slp = (struct session_list*)sessp;

    if ((slp) && (slp->session))
	snmp_error(slp->session, p_errno, p_snmp_errno, p_str);
}

/* snmp_sess_perror(): print a error stored in a session pointer */ 
void
snmp_sess_perror(const char *prog_string, struct snmp_session *ss) {
  char *err;
  snmp_error(ss, NULL, NULL, &err);
  snmp_log(LOG_ERR, "%s: %s\n", prog_string, err);
  free(err);
}


/*
 * Primordial SNMP library initialization.
 * Initializes mutex locks.
 * Invokes minimum required initialization for displaying MIB objects.
 * Gets initial request ID for all transactions,
 * and finds which port SNMP over UDP uses.
 * SNMP over AppleTalk or IPX is not currently supported.
 *
 * Warning: no debug messages here.
 */
static void
_init_snmp (void)
{
#ifdef  HAVE_GETSERVBYNAME
    struct servent *servp;
#endif
    
    struct timeval tv;
    long tmpReqid, tmpMsgid;
    u_short s_port = SNMP_PORT;

    if (Reqid) return;
    Reqid = 1; /* quick set to avoid multiple inits */

    snmp_res_init();	/* initialize the mt locking structures */
    init_mib_internals();

    gettimeofday(&tv,(struct timezone *)0);
    /*Now = tv;*/

    /* get pseudo-random values for request ID and message ID */
    /* don't allow zero value to repeat init */
#ifdef SVR4
    srand48(tv.tv_sec ^ tv.tv_usec);
    tmpReqid = lrand48();
    tmpMsgid = lrand48();
#else
    srandom(tv.tv_sec ^ tv.tv_usec);
    tmpReqid = random();
    tmpMsgid = random();
#endif

    if (tmpReqid == 0) tmpReqid = 1;
    if (tmpMsgid == 0) tmpMsgid = 1;
    Reqid = tmpReqid;
    Msgid = tmpMsgid;

#ifdef HAVE_GETSERVBYNAME   
    servp = getservbyname("snmp", "udp");
    if (servp) {
      /* store it in host byte order */
      s_port = ntohs(servp->s_port);
    }
#endif
    ds_set_int(DS_LIBRARY_ID, DS_LIB_DEFAULT_PORT, s_port);
}

/*
 * Initializes the session structure.
 * May perform one time minimal library initialization.
 * No MIB file processing is done via this call.
 */
void
snmp_sess_init(struct snmp_session *session)
{
    _init_snmp();

    /* initialize session to default values */

    memset(session, 0, sizeof(struct snmp_session));
    session->remote_port = SNMP_DEFAULT_REMPORT;
    session->timeout = SNMP_DEFAULT_TIMEOUT;
    session->retries = SNMP_DEFAULT_RETRIES;
    session->version = SNMP_DEFAULT_VERSION;
}


void
register_default_handlers(void) {
  ds_register_config(ASN_BOOLEAN, "snmp","dumpPacket",
                     DS_LIBRARY_ID, DS_LIB_DUMP_PACKET);
  ds_register_config(ASN_INTEGER, "snmp","defaultPort",
                     DS_LIBRARY_ID, DS_LIB_DEFAULT_PORT);
  ds_register_config(ASN_OCTET_STR, "snmp","defCommunity",
		     DS_LIBRARY_ID, DS_LIB_COMMUNITY);
  ds_register_premib(ASN_BOOLEAN, "snmp", "noTokenWarnings",
                     DS_LIBRARY_ID, DS_LIB_NO_TOKEN_WARNINGS);
  ds_register_config(ASN_OCTET_STR, "snmp","noRangeCheck",
		     DS_LIBRARY_ID, DS_LIB_DONT_CHECK_RANGE );
}


/*******************************************************************-o-******
 * init_snmp
 *
 * Parameters:
 *      *type   Label for the config file "type" used by calling entity.
 *
 * Call appropriately the functions to do config file loading and
 * mib module parsing in the correct order.
 */
void
init_snmp(const char *type)
{
  static int	done_init = 0;	/* To prevent double init's. */

  if (done_init) {
    return;
  }
  
  done_init = 1;

  _init_snmp();

/* set our current locale properly to initialize isprint() type functions */
#ifdef HAVE_SETLOCALE
  setlocale(LC_CTYPE, "");
#endif

  snmp_debug_init(); /* should be done first, to turn on debugging ASAP */
  if ( type != NULL )
    ds_set_string(DS_LIBRARY_ID, DS_LIB_APPTYPE, type);
  init_callbacks();
  init_snmp_logging();
  snmp_init_statistics();
  register_mib_handlers();
  register_default_handlers();
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
  init_snmpv3(type);
#endif
  init_snmp_alarm();

  read_premib_configs();
  init_mib();

  read_configs();

}  /* end init_snmp() */

void
snmp_store(const char *type) {
  DEBUGMSGTL(("snmp_store","storing stuff...\n"));
  snmp_save_persistent(type);
  snmp_call_callbacks(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_STORE_DATA, NULL);
  snmp_clean_persistent(type);
}


/* snmp_shutdown(const char *type):

   Parameters:
        *type   Label for the config file "type" used by calling entity.

   Does the appropriate shutdown calls for the library, saving
   persistent data, clean up, etc...
*/
void
snmp_shutdown(const char *type) {
  snmp_store(type);
  snmp_call_callbacks(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_SHUTDOWN, NULL);
  snmp_close_sessions();
}


/*
 * Sets up the session with the snmp_session information provided
 * by the user.  Then opens and binds the necessary UDP port.
 * A handle to the created session is returned (this is different than
 * the pointer passed to snmp_open()).  On any error, NULL is returned
 * and snmp_errno is set to the appropriate error code.
 */
struct snmp_session *
snmp_open(struct snmp_session *session)
{
    struct session_list *slp;
    slp = (struct session_list *)snmp_sess_open(session);
    if (!slp) return NULL;

    snmp_res_lock(MT_LIBRARY_ID, MT_LIB_SESSION);
    slp->next = Sessions;
    Sessions = slp;
    snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_SESSION);

    return (slp->session);
}

/* extended open */
struct snmp_session *snmp_open_ex (
  struct snmp_session *session,
  int (*fpre_parse) (struct snmp_session *, snmp_ipaddr),
  int (*fparse) (struct snmp_session *, struct snmp_pdu *, u_char *, size_t),
  int (*fpost_parse) (struct snmp_session *, struct snmp_pdu *, int),
  int (*fbuild) (struct snmp_session *, struct snmp_pdu *, u_char *, size_t *),
  int (*fcheck) (u_char *, size_t )
)
{
    struct session_list *slp;
    slp = (struct session_list *)snmp_sess_open(session);
    if (!slp) return NULL;
    slp->internal->hook_pre = fpre_parse;
    slp->internal->hook_parse = fparse;
    slp->internal->hook_post = fpost_parse;
    slp->internal->hook_build = fbuild;
    slp->internal->check_packet = fcheck;

    snmp_res_lock(MT_LIBRARY_ID, MT_LIB_SESSION);
    slp->next = Sessions;
    Sessions = slp;
    snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_SESSION);

    return (slp->session);
}

static struct session_list *
_sess_copy( struct snmp_session *in_session)
{
    struct session_list *slp;
    struct snmp_internal_session *isp;
    struct snmp_session *session;
    char *cp;
    u_char *ucp;
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
    size_t i;
#endif

    in_session->s_snmp_errno = 0;
    in_session->s_errno = 0;

    /* Copy session structure and link into list */
    slp = (struct session_list *)calloc(1,sizeof(struct session_list));
    if (slp == NULL) { 
      in_session->s_snmp_errno = SNMPERR_MALLOC;
      return(NULL);
    }

    isp = (struct snmp_internal_session *)calloc(1,sizeof(struct snmp_internal_session));
    if (isp == NULL) { 
      snmp_sess_close(slp);
      in_session->s_snmp_errno = SNMPERR_MALLOC;
      return(NULL);
    }

    slp->internal = isp;
    slp->internal->sd = -1; /* mark it not set */
    slp->session = (struct snmp_session *)malloc(sizeof(struct snmp_session));
    if (slp->session == NULL) {
      snmp_sess_close(slp);
      in_session->s_snmp_errno = SNMPERR_MALLOC;
      return(NULL);
    }
    memmove(slp->session, in_session, sizeof(struct snmp_session));
    session = slp->session;

    /* zero out pointers so if we have to free the session we wont free mem
       owned by in_session */
    session->peername = NULL;
    session->community = NULL;
    session->contextEngineID = NULL;
    session->contextName = NULL;
    session->securityEngineID = NULL;
    session->securityName = NULL;
    session->securityAuthProto = NULL;
    session->securityPrivProto = NULL;
    /*
     * session now points to the new structure that still contains pointers to
     * data allocated elsewhere.  Some of this data is copied to space malloc'd
     * here, and the pointer replaced with the new one.
     */

    if (in_session->peername != NULL){
	session->peername = (char *)malloc(strlen(in_session->peername) + 1);
	if (session->peername == NULL) {
          snmp_sess_close(slp);
	  in_session->s_snmp_errno = SNMPERR_MALLOC;
          return(NULL);
        }
	strcpy(session->peername, in_session->peername);
    }

    /* Fill in defaults if necessary */
    if (in_session->community_len != SNMP_DEFAULT_COMMUNITY_LEN){
	ucp = (u_char *)malloc(in_session->community_len);
	if (ucp != NULL)
	    memmove(ucp, in_session->community, in_session->community_len);
    } else {
	if ((cp = ds_get_string(DS_LIBRARY_ID, DS_LIB_COMMUNITY)) != NULL) {
	    session->community_len = strlen(cp);
	    ucp = (u_char *)malloc(session->community_len);
	    if (ucp)
		memmove(ucp, cp, session->community_len);
	}
	else {
#ifdef NO_ZEROLENGTH_COMMUNITY
	    session->community_len = strlen(DEFAULT_COMMUNITY);
	    ucp = (u_char *)malloc(session->community_len);
	    if (ucp)
		memmove(ucp, DEFAULT_COMMUNITY, session->community_len);
#else
	    ucp = (u_char *)strdup("");
#endif
	}
    }

    if (ucp == NULL) {
      snmp_sess_close(slp);
      in_session->s_snmp_errno = SNMPERR_MALLOC;
      return(NULL);
    }
    session->community = ucp;	/* replace pointer with pointer to new data */

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
    if (session->securityLevel <= 0)
      session->securityLevel = ds_get_int(DS_LIBRARY_ID, DS_LIB_SECLEVEL);

    if (session->securityLevel == 0)
      session->securityLevel = SNMP_SEC_LEVEL_NOAUTH;

    if (in_session->securityAuthProtoLen > 0) {
      session->securityAuthProto = 
	(oid*)malloc(in_session->securityAuthProtoLen * sizeof(oid));
      if (session->securityAuthProto == NULL) {
	snmp_sess_close(slp);
	in_session->s_snmp_errno = SNMPERR_MALLOC;
	return(NULL);
      }
      memmove(session->securityAuthProto, in_session->securityAuthProto,
	      in_session->securityAuthProtoLen * sizeof(oid));
    } else if (get_default_authtype(&i) != NULL) {
        session->securityAuthProto =
          snmp_duplicate_objid(get_default_authtype(NULL), i);
        session->securityAuthProtoLen = i;
    }

    if (in_session->securityPrivProtoLen > 0) {
      session->securityPrivProto = 
	(oid*)malloc((unsigned)in_session->securityPrivProtoLen * sizeof(oid));
      if (session->securityPrivProto == NULL) {
	snmp_sess_close(slp);
	in_session->s_snmp_errno = SNMPERR_MALLOC;
	return(NULL);
      }
      memmove(session->securityPrivProto, in_session->securityPrivProto,
	      in_session->securityPrivProtoLen * sizeof(oid));
    } else if (get_default_privtype(&i) != NULL) {
        session->securityPrivProto =
          snmp_duplicate_objid(get_default_privtype(NULL), i);
        session->securityPrivProtoLen = i;
    }

    if (in_session->securityEngineIDLen > 0) {
      ucp = (u_char*)malloc((unsigned)in_session->securityEngineIDLen *
			   sizeof(u_char));
      if (ucp == NULL) {
	snmp_sess_close(slp);
	in_session->s_snmp_errno = SNMPERR_MALLOC;
	return(NULL);
      }
      memmove(ucp, in_session->securityEngineID,
	      in_session->securityEngineIDLen * sizeof(u_char));
      session->securityEngineID = ucp;

    }

    if (in_session->contextEngineIDLen > 0) {
      ucp = (u_char*)malloc((unsigned)in_session->contextEngineIDLen *
			   sizeof(u_char));
      if (ucp == NULL) {
	snmp_sess_close(slp);
	in_session->s_snmp_errno = SNMPERR_MALLOC;
	return(NULL);
      }
      memmove(ucp, in_session->contextEngineID,
	      in_session->contextEngineIDLen * sizeof(u_char));
      session->contextEngineID = ucp;
    } else if (in_session->securityEngineIDLen > 0) {
      /* default contextEngineID to securityEngineIDLen if defined */
      ucp = (u_char*)malloc((unsigned)in_session->securityEngineIDLen *
			   sizeof(u_char));
      if (ucp == NULL) {
	snmp_sess_close(slp);
	in_session->s_snmp_errno = SNMPERR_MALLOC;
	return(NULL);
      }
      memmove(ucp, in_session->securityEngineID,
	      in_session->securityEngineIDLen * sizeof(u_char));
      session->contextEngineID = ucp;
      session->contextEngineIDLen = in_session->securityEngineIDLen;
    }

    if (in_session->contextName) {
      session->contextName = strdup(in_session->contextName);
      if (session->contextName == NULL) {
	snmp_sess_close(slp);
	return(NULL);
      }
    } else if ((cp = ds_get_string(DS_LIBRARY_ID, DS_LIB_CONTEXT)) != NULL) {
      cp = strdup(cp);
      if (cp == NULL) {
	snmp_sess_close(slp);
	return(NULL);
      }
      session->contextName = cp;
      session->contextNameLen = strlen(cp);
    } else {
      cp = strdup(SNMP_DEFAULT_CONTEXT);
      session->contextName = cp;
      session->contextNameLen = strlen(cp);
    }

    if (in_session->securityName) {
      session->securityName = strdup(in_session->securityName);
      if (session->securityName == NULL) {
	snmp_sess_close(slp);
	return(NULL);
      }
    } else if ((cp = ds_get_string(DS_LIBRARY_ID, DS_LIB_SECNAME)) != NULL) {
      cp = strdup(cp);
      if (cp == NULL) {
	snmp_sess_close(slp);
	return(NULL);
      }
      session->securityName = cp;
      session->securityNameLen = strlen(cp);
    }

    if ((in_session->securityAuthKeyLen <= 0) &&
	(cp = ds_get_string(DS_LIBRARY_ID, DS_LIB_AUTHPASSPHRASE))) {
      session->securityAuthKeyLen = USM_AUTH_KU_LEN;
      if (generate_Ku(session->securityAuthProto,
                      session->securityAuthProtoLen,
                      (u_char*)cp, strlen(cp),
                      session->securityAuthKey,
                      &session->securityAuthKeyLen) != SNMPERR_SUCCESS) {
        snmp_set_detail("Error generating Ku from authentication pass phrase.");
	snmp_sess_close(slp);
        return NULL;
      }
    }

    if ((in_session->securityPrivKeyLen <= 0) && 
	       (cp = ds_get_string(DS_LIBRARY_ID, DS_LIB_PRIVPASSPHRASE))) {
      session->securityPrivKeyLen = USM_PRIV_KU_LEN;
      if (generate_Ku(session->securityAuthProto,
                      session->securityAuthProtoLen,
                      (u_char *)cp, strlen(cp),
                      session->securityPrivKey,
                      &session->securityPrivKeyLen) != SNMPERR_SUCCESS) {
        snmp_set_detail("Error generating Ku from privacy pass phrase.");
	snmp_sess_close(slp);
        return NULL;
      }
    }
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */

    if (session->retries == SNMP_DEFAULT_RETRIES)
	session->retries = DEFAULT_RETRIES;
    if (session->timeout == SNMP_DEFAULT_TIMEOUT)
	session->timeout = DEFAULT_TIMEOUT;
    session->sessid = snmp_get_next_sessid();

    return( slp );
}

struct session_list *
snmp_sess_copy( struct snmp_session *pss)
{
    struct session_list * psl;
    psl = _sess_copy(pss);
    if ( !psl) {
        if ( !pss->s_snmp_errno)
	        pss->s_snmp_errno = SNMPERR_GENERR;
	    SET_SNMP_ERROR(pss->s_snmp_errno);
    }
    return psl;
}

/*******************************************************************-o-******
 * snmp_sess_open
 *
 * Parameters:
 *	*in_session
 *
 * Returns:
 *      Pointer to a session in the session list   -OR-		FIX -- right?
 *	NULL on failure.
 *
 * The "spin-free" version of snmp_open.
 */
static void *
_sess_open(struct snmp_session *in_session)
{
    struct session_list *slp;
    struct snmp_internal_session *isp;
    struct snmp_session *session;
    int sd;
    in_addr_t addr;
    struct sockaddr_in *isp_addr, *meIp;
#ifdef HAVE_GETHOSTBYNAME
    struct hostent *hp;
#endif
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
    struct snmp_pdu *pdu, *response;
    int status;
#endif
    size_t i, addr_size;
    char *cp = NULL;

    in_session->s_snmp_errno = 0;
    in_session->s_errno = 0;

    if (Reqid == 0)
      _init_snmp();

    if ((slp = snmp_sess_copy( in_session )) == NULL )
        return( NULL );
    isp     = slp->internal;
    session = slp->session;

    if ( isp->addr.sa_family == AF_UNSPEC ) {
        if ( session->peername && session->peername[0] == '/' ) {
#ifdef AF_UNIX
            isp->addr.sa_family = AF_UNIX;
            strcpy( isp->addr.sa_data, session->peername);
#else /* AF_UNIX */
            snmp_log(LOG_ERR,"%s:%d: _sess_open invalid session name %s- unix sockets not supported  \n",
                    __FILE__,__LINE__,
                    session->peername);
            return(NULL);
#endif /* AF_UNIX */
            
        } else {
            isp->addr.sa_family = AF_INET;
            isp_addr = (struct sockaddr_in *)&(isp->addr);
            if (session->peername != SNMP_DEFAULT_PEERNAME) {

			/* Try and extract an appended port number */
		cp = strchr( session->peername, ':' );
		if ( cp ) {
		    *cp = '\0';
		    cp++;
		    session->remote_port = atoi( cp );
		    if ( session->local_port )	/* i.e. server */
			session->local_port = session->remote_port;
		}

			/* Interpret the peername as an IP port ... */
		cp = strchr( session->peername, '.' );
		if ( !cp && (( i = atoi( session->peername )) != 0 )) {
		    session->remote_port = i;
		    if ( session->local_port )	/* i.e. server */
			session->local_port = session->remote_port;
		}

			/* ... failing that, as an IP address ... */
                else if ((int)(addr = inet_addr(session->peername)) != -1){
                    memmove(&isp_addr->sin_addr, &addr, sizeof(isp_addr->sin_addr));
                } else {
			/* .... failing that, as a hostname */
#ifdef HAVE_GETHOSTBYNAME
                    hp = gethostbyname(session->peername);
                    if (hp == NULL){
                        in_session->s_snmp_errno = SNMPERR_BAD_ADDRESS;
                        in_session->s_errno = errno;
                        snmp_set_detail(session->peername);
                        snmp_sess_close(slp);
                        return 0;
                    } else {
                        memmove(&isp_addr->sin_addr, hp->h_addr, hp->h_length);
                    }

#else /* HAVE_GETHOSTBYNAME */
                    snmp_log(LOG_ERR,"%s:%d: _sess_open do not have get host by name - cannot resolve %s \n",
                            __FILE__,__LINE__,
                            session->peername);
                    return(0);
#endif /* HAVE_GETHOSTBYNAME */

                }
                if (session->remote_port == SNMP_DEFAULT_REMPORT){
                    short iport = ds_get_int(DS_LIBRARY_ID, DS_LIB_DEFAULT_PORT);
                    isp_addr->sin_port = htons(iport);
                } else {
                    isp_addr->sin_port = htons(session->remote_port);
                }
            } else {
                isp_addr->sin_addr.s_addr = SNMP_DEFAULT_ADDRESS;
            }
        }
    }


    if ( session->local_port ) {
	/*
	 *  If the session structure includes a non-null value for
	 *    local_port, then this session is intended as a server.
	 *    This means that the isp->addr structure will not be 
	 *    needed to contact a remote entity.
	 *
	 *  By using this address as the local address to bind to,
	 *    we can provide a facility for listening on selected
	 *    (rather than all) interfaces.
	 */
	memcpy( &isp->me, &isp->addr, sizeof(isp->me));

	if ( isp->addr.sa_family == AF_INET ) {
			/*
			 * Remember to use the specified local port,
			 *   rather than the (default?) remote one.
			 * If no local interface address is specified,
			 *   default to listening on all interfaces,
			 *   rather than the default connection host
			 *   (SNMP_DEFAULT_ADDRESS)
			 */
	    meIp = (struct sockaddr_in*)&(isp->me);
	    meIp->sin_port = htons(session->local_port);
            if (session->peername == SNMP_DEFAULT_PEERNAME)
                meIp->sin_addr.s_addr = INADDR_ANY;
	}
    }
    else {
        memset(&isp->me, '\0', sizeof(isp->me));
        isp->me.sa_family = isp->addr.sa_family;
        if ( isp->me.sa_family == AF_INET ) {
	    meIp = (struct sockaddr_in*)&(isp->me);
            meIp->sin_addr.s_addr = INADDR_ANY;
            meIp->sin_port = htons(session->local_port);
        }
#ifdef AF_UNIX
        else if ( isp->me.sa_family == AF_UNIX ) {
    		/* Need a unique socket name */
#ifndef UNIX_SOCKET_BASE_NAME
#define UNIX_SOCKET_BASE_NAME  "/tmp/s."
#endif
    
#ifndef WIN32
                strcpy( isp->me.sa_data, UNIX_SOCKET_BASE_NAME );
                strcat( isp->me.sa_data, "XXXXXX" );
                mktemp( isp->me.sa_data );
#endif
        }
#endif /* AF_UNIX */
    }
    addr_size = snmp_socket_length(isp->me.sa_family);

    /* Set up connections */
    if ( session->flags & SNMP_FLAGS_STREAM_SOCKET ) {
        if ( session->local_port != 0 )
            session->flags |= SNMP_FLAGS_LISTENING;
        sd = socket(isp->me.sa_family, SOCK_STREAM, 0);
    }
    else
        sd = socket(isp->me.sa_family, SOCK_DGRAM, 0);
    if (sd < 0){
	in_session->s_snmp_errno = SNMPERR_NO_SOCKET;
	in_session->s_errno = errno;
	snmp_set_detail(strerror(errno));
	snmp_sess_close(slp);
	return 0;
    }
    isp->sd = sd;

#ifdef SO_BSDCOMPAT
    /* Patch for Linux.  Without this, UDP packets that fail get an ICMP
     * response.  Linux turns the failed ICMP response into an error message
     * and return value, unlike all other OS's.
     */
    {
	int one=1;
	setsockopt(sd, SOL_SOCKET, SO_BSDCOMPAT, &one, sizeof(one));
    }
#endif /* SO_BSDCOMPAT */

//#ifndef __ECOS
#ifndef SERVER_REQUIRES_CLIENT_SOCKET
    if (!(( session->flags & SNMP_FLAGS_STREAM_SOCKET ) &&
#ifdef AF_UNIX
        ( isp->me.sa_family == AF_UNIX ) &&
#endif /* AF_UNIX */
        ( session->local_port == 0 ))) {

		/* Client Unix-domain stream sockets don't need to 'bind' */
#endif
        if (bind(sd, (struct sockaddr *)&isp->me, addr_size) != 0){
            in_session->s_snmp_errno = SNMPERR_BAD_LOCPORT;
            in_session->s_errno = errno;
            snmp_set_detail(strerror(errno));
            snmp_sess_close(slp);
            return 0;
        }
#ifndef SERVER_REQUIRES_CLIENT_SOCKET
    }
#endif
//#endif // not __ECOS

    if ( session->flags & SNMP_FLAGS_STREAM_SOCKET ) {
        if ( session->local_port == 0 ) {	/* Client session */

            if ( connect( sd, (struct sockaddr *)&(isp->addr),
                               snmp_socket_length(isp->addr.sa_family)) != 0 ) {
	        in_session->s_snmp_errno = SNMPERR_BAD_LOCPORT;
	        in_session->s_errno = errno;
	        snmp_set_detail(strerror(errno));
	        snmp_sess_close(slp);
	        return 0;
            }
        } else {				/* Server session */

            if ( listen( sd, SNMP_STREAM_QUEUE_LEN ) != 0 ) {
	        in_session->s_snmp_errno = SNMPERR_BAD_LOCPORT;
	        in_session->s_errno = errno;
	        snmp_set_detail(strerror(errno));
	        snmp_sess_close(slp);
	        return 0;
            }
        }
    }

    /* if we are opening a V3 session and we don't know engineID
       we must probe it - this must be done after the session is
       created and inserted in the list so that the response can
       handled correctly */

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT

    if (session->version == SNMP_VERSION_3) {
      if (session->securityEngineIDLen == 0 &&
          (session->securityEngineIDLen & SNMP_FLAGS_DONT_PROBE) !=
          SNMP_FLAGS_DONT_PROBE) {
	snmpv3_build_probe_pdu(&pdu);
	DEBUGMSGTL(("snmp_api","probing for engineID...\n"));
	status = snmp_sess_synch_response(slp, pdu, &response);

	if ((response == NULL) && (status == STAT_SUCCESS)) status = STAT_ERROR;

	switch (status) {
	case STAT_SUCCESS:
	  in_session->s_snmp_errno = SNMPERR_INVALID_MSG; /* XX?? */
	  DEBUGMSGTL(("snmp_sess_open",
                      "error: expected Report as response to probe: %s (%d)\n",
                      snmp_errstring(response->errstat), response->errstat));
	  break;
	case STAT_ERROR: /* this is what we expected -> Report == STAT_ERROR */
	  in_session->s_snmp_errno = SNMPERR_UNKNOWN_ENG_ID;
	  break; 
	case STAT_TIMEOUT:
	  in_session->s_snmp_errno = SNMPERR_TIMEOUT;
	default:
	  DEBUGMSGTL(("snmp_sess_open",
                      "unable to connect with remote engine: %s (%d)\n",
                      snmp_api_errstring(session->s_snmp_errno),
                      session->s_snmp_errno));
	  break;
	}
	if (slp->session->securityEngineIDLen == 0) {
	  DEBUGMSGTL(("snmp_api","unable to determine remote engine ID\n"));
	  snmp_sess_close(slp);
	  return NULL;
	}
	in_session->s_snmp_errno = SNMPERR_SUCCESS;
	if (snmp_get_do_debugging()) {
          DEBUGMSGTL(("snmp_sess_open", "  probe found engineID:  "));
	  for(i = 0; i < slp->session->securityEngineIDLen; i++)
	    DEBUGMSG(("snmp_sess_open", "%02x",
                      slp->session->securityEngineID[i]));
	  DEBUGMSG(("snmp_sess_open","\n"));
	}
      }
      /* if boot/time supplied set it for this engineID */
      if (session->engineBoots || session->engineTime) {
	set_enginetime(session->securityEngineID, session->securityEngineIDLen,
		       session->engineBoots, session->engineTime, TRUE);
      }
      if (create_user_from_session(slp->session) != SNMPERR_SUCCESS) {
	  in_session->s_snmp_errno = SNMPERR_UNKNOWN_USER_NAME; /* XX?? */
	DEBUGMSGTL(("snmp_api","snmp_sess_open(): failed(2) to create a new user from session\n"));
	  snmp_sess_close(slp);
	  return NULL;
      }
    }
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */


    return (void *)slp;
}  /* end snmp_sess_open() */

void *
snmp_sess_open(struct snmp_session *pss)
{
    void * pvoid;
    pvoid = _sess_open(pss);
    if ( !pvoid) {
        SET_SNMP_ERROR(pss->s_snmp_errno);
    }
    return pvoid;
}



#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
/* create_user_from_session(struct snmp_session *session):

   creates a user in the usm table from the information in a session

   Parameters:
        session -- IN: pointer to the session to use when creating the user.

   Returns:
        SNMPERR_SUCCESS
        SNMPERR_GENERR
*/
int
create_user_from_session(struct snmp_session *session)
{
  struct usmUser *user;


  /* now that we have the engineID, create an entry in the USM list
     for this user using the information in the session */
  user = usm_get_user_from_list(session->securityEngineID,
                                session->securityEngineIDLen,
                                session->securityName,
                                usm_get_userList(), 0);
  if (user == NULL) {
    DEBUGMSGTL(("snmp_api","Building user %s...\n",session->securityName));
    /* user doesn't exist so we create and add it */
    user = (struct usmUser *) calloc(1,sizeof(struct usmUser));
    if (user == NULL)
      return SNMPERR_GENERR;

    /* copy in the securityName */
    if (session->securityName) {
      user->name = strdup(session->securityName);
      user->secName = strdup(session->securityName);
      if (user->name == NULL || user->secName == NULL) {
        usm_free_user(user);
        return SNMPERR_GENERR;
      }
    }

    /* copy in the engineID */
    if (memdup(&user->engineID, session->securityEngineID,
               session->securityEngineIDLen) != SNMPERR_SUCCESS) {
      usm_free_user(user);
      return SNMPERR_GENERR;
    }
    user->engineIDLen = session->securityEngineIDLen;

    /* copy the auth protocol */
    if (session->securityAuthProto != NULL) {
      user->authProtocol =
        snmp_duplicate_objid(session->securityAuthProto,
                             session->securityAuthProtoLen);
      if (user->authProtocol == NULL) {
        usm_free_user(user);
        return SNMPERR_GENERR;
      }
      user->authProtocolLen = session->securityAuthProtoLen;
    }

    /* copy the priv protocol */
    if (session->securityPrivProto != NULL) {
      user->privProtocol =
        snmp_duplicate_objid(session->securityPrivProto,
                             session->securityPrivProtoLen);
      if (user->privProtocol == NULL) {
        usm_free_user(user);
        return SNMPERR_GENERR;
      }
      user->privProtocolLen = session->securityPrivProtoLen;
    }

    /* copy in the authentication Key, and convert to the localized version */
    if (session->securityAuthKey != NULL && session->securityAuthKeyLen != 0) {
      user->authKey = (u_char *)malloc (USM_LENGTH_KU_HASHBLOCK);
      user->authKeyLen = USM_LENGTH_KU_HASHBLOCK;
      if (generate_kul( user->authProtocol, user->authProtocolLen,
                        session->securityEngineID, session->securityEngineIDLen,
                        session->securityAuthKey, session->securityAuthKeyLen,
                        user->authKey, &user->authKeyLen ) != SNMPERR_SUCCESS) {
        usm_free_user(user);
        return SNMPERR_GENERR;
      }
    }

    /* copy in the privacy Key, and convert to the localized version */
    if (session->securityPrivKey != NULL && session->securityPrivKeyLen != 0) {
      user->privKey = (u_char *)malloc (USM_LENGTH_KU_HASHBLOCK);
      user->privKeyLen = USM_LENGTH_KU_HASHBLOCK;
      if (generate_kul( user->authProtocol, user->authProtocolLen,
                        session->securityEngineID, session->securityEngineIDLen,
                        session->securityPrivKey, session->securityPrivKeyLen,
                        user->privKey, &user->privKeyLen ) != SNMPERR_SUCCESS) {
        usm_free_user(user);
        return SNMPERR_GENERR;
      }
    }

    /* add the user into the database */
    usm_add_user(user);
  }

  return  SNMPERR_SUCCESS;


}  /* end create_user_from_session() */
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */

/*
 * Close the input session.  Frees all data allocated for the session,
 * dequeues any pending requests, and closes any sockets allocated for
 * the session.  Returns 0 on error, 1 otherwise.
 */
int
snmp_sess_close(void *sessp)
{
    struct session_list *slp = (struct session_list *)sessp;
    struct snmp_internal_session *isp;
    struct snmp_session *sesp;

    if (slp == NULL)
	return 0;

    isp = slp->internal; slp->internal = 0;
    if (isp) {
	struct request_list *rp, *orp;

        SNMP_FREE(isp->packet);

	if (isp->sd != -1)
	{
#ifndef HAVE_CLOSESOCKET
	    close(isp->sd);
#else
	    closesocket(isp->sd);
#endif
#ifdef AF_UNIX
            if ( isp->me.sa_family == AF_UNIX )
                unlink( isp->me.sa_data );
#endif /* AF_UNIX */
	}

	/* Free each element in the input request list.  */
	rp = isp->requests;
	while(rp){
	    orp = rp;
	    rp = rp->next_request;
		snmp_free_pdu(orp->pdu);
	    free((char *)orp);
	}

    	free((char *)isp);
    }

    sesp = slp->session; slp->session = 0;
    if (sesp) {
        SNMP_FREE(sesp->peername);
        SNMP_FREE(sesp->community);
        SNMP_FREE(sesp->contextEngineID);
        SNMP_FREE(sesp->contextName);
        SNMP_FREE(sesp->securityEngineID);
        SNMP_FREE(sesp->securityName);
        SNMP_FREE(sesp->securityAuthProto);
        SNMP_FREE(sesp->securityPrivProto);
        free((char *)sesp);
    }

    free((char *)slp);

    return 1;
}

int 
snmp_close(struct snmp_session *session)
{
    struct session_list *slp = NULL, *oslp = NULL;

    { /*MTCRITICAL_RESOURCE*/
	snmp_res_lock(MT_LIBRARY_ID, MT_LIB_SESSION);
    if (Sessions && Sessions->session == session){	/* If first entry */
	slp = Sessions;
	Sessions = slp->next;
    } else {
	for(slp = Sessions; slp; slp = slp->next){
	    if (slp->session == session){
		if (oslp)   /* if we found entry that points here */
		    oslp->next = slp->next;	/* link around this entry */
		break;
	    }
	    oslp = slp;
	}
    }
	snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_SESSION);
    } /*END MTCRITICAL_RESOURCE*/
    if (slp == NULL){
	return 0;
    }
    return snmp_sess_close((void *)slp);
}

int
snmp_close_sessions( void )
{
    struct session_list *slp;

    snmp_res_lock(MT_LIBRARY_ID, MT_LIB_SESSION);
    while ( Sessions ) {
        slp = Sessions;
        Sessions = Sessions->next;
        snmp_sess_close((void *)slp);
    }
    snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_SESSION);
    return 1;
}

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
static int
snmpv3_build_probe_pdu (struct snmp_pdu **pdu)
{
  struct usmUser *user;

  /* create the pdu */
  if (!pdu) return -1;
  *pdu = snmp_pdu_create(SNMP_MSG_GET);
  (*pdu)->version = SNMP_VERSION_3;
  (*pdu)->securityName = strdup("");
  (*pdu)->securityNameLen = strlen((*pdu)->securityName);
  (*pdu)->securityLevel = SNMP_SEC_LEVEL_NOAUTH;
  (*pdu)->securityModel = SNMP_SEC_MODEL_USM;

  /* create the empty user */
  user = usm_get_user(NULL, 0, (*pdu)->securityName);
  if (user == NULL) {
    user = (struct usmUser *) calloc(1,sizeof(struct usmUser));
    user->name = strdup((*pdu)->securityName);
    user->secName = strdup((*pdu)->securityName);
    user->authProtocolLen = sizeof(usmNoAuthProtocol)/sizeof(oid);
    user->authProtocol =
      snmp_duplicate_objid(usmNoAuthProtocol, user->authProtocolLen);
    user->privProtocolLen = sizeof(usmNoPrivProtocol)/sizeof(oid);
    user->privProtocol =
      snmp_duplicate_objid(usmNoPrivProtocol, user->privProtocolLen);
    usm_add_user(user);
  }
  return 0;
}

static void
snmpv3_calc_msg_flags (int sec_level, int msg_command, u_char *flags)
{
  *flags = 0;
  if (sec_level == SNMP_SEC_LEVEL_AUTHNOPRIV)
    *flags = SNMP_MSG_FLAG_AUTH_BIT;
  else if (sec_level == SNMP_SEC_LEVEL_AUTHPRIV)
    *flags = SNMP_MSG_FLAG_AUTH_BIT | SNMP_MSG_FLAG_PRIV_BIT;

  if (SNMP_CMD_CONFIRMED(msg_command)) *flags |= SNMP_MSG_FLAG_RPRT_BIT;

  return;
}

static int
snmpv3_verify_msg(struct request_list *rp, struct snmp_pdu *pdu)
{
  struct snmp_pdu     *rpdu;
  
  if (!rp || !rp->pdu || !pdu) return 0;
  /* Reports don't have to match anything according to the spec */
  if (pdu->command == SNMP_MSG_REPORT) return 1;
  rpdu = rp->pdu;
  if (rp->request_id != pdu->reqid || rpdu->reqid != pdu->reqid) return 0;
  if (rpdu->version != pdu->version) return 0;
  if (rpdu->securityModel != pdu->securityModel) return 0;
  if (rpdu->securityLevel != pdu->securityLevel) return 0;

  if (rpdu->contextEngineIDLen != pdu->contextEngineIDLen || 
      memcmp(rpdu->contextEngineID, pdu->contextEngineID, 
	     pdu->contextEngineIDLen))
    return 0;
  if (rpdu->contextNameLen != pdu->contextNameLen || 
      memcmp(rpdu->contextName, pdu->contextName, pdu->contextNameLen)) 
    return 0;
  if (rpdu->securityEngineIDLen != pdu->securityEngineIDLen || 
      memcmp(rpdu->securityEngineID, pdu->securityEngineID, 
	     pdu->securityEngineIDLen))
    return 0;
  if (rpdu->securityNameLen != pdu->securityNameLen || 
      memcmp(rpdu->securityName, pdu->securityName, pdu->securityNameLen)) 
    return 0;
  return 1;
}


/* SNMPv3
 * Takes a session and a pdu and serializes the ASN PDU into the area
 * pointed to by packet.  out_length is the size of the data area available.
 * Returns the length of the completed packet in out_length.  If any errors
 * occur, -1 is returned.  If all goes well, 0 is returned.
 */
static int
snmpv3_build(struct snmp_session	*session,
             struct snmp_pdu	        *pdu,
             u_char	        	*packet,
             size_t			*out_length)
{
  int ret;

    session->s_snmp_errno = 0;
    session->s_errno = 0;

	/* do validation for PDU types */
  switch (pdu->command) {
	case SNMP_MSG_RESPONSE:
	case SNMP_MSG_TRAP2:
	case SNMP_MSG_REPORT:
	    pdu->flags &= (~UCD_MSG_FLAG_EXPECT_RESPONSE);
	    /* Fallthrough */
	case SNMP_MSG_GET:
	case SNMP_MSG_GETNEXT:
	case SNMP_MSG_SET:
	case SNMP_MSG_INFORM:
	    if (pdu->errstat == SNMP_DEFAULT_ERRSTAT)
	        pdu->errstat = 0;
	    if (pdu->errindex == SNMP_DEFAULT_ERRINDEX)
	        pdu->errindex = 0;
	    break;

	case SNMP_MSG_GETBULK:
	    if (pdu->max_repetitions < 0) {
	        session->s_snmp_errno = SNMPERR_BAD_REPETITIONS;
	        return -1;
	    }
	    if (pdu->non_repeaters < 0) {
	        session->s_snmp_errno = SNMPERR_BAD_REPEATERS;
	        return -1;
	    }
	    break;

	case SNMP_MSG_TRAP:
	    session->s_snmp_errno = SNMPERR_V1_IN_V2;
	    return -1;
	
	default:
	    session->s_snmp_errno = SNMPERR_UNKNOWN_PDU;
	    return -1;
  }

      if (pdu->securityEngineIDLen == 0) {
	if (session->securityEngineIDLen) {
	  snmpv3_clone_engineID(&pdu->securityEngineID, 
				&pdu->securityEngineIDLen,
				session->securityEngineID,
				session->securityEngineIDLen);
	}
      }

      if (pdu->contextEngineIDLen == 0) {
	if (session->contextEngineIDLen) {
	  snmpv3_clone_engineID(&pdu->contextEngineID, 
				&pdu->contextEngineIDLen,
				session->contextEngineID,
				session->contextEngineIDLen);
	} else if (pdu->securityEngineIDLen) {
	  snmpv3_clone_engineID(&pdu->contextEngineID, 
				&pdu->contextEngineIDLen,
				pdu->securityEngineID,
				pdu->securityEngineIDLen);
	}
      }

      if (pdu->contextName == NULL) {
	if (!session->contextName){
	  session->s_snmp_errno = SNMPERR_BAD_CONTEXT;
	  return -1;
	}
	pdu->contextName = strdup(session->contextName);
	if (pdu->contextName == NULL) {
	  session->s_snmp_errno = SNMPERR_GENERR;
	  return -1;
	}
	pdu->contextNameLen = session->contextNameLen;
      }
      pdu->securityModel = SNMP_SEC_MODEL_USM;
      if (pdu->securityNameLen == 0 && pdu->securityName == 0) {
	if (session->securityNameLen == 0){
	  session->s_snmp_errno = SNMPERR_BAD_SEC_NAME;
	  return -1;
	}
	pdu->securityName = strdup(session->securityName);
	if (pdu->securityName == NULL) {
	  session->s_snmp_errno = SNMPERR_GENERR;
	  return -1;
	}
	pdu->securityNameLen = session->securityNameLen;
      }
      if (pdu->securityLevel == 0) {
	if (session->securityLevel == 0) {
	    session->s_snmp_errno = SNMPERR_BAD_SEC_LEVEL;
	    return -1;
	}
	pdu->securityLevel = session->securityLevel;
      }
      DEBUGMSGTL(("snmp_build",
                  "Building SNMPv3 message (secName:\"%s\", secLevel:%s)...\n",
                  ((session->securityName) ? (char *)session->securityName :
                   ((pdu->securityName) ? (char *)pdu->securityName : 
                    "ERROR: undefined")),
                  usmSecLevelName[pdu->securityLevel]));

  ret = snmpv3_packet_build(pdu, packet, out_length, NULL, 0);
  if (-1 != ret) {
      session->s_snmp_errno = ret;
  }

  return ret;

}  /* end snmpv3_build() */




static u_char *
snmpv3_header_build(struct snmp_pdu *pdu, u_char *packet,
                    size_t *out_length, size_t length, u_char **msg_hdr_e)

{
    u_char			*global_hdr, *global_hdr_e;
    u_char 			*cp;
    u_char			 msg_flags;
    long			 max_size;
    long			 sec_model;
    u_char			*pb, *pb0e;

    /* Save current location and build SEQUENCE tag and length placeholder
     * for SNMP message sequence (actual length inserted later)
     */
    cp = asn_build_sequence(packet, out_length,
			    (u_char)(ASN_SEQUENCE | ASN_CONSTRUCTOR), length);
    if (cp == NULL) return NULL;
    if (msg_hdr_e != NULL)
      *msg_hdr_e = cp;
    pb0e = cp;


    /* store the version field - msgVersion
     */
    cp = asn_build_int(cp, out_length,
		       (u_char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
		       (long *) &pdu->version, sizeof(pdu->version));
    if (cp == NULL) return NULL;

    global_hdr = cp;
    /* msgGlobalData HeaderData */
    cp = asn_build_sequence(cp, out_length,
			    (u_char)(ASN_SEQUENCE | ASN_CONSTRUCTOR), 0);
    if (cp == NULL) return NULL;
    global_hdr_e = cp;


    /* msgID */
    cp = asn_build_int(cp, out_length,
		       (u_char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
		       &pdu->msgid, sizeof(pdu->msgid));
    if (cp == NULL) return NULL;

    							/* msgMaxSize */
    max_size = SNMP_MAX_MSG_SIZE;
    cp = asn_build_int(cp, out_length,
		       (u_char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
		       &max_size, sizeof(max_size));
    if (cp == NULL) return NULL;

    /* msgFlags */
    snmpv3_calc_msg_flags(pdu->securityLevel, pdu->command, &msg_flags);
    cp = asn_build_string(cp, out_length,
			  (u_char)(ASN_UNIVERSAL|ASN_PRIMITIVE|ASN_OCTET_STR),
			  &msg_flags, sizeof(msg_flags));
    if (cp == NULL) return NULL;

    							/* msgSecurityModel */
    sec_model = SNMP_SEC_MODEL_USM;
    cp = asn_build_int(cp, out_length,
		       (u_char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
		       &sec_model, sizeof(sec_model));
    if (cp == NULL) return NULL;


    /* insert actual length of globalData
     */
    pb = asn_build_sequence(global_hdr, out_length,
                            (u_char)(ASN_SEQUENCE | ASN_CONSTRUCTOR),
                            cp - global_hdr_e);
    if (pb == NULL) return NULL;


    /* insert the actual length of the entire packet
     */
    pb = asn_build_sequence(packet, out_length,
			    (u_char)(ASN_SEQUENCE | ASN_CONSTRUCTOR),
                            length + (cp - pb0e));
    if (pb == NULL) return NULL;

    return cp;

}  /* end snmpv3_header_build() */



static u_char *
snmpv3_scopedPDU_header_build(struct snmp_pdu *pdu,
                              u_char *packet, size_t *out_length,
                              u_char **spdu_e)

{
  size_t	 init_length;
  u_char	*scopedPdu, *pb;


  init_length = *out_length;

  pb = scopedPdu = packet;
  pb = asn_build_sequence(pb, out_length,
                          (u_char)(ASN_SEQUENCE | ASN_CONSTRUCTOR), 0);
  if (pb == NULL) return NULL;
  if (spdu_e)
    *spdu_e = pb;

  pb = asn_build_string(pb, out_length,
                        (ASN_UNIVERSAL|ASN_PRIMITIVE|ASN_OCTET_STR),
                        pdu->contextEngineID, pdu->contextEngineIDLen);
  if (pb == NULL) return NULL;

  pb = asn_build_string(pb, out_length,
                        (ASN_UNIVERSAL|ASN_PRIMITIVE|ASN_OCTET_STR),
                        (u_char *)pdu->contextName, pdu->contextNameLen);
  if (pb == NULL) return NULL;

  return pb;

}  /* end snmpv3_scopedPDU_header_build() */


/* returns 0 if success, -1 if fail, not 0 if USM build failure */
int
snmpv3_packet_build(struct snmp_pdu *pdu, u_char *packet, size_t *out_length,
		    u_char *pdu_data, size_t pdu_data_len)
{
    u_char	*global_data,		*sec_params,	*spdu_hdr_e;
    size_t	 global_data_len,	 sec_params_len;
    u_char	 spdu_buf[SNMP_MAX_MSG_SIZE];
    size_t	 spdu_buf_len, spdu_len;
    u_char	*cp;
    int      result;

    global_data = packet;

    /* 
     * build the headers for the packet, returned addr = start of secParams
     */
    sec_params = snmpv3_header_build(pdu, global_data, out_length, 0, NULL);
    if (sec_params == NULL) return -1;
    global_data_len = sec_params - global_data;
    sec_params_len = *out_length; /* length left in packet buf for sec_params */


    /* 
     * build a scopedPDU structure into spdu_buf
     */
    spdu_buf_len = SNMP_MAX_MSG_SIZE;
    cp = snmpv3_scopedPDU_header_build(pdu,spdu_buf,&spdu_buf_len,&spdu_hdr_e);
    if (cp == NULL) return -1;

    /* build the PDU structure onto the end of spdu_buf 
     */
    if (pdu_data) {
      memcpy(cp, pdu_data, pdu_data_len);
      cp += pdu_data_len;
    } else {
      cp = snmp_pdu_build(pdu, cp, &spdu_buf_len);
      if (cp == NULL) return -1;
    }


    /* 
     * re-encode the actual ASN.1 length of the scopedPdu
     */
    spdu_len = cp - spdu_hdr_e; /* length of scopedPdu minus ASN.1 headers */
    spdu_buf_len = SNMP_MAX_MSG_SIZE;
    if (asn_build_sequence(spdu_buf, &spdu_buf_len,
                           (u_char)(ASN_SEQUENCE | ASN_CONSTRUCTOR),
                           spdu_len) == NULL)
      return -1;
    spdu_len = cp - spdu_buf;	/* the length of the entire scopedPdu */


    /* 
     * call the security module to possibly encrypt and authenticate the
     * message - the entire message to transmitted on the wire is returned
     */
    cp = NULL; *out_length = SNMP_MAX_MSG_SIZE;
    result =
     	usm_generate_out_msg(
			SNMP_VERSION_3,		
			global_data,		global_data_len,
                        SNMP_MAX_MSG_SIZE,	
			SNMP_SEC_MODEL_USM,
                        pdu->securityEngineID,	pdu->securityEngineIDLen,
                        pdu->securityName,	pdu->securityNameLen,
                        pdu->securityLevel,	
			spdu_buf,		spdu_len, 
			pdu->securityStateRef,
			sec_params,		&sec_params_len,
                        &cp,			out_length);

    return result;

}  /* end snmpv3_packet_build() */
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */


/*
 * Takes a session and a pdu and serializes the ASN PDU into the area
 * pointed to by packet.  out_length is the size of the data area available.
 * Returns the length of the completed packet in out_length.  If any errors
 * occur, -1 is returned.  If all goes well, 0 is returned.
 */
static int
_snmp_build(struct snmp_session *session,
	   struct snmp_pdu *pdu,
	   u_char *packet,
	   size_t *out_length)
{
    u_char *h0, *h0e = 0, *h1;
    u_char  *cp;
    size_t length;
    long version;

    session->s_snmp_errno = 0;
    session->s_errno = 0;

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
    if (pdu->version == SNMP_VERSION_3)
      return snmpv3_build(session, pdu, packet, out_length);
#endif

    switch (pdu->command) {
	case SNMP_MSG_RESPONSE:
	    pdu->flags &= (~UCD_MSG_FLAG_EXPECT_RESPONSE);
		/* Fallthrough */
	case SNMP_MSG_GET:
	case SNMP_MSG_GETNEXT:
	case SNMP_MSG_SET:
            /* all versions support these PDU types */
            /* initialize defaulted PDU fields */

	    if (pdu->errstat == SNMP_DEFAULT_ERRSTAT)
	        pdu->errstat = 0;
	    if (pdu->errindex == SNMP_DEFAULT_ERRINDEX)
	        pdu->errindex = 0;
	    break;

	case SNMP_MSG_TRAP2:
	    pdu->flags &= (~UCD_MSG_FLAG_EXPECT_RESPONSE);
		/* Fallthrough */
	case SNMP_MSG_INFORM:
            /* not supported in SNMPv1 and SNMPsec */
	    if (pdu->version == SNMP_VERSION_1) {
	            session->s_snmp_errno = SNMPERR_V2_IN_V1;
	            return -1;
	    }
	    if (pdu->errstat == SNMP_DEFAULT_ERRSTAT)
	        pdu->errstat = 0;
	    if (pdu->errindex == SNMP_DEFAULT_ERRINDEX)
	        pdu->errindex = 0;
	    break;

	case SNMP_MSG_GETBULK:
            /* not supported in SNMPv1 and SNMPsec */
	    if (pdu->version == SNMP_VERSION_1) {
	            session->s_snmp_errno = SNMPERR_V2_IN_V1;
	            return -1;
            }
	    if (pdu->max_repetitions < 0) {
	        session->s_snmp_errno = SNMPERR_BAD_REPETITIONS;
	        return -1;
	    }
	    if (pdu->non_repeaters < 0) {
	        session->s_snmp_errno = SNMPERR_BAD_REPEATERS;
	        return -1;
	    }
	    break;

	case SNMP_MSG_TRAP:
            /* *only* supported in SNMPv1 and SNMPsec */
	    if (pdu->version != SNMP_VERSION_1) {
	            session->s_snmp_errno = SNMPERR_V1_IN_V2;
	            return -1;
            }
            /* initialize defaulted Trap PDU fields */
	    pdu->reqid = 1;	/* give a bogus non-error reqid for traps */
	    if (pdu->enterprise_length == SNMP_DEFAULT_ENTERPRISE_LENGTH){
	        pdu->enterprise = (oid *)malloc(sizeof(DEFAULT_ENTERPRISE));
	        memmove(pdu->enterprise, DEFAULT_ENTERPRISE,
		    sizeof(DEFAULT_ENTERPRISE));
	        pdu->enterprise_length = sizeof(DEFAULT_ENTERPRISE)/sizeof(oid);
	    }
	    if (pdu->time == SNMP_DEFAULT_TIME)
	        pdu->time = DEFAULT_TIME;
            /* don't expect a response */
	    pdu->flags &= (~UCD_MSG_FLAG_EXPECT_RESPONSE);
	    break;

	case SNMP_MSG_REPORT:		/* SNMPv3 only */
	default:
            session->s_snmp_errno = SNMPERR_UNKNOWN_PDU;
            return -1;
    }

    /* save length */
    length = *out_length;

    /* setup administrative fields based on version */
    /* build the message wrapper and all the administrative fields
       upto the PDU sequence
       (note that actual length of message will be inserted later) */
    h0 = packet;
    switch (pdu->version) {
    case SNMP_VERSION_1:
    case SNMP_VERSION_2c:
#ifdef NO_ZEROLENGTH_COMMUNITY
	if (pdu->community_len == 0){
	    if (session->community_len == 0){
		session->s_snmp_errno = SNMPERR_BAD_ADDRESS;
		return -1;
	    }
	    pdu->community = (u_char *)malloc(session->community_len);
	    memmove(pdu->community, session->community,
                        session->community_len);
	    pdu->community_len = session->community_len;
	}
#else /* !NO_ZEROLENGTH_COMMUNITY */
	if (! (pdu->community_len != 0 &&
	       pdu->command == SNMP_MSG_RESPONSE )) {
	/* copy session community exactly to pdu community */
	    if (0 == session->community_len) {
		SNMP_FREE(pdu->community); pdu->community = 0;
	    }
	    else if (pdu->community_len == session->community_len) {
		memmove(pdu->community, session->community,
			    session->community_len);
	    }
	    else {
	    SNMP_FREE(pdu->community);
	    pdu->community = (u_char *)malloc(session->community_len);
	    memmove(pdu->community, session->community,
                        session->community_len);
	    }
	    pdu->community_len = session->community_len;
	}
#endif /* !NO_ZEROLENGTH_COMMUNITY */

        DEBUGMSGTL(("snmp_send","Building SNMPv%d message...\n", (1 + pdu->version)));
        /* Save current location and build SEQUENCE tag and length
           placeholder for SNMP message sequence
          (actual length will be inserted later) */
        cp = asn_build_sequence(packet, out_length,
                                (u_char)(ASN_SEQUENCE | ASN_CONSTRUCTOR),
                                0);
        if (cp == NULL)
            return -1;
        h0e = cp;

        /* store the version field */
        version = pdu->version;
        cp = asn_build_int(cp, out_length,
                    (u_char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
                    (long *) &version, sizeof(version));
        if (cp == NULL)
            return -1;

        /* store the community string */
        cp = asn_build_string(cp, out_length,
                    (u_char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OCTET_STR),
                    pdu->community, pdu->community_len);
        if (cp == NULL)
            return -1;
        break;

    case SNMP_VERSION_2p:
    case SNMP_VERSION_sec:
    case SNMP_VERSION_2u:
    case SNMP_VERSION_2star:
    default:
        session->s_snmp_errno = SNMPERR_BAD_VERSION;
	return -1;
    }

    h1 = cp;
    cp = snmp_pdu_build(pdu, cp, out_length);
    if (cp == NULL)
	return -1;

    /* insert the actual length of the message sequence */
    switch (pdu->version) {
    case SNMP_VERSION_1:
    case SNMP_VERSION_2c:
        asn_build_sequence(packet, &length,
		       (u_char)(ASN_SEQUENCE | ASN_CONSTRUCTOR),
                       cp - h0e);
        break;

    case SNMP_VERSION_2p:
    case SNMP_VERSION_sec:
    case SNMP_VERSION_2u:
    case SNMP_VERSION_2star:
    default:
	session->s_snmp_errno = SNMPERR_BAD_VERSION;
	return -1;
    }
    *out_length = cp - packet;
    return 0;
}

int
snmp_build(struct snmp_session *pss,
	   struct snmp_pdu *pdu,
	   u_char *packet,
	   size_t *out_length)
{
    int rc;
    rc = _snmp_build(pss,pdu,packet,out_length);
    if (rc) {
        if ( !pss->s_snmp_errno)
            pss->s_snmp_errno = SNMPERR_BAD_ASN1_BUILD;
        SET_SNMP_ERROR(pss->s_snmp_errno);
        rc = -1;
    }
    return rc;
}

/* on error, returns NULL (likely an encoding problem). */
u_char *
snmp_pdu_build (struct snmp_pdu *pdu, u_char *cp, size_t *out_length)
{
  u_char *h1, *h1e, *h2, *h2e;
  struct variable_list *vp;
  struct sockaddr_in *pduIp = (struct sockaddr_in *)&(pdu->agent_addr);
  size_t length;

  length = *out_length;
  /* Save current location and build PDU tag and length placeholder
     (actual length will be inserted later) */
  h1 = cp;
  cp = asn_build_sequence(cp, out_length, (u_char)pdu->command, 0);
  if (cp == NULL)
    return NULL;
  h1e = cp;

  /* store fields in the PDU preceeding the variable-bindings sequence */
  if (pdu->command != SNMP_MSG_TRAP){
    /* PDU is not an SNMPv1 trap */

    /* request id */
    cp = asn_build_int(cp, out_length,
		       (u_char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
		       &pdu->reqid, sizeof(pdu->reqid));
    if (cp == NULL)
      return NULL;

    /* error status (getbulk non-repeaters) */
    cp = asn_build_int(cp, out_length,
		       (u_char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
		       &pdu->errstat, sizeof(pdu->errstat));
    if (cp == NULL)
      return NULL;

    /* error index (getbulk max-repetitions) */
    cp = asn_build_int(cp, out_length,
		       (u_char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
		       &pdu->errindex, sizeof(pdu->errindex));
    if (cp == NULL)
      return NULL;
  } else {
    /* an SNMPv1 trap PDU */

        /* enterprise */
    cp = asn_build_objid(cp, out_length,
			 (u_char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_OBJECT_ID),
			 (oid *)pdu->enterprise, pdu->enterprise_length);
    if (cp == NULL)
      return NULL;

        /* agent-addr */
    cp = asn_build_string(cp, out_length,
			  (u_char)(ASN_IPADDRESS | ASN_PRIMITIVE),
			  (u_char *)&pduIp->sin_addr.s_addr,
			  sizeof(pduIp->sin_addr.s_addr));
    if (cp == NULL)
      return NULL;

        /* generic trap */
    cp = asn_build_int(cp, out_length,
		       (u_char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
		       (long *)&pdu->trap_type, sizeof(pdu->trap_type));
    if (cp == NULL)
      return NULL;

        /* specific trap */
    cp = asn_build_int(cp, out_length,
		       (u_char)(ASN_UNIVERSAL | ASN_PRIMITIVE | ASN_INTEGER),
		       (long *)&pdu->specific_type, sizeof(pdu->specific_type));
    if (cp == NULL)
      return NULL;

        /* timestamp  */
    cp = asn_build_unsigned_int(cp, out_length,
				(u_char)(ASN_TIMETICKS | ASN_PRIMITIVE),
				&pdu->time, sizeof(pdu->time));
    if (cp == NULL)
      return NULL;
  }

  /* Save current location and build SEQUENCE tag and length placeholder
       for variable-bindings sequence
       (actual length will be inserted later) */
  h2 = cp;
  cp = asn_build_sequence(cp, out_length,
                          (u_char)(ASN_SEQUENCE | ASN_CONSTRUCTOR),
                          0);
  if (cp == NULL)
    return NULL;
  h2e = cp;

  /* Store variable-bindings */
  for(vp = pdu->variables; vp; vp = vp->next_variable){
    cp = snmp_build_var_op(cp, vp->name, &vp->name_length, vp->type,
			   vp->val_len, (u_char *)vp->val.string,
			   out_length);
    if (cp == NULL)
      return NULL;
  }

  /* insert actual length of variable-bindings sequence */
  asn_build_sequence(h2,&length,(u_char)(ASN_SEQUENCE|ASN_CONSTRUCTOR),cp-h2e);

  /* insert actual length of PDU sequence */
  asn_build_sequence(h1, &length, (u_char)pdu->command, cp - h1e);

  return cp;
}


/*
 * Parses the packet received to determine version, either directly
 * from packets version field or inferred from ASN.1 construct.
 */
static int
snmp_parse_version (u_char *data, size_t length)
{
  u_char type;
  long version = SNMPERR_BAD_VERSION;

  data = asn_parse_sequence(data, &length, &type,
                        (ASN_SEQUENCE | ASN_CONSTRUCTOR), "version");
  if (data) {
    data = asn_parse_int(data, &length, &type, &version, sizeof(version));
    if (!data) return SNMPERR_BAD_VERSION;
  }
  return version;
}


#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
int
snmpv3_parse(
     struct snmp_pdu	 *pdu,
     u_char 		 *data,
     size_t 		 *length,
     u_char 		**after_header)
{
  u_char	 type, msg_flags;
  long		 ver, msg_max_size, msg_sec_model;
  size_t	 max_size_response;
  u_char	 tmp_buf[SNMP_MAX_MSG_SIZE];
  size_t	 tmp_buf_len;
  u_char	 pdu_buf[SNMP_MAX_MSG_SIZE];
  size_t	 pdu_buf_len = SNMP_MAX_MSG_SIZE;
  u_char	*sec_params;
  u_char	*msg_data;
  u_char	*cp;
  size_t	 asn_len, msg_len;
  int		 ret, ret_val;


  msg_data =  data;
  msg_len  = *length;


  /* message is an ASN.1 SEQUENCE
   */
  DEBUGDUMPHEADER("dump_recv", "Parsing SNMPv3 Message\n");
  data = asn_parse_sequence(data, length, &type,
                        (ASN_SEQUENCE | ASN_CONSTRUCTOR), "message");
  if (data == NULL){
    /* error msg detail is set */
    snmp_increment_statistic(STAT_SNMPINASNPARSEERRS);
    DEBUGINDENTLESS();
    return SNMPERR_ASN_PARSE_ERR;
  }

  /* parse msgVersion
   */
  DEBUGDUMPHEADER("dump_recv", "Parsing SNMPv3 Version Number\n");
  data = asn_parse_int(data, length, &type, &ver, sizeof(ver));
  DEBUGINDENTLESS();
  if (data == NULL){
    ERROR_MSG("bad parse of version");
    snmp_increment_statistic(STAT_SNMPINASNPARSEERRS);
    DEBUGINDENTLESS();
    return SNMPERR_ASN_PARSE_ERR;
  }
  pdu->version = ver;

  /* parse msgGlobalData sequence
   */
  cp	  = data;
  asn_len = *length;
  DEBUGDUMPHEADER("dump_recv", "Parsing msgGlobalData\n");
  data = asn_parse_sequence(data, &asn_len, &type,
                        (ASN_SEQUENCE | ASN_CONSTRUCTOR), "msgGlobalData");
  if (data == NULL){
    /* error msg detail is set */
    snmp_increment_statistic(STAT_SNMPINASNPARSEERRS);
    DEBUGINDENTADD(-4);
    return SNMPERR_ASN_PARSE_ERR;
  }
  *length -= data - cp;  /* subtract off the length of the header */

  /* msgID */
  DEBUGDUMPHEADER("dump_recv", "Parsing msgID\n");
  data = asn_parse_int(data, length, &type, &pdu->msgid, sizeof(pdu->msgid));
  DEBUGINDENTLESS();
  if (data == NULL) {
    ERROR_MSG("error parsing msgID");
    DEBUGINDENTADD(-4);
    snmp_increment_statistic(STAT_SNMPINASNPARSEERRS);
    return SNMPERR_ASN_PARSE_ERR;
  }

  /* msgMaxSize */
  DEBUGDUMPHEADER("dump_recv", "Parsing msgMaxSize\n");
  data = asn_parse_int(data, length, &type, &msg_max_size,
		       sizeof(msg_max_size));
  DEBUGINDENTLESS();
  if (data == NULL) {
    ERROR_MSG("error parsing msgMaxSize");
    snmp_increment_statistic(STAT_SNMPINASNPARSEERRS);
    DEBUGINDENTADD(-4);
    return SNMPERR_ASN_PARSE_ERR;
  }

  /* msgFlags */
  tmp_buf_len = SNMP_MAX_MSG_SIZE;
  DEBUGDUMPHEADER("dump_recv", "Parsing msgFlags\n");
  data = asn_parse_string(data, length, &type, tmp_buf, &tmp_buf_len);
  DEBUGINDENTLESS();
  if (data == NULL || tmp_buf_len != 1) {
    ERROR_MSG("error parsing msgFlags");
    snmp_increment_statistic(STAT_SNMPINASNPARSEERRS);
    DEBUGINDENTADD(-4);
    return SNMPERR_ASN_PARSE_ERR;
  }
  msg_flags = *tmp_buf;
  if (msg_flags & SNMP_MSG_FLAG_RPRT_BIT)
    pdu->flags |= SNMP_MSG_FLAG_RPRT_BIT;
  else
    pdu->flags &= (~SNMP_MSG_FLAG_RPRT_BIT);

  /* msgSecurityModel */
  DEBUGDUMPHEADER("dump_recv", "Parsing msgSecurityModel\n");
  data = asn_parse_int(data, length, &type, &msg_sec_model,
		       sizeof(msg_sec_model));
  DEBUGINDENTADD(-4); /* return from global data indent */
  if (data == NULL) {
    ERROR_MSG("error parsing msgSecurityModel");
    snmp_increment_statistic(STAT_SNMPINASNPARSEERRS);
    DEBUGINDENTLESS();
    return SNMPERR_ASN_PARSE_ERR;
  }
  if (msg_sec_model != SNMP_SEC_MODEL_USM) {
    ERROR_MSG("unknown security model");
    snmp_increment_statistic(STAT_SNMPUNKNOWNSECURITYMODELS);
    DEBUGINDENTLESS();
    return SNMPERR_UNKNOWN_SEC_MODEL;
  }
  pdu->securityModel = msg_sec_model;

  if (msg_flags & SNMP_MSG_FLAG_PRIV_BIT && 
      !(msg_flags & SNMP_MSG_FLAG_AUTH_BIT)) {
    ERROR_MSG("invalid message, illegal msgFlags");
    snmp_increment_statistic(STAT_SNMPINVALIDMSGS);
    DEBUGINDENTLESS();
    return SNMPERR_INVALID_MSG;
  }
  pdu->securityLevel = ( (msg_flags & SNMP_MSG_FLAG_AUTH_BIT)
				?  ( (msg_flags & SNMP_MSG_FLAG_PRIV_BIT)
					? SNMP_SEC_LEVEL_AUTHPRIV
					: SNMP_SEC_LEVEL_AUTHNOPRIV )
				: SNMP_SEC_LEVEL_NOAUTH );
  /* end of msgGlobalData */

  /* securtityParameters OCTET STRING begins after msgGlobalData */
  sec_params			= data;
  pdu->contextEngineID		= (u_char *)calloc(1,SNMP_MAX_ENG_SIZE);
  pdu->contextEngineIDLen	= SNMP_MAX_ENG_SIZE;
  pdu->securityEngineID         = (u_char *)calloc(1,SNMP_MAX_ENG_SIZE);
  pdu->securityEngineIDLen	= SNMP_MAX_ENG_SIZE;
  pdu->securityName		= (char *)calloc(1,SNMP_MAX_SEC_NAME_SIZE);
  pdu->securityNameLen		= SNMP_MAX_SEC_NAME_SIZE;

  memset(pdu_buf, 0, pdu_buf_len);
  cp = pdu_buf;

  DEBUGDUMPHEADER("dump_recv", "Parsing USM msgSecurityParameters\n");
  ret_val = usm_process_in_msg(SNMP_VERSION_3, msg_max_size,
			       sec_params, msg_sec_model, pdu->securityLevel,
			       msg_data, msg_len,
			       pdu->securityEngineID, &pdu->securityEngineIDLen,
			       pdu->securityName, &pdu->securityNameLen,
			       &cp,
			       &pdu_buf_len, &max_size_response,
			       &pdu->securityStateRef);
  DEBUGINDENTLESS();

  if (ret_val != SNMPERR_SUCCESS) {
    snmpv3_scopedPDU_parse(pdu, cp, &pdu_buf_len); /* DO ignore return code */
    DEBUGINDENTLESS();
    return ret_val;
  }
  
  /* parse plaintext ScopedPDU sequence */
  *length = pdu_buf_len;
  DEBUGDUMPHEADER("dump_recv", "Parsing ScopedPdu\n");
  data = snmpv3_scopedPDU_parse(pdu, cp, length);
  if (data == NULL) {
    snmp_increment_statistic(STAT_SNMPINASNPARSEERRS);
    DEBUGINDENTADD(-4);
    return SNMPERR_ASN_PARSE_ERR;
  }

  /* parse the PDU.
   */
  if (after_header != NULL) {
    tmp_buf_len		 = *length;
    *after_header	 = data;
  }

  DEBUGDUMPHEADER("dump_recv", "Parsing PDU\n");
  ret = snmp_pdu_parse(pdu, data, length);
  DEBUGINDENTADD(-8); 

  if (after_header != NULL)
    *length = tmp_buf_len;

  if (ret != SNMPERR_SUCCESS) {
    ERROR_MSG("error parsing PDU");
    snmp_increment_statistic(STAT_SNMPINASNPARSEERRS);
    return SNMPERR_ASN_PARSE_ERR;
  }

  return SNMPERR_SUCCESS;
}  /* end snmpv3_parse() */

#define ERROR_STAT_LENGTH 11

int
snmpv3_make_report(struct snmp_pdu *pdu, int error)
{

  long ltmp;
  static oid unknownSecurityLevel[] = {1,3,6,1,6,3,15,1,1,1,0};
  static oid notInTimeWindow[]      = {1,3,6,1,6,3,15,1,1,2,0};
  static oid unknownUserName[]      = {1,3,6,1,6,3,15,1,1,3,0};
  static oid unknownEngineID[]      = {1,3,6,1,6,3,15,1,1,4,0};
  static oid wrongDigest[]          = {1,3,6,1,6,3,15,1,1,5,0};
  static oid decryptionError[]      = {1,3,6,1,6,3,15,1,1,6,0};
  oid *err_var;
  int err_var_len;
  int stat_ind;

  switch (error) {
  case SNMPERR_USM_UNKNOWNENGINEID:
    stat_ind = STAT_USMSTATSUNKNOWNENGINEIDS;
    err_var = unknownEngineID;
    err_var_len = ERROR_STAT_LENGTH;
    break;
  case SNMPERR_USM_UNKNOWNSECURITYNAME:
    stat_ind = STAT_USMSTATSUNKNOWNUSERNAMES;
    err_var = unknownUserName;
    err_var_len = ERROR_STAT_LENGTH;
    break;
  case SNMPERR_USM_UNSUPPORTEDSECURITYLEVEL:
    stat_ind = STAT_USMSTATSUNSUPPORTEDSECLEVELS;
    err_var = unknownSecurityLevel;
    err_var_len = ERROR_STAT_LENGTH;
    break;
  case SNMPERR_USM_AUTHENTICATIONFAILURE:
    stat_ind = STAT_USMSTATSWRONGDIGESTS;
    err_var = wrongDigest;
    err_var_len = ERROR_STAT_LENGTH;
    break;
  case SNMPERR_USM_NOTINTIMEWINDOW:
    stat_ind = STAT_USMSTATSNOTINTIMEWINDOWS;
    err_var = notInTimeWindow;
    err_var_len = ERROR_STAT_LENGTH;
    break;
  case SNMPERR_USM_DECRYPTIONERROR:
    stat_ind = STAT_USMSTATSDECRYPTIONERRORS;
    err_var = decryptionError;
    err_var_len = ERROR_STAT_LENGTH;
    break;
  default:
    return SNMPERR_GENERR;
    break;
  }

  snmp_free_varbind(pdu->variables);	/* free the current varbind */

  pdu->variables	= NULL;
  SNMP_FREE(pdu->securityEngineID);
  pdu->securityEngineID	= snmpv3_generate_engineID(&pdu->securityEngineIDLen);
  SNMP_FREE(pdu->contextEngineID);
  pdu->contextEngineID	= snmpv3_generate_engineID(&pdu->contextEngineIDLen);
  pdu->command		= SNMP_MSG_REPORT;
  pdu->errstat		= 0;
  pdu->errindex		= 0;
  pdu->contextName	= strdup("");
  pdu->contextNameLen	= strlen(pdu->contextName);

  /* reports shouldn't cache previous data. */
  /* FIX - yes they should but USM needs to follow new EoP to determine
     which cached values to use 
  */
  if (pdu->securityStateRef) {
    usm_free_usmStateReference(pdu->securityStateRef);
    pdu->securityStateRef = NULL;
  }
  
  if (error != SNMPERR_USM_NOTINTIMEWINDOW) 
    pdu->securityLevel          = SNMP_SEC_LEVEL_NOAUTH;
  else
    pdu->securityLevel          = SNMP_SEC_LEVEL_AUTHNOPRIV;

  /* find the appropriate error counter
   */
  ltmp = snmp_get_statistic(stat_ind);

  /* return the appropriate error counter
   */
  snmp_pdu_add_variable(pdu, err_var, err_var_len,
                        ASN_COUNTER, (u_char *) &ltmp, sizeof(ltmp));

  return SNMPERR_SUCCESS;
}  /* end snmpv3_make_report() */


int
snmpv3_get_report_type(struct snmp_pdu *pdu)
{
  static oid snmpMPDStats[] = {1,3,6,1,6,3,11,2,1};
  static oid usmStats[] = {1,3,6,1,6,3,15,1,1};
  struct variable_list *vp;
  int rpt_type = SNMPERR_UNKNOWN_REPORT;

  if (pdu == NULL || pdu->variables == NULL) return rpt_type;
  vp = pdu->variables;
  if (vp->name_length == REPORT_STATS_LEN+2) {
    if (memcmp(snmpMPDStats,vp->name,REPORT_STATS_LEN*sizeof(oid)) == 0) {
      switch (vp->name[REPORT_STATS_LEN]) {
      case REPORT_snmpUnknownSecurityModels_NUM:
	rpt_type = SNMPERR_UNKNOWN_SEC_MODEL;
	break;
      case REPORT_snmpInvalidMsgs_NUM:
	rpt_type = SNMPERR_INVALID_MSG;
	break;
      }
    } else if (memcmp(usmStats,vp->name,REPORT_STATS_LEN*sizeof(oid)) == 0) {
      switch (vp->name[REPORT_STATS_LEN]) {
      case REPORT_usmStatsUnsupportedSecLevels_NUM:
	rpt_type = SNMPERR_UNSUPPORTED_SEC_LEVEL;
	break;
      case REPORT_usmStatsNotInTimeWindows_NUM:
	rpt_type = SNMPERR_NOT_IN_TIME_WINDOW;
	break;
      case REPORT_usmStatsUnknownUserNames_NUM:
	rpt_type = SNMPERR_UNKNOWN_USER_NAME;
	break;
      case REPORT_usmStatsUnknownEngineIDs_NUM:
	rpt_type = SNMPERR_UNKNOWN_ENG_ID;
	break;
      case REPORT_usmStatsWrongDigests_NUM:
	rpt_type = SNMPERR_AUTHENTICATION_FAILURE;
	break;
      case REPORT_usmStatsDecryptionErrors_NUM:
	rpt_type = SNMPERR_DECRYPTION_ERR;
	break;
      }
    }
  }
  DEBUGMSGTL(("report", "Report type: %d\n", rpt_type));
  return rpt_type;
}

#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */

/*
 * Parses the packet received on the input session, and places the data into
 * the input pdu.  length is the length of the input packet.
 * If any errors are encountered, -1 or USM error is returned.
 * Otherwise, a 0 is returned.
 */
static int
_snmp_parse(void * sessp,
	   struct snmp_session *session,
	   struct snmp_pdu *pdu,
	   u_char *data,
	   size_t length)
{
    u_char community[COMMUNITY_MAX_LEN];
    size_t community_length = COMMUNITY_MAX_LEN;
    int result = -1;

    session->s_snmp_errno = 0;
    session->s_errno = 0;

	/* Ensure all incoming PDUs have a unique means of identification 
		(This is not restricted to AgentX handling,
		 though that is where the need becomes visible)	*/
    pdu->transid = snmp_get_next_transid();

    if (session->version != SNMP_DEFAULT_VERSION)
	pdu->version = session->version;
    else
        pdu->version = snmp_parse_version(data,length);

    switch (pdu->version) {
    case SNMP_VERSION_1:
    case SNMP_VERSION_2c:
        DEBUGMSGTL(("snmp_api","Parsing SNMPv%d message...\n", (1 + pdu->version)));

	/* authenticates message and returns length if valid */
        DEBUGDUMPSETUP("dump_recv", data, 4);
        DEBUGMSG(("dump_recv", "SNMPv%d message\n", (1+pdu->version)));

        DEBUGINDENTMORE();
	data = snmp_comstr_parse(data, &length,
                                 community, &community_length,
			         &pdu->version);
        DEBUGINDENTLESS();
	if (data == NULL)
	    return -1;

        if (pdu->version != session->version &&
	    session->version != SNMP_DEFAULT_VERSION)
	{
            session->s_snmp_errno = SNMPERR_BAD_VERSION;
            return -1;
	}

	/* maybe get the community string. */
	pdu->securityLevel = SNMP_SEC_LEVEL_NOAUTH;
	pdu->securityModel = (pdu->version == SNMP_VERSION_1) ?
          SNMP_SEC_MODEL_SNMPv1 : SNMP_SEC_MODEL_SNMPv2c;
	SNMP_FREE(pdu->community);
	pdu->community_len = 0;
	pdu->community = (u_char *)0;
	if (community_length) {
	    pdu->community_len = community_length;
	    pdu->community = (u_char *)malloc(community_length);
	    memmove(pdu->community, community, community_length);
	}
	if (session->authenticator){
	    data = session->authenticator(data, &length,
					  community,
                                          community_length);
	    if (data == NULL)
	    {
		session->s_snmp_errno = SNMPERR_AUTHENTICATION_FAILURE;
		return -1;
	    }
	}
	result = snmp_pdu_parse(pdu, data, &length);
        break;

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
    case SNMP_VERSION_3:
      result = snmpv3_parse(pdu, data, &length, NULL);
      DEBUGMSGTL(("snmp_parse",
                   "Parsed SNMPv3 message (secName:%s, secLevel:%s): %s\n",
                   pdu->securityName, usmSecLevelName[pdu->securityLevel],
                   snmp_api_errstring(result)));

      if (result) {
	if (!sessp)
	  session->s_snmp_errno = result;
	else

	/* handle reportable errors */
	switch (result) {
	case SNMPERR_USM_UNKNOWNENGINEID:
	case SNMPERR_USM_UNKNOWNSECURITYNAME:
	case SNMPERR_USM_UNSUPPORTEDSECURITYLEVEL:
	case SNMPERR_USM_AUTHENTICATIONFAILURE:
	case SNMPERR_USM_NOTINTIMEWINDOW:
	case SNMPERR_USM_DECRYPTIONERROR:
          if (SNMP_CMD_CONFIRMED(pdu->command) ||
	      (pdu->command == 0 && 
              (pdu->flags & SNMP_MSG_FLAG_RPRT_BIT ))) {
	    struct snmp_pdu *pdu2;
	    int flags = pdu->flags;
	    pdu->flags |= UCD_MSG_FLAG_FORCE_PDU_COPY;
	    pdu2 = snmp_clone_pdu(pdu);
	    pdu->flags = pdu2->flags = flags;
	    snmpv3_make_report(pdu2, result);
	    snmp_sess_send(sessp, pdu2);
	  }
	  break;
	default:
	  session->s_snmp_errno = result;
	  break;
	}
      }
      break;
#endif
    case SNMPERR_BAD_VERSION:
      ERROR_MSG("error parsing snmp message version");
      snmp_increment_statistic(STAT_SNMPINASNPARSEERRS);
      session->s_snmp_errno = SNMPERR_BAD_VERSION;
      break;
    case SNMP_VERSION_sec:
    case SNMP_VERSION_2u:
    case SNMP_VERSION_2star:
    case SNMP_VERSION_2p:
    default:
        ERROR_MSG("unsupported snmp message version");
	snmp_increment_statistic(STAT_SNMPINBADVERSIONS);
	session->s_snmp_errno = SNMPERR_BAD_VERSION;
        break;
    }

    return result;
}

static int
snmp_parse(void *sessp,
	   struct snmp_session *pss,
	   struct snmp_pdu *pdu,
	   u_char *data,
	   size_t length)
{
    int rc;

    rc = _snmp_parse(sessp,pss,pdu,data,length);
    if (rc) {
        if ( !pss->s_snmp_errno)
            pss->s_snmp_errno = SNMPERR_BAD_PARSE;
        SET_SNMP_ERROR(pss->s_snmp_errno);
    }

    return rc;
}

int
snmp_pdu_parse(struct snmp_pdu *pdu, u_char  *data, size_t *length) {
  u_char  type;
  u_char  msg_type;
  u_char  *var_val;
  int      badtype;
  size_t   len;
  size_t   four;
  struct variable_list *vp = NULL;
  struct sockaddr_in *pduIp = (struct sockaddr_in *)&(pdu->agent_addr);
  oid objid[MAX_OID_LEN];

  badtype = 0;

  DEBUGPRINTINDENT("dump_recv");
  DEBUGINDENTMORE();
  DEBUGMSG(("dump_recv", "PDU\n"));
  /* Get the PDU type */
  data = asn_parse_header(data, length, &msg_type);
  if (data == NULL)
    return -1;
  pdu->command = msg_type;
  pdu->flags &= (~UCD_MSG_FLAG_RESPONSE_PDU);

  /* get the fields in the PDU preceeding the variable-bindings sequence */
  switch (pdu->command) {
  case SNMP_MSG_TRAP:
    /* enterprise */
    pdu->enterprise_length = MAX_OID_LEN;
    data = asn_parse_objid(data, length, &type, objid,
			   &pdu->enterprise_length);
    if (data == NULL)
      return -1;
    pdu->enterprise = (oid *)malloc(pdu->enterprise_length * sizeof(oid));
    memmove(pdu->enterprise, objid, pdu->enterprise_length * sizeof(oid));

    /* agent-addr */
    four = 4;
    pduIp->sin_family = AF_INET;
    pduIp->sin_len = sizeof(*pduIp);
    data = asn_parse_string(data, length, &type,
			    (u_char *)&pduIp->sin_addr.s_addr,
			    &four);
    if (data == NULL)
      return -1;

    /* generic trap */
    data = asn_parse_int(data, length, &type, (long *)&pdu->trap_type,
			 sizeof(pdu->trap_type));
    if (data == NULL)
      return -1;
    /* specific trap */
    data = asn_parse_int(data, length, &type, (long *)&pdu->specific_type,
			 sizeof(pdu->specific_type));
    if (data == NULL)
      return -1;

    /* timestamp  */
    data = asn_parse_unsigned_int(data, length, &type, &pdu->time,
				  sizeof(pdu->time));
    if (data == NULL)
      return -1;

    break;

  case SNMP_MSG_RESPONSE:
  case SNMP_MSG_REPORT:
    pdu->flags |= UCD_MSG_FLAG_RESPONSE_PDU;
    /* fallthrough */

  default:
    /* PDU is not an SNMPv1 TRAP */

    /* request id */
    DEBUGDUMPHEADER("dump_recv", "Parsing request_id\n");
    data = asn_parse_int(data, length, &type, &pdu->reqid,
			 sizeof(pdu->reqid));
    DEBUGINDENTLESS();
    if (data == NULL) {
      return -1;
    }

    /* error status (getbulk non-repeaters) */
    DEBUGDUMPHEADER("dump_recv", "Parsing error status\n");
    data = asn_parse_int(data, length, &type, &pdu->errstat,
			 sizeof(pdu->errstat));
    DEBUGINDENTLESS();
    if (data == NULL) {
      return -1;
    }

    /* error index (getbulk max-repetitions) */
    DEBUGDUMPHEADER("dump_recv", "Parsing error index\n");
    data = asn_parse_int(data, length, &type, &pdu->errindex,
			 sizeof(pdu->errindex));
    DEBUGINDENTLESS();
    if (data == NULL) {
      return -1;
    }
  }

  /* get header for variable-bindings sequence */
  DEBUGDUMPHEADER("dump_recv", "VarBindList:\n");
  data = asn_parse_sequence(data, length, &type,
                        (ASN_SEQUENCE | ASN_CONSTRUCTOR), "varbinds");
  if (data == NULL)
    return -1;

    /* get each varBind sequence */
  while((int)*length > 0){
    struct variable_list *vptemp;
    vptemp = (struct variable_list *)malloc(sizeof(*vptemp));
    if (0 == vptemp) {
        return -1;
    }
    if (0 == vp){
        pdu->variables = vptemp;
    } else {
        vp->next_variable = vptemp;
    }
    vp = vptemp;

    vp->next_variable = NULL;
    vp->val.string = NULL;
    vp->name_length = MAX_OID_LEN;
    vp->name = 0;
    DEBUGDUMPHEADER("dump_recv", "VarBind:\n");
    data = snmp_parse_var_op(data, objid, &vp->name_length, &vp->type,
			     &vp->val_len, &var_val, length);
    if (data == NULL)
      return -1;
    if (snmp_set_var_objid(vp, objid, vp->name_length))
        return -1;

    len = PACKET_LENGTH;
    switch((short)vp->type){
    case ASN_INTEGER:
      vp->val.integer = (long *)vp->buf;
      vp->val_len = sizeof(long);
      asn_parse_int(var_val, &len, &vp->type,
		    (long *)vp->val.integer,
		    sizeof(vp->val.integer));
      break;
    case ASN_COUNTER:
    case ASN_GAUGE:
    case ASN_TIMETICKS:
    case ASN_UINTEGER:
      vp->val.integer = (long *)vp->buf;
      vp->val_len = sizeof(u_long);
      asn_parse_unsigned_int(var_val, &len, &vp->type,
			     (u_long *)vp->val.integer,
			     sizeof(vp->val.integer));
      break;
#ifdef OPAQUE_SPECIAL_TYPES
    case ASN_OPAQUE_COUNTER64:
    case ASN_OPAQUE_U64:
#endif /* OPAQUE_SPECIAL_TYPES */
    case ASN_COUNTER64:
      vp->val.counter64 = (struct counter64 *)vp->buf;
      vp->val_len = sizeof(struct counter64);
      asn_parse_unsigned_int64(var_val, &len, &vp->type,
			       (struct counter64 *)vp->val.counter64,
			       sizeof(*vp->val.counter64));
      break;
#ifdef OPAQUE_SPECIAL_TYPES
    case ASN_OPAQUE_FLOAT:
      vp->val.floatVal = (float *)vp->buf;
      vp->val_len = sizeof(float);
      asn_parse_float(var_val, &len, &vp->type,
		      vp->val.floatVal,
		      vp->val_len);
      break;
    case ASN_OPAQUE_DOUBLE:
      vp->val.doubleVal = (double *)vp->buf;
      vp->val_len = sizeof(double);
      asn_parse_double(var_val, &len, &vp->type,
		       vp->val.doubleVal,
		       vp->val_len);
      break;
    case ASN_OPAQUE_I64:
      vp->val.counter64 = (struct counter64 *)vp->buf;
      vp->val_len = sizeof(struct counter64);
      asn_parse_signed_int64(var_val, &len, &vp->type,
			     (struct counter64 *)vp->val.counter64,
			     sizeof(*vp->val.counter64));

      break;
#endif /* OPAQUE_SPECIAL_TYPES */
      case ASN_OCTET_STR:
      case ASN_IPADDRESS:
      case ASN_OPAQUE:
      case ASN_NSAP:
        if (vp->val_len < sizeof(vp->buf)){
          vp->val.string = (u_char *)vp->buf;
        } else {
          vp->val.string = (u_char *)malloc((unsigned)vp->val_len);
        }
        asn_parse_string(var_val, &len, &vp->type, vp->val.string,
                         &vp->val_len);
        break;
      case ASN_OBJECT_ID:
        vp->val_len = MAX_OID_LEN;
        asn_parse_objid(var_val, &len, &vp->type, objid, &vp->val_len);
        vp->val_len *= sizeof(oid);
        vp->val.objid = (oid *)malloc((unsigned)vp->val_len);
        memmove(vp->val.objid, objid, vp->val_len);
        break;
      case SNMP_NOSUCHOBJECT:
      case SNMP_NOSUCHINSTANCE:
      case SNMP_ENDOFMIBVIEW:
      case ASN_NULL:
        break;
      case ASN_BIT_STR:
        vp->val.bitstring = (u_char *)malloc(vp->val_len);
        asn_parse_bitstring(var_val, &len, &vp->type,
                            vp->val.bitstring, &vp->val_len);
        break;
      default:
        snmp_log(LOG_ERR,"bad type returned (%x)\n", vp->type);
        badtype = 1;
        break;
    }
    DEBUGINDENTLESS();
  }
  DEBUGINDENTLESS();
  DEBUGINDENTLESS();
  return badtype;
}

/* snmp v3 utility function to parse into the scopedPdu. stores contextName
   and contextEngineID in pdu struct. Also stores pdu->command (handy for 
   Report generation).

   returns pointer to begining of PDU or NULL on error.
*/

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
u_char *
snmpv3_scopedPDU_parse(struct snmp_pdu *pdu,
			u_char  *cp,
			size_t  *length)
{
  u_char  tmp_buf[SNMP_MAX_MSG_SIZE];
  size_t  tmp_buf_len;
  u_char  type;
  size_t  asn_len;
  u_char* data;

  pdu->command = 0; /* initialize so we know if it got parsed */
  asn_len = *length;
  data = asn_parse_sequence(cp, &asn_len, &type,
                        (ASN_SEQUENCE | ASN_CONSTRUCTOR), "plaintext scopedPDU");
  if (data == NULL){
    return NULL;
  }
  *length -= data - cp;

  /* contextEngineID from scopedPdu  */
  DEBUGDUMPHEADER("dump_recv", "Parsing contextEngineID\n");
  data = asn_parse_string(data, length, &type, pdu->contextEngineID, 
			  &pdu->contextEngineIDLen);
  DEBUGINDENTLESS();
  if (data == NULL) {
    ERROR_MSG("error parsing contextEngineID from scopedPdu");
    return NULL;
  }

  /* check that it agrees with engineID returned from USM above
   * only a warning because this could be legal if we are a proxy
   */
  if (pdu->securityEngineIDLen != pdu->contextEngineIDLen ||
      memcmp(pdu->securityEngineID, pdu->contextEngineID,
	     pdu->securityEngineIDLen) != 0) {
    DEBUGMSGTL(("scopedPDU_parse",
                "inconsistent engineID information in message\n"));
  }

  /* parse contextName from scopedPdu
   */
  tmp_buf_len = SNMP_MAX_CONTEXT_SIZE;
  DEBUGDUMPHEADER("dump_recv", "Parsing contextName\n");
  data = asn_parse_string(data, length, &type, tmp_buf, &tmp_buf_len);
  DEBUGINDENTLESS();
  if (data == NULL) {
    ERROR_MSG("error parsing contextName from scopedPdu");
    return NULL;
  }

  if (tmp_buf_len) {
    pdu->contextName	 = (char *)malloc(tmp_buf_len);
    memmove(pdu->contextName, tmp_buf, tmp_buf_len);
    pdu->contextNameLen	 = tmp_buf_len;
  } else {
    pdu->contextName	 = strdup("");
    pdu->contextNameLen	 = 0;
  }

  /* Get the PDU type */
  asn_len = *length;
  DEBUGDUMPHEADER("dump_recv", "Parsing PDU type\n");
  cp = asn_parse_header(data, &asn_len, &type);
  DEBUGINDENTLESS();
  if (cp == NULL)
    return NULL;

  pdu->command = type;

  return data;
}
#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */

/*
 * Sends the input pdu on the session after calling snmp_build to create
 * a serialized packet.  If necessary, set some of the pdu data from the
 * session defaults.  Add a request corresponding to this pdu to the list
 * of outstanding requests on this session, then send the pdu.
 * Returns the request id of the generated packet if applicable, otherwise 1.
 * On any error, 0 is returned.
 * The pdu is freed by snmp_send() unless a failure occured.
 */
int
snmp_send(struct snmp_session *session,
	  struct snmp_pdu *pdu)
{
  return snmp_async_send(session, pdu, NULL, NULL);
}

int
snmp_sess_send(void *sessp,
	       struct snmp_pdu *pdu)
{
  return snmp_sess_async_send(sessp, pdu, NULL, NULL);
}

/*
 * int snmp_async_send(session, pdu, callback, cb_data)
 *     struct snmp_session *session;
 *     struct snmp_pdu	*pdu;
 *     snmp_callback callback;
 *     void   *cb_data;
 *
 * Sends the input pdu on the session after calling snmp_build to create
 * a serialized packet.  If necessary, set some of the pdu data from the
 * session defaults.  Add a request corresponding to this pdu to the list
 * of outstanding requests on this session and store callback and data,
 * then send the pdu.
 * Returns the request id of the generated packet if applicable, otherwise 0.
 * On any error, 0 is returned.
 * The pdu is freed by snmp_send() unless a failure occurred.
 */
int
snmp_async_send(struct snmp_session *session,
		struct snmp_pdu	*pdu,
		snmp_callback callback,
		void *cb_data)
{
    void *sessp = snmp_sess_pointer(session);
    return snmp_sess_async_send(sessp, pdu, callback, cb_data);
}

static int
_sess_async_send(void *sessp,
		     struct snmp_pdu *pdu,
		     snmp_callback callback,
		     void *cb_data)
{
    struct session_list *slp = (struct session_list *)sessp;
    struct snmp_session *session;
    struct snmp_internal_session *isp;
    u_char  packet[PACKET_LENGTH];
    size_t length = PACKET_LENGTH;
    struct sockaddr_in *isp_addr;
    struct sockaddr_in *pduIp;
    int result, addr_size;
    long reqid;

    session = slp->session; isp = slp->internal;
    if (!session || !isp) {
      DEBUGMSGTL(("sess_read","send fail: closing...\n"));
      return 0;
    }

    session->s_snmp_errno = 0;
    session->s_errno = 0;

    if (pdu == NULL) {
        session->s_snmp_errno = SNMPERR_NULL_PDU;
        return 0;
    }
#if TEMPORARILY_DISABLED
	 /*
	  *  NULL variable are allowed in certain PDU types.
	  *  In particular, SNMPv3 engineID probes are of this form.
	  *  There is an internal PDU flag to indicate that this
	  *    is acceptable, but until the construction of engineID
	  *    probes can be amended to set this flag, we'll simply
	  *    skip this test altogether.
	  */
    if (pdu->variables == NULL) {
	switch (pdu->command) {
	case SNMP_MSG_GET:
	case SNMP_MSG_SET:
	case SNMP_MSG_GETNEXT:
	case SNMP_MSG_GETBULK:
	case SNMP_MSG_RESPONSE:
	case SNMP_MSG_TRAP2:
	case SNMP_MSG_REPORT:
	case SNMP_MSG_INFORM:
	    session->s_snmp_errno = snmp_errno = SNMPERR_NO_VARS;
	    return 0;
	case SNMP_MSG_TRAP:
	    break;
        }
    }
#endif

    pduIp = (struct sockaddr_in *)&(pdu->address);
    pdu->flags |= UCD_MSG_FLAG_EXPECT_RESPONSE;

    /* check/setup the version */
    if (pdu->version == SNMP_DEFAULT_VERSION) {
        if (session->version == SNMP_DEFAULT_VERSION) {
	    session->s_snmp_errno = SNMPERR_BAD_VERSION;
	    return 0;
        }
        pdu->version = session->version;
    } else if (session->version == SNMP_DEFAULT_VERSION) {
	/* It's OK */
    } else if (pdu->version != session->version) {
      /* ENHANCE: we should support multi-lingual sessions */
        session->s_snmp_errno = SNMPERR_BAD_VERSION;
        return 0;
    }

    if (pdu->address.sa_family == AF_UNSPEC){
	isp_addr = (struct sockaddr_in *)&(isp->addr);
	if (isp->addr.sa_family == AF_UNSPEC ||
	   (isp->addr.sa_family == AF_INET &&
	    isp_addr->sin_addr.s_addr == SNMP_DEFAULT_ADDRESS)){
	        session->s_snmp_errno = SNMPERR_BAD_ADDRESS;
	        return 0;
	}
	memmove(&pdu->address, &(isp->addr), sizeof(isp->addr));
    }

    addr_size = snmp_socket_length(pdu->address.sa_family);

    /* build the message to send */
    if (isp->hook_build)
	result = isp->hook_build(session, pdu, packet, &length);
    else
	result = snmp_build(session, pdu, packet, &length);
    if (result < 0){
	return 0;
    }
    if (ds_get_boolean(DS_LIBRARY_ID, DS_LIB_DUMP_PACKET)){
	snmp_log(LOG_DEBUG, "\nSending %u bytes to %s:%hu\n", length,
	       inet_ntoa(pduIp->sin_addr), ntohs(pduIp->sin_port));
	xdump(packet, length, "");
    }

    /* send the message */
#if !defined(__ECOS)
    if ( session->flags & SNMP_FLAGS_STREAM_SOCKET ) {
      result = send(isp->sd, (char *)packet, length, 0);
    }
    else
#endif
        result = sendto(isp->sd, (char *)packet, length, 0,
	       (struct sockaddr *)&pdu->address, addr_size);
    if ( result < 0){
	session->s_snmp_errno = SNMPERR_BAD_SENDTO;
	session->s_errno = errno;
	return 0;
    }

    reqid = pdu->reqid;

    /* add to pending requests list if expect a response */
    if (pdu->flags & UCD_MSG_FLAG_EXPECT_RESPONSE) {
        struct request_list *rp;
        struct timeval tv;

	rp = (struct request_list *)calloc( 1, sizeof(struct request_list));
	if (rp == NULL) {
	    session->s_snmp_errno = SNMPERR_GENERR;
	    return 0;
	}

	gettimeofday(&tv, (struct timezone *)0);
	rp->pdu = pdu;
	rp->request_id = pdu->reqid;
	rp->message_id = pdu->msgid;
	rp->callback = callback;
	rp->cb_data = cb_data;
	rp->retries = 0;
	rp->timeout = session->timeout;
	rp->time = tv;
	tv.tv_usec += rp->timeout;
	tv.tv_sec += tv.tv_usec / 1000000L;
	tv.tv_usec %= 1000000L;
	rp->expire = tv;

	/* XX lock should be per session ! */
      snmp_res_lock(MT_LIBRARY_ID, MT_LIB_SESSION);
	if (isp->requestsEnd){
	    rp->next_request = isp->requestsEnd->next_request;
	    isp->requestsEnd->next_request = rp;
	    isp->requestsEnd = rp;
	} else {
	    rp->next_request = isp->requests;
	    isp->requests = rp;
	    isp->requestsEnd = rp;
	}
      snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_SESSION);
    }
    else
        snmp_free_pdu(pdu);  /* free v1 or v2 TRAP PDU */

    return reqid;
}

int
snmp_sess_async_send(void *sessp,
		     struct snmp_pdu *pdu,
		     snmp_callback callback,
		     void *cb_data)
{
    int rc;

    if (sessp == NULL){
	snmp_errno = SNMPERR_BAD_SESSION; /*MTCRITICAL_RESOURCE*/
	return(0);
    }
    rc = _sess_async_send(sessp,pdu,callback,cb_data);
    if (rc == 0) {
        struct session_list *psl;
        struct snmp_session *pss;
        psl = (struct session_list *)sessp;
        pss = psl->session;
        SET_SNMP_ERROR(pss->s_snmp_errno);
    }
    return rc;
}


/*
 * Frees the variable and any malloc'd data associated with it.
 */
void
snmp_free_var(struct variable_list *var)
{
    if (!var) return;

    if (var->name != var->name_loc)
        SNMP_FREE(var->name);
    if (var->val.string != var->buf)
        SNMP_FREE(var->val.string);

    free((char *)var);
}

void snmp_free_varbind(struct variable_list *var)
{
  struct variable_list *ptr;
  while(var) {
    ptr = var->next_variable;
    snmp_free_var(var);
    var = ptr;
  }
}

/*
 * Frees the pdu and any malloc'd data associated with it.
 */
void
snmp_free_pdu(struct snmp_pdu *pdu)
{
    if (!pdu) return;

    snmp_free_varbind(pdu->variables);
    SNMP_FREE(pdu->enterprise);
    SNMP_FREE(pdu->community);
    SNMP_FREE(pdu->contextEngineID);
    SNMP_FREE(pdu->securityEngineID);
    SNMP_FREE(pdu->contextName);
    SNMP_FREE(pdu->securityName);
    free((char *)pdu);
}

/*
 * Checks to see if any of the fd's set in the fdset belong to
 * snmp.  Each socket with it's fd set has a packet read from it
 * and snmp_parse is called on the packet received.  The resulting pdu
 * is passed to the callback routine for that session.  If the callback
 * routine returns successfully, the pdu and it's request are deleted.
 */
void
snmp_read(fd_set *fdset)
{
    struct session_list *slp;
    snmp_res_lock(MT_LIBRARY_ID, MT_LIB_SESSION);
    for(slp = Sessions; slp; slp = slp->next){
        snmp_sess_read((void *)slp, fdset);
    }
    snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_SESSION);
}

/* Same as snmp_read, but works just one session. */
/* returns 0 if success, -1 if fail */
/* MTR: can't lock here and at snmp_read */
/* Beware recursive send maybe inside snmp_read callback function. */
int
_sess_read(void *sessp,
	       fd_set *fdset)
{
    struct session_list *slp = (struct session_list *)sessp;
    struct snmp_session *sp;
    struct snmp_internal_session *isp;
    u_char packet[PACKET_LENGTH], *packetptr = packet, *ucp = 0;
    snmp_ipaddr        from;
    struct sockaddr_in *fromIp = (struct sockaddr_in *)&from;
    size_t length = 0;
    struct snmp_pdu *pdu;
    struct request_list *rp, *orp = NULL;
    int ret;
    int addrlen;
    int fromlength;

    sp = slp->session; isp = slp->internal;
    if (!sp || !isp) {
      DEBUGMSGTL(("sess_read","read fail: closing...\n"));
      return 0;
    }

    if ((!isp->newpkt && !(FD_ISSET(isp->sd, fdset)))) {
      DEBUGMSGTL(("sess_read","not reading...\n"));
      return 0;
    }
    
    sp->s_snmp_errno = 0;
    sp->s_errno = 0;

    if ( sp->flags & SNMP_FLAGS_STREAM_SOCKET ) {
        if ( sp->flags & SNMP_FLAGS_LISTENING ) {
		/*
		 * Accept the new stream based connection,
		 * and create a new session for it.
		 */
            struct session_list *new_slp;
            int new_sd;

            addrlen = sizeof(struct sockaddr);
            new_sd = accept(isp->sd, (struct sockaddr *)&(isp->addr), &addrlen);
            if ( new_sd == -1 ) {
	        sp->s_snmp_errno = SNMPERR_BAD_RECVFROM;
	        sp->s_errno = errno;
	        snmp_set_detail(strerror(errno));
	        return -1;
            }

            new_slp = snmp_sess_copy( sp );
            if ( new_slp == NULL )
                return -1;
    { /*MTCRITICAL_RESOURCE*/
	/* indirectly accesses the Sessions list */
            /* MTR snmp_res_lock(MT_LIBRARY_ID, MT_LIB_SESSION); */
            new_slp->next = slp->next;
            slp->next     = new_slp;

            sp  = new_slp->session;
            isp = new_slp->internal;
            memcpy((u_char *)isp,
                   (u_char *)slp->internal,
                   sizeof(*slp->internal));
            isp->sd = new_sd;
            isp->addr.sa_family = isp->me.sa_family;
            sp->flags &= (~SNMP_FLAGS_LISTENING);
            /* MTR snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_SESSION); */
    } /*END MTCRITICAL_RESOURCE*/
        }
        memcpy((u_char *)&from, (u_char *)&(isp->addr), sizeof( isp->addr ));
    }
    else
        memset(&from, 0, sizeof(from));
    fromlength = sizeof from;
#if !defined(__ECOS)
    if ( sp->flags & SNMP_FLAGS_STREAM_SOCKET ) {
      if (!isp->newpkt)
        length = recv(isp->sd, (char *)packet, PACKET_LENGTH, 0);
    } else {
#endif
        length = recvfrom(isp->sd, (char *)packet, PACKET_LENGTH, 0,
		      (struct sockaddr *)&from, &fromlength);
        if (from.sa_family == AF_UNSPEC)
            from.sa_family = AF_INET; /* bad bad bad OS, no bone! */
#if !defined(__ECOS)
    }
#endif

    if (length == -1) {
	sp->s_snmp_errno = SNMPERR_BAD_RECVFROM;
	sp->s_errno = errno;
	snmp_set_detail(strerror(errno));
	return -1;
    }

		/* Remote end closed connection */
    if ((length == 0 && !isp->newpkt) &&
        (sp->flags & SNMP_FLAGS_STREAM_SOCKET )) {
        isp->sd = -1;	/* Mark session for deletion */
        /* XXX: its not properly closing... */
		/* Don't unlink the server listening socket prematurely */
#ifdef AF_UNIX
        if (( isp->me.sa_family == AF_UNIX ) &&
           !( sp->flags & SNMP_FLAGS_LISTENING ))
                  isp->me.sa_family = AF_UNSPEC;
#endif /*AF_UNIX */
        return -1;
    }

    if (sp->flags & SNMP_FLAGS_STREAM_SOCKET ) {

      if (isp->newpkt == 1) {
        /* move the old memory down, if we have saved data */
        memmove(isp->packet, isp->packet+isp->proper_len, isp->proper_len);
        isp->newpkt = 0;
        isp->packet_len -= isp->proper_len;
        isp->proper_len = 0;
      }

      /* malloc the save space if needed */
      if (isp->packet == NULL) {
        isp->packet_size = (PACKET_LENGTH < length)?length:PACKET_LENGTH;
        isp->packet = (u_char *) malloc(isp->packet_size);
      }

      /* do we have enough space? */
      if (isp->packet_size < (isp->packet_len + length)) {
        if (isp->packet_size+length > MAX_PACKET_LENGTH) {
          /* maximum length exceeded, drop connection */
          snmp_log(LOG_ERR,"Maximum saved packet size exceeded.\n");
          isp->sd = -1;
          /* Don't unlink the server listening socket prematurely */
          /* XXX: do this?? */
#ifdef AF_UNIX
        if (( isp->me.sa_family == AF_UNIX ) &&
           !( sp->flags & SNMP_FLAGS_LISTENING ))
                  isp->me.sa_family = AF_UNSPEC;
#endif /*AF_UNIX */
          return -1;
        }
        isp->packet_size = isp->packet_size*2;
        if (isp->packet_size < (isp->packet_len + length))
          isp->packet_size = (isp->packet_len + length); /* shouldn't happen */
        isp->packet = (u_char *) realloc(isp->packet, isp->packet_size);
      }

      /* add the new data to the end of our buffer */
      memcpy(isp->packet+isp->packet_len, packet, length);
      isp->packet_len += length;
      
      /* check for agentx length parser */
      if (isp->proper_len == 0) {
        /* get the total data length we're expecting (and need to wait for) */
        if (isp->check_packet)
          isp->proper_len = isp->check_packet(isp->packet, isp->packet_len);
        else
          isp->proper_len = asn_check_packet(isp->packet, isp->packet_len);

        if (isp->proper_len > MAX_PACKET_LENGTH) {
          /* illegal length, drop the connection */
          snmp_log(LOG_ERR,"Maximum packet size exceeded in a request.\n");
          isp->sd = -1;
          /* Don't unlink the server listening socket prematurely */
          /* XXX: do this?? */
#ifdef AF_UNIX
        if (( isp->me.sa_family == AF_UNIX ) &&
           !( sp->flags & SNMP_FLAGS_LISTENING ))
                  isp->me.sa_family = AF_UNSPEC;
#endif /*AF_UNIX */
          return -1;
        }
      }
      
      /* if its not long enough now, give up and contiune waiting */
      if (isp->proper_len == 0 || isp->packet_len < isp->proper_len) {
        DEBUGMSGTL(("sess_read", "short packet!  (%d/%d)\n",
                    isp->packet_len, isp->proper_len));
        return 0;
      }

      /* else we need to continue, and process the saved data.
         Careful though, we may have more than is needed! save it! */
      packetptr = isp->packet;
      length = isp->proper_len;
      if (isp->packet_len - isp->proper_len == 0) {
        isp->packet_len -= isp->proper_len;
        isp->proper_len = 0;
      } else if (isp->packet_len - isp->proper_len < 0) {
        snmp_log(LOG_ERR,"something seriously wrong, packet size calculations are negative.\n");
        isp->packet_len = 0;
        isp->proper_len = 0;
      } else if (isp->packet_len - isp->proper_len > 0) {
        isp->newpkt = 1;
      }
    }

    if (ds_get_boolean(DS_LIBRARY_ID, DS_LIB_DUMP_PACKET)){
      snmp_log(LOG_DEBUG, "\nReceived %d bytes from %s:%hu\n", length,
               inet_ntoa(fromIp->sin_addr), ntohs(fromIp->sin_port));
      xdump(packetptr, length, "");
    }
    if ( isp->hook_pre ) {
      if ( isp->hook_pre( sp, from ) == 0 )
        return -1;
    }

    pdu = (struct snmp_pdu *)malloc(sizeof(struct snmp_pdu));
    memset (pdu, 0, sizeof(*pdu));
    pdu->address = from;

    if ( isp->hook_parse )
      ret = isp->hook_parse(sp, pdu, packetptr, length);
    else
      ret = snmp_parse(sessp, sp, pdu, packetptr, length);
    if ( isp->hook_post ) {
      if ( isp->hook_post( sp, pdu, ret ) == 0 ) {
        snmp_free_pdu(pdu);
        return -1;
      }
    }
    if (ret != SNMP_ERR_NOERROR) {
      snmp_free_pdu(pdu);
      return -1;
    }

    if (pdu->flags & UCD_MSG_FLAG_RESPONSE_PDU) {
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
      /* call USM to free any securityStateRef supplied with the message */
      if (pdu->securityStateRef) {
        usm_free_usmStateReference(pdu->securityStateRef);
        pdu->securityStateRef = NULL;
      }
#endif
      for(rp = isp->requests; rp; orp = rp, rp = rp->next_request) {
        snmp_callback callback;
        void *magic;
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
        if (pdu->version == SNMP_VERSION_3) {
          /* msgId must match for V3 messages */
          if (rp->message_id != pdu->msgid) continue;
          /* check that message fields match original,
           * if not, no further processing */
          if (!snmpv3_verify_msg(rp,pdu)) break;
        } else {
#endif
          if (rp->request_id != pdu->reqid) continue;
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
        }
#endif
        if (rp->callback) {
          callback = rp->callback;
          magic = rp->cb_data;
        } else {
          callback = sp->callback;
          magic = sp->callback_magic;
        }

        /* MTR snmp_res_lock(MT_LIBRARY_ID, MT_LIB_SESSION);  ?* XX lock should be per session ! */
        if (callback == NULL || 
            callback(RECEIVED_MESSAGE,sp,pdu->reqid,pdu,magic) == 1){
          if (pdu->command == SNMP_MSG_REPORT) {
            if (sp->s_snmp_errno == SNMPERR_NOT_IN_TIME_WINDOW) {
              /* trigger immediate retry on recoverable Reports 
               * (notInTimeWindow), incr_retries == TRUE to prevent
               * inifinite resend 		       */
              if (rp->retries <= sp->retries) {
                snmp_resend_request(slp, rp, TRUE);
                break;
              }
            } else {
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
              if (SNMPV3_IGNORE_UNAUTH_REPORTS) break;
#endif
            }
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
            /* handle engineID discovery - */
            if (!sp->securityEngineIDLen && pdu->securityEngineIDLen) {
              sp->securityEngineID = (u_char *)malloc(pdu->securityEngineIDLen);
              memcpy(sp->securityEngineID, pdu->securityEngineID,
                     pdu->securityEngineIDLen);
              sp->securityEngineIDLen = pdu->securityEngineIDLen;
              if (!sp->contextEngineIDLen) {
                sp->contextEngineID = (u_char *)malloc(pdu->securityEngineIDLen);
                memcpy(sp->contextEngineID, pdu->securityEngineID,
                       pdu->securityEngineIDLen);
                sp->contextEngineIDLen = pdu->securityEngineIDLen;
              }
            }
#endif
          }
          /* successful, so delete request */
          if (isp->requests == rp){
            /* first in list */
            isp->requests = rp->next_request;
            if (isp->requestsEnd == rp)
              isp->requestsEnd = NULL;
          } else {
            orp->next_request = rp->next_request;
            if (isp->requestsEnd == rp)
              isp->requestsEnd = orp;
          }
          snmp_free_pdu(rp->pdu);
          free((char *)rp);
          /* there shouldn't be any more requests with the
             same reqid */
          break;
        }
        /* MTR snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_SESSION);  ?* XX lock should be per session ! */
      }
    } else {
      if (sp->callback)
        {
          /* MTR snmp_res_lock(MT_LIBRARY_ID, MT_LIB_SESSION); */
          sp->callback(RECEIVED_MESSAGE, sp, pdu->reqid, pdu,
                       sp->callback_magic);
          /* MTR snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_SESSION); */
        }	
    }
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
    /* call USM to free any securityStateRef supplied with the message */
    if (pdu->securityStateRef && pdu->command == SNMP_MSG_TRAP2) {
      usm_free_usmStateReference(pdu->securityStateRef);
      pdu->securityStateRef = NULL;
    }
#endif
    snmp_free_pdu(pdu);
    return 0;
}

/* returns 0 if success, -1 if fail */
int
snmp_sess_read(void *sessp,
	       fd_set *fdset)
    {
    struct session_list *psl;
    struct snmp_session *pss;
    int rc;

    rc = _sess_read(sessp, fdset);
    psl = (struct session_list *)sessp;
    pss = psl->session;
    if (rc && pss->s_snmp_errno) {
        SET_SNMP_ERROR(pss->s_snmp_errno);
    }
    return rc;
}


/*
 * Returns info about what snmp requires from a select statement.
 * numfds is the number of fds in the list that are significant.
 * All file descriptors opened for SNMP are OR'd into the fdset.
 * If activity occurs on any of these file descriptors, snmp_read
 * should be called with that file descriptor set
 *
 * The timeout is the latest time that SNMP can wait for a timeout.  The
 * select should be done with the minimum time between timeout and any other
 * timeouts necessary.  This should be checked upon each invocation of select.
 * If a timeout is received, snmp_timeout should be called to check if the
 * timeout was for SNMP.  (snmp_timeout is idempotent)
 *
 * The value of block indicates how the timeout value is interpreted.
 * If block is true on input, the timeout value will be treated as undefined,
 * but it must be available for setting in snmp_select_info.  On return,
 * block is set to true if the value returned for timeout is undefined;
 * when block is set to false, timeout may be used as a parmeter to 'select'.
 *
 * snmp_select_info returns the number of open sockets.  (i.e. The number of
 * sessions open)
 */

int
snmp_select_info(int *numfds,
		 fd_set *fdset,
		 struct timeval *timeout,
		 int *block)
    /* input:  set to 1 if input timeout value is undefined  */
    /*         set to 0 if input timeout value is defined    */
    /* output: set to 1 if output timeout value is undefined */
    /*         set to 0 if output rimeout vlaue id defined   */
{
    return snmp_sess_select_info((void *)0, numfds, fdset, timeout, block);
}

/* Same as snmp_select_info, but works just one session. */
int
snmp_sess_select_info(void *sessp,
		      int *numfds,
		      fd_set *fdset,
		      struct timeval *timeout,
		      int *block)
{
    struct session_list *slptest = (struct session_list *)sessp;
    struct session_list *slp, *next=NULL, *prev=NULL;
    struct snmp_internal_session *isp;
    struct request_list *rp;
    struct timeval now, earliest;
    int timer_set = 0;
    int active = 0, requests = 0;
    int next_alarm = 0;

    timerclear(&earliest);
    /*
     * For each request outstanding, add it's socket to the fdset,
     * and if it is the earliest timeout to expire, mark it as lowest.
     * If a single session is specified, do just for that session.
     */
    if (sessp) slp = slptest; else slp = Sessions;
    for(; slp; slp = next){
	isp = slp->internal;
        if (!isp) {
          DEBUGMSGTL(("sess_select","select fail: closing...\n"));
          continue;  /* close in progress - skip this one */
        }

	if (isp->sd == -1) {
	    if (sessp == NULL) {
		/* This session was marked for deletion */
	    if ( prev == NULL )
		Sessions = slp->next;
	    else
		prev->next = slp->next;
	    next = slp->next;
	    }
	    snmp_sess_close( slp );
	    continue;
	}
	if ((isp->sd + 1) > *numfds)
	    *numfds = (isp->sd + 1);
	FD_SET(isp->sd, fdset);
	if (isp->requests){
	    /* found another session with outstanding requests */
	    requests++;
	    for(rp = isp->requests; rp; rp = rp->next_request){
		if ((!timerisset(&earliest)
		    || (timercmp(&rp->expire, &earliest, <))))
		    earliest = rp->expire;
	    }
	}
        if (isp->newpkt) {
          /* don't block at all, more data waiting to be processed */
          DEBUGMSGTL(("sess_select","more data in buffer, not blocking\n"));
          requests++;
          timer_set = 1;
          *block = 0;
        }
	active++;
	if (slp == slptest) break;
	prev = slp;
	next = slp->next;
    }

    if (ds_get_boolean(DS_LIBRARY_ID, DS_LIB_ALARM_DONT_USE_SIG)) {
      next_alarm = get_next_alarm_delay_time();
    }
    if (next_alarm == 0 && requests == 0) { /* if none are active, skip arithmetic */
       *block = 1; /* can block - timeout value is undefined if no requests*/
	return active;
    }

    /*
     * Now find out how much time until the earliest timeout.  This
     * transforms earliest from an absolute time into a delta time, the
     * time left until the select should timeout.
     */
    gettimeofday(&now,(struct timezone *)0);
    /*Now = now;*/

    if (next_alarm != 0 && earliest.tv_sec > next_alarm) {
      earliest.tv_sec = next_alarm;
      earliest.tv_usec = 0;
    }

    if (timer_set || earliest.tv_sec < now.tv_sec) {
       earliest.tv_sec  = 0;
       earliest.tv_usec = 100;
    }
    else if (earliest.tv_sec == now.tv_sec) {
       earliest.tv_sec  = 0;
       earliest.tv_usec = (earliest.tv_usec - now.tv_usec);
       if (earliest.tv_usec < 0) {
          earliest.tv_usec = 100;
       }
    }
    else {
       earliest.tv_sec  = (earliest.tv_sec  - now.tv_sec);
       earliest.tv_usec = (earliest.tv_usec - now.tv_usec);
       if (earliest.tv_usec < 0) {
          earliest.tv_sec --;
          earliest.tv_usec = (1000000L + earliest.tv_usec);
       }
    }

    /* if it was blocking before or our delta time is less, reset timeout */
    if ((*block || (timercmp(&earliest, timeout, <)))){
	*timeout = earliest;
	*block = 0;
    }
    return active;
}

/*
 * snmp_timeout should be called whenever the timeout from snmp_select_info
 * expires, but it is idempotent, so snmp_timeout can be polled (probably a
 * cpu expensive proposition).  snmp_timeout checks to see if any of the
 * sessions have an outstanding request that has timed out.  If it finds one
 * (or more), and that pdu has more retries available, a new packet is formed
 * from the pdu and is resent.  If there are no more retries available, the
 *  callback for the session is used to alert the user of the timeout.
 */
void
snmp_timeout (void)
{
    struct session_list *slp;
    snmp_res_lock(MT_LIBRARY_ID, MT_LIB_SESSION);
    for(slp = Sessions; slp; slp = slp->next){
	snmp_sess_timeout((void *)slp);
    }
    snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_SESSION);
}

static int
snmp_resend_request(struct session_list *slp, struct request_list *rp, 
		    int incr_retries)
{
  u_char  packet[PACKET_LENGTH];
  size_t length = PACKET_LENGTH;
  struct timeval tv;
  struct snmp_session *sp;
  struct snmp_internal_session *isp;
  struct timeval now;
  int result, addr_size;

  sp = slp->session; isp = slp->internal;
  if (!sp || !isp) {
      DEBUGMSGTL(("sess_read","resend fail: closing...\n"));
      return 0;
  }

  if (incr_retries) rp->retries++;

  /* always increment msgId for resent messages */
  rp->pdu->msgid = rp->message_id = snmp_get_next_msgid();

  /* retransmit this pdu */
  if (snmp_build(sp, rp->pdu, packet, &length) < 0){
    /* this should never happen */
    return -1;
  }
  if (ds_get_boolean(DS_LIBRARY_ID, DS_LIB_DUMP_PACKET)){
    struct sockaddr_in *pduIp;
    pduIp = (struct sockaddr_in *)&(rp->pdu->address);
    snmp_log(LOG_DEBUG, "\nResending %d bytes to %s:%hu\n", length,
	   inet_ntoa(pduIp->sin_addr), ntohs(pduIp->sin_port));
    xdump(packet, length, "");
  }

    addr_size = snmp_socket_length(rp->pdu->address.sa_family);

#if !defined(__ECOS)
  if ( sp->flags & SNMP_FLAGS_STREAM_SOCKET )
    result = send(isp->sd, (char *)packet, length, 0);
  else
#endif
    result = sendto(isp->sd, (char *)packet, length, 0,
	     (struct sockaddr *)&rp->pdu->address, addr_size);
  if ( result < 0){
    sp->s_snmp_errno = SNMPERR_BAD_SENDTO;
    sp->s_errno = errno;
    snmp_set_detail(strerror(errno));
    return -1;
  }
  else {
    gettimeofday(&now, (struct timezone *)0);
    tv = now;
    rp->time = tv;
    tv.tv_usec += rp->timeout;
    tv.tv_sec += tv.tv_usec / 1000000L;
    tv.tv_usec %= 1000000L;
    rp->expire = tv;
  }
  return 0;
}

void
snmp_sess_timeout(void *sessp)
{
    struct session_list *slp = (struct session_list*)sessp;
    struct snmp_session *sp;
    struct snmp_internal_session *isp;
    struct request_list *rp, *orp = NULL, *freeme = NULL;
    struct timeval now;
    snmp_callback callback;
    void *magic;

    sp = slp->session; isp = slp->internal;
    if (!sp || !isp) {
      DEBUGMSGTL(("sess_read","timeout fail: closing...\n"));
      return;
    }

    gettimeofday(&now,(struct timezone *)0);

    /*
     * For each request outstanding, check to see if it has expired.
     */
    for(rp = isp->requests; rp; rp = rp->next_request){
        if (freeme != NULL){
    	/* frees rp's after the for loop goes on to the next_request */
    	free((char *)freeme);
    	freeme = NULL;
        }
        if ((timercmp(&rp->expire, &now, <))){
    	/* this timer has expired */
    	if (rp->retries >= sp->retries){
            if (rp->callback) {
              callback = rp->callback;
              magic = rp->cb_data;
            } else {
              callback = sp->callback;
              magic = sp->callback_magic;
            }
    	    /* No more chances, delete this entry */
    	    if (callback)
		callback(TIMED_OUT, sp, rp->pdu->reqid, rp->pdu, magic);
    	    if (isp->requests == rp){
    		isp->requests = rp->next_request;
    		if (isp->requestsEnd == rp)
    		    isp->requestsEnd = NULL;
    	    } else {
    		orp->next_request = rp->next_request;
    		if (isp->requestsEnd == rp)
    		    isp->requestsEnd = orp;
    	    }
    	    snmp_free_pdu(rp->pdu);	/* FIX  rp is already free'd! */
    	    freeme = rp;
    	    continue;	/* don't update orp below */
    	} else {
	  if (snmp_resend_request(slp, rp, TRUE)) break;
    	}
        }
        orp = rp;
    }
    if (freeme != NULL){
        free((char *)freeme);
        freeme = NULL;
    }
}

/* lexicographical compare two object identifiers.
 * Returns -1 if name1 < name2,
 *          0 if name1 = name2,
 *          1 if name1 > name2
 *
 * Caution: this method is called often by
 *          command responder applications (ie, agent).
 */
int
snmp_oid_compare(const oid *in_name1, 
		 size_t len1,
		 const oid *in_name2, 
		 size_t len2)
{
    register int len, res;
    register const oid * name1 = in_name1;
    register const oid * name2 = in_name2;

    /* len = minimum of len1 and len2 */
    if (len1 < len2)
	len = len1;
    else
	len = len2;
    /* find first non-matching OID */
    while(len-- > 0){
	res = *(name1++) - *(name2++);
	if (res < 0)
	    return -1;
	if (res > 0)
	    return 1;
    }
    /* both OIDs equal up to length of shorter OID */
    if (len1 < len2)
	return -1;
    if (len2 < len1)
	return 1;
    return 0;
}

/*
 * Add a variable with the requested name to the end of the list of
 * variables for this pdu.
 */
struct variable_list *
snmp_pdu_add_variable(struct snmp_pdu *pdu,
		      oid *name,
		      size_t name_length,
		      u_char type,
		      u_char *value,
		      size_t len)
{
  return snmp_varlist_add_variable(&pdu->variables, name, name_length, type,
                                   value, len);
}

/*
 * Add a variable with the requested name to the end of the list of
 * variables for this pdu.
 */
struct variable_list *
snmp_varlist_add_variable(struct variable_list **varlist,
		      oid *name,
		      size_t name_length,
		      u_char type,
		      u_char *value,
		      size_t len)
{
    struct variable_list *vars, *vtmp;
    int largeval = 1;

    if (varlist == NULL)
      return NULL;

    vars = (struct variable_list *)malloc(sizeof(struct variable_list));
    if (vars == NULL)
      return NULL;

    vars->next_variable = 0; vars->name = 0; vars->val.string = 0;

    /* use built-in storage for smaller values */
    if (len <= sizeof(vars->buf)) {
        vars->val.string = (u_char *)vars->buf;
        largeval = 0;
    }

    vars->type = type;
    vars->val_len = len;
    switch(type){
      case ASN_INTEGER:
      case ASN_UNSIGNED:
      case ASN_TIMETICKS:
      case ASN_IPADDRESS:
      case ASN_COUNTER:
        memmove(vars->val.integer, value, vars->val_len);
        vars->val_len = sizeof(long);
        break;

      case ASN_OBJECT_ID:
      case ASN_PRIV_INCL_RANGE:
      case ASN_PRIV_EXCL_RANGE:
        if (largeval) {
            vars->val.objid = (oid *)malloc(vars->val_len);
        }
        memmove(vars->val.objid, value, vars->val_len);
        break;

      case ASN_OCTET_STR:
      case ASN_OPAQUE:
      case ASN_NSAP:
        if (largeval) {
            vars->val.string = (u_char *)malloc(vars->val_len);
        }
        memmove(vars->val.string, value, vars->val_len);
        break;

      case SNMP_NOSUCHOBJECT:
      case SNMP_NOSUCHINSTANCE:
      case SNMP_ENDOFMIBVIEW:
      case ASN_NULL:
        vars->val_len = 0;
        vars->val.string = NULL;
        break;

#ifdef OPAQUE_SPECIAL_TYPES
      case ASN_OPAQUE_U64:
      case ASN_OPAQUE_I64:
#endif /* OPAQUE_SPECIAL_TYPES */
      case ASN_COUNTER64:
        vars->val_len = sizeof(struct counter64);
        memmove(vars->val.counter64, value, vars->val_len);
        break;

#ifdef OPAQUE_SPECIAL_TYPES
      case ASN_OPAQUE_FLOAT:
        vars->val_len = sizeof(float);
        memmove(vars->val.floatVal, value, vars->val_len);
        break;
      
      case ASN_OPAQUE_DOUBLE:
        vars->val_len = sizeof(double);
        memmove(vars->val.doubleVal, value, vars->val_len);

#endif /* OPAQUE_SPECIAL_TYPES */
      
      default:
        snmp_set_detail("Internal error in type switching\n");
        snmp_free_var(vars);
        return (0);
    }

    if (name != NULL && snmp_set_var_objid(vars, name, name_length)) {
        snmp_free_var(vars);
        return (0);
    }
    
    /* put only qualified variable onto varlist */
    if (*varlist == NULL){
      *varlist = vars;
    } else {
      for(vtmp = *varlist;
            vtmp->next_variable;
            vtmp = vtmp->next_variable)
        ;

      vtmp->next_variable = vars;
    }

	return vars;
}

/*
 * Parses dotted notation object identifier
 * into unsigned character array.
 * Returns: SNMPERR_RANGE if any sub-identifier > 255.
 * Returns: SNMPERR_VALUE if input string is not octet string.
 * Returns: non-negative number of sub-identifiers parsed,
 */
int
ascii_to_binary(const char *cp,
		u_char *bufp)
{
    int  subidentifier;
    u_char *bp = bufp;

    for(; *cp != '\0'; cp++){
      if (isspace(*cp) || *cp == '.')
        continue;
      if (!isdigit(*cp)){
        return SNMPERR_VALUE;
      }
      subidentifier = atoi(cp);
      if (subidentifier > 255){
        return SNMPERR_RANGE;
      }
      *bp++ = (u_char)subidentifier;
      while(isdigit(*cp))
        cp++;
      cp--;
    }
    return bp - bufp;
}

int
hex_to_binary(const char *str,
	      u_char *bufp)
{
  int len, itmp;
  if (!bufp) return -1;
  if (*str && *str == '0' && (*(str+1) == 'x' || *(str+1) == 'X')) str += 2;
  for (len = 0; *str; str++) {
    if (isspace(*str)) continue;
    if (!isxdigit(*str)) return -1;
    len++;
    if (sscanf(str++, "%2x", &itmp) == 0) return -1;
    *bufp++ = itmp;
    if (!*str) return -1; /* odd number of chars is an error */
  }
  return len;
}


/*
 * Add a variable with the requested name to the end of the list of
 * variables for this pdu.
 * Returns:
 * may set these error types :
 * SNMPERR_RANGE - type, value, or length not found or out of range
 * SNMPERR_VALUE - value is not correct
 * SNMPERR_BAD_NAME - name is not found
 *
 * returns 0 if success, error if failure.
 */
int
snmp_add_var(struct snmp_pdu *pdu,
	     oid *name,
	     size_t name_length,
	     char type,
	     const char *value)
{
    int result = 0;
    u_char buf[SPRINT_MAX_LEN];
    size_t tint;
    long ltmp;
    struct tree *tp;
    struct enum_list *ep;
    struct range_list *rp;
#ifdef OPAQUE_SPECIAL_TYPES
    double dtmp;
    float ftmp;
    struct counter64 c64tmp;
#endif /* OPAQUE_SPECIAL_TYPES */

    switch(type){
      case 'i':
	tp = get_tree(name, name_length, get_tree_head());
        if (sscanf(value, "%ld", &ltmp) != 1) {
	    ep = tp ? tp->enums : NULL;
	    while (ep) {
		if (strcmp(value, ep->label) == 0) {
		    ltmp = ep->value;
		    break;
		}
		ep = ep->next;
	    }
	    if (!ep) {
		result = SNMPERR_BAD_NAME;
		snmp_set_detail(value);
		break;
	    }
	}

	if (tp && tp->ranges && !ds_get_boolean(DS_LIBRARY_ID, DS_LIB_DONT_CHECK_RANGE)) {
	    rp = tp->ranges;
	    while (rp) {
		if (rp->low <= ltmp && ltmp <= rp->high) break;
		rp = rp->next;
	    }
	    if (!rp) {
		result = SNMPERR_RANGE;
		snmp_set_detail("Value");
		break;
	    }
	}
        snmp_pdu_add_variable(pdu, name, name_length, ASN_INTEGER,
                              (u_char *) &ltmp, sizeof(ltmp));
        break;

      case 'u':
        if (sscanf(value, "%lu", &ltmp) == 1)
	    snmp_pdu_add_variable(pdu, name, name_length, ASN_UNSIGNED,
				  (u_char *) &ltmp, sizeof(ltmp));
	else goto fail;
        break;

      case 'c':
        if (sscanf(value, "%lu", &ltmp) == 1)
	    snmp_pdu_add_variable(pdu, name, name_length, ASN_COUNTER,
				  (u_char *) &ltmp, sizeof(ltmp));
	else goto fail;
        break;

      case 't':
        if (sscanf(value, "%lu", &ltmp) == 1)
	    snmp_pdu_add_variable(pdu, name, name_length, ASN_TIMETICKS,
				  (u_char *) &ltmp, sizeof(long));
	else goto fail;
        break;

      case 'a':
        if ((ltmp = inet_addr(value)) != (long)-1)
	    snmp_pdu_add_variable(pdu, name, name_length, ASN_IPADDRESS,
				  (u_char *) &ltmp, sizeof(long));
	else goto fail;
        break;

      case 'o':
        tint = sizeof(buf) / sizeof(oid);
        if (read_objid(value, (oid *)buf, &tint))
            snmp_pdu_add_variable(pdu, name, name_length, ASN_OBJECT_ID, buf,
                              sizeof(oid)*tint);
	else result = snmp_errno;
        break;

      case 's':
      case 'x':
      case 'd':
        if (type == 'd'){
          ltmp = ascii_to_binary(value, buf);
        } else if (type == 's'){
          strcpy((char*)buf, value);
          ltmp = strlen((char*)buf);
        } else if (type == 'x'){
          ltmp = hex_to_binary(value, buf);
        }
        if (ltmp < 0) {
          result = SNMPERR_VALUE;
          snmp_set_detail(value);
          break;
        }
	tp = get_tree(name, name_length, get_tree_head());
	if (tp && tp->ranges && !ds_get_boolean(DS_LIBRARY_ID, DS_LIB_DONT_CHECK_RANGE)) {
	    rp = tp->ranges;
	    while (rp) {
		if (rp->low <= ltmp && ltmp <= rp->high) break;
		rp = rp->next;
	    }
	    if (!rp) {
		result = SNMPERR_RANGE;
		snmp_set_detail("Length");
		break;
	    }
	}
        snmp_pdu_add_variable(pdu, name, name_length, ASN_OCTET_STR, buf, ltmp);
        break;

      case 'n':
        snmp_pdu_add_variable(pdu, name, name_length, ASN_NULL, 0, 0);
        break;

#ifdef OPAQUE_SPECIAL_TYPES
      case 'U':
        if (read64(&c64tmp, value))
	    snmp_pdu_add_variable(pdu, name, name_length, ASN_OPAQUE_U64,
				  (u_char *) &c64tmp, sizeof(c64tmp));
	else goto fail;
        break;

      case 'I':
        if (read64(&c64tmp, value))
	    snmp_pdu_add_variable(pdu, name, name_length, ASN_OPAQUE_I64,
				  (u_char *) &c64tmp, sizeof(c64tmp));
	else goto fail;
        break;

      case 'F':
	if (sscanf(value, "%f", &ftmp) == 1)
	    snmp_pdu_add_variable(pdu, name, name_length, ASN_OPAQUE_FLOAT, 
				  (u_char *) &ftmp, sizeof(ftmp));
	else goto fail;
        break;

      case 'D':
	if (sscanf(value, "%lf", &dtmp) == 1)
	    snmp_pdu_add_variable(pdu, name, name_length, ASN_OPAQUE_DOUBLE,
				  (u_char *) &dtmp, sizeof(dtmp));
	else goto fail;
        break;
#endif /* OPAQUE_SPECIAL_TYPES */

      default:
	result = SNMPERR_VAR_TYPE;
	sprintf((char *)buf, "%c", type);
	snmp_set_detail((const char *)buf);
        break;
    }

    SET_SNMP_ERROR(result);
    return result;

fail:
    result = SNMPERR_VALUE;
    snmp_set_detail(value);
    SET_SNMP_ERROR(result);
    return result;
}

/*
 * returns NULL or internal pointer to session
 * use this pointer for the other snmp_sess* routines,
 * which guarantee action will occur ONLY for this given session.
 */
void *
snmp_sess_pointer(struct snmp_session *session)
{
    struct session_list *slp;

    snmp_res_lock(MT_LIBRARY_ID, MT_LIB_SESSION);
    for(slp = Sessions; slp; slp = slp->next){
	if (slp->session == session){
	    break;
	}
    }
    snmp_res_unlock(MT_LIBRARY_ID, MT_LIB_SESSION);

    if (slp == NULL){
	snmp_errno = SNMPERR_BAD_SESSION; /*MTCRITICAL_RESOURCE*/
	return(NULL);
    }
    return((void *)slp);
}

/*
 * Input : an opaque pointer, returned by snmp_sess_open.
 * returns NULL or pointer to session.
 */
struct snmp_session *
snmp_sess_session(void *sessp)
{
    struct session_list *slp = (struct session_list *)sessp;
    if (slp == NULL) return(NULL);
    return (slp->session);
}
#ifdef CMU_COMPATIBLE

char *
snmp_pdu_type(struct snmp_pdu *PDU)
{
  switch(PDU->command) {
  case SNMP_MSG_GET:
    return("GET");
    break;
  case SNMP_MSG_GETNEXT:
    return("GETNEXT");
    break;
  case SNMP_MSG_RESPONSE:
    return("RESPONSE");
    break;
  case SNMP_MSG_SET:
    return("SET");
    break;
  case SNMP_MSG_GETBULK:
    return("GETBULK");
    break;
  case SNMP_MSG_INFORM:
    return("INFORM");
    break;
  case SNMP_MSG_TRAP2:
    return("V2TRAP");
    break;
  case SNMP_MSG_REPORT:
    return("REPORT");
    break;
    
  case SNMP_MSG_TRAP:
    return("V1TRAP");
    break;
  default:
    return("Unknown");
    break;
  }
}

/*
 * cmu_snmp_parse - emulate CMU library's snmp_parse.
 *
 * Parse packet, storing results into PDU.
 * Returns community string if success, NULL if fail.
 * WARNING: may return a zero length community string.
 *
 * Note:
 * Some CMU-aware apps call init_mib(), but do not
 * initialize a session.
 * Check Reqid to make sure that this module is initialized.
 */

u_char *
cmu_snmp_parse (struct snmp_session *session,
    struct snmp_pdu *pdu,
    u_char *data,
    size_t length)
{
    u_char *bufp = NULL;

    if (Reqid == 0) {
	snmp_sess_init(session); /* gimme a break! */
    }

    switch(pdu->version) {
    case SNMP_VERSION_1:
    case SNMP_VERSION_2c:
    case SNMP_DEFAULT_VERSION:
	    break;
    default:
	    return NULL;
    }
#ifndef NO_INTERNAL_VARLIST
    if (snmp_parse( 0, session, pdu, data, length) != SNMP_ERR_NOERROR){
	return NULL;
    }
#else
/*
 * while there are two versions of variable_list:
 * use an internal variable list for snmp_parse;
 * clone the result.
 */
if (1) {
struct snmp_pdu *snmp_clone_pdu (struct snmp_pdu *);
struct snmp_pdu *snmp_2clone_pdu(struct snmp_pdu *from_pdu, struct snmp_pdu *to_pdu);

    struct snmp_pdu *ipdu;
    ipdu = snmp_clone_pdu(pdu);
    if (snmp_parse( 0, session, ipdu, data, length) != SNMP_ERR_NOERROR){
	snmp_free_internal_pdu(ipdu);
	return NULL;
    }
    pdu = snmp_2clone_pdu(ipdu, pdu);
    snmp_free_internal_pdu(ipdu);
}
#endif /* NO_INTERNAL_VAR_LIST */

    /* Add a null to meet the caller's expectations. */

    bufp = (u_char *)malloc(1+pdu->community_len);
    if (bufp && pdu->community_len) {
	memcpy(bufp, pdu->community, pdu->community_len);
	bufp[pdu->community_len] = '\0';
    }
    return(bufp);
}


#endif /* CMU_COMPATIBLE */

/* snmp_duplicate_objid: duplicates (mallocs) an objid based on the
   input objid */
oid *
snmp_duplicate_objid(oid *objToCopy, size_t objToCopyLen)
{
  oid *returnOid;
  returnOid = (oid *) malloc(objToCopyLen*sizeof(oid));
  if (returnOid) {
    memmove(returnOid, objToCopy, objToCopyLen*sizeof(oid));
  }
  return returnOid;
}

/* generic statistics counter functions */
static u_int statistics[MAX_STATS];

u_int
snmp_increment_statistic(int which)
{
  if (which >= 0 && which < MAX_STATS) {
    statistics[which]++;
    return statistics[which];
  }
  return 0;
}

u_int
snmp_increment_statistic_by(int which, int count)
{
  if (which >= 0 && which < MAX_STATS) {
    statistics[which] += count;
    return statistics[which];
  }
  return 0;
}

u_int
snmp_get_statistic(int which)
{
  if (which >= 0 && which < MAX_STATS)
    return statistics[which];
  return 0;
}

void
snmp_init_statistics(void)
{
  memset(statistics, 0, sizeof(statistics));
}

/* returns the length of a socket structure */

size_t snmp_socket_length( int family)
{
  size_t length;
  switch (family)
    {
#ifndef cygwin
#ifndef WIN32
#ifdef AF_UNIX
    case AF_UNIX:
      length = sizeof (struct sockaddr_un);
      break;
#endif /* AF_UNIX */
#endif
#endif

#ifndef aix3
#ifdef AF_LINK
    case AF_LINK:
#ifdef _MAX_SA_LEN
      length = _MAX_SA_LEN;
#elif SOCK_MAXADDRLEN
      length = SOCK_MAXADDRLEN;
#else
      length = sizeof (struct sockaddr_dl);
#endif
      break;
#endif /* AF_LINK */
#endif

    case AF_INET:
      length = sizeof (struct sockaddr_in);
      break;
    default:
      length = sizeof (struct sockaddr);
      break;
    }

    return length;
}

/*
 * For compatibility with applications built using
 * previous versions only.
 */

/* use <struct snmp_session *)->s_snmp_errno instead */
int  snmp_get_errno   (void)  { return SNMPERR_SUCCESS; }

/* synch_reset and synch_setup are no longer used. */
void snmp_synch_reset (struct snmp_session * notused) {}
void snmp_synch_setup (struct snmp_session * notused) {}

/* provide for backwards compatibility */
void
snmp_set_dump_packet(int x) {
    ds_set_boolean(DS_LIBRARY_ID, DS_LIB_DUMP_PACKET, x);
}

int
snmp_get_dump_packet(void) {
    return ds_get_boolean(DS_LIBRARY_ID, DS_LIB_DUMP_PACKET);
}

void
snmp_set_quick_print(int x) {
    ds_set_boolean(DS_LIBRARY_ID, DS_LIB_QUICK_PRINT, x);
}
  
int
snmp_get_quick_print(void) {
    return ds_get_boolean(DS_LIBRARY_ID, DS_LIB_QUICK_PRINT);
}
 

void
snmp_set_suffix_only(int x) {
    ds_set_int(DS_LIBRARY_ID, DS_LIB_PRINT_SUFFIX_ONLY, x);
}
  
int
snmp_get_suffix_only(void) {
    return ds_get_int(DS_LIBRARY_ID, DS_LIB_PRINT_SUFFIX_ONLY);
}
 
void
snmp_set_full_objid(int x) {
      ds_set_boolean(DS_LIBRARY_ID, DS_LIB_PRINT_FULL_OID, x);
}

int
snmp_get_full_objid(void) {
    return ds_get_boolean(DS_LIBRARY_ID, DS_LIB_PRINT_SUFFIX_ONLY);
}
 
void
snmp_set_random_access(int x) {
    ds_set_boolean(DS_LIBRARY_ID, DS_LIB_RANDOM_ACCESS, x);
}
 
int
snmp_get_random_access(void) {
    return ds_get_boolean(DS_LIBRARY_ID, DS_LIB_RANDOM_ACCESS);
}

