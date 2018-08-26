/*
Copyright (C) 2004-2011 John E. Davis

This file is part of the S-Lang Library.

The S-Lang Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The S-Lang Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.
*/

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include "slinclud.h"

#include <limits.h>
#include <ctype.h>

#include "slang.h"
#include "_slang.h"

#ifndef ULONG_MAX
# define ULONG_MAX ((unsigned long)-1L)
#endif

#ifdef HAVE_LONG_LONG
# ifndef ULLONG_MAX
#  define ULLONG_MAX ((unsigned long long)-1LL)
# endif
#endif

char *SLmake_string (SLFUTURE_CONST char *str)
{
   return SLmake_nstring(str, strlen (str));
}

char *SLmake_nstring (SLFUTURE_CONST char *str, unsigned int n)
{
   char *ptr;

   if (NULL == (ptr = SLmalloc(n + 1)))
     {
	return NULL;
     }
   SLMEMCPY (ptr, str, n);
   ptr[n] = 0;
   return(ptr);
}

void SLmake_lut (unsigned char *lut, unsigned char *range, unsigned char reverse)
{
   /* register unsigned char *l = lut, *lmax = lut + 256; */
   int i, r1, r2;

   memset ((char *)lut, reverse, 256);
   /* while (l < lmax) *l++ = reverse; */
   reverse = !reverse;

   r1 = *range++;
   while (r1)
     {
	r2 = *range++;
	if ((r2 == '-') && (*range != 0))
	  {
	     r2 = *range++;
	     for (i = r1; i <= r2; i++) lut[i] = reverse;
	     r1 = *range++;
	     continue;
	  }
	lut[r1] = reverse;
	r1 = r2;
     }
}

char *_pSLskip_whitespace (SLCONST char *s)
{
   while (isspace (*s))
     s++;

   return (char *) s;
}

/*
 * This function assumes that the initial \ char has been removed.
 */
char *_pSLexpand_escaped_char(char *p, char *pmax, SLwchar_Type *ch, int *isunicodep)
{
   int i = 0;
   SLwchar_Type max = 0;
   SLwchar_Type num, base = 0;
   SLwchar_Type ch1;
   int isunicode;
   int needs_brace;

   if (p >= pmax)
     goto malformed_error;

   ch1 = *p++;
   isunicode = 0;
   needs_brace = 0;

   switch (ch1)
     {
      default: num = ch1; break;
      case 'n': num = '\n'; break;
      case 't': num = '\t'; break;
      case 'v': num = '\v'; break;
      case 'b': num = '\b'; break;
      case 'r': num = '\r'; break;
      case 'f': num = '\f'; break;
      case 'E': case 'e': num = 27; break;
      case 'a': num = 7;
	break;

	/* octal */
      case '0': case '1': case '2': case '3':
      case '4': case '5': case '6': case '7':
	max = '7';
	base = 8; i = 2; num = ch1 - '0';
	break;

      case 'd':			       /* decimal -- S-Lang extension */
	base = 10;
	i = 3;
	max = '9';
	num = 0;
	break;

      case 'u':
	isunicode = 1;
	/* drop */
      case 'x':			       /* hex */
	base = 16;
	max = '9';
	i = 2;
	num = 0;

	if (p == pmax)
	  goto malformed_error;

	if (*p == '{')
	  {
	     p++;
	     i = 0;
	     while (1)
	       {
		  if (p + i >= pmax)
		    goto malformed_error;

		  if ((p[i]==0) || (p[i] == '}'))
		    break;

		  i++;
	       }
	     if (p[i] != '}')
	       {
		  _pSLang_verror (SL_SYNTAX_ERROR, "Escaped character missing closing }.");
		  return NULL;
	       }
	     /* The meaning of \x{...} is mode dependent.  If in UTF-8 mode, then
	      * \x{...} always generates a unicode character.  Otherwise, the
	      * meaning of \x{...} depends upon the number of characters enclosed
	      * by the brace.  If there are less than 3, then assume no unicode.
	      * If greater than or equal to 3, then assume unicode.
	      */
	     if (isunicode == 0)       /* \x... */
	       isunicode = _pSLinterp_UTF8_Mode || (i > 2);
	     needs_brace = 1;
	  }
	break;
     }

   while (i && (p < pmax))
     {
	ch1 = *p;

	i--;

	if ((ch1 <= max) && (ch1 >= '0'))
	  {
	     num = base * num + (ch1 - '0');
	  }
	else if (base == 16)
	  {
	     ch1 |= 0x20;
	     if ((ch1 < 'a') || ((ch1 > 'f'))) break;
	     num = base * num + 10 + (ch1 - 'a');
	  }
	else break;
	p++;
     }

   if (needs_brace)
     {
	if ((p >= pmax) || (*p != '}'))
	  goto malformed_error;
	p++;
     }

   if (isunicodep != NULL)
     *isunicodep = isunicode;

   *ch = num;
   return p;

malformed_error:
   _pSLang_verror (SL_SYNTAX_ERROR, "Malformed escaped character.");
   return NULL;
}

