//==========================================================================
//
//      ./lib/current/src/parse.c
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
 * parse.c
 *
 * Update: 1998-09-22 <mslifcak@iss.net>
 * Clear nbuckets in init_node_hash.
 * New method xcalloc returns zeroed data structures.
 * New method alloc_node encapsulates common node creation.
 * New method to configure terminate comment at end of line.
 * New method to configure accept underscore in labels.
 *
 * Update: 1998-10-10 <daves@csc.liv.ac.uk>
 * fully qualified OID parsing patch
 *
 * Update: 1998-10-20 <daves@csc.liv.ac.uk>
 * merge_anon_children patch
 *
 * Update: 1998-10-21 <mslifcak@iss.net>
 * Merge_parse_objectid associates information with last node in chain.
 */
/******************************************************************
        Copyright 1989, 1991, 1992 by Carnegie Mellon University

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
#include <config.h>
#include <snmp_logging.h>
#include <stdio.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <ctype.h>
#include <sys/types.h>
#ifndef __ECOS
#include <sys/stat.h>
#endif

/* Wow.  This is ugly.  -- Wes */
#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
#include <regex.h>
#endif
#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include "system.h"
#include "parse.h"
#include "asn1.h"
#include "mib.h"
#include "snmp_api.h"
#include "snmp_debug.h"
#include "snmp_logging.h"
#include "default_store.h"
#include "tools.h"

/*
 * This is one element of an object identifier with either an integer
 * subidentifier, or a textual string label, or both.
 * The subid is -1 if not present, and label is NULL if not present.
 */
struct subid_s {
    int subid;
    int modid;
    char *label;
};

#define MAXTC   1024
struct tc {     /* textual conventions */
    int type;
    int modid;
    char *descriptor;
    char *hint;
    struct enum_list *enums;
    struct range_list *ranges;
} tclist[MAXTC];

int Line = 0;
const char *File = "(none)";
static int anonymous = 0;

#define SYNTAX_MASK     0x80
/* types of tokens
 Tokens wiht the SYNTAX_MASK bit set are syntax tokens */
#define CONTINUE    -1
#define ENDOFFILE   0
#define LABEL       1
#define SUBTREE     2
#define SYNTAX      3
#define OBJID       (4 | SYNTAX_MASK)
#define OCTETSTR    (5 | SYNTAX_MASK)
#define INTEGER     (6 | SYNTAX_MASK)
#define INTEGER32   INTEGER
#define NETADDR     (7 | SYNTAX_MASK)
#define IPADDR      (8 | SYNTAX_MASK)
#define COUNTER     (9 | SYNTAX_MASK)
#define GAUGE       (10 | SYNTAX_MASK)
#define TIMETICKS   (11 | SYNTAX_MASK)
#define KW_OPAQUE   (12 | SYNTAX_MASK)
#define NUL         (13 | SYNTAX_MASK)
#define SEQUENCE    14
#define OF          15  /* SEQUENCE OF */
#define OBJTYPE     16
#define ACCESS      17
#define READONLY    18
#define READWRITE   19
#define WRITEONLY   20
#define NOACCESS    21
#define STATUS      22
#define MANDATORY   23
#define KW_OPTIONAL    24
#define OBSOLETE    25
/* #define RECOMMENDED 26 */
#define PUNCT       27
#define EQUALS      28
#define NUMBER      29
#define LEFTBRACKET 30
#define RIGHTBRACKET 31
#define LEFTPAREN   32
#define RIGHTPAREN  33
#define COMMA       34
#define DESCRIPTION 35
#define QUOTESTRING 36
#define INDEX       37
#define DEFVAL      38
#define DEPRECATED  39
#define SIZE        40
#define BITSTRING   (41 | SYNTAX_MASK)
#define NSAPADDRESS (42 | SYNTAX_MASK)
#define COUNTER64   (43 | SYNTAX_MASK)
#define OBJGROUP    44
#define NOTIFTYPE   45
#define AUGMENTS    46
#define COMPLIANCE  47
#define READCREATE  48
#define UNITS       49
#define REFERENCE   50
#define NUM_ENTRIES 51
#define MODULEIDENTITY 52
#define LASTUPDATED 53
#define ORGANIZATION 54
#define CONTACTINFO 55
#define UINTEGER32 (56 | SYNTAX_MASK)
#define CURRENT     57
#define DEFINITIONS 58
#define END         59
#define SEMI        60
#define TRAPTYPE    61
#define ENTERPRISE  62
/* #define DISPLAYSTR (63 | SYNTAX_MASK) */
#define BEGIN       64
#define IMPORTS     65
#define EXPORTS     66
#define ACCNOTIFY   67
#define BAR         68
#define RANGE       69
#define CONVENTION  70
#define DISPLAYHINT 71
#define FROM        72
#define CAPABILITIES 73
#define MACRO       74
#define IMPLIED     75

struct tok {
    const char *name;                 /* token name */
    int len;                    /* length not counting nul */
    int token;                  /* value */
    int hash;                   /* hash of name */
    struct tok *next;           /* pointer to next in hash table */
};


static struct tok tokens[] = {
    { "obsolete", sizeof ("obsolete")-1, OBSOLETE },
    { "Opaque", sizeof ("Opaque")-1, KW_OPAQUE },
    { "optional", sizeof ("optional")-1, KW_OPTIONAL },
    { "LAST-UPDATED", sizeof ("LAST-UPDATED")-1, LASTUPDATED },
    { "ORGANIZATION", sizeof ("ORGANIZATION")-1, ORGANIZATION },
    { "CONTACT-INFO", sizeof ("CONTACT-INFO")-1, CONTACTINFO },
    { "MODULE-IDENTITY", sizeof ("MODULE-IDENTITY")-1, MODULEIDENTITY },
    { "MODULE-COMPLIANCE", sizeof ("MODULE-COMPLIANCE")-1, COMPLIANCE },
    { "DEFINITIONS", sizeof("DEFINITIONS")-1, DEFINITIONS},
    { "END", sizeof("END")-1, END},
    { "AUGMENTS", sizeof ("AUGMENTS")-1, AUGMENTS },
    { "not-accessible", sizeof ("not-accessible")-1, NOACCESS },
    { "write-only", sizeof ("write-only")-1, WRITEONLY },
    { "NsapAddress", sizeof("NsapAddress")-1, NSAPADDRESS},
    { "UNITS", sizeof("Units")-1, UNITS},
    { "REFERENCE", sizeof("REFERENCE")-1, REFERENCE},
    { "NUM-ENTRIES", sizeof("NUM-ENTRIES")-1, NUM_ENTRIES},
    { "BITSTRING", sizeof("BITSTRING")-1, BITSTRING},
    { "BIT", sizeof("BIT")-1, CONTINUE},
    { "BITS", sizeof("BITS")-1, BITSTRING},
    { "Counter64", sizeof("Counter64")-1, COUNTER64},
    { "TimeTicks", sizeof ("TimeTicks")-1, TIMETICKS },
    { "NOTIFICATION-TYPE", sizeof ("NOTIFICATION-TYPE")-1, NOTIFTYPE },
    { "OBJECT-GROUP", sizeof ("OBJECT-GROUP")-1, OBJGROUP },
    { "OBJECT-IDENTITY", sizeof ("OBJECT-IDENTITY")-1, OBJGROUP },
    { "OBJECTIDENTIFIER", sizeof ("OBJECTIDENTIFIER")-1, OBJID },
    { "OBJECT", sizeof ("OBJECT")-1, CONTINUE },
    { "NetworkAddress", sizeof ("NetworkAddress")-1, NETADDR },
    { "Gauge", sizeof ("Gauge")-1, GAUGE },
    { "Gauge32", sizeof ("Gauge32")-1, GAUGE },
    { "Unsigned32", sizeof ("Unsigned32")-1, GAUGE },
    { "read-write", sizeof ("read-write")-1, READWRITE },
    { "read-create", sizeof ("read-create")-1, READCREATE },
    { "OCTETSTRING", sizeof ("OCTETSTRING")-1, OCTETSTR },
    { "OCTET", sizeof ("OCTET")-1, CONTINUE },
    { "OF", sizeof ("OF")-1, OF },
    { "SEQUENCE", sizeof ("SEQUENCE")-1, SEQUENCE },
    { "NULL", sizeof ("NULL")-1, NUL },
    { "IpAddress", sizeof ("IpAddress")-1, IPADDR },
    { "UInteger32", sizeof ("UInteger32")-1, UINTEGER32 },
    { "INTEGER", sizeof ("INTEGER")-1, INTEGER },
    { "Integer32", sizeof ("Integer32")-1, INTEGER32 },
    { "Counter", sizeof ("Counter")-1, COUNTER },
    { "Counter32", sizeof ("Counter32")-1, COUNTER },
    { "read-only", sizeof ("read-only")-1, READONLY },
    { "DESCRIPTION", sizeof ("DESCRIPTION")-1, DESCRIPTION },
    { "INDEX", sizeof ("INDEX")-1, INDEX },
    { "DEFVAL", sizeof ("DEFVAL")-1, DEFVAL },
    { "deprecated", sizeof ("deprecated")-1, DEPRECATED },
    { "SIZE", sizeof ("SIZE")-1, SIZE },
    { "MAX-ACCESS", sizeof ("MAX-ACCESS")-1, ACCESS },
    { "ACCESS", sizeof ("ACCESS")-1, ACCESS },
    { "mandatory", sizeof ("mandatory")-1, MANDATORY },
    { "current", sizeof ("current")-1, CURRENT },
    { "STATUS", sizeof ("STATUS")-1, STATUS },
    { "SYNTAX", sizeof ("SYNTAX")-1, SYNTAX },
    { "OBJECT-TYPE", sizeof ("OBJECT-TYPE")-1, OBJTYPE },
    { "TRAP-TYPE", sizeof ("TRAP-TYPE")-1, TRAPTYPE },
    { "ENTERPRISE", sizeof ("ENTERPRISE")-1, ENTERPRISE },
    { "BEGIN", sizeof ("BEGIN")-1, BEGIN },
    { "IMPORTS", sizeof ("IMPORTS")-1, IMPORTS },
    { "EXPORTS", sizeof ("EXPORTS")-1, EXPORTS },
    { "accessible-for-notify", sizeof ("accessible-for-notify")-1, ACCNOTIFY },
    { "TEXTUAL-CONVENTION", sizeof ("TEXTUAL-CONVENTION")-1, CONVENTION },
    { "NOTIFICATION-GROUP", sizeof ("NOTIFICATION-GROUP")-1, NOTIFTYPE },
    { "DISPLAY-HINT", sizeof ("DISPLAY-HINT")-1, DISPLAYHINT },
    { "FROM", sizeof ("FROM")-1, FROM },
    { "AGENT-CAPABILITIES", sizeof ("AGENT-CAPABILITIES")-1, CAPABILITIES },
    { "MACRO", sizeof ("MACRO")-1, MACRO },
    { "IMPLIED", sizeof ("IMPLIED")-1, IMPLIED },
    { NULL }
};

static struct module_compatability *module_map_head;
static struct module_compatability module_map[] = {
	{ "RFC1065-SMI",	"RFC1155-SMI",	NULL,	0},
	{ "RFC1066-MIB",	"RFC1156-MIB",	NULL,	0},
	         	/* 'mib' -> 'mib-2' */
	{ "RFC1156-MIB",	"RFC1158-MIB",	NULL,	0},
	         	/* 'snmpEnableAuthTraps' -> 'snmpEnableAuthenTraps' */
	{ "RFC1158-MIB",       "RFC1213-MIB",	NULL,	0},
	         	/* 'nullOID' -> 'zeroDotZero' */
	{ "RFC1155-SMI",	"SNMPv2-SMI",	NULL,	0},
	{ "RFC1213-MIB",	"SNMPv2-SMI",	"mib-2", 0},
	{ "RFC1213-MIB",	"SNMPv2-MIB",	"sys",	3},
	{ "RFC1213-MIB",	"IF-MIB",	"if",	2},
	{ "RFC1213-MIB",	"IP-MIB",	"ip",	2},
	{ "RFC1213-MIB",	"IP-MIB",	"icmp",	4},
	{ "RFC1213-MIB",	"TCP-MIB",	"tcp",	3},
	{ "RFC1213-MIB",	"UDP-MIB",	"udp",	3},
	{ "RFC1213-MIB",	"SNMPv2-SMI",	"transmission", 0},
	{ "RFC1213-MIB",	"SNMPv2-MIB",	"snmp",	4},
	{ "RFC1271-MIB",	"RMON-MIB",	NULL,	0},
	{ "RFC1286-MIB",	"SOURCE-ROUTING-MIB",	"dot1dSr", 7},
	{ "RFC1286-MIB",	"BRIDGE-MIB",	NULL,	0},
	{ "RFC1315-MIB",	"FRAME-RELAY-DTE-MIB",	NULL,	0},
	{ "RFC1316-MIB",	"CHARACTER-MIB", NULL,	0},
};
#define MODULE_NOT_FOUND	0
#define MODULE_LOADED_OK	1
#define MODULE_ALREADY_LOADED	2
/* #define MODULE_LOAD_FAILED	3 	*/
#define MODULE_LOAD_FAILED	MODULE_NOT_FOUND


#define HASHSIZE        32
#define BUCKET(x)       (x & (HASHSIZE-1))

#define NHASHSIZE    128
#define NBUCKET(x)   (x & (NHASHSIZE-1))

static struct tok      *buckets[HASHSIZE];

static struct node *nbuckets[NHASHSIZE];
static struct tree *tbuckets[NHASHSIZE];
static struct module *module_head = NULL;

struct node *orphan_nodes = NULL;
struct tree   *tree_head = NULL;

#define	NUMBER_OF_ROOT_NODES	3
static struct module_import	root_imports[NUMBER_OF_ROOT_NODES];

static int current_module = 0;
static int     max_module = 0;
static char *last_err_module = 0; /* no repeats on "Cannot find module..." */

