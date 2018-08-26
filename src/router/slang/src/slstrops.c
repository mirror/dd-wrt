/* -*- mode: C; mode: fold; -*- */
/* string manipulation functions for S-Lang. */
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

#include "slinclud.h"
/*{{{ Include Files */

#include <time.h>

#ifndef __QNX__
# if defined(__GO32__) || defined(__WATCOMC__)
#  include <dos.h>
#  include <bios.h>
# endif
#endif

#if SLANG_HAS_FLOAT
#include <math.h>
#endif

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#ifndef isdigit
# define isdigit(x) (((x) >= '0') && ((x) <= '9'))
#endif

#include "slang.h"
#include "_slang.h"

/*}}}*/

#define STRLEN(x,ignore_combining) (_pSLinterp_UTF8_Mode \
		      ? SLutf8_strlen ((SLuchar_Type *)(x), ignore_combining) \
		      : strlen (x))
#define SKIP_CHAR(s,smax) (_pSLinterp_UTF8_Mode \
			      ? SLutf8_skip_char ((s),(smax)) : (s)+1)

static SLwchar_Lut_Type *WhiteSpace_Lut;

_INLINE_
static SLwchar_Lut_Type *make_whitespace_lut (void)
{
   if (WhiteSpace_Lut != NULL)
     return WhiteSpace_Lut;

   return WhiteSpace_Lut = SLwchar_strtolut ((SLuchar_Type *)"\\s", 1, 1);
}

static SLwchar_Lut_Type *pop_lut (int *invertp)
{
   char *white;
   int invert;
   SLwchar_Lut_Type *lut;

   if (-1 == SLang_pop_slstring (&white))
     return NULL;
   if (*white == '^')
     invert = 1;
   else
     invert = 0;

   lut = SLwchar_strtolut ((SLuchar_Type *)white+invert, 1, 1);
   _pSLang_free_slstring (white);
   *invertp = invert;
   return lut;
}

static unsigned int do_trim (SLuchar_Type **beg, int do_beg,
			     SLuchar_Type **end, int do_end,
			     SLwchar_Lut_Type *lut, int invert) /*{{{*/
{
   unsigned int len;
   SLuchar_Type *a, *b;
   int ignore_combining = 0;

   a = *beg;
   len = _pSLstring_bytelen ((char *)a);
   b = a + len;

   if (do_beg)
     a = SLwchar_skip_range (lut, a, b, ignore_combining, invert);

   if (do_end)
     b = SLwchar_bskip_range (lut, a, b, ignore_combining, invert);

   len = (unsigned int) (b - a);
   *beg = a;
   *end = b;

   return len;
}

/*}}}*/

/*}}}*/

static int pop_3_malloced_strings (char **a, char **b, char **c)
{
   *a = *b = *c = NULL;
   if (-1 == SLpop_string (c))
     return -1;

   if (-1 == SLpop_string (b))
     {
	SLfree (*c);
	*c = NULL;
	return -1;
     }

   if (-1 == SLpop_string (a))
     {
	SLfree (*b);
	SLfree (*c);
	*b = *c = NULL;
	return -1;
     }

   return 0;
}

static void free_3_malloced_strings (char *a, char *b, char *c)
{
   SLfree (a);
   SLfree (b);
   SLfree (c);
}

static void strcat_cmd (void) /*{{{*/
{
   char *c, *c1;
   int nargs;
   int i;
   char **ptrs;
   unsigned int len;
   char *ptrs_buf[10];

   nargs = SLang_Num_Function_Args;
   if (nargs <= 0) nargs = 2;

   if (nargs <= 10)
     ptrs = ptrs_buf;
   else if (NULL == (ptrs = (char **)_SLcalloc (nargs, sizeof (char *))))
     return;

   memset ((char *) ptrs, 0, sizeof (char *) * nargs);

   c = NULL;
   i = nargs;
   len = 0;
   while (i != 0)
     {
	char *s;

	i--;
	if (-1 == SLang_pop_slstring (&s))
	  goto free_and_return;
	ptrs[i] = s;
	len += _pSLstring_bytelen (s);
     }
   if (NULL == (c = _pSLallocate_slstring (len)))
     goto free_and_return;

   c1 = c;
   for (i = 0; i < nargs; i++)
     {
	unsigned int len2 = _pSLstring_bytelen (ptrs[i]);
	memcpy (c1, ptrs[i], len2);
	c1 += len2; /* c1 does not take advantage of the slstring cache */
     }
   *c1 = 0;

   free_and_return:
   for (i = 0; i < nargs; i++)
     _pSLang_free_slstring (ptrs[i]);
   if (ptrs != ptrs_buf)
     SLfree ((char *) ptrs);

   (void) _pSLpush_alloced_slstring (c, len);   /* NULL ok */
}

/*}}}*/

_INLINE_
static int _pSLang_push_nstring (char *a, unsigned int len)
{
   a = SLang_create_nslstring (a, len);
   if (a == NULL)
     return -1;

   if (0 == SLclass_push_ptr_obj (SLANG_STRING_TYPE, (VOID_STAR)a))
     return 0;

   SLang_free_slstring (a);
   return -1;
}

static int str_replace_cmd_1 (char *orig, char *match, char *rep, unsigned int max_num_replaces,
			      char **new_strp) /*{{{*/
{
   char *s, *t, *new_str;
   unsigned int rep_len, match_len, new_len;
   unsigned int num_replaces;

   *new_strp = NULL;

   match_len = strlen (match);

   if (match_len == 0)
     return 0;

   num_replaces = 0;
   s = orig;
   while (num_replaces < max_num_replaces)
     {
	s = strstr (s, match);
	if (s == NULL)
	  break;
	s += match_len;
	num_replaces++;
     }

   if (num_replaces == 0)
     return 0;

   max_num_replaces = num_replaces;

   rep_len = strlen (rep);

   new_len = (strlen (orig) - num_replaces * match_len) + num_replaces * rep_len;
   new_str = SLmalloc (new_len + 1);
   if (new_str == NULL)
     return -1;

   s = orig;
   t = new_str;

   for (num_replaces = 0; num_replaces < max_num_replaces; num_replaces++)
     {
	char *next_s;
	unsigned int len;

	next_s = strstr (s, match);    /* cannot be NULL */
	len = (unsigned int) (next_s - s);
	memcpy (t, s, len);
	t += len;
	memcpy (t, rep, rep_len);
	t += rep_len;

	s = next_s + match_len;
     }
   strcpy (t, s);		       /* will \0 terminate t */
   *new_strp = new_str;

   return (int) num_replaces;
}

/*}}}*/

static void reverse_string (char *a)
{
   char *b;

   b = a + strlen (a);
   while (b > a)
     {
	char ch;

	b--;
	ch = *a;
	*a++ = *b;
	*b = ch;
     }
}

static void strreplace_cmd (void)
{
   char *orig, *match, *rep;
   char *new_str;
   int max_num_replaces = -1;
   int ret;
   int has_max_num_replaces;

   has_max_num_replaces = (SLang_Num_Function_Args == 4);
   if (has_max_num_replaces
       && (-1 == SLang_pop_int (&max_num_replaces)))
     return;

   if (-1 == pop_3_malloced_strings (&orig, &match, &rep))
     return;

   if (has_max_num_replaces == 0)
     max_num_replaces = _pSLstring_bytelen (orig);

   if (max_num_replaces < 0)
     {
	reverse_string (orig);
	reverse_string (match);
	reverse_string (rep);
	ret = str_replace_cmd_1 (orig, match, rep, -max_num_replaces, &new_str);
	if (ret > 0) reverse_string (new_str);
	else if (ret == 0)
	  reverse_string (orig);
     }
   else ret = str_replace_cmd_1 (orig, match, rep, max_num_replaces, &new_str);

   if (ret >= 0)
     {
	if (ret == 0)
	  {
	     (void) SLang_push_malloced_string (orig);   /* Always frees orig */
	     orig = NULL;
	  }
	else
	  (void) SLang_push_malloced_string (new_str);

	if (has_max_num_replaces)
	  (void) SLang_push_integer (ret);
     }
   free_3_malloced_strings (orig, match, rep);
}

/* FIXME: This function is deprecated and should removed  */
static int str_replace_cmd (char *orig, char *match, char *rep)
{
   char *s;
   int ret;

   ret = str_replace_cmd_1 (orig, match, rep, 1, &s);
   if (ret == 1)
     (void) SLang_push_malloced_string (s);
   return ret;
}

static void strtok_cmd (char *str)
{
   _pSLString_List_Type sl;
   SLuchar_Type *s, *smax, *white;
   SLwchar_Lut_Type *lut;
   int invert;
   int ignore_combining = 0;

   invert = 0;

   if (SLang_Num_Function_Args == 1)
     {
	white = NULL;
	lut = make_whitespace_lut ();
     }
   else
     {
	white = (SLuchar_Type *)str;
	if (-1 == SLang_pop_slstring (&str))
	  return;
	if (*white == '^')
	  {
	     invert = 1;
	     white++;
	  }
	lut = SLwchar_strtolut (white, 1, 1);
     }

   if ((lut == NULL)
       || (-1 == _pSLstring_list_init (&sl, 256, 1024)))
     goto the_return;

   s = (SLuchar_Type *)str;
   smax = s + _pSLstring_bytelen ((SLstr_Type *)s);
   while (s < smax)
     {
	SLuchar_Type *s0;
	char *new_s;

	/* Skip whitespace */
	s0 = SLwchar_skip_range (lut, s, smax, ignore_combining, invert);

	if (s0 == smax)
	  break;

	/* skip non-whitespace */
	s = SLwchar_skip_range (lut, s0, smax, ignore_combining, !invert);

	new_s = SLang_create_nslstring ((char *)s0, (unsigned int) (s - s0));
	if (new_s == NULL)
	  {
	     _pSLstring_list_delete (&sl);
	     goto the_return;
	  }
	if (-1 == _pSLstring_list_append (&sl, new_s))
	  {
	     _pSLang_free_slstring (new_s);
	     _pSLstring_list_delete (&sl);
	     goto the_return;
	  }
     }

   /* Deletes sl */
   (void) _pSLstring_list_push (&sl, 1);

   the_return:
   if (white != NULL)
     {
	_pSLang_free_slstring (str);
	SLwchar_free_lut (lut);
     }
}

