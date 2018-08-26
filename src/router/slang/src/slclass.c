/* User defined objects */
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
#include <errno.h>

/* #define SL_APP_WANTS_FOREACH */
#include "slang.h"
#include "_slang.h"

/* This implementation of the class tables assumes SLtype is 16 bit */
typedef struct
{
   SLang_Class_Type *classes[256];
   unsigned int nclasses;
}
Class_Table_Type;
static Class_Table_Type *Class_Tables[256];

static void add_class_to_slot (Class_Table_Type *t, SLang_Class_Type **clp,
			       SLang_Class_Type *cl)
{
   *clp = cl;
   t->nclasses++;
#if SLANG_OPTIMIZE_FOR_SPEED
   _pSLang_set_class_info (cl->cl_data_type, cl);
#endif
}

static SLang_Class_Type *lookup_class (SLtype type)
{
   Class_Table_Type *t;

   t = Class_Tables[(type >> 8)&0xFF];

   if (t == NULL)
     return NULL;
   return t->classes[type & 0xFF];
}

static SLang_Class_Type **alloc_class_slot (SLtype type, Class_Table_Type **tp)
{
   unsigned int i;
   Class_Table_Type *t;

   if ((type&0xFFFF) != type)
     {
	_pSLang_verror (SL_APPLICATION_ERROR, "Class-Id larger than 0xFFFF is not supported");
	return NULL;
     }

   i = ((type >> 8) & 0xFF);
   if (NULL == (t = Class_Tables[i]))
     {
	t = (Class_Table_Type *)SLcalloc (1, sizeof (Class_Table_Type));
	if (t == NULL)
	  return NULL;
	Class_Tables[i] = t;
     }
   *tp = t;
   return t->classes + (type&0xFF);
}

static SLang_Class_Type **find_empty_class_slot (SLtype *typep, Class_Table_Type **tp)
{
   unsigned int i;
   Class_Table_Type *t;
   SLtype type;

   /* Class_Tables[0] is reserved (0 <= SLtype < 256) by interpreter */
   for (i = 1; i < 256; i++)
     {
	unsigned int j;
	SLang_Class_Type **clp;

	if (NULL == (t = Class_Tables[i]))
	  {
	     type = (SLtype) (i << 8);
	     clp = alloc_class_slot (type, &t);
	     if (clp != NULL)
	       {
		  *typep = type;
		  *tp = t;
	       }
	     return clp;
	  }

	if (t->nclasses == 256)
	  continue;

	clp = t->classes;

	for (j = 0; j < 256; j++)
	  {
	     if (clp[j] == NULL)
	       {
		  *typep = (SLtype) ((i << 8) | j);
		  *tp = t;
		  return clp + j;
	       }
	  }

	_pSLang_verror (SL_INTERNAL_ERROR, "Class table nclasses variable is out of sync");
	return NULL;
     }

   return NULL;
}

static SLang_Class_Type *lookup_class_by_name (SLCONST char *name)
{
   unsigned int i;

   for (i = 0; i < 256; i++)
     {
	Class_Table_Type *t = Class_Tables[i];
	SLang_Class_Type **clp, **clpmax;

	if (t == NULL)
	  continue;

	clp = t->classes;
	clpmax = t->classes + 256;

	while (clp < clpmax)
	  {
	     SLang_Class_Type *cl;
	     if ((NULL != (cl = *clp))
		 && (0 == strcmp (cl->cl_name, name)))
	       return cl;
	     clp++;
	  }
     }
   return NULL;
}

SLang_Class_Type *_pSLclass_get_class (SLtype type)
{
   SLang_Class_Type *cl;

   if (NULL == (cl = lookup_class (type)))
     SLang_exit_error ("Application error: Type %d not registered", (int) type);

   return cl;
}

int SLclass_is_class_defined (SLtype type)
{
   return (NULL != lookup_class (type));
}

int _pSLclass_copy_class (SLtype to, SLtype from)
{
   SLang_Class_Type *cl, **clp;
   Class_Table_Type *t;

   cl = _pSLclass_get_class (from);
   if (NULL == (clp = alloc_class_slot (to, &t)))
     return -1;

   if (*clp != NULL)
     {
	_pSLang_verror (SL_APPLICATION_ERROR, "Class %d already exists", to);
	SLang_exit_error ("Application error: Fatal error");
     }
   add_class_to_slot (t, clp, cl);
#if SLANG_OPTIMIZE_FOR_SPEED
   _pSLang_set_class_info (to, cl);
#endif
   return 0;
}

VOID_STAR _pSLclass_get_ptr_to_value (SLang_Class_Type *cl,
				     SLang_Object_Type *obj)
{
   VOID_STAR p;

   switch (cl->cl_class_type)
     {
      case SLANG_CLASS_TYPE_MMT:
      case SLANG_CLASS_TYPE_PTR:
      case SLANG_CLASS_TYPE_SCALAR:
	p = (VOID_STAR) &obj->v;
	break;

      case SLANG_CLASS_TYPE_VECTOR:
	p = obj->v.ptr_val;
	break;

      default:
	p = NULL;
     }
   return p;
}

SLFUTURE_CONST char *SLclass_get_datatype_name (SLtype stype)
{
   SLang_Class_Type *cl;

   cl = _pSLclass_get_class (stype);
   return cl->cl_name;
}

static int method_undefined_error (SLtype type, SLCONST char *method, SLCONST char *name)
{
   if (name == NULL) name = SLclass_get_datatype_name (type);

   _pSLang_verror (SL_TYPE_MISMATCH, "%s method not defined for %s",
		 method, name);
   return -1;
}

static int
scalar_vector_bin_op_result (int op, SLtype a, SLtype b,
			     SLtype *c)
{
   (void) a; (void) b;
   switch (op)
     {
      case SLANG_NE:
      case SLANG_EQ:
	*c = SLANG_CHAR_TYPE;
	return 1;
     }
   return 0;
}

static int
scalar_vector_bin_op (int op,
		      SLtype a_type, VOID_STAR ap, SLuindex_Type na,
		      SLtype b_type, VOID_STAR bp, SLuindex_Type nb,
		      VOID_STAR cp)
{
   char *c;
   char *a, *b;
   unsigned int da, db;
   unsigned int n, n_max;
   unsigned int data_type_len;
   SLang_Class_Type *cl;

   (void) b_type;
   cl = _pSLclass_get_class (a_type);

   data_type_len = cl->cl_sizeof_type;

   a = (char *) ap;
   b = (char *) bp;
   c = (char *) cp;

   if (na == 1) da = 0; else da = data_type_len;
   if (nb == 1) db = 0; else db = data_type_len;
   if (na > nb) n_max = na; else n_max = nb;

   switch (op)
     {
      default:
	return 0;

      case SLANG_NE:
	for (n = 0; n < n_max; n++)
	  {
	     c[n] = (0 != SLMEMCMP(a, b, data_type_len));
	     a += da; b += db;
	  }
	break;

      case SLANG_EQ:
	for (n = 0; n < n_max; n++)
	  {
	     c[n] = (0 == SLMEMCMP(a, b, data_type_len));
	     a += da; b += db;
	  }
	break;
     }
   return 1;
}

static int scalar_fread (SLtype type, FILE *fp, VOID_STAR ptr,
			 unsigned int desired, unsigned int *actual)
{
   unsigned int n;
   char *buf = (char *)ptr;
   size_t desired_bytes, actual_bytes;
   size_t size = _pSLclass_get_class (type)->cl_sizeof_type;

   desired_bytes = size * desired;
   actual_bytes = 0;

   while (desired_bytes)
     {
	int e;

	errno = 0;
	clearerr (fp);
	n = fread (buf, 1, desired_bytes, fp);

	actual_bytes += n;
	if (n == desired_bytes)
	  break;

	e = errno;
	desired_bytes -= n;
	buf += n;

	/* clearerr (fp); */
#ifdef EINTR
	if ((e == EINTR)
	    && (0 == SLang_handle_interrupt ()))
	  continue;
#endif
	_pSLerrno_errno = e;
	break;
     }

   if (actual_bytes % size)
     {
	/* Sigh.  We failed to read a full object. */
     }
   *actual = actual_bytes / size;
   return 0;
}

static int scalar_fwrite (SLtype type, FILE *fp, VOID_STAR ptr,
			 unsigned int desired, unsigned int *actual)
{
   unsigned int n;
   char *buf = (char *)ptr;
   size_t desired_bytes, actual_bytes;
   size_t size = _pSLclass_get_class (type)->cl_sizeof_type;

   desired_bytes = size * desired;
   actual_bytes = 0;

   while (desired_bytes)
     {
	int e;

	errno = 0;
	clearerr (fp);
	n = fwrite (buf, 1, desired_bytes, fp);

	actual_bytes += n;
	if (n == desired_bytes)
	  break;

	e = errno;
	desired_bytes -= n;
	buf += n;

	/* clearerr (fp); */
#ifdef EINTR
	if ((e == EINTR)
	    && (0 == SLang_handle_interrupt ()))
	  continue;
#endif
	_pSLerrno_errno = e;

	/* Apparantly, the write can be interrupted returning a short item
	 * count but not set errno.
	 */
	if (n == 0)
	  break;

	/* See the comment in slstdio.c:signal_safe_fputs about this */
#ifdef EPIPE
	if (e == EPIPE)
	  break;
#endif
     }

   if (actual_bytes % size)
     {
	/* Sigh.  We failed to write out a full object. */
     }
   *actual = actual_bytes / size;
   return 0;
}

static int vector_apush (SLtype type, VOID_STAR ptr)
{
   SLang_Class_Type *cl;

   cl = _pSLclass_get_class (type);
   return (*cl->cl_push)(type, (VOID_STAR) &ptr);
}

static int vector_apop (SLtype type, VOID_STAR ptr)
{
   SLang_Class_Type *cl;

   cl = _pSLclass_get_class (type);
   return (*cl->cl_pop)(type, (VOID_STAR) &ptr);
}

static int default_push_mmt (SLtype type_unused, VOID_STAR ptr)
{
   SLang_MMT_Type *ref;

   (void) type_unused;
   ref = *(SLang_MMT_Type **) ptr;
   return SLang_push_mmt (ref);
}

static void default_destroy_simple (SLtype type_unused, VOID_STAR ptr_unused)
{
   (void) type_unused;
   (void) ptr_unused;
}

static void default_destroy_user (SLtype type, VOID_STAR ptr)
{
   (void) type;
   SLang_free_mmt (*(SLang_MMT_Type **) ptr);
}

static int default_pop (SLtype type, VOID_STAR ptr)
{
   return SLclass_pop_ptr_obj (type, (VOID_STAR *) ptr);
}

static int default_datatype_deref (SLtype type)
{
   return method_undefined_error (type, "datatype_deref", NULL);
}

static int default_acopy (SLtype type, VOID_STAR from, VOID_STAR to)
{
   SLang_Class_Type *cl;

   cl = _pSLclass_get_class (type);
   if (-1 == (*cl->cl_apush) (type, from))
     return -1;
   return (*cl->cl_apop) (type, to);
}

static int scalar_acopy (SLtype type, VOID_STAR from, VOID_STAR to)
{
   memcpy ((char *)to, (char *)from, _pSLclass_get_class (type)->cl_sizeof_type);
   return 0;
}

int SLclass_dup_object (SLtype type, VOID_STAR from, VOID_STAR to)
{
   SLang_Class_Type *cl = _pSLclass_get_class (type);
   return cl->cl_acopy (type, from, to);
}

static int default_dereference_object (SLtype type, VOID_STAR ptr)
{
   (void) ptr;
   return method_undefined_error (type, "dereference", NULL);
}

static char *default_string (SLtype stype, VOID_STAR v)
{
   char buf [256];
   char *s;
#if SLANG_HAS_COMPLEX
   double *cplx;
#endif
   s = buf;

   switch (stype)
     {
      case SLANG_STRING_TYPE:
	s = *(char **) v;
	break;

      case SLANG_NULL_TYPE:
	s = (char *) "NULL";
	break;

      case SLANG_DATATYPE_TYPE:
	s = (char *) SLclass_get_datatype_name ((SLtype) *(int *)v);
	break;

#if SLANG_HAS_COMPLEX
      case SLANG_COMPLEX_TYPE:
	cplx = *(double **) v;
	if (cplx[1] < 0)
	  sprintf (s, "(%g - %gi)", cplx [0], -cplx [1]);
	else
	  sprintf (s, "(%g + %gi)", cplx [0], cplx [1]);
	break;
#endif
      default:
	s = (char *) SLclass_get_datatype_name (stype);
     }

   return SLmake_string (s);
}

static int
use_cmp_bin_op_result (int op, SLtype a, SLtype b,
		       SLtype *c)
{
   if (a != b)
     return 0;
   switch (op)
     {
      case SLANG_NE:
      case SLANG_EQ:
      case SLANG_LT:
      case SLANG_LE:
      case SLANG_GT:
      case SLANG_GE:
	*c = SLANG_INT_TYPE;
	return 1;
     }
   return 0;
}

