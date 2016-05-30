/* src/cacao/cacao.cpp - contains main() of cacao

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


#include "config.h"

#if defined(ENABLE_JRE_LAYOUT)
# include <errno.h>
# include <libgen.h>
# include <unistd.h>
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "native/jni.hpp"
#include "native/native.hpp"

#include "vm/os.hpp"
#include "vm/vm.hpp"


/* Defines. *******************************************************************/

#define LIBJVM_NAME    NATIVE_LIBRARY_PREFIX"jvm"NATIVE_LIBRARY_SUFFIX


/* forward declarations *******************************************************/

static JavaVMInitArgs* prepare_options(int argc, char** argv);


/* main ************************************************************************

   The main program.
   
*******************************************************************************/

int main(int argc, char **argv)
{
#if defined(ENABLE_LIBJVM)
	char* path;

# if defined(ENABLE_JRE_LAYOUT)
	int         len;
# endif
#endif

#if defined(ENABLE_LIBJVM)	
	/* Variables for JNI_CreateJavaVM dlopen call. */
	void*       libjvm_handle;
	void*       libjvm_VM_create;
	void*       libjvm_vm_run;
	const char* lterror;

	bool (*VM_create)(JavaVM**, void**, void*);
	void (*vm_run)(JavaVM*, JavaVMInitArgs*);
#endif

	// Prepare the options.
	JavaVMInitArgs* vm_args = prepare_options(argc, argv);
	
	/* load and initialize a Java VM, return a JNI interface pointer in env */

#if defined(ENABLE_LIBJVM)
# if defined(ENABLE_JRE_LAYOUT)
	/* SUN also uses a buffer of 4096-bytes (strace is your friend). */

	path = (char*) os::malloc(sizeof(char) * 4096);

	if (readlink("/proc/self/exe", path, 4095) == -1) {
		fprintf(stderr, "main: readlink failed: %s\n", strerror(errno));
		os::abort();
	}

	/* get the path of the current executable */

	path = os::dirname(path);
	len  = os::strlen(path) + os::strlen("/../lib/"LIBJVM_NAME) + os::strlen("0");

	if (len > 4096) {
		fprintf(stderr, "main: libjvm name to long for buffer\n");
		os::abort();
	}

	/* concatenate the library name */

	strcat(path, "/../lib/"LIBJVM_NAME);
# else
	path = (char*) CACAO_LIBDIR"/"LIBJVM_NAME;
# endif

	/* First try to open where dlopen searches, e.g. LD_LIBRARY_PATH.
	   If not found, try the absolute path. */

	libjvm_handle = os::dlopen(LIBJVM_NAME, RTLD_NOW);

	if (libjvm_handle == NULL) {
		/* save the error message */

		lterror = strdup(os::dlerror());

		libjvm_handle = os::dlopen(path, RTLD_NOW);

		if (libjvm_handle == NULL) {
			/* print the first error message too */

			fprintf(stderr, "main: os::dlopen failed: %s\n", lterror);

			/* and now the current one */

			fprintf(stderr, "main: os::dlopen failed: %s\n", os::dlerror());
			os::abort();
		}

		// Free the error string.
		os::free((void*) lterror);
	}

	libjvm_VM_create = os::dlsym(libjvm_handle, "VM_create");

	if (libjvm_VM_create == NULL) {
		fprintf(stderr, "main: lt_dlsym failed: %s\n", os::dlerror());
		os::abort();
	}

	VM_create = (bool (*)(JavaVM**, void**, void*)) (uintptr_t) libjvm_VM_create;
#endif

	// Create the Java VM.
	JavaVM* vm;
	void*   env; // We use a void* instead of a JNIEnv* here to prevent a compiler warning.

	(void) VM_create(&vm, &env, vm_args);

#if defined(ENABLE_LIBJVM)
	libjvm_vm_run = os::dlsym(libjvm_handle, "vm_run");

	if (libjvm_vm_run == NULL) {
		fprintf(stderr, "main: os::dlsym failed: %s\n", os::dlerror());
		os::abort();
	}

	vm_run = (void (*)(JavaVM*, JavaVMInitArgs*)) (uintptr_t) libjvm_vm_run;
#endif

	// Run the VM.
	vm_run(vm, vm_args);

	// Keep compiler happy.
	return 0;
}


/**
 * Prepare the JavaVMInitArgs structure.
 */
static JavaVMInitArgs* prepare_options(int argc, char** argv)
{
	JavaVMInitArgs* vm_args;

	vm_args = (JavaVMInitArgs*) malloc(sizeof(JavaVMInitArgs));

	vm_args->version            = JNI_VERSION_1_2;
	vm_args->nOptions           = argc - 1;
	vm_args->options            = (JavaVMOption*) malloc(sizeof(JavaVMOption) * argc);
	vm_args->ignoreUnrecognized = JNI_FALSE;

	for (int i = 1; i < argc; i++)
		vm_args->options[i - 1].optionString = argv[i];

	return vm_args;
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
 */
