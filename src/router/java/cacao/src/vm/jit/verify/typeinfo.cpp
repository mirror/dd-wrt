/* src/vm/jit/verify/typeinfo.cpp - type system used by the type checker

   Copyright (C) 1996-2014
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/


#include "config.h"

#include <assert.h>
#include <string.h>

#include "mm/dumpmemory.hpp"

#include "toolbox/logging.hpp"

#include "vm/array.hpp"
#include "vm/class.hpp"
#include "vm/descriptor.hpp"
#include "vm/exceptions.hpp"
#include "vm/globals.hpp"
#include "vm/primitive.hpp"
#include "vm/resolve.hpp"

#include "vm/jit/jit.hpp"
#include "vm/jit/verify/typeinfo.hpp"

#include "vm/jit/ir/instruction.hpp"


/* check if a linked class is an array class. Only use for linked classes! */
#define CLASSINFO_IS_ARRAY(clsinfo)  ((clsinfo)->vftbl->arraydesc != NULL)

/* check if a linked class implements the interface with the given index */
#define CLASSINFO_IMPLEMENTS_INTERFACE(cls,index)                   \
    ( ((index) < (cls)->vftbl->interfacetablelength)            \
      && ( (cls)->vftbl->interfacetable[-(index)] != NULL ) )

/******************************************************************************/
/* DEBUG HELPERS                                                              */
/******************************************************************************/

#ifdef TYPEINFO_DEBUG
#define TYPEINFO_ASSERT(cond)  assert(cond)
#else
#define TYPEINFO_ASSERT(cond)
#endif

/**********************************************************************/
/* TYPEVECTOR FUNCTIONS                                               */
/**********************************************************************/

#if defined(ENABLE_VERIFIER)

/* typevector_copy *************************************************************
 
   Return a copy of the given typevector.
  
   IN:
	   src..............typevector set to copy, must be != NULL
	   size.............number of elements per typevector

   RETURN VALUE:
       a pointer to the new typevector set

*******************************************************************************/

varinfo *
typevector_copy(varinfo *src, int size)
{
	varinfo *dst;
	
	TYPEINFO_ASSERT(src);
	
	dst = DNEW_TYPEVECTOR(size);
	memcpy(dst,src,TYPEVECTOR_SIZE(size));

	return dst;
}

/* typevector_copy_inplace *****************************************************
 
   Copy a typevector to a given destination.

   IN:
	   src..............typevector to copy, must be != NULL
	   dst..............destination to write the copy to
	   size.............number of elements per typevector

*******************************************************************************/

void
typevector_copy_inplace(varinfo *src,varinfo *dst,int size)
{
	memcpy(dst,src,TYPEVECTOR_SIZE(size));
}

/* typevector_checktype ********************************************************
 
   Check if the typevector contains a given type at a given index.
  
   IN:
	   vec..............typevector set, must be != NULL
	   index............index of component to check
	   type.............TYPE_* constant to check against

   RETURN VALUE:
       true if the typevector contains TYPE at INDEX,
	   false otherwise

*******************************************************************************/

bool
typevector_checktype(varinfo *vec,int index,int type)
{
	TYPEINFO_ASSERT(vec);

	return vec[index].type == type;
}

/* typevector_checkreference ***************************************************

   Check if the typevector contains a reference at a given index.

   IN:
	   vec..............typevector, must be != NULL
	   index............index of component to check

   RETURN VALUE:
       true if the typevector contains a reference at INDEX,
	   false otherwise

*******************************************************************************/

bool
typevector_checkreference(varinfo *vec, int index)
{
	TYPEINFO_ASSERT(vec);
	return vec[index].is_reference();
}

/* typevectorset_checkretaddr **************************************************

   Check if the typevectors contains a returnAddress at a given index.

   IN:
	   vec..............typevector, must be != NULL
	   index............index of component to check

   RETURN VALUE:
       true if the typevector contains a returnAddress at INDEX,
	   false otherwise

*******************************************************************************/

bool
typevector_checkretaddr(varinfo *vec,int index)
{
	TYPEINFO_ASSERT(vec);
	return vec[index].is_returnaddress();
}

/* typevector_store ************************************************************

   Store a type at a given index in the typevector.

   IN:
	   vec..............typevector set, must be != NULL
	   index............index of component to set
	   type.............TYPE_* constant of type to set
	   info.............typeinfo of type to set, may be NULL,
	                    if TYPE != TYPE_ADR

*******************************************************************************/

void
typevector_store(varinfo *vec,int index, Type type, typeinfo_t *info)
{
	TYPEINFO_ASSERT(vec);

	vec[index].type = type;
	if (info)
		vec[index].typeinfo = *info;
}

/* typevector_store_retaddr ****************************************************

   Store a returnAddress type at a given index in the typevector.

   IN:
	   vec..............typevector set, must be != NULL
	   index............index of component to set
	   info.............typeinfo of the returnAddress.

*******************************************************************************/

void
typevector_store_retaddr(varinfo *vec,int index,typeinfo_t *info)
{
	TYPEINFO_ASSERT(vec);
	TYPEINFO_ASSERT(info->is_primitive());

	vec[index].type = TYPE_ADR;
	vec[index].typeinfo.init_returnaddress(info->returnaddress());
}

/* typevector_init_object ******************************************************

   Replace all uninitialized object types in the typevector set which were
   created by the given instruction by initialized object types.

   IN:
	   set..............typevector set
	   ins..............instruction which created the uninitialized object type
	   initclass........class of the initialized object type to set
	   size.............number of elements per typevector

   RETURN VALUE:
       true.............success
	   false............an exception has been thrown

   XXX maybe we should do the lazy resolving before calling this function

*******************************************************************************/

bool
typevector_init_object(varinfo *set,void *ins,
					   classref_or_classinfo initclass,
					   int size)
{
	int i;

	for (i=0; i<size; ++i) {
		if (set[i].type == TYPE_ADR
			&& set[i].typeinfo.is_newobject()
			&& set[i].typeinfo.newobject_instruction() == ins)
		{
			if (!set[i].typeinfo.init_class(initclass))
				return false;
		}
	}
	return true;
}

/* typevector_merge ************************************************************
 
   Merge a typevector with another one.
   The given typevectors must have the same number of components.
  
   IN:
       m................method for exception messages
	   dst..............the first typevector
	   y................the second typevector
	   size.............number of elements per typevector

   OUT:
       *dst.............the resulting typevector

   RETURN VALUE:
       typecheck_TRUE...dst has been modified
	   typecheck_FALSE..dst has not been modified
	   typecheck_FAIL...an exception has been thrown

*******************************************************************************/

typecheck_result
typevector_merge(methodinfo *m,varinfo *dst,varinfo *y,int size)
{
	bool changed = false;
	typecheck_result r;

	varinfo *a = dst;
	varinfo *b = y;
	while (size--) {
		if (a->type != TYPE_VOID && a->type != b->type) {
			a->type = TYPE_VOID;
			changed = true;
		}
		else if (a->type == TYPE_ADR) {
			if (a->typeinfo.is_primitive()) {
				/* 'a' is a returnAddress */
				if (!b->typeinfo.is_primitive()
					|| (a->typeinfo.returnaddress() != b->typeinfo.returnaddress()))
				{
					a->type = TYPE_VOID;
					changed = true;
				}
			}
			else {
				/* 'a' is a reference */
				if (b->typeinfo.is_primitive()) {
					a->type = TYPE_VOID;
					changed = true;
				}
				else {
					/* two reference types are merged. There cannot be */
					/* a merge error. In the worst case we get j.l.O.  */
					r = a->typeinfo.merge(m, &(b->typeinfo));
					if (r == typecheck_FAIL)
						return r;
					changed |= r;
				}
			}
		}
		a++;
		b++;
	}
	return (typecheck_result) changed;
}

/**********************************************************************/
/* READ-ONLY FUNCTIONS                                                */
/* The following functions don't change typeinfo data.                */
/**********************************************************************/

/* interface_extends_interface *************************************************

   Check if a resolved interface extends a given resolved interface.

   IN:
	   cls..............the interface, must be linked
	   interf...........the interface to check against

   RETURN VALUE:
       true.............CLS extends INTERF
	   false............CLS does not extend INTERF

*******************************************************************************/

