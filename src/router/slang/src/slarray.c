/* Array manipulation routines for S-Lang */
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

typedef struct Range_Array_Type SLarray_Range_Array_Type;

struct Range_Array_Type
{
   SLindex_Type first_index;
   SLindex_Type last_index;
   SLindex_Type delta;
   int has_first_index;
   int has_last_index;
   int (*to_linear_fun) (SLang_Array_Type *, SLarray_Range_Array_Type *, VOID_STAR);
};

static SLang_Array_Type *inline_implicit_index_array (SLindex_Type *, SLindex_Type *, SLindex_Type *);

/* Use SLang_pop_array when a linear array is required. */
static int pop_array (SLang_Array_Type **at_ptr, int convert_scalar)
{
   SLang_Array_Type *at;
   SLindex_Type one = 1;
   int type;

   *at_ptr = NULL;
   type = SLang_peek_at_stack ();

   switch (type)
     {
      case -1:
	return -1;

      case SLANG_ARRAY_TYPE:
	return SLclass_pop_ptr_obj (SLANG_ARRAY_TYPE, (VOID_STAR *) at_ptr);

      case SLANG_NULL_TYPE:
	/* convert_scalar = 0; */  /* commented out for 2.0.5 to fix array_map NULL bug */
	/* drop */
      default:
	if (convert_scalar == 0)
	  {
	     SLdo_pop ();
	     _pSLang_verror (SL_TYPE_MISMATCH, "Context requires an array.  Scalar not converted");
	     return -1;
	  }
	break;
     }

   if (NULL == (at = SLang_create_array ((SLtype) type, 0, NULL, &one, 1)))
     return -1;

   if (at->flags & SLARR_DATA_VALUE_IS_POINTER)
     {
	/* The act of creating the array could have initialized the array
	 * with pointers to an object of the type.  For example, this could
	 * happen with user-defined structs.
	 */
	if (*(VOID_STAR *)at->data != NULL)
	  {
	     at->cl->cl_destroy ((SLtype) type, at->data);
	     *(VOID_STAR *) at->data = NULL;
	  }
     }

   if (-1 == at->cl->cl_apop ((SLtype) type, at->data))
     {
	SLang_free_array (at);
	return -1;
     }
   at->flags |= SLARR_DERIVED_FROM_SCALAR;
   *at_ptr = at;

   return 0;
}

static void throw_size_error (int e)
{
   _pSLang_verror (e, "Unable to create a multi-dimensional array of the desired size");
}

static VOID_STAR linear_get_data_addr (SLang_Array_Type *at, SLindex_Type *dims)
{
   size_t ofs;

   if (at->num_dims == 1)
     {
	if (*dims < 0)
	  ofs = (size_t) (*dims + at->dims[0]);
	else
	  ofs = (size_t)*dims;
     }
   else
     {
	unsigned int i;
	SLindex_Type *max_dims = at->dims;
	unsigned int num_dims = at->num_dims;
	ofs = 0;
	for (i = 0; i < num_dims; i++)
	  {
	     size_t new_ofs;
	     SLindex_Type d = dims[i];
	     if (d < 0)
	       d = d + max_dims[i];

	     new_ofs = ofs * (size_t)max_dims [i] + (size_t) d;
	     if ((max_dims[i] != 0)
		 && ((new_ofs - (size_t)d)/max_dims[i] != ofs))
	       {
		  throw_size_error (SL_Index_Error);
		  return NULL;
	       }
	     ofs = new_ofs;
	  }
     }
   if (ofs >= at->num_elements)
     {
	SLang_set_error (SL_Index_Error);
	return NULL;
     }
   return (VOID_STAR) ((char *)at->data + (ofs * at->sizeof_type));
}

_INLINE_
static VOID_STAR get_data_addr (SLang_Array_Type *at, SLindex_Type *dims)
{
   VOID_STAR data;

   data = at->data;
   if (data == NULL)
     {
	_pSLang_verror (SL_UNKNOWN_ERROR, "Array has no data");
	return NULL;
     }

   data = (*at->index_fun) (at, dims);

   if (data == NULL)
     {
	_pSLang_verror (SL_UNKNOWN_ERROR, "Unable to access array element");
	return NULL;
     }

   return data;
}

void _pSLarray_free_array_elements (SLang_Class_Type *cl, VOID_STAR s, SLuindex_Type num)
{
   size_t sizeof_type;
   void (*f) (SLtype, VOID_STAR);
   char *p;
   SLtype type;

   if ((cl->cl_class_type == SLANG_CLASS_TYPE_SCALAR)
       || (cl->cl_class_type == SLANG_CLASS_TYPE_VECTOR))
     return;

   f = cl->cl_destroy;
   sizeof_type = cl->cl_sizeof_type;
   type = cl->cl_data_type;

   p = (char *) s;
   while (num != 0)
     {
	if (NULL != *(VOID_STAR *)p)
	  {
	     (*f) (type, (VOID_STAR)p);
	     *(VOID_STAR *) p = NULL;
	  }
	p += sizeof_type;
	num--;
     }
}

static int destroy_element (SLang_Array_Type *at,
			    SLindex_Type *dims,
			    VOID_STAR data)
{
   data = get_data_addr (at, dims);
   if (data == NULL)
     return -1;

   /* This function should only get called for arrays that have
    * pointer elements.  Do not call the destroy method if the element
    * is NULL.
    */
   if (NULL != *(VOID_STAR *)data)
     {
	(*at->cl->cl_destroy) (at->data_type, data);
	*(VOID_STAR *) data = NULL;
     }
   return 0;
}

/* This function only gets called when a new array is created.  Thus there
 * is no need to destroy the object first.
 */
static int new_object_element (SLang_Array_Type *at,
			       SLindex_Type *dims,
			       VOID_STAR data)
{
   data = get_data_addr (at, dims);
   if (data == NULL)
     return -1;

   return (*at->cl->cl_init_array_object) (at->data_type, data);
}

int _pSLarray_next_index (SLindex_Type *dims, SLindex_Type *max_dims, unsigned int num_dims)
{
   while (num_dims)
     {
	SLindex_Type dims_i;

	num_dims--;

	dims_i = dims [num_dims] + 1;
	if (dims_i < (int) max_dims [num_dims])
	  {
	     dims [num_dims] = dims_i;
	     return 0;
	  }
	dims [num_dims] = 0;
     }

   return -1;
}

static int do_method_for_all_elements (SLang_Array_Type *at,
				       int (*method)(SLang_Array_Type *,
						     SLindex_Type *,
						     VOID_STAR),
				       VOID_STAR client_data)
{
   SLindex_Type dims [SLARRAY_MAX_DIMS];
   SLindex_Type *max_dims;
   unsigned int num_dims;

   if (at->num_elements == 0)
     return 0;

   max_dims = at->dims;
   num_dims = at->num_dims;

   SLMEMSET((char *)dims, 0, sizeof(dims));

   do
     {
	if (-1 == (*method) (at, dims, client_data))
	  return -1;
     }
   while (0 == _pSLarray_next_index (dims, max_dims, num_dims));

   return 0;
}

static void free_array (SLang_Array_Type *at)
{
   unsigned int flags;

   if (at == NULL) return;

   if (at->num_refs > 1)
     {
	at->num_refs -= 1;
	return;
     }

   flags = at->flags;

   if (flags & SLARR_DATA_VALUE_IS_INTRINSIC)
     return;			       /* not to be freed */

   if (flags & SLARR_DATA_VALUE_IS_POINTER)
     (void) do_method_for_all_elements (at, destroy_element, NULL);

   if (at->free_fun != NULL)
     at->free_fun (at);
   else
     SLfree ((char *) at->data);

   SLfree ((char *) at);
}

void SLang_free_array (SLang_Array_Type *at)
{
   free_array (at);
}

SLang_Array_Type *
SLang_create_array1 (SLtype type, int read_only, VOID_STAR data,
		     SLindex_Type *dims, unsigned int num_dims, int no_init)
{
   SLang_Class_Type *cl;
   SLang_Array_Type *at;
   unsigned int i;
   SLuindex_Type num_elements;
   size_t sizeof_type;
   unsigned int size;

   if ((num_dims == 0) || (num_dims > SLARRAY_MAX_DIMS))
     {
	_pSLang_verror (SL_NOT_IMPLEMENTED, "%u dimensional arrays are not supported", num_dims);
	return NULL;
     }

   for (i = 0; i < num_dims; i++)
     {
	if (dims[i] < 0)
	  {
	     _pSLang_verror (SL_INVALID_PARM, "Size of array dim %u is less than 0", i);
	     return NULL;
	  }
     }

   cl = _pSLclass_get_class (type);

   at = (SLang_Array_Type *) SLmalloc (sizeof(SLang_Array_Type));
   if (at == NULL)
     return NULL;

   memset ((char*) at, 0, sizeof(SLang_Array_Type));

   at->data_type = type;
   at->cl = cl;
   at->num_dims = num_dims;
   at->num_refs = 1;

   if (read_only) at->flags = SLARR_DATA_VALUE_IS_READ_ONLY;

   if ((cl->cl_class_type != SLANG_CLASS_TYPE_SCALAR)
       && (cl->cl_class_type != SLANG_CLASS_TYPE_VECTOR))
     at->flags |= SLARR_DATA_VALUE_IS_POINTER;

   num_elements = 1;
   for (i = 0; i < num_dims; i++)
     {
	SLuindex_Type new_num_elements;
	at->dims[i] = dims[i];
	new_num_elements = dims[i] * num_elements;
	if (dims[i] && (new_num_elements/dims[i] != num_elements))
	  {
	     _pSLang_verror (SL_INVALID_PARM, "Unable to create array of the desired size");
	     free_array (at);
	     return NULL;
	  }
	num_elements = new_num_elements;
     }

   /* Now set the rest of the unused dimensions to 1.  This makes it easier
    * when transposing arrays and when indexing arrays derived from scalars.
    */
   while (i < SLARRAY_MAX_DIMS)
     at->dims[i++] = 1;

   at->num_elements = num_elements;
   at->index_fun = linear_get_data_addr;
   at->sizeof_type = sizeof_type = cl->cl_sizeof_type;

   if (data != NULL)
     {
	at->data = data;
	return at;
     }

   size = (unsigned int) (num_elements * sizeof_type);
   if (size/sizeof_type != num_elements)
     {
	throw_size_error (SL_INVALID_PARM);
	free_array (at);
	return NULL;
     }

   if (size == 0) size = 1;

   if (NULL == (data = (VOID_STAR) SLmalloc (size)))
     {
	free_array (at);
	return NULL;
     }

   at->data = data;

   if ((no_init == 0) || (at->flags & SLARR_DATA_VALUE_IS_POINTER))
     memset ((char *) data, 0, size);

   if ((no_init == 0)
       && (cl->cl_init_array_object != NULL)
       && (-1 == do_method_for_all_elements (at, new_object_element, NULL)))
     {
	free_array (at);
	return NULL;
     }

   return at;
}

SLang_Array_Type *
SLang_create_array (SLtype type, int read_only, VOID_STAR data,
		    SLindex_Type *dims, unsigned int num_dims)
{
   return SLang_create_array1 (type, read_only, data, dims, num_dims, 0);
}

int SLang_add_intrinsic_array (SLFUTURE_CONST char *name,
			       SLtype type,
			       int read_only,
			       VOID_STAR data,
			       unsigned int num_dims, ...)
{
   va_list ap;
   unsigned int i;
   SLindex_Type dims[SLARRAY_MAX_DIMS];
   SLang_Array_Type *at;

   if ((num_dims > SLARRAY_MAX_DIMS)
       || (name == NULL)
       || (data == NULL))
     {
	_pSLang_verror (SL_INVALID_PARM, "Unable to create intrinsic array");
	return -1;
     }

   va_start (ap, num_dims);
   for (i = 0; i < num_dims; i++)
     dims [i] = va_arg (ap, int);
   va_end (ap);

   at = SLang_create_array (type, read_only, data, dims, num_dims);
   if (at == NULL)
     return -1;
   at->flags |= SLARR_DATA_VALUE_IS_INTRINSIC;

   /* Note: The variable that refers to the intrinsic array is regarded as
    * read-only.  That way, Array_Name = another_array; will fail.
    */
   if (-1 == SLadd_intrinsic_variable (name, (VOID_STAR) at, SLANG_ARRAY_TYPE, 1))
     {
	free_array (at);
	return -1;
     }
   return 0;
}

static int pop_array_indices (SLindex_Type *dims, unsigned int num_dims)
{
   unsigned int n;
   int i;

   if (num_dims > SLARRAY_MAX_DIMS)
     {
	_pSLang_verror (SL_INVALID_PARM, "Array size not supported");
	return -1;
     }

   n = num_dims;
   while (n != 0)
     {
	n--;
	if (-1 == SLang_pop_integer (&i))
	  return -1;

	dims[n] = i;
     }

   return 0;
}

/* This function gets called via expressions such as Double_Type[10, 20];
 */
static int push_create_new_array (unsigned int num_dims)
{
   SLang_Array_Type *at;
   SLtype type;
   SLindex_Type dims [SLARRAY_MAX_DIMS];
   int (*anew) (SLtype, unsigned int);

   if (-1 == SLang_pop_datatype (&type))
     return -1;

   anew = (_pSLclass_get_class (type))->cl_anew;
   if (anew != NULL)
     return (*anew) (type, num_dims);

   if (-1 == pop_array_indices (dims, num_dims))
     return -1;

   if (NULL == (at = SLang_create_array (type, 0, NULL, dims, num_dims)))
     return -1;

   return SLang_push_array (at, 1);
}

static int push_element_at_addr (SLang_Array_Type *at,
				 VOID_STAR data, int allow_null)
{
   SLang_Class_Type *cl;

   cl = at->cl;
   if ((at->flags & SLARR_DATA_VALUE_IS_POINTER)
       && (*(VOID_STAR *) data == NULL))
     {
	if (allow_null)
	  return SLang_push_null ();

	_pSLang_verror (SL_VARIABLE_UNINITIALIZED,
		      "%s array has uninitialized element", cl->cl_name);
	return -1;
     }

   return (*cl->cl_apush)(at->data_type, data);
}

static int coerse_array_to_linear (SLang_Array_Type *at)
{
   SLarray_Range_Array_Type *range;
   VOID_STAR vdata;
   SLuindex_Type imax;

   /* FIXME: Priority = low.  This assumes that if an array is not linear, then
    * it is a range.
    */
   if (0 == (at->flags & SLARR_DATA_VALUE_IS_RANGE))
     return 0;

   range = (SLarray_Range_Array_Type *) at->data;
   if ((range->has_last_index == 0) || (range->has_first_index == 0))
     {
	_pSLang_verror (SL_INVALID_PARM, "Invalid context for a range array of indeterminate size");
	return -1;
     }

   imax = at->num_elements;
   
   vdata = (VOID_STAR) _SLcalloc (imax, at->sizeof_type);
   if (vdata == NULL)
     return -1;
   (void) (*range->to_linear_fun)(at, range, vdata);
   SLfree ((char *) range);
   at->data = (VOID_STAR) vdata;
   at->flags &= ~SLARR_DATA_VALUE_IS_RANGE;
   at->index_fun = linear_get_data_addr;
   return 0;
}

static void
free_index_objects (SLang_Object_Type *index_objs, unsigned int num_indices)
{
   unsigned int i;
   SLang_Object_Type *obj;

   for (i = 0; i < num_indices; i++)
     {
	obj = index_objs + i;
	if (obj->o_data_type != 0)
	  SLang_free_object (obj);
     }
}

/* If *is_index_array!=0, then only one index object is returned, which is
 * to index all the elements, and not just a single dimension.
 */
static int
pop_indices (unsigned num_dims, SLindex_Type *dims, SLuindex_Type num_elements,
	     SLang_Object_Type *index_objs, unsigned int num_indices,
	     int *is_index_array)
{
   unsigned int i;

   memset((char *) index_objs, 0, num_indices * sizeof (SLang_Object_Type));

   *is_index_array = 0;

   if (num_indices != num_dims)
     {
	if (num_indices != 1)	       /* when 1, it is an index array */
	  {
	     _pSLang_verror (SL_INVALID_PARM, "wrong number of indices for array");
	     return -1;
	  }
     }

   i = num_indices;
   while (i != 0)
     {
	SLang_Object_Type *obj;
	SLtype data_type;
	SLang_Array_Type *at;

	i--;
	obj = index_objs + i;
	if (SLANG_ARRAY_TYPE != _pSLang_peek_at_stack2 (&data_type))
	  {
	     if (-1 == _pSLang_pop_object_of_type (SLANG_ARRAY_INDEX_TYPE, obj, 0))
	       goto return_error;

	     continue;
	  }
	if (data_type != SLANG_ARRAY_INDEX_TYPE)
	  {
	     if (-1 == SLclass_typecast (SLANG_ARRAY_INDEX_TYPE, 1, 1))
	       return -1;
	  }
	if (-1 == SLang_pop (obj))
	  goto return_error;

	at = obj->v.array_val;
	if (at->flags & SLARR_DATA_VALUE_IS_RANGE)
	  {
	     SLarray_Range_Array_Type *r = (SLarray_Range_Array_Type *) at->data;
	     if ((r->has_last_index == 0) || (r->has_first_index == 0))
	       {
		  /* Cases to consider (positive increment)
		   *   [:]  ==> [0:n-1] all elements
		   *   [:i] ==> [0:i] for i>=0, [0:n+i] for i<0
		   *   [i:] ==> [i:n-1] for i>=0, [i+n:n-1] for i<0
		   * This will allow: [:-3] to index all but last 3, etc.
		   * Also consider cases with a negative increment:
		   *   [::-1] = [n-1,n-2,...0] = [n-1:0:-1]
		   *   [:i:-1] = [n-1,n-2,..i] = [n-1:i:-1]
		   *   [i::-1] = [i,i-1,...0] = [i:0:-1]
		   */
		  SLang_Array_Type *new_at;
		  SLindex_Type first_index, last_index;
		  SLindex_Type delta = r->delta;
		  SLindex_Type n;

		  if (num_indices == 1)/* could be index array */
		    n = (SLindex_Type)num_elements;
		  else
		    n = dims[i];

		  if (r->has_first_index)
		    {
		       /* Case 3 */
		       first_index = r->first_index;
		       if (first_index < 0) first_index += n;
		       if (delta > 0) last_index = n-1; else last_index = 0;
		    }
		  else if (r->has_last_index)
		    {
		       /* case 2 */
		       if (delta > 0) first_index = 0; else first_index = n-1;
		       last_index = r->last_index;
		       if (last_index < 0)
			 last_index += n;
		    }
		  else
		    {
		       /* case 0 */
		       if (delta > 0)
			 {
			    first_index = 0;
			    last_index = n - 1;
			 }
		       else
			 {
			    first_index = n-1;
			    last_index = 0;
			 }
		    }

		  if (NULL == (new_at = inline_implicit_index_array (&first_index, &last_index, &delta)))
		    goto return_error;

		  free_array (at);
		  obj->v.array_val = new_at;
	       }
	  }
	if (num_indices == 1)
	  {
	     *is_index_array = 1;
	     return 0;
	  }
     }
   return 0;

   return_error:
   free_index_objects (index_objs, num_indices);
   return -1;
}

