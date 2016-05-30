/* src/vm/utf8.cpp - utf8 string functions

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

#include "vm/utf8.hpp"
#include <algorithm>                    // for std::equal
#include "mm/memory.hpp"                // for mem_alloc, mem_free
#include "toolbox/logging.hpp"          // for OStream
#include "toolbox/intern_table.hpp"     // for InternTable
#include "toolbox/utf_utils.hpp"        // for transform, Tag, etc
#include "vm/options.hpp"
#include "vm/statistics.hpp"
#include "toolbox/assert.hpp"

using namespace cacao;

STAT_REGISTER_VAR(int,count_utf_new,0,"utf new","Calls of utf_new")
STAT_DECLARE_VAR(int,count_utf_len,0)

//****************************************************************************//
//*****          GLOBAL UTF8-STRING INTERN TABLE                         *****//
//****************************************************************************//

// used to for tag dispatch
struct utf8_tag  {};
struct utf16_tag {};

struct InternedUtf8String {
	InternedUtf8String()             : string(0) {}
	InternedUtf8String(Utf8String u) : string(u) {}

	/// Interface to HashTable

	bool is_empty()    const { return string == ((utf*) 0); }
	bool is_occupied() const { return string != ((utf*) 0); }
	bool is_deleted()  const { return false; }

	template<typename T>
	void set_occupied(const T& t) { string = t.get_string(); }

//	template<typename Iterator>
//	bool operator==(const FromUtf16Builder<Iterator>& t) const;

	template<typename T>
	bool operator==(const T& t) const {
		return equal(t.hash(), t.size(), t.begin(), t.tag());
	}

	template<typename Iterator>
	bool equal(size_t _hash, size_t _size, Iterator it, utf8_tag) const {
		return hash() == _hash
		    && size() == _size
		    && std::equal(it, it + _size, begin());
	}

	template<typename Iterator>
	bool equal(size_t _hash, size_t _size, Iterator it, utf16_tag) const {
		return hash()       == _hash
		    && utf16_size() == _size
		    && std::equal(it, it + _size, utf16_begin());
	}

	/// used by operator==

	utf8_tag tag() const { return utf8_tag(); }

	size_t hash() const { return string.hash(); }
	size_t size() const { return string.size(); }

	size_t utf16_size() const { return string.utf16_size(); }

	Utf8String::byte_iterator begin() const { return string.begin(); }
	Utf8String::byte_iterator end()   const { return string.end();   }

	Utf8String::utf16_iterator utf16_begin() const { return string.utf16_begin(); }
	Utf8String::utf16_iterator utf16_end()   const { return string.utf16_end();   }

	/// used by set_occupied

	Utf8String get_string() const { return string; }
private:
	Utf8String string;
};

static InternTable<InternedUtf8String> intern_table;

// initial size of intern table
#define HASHTABLE_UTF_SIZE 16384

void Utf8String::initialize(void)
{
	TRACESUBSYSTEMINITIALIZATION("utf8_init");

	assert(!is_initialized());

	intern_table.initialize(HASHTABLE_UTF_SIZE);

	STATISTICS(count_utf_len += sizeof(utf*) * HASHTABLE_UTF_SIZE);

	// create utf-symbols for pointer comparison of frequently used strings

#define UTF8(NAME, STR) utf8::NAME = Utf8String::from_utf8(STR);
#include "vm/utf8.inc"
}


/* Utf8String::initialize ******************************************************

   Check if utf8 subsytem is initialized

*******************************************************************************/

bool Utf8String::is_initialized(void)
{
	return intern_table.is_initialized();
}

//****************************************************************************//
//*****          INTERNAL DATA REPRESENTATION                            *****//
//****************************************************************************//

/// allocate a Utf8String with given hash and size
/// You still have to fill in the strings text!
inline Utf8String::Data* Utf8String::alloc(size_t hash,
		                                   size_t utf8_size,
		                                   size_t utf16_size) {
	Data* str = (Data*) mem_alloc(offsetof(Data,text) + utf8_size + 1);

	STATISTICS(count_utf_new++);

	str->hash       = hash;
	str->utf8_size  = utf8_size;
	str->utf16_size = utf16_size;

	return str;
}


//****************************************************************************//
//*****          HASHING                                                 *****//
//****************************************************************************//

