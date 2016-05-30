/* src/toolbox/buffer.hpp - String buffer header

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


#ifndef BUFFER_HPP_
#define BUFFER_HPP_ 1

#include <inttypes.h>
#include <cstdarg>
#include "mm/memory.hpp"
#include "toolbox/utf_utils.hpp"
#include "vm/global.hpp"            // for MAX
#include "vm/utf8.hpp"
#include "vm/string.hpp"


#ifndef __va_copy
#define __va_copy(d,s)	__builtin_va_copy(d,s)
#endif
/* Buffer **********************************************************************

	An in memory buffer.

	The buffer automatically grows to fit it's input.

	When a buffer goes out of scope all memory associated with it will be freed

	Don't forget to zero terminate contents of buffer if you use them directly.

	TEMPLATE PARAMETERS:
		Allocator ... The type of allocator to use for allocating the buffer.
		              Must support the non-standard reallocate function.

*******************************************************************************/

template<typename Allocator = MemoryAllocator<uint8_t> >
class Buffer {
	public:
		// construct a buffer with size
		Buffer(size_t initial_size = 64);

		// free content of buffer
		~Buffer();

		// *********************************************************************
		// ***** WRITE TO BUFFER

		// write to buffer byte-by-byte
		inline Buffer& write(char);
		inline Buffer& write(Utf8String);
		inline Buffer& write(JavaString);
		inline Buffer& write(const char*);
		inline Buffer& write(const char*, size_t);
		inline Buffer& write(const uint16_t*,   size_t);

		/// write to buffer, replacing '/' by '.'
		inline Buffer& write_slash_to_dot(const char*);
		inline Buffer& write_slash_to_dot(Utf8String);

		/// write to buffer, replacing '.' by '/'
		inline Buffer& write_dot_to_slash(Utf8String);

		/// copy contents of buffer (calls c_str() on src)
		template<typename A>
		inline Buffer& write(Buffer<A>& src) { return write(src.c_str()); }

		/// write address of pointer as hex to buffer
		inline Buffer& write_ptr(void*);

		/// write number to buffer as decimal
		inline Buffer& write_dec(s4);
		inline Buffer& write_dec(s8);
		inline Buffer& write_dec(float);
		inline Buffer& write_dec(double);

		/// write number to buffer as hexadecimal
		inline Buffer& write_hex(s4);
		inline Buffer& write_hex(s8);
		inline Buffer& write_hex(float);
		inline Buffer& write_hex(double);

		// like printf
		inline Buffer& writef(const char* fmt, ...);
		inline Buffer& writevf(const char* fmt, va_list ap);

		/// ensure string in buffer is zero terminated
		inline Buffer& zero_terminate();

		// *********************************************************************
		// ***** GET CONTENTS OF BUFFER

		/// get contents of buffer as zero-terminated c-style-string
		/// This strings lifetime is tied to it's buffer.
		inline const char* c_str();

		/// get copy contents of buffer as zero-terminated c-style-string
		/// You must free the returned string yourself (
		/// use a copy of the buffer's allocator via get_allocator())
		inline const char* c_str_copy();

		/// get utf-8 string contents of buffer as utf8-string
		inline Utf8String utf8_str();

		/// get raw contents of buffer (not necessarily zero terminated).
		inline uint8_t* data();
		inline const uint8_t* data() const;

		/// get size of buffer contents
		inline size_t size();

		// *********************************************************************
		// ***** CHANGE BUFFER POSITION

		/// Reset buffer position to start of buffer.
		/// This effectively clears the buffer.
		/// O(1)
		inline void reset();

		/// advance buffer position by n bytes
		/// O(1)
		inline void skip(size_t num_bytes);

		/// remove data from the back of this buffer.
		/// O(1)
		inline void rewind(size_t bytes_to_drop);

		// *********************************************************************
		// ***** MISC

		/// ensure buffer contains space for at least sz bytes
		void ensure_capacity(size_t sz);
	private:
		/// non-copyable
		Buffer(const Buffer&);
		///non-assignable
		Buffer& operator=(const Buffer&);

		uint8_t  *_start, *_end, *_pos;
		Allocator _alloc;

		// used to encode utf16 strings to utf8
		struct Encode : utf16::VisitorBase<void> {
			typedef void ReturnType;

			Encode(Buffer& dst) : _dst(dst) {}

			void utf8 (uint8_t c) { _dst.write(c); }
		private:
			Buffer& _dst;
		};
};

