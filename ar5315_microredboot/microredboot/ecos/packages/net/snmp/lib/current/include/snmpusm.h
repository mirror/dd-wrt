//==========================================================================
//
//      ./lib/current/include/snmpusm.h
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
 * snmpusm.h
 *
 * Header file for USM support.
 */

#ifndef SNMPUSM_H
#define SNMPUSM_H

#ifdef __cplusplus
extern "C" {
#endif

#define WILDCARDSTRING "*"

/*
 * General.
 */
#define USM_MAX_ID_LENGTH		1024		/* In bytes. */
#define USM_MAX_SALT_LENGTH		64		/* In BITS. */
#define USM_MAX_KEYEDHASH_LENGTH	128		/* In BITS. */

#define USM_TIME_WINDOW			150


/*
 * Structures.
 */
struct usmStateReference {
	char		*usr_name;
	size_t		 usr_name_length;
	u_char		*usr_engine_id;
	size_t		 usr_engine_id_length;
	oid		*usr_auth_protocol;
	size_t		 usr_auth_protocol_length;
	u_char		*usr_auth_key;
	size_t		 usr_auth_key_length;
	oid		*usr_priv_protocol;
	size_t		 usr_priv_protocol_length;
	u_char		*usr_priv_key;
	size_t		 usr_priv_key_length;
	u_int		 usr_sec_level;
};


/* struct usmUser: a structure to represent a given user in a list */
/* Note: Any changes made to this structure need to be reflected in
   the following functions: */

struct usmUser;
struct usmUser {
   u_char         *engineID;
   size_t          engineIDLen;
   char           *name;
   char           *secName;
   oid            *cloneFrom;
   size_t          cloneFromLen;
   oid            *authProtocol;
   size_t          authProtocolLen;
   u_char         *authKey;
   size_t          authKeyLen;
   oid            *privProtocol;
   size_t          privProtocolLen;
   u_char         *privKey;
   size_t          privKeyLen;
   u_char         *userPublicString;
   int             userStatus;
   int             userStorageType;
   struct usmUser *next;
   struct usmUser *prev;
};



/*
 * Prototypes.
 */
void	usm_set_reportErrorOnUnknownID (int value);

struct usmStateReference *
	usm_malloc_usmStateReference (void);

void	usm_free_usmStateReference (void *old);

int	usm_set_usmStateReference_name (
		struct usmStateReference	*ref,
		char				*name,
		size_t				 name_len);

int	usm_set_usmStateReference_engine_id (
		struct usmStateReference	*ref,
		u_char				*engine_id,
		size_t				 engine_id_len);

int	usm_set_usmStateReference_auth_protocol (
		struct usmStateReference *ref,
		oid *auth_protocol,
		size_t auth_protocol_len);

int	usm_set_usmStateReference_auth_key (
		struct usmStateReference *ref,
		u_char *auth_key,
		size_t auth_key_len);

int	usm_set_usmStateReference_priv_protocol (
		struct usmStateReference *ref,
		oid *priv_protocol,
		size_t priv_protocol_len);

int	usm_set_usmStateReference_priv_key (
		struct usmStateReference *ref,
		u_char *priv_key,
		size_t priv_key_len);

int	usm_set_usmStateReference_sec_level (
		struct usmStateReference *ref,
		int sec_level);

#ifdef SNMP_TESTING_CODE
void	emergency_print (u_char *field, u_int length);
#endif

int	asn_predict_int_length (int type, long number, size_t len);

int	asn_predict_length (int type, u_char *ptr, size_t u_char_len);

int	usm_set_salt (
		u_char		*iv,
		size_t		*iv_length,
		u_char		*priv_salt,
		size_t		 priv_salt_length,
		u_char		*msgSalt );

int	usm_parse_security_parameters (
		u_char  *secParams,
		size_t   remaining,
		u_char  *secEngineID,
		size_t  *secEngineIDLen,
		u_int   *boots_uint,
		u_int   *time_uint,
		char    *secName,
		size_t  *secNameLen,
		u_char  *signature,
		size_t  *signature_length,
		u_char  *salt,
		size_t  *salt_length,
		u_char **data_ptr);

int	usm_check_and_update_timeliness (
		u_char *secEngineID,
		size_t  secEngineIDLen,
		u_int   boots_uint,
		u_int   time_uint,
		int    *error);

int usm_generate_out_msg (int, u_char *, size_t, int, int, u_char *, size_t,
			      char *,  size_t, int, u_char *, size_t, void *,
			      u_char *, size_t *, u_char **, size_t *);

int usm_process_in_msg (int, size_t, u_char *, int, int, u_char *, size_t,
			    u_char *, size_t *, char *, size_t *, u_char **, size_t *,
			    size_t *, void **);

int             usm_check_secLevel(int level, struct usmUser *user);
void            usm_update_engine_time(void);
struct usmUser *usm_get_userList(void);
struct usmUser *usm_get_user(u_char *engineID, size_t engineIDLen, char *name);
struct usmUser *usm_get_user_from_list(u_char *engineID, size_t engineIDLen,
                                       char *name, struct usmUser *userList,
                                       int use_default);
struct usmUser *usm_add_user(struct usmUser *user);
struct usmUser *usm_add_user_to_list(struct usmUser *user,
                                     struct usmUser *userList);
struct usmUser *usm_free_user(struct usmUser *user);
struct usmUser *usm_create_user(void);
struct usmUser *usm_create_initial_user(const char *name,
                                     oid *authProtocol, size_t authProtocolLen,
                                     oid *privProtocol, size_t privProtocolLen);
struct usmUser *usm_cloneFrom_user(struct usmUser *from, struct usmUser *to);
struct usmUser *usm_remove_user(struct usmUser *user);
struct usmUser *usm_remove_user_from_list(struct usmUser *user,
                                          struct usmUser **userList);
char           *get_objid(char *line, oid **optr, size_t *len);
void            usm_save_users(const char *token, const char *type);
void            usm_save_users_from_list(struct usmUser *user, const char *token,
                                        const char *type);
void            usm_save_user(struct usmUser *user, const char *token, const char *type);
SNMPCallback    usm_store_users;
struct usmUser *usm_read_user(char *line);
void            usm_parse_config_usmUser(const char *token, char *line);

void            usm_set_password(const char *token, char *line);
void            usm_set_user_password(struct usmUser *user, const char *token,
                                      char *line);
void		init_usm(void);
int		init_usm_post_config(int majorid, int minorid, void *serverarg,
                                     void *clientarg);

#ifdef __cplusplus
}
#endif

#endif /* SNMPUSM_H */
