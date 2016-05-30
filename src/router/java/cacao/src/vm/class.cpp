/* src/vm/class.cpp - class related functions

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

#include "vm/class.hpp"

#include "config.h"

#include <assert.h>
#include <inttypes.h>               // for PRId64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch.hpp"

#include "mm/memory.hpp"

#include "native/llni.hpp"

#include "threads/lock.hpp"
#include "threads/lockword.hpp"         // for Lockword
#include "threads/mutex.hpp"
#include "threads/thread.hpp"           // for thread_get_current, etc

#include "toolbox/OStream.hpp"          // for OStream
#include "toolbox/hashtable.hpp"        // for hashtable
#include "toolbox/logging.hpp"

#include "vm/annotation.hpp"
#include "vm/array.hpp"
#include "vm/class.hpp"
#include "vm/classcache.hpp"
#include "vm/exceptions.hpp"
#include "vm/field.hpp"                 // for fieldinfo, etc
#include "vm/global.hpp"
#include "vm/globals.hpp"
#include "vm/javaobjects.hpp"
#include "vm/linker.hpp"
#include "vm/loader.hpp"
#include "vm/method.hpp"                // for methodinfo, etc
#include "vm/options.hpp"
#include "vm/primitive.hpp"             // for Primitive
#include "vm/resolve.hpp"
#include "vm/statistics.hpp"
#include "vm/string.hpp"                // for JavaString
#include "vm/suck.hpp"
#include "vm/string.hpp"
#include "vm/types.hpp"
#include "vm/utf8.hpp"

#include "vm/jit/builtin.hpp"

STAT_DECLARE_GROUP(info_struct_stat)
STAT_DECLARE_VAR(int,size_classinfo,0)

using namespace cacao;

/**
 * Returns the classname of the class, where slashes ('/') are
 * replaced by dots ('.').
 *
 * @param c class to get name of
 * @return classname
 */
extern java_handle_t* class_get_classname(classinfo* c)
{
	return JavaString::from_utf8_slash_to_dot(c->name);
}


/* class_set_packagename *******************************************************

   Derive the package name from the class name and store it in the
   struct.

   For classes in the unnamed package, the package name is set to
   NULL.

*******************************************************************************/

void class_set_packagename(classinfo *c)
{
	Utf8String name = c->name;

	const char *p     = name.end() - 1;
	const char *start = name.begin();

	if (name[0] == '[') {
		/* Set packagename of arrays to the element's package. */

		for (; *start == '['; start++);

		/* Skip the 'L' in arrays of references. */

		if (*start == 'L')
			start++;
	}

	/* Search for last '/'. */

	for (; (p > start) && (*p != '/'); --p);

	/* If we found a '/' we set the package name.  Otherwise we set the
	   packagename to NULL. */

	if (p > start)
		c->packagename = Utf8String::from_utf8_slash_to_dot(start, p - start);
	else
		c->packagename = NULL;
}


/* class_create_classinfo ******************************************************

   Create a new classinfo struct. The class name is set to the given utf string,
   most other fields are initialized to zero.

   Note: classname may be NULL. In this case a not-yet-named classinfo is
         created. The name must be filled in later and class_set_packagename
		 must be called after that.

*******************************************************************************/

classinfo *class_create_classinfo(Utf8String classname)
{
	classinfo *c;

	STATISTICS(size_classinfo += sizeof(classinfo));

	/* we use a safe name for temporarily unnamed classes */

	if (classname == NULL)
		classname = utf8::not_named_yet;

#if !defined(NDEBUG)
	if (initverbose)
		log_message_utf("Creating class: ", classname);
#endif

#if !defined(ENABLE_GC_BOEHM)
	c = (classinfo *) heap_alloc_uncollectable(sizeof(classinfo));
	/*c = NEW(classinfo);
	MZERO(c, classinfo, 1);*/
#else
	c = GCNEW_UNCOLLECTABLE(classinfo, 1);
	/* GCNEW_UNCOLLECTABLE clears the allocated memory */
#endif

	c->name = classname;

	/* Set the header.vftbl of all loaded classes to the one of
       java.lang.Class, so Java code can use a class as object. */

	if (class_java_lang_Class != NULL)
		if (class_java_lang_Class->vftbl != NULL)
			c->object.header.vftbl = class_java_lang_Class->vftbl;

#if defined(ENABLE_JAVASE)
	/* check if the class is a reference class and flag it */

	if (classname == utf8::java_lang_ref_SoftReference) {
		c->flags |= ACC_CLASS_REFERENCE_SOFT;
	}
	else if (classname == utf8::java_lang_ref_WeakReference) {
		c->flags |= ACC_CLASS_REFERENCE_WEAK;
	}
	else if (classname == utf8::java_lang_ref_PhantomReference) {
		c->flags |= ACC_CLASS_REFERENCE_PHANTOM;
	}
#endif

	if (classname != utf8::not_named_yet)
		class_set_packagename(c);

	Lockword(c->object.header.lockword).init();

	return c;
}


/* class_postset_header_vftbl **************************************************

   Set the header.vftbl of all classes created before java.lang.Class
   was linked.  This is necessary that Java code can use a class as
   object.

*******************************************************************************/

void class_postset_header_vftbl(void)
{
	classinfo *c;
	u4 slot;
	classcache_name_entry *nmen;
	classcache_class_entry *clsen;

	assert(class_java_lang_Class);

	for (slot = 0; slot < hashtable_classcache.size; slot++) {
		nmen = (classcache_name_entry *) hashtable_classcache.ptr[slot];

		for (; nmen; nmen = nmen->hashlink) {
			/* iterate over all class entries */

			for (clsen = nmen->classes; clsen; clsen = clsen->next) {
				c = clsen->classobj;

				/* now set the the vftbl */

				if (c->object.header.vftbl == NULL)
					c->object.header.vftbl = class_java_lang_Class->vftbl;
			}
		}
	}
}

/* class_define ****************************************************************

   Calls the loader and defines a class in the VM.

*******************************************************************************/

classinfo *class_define(Utf8String name, classloader_t *cl, int32_t length, uint8_t *data, java_handle_t *pd)
{
	if (name != NULL) {
		/* check if this class has already been defined */

		classinfo *c = classcache_lookup_defined_or_initiated(cl, name);

		if (c != NULL) {
			exceptions_throw_linkageerror("duplicate class definition: ", c);
			return NULL;
		}
	}

	/* create a new classinfo struct */

	classinfo *c = class_create_classinfo(name);

#if defined(ENABLE_STATISTICS)
	/* measure time */

	if (opt_getloadingtime)
		loadingtime_start();
#endif

	/* preset the defining classloader */

	c->classloader = cl;

	/* build a classbuffer with the given data */

	ClassBuffer cb(c, data, length);

	/* load the class from this buffer */

	classinfo *r = load_class_from_classbuffer(cb);

#if defined(ENABLE_STATISTICS)
	/* measure time */

	if (opt_getloadingtime)
		loadingtime_stop();
#endif

	if (r == NULL) {
		/* If return value is NULL, we had a problem and the class is
		   not loaded.  Now free the allocated memory, otherwise we
		   could run into a DOS. */

		class_free(c);

		return NULL;
	}

#if defined(ENABLE_JAVASE)
# if defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
	/* Store the protection domain. */

	c->protectiondomain = pd;
# endif
#endif

	/* Store the newly defined class in the class cache. This call
	   also checks whether a class of the same name has already been
	   defined by the same defining loader, and if so, replaces the
	   newly created class by the one defined earlier. */

	/* Important: The classinfo given to classcache_store must be
	              fully prepared because another thread may return
	              this pointer after the lookup at to top of this
	              function directly after the class cache lock has
	              been released. */

	c = classcache_store(cl, c, true);

	return c;
}


