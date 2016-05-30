/* src/vm/access.c - checking access rights

   Copyright (C) 1996-2013
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

#include "vm/types.hpp"

#include "native/llni.hpp"

#include "vm/access.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/class.hpp"
#include "vm/exceptions.hpp"
#include "vm/field.hpp"
#include "vm/globals.hpp"
#include "vm/method.hpp"

#include "vm/jit/stacktrace.hpp"

#include "toolbox/buffer.hpp"

/* access_is_accessible_class **************************************************
 
   Check if a class is accessible from another class
  
   IN:
       referer..........the class containing the reference
       cls..............the result of resolving the reference
  
   RETURN VALUE:
       true.............access permitted
       false............access denied
   
   NOTE:
       This function performs the checks listed in section 5.4.4.
	   "Access Control" of "The Java(TM) Virtual Machine Specification,
	   Second Edition".

*******************************************************************************/

bool access_is_accessible_class(classinfo *referer, classinfo *cls)
{
	assert(referer);
	assert(cls);

	/* Public classes are always accessible. */

	if (cls->flags & ACC_PUBLIC)
		return true;

	/* A class in the same package is always accessible. */

	if (SAME_PACKAGE(referer, cls))
		return true;

#if defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
	/* Code for Sun's OpenJDK (see
	   hotspot/src/share/vm/runtime/reflection.cpp
	   (Reflection::verify_class_access)): Allow all accesses from
	   sun/reflect/MagicAccessorImpl subclasses to succeed
	   trivially. */

	/* NOTE: This check must be before checks that could return
	   false. */

	if (class_issubclass(referer, class_sun_reflect_MagicAccessorImpl))
		return true;
#endif

	/* A non-public class in another package is not accessible. */

	return false;
}


/* access_is_accessible_member *************************************************
 
   Check if a field or method is accessible from a given class
  
   IN:
       referer..........the class containing the reference
       declarer.........the class declaring the member
       memberflags......the access flags of the member
  
   RETURN VALUE:
       true.............access permitted
       false............access denied

   NOTE:
       This function only performs the checks listed in section 5.4.4.
	   "Access Control" of "The Java(TM) Virtual Machine Specification,
	   Second Edition".

	   In particular a special condition for protected access with is
	   part of the verification process according to the spec is not
	   checked in this function.
   
*******************************************************************************/

bool access_is_accessible_member(classinfo *referer, classinfo *declarer,
								 s4 memberflags)
{
	assert(referer);
	assert(declarer);

	/* Public members are accessible. */

	if (memberflags & ACC_PUBLIC)
		return true;

#if defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
	/* Code for Sun's OpenJDK (see
	   hotspot/src/share/vm/runtime/reflection.cpp
	   (Reflection::verify_class_access)): Allow all accesses from
	   sun/reflect/MagicAccessorImpl subclasses to succeed
	   trivially. */

	/* NOTE: This check must be before checks that could return
	   false. */

	if (class_issubclass(referer, class_sun_reflect_MagicAccessorImpl))
		return true;
#endif

	/* {declarer is not an interface} */

	/* private members are only accessible by the class itself */

	if (memberflags & ACC_PRIVATE)
		return (referer == declarer);

	/* {the member is protected or package private} */

	/* protected and package private members are accessible in the
	   same package */

	if (SAME_PACKAGE(referer, declarer))
		return true;

	/* package private members are not accessible outside the package */

	if (!(memberflags & ACC_PROTECTED))
		return false;

	/* {the member is protected and declarer is in another package} */

	/* a necessary condition for access is that referer is a subclass
	   of declarer */

	assert((referer->state & CLASS_LINKED) && (declarer->state & CLASS_LINKED));

	if (class_isanysubclass(referer, declarer))
		return true;

	return false;
}


/* access_check_field **********************************************************
 
   Check if the (indirect) caller has access rights to the specified
   field.
  
   IN:
       f................the field to check
	   callerdepth......number of callers to ignore
	                    For example if the stacktrace looks like this:

				   [0] java.lang.reflect.Method.invokeNative (Native Method)
				   [1] java.lang.reflect.Method.invoke
				   [2] <caller>

				        you must specify 2 so the access rights of <caller> 
						are checked.
  
   RETURN VALUE:
       true.............access permitted
       false............access denied, an exception has been thrown
   
*******************************************************************************/

#if defined(ENABLE_JAVASE)
bool access_check_field(fieldinfo *f, int callerdepth)
{
	classinfo *callerclass;

	/* If everything is public, there is nothing to check. */

	if ((f->clazz->flags & ACC_PUBLIC) && (f->flags & ACC_PUBLIC))
		return true;

	/* Get the caller's class. */

	callerclass = stacktrace_get_caller_class(callerdepth);

	if (callerclass == NULL)
		return false;

	/* Check access rights. */

	if (!access_is_accessible_member(callerclass, f->clazz, f->flags)) {
		Buffer<> buf;

		Utf8String u = buf.write_slash_to_dot(f->clazz->name)
		                  .write('.')
		                  .write_slash_to_dot(f->name)
		                  .write(" not accessible from ", 21)
		                  .write_slash_to_dot(callerclass->name)
		                  .utf8_str();

		exceptions_throw_illegalaccessexception(u);
		return false;
	}

	/* access granted */

	return true;
}
#endif


/* access_check_method *********************************************************
 
   Check if the (indirect) caller has access rights to the specified
   method.
  
   IN:
       m................the method to check
	   callerdepth......number of callers to ignore
	                    For example if the stacktrace looks like this:

				   [1] java.lang.reflect.Method.invokeNative (Native Method)
				   [1] java.lang.reflect.Method.invoke
				   [2] <caller>

				        you must specify 2 so the access rights of <caller> 
						are checked.
  
   RETURN VALUE:
       true.............access permitted
       false............access denied, an exception has been thrown
   
*******************************************************************************/

#if defined(ENABLE_JAVASE)
bool access_check_method(methodinfo *m, int callerdepth)
{
	classinfo *callerclass;

	/* If everything is public, there is nothing to check. */

	if ((m->clazz->flags & ACC_PUBLIC) && (m->flags & ACC_PUBLIC))
		return true;

	/* Get the caller's class. */

	callerclass = stacktrace_get_caller_class(callerdepth);

	if (callerclass == NULL)
		return false;

	/* Check access rights. */

	if (!access_is_accessible_member(callerclass, m->clazz, m->flags)) {
		Buffer<> buf(64);

		buf.write_slash_to_dot(m->clazz->name)
		   .write('.')
		   .write_slash_to_dot(m->name)
		   .write_slash_to_dot(m->descriptor)
		   .write(" not accessible from ", 21)
		   .write_slash_to_dot(callerclass->name);

		exceptions_throw_illegalaccessexception(buf.utf8_str());
		return false;
	}

	/* access granted */

	return true;
}

#endif

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

