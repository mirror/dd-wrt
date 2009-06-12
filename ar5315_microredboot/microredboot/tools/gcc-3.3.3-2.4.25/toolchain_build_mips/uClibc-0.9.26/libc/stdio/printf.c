/*  Copyright (C) 2002, 2003     Manuel Novoa III
 *  My stdio library for linux and (soon) elks.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* This code needs a lot of clean up.  Some of that is on hold until uClibc
 * gets a better configuration system (on Erik's todo list).
 * The other cleanup will take place during the implementation/integration of
 * the wide char (un)formatted i/o functions which I'm currently working on.
 */

/*  ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!
 *
 *  This code is currently under development.  Also, I plan to port
 *  it to elks which is a 16-bit environment with a fairly limited
 *  compiler.  Therefore, please refrain from modifying this code
 *  and, instead, pass any bug-fixes, etc. to me.  Thanks.  Manuel
 *
 *  ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION! */


/* April 1, 2002
 * Initialize thread locks for fake files in vsnprintf and vdprintf.
 *    reported by Erik Andersen (andersen@codepoet.com)
 * Fix an arg promotion handling bug in _do_one_spec for %c. 
 *    reported by Ilguiz Latypov <ilatypov@superbt.com>
 *
 * May 10, 2002
 * Remove __isdigit and use new ctype.h version.
 * Add conditional setting of QUAL_CHARS for size_t and ptrdiff_t.
 *
 * Aug 16, 2002
 * Fix two problems that showed up with the python 2.2.1 tests; one
 *    involving %o and one involving %f.
 *
 * Oct 28, 2002
 * Fix a problem in vasprintf (reported by vodz a while back) when built
 *    without custom stream support.  In that case, it is necessary to do
 *    a va_copy.
 * Make sure each va_copy has a matching va_end, as required by C99.
 *
 * Nov 4, 2002
 * Add locale-specific grouping support for integer decimal conversion.
 * Add locale-specific decimal point support for floating point conversion.
 *   Note: grouping will have to wait for _dtostr() rewrite.
 * Add printf wchar support for %lc (%C) and %ls (%S).
 * Require printf format strings to be valid multibyte strings beginning and
 *   ending in their initial shift state, as per the stds.
 *
 * Nov 21, 2002
 * Add *wprintf functions.  Currently they don't support floating point
 *   conversions.  That will wait until the rewrite of _dtostr.
 *
 * Aug 1, 2003
 * Optional hexadecimal float notation support for %a/%A.
 * Floating point output now works for *wprintf.
 * Support for glibc locale-specific digit grouping for floats.
 * Misc bug fixes.
 *
 * Aug 31, 2003
 * Fix precision bug for %g conversion specifier when using %f style.
 *
 * Sep 5, 2003
 * Implement *s*scanf for the non-buffered stdio case with old_vfprintf.
 *
 * Sep 23, 2003
 * vfprintf was not always checking for narrow stream orientation.
 */

/* TODO:
 *
 * Should we validate that *printf format strings are valid multibyte
 *   strings in the current locale?  ANSI/ISO C99 seems to imply this
 *   and Plauger's printf implementation in his Standard C Library book
 *   treats this as an error.
 */


#define _ISOC99_SOURCE			/* for ULLONG primarily... */
#define _GNU_SOURCE
#define _STDIO_UTILITY			/* We're using _uintmaxtostr. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <assert.h>
#include <stdint.h>
#include <errno.h>
#include <locale.h>

#define __PRINTF_INFO_NO_BITFIELD
#include <printf.h>

#ifdef __STDIO_THREADSAFE
#include <stdio_ext.h>
#include <pthread.h>
#endif /* __STDIO_THREADSAFE */

#ifdef __UCLIBC_HAS_WCHAR__
#include <wchar.h>
#endif /* __UCLIBC_HAS_WCHAR__ */

/* Some older or broken gcc toolchains define LONG_LONG_MAX but not
 * LLONG_MAX.  Since LLONG_MAX is part of the standard, that's what
 * we use.  So complain if we do not have it but should.
 */
#if !defined(LLONG_MAX) && defined(LONG_LONG_MAX)
#error Apparently, LONG_LONG_MAX is defined but LLONG_MAX is not.  You need to fix your toolchain headers to support the standard macros for (unsigned) long long.
#endif

/**********************************************************************/
/* These provide some control over printf's feature set */

/* This is undefined below depeding on uClibc's configuration. */
#define __STDIO_PRINTF_FLOAT 1

/* Now controlled by uClibc_stdio.h. */
/* #define __STDIO_PRINTF_M_SUPPORT */


/**********************************************************************/

#if defined(__UCLIBC__) && !defined(__UCLIBC_HAS_FLOATS__)
#undef __STDIO_PRINTF_FLOAT
#endif

#ifdef __BCC__
#undef __STDIO_PRINTF_FLOAT
#endif

#ifdef __STDIO_PRINTF_FLOAT
#include <float.h>
#include <bits/uClibc_fpmax.h>
#else  /* __STDIO_PRINTF_FLOAT */
#undef L__fpmaxtostr
#endif /* __STDIO_PRINTF_FLOAT */


#undef __STDIO_HAS_VSNPRINTF
#if defined(__STDIO_BUFFERS) || defined(__USE_OLD_VFPRINTF__) || defined(__STDIO_GLIBC_CUSTOM_STREAMS)
#define __STDIO_HAS_VSNPRINTF 1
#endif

/**********************************************************************/

/* Now controlled by uClibc_stdio.h. */
/* #define __STDIO_GLIBC_CUSTOM_PRINTF */

/* TODO -- move these to a configuration section? */
#define MAX_FIELD_WIDTH		4095

#ifdef __UCLIBC_MJN3_ONLY__
#ifdef L_register_printf_function
/* emit only once */
#warning WISHLIST: Make MAX_USER_SPEC configurable?
#warning WISHLIST: Make MAX_ARGS_PER_SPEC configurable?
#endif
#endif /* __UCLIBC_MJN3_ONLY__ */

#ifdef __STDIO_GLIBC_CUSTOM_PRINTF

#define MAX_USER_SPEC       10
#define MAX_ARGS_PER_SPEC    5

#else  /* __STDIO_GLIBC_CUSTOM_PRINTF */

#undef MAX_USER_SPEC
#define MAX_ARGS_PER_SPEC    1

#endif /* __STDIO_GLIBC_CUSTOM_PRINTF */

#if MAX_ARGS_PER_SPEC < 1
#error MAX_ARGS_PER_SPEC < 1!
#undef MAX_ARGS_PER_SPEC
#define MAX_ARGS_PER_SPEC    1
#endif

#if defined(NL_ARGMAX) && (NL_ARGMAX < 9)
#error NL_ARGMAX < 9!
#endif

#if defined(NL_ARGMAX) && (NL_ARGMAX >= (MAX_ARGS_PER_SPEC + 2))
#define MAX_ARGS        NL_ARGMAX
#else
/* N for spec itself, plus 1 each for width and precision */
#define MAX_ARGS        (MAX_ARGS_PER_SPEC + 2)
#endif


/**********************************************************************/
/* Deal with pre-C99 compilers. */

#ifndef va_copy

#ifdef __va_copy
#define va_copy(A,B)	__va_copy(A,B)
#else
	/* TODO -- maybe create a bits/vacopy.h for arch specific versions
	 * to ensure we get the right behavior?  Either that or fall back
	 * on the portable (but costly in size) method of using a va_list *.
	 * That means a pointer derefs in the va_arg() invocations... */
#warning Neither va_copy (C99/SUSv3) or __va_copy is defined.  Using a simple copy instead.  But you should really check that this is appropriate...
	/* the glibc manual suggests that this will usually suffice when
        __va_copy doesn't exist.  */
#define va_copy(A,B)	A = B
#endif

#endif /* va_copy */

/**********************************************************************/

#define __PA_FLAG_INTMASK \
	(__PA_FLAG_CHAR|PA_FLAG_SHORT|__PA_FLAG_INT|PA_FLAG_LONG|PA_FLAG_LONG_LONG)

#ifdef __STDIO_GLIBC_CUSTOM_PRINTF
extern printf_function _custom_printf_handler[MAX_USER_SPEC];
extern printf_arginfo_function *_custom_printf_arginfo[MAX_USER_SPEC];
extern char *_custom_printf_spec;
#endif /* __STDIO_GLIBC_CUSTOM_PRINTF */

/**********************************************************************/

#define SPEC_FLAGS		" +0-#'I"
enum {
	FLAG_SPACE		=	0x01,
	FLAG_PLUS		=	0x02,	/* must be 2 * FLAG_SPACE */
	FLAG_ZERO		=	0x04,
	FLAG_MINUS		=	0x08,	/* must be 2 * FLAG_ZERO */
	FLAG_HASH		=	0x10,
	FLAG_THOUSANDS	=	0x20,
	FLAG_I18N		=	0x40,	/* only works for d, i, u */
	FLAG_WIDESTREAM =   0x80
};	  

/**********************************************************************/

/* float layout          01234567890123456789   TODO: B?*/
#define SPEC_CHARS		"npxXoudifFeEgGaACScs"
enum {
	CONV_n = 0,
	CONV_p,
	CONV_x, CONV_X,	CONV_o,	CONV_u,	CONV_d,	CONV_i,
	CONV_f, CONV_F, CONV_e, CONV_E, CONV_g, CONV_G, CONV_a, CONV_A,
	CONV_C, CONV_S, CONV_c, CONV_s,
#ifdef __STDIO_PRINTF_M_SUPPORT
	CONV_m,
#endif
	CONV_custom0				/* must be last */
};

/*                         p   x   X  o   u   d   i */
#define SPEC_BASE		{ 16, 16, 16, 8, 10, 10, 10 }

#define SPEC_RANGES		{ CONV_n, CONV_p, CONV_i, CONV_A, \
						  CONV_C, CONV_S, CONV_c, CONV_s, CONV_custom0 }

#define SPEC_OR_MASK		 { \
	/* n */			(PA_FLAG_PTR|PA_INT), \
	/* p */			PA_POINTER, \
	/* oxXudi */	PA_INT, \
	/* fFeEgGaA */	PA_DOUBLE, \
	/* C */			PA_WCHAR, \
	/* S */			PA_WSTRING, \
	/* c */			PA_CHAR, \
	/* s */			PA_STRING, \
}

#define SPEC_AND_MASK		{ \
	/* n */			(PA_FLAG_PTR|__PA_INTMASK), \
	/* p */			PA_POINTER, \
	/* oxXudi */	(__PA_INTMASK), \
	/* fFeEgGaA */	(PA_FLAG_LONG_DOUBLE|PA_DOUBLE), \
	/* C */			(PA_WCHAR), \
	/* S */			(PA_WSTRING), \
	/* c */			(PA_CHAR), \
	/* s */			(PA_STRING), \
}

/**********************************************************************/
/*
 * In order to ease translation to what arginfo and _print_info._flags expect,
 * we map:  0:int  1:char  2:longlong 4:long  8:short
 * and then _flags |= (((q << 7) + q) & 0x701) and argtype |= (_flags & 0x701)
 */

/* TODO -- Fix the table below to take into account stdint.h. */
/*  #ifndef LLONG_MAX */
/*  #error fix QUAL_CHARS for no long long!  Affects 'L', 'j', 'q', 'll'. */
/*  #else */
/*  #if LLONG_MAX != INTMAX_MAX */
/*  #error fix QUAL_CHARS intmax_t entry 'j'! */
/*  #endif */
/*  #endif */

#ifdef PDS
#error PDS already defined!
#endif
#ifdef SS
#error SS already defined!
#endif
#ifdef IMS
#error IMS already defined!
#endif

#if PTRDIFF_MAX == INT_MAX
#define PDS		0
#elif PTRDIFF_MAX == LONG_MAX
#define PDS		4
#elif defined(LLONG_MAX) && (PTRDIFF_MAX == LLONG_MAX)
#define PDS		8
#else
#error fix QUAL_CHARS ptrdiff_t entry 't'!
#endif

#if SIZE_MAX == UINT_MAX
#define SS		0
#elif SIZE_MAX == ULONG_MAX
#define SS		4
#elif defined(LLONG_MAX) && (SIZE_MAX == ULLONG_MAX)
#define SS		8
#else
#error fix QUAL_CHARS size_t entries 'z', 'Z'!
#endif

#if INTMAX_MAX == INT_MAX
#define IMS		0
#elif INTMAX_MAX == LONG_MAX
#define IMS		4
#elif defined(LLONG_MAX) && (INTMAX_MAX == LLONG_MAX)
#define IMS		8
#else
#error fix QUAL_CHARS intmax_t entry 'j'!
#endif

#define QUAL_CHARS		{ \
	/* j:(u)intmax_t z:(s)size_t  t:ptrdiff_t  \0:int */ \
	/* q:long_long  Z:(s)size_t */ \
	'h',   'l',  'L',  'j',  'z',  't',  'q', 'Z',  0, \
	 2,     4,    8,  IMS,   SS,  PDS,    8,  SS,   0, /* TODO -- fix!!! */\
     1,     8 \
}

/**********************************************************************/

#ifdef __STDIO_VA_ARG_PTR
#ifdef __BCC__
#define __va_arg_ptr(ap,type)		(((type *)(ap += sizeof(type))) - 1)
#endif

