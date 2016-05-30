/* src/vm/suck.hpp - functions to read LE ordered types from a buffer

   Copyright (C) 1996-2005, 2006, 2007, 2008
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


#ifndef SUCK_HPP_
#define SUCK_HPP_ 1

#include "config.h"
#include <list>

#include "toolbox/endianess.hpp"
#include "vm/exceptions.hpp"
#include "vm/loader.hpp"
#include "vm/types.hpp"

struct classinfo;
struct hashtable;
class Mutex;
class ZipFile;

/* list_classpath_entry *******************************************************/

enum {
	CLASSPATH_PATH,
	CLASSPATH_ARCHIVE
};

struct list_classpath_entry {
	Mutex             *mutex;	        /* mutex locking on zip/jar files */
	s4                 type;
	char              *path;
	s4                 pathlen;
#if defined(ENABLE_ZLIB)
	ZipFile           *zip;
#endif
};

/**
 * Classpath entries list.
 */
class SuckClasspath : protected std::list<list_classpath_entry*> {
public:
	void add(char *classpath);
	void add_from_property(const char *key);

	// make iterator of std::list visible
	using std::list<list_classpath_entry*>::iterator;

	// make functions of std::list visible
	using std::list<list_classpath_entry*>::begin;
	using std::list<list_classpath_entry*>::end;

	using std::list<list_classpath_entry*>::size;
};

/* classbuffer ****************************************************************/

namespace cacao {
	struct ClassBuffer {
		/// Locate and load class file for class
		ClassBuffer(Utf8String classname);
		ClassBuffer(classinfo *clazz);

		/// Initialize with an already loaded class file
		ClassBuffer(classinfo *clazz, uint8_t *data, size_t sz, const char *path = NULL);

		/// Check if an error occured while creating this classbuffer
		operator bool() { return data != NULL; }

		/// Assert that at least <sz> bytes are left to read
		bool check_size(size_t sz);

		uint8_t  read_u1();
		uint16_t read_u2();
		uint32_t read_u4();
		uint64_t read_u8();

		int32_t read_s4();
		int64_t read_s8();

		float  read_float();
		double read_double();

		/// Transfer block of classfile into a buffer
		void read_nbytes(uint8_t *dst, size_t num_bytes);

		/// Skip block of classfile data
		void skip_nbytes(size_t num_bytes);

		/// The number of unread bytes in the buffer
		size_t remaining();

		/// Free memory held by this classbuffer
		void free();

		classinfo     *get_class() const { return clazz; }
		const uint8_t *get_data()  const { return pos;   }
		const char    *get_path()  const { return path;  }

		ClassFileVersion version() const;
	private:
		ClassBuffer(const ClassBuffer&);
		ClassBuffer& operator=(const ClassBuffer&);

		void init(classinfo*, uint8_t*, size_t, const char*);

		classinfo  *clazz;      // pointer to classinfo structure
		uint8_t    *data;       // pointer to start of buffer
		uint8_t    *pos;        // pointer to current position in buffer
		uint8_t    *end;        // pointer to end of buffer
		const char *path;       // path to file (for debugging)
	};

	inline bool ClassBuffer::check_size(size_t sz) {
#ifdef ENABLE_VERIFIER
		if (remaining() < sz) {
			exceptions_throw_classformaterror(clazz, "Truncated class file");
			return false;
		}
#endif
		return true;
	}

	inline uint8_t ClassBuffer::read_u1() {
		uint8_t u = read_u1_be(pos);
		skip_nbytes(sizeof(uint8_t));
		return u;
	}
	inline uint16_t ClassBuffer::read_u2() {
		uint16_t u = read_u2_be(pos);
		skip_nbytes(sizeof(uint16_t));
		return u;
	}
	inline uint32_t ClassBuffer::read_u4() {
		uint32_t u = read_u4_be(pos);
		skip_nbytes(sizeof(uint32_t));
		return u;
	}
	inline uint64_t ClassBuffer::read_u8() {
		uint64_t u = read_u8_be(pos);
		skip_nbytes(sizeof(uint64_t));
		return u;
	}
	inline int32_t ClassBuffer::read_s4() {
		int32_t u = read_s4_be(pos);
		skip_nbytes(sizeof(int32_t));
		return u;
	}
	inline int64_t ClassBuffer::read_s8() {
		int64_t u = read_s8_be(pos);
		skip_nbytes(sizeof(int64_t));
		return u;
	}
	inline float ClassBuffer::read_float() {
		float u = read_float_be(pos);
		skip_nbytes(sizeof(float));
		return u;
	}
	inline double ClassBuffer::read_double() {
		double u = read_double_be(pos);
		skip_nbytes(sizeof(double));
		return u;
	}

	inline void ClassBuffer::read_nbytes(u1 *dst, size_t num_bytes) {
		MCOPY(dst, pos, u1, num_bytes);
		skip_nbytes(num_bytes);
	}
	inline void ClassBuffer::skip_nbytes(size_t num_bytes) {
		pos += num_bytes;
	}

	inline size_t ClassBuffer::remaining() {
		return end - pos;
	}
}

#endif // SUCK_HPP_


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
