/* List of objects */
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

/* A list is a heterogeneous collection of objects.  The list container
 * object here supports the following operations:
 *
 *    append (list, a) ==> list = {list, a}.
 *    prepend (list, a) ==> list = {a, list}.
 *    extract (list, n) ==> list[n]  (n = 0, ...)
 *    insert (list, a, n) ==> list = {list[0],...list[n-1],a,list[n],...}
 *    delete (list, n) ==> list = {list[0], ... list[n-1],list[n+1],...}
 *    length (list)  ==> num elements
 *    eqs (listA, listB)
 *    reverse (list)
 *    copy (list)
 */

typedef struct _pSLang_List_Type SLang_List_Type;

#define CHUNK_SIZE 128

typedef struct _Chunk_Type
{
   struct _Chunk_Type *next;
   struct _Chunk_Type *prev;
   int num_elements;
   SLang_Object_Type *elements;	       /* CHUNK_SIZE of em */
}
Chunk_Type;

struct _pSLang_List_Type
{
   SLindex_Type length;
   Chunk_Type *first;
   Chunk_Type *last;

   Chunk_Type *recent;		       /* most recent chunk accessed */
   SLindex_Type recent_num;	       /* num elements before the recent chunk */
};

static void delete_chunk (Chunk_Type *c)
{
   unsigned int i, n;
   SLang_Object_Type *objs;

   if (c == NULL)
     return;

   n = c->num_elements;
   objs = c->elements;
   for (i = 0; i < n; i++)
     SLang_free_object (objs+i);
   SLfree ((char *) objs);
   SLfree ((char *) c);
}

static Chunk_Type *new_chunk (void)
{
   Chunk_Type *c;

   c = (Chunk_Type *) SLcalloc (1, sizeof (Chunk_Type));
   if (c == NULL)
     return c;

   c->elements = (SLang_Object_Type *)SLcalloc(CHUNK_SIZE, sizeof(SLang_Object_Type));
   if (c->elements == NULL)
     {
	SLfree ((char *) c);
	return NULL;
     }
   return c;
}

static void delete_chunk_chain (Chunk_Type *c)
{
   while (c != NULL)
     {
	Chunk_Type *next;
	next = c->next;
	delete_chunk (c);
	c = next;
     }
}

static void delete_list (SLang_List_Type *list)
{
   if (list == NULL)
     return;

   delete_chunk_chain (list->first);
   SLfree ((char *) list);
}

static void list_destroy (SLtype type, VOID_STAR ptr)
{
   (void) type;
   delete_list ((SLang_List_Type *) ptr);
}

static int make_chunk_chain (int length, Chunk_Type **firstp, Chunk_Type **lastp)
{
   Chunk_Type *last, *first;

   if (NULL == (first = new_chunk ()))
     return -1;
   length -= CHUNK_SIZE;

   last = first;
   while (length > 0)
     {
	Chunk_Type *next;
	if (NULL == (next = new_chunk ()))
	  {
	     delete_chunk_chain (first);
	     return -1;
	  }
	last->next = next;
	next->prev = last;
	last = next;
	length -= CHUNK_SIZE;
     }
   *firstp = first;
   *lastp = last;
   return 0;
}

static SLang_List_Type *allocate_list (void)
{
   return (SLang_List_Type *)SLcalloc (1, sizeof (SLang_List_Type));
}

