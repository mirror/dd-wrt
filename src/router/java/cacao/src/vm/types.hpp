/* src/vm/types.hpp - type definitions for CACAO's internal types

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/


#ifndef CACAO_TYPES_HPP_
#define CACAO_TYPES_HPP_ 1

#include "config.h"

#include <stdint.h>


/* In this file we check for unknown pointersizes, so we don't have to
   do this somewhere else. */

/* Define the sizes of the integer types used internally by CACAO. ************/

typedef int8_t            s1;
typedef uint8_t           u1;

typedef int16_t           s2;
typedef uint16_t          u2;

typedef int32_t           s4;
typedef uint32_t          u4;

typedef int64_t           s8;
typedef uint64_t          u8;


/* Define the size of a function pointer used in function pointer casts. ******/

typedef uintptr_t                      ptrint;

#endif // CACAO_TYPES_HPP_


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */
