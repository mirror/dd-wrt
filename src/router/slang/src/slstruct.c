/* Structure type implementation */
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

/* #define SL_APP_WANTS_FOREACH */
#include "slang.h"
#include "_slang.h"

static void free_fields (_pSLstruct_Field_Type *fields, unsigned int n)
{
   _pSLstruct_Field_Type *field, *field_max;

   if (fields == NULL)
     return;

   field = fields;
   field_max = field + n;

   while (field < field_max)
     {
	SLang_free_object (&field->obj);
	SLang_free_slstring ((char *) field->name);   /* could be NULL */
	field++;
     }
   SLfree ((char *) fields);
}

static void free_struct (_pSLang_Struct_Type *s)
{
   if (s == NULL) return;

   if (s->num_refs > 1)
     {
	s->num_refs -= 1;
	return;
     }

   if (s->destroy_method != NULL)
     {
	if ((0 == SLang_start_arg_list ())
	    && (0 == SLang_push_struct (s))
	    && (0 == SLang_end_arg_list ()))
	  (void) SLexecute_function (s->destroy_method);

	if (s->num_refs > 1)
	  {
	     s->num_refs -= 1;
	     return;
	  }

	SLang_free_function (s->destroy_method);
     }

   free_fields (s->fields, s->nfields);
   SLfree ((char *) s);
}

void SLang_free_struct (_pSLang_Struct_Type *s)
{
   free_struct (s);
}

static _pSLang_Struct_Type *allocate_struct (unsigned int nfields)
{
   _pSLang_Struct_Type *s;
   _pSLstruct_Field_Type *f;
   unsigned int i, size;

   s = (_pSLang_Struct_Type *) SLmalloc (sizeof (_pSLang_Struct_Type));
   if (s == NULL) return NULL;

   SLMEMSET((char *) s, 0, sizeof (_pSLang_Struct_Type));

   size = nfields * sizeof(_pSLstruct_Field_Type);
   if (NULL == (f = (_pSLstruct_Field_Type *) _SLcalloc (nfields, size)))
     {
	SLfree ((char *) s);
	return NULL;
     }
   SLMEMSET ((char *) f, 0, size);
   s->nfields = nfields;
   s->fields = f;

   /* By default, all structs will be created with elements set to NULL.  I
    * do not know whether or not it is better to use SLANG_UNDEFINED_TYPE.
    */
   for (i = 0; i < nfields; i++)
     f[i].obj.o_data_type = SLANG_NULL_TYPE;

   return s;
}

static int push_struct_of_type (SLtype type, _pSLang_Struct_Type *s)
{
   SLang_Object_Type obj;

   obj.o_data_type = type;
   obj.v.struct_val = s;
   s->num_refs += 1;

   if (0 == SLang_push (&obj))
     return 0;

   s->num_refs -= 1;
   return -1;
}

int SLang_push_struct (_pSLang_Struct_Type *s)
{
   if (s == NULL)
     return SLang_push_null ();

   return push_struct_of_type (SLANG_STRUCT_TYPE, s);
}

int SLang_pop_struct (_pSLang_Struct_Type **sp)
{
   SLang_Object_Type obj;
   SLang_Class_Type *cl;
   SLtype type;

   if (0 != SLang_pop (&obj))
     return -1;

   type = obj.o_data_type;
   if (type != SLANG_STRUCT_TYPE)
     {
	cl = _pSLclass_get_class (type);
	if (cl->is_struct == 0)
	  {
	     *sp = NULL;
	     SLang_free_object (&obj);
	     _pSLang_verror (SL_TYPE_MISMATCH,
			   "Expecting struct type object.  Found %s",
			   cl->cl_name);
	     return -1;
	  }
     }

   *sp = obj.v.struct_val;
   return 0;
}

static void struct_destroy (SLtype type, VOID_STAR vs)
{
   (void) type;
   SLang_free_struct (*(_pSLang_Struct_Type **) vs);
}

static int struct_push (SLtype type, VOID_STAR ptr)
{
   return push_struct_of_type (type, *(_pSLang_Struct_Type **) ptr);
}

static _pSLstruct_Field_Type *find_field_in_fields (_pSLstruct_Field_Type *fields, unsigned int n, SLCONST char *name)
{
   _pSLstruct_Field_Type *f, *fmax;

   f = fields;
   fmax = fields + n;

   while (f < fmax)
     {
	/* Since both these are slstrings, only compare pointer */
	if (name == f->name)
	  return f;

	f++;
     }

   return NULL;
}

static _pSLstruct_Field_Type *find_field (_pSLang_Struct_Type *s, SLCONST char *name)
{
   return find_field_in_fields (s->fields, s->nfields, name);
}

static _pSLstruct_Field_Type *find_field_strcmp (_pSLang_Struct_Type *s, SLCONST char *name)
{
   _pSLstruct_Field_Type *f, *fmax;

   f = s->fields;
   fmax = f + s->nfields;

   while (f < fmax)
     {
	if ((name == f->name)
	    || (0 == strcmp (name, f->name)))
	  return f;

	f++;
     }

   return NULL;
}

/* This function is used by the qualifier-code.  Here "name" cannot be
 * assumed to be an slstring.
 */
SLang_Object_Type *_pSLstruct_get_field_value (SLang_Struct_Type *s, SLCONST char *name)
{
   _pSLstruct_Field_Type *f = find_field_strcmp (s, name);

   if (f == NULL)
     return NULL;

   return &f->obj;
}

static _pSLstruct_Field_Type *pop_field (_pSLang_Struct_Type *s, SLCONST char *name,
					_pSLstruct_Field_Type *(*find)(_pSLang_Struct_Type *, SLCONST char *))
{
   _pSLstruct_Field_Type *f;

   f = (*find) (s, name);
   if (f == NULL)
     _pSLang_verror (SL_INVALID_PARM, "struct has no field named %s", name);
   return f;
}

static _pSLang_Struct_Type *
  create_struct (unsigned int nfields,
		 SLFUTURE_CONST char **field_names,
		 SLtype *field_types,
		 VOID_STAR *field_values)
{
   _pSLang_Struct_Type *s;
   _pSLstruct_Field_Type *f;
   unsigned int i;

   if (NULL == (s = allocate_struct (nfields)))
     return NULL;

   f = s->fields;
   for (i = 0; i < nfields; i++)
     {
	SLtype type;
	SLang_Class_Type *cl;
	VOID_STAR value;
	SLFUTURE_CONST char *name = field_names [i];

	if (name == NULL)
	  {
	     _pSLang_verror (SL_APPLICATION_ERROR, "A struct field name cannot be NULL");
	     goto return_error;
	  }

	if (-1 == _pSLcheck_identifier_syntax (name))
	  goto return_error;

	if (NULL == (f->name = SLang_create_slstring (name)))
	  goto return_error;

	if ((field_values == NULL)
	    || (NULL == (value = field_values [i])))
	  {
	     f++;
	     continue;
	  }

	type = field_types[i];
	cl = _pSLclass_get_class (type);

	if ((-1 == (cl->cl_apush (type, value)))
	    || (-1 == SLang_pop (&f->obj)))
	  goto return_error;

	f++;
     }

   return s;

   return_error:
   SLang_free_struct (s);
   return NULL;
}

int SLstruct_create_struct (unsigned int nfields,
			    SLFUTURE_CONST char **field_names,
			    SLtype *field_types,
			    VOID_STAR *field_values)
{
   _pSLang_Struct_Type *s;

   if (NULL == (s = create_struct (nfields, field_names, field_types, field_values)))
     return -1;

   if (0 == SLang_push_struct (s))
     return 0;

   SLang_free_struct (s);
   return -1;
}

/* Interpreter interface */

static _pSLang_Struct_Type *struct_from_struct_fields (int nfields)
{
   _pSLang_Struct_Type *s;
   _pSLstruct_Field_Type *f;
   int max_fields;

   if (nfields <= 0)
     {
	_pSLang_verror (SL_INVALID_PARM, "Number of struct fields must be > 0");
	return NULL;
     }

   if (NULL == (s = allocate_struct (nfields)))
     return NULL;

   f = s->fields;
   max_fields = nfields;
   while (nfields)
     {
	char *name;
	int i;

	nfields--;
	if (-1 == SLang_pop_slstring (&name))
	  {
	     SLang_free_struct (s);
	     return NULL;
	  }

	f[nfields].name = name;

	for (i = nfields + 1; i < max_fields; i++)
	  {
	     if (name != f[i].name)
	       continue;

	     _pSLang_verror (SL_DuplicateDefinition_Error,
			   "Field %s used more than once in the struct",
			   name);
	     SLang_free_struct (s);
	     return NULL;
	  }
     }

   return s;
}

int _pSLstruct_define_struct (void)
{
   _pSLang_Struct_Type *s;
   int nfields;

   if (-1 == SLang_pop_integer (&nfields))
     return -1;

   if (NULL == (s = struct_from_struct_fields (nfields)))
     return -1;

   if (-1 == SLang_push_struct (s))
     {
	SLang_free_struct (s);
	return -1;
     }
   return 0;
}

static int pop_to_struct_field (_pSLang_Struct_Type *s, SLCONST char *name)
{
   _pSLstruct_Field_Type *f;
   SLang_Object_Type obj;

   if ((NULL == (f = pop_field (s, name, find_field)))
       || (-1 == SLang_pop (&obj)))
     return -1;

   SLang_free_object (&f->obj);
   f->obj = obj;

   return 0;
}

/* Take the fields of b and use those to replace the field atname of a */
static int merge_struct_fields (SLCONST char *atname, _pSLang_Struct_Type *a, _pSLang_Struct_Type *b)
{
   unsigned int i, j;
   char **new_names;
   _pSLstruct_Field_Type *f, *fmax, *new_fields;
   _pSLstruct_Field_Type *atfield;
   unsigned int num_before, num_insert, num_after, new_num;

   atfield = find_field (a, atname);
   if (atfield == NULL)
     {
	SLang_verror (SL_Internal_Error, "Unable to find struct field %s", atname);
	return -1;
     }
   num_before = atfield - a->fields;
   num_after = a->nfields - (1 + num_before);
   num_insert = 0;

   if (b != NULL)
     {
	unsigned int nb = b->nfields;
	new_names = (char **)_SLcalloc (nb, sizeof (char *));
	if (new_names == NULL)
	  return -1;

	f = b->fields;
	fmax = f + nb;
	while (f < fmax)
	  {
	     if (NULL == find_field (a, f->name))
	       new_names[num_insert++] = f->name;
	     f++;
	  }
     }
   else new_names = NULL;

   new_num = num_before + num_insert + num_after;
   new_fields = (_pSLstruct_Field_Type *)SLcalloc (new_num, sizeof(_pSLstruct_Field_Type));
   if (new_fields == NULL)
     {
	SLfree ((char *) new_names);
	return -1;
     }

   f = a->fields;
   j = 0;
   for (i = 0; i < num_before; i++)
     {
	new_fields[j++] = f[i];
	memset ((char *)&f[i], 0, sizeof(_pSLstruct_Field_Type));
     }

   for (i = 0; i < num_insert; i++)
     {
	if (NULL == (new_fields[j].name = SLang_create_slstring (new_names[i])))
	  goto return_error;
	j++;
     }

   f = a->fields + num_before + 1;
   for (i = 0; i < num_after; i++)
     {
	new_fields[j++] = f[i];
	memset ((char *)&f[i], 0, sizeof(_pSLstruct_Field_Type));
     }

   if (b != NULL)
     {
	f = b->fields;
	fmax = f + b->nfields;
	while (f < fmax)
	  {
	     _pSLstruct_Field_Type *fa;

	     /* Cannot fail by construction */
	     fa = find_field_in_fields (new_fields, new_num, f->name);

	     if ((-1 == _pSLpush_slang_obj (&f->obj))
		 || (-1 == SLang_pop (&fa->obj)))
	       goto return_error;

	     f++;
	  }
     }

   SLfree ((char *) new_names);
   free_fields (a->fields, a->nfields);
   a->fields = new_fields;
   a->nfields = new_num;
   return 0;

return_error:

   free_fields (new_fields, new_num);
   SLfree ((char *) new_names);
   return -1;
}

static int pop_struct_into_field (_pSLang_Struct_Type *s, SLCONST char *name)
{
   _pSLang_Struct_Type *t;
   int status;

   if (SLang_peek_at_stack () == SLANG_NULL_TYPE)
     {
	(void) SLdo_pop_n(1);
	return merge_struct_fields (name, s, NULL);
     }

   if (-1 == SLang_pop_struct (&t))
     {
	SLang_verror (SL_TypeMismatch_Error, "Field %s should represent a struct", name);
	return -1;
     }

   status = merge_struct_fields (name, s, t);
   free_struct (t);
   return status;
}

/* This function is used for structure definitions with embedded assignments */
int _pSLstruct_define_struct2 (void)
{
   _pSLang_Struct_Type *s;
   int nfields;
   int nassigns;

   if (-1 == SLang_pop_integer (&nassigns))
     return -1;

   if (-1 == SLang_pop_integer (&nfields))
     return -1;

   if (NULL == (s = struct_from_struct_fields (nfields)))
     return -1;

   /* On stack: nameN, valN, ...., name1, val1 .... */
   if (nassigns
       && (-1 == SLreverse_stack (2*nassigns)))
     goto return_error;
   while (nassigns > 0)
     {
	char *name;
	int status;

	/* Stack: val1, name1, val2, name2, ...
	 */
	if ((-1 == SLreverse_stack (2))
	    || (-1 == SLang_pop_slstring (&name)))
	  goto return_error;

	if (*name == '@')
	  status = pop_struct_into_field (s, name);
	else
	  status = pop_to_struct_field (s, name);

	SLang_free_slstring (name);
	if (status == -1)
	  goto return_error;

	nassigns--;
     }

   if (0 == SLang_push_struct (s))
     return 0;

return_error:

   SLang_free_struct (s);
   return -1;
}

static int init_struct_with_user_methods (SLtype, _pSLang_Struct_Type *);
/* Simply make a struct that contains the same fields as struct s.  Do not
 * duplicate the field values.
 */
static _pSLang_Struct_Type *make_struct_shell (_pSLang_Struct_Type *s, SLtype type)
{
   _pSLang_Struct_Type *new_s;
   _pSLstruct_Field_Type *new_f, *old_f;
   unsigned int i, nfields;

   nfields = s->nfields;
   if (NULL == (new_s = allocate_struct (nfields)))
     return NULL;

   new_f = new_s->fields;
   old_f = s->fields;

   for (i = 0; i < nfields; i++)
     {
	if (NULL == (new_f[i].name = SLang_create_slstring (old_f[i].name)))
	  {
	     SLang_free_struct (new_s);
	     return NULL;
	  }
     }

   if (type != SLANG_STRUCT_TYPE)
     (void) init_struct_with_user_methods (type, new_s);
   return new_s;
}

static int struct_init_array_object (SLtype type, VOID_STAR addr)
{
   SLang_Class_Type *cl;
   _pSLang_Struct_Type *s;

   cl = _pSLclass_get_class (type);
   if (NULL == (s = make_struct_shell (cl->cl_struct_def, type)))
     return -1;

   s->num_refs = 1;
   *(_pSLang_Struct_Type **) addr = s;
   return 0;
}

static int
typedefed_struct_datatype_deref (SLtype type)
{
   SLang_Class_Type *cl;
   _pSLang_Struct_Type *s;

   cl = _pSLclass_get_class (type);
   if (NULL == (s = make_struct_shell (cl->cl_struct_def, type)))
     return -1;

   if (-1 == push_struct_of_type (type, s))
     {
	SLang_free_struct (s);
	return -1;
     }

   return 0;
}

static _pSLang_Struct_Type *duplicate_struct (_pSLang_Struct_Type *s, SLtype type)
{
   _pSLang_Struct_Type *new_s;
   _pSLstruct_Field_Type *new_f, *f, *fmax;

   new_s = make_struct_shell (s, type);

   if (new_s == NULL)
     return NULL;

   f = s->fields;
   fmax = f + s->nfields;
   new_f = new_s->fields;

   while (f < fmax)
     {
	SLang_Object_Type *obj;

	obj = &f->obj;
	if (obj->o_data_type != SLANG_UNDEFINED_TYPE)
	  {
	     if ((-1 == _pSLpush_slang_obj (obj))
		 || (-1 == SLang_pop (&new_f->obj)))
	       {
		  SLang_free_struct (new_s);
		  return NULL;
	       }
	  }
	new_f++;
	f++;
     }

   return new_s;
}

