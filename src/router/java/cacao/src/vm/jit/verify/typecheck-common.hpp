/* src/vm/jit/verify/typecheck-common.h - internal header for the type checker

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


#ifndef _TYPECHECK_COMMON_H
#define _TYPECHECK_COMMON_H

#include "config.h"
#include "vm/types.hpp"
#include "vm/global.hpp"

#include <assert.h>

#include "vm/jit/jit.hpp"


/****************************************************************************/
/* DEBUG HELPERS                                                            */
/****************************************************************************/

// TODO port to the new logging framework!

#ifdef TYPECHECK_DEBUG
#define TYPECHECK_ASSERT(cond)  assert(cond)
#else
#define TYPECHECK_ASSERT(cond)
#endif

#ifdef TYPECHECK_VERBOSE_OPT
extern bool opt_typecheckverbose;
#define DOLOG(action)  do { if (opt_typecheckverbose) {action;} } while(0)
#else
#define DOLOG(action)
#endif

#ifdef TYPECHECK_VERBOSE
#define TYPECHECK_VERBOSE_IMPORTANT
#define OLD_LOGNL              DOLOG(puts(""))
#define OLD_LOG(str)           DOLOG(puts(str);)
#define OLD_LOG1(str,a)        DOLOG(printf(str,a); OLD_LOGNL)
#define OLD_LOG2(str,a,b)      DOLOG(printf(str,a,b); OLD_LOGNL)
#define OLD_LOG3(str,a,b,c)    DOLOG(printf(str,a,b,c); OLD_LOGNL)
#define OLD_LOGIF(cond,str)    DOLOG(do {if (cond) { puts(str); }} while(0))
#ifdef  TYPEINFO_DEBUG
#define OLD_LOGINFO(info)      DOLOG(do {typeinfo_print_short(stdout,(info)); OLD_LOGNL;} while(0))
#else
#define OLD_LOGINFO(info)
#define typevector_print(x,y,z)
#endif
#define OLD_LOGFLUSH           DOLOG(fflush(stdout))
#define OLD_LOGSTR(str)        DOLOG(printf("%s", str))
#define OLD_LOGSTR1(str,a)     DOLOG(printf(str,a))
#define OLD_LOGSTR2(str,a,b)   DOLOG(printf(str,a,b))
#define OLD_LOGSTR3(str,a,b,c) DOLOG(printf(str,a,b,c))
#define OLD_LOGNAME(c)         DOLOG(class_classref_or_classinfo_print(c))
#define OLD_LOGMETHOD(str,m)   DOLOG(printf("%s", str); method_println(m);)
#else
#define OLD_LOG(str)
#define OLD_LOG1(str,a)
#define OLD_LOG2(str,a,b)
#define OLD_LOG3(str,a,b,c)
#define OLD_LOGIF(cond,str)
#define OLD_LOGINFO(info)
#define OLD_LOGFLUSH
#define OLD_LOGNL
#define OLD_LOGSTR(str)
#define OLD_LOGSTR1(str,a)
#define OLD_LOGSTR2(str,a,b)
#define OLD_LOGSTR3(str,a,b,c)
#define OLD_LOGNAME(c)
#define OLD_LOGMETHOD(str,m)
#endif

#ifdef TYPECHECK_VERBOSE_IMPORTANT
#define OLD_LOGimp(str)     DOLOG(puts(str);OLD_LOGNL)
#define OLD_LOGimpSTR(str)  DOLOG(puts(str))
#else
#define OLD_LOGimp(str)
#define OLD_LOGimpSTR(str)
#endif

#if defined(TYPECHECK_VERBOSE) || defined(TYPECHECK_VERBOSE_IMPORTANT)
#include <stdio.h>
void typecheck_print_var(FILE *file, jitdata *jd, s4 index);
void typecheck_print_vararray(FILE *file, jitdata *jd, s4 *vars, int len);
#endif


/****************************************************************************/
/* STATISTICS                                                               */
/****************************************************************************/

#if defined(TYPECHECK_DEBUG) && !defined(TYPECHECK_NO_STATISTICS)
/*#define TYPECHECK_STATISTICS*/
#endif

#ifdef TYPECHECK_STATISTICS
#define STAT_ITERATIONS  10
#define STAT_BLOCKS      10
#define STAT_LOCALS      16

extern int stat_typechecked;
extern int stat_methods_with_handlers;
extern int stat_methods_maythrow;
extern int stat_iterations[STAT_ITERATIONS+1];
extern int stat_reached;
extern int stat_copied;
extern int stat_merged;
extern int stat_merging_changed;
extern int stat_blocks[STAT_BLOCKS+1];
extern int stat_locals[STAT_LOCALS+1];
extern int stat_ins;
extern int stat_ins_maythrow;
extern int stat_ins_stack;
extern int stat_ins_field;
extern int stat_ins_field_unresolved;
extern int stat_ins_field_uninitialized;
extern int stat_ins_invoke;
extern int stat_ins_invoke_unresolved;
extern int stat_ins_primload;
extern int stat_ins_aload;
extern int stat_ins_builtin;
extern int stat_ins_builtin_gen;
extern int stat_ins_branch;
extern int stat_ins_switch;
extern int stat_ins_primitive_return;
extern int stat_ins_areturn;
extern int stat_ins_areturn_unresolved;
extern int stat_ins_athrow;
extern int stat_ins_athrow_unresolved;
extern int stat_ins_unchecked;
extern int stat_handlers_reached;
extern int stat_savedstack;