/* This routine returns the string with text removed between single character
   comment delimiters from the set b and e. */

static void str_uncomment_string_cmd (char *str, char *b, char *e) /*{{{*/
{
   SLuchar_Type *s, *smax, *bmax, *emax;
   SLwchar_Lut_Type *lut;
   unsigned int len, elen, blen;
   SLuchar_Type *etable;
   SLuchar_Type *b1, *e1;
   int ignore_combining = 0;

   blen = _pSLstring_bytelen (b);
   elen = _pSLstring_bytelen (e);

   if (((elen != blen) && (_pSLinterp_UTF8_Mode == 0))
       || (_pSLinterp_UTF8_Mode
	   && SLutf8_strlen ((SLuchar_Type*)b,ignore_combining) != SLutf8_strlen ((SLuchar_Type*)e,ignore_combining)))
     {
	_pSLang_verror (SL_INVALID_PARM, "Comment delimiter length mismatch.");
	return;
     }

   if (NULL == (etable = (SLuchar_Type *) _SLcalloc (blen, (SLUTF8_MAX_MBLEN+1))))
     return;

   b1 = (SLuchar_Type *) b;
   e1 = (SLuchar_Type *) e;
   emax = e1 + elen;
   bmax = b1 + blen;

   if (_pSLinterp_UTF8_Mode)
     {
	while (b1 < bmax)
	  {
	     SLuchar_Type *et = etable + (SLUTF8_MAX_MBLEN+1)*((char*)b1-b);
	     e1 = SLutf8_extract_utf8_char (e1, emax, et);
	     b1 = SLutf8_skip_char (b1, bmax);
	  }
     }
   else
     {
	while (b1 < bmax)
	  {
	     SLuchar_Type *et = etable + (SLUTF8_MAX_MBLEN+1)*((char*)b1-b);
	     *et++ = *e1++; *et = 0;
	     b1++;
	  }
     }

   if (NULL == (lut = SLwchar_strtolut ((SLuchar_Type *)b, 0, 0)))
     {
	SLfree ((char *)etable);
	return;
     }

   len = strlen (str);

   if (NULL == (str = (char *) SLmake_nstring(str, len)))
     {
	SLwchar_free_lut (lut);
	SLfree ((char *)etable);
	return;
     }

   s = (SLuchar_Type *) str;
   smax = s + len;

   while (s < smax)
     {
	SLuchar_Type *s1, *s2;
	SLuchar_Type buf[SLUTF8_MAX_MBLEN+1];

	s = SLwchar_skip_range (lut, s, smax, ignore_combining, 1);
	if (s == smax)
	  break;

	/* s is now at the position of the comment-start character.
	 * Skip to the corresponding comment-end character.
	 */
	if (_pSLinterp_UTF8_Mode)
	  s1 = SLutf8_extract_utf8_char (s, smax, buf);
	else
	  {
	     s1 = s;
	     buf[0] = *s1++;
	     buf[1] = 0;
	  }

	/* buf contains the comment-start character.  Find the corresponding
	 * comment end via the etable constructed above.  The use of
	 * strstr should not fail, unless there is an error in the algorithm.
	 */
	e1 = etable + (strstr (b, (char*)buf) - b) * (SLUTF8_MAX_MBLEN+1);

	s1 = (SLuchar_Type *)strstr ((char*)s1, (char*)e1);
	if (s1 != NULL)
	  s1 += strlen ((char*)e1);
	else
	  s1 = smax;

	/* Delete characters between s and s1 */
	s2 = s;
	while (s1 < smax)
	  *s2++ = *s1++;
	*s2 = 0;

	smax = s2;
     }
   (void) SLang_push_malloced_string (str);   /* frees str */
   SLwchar_free_lut (lut);
   SLfree ((char *)etable);
   return;
}

/*}}}*/

static void str_quote_string_cmd (char *str, char *quotes, SLwchar_Type *slash_ptr) /*{{{*/
{
   char *q, *q1;
   int slash;
   unsigned int len;
   SLwchar_Lut_Type *lut;
   SLuchar_Type slash_utf8 [SLUTF8_MAX_MBLEN+1];
   SLuchar_Type *s, *smax;
   unsigned int slash_len;
   int ignore_combining = 0;

   slash = *slash_ptr;

   if (NULL == _pSLinterp_encode_wchar (slash, slash_utf8, &slash_len))
     return;

   lut = SLwchar_strtolut ((SLuchar_Type *)quotes, 0, 0);
   if (lut == NULL)
     return;

   /* Make sure slash character gets escaped */
   if (-1 == SLwchar_add_range_to_lut (lut, slash, slash))
     {
	SLwchar_free_lut (lut);
	return;
     }

   /* calculate length */
   s = (SLuchar_Type *) str;
   len = strlen (str);
   smax = s + len;

   while (1)
     {
	SLuchar_Type *s1;

	s1 = SLwchar_skip_range (lut, s, smax, ignore_combining, 1);
	if (s1 == smax)
	  break;

	len += slash_len;
	s = SKIP_CHAR(s1, smax);
     }
   len++;			       /* null terminate */

   if (NULL == (q = SLmalloc(len)))
     {
	SLwchar_free_lut (lut);
	return;
     }

   s = (SLuchar_Type *) str; q1 = q;
   while (1)
     {
	SLuchar_Type *s1;
	unsigned int dlen;

	s1 = SLwchar_skip_range (lut, s, smax, ignore_combining, 1);

	dlen = (unsigned int) (s1 - s);
	memcpy (q1, (char *)s, dlen);
	q1 += dlen;

	if (s1 == smax)
	  break;

	memcpy (q1, (char *)slash_utf8, slash_len);
	q1 += slash_len;

	s = SKIP_CHAR(s1, smax);
	dlen = (unsigned int) (s - s1);
	memcpy (q1, s1, dlen);
	q1 += dlen;
     }
   *q1 = 0;
   (void) SLang_push_malloced_string(q);      /* frees q */
   SLwchar_free_lut (lut);
}

/*}}}*/

static void subbytes_cmd (SLstr_Type *a, int *n_ptr, int *m_ptr) /*{{{*/
{
   int m;
   unsigned int n;
   unsigned int lena;

   n = (unsigned int) (*n_ptr - 1);
   m = *m_ptr;

   lena = _pSLstring_bytelen (a);

   if (n > lena)
     n = lena;

   if (m < 0) m = lena;
   if (n + m > lena) m = lena - n;

   (void) _pSLang_push_nstring (a + n, (unsigned int) m);
}

/*}}}*/

static void substr_cmd (SLstr_Type *a, int *n_ptr, int *m_ptr) /*{{{*/
{
   int n, m;
   int lena;
   int ignore_combining;
   char *a1;

   if (_pSLinterp_UTF8_Mode == 0)
     {
	subbytes_cmd (a, n_ptr, m_ptr);
	return;
     }

   ignore_combining = 0;

   n = *n_ptr;
   m = *m_ptr;

   lena = SLutf8_strlen ((SLuchar_Type *)a, ignore_combining);

   if (n > lena) n = lena + 1;
   if (n < 1)
     {
	SLang_set_error (SL_INVALID_PARM);
	return;
     }

   n--;
   if (m < 0) m = lena;
   if (n + m > lena) m = lena - n;

   /* FIXME: Are the strlens necessary here?  */
   a = (char *)SLutf8_skip_chars ((SLuchar_Type *)a, (SLuchar_Type*)a + strlen(a),
				  (unsigned int)n, NULL, ignore_combining);

   a1 = (char *)SLutf8_skip_chars ((SLuchar_Type *)a, (SLuchar_Type*)a + strlen(a),
				  (unsigned int)m, NULL, ignore_combining);

   (void) _pSLang_push_nstring (a, (unsigned int)(a1 - a));
}

/*}}}*/

/* substitute byte ch at byte-position n in string*/
static void strbytesub_cmd (int *nptr, char *chp)
{
   char *a;
   unsigned int n;
   unsigned int lena;

   if (-1 == SLpop_string (&a))
     return;

   n = (unsigned int) (*nptr-1);
   lena = strlen (a);

   if (n >= lena)
     {
	SLang_set_error (SL_INVALID_PARM);
	SLfree(a);
	return;
     }

   a[n] = *chp;

   SLang_push_malloced_string (a);
}

/* substitute char m at position n in string*/
static void strsub_cmd (int *nptr, SLwchar_Type *mptr) /*{{{*/
{
   char *a;
   unsigned int n;
   SLwchar_Type m;
   unsigned int lena;
   int ignore_combining = 0;

   if (-1 == SLpop_string (&a))
     return;

   n = (unsigned int) *nptr;
   m = (SLwchar_Type) *mptr;

   lena = STRLEN (a, ignore_combining);

   if ((n == 0) || (lena < (unsigned int) n))
     {
	SLang_set_error (SL_INVALID_PARM);
	SLfree(a);
	return;
     }

   /* The API for this function specifies 1-based indices */
   n--;
   if (_pSLinterp_UTF8_Mode)
     {

	SLstr_Type *b = SLutf8_subst_wchar ((SLuchar_Type *)a,
					      (SLuchar_Type *)a + strlen (a),
					      m, n, ignore_combining);
	if (b != NULL)
	  _pSLang_push_slstring (b);   /* frees b */

	SLfree (a);
	return;
     }

   a[n] = (char) m;

   SLang_push_malloced_string (a);
}

/*}}}*/