static int
use_cmp_bin_op (int op,
		SLtype a_type, VOID_STAR ap, SLuindex_Type na,
		SLtype b_type, VOID_STAR bp, SLuindex_Type nb,
		VOID_STAR cp)
{
   int *c;
   char *a, *b;
   unsigned int da, db;
   unsigned int n, n_max;
   unsigned int data_type_len;
   SLang_Class_Type *cl;
   int (*cmp)(SLtype, VOID_STAR, VOID_STAR, int *);

   (void) b_type;
   cl = _pSLclass_get_class (a_type);
   cmp = cl->cl_cmp;
   data_type_len = cl->cl_sizeof_type;

   a = (char *) ap;
   b = (char *) bp;
   c = (int *) cp;

   if (na == 1) da = 0; else da = data_type_len;
   if (nb == 1) db = 0; else db = data_type_len;
   if (na > nb) n_max = na; else n_max = nb;

   switch (op)
     {
	int result;

      default:
	return 0;

      case SLANG_NE:
	for (n = 0; n < n_max; n++)
	  {
	     if (-1 == (*cmp) (a_type, (VOID_STAR)a, (VOID_STAR)b, &result))
	       return -1;
	     c[n] = (result != 0);
	     a += da; b += db;
	  }
	break;

      case SLANG_EQ:
	for (n = 0; n < n_max; n++)
	  {
	     if (-1 == (*cmp) (a_type, (VOID_STAR)a, (VOID_STAR)b, &result))
	       return -1;
	     c[n] = (result == 0);
	     a += da; b += db;
	  }
	break;

      case SLANG_GT:
	for (n = 0; n < n_max; n++)
	  {
	     if (-1 == (*cmp) (a_type, (VOID_STAR)a, (VOID_STAR)b, &result))
	       return -1;
	     c[n] = (result > 0);
	     a += da; b += db;
	  }
	break;
      case SLANG_GE:
	for (n = 0; n < n_max; n++)
	  {
	     if (-1 == (*cmp) (a_type, (VOID_STAR)a, (VOID_STAR)b, &result))
	       return -1;
	     c[n] = (result >= 0);
	     a += da; b += db;
	  }
	break;
      case SLANG_LT:
	for (n = 0; n < n_max; n++)
	  {
	     if (-1 == (*cmp) (a_type, (VOID_STAR)a, (VOID_STAR)b, &result))
	       return -1;
	     c[n] = (result < 0);
	     a += da; b += db;
	  }
	break;
      case SLANG_LE:
	for (n = 0; n < n_max; n++)
	  {
	     if (-1 == (*cmp) (a_type, (VOID_STAR)a, (VOID_STAR)b, &result))
	       return -1;
	     c[n] = (result <= 0);
	     a += da; b += db;
	  }
	break;
     }
   return 1;
}

int _pSLclass_is_same_obj (SLang_Object_Type *a, SLang_Object_Type *b)
{
   SLang_Class_Type *cl;
   unsigned int sizeof_type;

   if (a->o_data_type != b->o_data_type)
     return 0;

   cl = _pSLclass_get_class (a->o_data_type);
   sizeof_type = cl->cl_sizeof_type;

   switch (cl->cl_class_type)
     {
      case SLANG_CLASS_TYPE_MMT:
      case SLANG_CLASS_TYPE_PTR:
	return (a->v.ptr_val == b->v.ptr_val);

      case SLANG_CLASS_TYPE_SCALAR:
	return !memcmp (&a->v, &b->v, sizeof_type);

      case SLANG_CLASS_TYPE_VECTOR:
	return !memcmp (a->v.ptr_val, b->v.ptr_val, sizeof_type);
     }
   return 0;
}

static int do_default_eqs (SLang_Class_Type *a_cl, VOID_STAR pa,
			   SLang_Class_Type *b_cl, VOID_STAR pb)
{
   SLang_Class_Type *c_cl;
   int (*binary_fun) (int,
		      SLtype, VOID_STAR, SLuindex_Type,
		      SLtype, VOID_STAR, SLuindex_Type,
		      VOID_STAR);
   VOID_STAR pc;
   int ret;

   if (NULL == (binary_fun = _pSLclass_get_binary_fun (SLANG_EQ, a_cl, b_cl, &c_cl, 0)))
     {
	if (a_cl != b_cl)
	  return 0;

	switch (a_cl->cl_class_type)
	  {
	   case SLANG_CLASS_TYPE_MMT:
	   case SLANG_CLASS_TYPE_PTR:
	     return (*(VOID_STAR *)pa == *(VOID_STAR *)pb);

	   case SLANG_CLASS_TYPE_SCALAR:
	   case SLANG_CLASS_TYPE_VECTOR:
	     return !memcmp ((char *)pa, (char *)pb, a_cl->cl_sizeof_type);
	  }
	return 0;
     }

   pc = c_cl->cl_transfer_buf;

   if (1 != (*binary_fun) (SLANG_EQ, a_cl->cl_data_type, pa, 1, b_cl->cl_data_type, pb, 1, pc))
     return 0;

   /* apush will create a copy, so make sure we free after the push */
   ret = (*c_cl->cl_apush)(c_cl->cl_data_type, pc);
   (*c_cl->cl_adestroy)(c_cl->cl_data_type, pc);

   if (ret != 0)
     return -1;

   if (-1 == SLang_pop_integer (&ret))
     return -1;

   return (ret != 0);
}

/* This stack business is necessary to avoid problems with circular references */
typedef struct Eqs_Stack_Type
{
   SLang_Object_Type *a, *b;
   struct Eqs_Stack_Type *next;
}
Eqs_Stack_Type;
static Eqs_Stack_Type *Eqs_Stack;
static int push_eqs_comparison (SLang_Object_Type *a, SLang_Object_Type *b)
{
   Eqs_Stack_Type *s = Eqs_Stack;
   while (s != NULL)
     {
	if (((s->a == a) && (s->b == b))
	    || ((s->b == a) || (s->a == b)))
	  return 1;

	s = s->next;
     }
   s = (Eqs_Stack_Type *) SLmalloc (sizeof (Eqs_Stack_Type));
   if (s == NULL)
     return -1;
   s->a = a;
   s->b = b;
   s->next = Eqs_Stack;
   Eqs_Stack = s;
   return 0;
}

static void pop_eqs_comparison (void)
{
   Eqs_Stack_Type *s = Eqs_Stack;
   Eqs_Stack = s->next;
   SLfree ((char *) s);
}

int _pSLclass_obj_eqs (SLang_Object_Type *a, SLang_Object_Type *b)
{
   SLang_Class_Type *a_cl, *b_cl;
   VOID_STAR pa, pb;
   int (*eqs)(SLtype, VOID_STAR, SLtype, VOID_STAR);
   int status;

   a_cl = _pSLclass_get_class (a->o_data_type);
   b_cl = _pSLclass_get_class (b->o_data_type);

   pa = _pSLclass_get_ptr_to_value (a_cl, a);
   pb = _pSLclass_get_ptr_to_value (b_cl, b);

   if ((pa == NULL) || (pb == NULL))
     return -1;

   if ((NULL == (eqs = a_cl->cl_eqs))
       && (NULL == (eqs = b_cl->cl_eqs)))
     return do_default_eqs (a_cl, pa, b_cl, pb);

   status = push_eqs_comparison (a, b);
   if (status != 0)
     return status;

   status = (*eqs) (a->o_data_type, pa, b->o_data_type, pb);
   pop_eqs_comparison ();
   return status;
}

int SLclass_get_class_id (SLang_Class_Type *cl)
{
   if (cl == NULL)
     return -1;
   return (int) cl->cl_data_type;
}

SLang_Class_Type *SLclass_allocate_class (SLFUTURE_CONST char *name)
{
   SLang_Class_Type *cl;

   if (NULL != (cl = lookup_class_by_name (name)))
     {
	_pSLang_verror (SL_DUPLICATE_DEFINITION, "Type name %s already exists", name);
	return NULL;
     }

   cl = (SLang_Class_Type *) SLmalloc (sizeof (SLang_Class_Type));
   if (cl == NULL) return NULL;

   SLMEMSET ((char *) cl, 0, sizeof (SLang_Class_Type));

   if (NULL == (cl->cl_name = SLang_create_slstring (name)))
     {
	SLfree ((char *) cl);
	return NULL;
     }

   return cl;
}

