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

#include "slang.h"
#include "_slang.h"

#ifdef upcase
# undef upcase
#endif

/* The Boyer-Moore search algorthm is used below under the following
 * conditions:
 *
 *    1.  The search is case-sensitive.
 *    3.  UTF-8 mode is not active.
 *    2.  The search key contains only of single byte characters.
 *    4.  The multi-byte characters in the key are case-less.
 *
 * Otherwise, the search is case-insensitive and the key consists of
 * multi-byte characters.  In this case, a modified BM algorithm is used
 * _if_ each character in the key is composed of multi-byte characters that
 * have upper and lower case versions of the same length.
 *
 * Otherwise, the search search will be performed in a brute-force manner.
 */

typedef struct
{
   SLuchar_Type *key;
   unsigned int key_len;
   unsigned int fskip_table[256];
   unsigned int bskip_table[256];
}
BoyerMoore_Search_Type;

typedef struct
{
   SLuchar_Type **lower_chars;
   SLuchar_Type **upper_chars;
   unsigned int nlower_chars;
   unsigned int nupper_chars;
   SLsearch_Type *st;
}
BruteForce_Search_Type;

struct _pSLsearch_Type
{
   SLuchar_Type *(*search_fun)(SLsearch_Type *, SLuchar_Type *, SLuchar_Type *, SLuchar_Type *, int);
   void (*free_fun) (SLsearch_Type *);
   int flags;
   unsigned int match_len;
   union
     {
        BoyerMoore_Search_Type bm;
        BruteForce_Search_Type bf;
     }
   s;
};

/* Simple BM search--- case-sensitive, or case-less, or no UTF-8, or 7-bit ascii */
static SLuchar_Type *
  bm_search_forward (SLsearch_Type *st, SLuchar_Type *beg, SLuchar_Type *end)
{
   register unsigned char char1;
   unsigned char *pos;
   unsigned int *skip_table;
   SLuchar_Type *key;
   unsigned int key_len;
   int case_fold;
   BoyerMoore_Search_Type *bm;

   bm = &st->s.bm;
   key_len = bm->key_len;

   st->match_len = 0;

   if ((key_len > (unsigned int) (end - beg))
       || (key_len == 0))
     return NULL;

   case_fold = st->flags & SLSEARCH_CASELESS;
   key = bm->key;
   skip_table = bm->fskip_table;

   char1 = key[key_len - 1];
   beg += (key_len - 1);

   while(1)
     {
	SLuchar_Type ch;
	unsigned int dbeg;
        unsigned int j;

        while (beg < end)
          {
             ch = *beg;
             dbeg = skip_table[ch];
             if ((dbeg < key_len)
                 && ((ch == char1)
                     || (case_fold && (char1 == UPPER_CASE(ch)))))
               break;
             beg += dbeg;
          }
	if (beg >= end) return NULL;

	pos = beg - (key_len - 1);
	for (j = 0; j < key_len; j++)
	  {
             ch = pos[j];
	     if ((key[j] != ch)
                 && ((case_fold == 0)
                     || (key[j] != UPPER_CASE(ch))))
               break;
          }

	if (j == key_len)
          {
             st->match_len = key_len;
             return pos;
          }

	beg += 1;
     }
}

static SLuchar_Type *
  bm_search_backward (SLsearch_Type *st,
                      SLuchar_Type *beg, SLuchar_Type *start, SLuchar_Type *end)
{
   SLuchar_Type char1;
   unsigned int j, ofs;
   unsigned int key_len;
   SLuchar_Type *key;
   int case_fold;
   unsigned int *skip_table;
   BoyerMoore_Search_Type *bm;

   st->match_len = 0;
   bm = &st->s.bm;

   key_len = bm->key_len;
   if ((key_len > (unsigned int) (end - beg))
       || (key_len == 0)
       || (end <= beg)
       || (start < beg)
       || (start >= end))
     return NULL;

   case_fold = st->flags & SLSEARCH_CASELESS;
   key = bm->key;
   skip_table = bm->bskip_table;

   if (start + key_len > end)
     start = end - key_len;

   char1 = key[0];

    while(1)
      {
         SLuchar_Type ch;

	 while (beg <= start)
	   {
              ch = *start;

              if ((ch == char1)
                  || (case_fold && (char1 == UPPER_CASE(ch))))
                break;

              ofs = skip_table[ch];
	      start -= ofs;
	   }
	 if (beg > start) return NULL;

	 for (j = 1; j < key_len; j++)
	   {
              ch = start[j];
              if ((key[j] != ch)
                  && ((case_fold == 0)
                      || (key[j] != UPPER_CASE(ch))))
                break;
	   }
	 if (j == key_len)
           {
              st->match_len = key_len;
              return start;
           }
	 start--;
      }
}

