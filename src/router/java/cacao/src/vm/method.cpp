/* src/vm/method.cpp - method functions

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


#include "vm/method.hpp"
#include "config.h"                     // for ENABLE_ANNOTATIONS, etc

#include <cassert>                      // for assert
#include <cstdio>                       // for printf
#include <stdint.h>                     // for int32_t

#include "mm/memory.hpp"

#include "native/llni.hpp"

#include "threads/mutex.hpp"            // for Mutex

#include "vm/annotation.hpp"
#include "vm/array.hpp"                 // for ObjectArray, ClassArray
#include "vm/breakpoint.hpp"            // for BreakpointTable
#include "vm/class.hpp"                 // for classinfo, etc
#include "vm/descriptor.hpp"
#include "vm/exceptions.hpp"
#include "vm/global.hpp"                // for java_handle_bytearray_t, etc
#include "vm/globals.hpp"
#include "vm/linker.hpp"
#include "vm/loader.hpp"                // for loader_skip_attribute_body, etc
#include "vm/options.hpp"               // for opt_debugcolor, opt_verify, etc
#include "vm/references.hpp"            // for classref_or_classinfo, etc
#include "vm/resolve.hpp"               // for resolve_class_from_typedesc, etc
#include "vm/stackmap.hpp"
#include "vm/statistics.hpp"
#include "vm/suck.hpp"                  // for suck_u2, etc
#include "vm/types.hpp"                 // for s4, u2, u1, u4
#include "vm/utf8.hpp"                  // for Utf8String, etc
#include "vm/vftbl.hpp"                 // for vftbl_t
#include "vm/vm.hpp"                    // for vm_abort

#include "vm/jit/builtin.hpp"           // for builtintable_entry
#include "vm/jit/code.hpp"              // for code_free_code_of_method, etc
#include "vm/jit/methodheader.hpp"
#include "vm/jit/stubs.hpp"             // for CompilerStub, NativeStub

using namespace cacao;


#if !defined(NDEBUG) && defined(ENABLE_INLINING)
#define INLINELOG(code)  do { if (opt_TraceInlining) { code } } while (0)
#else
#define INLINELOG(code)
#endif


STAT_REGISTER_VAR(int,count_all_methods,0,"all methods","Number of loaded Methods")

STAT_DECLARE_GROUP(info_struct_stat)
STAT_REGISTER_GROUP_VAR(int,size_lineinfo,0,"size lineinfo","lineinfo",info_struct_stat) // sizeof(lineinfo)?

STAT_DECLARE_GROUP(memory_stat)
STAT_REGISTER_SUM_SUBGROUP(table_stat,"info structs","info struct usage",memory_stat)
STAT_REGISTER_GROUP_VAR(int,count_extable_len,0,"extable len","exception tables",table_stat)
STAT_REGISTER_GROUP_VAR_EXTERN(int,size_linenumbertable,0,"size linenumbertable","size of linenumber tables",table_stat)
STAT_REGISTER_GROUP_VAR_EXTERN(int,count_linenumbertable,0,"count linenumbertable","number of linenumber tables",table_stat)
STAT_REGISTER_GROUP_VAR_EXTERN(int,size_patchref,0,"patchref","patcher references",table_stat)

STAT_DECLARE_VAR(int,count_vmcode_len,0)
/* global variables ***********************************************************/

methodinfo *method_java_lang_reflect_Method_invoke;


/* method_init *****************************************************************

   Initialize method subsystem.

*******************************************************************************/

void method_init(void)
{
#if defined(ENABLE_JAVASE)
	/* Sanity check. */

	if (class_java_lang_reflect_Method == NULL)
		vm_abort("method_init: class_java_lang_reflect_Method is NULL");

	/* Cache java.lang.reflect.Method.invoke() */

	method_java_lang_reflect_Method_invoke =
		class_findmethod(class_java_lang_reflect_Method, utf8::invoke, NULL);

	if (method_java_lang_reflect_Method_invoke == NULL)
		vm_abort("method_init: Could not resolve method java.lang.reflect.Method.invoke().");
#endif
}


