//==========================================================================
//
//      ./lib/current/src/vacm.c
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
 * vacm.c
 *
 * SNMPv3 View-based Access Control Model
 */

#include <config.h>

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
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
#include <stdio.h>
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

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include "asn1.h"
#include "snmp.h"
#include "snmp_api.h"
#include "vacm.h"
#include "snmp_debug.h"

static struct vacm_viewEntry *viewList = NULL, *viewScanPtr = NULL;
static struct vacm_accessEntry *accessList = NULL, *accessScanPtr = NULL;
static struct vacm_groupEntry *groupList = NULL, *groupScanPtr = NULL;

struct vacm_viewEntry *
vacm_getViewEntry(const char *viewName,
		  oid *viewSubtree,
		  size_t viewSubtreeLen)
{
    struct vacm_viewEntry *vp, *vpret = NULL;
    char view[VACMSTRINGLEN];
    int found, glen;

    glen = (int)strlen(viewName);
    if (glen < 0 || glen >= VACM_MAX_STRING)
        return NULL;
    view[0] = glen;
    strcpy(view+1, viewName);
    for(vp = viewList; vp; vp = vp->next){
        if (!memcmp(view, vp->viewName,glen+1)
	    && viewSubtreeLen >= vp->viewSubtreeLen) {
	    int mask = 0x80, maskpos = 0;
	    int oidpos;
            found = 1;
	    for (oidpos = 0; found && oidpos < (int)vp->viewSubtreeLen; oidpos++) {
		if ((vp->viewMask[maskpos] & mask) != 0) {
		    if (viewSubtree[oidpos] != vp->viewSubtree[oidpos])
                        found = 0;
		}
		if (mask == 1) {
		    mask = 0x80;
		    maskpos++;
		}
		else mask >>= 1;
	    }
            if (found) {
              /* match successful, keep this node if its longer than
                 the previous or (equal and lexicographically greater
                 than the previous). */
              if (vpret == NULL || vp->viewSubtreeLen > vpret->viewSubtreeLen ||
                  (vp->viewSubtreeLen == vpret->viewSubtreeLen &&
                   snmp_oid_compare(vp->viewSubtree, vp->viewSubtreeLen,
                                    vpret->viewSubtree,
                                    vpret->viewSubtreeLen) > 0))
                vpret = vp;
            }
	}
    }
    DEBUGMSGTL(("vacm:getView", ", %s", (vpret)?"found":"none"));
    return vpret;
}

void
vacm_scanViewInit (void)
{
    viewScanPtr = viewList;
}

struct vacm_viewEntry *
vacm_scanViewNext (void)
{
    struct vacm_viewEntry *returnval = viewScanPtr;
    if (viewScanPtr) viewScanPtr = viewScanPtr->next;
    return returnval;
}

struct vacm_viewEntry *
vacm_createViewEntry(const char *viewName,
		     oid *viewSubtree,
		     size_t viewSubtreeLen)
{
    struct vacm_viewEntry *vp, *lp, *op = NULL;
    int cmp, glen;

    glen = (int)strlen(viewName);
    if (glen < 0 || glen >= VACM_MAX_STRING)
        return NULL;
    vp = (struct vacm_viewEntry *)calloc(1, sizeof(struct vacm_viewEntry));
    if (vp == NULL)
        return NULL;
    vp->reserved = (struct vacm_viewEntry *)calloc(1, sizeof(struct vacm_viewEntry));
    if (vp->reserved == NULL) {
        free(vp);
        return NULL;
    }

    vp->viewName[0] = glen;
    strcpy(vp->viewName+1, viewName);
    memcpy(vp->viewSubtree, viewSubtree, viewSubtreeLen * sizeof(oid));
    vp->viewSubtreeLen = viewSubtreeLen;

    lp = viewList;
    while (lp) {
	cmp = memcmp(lp->viewName, vp->viewName, glen+1);
	if (cmp > 0) break;
	if (cmp < 0) goto next;
	
next:
	op = lp;
	lp = lp->next;
    }
    vp->next = lp;
    if (op) op->next = vp;
    else viewList = vp;
    return vp;
}

