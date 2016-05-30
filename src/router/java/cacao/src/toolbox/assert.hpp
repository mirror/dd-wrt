/* src/toolbox/assert.hpp - Additional assertion macros

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


#ifndef ASSERT_HPP_
#define ASSERT_HPP_ 1

#include <cassert>        // for assert
#include "config.h"       // for HAS_BUILTIN_STATIC_ASSERT, ENABLE_EXPENSIVE_ASSERT

/**
 * @file
 * Additional assertion macros
 */

/**
 * @def STATIC_ASSERT(EXPR, MSG)
 * An assertion that is evaluated at compile time.
 *
 * The evaluated expression must be a compile time constant.
 *
 * If possible this uses the C++11 static_assert feature, otherwise it uses
 * a fallback that produces substantially worse error messages.
 *
 * A static assert can appear anywhere a typedef can, that is, in global,
 * class or function scope.
 * In order to support disabling static assertions STATIC_ASSERT does not require a trailing ';'
 *
 * An example:
 *    @code
 *       STATIC_ASSERT(1 + 1 == 2, "Math is hard") // compiles without error
 *       STATIC_ASSERT(5 > 6,      "Math is hard") // causes a compilation error
 *    @endcode
 *
 * @remark The fallback version cannot handle multiple STATIC_ASSERTs on one line.
 */
#ifdef HAS_BUILTIN_STATIC_ASSERT
#	define STATIC_ASSERT(EXPR, MSG) static_assert(EXPR, MSG);
#else
#	define STATIC_ASSERT(EXPR, MSG)                                            \
		typedef ::cacao::static_assert_test<                                    \
			sizeof(::cacao::STATIC_ASSERTION_FAILURE<static_cast<bool>((EXPR))>) \
		> CONCAT(static_assert_, __LINE__) __attribute__((unused));

	namespace cacao {
		template <bool expr> struct STATIC_ASSERTION_FAILURE;

		template <> struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };

		// STATIC_ASSERTION_FAILURE<false> is undefined, that's the whole trick

		template<int expr> struct static_assert_test {};
	}
#	define CONCAT(A, B)   CONCAT_(A, B)
#	define CONCAT_(A, B)  A ## B
#endif


/**
 * @def EXPENSIVE_ASSERT(EXPR)
 * An assertion that performs computations too expensive even for a normal
 * debug build.
 *
 * EXPENSIVE_ASSERT is even disabled in a normal debug build.
 * You can enable these assertions explicitly by configuring CACAO with:
 *  --enable-expensive-assert
 */
#ifdef ENABLE_EXPENSIVE_ASSERT
#	define EXPENSIVE_ASSERT(EXPR) assert(EXPR)
#else
#	define EXPENSIVE_ASSERT(EXPR) (static_cast<void>(0))
#endif


#endif // ASSERT_HPP_

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
