//==========================================================================
//
//      ./agent/current/src/mibgroup/snmpv3/usmUser.c
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
// Author(s):    Andrew.Lunn@ascom.ch, Manu.Sharma@ascom.com
// Contributors: hmt
// Date:         2001-05-29
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
 * usmUser.c
 */

#include <config.h>

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
#include <stdlib.h>

#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#include "mibincl.h"
#include "snmpusm.h"
#include "snmpv3.h"
#include "snmp-tc.h"
#include "read_config.h"
#include "agent_read_config.h"
//#include "util_funcs.h"
#include "keytools.h"
#include "tools.h"
#include "scapi.h"

#include "usmUser.h"
#include "transform_oids.h"

struct variable4 usmUser_variables[] = {
  { USMUSERSPINLOCK     , ASN_INTEGER   , RWRITE, var_usmUser, 1, { 1 } },
  { USMUSERSECURITYNAME , ASN_OCTET_STR , RONLY , var_usmUser, 3, { 2,1,3 } },
  { USMUSERCLONEFROM    , ASN_OBJECT_ID , RWRITE, var_usmUser, 3, { 2,1,4 } },
  { USMUSERAUTHPROTOCOL , ASN_OBJECT_ID , RWRITE, var_usmUser, 3, { 2,1,5 } },
  { USMUSERAUTHKEYCHANGE, ASN_OCTET_STR , RWRITE, var_usmUser, 3, { 2,1,6 } },
  { USMUSEROWNAUTHKEYCHANGE, ASN_OCTET_STR , RWRITE, var_usmUser, 3, { 2,1,7 } },
  { USMUSERPRIVPROTOCOL , ASN_OBJECT_ID , RWRITE, var_usmUser, 3, { 2,1,8 } },
  { USMUSERPRIVKEYCHANGE, ASN_OCTET_STR , RWRITE, var_usmUser, 3, { 2,1,9 } },
  { USMUSEROWNPRIVKEYCHANGE, ASN_OCTET_STR , RWRITE, var_usmUser, 3, { 2,1,10 } },
  { USMUSERPUBLIC       , ASN_OCTET_STR , RWRITE, var_usmUser, 3, { 2,1,11 } },
  { USMUSERSTORAGETYPE  , ASN_INTEGER   , RWRITE, var_usmUser, 3, { 2,1,12 } },
  { USMUSERSTATUS       , ASN_INTEGER   , RWRITE, var_usmUser, 3, { 2,1,13 } },

};

oid usmUser_variables_oid[] = {1,3,6,1,6,3,15,1,2};


/* needed for the write_ functions to find the start of the index */
#define USM_MIB_LENGTH 12

static unsigned int usmUserSpinLock=0;

void
init_usmUser(void)
{
  snmpd_register_config_handler("usmUser",
                                usm_parse_config_usmUser, NULL, NULL);
  snmpd_register_config_handler("createUser",
                                usm_parse_create_usmUser, NULL,
                                "username (MD5|SHA) passphrase [DES] [passphrase]");

  /* we need to be called back later */
  snmp_register_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_STORE_DATA,
                         usm_store_users, NULL);
  
  REGISTER_MIB("snmpv3/usmUser", usmUser_variables, variable4,
				 usmUser_variables_oid);

}

/*******************************************************************-o-******
 * usm_generate_OID
 *
 * Parameters:
 *	*prefix		(I) OID prefix to the usmUser table entry.
 *	 prefixLen	(I)
 *	*uptr		(I) Pointer to a user in the user list.
 *	*length		(O) Length of generated index OID.
 *      
 * Returns:
 *	Pointer to the OID index for the user (uptr)  -OR-
 *	NULL on failure.
 *
 *
 * Generate the index OID for a given usmUser name.  'length' is set to
 * the length of the index OID.
 *
 * Index OID format is:
 *
 *    <...prefix>.<engineID_length>.<engineID>.<user_name_length>.<user_name>
 */
oid *
usm_generate_OID(oid *prefix, size_t prefixLen, struct usmUser *uptr,
                       size_t *length)
{
  oid *indexOid;
  int i;

  *length = 2 + uptr->engineIDLen + strlen(uptr->name) + prefixLen;
  indexOid = (oid *) malloc(*length * sizeof(oid));
  if (indexOid) {
    memmove(indexOid, prefix, prefixLen * sizeof (oid));

    indexOid[prefixLen] = uptr->engineIDLen;
    for(i = 0; i < (int)uptr->engineIDLen; i++)
      indexOid[prefixLen+1+i] = (oid) uptr->engineID[i];

    indexOid[prefixLen + uptr->engineIDLen + 1] = strlen(uptr->name);
    for(i = 0; i < (int)strlen(uptr->name); i++)
      indexOid[prefixLen + uptr->engineIDLen + 2 + i] = (oid) uptr->name[i];
  }
  return indexOid;

}  /* end usm_generate_OID() */

