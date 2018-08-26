/* -*- mode: C; mode: fold -*-
Copyright (C) 2010-2011 John E. Davis

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
#include "config.h"

#include <stdio.h>
#include <slang.h>
#include <string.h>
#include <pcre.h>

SLANG_MODULE(pcre);

static int PCRE_Type_Id = 0;

typedef struct
{
   pcre *p;
   pcre_extra *extra;
   int *ovector;
   unsigned int ovector_len;	       /* must be a multiple of 3 */
   unsigned int num_matches;	       /* return value of pcre_exec (>= 1)*/
}
PCRE_Type;

static void free_pcre_type (PCRE_Type *pt)
{
   if (pt->ovector != NULL)
     SLfree ((char *) pt->ovector);

   SLfree ((char *) pt);
}

static SLang_MMT_Type *allocate_pcre_type (pcre *p, pcre_extra *extra)
{
   PCRE_Type *pt;
   SLang_MMT_Type *mmt;
   int ovector_len;

   pt = (PCRE_Type *) SLmalloc (sizeof (PCRE_Type));
   if (pt == NULL)
     return NULL;
   memset ((char *) pt, 0, sizeof (PCRE_Type));

   pt->p = p;
   pt->extra = extra;

   if (0 != pcre_fullinfo (p, extra, PCRE_INFO_CAPTURECOUNT, &ovector_len))
     {
	free_pcre_type (pt);
	SLang_verror (SL_INTRINSIC_ERROR, "pcre_fullinfo failed");
	return NULL;
     }

   ovector_len += 1;		       /* allow for pattern matched */
   ovector_len *= 3;		       /* required to be multiple of 3 */
   if (NULL == (pt->ovector = (int *)SLmalloc (ovector_len * sizeof (int))))
     {
	free_pcre_type (pt);
	return NULL;
     }
   pt->ovector_len = ovector_len;

   if (NULL == (mmt = SLang_create_mmt (PCRE_Type_Id, (VOID_STAR) pt)))
     {
	free_pcre_type (pt);
	return NULL;
     }
   return mmt;
}

static int _pcre_compile_1 (char *pattern, int options)
{
   pcre *p;
   pcre_extra *extra;
   SLCONST char *err;
   int erroffset;
   unsigned char *table;
   SLang_MMT_Type *mmt;

   table = NULL;
   p = pcre_compile (pattern, options, &err, &erroffset, table);
   if (NULL == p)
     {
	SLang_verror (SL_Parse_Error, "Error compiling pattern '%s' at offset %d: %s",
		      pattern, erroffset, err);
	return -1;
     }

   extra = pcre_study (p, 0, &err);
   /* apparantly, a NULL return is ok */
   if (err != NULL)
     {
	SLang_verror (SL_INTRINSIC_ERROR, "pcre_study failed: %s", err);
	pcre_free (p);
	return -1;
     }

   if (NULL == (mmt = allocate_pcre_type (p, extra)))
     {
	pcre_free ((char *) p);
	pcre_free ((char *) extra);
	return -1;
     }

   if (-1 == SLang_push_mmt (mmt))
     {
	SLang_free_mmt (mmt);
	return -1;
     }
   return 0;
}

static void _pcre_compile (void)
{
   char *pattern;
   int options = 0;

   switch (SLang_Num_Function_Args)
     {
      case 2:
	if (-1 == SLang_pop_integer (&options))
	  return;
	/* drop */
      case 1:
      default:
	if (-1 == SLang_pop_slstring (&pattern))
	  return;
     }
   (void) _pcre_compile_1 (pattern, options);
   SLang_free_slstring (pattern);
}

/* returns number of matches */
static int _pcre_exec_1 (PCRE_Type *pt, char *str, unsigned int len, int pos, int options)
{
   int rc;

   pt->num_matches = 0;
   if ((unsigned int) pos > len)
     return 0;

   rc = pcre_exec (pt->p, pt->extra, str, len, pos,
		   options, pt->ovector, pt->ovector_len);

   if (rc == PCRE_ERROR_NOMATCH)
     return 0;

   if (rc <= 0)
     {
	SLang_verror (SL_INTRINSIC_ERROR, "pcre_exec returned %d", rc);
	return -1;
     }
   pt->num_matches = (unsigned int) rc;
   return rc;
}

