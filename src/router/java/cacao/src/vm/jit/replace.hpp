/* src/vm/jit/replace.hpp - on-stack replacement of methods

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


#ifndef REPLACE_HPP_
#define REPLACE_HPP_ 1

#include <stddef.h>                     // for NULL
#include "config.h"                     // for ENABLE_JIT, etc
#include "md-abi.hpp"                   // for FLT_REG_CNT, INT_SAV_CNT
#include "vm/jit/jit.hpp"               // for basicblock::Type
#include "vm/types.hpp"                 // for s4, u1, ptrint, u4, s8

struct codeinfo;
struct executionstate_t;
struct java_object_t;
struct jitdata;
struct methodinfo;
struct rplalloc;
struct rplpoint;
struct sourceframe_t;
struct sourcestate_t;
struct stackframeinfo_t;
union replace_val_t;

#if !defined(ENABLE_REPLACEMENT)

/*** macros for the codegens (disabled version) ************************/

#define REPLACEMENT_POINTS_INIT(cd, jd)
#define REPLACEMENT_POINTS_RESET(cd, jd)
#define REPLACEMENT_POINT_BLOCK_START(cd, bptr)
#define REPLACEMENT_POINT_INLINE_START(cd, iptr)
#define REPLACEMENT_POINT_INLINE_BODY(cd, iptr)
#define REPLACEMENT_POINT_RETURN(cd, iptr)
#define REPLACEMENT_POINT_INVOKE(cd, iptr)
#define REPLACEMENT_POINT_INVOKE_RETURN(cd, iptr)
#define REPLACEMENT_POINT_FORGC_BUILTIN(cd, iptr)
#define REPLACEMENT_POINT_FORGC_BUILTIN_RETURN(cd, iptr)

#else /* defined(ENABLE_REPLACEMENT) */

/*** structs *********************************************************/

#define RPLALLOC_STACK  -1
#define RPLALLOC_PARAM  -2
#define RPLALLOC_SYNC   -3

/* `rplalloc` is a compact struct for register allocation info        */

/* XXX optimize this for space efficiency */
struct rplalloc {
	s4           index;     /* local index, -1 for stack slot         */
	s4           regoff;    /* register index / stack slot offset     */
	unsigned int flags:4;   /* OR of (INMEMORY,...)                   */
	unsigned int type:4;    /* TYPE_... constant                      */
};

#if INMEMORY > 0x08
#error value of INMEMORY is too big to fit in rplalloc.flags
#endif

#if !defined(NDEBUG)
#define RPLPOINT_CHECK(type)     , rplpoint::TYPE_##type
#define RPLPOINT_CHECK_BB(bptr)  , (bptr)->type
#else
#define RPLPOINT_CHECK(type)
#define RPLPOINT_CHECK_BB(bptr)
#endif

/* An `rplpoint` represents a replacement point in a compiled method  */

struct rplpoint {
	/**
	 * CAUTION: Do not change the numerical values. These are used as
	 *          indices into replace_normalize_type_map.
	 * XXX what to do about overlapping rplpoints?
	 */
	enum Type {
		TYPE_STD     = basicblock::TYPE_STD,
		TYPE_EXH     = basicblock::TYPE_EXH,
		TYPE_SBR     = basicblock::TYPE_SBR,
		TYPE_CALL    = 3,
		TYPE_INLINE  = 4,
		TYPE_RETURN  = 5,
		TYPE_BODY    = 6
	};

	enum Flag {
		FLAG_NOTRAP    = 0x01,  // rplpoint cannot be trapped
		FLAG_COUNTDOWN = 0x02,  // count down hits
		FLAG_ACTIVE    = 0x08   // trap is active
	};

	u1          *pc;               /* machine code PC of this point    */
	methodinfo  *method;           /* source method this point is in   */
	rplpoint    *parent;           /* rplpoint of the inlined body     */ /* XXX unify with code */
	rplalloc    *regalloc;         /* pointer to register index table  */
	s4           id;               /* id of the rplpoint within method */
	s4           callsize;         /* size of call code in bytes       */
	unsigned int regalloccount:20; /* number of local allocations      */
	Type         type:4;           /* type of replacement point        */
	unsigned int flags:8;          /* OR of Flag constants             */
};


union replace_val_t {
	s4             i;
	s8             l;
	ptrint         p;
	struct {
		u4 lo;
		u4 hi;
	}              words;
	float          f;
	double         d;
	java_object_t *a;
};


struct sourceframe_t {
	sourceframe_t *down;           /* source frame down the call chain */

	methodinfo    *method;                  /* method this frame is in */
	s4             id;
	s4             type;

	/* values */
	replace_val_t  instance;

	replace_val_t *javastack;                  /* values of stack vars */
	u1            *javastacktype;              /*  types of stack vars */
	s4             javastackdepth;             /* number of stack vars */

