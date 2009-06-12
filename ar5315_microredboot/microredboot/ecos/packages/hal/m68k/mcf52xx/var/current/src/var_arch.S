##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
##
## eCos is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free
## Software Foundation; either version 2 or (at your option) any later version.
##
## eCos is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with eCos; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
##
## As a special exception, if other files instantiate templates or use macros
## or inline functions from this file, or you compile this file and link it
## with other works to produce a work based on this file, this file does not
## by itself cause the resulting work to be covered by the GNU General Public
## License. However the source code for this file must still be made available
## in accordance with section (3) of the GNU General Public License.
##
## This exception does not invalidate any other reasons why a work based on
## this file might be covered by the GNU General Public License.
##
## Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
## at http://sources.redhat.com/ecos/ecos-license/
## -------------------------------------------
#####ECOSGPLCOPYRIGHTEND####
##=============================================================================
/*****************************************************************************
var_arch.S -- mcf52xx variant code
*****************************************************************************/

#include <pkgconf/hal.h>

/*****************************************************************************
FUNC_START -- Function declaration macro
*****************************************************************************/
#define FUNC_START(name)                        \
        .text;                                  \
        .even;                                  \
        .globl name;                            \
name:

/* ************************************************************************ */
/* These routines write to  the special purpose  registers in the  ColdFire */
/* core.  Since these registers are write-only in the supervisor model,  no */
/* corresponding read routines exist.                                       */

FUNC_START(mcf52xx_wr_vbr)
    move.l  4(%sp),%d0
    andi.l  #0xFFF00000,%d0 /* align to 1M boundary  */
    movec   %d0,%vbr
    nop
    rts

FUNC_START(mcf52xx_wr_cacr)
    move.l  4(%sp),%d0
    movec   %d0,%cacr
    nop
    rts

FUNC_START(mcf52xx_wr_acr0)
    move.l  4(%sp),%d0
    movec   %d0,%acr0
    nop
    rts

FUNC_START(mcf52xx_wr_acr1)
    move.l  4(%sp),%d0
    movec   %d0,%acr1
    nop
    rts

FUNC_START(mcf52xx_wr_rambar)
    move.l  4(%sp),%d0
    .equ    rambar,0xc04    /* Rc for movec */
    movec   %d0,#rambar
    nop
    rts


