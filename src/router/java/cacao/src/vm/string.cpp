/* src/vm/string.cpp - java.lang.String related functions

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

#include "vm/string.hpp"

#include <cassert>

#include "vm/array.hpp"
#include "vm/exceptions.hpp"
#include "vm/globals.hpp"
#include "vm/javaobjects.hpp"
#include "vm/options.hpp"
#include "vm/statistics.hpp"

#include "toolbox/intern_table.hpp"
#include "toolbox/logging.hpp"
#include "toolbox/OStream.hpp"
#include "toolbox/utf_utils.hpp"

using namespace cacao;

STAT_DECLARE_VAR(int,size_string,0)

//****************************************************************************//
//*****          GLOBAL JAVA/LANG/STRING INTERN TABLE                    *****//
//****************************************************************************//

struct InternedJavaString {
	/// Interface to HashTable

	InternedJavaString() : _hash(0), _str(0) {}

	size_t hash() const { return _hash; }
	size_t size() const { return _str.size(); }

	bool is_empty()    const { return _str == (java_object_t*) 0; }
	bool is_occupied() const { return _str != (java_object_t*) 0; }
	bool is_deleted()  const { return false; }

	template<typename T>
	void set_occupied(const T& t) {
		_hash = t.hash();
		_str  = t.get_string();
	}

	template<typename T>
	bool operator==(const T& t) const {
		return hash() == t.hash()
		    && size() == t.size()
		    && std::equal(begin(), end(), t.begin());
	}

	const uint16_t *begin() const { return _str.begin(); }
	const uint16_t *end()   const { return _str.end();   }

	/// used by set_occupied

	JavaString get_string() const { return _str; }
private:
	size_t     _hash;
	JavaString _str;
};


static InternTable<InternedJavaString> intern_table;

//****************************************************************************//
//*****          JAVA STRING SUBSYSTEM INITIALIZATION                    *****//
//****************************************************************************//

/***
 * Initialize string subsystem
 */
void JavaString::initialize() {
	TRACESUBSYSTEMINITIALIZATION("string_init");

	assert(!is_initialized());

	intern_table.initialize(4096);
}

/***
 * Check is string subsystem is initialized
 */
bool JavaString::is_initialized() {
	return intern_table.is_initialized();
}

//****************************************************************************//
//*****          JAVA STRING CONSTRUCTORS                                *****//
//****************************************************************************//

/***
 *	Allocate a new java/lang/String object, fill it with string content
 *	and set its fields.
 *
 *	If input chars is NULL, a NullPointerException is raised.
 *
 *	@param src       iterator range that contain the text for the new string
 *	@param end       end of input
 *	@param dst_size  number of UTF-16 chars new string will contain.
 *
 *	@tparam Iterator A STL style iterator over utf8 chars.
 */
template<typename Iterator>
static inline java_handle_t* makeJavaString(Iterator src, Iterator end, size_t dst_size) {
	if (src == NULL) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}

	// allocate new java/lang/String
	java_handle_t *h = builtin_new(class_java_lang_String);
	if (h == NULL) return NULL;

	// allocate char[] for strings text
	CharArray ca(dst_size);
	if (ca.is_null()) return NULL;

	java_lang_String::set_fields(h, ca.get_handle());

	// copy text into char[]

	if (!utf8::decode(src, end, ca.get_raw_data_ptr()))
		return NULL;

	return h;
}


static inline JavaString allocate_on_system_heap(size_t size) {
	// allocate string
	java_handle_t *h = (java_object_t*) MNEW(uint8_t, class_java_lang_String->instancesize);
	if (h == NULL) return NULL;

	// set string VTABLE and lockword
	Lockword(h->lockword).init();
	h->vftbl = class_java_lang_String->vftbl;

	// allocate array
	java_chararray_t *a = (java_chararray_t*) MNEW(uint8_t, sizeof(java_chararray_t) + sizeof(u2) * size);

	// set array VTABLE, lockword and length
	a->header.objheader.vftbl = Primitive::get_arrayclass_by_type(ARRAYTYPE_CHAR)->vftbl;
	Lockword(a->header.objheader.lockword).init();
	a->header.size            = size;

	java_lang_String::set_fields(h, (java_handle_chararray_t*) a);

	STATISTICS(size_string += sizeof(class_java_lang_String->instancesize));

	return h;
}


