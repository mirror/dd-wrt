
#ifndef VSTR__HEADER_H
# error " You must _just_ #include <vstr.h>"
#endif
/*
 *  Copyright (C) 2004  James Antill
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  email: james@and.org
 */

#if VSTR_COMPILE_INCLUDE

# ifdef VSTR_AUTOCONF_HAVE_POSIX_HOST

#  if VSTR_AUTOCONF_HAVE_OFF64_T && defined(__linux__) && defined(__GLIBC__)
/* already included glibc headers, so we can't assume off64_t works */
#   ifndef _LARGEFILE64_SOURCE
#     define VSTR__COMPILE_SKIP_BROKEN_LFS 1
#   endif
#  endif

#  ifndef  _LARGEFILE64_SOURCE
#   define _LARGEFILE64_SOURCE 1
#  endif
# endif

# include <stdlib.h> /* size_t */
# include <stdarg.h> /* va_list */
# include <string.h> /* strlen()/memcpy()/memset() in headers */

# ifdef VSTR_AUTOCONF_HAVE_POSIX_HOST
#  include <sys/types.h>  /* mode_t off64_t/off_t */
#  include <sys/uio.h>    /* struct iovec */
# else
struct iovec /* need real definition, as it's used inline */
{
 void *iov_base;
 size_t iov_len;
};

# endif

#ifdef VSTR_AUTOCONF_HAVE_INTTYPES_H
# include <inttypes.h>
#else
# ifdef VSTR_AUTOCONF_HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif

#endif