static bool
interface_extends_interface(classinfo *cls,classinfo *interf)
{
    int i;
    
	TYPEINFO_ASSERT(cls);
	TYPEINFO_ASSERT(interf);
	TYPEINFO_ASSERT((interf->flags & ACC_INTERFACE) != 0);
	TYPEINFO_ASSERT((cls->flags & ACC_INTERFACE) != 0);
	TYPEINFO_ASSERT(cls->state & CLASS_LINKED);

    /* first check direct superinterfaces */
    for (i=0; i<cls->interfacescount; ++i) {
        if (cls->interfaces[i] == interf)
            return true;
    }
    
    /* check indirect superinterfaces */
    for (i=0; i<cls->interfacescount; ++i) {
        if (interface_extends_interface(cls->interfaces[i],interf))
            return true;
    }
    
    return false;
}

/* classinfo_implements_interface **********************************************
 
   Check if a resolved class implements a given resolved interface.
   
   IN:
	   cls..............the class
	   interf...........the interface

   RETURN VALUE:
       typecheck_TRUE...CLS implements INTERF
	   typecheck_FALSE..CLS does not implement INTERF
	   typecheck_FAIL...an exception has been thrown

*******************************************************************************/

static typecheck_result
classinfo_implements_interface(classinfo *cls,classinfo *interf)
{
	TYPEINFO_ASSERT(cls);
	TYPEINFO_ASSERT(interf);
	TYPEINFO_ASSERT((interf->flags & ACC_INTERFACE) != 0);

	if (!(cls->state & CLASS_LINKED))
		if (!link_class(cls))
			return typecheck_FAIL;

    if (cls->flags & ACC_INTERFACE) {
        /* cls is an interface */
        if (cls == interf)
            return typecheck_TRUE;

        /* check superinterfaces */
        return (typecheck_result) interface_extends_interface(cls,interf);
    }

	TYPEINFO_ASSERT(cls->state & CLASS_LINKED);
    return (typecheck_result) CLASSINFO_IMPLEMENTS_INTERFACE(cls,interf->index);
}

/* mergedlist_implements_interface *********************************************
 
   Check if all the classes in a given merged list implement a given resolved
   interface.
   
   IN:
	   merged...........the list of merged class types
	   interf...........the interface to check against

   RETURN VALUE:
       typecheck_TRUE...all classes implement INTERF
	   typecheck_FALSE..there is at least one class that does not implement
	                    INTERF
	   typecheck_MAYBE..check cannot be performed now because of unresolved
	                    classes
	   typecheck_FAIL...an exception has been thrown

*******************************************************************************/

static typecheck_result
mergedlist_implements_interface(typeinfo_mergedlist_t *merged,
                                classinfo *interf)
{
    int i;
    classref_or_classinfo *mlist;
	typecheck_result r;

	TYPEINFO_ASSERT(interf);
	TYPEINFO_ASSERT((interf->flags & ACC_INTERFACE) != 0);

    /* Check if there is an non-empty mergedlist. */
    if (!merged)
        return typecheck_FALSE;

    /* If all classinfos in the (non-empty) merged array implement the
     * interface return true, otherwise false.
     */
    mlist = merged->list;
    i = merged->count;
    while (i--) {
		if (mlist->is_classref()) {
			return typecheck_MAYBE;
		}
        r = classinfo_implements_interface((mlist++)->cls,interf);
        if (r != typecheck_TRUE)
			return r;
    }
    return typecheck_TRUE;
}

/* merged_implements_interface *************************************************

   Check if a possible merged type implements a given resolved interface
   interface.

   IN:
       typeclass........(common) class of the (merged) type
	   merged...........the list of merged class types
	   interf...........the interface to check against

   RETURN VALUE:
       typecheck_TRUE...the type implement INTERF
	   typecheck_FALSE..the type does not implement INTERF
	   typecheck_MAYBE..check cannot be performed now because of unresolved
	                    classes
	   typecheck_FAIL...an exception has been thrown

*******************************************************************************/

static typecheck_result
merged_implements_interface(classinfo *typeclass,typeinfo_mergedlist_t *merged,
                            classinfo *interf)
{
	typecheck_result r;
	
    /* primitive types don't support interfaces. */
    if (!typeclass)
        return typecheck_FALSE;

    /* the null type can be cast to any interface type. */
    if (typeclass == pseudo_class_Null)
        return typecheck_TRUE;

    /* check if typeclass implements the interface. */
    r = classinfo_implements_interface(typeclass,interf);
	if (r != typecheck_FALSE)
        return r;

    /* check the mergedlist */
	if (!merged)
		return typecheck_FALSE;
    return mergedlist_implements_interface(merged,interf);
}

/* merged_is_subclass **********************************************************
 
   Check if a possible merged type is a subclass of a given class.
   A merged type is a subclass of a class C if all types in the merged list
   are subclasses of C. A sufficient condition for this is that the
   common type of the merged type is a subclass of C.

   IN:
       typeclass........(common) class of the (merged) type
	                    MUST be a loaded and linked class
	   merged...........the list of merged class types
	   cls..............the class to theck against

   RETURN VALUE:
       typecheck_TRUE...the type is a subclass of CLS
	   typecheck_FALSE..the type is not a subclass of CLS
	   typecheck_MAYBE..check cannot be performed now because of unresolved
	                    classes
	   typecheck_FAIL...an exception has been thrown

*******************************************************************************/

static typecheck_result
merged_is_subclass(classinfo *typeclass,typeinfo_mergedlist_t *merged,
		classinfo *cls)
{
    int i;
    classref_or_classinfo *mlist;

	TYPEINFO_ASSERT(cls);
	
    /* primitive types aren't subclasses of anything. */
    if (!typeclass)
        return typecheck_FALSE;

    /* the null type can be cast to any reference type. */
    if (typeclass == pseudo_class_Null)
        return typecheck_TRUE;

	TYPEINFO_ASSERT(typeclass->state & CLASS_LOADED);
	TYPEINFO_ASSERT(typeclass->state & CLASS_LINKED);

    /* check if the common typeclass is a subclass of CLS. */
	if (class_issubclass(typeclass,cls))
		return typecheck_TRUE;

    /* check the mergedlist */
	if (!merged)
		return typecheck_FALSE;
    /* If all classinfos in the (non-empty) merged list are subclasses
	 * of CLS, return true, otherwise false.
	 * If there is at least one unresolved type in the list,
	 * return typecheck_MAYBE.
     */
    mlist = merged->list;
    i = merged->count;
    while (i--) {
		if (mlist->is_classref()) {
			return typecheck_MAYBE;
		}
		if (!(mlist->cls->state & CLASS_LINKED))
			if (!link_class(mlist->cls))
				return typecheck_FAIL;
		if (!class_issubclass(mlist->cls,cls))
			return typecheck_FALSE;
		mlist++;
    }
    return typecheck_TRUE;
}

/***
 *	Check if a type is assignable to a given class type.
 *
 *	RETURN VALUE:
 *		typecheck_TRUE...the type is assignable
 *		typecheck_FALSE..the type is not assignable
 *		typecheck_MAYBE..check cannot be performed now because of unresolved classes
 *		typecheck_FAIL...an exception has been thrown
 */
typecheck_result typeinfo_t::is_assignable_to_class(classref_or_classinfo dest) const {
    classref_or_classinfo c = typeclass;

    /* assignments of primitive values are not checked here. */
    if (!c.any && !dest.any)
        return typecheck_TRUE;

    /* primitive and reference types are not assignment compatible. */
    if (!c.any || !dest.any)
        return typecheck_FALSE;

    /* the null type can be assigned to any type */
    if (is_nulltype())
        return typecheck_TRUE;

    /* uninitialized objects are not assignable */
    if (is_newobject())
        return typecheck_FALSE;

    Utf8String classname = CLASSREF_OR_CLASSINFO_NAME(c);

	if (dest.is_classref()) {
		/* the destination type is an unresolved class reference */
		/* In this case we cannot tell a lot about assignability. */

		/* the common case of value and dest type having the same classname */
		if (dest.ref->name == classname && !merged)
			return typecheck_TRUE;

		/* we cannot tell if value is assignable to dest, so we */
		/* leave it up to the resolving code to check this      */
		return typecheck_MAYBE;
	}

	/* { we know that dest is a loaded class } */

	if (c.is_classref()) {
		/* the value type is an unresolved class reference */

		/* the common case of value and dest type having the same classname */
		if (dest.cls->name == classname)
			return typecheck_TRUE;

		/* we cannot tell if value is assignable to dest, so we */
		/* leave it up to the resolving code to check this      */
		return typecheck_MAYBE;
	}

	/* { we know that both c and dest are loaded classes } */
	/* (c may still have a merged list containing unresolved classrefs!) */

	TYPEINFO_ASSERT(c.is_classinfo());
	TYPEINFO_ASSERT(dest.is_classinfo());

	classinfo *cls = c.cls;

	TYPEINFO_ASSERT(cls->state & CLASS_LOADED);
	TYPEINFO_ASSERT(dest.cls->state & CLASS_LOADED);

	/* maybe we need to link the classes */
	if (!(cls->state & CLASS_LINKED))
		if (!link_class(cls))
			return typecheck_FAIL;
	if (!(dest.cls->state & CLASS_LINKED))
		if (!link_class(dest.cls))
			return typecheck_FAIL;

	/* { we know that both c and dest are linked classes } */
	TYPEINFO_ASSERT(cls->state & CLASS_LINKED);
	TYPEINFO_ASSERT(dest.cls->state & CLASS_LINKED);

    if (dest.cls->flags & ACC_INTERFACE) {
        /* We are assigning to an interface type. */
        return merged_implements_interface(cls, merged, dest.cls);
    }

    if (CLASSINFO_IS_ARRAY(dest.cls)) {
		arraydescriptor *arraydesc    = dest.cls->vftbl->arraydesc;
		int              dimension    = arraydesc->dimension;
		classinfo       *elementclass = (arraydesc->elementvftbl) ? arraydesc->elementvftbl->clazz : NULL;
			
        /* We are assigning to an array type. */
        if (!is_array())
            return typecheck_FALSE;

        /* {Both value and dest.cls are array types.} */

        /* value must have at least the dimension of dest.cls. */
        if (this->dimension < dimension)
            return typecheck_FALSE;

        if (this->dimension > dimension) {
            /* value has higher dimension so we need to check
             * if its component array can be assigned to the
             * element type of dest.cls */

			if (!elementclass) return typecheck_FALSE;
            
            if (elementclass->flags & ACC_INTERFACE) {
                /* We are assigning to an interface type. */
                return classinfo_implements_interface(pseudo_class_Arraystub, elementclass);
            }

            /* We are assigning to a class type. */
            return (typecheck_result) class_issubclass(pseudo_class_Arraystub, elementclass);
        }

        /* {value and dest.cls have the same dimension} */

        if (elementtype != arraydesc->elementtype)
            return typecheck_FALSE;

        if (this->elementclass.any) {
            /* We are assigning an array of objects so we have to
             * check if the elements are assignable.
             */

            if (elementclass->flags & ACC_INTERFACE) {
                /* We are assigning to an interface type. */

                return merged_implements_interface(this->elementclass.cls, merged, elementclass);
            }

            /* We are assigning to a class type. */
            return merged_is_subclass(this->elementclass.cls, merged, elementclass);
        }

        return typecheck_TRUE;
    }

    /* {dest.cls is not an array} */
    /* {dest.cls is a loaded class} */

	/* If there are any unresolved references in the merged list, we cannot */
	/* tell if the assignment will be ok.                                   */
	/* This can only happen when cls is java.lang.Object                    */
	if (cls == class_java_lang_Object && merged) {
		classref_or_classinfo *mlist = merged->list;
		int i = merged->count;
		while (i--) {
			if (mlist->is_classref())
				return typecheck_MAYBE;
			mlist++;
		}
	}

    /* We are assigning to a class type */
    if (cls->flags & ACC_INTERFACE)
        cls = class_java_lang_Object;

    return merged_is_subclass(cls, merged, dest.cls);
}

/***
 *
 *	Check if a type is assignable to a given type.
 *
 *	@param dest  the type of the destination, must not be a merged type
 *
 *	RETURN VALUE:
 *		typecheck_TRUE...the type is assignable
 *		typecheck_FALSE..the type is not assignable
 *		typecheck_MAYBE..check cannot be performed now because of unresolved classes
 *		typecheck_FAIL...an exception has been thrown
 */
typecheck_result typeinfo_t::is_assignable_to(typeinfo_t *dest) const {
	TYPEINFO_ASSERT(dest);
	TYPEINFO_ASSERT(dest->merged == NULL);

	return is_assignable_to_class(dest->typeclass);
}

/**********************************************************************/
/* INITIALIZATION FUNCTIONS                                           */
/* The following functions fill in uninitialized typeinfo structures. */
/**********************************************************************/

/* internally used macros ***************************************************/

#define TYPEINFO_ALLOCMERGED(mergedlist,count)					\
    do {(mergedlist) = (typeinfo_mergedlist_t *) DumpMemory::allocate(sizeof(typeinfo_mergedlist_t) \
            + ((count)-1)*sizeof(classinfo*));} while(0)

#define TYPEINFO_FREEMERGED(mergedlist)

#define TYPEINFO_FREEMERGED_IF_ANY(mergedlist)


/* typeinfo_t::init_class ******************************************************

   Initialize a typeinfo to a resolved class.

   IN:
	   c................the class

   OUT:
       *info............is initialized

   RETURN VALUE:
       true.............success
	   false............an exception has been thrown

*******************************************************************************/

void typeinfo_t::init_class(classinfo *c) {
	if ((typeclass.cls = c)->vftbl->arraydesc) {
		if (c->vftbl->arraydesc->elementvftbl)
			elementclass.cls = c->vftbl->arraydesc->elementvftbl->clazz;
		else
			elementclass.any = NULL;
		dimension   = c->vftbl->arraydesc->dimension;
		elementtype = c->vftbl->arraydesc->elementtype;
	}
	else {
		elementclass.any = NULL;
		dimension        = 0;
		elementtype      = ARRAYTYPE_INT;
	}
	merged = NULL;
}

/* typeinfo_t::init_class ******************************************************

   Initialize a typeinfo to a possibly unresolved class type.

   IN:
	   c................the class type

   OUT:
       *info............is initialized

   RETURN VALUE:
       true.............success
	   false............an exception has been thrown

*******************************************************************************/

bool typeinfo_t::init_class(classref_or_classinfo c) {
	TYPEINFO_ASSERT(c.any);

	classinfo  *cls;

	/* if necessary, try to resolve lazily */
	if (!resolve_classref_or_classinfo(NULL /* XXX should know method */,
				c,resolveLazy,false,true,&cls))
	{
		return false;
	}

	if (cls) {
		init_class(cls);
		return true;
	}

	/* {the type could no be resolved lazily} */

	typeclass.ref    = c.ref;
	elementclass.any = NULL;
	dimension        = 0;
	merged           = NULL;

	/* handle array type references */
	const char *utf_ptr = c.ref->name.begin();
	int         len     = c.ref->name.size();
	if (*utf_ptr == '[') {
		/* count dimensions */
		while (*utf_ptr == '[') {
			utf_ptr++;
			dimension++;
			len--;
		}
		if (*utf_ptr == 'L') {
			utf_ptr++;
			len -= 2;
			elementtype      = ARRAYTYPE_OBJECT;
			elementclass.ref = class_get_classref(c.ref->referer, Utf8String::from_utf8(utf_ptr,len));
		}
		else {
			/* an array with primitive element type */
			/* should have been resolved above */
			TYPEINFO_ASSERT(false);
		}
	}
	return true;
}

/* typeinfo_t::init_from_typedesc **********************************************

   Initialize a typeinfo from a typedesc.
   
   IN:
	   desc.............the typedesc

   OUT:
       *type............set to the TYPE_* constant of DESC (if type != NULL)
       *info............receives the typeinfo (if info != NULL)

   RETURN VALUE:
       true.............success
	   false............an exception has been thrown

*******************************************************************************/

bool typeinfo_t::init_from_typedesc(const typedesc *desc, u1 *type) {
#ifdef TYPEINFO_VERBOSE
	fprintf(stderr,"typeinfo_init_from_typedesc(");
	descriptor_debug_print_typedesc(stderr,this);
	fprintf(stderr,")\n");
#endif

	if (type)
		*type = desc->type;

	if (desc->type == TYPE_ADR) {
		TYPEINFO_ASSERT(desc->classref);
		if (!init_class(desc->classref))
			return false;
	}
	else {
		init_primitive();
	}
	return true;
}


