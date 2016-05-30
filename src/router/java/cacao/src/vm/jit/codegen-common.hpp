/* src/vm/jit/codegen-common.hpp - architecture independent code generator stuff

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


#ifndef CODEGEN_COMMON_HPP_
#define CODEGEN_COMMON_HPP_ 1

#include "config.h"
#include "arch.hpp"
#include "md-abi.hpp"                   // for REG_ITMP1, REG_ITMP2, etc
#include "vm/global.hpp"                // for functionptr, java_handle_t, etc
#include "vm/types.hpp"                 // for s4, u1, u4, u2

class Linenumber;
struct basicblock;
struct branch_label_ref_t;
struct branchref;
struct codegendata;
struct dataref;
struct dsegentry;
struct fieldinfo;
struct instruction;
struct jitdata;
struct jumpref;
struct methoddesc;
struct methodinfo;
struct patchref_t;
struct rplpoint;
struct varinfo;
template <class T> class DumpList;

#define MCODEINITSIZE (1<<15)       /* 32 Kbyte code area initialization size */
#define DSEGINITSIZE  (1<<12)       /*  4 Kbyte data area initialization size */

#define NCODEINITSIZE (1<<15)       /* 32 Kbyte code area initialization size */


/* Register Pack/Unpack Macros ************************************************/

/* ATTENTION: Don't change the order where low and high bits are
   stored! At least mips32 relies in one case on that order. */

#define PACK_REGS(low,high) \
    ( (((high) & 0x0000ffff) << 16) | ((low) & 0x0000ffff) )

#define GET_LOW_REG(a)      ((a) & 0x0000ffff)
#define GET_HIGH_REG(a)    (((a) & 0xffff0000) >> 16)

/* All 32-bit machines we support use packed registers to store
   return values and temporary values. */

#if SIZEOF_VOID_P == 8
# define REG_LRESULT         REG_RESULT
# define REG_LTMP12          REG_ITMP1
# define REG_LTMP23          REG_ITMP2
#else
# define REG_LRESULT         REG_RESULT_PACKED
# define REG_LTMP12          REG_ITMP12_PACKED
# define REG_LTMP23          REG_ITMP23_PACKED
#endif


/* branch conditions **********************************************************/

#define BRANCH_UNCONDITIONAL    -1

#define BRANCH_EQ               (ICMD_IFEQ - ICMD_IFEQ)
#define BRANCH_NE               (ICMD_IFNE - ICMD_IFEQ)
#define BRANCH_LT               (ICMD_IFLT - ICMD_IFEQ)
#define BRANCH_GE               (ICMD_IFGE - ICMD_IFEQ)
#define BRANCH_GT               (ICMD_IFGT - ICMD_IFEQ)
#define BRANCH_LE               (ICMD_IFLE - ICMD_IFEQ)

#define BRANCH_ULT              256
#define BRANCH_ULE              257
#define BRANCH_UGE              258
#define BRANCH_UGT              259

#define BRANCH_NAN              260


/* common branch options ******************************************************/

#define BRANCH_OPT_NONE         0


/* codegendata ****************************************************************/

struct codegendata {
	u4              flags;          /* code generator flags                   */
	u1             *mcodebase;      /* base pointer of code area              */
	u1             *mcodeend;       /* pointer to end of code area            */
	s4              mcodesize;      /* complete size of code area (bytes)     */
	u1             *mcodeptr;       /* code generation pointer                */
	u1             *lastmcodeptr;   /* last patcher position of basic block   */

#if defined(ENABLE_INTRP)
	u1             *ncodebase;      /* base pointer of native code area       */
	s4              ncodesize;      /* complete size of native code area      */
	u1             *ncodeptr;       /* native code generation pointer         */

	u4              lastinstwithoutdispatch; /* ~0 if there was a dispatch    */

	s4              lastpatcheroffset; /* -1 if current super has no patcher  */
	s4              dynsuperm;      /* offsets of start of current dynamic ...*/
	s4              dynsupern;      /* ... superinstruction starts            */
	struct superstart *superstarts; /* list of supers without patchers        */
#endif

	dsegentry      *dseg;           /* chain of data segment entries          */
	s4              dseglen;        /* used size of data area (bytes)         */
                                    /* data area grows from top to bottom     */

	jumpref        *jumpreferences; /* list of jumptable target addresses     */

#if defined(__I386__) || defined(__X86_64__) || defined(__XDSPCORE__) || defined(ENABLE_INTRP) || defined(__S390__)
	dataref        *datareferences; /* list of data segment references        */
#endif