int SLang_push_datatype (SLtype data_type)
{
   /* This data type could be a copy of another type, e.g., short and
    * int if they are the same size (Int16 == Short).  So, make sure
    * we push the original and not the copy.
    */
   data_type = _pSLclass_get_class (data_type)->cl_data_type;
   return SLclass_push_int_obj (SLANG_DATATYPE_TYPE, data_type);
}

static int datatype_deref (SLtype type, VOID_STAR ptr)
{
   SLang_Class_Type *cl;
   int status;

   /* The parser generated code for this as if a function call were to be
    * made.  However, we are calling the deref object routine
    * instead of the function call.  So, I must simulate the function call.
    */
   if (-1 == _pSL_increment_frame_pointer ())
     return -1;

   type = (SLtype) *(int *) ptr;
   cl = _pSLclass_get_class (type);
   status = (*cl->cl_datatype_deref) (type);

   (void) _pSL_decrement_frame_pointer ();
   return status;
}

static int datatype_push (SLtype type_unused, VOID_STAR ptr)
{
   (void) type_unused;
   return SLang_push_datatype (*(SLtype *) ptr);
}

int SLang_pop_datatype (SLtype *type)
{
   int i;
   if (-1 == SLclass_pop_int_obj (SLANG_DATATYPE_TYPE, &i))
     return -1;

   *type = (SLtype) i;
   return 0;
}

static int datatype_pop (SLtype type, VOID_STAR ptr)
{
   if (-1 == SLang_pop_datatype (&type))
     return -1;

   *(SLtype *) ptr = type;
   return 0;
}

int _pSLclass_init (void)
{
   SLang_Class_Type *cl;

   /* First initialize the container classes.  This is so binary operations
    * added later will work with them.
    */
   if (-1 == _pSLarray_init_slarray ())
     return -1;

   /* DataType_Type */
   if (NULL == (cl = SLclass_allocate_class ("DataType_Type")))
     return -1;
   cl->cl_pop = datatype_pop;
   cl->cl_push = datatype_push;
   cl->cl_dereference = datatype_deref;
   if (-1 == SLclass_register_class (cl, SLANG_DATATYPE_TYPE, sizeof(SLtype),
				     SLANG_CLASS_TYPE_SCALAR))
     return -1;

   return 0;
}

static int register_new_datatype (SLFUTURE_CONST char *name, SLtype type)
{
   return SLns_add_iconstant (NULL, name, SLANG_DATATYPE_TYPE, type);
}

int SLclass_create_synonym (SLFUTURE_CONST char *name, SLtype type)
{
   if (NULL == _pSLclass_get_class (type))
     return -1;

   return register_new_datatype (name, type);
}

int SLclass_register_class (SLang_Class_Type *cl, SLtype type, unsigned int type_size, SLclass_Type class_type)
{
   Class_Table_Type *t;
   SLang_Class_Type **clp;
   char *name;
   int can_binop = 1;		       /* scalar_vector_bin_op should work
					* for all data types.
					*/

   if (type == SLANG_VOID_TYPE)
     clp = find_empty_class_slot (&type, &t);
   else
     clp = alloc_class_slot (type, &t);

   if (clp == NULL)
     {
	_pSLang_verror (SL_APPLICATION_ERROR, "Class type %d already in use", (int) type);
	return -1;
     }

   cl->cl_data_type = type;
   cl->cl_class_type = class_type;
   name = cl->cl_name;

   switch (class_type)
     {
      case SLANG_CLASS_TYPE_MMT:
	if (cl->cl_push == NULL) cl->cl_push = default_push_mmt;
	if (cl->cl_destroy == NULL)
	  return method_undefined_error (type, "destroy", name);
	cl->cl_user_destroy_fun = cl->cl_destroy;
	cl->cl_destroy = default_destroy_user;
	type_size = sizeof (VOID_STAR);
	break;

      case SLANG_CLASS_TYPE_SCALAR:
	if (cl->cl_destroy == NULL) cl->cl_destroy = default_destroy_simple;
	if ((type_size == 0)
	    || (type_size > sizeof (_pSL_Object_Union_Type)))
	  {
	     _pSLang_verror (SL_INVALID_PARM,
			   "Type size for %s not appropriate for SCALAR type",
			   name);
	     return -1;
	  }
	if (cl->cl_pop == NULL)
	  return method_undefined_error (type, "pop", name);
	if (cl->cl_fread == NULL) cl->cl_fread = scalar_fread;
	if (cl->cl_fwrite == NULL) cl->cl_fwrite = scalar_fwrite;
	if (cl->cl_acopy == NULL) cl->cl_acopy = scalar_acopy;
	if (cl->cl_dereference == NULL) cl->cl_dereference = cl->cl_push;
	can_binop = 1;
	break;

      case SLANG_CLASS_TYPE_PTR:
	if (cl->cl_destroy == NULL)
	  return method_undefined_error (type, "destroy", name);
	type_size = sizeof (VOID_STAR);
	break;

      case SLANG_CLASS_TYPE_VECTOR:
	if (cl->cl_destroy == NULL)
	  return method_undefined_error (type, "destroy", name);
	if (cl->cl_pop == NULL)
	  return method_undefined_error (type, "pop", name);
	cl->cl_apop = vector_apop;
	cl->cl_apush = vector_apush;
	cl->cl_adestroy = default_destroy_simple;
	if (cl->cl_fread == NULL) cl->cl_fread = scalar_fread;
	if (cl->cl_fwrite == NULL) cl->cl_fwrite = scalar_fwrite;
	if (cl->cl_acopy == NULL) cl->cl_acopy = scalar_acopy;
	if (cl->cl_dereference == NULL) cl->cl_dereference = cl->cl_push;
	can_binop = 1;
	break;

      default:
	_pSLang_verror (SL_INVALID_PARM, "%s: unknown class type (%d)", name, class_type);
	return -1;
     }

   if (type_size == 0)
     {
	_pSLang_verror (SL_INVALID_PARM, "type size must be non-zero for %s", name);
	return -1;
     }

   if (cl->cl_string == NULL) cl->cl_string = default_string;
   if (cl->cl_acopy == NULL) cl->cl_acopy = default_acopy;
   if (cl->cl_datatype_deref == NULL) cl->cl_datatype_deref = default_datatype_deref;

   if (cl->cl_pop == NULL) cl->cl_pop = default_pop;

   if (cl->cl_push == NULL)
     return method_undefined_error (type, "push", name);

   if (cl->cl_byte_code_destroy == NULL)
     cl->cl_byte_code_destroy = cl->cl_destroy;
   if (cl->cl_push_literal == NULL)
     cl->cl_push_literal = cl->cl_push;

   if (cl->cl_dereference == NULL)
     cl->cl_dereference = default_dereference_object;

   if (cl->cl_apop == NULL) cl->cl_apop = cl->cl_pop;
   if (cl->cl_apush == NULL) cl->cl_apush = cl->cl_push;
   if (cl->cl_adestroy == NULL) cl->cl_adestroy = cl->cl_destroy;
   if (cl->cl_push_intrinsic == NULL) cl->cl_push_intrinsic = cl->cl_push;

   if ((cl->cl_foreach == NULL)
       || (cl->cl_foreach_open == NULL)
       || (cl->cl_foreach_close == NULL))
     {
	cl->cl_foreach = _pSLarray_cl_foreach;
	cl->cl_foreach_open = _pSLarray_cl_foreach_open;
	cl->cl_foreach_close = _pSLarray_cl_foreach_close;
     }

   cl->cl_sizeof_type = type_size;

   if (NULL == (cl->cl_transfer_buf = (VOID_STAR) SLmalloc (type_size)))
     return -1;

   add_class_to_slot (t, clp, cl);

   if (-1 == register_new_datatype (name, type))
     return -1;

   if (cl->cl_cmp != NULL)
     {
	if (-1 == SLclass_add_binary_op (type, type, use_cmp_bin_op, use_cmp_bin_op_result))
	  return -1;
     }
   else if (can_binop
	    && (-1 == SLclass_add_binary_op (type, type, scalar_vector_bin_op, scalar_vector_bin_op_result)))
     return -1;

   cl->cl_anytype_typecast = _pSLanytype_typecast;

   return 0;
}

