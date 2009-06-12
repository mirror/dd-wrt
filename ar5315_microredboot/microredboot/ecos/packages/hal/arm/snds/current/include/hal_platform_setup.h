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
// Contributors: gthomas, jskov
//               Grant Edwards <grante@visi.com>
// Date:         2001-07-31
// Purpose:      
// Description:  
//
//####DESCRIPTIONEND####
//
//========================================================================*/

// does normal DRAM setup, which doesn't fail?!?!

#include <cyg/hal/plf_io.h>

#define CYGHWR_LED_MACRO                                                  \
        ldr     r0,=KS32C_IOPDATA                                        ;\
        mov     r1, #((255 & (~(\x))))                                   ;\
        str     r1, [r0]                                                 ;
 
#if CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE==4096
// Override default to a more sensible value
#undef  CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE
#define CYGNUM_HAL_COMMON_INTERRUPTS_STACK_SIZE 2048
#endif

// Use relative branch since we are going to switch the address space
// around.
#define CYGSEM_HAL_ROM_RESET_USES_JUMP

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
#define PLATFORM_SETUP1                                                    \
        ldr     r1,=KS32C_IOPMOD                                          ;\
        ldr     r2,=0xff /* set led display to output */                  ;\
        str     r2,[r1,#0x00]                                             ;\
        LED     0xaa                                                      ;\
                                                                          ;\
        /* Normal DRAM mode */                                            ;\
        ldr     r3, =0x03ffff90 /* cache+wb disabled, regs @ 0x03ff0000 */;\
        ldr     r0, =KS32C_SYSCFG                                         ;\
        str     r3,[r0]                                                   ;\
        ldr     lr,=10f                                                   ;\
1:      mov     r1,pc                                                     ;\
        sub     r1,r1,#8                                                  ;\
        ldr     r0,=1b                                                    ;\
        sub     r1,r1,r0                                                  ;\
        ldr     r0,=30f                                                   ;\
        add     r0,r0,r1                                                  ;\
        ldmia   r0,{r1-r12}                                               ;\
        ldr     r0,=KS32C_EXTDBWTH                                        ;\
        stmia   r0,{r1-r12}                                               ;\
        mov     pc,lr                                                     ;\
10:     LED 0x80                                                          ;\
                                                                           \
        /* Check that it worked, otherwise try Sync DRAM setup */         ;\
        ldr     r1,=0x00000000                                            ;\
        str     r1,[r1]                                                   ;\
        ldr     r2,[r1]                                                   ;\
        cmp     r2,r1                                                     ;\
        beq     99f                                                       ;\
                                                                           \
        /* Sync DRAM mode */                                              ;\
        LED 0x81                                                          ;\
        ldr     r3, =0x83ffff90 /* sdram c+wb disabled, regs @ 0x03ff0000 */;\
        ldr     r0, =KS32C_SYSCFG                                         ;\
        str     r3,[r0]                                                   ;\
        ldr     lr,=99f                                                   ;\
1:      mov     r1,pc                                                     ;\
        sub     r1,r1,#8                                                  ;\
        ldr     r0,=1b                                                    ;\
        sub     r1,r1,r0                                                  ;\
        ldr     r0,=40f                                                   ;\
        add     r0,r0,r1                                                  ;\
        ldmia   r0,{r1-r12}                                               ;\
        ldr     r0,=KS32C_EXTDBWTH                                        ;\
        stmia   r0,{r1-r12}                                               ;\
        mov     pc,lr                                                     ;\
                                                                          ;\
        /* The below are set with a store-multiple instruction */         ;\
                                                                          ;\
        /* Normal DRAM setup */                                           ;\
        /* Flash is 16 bit, everything else 32 bit */                     ;\
        /* .long   KS32C_EXTDBWTH */                                      ;\
30:     .long  ( (KS32C_EXTDBWTH_16BIT<<KS32C_EXTDBWTH_DSR0_shift)         \
                |(KS32C_EXTDBWTH_16BIT<<KS32C_EXTDBWTH_DSR1_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSR2_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSR3_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSR4_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSR5_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSD0_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSD1_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSD2_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSD3_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSX0_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSX1_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSX2_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSX3_shift) )      ;\
        /* Flash at 0x01800000-0x01880000, 5 cycles, 7 cycles */          ;\
        /* .long   KS32C_ROMCON0 */                                       ;\
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_7C)                                    \
                |((0x01800000 >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((0x01880000 >> 16) << KS32C_ROMCON_NEXT_shift))         ;\
        /* .long   KS32C_ROMCON1 */                                       ;\
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_5C)                                    \
                |((0x00000000 >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((0x00000000 >> 16) << KS32C_ROMCON_NEXT_shift))         ;\
        /* .long   KS32C_ROMCON2 */                                       ;\
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_5C)                                    \
                |((0x00000000 >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((0x00000000 >> 16) << KS32C_ROMCON_NEXT_shift))         ;\
        /* .long   KS32C_ROMCON3 */                                       ;\
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_3C)                                    \
                |((0x00000000 >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((0x00000000 >> 16) << KS32C_ROMCON_NEXT_shift))         ;\
        /* .long   KS32C_ROMCON4 */                                       ;\
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_5C)                                    \
                |((0x00000000 >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((0x00000000 >> 16) << KS32C_ROMCON_NEXT_shift))         ;\
        /* .long   KS32C_ROMCON5 */                                       ;\
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_5C)                                    \
                |((0x00000000 >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((0x00000000 >> 16) << KS32C_ROMCON_NEXT_shift))         ;\
        /* .long   KS32C_DRAMCON0 */                                      ;\
        .long  ( (KS32C_DRAMCON_RESERVED)                                  \
                |(KS32C_DRAMCON_CAN_10)                                    \
                |(KS32C_DRAMCON_TRP_3C)                                    \
                |(KS32C_DRAMCON_TRC_1C)                                    \
                |(KS32C_DRAMCON_TCP_1C)                                    \
                |(KS32C_DRAMCON_TCS_2C)                                    \
                |(KS32C_DRAMCON_EDO)                                       \
                |((0x00000000 >> 16) << KS32C_DRAMCON_BASE_shift)          \
                |((0x00400000 >> 16) << KS32C_DRAMCON_NEXT_shift))        ;\
        /* .long   KS32C_DRAMCON1 */                                      ;\
        .long  ( (KS32C_DRAMCON_RESERVED)                                  \
                |(KS32C_DRAMCON_CAN_10)                                    \
                |(KS32C_DRAMCON_TRP_1C)                                    \
                |(KS32C_DRAMCON_TRC_1C)                                    \
                |(KS32C_DRAMCON_TCP_1C)                                    \
                |(KS32C_DRAMCON_TCS_2C)                                    \
                |(KS32C_DRAMCON_EDO)                                       \
                |((0x00400000 >> 16) << KS32C_DRAMCON_BASE_shift)          \
                |((0x00800000 >> 16) << KS32C_DRAMCON_NEXT_shift))        ;\
        /* .long   KS32C_DRAMCON2 */                                      ;\
        .long  ( (KS32C_DRAMCON_RESERVED)                                  \
                |(KS32C_DRAMCON_CAN_10)                                    \
                |(KS32C_DRAMCON_TRP_1C)                                    \
                |(KS32C_DRAMCON_TRC_1C)                                    \
                |(KS32C_DRAMCON_TCP_1C)                                    \
                |(KS32C_DRAMCON_TCS_2C)                                    \
                |((0x00800000 >> 16) << KS32C_DRAMCON_BASE_shift)          \
                |((0x00c00000 >> 16) << KS32C_DRAMCON_NEXT_shift))        ;\
        /* .long   KS32C_DRAMCON3 */                                      ;\
        .long  ( (KS32C_DRAMCON_RESERVED)                                  \
                |(KS32C_DRAMCON_CAN_10)                                    \
                |(KS32C_DRAMCON_TRP_1C)                                    \
                |(KS32C_DRAMCON_TRC_1C)                                    \
                |(KS32C_DRAMCON_TCP_1C)                                    \
                |(KS32C_DRAMCON_TCS_2C)                                    \
                |((0x00c00000 >> 16) << KS32C_DRAMCON_BASE_shift)          \
                |((0x01000000 >> 16) << KS32C_DRAMCON_NEXT_shift))        ;\
        /* .long   KS32C_REFEXTCON */                                     ;\
        .long   (((2048+1-(16*CYGNUM_HAL_CPUCLOCK/1000000)) << KS32C_REFEXTCON_RCV_shift) \
                 |(KS32C_REFEXTCON_TCSR_1C)                                \
                 |(KS32C_REFEXTCON_TCHR_1C)                                \
                 |(KS32C_REFEXTCON_REN)                                    \
                 |(KS32C_REFEXTCON_VSF)                                    \
                 |(KS32C_REFEXTCON_BASE))                                 ;\
                                                                          ;\
        /* Sync DRAM setup */                                             ;\
        /* Flash is 16 bit, everything else 32 bit */                     ;\
        /* .long   KS32C_EXTDBWTH */                                      ;\
40:     .long  ( (KS32C_EXTDBWTH_16BIT<<KS32C_EXTDBWTH_DSR0_shift)         \
                |(KS32C_EXTDBWTH_16BIT<<KS32C_EXTDBWTH_DSR1_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSR2_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSR3_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSR4_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSR5_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSD0_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSD1_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSD2_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSD3_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSX0_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSX1_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSX2_shift)         \
                |(KS32C_EXTDBWTH_32BIT<<KS32C_EXTDBWTH_DSX3_shift) )      ;\
        /* Flash at 0x01800000-0x01880000, 5 cycles, 7 cycles */          ;\
        /* .long   KS32C_ROMCON0 */                                       ;\
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_7C)                                    \
                |((0x01800000 >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((0x01880000 >> 16) << KS32C_ROMCON_NEXT_shift))         ;\
        /* .long   KS32C_ROMCON1 */                                       ;\
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_5C)                                    \
                |((0x00000000 >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((0x00000000 >> 16) << KS32C_ROMCON_NEXT_shift))         ;\
        /* .long   KS32C_ROMCON2 */                                       ;\
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_5C)                                    \
                |((0x00000000 >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((0x00000000 >> 16) << KS32C_ROMCON_NEXT_shift))         ;\
        /* .long   KS32C_ROMCON3 */                                       ;\
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_5C)                                    \
                |((0x00000000 >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((0x00000000 >> 16) << KS32C_ROMCON_NEXT_shift))         ;\
        /* .long   KS32C_ROMCON4 */                                       ;\
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_5C)                                    \
                |((0x00000000 >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((0x00000000 >> 16) << KS32C_ROMCON_NEXT_shift))         ;\
        /* .long   KS32C_ROMCON5 */                                       ;\
        .long  ( (KS32C_ROMCON_PMC_ROM)                                    \
                |(KS32C_ROMCON_TPA_5C)                                     \
                |(KS32C_ROMCON_TACC_5C)                                    \
                |((0x00000000 >> 16) << KS32C_ROMCON_BASE_shift)           \
                |((0x00000000 >> 16) << KS32C_ROMCON_NEXT_shift))         ;\
        /* .long   KS32C_DRAMCON0 */                                      ;\
        .long  ( (KS32C_DRAMCON_RESERVED)                                  \
                |(KS32C_DRAMCON_CAN_8)                                     \
                |(KS32C_DRAMCON_TRP_4C)                                    \
                |(KS32C_DRAMCON_TRC_2C)                                    \
                |((0x00000000 >> 16) << KS32C_DRAMCON_BASE_shift)          \
                |((0x00400000 >> 16) << KS32C_DRAMCON_NEXT_shift))        ;\
        /* .long   KS32C_DRAMCON1 */                                      ;\
        .long  ( (KS32C_DRAMCON_RESERVED)                                  \
                |(KS32C_DRAMCON_CAN_8)                                     \
                |(KS32C_DRAMCON_TRP_2C)                                    \
                |(KS32C_DRAMCON_TRC_2C)                                    \
                |((0x00400000 >> 16) << KS32C_DRAMCON_BASE_shift)          \
                |((0x00800000 >> 16) << KS32C_DRAMCON_NEXT_shift))        ;\
        /* .long   KS32C_DRAMCON2 */                                      ;\
        .long  ( (KS32C_DRAMCON_RESERVED)                                  \
                |(KS32C_DRAMCON_CAN_8)                                     \
                |(KS32C_DRAMCON_TRP_2C)                                    \
                |(KS32C_DRAMCON_TRC_2C)                                    \
                |((0x00800000 >> 16) << KS32C_DRAMCON_BASE_shift)          \
                |((0x00c00000 >> 16) << KS32C_DRAMCON_NEXT_shift))        ;\
        /* .long   KS32C_DRAMCON3 */                                      ;\
        .long  ( (KS32C_DRAMCON_RESERVED)                                  \
                |(KS32C_DRAMCON_CAN_8)                                     \
                |(KS32C_DRAMCON_TRP_2C)                                    \
                |(KS32C_DRAMCON_TRC_2C)                                    \
                |((0x00c00000 >> 16) << KS32C_DRAMCON_BASE_shift)          \
                |((0x01000000 >> 16) << KS32C_DRAMCON_NEXT_shift))        ;\
        /* .long   KS32C_REFEXTCON */                                     ;\
        .long  (((2048+1-(8*CYGNUM_HAL_CPUCLOCK/1000000)) << KS32C_REFEXTCON_RCV_shift) \
                 |(KS32C_REFEXTCON_TRC_4C)                                 \
                 |(KS32C_REFEXTCON_REN)                                    \
                 |(KS32C_REFEXTCON_VSF)                                    \
                 |(KS32C_REFEXTCON_BASE))                                 ;\
99:     LED 0x82                                                          ;\
        ldr     r3,=0x00000000                                            ;\
        str     r3,[r3]                                                   ;\
        ldr     r4,[r3]                                                   ;\
        cmp     r4,r3                                                     ;\
        beq     15f                                                       ;\
11:     LED 0x83                                                          ;\
        b 11b                                                             ;\
15:     LED 0x84
#else
#define PLATFORM_SETUP1
#endif

//-----------------------------------------------------------------------------
// end of hal_platform_setup.h
#endif // CYGONCE_HAL_PLATFORM_SETUP_H
