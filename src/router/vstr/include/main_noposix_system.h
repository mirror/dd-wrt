#ifndef MAIN_NOPOSIX_SYSTEM_H
#define MAIN_NOPOSIX_SYSTEM_H

#undef _GNU_SOURCE
#define _GNU_SOURCE 1

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#if STDC_HEADERS
# include <string.h>
#else
/* this probably needs more stuff in it -- stdarg.h ? */
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

#include <limits.h>
#include <ctype.h>

#ifndef USE_RESTRICTED_HEADERS /* dietlibc/klibc */
# include <math.h>
# include <wchar.h>
# include <locale.h>
#endif

#ifdef HAVE_MALLOC_H
# include <malloc.h>
#endif

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#else
# ifdef HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif

/* useful */
#ifndef FALSE
# define FALSE 0
#endif

#ifndef TRUE
# define TRUE 1
#endif

#endif
