/* src/vm/exceptions.cpp - exception related functions

   Copyright (C) 1996-2014
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

#include "vm/exceptions.hpp"
#include <stdint.h>                     // for uintptr_t
#include <cassert>                      // for assert
#include <cstring>
#include <cstdio>                       // for fprintf, printf, putc, etc
#include <cstdarg>                      // for va_list
#include "config.h"
#include "md-abi.hpp"
#include "jit/code.hpp"                 // for codeinfo, etc
#include "native/llni.hpp"
#include "native/native.hpp"            // for native_new_and_init_string, etc
#include "threads/lock.hpp"             // for lock_monitor_exit
#include "threads/thread.hpp"           // for threadobject, etc
#include "toolbox/buffer.hpp"           // for Buffer
#include "toolbox/logging.hpp"          // for log_finish, log_print, etc
#include "vm/class.hpp"                 // for classinfo, etc
#include "vm/global.hpp"                // for java_handle_t, etc
#include "vm/globals.hpp"               // for class_java_lang_Object, etc
#include "vm/javaobjects.hpp"           // for java_lang_Throwable, etc
#include "vm/jit/asmpart.hpp"
#include "vm/jit/builtin.hpp"           // for builtin_new, etc
#include "vm/jit/exceptiontable.hpp"    // for exceptiontable_entry_t, etc
#include "vm/jit/methodheader.hpp"
#include "vm/jit/patcher-common.hpp"
#include "vm/jit/show.hpp"
#include "vm/jit/stacktrace.hpp"        // for stacktrace_print_exception, etc
#include "vm/jit/trace.hpp"             // for trace_exception
#include "vm/linker.hpp"                // for link_class
#include "vm/loader.hpp"                // for load_class_bootstrap, etc
#include "vm/method.hpp"                // for methodinfo
#include "vm/options.hpp"               // for opt_TraceExceptions, etc
#include "vm/os.hpp"                    // for os
#include "vm/references.hpp"            // for classref_or_classinfo
#include "vm/resolve.hpp"               // for resolve_classref_eager
#include "vm/string.hpp"                // for JavaString
#include "vm/types.hpp"                 // for s4, u1, u4
#include "vm/vm.hpp"                    // for VM, vm_call_method

#define DEBUG_NAME "exceptions"

/***
 * Get the the name of a classinfo* for an exception message
 */
static inline Utf8String get_classname_for_exception(classinfo *c);

/* exceptions_get_exception ****************************************************

   Returns the current exception pointer of the current thread.

*******************************************************************************/

java_handle_t *exceptions_get_exception(void)
{
	threadobject *t = THREADOBJECT;

	/* Get the exception. */

	LLNI_CRITICAL_START;

	java_object_t *o = t->_exceptionptr;
	java_handle_t *e = LLNI_WRAP(o);

	LLNI_CRITICAL_END;

	/* Return the exception. */

	return e;
}


/* exceptions_set_exception ****************************************************

   Sets the exception pointer of the current thread.

*******************************************************************************/

void exceptions_set_exception(java_handle_t *e)
{
	threadobject *t = THREADOBJECT;

	/* Set the exception. */

	LLNI_CRITICAL_START;

	java_object_t *o = LLNI_UNWRAP(e);

	LOG("[exceptions_set_exception  : t=" << (void *) t
	    << ", o=" << (void *) o
	    << ", class=" << o->vftbl->clazz << "]" << cacao::nl);

	t->_exceptionptr = o;

	LLNI_CRITICAL_END;
}


/* exceptions_clear_exception **************************************************

   Clears the current exception pointer of the current thread.

*******************************************************************************/

void exceptions_clear_exception(void)
{
	threadobject *t = THREADOBJECT;

	/* Set the exception. */

	LOG("[exceptions_clear_exception: t=" << (void *) t << cacao::nl);

	t->_exceptionptr = NULL;
}


/* exceptions_get_and_clear_exception ******************************************

   Gets the exception pointer of the current thread and clears it.
   This function may return NULL.

*******************************************************************************/

java_handle_t *exceptions_get_and_clear_exception(void)
{
	java_handle_t *o;

	/* Get the exception... */

	o = exceptions_get_exception();

	/* ...and clear the exception if it is set. */

	if (o != NULL)
		exceptions_clear_exception();

	/* return the exception */

	return o;
}


/* exceptions_abort ************************************************************

   Prints exception to be thrown and aborts.

   IN:
      classname....class name
      message......exception message

*******************************************************************************/

static void exceptions_abort(Utf8String classname, Utf8String message)
{
	log_println("exception thrown while VM is initializing: ");

	log_start();
	utf_display_printable_ascii_classname(classname);

	if (message != NULL) {
		log_print(": ");
		utf_display_printable_ascii_classname(message);
	}

	log_finish();

	os::abort("Aborting...");
}


/* exceptions_new_class_Utf8String ***************************************************

   Creates an exception object with the given class and initalizes it
   with the given utf message.

   IN:
      c ......... exception class
	  message ... the message as an Utf8String 

   RETURN VALUE:
     an exception pointer (in any case -- either it is the newly
     created exception, or an exception thrown while trying to create
     it).

*******************************************************************************/

