/* src/vm/descriptor.cpp - checking and parsing of field / method descriptors

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

#include "vm/descriptor.hpp"
#include "config.h"                     // for ENABLE_JIT

#include <cassert>                      // for assert
#include <cstdarg>                      // for va_end, va_list, va_start

#include "mm/memory.hpp"                // for MNEW, MMOVE

#include "threads/mutex.hpp"            // for Mutex

#include "toolbox/OStream.hpp"          // for OStream
#include "toolbox/buffer.hpp"           // for Buffer
#include "toolbox/logging.hpp"          // for LOG

#include "vm/exceptions.hpp"
#include "vm/global.hpp"                // for Type::TYPE_ADR, etc
#include "vm/options.hpp"
#include "vm/primitive.hpp"             // for PrimitiveType, etc
#include "vm/references.hpp"            // for constant_classref
#include "vm/types.hpp"                 // for u4, u1, s4, s2, u2
#include "vm/utf8.hpp"                  // for Utf8String, operator<<, etc
#include "vm/vm.hpp"                    // for vm_abort

#include "vm/jit/abi.hpp"               // for md_param_alloc, etc

struct classinfo;

using namespace cacao;

#define DEBUG_NAME "descriptor"


/***
 *
 *	Field & method descriptor parsing
 *
 *	See JVM Spec §4.3.2 (Field Descriptors) and §4.3.3 (Method Descriptors)
 *
 *	All methods return true on success and throw an exception on error.
 *	They also check if the parsed type is valid for its position, for example 
 *	a field type can't be void but a return type can.
 *
 *	Calling a check or parse method consumes the input.
 *	In the following example the call to parse_field_descriptor() will fail
 *	because check_field_descriptor() has consumed the whole descriptor.
 *	@code
 *		DescriptorParser p(pool, Utf8String::from_utf8("J"));
 *		if (!p.check_field_descriptor())
 *			return false;
 *		if (!p.parse_field_descriptor(&t))
 *			return false;
 *	@endcode
 */
struct DescriptorParser {
	DescriptorParser(DescriptorPool *pool, Utf8String desc);

	/***
	 *	Check if descriptor is valid.
	 *	Any class references in descriptor will be added to pool
	 */
	bool check_field_descriptor();
	bool check_param_descriptor(Type &dst);
	bool check_return_descriptor();

	/***
	 *	Parse descriptor into typedesc.
	 *	Descriptor must have previously been checked with check_*_descriptor()
	 *	All class references are looked up in pool.
	 */
	bool parse_field_descriptor(typedesc *dst);
	bool parse_param_descriptor(typedesc *dst);
	bool parse_return_descriptor(typedesc *dst);

	/***
	 *	Start parsing a method descriptors parameter list
	 */
	bool start_param_list();

	/***
	 *	Check if the parameter list we are currently parsing 
	 *	has more parameters to parse.
	 */
	bool has_more_params();
private:
	enum ParserFlag {
		FORBID_VOID_TYPE,
		EXPECT_END_OF_INPUT
	};

	enum ParseResult {
		PARSE_ERROR,
		PRIMITIVE_TYPE,
		REFERENCE_TYPE
	};


	template<int flags>
	bool check_type(const char *descriptor_type, Type& t);

	template<int flags>
	bool parse_type(const char *descriptor_type, typedesc *t);

	template<int flags>
	ParseResult parse_type(Type& t, PrimitiveType& p,
	                       Utf8String& classname, size_t& arraydim);

	bool parse_classname(Utf8String& classname);
	bool parse_arraytype(Utf8String& classname, size_t& arraydim);

	bool skip_classname();

	bool read(char&);
	void unread();

	bool has_more_input() const;

	/// throws a classformat error and returns false
	bool throw_error(const char *reason, ...);

	DescriptorPool *const pool;
	const char     *descriptor_type;  // what kind of descriptor we are currently parsing
	const char     *pos, *end;        // text of descriptor
};

inline DescriptorParser::DescriptorParser(DescriptorPool *pool, Utf8String desc)
 : pool(pool), pos(desc.begin()), end(desc.end()) {}

