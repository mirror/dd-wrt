//==========================================================================
//
//      ./lib/current/include/mt_support.h
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

/* mt_support.h - multi-thread resource locking support declarations */
/*
 * Author: Markku Laukkanen
 * Created: 6-Sep-1999
 * History:
 *  8-Sep-1999 M. Slifcak method names changed;
 *                        use array of resource locking structures.
 */

#ifndef MT_SUPPORT_H
#define MT_SUPPORT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Lock group identifiers */

#define MT_LIBRARY_ID      0
#define MT_APPLICATION_ID  1
#define MT_TOKEN_ID        2

#define MT_MAX_IDS         3  /* one greater than last from above */
#define MT_MAX_SUBIDS      10


/* Lock resource identifiers for library resources */

#define MT_LIB_NONE        0
#define MT_LIB_SESSION     1
#define MT_LIB_REQUESTID   2
#define MT_LIB_MESSAGEID   3
#define MT_LIB_SESSIONID   4
#define MT_LIB_TRANSID     5

#define MT_LIB_MAXIMUM     6  /* must be one greater than the last one */


#ifdef _REENTRANT

#if HAVE_PTHREAD_H

#include <pthread.h>
typedef pthread_mutex_t mutex_type;
#ifdef pthread_mutexattr_default
    #define MT_MUTEX_INIT_DEFAULT pthread_mutexattr_default
#else
    #define MT_MUTEX_INIT_DEFAULT 0
#endif

#elif defined(WIN32) || defined(cygwin)

#include <windows.h>
typedef CRITICAL_SECTION  mutex_type;

#else
  error "There is no re-entrant support as defined."
#endif

int snmp_res_init(void);
int snmp_res_lock(int groupID, int resourceID);
int snmp_res_unlock(int groupID, int resourceID);
int snmp_res_destroy_mutex(int groupID, int resourceID);

#else  /* !_REENTRANT */

#define snmp_res_init() do {} while (0)
#define snmp_res_lock(x,y) do {} while (0)
#define snmp_res_unlock(x,y) do {} while (0)
#define snmp_res_destroy_mutex(x,y) do {} while (0)

#endif /* !_REENTRANT */

#ifdef __cplusplus
}
#endif

#endif /* MT_SUPPORT_H */