static void tree_from_node(struct tree *tp, struct node *np);
static void do_subtree (struct tree *, struct node **);
static void do_linkup (struct module *, struct node *);
static void dump_module_list (void);
static int get_token (FILE *, char *, int);
static int parseQuoteString (FILE *, char *, int);
static int tossObjectIdentifier (FILE *);
static int  name_hash (const char *);
static void init_node_hash (struct node *);
static void print_error (const char *, const char *, int);
static void free_tree (struct tree *);
static void free_partial_tree (struct tree *, int);
static void free_node (struct node *);
static void build_translation_table (void);
static void init_tree_roots (void);
static void merge_anon_children (struct tree *, struct tree *);
static void unlink_tbucket(struct tree *);
static void unlink_tree(struct tree *);
static int getoid (FILE *, struct subid_s *, int);
static struct node *parse_objectid (FILE *, char *);
static int get_tc (const char *, int, int *, struct enum_list **, struct range_list **, char **);
static int get_tc_index (const char *, int);
static struct enum_list *parse_enumlist (FILE *, struct enum_list **);
static struct range_list *parse_ranges(FILE *fp, struct range_list **);
static struct node *parse_asntype (FILE *, char *, int *, char *);
static struct node *parse_objecttype (FILE *, char *);
static struct node *parse_objectgroup (FILE *, char *);
static struct node *parse_notificationDefinition (FILE *, char *);
static struct node *parse_trapDefinition (FILE *, char *);
static struct node *parse_compliance (FILE *, char *);
static struct node *parse_capabilities(FILE *, char *);
static struct node *parse_moduleIdentity (FILE *, char *);
static struct node *parse_macro(FILE *, char *);
static        void  parse_imports (FILE *);
static struct node *parse (FILE *, struct node *);

static int read_module_internal (const char *);
static void read_module_replacements (const char *);
static void read_import_replacements (const char *, struct module_import *);

static void  new_module  (const char *, const char *);

static struct node *merge_parse_objectid (struct node *, FILE *, char *);
static struct index_list *getIndexes(FILE *fp, struct index_list **);
static void free_indexes(struct index_list **);
static void free_ranges(struct range_list **);
static void free_enums(struct enum_list **);
static struct range_list * copy_ranges(struct range_list *);
static struct enum_list  * copy_enums(struct enum_list *);
static struct index_list * copy_indexes(struct index_list *);

/* backwards compatibility wrappers */
void snmp_set_mib_errors(int err)
{
  ds_set_boolean(DS_LIBRARY_ID, DS_LIB_MIB_ERRORS, err);
}

void snmp_set_mib_warnings(int warn)
{
  ds_set_int(DS_LIBRARY_ID, DS_LIB_MIB_WARNINGS, warn);
}

void snmp_set_save_descriptions(int save)
{
  ds_set_boolean(DS_LIBRARY_ID, DS_LIB_SAVE_MIB_DESCRS, save);
}

void snmp_set_mib_comment_term(int save)
{
  /* 0=strict, 1=EOL terminated */
  ds_set_boolean(DS_LIBRARY_ID, DS_LIB_MIB_COMMENT_TERM, save);
}

void snmp_set_mib_parse_label(int save)
{
  /* 0=strict, 1=underscore OK in label */
  ds_set_boolean(DS_LIBRARY_ID, DS_LIB_MIB_PARSE_LABEL, save);
}

/* end wrappers */

void snmp_mib_toggle_options_usage(const char *lead, FILE *outf) {
  fprintf(outf, "%sMIBOPTS values:\n", lead);
  fprintf(outf, "%s    u: %sallow the usage of underlines in mib symbols.\n",
          lead, ((ds_get_boolean(DS_LIBRARY_ID, DS_LIB_MIB_PARSE_LABEL))?"dis":""));
  fprintf(outf, "%s    c: %sallow the usage of \"--\" to terminate comments.\n",
          lead, ((ds_get_boolean(DS_LIBRARY_ID, DS_LIB_MIB_COMMENT_TERM))?"":"dis"));
  fprintf(outf, "%s    d: %ssave the descriptions of the mib objects.\n",
          lead, ((ds_get_boolean(DS_LIBRARY_ID, DS_LIB_SAVE_MIB_DESCRS))?"don't ":""));
  fprintf(outf, "%s    e: Disable mib errors of MIB symbols conflicts\n",
          lead);
  fprintf(outf, "%s    w: Enable mib warnings of MIB symbols conflicts\n",
          lead);
  fprintf(outf, "%s    W: Enable detailed warnings of MIB symbols conflicts\n",
          lead);
  fprintf(outf, "%s    R: Replace MIB symbols from latest module\n",
          lead);
}

char *snmp_mib_toggle_options(char *options) {
  if (options) {
    while(*options) {
      switch(*options) {
        case 'u':
          ds_set_boolean(DS_LIBRARY_ID, DS_LIB_MIB_PARSE_LABEL, !ds_get_boolean(DS_LIBRARY_ID, DS_LIB_MIB_PARSE_LABEL));
          break;

        case 'c':
          ds_toggle_boolean(DS_LIBRARY_ID, DS_LIB_MIB_COMMENT_TERM);
          break;

        case 'e':
          ds_toggle_boolean(DS_LIBRARY_ID, DS_LIB_MIB_ERRORS);
          break;

        case 'w':
          ds_set_int(DS_LIBRARY_ID, DS_LIB_MIB_WARNINGS, 1);
          break;

        case 'W':
          ds_set_int(DS_LIBRARY_ID, DS_LIB_MIB_WARNINGS, 2);
          break;

        case 'd':
          ds_toggle_boolean(DS_LIBRARY_ID, DS_LIB_SAVE_MIB_DESCRS);
          break;

        case 'R':
          ds_toggle_boolean(DS_LIBRARY_ID, DS_LIB_MIB_REPLACE);
          break;

        default:
          /* return at the unknown option */
          return options;
      }
      options++;
    }
  }
  return NULL;
}

static int
name_hash(const char* name)
{
    int hash = 0;
    const char *cp;

    if (name) {
      for(cp = name; *cp; cp++) {
        hash += tolower(*cp);
      }
    }
    return(hash);
}

void
init_mib_internals (void)
{
    register struct tok *tp;
    register int        b, i;
    int			max_modc;

    if (tree_head)
	return;

	/*
	 * Set up hash list of pre-defined tokens
	 */
    memset(buckets, 0, sizeof(buckets));
    for (tp = tokens; tp->name; tp++) {
        tp->hash = name_hash( tp->name );
        b = BUCKET(tp->hash);
        if (buckets[b])
            tp->next = buckets[b]; /* BUG ??? */
        buckets[b] = tp;
    }

	/*
	 * Initialise other internal structures
	 */

    max_modc = sizeof(module_map)/sizeof(module_map[0])-1;
    for ( i = 0; i < max_modc; ++i )
	module_map[i].next = &(module_map[i+1]);
    module_map[max_modc].next = NULL;
    module_map_head = module_map;

    memset(nbuckets, 0, sizeof(nbuckets));
    memset(tbuckets, 0, sizeof(tbuckets));
    memset(tclist, 0, MAXTC * sizeof(struct tc));
    build_translation_table();
    init_tree_roots();	/* Set up initial roots */
		/* Relies on 'add_mibdir' having set up the modules */
}

static void
init_node_hash(struct node *nodes)
{
     register struct node *np, *nextp;
     register int hash;

     memset(nbuckets, 0, sizeof(nbuckets));
     for(np = nodes; np;){
         nextp = np->next;
         hash = NBUCKET(name_hash(np->parent));
         np->next = nbuckets[hash];
         nbuckets[hash] = np;
         np = nextp;
     }
}

static int erroneousMibs = 0;

int get_mib_parse_error_count(void)
{
    return erroneousMibs;
}


static void
print_error(const char *string,
	    const char *token,
	    int type)
{
    erroneousMibs++;
    DEBUGMSGTL(("parse-mibs", "\n"));
    if (type == ENDOFFILE)
        snmp_log(LOG_ERR, "%s (EOF): At line %d in %s\n", string, Line,
                File);
    else if (token && *token)
        snmp_log(LOG_ERR, "%s (%s): At line %d in %s\n", string, token,
                Line, File);
    else
        snmp_log(LOG_ERR, "%s: At line %d in %s\n", string, Line, File);
}

static void
print_module_not_found(const char *cp)
{
    if (!last_err_module || strcmp(cp, last_err_module))
        print_error("Cannot find module", cp, CONTINUE);
    if (last_err_module) free(last_err_module);
    last_err_module = strdup(cp);
}

static struct node *
alloc_node(int modid)
{
    struct node *np;
    np = (struct node *) calloc(1, sizeof(struct node));
    if (np) {
        np->tc_index = -1;
        np->modid = modid;
    }
    return np;
}

static void unlink_tbucket(struct tree *tp)
{
    int hash = NBUCKET(name_hash(tp->label));
    struct tree *otp = NULL, *ntp = tbuckets[hash];

    while (ntp && ntp != tp) {
	otp = ntp; ntp = ntp->next;
    }
    if (!ntp) snmp_log(LOG_EMERG, "Can't find %s in tbuckets\n", tp->label);
    else if (otp) otp->next = ntp->next;
    else tbuckets[hash] = tp->next;
}

static void unlink_tree(struct tree *tp)
{
    struct tree *otp = NULL, *ntp = tp->parent->child_list;

    while (ntp && ntp != tp) {
	otp = ntp; ntp = ntp->next_peer;
    }
    if (!ntp) snmp_log(LOG_EMERG, "Can't find %s in %s's children\n",
	tp->label, tp->parent->label);
    else if (otp) otp->next_peer = ntp->next_peer;
    else tp->parent->child_list = tp->next_peer;
}

static void
free_partial_tree(struct tree *tp, int keep_label)
{
    if ( !tp)
        return;

    /* remove the data from this tree node */
    free_enums(&tp->enums);
    free_ranges(&tp->ranges);
    free_indexes(&tp->indexes);
    if (!keep_label)
	SNMP_FREE(tp->label);
    SNMP_FREE(tp->hint);
    SNMP_FREE(tp->units);
    SNMP_FREE(tp->description);
}

/*
 * free a tree node. Note: the node must already have been unlinked
 * from the tree when calling this routine
 */
static void
free_tree(struct tree *Tree)
{
    if (!Tree)
        return;

    unlink_tbucket(Tree);
    free_partial_tree (Tree, FALSE);
    if (Tree->number_modules > 1 )
        free((char*)Tree->module_list);
    free ((char*)Tree);
}

static void
free_node(struct node *np)
{
    if ( !np) return;

    free_enums(&np->enums);
    free_ranges(&np->ranges);
    free_indexes(&np->indexes);
    if (np->label) free(np->label);
    if (np->hint) free(np->hint);
    if (np->units) free(np->units);
    if (np->description) free(np->description);
    if (np->parent) free(np->parent);
    free((char*)np);
}

#ifdef TEST
static void
print_nodes(FILE *fp,
	    struct node *root)
{
extern void xmalloc_stats (FILE *);
    struct enum_list *ep;
    struct index_list *ip;
    struct range_list *rp;
    struct node *np;

    for(np = root; np; np = np->next){
        fprintf(fp, "%s ::= { %s %ld } (%d)\n", np->label, np->parent,
                np->subid, np->type);
        if (np->tc_index >= 0)
            fprintf(fp, "  TC = %s\n", tclist[np->tc_index].descriptor);
        if (np->enums){
            fprintf(fp, "  Enums: \n");
            for(ep = np->enums; ep; ep = ep->next){
                fprintf(fp, "    %s(%d)\n", ep->label, ep->value);
            }
        }
        if (np->ranges){
            fprintf(fp, "  Ranges: \n");
            for(rp = np->ranges; rp; rp = rp->next){
                fprintf(fp, "    %d..%d\n", rp->low, rp->high);
            }
        }
        if (np->indexes){
            fprintf(fp, "  Indexes: \n");
            for(ip = np->indexes; ip; ip = ip->next){
                fprintf(fp, "    %s\n", ip->ilabel);
            }
        }
        if (np->hint)
            fprintf(fp, "  Hint: %s\n", np->hint);
        if (np->units)
            fprintf(fp, "  Units: %s\n", np->units);
    }
}
#endif

void
print_subtree(FILE *f,
	      struct tree *tree,
	      int count)
{
    struct tree *tp;
    int i;
    char modbuf[256];

    for(i = 0; i < count; i++)
        fprintf(f, "  ");
    fprintf(f, "Children of %s(%ld):\n", tree->label, tree->subid);
    count++;
    for(tp = tree->child_list; tp; tp = tp->next_peer){
        for(i = 0; i < count; i++)
            fprintf(f, "  ");
        fprintf(f, "%s:%s(%ld) type=%d",
                module_name(tp->module_list[0], modbuf),
                tp->label, tp->subid, tp->type);
        if (tp->tc_index != -1) fprintf(f, " tc=%d", tp->tc_index);
        if (tp->hint) fprintf(f, " hint=%s", tp->hint);
        if (tp->units) fprintf(f, " units=%s", tp->units);
	if (tp->number_modules > 1) {
	    fprintf(f, " modules:");
	    for (i = 1; i < tp->number_modules; i++)
		fprintf(f, " %s", module_name(tp->module_list[i], modbuf));
	}
	fprintf(f, "\n");
    }
    for(tp = tree->child_list; tp; tp = tp->next_peer){
        if (tp->child_list)
            print_subtree(f, tp, count);
    }
}

void
print_ascii_dump_tree(FILE *f,
		      struct tree *tree,
		      int count)
{
    struct tree *tp;

    count++;
    for(tp = tree->child_list; tp; tp = tp->next_peer){
          fprintf(f, "%s OBJECT IDENTIFIER ::= { %s %ld }\n", tp->label, tree->label, tp->subid);
    }
    for(tp = tree->child_list; tp; tp = tp->next_peer){
        if (tp->child_list)
            print_ascii_dump_tree(f, tp, count);
    }
}

static int translation_table[256];

static void
build_translation_table()
{
    int count;

    for(count = 0; count < 256; count++){
        switch(count){
            case OBJID:
                translation_table[count] = TYPE_OBJID;
                break;
            case OCTETSTR:
                translation_table[count] = TYPE_OCTETSTR;
                break;
            case INTEGER:
                translation_table[count] = TYPE_INTEGER;
                break;
            case NETADDR:
                translation_table[count] = TYPE_IPADDR;
                break;
            case IPADDR:
                translation_table[count] = TYPE_IPADDR;
                break;
            case COUNTER:
                translation_table[count] = TYPE_COUNTER;
                break;
            case GAUGE:
                translation_table[count] = TYPE_GAUGE;
                break;
            case TIMETICKS:
                translation_table[count] = TYPE_TIMETICKS;
                break;
            case KW_OPAQUE:
                translation_table[count] = TYPE_OPAQUE;
                break;
            case NUL:
                translation_table[count] = TYPE_NULL;
                break;
            case COUNTER64:
                translation_table[count] = TYPE_COUNTER64;
                break;
            case BITSTRING:
                translation_table[count] = TYPE_BITSTRING;
                break;
            case NSAPADDRESS:
                translation_table[count] = TYPE_NSAPADDRESS;
                break;
            case UINTEGER32:
                translation_table[count] = TYPE_UINTEGER;
                break;
            default:
                translation_table[count] = TYPE_OTHER;
                break;
        }
    }
}

