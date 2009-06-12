/*========================================================================
//
//      sys/select.h
//
//      POSIX definitions for select()
//
//========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Nick Garnett
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2001-07-26
// Purpose:       This file provides the macros, types and functions
//                required by POSIX 1003.1.
// Description:   Much of the real contents of this file get set from the
//                configuration (set by the implementation)
// Usage:         #include <sys/select.h>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/isoinfra.h>          /* Configuration header */

/* ------------------------------------------------------------------- */

#if !defined(_POSIX_SOURCE)

#ifdef CYGINT_ISO_SELECT
# ifdef CYGBLD_ISO_SELECT_HEADER
#  include CYGBLD_ISO_SELECT_HEADER
# else

#   ifndef CYGONCE_ISO_SYS_SELECT_FD_SETS
#   define CYGONCE_ISO_SYS_SELECT_FD_SETS

#define	NBBY	8		/* number of bits in a byte */

/*
 * Select uses bit masks of file descriptors in longs.  These macros
 * manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here should
 * be enough for most uses.
 */
#ifndef	FD_SETSIZE
#define	FD_SETSIZE	256
#endif

typedef unsigned int	fd_mask;
#define __NFDBITS	(sizeof(fd_mask) * NBBY)	/* bits per mask */

#ifndef __howmany
#define	__howmany(__x, __y)	(((__x) + ((__y) - 1)) / (__y))
#endif

typedef	struct fd_set {
	fd_mask	fds_bits[__howmany(FD_SETSIZE, __NFDBITS)];
} fd_set;

#define	FD_SET(__n, __p)   ((__p)->fds_bits[(__n)/__NFDBITS] |= (1 << ((__n) % __NFDBITS)))
#define	FD_CLR(__n, __p)   ((__p)->fds_bits[(__n)/__NFDBITS] &= ~(1 << ((__n) % __NFDBITS)))
#define	FD_ISSET(__n, __p) ((__p)->fds_bits[(__n)/__NFDBITS] & (1 << ((__n) % __NFDBITS)))

#define	FD_COPY(__f, __t)                                       \
{                                                               \
    unsigned int _i;                                            \
    for( _i = 0; _i < __howmany(FD_SETSIZE, __NFDBITS) ; _i++ ) \
        (__t)->fds_bits[_i] = (__f)->fds_bits[_i];              \
}

#define	FD_ZERO(__p)                                            \
{                                                               \
    unsigned int _i;                                            \
    for( _i = 0; _i < __howmany(FD_SETSIZE, __NFDBITS) ; _i++ ) \
        (__p)->fds_bits[_i] = 0;                                \
}

#   endif /* CYGONCE_ISO_SYS_SELECT_FD_SETS */

#  ifndef __NEED_FD_SETS_ONLY

#   ifndef CYGONCE_ISO_SYS_SELECT_H
#   define CYGONCE_ISO_SYS_SELECT_H

#   ifdef __cplusplus
extern "C" {
#   endif

struct timeval;
extern int
select( int /* nfd */, fd_set * /* in */, fd_set * /* out */,
        fd_set * /* ex */, struct timeval * /* tv */ );

#ifdef CYGPKG_POSIX
# include <pkgconf/posix.h>
# ifdef CYGPKG_POSIX_SIGNALS
#  include <signal.h>
struct timespec;
extern int
pselect( int /* nfd */, fd_set * /* in */, fd_set * /* out */,
        fd_set * /* ex */, const struct timespec * /* ts */,
        const sigset_t * /* mask */);
# endif
#endif
    
#   ifdef __cplusplus
}   /* extern "C" */
#   endif

#   endif /* CYGONCE_ISO_SYS_SELECT_H multiple inclusion protection */

#  endif /* __NEED_FD_SETS_ONLY */

# endif
#endif


#endif /* if !defined(_POSIX_SOURCE) */
/* ------------------------------------------------------------------- */

/* EOF sys/select.h */