/* init/update/finish_hash *****************************************************

	These routines are used to compute the hash for a utf-8 string byte by byte.

	Use like this:
		size_t hash = 0;

		for each byte in string:
			hash = update_hash( hash, byte );

		hash = finish_hash(hash);

	The algorithm is the "One-at-a-time" algorithm as published
	by Bob Jenkins on http://burtleburtle.net/bob/hash/doobs.html.

*******************************************************************************/

static inline size_t update_hash(size_t hash, uint8_t byte)
{
	hash += byte;
	hash += (hash << 10);
	hash ^= (hash >> 6);

	return hash;
}

static inline size_t finish_hash(size_t hash)
{
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}


//****************************************************************************//
//*****          UTF-8 STRING                                            *****//
//****************************************************************************//

// create & intern string

// Builds a new utf8 string.
// Only allocates a new string if the string was not already intern_table.
template<typename Iterator>
struct FromUtf8Builder : utf8::VisitorBase<Utf8String, utf8::ABORT_ON_ERROR> {
	FromUtf8Builder(Iterator text, size_t utf8_size)
	 : _hash(0), _utf8_size(utf8_size), _utf16_size(0), _text(text) {}

	/// interface to utf8::transform

	typedef Utf8String ReturnType;

	void utf8 (uint8_t  c) {
		_hash = update_hash(_hash, c);
	}

	void utf16(uint16_t c) {
		_utf16_size++;
	}

	Utf8String finish() {
		_hash = finish_hash(_hash);

		return intern_table.intern(*this).get_string();
	}

	Utf8String abort() {
		return 0;
	}

	/// interface to HashTable

	size_t hash() const { return _hash; }

	/// interface to InternTableEntry

	utf8_tag tag() const { return utf8_tag(); }

	Iterator begin() const { return _text; }

	size_t size() const { return _utf8_size; }

	Utf8String get_string() const {
		Utf8String::Data *u  = Utf8String::alloc(_hash, _utf8_size, _utf16_size);
		char             *cs = u->text;

		cs  = std::copy(_text, _text + _utf8_size, cs);
		*cs = '\0';

		return (utf*) u;
	}
private:
	size_t   _hash;
	size_t   _utf8_size;
	size_t   _utf16_size;
	Iterator _text;
};


// Builds a new utf8 string from an utf16 string.
// Only allocates a new string if the string was not already intern_table.
template<typename Iterator>
struct FromUtf16Builder : utf8::VisitorBase<Utf8String, utf8::ABORT_ON_ERROR> {
	FromUtf16Builder(Iterator text, size_t utf16_size)
	 : _hash(0), _utf8_size(0), _utf16_size(utf16_size), _text(text) {}

	/// interface to utf8::transform

	typedef Utf8String ReturnType;

	void utf8 (uint8_t  c) {
		_hash = update_hash(_hash, c);
		_utf8_size++;
	}

	Utf8String finish() {
		_hash = finish_hash(_hash);

		return intern_table.intern(*this).get_string();
	}

	Utf8String abort() {
		return 0;
	}

	/// interface to HashTable

	size_t hash() const { return _hash; }

	/// interface to InternTableEntry

	utf16_tag tag() const { return utf16_tag(); }

	Iterator begin() const { return _text; }

	size_t size() const { return _utf16_size; }

	Utf8String get_string() const {
		Utf8String::Data *u  = Utf8String::alloc(_hash, _utf8_size, _utf16_size);
		char             *cs = u->text;

		utf16::encode(_text, _text + _utf16_size, cs);
		cs[_utf8_size] = '\0';

		return (utf*) u;
	}
private:
	size_t   _hash;
	size_t   _utf8_size;
	size_t   _utf16_size;
	Iterator _text;
};


template<typename Iterator>
static inline Utf8String string_from_utf8(const char *cs, size_t size) {
	Iterator begin = cs;
	Iterator end   = cs + size;

	return utf8::transform(begin, end, FromUtf8Builder<Iterator>(begin, size));
}

template<typename Iterator>
static inline Utf8String string_from_utf16(const uint16_t *cs, size_t size) {
	Iterator begin = cs;
	Iterator end   = cs + size;

	return utf16::transform(begin, end, FromUtf16Builder<Iterator>(begin, size));
}


Utf8String Utf8String::from_utf8(const char *cs, size_t sz) {
	return string_from_utf8<const char*>(cs, sz);
}

Utf8String Utf8String::from_utf8_dot_to_slash(const char *cs, size_t sz) {
	return string_from_utf8<utf8::DotToSlash>(cs, sz);
}

Utf8String Utf8String::from_utf8_slash_to_dot(const char *cs, size_t sz) {
	return string_from_utf8<utf8::SlashToDot>(cs, sz);
}

