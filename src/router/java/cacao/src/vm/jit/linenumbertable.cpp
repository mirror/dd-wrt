/* src/vm/jit/linenumbertable.cpp - linenumber handling stuff

   Copyright (C) 2007-2013
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

#include "vm/jit/linenumbertable.hpp"
#include <assert.h>                     // for assert
#include <stdint.h>                     // for int32_t, uintptr_t
#include <algorithm>                    // for find_if, for_each
#include <list>                         // for _List_iterator
#include "config.h"                     // for ENABLE_STATISTICS
#include "toolbox/list.hpp"             // for DumpList
#include "vm/jit/code.hpp"              // for codeinfo
#include "vm/jit/codegen-common.hpp"    // for codegendata
#include "vm/jit/ir/instruction.hpp"    // for insinfo_inline, instruction, etc
#include "vm/jit/jit.hpp"               // for jitdata
#include "vm/os.hpp"                    // for os
#include "vm/statistics.hpp"            // for StatVar

#if defined(__S390__)
#  define ADDR_MASK(type, x) ((type)((uintptr_t)(x) & 0x7FFFFFFF))
#else
#  define ADDR_MASK(type, x) (x)
#endif

STAT_DECLARE_VAR(int,count_linenumbertable,0)
STAT_DECLARE_VAR(int,size_linenumbertable,0)

/**
 * Resolve the linenumber.
 *
 * If the entry contains an mcode pointer (normal case), resolve it
 * (see doc/inlining_stacktrace.txt for details).
 *
 * @param code Code structure.
 */
void Linenumber::resolve(const codeinfo* code)
{
	void* pv = ADDR_MASK(void*, code->entrypoint);

	// TODO Use constant.
	if (_linenumber >= -2)
		_pc = (void*) ((uintptr_t) pv + (uintptr_t) _pc);
}


/**
 * Creates a linenumber table.
 *
 * We allocate an array and store the linenumber entry in
 * reverse-order, so we can search the correct linenumber more easily.
 *
 * @param jd JIT data.
 */
LinenumberTable::LinenumberTable(jitdata* jd) : _linenumbers(jd->cd->linenumbers->begin(), jd->cd->linenumbers->end())
{
	// Get required compiler data.
	codeinfo* code = jd->code;

	STATISTICS(count_linenumbertable++);
	STATISTICS(size_linenumbertable +=
		sizeof(LinenumberTable) +
		sizeof(Linenumber) * _linenumbers.size());

	// Resolve all linenumbers in the vector.
	(void) for_each(_linenumbers.begin(), _linenumbers.end(), std::bind2nd(LinenumberResolver(), code));
}


/**
 * Search the the line number table for the line corresponding to a
 * given program counter.
 *
 * @param pc Program counter.
 *
 * @return Line number.
 */
int32_t LinenumberTable::find(methodinfo **pm, void* pc)
{
	void* maskpc = ADDR_MASK(void*, pc);

	std::vector<Linenumber>::iterator it = find_if(_linenumbers.begin(), _linenumbers.end(), std::bind2nd(comparator(), maskpc));

	// No matching entry found.
	if (it == _linenumbers.end())
		return 0;

	Linenumber& ln = *it;
	int32_t linenumber = ln.get_linenumber();

	// Check for linenumber entry type.
	if (linenumber < 0) {
		os::abort("FIX ME!");

#if 0
		// We found a special inline entry (see
		// doc/inlining_stacktrace.txt for details).
		switch (linenumber) {
		case -1:
			// Begin of an inlined method (ie. INLINE_END instruction.
			lntinline = --lnte;            /* get entry with methodinfo * */
			lnte--;                        /* skip the special entry      */

			/* search inside the inlined method */

			if (linenumbertable_linenumber_for_pc_intern(pm, lnte, lntsize, pc)) {
				/* the inlined method contained the pc */

				*pm = (methodinfo *) lntinline->pc;

				assert(lntinline->linenumber <= -3);

				return (-3) - lntinline->linenumber;
			}

			/* pc was not in inlined method, continue search.
			   Entries inside the inlined method will be skipped
			   because their lntentry->pc is higher than pc.  */
			break;

		case -2: 
			/* end of inlined method */

			return 0;

			/* default: is only reached for an -3-line entry after
			   a skipped -2 entry. We can safely ignore it and
			   continue searching.  */
		}
#endif
	}

	// Normal linenumber entry, return it.
	return linenumber;
}


/* linenumbertable_list_entry_add **********************************************

   Add a line number reference.

   IN:
      cd.............current codegen data
      linenumber.....number of line that starts with the given mcodeptr

*******************************************************************************/

void linenumbertable_list_entry_add(codegendata *cd, int32_t linenumber)
{
	void* pc = (void*) (cd->mcodeptr - cd->mcodebase);
	Linenumber ln(linenumber, pc);

	cd->linenumbers->push_front(ln);
}


/* linenumbertable_list_entry_add_inline_start *********************************

   Add a marker to the line number table indicating the start of an
   inlined method body. (see doc/inlining_stacktrace.txt)

   IN:
      cd ..... current codegen data
      iptr ... the ICMD_INLINE_BODY instruction

*******************************************************************************/

void linenumbertable_list_entry_add_inline_start(codegendata *cd, instruction *iptr)
{
	void* pc = (void*) (cd->mcodeptr - cd->mcodebase);

	Linenumber ln(-2 /* marks start of inlined method */, pc);

	cd->linenumbers->push_front(ln);

	insinfo_inline* insinfo = iptr->sx.s23.s3.inlineinfo;
	insinfo->startmpc = (int32_t) (uintptr_t) pc; /* store for corresponding INLINE_END */
}


/* linenumbertable_list_entry_add_inline_end ***********************************

   Add a marker to the line number table indicating the end of an
   inlined method body. (see doc/inlining_stacktrace.txt)

   IN:
      cd ..... current codegen data
      iptr ... the ICMD_INLINE_END instruction

   Note:
      iptr->method must point to the inlined callee.

*******************************************************************************/

void linenumbertable_list_entry_add_inline_end(codegendata *cd, instruction *iptr)
{
	insinfo_inline* insinfo = iptr->sx.s23.s3.inlineinfo;

	// Sanity check.
	assert(insinfo);

	// Special entry containing the methodinfo.
	Linenumber ln((-3) - iptr->line, insinfo->method);

	cd->linenumbers->push_front(ln);

	// End marker with PC of start of body.
	Linenumber lne(-1, (void*) (std::ptrdiff_t)insinfo->startmpc);

	cd->linenumbers->push_front(lne);
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
