/* src/native/vm/reflection.cpp - helper functions for java/lang/reflect

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

#include "native/llni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_ANNOTATIONS) && defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
# include "vm/vm.hpp"
#endif

#include "native/vm/reflection.hpp"

#include "vm/access.hpp"
#include "vm/array.hpp"
#include "vm/descriptor.hpp"
#include "vm/exceptions.hpp"
#include "vm/global.hpp"
#include "vm/globals.hpp"
#include "vm/initialize.hpp"
#include "vm/javaobjects.hpp"
#include "vm/method.hpp"
#include "vm/vm.hpp"

#include "vm/jit/builtin.hpp"


/**
 * Invoke a method on the given object with the given arguments.
 *
 * For instance methods OBJ must be != NULL and the method is looked up
 * in the vftbl of the object.
 *
 * For static methods, OBJ is ignored.
 */
java_handle_t* Reflection::invoke(methodinfo *m, java_handle_t *o, java_handle_objectarray_t *params)
{
	methodinfo    *resm;
	java_handle_t *ro;
	int            argcount;
	int            paramcount;

	/* Sanity check. */

	assert(m != NULL);

	argcount = m->parseddesc->paramcount;
	paramcount = argcount;

	/* If method is non-static, remove the `this' pointer. */

	if (!(m->flags & ACC_STATIC))
		paramcount--;

	/* For instance methods the object has to be an instance of the
	   class the method belongs to. For static methods the obj
	   parameter is ignored. */

	if (!(m->flags & ACC_STATIC) && o && (!builtin_instanceof(o, m->clazz))) {
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	/* check if we got the right number of arguments */

	ObjectArray oa(params);

	if (((params == NULL) && (paramcount != 0)) ||
		(params && (oa.get_length() != paramcount))) 
	{
		exceptions_throw_illegalargumentexception();
		return NULL;
	}

	/* for instance methods we need an object */

	if (!(m->flags & ACC_STATIC) && (o == NULL)) {
		/* XXX not sure if that is the correct exception */
		exceptions_throw_nullpointerexception();
		return NULL;
	}

	/* for static methods, zero object to make subsequent code simpler */
	if (m->flags & ACC_STATIC)
		o = NULL;

	if (o != NULL) {
		/* for instance methods we must do a vftbl lookup */
		resm = method_vftbl_lookup(LLNI_vftbl_direct(o), m);
	}
	else {
		/* for static methods, just for convenience */
		resm = m;
	}

	ro = vm_call_method_objectarray(resm, o, params);

	return ro;
}


#if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH) && defined(ENABLE_ANNOTATIONS)
/* reflect_get_declaredannotations *********************************************

   Calls the annotation parser with the unparsed annotations and returnes
   the parsed annotations as a map.
   
   IN:
       annotations........the unparsed annotations
       declaringClass.....the class in which the annotated element is declared
       referer............the calling class (for the 'referer' parameter of
                          vm_call_method())

   RETURN VALUE:
       The parsed annotations as a
	   java.util.Map<Class<? extends Annotation>, Annotation>.

*******************************************************************************/

java_handle_t* Reflection::get_declaredannotations(java_handle_bytearray_t *annotations, classinfo* declaringClass, classinfo *referer)
{
	static methodinfo* m_parseAnnotations = NULL;

	java_handle_t* h = native_new_and_init(class_sun_reflect_ConstantPool);
		
	if (h == NULL)
		return NULL;

	sun_reflect_ConstantPool cp(h);
	cp.set_constantPoolOop(declaringClass);
		
	/* only resolve the parser method the first time */
	if (m_parseAnnotations == NULL) {
		// FIXME Use globals.
		Utf8String utf_parseAnnotations = Utf8String::from_utf8("parseAnnotations");
		Utf8String utf_desc             = Utf8String::from_utf8("([BLsun/reflect/ConstantPool;Ljava/lang/Class;)Ljava/util/Map;");

		if (utf_parseAnnotations == NULL || utf_desc == NULL)
			return NULL;
		
		m_parseAnnotations = class_resolveclassmethod(
			class_sun_reflect_annotation_AnnotationParser,
			utf_parseAnnotations,
			utf_desc,
			referer,
			true);
	
		if (m_parseAnnotations == NULL)
			return NULL;
	}
	
	return (java_handle_t*) vm_call_method(m_parseAnnotations, NULL, annotations, cp.get_handle(), declaringClass);
}


/* reflect_get_parameterannotations *******************************************

   Calls the annotation parser with the unparsed parameter annotations of
   a method and returnes the parsed parameter annotations in a 2 dimensional
   array.
   
   IN:
       parameterAnnotations....the unparsed parameter annotations
	   slot....................the slot of the method
       declaringClass..........the class in which the annotated element is
	                           declared
       referer.................the calling class (for the 'referer' parameter
                               of vm_call_method())

   RETURN VALUE:
       The parsed parameter annotations in a 2 dimensional array.

*******************************************************************************/

java_handle_objectarray_t* Reflection::get_parameterannotations(java_handle_bytearray_t* parameterAnnotations, methodinfo* m, classinfo* referer)
{
	/* This method in java would be basically the following.
	 * We don't do it in java because we don't want to make a
	 * public method with wich you can get a ConstantPool, because
	 * with that you could read any kind of constants (even private
	 * ones).
	 *
	 * ConstantPool constPool = new ConstantPool();
	 * constPool.constantPoolOop = method.getDeclaringClass();
	 * return sun.reflect.AnnotationParser.parseParameterAnnotations(
	 * 	  parameterAnnotations,
	 * 	  constPool,
	 * 	  method.getDeclaringClass(),
	 * 	  method.getParameterTypes().length);
	 */

	static methodinfo* m_parseParameterAnnotations = NULL;

	/* get parameter count */

	int32_t numParameters = method_get_parametercount(m);

	/* get ConstantPool */

	java_handle_t* h = native_new_and_init(class_sun_reflect_ConstantPool);
	
	if (h == NULL)
		return NULL;

	sun_reflect_ConstantPool cp(h);
	cp.set_constantPoolOop(m->clazz);

	/* only resolve the parser method the first time */
	if (m_parseParameterAnnotations == NULL) {
		Utf8String utf_parseParameterAnnotations = Utf8String::from_utf8("parseParameterAnnotations");
		Utf8String utf_desc                      = Utf8String::from_utf8("([BLsun/reflect/ConstantPool;Ljava/lang/Class;I)[[Ljava/lang/annotation/Annotation;");

		if (utf_parseParameterAnnotations == NULL || utf_desc == NULL)
			return NULL;

		/* get parser method */

		m_parseParameterAnnotations = class_resolveclassmethod(
			class_sun_reflect_annotation_AnnotationParser,
			utf_parseParameterAnnotations,
			utf_desc,
			referer,
			true);

		if (m_parseParameterAnnotations == NULL)
			return NULL;
	}

	return (java_handle_objectarray_t*) vm_call_method(m_parseParameterAnnotations, NULL, parameterAnnotations, cp.get_handle(), m->clazz, numParameters);
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