inline bool DescriptorParser::check_field_descriptor() {
	Type dummy;

	return check_type<FORBID_VOID_TYPE | EXPECT_END_OF_INPUT>("field", dummy);
}
inline bool DescriptorParser::check_param_descriptor(Type &dst) {
	return check_type<FORBID_VOID_TYPE>("parameter", dst);
}
inline bool DescriptorParser::check_return_descriptor() {
	Type dummy;

	return check_type<EXPECT_END_OF_INPUT>("return type", dummy);
}

inline bool DescriptorParser::parse_field_descriptor(typedesc *dst) {
	return parse_type<FORBID_VOID_TYPE | EXPECT_END_OF_INPUT>("field", dst);
}
inline bool DescriptorParser::parse_param_descriptor(typedesc *dst) {
	return parse_type<FORBID_VOID_TYPE>("parameter", dst);
}
inline bool DescriptorParser::parse_return_descriptor(typedesc *dst) {
	return parse_type<EXPECT_END_OF_INPUT>("return type", dst);
}

inline bool DescriptorParser::start_param_list() {
	descriptor_type = "method";

	char c = 0;

	if (!read(c))
		return false;

	if (c != '(')
		return throw_error("Method descriptor does not start with '('");

	return true;
}

inline bool DescriptorParser::has_more_params() {
	if (!has_more_input())
		return false;
	if (*pos == ')') {
		pos++;
		return false;
	}

	return true;
}

inline bool DescriptorParser::has_more_input() const {
	return pos != end;
}

template<int flags>
inline bool DescriptorParser::check_type(const char *descriptor_type, Type& dst) {
	this->descriptor_type = descriptor_type;

	Type          type;
	PrimitiveType primtype;
	Utf8String    classname;
	size_t        arraydim = 0;

	switch (parse_type<flags>(type, primtype, classname, arraydim)) {
	case PARSE_ERROR:
		return false;
	case PRIMITIVE_TYPE:
		dst = type;
		return true;
	case REFERENCE_TYPE:
		// descriptor is array or class type
		dst = type;
		return pool->add_class(classname);
	default:
		assert(false);
		return false;
	}
}

template<int flags>
inline bool DescriptorParser::parse_type(const char *descriptor_type, typedesc *dst) {
	this->descriptor_type = descriptor_type;

	Type          type     = TYPE_INT;
	PrimitiveType primtype = PRIMITIVETYPE_INT;
	Utf8String    classname;
	size_t        arraydim = 0;

	switch (parse_type<flags>(type, primtype, classname, arraydim)) {
	case PARSE_ERROR:
		return false;
	case PRIMITIVE_TYPE:
		dst->classref      = NULL;
		dst->type          = type;
		dst->primitivetype = primtype;
		dst->arraydim      = 0;
		return true;
	case REFERENCE_TYPE:
		dst->classref      = pool->lookup_classref(classname);
		dst->type          = TYPE_ADR;
		dst->primitivetype = (PrimitiveType) TYPE_ADR;
		dst->arraydim      = arraydim;
		return true;
	default:
		assert(false);
		return false;
	}
}

