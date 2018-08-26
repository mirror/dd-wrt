/* -*- mode: C; mode: fold; -*- */
/* Standard intrinsic functions for S-Lang.  Included here are string
   and array operations */
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
# include <math.h>
#endif

#include "slang.h"
#include "_slang.h"

/*}}}*/

/* builtin stack manipulation functions */
int SLdo_pop(void) /*{{{*/
{
   return SLdo_pop_n (1);
}

/*}}}*/

int SLdo_pop_n (unsigned int n)
{
   SLang_Object_Type x;

   while (n--)
     {
	if (SLang_pop(&x)) return -1;
	SLang_free_object (&x);
     }

   return 0;
}

static void do_dup(void) /*{{{*/
{
   (void) SLdup_n (1);
}

/*}}}*/

static void length_cmd (void)
{
   SLang_Class_Type *cl;
   SLang_Object_Type obj;
   VOID_STAR p;
   SLuindex_Type length;
   SLindex_Type ilen;

   if (-1 == SLang_pop (&obj))
     return;

   cl = _pSLclass_get_class (obj.o_data_type);
   p = _pSLclass_get_ptr_to_value (cl, &obj);

   ilen = 1;
   if (cl->cl_length != NULL)
     {
	if (0 == (*cl->cl_length)(obj.o_data_type, p, &length))
	  ilen = (SLindex_Type) length;
	else
	  ilen = -1;
     }

   SLang_free_object (&obj);
   (void) SLang_push_array_index (ilen);
}

/* convert integer to a string of length 1 */
static void char_cmd (SLwchar_Type *x) /*{{{*/
{
   SLuchar_Type buf[SLUTF8_MAX_MBLEN + 1];
   int is_byte;

   is_byte = ((signed)*x < 0);
   if (is_byte)
     {
	buf[0] = (SLuchar_Type) (-(signed)*x);
	buf[1] = 0;
     }
   else if ((_pSLinterp_UTF8_Mode == 0)
	    || (*x < 0x80))
     {
        buf[0] = (SLuchar_Type) *x;
        buf[1] = 0;
     }
   else
     {
        SLuchar_Type *p;

        p = SLutf8_encode (*x, buf, SLUTF8_MAX_MBLEN);
        if (p == NULL) p = buf;

        *p = 0;
     }

   SLang_push_string ((char *)buf);
}

/*}}}*/

/* format object into a string and returns slstring */
char *_pSLstringize_object (SLang_Object_Type *obj) /*{{{*/
{
   SLang_Class_Type *cl;
   SLtype stype;
   VOID_STAR p;
   char *s, *s1;

   stype = obj->o_data_type;
   p = (VOID_STAR) &obj->v.ptr_val;

   cl = _pSLclass_get_class (stype);

   s = (*cl->cl_string) (stype, p);
   if (s != NULL)
     {
	s1 = SLang_create_slstring (s);
	SLfree (s);
	s = s1;
     }
   return s;
}
/*}}}*/

int SLang_run_hooks (SLFUTURE_CONST char *hook, unsigned int num_args, ...)
{
   unsigned int i;
   va_list ap;

   if (SLang_get_error ())
     return -1;

   if (0 == SLang_is_defined (hook))
     return 0;

   (void) SLang_start_arg_list ();
   va_start (ap, num_args);
   for (i = 0; i < num_args; i++)
     {
	char *arg;

	arg = va_arg (ap, char *);
	if (-1 == SLang_push_string (arg))
	  break;
     }
   va_end (ap);
   (void) SLang_end_arg_list ();

   if (_pSLang_Error) return -1;
   return SLang_execute_function (hook);
}

static void intrin_getenv_cmd (char *s)
{
   SLang_push_string (getenv (s));
}

#ifdef HAVE_PUTENV
/* This is a silly hack to deal with the ambiguity of whether or not to free
 * a pointer passed to putenv.  Here it is attached to an array so that leak
 * checkers can located it.
 */
# define MAX_PUTENV_ARRAY_SIZE 64
static char *Putenv_Pointer_Array[MAX_PUTENV_ARRAY_SIZE];
unsigned int Num_Putenv_Pointers = 0;
static void intrin_putenv (void) /*{{{*/
{
   char *s;

   /* Some putenv implementations require malloced strings. */
   if (SLpop_string(&s)) return;

   if (putenv (s))
     {
	SLang_set_error (SL_OS_Error);
	SLfree (s);
     }
   /* Note that s is NOT freed */

   if (Num_Putenv_Pointers < MAX_PUTENV_ARRAY_SIZE)
     Putenv_Pointer_Array[Num_Putenv_Pointers++] = s;
}

/*}}}*/

#endif

static void byte_compile_file (char *f, int *m)
{
   SLang_byte_compile_file (f, *m);
}