/* usm_parse_oid(): parses an index to the usmTable to break it down into
   a engineID component and a name component.  The results are stored in:

   **engineID:   a newly malloced string.
   *engineIDLen: The length of the malloced engineID string above.
   **name:       a newly malloced string.
   *nameLen:     The length of the malloced name string above.

   returns 1 if an error is encountered, or 0 if successful.
*/
int 
usm_parse_oid(oid *oidIndex, size_t oidLen,
              unsigned char **engineID, size_t *engineIDLen,
              unsigned char **name, size_t *nameLen)
{
  int nameL;
  int engineIDL;
  int i;

  /* first check the validity of the oid */
  if ((oidLen <= 0) || (!oidIndex)) {
    DEBUGMSGTL(("usmUser","parse_oid: null oid or zero length oid passed in\n"));
    return 1;
  }
  engineIDL = *oidIndex;		/* initial engineID length */
  if ((int)oidLen < engineIDL + 2) {
    DEBUGMSGTL(("usmUser","parse_oid: invalid oid length: less than the engineIDLen\n"));
    return 1;
  }
  nameL = oidIndex[engineIDL+1];	/* the initial name length */
  if ((int)oidLen != engineIDL + nameL + 2) {
    DEBUGMSGTL(("usmUser","parse_oid: invalid oid length: length is not exact\n"));
    return 1;
  }

  /* its valid, malloc the space and store the results */
  if (engineID == NULL || name == NULL) {
    DEBUGMSGTL(("usmUser","parse_oid: null storage pointer passed in.\n"));
    return 1;
  }

  *engineID = (unsigned char *) malloc(engineIDL);
  if (*engineID == NULL) {
    DEBUGMSGTL(("usmUser","parse_oid: malloc of the engineID failed\n"));
    return 1;
  }
  *engineIDLen = engineIDL;

  *name = (unsigned char *) malloc(nameL+1);
  if (*name == NULL) {
    DEBUGMSGTL(("usmUser","parse_oid: malloc of the name failed\n"));
    free(*engineID);
    return 1;
  }
  *nameLen = nameL;
  
  for(i = 0; i < engineIDL; i++) {
    if (oidIndex[i+1] > 255) {
      goto UPO_parse_error;
    }
    engineID[0][i] = (unsigned char) oidIndex[i+1];
  }

  for(i = 0; i < nameL; i++) {
    if (oidIndex[i+2+engineIDL] > 255) {
      UPO_parse_error:
      free(*engineID);
      free(*name);
      return 1;
    }
    name[0][i] = (unsigned char) oidIndex[i+2+engineIDL];
  }
  name[0][nameL] = 0;

  return 0;

}  /* end usm_parse_oid() */

/*******************************************************************-o-******
 * usm_parse_user
 *
 * Parameters:
 *	*name		Complete OID indexing a given usmUser entry.
 *	 name_length
 *      
 * Returns:
 *	Pointer to a usmUser  -OR-
 *	NULL if name does not convert to a usmUser.
 * 
 * Convert an (full) OID and return a pointer to a matching user in the
 * user list if one exists.
 */
struct usmUser *
usm_parse_user(oid *name, size_t name_len)
{
  struct usmUser *uptr;

  char *newName;
  u_char *engineID;
  size_t nameLen, engineIDLen;
  
  /* get the name and engineID out of the incoming oid */
  if (usm_parse_oid(&name[USM_MIB_LENGTH], name_len-USM_MIB_LENGTH,
                    &engineID, &engineIDLen, (u_char **)&newName, &nameLen))
    return NULL;

  /* Now see if a user exists with these index values */
  uptr = usm_get_user(engineID, engineIDLen, newName);
  free(engineID);
  free(newName);

  return uptr;

}  /* end usm_parse_user() */

/*******************************************************************-o-******
 * var_usmUser
 *
 * Parameters:
 *	  *vp	   (I)     Variable-binding associated with this action.
 *	  *name	   (I/O)   Input name requested, output name found.
 *	  *length  (I/O)   Length of input and output oid's.
 *	   exact   (I)     TRUE if an exact match was requested.
 *	  *var_len (O)     Length of variable or 0 if function returned.
 *	(**write_method)   Hook to name a write method (UNUSED).
 *      
 * Returns:
 *	Pointer to (char *) containing related data of length 'length'
 *	  (May be NULL.)
 *
 *
 * Call-back function passed to the agent in order to return information
 * for the USM MIB tree.
 *
 *
 * If this invocation is not for USMUSERSPINLOCK, lookup user name
 * in the usmUser list.
 *
 * If the name does not match any user and the request
 * is for an exact match, -or- if the usmUser list is empty, create a 
 * new list entry.
 *
 * Finally, service the given USMUSER* var-bind.  A NULL user generally
 * results in a NULL return value.
 */