/* class_load_attribute_sourcefile *********************************************

   SourceFile_attribute {
       u2 attribute_name_index;
       u4 attribute_length;
	   u2 sourcefile_index;
   }

*******************************************************************************/

static bool class_load_attribute_sourcefile(ClassBuffer& cb)
{
	/* get classinfo */

	classinfo *c = cb.get_class();

	/* check buffer size */

	if (!cb.check_size(4 + 2))
		return false;

	/* check attribute length */

	u4 attribute_length = cb.read_u4();

	if (attribute_length != 2) {
		exceptions_throw_classformaterror(c, "Wrong size for VALUE attribute");
		return false;
	}

	/* there can be no more than one SourceFile attribute */

	if (c->sourcefile != NULL) {
		exceptions_throw_classformaterror(c, "Multiple SourceFile attributes");
		return false;
	}

	/* get sourcefile */

	u2         sourcefile_index = cb.read_u2();
	Utf8String sourcefile       = (utf*) class_getconstant(c, sourcefile_index, CONSTANT_Utf8);

	if (sourcefile == NULL)
		return false;

	/* store sourcefile */

	c->sourcefile = sourcefile;

	return true;
}


/* class_load_attribute_enclosingmethod ****************************************

   EnclosingMethod_attribute {
       u2 attribute_name_index;
       u4 attribute_length;
	   u2 class_index;
	   u2 method_index;
   }

*******************************************************************************/

#if defined(ENABLE_JAVASE)
static bool class_load_attribute_enclosingmethod(ClassBuffer& cb) {
	classref_or_classinfo  cr;

	/* get classinfo */

	classinfo *c = cb.get_class();

	/* check buffer size */

	if (!cb.check_size(4 + 2 + 2))
		return false;

	/* check attribute length */

	u4 attribute_length = cb.read_u4();

	if (attribute_length != 4) {
		exceptions_throw_classformaterror(c, "Wrong size for VALUE attribute");
		return false;
	}

	/* there can be no more than one EnclosingMethod attribute */

	if (c->enclosingmethod != NULL) {
		exceptions_throw_classformaterror(c, "Multiple EnclosingMethod attributes");
		return false;
	}

	/* get class index */

	u2 class_index = cb.read_u2();
	cr.ref         = (constant_classref*) innerclass_getconstant(c, class_index, CONSTANT_Class);

	/* get method index */

	u2                    method_index = cb.read_u2();
	constant_nameandtype *cn           = (constant_nameandtype*) innerclass_getconstant(c, method_index, CONSTANT_NameAndType);

	/* store info in classinfo */

	c->enclosingclass.any = cr.any;
	c->enclosingmethod    = cn;

	return true;
}
#endif /* defined(ENABLE_JAVASE) */


/* class_load_attributes *******************************************************

   Read attributes from ClassFile.

   attribute_info {
       u2 attribute_name_index;
       u4 attribute_length;
       u1 info[attribute_length];
   }

   InnerClasses_attribute {
       u2 attribute_name_index;
       u4 attribute_length;
   }

*******************************************************************************/

bool class_load_attributes(ClassBuffer& cb)
{
	classref_or_classinfo outer, inner;

	classinfo *c = cb.get_class();

	/* get attributes count */

	if (!cb.check_size(2))
		return false;

	uint16_t attributes_count = cb.read_u2();

	for (int i = 0; i < attributes_count; i++) {
		/* get attribute name */

		if (!cb.check_size(2))
			return false;

		uint16_t   attribute_name_index = cb.read_u2();
		Utf8String attribute_name       = (utf*) class_getconstant(c, attribute_name_index, CONSTANT_Utf8);

		if (attribute_name == NULL)
			return false;

		if (attribute_name == utf8::InnerClasses) {
			/* InnerClasses */

			if (c->innerclass != NULL) {
				exceptions_throw_classformaterror(c, "Multiple InnerClasses attributes");
				return false;
			}

			if (!cb.check_size(4 + 2))
				return false;

			/* skip attribute length */
			cb.read_u4();

			/* number of records */
			c->innerclasscount = cb.read_u2();

			if (!cb.check_size((2 + 2 + 2 + 2) * c->innerclasscount))
				return false;

			/* allocate memory for innerclass structure */
			c->innerclass = MNEW(innerclassinfo, c->innerclasscount);

			for (int j = 0; j < c->innerclasscount; j++) {
				/* The innerclass structure contains a class with an encoded
				   name, its defining scope, its simple name and a bitmask of
				   the access flags. */

				innerclassinfo *info = c->innerclass + j;

				inner.ref         = (constant_classref*) innerclass_getconstant(c, cb.read_u2(), CONSTANT_Class);
				outer.ref         = (constant_classref*) innerclass_getconstant(c, cb.read_u2(), CONSTANT_Class);
				Utf8String  name  = (utf*) innerclass_getconstant(c, cb.read_u2(), CONSTANT_Utf8);
				uint16_t    flags = cb.read_u2();

				/* If the current inner-class is the currently loaded
				   class check for some special flags. */

				if (inner.ref->name == c->name) {
					/* If an inner-class is not a member, its
					   outer-class is NULL. */

					if (outer.ref != NULL) {
						c->flags |= ACC_CLASS_MEMBER;

						/* A member class doesn't have an
						   EnclosingMethod attribute, so set the
						   enclosing-class to be the same as the
						   declaring-class. */

						c->declaringclass = outer;
						c->enclosingclass = outer;
					}

					/* If an inner-class is anonymous, its name is
					   NULL. */

					if (name == NULL)
						c->flags |= ACC_CLASS_ANONYMOUS;
				}

				info->inner_class = inner;
				info->outer_class = outer;
				info->name        = name;
				info->flags       = flags;
			}
		}
		else if (attribute_name == utf8::SourceFile) {
			/* SourceFile */

			if (!class_load_attribute_sourcefile(cb))
				return false;
		}
#if defined(ENABLE_JAVASE)
		else if (attribute_name == utf8::EnclosingMethod) {
			/* EnclosingMethod */

			if (!class_load_attribute_enclosingmethod(cb))
				return false;
		}
		else if (attribute_name == utf8::Signature) {
			/* Signature */

			// TODO: change classinfo.signature to Utf8String
			//       and use it directly

			Utf8String signature = c->signature;

			if (!loader_load_attribute_signature(cb, signature)) {
				return false;
			}

			c->signature = signature;
		}
#endif

#if defined(ENABLE_ANNOTATIONS)
		else if (attribute_name == utf8::RuntimeVisibleAnnotations) {
			/* RuntimeVisibleAnnotations */
			if (!annotation_load_class_attribute_runtimevisibleannotations(cb))
				return false;
		}
		else if (attribute_name == utf8::RuntimeInvisibleAnnotations) {
			/* RuntimeInvisibleAnnotations */
			if (!annotation_load_class_attribute_runtimeinvisibleannotations(cb))
				return false;
		}
#endif

		else {
			/* unknown attribute */

			if (!loader_skip_attribute_body(cb))
				return false;
		}
	}

	return true;
}


/* class_freepool **************************************************************

	Frees all resources used by this classes Constant Pool.

*******************************************************************************/

