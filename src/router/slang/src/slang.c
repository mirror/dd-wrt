/* -*- mode: C; mode: fold; -*- */
/* slang.c  --- guts of S-Lang interpreter */
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

#if SLANG_HAS_FLOAT
# include <math.h>
#endif

#include "slang.h"
#include "_slang.h"

#if SLANG_OPTIMIZE_FOR_SPEED
# define USE_COMBINED_BYTECODES		1
#else
# define USE_COMBINED_BYTECODES		0
#endif

#define USE_UNUSED_BYCODES_IN_SWITCH	1

struct _pSLBlock_Type;

typedef struct
{
   struct _pSLBlock_Type *body;
   unsigned int num_refs;

   SLFUTURE_CONST char *file;
#define SLANG_MAX_LOCAL_VARIABLES 255
   unsigned char nlocals;	       /* number of local variables */
   unsigned char nargs;		       /* number of arguments */
   char **local_variables;
   SLang_NameSpace_Type *static_ns;  /* namespace containing this function */
   SLang_NameSpace_Type *private_ns;  /* private namespace where this function was declared */
#if SLANG_HAS_BOSEOS
   int issue_bofeof_info;
#endif
}
Function_Header_Type;

typedef struct
{
   SLFUTURE_CONST char *name;
   SLang_Name_Type *next;
   char name_type;

   /* If header is NULL, then the autoload_file field will be used.  The file
    * is to be loaded into the ns namespace.
    */
   Function_Header_Type *header;    /* body of function */
   SLFUTURE_CONST char *autoload_file;
   SLang_NameSpace_Type *autoload_ns;
}
_pSLang_Function_Type;
static char *Local_Variable_Names[SLANG_MAX_LOCAL_VARIABLES];

typedef struct
{
   char *name;
   SLang_Name_Type *next;
   char name_type;

   int local_var_number;
}
SLang_Local_Var_Type;

#if SLANG_HAS_DEBUG_CODE
typedef struct
{
   int linenum;
   char *filename;
}
Linenum_Info_Type;
#endif

typedef struct _pSLBlock_Type
{
   _pSLang_BC_Type bc_main_type;
   unsigned char bc_sub_type;	       /* no types greater than 255 allowed here */
   unsigned short linenum;
   union
     {
	struct _pSLBlock_Type *blk;
	int i_blk;

	SLang_Name_Type *nt_blk;
	SLang_App_Unary_Type *nt_unary_blk;
	SLang_Arith_Binary_Type *nt_binary_blk;
	SLang_Intrin_Var_Type *nt_ivar_blk;
	SLang_Intrin_Fun_Type *nt_ifun_blk;
	SLang_Global_Var_Type *nt_gvar_blk;
	SLang_HConstant_Type *hconst_blk;
	SLang_IConstant_Type *iconst_blk;
	SLang_LConstant_Type *lconst_blk;
	SLang_FConstant_Type *fconst_blk;
	SLang_DConstant_Type *dconst_blk;
#ifdef HAVE_LONG_LONG
	_pSLang_LLConstant_Type *llconst_blk;
#endif
	_pSLang_Function_Type *nt_fun_blk;

	VOID_STAR ptr_blk;
	SLFUTURE_CONST char *s_blk;
	SLang_BString_Type *bs_blk;

#if SLANG_HAS_FLOAT
	double *double_blk;		       /*literal double is a pointer */
#endif
#ifdef HAVE_LONG_LONG
	long long *llong_blk;
#endif
	float float_blk;
	long l_blk;
	struct _pSLang_Struct_Type *struct_blk;
	int (*call_function)(void);
#if SLANG_HAS_DEBUG_CODE
	Linenum_Info_Type *line_info;
#endif
     }
   b;
}
SLBlock_Type;

/* Debugging and tracing variables */

void (*SLang_Enter_Function)(SLFUTURE_CONST char *) = NULL;
void (*SLang_Exit_Function)(SLFUTURE_CONST char *) = NULL;
/* If non null, these call C functions before and after a slang function. */

int _pSLang_Trace = 0;
/* If _pSLang_Trace = -1, do not trace intrinsics */
static int Trace_Mode = 0;

static SLFUTURE_CONST char *Trace_Function;	       /* function to be traced */

int SLang_Traceback = SL_TB_PARTIAL;

#if SLANG_HAS_BOSEOS
static int BOS_Stack_Depth;
#endif

static SLFUTURE_CONST char *This_Compile_Filename;
static unsigned int This_Compile_Linenum;

/* These variables handle _NARGS processing by the parser */
int SLang_Num_Function_Args = 0;
static int *Num_Args_Stack;
static unsigned int Recursion_Depth;
static SLang_Object_Type *Frame_Pointer;
static int Next_Function_Num_Args;

static unsigned int Frame_Pointer_Depth;
static unsigned int *Frame_Pointer_Stack;

#if SLANG_HAS_QUALIFIERS
static SLang_Struct_Type *Next_Function_Qualifiers;
static SLang_Struct_Type *Function_Qualifiers;
static SLang_Struct_Type **Function_Qualifiers_Stack;
#endif

static _pSLang_Function_Type *Current_Function = NULL;
static Function_Header_Type *Current_Function_Header;

typedef struct
{
   _pSLang_Function_Type *function;
   Function_Header_Type *header;       /* could be different from function->header */
   SLang_Object_Type *local_variable_frame;
   SLang_NameSpace_Type *static_ns;
   SLang_NameSpace_Type *private_ns;
   /* file and line where function call occurs, 0 if no-info */
   SLCONST char *file;
   unsigned int line;
}
Function_Stack_Type;
static Function_Stack_Type *Function_Stack;
static Function_Stack_Type *Function_Stack_Ptr;
/* static Function_Stack_Type *Function_Stack_Ptr_Max; */

static SLang_NameSpace_Type *This_Private_NameSpace;
static SLang_NameSpace_Type *This_Static_NameSpace;
static SLang_NameSpace_Type *Global_NameSpace;
static SLang_NameSpace_Type *Locals_NameSpace;

static SLang_Name_Type *
  find_global_hashed_name (SLCONST char *, unsigned long,
			   SLang_NameSpace_Type *, SLang_NameSpace_Type *,
			   SLang_NameSpace_Type *, int);

static int Lang_Break_Condition = 0;
/* true if any one below is true.  This keeps us from testing 3 variables.
 * I know this can be perfomed with a bitmapped variable, but...
 *
 * Note that Lang_Break is positive when handling the break statements,
 * and is negative when handling continue-N forms.  The reason this variable
 * is involved is that continue-N is equivalent to break, break,...,continue.
 */
static int Lang_Break = 0;
static int Lang_Return = 0;
/* static int Lang_Continue = 0; */

static SLang_Object_Type *Run_Stack;
static SLang_Object_Type *Stack_Pointer;
static SLang_Object_Type *Stack_Pointer_Max;

/* Might want to increase this. */
static SLang_Object_Type Local_Variable_Stack[SLANG_MAX_LOCAL_STACK];
static SLang_Object_Type *Local_Variable_Frame = Local_Variable_Stack;

#define INTERRUPT_ERROR		0x01
#define INTERRUPT_SIGNAL	0x02
static volatile int Handle_Interrupt;	       /* bitmapped value */
#define IS_SLANG_ERROR	(Handle_Interrupt & INTERRUPT_ERROR)

static void free_function_header (Function_Header_Type *);

#if SLANG_HAS_SIGNALS
static int check_signals (void);
#endif

#if SLANG_OPTIMIZE_FOR_SPEED
# define NUM_CLASSES 512		       /* must be large enough for built-ins */
static SLclass_Type The_Class_Types [NUM_CLASSES];
static SLang_Class_Type *The_Classes[NUM_CLASSES];

# define GET_CLASS_TYPE(x) \
   (((x) < NUM_CLASSES) ? The_Class_Types[(x)] : _pSLang_get_class_type(x))

void _pSLang_set_class_info (SLtype t, SLang_Class_Type *cl)
{
   if (t < NUM_CLASSES)
     {
	The_Class_Types[t] = cl->cl_class_type;
	The_Classes [t] = cl;
     }
}
# define GET_CLASS(cl,t) \
   if (((t)>=NUM_CLASSES) || (NULL == (cl = The_Classes[(t)]))) \
       cl = _pSLclass_get_class(t)
# define GET_BUILTIN_CLASS(cl,t) \
   if (NULL == (cl = The_Classes[(t)])) \
       cl = _pSLclass_get_class(t)
#else
# define GET_CLASS(cl,t) cl = _pSLclass_get_class(t)
# define GET_BUILTIN_CLASS(cl,t) cl = _pSLclass_get_class(t)
# define GET_CLASS_TYPE(t) _pSLclass_get_class(t)->cl_class_type
#endif

#if SLANG_OPTIMIZE_FOR_SPEED
SLclass_Type _pSLang_get_class_type (SLtype t)
{
   SLang_Class_Type *cl;
#if SLANG_OPTIMIZE_FOR_SPEED
   if (t < NUM_CLASSES)
     return The_Class_Types[t];
#endif
   cl = _pSLclass_get_class (t);
   return cl->cl_class_type;
}
#endif

/* If 0, not an arith type.  Otherwise it is.  Also, value implies precedence
 * See slarith.c for how this is used.
 */
static int Is_Arith_Type_Array [256];
#define IS_ARITH_TYPE(t) \
   (((t) < 256) ? Is_Arith_Type_Array[t] : 0)

static void do_traceback (SLCONST char *);
static void do_function_traceback (Function_Header_Type *, unsigned int);

static int init_interpreter (void);

/*{{{ push/pop/etc stack manipulation functions */

/* These routines are assumed to work even in the presence of a SLang_Error. */

_INLINE_ static int pop_object (SLang_Object_Type *x)
{
   register SLang_Object_Type *y;

   y = Stack_Pointer;
   IF_UNLIKELY(y == Run_Stack)
     {
	(void) SLang_set_error (SL_STACK_UNDERFLOW);
	x->o_data_type = 0;
	return -1;
     }
   y--;
   *x = *y;

   Stack_Pointer = y;
   return 0;
}

int SLang_pop (SLang_Object_Type *x)
{
   return pop_object (x);
}

#if 0
static int pop_2_objs (SLang_Object_Type *a, SLang_Object_Type *b)
{
   register SLang_Object_Type *y;

   y = Stack_Pointer;

   if (Run_Stack + 2 > y)
     {
	(void) SLang_set_error (SL_STACK_UNDERFLOW);
	a->o_data_type = 0;
	b->o_data_type = 0;
	SLdo_pop_n (y - Run_Stack);
	return -1;
     }
   *b = *(--y);
   *a = *(--y);
   Stack_Pointer = y;
   return 0;
}
#endif

/* This function pops the top of the stack to x[0], next to x[1], etc...  This is
 * backwards from what might be expected but this corresponds to the order of the local
 * variable stack.
 */
_INLINE_
static int pop_n_objs_reverse (SLang_Object_Type *x, unsigned int n)
{
   register SLang_Object_Type *y;
   unsigned int i;

   y = Stack_Pointer;

   if (Run_Stack + n > y)
     {
	(void) SLang_set_error (SL_STACK_UNDERFLOW);
	for (i = 0; i < n; i++)
	  x[i].o_data_type = 0;
	(void) SLdo_pop_n ((unsigned int) (y - Run_Stack));
	return -1;
     }

   for (i = 0; i < n; i++)
     {
	y--;
	x[i] = *y;
     }
   Stack_Pointer = y;
   return 0;
}

_INLINE_
static int peek_at_stack (void)
{
   IF_UNLIKELY(Stack_Pointer == Run_Stack)
     {
	(void) SLang_set_error (SL_STACK_UNDERFLOW);
	return -1;
     }

   return (int) (Stack_Pointer - 1)->o_data_type;
}

int SLang_peek_at_stack (void)
{
   return peek_at_stack ();
}

int _pSLang_peek_at_stack2 (SLtype *_typep)
{
   SLtype type;
   if (Stack_Pointer == Run_Stack)
     {
	(void) SLang_set_error (SL_STACK_UNDERFLOW);
	return -1;
     }

   type = (Stack_Pointer - 1)->o_data_type;
   if (type == SLANG_ARRAY_TYPE)
     *_typep = (Stack_Pointer - 1)->v.array_val->data_type;
   else
     *_typep = type;
   return (int) type;
}

int SLang_peek_at_stack_n (unsigned int n)
{
   unsigned int stklen = (unsigned int)(Stack_Pointer - Run_Stack);

   if (n >= stklen)
     {
	(void) SLang_set_error (SL_STACK_UNDERFLOW);
	return -1;
     }
   return (int) (Stack_Pointer - (n+1))->o_data_type;
}

static int pop_ctrl_integer (int *i)
{
   int type;
   SLang_Class_Type *cl;
#if SLANG_OPTIMIZE_FOR_SPEED
   register SLang_Object_Type *y;

   /* Most of the time, either an integer or a char will be on the stack.
    * Optimize these cases.
    */
   y = Stack_Pointer;
   IF_UNLIKELY(y == Run_Stack)
     {
	(void) SLang_set_error (SL_STACK_UNDERFLOW);
	return -1;
     }
   y--;

   type = (int) y->o_data_type;
   if (type == SLANG_INT_TYPE)
     {
	Stack_Pointer = y;
	*i = y->v.int_val;
	return 0;
     }
   if (type == SLANG_CHAR_TYPE)
     {
	Stack_Pointer = y;
	*i = y->v.char_val;
	return 0;
     }
#else
   if (-1 == (type = peek_at_stack ()))
     return -1;
#endif

   GET_CLASS(cl,type);
   if (cl->cl_to_bool == NULL)
     {
	_pSLang_verror (SL_TYPE_MISMATCH,
		      "%s cannot be used in a boolean context",
		      cl->cl_name);
	return -1;
     }
   return cl->cl_to_bool ((unsigned char) type, i);
}

int SLang_peek_at_stack1_n (unsigned int n)
{
   int type;

   type = SLang_peek_at_stack_n (n);
   if (type == SLANG_ARRAY_TYPE)
     type = (Stack_Pointer - (n+1))->v.array_val->data_type;

   return type;
}

int SLang_peek_at_stack1 (void)
{
   return SLang_peek_at_stack1_n (0);
}

/* _INLINE_ */
static void free_object (SLang_Object_Type *obj, SLang_Class_Type *cl)
{
   if ((obj == NULL)
       || (cl->cl_class_type == SLANG_CLASS_TYPE_SCALAR))
     return;

#if SLANG_OPTIMIZE_FOR_SPEED
   if (obj->o_data_type == SLANG_STRING_TYPE)
     {
	_pSLang_free_slstring (obj->v.s_val);
	return;
     }
#endif
   (*cl->cl_destroy) (obj->o_data_type, (VOID_STAR) &obj->v);
}

void SLang_free_object (SLang_Object_Type *obj)
{
   SLtype data_type;
   SLang_Class_Type *cl;

   if (obj == NULL) return;
   data_type = obj->o_data_type;
   GET_CLASS(cl,data_type);
   free_object (obj, cl);
}

_INLINE_ static int push_object (SLang_Object_Type *x)
{
   register SLang_Object_Type *y;
   y = Stack_Pointer;

   /* if there is a SLang_Error, probably not much harm will be done
    if it is ignored here */
   /* if (SLang_Error) return; */

   /* flag it now */
   IF_UNLIKELY(y >= Stack_Pointer_Max)
     {
	(void) SLang_set_error (SL_STACK_OVERFLOW);
	return -1;
     }

   *y = *x;
   Stack_Pointer = y + 1;
   return 0;
}

int SLang_push (SLang_Object_Type *x)
{
   return push_object (x);
}

/* _INLINE_ */
int SLclass_push_ptr_obj (SLtype type, VOID_STAR pval)
{
   register SLang_Object_Type *y;
   y = Stack_Pointer;

   IF_UNLIKELY(y >= Stack_Pointer_Max)
     {
	(void) SLang_set_error (SL_STACK_OVERFLOW);
	return -1;
     }

   y->o_data_type = type;
   y->v.ptr_val = pval;

   Stack_Pointer = y + 1;
   return 0;
}

_INLINE_ static int push_int_object (SLtype type, int x)
{
   register SLang_Object_Type *y;
   y = Stack_Pointer;

   IF_UNLIKELY(y >= Stack_Pointer_Max)
     {
	(void) SLang_set_error (SL_STACK_OVERFLOW);
	return -1;
     }

   y->o_data_type = type;
   y->v.int_val = x;

   Stack_Pointer = y + 1;
   return 0;
}

#if (SLANG_ARRAY_INDEX_TYPE == SLANG_INT_TYPE)
# define push_array_index push_int_object
#else
_INLINE_ static int push_array_index (SLtype type, SLindex_Type x)
{
   register SLang_Object_Type *y;
   y = Stack_Pointer;

   IF_UNLIKELY(y >= Stack_Pointer_Max)
     {
	(void) SLang_set_error (SL_STACK_OVERFLOW);
	return -1;
     }

   y->o_data_type = type;
   y->v.index_val = x;

   Stack_Pointer = y + 1;
   return 0;
}
#endif

int SLclass_push_int_obj (SLtype type, int x)
{
   return push_int_object (type, x);
}

int _pSLang_push_array (SLang_Array_Type *at, int free_array)
{
   register SLang_Object_Type *y;

   y = Stack_Pointer;

   IF_UNLIKELY(y >= Stack_Pointer_Max)
     {
	(void) SLang_set_error (SL_STACK_OVERFLOW);
	if (free_array) SLang_free_array (at);
	return -1;
     }

   if (free_array == 0) at->num_refs++;
   y->o_data_type = SLANG_ARRAY_TYPE;
   y->v.ptr_val = (VOID_STAR)at;

   Stack_Pointer = y + 1;
   return 0;
}

#if SLANG_HAS_FLOAT
_INLINE_ static int push_double_object (SLtype type, double x)
{
   register SLang_Object_Type *y;
   y = Stack_Pointer;

   IF_UNLIKELY(y >= Stack_Pointer_Max)
     {
	(void) SLang_set_error (SL_STACK_OVERFLOW);
	return -1;
     }

   y->o_data_type = type;
   y->v.double_val = x;

   Stack_Pointer = y + 1;
   return 0;
}

int SLclass_push_double_obj (SLtype type, double x)
{
   return push_double_object (type, x);
}
#endif

_INLINE_ static int push_char_object (SLtype type, char x)
{
   register SLang_Object_Type *y;
   y = Stack_Pointer;

   IF_UNLIKELY(y >= Stack_Pointer_Max)
     {
	(void) SLang_set_error (SL_STACK_OVERFLOW);
	return -1;
     }

   y->o_data_type = type;
   y->v.char_val = x;

   Stack_Pointer = y + 1;
   return 0;
}

int SLclass_push_char_obj (SLtype type, char x)
{
   return push_char_object (type, x);
}

/* This function is "fragile".  It is a helper routine and assumes that y is on the stack */
static int _typecast_object_to_type (SLang_Object_Type *y, SLang_Object_Type *obj, SLtype type, int allow_arrays)
{
#if SLANG_OPTIMIZE_FOR_SPEED
   /* This is an implicit typecast.  We do not want to typecast
    * floats to ints implicitly.
    */
   if (IS_ARITH_TYPE(type)
       && IS_ARITH_TYPE(y->o_data_type)
       && (type >= y->o_data_type))
     {
	/* This should not fail */
	(void) _pSLarith_typecast (y->o_data_type, (VOID_STAR)&y->v, 1,
				   type, (VOID_STAR)&obj->v);
	obj->o_data_type = type;
	return 0;
     }
#endif
   if ((allow_arrays == 0)
       || (y->o_data_type != SLANG_ARRAY_TYPE)
       || (y->v.array_val->data_type != type))
     if (-1 == SLclass_typecast (type, 1, 0))
       return -1;

   /* Here, *y has been replaced by the object of the specified type */
   *obj = *y;
   return 0;
}

_INLINE_
  static int pop_int (int *i)
{
   SLang_Object_Type *y;
   SLang_Object_Type obj;

   y = Stack_Pointer;
   IF_UNLIKELY(y == Run_Stack)
     return SLang_pop(&obj);	       /* let it fail */
   y--;
   if (y->o_data_type == SLANG_INT_TYPE)
     {
	*i = y->v.int_val;
	Stack_Pointer = y;
	return 0;
     }
   if (-1 == _typecast_object_to_type (y, &obj, SLANG_INT_TYPE, 0))
     {
	Stack_Pointer = y;
	return -1;
     }
   *i = obj.v.int_val;
   Stack_Pointer = y;
   return 0;
}

int SLang_pop_int (int *i)
{
   return pop_int (i);
}

int SLang_pop_array_index (SLindex_Type *i)
{
   SLang_Object_Type *y;
   SLang_Object_Type obj;

   y = Stack_Pointer;
   if (y == Run_Stack)
     return SLang_pop (&obj);	       /* let it fail */
   y--;
   if (y->o_data_type == SLANG_ARRAY_INDEX_TYPE)
     {
	*i = y->v.index_val;
	Stack_Pointer = y;
	return 0;
     }
   if (-1 == _typecast_object_to_type (y, &obj, SLANG_ARRAY_INDEX_TYPE, 0))
     {
	Stack_Pointer = y;
	return -1;
     }
   *i = obj.v.index_val;
   Stack_Pointer = y;
   return 0;
}

int SLang_push_array_index (SLindex_Type i)
{
   return push_array_index (SLANG_ARRAY_INDEX_TYPE, i);
}

_INLINE_ static int pop_object_of_type (SLtype type, SLang_Object_Type *obj,
					int allow_arrays)
{
   register SLang_Object_Type *y;

   y = Stack_Pointer;
   IF_UNLIKELY(y == Run_Stack)
     return SLang_pop(obj);	       /* let it fail */
   y--;
   if (y->o_data_type == type)
     {
	*obj = *y;
	Stack_Pointer = y;
	return 0;
     }
   if (-1 == _typecast_object_to_type (y, obj, type, allow_arrays))
     {
	Stack_Pointer = y;
	return -1;
     }
   Stack_Pointer = y;
   return 0;
}

int SLclass_pop_ptr_obj (SLtype type, VOID_STAR *s)
{
   SLang_Object_Type obj;

   if (-1 == pop_object_of_type (type, &obj, 0))
     {
	*s = (VOID_STAR) NULL;
	return -1;
     }
   *s = obj.v.ptr_val;
   return 0;
}

int _pSLang_pop_object_of_type (SLtype type, SLang_Object_Type *obj,
			       int allow_arrays)
{
   return pop_object_of_type (type, obj, allow_arrays);
}

/*  This function reverses the top n items on the stack and returns a
 *  an offset from the start of the stack to the last item.
 */
int SLreverse_stack (int n)
{
   SLang_Object_Type *otop, *obot, tmp;

   otop = Stack_Pointer;
   if ((n > otop - Run_Stack) || (n < 0))
     {
	(void) SLang_set_error (SL_STACK_UNDERFLOW);
	return -1;
     }
   obot = otop - n;
   otop--;
   while (otop > obot)
     {
	tmp = *obot;
	*obot = *otop;
	*otop = tmp;
	otop--;
	obot++;
     }
   return (int) ((Stack_Pointer - n) - Run_Stack);
}

/* _INLINE_ */
static int roll_stack (int np)
{
   int n, i;
   SLang_Object_Type *otop, *obot, tmp;

   if ((n = abs(np)) <= 1) return 0;    /* identity */

   obot = otop = Stack_Pointer;
   i = n;
   while (i != 0)
     {
	if (obot <= Run_Stack)
	  {
	     (void) SLang_set_error (SL_STACK_UNDERFLOW);
	     return -1;
	  }
	obot--;
	i--;
     }
   otop--;

   if (np > 0)
     {
	/* Put top on bottom and roll rest up. */
	tmp = *otop;
	while (otop > obot)
	  {
	     *otop = *(otop - 1);
	     otop--;
	  }
	*otop = tmp;
     }
   else
     {
	/* Put bottom on top and roll rest down. */
	tmp = *obot;
	while (obot < otop)
	  {
	     *obot = *(obot + 1);
	     obot++;
	  }
	*obot = tmp;
     }
   return 0;
}

int SLroll_stack (int np)
{
   return roll_stack (np);
}

int SLstack_depth (void)
{
   return (int) (Stack_Pointer - Run_Stack);
}

int SLdup_n (int n)
{
   SLang_Object_Type *bot, *top;

   if (n <= 0)
     return 0;

   top = Stack_Pointer;
   if (top < Run_Stack + n)
     {
	(void) SLang_set_error (SL_STACK_UNDERFLOW);
	return -1;
     }
   if (top + n > Stack_Pointer_Max)
     {
	(void) SLang_set_error (SL_STACK_OVERFLOW);
	return -1;
     }
   bot = top - n;

   while (bot < top)
     {
	SLang_Class_Type *cl;
	SLtype data_type = bot->o_data_type;

#if SLANG_OPTIMIZE_FOR_SPEED
	if (SLANG_CLASS_TYPE_SCALAR == GET_CLASS_TYPE(data_type))
	  {
	     *Stack_Pointer++ = *bot++;
	     continue;
	  }
#endif
	GET_CLASS(cl,data_type);
	if (-1 == (*cl->cl_push) (data_type, (VOID_STAR) &bot->v))
	  return -1;
	bot++;
     }
   return 0;
}

/*}}}*/

/*{{{ inner interpreter and support functions */

_INLINE_
int _pSL_increment_frame_pointer (void)
{
   IF_UNLIKELY(Recursion_Depth >= SLANG_MAX_RECURSIVE_DEPTH)
     {
#if SLANG_HAS_QUALIFIERS
	if (Next_Function_Qualifiers != NULL)
	  {
	     SLang_free_struct (Next_Function_Qualifiers);
	     Next_Function_Qualifiers = NULL;
	  }
#endif
	_pSLang_verror (SL_STACK_OVERFLOW, "Num Args Stack Overflow");
	return -1;
     }
   Num_Args_Stack [Recursion_Depth] = SLang_Num_Function_Args;
   SLang_Num_Function_Args = Next_Function_Num_Args;
   Next_Function_Num_Args = 0;
#if SLANG_HAS_QUALIFIERS
   Function_Qualifiers_Stack[Recursion_Depth] = Function_Qualifiers;
   Function_Qualifiers = Next_Function_Qualifiers;
   Next_Function_Qualifiers = NULL;
#endif
   Recursion_Depth++;
   return 0;
}

_INLINE_
int _pSL_decrement_frame_pointer (void)
{
#if SLANG_HAS_QUALIFIERS
   if (Function_Qualifiers != NULL)
     {
	SLang_free_struct (Function_Qualifiers);
	Function_Qualifiers = NULL;
     }
#endif
   IF_UNLIKELY(Recursion_Depth == 0)
     {
	_pSLang_verror (SL_STACK_UNDERFLOW, "Num Args Stack Underflow");
	return -1;
     }

   Recursion_Depth--;
   if (Recursion_Depth < SLANG_MAX_RECURSIVE_DEPTH)
     {
	SLang_Num_Function_Args = Num_Args_Stack [Recursion_Depth];
#if SLANG_HAS_QUALIFIERS
	Function_Qualifiers = Function_Qualifiers_Stack[Recursion_Depth];
#endif
     }
   return 0;
}

static int decrement_slang_frame_pointer (void)
{
   Function_Stack_Type *s;

   if (-1 == _pSL_decrement_frame_pointer ())
     return -1;

   Function_Stack_Ptr--;
   s = Function_Stack_Ptr;
   Current_Function = s->function;
   Current_Function_Header = s->header;
   This_Compile_Linenum = s->line;
   return 0;
}

static int increment_slang_frame_pointer (_pSLang_Function_Type *fun, unsigned int linenum)
{
   Function_Stack_Type *s;

   if (-1 == _pSL_increment_frame_pointer ())
     return -1;

   /* No need to check for stack underflow/overflow errors here since
    * this stack is the same size as the "frame pointer stack".
    */
   s = Function_Stack_Ptr++;
   s->function = Current_Function;
   s->header = Current_Function_Header;
   s->local_variable_frame = Local_Variable_Frame;
   s->line = linenum;
   if (Current_Function_Header != NULL)
     {
	s->file = Current_Function_Header->file;
	s->static_ns = Current_Function_Header->static_ns;
	s->private_ns = Current_Function_Header->private_ns;
     }
   else
     {
	s->file = This_Compile_Filename;
	s->static_ns = This_Static_NameSpace;
	s->private_ns = This_Private_NameSpace;
     }
   if (fun == NULL)
     return 0;			       /* called from SLexecute_function */

   if (fun->header == NULL)
     {
	if (fun->autoload_ns == NULL)
	  {
	     if (-1 == SLang_load_file(fun->autoload_file))
	       {
		  (void) decrement_slang_frame_pointer ();
		  return -1;
	       }
	  }
	else if (-1 == SLns_load_file (fun->autoload_file, fun->autoload_ns->namespace_name))
	  {
	     (void) decrement_slang_frame_pointer ();
	     return -1;
	  }

	if (NULL == fun->header)
	  {
	     _pSLang_verror (SL_UNDEFINED_NAME, "%s: Function did not autoload",
			   fun->name);
             (void) decrement_slang_frame_pointer ();
	     return -1;
	  }
     }
   Current_Function = fun;
   Current_Function_Header = fun->header;
   return 0;
}

#if SLANG_HAS_QUALIFIERS
static int set_qualifier (void)
{
   if (SLANG_NULL_TYPE == peek_at_stack ())
     {
	Next_Function_Qualifiers = NULL;
	return SLang_pop_null ();
     }
   return SLang_pop_struct (&Next_Function_Qualifiers);
}

/* This function is called from slang code */
int _pSLang_get_qualifiers_intrin (SLang_Struct_Type **qp)
{
   /* The assumption is that this is being called from a function one level up.
    * Grab the qualifiers from the previous frame stack.
    */
   if (Recursion_Depth > 1)
     *qp = Function_Qualifiers_Stack[Recursion_Depth-1];
   else
     *qp = NULL;

   return 0;
}

/* This may be called from intrinsic functions */
int _pSLang_qualifier_exists (SLCONST char *name)
{
   if (Function_Qualifiers == NULL)
     return 0;

   return (NULL != _pSLstruct_get_field_value (Function_Qualifiers, name));
}

int _pSLang_get_int_qualifier (SLCONST char *name, int *p, int def)
{
   SLang_Object_Type *objp;

   if ((Function_Qualifiers == NULL)
       || (NULL == (objp = _pSLstruct_get_field_value (Function_Qualifiers, name))))
     {
	*p = def;
	return 0;
     }
   if (objp->o_data_type == SLANG_INT_TYPE)
     {
	*p = objp->v.int_val;
	return 0;
     }
   if ((-1 == _pSLpush_slang_obj (objp))
       || (-1 == pop_int (p)))
     {
	SLang_verror (0, "Expecting '%s' qualifier to be an integer", name);
	return -1;
     }
   return 0;
}

int _pSLang_get_string_qualifier (SLCONST char *name, char **p, SLFUTURE_CONST char *def)
{
   SLang_Object_Type *objp;

   if ((Function_Qualifiers == NULL)
       || (NULL == (objp = _pSLstruct_get_field_value (Function_Qualifiers, name))))
     {
	if (def == NULL)
	  {
	     *p = NULL;
	     return 0;
	  }

	if (NULL == (*p = SLang_create_slstring (def)))
	  return -1;

	return 0;
     }

   if (objp->o_data_type == SLANG_STRING_TYPE)
     {
	if (NULL == (*p = SLang_create_slstring (objp->v.s_val)))
	  return -1;
	return 0;
     }

   if ((-1 == _pSLpush_slang_obj (objp))
       || (-1 == SLang_pop_slstring (p)))
     {
	SLang_verror (0, "Expecting '%s' qualifier to be a string", name);
	return -1;
     }
   return 0;
}
#endif

_INLINE_
static int start_arg_list (void)
{
   IF_LIKELY(Frame_Pointer_Depth < SLANG_MAX_RECURSIVE_DEPTH)
     {
	Frame_Pointer_Stack [Frame_Pointer_Depth] = (unsigned int) (Frame_Pointer - Run_Stack);
	Frame_Pointer = Stack_Pointer;
	Frame_Pointer_Depth++;
	Next_Function_Num_Args = 0;
	return 0;
     }

   _pSLang_verror (SL_STACK_OVERFLOW, "Frame Stack Overflow");
   return -1;
}

int SLang_start_arg_list (void)
{
   return start_arg_list ();
}

int _pSLang_restart_arg_list (int nargs)
{
   if (Frame_Pointer_Depth < SLANG_MAX_RECURSIVE_DEPTH)
     {
	if ((nargs < 0) || (Run_Stack + nargs > Stack_Pointer))
	  {
	     _pSLang_verror (SL_Internal_Error, "restart_arg_list: stack underflow");
	     return -1;
	  }
	Frame_Pointer_Stack [Frame_Pointer_Depth] = (unsigned int) (Frame_Pointer - Run_Stack);
	Frame_Pointer = Stack_Pointer - nargs;
	Frame_Pointer_Depth++;
	Next_Function_Num_Args = 0;
	return 0;
     }

   _pSLang_verror (SL_STACK_OVERFLOW, "Frame Stack Overflow");
   return -1;
}

_INLINE_ static int end_arg_list (void)
{
   IF_UNLIKELY(Frame_Pointer_Depth == 0)
     {
	_pSLang_verror (SL_STACK_UNDERFLOW, "Frame Stack Underflow");
	return -1;
     }
   Frame_Pointer_Depth--;
   if (Frame_Pointer_Depth < SLANG_MAX_RECURSIVE_DEPTH)
     {
	Next_Function_Num_Args = (int) (Stack_Pointer - Frame_Pointer);
	Frame_Pointer = Run_Stack + Frame_Pointer_Stack [Frame_Pointer_Depth];
     }
   return 0;
}

int SLang_end_arg_list (void)
{
   return end_arg_list ();
}

_INLINE_
static int do_bc_call_direct_frame (int (*f)(void))
{
   if ((0 == end_arg_list ())
       && (0 == _pSL_increment_frame_pointer ()))
     {
	(void) (*f) ();
	_pSL_decrement_frame_pointer ();
     }
   if (IS_SLANG_ERROR)
     return -1;
   return 0;
}

_INLINE_
static int do_bc_call_direct_nargs (int (*f)(void))
{
   if (0 == end_arg_list ())
     {
	int nargs = SLang_Num_Function_Args;

	SLang_Num_Function_Args = Next_Function_Num_Args;
	Next_Function_Num_Args = 0;
	(void) (*f) ();
	SLang_Num_Function_Args = nargs;
     }
   if (IS_SLANG_ERROR)
     return -1;
   return 0;
}

static int do_name_type_error (SLang_Name_Type *nt)
{
   char buf[256];
   if (nt != NULL)
     {
	(void) _pSLsnprintf (buf, sizeof (buf), "(Error occurred processing %s)", nt->name);
	do_traceback (buf);
     }
   return -1;
}

/* local and global variable assignments */

static int do_binary_ab (int op, SLang_Object_Type *obja, SLang_Object_Type *objb)
{
   SLang_Class_Type *a_cl, *b_cl, *c_cl;
   SLtype b_data_type, a_data_type, c_data_type;
   int (*binary_fun) (int,
		      SLtype, VOID_STAR, SLuindex_Type,
		      SLtype, VOID_STAR, SLuindex_Type,
		      VOID_STAR);
   VOID_STAR pa;
   VOID_STAR pb;
   VOID_STAR pc;
   int ret;

   b_data_type = objb->o_data_type;
   a_data_type = obja->o_data_type;

#if SLANG_OPTIMIZE_FOR_SPEED
   if (IS_ARITH_TYPE(a_data_type)
       && IS_ARITH_TYPE(b_data_type))
     {
	int status;
	status = _pSLarith_bin_op (obja, objb, op);
	if (status != 1)
	  return status;
	/* drop and try it the hard way */
     }

   if (a_data_type == b_data_type)
     {
	if (a_data_type == SLANG_ARRAY_TYPE)
	  return _pSLarray_bin_op (obja, objb, op);

	if ((a_data_type == SLANG_STRING_TYPE)
	    && (op == SLANG_PLUS))
	  {
	     char *stra = obja->v.s_val, *strb = objb->v.s_val, *strc;

	     strc = SLang_concat_slstrings (stra, strb);
	     if (strc == NULL)
	       return -1;

	     return _pSLang_push_slstring (strc);   /* frees strc */
	  }
     }
#endif

   GET_CLASS(a_cl,a_data_type);
   if (a_data_type == b_data_type)
     b_cl = a_cl;
   else
     GET_CLASS(b_cl, b_data_type);

   if (NULL == (binary_fun = _pSLclass_get_binary_fun (op, a_cl, b_cl, &c_cl, 1)))
     return -1;

   c_data_type = c_cl->cl_data_type;

#if SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_VECTOR == a_cl->cl_class_type)
     pa = (VOID_STAR) obja->v.ptr_val;
   else
     pa = (VOID_STAR) &obja->v;
#else
   pa = _pSLclass_get_ptr_to_value (a_cl, obja);
#endif

#if SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_VECTOR == b_cl->cl_class_type)
     pb = (VOID_STAR) objb->v.ptr_val;
   else
     pb = (VOID_STAR) &objb->v;
#else
   pb = _pSLclass_get_ptr_to_value (b_cl, objb);
#endif

   pc = c_cl->cl_transfer_buf;

   if (1 != (*binary_fun) (op,
			   a_data_type, pa, 1,
			   b_data_type, pb, 1,
			   pc))
     {
	_pSLang_verror (SL_NOT_IMPLEMENTED,
		      "Binary operation between %s and %s failed",
		      a_cl->cl_name, b_cl->cl_name);

	return -1;
     }

   /* apush will create a copy, so make sure we free after the push */
   ret = (*c_cl->cl_apush)(c_data_type, pc);
#if SLANG_OPTIMIZE_FOR_SPEED
   if ((SLANG_CLASS_TYPE_SCALAR != c_cl->cl_class_type)
       && (SLANG_CLASS_TYPE_VECTOR != c_cl->cl_class_type))
#endif
     (*c_cl->cl_adestroy)(c_data_type, pc);

   return ret;
}

#if SLANG_OPTIMIZE_FOR_SPEED
static int int_int_binary_result (int op, SLang_Object_Type *obja, SLang_Object_Type *objb, SLang_Object_Type *objc)
{
   int a, b;

   a = obja->v.int_val; b = objb->v.int_val;
   switch (op)
     {
      case SLANG_PLUS:
	objc->v.int_val = a + b;  objc->o_data_type = SLANG_INT_TYPE;
	return 0;
      case SLANG_MINUS:
	objc->v.int_val = a - b;  objc->o_data_type = SLANG_INT_TYPE;
	return 0;
      case SLANG_TIMES:
	objc->v.int_val = a * b;  objc->o_data_type = SLANG_INT_TYPE;
	return 0;
      case SLANG_DIVIDE:
	if (b == 0)
	  {
	     SLang_set_error (SL_DIVIDE_ERROR);
	     return -1;
	  }
	objc->v.int_val = a/b;  objc->o_data_type = SLANG_INT_TYPE;
	return 0;
      case SLANG_MOD:
	if (b == 0)
	  {
	     SLang_set_error (SL_DIVIDE_ERROR);
	     return -1;
	  }
	objc->v.int_val = a % b;  objc->o_data_type = SLANG_INT_TYPE;
	return 0;

      case SLANG_BAND:
	objc->v.int_val = (a & b); objc->o_data_type = SLANG_INT_TYPE;
	return 0;
      case SLANG_BXOR:
	objc->v.int_val = (a ^ b); objc->o_data_type = SLANG_INT_TYPE;
	return 0;
      case SLANG_BOR:
	objc->v.int_val = (a | b); objc->o_data_type = SLANG_INT_TYPE;
	return 0;
      case SLANG_SHL:
	objc->v.int_val = (a << b); objc->o_data_type = SLANG_INT_TYPE;
	return 0;
      case SLANG_SHR:
	objc->v.int_val = (a >> b); objc->o_data_type = SLANG_INT_TYPE;
	return 0;

      case SLANG_EQ:
	objc->v.char_val = (a == b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_NE:
	objc->v.char_val = (a != b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_GT:
	objc->v.char_val = (a > b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_GE:
	objc->v.char_val = (a >= b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_LT:
	objc->v.char_val = (a < b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_LE:
	objc->v.char_val = (a <= b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
     }
   if (-1 == do_binary_ab (op, obja, objb))
     return -1;

   return pop_object (objc);
}

static int int_int_binary (int op, SLang_Object_Type *obja, SLang_Object_Type *objb)
{
   int a = obja->v.int_val;
   int b = objb->v.int_val;

   switch (op)
     {
      case SLANG_PLUS:
	return push_int_object (SLANG_INT_TYPE, a + b);
      case SLANG_MINUS:
	return push_int_object (SLANG_INT_TYPE, a - b);
      case SLANG_TIMES:
	return push_int_object (SLANG_INT_TYPE, a * b);
      case SLANG_DIVIDE:
	if (b == 0)
	  {
	     SLang_set_error (SL_DIVIDE_ERROR);
	     return -1;
	  }
	return push_int_object (SLANG_INT_TYPE, a/b);
      case SLANG_MOD:
	if (b == 0)
	  {
	     SLang_set_error (SL_DIVIDE_ERROR);
	     return -1;
	  }
	return push_int_object (SLANG_INT_TYPE, a%b);

      case SLANG_BAND:
	return push_int_object (SLANG_INT_TYPE, a&b);
      case SLANG_BXOR:
	return push_int_object (SLANG_INT_TYPE, a^b);
      case SLANG_BOR:
	return push_int_object (SLANG_INT_TYPE, a|b);
      case SLANG_SHL:
	return push_int_object (SLANG_INT_TYPE, a<<b);
      case SLANG_SHR:
	return push_int_object (SLANG_INT_TYPE, a>>b);

      case SLANG_EQ:
	return push_char_object (SLANG_CHAR_TYPE, a == b);
      case SLANG_NE:
	return push_char_object (SLANG_CHAR_TYPE, a != b);
      case SLANG_GT:
	return push_char_object (SLANG_CHAR_TYPE, a > b);
      case SLANG_GE:
	return push_char_object (SLANG_CHAR_TYPE, a >= b);
      case SLANG_LT:
	return push_char_object (SLANG_CHAR_TYPE, a < b);
      case SLANG_LE:
	return push_char_object (SLANG_CHAR_TYPE, a <= b);
     }
   return do_binary_ab (op, obja, objb);
}

#if SLANG_HAS_FLOAT
static int dbl_dbl_binary (int op, SLang_Object_Type *obja, SLang_Object_Type *objb)
{
   switch (op)
     {
      case SLANG_PLUS:
	return push_double_object (SLANG_DOUBLE_TYPE, obja->v.double_val + objb->v.double_val);
      case SLANG_MINUS:
	return push_double_object (SLANG_DOUBLE_TYPE, obja->v.double_val - objb->v.double_val);
      case SLANG_TIMES:
	return push_double_object (SLANG_DOUBLE_TYPE, obja->v.double_val * objb->v.double_val);
      case SLANG_DIVIDE:
	return push_double_object (SLANG_DOUBLE_TYPE, obja->v.double_val / objb->v.double_val);
      case SLANG_EQ:
	return push_char_object (SLANG_CHAR_TYPE, obja->v.double_val == objb->v.double_val);
      case SLANG_NE:
	return push_char_object (SLANG_CHAR_TYPE, obja->v.double_val != objb->v.double_val);
      case SLANG_GT:
	return push_char_object (SLANG_CHAR_TYPE, obja->v.double_val > objb->v.double_val);
      case SLANG_GE:
	return push_char_object (SLANG_CHAR_TYPE, obja->v.double_val >= objb->v.double_val);
      case SLANG_LT:
	return push_char_object (SLANG_CHAR_TYPE, obja->v.double_val < objb->v.double_val);
      case SLANG_LE:
	return push_char_object (SLANG_CHAR_TYPE, obja->v.double_val <= objb->v.double_val);
      case SLANG_POW:
	return push_double_object (SLANG_DOUBLE_TYPE, pow(obja->v.double_val, objb->v.double_val));
     }
   return do_binary_ab (op, obja, objb);
}

static int int_dbl_binary_result (int op, SLang_Object_Type *obja, SLang_Object_Type *objb, SLang_Object_Type *objc)
{
   int a = obja->v.int_val;
   double b = objb->v.double_val;

   switch (op)
     {
      case SLANG_PLUS:
	objc->v.double_val = a + b;  objc->o_data_type = SLANG_DOUBLE_TYPE;
	return 0;
      case SLANG_MINUS:
	objc->v.double_val = a - b;  objc->o_data_type = SLANG_DOUBLE_TYPE;
	return 0;
      case SLANG_TIMES:
	objc->v.double_val = a * b;  objc->o_data_type = SLANG_DOUBLE_TYPE;
	return 0;
      case SLANG_DIVIDE:
	objc->v.double_val = a / b;  objc->o_data_type = SLANG_DOUBLE_TYPE;
	return 0;
      case SLANG_EQ:
	objc->v.char_val = (a == b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_NE:
	objc->v.char_val = (a != b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_GT:
	objc->v.char_val = (a > b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_GE:
	objc->v.char_val = (a >= b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_LT:
	objc->v.char_val = (a < b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_LE:
	objc->v.char_val = (a <= b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_POW:
	objc->v.double_val = pow(a,b);  objc->o_data_type = SLANG_DOUBLE_TYPE;
	return 0;
     }
   if (-1 == do_binary_ab (op, obja, objb))
     return -1;

   return pop_object (objc);
}

static int dbl_int_binary_result (int op, SLang_Object_Type *obja, SLang_Object_Type *objb, SLang_Object_Type *objc)
{
   double a = obja->v.double_val;
   int b = objb->v.int_val;

   switch (op)
     {
      case SLANG_PLUS:
	objc->v.double_val = a + b;  objc->o_data_type = SLANG_DOUBLE_TYPE;
	return 0;
      case SLANG_MINUS:
	objc->v.double_val = a - b;  objc->o_data_type = SLANG_DOUBLE_TYPE;
	return 0;
      case SLANG_TIMES:
	objc->v.double_val = a * b;  objc->o_data_type = SLANG_DOUBLE_TYPE;
	return 0;
      case SLANG_DIVIDE:
	objc->v.double_val = a / b;  objc->o_data_type = SLANG_DOUBLE_TYPE;
	return 0;
      case SLANG_EQ:
	objc->v.char_val = (a == b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_NE:
	objc->v.char_val = (a != b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_GT:
	objc->v.char_val = (a > b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_GE:
	objc->v.char_val = (a >= b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_LT:
	objc->v.char_val = (a < b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_LE:
	objc->v.char_val = (a <= b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_POW:
	objc->v.double_val = pow(a,b);  objc->o_data_type = SLANG_DOUBLE_TYPE;
	return 0;
     }
   if (-1 == do_binary_ab (op, obja, objb))
     return -1;

   return pop_object (objc);
}

static int dbl_dbl_binary_result (int op, SLang_Object_Type *obja, SLang_Object_Type *objb, SLang_Object_Type *objc)
{
   double a = obja->v.double_val;
   double b = objb->v.double_val;

   switch (op)
     {
      case SLANG_PLUS:
	objc->v.double_val = a + b;  objc->o_data_type = SLANG_DOUBLE_TYPE;
	return 0;
      case SLANG_MINUS:
	objc->v.double_val = a - b;  objc->o_data_type = SLANG_DOUBLE_TYPE;
	return 0;
      case SLANG_TIMES:
	objc->v.double_val = a * b;  objc->o_data_type = SLANG_DOUBLE_TYPE;
	return 0;
      case SLANG_DIVIDE:
	objc->v.double_val = a / b;  objc->o_data_type = SLANG_DOUBLE_TYPE;
	return 0;
      case SLANG_EQ:
	objc->v.char_val = (a == b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_NE:
	objc->v.char_val = (a != b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_GT:
	objc->v.char_val = (a > b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_GE:
	objc->v.char_val = (a >= b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_LT:
	objc->v.char_val = (a < b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_LE:
	objc->v.char_val = (a <= b);  objc->o_data_type = SLANG_CHAR_TYPE;
	return 0;
      case SLANG_POW:
	objc->v.double_val = pow(a,b);  objc->o_data_type = SLANG_DOUBLE_TYPE;
	return 0;
     }
   if (-1 == do_binary_ab (op, obja, objb))
     return -1;

   return pop_object (objc);
}
#endif				       /* SLANG_HAS_FLOAT */
#endif				       /* SLANG_OPTIMIZE_FOR_SPEED */

int _pSLang_do_binary_ab (int op, SLang_Object_Type *obja, SLang_Object_Type *objb)
{
#if SLANG_OPTIMIZE_FOR_SPEED
   if (obja->o_data_type == objb->o_data_type)
     {
	if (obja->o_data_type == SLANG_INT_TYPE)
	  return int_int_binary (op, obja, objb);
#if SLANG_HAS_FLOAT
	if (obja->o_data_type == SLANG_DOUBLE_TYPE)
	  return dbl_dbl_binary (op, obja, objb);
#endif
     }
#endif
   return do_binary_ab (op, obja, objb);
}

#define INC_REF(cl,type,addr,dr) \
   if (cl->cl_inc_ref != NULL) (*cl->cl_inc_ref)(type,addr,dr)

/* _INLINE_ */
static int do_binary_ab_inc_ref (int op, SLang_Object_Type *obja, SLang_Object_Type *objb)
{
   int ret;
   SLang_Class_Type *cl_a, *cl_b;
   SLtype atype = obja->o_data_type;
   SLtype btype = objb->o_data_type;

   if (atype == SLANG_INT_TYPE)
     {
	if (btype == SLANG_INT_TYPE)
	  {
	     int a, b;

	     a = obja->v.int_val; b = objb->v.int_val;
	     switch (op)
	       {
		case SLANG_PLUS:
		  return push_int_object (SLANG_INT_TYPE, a+b);
		case SLANG_MINUS:
		  return push_int_object (SLANG_INT_TYPE, a-b);
		case SLANG_TIMES:
		  return push_int_object (SLANG_INT_TYPE, a*b);
		case SLANG_DIVIDE:
		  if (b == 0)
		    {
		       SLang_set_error (SL_DIVIDE_ERROR);
		       return -1;
		    }
		  return push_int_object (SLANG_INT_TYPE, a/b);
		case SLANG_MOD:
		  if (b == 0)
		    {
		       SLang_set_error (SL_DIVIDE_ERROR);
		       return -1;
		    }
		  return push_int_object (SLANG_INT_TYPE, a%b);

		case SLANG_BAND:
		  return push_int_object (SLANG_INT_TYPE, a&b);
		case SLANG_BXOR:
		  return push_int_object (SLANG_INT_TYPE, a^b);
		case SLANG_BOR:
		  return push_int_object (SLANG_INT_TYPE, a|b);
		case SLANG_SHL:
		  return push_int_object (SLANG_INT_TYPE, a<<b);
		case SLANG_SHR:
		  return push_int_object (SLANG_INT_TYPE, a>>b);

		case SLANG_EQ:
		  return push_char_object (SLANG_CHAR_TYPE, a==b);
		case SLANG_NE:
		  return push_char_object (SLANG_CHAR_TYPE, a!=b);
		case SLANG_GT:
		  return push_char_object (SLANG_CHAR_TYPE, a>b);
		case SLANG_GE:
		  return push_char_object (SLANG_CHAR_TYPE, a>=b);
		case SLANG_LT:
		  return push_char_object (SLANG_CHAR_TYPE, a<b);
		case SLANG_LE:
		  return push_char_object (SLANG_CHAR_TYPE, a<=b);
	       }
	     return do_binary_ab (op, obja, objb);
	  }
#if SLANG_HAS_FLOAT
	if (btype == SLANG_DOUBLE_TYPE)
	  {
	     int a;
	     double b;

	     a = obja->v.int_val; b = objb->v.double_val;
	     switch (op)
	       {
		case SLANG_PLUS:
		  return push_double_object (SLANG_DOUBLE_TYPE, a+b);
		case SLANG_MINUS:
		  return push_double_object (SLANG_DOUBLE_TYPE, a-b);
		case SLANG_TIMES:
		  return push_double_object (SLANG_DOUBLE_TYPE, a*b);
		case SLANG_DIVIDE:
		  return push_double_object (SLANG_DOUBLE_TYPE, a/b);
		case SLANG_EQ:
		  return push_char_object (SLANG_CHAR_TYPE, a==b);
		case SLANG_NE:
		  return push_char_object (SLANG_CHAR_TYPE, a!=b);
		case SLANG_GT:
		  return push_char_object (SLANG_CHAR_TYPE, a>b);
		case SLANG_GE:
		  return push_char_object (SLANG_CHAR_TYPE, a>=b);
		case SLANG_LT:
		  return push_char_object (SLANG_CHAR_TYPE, a<b);
		case SLANG_LE:
		  return push_char_object (SLANG_CHAR_TYPE, a<=b);
		case SLANG_POW:
		  return push_double_object (SLANG_DOUBLE_TYPE, pow(a, b));
	       }
	     return do_binary_ab (op, obja, objb);
	  }
#endif				       /* SLANG_HAS_FLOAT */
     }
#if SLANG_HAS_FLOAT
   else if (atype == SLANG_DOUBLE_TYPE)
     {
	double a, b;

	if (btype == SLANG_DOUBLE_TYPE)
	  b = objb->v.double_val;
	else if (btype == SLANG_INT_TYPE)
	  b = (double) objb->v.int_val;
	else
	  goto the_hard_way;

	a = obja->v.double_val;
	switch (op)
	  {
	   case SLANG_PLUS:
	     return push_double_object (SLANG_DOUBLE_TYPE, a+b);
	   case SLANG_MINUS:
	     return push_double_object (SLANG_DOUBLE_TYPE, a-b);
	   case SLANG_TIMES:
	     return push_double_object (SLANG_DOUBLE_TYPE, a*b);
	   case SLANG_DIVIDE:
	     return push_double_object (SLANG_DOUBLE_TYPE, a/b);
	   case SLANG_EQ:
	     return push_char_object (SLANG_CHAR_TYPE, a==b);
	   case SLANG_NE:
	     return push_char_object (SLANG_CHAR_TYPE, a!=b);
	   case SLANG_GT:
	     return push_char_object (SLANG_CHAR_TYPE, a>b);
	   case SLANG_GE:
	     return push_char_object (SLANG_CHAR_TYPE, a>=b);
	   case SLANG_LT:
	     return push_char_object (SLANG_CHAR_TYPE, a<b);
	   case SLANG_LE:
	     return push_char_object (SLANG_CHAR_TYPE, a<=b);
	   case SLANG_POW:
	     return push_double_object (SLANG_DOUBLE_TYPE, pow(a, b));
	  }
	return do_binary_ab (op, obja, objb);
     }
#endif				       /* SLANG_HAS_FLOAT */

the_hard_way:

   GET_CLASS(cl_a, atype);
   GET_CLASS(cl_b, btype);
   INC_REF(cl_a, atype, &obja->v, 1);
   INC_REF(cl_b, btype, &objb->v, 1);
   ret = do_binary_ab (op, obja, objb);
   INC_REF(cl_a, atype, &obja->v, -1);
   INC_REF(cl_b, btype, &objb->v, -1);

   return ret;
}

#if SLANG_OPTIMIZE_FOR_SPEED
/* Only for SLANG_BCST_ASSIGN */
/* _INLINE_ */
static int do_binary_ab_inc_ref_assign (int op, SLang_Object_Type *obja, SLang_Object_Type *objb, SLang_Object_Type *objc)
{
   int ret;
   SLang_Class_Type *cl, *cl_a, *cl_b;
   int c_needs_freed;
   SLtype atype, btype;

   GET_CLASS(cl, objc->o_data_type);
   c_needs_freed = (SLANG_CLASS_TYPE_SCALAR != cl->cl_class_type);

   atype = obja->o_data_type;
   btype = objb->o_data_type;

   if (atype == SLANG_INT_TYPE)
     {
	if (btype == SLANG_INT_TYPE)
	  {
	     if (c_needs_freed)
	       {
		  ret = int_int_binary (op, obja, objb);
		  goto the_return;
	       }
	     return int_int_binary_result (op, obja, objb, objc);
	  }
#if SLANG_HAS_FLOAT
	if (btype == SLANG_DOUBLE_TYPE)
	  {
	     if (c_needs_freed)
	       {
		  ret = do_binary_ab (op, obja, objb);
		  goto the_return;
	       }
	     return int_dbl_binary_result (op, obja, objb, objc);
	  }
#endif				       /* SLANG_HAS_FLOAT */
     }
#if SLANG_HAS_FLOAT
   else if (atype == SLANG_DOUBLE_TYPE)
     {
	if (btype == SLANG_DOUBLE_TYPE)
	  {
	     if (c_needs_freed)
	       {
		  ret = dbl_dbl_binary (op, obja, objb);
		  goto the_return;
	       }
	     return dbl_dbl_binary_result (op, obja, objb, objc);
	  }

	if (btype == SLANG_INT_TYPE)
	  {
	     if (c_needs_freed)
	       {
		  ret = do_binary_ab (op, obja, objb);
		  goto the_return;
	       }

	     return dbl_int_binary_result (op, obja, objb, objc);
	  }
     }
#endif				       /* SLANG_HAS_FLOAT */

   GET_CLASS(cl_a, atype);
   GET_CLASS(cl_b, btype);
   INC_REF(cl_a, atype, &obja->v, 1);
   INC_REF(cl_b, btype, &objb->v, 1);
   ret = do_binary_ab (op, obja, objb);
   INC_REF(cl_b, btype, &objb->v, -1);
   INC_REF(cl_a, atype, &obja->v, -1);

   the_return:

   if (ret != 0)
     return ret;

   if (c_needs_freed)
     free_object (objc, cl);

   return pop_object(objc);
}
#endif

/* _INLINE_ */
static int do_binary (int op)
{
   SLang_Object_Type obja, objb, *objap, *objbp;
#if SLANG_OPTIMIZE_FOR_SPEED
   SLang_Class_Type *cl;
#endif
   int ret;

   objbp = Stack_Pointer;
   if (Run_Stack + 2 > objbp)
     {
	(void) SLang_set_error (SL_STACK_UNDERFLOW);
	SLdo_pop_n (objbp - Run_Stack);
	return -1;
     }
   objbp--;
   objap = objbp-1;

   Stack_Pointer = objap;
#if SLANG_OPTIMIZE_FOR_SPEED
   if (objbp->o_data_type == objap->o_data_type)
     {
	if (objbp->o_data_type == SLANG_INT_TYPE)
	  return int_int_binary (op, objap, objbp);
#if SLANG_HAS_FLOAT
	if (objbp->o_data_type == SLANG_DOUBLE_TYPE)
	  return dbl_dbl_binary (op, objap, objbp);
#endif
     }
#endif

   obja = *objap;
   objb = *objbp;

   ret = do_binary_ab (op, &obja, &objb);
#if SLANG_OPTIMIZE_FOR_SPEED
   GET_CLASS(cl, obja.o_data_type);
   if (SLANG_CLASS_TYPE_SCALAR != cl->cl_class_type)
     free_object (&obja, cl);
#else
     SLang_free_object (&obja);
#endif

#if SLANG_OPTIMIZE_FOR_SPEED
   if (obja.o_data_type != objb.o_data_type)
     {
	GET_CLASS(cl, objb.o_data_type);
     }
   if (SLANG_CLASS_TYPE_SCALAR != cl->cl_class_type)
     free_object (&objb, cl);
#else
     SLang_free_object (&objb);
#endif

   return ret;
}

_INLINE_
static int do_binary_b (int op, SLang_Object_Type *bp)
{
   SLang_Object_Type a;
#if SLANG_OPTIMIZE_FOR_SPEED
   SLang_Class_Type *cl;
#endif
   int ret;

   if (pop_object(&a)) return -1;
#if SLANG_OPTIMIZE_FOR_SPEED
   if (a.o_data_type == bp->o_data_type)
     {
	if (a.o_data_type == SLANG_INT_TYPE)
	  return int_int_binary (op, &a, bp);

#if SLANG_HAS_FLOAT
	if (a.o_data_type == SLANG_DOUBLE_TYPE)
	  return dbl_dbl_binary (op, &a, bp);
#endif
     }
#endif
   ret = do_binary_ab (op, &a, bp);
#if SLANG_OPTIMIZE_FOR_SPEED
   GET_CLASS(cl, a.o_data_type);
   if (SLANG_CLASS_TYPE_SCALAR != cl->cl_class_type)
     free_object (&a, cl);
#else
     SLang_free_object (&a);
#endif
   return ret;
}

#if SLANG_OPTIMIZE_FOR_SPEED
/* _INLINE_ */
static void do_binary_b_inc_ref (int op, SLang_Object_Type *objbp)
{
   SLang_Object_Type obja;
   SLang_Class_Type *cl_a, *cl_b;
   SLang_Object_Type *objap;
   SLtype atype, btype;

   btype = objbp->o_data_type;

   if (btype == SLANG_INT_TYPE)
     {
	if (Stack_Pointer == Run_Stack)
	  {
	     (void) SLang_set_error (SL_STACK_UNDERFLOW);
	     return;
	  }
	objap = (Stack_Pointer-1);
	atype = objap->o_data_type;

	if (atype == SLANG_INT_TYPE)
	  {
	     int a, b;

	     a = objap->v.int_val; b = objbp->v.int_val;
	     switch (op)
	       {
		case SLANG_PLUS:
		  objap->v.int_val = a + b; return;
		case SLANG_MINUS:
		  objap->v.int_val = a - b; return;
		case SLANG_TIMES:
		  objap->v.int_val = a * b; return;
		case SLANG_EQ:
		  objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a == b); return;
		case SLANG_NE:
		  objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a != b); return;
		case SLANG_GT:
		  objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a > b); return;
		case SLANG_GE:
		  objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a >= b); return;
		case SLANG_LT:
		  objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a < b); return;
		case SLANG_LE:
		  objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a <= b); return;
	       }
	     (void) pop_object (&obja);
	     do_binary_ab (op, &obja, objbp);
	     return;
	  }
#if SLANG_HAS_FLOAT
	if (atype == SLANG_DOUBLE_TYPE)
	  {
	     double a;
	     int b;

	     a = objap->v.double_val; b = objbp->v.int_val;
	     switch (op)
	       {
		case SLANG_PLUS:
		  objap->v.double_val = a + b; return;
		case SLANG_MINUS:
		  objap->v.double_val = a - b; return;
		case SLANG_TIMES:
		  objap->v.double_val = a * b; return;
		case SLANG_DIVIDE:
		  objap->v.double_val = a / b; return;
		case SLANG_EQ:
		  objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a == b); return;
		case SLANG_NE:
		  objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a != b); return;
		case SLANG_GT:
		  objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a > b); return;
		case SLANG_GE:
		  objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a >= b); return;
		case SLANG_LT:
		  objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a < b); return;
		case SLANG_LE:
		  objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a <= b); return;
		case SLANG_POW:
		  objap->v.double_val = pow(a,b); return;
	       }
	     (void) pop_object (&obja);
	     do_binary_ab (op, &obja, objbp);
	     return;
	  }
#endif				       /* SLANG_HAS_FLOAT */
     }
#if SLANG_HAS_FLOAT
   else if (btype == SLANG_DOUBLE_TYPE)
     {
	double a, b;

	if (Stack_Pointer == Run_Stack)
	  {
	     (void) SLang_set_error (SL_STACK_UNDERFLOW);
	     return;
	  }
	objap = (Stack_Pointer-1);
	atype = objap->o_data_type;

	if (atype == SLANG_DOUBLE_TYPE)
	  a = objap->v.double_val;
	else if (atype == SLANG_INT_TYPE)
	  a = (double) objap->v.int_val;
	else
	  goto the_hard_way;

	b = objbp->v.double_val;
	switch (op)
	  {
	   case SLANG_PLUS:
	     objap->o_data_type = SLANG_DOUBLE_TYPE; objap->v.double_val = a + b; return;
	   case SLANG_MINUS:
	     objap->o_data_type = SLANG_DOUBLE_TYPE; objap->v.double_val = a - b; return;
	   case SLANG_TIMES:
	     objap->o_data_type = SLANG_DOUBLE_TYPE; objap->v.double_val = a * b; return;
	   case SLANG_DIVIDE:
	     objap->o_data_type = SLANG_DOUBLE_TYPE; objap->v.double_val = a / b; return;
	   case SLANG_EQ:
	     objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a == b); return;
	   case SLANG_NE:
	     objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a != b); return;
	   case SLANG_GT:
	     objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a > b); return;
	   case SLANG_GE:
	     objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a >= b); return;
	   case SLANG_LT:
	     objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a < b); return;
	   case SLANG_LE:
	     objap->o_data_type = SLANG_CHAR_TYPE; objap->v.char_val = (a <= b); return;
	   case SLANG_POW:
	     objap->o_data_type = SLANG_DOUBLE_TYPE; objap->v.double_val = pow(a,b); return;
	  }
	(void) pop_object (&obja);
	do_binary_ab (op, &obja, objbp);
	return;
     }
#endif				       /* SLANG_HAS_FLOAT */

the_hard_way:

   if (-1 == pop_object (&obja))
     return;

   GET_CLASS(cl_a, obja.o_data_type);
   GET_CLASS(cl_b, btype);

   INC_REF(cl_b, btype, &objbp->v, 1);
   (void) do_binary_ab (op, &obja, objbp);
   INC_REF(cl_b, btype, &objbp->v, -1);

   if (SLANG_CLASS_TYPE_SCALAR != cl_a->cl_class_type)
     free_object (&obja, cl_a);
}
#endif

static int do_unary_op (int op, SLang_Object_Type *obj, int unary_type)
{
   int (*f) (int, SLtype, VOID_STAR, SLuindex_Type, VOID_STAR);
   VOID_STAR pa;
   VOID_STAR pb;
   SLang_Class_Type *a_cl, *b_cl;
   SLtype a_type, b_type;
   int ret;

   a_type = obj->o_data_type;
   GET_CLASS (a_cl, a_type);

   if (NULL == (f = _pSLclass_get_unary_fun (op, a_cl, &b_cl, unary_type)))
     return -1;

   b_type = b_cl->cl_data_type;

#if SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_VECTOR == a_cl->cl_class_type)
     pa = (VOID_STAR) obj->v.ptr_val;
   else
     pa = (VOID_STAR) &obj->v;
#else
   pa = _pSLclass_get_ptr_to_value (a_cl, obj);
#endif
   pb = b_cl->cl_transfer_buf;

   if (1 != (*f) (op, a_type, pa, 1, pb))
     {
	_pSLang_verror (SL_NOT_IMPLEMENTED,
		      "Unary operation/function for %s failed", a_cl->cl_name);
	return -1;
     }

   ret = (*b_cl->cl_apush)(b_type, pb);
   /* cl_apush creates a copy, so make sure we call cl_adestroy */
#if SLANG_OPTIMIZE_FOR_SPEED
   if ((SLANG_CLASS_TYPE_SCALAR != b_cl->cl_class_type)
       && (SLANG_CLASS_TYPE_VECTOR != b_cl->cl_class_type))
#endif
     (*b_cl->cl_adestroy)(b_type, pb);

   return ret;
}

_INLINE_
static int do_unary (int op, int unary_type)
{
   SLang_Object_Type obj;
#if SLANG_OPTIMIZE_FOR_SPEED
   SLang_Class_Type *cl;
#endif
   int ret;

   if (-1 == pop_object(&obj)) return -1;
   ret = do_unary_op (op, &obj, unary_type);
#if SLANG_OPTIMIZE_FOR_SPEED
   GET_CLASS(cl, obj.o_data_type);
   if (SLANG_CLASS_TYPE_SCALAR != cl->cl_data_type)
     free_object (&obj, cl);
#else
   SLang_free_object (&obj);
#endif
   return ret;
}

static int do_assignment_binary (int op, SLang_Object_Type *obja_ptr)
{
   SLang_Object_Type objb;
#if SLANG_OPTIMIZE_FOR_SPEED
   SLtype btype;
   SLang_Class_Type *cl;
#endif
   int ret;

   if (pop_object(&objb))
     return -1;
#if SLANG_OPTIMIZE_FOR_SPEED
   btype = objb.o_data_type;
#endif

#if 0 && SLANG_OPTIMIZE_FOR_SPEED
   if (op == SLANG_PLUS)
     {
	if (obja_ptr->o_data_type == SLANG_BSTRING_TYPE)
	  {
	     if (btype == SLANG_BSTRING_TYPE)
	       {
		  ret = _pSLbstring_concat_bstr (obja_ptr, (SLang_BString_Type*)objb.v.ptr_val);
		  SLbstring_free ((SLang_BString_Type *) objb.v.ptr_val);
		  return ret;
	       }
	     if (btype == SLANG_STRING_TYPE)
	       {
		  ret = _pSLbstring_concat_str (obja_ptr, objb.v.s_val);
		  _pSLang_free_slstring (objb.v.s_val);
		  return ret;
	       }
	  }
     }
#endif

   ret = do_binary_ab (op, obja_ptr, &objb);
#if SLANG_OPTIMIZE_FOR_SPEED
   GET_CLASS(cl, btype);
   if (SLANG_CLASS_TYPE_SCALAR != cl->cl_class_type)
     free_object (&objb, cl);
#else
   SLang_free_object (&objb);
#endif
   return ret;
}

/* The order of these is assumed to match the binary operators
 * defined in slang.h
 */
static int
map_assignment_op_to_binary (int op_type, int *op, int *is_unary)
{
   *is_unary = 0;
   switch (op_type)
     {
      case SLANG_BCST_PLUSEQS:
      case SLANG_BCST_MINUSEQS:
      case SLANG_BCST_TIMESEQS:
      case SLANG_BCST_DIVEQS:
	*op = SLANG_PLUS + (op_type - SLANG_BCST_PLUSEQS);
	break;

      case SLANG_BCST_BOREQS:
	*op = SLANG_BOR;
	break;

      case SLANG_BCST_BANDEQS:
	*op = SLANG_BAND;
	break;

      case SLANG_BCST_POST_MINUSMINUS:
      case SLANG_BCST_MINUSMINUS:
	*op = SLANG_MINUS;
	*is_unary = 1;
	break;

      case SLANG_BCST_PLUSPLUS:
      case SLANG_BCST_POST_PLUSPLUS:
	*op = SLANG_PLUS;
	*is_unary = 1;
	break;

      default:
	_pSLang_verror (SL_NOT_IMPLEMENTED, "Assignment operator not implemented");
	return -1;
     }
   return 0;
}

static int
perform_lvalue_operation (int op_type, SLang_Object_Type *obja_ptr)
{
   switch (op_type)
     {
      case SLANG_BCST_ASSIGN:
	break;

	/* The order of these is assumed to match the binary operators
	 * defined in slang.h
	 */
      case SLANG_BCST_PLUSEQS:
      case SLANG_BCST_MINUSEQS:
      case SLANG_BCST_TIMESEQS:
      case SLANG_BCST_DIVEQS:
	if (-1 == do_assignment_binary (SLANG_PLUS + (op_type - SLANG_BCST_PLUSEQS), obja_ptr))
	  return -1;
	break;

      case SLANG_BCST_BOREQS:
	if (-1 == do_assignment_binary (SLANG_BOR, obja_ptr))
	  return -1;
	break;

      case SLANG_BCST_BANDEQS:
	if (-1 == do_assignment_binary (SLANG_BAND, obja_ptr))
	  return -1;
	break;

      case SLANG_BCST_PLUSPLUS:
      case SLANG_BCST_POST_PLUSPLUS:
#if SLANG_OPTIMIZE_FOR_SPEED
	if (obja_ptr->o_data_type == SLANG_INT_TYPE)
	  return push_int_object (SLANG_INT_TYPE, obja_ptr->v.int_val + 1);
#endif
	if (-1 == do_unary_op (SLANG_PLUSPLUS, obja_ptr, SLANG_BC_UNARY))
	  return -1;
	break;

      case SLANG_BCST_MINUSMINUS:
      case SLANG_BCST_POST_MINUSMINUS:
#if SLANG_OPTIMIZE_FOR_SPEED
	if (obja_ptr->o_data_type == SLANG_INT_TYPE)
	  return push_int_object (SLANG_INT_TYPE, obja_ptr->v.int_val - 1);
#endif
	if (-1 == do_unary_op (SLANG_MINUSMINUS, obja_ptr, SLANG_BC_UNARY))
	  return -1;
	break;

      default:
	(void) SLang_set_error (SL_INTERNAL_ERROR);
	return -1;
     }
   return 0;
}

_INLINE_
static int
set_lvalue_obj (int op_type, SLang_Object_Type *obja_ptr)
{
#if SLANG_OPTIMIZE_FOR_SPEED
   SLang_Class_Type *cl;
#endif
   if (op_type != SLANG_BCST_ASSIGN)
     {
	if (-1 == perform_lvalue_operation (op_type, obja_ptr))
	  return -1;
     }
#if SLANG_OPTIMIZE_FOR_SPEED
   GET_CLASS(cl, obja_ptr->o_data_type);
   if (SLANG_CLASS_TYPE_SCALAR != cl->cl_class_type)
     free_object (obja_ptr, cl);
#else
   SLang_free_object (obja_ptr);
#endif

   return pop_object(obja_ptr);
}

/* a = b; a += b; ... */
_INLINE_
static int
set_lvalue_obj_with_obj (int op_type, SLang_Object_Type *obja_ptr, SLang_Object_Type *objb_ptr)
{
   SLang_Class_Type *cl;

   if (op_type != SLANG_BCST_ASSIGN)
     {
	if (-1 == _pSLpush_slang_obj (objb_ptr))
	  return -1;
	if (-1 == perform_lvalue_operation (op_type, obja_ptr))
	  return -1;

	goto pop_method;
     }

   if (SLANG_CLASS_TYPE_SCALAR == GET_CLASS_TYPE(objb_ptr->o_data_type))
     {
	/* We can copy b to a */
#if SLANG_OPTIMIZE_FOR_SPEED
	GET_CLASS(cl, obja_ptr->o_data_type);
	if (SLANG_CLASS_TYPE_SCALAR != cl->cl_class_type)
	  free_object (obja_ptr, cl);
#else
	SLang_free_object (obja_ptr);
#endif
	*obja_ptr = *objb_ptr;
	return 0;
     }

   /* a and b could refer to the same object.  So push b first before freeing a */

   GET_CLASS(cl, objb_ptr->o_data_type);
   if (-1 == (*cl->cl_push)(objb_ptr->o_data_type, (VOID_STAR)&objb_ptr->v))
     return -1;

pop_method:

#if SLANG_OPTIMIZE_FOR_SPEED
   GET_CLASS(cl, obja_ptr->o_data_type);
   if (SLANG_CLASS_TYPE_SCALAR != cl->cl_class_type)
     free_object (obja_ptr, cl);
#else
   SLang_free_object (obja_ptr);
#endif

   return pop_object(obja_ptr);
}

/* A.x = stackobj;  A.x += stackobj, etc... */
static int
set_struct_obj_lvalue (SLBlock_Type *bc_blk, SLang_Object_Type *objA, int do_free)
{
   SLtype type;
   SLang_Class_Type *cl;
   SLFUTURE_CONST char *name;
   int op, ret;

   type = objA->o_data_type;

   GET_CLASS(cl,type);
   if ((cl->cl_sput == NULL)
       || (cl->cl_sget == NULL))
     {
	_pSLang_verror (SL_NOT_IMPLEMENTED,
		      "%s does not support structure access",
		      cl->cl_name);
	if (do_free)
	  free_object (objA, cl);
	return -1;
     }
   name = bc_blk->b.s_blk;
   op = bc_blk->bc_sub_type;

   if (op != SLANG_BCST_ASSIGN)
     {
	/* We have something like (A.x += b) or (A.x++).  In either case,
	 * we need A.x.
	 */
	SLang_Object_Type obj;
#if SLANG_USE_TMP_OPTIMIZATION
	SLang_Class_Type *cl_obj;
#endif
	if (cl->is_struct)
	  {
	     if ((-1 == _pSLstruct_push_field (objA->v.struct_val, name, 0))
		 || (-1 == pop_object(&obj)))
	       {
		  if (do_free) free_object (objA, cl);
		  return -1;
	       }
	  }
	else if ((-1 == _pSLpush_slang_obj (objA))
		 || (-1 == cl->cl_sget ((SLtype) type, name))
		 || (-1 == pop_object(&obj)))
	  {
	     if (do_free) free_object (objA, cl);
	     return -1;
	  }

	/* Now the value of A.x is in obj. */
#if SLANG_USE_TMP_OPTIMIZATION
	/*
	 * It has at least 2 references: A.x and obj.  Decrement its reference
	 * to allow for the possibility of __tmp optimization.
	 */
	GET_CLASS(cl_obj,obj.o_data_type);
	INC_REF(cl_obj, obj.o_data_type, &obj.v, -1);
#endif
	ret = perform_lvalue_operation (op, &obj);
#if SLANG_USE_TMP_OPTIMIZATION
	INC_REF(cl_obj, obj.o_data_type, &obj.v, 1);
#endif
	if (ret == -1)
	  {
	     SLang_free_object (&obj);
	     if (do_free) free_object (objA, cl);
	     return -1;
	  }
	free_object (&obj, cl_obj);
     }

   /* The result of the operation is now on the stack.
    * Perform assignment
    */
   if (cl->is_struct)
     {
	ret = _pSLstruct_pop_field (objA->v.struct_val, name, 0);
	if (do_free) free_object (objA, cl);
	return ret;
     }

   if (-1 == _pSLpush_slang_obj (objA))
     {
	if (do_free) free_object (objA, cl);
	return -1;
     }

   ret = (*cl->cl_sput) ((SLtype) type, name);
   if (do_free) free_object (objA, cl);
   return ret;
}

/* A.x = stack, A.x += stack, ...  A is also on the stack. */
static int set_struct_lvalue (SLBlock_Type *bc_blk)
{
   SLang_Object_Type objA;

   if (-1 == pop_object (&objA))
     return -1;

   return set_struct_obj_lvalue (bc_blk, &objA, 1);
}

/* handle: @x op y
 *         @x++, @x--
 */
static int set_deref_lvalue (int op)
{
   int ret;
   SLang_Object_Type x;
   SLang_Ref_Type *ref;

   if (-1 == SLang_pop_ref (&ref))
     return -1;

   if (op == SLANG_BCST_ASSIGN)
     {
	ret = _pSLang_deref_assign (ref);
	SLang_free_ref (ref);
	return ret;
     }

   ret = -1;
   if ((0 == _pSLang_dereference_ref (ref))
       && (0 == pop_object(&x)))
     {
	if (0 == perform_lvalue_operation (op, &x))
	  ret = _pSLang_deref_assign (ref);

	SLang_free_object (&x);
     }

   SLang_free_ref (ref);
   return ret;
}

static int make_unit_object (SLang_Object_Type *a, SLang_Object_Type *u)
{
   SLtype type;

   type = a->o_data_type;
   if (type == SLANG_ARRAY_TYPE)
     type = a->v.array_val->data_type;

   u->o_data_type = type;
   switch (type)
     {
      case SLANG_UCHAR_TYPE:
      case SLANG_CHAR_TYPE:
	u->v.char_val = 1;
	break;

      case SLANG_SHORT_TYPE:
      case SLANG_USHORT_TYPE:
	u->v.short_val = 1;
	break;

      case SLANG_LONG_TYPE:
      case SLANG_ULONG_TYPE:
	u->v.long_val = 1;
	break;

#if SLANG_HAS_FLOAT
      case SLANG_FLOAT_TYPE:
	u->v.float_val = 1;
	break;

      case SLANG_COMPLEX_TYPE:
	u->o_data_type = SLANG_DOUBLE_TYPE;
      case SLANG_DOUBLE_TYPE:
	u->v.double_val = 1;
	break;
#endif
      default:
	u->o_data_type = SLANG_INT_TYPE;
	u->v.int_val = 1;
     }
   return 0;
}

/* We want to convert 'A[i] op X' to 'A[i] = A[i] op X'.  The code that
 * has been generated is:  X __args i A __aput-op
 * where __aput-op represents this function.  We need to generate:
 * __args i A __eargs __aget X op __args i A __eargs __aput
 * Here, __eargs implies a call to do_bc_call_direct_nargs with either
 * the aput or aget function.  In addition, __args represents a call to
 * start_arg_list.  Of course, i represents a set of indices.
 *
 * Note: If op is an unary operation (e.g., ++ or --), then X will not
 * be present an will have to be taken to be 1.
 *
 * Implementation note: For efficiency, calls to setup the frame, start
 * arg list will be omitted and SLang_Num_Function_Args will be set.
 * This is ugly but the alternative is much less efficient rendering these
 * assignment operators useless.  So, the plan is to roll the stack to get X,
 * then duplicate the next N values, call __aget followed by op X, finally
 * calling __aput.  Hence, the sequence is:
 *
 *     start:   X i .. j A
 *      dupN:   X i .. j A i .. j A
 *    __aget:   X i .. j A Y
 *      roll:   i .. j A Y X
 *        op:   i .. j A Z
 *      roll:   Z i .. j A
 *    __aput:
 */
static int
set_array_lvalue (int op)
{
   SLang_Object_Type x, y;
   int is_unary;
   int status;
#if SLANG_OPTIMIZE_FOR_SPEED
   int class_type;
   SLang_Class_Type *cl;
#endif
   int num_args;

   if (-1 == map_assignment_op_to_binary (op, &op, &is_unary))
     return -1;

   /* Grab the indices and the array.  Do not start a new frame. */
   if (-1 == end_arg_list ())
     return -1;
   if ((num_args = Next_Function_Num_Args) <= 0)
     {
	SLang_verror (SL_INTERNAL_ERROR, "set_array_lvalue: Next_Function_Num_Args<=0");
	return -1;
     }
   Next_Function_Num_Args = 0;

   if (is_unary)		       /* PLUSPLUS, MINUSMINUS */
     {
	int type = peek_at_stack ();
	if (type == SLANG_ASSOC_TYPE)
	  {
	     return _pSLassoc_inc_value (num_args-1,
					 (op == SLANG_PLUS) ? +1 : -1);
	  }
     }

   if (-1 == SLdup_n (num_args))
     return -1;

   if (-1 == _pSLarray_aget1 (num_args-1))
     return -1;

   if (-1 == pop_object(&y))
     return -1;

   if (is_unary == 0)
     {
	if ((-1 == roll_stack (-(num_args + 1)))
	    || (-1 == pop_object(&x)))
	  {
	     SLang_free_object (&y);
	     return -1;
	  }
     }
   else if (-1 == make_unit_object (&y, &x))
     {
	SLang_free_object (&y);
	return -1;
     }
#if SLANG_OPTIMIZE_FOR_SPEED
   if (x.o_data_type == y.o_data_type)
     {
	if (x.o_data_type == SLANG_INT_TYPE)
	  status = int_int_binary (op, &y, &x);
#if SLANG_HAS_FLOAT
	else if (x.o_data_type == SLANG_DOUBLE_TYPE)
	  status = dbl_dbl_binary (op, &y, &x);
#endif
	else status = do_binary_ab (op, &y, &x);
     }
   else
#endif
     status = do_binary_ab (op, &y, &x);

   if (status != 0)
     {
	SLang_free_object (&y);
	SLang_free_object (&x);
	return -1;
     }

#if SLANG_OPTIMIZE_FOR_SPEED
   GET_CLASS(cl, y.o_data_type);
   class_type = cl->cl_class_type;
   if (SLANG_CLASS_TYPE_SCALAR != class_type)
     free_object (&y, cl);
#else
   SLang_free_object (&y);
#endif

#if SLANG_OPTIMIZE_FOR_SPEED
   if (cl->cl_data_type != x.o_data_type)
     {
	GET_CLASS(cl, x.o_data_type);
	class_type = cl->cl_class_type;
     }
   if (SLANG_CLASS_TYPE_SCALAR != class_type)
     free_object (&x, cl);
#else
   SLang_free_object (&x);
#endif

   if (-1 == roll_stack (num_args + 1))
     return -1;

   return _pSLarray_aput1 (num_args-1);
}

static int
set_intrin_lvalue (SLBlock_Type *bc_blk)
{
   int op_type;
   SLang_Object_Type obja;
   SLang_Class_Type *cl;
   SLang_Intrin_Var_Type *ivar;
   VOID_STAR intrinsic_addr;
   SLtype intrinsic_type;

   ivar = bc_blk->b.nt_ivar_blk;

   intrinsic_type = ivar->type;
   intrinsic_addr = ivar->addr;

   op_type = bc_blk->bc_sub_type;

   GET_CLASS(cl, intrinsic_type);

   if (op_type != SLANG_BCST_ASSIGN)
     {
	/* We want to get the current value into obja.  This is the
	 * easiest way.
	 */
	if ((-1 == (*cl->cl_push) (intrinsic_type, intrinsic_addr))
	    || (-1 == pop_object(&obja)))
	  return -1;

	(void) perform_lvalue_operation (op_type, &obja);
	SLang_free_object (&obja);

	if (IS_SLANG_ERROR)
	  return -1;
     }

   return (*cl->cl_pop) (intrinsic_type, intrinsic_addr);
}

static int push_intrinsic_variable (SLang_Intrin_Var_Type *ivar)
{
   SLang_Class_Type *cl;
   SLtype stype;

   stype = ivar->type;
   GET_CLASS(cl,stype);

   if (-1 == (*cl->cl_push_intrinsic) (stype, ivar->addr))
     {
	do_name_type_error ((SLang_Name_Type *) ivar);
	return -1;
     }
   return 0;
}

static int push_nametype_variable (SLang_Name_Type *nt)
{
   switch (nt->name_type)
     {
      case SLANG_PVARIABLE:
      case SLANG_GVARIABLE:
	return _pSLpush_slang_obj (&((SLang_Global_Var_Type *)nt)->obj);

      case SLANG_IVARIABLE:
      case SLANG_RVARIABLE:
	return push_intrinsic_variable ((SLang_Intrin_Var_Type *)nt);

      case SLANG_HCONSTANT:
	return SLclass_push_short_obj (((SLang_HConstant_Type *)nt)->data_type, ((SLang_HConstant_Type*)nt)->value);
      case SLANG_ICONSTANT:
	return push_int_object (((SLang_IConstant_Type *)nt)->data_type, ((SLang_IConstant_Type*)nt)->value);
      case SLANG_LCONSTANT:
	return SLclass_push_long_obj (((SLang_LConstant_Type *)nt)->data_type, ((SLang_LConstant_Type*)nt)->value);

#if SLANG_HAS_FLOAT
      case SLANG_DCONSTANT:
	return push_double_object (SLANG_DOUBLE_TYPE, ((SLang_DConstant_Type*)nt)->d);
      case SLANG_FCONSTANT:
	return SLclass_push_float_obj (SLANG_FLOAT_TYPE, ((SLang_FConstant_Type*)nt)->f);
#endif
#ifdef HAVE_LONG_LONG
      case SLANG_LLCONSTANT:
	return SLclass_push_llong_obj (SLANG_LLONG_TYPE, ((SLang_LLConstant_Type*)nt)->ll);
#endif
     }
   _pSLang_verror (SL_TYPE_MISMATCH, "Symbol %s is not a variable", nt->name);
   return -1;
}

static int set_nametype_variable (SLang_Name_Type *nt)
{
   SLBlock_Type blk;

   switch (nt->name_type)
     {
      case SLANG_GVARIABLE:
      case SLANG_PVARIABLE:
	if (-1 == set_lvalue_obj (SLANG_BCST_ASSIGN,
				  &((SLang_Global_Var_Type *)nt)->obj))
	  {
	     do_name_type_error (nt);
	     return -1;
	  }
	break;

      case SLANG_IVARIABLE:
	blk.b.nt_blk = nt;
	blk.bc_sub_type = SLANG_BCST_ASSIGN;
	if (-1 == set_intrin_lvalue (&blk))
	  {
	     do_name_type_error (nt);
	     return -1;
	  }
	break;

      case SLANG_LVARIABLE:
	(void) SLang_set_error (SL_INTERNAL_ERROR);
	return -1;

      case SLANG_RVARIABLE:
      default:
	_pSLang_verror (SL_READONLY_ERROR, "%s is read-only", nt->name);
	return -1;
     }

   return 0;
}

/* References to Nametype objects */
static char *nt_ref_string (VOID_STAR vdata)
{
   SLang_NameSpace_Type *ns;
   SLang_Name_Type *nt = *(SLang_Name_Type **)vdata;
   SLCONST char *name;
   unsigned int len;
   char *s;

   ns = _pSLns_find_object_namespace (nt);
   if (ns == NULL)
     return NULL;

   name = nt->name;
   len = strlen (name);

   if ((ns->namespace_name != NULL)
       && (0 != strcmp (ns->namespace_name, "Global")))
     {
	unsigned int dlen = strlen (ns->namespace_name);
	s = SLmalloc (len + dlen + 4);
	if (s == NULL)
	  return NULL;
	(void) sprintf (s, "&%s->%s", ns->namespace_name, name);
	return s;
     }

   if (NULL == (s = SLmalloc (len + 2)))
     return NULL;

   *s = '&';
   strcpy (s + 1, name);
   return s;
}

static void nt_ref_destroy (VOID_STAR vdata)
{
   /* SLang_free_function ((SLang_Name_Type *)nt) -- someday */
   (void) vdata;
}

static int nt_ref_deref_assign (VOID_STAR vdata)
{
   return set_nametype_variable (*(SLang_Name_Type **) vdata);
}

static int inner_interp_nametype (SLang_Name_Type *, int);
static int nt_ref_deref (VOID_STAR vdata)
{
   (void) inner_interp_nametype (*(SLang_Name_Type **)vdata, 0);
   return 0;
}

static int nt_ref_is_initialized (VOID_STAR v)
{
   SLang_Name_Type *nt = *(SLang_Name_Type **)v;

   if ((nt->name_type != SLANG_GVARIABLE)
       && (nt->name_type != SLANG_PVARIABLE))
     return 1;

   return ((SLang_Global_Var_Type *)nt)->obj.o_data_type != SLANG_UNDEFINED_TYPE;
}

static int nt_ref_uninitialize (VOID_STAR v)
{
   SLang_Name_Type *nt = *(SLang_Name_Type **)v;
   SLang_Object_Type *obj;

   if ((nt->name_type != SLANG_GVARIABLE)
       && (nt->name_type != SLANG_PVARIABLE))
     return -1;

   obj = &((SLang_Global_Var_Type *)nt)->obj;
   SLang_free_object (obj);
   obj->o_data_type = SLANG_UNDEFINED_TYPE;
   obj->v.ptr_val = NULL;
   return 0;
}

static SLang_Ref_Type *create_ref_to_nametype (SLang_Name_Type *nt)
{
   SLang_Ref_Type *ref;

   if (NULL == (ref = _pSLang_new_ref (sizeof (SLang_Name_Type *))))
     return NULL;

   ref->data_is_nametype = 1;
   *(SLang_Name_Type **)ref->data = nt;
   ref->destroy = nt_ref_destroy;
   ref->string = nt_ref_string;
   ref->deref = nt_ref_deref;
   ref->deref_assign = nt_ref_deref_assign;
   ref->is_initialized = nt_ref_is_initialized;
   ref->uninitialize = nt_ref_uninitialize;
   return ref;
}

int SLang_assign_nametype_to_ref (SLang_Ref_Type *ref, SLang_Name_Type *nt)
{
   SLang_Ref_Type *r;

   if ((nt == NULL) || (ref == NULL))
     return -1;

   if (NULL == (r = create_ref_to_nametype (nt)))
     return -1;

   if (-1 == SLang_assign_to_ref (ref, SLANG_REF_TYPE, (VOID_STAR) &r))
     {
	SLang_free_ref (r);
	return -1;
     }
   SLang_free_ref (r);
   return 0;
}

/* Note: This is ok if nt is NULL.  Some routines rely on this behavior */
int _pSLang_push_nt_as_ref (SLang_Name_Type *nt)
{
   SLang_Ref_Type *r;
   int ret;

   if (nt == NULL)
     return SLang_push_null ();

   r = create_ref_to_nametype (nt);
   if (r == NULL) return -1;

   ret = SLang_push_ref (r);
   SLang_free_ref (r);
   return ret;
}

/* Local variable references */
static SLang_Object_Type *lv_ref_check_object (VOID_STAR vdata)
{
   SLang_Object_Type *obj = *(SLang_Object_Type **)vdata;

   if (obj > Local_Variable_Frame)
     {
	_pSLang_verror (SL_UNDEFINED_NAME, "Local variable reference is out of scope");
	return NULL;
     }
   return obj;
}

static int lv_ref_deref (VOID_STAR vdata)
{
   SLang_Object_Type *obj = lv_ref_check_object (vdata);
   if (obj == NULL)
     return -1;
   return _pSLpush_slang_obj (obj);
}

static int lv_ref_deref_assign (VOID_STAR vdata)
{
   SLang_Object_Type *objp = lv_ref_check_object (vdata);
   if (objp == NULL)
     return -1;
   return set_lvalue_obj (SLANG_BCST_ASSIGN, objp);
}

static void lv_ref_destroy (VOID_STAR vdata)
{
   (void)vdata;
}

static char *lv_ref_string (VOID_STAR vdata)
{
   (void) vdata;
   return SLmake_string ("Local variable reference");
}

static int lv_ref_is_initialized (VOID_STAR v)
{
   SLang_Object_Type *objp = lv_ref_check_object (v);
   if (objp == NULL)
     return -1;

   return objp->o_data_type != SLANG_UNDEFINED_TYPE;
}

static int lv_ref_uninitialize (VOID_STAR v)
{
   SLang_Object_Type *obj = lv_ref_check_object (v);
   if (obj == NULL)
     return -1;

   SLang_free_object (obj);
   obj->o_data_type = SLANG_UNDEFINED_TYPE;
   obj->v.ptr_val = NULL;
   return 0;
}

static SLang_Ref_Type *lv_new_ref (SLang_Object_Type *objp)
{
   SLang_Ref_Type *ref;

   if (NULL == (ref = _pSLang_new_ref (sizeof (SLang_Object_Type *))))
     return NULL;
   *(SLang_Object_Type **)ref->data = objp;
   ref->destroy = lv_ref_destroy;
   ref->string = lv_ref_string;
   ref->deref = lv_ref_deref;
   ref->deref_assign = lv_ref_deref_assign;
   ref->is_initialized = lv_ref_is_initialized;
   ref->uninitialize = lv_ref_uninitialize;
   return ref;
}

static int push_lv_as_ref (SLang_Object_Type *objp)
{
   int ret;
   SLang_Ref_Type *ref = lv_new_ref (objp);

   if (ref == NULL)
     return -1;

   ret = SLang_push_ref (ref);
   SLang_free_ref (ref);
   return ret;
}

#if 0
static void set_deref_lvalue (SLBlock_Type *bc_blk)
{
   SLang_Object_Type *objp;
   SLang_Ref_Type *ref;

   switch (bc_blk->bc_sub_type)
     {
      case SLANG_LVARIABLE:
	objp =  (Local_Variable_Frame - bc_blk->b.i_blk);
	break;
      case SLANG_GVARIABLE:
      case SLANG_PVARIABLE:
	objp = &bc_blk->b.nt_gvar_blk->obj;
	break;
      default:
	(void) SLang_set_error (SL_INTERNAL_ERROR);
	return;
     }

   if (-1 == _pSLpush_slang_obj (objp))
     return;

   if (-1 == SLang_pop_ref (&ref))
     return;
   (void) _pSLang_deref_assign (ref);
   SLang_free_ref (ref);
}
#endif

static int push_struct_field (SLFUTURE_CONST char *name)
{
   SLang_Class_Type *cl;
   SLang_Object_Type obj;
   SLtype type;

   if (-1 == pop_object (&obj))
     return -1;

   if (SLANG_STRUCT_TYPE == (type = obj.o_data_type))
     return _pSLstruct_push_field (obj.v.struct_val, name, 1);

   GET_CLASS(cl, (SLtype) type);

   if (cl->is_struct)
     return _pSLstruct_push_field (obj.v.struct_val, name, 1);

   if (cl->cl_sget == NULL)
     {
	_pSLang_verror (SL_NOT_IMPLEMENTED,
		      "%s does not permit structure access",
		      cl->cl_name);
	free_object (&obj, cl);
	return -1;
     }
   if (-1 == push_object (&obj))
     {
	free_object (&obj, cl);
	return -1;
     }
   return (*cl->cl_sget) ((SLtype) type, name);
}

static int is_nametype_callable (SLang_Name_Type *nt)
{
   switch (nt->name_type)
     {
      case SLANG_PFUNCTION:
      case SLANG_FUNCTION:
      case SLANG_INTRINSIC:
      case SLANG_ARITH_UNARY:
      case SLANG_MATH_UNARY:
      case SLANG_APP_UNARY:
      case SLANG_ARITH_BINARY:
	return 1;
     }
   return 0;
}

int _pSLang_ref_is_callable (SLang_Ref_Type *ref)
{
   if (ref->data_is_nametype == 0)
     return 0;

   return is_nametype_callable (*(SLang_Name_Type **)ref->data);
}

static int inner_interp(register SLBlock_Type *);
static int inner_interp_nametype (SLang_Name_Type *nt, int linenum)
{
   SLBlock_Type bc_blks[2];

   bc_blks[0].b.nt_blk = nt;
   bc_blks[0].bc_main_type = (_pSLang_BC_Type)nt->name_type;
   bc_blks[0].bc_sub_type = 0;
   bc_blks[0].linenum = linenum;
   bc_blks[1].bc_main_type = SLANG_BC_LAST_BLOCK;

   return inner_interp(bc_blks);
}

/* This function also frees the object */
static int deref_call_object (SLang_Object_Type *obj, int linenum)
{
   if (obj->o_data_type == SLANG_REF_TYPE)
     {
	SLang_Ref_Type *ref = (SLang_Ref_Type *)obj->v.ref;
	if ((ref != NULL) && ref->data_is_nametype
	    && is_nametype_callable (*(SLang_Name_Type **)ref->data))
	  {
	     int ret = inner_interp_nametype (*(SLang_Name_Type**)ref->data, linenum);
	     SLang_free_ref (ref);
	     return ret;
	  }
     }
   _pSLang_verror (SL_TYPE_MISMATCH, "Expected a reference to a function");
   SLang_free_object (obj);
   return -1;
}

/*  This arises from code such as a.f(x,y) with the following on the stack:
 *
 *     __args x y a
 *
 *  This function turns this into
 *
 *     __args a x y _eargs (@a.field)
 *
 */
static int do_struct_method (SLFUTURE_CONST char *name, int linenum)
{
   SLang_Object_Type obj;

   if (-1 == SLdup_n (1))
     return -1;			       /* stack: __args x y a a */

   if (-1 == push_struct_field (name))
     return -1;			       /* stack: __args x y a a.name */

   if (-1 == pop_object(&obj))
     return -1;

   if (-1 == end_arg_list ())
     {
	SLang_free_object (&obj);
	return -1;
     }
   /* stack: __args x y a __eargs */
   if (-1 == roll_stack (Next_Function_Num_Args))
     {
	SLang_free_object (&obj);
	return -1;
     }
   return deref_call_object (&obj, linenum);    /* frees obj */
}

static void trace_dump (SLFUTURE_CONST char *format, char *name, SLang_Object_Type *objs, int n, int dir)
{
   unsigned int len;
   char prefix [52];

   len = Trace_Mode - 1;
   if (len + 2 >= sizeof (prefix))
     len = sizeof (prefix) - 2;

   SLMEMSET (prefix, ' ', len);
   prefix[len] = 0;

   _pSLerr_dump_msg ("%s", prefix);
   _pSLerr_dump_msg (format, name, n);

   if (n > 0)
     {
	prefix[len] = ' ';
	len++;
	prefix[len] = 0;

	_pSLerr_dump_msg (prefix, objs, n, dir);
     }
}

/*  Pop a data item from the stack and return a pointer to it.
 *  Strings are not freed from stack so use another routine to do it.
 */
static VOID_STAR pop_pointer (SLang_Object_Type *obj, SLtype type)
{
#ifndef SLANG_OPTIMIZE_FOR_SPEED
   SLang_Class_Type *cl;
#endif

   SLang_Array_Type *at;

   /* Arrays are special.  Allow scalars to automatically convert to arrays.
    */
   if (type == SLANG_ARRAY_TYPE)
     {
	if (-1 == SLang_pop_array (&at, 1))
	  return NULL;
	obj->o_data_type = SLANG_ARRAY_TYPE;
	return obj->v.ptr_val = (VOID_STAR) at;
     }

   if (type == 0)
     {
	/* This happens when an intrinsic is declared without any information
	 * regarding parameter types.
	 */
	if (-1 == pop_object(obj))
	  return NULL;
	type = obj->o_data_type;
     }
   else if (-1 == pop_object_of_type (type, obj, 0))
     return NULL;

   type = GET_CLASS_TYPE(type);

   if (type == SLANG_CLASS_TYPE_SCALAR)
     return (VOID_STAR) &obj->v;
   else if (type == SLANG_CLASS_TYPE_MMT)
     return SLang_object_from_mmt (obj->v.ref);
   else
     return obj->v.ptr_val;
}

/* This is ugly.  Does anyone have a advice for a cleaner way of doing
 * this??
 */
typedef void (*VF0_Type)(void);
typedef void (*VF1_Type)(VOID_STAR);
typedef void (*VF2_Type)(VOID_STAR, VOID_STAR);
typedef void (*VF3_Type)(VOID_STAR, VOID_STAR, VOID_STAR);
typedef void (*VF4_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef void (*VF5_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef void (*VF6_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef void (*VF7_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef long (*LF0_Type)(void);
typedef long (*LF1_Type)(VOID_STAR);
typedef long (*LF2_Type)(VOID_STAR, VOID_STAR);
typedef long (*LF3_Type)(VOID_STAR, VOID_STAR, VOID_STAR);
typedef long (*LF4_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef long (*LF5_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef long (*LF6_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef long (*LF7_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
#if SLANG_HAS_FLOAT
typedef double (*FF0_Type)(void);
typedef double (*FF1_Type)(VOID_STAR);
typedef double (*FF2_Type)(VOID_STAR, VOID_STAR);
typedef double (*FF3_Type)(VOID_STAR, VOID_STAR, VOID_STAR);
typedef double (*FF4_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef double (*FF5_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef double (*FF6_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
typedef double (*FF7_Type)(VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR, VOID_STAR);
#endif

static int execute_intrinsic_fun (SLang_Intrin_Fun_Type *objf)
{
#if SLANG_HAS_FLOAT
   double xf;
#endif
   VOID_STAR p[SLANG_MAX_INTRIN_ARGS];
   SLang_Object_Type objs[SLANG_MAX_INTRIN_ARGS];
   long ret;
   SLtype ret_type;
   unsigned int argc;
   unsigned int i;
   FVOID_STAR fptr;
   SLtype *arg_types;
   int stk_depth;
   int num_args;

   fptr = objf->i_fun;
   argc = objf->num_args;
   ret_type = objf->return_type;
   arg_types = objf->arg_types;

   if (argc > SLANG_MAX_INTRIN_ARGS)
     {
	_pSLang_verror(SL_APPLICATION_ERROR,
		     "Intrinsic function %s requires too many parameters", objf->name);
	return -1;
     }

   if (-1 == _pSL_increment_frame_pointer ())
     return -1;
   num_args = SLang_Num_Function_Args;

   stk_depth = -1;
   if (Trace_Mode && (_pSLang_Trace > 0))
     {
	int nargs;

	stk_depth = SLstack_depth ();

	nargs = SLang_Num_Function_Args;
	if (nargs == 0)
	  nargs = (int)argc;

	stk_depth -= nargs;

	if (stk_depth >= 0)
	  trace_dump (">>%s (%d args)\n",
		      (char *) objf->name,
		      Stack_Pointer - nargs,
		      nargs,
		      1);
     }

   i = argc;
   while (i != 0)
     {
	i--;
	if (NULL == (p[i] = pop_pointer (objs + i, arg_types[i])))
	  {
	     i++;
	     goto free_and_return;
	  }
     }

   ret = 0;
#if SLANG_HAS_FLOAT
   xf = 0.0;
#endif

   switch (argc)
     {
      case 0:
	if (ret_type == SLANG_VOID_TYPE) ((VF0_Type) fptr) ();
#if SLANG_HAS_FLOAT
	else if (ret_type == SLANG_DOUBLE_TYPE) xf = ((FF0_Type) fptr)();
#endif
	else ret = ((LF0_Type) fptr)();
	break;

      case 1:
	if (ret_type == SLANG_VOID_TYPE) ((VF1_Type) fptr)(p[0]);
#if SLANG_HAS_FLOAT
	else if (ret_type == SLANG_DOUBLE_TYPE) xf =  ((FF1_Type) fptr)(p[0]);
#endif
	else ret =  ((LF1_Type) fptr)(p[0]);
	break;

      case 2:
	if (ret_type == SLANG_VOID_TYPE)  ((VF2_Type) fptr)(p[0], p[1]);
#if SLANG_HAS_FLOAT
	else if (ret_type == SLANG_DOUBLE_TYPE) xf = ((FF2_Type) fptr)(p[0], p[1]);
#endif
	else ret = ((LF2_Type) fptr)(p[0], p[1]);
	break;

      case 3:
	if (ret_type == SLANG_VOID_TYPE) ((VF3_Type) fptr)(p[0], p[1], p[2]);
#if SLANG_HAS_FLOAT
	else if (ret_type == SLANG_DOUBLE_TYPE) xf = ((FF3_Type) fptr)(p[0], p[1], p[2]);
#endif
	else ret = ((LF3_Type) fptr)(p[0], p[1], p[2]);
	break;

      case 4:
	if (ret_type == SLANG_VOID_TYPE) ((VF4_Type) fptr)(p[0], p[1], p[2], p[3]);
#if SLANG_HAS_FLOAT
	else if (ret_type == SLANG_DOUBLE_TYPE) xf = ((FF4_Type) fptr)(p[0], p[1], p[2], p[3]);
#endif
	else ret = ((LF4_Type) fptr)(p[0], p[1], p[2], p[3]);
	break;

      case 5:
	if (ret_type == SLANG_VOID_TYPE) ((VF5_Type) fptr)(p[0], p[1], p[2], p[3], p[4]);
#if SLANG_HAS_FLOAT
	else if (ret_type == SLANG_DOUBLE_TYPE) xf = ((FF5_Type) fptr)(p[0], p[1], p[2], p[3], p[4]);
#endif
	else ret = ((LF5_Type) fptr)(p[0], p[1], p[2], p[3], p[4]);
	break;

      case 6:
	if (ret_type == SLANG_VOID_TYPE) ((VF6_Type) fptr)(p[0], p[1], p[2], p[3], p[4], p[5]);
#if SLANG_HAS_FLOAT
	else if (ret_type == SLANG_DOUBLE_TYPE) xf = ((FF6_Type) fptr)(p[0], p[1], p[2], p[3], p[4], p[5]);
#endif
	else ret = ((LF6_Type) fptr)(p[0], p[1], p[2], p[3], p[4], p[5]);
	break;

      case 7:
	if (ret_type == SLANG_VOID_TYPE) ((VF7_Type) fptr)(p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
#if SLANG_HAS_FLOAT
	else if (ret_type == SLANG_DOUBLE_TYPE) xf = ((FF7_Type) fptr)(p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
#endif
	else ret = ((LF7_Type) fptr)(p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
	break;
     }

   switch (ret_type)
     {
      case SLANG_VOID_TYPE:
	break;

#if SLANG_HAS_FLOAT
      case SLANG_DOUBLE_TYPE:
	(void) push_double_object (SLANG_DOUBLE_TYPE, xf);
	break;
#endif
      case SLANG_UINT_TYPE:
      case SLANG_INT_TYPE: (void) push_int_object (ret_type, (int) ret);
	break;

      case SLANG_CHAR_TYPE:
      case SLANG_UCHAR_TYPE: (void) push_char_object (ret_type, (char) ret);
	break;

      case SLANG_SHORT_TYPE:
	(void) SLclass_push_short_obj (_pSLANG_SHORT_TYPE, (short) ret);
	break;
      case SLANG_USHORT_TYPE:
	(void) SLclass_push_short_obj (_pSLANG_USHORT_TYPE, (short) ret);
	break;

      case SLANG_LONG_TYPE:
	(void) SLclass_push_long_obj (_pSLANG_LONG_TYPE, ret);
	break;
      case SLANG_ULONG_TYPE:
	(void) SLclass_push_long_obj (_pSLANG_ULONG_TYPE, ret);
	break;

      case SLANG_STRING_TYPE:
	if (NULL == (char *)ret)
	  (void) SLang_set_error (SL_INTRINSIC_ERROR);
	else (void) SLang_push_string ((char *)ret);
	break;

      default:
	_pSLang_verror (SL_NOT_IMPLEMENTED,
		      "Support for intrinsic functions returning %s is not provided.  Use the appropriate push function.",
		      SLclass_get_datatype_name (ret_type));
     }

   if (stk_depth >= 0)
     {
	stk_depth = SLstack_depth () - stk_depth;

	trace_dump ("<<%s (returning %d values)\n",
		      (char *) objf->name,
		      Stack_Pointer - stk_depth,
		      stk_depth,
		      1);
     }

   free_and_return:
   while (i < argc)
     {
#if SLANG_OPTIMIZE_FOR_SPEED
	SLang_Class_Type *cl;
	SLtype type = objs[i].o_data_type;
	GET_CLASS(cl, type);
	if (SLANG_CLASS_TYPE_SCALAR != cl->cl_class_type)
	  free_object (objs+i, cl);
#else
	SLang_free_object (objs + i);
#endif
	i++;
     }

   if (num_args != SLang_Num_Function_Args)
     SLang_verror (SL_INTERNAL_ERROR, "execute_intrinsic_fun: SLang_Num_Function_Args changed");

   return _pSL_decrement_frame_pointer ();
}

/* Switch_Obj_Ptr points to the NEXT available free switch object */
static SLang_Object_Type Switch_Objects[SLANG_MAX_NESTED_SWITCH];
static SLang_Object_Type *Switch_Obj_Ptr = Switch_Objects;
static SLang_Object_Type *Switch_Obj_Max = Switch_Objects + SLANG_MAX_NESTED_SWITCH;

/* Returns 0 if the loops were completed, 1 if they were terminated via break,
 * or -1 if an error occured.
 */
static int
lang_do_loops (int stype, SLBlock_Type *block, unsigned int num_blocks)
{
   int i, ctrl;
   int first, last;
   SLBlock_Type *blks[4];
   SLCONST char *loop_name;
   SLang_Foreach_Context_Type *foreach_context;
   SLang_Class_Type *cl;
   int type;
   unsigned int j;

   j = 0;
   for (i = 0; i < (int) num_blocks; i++)
     {
	if (block[i].bc_main_type != SLANG_BC_BLOCK)
	  {
#if USE_BC_LINE_NUM
	     if (block[i].bc_main_type == SLANG_BC_LINE_NUM)
	       continue;
#endif
	     _pSLang_verror (SL_SYNTAX_ERROR, "Bytecode is not a looping block");
	     return -1;
	  }
	blks[j] = block[i].b.blk;
	j++;
     }

   num_blocks = j;
   block = blks[0];

   switch (stype)
     {
	int next_fn_args;

      case SLANG_BCST_FOREACH_EARGS:
	if (-1 == end_arg_list ())
	  goto return_error;
	/* drop */
      case SLANG_BCST_FOREACH:	       /* obsolete */
	loop_name = "foreach";
	if (num_blocks != 1)
	  goto wrong_num_blocks_error;

	/* We should find Next_Function_Num_Args + 1 items on the stack.
	 * The first Next_Function_Num_Args items represent the arguments to
	 * to USING.  The last item (deepest in stack) is the object to loop
	 * over.  So, roll the stack up and grab it.
	 */
	next_fn_args = Next_Function_Num_Args;
	Next_Function_Num_Args = 0;
	if ((-1 == roll_stack (-(next_fn_args + 1)))
	    || (-1 == (type = peek_at_stack ())))
	  goto return_error;

	GET_CLASS(cl, (SLtype)type);
	if ((cl->cl_foreach == NULL)
	    || (cl->cl_foreach_open == NULL)
	    || (cl->cl_foreach_close == NULL))
	  {
	     _pSLang_verror (SL_NOT_IMPLEMENTED, "%s does not permit foreach", cl->cl_name);
	     SLdo_pop_n (next_fn_args + 1);
	     goto return_error;
	  }

	if (NULL == (foreach_context = (*cl->cl_foreach_open) ((SLtype)type, next_fn_args)))
	  goto return_error;

	while (1)
	  {
	     int status;

	     if (IS_SLANG_ERROR)
	       {
		  (*cl->cl_foreach_close) ((SLtype) type, foreach_context);
		  goto return_error;
	       }

	     status = (*cl->cl_foreach) ((SLtype) type, foreach_context);
	     if (status <= 0)
	       {
		  if (status == 0)
		    break;

		  (*cl->cl_foreach_close) ((SLtype) type, foreach_context);
		  goto return_error;
	       }

	     inner_interp (block);
	     if (Lang_Break) break;
	     Lang_Break_Condition = /* Lang_Continue = */ 0;
	  }
	(*cl->cl_foreach_close) ((SLtype) type, foreach_context);
	break;

      case SLANG_BCST_WHILE:
	loop_name = "while";

	if (num_blocks != 2)
	  goto wrong_num_blocks_error;

	type = blks[1]->bc_main_type;
	while (1)
	  {
	     if (IS_SLANG_ERROR)
	       goto return_error;

	     inner_interp (block);
	     if (Lang_Break) break;

	     if (-1 == pop_ctrl_integer (&ctrl))
	       goto return_error;

	     if (ctrl == 0) break;

	     if (type)
	       {
		  inner_interp (blks[1]);
		  if (Lang_Break) break;
		  Lang_Break_Condition = /* Lang_Continue = */ 0;
	       }
	  }
	break;

      case SLANG_BCST_DOWHILE:
	loop_name = "do...while";

	if (num_blocks != 2)
	  goto wrong_num_blocks_error;

	while (1)
	  {
	     if (IS_SLANG_ERROR)
	       goto return_error;

	     Lang_Break_Condition = /* Lang_Continue = */ 0;
	     inner_interp (block);
	     if (Lang_Break) break;
	     Lang_Break_Condition = /* Lang_Continue = */ 0;
	     inner_interp (blks[1]);
	     if (-1 == pop_ctrl_integer (&ctrl))
	       goto return_error;

	     if (ctrl == 0) break;
	  }
	break;

      case SLANG_BCST_CFOR:
	loop_name = "for";

	/* we need 4 blocks: first 3 control, the last is code */
	if (num_blocks != 4) goto wrong_num_blocks_error;

	inner_interp (block);
	while (1)
	  {
	     if (IS_SLANG_ERROR)
	       goto return_error;

	     inner_interp(blks[1]);       /* test */
	     if (-1 == pop_ctrl_integer (&ctrl))
	       goto return_error;

	     if (ctrl == 0) break;
	     inner_interp(blks[3]);       /* code */
	     if (Lang_Break) break;
	     inner_interp(blks[2]);       /* bump */
	     Lang_Break_Condition = /* Lang_Continue = */ 0;
	  }
	break;

      case SLANG_BCST_FOR:
	  {
#if SLANG_OPTIMIZE_FOR_SPEED
	     SLang_Object_Type *objp;
#endif
	     loop_name = "_for";

	     if (num_blocks != 1)
	       goto wrong_num_blocks_error;

	     /* 3 elements: first, last, step */
	     if ((-1 == pop_int (&ctrl))
		 || (-1 == pop_int (&last))
		 || (-1 == pop_int (&first)))
	       goto return_error;

#if SLANG_OPTIMIZE_FOR_SPEED
	     objp = NULL;
	     if ((block->bc_main_type == SLANG_BC_SET_LOCAL_LVALUE)
		 && (block->bc_sub_type == SLANG_BCST_ASSIGN))
	       {
		  objp = Local_Variable_Frame - block->b.i_blk;
		  block++;
	       }
#endif
	     i = first;
	     while (1)
	       {
		  /* It is ugly to have this test here but I do not know of a
		   * simple way to do this without using two while loops.
		   */
		  if (ctrl >= 0)
		    {
		       if (i > last) break;
		    }
		  else if (i < last) break;

		  if (IS_SLANG_ERROR) goto return_error;
#if SLANG_OPTIMIZE_FOR_SPEED
		  if (objp != NULL)
		    {
		       if (objp->o_data_type != SLANG_INT_TYPE)
			 {
			    if (SLANG_CLASS_TYPE_SCALAR != GET_CLASS_TYPE(objp->o_data_type))
			      SLang_free_object (objp);
			    objp->o_data_type = SLANG_INT_TYPE;
			 }
		       objp->v.int_val = i;
		    }
		  else
#endif
		    push_int_object (SLANG_INT_TYPE, i);

		  inner_interp (block);
		  if (Lang_Break) break;
		  Lang_Break_Condition = /* Lang_Continue = */ 0;

		  i += ctrl;
	       }
	  }
	break;

      case SLANG_BCST_LOOP:
	loop_name = "loop";
	if (num_blocks != 1)
	  goto wrong_num_blocks_error;

	if (-1 == pop_int (&ctrl))
	  goto return_error;
	while (ctrl > 0)
	  {
	     ctrl--;

	     if (IS_SLANG_ERROR)
	       goto return_error;

	     inner_interp (block);
	     if (Lang_Break) break;
	     Lang_Break_Condition = /* Lang_Continue = */ 0;
	  }
	break;

      case SLANG_BCST_FOREVER:
	loop_name = "forever";

	if (num_blocks != 1)
	  goto wrong_num_blocks_error;

	while (1)
	  {
	     if (IS_SLANG_ERROR)
	       goto return_error;

	     inner_interp (block);
	     if (Lang_Break) break;
	     Lang_Break_Condition = /* Lang_Continue = */ 0;
	  }
	break;

      default:  _pSLang_verror(SL_INTERNAL_ERROR, "Unknown loop type");
	return -1;
     }
   if (Lang_Break == 0)
     {
	Lang_Break_Condition = Lang_Return;
	return 0;
     }

   if (Lang_Break < 0)
     {
	Lang_Break++;
	Lang_Break_Condition = 1;
     }
   else
     {
	Lang_Break--;
	Lang_Break_Condition = (Lang_Return || Lang_Break);
     }
   return 1;

   wrong_num_blocks_error:
   _pSLang_verror (SL_SYNTAX_ERROR, "Wrong number of blocks for '%s' construct", loop_name);

   /* drop */
   return_error:
   return -1;
   /* do_traceback (loop_name, NULL, -1); */
}

static void lang_do_and_orelse (int is_or, SLBlock_Type *addr, SLBlock_Type *addr_max)
{
   int test = 0;

   while (addr <= addr_max)
     {
#if USE_BC_LINE_NUM
	if (addr->bc_main_type == SLANG_BC_LINE_NUM)
	  {
	     addr++;
	     continue;
	  }
#endif
	inner_interp (addr->b.blk);
	if (IS_SLANG_ERROR
	    || Lang_Break_Condition
	    || (-1 == pop_ctrl_integer (&test)))
	  return;

	if (is_or == (test != 0))
	  break;

	/* if (((stype == SLANG_BCST_ANDELSE) && (test == 0))
	 *   || ((stype == SLANG_BCST_ORELSE) && test))
	 * break;
	 */

	addr++;
     }
   push_char_object (SLANG_CHAR_TYPE, (char) test);
}

/* Executes the block in error-free context.  If execution goes wrong, returns -1 */
static int try_interp_block (SLBlock_Type **bp)
{
   SLBlock_Type *b = *bp;

#if USE_BC_LINE_NUM
   while (b->bc_main_type == SLANG_BC_LINE_NUM)
     b++;
   *bp = b;
#endif

   b = b->b.blk;
   if (b->bc_main_type == 0)
     return 0;

   (void) inner_interp (b);

   if (IS_SLANG_ERROR)
     return -1;

   return 0;
}

static int do_try_internal (SLBlock_Type *ev_block, SLBlock_Type *final)
{
   SLBlock_Type *b;
   int stack_depth, num;
   unsigned int frame_depth, recurs_depth;
#if SLANG_HAS_BOSEOS
   int bos_stack_depth;
#endif
   int e1;
   int status;

   /* Try blocks have the form:
    * {ev_block} {try-statements} {exception-list}{catch-block}...{final}
    *
    * The parser guarantees that the first, second, and final blocks will be
    * present.  Line number blocks may also be present.
    */
   stack_depth = SLstack_depth ();
   frame_depth = Frame_Pointer_Depth;
   recurs_depth = Recursion_Depth;

#if SLANG_HAS_BOSEOS
   bos_stack_depth = BOS_Stack_Depth;
#endif
   b = ev_block + 1;
#if USE_BC_LINE_NUM
   while (b->bc_main_type == SLANG_BC_LINE_NUM)
     b++;
#endif
   (void) inner_interp (b->b.blk); /* try-block */

   if (0 == (e1 = SLang_get_error ()))
     return 0;

   num = SLstack_depth () - stack_depth;
   if (num > 0)
     SLdo_pop_n (num);

#if SLANG_HAS_BOSEOS
   while (bos_stack_depth < BOS_Stack_Depth)
     {
	(void) _pSLcall_eos_handler ();
	BOS_Stack_Depth--;
     }
#endif
   while (Recursion_Depth > recurs_depth)
     {
	(void) _pSL_decrement_frame_pointer ();
     }
   while (frame_depth < Frame_Pointer_Depth)
     {
	end_arg_list ();
     }

   if (-1 == _pSLang_push_error_context ())
     return -1;

   status = -1;

   if (-1 == try_interp_block (&ev_block))   /* evaluate the exception */
     goto return_error;

   b++;				       /* skip try-block */

   while (b < final)
     {
	stack_depth = SLstack_depth ();

	if (-1 == try_interp_block (&b))/* exception-list */
	  goto return_error;

	num = SLstack_depth () - stack_depth;
	if (num < 0)
	  {
	     _pSLang_verror (SL_StackUnderflow_Error, "Exception list is invalid");
	     goto return_error;
	  }

	if (num > 0)
	  {
	     while (num)
	       {
		  int e;
		  if (-1 == _pSLerr_pop_exception (&e))
		    goto return_error;

		  if (SLerr_exception_eqs (e1, e))
		    break;

		  num--;
	       }
	     if (num == 0)
	       {
		  /* No match, skip the exception-list */
		  b++;
#if USE_BC_LINE_NUM
		  while (b->bc_main_type == SLANG_BC_LINE_NUM)
		    b++;
#endif
		  /* And the block associated with it */
		  b++;
		  continue;
	       }
	     if (num > 1)
	       SLdo_pop_n (num-1);
	  }

	/* Found a match--- move on to the code to be executed */
	b++;
	/* _pSLerr_clear_error (); */
#if USE_BC_LINE_NUM
	while (b->bc_main_type == SLANG_BC_LINE_NUM)
	  b++;
#endif
	status = try_interp_block (&b);
	_pSLang_pop_error_context (status);
	if (status == 0)
	  _pSLerr_clear_error (0);
	return status;
     }

   if (b == final)
     {
	/* No matching catch block */
	status = 0;
     }

   return_error:
   _pSLang_pop_error_context (status);

   return -1;
}

static void do_try (SLBlock_Type *ev_block, SLBlock_Type *final)
{
   (void) do_try_internal (ev_block, final);

   if (final->b.blk->bc_main_type)
     {
	int bc = Lang_Break_Condition, r = Lang_Return, br = Lang_Break;
	/* Need to reset these so that loops work in the finally block */
	Lang_Break_Condition = Lang_Break = Lang_Return = 0;
	if (-1 == _pSLang_push_error_context ())
	  return;

	if (-1 == try_interp_block (&final))
	  _pSLang_pop_error_context (1);
	else
	  _pSLang_pop_error_context (0);

	Lang_Break = br; Lang_Return = r; Lang_Break_Condition = bc;
     }
}

/* This evaluates:
 *  (x0 op1 x1) and (x1 op2 x2) ... and (x{N-1} opN xN)
 * On stack: x0 x1 ... xN
 * Need to perform:
 *  x0 x1 op1 x1 x2 op2 and x2 x3 op3 and ... x{N-1} xN opN and
 *  --------  --------      --------  ....... -------------
 *     y1        y2            y3                    yN
 *     y1 y2 and y3 and ... yN and
 */
static int do_compare (SLBlock_Type *ops1)
{
   SLang_Object_Type a, b, c;
   SLang_Object_Type *ap, *bp, *cp;
   SLBlock_Type *ops = ops1;
   int ret = -1;

   /* skip to opN */
   while ((ops->bc_main_type == SLANG_BC_BINARY)
	  || (ops->bc_main_type == SLANG_BC_BINARY2)
	  || (ops->bc_main_type == SLANG_BC_COMBINED)
	  || (ops->bc_main_type == SLANG_BC_BINARY_LASTBLOCK)
	  || (ops->bc_main_type == SLANG_BC_BINARY_SET_LOCLVAL))
     ops++;

   bp = &b;
   if (-1 == SLang_pop (bp))
     return -1;

   ap = &a;
   cp = NULL;
   while (1)
     {
	SLang_Object_Type *tmp;
	ops--;
	if (-1 == SLang_pop (ap))
	  goto return_error;

	if (-1 == do_binary_ab_inc_ref (ops->b.i_blk, ap, bp))
	  {
	     SLang_free_object (ap);
	     goto return_error;
	  }
	SLang_free_object (bp);
	tmp = bp; bp = ap; ap = tmp;

	if (cp == NULL)
	  {
	     if (-1 == SLang_pop (&c))
	       goto return_error;
	     cp = &c;
	     continue;
	  }

	if (-1 == do_binary_b (SLANG_AND, cp))
	  goto return_error;
	SLang_free_object (cp);

	if (ops == ops1)
	  {
	     cp = NULL;
	     break;
	  }

	if (-1 == SLang_pop (cp))
	  {
	     cp = NULL;
	     goto return_error;
	  }
     }
   ret = 0;
   /* drop */
   return_error:
   if (cp != NULL)
     SLang_free_object (cp);
   SLang_free_object (bp);
   return ret;
}

static void do_else_if (SLBlock_Type *zero_block, SLBlock_Type *non_zero_block)
{
   int test;

   if (-1 == pop_ctrl_integer (&test))
     return;

   if (test == 0)
     non_zero_block = zero_block;

   if (non_zero_block != NULL)
     inner_interp (non_zero_block->b.blk);
}

int _pSLang_trace_fun (SLFUTURE_CONST char *f)
{
   if (NULL == (f = SLang_create_slstring (f)))
     return -1;

   SLang_free_slstring ((char *) Trace_Function);
   Trace_Function = f;
   _pSLang_Trace = 1;
   return 0;
}

int _pSLdump_objects (char *prefix, SLang_Object_Type *x, unsigned int n, int dir)
{
   char *s;
   SLang_Class_Type *cl;

   while (n)
     {
	GET_CLASS(cl,x->o_data_type);

	s = _pSLstringize_object (x);

	_pSLerr_dump_msg ("%s[%s]:%s\n", prefix, cl->cl_name,
			  ((s != NULL) ? s : "??"));
	SLang_free_slstring (s);

	x += dir;
	n--;
     }
   return 0;
}

/* Do NOT change this-- it corresponds to USRBLK[0-4]_TOKEN */
#define MAX_USER_BLOCKS	5
static SLBlock_Type *Exit_Block_Ptr;
static SLBlock_Type *Global_User_Block[MAX_USER_BLOCKS];
static SLBlock_Type **User_Block_Ptr = Global_User_Block;

static int find_local_variable_index (Function_Header_Type *header, char *name)
{
   char **local_variables;
   unsigned int nlocals;
   unsigned int i;

   if (header == NULL)
     return -1;
   local_variables = header->local_variables;
   nlocals = header->nlocals;

   for (i = 0; i < nlocals; i++)
     {
	if ((*name == local_variables[i][0])
	    && (0 == strcmp (name, local_variables[i])))
	  return (int)i;
     }
   return -1;
}

static SLang_Name_Type *
  find_global_name (char *name,
		    SLang_NameSpace_Type *pns, SLang_NameSpace_Type *sns,
		    SLang_NameSpace_Type *gns,
		    int do_error)
{
   return find_global_hashed_name (name, _pSLcompute_string_hash (name), pns, sns, gns, do_error);
}

#if SLANG_HAS_DEBUGGER_SUPPORT
/* frame depth numbered two ways:
 *
 *  positive:  12345..N
 *  negative:  ..543210
 */
int _pSLang_get_frame_depth (void)
{
   return (int) (Function_Stack_Ptr - Function_Stack);
}
static int get_function_stack_info (int depth, Function_Stack_Type *sp)
{
   int current_depth = _pSLang_get_frame_depth ();

   if (depth <= 0)
     depth += current_depth;

   if (depth == current_depth)
     {
	sp->function = Current_Function;
	sp->header = Current_Function_Header;
	sp->local_variable_frame = Local_Variable_Frame;
	sp->line = This_Compile_Linenum;
	sp->file = This_Compile_Filename;
	sp->static_ns = This_Static_NameSpace;
	sp->private_ns = This_Private_NameSpace;
	return 0;
     }

   if ((depth >= current_depth) || (depth <= 0))
     {
	_pSLang_verror (SL_INVALID_PARM, "Invalid Frame Depth");
	return -1;
     }
   *sp = *(Function_Stack + depth);
   return 0;
}

int _pSLang_get_frame_fun_info (int depth, _pSLang_Frame_Info_Type *f)
{
   Function_Stack_Type s;

   if (-1 == get_function_stack_info (depth, &s))
     return -1;

   f->locals = NULL;
   f->nlocals = 0;
   f->function = NULL;

   f->line = s.line;
   f->file = s.file;
   f->ns = s.static_ns->namespace_name;

   if (s.header != NULL)
     {
	f->locals = s.header->local_variables;
	f->nlocals = s.header->nlocals;
     }

   if (s.function != NULL)
     f->function = s.function->name;

   return 0;
}

int _pSLang_set_frame_variable (int depth, char *name)
{
   Function_Stack_Type s;
   SLang_Name_Type *nt;
   int i;

   if (-1 == get_function_stack_info (depth, &s))
     return -1;

   if (-1 != (i = find_local_variable_index (s.header, name)))
     {
	SLang_Object_Type *obj = s.local_variable_frame - i;
	return set_lvalue_obj (SLANG_BCST_ASSIGN, obj);
     }

   if (NULL == (nt = find_global_name (name, s.private_ns, s.static_ns, Global_NameSpace, 1)))
     return -1;

   return set_nametype_variable (nt);
}

int _pSLang_get_frame_variable (int depth, char *name)
{
   SLang_Class_Type *cl;
   Function_Stack_Type s;
   SLang_Name_Type *nt;
   int i;

   if (-1 == get_function_stack_info (depth, &s))
     return -1;

   if (-1 != (i = find_local_variable_index (s.header, name)))
     {
	SLang_Object_Type *obj = s.local_variable_frame - i;
	GET_CLASS(cl,obj->o_data_type);
	return (*cl->cl_push) (obj->o_data_type, (VOID_STAR) &obj->v);
     }

   if (NULL == (nt = find_global_name (name, s.private_ns, s.static_ns, Global_NameSpace, 1)))
     return -1;

   return push_nametype_variable (nt);
}

#endif

static void execute_slang_fun (_pSLang_Function_Type *fun, unsigned int linenum)
{
   register unsigned int i;
   register SLang_Object_Type *frame, *lvf;
   register unsigned int n_locals;
   Function_Header_Type *header;
   /* SLBlock_Type *val; */
   SLBlock_Type *exit_block_save;
   SLBlock_Type **user_block_save;
   SLBlock_Type *user_blocks[MAX_USER_BLOCKS];
   int issue_bofeof_info = 0;
   int nargs;

   exit_block_save = Exit_Block_Ptr;
   user_block_save = User_Block_Ptr;
   User_Block_Ptr = user_blocks;
   memset ((char *)user_blocks, 0, MAX_USER_BLOCKS*sizeof (SLBlock_Type *));
   Exit_Block_Ptr = NULL;

   if (-1 == increment_slang_frame_pointer (fun, linenum))
     return;
   nargs = SLang_Num_Function_Args;

   header = fun->header;
   /* Make sure we do not allow this header to get destroyed by something
    * like:  define crash () { eval ("define crash ();") }
    */
   header->num_refs++;
   n_locals = header->nlocals;

   /* let the error propagate through since it will do no harm
    and allow us to restore stack. */

   /* set new stack frame */
   lvf = frame = Local_Variable_Frame;
   i = n_locals;
   if ((lvf + i) >= Local_Variable_Stack + SLANG_MAX_LOCAL_STACK)
     {
	_pSLang_verror(SL_STACK_OVERFLOW, "%s: Local Variable Stack Overflow",
		     fun->name);
	goto the_return;
     }

   while (i--)
     {
	lvf++;
	lvf->o_data_type = SLANG_UNDEFINED_TYPE;
     }
   Local_Variable_Frame = lvf;

   /* read values of function arguments */
   if (header->nargs)
     (void) pop_n_objs_reverse (Local_Variable_Frame - (header->nargs-1), header->nargs);

#if SLANG_HAS_BOSEOS
   if (header->issue_bofeof_info)
     {
	issue_bofeof_info = 1;
	(void) _pSLcall_bof_handler (fun->name, header->file);
     }
#endif
   if (SLang_Enter_Function != NULL)
     (*SLang_Enter_Function) (fun->name);

   if (_pSLang_Trace)
     {
	int stack_depth;

	stack_depth = SLstack_depth ();

	if ((Trace_Function != NULL)
	    && (0 == strcmp (Trace_Function, fun->name))
	    && (Trace_Mode == 0))
	  Trace_Mode = 1;

	if (Trace_Mode)
	  {
	     /* The local variable frame grows backwards */
	     trace_dump (">>%s (%d args)\n",
			 (char *) fun->name,
			 Local_Variable_Frame,
			 (int) header->nargs,
			 -1);
	     Trace_Mode++;
	  }

	inner_interp (header->body);
	Lang_Break_Condition = Lang_Return = Lang_Break = 0;
	if (Exit_Block_Ptr != NULL) inner_interp(Exit_Block_Ptr);

	if (Trace_Mode)
	  {
	     Trace_Mode--;
	     stack_depth = SLstack_depth () - stack_depth;

	     trace_dump ("<<%s (returning %d values)\n", (char *) fun->name,
			 Stack_Pointer - stack_depth,
			 stack_depth,
			 1);

	     if (Trace_Mode == 1)
	       Trace_Mode = 0;
	  }
     }
   else
     {
	inner_interp (header->body);
	Lang_Break_Condition = Lang_Return = Lang_Break = 0;
	if (Exit_Block_Ptr != NULL) inner_interp(Exit_Block_Ptr);
     }

   if (SLang_Exit_Function != NULL)
     (*SLang_Exit_Function)(fun->name);

   if (IS_SLANG_ERROR)
     {
	do_function_traceback (header, linenum);
#if SLANG_HAS_BOSEOS && SLANG_HAS_DEBUGGER_SUPPORT
	/* (void) _pSLcall_debug_hook (); */
#endif
     }

   /* free local variables.... */
   lvf = Local_Variable_Frame;
   while (lvf > frame)
     {
#if SLANG_OPTIMIZE_FOR_SPEED
	SLang_Class_Type *cl;
	GET_CLASS(cl, lvf->o_data_type);
	if (SLANG_CLASS_TYPE_SCALAR != cl->cl_class_type)
	  free_object (lvf, cl);
#else
	SLang_free_object (lvf);
#endif
	lvf--;
     }
   Local_Variable_Frame = lvf;

   the_return:

   if (header->num_refs == 1)
     free_function_header (header);
   else
     header->num_refs--;

   Lang_Break_Condition = Lang_Return = Lang_Break = 0;
   Exit_Block_Ptr = exit_block_save;
   User_Block_Ptr = user_block_save;

   if (nargs != SLang_Num_Function_Args)
     SLang_verror (SL_INTERNAL_ERROR, "execute_slang_fun: SLang_Num_Function_Args changed");

   (void) decrement_slang_frame_pointer ();
#if SLANG_HAS_BOSEOS
   if (issue_bofeof_info)
     (void) _pSLcall_eof_handler ();
#endif
}

static void do_traceback (SLCONST char *message)
{
   if (SLang_Traceback == SL_TB_NONE)
     return;

   if (message != NULL)
     _pSLerr_traceback_msg ("Traceback: %s\n", message);
}

static void do_function_traceback (Function_Header_Type *header, unsigned int linenum)
{
   unsigned int nlocals;
   char *s;
   unsigned int i;
   SLang_Object_Type *objp;
   SLtype stype;

   if (SLang_Traceback == SL_TB_NONE)
     return;

   /* Doing this will allow line number errors in recursive functions to be reported */
   _pSLerr_set_line_info (header->file, (int)linenum, "");

   if ((0 == (SLang_Traceback & SL_TB_FULL))
       || (SLang_Traceback & SL_TB_OMIT_LOCALS)
       || (0 == (nlocals = header->nlocals))
       || (header->local_variables == NULL))
     return;

   _pSLerr_traceback_msg ("  Local variables for %s:\n", Current_Function->name);

   for (i = 0; i < nlocals; i++)
     {
	SLang_Class_Type *cl;
	char *class_name;

	objp = Local_Variable_Frame - i;
	stype = objp->o_data_type;

	s = _pSLstringize_object (objp);
	GET_CLASS(cl,stype);
	class_name = cl->cl_name;

	_pSLerr_traceback_msg ("\t%s %s = ", class_name, header->local_variables[i]);

	if (s == NULL) _pSLerr_traceback_msg ("??\n");
	else
	  {
	     SLCONST char *q = "";
#ifndef HAVE_VSNPRINTF
	     char buf[256];
	     if (strlen (s) >= sizeof (buf))
	       {
		  strncpy (buf, s, sizeof(buf));
		  s = buf;
		  s[sizeof(buf) - 1] = 0;
	       }
#endif
	     if (SLANG_STRING_TYPE == stype) q = "\"";
	     _pSLerr_traceback_msg ("%s%s%s\n", q, s, q);
	  }
     }
}

static void do_app_unary (SLang_App_Unary_Type *nt)
{
   if (-1 == do_unary (nt->unary_op, nt->name_type))
     do_traceback (nt->name);
}

static void do_arith_binary (SLang_Arith_Binary_Type *nt)
{
   if (-1 == do_binary (nt->binary_op))
     do_traceback (nt->name);
}

int _pSLang_dereference_ref (SLang_Ref_Type *ref)
{
   return ref->deref (ref->data);
}

int _pSLang_is_ref_initialized (SLang_Ref_Type *ref)
{
   if (ref->is_initialized != NULL)
     return ref->is_initialized (ref->data);

   return 1;			       /* punt */
}

int _pSLang_uninitialize_ref (SLang_Ref_Type *ref)
{
   if (ref->uninitialize != NULL)
     return (*ref->uninitialize) (ref->data);

   return 0;
}

void (*SLang_Interrupt)(void) = NULL;

int _pSLpush_slang_obj (SLang_Object_Type *obj)
{
   SLtype subtype;
   SLang_Class_Type *cl;

   if (obj == NULL) return SLang_push_null ();

   subtype = obj->o_data_type;

#if SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR == GET_CLASS_TYPE(subtype))
     return push_object (obj);
#endif

   GET_CLASS(cl,subtype);
   return (*cl->cl_push) (subtype, (VOID_STAR) &obj->v);
}

_INLINE_
static int carefully_push_object (SLang_Object_Type *obj)
{
   SLang_Class_Type *cl;
   SLtype subtype;

   subtype = obj->o_data_type;

   GET_CLASS(cl,subtype);

#if SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR == cl->cl_class_type)
     return push_object (obj);
   if (subtype == SLANG_STRING_TYPE)
     return _pSLang_dup_and_push_slstring (obj->v.s_val);
   if (subtype == SLANG_ARRAY_TYPE)
     return _pSLang_push_array (obj->v.array_val, 0);
#endif

   return (*cl->cl_push) (subtype, (VOID_STAR) &obj->v);
}

int _pSLslang_copy_obj (SLang_Object_Type *obja, SLang_Object_Type *objb)
{
   SLtype type;

   type = obja->o_data_type;

#if SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR == GET_CLASS_TYPE(type))
     {
	*objb = *obja;
	return 0;
     }
#endif

   if (-1 == carefully_push_object (obja))
     return -1;

   return pop_object (objb);
}

#define PUSH_LOCAL_VARIABLE(_indx) \
   { \
      SLang_Object_Type *_obj = Local_Variable_Frame - (_indx); \
      (void) carefully_push_object (_obj); \
   }

static int push_local_variable (int i)
{
   SLang_Object_Type *obj = Local_Variable_Frame - i;
   SLang_Class_Type *cl;
   SLtype subtype;

   subtype = obj->o_data_type;

#if SLANG_OPTIMIZE_FOR_SPEED
   if (SLANG_CLASS_TYPE_SCALAR == GET_CLASS_TYPE(subtype))
     return push_object (obj);
   if (subtype == SLANG_STRING_TYPE)
     return _pSLang_dup_and_push_slstring (obj->v.s_val);
   if (subtype == SLANG_ARRAY_TYPE)
     return _pSLang_push_array (obj->v.array_val, 0);
#endif

   GET_CLASS(cl,subtype);
   return (*cl->cl_push) (subtype, (VOID_STAR) &obj->v);
}

#if SLANG_OPTIMIZE_FOR_SPEED
static int push_array_element (int lvaridx, SLindex_Type idx)
{
   SLang_Object_Type *obj = Local_Variable_Frame - lvaridx;

   if (obj->o_data_type == SLANG_ARRAY_TYPE)
     {
	SLang_Array_Type *at = obj->v.array_val;

	if (at->num_dims == 1)
	  {
	     if (at->data_type == SLANG_INT_TYPE)
	       {
		  int *ptr = (int *)at->index_fun (at, &idx);
		  if (ptr == NULL)
		    return -1;
		  return push_int_object (SLANG_INT_TYPE, *ptr);
	       }
# if SLANG_HAS_FLOAT
	     else if (at->data_type == SLANG_DOUBLE_TYPE)
	       {
		  double *ptr = (double *)at->index_fun (at, &idx);
		  if (ptr == NULL)
		    return -1;
		  return push_double_object (SLANG_DOUBLE_TYPE, *ptr);
	       }
# endif
	     return _pSLarray1d_push_elem (at, idx);
	  }
	/* drop and fail */
     }

   /* Do it the hard way */
   if ((0 == push_array_index (SLANG_ARRAY_INDEX_TYPE, idx))
       && (0 == push_local_variable (lvaridx)))
     return _pSLarray_aget1 (1);

   return -1;
}
#endif

#if SLANG_OPTIMIZE_FOR_SPEED
static int pop_to_lvar_array_element (int lvaridx, SLindex_Type idx)
{
   SLang_Object_Type *obj = Local_Variable_Frame - lvaridx;

   if ((obj->o_data_type == SLANG_ARRAY_TYPE)
       && (idx >= 0))
     {
	SLang_Array_Type *at = obj->v.array_val;

	if ((at->num_dims == 1)
	    && (at->flags == 0)
	    && (idx < (SLindex_Type) at->num_elements))
	  {
	     if (at->data_type == SLANG_INT_TYPE)
	       return pop_int ((int *)at->data + idx);
#if SLANG_HAS_FLOAT
	     else if (at->data_type == SLANG_DOUBLE_TYPE)
	       {
		  SLang_Object_Type dobj;
		  if (-1 == pop_object_of_type (SLANG_DOUBLE_TYPE, &dobj, 0))
		    return -1;
		  *((double *)at->data + idx) = dobj.v.double_val;
		  return 0;
	       }
#endif
	  }
     }
   /* Do it the hard way */
   if ((0 == push_array_index (SLANG_ARRAY_INDEX_TYPE, idx))
       && (0 == push_local_variable (lvaridx)))
     return _pSLarray_aput1 (1);

   return -1;
}
#endif

#if SLANG_OPTIMIZE_FOR_SPEED
static int aget1_from_lvar_binary (int lvaridx, SLindex_Type idx, int op,
				  SLang_Object_Type *objb)
{
   SLang_Object_Type *obj;

   obj = Local_Variable_Frame - lvaridx;

   if (obj->o_data_type == SLANG_ARRAY_TYPE)
     {
	SLang_Array_Type *at = obj->v.array_val;

	if (at->num_dims == 1)
	  {
	     SLang_Object_Type a, c;
	     if (at->data_type == SLANG_INT_TYPE)
	       {
		  int *ptr = (int *)at->index_fun (at, &idx);
		  if (ptr == NULL)
		    return -1;
		  a.o_data_type = SLANG_INT_TYPE;
		  a.v.int_val = *ptr;
		  if (objb->o_data_type == SLANG_INT_TYPE)
		    return int_int_binary (op, &a, objb);
#if SLANG_HAS_FLOAT
		  else if (objb->o_data_type == SLANG_DOUBLE_TYPE)
		    {
		       if (-1 == int_dbl_binary_result (op, &a, objb, &c))
			 return -1;
		       return push_object (&c);
		    }
#endif
		  /* else handled below */
	       }
# if SLANG_HAS_FLOAT
	     else if (at->data_type == SLANG_DOUBLE_TYPE)
	       {
		  double *ptr;

		  ptr = (double *)at->index_fun (at, &idx);
		  if (ptr == NULL)
		    return -1;

		  a.o_data_type = SLANG_DOUBLE_TYPE;
		  a.v.double_val = *ptr;

		  if (objb->o_data_type == SLANG_DOUBLE_TYPE)
		    return dbl_dbl_binary (op, &a, objb);
		  else if (objb->o_data_type == SLANG_INT_TYPE)
		    {
		       if (-1 == dbl_int_binary_result (op, &a, objb, &c))
			 return -1;
		       return push_object (&c);
		    }
		  /* else handled below */
	       }
# endif
	     if (-1 == _pSLarray1d_push_elem (at, idx))
	       return -1;
	     return do_binary_b (op, objb);
	  }
     }

   if (-1 == push_array_element (lvaridx, idx))
     return -1;

   return do_binary_b (op, objb);
}
#endif

static int dereference_object (void)
{
   SLang_Object_Type obj;
   SLang_Class_Type *cl;
   SLtype type;
   int ret;

   if (-1 == pop_object(&obj))
     return -1;

   type = obj.o_data_type;

   GET_CLASS(cl,type);
   ret = (*cl->cl_dereference)(type, (VOID_STAR) &obj.v);

   free_object (&obj, cl);
   return ret;
}

/* This function gets called with the stack of the form:
 *   ... func __args ...
 * We need to pop func from within the stack.
 */
/* End the argument list, and make the function call */
static int deref_fun_call (int linenum)
{
   SLang_Object_Type obj;

   if (-1 == end_arg_list ())
     return -1;

   if (-1 == roll_stack (-(Next_Function_Num_Args + 1)))
     return -1;

   if (-1 == pop_object(&obj))
     return -1;

   return deref_call_object (&obj, linenum);
}

static int obsolete_deref_fun_call (int linenum)
{
   SLang_Object_Type obj;

   if (-1 == end_arg_list ())
     return -1;

  Next_Function_Num_Args--;          /* do not include function to be derefed. */

   if (-1 == pop_object(&obj))
     return -1;

   return deref_call_object (&obj, linenum);
}

static int case_function (void)
{
   SLang_Object_Type a_obj;
   SLang_Object_Type *swobjptr;
   int eqs;

   swobjptr = Switch_Obj_Ptr - 1;

   if ((swobjptr < Switch_Objects)
       || (0 == swobjptr->o_data_type))
     {
	_pSLang_verror (SL_SYNTAX_ERROR, "Misplaced 'case' keyword");
	return -1;
     }

   if (-1 == pop_object(&a_obj))
     return -1;

   eqs = _pSLclass_obj_eqs (&a_obj, swobjptr);
   SLang_free_object (&a_obj);

   if (eqs == -1)
     return -1;

   return push_int_object (SLANG_INT_TYPE, eqs);
}

static void tmp_variable_function (SLBlock_Type *addr)
{
   SLang_Object_Type *obj;

   switch (addr->bc_sub_type)
     {
      case SLANG_GVARIABLE:
      case SLANG_PVARIABLE:
	obj = &addr->b.nt_gvar_blk->obj;
	break;

      case SLANG_LVARIABLE:
	obj = Local_Variable_Frame - addr->b.i_blk;
	break;

      default:
	(void) SLang_set_error (SL_INTERNAL_ERROR);
	return;
     }

   /* There is no need to go through higher level routines since we are
    * not creating or destroying extra copies.
    */
   if (-1 == push_object (obj))
     return;

   obj->o_data_type = SLANG_UNDEFINED_TYPE;
   obj->v.ptr_val = NULL;
}

static
int _pSLang_parse_dollar_string (SLFUTURE_CONST char *str, char ***argvp, unsigned int *argcp)
{
   unsigned int len;
   SLFUTURE_CONST char *s;
   char *fmt;
   char **argv;
   char ch;
   unsigned int num_dollars;
   unsigned int i, argc;

   len = 0;
   num_dollars = 1;		       /* allow for argv[0]=fmt */
   s = str;
   while ((ch = *s++) != 0)
     {
	len++;
	if (ch == '%')
	  {
	     len++;
	     continue;
	  }
	if (ch == '$')
	  {
	     num_dollars++;
	     continue;
	  }
     }
   fmt = SLmalloc (len+1);
   if (fmt == NULL)
     return -1;

   argv = (char **)SLcalloc (sizeof (char *), num_dollars);
   if (argv == NULL)
     {
	SLfree (fmt);
	return -1;
     }

   argc = 0;
   argv[argc] = fmt;
   argc++;

   s = str;
   while ((ch = *s++) != 0)
     {
	SLFUTURE_CONST char *s0, *s1;
	char *arg;

	if (ch != '$')
	  {
	     *fmt++ = ch;
	     if (ch == '%')
	       *fmt++ = ch;
	     continue;
	  }
	ch = *s++;
	if (ch == '$')
	  {
	     *fmt++ = '$';
	     continue;
	  }
	if (ch == 0)
	  {
	     *fmt++ = '$';
	     break;
	  }

	if ((ch != '_')
	    && (0 == SLwchar_isalnum (ch)))
	  {
	     if (ch != '{')
	       {
		  *fmt++ = '$';
		  *fmt++ = ch;
		  continue;
	       }
	     s0 = s;
	     while (*s && (*s != '}'))
	       s++;
	     if (*s == 0)
	       {
		  _pSLang_verror (SL_SYNTAX_ERROR, "Unable to find matching }");
		  goto return_error;
	       }
	     s1 = s + 1;
	  }
	else
	  {
	     s0 = s-1;
	     if (SLwchar_isdigit (ch)) /* e.g., $1 */
	       s0--;

	     while ((*s == '_')
		    || SLwchar_isalnum (*s))
	       s++;
	     s1 = s;
	  }

	if (NULL == (arg = SLmake_nstring (s0, s-s0)))
	  goto return_error;

	argv[argc] = arg;
	argc++;

	*fmt++ = '%';
	*fmt++ = 'S';

	s = s1;
     }
   *fmt = 0;

   *argvp = argv;
   *argcp = argc;
   return 0;

   return_error:
   for (i = 0; i < argc; i++)
     SLfree (argv[i]);
   SLfree ((char *)argv);
   return -1;
}

/* Convert "bla bla $foo bla" to sprintf ("bla bla %S bla", foo)
 */
int _pSLpush_dollar_string (SLFUTURE_CONST char *str)
{
   /* return SLang_push_string (str); */
   char **argv;
   unsigned int argc;
   unsigned int i;
   SLang_NameSpace_Type *private_ns, *static_ns;
   int status = -1;

   if (-1 == _pSLang_parse_dollar_string (str, &argv, &argc))
     return -1;

   if (-1 == SLang_push_string (argv[0]))
     goto free_return;

   if (Current_Function_Header == NULL)
     {
	private_ns = This_Private_NameSpace;
	static_ns = This_Static_NameSpace;
     }
   else
     {
	private_ns = Current_Function_Header->private_ns;
	static_ns = Current_Function_Header->static_ns;
     }

   for (i = 1; i < argc; i++)
     {
	char *name = argv[i];
	SLang_Name_Type *nt;
	SLFUTURE_CONST char *env;
	int j;

	if (*name == 0)
	  {
	     if (-1 == SLang_push_string (name))
	       goto free_return;

	     continue;
	  }

	j = find_local_variable_index (Current_Function_Header, name);
	if (j != -1)
	  {
	     if (-1 == push_local_variable (j))
	       goto free_return;

	     continue;
	  }
	if (NULL != (nt = find_global_name (name, private_ns, static_ns, Global_NameSpace, 0)))
	  {
	     if (-1 == push_nametype_variable (nt))
	       goto free_return;
	     continue;
	  }

	/* Assume it is an environment variable */
	env = getenv (name);
	if (env == NULL)
	  env = "";
	if (-1 == SLang_push_string (env))
	  goto free_return;
     }

   status = _pSLstrops_do_sprintf_n (argc-1);
   /* drop */

   free_return:
   for (i = 0; i < argc; i++)
     SLfree (argv[i]);

   SLfree ((char *) argv);
   return status;
}

static int
do_inner_interp_error (SLBlock_Type *err_block,
		       SLBlock_Type *addr_start,
		       SLBlock_Type *addr)
{
   SLFUTURE_CONST char *file = NULL, *funname = NULL;
   int line;

   /* Someday I can use the theses variable to provide extra information
    * about what went wrong.
    */
   (void) addr_start;
   (void) addr;

   if (Current_Function_Header != NULL)
     {
	file = Current_Function_Header->file;
	funname = Current_Function->name;
     }
   else file = This_Compile_Filename;

   line = addr->linenum;

/* #if SLANG_HAS_BOSEOS && SLANG_HAS_DEBUGGER_SUPPORT */
/*    if (SLang_get_error () == SL_USER_BREAK) */
/*      (void) _pSLcall_debug_hook (file, linenum); */
/* #endif */

   if (err_block == NULL)
     goto return_error;

   if (-1 == _pSLang_push_error_context ())
     goto return_error;

   inner_interp (err_block->b.blk);

   (void) _pSLang_pop_error_context (0);
   if (SLang_get_error () == 0)
     return 0;

   return_error:
#if SLANG_HAS_DEBUG_CODE
   if ((_pSLang_Error == SL_USAGE_ERROR)
       && (SLang_Traceback == SL_TB_NONE))
     return -1;

   if (file != NULL)
     (void) _pSLerr_set_line_info (file, line, funname);
#endif
   return -1;
}

#define GATHER_STATISTICS 0
#if GATHER_STATISTICS
static unsigned int Bytecodes[0xFFFF];

static void print_stats (void)
{
   unsigned int i;
   unsigned long total;
   FILE *fp = fopen ("stats.txt", "w");
   if (fp == NULL)
     return;

   total = 0;
   for (i = 0; i < 0xFFFF; i++)
     total += Bytecodes[i];

   if (total == 0)
     total = 1;

   for (i = 0; i < 0xFFFF; i++)
     {
	if (Bytecodes[i])
	  fprintf (fp, "0x%04X %9u %e\n", i, Bytecodes[i], Bytecodes[i]/(double) total);
     }
   fclose (fp);
}

static void add_to_statistics (SLBlock_Type *b)
{
   unsigned short x, y;

   while (1)
     {
	x = b->bc_main_type;
	if (x == 0)
	  {
	     Bytecodes[0] += 1;
	     return;
	  }
	b++;

	while (SLANG_IS_BC_COMBINED(b->bc_main_type))
	  b++;

	y = b->bc_main_type;

	Bytecodes[(x << 8) | y] += 1;
     }
}

#endif

#define EXECUTE_INTRINSIC(addr) \
   { \
      SLang_Intrin_Fun_Type *f = (addr)->b.nt_ifun_blk; \
      if ((f->num_args == 0) && (f->return_type == SLANG_VOID_TYPE) && (Trace_Mode == 0)) \
	{ \
	   if (0 == _pSL_increment_frame_pointer ()) \
	     { \
		((VF0_Type) f->i_fun)(); \
		(void) _pSL_decrement_frame_pointer (); \
	     } \
	} \
      else execute_intrinsic_fun (f); \
   }

/* inner interpreter */
/* The return value from this function is only meaningful when it is used
 * to process blocks for the switch statement.  If it returns 0, the calling
 * routine should pass the next block to it.  Otherwise it will
 * return non-zero, with or without error.
 */
static int inner_interp (SLBlock_Type *addr_start)
{
   SLBlock_Type *block, *err_block, *addr;
#if GATHER_STATISTICS
   static int inited = 0;

   if (inited == 0)
     {
	(void) SLang_add_cleanup_function (print_stats);
	inited = 1;
     }
#endif

   /* for systems that have no real interrupt facility (e.g. go32 on dos) */
   if (SLang_Interrupt != NULL) (*SLang_Interrupt)();

   block = err_block = NULL;
   addr = addr_start;

   if (IS_SLANG_ERROR)
     {
	(void) do_inner_interp_error (err_block, addr_start, addr);
	return 0;
     }

#if GATHER_STATISTICS
   add_to_statistics (addr);
#endif
   /* Moving addr++ to top of switch instead of bottom was suggested by
    * Paul Boekholt to improve branch-prediction.
    */
   addr--;

   while (1)
     {
	addr++;
	switch (addr->bc_main_type)
	  {
	   case SLANG_BC_LAST_BLOCK:
	     goto return_1;
	   case SLANG_BC_LVARIABLE:
	     PUSH_LOCAL_VARIABLE (addr->b.i_blk)
	     break;
	   case SLANG_BC_GVARIABLE:
	     if (-1 == _pSLpush_slang_obj (&addr->b.nt_gvar_blk->obj))
	       do_name_type_error (addr->b.nt_blk);
	     break;

	   case SLANG_BC_IVARIABLE:
	   case SLANG_BC_RVARIABLE:
	     push_intrinsic_variable (addr->b.nt_ivar_blk);
	     break;

	   case SLANG_BC_INTRINSIC:
	     EXECUTE_INTRINSIC(addr)
	     if (IS_SLANG_ERROR)
	       do_traceback(addr->b.nt_ifun_blk->name);
	     break;

	   case SLANG_BC_FUNCTION:
	     execute_slang_fun (addr->b.nt_fun_blk, addr->linenum);
	     if (Lang_Break_Condition) goto handle_break_condition;
	     break;

	   case SLANG_BC_MATH_UNARY:
	   case SLANG_BC_APP_UNARY:
	   case SLANG_BC_ARITH_UNARY:
	     /* Make sure we treat these like function calls since the
	      * parser took abs(x), sin(x), etc to be a function call.
	      */
	     if (0 == _pSL_increment_frame_pointer ())
	       {
		  do_app_unary (addr->b.nt_unary_blk);
		  (void) _pSL_decrement_frame_pointer ();
	       }
	     break;

	   case SLANG_BC_ARITH_BINARY:
	     /* Make sure we treat these like function calls since the
	      * parser took _op_eqs, etc as function calls.
	      */
	     if (0 == _pSL_increment_frame_pointer ())
	       {
		  do_arith_binary (addr->b.nt_binary_blk);
		  (void) _pSL_decrement_frame_pointer ();
	       }
	     break;

	   case SLANG_BC_ICONST:
	     push_int_object (addr->b.iconst_blk->data_type, addr->b.iconst_blk->value);
	     break;

#if SLANG_HAS_FLOAT
	   case SLANG_BC_DCONST:
	     push_double_object (SLANG_DOUBLE_TYPE, addr->b.dconst_blk->d);
	     break;
	   case SLANG_BC_FCONST:
	     SLclass_push_float_obj (SLANG_FLOAT_TYPE, addr->b.fconst_blk->f);
	     break;
#endif
#ifdef HAVE_LONG_LONG
	   case SLANG_BC_LLCONST:
	     SLclass_push_llong_obj (addr->b.llconst_blk->data_type, addr->b.llconst_blk->value);
	     break;
#endif
	   case SLANG_BC_PVARIABLE:
	     if (-1 == _pSLpush_slang_obj (&addr->b.nt_gvar_blk->obj))
	       do_name_type_error (addr->b.nt_blk);
	     break;

	   case SLANG_BC_PFUNCTION:
	     execute_slang_fun (addr->b.nt_fun_blk, addr->linenum);
	     if (Lang_Break_Condition) goto handle_break_condition;
	     break;
	   case SLANG_BC_HCONST:
	     SLclass_push_short_obj (addr->b.iconst_blk->data_type, addr->b.hconst_blk->value);
	     break;
	   case SLANG_BC_LCONST:
	     SLclass_push_long_obj (addr->b.iconst_blk->data_type, addr->b.lconst_blk->value);
	     break;

#if USE_UNUSED_BYCODES_IN_SWITCH
# ifndef HAVE_LONG_LONG
	   case SLANG_BC_LLCONST:
# endif
	   case SLANG_BC_UNUSED_0x13:
	   case SLANG_BC_UNUSED_0x14:
	   case SLANG_BC_UNUSED_0x15:
	   case SLANG_BC_UNUSED_0x16:
	   case SLANG_BC_UNUSED_0x17:
	   case SLANG_BC_UNUSED_0x18:
	   case SLANG_BC_UNUSED_0x19:
	   case SLANG_BC_UNUSED_0x1A:
	   case SLANG_BC_UNUSED_0x1B:
	   case SLANG_BC_UNUSED_0x1C:
	   case SLANG_BC_UNUSED_0x1D:
	   case SLANG_BC_UNUSED_0x1E:
	   case SLANG_BC_UNUSED_0x1F:
	     _pSLang_verror (SL_INTERNAL_ERROR, "Byte-Code 0x%X is not valid", addr->bc_main_type);
	     break;
#endif
	   case SLANG_BC_SET_LOCAL_LVALUE:
	     set_lvalue_obj (addr->bc_sub_type, Local_Variable_Frame - addr->b.i_blk);
	     break;
	   case SLANG_BC_SET_GLOBAL_LVALUE:
	     if (-1 == set_lvalue_obj (addr->bc_sub_type, &addr->b.nt_gvar_blk->obj))
	       do_name_type_error (addr->b.nt_blk);
	     break;
	   case SLANG_BC_SET_INTRIN_LVALUE:
	     set_intrin_lvalue (addr);
	     break;
	   case SLANG_BC_SET_STRUCT_LVALUE:
	     set_struct_lvalue (addr);
	     break;
	   case SLANG_BC_SET_ARRAY_LVALUE:
	     set_array_lvalue (addr->bc_sub_type);
	     break;
	   case SLANG_BC_SET_DEREF_LVALUE:
	     set_deref_lvalue (addr->bc_sub_type);
	     break;

	   case SLANG_BC_FIELD:
	     (void) push_struct_field (addr->b.s_blk);
	     break;
	   case SLANG_BC_METHOD:
	     do_struct_method (addr->b.s_blk, addr->linenum);
	     break;
#if SLANG_OPTIMIZE_FOR_SPEED
	   case SLANG_BC_LVARIABLE_AGET:
	       {
		  SLang_Object_Type *obj = Local_Variable_Frame - addr->b.i_blk;
		  if (0 == carefully_push_object (obj))
		    do_bc_call_direct_nargs (_pSLarray_aget);
	       }
	     break;

	   case SLANG_BC_LVARIABLE_APUT:
	       {
		  SLang_Object_Type *obj = Local_Variable_Frame - addr->b.i_blk;
		  if (0 == carefully_push_object (obj))
		    do_bc_call_direct_nargs (_pSLarray_aput);
	       }
	     break;
#else
	   case SLANG_BC_LVARIABLE_AGET:
	   case SLANG_BC_LVARIABLE_APUT:
	     _pSLang_verror (SL_INTERNAL_ERROR, "Byte-Code 0x%X is not valid", addr->bc_main_type);
	     break;
#endif
	   case SLANG_BC_LOBJPTR:
	     (void)push_lv_as_ref (Local_Variable_Frame - addr->b.i_blk);
	     break;

	   case SLANG_BC_GOBJPTR:
	     (void)_pSLang_push_nt_as_ref (addr->b.nt_blk);
	     break;

	   case SLANG_BC_FIELD_REF:
	     (void) _pSLstruct_push_field_ref (addr->b.s_blk);
	     break;

	   case SLANG_BC_OBSOLETE_DEREF_FUN_CALL:
	     (void) obsolete_deref_fun_call (addr->linenum);
	     break;

	   case SLANG_BC_DEREF_FUN_CALL:
	     (void) deref_fun_call (addr->linenum);
	     break;

#if USE_UNUSED_BYCODES_IN_SWITCH
	   case SLANG_BC_UNUSED_0x2F:
	   case SLANG_BC_UNUSED_0x30:
	   case SLANG_BC_UNUSED_0x31:
	   case SLANG_BC_UNUSED_0x32:
	   case SLANG_BC_UNUSED_0x33:
	   case SLANG_BC_UNUSED_0x34:
	   case SLANG_BC_UNUSED_0x35:
	   case SLANG_BC_UNUSED_0x36:
	   case SLANG_BC_UNUSED_0x37:
	   case SLANG_BC_UNUSED_0x38:
	   case SLANG_BC_UNUSED_0x39:
	   case SLANG_BC_UNUSED_0x3A:
	   case SLANG_BC_UNUSED_0x3B:
	   case SLANG_BC_UNUSED_0x3C:
	   case SLANG_BC_UNUSED_0x3D:
	   case SLANG_BC_UNUSED_0x3E:
	   case SLANG_BC_UNUSED_0x3F:
	     _pSLang_verror (SL_INTERNAL_ERROR, "Byte-Code 0x%X is not valid", addr->bc_main_type);
	     break;
#endif
	   case SLANG_BC_LITERAL:
#if !SLANG_OPTIMIZE_FOR_SPEED
	   case SLANG_BC_LITERAL_INT:
# if SLANG_HAS_FLOAT
	   case SLANG_BC_LITERAL_DBL:
# endif
	   case SLANG_BC_LITERAL_STR:
#endif
	       {
		  SLang_Class_Type *cl;

		  /* No user types should be here */
		  GET_BUILTIN_CLASS(cl, addr->bc_sub_type);
		  (*cl->cl_push_literal) (addr->bc_sub_type, (VOID_STAR) &addr->b.ptr_blk);
	       }
	     break;
#if SLANG_OPTIMIZE_FOR_SPEED
	   case SLANG_BC_LITERAL_INT:
	     push_int_object (addr->bc_sub_type, (int) addr->b.l_blk);
	     break;
#if SLANG_HAS_FLOAT
	   case SLANG_BC_LITERAL_DBL:
	     push_double_object (addr->bc_sub_type, *addr->b.double_blk);
	     break;
#endif
	   case SLANG_BC_LITERAL_STR:
	     _pSLang_dup_and_push_slstring (addr->b.s_blk);
	     break;
#endif
	   case SLANG_BC_DOLLAR_STR:
	     (void) _pSLpush_dollar_string (addr->b.s_blk);
	     break;

#if USE_UNUSED_BYCODES_IN_SWITCH
	   case SLANG_BC_UNUSED_0x45:
	   case SLANG_BC_UNUSED_0x46:
	   case SLANG_BC_UNUSED_0x47:
	   case SLANG_BC_UNUSED_0x48:
	   case SLANG_BC_UNUSED_0x49:
	   case SLANG_BC_UNUSED_0x4A:
	   case SLANG_BC_UNUSED_0x4B:
	   case SLANG_BC_UNUSED_0x4C:
	   case SLANG_BC_UNUSED_0x4D:
	   case SLANG_BC_UNUSED_0x4E:
	   case SLANG_BC_UNUSED_0x4F:
	     _pSLang_verror (SL_INTERNAL_ERROR, "Byte-Code 0x%X is not valid", addr->bc_main_type);
	     break;
#endif
	   case SLANG_BC_UNARY:
	     do_unary (addr->b.i_blk, SLANG_BC_UNARY);
	     break;

	   case SLANG_BC_BINARY:
	     (void) do_binary (addr->b.i_blk);
	     break;

#if SLANG_OPTIMIZE_FOR_SPEED
	   case SLANG_BC_INTEGER_PLUS:
	     if (0 == push_int_object (addr->bc_sub_type, (int) addr->b.l_blk))
	       (void) do_binary (SLANG_PLUS);
	     break;

	   case SLANG_BC_INTEGER_MINUS:
	     if (0 == push_int_object (addr->bc_sub_type, (int) addr->b.l_blk))
	       (void) do_binary (SLANG_MINUS);
	     break;
#endif
#if USE_UNUSED_BYCODES_IN_SWITCH
# if !SLANG_OPTIMIZE_FOR_SPEED
	   case SLANG_BC_INTEGER_PLUS:
	   case SLANG_BC_INTEGER_MINUS:
	     break;
# endif
	   case SLANG_BC_UNUSED_0x54:
	   case SLANG_BC_UNUSED_0x55:
	   case SLANG_BC_UNUSED_0x56:
	   case SLANG_BC_UNUSED_0x57:
	   case SLANG_BC_UNUSED_0x58:
	   case SLANG_BC_UNUSED_0x59:
	   case SLANG_BC_UNUSED_0x5A:
	   case SLANG_BC_UNUSED_0x5B:
	   case SLANG_BC_UNUSED_0x5C:
	   case SLANG_BC_UNUSED_0x5D:
	   case SLANG_BC_UNUSED_0x5E:
	   case SLANG_BC_UNUSED_0x5F:
	     _pSLang_verror (SL_INTERNAL_ERROR, "Byte-Code 0x%X is not valid", addr->bc_main_type);
	     break;
#endif
	   case SLANG_BC_TMP:
	     tmp_variable_function (addr);
	     break;
	   case SLANG_BC_EXCH:
	     (void) SLreverse_stack (2);
	     break;
	   case SLANG_BC_LABEL:
	       {
		  int test;
		  if ((0 == pop_int (&test))
		      && (test == 0))
		    goto return_0;
	       }
	     break;

	   case SLANG_BC_BLOCK:
#if SLANG_OPTIMIZE_FOR_SPEED
	     while ((addr->bc_main_type == SLANG_BC_BLOCK)
		    && (addr->bc_sub_type == 0))
	       {
		  if (block == NULL)
		    block = addr;

		  addr++;
	       }
	     if (addr->bc_main_type != SLANG_BC_BLOCK)
	       {
		  addr--;
		  break;
	       }
#endif
	     switch (addr->bc_sub_type) /*{{{*/
	       {
		case SLANG_BCST_ERROR_BLOCK:
		  err_block = addr;
		  break;

		case SLANG_BCST_EXIT_BLOCK:
		  Exit_Block_Ptr = addr->b.blk;
		  break;

		case SLANG_BCST_USER_BLOCK0:
		case SLANG_BCST_USER_BLOCK1:
		case SLANG_BCST_USER_BLOCK2:
		case SLANG_BCST_USER_BLOCK3:
		case SLANG_BCST_USER_BLOCK4:
		  User_Block_Ptr[addr->bc_sub_type - SLANG_BCST_USER_BLOCK0] = addr->b.blk;
		  break;

		case SLANG_BCST_LOOP:
		case SLANG_BCST_WHILE:
		case SLANG_BCST_FOR:
		case SLANG_BCST_FOREVER:
		case SLANG_BCST_CFOR:
		case SLANG_BCST_DOWHILE:
		case SLANG_BCST_FOREACH:
		case SLANG_BCST_FOREACH_EARGS:
		    {
		       int status;
		       SLBlock_Type *addr1 = addr + 1;
		       if (block == NULL) block = addr;

		       status = lang_do_loops(addr->bc_sub_type, block, 1 + (unsigned int) (addr - block));
		       block = NULL;
		       while (addr1->bc_main_type == SLANG_BC_BLOCK)
			 {
			    if (addr1->bc_sub_type == SLANG_BCST_LOOP_THEN)
			      {
				 addr = addr1;
				 if (status == 0)
				   inner_interp (addr->b.blk);
				 addr1++;
				 continue;
			      }
#ifdef LOOP_ELSE_TOKEN
			    if (addr1->bc_sub_type == SLANG_BCST_LOOP_ELSE)
			      {
				 addr = addr1;
				 if (status == 1)
				   inner_interp (addr->b.blk);
				 addr1++;
				 continue;
			      }
#endif
			    break;
			 }
		    }
		  break;

		case SLANG_BCST_IFNOT:
#if SLANG_OPTIMIZE_FOR_SPEED
		    {
		       int i;

		       if ((0 == pop_ctrl_integer (&i)) && (i == 0))
			 inner_interp (addr->b.blk);
		    }
#else
		  do_else_if (addr, NULL);
#endif
		  break;

		case SLANG_BCST_IF:
#if SLANG_OPTIMIZE_FOR_SPEED
		    {
		       int i;

		       if ((0 == pop_ctrl_integer (&i)) && i)
			 inner_interp (addr->b.blk);
		    }
#else
		  do_else_if (NULL, addr);
#endif
		  break;

		case SLANG_BCST_NOTELSE:
		  do_else_if (block, addr);
		  block = NULL;
		  break;

		case SLANG_BCST_ELSE:
		  do_else_if (addr, block);
		  block = NULL;
		  break;

		case SLANG_BCST_SWITCH:
		  if (Switch_Obj_Ptr == Switch_Obj_Max)
		    {
		       _pSLang_verror (SL_BUILTIN_LIMIT_EXCEEDED, "switch nesting too deep");
		       break;
		    }
		  (void) pop_object(Switch_Obj_Ptr);
		  Switch_Obj_Ptr++;

		  if (block == NULL) block = addr;
		  while ((0 == IS_SLANG_ERROR)
			 && (block <= addr)
			 && (Lang_Break_Condition == 0)
			 && (0 == inner_interp (block->b.blk)))
		    block++;
		  Switch_Obj_Ptr--;
		  SLang_free_object (Switch_Obj_Ptr);
		  Switch_Obj_Ptr->o_data_type = 0;
		  block = NULL;
		  break;

		case SLANG_BCST_SC_AND:
		    {
		       int i;
		       if ((0 == pop_ctrl_integer (&i)) && (i == 0))
			 {
			    (void) push_char_object (SLANG_CHAR_TYPE, 0);
			    block = NULL;
			    break;
			 }
		    }
		  /* drop */
		case SLANG_BCST_ANDELSE:
		  if (block == NULL) block = addr;
		  lang_do_and_orelse (0, block, addr);
		  block = NULL;
		  break;

		case SLANG_BCST_SC_OR:
		    {
		       int i;
		       if ((0 == pop_ctrl_integer (&i)) && i)
			 {
			    (void) push_char_object (SLANG_CHAR_TYPE, (char)i);
			    block = NULL;
			    break;
			 }
		    }
		  /* drop */
		case SLANG_BCST_ORELSE:
		  if (block == NULL) block = addr;
		  lang_do_and_orelse (1, block, addr);
		  block = NULL;
		  break;

		case SLANG_BCST_TRY:
		  do_try (block, addr);
		  block = NULL;
		  break;

		case SLANG_BCST_COMPARE:
		  do_compare (addr->b.blk);
		  block = NULL;
		  break;

		default:
		  if (block == NULL) block =  addr;
		  break;
	       }

/*}}}*/

	     if (Lang_Break_Condition) goto handle_break_condition;
	     break;
	  /* End of SLANG_BC_BLOCK */

	   case SLANG_BC_RETURN:
	     Lang_Break_Condition = Lang_Return = Lang_Break = 1; goto return_1;
	   case SLANG_BC_BREAK:
	     Lang_Break_Condition = Lang_Break = 1; goto return_1;
	   case SLANG_BC_CONTINUE:
	     Lang_Break_Condition = /* Lang_Continue = */ 1; goto return_1;

#if USE_UNUSED_BYCODES_IN_SWITCH
	   case SLANG_BC_UNUSED_0x67:
	     _pSLang_verror (SL_INTERNAL_ERROR, "Byte-Code 0x%X is not valid", addr->bc_main_type);
	     break;
#endif
	   case SLANG_BC_BREAK_N:
	     Lang_Break_Condition = Lang_Break = addr->b.i_blk;
	     goto return_1;

	   case SLANG_BC_CONTINUE_N:
	     Lang_Break = -(addr->b.i_blk - 1);
	     Lang_Break_Condition = 1;
	     goto return_1;

	   case SLANG_BC_X_ERROR:
	     if (err_block != NULL)
	       {
		  inner_interp(err_block->b.blk);
		  if (SLang_get_error ()) err_block = NULL;
	       }
	     else _pSLang_verror(SL_SYNTAX_ERROR, "No ERROR_BLOCK");
	     if (Lang_Break_Condition) goto handle_break_condition;
	     break;

	   case SLANG_BC_X_USER0:
	   case SLANG_BC_X_USER1:
	   case SLANG_BC_X_USER2:
	   case SLANG_BC_X_USER3:
	   case SLANG_BC_X_USER4:
	     if (User_Block_Ptr[addr->bc_main_type - SLANG_BC_X_USER0] != NULL)
	       {
		  inner_interp(User_Block_Ptr[addr->bc_main_type - SLANG_BC_X_USER0]);
	       }
	     else _pSLang_verror(SL_SYNTAX_ERROR, "No block for X_USERBLOCK");
	     if (Lang_Break_Condition) goto handle_break_condition;
	     break;

	   case SLANG_BC_CALL_DIRECT:
	     (*addr->b.call_function) ();
	     break;

	   case SLANG_BC_CALL_DIRECT_FRAME:
	     do_bc_call_direct_frame (addr->b.call_function);
	     break;

	   case SLANG_BC_CALL_DIRECT_NARGS:
	     do_bc_call_direct_nargs (addr->b.call_function);
	     break;

	   case SLANG_BC_EARG_LVARIABLE:
	     PUSH_LOCAL_VARIABLE(addr->b.i_blk)
	     (void) end_arg_list ();
	     break;
#if USE_BC_LINE_NUM
	   case SLANG_BC_LINE_NUM:
	     break;
#else
# if USE_UNUSED_BYCODES_IN_SWITCH
	   case SLANG_BC_UNUSED_0x74:
	     _pSLang_verror (SL_INTERNAL_ERROR, "Byte-Code 0x%X is not valid", addr->bc_main_type);
	     break;
# endif
#endif
	   case SLANG_BC_BOS:
#if SLANG_HAS_BOSEOS
	     BOS_Stack_Depth++;
	     This_Compile_Linenum = addr->b.line_info->linenum;
	     (void) _pSLcall_bos_handler (addr->b.line_info->filename, addr->b.line_info->linenum);
#endif
	     break;
	   case SLANG_BC_EOS:
#if SLANG_HAS_BOSEOS
	     This_Compile_Linenum = addr->linenum;
	     (void) _pSLcall_eos_handler ();
	     BOS_Stack_Depth--;
#endif
	     break;

#if USE_UNUSED_BYCODES_IN_SWITCH
	   case SLANG_BC_UNUSED_0x77:
	   case SLANG_BC_UNUSED_0x78:
	   case SLANG_BC_UNUSED_0x79:
	   case SLANG_BC_UNUSED_0x7A:
	   case SLANG_BC_UNUSED_0x7B:
	   case SLANG_BC_UNUSED_0x7C:
	   case SLANG_BC_UNUSED_0x7D:
	   case SLANG_BC_UNUSED_0x7E:
	   case SLANG_BC_UNUSED_0x7F:
	     _pSLang_verror (SL_INTERNAL_ERROR, "Byte-Code 0x%X is not valid", addr->bc_main_type);
	     break;
#endif

#if USE_COMBINED_BYTECODES
	   case SLANG_BC_CALL_DIRECT_INTRINSIC:
	     (*addr->b.call_function) ();
	     addr++;
	     EXECUTE_INTRINSIC(addr)
	     if (IS_SLANG_ERROR)
	       do_traceback(addr->b.nt_ifun_blk->name);
	     break;

	   case SLANG_BC_INTRINSIC_CALL_DIRECT:
	     EXECUTE_INTRINSIC(addr)
	     if (IS_SLANG_ERROR)
	       {
		  do_traceback(addr->b.nt_ifun_blk->name);
		  break;
	       }
	     addr++;
	     (*addr->b.call_function) ();
	     break;

	   case SLANG_BC_CALL_DIRECT_LSTR:
	     (*addr->b.call_function) ();
	     addr++;
	     _pSLang_dup_and_push_slstring (addr->b.s_blk);
	     break;

	   case SLANG_BC_CALL_DIRECT_SLFUN:
	     (*addr->b.call_function) ();
	     addr++;
	     execute_slang_fun (addr->b.nt_fun_blk, addr->linenum);
	     if (Lang_Break_Condition) goto handle_break_condition;
	     break;

	   case SLANG_BC_CALL_DIRECT_RETINTR:
	     (*addr->b.call_function) ();
	     addr++;
	     /* drop */
	   case SLANG_BC_RET_INTRINSIC:
	     EXECUTE_INTRINSIC (addr)
	     if (0 == Handle_Interrupt)
	       return 1;
	     if (IS_SLANG_ERROR)
	       do_traceback(addr->b.nt_ifun_blk->name);
	     break;

	   case SLANG_BC_CALL_DIRECT_EARG_LVAR:
	     (*addr->b.call_function) ();
	     addr++;
	     PUSH_LOCAL_VARIABLE (addr->b.i_blk)
	     (void) end_arg_list ();
	     break;

	   case SLANG_BC_CALL_DIRECT_LINT:
	     (*addr->b.call_function) ();
	     addr++;
	     push_int_object (addr->bc_sub_type, (int) addr->b.l_blk);
	     break;

	   case SLANG_BC_CALL_DIRECT_LVAR:
	     (*addr->b.call_function) ();
	     addr++;
	     PUSH_LOCAL_VARIABLE (addr->b.i_blk)
	     break;

	   case SLANG_BC_LLVARIABLE_BINARY:
	       {
		  SLang_Object_Type *obj1 = Local_Variable_Frame - (addr+1)->b.i_blk;
		  SLang_Object_Type *obj2 = Local_Variable_Frame - (addr+2)->b.i_blk;
		  if (obj1->o_data_type == obj2->o_data_type)
		    {
		       if (obj1->o_data_type == SLANG_INT_TYPE)
			 (void) int_int_binary (addr->b.i_blk, obj1, obj2);
# if SLANG_HAS_FLOAT
		       else if (obj1->o_data_type == SLANG_DOUBLE_TYPE)
			 (void) dbl_dbl_binary (addr->b.i_blk, obj1, obj2);
# endif
		       else
			 (void) do_binary_ab_inc_ref (addr->b.i_blk, obj1, obj2);
		    }
		  else do_binary_ab_inc_ref (addr->b.i_blk, obj1, obj2);
		  addr += 2;
	       }
	     break;

	   case SLANG_BC_LGVARIABLE_BINARY:
	     do_binary_ab_inc_ref (addr->b.i_blk,
			   Local_Variable_Frame - (addr+1)->b.i_blk,
			   &(addr+2)->b.nt_gvar_blk->obj);
	     addr += 2;
	     break;

	   case SLANG_BC_GLVARIABLE_BINARY:
	     do_binary_ab_inc_ref (addr->b.i_blk,
			   &(addr+1)->b.nt_gvar_blk->obj,
			   Local_Variable_Frame - (addr+2)->b.i_blk);
	     addr += 2;
	     break;
	   case SLANG_BC_GGVARIABLE_BINARY:
	     do_binary_ab_inc_ref (addr->b.i_blk,
			   &(addr+1)->b.nt_gvar_blk->obj,
			   &(addr+2)->b.nt_gvar_blk->obj);
	     addr += 2;
	     break;

	   case SLANG_BC_LIVARIABLE_BINARY:
	       {
		  SLang_Object_Type o, *obj1;
		  o.o_data_type = SLANG_INT_TYPE;
		  o.v.int_val = (int) (addr+2)->b.l_blk;
		  obj1 = Local_Variable_Frame - (addr+1)->b.i_blk;
		  if (obj1->o_data_type == SLANG_INT_TYPE)
		    (void) int_int_binary (addr->b.i_blk, obj1, &o);
		  else
		    do_binary_ab_inc_ref (addr->b.i_blk, obj1, &o);
	       }
	     addr += 2;
	     break;
# if SLANG_HAS_FLOAT
	   case SLANG_BC_LDVARIABLE_BINARY:
	       {
		  SLang_Object_Type o, *obj1;
		  o.o_data_type = SLANG_DOUBLE_TYPE;
		  o.v.double_val = *(addr+2)->b.double_blk;
		  obj1 = Local_Variable_Frame - (addr+1)->b.i_blk;
		  if (obj1->o_data_type == SLANG_DOUBLE_TYPE)
		    (void) dbl_dbl_binary (addr->b.i_blk, obj1, &o);
		  else
		    do_binary_ab_inc_ref (addr->b.i_blk, obj1, &o);
	       }
	     addr += 2;
	     break;
# endif
	   case SLANG_BC_ILVARIABLE_BINARY:
	       {
		  SLang_Object_Type o, *obj1;
		  o.o_data_type = SLANG_INT_TYPE;
		  o.v.int_val = (int) (addr+1)->b.l_blk;
		  obj1 = Local_Variable_Frame - (addr+2)->b.i_blk;
		  if (obj1->o_data_type == SLANG_INT_TYPE)
		    (void) int_int_binary (addr->b.i_blk, &o, obj1);
		  else
		    (void) do_binary_ab_inc_ref (addr->b.i_blk, &o, obj1);
	       }
	     addr += 2;
	     break;
# if SLANG_HAS_FLOAT
	   case SLANG_BC_DLVARIABLE_BINARY:
	       {
		  SLang_Object_Type o, *obj1;
		  o.o_data_type = SLANG_DOUBLE_TYPE;
		  o.v.double_val = *(addr+1)->b.double_blk;
		  obj1 = Local_Variable_Frame - (addr+2)->b.i_blk;
		  if (obj1->o_data_type == SLANG_DOUBLE_TYPE)
		    (void) dbl_dbl_binary (addr->b.i_blk, &o, obj1);
		  else
		    (void) do_binary_ab_inc_ref (addr->b.i_blk, &o, obj1);

	       }
	     addr += 2;
	     break;
# endif
	   case SLANG_BC_LVARIABLE_BINARY:
	     do_binary_b_inc_ref (addr->b.i_blk,
			  Local_Variable_Frame - (addr+1)->b.i_blk);
	     addr++;
	     break;

	   case SLANG_BC_GVARIABLE_BINARY:
	     do_binary_b_inc_ref (addr->b.i_blk,
			  &(addr+1)->b.nt_gvar_blk->obj);
	     addr++;
	     break;

	   case SLANG_BC_LITERAL_INT_BINARY:
	       {
		  SLang_Object_Type o;
		  o.o_data_type = SLANG_INT_TYPE;
		  o.v.int_val = (int) (addr+1)->b.l_blk;
		  (void) do_binary_b (addr->b.i_blk, &o);
	       }
	     addr++;
	     break;
# if SLANG_HAS_FLOAT
	   case SLANG_BC_LITERAL_DBL_BINARY:
	       {
		  SLang_Object_Type o;
		  o.o_data_type = SLANG_DOUBLE_TYPE;
		  o.v.double_val = *(addr+1)->b.double_blk;
		  (void) do_binary_b (addr->b.i_blk, &o);
	       }
	     addr++;
	     break;
# endif
	   case SLANG_BC_LASSIGN_LLBINARY:
	     (void) do_binary_ab_inc_ref_assign ((addr+1)->b.i_blk,
						 Local_Variable_Frame - (addr+2)->b.i_blk,
						 Local_Variable_Frame - (addr+3)->b.i_blk,
						 Local_Variable_Frame - addr->b.i_blk);
	     addr += 3;
	     break;

	   case SLANG_BC_LASSIGN_LIBINARY:
	       {
		  SLang_Object_Type o;
		  o.o_data_type = SLANG_INT_TYPE;
		  o.v.int_val = (int) (addr+3)->b.l_blk;

		  (void) do_binary_ab_inc_ref_assign ((addr+1)->b.i_blk,
						      Local_Variable_Frame - (addr+2)->b.i_blk,
						      &o,
						      Local_Variable_Frame - addr->b.i_blk);
	       }
	     addr += 3;
	     break;
	   case SLANG_BC_LASSIGN_ILBINARY:
	       {
		  SLang_Object_Type o;
		  o.o_data_type = SLANG_INT_TYPE;
		  o.v.int_val = (int) (addr+2)->b.l_blk;

		  (void) do_binary_ab_inc_ref_assign ((addr+1)->b.i_blk,
						      &o,
						      Local_Variable_Frame - (addr+3)->b.i_blk,
						      Local_Variable_Frame - addr->b.i_blk);
	       }
	     addr += 3;
	     break;
# if SLANG_HAS_FLOAT
	   case SLANG_BC_LASSIGN_LDBINARY:
	       {
		  SLang_Object_Type o;
		  o.o_data_type = SLANG_DOUBLE_TYPE;
		  o.v.double_val = *(addr+3)->b.double_blk;

		  (void) do_binary_ab_inc_ref_assign ((addr+1)->b.i_blk,
						      Local_Variable_Frame - (addr+2)->b.i_blk,
						      &o,
						      Local_Variable_Frame - addr->b.i_blk);
	       }
	     addr += 3;
	     break;
	   case SLANG_BC_LASSIGN_DLBINARY:
	       {
		  SLang_Object_Type o;
		  o.o_data_type = SLANG_DOUBLE_TYPE;
		  o.v.double_val = *(addr+2)->b.double_blk;

		  (void) do_binary_ab_inc_ref_assign ((addr+1)->b.i_blk,
						      &o,
						      Local_Variable_Frame - (addr+3)->b.i_blk,
						      Local_Variable_Frame - addr->b.i_blk);
	       }
	     addr += 3;
	     break;
# endif
	   case SLANG_BC_RET_LVARIABLE:
	     if (0 != push_local_variable (addr->b.i_blk))
	       break;
	     Lang_Break_Condition = Lang_Return = Lang_Break = 1;
	     goto return_1;

	   case SLANG_BC_RET_LITERAL_INT:
	     if (-1 == push_int_object (addr->bc_sub_type, (int) addr->b.l_blk))
	       break;
	     Lang_Break_Condition = Lang_Return = Lang_Break = 1;
	     goto return_1;

	   case SLANG_BC_MANY_LVARIABLE:
	     (void) push_local_variable (addr->b.i_blk);
	     addr++;
	     (void) push_local_variable (addr->b.i_blk);
	     addr++;
	     while (addr->bc_main_type == SLANG_BC_LVARIABLE_COMBINED)
	       {
		  (void) push_local_variable (addr->b.i_blk);
		  addr++;
	       }
	     addr--;
	     break;

	   case SLANG_BC_MANY_LVARIABLE_DIR:
	     (void) push_local_variable (addr->b.i_blk);
	     addr++;
	     (void) push_local_variable (addr->b.i_blk);
	     addr++;
	     while (addr->bc_main_type == SLANG_BC_LVARIABLE_COMBINED)
	       {
		  (void) push_local_variable (addr->b.i_blk);
		  addr++;
	       }
	     (*addr->b.call_function) ();
	     break;

	   case SLANG_BC_LVARIABLE_AGET1:
	       {
		  SLang_Object_Type *o;

		  addr++;		       /* not used */
		  o = (Local_Variable_Frame - addr->b.i_blk);
		  if (o->o_data_type == SLANG_INT_TYPE)
		    {
		       addr++;
		       (void) push_array_element (addr->b.i_blk, o->v.int_val);
		       break;
		    }
		  if (-1 == push_local_variable (addr->b.i_blk))
		    break;
		  addr++;
		  if (-1 == push_local_variable (addr->b.i_blk))
		    break;
		  (void) _pSLarray_aget1 (1);
		  break;
	       }

	   case SLANG_BC_LITERAL_AGET1:
	     addr++;		       /* not used */
	     push_array_element ((addr+1)->b.i_blk, (int) addr->b.l_blk);
	     addr++;
	     break;

	   case SLANG_BC_LVAR_LVAR_APUT1:
	     if (-1 == push_local_variable (addr->b.i_blk))
	       break;
	     addr++;
	     /* drop */
	   case SLANG_BC_LVARIABLE_APUT1:
	       {
		  SLang_Object_Type *o;

		  addr++;		       /* not used */
		  o = (Local_Variable_Frame - addr->b.i_blk);
		  if (o->o_data_type == SLANG_INT_TYPE)
		    {
		       addr++;
		       (void) pop_to_lvar_array_element (addr->b.i_blk, o->v.int_val);
		       break;
		    }
		  if (-1 == push_local_variable (addr->b.i_blk))
		    break;
		  addr++;
		  if (-1 == push_local_variable (addr->b.i_blk))
		    break;
		  (void) _pSLarray_aput1 (1);
	       }
	     break;

	   case SLANG_BC_LITERAL_APUT1:
	     (void) pop_to_lvar_array_element ((addr+2)->b.i_blk, (SLindex_Type)((addr+1)->b.l_blk));
	     addr += 2;
	     break;

	   case SLANG_BC_LLVARIABLE_BINARY2:
	       {
		  SLang_Object_Type *obj1 = Local_Variable_Frame - (addr+1)->b.i_blk;
		  SLang_Object_Type *obj2 = Local_Variable_Frame - (addr+2)->b.i_blk;
		  SLang_Object_Type obj3;

		  if (obj1->o_data_type == obj2->o_data_type)
		    {
		       if (obj1->o_data_type == SLANG_INT_TYPE)
			 {
			    if (-1 == int_int_binary_result (addr->b.i_blk, obj1, obj2, &obj3))
			      break;
			    addr += 3;
			    (void) do_binary_b (addr->b.i_blk, &obj3);
			    break;
			 }
# if SLANG_HAS_FLOAT
		       else if (obj1->o_data_type == SLANG_DOUBLE_TYPE)
			 {
			    if (-1 == dbl_dbl_binary_result (addr->b.i_blk, obj1, obj2, &obj3))
			      break;
			    addr += 3;
			    (void) do_binary_b (addr->b.i_blk, &obj3);
			    break;
			 }
# endif
		    }

		  obj3.o_data_type = SLANG_UNDEFINED_TYPE;
		  if (-1 == do_binary_ab_inc_ref_assign (addr->b.i_blk, obj1, obj2, &obj3))
		    break;
		  addr += 3;
		  (void) do_binary_b (addr->b.i_blk, &obj3);
		  SLang_free_object (&obj3);
	       }
	     break;

	   case SLANG_BC_SET_LOCLV_LIT_INT:
	     set_lvalue_obj (addr->bc_sub_type, Local_Variable_Frame - addr->b.i_blk);
	     addr++;
	     push_int_object (addr->bc_sub_type, (int) addr->b.l_blk);
	     break;

	   case SLANG_BC_SET_LOCLV_LIT_AGET1:
	     set_lvalue_obj (addr->bc_sub_type, Local_Variable_Frame - addr->b.i_blk);
	     addr += 3;
	     push_array_element (addr->b.i_blk, (int) (addr-1)->b.l_blk);
	     break;

	   case SLANG_BC_SET_LOCLV_LVAR:
	     set_lvalue_obj (addr->bc_sub_type, Local_Variable_Frame - addr->b.i_blk);
	     addr++;
	     PUSH_LOCAL_VARIABLE (addr->b.i_blk)
	     break;

	   case SLANG_BC_SET_LOCLV_LASTBLOCK:
	     if (0 == set_lvalue_obj (addr->bc_sub_type, Local_Variable_Frame - addr->b.i_blk))
	       goto return_1;
	     break;

	   case SLANG_BC_LVAR_EARG_LVAR:
	     PUSH_LOCAL_VARIABLE (addr->b.i_blk);
	     addr++;
	     PUSH_LOCAL_VARIABLE(addr->b.i_blk);
	     (void) end_arg_list ();
	     break;

	   case SLANG_BC_LVAR_FIELD:
	     PUSH_LOCAL_VARIABLE (addr->b.i_blk);
	     addr++;
	     (void) push_struct_field (addr->b.s_blk);
	     break;

	   case SLANG_BC_BINARY_LASTBLOCK:
	     if (-1 == do_binary (addr->b.i_blk))
	       break;
	     goto return_1;

	   case SLANG_BC_EARG_LVARIABLE_INTRINSIC:
	     PUSH_LOCAL_VARIABLE(addr->b.i_blk);
	     if (0 == end_arg_list ())
	       {
		  addr++;
		  EXECUTE_INTRINSIC(addr)
		  if (IS_SLANG_ERROR)
		    do_traceback(addr->b.nt_ifun_blk->name);
	       }
	     break;

	   case SLANG_BC_LVAR_LITERAL_INT:
	     PUSH_LOCAL_VARIABLE (addr->b.i_blk);
	     addr++;
	     push_int_object (addr->bc_sub_type, (int) addr->b.l_blk);
	     break;

	   case SLANG_BC_BINARY_SET_LOCLVAL:
	     if (-1 == do_binary (addr->b.i_blk))
	       break;
	     addr++;
	     (void) set_lvalue_obj (addr->bc_sub_type, Local_Variable_Frame - addr->b.i_blk);
	     break;

	   case SLANG_BC_LVAR_AGET_SET_LOCLVAL:
	       {
		  SLang_Object_Type *obj = Local_Variable_Frame - addr->b.i_blk;
		  if ((0 == carefully_push_object (obj))
		      && (0 == do_bc_call_direct_nargs (_pSLarray_aget)))
		    {
		       addr++;
		       (void) set_lvalue_obj (addr->bc_sub_type, Local_Variable_Frame - addr->b.i_blk);
		    }
	       }
	     break;

	   case SLANG_BC_LLVAR_BINARY_IF:
	       {
		  SLang_Object_Type *obj1 = Local_Variable_Frame - (addr+1)->b.i_blk;
		  SLang_Object_Type *obj2 = Local_Variable_Frame - (addr+2)->b.i_blk;
		  SLang_Object_Type obj3;

		  if (obj1->o_data_type == obj2->o_data_type)
		    {
		       if (obj1->o_data_type == SLANG_INT_TYPE)
			 {
			    if (-1 == int_int_binary_result (addr->b.i_blk, obj1, obj2, &obj3))
			      break;
			 }
# if SLANG_HAS_FLOAT
		       else if (obj1->o_data_type == SLANG_DOUBLE_TYPE)
			 {
			    if (-1 == dbl_dbl_binary_result (addr->b.i_blk, obj1, obj2, &obj3))
			      break;
			 }
# endif
		       else
			 {
			    obj3.o_data_type = SLANG_UNDEFINED_TYPE;
			    if (-1 == do_binary_ab_inc_ref_assign (addr->b.i_blk, obj1, obj2, &obj3))
			      break;
			 }
		    }
		  else
		    {
		       obj3.o_data_type = SLANG_UNDEFINED_TYPE;
		       if (-1 == do_binary_ab_inc_ref_assign (addr->b.i_blk, obj1, obj2, &obj3))
			 break;
		    }
		  addr += 3;
		  if (obj3.o_data_type == SLANG_CHAR_TYPE)
		    {
		       if (obj3.v.char_val)
			 goto execute_BC_IF_BLOCK;
		       break;
		    }
		  if (obj3.o_data_type == SLANG_INT_TYPE)
		    {
		       if (obj3.v.int_val)
			 goto execute_BC_IF_BLOCK;
		       break;
		    }

		  /* Otherwise let pop_ctrl_integer do the dirty work */
		  if (-1 == push_object (&obj3))
		    break;
	       }
	     /* drop */
	   case SLANG_BC_IF_BLOCK:
	       {
		  int i;

		  if ((-1 == pop_ctrl_integer (&i)) || (i == 0))
		    break;
	       }

execute_BC_IF_BLOCK:
	       {
		  SLBlock_Type *addr1 = addr->b.blk;
		  if (addr1->bc_main_type == SLANG_BC_RETURN)
		    {
		       Lang_Break_Condition = Lang_Return = Lang_Break = 1;
		       goto return_1;
		    }
		  if (addr1->bc_main_type == SLANG_BC_RET_LVARIABLE)
		    {
		       if (0 != push_local_variable (addr1->b.i_blk))
			 break;
		       Lang_Break_Condition = Lang_Return = Lang_Break = 1;
		       goto return_1;
		    }
		  inner_interp (addr1);
		  if (Lang_Break_Condition) goto handle_break_condition;
	       }
	     break;

	   case SLANG_BC_LVAR_SET_FIELD:
	     (void) set_struct_obj_lvalue (addr+1, Local_Variable_Frame - addr->b.i_blk, 0);
	     addr++;
	     break;

	   case SLANG_BC_PVAR_SET_GLOB_LVAL:
	     addr++;
	     if (-1 == set_lvalue_obj_with_obj (addr->bc_sub_type,
						&addr->b.nt_gvar_blk->obj,
						&(addr-1)->b.nt_gvar_blk->obj))
	       do_name_type_error (addr->b.nt_blk);
	     break;
	   case SLANG_BC_LVAR_SET_GLOB_LVAL:
	     addr++;
	     if (-1 == set_lvalue_obj_with_obj (addr->bc_sub_type,
						&addr->b.nt_gvar_blk->obj,
						Local_Variable_Frame - (addr-1)->b.i_blk))
	       do_name_type_error (addr->b.nt_blk);
	     break;

	   case SLANG_BC_LIT_AGET1_INT_BINARY:
	       {
		  SLang_Object_Type o;
		  o.o_data_type = SLANG_INT_TYPE;
		  o.v.int_val = (int)(addr+4)->b.l_blk;
		  (void) aget1_from_lvar_binary ((addr+2)->b.i_blk, (SLindex_Type) (addr+1)->b.l_blk,
						 (addr+3)->b.i_blk, &o);
	       }
	     addr += 4;
	     break;
	   case SLANG_BC_BINARY2:
	     if (0 == do_binary (addr->b.i_blk))
	       {
		  addr++;
		  (void) do_binary (addr->b.i_blk);
	       }
	     break;

	   case SLANG_BC_LVAR_LIT_AGET1:
	     PUSH_LOCAL_VARIABLE(addr->b.i_blk);
	     (void) push_array_element ((addr+3)->b.i_blk, (int)((addr+2)->b.l_blk));
	     addr += 3;
	     break;
#endif				       /* USE_COMBINED_BYTECODES */

#if USE_UNUSED_BYCODES_IN_SWITCH
# if !USE_COMBINED_BYTECODES
	   case SLANG_BC_CALL_DIRECT_INTRINSIC:
	   case SLANG_BC_INTRINSIC_CALL_DIRECT:
	   case SLANG_BC_CALL_DIRECT_LSTR:
	   case SLANG_BC_CALL_DIRECT_SLFUN:
	   case SLANG_BC_CALL_DIRECT_RETINTR:
	   case SLANG_BC_RET_INTRINSIC:
	   case SLANG_BC_CALL_DIRECT_EARG_LVAR:
	   case SLANG_BC_CALL_DIRECT_LINT:
	   case SLANG_BC_CALL_DIRECT_LVAR:
	   case SLANG_BC_LLVARIABLE_BINARY:
	   case SLANG_BC_LGVARIABLE_BINARY:
	   case SLANG_BC_GLVARIABLE_BINARY:
	   case SLANG_BC_GGVARIABLE_BINARY:
	   case SLANG_BC_LIVARIABLE_BINARY:
	   case SLANG_BC_LDVARIABLE_BINARY:
	   case SLANG_BC_ILVARIABLE_BINARY:
	   case SLANG_BC_DLVARIABLE_BINARY:
	   case SLANG_BC_LVARIABLE_BINARY:
	   case SLANG_BC_GVARIABLE_BINARY:
	   case SLANG_BC_LITERAL_INT_BINARY:
	   case SLANG_BC_LITERAL_DBL_BINARY:
	   case SLANG_BC_LASSIGN_LLBINARY:
	   case SLANG_BC_LASSIGN_LIBINARY:
	   case SLANG_BC_LASSIGN_ILBINARY:
	   case SLANG_BC_LASSIGN_LDBINARY:
	   case SLANG_BC_LASSIGN_DLBINARY:
	   case SLANG_BC_RET_LVARIABLE:
	   case SLANG_BC_RET_LITERAL_INT:
	   case SLANG_BC_MANY_LVARIABLE:
	   case SLANG_BC_MANY_LVARIABLE_DIR:
	   case SLANG_BC_LVARIABLE_AGET1:
	   case SLANG_BC_LITERAL_AGET1:
	   case SLANG_BC_LVAR_LVAR_APUT1:
	   case SLANG_BC_LVARIABLE_APUT1:
	   case SLANG_BC_LITERAL_APUT1:
	   case SLANG_BC_LLVARIABLE_BINARY2:
	   case SLANG_BC_SET_LOCLV_LIT_INT:
	   case SLANG_BC_SET_LOCLV_LIT_AGET1:
	   case SLANG_BC_SET_LOCLV_LVAR:
	   case SLANG_BC_SET_LOCLV_LASTBLOCK:
	   case SLANG_BC_LVAR_EARG_LVAR:
	   case SLANG_BC_LVAR_FIELD:
	   case SLANG_BC_BINARY_LASTBLOCK:
	   case SLANG_BC_EARG_LVARIABLE_INTRINSIC:
	   case SLANG_BC_LVAR_LITERAL_INT:
	   case SLANG_BC_BINARY_SET_LOCLVAL:
	   case SLANG_BC_LVAR_AGET_SET_LOCLVAL:
	   case SLANG_BC_LLVAR_BINARY_IF:
	   case SLANG_BC_IF_BLOCK:
	   case SLANG_BC_LVAR_SET_FIELD:
	   case SLANG_BC_PVAR_SET_GLOB_LVAL:
	   case SLANG_BC_LVAR_SET_GLOB_LVAL:
	   case SLANG_BC_LIT_AGET1_INT_BINARY:
	   case SLANG_BC_BINARY2:
	   case SLANG_BC_LVAR_LIT_AGET1:
# endif
	   case SLANG_BC_UNUSED_0xB7:
	   case SLANG_BC_UNUSED_0xB8:
	   case SLANG_BC_UNUSED_0xB9:
	   case SLANG_BC_UNUSED_0xBA:
	   case SLANG_BC_UNUSED_0xBB:
	   case SLANG_BC_UNUSED_0xBC:
	   case SLANG_BC_UNUSED_0xBD:
	   case SLANG_BC_UNUSED_0xBE:
	   case SLANG_BC_UNUSED_0xBF:
	   case SLANG_BC_LVARIABLE_COMBINED:
	   case SLANG_BC_GVARIABLE_COMBINED:
	   case SLANG_BC_LITERAL_COMBINED:
	   case SLANG_BC_CALL_DIRECT_COMB:
	   case SLANG_BC_COMBINED:
	   case SLANG_BC_BLOCK_COMBINED:
	   case SLANG_BC_UNUSED_0xC6:
	   case SLANG_BC_UNUSED_0xC7:
	   case SLANG_BC_UNUSED_0xC8:
	   case SLANG_BC_UNUSED_0xC9:
	   case SLANG_BC_UNUSED_0xCA:
	   case SLANG_BC_UNUSED_0xCB:
	   case SLANG_BC_UNUSED_0xCC:
	   case SLANG_BC_UNUSED_0xCD:
	   case SLANG_BC_UNUSED_0xCE:
	   case SLANG_BC_UNUSED_0xCF:
	   case SLANG_BC_UNUSED_0xD0:
	   case SLANG_BC_UNUSED_0xD1:
	   case SLANG_BC_UNUSED_0xD2:
	   case SLANG_BC_UNUSED_0xD3:
	   case SLANG_BC_UNUSED_0xD4:
	   case SLANG_BC_UNUSED_0xD5:
	   case SLANG_BC_UNUSED_0xD6:
	   case SLANG_BC_UNUSED_0xD7:
	   case SLANG_BC_UNUSED_0xD8:
	   case SLANG_BC_UNUSED_0xD9:
	   case SLANG_BC_UNUSED_0xDA:
	   case SLANG_BC_UNUSED_0xDB:
	   case SLANG_BC_UNUSED_0xDC:
	   case SLANG_BC_UNUSED_0xDD:
	   case SLANG_BC_UNUSED_0xDE:
	   case SLANG_BC_UNUSED_0xDF:
	   case SLANG_BC_UNUSED_0xE0:
	   case SLANG_BC_UNUSED_0xE1:
	   case SLANG_BC_UNUSED_0xE2:
	   case SLANG_BC_UNUSED_0xE3:
	   case SLANG_BC_UNUSED_0xE4:
	   case SLANG_BC_UNUSED_0xE5:
	   case SLANG_BC_UNUSED_0xE6:
	   case SLANG_BC_UNUSED_0xE7:
	   case SLANG_BC_UNUSED_0xE8:
	   case SLANG_BC_UNUSED_0xE9:
	   case SLANG_BC_UNUSED_0xEA:
	   case SLANG_BC_UNUSED_0xEB:
	   case SLANG_BC_UNUSED_0xEC:
	   case SLANG_BC_UNUSED_0xED:
	   case SLANG_BC_UNUSED_0xEE:
	   case SLANG_BC_UNUSED_0xEF:
	   case SLANG_BC_UNUSED_0xF0:
	   case SLANG_BC_UNUSED_0xF1:
	   case SLANG_BC_UNUSED_0xF2:
	   case SLANG_BC_UNUSED_0xF3:
	   case SLANG_BC_UNUSED_0xF4:
	   case SLANG_BC_UNUSED_0xF5:
	   case SLANG_BC_UNUSED_0xF6:
	   case SLANG_BC_UNUSED_0xF7:
	   case SLANG_BC_UNUSED_0xF8:
	   case SLANG_BC_UNUSED_0xF9:
	   case SLANG_BC_UNUSED_0xFA:
	   case SLANG_BC_UNUSED_0xFB:
	   case SLANG_BC_UNUSED_0xFC:
	   case SLANG_BC_UNUSED_0xFD:
	   case SLANG_BC_UNUSED_0xFE:
	   case SLANG_BC_UNUSED_0xFF:
#else
	   default:
#endif
	     _pSLang_verror (SL_INTERNAL_ERROR, "Byte-Code 0x%X is not valid", addr->bc_main_type);
	  }

	IF_UNLIKELY(Handle_Interrupt != 0)
	  {
	     if (SLang_get_error ())
	       {
		  if (-1 == do_inner_interp_error (err_block, addr_start, addr))
		    return 1;
		  if (SLang_get_error ())
		    return 1;
		  /* Otherwise, error cleared.  Continue onto next bytecode */
	       }
#if SLANG_HAS_SIGNALS
	     (void) check_signals ();
#endif
	     if (Lang_Break_Condition) goto handle_break_condition;
	  }
     }

   handle_break_condition:
   /* Get here if Lang_Break_Condition != 0, which implies that either
    * Lang_Return, Lang_Break, or Lang_Continue is non zero
    */
   if (Lang_Return)
     Lang_Break = 1;

return_1:
#if SLANG_HAS_SIGNALS
   if (Handle_Interrupt)
     check_signals ();
#endif
   return 1;

return_0:
#if SLANG_HAS_SIGNALS
   if (Handle_Interrupt)
     check_signals ();
#endif
   return 0;
}

/*}}}*/

/* The functions below this point are used to implement the parsed token
 * to byte-compiled code.
 */
/* static SLang_Name_Type **Static_Hash_Table; */

/* static SLang_Name_Type **Locals_Hash_Table; */
int _pSLang_Auto_Declare_Globals = 0;
int (*SLang_Auto_Declare_Var_Hook) (SLFUTURE_CONST char *);

static int Local_Variable_Number;
static unsigned int Function_Args_Number;

static int Lang_Defining_Function;
static void (*Default_Variable_Mode) (_pSLang_Token_Type *);
static void (*Default_Define_Function) (SLFUTURE_CONST char *, unsigned long);
static int setup_default_compile_linkage (int);

static int push_compile_context (SLFUTURE_CONST char *);
static int pop_compile_context (void);

typedef struct
{
   int block_type;
   SLBlock_Type *block;		       /* beginning of block definition */
   SLBlock_Type *block_ptr;	       /* current location */
   SLBlock_Type *block_max;	       /* end of definition */
   SLang_NameSpace_Type *static_namespace;
}
Block_Context_Type;

static Block_Context_Type Block_Context_Stack [SLANG_MAX_BLOCK_STACK_LEN];
static unsigned int Block_Context_Stack_Len;

static SLBlock_Type *Compile_ByteCode_Ptr;
static SLBlock_Type *This_Compile_Block;
static SLBlock_Type *This_Compile_Block_Max;

#define COMPILE_BLOCK_TYPE_NONE		0
#define COMPILE_BLOCK_TYPE_FUNCTION	1
#define COMPILE_BLOCK_TYPE_BLOCK	2
#define COMPILE_BLOCK_TYPE_TOP_LEVEL	3
static int This_Compile_Block_Type = COMPILE_BLOCK_TYPE_NONE;

/* If it returns 0, DO NOT FREE p */
static int lang_free_branch (SLBlock_Type *p)
{
   while (1)
     {
	SLang_Class_Type *cl;

        switch (p->bc_main_type)
	  {
	   case SLANG_BC_BLOCK:
	   case SLANG_BC_IF_BLOCK:
	   case SLANG_BC_BLOCK_COMBINED:
	     if (lang_free_branch(p->b.blk))
	       SLfree((char *)p->b.blk);
	     break;

	   case SLANG_BC_LITERAL:
	   case SLANG_BC_LITERAL_STR:
	   case SLANG_BC_LITERAL_DBL:
	   case SLANG_BC_LITERAL_COMBINED:
	     /* No user types should be here. */
	     GET_BUILTIN_CLASS(cl, p->bc_sub_type);
	     (*cl->cl_byte_code_destroy) (p->bc_sub_type, (VOID_STAR) &p->b.ptr_blk);
	     break;

	   case SLANG_BC_FIELD:
	   case SLANG_BC_FIELD_REF:
	   case SLANG_BC_SET_STRUCT_LVALUE:
	     SLang_free_slstring ((char *) p->b.s_blk);
	     break;
	   case SLANG_BC_BOS:
#if USE_BC_LINE_NUM
	   case SLANG_BC_LINE_NUM:
#endif
#if SLANG_HAS_DEBUG_CODE
	     if (p->b.line_info != NULL)
	       {
		  SLang_free_slstring (p->b.line_info->filename);
		  SLfree ((char *) p->b.line_info);
	       }
#endif
	     break;
	   default:
	     break;

	   case 0:
	     return 1;
	  }
	p++;
     }
}

static void free_local_variable_names (char **list, unsigned int num)
{
   unsigned int i;

   if (list == NULL)
     return;

   for (i = 0; i < num; i++)
     SLang_free_slstring (list[i]);

   SLfree ((char *) list);
}

static void free_function_header (Function_Header_Type *h)
{
   if (h->num_refs > 1)
     {
	h->num_refs--;
	return;
     }

   if (h->body != NULL)
     {
	if (lang_free_branch (h->body))
	  SLfree ((char *) h->body);
     }

   if (h->file != NULL) SLang_free_slstring ((char *) h->file);

   if (h->local_variables != NULL)
     free_local_variable_names (h->local_variables, h->nlocals);

   SLfree ((char *) h);
}

static Function_Header_Type *
  allocate_function_header (unsigned int nargs, unsigned int nlocals, SLFUTURE_CONST char *file)
{
   Function_Header_Type *h;
   char **local_variables;
   unsigned int i;

   if (NULL == (h = (Function_Header_Type *)SLcalloc (sizeof (Function_Header_Type), 1)))
     return h;

   h->num_refs = 1;
   /* h->body = NULL; */		       /* body added later */
   h->nlocals = nlocals;
   h->nargs = nargs;
   if (NULL == (h->file = SLang_create_slstring (file)))
     {
	free_function_header (h);
	return NULL;
     }

#if SLANG_HAS_BOSEOS
   h->issue_bofeof_info = (_pSLang_Compile_BOFEOF != 0);
#endif

   if (nlocals == 0)
     return h;

   if (NULL == (local_variables = (char **)SLcalloc (nlocals, sizeof (char *))))
     {
	free_function_header (h);
	return NULL;
     }
   h->local_variables = local_variables;

   for (i = 0; i < nlocals; i++)
     {
	if (NULL == (local_variables[i] = SLang_create_slstring (Local_Variable_Names[i])))
	  {
	     free_function_header (h);
	     return NULL;
	  }
     }
   return h;
}

static int push_block_context (int type)
{
   Block_Context_Type *c;
   unsigned int num;
   SLBlock_Type *b;

   if (Block_Context_Stack_Len == SLANG_MAX_BLOCK_STACK_LEN)
     {
	_pSLang_verror (SL_STACK_OVERFLOW, "Block stack overflow");
	return -1;
     }

   num = 20;
   if (NULL == (b = (SLBlock_Type *) _SLcalloc (num, sizeof (SLBlock_Type))))
     return -1;

   c = Block_Context_Stack + Block_Context_Stack_Len;
   c->block = This_Compile_Block;
   c->block_ptr = Compile_ByteCode_Ptr;
   c->block_max = This_Compile_Block_Max;
   c->block_type = This_Compile_Block_Type;
   c->static_namespace = This_Static_NameSpace;

   Compile_ByteCode_Ptr = This_Compile_Block = b;
   This_Compile_Block_Max = b + num;
   This_Compile_Block_Type = type;

   Block_Context_Stack_Len += 1;
   return 0;
}

static int pop_block_context (void)
{
   Block_Context_Type *c;

   if (Block_Context_Stack_Len == 0)
     {
	if (_pSLang_Error == 0)
	  SLang_verror (SL_StackUnderflow_Error, "block context stack underflow");
	return -1;
     }

   Block_Context_Stack_Len -= 1;
   c = Block_Context_Stack + Block_Context_Stack_Len;

   if (This_Compile_Block != NULL)
     {
	SLang_verror (SL_Internal_Error, "pop_block_context: block is not NULL");
     }

   This_Compile_Block = c->block;
   This_Compile_Block_Max = c->block_max;
   This_Compile_Block_Type = c->block_type;
   Compile_ByteCode_Ptr = c->block_ptr;
   This_Static_NameSpace = c->static_namespace;

   return 0;
}

static int setup_compile_namespaces (SLFUTURE_CONST char *name, SLFUTURE_CONST char *namespace_name)
{
   SLang_NameSpace_Type *static_ns = NULL, *private_ns = NULL;

   if (NULL == (private_ns = _pSLns_get_private_namespace (name, namespace_name)))
     return -1;

   /* The Global namespace is special.  It does not have the same semantics as
    * the static namespace since the Global namespace is _always_ available.
    */
   if ((namespace_name != NULL)
       && (*namespace_name)
       && (0 != strcmp (namespace_name, "Global")))
     {
	if (NULL == (static_ns = _pSLns_create_namespace2 (name, namespace_name)))
	  return -1;
     }
   else static_ns = private_ns;

   setup_default_compile_linkage (static_ns == private_ns);

   This_Static_NameSpace = static_ns;
   This_Private_NameSpace = private_ns;
   return 0;
}

int _pSLcompile_push_context (SLang_Load_Type *load_object)
{
   SLFUTURE_CONST char *name = load_object->name;
   char *ext;
   int status = -1;
   int free_name = 0;

   ext = SLpath_extname (name);
   if (((0 == strncmp (ext, ".slc", 4)) || (0 == strncmp (ext, ".SLC", 4)))
       && ((ext[4] == 0)
#ifdef VMS
	   || (ext[4] == ';')
#endif
	  ))
     {
	unsigned int len = (unsigned int) (ext - name) + 3;

	if (NULL == (name = SLang_create_nslstring (name, len)))
	  return -1;
	free_name = 1;
     }

   if (-1 == push_compile_context (name))
     goto free_return;

   if (-1 == setup_compile_namespaces (name, load_object->namespace_name))
     {
	pop_compile_context ();
	goto free_return;
     }

   if (-1 == push_block_context (COMPILE_BLOCK_TYPE_TOP_LEVEL))
     {
	pop_compile_context ();
	goto free_return;
     }

   (void) _pSLerr_suspend_messages ();
   status = 0;
   /* drop */

   free_return:
   if (free_name)
     SLang_free_slstring ((char *) name);

   return status;
}

static void reset_compiler_state (void);
int _pSLcompile_pop_context (void)
{
   (void) _pSLerr_resume_messages ();

   if (_pSLang_Error) reset_compiler_state ();

   if (This_Compile_Block_Type == COMPILE_BLOCK_TYPE_TOP_LEVEL)
     {
	Compile_ByteCode_Ptr->bc_main_type = SLANG_BC_LAST_BLOCK;
	if (lang_free_branch (This_Compile_Block))
	  {
	     SLfree ((char *) This_Compile_Block);
	     This_Compile_Block = NULL;
	  }
     }

   (void) pop_block_context ();
   (void) pop_compile_context ();

   if (This_Compile_Block == NULL)
     return 0;

#if 0
   if (This_Compile_Block_Type != COMPILE_BLOCK_TYPE_TOP_LEVEL)
     {
	_pSLang_verror (SL_INTERNAL_ERROR, "Not at top-level");
	return -1;
     }
#endif

   return 0;
}

/*{{{ Hash and Name Table Functions */

/* Returns a pointer to the first character past "->" in name.  If "->" does
 * not exist, then it returns a pointer to the beginning of name.
 */
_INLINE_
static char *parse_namespace_encoded_name (SLCONST char *name)
{
   char *ns;

   ns = (char *)name;
   name = strchr (name, '-');
   if ((name == NULL) || (name [1] != '>'))
     return ns;

   return (char*)name + 2;
}

static SLang_Name_Type *locate_namespace_encoded_name (SLCONST char *name, int err_on_bad_ns)
{
   char *ns;
   SLang_NameSpace_Type *table;
   SLang_Name_Type *nt;

   ns = (char *) name;
   name = parse_namespace_encoded_name (ns);

   if (name == ns)
     {
	/* Use Global Namespace */
	return _pSLns_locate_hashed_name (Global_NameSpace, name, _pSLcompute_string_hash (name));
     }

   ns = SLang_create_nslstring (ns, (unsigned int) ((name-2) - ns));
   if (ns == NULL)
     return NULL;
   if (NULL == (table = _pSLns_find_namespace (ns)))
     {
	if (err_on_bad_ns)
	  _pSLang_verror (SL_SYNTAX_ERROR, "Unable to find namespace called %s", ns);
	SLang_free_slstring (ns);
	return NULL;
     }
   SLang_free_slstring (ns);

   nt = _pSLns_locate_hashed_name (table, name, _pSLcompute_string_hash (name));
   if (nt == NULL)
     return NULL;

   switch (nt->name_type)
     {
	/* These are private and cannot be accessed through the namespace. */
      case SLANG_PVARIABLE:
      case SLANG_PFUNCTION:
	return NULL;
     }
   return nt;
}

static SLang_Name_Type *
  find_global_hashed_name (SLCONST char *name, unsigned long hash,
			   SLang_NameSpace_Type *pns, SLang_NameSpace_Type *sns,
			   SLang_NameSpace_Type *gns,
			   int do_error)
{
   SLang_Name_Type *nt;

   if ((pns != NULL) && (pns != sns))
     {
	if (NULL != (nt = _pSLns_locate_hashed_name (pns, name, hash)))
	  return nt;
     }
   if ((sns != NULL) && (sns != gns))
     {
	if (NULL != (nt = _pSLns_locate_hashed_name (sns, name, hash)))
	  return nt;
     }

   if (gns != NULL)
     {
	if (NULL != (nt = _pSLns_locate_hashed_name (gns, name, hash)))
	  return nt;
     }

   if (do_error)
     _pSLang_verror (SL_UNDEFINED_NAME, "Unable to locate '%s'", name);

   return NULL;
}

static SLang_Name_Type *locate_hashed_name (SLCONST char *name, unsigned long hash, int err_on_bad_ns)
{
   SLang_Name_Type *t;

   if (Locals_NameSpace != NULL)
     {
	t = _pSLns_locate_hashed_name (Locals_NameSpace, name, hash);
	if (t != NULL)
	  return t;
     }

   t = find_global_hashed_name (name, hash, This_Private_NameSpace, This_Static_NameSpace, Global_NameSpace, 0);
   if (t == NULL)
     t = locate_namespace_encoded_name (name, err_on_bad_ns);
   return t;
}

SLang_Name_Type *_pSLlocate_name (SLCONST char *name)
{
   return locate_hashed_name (name, _pSLcompute_string_hash (name), 0);
}

SLang_Name_Type *_pSLlocate_global_name (SLCONST char *name)
{
   return _pSLns_locate_name (Global_NameSpace, name);
}

static SLang_Name_Type *
  add_name_to_namespace (SLCONST char *name, unsigned long hash,
			 unsigned int sizeof_obj, unsigned char name_type,
			 SLang_NameSpace_Type *ns)
{
   SLang_Name_Type *t;

   if (-1 == _pSLcheck_identifier_syntax (name))
     return NULL;

   t = (SLang_Name_Type *) SLcalloc (sizeof_obj, 1);
   if (t == NULL)
     return t;

   t->name_type = name_type;

   if ((NULL == (t->name = _pSLstring_dup_hashed_string (name, hash)))
       || (-1 == _pSLns_add_hashed_name (ns, t, hash)))
     {
	SLfree ((char *) t);
	return NULL;
     }
   return t;
}

static SLang_Name_Type *
add_global_name (SLCONST char *name, unsigned long hash,
		 unsigned char name_type, unsigned int sizeof_obj,
		 SLang_NameSpace_Type *ns)
{
   SLang_Name_Type *nt;

   nt = _pSLns_locate_hashed_name (ns, name, hash);
   if (nt != NULL)
     {
	if (nt->name_type == name_type)
	  return nt;

	_pSLang_verror (SL_DUPLICATE_DEFINITION, "%s cannot be re-defined", name);
	return NULL;
     }

   return add_name_to_namespace (name, hash, sizeof_obj, name_type, ns);
}

static int add_intrinsic_function (SLang_NameSpace_Type *ns,
				   SLCONST char *name, FVOID_STAR addr, SLtype ret_type,
				   unsigned int nargs, SLtype *arg_types)
{
   SLang_Intrin_Fun_Type *f;
   unsigned int i;

   if (-1 == init_interpreter ())
     return -1;

   if (ns == NULL) ns = Global_NameSpace;

   if (ret_type == SLANG_FLOAT_TYPE)
     {
	_pSLang_verror (SL_NOT_IMPLEMENTED, "Function %s is not permitted to return float", name);
	return -1;
     }

   f = (SLang_Intrin_Fun_Type *) add_global_name (name, _pSLcompute_string_hash (name),
						  SLANG_INTRINSIC, sizeof (SLang_Intrin_Fun_Type),
						  ns);

   if (f == NULL)
     return -1;

   f->i_fun = addr;
   f->num_args = nargs;
   f->return_type = ret_type;

   for (i = 0; i < nargs; i++)
     f->arg_types [i] = arg_types[i];

   return 0;
}

static int va_add_intrinsic_function (SLang_NameSpace_Type *ns,
				      SLCONST char *name, FVOID_STAR addr, SLtype ret_type,
				      unsigned int nargs, va_list ap)
{
   SLtype arg_types [SLANG_MAX_INTRIN_ARGS];
   unsigned int i;

   if (nargs > SLANG_MAX_INTRIN_ARGS)
     {
	_pSLang_verror (SL_APPLICATION_ERROR, "Function %s requires too many arguments", name);
	return -1;
     }

   for (i = 0; i < nargs; i++)
     arg_types [i] = va_arg (ap, unsigned int);

   return add_intrinsic_function (ns, name, addr, ret_type, nargs, arg_types);
}

int SLadd_intrinsic_function (SLFUTURE_CONST char *name, FVOID_STAR addr, SLtype ret_type,
			      unsigned int nargs, ...)
{
   va_list ap;
   int status;

   va_start (ap, nargs);
   status = va_add_intrinsic_function (NULL, name, addr, ret_type, nargs, ap);
   va_end (ap);

   return status;
}

int SLns_add_intrinsic_function (SLang_NameSpace_Type *ns,
				 SLFUTURE_CONST char *name, FVOID_STAR addr, SLtype ret_type,
				 unsigned int nargs, ...)
{
   va_list ap;
   int status;

   va_start (ap, nargs);
   status = va_add_intrinsic_function (ns, name, addr, ret_type, nargs, ap);
   va_end (ap);

   return status;
}

static SLang_Name_Type *add_xxx_helper (SLang_NameSpace_Type *ns, SLCONST char *name,
					int what, unsigned int sizeof_what)
{
   if (-1 == init_interpreter ())
     return NULL;

   if (ns == NULL) ns = Global_NameSpace;

   return add_global_name (name,
			   _pSLcompute_string_hash (name),
			   what, sizeof_what, ns);
}

int SLns_add_hconstant (SLang_NameSpace_Type *ns, SLFUTURE_CONST char *name, SLtype type, short value)
{
   SLang_HConstant_Type *v;

   v = (SLang_HConstant_Type *)add_xxx_helper (ns, name, SLANG_HCONSTANT, sizeof (SLang_HConstant_Type));
   if (v == NULL)
     return -1;
   v->value = value;
   v->data_type = type;
   return 0;
}

int SLns_add_iconstant (SLang_NameSpace_Type *ns, SLFUTURE_CONST char *name, SLtype type, int value)
{
   SLang_IConstant_Type *v;

   v = (SLang_IConstant_Type *)add_xxx_helper (ns, name, SLANG_ICONSTANT, sizeof (SLang_IConstant_Type));
   if (v == NULL)
     return -1;
   v->value = value;
   v->data_type = type;
   return 0;
}

int SLns_add_lconstant (SLang_NameSpace_Type *ns, SLFUTURE_CONST char *name, SLtype type, long value)
{
   SLang_LConstant_Type *v;

   v = (SLang_LConstant_Type *)add_xxx_helper (ns, name, SLANG_LCONSTANT, sizeof (SLang_LConstant_Type));
   if (v == NULL)
     return -1;
   v->value = value;
   v->data_type = type;
   return 0;
}

#ifdef HAVE_LONG_LONG
int _pSLns_add_llconstant (SLang_NameSpace_Type *ns, SLFUTURE_CONST char *name, SLtype type, long long value)
{
   _pSLang_LLConstant_Type *v;

   v = (_pSLang_LLConstant_Type *)add_xxx_helper (ns, name, SLANG_LLCONSTANT, sizeof (_pSLang_LLConstant_Type));
   if (v == NULL)
     return -1;
   v->value = value;
   v->data_type = type;
   return 0;
}
#endif

#if SLANG_HAS_FLOAT
int SLns_add_dconstant (SLang_NameSpace_Type *ns, SLFUTURE_CONST char *name, double value)
{
   SLang_DConstant_Type *v;

   v = (SLang_DConstant_Type *)add_xxx_helper (ns, name, SLANG_DCONSTANT, sizeof (SLang_DConstant_Type));
   if (v == NULL)
     return -1;
   v->d = value;
   return 0;
}

int SLns_add_fconstant (SLang_NameSpace_Type *ns, SLFUTURE_CONST char *name, float value)
{
   SLang_FConstant_Type *v;

   v = (SLang_FConstant_Type *)add_xxx_helper (ns, name, SLANG_FCONSTANT, sizeof (SLang_FConstant_Type));
   if (v == NULL)
     return -1;
   v->f = value;
   return 0;
}
#endif

#ifdef HAVE_LONG_LONG
int SLns_add_llconstant (SLang_NameSpace_Type *ns, SLFUTURE_CONST char *name, long long value)
{
   SLang_LLConstant_Type *v;

   v = (SLang_LLConstant_Type *)add_xxx_helper (ns, name, SLANG_LLCONSTANT, sizeof (SLang_LLConstant_Type));
   if (v == NULL)
     return -1;
   v->ll = value;
   return 0;
}

#endif
int SLns_add_intrinsic_variable (SLang_NameSpace_Type *ns,
				 SLFUTURE_CONST char *name, VOID_STAR addr, SLtype data_type, int ro)
{
   SLang_Intrin_Var_Type *v;

   v = (SLang_Intrin_Var_Type *)add_xxx_helper (ns, name,
						(ro ? SLANG_RVARIABLE : SLANG_IVARIABLE),
						 sizeof (SLang_Intrin_Var_Type));
   if (v == NULL)
     return -1;

   v->addr = addr;
   v->type = data_type;
   return 0;
}

int SLadd_intrinsic_variable (SLFUTURE_CONST char *name, VOID_STAR addr, SLtype data_type, int ro)
{
   return SLns_add_intrinsic_variable (NULL, name, addr, data_type, ro);
}

static int
add_slang_function (SLFUTURE_CONST char *name, unsigned char type, unsigned long hash,
		    Function_Header_Type *h, SLFUTURE_CONST char *file,
		    SLang_NameSpace_Type *ns)
{
   _pSLang_Function_Type *f;

   if (file != NULL)
     {
	if (NULL == (file = SLang_create_slstring (file)))
	  return -1;
     }

   f = (_pSLang_Function_Type *)add_global_name (name, hash,
						 type,
						 sizeof (_pSLang_Function_Type),
						 ns);
   if (f == NULL)
     {
	SLang_free_slstring ((char *) file);
	return -1;
     }

   if (f->header != NULL)
     {
	free_function_header (f->header);
	/* free_namespace (f->v.ns); */
     }
   else if (f->autoload_file != NULL)
     {
	SLang_free_slstring ((char *) f->autoload_file);
	f->autoload_file = NULL;
     }

   f->header = h;

   if (h != NULL)
     {
	h->private_ns = This_Private_NameSpace;
	h->static_ns = This_Static_NameSpace;
     }
   else
     {
	f->autoload_ns = ns;
	f->autoload_file = file;
     }

   return 0;
}

static int SLns_autoload (SLFUTURE_CONST char *name, SLFUTURE_CONST char *file, SLFUTURE_CONST char *nsname)
{
   _pSLang_Function_Type *f;
   unsigned long hash;
   SLang_NameSpace_Type *ns;
   SLFUTURE_CONST char *cnsname = nsname;

   if (cnsname == NULL)
     cnsname = _pSLang_cur_namespace_intrinsic ();

   if (*cnsname == 0)
     cnsname = "Global";

   hash = _pSLcompute_string_hash (name);
   if (NULL != (ns = _pSLns_find_namespace (cnsname)))
     {
	f = (_pSLang_Function_Type *)_pSLns_locate_hashed_name (ns, name, hash);

	if ((f != NULL)
	    && (f->name_type == SLANG_FUNCTION)
	    && (f->header != NULL))
	  {
	     /* already loaded */
	     return 0;
	  }
     }
   else if (NULL == (ns = SLns_create_namespace (cnsname)))
     return -1;

   if (-1 == add_slang_function (name, SLANG_FUNCTION, hash,
				 NULL, file, ns))
     return -1;

   return 0;
}

int SLang_autoload (SLFUTURE_CONST char *name, SLFUTURE_CONST char *file)
{
   SLFUTURE_CONST char *ns;
   int status;

   ns = name;
   name = parse_namespace_encoded_name (ns);
   if (ns == name)
     return SLns_autoload (name, file, NULL);

   /* At this point, name points past "->" in ns. */
   if (NULL == (ns = SLmake_nstring (ns, ((name-2) - ns))))
     return -1;

   status = SLns_autoload (name, file, ns);
   SLfree ((char *) ns);
   return status;
}

/*}}}*/

/* call inner interpreter or return for more */
static void lang_try_now(void)
{
/* #if SLANG_HAS_DEBUG_CODE */
   Compile_ByteCode_Ptr->linenum = (unsigned short) This_Compile_Linenum;
/* #endif */
   Compile_ByteCode_Ptr++;
   if (This_Compile_Block_Type != COMPILE_BLOCK_TYPE_TOP_LEVEL)
     return;

/* #if SLANG_HAS_DEBUG_CODE */
   Compile_ByteCode_Ptr->linenum = (unsigned short) This_Compile_Linenum;
/* #endif */
   Compile_ByteCode_Ptr->bc_main_type = SLANG_BC_LAST_BLOCK;

   /* now do it */
   inner_interp (This_Compile_Block);
   (void) lang_free_branch (This_Compile_Block);
   Compile_ByteCode_Ptr = This_Compile_Block;
   Lang_Break = Lang_Break_Condition = Lang_Return = 0;
}

static void interp_pending_blocks (void)
{
   if ((This_Compile_Block_Type != COMPILE_BLOCK_TYPE_TOP_LEVEL)
       || (Compile_ByteCode_Ptr == This_Compile_Block))
     return;

   Compile_ByteCode_Ptr->linenum = (unsigned short) This_Compile_Linenum;
   Compile_ByteCode_Ptr->bc_main_type = SLANG_BC_LAST_BLOCK;

   inner_interp (This_Compile_Block);
   (void) lang_free_branch (This_Compile_Block);
   Compile_ByteCode_Ptr = This_Compile_Block;
   Lang_Break = Lang_Break_Condition = Lang_Return = 0;
}

/* returns positive number if name is a function or negative number if it
 is a variable.  If it is intrinsic, it returns magnitude of 1, else 2 */
int SLang_is_defined(SLFUTURE_CONST char *name)
{
   SLang_Name_Type *t;

   if (-1 == init_interpreter ())
     return -1;

   t = locate_namespace_encoded_name (name, 0);
   if (t == NULL)
     return 0;

   switch (t->name_type)
     {
      case SLANG_FUNCTION:
      /* case SLANG_PFUNCTION: */
	return 2;
      case SLANG_GVARIABLE:
      /* case SLANG_PVARIABLE: */
	return -2;

      case SLANG_HCONSTANT:
      case SLANG_ICONSTANT:
      case SLANG_LCONSTANT:
      case SLANG_LLCONSTANT:
      case SLANG_FCONSTANT:
      case SLANG_DCONSTANT:
      case SLANG_RVARIABLE:
      case SLANG_IVARIABLE:
	return -1;

      case SLANG_INTRINSIC:
      case SLANG_MATH_UNARY:
      case SLANG_APP_UNARY:
      case SLANG_ARITH_UNARY:
      case SLANG_ARITH_BINARY:
      default:
	return 1;
     }
}

SLang_Name_Type *SLang_get_fun_from_ref (SLang_Ref_Type *ref)
{
   if (ref->data_is_nametype)
     {
	SLang_Name_Type *nt = *(SLang_Name_Type **)ref->data;

	if (_pSLang_ref_is_callable (ref))
	  return nt;

	_pSLang_verror (SL_TYPE_MISMATCH,
		      "Reference to a function expected.  Found &%s",
		      nt->name);
     }
   else
     _pSLang_verror (SL_TYPE_MISMATCH,
		   "Reference to a function expected");
   return NULL;
}

int SLexecute_function (SLang_Name_Type *nt)
{
   unsigned char type;
   SLCONST char *name;
   int status = 1;

   if (nt == NULL)
     return -1;

   if (IS_SLANG_ERROR)
     return -1;

   (void) _pSLerr_suspend_messages ();

   type = nt->name_type;
   name = nt->name;

   switch (type)
     {
      case SLANG_PFUNCTION:
      case SLANG_FUNCTION:
	execute_slang_fun ((_pSLang_Function_Type *) nt, This_Compile_Linenum);
	break;

      case SLANG_INTRINSIC:
	execute_intrinsic_fun ((SLang_Intrin_Fun_Type *) nt);
	break;

      case SLANG_MATH_UNARY:
      case SLANG_APP_UNARY:
      case SLANG_ARITH_UNARY:
      case SLANG_ARITH_BINARY:
	inner_interp_nametype (nt, 0);
	break;

      default:
	_pSLang_verror (SL_TYPE_MISMATCH, "%s is not a function", name);
     }

   if (IS_SLANG_ERROR)
     {
	if (SLang_Traceback & SL_TB_FULL)
	  _pSLang_verror (0, "Error encountered while executing %s", name);
	status = -1;
     }

   (void) _pSLerr_resume_messages ();
   return status;
}

int SLang_execute_function (SLFUTURE_CONST char *name)
{
   SLang_Name_Type *entry;

   if (NULL == (entry = SLang_get_function (name)))
     return 0;

   return SLexecute_function (entry);
}

/* return S-Lang function or NULL */
SLang_Name_Type *SLang_get_function (SLFUTURE_CONST char *name)
{
   SLang_Name_Type *entry;

   if (NULL == (entry = locate_namespace_encoded_name (name, 0)))
     return NULL;

   if (is_nametype_callable (entry))
     return entry;

   return NULL;
}

static void lang_begin_function (void)
{
   if (This_Compile_Block_Type != COMPILE_BLOCK_TYPE_TOP_LEVEL)
     {
	_pSLang_verror (SL_SYNTAX_ERROR, "Function nesting is illegal");
	return;
     }
   Lang_Defining_Function = 1;
   (void) push_block_context (COMPILE_BLOCK_TYPE_FUNCTION);
}

#if USE_COMBINED_BYTECODES
static void rearrange_optimized_binary (SLBlock_Type *b, _pSLang_BC_Type t1, _pSLang_BC_Type t2, _pSLang_BC_Type t3)
{
   SLBlock_Type tmp;

   b->bc_main_type = t1;
   (b-1)->bc_main_type = t3;
   (b-2)->bc_main_type = t2;

   tmp = *b;
   *b = *(b-1);
   *(b-1) = *(b-2);
   *(b-2) = tmp;
}

static void rearrange_optimized_optimized (SLBlock_Type *b, _pSLang_BC_Type t1)
{
   SLBlock_Type tmp;
   b->bc_main_type = t1;

   tmp = *b;
   *b = *(b-1);
   *(b-1) = *(b-2);
   *(b-2) = *(b-3);
   *(b-3) = tmp;
# if 1
   (b)->bc_main_type = SLANG_BC_COMBINED;
   (b-1)->bc_main_type = SLANG_BC_COMBINED;
   (b-2)->bc_main_type = SLANG_BC_COMBINED;
# endif
}

static void rearrange_optimized_unary (SLBlock_Type *b, _pSLang_BC_Type t1, _pSLang_BC_Type t2)
{
   SLBlock_Type tmp;

   b->bc_main_type = t1;
   (b-1)->bc_main_type = t2;

   tmp = *b;
   *b = *(b-1);
   *(b-1) = tmp;
}

#define COMPILE_COMBINE_STATS 0
#if COMPILE_COMBINE_STATS
static unsigned int Combine_Statistics [0x10000U];
static void write_combine_stats (void)
{
   unsigned int i;
   FILE *fp = fopen ("combine_stats.dat", "w");
   if (fp == NULL)
     return;

   for (i = 0; i < 0x10000U; i++)
     {
	if (Combine_Statistics[i])
	  fprintf (fp, "%5d\t0x%04X\n", Combine_Statistics[i], i);
     }
   (void) fclose (fp);
}

static void gather_statistics (SLBlock_Type *b)
{
   static int inited = 0;
   unsigned char last, next;

   if (inited == 0)
     SLang_add_cleanup_function (write_combine_stats);

   last = 0;
   while ((next = b->bc_main_type) != 0)
     {
	Combine_Statistics[last*256 + next] += 1;
	last = next;
	b++;
     }
}

#endif

static void optimize_block4 (SLBlock_Type *b)
{
   while (1)
     {
	switch (b->bc_main_type)
	  {
	   case SLANG_BC_LAST_BLOCK:
	     return;

	   default:
	     b++;
	     break;

   	   case SLANG_BC_LVARIABLE:
	     b++;
	     if (b->bc_main_type == SLANG_BC_LVARIABLE)
	       {
		  SLBlock_Type *b0 = b - 1;
		  b0->bc_main_type = SLANG_BC_MANY_LVARIABLE;
		  do
		    {
		       b->bc_main_type = SLANG_BC_LVARIABLE_COMBINED;
		       b++;
		    }
		  while (b->bc_main_type == SLANG_BC_LVARIABLE);
		  if (b->bc_main_type == SLANG_BC_CALL_DIRECT)
		    {
		       b0->bc_main_type = SLANG_BC_MANY_LVARIABLE_DIR;
		       b->bc_main_type = SLANG_BC_CALL_DIRECT_COMB;
		       b++;
		    }
		  break;
	       }
	     if (b->bc_main_type == SLANG_BC_LITERAL_AGET1)
	       {
		  (b-1)->bc_main_type = SLANG_BC_LVAR_LIT_AGET1;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b += 3;
		  break;
	       }
	     break;
	  }
     }
}

static void optimize_block3 (SLBlock_Type *b)
{
   while (1)
     {
	switch (b->bc_main_type)
	  {
	   case SLANG_BC_LAST_BLOCK:
	     return;

	   default:
	     b++;
	     break;

	   case SLANG_BC_SET_LOCAL_LVALUE:
	     b++;
	     if (b->bc_main_type == SLANG_BC_LITERAL_INT)
	       {
		  (b-1)->bc_main_type = SLANG_BC_SET_LOCLV_LIT_INT;
		  b->bc_main_type = SLANG_BC_LITERAL_COMBINED;
		  b++;
		  break;
	       }
	     if (b->bc_main_type == SLANG_BC_LITERAL_AGET1)
	       {
		  (b-1)->bc_main_type = SLANG_BC_SET_LOCLV_LIT_AGET1;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  break;
	       }
	     if (b->bc_main_type == SLANG_BC_LVARIABLE)
	       {
		  (b-1)->bc_main_type = SLANG_BC_SET_LOCLV_LVAR;
		  b->bc_main_type = SLANG_BC_LVARIABLE_COMBINED;
		  b++;
		  break;
	       }
	     if (b->bc_main_type == SLANG_BC_LAST_BLOCK)
	       {
		  (b-1)->bc_main_type = SLANG_BC_SET_LOCLV_LASTBLOCK;
		  return;
	       }
	     break;

	   case SLANG_BC_LVARIABLE:
	     b++;
	     if (b->bc_main_type == SLANG_BC_EARG_LVARIABLE)
	       {
		  (b-1)->bc_main_type = SLANG_BC_LVAR_EARG_LVAR;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  break;
	       }
	     if (b->bc_main_type == SLANG_BC_FIELD)
	       {
		  (b-1)->bc_main_type = SLANG_BC_LVAR_FIELD;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  break;
	       }
	     if (b->bc_main_type == SLANG_BC_LITERAL_INT)
	       {
		  (b-1)->bc_main_type = SLANG_BC_LVAR_LITERAL_INT;
		  b->bc_main_type = SLANG_BC_LITERAL_COMBINED;
		  b++;
		  break;
	       }
	     if (b->bc_main_type == SLANG_BC_LVARIABLE_APUT1)
	       {
		  (b-1)->bc_main_type = SLANG_BC_LVAR_LVAR_APUT1;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  break;
	       }
	     if (b->bc_main_type == SLANG_BC_SET_STRUCT_LVALUE)
	       {
		  (b-1)->bc_main_type = SLANG_BC_LVAR_SET_FIELD;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  break;
	       }
	     if (b->bc_main_type == SLANG_BC_SET_GLOBAL_LVALUE)
	       {
		  (b-1)->bc_main_type = SLANG_BC_LVAR_SET_GLOB_LVAL;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  break;
	       }
	     break;

	   case SLANG_BC_PVARIABLE:
	     b++;
	     if (b->bc_main_type == SLANG_BC_SET_GLOBAL_LVALUE)
	       {
		  (b-1)->bc_main_type = SLANG_BC_PVAR_SET_GLOB_LVAL;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  break;
	       }
	     break;

	   case SLANG_BC_BINARY:
	     /* Anything added here may need to be accounted for in do_compare() */
	     b++;
	     if (b->bc_main_type == SLANG_BC_LAST_BLOCK)
	       {
		  (b-1)->bc_main_type = SLANG_BC_BINARY_LASTBLOCK;
		  return;
	       }
	     if (b->bc_main_type == SLANG_BC_SET_LOCAL_LVALUE)
	       {
		  (b-1)->bc_main_type = SLANG_BC_BINARY_SET_LOCLVAL;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  break;
	       }
	     if (b->bc_main_type == SLANG_BC_BINARY)
	       {
		  (b-1)->bc_main_type = SLANG_BC_BINARY2;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  break;
	       }
	     break;

	   case SLANG_BC_EARG_LVARIABLE:
	     b++;
	     if (b->bc_main_type == SLANG_BC_INTRINSIC)
	       {
		  (b-1)->bc_main_type = SLANG_BC_EARG_LVARIABLE_INTRINSIC;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  break;
	       }
	     break;

	   case SLANG_BC_LVARIABLE_AGET:
	     b++;
	     if (b->bc_main_type == SLANG_BC_SET_LOCAL_LVALUE)
	       {
		  (b-1)->bc_main_type = SLANG_BC_LVAR_AGET_SET_LOCLVAL;
		  b->bc_main_type= SLANG_BC_COMBINED;
		  b++;
		  break;
	       }
	     break;

	   case SLANG_BC_LLVARIABLE_BINARY:
	     b += 3;
	     if (b->bc_main_type == SLANG_BC_BINARY)
	       {
		  (b-3)->bc_main_type = SLANG_BC_LLVARIABLE_BINARY2;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  break;
	       }
	     if (b->bc_main_type == SLANG_BC_IF_BLOCK)
	       {
		  (b-3)->bc_main_type = SLANG_BC_LLVAR_BINARY_IF;
		  b->bc_main_type = SLANG_BC_BLOCK_COMBINED;
		  b++;
		  break;
	       }
	     break;

	   case SLANG_BC_LITERAL_AGET1:
	     b += 3;
	     if (b->bc_main_type == SLANG_BC_LITERAL_INT_BINARY)
	       {
		  (b-3)->bc_main_type = SLANG_BC_LIT_AGET1_INT_BINARY;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b += 2;
		  break;
	       }
	     break;
	  }
     }
}

static void optimize_block2 (SLBlock_Type *b)
{
   while (1)
     {
	switch (b->bc_main_type)
	  {
	   default:
	     b++;
	     break;

	   case SLANG_BC_LAST_BLOCK:
	     return;

	   case SLANG_BC_CALL_DIRECT_LVAR:
	     if (b->b.call_function != start_arg_list)
	       {
		  b += 2;	       /* combined code, add 2 */
		  break;
	       }
	     b += 2;		       /* combined code */
	     if (((b-1)->bc_main_type == SLANG_BC_LVARIABLE_COMBINED)
		 && (b->bc_main_type == SLANG_BC_LVARIABLE_AGET))
	       {
		  b->bc_main_type = SLANG_BC_COMBINED;
		  (b-2)->bc_main_type = SLANG_BC_LVARIABLE_AGET1;
		  b++;
		  break;
	       }
	     if (((b-1)->bc_main_type == SLANG_BC_LVARIABLE_COMBINED)
		 && (b->bc_main_type == SLANG_BC_LVARIABLE_APUT))
	       {
		  b->bc_main_type = SLANG_BC_COMBINED;
		  (b-2)->bc_main_type = SLANG_BC_LVARIABLE_APUT1;
		  b++;
		  break;
	       }
	     break;

	   case SLANG_BC_CALL_DIRECT_LINT:
	     if (b->b.call_function != start_arg_list)
	       {
		  b += 2;	       /* combined code, add 2 */
		  break;
	       }
	     b += 2;		       /* combined code */
	     if (((b-1)->bc_main_type == SLANG_BC_LITERAL_COMBINED)
		 && (b->bc_main_type == SLANG_BC_LVARIABLE_AGET))
	       {
		  b->bc_main_type = SLANG_BC_COMBINED;
		  (b-2)->bc_main_type = SLANG_BC_LITERAL_AGET1;
		  b++;
		  break;
	       }
	     if (((b-1)->bc_main_type == SLANG_BC_LITERAL_COMBINED)
		 && (b->bc_main_type == SLANG_BC_LVARIABLE_APUT))
	       {
		  b->bc_main_type = SLANG_BC_COMBINED;
		  (b-2)->bc_main_type = SLANG_BC_LITERAL_APUT1;
		  b++;
		  break;
	       }
	     break;
	  }
     }
}

/* Note: Make sure lang_free_branch is suitably modified to account for
 * changes here.
 */
static void optimize_block1 (SLBlock_Type *b)
{
   SLBlock_Type *bstart, *b1, *b2;
   SLtype b2_main_type;

   bstart = b;

   while (1)
     {
	switch (b->bc_main_type)
	  {
	   case 0:
	     return;

	   default:
	     b++;
	     break;

	   case SLANG_BC_SET_LOCAL_LVALUE:
	     if ((b->bc_sub_type != SLANG_BCST_ASSIGN)
		 || (bstart + 3 > b))
	       {
		  b++;
		  continue;
	       }
	     b2 = b - 3;
	     b2_main_type = b2->bc_main_type;

	     switch (b2_main_type)
	       {
		case SLANG_BC_LLVARIABLE_BINARY:
		  rearrange_optimized_optimized (b, SLANG_BC_LASSIGN_LLBINARY);
		  break;
		case SLANG_BC_LIVARIABLE_BINARY:
		  rearrange_optimized_optimized (b, SLANG_BC_LASSIGN_LIBINARY);
		  break;
		case SLANG_BC_ILVARIABLE_BINARY:
		  rearrange_optimized_optimized (b, SLANG_BC_LASSIGN_ILBINARY);
		  break;
		case SLANG_BC_LDVARIABLE_BINARY:
		  rearrange_optimized_optimized (b, SLANG_BC_LASSIGN_LDBINARY);
		  break;
		case SLANG_BC_DLVARIABLE_BINARY:
		  rearrange_optimized_optimized (b, SLANG_BC_LASSIGN_DLBINARY);
		  break;
	       }
	     b++;
	     break;

	   case SLANG_BC_BINARY:
	     if (bstart + 2 > b)
	       {
		  b++;
		  break;
	       }
	     b2 = b-1;
	     b1 = b2-1;
	     b2_main_type = b2->bc_main_type;

	     switch (b1->bc_main_type)
	       {
		case SLANG_LVARIABLE:
		  if (b2_main_type == SLANG_LVARIABLE)
		    rearrange_optimized_binary (b,
						SLANG_BC_LLVARIABLE_BINARY,
						SLANG_BC_LVARIABLE_COMBINED,
						SLANG_BC_LVARIABLE_COMBINED);
		  else if (b2_main_type == SLANG_GVARIABLE)
		    rearrange_optimized_binary (b,
						SLANG_BC_LGVARIABLE_BINARY,
						SLANG_BC_LVARIABLE_COMBINED,
						SLANG_BC_GVARIABLE_COMBINED);
		  else if (b2_main_type == SLANG_BC_LITERAL_INT)
		    rearrange_optimized_binary (b,
						SLANG_BC_LIVARIABLE_BINARY,
						SLANG_BC_LVARIABLE_COMBINED,
						SLANG_BC_LITERAL_COMBINED);
		  else if (b2_main_type == SLANG_BC_LITERAL_DBL)
		    rearrange_optimized_binary (b,
						SLANG_BC_LDVARIABLE_BINARY,
						SLANG_BC_LVARIABLE_COMBINED,
						SLANG_BC_LITERAL_COMBINED);
		  break;

		case SLANG_GVARIABLE:
		  if (b2_main_type == SLANG_LVARIABLE)
		    rearrange_optimized_binary (b,
						SLANG_BC_GLVARIABLE_BINARY,
						SLANG_BC_GVARIABLE_COMBINED,
						SLANG_BC_LVARIABLE_COMBINED);
		  else if (b2_main_type == SLANG_GVARIABLE)
		    rearrange_optimized_binary (b,
						SLANG_BC_GGVARIABLE_BINARY,
						SLANG_BC_GVARIABLE_COMBINED,
						SLANG_BC_GVARIABLE_COMBINED);
		  break;

		case SLANG_BC_LITERAL_INT:
		  if (b2_main_type == SLANG_LVARIABLE)
		    rearrange_optimized_binary (b,
						SLANG_BC_ILVARIABLE_BINARY,
						SLANG_BC_LITERAL_COMBINED,
						SLANG_BC_LVARIABLE_COMBINED);
		  break;

		case SLANG_BC_LITERAL_DBL:
		  if (b2_main_type == SLANG_LVARIABLE)
		    rearrange_optimized_binary (b,
						SLANG_BC_DLVARIABLE_BINARY,
						SLANG_BC_LITERAL_COMBINED,
						SLANG_BC_LVARIABLE_COMBINED);
		  break;

		default:
		  if (b2_main_type == SLANG_LVARIABLE)
		    rearrange_optimized_unary (b,
					       SLANG_BC_LVARIABLE_BINARY,
					       SLANG_BC_LVARIABLE_COMBINED);
		  else if (b2_main_type == SLANG_GVARIABLE)
		    rearrange_optimized_unary (b,
					       SLANG_BC_GVARIABLE_BINARY,
					       SLANG_BC_GVARIABLE_COMBINED);
		  else if (b2_main_type == SLANG_BC_LITERAL_INT)
		    rearrange_optimized_unary (b,
					       SLANG_BC_LITERAL_INT_BINARY,
					       SLANG_BC_LITERAL_COMBINED);
		  else if (b2_main_type == SLANG_BC_LITERAL_DBL)
		    rearrange_optimized_unary (b,
					       SLANG_BC_LITERAL_DBL_BINARY,
					       SLANG_BC_LITERAL_COMBINED);
	       }
	     b++;
	     break;

	   case SLANG_BC_CALL_DIRECT:
	     b++;
	     switch (b->bc_main_type)
	       {
		default:
		  break;

		case 0:
		  return;

		case SLANG_BC_INTRINSIC:
		  b->bc_main_type = SLANG_BC_COMBINED;
		  if ((b+1)->bc_main_type == 0)
		    {
		       (b-1)->bc_main_type = SLANG_BC_CALL_DIRECT_RETINTR;
		       return;
		    }
		  (b-1)->bc_main_type = SLANG_BC_CALL_DIRECT_INTRINSIC;
		  b++;
		  break;
		case SLANG_BC_LITERAL_STR:
		  (b-1)->bc_main_type = SLANG_BC_CALL_DIRECT_LSTR;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  break;
		case SLANG_BC_FUNCTION:
		case SLANG_BC_PFUNCTION:
		  (b-1)->bc_main_type = SLANG_BC_CALL_DIRECT_SLFUN;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  break;
		case SLANG_BC_EARG_LVARIABLE:
		  (b-1)->bc_main_type = SLANG_BC_CALL_DIRECT_EARG_LVAR;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  break;
		case SLANG_BC_LITERAL_INT:
		  b->bc_main_type = SLANG_BC_LITERAL_COMBINED;
		  (b-1)->bc_main_type = SLANG_BC_CALL_DIRECT_LINT;
		  b++;
		  break;
		case SLANG_BC_LVARIABLE:
		  b->bc_main_type = SLANG_BC_LVARIABLE_COMBINED;
		  (b-1)->bc_main_type = SLANG_BC_CALL_DIRECT_LVAR;
		  b++;
		  break;
	       }
	     break;

	   case SLANG_BC_INTRINSIC:
	     b++;
	     switch (b->bc_main_type)
	       {
		case SLANG_BC_CALL_DIRECT:
		  (b-1)->bc_main_type = SLANG_BC_INTRINSIC_CALL_DIRECT;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  break;
#if 0
		case SLANG_BC_BLOCK:
		  (b-1)->bc_main_type = SLANG_BC_INTRINSIC_BLOCK;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
		  break;
#endif

		case 0:
		  (b-1)->bc_main_type = SLANG_BC_RET_INTRINSIC;
		  return;

		default:
		  break;
	       }
	     break;

	   case SLANG_BC_BLOCK:
	     if (b->bc_sub_type == SLANG_BCST_IF)
	       {
		  b->bc_main_type = SLANG_BC_IF_BLOCK;
		  b++;
		  break;
	       }
	     b++;
	     break;
	   case SLANG_BC_LITERAL_INT:
	     b++;
	     if (b->bc_main_type == SLANG_BC_RETURN)
	       {
		  (b-1)->bc_main_type = SLANG_BC_RET_LITERAL_INT;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
	       }
	     break;

	   case SLANG_BC_LVARIABLE:
	     b++;
	     if (b->bc_main_type == SLANG_BC_RETURN)
	       {
		  (b-1)->bc_main_type = SLANG_BC_RET_LVARIABLE;
		  b->bc_main_type = SLANG_BC_COMBINED;
		  b++;
	       }
	     break;
	  }
     }
}

static void optimize_block (SLBlock_Type *b)
{
   optimize_block1 (b);
   optimize_block2 (b);
   optimize_block3 (b);
   optimize_block4 (b);
}

#endif

static void end_define_function (void)
{
   /* free_local_variable_table (); */
   _pSLns_deallocate_namespace (Locals_NameSpace);
   Locals_NameSpace = NULL;
   Local_Variable_Number = 0;
   Function_Args_Number = 0;
   Lang_Defining_Function = 0;
}

/* name will be NULL if the object is to simply terminate the function
 * definition.  See SLang_restart.
 */
static int lang_define_function (SLFUTURE_CONST char *name, unsigned char type, unsigned long hash,
				 SLang_NameSpace_Type *ns)
{
   Function_Header_Type *h;

   if (This_Compile_Block_Type != COMPILE_BLOCK_TYPE_FUNCTION)
     {
	_pSLang_verror (SL_SYNTAX_ERROR, "Premature end of function");
	return -1;
     }

   /* terminate function */
   Compile_ByteCode_Ptr->bc_main_type = SLANG_BC_LAST_BLOCK;
   if (name == NULL)
     {
	end_define_function ();
	return -1;
     }

   h = allocate_function_header (Function_Args_Number,
				 Local_Variable_Number,
				 This_Compile_Filename);
   if ((h == NULL)
       || (-1 == add_slang_function (name, type, hash, h, NULL, ns)))
     {
	free_function_header (h);
	end_define_function ();
	return -1;
     }

   h->body = This_Compile_Block;
   This_Compile_Block = NULL;
#if USE_COMBINED_BYTECODES
   optimize_block (h->body);
#endif
   end_define_function ();
   pop_block_context ();

   /* A function is only defined at top-level */
   if (This_Compile_Block_Type != COMPILE_BLOCK_TYPE_TOP_LEVEL)
     {
	_pSLang_verror (SL_INTERNAL_ERROR, "Not at top-level");
	return -1;
     }
   Compile_ByteCode_Ptr = This_Compile_Block;
   return 0;
}

static int check_linkage (SLCONST char *name, unsigned long hash, int check_static)
{
   SLang_NameSpace_Type *ns;
   int found = 0;

   /* If the variable is already defined in the static hash table,
    * generate an error.
    */
   ns = This_Private_NameSpace;
   if ((ns != NULL) && (This_Static_NameSpace != ns))
     found = (NULL != _pSLns_locate_hashed_name (ns, name, hash));

   if ((found == 0) && (check_static))
     {
	ns = This_Static_NameSpace;
	if ((ns != NULL) && (Global_NameSpace != ns))
	  found = (NULL != _pSLns_locate_hashed_name (ns, name, hash));
     }

   if (found == 0)
     return 0;

   _pSLang_verror (SL_DUPLICATE_DEFINITION,
		 "%s already has static or private linkage in this unit",
		 name);
   return -1;
}

static void define_private_function (SLFUTURE_CONST char *name, unsigned long hash)
{
   (void) lang_define_function (name, SLANG_PFUNCTION, hash, This_Private_NameSpace);
}

static void define_static_function (SLFUTURE_CONST char *name, unsigned long hash)
{
   if (0 == check_linkage (name, hash, 0))
     (void) lang_define_function (name, SLANG_FUNCTION, hash, This_Static_NameSpace);
}

static void define_public_function (SLFUTURE_CONST char *name, unsigned long hash)
{
   if (0 == check_linkage (name, hash, 1))
     (void) lang_define_function (name, SLANG_FUNCTION, hash, Global_NameSpace);
}

static void lang_end_block (void)
{
   SLBlock_Type *node, *branch;

   if (This_Compile_Block_Type != COMPILE_BLOCK_TYPE_BLOCK)
     {
	_pSLang_verror (SL_SYNTAX_ERROR, "Not defining a block");
	return;
     }

   /* terminate the block */
/* #if SLANG_HAS_DEBUG_CODE */
   Compile_ByteCode_Ptr->linenum = (unsigned short) This_Compile_Linenum;
/* #endif */
   Compile_ByteCode_Ptr->bc_main_type = SLANG_BC_LAST_BLOCK;
   branch = This_Compile_Block;  This_Compile_Block = NULL;

#if USE_COMBINED_BYTECODES
   optimize_block (branch);
#endif

   pop_block_context ();
   node = Compile_ByteCode_Ptr++;

   node->bc_main_type = SLANG_BC_BLOCK;
   node->bc_sub_type = 0;
   node->b.blk = branch;
}

static int lang_begin_block (void)
{
   return push_block_context (COMPILE_BLOCK_TYPE_BLOCK);
}

static int lang_check_space (void)
{
   unsigned int n;
   SLBlock_Type *p;

   if (NULL == (p = This_Compile_Block))
     {
	_pSLang_verror (SL_INTERNAL_ERROR, "Top-level block not present");
	return -1;
     }

   /* Allow 1 extra for terminator */
   if (Compile_ByteCode_Ptr + 1 < This_Compile_Block_Max)
     return 0;

   n = (unsigned int) (This_Compile_Block_Max - p);

   /* enlarge the space by 2 objects */
   n += 20;

   if (NULL == (p = (SLBlock_Type *) _SLrecalloc((char *)p, n, sizeof(SLBlock_Type))))
     return -1;

   This_Compile_Block_Max = p + n;
   n = (unsigned int) (Compile_ByteCode_Ptr - This_Compile_Block);
   This_Compile_Block = p;
   Compile_ByteCode_Ptr = p + n;

   return 0;
}

static int add_global_variable (SLCONST char *name, char name_type, unsigned long hash,
				SLang_NameSpace_Type *ns)
{
   SLang_Name_Type *g;

   /* Note the importance of checking if it is already defined or not.  For example,
    * suppose X is defined as an intrinsic variable.  Then S-Lang code like:
    * !if (is_defined("X")) { variable X; }
    * will not result in a global variable X.  On the other hand, this would
    * not be an issue if 'variable' statements always were not processed
    * immediately.  That is, as it is now, 'if (0) {variable ZZZZ;}' will result
    * in the variable ZZZZ being defined because of the immediate processing.
    * The current solution is to do: if (0) { eval("variable ZZZZ;"); }
    */
   /* hash = _pSLcompute_string_hash (name); */
   g = _pSLns_locate_hashed_name (ns, name, hash);

   if (g != NULL)
     {
	if (g->name_type == name_type)
	  return 0;
     }

   if (NULL == add_global_name (name, hash, name_type,
				sizeof (SLang_Global_Var_Type), ns))
     return -1;

   return 0;
}

int SLadd_global_variable (SLCONST char *name)
{
   if (-1 == init_interpreter ())
     return -1;

   return add_global_variable (name, SLANG_GVARIABLE,
			       _pSLcompute_string_hash (name),
			       Global_NameSpace);
}

static int add_local_variable (SLCONST char *name, unsigned long hash)
{
   SLang_Local_Var_Type *t;

   /* local variable */
   if (Local_Variable_Number >= SLANG_MAX_LOCAL_VARIABLES)
     {
	_pSLang_verror (SL_SYNTAX_ERROR, "Too many local variables");
	return -1;
     }

   if (NULL != _pSLns_locate_hashed_name (Locals_NameSpace, name, hash))
     {
	_pSLang_verror (SL_SYNTAX_ERROR, "Local variable %s has already been defined", name);
	return -1;
     }

   t = (SLang_Local_Var_Type *)
     add_name_to_namespace (name, hash,
			    sizeof (SLang_Local_Var_Type), SLANG_LVARIABLE,
			    Locals_NameSpace);
   if (t == NULL)
     return -1;

   t->local_var_number = Local_Variable_Number;
   Local_Variable_Names[Local_Variable_Number] = t->name;
   /* we will copy this later -- it is an slstring and will persist as long
    * as the Locals_NameSpace persists
    */

   Local_Variable_Number++;
   return 0;
}

static void (*Compile_Mode_Function) (_pSLang_Token_Type *);
static void compile_basic_token_mode (_pSLang_Token_Type *);

/* This function could be called when an error has occured during parsing.
 * Its purpose is to "close" any currently opened blocks and functions.
 */
void reset_compiler_state (void)
{
   _pSLcompile_ptr = _pSLcompile;
   Compile_Mode_Function = compile_basic_token_mode;

   while (This_Compile_Block_Type == COMPILE_BLOCK_TYPE_BLOCK)
     lang_end_block();

   if (This_Compile_Block_Type == COMPILE_BLOCK_TYPE_FUNCTION)
     {
	/* Terminate function definition and free variables */
	lang_define_function (NULL, SLANG_FUNCTION, 0, Global_NameSpace);
	if (lang_free_branch (This_Compile_Block))
	  {
	     SLfree((char *)This_Compile_Block);
	     This_Compile_Block = NULL;
	  }
     }
   Lang_Defining_Function = 0;

   while ((This_Compile_Block_Type != COMPILE_BLOCK_TYPE_TOP_LEVEL)
	  && (This_Compile_Block_Type != COMPILE_BLOCK_TYPE_NONE)
	  && (0 == pop_block_context ()))
     ;
}

/* The active interpreter is the one that is currently executing.  We may
 * have a situation where:  slang -> C -> slang
 * Each "slang" is an interpreter and only one is active.
 */
static void reset_active_interpreter (void)
{
   reset_compiler_state ();

   Trace_Mode = 0;
   Lang_Break = Lang_Return = 0;
}

#if SLANG_HAS_QUALIFIERS
static void clear_qualifier_stack (void)
{
   unsigned int i;

   for (i = 0; i < Recursion_Depth; i++)
     {
	if (Function_Qualifiers_Stack[i] != NULL)
	  {
	     SLang_free_struct (Function_Qualifiers_Stack[i]);
	     Function_Qualifiers_Stack[i] = NULL;
	  }
     }
}
#endif
void SLang_restart (int localv)
{
   reset_active_interpreter ();

   if (SLang_get_error () == SL_STACK_OVERFLOW)
     {
	/* This loop guarantees that the stack is properly cleaned. */
	/* The validity of this step needs to be reexamined in the context
	 * of the new exception handling for slang 2
	 */
	while (Stack_Pointer != Run_Stack)
	  {
	     SLdo_pop ();
	  }
     }

   if (localv)
     {
	Next_Function_Num_Args = SLang_Num_Function_Args = 0;
	Local_Variable_Frame = Local_Variable_Stack;
#if SLANG_HAS_QUALIFIERS
	clear_qualifier_stack ();
#endif
	Recursion_Depth = 0;
	Frame_Pointer = Stack_Pointer;
	Frame_Pointer_Depth = 0;
	Function_Stack_Ptr = Function_Stack;
	Switch_Obj_Ptr = Switch_Objects;
	while (Switch_Obj_Ptr < Switch_Obj_Max)
	  {
	     if (Switch_Obj_Ptr->o_data_type != 0)
	       {
		  SLang_free_object (Switch_Obj_Ptr);
		  Switch_Obj_Ptr->o_data_type = 0;
	       }
	     Switch_Obj_Ptr++;
	  }
	Switch_Obj_Ptr = Switch_Objects;
     }
   _pSLerr_print_message_queue ();

   _pSLerr_clear_error (0);
}

#if SLANG_HAS_DEBUG_CODE
static void compile_line_info (_pSLang_BC_Type bc_main_type, SLFUTURE_CONST char *file, long linenum)
{
   Linenum_Info_Type *info;

   if (NULL == (info = (Linenum_Info_Type *) SLmalloc (sizeof (Linenum_Info_Type))))
     return;

   info->linenum = (int) linenum;
   if (file == NULL)
     file = "";

   if (NULL == (info->filename = SLang_create_slstring (file)))
     {
	SLfree ((char *) info);
	return;
     }
   Compile_ByteCode_Ptr->bc_main_type = bc_main_type;
   Compile_ByteCode_Ptr->b.line_info = info;
   lang_try_now ();
}

static void set_line_number_info (long val)
{
   This_Compile_Linenum = (unsigned int) val;
}
#endif

static void compile_directive (unsigned char sub_type, int delay_inner_interp)
{
   /* This function is called only from compile_directive_mode which is
    * only possible when a block is available.
    */

   /* use BLOCK */
   Compile_ByteCode_Ptr--;
   Compile_ByteCode_Ptr->bc_sub_type = sub_type;

   if (delay_inner_interp)
     {
	Compile_ByteCode_Ptr->linenum = (unsigned short) This_Compile_Linenum;
	Compile_ByteCode_Ptr++;
	return;
     }
   lang_try_now ();
}

static void compile_unary (int op, _pSLang_BC_Type mt)
{
   Compile_ByteCode_Ptr->bc_main_type = mt;
   Compile_ByteCode_Ptr->b.i_blk = op;
   Compile_ByteCode_Ptr->bc_sub_type = 0;

   lang_try_now ();
}

static void compile_binary (int op)
{
   Compile_ByteCode_Ptr->bc_main_type = SLANG_BC_BINARY;
   Compile_ByteCode_Ptr->b.i_blk = op;
   Compile_ByteCode_Ptr->bc_sub_type = 0;

   lang_try_now ();
}

#if SLANG_OPTIMIZE_FOR_SPEED
static int try_compressed_bytecode (_pSLang_BC_Type last_bc, _pSLang_BC_Type bc)
{
   if (Compile_ByteCode_Ptr != This_Compile_Block)
     {
	SLBlock_Type *b;
	b = Compile_ByteCode_Ptr - 1;
	if (b->bc_main_type == last_bc)
	  {
	     Compile_ByteCode_Ptr = b;
	     b->bc_main_type = bc;
	     lang_try_now ();
	     return 0;
	  }
     }
   return -1;
}
#endif

/* This is a hack */
typedef struct _Special_NameTable_Type
{
   SLCONST char *name;
   int (*fun) (struct _Special_NameTable_Type *, _pSLang_Token_Type *);
   VOID_STAR blk_data;
   _pSLang_BC_Type main_type;
}
Special_NameTable_Type;

static int handle_special (Special_NameTable_Type *nt, _pSLang_Token_Type *tok)
{
   (void) tok;
   Compile_ByteCode_Ptr->bc_main_type = nt->main_type;
   Compile_ByteCode_Ptr->b.ptr_blk = nt->blk_data;
   return 0;
}

static int handle_special_file (Special_NameTable_Type *nt, _pSLang_Token_Type *tok)
{
   SLFUTURE_CONST char *name;

   (void) nt; (void) tok;

   if (This_Private_NameSpace == NULL) name = "***Unknown***";
   else
     name = This_Private_NameSpace->name;

   name = SLang_create_slstring (name);
   if (name == NULL)
     return -1;

   Compile_ByteCode_Ptr->b.s_blk = name;
   Compile_ByteCode_Ptr->bc_main_type = SLANG_BC_LITERAL_STR;
   Compile_ByteCode_Ptr->bc_sub_type = SLANG_STRING_TYPE;
   return 0;
}

static int handle_special_line (Special_NameTable_Type *nt, _pSLang_Token_Type *tok)
{
   (void) nt;
#if SLANG_HAS_DEBUG_CODE
   if ((Compile_ByteCode_Ptr->b.l_blk = (long) tok->line_number) <= 0)
     Compile_ByteCode_Ptr->b.l_blk = This_Compile_Linenum;
#else
   (void) tok;
#endif
   Compile_ByteCode_Ptr->bc_main_type = SLANG_BC_LITERAL;
   Compile_ByteCode_Ptr->bc_sub_type = SLANG_UINT_TYPE;

   return 0;
}

static Special_NameTable_Type Special_Name_Table [] =
{
     {"EXECUTE_ERROR_BLOCK", handle_special, NULL, SLANG_BC_X_ERROR},
     {"X_USER_BLOCK0", handle_special, NULL, SLANG_BC_X_USER0},
     {"X_USER_BLOCK1", handle_special, NULL, SLANG_BC_X_USER1},
     {"X_USER_BLOCK2", handle_special, NULL, SLANG_BC_X_USER2},
     {"X_USER_BLOCK3", handle_special, NULL, SLANG_BC_X_USER3},
     {"X_USER_BLOCK4", handle_special, NULL, SLANG_BC_X_USER4},
     {"__FILE__", handle_special_file, NULL, SLANG_BC_LAST_BLOCK},
     {"__LINE__", handle_special_line, NULL, SLANG_BC_LAST_BLOCK},
#if 0
     {"__NAMESPACE__", handle_special_namespace, NULL, SLANG_BC_LAST_BLOCK},
#endif
     {NULL, NULL, NULL, SLANG_BC_LAST_BLOCK}
};

static void compile_hashed_identifier (SLCONST char *name, unsigned long hash, _pSLang_Token_Type *tok)
{
   SLang_Name_Type *entry;
   _pSLang_BC_Type name_type;

   entry = locate_hashed_name (name, hash, 1);

   if (entry == NULL)
     {
	Special_NameTable_Type *nt = Special_Name_Table;

	while (nt->name != NULL)
	  {
	     if (strcmp (name, nt->name))
	       {
		  nt++;
		  continue;
	       }

	     if (0 == (*nt->fun)(nt, tok))
	       lang_try_now ();
	     return;
	  }

	_pSLang_verror (SL_UNDEFINED_NAME, "%s is undefined", name);
	return;
     }

   name_type = (_pSLang_BC_Type)entry->name_type;
   Compile_ByteCode_Ptr->bc_main_type = name_type;

   if (name_type == SLANG_LVARIABLE)   /* == SLANG_BC_LVARIABLE */
     Compile_ByteCode_Ptr->b.i_blk = ((SLang_Local_Var_Type *) entry)->local_var_number;
   else
     Compile_ByteCode_Ptr->b.nt_blk = entry;

   lang_try_now ();
}

static void compile_tmp_variable (SLCONST char *name, unsigned long hash)
{
   SLang_Name_Type *entry;
   unsigned char name_type;

   if (NULL == (entry = locate_hashed_name (name, hash, 1)))
     {
	_pSLang_verror (SL_UNDEFINED_NAME, "%s is undefined", name);
	return;
     }

   name_type = entry->name_type;
   switch (name_type)
     {
      case SLANG_LVARIABLE:
	Compile_ByteCode_Ptr->b.i_blk = ((SLang_Local_Var_Type *) entry)->local_var_number;
	break;

      case SLANG_GVARIABLE:
      case SLANG_PVARIABLE:
	Compile_ByteCode_Ptr->b.nt_blk = entry;
	break;

      default:
	_pSLang_verror (SL_SYNTAX_ERROR, "__tmp(%s) does not specifiy a variable", name);
	return;
     }

   Compile_ByteCode_Ptr->bc_main_type = SLANG_BC_TMP;
   Compile_ByteCode_Ptr->bc_sub_type = name_type;

   lang_try_now ();
}

static void compile_simple (_pSLang_BC_Type main_type)
{
   Compile_ByteCode_Ptr->bc_main_type = main_type;
   Compile_ByteCode_Ptr->bc_sub_type = 0;
   Compile_ByteCode_Ptr->b.blk = NULL;
   lang_try_now ();
}

static void compile_identifier (SLCONST char *name, _pSLang_Token_Type *tok)
{
   compile_hashed_identifier (name, _pSLcompute_string_hash (name), tok);
}

static void compile_call_direct (int (*f) (void), _pSLang_BC_Type byte_code)
{
   Compile_ByteCode_Ptr->b.call_function = f;
   Compile_ByteCode_Ptr->bc_main_type = byte_code;
   Compile_ByteCode_Ptr->bc_sub_type = 0;
   lang_try_now ();
}

static void compile_lvar_call_direct (int (*f)(void), _pSLang_BC_Type bc,
				      _pSLang_BC_Type frame_op)
{
#if 1 && SLANG_OPTIMIZE_FOR_SPEED
   if (0 == try_compressed_bytecode (SLANG_BC_LVARIABLE, bc))
     return;
#else
   (void) bc;
#endif

   compile_call_direct (f, frame_op);
}

static void compile_integer (long i, _pSLang_BC_Type bc_main_type, SLtype bc_sub_type)
{
   Compile_ByteCode_Ptr->b.l_blk = i;
   Compile_ByteCode_Ptr->bc_main_type = bc_main_type;
   Compile_ByteCode_Ptr->bc_sub_type = bc_sub_type;

   lang_try_now ();
}

#ifdef HAVE_LONG_LONG
static void compile_llong (long long i, _pSLang_BC_Type bc_main_type, SLtype bc_sub_type)
{
# if LLONG_IS_NOT_LONG
   long long *ptr;

   if (NULL == (ptr = (long long *) SLmalloc (sizeof(long long))))
     return;
   *ptr = i;

   Compile_ByteCode_Ptr->b.llong_blk = ptr;
# else
   Compile_ByteCode_Ptr->b.l_blk = i;
# endif
   Compile_ByteCode_Ptr->bc_main_type = bc_main_type;
   Compile_ByteCode_Ptr->bc_sub_type = bc_sub_type;

   lang_try_now ();
}
#endif

#if SLANG_HAS_FLOAT
static void compile_double (_pSLang_Token_Type *t, _pSLang_BC_Type main_type, SLtype type)
{
   unsigned int factor = 1;
   double *ptr;
   double d;

   d = _pSLang_atof (t->v.s_val);

#if SLANG_HAS_COMPLEX
   if (type == SLANG_COMPLEX_TYPE) factor = 2;
#endif
   if (NULL == (ptr = (double *) SLmalloc(factor * sizeof(double))))
     return;

   Compile_ByteCode_Ptr->b.double_blk = ptr;
#if SLANG_HAS_COMPLEX
   if (type == SLANG_COMPLEX_TYPE)
     *ptr++ = 0;
#endif
   *ptr = d;

   Compile_ByteCode_Ptr->bc_main_type = main_type;
   Compile_ByteCode_Ptr->bc_sub_type = type;
   lang_try_now ();
}

static void compile_float (_pSLang_Token_Type *t)
{
   float f = (float) _pSLang_atof (t->v.s_val);

   Compile_ByteCode_Ptr->b.float_blk = f;
   Compile_ByteCode_Ptr->bc_main_type = SLANG_BC_LITERAL;
   Compile_ByteCode_Ptr->bc_sub_type = SLANG_FLOAT_TYPE;
   lang_try_now ();
}

#endif

static void compile_string (SLCONST char *s, unsigned long hash)
{
   if (NULL == (Compile_ByteCode_Ptr->b.s_blk = _pSLstring_dup_hashed_string (s, hash)))
     return;

   Compile_ByteCode_Ptr->bc_main_type = SLANG_BC_LITERAL_STR;
   Compile_ByteCode_Ptr->bc_sub_type = SLANG_STRING_TYPE;

   lang_try_now ();
}

static void compile_string_dollar (SLCONST char *s, unsigned long hash)
{
   if (NULL == (Compile_ByteCode_Ptr->b.s_blk = _pSLstring_dup_hashed_string (s, hash)))
     return;

   Compile_ByteCode_Ptr->bc_main_type = SLANG_BC_DOLLAR_STR;
   Compile_ByteCode_Ptr->bc_sub_type = SLANG_STRING_TYPE;

   lang_try_now ();
}

static void compile_bstring (SLang_BString_Type *s)
{
   if (NULL == (Compile_ByteCode_Ptr->b.bs_blk = SLbstring_dup (s)))
     return;

   Compile_ByteCode_Ptr->bc_main_type = SLANG_BC_LITERAL;
   Compile_ByteCode_Ptr->bc_sub_type = SLANG_BSTRING_TYPE;

   lang_try_now ();
}

static SLang_Name_Type *locate_hashed_name_autodeclare (SLFUTURE_CONST char *name, unsigned long hash,
							unsigned char assign_type)
{
   SLang_Name_Type *v;

   v = locate_hashed_name (name, hash, 1);

   if (v != NULL)
     return v;

   if ((_pSLang_Auto_Declare_Globals == 0)
       || Lang_Defining_Function
       || (NULL != strchr (name, '-'))   /* namespace->name form */
       || (assign_type != SLANG_BCST_ASSIGN)
       || (This_Static_NameSpace == NULL))
     {
	_pSLang_verror (SL_UNDEFINED_NAME, "%s is undefined", name);
	return NULL;
     }
   /* Note that function local variables are not at top level */

   /* Variables that are automatically declared are given static
    * scope.
    */
   if ((NULL != SLang_Auto_Declare_Var_Hook)
       && (-1 == (*SLang_Auto_Declare_Var_Hook) (name)))
     return NULL;

   if ((-1 == add_global_variable (name, SLANG_GVARIABLE, hash, This_Static_NameSpace))
       || (NULL == (v = locate_hashed_name (name, hash, 1))))
     return NULL;

   return v;
}

/* assign_type is one of SLANG_BCST_ASSIGN, ... values */
static void compile_assign (unsigned char assign_type,
			    SLFUTURE_CONST char *name, unsigned long hash)
{
   SLang_Name_Type *v;
   _pSLang_BC_Type main_type;
   SLang_Class_Type *cl;

   if (NULL == (v = locate_hashed_name_autodeclare (name, hash, assign_type)))
     return;

   switch (v->name_type)
     {
      case SLANG_LVARIABLE:
	main_type = SLANG_BC_SET_LOCAL_LVALUE;
	Compile_ByteCode_Ptr->b.i_blk = ((SLang_Local_Var_Type *) v)->local_var_number;
	break;

      case SLANG_GVARIABLE:
      case SLANG_PVARIABLE:
	main_type = SLANG_BC_SET_GLOBAL_LVALUE;
	Compile_ByteCode_Ptr->b.nt_blk = v;
	break;

      case SLANG_IVARIABLE:
	GET_CLASS(cl, ((SLang_Intrin_Var_Type *)v)->type);
	if (cl->cl_class_type != SLANG_CLASS_TYPE_SCALAR)
	  {
	     _pSLang_verror (SL_Forbidden_Error, "Assignment to %s is not allowed", name);
	     return;
	  }
	main_type = SLANG_BC_SET_INTRIN_LVALUE;
	Compile_ByteCode_Ptr->b.nt_blk = v;
	break;

      case SLANG_RVARIABLE:
	_pSLang_verror (SL_READONLY_ERROR, "%s is read-only", name);
	return;

      default:
	_pSLang_verror (SL_Forbidden_Error, "%s may not be used as an lvalue", name);
	return;
     }

   Compile_ByteCode_Ptr->bc_sub_type = assign_type;
   Compile_ByteCode_Ptr->bc_main_type = main_type;

   lang_try_now ();
}

#if 0
static void compile_deref_assign (char *name, unsigned long hash)
{
   SLang_Name_Type *v;

   v = locate_hashed_name (name, hash, 1);

   if (v == NULL)
     {
	_pSLang_verror (SL_UNDEFINED_NAME, "%s is undefined", name);
	return;
     }

   switch (v->name_type)
     {
      case SLANG_LVARIABLE:
	Compile_ByteCode_Ptr->b.i_blk = ((SLang_Local_Var_Type *) v)->local_var_number;
	break;

      case SLANG_GVARIABLE:
      case SLANG_PVARIABLE:
	Compile_ByteCode_Ptr->b.nt_blk = v;
	break;

      default:
	/* FIXME: Priority=low
	 * This could be made to work.  It is not a priority because
	 * I cannot imagine application intrinsics which are references.
	 */
	_pSLang_verror (SL_NOT_IMPLEMENTED, "Deref assignment to %s is not allowed", name);
	return;
     }

   Compile_ByteCode_Ptr->bc_sub_type = v->name_type;
   Compile_ByteCode_Ptr->bc_main_type = SLANG_BC_DEREF_ASSIGN;

   lang_try_now ();
}
#endif
static void
compile_struct_assign (_pSLang_Token_Type *t)
{
   Compile_ByteCode_Ptr->bc_sub_type = SLANG_BCST_ASSIGN + (t->type - _STRUCT_ASSIGN_TOKEN);
   Compile_ByteCode_Ptr->bc_main_type = SLANG_BC_SET_STRUCT_LVALUE;
   Compile_ByteCode_Ptr->b.s_blk = _pSLstring_dup_hashed_string (t->v.s_val, t->hash);
   lang_try_now ();
}

static void
compile_array_assign (_pSLang_Token_Type *t)
{
   Compile_ByteCode_Ptr->bc_sub_type = SLANG_BCST_ASSIGN + (t->type - _ARRAY_ASSIGN_TOKEN);
   Compile_ByteCode_Ptr->bc_main_type = SLANG_BC_SET_ARRAY_LVALUE;
   Compile_ByteCode_Ptr->b.s_blk = NULL;
   lang_try_now ();
}

static void
compile_deref_assign (_pSLang_Token_Type *t)
{
   Compile_ByteCode_Ptr->bc_sub_type = SLANG_BCST_ASSIGN + (t->type - _DEREF_ASSIGN_TOKEN);
   Compile_ByteCode_Ptr->bc_main_type = SLANG_BC_SET_DEREF_LVALUE;
   Compile_ByteCode_Ptr->b.s_blk = NULL;
   lang_try_now ();
}

static void compile_dot (_pSLang_Token_Type *t, _pSLang_BC_Type bc_main_type)
{
   Compile_ByteCode_Ptr->bc_main_type = bc_main_type;
   Compile_ByteCode_Ptr->b.s_blk = _pSLstring_dup_hashed_string(t->v.s_val, t->hash);
   lang_try_now ();
}

static void compile_ref (SLFUTURE_CONST char *name, unsigned long hash)
{
   SLang_Name_Type *entry;
   _pSLang_BC_Type main_type;

   if (NULL == (entry = locate_hashed_name_autodeclare (name, hash, SLANG_BCST_ASSIGN)))
     return;

   main_type = (_pSLang_BC_Type) entry->name_type;

   if (main_type == SLANG_LVARIABLE)
     {
	main_type = SLANG_BC_LOBJPTR;
	Compile_ByteCode_Ptr->b.i_blk = ((SLang_Local_Var_Type *)entry)->local_var_number;
     }
   else
     {
	main_type = SLANG_BC_GOBJPTR;
	Compile_ByteCode_Ptr->b.nt_blk = entry;
     }

   Compile_ByteCode_Ptr->bc_main_type = main_type;
   lang_try_now ();
}

static void compile_break (_pSLang_BC_Type break_type,
			   int requires_block, int requires_fun,
			   SLCONST char *str, int opt_val)
{
   if ((requires_fun
	&& (Lang_Defining_Function == 0))
       || (requires_block
	   && (This_Compile_Block_Type != COMPILE_BLOCK_TYPE_BLOCK)))
     {
	_pSLang_verror (SL_SYNTAX_ERROR, "misplaced %s", str);
	return;
     }

   Compile_ByteCode_Ptr->bc_main_type = break_type;
   Compile_ByteCode_Ptr->bc_sub_type = 0;
   Compile_ByteCode_Ptr->b.i_blk = opt_val;

   lang_try_now ();
}

static void compile_public_variable_mode (_pSLang_Token_Type *t)
{
   if (t->type == IDENT_TOKEN)
     {
	if (-1 == check_linkage (t->v.s_val, t->hash, 1))
	  return;
	add_global_variable (t->v.s_val, SLANG_GVARIABLE, t->hash, Global_NameSpace);
     }
   else if (t->type == CBRACKET_TOKEN)
     Compile_Mode_Function = compile_basic_token_mode;
   else
     _pSLang_verror (SL_SYNTAX_ERROR, "Misplaced token in variable list");
}

static void compile_local_variable_mode (_pSLang_Token_Type *t)
{
   if (Locals_NameSpace == NULL)
     {
	Locals_NameSpace = _pSLns_allocate_namespace ("**locals**", SLLOCALS_HASH_TABLE_SIZE);
	if (Locals_NameSpace == NULL)
	  return;
     }

   if (t->type == IDENT_TOKEN)
     add_local_variable (t->v.s_val, t->hash);
   else if (t->type == CBRACKET_TOKEN)
     Compile_Mode_Function = compile_basic_token_mode;
   else
     _pSLang_verror (SL_SYNTAX_ERROR, "Misplaced token in variable list");
}

static void compile_static_variable_mode (_pSLang_Token_Type *t)
{
   if (t->type == IDENT_TOKEN)
     {
	if (-1 == check_linkage (t->v.s_val, t->hash, 0))
	  return;
	add_global_variable (t->v.s_val, SLANG_GVARIABLE, t->hash, This_Static_NameSpace);
     }
   else if (t->type == CBRACKET_TOKEN)
     Compile_Mode_Function = compile_basic_token_mode;
   else
     _pSLang_verror (SL_SYNTAX_ERROR, "Misplaced token in variable list");
}

static void compile_private_variable_mode (_pSLang_Token_Type *t)
{
   if (t->type == IDENT_TOKEN)
     add_global_variable (t->v.s_val, SLANG_PVARIABLE, t->hash, This_Private_NameSpace);
   else if (t->type == CBRACKET_TOKEN)
     Compile_Mode_Function = compile_basic_token_mode;
   else
     _pSLang_verror (SL_SYNTAX_ERROR, "Misplaced token in variable list");
}

static void compile_function_mode (_pSLang_Token_Type *t)
{
   if (-1 == lang_check_space ())
     return;

   if (t->type != IDENT_TOKEN)
     _pSLang_verror (SL_SYNTAX_ERROR, "Expecting a function name");
   else
     lang_define_function (t->v.s_val, SLANG_FUNCTION, t->hash, Global_NameSpace);

   Compile_Mode_Function = compile_basic_token_mode;
}

/* An error block is not permitted to contain continue or break statements.
 * This restriction may be removed later but for now reject them.
 */
static int check_error_block (void)
{
   SLBlock_Type *p;
   _pSLang_BC_Type t;

   /* Back up to the block and then scan it. */
   p = (Compile_ByteCode_Ptr - 1)->b.blk;

   while (0 != (t = p->bc_main_type))
     {
	if ((t == SLANG_BC_BREAK)
	    || (t == SLANG_BC_CONTINUE))
	  {
	     _pSLang_verror (SL_SYNTAX_ERROR,
			   "An ERROR_BLOCK is not permitted to contain continue or break statements");
	     return -1;
	  }
	p++;
     }
   return 0;
}

/* The only allowed tokens are the directives and another block start.
 * The mode is only active if a block is available.  The inner_interp routine
 * expects such safety checks.
 */
static void compile_directive_mode (_pSLang_Token_Type *t)
{
   int bc_sub_type;
   int delay;

   if (-1 == lang_check_space ())
     return;

   bc_sub_type = -1;
   delay = 0;
   switch (t->type)
     {
      case FOREVER_TOKEN:
	bc_sub_type = SLANG_BCST_FOREVER;
	delay = 1;
	break;

      case IFNOT_TOKEN:
	bc_sub_type = SLANG_BCST_IFNOT;
	break;

      case IF_TOKEN:
	bc_sub_type = SLANG_BCST_IF;
	break;

      case ANDELSE_TOKEN:
	bc_sub_type = SLANG_BCST_ANDELSE;
	break;

      case SWITCH_TOKEN:
	bc_sub_type = SLANG_BCST_SWITCH;
	break;

      case EXITBLK_TOKEN:
	if (Lang_Defining_Function == 0)
	  {
	     _pSLang_verror (SL_SYNTAX_ERROR, "misplaced EXIT_BLOCK");
	     break;
	  }
	bc_sub_type = SLANG_BCST_EXIT_BLOCK;
	break;

      case ERRBLK_TOKEN:
	if (This_Compile_Block_Type == COMPILE_BLOCK_TYPE_TOP_LEVEL)
	  {
	     _pSLang_verror (SL_SYNTAX_ERROR, "misplaced ERROR_BLOCK");
	     break;
	  }
	if (0 == check_error_block ())
	  bc_sub_type = SLANG_BCST_ERROR_BLOCK;
	break;

      case USRBLK0_TOKEN:
      case USRBLK1_TOKEN:
      case USRBLK2_TOKEN:
      case USRBLK3_TOKEN:
      case USRBLK4_TOKEN:
	/* if (This_Compile_Block_Type == COMPILE_BLOCK_TYPE_TOP_LEVEL) */
	if (Lang_Defining_Function == 0)
	  {
	     _pSLang_verror (SL_SYNTAX_ERROR, "misplaced USER_BLOCK");
	     break;
	  }
	bc_sub_type = SLANG_BCST_USER_BLOCK0 + (t->type - USRBLK0_TOKEN);
	break;

      case NOTELSE_TOKEN:
	bc_sub_type = SLANG_BCST_NOTELSE;
	break;

      case ELSE_TOKEN:
	bc_sub_type = SLANG_BCST_ELSE;
	break;
#ifdef LOOP_ELSE_TOKEN
      case LOOP_ELSE_TOKEN:
	bc_sub_type = SLANG_BCST_LOOP_ELSE;
	break;
#endif
      case LOOP_THEN_TOKEN:
	bc_sub_type = SLANG_BCST_LOOP_THEN;
	break;

      case LOOP_TOKEN:
	bc_sub_type = SLANG_BCST_LOOP;
	delay = 1;
	break;

      case DOWHILE_TOKEN:
	bc_sub_type = SLANG_BCST_DOWHILE;
	delay = 1;
	break;

      case WHILE_TOKEN:
	bc_sub_type = SLANG_BCST_WHILE;
	delay = 1;
	break;

      case ORELSE_TOKEN:
	bc_sub_type = SLANG_BCST_ORELSE;
	break;

      case _FOR_TOKEN:
	bc_sub_type = SLANG_BCST_FOR;
	delay = 1;
	break;

      case FOR_TOKEN:
	bc_sub_type = SLANG_BCST_CFOR;
	delay = 1;
	break;

      case FOREACH_TOKEN:
	delay = 1;
	bc_sub_type = SLANG_BCST_FOREACH;
	break;

      case FOREACH_EARGS_TOKEN:
	delay = 1;
	bc_sub_type = SLANG_BCST_FOREACH_EARGS;
	break;

      case OBRACE_TOKEN:
	lang_begin_block ();
	break;

      case TRY_TOKEN:
	bc_sub_type = SLANG_BCST_TRY;
	break;

      case SC_OR_TOKEN:
	bc_sub_type = SLANG_BCST_SC_OR;
	break;

      case SC_AND_TOKEN:
	bc_sub_type = SLANG_BCST_SC_AND;
	break;

      case _COMPARE_TOKEN:
	bc_sub_type = SLANG_BCST_COMPARE;
	break;

      default:
	_pSLang_verror (SL_SYNTAX_ERROR, "Expecting directive token.  Found 0x%X", t->type);
	break;
     }

   /* Reset this pointer first because compile_directive may cause a
    * file to be loaded.
    */
   Compile_Mode_Function = compile_basic_token_mode;

   if (bc_sub_type != -1)
     compile_directive (bc_sub_type, delay);
}

static unsigned int Assign_Mode_Type;
static void compile_assign_mode (_pSLang_Token_Type *t)
{
   if (t->type != IDENT_TOKEN)
     {
	_pSLang_verror (SL_SYNTAX_ERROR, "Expecting identifier for assignment");
	return;
     }

   compile_assign (Assign_Mode_Type, t->v.s_val, t->hash);
   Compile_Mode_Function = compile_basic_token_mode;
}

static void compile_basic_token_mode (_pSLang_Token_Type *t)
{
   if (-1 == lang_check_space ())
     return;

   switch (t->type)
     {
      case EOF_TOKEN:
      case NOP_TOKEN:
	interp_pending_blocks ();
	break;

      case PUSH_TOKEN:
      case READONLY_TOKEN:
      case DO_TOKEN:
      case VARIABLE_TOKEN:
      case SEMICOLON_TOKEN:
      default:
	_pSLang_verror (SL_SYNTAX_ERROR, "Unknown or unsupported token type 0x%X", t->type);
	break;

      case DEREF_TOKEN:
	compile_call_direct (dereference_object, SLANG_BC_CALL_DIRECT);
	break;

      case _DEREF_OBSOLETE_FUNCALL_TOKEN:
	compile_simple (SLANG_BC_OBSOLETE_DEREF_FUN_CALL);
	break;

      case _DEREF_FUNCALL_TOKEN:
	compile_simple (SLANG_BC_DEREF_FUN_CALL);
	break;

      case STRUCT_TOKEN:
	compile_call_direct (_pSLstruct_define_struct, SLANG_BC_CALL_DIRECT);
	break;

      case STRUCT_WITH_ASSIGN_TOKEN:
	compile_call_direct (_pSLstruct_define_struct2, SLANG_BC_CALL_DIRECT);
	break;

      case TYPEDEF_TOKEN:
	compile_call_direct (_pSLstruct_define_typedef, SLANG_BC_CALL_DIRECT);
	break;

      case DOT_TOKEN:		       /* X . field */
	compile_dot (t, SLANG_BC_FIELD);
	break;

      case DOT_METHOD_CALL_TOKEN:      /* X . field (args) */
	compile_dot (t, SLANG_BC_METHOD);
	break;

      case COMMA_TOKEN:
	break;			       /* do nothing */

      case IDENT_TOKEN:
	compile_hashed_identifier (t->v.s_val, t->hash, t);
	break;

      case _REF_TOKEN:
	compile_ref (t->v.s_val, t->hash);
	break;

      case ARG_TOKEN:
	compile_call_direct (start_arg_list, SLANG_BC_CALL_DIRECT);
	break;

      case EARG_TOKEN:
	compile_lvar_call_direct (end_arg_list, SLANG_BC_EARG_LVARIABLE, SLANG_BC_CALL_DIRECT);
	break;

      case COLON_TOKEN:
	if (This_Compile_Block_Type == COMPILE_BLOCK_TYPE_BLOCK)
	  compile_simple (SLANG_BC_LABEL);
	else (void) SLang_set_error (SL_SYNTAX_ERROR);
	break;

      case POP_TOKEN:
	compile_call_direct (SLdo_pop, SLANG_BC_CALL_DIRECT);
	break;

      case CASE_TOKEN:
	if (This_Compile_Block_Type != COMPILE_BLOCK_TYPE_BLOCK)
	  _pSLang_verror (SL_SYNTAX_ERROR, "Misplaced 'case'");
	else
	  compile_call_direct (case_function, SLANG_BC_CALL_DIRECT);
	break;

      case CHAR_TOKEN:
	compile_integer (t->v.long_val, SLANG_BC_LITERAL, SLANG_CHAR_TYPE);
	break;
      case UCHAR_TOKEN:
	compile_integer (t->v.long_val, SLANG_BC_LITERAL, SLANG_UCHAR_TYPE);
	break;
#if SHORT_IS_NOT_INT
      case SHORT_TOKEN:
	compile_integer (t->v.long_val, SLANG_BC_LITERAL, SLANG_SHORT_TYPE);
	break;
      case USHORT_TOKEN:
	compile_integer (t->v.long_val, SLANG_BC_LITERAL, SLANG_USHORT_TYPE);
	break;
#endif
#if SHORT_IS_INT
      case SHORT_TOKEN:
#endif
#if LONG_IS_INT
      case LONG_TOKEN:
#endif
      case INT_TOKEN:
	compile_integer (t->v.long_val, SLANG_BC_LITERAL_INT, SLANG_INT_TYPE);
	break;
#if SHORT_IS_INT
      case USHORT_TOKEN:
#endif
#if LONG_IS_INT
      case ULONG_TOKEN:
#endif
      case UINT_TOKEN:
	compile_integer (t->v.long_val, SLANG_BC_LITERAL, SLANG_UINT_TYPE);
	break;
#if LONG_IS_NOT_INT
      case LONG_TOKEN:
	compile_integer (t->v.long_val, SLANG_BC_LITERAL, SLANG_LONG_TYPE);
	break;
      case ULONG_TOKEN:
	compile_integer (t->v.long_val, SLANG_BC_LITERAL, SLANG_ULONG_TYPE);
	break;
#endif

#if SLANG_HAS_FLOAT
      case FLOAT_TOKEN:
	compile_float (t);
	break;

      case DOUBLE_TOKEN:
	compile_double (t, SLANG_BC_LITERAL_DBL, SLANG_DOUBLE_TYPE);
	break;
#endif
#if SLANG_HAS_COMPLEX
      case COMPLEX_TOKEN:
	compile_double (t, SLANG_BC_LITERAL, SLANG_COMPLEX_TYPE);
	break;
#endif
#ifdef HAVE_LONG_LONG
      case LLONG_TOKEN:
	compile_llong (t->v.llong_val, SLANG_BC_LITERAL, _pSLANG_LLONG_TYPE);
	break;
      case ULLONG_TOKEN:
	compile_llong (t->v.ullong_val, SLANG_BC_LITERAL, _pSLANG_ULLONG_TYPE);
	break;
#endif
      case STRING_TOKEN:
	compile_string (t->v.s_val, t->hash);
	break;

      case STRING_DOLLAR_TOKEN:
	compile_string_dollar (t->v.s_val, t->hash);
	break;

      case _BSTRING_TOKEN:
	  {
	     SLang_BString_Type *b = SLbstring_create ((unsigned char *)t->v.s_val, (unsigned int) t->hash);
	     if (b != NULL)
	       {
		  compile_bstring (b);
		  SLbstring_free (b);
	       }
	  }
	break;

      case BSTRING_TOKEN:
	compile_bstring (t->v.b_val);
	break;

      case MULTI_STRING_TOKEN:
	  {
	     _pSLang_Multiline_String_Type *m = t->v.multistring_val;
	     if (m->type == STRING_TOKEN)
	       compile_string (m->v.s_val, m->hash);
	     else if (m->type == STRING_DOLLAR_TOKEN)
	       compile_string_dollar (m->v.s_val, m->hash);
	     else if (m->type == BSTRING_TOKEN)
	       compile_bstring (m->v.b_val);
	  }
	break;

      case _NULL_TOKEN:
	compile_identifier ("NULL", t);
	break;

      case _INLINE_WILDCARD_ARRAY_TOKEN:
	compile_call_direct (_pSLarray_wildcard_array, SLANG_BC_CALL_DIRECT);
	break;

      case _INLINE_ARRAY_TOKEN:
	compile_call_direct (_pSLarray_inline_array, SLANG_BC_CALL_DIRECT_NARGS);
	break;

      case _INLINE_IMPLICIT_ARRAY_TOKEN:
	compile_call_direct (_pSLarray_inline_implicit_array, SLANG_BC_CALL_DIRECT_NARGS);
	break;

      case _INLINE_LIST_TOKEN:
	compile_call_direct (_pSLlist_inline_list, SLANG_BC_CALL_DIRECT_NARGS);
	break;

      case ARRAY_TOKEN:
	compile_lvar_call_direct (_pSLarray_aget, SLANG_BC_LVARIABLE_AGET, SLANG_BC_CALL_DIRECT_NARGS);
	break;

      case _INLINE_IMPLICIT_ARRAYN_TOKEN:
	compile_call_direct (_pSLarray_inline_implicit_arrayn, SLANG_BC_CALL_DIRECT_NARGS);
	break;

	/* Note: I need to add the other _ARRAY assign tokens. */
      case _ARRAY_PLUSEQS_TOKEN:
      case _ARRAY_MINUSEQS_TOKEN:
      case _ARRAY_TIMESEQS_TOKEN:
      case _ARRAY_DIVEQS_TOKEN:
      case _ARRAY_BOREQS_TOKEN:
      case _ARRAY_BANDEQS_TOKEN:
      case _ARRAY_POST_MINUSMINUS_TOKEN:
      case _ARRAY_MINUSMINUS_TOKEN:
      case _ARRAY_POST_PLUSPLUS_TOKEN:
      case _ARRAY_PLUSPLUS_TOKEN:
	compile_array_assign (t);
	break;

      case _ARRAY_ASSIGN_TOKEN:
	compile_lvar_call_direct (_pSLarray_aput, SLANG_BC_LVARIABLE_APUT, SLANG_BC_CALL_DIRECT_NARGS);
	break;

      case _STRUCT_ASSIGN_TOKEN:
      case _STRUCT_PLUSEQS_TOKEN:
      case _STRUCT_MINUSEQS_TOKEN:
      case _STRUCT_TIMESEQS_TOKEN:
      case _STRUCT_DIVEQS_TOKEN:
      case _STRUCT_BOREQS_TOKEN:
      case _STRUCT_BANDEQS_TOKEN:
      case _STRUCT_POST_MINUSMINUS_TOKEN:
      case _STRUCT_MINUSMINUS_TOKEN:
      case _STRUCT_POST_PLUSPLUS_TOKEN:
      case _STRUCT_PLUSPLUS_TOKEN:
	compile_struct_assign (t);
	break;

      case _SCALAR_ASSIGN_TOKEN:
      case _SCALAR_PLUSEQS_TOKEN:
      case _SCALAR_MINUSEQS_TOKEN:
      case _SCALAR_TIMESEQS_TOKEN:
      case _SCALAR_DIVEQS_TOKEN:
      case _SCALAR_BOREQS_TOKEN:
      case _SCALAR_BANDEQS_TOKEN:
      case _SCALAR_POST_MINUSMINUS_TOKEN:
      case _SCALAR_MINUSMINUS_TOKEN:
      case _SCALAR_POST_PLUSPLUS_TOKEN:
      case _SCALAR_PLUSPLUS_TOKEN:
	compile_assign (SLANG_BCST_ASSIGN + (t->type - _SCALAR_ASSIGN_TOKEN),
			t->v.s_val, t->hash);
	break;

      case _DEREF_ASSIGN_TOKEN:
      case _DEREF_PLUSEQS_TOKEN:
      case _DEREF_MINUSEQS_TOKEN:
      case _DEREF_TIMESEQS_TOKEN:
      case _DEREF_DIVEQS_TOKEN:
      case _DEREF_BOREQS_TOKEN:
      case _DEREF_BANDEQS_TOKEN:
      case _DEREF_PLUSPLUS_TOKEN:
      case _DEREF_POST_PLUSPLUS_TOKEN:
      case _DEREF_MINUSMINUS_TOKEN:
      case _DEREF_POST_MINUSMINUS_TOKEN:
	compile_deref_assign (t);
	break;

 	/* For processing RPN tokens */
      case ASSIGN_TOKEN:
      case PLUSEQS_TOKEN:
      case MINUSEQS_TOKEN:
      case TIMESEQS_TOKEN:
      case DIVEQS_TOKEN:
      case BOREQS_TOKEN:
      case BANDEQS_TOKEN:
      case POST_MINUSMINUS_TOKEN:
      case MINUSMINUS_TOKEN:
      case POST_PLUSPLUS_TOKEN:
      case PLUSPLUS_TOKEN:
	Compile_Mode_Function = compile_assign_mode;
	Assign_Mode_Type = SLANG_BCST_ASSIGN + (t->type - ASSIGN_TOKEN);
	break;

      case LT_TOKEN:
	compile_binary (SLANG_LT);
	break;

      case LE_TOKEN:
	compile_binary (SLANG_LE);
	break;

      case GT_TOKEN:
	compile_binary (SLANG_GT);
	break;

      case GE_TOKEN:
	compile_binary (SLANG_GE);
	break;

      case EQ_TOKEN:
	compile_binary (SLANG_EQ);
	break;

      case NE_TOKEN:
	compile_binary (SLANG_NE);
	break;

      case AND_TOKEN:
	compile_binary (SLANG_AND);
	break;

      case ADD_TOKEN:
	compile_binary (SLANG_PLUS);
	break;

      case SUB_TOKEN:
	compile_binary (SLANG_MINUS);
	break;

      case TIMES_TOKEN:
	compile_binary (SLANG_TIMES);
	break;

      case DIV_TOKEN:
	compile_binary (SLANG_DIVIDE);
	break;

      case POW_TOKEN:
	compile_binary (SLANG_POW);
	break;

      case BXOR_TOKEN:
	compile_binary (SLANG_BXOR);
	break;

      case BAND_TOKEN:
	compile_binary (SLANG_BAND);
	break;

      case BOR_TOKEN:
	compile_binary (SLANG_BOR);
	break;

      case SHR_TOKEN:
	compile_binary (SLANG_SHR);
	break;

      case SHL_TOKEN:
	compile_binary (SLANG_SHL);
	break;

      case MOD_TOKEN:
	compile_binary (SLANG_MOD);
	break;

      case OR_TOKEN:
	compile_binary (SLANG_OR);
	break;

      case NOT_TOKEN:
	compile_unary (SLANG_NOT, SLANG_BC_UNARY);
	break;

      case BNOT_TOKEN:
	compile_unary (SLANG_BNOT, SLANG_BC_UNARY);
	break;
      case CHS_TOKEN:
	compile_unary (SLANG_CHS, SLANG_BC_UNARY);
	break;
#if 0
      case MUL2_TOKEN:
	compile_unary (SLANG_MUL2, SLANG_BC_ARITH_UNARY);
	break;
      case ABS_TOKEN:
	compile_unary (SLANG_ABS, SLANG_BC_ARITH_UNARY);
	break;
      case SQR_TOKEN:
	compile_unary (SLANG_SQR, SLANG_BC_ARITH_UNARY);
	break;
      case SIGN_TOKEN:
	compile_unary (SLANG_SIGN, SLANG_BC_ARITH_UNARY);
	break;
#endif
      case BREAK_TOKEN:
	compile_break (SLANG_BC_BREAK, 1, 0, "break", 1);
	break;

      case BREAK_N_TOKEN:
	compile_break (SLANG_BC_BREAK_N, 1, 0, "break", abs(t->v.long_val));
	break;

      case RETURN_TOKEN:
	compile_break (SLANG_BC_RETURN, 0, 1, "return", 1);
	break;

      case CONT_TOKEN:
	compile_break (SLANG_BC_CONTINUE, 1, 0, "continue", 1);
	break;
      case CONT_N_TOKEN:
	compile_break (SLANG_BC_CONTINUE_N, 1, 0, "continue", abs(t->v.long_val));
	break;
      case EXCH_TOKEN:
	compile_break (SLANG_BC_EXCH, 0, 0, "", 0);   /* FIXME: Priority=low */
	break;

      case STATIC_TOKEN:
	interp_pending_blocks ();
	if (Lang_Defining_Function == 0)
	  Compile_Mode_Function = compile_static_variable_mode;
	else
	  _pSLang_verror (SL_NOT_IMPLEMENTED, "static variables not permitted in functions");
	break;

      case PRIVATE_TOKEN:
	interp_pending_blocks ();
	if (Lang_Defining_Function == 0)
	  Compile_Mode_Function = compile_private_variable_mode;
	else
	  _pSLang_verror (SL_NOT_IMPLEMENTED, "private variables not permitted in functions");
	break;

      case PUBLIC_TOKEN:
	interp_pending_blocks ();
	if (Lang_Defining_Function == 0)
	  Compile_Mode_Function = compile_public_variable_mode;
	else
	  _pSLang_verror (SL_NOT_IMPLEMENTED, "public variables not permitted in functions");
	break;

      case OBRACKET_TOKEN:
	interp_pending_blocks ();
	if (Lang_Defining_Function == 0)
	  Compile_Mode_Function = Default_Variable_Mode;
	else
	  Compile_Mode_Function = compile_local_variable_mode;
	break;

      case OPAREN_TOKEN:
	interp_pending_blocks ();
	lang_begin_function ();
	break;

      case DEFINE_STATIC_TOKEN:
	if (Lang_Defining_Function)
	  define_static_function (t->v.s_val, t->hash);
	else (void) SLang_set_error (SL_SYNTAX_ERROR);
	break;

      case DEFINE_PRIVATE_TOKEN:
	interp_pending_blocks ();
	if (Lang_Defining_Function)
	  define_private_function (t->v.s_val, t->hash);
	else (void) SLang_set_error (SL_SYNTAX_ERROR);
	break;

      case DEFINE_PUBLIC_TOKEN:
	if (Lang_Defining_Function)
	  define_public_function (t->v.s_val, t->hash);
	else (void) SLang_set_error (SL_SYNTAX_ERROR);
	break;

      case THROW_TOKEN:
	compile_call_direct (_pSLerr_throw, SLANG_BC_CALL_DIRECT_NARGS);
	break;

      case DEFINE_TOKEN:
	if (Lang_Defining_Function)
	  (*Default_Define_Function) (t->v.s_val, t->hash);
	else
	  (void) SLang_set_error (SL_SYNTAX_ERROR);
	break;

      case CPAREN_TOKEN:
	if (Lang_Defining_Function)
	  Compile_Mode_Function = compile_function_mode;
	else (void) SLang_set_error (SL_SYNTAX_ERROR);
	break;

      case CBRACE_TOKEN:
	lang_end_block ();
	Compile_Mode_Function = compile_directive_mode;
	break;

      case OBRACE_TOKEN:
	lang_begin_block ();
	break;

      case FARG_TOKEN:
	Function_Args_Number = Local_Variable_Number;
	break;

      case TMP_TOKEN:
	compile_tmp_variable (t->v.s_val, t->hash);
	break;
#if SLANG_HAS_QUALIFIERS
      case QUALIFIER_TOKEN:
	compile_call_direct (set_qualifier, SLANG_BC_CALL_DIRECT);
	break;
#endif
      case BOS_TOKEN:
#if SLANG_HAS_DEBUG_CODE && SLANG_HAS_BOSEOS
	compile_line_info (SLANG_BC_BOS, This_Compile_Filename, t->v.long_val);
#endif
	break;
      case EOS_TOKEN:
#if SLANG_HAS_DEBUG_CODE && SLANG_HAS_BOSEOS
	compile_simple (SLANG_BC_EOS);
#endif
	break;

      case LINE_NUM_TOKEN:
#if SLANG_HAS_DEBUG_CODE
	set_line_number_info (t->v.long_val);
#endif
	break;

      case _ARRAY_ELEM_REF_TOKEN:
	compile_call_direct (_pSLarray_push_elem_ref, SLANG_BC_CALL_DIRECT_NARGS);
	break;

      case _STRUCT_FIELD_REF_TOKEN:
	compile_dot (t, SLANG_BC_FIELD_REF);
	break;

      case POUND_TOKEN:
	compile_call_direct (_pSLarray_matrix_multiply, SLANG_BC_CALL_DIRECT);
	break;
     }
}

void _pSLcompile (_pSLang_Token_Type *t)
{
   if (SLang_get_error () == 0)
     {
	if (Compile_Mode_Function != compile_basic_token_mode)
	  {
	     if (Compile_Mode_Function == NULL)
	       Compile_Mode_Function = compile_basic_token_mode;
	     if (t->type == LINE_NUM_TOKEN)
	       {
		  compile_basic_token_mode (t);
		  return;
	       }
	  }

	(*Compile_Mode_Function) (t);
     }

   if (SLang_get_error ())
     {
	Compile_Mode_Function = compile_basic_token_mode;
	reset_active_interpreter ();
     }
}

void (*_pSLcompile_ptr)(_pSLang_Token_Type *) = _pSLcompile;

typedef struct _Compile_Context_Type
{
   struct _Compile_Context_Type *next;
   SLang_NameSpace_Type *static_namespace;
   SLang_NameSpace_Type *private_namespace;
   SLang_NameSpace_Type *locals_namespace;
   void (*compile_variable_mode) (_pSLang_Token_Type *);
   void (*define_function) (SLFUTURE_CONST char *, unsigned long);
   int lang_defining_function;
   int local_variable_number;
   char *local_variable_names[SLANG_MAX_LOCAL_VARIABLES];
   unsigned int function_args_number;
   void (*compile_mode_function)(_pSLang_Token_Type *);
   SLFUTURE_CONST char *compile_filename;
   unsigned int compile_linenum;
   _pSLang_Function_Type *current_function;
   Function_Header_Type *current_function_header;
}
Compile_Context_Type;

static Compile_Context_Type *Compile_Context_Stack;

/* The only way the push/pop_context functions can get called is via
 * an eval type function.  That can only happen when executed from a
 * top level block.  This means that Compile_ByteCode_Ptr can always be
 * rest back to the beginning of a block.
 */

static int pop_compile_context (void)
{
   Compile_Context_Type *cc;

   if (NULL == (cc = Compile_Context_Stack))
     return -1;

   This_Static_NameSpace = cc->static_namespace;
   This_Private_NameSpace = cc->private_namespace;
   Compile_Context_Stack = cc->next;
   Default_Variable_Mode = cc->compile_variable_mode;
   Default_Define_Function = cc->define_function;
   Compile_Mode_Function = cc->compile_mode_function;

   Lang_Defining_Function = cc->lang_defining_function;
   Local_Variable_Number = cc->local_variable_number;
   memcpy ((char *)Local_Variable_Names, (char *)cc->local_variable_names, sizeof(Local_Variable_Names));

   Function_Args_Number = cc->function_args_number;

   SLang_free_slstring ((char *) This_Compile_Filename);
   This_Compile_Filename = cc->compile_filename;
   This_Compile_Linenum = cc->compile_linenum;

   Current_Function_Header = cc->current_function_header;
   Current_Function = cc->current_function;

   Locals_NameSpace = cc->locals_namespace;

   /* These should be when returning from a compile context */
   Lang_Return = 0;
   Lang_Break = 0;
   Lang_Break_Condition = 0;

   SLfree ((char *) cc);

   return decrement_slang_frame_pointer ();
}

static int push_compile_context (SLFUTURE_CONST char *name)
{
   Compile_Context_Type *cc;

   cc = (Compile_Context_Type *)SLmalloc (sizeof (Compile_Context_Type));
   if (cc == NULL)
     return -1;
   memset ((char *) cc, 0, sizeof (Compile_Context_Type));

   if ((name != NULL)
       && (NULL == (name = SLang_create_slstring (name))))
     {
	SLfree ((char *) cc);
	return -1;
     }

   /* This is necessary since Current_Function and Current_Function_Header
    * will be set to NULL.  Otherwise, top-level blocks would not get
    * properly counted by the _get_frame_depth function.  It is also probably
    * not necessary to add the current_function and current_function_header
    * to the compile context structure.
    */
   if (-1 == increment_slang_frame_pointer (NULL, This_Compile_Linenum))
     {
	SLfree ((char *)cc);
	SLang_free_slstring ((char *) name);    /* NULL ok */
	return -1;
     }

   cc->compile_filename = This_Compile_Filename;
   This_Compile_Filename = name;
   cc->compile_linenum = This_Compile_Linenum;
   This_Compile_Linenum = 0;

   cc->static_namespace = This_Static_NameSpace;
   cc->private_namespace = This_Private_NameSpace;
   cc->compile_variable_mode = Default_Variable_Mode;
   cc->define_function = Default_Define_Function;
   cc->locals_namespace = Locals_NameSpace;

   cc->lang_defining_function = Lang_Defining_Function;
   cc->local_variable_number = Local_Variable_Number;
   memcpy ((char *)cc->local_variable_names, (char *)Local_Variable_Names, sizeof(Local_Variable_Names));

   cc->function_args_number = Function_Args_Number;
   cc->compile_mode_function = Compile_Mode_Function;

   cc->current_function_header = Current_Function_Header;
   cc->current_function = Current_Function;

   cc->next = Compile_Context_Stack;
   Compile_Context_Stack = cc;

   Compile_Mode_Function = compile_basic_token_mode;
   Default_Variable_Mode = compile_public_variable_mode;
   Default_Define_Function = define_public_function;
   Lang_Defining_Function = 0;
   Function_Args_Number = 0;
   Local_Variable_Number = 0;
   Locals_NameSpace = NULL;
   Current_Function = NULL;
   Current_Function_Header = NULL;

   This_Static_NameSpace = NULL;       /* allocated by caller-- here for completeness */
   This_Private_NameSpace = NULL;       /* allocated by caller-- here for completeness */
   return 0;
}

/* Error handling */
static int unset_interrupt_state (VOID_STAR maskp)
{
   Handle_Interrupt &= ~(*(int *)maskp);
   return 0;
}

static int set_interrupt_state (VOID_STAR maskp)
{
   Handle_Interrupt |= *(int *)maskp;
   return 0;
}

static void interpreter_error_hook (int set)
{
   int mask = INTERRUPT_ERROR;
   if (set)
     (void) _pSLsig_block_and_call (set_interrupt_state, (VOID_STAR) &mask);
   else
     (void) _pSLsig_block_and_call (unset_interrupt_state, (VOID_STAR) &mask);
}

#if SLANG_HAS_SIGNALS
void _pSLang_signal_interrupt (void)
{
   int mask = INTERRUPT_SIGNAL;
   (void) _pSLsig_block_and_call (set_interrupt_state, (VOID_STAR)&mask);
}

#define CHECK_SIGNALS_NOT_REENTRANT 0
static int check_signals (void)
{
#if CHECK_SIGNALS_NOT_REENTRANT
   static volatile int inprogress = 0;
#endif
   int nargs = SLang_Num_Function_Args;
   int nnargs = Next_Function_Num_Args;
   int bc, r, br;
   int status;

   if (0 == (Handle_Interrupt & INTERRUPT_SIGNAL))
     return 0;
#if CHECK_SIGNALS_NOT_REENTRANT
   if (inprogress)
     return 0;
   inprogress = 1;
#endif
   bc = Lang_Break_Condition; r = Lang_Return; br = Lang_Break;
   status = 0;
   /* The race condition that may be here can be ignored as long
    * as the Handle_Interrupt variable is modified before the
    * call to _pSLinterp_handle_signals.
    */
   Handle_Interrupt &= ~INTERRUPT_SIGNAL;
   if (-1 == _pSLsig_handle_signals ())
     status = -1;

   SLang_Num_Function_Args = nargs;
   Next_Function_Num_Args = nnargs;
   Lang_Break = br; Lang_Return = r; Lang_Break_Condition = bc;
#if CHECK_SIGNALS_NOT_REENTRANT
   inprogress = 0;
#endif
   return status;
}

int _pSLang_check_signals_hook (VOID_STAR unused)
{
   (void) unused;
   (void) check_signals ();
   if (_pSLang_Error)
     return -1;
   return 0;
}

#endif				       /* SLANG_HAS_SIGNALS */

static void free_stacks (void)
{
   /* SLfree can grok NULLs */
   SLfree ((char *)Num_Args_Stack); Num_Args_Stack = NULL;
   SLfree ((char *)Run_Stack); Run_Stack = NULL;
   SLfree ((char *)Num_Args_Stack); Num_Args_Stack = NULL;
   SLfree ((char *)Frame_Pointer_Stack); Frame_Pointer_Stack = NULL;
#if SLANG_HAS_QUALIFIERS
   SLfree ((char *)Function_Qualifiers_Stack); Function_Qualifiers_Stack = NULL;
#endif
}

static void delete_interpreter (void)
{
   if (Run_Stack != NULL)
     {
	/* Allow any object destructors to run */
	while (Stack_Pointer != Run_Stack)
	  {
	     SLdo_pop ();
	  }
     }

   SLang_restart (0);

   while ((This_Compile_Block_Type != COMPILE_BLOCK_TYPE_NONE)
	  && (0 == _pSLcompile_pop_context ()))
     ;

   _pSLns_delete_namespaces ();
   free_stacks ();
}

static int init_interpreter (void)
{
   SLang_NameSpace_Type *ns;

   if (Global_NameSpace != NULL)
     return 0;

   free_stacks ();

   _pSLinterpreter_Error_Hook = interpreter_error_hook;

   if (NULL == (ns = _pSLns_new_namespace (NULL, SLGLOBALS_HASH_TABLE_SIZE)))
     return -1;
   if (-1 == _pSLns_set_namespace_name (ns, "Global"))
     return -1;
   Global_NameSpace = ns;

   Run_Stack = (SLang_Object_Type *) SLcalloc (SLANG_MAX_STACK_LEN,
						  sizeof (SLang_Object_Type));
   if (Run_Stack == NULL)
     goto return_error;

   Stack_Pointer = Run_Stack;
   Stack_Pointer_Max = Run_Stack + SLANG_MAX_STACK_LEN;

   Num_Args_Stack = (int *) _SLcalloc (SLANG_MAX_RECURSIVE_DEPTH, sizeof(int));
   if (Num_Args_Stack == NULL)
     goto return_error;

   Recursion_Depth = 0;
   Frame_Pointer_Stack = (unsigned int *) _SLcalloc (SLANG_MAX_RECURSIVE_DEPTH, sizeof(unsigned int));
   if (Frame_Pointer_Stack == NULL)
     goto return_error;
   Frame_Pointer_Depth = 0;
   Frame_Pointer = Run_Stack;

#if SLANG_HAS_QUALIFIERS
   Function_Qualifiers_Stack = (SLang_Struct_Type **) SLcalloc (SLANG_MAX_RECURSIVE_DEPTH, sizeof (SLang_Struct_Type *));
   if (Function_Qualifiers_Stack == NULL)
     goto return_error;
#endif

   Function_Stack = (Function_Stack_Type *) _SLcalloc (SLANG_MAX_RECURSIVE_DEPTH, sizeof (Function_Stack_Type));
   if (Function_Stack == NULL)
     goto return_error;

   Function_Stack_Ptr = Function_Stack;
   /* Function_Stack_Ptr_Max = Function_Stack_Ptr + SLANG_MAX_RECURSIVE_DEPTH; */

   (void) setup_default_compile_linkage (1);
   if (-1 == SLang_add_cleanup_function (delete_interpreter))
     goto return_error;

   return 0;

return_error:
   free_stacks ();
   return -1;
}

static int add_generic_table (SLang_NameSpace_Type *ns,
			      SLang_Name_Type *table, SLFUTURE_CONST char *pp_name,
			      unsigned int entry_len)
{
   SLang_Name_Type *t, **ns_table;
   SLFUTURE_CONST char *name;
   unsigned int table_size;

   if (-1 == init_interpreter ())
     return -1;

   if (ns == NULL)
     ns = Global_NameSpace;

   if ((pp_name != NULL)
       && (-1 == SLdefine_for_ifdef (pp_name)))
     return -1;

   ns_table = ns->table;
   table_size = ns->table_size;

   t = table;
   while (NULL != (name = t->name))
     {
	unsigned long hash;

	/* Backward compatibility: '.' WAS used as hash marker */
	if (*name == '.')
	  {
	     name++;
	     t->name = name;
	  }

	if (-1 == _pSLcheck_identifier_syntax (name))
	  return -1;

	if (NULL == (name = SLang_create_slstring (name)))
	  return -1;

	t->name = name;

	hash = _pSLcompute_string_hash (name);
	hash = hash % table_size;

	/* First time.  Make sure this has not already been added */
	if (t == table)
	  {
	     SLang_Name_Type *tt = ns_table[(unsigned int) hash];
	     while (tt != NULL)
	       {
		  if (tt == t)
		    {
		       _pSLang_verror (SL_APPLICATION_ERROR,
				     "An intrinsic symbol table may not be added twice. [%s]",
				     pp_name == NULL ? "" : pp_name);
		       return -1;
		    }
		  tt = tt->next;
	       }
	  }

	t->next = ns_table [(unsigned int) hash];
	ns_table [(unsigned int) hash] = t;

	t = (SLang_Name_Type *) ((char *)t + entry_len);
     }

   return 0;
}

/* FIXME: I should add code to make sure that the objects in, e.g., an iconstant table
 * have sizeof(int).
 */
int SLadd_intrin_fun_table (SLang_Intrin_Fun_Type *tbl, SLFUTURE_CONST char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (SLang_Intrin_Fun_Type));
}

int SLadd_intrin_var_table (SLang_Intrin_Var_Type *tbl, SLFUTURE_CONST char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (SLang_Intrin_Var_Type));
}

int SLadd_app_unary_table (SLang_App_Unary_Type *tbl, SLFUTURE_CONST char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (SLang_App_Unary_Type));
}

int _pSLadd_arith_unary_table (SLang_Arith_Unary_Type *tbl, SLFUTURE_CONST char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (SLang_Arith_Unary_Type));
}

int _pSLadd_arith_binary_table (SLang_Arith_Binary_Type *tbl, SLFUTURE_CONST char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (SLang_Arith_Binary_Type));
}

int SLadd_math_unary_table (SLang_Math_Unary_Type *tbl, SLFUTURE_CONST char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (SLang_Math_Unary_Type));
}

int SLadd_iconstant_table (SLang_IConstant_Type *tbl, SLFUTURE_CONST char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (SLang_IConstant_Type));
}

int SLadd_lconstant_table (SLang_LConstant_Type *tbl, SLFUTURE_CONST char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (SLang_LConstant_Type));
}

#if SLANG_HAS_FLOAT
int SLadd_dconstant_table (SLang_DConstant_Type *tbl, SLFUTURE_CONST char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (SLang_DConstant_Type));
}
int SLadd_fconstant_table (SLang_FConstant_Type *tbl, SLFUTURE_CONST char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (SLang_FConstant_Type));
}
#endif
#ifdef HAVE_LONG_LONG
int SLadd_llconstant_table (SLang_LLConstant_Type *tbl, SLFUTURE_CONST char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (SLang_LLConstant_Type));
}
int _pSLadd_llconstant_table (_pSLang_LLConstant_Type *tbl, SLFUTURE_CONST char *pp)
{
   return add_generic_table (NULL, (SLang_Name_Type *) tbl, pp, sizeof (_pSLang_LLConstant_Type));
}
#endif

/* ----------- */
int SLns_add_intrin_fun_table (SLang_NameSpace_Type *ns, SLang_Intrin_Fun_Type *tbl, SLFUTURE_CONST char *pp)
{
   if ((ns == NULL) || (ns == (Global_NameSpace)))
     return add_generic_table (ns, (SLang_Name_Type *) tbl, pp, sizeof (SLang_Intrin_Fun_Type));

   if ((pp != NULL)
       && (-1 == SLdefine_for_ifdef (pp)))
     return -1;

   while (tbl->name != NULL)
     {
	if (-1 == add_intrinsic_function (ns, tbl->name, tbl->i_fun,
					  tbl->return_type, tbl->num_args,
					  tbl->arg_types))
	  return -1;
	tbl++;
     }
   return 0;
}

int SLns_add_intrin_var_table (SLang_NameSpace_Type *ns, SLang_Intrin_Var_Type *tbl, SLFUTURE_CONST char *pp)
{
   if ((ns == NULL) || (ns == Global_NameSpace))
     return add_generic_table (ns, (SLang_Name_Type *) tbl, pp, sizeof (SLang_Intrin_Var_Type));

   if ((pp != NULL)
       && (-1 == SLdefine_for_ifdef (pp)))
     return -1;

   while (tbl->name != NULL)
     {
	if (-1 == SLns_add_intrinsic_variable (ns, tbl->name, tbl->addr, tbl->type, tbl->name_type == SLANG_RVARIABLE))
	  return -1;

	tbl++;
     }
   return 0;
}

int SLns_add_app_unary_table (SLang_NameSpace_Type *ns, SLang_App_Unary_Type *tbl, SLFUTURE_CONST char *pp)
{
   if ((ns == NULL) || (ns == (Global_NameSpace)))
     return add_generic_table (ns, (SLang_Name_Type *) tbl, pp, sizeof (SLang_App_Unary_Type));

   if ((pp != NULL)
       && (-1 == SLdefine_for_ifdef (pp)))
     return -1;

   while (tbl->name != NULL)
     {
	SLang_App_Unary_Type *a;

	a = (SLang_App_Unary_Type *)add_xxx_helper (ns, tbl->name, SLANG_APP_UNARY, sizeof(SLang_App_Unary_Type));
	if (a == NULL)
	  return -1;
	a->unary_op = tbl->unary_op;
	tbl++;
     }
   return 0;
}

int SLns_add_math_unary_table (SLang_NameSpace_Type *ns, SLang_Math_Unary_Type *tbl, SLFUTURE_CONST char *pp)
{
   if ((ns == NULL) || (ns == (Global_NameSpace)))
     return add_generic_table (ns, (SLang_Name_Type *) tbl, pp, sizeof (SLang_Math_Unary_Type));

   if ((pp != NULL)
       && (-1 == SLdefine_for_ifdef (pp)))
     return -1;

   while (tbl->name != NULL)
     {
	SLang_Math_Unary_Type *a;

	a = (SLang_Math_Unary_Type *)add_xxx_helper (ns, tbl->name, SLANG_MATH_UNARY, sizeof(SLang_Math_Unary_Type));
	if (a == NULL)
	  return -1;
	a->unary_op = tbl->unary_op;
	tbl++;
     }
   return 0;
}

int SLns_add_hconstant_table (SLang_NameSpace_Type *ns, SLang_HConstant_Type *tbl, SLFUTURE_CONST char *pp)
{
   if ((ns == NULL) || (ns == (Global_NameSpace)))
     return add_generic_table (ns, (SLang_Name_Type *) tbl, pp, sizeof (SLang_HConstant_Type));

   if ((pp != NULL)
       && (-1 == SLdefine_for_ifdef (pp)))
     return -1;

   while (tbl->name != NULL)
     {
	if (-1 == SLns_add_hconstant (ns, tbl->name, tbl->data_type, tbl->value))
	  return -1;
	tbl++;
     }
   return 0;
}

int SLns_add_iconstant_table (SLang_NameSpace_Type *ns, SLang_IConstant_Type *tbl, SLFUTURE_CONST char *pp)
{
   if ((ns == NULL) || (ns == (Global_NameSpace)))
     return add_generic_table (ns, (SLang_Name_Type *) tbl, pp, sizeof (SLang_IConstant_Type));

   if ((pp != NULL)
       && (-1 == SLdefine_for_ifdef (pp)))
     return -1;

   while (tbl->name != NULL)
     {
	if (-1 == SLns_add_iconstant (ns, tbl->name, tbl->data_type, tbl->value))
	  return -1;
	tbl++;
     }
   return 0;
}

int SLns_add_lconstant_table (SLang_NameSpace_Type *ns, SLang_LConstant_Type *tbl, SLFUTURE_CONST char *pp)
{
   if ((ns == NULL) || (ns == (Global_NameSpace)))
     return add_generic_table (ns, (SLang_Name_Type *) tbl, pp, sizeof (SLang_LConstant_Type));

   if ((pp != NULL)
       && (-1 == SLdefine_for_ifdef (pp)))
     return -1;

   while (tbl->name != NULL)
     {
	if (-1 == SLns_add_lconstant (ns, tbl->name, tbl->data_type, tbl->value))
	  return -1;
	tbl++;
     }
   return 0;
}

#ifdef HAVE_LONG_LONG
int _pSLns_add_llconstant_table (SLang_NameSpace_Type *ns, _pSLang_LLConstant_Type *tbl, SLFUTURE_CONST char *pp)
{
   if ((ns == NULL) || (ns == (Global_NameSpace)))
     return add_generic_table (ns, (SLang_Name_Type *) tbl, pp, sizeof (_pSLang_LLConstant_Type));

   if ((pp != NULL)
       && (-1 == SLdefine_for_ifdef (pp)))
     return -1;

   while (tbl->name != NULL)
     {
	if (-1 == _pSLns_add_llconstant (ns, tbl->name, tbl->data_type, tbl->value))
	  return -1;
	tbl++;
     }
   return 0;
}
#endif

#if SLANG_HAS_FLOAT
int SLns_add_dconstant_table (SLang_NameSpace_Type *ns, SLang_DConstant_Type *tbl, SLFUTURE_CONST char *pp)
{
   if ((ns == NULL) || (ns == (Global_NameSpace)))
     return add_generic_table (ns, (SLang_Name_Type *) tbl, pp, sizeof (SLang_DConstant_Type));

   if ((pp != NULL)
       && (-1 == SLdefine_for_ifdef (pp)))
     return -1;

   while (tbl->name != NULL)
     {
	if (-1 == SLns_add_dconstant (ns, tbl->name, tbl->d))
	  return -1;
	tbl++;
     }
   return 0;
}
#endif

static int setup_default_compile_linkage (int is_public)
{
   if (is_public)
     {
	Default_Define_Function = define_public_function;
	Default_Variable_Mode = compile_public_variable_mode;
     }
   else
     {
	Default_Define_Function = define_static_function;
	Default_Variable_Mode = compile_static_variable_mode;
     }

   return 0;
}

/* what is a bitmapped value:
 * 1 intrin fun
 * 2 user fun
 * 4 intrin var
 * 8 user defined var
 */
SLang_Array_Type *_pSLang_apropos (SLFUTURE_CONST char *namespace_name, SLFUTURE_CONST char *pat, unsigned int what)
{
   SLang_NameSpace_Type *ns;

   if (namespace_name == NULL)
     namespace_name = "Global";

   if (*namespace_name == 0)
     ns = This_Static_NameSpace;
   else ns = _pSLns_find_namespace (namespace_name);

   return _pSLnspace_apropos (ns, pat, what);
}

/* An anonymous namespace in one in which the private and static objects share
 * the same namespace.  This function causes a new namespace to be created or
 * an existing one used, and where the static objects will be placed.  The
 * private objects will continue using the private namespace.
 */
static int implements_ns (SLFUTURE_CONST char *namespace_name)
{
   SLang_NameSpace_Type *ns;
   SLFUTURE_CONST char *name;

   if ((This_Private_NameSpace == NULL) || (This_Static_NameSpace == NULL))
     {
	/* This error should never happen */
	_pSLang_verror (SL_INTERNAL_ERROR, "No namespace available");
	return -1;
     }
   name = This_Private_NameSpace->name;

   if (NULL != (ns = _pSLns_find_namespace (namespace_name)))
     {
	/* If it exists but is associated with the private namespace, the it is ok */
	if (ns->name != name)
	  {
	     _pSLang_verror (SL_Namespace_Error, "Namespace %s already exists", namespace_name);
	     return -1;
	  }
     }
   return setup_compile_namespaces (name, namespace_name);
}

void _pSLang_implements_intrinsic (SLFUTURE_CONST char *name)
{
   (void) implements_ns (name);
}

void _pSLang_use_namespace_intrinsic (char *name)
{
   SLang_NameSpace_Type *ns;

   if (NULL == (ns = _pSLns_find_namespace (name)))
     {
	_pSLang_verror (SL_Namespace_Error, "Namespace %s does not exist", name);
	return;
     }
   This_Static_NameSpace = ns;
   (void) setup_default_compile_linkage (ns == Global_NameSpace);
}

static SLang_NameSpace_Type *_pSLang_cur_namespace (void)
{
   if (This_Static_NameSpace == NULL)
     return Global_NameSpace;

   if (This_Static_NameSpace->namespace_name == NULL)
     return NULL;

   return This_Static_NameSpace;
}

SLFUTURE_CONST char *_pSLang_cur_namespace_intrinsic (void)
{
   SLang_NameSpace_Type *ns = _pSLang_cur_namespace ();
   if (ns == NULL)
     return "";
   return ns->namespace_name;
}

SLCONST char *_pSLang_current_function_name (void)
{
   if (Current_Function == NULL)
     return NULL;

   return Current_Function->name;
}

SLang_Object_Type *_pSLang_get_run_stack_pointer (void)
{
   return Stack_Pointer;
}

SLang_Object_Type *_pSLang_get_run_stack_base (void)
{
   return Run_Stack;
}

int _pSLang_dump_stack (void) /*{{{*/
{
   char buf[32];
   unsigned int n;

   n = (unsigned int) (Stack_Pointer - Run_Stack);
   while (n)
     {
	n--;
	sprintf (buf, "(%u)", n);
	_pSLdump_objects (buf, Run_Stack + n, 1, 1);
     }
   return 0;
}

/*}}}*/

int _pSLang_is_arith_type (SLtype t)
{
   return (int) IS_ARITH_TYPE(t);
}
void _pSLang_set_arith_type (SLtype t, unsigned char v)
{
   if (t < 256)
     Is_Arith_Type_Array[t] = v;
}

#if SLANG_HAS_DEBUGGER_SUPPORT
void _pSLang_use_frame_namespace (int depth)
{
   Function_Stack_Type s;
   if (-1 == get_function_stack_info (depth, &s))
     return;

   This_Static_NameSpace = s.static_ns;
   This_Private_NameSpace = s.private_ns;
   (void) setup_default_compile_linkage (s.static_ns == Global_NameSpace);
}
#endif
