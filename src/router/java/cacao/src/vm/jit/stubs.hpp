/* src/vm/jit/stubs.hpp - JIT stubs

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


#ifndef _STUBS_HPP
#define _STUBS_HPP

#include "config.h"
#include "vm/global.hpp"                // for functionptr

struct builtintable_entry;
struct codeinfo;
struct methodinfo;


/**
 * Class for compiler stub generation.
 */
class CompilerStub {
public:
	static inline int get_code_size();

	static void* generate(methodinfo* m);
	static void  remove(void* stub);
};


/**
 * Class for builtin stub generation.
 */
class BuiltinStub {
public:
	static void generate(methodinfo* m, builtintable_entry* bte);
};


/**
 * Class for native stub generation.
 */
class NativeStub {
public:
	static codeinfo* generate(methodinfo* m, functionptr f);
	static void      remove(void* stub);
};


// Include machine dependent implementation.
#include "md-stubs.hpp"

#endif // _STUBS_HPP


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