u_char *
var_usmUser(
    struct variable *vp,
    oid     *name,
    size_t  *length,
    int     exact,
    size_t  *var_len,
    WriteMethod **write_method)
{
  struct usmUser *uptr=NULL, *nptr, *pptr;
  int i, rtest, result;
  oid *indexOid;
  size_t len;

  /* variables we may use later */
  static long long_ret;
  static u_char string[1];
  static oid objid[2];                      /* for .0.0 */

  *write_method = 0;           /* assume it isnt writable for the time being */
  *var_len = sizeof(long_ret); /* assume an integer and change later if not */

  if (vp->magic != USMUSERSPINLOCK) {
    oid newname[MAX_OID_LEN];
    len = (*length < vp->namelen) ? *length : vp->namelen;
    rtest = snmp_oid_compare(name, len, vp->name, len);
    if (rtest > 0 ||
/*      (rtest == 0 && !exact && (int) vp->namelen+1 < (int) *length) || */
        (exact == 1 && rtest != 0)) {
      if (var_len)
	*var_len = 0;
      return 0;
    }
    memset(newname, 0, sizeof(newname));
    if (((int) *length) <= (int) vp->namelen || rtest == -1) {
      /* oid is not within our range yet */
      /* need to fail if not exact */
      uptr = usm_get_userList();

    } else {
      for(nptr = usm_get_userList(), pptr = NULL, uptr = NULL; nptr != NULL;
          pptr = nptr, nptr = nptr->next) {
        indexOid = usm_generate_OID(vp->name, vp->namelen, nptr, &len);
        result = snmp_oid_compare(name, *length, indexOid, len);
        DEBUGMSGTL(("usmUser", "Checking user: %s - ", nptr->name));
        for(i = 0; i < (int)nptr->engineIDLen; i++) {
          DEBUGMSG(("usmUser", " %x",nptr->engineID[i]));
        }
        DEBUGMSG(("usmUser"," - %d \n  -> OID: ", result));
        DEBUGMSGOID(("usmUser", indexOid, len));
        DEBUGMSG(("usmUser","\n"));

        free(indexOid);

        if (exact) {
          if (result == 0) {
            uptr = nptr;
          }
        } else {
          if (result == 0) {
            /* found an exact match.  Need the next one for !exact */
            uptr = nptr->next;
          } else if (result == 1) {
            uptr = nptr;
          }
        }
      }
    }  /* endif -- name <= vp->name */

    /* if uptr is NULL and exact we need to continue for creates */
    if (uptr == NULL && !exact)
      return(NULL);

    if (uptr) {
      indexOid = usm_generate_OID(vp->name, vp->namelen, uptr, &len);
      *length = len;
      memmove(name, indexOid, len*sizeof(oid));
      DEBUGMSGTL(("usmUser", "Found user: %s - ", uptr->name));
      for(i = 0; i < (int)uptr->engineIDLen; i++) {
        DEBUGMSG(("usmUser", " %x",uptr->engineID[i]));
      }
      DEBUGMSG(("usmUser","\n  -> OID: "));
      DEBUGMSGOID(("usmUser", indexOid, len));
      DEBUGMSG(("usmUser","\n"));

      free(indexOid);
    }
  } else {
    if (header_generic(vp,name,length,exact,var_len,write_method))
      return 0;
  }  /* endif -- vp->magic != USMUSERSPINLOCK */

  switch(vp->magic) {
    case USMUSERSPINLOCK:
      *write_method = write_usmUserSpinLock;
      long_ret = usmUserSpinLock;
      return (unsigned char *) &long_ret;

    case USMUSERSECURITYNAME:
      if (uptr) {
        *var_len = strlen(uptr->secName);
        return (unsigned char *) uptr->secName;
      }
      return NULL;

    case USMUSERCLONEFROM:
      *write_method = write_usmUserCloneFrom;
      if (uptr) {
        objid[0] = 0; /* "When this object is read, the ZeroDotZero OID */
        objid[1] = 0; /*  is returned." */
        *var_len = sizeof(oid)*2;
        return (unsigned char *) objid;
      }
      return NULL;

    case USMUSERAUTHPROTOCOL:
      *write_method = write_usmUserAuthProtocol;
      if (uptr) {
        *var_len = uptr->authProtocolLen*sizeof(oid);
        return (u_char *)uptr->authProtocol;
      }
      return NULL;

    case USMUSERAUTHKEYCHANGE:
    case USMUSEROWNAUTHKEYCHANGE:

      /* we treat these the same, and let the calling module
         distinguish between them */
      *write_method = write_usmUserAuthKeyChange;
      if (uptr) {
        *string = 0; /* always return a NULL string */
        *var_len = 0;
        return string;
      }
      return NULL;

    case USMUSERPRIVPROTOCOL:
      *write_method = write_usmUserPrivProtocol;
      if (uptr) {
        *var_len = uptr->privProtocolLen*sizeof(oid);
        return (u_char *)uptr->privProtocol;
      }
      return NULL;

    case USMUSERPRIVKEYCHANGE:
    case USMUSEROWNPRIVKEYCHANGE:
      /* we treat these the same, and let the calling module
         distinguish between them */
      *write_method = write_usmUserPrivKeyChange;
      if (uptr) {
        *string = 0; /* always return a NULL string */
        *var_len = 0;
        return string;
      }
      return NULL;

    case USMUSERPUBLIC:
      *write_method = write_usmUserPublic;
      if (uptr) {
        if (uptr->userPublicString) {
          *var_len = strlen((char *)uptr->userPublicString);
          return uptr->userPublicString;
        }
        *string = 0;
        *var_len = 0; /* return an empty string if the public
                                      string hasn't been defined yet */
        return string;
      }
      return NULL;

    case USMUSERSTORAGETYPE:
      *write_method = write_usmUserStorageType;
      if (uptr) {
        long_ret = uptr->userStorageType;
        return (unsigned char *) &long_ret;
      }
      return NULL;

    case USMUSERSTATUS:
      *write_method = write_usmUserStatus;
      if (uptr) {
        long_ret = uptr->userStatus;
        return (unsigned char *) &long_ret;
      }
      return NULL;

    default:
      DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_usmUser\n", vp->magic));
  }
  return 0;

}  /* end var_usmUser() */

