//==========================================================================
//
//      ./lib/current/src/snmpv3.c
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
 * snmpv3.c
 */

#include <config.h>

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
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
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_STDLIB_H
#       include <stdlib.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include "system.h"
#include "asn1.h"
#include "snmpv3.h"
#include "callback.h"
#include "snmpusm.h"
#include "snmp.h"
#include "snmp_api.h"
#include "snmp_impl.h"
#include "read_config.h"
#include "lcd_time.h"
#include "scapi.h"
#include "tools.h"
#include "keytools.h"
#include "lcd_time.h"
#include "snmp_debug.h"
#include "snmp_logging.h"
#include "default_store.h"

#include "transform_oids.h"

static u_long		 engineBoots	   = 1;
static unsigned char	*engineID	   = NULL;
static size_t		 engineIDLength	   = 0;
static unsigned char	*oldEngineID	   = NULL;
static size_t		 oldEngineIDLength = 0;
static struct timeval	 snmpv3starttime;

/* 
 * Set up default snmpv3 parameter value storage.
 */
static oid	*defaultAuthType	= NULL;
static size_t	 defaultAuthTypeLen	= 0;
static oid	*defaultPrivType	= NULL;
static size_t	 defaultPrivTypeLen	= 0;

void
snmpv3_authtype_conf(const char *word, char *cptr)
{
  if (strcasecmp(cptr,"MD5") == 0)
    defaultAuthType = usmHMACMD5AuthProtocol;
  else if (strcasecmp(cptr,"SHA") == 0)
    defaultAuthType = usmHMACMD5AuthProtocol;
  else
    config_perror("Unknown authentication type");
  defaultAuthTypeLen = USM_LENGTH_OID_TRANSFORM;
  DEBUGMSGTL(("snmpv3","set default authentication type: %s\n", cptr));
}

oid *
get_default_authtype(size_t *len)
{
  if (defaultAuthType == NULL) {
    defaultAuthType = SNMP_DEFAULT_AUTH_PROTO;
    defaultAuthTypeLen = SNMP_DEFAULT_AUTH_PROTOLEN;
  }
  if (len)
    *len = defaultAuthTypeLen;
  return defaultAuthType;
}

void
snmpv3_privtype_conf(const char *word, char *cptr)
{
  if (strcasecmp(cptr,"DES") == 0)
    defaultPrivType = SNMP_DEFAULT_PRIV_PROTO;
  else
    config_perror("Unknown privacy type");
  defaultPrivTypeLen = SNMP_DEFAULT_PRIV_PROTOLEN;
  DEBUGMSGTL(("snmpv3","set default privacy type: %s\n", cptr));
}

oid *
get_default_privtype(size_t *len)
{
  if (defaultAuthType == NULL) {
    defaultAuthType = usmDESPrivProtocol;
    defaultPrivTypeLen = USM_LENGTH_OID_TRANSFORM;
  }
  if (len)
    *len = defaultPrivTypeLen;
  return defaultPrivType;
}

/*******************************************************************-o-******
 * snmpv3_secLevel_conf
 *
 * Parameters:
 *	*word
 *	*cptr
 *
 * Line syntax:
 *	defSecurityLevel "noAuthNoPriv" | "authNoPriv" | "authPriv"
 */
void
snmpv3_secLevel_conf(const char *word, char *cptr)
{
  char buf[1024];
  
  if (strcasecmp(cptr,"noAuthNoPriv") == 0 || strcmp(cptr, "1") == 0
	|| strcasecmp(cptr, "nanp") == 0)
    ds_set_int(DS_LIBRARY_ID, DS_LIB_SECLEVEL, SNMP_SEC_LEVEL_NOAUTH);
  else if (strcasecmp(cptr,"authNoPriv") == 0 || strcmp(cptr, "2") == 0
	|| strcasecmp(cptr, "anp") == 0)
    ds_set_int(DS_LIBRARY_ID, DS_LIB_SECLEVEL, SNMP_SEC_LEVEL_AUTHNOPRIV);
  else if (strcasecmp(cptr,"authPriv") == 0 || strcmp(cptr, "3") == 0
	|| strcasecmp(cptr, "ap") == 0)
    ds_set_int(DS_LIBRARY_ID, DS_LIB_SECLEVEL, SNMP_SEC_LEVEL_AUTHPRIV);
  else {
    sprintf(buf,"Unknown security level: %s", cptr);
    config_perror(buf);
  }
  DEBUGMSGTL(("snmpv3","default secLevel set to: %s = %d\n", cptr,
              ds_get_int(DS_LIBRARY_ID, DS_LIB_SECLEVEL)));
}

