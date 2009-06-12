##==========================================================================
##
##      variant.S
##
##      SH2 variant assembly code
##
##==========================================================================
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
##==========================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):    jskov
## Contributors: jskov
## Date:         2002-01-09
## Purpose:      SH2 misc assembly code
######DESCRIPTIONEND####
##
##==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_sh.h>

#include <cyg/hal/sh_regs.h>
#include <cyg/hal/sh2_offsets.inc>

#include <cyg/hal/arch.inc>

#---------------------------------------------------------------------------
# Cache operations
# These need to be written in assembly to ensure they do not rely on data
# in cachable space (i.e., code must use registers exclusively, not the stack).

#if (CYGARC_SH_MOD_CAC == 1)
# This macro must be used at the top of each cache function. It ensures
# that the code gets executed from a shadow region where caching is disabled
# (0x20000000).
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
        mov.l   $CYGARC_REG_CCR,r1
        mov.b   @r1,r0
        mov     #CYGARC_REG_CCR_CE,r2
        or      r2,r0
        mov.b   r0,@r1
        nop
        rts
         nop

FUNC_START(cyg_hal_cache_disable)
        GOTO_NONCACHED_SHADOW
        mov.l   $CYGARC_REG_CCR,r1
        mov.b   @r1,r0
        mov     #CYGARC_REG_CCR_CE,r2
        not     r2,r2
        and     r2,r0
        mov.b   r0,@r1
        nop
        rts
         nop

# FIXME: Doc sez we need to disable cache before purging - but only to avoid
# messing with the instruction fetches. Since they happen via the non-cached
# reflection, try to make do without disable/contidional-enable.        
FUNC_START(cyg_hal_cache_invalidate_all)
        GOTO_NONCACHED_SHADOW
        mov.l   $CYGARC_REG_CCR,r1
        mov.b   @r1,r0
        mov     #CYGARC_REG_CCR_CP,r2
        or      r2,r0
        mov.b   r0,@r1
        nop                             ! Nothing in the docs suggest we need
        nop                             ! nops here, but without them, the
        nop                             ! CPU crashes.
        rts
         nop

# Sync the cache by forcing read-misses twice for each line
# (doing it only once could leave dirty data if it happened
# to coincided with the "flush" area used)
FUNC_START(cyg_hal_cache_sync)
        GOTO_NONCACHED_SHADOW
        mov.l   $CYGARC_REG_CACHE_ADDRESS_BASE,r1
        mov.l   $CYGARC_REG_CACHE_ADDRESS_TOP,r2
        mov.l   $CYGARC_REG_CACHE_ADDRESS_STEP,r3
1:      cmp/hi  r1,r2
        bf      2f
        mov.l   @r1,r0
        bra     1b        
         add     r3,r1                  ! delay slot!
2:      nop
        rts
         nop

FUNC_START(cyg_hal_cache_write_mode)
        GOTO_NONCACHED_SHADOW
        # Mode argument in r4.
        # Read current state and mask out the caching mode bit
        mov.l   $CYGARC_REG_CCR,r1
        mov.b   @r1,r3
        mov     #CYGARC_REG_CCR_WB,r2
	and	r2,r4
        not     r2,r2
        and     r2,r3
        # Or in the new setting and restore to CCR
        or      r4,r3
        mov.b   r3,@r1
        nop
        rts
         nop

        .align  2
$CYGARC_REG_CACHE_ADDRESS_BASE:
        .long   0
$CYGARC_REG_CACHE_ADDRESS_TOP:
        .long   (HAL_UCACHE_SIZE*2)
$CYGARC_REG_CACHE_ADDRESS_STEP:
        .long   HAL_UCACHE_LINE_SIZE
$CYGARC_REG_CCR:
	.long   CYGARC_REG_CCR

$MASK:
        .long   0x1fffffff              ! mask off top 3 bits
$BASE:
        .long   0x20000000              ! base of non-cachable memory

#elif (CYGARC_SH_MOD_CAC == 2)

FUNC_START(cyg_hal_cache_enable)
        mov.l   $CYGARC_REG_CCR,r1
        mov.w   @r1,r0
        mov     #CYGARC_REG_CCR_CE,r2
        or      r2,r0
        mov.w   r0,@r1
        nop
        rts
         nop

FUNC_START(cyg_hal_cache_disable)
        mov.l   $CYGARC_REG_CCR,r1
        mov.w   @r1,r0
        mov     #CYGARC_REG_CCR_CE,r2
        not     r2,r2
        and     r2,r0
        mov.w   r0,@r1
        nop
        rts
         nop

FUNC_START(cyg_hal_cache_invalidate_all)
        mov.l   $CYGARC_SH_MOD_CAC_CACHE_ADDRESS_ARRAY_BASE,r1
        mov.l   $CYGARC_SH_MOD_CAC_CACHE_ADDRESS_ARRAY_TOP,r2
	mov     #0,r0
1:      cmp/eq  r1,r2
        bt      2f
        mov.l   r0,@r1
	bra     1b
	 add    #4,r1
2:      nop
        rts
         nop

        .align  2
$CYGARC_SH_MOD_CAC_CACHE_ADDRESS_ARRAY_BASE:
        .long   CYGARC_SH_MOD_CAC_CACHE_ADDRESS_ARRAY_BASE
$CYGARC_SH_MOD_CAC_CACHE_ADDRESS_ARRAY_TOP:
        .long   CYGARC_SH_MOD_CAC_CACHE_ADDRESS_ARRAY_TOP
$CYGARC_REG_CCR:
	.long   CYGARC_REG_CCR
#else
# error "No cache operators for INTC type"
#endif
        .data

SYM_DEF(cyg_hal_ILVL_table)
        # The first entries in the table have static priorities.

        .byte   0xf                     // NMI
        .byte   0xf                     // LVL0
        .byte   0xe                     // LVL1
        .byte   0xd                     // LVL2
        .byte   0xc                     // LVL3
        .byte   0xb                     // LVL4
        .byte   0xa                     // LVL5
        .byte   0x9                     // LVL6
        .byte   0x8                     // LVL7
#ifdef CYGNUM_HAL_INTERRUPT_LVL14
        .byte   0x7                     // LVL8
        .byte   0x6                     // LVL9
        .byte   0x5                     // LVL10
        .byte   0x4                     // LVL11
        .byte   0x3                     // LVL12
        .byte   0x2                     // LVL13
        .byte   0x1                     // LVL14
#endif

        # The rest of the table consists of programmable levels, maintained
        # by the HAL_INTERRUPT_SET_LEVEL macro. 
        # These default to the highest level so that a spurious
        # interrupt cause the IPL to be suddenly lowered to allow all
        # interrupts. This should give a better chance at tracking down
        # the problem.
        .rept   (CYGNUM_HAL_ISR_MAX- CYGNUM_HAL_INTERRUPT_LVL_MAX)
        .byte   0xf
        .endr

        # All interrupts are masked initally. Set to 1 to enable.
SYM_DEF(cyg_hal_IMASK_table)
        .rept   (CYGNUM_HAL_ISR_MAX)
        .byte   0x0
        .endr