#if 1
#ifdef __GNUC__
/* TODO -- need other than for 386 as well! */

#ifndef __va_rounded_size
#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))
#endif
#define __va_arg_ptr(AP, TYPE)						\
 (AP = (va_list) ((char *) (AP) + __va_rounded_size (TYPE)),	\
  ((void *) ((char *) (AP) - __va_rounded_size (TYPE))))
#endif
#endif
#endif /* __STDIO_VA_ARG_PTR */

#ifdef __va_arg_ptr
#define GET_VA_ARG(AP,F,TYPE,ARGS)	(*(AP) = __va_arg_ptr(ARGS,TYPE))
#define GET_ARG_VALUE(AP,F,TYPE)	(*((TYPE *)(*(AP))))
#else
typedef union {
	wchar_t wc;
	unsigned int u;
	unsigned long ul;
#ifdef ULLONG_MAX
	unsigned long long ull;
#endif
#ifdef __STDIO_PRINTF_FLOAT
	double d;
	long double ld;
#endif /* __STDIO_PRINTF_FLOAT */
	void *p;
} argvalue_t;

#define GET_VA_ARG(AU,F,TYPE,ARGS)	(AU->F = va_arg(ARGS,TYPE))
#define GET_ARG_VALUE(AU,F,TYPE)	((TYPE)((AU)->F))
#endif

typedef struct {
	const char *fmtpos;			/* TODO: move below struct?? */
	struct printf_info info;
#ifdef NL_ARGMAX
	int maxposarg;				/* > 0 if args are positional, 0 if not, -1 if unknown */
#endif /* NL_ARGMAX */
	int num_data_args;			/* TODO: use sentinal??? */
	unsigned int conv_num;
	unsigned char argnumber[4]; /* width | prec | 1st data | unused */
	int argtype[MAX_ARGS];
	va_list arg;
#ifdef __va_arg_ptr
	void *argptr[MAX_ARGS];
#else
/* if defined(NL_ARGMAX) || defined(__STDIO_GLIBC_CUSTOM_PRINTF) */
	/* While this is wasteful of space in the case where pos args aren't
	 * enabled, it is also needed to support custom printf handlers. */
	argvalue_t argvalue[MAX_ARGS];
#endif
} ppfs_t;						/* parse printf format state */

/**********************************************************************/

/* TODO: fix printf to return 0 and set errno if format error.  Standard says
   only returns -1 if sets error indicator for the stream. */

#ifdef __STDIO_PRINTF_FLOAT
typedef void (__fp_outfunc_t)(FILE *fp, intptr_t type, intptr_t len,
							  intptr_t buf);

extern size_t _fpmaxtostr(FILE * fp, __fpmax_t x, struct printf_info *info,
						  __fp_outfunc_t fp_outfunc);
#endif

extern int _ppfs_init(ppfs_t *ppfs, const char *fmt0); /* validates */
extern void _ppfs_prepargs(ppfs_t *ppfs, va_list arg); /* sets posargptrs */
extern void _ppfs_setargs(ppfs_t *ppfs); /* sets argptrs for current spec */
extern int _ppfs_parsespec(ppfs_t *ppfs); /* parses specifier */

extern void _store_inttype(void *dest, int desttype, uintmax_t val);
extern uintmax_t _load_inttype(int desttype, const void *src, int uflag);

/**********************************************************************/
#ifdef L_parse_printf_format

/* NOTE: This function differs from the glibc version in that parsing stops
 * upon encountering an invalid conversion specifier.  Since this is the way
 * my printf functions work, I think it makes sense to do it that way here.
 * Unfortunately, since glibc sets the return type as size_t, we have no way
 * of returning that the template is illegal, other than returning 0.
 */

size_t parse_printf_format(register const char *template,
						   size_t n, register int *argtypes)
{
	ppfs_t ppfs;
	size_t i;
	size_t count = 0;

	if (_ppfs_init(&ppfs, template) >= 0) {
#ifdef NL_ARGMAX
		if (ppfs.maxposarg > 0)  { /* Using positional args. */
			count = ppfs.maxposarg;
			if (n > count) {
				n = count;
			}
			for (i = 0 ; i < n ; i++) {
				*argtypes++ = ppfs.argtype[i];
			}
		} else {				/* Not using positional args. */
#endif /* NL_ARGMAX */
			while (*template) {
				if ((*template == '%') && (*++template != '%')) {
					ppfs.fmtpos = template;
					_ppfs_parsespec(&ppfs); /* Can't fail. */
					template = ppfs.fmtpos; /* Update to one past spec end. */
					if (ppfs.info.width == INT_MIN) {
						++count;
						if (n > 0) {
							*argtypes++ = PA_INT;
							--n;
						}
					}
					if (ppfs.info.prec == INT_MIN) {
						++count;
						if (n > 0) {
							*argtypes++ = PA_INT;
							--n;
						}
					}
					for (i = 0 ; i < ppfs.num_data_args ; i++) {
						if ((ppfs.argtype[i]) != __PA_NOARG) {
							++count;
							if (n > 0) {
								*argtypes++ = ppfs.argtype[i];
								--n;
							}
						}
					}
				} else {
					++template;
				}
			}
#ifdef NL_ARGMAX
		}
#endif /* NL_ARGMAX */
	}

	return count;
}

#endif
/**********************************************************************/
#ifdef L__ppfs_init

int _ppfs_init(register ppfs_t *ppfs, const char *fmt0)
{
	int r;

	/* First, zero out everything... argnumber[], argtype[], argptr[] */
	memset(ppfs, 0, sizeof(ppfs_t)); /* TODO: nonportable???? */
#ifdef NL_ARGMAX
	--ppfs->maxposarg;			/* set to -1 */
#endif /* NL_ARGMAX */
	ppfs->fmtpos = fmt0;
#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Make checking of the format string in C locale an option.
#endif
#ifdef __UCLIBC_HAS_LOCALE__
	/* To support old programs, don't check mb validity if in C locale. */
	if (((__UCLIBC_CURLOCALE_DATA).encoding) != __ctype_encoding_7_bit) {
		/* ANSI/ISO C99 requires format string to be a valid multibyte string
		 * beginning and ending in its initial shift state. */
		static const char invalid_mbs[] = "Invalid multibyte format string.";
		mbstate_t mbstate;
		const char *p;
		mbstate.mask = 0;	/* Initialize the mbstate. */
		p = fmt0;
		if (mbsrtowcs(NULL, &p, SIZE_MAX, &mbstate) == ((size_t)(-1))) {
			ppfs->fmtpos = invalid_mbs;
			return -1;
		}
	}
#endif /* __UCLIBC_HAS_LOCALE__ */
	/* now set all argtypes to no-arg */
	{
#if 1
		/* TODO - use memset here since already "paid for"? */
		register int *p = ppfs->argtype;
		
		r = MAX_ARGS;
		do {
			*p++ = __PA_NOARG;
		} while (--r);
#else
		/* TODO -- get rid of this?? */
		register char *p = (char *) ((MAX_ARGS-1) * sizeof(int));

		do {
			*((int *)(((char *)ppfs) + ((int)p) + offsetof(ppfs_t,argtype))) = __PA_NOARG;
			p -= sizeof(int);
		} while (p);
#endif
	}

	/*
	 * Run through the entire format string to validate it and initialize
	 * the positional arg numbers (if any).
	 */
	{
		register const char *fmt = fmt0;

		while (*fmt) {
			if ((*fmt == '%') && (*++fmt != '%')) {
				ppfs->fmtpos = fmt; /* back up to the '%' */
				if ((r = _ppfs_parsespec(ppfs)) < 0) {
					return -1;
				}
				fmt = ppfs->fmtpos;	/* update to one past end of spec */
			} else {
				++fmt;
			}
		}
		ppfs->fmtpos = fmt0;		/* rewind */
	}

#ifdef NL_MAX_ARG
	/* If we have positional args, make sure we know all the types. */
	{
		register int *p = ppfs->argtype;
		r = ppfs->maxposarg;
		while (--r >= 0) {
			if ( *p == __PA_NOARG ) { /* missing arg type!!! */
				return -1;
			}
			++p;
		}
	}
#endif /* NL_MAX_ARG */

	return 0;
}
#endif
/**********************************************************************/
#ifdef L__ppfs_prepargs
void _ppfs_prepargs(register ppfs_t *ppfs, va_list arg)
{
	int i;

	va_copy(ppfs->arg, arg);

#ifdef NL_ARGMAX
	if ((i = ppfs->maxposarg) > 0)  { /* init for positional args */
		ppfs->num_data_args = i;
		ppfs->info.width = ppfs->info.prec = ppfs->maxposarg = 0;
		_ppfs_setargs(ppfs);
		ppfs->maxposarg = i;
	}
#endif /* NL_ARGMAX */
}
#endif
/**********************************************************************/
#ifdef L__ppfs_setargs

void _ppfs_setargs(register ppfs_t *ppfs)
{
#ifdef __va_arg_ptr
	register void **p = ppfs->argptr;
#else
	register argvalue_t *p = ppfs->argvalue;
#endif
	int i;

#ifdef NL_ARGMAX
	if (ppfs->maxposarg == 0) {	/* initing for or no pos args */
#endif /* NL_ARGMAX */
		if (ppfs->info.width == INT_MIN) {
			ppfs->info.width =
#ifdef __va_arg_ptr
				*(int *)
#endif
				GET_VA_ARG(p,u,unsigned int,ppfs->arg);
		} 
		if (ppfs->info.prec == INT_MIN) {
			ppfs->info.prec =
#ifdef __va_arg_ptr
				*(int *)
#endif
				GET_VA_ARG(p,u,unsigned int,ppfs->arg);
		}
		i = 0;
		while (i < ppfs->num_data_args) {
			switch(ppfs->argtype[i++]) {
				case (PA_INT|PA_FLAG_LONG_LONG):
#ifdef ULLONG_MAX
					GET_VA_ARG(p,ull,unsigned long long,ppfs->arg);
					break;
#endif
				case (PA_INT|PA_FLAG_LONG):
#if ULONG_MAX != UINT_MAX
					GET_VA_ARG(p,ul,unsigned long,ppfs->arg);
					break;
#endif
				case PA_CHAR:	/* TODO - be careful */
 					/* ... users could use above and really want below!! */
				case (PA_INT|__PA_FLAG_CHAR):/* TODO -- translate this!!! */
				case (PA_INT|PA_FLAG_SHORT):
				case PA_INT:
					GET_VA_ARG(p,u,unsigned int,ppfs->arg);
					break;
				case PA_WCHAR:	/* TODO -- assume int? */
					/* we're assuming wchar_t is at least an int */
					GET_VA_ARG(p,wc,wchar_t,ppfs->arg);
					break;
#ifdef __STDIO_PRINTF_FLOAT
					/* PA_FLOAT */
				case PA_DOUBLE:
					GET_VA_ARG(p,d,double,ppfs->arg);
					break;
				case (PA_DOUBLE|PA_FLAG_LONG_DOUBLE):
					GET_VA_ARG(p,ld,long double,ppfs->arg);
					break;
#else  /* __STDIO_PRINTF_FLOAT */
				case PA_DOUBLE:
				case (PA_DOUBLE|PA_FLAG_LONG_DOUBLE):
					assert(0);
					continue;
#endif /* __STDIO_PRINTF_FLOAT */
				default:
					/* TODO -- really need to ensure this can't happen */
					assert(ppfs->argtype[i-1] & PA_FLAG_PTR);
				case PA_POINTER:
				case PA_STRING:
				case PA_WSTRING:
					GET_VA_ARG(p,p,void *,ppfs->arg);
					break;				
				case __PA_NOARG:
					continue;
			}
			++p;
		}
#ifdef NL_ARGMAX
	} else {
		if (ppfs->info.width == INT_MIN) {
			ppfs->info.width
				= (int) GET_ARG_VALUE(p + ppfs->argnumber[0] - 1,u,unsigned int);
		} 
		if (ppfs->info.prec == INT_MIN) {
			ppfs->info.prec
				= (int) GET_ARG_VALUE(p + ppfs->argnumber[1] - 1,u,unsigned int);
		}
	}
#endif /* NL_ARGMAX */

	/* Now we know the width and precision. */
	if (ppfs->info.width < 0) {
		ppfs->info.width = -ppfs->info.width;
		PRINT_INFO_SET_FLAG(&(ppfs->info),left);
		PRINT_INFO_CLR_FLAG(&(ppfs->info),space);
		ppfs->info.pad = ' ';
	}
#if 0
	/* NOTE -- keep neg for now so float knows! */
	if (ppfs->info.prec < 0) {	/* spec says treat as omitted. */
		/* so use default prec... 1 for everything but floats and strings. */
		ppfs->info.prec = 1;
	}
#endif
}
#endif
/**********************************************************************/
#ifdef L__ppfs_parsespec

/* Notes: argtype differs from glibc for the following:
 *         mine              glibc
 *  lc     PA_WCHAR          PA_CHAR       the standard says %lc means %C
 *  ls     PA_WSTRING        PA_STRING     the standard says %ls means %S
 *  {*}n   {*}|PA_FLAG_PTR   PA_FLAG_PTR   size of n can be qualified
 */

/* TODO: store positions of positional args */