/* method_load *****************************************************************

   Loads a method from the class file and fills an existing methodinfo
   structure.

   method_info {
       u2 access_flags;
	   u2 name_index;
	   u2 descriptor_index;
	   u2 attributes_count;
	   attribute_info attributes[attribute_count];
   }

   attribute_info {
       u2 attribute_name_index;
	   u4 attribute_length;
	   u1 info[attribute_length];
   }

   LineNumberTable_attribute {
       u2 attribute_name_index;
	   u4 attribute_length;
	   u2 line_number_table_length;
	   {
	       u2 start_pc;
		   u2 line_number;
	   } line_number_table[line_number_table_length];
   }

   LocalVariableTable_attribute {
       u2 attribute_name_index;
       u4 attribute_length;
       u2 local_variable_table_length;
       {
           u2 start_pc;
           u2 length;
           u2 name_index;
           u2 descriptor_index;
           u2 index;
       } local_variable_table[local_variable_table_length];
   }

*******************************************************************************/

bool method_load(ClassBuffer& cb, methodinfo *m, DescriptorPool& descpool)
{
	int argcount;
	s4         i, j, k, l;
	Utf8String u;
	u2         name_index;
	u2         descriptor_index;
	u2         attributes_count;
	u2         attribute_name_index;
	Utf8String attribute_name;
	u2         code_attributes_count;
	u2         code_attribute_name_index;
	Utf8String code_attribute_name;

	/* get classinfo */

	classinfo *c = cb.get_class();

	m->mutex = new Mutex();

	STATISTICS(count_all_methods++);

	/* all fields of m have been zeroed in load_class_from_classbuffer */

	m->clazz = c;

	if (!cb.check_size(2 + 2 + 2))
		return false;

	/* access flags */

	m->flags = cb.read_u2();

	/* name */

	name_index = cb.read_u2();

	if (!(u = (utf*) class_getconstant(c, name_index, CONSTANT_Utf8)))
		return false;

	m->name = u;

	/* descriptor */

	descriptor_index = cb.read_u2();

	if (!(u = (utf*) class_getconstant(c, descriptor_index, CONSTANT_Utf8)))
		return false;

	m->descriptor = u;

	if ((argcount = descpool.add_method(u)) == -1)
		return false;

#ifdef ENABLE_VERIFIER
	if (opt_verify) {
		if (!Utf8String(m->name).is_valid_name()) {
			exceptions_throw_classformaterror(c, "Method with invalid name");
			return false;
		}

		if (m->name[0] == '<' &&
			m->name != utf8::init && m->name != utf8::clinit) {
			exceptions_throw_classformaterror(c, "Method with invalid special name");
			return false;
		}
	}
#endif /* ENABLE_VERIFIER */

	/* Ignore flags for class initializer according to section 4.6
	   of "The Java Virtual Machine Specification, 2nd Edition" (see PR125). */

	if (m->name == utf8::clinit) {
		m->flags &= ACC_STRICT;
		m->flags |= ACC_STATIC;
	}

	if (!(m->flags & ACC_STATIC))
		argcount++; /* count the 'this' argument */

#ifdef ENABLE_VERIFIER
	if (opt_verify) {
		if (argcount > 255) {
			exceptions_throw_classformaterror(c, "Too many arguments in signature");
			return false;
		}

		/* check flag consistency */
		if (m->name != utf8::clinit) {
			i = (m->flags & (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED));

			if (i != 0 && i != ACC_PUBLIC && i != ACC_PRIVATE && i != ACC_PROTECTED) {
				exceptions_throw_classformaterror(c,
												  "Illegal method modifiers: 0x%X",
												  m->flags);
				return false;
			}

			if (m->flags & ACC_ABSTRACT) {
				if ((m->flags & (ACC_FINAL | ACC_NATIVE | ACC_PRIVATE |
								 ACC_STATIC | ACC_STRICT | ACC_SYNCHRONIZED))) {
					exceptions_throw_classformaterror(c,
													  "Illegal method modifiers: 0x%X",
													  m->flags);
					return false;
				}
			}

			if (c->flags & ACC_INTERFACE) {
				if ((m->flags & (ACC_ABSTRACT | ACC_PUBLIC)) != (ACC_ABSTRACT | ACC_PUBLIC)) {
					exceptions_throw_classformaterror(c,
													  "Illegal method modifiers: 0x%X",
													  m->flags);
					return false;
				}
			}

			if (m->name == utf8::init) {
				if (m->flags & (ACC_STATIC | ACC_FINAL | ACC_SYNCHRONIZED |
								ACC_NATIVE | ACC_ABSTRACT)) {
					exceptions_throw_classformaterror(c, "Instance initialization method has invalid flags set");
					return false;
				}
			}
		}
	}
#endif /* ENABLE_VERIFIER */

	/* mark the method as monomorphic until further notice */

	m->flags |= ACC_METHOD_MONOMORPHIC;

	/* non-abstract methods have an implementation in this class */

	if (!(m->flags & ACC_ABSTRACT))
		m->flags |= ACC_METHOD_IMPLEMENTED;
		
	if (!cb.check_size(2))
		return false;

	/* attributes count */

	attributes_count = cb.read_u2();

	for (i = 0; i < attributes_count; i++) {
		if (!cb.check_size(2))
			return false;

		/* attribute name index */

		attribute_name_index = cb.read_u2();

		attribute_name =
			(utf*) class_getconstant(c, attribute_name_index, CONSTANT_Utf8);

		if (attribute_name == NULL)
			return false;

		if (attribute_name == utf8::Code) {
			/* Code */

			if (m->flags & (ACC_ABSTRACT | ACC_NATIVE)) {
				exceptions_throw_classformaterror(c, "Code attribute in native or abstract methods");
				return false;
			}
			
			if (m->jcode) {
				exceptions_throw_classformaterror(c, "Multiple Code attributes");
				return false;
			}

			if (!cb.check_size(4 + 2 + 2))
				return false;

			cb.read_u4();
			m->maxstack  = cb.read_u2();
			m->maxlocals = cb.read_u2();

			if (m->maxlocals < argcount) {
				exceptions_throw_classformaterror(c, "Arguments can't fit into locals");
				return false;
			}
			
			if (!cb.check_size(4))
				return false;

			m->jcodelength = cb.read_u4();

			if (m->jcodelength == 0) {
				exceptions_throw_classformaterror(c, "Code of a method has length 0");
				return false;
			}
			
			if (m->jcodelength > 65535) {
				exceptions_throw_classformaterror(c, "Code of a method longer than 65535 bytes");
				return false;
			}

			if (!cb.check_size(m->jcodelength))
				return false;

			m->jcode = MNEW(u1, m->jcodelength);
			cb.read_nbytes(m->jcode, m->jcodelength);

			if (!cb.check_size(2))
				return false;

			m->rawexceptiontablelength = cb.read_u2();
			if (!cb.check_size((2 + 2 + 2 + 2) * m->rawexceptiontablelength))
				return false;

			m->rawexceptiontable = MNEW(raw_exception_entry, m->rawexceptiontablelength);

			STATISTICS(count_vmcode_len += m->jcodelength + 18);
			STATISTICS(count_extable_len +=
				m->rawexceptiontablelength * sizeof(raw_exception_entry));

			for (j = 0; j < m->rawexceptiontablelength; j++) {
				u4 idx;
				m->rawexceptiontable[j].startpc   = cb.read_u2();
				m->rawexceptiontable[j].endpc     = cb.read_u2();
				m->rawexceptiontable[j].handlerpc = cb.read_u2();

				idx = cb.read_u2();

				if (!idx) {
					m->rawexceptiontable[j].catchtype.any = NULL;
				}
				else {
					/* the classref is created later */
					if (!(m->rawexceptiontable[j].catchtype.any =
						  (utf *) class_getconstant(c, idx, CONSTANT_ClassName)))
						return false;
				}
			}

			if (!cb.check_size(2))
				return false;

			/* code attributes count */

			code_attributes_count = cb.read_u2();

			for (k = 0; k < code_attributes_count; k++) {
				if (!cb.check_size(2))
					return false;

				/* code attribute name index */

				code_attribute_name_index = cb.read_u2();

				code_attribute_name =
					(utf*) class_getconstant(c, code_attribute_name_index, CONSTANT_Utf8);

				if (code_attribute_name == NULL)
					return false;

				/* check which code attribute */

				if (code_attribute_name == utf8::LineNumberTable) {
					/* LineNumberTable */

					if (!cb.check_size(4 + 2))
						return false;

					/* attribute length */

					(void) cb.read_u4();

					/* line number table length */

					m->linenumbercount = cb.read_u2();

					if (!cb.check_size((2 + 2) * m->linenumbercount))
						return false;

					m->linenumbers = MNEW(lineinfo, m->linenumbercount);

					STATISTICS(size_lineinfo += sizeof(lineinfo) * m->linenumbercount);

					for (l = 0; l < m->linenumbercount; l++) {
						m->linenumbers[l].start_pc    = cb.read_u2();
						m->linenumbers[l].line_number = cb.read_u2();
					}
				}
#if defined(ENABLE_JAVASE)
				else if (code_attribute_name == utf8::StackMapTable) {
					/* StackTableMap */

					if (!stackmap_load_attribute_stackmaptable(cb, m))
						return false;
				}
# if defined(ENABLE_JVMTI)
				else if (code_attribute_name == utf8::LocalVariableTable) {
					/* LocalVariableTable */

					if (m->localvars != NULL) {
						exceptions_throw_classformaterror(c, "Multiple LocalVariableTable attributes");
						return false;
					}

					if (!cb.check_size(4 + 2))
						return false;

					// Attribute length.
					(void) cb.read_u4();

					m->localvarcount = cb.read_u2();

					if (!cb.check_size(10 * m->localvarcount))
						return false;

					m->localvars = MNEW(localvarinfo, m->localvarcount);

					for (l = 0; l < m->localvarcount; l++) {
						m->localvars[l].start_pc = cb.read_u2();
						m->localvars[l].length   = cb.read_u2();

						uint16_t name_index = cb.read_u2();
						if (!(m->localvars[l].name = (utf*) class_getconstant(c, name_index, CONSTANT_Utf8)))
							return false;

						uint16_t descriptor_index = cb.read_u2();
						if (!(m->localvars[l].descriptor = (utf*) class_getconstant(c, descriptor_index, CONSTANT_Utf8)))
							return false;

						m->localvars[l].index = cb.read_u2();

						// XXX Check if index is in range.
						// XXX Check if index already taken.
					}
				}
# endif /* defined(ENABLE_JVMTI) */
#endif /* defined(ENABLE_JAVASE) */
				else {
					/* unknown code attribute */

					if (!loader_skip_attribute_body(cb))
						return false;
				}
			}
		}
		else if (attribute_name == utf8::Exceptions) {
			/* Exceptions */

			if (m->thrownexceptions != NULL) {
				exceptions_throw_classformaterror(c, "Multiple Exceptions attributes");
				return false;
			}

			if (!cb.check_size(4 + 2))
				return false;

			/* attribute length */

			(void) cb.read_u4();

			m->thrownexceptionscount = cb.read_u2();

			if (!cb.check_size(2 * m->thrownexceptionscount))
				return false;

			m->thrownexceptions = MNEW(classref_or_classinfo, m->thrownexceptionscount);

			for (j = 0; j < m->thrownexceptionscount; j++) {
				/* the classref is created later */
				if (!((m->thrownexceptions)[j].any =
					  (utf*) class_getconstant(c, cb.read_u2(), CONSTANT_ClassName)))
					return false;
			}
		}
#if defined(ENABLE_JAVASE)
		else if (attribute_name == utf8::Signature) {
			/* Signature */

			// TODO: change methodinfo.signature to Utf8String
			//       and use it directly

			Utf8String signature = m->signature;

			if (!loader_load_attribute_signature(cb, signature)) {
				return false;
			}

			m->signature = signature;
		}

# if defined(ENABLE_ANNOTATIONS)
		else if (attribute_name == utf8::RuntimeVisibleAnnotations) {
			/* RuntimeVisibleAnnotations */
			if (!annotation_load_method_attribute_runtimevisibleannotations(cb, m))
				return false;
		}
		else if (attribute_name == utf8::RuntimeInvisibleAnnotations) {
			/* RuntimeInvisibleAnnotations */
			if (!annotation_load_method_attribute_runtimeinvisibleannotations(cb, m))
				return false;
		}
		else if (attribute_name == utf8::RuntimeVisibleParameterAnnotations) {
			/* RuntimeVisibleParameterAnnotations */
			if (!annotation_load_method_attribute_runtimevisibleparameterannotations(cb, m))
				return false;
		}
		else if (attribute_name == utf8::RuntimeInvisibleParameterAnnotations) {
			/* RuntimeInvisibleParameterAnnotations */
			if (!annotation_load_method_attribute_runtimeinvisibleparameterannotations(cb, m))
				return false;
		}
		else if (attribute_name == utf8::AnnotationDefault) {
			/* AnnotationDefault */
			if (!annotation_load_method_attribute_annotationdefault(cb, m))
				return false;
		}
# endif
#endif
		else {
			/* unknown attribute */

			if (!loader_skip_attribute_body(cb))
				return false;
		}
	}

	if ((m->jcode == NULL) && !(m->flags & (ACC_ABSTRACT | ACC_NATIVE))) {
		exceptions_throw_classformaterror(c, "Missing Code attribute");
		return false;
	}

#if defined(ENABLE_REPLACEMENT)
	/* initialize the hit countdown field */

	m->hitcountdown = METHOD_INITIAL_HIT_COUNTDOWN;
#endif

	/* everything was ok */

	return true;
}