int SLclass_add_math_op (SLtype type,
			 int (*handler)(int,
					SLtype, VOID_STAR, SLuindex_Type,
					VOID_STAR),
			 int (*result) (int, SLtype, SLtype *))
{
   SLang_Class_Type *cl = _pSLclass_get_class (type);

   cl->cl_math_op = handler;
   cl->cl_math_op_result_type = result;
   return 0;
}

int SLclass_add_binary_op (SLtype a, SLtype b,
			   int (*f) (int,
				     SLtype, VOID_STAR, SLuindex_Type,
				     SLtype, VOID_STAR, SLuindex_Type,
				     VOID_STAR),
			   int (*r) (int, SLtype, SLtype, SLtype *))
{
   SL_OOBinary_Type *ab;
   SLang_Class_Type *cl;

   if ((f == NULL) || (r == NULL)
       || ((a == SLANG_VOID_TYPE) && (b == SLANG_VOID_TYPE)))
     {
	_pSLang_verror (SL_INVALID_PARM, "SLclass_add_binary_op");
	return -1;
     }

   if (NULL == (ab = (SL_OOBinary_Type *) SLmalloc (sizeof(SL_OOBinary_Type))))
     return -1;

   ab->binary_function = f;
   ab->binary_result = r;

   if (a == SLANG_VOID_TYPE)
     {
	cl = _pSLclass_get_class (b);
	ab->data_type = a;
	ab->next = NULL;
	cl->cl_void_binary_this = ab;
     }
   else if (b == SLANG_VOID_TYPE)
     {
	cl = _pSLclass_get_class (a);
	ab->data_type = b;
	ab->next = NULL;
	cl->cl_this_binary_void = ab;
     }
   else
     {
	cl = _pSLclass_get_class (a);
	ab->next = cl->cl_binary_ops;
	ab->data_type = b;
	cl->cl_binary_ops = ab;
     }

   if ((a != SLANG_ARRAY_TYPE)
       && (b != SLANG_ARRAY_TYPE))
     {
	if ((-1 == _pSLarray_add_bin_op (a))
	    || (-1 == _pSLarray_add_bin_op (b)))
	  return -1;
     }

   return 0;
}

int SLclass_add_unary_op (SLtype type,
			  int (*f)(int,
				   SLtype, VOID_STAR, SLuindex_Type,
				   VOID_STAR),
			  int (*r)(int, SLtype, SLtype *))
{
   SLang_Class_Type *cl;

   cl = _pSLclass_get_class (type);
   if ((f == NULL) || (r == NULL))
     {
	_pSLang_verror (SL_INVALID_PARM, "SLclass_add_unary_op");
	return -1;
     }

   cl->cl_unary_op = f;
   cl->cl_unary_op_result_type = r;

   return 0;
}

#if 0
int _pSLclass_add_arith_unary_op (SLtype type,
				 int (*f)(int,
					  SLtype, VOID_STAR, unsigned int,
					  VOID_STAR),
				 int (*r)(int, SLtype, SLtype *))
{
   SLang_Class_Type *cl;

   cl = _pSLclass_get_class (type);
   if ((f == NULL) || (r == NULL))
     {
	_pSLang_verror (SL_INVALID_PARM, "SLclass_add_arith_unary_op");
	return -1;
     }

   cl->cl_arith_unary_op = f;
   cl->cl_arith_unary_op_result_type = r;

   return 0;
}
#endif

int SLclass_add_app_unary_op (SLtype type,
			      int (*f)(int,
				       SLtype, VOID_STAR, SLuindex_Type,
				       VOID_STAR),
			      int (*r)(int, SLtype, SLtype *))
{
   SLang_Class_Type *cl;

   cl = _pSLclass_get_class (type);
   if ((f == NULL) || (r == NULL))
     {
	_pSLang_verror (SL_INVALID_PARM, "SLclass_add_app_unary_op");
	return -1;
     }

   cl->cl_app_unary_op = f;
   cl->cl_app_unary_op_result_type = r;

   return 0;
}

int SLclass_set_pop_function (SLang_Class_Type *cl, int (*f)(SLtype, VOID_STAR))
{
   if (cl == NULL) return -1;
   cl->cl_pop = f;

   return 0;
}

int SLclass_set_push_function (SLang_Class_Type *cl, int (*f)(SLtype, VOID_STAR))
{
   if (cl == NULL) return -1;
   cl->cl_push = f;

   return 0;
}

int SLclass_set_apush_function (SLang_Class_Type *cl, int (*f)(SLtype, VOID_STAR))
{
   if (cl == NULL) return -1;
   cl->cl_apush = f;

   return 0;
}

int SLclass_set_acopy_function (SLang_Class_Type *cl, int (*f)(SLtype, VOID_STAR, VOID_STAR))
{
   if (cl == NULL) return -1;
   cl->cl_acopy = f;

   return 0;
}

int SLclass_set_deref_function (SLang_Class_Type *cl, int (*f)(SLtype, VOID_STAR))
{
   if (cl == NULL) return -1;
   cl->cl_dereference = f;

   return 0;
}

int SLclass_set_eqs_function (SLang_Class_Type *cl, int (*f)(SLtype, VOID_STAR, SLtype, VOID_STAR))
{
   if (cl == NULL) return -1;
   cl->cl_eqs = f;

   return 0;
}

int SLclass_set_length_function (SLang_Class_Type *cl, int (*f)(SLtype, VOID_STAR, SLuindex_Type *))
{
   if (cl == NULL) return -1;
   cl->cl_length = f;

   return 0;
}

extern int SLclass_set_is_container (SLang_Class_Type *cl, int ic)
{
   if (cl == NULL)
     return -1;
   cl->is_container = ic;
   return 0;
}

int SLclass_set_string_function (SLang_Class_Type *cl, char *(*f)(SLtype, VOID_STAR))
{
   if (cl == NULL) return -1;

   cl->cl_string = f;
   return 0;
}

int SLclass_set_destroy_function (SLang_Class_Type *cl, void (*f)(SLtype, VOID_STAR))
{
   if (cl == NULL) return -1;

   cl->cl_destroy = f;
   return 0;
}