static SLuchar_Type *bm_search (SLsearch_Type *st,
                                SLuchar_Type *pmin, SLuchar_Type *p, SLuchar_Type *pmax,
                                int dir)
{
   if (dir > 0)
     return bm_search_forward (st, p, pmax);
   else
     return bm_search_backward (st, pmin, p, pmax);
}

/* Return value, if non-NULL returns pointer to the end of the string */
static SLuchar_Type *
  looking_at_bf (SLuchar_Type *pmin, SLuchar_Type *pmax,
                 SLuchar_Type **lower_chars, unsigned int nlower_chars,
                 SLuchar_Type **upper_chars, unsigned int nupper_chars)

{
   unsigned int n;

   n = 0;
   while ((n < nupper_chars) && (n < nlower_chars))
     {
        SLuchar_Type *up, *lo, *p;
        up = upper_chars[n];
        lo = lower_chars[n];

        n++;

        p = pmin;
        while ((p < pmax)
               && (*up == *p)
               && (*up != 0))
          {
             up++;
             p++;
          }

        if (*up == 0)
          {
             pmin = p;
             continue;
          }

        p = pmin;
        while ((p < pmax)
               && (*lo == *p)
               && (*lo != 0))
          {
             lo++;
             p++;
          }

        if (*lo == 0)
          {
             pmin = p;
             continue;
          }

        return NULL;
     }
   return pmin;
}

/* Brute force case-insensitive searches */
static SLuchar_Type *
  bf_search_forward (SLsearch_Type *st,
                     SLuchar_Type *pmin, SLuchar_Type *pmax)
{
   SLsearch_Type *bf_st;
   SLuchar_Type chup, chlo;
   SLuchar_Type **upper_chars;
   SLuchar_Type **lower_chars;
   unsigned int nupper_chars, nlower_chars;

   bf_st = st->s.bf.st;
   upper_chars = st->s.bf.upper_chars;
   lower_chars = st->s.bf.lower_chars;
   nupper_chars = st->s.bf.nupper_chars;
   nlower_chars = st->s.bf.nlower_chars;

   chup = upper_chars[0][0];
   chlo = lower_chars[0][0];

   while (1)
     {
        SLuchar_Type *p;

        if (bf_st != NULL)
          {
             if (NULL == (pmin = SLsearch_forward (bf_st, pmin, pmax)))
               {
                  st->match_len = 0;
                  return NULL;
               }
             p = pmin + bf_st->match_len;
          }
        else
          {
             while (pmin < pmax)
               {
                  if ((*pmin == chup) || (*pmin == chlo))
                    break;

                  pmin++;
               }

             if (pmin >= pmax)
               {
                  st->match_len = 0;
                  return NULL;
               }

             p = pmin;
          }

        p = looking_at_bf (p, pmax, lower_chars, nlower_chars,
                           upper_chars, nupper_chars);

        if (p != NULL)
          {
             st->match_len = p - pmin;
             return pmin;
          }

        pmin++;
     }
}

static SLuchar_Type *
  bf_search_backward (SLsearch_Type *st,
                      SLuchar_Type *pmin, SLuchar_Type *start, SLuchar_Type *pmax)
{
   SLsearch_Type *bf_st;
   SLuchar_Type chup, chlo;
   SLuchar_Type **upper_chars;
   SLuchar_Type **lower_chars;
   unsigned int nupper_chars, nlower_chars;

   bf_st = st->s.bf.st;
   upper_chars = st->s.bf.upper_chars;
   lower_chars = st->s.bf.lower_chars;
   nupper_chars = st->s.bf.nupper_chars;
   nlower_chars = st->s.bf.nlower_chars;

   chup = upper_chars[0][0];
   chlo = lower_chars[0][0];

   while (1)
     {
        SLuchar_Type *p;

        if (bf_st != NULL)
          {
             if (NULL == (start = SLsearch_backward (bf_st, pmin, start+1, pmax)))
               {
                  st->match_len = 0;
                  return NULL;
               }
             p = start + bf_st->match_len;
          }
        else
          {
             /* start--; */
             while (start >= pmin)
               {
                  if ((*start == chup) || (*start == chlo))
                    break;
                  start--;
               }

             if (start < pmin)
               {
                  st->match_len = 0;
                  return NULL;
               }

             p = start;
          }

        p = looking_at_bf (p, pmax, lower_chars, nlower_chars,
                           upper_chars, nupper_chars);

        if (p != NULL)
          {
             st->match_len = p - start;
             return start;
          }

        start--;
     }
}

