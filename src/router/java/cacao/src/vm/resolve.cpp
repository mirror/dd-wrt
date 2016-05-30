/* src/vm/resolve.cpp - resolving classes/interfaces/fields/methods

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


#include "vm/resolve.hpp"
#include <assert.h>                     // for assert
#include <stdint.h>                     // for int32_t
#include "config.h"                     // for ENABLE_VERIFIER
#include "mm/dumpmemory.hpp"            // for DNEW
#include "mm/memory.hpp"
#include "toolbox/buffer.hpp"           // for Buffer
#include "vm/access.hpp"                // for access_is_accessible_member, etc
#include "vm/class.hpp"                 // for classinfo, etc
#include "vm/classcache.hpp"            // for classcache_add_constraint, etc
#include "vm/descriptor.hpp"            // for typedesc, methoddesc, etc
#include "vm/exceptions.hpp"
#include "vm/field.hpp"                 // for fieldinfo
#include "vm/global.hpp"                // for Type::TYPE_ADR, etc
#include "vm/globals.hpp"
#include "vm/jit/builtin.hpp"           // for builtin_instanceof
#include "vm/jit/ir/icmd.hpp"           // for ::ICMD_GETFIELD, etc
#include "vm/jit/ir/instruction.hpp"    // for instruction, etc
#include "vm/jit/jit.hpp"
#include "vm/jit/reg.hpp"               // for varinfo
#include "vm/jit/verify/typeinfo.hpp"   // for typeinfo_t, etc
#include "vm/linker.hpp"                // for link_class
#include "vm/loader.hpp"                // for load_class_from_classloader
#include "vm/method.hpp"                // for methodinfo
#include "vm/options.hpp"               // for opt_verify
#include "vm/primitive.hpp"             // for Primitive
#include "vm/types.hpp"                 // for s4

struct jitdata;


/******************************************************************************/
/* DEBUG HELPERS                                                              */
/******************************************************************************/

/*#define RESOLVE_VERBOSE*/

/* resolve_handle_pending_exception ********************************************

   Convert a pending ClassNotFoundException into a
   NoClassDefFoundError if requested.

   See: hotspot/src/share/vm/classfile/systemDictionary.cpp
   (handle_resolution_exception)

   ARGUMENTS:
       classname .... name of the class currently resolved
       throwError ... if true throw a NoClassDefFoundError instead of
                      a ClassNotFoundException

*******************************************************************************/

void resolve_handle_pending_exception(bool throwError)
{
	java_handle_t *e;

	/* Get the current exception. */

	e = exceptions_get_exception();

	if (e != NULL) {
		if (throwError == true) {
			/* Convert ClassNotFoundException to
			   NoClassDefFoundError. */

			if (builtin_instanceof(e, class_java_lang_ClassNotFoundException)) {
				/* Clear exception, because we are calling Java code
				   again. */

				exceptions_clear_exception();

				/* create new error */

				exceptions_throw_noclassdeffounderror_cause(e);
			}
			else {
				return;
			}
		}
		else {
			/* An exception conversion was not requested.  Simply
			   return. */

			return;
		}
	}
}


/******************************************************************************/
/* CLASS RESOLUTION                                                           */
/******************************************************************************/

/* resolve_class_from_name *****************************************************
 
   Resolve a symbolic class reference
  
   IN:
       referer..........the class containing the reference
       refmethod........the method from which resolution was triggered
                        (may be NULL if not applicable)
       classname........class name to resolve
       mode.............mode of resolution:
                            resolveLazy...only resolve if it does not
                                          require loading classes
                            resolveEager..load classes if necessary
	   checkaccess......if true, access rights to the class are checked
	   link.............if true, guarantee that the returned class, if any,
	                    has been linked
  
   OUT:
       *result..........set to result of resolution, or to NULL if
                        the reference has not been resolved
                        In the case of an exception, *result is
                        guaranteed to be set to NULL.
  
   RETURN VALUE:
       true.............everything ok 
                        (*result may still be NULL for resolveLazy)
       false............an exception has been thrown

   NOTE:
       The returned class is *not* guaranteed to be linked!
	   (It is guaranteed to be loaded, though.)
   
*******************************************************************************/

bool resolve_class_from_name(classinfo *referer,
							 methodinfo *refmethod,
							 Utf8String classname,
							 resolve_mode_t mode,
							 bool checkaccess,
							 bool link,
							 classinfo **result)
{
	classinfo  *cls;
	const char *utf_ptr;
	int         len;

	assert(result);
	assert(referer);
	assert(classname);
	assert(mode == resolveLazy || mode == resolveEager);
	
	*result = NULL;

#ifdef RESOLVE_VERBOSE
	printf("resolve_class_from_name(");
	utf_fprint_printable_ascii(stdout,referer->name);
	printf(",%p,",(void*)referer->classloader);
	utf_fprint_printable_ascii(stdout,classname);
	printf(",%d,%d)\n",(int)checkaccess,(int)link);
#endif

	/* lookup if this class has already been loaded */

	cls = classcache_lookup(referer->classloader, classname);

#ifdef RESOLVE_VERBOSE
	printf("    lookup result: %p\n",(void*)cls);
#endif

	if (!cls) {
		/* resolve array types */

		if (classname[0] == '[') {
			utf_ptr = classname.begin() + 1;
			len     = classname.size()  - 1;

			/* classname is an array type name */

			switch (*utf_ptr) {
				case 'L':
					utf_ptr++;
					len -= 2;
					/* FALLTHROUGH */
				case '[':
					/* the component type is a reference type */
					/* resolve the component type */
					if (!resolve_class_from_name(referer,refmethod,
					                             Utf8String::from_utf8(utf_ptr,len),
					                             mode,checkaccess,link,&cls))
						return false; /* exception */
					if (!cls) {
						assert(mode == resolveLazy);
						return true; /* be lazy */
					}
					/* create the array class */
					cls = class_array_of(cls,false);
					if (!cls)
						return false; /* exception */
			}
		}
		else {
			/* the class has not been loaded, yet */
			if (mode == resolveLazy)
				return true; /* be lazy */
		}

#ifdef RESOLVE_VERBOSE
		printf("    loading...\n");
#endif

		/* load the class */

		if (cls == NULL) {
			cls = load_class_from_classloader(classname, referer->classloader);

			if (cls == NULL)
				return false;
		}
	}

	/* the class is now loaded */
	assert(cls);
	assert(cls->state & CLASS_LOADED);

#ifdef RESOLVE_VERBOSE
	printf("    checking access rights...\n");
#endif
	
	/* check access rights of referer to refered class */

	if (checkaccess && !access_is_accessible_class(referer,cls)) {
		Buffer<> buf;

		Utf8String u = buf.write("class is not accessible (")
		                  .write_slash_to_dot(cls->name)
		                  .write(" from ")
		                  .write_slash_to_dot(referer->name)
		                  .write(")")
		                  .utf8_str();

		exceptions_throw_illegalaccessexception(u);
		return false; /* exception */
	}

	/* link the class if necessary */
	if (link) {
		if (!(cls->state & CLASS_LINKED))
			if (!link_class(cls))
				return false; /* exception */

		assert(cls->state & CLASS_LINKED);
	}

	/* resolution succeeds */
#ifdef RESOLVE_VERBOSE
	printf("    success.\n");
#endif
	*result = cls;
	return true;
}

/* resolve_classref ************************************************************
 
   Resolve a symbolic class reference
  
   IN:
       refmethod........the method from which resolution was triggered
                        (may be NULL if not applicable)
       ref..............class reference
       mode.............mode of resolution:
                            resolveLazy...only resolve if it does not
                                          require loading classes
                            resolveEager..load classes if necessary
	   checkaccess......if true, access rights to the class are checked
	   link.............if true, guarantee that the returned class, if any,
	                    has been linked
  
   OUT:
       *result..........set to result of resolution, or to NULL if
                        the reference has not been resolved
                        In the case of an exception, *result is
                        guaranteed to be set to NULL.
  
   RETURN VALUE:
       true.............everything ok 
                        (*result may still be NULL for resolveLazy)
       false............an exception has been thrown
   
*******************************************************************************/

bool resolve_classref(methodinfo *refmethod,
					  constant_classref *ref,
					  resolve_mode_t mode,
					  bool checkaccess,
					  bool link,
					  classinfo **result)
{
	return resolve_classref_or_classinfo(refmethod,to_classref_or_classinfo(ref),mode,checkaccess,link,result);
}

/* resolve_classref_or_classinfo ***********************************************
 
   Resolve a symbolic class reference if necessary

   NOTE: If given, refmethod->clazz is used as the referring class.
         Otherwise, cls.ref->referer is used.

   IN:
       refmethod........the method from which resolution was triggered
                        (may be NULL if not applicable)
       cls..............class reference or classinfo
       mode.............mode of resolution:
                            resolveLazy...only resolve if it does not
                                          require loading classes
                            resolveEager..load classes if necessary
	   checkaccess......if true, access rights to the class are checked
	   link.............if true, guarantee that the returned class, if any,
	                    has been linked
  
   OUT:
       *result..........set to result of resolution, or to NULL if
                        the reference has not been resolved
                        In the case of an exception, *result is
                        guaranteed to be set to NULL.
  
   RETURN VALUE:
       true.............everything ok 
                        (*result may still be NULL for resolveLazy)
       false............an exception has been thrown

*******************************************************************************/

bool resolve_classref_or_classinfo(methodinfo *refmethod,
								   classref_or_classinfo cls,
								   resolve_mode_t mode,
								   bool checkaccess,
								   bool link,
								   classinfo **result)
{
	classinfo         *c;
	classinfo         *referer;

	assert(cls.any);
	assert(mode == resolveEager || mode == resolveLazy);
	assert(result);

#ifdef RESOLVE_VERBOSE
	printf("resolve_classref_or_classinfo(");
	utf_fprint_printable_ascii(stdout, CLASSREF_OR_CLASSINFO_NAME(cls));
	printf(",%i,%i,%i)\n",mode,(int)checkaccess,(int)link);
#endif

	*result = NULL;

	if (cls.is_classref()) {
		/* we must resolve this reference */

		/* determine which class to use as the referer */

		/* Common cases are refmethod == NULL or both referring classes */
		/* being the same, so the referer usually is cls.ref->referer.    */
		/* There is one important case where it is not: When we do a      */
		/* deferred assignability check to a formal argument of a method, */
		/* we must use refmethod->clazz (the caller's class) to resolve   */
		/* the type of the formal argument.                               */

		referer = (refmethod) ? refmethod->clazz : cls.ref->referer;

		if (!resolve_class_from_name(referer, refmethod, cls.ref->name,
									 mode, checkaccess, link, &c))
			goto return_exception;

	} else {
		/* cls has already been resolved */
		c = cls.cls;
		assert(c->state & CLASS_LOADED);
	}
	assert(c || (mode == resolveLazy));

	if (!c)
		return true; /* be lazy */
	
	assert(c);
	assert(c->state & CLASS_LOADED);

	if (link) {
		if (!(c->state & CLASS_LINKED))
			if (!link_class(c))
				goto return_exception;

		assert(c->state & CLASS_LINKED);
	}

	/* succeeded */
	*result = c;
	return true;

 return_exception:
	*result = NULL;
	return false;
}