static void do_index_error (unsigned int i, SLindex_Type indx, SLindex_Type dim)
{
   _pSLang_verror (SL_Index_Error, "Array index %u (value=%ld) out of allowed range 0<=index<%ld",
		 i, (long)indx, (long)dim);
}

int _pSLarray_pop_index (unsigned int num_elements, SLang_Array_Type **ind_atp, SLindex_Type *ind)
{
   SLang_Object_Type index_obj;
   SLindex_Type dims;
   int is_index_array = 0;
   SLang_Array_Type *ind_at;

   *ind_atp = NULL;
   dims = (SLindex_Type) num_elements;
   if (dims < 0)
     {
	SLang_verror (SL_Index_Error, "Object is too large to be indexed");
	return -1;
     }

   if (-1 == pop_indices (1, &dims, num_elements, &index_obj, 1, &is_index_array))
     return -1;

   if (index_obj.o_data_type != SLANG_ARRAY_TYPE)
     {
	*ind = index_obj.v.index_val;
	return 0;
     }

   ind_at = index_obj.v.array_val;

   if (-1 == coerse_array_to_linear (ind_at))
     {
	SLang_free_array (ind_at);
	return -1;
     }
   *ind_atp = ind_at;
   return 0;
}

static int
transfer_n_elements (SLang_Array_Type *at, VOID_STAR dest_data, VOID_STAR src_data,
		     size_t sizeof_type, SLuindex_Type n, int is_ptr)
{
   SLtype data_type;
   SLang_Class_Type *cl;

   if (is_ptr == 0)
     {
	SLMEMCPY ((char *) dest_data, (char *)src_data, n * sizeof_type);
	return 0;
     }

   data_type = at->data_type;
   cl = at->cl;

   while (n != 0)
     {
	if (*(VOID_STAR *)dest_data != NULL)
	  {
	     (*cl->cl_destroy) (data_type, dest_data);
	     *(VOID_STAR *) dest_data = NULL;
	  }

	if (*(VOID_STAR *) src_data == NULL)
	  *(VOID_STAR *) dest_data = NULL;
	else
	  {
	     if (-1 == (*cl->cl_acopy) (data_type, src_data, dest_data))
	       /* No need to destroy anything */
	       return -1;
	  }

	src_data = (VOID_STAR) ((char *)src_data + sizeof_type);
	dest_data = (VOID_STAR) ((char *)dest_data + sizeof_type);

	n--;
     }

   return 0;
}

_INLINE_
int
_pSLarray_aget_transfer_elem (SLang_Array_Type *at, SLindex_Type *indices,
			     VOID_STAR new_data, size_t sizeof_type, int is_ptr)
{
   VOID_STAR at_data;

   /* Since 1 element is being transferred, there is no need to coerce
    * the array to linear.
    */
   if (NULL == (at_data = get_data_addr (at, indices)))
     return -1;

   if (is_ptr == 0)
     {
	memcpy ((char *) new_data, (char *)at_data, sizeof_type);
	return 0;
     }

   return transfer_n_elements (at, new_data, at_data, sizeof_type, 1, is_ptr);
}

static int
aget_transfer_n_elems (SLang_Array_Type *at, SLindex_Type num, SLindex_Type *start_indices,
		       VOID_STAR new_data, size_t sizeof_type, int is_ptr)
{
   VOID_STAR at_data;
   SLindex_Type i;
   int last_index = (int)at->num_dims-1;
   SLindex_Type indices[SLARRAY_MAX_DIMS];

   if (num == 0)
     return 0;

   for (i = 0; i <= (int) last_index; i++)
     indices[i] = start_indices[i];

   if ((at->data != NULL)
       && (at->index_fun == linear_get_data_addr))
     {
	VOID_STAR addr_start, addr_end;
	if (NULL == (addr_start = linear_get_data_addr (at, indices)))
	  return -1;
	indices[last_index] += (num-1);
	if (NULL == (addr_end = linear_get_data_addr (at, indices)))
	  return -1;

	if (is_ptr == 0)
	  {
	     memcpy ((char *) new_data, (char *)addr_start, num * sizeof_type);
	     return 0;
	  }

	return transfer_n_elements (at, new_data, (char *)addr_start, sizeof_type, num, is_ptr);
     }

   for (i = 0; i < num; i++)
     {
	/* Since 1 element is being transferred, there is no need to coerce
	 * the array to linear.
	 */
	if (NULL == (at_data = get_data_addr (at, indices)))
	  return -1;

	if (is_ptr == 0)
	  memcpy ((char *) new_data, (char *)at_data, sizeof_type);
	else if (-1 == transfer_n_elements (at, new_data, at_data, sizeof_type, 1, is_ptr))
	  return -1;

	new_data = (VOID_STAR) ((char *)new_data + sizeof_type);
	indices[last_index]++;
     }
   return 0;
}

#if SLANG_OPTIMIZE_FOR_SPEED
# if SLANG_HAS_FLOAT
#  define GENERIC_TYPE double
#  define AGET_FROM_INDEX_ARRAY_FUN aget_doubles_from_index_array
#  define APUT_FROM_INDEX_ARRAY_FUN aput_doubles_from_index_array
#  include "slagetput.inc"
#  define GENERIC_TYPE float
#  define AGET_FROM_INDEX_ARRAY_FUN aget_floats_from_index_array
#  define APUT_FROM_INDEX_ARRAY_FUN aput_floats_from_index_array
#  include "slagetput.inc"
# endif

# define GENERIC_TYPE int
# define AGET_FROM_INDEX_ARRAY_FUN aget_ints_from_index_array
# define APUT_FROM_INDEX_ARRAY_FUN aput_ints_from_index_array
# include "slagetput.inc"
# define GENERIC_TYPE long
# define AGET_FROM_INDEX_ARRAY_FUN aget_longs_from_index_array
# define APUT_FROM_INDEX_ARRAY_FUN aput_longs_from_index_array
# include "slagetput.inc"
# define GENERIC_TYPE char
# define AGET_FROM_INDEX_ARRAY_FUN aget_chars_from_index_array
# define APUT_FROM_INDEX_ARRAY_FUN aput_chars_from_index_array
# include "slagetput.inc"
# define GENERIC_TYPE short
# define AGET_FROM_INDEX_ARRAY_FUN aget_shorts_from_index_array
# define APUT_FROM_INDEX_ARRAY_FUN aput_shorts_from_index_array
# include "slagetput.inc"
#endif

static int aget_generic_from_index_array (SLang_Array_Type *at,
					  SLang_Array_Type *at_ind, int is_range,
					  unsigned char *dest_data)
{
   SLindex_Type *indices, *indices_max;
   unsigned char *src_data = (unsigned char *) at->data;
   SLindex_Type num_elements = (SLindex_Type) at->num_elements;
   size_t sizeof_type = at->sizeof_type;
   int is_ptr = at->flags & SLARR_DATA_VALUE_IS_POINTER;

   if (is_range)
     {
	SLarray_Range_Array_Type *r = (SLarray_Range_Array_Type *)at_ind->data;
	SLindex_Type idx = r->first_index, delta = r->delta;
	SLuindex_Type j, jmax = at_ind->num_elements;

	for (j = 0; j < jmax; j++)
	  {
	     size_t offset;
	     SLindex_Type i = idx;

	     if (i < 0)
	       {
		  i += num_elements;
		  if (i < 0)
		    i = num_elements;
	       }
	     if (i >= num_elements)
	       {
		  SLang_set_error (SL_Index_Error);
		  return -1;
	       }
	     offset = sizeof_type * (SLuindex_Type)i;
	     if (-1 == transfer_n_elements (at, (VOID_STAR) dest_data,
					    (VOID_STAR) (src_data + offset),
					    sizeof_type, 1, is_ptr))
	       return -1;

	     dest_data += sizeof_type;
	     idx += delta;
	  }
	return 0;
     }

   /* Since the index array is linear, I can address it directly */
   indices = (SLindex_Type *) at_ind->data;
   indices_max = indices + at_ind->num_elements;
   while (indices < indices_max)
     {
	size_t offset;
	SLindex_Type i = *indices;

	if (i < 0)
	  {
	     i += num_elements;
	     if (i < 0)
	       i = num_elements;
	  }
	if (i >= num_elements)
	  {
	     SLang_set_error (SL_Index_Error);
	     return -1;
	  }

	offset = sizeof_type * (SLuindex_Type)i;
	if (-1 == transfer_n_elements (at, (VOID_STAR) dest_data,
				       (VOID_STAR) (src_data + offset),
				       sizeof_type, 1, is_ptr))
	  return -1;

	dest_data += sizeof_type;
	indices++;
     }
   return 0;
}

/* Here the ind_at index-array is an n-d array of indices.  This function
 * creates an n-d array of made up of values of 'at' at the locations
 * specified by the indices.  The result is pushed.
 */
static int
aget_from_index_array (SLang_Array_Type *at, SLang_Array_Type *ind_at)
{
   SLang_Array_Type *new_at;
   SLindex_Type num_elements;
   unsigned char *new_data, *src_data;
   size_t sizeof_type;
   int is_ptr, is_range;

   if (-1 == coerse_array_to_linear (at))
     return -1;

   is_range = ind_at->flags & SLARR_DATA_VALUE_IS_RANGE;

   if ((is_range == 0)
       && (-1 == coerse_array_to_linear (ind_at)))
     return -1;

   is_ptr = (at->flags & SLARR_DATA_VALUE_IS_POINTER);
   /* Only initialize the elements of the array if it contains pointers.  If
    * an error occurs when filling the array, the array and its elements will
    * be freed.  Hence, to avoid freeing garbage, the array should be initialized
    */
   if (NULL == (new_at = SLang_create_array1 (at->data_type, 0, NULL, ind_at->dims, ind_at->num_dims, !is_ptr)))
     return -1;

   src_data = (unsigned char *) at->data;
   new_data = (unsigned char *) new_at->data;
   num_elements = (SLindex_Type) at->num_elements;
   sizeof_type = new_at->sizeof_type;

   if (num_elements < 0)
     {
	_pSLang_verror (SL_Index_Error, "Array is too large to be indexed using an index-array");
	goto return_error;
     }

   switch (at->data_type)
     {
#if SLANG_OPTIMIZE_FOR_SPEED
# if SLANG_HAS_FLOAT
      case SLANG_DOUBLE_TYPE:
	if (-1 == aget_doubles_from_index_array ((double *)src_data, num_elements,
						 ind_at, is_range, (double *)new_data))
	  goto return_error;
	break;
      case SLANG_FLOAT_TYPE:
	if (-1 == aget_floats_from_index_array ((float *)src_data, num_elements,
						ind_at, is_range, (float *)new_data))
	  goto return_error;
	break;
# endif
      case SLANG_CHAR_TYPE:
      case SLANG_UCHAR_TYPE:
	if (-1 == aget_chars_from_index_array ((char *)src_data, num_elements,
					       ind_at, is_range, (char *)new_data))
	  goto return_error;
	break;
      case SLANG_SHORT_TYPE:
      case SLANG_USHORT_TYPE:
	if (-1 == aget_shorts_from_index_array ((short *)src_data, num_elements,
					      ind_at, is_range, (short *)new_data))
	  goto return_error;
	break;
      case SLANG_INT_TYPE:
      case SLANG_UINT_TYPE:
	if (-1 == aget_ints_from_index_array ((int *)src_data, num_elements,
					      ind_at, is_range, (int *)new_data))
	  goto return_error;
	break;
      case SLANG_LONG_TYPE:
      case SLANG_ULONG_TYPE:
	if (-1 == aget_longs_from_index_array ((long *)src_data, num_elements,
					      ind_at, is_range, (long *)new_data))
	  goto return_error;
	break;
#endif
      default:
	if (-1 == aget_generic_from_index_array (at, ind_at, is_range, new_data))
	  goto return_error;
     }

   return SLang_push_array (new_at, 1);

   return_error:
   free_array (new_at);
   return -1;
}

/* This is extremely ugly.  It is due to the fact that the index_objects
 * may contain ranges.  This is a utility function for the aget/aput
 * routines
 */
static int
convert_nasty_index_objs (SLang_Array_Type *at,
			  SLang_Object_Type *index_objs,
			  unsigned int num_indices,
			  SLindex_Type **index_data,
			  SLindex_Type *range_buf, SLindex_Type *range_delta_buf,
			  SLindex_Type *max_dims,
			  unsigned int *num_elements,
			  int *is_array, int is_dim_array[SLARRAY_MAX_DIMS])
{
   SLuindex_Type i, total_num_elements;
   SLang_Array_Type *ind_at;

   if (num_indices != at->num_dims)
     {
	_pSLang_verror (SL_INVALID_PARM, "Array requires %u indices", at->num_dims);
	return -1;
     }

   *is_array = 0;
   total_num_elements = 1;
   for (i = 0; i < num_indices; i++)
     {
	SLuindex_Type new_total_num_elements;
	SLang_Object_Type *obj = index_objs + i;
	range_delta_buf [i] = 0;

	if (obj->o_data_type == SLANG_ARRAY_INDEX_TYPE)
	  {
	     range_buf [i] = obj->v.index_val;
	     max_dims [i] = 1;
	     index_data[i] = range_buf + i;
	     is_dim_array[i] = 0;
	  }
#if SLANG_ARRAY_INDEX_TYPE != SLANG_INT_TYPE
	else if (obj->o_data_type == SLANG_INT_TYPE)
	  {
	     range_buf [i] = obj->v.index_val;
	     max_dims [i] = 1;
	     index_data[i] = range_buf + i;
	     is_dim_array[i] = 0;
	  }
#endif
	else
	  {
	     *is_array = 1;
	     is_dim_array[i] = 1;
	     ind_at = obj->v.array_val;

	     if (ind_at->flags & SLARR_DATA_VALUE_IS_RANGE)
	       {
		  SLarray_Range_Array_Type *r;

		  r = (SLarray_Range_Array_Type *) ind_at->data;
		  range_buf[i] = r->first_index;
		  range_delta_buf [i] = r->delta;
		  max_dims[i] = (SLindex_Type) ind_at->num_elements;
	       }
	     else
	       {
		  index_data [i] = (SLindex_Type *) ind_at->data;
		  max_dims[i] = (SLindex_Type) ind_at->num_elements;
	       }
	  }

	new_total_num_elements = total_num_elements * max_dims[i];
	if (max_dims[i] && (new_total_num_elements/max_dims[i] != total_num_elements))
	  {
	     throw_size_error (SL_INVALID_PARM);
	     return -1;
	  }
       total_num_elements = new_total_num_elements;
     }

   *num_elements = total_num_elements;
   return 0;
}

/* This routine pushes a 1-d vector of values from 'at' indexed by
 * the objects 'index_objs'.  These objects can either be integers or
 * 1-d integer arrays.  The fact that the 1-d arrays can be ranges
 * makes this look ugly.
 */
static int
aget_from_indices (SLang_Array_Type *at,
		   SLang_Object_Type *index_objs, unsigned int num_indices)
{
   SLindex_Type *index_data [SLARRAY_MAX_DIMS];
   SLindex_Type range_buf [SLARRAY_MAX_DIMS];
   SLindex_Type range_delta_buf [SLARRAY_MAX_DIMS];
   SLindex_Type max_dims [SLARRAY_MAX_DIMS];
   unsigned int i, num_elements;
   SLang_Array_Type *new_at;
   SLindex_Type map_indices[SLARRAY_MAX_DIMS];
   SLindex_Type indices [SLARRAY_MAX_DIMS];
   SLindex_Type *at_dims;
   size_t sizeof_type;
   int is_ptr, ret, is_array;
   char *new_data;
   SLang_Class_Type *cl;
   int is_dim_array[SLARRAY_MAX_DIMS];
   unsigned int last_index;
   SLindex_Type last_index_num;

   if (-1 == convert_nasty_index_objs (at, index_objs, num_indices,
				       index_data, range_buf, range_delta_buf,
				       max_dims, &num_elements, &is_array,
				       is_dim_array))
     return -1;

   is_ptr = (at->flags & SLARR_DATA_VALUE_IS_POINTER);
   sizeof_type = at->sizeof_type;

   /* cl = _pSLclass_get_class (at->data_type); */
   cl = at->cl;

   if ((is_array == 0) && (num_elements == 1))
     {
	new_data = (char *)cl->cl_transfer_buf;
	memset (new_data, 0, sizeof_type);
	new_at = NULL;
     }
   else
     {
	SLindex_Type i_num_elements = (SLindex_Type)num_elements;

	new_at = SLang_create_array (at->data_type, 0, NULL, &i_num_elements, 1);
	if (NULL == new_at)
	  return -1;

	new_data = (char *)new_at->data;

	if (num_elements == 0)
	  goto fixup_dims;
     }

   at_dims = at->dims;
   memset ((char *) map_indices, 0, sizeof(map_indices));

   last_index = num_indices - 1;
   /* if last_index_num is non-zero, then that many can be transferred quickly */
   if ((range_delta_buf[last_index] == 1) && (range_buf[last_index] >= 0))
     last_index_num = max_dims[last_index];
   else
     last_index_num = 0;

   while (1)
     {
	for (i = 0; i < num_indices; i++)
	  {
	     SLindex_Type j = map_indices[i];
	     SLindex_Type indx;

	     if (0 != range_delta_buf[i])
	       indx = range_buf[i] + j * range_delta_buf[i];
	     else
	       indx = index_data [i][j];

	     if (indx < 0)
	       indx += at_dims[i];

	     if ((indx < 0) || (indx >= at_dims[i]))
	       {
		  do_index_error (i, indx, at_dims[i]);
		  free_array (new_at);
		  return -1;
	       }
	     indices[i] = indx;
	  }

	if (last_index_num)
	  {
	     if (-1 == aget_transfer_n_elems (at, last_index_num, indices, (VOID_STAR)new_data, sizeof_type, is_ptr))
	       {
		  free_array (new_at);
		  return -1;
	       }
	     new_data += last_index_num * sizeof_type;
	     map_indices[last_index] = last_index_num;

	     if (0 != _pSLarray_next_index (map_indices, max_dims, num_indices))
	       break;
	  }
	else
	  {
	     if (-1 == _pSLarray_aget_transfer_elem (at, indices, (VOID_STAR)new_data, sizeof_type, is_ptr))
	       {
		  free_array (new_at);
		  return -1;
	       }
	     new_data += sizeof_type;

	     if (num_indices == 1)
	       {
		  map_indices[0]++;
		  if (map_indices[0] == max_dims[0])
		    break;
	       }
	     else if (0 != _pSLarray_next_index (map_indices, max_dims, num_indices))
	       break;
	  }
     }

fixup_dims:

   if (new_at != NULL)
     {
	int num_dims = 0;
	/* Fixup dimensions on array */
	for (i = 0; i < num_indices; i++)
	  {
	     if (is_dim_array[i]) /* was: (max_dims[i] > 1) */
	       {
		  new_at->dims[num_dims] = max_dims[i];
		  num_dims++;
	       }
	  }

	if (num_dims != 0) new_at->num_dims = num_dims;
	return SLang_push_array (new_at, 1);
     }

   /* Here new_data is a whole new copy, so free it after the push */
   new_data -= sizeof_type;
   if (is_ptr && (*(VOID_STAR *)new_data == NULL))
     ret = SLang_push_null ();
   else
     {
	ret = (*cl->cl_apush) (at->data_type, (VOID_STAR)new_data);
	(*cl->cl_adestroy) (at->data_type, (VOID_STAR)new_data);
     }

   return ret;
}

