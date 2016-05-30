/* src/future/memory.hpp - future memory library features

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


#ifndef FUTURE_MEMORY_HPP_
#define FUTURE_MEMORY_HPP_ 1

#include "config.h"

// get shared_ptr
#if HAVE_STD_TR1_SHARED_PTR

#include <tr1/memory>

namespace cacao {
using std::tr1::shared_ptr;
} // end namespace cacao

#elif HAVE_BOOST_SHARED_PTR

#include <boost/shared_ptr.hpp>

namespace cacao {
using boost::shared_ptr;
} // end namespace cacao

#else
#error "No implementation of shared_ptr available"
#endif


#endif /* FUTURE_MEMORY_HPP_ */

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
