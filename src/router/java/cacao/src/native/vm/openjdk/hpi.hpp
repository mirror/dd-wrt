/* src/native/vm/openjdk/hpi.hpp - HotSpot HPI interface functions

   Copyright (C) 2008
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


#ifndef _HPI_HPP
#define _HPI_HPP

#include "config.h"

/* HPI headers *****************************************************************

   We include hpi_md.h before hpi.h as the latter includes the former.

   These includes define:

   #define _JAVASOFT_HPI_MD_H_
   #define _JAVASOFT_HPI_H_

*******************************************************************************/

// Include our JNI header before the HPI headers, because the HPI
// headers include jni.h and we want to override the typedefs in
// jni.h.
#include "native/jni.hpp"

#include INCLUDE_HPI_MD_H
#include INCLUDE_HPI_H

/**
 * Host Porting Interface (HPI).
 */
class HPI {
private:
	GetInterfaceFunc      _get_interface;
	HPI_FileInterface*    _file;
	HPI_SocketInterface*  _socket;
	HPI_LibraryInterface* _library;
	HPI_SystemInterface*  _system;

public:	
	HPI();

	inline HPI_FileInterface&    get_file   () const { return *_file; }
	inline HPI_SocketInterface&  get_socket () const { return *_socket; }
	inline HPI_LibraryInterface& get_library() const { return *_library; }
	inline HPI_SystemInterface&  get_system () const { return *_system; }
	
	void initialize(); // REMOVEME
	int initialize_socket_library();
};

#endif // _HPI_HPP


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