static void class_freecpool(classinfo *c)
{
	if (c->cptags && c->cpinfos) {
		for (int32_t idx = 0; idx < c->cpcount; idx++) {
			ConstantPoolTag  tag  = (ConstantPoolTag) c->cptags[idx];
			void            *info = c->cpinfos[idx];

			if (info != NULL) {
				switch (tag) {
				case CONSTANT_Class:
				case CONSTANT_ClassName:
				case CONSTANT_String:
				case CONSTANT_Utf8:
					// these live forever
					break;

				case CONSTANT_Fieldref:
				case CONSTANT_Methodref:
				case CONSTANT_InterfaceMethodref:
					FREE(info, constant_FMIref);
					break;
				case CONSTANT_Integer:
					FREE(info, int32_t);
					break;
				case CONSTANT_Float:
					FREE(info, float);
					break;
				case CONSTANT_Long:
					FREE(info, int64_t);
					break;
				case CONSTANT_Double:
					FREE(info, double);
					break;
				case CONSTANT_NameAndType:
					FREE(info, constant_nameandtype);
					break;

				case CONSTANT_MethodType:
					delete ((constant_MethodType*) info);
					break;
				case CONSTANT_MethodHandle:
					delete ((constant_MethodHandle*) info);
					break;
				case CONSTANT_InvokeDynamic:
					delete ((constant_InvokeDynamic*) info);
					break;

				case CONSTANT_UNUSED:
					assert(info == 0);
					break;
				}
			}
		}
	}

	if (c->cptags)
		MFREE(c->cptags, u1, c->cpcount);

	if (c->cpinfos)
		MFREE(c->cpinfos, void*, c->cpcount);
}


/* class_getconstant ***********************************************************

   Retrieves the value at position 'pos' of the constantpool of a
   class. If the type of the value is other than 'ctype', an error is
   thrown.

*******************************************************************************/

void* class_getconstant(classinfo *c, u4 pos, ConstantPoolTag ctype)
{
	// check index and type of constantpool entry
	// (pos == 0 is caught by type comparison)

	if ((((int32_t)pos) >= c->cpcount) || (c->cptags[pos] != ctype)) {
		// this is the slow path,
		// we can afford to repeat the separate checks for a better error message

		if ((pos == 0) || (((int32_t)pos) >= c->cpcount)) {
			exceptions_throw_classformaterror(c, "Illegal constant pool index: %u", pos);
		} else if (c->cptags[pos] != ctype) {
			exceptions_throw_classformaterror(c, "Illegal constant pool type %u (expected %u)", ctype, c->cptags[pos]);
		}

		assert(exceptions_get_exception());
		return NULL;
	}

	return c->cpinfos[pos];
}


/* innerclass_getconstant ******************************************************

   Like class_getconstant, but if cptags is ZERO, null is returned.

*******************************************************************************/

void* innerclass_getconstant(classinfo *c, u4 pos, ConstantPoolTag ctype)
{
	/* invalid position in constantpool */

	if (((int32_t)pos) >= c->cpcount) {
		exceptions_throw_classformaterror(c, "Illegal constant pool index: %u", pos);
		return NULL;
	}

	/* constantpool entry of type 0 */

	if (c->cptags[pos] == 0)
		return NULL;

	/* check type of constantpool entry */

	if (c->cptags[pos] != ctype) {
		exceptions_throw_classformaterror(c, "Illegal constant pool type %u (expected %u)", ctype, c->cptags[pos]);
		return NULL;
	}

	return c->cpinfos[pos];
}


/* class_free ******************************************************************

   Frees all resources used by the class.

*******************************************************************************/

void class_free(classinfo *c)
{
	class_freecpool(c);

	if (c->interfaces != NULL)
		MFREE(c->interfaces, classinfo*, c->interfacescount);

	if (c->fields) {
		for (int32_t i = 0; i < c->fieldscount; i++)
			field_free(&(c->fields[i]));
		MFREE(c->fields, fieldinfo, c->fieldscount);
	}

	if (c->methods) {
		for (int32_t i = 0; i < c->methodscount; i++)
			method_free(&(c->methods[i]));
		MFREE(c->methods, methodinfo, c->methodscount);
	}

	if (vftbl_t *v = c->vftbl) {
		if (v->arraydesc)
			mem_free(v->arraydesc,sizeof(arraydescriptor));

		for (int32_t i = 0; i < v->interfacetablelength; i++) {
			MFREE(v->interfacetable[-i], methodptr, v->interfacevftbllength[i]);
		}
		MFREE(v->interfacevftbllength, s4, v->interfacetablelength);

		int32_t i = sizeof(vftbl_t)
		          + sizeof(methodptr)  * (v->vftbllength - 1)
		          + sizeof(methodptr*) * (v->interfacetablelength - (v->interfacetablelength > 0));
		methodptr *m = ((methodptr*) v) - (v->interfacetablelength - 1) * (v->interfacetablelength > 1);
		mem_free(m, i);
	}

	if (c->innerclass)
		MFREE(c->innerclass, innerclassinfo, c->innerclasscount);

	/*	if (c->classvftbl)
		mem_free(c->header.vftbl, sizeof(vftbl) + sizeof(methodptr)*(c->vftbl->vftbllength-1)); */

/*  	GCFREE(c); */
}


/* get_array_class *************************************************************

   Returns the array class with the given name for the given
   classloader, or NULL if an exception occurred.

   Note: This function does eager loading.

*******************************************************************************/

static classinfo *get_array_class(Utf8String name,classloader_t *initloader,
											      classloader_t *defloader,bool link)
{
	classinfo *c;

	/* lookup this class in the classcache */
	c = classcache_lookup(initloader,name);
	if (!c)
		c = classcache_lookup_defined(defloader,name);

	if (!c) {
		/* we have to create it */
		c = class_create_classinfo(name);
		c = load_newly_created_array(c,initloader);
		if (c == NULL)
			return NULL;
	}

	assert(c);
	assert(c->state & CLASS_LOADED);
	assert(c->classloader == defloader);

	if (link && !(c->state & CLASS_LINKED))
		if (!link_class(c))
			return NULL;

	assert(!link || (c->state & CLASS_LINKED));

	return c;
}


/* class_array_of **************************************************************

   Returns an array class with the given component class. The array
   class is dynamically created if neccessary.

*******************************************************************************/

classinfo *class_array_of(classinfo *component, bool link)
{
	classloader_t     *cl;
	s4                 namelen;
	char              *namebuf;
	Utf8String         u;
	classinfo         *c;

	Utf8String component_name = component->name;

	cl = component->classloader;

    /* Assemble the array class name */
    namelen = component_name.size();

    if (component_name[0] == '[') {
        /* the component is itself an array */
        namebuf = MNEW(char, namelen + 1);
        namebuf[0] = '[';
        MCOPY(namebuf + 1, component_name.begin(), char, namelen);
        namelen++;
    }
	else {
        /* the component is a non-array class */
        namebuf = MNEW(char, namelen + 3);
        namebuf[0] = '[';
        namebuf[1] = 'L';
        MCOPY(namebuf + 2, component_name.begin(), char, namelen);
        namebuf[2 + namelen] = ';';
        namelen += 3;
    }

	u = Utf8String::from_utf8(namebuf, namelen);

	MFREE(namebuf, char, namelen);

	c = get_array_class(u, cl, cl, link);

	return c;
}


/* class_multiarray_of *********************************************************

   Returns an array class with the given dimension and element class.
   The array class is dynamically created if neccessary.

*******************************************************************************/

classinfo *class_multiarray_of(s4 dim, classinfo *element, bool link)
{
    s4 namelen;
    char *namebuf;
	classinfo *c;

	Utf8String element_name = element->name;

	if (dim < 1) {
		log_text("Invalid array dimension requested");
		assert(0);
	}

    /* Assemble the array class name */
    namelen = element_name.size();

    if (element_name[0] == '[') {
        /* the element is itself an array */
        namebuf = MNEW(char, namelen + dim);
        memcpy(namebuf + dim, element_name.begin(), namelen);
        namelen += dim;
    }
    else {
        /* the element is a non-array class */
        namebuf = MNEW(char, namelen + 2 + dim);
        namebuf[dim] = 'L';
        memcpy(namebuf + dim + 1, element_name.begin(), namelen);
        namelen += (2 + dim);
        namebuf[namelen - 1] = ';';
    }
	memset(namebuf, '[', dim);

	Utf8String u = Utf8String::from_utf8(namebuf, namelen);

	MFREE(namebuf, char, namelen);

	c = get_array_class(u,
	                    element->classloader,
	                    element->classloader,
	                    link);

	return c;
}