static SLang_Object_Type *find_nth_element (SLang_List_Type *list, SLindex_Type nth, Chunk_Type **cp)
{
   Chunk_Type *c, *first, *last;
   SLindex_Type n, next_n;
   SLindex_Type length;
   int dir;

   length = list->length;
   if (nth < 0)
     nth += length;

   if ((nth < 0) || (nth >= length))
     {
	_pSLang_verror (SL_Index_Error, "List Index out of range");
	return NULL;
     }

   n = 0;
   first = list->first;
   last = list->last;
   dir = 1;

   if (list->recent != NULL)
     {
	SLindex_Type mid = list->recent_num;
	if (nth < mid)
	  {
	     if (nth > mid/2)
	       {
		  dir = -1;
		  n = mid;
		  last = list->recent->prev;
		  /* Note: last cannot be NULL.  The only way for it to be NULL
		   * is for list->recent == list_first, which would mean that
		   * mid==0.  But the condition here is that nth < mid, and
		   * nth >= 0
		   */
	       }
	  }
	else
	  {
	     if (nth > (mid/2 + length/2))
	       {
		  dir = -1;
		  n = length;
	       }
	     else
	       {
		  dir = 1;
		  n = mid;
		  first = list->recent;
	       }
	  }
     }
   else if (nth > length/2)
     {
	n = length;
	dir = -1;
     }

   if (dir > 0)
     {
	c = first;
	while (nth >= (next_n = n + c->num_elements))
	  {
	     n = next_n;
	     c = c->next;
	  }
	if (cp != NULL)
	  *cp = c;

	list->recent = c;
	list->recent_num = n;

	return c->elements + (nth - n);
     }

   c = last;
   while ((next_n = n - c->num_elements) > nth)
     {
	n = next_n;
	c = c->prev;
     }

   if (cp != NULL)
     *cp = c;

   list->recent = c;
   list->recent_num = next_n;
   return c->elements + (nth - next_n);
}

static int pop_list (SLang_MMT_Type **mmtp, SLang_List_Type **list)
{
   SLang_MMT_Type *mmt;

   if (NULL == (mmt = SLang_pop_mmt (SLANG_LIST_TYPE)))
     return -1;
   *list = (SLang_List_Type *)SLang_object_from_mmt (mmt);
   *mmtp = mmt;
   return 0;
}

/* FIXME: This is currently used only by list_dereference and breaks on an
 * empty list.  For this reason, it would probably fail in other contexts.
 */
static SLang_List_Type *make_sublist (SLang_List_Type *list, SLindex_Type indx_a, SLindex_Type length)
{
   SLang_List_Type *new_list;
   Chunk_Type *c, *new_c;
   SLindex_Type i;
   SLang_Object_Type *obj, *obj_max, *new_obj, *new_obj_max;

   if (length == 0)
     return allocate_list ();

   if ((indx_a < 0) || (indx_a + (length - 1) >= list->length))
     {
	_pSLang_verror (SL_Index_Error, "Indices are out of range for list object");
	return NULL;
     }

   if (NULL == (new_list = allocate_list ()))
     return NULL;

   if (-1 == make_chunk_chain (length, &new_list->first, &new_list->last))
     {
	delete_list (new_list);
	return NULL;
     }
   obj = find_nth_element (list, indx_a, &c);
   if (obj == NULL)
     {
	delete_list (new_list);
	return NULL;
     }
   obj_max = c->elements + c->num_elements;

   new_list->length = length;
   new_c = new_list->first;
   new_obj = new_c->elements;
   new_obj_max = new_obj + CHUNK_SIZE;

   for (i = 0; i < length; i++)
     {
	while (obj == obj_max)
	  {
	     c = c->next;
	     obj = c->elements;
	     obj_max = obj + c->num_elements;
	  }
	if (new_obj == new_obj_max)
	  {
	     new_c = new_c->next;
	     new_obj = new_c->elements;
	     new_obj_max = new_obj + CHUNK_SIZE;
	  }

	if ((-1 == _pSLpush_slang_obj (obj))
	    || (-1 == SLang_pop (new_obj)))
	  {
	     delete_list (new_list);
	     return NULL;
	  }

	new_c->num_elements++;
	obj++;
	new_obj++;
     }
   return new_list;
}