static int push_string_as_array (unsigned char *s, unsigned int len)
{
   SLindex_Type ilen;
   SLang_Array_Type *at;

   ilen = (SLindex_Type) len;

   at = SLang_create_array (SLANG_UCHAR_TYPE, 0, NULL, &ilen, 1);
   if (at == NULL)
     return -1;

   memcpy ((char *)at->data, (char *)s, len);
   return SLang_push_array (at, 1);
}

static int pop_array_as_string (char **sp)
{
   SLang_Array_Type *at;
   int ret;

   *sp = NULL;

   if (-1 == SLang_pop_array_of_type (&at, SLANG_UCHAR_TYPE))
     return -1;

   ret = 0;

   if (NULL == (*sp = SLang_create_nslstring ((char *) at->data, at->num_elements)))
     ret = -1;

   free_array (at);
   return ret;
}

static int pop_array_as_bstring (SLang_BString_Type **bs)
{
   SLang_Array_Type *at;
   int ret;

   *bs = NULL;

   if (-1 == SLang_pop_array_of_type (&at, SLANG_UCHAR_TYPE))
     return -1;

   ret = 0;

   if (NULL == (*bs = SLbstring_create ((unsigned char *) at->data, at->num_elements)))
     ret = -1;

   free_array (at);
   return ret;
}

#if SLANG_OPTIMIZE_FOR_SPEED
/* This routine assumes that the array is 1d */
int _pSLarray1d_push_elem (SLang_Array_Type *at, SLindex_Type idx)
{
   VOID_STAR data;
   char *new_data;
   size_t sizeof_type;
   int is_ptr, ret;
   SLang_Class_Type *cl;

   switch (at->data_type)
     {
      case SLANG_CHAR_TYPE:
	if (NULL == (data = at->index_fun(at, &idx))) return -1;
	return SLclass_push_char_obj (SLANG_CHAR_TYPE, *(char *)data);

      case SLANG_INT_TYPE:
	if (NULL == (data = at->index_fun(at, &idx))) return -1;
	return SLclass_push_int_obj (SLANG_INT_TYPE, *(int *)data);

#if SLANG_HAS_FLOAT
      case SLANG_DOUBLE_TYPE:
	if (NULL == (data = at->index_fun(at, &idx))) return -1;
	return SLclass_push_double_obj (SLANG_DOUBLE_TYPE, *(double *)data);
#endif
     }

   is_ptr = (at->flags & SLARR_DATA_VALUE_IS_POINTER);
   sizeof_type = at->sizeof_type;
   cl = at->cl;
   new_data = (char *)cl->cl_transfer_buf;
   memset (new_data, 0, sizeof_type);

   if (-1 == _pSLarray_aget_transfer_elem (at, &idx, (VOID_STAR)new_data, sizeof_type, is_ptr))
     return -1;

   if (is_ptr && (*(VOID_STAR *)new_data == NULL))
     return SLang_push_null ();

   ret = (*cl->cl_apush) (at->data_type, (VOID_STAR)new_data);
   (*cl->cl_adestroy) (at->data_type, (VOID_STAR)new_data);
   return ret;
}
#endif

static int aget_from_array (unsigned int num_indices)
{
   SLang_Array_Type *at;
   SLang_Object_Type index_objs [SLARRAY_MAX_DIMS];
   int ret;
   int is_index_array, free_indices;
   unsigned int i;

   if (num_indices == 0)
     {
	SLang_set_error (SL_Index_Error);
	return -1;
     }

   if (num_indices > SLARRAY_MAX_DIMS)
     {
	_pSLang_verror (SL_INVALID_PARM, "Number of dims must be less than %d", 1+SLARRAY_MAX_DIMS);
	return -1;
     }

   if (-1 == pop_array (&at, 1))
     return -1;

   /* Allow a scalar to be indexed using any number of indices, e.g.,
    *    x = 2;  a = x[0]; b = x[0,0];
    */
   if ((at->flags & SLARR_DERIVED_FROM_SCALAR)
       && (at->num_refs == 1))
     {
	at->num_dims = num_indices;
     }

   if (-1 == pop_indices (at->num_dims, at->dims, at->num_elements, index_objs, num_indices, &is_index_array))
     {
	free_array (at);
	return -1;
     }
   free_indices = 1;

   if (is_index_array == 0)
     {
#if SLANG_OPTIMIZE_FOR_SPEED
	if ((num_indices == 1)
	    && (index_objs[0].o_data_type == SLANG_ARRAY_INDEX_TYPE)
	    && (at->num_dims == 1))
	  {
	     ret = _pSLarray1d_push_elem (at, index_objs[0].v.index_val);
	     free_indices = 0;
	  }
	else
#endif
	ret = aget_from_indices (at, index_objs, num_indices);
     }
   else
     ret = aget_from_index_array (at, index_objs[0].v.array_val);

   free_array (at);
   if (free_indices)
     {
	for (i = 0; i < num_indices; i++)
	  SLang_free_object (index_objs + i);
     }

   return ret;
}

static int push_string_element (SLtype type, unsigned char *s, unsigned int len)
{
   int i;

   if (SLang_peek_at_stack () == SLANG_ARRAY_TYPE)
     {
	char *str;

	/* The indices are array values.  So, do this: */
	if (-1 == push_string_as_array (s, len))
	  return -1;

	if (-1 == aget_from_array (1))
	  return -1;

	if (type == SLANG_BSTRING_TYPE)
	  {
	     SLang_BString_Type *bs;
	     int ret;

	     if (-1 == pop_array_as_bstring (&bs))
	       return -1;

	     ret = SLang_push_bstring (bs);
	     SLbstring_free (bs);
	     return ret;
	  }

	if (-1 == pop_array_as_string (&str))
	  return -1;
	return _pSLang_push_slstring (str);   /* frees s upon error */
     }

   if (-1 == SLang_pop_integer (&i))
     return -1;

   if (i < 0) i = i + (int)len;
   if ((unsigned int) i > len)
     i = len;			       /* get \0 character --- bstrings include it as well */

   return SLang_push_uchar (s[(unsigned int)i]);
}

/* ARRAY[i, j, k] generates code: __args i j ...k ARRAY __aput/__aget
 * Here i, j, ... k may be a mixture of integers and 1-d arrays, or
 * a single array of indices.  The index array is generated by the
 * 'where' function.
 *
 * If ARRAY is of type DataType, then this function will create an array of
 * the appropriate type.  In that case, the indices i, j, ..., k must be
 * integers.
 */
int _pSLarray_aget1 (unsigned int num_indices)
{
   int type;
   int (*aget_fun) (SLtype, unsigned int);

   type = SLang_peek_at_stack ();
   switch (type)
     {
      case -1:
	return -1;		       /* stack underflow */

      case SLANG_DATATYPE_TYPE:
	return push_create_new_array (num_indices);

      case SLANG_BSTRING_TYPE:
	if (1 == num_indices)
	  {
	     SLang_BString_Type *bs;
	     int ret;
	     unsigned int len;
	     unsigned char *s;

	     if (-1 == SLang_pop_bstring (&bs))
	       return -1;

	     if (NULL == (s = SLbstring_get_pointer (bs, &len)))
	       ret = -1;
	     else
	       ret = push_string_element (type, s, len);

	     SLbstring_free (bs);
	     return ret;
	  }
	break;

      case SLANG_STRING_TYPE:
	if (1 == num_indices)
	  {
	     char *s;
	     int ret;

	     if (-1 == SLang_pop_slstring (&s))
	       return -1;

	     ret = push_string_element (type, (unsigned char *)s, _pSLstring_bytelen (s));
	     _pSLang_free_slstring (s);
	     return ret;
	  }
	break;

      case SLANG_ARRAY_TYPE:
	break;

      case SLANG_ASSOC_TYPE:
	return _pSLassoc_aget (type, num_indices);

      default:
	aget_fun = _pSLclass_get_class (type)->cl_aget;
	if (NULL != aget_fun)
	  return (*aget_fun) (type, num_indices);
     }

   return aget_from_array (num_indices);
}

int _pSLarray_aget (void)
{
   return _pSLarray_aget1 ((unsigned int)(SLang_Num_Function_Args-1));
}

/* A[indices...indices+num] = data... */
static int
aput_transfer_n_elems (SLang_Array_Type *at, SLindex_Type num, SLindex_Type *start_indices,
		       char *data_to_put, SLuindex_Type data_increment, size_t sizeof_type, int is_ptr)
{
   VOID_STAR addr_start;
   SLindex_Type i;
   int last_index = (int)at->num_dims-1;
   SLindex_Type indices[SLARRAY_MAX_DIMS];

   if (num == 0)
     return 0;

   for (i = 0; i <= (int) last_index; i++)
     indices[i] = start_indices[i];

   if ((at->data != NULL)
       && (at->index_fun == linear_get_data_addr))
     {
	VOID_STAR addr_end;
	if (NULL == (addr_start = linear_get_data_addr (at, indices)))
	  return -1;
	indices[last_index] += (num-1);
	if (NULL == (addr_end = linear_get_data_addr (at, indices)))
	  return -1;

	if (is_ptr == 0)
	  {
	     while (addr_start <= addr_end)
	       {
		  memcpy ((char *) addr_start, data_to_put, sizeof_type);
		  data_to_put += data_increment;
		  addr_start = (VOID_STAR) ((char *)addr_start + sizeof_type);
	       }
	     return 0;
	  }

	while (addr_start <= addr_end)
	  {
	     if (-1 == transfer_n_elements (at, addr_start, data_to_put, sizeof_type, 1, is_ptr))
	       return -1;
	     data_to_put += data_increment;
	     addr_start = (VOID_STAR) ((char *)addr_start + sizeof_type);
	  }
	return 0;
     }

   for (i = 0; i < num; i++)
     {
	/* Since 1 element is being transferred, there is no need to coerce
	 * the array to linear.
	 */
	if (NULL == (addr_start = get_data_addr (at, indices)))
	  return -1;

	if (is_ptr == 0)
	  memcpy ((char *) addr_start, data_to_put, sizeof_type);
	else if (-1 == transfer_n_elements (at, addr_start, (VOID_STAR)data_to_put, sizeof_type, 1, is_ptr))
	  return -1;

	data_to_put += data_increment;
	indices[last_index]++;
     }
   return 0;
}

_INLINE_ int
_pSLarray_aput_transfer_elem (SLang_Array_Type *at, SLindex_Type *indices,
			     VOID_STAR data_to_put, size_t sizeof_type, int is_ptr)
{
   VOID_STAR at_data;

   /*
    * A range array is not allowed here.  I should add a check for it.  At
    * the moment, one will not get here.
    */
   if (NULL == (at_data = get_data_addr (at, indices)))
     return -1;

   if (is_ptr == 0)
     {
	memcpy ((char *) at_data, (char *)data_to_put, sizeof_type);
	return 0;
     }

   return transfer_n_elements (at, at_data, data_to_put, sizeof_type, 1, is_ptr);
}

static int
aput_get_data_to_put (SLang_Class_Type *cl, unsigned int num_elements, int allow_array,
		       SLang_Array_Type **at_ptr, char **data_to_put, SLuindex_Type *data_increment)
{
   SLtype data_type;
   int type;
   SLang_Array_Type *at;

   *at_ptr = NULL;

   data_type = cl->cl_data_type;
   type = SLang_peek_at_stack ();

   if ((SLtype)type != data_type)
     {
	if ((type != SLANG_NULL_TYPE)
	    || ((cl->cl_class_type != SLANG_CLASS_TYPE_PTR)
		&& (cl->cl_class_type != SLANG_CLASS_TYPE_MMT)))
	  {
	     if (-1 == SLclass_typecast (data_type, 1, allow_array))
	       return -1;
	  }
	else
	  {
	     /* This bit of code allows, e.g., a[10] = NULL; */
	     *data_increment = 0;
	     *data_to_put = (char *) cl->cl_transfer_buf;
	     *((char **)cl->cl_transfer_buf) = NULL;
	     return SLdo_pop ();
	  }
     }

   if (allow_array
       && (data_type != SLANG_ARRAY_TYPE)
       && (data_type != SLANG_ANY_TYPE)
       && (SLANG_ARRAY_TYPE == SLang_peek_at_stack ()))
     {
	if (-1 == SLang_pop_array (&at, 0))
	  return -1;

	if ((at->num_elements != num_elements)
#if 0
	    || (at->num_dims != 1)
#endif
	    )
	  {
	     _pSLang_verror (SL_Index_Error, "Array size is inappropriate for use with index-array");
	     free_array (at);
	     return -1;
	  }

	*data_to_put = (char *) at->data;
	*data_increment = at->sizeof_type;
	*at_ptr = at;
	return 0;
     }

   *data_increment = 0;
   *data_to_put = (char *) cl->cl_transfer_buf;

   if (-1 == (*cl->cl_apop)(data_type, (VOID_STAR) *data_to_put))
     return -1;

   return 0;
}

static int
aput_from_indices (SLang_Array_Type *at,
		   SLang_Object_Type *index_objs, unsigned int num_indices)
{
   SLindex_Type *index_data [SLARRAY_MAX_DIMS];
   SLindex_Type range_buf [SLARRAY_MAX_DIMS];
   SLindex_Type range_delta_buf [SLARRAY_MAX_DIMS];
   SLindex_Type max_dims [SLARRAY_MAX_DIMS];
   SLindex_Type *at_dims;
   unsigned int i, num_elements;
   SLang_Array_Type *bt;
   SLindex_Type map_indices[SLARRAY_MAX_DIMS];
   SLindex_Type indices [SLARRAY_MAX_DIMS];
   size_t sizeof_type;
   int is_ptr, is_array, ret;
   char *data_to_put;
   SLuindex_Type data_increment;
   SLang_Class_Type *cl;
   int is_dim_array [SLARRAY_MAX_DIMS];
   SLindex_Type last_index_num;
   unsigned int last_index;

   if (-1 == convert_nasty_index_objs (at, index_objs, num_indices,
				       index_data, range_buf, range_delta_buf,
				       max_dims, &num_elements, &is_array,
				       is_dim_array))
     return -1;

   cl = at->cl;

   if (-1 == aput_get_data_to_put (cl, num_elements, is_array,
				    &bt, &data_to_put, &data_increment))
     return -1;

   sizeof_type = at->sizeof_type;
   is_ptr = (at->flags & SLARR_DATA_VALUE_IS_POINTER);

   ret = -1;

   at_dims = at->dims;
   SLMEMSET((char *) map_indices, 0, sizeof(map_indices));

   last_index = num_indices - 1;
   /* if last_index_num is non-zero, then that many can be transferred quickly */
   if ((range_delta_buf[last_index] == 1) && (range_buf[last_index] >= 0))
     last_index_num = max_dims[last_index];
   else
     last_index_num = 0;

   if (num_elements) while (1)
     {
	for (i = 0; i < num_indices; i++)
	  {
	     SLindex_Type j = map_indices[i];
	     SLindex_Type indx;

	     if (0 != range_delta_buf[i])
	       indx = range_buf[i] + j * range_delta_buf[i];
	     else
	       indx = index_data [i][j];

	     if (indx < 0)
	       indx += at_dims[i];

	     if ((indx < 0) || (indx >= at_dims[i]))
	       {
		  do_index_error (i, indx, at_dims[i]);
		  goto return_error;
	       }
	     indices[i] = indx;
	  }

	if (last_index_num)
	  {
	     if (-1 == aput_transfer_n_elems (at, last_index_num, indices,
					      data_to_put, data_increment,
					      sizeof_type, is_ptr))
	       goto return_error;

	     data_to_put += last_index_num * data_increment;
	     map_indices[last_index] = last_index_num;

	     if (0 != _pSLarray_next_index (map_indices, max_dims, num_indices))
	       break;
	  }
	else
	  {
	     if (-1 == _pSLarray_aput_transfer_elem (at, indices, (VOID_STAR)data_to_put, sizeof_type, is_ptr))
	       goto return_error;

	     data_to_put += data_increment;
	     if (num_indices == 1)
	       {
		  map_indices[0]++;
		  if (map_indices[0] == max_dims[0])
		    break;
	       }
	     else if (0 != _pSLarray_next_index (map_indices, max_dims, num_indices))
	       break;
	  }
     }

   ret = 0;

   /* drop */

   return_error:
   if (bt == NULL)
     {
	if (is_ptr)
	  (*cl->cl_destroy) (cl->cl_data_type, (VOID_STAR) data_to_put);
     }
   else free_array (bt);

   return ret;
}

static int
  aput_generic_from_index_array (char *src_data,
				 SLuindex_Type data_increment,
				 SLang_Array_Type *ind_at, int is_range,
				 SLang_Array_Type *dest_at)
{
   SLindex_Type num_elements = (SLindex_Type) dest_at->num_elements;
   size_t sizeof_type = dest_at->sizeof_type;
   int is_ptr = dest_at->flags & SLARR_DATA_VALUE_IS_POINTER;
   char *dest_data = (char *)dest_at->data;
   SLindex_Type *indices, *indices_max;

   if (is_range)
     {
	SLarray_Range_Array_Type *r = (SLarray_Range_Array_Type *)ind_at->data;
	SLindex_Type idx = r->first_index, delta = r->delta;
	SLuindex_Type j, jmax = ind_at->num_elements;

	for (j = 0; j < jmax; j++)
	  {
	     size_t offset;
	     SLindex_Type i = idx;

	     if (i < 0)
	       {
		  i += num_elements;
		  if (i < 0)
		    i = num_elements;
	       }
	     if (i >= num_elements)
	       {
		  SLang_set_error (SL_Index_Error);
		  return -1;
	       }
	     offset = sizeof_type * (SLuindex_Type)i;
	     if (-1 == transfer_n_elements (dest_at, (VOID_STAR) (dest_data + offset),
					    (VOID_STAR) src_data, sizeof_type,
					    1, is_ptr))
	       return -1;

	     src_data += data_increment;
	     idx += delta;
	  }
	return 0;
     }

   /* Since the index array is linear, I can address it directly */
   indices = (SLindex_Type *) ind_at->data;
   indices_max = indices + ind_at->num_elements;

   while (indices < indices_max)
     {
	size_t offset;
	SLindex_Type i = *indices;

	if (i < 0)
	  {
	     i += num_elements;
	     if (i < 0)
	       i = num_elements;
	  }
	if (i >= num_elements)
	  {
	     SLang_set_error (SL_Index_Error);
	     return -1;
	  }

	offset = sizeof_type * (SLuindex_Type)i;

	if (-1 == transfer_n_elements (dest_at, (VOID_STAR) (dest_data + offset),
				       (VOID_STAR) src_data, sizeof_type, 1,
				       is_ptr))
	  return -1;

	indices++;
	src_data += data_increment;
     }

   return 0;
}