//****************************************************************************//
//*****          IMPLEMENTATION                                          *****//
//****************************************************************************//

/* Buffer::Buffer **************************************************************

	Construct a new Buffer with a given size.

	IN:
		buf_size ....... The number of bytes that should be preallocated
		                 for input in the buffer.

*******************************************************************************/

template<typename Allocator>
Buffer<Allocator>::Buffer(size_t initial_size)
{
	_start        = _alloc.allocate(initial_size);
	_end          = _start + initial_size;
	_pos          = _start;
}

template<typename Allocator>
Buffer<Allocator>::~Buffer()
{
	_alloc.deallocate(_start, _end - _start);
	_start = _end = _pos = 0;
}

/* Buffer::write(Utf8String) ***************************************************

	Insert a utf-8 string into buffer byte by byte.
	Does NOT inserts a zero terminator.

*******************************************************************************/

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write(Utf8String u)
{
	return write(u.begin(), u.size());
}

/* Buffer::write(JavaString) ***************************************************

	Decode a java lang string into buffer
	Does NOT inserts a zero terminator.

*******************************************************************************/

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write(JavaString js)
{
	return write(js.begin(), js.size());
}

/* Buffer::write(const char*) **************************************************

	Insert a zero terminated string into buffer byte by byte
	Does NOT inserts a zero terminator.

*******************************************************************************/

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write(const char *cs)
{
	return write(cs, strlen(cs));
}

/* Buffer::write(const char*, size_t) ********************************************

	Insert string with a given length into buffer byte by byte
	Does NOT inserts a zero terminator.

*******************************************************************************/

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write(const char *cs, size_t sz)
{
	ensure_capacity(sz);

	memcpy(_pos, cs, sizeof(char) * sz);

	_pos += sz;

	return *this;
}

/* Buffer::write(const uint16_t*, size_t) **************************************

	Encode utf-16 string with a given length into buffer
	Does NOT inserts a zero terminator.

*******************************************************************************/

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write(const uint16_t *cs, size_t sz)
{
	utf16::transform(cs, cs + sz, Encode(*this));

	return *this;
}

/* Buffer::write(char) *********************************************************

	Write a char into buffer.
	Does NOT inserts a zero terminator.

*******************************************************************************/

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write(char c)
{
	ensure_capacity(1);

	*_pos++ = c;

	return *this;
}

/* Buffer::write_slash_to_dot(Utf8String) **************************************

	Insert a utf-8 string into buffer byte by byte, replacing '/' by '.'.
	Does NOT inserts a zero terminator.

*******************************************************************************/

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write_slash_to_dot(const char *cs) {
	size_t sz = std::strlen(cs);

	ensure_capacity(sz);

	const char* src = cs;
	const char* end = cs + sz;

	for ( ; src != end; ++_pos, ++src ) {
		char c = *src;

		*_pos = (c == '/') ? '.' : c;
	}

	return *this;
}

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write_slash_to_dot(Utf8String u) {
	ensure_capacity(u.size());

	const char* src = u.begin();
	const char* end = u.end();

	for ( ; src != end; ++_pos, ++src ) {
		char c = *src;

		*_pos = (c == '/') ? '.' : c;
	}

	return *this;
}

/* Buffer::write_dot_to_slash(Utf8String) **************************************

	Insert a utf-8 string into buffer byte by byte, replacing '.' by '/'.
	Does NOT inserts a zero terminator.

*******************************************************************************/

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write_dot_to_slash(Utf8String u) {
	ensure_capacity(u.size());

	const char* src = u.begin();
	const char* end = u.end();

	for ( ; src != end; ++_pos, ++src ) {
		char c = *src;

		*_pos = (c == '.') ? '/' : c;
	}

	return *this;
}

/* Buffer::write_ptr/dec/hex **************************************************


	Like (v)snprintf but for buffers.

*******************************************************************************/

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write_ptr(void *ptr) {
	return writef("0x%" PRIxPTR, (uintptr_t) ptr);
}

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write_dec(s4 n) {
	return writef("%d", n);
}

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write_dec(s8 n) {
	return writef("0x%" PRId64, n);
}

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write_dec(float n) {
	return writef("%g", n);
}

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write_dec(double n) {
	return writef("%g", n);
}

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write_hex(s4 n) {
	return writef("%08x", n);
}

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write_hex(s8 n) {
	return writef("0x%" PRIx64, n);
}

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write_hex(float n) {
	union {
		float f;
		s4    i;
	} u;

	u.f = n;

	return write_hex(u.i);
}

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::write_hex(double n) {
	union {
		double d;
		s8     l;
	} u;

	u.d = n;

	return write_hex(u.l);
}

