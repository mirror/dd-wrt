/*
Copyright (C) 2007-2011 John E. Davis

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

#include <oniguruma.h>

SLANG_MODULE(onig);

#if 0
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
#endif

static int slOnig_Error = -1;
static int Onig_Type_Id = 0;

static void throw_onig_error (int err_code, OnigErrorInfo *einfo)
{
   UChar err_buf[ONIG_MAX_ERROR_MESSAGE_LEN];

   (void) onig_error_code_to_str (err_buf, err_code, einfo);
   SLang_verror (slOnig_Error, "%s", err_buf);
}

/* These 3 typedefs should be merged into a union, but C does not support
 * initializing a union.
 */
typedef struct
{
   char *name;
   VOID_STAR ptr;
}
Name_Map_Type;

typedef struct
{
   char *name;
   OnigSyntaxType *syn;
}
Syntax_Table_Map_Type;

typedef struct
{
   char *name;
   OnigEncodingType *encoding;
}
Encoding_Table_Map_Type;

static VOID_STAR pop_onig_name_ptr (Name_Map_Type *map, char *onig_object)
{
   char *str;

   if (-1 == SLang_pop_slstring (&str))
     return NULL;

   while (map->name != NULL)
     {
	if (0 == strcmp (str, map->name))
	  {
	     SLang_free_slstring (str);
	     return map->ptr;
	  }
	map++;
     }

   SLang_verror (SL_InvalidParm_Error, "Unsupported or unknown onig %s: %s", onig_object, str);
   SLang_free_slstring (str);
   return NULL;
}

static void get_onig_names (Name_Map_Type *map)
{
   SLindex_Type i, num;
   SLang_Array_Type *at;
   char **names;
   Name_Map_Type *table;

   table = map;
   while (table->name != NULL)
     table++;
   num = (SLindex_Type) (table - map);

   if (NULL == (at = SLang_create_array (SLANG_STRING_TYPE, 0, NULL, &num, 1)))
     return;

   table = map;
   names = (char **)at->data;
   for (i = 0; i < num; i++)
     {
	if (NULL == (names[i] = SLang_create_slstring (table->name)))
	  {
	     SLang_free_array (at);
	     return;
	  }
	table++;
     }
   (void) SLang_push_array (at, 1);
}

Syntax_Table_Map_Type Syntax_Table_Map [] =
{
   { "asis", ONIG_SYNTAX_ASIS },
   { "posix_basic", ONIG_SYNTAX_POSIX_BASIC },
   { "posix_extended", ONIG_SYNTAX_POSIX_EXTENDED },
   { "emacs", ONIG_SYNTAX_EMACS },
   { "grep", ONIG_SYNTAX_GREP },
   { "gnu_regex", ONIG_SYNTAX_GNU_REGEX },
   { "java", ONIG_SYNTAX_JAVA },
   { "perl", ONIG_SYNTAX_PERL },
   { "perl_ng", ONIG_SYNTAX_PERL_NG },
   { "ruby", ONIG_SYNTAX_RUBY },
   { NULL, NULL}
};

static OnigSyntaxType *pop_onig_syntax (void)
{
   return (OnigSyntaxType *) pop_onig_name_ptr ((Name_Map_Type *)Syntax_Table_Map, "syntax");
}

