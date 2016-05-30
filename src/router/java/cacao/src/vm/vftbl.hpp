/* src/vm/vftbl.hpp - virtual function table

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


#ifndef VFTBL_HPP_
#define VFTBL_HPP_ 1

/* virtual function table ******************************************************

   The vtbl has a bidirectional layout with open ends at both sides.
   interfacetablelength gives the number of entries of the interface
   table at the start of the vftbl. The vftbl pointer points to
   &interfacetable[0].  vftbllength gives the number of entries of
   table at the end of the vftbl.

   runtime type check (checkcast):

   Different methods are used for runtime type check depending on the
   argument of checkcast/instanceof.
	
   A check against a class is implemented via relative numbering on
   the class hierachy tree. The tree is numbered in a depth first
   traversal setting the base field and the diff field. The diff field
   gets the result of (high - base) so that a range check can be
   implemented by an unsigned compare. A sub type test is done by
   checking the inclusion of base of the sub class in the range of the
   superclass.

   A check against an interface is implemented via the
   interfacevftbl. If the interfacevftbl contains a nonnull value a
   class is a subclass of this interface.

   interfacetable:

   Like standard virtual methods interface methods are called using
   virtual function tables. All interfaces are numbered sequentially
   (starting with zero). For each class there exist an interface table
   of virtual function tables for each implemented interface. The
   length of the interface table is determined by the highest number
   of an implemented interface.

   The following example assumes a class which implements interface 0 and 3:

   interfacetablelength = 4

                  | ...       |            +----------+
                  +-----------+            | method 2 |---> method z
                  | class     |            | method 1 |---> method y
                  +-----------+            | method 0 |---> method x
                  | ivftbl  0 |----------> +----------+
    vftblptr ---> +-----------+
                  | ivftbl -1 |--> NULL    +----------+
                  | ivftbl -2 |--> NULL    | method 1 |---> method x
                  | ivftbl -3 |-----+      | method 0 |---> method a
                  +-----------+     +----> +----------+
     
                              +---------------+
                              | length 3 = 2  |
                              | length 2 = 0  |
                              | length 1 = 0  |
                              | length 0 = 3  |
    interfacevftbllength ---> +---------------+

*******************************************************************************/

// Includes.
#include "arch.hpp"        // for USES_NEW_SUBTYPE
#include "vm/global.hpp"   // for methodptr

#if USES_NEW_SUBTYPE
#define DISPLAY_SIZE 4
#endif

struct classinfo;
struct arraydescriptor;

struct vftbl_t {
	methodptr              *interfacetable[1];    /* interface table (access via macro)  */
   classinfo              *clazz;                /* class, the vtbl belongs to          */
   arraydescriptor        *arraydesc;            /* for array classes, otherwise NULL   */
	s4                      vftbllength;          /* virtual function table length       */
	s4                      interfacetablelength; /* interface table length              */
	s4                      baseval;              /* base for runtime type check         */
	                                              /* (-index for interfaces)             */
	s4                      diffval;              /* high - base for runtime type check  */

#if USES_NEW_SUBTYPE
	s4        subtype_depth;
	s4        subtype_offset;
   vftbl_t  *subtype_display[DISPLAY_SIZE+1];  /* the last one is cache */
   vftbl_t **subtype_overflow;
#endif

	s4          *interfacevftbllength; /* length of interface vftbls          */
	methodptr    table[1];             /* class vftbl                         */
};

#endif // VFTBL_HPP_


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