static int pop_wchar (SLwchar_Type *wchp)
{
   if (SLang_peek_at_stack() == SLANG_STRING_TYPE)
     {
	char *s;
	SLwchar_Type wch;

	if (-1 == SLang_pop_slstring (&s))
	  return -1;

	if (_pSLinterp_UTF8_Mode)
	  {
	     if (NULL == SLutf8_decode ((unsigned char *)s, (unsigned char *)s+strlen(s), &wch, NULL))
	       wch = 0;
	  }
	else wch = s[0];

	_pSLang_free_slstring (s);
	*wchp = wch;
	return 0;
     }

   return SLang_pop_wchar (wchp);
}

#define ISXXX_INTRIN(name,isxxx) \
   static int name (void) \
   { \
      SLwchar_Type wch; \
      if (-1 == pop_wchar (&wch)) \
	return -1; \
      return (0 != isxxx (wch)); \
   }
ISXXX_INTRIN(islower_intrin, SLwchar_islower)
ISXXX_INTRIN(isupper_intrin, SLwchar_isupper)
ISXXX_INTRIN(isalpha_intrin, SLwchar_isalpha)
ISXXX_INTRIN(isxdigit_intrin, SLwchar_isxdigit)
ISXXX_INTRIN(isspace_intrin, SLwchar_isspace)
ISXXX_INTRIN(isblank_intrin, SLwchar_isblank)
ISXXX_INTRIN(iscntrl_intrin, SLwchar_iscntrl)
ISXXX_INTRIN(isprint_intrin, SLwchar_isprint)
ISXXX_INTRIN(isdigit_intrin, SLwchar_isdigit)
ISXXX_INTRIN(isgraph_intrin, SLwchar_isgraph)
ISXXX_INTRIN(isalnum_intrin, SLwchar_isalnum)
ISXXX_INTRIN(ispunct_intrin, SLwchar_ispunct)
static int isascii_fun (SLwchar_Type wch)
{
   return wch < 0x80;
}
ISXXX_INTRIN(isascii_intrin, isascii_fun)

static int pop_skipintrin_args (SLuchar_Type **strp, SLstrlen_Type *lenp, SLstrlen_Type *posp, int *skip_combp)
{
   char *str;
   SLstrlen_Type len, pos;

   *skip_combp = 1;
   if (SLang_Num_Function_Args == 3)
     {
	if (-1 == SLang_pop_int (skip_combp))
	  return -1;
     }
   if (-1 == SLang_pop_strlen_type (&pos))
     return -1;
   if (-1 == SLang_pop_slstring (&str))
     return -1;
   len = _pSLstring_bytelen (str);
   if (pos > len)
     {
	SLang_verror (SL_Index_Error, "%s", "String index lies outside the string");
	SLang_free_slstring (str);
	return -1;
     }
   *strp = (SLuchar_Type *)str;
   *lenp = len;
   *posp = pos;
   return 0;
}

/* Usage: (wc, pos) = strskipchar (str, pos [,skip_comb]);
 */
static void strskipchar_intrin (void)
{
   SLuchar_Type *str, *str0, *str1, *strmax;
   int skip_combining;
   SLstrlen_Type pos, len;
   SLwchar_Type wch;

   if (-1 == pop_skipintrin_args (&str, &len, &pos, &skip_combining))
     return;

   str0 = str + pos;
   strmax = str + len;
   if (str0 == strmax)
     {
	(void) SLang_push_strlen_type (pos);
	(void) SLang_push_uchar (0);
	goto free_and_return;
     }
   if (_pSLinterp_UTF8_Mode == 0)
     {
	(void) SLang_push_strlen_type (pos+1);
	(void) SLang_push_uchar (*str0);
	goto free_and_return;
     }
   str1 = SLutf8_skip_chars (str0, strmax, 1, NULL, skip_combining);
   pos = str1 - str;
   (void) SLang_push_strlen_type (pos);

   if (NULL == SLutf8_decode (str0, str1, &wch, NULL))
     {
	(void) SLang_push_integer (-(int)*str0);
	goto free_and_return;
     }
   (void) SLang_push_wchar (wch);
   /* drop */
free_and_return:
   SLang_free_slstring ((char *)str);
}

static void strbskipchar_intrin (void)
{
   SLuchar_Type *str, *str0, *str1;
   int skip_combining;
   SLstrlen_Type pos, len;
   SLwchar_Type wch;

   if (-1 == pop_skipintrin_args (&str, &len, &pos, &skip_combining))
     return;

   str0 = str + pos;
   if (pos == 0)
     {
	(void) SLang_push_strlen_type (pos);
	(void) SLang_push_uchar (0);
	goto free_and_return;
     }
   if (_pSLinterp_UTF8_Mode == 0)
     {
	(void) SLang_push_strlen_type (pos-1);
	(void) SLang_push_uchar (*str0);
	goto free_and_return;
     }
   str1 = SLutf8_bskip_chars (str, str0, 1, NULL, skip_combining);
   pos = str1 - str;
   (void) SLang_push_strlen_type (pos);

   if (NULL == SLutf8_decode (str1, str0, &wch, NULL))
     {
	(void) SLang_push_integer (-(int)*str1);
	goto free_and_return;
     }
   (void) SLang_push_wchar (wch);
   /* drop */
free_and_return:
   SLang_free_slstring ((char *)str);
}

static int toupper_cmd (SLwchar_Type *ch) /*{{{*/
{
   if (_pSLinterp_UTF8_Mode)
     return SLwchar_toupper (*ch);

   return UPPER_CASE(*ch);
}

/*}}}*/

static int tolower_cmd (SLwchar_Type *ch) /*{{{*/
{
   if (_pSLinterp_UTF8_Mode)
     return SLwchar_tolower (*ch);

   return LOWER_CASE(*ch);
}

/*}}}*/

static SLang_Array_Type *do_strchop (SLuchar_Type *str, SLwchar_Type delim, SLwchar_Type quote)
{
   SLindex_Type count;
   SLuchar_Type *s0, *s1;
   SLang_Array_Type *at;
   char **data;
   SLuchar_Type delim_utf8 [SLUTF8_MAX_MBLEN+1];
   SLuchar_Type quote_utf8 [SLUTF8_MAX_MBLEN+1];
   unsigned int delim_len, quote_len;
   SLwchar_Lut_Type *lut;
   SLuchar_Type *smax;
   int ignore_combining = 0;

   if (NULL == _pSLinterp_encode_wchar ((SLwchar_Type)delim, delim_utf8, &delim_len))
     return NULL;
   if (NULL == _pSLinterp_encode_wchar ((SLwchar_Type)quote, quote_utf8, &quote_len))
     return NULL;

   if (NULL == (lut = SLwchar_create_lut (2)))
     return NULL;

   if ((-1 == SLwchar_add_range_to_lut (lut, delim, delim))
       || ((quote != 0)
	   && (-1 == SLwchar_add_range_to_lut (lut, quote, quote))))
     {
	SLwchar_free_lut (lut);
	return NULL;
     }

   smax = str + strlen ((char *) str);
   s1 = s0 = str;

   count = 1;			       /* at least 1 */

   /* Count strings on first pass */
   while (1)
     {
	SLwchar_Type wch;

	/* Look for the delimiter or the quote */
	s1 = SLwchar_skip_range (lut, s1, smax, ignore_combining, 1);

	if (s1 == smax)
	  break;

	/* Test for quote */
	if (NULL == (s1 = _pSLinterp_decode_wchar (s1, smax, &wch)))
	  {
	     SLwchar_free_lut (lut);
	     return NULL;
	  }

	if ((wch == quote) && quote)
	  {
	     if (s1 == smax)
	       break;

	     s1 = SKIP_CHAR(s1, smax);
	     continue;
	  }

	if (wch == delim)
	  {
	     count++;
	     continue;
	  }
     }

   if (NULL == (at = SLang_create_array (SLANG_STRING_TYPE, 0, NULL, &count, 1)))
     {
	SLwchar_free_lut (lut);
	return NULL;
     }

   data = (char **)at->data;

   count = 0;
   s1 = str;

   while (1)
     {
	SLwchar_Type wch;
	char *elm;

	/* Look for the delimiter or the quote */
	s1 = SLwchar_skip_range (lut, s1, smax, ignore_combining, 1);

	if (s1 != smax)
	  {
	     SLuchar_Type *s1_save = s1;

	     if (NULL == (s1 = _pSLinterp_decode_wchar (s1, smax, &wch)))
	       break;

	     if ((wch == quote) && quote)
	       {
		  if (s1 != smax)
		    s1 = SKIP_CHAR (s1, smax);

		  continue;
	       }
	     s1 = s1_save;
	     /* Otherwise it must be the delim */
	  }
	elm = SLang_create_nslstring ((char *)s0, (unsigned int) (s1 - s0));

	if (elm == NULL)
	  break;

	data[count] = elm;
	count++;

	if (s1 == smax)
	  {
	     SLwchar_free_lut (lut);
	     return at;
	  }

	s1 = SKIP_CHAR(s1, smax);      /* skip past delim */
	s0 = s1;		       /* and reset */
     }

   SLwchar_free_lut (lut);
   SLang_free_array (at);
   return NULL;
}

static void strchop_cmd (char *str, SLwchar_Type *q, SLwchar_Type *d)
{
   (void) SLang_push_array (do_strchop ((SLuchar_Type *)str, *q, *d), 1);
}

static void strchopr_cmd (char *str, SLwchar_Type *q, SLwchar_Type *d)
{
   SLang_Array_Type *at;

   if (NULL != (at = do_strchop ((SLuchar_Type *)str, *q, *d)))
     {
	char **d0, **d1;

	d0 = (char **) at->data;
	d1 = d0 + (at->num_elements - 1);

	while (d0 < d1)
	  {
	     char *tmp;

	     tmp = *d0;
	     *d0 = *d1;
	     *d1 = tmp;
	     d0++;
	     d1--;
	  }
     }
   SLang_push_array (at, 1);
}

/*}}}*/

typedef struct
{
   SLstr_Type **sp;
   SLuindex_Type num;
   SLstr_Type *str;
   SLang_Array_Type *at;
}
Array_Or_String_Type;

