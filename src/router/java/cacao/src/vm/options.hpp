/* src/vm/options.hpp - define global options extern

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


#ifndef OPTIONS_HPP_
#define OPTIONS_HPP_ 1

#include <stdint.h>                     // for int64_t
#include <stdio.h>                      // for FILE
#include "config.h"                     // for ENABLE_DEBUG_FILTER, etc
#include "native/jni.hpp"               // for JavaVMInitArgs
#include "toolbox/logging.hpp"
#include "vm/types.hpp"                 // for s4

struct opt_struct;
struct option_t;

/* reserved option numbers ****************************************************/

/* define these negative since the other options are an enum */

#define OPT_DONE       -1
#define OPT_ERROR      -2
#define OPT_IGNORE     -3


typedef struct opt_struct opt_struct;

struct opt_struct {
	const char *name;
	bool        arg;
	int         value;
};


typedef struct option_t option_t;

struct option_t {
	const char *name;
	int         value;
	int         type;
	const char *doc;
};


/* global variables ***********************************************************/

extern s4    opt_index;
extern char *opt_arg;

extern bool opt_foo;

extern bool opt_jit;
extern bool opt_intrp;

extern bool opt_jar;
extern bool opt_run;

extern s4   opt_heapmaxsize;
extern s4   opt_heapstartsize;
extern s4   opt_stacksize;

extern bool opt_verbose;
extern bool opt_debugcolor;

extern bool loadverbose;         /* Print debug messages during loading */
extern bool initverbose;         /* Log class initialization */

extern bool opt_verboseclass;
extern bool opt_verbosegc;
extern bool opt_verbosejni;
extern bool opt_verbosecall;

extern bool showmethods;
extern bool showconstantpool;
extern bool showutf;

extern bool compileverbose;
extern bool showstack;

extern bool opt_showdisassemble;
extern bool opt_showddatasegment;
extern bool opt_showintermediate;

extern bool checkbounds;
extern bool checksync;
#if defined(ENABLE_LOOP)
extern bool opt_loops;
#endif

extern bool makeinitializations;

#if defined(ENABLE_STATISTICS)
extern bool opt_stat;
extern bool opt_getloadingtime;
extern bool opt_getcompilingtime;
#endif
#if defined(ENABLE_VERIFIER)
extern bool opt_verify;
#endif

#if defined(ENABLE_PROFILING)
extern bool opt_prof;
extern bool opt_prof_bb;
#endif

/* optimization options *******************************************************/

#if defined(ENABLE_IFCONV)
extern bool opt_ifconv;
#endif

#if defined(ENABLE_LSRA) || defined(ENABLE_SSA)
extern bool opt_lsra;
#endif
#if defined(ENABLE_SSA)
extern bool opt_ssa_dce;          /* enable dead code elemination */
extern bool opt_ssa_cp;           /* enable copy propagation      */
#endif

/* interpreter options ********************************************************/

#if defined(ENABLE_INTRP)
extern bool opt_no_dynamic;
extern bool opt_no_replication;
extern bool opt_no_quicksuper;

extern s4   opt_static_supers;
extern bool vm_debug;
#endif

/* debug output filtering options *********************************************/

#if defined(ENABLE_DEBUG_FILTER)
extern const char *opt_filter_verbosecall_include;
extern const char *opt_filter_verbosecall_exclude;
extern const char *opt_filter_show_method;
#endif


/* -XX options ****************************************************************/

/* NOTE: For better readability keep these alpha-sorted. */

/* Options which must always be available (production options in
   HotSpot). */

extern int64_t  opt_MaxDirectMemorySize;
extern int      opt_MaxPermSize;
extern int      opt_PermSize;
extern int      opt_ThreadStackSize;

/* Debugging options which can be turned off. */

extern bool     opt_AlwaysEmitLongBranches;
extern bool     opt_AlwaysMmapFirstPage;
extern int      opt_CompileAll;
extern char*    opt_CompileMethod;
extern char*    opt_CompileSignature;
extern int      opt_DebugLocalReferences;
extern int      opt_DebugLocks;
extern int      opt_DebugPatcher;
extern int      opt_DebugPackage;
extern int      opt_DebugStackFrameInfo;
extern int      opt_DebugStackTrace;
extern int      opt_DebugThreads;
#if defined(ENABLE_DISASSEMBLER)
extern int      opt_DisassembleStubs;
#endif
#if defined(ENABLE_OPAGENT)
extern int      opt_EnableOpagent;
#endif
#if defined(ENABLE_GC_CACAO)
extern int      opt_GCDebugRootSet;
extern int      opt_GCStress;
#endif
#if defined(ENABLE_INLINING)
extern int      opt_Inline;
#if defined(ENABLE_INLINING_DEBUG) || !defined(NDEBUG)
extern int      opt_InlineAll;
extern int      opt_InlineCount;
extern int      opt_InlineMaxSize;
extern int      opt_InlineMinSize;
#endif
#endif
extern int      opt_PrintConfig;
extern int      opt_PrintWarnings;
extern int      opt_ProfileGCMemoryUsage;
extern int      opt_ProfileMemoryUsage;
extern FILE    *opt_ProfileMemoryUsageGNUPlot;
extern int      opt_RegallocSpillAll;
#if defined(ENABLE_REPLACEMENT)
extern int      opt_TestReplacement;
#endif
extern int      opt_TraceBuiltinCalls;
extern int      opt_TraceCompilerCalls;
extern int      opt_TraceExceptions;
extern int      opt_TraceHPI;
#if defined(ENABLE_INLINING) && !defined(NDEBUG)
extern int      opt_TraceInlining;
#endif
extern int      opt_TraceJavaCalls;
extern bool     opt_TraceJMMCalls;
extern int      opt_TraceJNICalls;
extern int      opt_TraceJVMCalls;
extern int      opt_TraceJVMCallsVerbose;
#if defined(ENABLE_JVMTI)
extern int      opt_TraceJVMTICalls;
#endif
extern int      opt_TraceLinkClass;
#if defined(ENABLE_RT_TIMING)
extern FILE    *opt_RtTimingLogfile;
#endif
#if defined(ENABLE_STATISTICS)
extern FILE    *opt_StatisticsLogfile;
#endif
#if defined(ENABLE_REPLACEMENT)
extern int      opt_TraceReplacement;
#endif
extern int      opt_TraceSubsystemInitialization;
extern int      opt_TraceTraps;


/* function prototypes ********************************************************/

int  options_get(opt_struct *opts, JavaVMInitArgs *vm_args);
void options_xx(JavaVMInitArgs *vm_args);


/* debug **********************************************************************/

#if !defined(NDEBUG)
# define TRACESUBSYSTEMINITIALIZATION(text)						\
    do {														\
        if (opt_TraceSubsystemInitialization) {					\
            log_println("[Initializing subsystem: %s]", text);	\
        }														\
    } while (0)
#else
# define TRACESUBSYSTEMINITIALIZATION(text)
#endif

#endif // OPTIONS_HPP_


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