static void list_delete_elem (SLang_List_Type *list, SLindex_Type *indxp)
{
   SLang_Object_Type *elem;
   Chunk_Type *c;
   char *src, *dest, *src_max;
   SLindex_Type indx;

   indx = *indxp;
   if (NULL == (elem = find_nth_element (list, indx, &c)))
     return;

   if (indx < 0) indx += list->length; /* checked by find_nth_element */

   SLang_free_object (elem);
   c->num_elements--;
   list->length--;

   if (c->num_elements == 0)
     {
	if (list->first == c)
	  list->first = c->next;
	if (list->last == c)
	  list->last = c->prev;
	if (c->next != NULL)
	  c->next->prev = c->prev;
	if (c->prev != NULL)
	  c->prev->next = c->next;
	delete_chunk (c);
	if (list->recent == c)
	  list->recent = NULL;
	return;
     }

   src = (char *) (elem + 1);
   dest = (char *) elem;
   src_max = src + sizeof(SLang_Object_Type)*((c->elements+c->num_elements)-elem);
   while (src < src_max)
     *dest++ = *src++;

   if ((list->recent != NULL) && (list->recent_num > indx))
     list->recent_num--;
}

static void slide_right (Chunk_Type *c, int n)
{
   SLang_Object_Type *e2, *e1;

   e2 = c->elements + c->num_elements;
   e1 = c->elements + n;

   while (e2 != e1)
     {
	*e2 = *(e2-1);
	e2--;
     }
}

static int insert_element (SLang_List_Type *list, SLang_Object_Type *obj, SLindex_Type indx)
{
   Chunk_Type *c, *c1;
   SLang_Object_Type *elem;
   int num;
   int equality_ok = 0;

   if (indx == 0)
     {
	c = list->first;
	if ((c != NULL)
	    && (c->num_elements < CHUNK_SIZE))
	  {
	     slide_right (c, 0);
	     c->elements[0] = *obj;
	     goto the_return;
	  }
	if (NULL == (c = new_chunk ()))
	  return -1;

	c->next = list->first;
	if (list->first != NULL)
	  list->first->prev = c;
	list->first = c;
	if (list->last == NULL)
	  list->last = c;
	c->elements[0] = *obj;

        equality_ok = 1;
	goto the_return;
     }

   if (indx == list->length)
     {
	c = list->last;
	if (c->num_elements < CHUNK_SIZE)
	  {
	     c->elements[c->num_elements] = *obj;
	     goto the_return;
	  }
	if (NULL == (c = new_chunk ()))
	  return -1;
	c->prev = list->last;
	list->last->next = c;
	list->last = c;
	c->elements[0] = *obj;
	goto the_return;
     }

   if (NULL == (elem = find_nth_element (list, indx, &c)))
     return -1;

   if (c->num_elements < CHUNK_SIZE)
     {
	slide_right (c, elem - c->elements);
	*elem = *obj;
	goto the_return;
     }

   if (NULL == (c1 = new_chunk ()))
     return -1;

   num = CHUNK_SIZE - (elem - c->elements);
   if (num == CHUNK_SIZE)
     {
	c1->next = c;
	c1->prev = c->prev;
	if (c->prev != NULL)
	  c->prev->next = c1;
	c->prev = c1;
	if (list->first == c)
	  list->first = c1;
	c = c1;
	c->elements[0] = *obj;
	equality_ok = 1;
	goto the_return;
     }
   c1->prev = c;
   c1->next = c->next;
   if (c->next != NULL)
     c->next->prev = c1;
   c->next = c1;
   if (list->last == c)
     list->last = c1;

   memcpy ((char *)c1->elements, (char *)elem, num*sizeof(SLang_Object_Type));
   c1->num_elements = num;
   c->num_elements -= num;
   c->elements[c->num_elements] = *obj;
   /* drop */

the_return:

   if (list->recent != NULL)
     {
	if ((list->recent_num > indx)
	    || ((list->recent_num == indx) && equality_ok))
	  list->recent_num++;
     }

   c->num_elements++;
   list->length++;
   return 0;
}

/* Upon sucess, obj is stored in list and calling routine should not free it.
 * Upon failure, calling routine should free obj.
 */

static int pop_insert_append_args (SLang_MMT_Type **mmtp, SLang_List_Type **listp,
				   SLang_Object_Type *obj, int *indx)
{
   if (SLang_Num_Function_Args == 3)
     {
	if (-1 == SLang_pop_integer (indx))
	  return -1;
     }
   if (-1 == SLang_pop (obj))
     return -1;
   if (-1 == pop_list (mmtp, listp))
     {
	SLang_free_object (obj);
	return -1;
     }
   return 0;
}