/* resolve_classref_or_classinfo_eager *****************************************
 
   Resolve a symbolic class reference eagerly if necessary.
   No attempt is made to link the class.

   IN:
       cls..............class reference or classinfo
       checkaccess......if true, access rights to the class are checked
  
   RETURN VALUE:
       classinfo *......the resolved class
       NULL.............an exception has been thrown
   
*******************************************************************************/

classinfo *resolve_classref_or_classinfo_eager(classref_or_classinfo cls,
											   bool checkaccess)
{
	classinfo *c;

	if (!resolve_classref_or_classinfo(NULL, cls, resolveEager, checkaccess, false, &c))
		return NULL;

	return c;
}


/* resolve_class_from_typedesc *************************************************
 
   Return a classinfo * for the given type descriptor
  
   IN:
       d................type descriptor
	   checkaccess......if true, access rights to the class are checked
	   link.............if true, guarantee that the returned class, if any,
	                    has been linked
   OUT:
       *result..........set to result of resolution, or to NULL if
                        the reference has not been resolved
                        In the case of an exception, *result is
                        guaranteed to be set to NULL.
  
   RETURN VALUE:
       true.............everything ok 
       false............an exception has been thrown

   NOTE:
       This function always resolves eagerly.
   
*******************************************************************************/

bool resolve_class_from_typedesc(typedesc *d, bool checkaccess, bool link, classinfo **result)
{
	classinfo *cls;
	
	assert(d);
	assert(result);

	*result = NULL;

#ifdef RESOLVE_VERBOSE
	printf("resolve_class_from_typedesc(");
	descriptor_debug_print_typedesc(stdout,d);
	printf(",%i,%i)\n",(int)checkaccess,(int)link);
#endif

	if (d->type == TYPE_ADR) {
		/* a reference type */
		assert(d->classref);
		if (!resolve_classref_or_classinfo(NULL,to_classref_or_classinfo(d->classref),
										   resolveEager,checkaccess,link,&cls))
			return false; /* exception */
	}
	else {
		/* a primitive type */

		cls = Primitive::get_class_by_type(d->primitivetype);

		assert(cls->state & CLASS_LOADED);

		if (!(cls->state & CLASS_LINKED))
			if (!link_class(cls))
				return false; /* exception */
	}

	assert(cls);
	assert(cls->state & CLASS_LOADED);
	assert(!link || (cls->state & CLASS_LINKED));

#ifdef RESOLVE_VERBOSE
	printf("    result = ");utf_fprint_printable_ascii(stdout,cls->name);printf("\n");
#endif

	*result = cls;
	return true;
}

/******************************************************************************/
/* SUBTYPE SET CHECKS                                                         */
/******************************************************************************/

/* resolve_subtype_check *******************************************************
 
   Resolve the given types lazily and perform a subtype check
  
   IN:
       refmethod........the method triggering the resolution
       subtype..........checked to be a subtype of supertype
	   supertype........the super type to check agaings
	   mode.............mode of resolution:
                            resolveLazy...only resolve if it does not
                                          require loading classes
                            resolveEager..load classes if necessary
       error............which type of exception to throw if
                        the test fails. May be:
                            resolveLinkageError, or
                            resolveIllegalAccessError
						IMPORTANT: If error==resolveIllegalAccessError,
						then array types are not checked.

   RETURN VALUE:
       resolveSucceeded.....the check succeeded
       resolveDeferred......the check could not be performed due to
	                        unresolved types. (This can only happen for
							mode == resolveLazy.)
	   resolveFailed........the check failed, an exception has been thrown.
   
   NOTE:
	   The types are resolved first, so any
	   exception which may occurr during resolution may
	   be thrown by this function.
   
*******************************************************************************/

#if defined(ENABLE_VERIFIER)
static resolve_result_t resolve_subtype_check(methodinfo *refmethod,
										      classref_or_classinfo subtype,
											  classref_or_classinfo supertype,
											  resolve_mode_t mode,
											  resolve_err_t error)
{
	classinfo        *subclass;
	typeinfo_t        subti;
	typecheck_result  r;

	assert(refmethod);
	assert(subtype.any);
	assert(supertype.any);
	assert(mode == resolveLazy || mode == resolveEager);
	assert(error == resolveLinkageError || error == resolveIllegalAccessError);

	/* resolve the subtype */

	if (!resolve_classref_or_classinfo(refmethod,subtype,mode,false,true,&subclass)) {
		/* the subclass could not be resolved. therefore we are sure that  */
		/* no instances of this subclass will ever exist -> skip this test */
		/* XXX this assumes that class loading has invariant results (as in JVM spec) */
		exceptions_clear_exception();
		return resolveSucceeded;
	}
	if (!subclass)
		return resolveDeferred; /* be lazy */

	assert(subclass->state & CLASS_LINKED);

	/* do not check access to protected members of arrays */

	if (error == resolveIllegalAccessError && subclass->name[0] == '[') {
		return resolveSucceeded;
	}

	/* perform the subtype check */

	subti.init_class(subclass);
check_again:
	r = subti.is_assignable_to_class(supertype);
	if (r == typecheck_FAIL)
		return resolveFailed; /* failed, exception is already set */

	if (r == typecheck_MAYBE) {
		assert(supertype.is_classref());
		if (mode == resolveEager) {
			if (!resolve_classref_or_classinfo(refmethod,supertype,
											   resolveEager,false,true,
											   &supertype.cls))
			{
				return resolveFailed;
			}
			assert(supertype.cls);
			goto check_again;
		}

		return resolveDeferred; /* be lazy */
	}

	if (!r) {
		/* sub class relationship is false */

#if defined(RESOLVE_VERBOSE)
		printf("SUBTYPE CHECK FAILED!\n");
#endif
		Buffer<> buf;

		if (error == resolveIllegalAccessError)
			buf.write("illegal access to protected member (");
		else
			buf.write("subtype constraint violated (");
	
		buf.write_slash_to_dot(subclass->name)
		   .write(" is not a subclass of ")
		   .write_slash_to_dot(CLASSREF_OR_CLASSINFO_NAME(supertype))
		   .write(')');

		if (error == resolveIllegalAccessError)
			exceptions_throw_illegalaccessexception(buf.utf8_str());
		else
			exceptions_throw_linkageerror(buf.c_str(), NULL);

		return resolveFailed; /* exception */
	}

	/* everything ok */

	return resolveSucceeded;
}
#endif /* defined(ENABLE_VERIFIER) */

/* resolve_lazy_subtype_checks *************************************************
 
   Resolve the types to check lazily and perform subtype checks
  
   IN:
       refmethod........the method triggering the resolution
       subtinfo.........the typeinfo containing the subtypes
       supertype........the supertype to test againgst
	   mode.............mode of resolution:
                            resolveLazy...only resolve if it does not
                                          require loading classes
                            resolveEager..load classes if necessary
       error............which type of exception to throw if
                        the test fails. May be:
                            resolveLinkageError, or
                            resolveIllegalAccessError
						IMPORTANT: If error==resolveIllegalAccessError,
						then array types in the set are skipped.

   RETURN VALUE:
       resolveSucceeded.....the check succeeded
       resolveDeferred......the check could not be performed due to
	                        unresolved types
	   resolveFailed........the check failed, an exception has been thrown.
   
   NOTE:
       The references in the set are resolved first, so any
       exception which may occurr during resolution may
       be thrown by this function.
   
*******************************************************************************/

#if defined(ENABLE_VERIFIER)
static resolve_result_t resolve_lazy_subtype_checks(methodinfo *refmethod,
													typeinfo_t *subtinfo,
													classref_or_classinfo supertype,
													resolve_err_t error)
{
	int count;
	int i;
	resolve_result_t result;

	assert(refmethod);
	assert(subtinfo);
	assert(supertype.any);
	assert(error == resolveLinkageError || error == resolveIllegalAccessError);

	/* returnAddresses are illegal here */

	if (subtinfo->is_primitive()) {
		exceptions_throw_verifyerror(refmethod,
				"Invalid use of returnAddress");
		return resolveFailed;
	}

	/* uninitialized objects are illegal here */

	if (subtinfo->is_primitive()) {
		exceptions_throw_verifyerror(refmethod,
				"Invalid use of uninitialized object");
		return resolveFailed;
	}

	/* the nulltype is always assignable */

	if (subtinfo->is_nulltype())
		return resolveSucceeded;

	/* every type is assignable to (BOOTSTRAP)java.lang.Object */

	if (supertype.cls == class_java_lang_Object
		|| (CLASSREF_OR_CLASSINFO_NAME(supertype) == utf8::java_lang_Object
			&& refmethod->clazz->classloader == NULL))
	{
		return resolveSucceeded;
	}

	if (subtinfo->merged) {

		/* for a merged type we have to do a series of checks */

		count = subtinfo->merged->count;
		for (i=0; i<count; ++i) {
			classref_or_classinfo c = subtinfo->merged->list[i];
			if (subtinfo->dimension > 0) {
				/* a merge of array types */
				/* the merged list contains the possible _element_ types, */
				/* so we have to create array types with these elements.  */
				if (c.is_classref()) {
					c.ref = class_get_classref_multiarray_of(subtinfo->dimension,c.ref);
				}
				else {
					c.cls = class_multiarray_of(subtinfo->dimension,c.cls,false);
				}
			}

			/* do the subtype check against the type c */

			result = resolve_subtype_check(refmethod,c,supertype,resolveLazy,error);
			if (result != resolveSucceeded)
				return result;
		}
	}
	else {

		/* a single type, this is the common case, hopefully */

		if (CLASSREF_OR_CLASSINFO_NAME(subtinfo->typeclass)
			== CLASSREF_OR_CLASSINFO_NAME(supertype))
		{
			/* the class names are the same */
		    /* equality is guaranteed by the loading constraints */
			return resolveSucceeded;
		}
		else {

			/* some other type name, try to perform the check lazily */

			return resolve_subtype_check(refmethod,
										 subtinfo->typeclass,supertype,
										 resolveLazy,
										 error);
		}
	}

	/* everything ok */
	return resolveSucceeded;
}
#endif /* defined(ENABLE_VERIFIER) */

/* resolve_and_check_subtype_set ***********************************************
 
   Resolve the references in the given set and test subtype relationships
  
   IN:
       refmethod........the method triggering the resolution
       ref..............a set of class/interface references
                        (may be empty)
       typeref..........the type to test against the set
       mode.............mode of resolution:
                            resolveLazy...only resolve if it does not
                                          require loading classes
                            resolveEager..load classes if necessary
       error............which type of exception to throw if
                        the test fails. May be:
                            resolveLinkageError, or
                            resolveIllegalAccessError
						IMPORTANT: If error==resolveIllegalAccessError,
						then array types in the set are skipped.

   RETURN VALUE:
       resolveSucceeded.....the check succeeded
       resolveDeferred......the check could not be performed due to
	                        unresolved types. (This can only happen if
							mode == resolveLazy.)
	   resolveFailed........the check failed, an exception has been thrown.
   
   NOTE:
       The references in the set are resolved first, so any
       exception which may occurr during resolution may
       be thrown by this function.
   
*******************************************************************************/

#if defined(ENABLE_VERIFIER)
static resolve_result_t resolve_and_check_subtype_set(methodinfo *refmethod,
								          unresolved_subtype_set *ref,
								          classref_or_classinfo typeref,
								          resolve_mode_t mode,
								          resolve_err_t error)
{
	classref_or_classinfo *setp;
	resolve_result_t checkresult;

	assert(refmethod);
	assert(ref);
	assert(typeref.any);
	assert(mode == resolveLazy || mode == resolveEager);
	assert(error == resolveLinkageError || error == resolveIllegalAccessError);

#if defined(RESOLVE_VERBOSE)
	printf("resolve_and_check_subtype_set:\n");
	unresolved_subtype_set_debug_dump(ref, stdout);
	if (typeref.is_classref())
		class_classref_println(typeref.ref);
	else
		class_println(typeref.cls);
#endif

	setp = ref->subtyperefs;

	/* an empty set of tests always succeeds */
	if (!setp || !setp->any) {
		return resolveSucceeded;
	}

	/* first resolve the type if necessary */
	if (!resolve_classref_or_classinfo(refmethod,typeref,mode,false,true,&(typeref.cls)))
		return resolveFailed; /* exception */
	if (!typeref.cls)
		return resolveDeferred; /* be lazy */

	assert(typeref.cls->state & CLASS_LINKED);

	/* iterate over the set members */

	for (; setp->any; ++setp) {
		checkresult = resolve_subtype_check(refmethod,*setp,typeref,mode,error);
#if defined(RESOLVE_VERBOSE)
		if (checkresult != resolveSucceeded)
			printf("SUBTYPE CHECK FAILED!\n");
#endif
		if (checkresult != resolveSucceeded)
			return checkresult;
	}

	/* check succeeds */
	return resolveSucceeded;
}
#endif /* defined(ENABLE_VERIFIER) */

/******************************************************************************/
/* CLASS RESOLUTION                                                           */
/******************************************************************************/

/* resolve_class ***************************************************************
 
   Resolve an unresolved class reference. The class is also linked.
  
   IN:
       ref..............struct containing the reference
       mode.............mode of resolution:
                            resolveLazy...only resolve if it does not
                                          require loading classes
                            resolveEager..load classes if necessary
	   checkaccess......if true, access rights to the class are checked
   
   OUT:
       *result..........set to the result of resolution, or to NULL if
                        the reference has not been resolved
                        In the case of an exception, *result is
                        guaranteed to be set to NULL.
  
   RETURN VALUE:
       true.............everything ok 
                        (*result may still be NULL for resolveLazy)
       false............an exception has been thrown
   
*******************************************************************************/

#ifdef ENABLE_VERIFIER
bool resolve_class(unresolved_class *ref,
				   resolve_mode_t mode,
				   bool checkaccess,
				   classinfo **result)
{
	classinfo *cls;
	resolve_result_t checkresult;
	
	assert(ref);
	assert(result);
	assert(mode == resolveLazy || mode == resolveEager);

	*result = NULL;

#ifdef RESOLVE_VERBOSE
	unresolved_class_debug_dump(ref,stdout);
#endif

	/* first we must resolve the class */
	if (!resolve_classref(ref->referermethod,
					      ref->classref,mode,checkaccess,true,&cls))
	{
		/* the class reference could not be resolved */
		return false; /* exception */
	}
	if (!cls)
		return true; /* be lazy */

	assert(cls);
	assert((cls->state & CLASS_LOADED) && (cls->state & CLASS_LINKED));

	/* now we check the subtype constraints */
	
	checkresult = resolve_and_check_subtype_set(ref->referermethod,
									   &(ref->subtypeconstraints),
									   to_classref_or_classinfo(cls),
									   mode,
									   resolveLinkageError);
	if (checkresult != resolveSucceeded)
		return (bool) checkresult;

	/* succeed */
	*result = cls;
	return true;
}
#endif /* ENABLE_VERIFIER */

/* resolve_classref_eager ******************************************************
 
   Resolve an unresolved class reference eagerly. The class is also linked and
   access rights to the class are checked.
  
   IN:
       ref..............constant_classref to the class
   
   RETURN VALUE:
       classinfo * to the class, or
	   NULL if an exception has been thrown
   
*******************************************************************************/

classinfo * resolve_classref_eager(constant_classref *ref)
{
	classinfo *c;

	if (!resolve_classref(NULL,ref,resolveEager,true,true,&c))
		return NULL;

	return c;
}

/* resolve_classref_eager_nonabstract ******************************************
 
   Resolve an unresolved class reference eagerly. The class is also linked and
   access rights to the class are checked. A check is performed that the class
   is not abstract.
  
   IN:
       ref..............constant_classref to the class
   
   RETURN VALUE:
       classinfo * to the class, or
	   NULL if an exception has been thrown
   
*******************************************************************************/

classinfo * resolve_classref_eager_nonabstract(constant_classref *ref)
{
	classinfo *c;

	if (!resolve_classref(NULL,ref,resolveEager,true,true,&c))
		return NULL;

	/* ensure that the class is not abstract */

	if (c->flags & ACC_ABSTRACT) {
		exceptions_throw_verifyerror(NULL,"creating instance of abstract class");
		return NULL;
	}

	return c;
}

/* resolve_class_eager *********************************************************
 
   Resolve an unresolved class reference eagerly. The class is also linked and
   access rights to the class are checked.
  
   IN:
       ref..............struct containing the reference
   
   RETURN VALUE:
       classinfo * to the class, or
	   NULL if an exception has been thrown
   
*******************************************************************************/

#ifdef ENABLE_VERIFIER
classinfo * resolve_class_eager(unresolved_class *ref)
{
	classinfo *c;

	if (!resolve_class(ref,resolveEager,true,&c))
		return NULL;

	return c;
}
#endif /* ENABLE_VERIFIER */

/* resolve_class_eager_no_access_check *****************************************
 
   Resolve an unresolved class reference eagerly. The class is also linked.
   Access rights are _not_ checked.
  
   IN:
       ref..............struct containing the reference
   
   RETURN VALUE:
       classinfo * to the class, or
	   NULL if an exception has been thrown
   
*******************************************************************************/

#ifdef ENABLE_VERIFIER
classinfo * resolve_class_eager_no_access_check(unresolved_class *ref)
{
	classinfo *c;

	if (!resolve_class(ref, resolveEager, false, &c))
		return NULL;

	return c;
}
#endif /* ENABLE_VERIFIER */

/******************************************************************************/
/* FIELD RESOLUTION                                                           */
/******************************************************************************/

/* resolve_field_verifier_checks *******************************************
 
   Do the verifier checks necessary after field has been resolved.
  
   IN:
       refmethod........the method containing the reference
	   fieldref.........the field reference
	   container........the class where the field was found
	   fi...............the fieldinfo of the resolved field
	   instanceti.......instance typeinfo, if available
	   valueti..........value typeinfo, if available
	   isstatic.........true if this is a *STATIC* instruction
	   isput............true if this is a PUT* instruction
  
   RETURN VALUE:
       resolveSucceeded....everything ok
	   resolveDeferred.....tests could not be done, have been deferred
       resolveFailed.......exception has been thrown
   
*******************************************************************************/

#if defined(ENABLE_VERIFIER)
resolve_result_t resolve_field_verifier_checks(methodinfo *refmethod,
											   constant_FMIref *fieldref,
											   classinfo *container,
											   fieldinfo *fi,
											   typeinfo_t *instanceti,
											   typeinfo_t *valueti,
											   bool isstatic,
											   bool isput)
{
	classinfo         *declarer;
	classinfo         *referer;
	resolve_result_t   result;
	constant_classref *fieldtyperef;

	assert(refmethod);
	assert(fieldref);
	assert(container);
	assert(fi);

	/* get the classinfos and the field type */

	referer = refmethod->clazz;
	assert(referer);

	declarer = fi->clazz;
	assert(declarer);
	assert(referer->state & CLASS_LINKED);

	fieldtyperef = fieldref->parseddesc.fd->classref;

	/* check static */

#if true != 1
#error This code assumes that `true` is `1`. Otherwise, use the ternary operator below.
#endif

	if (((fi->flags & ACC_STATIC) != 0) != isstatic) {
		/* a static field is accessed via an instance, or vice versa */
		exceptions_throw_incompatibleclasschangeerror(declarer,
													  (fi->flags & ACC_STATIC)
													  ? "static field accessed via instance"
													  : "instance field  accessed without instance");

		return resolveFailed;
	}

	/* check access rights */

	if (!access_is_accessible_member(referer,declarer,fi->flags)) {
		Buffer<> buf(64);

		Utf8String u = buf.write("field is not accessible (")
		                  .write_slash_to_dot(declarer->name)
		                  .write(".")
		                  .write(fi->name)
		                  .write(" from ")
		                  .write_slash_to_dot(referer->name)
		                  .write(")")
		                  .utf8_str();

		exceptions_throw_illegalaccessexception(u);

		return resolveFailed; /* exception */
	}

	/* for non-static methods we have to check the constraints on the         */
	/* instance type                                                          */

	if (instanceti) {
		typeinfo_t *insttip;
		typeinfo_t tinfo;

		/* The instanceslot must contain a reference to a non-array type */

		if (!instanceti->is_reference()) {
			exceptions_throw_verifyerror(refmethod, "illegal instruction: field access on non-reference");
			return resolveFailed;
		}
		if (instanceti->is_array()) {
			exceptions_throw_verifyerror(refmethod, "illegal instruction: field access on array");
			return resolveFailed;
		}

		if (isput && instanceti->is_newobject())
		{
			/* The instruction writes a field in an uninitialized object. */
			/* This is only allowed when a field of an uninitialized 'this' object is */
			/* written inside an initialization method                                */

			classinfo *initclass;
			instruction *ins = instanceti->newobject_instruction();

			if (ins != NULL) {
				exceptions_throw_verifyerror(refmethod, "accessing field of uninitialized object");
				return resolveFailed;
			}

			/* XXX check that class of field == refmethod->clazz */
			initclass = referer; /* XXX classrefs */
			assert(initclass->state & CLASS_LINKED);

			tinfo.init_class(initclass);
			insttip = &tinfo;
		}
		else {
			insttip = instanceti;
		}

		result = resolve_lazy_subtype_checks(refmethod,
				insttip,
				to_classref_or_classinfo(container),
				resolveLinkageError);
		if (result != resolveSucceeded)
			return result;

		/* check protected access */

		if (((fi->flags & ACC_PROTECTED) != 0) && !SAME_PACKAGE(declarer,referer))
		{
			result = resolve_lazy_subtype_checks(refmethod,
					instanceti,
					to_classref_or_classinfo(referer),
					resolveIllegalAccessError);
			if (result != resolveSucceeded)
				return result;
		}

	}

	/* for PUT* instructions we have to check the constraints on the value type */

	if (valueti) {
		assert(fieldtyperef);

		/* check subtype constraints */
		result = resolve_lazy_subtype_checks(refmethod,
				valueti,
				to_classref_or_classinfo(fieldtyperef),
				resolveLinkageError);

		if (result != resolveSucceeded)
			return result;
	}

	/* impose loading constraint on field type */

	if (fi->type == TYPE_ADR) {
		assert(fieldtyperef);
		if (!classcache_add_constraint(declarer->classloader,
									   referer->classloader,
									   fieldtyperef->name))
			return resolveFailed;
	}

	/* XXX impose loading constraint on instance? */

	/* everything ok */
	return resolveSucceeded;
}
#endif /* defined(ENABLE_VERIFIER) */

/* resolve_field_lazy **********************************************************
 
   Resolve an unresolved field reference lazily

   NOTE: This function does NOT do any verification checks. In case of a
         successful resolution, you must call resolve_field_verifier_checks
		 in order to perform the necessary checks!
  
   IN:
	   refmethod........the referer method
	   fieldref.........the field reference
  
   RETURN VALUE:
       resolveSucceeded.....the reference has been resolved
       resolveDeferred......the resolving could not be performed lazily
	   resolveFailed........resolving failed, an exception has been thrown.

*******************************************************************************/

resolve_result_t resolve_field_lazy(methodinfo *refmethod,
									constant_FMIref *fieldref)
{
	classinfo *referer;
	classinfo *container;
	fieldinfo *fi;

	assert(refmethod);

	/* the class containing the reference */

	referer = refmethod->clazz;
	assert(referer);

	/* check if the field itself is already resolved */

	if (fieldref->is_resolved())
		return resolveSucceeded;

	/* first we must resolve the class containg the field */

	/* XXX can/may lazyResolving trigger linking? */

	if (!resolve_class_from_name(referer, refmethod,
		   fieldref->p.classref->name, resolveLazy, true, true, &container))
	{
		/* the class reference could not be resolved */
		return resolveFailed; /* exception */
	}
	if (!container)
		return resolveDeferred; /* be lazy */

	assert(container->state & CLASS_LINKED);

	/* now we must find the declaration of the field in `container`
	 * or one of its superclasses */

	fi = class_resolvefield(container,
							fieldref->name, fieldref->descriptor,
							referer);
	if (!fi) {
		/* The field does not exist. But since we were called lazily, */
		/* this error must not be reported now. (It will be reported   */
		/* if eager resolving of this field is ever tried.)           */

		exceptions_clear_exception();
		return resolveDeferred; /* be lazy */
	}

	/* cache the result of the resolution */

	fieldref->p.field = fi;

	/* everything ok */
	return resolveSucceeded;
}

/* resolve_field ***************************************************************
 
   Resolve an unresolved field reference
  
   IN:
       ref..............struct containing the reference
       mode.............mode of resolution:
                            resolveLazy...only resolve if it does not
                                          require loading classes
                            resolveEager..load classes if necessary
  
   OUT:
       *result..........set to the result of resolution, or to NULL if
                        the reference has not been resolved
                        In the case of an exception, *result is
                        guaranteed to be set to NULL.
  
   RETURN VALUE:
       true.............everything ok 
                        (*result may still be NULL for resolveLazy)
       false............an exception has been thrown
   
*******************************************************************************/

bool resolve_field(unresolved_field *ref,
				   resolve_mode_t mode,
				   fieldinfo **result)
{
	classinfo *referer;
	classinfo *container;
	fieldinfo *fi;

	assert(ref);
	assert(result);
	assert(mode == resolveLazy || mode == resolveEager);

	*result = NULL;

#ifdef RESOLVE_VERBOSE
	unresolved_field_debug_dump(ref,stdout);
#endif

	/* the class containing the reference */

	referer = ref->referermethod->clazz;
	assert(referer);

	/* check if the field itself is already resolved */
	if (ref->fieldref->is_resolved()) {
		fi = ref->fieldref->p.field;
		container = fi->clazz;
		goto resolved_the_field;
	}

	/* first we must resolve the class containg the field */
	if (!resolve_class_from_name(referer,ref->referermethod,
					   ref->fieldref->p.classref->name,mode,true,true,&container))
	{
		/* the class reference could not be resolved */
		return false; /* exception */
	}
	if (!container)
		return true; /* be lazy */

	assert(container);
	assert(container->state & CLASS_LOADED);
	assert(container->state & CLASS_LINKED);

	/* now we must find the declaration of the field in `container`
	 * or one of its superclasses */

#ifdef RESOLVE_VERBOSE
		printf("    resolving field in class...\n");
#endif

	fi = class_resolvefield(container,
							ref->fieldref->name,ref->fieldref->descriptor,
							referer);
	if (!fi) {
		if (mode == resolveLazy) {
			/* The field does not exist. But since we were called lazily, */
			/* this error must not be reported now. (It will be reported   */
			/* if eager resolving of this field is ever tried.)           */

			exceptions_clear_exception();
			return true; /* be lazy */
		}

		return false; /* exception */
	}

	/* cache the result of the resolution */
	ref->fieldref->p.field = fi;

resolved_the_field:

#ifdef ENABLE_VERIFIER
	/* Checking opt_verify is ok here, because the NULL iptr guarantees */
	/* that no missing parts of an instruction will be accessed.        */
	if (opt_verify) {
		resolve_result_t checkresult = resolve_field_verifier_checks(
				ref->referermethod,
				ref->fieldref,
				container,
				fi,
				NULL, /* instanceti, handled by constraints below */
				NULL, /* valueti, handled by constraints below  */
				(ref->flags & RESOLVE_STATIC) != 0, /* isstatic */
				(ref->flags & RESOLVE_PUTFIELD) != 0 /* isput */);

		if (checkresult != resolveSucceeded)
			return (bool) checkresult;

		classinfo *declarer = fi->clazz;
		assert(declarer);
		assert(declarer->state & CLASS_LOADED);
		assert(declarer->state & CLASS_LINKED);

		/* for non-static accesses we have to check the constraints on the */
		/* instance type */

		if (!(ref->flags & RESOLVE_STATIC)) {
			checkresult = resolve_and_check_subtype_set(ref->referermethod,
					&(ref->instancetypes),
					to_classref_or_classinfo(container),
					mode, resolveLinkageError);
			if (checkresult != resolveSucceeded)
				return (bool) checkresult;
		}

		constant_classref *fieldtyperef = ref->fieldref->parseddesc.fd->classref;

		/* for PUT* instructions we have to check the constraints on the value type */
		if (((ref->flags & RESOLVE_PUTFIELD) != 0) && fi->type == TYPE_ADR) {
			assert(fieldtyperef);
			if (!SUBTYPESET_IS_EMPTY(ref->valueconstraints)) {
				/* check subtype constraints */
				checkresult = resolve_and_check_subtype_set(ref->referermethod,
						&(ref->valueconstraints),
						to_classref_or_classinfo(fieldtyperef),
						mode, resolveLinkageError);
				if (checkresult != resolveSucceeded)
					return (bool) checkresult;
			}
		}

		/* check protected access */
		if (((fi->flags & ACC_PROTECTED) != 0) && !SAME_PACKAGE(declarer,referer)) {
			checkresult = resolve_and_check_subtype_set(ref->referermethod,
					&(ref->instancetypes),
					to_classref_or_classinfo(referer),
					mode,
					resolveIllegalAccessError);
			if (checkresult != resolveSucceeded)
				return (bool) checkresult;
		}

	}
#endif /* ENABLE_VERIFIER */

	/* succeed */
	*result = fi;

	return true;
}

/* resolve_field_eager *********************************************************
 
   Resolve an unresolved field reference eagerly.
  
   IN:
       ref..............struct containing the reference
   
   RETURN VALUE:
       fieldinfo * to the field, or
	   NULL if an exception has been thrown
   
*******************************************************************************/

fieldinfo * resolve_field_eager(unresolved_field *ref)
{
	fieldinfo *fi;

	if (!resolve_field(ref,resolveEager,&fi))
		return NULL;

	return fi;
}

/******************************************************************************/
/* METHOD RESOLUTION                                                          */
/******************************************************************************/

/* resolve_method_invokespecial_lookup *****************************************
 
   Do the special lookup for methods invoked by INVOKESPECIAL
  
   IN:
       refmethod........the method containing the reference
	   mi...............the methodinfo of the resolved method
  
   RETURN VALUE:
       a methodinfo *...the result of the lookup,
	   NULL.............an exception has been thrown
   
*******************************************************************************/

methodinfo * resolve_method_invokespecial_lookup(methodinfo *refmethod,
												 methodinfo *mi)
{
	classinfo *declarer;
	classinfo *referer;

	assert(refmethod);
	assert(mi);

	/* get referer and declarer classes */

	referer = refmethod->clazz;
	assert(referer);

	declarer = mi->clazz;
	assert(declarer);
	assert(referer->state & CLASS_LINKED);

	/* checks for INVOKESPECIAL:                                       */
	/* for <init> and methods of the current class we don't need any   */
	/* special checks. Otherwise we must verify that the called method */
	/* belongs to a super class of the current class                   */

	if ((referer != declarer) && (mi->name != utf8::init)) {
		/* check that declarer is a super class of the current class   */

		if (!class_issubclass(referer,declarer)) {
			exceptions_throw_verifyerror(refmethod,
					"INVOKESPECIAL calling non-super class method");
			return NULL;
		}

		/* if the referer has ACC_SUPER set, we must do the special    */
		/* lookup starting with the direct super class of referer      */

		if ((referer->flags & ACC_SUPER) != 0) {
			mi = class_resolvemethod(referer->super,
									 mi->name,
									 mi->descriptor);

			if (mi == NULL) {
				/* the spec calls for an AbstractMethodError in this case */

				exceptions_throw_abstractmethoderror();

				return NULL;
			}
		}
	}

	/* everything ok */
	return mi;
}

/* resolve_method_verifier_checks ******************************************
 
   Do the verifier checks necessary after a method has been resolved.
  
   IN:
       refmethod........the method containing the reference
	   methodref........the method reference
	   mi...............the methodinfo of the resolved method
	   invokestatic.....true if the method is invoked by INVOKESTATIC
  
   RETURN VALUE:
       resolveSucceeded....everything ok
	   resolveDeferred.....tests could not be done, have been deferred
       resolveFailed.......exception has been thrown
   
*******************************************************************************/

#if defined(ENABLE_VERIFIER)
resolve_result_t resolve_method_verifier_checks(methodinfo *refmethod,
												constant_FMIref *methodref,
												methodinfo *mi,
												bool invokestatic)
{
	classinfo *declarer;
	classinfo *referer;

	assert(refmethod);
	assert(methodref);
	assert(mi);

#ifdef RESOLVE_VERBOSE
	printf("resolve_method_verifier_checks\n");
	printf("    flags: %02x\n",mi->flags);
#endif

	/* get the classinfos and the method descriptor */

	referer = refmethod->clazz;
	assert(referer);

	declarer = mi->clazz;
	assert(declarer);

	/* check static */

	if (((mi->flags & ACC_STATIC) != 0) != (invokestatic != false)) {
		/* a static method is accessed via an instance, or vice versa */
		exceptions_throw_incompatibleclasschangeerror(declarer,
													  (mi->flags & ACC_STATIC)
													  ? "static method called via instance"
													  : "instance method called without instance");

		return resolveFailed;
	}

	/* check access rights */

	if (!access_is_accessible_member(referer,declarer,mi->flags)) {
		/* XXX clean this up. this should be in exceptions.c */

		Buffer<> buf(64);

		Utf8String u = buf.write("method is not accessible (")
		                  .write_slash_to_dot(declarer->name)
		                  .write(".")
		                  .write(mi->name)
		                  .write(mi->descriptor)
		                  .write(" from ")
		                  .write_slash_to_dot(referer->name)
		                  .write(")")
		                  .utf8_str();

		exceptions_throw_illegalaccessexception(u);
		return resolveFailed; /* exception */
	}

	/* everything ok */

	return resolveSucceeded;
}
#endif /* defined(ENABLE_VERIFIER) */


/* resolve_method_instance_type_checks *****************************************

   Check the instance type of a method invocation.

   IN:
       refmethod........the method containing the reference
	   mi...............the methodinfo of the resolved method
	   instanceti.......typeinfo of the instance slot
	   invokespecial....true if the method is invoked by INVOKESPECIAL

   RETURN VALUE:
       resolveSucceeded....everything ok
	   resolveDeferred.....tests could not be done, have been deferred
       resolveFailed.......exception has been thrown

*******************************************************************************/

#if defined(ENABLE_VERIFIER)
resolve_result_t resolve_method_instance_type_checks(methodinfo *refmethod,
													 methodinfo *mi,
													 typeinfo_t *instanceti,
													 bool invokespecial)
{
	typeinfo_t         tinfo;
	typeinfo_t        *tip;
	resolve_result_t result;

	if (invokespecial && instanceti->is_newobject())
	{   /* XXX clean up */
		instruction *ins = instanceti->newobject_instruction();
		classref_or_classinfo initclass = (ins) ? ins[-1].sx.val.c : to_classref_or_classinfo(refmethod->clazz);
		tip = &tinfo;
		if (!tip->init_class(initclass))
			return resolveFailed;
	}
	else {
		tip = instanceti;
	}

	result = resolve_lazy_subtype_checks(refmethod,
										 tip,
										 to_classref_or_classinfo(mi->clazz),
										 resolveLinkageError);
	if (result != resolveSucceeded)
		return result;

	/* check protected access */

	/* XXX use other `declarer` than mi->clazz? */
	if (((mi->flags & ACC_PROTECTED) != 0)
			&& !SAME_PACKAGE(mi->clazz, refmethod->clazz))
	{
		result = resolve_lazy_subtype_checks(refmethod,
				tip,
				to_classref_or_classinfo(refmethod->clazz),
				resolveIllegalAccessError);
		if (result != resolveSucceeded)
			return result;
	}

	/* everything ok */

	return resolveSucceeded;
}
#endif /* defined(ENABLE_VERIFIER) */


/* resolve_method_param_type_checks ********************************************

   Check non-instance parameter types of a method invocation.

   IN:
   	   jd...............jitdata of the method doing the call
       refmethod........the method containing the reference
	   iptr.............the invoke instruction
	   mi...............the methodinfo of the resolved method
	   invokestatic.....true if the method is invoked by INVOKESTATIC

   RETURN VALUE:
       resolveSucceeded....everything ok
	   resolveDeferred.....tests could not be done, have been deferred
       resolveFailed.......exception has been thrown

*******************************************************************************/

#if defined(ENABLE_VERIFIER)
resolve_result_t resolve_method_param_type_checks(jitdata *jd, 
												  methodinfo *refmethod,
												  instruction *iptr, 
												  methodinfo *mi,
												  bool invokestatic)
{
	varinfo         *param;
	resolve_result_t result;
	methoddesc      *md;
	typedesc        *paramtypes;
	s4               type;
	s4               instancecount;
	s4               i;

	assert(jd);

	instancecount = (invokestatic) ? 0 : 1;

	/* check subtype constraints for TYPE_ADR parameters */

	md = mi->parseddesc;
	paramtypes = md->paramtypes;

	for (i = md->paramcount-1-instancecount; i>=0; --i) {
		param = VAR(iptr->sx.s23.s2.args[i+instancecount]);
		type = md->paramtypes[i+instancecount].type;

		assert(param);
		assert(type == param->type);

		if (type == TYPE_ADR) {
			result = resolve_lazy_subtype_checks(refmethod,
					&(param->typeinfo),
					to_classref_or_classinfo(paramtypes[i+instancecount].classref),
					resolveLinkageError);
			if (result != resolveSucceeded)
				return result;
		}
	}

	/* everything ok */

	return resolveSucceeded;
}
#endif /* defined(ENABLE_VERIFIER) */


/* resolve_method_param_type_checks_stackbased *********************************

   Check non-instance parameter types of a method invocation.

   IN:
       refmethod........the method containing the reference
	   mi...............the methodinfo of the resolved method
	   invokestatic.....true if the method is invoked by INVOKESTATIC
	   stack............TOS before the INVOKE instruction

   RETURN VALUE:
       resolveSucceeded....everything ok
	   resolveDeferred.....tests could not be done, have been deferred
       resolveFailed.......exception has been thrown

*******************************************************************************/

#if defined(ENABLE_VERIFIER)
resolve_result_t resolve_method_param_type_checks_stackbased(
		methodinfo *refmethod, 
		methodinfo *mi,
		bool invokestatic, 
		typedescriptor_t *stack)
{
	typedescriptor_t  *param;
	resolve_result_t result;
	methoddesc      *md;
	typedesc        *paramtypes;
	s4               type;
	s4               instancecount;
	s4               i;

	instancecount = (invokestatic) ? 0 : 1;

	/* check subtype constraints for TYPE_ADR parameters */

	md = mi->parseddesc;
	paramtypes = md->paramtypes;

	param = stack - (md->paramslots - 1 - instancecount);

	for (i = instancecount; i < md->paramcount; ++i) {
		type = md->paramtypes[i].type;

		assert(type == param->type);

		if (type == TYPE_ADR) {
			result = resolve_lazy_subtype_checks(refmethod,
					&(param->typeinfo),
					to_classref_or_classinfo(paramtypes[i].classref),
					resolveLinkageError);
			if (result != resolveSucceeded)
				return result;
		}

		param += (IS_2_WORD_TYPE(type)) ? 2 : 1;
	}

	/* everything ok */

	return resolveSucceeded;
}
#endif /* defined(ENABLE_VERIFIER) */


/* resolve_method_loading_constraints ******************************************

   Impose loading constraints on the parameters and return type of the
   given method.

   IN:
       referer..........the class refering to the method
	   mi...............the method

   RETURN VALUE:
       true................everything ok
	   false...............an exception has been thrown

*******************************************************************************/

#if defined(ENABLE_VERIFIER)
bool resolve_method_loading_constraints(classinfo *referer,
										methodinfo *mi)
{
	methoddesc *md;
	typedesc   *paramtypes;
	Utf8String  name;
	s4          i;
	s4          instancecount;

	/* impose loading constraints on parameters (including instance) */

	md = mi->parseddesc;
	paramtypes = md->paramtypes;
	instancecount = (mi->flags & ACC_STATIC) / ACC_STATIC;

	for (i = 0; i < md->paramcount; i++) {
		if (i < instancecount || paramtypes[i].type == TYPE_ADR) {
			if (i < instancecount) {
				/* The type of the 'this' pointer is the class containing */
				/* the method definition. Since container is the same as, */
				/* or a subclass of declarer, we also constrain declarer  */
				/* by transitivity of loading constraints.                */
				name = mi->clazz->name;
			}
			else {
				name = paramtypes[i].classref->name;
			}

			/* The caller (referer) and the callee (container) must agree */
			/* on the types of the parameters.                            */
			if (!classcache_add_constraint(referer->classloader,
										   mi->clazz->classloader, name))
				return false; /* exception */
		}
	}

	/* impose loading constraint onto return type */

	if (md->returntype.type == TYPE_ADR) {
		/* The caller (referer) and the callee (container) must agree */
		/* on the return type.                                        */
		if (!classcache_add_constraint(referer->classloader,
					mi->clazz->classloader,
					md->returntype.classref->name))
			return false; /* exception */
	}

	/* everything ok */

	return true;
}
#endif /* defined(ENABLE_VERIFIER) */


/* resolve_method_lazy *********************************************************
 
   Resolve an unresolved method reference lazily
  
   NOTE: This function does NOT do any verification checks. In case of a
         successful resolution, you must call resolve_method_verifier_checks
		 in order to perform the necessary checks!
  
   IN:
	   refmethod........the referer method
	   methodref........the method reference
	   invokespecial....true if this is an INVOKESPECIAL instruction
  
   RETURN VALUE:
       resolveSucceeded.....the reference has been resolved
       resolveDeferred......the resolving could not be performed lazily
	   resolveFailed........resolving failed, an exception has been thrown.
   
*******************************************************************************/

resolve_result_t resolve_method_lazy(methodinfo *refmethod,
									 constant_FMIref *methodref,
									 bool invokespecial)
{
	classinfo *referer;
	classinfo *container;
	methodinfo *mi;

	assert(refmethod);

#ifdef RESOLVE_VERBOSE
	printf("resolve_method_lazy\n");
#endif

	/* the class containing the reference */

	referer = refmethod->clazz;
	assert(referer);

	/* check if the method itself is already resolved */

	if (methodref->is_resolved())
		return resolveSucceeded;

	/* first we must resolve the class containg the method */

	if (!resolve_class_from_name(referer, refmethod,
		   methodref->p.classref->name, resolveLazy, true, true, &container))
	{
		/* the class reference could not be resolved */
		return resolveFailed; /* exception */
	}
	if (!container)
		return resolveDeferred; /* be lazy */

	assert(container->state & CLASS_LINKED);

	/* now we must find the declaration of the method in `container`
	 * or one of its superclasses */

	if (container->flags & ACC_INTERFACE) {
		mi = class_resolveinterfacemethod(container,
									      methodref->name,
										  methodref->descriptor,
									      referer, true);

	} else {
		mi = class_resolveclassmethod(container,
									  methodref->name,
									  methodref->descriptor,
									  referer, true);
	}

	if (!mi) {
		/* The method does not exist. But since we were called lazily, */
		/* this error must not be reported now. (It will be reported   */
		/* if eager resolving of this method is ever tried.)           */

		exceptions_clear_exception();
		return resolveDeferred; /* be lazy */
	}

	if (invokespecial) {
		mi = resolve_method_invokespecial_lookup(refmethod, mi);
		if (!mi)
			return resolveFailed; /* exception */
	}

	/* have the method params already been parsed? no, do it. */

	mi->parseddesc->params_from_paramtypes(mi->flags);

	/* cache the result of the resolution */

	methodref->p.method = mi;

	/* succeed */

	return resolveSucceeded;
}

/* resolve_method **************************************************************

   Resolve an unresolved method reference

   IN:
       ref..............struct containing the reference
       mode.............mode of resolution:
                            resolveLazy...only resolve if it does not
                                          require loading classes
                            resolveEager..load classes if necessary

   OUT:
       *result..........set to the result of resolution, or to NULL if
                        the reference has not been resolved
                        In the case of an exception, *result is
                        guaranteed to be set to NULL.

   RETURN VALUE:
       true.............everything ok
                        (*result may still be NULL for resolveLazy)
       false............an exception has been thrown

*******************************************************************************/

bool resolve_method(unresolved_method *ref, resolve_mode_t mode, methodinfo **result)
{
	classinfo *referer;
	classinfo *container;
	methodinfo *mi;

	assert(ref);
	assert(result);
	assert(mode == resolveLazy || mode == resolveEager);

#ifdef RESOLVE_VERBOSE
	unresolved_method_debug_dump(ref,stdout);
#endif

	*result = NULL;

	/* the class containing the reference */

	referer = ref->referermethod->clazz;
	assert(referer);

	/* check if the method itself is already resolved */

	if (ref->methodref->is_resolved()) {
		mi = ref->methodref->p.method;
		container = mi->clazz;
		goto resolved_the_method;
	}

	/* first we must resolve the class containing the method */

	if (!resolve_class_from_name(referer,ref->referermethod,
					   ref->methodref->p.classref->name,mode,true,true,&container))
	{
		/* the class reference could not be resolved */
		return false; /* exception */
	}
	if (!container)
		return true; /* be lazy */

	assert(container);
	assert(container->state & CLASS_LINKED);

	/* now we must find the declaration of the method in `container`
	 * or one of its superclasses */

	if (container->flags & ACC_INTERFACE) {
		mi = class_resolveinterfacemethod(container,
									      ref->methodref->name,
										  ref->methodref->descriptor,
									      referer, true);

	} else {
		mi = class_resolveclassmethod(container,
									  ref->methodref->name,
									  ref->methodref->descriptor,
									  referer, true);
	}

	if (!mi) {
		if (mode == resolveLazy) {
			/* The method does not exist. But since we were called lazily, */
			/* this error must not be reported now. (It will be reported   */
			/* if eager resolving of this method is ever tried.)           */

			exceptions_clear_exception();
			return true; /* be lazy */
		}

		return false; /* exception */ /* XXX set exceptionptr? */
	}

	/* { the method reference has been resolved } */

	if (ref->flags & RESOLVE_SPECIAL) {
		mi = resolve_method_invokespecial_lookup(ref->referermethod,mi);
		if (!mi)
			return false; /* exception */
	}

	/* have the method params already been parsed? no, do it. */

	mi->parseddesc->params_from_paramtypes(mi->flags);

	/* cache the resolution */

	ref->methodref->p.method = mi;

resolved_the_method:

#ifdef ENABLE_VERIFIER
	if (opt_verify) {
		resolve_result_t checkresult = resolve_method_verifier_checks(
				ref->referermethod,
				ref->methodref,
				mi,
				(ref->flags & RESOLVE_STATIC));

		if (checkresult != resolveSucceeded)
			return (bool) checkresult;

		/* impose loading constraints on params and return type */

		if (!resolve_method_loading_constraints(referer, mi))
			return false;

		classinfo *declarer = mi->clazz;
		assert(declarer);
		assert(referer->state & CLASS_LINKED);

		/* for non-static methods we have to check the constraints on the         */
		/* instance type                                                          */

		int instancecount;

		if (!(ref->flags & RESOLVE_STATIC)) {
			checkresult = resolve_and_check_subtype_set(ref->referermethod,
					&(ref->instancetypes),
					to_classref_or_classinfo(container),
					mode,
					resolveLinkageError);
			if (checkresult != resolveSucceeded)
				return (bool) checkresult;
			instancecount = 1;
		}
		else {
			instancecount = 0;
		}

		/* check subtype constraints for TYPE_ADR parameters */

		assert(mi->parseddesc->paramcount == ref->methodref->parseddesc.md->paramcount);
		typedesc *paramtypes = mi->parseddesc->paramtypes;

		for (int i = 0; i < mi->parseddesc->paramcount-instancecount; i++) {
			if (paramtypes[i+instancecount].type == TYPE_ADR) {
				if (ref->paramconstraints) {
					checkresult = resolve_and_check_subtype_set(ref->referermethod,
							ref->paramconstraints + i,
							to_classref_or_classinfo(paramtypes[i+instancecount].classref),
							mode,
							resolveLinkageError);
					if (checkresult != resolveSucceeded)
						return (bool) checkresult;
				}
			}
		}

		/* check protected access */

		if (((mi->flags & ACC_PROTECTED) != 0) && !SAME_PACKAGE(declarer,referer))
		{
			checkresult = resolve_and_check_subtype_set(ref->referermethod,
					&(ref->instancetypes),
					to_classref_or_classinfo(referer),
					mode,
					resolveIllegalAccessError);
			if (checkresult != resolveSucceeded)
				return (bool) checkresult;
		}
	}
#endif /* ENABLE_VERIFIER */

	/* succeed */
	*result = mi;
	return true;
}

/* resolve_method_eager ********************************************************
 
   Resolve an unresolved method reference eagerly.
  
   IN:
       ref..............struct containing the reference
   
   RETURN VALUE:
       methodinfo * to the method, or
	   NULL if an exception has been thrown
   
*******************************************************************************/

methodinfo * resolve_method_eager(unresolved_method *ref)
{
	methodinfo *mi;

	if (!resolve_method(ref,resolveEager,&mi))
		return NULL;

	return mi;
}

/******************************************************************************/
/* CREATING THE DATA STRUCTURES                                               */
/******************************************************************************/

#ifdef ENABLE_VERIFIER
static bool unresolved_subtype_set_from_typeinfo(classinfo *referer,
												 methodinfo *refmethod,
												 unresolved_subtype_set *stset,
												 typeinfo_t *tinfo,
												 Utf8String declaredclassname)
{
	int count;
	int i;

	assert(stset);
	assert(tinfo);

#ifdef RESOLVE_VERBOSE
	printf("unresolved_subtype_set_from_typeinfo\n");
#ifdef TYPEINFO_DEBUG
	typeinfo_print(stdout,tinfo,4);
#endif
	printf("    declared classname:");utf_fprint_printable_ascii(stdout,declaredclassname);
	printf("\n");
#endif

	if (tinfo->is_primitive()) {
		exceptions_throw_verifyerror(refmethod,
				"Invalid use of returnAddress");
		return false;
	}

	if (tinfo->is_newobject()) {
		exceptions_throw_verifyerror(refmethod,
				"Invalid use of uninitialized object");
		return false;
	}

	/* the nulltype is always assignable */
	if (tinfo->is_nulltype())
		goto empty_set;

	/* every type is assignable to (BOOTSTRAP)java.lang.Object */
	if (declaredclassname == utf8::java_lang_Object
			&& referer->classloader == NULL) /* XXX do loading constraints make the second check obsolete? */
	{
		goto empty_set;
	}

	if (tinfo->merged) {
		count = tinfo->merged->count;
		stset->subtyperefs = MNEW(classref_or_classinfo,count + 1);
		for (i=0; i<count; ++i) {
			classref_or_classinfo c = tinfo->merged->list[i];
			if (tinfo->dimension > 0) {
				/* a merge of array types */
				/* the merged list contains the possible _element_ types, */
				/* so we have to create array types with these elements.  */
				if (c.is_classref()) {
					c.ref = class_get_classref_multiarray_of(tinfo->dimension,c.ref);
				}
				else {
					c.cls = class_multiarray_of(tinfo->dimension,c.cls,false);
				}
			}
			stset->subtyperefs[i] = c;
		}
		stset->subtyperefs[count].any = NULL; /* terminate */
	}
	else {
		if (CLASSREF_OR_CLASSINFO_NAME(tinfo->typeclass) == declaredclassname)
		{
			/* the class names are the same */
		    /* equality is guaranteed by the loading constraints */
			goto empty_set;
		}
		else {
			stset->subtyperefs = MNEW(classref_or_classinfo,1 + 1);
			stset->subtyperefs[0] = tinfo->typeclass;
			stset->subtyperefs[1].any = NULL; /* terminate */
		}
	}

	return true;

empty_set:
	UNRESOLVED_SUBTYPE_SET_EMTPY(*stset);
	return true;
}
#endif /* ENABLE_VERIFIER */

/* create_unresolved_class *****************************************************
 
   Create an unresolved_class struct for the given class reference
  
   IN:
	   refmethod........the method triggering the resolution (if any)
	   classref.........the class reference
	   valuetype........value type to check against the resolved class
	   					may be NULL, if no typeinfo is available

   RETURN VALUE:
       a pointer to a new unresolved_class struct, or
	   NULL if an exception has been thrown

*******************************************************************************/

#ifdef ENABLE_VERIFIER
unresolved_class * create_unresolved_class(methodinfo *refmethod,
										   constant_classref *classref,
										   typeinfo_t *valuetype)
{
	unresolved_class *ref;

#ifdef RESOLVE_VERBOSE
	printf("create_unresolved_class\n");
	printf("    referer: ");utf_fprint_printable_ascii(stdout,classref->referer->name);fputc('\n',stdout);
	if (refmethod) {
		printf("    rmethod: ");utf_fprint_printable_ascii(stdout,refmethod->name);fputc('\n',stdout);
		printf("    rmdesc : ");utf_fprint_printable_ascii(stdout,refmethod->descriptor);fputc('\n',stdout);
	}
	printf("    name   : ");utf_fprint_printable_ascii(stdout,classref->name);fputc('\n',stdout);
#endif

	ref = NEW(unresolved_class);
	ref->classref = classref;
	ref->referermethod = refmethod;

	if (valuetype) {
		if (!unresolved_subtype_set_from_typeinfo(classref->referer,refmethod,
					&(ref->subtypeconstraints),valuetype,classref->name))
			return NULL;
	}
	else {
		UNRESOLVED_SUBTYPE_SET_EMTPY(ref->subtypeconstraints);
	}

	return ref;
}
#endif /* ENABLE_VERIFIER */

/* resolve_create_unresolved_field *********************************************
 
   Create an unresolved_field struct for the given field access instruction
  
   IN:
       referer..........the class containing the reference
	   refmethod........the method triggering the resolution (if any)
	   iptr.............the {GET,PUT}{FIELD,STATIC}{,CONST} instruction

   RETURN VALUE:
       a pointer to a new unresolved_field struct, or
	   NULL if an exception has been thrown

*******************************************************************************/

