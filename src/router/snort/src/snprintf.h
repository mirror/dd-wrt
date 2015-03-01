/* $Id$ */
/*
** Copyright (C) 2002 Martin Roesch <roesch@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef __SNPRINTF_H__
#define __SNPRINTF_H__

#ifndef HAVE_SNPRINTF

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <sys/types.h>
#include <string.h>

#ifdef __STDC__

#include <stdarg.h>

# define VA_LOCAL_DECL  va_list ap;
# define VA_START(f)    va_start(ap, f)
# define VA_END         va_end(ap)

#else /* __STDC__ */

#ifndef WIN32
#include <varargs.h>
#endif  /* WIN32 */

# define VA_LOCAL_DECL  va_list ap;
# define VA_START(f)    va_start(ap)
# define VA_END         va_end(ap)

#endif /* __STDC__ */

#ifndef __P
#include "cdefs.h"
#endif /* ! __P */

#ifndef QUAD_T
# define QUAD_T unsigned long
#endif /* ! QUAD_T */



#define tTd(flag, level)        (tTdvect[flag] >= (u_char)level)
#define MAXSHORTSTR  203             /* max short string length */

u_char   tTdvect[100];   /* trace vector */

int snprintf(char *, size_t , const char *, ...);
#ifndef HAVE_VSNPRINTF
int vsnprintf(char *, size_t, const char *, va_list);
#endif /* HAVE_VSNPRINTF */
char *shortenstring(register const char *, int);


#endif /* HAVE_SNPRINTF */
#endif /* __SNPRINTF_H__ */