/* Buffer::writef/writevf ******************************************************

	Like (v)snprintf but for buffers.

*******************************************************************************/

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::writef(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	  writevf(fmt,ap);
	va_end(ap);

	return *this;
}

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::writevf(const char *fmt, va_list ap)
{
	va_list ap2;
	__va_copy(ap2, ap); // unfortunately va_copy is only exposed for C99/C++11 or later

	size_t size    = _end - _pos;
	size_t written = vsnprintf((char*) _pos, size, fmt, ap);

	if (written > size) {
		// buffer was too small (+1 for zero byte)
		ensure_capacity(written + 1);

		size    = _end - _pos;
		written = vsnprintf((char*) _pos, size, fmt, ap2);
		assert(written <= size);
	}

	_pos += written;

	va_end(ap2);

	return *this;
}

/* Buffer::zero_terminate ******************************************************

	Ensure content of buffer is a zero terminated string.
	Does not alter the buffers position.

*******************************************************************************/

template<typename Allocator>
Buffer<Allocator>& Buffer<Allocator>::zero_terminate()
{
	ensure_capacity(1);

	*_pos = '\0';

	return *this;
}

/* Buffer::c_str() *************************************************************

	Returns the buffers contents as read only c-style string.
	The string remains valid only as long as the buffer exists.

	Automatically ensures that this buffers contents are zero terminated

*******************************************************************************/

template<typename Allocator>
const char* Buffer<Allocator>::c_str()
{
	zero_terminate();

	return (const char*) _start;
}

/* Buffer::utf8_str() **********************************************************

	Create a new Utf8String whose contents are equal to the contents of this
	buffer.

	Automatically ensures that this buffers contents are zero terminated

*******************************************************************************/

template<typename Allocator>
Utf8String Buffer<Allocator>::utf8_str()
{
	zero_terminate();

	return Utf8String::from_utf8((char*) _start, _pos - _start);
}

/* Buffer::data() **************************************************************

	get raw contents of buffer (not necessarily zero terminated).
	The pointer remains valid only as long as the buffer exists.

	Returns NULL if buffer is empty

*******************************************************************************/

template<typename Allocator>
uint8_t* Buffer<Allocator>::data()
{
	return _start;
}

template<typename Allocator>
const uint8_t* Buffer<Allocator>::data() const
{
	return _start;
}

/* Buffer::size() **************************************************************

	get size of buffer contents

*******************************************************************************/

template<typename Allocator>
size_t Buffer<Allocator>::size()
{
	return _pos - _start;
}

/* Buffer::reset ***************************************************************

	Reset buffer position to start of buffer.

*******************************************************************************/

template<typename Allocator>
void Buffer<Allocator>::reset()
{
	_pos = _start;
}

/* Buffer::rewind **************************************************************

	Undo write operations by removing data of the back of this buffer.
	Does not perform a bounds check.

	IN:
		bytes_to_drop ... how many bytes of content should be removed from the
		                  back of the buffer.

	NOTE:
		The content of the buffer is not necesserily valid utf-8 or
		null terminated after calling this.

*******************************************************************************/

template<typename Allocator>
void Buffer<Allocator>::rewind(size_t bytes_to_drop)
{
	_pos -= bytes_to_drop;
}

/* Buffer::ensure_capacity *****************************************************

	Automatically grows buffer if doesn't have enough space.

	IN:
		write_size ... the number of bytes that will be written by the next
		               write operation. Buffer will be resized if it doesn't
		               have enough space to satisfy that write.

*******************************************************************************/

template<typename Allocator>
void Buffer<Allocator>::ensure_capacity(size_t write_size)
{
	size_t free_space = _end - _pos;

	if (free_space < write_size) {
		// increase capacity
		size_t old_size     = _pos - _start;
		size_t old_capacity = _end - _start;

		size_t new_capacity = MAX(old_capacity, write_size) * 2 + 1;
		assert(new_capacity > (old_capacity + write_size));

		// enlarge buffer
		_start = _alloc.reallocate(_start, old_capacity, new_capacity);
		_end   = _start + new_capacity;
		_pos   = _start + old_size;
	}
}


#endif // CACAO_BUFFER_HPP_


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