static int struct_dereference (SLtype type, VOID_STAR addr)
{
   _pSLang_Struct_Type *s;

   if (NULL == (s = duplicate_struct (*(_pSLang_Struct_Type **) addr, type)))
     return -1;

   if (-1 == push_struct_of_type (type, s))
     {
	SLang_free_struct (s);
	return -1;
     }

   return 0;
}

/*{{{ foreach */

struct _pSLang_Foreach_Context_Type
{
   _pSLang_Struct_Type *s;
   char *next_field_name;
};

static SLang_Foreach_Context_Type *
struct_foreach_open (SLtype type, unsigned int num)
{
   SLang_Foreach_Context_Type *c;
   _pSLang_Struct_Type *s;
   char *next_name;

   (void) type;

   if (-1 == SLang_pop_struct (&s))
     return NULL;

   switch (num)
     {
      case 0:
	next_name = SLang_create_slstring ("next");
	break;

      case 1:
	if (-1 == SLang_pop_slstring (&next_name))
	  next_name = NULL;
	break;

      default:
	next_name = NULL;
	_pSLang_verror (SL_NOT_IMPLEMENTED,
		      "'foreach (Struct_Type) using' requires single control value");
	SLdo_pop_n (num);
	break;
     }

   if (next_name == NULL)
     {
	SLang_free_struct (s);
	return NULL;
     }

   c = (SLang_Foreach_Context_Type *)SLmalloc (sizeof (SLang_Foreach_Context_Type));
   if (c == NULL)
     {
	SLang_free_struct (s);
	SLang_free_slstring (next_name);
	return NULL;
     }
   memset ((char *) c, 0, sizeof (SLang_Foreach_Context_Type));

   c->next_field_name = next_name;
   c->s = s;

   return c;
}

static void struct_foreach_close (SLtype type, SLang_Foreach_Context_Type *c)
{
   (void) type;
   if (c == NULL) return;

   SLang_free_slstring (c->next_field_name);
   if (c->s != NULL) SLang_free_struct (c->s);
   SLfree ((char *) c);
}

static int struct_foreach (SLtype type, SLang_Foreach_Context_Type *c)
{
   _pSLstruct_Field_Type *f;
   _pSLang_Struct_Type *next_s;

   (void) type;

   if (c == NULL)
     return -1;

   if (c->s == NULL)
     return 0;			       /* done */

   if (-1 == SLang_push_struct (c->s))
     return -1;

   /* Now get the next one ready for the next foreach loop */

   next_s = NULL;
   if (NULL != (f = find_field (c->s, c->next_field_name)))
     {
	SLang_Class_Type *cl;

	cl = _pSLclass_get_class (f->obj.o_data_type);
	/* Note that I cannot simply look for SLANG_STRUCT_TYPE since the
	 * user may have typedefed another struct type.  So, look at the
	 * class methods.
	 */
	if (cl->cl_foreach_open == struct_foreach_open)
	  {
	     next_s = f->obj.v.struct_val;
	     next_s->num_refs += 1;
	  }
     }

   SLang_free_struct (c->s);
   c->s = next_s;

   /* keep going */
   return 1;
}

/*}}}*/

/* Operator Overloading Functions */
static int push_struct_of_type (SLtype type, _pSLang_Struct_Type *s);

#define NUM_BINARY_OPS	(SLANG_BINARY_OP_MAX-SLANG_BINARY_OP_MIN+1)
#define NUM_UNARY_OPS	(SLANG_UNARY_OP_MAX-SLANG_UNARY_OP_MIN+1)

typedef struct
{
   SLang_Class_Type *result_any_this_cl;
   SLang_Class_Type *result_this_any_cl;
   SLang_Class_Type *result_this_this_cl;
   SLang_Name_Type *any_binary_this;
   SLang_Name_Type *this_binary_void;
   SLang_Name_Type *this_binary_this;
}
Binary_Op_Info_Type;

typedef struct
{
   SLang_Class_Type *result_cl;
   SLang_Name_Type *unary_function;
}
Unary_Op_Info_Type;

typedef struct _Typecast_Info_Type
{
   SLang_Name_Type *typecast_fun;
   SLtype totype;
   struct _Typecast_Info_Type *next;
}
Typecast_Info_Type;

typedef struct _Struct_Info_Type
{
   SLtype type;
   struct _Struct_Info_Type *next;

   int binary_registered;
   int unary_registered;
   Binary_Op_Info_Type *bi;
   Unary_Op_Info_Type *ui;
   Typecast_Info_Type *ti;

   /* Other methods */
   SLang_Name_Type *destroy_method;
   SLang_Name_Type *string_method;
   SLang_Name_Type *aget_method;
   SLang_Name_Type *aput_method;
}
Struct_Info_Type;

static Struct_Info_Type *Struct_Info_List;

static Binary_Op_Info_Type *find_binary_info (int, SLtype);
static Unary_Op_Info_Type *find_unary_info (int, SLtype);

static int allocate_struct_info (SLtype type)
{
   Struct_Info_Type *si;

   si = (Struct_Info_Type *)SLmalloc (sizeof (Struct_Info_Type));
   if (si == NULL)
     return -1;

   memset ((char *) si, 0, sizeof (Struct_Info_Type));
   si->type = type;
   si->next = Struct_Info_List;
   Struct_Info_List = si;

   return 0;
}

static Struct_Info_Type *find_struct_info (SLtype type, int do_error)
{
   Struct_Info_Type *s, *prev = NULL;

   s = Struct_Info_List;
   while (s != NULL)
     {
	Struct_Info_Type *next = s->next;
	if (s->type == type)
	  {
	     if (s != Struct_Info_List)
	       {
		  if (prev != NULL)
		    prev->next = next;
		  s->next = Struct_Info_List;
		  Struct_Info_List = s;
	       }
	     return s;
	  }
	prev = s;
	s = next;
     }
   if (do_error)
     _pSLang_verror (SL_TYPE_MISMATCH,
		   "%s is not a user-defined type", SLclass_get_datatype_name (type));
   return NULL;
}

static int struct_unary_result (int op, SLtype t, SLtype *result)
{
   Unary_Op_Info_Type *ui;

   if (NULL == (ui = find_unary_info (op, t)))
     return 0;

   if (ui->result_cl == NULL)
     return 0;

   *result = (SLtype) ui->result_cl->cl_data_type;
   return 1;
}

static int check_struct_array (SLtype t, SLang_Struct_Type **sp, unsigned int n)
{
   unsigned int i;

   for (i = 0; i < n; i++)
     {
	if (sp[i] == NULL)
	  {
	     _pSLang_verror (SL_VARIABLE_UNINITIALIZED, "%s[%u] not initialized for binary/unary operation",
			   SLclass_get_datatype_name(t), i);
	     return -1;
	  }
       }
   return 0;
}

static int struct_unary (int op, SLtype a_type, VOID_STAR ap, SLuindex_Type na,
			 VOID_STAR bp)
{
   SLang_Struct_Type **sa;
   Unary_Op_Info_Type *ui;
   unsigned int i;
   SLtype result_type;
   SLang_Name_Type *function;
   SLang_Class_Type *bcl;
   int (*apop) (SLtype, VOID_STAR);
   unsigned int binc;

   if (NULL == (ui = find_unary_info (op, a_type)))
     {
	_pSLang_verror (SL_INTERNAL_ERROR, "unary-op not supported");
	return -1;
     }

   sa = (SLang_Struct_Type **) ap;

   if (-1 == check_struct_array (a_type, sa, na))
     return -1;

   function = ui->unary_function;
   bcl = ui->result_cl;
   result_type = bcl->cl_data_type;
   apop = bcl->cl_apop;
   binc = bcl->cl_sizeof_type;

   for (i = 0; i < na; i++)
     {
	if ((-1 == SLang_start_arg_list ())
	    || (-1 == push_struct_of_type (a_type, sa[i]))
	    || (-1 == SLang_end_arg_list ())
	    || (-1 == SLexecute_function (function))
	    || (-1 == (*apop)(result_type, bp)))
	  goto return_error;

	bp = (VOID_STAR) ((char *)bp + binc);
     }

   return 1;

   return_error:
   while (i > 0)
     {
	i--;
	bp = (VOID_STAR) ((char *)bp - binc);
	bcl->cl_adestroy (result_type, bp);
	memset ((char *)bp, 0, binc);
     }
   return -1;
}

