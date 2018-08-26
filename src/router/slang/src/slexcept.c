/* Exception Handling */
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

#include "slang.h"

#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

#include "_slang.h"

static SLang_Object_Type *Object_Thrownp = NULL;
static SLang_Object_Type Object_Thrown;
static SLCONST char *File_With_Error = NULL;
static SLCONST char *Function_With_Error = NULL;
static SLCONST char *Last_Function_With_Error = NULL;   /* either slstring or "<top-level>" */
static _pSLerr_Error_Queue_Type *Error_Message_Queue;

static int Linenum_With_Error = -1;

static void free_thrown_object (void)
{
   if (Object_Thrownp != NULL)
     {
	SLang_free_object (Object_Thrownp);
	Object_Thrownp = NULL;
     }
}

typedef struct Error_Context_Type
{
   int err;
   int err_cleared;
   int rethrow;
   int linenum;
   SLCONST char *file;
   SLCONST char *function;
   _pSLerr_Error_Queue_Type *err_queue;
   int object_was_thrown;
   SLang_Object_Type object_thrown;
   struct Error_Context_Type *next;
}
Error_Context_Type;

static Error_Context_Type *Error_Context;

int _pSLang_push_error_context (void)
{
   Error_Context_Type *c;

   if (NULL == (c = (Error_Context_Type *)SLmalloc (sizeof (Error_Context_Type))))
     return -1;

   c->next = Error_Context;
   c->err = _pSLang_Error;
   c->err_cleared = 0;
   c->rethrow = 0;
   c->file = File_With_Error;
   c->function = Function_With_Error;  /* steal pointers */
   c->linenum = Linenum_With_Error;
   c->err_queue = Error_Message_Queue;

   File_With_Error = NULL;
   Function_With_Error = NULL;
   Linenum_With_Error = -1;

   if (NULL == (Error_Message_Queue = _pSLerr_new_error_queue (1)))
     {
	Error_Message_Queue = c->err_queue;
	SLfree ((char *) c);
	return -1;
     }

   Error_Context = c;
   SLKeyBoard_Quit = 0;

   c->object_was_thrown = (Object_Thrownp != NULL);
   if (c->object_was_thrown)
     {
	c->object_thrown = Object_Thrown;
	Object_Thrownp = NULL;
     }

   if (-1 == SLang_set_error (0))
     {
	_pSLang_pop_error_context (1);
	return -1;
     }
   return 0;
}

/* this will be called with use_current_queue set to 0 if the catch block
 * was processed with no error.  If an error occurs processing the catch
 * block, then that error will take precedence over the one triggering the
 * catch block.  However, if the original error is rethrown, then this routine
 * will still be called with use_current_queue non-zero since all the caller
 * knows is that an error occured and cannot tell if it was a rethrow.
 */
int _pSLang_pop_error_context (int use_current_queue)
{
   Error_Context_Type *e;

   e = Error_Context;
   if (e == NULL)
     return -1;

   Error_Context = e->next;

   if ((use_current_queue == 0) || (e->rethrow))
     {
	(void) _pSLerr_set_error_queue (e->err_queue);
	_pSLerr_delete_error_queue (Error_Message_Queue);
	Error_Message_Queue = e->err_queue;
	free_thrown_object ();
	if (e->object_was_thrown)
	  {
	     Object_Thrownp = &Object_Thrown;
	     Object_Thrown = e->object_thrown;
	  }
     }
   else
     {
	_pSLerr_delete_error_queue (e->err_queue);
	if (e->object_was_thrown)
	  SLang_free_object (&e->object_thrown);
     }

   if (_pSLang_Error == 0)
     {
	if (e->err_cleared == 0)
	  {
	     SLang_free_slstring ((char *)File_With_Error);
	     SLang_free_slstring ((char *)Function_With_Error);
	     File_With_Error = e->file; e->file = NULL;
	     Function_With_Error = e->function; e->function = NULL;
	     Linenum_With_Error = e->linenum;
	     (void) SLang_set_error (e->err);
	  }
     }

   if (_pSLang_Error == SL_UserBreak_Error)
     SLKeyBoard_Quit = 1;

   SLang_free_slstring ((char *) e->file);
   SLang_free_slstring ((char *) e->function);

   SLfree ((char *) e);
   return 0;
}

int _pSLerr_get_last_error (void)
{
   Error_Context_Type *e;

   e = Error_Context;
   if (e == NULL)
     return 0;
   return e->err;
}