static int
aput_from_index_array (SLang_Array_Type *at, SLang_Array_Type *ind_at)
{
   size_t sizeof_type;
   char *data_to_put, *dest_data;
   SLuindex_Type data_increment;
   SLindex_Type num_elements;
   int is_ptr, is_range;
   SLang_Array_Type *bt;
   SLang_Class_Type *cl;
   int ret;

   if (-1 == coerse_array_to_linear (at))
     return -1;

   is_range = ind_at->flags & SLARR_DATA_VALUE_IS_RANGE;

   if ((is_range == 0)
       && (-1 == coerse_array_to_linear (ind_at)))
     return -1;

   sizeof_type = at->sizeof_type;

   cl = at->cl;

   /* Note that if bt is returned as non NULL, then the array is a linear
    * one.
    */
   if (-1 == aput_get_data_to_put (cl, ind_at->num_elements, 1,
				    &bt, &data_to_put, &data_increment))
     return -1;

   is_ptr = (at->flags & SLARR_DATA_VALUE_IS_POINTER);
   dest_data = (char *) at->data;
   num_elements = (SLindex_Type) at->num_elements;

   ret = -1;
   switch (at->data_type)
     {
#if SLANG_OPTIMIZE_FOR_SPEED
# if SLANG_HAS_FLOAT
      case SLANG_DOUBLE_TYPE:
	if (-1 == aput_doubles_from_index_array (data_to_put, data_increment,
						 ind_at, is_range,
						 (double*)dest_data, num_elements))
	  goto return_error;
	break;

      case SLANG_FLOAT_TYPE:
	if (-1 == aput_floats_from_index_array (data_to_put, data_increment,
						ind_at, is_range,
						(float*)dest_data, num_elements))
	  goto return_error;
	break;
# endif

      case SLANG_CHAR_TYPE:
      case SLANG_UCHAR_TYPE:
	if (-1 == aput_chars_from_index_array (data_to_put, data_increment,
					       ind_at, is_range,
					       (char*)dest_data, num_elements))
	  goto return_error;
	break;
      case SLANG_SHORT_TYPE:
      case SLANG_USHORT_TYPE:
	if (-1 == aput_shorts_from_index_array (data_to_put, data_increment,
						ind_at, is_range,
						(short*)dest_data, num_elements))
	  goto return_error;
	break;

      case SLANG_INT_TYPE:
      case SLANG_UINT_TYPE:
	if (-1 == aput_ints_from_index_array (data_to_put, data_increment,
					      ind_at, is_range,
					      (int*)dest_data, num_elements))
	  goto return_error;
	break;
      case SLANG_LONG_TYPE:
      case SLANG_ULONG_TYPE:
	if (-1 == aput_longs_from_index_array (data_to_put, data_increment,
					      ind_at, is_range,
					      (long*)dest_data, num_elements))
	  goto return_error;
	break;
#endif
      default:
	if (-1 == aput_generic_from_index_array (data_to_put, data_increment,
						 ind_at, is_range, at))
	  goto return_error;
     }

   ret = 0;
   /* Drop */

   return_error:

   if (bt == NULL)
     {
	if (is_ptr)
	  (*cl->cl_destroy) (cl->cl_data_type, (VOID_STAR)data_to_put);
     }
   else free_array (bt);

   return ret;
}

/* ARRAY[i, j, k] = generates code: __args i j k ARRAY __aput
 */
int _pSLarray_aput1 (unsigned int num_indices)
{
   SLang_Array_Type *at;
   SLang_Object_Type index_objs [SLARRAY_MAX_DIMS];
   int ret;
   int is_index_array;
   int (*aput_fun) (SLtype, unsigned int);
   int type;

   ret = -1;

   type = SLang_peek_at_stack ();
   switch (type)
     {
      case -1:
	return -1;

      case SLANG_ARRAY_TYPE:
	break;

      case SLANG_ASSOC_TYPE:
	return _pSLassoc_aput (type, num_indices);

      default:
	if (NULL != (aput_fun = _pSLclass_get_class (type)->cl_aput))
	  return (*aput_fun) (type, num_indices);
	break;
     }

   if (-1 == SLang_pop_array (&at, 0))
     return -1;

   if (at->flags & SLARR_DATA_VALUE_IS_READ_ONLY)
     {
	_pSLang_verror (SL_READONLY_ERROR, "%s Array is read-only",
		      SLclass_get_datatype_name (at->data_type));
	free_array (at);
	return -1;
     }

   if (-1 == pop_indices (at->num_dims, at->dims, at->num_elements, index_objs, num_indices, &is_index_array))
     {
	free_array (at);
	return -1;
     }

   if (is_index_array == 0)
     {
#if SLANG_OPTIMIZE_FOR_SPEED
	if ((num_indices == 1) && (index_objs[0].o_data_type == SLANG_ARRAY_INDEX_TYPE)
	    && (0 == (at->flags & (SLARR_DATA_VALUE_IS_RANGE|SLARR_DATA_VALUE_IS_POINTER)))
	    && (1 == at->num_dims)
	    && (at->data != NULL))
	  {
	     SLindex_Type ofs = index_objs[0].v.index_val;
	     if (ofs < 0) ofs += at->dims[0];
	     if ((ofs >= at->dims[0]) || (ofs < 0))
	       ret = aput_from_indices (at, index_objs, num_indices);
	     else switch (at->data_type)
	       {
		case SLANG_CHAR_TYPE:
		  ret = SLang_pop_char (((char *)at->data + ofs));
		  break;
		case SLANG_INT_TYPE:
		  ret = SLang_pop_integer (((int *)at->data + ofs));
		  break;
#if SLANG_HAS_FLOAT
		case SLANG_DOUBLE_TYPE:
		  ret = SLang_pop_double ((double *)at->data + ofs);
		  break;
#endif
		default:
		  ret = aput_from_indices (at, index_objs, num_indices);
	       }
	     free_array (at);
	     return ret;
	  }
#endif
	ret = aput_from_indices (at, index_objs, num_indices);
     }
   else
     ret = aput_from_index_array (at, index_objs[0].v.array_val);

   free_array (at);
   free_index_objects (index_objs, num_indices);
   return ret;
}

int _pSLarray_aput (void)
{
   return _pSLarray_aput1 ((unsigned int)(SLang_Num_Function_Args-1));
}

static int _pSLmergesort (void *obj,
			  SLindex_Type *sort_indices, SLindex_Type n,
			  int (*cmp) (void *, SLindex_Type, SLindex_Type))
{
   SLindex_Type i, j, k, kmax, n1, m;
   SLindex_Type *tmp;
   int try_quick_merge;

   if (n < 0)
     {
	SLang_verror (SL_INVALID_PARM, "_pSLmergesort: The number of elements must be non-negative");
	return -1;
     }

   for (i = 0; i < n; i++)
     sort_indices[i] = i;

   /* Insertion sort for 4 elements */
   n1 = n-1;
   m = 4; i = 0;
   while (i < n1)
     {
	kmax = i + m - 1;
	if (kmax >= n)
	  kmax = n1;
	for (k = i+1; k <= kmax; k++)
	  {
	     j = k;
	     while (j > i)
	       {
		  SLindex_Type t;
		  j--;
		  if ((*cmp)(obj, sort_indices[j], k) <= 0)
		    break;
		  t = sort_indices[j];
		  sort_indices[j] = sort_indices[j+1];
		  sort_indices[j+1] = t;
	       }
	  }
	i += m;
     }

   if (m >= n)
     return 0;

   /* Note that 1073741824*2 < 0 */
   i = (n <= 65536) ? m : 65536;
   while ((i*2 < n) && (i*2 > 0))
     i *= 2;

   if (NULL == (tmp = (SLindex_Type *)_SLcalloc (i, sizeof(SLindex_Type))))
     return -1;

   try_quick_merge = 0;
   /* Now do a bottom-up merge sort */
   while ((m < n) && (m > 0))
     {
	i = 0;
	while (i < n-m)
	  {
	     SLindex_Type imax, jmax, l, e_j, e_k;
	     SLindex_Type *sort_indices_i;

	     sort_indices_i = sort_indices + i;
	     k = m;
	     e_k = sort_indices_i[k];
	     if (try_quick_merge)
	       {
		  if ((*cmp)(obj, sort_indices_i[k-1], e_k) <= 0)
		    goto next_i;
	       }

	     j = 0; jmax = m;
	     kmax = k+m;
	     imax = i + kmax;

	     if (imax > n)
	       {
		  imax = n;
		  kmax = imax - i;
	       }

	     /* Only need to copy the left group */
	     memcpy (tmp, sort_indices_i, jmax*sizeof(SLindex_Type));
	     e_j = tmp[j];

	     l = i;
	     while (1)
	       {
		  if ((*cmp)(obj, e_j, e_k) <= 0)
		    {
		       sort_indices[l] = e_j; l++; j++;
		       if (j == jmax)
			 break;
		       e_j = tmp[j];
		       continue;
		    }
		  sort_indices[l] = e_k; l++; k++;
		  if (k == kmax)
		    {
		       memcpy (sort_indices+l, tmp+j, (imax-l)*sizeof(SLindex_Type));
		       break;
		    }
		  e_k = sort_indices_i[k];
	       }
	     try_quick_merge = (k == m);
next_i:
	     i = i + 2*m;
	  }
	m = m * 2;
     }

   SLfree ((char *)tmp);
   return 0;
}

/* Sorting Functions */

typedef struct
{
   SLang_Name_Type *func;
   SLang_Object_Type obj;
   int dir;			       /* +1=ascend, -1=descend */
}
Sort_Object_Type;
static void *QSort_Obj = NULL;

/* This is for 1-d matrices only.  It is used by the sort function */
static int push_element_at_index (SLang_Array_Type *at, SLindex_Type indx)
{
   VOID_STAR data;

   if (NULL == (data = get_data_addr (at, &indx)))
     return -1;

   return push_element_at_addr (at, (VOID_STAR) data, 1);
}

#if SLANG_OPTIMIZE_FOR_SPEED
#define MS_SCALAR_CMP(func, type) \
   static int func(void *obj, SLindex_Type i, SLindex_Type j) \
   { \
      type *data = (type *)obj; \
      if (data[i] > data[j]) \
	return 1; \
      if (data[i] < data[j]) \
	return -1; \
      return i-j; \
   }

#define MS_SCALAR_CMP_DOWN(func, type) \
   static int func(void *obj, SLindex_Type i, SLindex_Type j) \
   { \
      type *data = (type *)obj; \
      if (data[i] > data[j]) \
	return -1; \
      if (data[i] < data[j]) \
	return 1; \
      return i-j; \
   }

#define QS_SCALAR_CMP(func, type) \
   static int func(const void *ip, const void *jp) \
   { \
      SLindex_Type i = *(SLindex_Type *)ip, j = *(SLindex_Type *)jp; \
      type *data = (type *)QSort_Obj; \
      if (data[i] > data[j]) \
	return 1; \
      if (data[i] < data[j]) \
	return -1; \
      return i-j; \
   }

#define QS_SCALAR_CMP_DOWN(func, type) \
   static int func(const void *ip, const void *jp) \
   { \
      SLindex_Type i = *(SLindex_Type *)ip, j = *(SLindex_Type *)jp; \
      type *data = (type *)QSort_Obj; \
      if (data[i] > data[j]) \
	return -1; \
      if (data[i] < data[j]) \
	return 1; \
      return i-j; \
   }

#if SLANG_HAS_FLOAT
MS_SCALAR_CMP(ms_double_sort_cmp, double)
MS_SCALAR_CMP_DOWN(ms_double_sort_down_cmp, double)
MS_SCALAR_CMP(ms_float_sort_cmp, float)
MS_SCALAR_CMP_DOWN(ms_float_sort_down_cmp, float)

QS_SCALAR_CMP(qs_double_sort_cmp, double)
QS_SCALAR_CMP_DOWN(qs_double_sort_down_cmp, double)
QS_SCALAR_CMP(qs_float_sort_cmp, float)
QS_SCALAR_CMP_DOWN(qs_float_sort_down_cmp, float)
#endif

MS_SCALAR_CMP(ms_int_sort_cmp, int)
MS_SCALAR_CMP_DOWN(ms_int_sort_down_cmp, int)
QS_SCALAR_CMP(qs_int_sort_cmp, int)
QS_SCALAR_CMP_DOWN(qs_int_sort_down_cmp, int)

#endif				       /* SLANG_OPTIMIZE_FOR_SPEED */

static int ms_sort_cmp_fun (void *vobj, SLindex_Type i, SLindex_Type j)
{
   int cmp;
   SLang_Array_Type *at;
   Sort_Object_Type *sort_obj = (Sort_Object_Type *)vobj;

   at = sort_obj->obj.v.array_val;

   if (SLang_get_error ()
       || (-1 == push_element_at_index (at, i))
       || (-1 == push_element_at_index (at, j))
       || (-1 == SLexecute_function (sort_obj->func))
       || (-1 == SLang_pop_integer (&cmp)))
     {
	/* error: return something meaningful */
	if (i > j) return 1;
	if (i < j) return -1;
	return 0;
     }
   if (cmp == 0)
     return i - j;

   return cmp*sort_obj->dir;
}

static int ms_sort_opaque_cmp_fun (void *vobj, SLindex_Type i, SLindex_Type j)
{
   int cmp;
   Sort_Object_Type *sort_obj = (Sort_Object_Type *)vobj;

   if (SLang_get_error ()
       || (-1 == _pSLpush_slang_obj (&sort_obj->obj))
       || (-1 == SLclass_push_int_obj (SLANG_ARRAY_INDEX_TYPE, i))
       || (-1 == SLclass_push_int_obj (SLANG_ARRAY_INDEX_TYPE, j))
       || (-1 == SLexecute_function (sort_obj->func))
       || (-1 == SLang_pop_integer (&cmp)))
     {
	/* error: return something meaninful */
	if (i > j) return 1;
	if (i < j) return -1;
	return 0;
     }
   if (cmp == 0)
     return i-j;

   return sort_obj->dir*cmp;
}

static int ms_builtin_sort_cmp_fun (void *vobj, SLindex_Type i, SLindex_Type j)
{
   VOID_STAR a_data;
   VOID_STAR b_data;
   Sort_Object_Type *sort_obj = (Sort_Object_Type *)vobj;
   SLang_Array_Type *at;
   SLang_Class_Type *cl;

   at = sort_obj->obj.v.array_val;
   cl = at->cl;

   if ((SLang_get_error () == 0)
       && (NULL != (a_data = get_data_addr (at, &i)))
       && (NULL != (b_data = get_data_addr (at, &j))))
     {
	int cmp;

	if ((at->flags & SLARR_DATA_VALUE_IS_POINTER)
	    && ((*(VOID_STAR *) a_data == NULL) || (*(VOID_STAR *) a_data == NULL)))
	  {
	     _pSLang_verror (SL_VARIABLE_UNINITIALIZED,
			   "%s array has uninitialized element", cl->cl_name);
	  }
	else if (0 == (*cl->cl_cmp)(at->data_type, a_data, b_data, &cmp))
	  {
	     if (cmp == 0) return i-j;
	     return cmp * sort_obj->dir;
	  }
     }

   if (i > j) return 1;
   if (i < j) return -1;
   return i-j;
}

static void ms_sort_array_internal (void *vobj, SLindex_Type n,
				    int (*sort_cmp)(void *, SLindex_Type, SLindex_Type))
{
   SLang_Array_Type *ind_at;
   SLindex_Type *indx;

   if (NULL == (ind_at = SLang_create_array1 (SLANG_ARRAY_INDEX_TYPE, 0, NULL, &n, 1, 1)))
     return;

   indx = (SLindex_Type *) ind_at->data;
   if (-1 == _pSLmergesort (vobj, indx, n, sort_cmp))
     {
	free_array (ind_at);
	return;
     }

   (void) SLang_push_array (ind_at, 1);
}

static int qs_builtin_sort_cmp_fun (const void *ip, const void *jp)
{
   return ms_builtin_sort_cmp_fun (QSort_Obj, *(SLindex_Type *)ip, *(SLindex_Type *)jp);
}
static int qs_sort_opaque_cmp_fun (const void *ip, const void *jp)
{
   return ms_sort_opaque_cmp_fun (QSort_Obj, *(SLindex_Type *)ip, *(SLindex_Type *)jp);
}
static int qs_sort_cmp_fun (const void *ip, const void *jp)
{
   return ms_sort_cmp_fun (QSort_Obj, *(SLindex_Type *)ip, *(SLindex_Type *)jp);
}

static void qs_sort_array_internal (void *vobj, SLindex_Type n,
				    int (*sort_cmp)(const void *, const void *))
{
   SLang_Array_Type *ind_at;
   SLindex_Type *indx;
   SLindex_Type i;
   void *save_vobj;

   if (NULL == (ind_at = SLang_create_array1 (SLANG_ARRAY_INDEX_TYPE, 0, NULL, &n, 1, 1)))
     return;

   indx = (SLindex_Type *) ind_at->data;
   for (i = 0; i < n; i++)
     indx[i] = i;

   save_vobj = QSort_Obj;
   QSort_Obj = vobj;
   qsort ((void *)indx, n, sizeof (SLindex_Type), sort_cmp);
   QSort_Obj = vobj;

   (void) SLang_push_array (ind_at, 1);
}

static int pop_1d_array (SLang_Array_Type **atp)
{
   SLang_Array_Type *at;

   if (-1 == SLang_pop_array (&at, 1))
     return -1;

   if (at->num_dims != 1)
     {
	_pSLang_verror (SL_INVALID_PARM, "sort is restricted to 1 dim arrays");
	free_array (at);
	return -1;
     }
   *atp = at;
   return 0;
}

#define SORT_METHOD_MSORT	0
#define SORT_METHOD_QSORT	1
static int Default_Sort_Method = SORT_METHOD_MSORT;
static void get_default_sort_method (void)
{
   char *method = NULL;
   switch (Default_Sort_Method)
     {
      case SORT_METHOD_QSORT: method = "qsort"; break;
      case SORT_METHOD_MSORT: method = "msort"; break;
     }
   (void) SLang_push_string (method);
}
static void set_default_sort_method (char *method)
{
   if (0 == strcmp (method, "qsort"))
     {
	Default_Sort_Method = SORT_METHOD_QSORT;
	return;
     }
   Default_Sort_Method = SORT_METHOD_MSORT;
}

/* Usage Forms:
 *    i = sort (a);
 *    i = sort (a, &fun);       % sort using function fun(a[i],a[j])
 *    i = sort (a, &fun, n);    % sort using fun(a, i, j); 0 <= i,j < n
 */
static void array_sort_intrin (void)
{
   SLang_Array_Type *at;
   Sort_Object_Type sort_obj;
   void *vobj;
   SLindex_Type n;
   int nargs = SLang_Num_Function_Args;
   int (*msort_fun)(void *, SLindex_Type, SLindex_Type);
   int (*qsort_fun)(const void *, const void *);
   int dir = 1;
   int use_qsort = 0;
   char *method;

   if (-1 == _pSLang_get_int_qualifier ("dir", &dir, 1))
     return;
   dir = (dir >= 0) ? 1 : -1;
   use_qsort = (Default_Sort_Method == SORT_METHOD_QSORT);
   if (_pSLang_qualifier_exists ("qsort")) use_qsort = 1;
   if (-1 == _pSLang_get_string_qualifier ("method", &method, NULL))
     return;
   if (method != NULL)
     {
	if (0 == strcmp(method, "qsort"))
	  use_qsort = 1;
	SLang_free_slstring (method);
     }

   if (nargs == 1)		       /* i = sort (a) */
     {
	if (-1 == pop_1d_array (&at))
	  return;

	switch (at->data_type)
	  {
#if SLANG_OPTIMIZE_FOR_SPEED
# if SLANG_HAS_FLOAT
	   case SLANG_DOUBLE_TYPE:
	     msort_fun = (dir > 0) ? ms_double_sort_cmp : ms_double_sort_down_cmp;
	     qsort_fun = (dir > 0) ? qs_double_sort_cmp : qs_double_sort_down_cmp;
	     vobj = at->data;
	     break;
	   case SLANG_FLOAT_TYPE:
	     msort_fun = (dir > 0) ? ms_float_sort_cmp : ms_float_sort_down_cmp;
	     qsort_fun = (dir > 0) ? qs_float_sort_cmp : qs_float_sort_down_cmp;
	     vobj = at->data;
	     break;
# endif
	   case SLANG_INT_TYPE:
	     msort_fun = (dir > 0) ? ms_int_sort_cmp : ms_int_sort_down_cmp;
	     qsort_fun = (dir > 0) ? qs_int_sort_cmp : qs_int_sort_down_cmp;
	     vobj = at->data;
	     break;
#endif
	   default:
	     if (at->cl->cl_cmp == NULL)
	       {
		  _pSLang_verror (SL_NOT_IMPLEMENTED,
				  "%s does not have a predefined sorting method",
				  at->cl->cl_name);
		  free_array (at);
		  return;
	       }
	     msort_fun = ms_builtin_sort_cmp_fun;
	     qsort_fun = qs_builtin_sort_cmp_fun;
	     sort_obj.obj.o_data_type = SLANG_ARRAY_TYPE;
	     sort_obj.obj.v.array_val = at;
	     sort_obj.dir = dir;
	     vobj = (void *)&sort_obj;
	  }

	n = (SLindex_Type) at->num_elements;
	if (use_qsort)
	  qs_sort_array_internal (vobj, n, qsort_fun);
	else
	  ms_sort_array_internal (vobj, n, msort_fun);
	free_array (at);
	return;
     }

   if (nargs == 2)		       /* i = sort (a, &fun) */
     {
	SLang_Name_Type *entry;

	if (NULL == (entry = SLang_pop_function ()))
	  return;

	if (-1 == pop_1d_array (&at))
	  {
	     SLang_free_function (entry);
	     return;
	  }

	sort_obj.func = entry;
	sort_obj.obj.o_data_type = SLANG_ARRAY_TYPE;
	sort_obj.obj.v.array_val = at;
	sort_obj.dir = dir;
	vobj = (void *)&sort_obj;

	n = (SLindex_Type) at->num_elements;
	if (use_qsort)
	  qs_sort_array_internal (vobj, n, qs_sort_cmp_fun);
	else
	  ms_sort_array_internal (vobj, n, ms_sort_cmp_fun);
	free_array (at);
	SLang_free_function (entry);
	return;
     }

   if (nargs == 3)		       /* i = sort (a, fun, n) */
     {
	SLang_Name_Type *entry;

	if (-1 == SLang_pop_array_index (&n))
	  return;

	if (n < 0)
	  {
	     SLang_verror (SL_Index_Error, "Sort object cannot have a negative size");
	     return;
	  }

	if (NULL == (entry = SLang_pop_function ()))
	  return;

	if (-1 == SLang_pop (&sort_obj.obj))
	  {
	     SLang_free_function (entry);
	     return;
	  }
	sort_obj.func = entry;
	sort_obj.dir = dir;
	vobj = (void *)&sort_obj;

	if (use_qsort)
	  qs_sort_array_internal (vobj, n, qs_sort_opaque_cmp_fun);
	else
	  ms_sort_array_internal (vobj, n, ms_sort_opaque_cmp_fun);
	SLang_free_object (&sort_obj.obj);
	SLang_free_function (entry);
	return;
     }

   SLang_verror (SL_Usage_Error, "\
Usage: i = array_sort(a);\n\
       i = array_sort(a, &func);        %% cmp = func(a[i], b[j]);\n\
       i = array_sort(obj, &func, n);   %% cmp = func(obj, i, j)\n");
}

static void bstring_to_array (SLang_BString_Type *bs)
{
   unsigned char *s;
   unsigned int len;

   if (NULL == (s = SLbstring_get_pointer (bs, &len)))
     (void) SLang_push_null ();
   else
     (void) push_string_as_array (s, len);
}

static void array_to_bstring (SLang_Array_Type *at)
{
   size_t nbytes;
   SLang_BString_Type *bs;

   nbytes = at->num_elements * at->sizeof_type;
   bs = SLbstring_create ((unsigned char *)at->data, nbytes);
   (void) SLang_push_bstring (bs);
   SLbstring_free (bs);
}

static void init_char_array (void)
{
   SLang_Array_Type *at;
   char *s;
   unsigned int n, ndim;

   if (SLang_pop_slstring (&s)) return;

   if (-1 == SLang_pop_array (&at, 0))
     goto free_and_return;

   if (at->data_type != SLANG_CHAR_TYPE)
     {
	_pSLang_verror (SL_TYPE_MISMATCH, "Operation requires a character array");
	goto free_and_return;
     }

   n = _pSLstring_bytelen (s);
   ndim = at->num_elements;
   if (n > ndim)
     {
	_pSLang_verror (SL_INVALID_PARM, "String too big to initialize array");
	goto free_and_return;
     }

   strncpy((char *) at->data, s, ndim);
   /* drop */

   free_and_return:
   free_array (at);
   _pSLang_free_slstring (s);
}

static VOID_STAR range_get_data_addr (SLang_Array_Type *at, SLindex_Type *dims)
{
   static int value;
   SLarray_Range_Array_Type *r;
   SLindex_Type d;

   d = *dims;
   r = (SLarray_Range_Array_Type *)at->data;

   if (d < 0)
     d += at->dims[0];

   if ((SLuindex_Type)d >= at->num_elements)
     {
	SLang_set_error (SL_Index_Error);
	return NULL;
     }
   value = r->first_index + d * r->delta;
   return (VOID_STAR) &value;
}

static SLang_Array_Type
  *create_range_array (SLarray_Range_Array_Type *range, SLindex_Type num,
		       SLtype type, int (*to_linear_fun) (SLang_Array_Type *, SLarray_Range_Array_Type *, VOID_STAR))
{
   SLarray_Range_Array_Type *r;
   SLang_Array_Type *at;

   r = (SLarray_Range_Array_Type *) SLmalloc (sizeof (SLarray_Range_Array_Type));
   if (r == NULL)
     return NULL;
   memset((char *) r, 0, sizeof (SLarray_Range_Array_Type));

   if (NULL == (at = SLang_create_array (type, 0, (VOID_STAR) range, &num, 1)))
     {
	SLfree ((char *)range);
	return NULL;
     }
   r->first_index = range->first_index;
   r->last_index = range->last_index;
   r->delta = range->delta;
   r->has_first_index = range->has_first_index;
   r->has_last_index = range->has_last_index;
   r->to_linear_fun = to_linear_fun;
   at->data = (VOID_STAR) r;
   at->index_fun = range_get_data_addr;
   at->flags |= SLARR_DATA_VALUE_IS_RANGE;
   return at;
}

static int get_range_array_limits (SLindex_Type *first_indexp, SLindex_Type *last_indexp, SLindex_Type *deltap,
				   SLarray_Range_Array_Type *r, SLindex_Type *nump)
{
   SLindex_Type first_index, last_index, delta;
   SLindex_Type num;

   if (deltap == NULL) delta = 1;
   else delta = *deltap;

   if (delta == 0)
     {
	_pSLang_verror (SL_INVALID_PARM, "range-array increment must be non-zero");
	return -1;
     }

   r->has_first_index = (first_indexp != NULL);
   if (r->has_first_index)
     first_index = *first_indexp;
   else
     first_index = 0;

   r->has_last_index = (last_indexp != NULL);
   if (r->has_last_index)
     last_index = *last_indexp;
   else
     last_index = -1;

   num = 0;
   if (delta > 0)
     {
	/* Case: [20:10:11] --> 0 elements, [10:20:11] --> 1 element */
	if (last_index >= first_index)
	  num = 1 + (last_index - first_index) / delta;
     }
   else
     {
	/* Case: [20:10:-11] -> 1 element, [20:30:-11] -> none */
	if (last_index <= first_index)
	  num = 1 + (last_index - first_index) / delta;
     }

   r->first_index = first_index;
   r->last_index = last_index;
   r->delta = delta;
   *nump = num;

   return 0;
}

static int index_range_to_linear (SLang_Array_Type *at, SLarray_Range_Array_Type *range, VOID_STAR buf)
{
   SLindex_Type *data = (SLindex_Type *)buf;
   SLuindex_Type i, imax;
   SLindex_Type xmin, dx;

   imax = at->num_elements;
   xmin = range->first_index;
   dx = range->delta;
   for (i = 0; i < imax; i++)
     {
	data [i] = xmin;
	xmin += dx;
     }
   return 0;
}

static SLang_Array_Type *inline_implicit_index_array (SLindex_Type *xminptr, SLindex_Type *xmaxptr, SLindex_Type *dxptr)
{
   SLarray_Range_Array_Type r;
   SLindex_Type num;

   if (-1 == get_range_array_limits (xminptr, xmaxptr, dxptr, &r, &num))
     return NULL;

   return create_range_array (&r, num, SLANG_ARRAY_INDEX_TYPE, index_range_to_linear);
}

#if (SLANG_ARRAY_INDEX_TYPE == SLANG_INT_TYPE)
# define int_range_to_linear index_range_to_linear
# define inline_implicit_int_array inline_implicit_index_array
#else
static int int_range_to_linear (SLang_Array_Type *at, SLarray_Range_Array_Type *range, VOID_STAR buf)
{
   int *data = (int *)buf;
   unsigned int i, imax;
   int xmin, dx;

   imax = (unsigned int)at->num_elements;
   xmin = (int) range->first_index;
   dx = (int) range->delta;
   for (i = 0; i < imax; i++)
     {
	data [i] = xmin;
	xmin += dx;
     }
   return 0;
}

static SLang_Array_Type *inline_implicit_int_array (SLindex_Type *xminptr, SLindex_Type *xmaxptr, SLindex_Type *dxptr)
{
   SLarray_Range_Array_Type r;
   SLindex_Type num;

   if (-1 == get_range_array_limits (xminptr, xmaxptr, dxptr, &r, &num))
     return NULL;

   return create_range_array (&r, num, SLANG_INT_TYPE, int_range_to_linear);
}
#endif

#if SLANG_HAS_FLOAT
static SLang_Array_Type *inline_implicit_floating_array (SLtype type,
							 double *xminptr, double *xmaxptr, double *dxptr,
							 int ntype, int nels)
{
   SLindex_Type n, i;
   SLang_Array_Type *at;
   SLindex_Type dims;
   double xmin, xmax, dx;

   if ((xminptr == NULL) || (xmaxptr == NULL))
     {
	_pSLang_verror (SL_INVALID_PARM, "range-array has unknown size");
	return NULL;
     }
   xmin = *xminptr;
   xmax = *xmaxptr;

   if (ntype)
     {
	/* [a:b:#n] == a + [0:(n-1)]*(b-a)/(n-1)
	 * ==> dx = (b-a)/(n-1)
	 */
	n = (SLindex_Type) nels;
	if (n <= 0)
	  {
	     n = 0;
	     dx = 1.0;
	  }
	else
	  {
	     if (n == 1)
	       dx = 0.0;
	     else
	       dx = (xmax-xmin)/(n-1);
	  }
     }
   else
     {
	if (dxptr == NULL) dx = 1.0;
	else dx = *dxptr;

	if (dx == 0.0)
	  {
	     _pSLang_verror (SL_INVALID_PARM, "range-array increment must be non-zero");
	     return NULL;
	  }

	/* I have convinced myself that it is better to use semi-open intervals
	 * because of less ambiguities.  So, [a:b:c] will represent the set of
	 * values a, a + c, a + 2c ... a + nc
	 * such that a + nc < b.  That is, b lies outside the interval.
	 */

	if (((xmax <= xmin) && (dx >= 0.0))
	    || ((xmax >= xmin) && (dx <= 0.0)))
	  n = 0;
	else
	  {
	     double last;

	     if ((xmin + dx == (volatile double)xmin) || (xmax + dx == (volatile double)xmax))
	       n = 0;
	     else
	     /* Allow for roundoff by adding 0.5 before truncation */
	       n = (int)(1.5 + ((xmax - xmin) / dx));

	     if (n <= 0)
	       {
		  _pSLang_verror (SL_INVALID_PARM, "range-array increment is too small");
		  return NULL;
	       }

	     last = xmin + (n-1)*dx;

	     if (dx > 0.0)
	       {
		  if (last >= xmax)
		    n -= 1;
	       }
	     else if (last <= xmax)
	       n -= 1;
	  }
     }

   dims = n;
   if (NULL == (at = SLang_create_array1 (type, 0, NULL, &dims, 1, 1)))
     return NULL;

   if (type == SLANG_DOUBLE_TYPE)
     {
	double *ptr;

	ptr = (double *) at->data;

	for (i = 0; i < n; i++)
	  ptr[i] = xmin + i * dx;

	/* Explicitly set the last element to xmax to avoid roundoff error */
	if (ntype && (n > 1))
	  ptr[n-1] = xmax;
     }
   else
     {
	float *ptr;

	ptr = (float *) at->data;

	for (i = 0; i < n; i++)
	  ptr[i] = (float) (xmin + i * dx);

	if (ntype && (n > 0))
	  ptr[n-1] = (float) xmax;
     }
   return at;
}
#endif

static int pop_range_int (SLindex_Type *ip)
{
   return SLang_pop_array_index (ip);
}

/* FIXME: Priority=medium
 * This needs to be updated to work with all integer types.
 * Adding support for other types is going to require a generalization
 * of the Range_Array_Type object.
 */
/* If ntype is non-zero, the array was specified using [a:b:#c] */
static int inline_implicit_array (int ntype)
{
   SLindex_Type index_vals[3];
#if SLANG_HAS_FLOAT
   double double_vals[3];
   int is_int;
#endif
   int has_vals[3];
   unsigned int i, count;
   SLindex_Type n = 0;
   SLang_Array_Type *at;
   int precedence;
   SLtype type;

   count = SLang_Num_Function_Args;

   if ((count == 2) && (ntype == 0))
     has_vals [2] = 0;
   else if (count != 3)
     {
	_pSLang_verror (SL_NUM_ARGS_ERROR, "wrong number of arguments to __implicit_inline_array");
	return -1;
     }

#if SLANG_HAS_FLOAT
   is_int = 1;
#endif

   type = 0;
   precedence = 0;

   if (ntype)
     {
	if (-1 == pop_range_int (&n))
	  return -1;
	has_vals[2] = 0;
	count--;
     }
   i = count;

   while (i--)
     {
	int this_type, this_precedence;
	SLindex_Type itmp;

	if (-1 == (this_type = SLang_peek_at_stack ()))
	  return -1;

	this_precedence = _pSLarith_get_precedence ((SLtype) this_type);
	if (precedence < this_precedence)
	  {
	     type = (SLtype) this_type;
	     precedence = this_precedence;
	  }

	has_vals [i] = 1;

	switch (this_type)
	  {
	   case SLANG_NULL_TYPE:
	     if (ntype)
	       {
		  _pSLang_verror (SL_Syntax_Error, "Arrays of the form [a:b:#c] must be fully specified");
		  return -1;
	       }
	     has_vals[i] = 0;
	     (void) SLdo_pop ();
	     break;

#if SLANG_HAS_FLOAT
	   case SLANG_DOUBLE_TYPE:
	   case SLANG_FLOAT_TYPE:
	     if (-1 == SLang_pop_double (double_vals + i))
	       return -1;
	     is_int = 0;
	     break;
#endif
	   default:
	     if (-1 == pop_range_int (&itmp))
	       return -1;
	     index_vals[i] = itmp;
#if SLANG_HAS_FLOAT
	     double_vals[i] = (double) itmp;
#endif
	  }
     }

#if SLANG_HAS_FLOAT
   if (ntype)
     {
	is_int = 0;
	if (type != SLANG_FLOAT_TYPE)
	  type = SLANG_DOUBLE_TYPE;
     }

   if (is_int == 0)
     at = inline_implicit_floating_array (type,
					  (has_vals[0] ? &double_vals[0] : NULL),
					  (has_vals[1] ? &double_vals[1] : NULL),
					  (has_vals[2] ? &double_vals[2] : NULL),
					  ntype, n);
   else
#endif
     at = inline_implicit_int_array ((has_vals[0] ? &index_vals[0] : NULL),
				     (has_vals[1] ? &index_vals[1] : NULL),
				     (has_vals[2] ? &index_vals[2] : NULL));

   if (at == NULL)
     return -1;

   return SLang_push_array (at, 1);
}

int _pSLarray_inline_implicit_array (void)
{
   return inline_implicit_array (0);
}

int _pSLarray_inline_implicit_arrayn (void)
{
   return inline_implicit_array (1);
}

static int try_typecast_range_array (SLang_Array_Type *at, SLtype to_type,
				     SLang_Array_Type **btp)
{
   SLang_Array_Type *bt;

   *btp = NULL;
   if (to_type == SLANG_ARRAY_INDEX_TYPE)
     {
	if (at->data_type == SLANG_INT_TYPE)
	  {
	     SLarray_Range_Array_Type *range;

	     range = (SLarray_Range_Array_Type *)at->data;
	     bt = create_range_array (range, at->num_elements,
				      to_type, index_range_to_linear);
	     if (bt == NULL)
	       return -1;
	     *btp = bt;
	     return 1;
	  }
     }
   return 0;
}

int _pSLarray_wildcard_array (void)
{
   SLang_Array_Type *at;

   if (NULL == (at = inline_implicit_int_array (NULL, NULL, NULL)))
     return -1;

   return SLang_push_array (at, 1);
}

/* FIXME: The type-promotion routine needs to be made more generic and
 * better support user-defined types.
 */

/* Test if the type cannot be promoted further */
_INLINE_ static int nowhere_to_promote (SLtype type)
{
   switch (type)
     {
      case SLANG_COMPLEX_TYPE:
      case SLANG_BSTRING_TYPE:
      case SLANG_ARRAY_TYPE:
	return 1;
     }

   return 0;
}

static int promote_to_common_type (SLtype a, SLtype b, SLtype *c)
{
   if (a == b)
     {
	*c = a;
	return 0;
     }
   if (nowhere_to_promote (a))
     {
	/* a type can always be converted to an array: T -> [T] */
	if (b == SLANG_ARRAY_TYPE)
	  *c = b;
	else
	  *c = a;
	return 0;
     }
   if (nowhere_to_promote (b))
     {
	*c = b;
	return 0;
     }

   if (_pSLang_is_arith_type (a) && _pSLang_is_arith_type (b))
     {
	if (_pSLarith_get_precedence (a) > _pSLarith_get_precedence (b))
	  *c = a;
	else
	  *c = b;
	return 0;
     }

   if (a == SLANG_NULL_TYPE)
     {
	*c = b;
	return 0;
     }
   if (b == SLANG_NULL_TYPE)
     {
	*c = a;
	return 0;
     }

   *c = a;
   return 0;
}

static SLtype get_type_for_concat (SLang_Array_Type **arrays, unsigned int n)
{
   SLtype type;
   unsigned int i;

   type = arrays[0]->data_type;

   for (i = 1; i < n; i++)
     {
	SLtype this_type = arrays[i]->data_type;

	if (this_type == type)
	  continue;

	if (-1 == promote_to_common_type (type, this_type, &type))
	  return SLANG_UNDEFINED_TYPE;
     }
   return type;
}

