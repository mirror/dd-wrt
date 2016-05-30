/* src/native/vm/openjdk/management.hpp - HotSpot management interface functions

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


#ifndef _MANAGEMENT_HPP
#define _MANAGEMENT_HPP

// Include our JNI header before the JMM header, because the JMM
// header include jni.h and we want to override the typedefs in jni.h.
#include "native/jni.hpp"

#include INCLUDE_JMM_H

/**
 * Management support.
 */
class Management {
private:
	jmmOptionalSupport _optional_support;

public:
	Management();

	static void* get_jmm_interface(int version);

	const jmmOptionalSupport& get_optional_support() const;
};


#endif // _MANAGEMENT_HPP


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