static void do_file_line_funct_error (SLCONST char *file, int linenum, SLCONST char *function)
{
   if ((file == NULL) || (_pSLang_Error == 0))
     return;

   if (Last_Function_With_Error == function)	       /* either slstring or "<top-level>" */
     return;
   Last_Function_With_Error = function;
   if (SLang_Traceback && *function)
     _pSLerr_traceback_msg ("%s:%d:%s:%s\n", file, linenum, function, SLerr_strerror (_pSLang_Error));
}

int _pSLerr_set_line_info (SLFUTURE_CONST char *file, int linenum, SLFUTURE_CONST char *fun)
{
   if (linenum == 0)
     linenum = -1;

   if (0 == (SLang_Traceback == SL_TB_FULL))
     {
	if ((File_With_Error != NULL) && (Linenum_With_Error != -1))
	  return 0;
	if ((linenum == -1) && (file == NULL))
	  return 0;
     }

   if (fun == NULL)
     fun = "<top-level>";

   do_file_line_funct_error (file, linenum, fun);

   if (File_With_Error != NULL)
     return 0;

   Linenum_With_Error = linenum;
   if (file != NULL)
     {
	if (NULL == (file = SLang_create_slstring (file)))
	  return -1;
     }
   if (NULL == (fun = SLang_create_slstring (fun)))
     {
	SLang_free_slstring ((char *) file);    /* NULL ok */
	return -1;
     }

   SLang_free_slstring ((char *)File_With_Error);
   SLang_free_slstring ((char *)Function_With_Error);

   File_With_Error = file;
   Function_With_Error = fun;

#if SLANG_HAS_BOSEOS && SLANG_HAS_DEBUGGER_SUPPORT
   (void) _pSLcall_debug_hook (file, linenum);
#endif

   return 0;
}

static int _pSLerr_get_last_error_line_info (SLCONST char **filep, int *linep, SLCONST char **funp)
{
   Error_Context_Type *e = Error_Context;
   if (e == NULL)
     {
	*filep = NULL;
	*linep = -1;
	*funp = NULL;
	return -1;
     }
   *filep = e->file;
   *linep = e->linenum;
   *funp = e->function;
   return 0;
}

static char *get_error_msg_from_queue (int type)
{
   Error_Context_Type *e = Error_Context;

   if (e == NULL)
     return NULL;

   return _pSLerr_get_error_from_queue (e->err_queue, type);
}

void (*SLang_User_Clear_Error)(void) = NULL;
void _pSLerr_clear_error (int set_clear_err_flag)
{
   SLang_set_error (0);
   free_thrown_object ();

   if ((Error_Context != NULL)
       && (set_clear_err_flag))
     {
	/* This is used only for error blocks */
	Error_Context->err_cleared = 1;
     }

   SLang_free_slstring ((char *) File_With_Error); File_With_Error = NULL;
   SLang_free_slstring ((char *) Function_With_Error); Function_With_Error = NULL;
   Linenum_With_Error = -1;
   Last_Function_With_Error = NULL;
   if (SLang_User_Clear_Error != NULL) (*SLang_User_Clear_Error)();
   _pSLerr_free_queued_messages ();
}

static int rethrow_error (void)
{
   Error_Context_Type *e = Error_Context;

   if (e == NULL)
     return 0;

   SLang_set_error (e->err);
   e->rethrow=1;
   e->err_cleared = 0;
   return 0;
}

int _pSLerr_throw (void)
{
   int e;
   int nargs = SLang_Num_Function_Args;
   char *msg = NULL;

   free_thrown_object ();

   switch (nargs)
     {
      case 3:
	if (-1 == SLang_pop (&Object_Thrown))
	  return -1;
	Object_Thrownp = &Object_Thrown;
	/* drop */
      case 2:
	if (-1 == SLang_pop_slstring (&msg))
	  {
	     free_thrown_object ();
	     return -1;
	  }
      case 1:
	/* drop */
	if (-1 == _pSLerr_pop_exception (&e))
	  {
	     SLang_free_slstring (msg);/* NULL ok */
	     free_thrown_object ();
	     return -1;
	  }
	break;

      case 0:			       /* rethrow */
	return rethrow_error ();

      default:
	_pSLang_verror (SL_NumArgs_Error, "expecting: throw error [, optional-message [, optional-arg]]");
	return -1;
     }

   if (msg != NULL)
     {
	_pSLang_verror (e, "%s", msg);
	SLang_free_slstring (msg);
     }
   else
     SLang_set_error (e);

   return 0;
}