static java_handle_t *exceptions_new_class_utf(classinfo *c, Utf8String message)
{
	java_handle_t *s;
	java_handle_t *o;

	if (VM::get_current()->is_initializing()) {
		/* This can happen when global class variables are used which
		   are not initialized yet. */

		if (c == NULL)
			exceptions_abort(NULL, message);
		else
			exceptions_abort(c->name, message);
	}

	s = JavaString::from_utf8(message);

	if (s == NULL)
		return exceptions_get_exception();

	o = native_new_and_init_string(c, s);

	if (o == NULL)
		return exceptions_get_exception();

	return o;
}


/* exceptions_new_Utf8String *********************************************************

   Creates an exception object with the given name and initalizes it.

   IN:
      classname....class name in UTF-8

*******************************************************************************/

static java_handle_t *exceptions_new_utf(Utf8String classname)
{
	classinfo     *c;
	java_handle_t *o;

	if (VM::get_current()->is_initializing())
		exceptions_abort(classname, NULL);

	c = load_class_bootstrap(classname);

	if (c == NULL)
		return exceptions_get_exception();

	o = native_new_and_init(c);

	if (o == NULL)
		return exceptions_get_exception();

	return o;
}


/* exceptions_new_utf_javastring ***********************************************

   Creates an exception object with the given name and initalizes it
   with the given java/lang/String message.

   IN:
      classname....class name in UTF-8
	  message......the message as a java.lang.String

   RETURN VALUE:
      an exception pointer (in any case -- either it is the newly created
	  exception, or an exception thrown while trying to create it).

*******************************************************************************/

static java_handle_t *exceptions_new_utf_javastring(Utf8String classname,
													java_handle_t *message)
{
	java_handle_t *o;
	classinfo     *c;
   
	if (VM::get_current()->is_initializing())
		exceptions_abort(classname, NULL);

	c = load_class_bootstrap(classname);

	if (c == NULL)
		return exceptions_get_exception();

	o = native_new_and_init_string(c, message);

	if (o == NULL)
		return exceptions_get_exception();

	return o;
}


/* exceptions_new_utf_Utf8String *****************************************************

   Creates an exception object with the given name and initalizes it
   with the given utf message.

   IN:
      classname....class name in UTF-8
	  message......the message as an Utf8String 

   RETURN VALUE:
      an exception pointer (in any case -- either it is the newly created
	  exception, or an exception thrown while trying to create it).

*******************************************************************************/

static java_handle_t *exceptions_new_utf_utf(Utf8String classname, Utf8String message)
{
	classinfo     *c;
	java_handle_t *o;

	if (VM::get_current()->is_initializing())
		exceptions_abort(classname, message);

	c = load_class_bootstrap(classname);

	if (c == NULL)
		return exceptions_get_exception();

	o = exceptions_new_class_utf(c, message);

	return o;
}


/* exceptions_throw_Utf8String *******************************************************

   Creates an exception object with the given name, initalizes and
   throws it.

   IN:
      classname....class name in UTF-8

*******************************************************************************/

static void exceptions_throw_utf(Utf8String classname)
{
	java_handle_t *o;

	o = exceptions_new_utf(classname);

	if (o == NULL)
		return;

	exceptions_set_exception(o);
}


/* exceptions_throw_utf_throwable **********************************************

   Creates an exception object with the given name and initalizes it
   with the given java/lang/Throwable exception.

   IN:
      classname....class name in UTF-8
	  cause........the given Throwable

*******************************************************************************/

static void exceptions_throw_utf_throwable(Utf8String classname,
										   java_handle_t *cause)
{
	classinfo           *c;
	methodinfo          *m;

	if (VM::get_current()->is_initializing())
		exceptions_abort(classname, NULL);

	java_lang_Throwable jlt(cause);

	c = load_class_bootstrap(classname);

	if (c == NULL)
		return;

	/* create object */

	java_handle_t* h = builtin_new(c);

	if (h == NULL)
		return;

	/* call initializer */

	m = class_resolveclassmethod(c,
								 utf8::init,
								 utf8::java_lang_Throwable__void,
								 NULL,
								 true);
	                      	                      
	if (m == NULL)
		return;

	(void) vm_call_method(m, h, jlt.get_handle());

	exceptions_set_exception(h);
}


/* exceptions_throw_utf_exception **********************************************

   Creates an exception object with the given name and initalizes it
   with the given java/lang/Exception exception.

   IN:
      classname....class name in UTF-8
	  exception....the given Exception

*******************************************************************************/

static void exceptions_throw_utf_exception(Utf8String classname,
										   java_handle_t *exception)
{
	classinfo     *c;
	java_handle_t *o;
	methodinfo    *m;

	if (VM::get_current()->is_initializing())
		exceptions_abort(classname, NULL);

	c = load_class_bootstrap(classname);

	if (c == NULL)
		return;

	/* create object */

	o = builtin_new(c);
	
	if (o == NULL)
		return;

	/* call initializer */

	m = class_resolveclassmethod(c,
								 utf8::init,
								 utf8::java_lang_Exception__V,
								 NULL,
								 true);
	                      	                      
	if (m == NULL)
		return;

	(void) vm_call_method(m, o, exception);

	exceptions_set_exception(o);
}


/* exceptions_throw_utf_cause **************************************************

   Creates an exception object with the given name and initalizes it
   with the given java/lang/Throwable exception with initCause.

   IN:
      classname....class name in UTF-8
	  cause........the given Throwable

*******************************************************************************/