/* TODO -- WARNING -- assumes aligned on integer boundaries!!! */

/* TODO -- disable if not using positional args!!! */
#define _OVERLAPPING_DIFFERENT_ARGS

/* TODO -- rethink this -- perhaps we should set to largest type??? */

#ifdef _OVERLAPPING_DIFFERENT_ARGS 

#define PROMOTED_SIZE_OF(X)		((sizeof(X) + sizeof(int) - 1) / sizeof(X))

static const short int type_codes[] = {
	__PA_NOARG,					/* must be first entry */
	PA_POINTER,
	PA_STRING,
	PA_WSTRING,
	PA_CHAR,
	PA_INT|PA_FLAG_SHORT,
	PA_INT,
	PA_INT|PA_FLAG_LONG,
	PA_INT|PA_FLAG_LONG_LONG,
	PA_WCHAR,
#ifdef __STDIO_PRINTF_FLOAT
	/* PA_FLOAT, */
	PA_DOUBLE,
	PA_DOUBLE|PA_FLAG_LONG_DOUBLE,
#endif /* __STDIO_PRINTF_FLOAT */
};

static const unsigned char type_sizes[] = {
	/* unknown type consumes no arg */
	0,							/* must be first entry */
	PROMOTED_SIZE_OF(void *),
	PROMOTED_SIZE_OF(char *),
	PROMOTED_SIZE_OF(wchar_t *),
	PROMOTED_SIZE_OF(char),
	PROMOTED_SIZE_OF(short),
	PROMOTED_SIZE_OF(int),
	PROMOTED_SIZE_OF(long),
#ifdef ULLONG_MAX
	PROMOTED_SIZE_OF(long long),
#else
	PROMOTED_SIZE_OF(long),		/* TODO -- is this correct? (above too) */
#endif
	PROMOTED_SIZE_OF(wchar_t),
#ifdef __STDIO_PRINTF_FLOAT
	/* PROMOTED_SIZE_OF(float), */
	PROMOTED_SIZE_OF(double),
	PROMOTED_SIZE_OF(long double),
#endif /* __STDIO_PRINTF_FLOAT */
};

static int _promoted_size(int argtype)
{
	register const short int *p;

	/* note -- since any unrecognized type is treated as a pointer */
	p = type_codes + sizeof(type_codes)/sizeof(type_codes[0]);
	do {
		if (*--p == argtype) {
			break;
		}
	} while (p > type_codes);

	return type_sizes[(int)(p - type_codes)];
}

static int _is_equal_or_bigger_arg(int curtype, int newtype)
{
	/* Quick test */
	if (newtype == __PA_NOARG) {
		return 0;
	}
	if ((curtype == __PA_NOARG) || (curtype == newtype)) {
		return 1;
	}
	/* Ok... slot is already filled and types are different in name. */
	/* So, compare promoted sizes of curtype and newtype args. */
	return _promoted_size(curtype) <= _promoted_size(newtype);
}

#else

#define _is_equal_or_bigger_arg(C,N)	(((C) == __PA_NOARG) || ((C) == (N)))

#endif

#ifdef __STDIO_GLIBC_CUSTOM_PRINTF
/* TODO - do this differently? */
static char _bss_custom_printf_spec[MAX_USER_SPEC]; /* 0-init'd for us.  */

char *_custom_printf_spec = _bss_custom_printf_spec;
printf_arginfo_function *_custom_printf_arginfo[MAX_USER_SPEC];
printf_function _custom_printf_handler[MAX_USER_SPEC];
#endif /* __STDIO_GLIBC_CUSTOM_PRINTF */

extern int _ppfs_parsespec(ppfs_t *ppfs)
{
	register const char *fmt;
	register const char *p;
	int preci;
	int width;
	int flags;
	int dataargtype;
	int i;
	int dpoint;
#ifdef NL_ARGMAX
	int maxposarg;
#endif /* NL_ARGMAX */
	int p_m_spec_chars;
	int n;
	int argtype[MAX_ARGS_PER_SPEC+2];
	int argnumber[3];			/* width, precision, 1st data arg */
	static const char spec_flags[] = SPEC_FLAGS;
	static const char spec_chars[] = SPEC_CHARS;/* TODO: b? */
	static const char spec_ranges[] = SPEC_RANGES;
	static const short spec_or_mask[] = SPEC_OR_MASK;
	static const short spec_and_mask[] = SPEC_AND_MASK;
	static const char qual_chars[] = QUAL_CHARS;
#ifdef __UCLIBC_HAS_WCHAR__
	char buf[32];
#endif /* __UCLIBC_HAS_WCHAR__ */

	/* WIDE note: we can test against '%' here since we don't allow */
	/* WIDE note: other mappings of '%' in the wide char set. */
	preci = -1;
	argnumber[0] = 0;
	argnumber[1] = 0;
	argtype[0] = __PA_NOARG;
	argtype[1] = __PA_NOARG;
#ifdef NL_ARGMAX
	maxposarg = ppfs->maxposarg;
#endif /* NL_ARGMAX */

#ifdef __UCLIBC_HAS_WCHAR__
	/* This is somewhat lame, but saves a lot of code.  If we're dealing with
	 * a wide stream, that means the format is a wchar string.  So, copy it
	 * char-by-char into a normal char buffer for processing.  Make the buffer
	 * (buf) big enough so that any reasonable format specifier will fit.
	 * While there a legal specifiers that won't, the all involve duplicate
	 * flags or outrageous field widths/precisions. */
	width = dpoint = 0;
	if ((flags = ppfs->info._flags & FLAG_WIDESTREAM) == 0) {
		fmt = ppfs->fmtpos;
	} else {
		fmt = buf + 1;
		i = 0;
		do {
			if ((buf[i] = (char) (((wchar_t *) ppfs->fmtpos)[i-1]))
				!= (((wchar_t *) ppfs->fmtpos)[i-1])
				) {
				return -1;
			}
		} while (buf[i++]);
		buf[sizeof(buf)-1] = 0;
	}
#else  /* __UCLIBC_HAS_WCHAR__ */
	width = flags = dpoint = 0;
	fmt = ppfs->fmtpos;
#endif /* __UCLIBC_HAS_WCHAR__ */

	assert(fmt[-1] == '%');
	assert(fmt[0] != '%');

	/* Process arg pos and/or flags and/or width and/or precision. */
 width_precision:
	p = fmt;
	if (*fmt == '*') {
		argtype[-dpoint] = PA_INT;
		++fmt;
	}
	i = 0;
	while (isdigit(*fmt)) {
		if (i < MAX_FIELD_WIDTH) { /* Avoid overflow. */
			i = (i * 10) + (*fmt - '0');
		}
		++fmt;
	}
	if (p[-1] == '%') { /* Check for a position. */

		/* TODO: if val not in range, then error */

#ifdef NL_ARGMAX
		if ((*fmt == '$') && (i > 0)) {/* Positional spec. */
			++fmt;
			if (maxposarg == 0) {
				return -1;
			}
			if ((argnumber[2] = i) > maxposarg) {
				maxposarg = i;
			}
			/* Now fall through to check flags. */
		} else {
			if (maxposarg > 0) {
#ifdef __STDIO_PRINTF_M_SUPPORT
#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Support prec and width for %m when positional args used
				/* Actually, positional arg processing will fail in general
				 * for specifiers that don't require an arg. */
#endif /* __UCLIBC_MJN3_ONLY__ */
				if (*fmt == 'm') {
					goto PREC_WIDTH;
				}
#endif /* __STDIO_PRINTF_M_SUPPORT */
				return -1;
			}
			maxposarg = 0;		/* Possible redundant store, but cuts size. */

			if ((fmt > p) && (*p != '0')) {
				goto PREC_WIDTH;
			}

			fmt = p;			/* Back up for possible '0's flag. */
			/* Now fall through to check flags. */
		}
#else  /* NL_ARGMAX */
		if (*fmt == '$') {		/* Positional spec. */
			return -1;
		}

		if ((fmt > p) && (*p != '0')) {
			goto PREC_WIDTH;
		}

		fmt = p;			/* Back up for possible '0's flag. */
		/* Now fall through to check flags. */
#endif /* NL_ARGMAX */

	restart_flags:		/* Process flags. */
		i = 1;
		p = spec_flags;
	
		do {
			if (*fmt == *p++) {
				++fmt;
				flags |= i;
				goto restart_flags;
			}
			i += i;				/* Better than i <<= 1 for bcc */
		} while (*p);
		i = 0;

		/* If '+' then ignore ' ', and if '-' then ignore '0'. */
		/* Note: Need to ignore '0' when prec is an arg with val < 0, */
		/*       but that test needs to wait until the arg is retrieved. */
		flags &= ~((flags & (FLAG_PLUS|FLAG_MINUS)) >> 1);
		/* Note: Ignore '0' when prec is specified < 0 too (in printf). */

		if (fmt[-1] != '%') {	/* If we've done anything, loop for width. */
			goto width_precision;
		}
	}
 PREC_WIDTH:
	if (*p == '*') {			/* Prec or width takes an arg. */
#ifdef NL_ARGMAX
		if (maxposarg) {
			if ((*fmt++ != '$') || (i <= 0)) {
				/* Using pos args and no $ or invalid arg number. */
				return -1;
			}
			argnumber[-dpoint] = i;
		} else
#endif /* NL_ARGMAX */
		if (++p != fmt) {
			 /* Not using pos args but digits followed *. */
			return -1;
		}
		i = INT_MIN;
	}

	if (!dpoint) {
		width = i;
		if (*fmt == '.') {
			++fmt;
			dpoint = -1;		/* To use as default precison. */
			goto width_precision;
		}
	} else {
		preci = i;
	}

	/* Process qualifier. */
	p = qual_chars;
	do {
		if (*fmt == *p) {
			++fmt;
			break;
		}
	} while (*++p);
	if ((p - qual_chars < 2) && (*fmt == *p)) {
		p += ((sizeof(qual_chars)-2) / 2);
		++fmt;
	}
	dataargtype = ((int)(p[(sizeof(qual_chars)-2) / 2])) << 8;

	/* Process conversion specifier. */
	if (!*fmt) {
		return -1;
	}

	p = spec_chars;

	do {
		if (*fmt == *p) {
			p_m_spec_chars = p - spec_chars;

			if ((p_m_spec_chars >= CONV_c)
				&& (dataargtype & PA_FLAG_LONG)) {
				p_m_spec_chars -= 2; /* lc -> C and ls -> S */
			}

			ppfs->conv_num = p_m_spec_chars;
			p = spec_ranges-1;
			while (p_m_spec_chars > *++p) {}

			i = p - spec_ranges;
			argtype[2] = (dataargtype | spec_or_mask[i]) & spec_and_mask[i];
			p = spec_chars;
			break;
		}
	} while(*++p);

	ppfs->info.spec = *fmt;
	ppfs->info.prec = preci;
	ppfs->info.width = width;
	ppfs->info.pad = ((flags & FLAG_ZERO) ? '0' : ' ');
	ppfs->info._flags = (flags & ~FLAG_ZERO) | (dataargtype & __PA_INTMASK);
	ppfs->num_data_args = 1;

	if (!*p) {
#ifdef __STDIO_PRINTF_M_SUPPORT
		if (*fmt == 'm') {
			ppfs->conv_num = CONV_m;
			ppfs->num_data_args = 0;
			goto DONE;
		}
#endif
#ifdef __STDIO_GLIBC_CUSTOM_PRINTF

		/* Handle custom arg -- WARNING -- overwrites p!!! */
		ppfs->conv_num = CONV_custom0;
		p = _custom_printf_spec;
		do {
			if (*p == *fmt) {
				if ((ppfs->num_data_args
					 = ((*_custom_printf_arginfo[(int)(p-_custom_printf_spec)])
						(&(ppfs->info), MAX_ARGS_PER_SPEC, argtype+2)))
					> MAX_ARGS_PER_SPEC) {
					break;		/* Error -- too many args! */
				}
				goto DONE;
			}
		} while (++p < (_custom_printf_spec + MAX_USER_SPEC));
#endif /* __STDIO_GLIBC_CUSTOM_PRINTF */
		/* Otherwise error. */
		return -1;
	}
		
#if defined(__STDIO_GLIBC_CUSTOM_PRINTF) || defined(__STDIO_PRINTF_M_SUPPORT)
 DONE:
#endif

#ifdef NL_ARGMAX
	if (maxposarg > 0) {
		i = 0;
		do {
			/* Update maxposarg and check that NL_ARGMAX is not exceeded. */
			n = ((i <= 2)
				 ? (ppfs->argnumber[i] = argnumber[i])
				 : argnumber[2] + (i-2));
			if (n > maxposarg) {
				if ((maxposarg = n) > NL_ARGMAX) {
					return -1;
				}
			}
			--n;
			/* Record argtype with largest size (current, new). */
			if (_is_equal_or_bigger_arg(ppfs->argtype[n], argtype[i])) {
				ppfs->argtype[n] = argtype[i];
			}
		} while (++i < ppfs->num_data_args + 2);
	} else {
#endif /* NL_ARGMAX */
		ppfs->argnumber[2] = 1;
		memcpy(ppfs->argtype, argtype + 2, ppfs->num_data_args * sizeof(int));
#ifdef NL_ARGMAX
	}

	ppfs->maxposarg = maxposarg;
#endif /* NL_ARGMAX */

#ifdef __UCLIBC_HAS_WCHAR__
	if ((flags = ppfs->info._flags & FLAG_WIDESTREAM) == 0) {
		ppfs->fmtpos = ++fmt;
	} else {
		ppfs->fmtpos = (const char *) (((const wchar_t *)(ppfs->fmtpos))
									   + (fmt - buf) );
	}
#else  /* __UCLIBC_HAS_WCHAR__ */
	ppfs->fmtpos = ++fmt;
#endif /* __UCLIBC_HAS_WCHAR__ */

 	return ppfs->num_data_args + 2;
}