template<int flags>
inline DescriptorParser::ParseResult DescriptorParser::parse_type(Type&          t,
                                                                  PrimitiveType& p,
                                                                  Utf8String&    classname,
                                                                  size_t&        arraydim) {
	ParseResult result;
	char        c = 0;

	if (!read(c))
		return PARSE_ERROR;

	switch (c) {
	case 'B':
		t      = TYPE_INT;
		p      = PRIMITIVETYPE_BYTE;
		result = PRIMITIVE_TYPE;
		break;
	case 'C':
		t      = TYPE_INT;
		p      = PRIMITIVETYPE_CHAR;
		result = PRIMITIVE_TYPE;
		break;
	case 'D':
		t      = TYPE_DBL;
		p      = PRIMITIVETYPE_DOUBLE;
		result = PRIMITIVE_TYPE;
		break;
	case 'F':
		t      = TYPE_FLT;
		p      = PRIMITIVETYPE_FLOAT;
		result = PRIMITIVE_TYPE;
		break;
	case 'I':
		t      = TYPE_INT;
		p      = PRIMITIVETYPE_INT;
		result = PRIMITIVE_TYPE;
		break;
	case 'J':
		t      = TYPE_LNG;
		p      = PRIMITIVETYPE_LONG;
		result = PRIMITIVE_TYPE;
		break;
	case 'S':
		t      = TYPE_INT;
		p      = PRIMITIVETYPE_SHORT;
		result = PRIMITIVE_TYPE;
		break;
	case 'Z':
		t      = TYPE_INT;
		p      = PRIMITIVETYPE_BOOLEAN;
		result = PRIMITIVE_TYPE;
		break;
	case 'V':
		if (flags & FORBID_VOID_TYPE) {
			throw_error("Type void is illegal");
			return PARSE_ERROR;
		}

		t      = TYPE_VOID;
		p      = PRIMITIVETYPE_VOID;
		result = PRIMITIVE_TYPE;
		break;

	case 'L': // object (L<classname>;)
		if (!parse_classname(classname))
			return PARSE_ERROR;

		t        = TYPE_ADR;
		arraydim = 0;
		result   = REFERENCE_TYPE;
		break;
	case '[': // array type
		unread(); // parse_arraytype counts the '['

		if (!parse_arraytype(classname, arraydim))
			return PARSE_ERROR;

		t      = TYPE_ADR;
		result = REFERENCE_TYPE;
		break;
	default:
		throw_error("Illegal character '%c'", c);
		return PARSE_ERROR;
	}

	if ((flags & EXPECT_END_OF_INPUT) && has_more_input()) {
		throw_error("Unexpected characters at end of descriptor");
		return PARSE_ERROR;
	}

	return result;
}

inline bool DescriptorParser::parse_classname(Utf8String& classname) {
	// find end of classname

	const char *mark = pos;

	if (!skip_classname())
		return false;

	// utf8 string from classname

	Utf8String str = Utf8String::from_utf8(mark, pos - mark - 1);

	if (str == NULL)
		return throw_error("Class name is not valid UTF-8");

	// done

	classname = str;
	return true;
}

inline bool DescriptorParser::parse_arraytype(Utf8String& classname, size_t& arraydim) {
	size_t array_dimension = 0;

	const char *mark = pos;

	char c = 0;

	// skip leading '['

	while (true) {
		if (!read(c))
			return false;
		if (c != '[')
			break;

		array_dimension++;
	}

	if (array_dimension > 255)
		return throw_error("Too large array dimension: %lu", array_dimension);

	// check if element type is valid

	switch (c) {
	case 'B': // primitive type
	case 'C':
	case 'D':
	case 'F':
	case 'I':
	case 'J':
	case 'S':
	case 'Z':
		break;

	case 'L': // object (L<classname>;)
		if(!skip_classname())
			return false;

		break;

	default:
		return throw_error("Illegal type of array element '%c'", c);
	}

	classname = Utf8String::from_utf8(mark, pos - mark);

	if (classname == NULL)
		return throw_error("Name is not valid utf8: '%s'", mark);

	arraydim = array_dimension;

	return true;
}

inline bool DescriptorParser::skip_classname() {
	// classname is delimited by 'L' and ';'
	// but at this point the 'L' was already consumed

	while (true) {
		char c = 0;

		if (!read(c))
			return false;
		if (c == ';')
			return true;
	}
}

inline bool DescriptorParser::read(char& c) {
	if (pos == end)
		return throw_error("Truncated descriptor");

	c = *pos++;
	return true;
}

inline void DescriptorParser::unread() {
	pos--;
}

inline bool DescriptorParser::throw_error(const char *reason, ...) {
	assert(descriptor_type);

	Buffer<> buf;
	va_list  ap;

	va_start(ap, reason);
	buf.write("Invalid ").write(descriptor_type).write(" descriptor: ").writevf(reason, ap);
	va_end(ap);

	exceptions_throw_classformaterror(pool->get_referer(), buf.utf8_str());
	return false;
}