static void exceptions_throw_utf_cause(Utf8String classname, java_handle_t *cause)
{
	if (VM::get_current()->is_initializing())
		exceptions_abort(classname, NULL);

	java_lang_Throwable jltcause(cause);

	classinfo* c = load_class_bootstrap(classname);

	if (c == NULL)
		return;

	/* create object */

	java_handle_t* h = builtin_new(c);
	
	if (h == NULL)
		return;

	/* call initializer */

	methodinfo* m = class_resolveclassmethod(c,
											 utf8::init,
											 utf8::java_lang_String__void,
											 NULL,
											 true);
	                      	                      
	if (m == NULL)
		return;

	(void) vm_call_method(m, h, jltcause.get_detailMessage());

	/* call initCause */

	m = class_resolveclassmethod(c,
								 utf8::initCause,
								 utf8::java_lang_Throwable__java_lang_Throwable,
								 NULL,
								 true);

	if (m == NULL)
		return;

	(void) vm_call_method(m, h, jltcause.get_handle());

	exceptions_set_exception(h);
}


/* exceptions_throw_utf_Utf8String ***************************************************

   Creates an exception object with the given name, initalizes and
   throws it with the given utf message.

   IN:
      classname....class name in UTF-8
	  message......the message as an Utf8String 

*******************************************************************************/

static void exceptions_throw_utf_utf(Utf8String classname, Utf8String message)
{
	java_handle_t *o;

	o = exceptions_new_utf_utf(classname, message);

	exceptions_set_exception(o);
}


/* exceptions_new_abstractmethoderror ****************************************

   Generates a java.lang.AbstractMethodError for the VM.

*******************************************************************************/

java_handle_t *exceptions_new_abstractmethoderror(void)
{
	java_handle_t *o;

	o = exceptions_new_utf(utf8::java_lang_AbstractMethodError);

	return o;
}


/* exceptions_new_error ********************************************************

   Generates a java.lang.Error for the VM.

*******************************************************************************/

#if defined(ENABLE_JAVAME_CLDC1_1)
static java_handle_t *exceptions_new_error(Utf8String message)
{
	java_handle_t *o;

	o = exceptions_new_utf_utf(utf8::java_lang_Error, message);

	return o;
}
#endif


/* exceptions_asm_new_abstractmethoderror **************************************

   Generates a java.lang.AbstractMethodError for
   asm_abstractmethoderror.

*******************************************************************************/

java_object_t *exceptions_asm_new_abstractmethoderror(u1 *sp, u1 *ra)
{
	stackframeinfo_t  sfi;
	java_handle_t    *e;
	java_object_t    *o;

	/* Fill and add a stackframeinfo (XPC is equal to RA). */

	stacktrace_stackframeinfo_add(&sfi, NULL, sp, ra, ra);

	/* create the exception */

#if defined(ENABLE_JAVASE)
	e = exceptions_new_abstractmethoderror();
#else
	e = exceptions_new_error(utf8::java_lang_AbstractMethodError);
#endif

	/* Remove the stackframeinfo. */

	stacktrace_stackframeinfo_remove(&sfi);

	/* unwrap the exception */
	/* ATTENTION: do the this _after_ the stackframeinfo was removed */

	o = LLNI_UNWRAP(e);

	return o;
}


/* exceptions_new_arraystoreexception ******************************************

   Generates a java.lang.ArrayStoreException for the VM.

*******************************************************************************/

java_handle_t *exceptions_new_arraystoreexception(void)
{
	java_handle_t *o;

	o = exceptions_new_utf(utf8::java_lang_ArrayStoreException);

	return o;
}


/* exceptions_throw_abstractmethoderror ****************************************

   Generates and throws a java.lang.AbstractMethodError for the VM.

*******************************************************************************/

void exceptions_throw_abstractmethoderror(void)
{
	exceptions_throw_utf(utf8::java_lang_AbstractMethodError);
}


/* exceptions_throw_classcircularityerror **************************************

   Generates and throws a java.lang.ClassCircularityError for the
   classloader.

   IN:
      c....the class in which the error was found

*******************************************************************************/

void exceptions_throw_classcircularityerror(classinfo *c)
{
	exceptions_throw_utf_utf(utf8::java_lang_ClassCircularityError, c->name);
}


/* exceptions_throw_classformaterror *******************************************

   Generates and throws a java.lang.ClassFormatError for the VM.

   IN:
      c............the class in which the error was found
	  message......UTF-8 format string

*******************************************************************************/

void exceptions_throw_classformaterror(classinfo *c, const char *message, ...)
{
	va_list  ap;

	/* allocate a buffer */

	Buffer<> buf;

	/* print message into allocated buffer */

	if (c != NULL)
		buf.write_slash_to_dot(c->name).write('(');

	va_start(ap, message);
	buf.writevf(message,ap);
	va_end(ap);

	if (c != NULL)
		buf.write(')');

	Utf8String u = buf.utf8_str();
	assert(u);

	/* throw exception */

	exceptions_throw_classformaterror(c, u);
}

void exceptions_throw_classformaterror(classinfo *c, Utf8String message)
{
	/* throw exception */

	assert(message);

	exceptions_throw_utf_utf(utf8::java_lang_ClassFormatError, message);
}

/* exceptions_throw_classnotfoundexception *************************************

   Generates and throws a java.lang.ClassNotFoundException for the
   VM.

   IN:
      name.........name of the class not found as a Utf8String 

*******************************************************************************/

