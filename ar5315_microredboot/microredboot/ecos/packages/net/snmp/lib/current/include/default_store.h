//==========================================================================
//
//      ./lib/current/include/default_store.h
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
/* default_store.h: storage space for defaults */
#ifndef DEFAULT_STORE_H
#define DEFAULT_STORE_H

#ifdef __cplusplus
extern "C" {
#endif

#define DS_MAX_IDS 3
#define DS_MAX_SUBIDS 32    /* needs to be a multiple of 8 */

/* begin storage definitions */
/* These definitions correspond with the "storid" argument to the API */
#define DS_LIBRARY_ID     0
#define DS_APPLICATION_ID 1
#define DS_TOKEN_ID       2

/* These definitions correspond with the "which" argument to the API,
   when the storeid argument is DS_LIBRARY_ID */
/* library booleans */
#define DS_LIB_MIB_ERRORS          0
#define DS_LIB_SAVE_MIB_DESCRS     1
#define DS_LIB_MIB_COMMENT_TERM    2
#define DS_LIB_MIB_PARSE_LABEL     3
#define DS_LIB_DUMP_PACKET         4
#define DS_LIB_LOG_TIMESTAMP       5
#define DS_LIB_DONT_READ_CONFIGS   6
#define DS_LIB_MIB_REPLACE         7  /* replace objects from latest module */
#define DS_LIB_PRINT_NUMERIC_ENUM  8  /* print only numeric enum values */
#define DS_LIB_PRINT_NUMERIC_OIDS  9  /* print only numeric enum values */
#define DS_LIB_DONT_BREAKDOWN_OIDS 10 /* dont print oid indexes specially */
#define DS_LIB_ALARM_DONT_USE_SIG  11 /* don't use the alarm() signal */
#define DS_LIB_PRINT_FULL_OID      12 /* print fully qualified oids */
#define DS_LIB_QUICK_PRINT         13 /* print very brief output for parsing */
#define DS_LIB_RANDOM_ACCESS	   14 /* random access to oid labels */
#define DS_LIB_REGEX_ACCESS	   15 /* regex matching to oid labels */
#define DS_LIB_DONT_CHECK_RANGE    16 /* don't check values for ranges on send*/
#define DS_LIB_NO_TOKEN_WARNINGS   17 /* no warn about unknown config tokens */

/* library integers */
#define DS_LIB_MIB_WARNINGS  0
#define DS_LIB_SECLEVEL      1
#define DS_LIB_SNMPVERSION   2
#define DS_LIB_DEFAULT_PORT  3
#define DS_LIB_PRINT_SUFFIX_ONLY 4 /* print out only a single oid node  == 1.
                                      like #1 but supply mib module too == 2. */

/* library strings */
#define DS_LIB_SECNAME         0
#define DS_LIB_CONTEXT         1
#define DS_LIB_PASSPHRASE      2
#define DS_LIB_AUTHPASSPHRASE  3
#define DS_LIB_PRIVPASSPHRASE  4
#define DS_LIB_OPTIONALCONFIG  5
#define DS_LIB_APPTYPE         6
#define DS_LIB_COMMUNITY       7

/* end storage definitions */

struct ds_read_config {
   u_char type;
   char  *token;
   int    storeid;
   int    which;
   struct ds_read_config *next;
};
   
int ds_set_boolean(int storeid, int which, int value);
int ds_get_boolean(int storeid, int which);
int ds_toggle_boolean(int storeid, int which);
int ds_set_int(int storeid, int which, int value);
int ds_get_int(int storeid, int which);
int ds_set_string(int storeid, int which, const char *value);
char *ds_get_string(int storeid, int which);
int ds_register_config(u_char type, const char *ftype, const char *token,
                       int storeid, int which);
int ds_register_premib(u_char type, const char *ftype, const char *token,
                       int storeid, int which);

#ifdef __cplusplus
}
#endif

#endif /* DEFAULT_STORE_H */
