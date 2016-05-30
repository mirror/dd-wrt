/* src/vm/jit/intrp/engine.c - #included by engine1.c and engine2.c

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


/* #define VM_DEBUG */

#include "config.h"

#include <assert.h>

#include "vm/types.hpp"

#include "arch.hpp"

#include "vm/jit/intrp/intrp.h"

#include "md-abi.hpp"                           /* required for TRACE_ARGS_NUM */

#include "mm/memory.hpp"

#include "threads/thread.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/loader.hpp"
#include "vm/options.hpp"

#include "vm/jit/methodheader.hpp"
#include "vm/jit/patcher.hpp"
#include "vm/jit/stacktrace.hpp"


#if defined(ENABLE_THREADS)
# include "threads/atomic.hpp"
#endif

#if !defined(STORE_ORDER_BARRIER) && !defined(ENABLE_THREADS)
#define STORE_ORDER_BARRIER() /* nothing */
#endif


/* threading macros */
#define GCC_PR15242_WORKAROUND
#ifdef GCC_PR15242_WORKAROUND
#  define NEXT_P1_5
#  define DO_GOTO goto before_goto
#else
#  define NEXT_P1_5
#  define DO_GOTO goto *ip[-1]
#endif

#  define NEXT_P0
#  define IP		(ip)
#  define SET_IP(p)	do {ip=(p); NEXT_P0;} while (0)
#  define NEXT_INST	(*IP)
#  define INC_IP(const_inc)	do { ip+=(const_inc);} while (0)
#  define DEF_CA
#  define NEXT_P1	(ip++)
#  define NEXT_P2   do {NEXT_P1_5; DO_GOTO;} while(0)

#define NEXT ({DEF_CA NEXT_P1; NEXT_P2;})
#define IPTOS NEXT_INST

#if defined(__POWERPC__) || defined(__POWERPC64__) || defined(__SPARC__)
# define USE_spTOS
#endif

#if defined(USE_spTOS)
#define IF_spTOS(x) x
#else
#define IF_spTOS(x)
#define spTOS (sp[0])
#endif

#if defined(__I386__)
/* works with gcc-2.95.4 20011002 (Debian prerelease) without static supers */
#define SPREG /* __asm__("%esi") */
#define TOSREG /* __asm__("%ecx") */
#elif defined(__X86_64__)
/* works with gcc-4.0.2 (Debian 4.0.2-2) */
#define SPREG /* __asm__("%r15") */
#define TOSREG
#else
#define SPREG
#define TOSREG
#endif


/* conversion on fetch */

#ifdef VM_PROFILING
#define SUPER_END  vm_count_block(IP)
#else
#define SUPER_END
#define vm_uncount_block(_ip)	/* nothing */
#endif


#define THROW0       goto throw
#if 1
#define THROW(_ball) do { \
                       __asm__(""); /* work around gcc PR 25285 */ \
                       goto *throw_##_ball; \
                     } while (0)
#else
#define THROW(_ball) do { \
                       goto throw_##_ball##1; \
                     } while (0)
#endif

