/* src/vm/jit/optimizing/recompiler.cpp - recompilation system

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

#include <assert.h>

#include "threads/condition.hpp"
#include "threads/mutex.hpp"
#include "threads/thread.hpp"

#include "vm/classcache.hpp"
#include "vm/exceptions.hpp"
#include "vm/options.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/jit/code.hpp"
#include "vm/jit/jit.hpp"

#include "vm/jit/optimizing/recompiler.hpp"


/**
 * Stop the worker thread.
 */
Recompiler::~Recompiler()
{
	// Set the running flag to false.
	_run = false;

	// Now signal the worker thread.
	_cond.signal();

	// TODO We should wait here until the thread exits.
}


/* recompile_replace_vftbl *****************************************************

   XXX

*******************************************************************************/

static void recompile_replace_vftbl(methodinfo *m)
{
	codeinfo               *code;
	codeinfo               *pcode;
	u4                      slot;
	classcache_name_entry  *nmen;
	classcache_class_entry *clsen;
	classinfo              *c;
	vftbl_t                *vftbl;
	s4                      i;

	/* get current and previous codeinfo structure */

	code  = m->code;
	pcode = code->prev;

	assert(pcode);

	/* iterate over all classes */

	for (slot = 0; slot < hashtable_classcache.size; slot++) {
		nmen = (classcache_name_entry *) hashtable_classcache.ptr[slot];

		for (; nmen; nmen = nmen->hashlink) {
			/* iterate over all class entries */

			for (clsen = nmen->classes; clsen; clsen = clsen->next) {
				c = clsen->classobj;

				if (c == NULL)
					continue;

				/* Search for entrypoint of the previous codeinfo in
				   the vftbl and replace it with the current one. */

				vftbl = c->vftbl;

				/* Is the class linked? Means, is the vftbl finished? */

				if (!(c->state & CLASS_LINKED))
					continue;

				/* Does the class have a vftbl? Some internal classes
				   (e.g. $NEW$) are linked, but do not have a
				   vftbl. */

				if (vftbl == NULL)
					continue;

				for (i = 0; i < vftbl->vftbllength; i++) {
					if (vftbl->table[i] == pcode->entrypoint) {
#if !defined(NDEBUG)
						printf("replacing vftbl in: ");
						class_println(c);
#endif
						vftbl->table[i] = code->entrypoint;
					}
				}
			}
		}
	}
}


/**
 * The actual recompilation thread.
 */
void Recompiler::thread()
{
	// FIXME This just works for one recompiler.
	Recompiler& r = VM::get_current()->get_recompiler();

	while (r._run == true) {
		// Enter the recompile mutex, so we can call wait.
		r._mutex.lock();

		// Wait forever on that condition until we are signaled.
		r._cond.wait(r._mutex);

		// Leave the mutex.
		r._mutex.unlock();

		// FIXME Move this into the for loop.
		if (r._run == false)
			break;

		// Get the next method form the queue and recompile it.
		while (r._methods.empty() == false) {
			methodinfo* m = r._methods.front();

			// Recompile this method.
			if (jit_recompile(m) != NULL) {
				// Replace in vftbl's.
				recompile_replace_vftbl(m);
			}
			else {
				// XXX What is the right-thing(tm) to do here?
				exceptions_print_current_exception();
			}

			// Remove the method from the queue.
			r._methods.pop();
		}
	}
}


/**
 * Start the recompilation thread.
 *
 * @return true on success, false otherwise.
 */
bool Recompiler::start()
{
	Utf8String name = Utf8String::from_utf8("Recompiler");

	if (!threads_thread_start_internal(name, (functionptr) &Recompiler::thread))
		return false;

	return true;
}


/**
 * Add a method to the recompilation queue and signal the
 * recompilation thread that there is some work to do.
 *
 * @param m Method to recompile.
 */
void Recompiler::queue_method(methodinfo *m)
{
	// Add the method to the queue.
	_methods.push(m);

	// Enter the recompile mutex, so we can call notify.
	_mutex.lock();

	// Signal the recompiler thread.
	_cond.signal();

	// Leave the mutex.
	_mutex.unlock();
}



// Legacy C interface.
void Recompiler_queue_method(methodinfo* m) { VM::get_current()->get_recompiler().queue_method(m); }

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
