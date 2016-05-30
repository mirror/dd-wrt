/* src/vm/jit/emit-common.hpp - common code emitter functions

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2009 Theobroma Systems Ltd.

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


#ifndef EMIT_COMMON_HPP_
#define EMIT_COMMON_HPP_ 1

#include "config.h"
#include <cassert>                      // for assert
#include <stdint.h>                     // for int32_t, uint32_t
#include "arch.hpp"
#include "codegen.hpp"
#include "vm/jit/codegen-common.hpp"    // for codegendata
#include "vm/types.hpp"                 // for s4, u4, s8

struct basicblock;
struct codegendata;
struct codeinfo;
struct instruction;
struct jitdata;
struct varinfo;

/* branch labels **************************************************************/

#define BRANCH_LABEL_1    1
#define BRANCH_LABEL_2    2
#define BRANCH_LABEL_3    3
#define BRANCH_LABEL_4    4
#define BRANCH_LABEL_5    5
#define BRANCH_LABEL_6    6
#define BRANCH_LABEL_7    7
#define BRANCH_LABEL_8    8
#define BRANCH_LABEL_9    9
#define BRANCH_LABEL_10  10


/* constant range macros ******************************************************/

#if SIZEOF_VOID_P == 8

# define IS_IMM8(c) \
    (((s8) (c) >= -128) && ((s8) (c) <= 127))

# define IS_IMM32(c) \
    (((s8) (c) >= (-2147483647-1)) && ((s8) (c) <= 2147483647))

#else

# define IS_IMM8(c) \
    (((s4) (c) >= -128) && ((s4) (c) <= 127))

# define IS_IMM16(c) \
    (((s4) (c) >= -32768) && ((s4) (c) <= 32767))

#endif


/* code generation functions **************************************************/

s4 emit_load(jitdata *jd, instruction *iptr, varinfo *src, s4 tempreg);
s4 emit_load_s1(jitdata *jd, instruction *iptr, s4 tempreg);
s4 emit_load_s2(jitdata *jd, instruction *iptr, s4 tempreg);
s4 emit_load_s3(jitdata *jd, instruction *iptr, s4 tempreg);

#if SIZEOF_VOID_P == 4
s4 emit_load_low(jitdata *jd, instruction *iptr, varinfo *src, s4 tempreg);
s4 emit_load_s1_low(jitdata *jd, instruction *iptr, s4 tempreg);
s4 emit_load_s2_low(jitdata *jd, instruction *iptr, s4 tempreg);
s4 emit_load_s3_low(jitdata *jd, instruction *iptr, s4 tempreg);

s4 emit_load_high(jitdata *jd, instruction *iptr, varinfo *src, s4 tempreg);
s4 emit_load_s1_high(jitdata *jd, instruction *iptr, s4 tempreg);
s4 emit_load_s2_high(jitdata *jd, instruction *iptr, s4 tempreg);
s4 emit_load_s3_high(jitdata *jd, instruction *iptr, s4 tempreg);
#endif

void emit_store(jitdata *jd, instruction *iptr, varinfo *dst, s4 d);
void emit_store_dst(jitdata *jd, instruction *iptr, s4 d);

#if SIZEOF_VOID_P == 4
void emit_store_low(jitdata *jd, instruction *iptr, varinfo *dst, s4 d);
void emit_store_high(jitdata *jd, instruction *iptr, varinfo *dst, s4 d);
#endif

void emit_copy(jitdata *jd, instruction *iptr);

void emit_iconst(codegendata *cd, s4 d, s4 value);
void emit_lconst(codegendata *cd, s4 d, s8 value);

/* compare-emitting functions targeting an integer register */

#if SUPPORT_BRANCH_CONDITIONAL_ONE_INTEGER_REGISTER
void emit_icmpeq_imm(codegendata* cd, int reg, int32_t value, int d);
#endif

/* compare-emitting functions targeting the condition register */