void exceptions_throw_classnotfoundexception(Utf8String name)
{	
	// We can't use the cached class_java_lang_ClassNotFoundException because
	// when there are bootstrap classpath problems it has not been set yet,
	// which leads to confusing error messages.

	exceptions_throw_utf_utf(utf8::java_lang_ClassNotFoundException, name);
}


/* exceptions_throw_noclassdeffounderror ***************************************

   Generates and throws a java.lang.NoClassDefFoundError.

   IN:
      name.........name of the class not found as a Utf8String 

*******************************************************************************/

void exceptions_throw_noclassdeffounderror(Utf8String name)
{
	exceptions_throw_utf_utf(utf8::java_lang_NoClassDefFoundError, name);
}


/* exceptions_throw_noclassdeffounderror_cause *********************************

   Generates and throws a java.lang.NoClassDefFoundError with the
   given cause.

*******************************************************************************/

void exceptions_throw_noclassdeffounderror_cause(java_handle_t *cause)
{
	exceptions_throw_utf_cause(utf8::java_lang_NoClassDefFoundError, cause);
}


/* exceptions_throw_noclassdeffounderror_wrong_name ****************************

   Generates and throws a java.lang.NoClassDefFoundError with a
   specific message:

   IN:
      name.........name of the class not found as a Utf8String 

*******************************************************************************/

void exceptions_throw_noclassdeffounderror_wrong_name(classinfo *c, Utf8String name)
{
	Buffer<> buf;

	buf.write_slash_to_dot(c->name)
	   .write(" (wrong name: ", 14)
	   .write_slash_to_dot(name)
	   .write(')');

	exceptions_throw_noclassdeffounderror(buf.utf8_str());
}


/* exceptions_throw_exceptionininitializererror ********************************

   Generates and throws a java.lang.ExceptionInInitializerError for
   the VM.

   IN:
      cause......cause exception object

*******************************************************************************/

void exceptions_throw_exceptionininitializererror(java_handle_t *cause)
{
	exceptions_throw_utf_throwable(utf8::java_lang_ExceptionInInitializerError,
								   cause);
}


/* exceptions_throw_incompatibleclasschangeerror *******************************

   Generates and throws a java.lang.IncompatibleClassChangeError for
   the VM.

   IN:
      message......UTF-8 message format string

*******************************************************************************/

void exceptions_throw_incompatibleclasschangeerror(classinfo *c, const char *message)
{
	/* allocate memory */

	Buffer<> buf;

	buf.write_slash_to_dot(c->name)
	   .write(message);

	/* throw exception */

	exceptions_throw_utf_utf(utf8::java_lang_IncompatibleClassChangeError, buf.utf8_str());
}


/* exceptions_throw_instantiationerror *****************************************

   Generates and throws a java.lang.InstantiationError for the VM.

*******************************************************************************/

void exceptions_throw_instantiationerror(classinfo *c)
{
	exceptions_throw_utf_utf(utf8::java_lang_InstantiationError, c->name);
}


/* exceptions_throw_internalerror **********************************************

   Generates and throws a java.lang.InternalError for the VM.

   IN:
      message......UTF-8 message format string

*******************************************************************************/

void exceptions_throw_internalerror(const char *message, ...)
{
	va_list  ap;
	Buffer<> buf;

	/* generate message */

	va_start(ap, message);
	buf.writevf(message,ap);
	va_end(ap);

	/* throw exception */

	exceptions_throw_utf_utf(utf8::java_lang_InternalError, buf.utf8_str());
}


/* exceptions_throw_linkageerror ***********************************************

   Generates and throws java.lang.LinkageError with an error message.

   IN:
      message......UTF-8 message, can be freed after the call
	  c............class related to the error. If this is != NULL
	               the name of c is appended to the error message.

*******************************************************************************/

void exceptions_throw_linkageerror(const char *message, classinfo *c)
{
	/* generate message */

	Buffer<> buf;

	if (c) {
		buf.write_slash_to_dot(c->name)
		   .write(": ");		
	}

	buf.write(message);

	Utf8String msg = buf.utf8_str();

	exceptions_throw_utf_utf(utf8::java_lang_LinkageError, msg);
}


/* exceptions_throw_nosuchfielderror *******************************************

   Generates and throws a java.lang.NoSuchFieldError with an error
   message.

   IN:
      c............class in which the field was not found
	  name.........name of the field

*******************************************************************************/

void exceptions_throw_nosuchfielderror(classinfo *c, Utf8String name)
{
	/* generate message */

	Buffer<> buf;

	buf.write_slash_to_dot(c->name)
	   .write('.')
	   .write(name);

	exceptions_throw_utf_utf(utf8::java_lang_NoSuchFieldError, buf.utf8_str());
}


/* exceptions_throw_nosuchmethoderror ******************************************

   Generates and throws a java.lang.NoSuchMethodError with an error
   message.

   IN:
      c............class in which the method was not found
	  name.........name of the method
	  desc.........descriptor of the method

*******************************************************************************/

void exceptions_throw_nosuchmethoderror(classinfo *c, Utf8String name, Utf8String desc)
{
	/* generate message */

	Buffer<> buf;

	buf.write_slash_to_dot(c->name)
	   .write('.')
	   .write(name)
	   .write(desc);

#if defined(ENABLE_JAVASE)
	exceptions_throw_utf_utf(utf8::java_lang_NoSuchMethodError, buf.utf8_str());
#else
	exceptions_throw_utf_utf(utf8::java_lang_Error, buf.utf8_str());
#endif
}