//******************************************************************************
//	Descriptor pool
//******************************************************************************

// initial number of entries for the classrefhash of a descriptor pool
#define CLASSREFHASH_INIT_SIZE  256

// initial number of entries for the fieldrefhash of a descriptor pool
#define FIELDREFHASH_INIT_SIZE  256


DescriptorPool::DescriptorPool(classinfo *referer) : classrefhash(CLASSREFHASH_INIT_SIZE),
                                                     fieldrefhash(FIELDREFHASH_INIT_SIZE),
                                                     referer(referer),
                                                     fieldcount(0),
                                                     methodcount(0),
                                                     paramcount(0),
                                                     descriptorsize(0),
                                                     classrefs(NULL),
                                                     mutex(new Mutex()),
                                                     descriptors(NULL),
                                                     descriptors_next(NULL) {}

/***
 * Allocate an object from the descriptor pools private block of memory.
 * This function performs no checks, you can only allocate objects you have
 * previously added via add_field or add_method.
 */
template<typename T>
T *DescriptorPool::allocate(size_t size) {
	T *t = (T*) descriptors_next;

	descriptors_next += size;

	return t;
}


/***
 * Add the given class reference to the pool
 *
 * @return false iff an exception was thrown.
*/
bool DescriptorPool::add_class(Utf8String name) {
	LOG("DescriptorPool::add_class(" << name << ")");

	assert(name);

	/* find a place in the hashtable */

	ClassrefHash::EntryRef ref = classrefhash.find(name);

	if (ref)
		return true; // classname was already present

	/* check if the name is a valid classname */

	if (!name.is_valid_name()) {
		exceptions_throw_classformaterror(referer, "Invalid class name");
		return false; /* exception */
	}

	/* XXX check maximum array dimension */

	classrefhash.insert(ref, name, classrefhash.size());
	return true;
}


/***
 * Check the given descriptor and add it to the pool
 *
 * @param desc the field descriptor to add.
 * @return false iff an exception has been thrown
 */
bool DescriptorPool::add_field(Utf8String desc) {
	LOG("DescriptorPool::add_field(" << ((void*) this) << ", " << desc << ")\n");

	// check if field descriptor was already in table

	FieldrefHash::EntryRef ref = fieldrefhash.find(desc);

	if (ref)
		return true;

	// check the descriptor

	DescriptorParser parser(this, desc);

	if (!parser.check_field_descriptor())
		return false;

	// add the descriptor to the pool

	fieldrefhash.insert(ref, desc, (typedesc*) NULL);

	// done

	fieldcount++;
	return true;
}

/***
 * Check the given descriptor and add it to the pool
 *
 * @param desc the method descriptor to add.
 * @return false iff an exception has been thrown
 */
ssize_t DescriptorPool::add_method(Utf8String desc) {
	LOG("DescriptorPool::add_method(" << ((void*) this) << ", " << desc << ")\n");

	assert(desc);

	// check the descriptor

	DescriptorParser parser(this, desc);

	size_t argcount = 0;

	// check parameters

	if (!parser.start_param_list())
		return -1;

	while (parser.has_more_params()) {
		// We cannot count the `this' argument here because
		// we don't know if the method is static.

		paramcount++;

		Type type;

		if (!parser.check_param_descriptor(type))
			return -1;

		argcount += IS_2_WORD_TYPE(type) ? 2 : 1;
	}

	if (argcount > 255) {
		exceptions_throw_classformaterror(referer, "Too many arguments in signature");
		return -1;
	}

	// check return type

	if (!parser.check_return_descriptor())
		return -1;

	// done

	methodcount++;
	return argcount;
}


/***
 * Create a table containing all the classrefs which were added to the pool
 *
 * @param[out] count  if count is non-NULL, this is set to the number
 *                    of classrefs in the table
 * @return a pointer to the constant_classref table
 */