/* method_free *****************************************************************

   Frees all memory that was allocated for this method.

*******************************************************************************/

void method_free(methodinfo *m)
{
	if (m->mutex)
		delete m->mutex;

	if (m->jcode)
		MFREE(m->jcode, u1, m->jcodelength);

	if (m->rawexceptiontable)
		MFREE(m->rawexceptiontable, raw_exception_entry, m->rawexceptiontablelength);

	code_free_code_of_method(m);

	if (m->stubroutine) {
		if (m->flags & ACC_NATIVE) {
			NativeStub::remove(m->stubroutine);
		}
		else {
			CompilerStub::remove(m->stubroutine);
		}
	}

	if (m->breakpoints)
		delete m->breakpoints;
}


/* method_canoverwrite *********************************************************

   Check if m and old are identical with respect to type and
   name. This means that old can be overwritten with m.
	
*******************************************************************************/

bool method_canoverwrite(methodinfo *m, methodinfo *old)
{
	if (m->name != old->name)
		return false;

	if (m->descriptor != old->descriptor)
		return false;

	if (m->flags & ACC_STATIC)
		return false;

	return true;
}


/* method_new_builtin **********************************************************

   Creates a minimal methodinfo structure for builtins. This comes handy
   when dealing with builtin stubs or stacktraces.

*******************************************************************************/