/* exceptions_throw_outofmemoryerror *******************************************

   Generates and throws an java.lang.OutOfMemoryError for the VM.

*******************************************************************************/

void exceptions_throw_outofmemoryerror(void)
{
	exceptions_throw_utf_utf(utf8::java_lang_OutOfMemoryError,
	                         Utf8String::from_utf8("Java heap space"));
}


/* exceptions_throw_unsatisfiedlinkerror ***************************************

   Generates and throws a java.lang.UnsatisfiedLinkError for the
   classloader.

   IN:
	  name......UTF-8 name string

*******************************************************************************/

void exceptions_throw_unsatisfiedlinkerror(Utf8String name)
{
#if defined(ENABLE_JAVASE)
	exceptions_throw_utf_utf(utf8::java_lang_UnsatisfiedLinkError, name);
#else
	exceptions_throw_utf_utf(utf8::java_lang_Error, name);
#endif
}


/* exceptions_throw_unsupportedclassversionerror *******************************

   Generates and throws a java.lang.UnsupportedClassVersionError for
   the classloader.

*******************************************************************************/

void exceptions_throw_unsupportedclassversionerror(classinfo *c)
{
	/* generate message */

	Buffer<> buf;

	buf.write_slash_to_dot(c->name)
	   .writef(" (Unsupported major.minor version %d.%d)", c->version.majr(), c->version.minr());

	/* throw exception */

	exceptions_throw_utf_utf(utf8::java_lang_UnsupportedClassVersionError, buf.utf8_str());
}


/* exceptions_throw_verifyerror ************************************************

   Generates and throws a java.lang.VerifyError for the JIT compiler.

   IN:
      m............method in which the error was found
	  message......UTF-8 format string

*******************************************************************************/

void exceptions_throw_verifyerror(methodinfo *m, const char *message, ...)
{
	va_list  ap;
	Buffer<> buf;

	/* generate message */

	if (m != NULL) {
		buf.write("(class: ")
		   .write_slash_to_dot(m->clazz->name)
		   .write(", method: ")
		   .write(m->name)
		   .write(" signature: ")
		   .write(m->descriptor)
		   .write(") ");
	}

	va_start(ap, message);
	buf.writevf(message, ap);
	va_end(ap);

	/* throw exception */

	exceptions_throw_utf_utf(utf8::java_lang_VerifyError, buf.utf8_str());
}


/* exceptions_throw_verifyerror_for_stack **************************************

   throws a java.lang.VerifyError for an invalid stack slot type

   IN:
      m............method in which the error was found
	  type.........the expected type

   RETURN VALUE:
      an exception pointer (in any case -- either it is the newly created
	  exception, or an exception thrown while trying to create it).

*******************************************************************************/

void exceptions_throw_verifyerror_for_stack(methodinfo *m, int type)
{
	/* generate message */

	Buffer<> buf;

	if (m != NULL) {
		buf.write("(class: ")
		   .write_slash_to_dot(m->clazz->name)
		   .write(", method: ")
		   .write(m->name)
		   .write(" signature: ")
		   .write(m->descriptor)
		   .write(") ");
	}

	buf.write("Expecting to find ");

	switch (type) {
	case TYPE_INT: buf.write("integer");              break;
	case TYPE_LNG: buf.write("long");                 break;
	case TYPE_FLT: buf.write("float");                break;
	case TYPE_DBL: buf.write("double");               break;
	case TYPE_ADR: buf.write("object/array");         break;
	case TYPE_RET: buf.write("returnAddress");        break;
	default:       buf.write("<INVALID>"); assert(0); break;
	}

	buf.write(" on stack");

	/* throw exception */

	exceptions_throw_utf_utf(utf8::java_lang_VerifyError, buf.utf8_str());
}


/* exceptions_new_arithmeticexception ******************************************

   Generates a java.lang.ArithmeticException for the JIT compiler.

*******************************************************************************/

java_handle_t *exceptions_new_arithmeticexception(void)
{
	java_handle_t *o;

	o = exceptions_new_utf_utf(utf8::java_lang_ArithmeticException,
							   utf8::division_by_zero);

	return o;
}


/* exceptions_new_arrayindexoutofboundsexception *******************************

   Generates a java.lang.ArrayIndexOutOfBoundsException for the VM
   system.

*******************************************************************************/

java_handle_t *exceptions_new_arrayindexoutofboundsexception(s4 index)
{
	java_handle_t *o;
	methodinfo    *m;
	java_handle_t *s;

	/* convert the index into a String, like Sun does */

	m = class_resolveclassmethod(class_java_lang_String,
								 Utf8String::from_utf8("valueOf"),
								 Utf8String::from_utf8("(I)Ljava/lang/String;"),
								 class_java_lang_Object,
								 true);

	if (m == NULL)
		return exceptions_get_exception();

	s = vm_call_method(m, NULL, index);

	if (s == NULL)
		return exceptions_get_exception();

	o = exceptions_new_utf_javastring(utf8::java_lang_ArrayIndexOutOfBoundsException,
									  s);

	if (o == NULL)
		return exceptions_get_exception();

	return o;
}


/* exceptions_throw_arrayindexoutofboundsexception *****************************

   Generates and throws a java.lang.ArrayIndexOutOfBoundsException for
   the VM.

*******************************************************************************/

