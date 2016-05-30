/* src/toolbox/utf16_transform.inc - implementation of utf16 encoder

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

#ifndef UTF16_TRANSFORM_INC
#define UTF16_TRANSFORM_INC 1

namespace utf16 {

namespace impl {
	struct CopyUtf16ToUtf8 : utf8::VisitorBase<void, utf8::IGNORE_ERRORS> {
		typedef void ReturnType;

		CopyUtf16ToUtf8(char *dst) : dst(dst) {}

		void utf8(uint8_t c) { *dst++ = c; }

		void finish() { *dst = '\0'; }
	private:
		char *dst;
	};
} // end namespace impl

	template<typename Iterator, typename Fn>
	inline typename Fn::ReturnType transform(Iterator it, Iterator end, Fn fn) {
		for (; it != end; ++it) {
			uint16_t c = *it;

			fn.utf16(c);

			if ((c != 0) && (c < 0x80)) {
				// 1 character
				fn.utf8((char) c);
			} else if (c < 0x800) {
				// 2 characters
	    		unsigned char high = c >> 6;
	    		unsigned char low  = c & 0x3F;

				fn.utf8(high | 0xC0);
				fn.utf8(low  | 0x80);
			} else {
		    	// 3 characters
		    	char low  = c & 0x3f;
		    	char mid  = (c >> 6) & 0x3F;
		    	char high = c >> 12;

				fn.utf8(high | 0xE0);
				fn.utf8(mid  | 0x80);
				fn.utf8(low  | 0x80);
			}
		}

		return fn.finish();
	}


	template<typename Utf16Iterator>
	inline void encode(Utf16Iterator begin, Utf16Iterator end, char *dst) {
		::utf16::transform(begin, end, ::utf16::impl::CopyUtf16ToUtf8(dst));
	}

} // end namespace utf16


#endif // UTF16_TRANSFORM_INC

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
