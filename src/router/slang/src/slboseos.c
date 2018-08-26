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

#if SLANG_HAS_BOSEOS
static SLang_Name_Type *BOS_Callback_Handler = NULL;
static SLang_Name_Type *EOS_Callback_Handler = NULL;
static SLang_Name_Type *BOF_Callback_Handler = NULL;
static SLang_Name_Type *EOF_Callback_Handler = NULL;

static void set_bos_eos_handlers (SLang_Name_Type *bos, SLang_Name_Type *eos)
{
   if (BOS_Callback_Handler != NULL)
     SLang_free_function (BOS_Callback_Handler);
   BOS_Callback_Handler = bos;

   if (EOS_Callback_Handler != NULL)
     SLang_free_function (EOS_Callback_Handler);
   EOS_Callback_Handler = eos;
}

static void set_bof_eof_handlers (SLang_Name_Type *bof, SLang_Name_Type *eof)
{
   if (BOF_Callback_Handler != NULL)
     SLang_free_function (BOF_Callback_Handler);
   BOF_Callback_Handler = bof;

   if (EOF_Callback_Handler != NULL)
     SLang_free_function (EOF_Callback_Handler);
   EOF_Callback_Handler = eof;
}

static int Handler_Active = 0;

int _pSLcall_bos_handler (SLFUTURE_CONST char *file, int line)
{
   int status = 0;
   int err;

   if (BOS_Callback_Handler == NULL)
     return 0;

   if (Handler_Active)
     return 0;

   if ((0 != (err = _pSLang_Error))
       && (-1 == _pSLang_push_error_context ()))
     return -1;

   Handler_Active++;
   if ((-1 == SLang_start_arg_list ())
       || (-1 == SLang_push_string (file))
       || (-1 == SLclass_push_int_obj (SLANG_INT_TYPE, line))
       || (-1 == SLang_end_arg_list ())
       || (-1 == SLexecute_function (BOS_Callback_Handler)))
     {
	set_bos_eos_handlers (NULL, NULL);
	status = -1;
     }
   Handler_Active--;

   if (err)
     _pSLang_pop_error_context (status != 0);

   return status;
}

int _pSLcall_eos_handler (void)
{
   int err, status = 0;

   if ((EOS_Callback_Handler == NULL)
       || (Handler_Active))
     return 0;

   if ((0 != (err = _pSLang_Error))
       && (-1 == _pSLang_push_error_context ()))
     return -1;

   Handler_Active++;
   if ((-1 == SLang_start_arg_list ())
       || (-1 == SLang_end_arg_list ())
       || (-1 == SLexecute_function (EOS_Callback_Handler)))
     {
	status = -1;
	set_bos_eos_handlers (NULL, NULL);
     }
   Handler_Active--;
   if (err)
     _pSLang_pop_error_context (status != 0);

   return status;
}

int _pSLcall_bof_handler (SLFUTURE_CONST char *fun, SLFUTURE_CONST char *file)
{
   int status = 0, err;

   if (BOF_Callback_Handler == NULL)
     return 0;

   if (Handler_Active)
     return 0;

   if ((0 != (err = _pSLang_Error))
       && (-1 == _pSLang_push_error_context ()))
     return -1;

   Handler_Active++;
   if ((-1 == SLang_start_arg_list ())
       || (-1 == SLang_push_string (fun))
       || (-1 == SLang_push_string (file))
       || (-1 == SLang_end_arg_list ())
       || (-1 == SLexecute_function (BOF_Callback_Handler)))
     {
	set_bof_eof_handlers (NULL, NULL);
	status = -1;
     }
   Handler_Active--;
   if (err)
     _pSLang_pop_error_context (status != 0);
   return status;
}

int _pSLcall_eof_handler (void)
{
   int status = 0, err;

   if ((EOF_Callback_Handler == NULL)
       || (Handler_Active))
     return 0;

   if ((0 != (err = _pSLang_Error))
       && (-1 == _pSLang_push_error_context ()))
     return -1;

   Handler_Active++;
   if ((-1 == SLang_start_arg_list ())
       || (-1 == SLang_end_arg_list ())
       || (-1 == SLexecute_function (EOF_Callback_Handler)))
     {
	status = -1;
	set_bof_eof_handlers (NULL, NULL);
     }
   Handler_Active--;
   if (err)
     _pSLang_pop_error_context (status != 0);
   return status;
}

static int pop_new_push_old (SLang_Name_Type **handler)
{
   SLang_Name_Type *new_handler;
   SLang_Name_Type *old_handler;

   old_handler = *handler;
   if (SLang_peek_at_stack () == SLANG_NULL_TYPE)
     {
	SLang_pop_null ();
	new_handler = NULL;
     }
   else if (NULL == (new_handler = SLang_pop_function ()))
     return -1;

   if (-1 == _pSLang_push_nt_as_ref (old_handler))
     {
	SLang_free_function (new_handler);
	return -1;
     }

   SLang_free_function (old_handler);
   *handler = new_handler;
   return 0;
}

static void set_bos_handler (void)
{
   (void) pop_new_push_old (&BOS_Callback_Handler);
}

static void set_eos_handler (void)
{
   (void) pop_new_push_old (&EOS_Callback_Handler);
}

static void set_bof_handler (void)
{
   (void) pop_new_push_old (&BOF_Callback_Handler);
}

