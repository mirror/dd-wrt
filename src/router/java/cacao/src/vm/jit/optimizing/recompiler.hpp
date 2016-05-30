/* src/vm/jit/optimizing/recompiler.hpp - recompilation system

   Copyright (C) 1996-2005, 2006, 2007, 2008
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


#ifndef _RECOMPILER_HPP
#define _RECOMPILER_HPP

#include "config.h"
#include <queue>
#include "threads/condition.hpp"
#include "threads/mutex.hpp"

struct methodinfo;

/**
 * Thread for JIT recompilations.
 */
class Recompiler {
private:
	Mutex                   _mutex;
	Condition               _cond;
	std::queue<methodinfo*> _methods;
	bool                    _run;       ///< Flag to stop worker thread.

	static void thread();               ///< Worker thread.

public:
	Recompiler() : _run(true) {}
	~Recompiler();

	bool start();                       ///< Start the worker thread.
	void queue_method(methodinfo* m);   ///< Queue a method for recompilation.
};


/* list_method_entry **********************************************************/

struct list_method_entry {
   methodinfo *m;
// listnode_t  linkage;
};


/* function prototypes ********************************************************/

void Recompiler_queue_method(methodinfo *m);

#endif // _RECOMPILER_HPP


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