#endif
/**********************************************************************/
#ifdef L_register_printf_function

#ifdef __STDIO_GLIBC_CUSTOM_PRINTF

int register_printf_function(int spec, printf_function handler,
							 printf_arginfo_function arginfo)
{
	register char *r;
	register char *p;

	if (spec && (arginfo != NULL)) { /* TODO -- check if spec is valid char */
		r = NULL;
		p = _custom_printf_spec + MAX_USER_SPEC;
		do {
			--p;
			if (!*p) {
				r = p;
			}
#ifdef __BCC__
			else				/* bcc generates less code with fall-through */
#endif
			if (*p == spec) {
				r = p;
				p = _custom_printf_spec;
			}
		} while (p > _custom_printf_spec);

		if (r) {
			if (handler) {
				*r = spec;
				_custom_printf_handler[(int)(r - p)] = handler;
				_custom_printf_arginfo[(int)(r - p)] = arginfo;
			} else {
				*r = 0;
			}
			return 0;
		}
		/* TODO -- if asked to unregister a non-existent spec, return what? */
	}
	return -1;
}

#endif

#endif
/**********************************************************************/
#ifdef L_vsnprintf

#ifdef __UCLIBC_MJN3_ONLY__
#warning WISHLIST: Implement vsnprintf for non-buffered and no custom stream case.
#endif /* __UCLIBC_MJN3_ONLY__ */

#ifdef __STDIO_BUFFERS

int vsnprintf(char *__restrict buf, size_t size,
			  const char * __restrict format, va_list arg)
{
	FILE f;
	int rv;

#ifdef __STDIO_GETC_MACRO
	f.bufgetc =
#endif
	f.bufpos = f.bufread = f.bufstart = buf;

	if (size > SIZE_MAX - (size_t) buf) {
		size = SIZE_MAX - (size_t) buf;
	}
#ifdef __STDIO_PUTC_MACRO
	f.bufputc =
#endif
	f.bufend = buf + size;

#if 0							/* shouldn't be necessary */
/*  #ifdef __STDIO_GLIBC_CUSTOM_STREAMS */
	f.cookie = &(f.filedes);
	f.gcs.read = 0;
	f.gcs.write = 0;
	f.gcs.seek = 0;
	f.gcs.close = 0;
#endif
	f.filedes = -2;				/* for debugging */
	f.modeflags = (__FLAG_NARROW|__FLAG_WRITEONLY|__FLAG_WRITING);

#ifdef __STDIO_MBSTATE
	__INIT_MBSTATE(&(f.state));
#endif /* __STDIO_MBSTATE */

#ifdef __STDIO_THREADSAFE
	f.user_locking = 0;
	__stdio_init_mutex(&f.lock);
#endif

	rv = vfprintf(&f, format, arg);
	if (size) {
		if (f.bufpos == f.bufend) {
			--f.bufpos;
		}
		*f.bufpos = 0;
	}
	return rv;
}

#elif defined(__USE_OLD_VFPRINTF__)

typedef struct {
	FILE f;
	unsigned char *bufend;		/* pointer to 1 past end of buffer */
	unsigned char *bufpos;
} __FILE_vsnprintf;

int vsnprintf(char *__restrict buf, size_t size,
			  const char * __restrict format, va_list arg)
{
	__FILE_vsnprintf f;
	int rv;

	f.bufpos = buf;

	if (size > SIZE_MAX - (size_t) buf) {
		size = SIZE_MAX - (size_t) buf;
	}
	f.bufend = buf + size;

#if 0							/* shouldn't be necessary */
/*  #ifdef __STDIO_GLIBC_CUSTOM_STREAMS */
	f.f.cookie = &(f.f.filedes);
	f.f.gcs.read = 0;
	f.f.gcs.write = 0;
	f.f.gcs.seek = 0;
	f.f.gcs.close = 0;
#endif
	f.f.filedes = -2;				/* for debugging */
	f.f.modeflags = (__FLAG_NARROW|__FLAG_WRITEONLY|__FLAG_WRITING);

#ifdef __STDIO_MBSTATE
	__INIT_MBSTATE(&(f.f.state));
#endif /* __STDIO_MBSTATE */

#ifdef __STDIO_THREADSAFE
	f.f.user_locking = 0;
	__stdio_init_mutex(&f.f.lock);
#endif

	rv = vfprintf((FILE *) &f, format, arg);
	if (size) {
		if (f.bufpos == f.bufend) {
			--f.bufpos;
		}
		*f.bufpos = 0;
	}
	return rv;
}

#elif defined(__STDIO_GLIBC_CUSTOM_STREAMS)

typedef struct {
	size_t pos;
	size_t len;
	unsigned char *buf;
	FILE *fp;
} __snpf_cookie;

#define COOKIE ((__snpf_cookie *) cookie)

static ssize_t snpf_write(register void *cookie, const char *buf,
						  size_t bufsize)
{
	size_t count;
	register char *p;

	/* Note: bufsize < SSIZE_MAX because of _stdio_WRITE. */

	if (COOKIE->len > COOKIE->pos) {
		count = COOKIE->len - COOKIE->pos - 1; /* Leave space for nul. */
		if (count > bufsize) {
			count = bufsize;
		}

		p = COOKIE->buf + COOKIE->pos;
		while (count) {
			*p++ = *buf++;
			--count;
		}
		*p = 0;
	}

	COOKIE->pos += bufsize;

	return bufsize;
}

#undef COOKIE

int vsnprintf(char *__restrict buf, size_t size,
			  const char * __restrict format, va_list arg)
{
	FILE f;
	__snpf_cookie cookie;
	int rv;

	cookie.buf = buf;
	cookie.len = size;
	cookie.pos = 0;
	cookie.fp = &f;

	f.cookie = &cookie;
	f.gcs.write = snpf_write;
	f.gcs.read = NULL;
	f.gcs.seek = NULL;
	f.gcs.close = NULL;

	f.filedes = -1;				/* For debugging. */
	f.modeflags = (__FLAG_NARROW|__FLAG_WRITEONLY|__FLAG_WRITING);

#ifdef __STDIO_MBSTATE
	__INIT_MBSTATE(&(f.state));
#endif /* __STDIO_MBSTATE */

#ifdef __STDIO_THREADSAFE
	f.user_locking = 0;
	__stdio_init_mutex(&f.lock);
#endif

	rv = vfprintf(&f, format, arg);

	return rv;
}

#else
#warning Skipping vsnprintf since no buffering, no custom streams, and not old vfprintf!
#ifdef __STDIO_HAS_VSNPRINTF
#error WHOA! __STDIO_HAS_VSNPRINTF is defined!
#endif
#endif

#endif
/**********************************************************************/
#ifdef L_vdprintf

int vdprintf(int filedes, const char * __restrict format, va_list arg)
{
	FILE f;
	int rv;
#ifdef __STDIO_BUFFERS
	char buf[64];				/* TODO: provide _optional_ buffering? */

#ifdef __STDIO_GETC_MACRO
	f.bufgetc =
#endif
	f.bufpos = f.bufread = f.bufstart = buf;
#ifdef __STDIO_PUTC_MACRO
	f.bufputc = 
#endif
	f.bufend = buf + sizeof(buf);
#endif /* __STDIO_BUFFERS */
#ifdef __STDIO_GLIBC_CUSTOM_STREAMS
	f.cookie = &(f.filedes);
	f.gcs.read = _cs_read;
	f.gcs.write = _cs_write;
	f.gcs.seek = 0;				/* The internal seek func handles normals. */
	f.gcs.close = _cs_close;
#endif
	f.filedes = filedes;
	f.modeflags = (__FLAG_NARROW|__FLAG_WRITEONLY|__FLAG_WRITING);

#ifdef __STDIO_MBSTATE
	__INIT_MBSTATE(&(f.state));
#endif /* __STDIO_MBSTATE */

#ifdef __STDIO_THREADSAFE
	f.user_locking = 0;
	__stdio_init_mutex(&f.lock);
#endif

	rv = vfprintf(&f, format, arg);

	return fflush(&f) ? -1 : rv;
}

#endif
/**********************************************************************/
#ifdef L_vasprintf

#ifndef __STDIO_HAS_VSNPRINTF
#warning Skipping vasprintf since no vsnprintf!
#else

int vasprintf(char **__restrict buf, const char * __restrict format,
			 va_list arg)
{
	/* TODO -- change the default implementation? */
#ifndef __STDIO_GLIBC_CUSTOM_STREAMS
	/* This implementation actually calls the printf machinery twice, but only
	 * only does one malloc.  This can be a problem though when custom printf
	 * specs or the %m specifier are involved because the results of the
	 * second call might be different from the first. */
	va_list arg2;
	int rv;

	va_copy(arg2, arg);
 	rv = vsnprintf(NULL, 0, format, arg2);
	va_end(arg2);

	return (((rv >= 0) && ((*buf = malloc(++rv)) != NULL))
			? vsnprintf(*buf, rv, format, arg)
			: -1);
#else
	FILE *f;
	size_t size;
	int rv;

	/* TODO - do the memstream stuff inline here to avoid fclose and the openlist? */
	if ((f = open_memstream(buf, &size)) == NULL) {
		return -1;
	}
	rv = vfprintf(f, format, arg);
	fclose(f);
	if (rv < 0) {
		free(*buf);
		*buf = NULL;
		return -1;
	}
	return rv;
#endif
}
#endif
#endif
/**********************************************************************/
#ifdef L_vprintf
int vprintf(const char * __restrict format, va_list arg)
{
	return vfprintf(stdout, format, arg);
}
#endif
/**********************************************************************/
#ifdef L_vsprintf

#ifndef __STDIO_HAS_VSNPRINTF
#warning Skipping vsprintf since no vsnprintf!
#else

int vsprintf(char *__restrict buf, const char * __restrict format,
			 va_list arg)
{
	return vsnprintf(buf, SIZE_MAX, format, arg);
}

#endif
#endif
/**********************************************************************/
#ifdef L_fprintf

int fprintf(FILE * __restrict stream, const char * __restrict format, ...)
{
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vfprintf(stream, format, arg);
	va_end(arg);

	return rv;
}

#endif
/**********************************************************************/
#ifdef L_snprintf

#ifndef __STDIO_HAS_VSNPRINTF
#warning Skipping snprintf since no vsnprintf!
#else

int snprintf(char *__restrict buf, size_t size,
			 const char * __restrict format, ...)
{
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vsnprintf(buf, size, format, arg);
	va_end(arg);
	return rv;
}

#endif
#endif
/**********************************************************************/
#ifdef L_dprintf

int dprintf(int filedes, const char * __restrict format, ...)
{
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vdprintf(filedes, format, arg);
	va_end(arg);

	return rv;
}

#endif
/**********************************************************************/
#ifdef L_asprintf

#ifndef __STDIO_HAS_VSNPRINTF
#warning Skipping asprintf and __asprintf since no vsnprintf!
#else

weak_alias(__asprintf,asprintf)

int __asprintf(char **__restrict buf, const char * __restrict format, ...)
{
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vasprintf(buf, format, arg);
	va_end(arg);

	return rv;
}

#endif
#endif
/**********************************************************************/
#ifdef L_printf
int printf(const char * __restrict format, ...)
{
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vfprintf(stdout, format, arg);
	va_end(arg);

	return rv;
}
#endif
/**********************************************************************/
#ifdef L_sprintf

#ifndef __STDIO_HAS_VSNPRINTF
#warning Skipping sprintf since no vsnprintf!
#else

int sprintf(char *__restrict buf, const char * __restrict format, ...)
{
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vsnprintf(buf, SIZE_MAX, format, arg);
	va_end(arg);

	return rv;
}

#endif
#endif
/**********************************************************************/
#ifdef L_vswprintf

