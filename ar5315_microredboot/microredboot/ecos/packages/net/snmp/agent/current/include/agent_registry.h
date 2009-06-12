//==========================================================================
//
//      ./agent/current/include/agent_registry.h
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
#ifndef AGENT_REGISTRY_H
#define AGENT_REGISTRY_H

/* the structure of parameters passed to registered ACM modules */
struct view_parameters {
   struct snmp_pdu *pdu;
   oid             *name;
   size_t           namelen;
   int              errorcode; /* do not change unless you're
                                  specifying an error,
                                  as it starts in a success state. */
};

struct register_parameters {
   oid    *name;
   size_t  namelen;
   int     priority;
   int     range_subid;
   oid     range_ubound;
};

#define MIB_REGISTERED_OK		 0
#define MIB_DUPLICATE_REGISTRATION	-1
#define MIB_REGISTRATION_FAILED		-2

#define MIB_UNREGISTERED_OK		 0
#define MIB_NO_SUCH_REGISTRATION	-1
#define MIB_UNREGISTRATION_FAILED	-2

#define DEFAULT_MIB_PRIORITY		127

#define ALLOCATE_THIS_INDEX		0x0
#define ALLOCATE_ANY_INDEX		0x1
#define ALLOCATE_NEW_INDEX		0x3
	/* N.B: it's deliberate that NEW_INDEX & ANY_INDEX == ANY_INDEX */

#define ANY_INTEGER_INDEX		-1
#define ANY_STRING_INDEX		NULL
#define ANY_OID_INDEX			NULL

#define	INDEX_ERR_GENERR		-1
#define	INDEX_ERR_WRONG_TYPE		-2
#define	INDEX_ERR_NOT_ALLOCATED		-3
#define	INDEX_ERR_WRONG_SESSION		-4

char*                 register_string_index( oid *, size_t, char *);
int                   register_int_index( oid *, size_t, int);
struct variable_list* register_oid_index( oid *, size_t, oid *, size_t);
struct variable_list* register_index( struct variable_list *, int, struct snmp_session*);

int  release_index( struct variable_list *);
int  remove_index( struct variable_list *, struct snmp_session*);
void unregister_index_by_session(struct snmp_session *);
int  unregister_index(struct variable_list *, int, struct snmp_session *);

void setup_tree (void);
struct subtree *find_subtree (oid *, size_t, struct subtree *);
struct subtree *find_subtree_next (oid *, size_t, struct subtree *);
struct subtree *find_subtree_previous (oid *, size_t, struct subtree *);
struct snmp_session *get_session_for_oid( oid *, size_t);

int register_mib(const char *, struct variable *, size_t, size_t, oid *, size_t);
int register_mib_priority(const char *, struct variable *, size_t, size_t, oid *, size_t, int);
int register_mib_range(const char *, struct variable *, size_t , size_t , oid *, size_t, int, int, oid, struct snmp_session *);

int unregister_mib (oid *, size_t);
int unregister_mib_priority (oid *, size_t, int);
int unregister_mib_range (oid *, size_t, int, int, oid);
void unregister_mibs_by_session (struct snmp_session *);

struct subtree *free_subtree (struct subtree *);
int compare_tree (const oid *, size_t, const oid *, size_t);
int in_a_view(oid *, size_t *, struct snmp_pdu *, int);
int check_access(struct snmp_pdu *pdu);

/* REGISTER_MIB(): This macro simply loads register_mib with less pain:

   descr:   A short description of the mib group being loaded.
   var:     The variable structure to load.
   vartype: The variable structure used to define it (variable2, variable4, ...)
   theoid:  A *initialized* *exact length* oid pointer.
            (sizeof(theoid) *must* return the number of elements!) 
*/
#define REGISTER_MIB(descr, var, vartype, theoid)                      \
  if (register_mib(descr, (struct variable *) var, sizeof(struct vartype), \
               sizeof(var)/sizeof(struct vartype),                     \
               theoid, sizeof(theoid)/sizeof(oid)) != MIB_REGISTERED_OK ) \
	DEBUGMSGTL(("register_mib", "%s registration failed\n", descr));

#endif /* AGENT_REGISTRY_H */