static Encoding_Table_Map_Type Encoding_Table_Map [] =
{
#ifdef ONIG_ENCODING_ASCII
   {"ascii", ONIG_ENCODING_ASCII},
#endif
#ifdef ONIG_ENCODING_ISO_8859_1
   {"iso_8859_1", ONIG_ENCODING_ISO_8859_1},
#endif
#ifdef ONIG_ENCODING_ISO_8859_2
   {"iso_8859_2", ONIG_ENCODING_ISO_8859_2},
#endif
#ifdef ONIG_ENCODING_ISO_8859_3
   {"iso_8859_3", ONIG_ENCODING_ISO_8859_3},
#endif
#ifdef ONIG_ENCODING_ISO_8859_4
   {"iso_8859_4", ONIG_ENCODING_ISO_8859_4},
#endif
#ifdef ONIG_ENCODING_ISO_8859_5
   {"iso_8859_5", ONIG_ENCODING_ISO_8859_5},
#endif
#ifdef ONIG_ENCODING_ISO_8859_6
   {"iso_8859_6", ONIG_ENCODING_ISO_8859_6},
#endif
#ifdef ONIG_ENCODING_ISO_8859_7
   {"iso_8859_7", ONIG_ENCODING_ISO_8859_7},
#endif
#ifdef ONIG_ENCODING_ISO_8859_8
   {"iso_8859_8", ONIG_ENCODING_ISO_8859_8},
#endif
#ifdef ONIG_ENCODING_ISO_8859_9
   {"iso_8859_9", ONIG_ENCODING_ISO_8859_9},
#endif
#ifdef ONIG_ENCODING_ISO_8859_10
   {"iso_8859_10", ONIG_ENCODING_ISO_8859_10},
#endif
#ifdef ONIG_ENCODING_ISO_8859_11
   {"iso_8859_11", ONIG_ENCODING_ISO_8859_11},
#endif
#ifdef ONIG_ENCODING_ISO_8859_13
   {"iso_8859_13", ONIG_ENCODING_ISO_8859_13},
#endif
#ifdef ONIG_ENCODING_ISO_8859_14
   {"iso_8859_14", ONIG_ENCODING_ISO_8859_14},
#endif
#ifdef ONIG_ENCODING_ISO_8859_15
   {"iso_8859_15", ONIG_ENCODING_ISO_8859_15},
#endif
#ifdef ONIG_ENCODING_ISO_8859_16
   {"iso_8859_16", ONIG_ENCODING_ISO_8859_16},
#endif
#ifdef ONIG_ENCODING_UTF8
   {"utf8", ONIG_ENCODING_UTF8},
#endif
#ifdef ONIG_ENCODING_UTF16_BE
   {"utf16_be", ONIG_ENCODING_UTF16_BE},
#endif
#ifdef ONIG_ENCODING_UTF16_LE
   {"utf16_le", ONIG_ENCODING_UTF16_LE},
#endif
#ifdef ONIG_ENCODING_UTF32_BE
   {"utf32_be", ONIG_ENCODING_UTF32_BE},
#endif
#ifdef ONIG_ENCODING_UTF32_LE
   {"utf32_le", ONIG_ENCODING_UTF32_LE},
#endif
#ifdef ONIG_ENCODING_EUC_JP
   {"euc_jp", ONIG_ENCODING_EUC_JP},
#endif
#ifdef ONIG_ENCODING_EUC_TW
   {"euc_tw", ONIG_ENCODING_EUC_TW},
#endif
#ifdef ONIG_ENCODING_EUC_KR
   {"euc_kr", ONIG_ENCODING_EUC_KR},
#endif
#ifdef ONIG_ENCODING_EUC_CN
   {"euc_cn", ONIG_ENCODING_EUC_CN},
#endif
#ifdef ONIG_ENCODING_SJIS
   {"sjis", ONIG_ENCODING_SJIS},
#endif
#ifdef ONIG_ENCODING_KOI8
   /* {"koi8", ONIG_ENCODING_KOI8}, */
#endif
#ifdef ONIG_ENCODING_KOI8_R
   {"koi8_r", ONIG_ENCODING_KOI8_R},
#endif
#ifdef ONIG_ENCODING_CP1251
   {"cp1251", ONIG_ENCODING_CP1251},
#endif
#ifdef ONIG_ENCODING_BIG5
   {"big5", ONIG_ENCODING_BIG5},
#endif
#ifdef ONIG_ENCODING_GB18030
   {"gb18030", ONIG_ENCODING_GB18030},
#endif
   {NULL, NULL}
};

static OnigEncodingType *pop_onig_encoding (void)
{
   return (OnigEncodingType *) pop_onig_name_ptr ((Name_Map_Type *)Encoding_Table_Map, "encoding");
}

typedef struct
{
   regex_t *re;
   OnigRegion *region;
   int match_pos;		       /* >= 0 if ok */
}
Onig_Type;

static void free_onig_type (Onig_Type *o)
{
   if (o == NULL)
     return;
   if (o->region != NULL)
     onig_region_free (o->region, 1);
   if (o->re != NULL)
     onig_free (o->re);
   SLfree ((char *) o);
}

static int push_onig_type (Onig_Type *o)
{
   SLang_MMT_Type *mmt;

   if (NULL == (mmt = SLang_create_mmt (Onig_Type_Id, (VOID_STAR) o)))
     {
	free_onig_type (o);
	return -1;
     }
   if (-1 == SLang_push_mmt (mmt))
     {
	SLang_free_mmt (mmt);
	return -1;
     }
   return 0;
}

