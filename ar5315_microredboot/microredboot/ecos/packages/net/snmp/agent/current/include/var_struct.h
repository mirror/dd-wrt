//==========================================================================
//
//      ./agent/current/include/var_struct.h
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
#ifndef VAR_STRUCT_H
#define VAR_STRUCT_H
/*
 * The subtree structure contains a subtree prefix which applies to
 * all variables in the associated variable list.
 *
 * By converting to a tree of subtree structures, entries can
 * now be subtrees of another subtree in the structure. i.e:
 * 1.2
 * 1.2.0
 */
struct subtree {
    oid			name[16];	/* objid prefix of registered subtree */
    u_char 		namelen;	/* number of subid's in name above */
    oid			start[16];	/* objid of start of covered range */
    u_char 		start_len;	/* number of subid's in start name */
    oid			end[16];	/* objid of end of covered range */
    u_char 		end_len;	/* number of subid's in end name */
    struct variable	*variables;   /* pointer to variables array */
    int			variables_len;	/* number of entries in above array */
    int			variables_width; /* sizeof each variable entry */
    char                label[256];     /* calling module's label */
    struct snmp_session *session;
    u_char		flags;
    u_char		priority;
    struct subtree      *next;		/* List of 'sibling' subtrees */
    struct subtree      *prev;		/* Make siblings a doubly-linked list */
    struct subtree      *children;	/* List of 'child' subtrees */
};

/*
 * This is a new variable structure that doesn't have as much memory
 * tied up in the object identifier.  It's elements have also been re-arranged
 * so that the name field can be variable length.  Any number of these
 * structures can be created with lengths tailor made to a particular
 * application.  The first 5 elements of the structure must remain constant.
 */
struct variable2 {
    u_char          magic;          /* passed to function as a hint */
    u_char          type;           /* type of variable */
    u_short         acl;            /* access control list for variable */
    FindVarMethod  *findVar;        /* function that finds variable */
    u_char          namelen;        /* length of name below */
    oid             name[2];       /* object identifier of variable */
};

struct variable4 {
    u_char          magic;          /* passed to function as a hint */
    u_char          type;           /* type of variable */
    u_short         acl;            /* access control list for variable */
    FindVarMethod  *findVar;        /* function that finds variable */
    u_char          namelen;        /* length of name below */
    oid             name[4];       /* object identifier of variable */
};

struct variable7 {
    u_char          magic;          /* passed to function as a hint */
    u_char          type;           /* type of variable */
    u_short         acl;            /* access control list for variable */
    FindVarMethod  *findVar;        /* function that finds variable */
    u_char          namelen;        /* length of name below */
    oid             name[7];       /* object identifier of variable */
};

struct variable8 {
    u_char          magic;          /* passed to function as a hint */
    u_char          type;           /* type of variable */
    u_short         acl;            /* access control list for variable */
    FindVarMethod  *findVar;        /* function that finds variable */
    u_char          namelen;        /* length of name below */
    oid             name[8];       /* object identifier of variable */
};

struct variable13 {
    u_char          magic;          /* passed to function as a hint */
    u_char          type;           /* type of variable */
    u_short         acl;            /* access control list for variable */
    FindVarMethod  *findVar;        /* function that finds variable */
    u_char          namelen;        /* length of name below */
    oid             name[13];       /* object identifier of variable */
};
#endif /* VAR_STRUCT_H */