static int pop_array_or_string (Array_Or_String_Type *aos)
{
   char *str;

   if (SLang_peek_at_stack () == SLANG_ARRAY_TYPE)
     {
	SLang_Array_Type *at;
	aos->str = NULL;
	if (-1 == SLang_pop_array_of_type (&at, SLANG_STRING_TYPE))
	  {
	     aos->at = NULL;
	     return -1;
	  }
	aos->num = at->num_elements;
	aos->sp = (char **)at->data;
	aos->at = at;
	return 0;
     }
   aos->at = NULL;
   if (-1 == SLang_pop_slstring (&str))
     {
	aos->str = NULL;
	return -1;
     }
   aos->num = 1;
   aos->sp = &str;
   aos->str = str;
   return 0;
}

static void free_array_or_string (Array_Or_String_Type *aos)
{
   if (aos->str != NULL)
     {
	SLang_free_slstring (aos->str);
	return;
     }
   if (aos->at != NULL)
     {
	SLang_free_array (aos->at);
	return;
     }
}

static int pop_matched_array_or_string (Array_Or_String_Type *aos, Array_Or_String_Type *bos,
					int *is_arrayp)
{
   if (-1 == pop_array_or_string (bos))
     return -1;
   if (-1 == pop_array_or_string (aos))
     {
	free_array_or_string (bos);
	return -1;
     }
   if (0 == (*is_arrayp = (aos->at != NULL) || (bos->at != NULL)))
     return 0;

   if ((aos->num != bos->num)
       && (aos->at != NULL) && (bos->at != NULL))
     {
	SLang_verror (SL_InvalidParm_Error, "String arrays must be the same length.");
	free_array_or_string (aos);
	free_array_or_string (bos);
	return -1;
     }
   return 0;
}

static int arraymap_int_func_str_str (int (*func)(char *, char *, void *), void *cd)
{
   int status = -1;
   int is_array;
   Array_Or_String_Type aos, bos;
   SLang_Array_Type *int_at;
   int *int_at_data;
   SLuindex_Type i, num;

   if (-1 == pop_matched_array_or_string (&aos, &bos, &is_array))
     return -1;
   if (0 == is_array)
     {
	status = SLang_push_int ((*func)(aos.str, bos.str, cd));
	goto free_and_return;
     }

   if (aos.at != NULL)
     {
	char **astrs, **bstrs;
	if (NULL == (int_at = (SLang_create_array1 (SLANG_INT_TYPE, 0, NULL, aos.at->dims, aos.at->num_dims, 0))))
	  goto free_and_return;

	int_at_data = (int *)int_at->data;
	num = aos.num;
	astrs = aos.sp;
	if (bos.at == NULL)
	  {
	     char *b = bos.str;
	     for (i = 0; i < num; i++)
	       int_at_data[i] = (*func)(astrs[i], b, cd);
	     goto push_and_return;
	  }
	bstrs = bos.sp;
	for (i = 0; i < num; i++)
	  int_at_data[i] = (*func)(astrs[i], bstrs[i], cd);
	goto push_and_return;
     }

   if (NULL == (int_at = (SLang_create_array1 (SLANG_INT_TYPE, 0, NULL, bos.at->dims, bos.at->num_dims, 0))))
     goto free_and_return;

   int_at_data = (int *)int_at->data;
   num = bos.num;
   for (i = 0; i < num; i++)
     int_at_data[i] = (*func)(aos.str, bos.sp[i], cd);

   /* drop */

push_and_return:
   status = SLang_push_array (int_at, 1);
   /* drop */
free_and_return:
   free_array_or_string (&aos);
   free_array_or_string (&bos);
   return status;
}

static int arraymap_int_func_str (int (*func)(char *, void *), void *cd)
{
   SLang_Array_Type *int_at, *at;
   SLuindex_Type i, num;
   int *int_at_data;
   char **at_data;

   if (SLang_peek_at_stack () != SLANG_ARRAY_TYPE)
     {
	int status;
	char *str;

	if (-1 == SLang_pop_slstring (&str))
	  return -1;
	status = SLang_push_int ((*func)(str, cd));
	SLang_free_slstring (str);
	return status;
     }

   if (-1 == SLang_pop_array_of_type (&at, SLANG_STRING_TYPE))
     return -1;

   if (NULL == (int_at = (SLang_create_array1 (SLANG_INT_TYPE, 0, NULL, at->dims, at->num_dims, 0))))
     {
	SLang_free_array (at);
	return -1;
     }

   at_data = (char **)at->data;
   int_at_data = (int *)int_at->data;
   num = at->num_elements;
   for (i = 0; i < num; i++)
     int_at_data[i] = (*func)(at_data[i], cd);

   SLang_free_array (at);
   return SLang_push_array (int_at, 1);
}

static int arraymap_str_func_str (char *(*func)(char *, void *), void *cd)
{
   SLang_Array_Type *at, *bt;
   SLuindex_Type i, num;
   char **adata, **bdata;

   if (SLang_peek_at_stack () != SLANG_ARRAY_TYPE)
     {
	char *a, *b;

	if (-1 == SLang_pop_slstring (&a))
	  return -1;

	b = (*func)(a, cd);
	SLang_free_slstring (a);
	return _pSLang_push_slstring (b);   /* frees string */
     }

   if (-1 == SLang_pop_array_of_type (&at, SLANG_STRING_TYPE))
     return -1;

   if (NULL == (bt = SLang_create_array (SLANG_STRING_TYPE, 0, NULL, at->dims, at->num_dims)))
     {
	SLang_free_array (at);
	return -1;
     }

   adata = (char **)at->data; bdata = (char **)bt->data;
   num = bt->num_elements;
   for (i = 0; i < num; i++)
     {
	char *s = adata[i];
	if (s != NULL)
	  {
	     s = (*func)(s, cd);
	     if (s == NULL)
	       {
		  SLang_free_array (bt);
		  SLang_free_array (at);
		  return -1;
	       }
	  }
	bdata[i] = s;
     }
   SLang_free_array (at);
   return SLang_push_array (bt, 1);
}

static int func_issubstr (char *a, char *b, void *cd)
{
   SLstrlen_Type n;
   char *c;

   (void) cd;

   if (NULL == (c = strstr(a, b)))
     return 0;

   if (_pSLinterp_UTF8_Mode == 0)
     return 1 + (int) (c - a);

   n = (unsigned int) (c - a);
   (void) SLutf8_skip_chars ((SLuchar_Type *)a, (SLuchar_Type *)c, n, &n, 0);
   return (int) (n+1);
}

static int issubstr_vintrin (void) /*{{{*/
{
   return arraymap_int_func_str_str (func_issubstr, NULL);
}


typedef struct
{
   int do_beg, do_end;
   SLwchar_Lut_Type *lut;
   int invert;
}
Strtrim_CD_Type;

static char *func_strtrim (char *str, void *cd)
{
   Strtrim_CD_Type *info;
   SLuchar_Type *beg, *end;
   unsigned int len;

   info = (Strtrim_CD_Type *)cd;

   beg = (SLuchar_Type *)str;
   len = do_trim (&beg, info->do_beg, &end, info->do_end, info->lut, info->invert);

   return SLang_create_nslstring ((char *) beg, len);
}

static int strtrim_internal (int do_beg, int do_end)
{
   Strtrim_CD_Type cd;
   int status;
   int free_lut;

   cd.do_beg = do_beg;
   cd.do_end = do_end;
   cd.invert = 0;
   free_lut = 0;
   if (SLang_Num_Function_Args == 2)
     {
	cd.lut = pop_lut (&cd.invert);
	free_lut = 1;
     }
   else cd.lut = make_whitespace_lut ();

   status = arraymap_str_func_str (func_strtrim, &cd);
   if (free_lut) SLwchar_free_lut (cd.lut);
   return status;
}

static void strtrim_vintrin (void)
{
   (void) strtrim_internal (1, 1);
}

static void strtrim_beg_vintrin (void)
{
   (void) strtrim_internal (1, 0);
}

static void strtrim_end_vintrin (void)
{
   (void) strtrim_internal (0, 1);
}

static char *func_strup (char *str, void *cd)
{
   unsigned int i, len;
   unsigned char *a;

   (void) cd;
   len = strlen (str);

   if (_pSLinterp_UTF8_Mode)
     return (char *)SLutf8_strup ((SLuchar_Type *)str, (SLuchar_Type *)str+len);

   if (NULL == (a = (unsigned char *)SLmalloc (len+1)))
     return NULL;

   for (i = 0; i < len; i++)
     {
	unsigned char c = (unsigned char)str[i];
	a[i] = UPPER_CASE(c);
     }
   a[len] = 0;
   str = SLang_create_nslstring ((char *)a, len);
   SLfree ((char *)a);
   return str;
}

static void strup_vintrin (void)
{
   (void) arraymap_str_func_str (&func_strup, NULL);
}

static char *func_strlow (char *str, void *cd)
{
   unsigned int i, len;
   unsigned char *a;

   (void) cd;
   len = strlen (str);

   if (_pSLinterp_UTF8_Mode)
     return (char *)SLutf8_strlo ((SLuchar_Type *)str, (SLuchar_Type *)str+len);

   if (NULL == (a = (unsigned char *)SLmalloc (len+1)))
     return NULL;

   for (i = 0; i < len; i++)
     {
	unsigned char c = (unsigned char)str[i];
	a[i] = LOWER_CASE(c);
     }
   a[len] = 0;
   str = SLang_create_nslstring ((char *)a, len);
   SLfree ((char *)a);
   return str;
}

static void strlow_vintrin (void)
{
   (void) arraymap_str_func_str (&func_strlow, NULL);
}

static int func_strcmp (char *a, char *b, void *cd)
{
   (void) cd;

   if (a == b)
     return 0;

   if ((a != NULL) && (b != NULL))
     return strcmp(a, b);

   if (a == NULL)
     return -1;

   return 1;
}
static void strcmp_vintrin (void)
{
   (void) arraymap_int_func_str_str (func_strcmp, NULL);
}

static int func_strnbytecmp (char *a, char *b, void *cd)
{
   if ((a != NULL) && (b != NULL))
     return strncmp (a, b, *(unsigned int *)cd);
   if (a == NULL)
     return b == NULL ? 0 : -1;
   return 1;
}

static void strnbytecmp_vintrin (void)
{
   unsigned int n;
   if (0 == SLang_pop_uint (&n))
     (void) arraymap_int_func_str_str (func_strnbytecmp, (void *)&n);
}

typedef struct
{
   unsigned int n;
   int skip_combining;
}
Strncmp_CD_Type;

static int func_strncmp (char *a, char *b, void *cd)
{
   int skip_combining;
   unsigned int n;
   char *p;
   unsigned int na, nb;
   unsigned int lena, lenb;
   int cmp;

   if (a == NULL)
     return b == NULL ? 0 : -1;
   if (b == NULL)
     return 1;

   skip_combining = ((Strncmp_CD_Type *)cd)->skip_combining;
   n = ((Strncmp_CD_Type *)cd)->n;

   lena = _pSLstring_bytelen (a);
   lenb = _pSLstring_bytelen (b);

   p = (char *)SLutf8_skip_chars ((SLuchar_Type *)a, (SLuchar_Type *)(a + lena),
                          (unsigned int) n, NULL, skip_combining);
   na = (unsigned int) (p - a);

   p = (char *)SLutf8_skip_chars ((SLuchar_Type *)b, (SLuchar_Type *)(b + lenb),
                          (unsigned int) n, NULL, skip_combining);
   nb = (unsigned int) (p - b);

   if (na > nb)
     {
        cmp = strncmp (a, b, nb);
        if (cmp == 0)
          return (int) (unsigned char) a[nb];
        return cmp;
     }

   if (na == nb)
     return strncmp (a, b, nb);

   /* nb > na */
   cmp = strncmp (a, b, na);
   if (cmp == 0)
     return -(int)(unsigned char) b[na];

   return cmp;
}

static void strncmp_vintrin (void)
{
   Strncmp_CD_Type cd;

   if (_pSLinterp_UTF8_Mode == 0)
     {
	strnbytecmp_vintrin ();
	return;
     }
   if (-1 == SLang_pop_uint (&cd.n))
     return;
   cd.skip_combining = 1;

   (void) arraymap_int_func_str_str (func_strncmp, (void *)&cd);
}

static void strncharcmp_vintrin (void)
{
   Strncmp_CD_Type cd;

   if (_pSLinterp_UTF8_Mode == 0)
     {
	strnbytecmp_vintrin ();
	return;
     }
   if (-1 == SLang_pop_uint (&cd.n))
     return;
   cd.skip_combining = 0;

   (void) arraymap_int_func_str_str (func_strncmp, (void *)&cd);
}

static int func_utf8_strlen (char *s, void *cd)
{
   if (s == NULL)
     return 0;
   return (int) SLutf8_strlen ((SLuchar_Type *)s, *(int *)cd);
}

static int func_bytelen (char *s, void *cd)
{
   (void) cd;
   if (s == NULL)
     return 0;
   return (int) _pSLstring_bytelen (s);
}

static void strlen_vintrin (void)
{
   int ignore_combining = 1;

   if (_pSLinterp_UTF8_Mode == 0)
     {
	(void) arraymap_int_func_str (&func_bytelen, NULL);
	return;
     }
   (void) arraymap_int_func_str (&func_utf8_strlen, (void *)&ignore_combining);
}

static void strcharlen_vintrin (void)
{
   int ignore_combining = 0;

   if (_pSLinterp_UTF8_Mode == 0)
     {
	(void) arraymap_int_func_str (&func_bytelen, NULL);
	return;
     }
   (void) arraymap_int_func_str (&func_utf8_strlen, (void *)&ignore_combining);
}

static void strbytelen_vintrin (void)
{
   (void) arraymap_int_func_str (&func_bytelen, NULL);
}

/*}}}*/

typedef struct
{
   SLwchar_Lut_Type *lut;
   int invert;
}
Str_Delete_Chars_CD_Type;

static char *func_str_delete_chars (char *str, void *cd)
{
   SLwchar_Lut_Type *lut;
   SLuchar_Type *s1;
   SLuchar_Type *t, *tmax;
   int invert, ignore_combining = 0;
   SLuchar_Type *s;

   lut = ((Str_Delete_Chars_CD_Type *)cd)->lut;
   invert = ((Str_Delete_Chars_CD_Type *)cd)->invert;

   /* Assume that the number of characters to be deleted is smaller then
    * the number not to be deleted.  In this case, it is better to
    * skip past call characters not in the set to be deleted.  Hence,
    * we want to invert the deletion set.
    */
   invert = !invert;

   if ((str == NULL)
       || (NULL == (s = (SLuchar_Type *)SLmake_string (str))))
     return NULL;

   s1 = s;
   t = s;
   tmax = t + strlen((char *)t);
   while (t != tmax)
     {
	SLuchar_Type *t1;
	unsigned int len;

	t1 = SLwchar_skip_range (lut, t, tmax, ignore_combining, invert);
	if (t1 == NULL)
	  break;

	len = t1 - t;
	if (len)
	  {
	     if (t != s1)
	       {
		  /* strncpy ((char *)s1, (char *)t, len); */
		  while (t < t1)
		    *s1++ = *t++;
	       }
	     else s1 += len;
	  }
	t = SLwchar_skip_range (lut, t1, tmax, ignore_combining, !invert);
	if (t == NULL)
	  break;
     }
   *s1 = 0;
   str = SLang_create_slstring ((char *)s);
   SLfree ((char *)s);
   return str;
}

static void str_delete_chars_vintrin (void)
{
   Str_Delete_Chars_CD_Type cd;
   int free_lut;

   cd.invert = 0;
   free_lut = 0;

   /* This function may also be called by strtrans_vintrin, with some of its
    * args on the stack.
    */
   if (SLang_Num_Function_Args > 1)
     {
	cd.lut = pop_lut (&cd.invert);
	free_lut = 1;
     }
   else
     cd.lut = make_whitespace_lut ();

   if (cd.lut == NULL)
     return;

   (void) arraymap_str_func_str (&func_str_delete_chars, (void *)&cd);

   if (free_lut)
     SLwchar_free_lut (cd.lut);
}

static char *func_strtrans (char *s, void *cd)
{
   SLuchar_Type *u;

   if (s == NULL)
     return NULL;

   u = SLuchar_apply_char_map ((SLwchar_Map_Type *)cd, (SLuchar_Type *)s);
   s = SLang_create_slstring ((char *) u);
   SLfree ((char *)u);
   return s;
}

static void strtrans_vintrin (char *to)
{
   SLwchar_Map_Type *map;
   char *from;

   if (*to == 0)
     {
	str_delete_chars_vintrin ();
	return;
     }

   if (-1 == SLang_pop_slstring (&from))
     return;

   if (NULL == (map = SLwchar_allocate_char_map ((SLuchar_Type *)from, (SLuchar_Type *)to)))
     return;

   _pSLang_free_slstring (from);

   (void) arraymap_str_func_str (&func_strtrans, (void *)map);
   SLwchar_free_char_map (map);
}

typedef struct
{
   SLwchar_Lut_Type *lut;
   SLuchar_Type pref_char_buf[SLUTF8_MAX_MBLEN+1];
   unsigned int pref_len;
}
Strcompress_CD_Type;

static char *func_strcompress (char *str, void *cd) /*{{{*/
{
   char *c;
   Strcompress_CD_Type *info;
   SLuchar_Type *s, *beg, *end;
   unsigned int len, pref_len;
   SLwchar_Lut_Type *lut;
   int ignore_combining = 0;

   if (str == NULL)
     return str;

   info = (Strcompress_CD_Type *)cd;
   pref_len = info->pref_len;
   lut = info->lut;

   beg = (SLuchar_Type *) str;
   (void) do_trim (&beg, 1, &end, 1, lut, 0);

   /* Determine the effective length */
   len = 0;
   s = (unsigned char *) beg;
   while (1)
     {
	SLuchar_Type *s1;

	s1 = SLwchar_skip_range (lut, s, end, ignore_combining, 1);
	len += (s1 - s);
	s = s1;

	if (s == end)
	  break;

	len += pref_len;

	s = SLwchar_skip_range (lut, s, end, ignore_combining, 0);
     }

   c = _pSLallocate_slstring (len);
   if (c == NULL)
     return NULL;

   s = (unsigned char *) c;

   while (1)
     {
	SLuchar_Type *beg1;
	unsigned int dlen;

	beg1 = SLwchar_skip_range (lut, beg, end, ignore_combining, 1);
	dlen = (unsigned int) (beg1 - beg);

	memcpy ((char *)s, beg, dlen);
	beg = beg1;
	s += dlen;

	if (beg == end)
	  break;

	memcpy (s, info->pref_char_buf, pref_len);
	s += pref_len;

	beg = SLwchar_skip_range (lut, beg, end, ignore_combining, 0);
     }
   *s = 0;

   return _pSLcreate_via_alloced_slstring (c, len);
}

static void strcompress_vintrin (char *white) /*{{{*/
{
   char *white_max;
   SLuchar_Type *s;
   SLwchar_Type pref_char;
   Strcompress_CD_Type cd;

   /* The first character of white is the preferred whitespace character */
   white_max = white + strlen (white);
   if (NULL == (s = _pSLinterp_decode_wchar ((SLuchar_Type *)white, (SLuchar_Type *)white_max,
					     &pref_char)))
     return;

   /* This cannot overflow since _pSLinterp_decode_wchar will not return an
    * offset of more than SLUTF8_MAX_BLEN bytes.
    */
   cd.pref_len = (unsigned int)(s - (SLuchar_Type*)white);
   memcpy ((char *)cd.pref_char_buf, white, cd.pref_len);
   cd.pref_char_buf[cd.pref_len] = 0;

   if (NULL == (cd.lut = SLwchar_strtolut ((SLuchar_Type *)white, 1, 0)))
     return;

   (void) arraymap_str_func_str (&func_strcompress, (void *)&cd);

   SLwchar_free_lut (cd.lut);
}

/*}}}*/

static char *SLdo_sprintf (char *fmt) /*{{{*/
{
   register char *p = fmt, ch;
   char *out = NULL, *outp = NULL;
   char dfmt[1024];	       /* used to hold part of format */
   char *f, *f1;
   char *str;
   unsigned int want_width, width, precis;
   int int_var, use_string;
   long long_var;
#ifdef HAVE_LONG_LONG
   long long llong_var;
#endif
   unsigned int len = 0, malloc_len = 0, dlen;
   int do_free;
   unsigned int guess_size;
#if SLANG_HAS_FLOAT
   int use_double;
   double x;
#endif
   unsigned char uch;
   int use_long = 0, use_alt_format = 0;

   while (1)
     {
	while ((ch = *p) != 0)
	  {
	     if (ch == '%')
	       break;
	     p++;
	  }

	/* p points at '%' or 0 */

	dlen = (unsigned int) (p - fmt);

	if (len + dlen >= malloc_len)
	  {
	     malloc_len = len + dlen;
	     if (out == NULL) outp = SLmalloc(malloc_len + 1);
	     else outp = SLrealloc(out, malloc_len + 1);
	     if (NULL == outp)
	       return out;
	     out = outp;
	     outp = out + len;
	  }

	strncpy(outp, fmt, dlen);
	len += dlen;
	outp = out + len;
	*outp = 0;
	if (ch == 0) break;

	/* bump it beyond '%' */
	++p;
	fmt = p;

	f = dfmt;
	*f++ = ch;
	/* handle flag char */
	ch = *p++;

	/* Make sure cases such as "% #g" can be handled. */
	if ((ch == '-') || (ch == '+') || (ch == ' ') || (ch == '#'))
	  {
	     if (ch == '#')
	       use_alt_format = 1;
	     *f++ = ch;
	     ch = *p++;
	     if ((ch == '-') || (ch == '+') || (ch == ' ') || (ch == '#'))
	       {
		  if (ch == '#')
		    use_alt_format = 1;
		  *f++ = ch;
		  ch = *p++;
	       }
	  }

	/* width */
	/* I have got to parse it myself so that I can see how big it needs
	 * to be.
	 */
	want_width = width = 0;
	if (ch == '*')
	  {
	     if (SLang_pop_uinteger(&width)) return (out);
	     want_width = 1;
	     ch = *p++;
	  }
	else
	  {
	     if (ch == '0')
	       {
		  *f++ = '0';
		  ch = *p++;
	       }

	     while ((ch <= '9') && (ch >= '0'))
	       {
		  width = width * 10 + (ch - '0');
		  ch = *p++;
		  want_width = 1;
	       }
	  }

	if (want_width)
	  {
	     sprintf(f, "%d", width);
	     f += strlen (f);
	  }
	precis = 0;
	/* precision -- also indicates max number of chars from string */
	if (ch == '.')
	  {
	     *f++ = ch;
	     ch = *p++;
	     want_width = 0;
	     if (ch == '*')
	       {
		  if (SLang_pop_uinteger(&precis)) return (out);
		  ch = *p++;
		  want_width = 1;
	       }
	     else while ((ch <= '9') && (ch >= '0'))
	       {
		  precis = precis * 10 + (ch - '0');
		  ch = *p++;
		  want_width = 1;
	       }
	     if (want_width)
	       {
		  sprintf(f, "%d", precis);
		  f += strlen (f);
	       }
	     else precis = 0;
	  }

	long_var = 0;
	int_var = 0;
#ifdef HAVE_LONG_LONG
	llong_var = 0;
#endif
	str = NULL;
	guess_size = 32;
#if SLANG_HAS_FLOAT
	use_double = 0;
#endif
	use_long = 0;
	use_string = 0;
	do_free = 0;

	if (ch == 'l')
	  {
	     use_long = 1;
	     ch = *p++;
	     if (ch == 'l')
	       {
		  use_long = 2;	       /* long long */
		  ch = *p++;
	       }
	  }
	else if (ch == 'h') ch = *p++; /* not supported */

	/* Now the actual format specifier */
	switch (ch)
	  {
	   case 'B':
	     if (-1 == _pSLformat_as_binary (precis, use_alt_format))
	       return out;
	     /* Remove the precision value from the format string */
	     f1 = f-1;
	     while (f1 > dfmt)
	       {
		  if (*f1 == '.')
		    {
		       *f1 = 0;
		       f = f1;
		       break;
		    }
		  f1--;
	       }

	     /* drop */
	   case 'S':
	     if (ch == 'S')
	       _pSLstring_intrinsic ();
	     ch = 's';
	     /* drop */
	   case 's':
	     if (-1 == SLang_pop_slstring(&str))
	       return (out);
	     do_free = 1;
	     guess_size = strlen(str);
	     use_string = 1;
	     break;

	   case '%':
	     guess_size = 1;
	     do_free = 0;
	     use_string = 1;
	     str = (char *) "%";
	     break;

	   case 'c':
#if 0
	     if (use_long)
#endif
	       {
		  SLwchar_Type wc;
		  SLuchar_Type utf8_buf[SLUTF8_MAX_MBLEN+1];

		  if (-1 == SLang_pop_wchar (&wc))
		    return out;
		  if ((_pSLinterp_UTF8_Mode == 0) && (wc <= 0xFF))
		    {
		       utf8_buf[0] = (unsigned char)wc;
		       utf8_buf[1] = 0;
		    }
		  else if (NULL == SLutf8_encode_null_terminate (wc, utf8_buf))
		    return out;
		  ch = 's';
		  str = (char *)utf8_buf;
		  use_string = 1;
	       }
	     break;

	   case 'b':
	     use_long = 0;
	     guess_size = 1;
	     if (-1 == SLang_pop_uchar (&uch))
	       return out;
	     int_var = (int) uch;
	     ch = 'c';
	     break;

	   case 'd':
	   case 'i':
	   case 'o':
	   case 'u':
	   case 'X':
	   case 'x':
	     if (use_long)
	       {
#ifdef HAVE_LONG_LONG
		  if (use_long > 1)
		    {
		       if (-1 == SLang_pop_long_long (&llong_var))
			 return out;
		       *f++ = 'l';
		    }
		  else
#endif
		    if (-1 == SLang_pop_long (&long_var))
		      return out;
		  *f++ = 'l';
	       }
	     else if (-1 == SLang_pop_int (&int_var))
	       return out;
	     break;

	   case 'f':
	   case 'e':
	   case 'g':
	   case 'E':
	   case 'G':
#if SLANG_HAS_FLOAT
	     if (SLang_pop_double(&x)) return (out);
	     use_double = 1;
	     guess_size = 256;
	     if (fabs(x) > 1e38)
	       {
		  if (0 == _pSLmath_isinf (x))
		    {
		       double expon = log10 (fabs(x));
		       if (expon > (double) 0xFFFF)
			 ch = 'E';
		       else
			 guess_size += (unsigned int) expon;
		    }
		  else ch = 'E';
	       }
	     use_long = 0;
	     break;
#endif
	   case 'p':
	     guess_size = 32;
	     /* Pointer type?? Why?? */
	     if (-1 == SLdo_pop ())
	       return out;
	     str = (char *) _pSLang_get_run_stack_pointer ();
	     use_string = 1;
	     use_long = 0;
	     break;

	   default:
	     _pSLang_verror (SL_INVALID_PARM, "Invalid printf format");
	     return(out);
	  }
	*f++ = ch; *f = 0;

	width = width + precis;
	if (width > guess_size) guess_size = width;

	if (len + guess_size > malloc_len)
	  {
	     outp = (char *) SLrealloc(out, len + guess_size + 1);
	     if (outp == NULL)
	       {
		  SLang_set_error (SL_MALLOC_ERROR);
		  return (out);
	       }
	     out = outp;
	     outp = out + len;
	     malloc_len = len + guess_size;
	  }

	if (use_string)
	  {
	     sprintf(outp, dfmt, str);
	     if (do_free) _pSLang_free_slstring (str);
	  }
#if SLANG_HAS_FLOAT
	else if (use_double) sprintf(outp, dfmt, x);
#endif
	else if (use_long)
	  {
#ifdef HAVE_LONG_LONG
	     if (use_long > 1)
	       sprintf (outp, dfmt, llong_var);
	     else
#endif
	       sprintf (outp, dfmt, long_var);
	  }
	else sprintf(outp, dfmt, int_var);

	len += strlen(outp);
	outp = out + len;
	fmt = p;
     }

   if (out != NULL)
     {
	outp = SLrealloc (out, (unsigned int) (outp - out) + 1);
	if (outp != NULL) out = outp;
     }

   return (out);
}

/*}}}*/

int _pSLstrops_do_sprintf_n (int n) /*{{{*/
{
   char *p;
   char *fmt;
   SLang_Object_Type *ptr;
   int ofs;

   if (-1 == (ofs = SLreverse_stack (n + 1)))
     return -1;

   ptr = _pSLang_get_run_stack_base () + ofs;

   if (SLang_pop_slstring(&fmt))
     return -1;

   p = SLdo_sprintf (fmt);
   _pSLang_free_slstring (fmt);

   SLdo_pop_n (_pSLang_get_run_stack_pointer () - ptr);

   if (_pSLang_Error)
     {
	SLfree (p);
	return -1;
     }

   return SLang_push_malloced_string (p);
}

/*}}}*/

static void sprintf_n_cmd (int *n)
{
   _pSLstrops_do_sprintf_n (*n);
}

static void sprintf_cmd (void)
{
   _pSLstrops_do_sprintf_n (SLang_Num_Function_Args - 1);    /* do not include format */
}