static void intrin_type_info1 (void)
{
   SLang_Object_Type obj;
   unsigned int type;

   if (-1 == SLang_pop (&obj))
     return;

   type = obj.o_data_type;
   if (type == SLANG_ARRAY_TYPE)
     type = obj.v.array_val->data_type;

   SLang_free_object (&obj);

   SLang_push_datatype (type);
}

static void intrin_type_info (void)
{
   SLang_Object_Type obj;

   if (-1 == SLang_pop (&obj))
     return;

   SLang_push_datatype (obj.o_data_type);
   SLang_free_object (&obj);
}

void _pSLstring_intrinsic (void) /*{{{*/
{
   SLang_Object_Type x;
   char *s;

   if (SLang_pop (&x)) return;
   if (NULL != (s = _pSLstringize_object (&x)))
     _pSLang_push_slstring (s);

   SLang_free_object (&x);
}

/*}}}*/

static void intrin_typecast (void)
{
   SLtype to_type;
   if (0 == SLang_pop_datatype (&to_type))
     (void) SLclass_typecast (to_type, 0, 1);
}

#if SLANG_HAS_FLOAT
static void intrin_double (void)
{
   (void) SLclass_typecast (SLANG_DOUBLE_TYPE, 0, 1);
}

#endif

static void intrin_int (void) /*{{{*/
{
   (void) SLclass_typecast (SLANG_INT_TYPE, 0, 1);
}

/*}}}*/

static SLCONST char *
intrin_function_name (void)
{
   SLCONST char *name;
   if (NULL == (name = _pSLang_current_function_name ()))
     return "";
   return name;
}

static void intrin_message (char *s)
{
   SLang_vmessage ("%s", s);
}

static void intrin_error (char *s)
{
   _pSLang_verror (SL_RunTime_Error, "%s", s);
}

static void intrin_pop_n (int *n)
{
   SLdo_pop_n ((unsigned int) *n);
}

static void intrin_reverse_stack (int *n)
{
   SLreverse_stack (*n);
}

static void intrin_roll_stack (int *n)
{
   SLroll_stack (*n);
}

static void usage (void)
{
   char *msg;

   _pSLstrops_do_sprintf_n (SLang_Num_Function_Args - 1);   /* do not include format */

   if (-1 == SLang_pop_slstring (&msg))
     return;

   _pSLang_verror (SL_USAGE_ERROR, "Usage: %s", msg);
   SLang_free_slstring (msg);
}

static void guess_type (char *s)
{
   SLang_push_datatype (SLang_guess_type(s));
}

static int load_string_or_file (int (*f) (SLFUTURE_CONST char *, SLFUTURE_CONST char *))
{
   char *file;
   char *ns = NULL;
   int status;

   if (SLang_Num_Function_Args == 2)
     {
	if (-1 == SLang_pop_slstring (&ns))
	  return -1;
     }

   if (-1 == SLang_pop_slstring (&file))
     {
	SLang_free_slstring (ns);
	return -1;
     }

   status = (*f) (file, ns);
   SLang_free_slstring (file);
   SLang_free_slstring (ns);
   return status;
}

static int load_file (void)
{
   return (0 == load_string_or_file (SLns_load_file));
}

static void load_string (void)
{
   /* FIXME: This should use the namespace of the currently executing code */
   (void) load_string_or_file (SLns_load_string);
}

static int get_doc_string (char *file, char *topic)
{
   FILE *fp;
   char line[1024];
   unsigned int topic_len, str_len;
   char *str;
   char ch;

   topic_len = strlen (topic);
   if (topic_len == 0)
     return -1;

   if (NULL == (fp = fopen (file, "r")))
     return -1;

   while (1)
     {
	char *pos;

	if (NULL == fgets (line, sizeof(line), fp))
	  {
	     fclose (fp);
	     return -1;
	  }
	ch = *line;
	if ((ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '-'))
	  continue;

	pos = strstr (line, topic);
	if (pos == NULL)
	  continue;

	ch = pos[topic_len];

	/* Most common case */
	if ((pos == line)
	    && ((ch == '\n') || (ch == 0) || (ch == ' ') || (ch == '\t') || (ch == ',')))
	  break;

	pos = line;
	while (NULL != (pos = strchr (pos, ',')))
	  {
	     /* Here *pos == ',' */
	     if (NULL == (pos = strstr (pos+1, topic)))
	       break;
	     ch = pos[-1];
	     if ((ch != ' ') && (ch != ',') && (ch != '\t'))
	       {
		  pos += topic_len;
		  continue;
	       }
	     ch = pos[topic_len];
	     if ((ch == '\n') || (ch == ',')
		 || (ch == ' ') || (ch == '\t') || (ch == 0))
	       break;
	  }
	if (pos != NULL)
	  break;
     }

   if (NULL == (str = SLmake_string (line)))
     {
	fclose (fp);
	return -1;
     }
   str_len = strlen (str);

   while (NULL != fgets (line, sizeof (line), fp))
     {
	unsigned int len;
	char *new_str;

	ch = *line;
	if (ch == '#') continue;
	if (ch == '-') break;

	len = strlen (line);
	if (NULL == (new_str = SLrealloc (str, str_len + len + 1)))
	  {
	     SLfree (str);
	     str = NULL;
	     break;
	  }
	str = new_str;
	strcpy (str + str_len, line);
	str_len += len;
     }

   fclose (fp);

   (void) SLang_push_malloced_string (str);
   return 0;
}