/* typedescriptor_init_from_typedesc *******************************************
 
   Initialize a typedescriptor from a typedesc.
   
   IN:
	   desc.............the typedesc

   OUT:
       *td..............receives the typedescriptor
	                    td must be != NULL

   RETURN VALUE:
       true.............success
	   false............an exception has been thrown

*******************************************************************************/

static bool typedescriptor_init_from_typedesc(typedescriptor_t *td, typedesc *desc) {
	TYPEINFO_ASSERT(td);
	TYPEINFO_ASSERT(desc);

	td->type = desc->type;
	if (td->type == TYPE_ADR) {
		if (!td->typeinfo.init_class(desc->classref))
			return false;
	}
	else {
		td->typeinfo.init_primitive();
	}
	return true;
}

/* typeinfo_init_varinfo_from_typedesc *****************************************
 
   Initialize a varinfo from a typedesc.
   
   IN:
	   desc.............the typedesc

   OUT:
       *var.............receives the type
	                    var must be != NULL

   RETURN VALUE:
       true.............success
	   false............an exception has been thrown

*******************************************************************************/

static bool typeinfo_init_varinfo_from_typedesc(varinfo *var, typedesc *desc) {
	TYPEINFO_ASSERT(var);
	TYPEINFO_ASSERT(desc);

	var->type = desc->type;
	if (var->type == TYPE_ADR) {
		if (!var->typeinfo.init_class(desc->classref))
			return false;
	}
	else {
		var->typeinfo.init_primitive();
	}
	return true;
}

/* typeinfo_init_varinfos_from_methoddesc **************************************
 
   Initialize an array of varinfos from a methoddesc.
   
   IN:
       desc.............the methoddesc
       buflen...........number of parameters the buffer can hold
	   startindex.......the zero-based index of the first parameter to
	                    write to the array. In other words the number of
						parameters to skip at the beginning of the methoddesc.
	   map..............map from parameter indices to varinfo indices
	                    (indexed like jitdata.local_map)

   OUT:
       *vars............array receiving the varinfos
	                    td[0] receives the type of the
						(startindex+1)th parameter of the method
       *returntype......receives the typedescriptor of the return type.
	                    returntype may be NULL

   RETURN VALUE:
       true.............everything ok
	   false............an exception has been thrown

   NOTE:
       If (according to BUFLEN) the buffer is to small to hold the
	   parameter types, an internal error is thrown. This must be
	   avoided by checking the number of parameters and allocating enough
	   space before calling this function.

*******************************************************************************/

bool
typeinfo_init_varinfos_from_methoddesc(varinfo *vars,
									 methoddesc *desc,
									 int buflen, int startindex,
									 s4 *map,
									 typedescriptor_t *returntype)
{
	s4 i;
    s4 varindex;
	s4 type;
	s4 slot = 0;

	/* skip arguments */
	for (i=0; i<startindex; ++i) {
		slot++;
		if (IS_2_WORD_TYPE(desc->paramtypes[i].type))
			slot++;
	}

    /* check arguments */
    for (i=startindex; i<desc->paramcount; ++i) {
		type = desc->paramtypes[i].type;
		varindex = map[5*slot + type];

		slot++;
		if (IS_2_WORD_TYPE(type))
			slot++;

		if (varindex == jitdata::UNUSED)
			continue;

		if (varindex >= buflen) {
			exceptions_throw_internalerror("Buffer too small for method arguments.");
			return false;
		}

		if (!typeinfo_init_varinfo_from_typedesc(vars + varindex, desc->paramtypes + i))
			return false;
    }

    /* check returntype */
    if (returntype) {
		if (!typedescriptor_init_from_typedesc(returntype,&(desc->returntype)))
			return false;
	}

	return true;
}

/* typedescriptors_init_from_methoddesc ****************************************
 
   Initialize an array of typedescriptors from a methoddesc.
   
   IN:
       desc.............the methoddesc
       buflen...........number of parameters the buffer can hold
       twoword..........if true, use two parameter slots for two-word types
	   startindex.......the zero-based index of the first parameter to
	                    write to the array. In other words the number of
						parameters to skip at the beginning of the methoddesc.

   OUT:
       *td..............array receiving the typedescriptors.
	                    td[0] receives the typedescriptor of the
						(startindex+1)th parameter of the method
       *returntype......receives the typedescriptor of the return type.
	                    returntype may be NULL

   RETURN VALUE:
       >= 0.............number of typedescriptors filled in TD
	   -1...............an exception has been thrown

   NOTE:
       If (according to BUFLEN) the buffer is to small to hold the
	   parameter types, an internal error is thrown. This must be
	   avoided by checking the number of parameters and allocating enough
	   space before calling this function.

*******************************************************************************/

int
typedescriptors_init_from_methoddesc(typedescriptor_t *td,
									 methoddesc *desc,
									 int buflen,bool twoword,int startindex,
									 typedescriptor_t *returntype)
{
	int i;
    int args = 0;

    /* check arguments */
    for (i=startindex; i<desc->paramcount; ++i) {
		if (++args > buflen) {
			exceptions_throw_internalerror("Buffer too small for method arguments.");
			return -1;
		}

		if (!typedescriptor_init_from_typedesc(td,desc->paramtypes + i))
			return -1;
		td++;

		if (twoword && (td[-1].type == TYPE_LNG || td[-1].type == TYPE_DBL)) {
			if (++args > buflen) {
				exceptions_throw_internalerror("Buffer too small for method arguments.");
				return -1;
			}

			td->type = TYPE_VOID;
			td->typeinfo.init_primitive();
			td++;
		}
    }

    /* check returntype */
    if (returntype) {
		if (!typedescriptor_init_from_typedesc(returntype,&(desc->returntype)))
			return -1;
	}

	return args;
}

/* typeinfo_t::init_component ***************************************************

   Initialize a typeinfo with the component type of a given array type.
   
   IN:
	   srcarray.........the typeinfo of the array type

   OUT:
       *dst.............receives the typeinfo of the component type

   RETURN VALUE:
       true.............success
	   false............an exception has been thrown

*******************************************************************************/

bool typeinfo_t::init_component(const typeinfo_t& srcarray) {
	if (srcarray.is_nulltype()) {
		init_nulltype();
		return true;
	}

	if (!srcarray.is_array()) {
		/* XXX should we make that a verify error? */
		exceptions_throw_internalerror("Trying to access component of non-array");
		return false;
	}

	/* save the mergedlist (maybe this == srcarray) */

	typeinfo_mergedlist_t *merged = srcarray.merged;

	if (srcarray.typeclass.is_classref()) {
		constant_classref *comp = class_get_classref_component_of(srcarray.typeclass.ref);

		if (comp) {
			if (!init_class(comp))
				return false;
		}
		else {
			init_primitive();
		}
	}
	else {
		if (!(srcarray.typeclass.cls->state & CLASS_LINKED)) {
			if (!link_class(srcarray.typeclass.cls)) {
				return false;
			}
		}

		TYPEINFO_ASSERT(srcarray.typeclass.cls->vftbl);
		TYPEINFO_ASSERT(srcarray.typeclass.cls->vftbl->arraydesc);

		if (vftbl_t *comp = srcarray.typeclass.cls->vftbl->arraydesc->componentvftbl)
			init_class(comp->clazz);
		else
			init_primitive();
	}
    
    this->merged = merged; /* XXX should we do a deep copy? */
	return true;
}

/***
 * Create a deep copy of the `merged' list of a typeinfo
 */
void typeinfo_t::clone_merged(const typeinfo_t& src, typeinfo_t& dst) {
	int count = src.merged->count;
	TYPEINFO_ALLOCMERGED(dst.merged,count);
	dst.merged->count = count;

	classref_or_classinfo *srclist = src.merged->list;
	classref_or_classinfo *dstlist = dst.merged->list;
	
	while (count--)
		*dstlist++ = *srclist++;
}


/**********************************************************************/
/* MISCELLANEOUS FUNCTIONS                                            */
/**********************************************************************/

/**********************************************************************/
/* MERGING FUNCTIONS                                                  */
/* The following functions are used to merge the types represented by */
/* two typeinfo structures into one typeinfo structure.               */
/**********************************************************************/

static void typeinfo_merge_error(methodinfo *m, const char *str, const typeinfo_t *x, const typeinfo_t *y) {
#ifdef TYPEINFO_VERBOSE
    fprintf(stderr,"Error in typeinfo_merge: %s\n",str);
    fprintf(stderr,"Typeinfo x:\n");
    typeinfo_print(stderr,x,1);
    fprintf(stderr,"Typeinfo y:\n");
    typeinfo_print(stderr,y,1);
    log_text(str);
#endif

	exceptions_throw_verifyerror(m, str);
}

/* Condition: clsx != clsy. */
/* Returns: true if dest was changed (currently always true). */
static
bool
typeinfo_merge_two(typeinfo_t *dest,classref_or_classinfo clsx,classref_or_classinfo clsy)
{
	TYPEINFO_ASSERT(dest);
    TYPEINFO_FREEMERGED_IF_ANY(dest->merged);
    TYPEINFO_ALLOCMERGED(dest->merged,2);
    dest->merged->count = 2;

	TYPEINFO_ASSERT(clsx.any != clsy.any);

    if (clsx.any < clsy.any) {
        dest->merged->list[0] = clsx;
        dest->merged->list[1] = clsy;
    }
    else {
        dest->merged->list[0] = clsy;
        dest->merged->list[1] = clsx;
    }

    return true;
}

/* Returns: true if dest was changed. */
static
bool
typeinfo_merge_add(typeinfo_t *dest,typeinfo_mergedlist_t *m,classref_or_classinfo cls)
{
    int count;
    typeinfo_mergedlist_t *newmerged;
    classref_or_classinfo *mlist,*newlist;

    count = m->count;
    mlist = m->list;

    /* Check if cls is already in the mergedlist m. */
    while (count--) {
        if ((mlist++)->any == cls.any) { /* XXX check equal classrefs? */
            /* cls is in the list, so m is the resulting mergedlist */
            if (dest->merged == m)
                return false;

            /* We have to copy the mergedlist */
            TYPEINFO_FREEMERGED_IF_ANY(dest->merged);
            count = m->count;
            TYPEINFO_ALLOCMERGED(dest->merged,count);
            dest->merged->count = count;
            newlist = dest->merged->list;
            mlist = m->list;
            while (count--) {
                *newlist++ = *mlist++;
            }
            return true;
        }
    }

    /* Add cls to the mergedlist. */
    count = m->count;
    TYPEINFO_ALLOCMERGED(newmerged,count+1);
    newmerged->count = count+1;
    newlist = newmerged->list;    
    mlist = m->list;
    while (count) {
        if (mlist->any > cls.any)
            break;
        *newlist++ = *mlist++;
        count--;
    }
    *newlist++ = cls;
    while (count--) {
        *newlist++ = *mlist++;
    }

    /* Put the new mergedlist into dest. */
    TYPEINFO_FREEMERGED_IF_ANY(dest->merged);
    dest->merged = newmerged;
    
    return true;
}

/* Returns: true if dest was changed. */
static
bool
typeinfo_merge_mergedlists(typeinfo_t *dest,typeinfo_mergedlist_t *x,
                           typeinfo_mergedlist_t *y)
{
    int count = 0;
    int countx,county;
    typeinfo_mergedlist_t *temp,*result;
    classref_or_classinfo *clsx,*clsy,*newlist;

    /* count the elements that will be in the resulting list */
    /* (Both lists are sorted, equal elements are counted only once.) */
    clsx = x->list;
    clsy = y->list;
    countx = x->count;
    county = y->count;
    while (countx && county) {
        if (clsx->any == clsy->any) {
            clsx++;
            clsy++;
            countx--;
            county--;
        }
        else if (clsx->any < clsy->any) {
            clsx++;
            countx--;
        }
        else {
            clsy++;
            county--;
        }
        count++;
    }
    count += countx + county;

    /* {The new mergedlist will have count entries.} */

    if ((x->count != count) && (y->count == count)) {
        temp = x; x = y; y = temp;
    }
    /* {If one of x,y is already the result it is x.} */
    if (x->count == count) {
        /* x->merged is equal to the result */
        if (x == dest->merged)
            return false;

        if (!dest->merged || dest->merged->count != count) {
            TYPEINFO_FREEMERGED_IF_ANY(dest->merged);
            TYPEINFO_ALLOCMERGED(dest->merged,count);
            dest->merged->count = count;
        }

        newlist = dest->merged->list;
        clsx = x->list;
        while (count--) {
            *newlist++ = *clsx++;
        }
        return true;
    }

    /* {We have to merge two lists.} */

    /* allocate the result list */
    TYPEINFO_ALLOCMERGED(result,count);
    result->count = count;
    newlist = result->list;

    /* merge the sorted lists */
    clsx = x->list;
    clsy = y->list;
    countx = x->count;
    county = y->count;
    while (countx && county) {
        if (clsx->any == clsy->any) {
            *newlist++ = *clsx++;
            clsy++;
            countx--;
            county--;
        }
        else if (clsx->any < clsy->any) {
            *newlist++ = *clsx++;
            countx--;
        }
        else {
            *newlist++ = *clsy++;
            county--;
        }
    }
    while (countx--)
            *newlist++ = *clsx++;
    while (county--)
            *newlist++ = *clsy++;

    /* replace the list in dest with the result list */
    TYPEINFO_FREEMERGED_IF_ANY(dest->merged);
    dest->merged = result;

    return true;
}

/* typeinfo_merge_nonarrays ****************************************************
 
   Merge two non-array types.
   
   IN:
       x................the first type
	   y................the second type
	   mergedx..........merged list of the first type, may be NULL
	   mergedy..........merged list of the descond type, may be NULL

   OUT:
       *dest............receives the resulting merged list
	   *result..........receives the resulting type

   RETURN VALUE:
       typecheck_TRUE...*dest has been modified
	   typecheck_FALSE..*dest has not been modified
	   typecheck_FAIL...an exception has been thrown

   NOTE:
       RESULT is an extra parameter so it can point to dest->typeclass or to
	   dest->elementclass.

*******************************************************************************/

static typecheck_result
typeinfo_merge_nonarrays(typeinfo_t *dest,
                         classref_or_classinfo *result,
                         classref_or_classinfo x,classref_or_classinfo y,
                         typeinfo_mergedlist_t *mergedx,
                         typeinfo_mergedlist_t *mergedy)
{
	classinfo *tcls,*common;
	bool changed;
	typecheck_result r;

	TYPEINFO_ASSERT(dest && result && x.any && y.any);
	TYPEINFO_ASSERT(x.cls != pseudo_class_Null);
	TYPEINFO_ASSERT(y.cls != pseudo_class_Null);
	TYPEINFO_ASSERT(x.cls != pseudo_class_New);
	TYPEINFO_ASSERT(y.cls != pseudo_class_New);

	/*--------------------------------------------------*/
	/* common cases                                     */
	/*--------------------------------------------------*/

    /* Common case 1: x and y are the same class or class reference */
    /* (This case is very simple unless *both* x and y really represent
     *  merges of subclasses of clsx==clsy.)
     */
    if ( (x.any == y.any) && (!mergedx || !mergedy) ) {
  return_simple_x:
        /* DEBUG */ /* log_text("return simple x"); */
        changed = (dest->merged != NULL);
        TYPEINFO_FREEMERGED_IF_ANY(dest->merged);
        dest->merged = NULL;
        *result = x;
        /* DEBUG */ /* log_text("returning"); */
        return (typecheck_result) changed;
    }

	Utf8String xname = CLASSREF_OR_CLASSINFO_NAME(x);
	Utf8String yname = CLASSREF_OR_CLASSINFO_NAME(y);

	/* Common case 2: xname == yname, at least one unresolved */
	if ((xname == yname) && (x.is_classref() || y.is_classref()))
	{
		/* use the loaded one if any */
		if (y.is_classinfo())
			x = y;
		goto return_simple_x;
	}

	/*--------------------------------------------------*/
	/* non-trivial cases                                */
	/*--------------------------------------------------*/

#ifdef TYPEINFO_VERBOSE
	{
		typeinfo_t dbgx,dbgy;
		fprintf(stderr,"merge_nonarrays:\n");
		fprintf(stderr,"    ");if(x.is_classref())fprintf(stderr,"<ref>");utf_fprint_printable_ascii(stderr,xname);fprintf(stderr,"\n");
		fprintf(stderr,"    ");if(y.is_classref())fprintf(stderr,"<ref>");utf_fprint_printable_ascii(stderr,yname);fprintf(stderr,"\n");
		fflush(stderr);
		dbgx.init_class(x);
		dbgx.merged = mergedx;
		dbgy.init_class(y);
		dbgy.merged = mergedy;
		typeinfo_print(stderr,&dbgx,4);
		fprintf(stderr,"  with:\n");
		typeinfo_print(stderr,&dbgy,4);
	}
#endif

	TYPEINFO_ASSERT(x.is_classref() || (x.cls->state & CLASS_LOADED));
	TYPEINFO_ASSERT(y.is_classref() || (y.cls->state & CLASS_LOADED));

	/* If y is unresolved or an interface, swap x and y. */
	if (y.is_classref() || (x.is_classinfo() && y.cls->flags & ACC_INTERFACE)) {
		classref_or_classinfo  tmp     = x;       x       = y;       y       = tmp;
		typeinfo_mergedlist_t *tmerged = mergedx; mergedx = mergedy; mergedy = tmerged;
	}

    /* {We know: If only one of x,y is unresolved it is x,} */
    /* {         If both x,y are resolved and only one of x,y is an interface it is x.} */

	if (x.is_classref()) {
		/* {We know: x and y have different class names} */

        /* Check if we are merging an unresolved type with java.lang.Object */
        if (y.cls == class_java_lang_Object && !mergedy) {
            x = y;
            goto return_simple_x;
        }

		common = class_java_lang_Object;
		goto merge_with_simple_x;
	}

	/* {We know: both x and y are resolved} */
    /* {We know: If only one of x,y is an interface it is x.} */

	TYPEINFO_ASSERT(x.is_classinfo() && y.is_classinfo());
	TYPEINFO_ASSERT(x.cls->state & CLASS_LOADED);
	TYPEINFO_ASSERT(y.cls->state & CLASS_LOADED);

    /* Handle merging of interfaces: */
    if (x.cls->flags & ACC_INTERFACE) {
        /* {x.cls is an interface and mergedx == NULL.} */

        if (y.cls->flags & ACC_INTERFACE) {
            /* We are merging two interfaces. */
            /* {mergedy == NULL} */

            /* {We know that x.cls!=y.cls (see common case at beginning.)} */
            result->cls = class_java_lang_Object;
            return (typecheck_result) typeinfo_merge_two(dest,x,y);
        }

        /* {We know: x is an interface, y is a class.} */

        /* Check if we are merging an interface with java.lang.Object */
        if (y.cls == class_java_lang_Object && !mergedy) {
            x = y;
            goto return_simple_x;
        }

        /* If the type y implements x then the result of the merge
         * is x regardless of mergedy.
         */

		/* we may have to link the classes */
		if (!(x.cls->state & CLASS_LINKED))
			if (!link_class(x.cls))
				return typecheck_FAIL;
		if (!(y.cls->state & CLASS_LINKED))
			if (!link_class(y.cls))
				return typecheck_FAIL;
        
		TYPEINFO_ASSERT(x.cls->state & CLASS_LINKED);
		TYPEINFO_ASSERT(y.cls->state & CLASS_LINKED);

        if (CLASSINFO_IMPLEMENTS_INTERFACE(y.cls,x.cls->index))
		{
            /* y implements x, so the result of the merge is x. */
            goto return_simple_x;
		}
		
        r = mergedlist_implements_interface(mergedy,x.cls);
		if (r == typecheck_FAIL)
			return r;
		if (r == typecheck_TRUE)
        {
            /* y implements x, so the result of the merge is x. */
            goto return_simple_x;
        }
        
        /* {We know: x is an interface, the type y a class or a merge
         * of subclasses and is not guaranteed to implement x.} */

        common = class_java_lang_Object;
        goto merge_with_simple_x;
    }

    /* {We know: x and y are classes (not interfaces).} */
    
	/* we may have to link the classes */
	if (!(x.cls->state & CLASS_LINKED))
		if (!link_class(x.cls))
			return typecheck_FAIL;
	if (!(y.cls->state & CLASS_LINKED))
		if (!link_class(y.cls))
			return typecheck_FAIL;
        
	TYPEINFO_ASSERT(x.cls->state & CLASS_LINKED);
	TYPEINFO_ASSERT(y.cls->state & CLASS_LINKED);

    /* If *x is deeper in the inheritance hierarchy swap x and y. */
	if (x.cls->index > y.cls->index) {
		classref_or_classinfo  tmp     = x;       x       = y;       y       = tmp;
		typeinfo_mergedlist_t *tmerged = mergedx; mergedx = mergedy; mergedy = tmerged;
	}

    /* {We know: y is at least as deep in the hierarchy as x.} */

    /* Find nearest common anchestor for the classes. */

    common = x.cls;
    tcls   = y.cls;

    while (tcls->index > common->index)
        tcls = tcls->super;

    while (common != tcls) {
        common = common->super;
        tcls = tcls->super;
    }

    /* {common == nearest common anchestor of x and y.} */

    /* If x.cls==common and x is a whole class (not a merge of subclasses)
     * then the result of the merge is x.
     */
    if (x.cls == common && !mergedx) {
        goto return_simple_x;
    }
   
    if (mergedx) {
        result->cls = common;
        if (mergedy)
            return (typecheck_result) typeinfo_merge_mergedlists(dest,mergedx,mergedy);
        else
            return (typecheck_result) typeinfo_merge_add(dest,mergedx,y);
    }

merge_with_simple_x:
    result->cls = common;
    if (mergedy)
        return (typecheck_result) typeinfo_merge_add(dest,mergedy,x);
    else
        return (typecheck_result) typeinfo_merge_two(dest,x,y);
}

/***
 *
 *	Merge two types, stores result of merge in `this'.
 *
 *	@param m  method for exception messages
 *	@param t  the second type
 *
 *	RETURN VALUE:
 *		typecheck_TRUE...*dest has been modified
 *		typecheck_FALSE..*dest has not been modified
 *		typecheck_FAIL...an exception has been thrown
 *
 *	@pre
 *		1) *dest must be a valid initialized typeinfo
 *		2) dest != y
 */
