/* src/native/vm/openjdk/hpi.cpp - HotSpot HPI interface functions

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

#include "config.h"

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
// Include this one early.
#include "native/vm/openjdk/hpi.hpp"

#include "native/native.hpp"

#include "toolbox/buffer.hpp"
#include "toolbox/logging.hpp"

#include "vm/options.hpp"
#include "vm/os.hpp"
#include "vm/properties.hpp"
#include "vm/utf8.hpp"
#include "vm/vm.hpp"


/* VM callback functions ******************************************************/

static vm_calls_t callbacks = {
	/* TODO What should we use here? */
/*   jio_fprintf, */
/*   unimplemented_panic, */
/*   unimplemented_monitorRegister, */
	NULL,
	NULL,
	NULL,
	
	NULL, /* unused */
	NULL, /* unused */
	NULL  /* unused */
};


/**
 * Initialize the Host Porting Interface (HPI).
 */
HPI::HPI()
{
}

void HPI::initialize() // REMOVEME
{
	TRACESUBSYSTEMINITIALIZATION("hpi_init");

	// Load libhpi.so
	VM* vm = VM::get_current();
	Properties& properties = vm->get_properties();
	const char* boot_library_path = properties.get("sun.boot.library.path");

	// Use Buffer to assemble library path.
	Buffer<> buf;

	buf.write(boot_library_path);
	buf.write("/native_threads/libhpi.so");

	Utf8String u = buf.utf8_str();

    if (opt_TraceHPI)
		log_println("HPI::initialize: Loading HPI %s ", buf.c_str());

	NativeLibrary nl(u);
	void* handle = nl.open();

	if (handle == NULL)
		if (opt_TraceHPI)
			os::abort("HPI::initialize: HPI open failed");

	// Resolve the DLL_Initialize function from the library.
	void* dll_initialize = os::dlsym(handle, "DLL_Initialize");

    jint (JNICALL *DLL_Initialize)(GetInterfaceFunc*, void*);
    DLL_Initialize = (jint (JNICALL *)(GetInterfaceFunc*, void*)) (uintptr_t) dll_initialize;

    if (opt_TraceHPI && DLL_Initialize == NULL)
		log_println("hpi_init: HPI dlsym of DLL_Initialize failed: %s", os::dlerror());

    if (DLL_Initialize == NULL || (*DLL_Initialize)(&_get_interface, &callbacks) < 0) {
        if (opt_TraceHPI)
			vm_abort("hpi_init: HPI DLL_Initialize failed");
    }

	NativeLibraries& nls = vm->get_nativelibraries();
	nls.add(nl);

    if (opt_TraceHPI)
		log_println("HPI::initialize: HPI loaded successfully");

	// Resolve the interfaces.
	/* NOTE: The intptr_t-case is only to prevent the a compiler
	   warning with -O2: warning: dereferencing type-punned pointer
	   will break strict-aliasing rules */

	int result;

	result = (*_get_interface)((void**) (uintptr_t) &_file, "File", 1);

	if (result != 0)
		os::abort("hpi_init: Can't find HPI_FileInterface");

	result = (*_get_interface)((void**) (uintptr_t) &_library, "Library", 1);

	if (result != 0)
		os::abort("hpi_init: Can't find HPI_LibraryInterface");

	result = (*_get_interface)((void**) (uintptr_t) &_system, "System", 1);

	if (result != 0)
		os::abort("hpi_init: Can't find HPI_SystemInterface");
}


/**
 * Initialize the Host Porting Interface (HPI) socket library.
 */
int HPI::initialize_socket_library()
{
	// Resolve the socket library interface.
	int result = (*_get_interface)((void**) (uintptr_t) &_socket, "Socket", 1);

	if (result != 0) {
		if (opt_TraceHPI)
			log_println("HPI::initialize_socket_library: Can't find HPI_SocketInterface");

		return JNI_ERR;
	}

	return JNI_OK;
}


// Legacy C interface.
extern "C" {
	void HPI_initialize() { VM::get_current()->get_hpi().initialize(); }
}

#endif /*not defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7)*/
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
