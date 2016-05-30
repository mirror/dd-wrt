/* src/native/vm/cldc1.1/com_sun_cldc_io_ResourceInputStream.cpp

   Copyright (C) 2007-2013
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

#include <errno.h>
#include <zlib.h>

#include "mm/memory.hpp"

#include "native/jni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/include/com_sun_cldc_io_ResourceInputStream.h"
#endif

#include "threads/mutex.hpp"

#include "toolbox/endianess.hpp"

#include "vm/array.hpp"
#include "vm/exceptions.hpp"
#include "vm/javaobjects.hpp"
#include "vm/os.hpp"
#include "vm/string.hpp"
#include "vm/types.hpp"
#include "vm/vm.hpp" /* REMOVE ME: temporarily */
#include "vm/zip.hpp"


static java_handle_t* zip_read_resource(list_classpath_entry *lce, Utf8String name) {
	// try to find the class in the current archive

	const ZipFileEntry *zip = lce->zip->find(c->name);

	if (zip == NULL)
		return NULL;

	// load data from zip file

	size_t   size = zip->uncompressedsize;
	uint8_t *data = MNEW(u1, size);

	zip->get(data);

 	// Create a file descriptor object.
	classinfo     *ci = load_class_bootstrap(Utf8String::from_utf8("com/sun/cldchi/jvm/FileDescriptor"));
	java_handle_t *h  = native_new_and_init(ci);

	if (h == NULL)
		return NULL;

	com_sun_cldchi_jvm_FileDescriptor fd(h, (int64_t) data, 0, size);

	return fd.get_handle();
}