int SLclass_set_sget_function (SLang_Class_Type *cl, int (*f)(SLtype, SLFUTURE_CONST char *))
{
   if (cl == NULL) return -1;
   cl->cl_sget = f;
   return 0;
}

int SLclass_set_sput_function (SLang_Class_Type *cl, int (*f)(SLtype, SLFUTURE_CONST char *))
{
   if (cl == NULL) return -1;
   cl->cl_sput = f;
   return 0;
}

int SLclass_set_aget_function (SLang_Class_Type *cl, int (*f)(SLtype, unsigned int))
{
   if (cl == NULL) return -1;
   cl->cl_aget = f;
   return 0;
}

int SLclass_set_aput_function (SLang_Class_Type *cl, int (*f)(SLtype, unsigned int))
{
   if (cl == NULL) return -1;
   cl->cl_aput = f;
   return 0;
}

int SLclass_set_anew_function (SLang_Class_Type *cl, int (*f)(SLtype, unsigned int))
{
   if (cl == NULL) return -1;
   cl->cl_anew = f;
   return 0;
}

int SLclass_set_aelem_init_function (SLang_Class_Type *cl, int (*f)(SLtype, VOID_STAR))
{
   if (cl == NULL) return -1;
   cl->cl_init_array_object = f;
   return 0;
}

int SLclass_set_foreach_functions (SLang_Class_Type *cl,
				   SLang_Foreach_Context_Type *(*fe_open)(SLtype, unsigned int),
				   int (*fe)(SLtype, SLang_Foreach_Context_Type *),
				   void (*fe_close)(SLtype, SLang_Foreach_Context_Type *))
{
   if (cl == NULL)
     return -1;

   if ((fe_open == NULL) || (fe == NULL) || (fe_close == NULL))
     {
	SLang_set_error (SL_APPLICATION_ERROR);
	return -1;
     }
   cl->cl_foreach_open = fe_open;
   cl->cl_foreach = fe;
   cl->cl_foreach_close = fe_close;

   return 0;
}

/* Misc */
void _pSLclass_type_mismatch_error (SLtype a, SLtype b)
{
   _pSLang_verror (SL_TYPE_MISMATCH, "Expecting %s, found %s",
		 SLclass_get_datatype_name (a),
		 SLclass_get_datatype_name (b));
}

/* */

static int null_binary_fun (int op,
			    SLtype a, VOID_STAR ap, SLuindex_Type na,
			    SLtype b, VOID_STAR bp, SLuindex_Type nb,
			    VOID_STAR cp)
{
   char *ic;
   unsigned int i;
   char c;

   (void) ap; (void) bp;

   switch (op)
     {
      case SLANG_EQ:
	c = (a == b);
	break;

      case SLANG_NE:
	c = (a != b);
	break;

      default:
	return 0;
     }

   if (na > nb) nb = na;
   ic = (char *) cp;
   for (i = 0; i < nb; i++)
     ic[i] = c;

   return 1;
}

static SLCONST char *Unary_Ops[SLANG_UNARY_OP_MAX-SLANG_UNARY_OP_MIN+2] =
{
   "++", "--", "-", "not", "~", "abs", "sign", "sqr", "mul2", "_ispos", "_isneg", "_isnonneg", NULL
};

static SLCONST char *Binary_Ops [SLANG_BINARY_OP_MAX - SLANG_BINARY_OP_MIN + 2] =
{
   "+", "-", "*", "/", "==", "!=", ">", ">=", "<", "<=", "^",
     "or", "and", "&", "|", "xor", "shl", "shr", "mod", NULL
};

static int get_binary_unary_opcode (SLCONST char *name, SLCONST char **tbl, int min_val)
{
   SLCONST char **u;

   u = tbl;
   while (*u != NULL)
     {
	if (0 == strcmp (name, *u))
	  return min_val + (int) (u - tbl);

	u++;
     }

   _pSLang_verror (SL_NOT_IMPLEMENTED,
		 "Binary/Unary function %s is unsupported", name);
   return -1;
}

int _pSLclass_get_unary_opcode (SLCONST char *name)
{
   return get_binary_unary_opcode (name, Unary_Ops, SLANG_UNARY_OP_MIN);
}

int _pSLclass_get_binary_opcode (SLCONST char *name)
{
   return get_binary_unary_opcode (name, Binary_Ops, SLANG_BINARY_OP_MIN);
}

static SLCONST char *get_binary_op_string (int op)
{
   if ((op < SLANG_BINARY_OP_MIN)
       || (op > SLANG_BINARY_OP_MAX))
     return "- ?? -";		       /* Note: -??- is a trigraph (sigh) */
   return Binary_Ops[op - SLANG_BINARY_OP_MIN];
}

int (*_pSLclass_get_binary_fun (int op,
			       SLang_Class_Type *a_cl, SLang_Class_Type *b_cl,
			       SLang_Class_Type **c_cl, int do_error))
(int,
 SLtype, VOID_STAR, SLuindex_Type,
 SLtype, VOID_STAR, SLuindex_Type,
 VOID_STAR)
{
   SL_OOBinary_Type *bt;
   SL_OOBinary_Type *last;
   SLtype a, b, c;

   a = a_cl->cl_data_type;
   b = b_cl->cl_data_type;

   if ((a == SLANG_NULL_TYPE) || (b == SLANG_NULL_TYPE))
     {
	*c_cl = _pSLclass_get_class (SLANG_CHAR_TYPE);
	return &null_binary_fun;
     }
   bt = a_cl->cl_binary_ops;
   last = NULL;

   while (bt != NULL)
     {
	if (bt->data_type == b)
	  break;

	last = bt;
	bt = bt->next;
     }

   if ((last != NULL) && (bt != NULL))
     {
	last->next = bt->next;
	bt->next = a_cl->cl_binary_ops;
	a_cl->cl_binary_ops = bt;
     }

   /* Did find find any specific function, so look for a more generic match */
   if ((bt != NULL)
       || (NULL != (bt = a_cl->cl_this_binary_void))
       || (NULL != (bt = b_cl->cl_void_binary_this)))
     {
	if (1 == (*bt->binary_result)(op, a, b, &c))
	  {
	     if (c == a) *c_cl = a_cl;
	     else if (c == b) *c_cl = b_cl;
	     else *c_cl = _pSLclass_get_class (c);

	     return bt->binary_function;
	  }
     }

   if (do_error)
     _pSLang_verror (SL_TYPE_MISMATCH, "%s %s %s is not possible",
		   a_cl->cl_name, get_binary_op_string (op), b_cl->cl_name);

   *c_cl = NULL;
   return NULL;
}

int (*_pSLclass_get_unary_fun (int op,
			      SLang_Class_Type *a_cl,
			      SLang_Class_Type **b_cl,
			      int utype))
