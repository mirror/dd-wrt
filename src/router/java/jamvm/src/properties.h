/*
 * Copyright (C) 2009 Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/* If we have endian.h include it.  Otherwise, include sys/param.h
   if we have it. If the BYTE_ORDER macro is still undefined, we
   fall-back, and work out the endianness ourselves at runtime --
   this always works.
*/
#ifdef HAVE_ENDIAN_H
#include <endian.h>
#elif HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#if defined BYTE_ORDER
#if BYTE_ORDER == BIG_ENDIAN
#define IS_BIG_ENDIAN TRUE
#else
#define IS_BIG_ENDIAN FALSE
#endif
#elif defined _BIG_ENDIAN
#define IS_BIG_ENDIAN TRUE
#elif defined _LITTLE_ENDIAN
#define IS_BIG_ENDIAN FALSE
#else
#define IS_BIG_ENDIAN ({                                           \
    /* No byte-order macro -- work it out ourselves at runtime */  \
    union {                                                        \
        int i;                                                     \
        char c[sizeof(int)];                                       \
    } u;                                                           \
    u.i = 1;                                                       \
    u.c[sizeof(int)-1] == 1;                                       \
})
#endif

#define IS_BE64 (sizeof(uintptr_t) == 8 && IS_BIG_ENDIAN)