constant_classref *DescriptorPool::create_classrefs(s4 *count)
{
	size_t nclasses;

	nclasses  = classrefhash.size();
	classrefs = MNEW(constant_classref,nclasses);

	// fill the constant_classref structs

	for (ClassrefHash::Iterator it = classrefhash.begin(), end = classrefhash.end(); it != end; ++it) {
		new (classrefs + it->value()) constant_classref(referer, it->key());
	}

	if (count)
		*count = nclasses;

	return classrefs;
}


/***
 *  Return the constant_classref for the given class name
 *
 * @param classname  name of the class to look up
 * @return a pointer to the constant_classref, or
 *           NULL if an exception has been thrown
 */
constant_classref *DescriptorPool::lookup_classref(Utf8String classname)
{
	assert(classrefs);
	assert(classname);

	if (ClassrefHash::EntryRef ref = classrefhash.find(classname)) {
		return classrefs + ref->value();
	} else {
		exceptions_throw_internalerror("Class reference not found in descriptor pool");
		return NULL;
	}
}


/***
 * Allocate space for the parsed descriptors
 *
 * @note
 *    This function must be called after all descriptors have been added
 *    with DescriptorPool::add.
 */
void DescriptorPool::alloc_parsed_descriptors() {
	/* TWISTI: paramcount + 1: we don't know if the method is static or   */
	/* not, i have no better solution yet.                                */

	size_t size = fieldcount  * sizeof(typedesc)
		        + methodcount * (sizeof(methoddesc) - sizeof(typedesc))
		        + paramcount  * sizeof(typedesc)
		        + methodcount * sizeof(typedesc);      // possible `this' pointer

	descriptorsize = size;

	if (size) {
		descriptors      = MNEW(u1, size);
		descriptors_next = descriptors;
	}
}


/***
 *  Parse the given field descriptor
 *
 * @param pool the descriptor_pool
 * @param desc the field descriptor
 * @return a pointer to the parsed field descriptor, or
 *         NULL if an exception has been thrown
 *
 * @note
 *    DescriptorPool::alloc_parsed_descriptors must be called (once)
 *    before this function is used.
 */
typedesc *DescriptorPool::parse_field_descriptor(Utf8String desc) {
	assert(descriptors);
	assert(descriptors_next);

	// lookup the descriptor in the hashtable

	FieldrefHash::EntryRef ref = fieldrefhash.find(desc);

	assert(ref);

	typedesc *td = ref->value();

	if (!td) {
		// field has been parsed yet.
		td = allocate<typedesc>();

		DescriptorParser parser(this, desc);

		if (!parser.parse_field_descriptor(td))
			return NULL;

		ref->value() = td;
	}

	return td;
}


/***
 * Parse the given method descriptor
 * 
 * @param desc      the method descriptor
 * @param mflags    the method flags
 * @param thisclass classref to the class containing the method.
 *                  This is ignored if mflags contains ACC_STATIC.
 *                  The classref is stored for inserting the 'this' argument.
 * @return a pointer to the parsed method descriptor, 
 *         or NULL if an exception has been thrown
 *
 * @note
 *      descriptor_pool::alloc_parsed_descriptors must be called
 *     (once) before this function is used.
 */