static _pSLString_List_Type *Doc_Files;

static int add_doc_file (char *file)
{
   if (Doc_Files == NULL)
     {
	Doc_Files = _pSLstring_list_new (5, 5);
	if (Doc_Files == NULL)
	  return -1;
     }

   if ((file == NULL) || (*file == 0))
     return 0;

   return _pSLstring_list_append_copy (Doc_Files, file);
}

static void add_doc_file_intrin (char *file)
{
   (void) add_doc_file (file);
}

static void get_doc_files_intrin (void)
{
   if (Doc_Files == NULL)
     Doc_Files = _pSLstring_list_new (5, 5);
   (void) _pSLstring_list_push (Doc_Files, 0);
}

static void set_doc_files_intrin (void)
{
   SLang_Array_Type *at;
   unsigned int i, num;
   char **data;

   if (-1 == SLang_pop_array_of_type (&at, SLANG_STRING_TYPE))
     return;

   _pSLstring_list_delete (Doc_Files);
   Doc_Files = NULL;

   num = at->num_elements;
   data = (char **) at->data;
   for (i = 0; i < num; i++)
     {
	if (-1 == add_doc_file (data[i]))
	  break;
     }
   SLang_free_array (at);
}

static void get_doc_string_intrin (char *topic)
{
   char *file;
   char **files;
   unsigned int i, num_files;

   if (SLang_Num_Function_Args == 2)
     {
	if (-1 == SLang_pop_slstring (&file))
	  return;

	if (-1 == get_doc_string (file, topic))
	  (void) SLang_push_null ();

	SLang_free_slstring (file);
	return;
     }

   if ((Doc_Files == NULL)
       || (NULL == (files = Doc_Files->buf)))
     {
	SLang_push_null ();
	return;
     }
   num_files = Doc_Files->num;
   for (i = 0; i < num_files; i++)
     {
	file = files[i];
	if (file == NULL)
	  continue;

	if (0 == get_doc_string (file, topic))
	  return;
     }
   (void) SLang_push_null ();
}

static int push_string_array_elements (SLang_Array_Type *at)
{
   char **strs;
   unsigned int num;
   unsigned int i;

   if (at == NULL)
     return -1;

   strs = (char **)at->data;
   num = at->num_elements;
   for (i = 0; i < num; i++)
     {
	if (-1 == SLang_push_string (strs[i]))
	  {
	     SLdo_pop_n (i);
	     return -1;
	  }
     }
   SLang_push_integer ((int) num);
   return 0;
}

static void intrin_apropos (void)
{
   int num_args;
   char *pat;
   char *namespace_name;
   unsigned int flags;
   SLang_Array_Type *at;

   num_args = SLang_Num_Function_Args;

   if (-1 == SLang_pop_uinteger (&flags))
     return;
   if (-1 == SLang_pop_slstring (&pat))
     return;

   namespace_name = NULL;
   at = NULL;
   if (num_args == 3)
     {
	if (-1 == SLang_pop_slstring (&namespace_name))
	  goto free_and_return;
     }

   at = _pSLang_apropos (namespace_name, pat, flags);
   if (num_args == 3)
     {
	(void) SLang_push_array (at, 0);
	goto free_and_return;
     }

   /* Maintain compatibility with old version of the function.  That version
    * did not take three arguments and returned everything to the stack.
    * Yuk.
    */
   (void) push_string_array_elements (at);

   free_and_return:
   /* NULLs ok */
   SLang_free_slstring (namespace_name);
   SLang_free_slstring (pat);
   SLang_free_array (at);
}

static int intrin_get_defines (void)
{
   int n = 0;
   SLFUTURE_CONST char **s = _pSLdefines;

   while (*s != NULL)
     {
	if (-1 == SLang_push_string (*s))
	  {
	     SLdo_pop_n ((unsigned int) n);
	     return -1;
	  }
	s++;
	n++;
     }
   return n;
}

static void intrin_get_reference (char *name)
{
   if (*name == '&') name++;
   _pSLang_push_nt_as_ref (_pSLlocate_name (name));
}

static void intrin_get_namespaces (void)
{
   SLang_push_array (_pSLns_list_namespaces (), 1);
}

#ifdef HAVE_SYS_UTSNAME_H
# include <sys/utsname.h>
#endif