	replace_val_t *javalocals;                 /* values of javalocals */
	u1            *javalocaltype;              /*  types of javalocals */
	s4             javalocalcount;             /* number of javalocals */

	replace_val_t *syncslots;
	s4             syncslotcount; /* XXX do we need more than one? */

	/* mapping info */
	rplpoint      *fromrp;         /* rplpoint used to read this frame */
	codeinfo      *fromcode;              /* code this frame was using */
	rplpoint      *torp;          /* rplpoint this frame was mapped to */
	codeinfo      *tocode;            /* code this frame was mapped to */

	/* info for native frames */
	stackframeinfo_t *sfi;      /* sfi for native frames, otherwise NULL */
	s4             nativeframesize;    /* size (bytes) of native frame */
	u1            *nativepc;
	ptrint         nativesavint[INT_SAV_CNT]; /* XXX temporary */
	double         nativesavflt[FLT_REG_CNT]; /* XXX temporary */
};

#define REPLACE_IS_NATIVE_FRAME(frame)  ((frame)->sfi != NULL)
#define REPLACE_IS_JAVA_FRAME(frame)    ((frame)->sfi == NULL)


struct sourcestate_t {
	sourceframe_t *frames;    /* list of source frames, from bottom up */
};


/*** macros for the codegens *******************************************/

#define REPLACEMENT_POINTS_INIT(cd, jd)                              \
    if (!replace_create_replacement_points(jd))                      \
        return false;                                                \
    (cd)->replacementpoint = (jd)->code->rplpoints;

#define REPLACEMENT_POINTS_RESET(cd, jd)                             \
    (cd)->replacementpoint = (jd)->code->rplpoints;

#define REPLACEMENT_POINT_BLOCK_START(cd, bptr)                      \
    if ((bptr)->bitflags & BBFLAG_REPLACEMENT)                       \
        codegen_set_replacement_point((cd) RPLPOINT_CHECK_BB(bptr));

#define REPLACEMENT_POINT_INLINE_START(cd, iptr)                     \
    codegen_set_replacement_point(cd RPLPOINT_CHECK(INLINE));

#define REPLACEMENT_POINT_INLINE_BODY(cd, iptr)                      \
    codegen_set_replacement_point_notrap(cd RPLPOINT_CHECK(BODY));

#define REPLACEMENT_POINT_RETURN(cd, iptr)                           \
    codegen_set_replacement_point(cd RPLPOINT_CHECK(RETURN));

#define REPLACEMENT_POINT_INVOKE(cd, iptr)                           \
    codegen_set_replacement_point(cd RPLPOINT_CHECK(CALL));

#define REPLACEMENT_POINT_INVOKE_RETURN(cd,  iptr)                   \
    if (iptr->opc != ICMD_BUILTIN)                                   \
        cd->replacementpoint[-1].callsize = (cd->mcodeptr - cd->mcodebase)\
                    - (ptrint) cd->replacementpoint[-1].pc;


/*** macros for the codegens (for GC) **********************************/

#if defined(ENABLE_GC_CACAO)

#define REPLACEMENT_POINT_FORGC_BUILTIN(cd, iptr)                    \
	codegen_set_replacement_point(cd RPLPOINT_CHECK(CALL));

#define REPLACEMENT_POINT_FORGC_BUILTIN_RETURN(cd, iptr)             \
	if (iptr->opc == ICMD_BUILTIN)                                   \
		cd->replacementpoint[-1].callsize = (cd->mcodeptr - cd->mcodebase)\
					- (ptrint) cd->replacementpoint[-1].pc;

#else // ENABLE_GC_CACAO

#define REPLACEMENT_POINT_FORGC_BUILTIN(cd, iptr)
#define REPLACEMENT_POINT_FORGC_BUILTIN_RETURN(cd, iptr)

#endif // ENABLE_GC_CACAO


/*** prototypes ********************************************************/

bool replace_create_replacement_points(jitdata *jd);
void replace_free_replacement_points(codeinfo *code);

void replace_activate_replacement_points(codeinfo *code, bool mappable);
void replace_deactivate_replacement_points(codeinfo *code);

bool replace_handler(u1 *pc, executionstate_t *es);

#if !defined(NDEBUG)
void replace_show_replacement_points(codeinfo *code);
void replace_replacement_point_println(rplpoint *rp, int depth);
void replace_sourcestate_println(sourcestate_t *ss);
void replace_sourcestate_println_short(sourcestate_t *ss);
void replace_source_frame_println(sourceframe_t *frame);
#endif

/* machine dependent functions (code in ARCH_DIR/md.c) */

#if defined(ENABLE_JIT)
void md_patch_replacement_point(u1 *pc, u1 *savedmcode, bool revert);
#endif

#endif // ENABLE_REPLACEMENT

#endif // REPLACE_HPP_


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
