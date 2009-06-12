##=============================================================================
##
##      context.S
##
##      mcf52xx context switch code
##
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

#------------------------------------------------------------------------------
# hal_thread_switch_context
# Switch thread contexts
# :     0(%sp)  :     return address
# :     4(%sp)  :     to - address of sp of next thread to execute
# :     8(%sp)  :     from - address of sp save location of current thread
#
# d0, d1, a0, a1 are ours to abuse.
#
# externC void hal_thread_switch_context( CYG_ADDRESS to, CYG_ADDRESS from );

FUNC_START(hal_thread_switch_context)

        move.l  4(%sp),%a0                  /*   Read the to and from  from */
        move.l  8(%sp),%a1                  /* parameters from the stack.   */

        lea     -(6+5)*4(%sp),%sp           /*   Save all of the  registers */
        movem.l %d2-%d7/%a2-%a6,(%sp)       /* that    the    C     calling */
                                            /* convention  expects  us   to */
                                            /* preserve.                    */

        move.l  %sp,(%a1)                   /*   Store this threads current */
                                            /* stack pointer to *from.      */

        move.l  (%a0),%sp                   /*   Load the stack pointer for */
                                            /* the next thread from *to.    */

        movem.l (%sp),%d2-%d7/%a2-%a6       /*   Load all of the  preserved */
        lea     (6+5)*4(%sp),%sp            /* registers from the stack.    */

        rts                                 /*   Restore the PC.            */

#------------------------------------------------------------------------------
# hal_thread_load_context
# Load thread context
# :     4(%sp)  :     to - address of sp of next thread to execute
#
# d0, d1, a0, a1 are ours to abuse.

FUNC_START(hal_thread_load_context)

        move.l  4(%sp),%a0                  /*   Read the to parameter from */
        move.l  (%a0),%sp                   /* the stack and switch to that */
                                            /* stack pointer.               */

        movem.l (%sp),%d2-%d7/%a2-%a6       /*   Load all of the  preserved */
        lea     (6+5)*4(%sp),%sp            /* registers from the stack.    */

        rts                                 /*   Restore the PC.            */

##=============================================================================
##=============================================================================