/*******************************************************************-o-******
 * setup_engineID
 *
 * Parameters:
 *	**eidp
 *	 *text	Printable (?) text to be plugged into the snmpEngineID.
 *
 * Return:
 *	Length of allocated engineID string in bytes,  -OR-
 *	-1 on error.
 *
 *
 * Create an snmpEngineID using text and the local IP address.  If eidp
 * is defined, use it to return a pointer to the newly allocated data.
 * Otherwise, use the result to define engineID defined in this module.
 *
 * Line syntax:
 *	engineID <text> | NULL
 *
 * XXX	What if a node has multiple interfaces?
 * XXX	What if multiple engines all choose the same address?
 *      (answer:  You're screwed, because you might need a kul database
 *       which is dependant on the current engineID.  Enumeration and other
 *       tricks won't work). 
 */
int
setup_engineID(u_char **eidp, const char *text)
{
  int		  enterpriseid	= htonl(ENTERPRISE_NUMBER),
		  localsetup	= (eidp) ? 0 : 1;
			/* Use local engineID if *eidp == NULL.  */
#ifdef HAVE_GETHOSTNAME
  u_char	  buf[SNMP_MAXBUF_SMALL];
  struct hostent *hent;
#endif
  u_char     *bufp = NULL;
  size_t	  len;
 

  /*
   * Determine length of the engineID string.
   */
  if (text) {
    len = 5+strlen(text);	/* 5 leading bytes+text. */

  } else {
    len = 5 + 4;		/* 5 leading bytes + four byte IPv4 address */
#ifdef HAVE_GETHOSTNAME
    gethostname((char *)buf, sizeof(buf));
    hent = gethostbyname((char *)buf);
#ifdef AF_INET6
    if (hent && hent->h_addrtype == AF_INET6)
      len += 12;		/* 16 bytes total for IPv6 address. */
#endif
#endif /* HAVE_GETHOSTNAME */
  }  /* endif -- text (1) */


  /*
   * Allocate memory and store enterprise ID.
   */
  if ((bufp = (u_char *) malloc(len)) == NULL) {
    snmp_log_perror("setup_engineID malloc");
    return -1;
  }

  memcpy(bufp, &enterpriseid, sizeof(enterpriseid)); /* XXX Must be 4 bytes! */
  bufp[0] |= 0x80;
  

  /*
   * Store the given text  -OR-   the first found IP address.
   */
  if (text) {
    bufp[4] = 4;
    memcpy((char *)bufp+5, text, strlen(text));

  } else {
    bufp[4] = 1;
#ifdef HAVE_GETHOSTNAME
    gethostname((char *)buf, sizeof(buf));
    hent = gethostbyname((char *)buf);

    if (hent && hent->h_addrtype == AF_INET) {
      memcpy(bufp+5, hent->h_addr_list[0], hent->h_length);

#ifdef AF_INET6
    } else if (hent && hent->h_addrtype == AF_INET6) {
      bufp[4] = 2;
      memcpy(bufp+5, hent->h_addr_list[0], hent->h_length);
#endif

    } else {		/* Unknown address type.  Default to 127.0.0.1. */

      bufp[5] = 127;
      bufp[6] = 0;
      bufp[7] = 0;
      bufp[8] = 1;
    }
#else /* HAVE_GETHOSTNAME */
    /* Unknown address type.  Default to 127.0.0.1. */
    
    bufp[5] = 127;
    bufp[6] = 0;
    bufp[7] = 0;
    bufp[8] = 1;
#endif /* HAVE_GETHOSTNAME */
    
  }  /* endif -- text (2) */


  /*
   * Pass the string back to the calling environment, or use it for
   * our local engineID.
   */
  if (localsetup) {
	SNMP_FREE(engineID);
	engineID	= bufp;
	engineIDLength	= len;

  } else {
	*eidp = bufp;
  }


  return len;

}  /* end setup_engineID() */