Utf8String Utf8String::from_utf8_slash_to_dot(Utf8String u) {
	return string_from_utf8<utf8::SlashToDot>(u.begin(), u.size());
}

Utf8String Utf8String::from_utf16(const uint16_t *cs, size_t sz) {
	return string_from_utf16<const uint16_t*>(cs, sz);
}

Utf8String Utf8String::from_utf16_dot_to_slash(const uint16_t *cs, size_t sz) {
	return string_from_utf16<utf16::DotToSlash>(cs, sz);
}

/* Utf8String::utf16_iterator **************************************************

	A forward iterator over the utf16 codepoints in a Utf8String

*******************************************************************************/

uint16_t Utf8String::utf16_iterator::operator*()
{
	return utf8::decode_char(next);
}


/* Utf8String::substring *******************************************************

	Access last element, accessing a null or empty string leads to
	undefined behaviour

*******************************************************************************/

Utf8String Utf8String::substring(size_t from) const
{
	return substring(from, size());
}

Utf8String Utf8String::substring(size_t from, size_t to) const
{
	EXPENSIVE_ASSERT(_data);
	EXPENSIVE_ASSERT(from >  0);
	EXPENSIVE_ASSERT(from <= to);
	EXPENSIVE_ASSERT(to   <= size());

	return Utf8String::from_utf8(begin() + from, to - from);
}

bool Utf8String::is_valid_name() const {
	Utf8String::byte_iterator it  = this->begin();
	Utf8String::byte_iterator end = this->end();

	for (; it != end; it++) {
		unsigned char c = *it;

		if (c < 0x20)
			return false; // disallow control characters
		if (c == 0xc0 && ((unsigned char) it[1]) == 0x80)
			return false; // disallow zero
	}

	return true;
}

//****************************************************************************//
//*****          PUBLIC UTF-8 FUNCTIONS                                  *****//
//****************************************************************************//

/* Utf8String::initialize ******************************************************

   Initializes the utf8 subsystem.

*******************************************************************************/

/* utf8::num_codepoints ********************************************************

	Count number of UTF-16 code points in UTF-8 string.

	Returns -1 on error

*******************************************************************************/

struct SafeCodePointCounter : utf8::VisitorBase<long, utf8::ABORT_ON_ERROR> {
	typedef long ReturnType;

	SafeCodePointCounter() : count(0) {}

	void utf16(uint16_t) { count++; }

	long finish() { return count; }
	long abort()  { return -1;    }
private:
	long count;
};

long utf8::num_codepoints(const char *cs, size_t sz) {
	return utf8::transform(cs, cs + sz, SafeCodePointCounter());
}

/* utf8::num_bytes *************************************************************

	Calculate how many bytes a UTF-8 encoded version of a UTF-16 string
	would need.

*******************************************************************************/

struct ByteCounter : utf8::VisitorBase<size_t, utf8::IGNORE_ERRORS> {
	typedef size_t ReturnType;

	ByteCounter() : count(0) {}

	void utf8(uint8_t) { count++; }

	size_t finish() { return count; }
private:
	size_t count;
};

size_t utf8::num_bytes(const uint16_t *cs, size_t sz)
{
	return utf16::transform(cs, cs + sz, ByteCounter());
}


/***
 * Compute the hash of a UTF-16 string.
 * The hash will be the same as for the UTF-8 encoded version of this string
 */
struct Utf16Hasher : utf16::VisitorBase<size_t> {
	typedef size_t ReturnType;

	Utf16Hasher() : hash(0) {}

	void utf8(uint8_t c) {
		hash = update_hash(hash, c);
	}

	size_t finish() { return finish_hash(hash); }
private:
	size_t hash;
};

size_t utf8::compute_hash(const uint16_t *cs, size_t sz) {
	return utf16::transform(cs, cs + sz, Utf16Hasher());
}


//****************************************************************************//
//*****          GLOBAL UTF8-STRING CONSTANTS                            *****//
//****************************************************************************//

#define UTF8( NAME, STR ) Utf8String utf8::NAME;
#include "vm/utf8.inc"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// LEGACY C API
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

extern const char *utf8_text(utf *u) { return Utf8String(u).begin(); }
extern const char *utf8_end (utf *u) { return Utf8String(u).end();   }

extern size_t utf8_size(utf *u) { return Utf8String(u).size(); }
extern size_t utf8_hash(utf *u) { return Utf8String(u).hash(); }

