/* src/vm/jit/builtin.hpp - prototypes of builtin functions

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


#ifndef _BUILTIN_HPP
#define _BUILTIN_HPP

/* forward typedefs ***********************************************************/

#include <sys/types.h>                  // for int32_t
#include "arch.hpp"                     // for USES_NEW_SUBTYPE
#include "threads/lock.hpp"             // for lock_monitor_enter, etc
#include "vm/global.hpp"                // for functionptr, java_handle_t, etc
#include "vm/jit/ir/icmd.hpp"           // for ICMD
#include "vm/types.hpp"                 // for s8, s4, u1, u4
#include "vm/utf8.hpp"                  // for Utf8String

struct builtintable_entry;
struct classinfo;
struct methoddesc;
struct methodinfo;
struct vftbl_t;

/* define infinity for floating point numbers */

#define FLT_NAN     0x7fc00000
#define FLT_POSINF  0x7f800000
#define FLT_NEGINF  0xff800000

/* define infinity for double floating point numbers */

#define DBL_NAN     0x7ff8000000000000LL
#define DBL_POSINF  0x7ff0000000000000LL
#define DBL_NEGINF  0xfff0000000000000LL


/* builtin functions table ****************************************************/

struct builtintable_entry {
	ICMD         opcode;                /* opcode which is replaced           */
	u4           flags;                 /* e.g. check for exception           */
	functionptr  fp;                    /* function pointer of builtin        */
	u1          *stub;                  /* pointer to builtin stub code       */
	const char*  cclassname;            /* char name of the class             */
	const char*  cname;                 /* char name of the function          */
	const char*  cdescriptor;           /* char name of the descriptor        */
	Utf8String   classname;             /* class of the function              */
	Utf8String   name;                  /* name of the function               */
	Utf8String   descriptor;            /* descriptor of the function         */
	methoddesc  *md;
	functionptr  emit_fastpath;         /* emitter for fast-path code         */
};


/* builtin table flag defines *************************************************/

#define BUILTINTABLE_FLAG_STUB         0x0001 /* builtin needs a stub         */
#define BUILTINTABLE_FLAG_EXCEPTION    0x0002 /* check for excepion on return */


/* function prototypes ********************************************************/