static SLuchar_Type *
  bf_search (SLsearch_Type *st,
             SLuchar_Type *pmin, SLuchar_Type *p, SLuchar_Type *pmax,
             int dir)
{
   if (dir > 0)
     return bf_search_forward (st, p, pmax);
   else
     return bf_search_backward (st, pmin, p, pmax);
}

SLuchar_Type *SLsearch_forward (SLsearch_Type *st,
                                SLuchar_Type *pmin, SLuchar_Type *pmax)
{
   if (st == NULL)
     return NULL;

   return st->search_fun (st, pmin, pmin, pmax, 1);
}

SLuchar_Type *SLsearch_backward (SLsearch_Type *st, SLuchar_Type *pmin,
                                 SLuchar_Type *p, SLuchar_Type *pmax)
{
   if (st == NULL)
     return NULL;

   /* For the backward search, the first character of the string
    * is assumed satisfy pmin <= BEG < p
    * and end satisfies pmin < END <= pmax
    */
   return st->search_fun (st, pmin, p-1, pmax, -1);
}

static int Case_Tables_Ok;

static void init_skip_table (SLuchar_Type *key, unsigned int key_len,
			     unsigned int *skip_table, int dir, int flags)
{
   unsigned int i;

   for (i = 0; i < 256; i++)
     skip_table[i] = key_len;

   if (dir < 0)
     key += (key_len-1);

   /* For a case-insensitive search, the key here will be uppercased */
   flags = flags & SLSEARCH_CASELESS;
   i = 0;
   while (i < key_len)
     {
	i++;
	skip_table[*key] = key_len - i;
	if (flags)
	  skip_table[LOWER_CASE(*key)] = key_len - i;
	key += dir;
     }
}

static void bm_free (SLsearch_Type *st)
{
   SLang_free_slstring ((char *) st->s.bm.key);
}

static void bf_free (SLsearch_Type *st)
{
   unsigned int i, n;
   SLuchar_Type **a;

   if (NULL != (a = st->s.bf.lower_chars))
     {
        n = st->s.bf.nlower_chars;
        for (i = 0; i < n; i++)
          SLang_free_slstring ((char *) a[i]);
        SLfree ((char *) a);
     }

   if (NULL != (a = st->s.bf.upper_chars))
     {
        n = st->s.bf.nupper_chars;
        for (i = 0; i < n; i++)
          SLang_free_slstring ((char *) a[i]);
        SLfree ((char *) a);
     }
}

unsigned int SLsearch_match_len (SLsearch_Type *st)
{
   if (st == NULL)
     return 0;

   return st->match_len;
}

void SLsearch_delete (SLsearch_Type *st)
{
   if (st == NULL)
     return;

   (*st->free_fun) (st);
   SLfree ((char *)st);
}

/* This is used if the key is not UTF-8, or it is but the search is case-sensitive */
static SLsearch_Type *bm_open_search (SLuchar_Type *key, int flags)
{
   SLsearch_Type *st;
   unsigned int keylen;

   keylen = strlen ((char *)key);
   if (NULL == (st = (SLsearch_Type *)SLcalloc (1, sizeof (SLsearch_Type))))
     return NULL;

   st->free_fun = bm_free;

   /* If the search is case-insensitive, then it must either be all ascii, or
    * it is not unicode.  In either case, the UPPER_CASE and LOWER_CASE macros
    * should be ok to use.
    */
   if (flags & SLSEARCH_CASELESS)
     {
	char *keyup = SLmake_nstring ((char *)key, keylen);
	if (keyup != NULL)
	  {
	     unsigned char *k = (unsigned char *)keyup;
	     while (*k != 0)
	       {
		  *k = UPPER_CASE(*k);
		  k++;
	       }
	     st->s.bm.key = (SLuchar_Type *)SLang_create_slstring (keyup);
	     SLfree (keyup);
	  }
	else st->s.bm.key = NULL;
     }
   else st->s.bm.key = (SLuchar_Type*) SLang_create_slstring ((char *)key);

   if (st->s.bm.key == NULL)
     {
        SLsearch_delete (st);
        return NULL;
     }
   st->s.bm.key_len = keylen;
   st->flags = flags;

   st->search_fun = bm_search;

   init_skip_table (st->s.bm.key, st->s.bm.key_len, st->s.bm.fskip_table, 1, flags);
   init_skip_table (st->s.bm.key, st->s.bm.key_len, st->s.bm.bskip_table, -1, flags);
   return st;
}