void
vacm_destroyViewEntry(const char *viewName,
		      oid *viewSubtree,
		      size_t viewSubtreeLen)
{
    struct vacm_viewEntry *vp, *lastvp = NULL;

    if (viewList && !strcmp(viewList->viewName+1, viewName)
	&& viewList->viewSubtreeLen == viewSubtreeLen
	&& !memcmp((char *)viewList->viewSubtree, (char *)viewSubtree,
		 viewSubtreeLen * sizeof(oid))){
	vp = viewList;
	viewList = viewList->next;
    } else {
	for (vp = viewList; vp; vp = vp->next){
	    if (!strcmp(vp->viewName+1, viewName)
		&& vp->viewSubtreeLen  == viewSubtreeLen 
		&& !memcmp((char *)vp->viewSubtree, (char *)viewSubtree,
			 viewSubtreeLen * sizeof(oid)))
		break;
	    lastvp = vp;
	}
	if (!vp)
	    return;
	lastvp->next = vp->next;
    }
    if (vp->reserved)
	free(vp->reserved);
    free(vp);
    return;
}

void vacm_destroyAllViewEntries (void)
{
    struct vacm_viewEntry *vp;
    while ((vp = viewList)) {
	viewList = vp->next;
	if (vp->reserved) free(vp->reserved);
	free(vp);
    }
}

struct vacm_groupEntry *
vacm_getGroupEntry(int securityModel,
		   const char *securityName)
{
    struct vacm_groupEntry *vp;
    char secname[VACMSTRINGLEN];
    int glen;

    glen = (int)strlen(securityName);
    if (glen < 0 || glen >= VACM_MAX_STRING)
        return NULL;
    secname[0] = glen;
    strcpy(secname+1, securityName);

    for (vp = groupList; vp; vp = vp->next) {
	if ((securityModel == vp->securityModel || vp->securityModel == SNMP_SEC_MODEL_ANY)
	    && !memcmp(vp->securityName, secname,glen+1))
	return vp;
    }
    return NULL;
}

void
vacm_scanGroupInit (void)
{
    groupScanPtr = groupList;
}

struct vacm_groupEntry *
vacm_scanGroupNext (void)
{
    struct vacm_groupEntry *returnval = groupScanPtr;
    if (groupScanPtr) groupScanPtr = groupScanPtr->next;
    return returnval;
}

struct vacm_groupEntry *
vacm_createGroupEntry(int securityModel,
		      const char *securityName)
{
    struct vacm_groupEntry *gp, *lg, *og;
    int cmp, glen;

    glen = (int)strlen(securityName);
    if (glen < 0 || glen >= VACM_MAX_STRING)
        return NULL;
    gp = (struct vacm_groupEntry *)calloc(1, sizeof(struct vacm_groupEntry));
    if (gp == NULL)
        return NULL;
    gp->reserved = (struct vacm_groupEntry *)calloc(1, sizeof(struct vacm_groupEntry));
    if (gp->reserved == NULL) {
        free(gp);
        return NULL;
    }

    gp->securityModel = securityModel;
    gp->securityName[0] = glen;
    strcpy(gp->securityName+1, securityName);

    lg = groupList;
    og = NULL;
    while (lg) {
	if (lg->securityModel > securityModel) break;
	if (lg->securityModel == securityModel && 
	    (cmp = memcmp(lg->securityName, gp->securityName, glen+1)) > 0) break;
	/* if (lg->securityModel == securityModel && cmp == 0) abort(); */
	og = lg; lg = lg->next;
    }
    gp->next = lg;
    if (og == NULL) groupList = gp;
    else og->next = gp;
    return gp;
}

void
vacm_destroyGroupEntry(int securityModel,
		       const char *securityName)
{
    struct vacm_groupEntry *vp, *lastvp = NULL;

    if (groupList && groupList->securityModel == securityModel
	&& !strcmp(groupList->securityName+1, securityName)) {
	vp = groupList;
	groupList = groupList->next;
    } else {
	for (vp = groupList; vp; vp = vp->next){
	    if (vp->securityModel == securityModel
		&& !strcmp(vp->securityName+1, securityName))
		break;
	    lastvp = vp;
	}
	if (!vp)
	    return;
	lastvp->next = vp->next;
    }
    if (vp->reserved)
	free(vp->reserved);
    free(vp);
    return;
}

void vacm_destroyAllGroupEntries (void)
{
    struct vacm_groupEntry *gp;
    while ((gp = groupList)) {
	groupList = gp->next;
	if (gp->reserved) free(gp->reserved);
	free(gp);
    }
}