static int _pcre_exec (void)
{
   PCRE_Type *p;
   SLang_MMT_Type *mmt;
   char *str;
   SLang_BString_Type *bstr = NULL;
   unsigned int len;
   int pos = 0;
   int options = 0;
   int ret = -1;

   switch (SLang_Num_Function_Args)
     {
      case 4:
	if (-1 == SLang_pop_integer (&options))
	  return -1;
	/* drop */
      case 3:
	/* drop */
	if (-1 == SLang_pop_integer (&pos))
	  return -1;
	/* drop */
      default:
	switch (SLang_peek_at_stack())
	  {
	   case SLANG_STRING_TYPE:
	     if (-1 == SLang_pop_slstring (&str))
	       return -1;
	     len = strlen (str);
	     break;

	   case SLANG_BSTRING_TYPE:
	   default:
	     if (-1 == SLang_pop_bstring(&bstr))
	       return -1;
	     str = (char *)SLbstring_get_pointer(bstr, &len);
	     if (str == NULL)
	       {
		  SLbstring_free (bstr);
		  return -1;
	       }
	     break;
	  }
     }

   if (NULL == (mmt = SLang_pop_mmt (PCRE_Type_Id)))
     goto free_and_return;
   p = (PCRE_Type *)SLang_object_from_mmt (mmt);

   ret = _pcre_exec_1 (p, str, len, pos, options);

free_and_return:

   SLang_free_mmt (mmt);
   if (bstr != NULL)
     SLbstring_free (bstr);
   else
     SLang_free_slstring (str);
   return ret;
}

static int get_nth_start_stop (PCRE_Type *pt, unsigned int n,
			       unsigned int *a, unsigned int *b)
{
   int start, stop;

   if (n >= pt->num_matches)
     return -1;

   start = pt->ovector[2*n];
   stop = pt->ovector[2*n+1];
   if ((start < 0) || (stop < start))
     return -1;

   *a = (unsigned int) start;
   *b = (unsigned int) stop;
   return 0;
}

static void _pcre_nth_match (PCRE_Type *pt, int *np)
{
   unsigned int start, stop;
   SLang_Array_Type *at;
   SLindex_Type two = 2;
   int *data;

   if (-1 == get_nth_start_stop (pt, (unsigned int) *np, &start, &stop))
     {
	SLang_push_null ();
	return;
     }

   if (NULL == (at = SLang_create_array (SLANG_INT_TYPE, 0, NULL, &two, 1)))
     return;

   data = (int *)at->data;
   data[0] = (int)start;
   data[1] = (int)stop;
   (void) SLang_push_array (at, 1);
}

static void _pcre_nth_substr (PCRE_Type *pt, char *str, int *np)
{
   unsigned int start, stop;
   unsigned int len;

   len = strlen (str);

   if ((-1 == get_nth_start_stop (pt, (unsigned int) *np, &start, &stop))
       || (start > len) || (stop > len))
     {
	SLang_push_null ();
	return;
     }

   str = SLang_create_nslstring (str + start, stop - start);
   (void) SLang_push_string (str);
   SLang_free_slstring (str);
}

/* This function converts a slang RE to a pcre expression.  It performs the
 * following transformations:
 *    (     -->   \(
 *    )     -->   \)
 *    #     -->   \#
 *    |     -->   \|
 *    {     -->   \{
 *    }     -->   \}
 *   \<     -->   \b
 *   \>     -->   \b
 *   \C     -->   (?i)
 *   \c     -->   (?-i)
 *   \(     -->   (
 *   \)     -->   )
 *   \{     -->   {
 *   \}     -->   }
 * Anything else?
 */
static char *_slang_to_pcre (char *slpattern)
{
   char *pattern, *p, *s;
   unsigned int len;
   int in_bracket;
   char ch;

   len = strlen (slpattern);
   pattern = SLmalloc (3*len + 1);
   if (pattern == NULL)
     return NULL;

   p = pattern;
   s = slpattern;
   in_bracket = 0;
   while ((ch = *s++) != 0)
     {
	switch (ch)
	  {
	   case '{':
	   case '}':
	   case '(':
	   case ')':
	   case '#':
	   case '|':
	     if (0 == in_bracket) *p++ = '\\';
	     *p++ = ch;
	     break;

	   case '[':
	     in_bracket = 1;
	     *p++ = ch;
	     break;

	   case ']':
	     in_bracket = 0;
	     *p++ = ch;
	     break;

	   case '\\':
	     ch = *s++;
	     switch (ch)
	       {
		case 0:
		  s--;
		  break;

		case '<':
		case '>':
		  *p++ = '\\'; *p++ = 'b';
		  break;

		case '(':
		case ')':
		case '{':
		case '}':
		  *p++ = ch;
		  break;

		case 'C':
		  *p++ = '('; *p++ = '?'; *p++ = 'i'; *p++ = ')';
		  break;
		case 'c':
		  *p++ = '('; *p++ = '?'; *p++ = '-'; *p++ = 'i'; *p++ = ')';
		  break;

		default:
		  *p++ = '\\';
		  *p++ = ch;
	       }
	     break;

	   default:
	     *p++ = ch;
	     break;
	  }
     }
   *p = 0;

   s = SLang_create_slstring (pattern);
   SLfree (pattern);
   return s;
}