/* class_lookup_classref *******************************************************

   Looks up the constant_classref for a given classname in the classref
   tables of a class.

   IN:
       cls..............the class containing the reference
	   name.............the name of the class refered to

    RETURN VALUE:
	   a pointer to a constant_classref, or
	   NULL if the reference was not found

*******************************************************************************/

constant_classref *class_lookup_classref(classinfo *cls, Utf8String name)
{
	constant_classref *ref;
	extra_classref *xref;
	int count;

	assert(cls);
	assert(name);
	assert(!cls->classrefcount || cls->classrefs);

	/* first search the main classref table */
	count = cls->classrefcount;
	ref = cls->classrefs;
	for (; count; --count, ++ref)
		if (ref->name == name)
			return ref;

	/* next try the list of extra classrefs */
	for (xref = cls->extclassrefs; xref; xref = xref->next) {
		if (xref->classref.name == name)
			return &(xref->classref);
	}

	/* not found */
	return NULL;
}


/* class_get_classref **********************************************************

   Returns the constant_classref for a given classname.

   IN:
       cls..............the class containing the reference
	   name.............the name of the class refered to

   RETURN VALUE:
       a pointer to a constant_classref (never NULL)

   NOTE:
       The given name is not checked for validity!

*******************************************************************************/

constant_classref *class_get_classref(classinfo *cls, Utf8String name)
{
	assert(cls);
	assert(name);

	if (constant_classref *ref = class_lookup_classref(cls,name))
		return ref;

	extra_classref *xref = new (MemAlloc) extra_classref(cls->extclassrefs, cls, name);

	cls->extclassrefs = xref;

	return &(xref->classref);
}


/* class_get_self_classref *****************************************************

   Returns the constant_classref to the class itself.

   IN:
       cls..............the class containing the reference

   RETURN VALUE:
       a pointer to a constant_classref (never NULL)

*******************************************************************************/

constant_classref *class_get_self_classref(classinfo *cls)
{
	/* XXX this should be done in a faster way. Maybe always make */
	/* the classref of index 0 a self reference.                  */
	return class_get_classref(cls,cls->name);
}

/* class_get_classref_multiarray_of ********************************************

   Returns an array type reference with the given dimension and element class
   reference.

   IN:
       dim..............the requested dimension
	                    dim must be in [1;255]. This is NOT checked!
	   ref..............the component class reference

   RETURN VALUE:
       a pointer to the class reference for the array type

   NOTE:
       The referer of `ref` is used as the referer for the new classref.

*******************************************************************************/

constant_classref *class_get_classref_multiarray_of(s4 dim, constant_classref *ref)
{
    s4 namelen;
    char *namebuf;
	constant_classref *cr;

	Utf8String refname = ref->name;

	assert(ref);
	assert(dim >= 1 && dim <= 255);

    /* Assemble the array class name */
    namelen = refname.size();

    if (refname[0] == '[') {
        /* the element is itself an array */
        namebuf = MNEW(char, namelen + dim);
        memcpy(namebuf + dim, refname.begin(), namelen);
        namelen += dim;
    }
    else {
        /* the element is a non-array class */
        namebuf = MNEW(char, namelen + 2 + dim);
        namebuf[dim] = 'L';
        memcpy(namebuf + dim + 1, refname.begin(), namelen);
        namelen += (2 + dim);
        namebuf[namelen - 1] = ';';
    }
	memset(namebuf, '[', dim);

	Utf8String u = Utf8String::from_utf8(namebuf, namelen);

	MFREE(namebuf, char, namelen);

    cr = class_get_classref(ref->referer, u);

	return cr;
}


/* class_get_classref_component_of *********************************************

   Returns the component classref of a given array type reference

   IN:
       ref..............the array type reference

   RETURN VALUE:
       a reference to the component class, or
	   NULL if `ref` is not an object array type reference

   NOTE:
       The referer of `ref` is used as the referer for the new classref.

*******************************************************************************/

constant_classref *class_get_classref_component_of(constant_classref *ref)
{
	assert(ref);

	Utf8String name = ref->name;
	size_t start    = 1;
	size_t end      = name.size() - 1;

	if (name[0] != '[')
		return NULL;

	if (name[1] == 'L') {
		start += 1;
		end   -= 2;
	}
	else if (name[1] != '[') {
		return NULL;
	}

    return class_get_classref(ref->referer, name.substring(start, end));
}


/* class_findmethod ************************************************************

   Searches a 'classinfo' structure for a method having the given name
   and descriptor. If descriptor is NULL, it is ignored.

*******************************************************************************/

methodinfo *class_findmethod(classinfo *c, Utf8String name, Utf8String desc)
{
	for (int32_t i = 0; i < c->methodscount; i++) {
		methodinfo *m = &(c->methods[i]);

		if ((m->name == name) && ((desc == NULL) || (m->descriptor == desc)))
			return m;
	}

	return NULL;
}


/* class_resolvemethod *********************************************************

   Searches a class and it's super classes for a method.

   Superinterfaces are *not* searched.

*******************************************************************************/

methodinfo *class_resolvemethod(classinfo *c, Utf8String name, Utf8String desc)
{
	while (c) {
		methodinfo *m = class_findmethod(c, name, desc);

		if (m)
			return m;

		/* JVM Specification bug:

		   It is important NOT to resolve special <init> and <clinit>
		   methods to super classes or interfaces; yet, this is not
		   explicited in the specification.  Section 5.4.3.3 should be
		   updated appropriately.  */

		if (name == utf8::init || name == utf8::clinit)
			return NULL;

		c = c->super;
	}

	return NULL;
}


/* class_resolveinterfacemethod_intern *****************************************

   Internally used helper function. Do not use this directly.

*******************************************************************************/

static methodinfo *class_resolveinterfacemethod_intern(classinfo *c,
													   Utf8String name, Utf8String desc)
{
	/* try to find the method in the class */

	methodinfo *m = class_findmethod(c, name, desc);

	if (m != NULL)
		return m;

	/* No method found?  Try the super interfaces. */

	for (int32_t i = 0; i < c->interfacescount; i++) {
		methodinfo *m = class_resolveinterfacemethod_intern(c->interfaces[i], name, desc);

		if (m != NULL)
			return m;
	}

	/* no method found */

	return NULL;
}


/* class_resolveclassmethod ****************************************************

   Resolves a reference from REFERER to a method with NAME and DESC in
   class C.

   If the method cannot be resolved the return value is NULL. If
   EXCEPT is true *exceptionptr is set, too.

*******************************************************************************/

methodinfo *class_resolveclassmethod(classinfo *c, Utf8String name, Utf8String desc,
									 classinfo *referer, bool throwexception)
{
	/* try class c and its superclasses */

	classinfo  *cls = c;

	methodinfo *m = class_resolvemethod(cls, name, desc);

	if (m != NULL)
		goto found;

	/* Try the super interfaces. */

	for (int32_t i = 0; i < c->interfacescount; i++) {
		methodinfo *m = class_resolveinterfacemethod_intern(c->interfaces[i], name, desc);

		if (m != NULL)
			goto found;
	}

	if (throwexception)
		exceptions_throw_nosuchmethoderror(c, name, desc);

	return NULL;

 found:
	if ((m->flags & ACC_ABSTRACT) && !(c->flags & ACC_ABSTRACT)) {
		if (throwexception)
			exceptions_throw_abstractmethoderror();

		return NULL;
	}

	/* XXX check access rights */

	return m;
}


