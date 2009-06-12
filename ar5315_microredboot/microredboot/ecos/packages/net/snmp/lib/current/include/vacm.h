//==========================================================================
//
//      ./lib/current/include/vacm.h
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
 * vacm.h
 *
 * SNMPv3 View-based Access Control Model
 */

#ifndef VACM_H
#define VACM_H

#ifdef __cplusplus
extern "C" {
#endif

#define SECURITYMODEL	1
#define SECURITYNAME	2
#define SECURITYGROUP	3
#define SECURITYSTORAGE	4
#define SECURITYSTATUS	5

#define ACCESSPREFIX	1
#define ACCESSMODEL	2
#define ACCESSLEVEL	3
#define ACCESSMATCH	4
#define ACCESSREAD	5
#define ACCESSWRITE	6
#define ACCESSNOTIFY	7
#define ACCESSSTORAGE	8
#define ACCESSSTATUS	9

#define VIEWNAME	1
#define VIEWSUBTREE	2
#define VIEWMASK	3
#define VIEWTYPE	4
#define VIEWSTORAGE	5
#define VIEWSTATUS	6

#define VACM_MAX_STRING 32
#define VACMSTRINGLEN   34  /* VACM_MAX_STRING + 2 */

struct vacm_securityEntry {
    char	securityName[VACMSTRINGLEN];
    snmp_ipaddr	sourceIp;
    snmp_ipaddr	sourceMask;
    char	community[VACMSTRINGLEN];
    struct vacm_securityEntry *next;
};

struct vacm_groupEntry {
    int		securityModel;
    char	securityName[VACMSTRINGLEN];
    char	groupName[VACMSTRINGLEN];
    int		storageType;
    int		status;

    u_long	bitMask;
    struct vacm_groupEntry *reserved;
    struct vacm_groupEntry *next;
};

struct vacm_accessEntry {
    char	groupName[VACMSTRINGLEN];
    char	contextPrefix[VACMSTRINGLEN];
    int		securityModel;
    int		securityLevel;
    int 	contextMatch;
    char	readView[VACMSTRINGLEN];
    char	writeView[VACMSTRINGLEN];
    char	notifyView[VACMSTRINGLEN];
    int		storageType;
    int		status;

    u_long	bitMask;
    struct vacm_accessEntry *reserved;
    struct vacm_accessEntry *next;
};

struct vacm_viewEntry {
    char	viewName[VACMSTRINGLEN];
    oid		viewSubtree[MAX_OID_LEN];
    size_t	viewSubtreeLen;
    u_char	viewMask[VACMSTRINGLEN];
    size_t	viewMaskLen;
    int		viewType;
    int		viewStorageType;
    int		viewStatus;

    u_long	bitMask;

    struct vacm_viewEntry *reserved;
    struct vacm_viewEntry *next;
};

void vacm_destroyViewEntry (const char *, oid *, size_t);
void vacm_destroyAllViewEntries (void);

struct vacm_viewEntry *
vacm_getViewEntry (const char *, oid *, size_t);
/*
 * Returns a pointer to the viewEntry with the
 * same viewName and viewSubtree
 * Returns NULL if that entry does not exist.
 */

void
vacm_scanViewInit (void);
/*
 * Initialized the scan routines so that they will begin at the
 * beginning of the list of viewEntries.
 *
 */


struct vacm_viewEntry *
vacm_scanViewNext (void);
/*
 * Returns a pointer to the next viewEntry.
 * These entries are returned in no particular order,
 * but if N entries exist, N calls to view_scanNext() will
 * return all N entries once.
 * Returns NULL if all entries have been returned.
 * view_scanInit() starts the scan over.
 */

struct vacm_viewEntry *
vacm_createViewEntry (const char *, oid *, size_t);
/*
 * Creates a viewEntry with the given index
 * and returns a pointer to it.
 * The status of this entry is created as invalid.
 */

void vacm_destroyGroupEntry (int, const char *);
void vacm_destroyAllGroupEntries (void);
struct vacm_groupEntry *vacm_createGroupEntry (int, const char *);
struct vacm_groupEntry *vacm_getGroupEntry (int, const char *);
void vacm_scanGroupInit (void);
struct vacm_groupEntry *vacm_scanGroupNext (void);

void vacm_destroyAccessEntry (const char *, const char *, int, int);
void vacm_destroyAllAccessEntries (void);
struct vacm_accessEntry *vacm_createAccessEntry (const char *, const char *, int, int);
struct vacm_accessEntry *vacm_getAccessEntry (const char *, const char *, int, int);
void vacm_scanAccessInit (void);
struct vacm_accessEntry *vacm_scanAccessNext (void);

void vacm_destroySecurityEntry (const char *);
struct vacm_securityEntry *vacm_createSecurityEntry (const char *);
struct vacm_securityEntry *vacm_getSecurityEntry (const char *);
void vacm_scanSecurityInit (void);
struct vacm_securityEntry *vacm_scanSecurityEntry (void);
int vacm_is_configured(void);

#ifdef __cplusplus
}
#endif

#endif /* VACM_H */
