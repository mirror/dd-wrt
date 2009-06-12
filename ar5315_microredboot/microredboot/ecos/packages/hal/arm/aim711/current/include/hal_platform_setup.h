#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H
//==========================================================================
//
//      hal_platform_setup.h
//
//      
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
// Author(s):    gthomas
// Contributors: gthomas, jskov, rcassebohm
//               Grant Edwards <grante@visi.com>
// Date:         2001-07-31
// Purpose:      
// Description:  
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <cyg/hal/plf_io.h>

        .macro LED_MACRO x

        ldr     r0,=KS32C_IOPDATA
        ldr     r1,[r0] 
        and     r1,r1,#(~0x7) 
        orr     r1,r1,#((0x7 & (~(\x)))) 
        str     r1,[r0] 
#ifdef CYGSEM_HAL_LED_WITH_DELAY 
        ldr     r1,=0x80000 
1:      sub     r1,r1,#1 
        cmp     r1,#0 
        bne     1b 
#endif 
        .endm

#define CYGHWR_LED_MACRO                                                 \
        LED_MACRO (\x)
 

// Use relative branch since we are going to switch the address space
// around.
#define CYGSEM_HAL_ROM_RESET_USES_JUMP

        .macro  PLATFORM_RELOCATE

        ldr     r1,=KS32C_IOPMOD
        ldr     r2,=0x07 /* set led display to output */
        str     r2,[r1]

        LED 0x0

        /* Check that it worked, otherwise try Sync DRAM setup */
        ldr     r1,=0x00000000
        str     r1,[r1]
        ldr     r2,[r1]
        cmp     r2,r1
        beq     99f

        /* Sync DRAM mode */
        LED 0x1

        ldr     r3, =0xe7ffff90 /* sdram c+wb disabled, regs @ 0x03ff0000 */
        ldr     r0, =KS32C_SYSCFG
        str     r3,[r0]
1:      mov     r1,pc           /* actual address  */
        sub     r1,r1,#8        /* + 8 */
        ldr     r0,=1b          /* address off 1: after remap */
        sub     r1,r1,r0
        ldr     r0,=40f
        add     r0,r0,r1

#ifdef CYG_HAL_STARTUP_ROMRAM
        ldr     lr,=4f
        add     lr,lr,r1
        ldr     r1,=AIM711_ROM0_LA_START
        add     lr,lr,r1
#else
        ldr     lr,=99f
#endif

        ldmia   r0,{r1-r12}
        ldr     r0,=KS32C_EXTDBWTH
        stmia   r0,{r1-r12}
        mov     pc,lr

#ifdef CYG_HAL_STARTUP_ROMRAM
4:
        /* Relocate text segment */
        ldr     r2,=__exception_handlers
        ldr     r3,=AIM711_ROM0_LA_START
        cmp     r2,r3
        beq     6f
        ldr     r4,=__rom_data_end
5:
        ldr     r0,[r3],#4
        str     r0,[r2],#4
        cmp     r2,r4
        bne     5b
6:
        ldr     lr,=99f
        mov     pc,lr
        nop
        nop
        nop
#endif

        /* The below are set with a store-multiple instruction */

        /* Sync DRAM setup */
        /* Flash is 8 bit, DRAM is 32 bit and EXTIO is 8 bit */
        /* .long   KS32C_EXTDBWTH */
