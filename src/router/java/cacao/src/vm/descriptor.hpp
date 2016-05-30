/* src/vm/descriptor.hpp - checking and parsing of field / method descriptors

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


#ifndef DESCRIPTOR_HPP_
#define DESCRIPTOR_HPP_ 1

#include "config.h"

#include <cstddef>                      // for size_t
#include <cstdio>                       // for FILE
#include <stdint.h>                     // for uint32_t, uint8_t
#include <sys/types.h>                  // for ssize_t

#include "toolbox/hashtable.hpp"        // for InsertOnlyStringEntry, etc

#include "vm/global.hpp"                // for Type, Type::TYPE_ADR, etc
#include "vm/primitive.hpp"             // for PrimitiveType
#include "vm/types.hpp"                 // for s4, u2, s2, u1
#include "vm/utf8.hpp"                  // for Utf8String

class Mutex;
struct classinfo;
struct constant_classref;
struct typedesc;
struct paramdesc;
struct methoddesc;


/* data structures ************************************************************/

/*----------------------------------------------------------------------------*/
/* Descriptor Pools                                                           */
/*                                                                            */
/* A descriptor_pool is a temporary data structure used during loading of     */
/* a class. The descriptor_pool is used to allocate the table of              */
/* constant_classrefs the class uses, and for parsing the field and method    */
/* descriptors which occurr within the class. The inner workings of           */
/* descriptor_pool are not important for outside code.                        */
/*                                                                            */
/* You use a descriptor_pool as follows:                                      */
/*                                                                            */
/* 1. create one with descriptor_pool_new                                     */
/* 2. add all explicit class references with descriptor_pool_add_class        */
/* 3. add all field/method descriptors with descriptor_pool_add               */
/* 4. call descriptor_pool_create_classrefs                                   */
/*    You can now lookup classrefs with descriptor_pool_lookup_classref       */
/* 5. call descriptor_pool_alloc_parsed_descriptors                           */
/* 6. for each field descriptor call descriptor_pool_parse_field_descriptor   */
/*    for each method descriptor call descriptor_pool_parse_method_descriptor */
/*                                                                            */
/* IMPORTANT: The descriptor_pool functions use DNEW and DMNEW for allocating */
/*            memory which can be thrown away when the steps above have been  */
/*            done.                                                           */
/*----------------------------------------------------------------------------*/

struct descriptor_pool;

namespace cacao {

struct DescriptorPool {
	DescriptorPool(classinfo *referer);

	bool    add_class(Utf8String name);
	bool    add_field(Utf8String desc);
	ssize_t add_method(Utf8String desc); // returns number of paramslots method needs,
	                                     // or -1 on error

	constant_classref *create_classrefs(s4 *count);
	constant_classref *lookup_classref(Utf8String classname);

	void alloc_parsed_descriptors();

	typedesc   *parse_field_descriptor(Utf8String desc);
	methoddesc *parse_method_descriptor(Utf8String desc, s4 mflags, constant_classref *thisclass);

	classinfo *get_referer();

	void get_sizes(size_t *classrefsize, size_t *descsize);
private:
	template<typename T>
	inline T *allocate(size_t size = sizeof(T));

	typedef HashTable<InsertOnlyNameValuePair<u2> >        ClassrefHash;
	typedef HashTable<InsertOnlyNameValuePair<typedesc*> > FieldrefHash;

	ClassrefHash  classrefhash;
	FieldrefHash  fieldrefhash;

	classinfo         *referer;
	size_t             fieldcount;
	size_t             methodcount;
	size_t             paramcount;
	size_t             descriptorsize;
	constant_classref *classrefs;
	Mutex             *mutex;

	// we allocate all typedesc & methoddesc from a big chunk of memory
	// created in alloc_parsed_descriptors
	uint8_t           *descriptors;
	uint8_t           *descriptors_next;
};


} // end namespace cacao


/* data structures for parsed field/method descriptors ************************/

struct typedesc {
	// return the size in bytes needed for the given type.
	inline size_t typesize() const;

	constant_classref *classref;          /* class reference for TYPE_ADR types   */
	Type               type          : 8; /* TYPE_??? constant [1]                */
	PrimitiveType      primitivetype : 8; /* (PRIMITIVE)TYPE_??? constant [2]     */
	u1                 arraydim;          /* array dimension (0 if no array)      */
};

/* [1]...the type field contains the basic type used within the VM. So ints,  */
/*       shorts, chars, bytes, booleans all have TYPE_INT.                    */
/* [2]...the primitivetype field contains the declared type.                  */
/*       So short is PRIMITIVETYPE_SHORT, char is PRIMITIVETYPE_CHAR.         */
/*       For non-primitive types primitivetype is TYPE_ADR.                   */

struct paramdesc {
#if defined(__MIPS__)
	Type     type : 8;          /* TYPE_??? of the register allocated         */
#endif
	bool     inmemory;          /* argument in register or on stack           */
	uint32_t index;             /* index into argument register array         */
	uint32_t regoff;            /* register index or stack offset             */
};

struct methoddesc {
	void params_from_paramtypes(s4 mflags);

	s2         paramcount;      /* number of parameters                       */
	s2         paramslots;      /* like above but LONG,DOUBLE count twice     */
	s4         argintreguse;    /* number of used integer argument registers  */
	s4         argfltreguse;    /* number of used float argument registers    */
	s4         memuse;          /* number of stack slots used                 */
	paramdesc *params;          /* allocated parameter descriptions [3]       */
	Mutex     *pool_lock;       /* synchronizes access to params              */
	typedesc   returntype;      /* parsed descriptor of the return type       */
	typedesc   paramtypes[1];   /* parameter types, variable length!          */
};

/* [3]...If params is NULL, the parameter descriptions have not yet been      */
/*       allocated. In this case ___the possible 'this' pointer of the method */
/*       is NOT counted in paramcount/paramslots and it is NOT included in    */
/*       the paramtypes array___.                                             */
/*       If params != NULL, the parameter descriptions have been              */
/*       allocated, and the 'this' pointer of the method, if any, IS included.*/
/*       In case the method has no parameters at all, the special value       */
/*       METHODDESC_NO_PARAMS is used (see below).                            */

/* METHODDESC_NO_PARAMS is a special value for the methoddesc.params field    */
/* indicating that the method is a static method without any parameters.      */
/* This special value must be != NULL and it may only be set if               */
/* md->paramcount == 0.                                                       */

#define METHODDESC_NOPARAMS  ((paramdesc*)1)

/* function prototypes ********************************************************/

Type descriptor_to_basic_type(Utf8String desc);

void descriptor_debug_print_typedesc(FILE*,typedesc*);
void descriptor_debug_print_methoddesc(FILE*,methoddesc*);

/* inline functions    ********************************************************/

inline size_t typedesc::typesize() const {
	switch (type) {
	case TYPE_INT:
	case TYPE_FLT:
		return 4;

	case TYPE_LNG:
	case TYPE_DBL:
		return 8;

	case TYPE_ADR:
		return SIZEOF_VOID_P;

	default:
		assert(false && "Illegal Type");
		return 0;
	}
}


#endif // DESCRIPTOR_HPP_


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