#if SUPPORT_BRANCH_CONDITIONAL_CONDITION_REGISTER
void emit_icmp_imm(codegendata* cd, int reg, int32_t value);
#endif

/* branch-emitting functions */
void emit_bccz(codegendata *cd, basicblock *target, s4 condition, s4 reg, u4 options);
void emit_bcc(codegendata *cd, basicblock *target, s4 condition, u4 options);

/* wrapper for unconditional branches */
void emit_br(codegendata *cd, basicblock *target);

/* wrappers for branches on one integer register */

#if SUPPORT_BRANCH_CONDITIONAL_ONE_INTEGER_REGISTER
void emit_beqz(codegendata *cd, basicblock *target, s4 reg);
void emit_bnez(codegendata *cd, basicblock *target, s4 reg);
void emit_bltz(codegendata *cd, basicblock *target, s4 reg);
void emit_bgez(codegendata *cd, basicblock *target, s4 reg);
void emit_bgtz(codegendata *cd, basicblock *target, s4 reg);
void emit_blez(codegendata *cd, basicblock *target, s4 reg);
#endif

/* wrappers for branches on two integer registers */

#if SUPPORT_BRANCH_CONDITIONAL_TWO_INTEGER_REGISTERS
void emit_beq(codegendata *cd, basicblock *target, s4 s1, s4 s2);
void emit_bne(codegendata *cd, basicblock *target, s4 s1, s4 s2);
#endif

/* wrappers for branches on condition codes */

#if SUPPORT_BRANCH_CONDITIONAL_CONDITION_REGISTER
void emit_beq(codegendata *cd, basicblock *target);
void emit_bne(codegendata *cd, basicblock *target);
void emit_blt(codegendata *cd, basicblock *target);
void emit_bge(codegendata *cd, basicblock *target);
void emit_bgt(codegendata *cd, basicblock *target);
void emit_ble(codegendata *cd, basicblock *target);
#endif

#if SUPPORT_BRANCH_CONDITIONAL_UNSIGNED_CONDITIONS
void emit_bult(codegendata *cd, basicblock *target);
void emit_bule(codegendata *cd, basicblock *target);
void emit_buge(codegendata *cd, basicblock *target);
void emit_bugt(codegendata *cd, basicblock *target);
#endif

#if defined(__POWERPC__) || defined(__POWERPC64__)
void emit_bnan(codegendata *cd, basicblock *target);
#endif

/* label-branches */
void emit_label_bccz(codegendata *cd, s4 label, s4 condition, s4 reg, u4 options);
void emit_label(codegendata *cd, s4 label);
void emit_label_bcc(codegendata *cd, s4 label, s4 condition, u4 options);

void emit_label_br(codegendata *cd, s4 label);

#if SUPPORT_BRANCH_CONDITIONAL_ONE_INTEGER_REGISTER
void emit_label_beqz(codegendata* cd, int label, int reg);
void emit_label_bnez(codegendata* cd, int label, int reg);
void emit_label_bltz(codegendata* cd, int label, int reg);
void emit_label_bgtz(codegendata* cd, int label, int reg);
#endif

#if SUPPORT_BRANCH_CONDITIONAL_TWO_INTEGER_REGISTERS
void emit_label_beq(codegendata* cd, int label, int s1, int s2);
void emit_label_bne(codegendata* cd, int label, int s1, int s2);
#endif

#if SUPPORT_BRANCH_CONDITIONAL_CONDITION_REGISTER
void emit_label_beq(codegendata *cd, s4 label);
void emit_label_bne(codegendata *cd, s4 label);
void emit_label_blt(codegendata *cd, s4 label);
void emit_label_bge(codegendata *cd, s4 label);
void emit_label_bgt(codegendata *cd, s4 label);
void emit_label_ble(codegendata *cd, s4 label);
#endif