	DumpList<branch_label_ref_t*>* brancheslabel;
	DumpList<Linenumber>* linenumbers; ///< List of line numbers.

	methodinfo     *method;

	s4              stackframesize;    /* stackframe size of this method      */

#if defined(ENABLE_REPLACEMENT)
	rplpoint       *replacementpoint;  /* current replacement point           */
#endif
};


#define CODEGENDATA_FLAG_ERROR           0x00000001
#define CODEGENDATA_FLAG_LONGBRANCHES    0x00000002


#define CODEGENDATA_HAS_FLAG_ERROR(cd) \
    ((cd)->flags & CODEGENDATA_FLAG_ERROR)

#define CODEGENDATA_HAS_FLAG_LONGBRANCHES(cd) \
    ((cd)->flags & CODEGENDATA_FLAG_LONGBRANCHES)


/* branchref *****************************************************************/

struct branchref {
	s4         branchmpc;       /* patching position in code segment          */
	s4         condition;       /* conditional branch condition               */
	s4         reg;             /* register number to check                   */
	u4         options;         /* branch options                             */
	branchref *next;            /* next element in branchref list             */
};


/* branch_label_ref_t *********************************************************/

struct branch_label_ref_t {
	s4         mpc;             /* position in code segment                   */
	s4         label;           /* label number                               */
	s4         condition;       /* conditional branch condition               */
	s4         reg;             /* register number to check                   */
	u4         options;         /* branch options                             */
/* 	listnode_t linkage; */
};


/* jumpref ********************************************************************/

struct jumpref {
	s4          tablepos;       /* patching position in data segment          */
	basicblock *target;         /* target basic block                         */
	jumpref    *next;           /* next element in jumpref list               */
};


/* dataref ********************************************************************/

struct dataref {
	s4       datapos;           /* patching position in generated code        */
	dataref *next;              /* next element in dataref list               */
};


/* function prototypes ********************************************************/

void codegen_init(void);
void codegen_setup(jitdata *jd);

bool codegen_generate(jitdata *jd);
bool codegen_emit(jitdata *jd);

void codegen_emit_prolog(jitdata* jd);
void codegen_emit_epilog(jitdata* jd);
void codegen_emit_instruction(jitdata* jd, instruction* iptr);

#if defined(USES_PATCHABLE_MEMORY_BARRIER)
void codegen_emit_patchable_barrier(instruction *iptr, codegendata *cd, struct patchref_t *pr, fieldinfo *fi);
#endif

#if defined(ENABLE_INTRP)
bool intrp_codegen(jitdata *jd);
#endif

void codegen_close(void);

void codegen_increase(codegendata *cd);

#if defined(ENABLE_INTRP)
u1 *codegen_ncode_increase(codegendata *cd, u1 *ncodeptr);
#endif

void codegen_add_branch_ref(codegendata *cd, basicblock *target, s4 condition, s4 reg, u4 options);
void codegen_resolve_branchrefs(codegendata *cd, basicblock *bptr);

void codegen_branch_label_add(codegendata *cd, s4 label, s4 condition, s4 reg, u4 options);

#if defined(ENABLE_REPLACEMENT)
#if !defined(NDEBUG)
void codegen_set_replacement_point_notrap(codegendata *cd, s4 type);
void codegen_set_replacement_point(codegendata *cd, s4 type);
#else
void codegen_set_replacement_point_notrap(codegendata *cd);
void codegen_set_replacement_point(codegendata *cd);
#endif
#endif /* defined(ENABLE_REPLACEMENT) */

void codegen_finish(jitdata *jd);

java_handle_t *codegen_start_native_call(u1 *currentsp, u1 *pv);
java_object_t *codegen_finish_native_call(u1 *currentsp, u1 *pv);

s4 codegen_reg_of_var(u2 opcode, varinfo *v, s4 tempregnum);
s4 codegen_reg_of_dst(jitdata *jd, instruction *iptr, s4 tempregnum);

#if defined(ENABLE_SSA)
void codegen_emit_phi_moves(jitdata *jd, basicblock *bptr);
#endif

// REMOVEME
void codegen_emit_stub_compiler(jitdata *jd);
void codegen_emit_stub_native(jitdata *jd, methoddesc *nmd, functionptr f, int skipparams);

#endif // CODEGEN_COMMON_HPP_


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