40:     .long  ( (KS32C_EXTDBWTH_8BIT<<KS32C_EXTDBWTH_DSR0_shift)          \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSD0_shift)         \
                |(KS32C_EXTDBWTH_8BIT<<KS32C_EXTDBWTH_DSX0_shift)          \
                |(KS32C_EXTDBWTH_8BIT<<KS32C_EXTDBWTH_DSX2_shift) )
        /* Flash at 0x02000000-0x02100000, 5 cycles, 7 cycles */
        /* .long   KS32C_ROMCON0 */
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_7C)                                    \
                |((AIM711_ROM0_LA_START >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((AIM711_ROM0_LA_END >> 16) << KS32C_ROMCON_NEXT_shift))
        /* .long   KS32C_ROMCON1 */
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_5C)                                    \
                |((0x00000000 >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((0x00000000 >> 16) << KS32C_ROMCON_NEXT_shift))
        /* .long   KS32C_ROMCON2 */
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_5C)                                    \
                |((0x00000000 >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((0x00000000 >> 16) << KS32C_ROMCON_NEXT_shift))
        /* .long   KS32C_ROMCON3 */
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_5C)                                    \
                |((0x00000000 >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((0x00000000 >> 16) << KS32C_ROMCON_NEXT_shift))
        /* .long   KS32C_ROMCON4 */
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_5C)                                    \
                |((0x00000000 >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((0x00000000 >> 16) << KS32C_ROMCON_NEXT_shift))
        /* .long   KS32C_ROMCON5 */
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_5C)                                    \
                |((0x00000000 >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((0x00000000 >> 16) << KS32C_ROMCON_NEXT_shift))
        /* .long   KS32C_DRAMCON0 */
        .long  ( (KS32C_DRAMCON_RESERVED)                                  \
                |(KS32C_DRAMCON_CAN_8)                                     \
                |(KS32C_DRAMCON_TRP_4C)                                    \
                |(KS32C_DRAMCON_TRC_2C)                                    \
                |((AIM711_DRAM_LA_START >> 16) << KS32C_DRAMCON_BASE_shift)          \
                |((AIM711_DRAM_LA_END >> 16) << KS32C_DRAMCON_NEXT_shift))
        /* .long   KS32C_DRAMCON1 */
        .long  ( (KS32C_DRAMCON_RESERVED)                                  \
                |(KS32C_DRAMCON_CAN_8)                                     \
                |(KS32C_DRAMCON_TRP_2C)                                    \
                |(KS32C_DRAMCON_TRC_2C)                                    \
                |((0x00000000 >> 16) << KS32C_DRAMCON_BASE_shift)          \
                |((0x00000000 >> 16) << KS32C_DRAMCON_NEXT_shift))
        /* .long   KS32C_DRAMCON2 */
        .long  ( (KS32C_DRAMCON_RESERVED)                                  \
                |(KS32C_DRAMCON_CAN_8)                                     \
                |(KS32C_DRAMCON_TRP_2C)                                    \
                |(KS32C_DRAMCON_TRC_2C)                                    \
                |((0x00000000 >> 16) << KS32C_DRAMCON_BASE_shift)          \
                |((0x00000000 >> 16) << KS32C_DRAMCON_NEXT_shift))
        /* .long   KS32C_DRAMCON3 */
        .long  ( (KS32C_DRAMCON_RESERVED)                                  \
                |(KS32C_DRAMCON_CAN_8)                                     \
                |(KS32C_DRAMCON_TRP_2C)                                    \
                |(KS32C_DRAMCON_TRC_2C)                                    \
                |((0x00000000 >> 16) << KS32C_DRAMCON_BASE_shift)          \
                |((0x00000000 >> 16) << KS32C_DRAMCON_NEXT_shift))
        /* .long   KS32C_REFEXTCON */
        .long  (((2048+1-(8*CYGNUM_HAL_CPUCLOCK/1000000)) << KS32C_REFEXTCON_RCV_shift) \
                 |(KS32C_REFEXTCON_TRC_4C)                                 \
                 |(KS32C_REFEXTCON_REN)                                    \
                 |(KS32C_REFEXTCON_VSF)                                    \
                 |(AIM711_EXT0_LA_START >> 16) )
#if 0
50:
        .long   0x07113001 /* ROM half-word, RAM word, EXT-IO */
        .long   0x21080060 /* ROM 32 - 33 MByte */
        .long   0
        .long   0
        .long   0
        .long   0
        .long   0
        .long   0x0800038e /* RAM 0 - 8 MByte */
        .long   0
        .long   0
        .long   0
        .long   0xc01583fd /* Reactivate external Bus */
#endif

99:     LED 0x2
        ldr     r3,=0x00000000
        str     r3,[r3]
        ldr     r4,[r3]
        cmp     r4,r3
        beq     15f
11:     LED 0x3
        b 11b
15:     LED 0x4

        .endm

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
#define PLATFORM_SETUP1                                                    \
        PLATFORM_RELOCATE
#else
#define PLATFORM_SETUP1
#endif

//-----------------------------------------------------------------------------
// end of hal_platform_setup.h
#endif // CYGONCE_HAL_PLATFORM_SETUP_H