/* write_usmUserSpinLock(): called when a set is performed on the
   usmUserSpinLock object */
int
write_usmUserSpinLock(
   int      action,
   u_char   *var_val,
   u_char   var_val_type,
   size_t   var_val_len,
   u_char   *statP,
   oid      *name,
   size_t   name_len)
{
  /* variables we may use later */
  static long long_ret;

  if (var_val_type != ASN_INTEGER){
      DEBUGMSGTL(("usmUser","write to usmUserSpinLock not ASN_INTEGER\n"));
      return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > sizeof(long_ret)){
      DEBUGMSGTL(("usmUser","write to usmUserSpinLock: bad length\n"));
      return SNMP_ERR_WRONGLENGTH;
  }
  long_ret = *((long *) var_val);

  if (long_ret != (long)usmUserSpinLock)
    return SNMP_ERR_INCONSISTENTVALUE;

  if (action == COMMIT) {
    if (usmUserSpinLock == 2147483647) {
      usmUserSpinLock = 0;
    } else {
      usmUserSpinLock++;
    }
  }
  return SNMP_ERR_NOERROR;
}  /* end write_usmUserSpinLock() */

/*******************************************************************-o-******
 * write_usmUserCloneFrom
 *
 * Parameters:
 *	 action
 *	*var_val
 *	 var_val_type
 *	 var_val_len
 *	*statP		(UNUSED)
 *	*name		OID of user to clone from.
 *	 name_len
 *      
 * Returns:
 *	SNMP_ERR_NOERROR		On success  -OR-  If user exists
 *					  and has already been cloned.
 *	SNMP_ERR_GENERR			Local function call failures.
 *	SNMP_ERR_INCONSISTENTNAME	'name' does not exist in user list
 *					  -OR-  user to clone from != RS_ACTIVE.
 *	SNMP_ERR_WRONGLENGTH		OID length > than local buffer size.
 *	SNMP_ERR_WRONGTYPE		ASN_OBJECT_ID is wrong.
 *
 *
 * XXX:  should handle action=UNDO's.
 */
