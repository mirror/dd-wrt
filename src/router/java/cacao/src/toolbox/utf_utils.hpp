/* src/toolbox/utf_utils.hpp - functions for handling utf8/utf16

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

#ifndef UTF_UTILS_HPP_
#define UTF_UTILS_HPP_ 1

#include <cassert>
#include <stdint.h>
#include <iterator>

// TODO: Maybe rename transform functions, this is not a transform in the STL sense.
//       It's a reduction, like std::accumulate.

namespace utf_utils {
	/***
	 * A STL style read-only forward iterator.
	 * Iterates over a char* but replaces '/' with '.'
	 */
	template<typename Char>
	struct SlashToDot {
		typedef std::forward_iterator_tag iterator_category;
		typedef const Char*               pointer;
		typedef const Char&               reference;
		typedef Char                      value_type;
		typedef const Char*               difference_type;

		SlashToDot(const Char *cs) : cs(cs) {}

		bool operator==(const SlashToDot& it) const { return cs == it.cs; }
		bool operator!=(const SlashToDot& it) const { return cs != it.cs; }
		bool operator> (const SlashToDot& it) const { return cs >  it.cs; }

		SlashToDot& operator++() {
			cs++;
			return *this;
		}
		SlashToDot operator++(int) {
			SlashToDot it(*this);
			++(*this);
			return it;
		}

		SlashToDot operator+(size_t sz) const { return SlashToDot(cs + sz); }

		Char operator*() const {
			char c = *cs;

			return (c == '/') ? '.' : c;
		}
	private:
		const Char *cs;
	};

	/***
	 * A STL style read-only forward iterator.
	 * Iterates over a char* but replaces '.' with '/'
	 */
	template<typename Char>
	struct DotToSlash {
		typedef std::forward_iterator_tag iterator_category;
		typedef const Char*               pointer;
		typedef const Char&               reference;
		typedef Char                      value_type;
		typedef const Char*               difference_type;

		DotToSlash(const Char *cs) : cs(cs) {}

		bool operator==(const DotToSlash& it) const { return cs == it.cs; }
		bool operator!=(const DotToSlash& it) const { return cs != it.cs; }
		bool operator> (const DotToSlash& it) const { return cs >  it.cs; }

		DotToSlash& operator++() {
			cs++;
			return *this;
		}
		DotToSlash operator++(int) {
			DotToSlash it(*this);
			++(*this);
			return it;
		}

		DotToSlash operator+(size_t sz) const { return DotToSlash(cs + sz); }

		Char operator*() const {
			char c = *cs;

			return (c == '.') ? '/' : c;
		}
	private:
		const Char *cs;
	};

	/***
	 * Helper that wraps a pair of iterators
	 */
	template<typename Iterator>
	struct Range {
		template<typename T>
		Range(T t)                    : _begin(t.begin()), _end(t.end()) {}
		Range(Iterator b, Iterator e) : _begin(b),         _end(e)       {}

		Iterator begin() { return _begin; }
		Iterator end()   { return _end;   }
	private:
		Iterator _begin, _end;
	};
}

namespace utf8 {
	// what the decoder should do when it encounters an error
	enum ErrorAction {
		IGNORE_ERRORS,    // Invalid input leads to undefined behaviour.

		ABORT_ON_ERROR    // The decoding is aborted an the result of
		                  // Visitor::abort() is returned.
	};

	/***
	 *	utf8::transform
	 *
	 *	Iterates over an UTF-8 string and calls a visitor for every UTF-8 byte and
	 *	UTF-16 codepoint encountered.
	 *	How the visitor handles errors is controlled via the enum ErrorAction.
	 *
	 *	A visitor must conform to the following interface:
	 *	 (The class VisitorBase stubs out all of these methods and can be used
	 *	 as a convenient base class)
	 *
	 *		struct Visitor {
	 *			typedef ... ReturnType;
	 *
	 *			ErrorAction error_action(); // called when an error is encountered
	 *
	 *			void utf8(uint8_t);     // called for every valid UTF-8 byte
	 *			void utf16(uint16_t);   // called for every valid UTF-16 codepoint
	 *
	 *			ReturnType finish();    // called on success
	 *			ReturnType abort();     // called on error
	 *			                        // (iff ErrorAction is ABORT_ON_ERROR)
	 *	};
	 *
	 * @Cpp11 Use decltype to get return type of Fn::finish without forcing
	 *        Fn to explicitly contain a typedef.
	 *        We could do this now with GCCs typeof, but that's non-standard.
	 */
	template<typename Iterator, typename Fn>
	typename Fn::ReturnType transform(Iterator begin, Iterator end, Fn);

	template<typename T, typename Fn>
	inline typename Fn::ReturnType transform(T t, Fn fn) {
		return ::utf8::transform(t.begin(), t.end(), fn);
	}