(int, SLtype, VOID_STAR, SLuindex_Type, VOID_STAR)
{
   int (*f)(int, SLtype, VOID_STAR, SLuindex_Type, VOID_STAR);
   int (*r)(int, SLtype, SLtype *);
   SLtype a;
   SLtype b;

   switch (utype)
     {
      case SLANG_BC_ARITH_UNARY:
      case SLANG_BC_UNARY:
	f = a_cl->cl_unary_op;
	r = a_cl->cl_unary_op_result_type;
	break;

      case SLANG_BC_MATH_UNARY:
	f = a_cl->cl_math_op;
	r = a_cl->cl_math_op_result_type;
	break;

      case SLANG_BC_APP_UNARY:
	f = a_cl->cl_app_unary_op;
	r = a_cl->cl_app_unary_op_result_type;
	break;

      default:
	f = NULL;
	r = NULL;
     }

   a = a_cl->cl_data_type;
   if ((f != NULL) && (r != NULL) && (1 == (*r) (op, a, &b)))
     {
	if (a == b)
	  *b_cl = a_cl;
	else
	  *b_cl = _pSLclass_get_class (b);
	return f;
     }

   _pSLang_verror (SL_TYPE_MISMATCH, "undefined unary operation/function on %s",
		 a_cl->cl_name);

   *b_cl = NULL;

   return NULL;
}

int
SLclass_typecast (SLtype to_type, int is_implicit, int allow_array)
{
   SLtype from_type;
   SLang_Class_Type *cl_to, *cl_from;
   SLang_Object_Type obj;
   VOID_STAR ap;
   VOID_STAR bp;
   int status;

   if (-1 == SLang_pop (&obj))
     return -1;

   from_type = obj.o_data_type;
   if (from_type == to_type)
     return SLang_push (&obj);

   cl_from = _pSLclass_get_class (from_type);
   cl_to = _pSLclass_get_class (to_type);

   /* Check for alias, e.g., int and long */
   if (cl_from == cl_to)
     {
	obj.o_data_type = to_type;
	return SLang_push (&obj);
     }

   /* Since the typecast functions are designed to work on arrays,
    * get the pointer to the value instead of just &obj.v.
    */
   ap = _pSLclass_get_ptr_to_value (cl_from, &obj);

   if ((from_type == SLANG_ARRAY_TYPE)
       && (allow_array || (to_type != SLANG_ANY_TYPE)))
     {
	if (allow_array == 0)
	  goto return_error;

	cl_to = _pSLclass_get_class (SLANG_ARRAY_TYPE);
	bp = cl_to->cl_transfer_buf;
	status = _pSLarray_typecast (from_type, ap, 1, to_type, bp, is_implicit);
     }
   else
     {
	int (*t) (SLtype, VOID_STAR, SLuindex_Type, SLtype, VOID_STAR);

	if (NULL == (t = _pSLclass_get_typecast (from_type, to_type, is_implicit)))
	  {
	     SLang_free_object (&obj);
	     return -1;
	  }

	bp = cl_to->cl_transfer_buf;
	status = (*t) (from_type, ap, 1, to_type, bp);
     }

   if (1 == status)
     {
	/* AnyType apush will do a reference, which is undesirable here.
	 * So, to avoid that, perform push instead of apush.  Yes, this is
	 * an ugly hack.
	 */
	if (to_type == SLANG_ANY_TYPE)
	  status = (*cl_to->cl_push)(to_type, bp);
	else
	  status = (*cl_to->cl_apush)(to_type, bp);

	if (status == -1)
	  {
	     (*cl_to->cl_adestroy) (to_type, bp);
	     SLang_free_object (&obj);
	     return -1;
	  }

	/* cl_apush will push a copy, so destry this one */
	(*cl_to->cl_adestroy) (to_type, bp);
	SLang_free_object (&obj);
	return 0;
     }

   return_error:

   _pSLang_verror (SL_TYPE_MISMATCH, "Unable to typecast %s to %s",
		 cl_from->cl_name,
		 SLclass_get_datatype_name (to_type));
   SLang_free_object (&obj);
   return -1;
}

int (*_pSLclass_get_typecast (SLtype from, SLtype to, int is_implicit))
(SLtype, VOID_STAR, SLuindex_Type,
 SLtype, VOID_STAR)
{
   SL_Typecast_Type *t;
   SLang_Class_Type *cl_from;

   cl_from = _pSLclass_get_class (from);

   t = cl_from->cl_typecast_funs;
   while (t != NULL)
     {
	if (t->data_type != to)
	  {
	     t = t->next;
	     continue;
	  }

	if (is_implicit && (t->allow_implicit == 0))
	  break;

	return t->typecast;
     }

   if (to == SLANG_ANY_TYPE)
     return &_pSLanytype_typecast;

   if ((is_implicit == 0)
       && (cl_from->cl_void_typecast != NULL))
     return cl_from->cl_void_typecast;

   _pSLang_verror (SL_TYPE_MISMATCH, "Unable to typecast %s to %s",
		 cl_from->cl_name,
		 SLclass_get_datatype_name (to));

   return NULL;
}

int
SLclass_add_typecast (SLtype from, SLtype to,
		      int (*f)_PROTO((SLtype, VOID_STAR, SLuindex_Type,
				      SLtype, VOID_STAR)),
		      int allow_implicit)
{
   SL_Typecast_Type *t;
   SLang_Class_Type *cl;

   cl = _pSLclass_get_class (from);
   if (to == SLANG_VOID_TYPE)
     {
	cl->cl_void_typecast = f;
	return 0;
     }

   (void) _pSLclass_get_class (to);

   if (NULL == (t = (SL_Typecast_Type *) SLmalloc (sizeof (SL_Typecast_Type))))
     return -1;

   SLMEMSET((char *) t, 0, sizeof(SL_Typecast_Type));
   t->data_type = to;
   t->next = cl->cl_typecast_funs;
   t->typecast = f;
   t->allow_implicit = allow_implicit;

   cl->cl_typecast_funs = t;

   return 0;
}

SLang_MMT_Type *SLang_pop_mmt (SLtype type) /*{{{*/
{
   SLang_MMT_Type *mmt;
   SLang_Class_Type *cl;

   cl = lookup_class (type);
   if (cl == NULL)
     {
	_pSLang_verror (SL_Application_Error, "SLtype %d is not registered", type);
	return NULL;
     }
   if (cl->cl_class_type != SLANG_CLASS_TYPE_MMT)
     {
	_pSLang_verror (SL_Application_Error, "SLtype %d is not an MMT", type);
	return NULL;
     }

   if (-1 == SLclass_pop_ptr_obj (type, VOID_STAR_STAR(&mmt)))
     mmt = NULL;
   return mmt;

#if 0
   SLang_Object_Type obj;
   SLang_Class_Type *cl;

   if (_pSLang_pop_object_of_type (type, &obj))
     return NULL;

   cl = _pSLclass_get_class (type);
   if ((cl->cl_class_type == SLANG_CLASS_TYPE_MMT)
       && (obj.data_type == type))
     {
	return obj.v.ref;
     }

   _pSLclass_type_mismatch_error (type, obj.data_type);
   SLang_free_object (&obj);
   return NULL;
#endif
}

/*}}}*/

int SLang_push_mmt (SLang_MMT_Type *ref) /*{{{*/
{
   if (ref == NULL)
     return SLang_push_null ();

   ref->count += 1;

   if (0 == SLclass_push_ptr_obj (ref->data_type, (VOID_STAR) ref))
     return 0;

   ref->count -= 1;
   return -1;
}

