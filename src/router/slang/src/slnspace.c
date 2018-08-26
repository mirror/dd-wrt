/* -*- mode: C; mode: fold; -*- */
/* slnspace.c  --- Name Space implementation */
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

#define MAP_HASH_TO_NS_INDEX(hash,ns) (unsigned int) ((hash) % (ns->table_size))

static SLang_NameSpace_Type *Namespace_Tables;

SLang_NameSpace_Type *_pSLns_find_namespace (SLCONST char *name)
{
   SLang_NameSpace_Type *table_list;

   table_list = Namespace_Tables;
   while (table_list != NULL)
     {
	if ((table_list->namespace_name != NULL)
	    && (0 == strcmp (table_list->namespace_name, name)))
	  break;
	table_list = table_list->next;
     }
   return table_list;
}

/* This function deletes the namespace.  It is up to the caller to ensure that
 * the table has already been removed from the list.
 */
void _pSLns_deallocate_namespace (SLang_NameSpace_Type *ns)
{
   SLang_Name_Type **table;
   unsigned int table_size;
   unsigned int i;

   if (ns == NULL)
     return;

   table = ns->table;
   table_size = ns->table_size;

   for (i = 0; i < table_size; i++)
     {
	SLang_Name_Type *t = table [i];
   	while (t != NULL)
	  {
	     SLang_Name_Type *t1 = t->next;
	     SLang_free_slstring ((char *) t->name);
	     SLfree ((char *) t);
	     t = t1;
	  }
     }
   SLang_free_slstring ((char *) ns->name);
   SLang_free_slstring ((char *) ns->namespace_name);
   SLang_free_slstring ((char *) ns->private_name);
   SLfree ((char *)table);
   SLfree ((char *) ns);
}

/* This function does not insert the namespace into the list */
SLang_NameSpace_Type *_pSLns_allocate_namespace (SLFUTURE_CONST char *name, unsigned int size)
{
   SLang_Name_Type **nt;
   SLang_NameSpace_Type *ns;

   if (NULL == (name = SLang_create_slstring (name)))
     return NULL;

   if (NULL == (ns = (SLang_NameSpace_Type *)
		SLcalloc (sizeof (SLang_NameSpace_Type), 1)))
     {
	SLang_free_slstring ((char *) name);
	return NULL;
     }

   if (NULL == (nt = (SLang_Name_Type **) SLcalloc (sizeof (SLang_Name_Type *), size)))
     {
	SLang_free_slstring ((char *) name);
	SLfree ((char *)ns);
	return NULL;
     }

   ns->name = name;
   ns->table = nt;
   ns->table_size = size;

   return ns;
}

/* allocate a namespace and add it to the internal list */
SLang_NameSpace_Type *_pSLns_new_namespace (SLFUTURE_CONST char *name, unsigned int size)
{
   SLang_NameSpace_Type *table_list;
   static int num;
   char namebuf[64];

   if (name == NULL)
     {
	sprintf (namebuf, " *** internal ns <%d> *** ", num);
	name = namebuf;
	num++;
     }

   if (NULL == (table_list = _pSLns_allocate_namespace (name, size)))
     return NULL;

   table_list->next = Namespace_Tables;
   Namespace_Tables = table_list;

   return table_list;
}

/* Find the private namespace associated with the object (file, etc) given by
 * `name', and whose private namespace is given by `namespace_name'.  If
 * `namespace_name' is NULL, then it is anonymous.
 */
SLang_NameSpace_Type *_pSLns_get_private_namespace (SLFUTURE_CONST char *name, SLFUTURE_CONST char *namespace_name)
{
   SLang_NameSpace_Type *ns;

   if ((namespace_name != NULL)
       && (*namespace_name == 0))
     namespace_name = NULL;

   ns = Namespace_Tables;
   while (ns != NULL)
     {
	if ((ns->namespace_name != NULL)   /* a static namespace */
	    || (0 != strcmp (ns->name, name)))
	  {
	     ns = ns->next;
	     continue;
	  }

	/* at this point, the namespace is anonymous and is associated with
	 * the correct file (given by name).
	 */
	if (ns->private_name == NULL)
	  {
	     if (namespace_name == NULL)
	       return ns;

	     ns = ns->next;
	     continue;
	  }

	if ((namespace_name != NULL)
	    && (0 == strcmp (ns->private_name, namespace_name)))
	  return ns;

	ns = ns->next;
     }

   if (NULL == (ns = _pSLns_new_namespace (name, SLSTATIC_HASH_TABLE_SIZE)))
     return ns;

   if (namespace_name == NULL)
     return ns;

   if (NULL == (namespace_name = SLang_create_slstring (namespace_name)))
     {
	SLns_delete_namespace (ns);
	return NULL;
     }
   ns->private_name = namespace_name;
   return ns;
}

