
/*
 * The olsr.org Optimized Link-State Routing daemon version 2 (olsrd2)
 * Copyright (c) 2004-2015, the olsr.org team - see HISTORY file
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

/**
 * @file
 */

#ifndef COMMON_TYPES_H_
#define COMMON_TYPES_H_

#include <stddef.h>

/*
 * This line forces gcc NOT to demand memcpy with glibc version 2.14
 * google for the memcpy/memmove debacle with gcc and glibc.
 */
// __asm__(".symver memcpy,memcpy@GLIBC_2.2.5");

#ifndef EXPORT
/*! Macro to declare a function should be visible for other subsystems */
#define EXPORT __attribute__((visibility ("default")))
#endif

/* give everyone an arraysize implementation */
#ifndef ARRAYSIZE
/**
 * @param a reference of an array (not a pointer!)
 * @returns number of elements in array
 */
#define ARRAYSIZE(a)  (sizeof(a) / sizeof(*(a)))
#endif

#ifndef STRINGIFY
/*! converts the parameter of the macro into a string */
#define STRINGIFY(x) #x
#endif

/*
 * This force gcc to always inline, which prevents errors
 * with option -Os
 */
#ifndef INLINE
#ifdef __GNUC__
/*! force inlining with GCC */
#define INLINE inline __attribute__((always_inline))
#else
/*! default to normal inlining */
#define INLINE inline
#endif
#endif

/* printf size_t modifiers*/
#if defined(__GNUC__)
#define PRINTF_SIZE_T_SPECIFIER     "zu"
#define PRINTF_SIZE_T_HEX_SPECIFIER "zx"
#define PRINTF_SSIZE_T_SPECIFIER    "zd"
#define PRINTF_PTRDIFF_T_SPECIFIER  "zd"
#else
/* maybe someone can check what to do about LLVM/Clang? */
#error Please implement size_t modifiers
#endif

#include <limits.h>

/* we have C99 ? */
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#include <inttypes.h>
#include <stdbool.h>
#else
#error "OONF needs C99"
#endif /* __STDC_VERSION__ && __STDC_VERSION__ >= 199901L */

#endif /* COMMON_TYPES_H_ */
