/* src/vm/jit/ir/instruction.hpp - IR instruction

   Copyright (C) 2008
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


#ifndef _INSTRUCTION_HPP
#define _INSTRUCTION_HPP

#include "config.h"                     // for SIZEOF_VOID_PTR
#include <stdint.h>                     // for int32_t, uint32_t, etc
#include "vm/global.hpp"                // for Type, Type::Type_Void, etc
#include "vm/jit/ir/icmd.hpp"           // for icmdtable_entry_t, etc
#include "vm/references.hpp"            // for classref_or_classinfo, etc
#include "vm/resolve.hpp"               // for unresolved_method (used in macro)
#include "vm/types.hpp"                 // for u2, u4

struct basicblock;
struct insinfo_inline;
struct instruction;
struct methoddesc;
struct methodinfo;
struct rplpoint;

// Instruction structure.

/* branch_target_t: used in TABLESWITCH tables */

typedef union {
    int32_t                    insindex; /* used in parse                     */
    basicblock                *block;    /* valid after parse                 */
} branch_target_t;

/* lookup_target_t: used in LOOKUPSWITCH tables */

typedef struct {
    int32_t                    value;    /* case value                        */
    branch_target_t            target;   /* branch target, see above          */
} lookup_target_t;

/*** s1 operand ***/

typedef union {
	int32_t                    varindex;
	int32_t                    argcount;
} s1_operand_t;

/*** s2 operand ***/

typedef union {
	int32_t                    varindex;
	int32_t                   *args;
    classref_or_classinfo      c;
    unresolved_class          *uc;
    uintptr_t                  constval;         /* for PUT*CONST             */
    int32_t                    tablelow;         /* for TABLESWITCH           */
    uint32_t                   lookupcount;      /* for LOOKUPSWITCH          */
	int32_t                    retaddrnr;        /* for ASTORE                */
	instruction              **iargs;            /* for PHI                   */
} s2_operand_t;

/*** s3 operand ***/

typedef union {
	int32_t                    varindex;
    uintptr_t                  constval;
    classref_or_classinfo      c;
    constant_FMIref           *fmiref;
    unresolved_method         *um;
    unresolved_field          *uf;
    insinfo_inline            *inlineinfo;       /* for INLINE_START/END      */
    int32_t                    tablehigh;        /* for TABLESWITCH           */
    branch_target_t            lookupdefault;    /* for LOOKUPSWITCH          */
    branch_target_t            jsrtarget;        /* for JSR                   */
	int32_t                    javaindex;        /* for *STORE                */
    struct builtintable_entry *bte;
} s3_operand_t;

/*** val operand ***/

typedef union {
    int32_t                   i;
    uint32_t                  u;
    int64_t                   l;
    float                     f;
    double                    d;
    void                     *anyptr;
    java_handle_t            *stringconst;       /* for ACONST with string    */
    classref_or_classinfo     c;                 /* for ACONST with class     */
} val_operand_t;

/*** dst operand ***/

typedef union {
	int32_t                    varindex;
    basicblock                *block;       /* valid after parse              */
    branch_target_t           *table;       /* for TABLESWITCH                */
    lookup_target_t           *lookup;      /* for LOOKUPSWITCH               */
    int32_t                    insindex;    /* used in parse                  */
} dst_operand_t;

/*** flags (32 bits) ***/

enum InstructionFlag {
	INS_FLAG_BASICBLOCK = 0x01,  // marks a basic block start
	INS_FLAG_UNRESOLVED = 0x02,  // contains unresolved field/meth/class
	INS_FLAG_CLASS      = 0x04,  // for ACONST, PUT*CONST with class
	INS_FLAG_ARRAY      = 0x08,  // for CHECKCAST/INSTANCEOF with array
	INS_FLAG_CHECK      = 0x10,  // for *ALOAD|*ASTORE: check index
	                             // for BUILTIN: check exception
	INS_FLAG_KILL_PREV  = 0x04,  // for *STORE, invalidate prev local
	INS_FLAG_KILL_NEXT  = 0x08,  // for *STORE, invalidate next local
	INS_FLAG_RETADDR    = 0x10   // for ASTORE: op is a returnAddress
};

#define INS_FLAG_ID_SHIFT      5
#define INS_FLAG_ID_MASK       (~0 << INS_FLAG_ID_SHIFT)

typedef union {
    u4                  bits;
} flags_operand_t;


