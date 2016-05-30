/* src/toolbox/endianess.hpp - utilities for reading/writing to/from a 
							   buffer in little or big endian

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

#ifndef CACAO_ENDIANESS_
#define CACAO_ENDIANESS_ 1

#include "mm/memory.hpp"
#include "toolbox/assert.hpp"
#include "vm/types.hpp"

namespace cacao {
	// ***** READ UNSIGNED, LITTLE ENDIAN **************************************

#if defined(__I386__) || defined(__X86_64__)

	// we can optimize the LE access on little endian machines without alignment

	static inline uint8_t  read_u1_le(const uint8_t *src) { return *src; }
	static inline uint16_t read_u2_le(const uint8_t *src) { return *((uint16_t*) src); }
	static inline uint32_t read_u4_le(const uint8_t *src) { return *((uint32_t*) src); }
	static inline uint64_t read_u8_le(const uint8_t *src) { return *((uint64_t*) src); }

#else // defined(__I386__) || defined(__X86_64__)

	static inline uint8_t read_u1_le(const uint8_t *src) {
		return *src;
	}

	static inline uint16_t read_u2_le(const uint8_t *src) {
		return (((uint16_t) src[1]) << 8) +
		        ((uint16_t) src[0]);
	}

	static inline uint32_t read_u4_le(const uint8_t *src) {
		return (((uint32_t) src[3]) << 24) +
		       (((uint32_t) src[2]) << 16) +
		       (((uint32_t) src[1]) <<  8) +
		        ((uint32_t) src[0]);
	}

	static inline uint64_t read_u8_le(const uint8_t *src) {
		return (((uint64_t) src[7]) << 56) +
		       (((uint64_t) src[6]) << 48) +
		       (((uint64_t) src[5]) << 40) +
		       (((uint64_t) src[4]) << 32) +
		       (((uint64_t) src[3]) << 24) +
		       (((uint64_t) src[2]) << 16) +
		       (((uint64_t) src[1]) <<  8) +
		        ((uint64_t) src[0]);
	}

#endif // defined(__I386__) || defined(__X86_64__)

	// ***** READ SIGNED, LITTLE ENDIAN ****************************************

	/// read int8_t from pointer little endian
	static inline int8_t  read_s1_le(const uint8_t *src) { return (int8_t) read_u1_le(src); }

	/// read int16_t from pointer little endian
	static inline int16_t read_s2_le(const uint8_t *src) { return (int16_t) read_u2_le(src); }

	/// read int32_t from pointer little endian
	static inline int32_t read_s4_le(const uint8_t *src) { return (int32_t) read_u4_le(src); }

	/// read int64_t from pointer little endian
	static inline int64_t read_s8_le(const uint8_t *src) { return (int64_t) read_u8_le(src); }

	// ***** READ UNSIGNED, BIG ENDIAN (for Java class files ) *****************

	/// read uint8_t from pointer big endian
	static inline uint8_t read_u1_be(const uint8_t *src) {
		return *src;
	}

	/// read uint16_t from pointer big endian
	static inline uint16_t read_u2_be(const uint8_t *src) {
		return (((uint16_t) src[0]) << 8) +
		        ((uint16_t) src[1]);
	}

	/// read uint32_t from pointer big endian
	static inline uint32_t read_u4_be(const uint8_t *src) {
		return (((uint32_t) src[0]) << 24) +
		       (((uint32_t) src[1]) << 16) +
		       (((uint32_t) src[2]) <<  8) +
		        ((uint32_t) src[3]);
	}

	/// read uint64_t from pointer big endian
	static inline uint64_t read_u8_be(const uint8_t *src) {
		return (((uint64_t) src[0]) << 56) +
		       (((uint64_t) src[1]) << 48) +
		       (((uint64_t) src[2]) << 40) +
		       (((uint64_t) src[3]) << 32) +
		       (((uint64_t) src[4]) << 24) +
		       (((uint64_t) src[5]) << 16) +
		       (((uint64_t) src[6]) <<  8) +
		        ((uint64_t) src[7]);
	}

	// ***** READ SIGNED, BIG ENDIAN ************************

	/// read int8_t from pointer big endian
	static inline int8_t  read_s1_be(const uint8_t *src) { return (int8_t)  read_u1_be(src); }

	/// read int16_t from pointer big endian
	static inline int16_t read_s2_be(const uint8_t *src) { return (int16_t) read_u2_be(src); }

	/// read int32_t from pointer big endian
	static inline int32_t read_s4_be(const uint8_t *src) { return (int32_t) read_u4_be(src); }

	/// read int64_t from pointer big endian
	static inline int64_t read_s8_be(const uint8_t *src) { return (int64_t) read_u8_be(src); }

	// ***** READ FLOATING POINT, BIG ENDIAN ****************

	static inline float read_float_be(const uint8_t *src) {
		EXPENSIVE_ASSERT(sizeof(float) == 4);
		
		float f;

	#if WORDS_BIGENDIAN == 0
		uint8_t  buffer[4];

		for (int i = 0; i < 4; i++)
			buffer[3 - i] = read_u1_be(src + i);

		MCOPY((uint8_t *) (&f), buffer, u1, 4);
	#else
		MCOPY((uint8_t *) (&f), src,    u1, 4);
	#endif
		
		return f;
	}

	static inline double  read_double_be(const uint8_t *src) {
		EXPENSIVE_ASSERT(sizeof(double) == 8);

		double d;

	#if WORDS_BIGENDIAN == 0
		uint8_t buffer[8];

	# if defined(__ARM__) && defined(__ARMEL__) && !defined(__VFP_FP__)
		// On little endian ARM processors when using FPA, word order
		// of doubles is still big endian. So take that into account
		// here. When using VFP, word order of doubles follows byte
		// order. (michi 2005/07/24)

		for (int i = 0; i < 4; i++)
			buffer[3 - i] = read_u1_be(src + i);
		for (int i = 0; i < 4; i++)
			buffer[7 - i] = read_u1_be(src + i + 4);
	# else
		for (int i = 0; i < 8; i++)
			buffer[7 - i] = read_u1_be(src + i);
	# endif

	  MCOPY((uint8_t *) (&d), buffer, u1, 8);
	#else 
	  MCOPY((uint8_t *) (&d), src,    u1, 8);
	#endif

		return d;
	}
} // end namespace cacao

#endif // CACAO_ENDIANESS_

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
