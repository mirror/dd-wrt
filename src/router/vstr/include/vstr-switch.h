#ifndef VSTR__HEADER_H
# error " You must _just_ #include <vstr.h>"
#endif
/*
 *  Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005  James Antill
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


#ifndef VSTR_COMPILE_ATTRIBUTES
# define VSTR_COMPILE_ATTRIBUTES 1
#endif

#ifndef VSTR_COMPILE_BUILTINS
# define VSTR_COMPILE_BUILTINS 1
#endif

#ifndef VSTR_COMPILE_INCLUDE
# define VSTR_COMPILE_INCLUDE 1
#endif

#ifndef VSTR_COMPILE_INLINE
# define VSTR_COMPILE_INLINE 1
#endif

#ifndef VSTR_COMPILE_MACRO_FUNCTIONS
# define VSTR_COMPILE_MACRO_FUNCTIONS 1
#endif

#ifndef VSTR_COMPILE_TYPEDEF
# define VSTR_COMPILE_TYPEDEF 1
#endif

/* end of user defineable switches */

#if VSTR_COMPILE_TYPEDEF
# define VSTR__DECL_TYPEDEF1(x) typedef x
# define VSTR__DECL_TYPEDEF2(x) x
#else
# define VSTR__DECL_TYPEDEF1(x) x
# define VSTR__DECL_TYPEDEF2(x) /* nothing */
#endif

/* struct hack arrays, for use in an array of the encompassing type
 * C99 doesn't allow that */
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
/*  The _SZ will be wrong if you compile your program with -ansi and *
 * the lib without ... but that doesn't matter because it's internal
 * so you shouldn't be using it anyway.
 */
# define VSTR__STRUCT_ARRAY_HACK_ARRAY(x) x[0]
# define VSTR__STRUCT_ARRAY_HACK_SZ(type) (0)
#else
# define VSTR__STRUCT_ARRAY_HACK_ARRAY(x) x[1]
# define VSTR__STRUCT_ARRAY_HACK_SZ(type) sizeof(type)
#endif

/* struct hack arrays, normal single objects */
#ifdef VSTR_AUTOCONF_HAVE_C9x_STRUCT_HACK
# define VSTR__STRUCT_HACK_ARRAY(x) x[]
# define VSTR__STRUCT_HACK_SZ(type) (0)
#else
# define VSTR__STRUCT_HACK_ARRAY(x) VSTR__STRUCT_ARRAY_HACK_ARRAY(x)
# define VSTR__STRUCT_HACK_SZ(x)    VSTR__STRUCT_ARRAY_HACK_SZ(x)
#endif

#if defined(__GNUC__) && !defined(__STRICT_ANSI__) && VSTR_COMPILE_ATTRIBUTES
# define VSTR__COMPILE_ATTR_FMT(x, y) \
 __attribute__ ((__format__ (__printf__, x, y)))
#else
# define VSTR__COMPILE_ATTR_FMT(x, y) /* nothing */
#endif

#if defined(__GNUC__) && !defined(__STRICT_ANSI__) && \
    defined(VSTR_AUTOCONF_HAVE_ATTRIB_NONNULL) && \
   !defined(__STRICT_ANSI__) && VSTR_COMPILE_ATTRIBUTES
# define VSTR__COMPILE_ATTR_NONNULL_A() \
 __attribute__ ((__nonnull__))
# define VSTR__COMPILE_ATTR_NONNULL_L(x) \
 __attribute__ ((__nonnull__ x))
#else
# define VSTR__COMPILE_ATTR_NONNULL_A() /* nothing */
# define VSTR__COMPILE_ATTR_NONNULL_L(x) /* nothing */
#endif

#if defined(__GNUC__) && !defined(__STRICT_ANSI__) && \
    defined(VSTR_AUTOCONF_HAVE_ATTRIB_PURE) && \
   !defined(__STRICT_ANSI__) && VSTR_COMPILE_ATTRIBUTES