static void
init_tree_roots()
{
    struct tree *tp, *lasttp;
    int  base_modid;
    int  hash;

    base_modid = which_module("SNMPv2-SMI");
    if (base_modid == -1 )
        base_modid = which_module("RFC1155-SMI");
    if (base_modid == -1 )
        base_modid = which_module("RFC1213-MIB");

    /* build root node */
    tp = (struct tree *) calloc(1, sizeof(struct tree));
    if (tp == NULL) return;
    tp->label = strdup("joint-iso-ccitt");
    tp->modid = base_modid;
    tp->number_modules = 1;
    tp->module_list = &(tp->modid);
    tp->subid = 2;
    tp->tc_index = -1;
    set_function(tp);		/* from mib.c */
    hash = NBUCKET(name_hash(tp->label));
    tp->next = tbuckets[hash];
    tbuckets[hash] = tp;
    lasttp = tp;
    root_imports[0].label = strdup( tp->label );
    root_imports[0].modid = base_modid;

    /* build root node */
    tp = (struct tree *) calloc(1, sizeof(struct tree));
    if (tp == NULL) return;
    tp->next_peer = lasttp;
    tp->label = strdup("ccitt");
    tp->modid = base_modid;
    tp->number_modules = 1;
    tp->module_list = &(tp->modid);
    tp->subid = 0;
    tp->tc_index = -1;
    set_function(tp);		/* from mib.c */
    hash = NBUCKET(name_hash(tp->label));
    tp->next = tbuckets[hash];
    tbuckets[hash] = tp;
    lasttp = tp;
    root_imports[1].label = strdup( tp->label );
    root_imports[1].modid = base_modid;

    /* build root node */
    tp = (struct tree *) calloc(1, sizeof(struct tree));
    if (tp == NULL) return;
    tp->next_peer = lasttp;
    tp->label = strdup("iso");
    tp->modid = base_modid;
    tp->number_modules = 1;
    tp->module_list = &(tp->modid);
    tp->subid = 1;
    tp->tc_index = -1;
    set_function(tp);		/* from mib.c */
    hash = NBUCKET(name_hash(tp->label));
    tp->next = tbuckets[hash];
    tbuckets[hash] = tp;
    lasttp = tp;
    root_imports[2].label = strdup( tp->label );
    root_imports[2].modid = base_modid;

    tree_head = tp;
}

#ifdef STRICT_MIB_PARSEING
#define	label_compare	strcasecmp
#else
#define	label_compare	strcmp
#endif


struct tree *
find_tree_node(const char *name,
	       int modid)
{
    struct tree *tp, *headtp;
    int count, *int_p;

    if (!name || !*name)
	return(NULL);

    headtp = tbuckets[NBUCKET(name_hash(name))];
    for ( tp = headtp ; tp ; tp=tp->next ) {
        if ( !label_compare(tp->label, name) ) {

            if ( modid == -1 )	/* Any module */
                return(tp);

            for (int_p = tp->module_list, count=0 ;
                       count < tp->number_modules ;
                       ++count, ++int_p )
                if ( *int_p == modid )
                    return(tp);
        }
    }

    return(NULL);
}

/* computes a value which represents how close name1 is to name2.
 * high scores mean a worse match.
 * (yes, the algorithm sucks!)
 */
#define MAX_BAD 0xffffff

u_int
compute_match(const char *search_base, const char *key) {
#if defined(HAVE_REGEX_H) && defined(HAVE_REGCOMP)
    int rc;
    regex_t parsetree;
    regmatch_t pmatch;

    rc=regcomp(&parsetree, key, REG_ICASE | REG_EXTENDED);
    if (rc == 0)
        rc=regexec(&parsetree, search_base, 1, &pmatch, 0);
    regfree(&parsetree);
    if (rc == 0) {
        /* found */
        return pmatch.rm_so;
    }
#else /* use our own wildcard matcher */
    /* first find the longest matching substring (ick) */
    char *first = NULL, *result = NULL, *entry;
    const char *position;
    char *newkey = strdup(key);


    entry = strtok( newkey, "*" );
    position = search_base;
    while ( entry ) {
        result = strcasestr(position, entry);

        if (result == NULL) {
            free(newkey);
            return MAX_BAD;
        }

        if (first == NULL)
            first = result;

        position = result + strlen(entry);
        entry = strtok( NULL, "*" );
    }
    free(newkey);
    if (result)
        return(first-search_base);
#endif

    /* not found */
    return MAX_BAD;
}

/*
 * Find the tree node that best matches the pattern string.
 * Use the "reported" flag such that only one match
 * is attempted for every node.
 *
 * Warning! This function may recurse.
 *
 * Caller _must_ invoke clear_tree_flags before first call
 * to this function.  This function may be called multiple times
 * to ensure that the entire tree is traversed.
 */

struct tree *
find_best_tree_node(const char *pattrn, struct tree *tree_top, u_int *match)
{
    struct tree *tp, *best_so_far = NULL, *retptr;
    u_int old_match=MAX_BAD, new_match=MAX_BAD;

    if (!pattrn || !*pattrn)
	return(NULL);

    if (!tree_top)
        tree_top = get_tree_head();

    for ( tp = tree_top ; tp ; tp=tp->next_peer ) {
        if (!tp->reported)
            new_match = compute_match(tp->label, pattrn);
        tp->reported = 1;

        if (new_match < old_match) {
            best_so_far = tp;
            old_match = new_match;
        }
        if (new_match == 0)
            break;  /* this is the best result we can get */
        if (tp->child_list) {
            retptr = find_best_tree_node(pattrn, tp->child_list, &new_match);
            if (new_match < old_match) {
                best_so_far = retptr;
                old_match = new_match;
            }
            if (new_match == 0)
                break;  /* this is the best result we can get */
        }
    }

    if (match)
        *match = old_match;
    return(best_so_far);
}


static void
merge_anon_children(struct tree *tp1,
		    struct tree *tp2)
		/* NB: tp1 is the 'anonymous' node */
{
    struct tree *child1, *child2, *previous;

    for ( child1 = tp1->child_list ; child1 ; ) {

        for ( child2 = tp2->child_list, previous = NULL ;
              child2 ; previous = child2, child2 = child2->next_peer ) {

            if ( child1->subid == child2->subid ) {
			/*
			 * Found 'matching' children,
			 *  so merge them
			 */
		if ( !strncmp( child1->label, ANON, ANON_LEN)) {
                    merge_anon_children( child1, child2 );

                    child1->child_list = NULL;
                    previous = child1;		/* Finished with 'child1' */
                    child1 = child1->next_peer;
                    free_tree( previous );
                    goto next;
                }

		else if ( !strncmp( child2->label, ANON, ANON_LEN)) {
                    merge_anon_children( child2, child1 );

                    if ( previous )
                         previous->next_peer = child2->next_peer;
                    else
                         tp2->child_list = child2->next_peer;
                    free_tree(child2);

                    previous = child1;		/* Move 'child1' to 'tp2' */
                    child1 = child1->next_peer;
                    previous->next_peer = tp2->child_list;
                    tp2->child_list = previous;
                    for ( previous = tp2->child_list ;
                          previous ;
                          previous = previous->next_peer )
                                previous->parent = tp2;
                    goto next;
                }
		else if ( !label_compare( child1->label, child2->label) ) {
	            if (ds_get_int(DS_LIBRARY_ID, DS_LIB_MIB_WARNINGS))
		        snmp_log(LOG_WARNING, "Warning: %s.%ld is both %s and %s (%s)\n",
			        tp2->label, child1->subid,
                                child1->label, child2->label, File);
                    continue;
                }
                else {
				/*
				 * Two copies of the same node.
				 * 'child2' adopts the children of 'child1'
				 */

                    if ( child2->child_list ) {
                        for ( previous = child2->child_list ;
                              previous->next_peer ;
                              previous = previous->next_peer )
                                  ;	/* Find the end of the list */
                        previous->next_peer = child1->child_list;
                    }
                    else
                        child2->child_list = child1->child_list;
                    for ( previous = child1->child_list ;
                          previous ;
                          previous = previous->next_peer )
                                  previous->parent = child2;
                    child1->child_list = NULL;

                    previous = child1;		/* Finished with 'child1' */
                    child1 = child1->next_peer;
                    free_tree( previous );
                    goto next;
                }
            }
        }
		/*
		 * If no match, move 'child1' to 'tp2' child_list
		 */
        if ( child1 ) {
            previous = child1;
            child1 = child1->next_peer;
            previous->parent = tp2;
            previous->next_peer = tp2->child_list;
            tp2->child_list = previous;
        }
      next:;
    }
}


/*
 * Find all the children of root in the list of nodes.  Link them into the
 * tree and out of the nodes list.
 */
static void
do_subtree(struct tree *root,
	   struct node **nodes)
{
    register struct tree *tp, *anon_tp=NULL;
    register struct node *np, **headp;
    struct node *oldnp = NULL, *child_list = NULL, *childp = NULL;
    int hash;
    int *int_p;

    tp = root;
    headp = &nbuckets[NBUCKET(name_hash(tp->label))];
    /*
     * Search each of the nodes for one whose parent is root, and
     * move each into a separate list.
     */
    for(np = *headp; np; np = np->next){
        if ( !label_compare(tp->label, np->parent)){
            /* take this node out of the node list */
            if (oldnp == NULL){
                *headp = np->next;  /* fix root of node list */
            } else {
                oldnp->next = np->next; /* link around this node */
            }
            if (child_list) childp->next = np;
            else child_list = np;
            childp = np;
        }
        else {
	    oldnp = np;
        }

    }
    if (childp) childp->next = NULL;
    /*
     * Take each element in the child list and place it into the tree.
     */
    for(np = child_list; np; np = np->next){
	anon_tp = NULL;
        tp = root->child_list;
        while (tp)
            if (tp->subid == np->subid) break;
            else tp = tp->next_peer;
        if (tp) {
	    if (!label_compare (tp->label, np->label)) {
		    /* Update list of modules */
                int_p = (int *) malloc((tp->number_modules+1) * sizeof(int));
                if (int_p == NULL) return;
                memcpy(int_p, tp->module_list, tp->number_modules*sizeof(int));
                int_p[tp->number_modules] = np->modid;
                if (tp->number_modules > 1 )
                   free((char*)tp->module_list);
                ++tp->number_modules;
                tp->module_list = int_p;

		if ( ds_get_boolean(DS_LIBRARY_ID, DS_LIB_MIB_REPLACE) ) {
		    /* Replace from node */
		    tree_from_node(tp,np);
		}
		    /* Handle children */
		do_subtree(tp, nodes);
		continue;
            }
            if (!strncmp( np->label, ANON, ANON_LEN) ||
                !strncmp( tp->label, ANON, ANON_LEN)) {
                anon_tp = tp;	/* Need to merge these two trees later */
            }
	    else if (ds_get_int(DS_LIBRARY_ID, DS_LIB_MIB_WARNINGS))
		snmp_log(LOG_WARNING, "Warning: %s.%ld is both %s and %s (%s)\n",
			root->label, np->subid, tp->label, np->label, File);
	}

        tp = (struct tree *) calloc(1, sizeof(struct tree));
        if (tp == NULL) return;
        tp->parent = root;
        tp->modid = np->modid;
        tp->number_modules = 1;
        tp->module_list = &(tp->modid);
        tree_from_node(tp, np);
        tp->next_peer = root->child_list;
        root->child_list = tp;
        hash = NBUCKET(name_hash(tp->label));
        tp->next = tbuckets[hash];
        tbuckets[hash] = tp;
/*      if (tp->type == TYPE_OTHER) */
            do_subtree(tp, nodes);      /* recurse on this child if it isn't
                                           an end node */
        if ( anon_tp ) {
            if (!strncmp( tp->label, ANON, ANON_LEN)) {
			/*
			 * The new node is anonymous,
			 *  so merge it with the existing one.
			 */
                merge_anon_children( tp, anon_tp );

		/* unlink and destroy tp */
		unlink_tree(tp);
		free_tree(tp);
            }
            else if (!strncmp( anon_tp->label, ANON, ANON_LEN)) {
		struct tree *ntp;
			/*
			 * The old node was anonymous,
			 *  so merge it with the existing one,
			 *  and fill in the full information.
			 */
                merge_anon_children( anon_tp, tp );

		/* unlink anon_tp from the hash */
		unlink_tbucket(anon_tp);

		/* get rid of old contents of anon_tp */
                free_partial_tree(anon_tp, FALSE);

		/* put in the current information */
                anon_tp->label = tp->label;
                anon_tp->child_list = tp->child_list;
                anon_tp->modid = tp->modid;
                anon_tp->tc_index = tp->tc_index;
                anon_tp->type = tp->type;
                anon_tp->enums = tp->enums;
                anon_tp->indexes = tp->indexes;
                anon_tp->ranges = tp->ranges;
                anon_tp->hint = tp->hint;
                anon_tp->units = tp->units;
                anon_tp->description = tp->description;
		anon_tp->parent = tp->parent;
                set_function(anon_tp);

		/* update parent pointer in moved children */
		ntp = anon_tp->child_list;
		while (ntp) {
		    ntp->parent = anon_tp;
		    ntp = ntp->next_peer;
		}

		/* hash in anon_tp in its new place */
		hash = NBUCKET(name_hash(anon_tp->label));
		anon_tp->next = tbuckets[hash];
		tbuckets[hash] = anon_tp;

		/* unlink and destroy tp */
		unlink_tbucket(tp);
		unlink_tree(tp);
		free(tp);
            }
            else {
                /* Uh?  One of these two should have been anonymous! */
	        if (ds_get_int(DS_LIBRARY_ID, DS_LIB_MIB_WARNINGS))
		    snmp_log(LOG_WARNING,
                             "Warning: expected anonymous node (either %s or %s) in %s\n",
                             tp->label, anon_tp->label, File);
            }
            anon_tp = NULL;
        }
    }
    /* free all nodes that were copied into tree */
    oldnp = NULL;
    for(np = child_list; np; np = np->next){
        if (oldnp)
            free_node(oldnp);
        oldnp = np;
    }
    if (oldnp)
        free_node(oldnp);
}

