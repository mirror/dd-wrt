/* src/mm/gc-boehm.hpp - include header for boehm gc

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


#ifndef _GC_BOEHM_HPP
#define _GC_BOEHM_HPP

#if defined(ENABLE_GC_BOEHM)
# if defined(__LINUX__)
#  define GC_LINUX_THREADS
# elif defined(__IRIX__)
#  define GC_IRIX_THREADS
# elif defined(__DARWIN__)
#  define GC_DARWIN_THREADS
# elif defined(__SOLARIS__)
#  define GC_SOLARIS_THREADS
# endif
# include "mm/boehm-gc/include/gc.h"
#endif

#endif // _GC_BOEHM_HPP


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
