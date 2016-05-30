/* src/mm/codememory.hpp - code memory management

   Copyright (C) 2007-2013
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


#ifndef CODEMEMORY_HPP_
#define CODEMEMORY_HPP_ 1

#include <stddef.h>                     // for size_t


/* convenience macros *********************************************************/

#define CNEW(type,num)        ((type *) codememory_get(sizeof(type) * (num)))
#define CFREE(ptr,num)        codememory_release((ptr),(num))


/* function prototypes ********************************************************/

void  codememory_init(void);

void *codememory_get(size_t size);
void  codememory_release(void *p, size_t size);

#endif // CODEMEMORY_HPP_


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