#ifdef __STDIO_BUFFERS
int vswprintf(wchar_t *__restrict buf, size_t size,
			  const wchar_t * __restrict format, va_list arg)
{
	FILE f;
	int rv;

#ifdef __STDIO_GETC_MACRO
	f.bufgetc =
#endif
	f.bufpos = f.bufread = f.bufstart = (char *) buf;

/* 	if (size > SIZE_MAX - (size_t) buf) { */
/* 		size = SIZE_MAX - (size_t) buf; */
/* 	} */
#ifdef __STDIO_PUTC_MACRO
	f.bufputc =
#endif
	f.bufend = (char *)(buf + size);

#if 0							/* shouldn't be necessary */
/*  #ifdef __STDIO_GLIBC_CUSTOM_STREAMS */
	f.cookie = &(f.filedes);
	f.gcs.read = 0;
	f.gcs.write = 0;
	f.gcs.seek = 0;
	f.gcs.close = 0;
#endif
	f.filedes = -3;				/* for debugging */
	f.modeflags = (__FLAG_WIDE|__FLAG_WRITEONLY|__FLAG_WRITING);

#ifdef __STDIO_MBSTATE
	__INIT_MBSTATE(&(f.state));
#endif /* __STDIO_MBSTATE */

#ifdef __STDIO_THREADSAFE
	f.user_locking = 0;
	__stdio_init_mutex(&f.lock);
#endif

	rv = vfwprintf(&f, format, arg);

	/* NOTE: Return behaviour differs from snprintf... */
	if (f.bufpos == f.bufend) {
		rv = -1;
		if (size) {
			f.bufpos = (char *)(((wchar_t *) f.bufpos) - 1);
		}
	}
	if (size) {
		*((wchar_t *) f.bufpos) = 0;
	}
	return rv;
}
#else  /* __STDIO_BUFFERS */
#warning Skipping vswprintf since no buffering!
#endif /* __STDIO_BUFFERS */
#endif
/**********************************************************************/
#ifdef L_swprintf
#ifdef __STDIO_BUFFERS

int swprintf(wchar_t *__restrict buf, size_t size,
			 const wchar_t * __restrict format, ...)
{
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vswprintf(buf, size, format, arg);
	va_end(arg);
	return rv;
}

#else  /* __STDIO_BUFFERS */
#warning Skipping vsWprintf since no buffering!
#endif /* __STDIO_BUFFERS */
#endif
/**********************************************************************/
#ifdef L_fwprintf

int fwprintf(FILE * __restrict stream, const wchar_t * __restrict format, ...)
{
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vfwprintf(stream, format, arg);
	va_end(arg);

	return rv;
}

#endif
/**********************************************************************/
#ifdef L_vwprintf
int vwprintf(const wchar_t * __restrict format, va_list arg)
{
	return vfwprintf(stdout, format, arg);
}
#endif
/**********************************************************************/
#ifdef L_wprintf
int wprintf(const wchar_t * __restrict format, ...)
{
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vfwprintf(stdout, format, arg);
	va_end(arg);

	return rv;
}
#endif
/**********************************************************************/
#ifdef L__fpmaxtostr

/* Copyright (C) 2000, 2001, 2003      Manuel Novoa III
 *
 * Function: 
 *
 *     size_t _fpmaxtostr(FILE * fp, __fpmax_t x, struct printf_info *info,
 *                         __fp_outfunc_t fp_outfunc);
 *
 * This is derived from the old _dtostr, whic I wrote for uClibc to provide
 * floating point support for the printf functions.  It handles +/- infinity,
 * nan, and signed 0 assuming you have ieee arithmetic.  It also now handles
 * digit grouping (for the uClibc supported locales) and hexadecimal float
 * notation.  Finally, via the fp_outfunc parameter, it now supports wide
 * output.
 *
 * Notes:
 *
 * At most DECIMAL_DIG significant digits are kept.  Any trailing digits
 * are treated as 0 as they are really just the results of rounding noise
 * anyway.  If you want to do better, use an arbitary precision arithmetic
 * package.  ;-)
 *
 * It should also be fairly portable, as no assumptions are made about the
 * bit-layout of doubles.  Of course, that does make it less efficient than
 * it could be.
 *
 */

/*****************************************************************************/
/* Don't change anything that follows unless you know what you're doing.     */
/*****************************************************************************/
/* Fairly portable nan check.  Bitwise for i386 generated larger code.
 * If you have a better version, comment this out.
 */
#define isnan(x)             ((x) != (x))

/* Without seminumerical functions to examine the sign bit, this is
 * about the best we can do to test for '-0'.
 */
#define zeroisnegative(x)    ((1./(x)) < 0)

/*****************************************************************************/
/* Don't change anything that follows peroid!!!  ;-)                         */
/*****************************************************************************/
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
#if FLT_RADIX != 2
#error FLT_RADIX != 2 is not currently supported
#endif
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */

#define NUM_HEX_DIGITS      ((FPMAX_MANT_DIG + 3)/ 4)

/* WARNING: Adjust _fp_out_wide() below if this changes! */
/* With 32 bit ints, we can get 9 decimal digits per block. */
#define DIGITS_PER_BLOCK     9
#define HEX_DIGITS_PER_BLOCK 8

/* Maximum number of subcases to output double is...
 *  0 - sign
 *  1 - padding and initial digit
 *  2 - digits left of the radix
 *  3 - 0s left of the radix        or   radix
 *  4 - radix                       or   digits right of the radix
 *  5 - 0s right of the radix
 *  6 - exponent
 *  7 - trailing space padding
 * although not all cases may occur.
 */
#define MAX_CALLS 8

/*****************************************************************************/

#define NUM_DIGIT_BLOCKS   ((DECIMAL_DIG+DIGITS_PER_BLOCK-1)/DIGITS_PER_BLOCK)
#define NUM_HEX_DIGIT_BLOCKS \
   ((NUM_HEX_DIGITS+HEX_DIGITS_PER_BLOCK-1)/HEX_DIGITS_PER_BLOCK)

/* WARNING: Adjust _fp_out_wide() below if this changes! */

/* extra space for '-', '.', 'e+###', and nul */
#define BUF_SIZE  ( 3 + NUM_DIGIT_BLOCKS * DIGITS_PER_BLOCK )

/*****************************************************************************/

static const char fmt[] = "inf\0INF\0nan\0NAN\0.\0,";

#define INF_OFFSET        0		/* must be 1st */
#define NAN_OFFSET        8		/* must be 2nd.. see hex sign handling */
#define DECPT_OFFSET     16
#define THOUSEP_OFFSET   18

#define EMPTY_STRING_OFFSET 3

/*****************************************************************************/
#if FPMAX_MAX_10_EXP < -FPMAX_MIN_10_EXP
#error scaling code can not handle FPMAX_MAX_10_EXP < -FPMAX_MIN_10_EXP
#endif

static const __fpmax_t exp10_table[] =
{
	1e1L, 1e2L, 1e4L, 1e8L, 1e16L, 1e32L,	/* floats */
#if FPMAX_MAX_10_EXP < 32
#error unsupported FPMAX_MAX_10_EXP (< 32).  ANSI/ISO C requires >= 37.
#endif
#if FPMAX_MAX_10_EXP >= 64
	1e64L,
#endif
#if FPMAX_MAX_10_EXP >= 128
	1e128L,
#endif
#if FPMAX_MAX_10_EXP >= 256
	1e256L,
#endif
#if FPMAX_MAX_10_EXP >= 512
	1e512L,
#endif
#if FPMAX_MAX_10_EXP >= 1024
	1e1024L,
#endif
#if FPMAX_MAX_10_EXP >= 2048
	1e2048L,
#endif
#if FPMAX_MAX_10_EXP >= 4096
	1e4096L
#endif
#if FPMAX_MAX_10_EXP >= 8192
#error unsupported FPMAX_MAX_10_EXP.  please increase table
#endif
};

#define EXP10_TABLE_SIZE     (sizeof(exp10_table)/sizeof(exp10_table[0]))
#define EXP10_TABLE_MAX      (1U<<(EXP10_TABLE_SIZE-1))

/*****************************************************************************/
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__

#if FLT_RADIX != 2
#error FLT_RADIX != 2 is not currently supported
#endif

#if FPMAX_MAX_EXP < -FPMAX_MIN_EXP
#error scaling code can not handle FPMAX_MAX_EXP < -FPMAX_MIN_EXP
#endif

static const __fpmax_t exp16_table[] = {
	0x1.0p4L, 0x1.0p8L, 0x1.0p16L, 0x1.0p32L, 0x1.0p64L,
#if FPMAX_MAX_EXP >= 128
	0x1.0p128L,
#endif
#if FPMAX_MAX_EXP >= 256
	0x1.0p256L,
#endif
#if FPMAX_MAX_EXP >= 512
	0x1.0p512L,
#endif
#if FPMAX_MAX_EXP >= 1024
	0x1.0p1024L,
#endif
#if FPMAX_MAX_EXP >= 2048
	0x1.0p2048L,
#endif
#if FPMAX_MAX_EXP >= 4096
	0x1.0p4096L,
#endif
#if FPMAX_MAX_EXP >= 8192
	0x1.0p8192L,
#endif
#if FPMAX_MAX_EXP >= 16384
	0x1.0p16384L
#endif
#if FPMAX_MAX_EXP >= 32768 
#error unsupported FPMAX_MAX_EXP.  please increase table
#endif
};

#define EXP16_TABLE_SIZE     (sizeof(exp16_table)/sizeof(exp16_table[0]))
#define EXP16_TABLE_MAX      (1U<<(EXP16_TABLE_SIZE-1))

#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */
/*****************************************************************************/

#define FPO_ZERO_PAD    (0x80 | '0')
#define FPO_STR_WIDTH   (0x80 | ' ');
#define FPO_STR_PREC    'p'