static java_handle_t* file_read_resource(char *path) 
{
	int len;
	struct stat statBuffer;
	u1 *filep;
	classinfo *ci;
	int fd;
	
	fd = open(path, O_RDONLY);
	
	if (fd > 0) {
		
		if (fstat(fd, &statBuffer) != -1) {
			len = statBuffer.st_size;
		} else {  
			return NULL;
		}
		
		/* Map file into the memory */
		filep = (u1*) mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);
		
		/* Create a file descriptor object */
		ci = load_class_bootstrap(Utf8String::from_utf8("com/sun/cldchi/jvm/FileDescriptor"));
		java_handle_t* h = native_new_and_init(ci);

		if (h == NULL)
			return NULL;

		com_sun_cldchi_jvm_FileDescriptor fd(h, (int64_t) filep, 0, len); 

		return fd.get_handle();	
	}
	else {
		return NULL;
	}
}


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     com/sun/cldc/io/ResourceInputStream
 * Method:    open
 * Signature: (Ljava/lang/String;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_com_sun_cldc_io_ResourceInputStream_open(JNIEnv *env, jclass clazz, jstring name)
{
	char *filename;
	s4 filenamelen;
	char *path;
	Utf8String uname;
	java_handle_t* descriptor;

	// Get current list of classpath entries.
	SuckClasspath& suckclasspath = VM::get_current()->get_suckclasspath();

	/* get the classname as char string (do it here for the warning at
	   the end of the function) */

	Buffer<> filename, path;

	filename.write(name);

	/* walk through all classpath entries */

	for (SuckClasspath::iterator it = suckclasspath.begin(); it != suckclasspath.end(); it++) {
		list_classpath_entry* lce = *it;

#if defined(ENABLE_ZLIB)
		if (lce->type == CLASSPATH_ARCHIVE) {
			/* enter a monitor on zip/jar archives */
			MutexLocker(lce->mutex);

			/* try to get the file in current archive */
			descriptor = zip_read_resource(lce, uname);

			if (descriptor != NULL) { /* file exists */
				break;
			}

		} else {
#endif
			path.write(lce->path).write(filename);

			descriptor = file_read_resource(path);

			path.clear();

			if (descriptor != NULL) { /* file exists */
				break;
			}
#if defined(ENABLE_ZLIB)
		}
#endif
	}

	return (jobject) descriptor;
}


/*
 * Class:     com/sun/cldc/io/ResourceInputStream
 * Method:    bytesRemain
 * Signature: (Ljava/lang/Object;)I
 */
JNIEXPORT jint JNICALL Java_com_sun_cldc_io_ResourceInputStream_bytesRemain(JNIEnv *env, jclass clazz, jobject jobj)
{
	com_sun_cldchi_jvm_FileDescriptor fd(jobj);
	int32_t length   = fd.get_position();
	int32_t position = fd.get_length();

	return length - position;
}


/*
 * Class:     com/sun/cldc/io/ResourceInputStream
 * Method:    readByte
 * Signature: (Ljava/lang/Object;)I
 */
JNIEXPORT jint JNICALL Java_com_sun_cldc_io_ResourceInputStream_readByte(JNIEnv *env, jclass clazz, jobject jobj)
{
	com_sun_cldchi_jvm_FileDescriptor fd(jobj);

	int64_t filep    = fd.get_pointer();
	int32_t position = fd.get_position();
	int32_t length   = fd.get_length();

	uint8_t byte;

	if (position < length) {
		byte = ((uint8_t*) filep)[position];
		position++;
	}
	else {
		return -1; /* EOF */
	}

	// Update access position.
	fd.set_position(position);
	
	return (byte & 0xFF);
}


/*
 * Class:     com/sun/cldc/io/ResourceInputStream
 * Method:    readBytes
 * Signature: (Ljava/lang/Object;[BII)I
 */
JNIEXPORT jint JNICALL Java_com_sun_cldc_io_ResourceInputStream_readBytes(JNIEnv *env, jclass clazz, jobject jobj, jbyteArray byteArray, jint off, jint len)
{
	/* get pointer to the buffer */
	// XXX Not GC safe.
	ByteArray ba(byteArray);
	void* buf = (void*) (((int8_t*) ba.get_raw_data_ptr()) + off);

	com_sun_cldchi_jvm_FileDescriptor fd(jobj);

	int64_t filep      = fd.get_pointer();
	int32_t position   = fd.get_position();
	int32_t fileLength = fd.get_length();

	int32_t readBytes = -1;

	if (position < fileLength) {
		int32_t available = fileLength - position;

		if (available < len) {
			readBytes = available;
		} else {
			readBytes = len;
		}

		os::memcpy(buf, ((uint8_t*) filep) + position, readBytes * sizeof(uint8_t));
		position += readBytes;
	}
	else {
		return -1; /* EOF */
	}

	// Update access position.
	fd.set_position(position);
	
	return readBytes;
}


/*
 * Class:     com/sun/cldc/io/ResourceInputStream
 * Method:    clone
 * Signature: (Ljava/lang/Object;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_com_sun_cldc_io_ResourceInputStream_clone(JNIEnv *env, jclass clazz, jobject jobj)
{
	com_sun_cldchi_jvm_FileDescriptor fd(jobj);

	classinfo* c = load_class_bootstrap(Utf8String::from_utf8("com/sun/cldchi/jvm/FileDescriptor"));
	java_handle_t* h = native_new_and_init(c);

	if (h == NULL)
		return NULL;

	com_sun_cldchi_jvm_FileDescriptor clonefd(h, fd);
	
	return (jobject) clonefd.get_handle();
}

} // extern "C"


/* native methods implemented by this file ************************************/
 
static JNINativeMethod methods[] = {
	{ (char*) "open",        (char*) "(Ljava/lang/String;)Ljava/lang/Object;", (void*) (uintptr_t) &Java_com_sun_cldc_io_ResourceInputStream_open        },
	{ (char*) "bytesRemain", (char*) "(Ljava/lang/Object;)I",                  (void*) (uintptr_t) &Java_com_sun_cldc_io_ResourceInputStream_bytesRemain },
	{ (char*) "readByte",    (char*) "(Ljava/lang/Object;)I",                  (void*) (uintptr_t) &Java_com_sun_cldc_io_ResourceInputStream_readByte    },
	{ (char*) "readBytes",   (char*) "(Ljava/lang/Object;[BII)I",              (void*) (uintptr_t) &Java_com_sun_cldc_io_ResourceInputStream_readBytes   },
	{ (char*) "clone",       (char*) "(Ljava/lang/Object;)Ljava/lang/Object;", (void*) (uintptr_t) &Java_com_sun_cldc_io_ResourceInputStream_clone       },
};
 

/* _Jv_com_sun_cldc_io_ResourceInputStream_init ********************************
 
   Register native functions.
 
*******************************************************************************/
 
void _Jv_com_sun_cldc_io_ResourceInputStream_init(void)
{
	Utf8String u = Utf8String::from_utf8("com/sun/cldc/io/ResourceInputStream");

	NativeMethods& nm = VM::get_current()->get_nativemethods();
	nm.register_methods(u, methods, NATIVE_METHODS_COUNT);
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
