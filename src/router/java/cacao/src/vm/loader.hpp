/* src/vm/loader.hpp - class loader header

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


#ifndef LOADER_HPP_
#define LOADER_HPP_ 1

#include "config.h"

#include <stdint.h>                     // for uint8_t, int32_t

#include "vm/global.hpp"                // for java_handle_t, etc
#include "vm/types.hpp"                 // for s4, s8
#include "vm/utf8.hpp"                  // for Utf8String

struct classinfo;
struct constant_nameandtype;

namespace cacao { struct ClassBuffer; }


/* classloader *****************************************************************

   [!ENABLE_HANDLES]: The classloader is a Java Object which cannot move.
   [ENABLE_HANDLES] : The classloader entry itself is a static handle for a
                      given classloader (use loader_hashtable_classloader_foo).

*******************************************************************************/

#if defined(ENABLE_HANDLES)
typedef hashtable_classloader_entry classloader_t;
#else
typedef java_object_t               classloader_t;
#endif

/* constant pool entries *******************************************************

	All constant pool entries need a data structure which contain the entrys
	value. In some cases this structure exist already, in the remaining cases
	this structure must be generated:

		kind                      structure                     generated?
	----------------------------------------------------------------------
    CONSTANT_Class               constant_classref                  yes
    CONSTANT_Fieldref            constant_FMIref                    yes
    CONSTANT_Methodref           constant_FMIref                    yes
    CONSTANT_InterfaceMethodref  constant_FMIref                    yes
    CONSTANT_String              unicode                             no
    CONSTANT_Integer             int32_t                            yes
    CONSTANT_Float               float                              yes
    CONSTANT_Long                int64_t                            yes
    CONSTANT_Double              double                             yes
    CONSTANT_NameAndType         constant_nameandtype               yes
    CONSTANT_Utf8                unicode                             no
    CONSTANT_UNUSED              -

*******************************************************************************/

struct  constant_nameandtype {         /* NameAndType (Field or Method)       */
	constant_nameandtype(Utf8String name, Utf8String desc) : name(name), descriptor(desc) {}

	const Utf8String name;               /* field/method name                   */
	const Utf8String descriptor;         /* field/method type descriptor string */
};


/* hashtable_classloader_entry *************************************************

   ATTENTION: The pointer to the classloader object needs to be the
   first field of the entry, so that it can be used as an indirection
   cell. This is checked by gc_init() during startup.

*******************************************************************************/

struct hashtable_classloader_entry {
	java_object_t               *object;
	hashtable_classloader_entry *hashlink;
};


namespace cacao {
	/**
	 * A version of the Java class file format.
	 *
	 * @Cpp11 Make all methods, ctors, statics a constexpr
	 */
	struct ClassFileVersion {
		/**
		 * The class file format version supported by CACAO
		 */
		static const ClassFileVersion CACAO_VERSION;

		/**
		 * The class file format version used by JDK 7
		 */
		static const ClassFileVersion JDK_7;


		ClassFileVersion(uint16_t major, uint16_t minor = 0) : _majr(major), _minr(minor) {}

		bool operator ==(ClassFileVersion v) const {
			return _majr == v._majr && _minr == v._minr;
		}

		/// A strict weak ordering as required by STL
		bool operator <(ClassFileVersion v) const {
			if (_majr < v._majr)
				return true;
			if (v._majr < _majr)
				return false;

			// major versions are equal

			if (_minr < v._minr)
				return true;
			return false;
		}

		bool operator <=(ClassFileVersion v) const {
			return (*this == v) || (*this < v);
		}

		// we can't call these major/minor because GCC defines macros of that name
		uint16_t majr() const { return _majr; }
		uint16_t minr() const { return _minr; }
	private:
		uint16_t _majr, _minr;
	};
}


/* function prototypes ********************************************************/

void loader_preinit(void);
void loader_init(void);

/* classloader management functions */
classloader_t *loader_hashtable_classloader_add(java_handle_t *cl);
classloader_t *loader_hashtable_classloader_find(java_handle_t *cl);

void loader_load_all_classes(void);

bool loader_skip_attribute_body(cacao::ClassBuffer& cb);

#if defined(ENABLE_JAVASE)
bool loader_load_attribute_signature(cacao::ClassBuffer& cb, Utf8String& signature);
#endif

/* free resources */
void loader_close(void);

/* class loading functions */
classinfo *load_class_from_sysloader(Utf8String name);
classinfo *load_class_from_classloader(Utf8String name, classloader_t *cl);
classinfo *load_class_bootstrap(Utf8String name);

/* (don't use the following directly) */
classinfo *load_class_from_classbuffer(cacao::ClassBuffer& cb);
classinfo *load_newly_created_array(classinfo *c, classloader_t *loader);

#endif // LOADER_HPP_


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