struct vacm_accessEntry *
vacm_getAccessEntry(const char *groupName, 
		    const char *contextPrefix,
		    int securityModel, 
		    int securityLevel)
{
    struct vacm_accessEntry *vp;
    char group[VACMSTRINGLEN];
    char context[VACMSTRINGLEN];
    int glen, clen;

    glen = (int)strlen(groupName);
    if (glen < 0 || glen >= VACM_MAX_STRING)
        return NULL;
    clen = (int)strlen(contextPrefix);
    if (clen < 0 || clen >= VACM_MAX_STRING)
        return NULL;

    group[0] = glen;
    strcpy(group+1, groupName);
    context[0] = clen;
    strcpy(context+1, contextPrefix);
    for(vp = accessList; vp; vp = vp->next){
        if ((securityModel == vp->securityModel || vp->securityModel == SNMP_SEC_MODEL_ANY)
	    && securityLevel >= vp->securityLevel
	    && !memcmp(vp->groupName, group, glen+1)
	    && !memcmp(vp->contextPrefix, context, clen+1))
	  return vp;
    }
    return NULL;
}

void
vacm_scanAccessInit (void)
{
    accessScanPtr = accessList;
}

struct vacm_accessEntry *
vacm_scanAccessNext (void)
{
    struct vacm_accessEntry *returnval = accessScanPtr;
    if (accessScanPtr) accessScanPtr = accessScanPtr->next;
    return returnval;
}

struct vacm_accessEntry *
vacm_createAccessEntry(const char *groupName, 
		       const char *contextPrefix,
		       int securityModel, 
		       int securityLevel)
{
    struct vacm_accessEntry *vp, *lp, *op = NULL;
    int cmp, glen, clen;

    glen = (int)strlen(groupName);
    if (glen < 0 || glen >= VACM_MAX_STRING)
        return NULL;
    clen = (int)strlen(contextPrefix);
    if (clen < 0 || clen >= VACM_MAX_STRING)
        return NULL;
    vp = (struct vacm_accessEntry *)calloc(1, sizeof(struct vacm_accessEntry));
    if (vp == NULL)
        return NULL;
    vp->reserved = (struct vacm_accessEntry *)calloc(1, sizeof(struct vacm_accessEntry));
    if (vp->reserved == NULL) {
        free(vp);
        return NULL;
    }

    vp->securityModel = securityModel;
    vp->securityLevel = securityLevel;
    vp->groupName[0] = glen;
    strcpy(vp->groupName+1, groupName);
    vp->contextPrefix[0] = clen;
    strcpy(vp->contextPrefix+1, contextPrefix);

    lp = accessList;
    while (lp) {
	cmp = memcmp(lp->groupName, vp->groupName, glen+1);
	if (cmp > 0) break;
	if (cmp < 0) goto next;
	cmp = memcmp(lp->contextPrefix, vp->contextPrefix, clen+1);
	if (cmp > 0) break;
	if (cmp < 0) goto next;
	if (lp->securityModel > securityModel) break;
	if (lp->securityModel < securityModel) goto next;
	if (lp->securityLevel > securityLevel) break;
next:
	op = lp;
	lp = lp->next;
    }
    vp->next = lp;
    if (op == NULL) accessList = vp;
    else op->next = vp;
    return vp;
}

void
vacm_destroyAccessEntry(const char *groupName, 
			const char *contextPrefix,
			int securityModel,
			int securityLevel)
{
    struct vacm_accessEntry *vp, *lastvp = NULL;

    if (accessList && accessList->securityModel == securityModel
	&& accessList->securityModel == securityModel
	&& !strcmp(accessList->groupName+1, groupName)
	&& !strcmp(accessList->contextPrefix+1, contextPrefix)) {
	vp = accessList;
	accessList = accessList->next;
    } else {
	for (vp = accessList; vp; vp = vp->next){
	    if (vp->securityModel == securityModel
		&& vp->securityLevel == securityLevel
		&& !strcmp(vp->groupName+1, groupName)
		&& !strcmp(vp->contextPrefix+1, contextPrefix))
		break;
	    lastvp = vp;
	}
	if (!vp)
	    return;
	lastvp->next = vp->next;
    }
    if (vp->reserved)
	free(vp->reserved);
    free(vp);
    return;
}

void vacm_destroyAllAccessEntries (void)
{
    struct vacm_accessEntry *ap;
    while ((ap = accessList)) {
	accessList = ap->next;
	if (ap->reserved) free(ap->reserved);
	free(ap);
    }
}

/* returns 1 if vacm has *any* configuration entries in it (regardless
   of weather or not there is enough to make a decision based on it),
   else return 0 */
int vacm_is_configured(void) {
    if (viewList == NULL && accessList == NULL && groupList == NULL)
        return 0;
    return 1;
}

#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */
