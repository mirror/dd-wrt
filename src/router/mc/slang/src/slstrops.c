/* -*- mode: C; mode: fold; -*- */
/* string manipulation functions for S-Lang. */
/*
Copyright (C) 2004-2009 John E. Davis

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


/* Here, if white is NULL and lut is NULL, a standard whitespace lut will be created.
 */
static unsigned int do_trim (SLuchar_Type **beg, int do_beg, 
			     SLuchar_Type **end, int do_end, 
			     SLuchar_Type *white, 
			     SLwchar_Lut_Type *lut) /*{{{*/
{
   unsigned int len;
   SLuchar_Type *a, *b;
   int invert;
   int ignore_combining = 0;

   invert = 0;

   if (lut == NULL)
     {
	if (white == NULL)
	  lut = make_whitespace_lut ();
	else
	  {
	     if (*white == '^')
	       {
		  white++;
		  invert = 1;
	       }
	     lut = SLwchar_strtolut (white, 1, 1);
	  }

	if (lut == NULL)
	  return 0;
     }
   else white = NULL;

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

   if (white != NULL)
     SLwchar_free_lut (lut);

   return len;
}

/*}}}*/

/*}}}*/

static int pop_3_strings (char **a, char **b, char **c)
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

static void free_3_strings (char *a, char *b, char *c)
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
   else if (NULL == (ptrs = (char **)SLmalloc (nargs * sizeof (char *))))
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


static void strtrim_cmd_internal (char *str, int do_beg, int do_end)
{
   SLuchar_Type *beg, *end, *white;
   int free_str;
   unsigned int len;

   /* Go through SLpop_string to get a private copy since it will be
    * modified.
    */
   
   free_str = 0;
   if (SLang_Num_Function_Args == 2)
     {
	white = (SLuchar_Type *)str;
	if (-1 == SLang_pop_slstring (&str))
	  return;
	free_str = 1;
     }
   else white = NULL;

   beg = (SLuchar_Type *)str;
   len = do_trim (&beg, do_beg, &end, do_end, white, NULL);
   
   (void) _pSLang_push_nstring ((char *)beg, len);
   if (free_str)
     _pSLang_free_slstring (str);
}

   
static void strtrim_cmd (char *str)
{
   strtrim_cmd_internal (str, 1, 1);
}

static void strtrim_beg_cmd (char *str)
{
   strtrim_cmd_internal (str, 1, 0);
}

static void strtrim_end_cmd (char *str)
{
   strtrim_cmd_internal (str, 0, 1);
}


static void strcompress_cmd (char *str, char *white) /*{{{*/
{
   char *c, *white_max;
   SLuchar_Type *s, *beg, *end;
   unsigned int len, pref_len;
   SLwchar_Type pref_char;
   SLuchar_Type pref_char_buf[SLUTF8_MAX_MBLEN+1];
   SLwchar_Lut_Type *lut;
   int ignore_combining = 0;

   /* The first character of white is the preferred whitespace character */
   white_max = white + strlen (white);
   if (NULL == (s = _pSLinterp_decode_wchar ((SLuchar_Type *)white, (SLuchar_Type *)white_max,
					    &pref_char)))
     return;
   
   /* This cannot overflow since _pSLinterp_decode_wchar will not return an
    * offset of more than SLUTF8_MAX_BLEN bytes.
    */
   pref_len = (unsigned int)(s - (SLuchar_Type*)white);
   memcpy ((char *)pref_char_buf, white, pref_len);
   pref_char_buf[pref_len] = 0;

   if (NULL == (lut = SLwchar_strtolut ((SLuchar_Type *)white, 1, 0)))
     return;

   beg = (SLuchar_Type *) str;
   (void) do_trim (&beg, 1, &end, 1, NULL, lut);

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
     {
	SLwchar_free_lut (lut);
	SLfree (str);
	return;
     }
   
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
	
	memcpy (s, pref_char_buf, pref_len);
	s += pref_len;

	beg = SLwchar_skip_range (lut, beg, end, ignore_combining, 0);
     }
   *s = 0;
   
   SLwchar_free_lut (lut);
   
   (void) _pSLpush_alloced_slstring (c, len);
}

/*}}}*/

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