static int this_binary_any_result (int op, SLtype a, SLtype b, SLtype *result)
{
   Binary_Op_Info_Type *bi;
   SLang_Class_Type *cl;

   (void) b;
   if (NULL == (bi = find_binary_info (op, a)))
     return 0;

   if (NULL == (cl = bi->result_this_any_cl))
     return 0;

   *result = cl->cl_data_type;
   return 1;
}

static int this_binary_this_result (int op, SLtype a, SLtype b, SLtype *result)
{
   Binary_Op_Info_Type *bi;
   SLang_Class_Type *cl;

   (void) b;
   if (NULL == (bi = find_binary_info (op, a)))
     return 0;

   if (NULL == (cl = bi->result_this_this_cl))
     return 0;

   *result = cl->cl_data_type;
   return 1;
}

static int any_binary_this_result (int op, SLtype a, SLtype b, SLtype *result)
{
   Binary_Op_Info_Type *bi;
   SLang_Class_Type *cl;

   (void) a;
   if (NULL == (bi = find_binary_info (op, b)))
     return 0;

   if (NULL == (cl = bi->result_any_this_cl))
     return 0;

   *result = cl->cl_data_type;
   return 1;
}

static int do_struct_binary (SLang_Name_Type *function,
			     SLang_Class_Type *cla, VOID_STAR ap, unsigned int na,
			     SLang_Class_Type *clb, VOID_STAR bp, unsigned int nb,
			     SLang_Class_Type *clc, VOID_STAR cp)
{
   unsigned int i;
   SLtype a_type, b_type, c_type;
   int (*cpop) (SLtype, VOID_STAR);
   int (*apush) (SLtype, VOID_STAR);
   int (*bpush) (SLtype, VOID_STAR);
   unsigned int ainc, binc, cinc;
   unsigned int num;

   if (na == 1) ainc = 0; else ainc = cla->cl_sizeof_type;
   if (nb == 1) binc = 0; else binc = clb->cl_sizeof_type;
   cinc = clc->cl_sizeof_type;

   a_type = cla->cl_data_type;
   b_type = clb->cl_data_type;
   c_type = clc->cl_data_type;
   apush = cla->cl_apush;
   bpush = clb->cl_apush;
   cpop = clc->cl_apop;

   if (na > nb) num = na; else num = nb;

   for (i = 0; i < num; i++)
     {
	if ((-1 == SLang_start_arg_list ())
	    || (-1 == (*apush) (a_type, ap))
	    || (-1 == (*bpush) (b_type, bp))
	    || (-1 == SLang_end_arg_list ())
	    || (-1 == SLexecute_function (function))
	    || (-1 == (*cpop)(c_type, cp)))
	  goto return_error;

	ap = (VOID_STAR) ((char *)ap + ainc);
	bp = (VOID_STAR) ((char *)bp + binc);
	cp = (VOID_STAR) ((char *)cp + cinc);
     }

   return 1;

   return_error:
   while (i > 0)
     {
	i--;
	cp = (VOID_STAR) ((char *)cp - cinc);
	clc->cl_adestroy (c_type, cp);
	memset ((char *)cp, 0, cinc);
     }
   return -1;
}

static int this_binary_any (int op,
			     SLtype a, VOID_STAR ap, SLuindex_Type na,
			     SLtype b, VOID_STAR bp, SLuindex_Type nb,
			     VOID_STAR cp)
{
   Binary_Op_Info_Type *bi;

   if (NULL == (bi = find_binary_info (op, a)))
     {
	_pSLang_verror (SL_INTERNAL_ERROR, "binary-op not supported");
	return -1;
     }

   return do_struct_binary (bi->this_binary_void,
			    _pSLclass_get_class (a), ap, na,
			    _pSLclass_get_class (b), bp, nb,
			    bi->result_this_any_cl, cp);
}

static int any_binary_this (int op,
			     SLtype a, VOID_STAR ap, SLuindex_Type na,
			     SLtype b, VOID_STAR bp, SLuindex_Type nb,
			     VOID_STAR cp)
{
   Binary_Op_Info_Type *bi;

   if (NULL == (bi = find_binary_info (op, b)))
     {
	_pSLang_verror (SL_INTERNAL_ERROR, "binary-op not supported");
	return -1;
     }

   return do_struct_binary (bi->any_binary_this,
			    _pSLclass_get_class (a), ap, na,
			    _pSLclass_get_class (b), bp, nb,
			    bi->result_any_this_cl, cp);
}

static int this_binary_this (int op,
			     SLtype a, VOID_STAR ap, SLuindex_Type na,
			     SLtype b, VOID_STAR bp, SLuindex_Type nb,
			     VOID_STAR cp)
{
   Binary_Op_Info_Type *bi;

   if (NULL == (bi = find_binary_info (op, a)))
     {
	_pSLang_verror (SL_INTERNAL_ERROR, "binary-op not supported");
	return -1;
     }

   return do_struct_binary (bi->this_binary_this,
			    _pSLclass_get_class (a), ap, na,
			    _pSLclass_get_class (b), bp, nb,
			    bi->result_this_this_cl, cp);
}

static int register_unary_ops (Struct_Info_Type *si, SLtype t)
{
   if (si->unary_registered)
     return 0;

   if (-1 == SLclass_add_unary_op (t, struct_unary, struct_unary_result))
     return -1;

   si->unary_registered = 1;
   return 0;
}

static int register_binary_ops (Struct_Info_Type *si, SLtype t)
{
   if (si->binary_registered)
     return 0;

   if ((-1 == SLclass_add_binary_op (t, SLANG_VOID_TYPE,
				     this_binary_any, this_binary_any_result))
       || (-1 == SLclass_add_binary_op (SLANG_VOID_TYPE, t,
					any_binary_this, any_binary_this_result))
       || (-1 == SLclass_add_binary_op (t, t,
					this_binary_this, this_binary_this_result)))
     return -1;

   si->binary_registered = 1;
   return 0;
}

static Unary_Op_Info_Type *find_unary_info (int op, SLtype t)
{
   Struct_Info_Type *si;

   if (NULL == (si = find_struct_info (t, 1)))
     return NULL;

   if (-1 == register_unary_ops (si, t))
     return NULL;

   if (si->ui == NULL)
     {
	Unary_Op_Info_Type *ui;

	ui = (Unary_Op_Info_Type *)_SLcalloc (NUM_UNARY_OPS,sizeof(Unary_Op_Info_Type));
	if (NULL == (si->ui = ui))
	  return NULL;

	memset ((char *) ui, 0, NUM_UNARY_OPS*sizeof(Unary_Op_Info_Type));
     }

   op -= SLANG_UNARY_OP_MIN;
   if ((op >= NUM_UNARY_OPS) || (op < 0))
     {
	_pSLang_verror (SL_INTERNAL_ERROR,
		      "struct_unary_op: op-code out of range");
	return NULL;
     }

  return si->ui + op;
}

static Binary_Op_Info_Type *find_binary_info (int op, SLtype t)
{
   Struct_Info_Type *si;

   if (NULL == (si = find_struct_info (t, 1)))
     return NULL;

   if (-1 == register_binary_ops (si, t))
     return NULL;

   if (si->bi == NULL)
     {
	Binary_Op_Info_Type *bi;

	bi = (Binary_Op_Info_Type *)_SLcalloc (NUM_BINARY_OPS, sizeof(Binary_Op_Info_Type));
	if (NULL == (si->bi = bi))
	  return NULL;

	memset ((char *) bi, 0, NUM_BINARY_OPS*sizeof(Binary_Op_Info_Type));
     }

   op -= SLANG_BINARY_OP_MIN;
   if ((op >= NUM_BINARY_OPS) || (op < 0))
     {
	_pSLang_verror (SL_INTERNAL_ERROR,
		      "struct_binary_op: op-code out of range");
	return NULL;
     }

  return si->bi + op;
}

