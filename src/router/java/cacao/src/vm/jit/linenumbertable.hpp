/* src/vm/jit/linenumbertable.hpp - linenumber table

   Copyright (C) 2007, 2008
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


#ifndef _LINENUMBERTABLE_HPP
#define _LINENUMBERTABLE_HPP

#include "config.h"

#include <stdint.h>
#include <functional>
#include <vector>

#include "toolbox/list.hpp"

#include "vm/method.hpp"

#include "vm/jit/code.hpp"

#include "vm/jit/ir/instruction.hpp"

struct codegendata;
struct instruction;

/**
 * Represents a Java line number.
 */
class Linenumber {
private:
	// TODO Add constants.
	/* -1......start of inlined body              */
	/* -2......end of inlined body                */
	/* <= -3...special entry with methodinfo *    */
	/* (see doc/inlining_stacktrace.txt)          */

	int32_t _linenumber;
	void*   _pc;

public:
	Linenumber(int32_t linenumber, void* pc) : _linenumber(linenumber), _pc(pc) {}

	inline int32_t get_linenumber() const { return _linenumber; }
	inline void*   get_pc        () const { return _pc; }

	void resolve(const codeinfo* code);
};


/**
 * Unary function to resolve Linenumber objects.
 */
class LinenumberResolver : public std::binary_function<Linenumber, codeinfo*, void> {
public:
	// Unary resolve function.
	void operator() (Linenumber& ln, const codeinfo* code) const
	{
		ln.resolve(code);
	}
};


/**
 * Linenumber table of a Java method.
 */
class LinenumberTable {
private:
	std::vector<Linenumber> _linenumbers;

	// Comparator class.
	class comparator : public std::binary_function<Linenumber, void*, bool> {
	public:
		bool operator() (const Linenumber& ln, const void* pc) const
		{
			return (pc >= ln.get_pc());
		}
	};

public:
	LinenumberTable(jitdata* jd);
	~LinenumberTable();

	int32_t find(methodinfo **pm, void* pc);
};

void linenumbertable_list_entry_add(codegendata *cd, int32_t linenumber);
void linenumbertable_list_entry_add_inline_start(codegendata *cd, instruction *iptr);
void linenumbertable_list_entry_add_inline_end(codegendata *cd, instruction *iptr);

#endif // _LINENUMBERTABLE_HPP


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