static void do_linkup(struct module *mp,
		      struct node *np)
{
    struct module_import *mip;
    struct node *onp;
    struct tree *tp;
    int i;
	/*
	 * All modules implicitly import
	 *   the roots of the tree
	 */
    if (snmp_get_do_debugging() > 1) dump_module_list();
    DEBUGMSGTL(("parse-mibs", "Processing IMPORTS for module %d %s\n", mp->modid, mp->name));
    if ( mp->no_imports == 0 ) {
	mp->no_imports = NUMBER_OF_ROOT_NODES;
	mp->imports = root_imports;
    }

	/*
	 * Build the tree
	 */
    init_node_hash( np );
    for ( i=0, mip=mp->imports ; i < mp->no_imports ; ++i, ++mip ) {
	char modbuf[256];
	DEBUGMSGTL(("parse-mibs", "  Processing import: %s\n", mip->label));
	if (get_tc_index( mip->label, mip->modid ) != -1)
	    continue;
	tp = find_tree_node( mip->label, mip->modid );
	if (!tp) {
	    if (mip->modid != -1)
		snmp_log(LOG_WARNING, "Did not find '%s' in module %s (%s)\n",
                         mip->label, module_name(mip->modid, modbuf), File);
	    continue;
	}
	do_subtree( tp, &np );
    }

	/*
	 * If any nodes left over,
	 *   check that they're not the result of a "fully qualified"
	 *   name, and then add them to the list of orphans
	 */

    if (!np) return;
    for ( tp = tree_head ; tp ; tp=tp->next_peer )
        do_subtree( tp, &np );
    if (!np) return;
    for ( np = orphan_nodes ; np && np->next ; np = np->next )
	;	/* find the end of the orphan list */
    for (i = 0; i < NHASHSIZE; i++)
	if ( nbuckets[i] ) {
	    if ( orphan_nodes )
		onp = np->next = nbuckets[i];
	    else
		onp = orphan_nodes = nbuckets[i];
	    nbuckets[i] = NULL;
	    while (onp) {
		if (ds_get_int(DS_LIBRARY_ID, DS_LIB_MIB_WARNINGS))
		    snmp_log(LOG_WARNING,
                             "Unlinked OID in %s: %s ::= { %s %ld }\n",
                             (mp->name ? mp->name : "<no module>"),
                             (onp->label ? onp->label : "<no label>"),
                             (onp->parent ? onp->parent : "<no parent>"),
                             onp->subid);
		np = onp;
		onp = onp->next;
	    }
	}

    return;
}


/*
 * Takes a list of the form:
 * { iso org(3) dod(6) 1 }
 * and creates several nodes, one for each parent-child pair.
 * Returns 0 on error.
 */
static int
getoid(FILE *fp,
       struct subid_s *id, /* an array of subids */
       int length)  /* the length of the array */
{
    register int count;
    int type;
    char token[MAXTOKEN];

    if ((type = get_token(fp, token, MAXTOKEN)) != LEFTBRACKET){
        print_error("Expected \"{\"", token, type);
        return 0;
    }
    type = get_token(fp, token, MAXTOKEN);
    for(count = 0; count < length; count++, id++){
        id->label = NULL;
        id->modid = current_module;
        id->subid = -1;
        if (type == RIGHTBRACKET){
            return count;
        } else if (type != LABEL && type != NUMBER){
            print_error("Not valid for object identifier", token, type);
            return 0;
        }
        if (type == LABEL){
            /* this entry has a label */
            id->label = strdup(token);
            type = get_token(fp, token, MAXTOKEN);
            if (type == LEFTPAREN){
                type = get_token(fp, token, MAXTOKEN);
                if (type == NUMBER){
                    id->subid = atoi(token);
                    if ((type = get_token(fp, token, MAXTOKEN)) != RIGHTPAREN){
                        print_error("Expected a closing parenthesis",
                                    token, type);
                        return 0;
                    }
                } else {
                    print_error("Expected a number", token, type);
                    return 0;
                }
            } else {
                continue;
            }
        } else if (type == NUMBER) {
            /* this entry  has just an integer sub-identifier */
            id->subid = atoi(token);
        }
	else {
	    print_error("Expected label or number", token, type);
	    return 0;
	}
        type = get_token(fp, token, MAXTOKEN);
    }
    print_error ("Too long OID", token, type);
    return 0;
}

/*
 * Parse a sequence of object subidentifiers for the given name.
 * The "label OBJECT IDENTIFIER ::=" portion has already been parsed.
 *
 * The majority of cases take this form :
 * label OBJECT IDENTIFIER ::= { parent 2 }
 * where a parent label and a child subidentifier number are specified.
 *
 * Variations on the theme include cases where a number appears with
 * the parent, or intermediate subidentifiers are specified by label,
 * by number, or both.
 *
 * Here are some representative samples :
 * internet        OBJECT IDENTIFIER ::= { iso org(3) dod(6) 1 }
 * mgmt            OBJECT IDENTIFIER ::= { internet 2 }
 * rptrInfoHealth  OBJECT IDENTIFIER ::= { snmpDot3RptrMgt 0 4 }
 *
 * Here is a very rare form :
 * iso             OBJECT IDENTIFIER ::= { 1 }
 *
 * Returns NULL on error.  When this happens, memory may be leaked.
 */
static struct node *
parse_objectid(FILE *fp,
	       char *name)
{
    register int count;
    register struct subid_s *op, *nop;
    int length;
    struct subid_s loid[32];
    struct node *np, *root = NULL, *oldnp = NULL;
    struct tree *tp;

    if ((length = getoid(fp, loid, 32)) == 0){
        print_error("Bad object identifier", NULL, CONTINUE);
        return NULL;
    }

    /*
     * Handle numeric-only object identifiers,
     *  by labelling the first sub-identifier
     */
    op = loid;
    if ( !op->label )
      for ( tp = tree_head ; tp ; tp=tp->next_peer )
        if ( (int)tp->subid == op->subid ) {
            op->label = strdup(tp->label);
            break;
        }

    /*
     * Handle  "label OBJECT-IDENTIFIER ::= { subid }"
     */
    if (length == 1) {
        op = loid;
        np = alloc_node(op->modid);
        if (np == NULL) return(NULL);
        np->subid = op->subid;
        np->label = strdup(name);
        if (op->label) free(op->label);
        return np;
    }

    /*
     * For each parent-child subid pair in the subid array,
     * create a node and link it into the node list.
     */
    for(count = 0, op = loid, nop=loid+1; count < (length - 1);
      count++, op++, nop++){
        /* every node must have parent's name and child's name or number */
/* XX the next statement is always true -- does it matter ?? */
        if (op->label && (nop->label || (nop->subid != -1))){
            np = alloc_node(nop->modid);
            if (np == NULL) return(NULL);
            if (root == NULL) root = np;

            np->parent = strdup (op->label);
            if (count == (length - 2)) {
                /* The name for this node is the label for this entry */
                np->label = strdup (name);
            }
            else {
                if (!nop->label) {
                    nop->label = (char *) malloc(20 + ANON_LEN);
                    if (nop->label == NULL) return(NULL);
                    sprintf(nop->label, "%s%d", ANON, anonymous++);
                }
                np->label = strdup (nop->label);
            }
            if (nop->subid != -1)
                np->subid = nop->subid;
            else
                print_error("Warning: This entry is pretty silly",
			    np->label, CONTINUE);

            /* set up next entry */
            if (oldnp) oldnp->next = np;
            oldnp = np;
        } /* end if(op->label... */
    }

    /* free the loid array */
    for(count = 0, op = loid; count < length; count++, op++){
        if (op->label)
            free(op->label);
    }

    return root;
}

static int
get_tc(const char *descriptor,
       int modid,
       int *tc_index,
       struct enum_list **ep,
       struct range_list **rp,
       char **hint)
{
    int i;
    struct tc *tcp;

    i = get_tc_index(descriptor, modid);
    if (tc_index) *tc_index = i;
    if (i != -1)
      {
 	tcp = &tclist[i];
	if (ep) {
	    free_enums(ep);
	    *ep = copy_enums(tcp->enums);
	}
	if (rp) {
	    free_ranges(rp);
	    *rp = copy_ranges(tcp->ranges);
	}
	if (hint) {
	    if (*hint) free(*hint);
	    *hint = (tcp->hint ? strdup(tcp->hint) : NULL);
	}
	return tcp->type;
      }
    return LABEL;
}

/* return index into tclist of given TC descriptor
   return -1 if not found
 */
static int
get_tc_index(const char *descriptor,
	     int modid)
{
    int i;
    struct tc *tcp;
    struct module *mp;
    struct module_import *mip;

	/*
	 * Check that the descriptor isn't imported
	 *  by searching the import list
	 */

    for ( mp = module_head ; mp ; mp = mp->next )
         if ( mp->modid == modid )
             break;
    if ( mp )
         for ( i=0, mip=mp->imports ; i < mp->no_imports ; ++i, ++mip ) {
             if ( !label_compare( mip->label, descriptor )) {
				/* Found it - so amend the module ID */
                  modid = mip->modid;
                  break;
             }
         }


    for(i=0, tcp=tclist; i < MAXTC; i++, tcp++){
      if (tcp->type == 0)
          break;
      if (!label_compare(descriptor, tcp->descriptor) &&
		((modid == tcp->modid) || (modid == -1))){
          return i;
      }
    }
    return -1;
}

/* translate integer tc_index to string identifier from tclist
 *
 * Returns pointer to string in table (should not be modified) or NULL
 */
const char *
get_tc_descriptor(int tc_index)
{
  if (tc_index < 0 || tc_index >= MAXTC) return NULL;
  return (tclist[tc_index].descriptor);
}


/*
 * Parses an enumeration list of the form:
 *        { label(value) label(value) ... }
 * The initial { has already been parsed.
 * Returns NULL on error.
 */

static struct enum_list *
parse_enumlist(FILE *fp, struct enum_list **retp)
{
    register int type;
    char token [MAXTOKEN];
    struct enum_list *ep = NULL, **epp = &ep;

    free_enums(retp);

    while((type = get_token(fp, token, MAXTOKEN)) != ENDOFFILE){
        if (type == RIGHTBRACKET)
            break;
        if (type == LABEL){
            /* this is an enumerated label */
            *epp = (struct enum_list *) calloc(1, sizeof(struct enum_list));
            if (*epp == NULL) return(NULL);
            /* a reasonable approximation for the length */
            (*epp)->label = strdup(token);
            type = get_token(fp, token, MAXTOKEN);
            if (type != LEFTPAREN) {
                print_error("Expected \"(\"", token, type);
                return NULL;
            }
            type = get_token(fp, token, MAXTOKEN);
            if (type != NUMBER) {
                print_error("Expected integer", token, type);
                return NULL;
            }
            (*epp)->value = atoi(token);
            type = get_token(fp, token, MAXTOKEN);
            if (type != RIGHTPAREN) {
                print_error("Expected \")\"", token, type);
                return NULL;
            }
            epp = &(*epp)->next;
        }
    }
    if (type == ENDOFFILE){
        print_error("Expected \"}\"", token, type);
        return NULL;
    }
    *retp = ep;
    return ep;
}

static struct range_list *parse_ranges(FILE *fp, struct range_list **retp)
{   int low, high;
    char nexttoken[MAXTOKEN];
    int nexttype;
    struct range_list *rp = NULL, **rpp = &rp;
    int size = 0, taken = 1;

    free_ranges(retp);

    nexttype = get_token(fp, nexttoken, MAXTOKEN);
    if (nexttype == SIZE) {
	size = 1;
	taken = 0;
	nexttype = get_token(fp, nexttoken, MAXTOKEN);
	if (nexttype != LEFTPAREN)
	    print_error("Expected \"(\" after SIZE", nexttoken, nexttype);
    }

    do {
	if (!taken) nexttype = get_token(fp, nexttoken, MAXTOKEN);
	else taken = 0;
	high = low = atol(nexttoken);
	nexttype = get_token(fp, nexttoken, MAXTOKEN);
	if (nexttype == RANGE) {
	    nexttype = get_token(fp, nexttoken, MAXTOKEN);
	    high = atol(nexttoken);
	    nexttype = get_token(fp, nexttoken, MAXTOKEN);
	}
	*rpp = (struct range_list *)calloc (1, sizeof(struct range_list));
	if (*rpp == NULL) break;
	(*rpp)->low = low;
	(*rpp)->high = high;
	rpp = &(*rpp)->next;

    } while (nexttype == BAR);
    if (size) {
	if (nexttype != RIGHTPAREN)
	    print_error ("Expected \")\" after SIZE", nexttoken, nexttype);
	nexttype = get_token(fp, nexttoken, nexttype);
    }
    if (nexttype != RIGHTPAREN)
	print_error ("Expected \")\"", nexttoken, nexttype);

    *retp = rp;
    return rp;
}

/*
 * Parses an asn type.  Structures are ignored by this parser.
 * Returns NULL on error.
 */