/* converts string s to a form that can be used in an eval */
/* UTF-8 ok */
static void make_printable_string(unsigned char *s) /*{{{*/
{
   unsigned int len;
   unsigned char *s1 = s, ch, *ss1;
   unsigned char *ss;

   /* compute length */
   len = 3;
   while ((ch = *s1++) != 0)
     {
	if ((ch == '\n') || (ch == '\\') || (ch == '"'))
	  {
	     len += 2;
	     continue;
	  }
	ch &= 0x7F;
	if ((ch == 127) || (ch < 32))
	  {
	     len += 4;
	     continue;
	  }
	len++;
     }

   if (NULL == (ss = (unsigned char *) SLmalloc(len)))
     return;

   s1 = s;
   ss1 = ss;
   *ss1++ = '"';
   while ((ch = *s1++) != 0)
     {
	if (ch == '\n')
	  {
	     *ss1++ = '\\';
	     *ss1++ = 'n';
	     continue;
	  }
	if ((ch == '\\') || (ch == '"'))
	  {
	     *ss1++ = '\\';
	     *ss1++ = ch;
	     continue;
	  }

	if ((ch == 127) || ((ch & 0x7F) < 32))
	  {
	     sprintf ((char *)ss1, "\\x%02X", ch);
	     ss1 += 4;
	     continue;
	  }
	*ss1++ = ch;
     }
   *ss1++ = '"';
   *ss1 = 0;
   (void) SLang_push_malloced_string ((char *)ss);
}

/*}}}*/

static void extract_element_cmd (char *list, int *nth_ptr, SLwchar_Type *delim_ptr)
{
   SLwchar_Type delim = *delim_ptr;
   SLuchar_Type delim_utf8[SLUTF8_MAX_MBLEN+1];
   unsigned int delim_len;
   char *list1;
   int n;

   n = *nth_ptr;
   if (n < 0)
     {
	SLang_push_null ();
	return;
     }

   if (NULL == _pSLinterp_encode_wchar (delim, delim_utf8, &delim_len))
     return;

   while (n > 0)
     {
	list = strstr (list, (char *)delim_utf8);
	if (list == NULL)
	  {
	     (void) SLang_push_null();
	     return;
	  }
	list += delim_len;
	n--;
     }

   list1 = strstr (list, (char *)delim_utf8);
   if (list1 == NULL)
     {
	SLang_push_string (list);
	return;
     }

   (void) _pSLang_push_nstring (list, (unsigned int)(list1 - list));
}

static int is_list_element_cmd (char *list, char *elem, SLwchar_Type *delim_ptr)
{
   SLuchar_Type delim_utf8[SLUTF8_MAX_MBLEN+1];
   unsigned int delim_len;
   unsigned int elem_len;
   int n;

   if (NULL == _pSLinterp_encode_wchar (*delim_ptr, delim_utf8, &delim_len))
     return 0;

   if (delim_len == 0)
     return (list == elem);

   n = 0;
   elem_len = strlen (elem);

   while (1)
     {
	unsigned int len;
	char *list_end = strstr (list, (char *)delim_utf8);

	if (list_end == NULL)
	  {
	     if (0 == strcmp (list, elem))
	       return n + 1;
	     return 0;
	  }
	len = list_end - list;
	if ((len == elem_len)
	    && (0 == strncmp (list, elem, len)))
	  return n + 1;

	list += len + delim_len;
	n++;
     }
}

/*}}}*/

/* Regular expression routines for strings */
static SLRegexp_Type *Regexp;
static unsigned int Regexp_Match_Byte_Offset;

static int string_match_internal (char *str, char *pat, int n) /*{{{*/
{
   char *match;
   unsigned int len;
   unsigned int byte_offset;

   if (Regexp != NULL)
     {
	SLregexp_free (Regexp);
	Regexp = NULL;
     }

   byte_offset = (unsigned int) (n - 1);
   len = strlen(str);

   if (byte_offset > len)
     return 0;

   if (NULL == (Regexp = SLregexp_compile (pat, 0)))
     return -1;
   Regexp_Match_Byte_Offset = byte_offset;

   if (NULL == (match = SLregexp_match (Regexp, str+byte_offset, len-byte_offset)))
     return 0;

   return 1 + (int) (match - str);
}

/*}}}*/


static int pop_string_match_args (int nargs, char **strp, char **patp, int *np)
{
   *strp = *patp = NULL;

   if (nargs == 2)
     *np = 1;
   else if (-1 == SLang_pop_int (np))
     return -1;

   if (-1 == SLang_pop_slstring (patp))
     return -1;

   if (0 == SLang_pop_slstring (strp))
     return 0;

   SLang_free_slstring (*patp);
   *patp = NULL;
   return -1;
}

static int string_match_cmd (void)
{
   char *str, *pat;
   int n, status;

   if (-1 == pop_string_match_args (SLang_Num_Function_Args, &str, &pat, &n))
     return -1;

   status = string_match_internal (str, pat, n);
   SLang_free_slstring (str);
   SLang_free_slstring (pat);
   return status;
}

static int string_match_nth_cmd (int *nptr) /*{{{*/
{
   unsigned int ofs, len;

   if (Regexp == NULL)
     {
	_pSLang_verror (SL_RunTime_Error, "A successful call to string_match was not made");
	return -1;
     }

   if (-1 == SLregexp_nth_match (Regexp, (unsigned int) *nptr, &ofs, &len))
     {
	_pSLang_verror (0, "SLregexp_nth_match failed");
	return -1;
     }

   ofs += Regexp_Match_Byte_Offset;

   /* zero based return value */
   SLang_push_integer((int) ofs);
   return (int) len;
}

/*}}}*/

static int string_matches_internal (char *str, char *pat, int n)
{
   int status;
   unsigned int i;
   unsigned int lens[10];
   unsigned int offsets[10];
   char **strs;
   SLindex_Type num;
   SLang_Array_Type *at;

   status = string_match_internal (str, pat, n);
   if (status <= 0)
     {
	SLang_push_null ();
	return -1;
     }

   for (i = 0; i < 10; i++)
     {
	if (-1 == SLregexp_nth_match (Regexp, i, offsets+i, lens+i))
	  break;
	offsets[i] += Regexp_Match_Byte_Offset;
     }

   num = (SLindex_Type)i;

   if (NULL == (at = SLang_create_array (SLANG_STRING_TYPE, 0, NULL, &num, 1)))
     return -1;

   strs = (char **) at->data;
   for (i = 0; i < (unsigned int) num; i++)
     {
	if (NULL == (strs[i] = SLang_create_nslstring (str+offsets[i], lens[i])))
	  {
	     SLang_free_array (at);
	     return -1;
	  }
     }

   return SLang_push_array (at, 1);
}

static void string_matches_cmd (void)
{
   char *str, *pat;
   int n;

   if (-1 == pop_string_match_args (SLang_Num_Function_Args, &str, &pat, &n))
     return;

   (void) string_matches_internal (str, pat, n);
   SLang_free_slstring (str);
   SLang_free_slstring (pat);
}

/* UTF-8 ok */
static char *create_delimited_string (char **list, unsigned int n,
				      char *delim)
{
   unsigned int len, dlen;
   unsigned int i;
   unsigned int num;
   char *str, *s;

   len = 1;			       /* allow room for \0 char */
   num = 0;
   for (i = 0; i < n; i++)
     {
	if (list[i] == NULL) continue;
	len += strlen (list[i]);
	num++;
     }

   dlen = strlen (delim);
   if (num > 1)
     len += (num - 1) * dlen;

   if (NULL == (str = SLmalloc (len)))
     return NULL;

   *str = 0;
   s = str;
   i = 0;

   while (num > 1)
     {
	unsigned int len2;

	while (list[i] == NULL)
	  i++;

	len2 = strlen (list[i]);
	memcpy (s, list[i], len2);
	s += len2;
	strcpy (s, delim);	       /* \0 terminates s */
	s += dlen;
	i++;
	num--;
     }

   if (num)
     {
	while (list[i] == NULL)
	  i++;

	strcpy (s, list[i]);
     }

   return str;
}

/* UTF-8 ok */
static void create_delimited_string_cmd (int *nptr)
{
   unsigned int n, i;
   char **strings;
   char *str;

   str = NULL;

   n = 1 + (unsigned int) *nptr;       /* n includes delimiter */

   if (NULL == (strings = (char **)_SLcalloc (n, sizeof (char *))))
     {
	SLdo_pop_n (n);
	return;
     }
   memset((char *)strings, 0, n * sizeof (char *));

   i = n;
   while (i != 0)
     {
	i--;
	if (-1 == SLang_pop_slstring (strings + i))
	  goto return_error;
     }

   str = create_delimited_string (strings + 1, (n - 1), strings[0]);
   /* drop */
   return_error:
   for (i = 0; i < n; i++) _pSLang_free_slstring (strings[i]);
   SLfree ((char *)strings);

   (void) SLang_push_malloced_string (str);   /* NULL Ok */
}

/* UTF-8 ok */
static void strjoin_cmd (void)
{
   SLang_Array_Type *at;
   char *str;
   char *delim;
   int free_delim;

   if (SLang_Num_Function_Args == 1)
     {
	free_delim = 0;
	delim = "";
     }
   else
     {
	if (-1 == SLang_pop_slstring (&delim))
	  return;
	free_delim = 1;
     }

   if (-1 == SLang_pop_array_of_type (&at, SLANG_STRING_TYPE))
     return;

   str = create_delimited_string ((char **)at->data, at->num_elements, delim);
   SLang_free_array (at);
   if (free_delim)
     SLang_free_slstring (delim);

   (void) SLang_push_malloced_string (str);   /* NULL Ok */
}

static unsigned int count_char_occurrences (char *str, SLwchar_Type *wchp)
{
   SLwchar_Type wch = *wchp;
   SLuchar_Type wch_utf8[SLUTF8_MAX_MBLEN+1];
   unsigned int wch_utf8_len;
   unsigned int n = 0;
   int is_byte;

   if (wch < 0x80)
     is_byte = 1;
   else
     {
	if (_pSLinterp_UTF8_Mode == 0)
	  {
	     if (wch >= 256)
	       {
		  _pSLang_verror (SL_InvalidParm_Error, "Character is invalid in non-UTF-8 mode");
		  return 0;
	       }
	     is_byte = 1;
	  }
	else
	  is_byte = 0;
     }

   if (is_byte)
     {
	unsigned char byte = (unsigned char) wch;
	while (*str != 0)
	  {
	     if (*str == byte) n++;
	     str++;
	  }
	return n;
     }

   if (NULL == _pSLinterp_encode_wchar (wch, wch_utf8, &wch_utf8_len))
     return 0;

   while (NULL != (str = strstr (str, (char *)wch_utf8)))
     {
	n++;
	str += wch_utf8_len;
     }

   return n;
}

