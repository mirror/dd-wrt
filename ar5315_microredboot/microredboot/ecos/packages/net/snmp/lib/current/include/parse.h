//==========================================================================
//
//      ./lib/current/include/parse.h
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
#ifndef PARSE_H
#define PARSE_H

#ifdef __cplusplus
extern "C" {
#endif
/*
 * parse.h
 */
/***********************************************************
        Copyright 1989 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/

#define MAXLABEL        64      /* maximum characters in a label */
#define MAXTOKEN        128     /* maximum characters in a token */
#define MAXQUOTESTR     4096    /* maximum characters in a quoted string */

struct variable_list;

/*
 * A linked list of tag-value pairs for enumerated integers.
 */
struct enum_list {
    struct enum_list *next;
    int value;
    char *label;
};

/*
 * A linked list of ranges
 */
struct range_list {
    struct range_list *next;
    int low, high;
};

/*
 * A linked list of indexes
 */
struct index_list {
    struct index_list *next;
    char *ilabel;
    char isimplied;
};

/*
 * A linked list of nodes.
 */
struct node {
    struct node *next;
    char *label;          	/* This node's (unique) textual name */
    u_long  subid;              /* This node's integer subidentifier */
    int     modid;              /* The module containing this node */
    char *parent;               /* The parent's textual name */
    int tc_index;               /* index into tclist (-1 if NA) */
    int type;                   /* The type of object this represents */
    int access;
    int status;
    struct enum_list *enums;    /* (optional) list of enumerated integers */
    struct range_list *ranges;
    struct index_list *indexes;
    char *hint;
    char *units;
    char *description;    	/* description (a quoted string) */
};

/*
 * A tree in the format of the tree structure of the MIB.
 */
struct tree {
    struct tree *child_list;    /* list of children of this node */
    struct tree *next_peer;     /* Next node in list of peers */
    struct tree *next;          /* Next node in hashed list of names */
    struct tree *parent;
    char *label;          	/* This node's textual name */
    u_long subid;               /* This node's integer subidentifier */
    int     modid;              /* The module containing this node */
    int     number_modules;
    int    *module_list;        /* To handle multiple modules */
    int tc_index;               /* index into tclist (-1 if NA) */
    int type;                   /* This node's object type */
    int access;			/* This nodes access */
    int status;			/* This nodes status */
    struct enum_list *enums;    /* (optional) list of enumerated integers */
    struct range_list *ranges;
    struct index_list *indexes;
    char *hint;
    char *units;
    void (*printer) (char *, struct variable_list *, struct enum_list *,
                         const char *, const char *);	/* Value printing function */
    char *description;    	/* description (a quoted string) */
    int  reported;              /* 1=report started in print_subtree... */
};

/*
 * Information held about each MIB module
 */
struct module_import {
    char *label;                /* The descriptor being imported */
    int   modid;                /* The module imported from */
};

struct module {
    char *name;                 /* This module's name */
    char *file;                 /* The file containing the module */
    struct module_import *imports;  /* List of descriptors being imported */
    int  no_imports;            /* The number of such import descriptors */
                     /* -1 implies the module hasn't been read in yet */
    int   modid;                /* The index number of this module */
    struct module *next;        /* Linked list pointer */
};

struct module_compatability {
    const char *old_module;
    const char *new_module;
    const char *tag;		/* NULL implies unconditional replacement,
				otherwise node identifier or prefix */
    size_t tag_len;		/* 0 implies exact match (or unconditional) */
    struct module_compatability *next;	/* linked list */
};


/* non-aggregate types for tree end nodes */
#define TYPE_OTHER          0
#define TYPE_OBJID          1
#define TYPE_OCTETSTR       2
#define TYPE_INTEGER        3
#define TYPE_NETADDR        4
#define TYPE_IPADDR         5
#define TYPE_COUNTER        6
#define TYPE_GAUGE          7
#define TYPE_TIMETICKS      8
#define TYPE_OPAQUE         9
#define TYPE_NULL           10
#define TYPE_COUNTER64      11
#define TYPE_BITSTRING      12
#define TYPE_NSAPADDRESS    13
#define TYPE_UINTEGER       14

#define MIB_ACCESS_READONLY    18
#define MIB_ACCESS_READWRITE   19
#define	MIB_ACCESS_WRITEONLY   20
#define MIB_ACCESS_NOACCESS    21
#define MIB_ACCESS_NOTIFY      67
#define MIB_ACCESS_CREATE      48

#define MIB_STATUS_MANDATORY   23
#define MIB_STATUS_OPTIONAL    24
#define MIB_STATUS_OBSOLETE    25
#define MIB_STATUS_DEPRECATED  39
#define MIB_STATUS_CURRENT     57

#ifdef CMU_COMPATIBLE
#define ACCESS_READONLY		MIB_ACCESS_READONLY
#define ACCESS_READWRITE	MIB_ACCESS_READWRITE
#define ACCESS_WRITEONLY	MIB_ACCESS_WRITEONLY
#define ACCESS_NOACCESS		MIB_ACCESS_NOACCESS
#define ACCESS_NOTIFY		MIB_ACCESS_NOTIFY
#define ACCESS_CREATE		MIB_ACCESS_CREATE
#define STATUS_MANDATORY	MIB_STATUS_MANDATORY
#define STATUS_OPTIONAL		MIB_STATUS_OPTIONAL
#define STATUS_OBSOLETE		MIB_STATUS_OBSOLETE
#define STATUS_DEPRECATED	MIB_STATUS_DEPRECATED
#define STATUS_CURRENT		MIB_STATUS_CURRENT
#endif	/* CMU_COMPATIBLE */

#define	ANON	"anonymous#"
#define	ANON_LEN  strlen(ANON)

struct tree *read_module (const char *);
struct tree *read_mib (const char *);
struct tree *read_all_mibs (void);
int unload_module(const char *name);
void init_mib_internals (void);
int  add_mibdir (const char *);
void add_module_replacement (const char *, const char *, const char *, int);
int  which_module (const char *);
char *module_name (int, char *);
void print_subtree (FILE *, struct tree *, int);
void print_ascii_dump_tree (FILE *, struct tree *, int);
struct tree *find_tree_node (const char *, int);
const char *get_tc_descriptor (int);
struct tree *find_best_tree_node(const char *, struct tree *, u_int *);
 /* backwards compatability */
struct tree *find_node (const char *, struct tree*);
struct module *find_module (int);
void adopt_orphans (void);
void snmp_set_mib_warnings (int);
void snmp_set_mib_errors (int);
void snmp_set_save_descriptions (int);
void snmp_set_mib_comment_term (int);
void snmp_set_mib_parse_label (int);
char *snmp_mib_toggle_options(char *options);
void snmp_mib_toggle_options_usage(const char *lead, FILE *outf);
void print_mib(FILE *);
void print_mib_tree(FILE *, struct tree *, int);
int  get_mib_parse_error_count(void);
int  snmp_get_token(FILE *fp, char *token, int maxtlen);
struct tree *
find_best_tree_node(const char *name, struct tree *tree_top, u_int *match);

#ifdef __cplusplus
}
#endif

#endif /* PARSE_H */
