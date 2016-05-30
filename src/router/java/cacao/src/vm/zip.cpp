/* src/vm/zip.cpp - ZIP file handling for bootstrap classloader

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


#include "vm/zip.hpp"
#include "config.h"

#include <cassert>
#include <cerrno>
#include <unistd.h>
#include <zlib.h>

#include "mm/memory.hpp"

#include "toolbox/endianess.hpp"
#include "toolbox/hashtable.hpp"

#include "mm/memory.hpp"

#include "vm/os.hpp"
#include "vm/suck.hpp"
#include "vm/types.hpp"
#include "vm/utf8.hpp"
#include "vm/vm.hpp"
#include "vm/zip.hpp"

using namespace cacao;


/* start size for classes hashtable *******************************************/

#define HASHTABLE_CLASSES_SIZE    (1 << 10)


/* info taken from:
   http://www.pkware.com/business_and_developers/developer/popups/appnote.txt
*/

/* all signatures in the ZIP file have a length of 4 bytes ********************/

#define SIGNATURE_LENGTH    4

/* Central directory structure *************************************************

   [file header 1]
   .
   .
   . 
   [file header n]
   [digital signature] 
   
   File header:
   
     central file header signature   4 bytes  (0x02014b50)
     version made by                 2 bytes
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
     file comment length             2 bytes
     disk number start               2 bytes
     internal file attributes        2 bytes
     external file attributes        4 bytes
     relative offset of local header 4 bytes
   
     file name (variable size)
     extra field (variable size)
     file comment (variable size)

   Digital signature:
   
     header signature                4 bytes  (0x05054b50)
     size of data                    2 bytes
     signature data (variable size)

*******************************************************************************/

#define CDSFH_HEADER_SIZE            46

#define CDSFH_SIGNATURE              0x02014b50
#define CDSFH_COMPRESSION_METHOD     10
#define CDSFH_COMPRESSED_SIZE        20
#define CDSFH_UNCOMPRESSED_SIZE      24
#define CDSFH_FILE_NAME_LENGTH       28
#define CDSFH_EXTRA_FIELD_LENGTH     30
#define CDSFH_FILE_COMMENT_LENGTH    32
#define CDSFH_RELATIVE_OFFSET        42
#define CDSFH_FILENAME               46

struct cdsfh {
	u2 compressionmethod;
	u4 compressedsize;
	u4 uncompressedsize;
	u2 filenamelength;
	u2 extrafieldlength;
	u2 filecommentlength;
	u4 relativeoffset;
};


/* End of central directory record *********************************************

   end of central dir signature    4 bytes  (0x06054b50)
   number of this disk             2 bytes
   number of the disk with the
   start of the central directory  2 bytes
   total number of entries in the
   central directory on this disk  2 bytes
   total number of entries in
   the central directory           2 bytes
   size of the central directory   4 bytes
   offset of start of central
   directory with respect to
   the starting disk number        4 bytes
   .ZIP file comment length        2 bytes
   .ZIP file comment       (variable size)

*******************************************************************************/

#define EOCDR_SIGNATURE              0x06054b50
#define EOCDR_ENTRIES                10
#define EOCDR_OFFSET                 16

struct eocdr {
	u2 entries;
	u4 offset;
};


/***
 * Load zip file into memory
 */