static int add_binary_op (char *op,
			  SLtype result_type, SLang_Name_Type *nt,
			  SLtype a_type, SLtype b_type)
{
   Binary_Op_Info_Type *bi;
   int opcode;
   SLang_Class_Type *cl;

   if (-1 == (opcode = _pSLclass_get_binary_opcode (op)))
     return -1;

   if (a_type == SLANG_ANY_TYPE)
     bi = find_binary_info (opcode, b_type);
   else
     bi = find_binary_info (opcode, a_type);

   if (bi == NULL)
     return -1;

   cl = _pSLclass_get_class (result_type);

   if (a_type != SLANG_ANY_TYPE)
     {
	if (b_type == SLANG_ANY_TYPE)
	  {
	     if (bi->this_binary_void != NULL)
	       SLang_free_function (bi->this_binary_void);
	     bi->this_binary_void = nt;
	     bi->result_this_any_cl = cl;
	     return 0;
	  }
	if (bi->this_binary_this != NULL)
	  SLang_free_function (bi->this_binary_this);
	bi->this_binary_this = nt;
	bi->result_this_this_cl = cl;
	return 0;
     }

   if (bi->any_binary_this != NULL)
     SLang_free_function (bi->any_binary_this);
   bi->any_binary_this = nt;
   bi->result_any_this_cl = cl;
   return 0;
}

static int add_unary_op (char *op,
			 SLtype result_type, SLang_Name_Type *nt, SLtype type)
{
   Unary_Op_Info_Type *ui;
   int opcode;

   if (-1 == (opcode = _pSLclass_get_unary_opcode (op)))
     return -1;

   if (NULL == (ui = find_unary_info (opcode, type)))
     return -1;

   if (ui->unary_function != NULL)
     SLang_free_function (ui->unary_function);

   ui->unary_function = nt;
   ui->result_cl = _pSLclass_get_class (result_type);
   return 0;
}

static void add_unary_op_intrin (void)
{
   SLtype type, result_type;
   SLang_Name_Type *nt;
   char *op;

   if ((-1 == SLang_pop_datatype (&type))
       || (NULL == (nt = SLang_pop_function ())))
     return;

   if ((-1 == SLang_pop_datatype (&result_type))
       || (-1 == SLang_pop_slstring (&op)))
     {
	SLang_free_function (nt);
	return;
     }

   if (-1 == add_unary_op (op, result_type, nt, type))
     SLang_free_function (nt);

   SLang_free_slstring (op);
}

static void add_binary_op_intrin (void)
{
   SLtype a_type, b_type, result_type;
   SLang_Name_Type *nt;
   char *op;

   if ((-1 == SLang_pop_datatype (&b_type))
       || (-1 == SLang_pop_datatype (&a_type))
       || (NULL == (nt = SLang_pop_function ())))
     return;

   if ((-1 == SLang_pop_datatype (&result_type))
       || (-1 == SLang_pop_slstring (&op)))
     {
	SLang_free_function (nt);
	return;
     }

   if (-1 == add_binary_op (op, result_type, nt, a_type, b_type))
     SLang_free_function (nt);

   SLang_free_slstring (op);
}

static void add_destroy_method (void)
{
   _pSLang_Struct_Type *s;
   SLang_Name_Type *f;

   if (NULL == (f = SLang_pop_function ()))
     return;

   if (SLang_peek_at_stack () == SLANG_DATATYPE_TYPE)
     {
	SLtype type;
	Struct_Info_Type *si;

	if ((-1 == SLang_pop_datatype (&type))
	    || (NULL == (si = find_struct_info (type, 1))))
	  {
	     SLang_free_function (f);
	     return;
	  }

	if (si->destroy_method != NULL)
	  SLang_free_function (si->destroy_method);
	si->destroy_method = f;
	return;
     }

   if (-1 == SLang_pop_struct (&s))
     {
	SLang_free_function (f);
	return;
     }

   if (s->destroy_method != NULL)
     SLang_free_function (s->destroy_method);
   s->destroy_method = SLang_copy_function (f);
   SLang_free_struct (s);
}

static void add_string_method (SLtype *typep, SLang_Ref_Type *ref)
{
   Struct_Info_Type *si;
   SLang_Name_Type *f;
   SLtype type = *typep;

   if (NULL == (f = SLang_get_fun_from_ref (ref)))
     return;

   if (NULL == (si = find_struct_info (type, 1)))
     return;

   if (si->string_method != NULL)
     SLang_free_function (si->string_method);

   si->string_method = SLang_copy_function (f);
}

static int aget_method (SLtype type, unsigned int num_indices)
{
   Struct_Info_Type *si;

   if (NULL == (si = find_struct_info (type, 1)))
     return -1;

   if (si->aget_method == NULL)
     {
	SLang_verror (SL_Internal_Error, "aget method called but is NULL");
	return -1;
     }

   if ((-1 == _pSLang_restart_arg_list ((int) num_indices))
       || (-1 == SLang_end_arg_list ())
       || (-1 == SLexecute_function (si->aget_method)))
     return -1;

   return 0;
}

static int aput_method (SLtype type, unsigned int num_indices)
{
   Struct_Info_Type *si;

   if (NULL == (si = find_struct_info (type, 1)))
     return -1;

   if (si->aput_method == NULL)
     {
	SLang_verror (SL_Internal_Error, "aput method called but is NULL");
	return -1;
     }

   if ((-1 == _pSLang_restart_arg_list ((int) num_indices))
       || (-1 == SLang_end_arg_list ())
       || (-1 == SLexecute_function (si->aput_method)))
     return -1;

   return 0;
}

static void add_aget_method (SLtype *typep, SLang_Ref_Type *ref)
{
   Struct_Info_Type *si;
   SLang_Name_Type *f;
   SLtype type = *typep;
   SLang_Class_Type *cl;

   if (NULL == (cl = _pSLclass_get_class (type)))
     return;

   if (NULL == (f = SLang_get_fun_from_ref (ref)))
     return;

   if (NULL == (si = find_struct_info (type, 1)))
     return;

   if (si->aget_method != NULL)
     SLang_free_function (si->aget_method);

   si->aget_method = SLang_copy_function (f);
   (void) SLclass_set_aget_function (cl, aget_method);
}

static void add_aput_method (SLtype *typep, SLang_Ref_Type *ref)
{
   Struct_Info_Type *si;
   SLang_Name_Type *f;
   SLtype type = *typep;
   SLang_Class_Type *cl;

   if (NULL == (cl = _pSLclass_get_class (type)))
     return;

   if (NULL == (f = SLang_get_fun_from_ref (ref)))
     return;

   if (NULL == (si = find_struct_info (type, 1)))
     return;

   if (si->aput_method != NULL)
     SLang_free_function (si->aput_method);

   si->aput_method = SLang_copy_function (f);
   (void) SLclass_set_aput_function (cl, aput_method);
}

static Typecast_Info_Type *find_typecast (Struct_Info_Type *si, SLtype to)
{
   Typecast_Info_Type *ti = si->ti;

   while (ti != NULL)
     {
	if (ti->totype == to)
	  return ti;
	ti = ti->next;
     }
   return ti;
}

static int typecast_method (SLtype a_type, VOID_STAR ap, SLuindex_Type na,
			    SLtype b_type, VOID_STAR bp)
{
   Struct_Info_Type *si;
   Typecast_Info_Type *ti;
   SLuindex_Type i;
   SLang_Class_Type *acl, *bcl;
   int (*apush) (SLtype, VOID_STAR);
   int (*bpop) (SLtype, VOID_STAR);
   unsigned int ainc, binc;
   SLang_Name_Type *f;

   if (NULL == (si = find_struct_info (a_type, 1)))
     return -1;

   if ((NULL == (ti = find_typecast (si, b_type)))
       || (NULL == (f = ti->typecast_fun)))
     {
	_pSLang_verror (SL_TYPE_MISMATCH, "Typecast method not found");
	return -1;
     }

   acl = _pSLclass_get_class (a_type);
   bcl = _pSLclass_get_class (b_type);
   apush = acl->cl_apush;
   bpop = bcl->cl_apop;
   ainc = acl->cl_sizeof_type;
   binc = bcl->cl_sizeof_type;

   for (i = 0; i < na; i++)
     {
	if ((-1 == SLang_start_arg_list ())
	    || (-1 == (*apush) (a_type, ap))
	    || (-1 == SLang_end_arg_list ())
	    || (-1 == SLexecute_function (f))
	    || (-1 == (*bpop)(b_type, bp)))
	  return -1;

	ap = (VOID_STAR) ((char *)ap + ainc);
	bp = (VOID_STAR) ((char *)bp + binc);
     }

   return 1;
}