/* s and t could represent the same space.  Note: the space for s is
 * assumed to be at least tmax-t bytes.
 */
int SLexpand_escaped_string (char *s, char *t, char *tmax, int utf8_encode)
{
   if (utf8_encode == -1)
     utf8_encode = _pSLinterp_UTF8_Mode;

   while (t < tmax)
     {
	SLwchar_Type wch;
	char *s1;
	int isunicode;
	char ch = *t++;

	if (ch != '\\')
	  {
	     *s++ = ch;
	     continue;
	  }

	if (NULL == (t = _pSLexpand_escaped_char (t, tmax, &wch, &isunicode)))
	  {
	     *s = 0;
	     return -1;
	  }

	/* As discussed above, the escaped representation is larger than the
	 * encoding.
	 */
	if ((isunicode == 0)
#if 0
	    && ((wch < 127)
		|| (utf8_encode == 0))
#endif
	    )
	  {
	     *s++ = (char) wch;
	     continue;
	  }

	s1 = (char *)SLutf8_encode (wch, (SLuchar_Type *)s, 6);
	if (s1 == NULL)
	  {
	     _pSLang_verror (SL_INVALID_UTF8, "Unable to UTF-8 encode 0x%lX\n", (unsigned long)wch);
	     *s = 0;
	     return -1;
	  }
	s = s1;
     }
   *s = 0;
   return 0;
}

int SLextract_list_element (SLFUTURE_CONST char *list, unsigned int nth, char delim,
			    char *elem, unsigned int buflen)
{
   char *el, *elmax;
   char ch;

   while (nth > 0)
     {
	while ((0 != (ch = *list)) && (ch != delim))
	  list++;

	if (ch == 0) return -1;

	list++;
	nth--;
     }

   el = elem;
   elmax = el + (buflen - 1);

   while ((0 != (ch = *list)) && (ch != delim) && (el < elmax))
     *el++ = *list++;
   *el = 0;

   return 0;
}

int SLvsnprintf (char *buf, unsigned int buflen, SLFUTURE_CONST char *fmt, va_list ap)
{
#ifdef HAVE_VSNPRINTF
   int status;

   status = vsnprintf(buf, buflen, fmt, ap);
   if ((unsigned int)status >= buflen)
     status = -1;
   return status;
#else
   unsigned int len;

   /* On some systems vsprintf returns useless information.  So, punt */
   vsprintf (buf, fmt, ap);
   len = strlen (buf);
   if (len >= buflen)
     {
	SLang_exit_error ("\
Your system lacks the vsnprintf system call and vsprintf overflowed a buffer.\n\
The integrity of this program has been violated.\n");
	return EOF;		       /* NOT reached */
     }
   return (int)len;
#endif
}

int SLsnprintf (char *buf, unsigned int buflen, SLFUTURE_CONST char *fmt, ...)
{
   int status;
   va_list ap;

   va_start (ap, fmt);
   status = SLvsnprintf (buf, buflen, fmt, ap);
   va_end (ap);

   return status;
}

int _pSLsnprintf (char *buf, unsigned int buflen, SLFUTURE_CONST char *fmt, ...)
{
   int status;
   va_list ap;

   va_start (ap, fmt);
   status = SLvsnprintf (buf, buflen, fmt, ap);
   va_end (ap);

   return status;
}

typedef struct _Cleanup_Function_Type
{
   struct _Cleanup_Function_Type *next;
   void (*f)(void);
}
Cleanup_Function_Type;

static Cleanup_Function_Type *Cleanup_Function_List;

static void cleanup_slang (void)
{
   while (Cleanup_Function_List != NULL)
     {
	Cleanup_Function_Type *next = Cleanup_Function_List->next;
	(*Cleanup_Function_List->f)();
	SLfree ((char *) Cleanup_Function_List);
	Cleanup_Function_List = next;
     }
}