methodinfo *method_new_builtin(builtintable_entry *bte)
{
	methodinfo *m;

	/* allocate the methodinfo structure */

	m = NEW(methodinfo);

	/* initialize methodinfo structure */

	MZERO(m, methodinfo, 1);
	
	m->mutex      = new Mutex();
	m->flags      = ACC_METHOD_BUILTIN;
	m->parseddesc = bte->md;
	m->name       = bte->name;
	m->descriptor = bte->descriptor;

	/* return the newly created methodinfo */

	return m;
}


/* method_vftbl_lookup *********************************************************

   Does a method lookup in the passed virtual function table.  This
   function does exactly the same thing as JIT, but additionally
   relies on the fact, that the methodinfo pointer is at the first
   data segment slot (even for compiler stubs).

*******************************************************************************/

methodinfo *method_vftbl_lookup(vftbl_t *vftbl, methodinfo* m)
{
	methodptr   mptr;
	methodptr  *pmptr;
	methodinfo *resm;                   /* pointer to new resolved method     */

	/* If the method is not an instance method, just return it. */

	if (m->flags & ACC_STATIC)
		return m;

	assert(vftbl);

	/* Get the method from the virtual function table.  Is this an
	   interface method? */

	if (m->clazz->flags & ACC_INTERFACE) {
		pmptr = vftbl->interfacetable[-(m->clazz->index)];
		mptr  = pmptr[(m - m->clazz->methods)];
	}
	else {
		mptr = vftbl->table[m->vftblindex];
	}

	/* and now get the codeinfo pointer from the first data segment slot */

	resm = code_get_methodinfo_for_pv(mptr);

	return resm;
}