ZipFile *ZipFile::open(const char *path) {
	int     fd;
	u1      lfh_signature[SIGNATURE_LENGTH];
	off_t   len;
	u1     *p;
	eocdr   eocdr;
	cdsfh   cdsfh;

	// first of all, open the file

	if ((fd = ::open(path, O_RDONLY)) == -1)
		return NULL;

	// check for signature in first local file header

	if (read(fd, lfh_signature, SIGNATURE_LENGTH) != SIGNATURE_LENGTH)
		return NULL;

	if (read_u4_le(lfh_signature) != LFH_SIGNATURE)
		return NULL;

	// get the file length

	if ((len = lseek(fd, 0, SEEK_END)) == -1)
		return NULL;

	// we better mmap the file

	u1 *filep = (u1*) mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);

	// some older compilers, like DEC OSF cc, don't like comparisons
	// on void* type

	if ((ptrint) filep == (ptrint) MAP_FAILED)
		return NULL;

	// find end of central directory record

	for (p = filep + len; p >= filep; p--)
		if (read_u4_le(p) == EOCDR_SIGNATURE)
			break;

	// get number of entries in central directory

	eocdr.entries = read_u2_le(p + EOCDR_ENTRIES);
	eocdr.offset  = read_u4_le(p + EOCDR_OFFSET);

	// create hashtable for filenames

	ZipFile *file = new ZipFile(HASHTABLE_CLASSES_SIZE);

	// add all file entries into the hashtable

	p = filep + eocdr.offset;

	for (s4 i = 0; i < eocdr.entries; i++) {
		// check file header signature

		if (read_u4_le(p) != CDSFH_SIGNATURE)
			return NULL;

		// we found an entry

		cdsfh.compressionmethod = read_u2_le(p + CDSFH_COMPRESSION_METHOD);
		cdsfh.compressedsize    = read_u4_le(p + CDSFH_COMPRESSED_SIZE);
		cdsfh.uncompressedsize  = read_u4_le(p + CDSFH_UNCOMPRESSED_SIZE);
		cdsfh.filenamelength    = read_u2_le(p + CDSFH_FILE_NAME_LENGTH);
		cdsfh.extrafieldlength  = read_u2_le(p + CDSFH_EXTRA_FIELD_LENGTH);
		cdsfh.filecommentlength = read_u2_le(p + CDSFH_FILE_COMMENT_LENGTH);
		cdsfh.relativeoffset    = read_u4_le(p + CDSFH_RELATIVE_OFFSET);

		// create utf8 string of filename, strip .class from classes

		const char *filename = (const char *) (p + CDSFH_FILENAME);
		const char *classext = filename + cdsfh.filenamelength - strlen(".class");

		// skip directory entries

		if (filename[cdsfh.filenamelength - 1] != '/') {
			Utf8String u;

			if (strncmp(classext, ".class", strlen(".class")) == 0)
				u = Utf8String::from_utf8(filename, cdsfh.filenamelength - strlen(".class"));
			else
				u = Utf8String::from_utf8(filename, cdsfh.filenamelength);

			// create zip entry

			ZipFileEntry entry;

			entry.filename          = u;
			entry.compressionmethod = cdsfh.compressionmethod;
			entry.compressedsize    = cdsfh.compressedsize;
			entry.uncompressedsize  = cdsfh.uncompressedsize;
			entry.data              = filep + cdsfh.relativeoffset;

			// insert into hashtable

			file->table.insert(entry);
		}

		// move to next central directory structure file header

		p = p
		  + CDSFH_HEADER_SIZE
		  + cdsfh.filenamelength
		  + cdsfh.extrafieldlength
		  + cdsfh.filecommentlength;
	}

	// return pointer to hashtable

	return file;
}


/***
 * Load file from zip archive into memory
 */
void ZipFileEntry::get(uint8_t *dst) const {
	lfh      lfh;
	z_stream zs;
	int      err;

	// read stuff from local file header

	lfh.filenamelength   = read_u2_le(data + LFH_FILE_NAME_LENGTH);
	lfh.extrafieldlength = read_u2_le(data + LFH_EXTRA_FIELD_LENGTH);

	u1 *indata = data
	           + LFH_HEADER_SIZE
	           + lfh.filenamelength
	           + lfh.extrafieldlength;

	// how is the file stored?

	switch (compressionmethod) {
	case Z_DEFLATED:
		// fill z_stream structure

		zs.next_in   = indata;
		zs.avail_in  = compressedsize;
		zs.next_out  = dst;
		zs.avail_out = uncompressedsize;

		zs.zalloc = Z_NULL;
		zs.zfree  = Z_NULL;
		zs.opaque = Z_NULL;

		// initialize this inflate run

		if (inflateInit2(&zs, -MAX_WBITS) != Z_OK)
			vm_abort("zip_get: inflateInit2 failed: %s", strerror(errno));

		// decompress the file into buffer

		err = inflate(&zs, Z_SYNC_FLUSH);

		if ((err != Z_STREAM_END) && (err != Z_OK))
			vm_abort("zip_get: inflate failed: %s", strerror(errno));

		// finish this inflate run

		if (inflateEnd(&zs) != Z_OK)
			vm_abort("zip_get: inflateEnd failed: %s", strerror(errno));
		break;

	case 0:
		// uncompressed file, just copy the data
		MCOPY(dst, indata, u1, compressedsize);
		break;

	default:
		vm_abort("zip_get: unknown compression method %d", compressionmethod);
		break;
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
