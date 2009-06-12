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
hal_arch.S -- m68k architecture code
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

/*****************************************************************************
     The following routines assume the hal_jmp_buf structure is defined  as
shown below.

typedef struct {
    cyg_uint32 d2;
    cyg_uint32 d3;
    cyg_uint32 d4;
    cyg_uint32 d5;
    cyg_uint32 d6;
    cyg_uint32 d7;
    cyg_uint32 a2;
    cyg_uint32 a3;
    cyg_uint32 a4;
    cyg_uint32 a5;
    cyg_uint32 a6;
    cyg_uint32 sp;
    cyg_uint32 pc;
} hal_jmp_buf_t;
typedef cyg_uint32 hal_jmp_buf[sizeof(hal_jmp_buf_t) / sizeof(cyg_uint32)];
*****************************************************************************/

/*****************************************************************************
hal_m68k_setjmp -- Generic setjmp for the m68k architecture

externC int hal_m68k_setjmp(hal_jmp_buf env);

INPUT:

     0(%sp): return address

     4(%sp): env - address of a hal_jmp_buf structure

OUTPUT:

     d0, d1, a0, a1 are ours to abuse.

RETURN VALUE:

     This routine always returns zero in d0.l.

*****************************************************************************/
FUNC_START(hal_m68k_setjmp)

    lea.l   4(%sp),%a1                  /*   Get a pointer to the  position */
                                        /* of the stack pointer before this */
                                        /* call was  made.  IMPORTANT:  The */
                                        /* longjmp    routine    and    the */
                                        /* exception  handler  assume  that */
                                        /* saved stack  pointers  point  to */
                                        /* the location of the stack before */
                                        /* the routine/exception occurred.  */

    move.l  (%a1),%a0                   /*   Get a pointer to the buffer to */
                                        /* save our state into.             */

    movem.l %d2-%d7/%a2-%a6,(%a0)       /*   Write  all  of  the  preserved */
    lea     (11*4)(%a0),%a0             /* registers,  the  stack  pointer, */
    move.l  %a1,(%a0)+                  /* and the return address into  the */
    move.l  (%sp),(%a0)                 /* structure.                       */

    moveq.l #0,%d0                      /*   Load a zero  return value  and */
    rts                                 /* return.                          */

/*****************************************************************************
hal_m68k_longjmp -- Generic longjmp for the m68k architecture

externC void hal_m68k_longjmp(hal_jmp_buf env, int val);

INPUT:

     0(%sp): return address

     4(%sp): env - address of a hal_jmp_buf structure

     8(%sp): val - the non-zero value to return

OUTPUT:

     d0, d1, a0, a1 are ours to abuse.

RETURN VALUE:

     This routine always returns the value  from the val parameter in  d0.l
and to the location of the PC in the env structure.

*****************************************************************************/
FUNC_START(hal_m68k_longjmp)

    move.l  8(%sp),%d0                  /*   Load    the    return    value */
                                        /* parameter.                       */

    move.l  4(%sp),%a0                  /*   Get a pointer to the buffer to */
                                        /* read our state from.             */

    movem.l (%a0),%d2-%d7/%a2-%a6       /*   Load our preserved  registers, */
    lea     (11*4)(%a0),%a0             /* stack pointer and return address */
    move.l  (%a0)+,%d1                  /* from the structure.              */
    move.l  (%a0),%a1

    move.l  %d1,%sp                     /*   Set the stack pointer from the */
    jmp     (%a1)                       /* structure as  out current  stack */
                                        /* pointer and jump  to the  return */
                                        /* address from the structure.      */



