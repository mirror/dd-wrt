//==========================================================================
//
//      ./lib/current/src/mt_support.c
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

/* mt_support.c - multi-thread resource locking support */
/*
 * Author: Markku Laukkanen
 * Created: 6-Sep-1999
 * History:
 *  8-Sep-1999 M. Slifcak method names changed;
 *                        use array of resource locking structures.
 */

#include <config.h>
#include <errno.h>
#include "mt_support.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _REENTRANT

static
mutex_type s_res[MT_MAX_IDS][MT_LIB_MAXIMUM];  /* locking structures */

static mutex_type * _mt_res(int groupID, int resourceID)
{
    if (groupID < 1) return 0;
    if (groupID >= MT_MAX_IDS) return 0;
    if (resourceID < 1) return 0;
    if (resourceID >= MT_LIB_MAXIMUM) return 0;
    return (&s_res[groupID][resourceID]);
}

static
int snmp_res_init_mutex(mutex_type * mutex)
{    
    int rc = 0;
#if HAVE_PTHREAD_H
    rc = pthread_mutex_init(mutex, MT_MUTEX_INIT_DEFAULT);
#elif defined(WIN32)
    InitializeCriticalSection(mutex);
#endif

    return rc;
}

int snmp_res_init(void)
{    
    int ii, jj;
    int rc = 0;
    mutex_type *mutex;

  for (jj = 0; (0 == rc) && (jj < MT_MAX_IDS); jj++)
  for (ii = 0; (0 == rc) && (ii < MT_LIB_MAXIMUM); ii++)
  {
    mutex = _mt_res(jj, ii);
    if (!mutex) continue;
    rc = snmp_res_init_mutex( mutex );
  }

    return rc;
}

int snmp_res_destroy_mutex(int groupID, int resourceID)
{    
    int rc = 0;
    mutex_type *mutex = _mt_res(groupID, resourceID);
    if (!mutex) return EFAULT;

#if HAVE_PTHREAD_H
    rc = pthread_mutex_destroy(mutex);
#elif defined(WIN32)
    DeleteCriticalSection(mutex);
#endif

    return rc;
}
    
int snmp_res_lock(int groupID, int resourceID)
{
    int rc = 0;
    mutex_type *mutex = _mt_res(groupID, resourceID);
    if (!mutex) return EFAULT;

#if HAVE_PTHREAD_H
    rc = pthread_mutex_lock(mutex);
#elif defined(WIN32)
    EnterCriticalSection(mutex);
#endif

    return rc;
}

int snmp_res_unlock(int groupID, int resourceID)
{
    int rc = 0;
    mutex_type *mutex = _mt_res(groupID, resourceID);
    if (!mutex) return EFAULT;

#if HAVE_PTHREAD_H
    rc = pthread_mutex_unlock(mutex);
#elif defined(WIN32)
    LeaveCriticalSection(mutex);
#endif

    return rc;
}


#else  /* !_REENTRANT */

#ifdef WIN32

/* Provide "do nothing" targets for Release (.DLL) builds. */
#undef snmp_res_init
#undef snmp_res_lock
#undef snmp_res_unlock
#undef snmp_res_destroy_mutex

int snmp_res_init(void) { return 0; }
int snmp_res_lock(int groupID, int resourceID) { return 0; }
int snmp_res_unlock(int groupID, int resourceID) { return 0; }
int snmp_res_destroy_mutex(int groupID, int resourceID) { return 0; }
#endif /* !WIN32 */

#endif /* !_REENTRANT */


#ifdef __cplusplus
};
#endif