/* class_resolveinterfacemethod ************************************************

   Resolves a reference from REFERER to a method with NAME and DESC in
   interface C.

   If the method cannot be resolved the return value is NULL. If
   EXCEPT is true *exceptionptr is set, too.

*******************************************************************************/

methodinfo *class_resolveinterfacemethod(classinfo *c, Utf8String name, Utf8String desc,
										 classinfo *referer, bool throwexception)
{
	methodinfo *mi;

	if (!(c->flags & ACC_INTERFACE)) {
		if (throwexception)
			exceptions_throw_incompatibleclasschangeerror(c, "Not an interface");

		return NULL;
	}

	mi = class_resolveinterfacemethod_intern(c, name, desc);

	if (mi != NULL)
		return mi;

	/* try class java.lang.Object */

	mi = class_findmethod(class_java_lang_Object, name, desc);

	if (mi != NULL)
		return mi;

	if (throwexception)
		exceptions_throw_nosuchmethoderror(c, name, desc);

	return NULL;
}


/* class_findfield *************************************************************

   Searches for field with specified name and type in a classinfo
   structure. If no such field is found NULL is returned.

*******************************************************************************/

fieldinfo *class_findfield(classinfo *c, Utf8String name, Utf8String desc)
{
	for (int32_t i = 0; i < c->fieldscount; i++)
		if ((c->fields[i].name == name) && (c->fields[i].descriptor == desc))
			return &(c->fields[i]);

	if (c->super != NULL)
		return class_findfield(c->super, name, desc);

	return NULL;
}


/* class_findfield_approx ******************************************************

   Searches in 'classinfo'-structure for a field with the specified
   name.

*******************************************************************************/

fieldinfo *class_findfield_by_name(classinfo* c, Utf8String name)
{
	for (int32_t i = 0; i < c->fieldscount; i++) {
		fieldinfo* f = &(c->fields[i]);

		if (f->name == name)
			return f;
	}

	// Field not found.
	exceptions_throw_nosuchfielderror(c, name);
	return NULL;
}


/****************** Function: class_resolvefield_int ***************************

    This is an internally used helper function. Do not use this directly.

	Tries to resolve a field having the given name and type.
    If the field cannot be resolved, NULL is returned.

*******************************************************************************/

static fieldinfo *class_resolvefield_int(classinfo *c, Utf8String name, Utf8String desc)
{
	/* search for field in class c */

	for (int32_t i = 0; i < c->fieldscount; i++) {
		if ((c->fields[i].name == name) && (c->fields[i].descriptor == desc)) {
			return &(c->fields[i]);
		}
    }

	/* Try super interfaces recursively. */

	for (int32_t i = 0; i < c->interfacescount; i++) {
		fieldinfo *fi = class_resolvefield_int(c->interfaces[i], name, desc);

		if (fi != NULL)
			return fi;
	}

	/* Try super class. */

	if (c->super != NULL)
		return class_resolvefield_int(c->super, name, desc);

	/* not found */

	return NULL;
}


/********************* Function: class_resolvefield ***************************

	Resolves a reference from REFERER to a field with NAME and DESC in class C.

    If the field cannot be resolved, an exception is thrown and the
    return value is NULL.

*******************************************************************************/

fieldinfo *class_resolvefield(classinfo *c, Utf8String name, Utf8String desc, classinfo *referer)
{
	fieldinfo *fi = class_resolvefield_int(c, name, desc);

	if (!fi) {
		exceptions_throw_nosuchfielderror(c, name);
		return NULL;
	}

	/* XXX check access rights */

	return fi;
}


/* class_issubclass ************************************************************

   Checks if sub is a descendant of super.

*******************************************************************************/

bool class_issubclass(classinfo *sub, classinfo *super)
{
	classinfo *c = sub;

	for (;;) {
		/* We reached java/lang/Object and did not find the requested
		   super class. */

		if (c == NULL)
			return false;

		/* We found the requested super class. */

		if (c == super)
			return true;

		c = c->super;
	}
}


/* class_isanysubclass *********************************************************

   Checks a subclass relation between two classes. Implemented
   interfaces are interpreted as super classes.

   Return value: 1 ... sub is subclass of super
                 0 ... otherwise

*******************************************************************************/

bool class_isanysubclass(classinfo *sub, classinfo *super)
{
	bool result;

	/* This is the trivial case. */

	if (sub == super)
		return true;

	/* Primitive classes are only subclasses of themselves. */

	if (class_is_primitive(sub) || class_is_primitive(super))
		return false;

	/* Check for interfaces. */

	if (super->flags & ACC_INTERFACE) {
		result = (sub->vftbl->interfacetablelength > super->index) &&
			(sub->vftbl->interfacetable[-super->index] != NULL);
	}
	else {
		/* java.lang.Object is the only super class of any
		   interface. */

		if (sub->state & ACC_INTERFACE)
			return (super == class_java_lang_Object);

#if USES_NEW_SUBTYPE
		result = fast_subtype_check(sub->vftbl, super->vftbl);
#else
		LOCK_CLASSRENUMBER_LOCK;

		uint32_t diffval = sub->vftbl->baseval - super->vftbl->baseval;
		result = diffval <= (uint32_t) super->vftbl->diffval;

		UNLOCK_CLASSRENUMBER_LOCK;
#endif
	}

	return result;
}


/* class_is_arraycompatible ****************************************************

   Checks if two array type descriptors are assignment compatible.

   RETURN VALUE:
      true .... target = desc is possible
      false ... otherwise

*******************************************************************************/

bool class_is_arraycompatible(arraydescriptor *desc, arraydescriptor *target)
{
	if (desc == target)
		return true;

	if (desc->arraytype != target->arraytype)
		return false;

	if (desc->arraytype != ARRAYTYPE_OBJECT)
		return true;

	/* {both arrays are arrays of references} */

	if (desc->dimension == target->dimension) {
		if (!desc->elementvftbl)
			return false;

		/* an array which contains elements of interface types is
		   allowed to be casted to array of Object (JOWENN) */

		if ((desc->elementvftbl->baseval < 0) &&
			(target->elementvftbl->baseval == 1))
			return true;

		return class_isanysubclass(desc->elementvftbl->clazz, target->elementvftbl->clazz);
	}

	if (desc->dimension < target->dimension)
		return false;

	/* {desc has higher dimension than target} */

	return class_isanysubclass(pseudo_class_Arraystub, target->elementvftbl->clazz);
}


/* class_is_assignable_from ****************************************************

   Return whether an instance of the "from" class parameter would be
   an instance of this class "to" as well.

   ARGUMENTS:
       to ..... class
	   from ... class

   RETURN:
       true .... is assignable
	   false ... is not assignable

*******************************************************************************/

bool class_is_assignable_from(classinfo *to, classinfo *from)
{
	if (!(to->state & CLASS_LINKED))
		if (!link_class(to))
			return false;

	if (!(from->state & CLASS_LINKED))
		if (!link_class(from))
			return false;

	/* Decide whether we are dealing with array types or object types. */

	if (class_is_array(to) && class_is_array(from))
		return class_is_arraycompatible(from->vftbl->arraydesc, to->vftbl->arraydesc);
	else
		return class_isanysubclass(from, to);
}


/* class_is_instance ***********************************************************

   Return if the given Java object is an instance of the given class.

   ARGUMENTS:
       c ... class
	   h ... Java object

   RETURN:
       true .... is instance
	   false ... is not instance

*******************************************************************************/

bool class_is_instance(classinfo *c, java_handle_t *h)
{
	if (!(c->state & CLASS_LINKED))
		if (!link_class(c))
			return false;

	/* Decide whether we are dealing with array types or object types. */

	if (class_is_array(c))
		return builtin_arrayinstanceof(h, c);
	else
		return builtin_instanceof(h, c);
}