	/***
	 * Handy base class for implementing visitors
	 */
	template<typename ReturnType, ErrorAction action>
	struct VisitorBase {
		ErrorAction error_action() const { return action; }

		void utf8(uint8_t)   const {}
		void utf16(uint16_t) const {}

		ReturnType finish() const { return ReturnType(); }
		ReturnType abort()  const { return ReturnType(); }
	};

	/***
	 * Decodes one utf-16 codepoints from input, automatically advances input
	 * pointer to start of next codepoint.
	 *
	 * Input MUST be valid UTF-8.
	 */
	uint16_t decode_char(const char*&);

	/***
	 * check if char is valid ascii
	 */
	inline bool is_ascii(uint8_t c) { return c < 128; }

	/***
	 * decode utf8 string into utf16 string, destination must have enough space.
	 * returns false on error
	 */
	template<typename Utf8Iterator>
	inline bool decode(Utf8Iterator begin, Utf8Iterator end, uint16_t *dst);

	typedef utf_utils::SlashToDot<char> SlashToDot;
	typedef utf_utils::DotToSlash<char> DotToSlash;

	/***
	 * Wrap iterators of container with SlashToDot
	 */
	template<typename T>
	utf_utils::Range<SlashToDot> slash_to_dot(T t) { return utf_utils::Range<SlashToDot>(t); }

	template<typename It>
	utf_utils::Range<SlashToDot> slash_to_dot(It a, It b) { return utf_utils::Range<SlashToDot>(a, b); }

	/***
	 * Wrap iterators of container with DotToSlash
	 */
	template<typename T>
	utf_utils::Range<DotToSlash> dot_to_slash(T t) { return utf_utils::Range<DotToSlash>(t); }

	template<typename It>
	utf_utils::Range<DotToSlash> dot_to_slash(It a, It b) { return utf_utils::Range<DotToSlash>(a, b); }

} // end namespace utf8

namespace utf16 {
	/***
	 *	utf16::transform
	 *
	 *	Iterates over an UTF-16 string and calls a visitor for every UTF-8 byte and
	 *	UTF-16 codepoint encountered.
	 *
	 *	A visitor must conform to the following interface:
	 *	 (The class VisitorBase stubs out all these methods and can be used
	 *	 as a convenient base class)
	 *
	 *		struct Visitor {
	 *			typedef ... ReturnType;
	 *
	 *			void utf8(uint8_t);     // called for every UTF-8 byte
	 *			void utf16(uint16_t);   // called for every UTF-16 codepoint
	 *
	 *			ReturnType finish();    // called on success
	 *	};
	 *
	 */
	template<typename Iterator, typename Fn>
	typename Fn::ReturnType transform(Iterator begin, Iterator end, Fn);

	template<typename T, typename Fn>
	inline typename Fn::ReturnType transform(T t, Fn fn) {
		return ::utf16::transform(t.begin(), t.end(), fn);
	}


	/***
	 * Handy base class for implementing visitors
	 */
	template<typename ReturnType>
	struct VisitorBase {
		void utf8(uint8_t)   const {}
		void utf16(uint16_t) const {}

		ReturnType finish() const { return ReturnType(); }
	};

	/***
	 * check if char is valid ascii
	 */
	inline bool is_ascii(uint16_t c) { return c < 128; }

	/***
	 * encode utf16 string into utf8 string, destination must have enough space.
	 */
	template<typename Utf16Iterator>
	void encode(Utf16Iterator begin, Utf16Iterator end, char *dst);

	typedef utf_utils::SlashToDot<uint16_t> SlashToDot;
	typedef utf_utils::DotToSlash<uint16_t> DotToSlash;

	/***
	 * Wrap iterators of container with SlashToDot
	 */
	template<typename T>
	utf_utils::Range<SlashToDot> slash_to_dot(T t) { return utf_utils::Range<SlashToDot>(t); }

	template<typename It>
	utf_utils::Range<SlashToDot> slash_to_dot(It a, It b) { return utf_utils::Range<SlashToDot>(a, b); }

	/***
	 * Wrap iterators of container with DotToSlash
	 */
	template<typename T>
	utf_utils::Range<DotToSlash> dot_to_slash(T t) { return utf_utils::Range<DotToSlash>(t); }

	template<typename It>
	utf_utils::Range<DotToSlash> dot_to_slash(It a, It b) { return utf_utils::Range<DotToSlash>(a, b); }

} // end namespace utf16

/*******************************************************************************
	IMPLEMENTATION
*******************************************************************************/

#include "toolbox/utf8_transform.inc"
#include "toolbox/utf16_transform.inc"

#endif // UTF_UTILS_HPP_


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