static int strreplace_cmd (int *np)
{   
   char *orig, *match, *rep;
   char *new_str;
   int max_num_replaces;
   int ret;

   max_num_replaces = *np;

   if (-1 == pop_3_strings (&orig, &match, &rep))
     return -1;

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
   
   if (ret == 0)
     {
	if (-1 == SLang_push_malloced_string (orig))
	  ret = -1;
	orig = NULL;
     }
   else if (ret > 0)
     {
	if (-1 == SLang_push_malloced_string (new_str))
	  ret = -1;
     }

   free_3_strings (orig, match, rep);
   return ret;
}

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

   if (NULL == (etable = (SLuchar_Type *) SLmalloc (blen * (SLUTF8_MAX_MBLEN+1))))
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

/* returns the character position of substring in a string or null */
static int issubstr_cmd (char *a, char *b) /*{{{*/
{
   char *c;
   unsigned int n;

   if (NULL == (c = strstr(a, b)))
     return 0;
   
   if (_pSLinterp_UTF8_Mode == 0)
     return 1 + (int) (c - a);
   
   n = (unsigned int) (c - a);
   (void) SLutf8_skip_chars ((SLuchar_Type *)a, (SLuchar_Type *)c, n, &n, 0);
   return (int) (n+1);
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

static void strup_cmd(void) /*{{{*/
{
   unsigned char c, *a;
   char *str;

   if (SLpop_string (&str))
     return;

   if (_pSLinterp_UTF8_Mode)
     {
	a = SLutf8_strup ((SLuchar_Type*)str, (SLuchar_Type*)str+strlen(str));
	SLfree (str);
	(void) _pSLang_push_slstring ((char *) a);   /* frees string */
	return;
     }
   
   a = (unsigned char *) str;
   while ((c = *a) != 0)
     {
	/* if ((*a >= 'a') && (*a <= 'z')) *a -= 32; */
	*a = UPPER_CASE(c);
	a++;
     }

   SLang_push_malloced_string (str);
}

/*}}}*/

static int isdigit_cmd (void) /*{{{*/
{
   SLwchar_Type wch;

   if (SLang_peek_at_stack() == SLANG_STRING_TYPE)
     {
	char *s;
	if (-1 == SLang_pop_slstring (&s))
	  return -1;
	
	if (_pSLinterp_UTF8_Mode)
	  {
	     if (NULL == SLutf8_decode ((unsigned char *)s, (unsigned char *)s+strlen(s), &wch, NULL))
	       wch = 0;
	  }
	else wch = s[0];

	_pSLang_free_slstring (s);
     }
   else if (-1 == SLang_pop_wchar (&wch))
     return -1;

   if (_pSLinterp_UTF8_Mode)
     return SLwchar_isdigit (wch);

   return isdigit((unsigned char)wch);
}

/*}}}*/
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

static void strlow_cmd (void) /*{{{*/
{
   unsigned char c, *a;
   char *str;

   if (SLpop_string(&str)) return;

   if (_pSLinterp_UTF8_Mode)
     {
	a = SLutf8_strlo ((SLuchar_Type *)str, (SLuchar_Type *)str+strlen(str));
	SLfree (str);
	(void) _pSLang_push_slstring ((char *) a);   /* frees string */
	return;
     }

   a = (unsigned char *) str;
   while ((c = *a) != 0)
     {
	/* if ((*a >= 'a') && (*a <= 'z')) *a -= 32; */
	*a = LOWER_CASE(c);
	a++;
     }

   SLang_push_malloced_string ((char *) str);
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

static int strcmp_cmd (char *a, char *b) /*{{{*/
{
   return strcmp(a, b);
}

/*}}}*/

static int do_strncmp_cmd (SLstr_Type *a, SLstr_Type *b, int n, int skip_combining) /*{{{*/
{
   char *p;
   unsigned int na, nb;
   unsigned int lena, lenb;
   int cmp;

   if (_pSLinterp_UTF8_Mode == 0)
     return strncmp(a, b, (unsigned int) n);
   
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

/*}}}*/

static int strncmp_cmd (SLstr_Type *a, SLstr_Type *b, int *n)
{
   return do_strncmp_cmd (a, b, *n, 1);
}

static int strnbytecmp_cmd (SLstr_Type *a, SLstr_Type *b, int *n)
{
   return (int) strncmp (a, b, *n);
}

static int strncharcmp_cmd (SLstr_Type *a, SLstr_Type *b, int *n)
{
   return do_strncmp_cmd (a, b, *n, 0);
}

static int do_strlen_combining (SLstr_Type *s, int ignore_combining) /*{{{*/
{
   if (_pSLinterp_UTF8_Mode == 0)
     return (int) _pSLstring_bytelen (s);
   
   return (int) SLutf8_strlen ((SLuchar_Type *)s, ignore_combining);
}
/*}}}*/


static int strlen_cmd (SLstr_Type *s) /*{{{*/
{
   return do_strlen_combining (s, 1);
}

static int strcharlen_cmd (SLstr_Type *s)
{
   return do_strlen_combining (s, 0);
}

static int strbytelen_cmd (SLstr_Type *s)
{
   return (int) _pSLstring_bytelen (s);
}

/*}}}*/


static char *SLdo_sprintf (char *fmt) /*{{{*/
{
   register char *p = fmt, ch;
   char *out = NULL, *outp = NULL;
   char dfmt[1024];	       /* used to hold part of format */
   char *f;
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
   int use_long = 0;

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
	     *f++ = ch;
	     ch = *p++;
	     if ((ch == '-') || (ch == '+') || (ch == ' ') || (ch == '#'))
	       {
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
	   case 'S':
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

static int string_match_cmd (char *str, char *pat, int *nptr) /*{{{*/
{
   char *match;
   unsigned int len;
   unsigned int byte_offset;
   
   if (Regexp != NULL)
     {
	SLregexp_free (Regexp);
	Regexp = NULL;
     }
   
   byte_offset = (unsigned int) (*nptr - 1);
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

static void string_matches_cmd (char *str, char *pat, int *nptr)
{
   int status;
   unsigned int i;
   unsigned int lens[10];
   unsigned int offsets[10];
   char **strs;
   SLindex_Type num;
   SLang_Array_Type *at;

   status = string_match_cmd (str, pat, nptr);
   if (status <= 0)
     {
	SLang_push_null ();
	return;
     }
   
   for (i = 0; i < 10; i++)
     {
	if (-1 == SLregexp_nth_match (Regexp, i, offsets+i, lens+i))
	  break;
	offsets[i] += Regexp_Match_Byte_Offset;
     }

   num = (SLindex_Type)i;

   if (NULL == (at = SLang_create_array (SLANG_STRING_TYPE, 0, NULL, &num, 1)))
     return;
   
   strs = (char **) at->data;
   for (i = 0; i < (unsigned int) num; i++)
     {
	if (NULL == (strs[i] = SLang_create_nslstring (str+offsets[i], lens[i])))
	  {
	     SLang_free_array (at);
	     return;
	  }
     }

   (void) SLang_push_array (at, 1);
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

   if (NULL == (strings = (char **)SLmalloc (n * sizeof (char *))))
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
static void strjoin_cmd (char *delim)
{
   SLang_Array_Type *at;
   char *str;

   if (-1 == SLang_pop_array_of_type (&at, SLANG_STRING_TYPE))
     return;
   
   str = create_delimited_string ((char **)at->data, at->num_elements, delim);
   SLang_free_array (at);
   (void) SLang_push_malloced_string (str);   /* NULL Ok */
}

static void str_delete_chars_cmd (SLuchar_Type *s, SLuchar_Type *d)
{
   SLwchar_Lut_Type *lut;
   SLuchar_Type *s1;
   SLuchar_Type *t, *tmax;
   int invert, ignore_combining = 0;

   /* Assume that the number of characters to be deleted is smaller then
    * the number to be deleted.  In this case, it is better to skip past call 
    * characters not in the set to be deleted.  Hence, we want to invert
    * the deletion set
    */
   invert = 1;
   if (*d == '^')
     {
	invert = 0;
	d++;
     }
   if (NULL == (lut = SLwchar_strtolut (d, 1, 1)))
     return;

   if (NULL == (s = (SLuchar_Type *)SLmake_string ((char *)s)))
     {
	SLwchar_free_lut (lut);
	return;
     }

   s1 = s;
   t = (SLuchar_Type *) s;
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

   SLwchar_free_lut (lut);
   (void) SLang_push_malloced_string ((char *)s);
}

static unsigned int count_char_occurances (char *str, SLwchar_Type *wchp)
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

/*
 * Supporting UTF-8 here will be tricky.  The non-UTF-8 version supports 
 * constructs such as strtrans(s, "A-Z", "a-z") and strtrans(s,"^0-9", " ")
 * where the latter replaces anything but 0-9 with a space.  The UTF-8
 * generalization of the former is strtrans(s, "\\u", "\\l"), where all
 * uppercase letters are replaced by their lowercase counterparts.
 */
static void strtrans_cmd (SLuchar_Type *s, SLuchar_Type *from, SLuchar_Type *to)
{
   SLwchar_Map_Type *map;

   if (*to == 0)
     {
	str_delete_chars_cmd (s, from);
	return;
     }
   
   if (NULL == (map = SLwchar_allocate_char_map (from, to)))
     return;

   s = SLuchar_apply_char_map (map, s);
   SLwchar_free_char_map (map);
   (void) SLang_push_malloced_string ((char *)s);
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
	  
static SLang_Intrin_Fun_Type Strops_Table [] = /*{{{*/
{
   MAKE_INTRINSIC_I("create_delimited_string",  create_delimited_string_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SS("strcmp",  strcmp_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_SSI("strncmp",  strncmp_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_SSI("strncharcmp",  strncharcmp_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_SSI("strnbytecmp",  strnbytecmp_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("strcat",  strcat_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("strlen",  strlen_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_S("strcharlen",  strcharlen_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_S("strbytelen",  strbytelen_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_3("strchop", strchop_cmd, SLANG_VOID_TYPE, SLANG_STRING_TYPE, SLANG_WCHAR_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_3("strchopr", strchopr_cmd, SLANG_VOID_TYPE, SLANG_STRING_TYPE, SLANG_WCHAR_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_I("strreplace", strreplace_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_SSS("str_replace", str_replace_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_SII("substr",  substr_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SII("substrbytes",  subbytes_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SS("is_substr",  issubstr_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_2("strsub",  strsub_cmd, SLANG_VOID_TYPE, SLANG_INT_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_2("strbytesub",  strbytesub_cmd, SLANG_VOID_TYPE, SLANG_INT_TYPE, SLANG_UCHAR_TYPE),
   MAKE_INTRINSIC_3("extract_element", extract_element_cmd, SLANG_VOID_TYPE, SLANG_STRING_TYPE, SLANG_INT_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_3("is_list_element", is_list_element_cmd, SLANG_INT_TYPE, SLANG_STRING_TYPE, SLANG_STRING_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_SSI("string_match", string_match_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_SSI("string_matches", string_matches_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_I("string_match_nth", string_match_nth_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("strlow", strlow_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_1("tolower", tolower_cmd, SLANG_INT_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_1("toupper", toupper_cmd, SLANG_INT_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_0("strup", strup_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("isdigit",  isdigit_cmd, SLANG_INT_TYPE),
   MAKE_INTRINSIC_S("strtrim", strtrim_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("strtrim_end", strtrim_end_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("strtrim_beg", strtrim_beg_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SS("strcompress", strcompress_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_I("Sprintf", sprintf_n_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("sprintf", sprintf_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("sscanf", _pSLang_sscanf, SLANG_INT_TYPE),
   MAKE_INTRINSIC_S("make_printable_string", make_printable_string, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_3("str_quote_string", str_quote_string_cmd, SLANG_VOID_TYPE, SLANG_STRING_TYPE, SLANG_STRING_TYPE, SLANG_WCHAR_TYPE),
   MAKE_INTRINSIC_SSS("str_uncomment_string", str_uncomment_string_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_II("define_case", define_case_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("strtok", strtok_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("strjoin", strjoin_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SSS("strtrans", strtrans_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SS("str_delete_chars", str_delete_chars_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("glob_to_regexp", glob_to_regexp, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_2("count_char_occurances", count_char_occurances, SLANG_UINT_TYPE, SLANG_STRING_TYPE, SLANG_WCHAR_TYPE),

   SLANG_END_INTRIN_FUN_TABLE
};

/*}}}*/

int _pSLang_init_slstrops (void)
{
   return SLadd_intrin_fun_table (Strops_Table, NULL);
}
