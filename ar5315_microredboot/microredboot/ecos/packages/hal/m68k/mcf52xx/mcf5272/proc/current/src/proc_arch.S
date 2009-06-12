##=============================================================================
##
##  proc_arch.S
##
##  mcf5272 processor code
##
##=============================================================================
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

#include <pkgconf/hal.h>

#------------------------------------------------------------------------------
# function declaration macro

#define FUNC_START(name)                        \
        .text;                                  \
        .even;                                  \
        .globl name;                            \
name:

/* ************************************************************************ */
/* These routines write to  the special purpose  registers in the  ColdFire */
/* core.  Since these registers are write-only in the supervisor model,  no */
/* corresponding read routines exist.                                       */

FUNC_START(mcf5272_wr_mbar)
    move.l  4(%sp),%d0
    movec   %d0,%mbar
    nop
    rts

##=============================================================================
##=============================================================================