static void add_typecast_method (SLtype *fromtype, SLtype *totype, SLang_Ref_Type *ref)
{
   Struct_Info_Type *si;
   SLang_Name_Type *f;
   SLtype to = *totype, from = *fromtype;
   Typecast_Info_Type *ti;

   if (NULL == (f = SLang_get_fun_from_ref (ref)))
     return;

   if (NULL == (si = find_struct_info (from, 1)))
     return;

   if (NULL != (ti = find_typecast (si, to)))
     {
	if (ti->typecast_fun != NULL)
	  SLang_free_function (ti->typecast_fun);
	ti->typecast_fun = SLang_copy_function (f);
	return;
     }

   if (NULL == (ti = (Typecast_Info_Type *)SLmalloc (sizeof (Typecast_Info_Type))))
     return;

   ti->totype = to;
   ti->typecast_fun = SLang_copy_function (f);
   ti->next = si->ti;
   si->ti = ti;
   (void) SLclass_add_typecast (from, to, typecast_method, 1);
}

static int init_struct_with_user_methods (SLtype type, _pSLang_Struct_Type *s)
{
   Struct_Info_Type *si;

   if (NULL == (si = find_struct_info (type, 1)))
     return -1;

   s->destroy_method = SLang_copy_function (si->destroy_method);

   return 0;
}

static int struct_sput (SLtype type, SLFUTURE_CONST char *name)
{
   _pSLang_Struct_Type *s;

   (void) type;

   if (-1 == SLang_pop_struct (&s))
     return -1;

   if (-1 == pop_to_struct_field (s, name))
     {
	SLang_free_struct (s);
	return -1;
     }
   SLang_free_struct (s);
   return 0;
}

int _pSLstruct_pop_field (SLang_Struct_Type *s, SLFUTURE_CONST char *name, int do_free)
{
   int ret = pop_to_struct_field (s, name);

   if (do_free)
     SLang_free_struct (s);

   return ret;
}

int _pSLstruct_push_field (SLang_Struct_Type *s, SLFUTURE_CONST char *name, int do_free)
{
   _pSLstruct_Field_Type *f;
   int ret;

   if (NULL == (f = pop_field (s, name, find_field)))
     {
	if (do_free) SLang_free_struct (s);
	return -1;
     }

   ret = _pSLpush_slang_obj (&f->obj);
   if (do_free) SLang_free_struct (s);
   return ret;
}

static int struct_sget (SLtype type, SLFUTURE_CONST char *name)
{
   _pSLang_Struct_Type *s;
   _pSLstruct_Field_Type *f;
   int ret;

   (void) type;

   if (-1 == SLang_pop_struct (&s))
     return -1;

   if (NULL == (f = pop_field (s, name, find_field)))
     {
	SLang_free_struct (s);
	return -1;
     }

   ret = _pSLpush_slang_obj (&f->obj);
   SLang_free_struct (s);
   return ret;
}

static int struct_typecast
  (SLtype a_type, VOID_STAR ap, SLuindex_Type na,
   SLtype b_type, VOID_STAR bp)
{
   _pSLang_Struct_Type **a, **b;
   unsigned int i;

   (void) a_type;
   (void) b_type;

   a = (_pSLang_Struct_Type **) ap;
   b = (_pSLang_Struct_Type **) bp;
   for (i = 0; i < na; i++)
     {
	b[i] = a[i];
	if (a[i] != NULL)
	  a[i]->num_refs += 1;
     }

   return 1;
}

static char *string_method (SLtype type, VOID_STAR p)
{
   SLang_Struct_Type *s;
   Struct_Info_Type *si;
   SLang_Name_Type *f;
   char *str;

   s = *(SLang_Struct_Type **)p;

   si = find_struct_info (type, 0);
   if ((si == NULL)
       || (NULL == (f = si->string_method)))
     {
	char buf[256];
	(void) SLsnprintf (buf, sizeof(buf), "%s with %d fields", SLclass_get_datatype_name (type), s->nfields);
	return SLmake_string (buf);
     }

   if ((-1 == SLang_start_arg_list ())
       || (-1 == SLang_push_struct (s))
       || (-1 == SLang_end_arg_list ())
       || (-1 == SLexecute_function (f)))
     return NULL;

   if (-1 == SLpop_string (&str))
     return NULL;

   return str;
}

static int struct_eqs_method (SLtype a_type, VOID_STAR ap, SLtype b_type, VOID_STAR bp)
{
   SLang_Struct_Type *a, *b;
   _pSLstruct_Field_Type *afields;
   unsigned int i, nfields;

   a = *(SLang_Struct_Type **) ap;
   b = *(SLang_Struct_Type **) bp;

   /* Suppose typedef struct {x}T;  a = @T; b = typecast (a, Struct_Type);
    * Then a_type != b_type BUT ap == bp.  So compare pointers after types
    */
   if ((a_type != b_type)
       || (a->nfields != b->nfields))
     return 0;

   if (a == b)
     return 1;

   nfields = a->nfields;
   afields = a->fields;

   for (i = 0; i < nfields; i++)
     {
	if (NULL == find_field (b, afields[i].name))
	  return 0;
     }

   for (i = 0; i < nfields; i++)
     {
	int status;

	_pSLstruct_Field_Type *afield = afields + i;
	_pSLstruct_Field_Type *bfield = find_field (b, afield->name);
	status = _pSLclass_obj_eqs (&afield->obj, &bfield->obj);
	if (status <= 0)
	  return status;
     }

   return 1;
}

static int struct_acopy (SLtype unused, VOID_STAR src_sptr, VOID_STAR dest_sptr)
{
   _pSLang_Struct_Type *s;

   (void) unused;
   s = *(_pSLang_Struct_Type **)src_sptr;
   s->num_refs++;
   *(_pSLang_Struct_Type **)dest_sptr = s;
   return 0;
}

int _pSLstruct_define_typedef (void)
{
   char *type_name;
   _pSLang_Struct_Type *s, *s1;
   SLang_Class_Type *cl;

   if (-1 == SLang_pop_slstring (&type_name))
     return -1;

   if (-1 == SLang_pop_struct (&s))
     {
	SLang_free_slstring (type_name);
	return -1;
     }

   if (NULL == (s1 = make_struct_shell (s, SLANG_STRUCT_TYPE)))
     {
	SLang_free_slstring (type_name);
	SLang_free_struct (s);
	return -1;
     }

   SLang_free_struct (s);

   if (NULL == (cl = SLclass_allocate_class (type_name)))
     {
	SLang_free_slstring (type_name);
	SLang_free_struct (s1);
	return -1;
     }
   SLang_free_slstring (type_name);

   cl->cl_struct_def = s1;
   cl->cl_init_array_object = struct_init_array_object;
   cl->cl_datatype_deref = typedefed_struct_datatype_deref;
   cl->cl_destroy = struct_destroy;
   cl->cl_push = struct_push;
   cl->cl_dereference = struct_dereference;
   cl->cl_foreach_open = struct_foreach_open;
   cl->cl_foreach_close = struct_foreach_close;
   cl->cl_foreach = struct_foreach;

   (void) SLclass_set_string_function (cl, string_method);
   (void) SLclass_set_eqs_function (cl, struct_eqs_method);
   (void) SLclass_set_acopy_function (cl, struct_acopy);

   cl->cl_sget = struct_sget;
   cl->cl_sput = struct_sput;
   cl->is_container = 1;
   cl->is_struct = 1;

   if ((-1 == SLclass_register_class (cl,
				     SLANG_VOID_TYPE,   /* any open slot */
				     sizeof (_pSLang_Struct_Type),
				     SLANG_CLASS_TYPE_PTR))
       || (-1 == allocate_struct_info (cl->cl_data_type)))
     {
	/* FIXME: Priority=low */
	/* There is a memory leak here if this fails... */
	return -1;
     }
   /* Note: typecast from a user type to a struct type allowed but not the other
    * way.
    */
   if (-1 == SLclass_add_typecast (cl->cl_data_type, SLANG_STRUCT_TYPE, struct_typecast, 1))
     return -1;

   return 0;
}

static int
struct_datatype_deref (SLtype stype)
{
   (void) stype;

   if (SLang_peek_at_stack () == SLANG_ARRAY_TYPE)
     {
	SLang_Array_Type *at;
	int status;

	if (-1 == SLang_pop_array_of_type (&at, SLANG_STRING_TYPE))
	  return -1;

	status = SLstruct_create_struct (at->num_elements,
					 (SLFUTURE_CONST char **) at->data, NULL, NULL);

	SLang_free_array (at);
	return status;
     }

   if (-1 == SLang_push_int (SLang_Num_Function_Args))
     return -1;

   return _pSLstruct_define_struct ();
}