unresolved_field * resolve_create_unresolved_field(classinfo *referer,
												   methodinfo *refmethod,
												   instruction *iptr)
{
	unresolved_field *ref;
	constant_FMIref *fieldref = NULL;

#ifdef RESOLVE_VERBOSE
	printf("create_unresolved_field\n");
	printf("    referer: ");utf_fprint_printable_ascii(stdout,referer->name);fputc('\n',stdout);
	printf("    rmethod: ");utf_fprint_printable_ascii(stdout,refmethod->name);fputc('\n',stdout);
	printf("    rmdesc : ");utf_fprint_printable_ascii(stdout,refmethod->descriptor);fputc('\n',stdout);
#endif

	ref = NEW(unresolved_field);
	ref->flags = 0;
	ref->referermethod = refmethod;
	UNRESOLVED_SUBTYPE_SET_EMTPY(ref->valueconstraints);

	switch (iptr->opc) {
		case ICMD_PUTFIELD:
			ref->flags |= RESOLVE_PUTFIELD;
			break;

		case ICMD_PUTFIELDCONST:
			ref->flags |= RESOLVE_PUTFIELD;
			break;

		case ICMD_PUTSTATIC:
			ref->flags |= RESOLVE_PUTFIELD | RESOLVE_STATIC;
			break;

		case ICMD_PUTSTATICCONST:
			ref->flags |= RESOLVE_PUTFIELD | RESOLVE_STATIC;
			break;

		case ICMD_GETFIELD:
			break;

		case ICMD_GETSTATIC:
			ref->flags |= RESOLVE_STATIC;
			break;

#if !defined(NDEBUG)
		default:
			assert(false);
#endif
	}

	fieldref = iptr->sx.s23.s3.fmiref;

	assert(fieldref);

#ifdef RESOLVE_VERBOSE
/*	printf("    class  : ");utf_fprint_printable_ascii(stdout,fieldref->p.classref->name);fputc('\n',stdout);*/
	printf("    name   : ");utf_fprint_printable_ascii(stdout,fieldref->name);fputc('\n',stdout);
	printf("    desc   : ");utf_fprint_printable_ascii(stdout,fieldref->descriptor);fputc('\n',stdout);
	printf("    type   : ");descriptor_debug_print_typedesc(stdout,fieldref->parseddesc.fd);
	fputc('\n',stdout);
#endif

	ref->fieldref = fieldref;

	return ref;
}

/* resolve_constrain_unresolved_field ******************************************
 
   Record subtype constraints for a field access.
  
   IN:
       ref..............the unresolved_field structure of the access
       referer..........the class containing the reference
	   refmethod........the method triggering the resolution (if any)
	   instanceti.......instance typeinfo, if available
	   valueti..........value typeinfo, if available

   RETURN VALUE:
       true.............everything ok
	   false............an exception has been thrown

*******************************************************************************/

#if defined(ENABLE_VERIFIER)
bool resolve_constrain_unresolved_field(unresolved_field *ref,
										classinfo *referer, 
										methodinfo *refmethod,
									    typeinfo_t *instanceti,
									    typeinfo_t *valueti)
{
	constant_FMIref *fieldref;
	int type;
	typeinfo_t tinfo;
	typedesc *fd;

	assert(ref);

	fieldref = ref->fieldref;
	assert(fieldref);

#ifdef RESOLVE_VERBOSE
	printf("constrain_unresolved_field\n");
	printf("    referer: ");utf_fprint_printable_ascii(stdout,referer->name);fputc('\n',stdout);
	printf("    rmethod: ");utf_fprint_printable_ascii(stdout,refmethod->name);fputc('\n',stdout);
	printf("    rmdesc : ");utf_fprint_printable_ascii(stdout,refmethod->descriptor);fputc('\n',stdout);
/*	printf("    class  : ");utf_fprint_printable_ascii(stdout,fieldref->p.classref->name);fputc('\n',stdout); */
	printf("    name   : ");utf_fprint_printable_ascii(stdout,fieldref->name);fputc('\n',stdout);
	printf("    desc   : ");utf_fprint_printable_ascii(stdout,fieldref->descriptor);fputc('\n',stdout);
	printf("    type   : ");descriptor_debug_print_typedesc(stdout,fieldref->parseddesc.fd);
	fputc('\n',stdout);
#endif

	assert(instanceti || ((ref->flags & RESOLVE_STATIC) != 0));
	fd = fieldref->parseddesc.fd;
	assert(fd);

	/* record subtype constraints for the instance type, if any */
	if (instanceti) {
		typeinfo_t *insttip;

		/* The instanceslot must contain a reference to a non-array type */
		if (!instanceti->is_reference()) {
			exceptions_throw_verifyerror(refmethod, "illegal instruction: field access on non-reference");
			return false;
		}
		if (instanceti->is_array()) {
			exceptions_throw_verifyerror(refmethod, "illegal instruction: field access on array");
			return false;
		}

		if (((ref->flags & RESOLVE_PUTFIELD) != 0) && instanceti->is_newobject())
		{
			/* The instruction writes a field in an uninitialized object. */
			/* This is only allowed when a field of an uninitialized 'this' object is */
			/* written inside an initialization method                                */

			classinfo *initclass;
			instruction *ins = instanceti->newobject_instruction();

			if (ins != NULL) {
				exceptions_throw_verifyerror(refmethod,
						"accessing field of uninitialized object");
				return false;
			}
			/* XXX check that class of field == refmethod->clazz */
			initclass = refmethod->clazz; /* XXX classrefs */
			assert(initclass->state & CLASS_LOADED);
			assert(initclass->state & CLASS_LINKED);

			tinfo.init_class(initclass);
			insttip = &tinfo;
		}
		else {
			insttip = instanceti;
		}
		if (!unresolved_subtype_set_from_typeinfo(referer, refmethod,
					&(ref->instancetypes), insttip,
					FIELDREF_CLASSNAME(fieldref)))
			return false;
	}
	else {
		UNRESOLVED_SUBTYPE_SET_EMTPY(ref->instancetypes);
	}

	/* record subtype constraints for the value type, if any */
	type = fd->type;
	if (type == TYPE_ADR && ((ref->flags & RESOLVE_PUTFIELD) != 0)) {
		assert(valueti);
		if (!unresolved_subtype_set_from_typeinfo(referer, refmethod,
					&(ref->valueconstraints), valueti, 
					fieldref->parseddesc.fd->classref->name))
			return false;
	}
	else {
		UNRESOLVED_SUBTYPE_SET_EMTPY(ref->valueconstraints);
	}

	return true;
}
#endif /* ENABLE_VERIFIER */

/* resolve_create_unresolved_method ********************************************
 
   Create an unresolved_method struct for the given method invocation
  
   IN:
       referer..........the class containing the reference
	   refmethod........the method triggering the resolution (if any)
	   iptr.............the INVOKE* instruction

   RETURN VALUE:
       a pointer to a new unresolved_method struct

*******************************************************************************/

unresolved_method * resolve_create_unresolved_method(classinfo *referer,
													 methodinfo *refmethod,
													 constant_FMIref *methodref,
													 bool invokestatic,
													 bool invokespecial)
{
	unresolved_method *ref;

	assert(methodref);

#ifdef RESOLVE_VERBOSE
	printf("create_unresolved_method\n");
	printf("    referer: ");utf_fprint_printable_ascii(stdout,referer->name);fputc('\n',stdout);
	printf("    rmethod: ");utf_fprint_printable_ascii(stdout,refmethod->name);fputc('\n',stdout);
	printf("    rmdesc : ");utf_fprint_printable_ascii(stdout,refmethod->descriptor);fputc('\n',stdout);
	printf("    name   : ");utf_fprint_printable_ascii(stdout,methodref->name);fputc('\n',stdout);
	printf("    desc   : ");utf_fprint_printable_ascii(stdout,methodref->descriptor);fputc('\n',stdout);
#endif

	/* allocate params if necessary */
	methodref->parseddesc.md->params_from_paramtypes(invokestatic ? ACC_STATIC : ACC_NONE);

	/* create the data structure */
	ref = NEW(unresolved_method);
	ref->flags = ((invokestatic) ? RESOLVE_STATIC : 0)
			   | ((invokespecial) ? RESOLVE_SPECIAL : 0);
	ref->referermethod = refmethod;
	ref->methodref = methodref;
	ref->paramconstraints = NULL;
	UNRESOLVED_SUBTYPE_SET_EMTPY(ref->instancetypes);

	return ref;
}


/* resolve_constrain_unresolved_method_instance ********************************

   Record subtype constraints for the instance argument of a method call.

   IN:
       ref..............the unresolved_method structure of the call
       referer..........the class containing the reference
	   refmethod........the method triggering the resolution (if any)
	   iptr.............the INVOKE* instruction

   RETURN VALUE:
       true.............everything ok
	   false............an exception has been thrown

*******************************************************************************/

#if defined(ENABLE_VERIFIER)
bool resolve_constrain_unresolved_method_instance(unresolved_method *ref,
												  methodinfo *refmethod,
												  typeinfo_t *instanceti,
												  bool invokespecial)
{
	constant_FMIref   *methodref;
	constant_classref *instanceref;
	typeinfo_t           tinfo;
	typeinfo_t          *tip;

	assert(ref);
	methodref = ref->methodref;
	assert(methodref);

	/* XXX clean this up */
	instanceref = methodref->is_resolved()
		? class_get_self_classref(methodref->p.method->clazz)
		: methodref->p.classref;

#ifdef RESOLVE_VERBOSE
	printf("resolve_constrain_unresolved_method_instance\n");
	printf("    rmethod: "); method_println(refmethod);
	printf("    mref   : "); method_methodref_println(methodref);
#endif

	/* record subtype constraints for the instance type, if any */

	if (invokespecial && instanceti->is_newobject())
	{   /* XXX clean up */
		instruction *ins = instanceti->newobject_instruction();
		classref_or_classinfo initclass = (ins) ? ins[-1].sx.val.c : to_classref_or_classinfo(refmethod->clazz);
		tip = &tinfo;
		if (!tip->init_class(initclass))
			return false;
	}
	else {
		tip = instanceti;
	}

	if (!unresolved_subtype_set_from_typeinfo(refmethod->clazz, refmethod,
				&(ref->instancetypes),tip,instanceref->name))
		return false;

	return true;
}
#endif /* defined(ENABLE_VERIFIER) */


/* resolve_constrain_unresolved_method_params  *********************************
 
   Record subtype constraints for the non-instance arguments of a method call.
  
   IN:
       jd...............current jitdata (for looking up variables)
       ref..............the unresolved_method structure of the call
	   refmethod........the method triggering the resolution (if any)
	   iptr.............the INVOKE* instruction

   RETURN VALUE:
       true.............everything ok
	   false............an exception has been thrown

*******************************************************************************/

#if defined(ENABLE_VERIFIER)
bool resolve_constrain_unresolved_method_params(jitdata *jd,
												unresolved_method *ref,
												methodinfo *refmethod,
												instruction *iptr)
{
	constant_FMIref *methodref;
	varinfo *param;
	methoddesc *md;
	int i,j;
	int type;
	int instancecount;

	assert(ref);
	methodref = ref->methodref;
	assert(methodref);
	md = methodref->parseddesc.md;
	assert(md);
	assert(md->params != NULL);

#ifdef RESOLVE_VERBOSE
	printf("resolve_constrain_unresolved_method_params\n");
	printf("    rmethod: "); method_println(refmethod);
	printf("    mref   : "); method_methodref_println(methodref);
#endif

	instancecount = (ref->flags & RESOLVE_STATIC) ? 0 : 1;

	/* record subtype constraints for the parameter types, if any */

	for (i=md->paramcount-1-instancecount; i>=0; --i) {
		param = VAR(iptr->sx.s23.s2.args[i+instancecount]);
		type = md->paramtypes[i+instancecount].type;

		assert(param);
		assert(type == param->type);

		if (type == TYPE_ADR) {
			if (!ref->paramconstraints) {
				ref->paramconstraints = MNEW(unresolved_subtype_set,md->paramcount);
				for (j=md->paramcount-1-instancecount; j>i; --j)
					UNRESOLVED_SUBTYPE_SET_EMTPY(ref->paramconstraints[j]);
			}
			assert(ref->paramconstraints);
			if (!unresolved_subtype_set_from_typeinfo(refmethod->clazz, refmethod,
						ref->paramconstraints + i,&(param->typeinfo),
						md->paramtypes[i+instancecount].classref->name))
				return false;
		}
		else {
			if (ref->paramconstraints)
				UNRESOLVED_SUBTYPE_SET_EMTPY(ref->paramconstraints[i]);
		}
	}

	return true;
}
#endif /* ENABLE_VERIFIER */