/* method_get_parametercount **************************************************

   Use the descriptor of a method to determine the number of parameters
   of the method. The this pointer of non-static methods is not counted.

   IN:
       m........the method of which the parameters should be counted

   RETURN VALUE:
       The parameter count.

*******************************************************************************/

int32_t method_get_parametercount(methodinfo *m)
{
	methoddesc *md;             /* method descriptor of m   */
	int32_t     paramcount = 0; /* the parameter count of m */

	md = m->parseddesc;

	/* is the descriptor fully parsed? */

	md->params_from_paramtypes(m->flags);

	paramcount = md->paramcount;

	/* skip `this' pointer */

	if (!(m->flags & ACC_STATIC)) {
		--paramcount;
	}

	return paramcount;
}


/* method_get_parametertypearray ***********************************************

   Use the descriptor of a method to generate a java.lang.Class array
   which contains the classes of the parametertypes of the method.

   This function is called by java.lang.reflect.{Constructor,Method}.

*******************************************************************************/

java_handle_objectarray_t *method_get_parametertypearray(methodinfo *m)
{
	methoddesc* md;
	typedesc*   paramtypes;
	int32_t     paramcount;
	int32_t     i;
	classinfo*  c;

	md = m->parseddesc;

	/* is the descriptor fully parsed? */

	md->params_from_paramtypes(m->flags);

	paramtypes = md->paramtypes;
	paramcount = md->paramcount;

	/* skip `this' pointer */

	if (!(m->flags & ACC_STATIC)) {
		paramtypes++;
		paramcount--;
	}

	/* create class-array */

	ClassArray ca(paramcount);

	if (ca.is_null())
		return NULL;

    /* get classes */

	for (i = 0; i < paramcount; i++) {
		if (!resolve_class_from_typedesc(&paramtypes[i], true, false, &c))
			return NULL;

		ca.set_element(i, c);
	}

	return ca.get_handle();
}


