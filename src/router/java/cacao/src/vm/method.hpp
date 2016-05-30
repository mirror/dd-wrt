/* src/vm/method.hpp - method functions header

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


#ifndef METHOD_HPP_
#define METHOD_HPP_ 1

#include "config.h"                     // for ENABLE_JAVASE, etc

#include <stdint.h>                     // for uint16_t, int32_t

#include "vm/global.hpp"                // for java_handle_bytearray_t, etc
#include "vm/references.hpp"            // for classref_or_classinfo
#include "vm/types.hpp"                 // for s4, u2, u1
#include "vm/utf8.hpp"                  // for Utf8String

class BreakpointTable;
class Mutex;
struct builtintable_entry;
struct classbuffer;
struct classinfo;
struct codeinfo;
struct lineinfo;
struct localvarinfo;
struct method_assumption;
struct method_worklist;
struct methoddesc;
struct methodinfo;
struct raw_exception_entry;
struct stack_map_t;
struct vftbl_t;

namespace cacao {
	struct ClassBuffer;
	struct DescriptorPool;
}


#if defined(ENABLE_REPLACEMENT)
// Initial value for the hit countdown field of each method.
#define METHOD_INITIAL_HIT_COUNTDOWN  1000
#endif


/* methodinfo *****************************************************************/

struct methodinfo {                 /* method structure                       */
	Mutex        *mutex;            /* we need this in jit's locking          */
	s4            flags;            /* ACC flags                              */
	Utf8String    name;             /* name of method                         */
	Utf8String    descriptor;       /* JavaVM descriptor string of method     */
#if defined(ENABLE_JAVASE)
	Utf8String    signature;        /* Signature attribute                    */
	stack_map_t  *stack_map;        /* StackMapTable attribute                */
#endif

	methoddesc   *parseddesc;       /* parsed descriptor                      */

	classinfo    *clazz;            /* class, the method belongs to           */
	s4            vftblindex;       /* index of method in virtual function    */
	                                /* table (if it is a virtual method)      */
	s4            maxstack;         /* maximum stack depth of method          */
	s4            maxlocals;        /* maximum number of local variables      */
	s4            jcodelength;      /* length of JavaVM code                  */
	u1           *jcode;            /* pointer to JavaVM code                 */

	s4            rawexceptiontablelength;  /* exceptiontable length          */
	raw_exception_entry *rawexceptiontable; /* the exceptiontable             */

	u2            thrownexceptionscount; /* number of exceptions attribute    */
	classref_or_classinfo *thrownexceptions; /* except. a method may throw    */

	u2            linenumbercount;  /* number of linenumber attributes        */
	lineinfo     *linenumbers;      /* array of lineinfo items                */

#if defined(ENABLE_JAVASE) && defined(ENABLE_JVMTI)
	uint16_t      localvarcount;    /* number of local variable attributes    */
	localvarinfo* localvars;        /* array of localvarinfo items            */
#endif

	u1           *stubroutine;      /* stub for compiling or calling natives  */
	codeinfo     *code;             /* current code of this method            */

#if defined(ENABLE_LSRA)
	s4            maxlifetimes;     /* helper for lsra                        */
#endif

	methodinfo   *overwrites;       /* method that is directly overwritten    */
	method_assumption *assumptions; /* list of assumptions about this method  */

	BreakpointTable* breakpoints;   /* breakpoints in this method             */

#if defined(ENABLE_REPLACEMENT)
	s4            hitcountdown;     /* decreased for each hit                 */
#endif

#if defined(ENABLE_DEBUG_FILTER)
	u1            filtermatches;    /* flags indicating which filters the method matches */
#endif

#if defined(ENABLE_ESCAPE)
	u1           *paramescape;
#endif
};

/* method_assumption ***********************************************************

   This struct is used for registering assumptions about methods.

*******************************************************************************/

struct method_assumption {
	method_assumption *next;
	methodinfo        *context;
};


/* method_worklist *************************************************************

   List node used for method worklists.

*******************************************************************************/

struct method_worklist {
	method_worklist *next;
	methodinfo      *m;
};


/* raw_exception_entry ********************************************************/

/* exception table entry read by the loader */

struct raw_exception_entry {    /* exceptiontable entry in a method           */
	classref_or_classinfo catchtype; /* catchtype of exc. (0 == catchall)     */
	u2              startpc;    /* start pc of guarded area (inclusive)       */
	u2              endpc;      /* end pc of guarded area (exklusive)         */
	u2              handlerpc;  /* pc of exception handler                    */
};


/* lineinfo *******************************************************************/

struct lineinfo {
	u2 start_pc;
	u2 line_number;
};


/* localvarinfo ***************************************************************/

struct localvarinfo {
	uint16_t   start_pc;
	uint16_t   length;
	Utf8String name;
	Utf8String descriptor;
	uint16_t   index;
};


/* global variables ***********************************************************/

extern methodinfo *method_java_lang_reflect_Method_invoke;


/* inline functions ***********************************************************/

inline static bool method_is_builtin(methodinfo* m)
{
	return m->flags & ACC_METHOD_BUILTIN;
}


/* function prototypes ********************************************************/

void method_init(void);

bool method_load(cacao::ClassBuffer& cb, methodinfo *m, cacao::DescriptorPool& descpool);
void method_free(methodinfo *m);
bool method_canoverwrite(methodinfo *m, methodinfo *old);

methodinfo *method_new_builtin(builtintable_entry *bte);

methodinfo *method_vftbl_lookup(vftbl_t *vftbl, methodinfo* m);

int32_t                    method_get_parametercount(methodinfo *m);
java_handle_objectarray_t *method_get_parametertypearray(methodinfo *m);
java_handle_objectarray_t *method_get_exceptionarray(methodinfo *m);
classinfo                 *method_returntype_get(methodinfo *m);

void method_add_assumption_monomorphic(methodinfo *m, methodinfo *caller);
void method_break_assumption_monomorphic(methodinfo *m, method_worklist **wl);

s4   method_count_implementations(methodinfo *m, classinfo *c, methodinfo **found);

java_handle_bytearray_t *method_get_annotations(methodinfo *m);
java_handle_bytearray_t *method_get_parameterannotations(methodinfo *m);
java_handle_bytearray_t *method_get_annotationdefault(methodinfo *m);

#if !defined(NDEBUG)
void method_printflags(methodinfo *m);
void method_print(methodinfo *m);
void method_println(methodinfo *m);
void method_methodref_print(constant_FMIref *mr);
void method_methodref_println(constant_FMIref *mr);
#endif


namespace cacao {

// forward declaration
class OStream;

cacao::OStream& operator<<(cacao::OStream &OS, const struct methodinfo &m);

} // end namespace cacao

#endif // METHOD_HPP_


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
