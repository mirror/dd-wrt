/* src/vm/jit/code.hpp - codeinfo struct for representing compiled code

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


#ifndef CODE_HPP_
#define CODE_HPP_ 1

#include "config.h"                     // for ENABLE_REPLACEMENT, ENABLE_PROFILING
#include <assert.h>                     // for assert
#include <stdint.h>                     // for int32_t, uint8_t, uint32_t, etc
#include <stdlib.h>                     // for NULL
#include "vm/jit/methodheader.hpp"      // for CodeInfoPointer
#include "vm/types.hpp"                 // for u1, s4

class LinenumberTable;
struct exceptiontable_t;
struct methodinfo;
struct patchref_t;
struct rplalloc;
struct rplpoint;
template <class T> class LockedList;


/* constants ******************************************************************/

enum CodeFlag {
	CODE_FLAG_INVALID      = 0x0001,
	CODE_FLAG_LEAFMETHOD   = 0x0002,
	CODE_FLAG_SYNCHRONIZED = 0x0004,
	CODE_FLAG_TLH          = 0x0008
};


/* codeinfo *******************************************************************

   A codeinfo represents a particular realization of a method in
   machine code.

   ATTENTION: The methodinfo entry in the code-structure MUST have the
   offset 0, otherwise we have a problem in our compiler stub. This is
   checked with an assert in code_init().

*******************************************************************************/

struct codeinfo {
	methodinfo   *m;                    /* method this is a realization of    */
	codeinfo     *prev;                 /* previous codeinfo of this method   */

	uint32_t      flags;                /* OR of CODE_FLAG_ constants         */

	u1            optlevel;             /* optimization level of this code    */

	/* machine code */
	u1           *mcode;                /* pointer to machine code            */
	u1           *entrypoint;           /* machine code entry point           */
	s4            mcodelength;          /* length of generated machine code   */

	/* runtime information */
	int32_t       stackframesize;       /* size of the stackframe in slots    */
	int32_t       synchronizedoffset;   /* stack offset of synchronized obj.  */
	uint8_t       savedintcount;        /* number of callee saved int regs    */
	uint8_t       savedfltcount;        /* number of callee saved flt regs    */

	exceptiontable_t  *exceptiontable;
	LinenumberTable* linenumbertable;

	/* patcher list */
	LockedList<patchref_t>* patchers;

	/* replacement */
#if defined(ENABLE_REPLACEMENT)
	rplpoint     *rplpoints;            /* replacement points                 */
	rplalloc     *regalloc;             /* register allocation info           */
	s4            rplpointcount;        /* number of replacement points       */
	s4            globalcount;          /* number of global allocations       */
	s4            regalloccount;        /* number of total allocations        */
	s4            memuse;               /* number of arg + local slots        */
	u1           *savedmcode;           /* saved code under patches           */
#endif

	/* profiling information */
#if defined(ENABLE_PROFILING)
	u4            frequency;            /* number of method invocations       */
	s4            basicblockcount;      /* number of basic blocks             */
	u4           *bbfrequency;          /* basic block profiling information  */
	s8            cycles;               /* number of cpu cycles               */
#endif
};


/* inline functions ***********************************************************/

/* code_xxx_invalid ************************************************************

   Functions for CODE_FLAG_INVALID.

*******************************************************************************/

inline static int code_is_invalid(codeinfo *code)
{
	return (code->flags & CODE_FLAG_INVALID);
}

inline static void code_flag_invalid(codeinfo *code)
{
	code->flags |= CODE_FLAG_INVALID;
}

inline static void code_unflag_invalid(codeinfo *code)
{
	code->flags &= ~CODE_FLAG_INVALID;
}


/* code_xxx_leafmethod *********************************************************

   Functions for CODE_FLAG_LEAFMETHOD.

*******************************************************************************/

inline static int code_is_leafmethod(codeinfo *code)
{
	return (code->flags & CODE_FLAG_LEAFMETHOD);
}

inline static void code_flag_leafmethod(codeinfo *code)
{
	code->flags |= CODE_FLAG_LEAFMETHOD;
}

inline static void code_unflag_leafmethod(codeinfo *code)
{
	code->flags &= ~CODE_FLAG_LEAFMETHOD;
}


/* code_xxx_synchronized *******************************************************

   Functions for CODE_FLAG_SYNCHRONIZED.

*******************************************************************************/

inline static int code_is_synchronized(codeinfo *code)
{
	return (code->flags & CODE_FLAG_SYNCHRONIZED);
}

inline static void code_flag_synchronized(codeinfo *code)
{
	code->flags |= CODE_FLAG_SYNCHRONIZED;
}

inline static void code_unflag_synchronized(codeinfo *code)
{
	code->flags &= ~CODE_FLAG_SYNCHRONIZED;
}


/* code_get_codeinfo_for_pv ****************************************************

   Return the codeinfo for the given PV.

   IN:
       pv...............PV

   RETURN VALUE:
       the codeinfo *

*******************************************************************************/

inline static codeinfo *code_get_codeinfo_for_pv(void *pv)
{
	codeinfo *code;

	assert(pv != NULL);

	code = *((codeinfo **) (((uintptr_t) pv) + CodeinfoPointer));

	return code;
}


/* function prototypes ********************************************************/

void code_init(void);

codeinfo *code_codeinfo_new(methodinfo *m);
void code_codeinfo_free(codeinfo *code);

codeinfo *code_find_codeinfo_for_pc(void *pc);
codeinfo *code_find_codeinfo_for_pc_nocheck(void *pc);

methodinfo *code_get_methodinfo_for_pv(void *pv);

#if defined(ENABLE_REPLACEMENT)
int code_get_sync_slot_count(codeinfo *code);
#endif /* defined(ENABLE_REPLACEMENT) */

void code_free_code_of_method(methodinfo *m);

#endif // CODE_HPP_


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