static struct node *
parse_asntype(FILE *fp,
	      char *name,
	      int *ntype,
	      char *ntoken)
{
    int type, i;
    char token[MAXTOKEN];
    char quoted_string_buffer[MAXQUOTESTR];
    char *hint = NULL;
    struct tc *tcp;
    int level;

    type = get_token(fp, token, MAXTOKEN);
    if (type == SEQUENCE){
        level = 0;
        while((type = get_token(fp, token, MAXTOKEN)) != ENDOFFILE){
            if (type == LEFTBRACKET){
                level++;
            }
            else if (type == RIGHTBRACKET && --level == 0){
                *ntype = get_token(fp, ntoken, MAXTOKEN);
                return NULL;
            }
        }
        print_error("Expected \"}\"", token, type);
        return NULL;
    } else if (type == LEFTBRACKET) {
        struct node *np;
        int ch_next = '{';
        ungetc(ch_next, fp);
        np = parse_objectid (fp, name);
        if (np != NULL) {
            *ntype = get_token(fp, ntoken, MAXTOKEN);
            return np;
        }
        return NULL;
    } else {
        if (type == CONVENTION) {
            while (type != SYNTAX && type != ENDOFFILE) {
                if (type == DISPLAYHINT) {
                    type = get_token(fp, token, MAXTOKEN);
                    if (type != QUOTESTRING) print_error("DISPLAY-HINT must be string", token, type);
                    else hint = strdup (token);
                }
                else
		    type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
            }
            type = get_token(fp, token, MAXTOKEN);
        }

        if (type == LABEL)
        {
            type = get_tc(token, current_module, NULL, NULL, NULL, NULL);
        }

        /* textual convention */
        for(i = 0; i < MAXTC; i++){
            if (tclist[i].type == 0)
                break;
        }

        if (i == MAXTC){
            print_error("Too many textual conventions", token, type);
            SNMP_FREE(hint);
            return NULL;
        }
        if (!(type & SYNTAX_MASK)){
            print_error("Textual convention doesn't map to real type", token,
                        type);
            SNMP_FREE(hint);
            return NULL;
        }
        tcp = &tclist[i];
        tcp->modid = current_module;
        tcp->descriptor = strdup(name);
        tcp->hint = hint;
        tcp->type = type;
        *ntype = get_token(fp, ntoken, MAXTOKEN);
        if (*ntype == LEFTPAREN){
	    tcp->ranges = parse_ranges(fp, &tcp->ranges);
            *ntype = get_token(fp, ntoken, MAXTOKEN);
        } else if (*ntype == LEFTBRACKET) {
            /* if there is an enumeration list, parse it */
            tcp->enums = parse_enumlist(fp, &tcp->enums);
            *ntype = get_token(fp, ntoken, MAXTOKEN);
        }
        return NULL;
    }
}


/*
 * Parses an OBJECT TYPE macro.
 * Returns 0 on error.
 */
static struct node *
parse_objecttype(FILE *fp,
		 char *name)
{
    register int type;
    char token[MAXTOKEN];
    char nexttoken[MAXTOKEN];
    char quoted_string_buffer[MAXQUOTESTR];
    int nexttype, tctype;
    register struct node *np;

    type = get_token(fp, token, MAXTOKEN);
    if (type != SYNTAX){
        print_error("Bad format for OBJECT-TYPE", token, type);
        return NULL;
    }
    np = alloc_node(current_module);
    if (np == NULL) return(NULL);
    type = get_token(fp, token, MAXTOKEN);
    if (type == LABEL){
        int tmp_index;
        tctype = get_tc(token, current_module, &tmp_index,
                        &np->enums, &np->ranges, &np->hint);
        if (tctype == LABEL &&
            ds_get_int(DS_LIBRARY_ID, DS_LIB_MIB_WARNINGS) > 1){
            print_error("Warning: No known translation for type", token, type);
        }
        type = tctype;
        np->tc_index = tmp_index; /* store TC for later reference */
    }
    np->type = type;
    nexttype = get_token(fp, nexttoken, MAXTOKEN);
    switch(type){
        case SEQUENCE:
            if (nexttype == OF){
                nexttype = get_token(fp, nexttoken, MAXTOKEN);
                nexttype = get_token(fp, nexttoken, MAXTOKEN);
            }
            break;
        case INTEGER:
        case UINTEGER32:
        case COUNTER:
        case GAUGE:
        case BITSTRING:
        case LABEL:
            if (nexttype == LEFTBRACKET) {
                /* if there is an enumeration list, parse it */
                np->enums = parse_enumlist(fp, &np->enums);
                nexttype = get_token(fp, nexttoken, MAXTOKEN);
            } else if (nexttype == LEFTPAREN){
                /* if there is a range list, parse it */
		np->ranges = parse_ranges(fp, &np->ranges);
                nexttype = get_token(fp, nexttoken, MAXTOKEN);
            }
            break;
        case OCTETSTR:
        case KW_OPAQUE:
            /* parse any SIZE specification */
            if (nexttype == LEFTPAREN) {
                nexttype = get_token(fp, nexttoken, MAXTOKEN);
                if (nexttype == SIZE) {
                    nexttype = get_token(fp, nexttoken, MAXTOKEN);
                    if (nexttype == LEFTPAREN) {
			np->ranges = parse_ranges(fp, &np->ranges);
                        nexttype = get_token(fp, nexttoken, MAXTOKEN); /* ) */
                        if (nexttype == RIGHTPAREN)
                        {
                            nexttype = get_token(fp, nexttoken, MAXTOKEN);
                            break;
                        }
                    }
                }
                print_error("Bad SIZE syntax", token, type);
                free_node(np);
                return NULL;
            }
            break;
        case OBJID:
        case NETADDR:
        case IPADDR:
        case TIMETICKS:
        case NUL:
        case NSAPADDRESS:
        case COUNTER64:
            break;
        default:
            print_error("Bad syntax", token, type);
            free_node(np);
            return NULL;
    }
    if (nexttype == UNITS){
        type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
        if (type != QUOTESTRING) {
            print_error("Bad UNITS", quoted_string_buffer, type);
            free_node(np);
            return NULL;
        }
	np->units = strdup (quoted_string_buffer);
        nexttype = get_token(fp, nexttoken, MAXTOKEN);
    }
    if (nexttype != ACCESS){
        print_error("Should be ACCESS", nexttoken, nexttype);
        free_node(np);
        return NULL;
    }
    type = get_token(fp, token, MAXTOKEN);
    if (type != READONLY && type != READWRITE && type != WRITEONLY
        && type != NOACCESS && type != READCREATE && type != ACCNOTIFY){
        print_error("Bad ACCESS type", token, type);
        free_node(np);
        return NULL;
    }
    np->access = type;
    type = get_token(fp, token, MAXTOKEN);
    if (type != STATUS){
        print_error("Should be STATUS", token, type);
        free_node(np);
        return NULL;
    }
    type = get_token(fp, token, MAXTOKEN);
    if (type != MANDATORY && type != CURRENT && type != KW_OPTIONAL &&
        type != OBSOLETE && type != DEPRECATED){
        print_error("Bad STATUS", token, type);
        free_node(np);
        return NULL;
    }
    np->status = type;
    /*
     * Optional parts of the OBJECT-TYPE macro
     */
    type = get_token(fp, token, MAXTOKEN);
    while (type != EQUALS && type != ENDOFFILE) {
      switch (type) {
        case DESCRIPTION:
          type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
          if (type != QUOTESTRING) {
              print_error("Bad DESCRIPTION", quoted_string_buffer, type);
              free_node(np);
              return NULL;
          }
          if (ds_get_boolean(DS_LIBRARY_ID, DS_LIB_SAVE_MIB_DESCRS)) {
              np->description = strdup (quoted_string_buffer);
          }
          break;

        case REFERENCE:
          type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
          if (type != QUOTESTRING) {
              print_error("Bad REFERENCE", quoted_string_buffer, type);
              free_node(np);
              return NULL;
          }
          break;
        case INDEX:
          np->indexes = getIndexes(fp, &np->indexes);
          if (np->indexes == NULL) {
            print_error("Bad Index List",token,type);
            free_node(np);
            return NULL;
          }
          break;

        case DEFVAL:
        case AUGMENTS:
        case NUM_ENTRIES:
          if (tossObjectIdentifier(fp) != OBJID) {
              print_error("Bad Object Identifier", token, type);
              free_node(np);
              return NULL;
          }
          break;

        default:
          print_error("Bad format of optional clauses", token, type);
          free_node(np);
          return NULL;

      }
      type = get_token(fp, token, MAXTOKEN);
    }
    if (type != EQUALS){
        print_error("Bad format", token, type);
        free_node(np);
        return NULL;
    }
    return merge_parse_objectid(np, fp, name);
}

/*
 * Parses an OBJECT GROUP macro.
 * Returns 0 on error.
 *
 * Also parses object-identity, since they are similar (ignore STATUS).
 *   - WJH 10/96
 */
static struct node *
parse_objectgroup(FILE *fp,
		  char *name)
{
    register int type;
    char token[MAXTOKEN];
    char quoted_string_buffer[MAXQUOTESTR];
    register struct node *np;

    np = alloc_node(current_module);
    if (np == NULL) return(NULL);
    type = get_token(fp, token, MAXTOKEN);
    while (type != EQUALS && type != ENDOFFILE) {
      switch (type) {
        case DESCRIPTION:
          type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
          if (type != QUOTESTRING) {
              print_error("Bad DESCRIPTION", quoted_string_buffer, type);
              free_node(np);
              return NULL;
          }
          if (ds_get_boolean(DS_LIBRARY_ID, DS_LIB_SAVE_MIB_DESCRS)) {
              np->description = strdup (quoted_string_buffer);
          }
          break;

	case REFERENCE:
	  type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
          if (type != QUOTESTRING) {
              print_error("Bad REFERENCE", quoted_string_buffer, type);
              free_node(np);
              return NULL;
          }
	  break;

        default:
          /* NOTHING */
          break;
      }
      type = get_token(fp, token, MAXTOKEN);
    }
    return merge_parse_objectid(np, fp, name);
}

/*
 * Parses a NOTIFICATION-TYPE macro.
 * Returns 0 on error.
 */
static struct node *
parse_notificationDefinition(FILE *fp,
			     char *name)
{
    register int type;
    char token[MAXTOKEN];
    char quoted_string_buffer[MAXQUOTESTR];
    register struct node *np;

    np = alloc_node(current_module);
    if (np == NULL) return(NULL);
    type = get_token(fp, token, MAXTOKEN);
    while (type != EQUALS && type != ENDOFFILE) {
      switch (type) {
        case DESCRIPTION:
          type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
          if (type != QUOTESTRING) {
              print_error("Bad DESCRIPTION", quoted_string_buffer, type);
              free_node(np);
              return NULL;
          }
          if (ds_get_boolean(DS_LIBRARY_ID, DS_LIB_SAVE_MIB_DESCRS)) {
              np->description = strdup (quoted_string_buffer);
          }
          break;

        default:
          /* NOTHING */
          break;
      }
      type = get_token(fp, token, MAXTOKEN);
    }
    return merge_parse_objectid(np, fp, name);
}

/*
 * Parses a TRAP-TYPE macro.
 * Returns 0 on error.
 */
static struct node *
parse_trapDefinition(FILE *fp,
		     char *name)
{
    register int type;
    char token[MAXTOKEN];
    char quoted_string_buffer[MAXQUOTESTR];
    register struct node *np;

    np = alloc_node(current_module);
    if (np == NULL) return(NULL);
    type = get_token(fp, token, MAXTOKEN);
    while (type != EQUALS && type != ENDOFFILE) {
        switch (type) {
            case DESCRIPTION:
                type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
                if (type != QUOTESTRING) {
                    print_error("Bad DESCRIPTION", quoted_string_buffer, type);
                    free_node(np);
                    return NULL;
                }
                if (ds_get_boolean(DS_LIBRARY_ID, DS_LIB_SAVE_MIB_DESCRS)) {
                    np->description = strdup (quoted_string_buffer);
                }
                break;
            case ENTERPRISE:
                type = get_token(fp, token, MAXTOKEN);
                if (type == LEFTBRACKET) {
                    type = get_token(fp, token, MAXTOKEN);
                    if (type != LABEL) {
                        print_error("Bad Trap Format", token, type);
                        free_node(np);
                        return NULL;
                    }
                    np->parent = strdup(token);
                    /* Get right bracket */
                    type = get_token(fp, token, MAXTOKEN);
                }
                else if (type == LABEL)
                    np->parent = strdup(token);
                break;
            default:
                /* NOTHING */
                break;
        }
        type = get_token(fp, token, MAXTOKEN);
    }
    type = get_token(fp, token, MAXTOKEN);

    np->label = strdup(name);

    if (type != NUMBER) {
        print_error("Expected a Number", token, type);
        free_node(np);
        return NULL;
    }
    np->subid = atoi(token);
    np->next = alloc_node(current_module);
    if (np->next == NULL)  {
        free_node(np);
        return(NULL);
    }
    np->next->parent = np->parent;
    np->parent = (char *)malloc(strlen(np->parent)+2);
    if (np->parent == NULL) {
        free_node(np->next); free_node(np);
        return(NULL);
    }
    strcpy(np->parent, np->next->parent);
    strcat(np->parent, "#");
    np->next->label = strdup(np->parent);
    return np;
}


/*
 * Parses a compliance macro
 * Returns 0 on error.
 */
static struct node *
parse_compliance(FILE *fp,
		 char *name)
{
    register int type;
    char token[MAXTOKEN];
    char quoted_string_buffer[MAXQUOTESTR];
    register struct node *np;

    np = alloc_node(current_module);
    if (np == NULL) return(NULL);
    type = get_token(fp, token, MAXTOKEN);
    while (type != EQUALS && type != ENDOFFILE) {
        type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
    }
    return merge_parse_objectid(np, fp, name);
}


/*
 * Parses a capabilities macro
 * Returns 0 on error.
 */
static struct node *
parse_capabilities(FILE *fp,
		   char *name)
{
    register int type;
    char token[MAXTOKEN];
    char quoted_string_buffer[MAXQUOTESTR];
    register struct node *np;

    np = alloc_node(current_module);
    if (np == NULL) return(NULL);
    type = get_token(fp, token, MAXTOKEN);
    while (type != EQUALS && type != ENDOFFILE) {
        type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
    }
    return merge_parse_objectid(np, fp, name);
}

/*
 * Parses a module identity macro
 * Returns 0 on error.
 */
static struct node *
parse_moduleIdentity(FILE *fp,
		     char *name)
{
    register int type;
    char token[MAXTOKEN];
    char quoted_string_buffer[MAXQUOTESTR];
    register struct node *np;

    np = alloc_node(current_module);
    if (np == NULL) return(NULL);
    type = get_token(fp, token, MAXTOKEN);
    while (type != EQUALS && type != ENDOFFILE) {
        type = get_token(fp, quoted_string_buffer, MAXQUOTESTR);
    }
    return merge_parse_objectid(np, fp, name);
}


/*
 * Parses a MACRO definition
 * Expect BEGIN, discard everything to end.
 * Returns 0 on error.
 */