/* class_get_componenttype *****************************************************

   Return the component class of the given class.  If the given class
   is not an array, return NULL.

*******************************************************************************/

classinfo *class_get_componenttype(classinfo *c)
{
	classinfo       *component;
	arraydescriptor *ad;

	/* XXX maybe we could find a way to do this without linking. */
	/* This way should be safe and easy, however.                */

	if (!(c->state & CLASS_LINKED))
		if (!link_class(c))
			return NULL;

	ad = c->vftbl->arraydesc;

	if (ad == NULL)
		return NULL;

	if (ad->arraytype == ARRAYTYPE_OBJECT)
		component = ad->componentvftbl->clazz;
	else
		component = Primitive::get_class_by_type(ad->arraytype);

	return component;
}


/* class_get_declaredclasses ***************************************************

   Return an array of declared classes of the given class.

*******************************************************************************/

java_handle_objectarray_t *class_get_declaredclasses(classinfo *c, bool publicOnly)
{
	int declaredclasscount = 0; // number of declared classes

	if (!class_is_primitive(c) && !class_is_array(c)) {
		/* Determine number of declared classes. */

		for (uint16_t i = 0; i < c->innerclasscount; i++) {
			/* Get outer-class.  If the inner-class is not a member
			   class, the outer-class is NULL. */

			classref_or_classinfo outer = c->innerclass[i].outer_class;

			if (outer.any == NULL)
				continue;

			/* Check if outer-class is a classref or a real class and
               get the class name from the structure. */

			Utf8String outername = CLASSREF_OR_CLASSINFO_NAME(outer);

			/* Outer class is this class. */

			if ((outername == c->name) &&
				((publicOnly == 0) || (c->innerclass[i].flags & ACC_PUBLIC)))
				declaredclasscount++;
		}
	}

	/* Allocate Class[] and check for OOM. */

	ClassArray declaredclasses(declaredclasscount);

	if (declaredclasses.is_null())
		return NULL;

	for (uint16_t i = 0, pos = 0; i < c->innerclasscount; i++) {
		classref_or_classinfo inner = c->innerclass[i].inner_class;
		classref_or_classinfo outer = c->innerclass[i].outer_class;

		/* Get outer-class.  If the inner-class is not a member class,
		   the outer-class is NULL. */

		if (outer.any == NULL)
			continue;

		/* Check if outer_class is a classref or a real class and get
		   the class name from the structure. */

		Utf8String outername = CLASSREF_OR_CLASSINFO_NAME(outer);

		/* Outer class is this class. */

		if ((outername == c->name) &&
			((publicOnly == 0) || (c->innerclass[i].flags & ACC_PUBLIC))) {

			classinfo *ic = resolve_classref_or_classinfo_eager(inner, false);

			if (ic == NULL)
				return NULL;

			if (!(ic->state & CLASS_LINKED))
				if (!link_class(ic))
					return NULL;

			declaredclasses.set_element(pos++, ic);
		}
	}

	return declaredclasses.get_handle();
}


/**
 * Return an array of declared constructors of the given class.
 *
 * @param c          class to get the constructors of
 * @param publicOnly show only public fields
 *
 * @return array of java.lang.reflect.Constructor
 */
#if defined(ENABLE_JAVASE)
java_handle_objectarray_t *class_get_declaredconstructors(classinfo *c, bool publicOnly)
{
	/* Determine number of constructors. */

	int count = 0;

	for (int32_t i = 0; i < c->methodscount; i++) {
		methodinfo* m = &(c->methods[i]);

		if (((m->flags & ACC_PUBLIC) || (publicOnly == 0)) && (m->name == utf8::init))
			count++;
	}

	/* Create array of constructors. */

	ObjectArray oa(count, class_java_lang_reflect_Constructor);

	if (oa.is_null())
		return NULL;

	/* Get the constructors and store them in the array. */

	for (int32_t i = 0, index = 0; i < c->methodscount; i++) {
		methodinfo* m = &(c->methods[i]);

		if (((m->flags & ACC_PUBLIC) || (publicOnly == 0)) &&
			(m->name == utf8::init)) {
			// Create a java.lang.reflect.Constructor object.

			java_lang_reflect_Constructor rc(m);

			/* Store object into array. */

			oa.set_element(index, rc.get_handle());
			index++;
		}
	}

	return oa.get_handle();
}
#endif


/* class_get_declaredfields ****************************************************

   Return an array of declared fields of the given class.

   ARGUMENTS:
       c ............ class to get the fields of
	   publicOnly ... show only public fields

   RETURN:
       array of java.lang.reflect.Field

*******************************************************************************/

#if defined(ENABLE_JAVASE)
java_handle_objectarray_t *class_get_declaredfields(classinfo *c, bool publicOnly)
{
	/* Determine number of fields. */

	int count = 0;

	for (int32_t i = 0; i < c->fieldscount; i++)
		if ((c->fields[i].flags & ACC_PUBLIC) || (publicOnly == 0))
			count++;

	/* Create array of fields. */

	ObjectArray oa(count, class_java_lang_reflect_Field);

	if (oa.is_null())
		return NULL;

	/* Get the fields and store them in the array. */

	for (int32_t i = 0, index = 0; i < c->fieldscount; i++) {
		fieldinfo* f = &(c->fields[i]);

		if ((f->flags & ACC_PUBLIC) || (publicOnly == 0)) {
			// Create a java.lang.reflect.Field object.

			java_lang_reflect_Field rf(f);

			/* Store object into array. */

			oa.set_element(index, rf.get_handle());
			index++;
		}
	}

	return oa.get_handle();
}
#endif


/* class_get_declaredmethods ***************************************************

   Return an array of declared methods of the given class.

   ARGUMENTS:
       c ............ class to get the methods of
	   publicOnly ... show only public methods

   RETURN:
       array of java.lang.reflect.Method

*******************************************************************************/

#if defined(ENABLE_JAVASE)
java_handle_objectarray_t *class_get_declaredmethods(classinfo *c, bool publicOnly)
{
	/* JOWENN: array classes do not declare methods according to mauve
	   test.  It should be considered, if we should return to my old
	   clone method overriding instead of declaring it as a member
	   function. */

	if (class_is_array(c)) {
		ObjectArray oa(0, class_java_lang_reflect_Method);
		return oa.get_handle();
	}

	/* Determine number of methods. */

	int count = 0;

	for (int32_t i = 0; i < c->methodscount; i++) {
		methodinfo* m = &(c->methods[i]);

		if (((m->flags & ACC_PUBLIC) || (publicOnly == false)) &&
			((m->name != utf8::init) && (m->name != utf8::clinit)) &&
			!(m->flags & ACC_MIRANDA))
			count++;
	}

	/* Create array of methods. */

	ObjectArray oa(count, class_java_lang_reflect_Method);

	if (oa.is_null())
		return NULL;

	/* Get the methods and store them in the array. */

	for (int32_t i = 0, index = 0; i < c->methodscount; i++) {
		methodinfo* m = &(c->methods[i]);

		if (((m->flags & ACC_PUBLIC) || (publicOnly == false)) &&
			((m->name != utf8::init) && (m->name != utf8::clinit)) &&
			!(m->flags & ACC_MIRANDA)) {
			// Create java.lang.reflect.Method object.

			java_lang_reflect_Method rm(m);

			/* Store object into array. */

			oa.set_element(index, rm.get_handle());
			index++;
		}
	}

	return oa.get_handle();
}
#endif


/* class_get_declaringclass ****************************************************

   If the class or interface given is a member of another class,
   return the declaring class.  For array and primitive classes return
   NULL.

*******************************************************************************/