#ifndef HAVE_ATEXIT
# ifdef HAVE_ON_EXIT
static void on_exit_cleanup_slang (int arg_unused)
{
   (void) arg_unused;
   cleanup_slang ();
}
# endif
#endif

int SLang_add_cleanup_function (void (*f)(void))
{
   Cleanup_Function_Type *l;

   l = (Cleanup_Function_Type *) SLmalloc (sizeof (Cleanup_Function_Type));
   if (l == NULL)
     return -1;

   l->f = f;
   l->next = Cleanup_Function_List;

   if (Cleanup_Function_List == NULL)
     {
#ifdef HAVE_ATEXIT
	(void) atexit (cleanup_slang);
#else
# ifdef HAVE_ON_EXIT
	(void) on_exit (on_exit_cleanup_slang, 0);
# endif
#endif
     }
   Cleanup_Function_List = l;
   return 0;
}

int SLang_guess_type (SLFUTURE_CONST char *t)
{
   SLCONST char *p;
   register char ch;
   int modifier;
#define MODIFIER_H	0x01
#define MODIFIER_L	0x02
#define MODIFIER_U	0x04
#define MODIFIER_LL	0x08
#define MODIFIER_SIZE_MASK 0x0F
#define MODIFIER_HEX	0x10
#define MODIFIER_BINARY	0x20

   modifier = 0;
   if ((*t == '-') || (*t == '+')) t++;
   p = t;

#if SLANG_HAS_FLOAT
   if (*p != '.')
     {
#endif
	while ((*p >= '0') && (*p <= '9')) p++;
	if (t == p) return (SLANG_STRING_TYPE);
	if (p == t + 1)
	  {
	     if (*p == 'x')
	       {
		  modifier |= MODIFIER_HEX;
		  p++;
		  while (ch = *p,
			 ((ch >= '0') && (ch <= '9'))
			 || (((ch | 0x20) >= 'a') && ((ch | 0x20) <= 'f'))) p++;
	       }
	     else if (*p == 'b')
	       {
		  modifier |= MODIFIER_BINARY;
		  p++;
		  while ((*p == '0') || (*p == '1'))
		    p++;
	       }
	  }

	/* Now look for U, H, L, LL, UH, UL, ULL, modifiers */
	ch = *p | 0x20;
	if (ch == 'u')
	  {
	     modifier |= MODIFIER_U;
	     p++;
	     ch = *p | 0x20;
	  }
	if (ch == 'h')
	  {
	     modifier |= MODIFIER_H;
	     p++;
	     ch = *p | 0x20;
	  }
	else if (ch == 'l')
	  {
	     p++;
	     ch = *p | 0x20;
	     if (ch == 'l')
	       {
		  modifier |= MODIFIER_LL;
		  p++;
		  ch = *p | 0x20;
	       }
	     else
	       modifier |= MODIFIER_L;
	  }
	if ((ch == 'u') && (0 == (modifier & MODIFIER_U)))
	  {
	     modifier |= MODIFIER_U;
	     p++;
	  }

	if (*p == 0) switch (modifier & MODIFIER_SIZE_MASK)
	  {
	   case 0:
	     return SLANG_INT_TYPE;
	   case MODIFIER_H:
	     return SLANG_SHORT_TYPE;
	   case MODIFIER_L:
	     return SLANG_LONG_TYPE;
	   case MODIFIER_U:
	     return SLANG_UINT_TYPE;
	   case MODIFIER_U|MODIFIER_H:
	     return SLANG_USHORT_TYPE;
	   case MODIFIER_U|MODIFIER_L:
	     return SLANG_ULONG_TYPE;
	   case MODIFIER_LL:
	     return SLANG_LLONG_TYPE;
	   case MODIFIER_U|MODIFIER_LL:
	     return SLANG_ULLONG_TYPE;
	   default:
	     return SLANG_STRING_TYPE;
	  }

	if (modifier)
	  return SLANG_STRING_TYPE;
#if SLANG_HAS_FLOAT
     }

   /* now down to double case */
   if (*p == '.')
     {
	p++;
	while ((*p >= '0') && (*p <= '9')) p++;
     }
   if (*p == 0) return(SLANG_DOUBLE_TYPE);
   if ((*p != 'e') && (*p != 'E'))
     {
# if SLANG_HAS_COMPLEX
	if (((*p == 'i') || (*p == 'j'))
	    && (p[1] == 0))
	  return SLANG_COMPLEX_TYPE;
# endif
	if (((*p | 0x20) == 'f') && (p[1] == 0))
	  return SLANG_FLOAT_TYPE;

	return SLANG_STRING_TYPE;
     }

   p++;
   if ((*p == '-') || (*p == '+')) p++;
   while ((*p >= '0') && (*p <= '9')) p++;
   if (*p != 0)
     {
# if SLANG_HAS_COMPLEX
	if (((*p == 'i') || (*p == 'j'))
	    && (p[1] == 0))
	  return SLANG_COMPLEX_TYPE;
# endif
	if (((*p | 0x20) == 'f') && (p[1] == 0))
	  return SLANG_FLOAT_TYPE;

	return SLANG_STRING_TYPE;
     }
   return SLANG_DOUBLE_TYPE;
#else
   return SLANG_STRING_TYPE;
#endif				       /* SLANG_HAS_FLOAT */
}