static int is_bm_ok (SLuchar_Type *key, unsigned int len, SLuchar_Type **non_ascii)
{
   SLuchar_Type *keymax;

   /* See if the key is ascii-only */
   keymax = key + len;

   while (key < keymax)
     {
        if (*key & 0x80)
          {
             *non_ascii = key;
             return 0;
          }

        key++;
     }

   return 1;
}

static SLuchar_Type **make_string_array (SLuchar_Type *u, unsigned int len, unsigned int *nump)
{
   SLuchar_Type *umax;
   SLuchar_Type **a;
   unsigned int num, i;
   int ignore_combining = 0;

   num = SLutf8_strlen (u, ignore_combining);
   if (num == 0)
     return NULL;                      /* should not happen */

   if (NULL == (a = (SLuchar_Type **) SLcalloc (sizeof (SLuchar_Type *), num)))
     return NULL;

   umax = u + len;
   for (i = 0; i < num; i++)
     {
        SLuchar_Type *u1 = SLutf8_skip_char (u, umax);
        if (NULL == (a[i] = (SLuchar_Type *)SLang_create_nslstring ((char *)u, u1 - u)))
          goto return_error;
	u = u1;
     }
   *nump = num;
   return a;

   return_error:
   for (i = 0; i < num; i++)
     SLang_free_slstring ((char *) a[i]);
   SLfree ((char *) a);
   return NULL;
}

SLsearch_Type *SLsearch_new (SLuchar_Type *key, int flags)
{
   SLsearch_Type *st, *bf_st;
   SLuchar_Type *key_upper, *key_lower, *non_ascii;
   unsigned int len, upper_len, lower_len;

   if (Case_Tables_Ok == 0)
     SLang_init_case_tables ();

   if (key == NULL)
     return NULL;

   if ((0 == (flags & SLSEARCH_CASELESS))
       || (0 == (flags & SLSEARCH_UTF8)))
     return bm_open_search (key, flags);

   /* Otherwise the key is UTF-8 and the search is case-insensitive */
   len = strlen ((char *)key);
   key_upper = SLutf8_strup (key, key + len);
   if (key_upper == NULL)
     return NULL;

   upper_len = strlen ((char *)key_upper);

   if (is_bm_ok (key_upper, upper_len, &non_ascii))
     {
        st = bm_open_search (key_upper, flags);
        SLang_free_slstring ((char *)key_upper);
        return st;
     }

   /* Tricky part */

   if (NULL == (key_lower = SLutf8_strlo (key, key + len)))
     {
        SLang_free_slstring ((char *)key_upper);
        return NULL;
     }

   lower_len = strlen ((char *)key_lower);

   /* Try a case-less search */
   if ((lower_len == upper_len)
       && (0 == strcmp ((char *)key_upper, (char *)key_lower)))
     {
        flags &= ~SLSEARCH_CASELESS;
        st = bm_open_search (key_upper, flags);
        SLang_free_slstring ((char *)key_upper);
        SLang_free_slstring ((char *)key_lower);
        return st;
     }

   /* Now Perform a brute-force search. */

   /* If the first few characters of the search string are ascii, then
    * use BM for that portion
    */
   bf_st = NULL;
   if (non_ascii - key_upper >= 3)
     {
        SLuchar_Type *key1 = (SLuchar_Type *) SLmake_nstring ((char *)key_upper, non_ascii - key_upper);

        /* ok to propagate NULL below */
        bf_st = SLsearch_new (key1, flags);
        SLfree ((char *)key1);
        if (bf_st == NULL)
          {
             SLang_free_slstring ((char *)key_upper);
             SLang_free_slstring ((char *)key_lower);
             return NULL;
          }

        key1 = (SLuchar_Type *) SLang_create_slstring ((char *)non_ascii);
        non_ascii = key_lower + (non_ascii - key_upper);
        SLang_free_slstring ((char *)key_upper);
        key_upper = key1;

        key1 = (SLuchar_Type *)SLang_create_slstring ((char *)non_ascii);
        SLang_free_slstring ((char *)key_lower);
        key_lower = key1;

        if ((key_lower == NULL) || (key_upper == NULL))
          {
             SLang_free_slstring ((char *)key_upper);
             SLang_free_slstring ((char *)key_lower);
             SLsearch_delete (bf_st);
             return NULL;
          }
        upper_len = strlen ((char *)key_upper);
        lower_len = strlen ((char *)key_lower);
     }

   st = (SLsearch_Type *)SLcalloc (sizeof (SLsearch_Type), 1);
   if (st == NULL)
     goto return_error;
   st->free_fun = bf_free;
   st->flags = flags;
   st->search_fun = bf_search;

   st->s.bf.st = bf_st;  bf_st = NULL;

   if (NULL == (st->s.bf.lower_chars = make_string_array (key_lower, lower_len, &st->s.bf.nlower_chars)))
     goto return_error;

   if (NULL == (st->s.bf.upper_chars = make_string_array (key_upper, upper_len, &st->s.bf.nupper_chars)))
     goto return_error;

   SLang_free_slstring ((char *)key_upper);
   SLang_free_slstring ((char *)key_lower);
   return st;

   return_error:
   SLsearch_delete (st);
   SLsearch_delete (bf_st);
   SLang_free_slstring ((char *)key_upper);
   SLang_free_slstring ((char *)key_lower);
   return NULL;
}