static int pop_onig_option (OnigOptionType *optp)
{
   int iopt;

   if (-1 == SLang_pop_int (&iopt))
     return -1;
   *optp = (OnigOptionType) iopt;
   return 0;
}

/* Usage: reg = onig_new (pattern, options, enc, syntax) */
static void do_onig_new (void)
{
   const UChar* pattern;
   const UChar* pattern_end;
   OnigOptionType option;
   OnigEncoding enc;
   OnigSyntaxType *syntax;
   OnigErrorInfo err_info;
   Onig_Type *o;
   int status;

   syntax = ONIG_SYNTAX_PERL;
   if (SLinterp_is_utf8_mode ())
     enc = ONIG_ENCODING_UTF8;
   else
     enc = ONIG_ENCODING_ISO_8859_1;
   option = ONIG_OPTION_DEFAULT;

   switch (SLang_Num_Function_Args)
     {
      default:
	SLang_verror (SL_Usage_Error, "Usage: r = onig_new (pattern [,options [,encoding [,syntax]]])");
	return;

      case 4:
	if (NULL == (syntax = pop_onig_syntax ()))
	  return;
      case 3:
	if (NULL == (enc = pop_onig_encoding ()))
	  return;
      case 2:
	if (-1 == pop_onig_option (&option))
	  return;
      case 1:
	if (-1 == SLang_pop_slstring ((char **)&pattern))
	  return;
     }

   if (NULL == (o = (Onig_Type *) SLcalloc (1, sizeof (Onig_Type))))
     {
	SLfree ((char *)pattern);
	return;
     }

   pattern_end = pattern + strlen ((char *)pattern);

   status = onig_new (&o->re, pattern, pattern_end,
		      option, enc, syntax, &err_info);

   if (status != ONIG_NORMAL)
     {
	throw_onig_error (status, &err_info);
	SLang_free_slstring ((char *)pattern);
	free_onig_type (o);
	return;
     }

   if (NULL == (o->region = onig_region_new ()))
     {
	SLang_verror (slOnig_Error, "failed to allocate a region");
	SLang_free_slstring ((char *)pattern);
	free_onig_type (o);
	return;
     }

   SLang_free_slstring ((char *)pattern);
   (void) push_onig_type (o);	       /* frees it */
}

static int do_onig_search_internal (Onig_Type *o, OnigOptionType option, UChar *str, UChar *str_end, int start_pos, int end_pos)
{
   UChar *start, *range;
   int status;

   onig_region_clear (o->region);

   start = str + start_pos;
   range = str + end_pos;
   /* fwd search: (start <= search string < range)
    * bkw search: (range <= search string <= start)
    */
   if ((start < str) || (start > str_end)
       || (range < str) || (range > str_end))
     {
	SLang_verror (SL_InvalidParm_Error, "Invalid string offsets");
	return -1;
     }
   status = onig_search (o->re, str, str_end, start, range, o->region, option);

   if (status >= 0)
     return status;

   if (status == ONIG_MISMATCH)
     return -1;

   throw_onig_error (status, NULL);
   return -2;
}

/* Usage: onig_search (o, str [start, end] [,option]) */
static int do_onig_search (void)
{
   int start_pos = 0, end_pos = -1;
   char *str, *str_end;
   SLang_BString_Type *bstr = NULL;
   Onig_Type *o;
   SLang_MMT_Type *mmt;
   int status = -1;
   OnigOptionType option = ONIG_OPTION_NONE;

   switch (SLang_Num_Function_Args)
     {
      default:
	SLang_verror (SL_Usage_Error, "Usage: n = onig_search (compiled_pattern, str [,start_ofs, end_ofs] [,option])");
	return -1;

      case 5:
	if (-1 == pop_onig_option (&option))
	  return -1;
	/* drop */
      case 4:
	if (-1 == SLang_pop_int (&end_pos))
	  return -1;
	if (-1 == SLang_pop_int (&start_pos))
	  return -1;
	break;
      case 3:
	if (-1 == pop_onig_option (&option))
	  return -1;
	break;
      case 2:
	 break;
     }

   switch(SLang_peek_at_stack())
     {
      case SLANG_STRING_TYPE:
	if (-1 == SLang_pop_slstring (&str))
	  return -1;
	str_end = str + strlen (str);
	break;

      case SLANG_BSTRING_TYPE:
      default:
	  {
	     unsigned int len;

	     if (-1 == SLang_pop_bstring(&bstr))
	       return -1;

	     str = (char *)SLbstring_get_pointer(bstr, &len);
	     if (str == NULL)
	       {
		  SLbstring_free (bstr);
		  return -1;
	       }
	     str_end = str + len;
	  }
	break;
     }

   if (end_pos < 0)
     end_pos = (int) (str_end - str);

   if (NULL == (mmt = SLang_pop_mmt (Onig_Type_Id)))
     goto free_and_return;
   o = (Onig_Type *)SLang_object_from_mmt (mmt);

   status = do_onig_search_internal (o, option, (UChar *)str, (UChar *)str_end, start_pos, end_pos);
   if (status >= 0)
     {
	o->match_pos = status;
	status = o->region->num_regs;
	goto free_and_return;
     }
   o->match_pos = -1;

   if (status == -1)
     {				       /* no match */
	status = 0;
	goto free_and_return;
     }

   /* Else an error occurred */
   /* drop */

free_and_return:

   SLang_free_mmt (mmt);
   if (bstr != NULL)
     SLbstring_free (bstr);
   else
     SLang_free_slstring (str);

   return status;
}