methoddesc *DescriptorPool::parse_method_descriptor(Utf8String         desc,
										            s4                 mflags,
                                                    constant_classref *thisclass) {
	LOG("DescriptorPool::parse_method_descriptor(" << ((void*) this) << ", " << mflags << ", " << thisclass << ")\n");

	assert(descriptors);
	assert(descriptors_next);

	// check that it is a method descriptor

	if (desc[0] != '(') {
		exceptions_throw_classformaterror(referer,
										  "Field descriptor used in method reference");
		return NULL;
	}

	methoddesc *md = allocate<methoddesc>(offsetof(methoddesc, paramtypes));
	md->pool_lock  = mutex;

	s2 paramcount = 0;
	s2 paramslots = 0;

	// count the `this' pointer

	if ((mflags != ACC_UNDEF) && !(mflags & ACC_STATIC)) {
		typedesc *td = allocate<typedesc>();

		td->type          = TYPE_ADR;
		td->arraydim      = 0;
		td->classref      = thisclass;

		paramcount++;
		paramslots++;
	}

	// parse parameters

	DescriptorParser parser(this, desc);

	if (!parser.start_param_list())
		return NULL;

	while (parser.has_more_params()) {
		// parse a parameter type

		typedesc *td = allocate<typedesc>();

		if (!parser.parse_param_descriptor(td))
			return NULL;

		if (IS_2_WORD_TYPE(td->type))
			paramslots++;

		paramcount++;
		paramslots++;
	}

	md->paramcount = paramcount;
	md->paramslots = paramslots;

	// Skip possible `this' pointer in paramtypes array to allow a possible
	// memory move later in parse.
	// We store the thisclass reference, so we can later correctly fill in
	// the parameter slot of the 'this' argument.

	if (mflags == ACC_UNDEF) {
		typedesc *td = allocate<typedesc>();

		td->classref = thisclass;
	}

	// parse return type

	if (!parser.parse_return_descriptor(&md->returntype))
		return NULL;

	// If mflags != ACC_UNDEF we parse a real loaded method, so do
	// param prealloc.  Otherwise we do this in stack analysis.

	if (mflags != ACC_UNDEF) {
		if (md->paramcount > 0) {
			// allocate memory for params

			md->params = MNEW(paramdesc, md->paramcount);
		} else {
			md->params = METHODDESC_NOPARAMS;
		}

		// fill the paramdesc
		// md_param_alloc has to be called if md->paramcount == 0,
		// too, so it can make the reservation for the Linkage Area,
		// Return Register...

#if defined(ENABLE_JIT)
		// As builtin-functions are native functions, we have
		// to pre-allocate for the native ABI.

		if (mflags & ACC_METHOD_BUILTIN)
			md_param_alloc_native(md);
		else
			md_param_alloc(md);
#endif

		// params already initialized; no need to lock
		md->pool_lock = NULL;
	} else {
		// params will be allocated later by
		// methoddesc::params_from_paramtypes if necessary

		md->params = NULL;
	}

	return md;
}

/***
 * Create the paramdescs for a method descriptor. This function is
 * called when we know whether the method is static or not. This
 * function does nothing if md->params != NULL (checked atomically).
 *
 * @param mflags the ACC_* access flags of the method. Only the
 *               ACC_STATIC bit is checked.
 *               The value ACC_UNDEF is NOT allowed.
 *
 * @post parms != NULL
 */
void methoddesc::params_from_paramtypes(s4 mflags) {
	bool has_lock = pool_lock != NULL;

	if (pool_lock)
		pool_lock->lock();
	if (params) {
		if (has_lock)
			pool_lock->unlock();
		return;
	}

	assert(params == NULL);
	assert(mflags != ACC_UNDEF);

	typedesc *td = paramtypes;

	/* check for `this' pointer */

	if (!(mflags & ACC_STATIC)) {
		constant_classref *thisclass;

		/* fetch class reference from reserved param slot */
		thisclass = td[paramcount].classref;
		assert(thisclass);

		if (paramcount > 0) {
			/* shift param types by 1 argument */
			MMOVE(td + 1, td, typedesc, paramcount);
		}

		/* fill in first argument `this' */

		td->type          = TYPE_ADR;
		td->primitivetype = (PrimitiveType) TYPE_ADR;
		td->arraydim      = 0;
		td->classref      = thisclass;

		paramcount++;
		paramslots++;
	}

	// if the method has params, process them

	if (paramcount > 0) {
		// allocate memory for params
		params = MNEW(paramdesc, paramcount);
	} else {
		params = METHODDESC_NOPARAMS;
	}

	// fill the paramdesc
	// md_param_alloc has to be called if md->paramcount == 0, too, so
	// it can make the reservation for the Linkage Area, Return Register, ...

#if defined(ENABLE_JIT)
	// As builtin-functions are native functions, we have to
	// pre-allocate for the native ABI.

	if (mflags & ACC_METHOD_BUILTIN)
		md_param_alloc_native(this);
	else
		md_param_alloc(this);
#endif

	if (has_lock)
		pool_lock->unlock();
}

classinfo *DescriptorPool::get_referer() {
	return referer;
}