static void glob_to_regexp (char *glob)
{
   unsigned int len;
   char *pat, *p;
   char ch;

   len = _pSLstring_bytelen (glob);
   pat = SLmalloc (2*len + 8);
   if (pat == NULL)
     return;

   p = pat;
   *p++ = '^';

   /* If the first character of a file is '.', it must be explicitly matched. */
   /*
    * This will not work until | is supported in REs.  Then if the glob
    * pattern is *X, the RE will be ^([^.].*X | ^X)$
    *
   if ((*glob == '?') || (*glob == '*'))
     {
	*p++ = '[';
	*p++ = '^';
	*p++ = '.';
	*p++ = ']';
	if (*glob == '?')
	  glob++;
     }
    */
   while (0 != (ch = *glob++))
     {
	if ((ch == '.') || (ch == '$') || (ch == '+') || (ch == '\\'))
	  {
	     *p++ = '\\';
	     *p++ = ch;
	     continue;
	  }
	if (ch == '*')
	  {
	     *p++ = '.';
	     *p++ = '*';
	     continue;
	  }
	if (ch == '?')
	  {
	     *p++ = '.';
	     continue;
	  }
	if (ch != '[')
	  {
	     *p++ = ch;
	     continue;
	  }

	/* Otherwise ch = '[' */
	if (*glob != 0)
	  {
	     char *g = glob;
	     int is_complement = 0;

	     ch = *g;
	     if ((ch == '^') || (ch == '!'))
	       {
		  is_complement = 1;
		  g++;
		  ch = *g;
	       }
	     if (ch == ']')
	       g++;

	     while (((ch = *g) != 0) && (ch != ']'))
	       g++;

	     if (ch == ']')
	       {
		  *p++ = '[';
		  if (is_complement)
		    {
		       *p++ = '^';
		       glob++;
		    }
		  while (glob <= g)
		    *p++ = *glob++;

		  continue;
	       }
	  }

	/* failed to find the matching ']'.  So quote it */
	*p++ = '\\';
	*p++ = '[';
     }
   *p++ = '$';
   *p = 0;

   (void) SLang_push_malloced_string (pat);   /* frees it too */
}

static void define_case_intrin (int *a, int *b)
{
   SLang_define_case (a, b);
}

static char *convert_offset_to_ptr (char *str, unsigned int len, int ofs)
{
   if (ofs < 0)
     {
	if ((unsigned int) -ofs > len)
	  {
	     SLang_verror (SL_InvalidParm_Error, "offset parameter is too large for input string");
	     return NULL;
	  }
	return (str + len) + ofs;
     }

   if ((unsigned int) ofs > len)
     {
	SLang_verror (SL_InvalidParm_Error, "offset parameter is too large for input string");
	return NULL;
     }
   return str + ofs;
}

static void skip_bytes_intrin (void)
{
   int nmax = -1, n0 = 0;
   int has_nmax = 0, utf8_mode;
   unsigned int len;
   char *str, *chars;
   SLuchar_Type *strmin, *strmax;
   int invert;
   int nargs = SLang_Num_Function_Args;
   SLwchar_Lut_Type *lut;
   int ignore_combining = 0;

   switch (nargs)
     {
      case 4:
	if (-1 == SLang_pop_int (&nmax))
	  return;
	has_nmax = 1;
	/* drop */
      case 3:
	if (-1 == SLang_pop_int (&n0))
	  return;
	/* drop */
      default:
	if (-1 == SLang_pop_slstring (&chars))
	  return;
	if (-1 == SLang_pop_slstring (&str))
	  {
	     SLang_free_slstring (chars);
	     return;
	  }
     }
   len = _pSLstring_bytelen (str);
   if (has_nmax)
     {
	strmax = (SLuchar_Type *)convert_offset_to_ptr (str, len, nmax);
	if (strmax == NULL)
	  goto free_and_return;
     }
   else strmax = (SLuchar_Type *)str + len;

   strmin = (SLuchar_Type *)convert_offset_to_ptr (str, len, n0);
   if (strmin == NULL)
     goto free_and_return;

   /* FIXME!! There should be a way of specifying this when making the lut */
   utf8_mode = _pSLinterp_UTF8_Mode; _pSLinterp_UTF8_Mode = 0;
   invert = (chars[0] == '^');
   if (invert)
     lut = SLwchar_strtolut ((SLuchar_Type*)chars+1, 1, 1);
   else
     lut = SLwchar_strtolut ((SLuchar_Type*)chars, 1, 1);
   _pSLinterp_UTF8_Mode = utf8_mode;
   if (lut == NULL)
     goto free_and_return;

   strmax = SLwchar_skip_range (lut, strmin, strmax, ignore_combining, invert);
   SLwchar_free_lut (lut);
   if (strmax == NULL)
     goto free_and_return;

   (void) SLang_push_integer ((int)((char *)strmax - str));
   /* drop */

free_and_return:
   SLang_free_slstring (str);
   SLang_free_slstring (chars);
}

static SLang_Intrin_Fun_Type Strops_Table [] = /*{{{*/
{
   MAKE_INTRINSIC_I("create_delimited_string",  create_delimited_string_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("strcmp",  strcmp_vintrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("strncmp",  strncmp_vintrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("strncharcmp",  strncharcmp_vintrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("strnbytecmp",  strnbytecmp_vintrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("strcat",  strcat_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("strlen",  strlen_vintrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("strcharlen",  strcharlen_vintrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("strbytelen",  strbytelen_vintrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_3("strchop", strchop_cmd, SLANG_VOID_TYPE, SLANG_STRING_TYPE, SLANG_WCHAR_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_3("strchopr", strchopr_cmd, SLANG_VOID_TYPE, SLANG_STRING_TYPE, SLANG_WCHAR_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_0("strreplace", strreplace_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SSS("str_replace", str_replace_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_SII("substr",  substr_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SII("substrbytes",  subbytes_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("is_substr",  issubstr_vintrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_2("strsub",  strsub_cmd, SLANG_VOID_TYPE, SLANG_INT_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_2("strbytesub",  strbytesub_cmd, SLANG_VOID_TYPE, SLANG_INT_TYPE, SLANG_UCHAR_TYPE),
   MAKE_INTRINSIC_3("extract_element", extract_element_cmd, SLANG_VOID_TYPE, SLANG_STRING_TYPE, SLANG_INT_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_3("is_list_element", is_list_element_cmd, SLANG_INT_TYPE, SLANG_STRING_TYPE, SLANG_STRING_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_0("string_match", string_match_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("string_matches", string_matches_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_I("string_match_nth", string_match_nth_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("strlow", strlow_vintrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_1("tolower", tolower_cmd, SLANG_INT_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_1("toupper", toupper_cmd, SLANG_INT_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_0("strup", strup_vintrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("strtrim", strtrim_vintrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("strtrim_end", strtrim_end_vintrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("strtrim_beg", strtrim_beg_vintrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("strcompress", strcompress_vintrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_I("Sprintf", sprintf_n_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("sprintf", sprintf_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("sscanf", _pSLang_sscanf, SLANG_INT_TYPE),
   MAKE_INTRINSIC_S("make_printable_string", make_printable_string, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_3("str_quote_string", str_quote_string_cmd, SLANG_VOID_TYPE, SLANG_STRING_TYPE, SLANG_STRING_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_SSS("str_uncomment_string", str_uncomment_string_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_II("define_case", define_case_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("strtok", strtok_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("strjoin", strjoin_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("strtrans", strtrans_vintrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("str_delete_chars", str_delete_chars_vintrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("glob_to_regexp", glob_to_regexp, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_2("count_char_occurrences", count_char_occurrences, SLANG_UINT_TYPE, SLANG_STRING_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_2("count_char_occurances", count_char_occurrences, SLANG_UINT_TYPE, SLANG_STRING_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_0("strskipbytes", skip_bytes_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("islower", islower_intrin, SLANG_CHAR_TYPE),
   MAKE_INTRINSIC_0("isupper", isupper_intrin, SLANG_CHAR_TYPE),
   MAKE_INTRINSIC_0("isalpha", isalpha_intrin, SLANG_CHAR_TYPE),
   MAKE_INTRINSIC_0("isxdigit", isxdigit_intrin, SLANG_CHAR_TYPE),
   MAKE_INTRINSIC_0("isspace", isspace_intrin, SLANG_CHAR_TYPE),
   MAKE_INTRINSIC_0("isblank", isblank_intrin, SLANG_CHAR_TYPE),
   MAKE_INTRINSIC_0("iscntrl", iscntrl_intrin, SLANG_CHAR_TYPE),
   MAKE_INTRINSIC_0("isprint", isprint_intrin, SLANG_CHAR_TYPE),
   MAKE_INTRINSIC_0("isdigit", isdigit_intrin, SLANG_CHAR_TYPE),
   MAKE_INTRINSIC_0("isgraph", isgraph_intrin, SLANG_CHAR_TYPE),
   MAKE_INTRINSIC_0("isalnum", isalnum_intrin, SLANG_CHAR_TYPE),
   MAKE_INTRINSIC_0("ispunct", ispunct_intrin, SLANG_CHAR_TYPE),
   MAKE_INTRINSIC_0("isascii", isascii_intrin, SLANG_CHAR_TYPE),
   MAKE_INTRINSIC_0("strskipchar", strskipchar_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("strbskipchar", strbskipchar_intrin, SLANG_VOID_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};

/*}}}*/

int _pSLang_init_slstrops (void)
{
   return SLadd_intrin_fun_table (Strops_Table, NULL);
}
