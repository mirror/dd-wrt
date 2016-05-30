/* src/vm/jit/stubs.cpp - JIT stubs

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2008 Theobroma Systems Ltd.

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

#include "vm/jit/stubs.hpp"
#include <stdint.h>                     // for uint8_t
#include "arch.hpp"                     // for JIT_COMPILER_VIA_SIGNAL
#include "config.h"                     // for ENABLE_JIT
#include "md-stubs.hpp"                 // for CompilerStub::get_code_size
#include "md.hpp"                       // for md_cacheflush
#include "mm/codememory.hpp"
#include "mm/dumpmemory.hpp"            // for DumpMemory, DumpMemoryArea
#include "vm/descriptor.hpp"            // for methoddesc, typedesc, etc
#include "vm/jit/abi.hpp"               // for md_param_alloc_native
#include "vm/jit/builtin.hpp"           // for builtintable_entry
#include "vm/jit/code.hpp"              // for code_unflag_leafmethod, etc
#include "vm/jit/codegen-common.hpp"    // for codegen_emit_stub_native, etc
#include "vm/jit/disass.hpp"
#include "vm/jit/dseg.hpp"
#include "vm/jit/emit-common.hpp"       // for emit_trap_compiler
#include "vm/jit/jit.hpp"               // for jitdata, jit_jitdata_new
#include "vm/jit/reg.hpp"               // for reg_setup
#include "vm/jit/show.hpp"
#include "vm/method.hpp"                // for methodinfo
#include "vm/options.hpp"               // for opt_verbosecall
#include "vm/statistics.hpp"            // for StatVar
#include "vm/types.hpp"                 // for ptrint, u1

STAT_DECLARE_VAR(int,count_cstub_len,0)
STAT_DECLARE_VAR(int,size_stub_native,0)

/**
 * Wrapper for codegen_emit_stub_compiler.
 *
 * @param m Method object.
 *
 * @return Pointer to the compiler stub code.
 */
void* CompilerStub::generate(methodinfo *m)
{
	jitdata     *jd;
	codegendata *cd;
	ptrint      *d;                     /* pointer to data memory             */
	u1          *c;                     /* pointer to code memory             */

	// Create new dump memory area.
	DumpMemoryArea dma;

	/* allocate required data structures */

	jd = (jitdata*) DumpMemory::allocate(sizeof(jitdata));

	jd->m     = m;
	jd->cd    = (codegendata*) DumpMemory::allocate(sizeof(codegendata));
	jd->flags = 0;

	/* get required compiler data */

	cd = jd->cd;

#if !defined(JIT_COMPILER_VIA_SIGNAL)
	/* allocate code memory */

	c = CNEW(u1, 3 * SIZEOF_VOID_P + get_code_size());

	/* set pointers correctly */

	d = (ptrint *) c;

	cd->mcodebase = c;

	c = c + 3 * SIZEOF_VOID_P;
	cd->mcodeptr = c;

	/* NOTE: The codeinfo pointer is actually a pointer to the
	   methodinfo (this fakes a codeinfo structure). */

	d[0] = (ptrint) asm_call_jit_compiler;
	d[1] = (ptrint) m;
	d[2] = (ptrint) &d[1];                                    /* fake code->m */

	/* call the emit function */

	codegen_emit_stub_compiler(jd);

	STATISTICS(count_cstub_len += 3 * SIZEOF_VOID_P + get_code_size());

	/* flush caches */

	md_cacheflush(cd->mcodebase, 3 * SIZEOF_VOID_P + get_code_size());
#else
	/* Allocate code memory. */

	c = CNEW(uint8_t, 2 * SIZEOF_VOID_P + get_code_size());

	/* Set pointers correctly. */

	d = (ptrint *) c;

	cd->mcodebase = c;

	c = c + 2 * SIZEOF_VOID_P;
	cd->mcodeptr = c;

	/* NOTE: The codeinfo pointer is actually a pointer to the
	   methodinfo (this fakes a codeinfo structure). */

	d[0] = (ptrint) m;
	d[1] = (ptrint) &d[0];                                    /* fake code->m */

	/* Emit the trap instruction. */

	emit_trap_compiler(cd);

	STATISTICS(count_cstub_len += 2 * SIZEOF_VOID_P + get_code_size());

	/* Flush caches. */

	md_cacheflush(cd->mcodebase, 2 * SIZEOF_VOID_P + get_code_size());
#endif

	/* return native stub code */

	return c;
}


/**
 * Free a compiler stub from memory.
 *
 * @param 
 */
void CompilerStub::remove(void* stub)
{
	// Pass size 1 to keep the intern function happy.
	CFREE(stub, 1);
}


/* codegen_disassemble_nativestub **********************************************

   Disassembles the generated builtin or native stub.

*******************************************************************************/

#if !defined(NDEBUG) && defined(ENABLE_DISASSEMBLER)
static void codegen_disassemble_stub(methodinfo *m, u1 *start, u1 *end)
{
	printf("Stub code: ");
	if (m->clazz != NULL)
		utf_fprint_printable_ascii_classname(stdout, m->clazz->name);
	else
		printf("NULL");
	printf(".");
	utf_fprint_printable_ascii(stdout, m->name);
	utf_fprint_printable_ascii(stdout, m->descriptor);
	printf("\nLength: %d\n\n", (s4) (end - start));

	DISASSEMBLE(start, end);
}
#endif


/**
 * Wrapper for codegen_emit_stub_native.
 *
 * @param m   Method object.
 * @param bte Builtin function structure.
 */
