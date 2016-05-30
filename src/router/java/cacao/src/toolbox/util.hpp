/* src/toolbox/util.hpp - contains some utility functions

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


#ifndef UTIL_HPP_
#define UTIL_HPP_ 1

#include <config.h>
#include <cstdarg>

#include "toolbox/assert.hpp"

/* function prototypes ********************************************************/

int get_variable_message_length(const char *fmt, va_list ap);

/**
 * find the smallest power of two >= n
 */
static inline size_t next_power_of_two(size_t n) {
   // the 'n--' and 'n++' make sure that a power of two is mapped to itself

	n--;

	// this must be repeated log2(bits in size_t) times
	// i.e. for 5 times for 32 bit, 6 times for 64 bit

	n = (n >>  1) | n;
	n = (n >>  2) | n;
	n = (n >>  4) | n;
	n = (n >>  8) | n;
	n = (n >> 16) | n;
#if SIZEOF_VOID_P == 8
	n = (n >> 32) | n;
#endif

	n++;

	return n;
}

static inline bool is_power_of_two(size_t n) {
	return (n & (n - 1)) == 0;
}

/**
 * fast computation of n % m.
 *
 * m MUST be a power of two and greater than zero
 */
static inline size_t fast_modulo(size_t n, size_t modul) {
   EXPENSIVE_ASSERT(modul > 0);
   EXPENSIVE_ASSERT(is_power_of_two(modul));

	return n & (modul - 1);
}

/**
 * Perform unsigned integer division.
 * But instead of rounding the result down, round it up.
 */
static inline size_t divide_rounding_up(size_t a, size_t b) {
   return (a + b - 1) / b;
}


#endif // UTIL_HPP_


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
 * vim:noexpandtab:sw=4:ts=4:
 */
