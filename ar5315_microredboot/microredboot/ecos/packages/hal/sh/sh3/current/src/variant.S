##==========================================================================
##
##      variant.S
##
##      SH3 variant assembly code
##
##==========================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 2003 Bart Veer        
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
##==========================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):    jskov
## Contributors: jskov
## Date:         2000-10-30
## Purpose:      SH3 misc assembly code
######DESCRIPTIONEND####
##
##==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_sh.h>

#include <cyg/hal/sh_regs.h>
#include <cyg/hal/sh3_offsets.inc>

#include <cyg/hal/arch.inc>

#---------------------------------------------------------------------------
# Cache operations
# These need to be written in assembly to ensure they do not rely on data
# in cachable space (i.e., code must use registers exclusively, not the stack).

# This macro must be used at the top of each cache function. It ensures
# that the code gets executed from a shadow region where caching is disabled
# (0xA0000000).
        .macro GOTO_NONCACHED_SHADOW
        mova    10f,r0
        mov.l   $MASK,r1
        and     r1,r0
        mov.l   $BASE,r1
        or      r1,r0
        jmp     @r0
         nop
        .align  2
10:
        .endm

FUNC_START(cyg_hal_cache_enable)
        GOTO_NONCACHED_SHADOW
        mov     #CYGARC_REG_CCR & 0x0FF,r1
        mov.l   @r1,r0
        mov     #CYGARC_REG_CCR_CE,r2
        or      r2,r0
        mov.l   r0,@r1
        nop
        rts
         nop

FUNC_START(cyg_hal_cache_disable)
        GOTO_NONCACHED_SHADOW
        mov     #CYGARC_REG_CCR & 0x0FF,r1
        mov.l   @r1,r0
        mov     #CYGARC_REG_CCR_CE,r2
        not     r2,r2
        and     r2,r0
        mov.l   r0,@r1
        nop
        rts
         nop
        
FUNC_START(cyg_hal_cache_invalidate_all)
        GOTO_NONCACHED_SHADOW
        mov     #CYGARC_REG_CCR & 0x0FF,r1
        mov.l   @r1,r0
        mov     #CYGARC_REG_CCR_CF,r2
        or      r2,r0
        mov.l   r0,@r1
        nop                             ! Nothing in the docs suggest we need
        nop                             ! nops here, but without them, the
        nop                             ! CPU crashes.
        rts
         nop

FUNC_START(cyg_hal_cache_sync)
        GOTO_NONCACHED_SHADOW
        mov.l   $CYGARC_REG_CACHE_ADDRESS_FLUSH,r0
        mov.l   $CYGARC_REG_CACHE_ADDRESS_BASE,r1
        mov.l   $CYGARC_REG_CACHE_ADDRESS_TOP,r2
        mov.l   $CYGARC_REG_CACHE_ADDRESS_STEP,r3
1:      cmp/hi  r1,r2
        bf      2f
        mov.l   r0,@r1
        bra     1b        
         add     r3,r1                  ! delay slot!
2:      nop
        rts
         nop

	! r4 = base
	! r5 = size
FUNC_START(cyg_hal_cache_sync_region)
        GOTO_NONCACHED_SHADOW
	mov.l   10f,r0
	and	r4,r0		! array index
        mov.l   11f,r1
	add	r0,r1           ! base (aligned, A set)

	! make sure top is aligned to start of _next_ cache line
	mov	r1,r0
	add     r5,r0		! top (non-aligned)
	add	#2*HAL_UCACHE_LINE_SIZE-1,r0
	mov.l	13f,r2
	and	r0,r2

        mov.l   $CYGARC_REG_CACHE_ADDRESS_STEP,r3
        mov.l   12f,r5
1:      cmp/hi  r1,r2
        bf      3f
	mov	r4,r0		! create address tag
	and	r5,r0
	mov.l   r0,@r1          ! store tag in array, causing (sync+)invalidate
		                ! if the tag matches any of the lines
        add     r3,r4		! inc address tag
        bra     1b        
         add     r3,r1          ! inc array index, delay slot!
3:      nop
        rts
         nop

	.align 2
10:	.long	((HAL_UCACHE_SIZE/HAL_UCACHE_WAYS)-1)&~0xf ! mask
11:	.long   CYGARC_REG_CACHE_ADDRESS_BASE|CYGARC_REG_CACHE_ADDRESS_ADDRESS
12:	.long	CYGARC_REG_CACHE_ADDRESS_TAG_Mask
13:	.long   ~(HAL_UCACHE_LINE_SIZE-1)
	
FUNC_START(cyg_hal_cache_write_mode)
        GOTO_NONCACHED_SHADOW
        # Mode argument in r4.
        # Read current state and mask out the two caching mode bits
        mov     #CYGARC_REG_CCR & 0x0FF,r1
        mov.l   @r1,r3
        mov     #CYGARC_REG_CCR_CB|CYGARC_REG_CCR_WT,r2
	and	r2,r4
        not     r2,r2
        and     r2,r3
        # Or in the new settings and restore to CCR
        or      r4,r3
        mov.l   r3,@r1
        nop
        rts
         nop

        .align  2
$CYGARC_REG_CACHE_ADDRESS_FLUSH:
        .long   CYGARC_REG_CACHE_ADDRESS_FLUSH
$CYGARC_REG_CACHE_ADDRESS_BASE:
        .long   CYGARC_REG_CACHE_ADDRESS_BASE
$CYGARC_REG_CACHE_ADDRESS_TOP:
        .long   CYGARC_REG_CACHE_ADDRESS_TOP
$CYGARC_REG_CACHE_ADDRESS_STEP:
        .long   CYGARC_REG_CACHE_ADDRESS_STEP


$MASK:
        .long   0x1fffffff              ! mask off top 3 bits
$BASE:
        .long   0xa0000000              ! base of non-cachable memory

        .data

SYM_DEF(cyg_hal_ILVL_table)
        # The first entries in the table have static priorities.

        .byte   0xf                     // NMI
        .byte   0xf                     // Reserved
        .byte   0xf                     // LVL0
        .byte   0xe                     // LVL1
        .byte   0xd                     // LVL2
        .byte   0xc                     // LVL3
        .byte   0xb                     // LVL4
        .byte   0xa                     // LVL5
        .byte   0x9                     // LVL6
        .byte   0x8                     // LVL7
        .byte   0x7                     // LVL8
        .byte   0x6                     // LVL9
        .byte   0x5                     // LVL10
        .byte   0x4                     // LVL11
        .byte   0x3                     // LVL12
        .byte   0x2                     // LVL13
        .byte   0x1                     // LVL14
        .byte   0xf                     // Reserved

        # The rest of the table consists of programmable levels, maintained
        # by the HAL_INTERRUPT_SET_LEVEL macro. 
        # These default to the highest level so that a spurious
        # interrupt cause the IPL to be suddenly lowered to allow all
        # interrupts. This should give a better chance at tracking down
        # the problem.
        .rept   (CYGNUM_HAL_ISR_MAX-CYGNUM_HAL_INTERRUPT_RESERVED_3E0)
        .byte   0xf
        .endr

        # All interrupts are masked initally. Set to 1 to enable.
SYM_DEF(cyg_hal_IMASK_table)
        .rept   (CYGNUM_HAL_ISR_MAX)
        .byte   0x0
        .endr
