/* src/vm/suck.cpp - functions to read LE ordered types from a buffer

   Copyright (C) 1996-2012
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


#include "config.h"

#include <cassert>
#include <cstdlib>

#include "vm/types.hpp"

#include "mm/memory.hpp"

#include "threads/mutex.hpp"

#include "toolbox/buffer.hpp"
#include "toolbox/endianess.hpp"
#include "toolbox/hashtable.hpp"
#include "toolbox/list.hpp"
#include "toolbox/logging.hpp"

#include "vm/exceptions.hpp"
#include "vm/loader.hpp"
#include "vm/options.hpp"
#include "vm/os.hpp"
#include "vm/properties.hpp"
#include "vm/suck.hpp"
#include "vm/vm.hpp"
#include "vm/zip.hpp"

using namespace cacao;


/* scandir_filter **************************************************************

   Filters for zip/jar files.

*******************************************************************************/

static int scandir_filter(const struct dirent *a)
{
	s4 namlen;

#if defined(_DIRENT_HAVE_D_NAMLEN)
	namlen = a->d_namlen;
#else
	namlen = strlen(a->d_name);
#endif

	if ((strncasecmp(a->d_name + namlen - 4, ".zip", 4) == 0) ||
		(strncasecmp(a->d_name + namlen - 4, ".jar", 4) == 0))
		return 1;

	return 0;
}


/**
 * Adds a classpath to the global classpath entries list.
 */
void SuckClasspath::add(char *classpath)
{
	char                 *start;
	char                 *end;
	char                 *cwd;
	s4                    cwdlen;

	/* parse the classpath string */

	for (start = classpath; (*start) != '\0'; ) {

		/* search for ':' delimiter to get the end of the current entry */
		for (end = start; ((*end) != '\0') && ((*end) != ':'); end++);

		if (start != end) {
			bool   is_zip      = false;
			size_t filenamelen = end - start;

			if (filenamelen > 4) {
				if ((strncasecmp(end - 4, ".zip", 4) == 0) ||
					(strncasecmp(end - 4, ".jar", 4) == 0)) {
					is_zip = true;
				}
			}

			/* save classpath entries as absolute pathnames */

			cwd = NULL;
			cwdlen = 0;

			if (*start != '/') {                      /* XXX fix me for win32 */
				cwd = os::getcwd();
				cwdlen = strlen(cwd) + strlen("/");
			}

			/* allocate memory for filename and fill it */

			char *filename = MNEW(char, filenamelen + cwdlen + strlen("/") + strlen("0"));

			if (cwd) {
				strcpy(filename, cwd);
				strcat(filename, "/");
				strncat(filename, start, filenamelen);

				/* add cwd length to file length */
				filenamelen += cwdlen;

			} else {
				strncpy(filename, start, filenamelen);
				filename[filenamelen] = '\0';
			}

			if (is_zip) {
#if defined(ENABLE_ZLIB)
				if (ZipFile *zip = ZipFile::open(filename)) {
					list_classpath_entry *lce = NEW(list_classpath_entry);

					lce->type    = CLASSPATH_ARCHIVE;
					lce->zip     = zip;
					lce->path    = filename;
					lce->pathlen = filenamelen;

					/* SUN compatible -verbose:class output */

					if (opt_verboseclass)
						printf("[Opened %s]\n", filename);

					push_back(lce);
				}
#else
				os::abort("suck_add: zip/jar files not supported");
#endif
			}
			else {
				if (filename[filenamelen - 1] != '/') {/* XXX fixme for win32 */
					filename[filenamelen] = '/';
					filename[filenamelen + 1] = '\0';
					filenamelen++;
				}

				list_classpath_entry *lce = NEW(list_classpath_entry);

				lce->type    = CLASSPATH_PATH;
				lce->path    = filename;
				lce->pathlen = filenamelen;

				push_back(lce);
			}
		}

		/* goto next classpath entry, skip ':' delimiter */

		if ((*end) == ':')
			start = end + 1;
		else
			start = end;
	}
}


/**
 * Adds a classpath form a property entry to the global classpath
 * entries list.
 */
void SuckClasspath::add_from_property(const char *key)
{
	const char     *value;
	const char     *start;
	const char     *end;
	s4              pathlen;
	struct dirent **namelist;
	s4              n;
	s4              i;
	s4              namlen;
	char           *p;
	char* boot_class_path = NULL;

	// Get the property value.
	Properties& properties = VM::get_current()->get_properties();
	value = properties.get(key);

	if (value == NULL)
		return;

	/* get the directory entries of the property */

	for (start = value; (*start) != '\0'; ) {

		/* search for ':' delimiter to get the end of the current entry */

		for (end = start; ((*end) != '\0') && ((*end) != ':'); end++);

		/* found an entry */

		if (start != end) {
			/* allocate memory for the path entry */

			pathlen = end - start;
			char* path = MNEW(char, pathlen + strlen("0"));

			/* copy and terminate the string */

			strncpy(path, start, pathlen);
			path[pathlen] = '\0';

			/* Reset namelist to NULL for the freeing in an error case
			   (see below). */

			namelist = NULL;

			/* scan the directory found for zip/jar files */

			n = os::scandir((const char*) path, &namelist, &scandir_filter, (int (*)(const void*, const void*)) &alphasort);

			/* On error, just continue, this should be ok. */

			if (n > 0) {
				for (i = 0; i < n; i++) {
#if defined(_DIRENT_HAVE_D_NAMLEN)
					namlen = namelist[i]->d_namlen;
#else
					namlen = strlen(namelist[i]->d_name);
#endif

					if (boot_class_path == NULL) {
						/* Allocate memory for bootclasspath. */
						p = MNEW(char,
								 pathlen + strlen("/") + namlen +
								 strlen("0"));

						strcpy(p, path);
						strcat(p, "/");
						strcat(p, namelist[i]->d_name);

					} else {
						/* Allocate memory for bootclasspath. */
						p = MNEW(char,
								 pathlen + strlen("/") + namlen +
								 strlen(":") +
								 strlen(boot_class_path) +
								 strlen("0"));

						/* Append the file found to the bootclasspath. */

						strcpy(p, boot_class_path);
						strcat(p, ":");
						strcat(p, path);
						strcat(p, "/");
						strcat(p, namelist[i]->d_name);

						MFREE(boot_class_path, char, strlen(boot_class_path));
					}

					boot_class_path = p;

					/* free the memory allocated by scandir */
					/* (We use `free` as the memory came from the C library.) */

					free(namelist[i]);
				}
			}

			/* On some systems (like Linux) when n == 0, then namelist
			   returned from scnadir is NULL, thus we don't have to
			   free it.
			   (Use `free` as the memory came from the C library.) */

			if (namelist != NULL)
				free(namelist);

			MFREE(path, char, pathlen + strlen("0"));
		}

		/* goto next entry, skip ':' delimiter */

		if ((*end) == ':')
			start = end + 1;
		else
			start = end;
	}

	if (boot_class_path != NULL) {
		// only update if something has changed

		// FIXME Make boot_class_path const char*.
		char* old_boot_class_path = (char*) properties.get("sun.boot.class.path");

		p = MNEW(char,
				 strlen(boot_class_path) +
				 strlen(":") +
				 strlen(old_boot_class_path) +
				 strlen("0"));

		/* Prepend the file found to the bootclasspath. */

		strcpy(p, boot_class_path);
		strcat(p, ":");
		strcat(p, old_boot_class_path);

		MFREE(boot_class_path, char, strlen(boot_class_path));
		MFREE(old_boot_class_path, char, strlen(old_boot_class_path));

		/* Prepend the file found to the bootclasspath. */
		properties.put("sun.boot.class.path", p);
		properties.put("java.boot.class.path", p);
	}

}


inline void ClassBuffer::init(classinfo *clazz, uint8_t *data, size_t sz, const char *path) {
	this->clazz = clazz;
	this->data  = data;
	this->pos   = data;
	this->end   = data + sz;
	this->path  = path;
}

ClassBuffer::ClassBuffer(classinfo *clazz, uint8_t *data, size_t sz, const char *path) {
	init(clazz, data, sz, path);
}

ClassFileVersion ClassBuffer::version() const { return clazz->version; }

/***
 *	Loads class file corresponding to given classinfo into new ClassBuffer.
 *	All directories of the searchpath are used to find the classfile (<classname>.class).
 *    Use operator bool to check if initialization was successfull.
 */
ClassBuffer::ClassBuffer(classinfo *c) {
	init(NULL, NULL, 0, NULL);

	// get the classname as char string 
	// (do it here for the warning at the end of the function)

	size_t filenamelen = c->name.size() + strlen(".class") + strlen("0");

	Buffer<> filename(filenamelen);
	Buffer<> path;

	filename.write(c->name)
	        .write(".class");

	// Get current list of classpath entries.
	SuckClasspath& suckclasspath = VM::get_current()->get_suckclasspath();

	// walk through all classpath entries

	for (SuckClasspath::iterator it = suckclasspath.begin(); it != suckclasspath.end(); it++) {
		list_classpath_entry *lce = *it;

#if defined(ENABLE_ZLIB)
		if (lce->type == CLASSPATH_ARCHIVE) {

			// enter a monitor on zip/jar archives
			MutexLocker lock(*lce->mutex);

			// try to get the file in current archive
			if (ZipFile::EntryRef zip = lce->zip->find(c->name)) {
				// found class, fill in classbuffer
				size_t   size = zip->uncompressedsize;
				uint8_t *data = MNEW(uint8_t, size);

				zip->get(data);

				init(c, data, size, lce->path);
				return;
			}
		} else {
#endif /* defined(ENABLE_ZLIB) */
			path.reset();

			path.write(lce->path)
			    .write(filename);

			if (FILE *classfile = os::fopen(path.c_str(), "r")) {
				struct stat stat_buffer;

				if (os::stat(path.c_str(), &stat_buffer) == -1)
					continue;

				size_t   size = stat_buffer.st_size;
				uint8_t *data = MNEW(u1, size);

				// read class data
				size_t bytes_read = os::fread(data, 1, size, classfile);
				os::fclose(classfile);

				if (bytes_read != size) {
					free();
					return;
				}

				init(c, data, size, lce->path);
				return;
			}
#if defined(ENABLE_ZLIB)
		}
#endif
	}

	// if we get here, we could not find the file
	if (opt_verbose)
		dolog("Warning: Can not open class file '%s'", filename.c_str());
}


/* suck_stop *******************************************************************

   Frees memory for buffer with classfile data.

   CAUTION: This function may only be called if buffer has been
   allocated by suck_start with reading a file.
	
*******************************************************************************/

void ClassBuffer::free() {
	// free memory

	MFREE(data, u1, end - data);
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
