//==========================================================================
//
//      ./lib/current/include/snmp_logging.h
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
#ifndef SNMP_LOGGING_H
#define SNMP_LOGGING_H

#ifdef __cplusplus
extern "C" {
#endif

#if HAVE_SYSLOG_H
#include <syslog.h>
#endif
#if HAVE_STDARG_H
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifndef LOG_ERR
#define LOG_EMERG       0       /* system is unusable */
#define LOG_ALERT       1       /* action must be taken immediately */
#define LOG_CRIT        2       /* critical conditions */
#define LOG_ERR         3       /* error conditions */
#define LOG_WARNING     4       /* warning conditions */
#define LOG_NOTICE      5       /* normal but significant condition */
#define LOG_INFO        6       /* informational */
#define LOG_DEBUG       7       /* debug-level messages */

#endif

typedef void (*snmp_log_callback)(int level, const char* text);
struct snmp_log_message {
   int priority;
   const char *msg;
};

void init_snmp_logging(void);
int  snmp_get_do_logging(void);
void snmp_disable_syslog(void);
void snmp_disable_filelog(void);
void snmp_disable_stderrlog(void);
void snmp_disable_calllog(void);
void snmp_disable_log(void);
void snmp_enable_syslog(void);
void snmp_enable_filelog(const char *logfilename, int dont_zero_log);
void snmp_enable_stderrlog(void);
void snmp_enable_calllog(void);

#if HAVE_STDARG_H
int snmp_log(int priority, const char *format, ...);
#else
int snmp_log (va_alist);
#endif
int snmp_vlog(int priority, const char *format, va_list ap);
   /*  0 - successful message formatting */
   /* -1 - Could not format log-string */
   /* -2 - Could not allocate memory for log-message */
   /* -3 - Log-message too long! */

void snmp_log_perror(const char *s);

#ifdef __cplusplus
}
#endif

#endif /* SNMP_LOGGING_H */