#define THROWCODE(_ball) \
    throw_##_ball##1: \
        global_sp = sp; \
        *exceptionptr = (stacktrace_inline_##_ball(NULL, (u1 *) fp, (u1 *) IP, (u1 *) IP)); \
        CLEAR_global_sp; \
        THROW0;

#define CHECK_NULL_PTR(ptr) \
    { \
        if ((ptr) == NULL) { \
            THROW(nullpointerexception); \
	    } \
	}

#define THROW_CLASSCASTEXCEPTION(o) \
    { \
		classcastexception_object = o; \
        THROW(classcastexception); \
	}

#define CHECK_OUT_OF_BOUNDS(_array, _idx)              \
        {                                            \
          if (length_array(_array) <= (u4) (_idx)) { \
                arrayindexoutofbounds_index = (_idx); \
				THROW(arrayindexoutofboundsexception); \
          } \
	}

#define CHECK_ZERO_DIVISOR(_divisor) \
  { if (_divisor == 0) \
      THROW(arithmeticexception); \
  } 

#if 0
/* !! alignment bug */
#define access_local_long(_offset) \
        ( *(s8 *)(((u1 *)fp) + (_offset)) )
#endif

#define length_array(array)                          \
        ( ((java_arrayheader*)(array))->size )

#define access_array_int(array, index)               \
        ((((java_intarray*)(array))->data)[index])

#define access_array_long(array, index)               \
        ((((java_longarray*)(array))->data)[index])

#define access_array_char(array, index)               \
        ((((java_chararray*)(array))->data)[index])

#define access_array_short(array, index)               \
        ((((java_shortarray*)(array))->data)[index])

#define access_array_byte(array, index)               \
        ((((java_bytearray*)(array))->data)[index])

#define access_array_addr(array, index)               \
        ((((java_objectarray*)(array))->data)[index])

#define access_array_float(array, index)               \
        ((((java_floatarray*)(array))->data)[index])

/* (see createcompilerstub in codegen.c) */
#define FRAMESIZE(stub) (((Cell *)stub)[1])

#if 0
#define CLEARSTACK(_start, _end) \
	 do {Cell *__start=(_start); MSET(__start,0,u1,(_end)-__start); } while (0)
#else
#define CLEARSTACK(_start, _end)
#endif


#ifdef VM_DEBUG
#define NAME(_x) if (vm_debug) {fprintf(vm_out, "%lx: %-20s, ", (long)(ip-1), _x); fprintf(vm_out,"fp=%p, sp=%p", fp, sp);}
#else
#define NAME(_x)
#endif

#define LABEL2(name) J_##name: __asm__("");
#define LABEL3(name) K_##name: __asm__("");


java_objectheader *
engine(Inst *ip0, Cell * sp0, Cell * fp)
{
  Inst *ip;
  register Cell *sp SPREG = sp0;
  /* Inst ca1; XXX unused? */ /* code address; this is the next dispatched instruction */
  IF_spTOS(register Cell spTOS TOSREG;)
  static Inst   labels[] = {
#define INST_ADDR(_inst) (&&I_##_inst)
#include <java-labels.i>
#undef INST_ADDR
	  NULL,
#define INST_ADDR(_inst) (&&J_##_inst)
#include <java-labels.i>
#undef INST_ADDR
#define INST_ADDR(_inst) (&&K_##_inst)
#include <java-labels.i>
#undef INST_ADDR
    (Label)&&after_last,
    (Label)&&before_goto,
    (Label)&&after_goto,
#define INST_ADDR(_inst) (&&H_##_inst)
#include <java-labels.i>
#undef INST_ADDR
  };
  /* local variables for the various throw codes; this helps make
	 potentially throwing instructions relocatable (instead of a
	 non-relocatable direct jump, they perform an indirect jump) */
  Label throw_arithmeticexception            = &&throw_arithmeticexception1;
  Label throw_arrayindexoutofboundsexception = &&throw_arrayindexoutofboundsexception1;
  Label throw_classcastexception 			 = &&throw_classcastexception1;  
  Label throw_nullpointerexception 		     = &&throw_nullpointerexception1;
  Label throw_arraystoreexception            = &&throw_arraystoreexception1;
  java_objectheader *classcastexception_object = NULL;
  s4 arrayindexoutofbounds_index = 0; /* pass the index to the throw code */

  if (vm_debug)
      fprintf(vm_out,"entering engine(%p,%p,%p)\n",ip0,sp,fp);
  if (ip0 == NULL) {
    return (java_objectheader *)labels;
  }

  if (0) {
  before_goto:
	  goto *ip[-1];
  after_goto:
	  /* ensure that gcc does not constant-propagate the contents of
		 these variables and thus undo our relocatability work */
	  throw_arithmeticexception = 0;
	  throw_arrayindexoutofboundsexception = 0;
	  throw_classcastexception = 0;
	  throw_nullpointerexception = 0;
	  throw_arraystoreexception = 0;

      /* the actual codes jumped to through the ...exception variables */
	  THROWCODE(arithmeticexception);
	  THROWCODE(nullpointerexception);
	  THROWCODE(arraystoreexception);

  throw_classcastexception1:
	  global_sp = sp;
	  *exceptionptr = stacktrace_inline_classcastexception(NULL, (u1 *) fp, (u1 *) IP, (u1 *) IP, classcastexception_object);
	  CLEAR_global_sp;
	  THROW0;

  throw_arrayindexoutofboundsexception1:
	  global_sp = sp;
	  *exceptionptr = stacktrace_inline_arrayindexoutofboundsexception(NULL, (u1 *) fp, (u1 *) IP, (u1 *) IP, arrayindexoutofbounds_index);
	  CLEAR_global_sp;
	  THROW0;
  }

  /* I don't have a clue where these things come from,
     but I've put them in macros.h for the moment */
  IF_spTOS(spTOS = sp[0]);

  SET_IP(ip0);
  NEXT;

#define INST_ADDR(_inst) (&&I_##_inst)
#include <java-vm.i>
#undef NAME
 after_last: return NULL;
}


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