void exceptions_throw_arrayindexoutofboundsexception(void)
{
	exceptions_throw_utf(utf8::java_lang_ArrayIndexOutOfBoundsException);
}


/* exceptions_throw_arraystoreexception ****************************************

   Generates and throws a java.lang.ArrayStoreException for the VM.

*******************************************************************************/

void exceptions_throw_arraystoreexception(void)
{
	exceptions_throw_utf(utf8::java_lang_ArrayStoreException);
}


/* exceptions_new_classcastexception *******************************************

   Generates a java.lang.ClassCastException for the JIT compiler.

*******************************************************************************/

java_handle_t *exceptions_new_classcastexception(java_handle_t *o)
{
	classinfo *c;

	LLNI_class_get(o, c);

	Utf8String classname = get_classname_for_exception(c);

	return exceptions_new_utf_utf(utf8::java_lang_ClassCastException, classname);
}


/* exceptions_throw_clonenotsupportedexception *********************************

   Generates and throws a java.lang.CloneNotSupportedException for the
   VM.

*******************************************************************************/

void exceptions_throw_clonenotsupportedexception(void)
{
	exceptions_throw_utf(utf8::java_lang_CloneNotSupportedException);
}


/* exceptions_throw_illegalaccessexception *************************************

   Generates and throws a java.lang.IllegalAccessException for the VM.

*******************************************************************************/

void exceptions_throw_illegalaccessexception(Utf8String message)
{
	exceptions_throw_utf_utf(utf8::java_lang_IllegalAccessException, message);
}


/* exceptions_throw_illegalargumentexception ***********************************

   Generates and throws a java.lang.IllegalArgumentException for the
   VM.

*******************************************************************************/

void exceptions_throw_illegalargumentexception(void)
{
	exceptions_throw_utf(utf8::java_lang_IllegalArgumentException);
}


/* exceptions_throw_illegalmonitorstateexception *******************************

   Generates and throws a java.lang.IllegalMonitorStateException for
   the VM.

*******************************************************************************/

void exceptions_throw_illegalmonitorstateexception(void)
{
	exceptions_throw_utf(utf8::java_lang_IllegalMonitorStateException);
}


/* exceptions_throw_instantiationexception *************************************

   Generates and throws a java.lang.InstantiationException for the VM.

*******************************************************************************/

void exceptions_throw_instantiationexception(classinfo *c)
{
	exceptions_throw_utf_utf(utf8::java_lang_InstantiationException, c->name);
}


/* exceptions_throw_interruptedexception ***************************************

   Generates and throws a java.lang.InterruptedException for the VM.

*******************************************************************************/

void exceptions_throw_interruptedexception(void)
{
	exceptions_throw_utf(utf8::java_lang_InterruptedException);
}


/* exceptions_throw_invocationtargetexception **********************************

   Generates and throws a java.lang.reflect.InvocationTargetException
   for the VM.

   IN:
      cause......cause exception object

*******************************************************************************/

void exceptions_throw_invocationtargetexception(java_handle_t *cause)
{
	exceptions_throw_utf_throwable(utf8::java_lang_reflect_InvocationTargetException,
								   cause);
}


/* exceptions_throw_negativearraysizeexception *********************************

   Generates and throws a java.lang.NegativeArraySizeException for the
   VM.

*******************************************************************************/

void exceptions_throw_negativearraysizeexception(void)
{
	exceptions_throw_utf(utf8::java_lang_NegativeArraySizeException);
}


/* exceptions_new_nullpointerexception *****************************************

   Generates a java.lang.NullPointerException for the VM system.

*******************************************************************************/

java_handle_t *exceptions_new_nullpointerexception(void)
{
	java_handle_t *o;

	o = exceptions_new_utf(utf8::java_lang_NullPointerException);

	return o;
}


/* exceptions_throw_nullpointerexception ***************************************

   Generates a java.lang.NullPointerException for the VM system and
   throw it in the VM system.

*******************************************************************************/

void exceptions_throw_nullpointerexception(void)
{
	exceptions_throw_utf(utf8::java_lang_NullPointerException);
}


/* exceptions_throw_privilegedactionexception **********************************

   Generates and throws a java.security.PrivilegedActionException.

*******************************************************************************/

void exceptions_throw_privilegedactionexception(java_handle_t *exception)
{
	exceptions_throw_utf_exception(utf8::java_security_PrivilegedActionException,
								   exception);
}


/* exceptions_throw_stringindexoutofboundsexception ****************************

   Generates and throws a java.lang.StringIndexOutOfBoundsException
   for the VM.

*******************************************************************************/

void exceptions_throw_stringindexoutofboundsexception(void)
{
	exceptions_throw_utf(utf8::java_lang_StringIndexOutOfBoundsException);
}


/* exceptions_fillinstacktrace *************************************************

   Calls the fillInStackTrace-method of the currently thrown
   exception.

*******************************************************************************/

java_handle_t *exceptions_fillinstacktrace(void)
{
	java_handle_t *o;
	classinfo     *c;
	methodinfo    *m;

	/* get exception */

	o = exceptions_get_and_clear_exception();

	assert(o);

	/* resolve methodinfo pointer from exception object */

	LLNI_class_get(o, c);

#if defined(ENABLE_JAVASE)
	m = class_resolvemethod(c,
							utf8::fillInStackTrace,
							utf8::void__java_lang_Throwable);
#elif defined(ENABLE_JAVAME_CLDC1_1)
	m = class_resolvemethod(c,
							utf8::fillInStackTrace,
							utf8::void__void);
#else
#error IMPLEMENT ME!
#endif

	/* call function */

	(void) vm_call_method(m, o);

	/* return exception object */

	return o;
}


