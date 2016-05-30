/* src/toolbox/utf8_transform.inc - implementation of utf8 decoder

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

#ifndef UTF8_TRANSFORM_INC
#define UTF8_TRANSFORM_INC 1

namespace utf8 {
namespace impl {
	struct CopyUtf8ToUtf16 : utf8::VisitorBase<bool, ABORT_ON_ERROR> {
		typedef bool ReturnType;

		CopyUtf8ToUtf16(uint16_t *dst) : dst(dst) {}

		inline void utf16(uint16_t c) { *dst++ = c; }

		inline bool finish() { return true;  }
		inline bool abort()  { return false; }
	private:
		uint16_t *dst;
	};
} // end namespace impl
} // end namespace utf8

template<typename Iterator, typename Fn>
inline typename Fn::ReturnType utf8::transform(Iterator it, Iterator end, Fn fn) {
	using namespace ::utf8::impl;

#define UTF8_HANDLE_ERROR {                                \
	if (fn.error_action() == ABORT_ON_ERROR) {             \
		return fn.abort();                                 \
	}                                                      \
}

	while (it != end) {
		unsigned byte = *it++;

		if (byte & 0x80) {
			// highest bit set, non-ASCII character

			if ((byte & 0xe0) == 0xc0) {
				// 2-byte: should be 110..... 10......

				if (it == end)
					UTF8_HANDLE_ERROR

				unsigned byte2 = *it++;

				if ((byte2 & 0xc0) != 0x80)
					UTF8_HANDLE_ERROR

				fn.utf8(byte);
				fn.utf8(byte2);
				fn.utf16(((byte & 0x1f) << 6) | (byte2 & 0x3f));
			} else if ((byte & 0xf0) == 0xe0) {
				// 3-byte: should be 1110.... 10...... 10......

				if (it + 2 > end)
					UTF8_HANDLE_ERROR

				unsigned byte2 = *it++;

				if ((byte2 & 0xc0) != 0x80)
					UTF8_HANDLE_ERROR

				unsigned byte3 = *it++;

				if ((byte3 & 0xc0) != 0x80)
					UTF8_HANDLE_ERROR

				fn.utf8(byte);
				fn.utf8(byte2);
				fn.utf8(byte3);
				fn.utf16(((byte & 0x0f) << 12) | ((byte2 & 0x3f) <<  6) | (byte3 & 0x3f));
			} else {
				UTF8_HANDLE_ERROR
			}
		} else {
			// Java forbids zero bytes in UTF8
			if (byte == 0)
				UTF8_HANDLE_ERROR

			// ASCII character: highest bit not set, at least one other bit set
			fn.utf8(byte);
			fn.utf16(byte);
		}
	}

	return fn.finish();

#undef UTF8_HANDLE_ERROR
}


inline uint16_t utf8::decode_char(const char*& src) {
	uint16_t ch1, ch2, ch3;

	ch1 = src[0];

	switch (((uint8_t) ch1) >> 4) {
	default:  // 1 byte (ASCII)
		src++;
		return ch1;
	case 0xC:
	case 0xD: // 2 bytes
		// mask out non-data bits
		ch1  = ch1    & 0x1F;
		ch2  = src[1] & 0x3F;
		src += 2;

		// stitch together data bits from individual bytes
		return (ch1 << 6) | ch2;
	case 0xE: // 3 bytes
		// mask out non-data bits
		ch1  = ch1    & 0x1F;
		ch2  = src[1] & 0x3F;
		ch3  = src[2] & 0x3F;
		src += 3;

		// stitch together data bits from individual bytes
		return (ch1 << 12) | (ch2 << 6) | ch3;
	}
}

template<typename Utf8Iterator>
inline bool utf8::decode(Utf8Iterator begin, Utf8Iterator end, uint16_t *dst) {
	return ::utf8::transform(begin, end, ::utf8::impl::CopyUtf8ToUtf16(dst));
}

#endif // UTF8_TRANSFORM_INC


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