/* utf_display_printable_ascii *************************************************

   Write utf symbol to stdout (for debugging purposes).
   Non-printable and non-ASCII characters are printed as '?'.

*******************************************************************************/

struct DisplayPrintableAscii : utf8::VisitorBase<void, utf8::IGNORE_ERRORS> {
	typedef void ReturnType;

	DisplayPrintableAscii(FILE *dst) : _dst(dst) {}

	void utf8(uint8_t c) {
		fputc((c >= 32 && c <= 127) ? c : '?', _dst);
	}

	void finish() {fflush(_dst);}
private:
	FILE *_dst;
};

void utf_display_printable_ascii(Utf8String u)
{
	if (u == NULL) {
		printf("NULL");
		fflush(stdout);
		return;
	}

	utf8::transform(u, DisplayPrintableAscii(stdout));
}


/* utf_display_printable_ascii_classname ***************************************

   Write utf symbol to stdout with `/' converted to `.' (for debugging
   purposes).
   Non-printable and non-ASCII characters are printed as '?'.

*******************************************************************************/

void utf_display_printable_ascii_classname(Utf8String u)
{
	if (u == NULL) {
		printf("NULL");
		fflush(stdout);
		return;
	}

	utf8::transform(utf8::slash_to_dot(u), DisplayPrintableAscii(stdout));
}


/* utf_sprint_convert_to_latin1 ************************************************

   Write utf symbol into c-string (for debugging purposes).
   Characters are converted to 8-bit Latin-1, non-Latin-1 characters yield
   invalid results.

*******************************************************************************/

struct SprintConvertToLatin1 : utf8::VisitorBase<void, utf8::IGNORE_ERRORS> {
	typedef void ReturnType;

	SprintConvertToLatin1(char* dst) : _dst(dst) {}

	void utf16(uint16_t c) { *_dst++ = c; }

	void finish() { *_dst = '\0'; }
private:
	char *_dst;
};

void utf_sprint_convert_to_latin1(char *buffer, Utf8String u)
{
	if (!u) {
		strcpy(buffer, "NULL");
		return;
	}

	utf8::transform(u, SprintConvertToLatin1(buffer));
}


/* utf_sprint_convert_to_latin1_classname **************************************

   Write utf symbol into c-string with `/' converted to `.' (for debugging
   purposes).
   Characters are converted to 8-bit Latin-1, non-Latin-1 characters yield
   invalid results.

*******************************************************************************/

void utf_sprint_convert_to_latin1_classname(char *buffer, Utf8String u)
{
	if (!u) {
		strcpy(buffer, "NULL");
		return;
	}

	utf8::transform(utf8::slash_to_dot(u), SprintConvertToLatin1(buffer));
}


/* utf_strcat_convert_to_latin1 ************************************************

   Like libc strcat, but uses an utf8 string.
   Characters are converted to 8-bit Latin-1, non-Latin-1 characters yield
   invalid results.

*******************************************************************************/

void utf_strcat_convert_to_latin1(char *buffer, utf *u)
{
	utf_sprint_convert_to_latin1(buffer + strlen(buffer), u);
}


/* utf_strcat_convert_to_latin1_classname **************************************

   Like libc strcat, but uses an utf8 string.
   Characters are converted to 8-bit Latin-1, non-Latin-1 characters yield
   invalid results.

*******************************************************************************/

void utf_strcat_convert_to_latin1_classname(char *buffer, Utf8String u)
{
	utf_sprint_convert_to_latin1_classname(buffer + strlen(buffer), u);
}


/* utf_fprint_printable_ascii **************************************************

   Write utf symbol into file.
   Non-printable and non-ASCII characters are printed as '?'.

*******************************************************************************/

void utf_fprint_printable_ascii(FILE *file, Utf8String u)
{
	if (!u) return;

	utf8::transform(u, DisplayPrintableAscii(file));
}


/* utf_fprint_printable_ascii_classname ****************************************

   Write utf symbol into file with `/' converted to `.'.
   Non-printable and non-ASCII characters are printed as '?'.

*******************************************************************************/

void utf_fprint_printable_ascii_classname(FILE *file, Utf8String u)
{
	if (!u) return;

	utf8::transform(utf8::slash_to_dot(u), DisplayPrintableAscii(file));
}

const size_t Utf8String::sizeof_utf = sizeof(Utf8String::Data);

namespace cacao {

// OStream operators
OStream& operator<<(OStream& os, const Utf8String &u) {
  return os << (u ? u.begin() : "(nil)");
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