static int hex_atoul (unsigned char *s, unsigned long *ul)
{
   unsigned char ch;
   unsigned long value;
   int base;
   unsigned int num_processed;
   unsigned long max_value_before_overflow;
   unsigned long max_ch1_before_overflow;

   num_processed = 0;
   if (*s != '0')
     base = 10;
   else
     {
	s++;				       /* skip the leading 0 */

	/* look for 'x' which indicates hex */
	if ((*s | 0x20) == 'x')
	  {
	     base = 16;
	     s++;
	     if (*s == 0)
	       {
		  SLang_set_error (SL_SYNTAX_ERROR);
		  return -1;
	       }
	  }
	else if ((*s | 0x20) == 'b')
	  {
	     base = 2;
	     s++;
	     if (*s == 0)
	       {
		  SLang_set_error (SL_SYNTAX_ERROR);
		  return -1;
	       }
	  }
	else
	  {
	     base = 8;
	     num_processed++;
	  }
     }

   max_value_before_overflow = ULONG_MAX/base;
   max_ch1_before_overflow = ULONG_MAX - (base*max_value_before_overflow);
   value = 0;
   while ((ch = *s++) != 0)
     {
	char ch1 = ch | 0x20;
	switch (ch1)
	  {
	   default:
	     SLang_set_error (SL_SYNTAX_ERROR);
	     return -1;

	   case 'u':
	   case 'l':
	   case 'h':
	     if (num_processed == 0)
	       {
		  SLang_set_error (SL_SYNTAX_ERROR);
		  return -1;
	       }
	     *ul = value;
	     return 0;

	   case '9':
	   case '8':
	     if (base <= 8)
	       {
		  _pSLang_verror (SL_SYNTAX_ERROR, "8 or 9 are not permitted in binary or octal numbers");
		  return -1;
	       }
	     /* drop */
	   case '7':
	   case '6':
	   case '5':
	   case '4':
	   case '3':
	   case '2':
	     if (base <= 2)
	       {
		  _pSLang_verror (SL_SYNTAX_ERROR, "Only digits 0 and 1 are permitted in binary numbers");
		  return -1;
	       }
	   case '1':
	   case '0':
	     ch1 -= '0';
	     break;

	   case 'a':
	   case 'b':
	   case 'c':
	   case 'd':
	   case 'e':
	   case 'f':
	     if (base != 16)
	       {
		  _pSLang_verror (SL_SYNTAX_ERROR, "Only digits may appear in a binary, octal, or decimal number");
		  return -1;
	       }
	     ch1 = (ch1 - 'a') + 10;
	     break;
	  }
	num_processed++;

	if ((value < max_value_before_overflow)
	    || ((value == max_value_before_overflow)
		&& ((unsigned long)ch1 <= max_ch1_before_overflow)))
	  {
	     value = value * base + (unsigned long)ch1;
	     continue;
	  }

	_pSLang_verror (SL_SYNTAX_ERROR, "Integer overflow detected: too many digits");
	return -1;
     }
   *ul = value;
   return 0;
}