/* method_get_exceptionarray ***************************************************

   Get the exceptions which can be thrown by a method.

*******************************************************************************/

java_handle_objectarray_t *method_get_exceptionarray(methodinfo *m)
{
	classinfo* c;
	s4         i;

	/* create class-array */

	ClassArray ca(m->thrownexceptionscount);

	if (ca.is_null())
		return NULL;

	/* iterate over all exceptions and store the class in the array */

	for (i = 0; i < m->thrownexceptionscount; i++) {
		c = resolve_classref_or_classinfo_eager(m->thrownexceptions[i], true);

		if (c == NULL)
			return NULL;

		ca.set_element(i, c);
	}

	return ca.get_handle();
}


/* method_returntype_get *******************************************************

   Get the return type of the method.

*******************************************************************************/

classinfo *method_returntype_get(methodinfo *m)
{
	typedesc  *td;
	classinfo *c;

	td = &(m->parseddesc->returntype);

	if (!resolve_class_from_typedesc(td, true, false, &c))
		return NULL;

	return c;
}


/* method_count_implementations ************************************************

   Count the implementations of a method in a class cone (a class and all its
   subclasses.)

   IN:
       m................the method to count
	   c................class at which to start the counting (this class and
	                    all its subclasses will be searched)

   OUT:
       *found...........if found != NULL, *found receives the method
	                    implementation that was found. This value is only
						meaningful if the return value is 1.

   RETURN VALUE:
       the number of implementations found

*******************************************************************************/

s4 method_count_implementations(methodinfo *m, classinfo *c, methodinfo **found)
{
	s4          count;
	methodinfo *mp;
	methodinfo *mend;
	classinfo  *child;

	count = 0;

	mp = c->methods;
	mend = mp + c->methodscount;

	for (; mp < mend; ++mp) {
		if (method_canoverwrite(mp, m)) {
			if (found)
				*found = mp;
			count++;
			break;
		}
	}

	for (child = c->sub; child != NULL; child = child->nextsub) {
		count += method_count_implementations(m, child, found);
	}

	return count;
}


/* method_get_annotations ******************************************************

   Get a methods' unparsed annotations in a byte array.

   IN:
       m........the method of which the annotations should be returned

   RETURN VALUE:
       The unparsed annotations in a byte array (or NULL if there aren't any).

*******************************************************************************/

java_handle_bytearray_t *method_get_annotations(methodinfo *m)
{
#if defined(ENABLE_ANNOTATIONS)
	classinfo     *c;                  /* methods' declaring class          */
	int            slot;               /* methods' slot                     */
	java_handle_t *method_annotations; /* all methods' unparsed annotations */
	                                   /* of the declaring class            */

	c    = m->clazz;
	slot = m - c->methods;

	LLNI_classinfo_field_get(c, method_annotations, method_annotations);

	ObjectArray oa((java_handle_objectarray_t*) method_annotations);

	/* the method_annotations array might be shorter then the method
	 * count if the methods above a certain index have no annotations.
	 */	
	if (method_annotations != NULL && oa.get_length() > slot) {
		return (java_handle_bytearray_t*) oa.get_element(slot);
	} else {
		return NULL;
	}
#else
	return NULL;
#endif
}


/* method_get_parameterannotations ********************************************

   Get a methods' unparsed parameter annotations in an array of byte
   arrays.

   IN:
       m........the method of which the parameter annotations should be
	            returned

   RETURN VALUE:
       The unparsed parameter annotations in a byte array (or NULL if
	   there aren't any).

*******************************************************************************/

java_handle_bytearray_t *method_get_parameterannotations(methodinfo *m)
{
#if defined(ENABLE_ANNOTATIONS)
	classinfo     *c;                           /* methods' declaring class */
	int            slot;                        /* methods' slot            */
	java_handle_t *method_parameterannotations; /* all methods' unparsed    */
	                                            /* parameter annotations of */
	                                            /* the declaring class      */

	c    = m->clazz;
	slot = m - c->methods;

	LLNI_classinfo_field_get(
		c, method_parameterannotations, method_parameterannotations);

	ObjectArray oa((java_handle_objectarray_t*) method_parameterannotations);

	/* the method_annotations array might be shorter then the method
	 * count if the methods above a certain index have no annotations.
	 */	
	if (method_parameterannotations != NULL && oa.get_length() > slot) {
		return (java_handle_bytearray_t*) oa.get_element(slot);
	} else {
		return NULL;
	}
#else
	return NULL;
#endif
}


