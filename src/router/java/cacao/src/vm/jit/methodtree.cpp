/* src/vm/jit/methodtree.cpp - AVL tree of methods

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


#include "config.h"

#include "mm/memory.hpp"

#include "toolbox/avl.hpp"
#include "toolbox/logging.hpp"

#include "vm/jit/asmpart.hpp"
#include "vm/jit/methodtree.hpp"
#include "vm/jit/stacktrace.hpp"


/* methodtree_element *********************************************************/

typedef struct methodtree_element_t methodtree_element_t;

struct methodtree_element_t {
	void *startpc;
	void *endpc;
};


/* in this tree we store all method addresses *********************************/

static avl_tree_t *methodtree = NULL;


/* static functions ***********************************************************/

static int methodtree_comparator(const void *treenode, const void *node);


/* methodtree_init *************************************************************

   Initialize the global method tree.

*******************************************************************************/

void methodtree_init(void)
{
#if defined(ENABLE_JIT)
	methodtree_element_t *mte;
#endif

	methodtree = avl_create(&methodtree_comparator);

#if defined(ENABLE_JIT)
	/* Insert asm_vm_call_method. */

	mte = NEW(methodtree_element_t);

	mte->startpc = (u1 *) (ptrint) asm_vm_call_method;
	mte->endpc   = (u1 *) (ptrint) asm_vm_call_method_end;

	avl_insert(methodtree, mte);
#endif
}


/* methodtree_comparator *******************************************************

   Comparator function used for the AVL tree of methods.

   ARGUMENTS:
       treenode ... the node from the tree
       node ....... the node to compare to the tree-node

   RETURN VALUE:
       0 .... found
       -1 ... go left
       1 .... go right

*******************************************************************************/

static int methodtree_comparator(const void *treenode, const void *node)
{
	methodtree_element_t *mte;
	methodtree_element_t *mtepc;

	mte   = (methodtree_element_t *) treenode;
	mtepc = (methodtree_element_t *) node;

	/* compare both startpc and endpc of pc, even if they have the same value,
	   otherwise the avl_probe sometimes thinks the element is already in the
	   tree */

#ifdef __S390__
	/* On S390 addresses are 31 bit. Compare only 31 bits of value.
	 */
#	define ADDR_MASK(a) ((a) & 0x7FFFFFFF)
#else
#	define ADDR_MASK(a) (a)
#endif

	if (ADDR_MASK((long) mte->startpc) <= ADDR_MASK((long) mtepc->startpc) &&
		ADDR_MASK((long) mtepc->startpc) <= ADDR_MASK((long) mte->endpc) &&
		ADDR_MASK((long) mte->startpc) <= ADDR_MASK((long) mtepc->endpc) &&
		ADDR_MASK((long) mtepc->endpc) <= ADDR_MASK((long) mte->endpc)) {
		return 0;

	} else if (ADDR_MASK((long) mtepc->startpc) < ADDR_MASK((long) mte->startpc)) {
		return -1;

	} else {
		return 1;
	}

#	undef ADDR_MASK
}


/* methodtree_insert ***********************************************************

   Insert the machine code range of a method into the AVL tree of
   methods.

   ARGUMENTS:
       startpc ... start address of the method
	   endpc ..... end address of the method

*******************************************************************************/

void methodtree_insert(void *startpc, void *endpc)
{
	methodtree_element_t *mte;

	/* Allocate new method entry. */

	mte = NEW(methodtree_element_t);

	mte->startpc = startpc;
	mte->endpc   = endpc;

	/* This function does not return an error, but asserts for
	   duplicate entries. */

	avl_insert(methodtree, mte);
}


/* methodtree_find *************************************************************

   Find the PV for the given PC by searching in the AVL tree of
   methods.

*******************************************************************************/

void *methodtree_find(void *pc)
{
	void *pv;

	// This flag indicates whether a methodtree lookup is failing. We need
	// to keep track of this to avoid endless loops during stacktrace creation.
	static bool methodtree_find_failing = false;

	/* Try to find a method. */

	pv = methodtree_find_nocheck(pc);

	if (pv == NULL) {
		/* No method was found.  Let's dump a stacktrace. */

		log_println("We received a SIGSEGV and tried to handle it, but we were");
		log_println("unable to find a Java method at:");
		log_println("");
#if SIZEOF_VOID_P == 8
		log_println("PC=0x%016lx", pc);
#else
		log_println("PC=0x%08x", pc);
#endif
		log_println("");

		// Detect and avoid endless loops.
		if (methodtree_find_failing)
			vm_abort("Exiting without stacktrace...");
		else
			methodtree_find_failing = true;

		// Actually try to dump a stacktrace.
		log_println("Dumping the current stacktrace:");
		stacktrace_print_current();

		vm_abort("Exiting...");
	}

	return pv;
}


/* methodtree_find_nocheck *****************************************************

   Find the PV for the given PC by searching in the AVL tree of
   methods.  This method does not check the return value and is used
   by the profiler.

*******************************************************************************/

void *methodtree_find_nocheck(void *pc)
{
	methodtree_element_t  mtepc;
	methodtree_element_t *mte;

	mtepc.startpc = pc;
	mtepc.endpc   = pc;

	mte = (methodtree_element_t*) avl_find(methodtree, &mtepc);

	if (mte == NULL)
		return NULL;
	else
		return mte->startpc;
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