#ifdef HAVE_LONG_LONG
static int hex_atoull (unsigned char *s, unsigned long long *ul)
{
   unsigned char ch;
   unsigned long long value;
   unsigned long long max_value_before_overflow, max_ch1_before_overflow;
   int base;

   if (*s != '0')
     base = 10;
   else
     {
	s++;				       /* skip the leading 0 */

	/* look for 'x' which indicates hex */
	if ((*s | 0x20) == 'x')
	  {
	     base = 16;
	     s++;
	     if (*s == 0)
	       {
		  SLang_set_error (SL_SYNTAX_ERROR);
		  return -1;
	       }
	  }
	else if ((*s | 0x20) == 'b')
	  {
	     base = 2;
	     s++;
	     if (*s == 0)
	       {
		  SLang_set_error (SL_SYNTAX_ERROR);
		  return -1;
	       }
	  }
	else base = 8;
     }

   value = 0;
   max_value_before_overflow = ULLONG_MAX/base;
   max_ch1_before_overflow = ULLONG_MAX - base*max_value_before_overflow;

   while ((ch = *s++) != 0)
     {
	char ch1 = ch | 0x20;

	switch (ch1)
	  {
	   default:
	     SLang_set_error (SL_SYNTAX_ERROR);
	     return -1;

	   case 'u':
	   case 'l':
	     *ul = value;
	     return 0;

	   case '9':
	   case '8':
	     if (base <= 8)
	       {
		  _pSLang_verror (SL_SYNTAX_ERROR, "8 or 9 are not permitted in binary or octal numbers");
		  return -1;
	       }
	     /* drop */
	   case '7':
	   case '6':
	   case '5':
	   case '4':
	   case '3':
	   case '2':
	     if (base <= 2)
	       {
		  _pSLang_verror (SL_SYNTAX_ERROR, "Only digits 0 and 1 are permitted in binary numbers");
		  return -1;
	       }
	   case '1':
	   case '0':
	     ch1 -= '0';
	     break;

	   case 'a':
	   case 'b':
	   case 'c':
	   case 'd':
	   case 'e':
	   case 'f':
	     if (base != 16)
	       {
		  _pSLang_verror (SL_SYNTAX_ERROR, "Only digits may appear in a binary, octal, or decimal number");
		  return -1;
	       }
	     ch1 = (ch1 - 'a') + 10;
	     break;
	  }

	if ((value < max_value_before_overflow)
	    || ((value == max_value_before_overflow)
		&& ((unsigned long long)ch1 <= max_ch1_before_overflow)))
	  {
	     value = value * base + (unsigned long long)ch1;
	     continue;
	  }
	_pSLang_verror (SL_SYNTAX_ERROR, "Integer overflow detected: too many digits");
	return -1;
     }
   *ul = value;
   return 0;
}
#endif				       /* HAVE_LONG_LONG */

/* Note: These routines do not check integer overflow.  I would use the C
 * library functions atol and atoul but some implementations check overflow
 * and some do not.  The following implementations provide a consistent
 * behavior.
 */
static unsigned char *get_sign (SLCONST unsigned char *s, int *signp)
{
   s = (unsigned char *) _pSLskip_whitespace ((char *)s);

   if (*s == '-')
     {
	*signp = -1;
	s++;
     }
   else
     {
	*signp = 1;
	if (*s == '+') s++;
     }
   return (unsigned char *) s;
}

unsigned long SLatoul (unsigned char *s)
{
   int sign;
   unsigned long value;

   s = get_sign (s, &sign);
   if (-1 == hex_atoul (s, &value))
     return (unsigned long) -1;

   if (sign == -1)
     value = (unsigned long)-1L * value;

   return value;
}

long SLatol (unsigned char *s)
{
   int sign;
   unsigned long value;

   s = get_sign (s, &sign);
   if (-1 == hex_atoul (s, &value))
     return -1L;

   if (sign == -1)
     return (long)(-1L * value);

   return (long) value;
}

int SLatoi (unsigned char *s)
{
   return (int) SLatol (s);
}

#ifdef HAVE_LONG_LONG
long long SLatoll (unsigned char *s)
{
   int sign;
   unsigned long long value;

   s = get_sign (s, &sign);
   if (-1 == hex_atoull (s, &value))
     return -1LL;

   if (sign == -1)
     return (long long)(-1LL * value);

   return (long long) value;
}

unsigned long long SLatoull (unsigned char *s)
{
   int sign;
   unsigned long long value;

   s = get_sign (s, &sign);
   if (-1 == hex_atoull (s, &value))
     return (unsigned long long) -1;

   if (sign == -1)
     return (unsigned long long)(-1LL * value);

   return value;
}

#endif