static void list_insert_elem (void)
{
   int indx;
   SLang_Object_Type obj;
   SLang_MMT_Type *mmt;
   SLang_List_Type *list;

   indx = 0;
   if (-1 == pop_insert_append_args (&mmt, &list, &obj, &indx))
     return;

   if (indx < 0)
     indx += list->length;

   if (-1 == insert_element (list, &obj, indx))
     SLang_free_object (&obj);

   SLang_free_mmt (mmt);
}

static void list_append_elem (void)
{
   int indx;
   SLang_Object_Type obj;
   SLang_MMT_Type *mmt;
   SLang_List_Type *list;

   indx = -1;
   if (-1 == pop_insert_append_args (&mmt, &list, &obj, &indx))
     return;

   if (indx < 0)
     indx += list->length;

   if (-1 == insert_element (list, &obj, indx+1))
     SLang_free_object (&obj);

   SLang_free_mmt (mmt);
}

static int push_list (SLang_List_Type *list)
{
   SLang_MMT_Type *mmt;
   if (NULL == (mmt = SLang_create_mmt (SLANG_LIST_TYPE, (VOID_STAR) list)))
     {
	delete_list (list);
	return -1;
     }

   if (-1 == SLang_push_mmt (mmt))
     {
	SLang_free_mmt (mmt);
	return -1;
     }
   return 0;
}

static void list_new (void)
{
   SLang_List_Type *list;

   if (NULL == (list = allocate_list ()))
     return;

   (void) push_list (list);
}

static void list_reverse (SLang_List_Type *list)
{
   Chunk_Type *c;

   c = list->first;
   list->first = list->last;
   list->last = c;
   while (c != NULL)
     {
	int i, j;
	SLang_Object_Type *objs = c->elements;
	int num = c->num_elements;
	Chunk_Type *c1;

	i = 0;
	j = num-1;
	while (j > i)
	  {
	     SLang_Object_Type tmp = objs[i];
	     objs[i] = objs[j];
	     objs[j] = tmp;
	     j--;
	     i++;
	  }

	c1 = c->next;
	c->next = c->prev;
	c->prev = c1;
	c = c1;
     }

   list->recent = NULL;
}

static int list_pop_nth (SLang_List_Type *list, SLindex_Type indx)
{
   SLang_Object_Type *obj;

   if (NULL == (obj = find_nth_element (list, indx, NULL)))
     return -1;

   if (-1 == _pSLpush_slang_obj (obj))
     return -1;

   list_delete_elem (list, &indx);
   return 0;
}

static void list_pop (void)
{
   int indx = 0;
   SLang_MMT_Type *mmt;
   SLang_List_Type *list;

   if (SLang_Num_Function_Args == 2)
     {
	if (-1 == SLang_pop_integer (&indx))
	  return;
     }
   if (-1 == pop_list (&mmt, &list))
     return;

   (void) list_pop_nth (list, indx);
   SLang_free_mmt (mmt);
}

static int pop_as_list_internal (unsigned int count)
{
   SLang_List_Type *list;

   if (NULL == (list = allocate_list ()))
     return -1;

   while (count)
     {
	SLang_Object_Type obj;

	if (-1 == SLang_pop (&obj))
	  goto return_error;

	if (-1 == insert_element (list, &obj, 0))
	  {
	     SLang_free_object (&obj);
	     goto return_error;
	  }

	count--;
     }
   return push_list (list);

   return_error:
   delete_list (list);
   return -1;
}

static void pop_as_list (int *nitems)
{
   if (*nitems < 0)
     {
	_pSLang_verror (SL_INVALID_PARM, "Expecting a non-negative integer");
	return;
     }
   (void) pop_as_list_internal (*nitems);
}

int _pSLlist_inline_list (void)
{
   return pop_as_list_internal (SLang_Num_Function_Args);
}