// Instruction.

/* The instruction format for the intermediate representation: */

struct instruction {
    ICMD                    opc   : 16; // opcode
    u2                      line;       // line number
#if SIZEOF_VOID_P == 8
    flags_operand_t         flags;      // 4 bytes
#endif
    s1_operand_t            s1;         // pointer-size
    union {
        struct {
            s2_operand_t    s2;         // pointer-size
            s3_operand_t    s3;         // pointer-size
        } s23;                          //     XOR
        val_operand_t       val;        //  long-size
    } sx;
    dst_operand_t           dst;        // pointer-size
#if SIZEOF_VOID_P == 4
    flags_operand_t         flags;      // 4 bytes
#endif
#if defined(ENABLE_ESCAPE_REASON)
	void *escape_reasons;
#endif
};


#define INSTRUCTION_STARTS_BASICBLOCK(iptr) \
	((iptr)->flags.bits & INS_FLAG_BASICBLOCK)

#define INSTRUCTION_IS_RESOLVED(iptr) \
	(!((iptr)->flags.bits & INS_FLAG_UNRESOLVED))

#define INSTRUCTION_IS_UNRESOLVED(iptr) \
	((iptr)->flags.bits & INS_FLAG_UNRESOLVED)

#define INSTRUCTION_MUST_CHECK(iptr) \
	((iptr)->flags.bits & INS_FLAG_CHECK)

#define INSTRUCTION_GET_FIELDREF(iptr,fref) \
	do { \
		if (iptr->flags.bits & INS_FLAG_UNRESOLVED) \
			fref = iptr->sx.s23.s3.uf->fieldref; \
		else \
			fref = iptr->sx.s23.s3.fmiref; \
	} while (0)

#define INSTRUCTION_GET_METHODREF(iptr,mref) \
	do { \
		if (iptr->flags.bits & INS_FLAG_UNRESOLVED) \
			mref = iptr->sx.s23.s3.um->methodref; \
		else \
			mref = iptr->sx.s23.s3.fmiref; \
	} while (0)

#define INSTRUCTION_GET_METHODDESC(iptr, md) \
	do { \
		if (iptr->flags.bits & INS_FLAG_UNRESOLVED) \
			md = iptr->sx.s23.s3.um->methodref->parseddesc.md; \
		else \
			md = iptr->sx.s23.s3.fmiref->parseddesc.md; \
	} while (0)


/* additional info structs for special instructions ***************************/

/* for ICMD_INLINE_START and ICMD_INLINE_END */

struct insinfo_inline {
	/* fields copied from the inlining tree ----------------------------------*/
	insinfo_inline *parent;     /* insinfo of the surrounding inlining, if any*/
	methodinfo     *method;     /* the inlined method starting/ending here    */
	methodinfo     *outer;      /* the outer method suspended/resumed here    */
	int32_t         synclocal;      /* local index used for synchronization   */
	bool            synchronize;    /* true if synchronization is needed      */
	int32_t         throughcount;   /* total # of pass-through variables      */
	int32_t         paramcount;     /* number of parameters of original call  */
	int32_t         stackvarscount; /* source stackdepth at INLINE_START      */
	int32_t        *stackvars;      /* stack vars at INLINE_START             */

	/* fields set by inlining ------------------------------------------------*/
	int32_t    *javalocals_start; /* javalocals at start of inlined body      */
	int32_t    *javalocals_end;   /* javalocals after inlined body            */

	/* fields set by replacement point creation ------------------------------*/
#if defined(ENABLE_REPLACEMENT)
	rplpoint   *rp;             /* replacement point at INLINE_START          */
#endif

	/* fields set by the codegen ---------------------------------------------*/
	int32_t     startmpc;       /* machine code offset of start of inlining   */
};


/* Additional instruction accessors */

methoddesc* instruction_call_site(const instruction* iptr);
Type        instruction_call_site_return_type(const instruction* iptr);

static inline bool instruction_has_dst(const instruction* iptr)
{
	if (
		(icmd_table[iptr->opc].dataflow == DF_INVOKE) ||
		(icmd_table[iptr->opc].dataflow == DF_BUILTIN)
		) {
		return instruction_call_site_return_type(iptr) != TYPE_VOID;
	}
	else {
		return icmd_table[iptr->opc].dataflow >= DF_DST_BASE;
	}
}

#endif // _INSTRUCTION_HPP


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