static void uname_cmd (void)
{
#ifdef HAVE_UNAME
   struct utsname u;
   SLFUTURE_CONST char *field_names [6];
   SLtype field_types[6];
   VOID_STAR field_values [6];
   char *ptrs[6];
   int i;

   if (-1 == uname (&u))
     (void) SLang_push_null ();

   field_names[0] = "sysname"; ptrs[0] = u.sysname;
   field_names[1] = "nodename"; ptrs[1] = u.nodename;
   field_names[2] = "release"; ptrs[2] = u.release;
   field_names[3] = "version"; ptrs[3] = u.version;
   field_names[4] = "machine"; ptrs[4] = u.machine;

   for (i = 0; i < 5; i++)
     {
	field_types[i] = SLANG_STRING_TYPE;
	field_values[i] = (VOID_STAR) &ptrs[i];
     }

   if (0 == SLstruct_create_struct (5, field_names, field_types, field_values))
     return;
#endif

   SLang_push_null ();
}

static void uninitialize_ref_intrin (SLang_Ref_Type *ref)
{
   (void) _pSLang_uninitialize_ref (ref);
}

static int class_type_intrinsic (void)
{
   SLtype type;

   if (-1 == SLang_pop_datatype (&type))
     return -1;
   return _pSLclass_get_class (type)->cl_class_type;
}

static int class_id_intrinsic (void)
{
   SLtype type;

   if (-1 == SLang_pop_datatype (&type))
     return -1;
   return _pSLclass_get_class (type)->cl_data_type;
}

static void datatype_intrinsic (SLtype *t)
{
   SLang_Class_Type *cl;

   if (0 == SLclass_is_class_defined (*t))
     {
	(void) SLang_push_null ();
	return;
     }

   cl = _pSLclass_get_class (*t);
   (void) SLang_push_datatype (cl->cl_data_type);
}

static int do_obj_cmp_fun (int (*fun)(SLang_Object_Type *, SLang_Object_Type *))
{
   int eqs;
   SLang_Object_Type a, b;

   if (-1 == SLang_pop (&b))
     return -1;

   if (-1 == SLang_pop (&a))
     {
	SLang_free_object (&b);
	return -1;
     }

   eqs = (*fun) (&a, &b);

   SLang_free_object (&a);
   SLang_free_object (&b);
   return eqs;
}

static int is_same_intrinsic (void)
{
   return do_obj_cmp_fun (_pSLclass_is_same_obj);
}

static int eqs_intrinsic (void)
{
   return do_obj_cmp_fun (_pSLclass_obj_eqs);
}

static int is_callable_intrinsic (void)
{
   SLang_Ref_Type *ref;
   int ret;

   if (SLang_peek_at_stack () != SLANG_REF_TYPE)
     {
	(void) SLdo_pop ();
	return 0;
     }

   if (-1 == SLang_pop_ref (&ref))
     return -1;

   ret = _pSLang_ref_is_callable (ref);
   SLang_free_ref (ref);

   return ret;
}

static int is_numeric (SLtype type)
{
   /* Version 2: Add attributes to the class tables to simplify this.
    * Also clarify exactly what _pSLang_is_arith_type is supposed to return.
    */
   if (0 == _pSLang_is_arith_type ((SLtype) type))
     {
	if (type == SLANG_COMPLEX_TYPE)
	  return 3;

	return 0;
     }
   if ((type == SLANG_DOUBLE_TYPE) || (type == SLANG_FLOAT_TYPE))
     return 2;

   return 1;
}

static int is_numeric_intrinsic (void)
{
   int type;

   if (-1 == (type = SLang_peek_at_stack1 ()))
     return -1;

   (void) SLdo_pop ();
   return is_numeric ((SLtype) type);
}

static int is_datatype_numeric_intrinsic (void)
{
   SLtype type;

   if (-1 == SLang_pop_datatype (&type))
     return -1;

   return is_numeric (type);
}

static void lang_print_stack (void)
{
   (void) _pSLang_dump_stack ();
}

static int pop_array_or_string (SLtype itype, char **sp,
				SLang_Array_Type **atsp, SLang_Array_Type **atip)
{
   char *s;

   if (SLang_peek_at_stack () == SLANG_ARRAY_TYPE)
     {
	SLang_Array_Type *ats, *ati;

	*sp = NULL;
	if (-1 == SLang_pop_array_of_type (&ats, SLANG_STRING_TYPE))
	  {
	     *atsp = NULL;
	     *atip = NULL;
	     return -1;
	  }
	if (NULL == (ati = SLang_create_array1 (itype, 0, NULL, ats->dims, ats->num_dims, 1)))
	  {
	     *atsp = NULL;
	     *atip = NULL;
	     SLang_free_array (ats);
	     return -1;
	  }
	*atsp = ats;
	*atip = ati;
	return 0;
     }

   *atsp = NULL;
   *atip = NULL;
   if (-1 == SLang_pop_slstring (&s))
     {
	*sp = NULL;
	return -1;
     }
   *sp = s;
   return 0;
}