/* exceptions_handle_exception *************************************************

   Try to find an exception handler for the given exception and return it.
   If no handler is found, exit the monitor of the method (if any)
   and return NULL.

   IN:
      xptr.........the exception object
	  xpc..........PC of where the exception was thrown
	  pv...........Procedure Value of the current method
	  sp...........current stack pointer

   RETURN VALUE:
      the address of the first matching exception handler, or
	  NULL if no handler was found

*******************************************************************************/

#if defined(ENABLE_JIT)
extern "C" void *exceptions_handle_exception(java_object_t *xptro, void *xpc, void *pv, void *sp)
{
	stackframeinfo_t        sfi;
	java_handle_t          *xptr;
	methodinfo             *m;
	codeinfo               *code;
	exceptiontable_t       *et;
	exceptiontable_entry_t *ete;
	s4                      i;
	classref_or_classinfo   cr;
	classinfo              *c;
	void                   *result;

#ifdef __S390__
	/* Addresses are 31 bit integers */
#	define ADDR_MASK(x) (void *) ((uintptr_t) (x) & 0x7FFFFFFF)
#else
#	define ADDR_MASK(x) (x)
#endif

	xptr = LLNI_WRAP(xptro);
	xpc  = ADDR_MASK(xpc);

	/* Fill and add a stackframeinfo (XPC is equal to RA). */

	stacktrace_stackframeinfo_add(&sfi, pv, sp, xpc, xpc);

	result = NULL;

	/* Get the codeinfo for the current method. */

	code = code_get_codeinfo_for_pv(pv);

	/* Get the methodinfo pointer from the codeinfo pointer. For
	   asm_vm_call_method the codeinfo pointer is NULL and we simply
	   can return the proper exception handler. */

	if (code == NULL) {
		result = (void *) (uintptr_t) &asm_vm_call_method_exception_handler;
		goto exceptions_handle_exception_return;
	}

	m = code->m;

#if !defined(NDEBUG)
	/* print exception trace */

	if (opt_TraceExceptions)
		trace_exception(LLNI_DIRECT(xptr), m, xpc);
#endif

	/* Get the exception table. */

	et = code->exceptiontable;

	if (et != NULL) {
	/* Iterate over all exception table entries. */

	ete = et->entries;

	for (i = 0; i < et->length; i++, ete++) {
		/* is the xpc is the current catch range */

		if ((ADDR_MASK(ete->startpc) <= xpc) && (xpc < ADDR_MASK(ete->endpc))) {
			cr = ete->catchtype;

			/* NULL catches everything */

			if (cr.any == NULL) {
#if !defined(NDEBUG)
				/* Print stacktrace of exception when caught. */

				if (opt_TraceExceptions) {
					exceptions_print_exception(xptr);
					stacktrace_print_exception(xptr);
				}
#endif

				result = ete->handlerpc;
				goto exceptions_handle_exception_return;
			}

			/* resolve or load/link the exception class */

			if (cr.is_classref()) {
				/* The exception class reference is unresolved. */
				/* We have to do _eager_ resolving here. While the
				   class of the exception object is guaranteed to be
				   loaded, it may well have been loaded by a different
				   loader than the defining loader of m's class, which
				   is the one we must use to resolve the catch
				   class. Thus lazy resolving might fail, even if the
				   result of the resolution would be an already loaded
				   class. */

				c = resolve_classref_eager(cr.ref);

				if (c == NULL) {
					/* Exception resolving the exception class, argh! */
					goto exceptions_handle_exception_return;
				}

				/* Ok, we resolved it. Enter it in the table, so we
				   don't have to do this again. */
				/* XXX this write should be atomic. Is it? */

				ete->catchtype.cls = c;
			}
			else {
				c = cr.cls;

				/* XXX I don't think this case can ever happen. -Edwin */
				if (!(c->state & CLASS_LOADED))
					/* use the methods' classloader */
					if (!load_class_from_classloader(c->name,
													 m->clazz->classloader))
						goto exceptions_handle_exception_return;

				/* XXX I think, if it is not linked, we can be sure
				   that the exception object is no (indirect) instance
				   of it, no?  -Edwin  */
				if (!(c->state & CLASS_LINKED))
					if (!link_class(c))
						goto exceptions_handle_exception_return;
			}

			/* is the thrown exception an instance of the catch class? */

			if (builtin_instanceof(xptr, c)) {
#if !defined(NDEBUG)
				/* Print stacktrace of exception when caught. */

				if (opt_TraceExceptions) {
					exceptions_print_exception(xptr);
					stacktrace_print_exception(xptr);
				}
#endif

				result = ete->handlerpc;
				goto exceptions_handle_exception_return;
			}
		}
	}
	}

	/* Is this method realization synchronized? */

	if (code_is_synchronized(code)) {
		/* Get synchronization object. */

		java_object_t *o = *((java_object_t **) (((uintptr_t) sp) + code->synchronizedoffset));

		assert(o != NULL);

		lock_monitor_exit(LLNI_QUICKWRAP(o));
	}

	/* none of the exceptions catch this one */

#if !defined(NDEBUG)

# if defined(ENABLE_DEBUG_FILTER)
	if (show_filters_test_verbosecall_exit(m)) {
# endif

	/* outdent the log message */

	if (opt_verbosecall) {
		if (TRACEJAVACALLINDENT)
			TRACEJAVACALLINDENT--;
		else
			log_text("exceptions_handle_exception: WARNING: unmatched unindent");
	}

# if defined(ENABLE_DEBUG_FILTER)
	}
# endif
#endif /* !defined(NDEBUG) */

	result = NULL;

exceptions_handle_exception_return:

	/* Remove the stackframeinfo. */

	stacktrace_stackframeinfo_remove(&sfi);

	return result;
}
#endif /* defined(ENABLE_JIT) */