# define VSTR__COMPILE_ATTR_PURE() \
 __attribute__ ((__pure__))
#else
# define VSTR__COMPILE_ATTR_PURE() /* nothing */
#endif

#if defined(__GNUC__) && !defined(__STRICT_ANSI__) && \
    defined(VSTR_AUTOCONF_HAVE_ATTRIB_CONST) && \
   !defined(__STRICT_ANSI__) && VSTR_COMPILE_ATTRIBUTES
# define VSTR__COMPILE_ATTR_CONST() \
 __attribute__ ((__const__))
#else
# define VSTR__COMPILE_ATTR_CONST() VSTR__COMPILE_ATTR_PURE()
#endif

#if defined(__GNUC__) && !defined(__STRICT_ANSI__) && \
    defined(VSTR_AUTOCONF_HAVE_ATTRIB_MALLOC) && \
   !defined(__STRICT_ANSI__) && VSTR_COMPILE_ATTRIBUTES
# define VSTR__COMPILE_ATTR_MALLOC() \
 __attribute__ ((__malloc__))
#else
# define VSTR__COMPILE_ATTR_MALLOC() /* nothing */
#endif

#if defined(__GNUC__) && !defined(__STRICT_ANSI__) && \
    defined(VSTR_AUTOCONF_HAVE_ATTRIB_WARN_UNUSED_RET) && \
   !defined(__STRICT_ANSI__) && VSTR_COMPILE_ATTRIBUTES
# define VSTR__COMPILE_ATTR_WARN_UNUSED_RET() \
 __attribute__ ((__warn_unused_result__))
#else
# define VSTR__COMPILE_ATTR_WARN_UNUSED_RET() /* nothing */
#endif

#if defined(__GNUC__) && !defined(__STRICT_ANSI__) && VSTR_COMPILE_BUILTINS
# define VSTR__AT_COMPILE_CONST_P(x) __builtin_constant_p (x)
# define VSTR__AT_COMPILE_STRLEN(x)                             \
    (__builtin_constant_p (x) ? __builtin_strlen (x) : strlen(x))
#else
# define VSTR__AT_COMPILE_CONST_P(x) (0)
# define VSTR__AT_COMPILE_STRLEN(x) strlen(x)
#endif

#if 0 /* what test ... autoconf? */
# define VSTR__COMPILE_STATIC_ARRAY() static
#else
# define VSTR__COMPILE_STATIC_ARRAY()
#endif

#ifdef VSTR_AUTOCONF_NDEBUG
# define VSTR_TYPE_CONST_DEBUG_1 0
# define VSTR_TYPE_CONST_DEBUG_16 0
# define VSTR_TYPE_CONST_DEBUG_32 0
#else
# define VSTR_TYPE_CONST_DEBUG_1 1
# define VSTR_TYPE_CONST_DEBUG_16 0x5555 /* ok for signed too */
# define VSTR_TYPE_CONST_DEBUG_32 0x55555555 /* ok for signed too */
#endif

#ifndef VSTR_AUTOCONF_intmax_t
# define VSTR_AUTOCONF_intmax_t intmax_t
#endif

#ifndef VSTR_AUTOCONF_uintmax_t
# define VSTR_AUTOCONF_uintmax_t uintmax_t
#endif

#ifndef VSTR_AUTOCONF_uint_least16_t
# define VSTR_AUTOCONF_uint_least16_t uint_least16_t
#endif

#ifndef VSTR_AUTOCONF_uint_least32_t
# define VSTR_AUTOCONF_uint_least32_t uint_least32_t
#endif

#ifndef VSTR_AUTOCONF_mode_t
# define VSTR_AUTOCONF_mode_t mode_t
/* always include types.h */
#endif

#ifndef VSTR_AUTOCONF_off64_t
# define VSTR_AUTOCONF_off64_t off64_t
/* always include types.h */
#endif