#if SLANG_HAS_FLOAT
static void intrin_atof (void)
{
   char *s;
   SLang_Array_Type *ats;
   SLang_Array_Type *ati;
   double *ip;
   char **strp, **strpmax;

   if (-1 == pop_array_or_string (SLANG_DOUBLE_TYPE, &s, &ats, &ati))
     return;

   if (s != NULL)
     {
	(void) SLang_push_double(_pSLang_atof(s));
	SLang_free_slstring (s);
	return;
     }

   strp = (char **) ats->data;
   strpmax = strp + ats->num_elements;
   ip = (double *) ati->data;

   while (strp < strpmax)
     {
	if (*strp == NULL)
	  *ip++ = _pSLang_NaN;
	else
	  *ip++ = _pSLang_atof (*strp);
	strp++;
     }
   SLang_free_array (ats);
   (void) SLang_push_array (ati, 1);
}
#endif

/* Convert string to integer */
static void intrin_integer (void)
{
   char *s;
   SLang_Array_Type *ats;
   SLang_Array_Type *ati;
   int *ip;
   unsigned char **strp, **strpmax;

   if (-1 == pop_array_or_string (SLANG_INT_TYPE, &s, &ats, &ati))
     return;

   if (s != NULL)
     {
	(void) SLang_push_integer (SLatoi ((unsigned char *) s));
	SLang_free_slstring (s);
	return;
     }

   strp = (unsigned char **) ats->data;
   strpmax = strp + ats->num_elements;
   ip = (int *) ati->data;

   while ((strp < strpmax) && (_pSLang_Error == 0))
     {
	if (*strp == NULL)
	  *ip++ = 0;
	else
	  *ip++ = SLatoi (*strp);
	strp++;
     }
   SLang_free_array (ats);
   (void) SLang_push_array (ati, 1);
}
/*}}}*/

static void atoi_intrin (void)
{
   char *s;
   SLang_Array_Type *ats;
   SLang_Array_Type *ati;
   int *ip;
   char **strp, **strpmax;

   if (-1 == pop_array_or_string (SLANG_INT_TYPE, &s, &ats, &ati))
     return;

   if (s != NULL)
     {
	(void) SLang_push_integer (atoi (s));
	SLang_free_slstring (s);
	return;
     }

   strp = (char **) ats->data;
   strpmax = strp + ats->num_elements;
   ip = (int *) ati->data;

   while (strp < strpmax)
     {
	if (*strp == NULL)
	  *ip++ = 0;
	else
	  *ip++ = atoi (*strp);
	strp++;
     }
   SLang_free_array (ats);
   (void) SLang_push_array (ati, 1);
}

static void atol_intrin (void)
{
   char *s;
   SLang_Array_Type *ats;
   SLang_Array_Type *ati;
   long *ip;
   char **strp, **strpmax;

   if (-1 == pop_array_or_string (_pSLANG_LONG_TYPE, &s, &ats, &ati))
     return;

   if (s != NULL)
     {
	(void) SLang_push_long (atol (s));
	SLang_free_slstring (s);
	return;
     }

   strp = (char **) ats->data;
   strpmax = strp + ats->num_elements;
   ip = (long *) ati->data;

   while (strp < strpmax)
     {
	if (*strp == NULL)
	  *ip++ = 0;
	else
	  *ip++ = atol (*strp);
	strp++;
     }
   SLang_free_array (ats);
   (void) SLang_push_array (ati, 1);
}

#ifdef HAVE_LONG_LONG
# ifdef HAVE_ATOLL
#  define ATOLL_FUN(s) atoll(s)
# else
#  ifdef HAVE_STRTOLL
#   define ATOLL_FUN(s) strtoll((s), NULL, 10)
#  else
#   define ATOLL_FUN(s) strtol((s), NULL, 10)
#  endif
# endif
static void atoll_intrin (void)
{
   char *s;
   SLang_Array_Type *ats;
   SLang_Array_Type *ati;
   long long *ip;
   char **strp, **strpmax;

   if (-1 == pop_array_or_string (_pSLANG_LLONG_TYPE, &s, &ats, &ati))
     return;

   if (s != NULL)
     {
	(void) SLang_push_long_long (ATOLL_FUN(s));
	SLang_free_slstring (s);
	return;
     }

   strp = (char **) ats->data;
   strpmax = strp + ats->num_elements;
   ip = (long long *) ati->data;

   while (strp < strpmax)
     {
	if (*strp == NULL)
	  *ip++ = 0;
	else
	  *ip++ = ATOLL_FUN (*strp);
	strp++;
     }
   SLang_free_array (ats);
   (void) SLang_push_array (ati, 1);
}
#endif

static void autoload_intrinsic (char *a, char *b)
{
   SLang_autoload (a, b);
}
static int is_defined_intrin (char *s)
{
   return SLang_is_defined (s);
}