static struct node *
parse_macro(FILE *fp,
	    char *name)
{
    register int type;
    char token[MAXTOKEN];
    struct node *np;
    int iLine = Line;

    np = alloc_node(current_module);
    if (np == NULL) return(NULL);
    type = get_token(fp, token, sizeof(token));
    while (type != EQUALS && type != ENDOFFILE) {
        type = get_token(fp, token, sizeof(token));
    }
    if (type != EQUALS) return NULL;
    while (type != BEGIN && type != ENDOFFILE) {
        type = get_token(fp, token, sizeof(token));
    }
    if (type != BEGIN) return NULL;
    while (type != END && type != ENDOFFILE) {
        type = get_token(fp, token, sizeof(token));
    }
    if (type != END) return NULL;

    if (ds_get_int(DS_LIBRARY_ID, DS_LIB_MIB_WARNINGS))
	snmp_log(LOG_WARNING,
		 "%s MACRO (lines %d..%d parsed and ignored).\n", name, iLine, Line);

    return np;
}

/*
 * Parses a module import clause
 *   loading any modules referenced
 */
static void
parse_imports(FILE *fp)
{
    register int type;
    char token[MAXTOKEN];
    char modbuf[256];
#define MAX_IMPORTS	256
    struct module_import import_list[MAX_IMPORTS];
    int this_module, old_current_module;
    const char *old_File;
    int old_line;
    struct module *mp;

    int import_count=0;		/* Total number of imported descriptors */
    int i=0, old_i;		/* index of first import from each module */

    type = get_token(fp, token, MAXTOKEN);

		/*
		 * Parse the IMPORTS clause
		 */
    while (type != SEMI && type != ENDOFFILE) {
	if (type == LABEL ) {
	    if (import_count == MAX_IMPORTS ) {
		print_error("Too many imported symbols", token, type);
		do {
		    type = get_token(fp, token, MAXTOKEN);
		} while (type != SEMI && type != ENDOFFILE);
		return;
	    }
	    import_list[import_count++].label = strdup(token);
	}
	else if ( type == FROM ) {
	    type = get_token(fp, token, MAXTOKEN);
            if ( import_count == i ) {	/* All imports are handled internally */
	       type = get_token(fp, token, MAXTOKEN);
               continue;
            }
	    this_module = which_module(token);

	    for ( old_i=i ; i<import_count ; ++i)
		import_list[i].modid = this_module;

	    old_current_module = current_module;	/* Save state */
            old_File = File;
	    old_line = Line;
	    current_module = this_module;

		/*
		 * Recursively read any pre-requisite modules
		 */
	    if  (read_module_internal(token) == MODULE_NOT_FOUND ) {
		for ( ; old_i<import_count ; ++old_i ) {
		    read_import_replacements( token, &import_list[old_i]);
		}
	    }

	    current_module = old_current_module;	/* Restore state */
	    File = old_File;
	    Line = old_line;
	}
	type = get_token(fp, token, MAXTOKEN);
    }

		/*
		 * Save the import information
		 *   in the global module table
		 */
    for ( mp=module_head ; mp ; mp=mp->next )
	if ( mp->modid == current_module) {
            if ( import_count == 0)
		return;
            if (mp->imports && (mp->imports != root_imports))
	    {
		/* this can happen if all modules are in one source file. */
		for ( i=0 ; i<mp->no_imports; ++i ) {
	DEBUGMSGTL(("parse-mibs",  "#### freeing Module %d '%s' %d\n",
	mp->modid, mp->imports[i].label, mp->imports[i].modid));
		    free((char *)mp->imports[i].label);
		}
		free((char*)mp->imports);
	    }
            mp->imports = (struct module_import *)
              calloc(import_count, sizeof(struct module_import));
            if (mp->imports == NULL) return;
	    for ( i=0 ; i<import_count ; ++i ) {
		mp->imports[i].label = import_list[i].label;
		mp->imports[i].modid = import_list[i].modid;
	DEBUGMSGTL(("parse-mibs",  "#### adding Module %d '%s' %d\n",
	mp->modid, mp->imports[i].label, mp->imports[i].modid));
	    }
	    mp->no_imports = import_count;
	    return;
	}

	/*
	 * Shouldn't get this far
	 */
    print_module_not_found(module_name(current_module,modbuf));
    return;
}



/*
 * MIB module handling routines
 */

static void dump_module_list (void)
{
    struct module *mp = module_head;

    DEBUGMSGTL(("parse-mibs", "Module list:\n"));
    while (mp) {
	DEBUGMSGTL(("parse-mibs", "  %s %d %s %d\n", mp->name, mp->modid, mp->file, mp->no_imports));
	mp = mp->next;
    }
}

int
which_module(const char *name)
{
    struct module *mp;

    for ( mp=module_head ; mp ; mp=mp->next )
	if ( !label_compare(mp->name, name))
	    return(mp->modid);

    DEBUGMSGTL(("parse-mibs", "Module %s not found\n", name));
    return(-1);
}

/*
 * module_name - copy module name to user buffer, return ptr to same.
 */
char *
module_name (int modid,
	     char *cp)
{
    struct module *mp;

    for ( mp=module_head ; mp ; mp=mp->next )
	if ( mp->modid == modid )
	{
	    strcpy(cp, mp->name);
	    return(cp);
	}

    DEBUGMSGTL(("parse-mibs", "Module %d not found\n", modid));
    sprintf(cp, "#%d", modid);
    return(cp);
}

/*
 *  Backwards compatability
 *  Read newer modules that replace the one specified:-
 *	either all of them (read_module_replacements),
 *	or those relating to a specified identifier (read_import_replacements)
 *	plus an interface to add new replacement requirements
 */
void
add_module_replacement(const char *old_module,
		       const char *new_module_name,
		       const char *tag,
		       int len)
{
    struct module_compatability *mcp;

    mcp =  (struct module_compatability *)
      calloc(1, sizeof( struct module_compatability));
    if (mcp == NULL) return;

    mcp->old_module = strdup( old_module );
    mcp->new_module = strdup( new_module_name );
	if (tag)
    mcp->tag	    = strdup( tag );
    mcp->tag_len = len;

    mcp->next    = module_map_head;
    module_map_head = mcp;
}

static void
read_module_replacements(const char *name)
{
    struct module_compatability *mcp;

    for ( mcp=module_map_head ; mcp; mcp=mcp->next ) {
	if ( !label_compare( mcp->old_module, name )) {
	    if (ds_get_int(DS_LIBRARY_ID, DS_LIB_MIB_WARNINGS))
		snmp_log(LOG_WARNING,
                         "Loading replacement module %s for %s (%s)\n",
                         mcp->new_module, name, File);
	    (void)read_module( mcp->new_module );
	    return;
	}
    }
    if (!ds_get_boolean(DS_LIBRARY_ID, DS_LIB_MIB_ERRORS))
	print_module_not_found(name);

}

static void
read_import_replacements(const char *old_module_name,
			 struct module_import *identifier)
{
    struct module_compatability *mcp;

	/*
	 * Look for matches first
	 */
    for ( mcp=module_map_head ; mcp; mcp=mcp->next ) {
      if ( !label_compare( mcp->old_module, old_module_name )) {

	if (	/* exact match */
	  	  ( mcp->tag_len==0 &&
		    (mcp->tag == NULL ||
                     !label_compare( mcp->tag, identifier->label ))) ||
		/* prefix match */
	          ( mcp->tag_len!=0 &&
		    !strncmp( mcp->tag, identifier->label, mcp->tag_len ))
	   ) {

	    if (ds_get_int(DS_LIBRARY_ID, DS_LIB_MIB_WARNINGS))
	        snmp_log(LOG_WARNING,
                         "Importing %s from replacement module %s instead of %s (%s)\n",
                         identifier->label, mcp->new_module, old_module_name, File);
	    (void)read_module( mcp->new_module );
	    identifier->modid = which_module(mcp->new_module);
	    return;	/* finished! */
        }
      }
    }

	/*
	 * If no exact match, load everything relevant
	 */
    read_module_replacements( old_module_name );
}


/*
 *  Read in the named module
 *	Returns the root of the whole tree
 *	(by analogy with 'read_mib')
 */
static int
read_module_internal (const char *name)
{
    struct module *mp;
    FILE *fp;
    struct node *np;

    if ( tree_head == NULL )
	init_mib_internals(); /* was init_mib */

    for ( mp=module_head ; mp ; mp=mp->next )
	if ( !label_compare(mp->name, name)) {
    	    const char *oldFile = File;
    	    int oldLine = Line;

	    if ( mp->no_imports != -1 ) {
		DEBUGMSGTL(("parse-mibs", "Module %s already loaded\n", name));
		return MODULE_ALREADY_LOADED;
	    }
	    if ((fp = fopen(mp->file, "r")) == NULL) {
		snmp_log_perror(mp->file);
		return MODULE_LOAD_FAILED;
	    }
	    mp->no_imports=0;		/* Note that we've read the file */
	    File = mp->file;
	    Line = 1;
		/*
		 * Parse the file
		 */
	    np = parse( fp, NULL );
	    fclose(fp);
	    File = oldFile;
	    Line = oldLine;
	    return MODULE_LOADED_OK;
	}

    if (ds_get_int(DS_LIBRARY_ID, DS_LIB_MIB_WARNINGS) > 1)
	snmp_log(LOG_WARNING, "Module %s not found\n", name);
    return MODULE_NOT_FOUND;
}

void
adopt_orphans (void)
{
    struct node *np, *onp;
    struct tree *tp;
    int i, adopted;

    if ( !orphan_nodes )
	return;
    init_node_hash(orphan_nodes);
    orphan_nodes = NULL;

    while (1) {
	adopted = 0;
	for ( i = 0; i < NHASHSIZE; i++)
	    if ( nbuckets[i] ) {
	        for ( np = nbuckets[i] ; np!= NULL ; np=np->next )
	            tp = find_tree_node( np->parent, -1 );
	            if ( tp ) {
		        do_subtree( tp, &np );
		        adopted = 1;
	            }
	    }
	if ( adopted == 0 )
	    break;
    }

	/*
	 * Report on outstanding orphans
	 *    and link them back into the orphan list
	 */
    for (i = 0; i < NHASHSIZE; i++)
	if ( nbuckets[i] ) {
	    if ( orphan_nodes )
		onp = np->next = nbuckets[i];
	    else
		onp = orphan_nodes = nbuckets[i];
	    nbuckets[i] = NULL;
	    while (onp) {
        	char modbuf[256];
        	snmp_log (LOG_WARNING,
                          "Unlinked OID in %s: %s ::= { %s %ld }\n",
                          module_name(onp->modid, modbuf),
                          (onp->label ? onp->label : "<no label>"),
                          (onp->parent ? onp->parent : "<no parent>"),
                          onp->subid);

		np = onp;
		onp = onp->next;
	    }
	}
}

struct tree *
read_module(const char *name)
{
    if ( read_module_internal(name) == MODULE_NOT_FOUND )
	read_module_replacements( name );
    return tree_head;
}

void
unload_module_by_ID( int modID, struct tree *tree_top )
{
    struct tree *tp, *prev, *next;
    int i;

    prev = NULL;
    for ( tp=tree_top ; tp ; tp=next ) {
	next = tp->next_peer;
		/*
		 * This next section looks rather complex.
		 * Essentially, this is equivalent to the code fragment:
		 *  	if (tp->modID = modID)
		 *	    tp->number_modules--;
		 * but handles one tree node being part of several modules.
		 */
	for ( i=0 ; i<tp->number_modules ; i++ ) {
	    if ( tp->module_list[i] == modID ) {
		tp->number_modules--;
		switch ( tp->number_modules ) {

		    case 0:	/* That was the only module */
			tp->modid = -1;		/* Mark as unused */
			break;

		    case 1:	/* We did have a list of two, but this is no
				   longer needed.  Transfer the other entry
				   ( i.e. module_list[1-i] - think about it! )
				   to the 'single' slot tp->modid, and discard
				   the list.
				 */
			tp->modid = tp->module_list[1-i];
			free(tp->module_list);
			tp->module_list = NULL;		/* let's be tidy */
			break;

		    default:	/* We still need the list, so shuffle down
				   all following entries to close up the gap */
			while ( i < tp->number_modules ) {
			   tp->module_list[i] = tp->module_list[i+1];
			   i++;
			}
			break;
		}
		break;	/* Don't need to look through the rest of the list */
	    }
	}

		/*
		 *  OK - that's dealt with *this* node.
		 *	Now let's look at the children.
		 *	(Isn't recursion wonderful!)
		 */
	if ( tp->child_list )
	    unload_module_by_ID( modID, tp->child_list );


	if ( tp->number_modules == 0 ) {
			/* This node isn't needed any more (except perhaps
				for the sake of the children) */
 	    if ( tp->child_list == NULL ) {
		if ( prev )
		    prev->next_peer = tp->next_peer;
		else
		    tp->parent->child_list = tp->next_peer;
		free_tree( tp );
	   }
	   else
		free_partial_tree( tp, TRUE );
	}
	else
	    prev = tp;
    }
}

int
unload_module(const char *name)
{
    struct module *mp;
    int modID = -1;

    for ( mp=module_head ; mp ; mp=mp->next )
	if ( !label_compare(mp->name, name)) {
	    modID = mp->modid;
	    break;
	}

    if ( modID == -1 ) {
	DEBUGMSGTL(("unload-mib", "Module %s not found to unload\n", name));
	return MODULE_NOT_FOUND;
    }
    unload_module_by_ID( modID, tree_head );
    mp->no_imports = -1;	/* mark as unloaded */
    return MODULE_LOADED_OK;	/* Well, you know what I mean! */
}

static void
new_module (const char *name,
	    const char *file)
{
    struct module *mp;

    for ( mp=module_head ; mp ; mp=mp->next )
	if ( !label_compare(mp->name, name)) {
	    DEBUGMSGTL(("parse-mibs", "Module %s already noted\n", name));
			/* Not the same file */
	    if (label_compare(mp->file, file)) {
		if (ds_get_int(DS_LIBRARY_ID, DS_LIB_MIB_WARNINGS))
		    snmp_log(LOG_WARNING,
                             "Warning: Module %s was in %s now is %s\n",
                             name, mp->file, file);

			/* Use the new one in preference */
		free(mp->file);
                mp->file = strdup(file);
            }
	    return;
	}

	/* Add this module to the list */
    DEBUGMSGTL(("parse-mibs", "  Module %d %s is in %s\n", max_module, name, file));
    mp = (struct module *) calloc(1, sizeof(struct module));
    if (mp == NULL) return;
    mp->name = strdup(name);
    mp->file = strdup(file);
    mp->imports = NULL;
    mp->no_imports = -1;	/* Not yet loaded */
    mp->modid = max_module;
    ++max_module;

    mp->next = module_head;	/* Or add to the *end* of the list? */
    module_head = mp;
}