int _pSLns_set_namespace_name (SLang_NameSpace_Type *t, SLFUTURE_CONST char *name)
{
   SLang_NameSpace_Type *t1;

   t1 = _pSLns_find_namespace (name);
   if (t == t1)
     return 0;			       /* already has this name */

   if (t1 == NULL)
     t1 = t;

   if ((t != t1) || (*name == 0))
     {
	_pSLang_verror (SL_Namespace_Error, "Namespace \"%s\" already exists",
		      name);
	return -1;
     }

   if (t->namespace_name != NULL)
     {
	_pSLang_verror (SL_Namespace_Error, "An attempt was made to redefine namespace from \"%s\" to \"%s\"\n",
		      t->namespace_name, name);
	return -1;
     }

   if (NULL == (name = SLang_create_slstring (name)))
     return -1;

   SLang_free_slstring ((char *) t->namespace_name);   /* NULL ok */
   t->namespace_name = name;

   return 0;
}

SLang_Array_Type *_pSLnspace_apropos (SLang_NameSpace_Type *ns, SLFUTURE_CONST char *pat, unsigned int what)
{
   SLang_Array_Type *at;
   unsigned int table_size;
   SLang_Name_Type *t, **table;
   SLindex_Type num_matches;
   unsigned int i;
   SLRegexp_Type *reg;
   unsigned int two;

   at = NULL;

   if ((ns == NULL)
       || ((table = ns->table) == NULL))
     return NULL;

   if (NULL == (reg = SLregexp_compile (pat, 0)))
     {
	_pSLang_verror (SL_Parse_Error, "Invalid regular expression: %s", pat);
	return NULL;
     }

   table_size = ns->table_size;

   two = 2;
   while (two != 0)
     {
	two--;

	num_matches = 0;
	for (i = 0; i < table_size; i++)
	  {
	     t = table[i];
	     while (t != NULL)
	       {
		  unsigned int flags;
		  SLFUTURE_CONST char *name = t->name;

		  switch (t->name_type)
		    {
		     case SLANG_GVARIABLE:
		       flags = 8;
		       break;

		     case SLANG_ICONSTANT:
		     case SLANG_DCONSTANT:
		     case SLANG_FCONSTANT:
		     case SLANG_LLCONSTANT:
		     case SLANG_HCONSTANT:
		     case SLANG_LCONSTANT:
		     case SLANG_RVARIABLE:
		     case SLANG_IVARIABLE:
		       flags = 4;
		       break;

		     case SLANG_INTRINSIC:
		     case SLANG_MATH_UNARY:
		     case SLANG_APP_UNARY:
		     case SLANG_ARITH_UNARY:
		     case SLANG_ARITH_BINARY:
		       flags = 1;
		       break;

		     case SLANG_FUNCTION:
		       flags = 2;
		       break;

		     default:
		       flags = 0;
		       break;
		    }

		  if ((flags & what)
		      && (NULL != SLregexp_match (reg, name, strlen (name))))
		    {
		       if (at != NULL)
			 {
			    if (-1 == SLang_set_array_element (at, &num_matches, (VOID_STAR)&name))
			      goto return_error;
			 }
		       num_matches++;
		    }
		  t = t->next;
	       }
	  }

	if (at == NULL)
	  {
	     at = SLang_create_array (SLANG_STRING_TYPE, 0, NULL, &num_matches, 1);
	     if (at == NULL)
	       goto return_error;
	  }
     }

   if (reg != NULL)
     SLregexp_free (reg);
   return at;

   return_error:
   SLregexp_free (reg);
   SLang_free_array (at);
   return NULL;
}

SLang_NameSpace_Type *_pSLns_create_namespace2 (SLFUTURE_CONST char *name, SLFUTURE_CONST char *namespace_name)
{
   SLang_NameSpace_Type *ns;

   if (namespace_name == NULL)
     namespace_name = "Global";

   ns = _pSLns_find_namespace (namespace_name);
   if (ns != NULL)
     return ns;

   if (NULL == (ns = _pSLns_new_namespace (name, SLSTATIC_HASH_TABLE_SIZE)))
     return NULL;

   if (-1 == _pSLns_set_namespace_name (ns, namespace_name))
     {
	SLns_delete_namespace (ns);
	return NULL;
     }

   return ns;
}

SLang_NameSpace_Type *SLns_create_namespace (SLFUTURE_CONST char *namespace_name)
{
   return _pSLns_create_namespace2 (NULL, namespace_name);
}

void SLns_delete_namespace (SLang_NameSpace_Type *ns)
{
   if (ns == NULL)
     return;

   if (ns == Namespace_Tables)
     Namespace_Tables = ns->next;
   else
     {
	SLang_NameSpace_Type *prev = Namespace_Tables;
	while (prev != NULL)
	  {
	     if (prev->next != ns)
	       {
		  prev = prev->next;
		  continue;
	       }
	     prev->next = ns->next;
	     break;
	  }
     }
   _pSLns_deallocate_namespace (ns);
}