static int register_struct (void)
{
   SLang_Class_Type *cl;

   if (NULL == (cl = SLclass_allocate_class ("Struct_Type")))
     return -1;

   (void) SLclass_set_destroy_function (cl, struct_destroy);
   (void) SLclass_set_push_function (cl, struct_push);
   cl->cl_dereference = struct_dereference;
   cl->cl_datatype_deref = struct_datatype_deref;

   cl->cl_foreach_open = struct_foreach_open;
   cl->cl_foreach_close = struct_foreach_close;
   cl->cl_foreach = struct_foreach;

   cl->cl_sget = struct_sget;
   cl->cl_sput = struct_sput;
   (void) SLclass_set_string_function (cl, string_method);
   (void) SLclass_set_eqs_function (cl, struct_eqs_method);
   (void) SLclass_set_acopy_function (cl, struct_acopy);

   cl->is_container = 1;
   cl->is_struct = 1;

   if (-1 == SLclass_register_class (cl, SLANG_STRUCT_TYPE, sizeof (_pSLang_Struct_Type),
				     SLANG_CLASS_TYPE_PTR))
     return -1;

   return 0;
}

static void get_struct_field_names (_pSLang_Struct_Type *s)
{
   SLang_Array_Type *a;
   char **data;
   SLindex_Type i, nfields;
   _pSLstruct_Field_Type *f;

   nfields = (SLindex_Type) s->nfields;

   if (NULL == (a = SLang_create_array (SLANG_STRING_TYPE, 0, NULL, &nfields, 1)))
     return;

   f = s->fields;
   data = (char **) a->data;
   for (i = 0; i < nfields; i++)
     {
	/* Since we are dealing with hashed strings, the next call should not
	 * fail.  If it does, the interpreter will handle it at some other
	 * level.
	 */
	data [i] = SLang_create_slstring (f[i].name);
     }

   SLang_push_array (a, 1);
}

static int push_struct_fields (_pSLang_Struct_Type *s)
{
   _pSLstruct_Field_Type *f, *fmax;
   int num;

   f = s->fields;
   fmax = f + s->nfields;

   num = 0;
   while (fmax > f)
     {
	fmax--;
	if (-1 == _pSLpush_slang_obj (&fmax->obj))
	  break;

	num++;
     }

   return num;
}

/* Syntax: set_struct_field (s, name, value); */
static void struct_set_field (void)
{
   _pSLang_Struct_Type *s;
   _pSLstruct_Field_Type *f;
   SLang_Object_Type obj;
   char *name;

   if (-1 == SLang_pop (&obj))
     return;

   if (-1 == SLang_pop_slstring (&name))
     {
	SLang_free_object (&obj);
	return;
     }

   if (-1 == SLang_pop_struct (&s))
     {
	SLang_free_slstring (name);
	SLang_free_object (&obj);
	return;
     }

   if (NULL == (f = pop_field (s, name, find_field)))
     {
	SLang_free_struct (s);
	SLang_free_slstring (name);
	SLang_free_object (&obj);
	return;
     }

   SLang_free_object (&f->obj);
   f->obj = obj;

   SLang_free_struct (s);
   SLang_free_slstring (name);
}

/* Syntax: set_struct_fields (s, values....); */
static void set_struct_fields (void)
{
   unsigned int n;
   _pSLang_Struct_Type *s;
   _pSLstruct_Field_Type *f;

   n = (unsigned int) SLang_Num_Function_Args;

   if (-1 == SLreverse_stack (n))
     return;

   n--;
   if (-1 == SLang_pop_struct (&s))
     {
	SLdo_pop_n (n);
	return;
     }

   if (n > s->nfields)
     {
	SLdo_pop_n (n);
	_pSLang_verror (SL_INVALID_PARM, "Too many values for structure");
	SLang_free_struct (s);
	return;
     }

   f = s->fields;
   while (n > 0)
     {
	SLang_Object_Type obj;

	if (-1 == SLang_pop (&obj))
	  break;

	SLang_free_object (&f->obj);
	f->obj = obj;

	f++;
	n--;
     }

   SLang_free_struct (s);
}

static void get_struct_field (char *name)
{
   (void) struct_sget (0, name);
}

static int is_struct_type (void)
{
   SLang_Object_Type obj;
   SLtype type;
   int status;

   if (-1 == SLang_pop (&obj))
     return -1;

   type = obj.o_data_type;
   if (type == SLANG_STRUCT_TYPE)
     status = 1;
   else
     status = (NULL != _pSLclass_get_class (type)->cl_struct_def);
   SLang_free_object (&obj);
   return status;
}

static int is_struct_type1 (void)
{
   SLang_Object_Type obj;
   SLtype type;
   int status;

   if (-1 == SLang_pop (&obj))
     return -1;

   type = obj.o_data_type;
   if (type == SLANG_ARRAY_TYPE)
     type = obj.v.array_val->data_type;
   if (type == SLANG_STRUCT_TYPE)
     status = 1;
   else
     status = (NULL != _pSLclass_get_class (type)->cl_struct_def);
   SLang_free_object (&obj);
   return status;
}