size_t _fpmaxtostr(FILE * fp, __fpmax_t x, struct printf_info *info,
				   __fp_outfunc_t fp_outfunc)
{
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
	__fpmax_t lower_bnd;
	__fpmax_t upper_bnd = 1e9;
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */
	uint_fast32_t digit_block;
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
	uint_fast32_t base = 10;
	const __fpmax_t *power_table;
	int dpb = DIGITS_PER_BLOCK;
	int ndb = NUM_DIGIT_BLOCKS;
	int nd = DECIMAL_DIG;
	int sufficient_precision = 0;
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */
#ifdef __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__
	int num_groups = 0;
	int initial_group;	   /* This does not need to be initialized. */
	int tslen;			   /* This does not need to be initialized. */
	int nblk2;			   /* This does not need to be initialized. */
	const char *ts;		   /* This does not need to be initialized. */
#endif /* __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__ */
	int i, j;
	int round, o_exp;
	int exp, exp_neg;
	int width, preci;
	int cnt;
	char *s;
	char *e;
	intptr_t pc_fwi[3*MAX_CALLS];
	intptr_t *ppc;
	intptr_t *ppc_last;
#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: The size of exp_buf[] should really be determined by the float constants.
#endif /* __UCLIBC_MJN3_ONLY__ */
	char exp_buf[16];
	char buf[BUF_SIZE];
	char sign_str[6];			/* Last 2 are for 1st digit + nul. */
	char o_mode;
	char mode;


	width = info->width;
	preci = info->prec;
	mode = info->spec;

	*exp_buf = 'e';
	if ((mode|0x20) == 'a') {
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
		*exp_buf = 'p';
		if (preci < 0) {
			preci = NUM_HEX_DIGITS;
			sufficient_precision = 1;
		}
#else
		mode += ('g' - 'a');
#endif
	}

	if (preci < 0) {
		preci = 6;
	}

	*sign_str = '\0';
	if (PRINT_INFO_FLAG_VAL(info,showsign)) {
		*sign_str = '+';
	} else if (PRINT_INFO_FLAG_VAL(info,space)) {
		*sign_str = ' ';
	}

	*(sign_str+1) = 0;
	pc_fwi[5] = INF_OFFSET;
	if (isnan(x)) {				/* First, check for nan. */
		pc_fwi[5] = NAN_OFFSET;
		goto INF_NAN;
	}

	if (x == 0) {				/* Handle 0 now to avoid false positive. */
#if 1
		if (zeroisnegative(x)) { /* Handle 'signed' zero. */
			*sign_str = '-';
		}
#endif
		exp = -1;
		goto GENERATE_DIGITS;
	}

	if (x < 0) {				/* Convert negatives to positives. */
		*sign_str = '-';
		x = -x;
	}

	if (__FPMAX_ZERO_OR_INF_CHECK(x)) {	/* Inf since zero handled above. */
	INF_NAN:
		info->pad = ' ';
		ppc = pc_fwi + 6;
		pc_fwi[3] = FPO_STR_PREC;
		pc_fwi[4] = 3;
		if (mode < 'a') {
			pc_fwi[5] += 4;
		}
		pc_fwi[5] = (intptr_t)(fmt + pc_fwi[5]);
		goto EXIT_SPECIAL;
	}

#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Clean up defines when hexadecimal float notation is unsupported.
#endif /* __UCLIBC_MJN3_ONLY__ */

#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__

	if ((mode|0x20) == 'a') {
		lower_bnd = 0x1.0p31L;
		upper_bnd = 0x1.0p32L;
		power_table = exp16_table;
		exp = HEX_DIGITS_PER_BLOCK - 1;
		i = EXP16_TABLE_SIZE;
		j = EXP16_TABLE_MAX;
		dpb = HEX_DIGITS_PER_BLOCK;
		ndb = NUM_HEX_DIGIT_BLOCKS;
		nd = NUM_HEX_DIGITS;
		base = 16;
	} else {
		lower_bnd = 1e8;
/* 		upper_bnd = 1e9; */
		power_table = exp10_table;
		exp = DIGITS_PER_BLOCK - 1;
		i = EXP10_TABLE_SIZE;
		j = EXP10_TABLE_MAX;
/* 		dpb = DIGITS_PER_BLOCK; */
/* 		ndb = NUM_DIGIT_BLOCKS; */
/* 		base = 10; */
	}



#else  /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */

#define lower_bnd    1e8
#define upper_bnd    1e9
#define power_table  exp10_table
#define dpb          DIGITS_PER_BLOCK
#define base         10
#define ndb          NUM_DIGIT_BLOCKS
#define nd           DECIMAL_DIG

	exp = DIGITS_PER_BLOCK - 1;
	i = EXP10_TABLE_SIZE;
	j = EXP10_TABLE_MAX;

#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */

	exp_neg = 0;
	if (x < lower_bnd) {		/* Do we need to scale up or down? */
		exp_neg = 1;
	}

	do {
		--i;
		if (exp_neg) {
			if (x * power_table[i] < upper_bnd) {
				x *= power_table[i];
				exp -= j;
			}
		} else {
			if (x / power_table[i] >= lower_bnd) {
				x /= power_table[i];
				exp += j;
			}
		}
		j >>= 1;
	} while (i);
	if (x >= upper_bnd) {		/* Handle bad rounding case. */
		x /= power_table[0];
		++exp;
	}
	assert(x < upper_bnd);

 GENERATE_DIGITS:
	s = buf + 2;				/* Leave space for '\0' and '0'. */
	i = 0;
	do {
		digit_block = (uint_fast32_t) x;
		assert(digit_block < upper_bnd);
#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: Can rounding be a problem?
#endif /* __UCLIBC_MJN3_ONLY__ */
		x = (x - digit_block) * upper_bnd;
		s += dpb;
		j = 0;
		do {
			s[- ++j] = '0' + (digit_block % base);
			digit_block /= base;
		} while (j < dpb);
	} while (++i < ndb);

	/*************************************************************************/

	if (mode < 'a') {
		*exp_buf -= ('a' - 'A'); /* e->E and p->P */
		mode += ('a' - 'A');
	} 

	o_mode = mode;
	if ((mode == 'g') && (preci > 0)){
		--preci;
	}
	round = preci;

	if (mode == 'f') {
		round += exp;
		if (round < -1) {
			memset(buf, '0', DECIMAL_DIG); /* OK, since 'f' -> decimal case. */
		    exp = -1;
		    round = -1;
		}
	}

	s = buf;
	*s++ = 0;					/* Terminator for rounding and 0-triming. */
	*s = '0';					/* Space to round. */

	i = 0;
	e = s + nd + 1;
	if (round < nd) {
		e = s + round + 2;
		if (*e >= '0' + (base/2)) {	/* NOTE: We always round away from 0! */
			i = 1;
		}
	}

	do {						/* Handle rounding and trim trailing 0s. */
		*--e += i;				/* Add the carry. */
	} while ((*e == '0') || (*e > '0' - 1 + base));

#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
	if ((mode|0x20) == 'a') {
		char *q;
			
		for (q = e ; *q ; --q) {
			if (*q > '9') {
				*q += (*exp_buf - ('p' - 'a') - '9' - 1);
			}
		}

		if (e > s) {
			exp *= 4;			/* Change from base 16 to base 2. */
		}
	}
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */

	o_exp = exp;
	if (e <= s) {				/* We carried into an extra digit. */
		++o_exp;
		e = s;					/* Needed if all 0s. */
	} else {
		++s;
	}
	*++e = 0;					/* Terminating nul char. */

	if ((mode == 'g') && ((o_exp >= -4) && (o_exp <= round))) {
		mode = 'f';
		preci = round - o_exp;
	}

	exp = o_exp;
	if (mode != 'f') {
		o_exp = 0;
	}

	if (o_exp < 0) {			/* Exponent is < 0, so */
		*--s = '0';				/* fake the first 0 digit. */
	}

	pc_fwi[3] = FPO_ZERO_PAD;
	pc_fwi[4] = 1;
	pc_fwi[5] = (intptr_t)(sign_str + 4);
	sign_str[4] = *s++;
	sign_str[5] = 0;
	ppc = pc_fwi + 6;

	i = e - s;					/* Total digits is 'i'. */
	if (o_exp >= 0) {
#ifdef __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__

		const char *p;

		if (PRINT_INFO_FLAG_VAL(info,group)
			&& *(p = __UCLIBC_CURLOCALE_DATA.grouping)
			) {
			int nblk1;

			nblk2 = nblk1 = *p;
			if (*++p) {
				nblk2 = *p;
				assert(!*++p);
			}

			if (o_exp >= nblk1) {
				num_groups = (o_exp - nblk1) / nblk2 + 1;
				initial_group = (o_exp - nblk1) % nblk2;

#ifdef __UCLIBC_HAS_WCHAR__
				if (PRINT_INFO_FLAG_VAL(info,wide)) {
					/* _fp_out_wide() will fix this up. */
					ts = fmt + THOUSEP_OFFSET;
					tslen = 1;
				} else {
#endif /* __UCLIBC_HAS_WCHAR__ */
					ts = __UCLIBC_CURLOCALE_DATA.thousands_sep;
					tslen = __UCLIBC_CURLOCALE_DATA.thousands_sep_len;
#ifdef __UCLIBC_HAS_WCHAR__
				}
#endif /* __UCLIBC_HAS_WCHAR__ */

				width -= num_groups * tslen;
			}
		}


#endif /* __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__ */
		ppc[0] = FPO_STR_PREC;
		ppc[2] = (intptr_t)(s);
		if (o_exp >= i) {		/* all digit(s) left of decimal */
			ppc[1] = i;
			ppc += 3;
			o_exp -= i;
			i = 0;
			if (o_exp>0) {		/* have 0s left of decimal */
				ppc[0] = FPO_ZERO_PAD;
				ppc[1] = o_exp;
				ppc[2] = (intptr_t)(fmt + EMPTY_STRING_OFFSET);
				ppc += 3;
			}
		} else if (o_exp > 0) {	/* decimal between digits */
			ppc[1] = o_exp;
			ppc += 3;
			s += o_exp;
			i -= o_exp;
		}
		o_exp = -1;
	}

	if (PRINT_INFO_FLAG_VAL(info,alt)
		|| (i)
		|| ((o_mode != 'g')
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
			&& (o_mode != 'a')
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */
			&& (preci > 0))
		) {
		ppc[0] = FPO_STR_PREC;
#ifdef __LOCALE_C_ONLY
		ppc[1] = 1;
		ppc[2] = (intptr_t)(fmt + DECPT_OFFSET);
#else  /* __LOCALE_C_ONLY */
#ifdef __UCLIBC_HAS_WCHAR__
			if (PRINT_INFO_FLAG_VAL(info,wide)) {
				/* _fp_out_wide() will fix this up. */
				ppc[1] = 1;
				ppc[2] = (intptr_t)(fmt + DECPT_OFFSET);
			} else {
#endif /* __UCLIBC_HAS_WCHAR__ */
				ppc[1] = __UCLIBC_CURLOCALE_DATA.decimal_point_len;
				ppc[2] = (intptr_t)(__UCLIBC_CURLOCALE_DATA.decimal_point);
#ifdef __UCLIBC_HAS_WCHAR__
			}
#endif /* __UCLIBC_HAS_WCHAR__ */
#endif /* __LOCALE_C_ONLY */
			ppc += 3;
	}

	if (++o_exp < 0) {			/* Have 0s right of decimal. */
		ppc[0] = FPO_ZERO_PAD;
		ppc[1] = -o_exp;
		ppc[2] = (intptr_t)(fmt + EMPTY_STRING_OFFSET);
		ppc += 3;
	}
	if (i) {					/* Have digit(s) right of decimal. */
		ppc[0] = FPO_STR_PREC;
		ppc[1] = i;
		ppc[2] = (intptr_t)(s);
		ppc += 3;
	}

	if (((o_mode != 'g') || PRINT_INFO_FLAG_VAL(info,alt))
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
		&& !sufficient_precision
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */
		) {
		i -= o_exp;
		if (i < preci) {		/* Have 0s right of digits. */
			i = preci - i;
			ppc[0] = FPO_ZERO_PAD;
			ppc[1] = i;
			ppc[2] = (intptr_t)(fmt + EMPTY_STRING_OFFSET);
			ppc += 3;
		}
	}

	/* Build exponent string. */
	if (mode != 'f') {
		char *p = exp_buf + sizeof(exp_buf);
		char exp_char = *exp_buf;
		char exp_sign = '+';
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
		int min_exp_dig_plus_2 = ((o_mode != 'a') ? (2+2) : (2+1));
#else  /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */
#define min_exp_dig_plus_2  (2+2)
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */

		if (exp < 0) {
			exp_sign = '-';
			exp = -exp;
		}

		*--p = 0;			/* nul-terminate */
		j = 2;				/* Count exp_char and exp_sign. */
		do {
			*--p = '0' + (exp % 10);
			exp /= 10;
		} while ((++j < min_exp_dig_plus_2) || exp); /* char+sign+mindigits */
		*--p = exp_sign;
		*--p = exp_char;

		ppc[0] = FPO_STR_PREC;
		ppc[1] = j;
		ppc[2] = (intptr_t)(p);
		ppc += 3;
	}

 EXIT_SPECIAL:
	ppc_last = ppc;
	ppc = pc_fwi + 4;	 /* Need width fields starting with second. */
	do {
		width -= *ppc;
		ppc += 3;
	} while (ppc < ppc_last);

	ppc = pc_fwi;
	ppc[0] = FPO_STR_WIDTH;
	ppc[1] = i = ((*sign_str) != 0);
	ppc[2] = (intptr_t) sign_str;

#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
	if (((mode|0x20) == 'a') && (pc_fwi[3] >= 16)) { /* Hex sign handling. */
		/* Hex and not inf or nan, so prefix with 0x. */
		char *h = sign_str + i;
		*h = '0';
		*++h = 'x' - 'p' + *exp_buf;
		*++h = 0;
		ppc[1] = (i += 2);
	}
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */

	if ((width -= i) > 0) {
		if (PRINT_INFO_FLAG_VAL(info,left)) { /* Left-justified. */
			ppc_last[0] = FPO_STR_WIDTH;
			ppc_last[1] = width;
			ppc_last[2] = (intptr_t)(fmt + EMPTY_STRING_OFFSET);
			ppc_last += 3;
		} else if (info->pad == '0') { /* 0 padding */
			ppc[4] += width;	/* Pad second field. */
		} else {
			ppc[1] += width;	/* Pad first (sign) field. */
		}
	}

	cnt = 0;

	do {
#ifdef __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__

		if ((ppc == pc_fwi + 6) && num_groups) {
			const char *gp = (const char *) ppc[2];
			int len = ppc[1];
			int blk = initial_group;

			cnt += num_groups * tslen; /* Adjust count now for sep chars. */

/* 			printf("\n"); */
			do {
				if (!blk) {		/* Initial group could be 0 digits long! */
					blk = nblk2;
				} else if (len >= blk) { /* Enough digits for a group. */
/* 					printf("norm:  len=%d blk=%d  \"%.*s\"\n", len, blk, blk, gp); */
					fp_outfunc(fp, *ppc, blk, (intptr_t) gp);
					assert(gp);
					if (*gp) {
						gp += blk;
					}
					len -= blk;
				} else {		/* Transition to 0s. */
/* 					printf("trans: len=%d blk=%d  \"%.*s\"\n", len, blk, len, gp); */
					if (len) {
/* 						printf("len\n"); */
						fp_outfunc(fp, *ppc, len, (intptr_t) gp);
						gp += len;
					}

					if (ppc[3] == FPO_ZERO_PAD) { /* Need to group 0s */
/* 						printf("zeropad\n"); */
						cnt += ppc[1];
						ppc += 3;
						gp = (const char *) ppc[2];
						blk -= len;	/* blk > len, so blk still > 0. */
						len = ppc[1];
						continue; /* Don't decrement num_groups here. */
					} else {
						assert(num_groups == 0);
						break;
					}
				}

				if (num_groups <= 0) {
					break;
				}
				--num_groups;

				fp_outfunc(fp, FPO_STR_PREC, tslen, (intptr_t) ts);
				blk = nblk2;

/* 				printf("num_groups=%d   blk=%d\n", num_groups, blk); */

			} while (1);
		} else

#endif /* __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__ */

		fp_outfunc(fp, *ppc, ppc[1], ppc[2]); /* NOTE: Remember 'else' above! */

		cnt += ppc[1];
		ppc += 3;
	} while (ppc < ppc_last);

	return cnt;
}

#endif
/**********************************************************************/
#ifdef L__store_inttype

/* Right now, we assume intmax_t is either long or long long */

#ifdef INTMAX_MAX

#ifdef LLONG_MAX

#if INTMAX_MAX > LLONG_MAX
#error INTMAX_MAX > LLONG_MAX!  The printf code needs to be updated!
#endif

#elif INTMAX_MAX > LONG_MAX

#error No LLONG_MAX and INTMAX_MAX > LONG_MAX!  The printf code needs to be updated!

#endif /* LLONG_MAX */

#endif /* INTMAX_MAX */

/* We assume int may be short or long, but short and long are different. */