int SLerr_throw (int err, SLFUTURE_CONST char *msg, SLtype obj_type, VOID_STAR objptr)
{
   free_thrown_object ();

   if ((obj_type != 0) || (objptr != NULL))
     {
	if (-1 == SLang_push_value (obj_type, objptr))
	  return -1;
	if (-1 == SLang_pop (&Object_Thrown))
	  return -1;
	Object_Thrownp = &Object_Thrown;
     }

   if (msg != NULL)
     _pSLang_verror (err, "%s", msg);
   else
     SLang_set_error (err);

   return 0;
}

static void new_exception (char *name, int *baseclass, char *description)
{
   (void) SLerr_new_exception (*baseclass, name, description);
}

static void get_exception_info_intrinsic (void)
{
#define NUM_EXCEPT_FIELDS 8
   static SLFUTURE_CONST char *field_names[NUM_EXCEPT_FIELDS] =
     {
	"error", "descr", "file", "line", "function", "object", "message", "traceback"
     };
   SLtype field_types[NUM_EXCEPT_FIELDS];
   VOID_STAR field_values[NUM_EXCEPT_FIELDS];
   int err;
   SLCONST char *desc;
   SLCONST char *file;
   SLCONST char *function;
   SLCONST char *errmsg;
   SLCONST char *tbmsg;
   int linenum;

   err = _pSLerr_get_last_error ();
   if (err == 0)
     {
	(void) SLang_push_null ();
	return;
     }

   desc = SLerr_strerror (err);
   (void) _pSLerr_get_last_error_line_info (&file, &linenum, &function);

   field_types[0] = SLANG_INT_TYPE;
   field_values[0] = (VOID_STAR) &err;

   field_types[1] = SLANG_STRING_TYPE;
   field_values[1] = (VOID_STAR) &desc;

   field_types[2] = SLANG_STRING_TYPE;
   field_values[2] = (VOID_STAR) &file;

   field_types[3] = SLANG_INT_TYPE;
   field_values[3] = (VOID_STAR) &linenum;

   field_types[4] = SLANG_STRING_TYPE;
   field_values[4] = (VOID_STAR) &function;

   if ((Error_Context == NULL)
       || (Error_Context->object_was_thrown == 0))
     {
	char *null = NULL;
	field_types[5] = SLANG_NULL_TYPE;
	field_values[5] = (VOID_STAR) &null;
     }
   else
     {
	SLtype data_type = Error_Context->object_thrown.o_data_type;
	field_types[5] = data_type;
	field_values[5] = _pSLclass_get_ptr_to_value (_pSLclass_get_class (data_type),
						      &Error_Context->object_thrown);
     }
   errmsg = get_error_msg_from_queue  (_SLERR_MSG_ERROR);
   if ((errmsg == NULL) || (*errmsg == 0))
     errmsg = desc;
   field_types[6] = SLANG_STRING_TYPE;
   field_values[6] = (VOID_STAR) &errmsg;

   tbmsg = get_error_msg_from_queue  (_SLERR_MSG_TRACEBACK);
   field_types[7] = (tbmsg == NULL) ? SLANG_NULL_TYPE : SLANG_STRING_TYPE;
   field_values[7] = (VOID_STAR) &tbmsg;

   (void) SLstruct_create_struct (NUM_EXCEPT_FIELDS, field_names, field_types, field_values);
   if (errmsg != desc)
     SLang_free_slstring ((char *) errmsg);
   SLang_free_slstring ((char *)tbmsg);
}

int _pSLerr_pop_exception (int *e)
{
   return SLang_pop_integer (e);
}

static int new_exception_hook (SLFUTURE_CONST char *name, SLFUTURE_CONST char *desc, int err_code)
{
   SLang_IConstant_Type *ic;

   (void) desc;
   if (NULL != (ic = (SLang_IConstant_Type *)_pSLlocate_name (name)))
     {
	if ((ic->name_type != SLANG_ICONSTANT)
	    || (ic->value != err_code))
	  {
	     _pSLang_verror (SL_RunTime_Error, "Exception %s already exists and may not be redefined", name);
	     return -1;
	  }
	return 0;
     }

   if (-1 == SLns_add_iconstant (NULL, name, SLANG_INT_TYPE, err_code))
     return -1;

   return 0;
}

static SLang_Intrin_Fun_Type Except_Table [] =
{
   MAKE_INTRINSIC_0("__get_exception_info", get_exception_info_intrinsic, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_SIS("new_exception", new_exception, SLANG_VOID_TYPE),
   SLANG_END_INTRIN_FUN_TABLE
};

int _pSLang_init_exceptions (void)
{
   _pSLerr_New_Exception_Hook = new_exception_hook;
   if (-1 == _pSLerr_init_interp_exceptions ())
     return -1;

   if (-1 == SLadd_intrin_fun_table(Except_Table, NULL))
     return -1;

   return 0;
}