static int system_intrinsic (char *s)
{
   return SLsystem (s);
}

static int system_intr_intrinsic (char *s)
{
   return SLsystem_intr (s);
}

static int stack_depth_intrin (void)
{
   return SLstack_depth ();
}

static void expand_dollar_string (char *s)
{
   (void) _pSLpush_dollar_string (s);
}

#if SLANG_HAS_QUALIFIERS
static void get_qualifiers_intrin (void)
{
   SLang_Struct_Type *q;
   if (0 == _pSLang_get_qualifiers_intrin (&q))
     (void) SLang_push_struct (q);
}

static int qualifier_exists_intrin (char *name)
{
   SLang_Struct_Type *q;

   if (-1 == _pSLang_get_qualifiers_intrin (&q))
     return -1;

   if ((q == NULL)
       || (NULL == _pSLstruct_get_field_value (q, name)))
     return 0;

   return 1;
}

static void qualifier_intrin (void)
{
   int has_default;
   char *name;
   SLang_Struct_Type *q;
   SLang_Object_Type *objp;

   if (-1 == _pSLang_get_qualifiers_intrin (&q))
     return;

   has_default = (SLang_Num_Function_Args == 2);
   if (has_default)
     {
	if (-1 == SLroll_stack (2))
	  return;
     }

   if (-1 == SLang_pop_slstring (&name))
     return;

   if (q != NULL)
     objp = _pSLstruct_get_field_value (q, name);
   else
     objp = NULL;

   SLang_free_slstring (name);

   if (objp != NULL)
     {
	if (has_default)
	  SLdo_pop ();
	_pSLpush_slang_obj (objp);
     }
   else if (has_default == 0)
     (void) SLang_push_null ();

   /* Note: objp and q should _not_ be freed since they were not allocated */
}
#endif

static void clear_error_intrin (void)
{
   (void) _pSLerr_clear_error (1);
}