/* method_get_annotationdefault ***********************************************

   Get a methods' unparsed annotation default value in a byte array.
   
   IN:
       m........the method of which the annotation default value should be
	            returned

   RETURN VALUE:
       The unparsed annotation default value in a byte array (or NULL if
	   there isn't one).

*******************************************************************************/

java_handle_bytearray_t *method_get_annotationdefault(methodinfo *m)
{
#if defined(ENABLE_ANNOTATIONS)
	classinfo     *c;                         /* methods' declaring class     */
	int            slot;                      /* methods' slot                */
	java_handle_t *method_annotationdefaults; /* all methods' unparsed        */
	                                          /* annotation default values of */
	                                          /* the declaring class          */

	c    = m->clazz;
	slot = m - c->methods;

	LLNI_classinfo_field_get(
		c, method_annotationdefaults, method_annotationdefaults);

	ObjectArray oa((java_handle_objectarray_t*) method_annotationdefaults);

	/* the method_annotations array might be shorter then the method
	 * count if the methods above a certain index have no annotations.
	 */
	if (method_annotationdefaults != NULL && oa.get_length() > slot) {
		return (java_handle_bytearray_t*) oa.get_element(slot);
	} else {
		return NULL;
	}
#else
	return NULL;
#endif
}


/* method_add_to_worklist ******************************************************

   Add the method to the given worklist. If the method already occurs in
   the worklist, the worklist remains unchanged.

*******************************************************************************/

static void method_add_to_worklist(methodinfo *m, method_worklist **wl)
{
	method_worklist *wi;

	for (wi = *wl; wi != NULL; wi = wi->next)
		if (wi->m == m)
			return;

	wi = NEW(method_worklist);
	wi->next = *wl;
	wi->m = m;

	*wl = wi;
}


/* method_add_assumption_monomorphic *******************************************

   Record the assumption that the method is monomorphic.

   IN:
      m.................the method
	  caller............the caller making the assumption

*******************************************************************************/

void method_add_assumption_monomorphic(methodinfo *m, methodinfo *caller)
{
	method_assumption *as;

	/* XXX LOCKING FOR THIS FUNCTION? */

	/* check if we already have registered this assumption */

	for (as = m->assumptions; as != NULL; as = as->next) {
		if (as->context == caller)
			return;
	}

	/* register the assumption */

	as = NEW(method_assumption);
	as->next = m->assumptions;
	as->context = caller;

	m->assumptions = as;
}

/* method_break_assumption_monomorphic *****************************************

   Break the assumption that this method is monomorphic. All callers that
   have registered this assumption are added to the worklist.

   IN:
      m.................the method
	  wl................worklist where to add invalidated callers

*******************************************************************************/

void method_break_assumption_monomorphic(methodinfo *m, method_worklist **wl)
{
	method_assumption *as;

	/* XXX LOCKING FOR THIS FUNCTION? */

	for (as = m->assumptions; as != NULL; as = as->next) {
		INLINELOG(
			printf("ASSUMPTION BROKEN (monomorphism): ");
			method_print(m);
			printf(" in ");
			method_println(as->context);
		);

		method_add_to_worklist(as->context, wl);

#if defined(ENABLE_TLH) && 0
		/* XXX hack */
		method_assumption *as2;
		as2 = m->assumptions;
		m->assumptions = NULL;
		method_break_assumption_monomorphic(as->context, wl);
		/*
		assert(m->assumptions == NULL);
		m->assumptions = as2;*/
#endif

	}
}

/* method_printflags ***********************************************************

   Prints the flags of a method to stdout like.

*******************************************************************************/

