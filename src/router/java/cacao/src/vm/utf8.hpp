/* src/vm/utf8.hpp - utf8 string functions

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


#ifndef UTF8_HPP_
#define UTF8_HPP_ 1

#include "config.h"                     // used in utf8.inc

#include <cstddef>                      // for size_t
#include <cstdio>                       // for FILE
#include <cstring>                      // for strlen
#include <stdint.h>                     // for uint32_t, uint8_t

#include <iterator>

namespace cacao { class OStream; }
struct utf;

/* Utf8String ******************************************************************

	A container for strings in Java's modified UTF-8 encoding.

	A Utf8String always contains either a valid (possibly empty) UTF-8 string
	or NULL.
	You can check for NULL like you would with any normal pointer.
	Invoking any method except operator void*() or c_ptr() on a NULL string leads to
	undefined behaviour.

	Use a Utf8String like a pointer, i.e. always pass by value.

	The contents of a Utf8String are zero terminated, and it never contains any
	zero bytes except the one at the end, so any C string processing functions
	work properly.

*******************************************************************************/

class Utf8String {
	public:
		/*** GLOBAL INITIALIZATION **********************************/

		// initialize the utf8 subsystem
		// MUST be called before any Utf8String can be constructed
		static void initialize();

		// check if utf8 subsytem is initialized
		static bool is_initialized();

		/*** CONSTRUCTORS  ******************************************/

		// constructs a null string
		Utf8String() : _data(0) {}

		// construct from a buffer with a given length
		// validates that input is really UTF-8
		// constructs a null string on error
		static Utf8String from_utf8(const char*, size_t);
		static Utf8String from_utf8_dot_to_slash(const char*, size_t);
		static Utf8String from_utf8_slash_to_dot(const char*, size_t);

		static Utf8String from_utf8(const char *cs) {
			return from_utf8(cs, strlen(cs));
		}
		static Utf8String from_utf8_dot_to_slash(const char *cs) {
			return from_utf8_dot_to_slash(cs, strlen(cs));
		}

		// construct from a UTF8String
		static Utf8String from_utf8_slash_to_dot(Utf8String);

		// construct from a UTF-16 string with a given length
		static Utf8String from_utf16(const uint16_t*, size_t);
		static Utf8String from_utf16_dot_to_slash(const uint16_t*, size_t);

		// constructs a Utf8String with a given content
		// is only public for interop with legacy C code
		// NOTE: does NOT perform any checks
		Utf8String(utf *u) : _data((Data*) u) {}

		/*** ITERATION     ******************************************/

		// iterator over the bytes in a string
		typedef const char* byte_iterator;

		byte_iterator begin() const { return _data->text; }
		byte_iterator end()   const { return begin() + size(); }

		// iterator over UTF-16 codepoints in a string
		struct utf16_iterator {
			typedef std::input_iterator_tag iterator_category;
			typedef std::ptrdiff_t          difference_type;
			typedef uint16_t                value_type;
			typedef const value_type*       pointer;
			typedef const value_type&       reference;

			uint16_t operator*();

			void operator++() { current = next; }

			bool operator!=(const utf16_iterator& it) {
				return current != it.current;
			}
		private:
			utf16_iterator(byte_iterator it) : current(it), next(it) {}

			byte_iterator current, next;

			friend class Utf8String;
		};

		utf16_iterator utf16_begin() const { return utf16_iterator(begin()); }
		utf16_iterator utf16_end()   const { return utf16_iterator(end());   }

		/*** HASHING       ******************************************/

		size_t hash() const { return _data->hash; }

		/*** COMPARISONS   ******************************************/

		/// check if utf-8 strings contains the same utf-16
		/// codepoints as a utf-16 string
		bool equals(const uint16_t *cs, size_t sz);

		/// check if utf-8 strings contains same bytes as C string
		bool equals(const char *cs) {
			return strcmp(begin(), cs) == 0;
		}

		/*** ACCESSORS     ******************************************/

		// access first element
		char front() const { return begin()[0]; }

		// access last element
		char back() const { return begin()[size() - 1]; }

		char operator[](size_t idx) const { return begin()[idx]; }

		// get the number of bytes in string, excluding zero terminator.
		size_t size() const { return _data->utf8_size; }

		// get the number of utf16 codepoints in string
		size_t utf16_size() const { return _data->utf16_size; }

		// for checking against NULL,
		// also allows interop with legacy C code
		operator void*() const { return _data; }

		utf* c_ptr() const { return (utf*) _data; }

		// create substring
		Utf8String substring(size_t from ) const;
		Utf8String substring(size_t from, size_t to ) const;

		/*** MISC ******************************************/

		bool is_valid_name() const;

		// TODO: remove (only used in loader.cpp)
		static const size_t sizeof_utf;
	private:
		// MUST be a POD type
		struct Data {
			size_t hash;       // cached hash of the string
			size_t utf8_size;  // text length in bytes (does NOT include zero terminator)
			size_t utf16_size; // number of utf16 codepoints in string

			char   text[sizeof(void*)]; // string content
				                        // directly embedded in struct utf
				                        // aligned to pointer size
		};

		static inline Data *alloc(size_t hash,
		                          size_t utf8_size,
		                          size_t utf16_size);

		static void free(Utf8String u);

		Data *_data;

		template<typename Iterator>
		friend struct FromUtf8Builder;

		template<typename Iterator>
		friend struct FromUtf16Builder;
};


// ***** UTF-8 HELPER FUNCTIONS

namespace utf8 {
	// count UTF-16 codepoints, -1 on error
	extern long num_codepoints(const char*, size_t);

	// count how many bytes a utf-8 version would need
	extern size_t num_bytes(const uint16_t*, size_t);

	extern size_t compute_hash(const uint16_t *cs, size_t);

	// named constants for common utf8 strings
	#define UTF8(NAME, STR) extern Utf8String NAME;
	#include "vm/utf8.inc"
}

// these are only used in old logging code

void utf_display_printable_ascii(Utf8String u);
void utf_display_printable_ascii_classname(Utf8String u);

void utf_fprint_printable_ascii(FILE *file, Utf8String u);
void utf_fprint_printable_ascii_classname(FILE *file, Utf8String u);

// OStream operators
namespace cacao {
class OStream;

OStream& operator<<(OStream& os, const Utf8String &u);

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// LEGACY C API
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// these are only used in jvmti and cacaodbg

#define UTF_END(u)     utf8_end(u)
#define UTF_SIZE(u)    utf8_size(u)

extern const char *utf8_end(utf*);
extern size_t      utf8_size(utf*);

void utf_sprint_convert_to_latin1(char *buffer, Utf8String u);
void utf_sprint_convert_to_latin1_classname(char *buffer, Utf8String u);

void utf_strcat_convert_to_latin1(char *buffer, Utf8String u);
void utf_strcat_convert_to_latin1_classname(char *buffer, Utf8String u);

#endif // UTF8_HPP_


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