#define TYPECHECK_MARK(var)   ((var) = true)
#define TYPECHECK_COUNT(cnt)  (cnt)++
#define TYPECHECK_COUNTIF(cond,cnt)  do{if(cond) (cnt)++;} while(0)
#define TYPECHECK_COUNT_FREQ(array,val,limit) \
	do {									  \
		if ((val) < (limit)) (array)[val]++;  \
		else (array)[limit]++;				  \
	} while (0)

void typecheck_print_statistics(FILE *file);

#else /* !defined(TYPECHECK_STATISTICS) */
						   
#define TYPECHECK_COUNT(cnt)
#define TYPECHECK_MARK(var)
#define TYPECHECK_COUNTIF(cond,cnt)
#define TYPECHECK_COUNT_FREQ(array,val,limit)

#endif /* defined(TYPECHECK_STATISTICS) */


/****************************************************************************/
/* MACROS FOR THROWING EXCEPTIONS                                           */
/****************************************************************************/

#define TYPECHECK_VERIFYERROR_ret(m,msg,retval)                      \
    do {                                                             \
        exceptions_throw_verifyerror((m), (msg));                    \
        return (retval);                                             \
    } while (0)

#define TYPECHECK_VERIFYERROR_main(msg)  TYPECHECK_VERIFYERROR_ret(state.m,(msg),NULL)
#define TYPECHECK_VERIFYERROR_bool(msg)  TYPECHECK_VERIFYERROR_ret(state->m,(msg),false)


/****************************************************************************/
/* MISC MACROS                                                              */
/****************************************************************************/

#define COPYTYPE(source,dest)                                        \
    {if (VAROP(source)->type == TYPE_ADR) {                          \
            VAROP(dest)->typeinfo = VAROP(source)->typeinfo;}}


/****************************************************************************/
/* JSR VERIFICATION (stack-based verifier)                                  */
/****************************************************************************/

typedef struct typecheck_jsr_t typecheck_jsr_t;
typedef struct typecheck_jsr_caller_t typecheck_jsr_caller_t;

struct typecheck_jsr_caller_t {
	typecheck_jsr_caller_t *next;                   /* next in linked list */
	basicblock *callblock;             /* block containing the calling JSR */
};

struct typecheck_jsr_t {
	typecheck_jsr_t *next;               /* next (lower) in the call chain */
	basicblock  *start;                                   /* for debugging */
	typecheck_jsr_caller_t *callers;  /* list of callers (blocks with JSR) */
	basicblock  *retblock;              /* block with the RET for this sub */
	bool         active;           /* true if this sub is currently active */
	char        *blockflags;   /* saved block flags when JSR was traversed */
	char        *usedlocals;       /* != 0 for each local used in this sub */
	typedescriptor_t *retlocals;                 /* locals on the RET edge */
	typedescriptor_t *retstack;                   /* stack on the RET edge */
	s4              retdepth;               /* stack depth on the RET edge */
};

/****************************************************************************/
/* VERIFIER STATE STRUCT                                                    */
/****************************************************************************/

/* verifier_state - This structure keeps the current state of the      */
/* bytecode verifier for passing it between verifier functions.        */

typedef struct verifier_state {
    instruction *iptr;               /* pointer to current instruction */
    basicblock *bptr;                /* pointer to current basic block */

	methodinfo *m;                               /* the current method */
	jitdata *jd;                         /* jitdata for current method */
	codegendata *cd;                 /* codegendata for current method */

	basicblock *basicblocks;
	s4 basicblockcount;
	
	s4 numlocals;                         /* number of local variables */
	s4 validlocals;                /* number of Java-accessible locals */
	
	typedescriptor_t returntype;  /* return type of the current method */

	s4 *savedindices;
	s4 *savedinvars;                            /* saved invar pointer */

	s4 exinvars;
	
    exception_entry **handlers;           /* active exception handlers */

    bool repeat;            /* if true, blocks are iterated over again */
    bool initmethod;             /* true if this is an "<init>" method */

#ifdef TYPECHECK_STATISTICS
	bool stat_maythrow;          /* at least one instruction may throw */
#endif

	/* the following fields are used by the stackbased verifier only:  */

	typedescriptor_t *locals;               /* current local variables */
	typedescriptor_t *startlocals;/* locals at the start of each block */
	typedescriptor_t *startstack;  /* stack at the start of each block */
	s4               *indepth;                /* stack depth at --''-- */
	typedescriptor_t *stackceiling;    /* upper edge of verifier stack */

	typecheck_jsr_t *topjsr;        /* most recently called subroutine */
	typecheck_jsr_t **jsrinfos;      /* subroutine info for each block */
} verifier_state;

void typecheck_init_state(verifier_state *state, basicblock::State minstate);
void typecheck_reset_state(verifier_state *state);

bool typecheck_copy_types(verifier_state *state,
						  s4 *srcvars, s4 *dstvars, s4 n);

typecheck_result typecheck_merge_types(verifier_state *state,
									   s4 *srcvars,
									   s4 *dstvars,
									   s4 n);

typecheck_result typestate_merge(verifier_state *state,
				                 s4 *srcvars, varinfo *srclocals,
				                 s4 *dstvars, varinfo *dstlocals,
				                 s4 n);

bool typestate_reach(verifier_state *state,
					 basicblock *destblock,
					 s4 *srcvars, varinfo *srclocals, s4 n);

bool typecheck_init_locals(verifier_state *state, bool newthis);


#endif /* _TYPECHECK_COMMON_H */

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
