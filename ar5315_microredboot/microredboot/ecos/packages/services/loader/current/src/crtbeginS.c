//==========================================================================
//
//      crtbeginS.c
//
//      C runtime support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           nickg
// Contributors:        nickg
// Date:                2000-11-25
// Purpose:             C runtime support
// Description:         This file contains code that must appear at the very start of
//                      a dynamically loaded library. It contains definitions for the
//                      start of the .init and .fini sections.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>

#include <cyg/loader/loader.hxx>

/*------------------------------------------------------------------------*/

#ifndef INIT_SECTION_ASM_OP
#define INIT_SECTION_ASM_OP  ".section\t.init,\"ax\"\n"
#endif

#ifndef FINI_SECTION_ASM_OP
#define FINI_SECTION_ASM_OP  ".section\t.fini,\"ax\"\n"
#endif

#ifndef TEXT_SECTION_ASM_OP
#define TEXT_SECTION_ASM_OP  ".text\n"
#endif

#ifndef CTORS_SECTION_ASM_OP
#define CTORS_SECTION_ASM_OP    "\t.section\t.ctors,\"aw\"\n"
#endif

#ifndef DTORS_SECTION_ASM_OP
#define DTORS_SECTION_ASM_OP    "\t.section\t.dtors,\"aw\"\n"
#endif

/*------------------------------------------------------------------------*/

typedef void (*pfunc) (void);

#if 1

asm(CTORS_SECTION_ASM_OP);
static pfunc __CTOR_LIST__[1] __attribute__((unused)) = { (pfunc) -1 };
asm(DTORS_SECTION_ASM_OP);
static pfunc __DTOR_LIST__[1] __attribute__((unused)) = { (pfunc) -1 };
asm(TEXT_SECTION_ASM_OP);

#else

extern pfunc __CTOR_LIST__[];
extern pfunc __DTOR_LIST__[];

#endif

/*------------------------------------------------------------------------*/    

asm (INIT_SECTION_ASM_OP);

void __attribute__ ((__unused__))
init_dummy (void)
{
    asm( TEXT_SECTION_ASM_OP );
}

/*------------------------------------------------------------------------*/

asm (FINI_SECTION_ASM_OP);

static void __attribute__ ((__unused__))
fini_dummy (void)
{
//  __do_global_dtors_aux ();
#ifdef FORCE_FINI_SECTION_ALIGN
  FORCE_FINI_SECTION_ALIGN;
#endif
  asm (TEXT_SECTION_ASM_OP);
}

/*------------------------------------------------------------------------*/

//==========================================================================
// End of crtbeginS.c
