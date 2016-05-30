/* src/vm/jit/optimizing/profile.cpp - runtime profiling

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


#include "config.h"

#include <assert.h>
#include <stdlib.h>

#include "vm/types.hpp"

#include "mm/memory.hpp"

#include "threads/threadlist.hpp"
#include "threads/thread.hpp"

#include "vm/class.hpp"
#include "vm/classcache.hpp"
#include "vm/method.hpp"
#include "vm/options.hpp"
#include "vm/string.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/jit/code.hpp"
#include "vm/jit/jit.hpp"
#include "vm/jit/methodheader.hpp"
#include "vm/jit/methodtree.hpp"

#include "vm/jit/optimizing/recompiler.hpp"


/* profile_init ****************************************************************

   Initializes the profile global lock.

*******************************************************************************/

bool profile_init(void)
{
	/* everything's ok */

	return true;
}


/* profile_thread **************************************************************

   XXX

*******************************************************************************/

static s4 runs = 0;
static s4 hits = 0;
static s4 misses = 0;

static void profile_thread(void)
{
	s4            nanos;
#if 0
	threadobject *t;
	u1           *pc;
	u1           *pv;
	methodinfo   *m;
	codeinfo     *code;
#endif

	while (true) {
		/* sleep thread for 0.5-1.0 ms */

		nanos = 500 + (int) (500.0 * (rand() / (RAND_MAX + 1.0)));
/* 		fprintf(stderr, "%d\n", nanos); */

		threads_sleep(0, nanos);
		runs++;

#if 0
		// Lock the thread lists.
		ThreadList::get()->lock();

		/* iterate over all started threads */

		for (t = ThreadList_first(); t != NULL; t = ThreadList_next(t)) {
			/* is this a Java thread? */

			if (!(t->flags & THREAD_FLAG_JAVA))
				continue;

			/* send SIGUSR2 to thread to get the current PC */
			/* XXX write a threads-function for that */

			pthread_kill(t->impl.tid, SIGUSR2);

			/* the thread object now contains the current thread PC */

			pc = t->pc;

			/* Get the PV for the current PC. */

			pv = methodtree_find_nocheck(pc);

			/* get methodinfo pointer from data segment */

			if (pv == NULL) {
				misses++;
			}
			else {
				code = *((codeinfo **) (pv + CodeinfoPointer));

				/* For asm_vm_call_method the codeinfo pointer is NULL
				   (which is also in the method tree). */

				if (code != NULL) {
					m = code->m;

					/* native methods are never recompiled */

					if (!(m->flags & ACC_NATIVE)) {
						/* increase the method incovation counter */

						code->frequency++;
						hits++;

						if (code->frequency > 500) {
							/* clear frequency count before
							   recompilation */

							code->frequency = 0;

							/* add this method to the method list and
							   start recompilation */

							Recompiler_queue_method(m);
						}
					}
				}
			}
		}

		// Unlock the thread lists.
		ThreadList::get()->unlock();
#endif
	}
}

/* profile_start_thread ********************************************************

   Starts the profile sampling thread.

*******************************************************************************/

bool profile_start_thread(void)
{
	Utf8String name = Utf8String::from_utf8("Profiling Sampler");

	if (!threads_thread_start_internal(name, profile_thread))
		return false;

	/* everything's ok */

	return true;
}


/**
 * Comparison function used to sort a method list from higher to lower by
 * comparing the method call frequencies.
 *
 * @param m1 First method to be compared.
 * @param m2 Second method to be compared.
 * @return Returns true if the first method goes before the second method in
 * the specific order, and false otherwise.
 */
#if !defined(NDEBUG)
static bool profile_compare_frequency(methodinfo* m1, methodinfo* m2)
{
	return (m1->code->frequency > m2->code->frequency);
}
#endif


/**
 * Prints profiling statistics gathered during runtime.
 */
#if !defined(NDEBUG)
void profile_printstats(void)
{
	classinfo              *c;
	methodinfo             *m;
	codeinfo               *code;
	u4                      slot;
	classcache_name_entry  *nmen;
	classcache_class_entry *clsen;
	s4                      i;
	s4                      j;
	u4                      frequency;
	s8                      cycles;

	frequency = 0;
	cycles    = 0;

	/* create new method list */

	std::list<methodinfo*> l;
	//DumpList<methodinfo*> l; // XXX currently the DumpList doesn't work here.

	/* iterate through all classes and methods */

	for (slot = 0; slot < hashtable_classcache.size; slot++) {
		nmen = (classcache_name_entry *) hashtable_classcache.ptr[slot];

		for (; nmen; nmen = nmen->hashlink) {
			/* iterate over all class entries */

			for (clsen = nmen->classes; clsen; clsen = clsen->next) {
				c = clsen->classobj;

				if (c == NULL)
					continue;

				/* interate over all class methods */

				for (i = 0; i < c->methodscount; i++) {
					m = &(c->methods[i]);

					code = m->code;

					/* was this method actually called? */

					if ((code != NULL) && (code->frequency > 0)) {
						/* add to overall stats */

						frequency += code->frequency;
						cycles    += code->cycles;

						/* add new entry into method list */

						l.push_back(m);
					}
				}
			}
		}
	}

	/* sort the method list */

	l.sort(profile_compare_frequency);

	/* print all methods sorted */

	printf(" frequency     ratio         cycles     ratio   method name\n");
	printf("----------- --------- -------------- --------- -------------\n");

	/* now iterate through the list and print it */

	for (DumpList<methodinfo*>::iterator it = l.begin(); it != l.end(); ++it) {
		m = *(it);

		code = m->code;

		printf("%10d   %.5f   %12ld   %.5f   ",
			   code->frequency,
			   (double) code->frequency / (double) frequency,
			   (long) code->cycles,
			   (double) code->cycles / (double) cycles);

		method_println(m);

		/* print basic block frequencies */

		if (opt_prof_bb) {
			for (j = 0; j < code->basicblockcount; j++)
				printf("                                                    L%03d: %10d\n",
					   j, code->bbfrequency[j]);
		}
	}

	printf("-----------           -------------- \n");
	printf("%10d             %12ld\n", frequency, (long) cycles);

	printf("\nruns  : %10d\n", runs);
	printf("hits  : %10d\n", hits);
	printf("misses: %10d\n", misses);
}
#endif /* !defined(NDEBUG) */


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