extern "C" {

bool builtin_init(void);

builtintable_entry *builtintable_get_internal(functionptr fp);
builtintable_entry *builtintable_get_automatic(s4 opcode);

bool builtintable_replace_function(void *iptr);


/**********************************************************************/
/* BUILTIN FUNCTIONS                                                  */
/**********************************************************************/

/* NOTE: Builtin functions which are used in the BUILTIN* opcodes must
 * have a BUILTIN_... macro defined as seen below. In code dealing
 * with the BUILTIN* opcodes the functions may only be addressed by
 * these macros, never by their actual name! (This helps to make this
 * code more portable.)
 *
 * C and assembler code which does not deal with the BUILTIN* opcodes,
 * can use the builtin functions normally (like all other functions).
 *
 * IMPORTANT:
 * For each builtin function which is used in a BUILTIN* opcode there
 * must be an entry in the builtin_desc table in jit/jit.c.
 *
 * Below each prototype is either the BUILTIN_ macro definition or a
 * comment specifiying that this function is not used in BUILTIN*
 * opcodes.
 *
 * (The BUILTIN* opcodes are ICMD_BUILTIN1, ICMD_BUILTIN2 and
 * ICMD_BUILTIN3.)
 */

#if USES_NEW_SUBTYPE
bool fast_subtype_check(vftbl_t*, vftbl_t*);
#endif

/* From lock.hpp: bool lock_monitor_enter(java_handle_t *); */
#define LOCK_monitor_enter          (functionptr) lock_monitor_enter
#if defined(__X86_64__)
# define EMIT_FASTPATH_monitor_enter (functionptr) emit_fastpath_monitor_enter
#else
# define EMIT_FASTPATH_monitor_enter (functionptr) NULL
#endif

/* From lock.hpp: bool lock_monitor_exit(java_handle_t *); */
#define LOCK_monitor_exit          (functionptr) lock_monitor_exit
#if defined(__X86_64__)
# define EMIT_FASTPATH_monitor_exit (functionptr) emit_fastpath_monitor_exit
#else
# define EMIT_FASTPATH_monitor_exit (functionptr) NULL
#endif

bool builtin_instanceof(java_handle_t *obj, classinfo *c);
/* NOT AN OP */
bool builtin_checkcast(java_handle_t *obj, classinfo *c);
/* NOT AN OP */
bool builtin_arrayinstanceof(java_handle_t *h, classinfo *targetclass);
/* NOT AN OP */
bool builtin_fast_arrayinstanceof(java_object_t *o, classinfo *targetclass);
#define BUILTIN_arrayinstanceof (functionptr) builtin_fast_arrayinstanceof
bool builtin_fast_arraycheckcast(java_object_t *o, classinfo *targetclass);
#define BUILTIN_arraycheckcast (functionptr) builtin_fast_arraycheckcast

bool builtin_canstore(java_handle_objectarray_t *oa, java_handle_t *o);
/* NOT AN OP */
bool builtin_fast_canstore(java_objectarray_t *oa, java_object_t *o);
#define BUILTIN_FAST_canstore (functionptr) builtin_fast_canstore

void *builtin_throw_exception(java_object_t *exception);
/* NOT AN OP */
java_object_t *builtin_retrieve_exception(void);
/* NOT AN OP */

java_handle_t *builtin_new(classinfo *c);
/* NOT AN OP */
java_handle_t *builtin_java_new(java_handle_t *c);
#define BUILTIN_new (functionptr) builtin_java_new

#if defined(ENABLE_TLH)
#define BUILTIN_tlh_new (functionptr) builtin_tlh_new
java_handle_t *builtin_tlh_new(classinfo *c);
#endif

#if defined(ENABLE_ESCAPE_REASON)
#define BUILTIN_escape_reason_new (functionptr)builtin_escape_reason_new
java_handle_t *builtin_escape_reason_new(classinfo *c);
#endif

java_object_t *builtin_fast_new(classinfo *c);
#define BUILTIN_FAST_new (functionptr) builtin_fast_new

java_handle_array_t *builtin_java_newarray(int32_t size, java_handle_t *arrayclass);
#define BUILTIN_newarray (functionptr) builtin_java_newarray

java_handle_booleanarray_t *builtin_newarray_boolean(int32_t size);
#define BUILTIN_newarray_boolean (functionptr) builtin_newarray_boolean
java_handle_chararray_t *builtin_newarray_char(int32_t size);
#define BUILTIN_newarray_char (functionptr) builtin_newarray_char
java_handle_floatarray_t *builtin_newarray_float(int32_t size);
#define BUILTIN_newarray_float (functionptr) builtin_newarray_float
java_handle_doublearray_t *builtin_newarray_double(int32_t size);
#define BUILTIN_newarray_double (functionptr) builtin_newarray_double
java_handle_bytearray_t *builtin_newarray_byte(int32_t size);
#define BUILTIN_newarray_byte (functionptr) builtin_newarray_byte
java_handle_shortarray_t *builtin_newarray_short(int32_t size);
#define BUILTIN_newarray_short (functionptr) builtin_newarray_short
java_handle_intarray_t *builtin_newarray_int(int32_t size);
#define BUILTIN_newarray_int (functionptr) builtin_newarray_int
java_handle_longarray_t *builtin_newarray_long(int32_t size);
#define BUILTIN_newarray_long (functionptr) builtin_newarray_long

java_handle_objectarray_t *builtin_multianewarray(int n,
												  java_handle_t *arrayclass,
												  long *dims);
#define BUILTIN_multianewarray (functionptr) builtin_multianewarray

#if defined(TRACE_ARGS_NUM)
void builtin_verbosecall_enter(s8 a0, s8 a1,
# if TRACE_ARGS_NUM >= 4
							   s8 a2, s8 a3,
# endif
# if TRACE_ARGS_NUM >= 6
							   s8 a4, s8 a5,
# endif
# if TRACE_ARGS_NUM == 8
							   s8 a6, s8 a7,
# endif
							   methodinfo *m);
/* NOT AN OP */
#endif /* defined(TRACE_ARGS_NUM) */

void builtin_verbosecall_exit(s8 l, double d, float f, methodinfo *m);
/* NOT AN OP */

s4 builtin_idiv(s4 a, s4 b);
#define BUILTIN_idiv (functionptr) builtin_idiv
s4 builtin_irem(s4 a, s4 b);
#define BUILTIN_irem (functionptr) builtin_irem

s8 builtin_ladd(s8 a, s8 b);
#define BUILTIN_ladd (functionptr) builtin_ladd
s8 builtin_lsub(s8 a, s8 b);
#define BUILTIN_lsub (functionptr) builtin_lsub
s8 builtin_lmul(s8 a, s8 b);
#define BUILTIN_lmul (functionptr) builtin_lmul

s8 builtin_ldiv(s8 a, s8 b);
#define BUILTIN_ldiv (functionptr) builtin_ldiv
s8 builtin_lrem(s8 a, s8 b);
#define BUILTIN_lrem (functionptr) builtin_lrem

s8 builtin_lshl(s8 a, s4 b);
#define BUILTIN_lshl (functionptr) builtin_lshl
s8 builtin_lshr(s8 a, s4 b);
#define BUILTIN_lshr (functionptr) builtin_lshr
s8 builtin_lushr(s8 a, s4 b);
#define BUILTIN_lushr (functionptr) builtin_lushr
s8 builtin_land(s8 a, s8 b);
#define BUILTIN_land (functionptr) builtin_land
s8 builtin_lor(s8 a, s8 b);
#define BUILTIN_lor (functionptr) builtin_lor
s8 builtin_lxor(s8 a, s8 b);
#define BUILTIN_lxor (functionptr) builtin_lxor
s8 builtin_lneg(s8 a);
#define BUILTIN_lneg (functionptr) builtin_lneg
s4 builtin_lcmp(s8 a, s8 b);
#define BUILTIN_lcmp (functionptr) builtin_lcmp

float builtin_fadd(float a, float b);
#define BUILTIN_fadd (functionptr) builtin_fadd
float builtin_fsub(float a, float b);
#define BUILTIN_fsub (functionptr) builtin_fsub
float builtin_fmul(float a, float b);
#define BUILTIN_fmul (functionptr) builtin_fmul
float builtin_fdiv(float a, float b);
#define BUILTIN_fdiv (functionptr) builtin_fdiv
float builtin_fneg(float a);         
#define BUILTIN_fneg (functionptr) builtin_fneg
s4 builtin_fcmpl(float a, float b);  
#define BUILTIN_fcmpl (functionptr) builtin_fcmpl
s4 builtin_fcmpg(float a, float b);  
#define BUILTIN_fcmpg (functionptr) builtin_fcmpg
float builtin_frem(float a, float b);
#define BUILTIN_frem (functionptr) builtin_frem

double builtin_dadd(double a, double b);
#define BUILTIN_dadd (functionptr) builtin_dadd
double builtin_dsub(double a, double b);
#define BUILTIN_dsub (functionptr) builtin_dsub
double builtin_dmul(double a, double b);
#define BUILTIN_dmul (functionptr) builtin_dmul
double builtin_ddiv(double a, double b);
#define BUILTIN_ddiv (functionptr) builtin_ddiv
double builtin_dneg(double a);          
#define BUILTIN_dneg (functionptr) builtin_dneg
s4 builtin_dcmpl(double a, double b);   
#define BUILTIN_dcmpl (functionptr) builtin_dcmpl
s4 builtin_dcmpg(double a, double b);   
#define BUILTIN_dcmpg (functionptr) builtin_dcmpg
double builtin_drem(double a, double b);
#define BUILTIN_drem (functionptr) builtin_drem

float    builtin_i2f(s4 i);
#define BUILTIN_i2f (functionptr) builtin_i2f
double   builtin_i2d(s4 i);
#define BUILTIN_i2d (functionptr) builtin_i2d
float    builtin_l2f(s8 l);
#define BUILTIN_l2f (functionptr) builtin_l2f
double   builtin_l2d(s8 l);
#define BUILTIN_l2d (functionptr) builtin_l2d

s4       builtin_f2i(float a);
#define BUILTIN_f2i (functionptr) builtin_f2i
s4       asm_builtin_f2i(float a);
/* NOT AN OP */
s8       builtin_f2l(float a);
#define BUILTIN_f2l (functionptr) builtin_f2l
s8       asm_builtin_f2l(float a);
/* NOT AN OP */

double   builtin_f2d(float a);
#define BUILTIN_f2d (functionptr) builtin_f2d

s4       builtin_d2i(double a);
#define BUILTIN_d2i (functionptr) builtin_d2i
s4       asm_builtin_d2i(double a);
/* NOT AN OP */
s8       builtin_d2l(double a);
#define BUILTIN_d2l (functionptr) builtin_d2l
s8       asm_builtin_d2l(double a);
/* NOT AN OP */

float    builtin_d2f(double a);
#define BUILTIN_d2f (functionptr) builtin_d2f

java_handle_t *builtin_clone(void *env, java_handle_t *o);
#define BUILTIN_clone (functionptr) builtin_clone

void builtin_arraycopy(java_handle_t *src, s4 srcStart,
					   java_handle_t *dest, s4 destStart, s4 len);
#define BUILTIN_arraycopy (functionptr) builtin_arraycopy

s8 builtin_nanotime(void);
s8 builtin_currenttimemillis(void);
#define BUILTIN_currenttimemillis (functionptr) builtin_currenttimemillis

#if defined(ENABLE_CYCLES_STATS)
void builtin_print_cycles_stats(FILE *file);
#endif

} // extern "C"

#endif // _BUILTIN_HPP


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
