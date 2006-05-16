/* acconfig.h -- autoheader configuration file

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
 */


#ifndef __LZO_CONFIG_H
#define __LZO_CONFIG_H

/* $TOP$ */
@TOP@

/* acconfig.h

   Descriptive text for the C preprocessor macros that
   the distributed Autoconf macros can define.
   No software package will use all of them; autoheader copies the ones
   your configure.in uses into your configuration header file templates.

   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  Although this order
   can split up related entries, it makes it easier to check whether
   a given entry is in the file.

   Leave the following blank line there!!  Autoheader needs it.  */


/* Define if your machine can copy aligned words much faster than bytes.  */
#undef LZO_ALIGNED_OK_4

/* Define for machines where assembler versions are not available.  */
#undef LZO_NO_ASM

/* Define for machines that can access unaligned short words.  */
#undef LZO_UNALIGNED_OK_2

/* Define for machines that can access unaligned words.  */
#undef LZO_UNALIGNED_OK_4

/* Define to your architecture name.  */
#undef MFX_ARCH

/* Define for machines where ".align 4" means align to a 4 byte boundary.  */
#undef MFX_ASM_ALIGN_BYTES

/* Define for machines where ".align 4" means align to a 2**4 boundary.  */
#undef MFX_ASM_ALIGN_PTWO

/* Define for i386 machines where the ebp register is reserved.  */
#undef MFX_ASM_CANNOT_USE_EBP

/* Define for machines where the assmbler understands ".type".  */
#undef MFX_ASM_HAVE_TYPE

/* Define for machines where global symbols don't have leading underscores.  */
#undef MFX_ASM_NAME_NO_UNDERSCORES

/* Define if your compiler is broken.  */
#undef MFX_PROG_CC_BUG_SIGNED_TO_UNSIGNED_CASTING

/* Define to your byte order.  */
#undef MFX_BYTE_ORDER

/* Define to your CPU name.  */
#undef MFX_CPU

/* Define if your memcmp is broken.  */
#undef NO_MEMCMP



/* Leave that blank line there!!  Autoheader needs it.
   If you're adding to this file, keep in mind:
   The entries are in sort -df order: alphabetical, case insensitive,
   ignoring punctuation (such as underscores).  */



@BOTTOM@

/* $BOTTOM$ */

#ifdef __cplusplus
#  undef /**/ const
#endif

#if defined(HAVE_SYS_RESOURCE_H) && !defined(TIME_WITH_SYS_TIME)
#  undef /**/ HAVE_SYS_RESOURCE_H
#endif

#if defined(HAVE_SYS_TIMES_H) && !defined(TIME_WITH_SYS_TIME)
#  undef /**/ HAVE_SYS_TIMES_H
#endif

#if !defined(HAVE_LIBZ)
#  undef /**/ HAVE_ZLIB_H
#endif

#if defined(NO_MEMCMP)
#  undef /**/ HAVE_MEMCMP
#endif

#if (SIZEOF_CHAR_P <= 0)
#  undef /**/ SIZEOF_CHAR_P
#endif

#if (SIZEOF_PTRDIFF_T <= 0)
#  undef /**/ SIZEOF_PTRDIFF_T
#endif

#if (SIZEOF_UNSIGNED <= 0)
#  undef /**/ SIZEOF_UNSIGNED
#endif

#if (SIZEOF_UNSIGNED_LONG <= 0)
#  undef /**/ SIZEOF_UNSIGNED_LONG
#endif

#if (SIZEOF_UNSIGNED_SHORT <= 0)
#  undef /**/ SIZEOF_UNSIGNED_SHORT
#endif

#if (SIZEOF_SIZE_T <= 0)
#  undef /**/ SIZEOF_SIZE_T
#endif

#endif /* already included */

/*
vi:ts=4
*/