classinfo *class_get_declaringclass(classinfo *c)
{
	classref_or_classinfo  cr;
	classinfo             *dc;

	/* Get declaring class. */

	cr = c->declaringclass;

	if (cr.any == NULL)
		return NULL;

	/* Resolve the class if necessary. */

	if (cr.is_classref()) {
/* 		dc = resolve_classref_eager(cr.ref); */
		dc = resolve_classref_or_classinfo_eager(cr, true);

		if (dc == NULL)
			return NULL;

		/* Store the resolved class in the class structure. */

		cr.cls = dc;
	}

	dc = cr.cls;

	return dc;
}


/* class_get_enclosingclass ****************************************************

   Return the enclosing class for the given class.

*******************************************************************************/

classinfo *class_get_enclosingclass(classinfo *c)
{
	classref_or_classinfo  cr;
	classinfo             *ec;

	/* Get enclosing class. */

	cr = c->enclosingclass;

	if (cr.any == NULL)
		return NULL;

	/* Resolve the class if necessary. */

	if (cr.is_classref()) {
/* 		ec = resolve_classref_eager(cr.ref); */
		ec = resolve_classref_or_classinfo_eager(cr, true);

		if (ec == NULL)
			return NULL;

		/* Store the resolved class in the class structure. */

		cr.cls = ec;
	}

	ec = cr.cls;

	return ec;
}


/**
 * Return the enclosing constructor as java.lang.reflect.Constructor
 * object for the given class.
 *
 * @param c class to return the enclosing constructor for
 *
 * @return java.lang.reflect.Constructor object of the enclosing
 * constructor
 */
#if defined(ENABLE_JAVASE)
java_handle_t* class_get_enclosingconstructor(classinfo *c)
{
	methodinfo* m = class_get_enclosingmethod_raw(c);

	if (m == NULL)
		return NULL;

	/* Check for <init>. */

	if (m->name != utf8::init)
		return NULL;

	// Create a java.lang.reflect.Constructor object.

	java_lang_reflect_Constructor rc(m);

	return rc.get_handle();
}
#endif


/* class_get_enclosingmethod ***************************************************

   Return the enclosing method for the given class.

   IN:
       c ... class to return the enclosing method for

   RETURN:
       methodinfo of the enclosing method

*******************************************************************************/

methodinfo *class_get_enclosingmethod_raw(classinfo *c)
{
	constant_nameandtype *cn;
	classinfo            *ec;
	methodinfo           *m;

	/* get enclosing class and method */

	ec = class_get_enclosingclass(c);
	cn = c->enclosingmethod;

	/* check for enclosing class and method */

	if (ec == NULL)
		return NULL;

	if (cn == NULL)
		return NULL;

	/* find method in enclosing class */

	m = class_findmethod(ec, cn->name, cn->descriptor);

	if (m == NULL) {
		exceptions_throw_internalerror("Enclosing method doesn't exist");
		return NULL;
	}

	return m;
}


/**
 * Return the enclosing method as java.lang.reflect.Method object for
 * the given class.
 *
 * @param c class to return the enclosing method for
 *
 * @return java.lang.reflect.Method object of the enclosing method
 */
#if defined(ENABLE_JAVASE)
java_handle_t* class_get_enclosingmethod(classinfo *c)
{
	methodinfo*    m;

	m = class_get_enclosingmethod_raw(c);

	if (m == NULL)
		return NULL;

	/* check for <init> */

	if (m->name == utf8::init)
		return NULL;

	// Create a java.lang.reflect.Method object.

	java_lang_reflect_Method rm(m);

	return rm.get_handle();
}
#endif


/* class_get_interfaces ********************************************************

   Return an array of interfaces of the given class.

*******************************************************************************/

java_handle_objectarray_t* class_get_interfaces(classinfo *c)
{
	if (!(c->state & CLASS_LINKED))
		if (!link_class(c))
			return NULL;

	ClassArray interfaces(c->interfacescount);

	if (interfaces.is_null())
		return NULL;

	for (int32_t i = 0; i < c->interfacescount; i++) {
		classinfo* ic = c->interfaces[i];

		interfaces.set_element(i, ic);
	}

	return interfaces.get_handle();
}


/* class_get_annotations *******************************************************

   Get the unparsed declared annotations in a byte array
   of the given class.

   IN:
       c........the class of which the annotations should be returned

   RETURN VALUE:
       The unparsed declared annotations in a byte array
       (or NULL if there aren't any).

*******************************************************************************/

java_handle_bytearray_t *class_get_annotations(classinfo *c)
{
#if defined(ENABLE_ANNOTATIONS)
	java_handle_t *annotations; /* unparsed annotations */

	LLNI_classinfo_field_get(c, annotations, annotations);

	return (java_handle_bytearray_t*) annotations;
#else
	return NULL;
#endif
}


/* class_get_modifiers *********************************************************

   Get the modifier flags of the given class.

   IN:
       c....the class of which the modifier flags should be returned
	   ignoreInnerClassesAttrib
   RETURN VALUE:
       modifier flags

*******************************************************************************/

int32_t class_get_modifiers(classinfo *c, bool ignoreInnerClassesAttrib)
{
	/* default to flags of passed class */

	int32_t flags = c->flags;

	/* if requested we check if passed class is inner class */

	if (!ignoreInnerClassesAttrib && (c->innerclasscount != 0)) {
		/* search for passed class as inner class */

		for (int i = 0; i < c->innerclasscount; i++) {
			classref_or_classinfo inner = c->innerclass[i].inner_class;
			classref_or_classinfo outer = c->innerclass[i].outer_class;

			/* Check if inner is a classref or a real class and get
               the name of the structure */

			Utf8String innername = CLASSREF_OR_CLASSINFO_NAME(inner);

			/* innerclass is this class */

			if (innername == c->name) {
				/* has the class actually an outer class? */

				if (outer.any)
					/* return flags got from the outer class file */
					flags = c->innerclass[i].flags;

				break;
			}
		}
	}

	/* remove ACC_SUPER bit from flags */

	return flags & ~ACC_SUPER & ACC_CLASS_REFLECT_MASK;
}


/**
 * Helper function for the function class_is_or_almost_initialized.
 */
bool class_initializing_thread_is_self(classinfo *c)
{
    threadobject *t = thread_get_current();
    return t == c->initializing_thread;
}

/* class_get_signature *********************************************************

   Return the signature of the given class.  For array and primitive
   classes return NULL.

*******************************************************************************/

#if defined(ENABLE_JAVASE)
Utf8String class_get_signature(classinfo *c)
{
	/* For array and primitive classes return NULL. */

	if (class_is_array(c) || class_is_primitive(c))
		return NULL;

	return c->signature;
}
#endif


/* class_printflags ************************************************************

   Prints flags of a class.

*******************************************************************************/

#if !defined(NDEBUG)
void class_printflags(classinfo *c)
{
	if (c == NULL) {
		printf("NULL");
		return;
	}

	if (c->flags & ACC_PUBLIC)       printf(" PUBLIC");
	if (c->flags & ACC_PRIVATE)      printf(" PRIVATE");
	if (c->flags & ACC_PROTECTED)    printf(" PROTECTED");
	if (c->flags & ACC_STATIC)       printf(" STATIC");
	if (c->flags & ACC_FINAL)        printf(" FINAL");
	if (c->flags & ACC_SYNCHRONIZED) printf(" SYNCHRONIZED");
	if (c->flags & ACC_VOLATILE)     printf(" VOLATILE");
	if (c->flags & ACC_TRANSIENT)    printf(" TRANSIENT");
	if (c->flags & ACC_NATIVE)       printf(" NATIVE");
	if (c->flags & ACC_INTERFACE)    printf(" INTERFACE");
	if (c->flags & ACC_ABSTRACT)     printf(" ABSTRACT");
}
#endif