/*
 * Parses a mib file and returns a linked list of nodes found in the file.
 * Returns NULL on error.
 */
static struct node *
parse(FILE *fp,
      struct node *root)
{
    char token[MAXTOKEN];
    char name[MAXTOKEN];
    int type = LABEL;
    int lasttype = LABEL;

#define BETWEEN_MIBS          1
#define IN_MIB                2
    int state = BETWEEN_MIBS;
    struct node *np, *nnp;

    DEBUGMSGTL(("parse-file", "Parsing file:  %s...\n", File));

    if (last_err_module) free(last_err_module); last_err_module = 0;

    np = root;
    if (np != NULL) {
        /* now find end of chain */
        while(np->next)
            np = np->next;
    }

    while (type != ENDOFFILE){
        if (lasttype == CONTINUE) lasttype = type;
        else type = lasttype = get_token(fp, token, MAXTOKEN);

        switch (type) {
        case END:
            if (state != IN_MIB){
                print_error("Error, END before start of MIB", NULL, type);
                return NULL;
            }
	    else {
		struct module *mp;
#ifdef TEST
		printf("\nNodes for Module %s:\n", name);
		print_nodes( stdout, np );
#endif
		for (mp = module_head; mp; mp = mp->next)
		    if (mp->modid == current_module) break;
		do_linkup(mp, root);
		np = root = NULL;
	    }
            state = BETWEEN_MIBS;
#ifdef TEST
            if (ds_get_int(DS_LIBRARY_ID, DS_LIB_MIB_WARNINGS))
              xmalloc_stats (stderr);
#endif
            continue;
        case IMPORTS:
            parse_imports( fp );
            continue;
        case EXPORTS:
            while (type != SEMI && type != ENDOFFILE)
                type = get_token(fp, token, MAXTOKEN);
            continue;
        case LABEL:
            break;
        case ENDOFFILE:
            continue;
        default:
	    strcpy(name, token);
	    type = get_token(fp, token, MAXTOKEN);
	    nnp = NULL;
	    if (type == MACRO) {
		nnp = parse_macro(fp, name);
		if (nnp == NULL){
		    print_error("Bad parse of MACRO", NULL, type);
		    /*return NULL;*/
		}
		free_node(nnp); /* IGNORE */
		nnp = NULL;
	    }
	    else
		print_error(name, "is a reserved word", lasttype);
            continue;         /* see if we can parse the rest of the file */
        }
        strcpy(name, token);
        type = get_token(fp, token, MAXTOKEN);
        nnp = NULL;

	/* Handle obsolete method to assign an object identifier to a
	   module*/
	if (lasttype == LABEL && type == LEFTBRACKET) {
	    while (type != RIGHTBRACKET && type != ENDOFFILE)
		type = get_token(fp, token, MAXTOKEN);
	    if (type == ENDOFFILE){
		print_error("Expected \"}\"", token, type);
		return NULL;
	    }
	    type = get_token(fp, token, MAXTOKEN);
	}

        switch (type) {
        case DEFINITIONS:
            if (state != BETWEEN_MIBS){
                print_error("Error, nested MIBS", NULL, type);
                return NULL;
            }
            state = IN_MIB;
            current_module = which_module( name );
            if ( current_module == -1 ) {
                new_module(name, File);
                current_module = which_module(name);
            }
            DEBUGMSGTL(("parse-mibs", "Parsing MIB: %d %s\n", current_module, name));
            while ((type = get_token (fp, token, MAXTOKEN)) != ENDOFFILE)
                if (type == BEGIN) break;
            break;
        case OBJTYPE:
            nnp = parse_objecttype(fp, name);
            if (nnp == NULL){
                print_error("Bad parse of OBJECT-TYPE", NULL, type);
                return NULL;
            }
            break;
        case OBJGROUP:
            nnp = parse_objectgroup(fp, name);
            if (nnp == NULL){
                print_error("Bad parse of OBJECT-GROUP", NULL, type);
                return NULL;
            }
            break;
        case TRAPTYPE:
            nnp = parse_trapDefinition(fp, name);
            if (nnp == NULL){
                print_error("Bad parse of TRAP-TYPE", NULL, type);
                return NULL;
            }
            break;
        case NOTIFTYPE:
            nnp = parse_notificationDefinition(fp, name);
            if (nnp == NULL){
                print_error("Bad parse of NOTIFICATION-TYPE", NULL, type);
                return NULL;
            }
            break;
        case COMPLIANCE:
            nnp = parse_compliance(fp, name);
            if (nnp == NULL){
                print_error("Bad parse of MODULE-COMPLIANCE", NULL, type);
                return NULL;
            }
            break;
        case CAPABILITIES:
            nnp = parse_capabilities(fp, name);
            if (nnp == NULL){
                print_error("Bad parse of AGENT-CAPABILITIES", NULL, type);
                return NULL;
            }
            break;
        case MACRO:
            nnp = parse_macro(fp, name);
            if (nnp == NULL){
                print_error("Bad parse of MACRO", NULL, type);
                /*return NULL;*/
            }
            free_node(nnp); /* IGNORE */
	    nnp = NULL;
            break;
        case MODULEIDENTITY:
            nnp = parse_moduleIdentity(fp, name);
            if (nnp == NULL){
                print_error("Bad parse of MODULE-IDENTITY", NULL, type);
                return NULL;
            }
            break;
        case OBJID:
            type = get_token(fp, token, MAXTOKEN);
            if (type != EQUALS){
                print_error("Expected \"::=\"", token, type);
                return NULL;
            }
            nnp = parse_objectid(fp, name);
            if (nnp == NULL){
                print_error("Bad parse of OBJECT IDENTIFIER", NULL, type);
                return NULL;
            }
            break;
        case EQUALS:
            nnp = parse_asntype(fp, name, &type, token);
            lasttype = CONTINUE;
            break;
        case ENDOFFILE:
            break;
        default:
	    print_error("Bad operator", token, type);
	    return NULL;
        }
        if (nnp) {
            if (np) np->next = nnp;
            else np = root = nnp;
            while (np->next) np = np->next;
        }
    }
    DEBUGMSGTL(("parse-file", "End of file (%s)\n", File));
    return root;
}

/* return zero if character is not a label character. */
static int
is_labelchar (int ich)
{
    if ((isalnum(ich)) || (ich == '-'))
	    return 1;
    if (ich == '_' && ds_get_boolean(DS_LIBRARY_ID, DS_LIB_MIB_PARSE_LABEL))
	    return 1;

    return 0;
}

/*
 * Parses a token from the file.  The type of the token parsed is returned,
 * and the text is placed in the string pointed to by token.
 * Warning: this method may recurse.
 */
static int
get_token(FILE *fp,
	  char *token,
	  int maxtlen)
{
    register int ch, ch_next;
    register char *cp = token;
    register int hash = 0;
    register struct tok *tp;
    int too_long = 0;

    /* skip all white space */
    do {
        ch = getc(fp);
        if (ch == '\n')
            Line++;
    }
    while(isspace(ch) && ch != EOF);
    *cp++ = ch; *cp = '\0';
    switch (ch) {
    case EOF:
        return ENDOFFILE;
    case '"':
        return parseQuoteString(fp, token, maxtlen);
    case '\'':	/* binary or hex constant */
	while ((ch = getc(fp)) != EOF && ch != '\'' && cp-token < maxtlen-2)
	    *cp++ = ch;
	if (ch == '\'') {
	    unsigned long val = 0;
	    *cp++ = '\'';
	    *cp++ = ch = getc(fp);
	    *cp = 0;
	    cp = token+1;
	    switch (ch) {
	    case EOF:
		return ENDOFFILE;
	    case 'b':
	    case 'B':
		while ((ch = *cp++) != '\'')
		    if (ch != '0' && ch != '1') return LABEL;
		    else val = val * 2 + ch - '0';
		break;
	    case 'h':
	    case 'H':
		while ((ch = *cp++) != '\'')
		    if ('0' <= ch && ch <= '9') val = val*16+ch-'0';
		    else if ('a' <= ch && ch <= 'f') val = val*16+ch-'a'+10;
		    else if ('A' <= ch && ch <= 'F') val = val*16+ch-'A'+10;
		    else return LABEL;
		break;
	    default:
		return LABEL;
	    }
	    sprintf(token, "%ld", val);
	    return NUMBER;
	}
	else return LABEL;
    case '(':
	return LEFTPAREN;
    case ')':
	return RIGHTPAREN;
    case '{':
	return LEFTBRACKET;
    case '}':
	return RIGHTBRACKET;
    case ';':
	return SEMI;
    case ',':
	return COMMA;
    case '|':
	return BAR;
    case '.':
	ch_next = getc(fp);
	if (ch_next == '.') return RANGE;
	ungetc(ch_next, fp);
	return LABEL;
    case ':':
	ch_next = getc(fp);
	if (ch_next != ':') {
	    ungetc(ch_next, fp);
	    return LABEL;
	}
	ch_next = getc(fp);
	if (ch_next != '=') {
	    ungetc(ch_next, fp);
	    return LABEL;
	}
	return EQUALS;
    case '-':
	ch_next = getc(fp);
	if (ch_next == '-') {
	  if (ds_get_boolean(DS_LIBRARY_ID, DS_LIB_MIB_COMMENT_TERM)) {
	    /* Treat the rest of this line as a comment. */
	    while ((ch_next != EOF) && (ch_next != '\n'))
	    ch_next = getc(fp);
	  } else {
            /* Treat the rest of the line or until another '--' as a comment */
            /* (this is the "technically" correct way to parse comments) */
	    ch = ' ';
	    ch_next = getc(fp);
	    while (ch_next != EOF && ch_next != '\n' &&
		(ch != '-' || ch_next != '-')) {
		ch = ch_next; ch_next = getc(fp);
	    }
	  }
	    if (ch_next == EOF) return ENDOFFILE;
	    if (ch_next == '\n') Line++;
	    return get_token (fp, token, maxtlen);
	}
	ungetc(ch_next, fp);
    default:
	/*
	 * Accumulate characters until end of token is found.  Then attempt to
	 * match this token as a reserved word.  If a match is found, return the
	 * type.  Else it is a label.
	 */
	if (!is_labelchar(ch)) return LABEL;
	hash += tolower(ch);
  more:
	while (is_labelchar(ch_next = getc(fp))) {
	    hash += tolower(ch_next);
	    if (cp - token < maxtlen - 1) *cp++ = ch_next;
	    else too_long = 1;
	}
	ungetc(ch_next, fp);
	*cp = '\0';

	if (too_long)
	    print_error("Warning: token too long", token, CONTINUE);
	for (tp = buckets[BUCKET(hash)]; tp; tp = tp->next) {
	    if ((tp->hash == hash) && (!label_compare(tp->name, token)))
		break;
	}
	if (tp) {
	    if (tp->token != CONTINUE) return (tp->token);
	    while (isspace((ch_next = getc(fp))))
		if (ch_next == '\n') Line++;
	    if (ch_next == EOF) return ENDOFFILE;
	    if (isalnum(ch_next)) {
		*cp++ = ch_next;
		hash += tolower(ch_next);
		goto more;
	    }
	}
	if (token[0] == '-' || isdigit(token[0])) {
	   for(cp = token+1; *cp; cp++)
	      if (!isdigit(*cp))
		  return LABEL;
	   return NUMBER;
	}
	return LABEL;
    }
}

int
snmp_get_token(FILE *fp,
	  char *token,
	  int maxtlen)
{
    return get_token(fp, token, maxtlen);
}

int
add_mibdir(const char *dirname)
{
#ifndef ECOSFIXME_NEEDFILESYSTEM
    FILE *fp, *ip;
    DIR *dir, *dir2;
    const char *oldFile = File;
    struct dirent *file;
    char token[MAXTOKEN];
    char tmpstr[300];
    int count = 0;
#ifndef WIN32
    struct stat dir_stat, idx_stat;
    char tmpstr1[300];
#endif

    DEBUGMSGTL(("parse-mibs", "Scanning directory %s\n", dirname));
#ifndef WIN32
    sprintf(token, "%s/%s", dirname, ".index");
    if (stat(token, &idx_stat) == 0 && stat(dirname, &dir_stat) == 0) {
	if (dir_stat.st_mtime < idx_stat.st_mtime) {
	    DEBUGMSGTL(("parse-mibs", "The index is good\n"));
	    if ((ip = fopen(token, "r")) != NULL) {
		while (fscanf(ip, "%s %s\n", token, tmpstr) == 2) {
		    sprintf(tmpstr1, "%s/%s", dirname, tmpstr);
		    new_module(token, tmpstr1);
		    count++;
		}
		fclose(ip);
		return count;
	    }
	    else DEBUGMSGTL(("parse-mibs", "Can't read index\n"));
	}
	else DEBUGMSGTL(("parse-mibs", "Index outdated\n"));
    }
    else DEBUGMSGTL(("parse-mibs", "No index\n"));
#endif

    if ((dir = opendir(dirname))) {
	sprintf(tmpstr, "%s/.index", dirname);
	ip = fopen(tmpstr, "w");
        while ((file = readdir(dir))) {
            /* Only parse file names not beginning with a '.' */
            if (file->d_name != NULL && file->d_name[0] != '.') {
                sprintf(tmpstr, "%s/%s", dirname, file->d_name);
                if ((dir2 = opendir(tmpstr))) {
                    /* file is a directory, don't read it */
                    closedir(dir2);
                } else {
                    /* which module is this */
                    if ((fp = fopen(tmpstr, "r")) == NULL) {
                        snmp_log_perror(tmpstr);
			continue;
                    }
                    DEBUGMSGTL(("parse-mibs", "Checking file: %s...\n", tmpstr));
                    Line = 1;
                    File = tmpstr;
                    get_token( fp, token, MAXTOKEN);
                    new_module(token, tmpstr);
                    count++;
                    fclose (fp);
		    if (ip) fprintf(ip, "%s %s\n", token, file->d_name);
                }
            }
        }
	File = oldFile;
        closedir(dir);
	if (ip) fclose(ip);
        return(count);
    }
    return(-1);

#else
    // __ECOS

#if 0 // MIBS not needed for agent use of library.
    int count = 0;
    int i;

    static const char *snmp_miblist[] = {
        "EtherLike-MIB",
        "IANAifType-MIB",
        "IF-MIB",
        "IP-MIB",
        "RFC-1215",
        "SNMPv2-CONF",
        "SNMPv2-MIB",
        "SNMPv2-SMI",
        "SNMPv2-TC",
        "SNMPv2-TM",
        "TCP-MIB",
        "UDP-MIB",
    };

    for ( i = 0;
          i < sizeof(snmp_miblist)/sizeof(snmp_miblist[0]);
          i++ ) {

        new_module(snmp_miblist[i],snmp_miblist[i]); 
        count++;
    }
    return(count);
#else
    return 0;
#endif

#endif
}


/*
 * Returns the root of the whole tree
 *   (for backwards compatability)
 */
struct tree *
read_mib(const char *filename)
{
    FILE *fp;
    char token[MAXTOKEN];

    fp = fopen(filename, "r");
    if (fp == NULL) {
        snmp_log_perror(filename);
        return NULL;
    }
    Line = 1;
    File = filename;
    DEBUGMSGTL(("parse-mibs", "Parsing file: %s...\n", filename));
    get_token( fp, token, MAXTOKEN);
    fclose(fp);
    new_module(token, filename);
    (void) read_module(token);

    return tree_head;
}


struct tree *
read_all_mibs()
{
    struct module *mp;

    for ( mp=module_head ; mp ; mp=mp->next )
	if ( mp->no_imports == -1 )
            read_module( mp->name );
    adopt_orphans();

    return tree_head;
}


#ifdef TEST
main(int argc, char *argv[])
{
    int i;
    struct tree *tp;
    ds_set_int(DS_LIBRARY_ID, DS_LIB_MIB_WARNINGS, 2);

    init_mib();

    if ( argc == 1 )
	(void) read_all_mibs();
    else
	for ( i=1 ; i<argc ; i++ )
	    read_mib( argv[i] );

    for ( tp = tree_head ; tp ; tp=tp->next_peer )
        print_subtree( stdout, tp, 0 );
    free_tree( tree_head );

    return 0;
}
#endif /* TEST */

static int
parseQuoteString(FILE *fp,
		 char *token,
		 int maxtlen)
{
    register int ch;
    int count = 0;
    int too_long = 0;
    char *token_start = token;

    for (ch = getc(fp); ch != EOF; ch = getc(fp)) {
        if (ch == '\r') continue;
        if (ch == '\n') {
            Line++;
        }
        else if (ch == '"') {
            *token = '\0';
            if (too_long &&
                ds_get_int(DS_LIBRARY_ID, DS_LIB_MIB_WARNINGS) > 1)
            {
                /* show short form for brevity sake */
                char ch_save = *(token_start + 50);
                *(token_start + 50) = '\0';
                print_error ("Warning: string too long",
                             token_start, QUOTESTRING);
                *(token_start + 50) = ch_save;
            }
            return QUOTESTRING;
        }
        /* maximum description length check.  If greater, keep parsing
           but truncate the string */
        if (++count < maxtlen)
            *token++ = ch;
        else too_long = 1;
    }

    return 0;
}

/*
 * struct index_list *
 * getIndexes(FILE *fp):
 *   This routine parses a string like  { blah blah blah } and returns a
 *   list of the strings enclosed within it.
 *
 */
static struct index_list *
getIndexes(FILE *fp, struct index_list **retp) {
  int type;
  char token[MAXTOKEN];
  char nextIsImplied = 0;

  struct index_list *mylist = NULL;
  struct index_list **mypp = &mylist;

  free_indexes(retp);

  type = get_token(fp, token, MAXTOKEN);

  if (type != LEFTBRACKET) {
    return NULL;
  }

  type = get_token(fp, token, MAXTOKEN);
  while (type != RIGHTBRACKET && type != ENDOFFILE) {
    if ((type == LABEL) || (type & SYNTAX_MASK)) {
      *mypp = (struct index_list *) calloc(1, sizeof(struct index_list));
      if (*mypp) {
        (*mypp)->ilabel = strdup(token);
        (*mypp)->isimplied = nextIsImplied;
        mypp = &(*mypp)->next;
        nextIsImplied = 0;
      }
    } else if (type == IMPLIED) {
        nextIsImplied = 1;
    }
    type = get_token(fp, token, MAXTOKEN);
  }

  *retp = mylist;
  return mylist;
}

static void
free_indexes(struct index_list **spp) {
  if (spp && *spp) {
  struct index_list *pp, *npp;

  pp = *spp; *spp = NULL;

  while(pp) {
    npp = pp->next;
    if (pp->ilabel) free(pp->ilabel);
    free(pp);
    pp = npp;
  }
  }
}

static void
free_ranges(struct range_list **spp) {
  if (spp && *spp) {
  struct range_list *pp, *npp;

  pp = *spp; *spp = NULL;

  while(pp) {
    npp = pp->next;
    free(pp);
    pp = npp;
  }
  }
}

static void
free_enums(struct enum_list **spp)
{
  if (spp && *spp) {
  struct enum_list *pp, *npp;

  pp = *spp; *spp = NULL;

  while(pp)
  {
    npp = pp->next;
    if (pp->label) free(pp->label);
    free(pp);
    pp = npp;
  }
  }
}

static struct enum_list *
copy_enums (struct enum_list *sp)
{
  struct enum_list *xp = NULL, **spp = &xp;

  while (sp) {
    *spp = (struct enum_list *) calloc(1, sizeof(struct enum_list));
    if (!*spp) break;
    (*spp)->label = strdup(sp->label);
    (*spp)->value = sp->value;
    spp = &(*spp)->next;
    sp = sp->next;
  }
  return (xp);
}

static struct range_list *
copy_ranges (struct range_list *sp)
{
  struct range_list *xp = NULL, **spp = &xp;

  while (sp) {
    *spp = (struct range_list *) calloc(1, sizeof(struct range_list));
    if (!*spp) break;
    (*spp)->low = sp->low;
    (*spp)->high = sp->high;
    spp = &(*spp)->next;
    sp = sp->next;
  }
  return (xp);
}

static struct index_list *
copy_indexes (struct index_list *sp)
{
  struct index_list *xp = NULL, **spp = &xp;

  while (sp) {
    *spp = (struct index_list *) calloc(1, sizeof(struct index_list));
    if (!*spp) break;
    (*spp)->ilabel = strdup(sp->ilabel);
    spp = &(*spp)->next;
    sp = sp->next;
  }
  return (xp);
}

/*
 * This routine parses a string like  { blah blah blah } and returns OBJID if
 * it is well formed, and NULL if not.
 */
static int
tossObjectIdentifier(FILE *fp)
{
    int type;
    char token[MAXTOKEN];
    int bracketcount = 1;

    type = get_token(fp, token, MAXTOKEN);

    if (type != LEFTBRACKET)
        return 0;
    while ((type != RIGHTBRACKET || bracketcount > 0) && type != ENDOFFILE )
    {
        type = get_token(fp, token, MAXTOKEN);
        if (type == LEFTBRACKET)
          bracketcount++;
        else if (type == RIGHTBRACKET)
          bracketcount--;
    }

    if (type == RIGHTBRACKET)
        return OBJID;
    else
        return 0;
}

struct tree *
find_node(const char *name,
	  struct tree *subtree)    /* Unused */
{
  return( find_tree_node( name, -1 ));
}

struct module *
find_module(int mid)
{
  struct module *mp;

  for(mp=module_head; mp!=NULL; mp = mp->next) {
    if (mp->modid == mid)
      break;
  }
  if (mp != 0)
    return mp;
  return NULL;
}


static char leave_indent[256];
static int leave_was_simple;

static void print_mib_leaves(FILE *f, struct tree *tp, int width)
{ struct tree *ntp;
  char *ip = leave_indent+strlen(leave_indent)-1;
  char last_ipch = *ip;

  *ip = '+';
  if (tp->type == 0)
    fprintf(f, "%s--%s(%ld)\n", leave_indent, tp->label, tp->subid);
  else {
    const char *acc, *typ;
    int size = 0;
    switch (tp->access) {
    case MIB_ACCESS_NOACCESS:	acc = "----"; break;
    case MIB_ACCESS_READONLY:	acc = "-R--"; break;
    case MIB_ACCESS_WRITEONLY:	acc = "--W-"; break;
    case MIB_ACCESS_READWRITE:	acc = "-RW-"; break;
    case MIB_ACCESS_NOTIFY:	acc = "---N"; break;
    case MIB_ACCESS_CREATE:	acc = "CR--"; break;
    default:			acc = "    "; break;
    }
    switch (tp->type) {
    case TYPE_OBJID:		typ = "ObjID    "; break;
    case TYPE_OCTETSTR:		typ = "String   "; size = 1; break;
    case TYPE_INTEGER:
		if (tp->enums)	typ = "EnumVal  ";
		else		typ = "Integer  "; break;
    case TYPE_NETADDR:		typ = "NetAddr  "; break;
    case TYPE_IPADDR:		typ = "IpAddr   "; break;
    case TYPE_COUNTER:		typ = "Counter  "; break;
    case TYPE_GAUGE:		typ = "Gauge    "; break;
    case TYPE_TIMETICKS:	typ = "TimeTicks"; break;
    case TYPE_OPAQUE:		typ = "Opaque   "; size = 1; break;
    case TYPE_NULL:		typ = "Null     "; break;
    case TYPE_COUNTER64:	typ = "Counter64"; break;
    case TYPE_BITSTRING:	typ = "BitString"; break;
    case TYPE_NSAPADDRESS:	typ = "NsapAddr "; break;
    case TYPE_UINTEGER:		typ = "UInteger "; break;
    default:			typ = "         "; break;
    }
    fprintf(f, "%s-- %s %s %s(%ld)\n", leave_indent, acc, typ, tp->label, tp->subid);
    *ip = last_ipch;
    if (tp->tc_index >= 0)
      fprintf(f, "%s        Textual Convention: %s\n", leave_indent,
	      tclist[tp->tc_index].descriptor);
    if (tp->enums) {
      struct enum_list *ep = tp->enums;
      int cpos = 0, cmax = width - strlen(leave_indent) - 16;
      fprintf(f, "%s        Values: ", leave_indent);
      while (ep) {
	char buf[80];
	int bufw;
	if (ep != tp->enums) fprintf(f, ", ");
	sprintf(buf, "%s(%d)", ep->label, ep->value);
	cpos += (bufw = strlen(buf) + 2);
	if (cpos >= cmax) {
	  fprintf(f, "\n%s                ", leave_indent);
	  cpos = bufw;
	}
	fprintf(f, "%s", buf);
	ep = ep->next;
      }
      fprintf(f, "\n");
    }
    if (tp->ranges) {
      struct range_list *rp = tp->ranges;
      if (size) fprintf(f, "%s        Size: ", leave_indent);
      else fprintf(f, "%s        Range: ", leave_indent);
      while (rp) {
        if (rp != tp->ranges) fprintf(f, " | ");
	if (rp->low == rp->high) fprintf(f, "%d", rp->low);
	else fprintf(f, "%d..%d", rp->low, rp->high);
	rp = rp->next;
      }
      fprintf(f, "\n");
    }
  }
  *ip = last_ipch;
  strcat(leave_indent, "  |");
  leave_was_simple = tp->type != 0;

  { int i, j, count = 0;
    struct leave {
      oid id;
      struct tree *tp;
    } *leaves, *lp;

    for (ntp = tp->child_list; ntp; ntp = ntp->next_peer) count++;
    if (count) {
      leaves = (struct leave *)calloc(count, sizeof(struct leave));
      if (!leaves) return;
      for (ntp = tp->child_list, count = 0; ntp; ntp = ntp->next_peer) {
	for (i = 0, lp = leaves; i < count; i++, lp++)
	  if (lp->id >= ntp->subid) break;
	for (j = count; j > i; j--) leaves[j] = leaves[j-1];
	lp->id = ntp->subid;
	lp->tp = ntp;
	count++;
      }
      for (i = 1, lp = leaves; i <= count; i++, lp++) {
	if (!leave_was_simple || lp->tp->type == 0)
	  fprintf(f, "%s\n", leave_indent);
	if (i == count) ip[3] = ' ';
	print_mib_leaves(f, lp->tp, width);
      }
      free(leaves);
      leave_was_simple = 0;
    }
  }
  ip[1] = 0;
}

void print_mib_tree(FILE *f, struct tree *tp, int width)
{
  leave_indent[0] = ' ';
  leave_indent[1] = 0;
  leave_was_simple = 1;
  print_mib_leaves(f, tp, width);
}


/*
 * Merge the parsed object identifier with the existing node.
 * If there is a problem with the identifier, release the existing node.
 */
static struct node *
merge_parse_objectid(struct node *np,
		     FILE *fp,
		     char *name)
{
    struct node *nnp;

    nnp = parse_objectid(fp, name);
    if (nnp) {
	/* apply last OID sub-identifier data to the information */
	/* already collected for this node. */
	struct node *headp, *nextp;
	int ncount = 0;
	nextp = headp = nnp;
	while (nnp->next) {
	    nextp = nnp;
	    ncount++;
	    nnp = nnp->next;
	}

	np->label = nnp->label;
	np->subid = nnp->subid;
	np->modid = nnp->modid;
	np->parent = nnp->parent;
	free(nnp);

	if (ncount) {
	    nextp->next = np;
	    np = headp;
	}
    }
    else {
        free_node(np); np = NULL;
    }

    return np;
}

/*
 * transfer data to tree from node
 *
 * move pointers for alloc'd data from np to tp.
 * this prevents them from being freed when np is released.
 * parent member is not moved.
 *
 * CAUTION: nodes may be repeats of existing tree nodes.
 * This can happen especially when resolving IMPORT clauses.
 *
 */
static void
tree_from_node(struct tree *tp, struct node *np)
{
    free_partial_tree(tp, FALSE);

    tp->label = np->label;  np->label = NULL;
    tp->enums = np->enums;  np->enums = NULL;
    tp->ranges = np->ranges;  np->ranges = NULL;
    tp->indexes = np->indexes;  np->indexes = NULL;
    tp->hint = np->hint;  np->hint = NULL;
    tp->units = np->units;  np->units = NULL;
    tp->description = np->description;  np->description = NULL;

    tp->subid = np->subid;
    tp->tc_index = np->tc_index;
    tp->type = translation_table[np->type];
    tp->access = np->access;
    tp->status = np->status;
    set_function(tp);
}