int
write_usmUserCloneFrom(
   int      action,
   u_char   *var_val,
   u_char   var_val_type,
   size_t   var_val_len,
   u_char   *statP,
   oid      *name,
   size_t   name_len)
{
  /* variables we may use later */
  static oid objid[USM_LENGTH_OID_MAX], *oidptr;
  struct usmUser *uptr, *cloneFrom;
  size_t size;
  
  if (var_val_type != ASN_OBJECT_ID){
      DEBUGMSGTL(("usmUser","write to usmUserCloneFrom not ASN_OBJECT_ID\n"));
      return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > sizeof(objid)){
      DEBUGMSGTL(("usmUser","write to usmUserCloneFrom: bad length\n"));
      return SNMP_ERR_WRONGLENGTH;
  }
  if (action == COMMIT){
    /* parse the clonefrom objid */
    size = var_val_len/sizeof(oid);
    memcpy(objid, var_val, var_val_len);

    if ((uptr = usm_parse_user(name, name_len)) == NULL) 
      /* We don't allow creations here */
      return SNMP_ERR_INCONSISTENTNAME;

    /* have the user already been cloned?  If so, second cloning is
       not allowed, but does not generate an error */
    if (uptr->cloneFrom)
      return SNMP_ERR_NOERROR;

    /* does the cloneFrom user exist? */
    if ((cloneFrom = usm_parse_user(objid, size)) == NULL)
      /* We don't allow creations here */
      return SNMP_ERR_INCONSISTENTNAME;

    /* is it active */
    if (cloneFrom->userStatus != RS_ACTIVE)
      return SNMP_ERR_INCONSISTENTNAME;

    /* set the cloneFrom OID */
    if ((oidptr = snmp_duplicate_objid(objid, size/sizeof(oid))) == NULL)
      return SNMP_ERR_GENERR;

    /* do the actual cloning */

    if (uptr->cloneFrom)
      free(uptr->cloneFrom);
    uptr->cloneFrom = oidptr;

    usm_cloneFrom_user(cloneFrom, uptr);
    
  }  /* endif: action == COMMIT */

  return SNMP_ERR_NOERROR;


}  /* end write_usmUserCloneFrom() */

/*******************************************************************-o-******
 * write_usmUserAuthProtocol
 *
 * Parameters:
 *	 action
 *	*var_val	OID of auth transform to set.
 *	 var_val_type
 *	 var_val_len
 *	*statP
 *	*name		OID of user upon which to perform set operation.
 *	 name_len
 *      
 * Returns:
 *	SNMP_ERR_NOERROR		On success.
 *	SNMP_ERR_GENERR
 *	SNMP_ERR_INCONSISTENTVALUE
 *	SNMP_ERR_NOSUCHNAME
 *	SNMP_ERR_WRONGLENGTH
 *	SNMP_ERR_WRONGTYPE
 */
int
write_usmUserAuthProtocol(
   int      action,
   u_char   *var_val,
   u_char   var_val_type,
   size_t   var_val_len,
   u_char   *statP,
   oid      *name,
   size_t   name_len)
{
  /* variables we may use later */
  static oid objid[USM_LENGTH_OID_MAX];
  static oid *optr;
  struct usmUser *uptr;
  size_t size;

  if (var_val_type != ASN_OBJECT_ID){
      DEBUGMSGTL(("usmUser","write to usmUserAuthProtocol not ASN_OBJECT_ID\n"));
      return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > sizeof(objid)){
      DEBUGMSGTL(("usmUser","write to usmUserAuthProtocol: bad length\n"));
      return SNMP_ERR_WRONGLENGTH;
  }
  if (action == COMMIT){
      size = var_val_len/sizeof(oid);
      memcpy(objid, var_val, var_val_len);

      /* don't allow creations here */
      if ((uptr = usm_parse_user(name, name_len)) == NULL)
        return SNMP_ERR_NOSUCHNAME;

      /* check the objid for validity */
      /* only allow sets to perform a change to usmNoAuthProtocol */
      if (snmp_oid_compare(objid, size, usmNoAuthProtocol,
                  sizeof(usmNoAuthProtocol)/sizeof(oid)) != 0)
        return SNMP_ERR_INCONSISTENTVALUE;
      
      /* if the priv protocol is not usmNoPrivProtocol, we can't change */
      if (snmp_oid_compare(uptr->privProtocol, uptr->privProtocolLen, usmNoPrivProtocol,
                  sizeof(usmNoPrivProtocol)/sizeof(oid)) != 0)
        return SNMP_ERR_INCONSISTENTVALUE;

      /* finally, we can do it */
      optr = uptr->authProtocol;
      if ((uptr->authProtocol = snmp_duplicate_objid(objid, size))
          == NULL) {
        uptr->authProtocol = optr;
        return SNMP_ERR_GENERR;
      }
      free(optr);
      uptr->authProtocolLen = size;
  }
  return SNMP_ERR_NOERROR;
}  /* end write_usmUserAuthProtocol() */

/*******************************************************************-o-******
 * write_usmUserAuthKeyChange
 *
 * Parameters:
 *	 action		
 *	*var_val	Octet string representing new KeyChange value.
 *	 var_val_type
 *	 var_val_len
 *	*statP		(UNUSED)
 *	*name		OID of user upon which to perform set operation.
 *	 name_len
 *      
 * Returns:
 *	SNMP_ERR_NOERR		Success.
 *	SNMP_ERR_WRONGTYPE	
 *	SNMP_ERR_WRONGLENGTH	
 *	SNMP_ERR_NOSUCHNAME	
 *	SNMP_ERR_GENERR
 *
 * Note: This function handles both the usmUserAuthKeyChange and
 *       usmUserOwnAuthKeyChange objects.  We are not passed the name
 *       of the user requseting the keychange, so we leave this to the
 *       calling module to verify when and if we should be called.  To
 *       change this would require a change in the mib module API to
 *       pass in the securityName requesting the change.
 *
 * XXX:  should handle action=UNDO's.
 */