/* class_print *****************************************************************

   Prints classname plus flags.

*******************************************************************************/

#if !defined(NDEBUG)
void class_print(classinfo *c)
{
	if (c == NULL) {
		printf("NULL");
		return;
	}

	utf_display_printable_ascii(c->name);
	class_printflags(c);
}
#endif


/* class_classref_print ********************************************************

   Prints classname plus referer class.

*******************************************************************************/

#if !defined(NDEBUG)
void class_classref_print(constant_classref *cr)
{
	if (cr == NULL) {
		printf("NULL");
		return;
	}

	utf_display_printable_ascii(cr->name);
	printf("(ref.by ");
	if (cr->referer)
		class_print(cr->referer);
	else
		printf("NULL");
	printf(")");
}
#endif


/* class_println ***************************************************************

   Prints classname plus flags and new line.

*******************************************************************************/

#if !defined(NDEBUG)
void class_println(classinfo *c)
{
	class_print(c);
	printf("\n");
}
#endif


/* class_classref_println ******************************************************

   Prints classname plus referer class and new line.

*******************************************************************************/

#if !defined(NDEBUG)
void class_classref_println(constant_classref *cr)
{
	class_classref_print(cr);
	printf("\n");
}
#endif


/* class_classref_or_classinfo_print *******************************************

   Prints classname plus referer class.

*******************************************************************************/

#if !defined(NDEBUG)
void class_classref_or_classinfo_print(classref_or_classinfo c)
{
	if (c.any == NULL) {
		printf("(classref_or_classinfo) NULL");
		return;
	}
	if (c.is_classref())
		class_classref_print(c.ref);
	else
		class_print(c.cls);
}
#endif


/* class_classref_or_classinfo_println *****************************************

   Prints classname plus referer class and a newline.

*******************************************************************************/

#if !defined(NDEBUG)
void class_classref_or_classinfo_println(classref_or_classinfo c)
{
	class_classref_or_classinfo_print(c);
	printf("\n");
}
#endif


/* class_showconstantpool ******************************************************

   Dump the constant pool of the given class to stdout.

*******************************************************************************/

#if !defined(NDEBUG)
void class_showconstantpool (classinfo *c)
{
	printf ("---- dump of constant pool ----\n");

	for (int32_t i=0; i<c->cpcount; i++) {
		printf ("#%u:  ", i);

		if (void *e = c->cpinfos[i]) {
			switch ((ConstantPoolTag) c->cptags[i]) {
			case CONSTANT_Class:
				printf ("Classreference -> ");
				utf_display_printable_ascii ( ((constant_classref*)e) -> name );
				break;
			case CONSTANT_Fieldref:
				printf ("Fieldref -> ");
				field_fieldref_print((constant_FMIref *) e);
				break;
			case CONSTANT_Methodref:
				printf ("Methodref -> ");
				method_methodref_print((constant_FMIref *) e);
				break;
			case CONSTANT_InterfaceMethodref:
				printf ("InterfaceMethod -> ");
				method_methodref_print((constant_FMIref *) e);
				break;
			case CONSTANT_String:
				printf ("String -> ");
				utf_display_printable_ascii ((utf*) e);
				break;
			case CONSTANT_Integer:
				printf ("Integer -> %d", *reinterpret_cast<int32_t*>(e));
				break;
			case CONSTANT_Float:
				printf ("Float -> %f", *reinterpret_cast<float*>(e));
				break;
			case CONSTANT_Double:
				printf ("Double -> %f", *reinterpret_cast<double*>(e));
				break;
			case CONSTANT_Long:
				printf ("Long -> %" PRId64, *reinterpret_cast<int64_t*>(e));
				break;
			case CONSTANT_NameAndType:
				{
					constant_nameandtype *cnt = (constant_nameandtype *) e;
					printf ("NameAndType: ");
					utf_display_printable_ascii (cnt->name);
					printf (" ");
					utf_display_printable_ascii (cnt->descriptor);
				}
				break;
			case CONSTANT_Utf8:
				printf ("Utf8 -> ");
				utf_display_printable_ascii ((utf*) e);
				break;
			case CONSTANT_MethodType: {
				constant_MethodType *type = (constant_MethodType*) e;

				printf ("MethodType -> ");
				utf_display_printable_ascii(type->descriptor);
				break;
			}
			case CONSTANT_MethodHandle: {
				constant_MethodHandle *handle = (constant_MethodHandle*) e;

				printf ("MethodHandle -> ");
				utf_display_printable_ascii(handle->fmi->name);
				utf_display_printable_ascii(handle->fmi->descriptor);
				break;
			}
			case CONSTANT_InvokeDynamic: {
				constant_InvokeDynamic* indy = (constant_InvokeDynamic*) e;

				printf ("InvokeDynamic -> ");
				utf_display_printable_ascii(indy->name);
				utf_display_printable_ascii(indy->descriptor);
				printf(" [%u]", indy->bootstrap_method_index);
				break;
			}
			default:
				log_text("Invalid type of ConstantPool-Entry");
				assert(0);
			}
		}

		printf ("\n");
	}
}
#endif /* !defined(NDEBUG) */


/* class_showmethods ***********************************************************

   Dump info about the fields and methods of the given class to stdout.

*******************************************************************************/

#if !defined(NDEBUG)
void class_showmethods (classinfo *c)
{
	printf("--------- Fields and Methods ----------------\n");
	printf("Flags: ");
	class_printflags(c);
	printf("\n");

	printf("This: ");
	utf_display_printable_ascii(c->name);
	printf("\n");

	if (c->super) {
		printf("Super: ");
		utf_display_printable_ascii(c->super->name);
		printf ("\n");
	}

	printf("Index: %d\n", c->index);

	printf("Interfaces:\n");
	for (int32_t i = 0; i < c->interfacescount; i++) {
		printf("   ");
		utf_display_printable_ascii(c->interfaces[i]->name);
		printf (" (%d)\n", c->interfaces[i]->index);
	}

	printf("Fields:\n");
	for (int32_t i = 0; i < c->fieldscount; i++)
		field_println(&(c->fields[i]));

	printf("Methods:\n");
	for (int32_t i = 0; i < c->methodscount; i++) {
		methodinfo *m = &(c->methods[i]);

		if (!(m->flags & ACC_STATIC))
			printf("vftblindex: %d   ", m->vftblindex);

		method_println(m);
	}

	printf ("Virtual function table:\n");
	for (int32_t i = 0; i < c->vftbl->vftbllength; i++)
		printf ("entry: %d,  %ld\n", i, (long int) (c->vftbl->table[i]));
}
#endif /* !defined(NDEBUG) */

namespace cacao {

OStream& operator<<(OStream& os, const classinfo *c) {
	if (c == NULL) {
	  os << "NULL";
	  return os;
	}

	os << (Utf8String)c->name;
	// print flags
	if (c->flags & ACC_PUBLIC)       os << " PUBLIC";
	if (c->flags & ACC_PRIVATE)      os << " PRIVATE";
	if (c->flags & ACC_PROTECTED)    os << " PROTECTED";
	if (c->flags & ACC_STATIC)       os << " STATIC";
	if (c->flags & ACC_FINAL)        os << " FINAL";
	if (c->flags & ACC_SYNCHRONIZED) os << " SYNCHRONIZED";
	if (c->flags & ACC_VOLATILE)     os << " VOLATILE";
	if (c->flags & ACC_TRANSIENT)    os << " TRANSIENT";
	if (c->flags & ACC_NATIVE)       os << " NATIVE";
	if (c->flags & ACC_INTERFACE)    os << " INTERFACE";
	if (c->flags & ACC_ABSTRACT)     os << " ABSTRACT";

	return os;
}

}

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
