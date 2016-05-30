/* src/vm/package.cpp - Java boot-package functions

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

#include "mm/dumpmemory.hpp"
#include "toolbox/logging.hpp"
#include "vm/options.hpp"
#include "vm/package.hpp"
#include "vm/utf8.hpp"

#include <set>

// Package list.

Mutex Package::_mutex;

static std::set<Utf8String>& packages() {
	static std::set<Utf8String> _packages;

	return _packages;
}

/**
 * Add a package to the boot-package list.
 *
 * @param packagename Package name as Java string.
 */
void Package::add(Utf8String packagename)
{
	MutexLocker lock(_mutex);

#if !defined(NDEBUG)
	if (opt_DebugPackage) {
		log_start();
		log_print("[package_add: packagename=");
		utf_display_printable_ascii(packagename);
		log_print("]");
		log_finish();
	}
#endif

	// Add the package name.
	::packages().insert(packagename);
}


/**
 * Find a package in the list.
 *
 * @param packagename Package name as Java string.
 *
 * @return Package name as Java string.
 */
Utf8String Package::find(Utf8String packagename)
{
	MutexLocker lock(_mutex);

	std::set<Utf8String>&          pkgs = ::packages();
	std::set<Utf8String>::iterator it   = pkgs.find(packagename);

	if (it == pkgs.end())
		return NULL;

	return *it;
}

const std::set<Utf8String> &Package::packages()
{
	return ::packages();
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