int
write_usmUserAuthKeyChange(
   int      action,
   u_char   *var_val,
   u_char   var_val_type,
   size_t   var_val_len,
   u_char   *statP,
   oid      *name,
   size_t   name_len)
{
    static unsigned char   string[SNMP_MAXBUF_SMALL];
  struct usmUser        *uptr;
  unsigned char          buf[SNMP_MAXBUF_SMALL];
  size_t                 buflen = SNMP_MAXBUF_SMALL;

  char                  fnAuthKey[]    = "write_usmUserAuthKeyChange",
                        fnOwnAuthKey[] = "write_usmUserOwnAuthKeyChange",
                        *fname;
  
  if (name[USM_MIB_LENGTH-1] == 6)
    fname = fnAuthKey;
  else
    fname = fnOwnAuthKey;
  
  if (var_val_type != ASN_OCTET_STR) {
    DEBUGMSGTL(("usmUser","write to %s not ASN_OCTET_STR\n", fname));
    return SNMP_ERR_WRONGTYPE;
  }

  if (var_val_len > sizeof(string)) {
    DEBUGMSGTL(("usmUser","write to %s: bad length\n", fname));
    return SNMP_ERR_WRONGLENGTH;
  }

  if (action == COMMIT) {
    /* don't allow creations here */
    if ((uptr = usm_parse_user(name, name_len)) == NULL) {
      return SNMP_ERR_NOSUCHNAME;
    }

    /* Change the key. */
    DEBUGMSGTL(("usmUser","%s: changing auth key for user %s\n", fname, uptr->secName));

    if (decode_keychange(uptr->authProtocol, uptr->authProtocolLen,
                         uptr->authKey, uptr->authKeyLen,
                         var_val, var_val_len,
                         buf, &buflen) != SNMPERR_SUCCESS) {
      DEBUGMSGTL(("usmUser","%s: ... failed\n", fname));
        return SNMP_ERR_GENERR;
    }
    DEBUGMSGTL(("usmUser","%s: ... succeeded\n", fname));
    SNMP_FREE(uptr->authKey);
    memdup(&uptr->authKey, buf, buflen);
    uptr->authKeyLen = buflen;
  }

  return SNMP_ERR_NOERROR;
} /* end write_usmUserAuthKeyChange() */

int
write_usmUserPrivProtocol(
   int      action,
   u_char   *var_val,
   u_char   var_val_type,
   size_t   var_val_len,
   u_char   *statP,
   oid      *name,
   size_t   name_len)
{
  /* variables we may use later */
  static oid objid[USM_LENGTH_OID_MAX];
  static oid *optr;
  struct usmUser *uptr;
  size_t size;

  if (var_val_type != ASN_OBJECT_ID){
      DEBUGMSGTL(("usmUser","write to usmUserPrivProtocol not ASN_OBJECT_ID\n"));
      return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > sizeof(objid)){
      DEBUGMSGTL(("usmUser","write to usmUserPrivProtocol: bad length\n"));
      return SNMP_ERR_WRONGLENGTH;
  }
  if (action == COMMIT){
      size = var_val_len/sizeof(oid);
      memcpy(objid, var_val, var_val_len);

      /* don't allow creations here */
      if ((uptr = usm_parse_user(name, name_len)) == NULL)
        return SNMP_ERR_NOSUCHNAME;

      /* check the objid for validity */
      /* only allow sets to perform a change to usmNoPrivProtocol */
      if (snmp_oid_compare(objid, size, usmNoPrivProtocol,
                  sizeof(usmNoPrivProtocol)/sizeof(oid)) != 0)
        return SNMP_ERR_INCONSISTENTVALUE;
      
      /* finally, we can do it */
      optr = uptr->privProtocol;
      if ((uptr->privProtocol = snmp_duplicate_objid(objid, size))
          == NULL) {
        uptr->privProtocol = optr;
        return SNMP_ERR_GENERR;
      }
      free(optr);
      uptr->privProtocolLen = size;
  }
  return SNMP_ERR_NOERROR;
}  /* end write_usmUserPrivProtocol() */

/*
 * Note: This function handles both the usmUserPrivKeyChange and
 *       usmUserOwnPrivKeyChange objects.  We are not passed the name
 *       of the user requseting the keychange, so we leave this to the
 *       calling module to verify when and if we should be called.  To
 *       change this would require a change in the mib module API to
 *       pass in the securityName requesting the change.
 *
 */
