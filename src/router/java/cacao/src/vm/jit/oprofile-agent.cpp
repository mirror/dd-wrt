/* src/vm/jit/oprofile-agent.cpp - oprofile agent implementation

   Copyright (C) 2008-2013
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

#include "vm/jit/code.hpp"
#include "vm/jit/oprofile-agent.hpp"

#include "toolbox/buffer.hpp"

#include <string.h>

/* static fields **************************************************************/
op_agent_t OprofileAgent::_handle = 0;

/**
 * Initializes the OprofileAgent system.
 *
 */
/* void OprofileAgent_initialize() */
void OprofileAgent::initialize(void)
{
	_handle = op_open_agent();
	if (!_handle)
		os::abort_errno("unable to open opagent handle");
}

/**
 * Reports the given method to oprofile.
 *
 * This has to be done once per JIT compilation step for a specific method.
 *
 * @param m Method to register.
 */
/* void OprofileAgent_newmethod(methodinfo *m) */
void OprofileAgent::newmethod(methodinfo *m)
{
	unsigned int real_length = (unsigned int) m->code->mcodelength -
		(unsigned int) (m->code->entrypoint - m->code->mcode);

	size_t len = Utf8String(m->clazz->name) + strlen(".")
	           + Utf8String(m->name)        + Utf8String(m->descriptor)
	           + strlen("0");

	// can the buffer free it's contents or not?
	Buffer<> buf(len, false);

	buf.write_slash_to_dot(m->clazz->name)
           .write('.')
           .write(m->name)
           .write(m->descriptor)

	if (_handle)
		op_write_native_code(_handle, (char*) buf,
			(uint64_t) (ptrint) m->code->entrypoint,
			(const void *) m->code->entrypoint,
			real_length);
}

/**
 * Shuts down the OprofileAgent system.
 *
 */
/* void OprofileAgent_close() */
void OprofileAgent::close()
{
	if (_handle)
		op_close_agent(_handle);

	_handle = 0;
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
 */