/* machine dependent branch-emitting function */
void emit_branch(codegendata *cd, s4 disp, s4 condition, s4 reg, u4 options);

void emit_arithmetic_check(codegendata *cd, instruction *iptr, s4 reg);
void emit_arrayindexoutofbounds_check(codegendata *cd, instruction *iptr, s4 s1, s4 s2);
void emit_arraystore_check(codegendata *cd, instruction *iptr);
void emit_classcast_check(codegendata *cd, instruction *iptr, s4 condition, s4 reg, s4 s1);
void emit_nullpointer_check(codegendata *cd, instruction *iptr, s4 reg);
void emit_exception_check(codegendata *cd, instruction *iptr);

void emit_trap_compiler(codegendata *cd);
void emit_trap_countdown(codegendata *cd, s4 *counter);
uint32_t emit_trap(codegendata *cd);

void emit_patcher_traps(jitdata *jd);

void emit_recompute_pv(codegendata* cd);

/* machine dependent faspath-emitting functions */
void emit_fastpath_monitor_enter(jitdata* jd, instruction* iptr, int d);
void emit_fastpath_monitor_exit(jitdata* jd, instruction* iptr, int d);

void emit_monitor_enter(jitdata* jd, int32_t syncslot_offset);
void emit_monitor_exit(jitdata* jd, int32_t syncslot_offset);

#if defined(ENABLE_PROFILING)
void emit_profile_method(codegendata* cd, codeinfo* code);
void emit_profile_basicblock(codegendata* cd, codeinfo* code, basicblock* bptr);
void emit_profile_cycle_start(codegendata* cd, codeinfo* code);
void emit_profile_cycle_stop(codegendata* cd, codeinfo* code);
#endif

void emit_verbosecall_enter(jitdata *jd);
void emit_verbosecall_exit(jitdata *jd);

/* inline code generation functions *******************************************/

/**
 * Generates an integer-move from register s to d. If s and d are
 * the same registers, no code will be generated.
 */
static inline void emit_imove(codegendata* cd, int s, int d)
{
	if (s != d)
#if defined(__ARM__)
		// XXX Fix this!!!
		M_MOV(d, s);
#else
		M_MOV(s, d);
#endif
}


/**
 * Generates a long-move from register s to d. If s and d are
 * the same registers, no code will be generated.
 */
static inline void emit_lmove(codegendata* cd, int s, int d)
{
#if SIZEOF_VOID_P == 8
	emit_imove(cd, s, d);
#else
	if (GET_HIGH_REG(s) == GET_LOW_REG(d)) {
		assert((GET_LOW_REG(s) != GET_HIGH_REG(d)));
		emit_imove(cd, GET_HIGH_REG(s), GET_HIGH_REG(d));
		emit_imove(cd, GET_LOW_REG(s), GET_LOW_REG(d));
	} else {
		emit_imove(cd, GET_LOW_REG(s), GET_LOW_REG(d));
		emit_imove(cd, GET_HIGH_REG(s), GET_HIGH_REG(d));
	}
#endif
}


/**
 * Generates a float-move from register s to d. If s and d are
 * the same registers, no code will be generated.
 */
static inline void emit_fmove(codegendata* cd, int s, int d)
{
	if (s != d)
		M_FMOV(s, d);
}


/**
 * Generates an double-move from register s to d. If s and d are
 * the same registers, no code will be generated.
 */
static inline void emit_dmove(codegendata* cd, int s, int d)
{
	if (s != d)
		M_DMOV(s, d);
}


/* preserve compatibility with legacy code ************************************/

#define M_INTMOVE(a, b)      emit_imove(cd, a, b)
#define M_LNGMOVE(a, b)      emit_lmove(cd, a, b)
#define M_FLTMOVE(a, b)      emit_fmove(cd, a, b)
#define M_DBLMOVE(a, b)      emit_dmove(cd, a, b)


#endif // EMIT_COMMON_HPP_


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