void
usm_parse_create_usmUser(const char *token, char *line) {
  char *cp;
  char buf[SNMP_MAXBUF_MEDIUM];
  struct usmUser *newuser;
  u_char	  userKey[SNMP_MAXBUF_SMALL];
  size_t	  userKeyLen = SNMP_MAXBUF_SMALL;
  int ret;

  newuser = usm_create_user();

  /* READ: Security Name */
  cp = copy_word(line, buf);
  newuser->secName = strdup(buf);
  newuser->name = strdup(buf);

  newuser->engineID = snmpv3_generate_engineID(&ret);
  if ( ret < 0 ) {
    usm_free_user(newuser);
    return;
  }
  newuser->engineIDLen = ret;

  if (!cp)
    goto add; /* no authentication or privacy type */

  /* READ: Authentication Type */
  if (strncmp(cp, "MD5", 3) == 0) {
    memcpy(newuser->authProtocol, usmHMACMD5AuthProtocol,
           sizeof(usmHMACMD5AuthProtocol));
  } else if (strncmp(cp, "SHA", 3) == 0) {
    memcpy(newuser->authProtocol, usmHMACSHA1AuthProtocol,
           sizeof(usmHMACSHA1AuthProtocol));
  } else {
    config_perror("Unknown authentication protocol");
    usm_free_user(newuser);
    return;
  }

  cp = skip_token(cp);

  /* READ: Authentication Pass Phrase */
  if (!cp) {
    config_perror("no authentication pass phrase");
    usm_free_user(newuser);
    return;
  }
  cp = copy_word(cp, buf);
  /* And turn it into a localized key */
  ret = generate_Ku(newuser->authProtocol, newuser->authProtocolLen,
		    (u_char *)buf, strlen(buf),
		    userKey, &userKeyLen );
  if (ret != SNMPERR_SUCCESS) {
    config_perror("Error generating auth key from pass phrase.");
    usm_free_user(newuser);
    return;
  }
  newuser->authKeyLen =
    sc_get_properlength(newuser->authProtocol, newuser->authProtocolLen);
  newuser->authKey = (u_char *) malloc(newuser->authKeyLen);
  ret = generate_kul(newuser->authProtocol, newuser->authProtocolLen,
		     newuser->engineID, newuser->engineIDLen,
		     userKey, userKeyLen,
		     newuser->authKey, &newuser->authKeyLen );
  if (ret != SNMPERR_SUCCESS) {
    config_perror("Error generating localized auth key (Kul) from Ku.");
    usm_free_user(newuser);
    return;
  }

  if (!cp)
    goto add; /* no privacy type (which is legal) */
  
  /* READ: Privacy Type */
  if (strncmp(cp, "DES", 3) == 0) {
    memcpy(newuser->privProtocol, usmDESPrivProtocol,
           sizeof(usmDESPrivProtocol));
  } else {
    config_perror("Unknown privacy protocol");
    usm_free_user(newuser);
    return;
  }

  cp = skip_token(cp);
  /* READ: Authentication Pass Phrase */
  if (!cp) {
    /* assume the same as the authentication key */
    memdup(&newuser->privKey, newuser->authKey, newuser->authKeyLen);
  } else {
    cp = copy_word(cp, buf);
    /* And turn it into a localized key */
    ret = generate_Ku(newuser->authProtocol, newuser->authProtocolLen,
                      (u_char *)buf, strlen(buf),
                      userKey, &userKeyLen );
    if (ret != SNMPERR_SUCCESS) {
      config_perror("Error generating priv key from pass phrase.");
      usm_free_user(newuser);
      return;
    }

    ret = sc_get_properlength(newuser->authProtocol, newuser->authProtocolLen);
    if (ret < 0) {
      config_perror("Error getting proper key length for priv algorithm.");
      usm_free_user(newuser);
      return;
    }
    newuser->privKeyLen = ret;
      
    newuser->privKey = (u_char *) malloc(newuser->privKeyLen);
    ret = generate_kul(newuser->authProtocol, newuser->authProtocolLen,
                       newuser->engineID, newuser->engineIDLen,
                       userKey, userKeyLen,
                       newuser->privKey, &newuser->privKeyLen );
    if (ret != SNMPERR_SUCCESS) {
      config_perror("Error generating localized priv key (Kul) from Ku.");
      usm_free_user(newuser);
      return;
    }
  }
add:
  usm_add_user(newuser);
  DEBUGMSGTL(("usmUser","created a new user %s\n", newuser->secName));
}