static void set_eof_handler (void)
{
   (void) pop_new_push_old (&EOF_Callback_Handler);
}

#if SLANG_HAS_DEBUGGER_SUPPORT
static SLang_Name_Type *Debug_Hook = NULL;
static int Debug_Handler_Active = 0;

static void set_debug_hook (SLang_Name_Type *deb)
{
   if (Debug_Hook != NULL)
     SLang_free_function (Debug_Hook);
   Debug_Hook = deb;
}

/* int _pSLcall_debug_hook (char *file, int line, char *funct) */
int _pSLcall_debug_hook (SLFUTURE_CONST char *file, int line)
{
   int status = 0, err;

   if (Debug_Hook == NULL)
     return 0;

   if (Debug_Handler_Active)
     return 0;

   if ((0 != (err = _pSLang_Error))
       && (-1 == _pSLang_push_error_context ()))
     return -1;

   Debug_Handler_Active++;
   if ((-1 == SLang_start_arg_list ())
       || (-1 == SLang_push_string (file))
       || (-1 == SLclass_push_int_obj (SLANG_INT_TYPE, line))
       || (-1 == SLang_end_arg_list ())
       || (-1 == SLexecute_function (Debug_Hook)))
     {
	status = -1;
	set_debug_hook (NULL);
     }
   Debug_Handler_Active--;

   if (err)
     (void) _pSLang_pop_error_context (status != 0);

   return status;
}

static void set_debug_hook_intrin (void)
{
   pop_new_push_old (&Debug_Hook);
}

static void get_frame_variable (int *depth, char *name)
{
   (void) _pSLang_get_frame_variable ((unsigned int) *depth, name);
}

static void set_frame_variable (void)
{
   char *name;
   int depth;

   if (-1 == SLroll_stack (3))
     return;

   if (-1 == SLang_pop_slstring (&name))
     return;

   if (0 == SLang_pop_int (&depth))
     (void) _pSLang_set_frame_variable ((unsigned int) depth, name);
   SLang_free_slstring (name);
}

static void get_frame_info (int *depth)
{
#define NUM_INFO_FIELDS 5
   static SLFUTURE_CONST char *field_names[NUM_INFO_FIELDS] =
     {
	"file", "line", "function", "locals", "namespace"
     };
   SLtype field_types[NUM_INFO_FIELDS];
   VOID_STAR field_values[NUM_INFO_FIELDS];
   SLang_Array_Type *at = NULL;
   _pSLang_Frame_Info_Type f;
   unsigned int i;

   if (-1 == _pSLang_get_frame_fun_info ((unsigned int)*depth, &f))
     return;

   i = 0;

   field_values[i] = (VOID_STAR) &f.file;
   if (f.file == NULL)
     field_types[i] = SLANG_NULL_TYPE;
   else
     field_types[i] = SLANG_STRING_TYPE;
   i++;

   field_values[i] = &f.line;
   field_types[i] = SLANG_UINT_TYPE;
   i++;

   field_values[i] = (VOID_STAR) &f.function;
   if (f.function == NULL)
     field_types[i] = SLANG_NULL_TYPE;
   else
     field_types[i] = SLANG_STRING_TYPE;
   i++;

   if (f.locals == NULL)
     {
	field_types[i] = SLANG_NULL_TYPE;
	field_values[i] = (VOID_STAR) &f.locals;
     }
   else
     {
	if (NULL == (at = _pSLstrings_to_array (f.locals, f.nlocals)))
	  return;

	field_values[i] = &at;
	field_types[i] = SLANG_ARRAY_TYPE;
     }
   i++;

   field_values[i] = (VOID_STAR) &f.ns;
   if (f.ns == NULL)
     field_types[i] = SLANG_NULL_TYPE;
   else
     field_types[i] = SLANG_STRING_TYPE;
   i++;

   (void) SLstruct_create_struct (NUM_INFO_FIELDS, field_names, field_types, field_values);

   if (at != NULL)
     SLang_free_array (at);
}

static int get_frame_depth (void)
{
   return _pSLang_get_frame_depth ();
}

static void use_frame_namespace (int *depth)
{
   _pSLang_use_frame_namespace (*depth);
}

#endif

static SLang_Intrin_Fun_Type Intrin_Funs [] =
{
   MAKE_INTRINSIC_0("_set_bos_handler", set_bos_handler, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_set_eos_handler", set_eos_handler, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_set_bof_handler", set_bof_handler, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_set_eof_handler", set_eof_handler, SLANG_VOID_TYPE),
#if SLANG_HAS_DEBUGGER_SUPPORT
   MAKE_INTRINSIC_0("_set_frame_variable", set_frame_variable, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_IS("_get_frame_variable", get_frame_variable, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_get_frame_depth", get_frame_depth, SLANG_INT_TYPE),
   MAKE_INTRINSIC_I("_get_frame_info", get_frame_info, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_I("_use_frame_namespace", use_frame_namespace, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_set_debug_hook", set_debug_hook_intrin, SLANG_VOID_TYPE),
#endif
   SLANG_END_INTRIN_FUN_TABLE
};

int _pSLang_init_boseos (void)
{
   return SLadd_intrin_fun_table (Intrin_Funs, NULL);
}
#endif				       /* SLANG_HAS_BOSEOS */