static SLang_Array_Type *concat_arrays (unsigned int count)
{
   SLang_Array_Type **arrays;
   SLang_Array_Type *at, *bt;
   unsigned int i;
   SLindex_Type num_elements;
   SLtype type;
   char *src_data, *dest_data;
   int is_ptr;
   size_t sizeof_type;
   int max_dims, min_dims, max_rows, min_rows;

   arrays = (SLang_Array_Type **)_SLcalloc (count, sizeof (SLang_Array_Type *));
   if (arrays == NULL)
     {
	SLdo_pop_n (count);
	return NULL;
     }
   SLMEMSET((char *) arrays, 0, count * sizeof(SLang_Array_Type *));

   at = NULL;

   num_elements = 0;
   i = count;

   while (i != 0)
     {
	i--;

	if (-1 == SLang_pop_array (&bt, 1))   /* bt is now linear */
	  goto free_and_return;

	arrays[i] = bt;
	num_elements += (int)bt->num_elements;
     }

   /* From here on, arrays[*] are linear */

   /* type = arrays[0]->data_type; */
   type = get_type_for_concat (arrays, count);

   max_dims = min_dims = arrays[0]->num_dims;
   min_rows = max_rows = arrays[0]->dims[0];

   for (i = 0; i < count; i++)
     {
	SLang_Array_Type *ct;
	int num;

	bt = arrays[i];

	num = bt->num_dims;
	if (num > max_dims) max_dims = num;
	if (num < min_dims) min_dims = num;

	num = bt->dims[0];
	if (num > max_rows) max_rows = num;
	if (num < min_rows) min_rows = num;

	if (type == bt->data_type)
	  continue;

	if (1 != _pSLarray_typecast (bt->data_type, (VOID_STAR) &bt, 1,
				    type, (VOID_STAR) &ct, 1))
	  goto free_and_return;

	free_array (bt);
	arrays [i] = ct;
     }

   if (NULL == (at = SLang_create_array (type, 0, NULL, &num_elements, 1)))
     goto free_and_return;

   is_ptr = (at->flags & SLARR_DATA_VALUE_IS_POINTER);
   sizeof_type = at->sizeof_type;
   dest_data = (char *) at->data;

   for (i = 0; i < count; i++)
     {
	bt = arrays[i];

	src_data = (char *) bt->data;
	num_elements = bt->num_elements;

	if (-1 == transfer_n_elements (bt, (VOID_STAR)dest_data, (VOID_STAR)src_data, sizeof_type,
				       num_elements, is_ptr))
	  {
	     free_array (at);
	     at = NULL;
	     goto free_and_return;
	  }

	dest_data += num_elements * sizeof_type;
     }

#if 0
   /* If the arrays are all 1-d, and all the same size, then reshape to a
    * 2-d array.  This will allow us to do, e.g.
    * a = [[1,2], [3,4]]
    * to specifiy a 2-d.
    * Someday I will generalize this.
    */
   /* This is a bad idea.  Everyone using it expects concatenation to happen.
    * Perhaps I will extend the syntax to allow a 2-d array to be expressed
    * as [[1,2];[3,4]].
    */
   if ((max_dims == min_dims) && (max_dims == 1) && (min_rows == max_rows))
     {
	at->num_dims = 2;
	at->dims[0] = count;
	at->dims[1] = min_rows;
     }
#endif
   free_and_return:

   for (i = 0; i < count; i++)
     free_array (arrays[i]);
   SLfree ((char *) arrays);

   return at;
}

int _pSLarray_inline_array (void)
{
   SLang_Object_Type *obj, *objmin;
   SLtype type, this_type;
   unsigned int count;
   SLang_Array_Type *at;

   obj = _pSLang_get_run_stack_pointer ();
   objmin = _pSLang_get_run_stack_base ();

   count = SLang_Num_Function_Args;
   type = 0;

   while ((count > 0) && (--obj >= objmin))
     {
	this_type = obj->o_data_type;

	if (type == 0)
	  type = this_type;
	else if (type != this_type)
	  {
	     if (-1 == promote_to_common_type (type, this_type, &type))
	       {
		  _pSLclass_type_mismatch_error (type, this_type);
		  return -1;
	       }
	  }
	count--;
     }

   if (count != 0)
     {
	SLang_set_error (SL_STACK_UNDERFLOW);
	return -1;
     }

   count = SLang_Num_Function_Args;

   if (count == 0)
     {
	_pSLang_verror (SL_NOT_IMPLEMENTED, "Empty inline-arrays not supported");
	return -1;
     }

   if (type == SLANG_ARRAY_TYPE)
     {
	if (count == 1)
	  return 0;		       /* no point in going on */

	if (NULL == (at = concat_arrays (count)))
	  return -1;
     }
   else
     {
	SLang_Object_Type index_obj;
	SLindex_Type icount = (SLindex_Type) count;

	if (NULL == (at = SLang_create_array (type, 0, NULL, &icount, 1)))
	  return -1;

	index_obj.o_data_type = SLANG_ARRAY_INDEX_TYPE;
	while (count != 0)
	  {
	     count--;
	     index_obj.v.index_val = count;
	     if (-1 == aput_from_indices (at, &index_obj, 1))
	       {
		  free_array (at);
		  SLdo_pop_n (count);
		  return -1;
	       }
	  }
     }

   return SLang_push_array (at, 1);
}

int _pSLarray_convert_to_array (VOID_STAR cd,
				int (*get_type)(VOID_STAR, SLuindex_Type, SLtype *),
				int (*push)(VOID_STAR, SLuindex_Type),
			        SLuindex_Type num_objects, SLtype type)
{
   SLtype this_type;
   SLang_Array_Type *at;
   SLuindex_Type i;
   SLindex_Type dims;
   SLang_Object_Type index_obj;

   at = NULL;

   if (type == 0) for (i = 0; i < num_objects; i++)
     {
	if (-1 == (*get_type)(cd, i, &this_type))
	  goto unknown_error;

	if (type == 0)
	  type = this_type;
	else if (type != this_type)
	  {
	     if (-1 == promote_to_common_type (type, this_type, &type))
	       {
		  _pSLclass_type_mismatch_error (type, this_type);
		  return -1;
	       }
	  }
     }

   if (type == 0)
     {
	SLang_verror (SL_TypeMismatch_Error, "Cannot convert an empty container object to an untyped array");
	return -1;
     }

   dims = (SLindex_Type) num_objects;

   if (NULL == (at = SLang_create_array (type, 0, NULL, &dims, 1)))
     return -1;

   index_obj.o_data_type = SLANG_ARRAY_INDEX_TYPE;
   for (i = 0; i < num_objects; i++)
     {
	if (-1 == (*push)(cd, i))
	  goto unknown_error;

	index_obj.v.index_val = i;
	if (-1 == aput_from_indices (at, &index_obj, 1))
	  goto return_error;
     }

   return SLang_push_array (at, 1);

unknown_error:
   SLang_verror (SL_Unknown_Error, "Unknown array conversion error");
return_error:
   if (at != NULL)
     free_array (at);

   return -1;
}

static int array_binary_op_result (int op, SLtype a, SLtype b,
				   SLtype *c)
{
   (void) op;
   (void) a;
   (void) b;
   *c = SLANG_ARRAY_TYPE;
   return 1;
}

static int try_range_int_binary (SLang_Array_Type *at, int op, int x, int swap, VOID_STAR cp)
{
   SLarray_Range_Array_Type *at_r;
   SLarray_Range_Array_Type rbuf;
   SLindex_Type first_index, last_index, delta;
   SLindex_Type num;

   at_r = (SLarray_Range_Array_Type *)at->data;
   if ((at_r->has_first_index == 0)
       || (at_r->has_last_index == 0))
     return 0;

   switch (op)
     {
      case SLANG_MINUS:
	if (swap)
	  {
	     first_index = x - at_r->first_index;
	     last_index = x - at_r->last_index;
	     delta = -at_r->delta;
	     break;
	  }
	x = -x;
	/* drop */
      case SLANG_PLUS:
	first_index = at_r->first_index + x;
	last_index = at_r->last_index + x;
	delta = at_r->delta;
	break;

      case SLANG_TIMES:
	if (x == 0)
	  return 0;
	first_index = at_r->first_index*x;
	last_index = at_r->last_index*x;
	delta = at_r->delta*x;
	break;

      default:
	return 0;
     }

   if (-1 == get_range_array_limits (&first_index, &last_index, &delta, &rbuf, &num))
     return -1;
   if ((SLuindex_Type)num != at->num_elements)
     return 0; /* This can happen if the integer arithmetic wrapped */

   if (NULL == (at = create_range_array (&rbuf, num, SLANG_INT_TYPE, int_range_to_linear)))
     return -1;

   *(SLang_Array_Type **)cp = at;
   return 1;
}

static int array_binary_op (int op,
			    SLtype a_type, VOID_STAR ap, SLuindex_Type na,
			    SLtype b_type, VOID_STAR bp, SLuindex_Type nb,
			    VOID_STAR cp)
{
   SLang_Array_Type *at, *bt, *ct;
   SLuindex_Type i, num_dims;
   int (*binary_fun) (int,
		      SLtype, VOID_STAR, SLuindex_Type,
		      SLtype, VOID_STAR, SLuindex_Type,
		      VOID_STAR);
   SLang_Class_Type *a_cl, *b_cl, *c_cl;
   int no_init, ret;

   if (a_type == SLANG_ARRAY_TYPE)
     {
	if (na != 1)
	  {
	     _pSLang_verror (SL_NOT_IMPLEMENTED, "Binary operation on multiple arrays not implemented");
	     return -1;
	  }

	at = *(SLang_Array_Type **) ap;
	if ((b_type == SLANG_INT_TYPE)
	    && (nb == 1)
	    && (at->flags & SLARR_DATA_VALUE_IS_RANGE)
	    && (at->data_type == b_type))
	  {
	     int status = try_range_int_binary (at, op, *(int *)bp, 0, cp);
	     if (status)
	       return status;
	     /* drop */
	  }

	if (-1 == coerse_array_to_linear (at))
	  return -1;
	ap = at->data;
	a_type = at->data_type;
	na = at->num_elements;
     }
   else
     {
	at = NULL;
     }

   if (b_type == SLANG_ARRAY_TYPE)
     {
	if (nb != 1)
	  {
	     _pSLang_verror (SL_NOT_IMPLEMENTED, "Binary operation on multiple arrays not implemented");
	     return -1;
	  }

	bt = *(SLang_Array_Type **) bp;

	if ((a_type == SLANG_INT_TYPE)
	    && (na == 1)
	    && (bt->flags & SLARR_DATA_VALUE_IS_RANGE)
	    && (bt->data_type == a_type))
	  {
	     int status = try_range_int_binary (bt, op, *(int *)ap, 1, cp);
	     if (status)
	       return status;
	     /* drop */
	  }

	if (-1 == coerse_array_to_linear (bt))
	  return -1;
	bp = bt->data;
	b_type = bt->data_type;
	nb = bt->num_elements;
     }
   else
     {
	bt = NULL;
     }

   if ((at != NULL) && (bt != NULL))
     {
	num_dims = at->num_dims;

	if (num_dims != bt->num_dims)
	  {
	     _pSLang_verror (SL_TYPE_MISMATCH, "Arrays must have same dimensions for binary operation");
	     return -1;
	  }

	for (i = 0; i < num_dims; i++)
	  {
	     if (at->dims[i] != bt->dims[i])
	       {
		  _pSLang_verror (SL_TYPE_MISMATCH, "Arrays must be the same for binary operation");
		  return -1;
	       }
	  }
     }

   a_cl = _pSLclass_get_class (a_type);
   if (a_type == b_type)
     b_cl = a_cl;
   else
     b_cl = _pSLclass_get_class (b_type);

   if (NULL == (binary_fun = _pSLclass_get_binary_fun (op, a_cl, b_cl, &c_cl, 1)))
     return -1;

   ct = NULL;

   no_init = ((c_cl->cl_class_type == SLANG_CLASS_TYPE_SCALAR)
	      || (c_cl->cl_class_type == SLANG_CLASS_TYPE_VECTOR));

#if SLANG_USE_TMP_OPTIMIZATION
   /* If we are dealing with scalar (or vector) objects, and if the object
    * appears to be owned by the stack, then use it instead of creating a
    * new version.  This can happen with code such as:
    * @  x = [1,2,3,4];
    * @  x = __tmp(x) + 1;
    */
   if (no_init)
     {
	if ((at != NULL)
	    && (at->num_refs == 1)
	    && (at->data_type == c_cl->cl_data_type)
	    && (0 == (at->flags & SLARR_DATA_VALUE_IS_READ_ONLY)))
	  {
	     ct = at;
	     ct->num_refs = 2;
	  }
	else if ((bt != NULL)
		 && (bt->num_refs == 1)
		 && (bt->data_type == c_cl->cl_data_type)
		 && (0 == (bt->flags & SLARR_DATA_VALUE_IS_READ_ONLY)))
	  {
	     ct = bt;
	     ct->num_refs = 2;
	  }
     }
#endif				       /* SLANG_USE_TMP_OPTIMIZATION */

   if (ct == NULL)
     {
	if (at != NULL) ct = at; else ct = bt;
	ct = SLang_create_array1 (c_cl->cl_data_type, 0, NULL, ct->dims, ct->num_dims, 1);
	if (ct == NULL)
	  return -1;
     }

   if ((na == 0) || (nb == 0))	       /* allow empty arrays */
     {
	*(SLang_Array_Type **) cp = ct;
	return 1;
     }

   if (a_cl->cl_inc_ref != NULL)(*a_cl->cl_inc_ref)(a_type, ap, 1);
   if (b_cl->cl_inc_ref != NULL)(*b_cl->cl_inc_ref)(b_type, bp, 1);
   ret = (*binary_fun) (op, a_type, ap, na, b_type, bp, nb, ct->data);
   if (a_cl->cl_inc_ref != NULL)(*a_cl->cl_inc_ref)(a_type, ap, -1);
   if (b_cl->cl_inc_ref != NULL)(*b_cl->cl_inc_ref)(b_type, bp, -1);

   if (ret == 1)
     {
	*(SLang_Array_Type **) cp = ct;
	return 1;
     }

   free_array (ct);
   return -1;
}

#if SLANG_OPTIMIZE_FOR_SPEED
int _pSLarray_bin_op (SLang_Object_Type *a, SLang_Object_Type *b, int op)
{
   SLang_Array_Type *c;

   if (-1 == array_binary_op (op, SLANG_ARRAY_TYPE, (VOID_STAR) &a->v, 1,
			      SLANG_ARRAY_TYPE, (VOID_STAR) &b->v, 1,
			      (VOID_STAR) &c))
     return -1;

   return _pSLang_push_array (c, 1);
}
#endif

static int array_eqs_method (SLtype a_type, VOID_STAR ap, SLtype b_type, VOID_STAR bp)
{
   SLang_Array_Type *at, *bt, *ct;
   SLuindex_Type i, num_dims, num_elements;
   SLang_Class_Type *a_cl, *b_cl, *c_cl;
   int is_eqs;
   int *ip, *ipmax;

   if ((a_type != SLANG_ARRAY_TYPE) || (b_type != SLANG_ARRAY_TYPE))
     return 0;

   at = *(SLang_Array_Type **) ap;
   bt = *(SLang_Array_Type **) bp;

   if (at == bt)
     return 1;

   if ((at->num_elements != (num_elements = bt->num_elements))
       || (at->num_dims != (num_dims = bt->num_dims)))
     return 0;

   for (i = 0; i < num_dims; i++)
     {
	if (at->dims[i] != bt->dims[i])
	  return 0;
     }

   a_type = at->data_type;
   b_type = bt->data_type;

   /* Check for an array of arrays.  If so, the arrays must reference the same set arrays */
   if ((a_type == SLANG_ARRAY_TYPE) || (b_type == SLANG_ARRAY_TYPE))
     {
	if (a_type != b_type)
	  return 0;

	return !memcmp ((char *)at->data, (char *)bt->data, num_elements*sizeof(SLang_Array_Type*));
     }

   a_cl = _pSLclass_get_class (a_type);
   if (a_type == b_type)
     b_cl = a_cl;
   else
     b_cl = _pSLclass_get_class (b_type);

   if ((a_cl == b_cl)
       && ((a_cl->cl_class_type == SLANG_CLASS_TYPE_SCALAR)
	   || (a_cl->cl_class_type == SLANG_CLASS_TYPE_VECTOR)))
     {
	if ((-1 == coerse_array_to_linear (at))
	    || (-1 == coerse_array_to_linear (bt)))
	  return -1;

	return !memcmp ((char *)at->data, (char *)bt->data, num_elements*at->sizeof_type);
     }

   /* Do it the hard way */

   if (NULL == _pSLclass_get_binary_fun (SLANG_EQ, a_cl, b_cl, &c_cl, 0))
     return 0;

   if (num_elements == 0)
     return 1;

   if (-1 == array_binary_op (SLANG_EQ, SLANG_ARRAY_TYPE, ap, 1, SLANG_ARRAY_TYPE, bp, 1,
			      (VOID_STAR) &ct))
     return -1;

   /* ct is linear */
   num_elements = ct->num_elements;
   is_eqs = 1;
   if ((ct->data_type == SLANG_CHAR_TYPE) || (ct->data_type == SLANG_UCHAR_TYPE))
     {
	unsigned char *p, *pmax;

	p = (unsigned char *)ct->data;
	pmax = p + num_elements;

	while (p < pmax)
	  {
	     if (*p == 0)
	       {
		  is_eqs = 0;
		  break;
	       }
	     p++;
	  }
	free_array (ct);
	return is_eqs;
     }

   if (ct->data_type != SLANG_INT_TYPE)
     {
	SLang_Array_Type *tmp;
	if (1 != _pSLarray_typecast (ct->data_type, (VOID_STAR) &ct, 1,
				    SLANG_INT_TYPE, (VOID_STAR) &tmp, 1))
	  {
	     free_array (ct);
	     return -1;
	  }
	free_array (ct);
	ct = tmp;
     }

   ip = (int *)ct->data;
   ipmax = ip + num_elements;

   while (ip < ipmax)
     {
	if (*ip == 0)
	  {
	     is_eqs = 0;
	     break;
	  }
	ip++;
     }
   free_array (ct);
   return is_eqs;
}

static void is_null_intrinsic (SLang_Array_Type *at)
{
   SLang_Array_Type *bt;

   bt = SLang_create_array (SLANG_CHAR_TYPE, 0, NULL, at->dims, at->num_dims);
   if (bt == NULL)
     return;

   if (at->flags & SLARR_DATA_VALUE_IS_POINTER)
     {
	char *cdata, *cdata_max;
	char **data;

	if (-1 == coerse_array_to_linear (at))
	  {
	     free_array (bt);
	     return;
	  }

	cdata = (char *)bt->data;
	cdata_max = cdata + bt->num_elements;
	data = (char **)at->data;

	while (cdata < cdata_max)
	  {
	     if (*data == NULL)
	       *cdata = 1;

	     data++;
	     cdata++;
	  }
     }

   SLang_push_array (bt, 1);
}