static void push_list_elements (SLang_List_Type *list)
{
   SLindex_Type n = list->length;
   SLindex_Type i;

   for (i = 0; i < n; i++)
     {
	SLang_Object_Type *objp = find_nth_element (list, i, NULL);
	if ((objp == NULL) || (-1 == _pSLpush_slang_obj (objp)))
	  return;
     }
}

static int l2a_get_type_callback (VOID_STAR vlist, SLuindex_Type i, SLtype *type)
{
   SLang_Object_Type *obj;

   if (NULL == (obj = find_nth_element ((SLang_List_Type *)vlist, i, NULL)))
     return -1;
   *type = obj->o_data_type;
   return 0;
}

static int l2a_push_callback (VOID_STAR vlist, SLuindex_Type i)
{
   SLang_Object_Type *obj;

   if (NULL == (obj = find_nth_element ((SLang_List_Type *)vlist, i, NULL)))
     return -1;

   return _pSLpush_slang_obj (obj);
}

static void list_to_array (void)
{
   SLang_List_Type *list;
   SLang_MMT_Type *mmt;
   SLtype type = 0;

   if ((SLang_Num_Function_Args == 2)
       && (-1 == SLang_pop_datatype (&type)))
     return;

   if (-1 == pop_list (&mmt, &list))
     return;

   (void) _pSLarray_convert_to_array ((VOID_STAR) list,
				      l2a_get_type_callback,
				      l2a_push_callback,
				      list->length, type);

   SLang_free_mmt (mmt);
}

/* joins  l2 onto l1 */
static int list_join_internal (SLang_List_Type *l1, SLang_List_Type *l2)
{
   Chunk_Type *chunk;
   SLindex_Type num2;

   chunk = l2->first;
   num2 = l2->length;
   while (num2 > 0)
     {
	SLindex_Type i, imax;
	SLang_Object_Type *objs;
	objs = chunk->elements;
	i = 0; imax = chunk->num_elements;

	while ((num2 > 0) && (i < imax))
	  {
	     SLang_Object_Type obj;
	     if (-1 == _pSLslang_copy_obj (objs+i, &obj))
	       return -1;

	     if (-1 == insert_element(l1, &obj,  l1->length))
	       {
		  SLang_free_object (&obj);
		  return -1;
	       }
	     i++;
	     num2--;
	  }
	chunk = chunk->next;
     }
   return 0;
}

static void list_join (SLang_List_Type *l1, SLang_List_Type *l2)
{
   (void) list_join_internal (l1, l2);
}

static void list_concat (SLang_List_Type *l1, SLang_List_Type *l2)
{
   SLang_List_Type *list;

   if (NULL == (list = make_sublist (l1, 0, l1->length)))
     return;

   if (-1 == list_join_internal (list, l2))
     {
	delete_list (list);
	return;
     }

  (void) push_list (list);	       /* will delete upon failure */
}