SLang_Array_Type *_pSLns_list_namespaces (void)
{
   SLang_NameSpace_Type *table_list;
   SLang_Array_Type *at;
   SLindex_Type num, i;

   num = 0;
   table_list = Namespace_Tables;
   while (table_list != NULL)
     {
	if (table_list->namespace_name != NULL)
	  num++;
	table_list = table_list->next;
     }
   at = SLang_create_array (SLANG_STRING_TYPE, 0, NULL, &num, 1);
   if (at == NULL)
     return NULL;

   table_list = Namespace_Tables;
   i = 0;
   while ((table_list != NULL)
	  && (i < num))
     {
	if (table_list->namespace_name != NULL)
	  {
	     SLCONST char *name = table_list->namespace_name;
	     if (-1 == SLang_set_array_element (at, &i, (VOID_STAR)&name))
	       {
		  SLang_free_array (at);
		  return NULL;
	       }
	     i++;
	  }
	table_list = table_list->next;
     }
   return at;
}

SLang_Name_Type *
  _pSLns_locate_hashed_name (SLang_NameSpace_Type *ns, SLCONST char *name, unsigned long hash)
{
   SLang_Name_Type *t;
   char ch;

   t = ns->table [MAP_HASH_TO_NS_INDEX(hash,ns)];
   ch = *name++;

   while (t != NULL)
     {
	if ((ch == t->name[0])
	    && (0 == strcmp (t->name + 1, name)))
	  break;

	t = t->next;
     }

   return t;
}

/* It is up to the caller to make sure that the name is not already in the table */
int _pSLns_add_hashed_name (SLang_NameSpace_Type *ns, SLang_Name_Type *nt, unsigned long hash)
{
   hash = MAP_HASH_TO_NS_INDEX(hash,ns);
   nt->next = ns->table [(unsigned int)hash];
   ns->table [(unsigned int) hash] = nt;

   return 0;
}

SLang_NameSpace_Type *_pSLns_find_object_namespace (SLang_Name_Type *nt)
{
   SLang_NameSpace_Type *ns;
   SLCONST char *name;
   unsigned long hash;

   if (nt == NULL)
     return NULL;

   name = nt->name;
   hash = _pSLcompute_string_hash (name);

   ns = Namespace_Tables;
   while (ns != NULL)
     {
	SLang_Name_Type *t = ns->table [MAP_HASH_TO_NS_INDEX(hash,ns)];
	while (t != NULL)
	  {
	     if (t == nt)
	       return ns;

	     t = t->next;
	  }
	ns = ns->next;
     }

   return NULL;
}

SLang_Name_Type *_pSLns_locate_name (SLang_NameSpace_Type *ns, SLCONST char *name)
{
   return _pSLns_locate_hashed_name (ns, name, _pSLcompute_string_hash (name));
}

static void delete_namespace_objects (SLang_NameSpace_Type *ns)
{
   SLang_Name_Type **table = ns->table;
   unsigned int i, table_size = ns->table_size;

   for (i = 0; i < table_size; i++)
     {
	SLang_Name_Type *t = table[i];
	while (t != NULL)
	  {
	     SLang_Name_Type *t1 = t->next;
	     switch (t->name_type)
	       {
		case SLANG_PVARIABLE:
		case SLANG_GVARIABLE:
		  SLang_free_object (&((SLang_Global_Var_Type *)t)->obj);
		  break;

		case SLANG_PFUNCTION:
		case SLANG_FUNCTION:
		  SLang_free_function (t);
		  break;

		case SLANG_ICONSTANT:
		case SLANG_DCONSTANT:
		case SLANG_FCONSTANT:
		case SLANG_LLCONSTANT:
		case SLANG_HCONSTANT:
		case SLANG_LCONSTANT:
		case SLANG_RVARIABLE:
		case SLANG_IVARIABLE:
		case SLANG_INTRINSIC:
		case SLANG_MATH_UNARY:
		case SLANG_APP_UNARY:
		case SLANG_ARITH_UNARY:
		case SLANG_ARITH_BINARY:
		default:
		  break;
	       }
	     SLang_free_slstring (t->name);
	     t = t1;
	  }
     }
}

/* This is only called at exit.  Until version 3, I cannot delete everything
 * since the namespace contains a mixture of statically allocated table
 * and dynamically allocated ones.  The only way to tell the difference is
 * to change the API by adding an additional field to SLang_Name_Type object.
 * So, here I delete what I can safely do, but to avoid leak checkers complaining
 * about false leaks, the Namespace_Tables pointer will be left as is.
 */
void _pSLns_delete_namespaces (void)
{
   SLang_NameSpace_Type *ns;

   ns = Namespace_Tables;
   while (ns != NULL)
     {
	SLang_NameSpace_Type *next = ns->next;
	delete_namespace_objects (ns);
	SLang_free_slstring ((char *) ns->name);
	SLang_free_slstring ((char *) ns->namespace_name);
	SLang_free_slstring ((char *) ns->private_name);
	/* SLfree ((char *)ns->table); v3*/
	/* SLfree ((char *) ns); v3 */
	ns = next;
     }
   /* Namespace_Tables = NULL; v3 */
}