/*}}}*/

void SLang_inc_mmt (SLang_MMT_Type *ref)
{
   if (ref != NULL)
     ref->count += 1;
}

VOID_STAR SLang_object_from_mmt (SLang_MMT_Type *ref)
{
   if (ref == NULL)
     return NULL;

   return ref->user_data;
}

SLang_MMT_Type *SLang_create_mmt (SLtype t, VOID_STAR p)
{
   SLang_MMT_Type *ref;

   (void) _pSLclass_get_class (t);      /* check to see if it is registered */

   if (NULL == (ref = (SLang_MMT_Type *) SLmalloc (sizeof (SLang_MMT_Type))))
     return NULL;

   SLMEMSET ((char *) ref, 0, sizeof (SLang_MMT_Type));

   ref->data_type = t;
   ref->user_data = p;
   /* FIXME!!  To be consistent with other types, the reference count should
    * be set to 1 here.  However, doing so will require other code changes
    * involving the use of MMTs.  For instance, SLang_free_mmt would have
    * to be called after every push of the MMT.
    */
   return ref;
}

void SLang_free_mmt (SLang_MMT_Type *ref)
{
   SLtype type;
   SLang_Class_Type *cl;

   if (ref == NULL)
     return;

   /* This can be zero if SLang_create_mmt is called followed
    * by this routine before anything gets a chance to attach itself
    * to it.
    */
   if (ref->count > 1)
     {
	ref->count -= 1;
	return;
     }

   type = ref->data_type;
   cl = _pSLclass_get_class (type);
   (*cl->cl_user_destroy_fun) (type, ref->user_data);
   SLfree ((char *)ref);
}

int SLang_push_value (SLtype type, VOID_STAR v)
{
   SLang_Class_Type *cl;

   cl = _pSLclass_get_class (type);
   return (*cl->cl_apush)(type, v);
}

int SLang_pop_value (SLtype type, VOID_STAR v)
{
   SLang_Class_Type *cl;

   cl = _pSLclass_get_class (type);
   return (*cl->cl_apop)(type, v);
}

void SLang_free_value (SLtype type, VOID_STAR v)
{
   SLang_Class_Type *cl;

   cl = _pSLclass_get_class (type);
   (*cl->cl_adestroy) (type, v);
}

/* These routines are very low-level and are designed for application data
 * types to access the stack from their push/pop methods.  The int and
 * pointer versions are in slang.c
 */
#if SLANG_HAS_FLOAT
int SLclass_push_float_obj (SLtype type, float x)
{
   SLang_Object_Type obj;
   obj.o_data_type = type;
   obj.v.float_val = x;
   return SLang_push (&obj);
}
#endif

int SLclass_push_long_obj (SLtype type, long x)
{
   SLang_Object_Type obj;
   obj.o_data_type = type;
   obj.v.long_val = x;
   return SLang_push (&obj);
}

#ifdef HAVE_LONG_LONG
int SLclass_push_llong_obj (SLtype type, long long x)
{
   SLang_Object_Type obj;
   obj.o_data_type = type;
   obj.v.llong_val = x;
   return SLang_push (&obj);
}
#endif

int SLclass_push_short_obj (SLtype type, short x)
{
   SLang_Object_Type obj;
   obj.o_data_type = type;
   obj.v.short_val = x;
   return SLang_push (&obj);
}

#if SLANG_HAS_FLOAT
int SLclass_pop_double_obj (SLtype type, double *x)
{
   SLang_Object_Type obj;

   if (-1 == _pSLang_pop_object_of_type (type, &obj, 0))
     return -1;

   *x = obj.v.double_val;
   return 0;
}

int SLclass_pop_float_obj (SLtype type, float *x)
{
   SLang_Object_Type obj;

   if (-1 == _pSLang_pop_object_of_type (type, &obj, 0))
     return -1;

   *x = obj.v.float_val;
   return 0;
}
#endif

int SLclass_pop_long_obj (SLtype type, long *x)
{
   SLang_Object_Type obj;

   if (-1 == _pSLang_pop_object_of_type (type, &obj, 0))
     return -1;

   *x = obj.v.long_val;
   return 0;
}

int SLclass_pop_int_obj (SLtype type, int *x)
{
   SLang_Object_Type obj;

   if (-1 == _pSLang_pop_object_of_type (type, &obj, 0))
     return -1;

   *x = obj.v.int_val;
   return 0;
}

int SLclass_pop_short_obj (SLtype type, short *x)
{
   SLang_Object_Type obj;

   if (-1 == _pSLang_pop_object_of_type (type, &obj, 0))
     return -1;

   *x = obj.v.short_val;
   return 0;
}

int SLclass_pop_char_obj (SLtype type, char *x)
{
   SLang_Object_Type obj;

   if (-1 == _pSLang_pop_object_of_type (type, &obj, 0))
     return -1;

   *x = obj.v.char_val;
   return 0;
}

SLtype SLang_get_int_type (int nbits)
{
   switch (nbits)
     {
      case -8:
	return SLANG_CHAR_TYPE;
      case 8:
	return SLANG_UCHAR_TYPE;
      case -16:
	return _pSLANG_INT16_TYPE;
      case 16:
	return _pSLANG_UINT16_TYPE;
      case -32:
	return _pSLANG_INT32_TYPE;
      case 32:
	return _pSLANG_UINT32_TYPE;
      case -64:
	return _pSLANG_INT64_TYPE;
      case 64:
	return _pSLANG_UINT64_TYPE;
     }
   return 0;
}

int SLang_get_int_size (SLtype type)
{
   switch (type)
     {
      case 0:
	return 0;
      case SLANG_CHAR_TYPE:
	return -8;
      case SLANG_UCHAR_TYPE:
	return 8;
      case _pSLANG_INT16_TYPE:
	return -16;
      case _pSLANG_UINT16_TYPE:
	return 16;
      case _pSLANG_INT32_TYPE:
	return -32;
      case _pSLANG_UINT32_TYPE:
	return 32;
      default:
	if (type == _pSLANG_INT64_TYPE)
	  return -64;
	if (type == _pSLANG_UINT64_TYPE)
	  return 64;
     }
   return 0;
}

int SLclass_patch_intrin_fun_table (SLang_Intrin_Fun_Type *table,
				  SLtype *from_types, SLtype *to_types, unsigned int n)
{
   unsigned int i, j;

   for (i = 0; i < n; i++)
     {
	SLang_Intrin_Fun_Type *t = table;
	SLtype dummy = from_types[i];
	SLtype type = to_types[i];

	while (t->name != NULL)
	  {
	     unsigned int nargs = t->num_args;
	     SLtype *args = t->arg_types;

	     for (j = 0; j < nargs; j++)
	       {
		  if (args[j] == dummy)
		    args[j] = type;
	       }

	     /* For completeness */
	     if (t->return_type == dummy)
	       t->return_type = type;
	     t++;
	  }
     }
   return 0;
}
int SLclass_patch_intrin_fun_table1 (SLang_Intrin_Fun_Type *table,
				   SLtype from_type, SLtype to_type)
{
   return SLclass_patch_intrin_fun_table (table, &from_type, &to_type, 1);
}