typecheck_result typeinfo_t::merge(methodinfo *m, const typeinfo_t *src) {
	const typeinfo_t *x;
	classref_or_classinfo common;
	classref_or_classinfo elementclass;
	int dimension;
	bool changed;
	typecheck_result r;

	/*--------------------------------------------------*/
	/* fast checks                                      */
	/*--------------------------------------------------*/

	/* Merging something with itself is a nop */
	if (this == src)
		return typecheck_FALSE;

	/* Merging two returnAddress types is ok. */
	/* Merging two different returnAddresses never happens, as the verifier */
	/* keeps them separate in order to check all the possible return paths  */
	/* from JSR subroutines.                                                */
	if (!typeclass.any && !src->typeclass.any) {
		TYPEINFO_ASSERT(returnaddress() == src->returnaddress());
		return typecheck_FALSE;
	}

	/* Primitive types cannot be merged with reference types */
	/* This must be checked before calls to typeinfo_merge.  */
	TYPEINFO_ASSERT(this->typeclass.any && src->typeclass.any);

	/* handle uninitialized object types */
	if (is_newobject() || src->is_newobject()) {
		if (!is_newobject() || !src->is_newobject()) {
			typeinfo_merge_error(m,(char*) "Trying to merge uninitialized object type.", this, src);
			return typecheck_FAIL;
		}
		if (newobject_instruction() != src->newobject_instruction()) {
			typeinfo_merge_error(m,(char*) "Trying to merge different uninitialized objects.", this, src);
			return typecheck_FAIL;
		}
		/* the same uninitialized object -- no change */
		return typecheck_FALSE;
	}

	/*--------------------------------------------------*/
	/* common cases                                     */
	/*--------------------------------------------------*/

    /* Common case: dest and y are the same class or class reference */
    /* (This case is very simple unless *both* dest and y really represent
     *  merges of subclasses of class dest==class y.)
     */
	if ((this->typeclass.any == src->typeclass.any) && (!this->merged || !src->merged)) {
return_simple:
		changed = (merged != NULL);
		TYPEINFO_FREEMERGED_IF_ANY(merged);
		merged = NULL;
		return (typecheck_result) changed;
	}

    /* Handle null types: */
	if (src->is_nulltype()) {
		return typecheck_FALSE;
	}
	if (is_nulltype()) {
 		TYPEINFO_FREEMERGED_IF_ANY(dest->merged);
		typeinfo_t::clone(*src, *this);
		return typecheck_TRUE;
	}

	/* Common case: two types with the same name, at least one unresolved */
	if (typeclass.is_classref()) {
		if (src->typeclass.is_classref()) {
			if (typeclass.ref->name == src->typeclass.ref->name)
				goto return_simple;
		}
		else {
			/* XXX should we take y instead of dest here? */
			if (typeclass.ref->name == src->typeclass.cls->name)
				goto return_simple;
		}
	}
	else {
		if (src->typeclass.is_classref() && (typeclass.cls->name == src->typeclass.ref->name))
		{
			goto return_simple;
		}
	}

	/*--------------------------------------------------*/
	/* non-trivial cases                                */
	/*--------------------------------------------------*/

#ifdef TYPEINFO_VERBOSE
	fprintf(stderr,"merge:\n");
    typeinfo_print(stderr,dest,4);
    typeinfo_print(stderr,y,4);
#endif

    /* This function uses x internally, so x and y can be swapped
     * without changing dest. */
    x       = this;
    changed = false;
    
    /* Handle merging of arrays: */
    if (x->is_array() && src->is_array()) {
        
        /* Make x the one with lesser dimension */
        if (x->dimension > src->dimension) {
            const typeinfo_t *tmp = x; x = src; src = tmp;
        }

        /* If one array (y) has higher dimension than the other,
         * interpret it as an array (same dim. as x) of Arraystubs. */
        if (x->dimension < src->dimension) {
            dimension = x->dimension;
            elementtype = ARRAYTYPE_OBJECT;
            elementclass.cls = pseudo_class_Arraystub;
        }
        else {
            dimension    = src->dimension;
            elementtype  = src->elementtype;
            elementclass = src->elementclass;
        }
        
        /* {The arrays are of the same dimension.} */
        
        if (x->elementtype != elementtype) {
            /* Different element types are merged, so the resulting array
             * type has one accessible dimension less. */
            if (--dimension == 0) {
                common.cls       = pseudo_class_Arraystub;
                elementtype      = ARRAYTYPE_INT;
                elementclass.any = NULL;
            }
            else {
                common.cls = class_multiarray_of(dimension,pseudo_class_Arraystub,true);
				if (!common.cls) {
					exceptions_throw_internalerror("XXX Coult not create array class");
					return typecheck_FAIL;
				}

                elementtype      = ARRAYTYPE_OBJECT;
                elementclass.cls = pseudo_class_Arraystub;
            }
        }
        else {
            /* {The arrays have the same dimension and elementtype.} */

            if (elementtype == ARRAYTYPE_OBJECT) {
                /* The elements are references, so their respective
                 * types must be merged.
                 */
				r = typeinfo_merge_nonarrays(this,
				                             &elementclass,
				                             x->elementclass,
				                             elementclass,
				                             x->merged, src->merged);
				TYPEINFO_ASSERT(r != typecheck_MAYBE);
				if (r == typecheck_FAIL)
					return r;
				changed |= r;

                /* DEBUG */ /* log_text("finding resulting array class: "); */
				if (elementclass.is_classref())
					common.ref = class_get_classref_multiarray_of(dimension,elementclass.ref);
				else {
					common.cls = class_multiarray_of(dimension,elementclass.cls,true);
					if (!common.cls) {
						exceptions_throw_internalerror("XXX Coult not create array class");
						return typecheck_FAIL;
					}
				}
                /* DEBUG */ /* utf_display_printable_ascii(common->name); printf("\n"); */
            }
			else {
				common.any = src->typeclass.any;
			}
        }
    }
    else {
        /* {We know that at least one of x or y is no array, so the
         *  result cannot be an array.} */

		r = typeinfo_merge_nonarrays(this,
		                             &common,
		                             x->typeclass, src->typeclass,
		                             x->merged,    src->merged);
		TYPEINFO_ASSERT(r != typecheck_MAYBE);
		if (r == typecheck_FAIL)
			return r;
		changed |= r;

        dimension        = 0;
        elementtype      = ARRAYTYPE_INT;
        elementclass.any = NULL;
    }

    /* Put the new values into dest if neccessary. */

    if (this->typeclass.any != common.any) {
        this->typeclass.any = common.any;
        changed = true;
    }
    if (this->dimension != dimension) {
        this->dimension = dimension;
        changed = true;
    }
    if (this->elementtype != elementtype) {
        this->elementtype = elementtype;
        changed = true;
    }
    if (this->elementclass.any != elementclass.any) {
        this->elementclass.any = elementclass.any;
        changed = true;
    }

    return (typecheck_result) changed;
}
#endif /* ENABLE_VERIFER */


/**********************************************************************/
/* DEBUGGING HELPERS                                                  */
/**********************************************************************/

#ifdef TYPEINFO_DEBUG

#if 0
static int
typeinfo_test_compare(classref_or_classinfo *a,classref_or_classinfo *b)
{
    if (a->any == b->any) return 0;
    if (a->any < b->any) return -1;
    return +1;
}

static void
typeinfo_test_parse(typeinfo_t *info,char *str)
{
    int num;
    int i;
    typeinfo_t *infobuf;
    u1 *typebuf;
    int returntype;
    Utf8String desc = Utf8String::from_utf8(str);
    
    num = typeinfo_count_method_args(desc,false);
    if (num) {
        typebuf = (u1*) DumpMemory::allocate(sizeof(u1) * num);
        infobuf = (typeinfo_t*) DumpMemory::allocate(sizeof(typeinfo_t) * num);
        
        typeinfo_init_from_method_args(desc,typebuf,infobuf,num,false,
                                       &returntype,info);

        TYPEINFO_ALLOCMERGED(info->merged,num);
        info->merged->count = num;

        for (i=0; i<num; ++i) {
            if (typebuf[i] != TYPE_ADR) {
                log_text("non-reference type in mergedlist");
				assert(0);
			}

            info->merged->list[i].any = infobuf[i].typeclass.any;
        }
        qsort(info->merged->list,num,sizeof(classref_or_classinfo),
              (int(*)(const void *,const void *))&typeinfo_test_compare);
    }
    else {
        typeinfo_init_from_method_args(desc,NULL,NULL,0,false,
                                       &returntype,info);
    }
}
#endif

#define TYPEINFO_TEST_BUFLEN  4000

static bool
typeinfo_equal(typeinfo_t *x,typeinfo_t *y)
{
    int i;
    
    if (x->typeclass.any != y->typeclass.any) return false;
    if (x->dimension != y->dimension) return false;
    if (x->dimension) {
        if (x->elementclass.any != y->elementclass.any) return false;
        if (x->elementtype != y->elementtype) return false;
    }

    if (x->is_newobject())
        if (x->newobject_instruction() != y->newobject_instruction())
            return false;

    if (x->merged || y->merged) {
        if (!(x->merged && y->merged)) return false;
        if (x->merged->count != y->merged->count) return false;
        for (i=0; i<x->merged->count; ++i)
            if (x->merged->list[i].any != y->merged->list[i].any)
                return false;
    }
    return true;
}

static void
typeinfo_testmerge(typeinfo_t *a,typeinfo_t *b,typeinfo_t *result,int *failed)
{
	typeinfo_t dest;

	typeinfo_t::clone(*a,dest);

    printf("\n          ");
    typeinfo_print_short(stdout,&dest);
    printf("\n          ");
    typeinfo_print_short(stdout,b);
    printf("\n");

	typecheck_result r = dest.merge(NULL, b);
	if (r == typecheck_FAIL) {
		printf("EXCEPTION\n");
		return;
	}

	bool changed           = (r) ? 1 : 0;
	bool changed_should_be = (!typeinfo_equal(&dest,a)) ? 1 : 0;

    printf("          %s\n",(changed) ? "changed" : "=");

    if (typeinfo_equal(&dest,result)) {
        printf("OK        ");
        typeinfo_print_short(stdout,&dest);
        printf("\n");
        if (changed != changed_should_be) {
            printf("WRONG RETURN VALUE!\n");
            (*failed)++;
        }
    }
    else {
        printf("RESULT    ");
        typeinfo_print_short(stdout,&dest);
        printf("\n");
        printf("SHOULD BE ");
        typeinfo_print_short(stdout,result);
        printf("\n");
        (*failed)++;
    }
}

