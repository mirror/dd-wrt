/* src/vm/vm.hpp - basic JVM functions

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


#ifndef _VM_HPP
#define _VM_HPP

#include "config.h"
#include "vm/global.hpp"

#include <stdarg.h>
#include <stdint.h>

// We need the JNI types for the VM class.
#include "native/jni.hpp"
#include "native/native.hpp"

#if defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
# include "native/vm/openjdk/hpi.hpp"
#endif
# include "native/vm/openjdk/management.hpp"
#endif

#include "vm/properties.hpp"
#include "vm/suck.hpp"

#include "vm/jit/optimizing/recompiler.hpp"

struct methodinfo;

/**
 * Represent an instance of a VM.
 */
class VM {
private:
	// This is _the_ VM instance.
	static VM* _vm;

	// JNI variables.
	JavaVM* _javavm;
	JNIEnv* _jnienv;

	// VM variables.
	bool    _initializing;
	bool    _created;
	bool    _exiting;
	int64_t _starttime;
	int64_t _inittime;

	// Subsystems.
	Properties      _properties;      ///< Commandline properties.
#if defined(ENABLE_THREADS)
	Recompiler      _recompiler;      ///< JIT recompilation framework.
#endif
#if defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	HPI             _hpi;             ///< Host Porting Interface.
#else
	Mutex						_jniMutex;				///< XXX should be part of the not existing class wrapper for src/native/vm/openjdk/jvm.cpp
#endif
	Management      _management;      ///< Java management interface.
#endif
	NativeLibraries _nativelibraries; ///< Native library table.
	NativeMethods   _nativemethods;   ///< Native methods table.
#if defined(ENABLE_JVMTI)
	NativeAgents    _nativeagents;    ///< Native agents table.
#endif
	SuckClasspath   _suckclasspath;   ///< Classpath entries list.

public:
	// Constructor, Destructor.
	VM(JavaVMInitArgs*);
	~VM();

	// Static methods.
	static bool create(JavaVM** p_vm, void** p_env, void* vm_args);
	static VM*  get_current() { return _vm; }

	static void print_build_time_config();
	void        print_run_time_config();

	// Getters for private members.
	JavaVM* get_javavm()      { return _javavm; }
	JNIEnv* get_jnienv()      { return _jnienv; }
	bool    is_initializing() { return _initializing; }
	bool    is_created()      { return _created; }
	bool    is_exiting()      { return _exiting; }
	int64_t get_starttime()   { return _starttime; }
	int64_t get_inittime()    { return _inittime; }

	Properties&      get_properties     () { return _properties; }

#if defined(ENABLE_THREADS)
	Recompiler&      get_recompiler     () { return _recompiler; } // REMOVEME
#endif

#if defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	HPI&             get_hpi            () { return _hpi; }
#else
	Mutex&					 get_jniMutex			  () { return _jniMutex; }
#endif
	Management&      get_management     () { return _management; }
#endif
	NativeLibraries& get_nativelibraries() { return _nativelibraries; }
	NativeMethods&   get_nativemethods  () { return _nativemethods; }
	SuckClasspath&   get_suckclasspath  () { return _suckclasspath; }

private:
	// Internal helper methods.
	bool start_runtime_agents();
};

/* These C methods are the exported interface. ********************************/

extern "C" bool VM_create(JavaVM** p_vm, void** p_env, void* vm_args);


/* export global variables ****************************************************/

#if defined(ENABLE_INTRP)
extern uint8_t* intrp_main_stack;
#endif


/* function prototypes ********************************************************/

extern "C" {

void vm_run(JavaVM *vm, JavaVMInitArgs *vm_args);
int32_t   vm_destroy(JavaVM *vm);
void vm_exit(int32_t status);
void vm_shutdown(int32_t status);

void vm_exit_handler(void);

void vm_abort_disassemble(void *pc, int count, const char *text, ...);

/* Java method calling functions */

java_handle_t *vm_call_method(methodinfo *m, java_handle_t *o, ...);
java_handle_t *vm_call_method_valist(methodinfo *m, java_handle_t *o,
										 va_list ap);
java_handle_t *vm_call_method_jvalue(methodinfo *m, java_handle_t *o,
										 const jvalue *args);

int32_t vm_call_method_int(methodinfo *m, java_handle_t *o, ...);
int32_t vm_call_method_int_valist(methodinfo *m, java_handle_t *o, va_list ap);
int32_t vm_call_method_int_jvalue(methodinfo *m, java_handle_t *o, const jvalue *args);

int64_t vm_call_method_long(methodinfo *m, java_handle_t *o, ...);
int64_t vm_call_method_long_valist(methodinfo *m, java_handle_t *o, va_list ap);
int64_t vm_call_method_long_jvalue(methodinfo *m, java_handle_t *o, const jvalue *args);

float   vm_call_method_float(methodinfo *m, java_handle_t *o, ...);
float   vm_call_method_float_valist(methodinfo *m, java_handle_t *o, va_list ap);
float   vm_call_method_float_jvalue(methodinfo *m, java_handle_t *o, const jvalue *args);

double  vm_call_method_double(methodinfo *m, java_handle_t *o, ...);
double  vm_call_method_double_valist(methodinfo *m, java_handle_t *o, va_list ap);
double  vm_call_method_double_jvalue(methodinfo *m, java_handle_t *o, const jvalue *args);

java_handle_t *vm_call_method_objectarray(methodinfo *m, java_handle_t *o, java_handle_objectarray_t *params);


// Legacy C interface.
void vm_abort(const char* text, ...);

} // extern "C"

#endif // _VM_HPP


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