static SLang_Array_Type *pop_bool_array (void)
{
   SLang_Array_Type *at;
   SLang_Array_Type *tmp_at;
   int zero;

   if (-1 == SLang_pop_array (&at, 1))
     return NULL;

   if (at->data_type == SLANG_CHAR_TYPE)
     return at;

   tmp_at = at;
   zero = 0;
   if (1 != array_binary_op (SLANG_NE,
			     SLANG_ARRAY_TYPE, (VOID_STAR) &at, 1,
			     SLANG_CHAR_TYPE, (VOID_STAR) &zero, 1,
			     (VOID_STAR) &tmp_at))
     {
	free_array (at);
	return NULL;
     }

   free_array (at);
   at = tmp_at;
   if (at->data_type != SLANG_CHAR_TYPE)
     {
	free_array (at);
	SLang_set_error (SL_TYPE_MISMATCH);
	return NULL;
     }
   return at;
}

static int pop_bool_array_and_start (int nargs, SLang_Array_Type **atp, SLindex_Type *sp)
{
   SLang_Array_Type *at;
   SLindex_Type istart = *sp;
   SLindex_Type num_elements;

   if (nargs == 2)
     {
	if (-1 == SLang_pop_array_index (&istart))
	  return -1;
     }

   if (NULL == (at = pop_bool_array ()))
     return -1;

   num_elements = (SLindex_Type) at->num_elements;

   if (istart < 0)
     istart += num_elements;

   if (istart < 0)
     {
	if (num_elements == 0)
	  istart = 0;
	else
	  {
	     SLang_set_error (SL_Index_Error);
	     free_array (at);
	     return -1;
	  }
     }

   *atp = at;
   *sp = istart;
   return 0;
}

/* Usage: i = wherefirst (at [,startpos]); */
static void array_where_first (void)
{
   SLang_Array_Type *at;
   char *a_data;
   SLindex_Type i, num_elements;
   SLindex_Type istart = 0;

   istart = 0;
   if (-1 == pop_bool_array_and_start (SLang_Num_Function_Args, &at, &istart))
     return;

   a_data = (char *) at->data;
   num_elements = (SLindex_Type) at->num_elements;

   for (i = istart; i < num_elements; i++)
     {
	if (a_data[i] == 0)
	  continue;

	(void) SLang_push_int (i);
	free_array (at);
	return;
     }
   free_array (at);
   SLang_push_null ();
}

/* Usage: i = wherelast (at [,startpos]); */
static void array_where_last (void)
{
   SLang_Array_Type *at;
   char *a_data;
   SLindex_Type i;
   SLindex_Type istart = 0;

   istart = -1;
   if (-1 == pop_bool_array_and_start (SLang_Num_Function_Args, &at, &istart))
     return;

   a_data = (char *) at->data;

   i = istart + 1;
   if (i > (SLindex_Type)at->num_elements)
     i = (SLindex_Type) at->num_elements;
   while (i > 0)
     {
	i--;
	if (a_data[i] == 0)
	  continue;

	(void) SLang_push_int (i);
	free_array (at);
	return;
     }
   free_array (at);
   SLang_push_null ();
}

static void array_where_intern (int cmp)
{
   SLang_Array_Type *at, *bt;
   char *a_data;
   SLindex_Type *b_data;
   SLuindex_Type i, num_elements;
   SLindex_Type b_num;
   SLang_Ref_Type *ref = NULL;

   if (SLang_Num_Function_Args == 2)
     {
	if (-1 == SLang_pop_ref (&ref))
	  return;
     }

   if (NULL == (at = pop_bool_array ()))
     return;

   a_data = (char *) at->data;
   num_elements = at->num_elements;

   b_num = 0;
   for (i = 0; i < num_elements; i++)
     if (cmp == (a_data[i] != 0)) b_num++;

   if (NULL == (bt = SLang_create_array1 (SLANG_ARRAY_INDEX_TYPE, 0, NULL, &b_num, 1, 1)))
     goto return_error;

   b_data = (SLindex_Type *) bt->data;

   if (ref != NULL)
     {
	SLindex_Type *c_data;
	SLindex_Type c_num;
	SLang_Array_Type *ct;

	c_num = num_elements - b_num;
	if (NULL == (ct = SLang_create_array1 (SLANG_ARRAY_INDEX_TYPE, 0, NULL, &c_num, 1, 1)))
	  goto return_error;
	c_data = (SLindex_Type *) ct->data;

	for (i = 0; i < num_elements; i++)
	  {
	     if (cmp == (a_data[i] != 0))
	       *b_data++ = i;
	     else
	       *c_data++ = i;
	  }
	(void) SLang_assign_to_ref (ref, SLANG_ARRAY_TYPE, &ct);
	/* Let any error propagate */
	free_array (ct);
	/* drop */
     }
   else
     {
	i = 0;
	while (b_num)
	  {
	     if (cmp == (a_data[i] != 0))
	       {
		  *b_data++ = i;
		  b_num--;
	       }
	     i++;
	  }
     }

   (void) SLang_push_array (bt, 0);
   /* drop */

   return_error:
   free_array (at);
   free_array (bt);
   if (ref != NULL)
     SLang_free_ref (ref);
}

static void array_where (void)
{
   array_where_intern (1);
}
static void array_wherenot (void)
{
   array_where_intern (0);
}

/* Up to the caller to ensure that ind_at is an index array */
static int do_array_reshape (SLang_Array_Type *at, SLang_Array_Type *ind_at)
{
   SLindex_Type *dims;
   unsigned int i, num_dims;
   SLuindex_Type num_elements;

   num_dims = ind_at->num_elements;
   dims = (SLindex_Type *) ind_at->data;

   num_elements = 1;
   for (i = 0; i < num_dims; i++)
     {
	SLindex_Type d = dims[i];
	if (d < 0)
	  {
	     _pSLang_verror (SL_INVALID_PARM, "reshape: dimension is less then 0");
	     return -1;
	  }

	num_elements = (SLuindex_Type) (d * num_elements);
     }

   if ((num_elements != at->num_elements)
       || (num_dims > SLARRAY_MAX_DIMS))
     {
	_pSLang_verror (SL_INVALID_PARM, "Unable to reshape array to specified size");
	return -1;
     }

   for (i = 0; i < num_dims; i++)
     at->dims [i] = dims[i];

   while (i < SLARRAY_MAX_DIMS)
     {
	at->dims [i] = 1;
	i++;
     }

   at->num_dims = num_dims;
   return 0;
}

static int pop_1d_index_array (SLang_Array_Type **ind_atp)
{
   SLang_Array_Type *ind_at;

   *ind_atp = NULL;
   if (-1 == SLang_pop_array_of_type (&ind_at, SLANG_ARRAY_INDEX_TYPE))
     return -1;
   if (ind_at->num_dims != 1)
     {
	_pSLang_verror (SL_TYPE_MISMATCH, "Expecting 1-d array of indices");
	return -1;
     }
   *ind_atp = ind_at;
   return 0;
}

static int pop_reshape_args (SLang_Array_Type **atp, SLang_Array_Type **ind_atp)
{
   SLang_Array_Type *at, *ind_at;

   *ind_atp = *atp = NULL;

   if (-1 == pop_1d_index_array (&ind_at))
     return -1;

   if (-1 == SLang_pop_array (&at, 1))
     {
	free_array (ind_at);
	return -1;
     }

   *atp = at;
   *ind_atp = ind_at;
   return 0;
}

static void array_reshape (void)
{
   SLang_Array_Type *at, *ind_at;

   if (-1 == pop_reshape_args (&at, &ind_at))
     return;
   (void) do_array_reshape (at, ind_at);
   free_array (at);
   free_array (ind_at);
}

static void _array_reshape (void)
{
   SLang_Array_Type *at;
   SLang_Array_Type *new_at;
   SLang_Array_Type *ind_at;

   if (-1 == pop_reshape_args (&at, &ind_at))
     return;

   /* FIXME: Priority=low: duplicate_array could me modified to look at num_refs */

   /* Now try to avoid the overhead of creating a new array if possible */
   if (at->num_refs == 1)
     {
	/* Great, we are the sole owner of this array. */
	if ((-1 == do_array_reshape (at, ind_at))
	    || (-1 == SLclass_push_ptr_obj (SLANG_ARRAY_TYPE, (VOID_STAR)at)))
	  free_array (at);
	free_array (ind_at);
	return;
     }

   new_at = SLang_duplicate_array (at);
   if (new_at != NULL)
     {
	if (0 == do_array_reshape (new_at, ind_at))
	  (void) SLang_push_array (new_at, 0);

	free_array (new_at);
     }
   free_array (at);
   free_array (ind_at);
}

typedef struct
{
   SLang_Array_Type *at;
   int is_array;
   size_t increment;
   char *addr;
}
Map_Arg_Type;
/* Usage: array_map (Return-Type, func, args,....); */
static void array_map (void)
{
   Map_Arg_Type *args;
   unsigned int num_args;
   unsigned int i, i_control;
   SLang_Name_Type *nt;
   unsigned int num_elements;
   SLang_Array_Type *at;
   char *addr;
   SLtype type;

   at = NULL;
   args = NULL;
   nt = NULL;

   if (SLang_Num_Function_Args < 3)
     {
	_pSLang_verror (SL_INVALID_PARM,
		      "Usage: array_map (Return-Type, &func, args...)");
	SLdo_pop_n (SLang_Num_Function_Args);
	return;
     }

   num_args = (unsigned int)SLang_Num_Function_Args - 2;
   args = (Map_Arg_Type *) _SLcalloc (num_args, sizeof (Map_Arg_Type));
   if (args == NULL)
     {
	SLdo_pop_n (SLang_Num_Function_Args);
	return;
     }
   memset ((char *) args, 0, num_args * sizeof (Map_Arg_Type));
   i = num_args;
   i_control = 0;
   while (i > 0)
     {
	i--;
	if (SLANG_ARRAY_TYPE == SLang_peek_at_stack ())
	  {
	     args[i].is_array = 1;
	     i_control = i;
	  }

	if (-1 == SLang_pop_array (&args[i].at, 1))
	  {
	     SLdo_pop_n (i + 2);
	     goto return_error;
	  }
     }

   if (NULL == (nt = SLang_pop_function ()))
     {
	SLdo_pop_n (1);
	goto return_error;
     }

   num_elements = args[i_control].at->num_elements;

   if (-1 == SLang_pop_datatype (&type))
     goto return_error;

   if (type == SLANG_UNDEFINED_TYPE)   /* Void_Type */
     at = NULL;
   else
     {
	at = args[i_control].at;

	if (NULL == (at = SLang_create_array (type, 0, NULL, at->dims, at->num_dims)))
	  goto return_error;
     }

   for (i = 0; i < num_args; i++)
     {
	SLang_Array_Type *ati = args[i].at;
	/* FIXME: Priority = low: The actual dimensions should be compared. */
	if (ati->num_elements == num_elements)
	  args[i].increment = ati->sizeof_type;
	/* memset already guarantees increment to be zero */

	if ((num_elements != 0)
	    && (ati->num_elements == 0))
	  {
	     _pSLang_verror (SL_TypeMismatch_Error, "array_map: function argument %d of %d is an empty array",
			   i+1, num_args);
	     goto return_error;
	  }

	args[i].addr = (char *) ati->data;
     }

   if (at == NULL)
     addr = NULL;
   else
     addr = (char *)at->data;

   for (i = 0; i < num_elements; i++)
     {
	unsigned int j;

	if (-1 == SLang_start_arg_list ())
	  goto return_error;

	for (j = 0; j < num_args; j++)
	  {
	     if (-1 == push_element_at_addr (args[j].at,
					     (VOID_STAR) args[j].addr,
					     1))
	       {
		  SLdo_pop_n (j);
		  goto return_error;
	       }

	     args[j].addr += args[j].increment;
	  }

	if (-1 == SLang_end_arg_list ())
	  {
	     SLdo_pop_n (num_args);
	     goto return_error;
	  }

	if (-1 == SLexecute_function (nt))
	  goto return_error;

	if (at == NULL)
	  continue;

	if (-1 == at->cl->cl_apop (type, (VOID_STAR) addr))
	  goto return_error;

	addr += at->sizeof_type;
     }

   if (at != NULL)
     (void) SLang_push_array (at, 0);

   /* drop */

   return_error:
   free_array (at);
   SLang_free_function (nt);
   if (args != NULL)
     {
	for (i = 0; i < num_args; i++)
	  free_array (args[i].at);

	SLfree ((char *) args);
     }
}

static int push_array_shape (SLang_Array_Type *at)
{
   SLang_Array_Type *bt;
   SLindex_Type num_dims;
   SLindex_Type *bdata, *a_dims;
   int i;

   num_dims = (SLindex_Type)at->num_dims;
   if (NULL == (bt = SLang_create_array (SLANG_ARRAY_INDEX_TYPE, 0, NULL, &num_dims, 1)))
     return -1;

   a_dims = at->dims;
   bdata = (SLindex_Type *) bt->data;
   for (i = 0; i < num_dims; i++) bdata [i] = a_dims [i];

   return SLang_push_array (bt, 1);
}

static void array_info (void)
{
   SLang_Array_Type *at;

   if (-1 == pop_array (&at, 1))
     return;

   if (0 == push_array_shape (at))
     {
	(void) SLang_push_integer ((int) at->num_dims);
	(void) SLang_push_datatype (at->data_type);
     }
   free_array (at);
}

static void array_shape (void)
{
   SLang_Array_Type *at;

   if (-1 == pop_array (&at, 1))
     return;

   (void) push_array_shape (at);
   free_array (at);
}

#if 0
static int pop_int_indices (SLindex_Type *dims, unsigned int ndims)
{
   int i;

   if (ndims > SLARRAY_MAX_DIMS)
     {
	_pSLang_verror (SL_INVALID_PARM, "Too many dimensions specified");
	return -1;
     }
   for (i = (int)ndims-1; i >= 0; i--)
     {
	if (-1 == SLang_pop_integer (dims+i))
	  return -1;
     }
   return 0;
}
/* Usage: aput(v, x, i,..,k) */
static void aput_intrin (void)
{
   char *data_to_put;
   SLuindex_Type data_increment;
   SLindex_Type indices[SLARRAY_MAX_DIMS];
   int is_ptr;
   unsigned int ndims = SLang_Num_Function_Args-2;
   SLang_Array_Type *at, *bt_unused;

   if (-1 == pop_int_indices (indices, ndims))
     return;

   if (-1 == SLang_pop_array (&at, 1))
     return;

   if (at->num_dims != ndims)
     {
	SLang_set_error (SL_Index_Error);
	free_array (at);
	return;
     }

   is_ptr = (at->flags & SLARR_DATA_VALUE_IS_POINTER);

   if (-1 == aput_get_data_to_put (at->cl, 1, 0, &bt_unused, &data_to_put, &data_increment))
     {
	free_array (at);
	return;
     }

   (void) _pSLarray_aput_transfer_elem (at, indices, (VOID_STAR)data_to_put,
				       at->sizeof_type, is_ptr);
   if (is_ptr)
     (*at->cl->cl_destroy) (at->cl->cl_data_type, (VOID_STAR) data_to_put);

   free_array (at);
}

/* Usage: x_i..k = aget(x, i,..,k) */
static void aget_intrin (void)
{
   SLindex_Type dims[SLARRAY_MAX_DIMS];
   unsigned int ndims = (unsigned int) (SLang_Num_Function_Args-1);
   SLang_Array_Type *at;
   VOID_STAR data;

   if (-1 == pop_int_indices (dims, ndims))
     return;

   if (-1 == pop_array (&at, 1))
     return;

   if (at->num_dims != ndims)
     {
	SLang_set_error (SL_Index_Error);
	free_array (at);
	return;
     }

   if ((ndims == 1)
       && (at->index_fun == linear_get_data_addr))
     {
	SLindex_Type i = dims[0];
	if (i < 0)
	  {
	     i += at->dims[0];
	     if (i < 0)
	       i = at->dims[0];
	  }
	if (i >= at->dims[0])
	  {
	     SLang_set_error (SL_Index_Error);
	     free_array (at);
	     return;
	  }
	if (at->data_type == SLANG_INT_TYPE)
	  {
	     (void) SLclass_push_int_obj (SLANG_INT_TYPE, *((int *)at->data + i));
	     goto free_and_return;
	  }
#if SLANG_HAS_FLOAT
	if (at->data_type == SLANG_DOUBLE_TYPE)
	  {
	     (void) SLclass_push_double_obj (SLANG_DOUBLE_TYPE, *((double *)at->data + i));
	     goto free_and_return;
	  }
#endif
	if (at->data_type == SLANG_CHAR_TYPE)
	  {
	     (void) SLclass_push_int_obj (SLANG_UCHAR_TYPE, *((unsigned char *)at->data + i));
	     goto free_and_return;
	  }
	data = (VOID_STAR) ((char *)at->data + (SLuindex_Type)i * at->sizeof_type);
     }
   else data = get_data_addr (at, dims);

   if (data != NULL)
     (void) push_element_at_addr (at, (VOID_STAR) data, ndims);

   free_and_return:
   free_array (at);
}
#endif

