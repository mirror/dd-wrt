/* src/vm/zip.hpp - ZIP file handling for bootstrap classloader

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


#ifndef ZIP_HPP_
#define ZIP_HPP_ 1

#include "config.h"                     // for ENABLE_ZLIB

#ifdef ENABLE_ZLIB

#include <cstddef>                      // for size_t
#include <stdint.h>                     // for uint8_t

#include "toolbox/hashtable.hpp"        // for InsertOnlyStringEntry, etc

#include "vm/types.hpp"                 // for u2, u4, u1
#include "vm/utf8.hpp"                  // for Utf8String


/* Local file header ***********************************************************

   local file header signature     4 bytes  (0x04034b50)
   version needed to extract       2 bytes
   general purpose bit flag        2 bytes
   compression method              2 bytes
   last mod file time              2 bytes
   last mod file date              2 bytes
   crc-32                          4 bytes
   compressed size                 4 bytes
   uncompressed size               4 bytes
   file name length                2 bytes
   extra field length              2 bytes

   file name (variable size)
   extra field (variable size)

*******************************************************************************/

#define LFH_HEADER_SIZE              30

#define LFH_SIGNATURE                0x04034b50
#define LFH_FILE_NAME_LENGTH         26
#define LFH_EXTRA_FIELD_LENGTH       28

struct lfh {
	u2 compressionmethod;
	u4 compressedsize;
	u4 uncompressedsize;
	u2 filenamelength;
	u2 extrafieldlength;
};

/* hashtable_zipfile_entry ****************************************************/

struct ZipFileEntry {
	Utf8String filename;
	u2         compressionmethod;
	u4         compressedsize;
	u4         uncompressedsize;
	u1        *data;

	/***
	 * Load data from zipped file into memory
	 * 
	 * `dst' must have room for `uncompressedsize' bytes.
	 */
	void get(uint8_t *dst) const;

	/// interface to HashTable
	size_t hash() const { return filename.hash(); }

	Utf8String key() const { return filename; }
	void set_key(Utf8String u) { filename = u; }
};

class ZipFile {
	typedef HashTable<InsertOnlyNamedEntry<ZipFileEntry> > Table;
public:
	typedef Table::Iterator Iterator;
	typedef Table::EntryRef EntryRef;

	/// Load zip archive
	static ZipFile *open(const char *path);

	/// Find file in zip archive
	EntryRef find(Utf8String filename) { return table.find(filename); }

	/// Allows iteration over all entries in zip archive
	Iterator begin() { return table.begin(); }
	Iterator end()   { return table.end();   }
private:
	ZipFile(size_t capacity) : table(capacity) {}

	Table table;
};

#endif // ENABLE_ZLIB

#endif // ZIP_HPP_


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