static void slang_to_pcre (char *pattern)
{
   /* NULL ok in code below */
   pattern = _slang_to_pcre (pattern);
   (void) SLang_push_string (pattern);
   SLang_free_slstring (pattern);
}

static void destroy_pcre (SLtype type, VOID_STAR f)
{
   PCRE_Type *pt;
   (void) type;

   pt = (PCRE_Type *) f;
   if (pt->extra != NULL)
     pcre_free ((char *) pt->extra);
   if (pt->p != NULL)
     pcre_free ((char *) pt->p);
   free_pcre_type (pt);
}

#define DUMMY_PCRE_TYPE ((SLtype)-1)
#define P DUMMY_PCRE_TYPE
#define I SLANG_INT_TYPE
#define V SLANG_VOID_TYPE
#define S SLANG_STRING_TYPE
static SLang_Intrin_Fun_Type PCRE_Intrinsics [] =
{
   MAKE_INTRINSIC_0("pcre_exec", _pcre_exec, I),
   MAKE_INTRINSIC_0("pcre_compile", _pcre_compile, V),
   MAKE_INTRINSIC_2("pcre_nth_match", _pcre_nth_match, V, P, I),
   MAKE_INTRINSIC_3("pcre_nth_substr", _pcre_nth_substr, V, P, S, I),
   MAKE_INTRINSIC_1("slang_to_pcre", slang_to_pcre, V, S),
   SLANG_END_INTRIN_FUN_TABLE
};

static SLang_IConstant_Type PCRE_Consts [] =
{
   /* compile options */
   MAKE_ICONSTANT("PCRE_ANCHORED", PCRE_ANCHORED),
   MAKE_ICONSTANT("PCRE_CASELESS", PCRE_CASELESS),
   MAKE_ICONSTANT("PCRE_DOLLAR_ENDONLY", PCRE_DOLLAR_ENDONLY),
   MAKE_ICONSTANT("PCRE_DOTALL", PCRE_DOTALL),
   MAKE_ICONSTANT("PCRE_EXTENDED", PCRE_EXTENDED),
   MAKE_ICONSTANT("PCRE_EXTRA", PCRE_EXTRA),
   MAKE_ICONSTANT("PCRE_MULTILINE", PCRE_MULTILINE),
   MAKE_ICONSTANT("PCRE_UNGREEDY", PCRE_UNGREEDY),
   MAKE_ICONSTANT("PCRE_UTF8", PCRE_UTF8),

   /* exec options */
   MAKE_ICONSTANT("PCRE_NOTBOL", PCRE_NOTBOL),
   MAKE_ICONSTANT("PCRE_NOTEOL", PCRE_NOTEOL),
   MAKE_ICONSTANT("PCRE_NOTEMPTY", PCRE_NOTEMPTY),
   SLANG_END_ICONST_TABLE
};

#undef P
#undef I
#undef V
#undef S

static int register_pcre_type (void)
{
   SLang_Class_Type *cl;

   if (PCRE_Type_Id != 0)
     return 0;

   if (NULL == (cl = SLclass_allocate_class ("PCRE_Type")))
     return -1;

   if (-1 == SLclass_set_destroy_function (cl, destroy_pcre))
     return -1;

   /* By registering as SLANG_VOID_TYPE, slang will dynamically allocate a
    * type.
    */
   if (-1 == SLclass_register_class (cl, SLANG_VOID_TYPE, sizeof (PCRE_Type), SLANG_CLASS_TYPE_MMT))
     return -1;

   PCRE_Type_Id = SLclass_get_class_id (cl);
   if (-1 == SLclass_patch_intrin_fun_table1 (PCRE_Intrinsics, DUMMY_PCRE_TYPE, PCRE_Type_Id))
     return -1;

   return 0;
}

static void *do_malloc (size_t n)
{
   return (void *) SLmalloc (n);
}

static void do_free (void *x)
{
   SLfree ((char *) x);
}

int init_pcre_module_ns (char *ns_name)
{
   SLang_NameSpace_Type *ns = SLns_create_namespace (ns_name);
   if (ns == NULL)
     return -1;

   if (-1 == register_pcre_type ())
     return -1;

   pcre_malloc = do_malloc;
   pcre_free = do_free;

   if ((-1 == SLns_add_intrin_fun_table (ns, PCRE_Intrinsics, "__PCRE__"))
       || (-1 == SLns_add_iconstant_table (ns, PCRE_Consts, NULL)))
     return -1;

   return 0;
}

/* This function is optional */
void deinit_pcre_module (void)
{
}