static void set_argv_intrinsic (void);
static SLang_Intrin_Fun_Type SLang_Basic_Table [] = /*{{{*/
{
   MAKE_INTRINSIC_0("__is_callable", is_callable_intrinsic, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("__is_numeric", is_numeric_intrinsic, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("__is_datatype_numeric", is_datatype_numeric_intrinsic, SLANG_INT_TYPE),
   MAKE_INTRINSIC_1("__is_initialized", _pSLang_is_ref_initialized, SLANG_INT_TYPE, SLANG_REF_TYPE),
   MAKE_INTRINSIC_S("__get_reference", intrin_get_reference, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_1("__uninitialize", uninitialize_ref_intrin, SLANG_VOID_TYPE, SLANG_REF_TYPE),
   MAKE_INTRINSIC_0("__is_same", is_same_intrinsic, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("__class_type", class_type_intrinsic, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("__class_id", class_id_intrinsic, SLANG_INT_TYPE),
   MAKE_INTRINSIC_1("__datatype", datatype_intrinsic, SLANG_VOID_TYPE, SLANG_SLTYPE_INT_TYPE),
   MAKE_INTRINSIC_0("_eqs", eqs_intrinsic, SLANG_INT_TYPE),
   MAKE_INTRINSIC_S("get_doc_string_from_file",  get_doc_string_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("add_doc_file", add_doc_file_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("get_doc_files", get_doc_files_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("set_doc_files", set_doc_files_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SS("autoload",  autoload_intrinsic, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("is_defined",  is_defined_intrin, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("string",  _pSLstring_intrinsic, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("uname", uname_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("getenv",  intrin_getenv_cmd, SLANG_VOID_TYPE),
#ifdef HAVE_PUTENV
   MAKE_INTRINSIC_0("putenv",  intrin_putenv, SLANG_VOID_TYPE),
#endif
   MAKE_INTRINSIC_0("evalfile",  load_file, SLANG_INT_TYPE),
   MAKE_INTRINSIC_I("char",  char_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("eval",  load_string, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("dup",  do_dup, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("integer",  intrin_integer, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("system",  system_intrinsic, SLANG_INT_TYPE),
   MAKE_INTRINSIC_S("system_intr",  system_intr_intrinsic, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("_apropos",  intrin_apropos, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_get_namespaces", intrin_get_namespaces, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("_trace_function",  _pSLang_trace_fun, SLANG_VOID_TYPE),
#if SLANG_HAS_FLOAT
   MAKE_INTRINSIC_0("atof", intrin_atof, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("double", intrin_double, SLANG_VOID_TYPE),
#endif
   MAKE_INTRINSIC_0("atoi", atoi_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("atol", atol_intrin, SLANG_VOID_TYPE),
#ifdef HAVE_LONG_LONG
   MAKE_INTRINSIC_0("atoll", atoll_intrin, SLANG_VOID_TYPE),
#endif
   MAKE_INTRINSIC_0("int",  intrin_int, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("typecast", intrin_typecast, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_stkdepth", stack_depth_intrin, SLANG_INT_TYPE),
   MAKE_INTRINSIC_I("_stk_reverse", intrin_reverse_stack, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("typeof", intrin_type_info, VOID_TYPE),
   MAKE_INTRINSIC_0("_typeof", intrin_type_info1, VOID_TYPE),
   MAKE_INTRINSIC_I("_pop_n", intrin_pop_n, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_print_stack", lang_print_stack, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_I("_stk_roll", intrin_roll_stack, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SI("byte_compile_file", byte_compile_file, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_clear_error", clear_error_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_function_name", intrin_function_name, SLANG_STRING_TYPE),
#if SLANG_HAS_FLOAT
   MAKE_INTRINSIC_S("set_float_format", _pSLset_double_format, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("get_float_format", _pSLget_double_format, SLANG_STRING_TYPE),
#endif
   MAKE_INTRINSIC_S("_slang_guess_type", guess_type, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("error", intrin_error, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("message", intrin_message, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("__get_defined_symbols", intrin_get_defines, SLANG_INT_TYPE),
   MAKE_INTRINSIC_I("__pop_args", _pSLstruct_pop_args, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_1("__push_args", _pSLstruct_push_args, SLANG_VOID_TYPE, SLANG_ARRAY_TYPE),
   MAKE_INTRINSIC_0("usage", usage, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("implements", _pSLang_implements_intrinsic, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("use_namespace", _pSLang_use_namespace_intrinsic, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("current_namespace", _pSLang_cur_namespace_intrinsic, SLANG_STRING_TYPE),
   MAKE_INTRINSIC_0("length", length_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("__set_argc_argv", set_argv_intrinsic, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("_$", expand_dollar_string, SLANG_VOID_TYPE),
#if SLANG_HAS_QUALIFIERS
   MAKE_INTRINSIC_0("__qualifiers", get_qualifiers_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("qualifier", qualifier_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("qualifier_exists", qualifier_exists_intrin, SLANG_INT_TYPE),
#endif
   SLANG_END_INTRIN_FUN_TABLE
};

/*}}}*/

#ifdef SLANG_DOC_DIR
SLFUTURE_CONST char *SLang_Doc_Dir = SLANG_DOC_DIR;
#else
SLFUTURE_CONST char *SLang_Doc_Dir = "";
#endif

#ifdef SLANG_INSTALL_PREFIX
static SLCONST char *Install_Prefix = SLANG_INSTALL_PREFIX;
#else
static char *Install_Prefix = "";
#endif

static int obsolete_int_variable;
static SLang_Intrin_Var_Type Intrin_Vars[] =
{
   MAKE_VARIABLE("_debug_info", &obsolete_int_variable, SLANG_INT_TYPE, 0),
#if SLANG_HAS_BOSEOS
   MAKE_VARIABLE("_boseos_info", &_pSLang_Compile_BOSEOS, SLANG_INT_TYPE, 0),
   MAKE_VARIABLE("_bofeof_info", &_pSLang_Compile_BOFEOF, SLANG_INT_TYPE, 0),
#endif
   MAKE_VARIABLE("_auto_declare", &_pSLang_Auto_Declare_Globals, SLANG_INT_TYPE, 0),
   MAKE_VARIABLE("_slangtrace", &_pSLang_Trace, SLANG_INT_TYPE, 0),
   MAKE_VARIABLE("_slang_utf8_ok", &_pSLinterp_UTF8_Mode, SLANG_INT_TYPE, 1),
   MAKE_VARIABLE("_slang_install_prefix", &Install_Prefix, SLANG_STRING_TYPE, 1),
   MAKE_VARIABLE("NULL", NULL, SLANG_NULL_TYPE, 1),
   SLANG_END_INTRIN_VAR_TABLE
};

int SLang_init_slang (void) /*{{{*/
{
   char name[3];
   unsigned int i;
   SLFUTURE_CONST char **s;
   static SLFUTURE_CONST char *sys_defines [] =
     {
#if defined(__os2__)
	"OS2",
#endif
#if defined(__MSDOS__)
	"MSDOS",
#endif
#if defined(__WIN16__)
	"WIN16",
#endif
#if defined (__WIN32__)
	"WIN32",
#endif
#if defined(__NT__)
	"NT",
#endif
#if defined (VMS)
	"VMS",
#endif
#ifdef REAL_UNIX_SYSTEM
	"UNIX",
#endif
#if SLANG_HAS_FLOAT
	"SLANG_DOUBLE_TYPE",
#endif
	NULL
     };

   if (-1 == _pSLerr_init ())
     return -1;

   if (-1 == _pSLregister_types ()) return -1;

   if ((-1 == SLadd_intrin_fun_table(SLang_Basic_Table, NULL))
       || (-1 == SLadd_intrin_var_table (Intrin_Vars, NULL))
       || (-1 == _pSLang_init_slstrops ())
       || (-1 == _pSLang_init_sltime ())
       || (-1 == _pSLang_init_sllist ())
       || (-1 == _pSLstruct_init ())
#if SLANG_HAS_ASSOC_ARRAYS
       || (-1 == SLang_init_slassoc ())
#endif
#if SLANG_HAS_BOSEOS
       || (-1 == _pSLang_init_boseos ())
#endif
       || (-1 == _pSLang_init_exceptions ())
       )
     return -1;

   /* More nonsense for the windoze loader */
   if ((-1 == SLadd_intrinsic_variable ("_NARGS", &SLang_Num_Function_Args, SLANG_INT_TYPE, 1))
       || (-1 == SLadd_intrinsic_variable ("_traceback", &SLang_Traceback, SLANG_INT_TYPE, 0))
       || (-1 == SLadd_intrinsic_variable ("_slang_version", &SLang_Version, SLANG_INT_TYPE, 1))
       || (-1 == SLadd_intrinsic_variable ("_slang_version_string", &SLang_Version_String, SLANG_STRING_TYPE, 1))
       || (-1 == SLadd_intrinsic_variable ("_slang_doc_dir", &SLang_Doc_Dir, SLANG_STRING_TYPE, 1)))
     return -1;

   SLadd_global_variable (SLANG_SYSTEM_NAME);

   s = sys_defines;
   while (*s != NULL)
     {
	if (-1 == SLdefine_for_ifdef (*s)) return -1;
	s++;
     }

   /* give temp global variables $0 --> $9 */
   name[2] = 0; name[0] = '$';
   for (i = 0; i < 10; i++)
     {
	name[1] = (char) (i + '0');
	SLadd_global_variable (name);
     }

   SLang_init_case_tables ();

   /* Now add a couple of macros */
   SLang_load_string (".(_NARGS 1 - Sprintf error)verror");
   SLang_load_string (".(_NARGS 1 - Sprintf message)vmessage");

#if SLANG_HAS_SIGNALS
   if (-1 == SLang_add_interrupt_hook (_pSLang_check_signals_hook, NULL))
     return -1;
#endif

   if ((SLang_Doc_Dir != NULL)
       && (*SLang_Doc_Dir != 0))
     {
	char *docfile = SLpath_dircat (SLang_Doc_Dir, "slangfun.txt");
	/* NULL ok */
	(void) add_doc_file (docfile);
	SLfree (docfile);
     }

   if (_pSLang_Error)
     return -1;

   return 0;
}

/*}}}*/

static int This_Argc;
static SLang_Array_Type *This_Argv = NULL;

static int add_argc_argv (SLang_Array_Type *at)
{
   This_Argc = at->num_elements;
   if (-1 == SLadd_intrinsic_variable ("__argc", (VOID_STAR)&This_Argc,
				       SLANG_INT_TYPE, 1))
     return -1;

   if (-1 == SLadd_intrinsic_variable ("__argv", (VOID_STAR)at, SLANG_ARRAY_TYPE, 0))
     return -1;
   if (This_Argv != NULL)
     SLang_free_array (This_Argv);
   This_Argv = at;
   return 0;
}

static void set_argv_intrinsic (void)
{
   SLang_Array_Type *at;

   if (-1 == SLang_pop_array_of_type (&at, SLANG_STRING_TYPE))
     return;

   if (-1 == add_argc_argv (at))
     SLang_free_array (at);
}

int SLang_set_argc_argv (int argc, char **argv)
{
   SLang_Array_Type *at = _pSLstrings_to_array (argv, argc);

   if (at == NULL)
     return -1;

   if (-1 == add_argc_argv (at))
     {
	SLang_free_array (at);
	return -1;
     }

   return 0;
}

#if 0
int SLang_set_argc_argv (int argc, char **argv)
{
   static int this_argc;
   static char **this_argv;
   int i;

   if (argc < 0) argc = 0;
   this_argc = argc;

   if (NULL == (this_argv = (char **) _SLcalloc ((argc + 1), sizeof (char *))))
     return -1;
   memset ((char *) this_argv, 0, sizeof (char *) * (argc + 1));

   for (i = 0; i < argc; i++)
     {
	if (NULL == (this_argv[i] = SLang_create_slstring (argv[i])))
	  goto return_error;
     }

   if (-1 == SLadd_intrinsic_variable ("__argc", (VOID_STAR)&this_argc,
				       SLANG_INT_TYPE, 1))
     goto return_error;

   if (-1 == SLang_add_intrinsic_array ("__argv", SLANG_STRING_TYPE, 1,
					(VOID_STAR) this_argv, 1, argc))
     goto return_error;

   return 0;

   return_error:
   for (i = 0; i < argc; i++)
     SLang_free_slstring (this_argv[i]);   /* NULL ok */
   SLfree ((char *) this_argv);

   return -1;
}
#endif