/***
 * Get the sizes of the class reference table and the parsed descriptors
 *
 * @param[out] classrefsize  set to size of the class reference table
 * @param[out] descsize      set to size of the parsed descriptors
 *
 * @note
 *     This function may only be called after both
 *     descriptor_pool::create_classrefs, and
 *     descriptor_pool::alloc_parsed_descriptors
 *     have been called.
 */
void DescriptorPool::get_sizes(size_t *classrefsize, size_t *descsize) {
	assert((!fieldcount && !methodcount) || descriptors);
	assert(classrefs);
	assert(classrefsize);
	assert(descsize);

	*classrefsize = classrefhash.size() * sizeof(constant_classref);
	*descsize     = descriptorsize;
}


/***
 * Return the basic type to use for a value with this descriptor.
 *
 * @pre This function assumes that the descriptor has passed
 *      DescriptorPool::add_field checks and that it does not start with '('.
 */
Type descriptor_to_basic_type(Utf8String descriptor) {
	assert(descriptor.size() >= 1);

	switch (descriptor[0]) {
	case 'Z':
	case 'B':
	case 'C':
	case 'S':
	case 'I':
		return TYPE_INT;

	case 'J':
		return TYPE_LNG;

	case 'F':
		return TYPE_FLT;

	case 'D':
		return TYPE_DBL;

	case 'L':
	case '[':
		return TYPE_ADR;

	default:
		vm_abort("descriptor_to_basic_type: invalid type %c", descriptor[0]);
		return TYPE_VOID;
	}
}


/****************************************************************************/
/* DEBUG HELPERS                                                            */
/****************************************************************************/

/***
 * Print the given typedesc to the given stream
 */
void descriptor_debug_print_typedesc(FILE *file,typedesc *d) {
	int ch;

	if (!d) {
		fprintf(file,"(typedesc *)NULL");
		return;
	}

	if (d->type == TYPE_ADR) {
		if (d->classref)
			utf_fprint_printable_ascii(file,d->classref->name);
		else
			fprintf(file,"<class=NULL>");
	}
	else {
		switch (d->primitivetype) {
			case PRIMITIVETYPE_INT    : ch='I'; break;
			case PRIMITIVETYPE_CHAR   : ch='C'; break;
			case PRIMITIVETYPE_BYTE   : ch='B'; break;
			case PRIMITIVETYPE_SHORT  : ch='S'; break;
			case PRIMITIVETYPE_BOOLEAN: ch='Z'; break;
			case PRIMITIVETYPE_LONG   : ch='J'; break;
			case PRIMITIVETYPE_FLOAT  : ch='F'; break;
			case PRIMITIVETYPE_DOUBLE : ch='D'; break;
			case PRIMITIVETYPE_VOID   : ch='V'; break;
			default                   : ch='!';
		}
		fputc(ch,file);
	}
	if (d->arraydim)
		fprintf(file,"[%d]",d->arraydim);
}

/***
 * Print the given paramdesc to the given stream
 */
void descriptor_debug_print_paramdesc(FILE *file,paramdesc *d) {
	if (!d) {
		fprintf(file,"(paramdesc *)NULL");
		return;
	}

	if (d->inmemory) {
		fprintf(file,"<m%d>",d->regoff);
	}
	else {
		fprintf(file,"<r%d>",d->regoff);
	}
}

/***
 * Print the given methoddesc to the given stream
 */
void descriptor_debug_print_methoddesc(FILE *file,methoddesc *d) {
	int i;

	if (!d) {
		fprintf(file,"(methoddesc *)NULL");
		return;
	}

	fputc('(',file);
	for (i=0; i<d->paramcount; ++i) {
		if (i)
			fputc(',',file);
		descriptor_debug_print_typedesc(file,d->paramtypes + i);
		if (d->params) {
			descriptor_debug_print_paramdesc(file,d->params + i);
		}
	}
	if (d->params == METHODDESC_NOPARAMS)
		fputs("<NOPARAMS>",file);
	fputc(')',file);
	descriptor_debug_print_typedesc(file,&(d->returntype));
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