void _store_inttype(register void *dest, int desttype, uintmax_t val)
{
	if (desttype == __PA_FLAG_CHAR) { /* assume char not int */
		*((unsigned char *) dest) = val;
		return;
	}
#if defined(LLONG_MAX) && (LONG_MAX != LLONG_MAX)
	if (desttype == PA_FLAG_LONG_LONG) {
		*((unsigned long long int *) dest) = val;
		return;
	}
#endif /* LLONG_MAX */
#if SHRT_MAX != INT_MAX
	if (desttype == PA_FLAG_SHORT) {
		*((unsigned short int *) dest) = val;
		return;
	}
#endif /* SHRT_MAX */
#if LONG_MAX != INT_MAX
	if (desttype == PA_FLAG_LONG) {
		*((unsigned long int *) dest) = val;
		return;
	}
#endif /* LONG_MAX */

	*((unsigned int *) dest) = val;
}

#endif
/**********************************************************************/
#ifdef L__load_inttype

extern uintmax_t _load_inttype(int desttype, register const void *src,
							   int uflag)
{
	if (uflag >= 0) {			/* unsigned */
#if LONG_MAX != INT_MAX
		if (desttype & (PA_FLAG_LONG|PA_FLAG_LONG_LONG)) {
#ifdef LLONG_MAX
			if (desttype == PA_FLAG_LONG_LONG) {
				return *((unsigned long long int *) src);
			}
#endif
			return *((unsigned long int *) src);
		}
#else  /* LONG_MAX != INT_MAX */
#ifdef LLONG_MAX
		if (desttype & PA_FLAG_LONG_LONG) {
			return *((unsigned long long int *) src);
		}
#endif
#endif /* LONG_MAX != INT_MAX */
		{
			unsigned int x;
			x = *((unsigned int *) src);
			if (desttype == __PA_FLAG_CHAR) x = (unsigned char) x;
#if SHRT_MAX != INT_MAX
			if (desttype == PA_FLAG_SHORT) x = (unsigned short int) x;
#endif
			return x;
		}
	} else {					/* signed */
#if LONG_MAX != INT_MAX
		if (desttype & (PA_FLAG_LONG|PA_FLAG_LONG_LONG)) {
#ifdef LLONG_MAX
			if (desttype == PA_FLAG_LONG_LONG) {
				return *((long long int *) src);
			}
#endif
			return *((long int *) src);
		}
#else  /* LONG_MAX != INT_MAX */
#ifdef LLONG_MAX
		if (desttype & PA_FLAG_LONG_LONG) {
			return *((long long int *) src);
		}
#endif
#endif /* LONG_MAX != INT_MAX */
		{
			int x;
			x = *((int *) src);
			if (desttype == __PA_FLAG_CHAR) x = (char) x;
#if SHRT_MAX != INT_MAX
			if (desttype == PA_FLAG_SHORT) x = (short int) x;
#endif
			return x;
		}
	}
}

#endif
/**********************************************************************/
#if defined(L_vfprintf) || defined(L_vfwprintf)

/* We only support ascii digits (or their USC equivalent codes) in
 * precision and width settings in *printf (wide) format strings.
 * In other words, we don't currently support glibc's 'I' flag.
 * We do accept it, but it is currently ignored. */

static void _charpad(FILE * __restrict stream, int padchar, size_t numpad);

#ifdef L_vfprintf

#define VFPRINTF vfprintf
#define FMT_TYPE char
#define OUTNSTR _outnstr
#define STRLEN  strlen
#define _PPFS_init _ppfs_init
#define OUTPUT(F,S)			fputs(S,F)
#define _outnstr(stream, string, len)	_stdio_fwrite(string, len, stream)
#define FP_OUT _fp_out_narrow

#ifdef __STDIO_PRINTF_FLOAT

static void _fp_out_narrow(FILE *fp, intptr_t type, intptr_t len, intptr_t buf)
{
	if (type & 0x80) {			/* Some type of padding needed. */
		int buflen = strlen((const char *) buf);
		if ((len -= buflen) > 0) {
			_charpad(fp, (type & 0x7f), len);
		}
		len = buflen;
	}
	OUTNSTR(fp, (const char *) buf, len);
}

#endif /* __STDIO_PRINTF_FLOAT */

#else  /* L_vfprintf */

#define VFPRINTF vfwprintf
#define FMT_TYPE wchar_t
#define OUTNSTR _outnwcs
#define STRLEN  wcslen
#define _PPFS_init _ppwfs_init
#define OUTPUT(F,S)			fputws(S,F)
#define _outnwcs(stream, wstring, len)	_wstdio_fwrite(wstring, len, stream)
#define FP_OUT _fp_out_wide

static void _outnstr(FILE *stream, const char *s, size_t wclen)
{
	/* NOTE!!! len here is the number of wchars we want to generate!!! */
	wchar_t wbuf[64];
	mbstate_t mbstate;
	size_t todo, r;

	mbstate.mask = 0;
	todo = wclen;
	
	while (todo) {
		r = mbsrtowcs(wbuf, &s,
					  ((todo <= sizeof(wbuf)/sizeof(wbuf[0]))
					   ? todo
					   : sizeof(wbuf)/sizeof(wbuf[0])),
					  &mbstate);
		assert(((ssize_t)r) > 0);
		_outnwcs(stream, wbuf, r);
		todo -= r;
	}
}

#ifdef __STDIO_PRINTF_FLOAT

#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Move defines from _fpmaxtostr.  Put them in a common header.
#endif

/* The following defines are from _fpmaxtostr.*/
#define DIGITS_PER_BLOCK     9
#define NUM_DIGIT_BLOCKS   ((DECIMAL_DIG+DIGITS_PER_BLOCK-1)/DIGITS_PER_BLOCK)
#define BUF_SIZE  ( 3 + NUM_DIGIT_BLOCKS * DIGITS_PER_BLOCK )

static void _fp_out_wide(FILE *fp, intptr_t type, intptr_t len, intptr_t buf)
{
	wchar_t wbuf[BUF_SIZE];
	const char *s = (const char *) buf;
	int i;

	if (type & 0x80) {			/* Some type of padding needed */
		int buflen = strlen(s);
		if ((len -= buflen) > 0) {
			_charpad(fp, (type & 0x7f), len);
		}
		len = buflen;
	}

	if (len > 0) {
		i = 0;
		do {
#ifdef __LOCALE_C_ONLY
			wbuf[i] = s[i];
#else  /* __LOCALE_C_ONLY */

#ifdef __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__
			if (s[i] == ',') {
				wbuf[i] = __UCLIBC_CURLOCALE_DATA.thousands_sep_wc;
			} else
#endif /* __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__ */
			if (s[i] == '.') {
				wbuf[i] = __UCLIBC_CURLOCALE_DATA.decimal_point_wc;
			} else {
				wbuf[i] = s[i];
			}
#endif /* __LOCALE_C_ONLY */

		} while (++i < len);

		OUTNSTR(fp, wbuf, len);
	}
}

#endif /* __STDIO_PRINTF_FLOAT */

static int _ppwfs_init(register ppfs_t *ppfs, const wchar_t *fmt0)
{
	static const wchar_t invalid_wcs[] = L"Invalid wide format string.";
	int r;

	/* First, zero out everything... argnumber[], argtype[], argptr[] */
	memset(ppfs, 0, sizeof(ppfs_t)); /* TODO: nonportable???? */
#ifdef NL_ARGMAX
	--ppfs->maxposarg;			/* set to -1 */
#endif /* NL_ARGMAX */
	ppfs->fmtpos = (const char *) fmt0;
	ppfs->info._flags = FLAG_WIDESTREAM;

	{
		mbstate_t mbstate;
		const wchar_t *p;
		mbstate.mask = 0;	/* Initialize the mbstate. */
		p = fmt0;
		if (wcsrtombs(NULL, &p, SIZE_MAX, &mbstate) == ((size_t)(-1))) {
			ppfs->fmtpos = (const char *) invalid_wcs;
			return -1;
		}
	}

	/* now set all argtypes to no-arg */
	{
#if 1
		/* TODO - use memset here since already "paid for"? */
		register int *p = ppfs->argtype;
		
		r = MAX_ARGS;
		do {
			*p++ = __PA_NOARG;
		} while (--r);
#else
		/* TODO -- get rid of this?? */
		register char *p = (char *) ((MAX_ARGS-1) * sizeof(int));

		do {
			*((int *)(((char *)ppfs) + ((int)p) + offsetof(ppfs_t,argtype))) = __PA_NOARG;
			p -= sizeof(int);
		} while (p);
#endif
	}

	/*
	 * Run through the entire format string to validate it and initialize
	 * the positional arg numbers (if any).
	 */
	{
		register const wchar_t *fmt = fmt0;

		while (*fmt) {
			if ((*fmt == '%') && (*++fmt != '%')) {
				ppfs->fmtpos = (const char *) fmt; /* back up to the '%' */
				if ((r = _ppfs_parsespec(ppfs)) < 0) {
					return -1;
				}
				fmt = (const wchar_t *) ppfs->fmtpos; /* update to one past end of spec */
			} else {
				++fmt;
			}
		}
		ppfs->fmtpos = (const char *) fmt0; /* rewind */
	}

#ifdef NL_ARGMAX
	/* If we have positional args, make sure we know all the types. */
	{
		register int *p = ppfs->argtype;
		r = ppfs->maxposarg;
		while (--r >= 0) {
			if ( *p == __PA_NOARG ) { /* missing arg type!!! */
				return -1;
			}
			++p;
		}
	}
#endif /* NL_ARGMAX */

	return 0;
}

#endif /* L_vfprintf */

static void _charpad(FILE * __restrict stream, int padchar, size_t numpad)
{
	/* TODO -- Use a buffer to cut down on function calls... */
	FMT_TYPE pad[1];

	*pad = padchar;
	while (numpad) {
		OUTNSTR(stream, pad, 1);
		--numpad;
	}
}