#define L SLANG_LIST_TYPE
#define I SLANG_INT_TYPE
#define A SLANG_ARRAY_INDEX_TYPE
static SLang_Intrin_Fun_Type Intrin_Table [] =
{
   MAKE_INTRINSIC_2("list_delete", list_delete_elem, SLANG_VOID_TYPE, L, A),
   MAKE_INTRINSIC_1("list_reverse", list_reverse, SLANG_VOID_TYPE, L),
   MAKE_INTRINSIC_0("list_insert", list_insert_elem, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("list_append", list_append_elem, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("list_to_array", list_to_array, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("list_new", list_new, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("list_pop", list_pop, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_2("list_concat", list_concat, SLANG_VOID_TYPE, L, L),
   MAKE_INTRINSIC_2("list_join", list_join, SLANG_VOID_TYPE, L, L),
   MAKE_INTRINSIC_1("__push_list", push_list_elements, SLANG_VOID_TYPE, L),
   MAKE_INTRINSIC_1("__pop_list", pop_as_list, SLANG_VOID_TYPE, I),
   SLANG_END_INTRIN_FUN_TABLE
};
#undef L
#undef I
#undef A

static int list_length (SLtype type, VOID_STAR v, SLuindex_Type *len)
{
   SLang_List_Type *list;

   (void) type;
   list = (SLang_List_Type *) SLang_object_from_mmt (*(SLang_MMT_Type **)v);
   *len = list->length;
   return 0;
}

static int list_dereference (SLtype type, VOID_STAR addr)
{
   SLang_List_Type *list;
   (void) type;

   list = (SLang_List_Type *) SLang_object_from_mmt (*(SLang_MMT_Type **)addr);

   if (NULL == (list = make_sublist (list, 0, list->length)))
     return -1;

   return push_list (list);	       /* will delete upon failure */
}

static char *string_method (SLtype type, VOID_STAR p)
{
   SLang_List_Type *list;
   char buf[256];

   (void) type;
   list = (SLang_List_Type *) SLang_object_from_mmt (*(SLang_MMT_Type **)p);

   sprintf (buf, "List_Type with %ld elements", (long)list->length);
   return SLmake_string (buf);
}

static int eqs_method (SLtype a_type, VOID_STAR pa, SLtype b_type, VOID_STAR pb)
{
   SLang_List_Type *a, *b;
   Chunk_Type *ca, *cb;
   SLang_Object_Type *objb, *objb_max;

   if ((a_type != SLANG_LIST_TYPE) || (b_type != SLANG_LIST_TYPE))
     return 0;

   a = (SLang_List_Type *) SLang_object_from_mmt (*(SLang_MMT_Type **)pa);
   b = (SLang_List_Type *) SLang_object_from_mmt (*(SLang_MMT_Type **)pb);

   if (a == b)
     return 1;

   if (a->length != b->length)
     return 0;

   if (a->length == 0)
     return 1;

   ca = a->first;
   cb = b->first;

   objb = cb->elements;
   objb_max = objb + cb->num_elements;

   while (ca != NULL)
     {
	SLang_Object_Type *obja = ca->elements;
	SLang_Object_Type *obja_max = obja + ca->num_elements;

	while (obja < obja_max)
	  {
	     int ret;

	     while (objb == objb_max)
	       {
		  cb = cb->next;
		  objb = cb->elements;
		  objb_max = objb + cb->num_elements;
	       }
	     /* For now, use _pSLclass_obj_eqs.  Doing so makes the semantics
	      * different from the array_eqs function.
	      */

	     ret = _pSLclass_obj_eqs (obja, objb);
	     if (ret != 1)
	       return ret;

	     obja++; objb++;
	  }
	ca = ca->next;
     }
   return 1;
}

struct _pSLang_Foreach_Context_Type
{
   SLang_List_Type *list;
   SLang_MMT_Type *mmt;
   int next_index;
};

static SLang_Foreach_Context_Type *
cl_foreach_open (SLtype type, unsigned int num)
{
   SLang_Foreach_Context_Type *c;

   (void) type;

   if (num != 0)
     {
	_pSLang_verror (SL_NOT_IMPLEMENTED,
		      "%s does not support 'foreach using' form",
		      SLclass_get_datatype_name (type));
	return NULL;
     }

   if (NULL == (c = (SLang_Foreach_Context_Type *) SLcalloc (1, sizeof (SLang_Foreach_Context_Type))))
     return NULL;

   if (-1 == pop_list (&c->mmt, &c->list))
     {
	SLfree ((char *) c);
	return NULL;
     }

   return c;
}

static void cl_foreach_close (SLtype type, SLang_Foreach_Context_Type *c)
{
   (void) type;
   if (c == NULL) return;
   SLang_free_mmt (c->mmt);
   SLfree ((char *) c);
}

static int cl_foreach (SLtype type, SLang_Foreach_Context_Type *c)
{
   SLang_Object_Type *obj;

   (void) type;
   if (c == NULL)
     return -1;

   if (c->list->length <= c->next_index)
     return 0;

   if ((NULL == (obj = find_nth_element (c->list, c->next_index, NULL)))
       || (-1 == _pSLpush_slang_obj (obj)))
     return -1;

   c->next_index++;
   return 1;
}

static int pop_list_and_index (unsigned int num_indices,
			       SLang_MMT_Type **mmtp, SLang_List_Type **listp,
			       SLang_Array_Type **ind_atp,
			       SLindex_Type *indx)
{
   SLang_MMT_Type *mmt;
   SLang_List_Type *list;

   if (-1 == pop_list (&mmt, &list))
     return -1;

   if (num_indices != 1)
     {
	_pSLang_verror (SL_InvalidParm_Error, "List_Type objects are limited to a single index");
	SLang_free_mmt (mmt);
	return -1;
     }

   *ind_atp = NULL;
   if (SLang_peek_at_stack () == SLANG_ARRAY_INDEX_TYPE)
     {
	if (-1 == SLang_pop_array_index (indx))
	  {
	     SLang_free_mmt (mmt);
	     return -1;
	  }
     }
   else
     {
	if (-1 == _pSLarray_pop_index (list->length, ind_atp, indx))
	  {
	     SLang_free_mmt (mmt);
	     return -1;
	  }
     }

   *listp = list;
   *mmtp = mmt;
   return 0;
}

/* FIXME: Extend this to allow an index array */
static int _pSLlist_aget (SLtype type, unsigned int num_indices)
{
   SLang_MMT_Type *mmt;
   SLang_List_Type *list, *new_list;
   SLang_Object_Type *obj;
   SLang_Array_Type *ind_at;
   SLindex_Type indx, *idx_data;
   SLuindex_Type i, num, list_len;
   int ret;

   (void) type;

   if (-1 == pop_list_and_index (num_indices, &mmt, &list, &ind_at, &indx))
     return -1;

   ret = -1;
   if (ind_at == NULL)
     {
	obj = find_nth_element (list, indx, NULL);
	if (obj != NULL)
	  ret = _pSLpush_slang_obj (obj);

	SLang_free_mmt (mmt);
	return ret;
     }

   if (NULL == (new_list = allocate_list ()))
     goto free_and_return;

   list_len = list->length;
   num = ind_at->num_elements;
   idx_data = (SLindex_Type *)ind_at->data;
   for (i = 0; i < num; i++)
     {
	SLang_Object_Type *obja;
	SLang_Object_Type objb;

	indx = idx_data[i];
	if (NULL == (obja = find_nth_element (list, idx_data[i], NULL)))
	  goto free_and_return;

	if (-1 == _pSLslang_copy_obj (obja, &objb))
	  goto free_and_return;

	if (-1 == insert_element (new_list, &objb, i))
	  {
	     SLang_free_object (&objb);
	     goto free_and_return;
	  }
     }

   ret = push_list (new_list);	       /* frees upon error */
   new_list = NULL;

free_and_return:

   if (new_list != NULL)
     delete_list (new_list);
   SLang_free_mmt (mmt);
   SLang_free_array (ind_at);
   return ret;
}

static int aput_object (SLang_List_Type *list, SLindex_Type indx, SLang_Object_Type *obj)
{
   SLang_Object_Type *elem;

   if (NULL == (elem = find_nth_element (list, indx, NULL)))
     return -1;

   SLang_free_object (elem);
   *elem = *obj;
   return 0;
}

static int _pSLlist_aput (SLtype type, unsigned int num_indices)
{
   SLang_MMT_Type *mmt;
   SLang_List_Type *list;
   SLang_Object_Type obj;
   SLang_Array_Type *ind_at;
   SLindex_Type indx, *idx_data;
   SLuindex_Type i, num, list_len;
   int ret;

   (void) type;

   if (-1 == pop_list_and_index (num_indices, &mmt, &list, &ind_at, &indx))
     return -1;

   if (ind_at == NULL)
     {
	if (-1 == SLang_pop (&obj))
	  {
	     SLang_free_mmt (mmt);
	     return -1;
	  }

	if (-1 == aput_object (list, indx, &obj))
	  {
	     SLang_free_object (&obj);
	     SLang_free_mmt (mmt);
	     return -1;
	  }
	SLang_free_mmt (mmt);
	return 0;
     }

   idx_data = (SLindex_Type *)ind_at->data;
   num = ind_at->num_elements;
   list_len = list->length;

   if (-1 == SLang_pop (&obj))
     {
	SLang_free_mmt (mmt);
	SLang_free_array (ind_at);
	return -1;
     }

   ret = -1;

   if (obj.o_data_type == SLANG_ARRAY_TYPE)
     {
	SLang_Array_Type *at = obj.v.array_val;
	if ((at->num_elements != num) || (at->num_dims != 1))
	  {
	     SLang_verror (SL_Index_Error, "Inappropriate array for list[indices]=array expression");
	     goto free_and_return;
	  }

	for (i = 0; i < num; i++)
	  {
	     SLang_Object_Type objb;
	     indx = idx_data[i];

	     if ((-1 == _pSLarray1d_push_elem (at, i))
		 || (-1 == SLang_pop (&objb)))
	       goto free_and_return;

	     if (-1 == aput_object (list, indx, &objb))
	       {
		  SLang_free_object (&objb);
		  goto free_and_return;
	       }
	  }
	ret = 0;
	goto free_and_return;
     }

   if (obj.o_data_type == SLANG_LIST_TYPE)
     {
	SLang_MMT_Type *mmt2;
	SLang_List_Type *list2;

	mmt2 = obj.v.ref;
	if (NULL == (list2 = (SLang_List_Type *)SLang_object_from_mmt (mmt2)))
	  goto free_and_return;

	if (list2->length != (SLindex_Type)num)
	  {
	     SLang_verror (SL_Index_Error, "Inappropriate list2 size for list[indices]=list2 expression");
	     goto free_and_return;
	  }

	for (i = 0; i < num; i++)
	  {
	     SLang_Object_Type *obja;
	     SLang_Object_Type objb;
	     indx = idx_data[i];

	     if (NULL == (obja = find_nth_element (list, idx_data[i], NULL)))
	       goto free_and_return;

	     if (-1 == _pSLslang_copy_obj (obja, &objb))
	       goto free_and_return;

	     if (-1 == aput_object (list, indx, &objb))
	       {
		  SLang_free_object (&objb);
		  goto free_and_return;
	       }
	  }
	ret = 0;
	goto free_and_return;
     }

   for (i = 0; i < num; i++)
     {
	SLang_Object_Type objb;
	indx = idx_data[i];

	if (-1 == _pSLslang_copy_obj (&obj, &objb))
	  goto free_and_return;

	if (-1 == aput_object (list, indx, &objb))
	  {
	     SLang_free_object (&objb);
	     goto free_and_return;
	  }
     }

   ret = 0;
   /* drop */
free_and_return:

   SLang_free_object (&obj);
   SLang_free_array (ind_at);
   SLang_free_mmt (mmt);
   return ret;
}

int _pSLang_init_sllist (void)
{
   SLang_Class_Type *cl;

   if (SLclass_is_class_defined (SLANG_LIST_TYPE))
     return 0;

   if (NULL == (cl = SLclass_allocate_class ("List_Type")))
     return -1;

   (void) SLclass_set_destroy_function (cl, list_destroy);
   (void) SLclass_set_aput_function (cl, _pSLlist_aput);
   (void) SLclass_set_aget_function (cl, _pSLlist_aget);
   (void) SLclass_set_deref_function (cl, list_dereference);
   (void) SLclass_set_string_function (cl, string_method);
   (void) SLclass_set_eqs_function (cl, eqs_method);
   (void) SLclass_set_is_container (cl, 1);

   cl->cl_length = list_length;

   cl->cl_foreach_open = cl_foreach_open;
   cl->cl_foreach_close = cl_foreach_close;
   cl->cl_foreach = cl_foreach;

   if (-1 == SLclass_register_class (cl, SLANG_LIST_TYPE, sizeof (SLang_List_Type), SLANG_CLASS_TYPE_MMT))
     return -1;

   if (-1 == SLadd_intrin_fun_table (Intrin_Table, NULL))
     return -1;

   return 0;
}