/* exceptions_print_exception **************************************************

   Prints an exception, the detail message and the cause, if
   available, with CACAO internal functions to stdout.

*******************************************************************************/

void exceptions_print_exception(java_handle_t *xptr)
{
	java_lang_Throwable jlt(xptr);

	if (jlt.is_null()) {
		puts("NULL\n");
		return;
	}

#if defined(ENABLE_JAVASE)
	java_lang_Throwable jltcause(jlt.get_cause());
#endif

	/* print the root exception */

	classinfo* c = jlt.get_Class();
	utf_display_printable_ascii_classname(c->name);

	java_lang_String jls(jlt.get_detailMessage());

	if (!jls.is_null()) {
		JavaString str = jls.get_handle();

		printf(": ");
		str.fprint_printable_ascii(stdout);
	}

	putc('\n', stdout);

#if defined(ENABLE_JAVASE)
	/* print the cause if available */

	// FIXME cause != t compare with operator override.
	if ((!jltcause.is_null()) && (jltcause.get_handle() != jlt.get_handle())) {
		printf("Caused by: ");

		c = jltcause.get_Class();
		utf_display_printable_ascii_classname(c->name);

		java_lang_String jlscause(jlt.get_detailMessage());

		if (jlscause.get_handle() != NULL) {
			JavaString str = jls.get_handle();

			printf(": ");
			str.fprint_printable_ascii(stdout);
		}

		putc('\n', stdout);
	}
#endif
}


/* exceptions_print_current_exception ******************************************

   Prints the current pending exception, the detail message and the
   cause, if available, with CACAO internal functions to stdout.

*******************************************************************************/

void exceptions_print_current_exception(void)
{
	java_handle_t *o;

	o = exceptions_get_exception();

	exceptions_print_exception(o);
}


/* exceptions_print_stacktrace *************************************************

   Prints a pending exception with Throwable.printStackTrace().  If
   there happens an exception during printStackTrace(), we print the
   thrown exception and the original one.

   NOTE: This function calls Java code.

*******************************************************************************/

void exceptions_print_stacktrace(void)
{
	java_handle_t    *e;
	java_handle_t    *ne;
	classinfo        *c;
	methodinfo       *m;

	/* Get and clear exception because we are calling Java code
	   again. */

	e = exceptions_get_and_clear_exception();

	if (e == NULL)
		return;

#if 0
	/* FIXME Enable me. */
	if (builtin_instanceof(e, class_java_lang_ThreadDeath)) {
		/* Don't print anything if we are being killed. */
	}
	else
#endif
	{
		/* Get the exception class. */

		LLNI_class_get(e, c);

		/* Find the printStackTrace() method. */

		m = class_resolveclassmethod(c,
									 utf8::printStackTrace,
									 utf8::void__void,
									 class_java_lang_Object,
									 false);

		if (m == NULL)
			os::abort("exceptions_print_stacktrace: printStackTrace()V not found");

		/* Print message. */

		fprintf(stderr, "Exception ");

		/* Print thread name.  We get the thread here explicitly as we
		   need it afterwards. */

		threadobject     *t  = thread_get_current();
		java_lang_Thread *to = (java_lang_Thread *) LLNI_WRAP(t->object);

		if (to != NULL) {
			fprintf(stderr, "in thread \"");
			thread_fprint_name(t, stderr);
			fprintf(stderr, "\" ");
		}

		/* Print the stacktrace. */

		if (builtin_instanceof(e, class_java_lang_Throwable)) {
			(void) vm_call_method(m, e);

			/* If this happens we are EXTREMLY out of memory or have a
			   serious problem while printStackTrace.  But may be
			   another exception, so print it. */

			ne = exceptions_get_exception();

			if (ne != NULL) {
				fprintf(stderr, "Exception while printStackTrace(): ");

				/* Print the current exception. */

				exceptions_print_exception(ne);
				stacktrace_print_exception(ne);

				/* Now print the original exception. */

				fprintf(stderr, "Original exception was: ");
				exceptions_print_exception(e);
				stacktrace_print_exception(e);
			}
		}
		else {
			fprintf(stderr, ". Uncaught exception of type ");
#if !defined(NDEBUG)
			/* FIXME This prints to stdout. */
			class_print(c);
#else
			fprintf(stderr, "UNKNOWN");
#endif
			fprintf(stderr, ".");
		}

		fflush(stderr);
	}
}

static inline Utf8String get_classname_for_exception(classinfo *c) {
#if defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
	return Utf8String::from_utf8_slash_to_dot(c->name.begin(), c->name.size());
#else
	return c->name;
#endif
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