void BuiltinStub::generate(methodinfo* m, builtintable_entry* bte)
{
	jitdata  *jd;
	codeinfo *code;
	int       skipparams;

	// Create new dump memory area.
	DumpMemoryArea dma;

	/* Create JIT data structure. */

	jd = jit_jitdata_new(m);

	/* Get required compiler data. */

	code = jd->code;

	/* Stubs are non-leaf methods. */

	code_unflag_leafmethod(code);

	/* setup code generation stuff */

	codegen_setup(jd);

	/* Set the number of native arguments we need to skip. */

	skipparams = 0;

	/* generate the code */

#if defined(ENABLE_JIT)
# if defined(ENABLE_INTRP)
	if (!opt_intrp) {
# endif
		assert(bte->fp != NULL);
		codegen_emit_stub_native(jd, bte->md, bte->fp, skipparams);
# if defined(ENABLE_INTRP)
	}
# endif
#endif

	/* reallocate the memory and finish the code generation */

	codegen_finish(jd);

	/* set the stub entry point in the builtin table */

	bte->stub = code->entrypoint;

	STATISTICS(size_stub_native += code->mcodelength);

#if !defined(NDEBUG) && defined(ENABLE_DISASSEMBLER)
	/* disassemble native stub */

	if (opt_DisassembleStubs) {
		codegen_disassemble_stub(m,
								 (u1 *) (ptrint) code->entrypoint,
								 (u1 *) (ptrint) code->entrypoint + (code->mcodelength - jd->cd->dseglen));

		/* show data segment */

		if (opt_showddatasegment)
			dseg_display(jd);
	}
#endif /* !defined(NDEBUG) && defined(ENABLE_DISASSEMBLER) */
}


/**
 * Wrapper for codegen_emit_stub_native.
 *
 * @param m Method object of the native function.
 * @param f Native function pointer.
 *
 * @return The codeinfo representing the stub code.
 */
codeinfo* NativeStub::generate(methodinfo* m, functionptr f)
{
	jitdata     *jd;
	codeinfo    *code;
	methoddesc  *md;
	methoddesc  *nmd;	
	int          skipparams;

	// Create new dump memory area.
	DumpMemoryArea dma;

	/* Create JIT data structure. */

	jd = jit_jitdata_new(m);

	/* Get required compiler data. */

	code = jd->code;

	/* Stubs are non-leaf methods. */

	code_unflag_leafmethod(code);

	/* set the flags for the current JIT run */

#if defined(ENABLE_PROFILING)
	if (opt_prof)
		jd->flags |= JITDATA_FLAG_INSTRUMENT;
#endif

	if (opt_verbosecall)
		jd->flags |= JITDATA_FLAG_VERBOSECALL;

	/* setup code generation stuff */

#if defined(ENABLE_JIT)
# if defined(ENABLE_INTRP)
	if (!opt_intrp)
# endif
		reg_setup(jd);
#endif

	codegen_setup(jd);

	/* create new method descriptor with additional native parameters */

	md = m->parseddesc;

	/* Set the number of native arguments we need to skip. */

	if (m->flags & ACC_STATIC)
		skipparams = 2;
	else
		skipparams = 1;
	
	nmd = (methoddesc*) DumpMemory::allocate(sizeof(methoddesc) - sizeof(typedesc) +
											 md->paramcount * sizeof(typedesc) +
											 skipparams * sizeof(typedesc));

	nmd->paramcount = md->paramcount + skipparams;

	nmd->params = (paramdesc*) DumpMemory::allocate(sizeof(paramdesc) * nmd->paramcount);

	nmd->paramtypes[0].type = TYPE_ADR; /* add environment pointer            */

	if (m->flags & ACC_STATIC)
		nmd->paramtypes[1].type = TYPE_ADR; /* add class pointer              */

	MCOPY(nmd->paramtypes + skipparams, md->paramtypes, typedesc,
		  md->paramcount);

#if defined(ENABLE_JIT)
# if defined(ENABLE_INTRP)
	if (!opt_intrp)
# endif
		/* pre-allocate the arguments for the native ABI */

		md_param_alloc_native(nmd);
#endif

	/* generate the code */

#if defined(ENABLE_JIT)
# if defined(ENABLE_INTRP)
	if (opt_intrp)
		intrp_createnativestub(f, jd, nmd);
	else
# endif
		codegen_emit_stub_native(jd, nmd, f, skipparams);
#else
	intrp_createnativestub(f, jd, nmd);
#endif

	/* reallocate the memory and finish the code generation */

	codegen_finish(jd);

	/* must be done after codegen_finish() */
	STATISTICS(size_stub_native += code->mcodelength);

#if !defined(NDEBUG) && defined(ENABLE_DISASSEMBLER)
	/* disassemble native stub */

	if (opt_DisassembleStubs) {
# if defined(ENABLE_DEBUG_FILTER)
		if (m->filtermatches & SHOW_FILTER_FLAG_SHOW_METHOD)
# endif
		{
			codegen_disassemble_stub(m,
									 (u1 *) (ptrint) code->entrypoint,
									 (u1 *) (ptrint) code->entrypoint + (code->mcodelength - jd->cd->dseglen));

			/* show data segment */

			if (opt_showddatasegment)
				dseg_display(jd);
		}
	}
#endif /* !defined(NDEBUG) && defined(ENABLE_DISASSEMBLER) */

	/* return native stub code */

	return code;
}


/**
 * Free a native stub from memory.
 *
 * @param stub Pointer to stub memory.
 */    
void NativeStub::remove(void* stub)
{
	// Pass size 1 to keep the intern function happy.
	CFREE(stub, 1);
}


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
