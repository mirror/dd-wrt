#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##      platform.inc
##
##      SH/CQ7750 board assembler header file
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
#######DESCRIPTIONBEGIN####
##
## Author(s):   jskov
## Contributors:jskov, t@keshi.org
## Date:        2000-04-18
## Purpose:     SH/Dreamcast platform initialization macros.
## Description: This file contains various definitions and macros that are
##              useful for writing assembly code for the SH/Dreamcast.
## Usage:
##              #include <cyg/hal/platform.inc>
##              ...
##              
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/sh_offsets.inc>
#include <cyg/hal/sh_regs.h>

#define CYG_SR (CYGARC_REG_SR_MD|CYGARC_REG_SR_IMASK)

#------------------------------------------------------------------------------
# Hardware initialization.

        .macro  hal_hardware_init
        .endm

#------------------------------------------------------------------------------
# Post reset initialization

#ifndef CYGPKG_HAL_SH_POST_RESET_INIT
        .macro  hal_post_reset_init
        # Initialize CPU
        mov.l   $nCYG_SR,r1             ! Put CPU in a well-known state
        ldc     r1,sr
        mov     #0,r0
#if     !defined(CYG_HAL_STARTUP_RAM)
        mov.l   $nCYGARC_REG_CCR,r1     ! Disable cache
        mov.l   r0,@r1
#endif
        mov.l   $nCYGARC_REG_MMUCR,r1   ! Disable MMU
        mov.l   r0,@r1
        mov.l   $nCYGARC_REG_BBRA,r1    ! Disable UBC Channel A
        mov.w   r0,@r1
        mov.l   $nCYGARC_REG_BBRB,r1    ! Disable UBC Channel B
        mov.w   r0,@r1
        mov.l   $nCYGARC_REG_BRCR,r1    ! Reset UBC common register
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_TSTR,r1     ! Disable timers
        mov.b   r0,@r1
        mov.l   $CYGARC_REG_IPRA,r1     ! Disable interrupt request sources
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_IPRB,r1
        mov.w   r0,@r1
        mov.l   $CYGARC_REG_IPRC,r1
        mov.w   r0,@r1
        mov.w   $nCYG_WTCSR,r0          ! Clear watchdog
        mov.l   $nCYGARC_REG_WTCSR,r1
        mov.w   r0,@r1

        # Initialize VBR if necessary
#if     !defined(CYG_HAL_STARTUP_RAM) ||                \
        (       defined(CYG_HAL_STARTUP_RAM) &&         \
                !defined(CYGSEM_HAL_USE_ROM_MONITOR))
        mov.l   $_reset,r1             ! Set VBR
        ldc     r1,vbr
#endif
        bra     1f
         nop
        
$nCYG_WTCSR:
        .word   0xa500          ! clear all CSR bits

        .align  2
$nCYG_SR:
        .long   CYG_SR
$nCYGARC_REG_CCR:
        .long   CYGARC_REG_CCR
$nCYGARC_REG_MMUCR:
        .long   CYGARC_REG_MMUCR
$nCYGARC_REG_BBRA:
        .long   CYGARC_REG_BBRA
$nCYGARC_REG_BBRB:
        .long   CYGARC_REG_BBRB
$nCYGARC_REG_BRCR:
        .long   CYGARC_REG_BRCR
$CYGARC_REG_TSTR:
        .long   CYGARC_REG_TSTR
$CYGARC_REG_IPRA:
        .long   CYGARC_REG_IPRA
$CYGARC_REG_IPRB:
        .long   CYGARC_REG_IPRB
$CYGARC_REG_IPRC:
        .long   CYGARC_REG_IPRC
$nCYGARC_REG_WTCSR:
        .long   CYGARC_REG_WTCSR

        SYM_PTR_REF(_reset)

1:      
        .endm
#define CYGPKG_HAL_SH_POST_RESET_INIT
#endif

#------------------------------------------------------------------------------
# Monitor initialization.

#ifndef CYGPKG_HAL_SH_MON_DEFINED

#if !defined(CYGSEM_HAL_USE_ROM_MONITOR)

        # If we are starting up from ROM, or we are starting in
        # RAM and NOT using a ROM monitor, initialize the VSR table.

        .macro  hal_mon_init
        mov.l   $hal_vsr_table,r3
        # Write exception vectors
        mov.l   $cyg_hal_default_exception_vsr,r4
        mov     #CYGNUM_HAL_VSR_EXCEPTION_COUNT,r5
1:      mov.l   r4,@r3
        add     #4,r3
        dt      r5
        bf      1b
        # Write interrupt vector
        mov.l   $cyg_hal_default_interrupt_vsr,r4
        mov.l   $hal_vsr_table,r3
        add     #CYGNUM_HAL_VECTOR_INTERRUPT*4,r3
        mov.l   r4,@r3
        bra     2f
         nop
        .align  2

        SYM_PTR_REF(cyg_hal_default_exception_vsr)
        SYM_PTR_REF(cyg_hal_default_interrupt_vsr)
        SYM_PTR_REF(hal_vsr_table)
2:
        .endm

#elif defined(CYGSEM_HAL_USE_ROM_MONITOR)

        # Initialize the VSR table entries
        # We only take control of the interrupt vector,
        # the rest are left to the ROM for now...

        .macro  hal_mon_init
        # Write interrupt vector
        mov.l   $hal_vsr_table,r3
        mov.l   $cyg_hal_default_interrupt_vsr,r4
        add     #CYGNUM_HAL_VECTOR_INTERRUPT*4,r3
        mov.l   r4,@r3
        bra     2f
         nop
        .align  2

        SYM_PTR_REF(cyg_hal_default_interrupt_vsr)
        SYM_PTR_REF(hal_vsr_table)
2:
        .endm

#else

        .macro  hal_mon_init
        .endm

#endif


#define CYGPKG_HAL_SH_MON_DEFINED

#endif // CYGPKG_HAL_SH_MON_DEFINED

#endif // CYGONCE_HAL_PLATFORM_INC