static int get_nth_start_stop (Onig_Type *o, unsigned int n,
			       unsigned int *a, unsigned int *b)
{
   if (o->match_pos < 0)
     {
	SLang_verror (SL_InvalidParm_Error, "The last match was unsuccessful");
	return -1;
     }
   if (n >= (unsigned int) o->region->num_regs)
     return -1;

   *a = (unsigned int) o->region->beg[n];
   *b = (unsigned int) o->region->end[n];
   return 0;
}

static void nth_match (Onig_Type *o, int *np)
{
   unsigned int start, stop;
   SLang_Array_Type *at;
   SLindex_Type two = 2;
   int *data;

   if (-1 == get_nth_start_stop (o, (unsigned int) *np, &start, &stop))
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

static void nth_substr (Onig_Type *o, char *str, int *np)
{
   unsigned int start, stop;
   unsigned int len;

   len = strlen (str);

   if ((-1 == get_nth_start_stop (o, (unsigned int) *np, &start, &stop))
       || (start > len) || (stop > len))
     {
	SLang_push_null ();
	return;
     }

   str = SLang_create_nslstring (str + start, stop - start);
   (void) SLang_push_string (str);
   SLang_free_slstring (str);
}

static SLang_Name_Type *Warn_Func;
static SLang_Name_Type *Verb_Warn_Func;

static void warn_func (const char *msg)
{
   if (Warn_Func != NULL)
     {
	if (0 == SLang_push_string ((char *) msg))
	  SLexecute_function (Warn_Func);
     }
}

static void verb_warn_func (const char *msg)
{
   if (Verb_Warn_Func != NULL)
     {
	if (0 == SLang_push_string ((char *) msg))
	  SLexecute_function (Verb_Warn_Func);
     }
}

static void set_warn_func (void)
{
   SLang_Name_Type *sf;

   if (NULL == (sf = SLang_pop_function ()))
     return;

   if (Warn_Func != NULL)
     SLang_free_function (Warn_Func);

   Warn_Func = sf;
}

static void set_verb_warn_func (void)
{
   SLang_Name_Type *sf;

   if (NULL == (sf = SLang_pop_function ()))
     return;

   if (Verb_Warn_Func != NULL)
     SLang_free_function (Verb_Warn_Func);

   Verb_Warn_Func = sf;
}

static char *do_onig_version (void)
{
   return (char *)onig_version ();
}

static void get_encodings (void)
{
   get_onig_names ((Name_Map_Type *)Encoding_Table_Map);
}

static void get_syntaxes (void)
{
   get_onig_names ((Name_Map_Type *) Syntax_Table_Map);
}

static SLang_IConstant_Type Onig_Consts [] =
{
   MAKE_ICONSTANT ("ONIG_OPTION_DEFAULT", ONIG_OPTION_DEFAULT),
   MAKE_ICONSTANT ("ONIG_OPTION_NONE", ONIG_OPTION_NONE),
   MAKE_ICONSTANT ("ONIG_OPTION_SINGLELINE", ONIG_OPTION_SINGLELINE),
   MAKE_ICONSTANT ("ONIG_OPTION_MULTILINE", ONIG_OPTION_MULTILINE),
   MAKE_ICONSTANT ("ONIG_OPTION_IGNORECASE", ONIG_OPTION_IGNORECASE),
   MAKE_ICONSTANT ("ONIG_OPTION_EXTEND", ONIG_OPTION_EXTEND),
   MAKE_ICONSTANT ("ONIG_OPTION_FIND_LONGEST", ONIG_OPTION_FIND_LONGEST),
   MAKE_ICONSTANT ("ONIG_OPTION_FIND_NOT_EMPTY", ONIG_OPTION_FIND_NOT_EMPTY),
   MAKE_ICONSTANT ("ONIG_OPTION_NEGATE_SINGLELINE", ONIG_OPTION_NEGATE_SINGLELINE),
   MAKE_ICONSTANT ("ONIG_OPTION_DONT_CAPTURE_GROUP", ONIG_OPTION_DONT_CAPTURE_GROUP),
   MAKE_ICONSTANT ("ONIG_OPTION_CAPTURE_GROUP", ONIG_OPTION_CAPTURE_GROUP),

   SLANG_END_ICONST_TABLE
};

#define DUMMY_ONIG_TYPE 0
#define O DUMMY_ONIG_TYPE
#define I SLANG_INT_TYPE
#define V SLANG_VOID_TYPE
#define S SLANG_STRING_TYPE
static SLang_Intrin_Fun_Type Onig_Intrinsics [] =
{
   MAKE_INTRINSIC_0("onig_version", do_onig_version, S),
   MAKE_INTRINSIC_0("onig_new", do_onig_new, V),
   MAKE_INTRINSIC_0("onig_search", do_onig_search, I),
   MAKE_INTRINSIC_2("onig_nth_match", nth_match, V, O, I),
   MAKE_INTRINSIC_3("onig_nth_substr", nth_substr, V, O, S, I),
   MAKE_INTRINSIC_0("onig_set_warn_func", set_warn_func, V),
   MAKE_INTRINSIC_0("onig_set_verb_warn_func", set_verb_warn_func, V),
   MAKE_INTRINSIC_0("onig_get_encodings", get_encodings, V),
   MAKE_INTRINSIC_0("onig_get_syntaxes", get_syntaxes, V),
   /* MAKE_INTRINSIC_1("slang_to_pcre", slang_to_pcre, V, S), */
   SLANG_END_INTRIN_FUN_TABLE
};
#undef V
#undef S
#undef I
#undef O

static void destroy_onig (SLtype type, VOID_STAR f)
{
   (void) type;

   free_onig_type ((Onig_Type *)f);
}

static int register_onig_type (void)
{
   SLang_Class_Type *cl;

   if (Onig_Type_Id != 0)
     return 0;

   if (NULL == (cl = SLclass_allocate_class ("Onig_Type")))
     return -1;

   if (-1 == SLclass_set_destroy_function (cl, destroy_onig))
     return -1;

   /* By registering as SLANG_VOID_TYPE, slang will dynamically allocate a
    * type.
    */
   if (-1 == SLclass_register_class (cl, SLANG_VOID_TYPE, sizeof (Onig_Type), SLANG_CLASS_TYPE_MMT))
     return -1;

   Onig_Type_Id = SLclass_get_class_id (cl);

   if (-1 == SLclass_patch_intrin_fun_table1 (Onig_Intrinsics, DUMMY_ONIG_TYPE, Onig_Type_Id))
     return -1;

   return 0;
}

static int setup_onig (void)
{
   static int inited = 0;

   if (inited)
     return 0;

   if ((-1 == slOnig_Error)
       && (-1 == (slOnig_Error = SLerr_new_exception (SL_RunTime_Error, "OnigError", "Onig Error"))))
     return -1;

   if (-1 == onig_init ())
     {
	SLang_verror (slOnig_Error, "onig_init failed");
	return -1;
     }

   onig_set_warn_func (&warn_func);
   onig_set_verb_warn_func (&verb_warn_func);
   onig_set_default_syntax (ONIG_SYNTAX_PERL);

   inited = 1;

   return 0;
}

int init_onig_module_ns (char *ns_name)
{
   SLang_NameSpace_Type *ns = SLns_create_namespace (ns_name);
   if (ns == NULL)
     return -1;

   if (-1 == setup_onig ())
     return -1;
   if (-1 == register_onig_type ())
     return -1;

   if ((-1 == SLns_add_intrin_fun_table (ns, Onig_Intrinsics, "__ONIG__"))
       || (-1 == SLns_add_iconstant_table (ns, Onig_Consts, NULL)))
     return -1;

   return 0;
}

/* This function is optional */
void deinit_onig_module (void)
{
   (void) onig_end ();
}