/* JavaString::from_utf8 *******************************************************

	Create a new java/lang/String filled with text decoded from an UTF-8 string.
	Returns NULL on error.

*******************************************************************************/

JavaString JavaString::from_utf8(Utf8String u) {
	return makeJavaString(u.begin(), u.end(), u.utf16_size());
}

JavaString JavaString::from_utf8(const char *cs, size_t sz) {
	return makeJavaString(cs, cs + sz, utf8::num_codepoints(cs, sz));
}

/* JavaString::from_utf8_slash_to_dot ******************************************

	Create a new java/lang/String filled with text decoded from an UTF-8 string.
	Replaces '/' with '.'.

	NOTE:
		If the input is not valid UTF-8 the process aborts!

*******************************************************************************/

JavaString JavaString::from_utf8_slash_to_dot(Utf8String u) {
	return makeJavaString<utf8::SlashToDot>(u.begin(), u.end(), u.utf16_size());
}

/* JavaString::from_utf8_dot_to_slash ******************************************

	Create a new java/lang/String filled with text decoded from an UTF-8 string.
	Replaces '.' with '/'.

	NOTE:
		If the input is not valid UTF-8 the process aborts!

*******************************************************************************/

JavaString JavaString::from_utf8_dot_to_slash(Utf8String u) {
	return makeJavaString<utf8::DotToSlash>(u.begin(), u.end(), u.utf16_size());
}

/* JavaString::literal *********************************************************

	Create and intern a java/lang/String filled with text decoded from an UTF-8
	string.

	NOTE:
		because the intern table is allocated on the system heap the GC
		can't see it and thus interned strings must also be allocated on
		the system heap.

*******************************************************************************/

/// Used to lazily construct a java.lang.String literal
struct LiteralBuilder {
	LiteralBuilder(Utf8String u) : _hash(u.hash()), _string(u) {}

	size_t hash() const { return _hash; }
	size_t size() const { return _string.utf16_size(); }

	Utf8String::utf16_iterator begin() const { return _string.utf16_begin(); }
	Utf8String::utf16_iterator end()   const { return _string.utf16_end();   }

	JavaString get_string() const {
		JavaString jstr = allocate_on_system_heap(size());
		assert(jstr);

		bool b = utf8::decode(_string.begin(), _string.end(), const_cast<uint16_t*>(jstr.begin()));
		(void) b;
		assert(b);

		return jstr;
	}
private:
	const size_t     _hash;
	const Utf8String _string;
};

JavaString JavaString::literal(Utf8String u) {
	return intern_table.intern(LiteralBuilder(u)).get_string();
}


/* JavaString:from_utf16 *******************************************************

	Create a new java/lang/String filled with text copied from an UTF-16 string.
	Returns NULL on error.

*******************************************************************************/

JavaString JavaString::from_utf16(const uint16_t *cs, size_t sz) {
	return makeJavaString(cs, cs + sz, sz);
}

/* JavaString:from_utf16 *******************************************************

	Creates a new java/lang/String with a given char[]

	WARNING: the char[] is not copied or validated,
	         you must make sure it is never changed.

*******************************************************************************/

#ifdef WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH

JavaString JavaString::from_array(java_handle_t *array, int32_t count, int32_t offset) {
	java_handle_t *str = builtin_new(class_java_lang_String);
	if (!str)
		return NULL;

	java_lang_String jstr(str);

	jstr.set_value((java_handle_chararray_t*) array);
	jstr.set_count (count);
	jstr.set_offset(offset);

	return str;
}

#endif

/* JavaString::intern **********************************************************

	intern string in global intern table

	NOTE:
		because the intern table is allocated on the system heap the GC
		can't see it and thus interned strings must also be allocated on
		the system heap.

*******************************************************************************/

/// Used to lazily copy a java.lang.String into the intern table
struct LazyStringCopy {
	LazyStringCopy(JavaString str)
	 : _hash(utf8::compute_hash(str.begin(), str.size())),
	   _string(str) {}

	size_t hash() const { return _hash; }
	size_t size() const { return _string.size(); }

	const uint16_t *begin() const { return _string.begin(); }
	const uint16_t *end()   const { return _string.end(); }

