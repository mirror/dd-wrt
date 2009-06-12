/* Definitions for BSD-style memory management.
   Copyright (C) 1994-1998,2000,01 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* These are the bits used by 4.4 BSD and its derivatives.  On systems
   (such as GNU) where these facilities are not system services but can be
   emulated in the C library, these are the definitions we emulate.  */

#ifndef _SYS_MMAN_H
# error "Never use <bits/mman.h> directly; include <sys/mman.h> instead."
#endif

/* Protections are chosen from these bits, OR'd together.  The
   implementation does not necessarily support PROT_EXEC or PROT_WRITE
   without PROT_READ.  The only guarantees are that no writing will be
   allowed without PROT_WRITE and no access will be allowed for PROT_NONE. */

#define	PROT_NONE	 0x00	/* No access.  */
#define	PROT_READ	 0x01	/* Pages can be read.  */
#define	PROT_WRITE	 0x02	/* Pages can be written.  */
#define	PROT_EXEC	 0x04	/* Pages can be executed.  */

/* Sharing types (must choose one and only one of these).  */
#define MAP_SHARED	0x01		/* Share changes.  */
#define MAP_PRIVATE	0x02		/* Changes are private.  */
#ifdef __USE_MISC
# define MAP_TYPE	0x0f		/* Mask for type of mapping.  */
#endif

/* Other flags.  */
#define MAP_FIXED	0x10		/* Interpret addr exactly.  */
#ifdef __USE_MISC
# define MAP_FILE	0
# define MAP_ANONYMOUS	0x20		/* Don't use a file.  */
# define MAP_ANON	MAP_ANONYMOUS
#endif

/* These are Linux-specific.  */
#ifdef __USE_MISC
# define MAP_GROWSDOWN	0x0100		/* Stack-like segment.  */
# define MAP_DENYWRITE	0x0800		/* ETXTBSY */
# define MAP_EXECUTABLE	0x1000		/* Mark it as an executable.  */
# define MAP_LOCKED	0x2000		/* Lock the mapping.  */
# define MAP_NORESERVE	0x4000		/* Don't check for reservations.  */
#endif

/* Flags to `msync'.  */
#define MS_ASYNC	1		/* Sync memory asynchronously.  */
#define MS_SYNC		4		/* Synchronous memory sync.  */
#define MS_INVALIDATE	2		/* Invalidate the caches.  */

/* Flags for `mlockall'.  */
#define MCL_CURRENT	1		/* Lock all currently mapped pages.  */
#define MCL_FUTURE	2		/* Lock all additions to address
					   space.  */

/* Flags for `mremap'.  */
#ifdef __USE_GNU
# define MREMAP_MAYMOVE	1
#endif
