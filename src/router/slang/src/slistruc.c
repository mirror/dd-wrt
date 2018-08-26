/* Intrinsic Structure type implementation */
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

/* Intrinsic structures */

typedef struct
{
   char *name;
   VOID_STAR addr;
   SLang_IStruct_Field_Type *fields;
}
_pSLang_IStruct_Type;

static SLang_IStruct_Field_Type *istruct_pop_field (SLCONST char *name, int no_readonly, VOID_STAR *addr)
{
   _pSLang_IStruct_Type *s;
   SLang_IStruct_Field_Type *f;
   char *struct_addr;

   /* Note: There is no need to free this object */
   if (-1 == SLclass_pop_ptr_obj (SLANG_ISTRUCT_TYPE, VOID_STAR_STAR(&s)))
     return NULL;

   if (NULL == (struct_addr = *(char **)s->addr))
     {
	_pSLang_verror (SL_INTRINSIC_ERROR,
		      "%s is NULL.  Unable to access field", s->name);
	return NULL;
     }

   f = s->fields;
   while (f->field_name != NULL)
     {
	/* Since both these are slstrings, just test pointers */
	if (f->field_name != name)
	  {
	     f++;
	     continue;
	  }

	if (no_readonly && f->read_only)
	  {
	     _pSLang_verror (SL_READONLY_ERROR,
			   "%s.%s is read-only", s->name, name);
	     return NULL;
	  }

	*addr = (VOID_STAR) (struct_addr + f->offset);
	return f;
     }

   _pSLang_verror (SL_TYPE_MISMATCH,
		 "%s has no field called %s", s->name, name);
   return NULL;
}

static int istruct_sget (SLtype type, SLFUTURE_CONST char *name)
{
   SLang_IStruct_Field_Type *f;
   VOID_STAR addr;
   SLang_Class_Type *cl;

   if (NULL == (f = istruct_pop_field (name, 0, &addr)))
     return -1;

   type = f->type;
   cl = _pSLclass_get_class (type);

   return (cl->cl_push_intrinsic)(f->type, addr);
}

static int istruct_sput (SLtype type, SLFUTURE_CONST char *name)
{
   SLang_IStruct_Field_Type *f;
   VOID_STAR addr;
   SLang_Class_Type *cl;

   if (NULL == (f = istruct_pop_field (name, 1, &addr)))
     return -1;

   type = f->type;
   cl = _pSLclass_get_class (type);

   return (*cl->cl_pop) (type, addr);
}

static int istruct_push (SLtype type, VOID_STAR ptr)
{
   _pSLang_IStruct_Type *s;

   s = *(_pSLang_IStruct_Type **) ptr;
   if ((s == NULL)
       || (s->addr == NULL)
       || (*(char **) s->addr == NULL))
     return SLang_push_null ();

   return SLclass_push_ptr_obj (type, (VOID_STAR) s);
}

static int istruct_pop (SLtype type, VOID_STAR ptr)
{
   return SLclass_pop_ptr_obj (type, (VOID_STAR *)ptr);
}

static void istruct_destroy (SLtype type, VOID_STAR ptr)
{
   (void) type;
   (void) ptr;
}

/* Intrinsic struct objects are not stored in a variable. So, the address that
 * is passed here is actually a pointer to the struct.  So, pass its address
 * to istruct_push since v is a variable.  Confusing, n'est pas?
 */
static int istruct_push_intrinsic (SLtype type, VOID_STAR v)
{
   return istruct_push (type, (VOID_STAR) &v);
}

static int init_intrin_struct (void)
{
   SLang_Class_Type *cl;
   static int initialized;

   if (initialized)
     return 0;

   if (NULL == (cl = SLclass_allocate_class ("IStruct_Type")))
     return -1;

   cl->cl_pop = istruct_pop;
   cl->cl_push = istruct_push;
   cl->cl_sget = istruct_sget;
   cl->cl_sput = istruct_sput;
   cl->cl_destroy = istruct_destroy;
   cl->cl_push_intrinsic = istruct_push_intrinsic;

   if (-1 == SLclass_register_class (cl, SLANG_ISTRUCT_TYPE, sizeof (_pSLang_IStruct_Type *),
				     SLANG_CLASS_TYPE_PTR))
     return -1;

   initialized = 1;
   return 0;
}

int SLns_add_istruct_table (SLang_NameSpace_Type *ns, SLang_IStruct_Field_Type *fields, VOID_STAR addr, SLFUTURE_CONST char *name)
{
   _pSLang_IStruct_Type *s;
   SLang_IStruct_Field_Type *f;

   if (-1 == init_intrin_struct ())
     return -1;

   if (addr == NULL)
     {
	_pSLang_verror (SL_INVALID_PARM,
		      "SLadd_istruct_table: address must be non-NULL");
	return -1;
     }

   if (fields == NULL)
     return -1;

   /* Make the field names slstrings so that only the pointers need to be
    * compared.  However, this table may have been already been added for
    * another instance of the intrinsic object.  So, check for the presence
    * of an slstring.
    */
   f = fields;
   while (f->field_name != NULL)
     {
	char *fname;

	fname = SLang_create_slstring (f->field_name);
	if (fname == NULL)
	  return -1;

	/* Here is the check for the slstring */
	if (f->field_name == fname)
	  SLang_free_slstring (fname);
	else /* replace string literal with slstring */
	  f->field_name = fname;

	f++;
     }

   s = (_pSLang_IStruct_Type *)SLmalloc (sizeof (_pSLang_IStruct_Type));
   if (s == NULL)
     return -1;

   memset ((char *)s, 0, sizeof (_pSLang_IStruct_Type));
   if (NULL == (s->name = SLang_create_slstring (name)))
     {
	SLfree ((char *) s);
	return -1;
     }

   s->addr = addr;
   s->fields = fields;

   if (-1 == SLns_add_intrinsic_variable (ns, name, (VOID_STAR) s, SLANG_ISTRUCT_TYPE, 1))
     {
	SLang_free_slstring (s->name);
	SLfree ((char *) s);
	return -1;
     }

   return 0;
}

int SLadd_istruct_table (SLang_IStruct_Field_Type *fields, VOID_STAR addr, SLFUTURE_CONST char *name)
{
   return SLns_add_istruct_table (NULL, fields, addr, name);
}

