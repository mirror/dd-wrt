/* src/vm/string.hpp - string header

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


#ifndef STRING_HPP_
#define STRING_HPP_ 1

#include "config.h"

#include "vm/types.hpp"
#include "vm/global.hpp"
#include "vm/utf8.hpp"

#include <cstdio>
#include <cstring>

namespace cacao {
	class OStream;
}

class JavaString {
	public:
		/*** GLOBAL INITIALIZATION **********************************/

		// initialize string subsystem
		static void initialize();

		// check if string subsystem is initialized
		static bool is_initialized();

		/*** CONSTRUCTORS  ******************************************/

		// creates a new java/lang/String from a utf-text
		static JavaString from_utf8(Utf8String);
		static JavaString from_utf8(const char*, size_t);

		static JavaString from_utf8(const char *cs) {
			return from_utf8(cs, std::strlen(cs));
		}

		// creates a new object of type java/lang/String from a utf-text,
		// changes '/' to '.'
		static JavaString from_utf8_slash_to_dot(Utf8String);
		// creates a new object of type java/lang/String from a utf-text,
		// changes '.' to '/'
		static JavaString from_utf8_dot_to_slash(Utf8String);
		// creates and interns a java/lang/String
		static JavaString literal(Utf8String);

		/// creates a new java/lang/String from a utf16-text
		static JavaString from_utf16(const u2*, size_t);

#ifdef WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH
		/// creates a new java/lang/String with a given char[]
		/// WARNING: the char[] is not copied or validated,
		///          you must make sure it is never changed.
		static JavaString from_array(java_handle_t *array, int32_t count, int32_t offset);
#endif

		/*** ACCESSORS     ******************************************/

		const u2* begin() const;
		const u2* end()   const;

		size_t    size()  const;

		// the number of bytes this string would need
		// in utf-8 encoding
		size_t utf8_size() const;

		/*** CONVERSIONS   ******************************************/

		char*      to_chars() const; // you must free the char* yourself
		Utf8String to_utf8()  const;
		Utf8String to_utf8_dot_to_slash() const;

		/*** MISC          ******************************************/

		JavaString intern() const;

		void fprint(FILE*) const;
		void fprint_printable_ascii(FILE*) const;

		JavaString() : str(0) {}
		JavaString(java_handle_t *h) : str(h) {}

		operator java_handle_t*() const { return str; }

		friend cacao::OStream& operator<<(cacao::OStream&, JavaString);
	private:
		java_handle_t *str;
};

#endif // STRING_HPP_

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
