/* src/toolbox/Option.hpp - Command line option parsing library

   Copyright (C) 1996-2014
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

#ifndef TOOLBOX_OPTION_HPP_
#define TOOLBOX_OPTION_HPP_

#include "config.h"
#include <cstring>
#include <cassert>
#include <set>
#include <cstdio>

/**
 * @file
 *
 * This file contains the command line option parsing library.
 *
 * @ingroup cmd-option
 */

/**
 * @defgroup cmd-option Command line option
 * Command line option parsing library.
 *
 * The aim of this module is to provide a decentralized, type-safe library for
 * command line options.
 *
 * Goals:
 * - Type-safe: no casting required
 * - Decentralized: Feature specific options can be defined in the feature
 *   implementation files. No global, shared data structures need changes.
 * - Support for common types: Ints, strings, enums.
 * - Namespace support: Options can be organized in namespaces.
 */
namespace cacao {

class OStream;
class OptionEntry;

class OptionPrefix {
public:
	typedef std::set<OptionEntry*> ChildSetTy;
	typedef ChildSetTy::iterator iterator;

	OptionPrefix(const char* name);

	const char* get_name() const {
		return name;
	}
	std::size_t size() const {
		return s;
	}
	iterator begin() { return children.begin(); }
	iterator end() { return children.end(); }
	void insert(OptionEntry* oe);
	#if 0
	{
		children.insert(oe);
	}
	#endif
private:
	const char* name;
	std::size_t s;
	ChildSetTy children;
};

class OptionParser {
public:
	static void print_usage(OptionPrefix& root, FILE* fp = stdout);
	static bool parse_option(OptionPrefix& root, const char* name, size_t name_len,
		const char* value, size_t value_len);
};

class OptionEntry {
public:
	OptionEntry(const char* name, const char* desc, OptionPrefix &parent)
			: name(name), name_size(std::strlen(name)), desc(desc) {
		parent.insert(this);
	}
	const char* get_name() const {
		return name;
	}
	std::size_t size() const {
		return name_size;
	}
	const char* get_desc() const {
		return desc;
	}
	virtual std::size_t print(OStream& OS) = 0;
	virtual bool parse(const char* value, std::size_t value_len) = 0;

	virtual ~OptionEntry() {} // make compiler happy
private:
	const char* name;
	std::size_t name_size;
	const char* desc;
};


template<class T>
class OptionBase : public OptionEntry {
public:
	OptionBase(const char* name, const char* desc, T value, OptionPrefix &parent)
		: OptionEntry(name, desc, parent), value(value) {}
	T get() { return value; }
	operator T() { return get(); }

	virtual std::size_t print(OStream& OS);
protected:
	void set_value(T v) {
		value = v;
	}
private:
	T value;
};

std::size_t option_print(OptionBase<bool>& option, OStream& OS);

std::size_t option_print(OptionEntry& option, OStream& OS);

template<class T>
inline std::size_t OptionBase<T>::print(OStream& OS) {
	return option_print(*this,OS);
}

template<class T>
class Option : public OptionBase<T> {
public:
	Option(const char* name, const char* desc, T value, OptionPrefix &parent)
		: OptionBase<T>(name,desc,value,parent) {}
	virtual bool parse(const char* value, std::size_t value_len);
};


namespace option {
OptionPrefix& root();
OptionPrefix& xx_root();
} // end namespace option

} // end namespace cacao

#endif // TOOLBOX_OPTION_HPP_

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