int
write_usmUserPrivKeyChange(
   int      action,
   u_char   *var_val,
   u_char   var_val_type,
   size_t   var_val_len,
   u_char   *statP,
   oid      *name,
   size_t   name_len)
{
  static unsigned char   string[SNMP_MAXBUF_SMALL];
  struct usmUser        *uptr;
  unsigned char          buf[SNMP_MAXBUF_SMALL];
  size_t                 buflen = SNMP_MAXBUF_SMALL;

  char                  fnPrivKey[]    = "write_usmUserPrivKeyChange",
                        fnOwnPrivKey[] = "write_usmUserOwnPrivKeyChange",
                        *fname;
  
  if (name[USM_MIB_LENGTH-1] == 9)
    fname = fnPrivKey;
  else
    fname = fnOwnPrivKey;
  
  if (var_val_type != ASN_OCTET_STR) {
    DEBUGMSGTL(("usmUser","write to %s not ASN_OCTET_STR\n", fname));
    return SNMP_ERR_WRONGTYPE;
  }

  if (var_val_len > sizeof(string)) {
    DEBUGMSGTL(("usmUser","write to %s: bad length\n", fname));
    return SNMP_ERR_WRONGLENGTH;
  }

  if (action == COMMIT) {
    /* don't allow creations here */
    if ((uptr = usm_parse_user(name, name_len)) == NULL) {
      return SNMP_ERR_NOSUCHNAME;
    }

    /* Change the key. */
    DEBUGMSGTL(("usmUser","%s: changing priv key for user %s\n", fname, uptr->secName));

    if (decode_keychange(uptr->authProtocol, uptr->authProtocolLen,
                         uptr->privKey, uptr->privKeyLen,
                         var_val, var_val_len,
                         buf, &buflen) != SNMPERR_SUCCESS) {
      DEBUGMSGTL(("usmUser","%s: ... failed\n", fname));
        return SNMP_ERR_GENERR;
    }
    DEBUGMSGTL(("usmUser","%s: ... succeeded\n", fname));
    SNMP_FREE(uptr->privKey);
    memdup(&uptr->privKey, buf, buflen);
    uptr->privKeyLen = buflen;
  }

  return SNMP_ERR_NOERROR;
}  /* end write_usmUserPrivKeyChange() */

int
write_usmUserPublic(
   int      action,
   u_char   *var_val,
   u_char   var_val_type,
   size_t   var_val_len,
   u_char   *statP,
   oid      *name,
   size_t   name_len)
{
  /* variables we may use later */
  static unsigned char string[SNMP_MAXBUF];

  struct usmUser *uptr;

  if (var_val_type != ASN_OCTET_STR){
      DEBUGMSGTL(("usmUser","write to usmUserPublic not ASN_OCTET_STR\n"));
      return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > sizeof(string)){
      DEBUGMSGTL(("usmUser","write to usmUserPublic: bad length\n"));
      return SNMP_ERR_WRONGLENGTH;
  }
  if (action == COMMIT) {
    /* don't allow creations here */
    if ((uptr = usm_parse_user(name, name_len)) == NULL) {
      return SNMP_ERR_NOSUCHNAME;
    }
    if (uptr->userPublicString)
      free(uptr->userPublicString);
    uptr->userPublicString = (u_char *) malloc(var_val_len+1);
    if (uptr->userPublicString == NULL) {
      return SNMP_ERR_GENERR;
    }
    memcpy(uptr->userPublicString, var_val, var_val_len);
    uptr->userPublicString[var_val_len] = 0;
    DEBUGMSG(("usmUser", "setting public string: %d - %s\n", var_val_len,
              uptr->userPublicString));
  }
  return SNMP_ERR_NOERROR;
}  /* end write_usmUserPublic() */

int
write_usmUserStorageType(
   int      action,
   u_char   *var_val,
   u_char   var_val_type,
   size_t   var_val_len,
   u_char   *statP,
   oid      *name,
   size_t   name_len)
{
  /* variables we may use later */
  static long long_ret;
  struct usmUser *uptr;
  
  if (var_val_type != ASN_INTEGER){
      DEBUGMSGTL(("usmUser","write to usmUserStorageType not ASN_INTEGER\n"));
      return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > sizeof(long_ret)){
      DEBUGMSGTL(("usmUser","write to usmUserStorageType: bad length\n"));
      return SNMP_ERR_WRONGLENGTH;
  }
  if (action == COMMIT){
      /* don't allow creations here */
      if ((uptr = usm_parse_user(name, name_len)) == NULL) {
        return SNMP_ERR_NOSUCHNAME;
      }
      long_ret = *((long *) var_val);
      if ((long_ret == ST_VOLATILE || long_ret == ST_NONVOLATILE) &&
          (uptr->userStorageType == ST_VOLATILE ||
           uptr->userStorageType == ST_NONVOLATILE))
        uptr->userStorageType = long_ret;
      else
        return SNMP_ERR_INCONSISTENTVALUE;
  }
  return SNMP_ERR_NOERROR;
}  /* end write_usmUserStorageType() */