/* TODO -- Dynamically allocate work space to accomodate stack-poor archs? */
static int _do_one_spec(FILE * __restrict stream,
						 register ppfs_t *ppfs, int *count)
{
	static const char spec_base[] = SPEC_BASE;
#ifdef L_vfprintf
	static const char prefix[] = "+\0-\0 \0000x\0000X";
	/*                            0  2  4  6   9 11*/
#else  /* L_vfprintf */
	static const wchar_t prefix[] = L"+\0-\0 \0000x\0000X";
#endif /* L_vfprintf */
	enum {
		PREFIX_PLUS = 0,
		PREFIX_MINUS = 2,
		PREFIX_SPACE = 4,
		PREFIX_LWR_X = 6,
		PREFIX_UPR_X = 9,
		PREFIX_NONE = 11
	};

#ifdef __va_arg_ptr
	const void * const *argptr;
#else
	const void * argptr[MAX_ARGS_PER_SPEC];
#endif
	int *argtype;
#ifdef __UCLIBC_HAS_WCHAR__
	const wchar_t *ws = NULL;
	mbstate_t mbstate;
#endif /* __UCLIBC_HAS_WCHAR__ */
	size_t slen;
#ifdef L_vfprintf
#define SLEN slen
#else
	size_t SLEN;
	wchar_t wbuf[2];
#endif
	int base;
	int numpad;
	int alphacase;
	int numfill = 0;			/* TODO: fix */
	int prefix_num = PREFIX_NONE;
	char padchar = ' ';
#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Determine appropriate buf size.
#endif /* __UCLIBC_MJN3_ONLY__ */
	/* TODO: buf needs to be big enough for any possible error return strings
	 * and also for any locale-grouped long long integer strings generated.
	 * This should be large enough for any of the current archs/locales, but
	 * eventually this should be handled robustly. */
	char buf[128];

#ifdef NDEBUG
	_ppfs_parsespec(ppfs);
#else
	if (_ppfs_parsespec(ppfs) < 0) { /* TODO: just for debugging */
		abort();
	}
#endif
	_ppfs_setargs(ppfs);

	argtype = ppfs->argtype + ppfs->argnumber[2] - 1;
	/* Deal with the argptr vs argvalue issue. */
#ifdef __va_arg_ptr
	argptr = (const void * const *) ppfs->argptr;
#ifdef NL_ARGMAX
	if (ppfs->maxposarg > 0) {	/* Using positional args... */
		argptr += ppfs->argnumber[2] - 1;
	}
#endif /* NL_ARGMAX */
#else
	/* Need to build a local copy... */
	{
		register argvalue_t *p = ppfs->argvalue;
		int i;
#ifdef NL_ARGMAX
		if (ppfs->maxposarg > 0) {	/* Using positional args... */
			p += ppfs->argnumber[2] - 1;
		}
#endif /* NL_ARGMAX */
		for (i = 0 ; i < ppfs->num_data_args ; i++ ) {
			argptr[i] = (void *) p++;
		}
	}
#endif
	{
		register char *s = NULL; /* TODO: Should s be unsigned char * ? */

		if (ppfs->conv_num == CONV_n) {
			_store_inttype(*(void **)*argptr,
						   ppfs->info._flags & __PA_INTMASK,
						   (intmax_t) (*count));
			return 0;
		}
		if (ppfs->conv_num <= CONV_i) {	/* pointer or (un)signed int */
			alphacase = __UIM_LOWER;

#ifdef __UCLIBC_MJN3_ONLY__
#ifdef L_vfprintf
#warning CONSIDER: Should we ignore these flags if stub locale?  What about custom specs?
#endif
#endif /* __UCLIBC_MJN3_ONLY__ */
			if ((base = spec_base[(int)(ppfs->conv_num - CONV_p)]) == 10) {
				if (PRINT_INFO_FLAG_VAL(&(ppfs->info),group)) {
					alphacase = __UIM_GROUP;
				}
				if (PRINT_INFO_FLAG_VAL(&(ppfs->info),i18n)) {
					alphacase |= 0x80;
				}
			}

			if (ppfs->conv_num <= CONV_u) { /* pointer or unsigned int */
				if (ppfs->conv_num == CONV_X) {
					alphacase = __UIM_UPPER;
				}
				if (ppfs->conv_num == CONV_p) { /* pointer */
					prefix_num = PREFIX_LWR_X;
				} else {		/* unsigned int */
				}
			} else {			/* signed int */
				base = -base;
			}
			if (ppfs->info.prec < 0) { /* Ignore '0' flag if prec specified. */
				padchar = ppfs->info.pad;
			}
#ifdef __UCLIBC_MJN3_ONLY__
#ifdef L_vfprintf
#warning CONSIDER: If using outdigits and/or grouping, how should we interpret precision?
#endif
#endif /* __UCLIBC_MJN3_ONLY__ */
			s = _uintmaxtostr(buf + sizeof(buf) - 1,
							  (uintmax_t)
							  _load_inttype(*argtype & __PA_INTMASK,
											*argptr, base), base, alphacase);
			if (ppfs->conv_num > CONV_u) { /* signed int */
				if (*s == '-') {
					PRINT_INFO_SET_FLAG(&(ppfs->info),showsign);
					++s;		/* handle '-' in the prefix string */
					prefix_num = PREFIX_MINUS;
				} else if (PRINT_INFO_FLAG_VAL(&(ppfs->info),showsign)) {
					prefix_num = PREFIX_PLUS;
				} else if (PRINT_INFO_FLAG_VAL(&(ppfs->info),space)) {
					prefix_num = PREFIX_SPACE;
				}
			}
			slen = (char *)(buf + sizeof(buf) - 1) - s;
#ifdef L_vfwprintf
			{
				const char *q = s;
				mbstate.mask = 0; /* Initialize the mbstate. */
				SLEN = mbsrtowcs(NULL, &q, 0, &mbstate);
			}
#endif
			numfill = ((ppfs->info.prec < 0) ? 1 : ppfs->info.prec);
			if (PRINT_INFO_FLAG_VAL(&(ppfs->info),alt)) {
				if (ppfs->conv_num <= CONV_x) {	/* x or p */
					prefix_num = PREFIX_LWR_X;
				}
				if (ppfs->conv_num == CONV_X) {
					prefix_num = PREFIX_UPR_X;
				}
				if ((ppfs->conv_num == CONV_o) && (numfill <= SLEN)) {
					numfill = ((*s == '0') ? 1 : SLEN + 1);
				}
			}
			if (*s == '0') {
				if (prefix_num >= PREFIX_LWR_X) {
					prefix_num = PREFIX_NONE;
				}
				if (ppfs->conv_num == CONV_p) {/* null pointer */
					s = "(nil)";
#ifdef L_vfwprintf
					SLEN =
#endif
					slen = 5;
					numfill = 0;
				} else if (numfill == 0) {	/* if precision 0, no output */
#ifdef L_vfwprintf
					SLEN =
#endif
					slen = 0;
				}
			}
			numfill = ((numfill > SLEN) ? numfill - SLEN : 0);
		} else if (ppfs->conv_num <= CONV_A) {	/* floating point */
#ifdef __STDIO_PRINTF_FLOAT
			*count +=
				_fpmaxtostr(stream,
							(__fpmax_t)
							(PRINT_INFO_FLAG_VAL(&(ppfs->info),is_long_double)
							 ? *(long double *) *argptr
							 : (long double) (* (double *) *argptr)),
							&ppfs->info, FP_OUT );
			return 0;
#else  /* __STDIO_PRINTF_FLOAT */
			return -1;			/* TODO -- try to continue? */
#endif /* __STDIO_PRINTF_FLOAT */
		} else if (ppfs->conv_num <= CONV_S) {	/* wide char or string */
#ifdef L_vfprintf

#ifdef __UCLIBC_HAS_WCHAR__
			mbstate.mask = 0;	/* Initialize the mbstate. */
			if (ppfs->conv_num == CONV_S) { /* wide string */
				if (!(ws = *((const wchar_t **) *argptr))) {
					goto NULL_STRING;
				}
				/* We use an awful uClibc-specific hack here, passing
				 * (char*) &ws as the conversion destination.  This signals
				 * uClibc's wcsrtombs that we want a "restricted" length
				 * such that the mbs fits in a buffer of the specified
				 * size with no partial conversions. */
				if ((slen = wcsrtombs((char *) &ws, &ws, /* Use awful hack! */
									  ((ppfs->info.prec >= 0)
									   ? ppfs->info.prec
									   : SIZE_MAX), &mbstate))
					== ((size_t)-1)
					) {
					return -1;	/* EILSEQ */
				}
			} else {			/* wide char */
				s = buf;
				slen = wcrtomb(s, (*((const wchar_t *) *argptr)), &mbstate);
				if (slen == ((size_t)-1)) {
					return -1;	/* EILSEQ */
				}
				s[slen] = 0;	/* TODO - Is this necessary? */
			}
#else  /* __UCLIBC_HAS_WCHAR__ */
			return -1;
#endif /* __UCLIBC_HAS_WCHAR__ */
		} else if (ppfs->conv_num <= CONV_s) {	/* char or string */
			if (ppfs->conv_num == CONV_s) { /* string */
				s = *((char **) (*argptr));
				if (s) {
#ifdef __STDIO_PRINTF_M_SUPPORT
				SET_STRING_LEN:
#endif
					slen = strnlen(s, ((ppfs->info.prec >= 0)
									   ? ppfs->info.prec : SIZE_MAX));
				} else {
#ifdef __UCLIBC_HAS_WCHAR__
				NULL_STRING:
#endif
					s = "(null)";
					slen = 6;
				}
			} else {			/* char */
				s = buf;
				*s = (unsigned char)(*((const int *) *argptr));
				s[1] = 0;
				slen = 1;
			}

#else  /* L_vfprintf */

			if (ppfs->conv_num == CONV_S) { /* wide string */
				ws = *((wchar_t **) (*argptr));
				if (!ws) {
					goto NULL_STRING;
				}
				SLEN = wcsnlen(ws, ((ppfs->info.prec >= 0)
									? ppfs->info.prec : SIZE_MAX));
			} else {			/* wide char */
				*wbuf = (wchar_t)(*((const wint_t *) *argptr));
			CHAR_CASE:
				ws = wbuf;
				wbuf[1] = 0;
				SLEN = 1;
			}

		} else if (ppfs->conv_num <= CONV_s) {	/* char or string */

			if (ppfs->conv_num == CONV_s) { /* string */
#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Fix %s for vfwprintf... output upto illegal sequence?
#endif /* __UCLIBC_MJN3_ONLY__ */
				s = *((char **) (*argptr));
				if (s) {
#ifdef __STDIO_PRINTF_M_SUPPORT
				SET_STRING_LEN:
#endif
					/* We use an awful uClibc-specific hack here, passing
					 * (wchar_t*) &mbstate as the conversion destination.
					 *  This signals uClibc's mbsrtowcs that we want a
					 * "restricted" length such that the mbs fits in a buffer
					 * of the specified size with no partial conversions. */
					{
						const char *q = s;
						mbstate.mask = 0;	/* Initialize the mbstate. */
						SLEN = mbsrtowcs((wchar_t *) &mbstate, &q,
										 ((ppfs->info.prec >= 0)
										  ? ppfs->info.prec : SIZE_MAX),
										 &mbstate);
					}
					if (SLEN == ((size_t)(-1))) {
						return -1;	/* EILSEQ */
					}
				} else {
				NULL_STRING:
					s = "(null)";
					SLEN = slen = 6;
				}
			} else {			/* char */
				*wbuf = btowc( (unsigned char)(*((const int *) *argptr)) );
				goto CHAR_CASE;
			}

#endif /* L_vfprintf */

#ifdef __STDIO_PRINTF_M_SUPPORT
		} else if (ppfs->conv_num == CONV_m) {
			s = _glibc_strerror_r(errno, buf, sizeof(buf));
			goto SET_STRING_LEN;
#endif
		} else {
#ifdef __STDIO_GLIBC_CUSTOM_PRINTF
			assert(ppfs->conv_num == CONV_custom0);

			s = _custom_printf_spec;
			do {
				if (*s == ppfs->info.spec) {
					int rv;
					/* TODO -- check return value for sanity? */
					rv = (*_custom_printf_handler
						  [(int)(s-_custom_printf_spec)])
						(stream, &ppfs->info, argptr);
					if (rv < 0) {
						return -1;
					}
					*count += rv;
					return 0;
				}
			} while (++s < (_custom_printf_spec + MAX_USER_SPEC));
#endif /* __STDIO_GLIBC_CUSTOM_PRINTF */
			assert(0);
			return -1;
		}

#ifdef __UCLIBC_MJN3_ONLY__
#ifdef L_vfprintf
#warning CONSIDER: If using outdigits and/or grouping, how should we pad?
#endif
#endif /* __UCLIBC_MJN3_ONLY__ */
		{
			size_t t;

			t = SLEN + numfill;
			if (prefix_num != PREFIX_NONE) {
				t += ((prefix_num < PREFIX_LWR_X) ? 1 : 2);
			}
			numpad = ((ppfs->info.width > t) ? (ppfs->info.width - t) : 0);
			*count += t + numpad;
		}
		if (padchar == '0') { /* TODO: check this */
			numfill += numpad;
			numpad = 0;
		}

		/* Now handle the output itself. */
		if (!PRINT_INFO_FLAG_VAL(&(ppfs->info),left)) {
			_charpad(stream, ' ', numpad);
			numpad = 0;
		}
		OUTPUT(stream, prefix + prefix_num);
		_charpad(stream, '0', numfill);

#ifdef L_vfprintf

#ifdef __UCLIBC_HAS_WCHAR__
		if (!ws) {
			assert(s);
			_outnstr(stream, s, slen);
		} else {				/* wide string */
			size_t t;
			mbstate.mask = 0;	/* Initialize the mbstate. */
			while (slen) {
				t = (slen <= sizeof(buf)) ? slen : sizeof(buf);
				t = wcsrtombs(buf, &ws, t, &mbstate);
				assert (t != ((size_t)(-1)));
				_outnstr(stream, buf, t);
				slen -= t;
			}
		}
#else  /* __UCLIBC_HAS_WCHAR__ */
		_outnstr(stream, s, slen);
#endif /* __UCLIBC_HAS_WCHAR__ */

#else  /* L_vfprintf */

		if (!ws) {
			assert(s);
			_outnstr(stream, s, SLEN);
		} else {
			_outnwcs(stream, ws, SLEN);
		}

#endif /* L_vfprintf */
		_charpad(stream, ' ', numpad);
	}

	return 0;
}

int VFPRINTF (FILE * __restrict stream,
			  register const FMT_TYPE * __restrict format,
			  va_list arg)
{
	ppfs_t ppfs;
	int count, r;
	register const FMT_TYPE *s;

	__STDIO_THREADLOCK(stream);

	count = 0;
	s = format;

#if defined(L_vfprintf) && defined(__UCLIBC_HAS_WCHAR__)
	/* Sigh... I forgot that by calling _stdio_fwrite, vfprintf doesn't
	 * always check the stream's orientation.  This is just a temporary
	 * fix until I rewrite the stdio core work routines. */
	if (stream->modeflags & __FLAG_WIDE) {
		stream->modeflags |= __FLAG_ERROR;
		count = -1;
		goto DONE;
	}
	stream->modeflags |= __FLAG_NARROW;
#endif

	if (_PPFS_init(&ppfs, format) < 0) { /* Bad format string. */
		OUTNSTR(stream, (const FMT_TYPE *) ppfs.fmtpos,
				STRLEN((const FMT_TYPE *)(ppfs.fmtpos)));
#if defined(L_vfprintf) && !defined(NDEBUG)
		fprintf(stderr,"\nIMbS: \"%s\"\n\n", format);
#endif
		count = -1;
	} else {
		_ppfs_prepargs(&ppfs, arg);	/* This did a va_copy!!! */

		do {
			while (*format && (*format != '%')) {
				++format;
			}

			if (format-s) {		/* output any literal text in format string */
				if ( (r = OUTNSTR(stream, s, format-s)) < 0) {
					count = -1;
					break;
				}
				count += r;
			}

			if (!*format) {			/* we're done */
				break;
			}
		
			if (format[1] != '%') {	/* if we get here, *format == '%' */
				/* TODO: _do_one_spec needs to know what the output funcs are!!! */
				ppfs.fmtpos = (const char *)(++format);
				/* TODO: check -- should only fail on stream error */
				if ( (r = _do_one_spec(stream, &ppfs, &count)) < 0) {
					count = -1;
					break;
				}
				s = format = (const FMT_TYPE *) ppfs.fmtpos;
			} else {			/* %% means literal %, so start new string */
				s = ++format;
				++format;
			}
		} while (1);

		va_end(ppfs.arg);		/* Need to clean up after va_copy! */
	}

#if defined(L_vfprintf) && defined(__UCLIBC_HAS_WCHAR__)
 DONE:
#endif

	__STDIO_THREADUNLOCK(stream);

	return count;
}
#endif
/**********************************************************************/