/* 8bit clean upper and lowercase tables.  These are used _only_ when UTF-8
 * mode is not active, or when uppercasing ASCII.
 */
unsigned char _pSLChg_LCase_Lut[256] = {0};
unsigned char _pSLChg_UCase_Lut[256] = {0};

void SLang_define_case (int *u, int *l)
{
   unsigned char up = (unsigned char) *u, dn = (unsigned char) *l;

   _pSLChg_LCase_Lut[up] = dn;
   _pSLChg_LCase_Lut[dn] = dn;
   _pSLChg_UCase_Lut[dn] = up;
   _pSLChg_UCase_Lut[up] = up;
}

void SLang_init_case_tables (void)
{
   int i, j;
   if (Case_Tables_Ok) return;

   for (i = 0; i < 256; i++)
     {
	_pSLChg_UCase_Lut[i] = i;
	_pSLChg_LCase_Lut[i] = i;
     }

   for (i = 'A'; i <= 'Z'; i++)
     {
	j = i + 32;
	_pSLChg_UCase_Lut[j] = i;
	_pSLChg_LCase_Lut[i] = j;
     }
#ifdef PC_SYSTEM
   /* Initialize for DOS code page 437. */
   _pSLChg_UCase_Lut[135] = 128; _pSLChg_LCase_Lut[128] = 135;
   _pSLChg_UCase_Lut[132] = 142; _pSLChg_LCase_Lut[142] = 132;
   _pSLChg_UCase_Lut[134] = 143; _pSLChg_LCase_Lut[143] = 134;
   _pSLChg_UCase_Lut[130] = 144; _pSLChg_LCase_Lut[144] = 130;
   _pSLChg_UCase_Lut[145] = 146; _pSLChg_LCase_Lut[146] = 145;
   _pSLChg_UCase_Lut[148] = 153; _pSLChg_LCase_Lut[153] = 148;
   _pSLChg_UCase_Lut[129] = 154; _pSLChg_LCase_Lut[154] = 129;
   _pSLChg_UCase_Lut[164] = 165; _pSLChg_LCase_Lut[165] = 164;
#else
   /* ISO Latin */
   for (i = 192; i <= 221; i++)
     {
	j = i + 32;
	_pSLChg_UCase_Lut[j] = i;
	_pSLChg_LCase_Lut[i] = j;
     }
   _pSLChg_UCase_Lut[215] = 215; _pSLChg_LCase_Lut[215] = 215;
   _pSLChg_UCase_Lut[223] = 223; _pSLChg_LCase_Lut[223] = 223;
   _pSLChg_UCase_Lut[247] = 247; _pSLChg_LCase_Lut[247] = 247;
   _pSLChg_UCase_Lut[255] = 255; _pSLChg_LCase_Lut[255] = 255;
#endif
   Case_Tables_Ok = 1;
}
