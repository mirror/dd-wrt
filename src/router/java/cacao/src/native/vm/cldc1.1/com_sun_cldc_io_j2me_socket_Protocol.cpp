/* src/native/vm/cldc1.1/com_sun_cldc_io_j2me_socket_Protocol.cpp

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

#include <stdint.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>

#include "vm/types.hpp"

#include "mm/memory.hpp"

#include "native/jni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/include/com_sun_cldc_io_j2me_socket_Protocol.h"
#endif

#include "vm/array.hpp"
#include "vm/global.hpp"
#include "vm/os.hpp"
#include "vm/vm.hpp" /* REMOVE ME: temporarily */


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     com/sun/cldc/io/j2me/socket/Protocol
 * Method:    open0
 * Signature: ([BII)I
 */
JNIEXPORT jint JNICALL Java_com_sun_cldc_io_j2me_socket_Protocol_open0(JNIEnv *env, jclass clazz, jbyteArray hostname, jint port, jint mode)
{
	struct hostent *phostent;
    struct sockaddr_in serv_addr;

	// The hostname byte-array is a NULL terminated C-string.
	// XXX Not GC safe.
	ByteArray ba(hostname);
	char* name = (char*) ba.get_raw_data_ptr();

	/* get the host */

	phostent = gethostbyname(name);

	if (phostent == NULL)
		return -1;

	/* fill the sockaddr structure */

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port   = htons(port);

	MCOPY(&serv_addr.sin_addr, phostent->h_addr, u1, phostent->h_length);

	/* create the socket */

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
		return -1;

	/* connect the socket */

	int result = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));

	if (result < 0)
		return -1;

	return sockfd;
}


/*
 * Class:     com/sun/cldc/io/j2me/socket/Protocol
 * Method:    readBuf
 * Signature: (I[BII)I
 */
JNIEXPORT jint JNICALL Java_com_sun_cldc_io_j2me_socket_Protocol_readBuf(JNIEnv *env, jclass clazz, jint handle, jbyteArray b, jint off, jint len)
{
	// Get pointer to the buffer.
	// XXX Not GC safe.
	ByteArray ba(b);
	void* buf = (void*) (((int8_t*) ba.get_raw_data_ptr()) + off);

	// Receive from the socket.
	ssize_t result = recv(handle, buf, len, 0);

	if (result == 0) {
		// The peer has performed an orderly shutdown.
		return -1;
	}
	else if (result < 0) {
		os::abort_errno("Java_com_sun_cldc_io_j2me_socket_Protocol_readBuf: recv failed");
	}

	return result;
}


/*
 * Class:     com/sun/cldc/io/j2me/socket/Protocol
 * Method:    readByte
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_sun_cldc_io_j2me_socket_Protocol_readByte(JNIEnv *env, jclass clazz, jint handle)
{
	char byte;
	
	// Receive from the socket.
	ssize_t result = recv(handle, &byte, 1, 0);

	if (result == 0) {
		// The peer has performed an orderly shutdown.
		return -1;
	}
	else if (result < 0) {
		// TODO Should throw an IOException.
		os::abort_errno("Java_com_sun_cldc_io_j2me_socket_Protocol_readByte: recv failed");
	}

	return byte;
}


/*
 * Class:     com/sun/cldc/io/j2me/socket/Protocol
 * Method:    writeBuf
 * Signature: (I[BII)I
 */
JNIEXPORT jint JNICALL Java_com_sun_cldc_io_j2me_socket_Protocol_writeBuf(JNIEnv *env, jclass clazz, jint handle, jbyteArray b, jint off, jint len)
{
	// Get pointer to the buffer.
	// XXX Not GC safe.
	ByteArray ba(b);
	void* buf = (void*) (((int8_t*) ba.get_raw_data_ptr()) + off);
	
	// Send the given byte to the socket.
	ssize_t result = send(handle, buf, len, 0);

	if (result < 0) {
		// TODO Should throw an IOException.
		os::abort_errno("Java_com_sun_cldc_io_j2me_socket_Protocol_writeBuf: send failed");
	}

	return result;
}


/*
 * Class:     com/sun/cldc/io/j2me/socket/Protocol
 * Method:    writeByte
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_com_sun_cldc_io_j2me_socket_Protocol_writeByte(JNIEnv *env, jclass clazz, jint handle, jint b)
{
	char byte = (char) b;

	// Send the given byte to the socket.
	ssize_t result = send(handle, &byte, 1, 0);

	if (result < 0)
		os::abort_errno("Java_com_sun_cldc_io_j2me_socket_Protocol_writeByte: send failed");

	return result;
}


/*
 * Class:     com/sun/cldc/io/j2me/socket/Protocol
 * Method:    available0
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_sun_cldc_io_j2me_socket_Protocol_available0(JNIEnv *env, jclass clazz, jint handle)
{
	// NOTE: Sun doesn't have an implementation too.
	return 0;
}


/*
 * Class:     com/sun/cldc/io/j2me/socket/Protocol
 * Method:    close0
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_sun_cldc_io_j2me_socket_Protocol_close0(JNIEnv *env, jclass clazz, jint handle)
{
	// Close the file descriptor.
	int result = close(handle);

	if (result < 0)
		os::abort_errno("Java_com_sun_cldc_io_j2me_socket_Protocol_close0: close failed");
}

} // extern "C"


/* native methods implemented by this file ************************************/
 
static JNINativeMethod methods[] = {
	{ (char*) "open0",      (char*) "([BII)I",  (void*) (uintptr_t) &Java_com_sun_cldc_io_j2me_socket_Protocol_open0      },
	{ (char*) "readBuf",    (char*) "(I[BII)I", (void*) (uintptr_t) &Java_com_sun_cldc_io_j2me_socket_Protocol_readBuf    },
	{ (char*) "readByte",   (char*) "(I)I",     (void*) (uintptr_t) &Java_com_sun_cldc_io_j2me_socket_Protocol_readByte   },
	{ (char*) "writeBuf",   (char*) "(I[BII)I", (void*) (uintptr_t) &Java_com_sun_cldc_io_j2me_socket_Protocol_writeBuf   },
	{ (char*) "writeByte",  (char*) "(II)I",    (void*) (uintptr_t) &Java_com_sun_cldc_io_j2me_socket_Protocol_writeByte  },
	{ (char*) "available0", (char*) "(I)I",     (void*) (uintptr_t) &Java_com_sun_cldc_io_j2me_socket_Protocol_available0 },
	{ (char*) "close0",     (char*) "(I)V",     (void*) (uintptr_t) &Java_com_sun_cldc_io_j2me_socket_Protocol_close0     },
};

 
/* _Jv_com_sun_cldc_io_j2me_socket_Protocol_init *******************************
 
   Register native functions.
 
*******************************************************************************/
 
void _Jv_com_sun_cldc_io_j2me_socket_Protocol_init(void)
{
	Utf8String u = Utf8String::from_utf8("com/sun/cldc/io/j2me/socket/Protocol");

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