	JavaString get_string() const {
		JavaString jstr = allocate_on_system_heap(size());
		EXPENSIVE_ASSERT(jstr);

		std::copy(begin(), end(), const_cast<uint16_t*>(jstr.begin()));

		return jstr;
	}
private:
	const size_t     _hash;
	const JavaString _string;
};

JavaString JavaString::intern() const {
	return intern_table.intern(LazyStringCopy(*this)).get_string();
}

//****************************************************************************//
//*****          JAVA STRING ACCESSORS                                   *****//
//****************************************************************************//

/* JavaString::begin ***********************************************************

	Get the utf-16 contents of string

*******************************************************************************/

const uint16_t* JavaString::begin() const {
	assert(str);

	java_handle_chararray_t *array = java_lang_String::get_value(str);

	if (array == NULL) {
		// this can only happen if the string has been allocated by java code
		// and <init> has not been called on it yet
		return NULL;
	}

	CharArray ca(array);

	int32_t   offset = java_lang_String::get_offset(str);
	uint16_t* ptr    = ca.get_raw_data_ptr();

	return ptr + offset;
}

const uint16_t* JavaString::end() const {
	const uint16_t *ptr = begin();

	return ptr ? ptr + size() : NULL;
}


/* JavaString::size ************************************************************

	Get the number of utf-16 characters in string

*******************************************************************************/

size_t JavaString::size() const {
	assert(str);

	return java_lang_String::get_count(str);
}

/* JavaString::utf8_size *******************************************************

	Get the number of bytes this string would need in utf-8 encoding

*******************************************************************************/

size_t JavaString::utf8_size() const {
	assert(str);

	return utf8::num_bytes(begin(), size());
}

//****************************************************************************//
//*****          JAVA STRING CONVERSIONS                                 *****//
//****************************************************************************//

/* JavaString::to_chars ********************************************************

	Decodes java/lang/String into newly allocated string (debugging)

	NOTE:
		You must free the string allocated yourself with MFREE

*******************************************************************************/

char *JavaString::to_chars() const {
	if (str == NULL) return MNEW(char, 1); // memory is zero initialized

	size_t sz = size();

	const uint16_t *src = begin();
	const uint16_t *end = src + sz;

	char *buf = MNEW(char, sz + 1);
	char *dst = buf;

	while (src != end) *dst++ = *src++;

	*dst = '\0';

	return buf;
}

/* JavaString::to_utf8() *******************************************************

	make utf symbol from java.lang.String

*******************************************************************************/

Utf8String JavaString::to_utf8() const {
	if (str == NULL) return utf8::empty;

	return Utf8String::from_utf16(begin(), size());
}

/* JavaString::to_utf8_dot_to_slash() ******************************************

	make utf symbol from java.lang.String
	replace '/' with '.'

*******************************************************************************/

Utf8String JavaString::to_utf8_dot_to_slash() const {
	if (str == NULL) return utf8::empty;

	return Utf8String::from_utf16_dot_to_slash(begin(), size());
}

//****************************************************************************//
//*****          JAVA STRING IO                                          *****//
//****************************************************************************//

/* JavaString::fprint **********************************************************

   Print the given Java string to the given stream.

*******************************************************************************/

void JavaString::fprint(FILE *stream) const
{
	const uint16_t* cs = begin();
	size_t          sz = size();

	for (size_t i = 0; i < sz; i++) {
		char c = cs[i];

		fputc(c, stream);
	}
}

void JavaString::fprint_printable_ascii(FILE *stream) const
{
	const uint16_t* cs = begin();
	size_t          sz = size();

	for (size_t i = 0; i < sz; i++) {
		char c = cs[i];

		c = (c >= 32 && (unsigned char)c <= 127) ? c : '?';

		fputc(c, stream);
	}
}

OStream& operator<<(OStream& os, JavaString js) {
	if (!js)
		return os << "<null string>";

	const u2 *cs = js.begin();

	if (cs == NULL) {
		// string has been allocated by java code
		// but <init> has not been called on it yet.
		return os << "<uninitialized string>";
	} else {
		os << '"';

		for (const u2 *end = js.end(); cs != end; ++cs) {
			os << ((char) *cs);
		}

		os << '"';

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