/*******************************************************************-o-******
 * engineBoots_conf
 *
 * Parameters:
 *	*word
 *	*cptr
 *
 * Line syntax:
 *	engineBoots <num_boots>
 */
void
engineBoots_conf(const char *word, char *cptr)
{
  engineBoots = atoi(cptr)+1;
  DEBUGMSGTL(("snmpv3","engineBoots: %d\n",engineBoots));
}



/*******************************************************************-o-******
 * engineID_conf
 *
 * Parameters:
 *	*word
 *	*cptr
 *
 * This function reads a string from the configuration file and uses that
 * string to initialize the engineID.  It's assumed to be human readable.
 */
void
engineID_conf(const char *word, char *cptr)
{
  setup_engineID(NULL, cptr);
  DEBUGMSGTL(("snmpv3","initialized engineID with: %s\n",cptr));
}

void
version_conf(const char *word, char *cptr)
{
  if (strcmp(cptr,"1") == 0) {
    ds_set_int(DS_LIBRARY_ID, DS_LIB_SNMPVERSION, SNMP_VERSION_1);
  } else if (strcasecmp(cptr,"2c") == 0) {
    ds_set_int(DS_LIBRARY_ID, DS_LIB_SNMPVERSION, SNMP_VERSION_2c);
  } else if (strcmp(cptr,"3") == 0) {
    ds_set_int(DS_LIBRARY_ID, DS_LIB_SNMPVERSION, SNMP_VERSION_3);
  } else {
    config_perror("Unknown version specification");
    return;
  }
  DEBUGMSGTL(("snmpv3","set default version to %d\n",
              ds_get_int(DS_LIBRARY_ID, DS_LIB_SNMPVERSION)));
}

/* engineID_old_conf(const char *, char *):

   Reads a octet string encoded engineID into the oldEngineID and
   oldEngineIDLen pointers.
*/
void
oldengineID_conf(const char *word, char *cptr)
{
  read_config_read_octet_string(cptr, &oldEngineID, &oldEngineIDLength);
}


/*******************************************************************-o-******
 * init_snmpv3
 *
 * Parameters:
 *	*type	Label for the config file "type" used by calling entity.
 *      
 * Set time and engineID.
 * Set parsing functions for config file tokens.
 * Initialize SNMP Crypto API (SCAPI).
 */
void
init_snmpv3(const char *type) {
  gettimeofday(&snmpv3starttime, NULL);

  if (type == NULL)
     type = "snmpapp";

  if (type && !strcmp(type,"snmpapp")) {
     setup_engineID(NULL,"__snmpapp__");
  } else {
     setup_engineID(NULL, NULL);
  }

  /* initialize submodules */
  init_usm();

  /* we need to be called back later */
  snmp_register_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_POST_READ_CONFIG,
                         init_snmpv3_post_config, NULL);
  /* we need to be called back later */
  snmp_register_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_STORE_DATA,
                         snmpv3_store, (void *) type);


#if		!defined(USE_INTERNAL_MD5)
  /* doesn't belong here at all */
  sc_init();