#if !defined(NDEBUG)
void method_printflags(methodinfo *m)
{
	if (m == NULL) {
		printf("NULL");
		return;
	}

	if (m->flags & ACC_PUBLIC)             printf(" PUBLIC");
	if (m->flags & ACC_PRIVATE)            printf(" PRIVATE");
	if (m->flags & ACC_PROTECTED)          printf(" PROTECTED");
	if (m->flags & ACC_STATIC)             printf(" STATIC");
	if (m->flags & ACC_FINAL)              printf(" FINAL");
	if (m->flags & ACC_SYNCHRONIZED)       printf(" SYNCHRONIZED");
	if (m->flags & ACC_VOLATILE)           printf(" VOLATILE");
	if (m->flags & ACC_TRANSIENT)          printf(" TRANSIENT");
	if (m->flags & ACC_NATIVE)             printf(" NATIVE");
	if (m->flags & ACC_INTERFACE)          printf(" INTERFACE");
	if (m->flags & ACC_ABSTRACT)           printf(" ABSTRACT");
	if (m->flags & ACC_METHOD_BUILTIN)     printf(" (builtin)");
	if (m->flags & ACC_METHOD_MONOMORPHIC) printf(" (mono)");
	if (m->flags & ACC_METHOD_IMPLEMENTED) printf(" (impl)");
}
#endif /* !defined(NDEBUG) */


/* method_print ****************************************************************

   Prints a method to stdout like:

   java.lang.Object.<init>()V

*******************************************************************************/

#if !defined(NDEBUG)
void method_print(methodinfo *m)
{
	if (m == NULL) {
		printf("NULL");
		return;
	}

	if (m->clazz != NULL)
		utf_display_printable_ascii_classname(m->clazz->name);
	else
		printf("NULL");
	printf(".");
	utf_display_printable_ascii(m->name);
	utf_display_printable_ascii(m->descriptor);

	method_printflags(m);
}
#endif /* !defined(NDEBUG) */


/* method_println **************************************************************

   Prints a method plus new line to stdout like:

   java.lang.Object.<init>()V

*******************************************************************************/

#if !defined(NDEBUG)
void method_println(methodinfo *m)
{
	if (opt_debugcolor) printf("\033[31m");	/* red */
	method_print(m);
	if (opt_debugcolor) printf("\033[m");
	printf("\n");
}
#endif /* !defined(NDEBUG) */


/* method_methodref_print ******************************************************

   Prints a method reference to stdout.

*******************************************************************************/

#if !defined(NDEBUG)
void method_methodref_print(constant_FMIref *mr)
{
	if (!mr) {
		printf("(constant_FMIref *)NULL");
		return;
	}

	if (mr->is_resolved()) {
		printf("<method> ");
		method_print(mr->p.method);
	}
	else {
		printf("<methodref> ");
		utf_display_printable_ascii_classname(mr->p.classref->name);
		printf(".");
		utf_display_printable_ascii(mr->name);
		utf_display_printable_ascii(mr->descriptor);
	}
}
#endif /* !defined(NDEBUG) */


/* method_methodref_println ****************************************************

   Prints a method reference to stdout, followed by a newline.

*******************************************************************************/

#if !defined(NDEBUG)
void method_methodref_println(constant_FMIref *mr)
{
	method_methodref_print(mr);
	printf("\n");
}
#endif /* !defined(NDEBUG) */


namespace cacao {
namespace {

inline OStream& method_printflags_OS(OStream &OS, const struct methodinfo &m)
{
	if (m.flags & ACC_PUBLIC)             OS << " PUBLIC" ;
	if (m.flags & ACC_PRIVATE)            OS << " PRIVATE" ;
	if (m.flags & ACC_PROTECTED)          OS << " PROTECTED" ;
	if (m.flags & ACC_STATIC)             OS << " STATIC" ;
	if (m.flags & ACC_FINAL)              OS << " FINAL" ;
	if (m.flags & ACC_SYNCHRONIZED)       OS << " SYNCHRONIZED" ;
	if (m.flags & ACC_VOLATILE)           OS << " VOLATILE" ;
	if (m.flags & ACC_TRANSIENT)          OS << " TRANSIENT" ;
	if (m.flags & ACC_NATIVE)             OS << " NATIVE" ;
	if (m.flags & ACC_INTERFACE)          OS << " INTERFACE" ;
	if (m.flags & ACC_ABSTRACT)           OS << " ABSTRACT" ;
	if (m.flags & ACC_METHOD_BUILTIN)     OS << " (builtin)" ;
	if (m.flags & ACC_METHOD_MONOMORPHIC) OS << " (mono)" ;
	if (m.flags & ACC_METHOD_IMPLEMENTED) OS << " (impl)" ;
	return OS;
}
} // end anonymous namespace

OStream& operator<<(OStream &OS, const struct methodinfo &m)
{
	OS << (Utf8String)m.clazz->name << "."
	   << (Utf8String)m.name
	   << (Utf8String)m.descriptor;
	method_printflags_OS(OS,m);
	return OS;
}

} // end namespace cacao

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