/*******************************************************************-o-******
 * write_usmUserStatus
 *
 * Parameters:
 *	 action
 *	*var_val
 *	 var_val_type
 *	 var_val_len
 *	*statP
 *	*name
 *	 name_len
 *      
 * Returns:
 *	SNMP_ERR_NOERROR		On success.
 *	SNMP_ERR_GENERR	
 *	SNMP_ERR_INCONSISTENTNAME
 *	SNMP_ERR_INCONSISTENTVALUE
 *	SNMP_ERR_WRONGLENGTH
 *	SNMP_ERR_WRONGTYPE
 */
int
write_usmUserStatus(
   int      action,
   u_char   *var_val,
   u_char   var_val_type,
   size_t   var_val_len,
   u_char   *statP,
   oid      *name,
   size_t   name_len)
{
  /* variables we may use later */
  static long long_ret;
  unsigned char *engineID;
  size_t engineIDLen;
  char *newName;
  size_t nameLen;
  struct usmUser *uptr;
  
  if (var_val_type != ASN_INTEGER){
      DEBUGMSGTL(("usmUser","write to usmUserStatus not ASN_INTEGER\n"));
      return SNMP_ERR_WRONGTYPE;
  }
  if (var_val_len > sizeof(long_ret)){
      DEBUGMSGTL(("usmUser","write to usmUserStatus: bad length\n"));
      return SNMP_ERR_WRONGLENGTH;
  }
  if (action == COMMIT){
    long_ret = *((long *) var_val);

    /* ditch illegal values now */
    /* notReady can not be used, but the return error code is not mentioned */
    if (long_ret == RS_NOTREADY || long_ret < 1 || long_ret > 6)
      return SNMP_ERR_INCONSISTENTVALUE;
    
    /* see if we can parse the oid for engineID/name first */
    if (usm_parse_oid(&name[USM_MIB_LENGTH], name_len-USM_MIB_LENGTH,
                      &engineID, &engineIDLen, (u_char **)&newName, &nameLen))
      return SNMP_ERR_INCONSISTENTNAME;

    /* Now see if a user already exists with these index values */
    uptr = usm_get_user(engineID, engineIDLen, newName);


    if (uptr) {			/* If so, we set the appropriate value... */
      free(engineID);
      free(newName);
      if (long_ret == RS_CREATEANDGO || long_ret == RS_CREATEANDWAIT) {
        return SNMP_ERR_INCONSISTENTVALUE;
      }
      if (long_ret == RS_DESTROY) {
        usm_remove_user(uptr);
        usm_free_user(uptr);
      } else {
        uptr->userStatus = long_ret;
      }

    } else {			/* ...else we create a new user */
      /* check for a valid status column set */
      if (long_ret == RS_ACTIVE || long_ret == RS_NOTINSERVICE) {
        free(engineID);
        free(newName);
        return SNMP_ERR_INCONSISTENTVALUE;
      }
      if (long_ret == RS_DESTROY) {
        /* destroying a non-existent row is actually legal */
        free(engineID);
        free(newName);
        return SNMP_ERR_NOERROR;
      }

      /* generate a new user */
      if ((uptr = usm_create_user()) == NULL) {
        free(engineID);
        free(newName);
        return SNMP_ERR_GENERR;
      }

      /* copy in the engineID */
      uptr->engineID =
        (unsigned char *) malloc(engineIDLen);
      if (uptr->engineID == NULL) {
        free(engineID);
        free(newName);
        usm_free_user(uptr);
        return SNMP_ERR_GENERR;
      }
      uptr->engineIDLen = engineIDLen;
      memcpy(uptr->engineID, engineID, engineIDLen);
      free(engineID);

      /* copy in the name and secname */
      if ((uptr->name = strdup(newName)) == NULL) {
        free(newName);
        usm_free_user(uptr);
        return SNMP_ERR_GENERR;
      }
      free(newName);
      if ((uptr->secName = strdup(uptr->name)) == NULL) {
        usm_free_user(uptr);
        return SNMP_ERR_GENERR;
      }

      /* set the status of the row based on the request */
      if (long_ret == RS_CREATEANDGO)
        uptr->userStatus = RS_ACTIVE;
      else if (long_ret == RS_CREATEANDWAIT)
        uptr->userStatus = RS_NOTINSERVICE;

      /* finally, add it to our list of users */
      usm_add_user(uptr);

    }  /* endif -- uptr */
  }  /* endif -- action==COMMIT */

  return SNMP_ERR_NOERROR;

}  /* end write_usmUserStatus() */

#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */
