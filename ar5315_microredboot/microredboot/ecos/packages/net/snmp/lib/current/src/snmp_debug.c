//==========================================================================
//
//      ./lib/current/src/snmp_debug.c
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
#include <config.h>

#include <stdio.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_STDARG_H
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include "asn1.h"
#include "mib.h"
#include "snmp_api.h"
#include "read_config.h"
#include "snmp_debug.h"
#include "snmp_impl.h"
#include "snmp_logging.h"

static int   dodebug = SNMP_ALWAYS_DEBUG;
static int   debug_num_tokens=0;
static char *debug_tokens[MAX_DEBUG_TOKENS];
static int   debug_print_everything=0;

/* indent debugging:  provide a space padded section to return an indent for */
static int debugindent=0;
#define INDENTMAX 80
static char debugindentchars[] = "                                                                                ";

char *
debug_indent(void) {
  return debugindentchars;
}

void
debug_indent_add(int amount) {
  if (debugindent+amount >= 0 && debugindent+amount < 80) {
    debugindentchars[debugindent] = ' ';
    debugindent += amount;
    debugindentchars[debugindent] = '\0';
  }
}

void
#if HAVE_STDARG_H
DEBUGP(const char *first, ...)
#else
DEBUGP(va_alist)
  va_dcl
#endif
{
  va_list args;
#if HAVE_STDARG_H
  va_start(args, first);
#else
  const char *first;
  va_start(args);
  first = va_arg(args, const char *);
#endif

  if (dodebug && (debug_print_everything || debug_num_tokens == 0)) {
    fprintf(stderr, "%s: ", DEBUG_ALWAYS_TOKEN);
    vfprintf(stderr, first, args);
  }
  va_end(args);
}

void
DEBUGPOID(oid *theoid,
	  size_t len)
{
  char c_oid[SPRINT_MAX_LEN];
  sprint_objid(c_oid,theoid,len);
  DEBUGP(c_oid);
}

void debug_config_register_tokens(const char *configtoken, char *tokens) {
  debug_register_tokens(tokens);
}

void debug_config_turn_on_debugging(const char *configtoken, char *line) {
  snmp_set_do_debugging(atoi(line));
}

void
snmp_debug_init(void) {
  debugindentchars[0] = '\0'; /* zero out the debugging indent array. */
  register_premib_handler("snmp","doDebugging",
                          debug_config_turn_on_debugging, NULL,
                          "(1|0)");
  register_premib_handler("snmp","debugTokens",
                          debug_config_register_tokens, NULL,
                          "token[,token...]");
}

void debug_register_tokens(char *tokens) {
  char *newp, *cp;
  
  if (tokens == 0 || *tokens == 0)
    return;

  newp = strdup(tokens); /* strtok messes it up */
  cp = strtok(newp, DEBUG_TOKEN_DELIMITER);
  while(cp) {
    if (strlen(cp) < MAX_DEBUG_TOKEN_LEN) {
      if (strcasecmp(cp, DEBUG_ALWAYS_TOKEN) == 0)
        debug_print_everything = 1;
      else if (debug_num_tokens < MAX_DEBUG_TOKENS)
        debug_tokens[debug_num_tokens++] = strdup(cp);
    }
    cp = strtok(NULL, DEBUG_TOKEN_DELIMITER);
  }
  free(newp);
}


/*
  debug_is_token_registered(char *TOKEN):

  returns SNMPERR_SUCCESS
       or SNMPERR_GENERR

  if TOKEN has been registered and debugging support is turned on.
*/
int
debug_is_token_registered(const char *token) {
  int i;

  /* debugging flag is on or off */
  if (!dodebug)
    return SNMPERR_GENERR;
  
  if (debug_num_tokens == 0 || debug_print_everything) {
    /* no tokens specified, print everything */
    return SNMPERR_SUCCESS;
  } else {
    for(i=0; i < debug_num_tokens; i++) {
      if (strncmp(debug_tokens[i], token, strlen(debug_tokens[i])) == 0) {
        return SNMPERR_SUCCESS;
      }
    }
  }
  return SNMPERR_GENERR;
}

void
#if HAVE_STDARG_H
debugmsg(const char *token, const char *format, ...)
#else
debugmsg(va_alist)
  va_dcl
#endif
{
  va_list debugargs;
  
#if HAVE_STDARG_H
  va_start(debugargs,format);
#else
  const char *format;
  const char *token;

  va_start(debugargs);
  token = va_arg(debugargs, const char *);
  format = va_arg(debugargs, const char *); /* ??? */
#endif

  if (debug_is_token_registered(token) == SNMPERR_SUCCESS) {
    snmp_vlog(LOG_DEBUG, format, debugargs);
  }
  va_end(debugargs);
}

void
debugmsg_oid(const char *token, oid *theoid, size_t len) {
  char c_oid[SPRINT_MAX_LEN];
  
  sprint_objid(c_oid, theoid, len);
  debugmsg(token, c_oid);
}

void
debugmsg_hex(const char *token, u_char *thedata, size_t len) {
  char buf[SPRINT_MAX_LEN];
  
  sprint_hexstring(buf, thedata, len);
  debugmsg(token, buf);
}

void
debugmsg_hextli(const char *token, u_char *thedata, size_t len) {
  char buf[SPRINT_MAX_LEN];
  int incr; 

  /*XX tracing lines removed from this function DEBUGTRACE; */
  DEBUGIF(token) {
    for(incr = 16; len > 0; len -= incr, thedata += incr) {
      if ((int)len < incr) incr = len;
      /*XXnext two lines were DEBUGPRINTINDENT(token);*/
      debugmsgtoken(token, "%s", debug_indent());
      debugmsg(token, "%s", debug_indent());
      sprint_hexstring(buf, thedata, incr);
      debugmsg(token, buf);
    }
  }
}

void
#if HAVE_STDARG_H
debugmsgtoken(const char *token, const char *format, ...)
#else
debugmsgtoken(va_alist)
  va_dcl
#endif
{
  va_list debugargs;

#if HAVE_STDARG_H
  va_start(debugargs,format);
#else
  const char *token;

  va_start(debugargs);
  token = va_arg(debugargs, const char *);
#endif

  debugmsg(token, "%s: ", token);

  va_end(debugargs);
}
  
/* for speed, these shouldn't be in default_storage space */
void
snmp_set_do_debugging(int val)
{
  dodebug = val;
}

int
snmp_get_do_debugging (void)
{
  return dodebug;
}
