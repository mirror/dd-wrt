//==========================================================================
//
//      ./lib/current/include/system.h
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
#ifndef SNMP_SYSTEM_H
#define SNMP_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
        Copyright 1993 by Carnegie Mellon University

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
/*
 * Definitions for the system dependent library file
 */
#ifdef WIN32
#include <sys/timeb.h>
#include <time.h>
/* structure of a directory entry */
typedef struct direct 
{
	long	d_ino;		/* inode number (not used by MS-DOS) */
	int	d_namlen;		/* Name length */
	char	d_name[257];/* file name */
} _DIRECT;

/* structure for dir operations */
typedef struct _dir_struc
{
	char	*start;			/* Starting position */
	char	*curr;			/* Current position */
	long	size;			/* Size of string table */
	long	nfiles;			/* number if filenames in table */
	struct direct dirstr;	/* Directory structure to return */
} DIR;

DIR *opendir (const char *filename);
struct direct *readdir (DIR *dirp);
int closedir (DIR *dirp);

#ifndef HAVE_GETTIMEOFDAY
int gettimeofday (struct timeval *, struct timezone *tz);
#endif
#ifndef HAVE_STRCASECMP
int strcasecmp(const char *s1, const char *s2);
#endif
#ifndef HAVE_STRNCASECMP
int strncasecmp(const char *s1, const char *s2, size_t n);
#endif

char * winsock_startup (void);
void winsock_cleanup (void);

#define SOCK_STARTUP winsock_startup()
#define SOCK_CLEANUP winsock_cleanup()
#else
#define SOCK_STARTUP
#define SOCK_CLEANUP
#endif

in_addr_t get_myaddr (void);
long get_uptime (void);

#if HAVE_STDARG_H
void DEBUGP (const char *, ...);
#else
void DEBUGP (va_alist);
#endif

#ifdef  HAVE_CPP_UNDERBAR_FUNCTION_DEFINED
#define DEBUGPL(x)	\
	    DEBUGP("%s():%s,%d: ",__FUNCTION__,__FILE__,__LINE__); DEBUGP x ;
#else
#define DEBUGPL(x)	\
	    DEBUGP("():%s,%d: ",__FILE__,__LINE__); DEBUGP x ;
#endif

#ifndef HAVE_STRDUP
char *strdup (const char *);
#endif
#ifndef HAVE_SETENV
int setenv (const char *, const char *, int);
#endif

#ifdef __ECOS
#include <pkgconf/system.h>
#ifdef CYGPKG_NET_FREEBSD_STACK
#include <sys/param.h>
#undef printf
#endif //CYGPKG_NET_FREEBSD_STACK
#endif // __ECOS

#if TIME_WITH_SYS_TIME	
#	ifdef WIN32
#		include <sys/timeb.h>
#	else
#		include <sys/time.h>
#	endif
#	include <time.h>
#else
#	if HAVE_SYS_TIME_H
#		include <sys/time.h>
#	else
#		include <time.h>
#	endif
#endif
 
int calculate_time_diff(struct timeval *, struct timeval *);

#ifndef HAVE_STRCASESTR
char *strcasestr(const char *, const char *);
#endif

int mkdirhier(const char *pathname, mode_t mode, int skiplast);

#ifdef __cplusplus
}
#endif

#ifndef HAVE_GETTIMEOFDAY
int gettimeofday (struct timeval *, struct timezone *tz);
#endif
#ifndef HAVE_STRCASECMP
int strcasecmp(const char *s1, const char *s2);
#endif
#ifndef HAVE_STRNCASECMP
int strncasecmp(const char *s1, const char *s2, size_t n);
#endif

#endif /* SNMP_SYSTEM_H */