#if 0
static void
typeinfo_inc_dimension(typeinfo_t *info)
{
    if (info->dimension++ == 0) {
        info->elementtype = ARRAYTYPE_OBJECT;
        info->elementclass = info->typeclass;
    }
    info->typeclass = class_array_of(info->typeclass,true);
}
#endif

#define TYPEINFO_TEST_MAXDIM  10

static void
typeinfo_testrun(const char *filename)
{
    char buf[TYPEINFO_TEST_BUFLEN];
    char bufa[TYPEINFO_TEST_BUFLEN];
    char bufb[TYPEINFO_TEST_BUFLEN];
    char bufc[TYPEINFO_TEST_BUFLEN];
    typeinfo_t a,b,c;
    int maxdim;
    int failed = 0;
    FILE *file = fopen(filename,"rt");
	int res;
    
    if (!file) {
        log_text("could not open typeinfo test file");
		assert(0);
	}

    while (fgets(buf,TYPEINFO_TEST_BUFLEN,file)) {
        if (buf[0] == '#' || !strlen(buf))
            continue;

        res = sscanf(buf,"%s\t%s\t%s\n",bufa,bufb,bufc);
        if (res != 3 || !strlen(bufa) || !strlen(bufb) || !strlen(bufc)) {
            log_text("Invalid line in typeinfo test file (none of empty, comment or test)");
			assert(0);
		}

#if 0
        typeinfo_test_parse(&a,bufa);
        typeinfo_test_parse(&b,bufb);
        typeinfo_test_parse(&c,bufc);
#endif
#if 0
        do {
#endif
            typeinfo_testmerge(&a,&b,&c,&failed); /* check result */
            typeinfo_testmerge(&b,&a,&c,&failed); /* check commutativity */

            if (a.is_nulltype()) break;
            if (b.is_nulltype()) break;
            if (c.is_nulltype()) break;

            maxdim = a.dimension;
            if (b.dimension > maxdim) maxdim = b.dimension;
            if (c.dimension > maxdim) maxdim = c.dimension;

#if 0
            if (maxdim < TYPEINFO_TEST_MAXDIM) {
                typeinfo_inc_dimension(&a);
                typeinfo_inc_dimension(&b);
                typeinfo_inc_dimension(&c);
            }
        } while (maxdim < TYPEINFO_TEST_MAXDIM);
#endif
    }

    fclose(file);

    if (failed) {
        fprintf(stderr,"Failed typeinfo_merge tests: %d\n",failed);
        log_text("Failed test");
		assert(0);
    }
}

void
typeinfo_test()
{
    log_text("Running typeinfo test file...");
    typeinfo_testrun("typeinfo.tst");
    log_text("Finished typeinfo test file.");
}

#if 0
void
typeinfo_init_from_fielddescriptor(typeinfo_t *info,char *desc)
{
    typeinfo_init_from_descriptor(info,desc,desc+strlen(desc));
}
#endif

#define TYPEINFO_MAXINDENT  80

void
typeinfo_print_class(FILE *file,classref_or_classinfo c)
{
	/*fprintf(file,"<class %p>",c.any);*/

	if (!c.any) {
		fprintf(file,"<null>");
	}
	else {
		if (c.is_classref()) {
			fprintf(file,"<ref>");
			utf_fprint_printable_ascii(file,c.ref->name);
		}
		else {
			utf_fprint_printable_ascii(file,c.cls->name);
		}
	}
}

void
typeinfo_print(FILE *file, const typeinfo_t *info, int indent)
{
	char ind[TYPEINFO_MAXINDENT + 1];

	if (indent > TYPEINFO_MAXINDENT)
		indent = TYPEINFO_MAXINDENT;

	for (int i = 0; i < indent; ++i)
		ind[i] = ' ';
	ind[indent] = (char) 0;

	if (info->is_primitive()) {
		if (basicblock *bptr = (basicblock*) info->returnaddress())
			fprintf(file,"%sreturnAddress (L%03d)\n",ind,bptr->nr);
		else
			fprintf(file,"%sprimitive\n",ind);
		return;
	}

    if (info->is_nulltype()) {
        fprintf(file,"%snull\n",ind);
        return;
    }

    if (info->is_newobject()) {
        if (instruction *ins = info->newobject_instruction()) {
            fprintf(file,"%sNEW(%p):",ind,(void*)ins);
			typeinfo_print_class(file,ins[-1].sx.val.c);
            fprintf(file,"\n");
        }
        else {
            fprintf(file,"%sNEW(this)",ind);
        }
        return;
    }

    fprintf(file,"%sClass:      ",ind);
	typeinfo_print_class(file,info->typeclass);
    fprintf(file,"\n");

    if (info->is_array()) {
        fprintf(file,"%sDimension:    %d",ind,(int)info->dimension);
        fprintf(file,"\n%sElements:     ",ind);
        switch (info->elementtype) {
          case ARRAYTYPE_INT     : fprintf(file,"int\n"); break;
          case ARRAYTYPE_LONG    : fprintf(file,"long\n"); break;
          case ARRAYTYPE_FLOAT   : fprintf(file,"float\n"); break;
          case ARRAYTYPE_DOUBLE  : fprintf(file,"double\n"); break;
          case ARRAYTYPE_BYTE    : fprintf(file,"byte\n"); break;
          case ARRAYTYPE_CHAR    : fprintf(file,"char\n"); break;
          case ARRAYTYPE_SHORT   : fprintf(file,"short\n"); break;
          case ARRAYTYPE_BOOLEAN : fprintf(file,"boolean\n"); break;
              
          case ARRAYTYPE_OBJECT:
			  typeinfo_print_class(file,info->elementclass);
              fprintf(file,"\n");
              break;
              
          default:
              fprintf(file,"INVALID ARRAYTYPE!\n");
        }
    }

    if (info->merged) {
        fprintf(file,"%sMerged:     ",ind);
        for (int i = 0; i < info->merged->count; ++i) {
            if (i) fprintf(file,", ");
			typeinfo_print_class(file,info->merged->list[i]);
        }
        fprintf(file,"\n");
    }
}

void
typeinfo_print_short(FILE *file, const typeinfo_t *info)
{
    int i;
    instruction *ins;
	basicblock *bptr;

	/*fprintf(file,"<typeinfo %p>",info);*/

	if (!info) {
		fprintf(file,"(typeinfo*)NULL");
		return;
	}

    if (info->is_primitive()) {
		bptr = (basicblock*) info->returnaddress();
		if (bptr)
			fprintf(file,"ret(L%03d)",bptr->nr);
		else
			fprintf(file,"primitive");
        return;
    }

    if (info->is_nulltype()) {
        fprintf(file,"null");
        return;
    }

    if (info->is_newobject()) {
        ins = (instruction *) info->newobject_instruction();
        if (ins) {
			/*fprintf(file,"<ins %p>",ins);*/
            fprintf(file,"NEW(%p):",(void*)ins);
			typeinfo_print_class(file,ins[-1].sx.val.c);
        }
        else
            fprintf(file,"NEW(this)");
        return;
    }

    typeinfo_print_class(file,info->typeclass);

    if (info->merged) {
        fprintf(file,"{");
        for (i=0; i<info->merged->count; ++i) {
            if (i) fprintf(file,",");
			typeinfo_print_class(file,info->merged->list[i]);
        }
        fprintf(file,"}");
    }
}

void
typeinfo_print_type(FILE *file, int type, const typeinfo_t *info)
{
    switch (type) {
      case TYPE_VOID: fprintf(file,"V"); break;
      case TYPE_INT:  fprintf(file,"I"); break;
      case TYPE_FLT:  fprintf(file,"F"); break;
      case TYPE_DBL:  fprintf(file,"D"); break;
      case TYPE_LNG:  fprintf(file,"J"); break;
	  case TYPE_RET:  fprintf(file,"R:"); /* FALLTHROUGH! */
      case TYPE_ADR:
		  typeinfo_print_short(file,info);
          break;
          
      default:
          fprintf(file,"!");
    }
}

void
typedescriptor_print(FILE *file, const typedescriptor_t *td)
{
	typeinfo_print_type(file,td->type, &(td->typeinfo));
}

void
typevector_print(FILE *file, const varinfo *vec, int size)
{
    int i;

    for (i=0; i<size; ++i) {
		fprintf(file," %d=",i);
        typeinfo_print_type(file, vec[i].type, &(vec[i].typeinfo));
    }
}

#endif /* TYPEINFO_DEBUG */


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