static SLang_Intrin_Fun_Type Array_Table [] =
{
   MAKE_INTRINSIC_0("array_map", array_map, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("array_sort", array_sort_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("get_default_sort_method", get_default_sort_method, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_S("set_default_sort_method", set_default_sort_method, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_1("array_to_bstring", array_to_bstring, SLANG_VOID_TYPE, SLANG_ARRAY_TYPE),
   MAKE_INTRINSIC_1("bstring_to_array", bstring_to_array, SLANG_VOID_TYPE, SLANG_BSTRING_TYPE),
   MAKE_INTRINSIC("init_char_array", init_char_array, SLANG_VOID_TYPE, 0),
   MAKE_INTRINSIC_1("_isnull", is_null_intrinsic, SLANG_VOID_TYPE, SLANG_ARRAY_TYPE),
   MAKE_INTRINSIC_0("array_info", array_info, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("array_shape", array_shape, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("where", array_where, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("wherenot", array_wherenot, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("wherefirst", array_where_first, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("wherelast", array_where_last, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("reshape", array_reshape, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_reshape", _array_reshape, SLANG_VOID_TYPE),
#if 0
   MAKE_INTRINSIC_0("__aget", aget_intrin, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("__aput", aput_intrin, SLANG_VOID_TYPE),
#endif
   SLANG_END_INTRIN_FUN_TABLE
};

static char *array_string (SLtype type, VOID_STAR v)
{
   SLang_Array_Type *at;
   char buf[512];
   unsigned int i, num_dims;
   SLindex_Type *dims;

   at = *(SLang_Array_Type **) v;
   type = at->data_type;
   num_dims = at->num_dims;
   dims = at->dims;

   sprintf (buf, "%s[%ld", SLclass_get_datatype_name (type), (long)at->dims[0]);

   for (i = 1; i < num_dims; i++)
     sprintf (buf + strlen(buf), ",%ld", (long)dims[i]);
   strcat (buf, "]");

   return SLmake_string (buf);
}

static void array_destroy (SLtype type, VOID_STAR v)
{
   SLang_Array_Type *at = *(SLang_Array_Type **) v;

   (void) type;
   if ((at != NULL)
       && (at->num_refs > 1))
     {
	at->num_refs -= 1;
	return;
     }
   free_array (*(SLang_Array_Type **) v);
}

static int array_push (SLtype type, VOID_STAR v)
{
   SLang_Array_Type *at;

   (void) type;

   at = *(SLang_Array_Type **) v;
   if (at == NULL)
     return SLang_push_null ();

   at->num_refs += 1;

   if (0 == SLclass_push_ptr_obj (SLANG_ARRAY_TYPE, (VOID_STAR) at))
     return 0;

   at->num_refs -= 1;
   return -1;
}

int SLang_push_array (SLang_Array_Type *at, int free_flag)
{
   if (at == NULL)
     return SLang_push_null ();

   return _pSLang_push_array (at, free_flag);
}

/* Intrinsic arrays are not stored in a variable. So, the address that
 * would contain the variable holds the array address.
 */
static int array_push_intrinsic (SLtype type, VOID_STAR v)
{
   (void) type;
   return SLang_push_array ((SLang_Array_Type *) v, 0);
}

int _pSLarray_add_bin_op (SLtype type)
{
   SL_OOBinary_Type *ab;
   SLang_Class_Type *cl;

   if (type == SLANG_VOID_TYPE)
     {
	cl = _pSLclass_get_class (SLANG_ARRAY_TYPE);
	if ((cl->cl_this_binary_void != NULL)
	    || (cl->cl_void_binary_this != NULL))
	  return 0;
     }
   else
     {
	cl = _pSLclass_get_class (type);
	ab = cl->cl_binary_ops;

	while (ab != NULL)
	  {
	     if (ab->data_type == SLANG_ARRAY_TYPE)
	       return 0;
	     ab = ab->next;
	  }
     }

   if ((-1 == SLclass_add_binary_op (SLANG_ARRAY_TYPE, type, array_binary_op, array_binary_op_result))
       || (-1 == SLclass_add_binary_op (type, SLANG_ARRAY_TYPE, array_binary_op, array_binary_op_result)))
     return -1;

   return 0;
}

static SLang_Array_Type *
do_array_math_op (int op, int unary_type,
		  SLang_Array_Type *at, unsigned int na)
{
   SLtype a_type, b_type;
   int (*f) (int, SLtype, VOID_STAR, SLuindex_Type, VOID_STAR);
   SLang_Array_Type *bt;
   SLang_Class_Type *b_cl;
   int no_init;

   if (na != 1)
     {
	_pSLang_verror (SL_NOT_IMPLEMENTED, "Operation restricted to 1 array");
	return NULL;
     }

   a_type = at->data_type;
   if (NULL == (f = _pSLclass_get_unary_fun (op, at->cl, &b_cl, unary_type)))
     return NULL;
   b_type = b_cl->cl_data_type;

   if (-1 == coerse_array_to_linear (at))
     return NULL;

   no_init = ((b_cl->cl_class_type == SLANG_CLASS_TYPE_SCALAR)
	      || (b_cl->cl_class_type == SLANG_CLASS_TYPE_VECTOR));

#if SLANG_USE_TMP_OPTIMIZATION
   /* If we are dealing with scalar (or vector) objects, and if the object
    * appears to be owned by the stack, then use it instead of creating a
    * new version.  This can happen with code such as:
    * @  x = [1,2,3,4];
    * @  x = UNARY_OP(__tmp(x));
    */
   if (no_init
       && (at->num_refs == 1)
       && (at->data_type == b_cl->cl_data_type)
       && (0 == (at->flags & SLARR_DATA_VALUE_IS_READ_ONLY)))
     {
	bt = at;
	bt->num_refs = 2;
     }
   else
#endif				       /* SLANG_USE_TMP_OPTIMIZATION */
     if (NULL == (bt = SLang_create_array1 (b_type, 0, NULL, at->dims, at->num_dims, 1)))
       return NULL;

   if (1 != (*f)(op, a_type, at->data, at->num_elements, bt->data))
     {
	free_array (bt);
	return NULL;
     }
   return bt;
}

static int
array_unary_op_result (int op, SLtype a, SLtype *b)
{
   (void) op;
   (void) a;
   *b = SLANG_ARRAY_TYPE;
   return 1;
}

static int
array_unary_op (int op,
		SLtype a, VOID_STAR ap, SLuindex_Type na,
		VOID_STAR bp)
{
   SLang_Array_Type *at;

   (void) a;
   at = *(SLang_Array_Type **) ap;
   if (NULL == (at = do_array_math_op (op, SLANG_BC_UNARY, at, na)))
     {
	if (SLang_get_error ()) return -1;
	return 0;
     }
   *(SLang_Array_Type **) bp = at;
   return 1;
}

static int
array_math_op (int op,
	       SLtype a, VOID_STAR ap, SLuindex_Type na,
	       VOID_STAR bp)
{
   SLang_Array_Type *at;

   (void) a;
   at = *(SLang_Array_Type **) ap;
   if (NULL == (at = do_array_math_op (op, SLANG_BC_MATH_UNARY, at, na)))
     {
	if (SLang_get_error ()) return -1;
	return 0;
     }
   *(SLang_Array_Type **) bp = at;
   return 1;
}

static int
array_app_op (int op,
	      SLtype a, VOID_STAR ap, SLuindex_Type na,
	      VOID_STAR bp)
{
   SLang_Array_Type *at;

   (void) a;
   at = *(SLang_Array_Type **) ap;
   if (NULL == (at = do_array_math_op (op, SLANG_BC_APP_UNARY, at, na)))
     {
	if (SLang_get_error ()) return -1;
	return 0;
     }
   *(SLang_Array_Type **) bp = at;
   return 1;
}

/* Typecast array from a_type to b_type */
int
_pSLarray_typecast (SLtype a_type, VOID_STAR ap, SLuindex_Type na,
		    SLtype b_type, VOID_STAR bp,
		    int is_implicit)
{
   SLang_Array_Type *at, *bt;
   SLang_Class_Type *b_cl;
   int no_init;
   int (*t) (SLtype, VOID_STAR, SLuindex_Type, SLtype, VOID_STAR);

   if (na != 1)
     {
	_pSLang_verror (SL_NOT_IMPLEMENTED, "typecast of multiple arrays not implemented");
	return -1;
     }

   at = *(SLang_Array_Type **) ap;
   a_type = at->data_type;

   if (a_type == b_type)
     {
	at->num_refs += 1;
	*(SLang_Array_Type **) bp = at;
	return 1;
     }

   /* check for alias */
   if (at->cl == (b_cl = _pSLclass_get_class (b_type)))
     {
	at->num_refs += 1;

	/* Force to the desired type.  Hopefully there will be no consequences
	 * from this.
	 */
	at->data_type = b_cl->cl_data_type;
	*(SLang_Array_Type **) bp = at;
	return 1;
     }

   if (at->flags & SLARR_DATA_VALUE_IS_RANGE)
     {
	if (-1 == try_typecast_range_array (at, b_type, &bt))
	  return -1;
	if (bt != NULL)
	  {
	     *(SLang_Array_Type **) bp = bt;
	     return 1;
	  }
	/* Couldn't do it, so drop */
     }

   /* Typecast NULL array to the desired type with elements set to NULL */
   if ((a_type == SLANG_NULL_TYPE)
       && ((b_cl->cl_class_type == SLANG_CLASS_TYPE_MMT)
	   || (b_cl->cl_class_type == SLANG_CLASS_TYPE_PTR)))
     {
	if (NULL == (bt = SLang_create_array1 (b_type, 0, NULL, at->dims, at->num_dims, 0)))
	  return -1;

	*(SLang_Array_Type **) bp = bt;
	return 1;
     }

   if (NULL == (t = _pSLclass_get_typecast (a_type, b_type, is_implicit)))
     return -1;

   if (-1 == coerse_array_to_linear (at))
     return -1;

   no_init = ((b_cl->cl_class_type == SLANG_CLASS_TYPE_SCALAR)
	      || (b_cl->cl_class_type == SLANG_CLASS_TYPE_VECTOR));

   if (NULL == (bt = SLang_create_array1 (b_type, 0, NULL, at->dims, at->num_dims, no_init)))
     return -1;

   if (1 == (*t) (a_type, at->data, at->num_elements, b_type, bt->data))
     {
	*(SLang_Array_Type **) bp = bt;
	return 1;
     }

   free_array (bt);
   return 0;
}

static SLang_Array_Type *duplicate_range_array (SLang_Array_Type *at)
{
   SLarray_Range_Array_Type *r = (SLarray_Range_Array_Type *)at->data;
   return create_range_array (r, (SLindex_Type)at->num_elements, at->data_type, r->to_linear_fun);
}

SLang_Array_Type *SLang_duplicate_array (SLang_Array_Type *at)
{
   SLang_Array_Type *bt;
   char *data, *a_data;
   SLuindex_Type i, num_elements;
   size_t sizeof_type, size;
   int (*cl_acopy) (SLtype, VOID_STAR, VOID_STAR);
   SLtype type;

   if (at->flags & SLARR_DATA_VALUE_IS_RANGE)
     return duplicate_range_array (at);

   if (-1 == coerse_array_to_linear (at))
     return NULL;

   type = at->data_type;
   num_elements = at->num_elements;
   sizeof_type = at->sizeof_type;

   if (NULL == (data = _SLcalloc (num_elements, sizeof_type)))
     return NULL;

   size = num_elements * sizeof_type;

   if (NULL == (bt = SLang_create_array (type, 0, (VOID_STAR)data, at->dims, at->num_dims)))
     {
	SLfree (data);
	return NULL;
     }

   a_data = (char *) at->data;
   if (0 == (at->flags & SLARR_DATA_VALUE_IS_POINTER))
     {
	SLMEMCPY (data, a_data, size);
	return bt;
     }

   SLMEMSET (data, 0, size);

   cl_acopy = at->cl->cl_acopy;
   for (i = 0; i < num_elements; i++)
     {
	if (NULL != *(VOID_STAR *) a_data)
	  {
	     if (-1 == (*cl_acopy) (type, (VOID_STAR) a_data, (VOID_STAR) data))
	       {
		  free_array (bt);
		  return NULL;
	       }
	  }

	data += sizeof_type;
	a_data += sizeof_type;
     }

   return bt;
}

static int array_dereference (SLtype type, VOID_STAR addr)
{
   SLang_Array_Type *at;

   (void) type;
   at = SLang_duplicate_array (*(SLang_Array_Type **) addr);
   if (at == NULL) return -1;
   return SLang_push_array (at, 1);
}

/* This function gets called via, e.g., @Array_Type (Double_Type, [10,20]);
 */
static int
array_datatype_deref (SLtype type)
{
   SLang_Array_Type *ind_at;
   SLang_Array_Type *at;

#if 0
   /* The parser generated code for this as if a function call were to be
    * made.  However, the interpreter simply called the deref object routine
    * instead of the function call.  So, I must simulate the function call.
    * This needs to be formalized to hide this detail from applications
    * who wish to do the same.  So...
    * FIXME: Priority=medium
    */
   if (0 == _pSL_increment_frame_pointer ())
     (void) _pSL_decrement_frame_pointer ();
#endif

   if (-1 == pop_1d_index_array (&ind_at))
     goto return_error;

   if (-1 == SLang_pop_datatype (&type))
     goto return_error;

   if (NULL == (at = SLang_create_array (type, 0, NULL,
					 (SLindex_Type *) ind_at->data,
					 ind_at->num_elements)))
     goto return_error;

   free_array (ind_at);
   return SLang_push_array (at, 1);

   return_error:
   free_array (ind_at);
   return -1;
}

static int array_length (SLtype type, VOID_STAR v, SLuindex_Type *len)
{
   SLang_Array_Type *at;

   (void) type;
   at = *(SLang_Array_Type **) v;
   *len = at->num_elements;
   return 0;
}

static void array_inc_ref (SLtype type, VOID_STAR v, int dr)
{
   SLang_Array_Type *at = *(SLang_Array_Type **) v;
   (void) type;
   if (at != NULL)
     at->num_refs += dr;
}

int
_pSLarray_init_slarray (void)
{
   SLang_Class_Type *cl;

   if (-1 == SLadd_intrin_fun_table (Array_Table, NULL))
     return -1;

   if (NULL == (cl = SLclass_allocate_class ("Array_Type")))
     return -1;

   (void) SLclass_set_string_function (cl, array_string);
   (void) SLclass_set_destroy_function (cl, array_destroy);
   (void) SLclass_set_push_function (cl, array_push);
   cl->cl_push_intrinsic = array_push_intrinsic;
   cl->cl_dereference = array_dereference;
   cl->cl_datatype_deref = array_datatype_deref;
   cl->cl_length = array_length;
   cl->is_container = 1;
   cl->cl_inc_ref = array_inc_ref;

   (void) SLclass_set_eqs_function (cl, array_eqs_method);

   if (-1 == SLclass_register_class (cl, SLANG_ARRAY_TYPE, sizeof (VOID_STAR),
				     SLANG_CLASS_TYPE_PTR))
     return -1;

   if ((-1 == SLclass_add_binary_op (SLANG_ARRAY_TYPE, SLANG_ARRAY_TYPE, array_binary_op, array_binary_op_result))
       || (-1 == SLclass_add_unary_op (SLANG_ARRAY_TYPE, array_unary_op, array_unary_op_result))
       || (-1 == SLclass_add_app_unary_op (SLANG_ARRAY_TYPE, array_app_op, array_unary_op_result))
       || (-1 == SLclass_add_math_op (SLANG_ARRAY_TYPE, array_math_op, array_unary_op_result)))
     return -1;

   return 0;
}

int SLang_pop_array (SLang_Array_Type **at_ptr, int convert_scalar)
{
   SLang_Array_Type *at;

   if (-1 == pop_array (&at, convert_scalar))
     {
	*at_ptr = NULL;
	return -1;
     }

   if (-1 == coerse_array_to_linear (at))
     {
	free_array (at);
	*at_ptr = NULL;
	return -1;
     }
   *at_ptr = at;
   return 0;
}

int SLang_pop_array_of_type (SLang_Array_Type **at, SLtype type)
{
   if (-1 == SLclass_typecast (type, 1, 1))
     return -1;

   return SLang_pop_array (at, 1);
}

void (*_pSLang_Matrix_Multiply)(void);

int _pSLarray_matrix_multiply (void)
{
   if (_pSLang_Matrix_Multiply != NULL)
     {
	(*_pSLang_Matrix_Multiply)();
	return 0;
     }
   _pSLang_verror (SL_NOT_IMPLEMENTED, "Matrix multiplication not available");
   return -1;
}

struct _pSLang_Foreach_Context_Type
{
   SLang_Array_Type *at;
   SLindex_Type next_element_index;
};

SLang_Foreach_Context_Type *
_pSLarray_cl_foreach_open (SLtype type, unsigned int num)
{
   SLang_Foreach_Context_Type *c;

   if (num != 0)
     {
	SLdo_pop_n (num + 1);
	_pSLang_verror (SL_NOT_IMPLEMENTED,
		      "%s does not support 'foreach using' form",
		      SLclass_get_datatype_name (type));
	return NULL;
     }

   if (NULL == (c = (SLang_Foreach_Context_Type *) SLmalloc (sizeof (SLang_Foreach_Context_Type))))
     return NULL;

   memset ((char *) c, 0, sizeof (SLang_Foreach_Context_Type));

   if (-1 == pop_array (&c->at, 1))
     {
	SLfree ((char *) c);
	return NULL;
     }

   return c;
}

void _pSLarray_cl_foreach_close (SLtype type, SLang_Foreach_Context_Type *c)
{
   (void) type;
   if (c == NULL) return;
   free_array (c->at);
   SLfree ((char *) c);
}

int _pSLarray_cl_foreach (SLtype type, SLang_Foreach_Context_Type *c)
{
   SLang_Array_Type *at;
   VOID_STAR data;

   (void) type;

   if (c == NULL)
     return -1;

   at = c->at;
   /* Use <= here to prepare for the time when arrays are permitted to change size */
   if ((SLindex_Type)at->num_elements <= c->next_element_index)
     return 0;

   /* FIXME: Priority = low.  The following assumes linear arrays
    * or Integer range arrays.  Fixing it right requires a method to get the
    * nth element of a multidimensional array.
    */

   if (at->flags & SLARR_DATA_VALUE_IS_RANGE)
     {
	SLindex_Type d = (SLindex_Type) c->next_element_index;
	data = range_get_data_addr (at, &d);
     }
   else
     data = (VOID_STAR) ((char *)at->data + (c->next_element_index * at->sizeof_type));

   c->next_element_index += 1;

   if ((at->flags & SLARR_DATA_VALUE_IS_POINTER)
       && (*(VOID_STAR *) data == NULL))
     {
	if (-1 == SLang_push_null ())
	  return -1;
     }
   else if (-1 == (*at->cl->cl_apush)(at->data_type, data))
     return -1;

   /* keep going */
   return 1;
}

/* References to array elements */
typedef struct
{
   SLang_Object_Type at;
   SLang_Object_Type index_objs [SLARRAY_MAX_DIMS];
   unsigned int num_indices;
}
Array_Elem_Ref_Type;

static int elem_ref_push_index_objs (Array_Elem_Ref_Type *ert)
{
   SLang_Object_Type *o, *omax;

   o = ert->index_objs;
   omax = o + ert->num_indices;

   while (o < omax)
     {
	if (-1 == _pSLpush_slang_obj (o))
	  return -1;
	o++;
     }
   if (-1 == _pSLpush_slang_obj (&ert->at))
     return -1;

   return 0;
}

static int elem_ref_deref_assign (VOID_STAR vdata)
{
   Array_Elem_Ref_Type *ert = (Array_Elem_Ref_Type *)vdata;

   if (-1 == elem_ref_push_index_objs (ert))
     return -1;

   return _pSLarray_aput1 (ert->num_indices);
}

static int elem_ref_deref (VOID_STAR vdata)
{
   Array_Elem_Ref_Type *ert = (Array_Elem_Ref_Type *)vdata;

   if (-1 == elem_ref_push_index_objs (ert))
     return -1;

   return _pSLarray_aget1 (ert->num_indices);
}

static void elem_ref_destroy (VOID_STAR vdata)
{
   Array_Elem_Ref_Type *ert = (Array_Elem_Ref_Type *)vdata;
   SLang_Object_Type *o, *omax;

   if (ert->at.o_data_type != 0)
     SLang_free_object (&ert->at);
   o = ert->index_objs;
   omax = o + ert->num_indices;
   while (o < omax)
     {
	if (o->o_data_type != 0)
	  SLang_free_object (o);
	o++;
     }
}

/* &A[i,...j] ==> __args i..j A ARRAY_REF */
int _pSLarray_push_elem_ref (void)
{
   unsigned int num_indices = (unsigned int) (SLang_Num_Function_Args-1);
   Array_Elem_Ref_Type *ert;
   SLang_Ref_Type *ref;
   unsigned int i;
   int ret;

   if (num_indices > SLARRAY_MAX_DIMS)
     {
	_pSLang_verror (SL_INVALID_PARM, "Number of dims must be less than %d", 1+SLARRAY_MAX_DIMS);
	return -1;
     }

   if (NULL == (ref = _pSLang_new_ref (sizeof (Array_Elem_Ref_Type))))
     return -1;

   ref->deref = elem_ref_deref;
   ref->deref_assign = elem_ref_deref_assign;
   ref->destroy = elem_ref_destroy;

   ert = (Array_Elem_Ref_Type *) ref->data;
   ert->num_indices = num_indices;
   if (-1 == SLang_pop (&ert->at))
     {
	SLang_free_ref (ref);
	return -1;
     }

   i = num_indices;
   while (i)
     {
	i--;
	if (-1 == SLang_pop (ert->index_objs + i))
	  {
	     SLang_free_ref (ref);
	     return -1;
	  }
     }
   ret = SLang_push_ref (ref);
   SLang_free_ref (ref);
   return ret;
}