#endif		/* !USE_INTERNAL_MD5 */

  /* register all our configuration handlers (ack, there's a lot) */

  /* handle engineID setup before everything else which may depend on it */
  register_premib_handler(type,"engineID", engineID_conf, NULL, "string");
  register_premib_handler(type,"oldEngineID", oldengineID_conf, NULL, NULL);
  register_config_handler(type,"engineBoots", engineBoots_conf, NULL, NULL);

  /* default store config entries */
  ds_register_config(ASN_OCTET_STR, "snmp", "defSecurityName", DS_LIBRARY_ID,
                     DS_LIB_SECNAME);
  ds_register_config(ASN_OCTET_STR, "snmp", "defContext", DS_LIBRARY_ID,
                     DS_LIB_CONTEXT);
  ds_register_config(ASN_OCTET_STR, "snmp", "defPassphrase", DS_LIBRARY_ID,
                     DS_LIB_PASSPHRASE);
  ds_register_config(ASN_OCTET_STR, "snmp", "defAuthPassphrase", DS_LIBRARY_ID,
                     DS_LIB_AUTHPASSPHRASE);
  ds_register_config(ASN_OCTET_STR, "snmp", "defPrivPassphrase", DS_LIBRARY_ID,
                     DS_LIB_PRIVPASSPHRASE);
  register_config_handler("snmp","defVersion", version_conf, NULL, "1|2c|3");

  register_config_handler("snmp","defAuthType", snmpv3_authtype_conf, NULL,
                          "MD5|SHA");
  register_config_handler("snmp","defPrivType", snmpv3_privtype_conf, NULL,
                          "DES (currently the only possible value)");
  register_config_handler("snmp","defSecurityLevel", snmpv3_secLevel_conf,
                          NULL, "noAuthNoPriv|authNoPriv|authPriv");
  register_config_handler(type,"userSetAuthPass", usm_set_password, NULL,
                          "secname engineIDLen engineID pass");
  register_config_handler(type,"userSetPrivPass", usm_set_password, NULL,
                          "secname engineIDLen engineID pass");
  register_config_handler(type,"userSetAuthKey", usm_set_password, NULL,
                          "secname engineIDLen engineID KuLen Ku");
  register_config_handler(type,"userSetPrivKey", usm_set_password, NULL,
                          "secname engineIDLen engineID KuLen Ku");
  register_config_handler(type,"userSetAuthLocalKey", usm_set_password, NULL,
                          "secname engineIDLen engineID KulLen Kul");
  register_config_handler(type,"userSetPrivLocalKey", usm_set_password, NULL,
                          "secname engineIDLen engineID KulLen Kul");
}

/*
 * initializations for SNMPv3 to be called after the configuration files
 * have been read.
 */

int
init_snmpv3_post_config(int majorid, int minorid, void *serverarg,
                        void *clientarg) {

  int engineIDLen;
  u_char *c_engineID;

  c_engineID = snmpv3_generate_engineID(&engineIDLen);

  if ( engineIDLen < 0 ) {
    /* Somethine went wrong - help! */
    return SNMPERR_GENERR;
  }

  /* if our engineID has changed at all, the boots record must be set to 1 */
  if (engineIDLen != (int)oldEngineIDLength ||
      oldEngineID == NULL || c_engineID == NULL ||
      memcmp(oldEngineID, c_engineID, engineIDLen) != 0) {
    engineBoots = 1;
  }

  /* set our local engineTime in the LCD timing cache */
  set_enginetime(c_engineID, engineIDLen, 
                 snmpv3_local_snmpEngineBoots(), 
                 snmpv3_local_snmpEngineTime(),
                 TRUE);

  free(c_engineID);
  return SNMPERR_SUCCESS;
}

/*******************************************************************-o-******
 * store_snmpv3
 *
 * Parameters:
 *	*type
 */