static SLang_Intrin_Fun_Type Struct_Table [] =
{
   MAKE_INTRINSIC_1("get_struct_field_names", get_struct_field_names, SLANG_VOID_TYPE, SLANG_STRUCT_TYPE),
   MAKE_INTRINSIC_1("get_struct_field", get_struct_field, SLANG_VOID_TYPE, SLANG_STRING_TYPE),
   MAKE_INTRINSIC_1("_push_struct_field_values", push_struct_fields, SLANG_INT_TYPE, SLANG_STRUCT_TYPE),
   MAKE_INTRINSIC_0("set_struct_field", struct_set_field, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("set_struct_fields", set_struct_fields, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("is_struct_type", is_struct_type, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("_is_struct_type", is_struct_type1, SLANG_INT_TYPE),
   MAKE_INTRINSIC_0("__add_unary", add_unary_op_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("__add_binary", add_binary_op_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("__add_destroy", add_destroy_method, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_2("__add_string", add_string_method, SLANG_VOID_TYPE, SLANG_DATATYPE_TYPE, SLANG_REF_TYPE),
   MAKE_INTRINSIC_2("__add_aget", add_aget_method, SLANG_VOID_TYPE, SLANG_DATATYPE_TYPE, SLANG_REF_TYPE),
   MAKE_INTRINSIC_2("__add_aput", add_aput_method, SLANG_VOID_TYPE, SLANG_DATATYPE_TYPE, SLANG_REF_TYPE),
   MAKE_INTRINSIC_3("__add_typecast", add_typecast_method, SLANG_VOID_TYPE, SLANG_DATATYPE_TYPE, SLANG_DATATYPE_TYPE, SLANG_REF_TYPE),

   /* MAKE_INTRINSIC_I("_create_struct", create_struct, SLANG_VOID_TYPE), */
   SLANG_END_INTRIN_FUN_TABLE
};

int _pSLstruct_init (void)
{
   if ((-1 == SLadd_intrin_fun_table (Struct_Table, NULL))
       || (-1 == register_struct ()))
     return -1;

   return 0;
}

void _pSLstruct_pop_args (int *np)
{
   SLang_Array_Type *at;
   SLindex_Type i, n;
   _pSLang_Struct_Type **data;

   n = *np;

   if (n < 0)
     {
	SLang_set_error (SL_INVALID_PARM);
	return;
     }

   data = (_pSLang_Struct_Type **) _SLcalloc (n, sizeof (_pSLang_Struct_Type *));
   if (data == NULL)
     {
	SLdo_pop_n (n);
	return;
     }

   memset ((char *)data, 0, n * sizeof (_pSLang_Struct_Type *));

   i = n;
   while (i > 0)
     {
	_pSLang_Struct_Type *s;
	_pSLstruct_Field_Type *f;

	i--;

	if (NULL == (s = allocate_struct (1)))
	  goto return_error;

	data[i] = s;
	s->num_refs += 1;	       /* keeping a copy */

	f = s->fields;
	if (NULL == (f->name = SLang_create_slstring ("value")))
	  goto return_error;

	if (-1 == SLang_pop (&f->obj))
	  goto return_error;
     }

   if (NULL == (at = SLang_create_array (SLANG_STRUCT_TYPE, 0,
					 (VOID_STAR) data, &n, 1)))
     goto return_error;

   (void) SLang_push_array (at, 1);
   return;

   return_error:
   for (i = 0; i < n; i++)
     {
	_pSLang_Struct_Type *s;

	s = data[i];
	if (s != NULL)
	  SLang_free_struct (s);
     }

   SLfree ((char *) data);
}

void _pSLstruct_push_args (SLang_Array_Type *at)
{
   _pSLang_Struct_Type **sp;
   unsigned int num;

   if (at->data_type != SLANG_STRUCT_TYPE)
     {
	SLang_set_error (SL_TYPE_MISMATCH);
	return;
     }

   sp = (_pSLang_Struct_Type **) at->data;
   num = at->num_elements;

   while ((_pSLang_Error == 0) && (num > 0))
     {
	_pSLang_Struct_Type *s;

	num--;
	if (NULL == (s = *sp++))
	  {
	     SLang_push_null ();
	     continue;
	  }

	/* I should check to see if the value field is present, but... */
	(void) _pSLpush_slang_obj (&s->fields->obj);
     }
}

/* C structures */
static _pSLstruct_Field_Type *find_field_via_strcmp (_pSLang_Struct_Type *s, SLCONST char *name)
{
   _pSLstruct_Field_Type *f, *fmax;

   f = s->fields;
   fmax = f + s->nfields;

   while (f < fmax)
     {
	if (0 == strcmp (name, f->name))
	  return f;

	f++;
     }
   return NULL;
}

static void free_cstruct_field (SLang_CStruct_Field_Type *cfield, VOID_STAR cs)
{
   SLang_Class_Type *cl;

   if ((cfield->read_only == 0)
       && (NULL != (cl = _pSLclass_get_class (cfield->type))))
     _pSLarray_free_array_elements (cl, (VOID_STAR)((char*)cs + cfield->offset), 1);
}

void SLang_free_cstruct (VOID_STAR cs, SLang_CStruct_Field_Type *cfields)
{
   if ((cs == NULL) || (cfields == NULL))
     return;

   while (cfields->field_name != NULL)
     {
	free_cstruct_field (cfields, cs);
	cfields++;
     }
}

int SLang_pop_cstruct (VOID_STAR cs, SLang_CStruct_Field_Type *cfields)
{
   _pSLang_Struct_Type *s;
   SLang_CStruct_Field_Type *cfield;
   SLCONST char *field_name;
   char *cs_addr;

   if ((cfields == NULL) || (cs == NULL))
     return -1;

   if (-1 == SLang_pop_struct (&s))
     return -1;

   cfield = cfields;
   cs_addr = (char *) cs;

   while (NULL != (field_name = cfield->field_name))
     {
	if (cfield->read_only == 0)
	  {
	     _pSLstruct_Field_Type *f;
	     SLang_Class_Type *cl;
	     VOID_STAR addr = (VOID_STAR) (cs_addr + cfield->offset);

	     if ((NULL == (f = pop_field (s, field_name, find_field_via_strcmp)))
		 || (-1 == _pSLpush_slang_obj (&f->obj)))
	       goto return_error;

	     if (cfield->type == SLANG_ARRAY_TYPE)
	       {
		  if (-1 == SLang_pop_array ((SLang_Array_Type **)addr, 1))
		    goto return_error;
	       }
	     else if ((NULL == (cl = _pSLclass_get_class (cfield->type)))
		      || (-1 == (*cl->cl_apop)(cfield->type, addr)))
	       goto return_error;
	  }

	cfield++;
     }

   SLang_free_struct (s);
   return 0;

   return_error:
   while (cfield != cfields)
     {
	free_cstruct_field (cfield, cs);
	cfield--;
     }
   SLang_free_struct (s);
   return -1;
}

static _pSLang_Struct_Type *create_cstruct (VOID_STAR cs, SLang_CStruct_Field_Type *cfields)
{
   unsigned int i, n;
   _pSLang_Struct_Type *s;
   SLang_CStruct_Field_Type *cfield;
   SLFUTURE_CONST char **field_names;
   VOID_STAR *field_values;
   SLtype *field_types;

   if ((cs == NULL) || (cfields == NULL))
     return NULL;

   cfield = cfields;
   while (cfield->field_name != NULL)
     cfield++;
   n = cfield - cfields;
   if (n == 0)
     {
	_pSLang_verror (SL_APPLICATION_ERROR, "C structure has no fields");
	return NULL;
     }

   s = NULL;
   field_types = NULL;
   field_values = NULL;
   if ((NULL == (field_names = (SLFUTURE_CONST char **) _SLcalloc (n,sizeof (char *))))
       || (NULL == (field_types = (SLtype *)_SLcalloc (n,sizeof(SLtype))))
       || (NULL == (field_values = (VOID_STAR *)_SLcalloc (n,sizeof(VOID_STAR)))))
     goto return_error;

   for (i = 0; i < n; i++)
     {
	cfield = cfields + i;
	field_names[i] = cfield->field_name;
	field_types[i] = cfield->type;
	field_values[i] = (VOID_STAR)((char *)cs + cfield->offset);
     }

   s = create_struct (n, field_names, field_types, field_values);
   /* drop */

   return_error:
   SLfree ((char *) field_values);
   SLfree ((char *) field_types);
   SLfree ((char *) field_names);

   return s;
}

int SLang_push_cstruct (VOID_STAR cs, SLang_CStruct_Field_Type *cfields)
{
   _pSLang_Struct_Type *s;

   if (NULL == (s = create_cstruct (cs, cfields)))
     return -1;

   if (0 == SLang_push_struct (s))
     return 0;

   SLang_free_struct (s);
   return -1;
}

int SLang_assign_cstruct_to_ref (SLang_Ref_Type *ref, VOID_STAR cs, SLang_CStruct_Field_Type *cfields)
{
   _pSLang_Struct_Type *s;

   if (NULL == (s = create_cstruct (cs, cfields)))
     return -1;

   if (0 == SLang_assign_to_ref (ref, SLANG_STRUCT_TYPE, (VOID_STAR) &s))
     return 0;

   SLang_free_struct (s);
   return -1;
}

/* Struct Field Reference */
typedef struct
{
   SLang_Struct_Type *s;
   SLCONST char *field_name;
}
Struct_Field_Ref_Type;

static int struct_field_deref_assign (VOID_STAR vdata)
{
   Struct_Field_Ref_Type *data = (Struct_Field_Ref_Type *)vdata;
   return pop_to_struct_field (data->s, data->field_name);
}

static int struct_field_deref (VOID_STAR vdata)
{
   Struct_Field_Ref_Type *frt = (Struct_Field_Ref_Type *)vdata;
   _pSLstruct_Field_Type *f;

   if (NULL == (f = pop_field (frt->s, frt->field_name, find_field)))
     return -1;

   return _pSLpush_slang_obj (&f->obj);
}

static void struct_field_ref_destroy (VOID_STAR vdata)
{
   Struct_Field_Ref_Type *frt = (Struct_Field_Ref_Type *)vdata;

   SLang_free_slstring ((char *) frt->field_name);
   SLang_free_struct (frt->s);
}

/* Stack: struct */
int _pSLstruct_push_field_ref (SLFUTURE_CONST char *name)
{
   SLang_Struct_Type *s;
   Struct_Field_Ref_Type *frt;
   SLang_Ref_Type *ref;
   int ret;

   if (-1 == SLang_pop_struct (&s))
     return -1;

   if (NULL == (name = SLang_create_slstring (name)))
     {
	SLang_free_struct (s);
	return -1;
     }
   if (NULL == (ref = _pSLang_new_ref (sizeof (Struct_Field_Ref_Type))))
     {
	SLang_free_struct (s);
	SLang_free_slstring ((char *) name);
     }
   frt = (Struct_Field_Ref_Type *) ref->data;
   frt->s = s;
   frt->field_name = name;
   ref->deref = struct_field_deref;
   ref->deref_assign = struct_field_deref_assign;
   ref->destroy = struct_field_ref_destroy;

   ret = SLang_push_ref (ref);
   SLang_free_ref (ref);
   return ret;
}

