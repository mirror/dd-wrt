/* src/vm/jit/builtintable.inc - tables of builtin functions

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

#include "arch.hpp"

#include "threads/lock.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/jit/jit.hpp"


/* internal and not automatically replaced functions **************************/

static builtintable_entry builtintable_internal[] = {

#if defined(__ALPHA__) || defined(DISABLE_GC)
	{
		ICMD_F2L,
		0,
		BUILTIN_f2l,
		NULL,
		NULL,
		"f2l",
		"(F)J",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_D2L,
		0,
		BUILTIN_d2l,
		NULL,
		NULL,
		"d2l",
		"(D)J",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_F2I,
		0,
		BUILTIN_f2i,
		NULL,
		NULL,
		"f2i",
		"(F)I",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_D2I,
		0,
		BUILTIN_d2i,
		NULL,
		NULL,
		"d2i",
		"(D)I",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif

	{
		ICMD_LCMP,
		0,
		BUILTIN_lcmp,
		NULL,
		NULL,
		"lcmp",
		"(JJ)I",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},

#if !SUPPORT_DIVISION || defined(DISABLE_GC)
	{
		ICMD_IDIV,
		0,
		BUILTIN_idiv,
		NULL,
		NULL,
		"idiv",
		"(II)I",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_IREM,
		0,
		BUILTIN_irem,
		NULL,
		NULL,
		"irem",
		"(II)I",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif

#if !(SUPPORT_DIVISION && SUPPORT_LONG_DIV) || defined(DISABLE_GC)
	{
		ICMD_LDIV,
		0,
		BUILTIN_ldiv,
		NULL,
		NULL,
		"ldiv",
		"(JJ)J",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_LREM,
		0,
		BUILTIN_lrem,
		NULL,
		NULL,
		"lrem",
		"(JJ)J",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif

	{
		ICMD_FREM,
		0,
		BUILTIN_frem,
		NULL,
		NULL,
		"frem",
		"(FF)F",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_DREM,
		0,
		BUILTIN_drem,
		NULL,
		NULL,
		"drem",
		"(DD)D",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},


	/* internal functions *****************************************************/

	{
		ICMD_NEW,
		BUILTINTABLE_FLAG_STUB,
		BUILTIN_new,
		NULL,
		NULL,
		"new",
		"(Ljava/lang/Class;)Ljava/lang/Object;",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},

#if defined(ENABLE_TLH)
	{
		ICMD_NEW,
		BUILTINTABLE_FLAG_STUB,
		BUILTIN_tlh_new,
		NULL,
		NULL,
		"tlh_new",
		"(Ljava/lang/Class;)Ljava/lang/Object;",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif

#if defined(ENABLE_ESCAPE_REASON)
	{
		ICMD_NEW,
		BUILTINTABLE_FLAG_STUB,
		BUILTIN_escape_reason_new,
		NULL,
		NULL,
		"escape_reason_new",
		"(Ljava/lang/Class;)Ljava/lang/Object;",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif

	{
		ICMD_NEW,
		0,
		BUILTIN_FAST_new,
		NULL,
		NULL,
		"fast-new",
		"(Ljava/lang/Class;)Ljava/lang/Object;",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_ANEWARRAY,
		BUILTINTABLE_FLAG_STUB,
		BUILTIN_newarray,
		NULL,
		NULL,
		"newarray",
		"(ILjava/lang/Class;)[Ljava/lang/Object;",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_NEWARRAY,
		BUILTINTABLE_FLAG_STUB,
		BUILTIN_newarray_boolean,
		NULL,
		NULL,
		"newarray_boolean",
		"(I)[Z",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_NEWARRAY,
		BUILTINTABLE_FLAG_STUB,
		BUILTIN_newarray_byte,
		NULL,
		NULL,
		"newarray_byte",
		"(I)[B",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_NEWARRAY,
		BUILTINTABLE_FLAG_STUB,
		BUILTIN_newarray_char,
		NULL,
		NULL,
		"newarray_char",
		"(I)[C",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_NEWARRAY,
		BUILTINTABLE_FLAG_STUB,
		BUILTIN_newarray_short,
		NULL,
		NULL,
		"newarray_short",
		"(I)[S",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_NEWARRAY,
		BUILTINTABLE_FLAG_STUB,
		BUILTIN_newarray_int,
		NULL,
		NULL,
		"newarray_int",
		"(I)[I",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_NEWARRAY,
		BUILTINTABLE_FLAG_STUB,
		BUILTIN_newarray_long,
		NULL,
		NULL,
		"newarray_long",
		"(I)[J",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_NEWARRAY,
		BUILTINTABLE_FLAG_STUB,
		BUILTIN_newarray_float,
		NULL,
		NULL,
		"newarray_float",
		"(I)[F",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_NEWARRAY,
		BUILTINTABLE_FLAG_STUB,
		BUILTIN_newarray_double,
		NULL,
		NULL,
		"newarray_double",
		"(I)[D",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_MULTIANEWARRAY,
		BUILTINTABLE_FLAG_STUB,
		BUILTIN_multianewarray,
		NULL,
		NULL,
		"multianewarray",
		/* XXX it should be: "(ILjava/lang/Class;[I)[Ljava/lang/Object;", */
		"(ILjava/lang/Class;I)[Ljava/lang/Object;",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_CHECKCAST,
		0,
		BUILTIN_arraycheckcast,
		NULL,
		NULL,
		"arraycheckcast",
		"([Ljava/lang/Object;[Ljava/lang/Object;)[Ljava/lang/Object;",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_INSTANCEOF,
		0,
		BUILTIN_arrayinstanceof,
		NULL,
		NULL,
		"arrayinstanceof",
		"([Ljava/lang/Object;[Ljava/lang/Object;)I",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_AASTORE,
		0,
		BUILTIN_FAST_canstore,
		NULL,
		NULL,
		"fast-canstore",
		"([Ljava/lang/Object;Ljava/lang/Object;)I",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},

	{
		ICMD_MONITORENTER,
		BUILTINTABLE_FLAG_STUB,
		LOCK_monitor_enter,
		NULL,
		NULL,
		"monitorenter",
		"(Ljava/lang/Object;)V",
		NULL,
		NULL,
		NULL,
		NULL,
		EMIT_FASTPATH_monitor_enter
	},
	{
		ICMD_MONITOREXIT,
		BUILTINTABLE_FLAG_STUB,
		LOCK_monitor_exit,
		NULL,
		NULL,
		"monitorexit",
		"(Ljava/lang/Object;)V",
		NULL,
		NULL,
		NULL,
		NULL,
		EMIT_FASTPATH_monitor_exit
	},

	/* stop entry */

	{
		(ICMD) 255,
		0,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
};


/* automatically replaced functions *******************************************/

static builtintable_entry builtintable_automatic[] = {

#if !SUPPORT_LONG_CMP
	{
		ICMD_LCMP,
		0,
		BUILTIN_lcmp,
		NULL,
		NULL,
		"lcmp",
		"(JJ)I",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif

#if !SUPPORT_LONG_SHIFT
	{
		ICMD_LSHL,
		0,
		BUILTIN_lshl,
		NULL,
		NULL,
		"lshl",
		"(JI)J",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_LSHR,
		0,
		BUILTIN_lshr,
		NULL,
		NULL,
		"lshr",
		"(JI)J",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_LUSHR,
		0,
		BUILTIN_lushr,
		NULL,
		NULL,
		"lushr",
		"(JI)J",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif

#if !SUPPORT_LONG_ADD
	{
		ICMD_LADD,
		0,
		BUILTIN_ladd,
		NULL,
		NULL,
		"ladd",
		"(JJ)J",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_LSUB,
		0,
		BUILTIN_lsub,
		NULL,
		NULL,
		"lsub",
		"(JJ)J",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_LNEG,
		0,
		BUILTIN_lneg,
		NULL,
		NULL,
		"lneg",
		"(J)J",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif

#if !SUPPORT_LONG_MUL
	{
		ICMD_LMUL,
		0,
		BUILTIN_lmul,
		NULL,
		NULL,
		"lmul",
		"(JJ)J",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif

#if !(SUPPORT_FLOAT && SUPPORT_I2F)
	{
		ICMD_I2F,
		0,
		BUILTIN_i2f,
		NULL,
		NULL,
		"i2f",
		"(I)F",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif

#if !(SUPPORT_DOUBLE && SUPPORT_I2D)
	{
		ICMD_I2D,
		0,
		BUILTIN_i2d,
		NULL,
		NULL,
		"i2d",
		"(I)D",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif

#if !(SUPPORT_FLOAT && SUPPORT_L2F)
	{
		ICMD_L2F,
		0,
		BUILTIN_l2f,
		NULL,
		NULL,
		"l2f",
		"(J)F",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif

#if !(SUPPORT_DOUBLE && SUPPORT_L2D)
	{
		ICMD_L2D,
		0,
		BUILTIN_l2d,
		NULL,
		NULL,
		"l2d",
		"(J)D",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif

#if !(SUPPORT_FLOAT && SUPPORT_F2I)
	{
		ICMD_F2I,
		0,
		BUILTIN_f2i,
		NULL,
		NULL,
		"f2i",
		"(F)I",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif

#if !(SUPPORT_FLOAT && SUPPORT_F2L)
	{
		ICMD_F2L,
		0,
		BUILTIN_f2l,
		NULL,
		NULL,
		"f2l",
		"(F)J",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif

#if !(SUPPORT_DOUBLE && SUPPORT_D2I)
	{
		ICMD_D2I,
		0,
		BUILTIN_d2i,
		NULL,
		NULL,
		"d2i",
		"(D)I",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif

#if !(SUPPORT_DOUBLE && SUPPORT_D2L)
	{
		ICMD_D2L,
		0,
		BUILTIN_d2l,
		NULL,
		NULL,
		"d2l",
		"(D)J",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif


	/* float functions ********************************************************/

#if !SUPPORT_FLOAT
	{
		ICMD_FADD,
		0,
		BUILTIN_fadd,
		NULL,
		NULL,
		"fadd",
		"(FF)F",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_FSUB,
		0,
		BUILTIN_fsub,
		NULL,
		NULL,
		"fsub",
		"(FF)F",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_FMUL,
		0,
		BUILTIN_fmul,
		NULL,
		NULL,
		"fmul",
		"(FF)F",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_FDIV,
		0,
		BUILTIN_fdiv,
		NULL,
		NULL,
		"fdiv",
		"(FF)F",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_FNEG,
		0,
		BUILTIN_fneg,
		NULL,
		NULL,
		"fneg",
		"(F)F",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif /* !SUPPORT_FLOAT */

#if !SUPPORT_FLOAT || !SUPPORT_FLOAT_CMP
	{
		ICMD_FCMPL,
		0,
		BUILTIN_fcmpl,
		NULL,
		NULL,
		"fcmpl",
		"(FF)I",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_FCMPG,
		0,
		BUILTIN_fcmpg,
		NULL,
		NULL,
		"fcmpg",
		"(FF)I",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif /* !SUPPORT_FLOAT || !SUPPORT_FLOAT_CMP */


	/* double functions *******************************************************/

#if !SUPPORT_DOUBLE
	{
		ICMD_DADD,
		0,
		BUILTIN_dadd,
		NULL,
		NULL,
		"dadd",
		"(DD)D",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_DSUB,
		0,
		BUILTIN_dsub,
		NULL,
		NULL,
		"dsub",
		"(DD)D",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_DMUL,
		0,
		BUILTIN_dmul,
		NULL,
		NULL,
		"dmul",
		"(DD)D",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_DDIV,
		0,
		BUILTIN_ddiv,
		NULL,
		NULL,
		"ddiv",
		"(DD)D",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_DNEG,
		0,
		BUILTIN_dneg,
		NULL,
		NULL,
		"dneg",
		"(D)D",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif /* !SUPPORT_DOUBLE */

#if !SUPPORT_DOUBLE || !SUPPORT_DOUBLE_CMP
	{
		ICMD_DCMPL,
		0,
		BUILTIN_dcmpl,
		NULL,
		NULL,
		"dcmpl",
		"(DD)I",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_DCMPG,
		0,
		BUILTIN_dcmpg,
		NULL,
		NULL,
		"dcmpg",
		"(DD)I",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif /* !SUPPORT_DOUBLE || !SUPPORT_DOUBLE_CMP */


	/* float/double functions *************************************************/

#if !(SUPPORT_FLOAT && SUPPORT_DOUBLE)
	{
		ICMD_F2D,
		0,
		BUILTIN_f2d,
		NULL,
		NULL,
		"f2d",
		"(F)D",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ICMD_D2F,
		0,
		BUILTIN_d2f,
		NULL,
		NULL,
		"d2f",
		"(D)F",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif /* !(SUPPORT_FLOAT && SUPPORT_DOUBLE) */


	/* prevent a compiler warning if everything is supported (e.g. i386) */

	{
		(ICMD) 255,
		0,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
};


/* automatically replaced functions *******************************************/

static builtintable_entry builtintable_function[] = {
#if defined(ENABLE_JIT)

	/* java.lang.VMSystem.arraycopy(Ljava/lang/Object;ILjava/lang/Object;II)V STATIC NATIVE */

	{
		ICMD_BUILTIN,
		BUILTINTABLE_FLAG_STUB | BUILTINTABLE_FLAG_EXCEPTION,
		BUILTIN_arraycopy,
		NULL,
		"java/lang/VMSystem",
		"arraycopy",
		"(Ljava/lang/Object;ILjava/lang/Object;II)V",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},

	/* java.lang.System.arraycopy(Ljava/lang/Object;ILjava/lang/Object;II)V PUBLIC STATIC */

#if 0
	/* We disable this one until we have a fix for the stacktrace
	   issue. */

	{
		ICMD_BUILTIN,
		BUILTINTABLE_FLAG_STUB | BUILTINTABLE_FLAG_EXCEPTION,
		BUILTIN_arraycopy,
		NULL,
		"java/lang/System",
		"arraycopy",
		"(Ljava/lang/Object;ILjava/lang/Object;II)V",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
#endif

	/* java.lang.VMSystem.currentTimeMillis()J PUBLIC STATIC */

	{
		ICMD_BUILTIN,
		0,
		BUILTIN_currenttimemillis,
		NULL,
		"java/lang/VMSystem",
		"currentTimeMillis",
		"()J",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},

	/* java.lang.System.currentTimeMillis()J PUBLIC STATIC */

	{
		ICMD_BUILTIN,
		0,
		BUILTIN_currenttimemillis,
		NULL,
		"java/lang/System",
		"currentTimeMillis",
		"()J",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},

#endif /* defined(ENABLE_JIT) */

	/* stop entry */

	{
		(ICMD) 255,
		0,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	},
};


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