/* resolve_constrain_unresolved_method_params_stackbased ***********************
 
   Record subtype constraints for the non-instance arguments of a method call.
  
   IN:
       ref..............the unresolved_method structure of the call
	   refmethod........the method triggering the resolution (if any)
	   stack............TOS before the INVOKE instruction

   RETURN VALUE:
       true.............everything ok
	   false............an exception has been thrown

*******************************************************************************/

#if defined(ENABLE_VERIFIER)
bool resolve_constrain_unresolved_method_params_stackbased(
		unresolved_method *ref,
		methodinfo *refmethod,
		typedescriptor_t *stack)
{
	constant_FMIref *methodref;
	typedescriptor_t *param;
	methoddesc *md;
	int i,j;
	int type;
	int instancecount;

	assert(ref);
	methodref = ref->methodref;
	assert(methodref);
	md = methodref->parseddesc.md;
	assert(md);
	assert(md->params != NULL);

#ifdef RESOLVE_VERBOSE
	printf("resolve_constrain_unresolved_method_params_stackbased\n");
	printf("    rmethod: "); method_println(refmethod);
	printf("    mref   : "); method_methodref_println(methodref);
#endif

	instancecount = (ref->flags & RESOLVE_STATIC) ? 0 : 1;

	/* record subtype constraints for the parameter types, if any */

	param = stack - (md->paramslots - 1 - instancecount);

	for (i = instancecount; i < md->paramcount; ++i) {
		type = md->paramtypes[i].type;

		assert(type == param->type);

		if (type == TYPE_ADR) {
			if (!ref->paramconstraints) {
				ref->paramconstraints = MNEW(unresolved_subtype_set,md->paramcount);
				for (j = 0; j < i - instancecount; ++j)
					UNRESOLVED_SUBTYPE_SET_EMTPY(ref->paramconstraints[j]);
			}
			assert(ref->paramconstraints);
			if (!unresolved_subtype_set_from_typeinfo(refmethod->clazz, refmethod,
						ref->paramconstraints + i - instancecount,&(param->typeinfo),
						md->paramtypes[i].classref->name))
				return false;
		}
		else {
			if (ref->paramconstraints)
				UNRESOLVED_SUBTYPE_SET_EMTPY(ref->paramconstraints[i]);
		}

		param += (IS_2_WORD_TYPE(type)) ? 2 : 1;
	}

	return true;
}
#endif /* ENABLE_VERIFIER */


/******************************************************************************/
/* FREEING MEMORY                                                             */
/******************************************************************************/

#ifdef ENABLE_VERIFIER
inline static void unresolved_subtype_set_free_list(classref_or_classinfo *list)
{
	if (list) {
		classref_or_classinfo *p = list;

		/* this is silly. we *only* need to count the elements for MFREE */
		while ((p++)->any)
			;
		MFREE(list,classref_or_classinfo,(p - list));
	}
}
#endif /* ENABLE_VERIFIER */

/* unresolved_class_free *******************************************************
 
   Free the memory used by an unresolved_class
  
   IN:
       ref..............the unresolved_class

*******************************************************************************/

void unresolved_class_free(unresolved_class *ref)
{
	assert(ref);

#ifdef ENABLE_VERIFIER
	unresolved_subtype_set_free_list(ref->subtypeconstraints.subtyperefs);
#endif
	FREE(ref,unresolved_class);
}

/* unresolved_field_free *******************************************************
 
   Free the memory used by an unresolved_field
  
   IN:
       ref..............the unresolved_field

*******************************************************************************/

void unresolved_field_free(unresolved_field *ref)
{
	assert(ref);

#ifdef ENABLE_VERIFIER
	unresolved_subtype_set_free_list(ref->instancetypes.subtyperefs);
	unresolved_subtype_set_free_list(ref->valueconstraints.subtyperefs);
#endif
	FREE(ref,unresolved_field);
}

/* unresolved_method_free ******************************************************
 
   Free the memory used by an unresolved_method
  
   IN:
       ref..............the unresolved_method

*******************************************************************************/

void unresolved_method_free(unresolved_method *ref)
{
	assert(ref);

#ifdef ENABLE_VERIFIER
	unresolved_subtype_set_free_list(ref->instancetypes.subtyperefs);
	if (ref->paramconstraints) {
		int i;
		int count = ref->methodref->parseddesc.md->paramcount;

		for (i=0; i<count; ++i)
			unresolved_subtype_set_free_list(ref->paramconstraints[i].subtyperefs);
		MFREE(ref->paramconstraints,unresolved_subtype_set,count);
	}
#endif
	FREE(ref,unresolved_method);
}

/******************************************************************************/
/* DEBUG DUMPS                                                                */
/******************************************************************************/

#if !defined(NDEBUG)

/* unresolved_subtype_set_debug_dump *******************************************

   Print debug info for unresolved_subtype_set to stream

   IN:
       stset............the unresolved_subtype_set
	   file.............the stream

*******************************************************************************/

void unresolved_subtype_set_debug_dump(unresolved_subtype_set *stset,FILE *file)
{
	classref_or_classinfo *p;

	if (SUBTYPESET_IS_EMPTY(*stset)) {
		fprintf(file,"        (empty)\n");
	}
	else {
		p = stset->subtyperefs;
		for (;p->any; ++p) {
			if (p->is_classref()) {
				fprintf(file,"        ref: ");
				utf_fprint_printable_ascii(file,p->ref->name);
			}
			else {
				fprintf(file,"        cls: ");
				utf_fprint_printable_ascii(file,p->cls->name);
			}
			fputc('\n',file);
		}
	}
}

/* unresolved_class_debug_dump *************************************************

   Print debug info for unresolved_class to stream

   IN:
       ref..............the unresolved_class
	   file.............the stream

*******************************************************************************/

void unresolved_class_debug_dump(unresolved_class *ref,FILE *file)
{
	fprintf(file,"unresolved_class(%p):\n",(void *)ref);
	if (ref) {
		fprintf(file,"    referer   : ");
		utf_fprint_printable_ascii(file,ref->classref->referer->name); fputc('\n',file);
		fprintf(file,"    refmethod : ");
		utf_fprint_printable_ascii(file,ref->referermethod->name); fputc('\n',file);
		fprintf(file,"    refmethodd: ");
		utf_fprint_printable_ascii(file,ref->referermethod->descriptor); fputc('\n',file);
		fprintf(file,"    classname : ");
		utf_fprint_printable_ascii(file,ref->classref->name); fputc('\n',file);
		fprintf(file,"    subtypeconstraints:\n");
		unresolved_subtype_set_debug_dump(&(ref->subtypeconstraints),file);
	}
}

/* unresolved_field_debug_dump *************************************************
 
   Print debug info for unresolved_field to stream
  
   IN:
       ref..............the unresolved_field
	   file.............the stream

*******************************************************************************/

void unresolved_field_debug_dump(unresolved_field *ref,FILE *file)
{
	fprintf(file,"unresolved_field(%p):\n",(void *)ref);
	if (ref) {
		fprintf(file,"    referer   : ");
		utf_fprint_printable_ascii(file,ref->referermethod->clazz->name); fputc('\n',file);
		fprintf(file,"    refmethod : ");
		utf_fprint_printable_ascii(file,ref->referermethod->name); fputc('\n',file);
		fprintf(file,"    refmethodd: ");
		utf_fprint_printable_ascii(file,ref->referermethod->descriptor); fputc('\n',file);
		fprintf(file,"    classname : ");
		utf_fprint_printable_ascii(file,FIELDREF_CLASSNAME(ref->fieldref)); fputc('\n',file);
		fprintf(file,"    name      : ");
		utf_fprint_printable_ascii(file,ref->fieldref->name); fputc('\n',file);
		fprintf(file,"    descriptor: ");
		utf_fprint_printable_ascii(file,ref->fieldref->descriptor); fputc('\n',file);
		fprintf(file,"    parseddesc: ");
		descriptor_debug_print_typedesc(file,ref->fieldref->parseddesc.fd); fputc('\n',file);
		fprintf(file,"    flags     : %04x\n",ref->flags);
		fprintf(file,"    instancetypes:\n");
		unresolved_subtype_set_debug_dump(&(ref->instancetypes),file);
		fprintf(file,"    valueconstraints:\n");
		unresolved_subtype_set_debug_dump(&(ref->valueconstraints),file);
	}
}

/* unresolved_method_debug_dump ************************************************
 
   Print debug info for unresolved_method to stream
  
   IN:
       ref..............the unresolved_method
	   file.............the stream

*******************************************************************************/

void unresolved_method_debug_dump(unresolved_method *ref,FILE *file)
{
	int i;

	fprintf(file,"unresolved_method(%p):\n",(void *)ref);
	if (ref) {
		fprintf(file,"    referer   : ");
		utf_fprint_printable_ascii(file,ref->referermethod->clazz->name); fputc('\n',file);
		fprintf(file,"    refmethod : ");
		utf_fprint_printable_ascii(file,ref->referermethod->name); fputc('\n',file);
		fprintf(file,"    refmethodd: ");
		utf_fprint_printable_ascii(file,ref->referermethod->descriptor); fputc('\n',file);
		fprintf(file,"    classname : ");
		utf_fprint_printable_ascii(file,METHODREF_CLASSNAME(ref->methodref)); fputc('\n',file);
		fprintf(file,"    name      : ");
		utf_fprint_printable_ascii(file,ref->methodref->name); fputc('\n',file);
		fprintf(file,"    descriptor: ");
		utf_fprint_printable_ascii(file,ref->methodref->descriptor); fputc('\n',file);
		fprintf(file,"    parseddesc: ");
		descriptor_debug_print_methoddesc(file,ref->methodref->parseddesc.md); fputc('\n',file);
		fprintf(file,"    flags     : %04x\n",ref->flags);
		fprintf(file,"    instancetypes:\n");
		unresolved_subtype_set_debug_dump(&(ref->instancetypes),file);
		fprintf(file,"    paramconstraints:\n");
		if (ref->paramconstraints) {
			for (i=0; i<ref->methodref->parseddesc.md->paramcount; ++i) {
				fprintf(file,"      param %d:\n",i);
				unresolved_subtype_set_debug_dump(ref->paramconstraints + i,file);
			}
		}
		else {
			fprintf(file,"      (empty)\n");
		}
	}
}
#endif /* !defined(NDEBUG) */


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