int
snmpv3_store(int majorID, int minorID, void *serverarg, void *clientarg) {
  char line[SNMP_MAXBUF_SMALL];
  u_char c_engineID[SNMP_MAXBUF_SMALL];
  int  engineIDLen;
  const char *type = (const char *) clientarg;

  if (type == NULL)  /* should never happen, since the arg is ours */
    type = "unknown";

  sprintf(line, "engineBoots %ld", engineBoots);
  read_config_store(type, line);

  engineIDLen = snmpv3_get_engineID(c_engineID, SNMP_MAXBUF_SMALL);

  if (engineIDLen) {
    /* store the engineID used for this run */
    sprintf(line, "oldEngineID ");
    read_config_save_octet_string(line+strlen(line), c_engineID,
                                  engineIDLen);
    read_config_store(type, line);
  }
  return SNMPERR_SUCCESS;
}  /* snmpv3_store() */

u_long
snmpv3_local_snmpEngineBoots(void)
{
  return engineBoots;
}


/*******************************************************************-o-******
 * snmpv3_get_engineID
 *
 * Parameters:
 *	*buf
 *	 buflen
 *      
 * Returns:
 *	Length of engineID	On Success
 *	SNMPERR_GENERR		Otherwise.
 *
 *
 * Store engineID in buf; return the length.
 *
 */
int
snmpv3_get_engineID(u_char *buf, size_t buflen)
{
  /*
   * Sanity check.
   */
  if ( !buf || (buflen < engineIDLength) ) {
    return SNMPERR_GENERR;
  }

  memcpy(buf,engineID,engineIDLength);
  return engineIDLength;

}  /* end snmpv3_get_engineID() */

/*******************************************************************-o-******
 * snmpv3_clone_engineID
 *
 * Parameters:
 *	**dest
 *       *dest_len
 *       src
 *	 srclen
 *      
 * Returns:
 *	Length of engineID	On Success
 *	0		        Otherwise.
 *
 *
 * Clones engineID, creates memory
 *
 */
int
snmpv3_clone_engineID(u_char **dest, size_t* destlen, u_char*src, size_t srclen)
{
  if ( !dest || !destlen ) return 0;

  *dest = NULL; *destlen = 0;

  if (srclen && src) {
    *dest = (u_char*)malloc((unsigned)srclen * sizeof(u_char));
    if (*dest == NULL) return 0;
    memmove(*dest, src, srclen * sizeof(u_char));
    *destlen = srclen;
  }
  return *destlen;
}  /* end snmpv3_clone_engineID() */


/*******************************************************************-o-******
 * snmpv3_generate_engineID
 *
 * Parameters:
 *	*length
 *      
 * Returns:
 *	Pointer to copy of engineID	On Success.
 *	NULL				If malloc() or snmpv3_get_engineID()
 *						fail.
 *
 * Generates a malloced copy of our engineID.
 *
 * 'length' is set to the length of engineID  -OR-  < 0 on failure.
 */
u_char *
snmpv3_generate_engineID(int *length)
{
  u_char *newID;
  newID = (u_char *) malloc(engineIDLength);

  if (newID) {
    *length = snmpv3_get_engineID(newID, engineIDLength);
  }

  if (*length < 0) {
    SNMP_FREE(newID);
    newID = NULL;
  }

  return newID;

}  /* end snmpv3_generate_engineID() */

/* snmpv3_local_snmpEngineTime(): return the number of seconds since the
   snmpv3 engine last incremented engine_boots */
u_long
snmpv3_local_snmpEngineTime(void)
{
  struct timeval now;

  gettimeofday(&now, NULL);
  return calculate_time_diff(&now, &snmpv3starttime)/100;
}

#ifdef SNMP_TESTING_CODE
/* snmpv3_set_engineBootsAndTime(): this function does not exist.  Go away. */
/*   It certainly should never be used, unless in a testing scenero,
     which is why it was created */
void
snmpv3_set_engineBootsAndTime(int boots, int ttime) {
  engineBoots = boots;
  gettimeofday(&snmpv3starttime, NULL);
  snmpv3starttime.tv_sec -= ttime;
}
#endif

#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */
