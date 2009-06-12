#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H
//=============================================================================
//
//      hal_platform_setup.h
//
//      Platform specific support for HAL (assembly code)
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jskov
// Contributors: jskov, gthomas
// Date:         2001-08-06
// Purpose:      ARM9/EXCALIBUR platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/hal_platform_setup.h>
//     Only used by "vectors.S"         
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include CYGHWR_MEMORY_LAYOUT_H
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#include <cyg/hal/excalibur.h>          // Platform specific hardware definitions

#define nDEBUG
#define n_ALTERA_CACHEHACK  // doesn't have any apparent effect

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM) || defined(CYG_HAL_STARTUP_REDBOOT)
#define PLATFORM_SETUP1 _platform_setup1
#define CYGHWR_HAL_ARM_HAS_MMU
#define CYGSEM_HAL_ROM_RESET_USES_JUMP

#define CYG_DEVICE_SERIAL_BAUD_DIV (CYGNUM_HAL_ARM_EXCALIBUR_PERIPHERAL_CLOCK/CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD/16)
#define CYG_DEVICE_SERIAL_BAUD_LSB (CYG_DEVICE_SERIAL_BAUD_DIV&0xff)
#define CYG_DEVICE_SERIAL_BAUD_MSB ((CYG_DEVICE_SERIAL_BAUD_DIV>>8)&0xff)

// We need this here - can't rely on a translation table until MMU has
// been initialized
        .macro RAW_LED_MACRO x
#ifdef DEBUG
        ldr     r0,=(EXCALIBUR_UART0_BASE+_UART_TD)
        mov     r1,#(\x + 0x41)
        str     r1,[r0]
#endif
        .endm

// This macro represents the initial startup code for the platform        
        .macro  _platform_setup1
        // IO controller init
        ldr     r1,=EXCALIBUR_IOCR_BASE
        ldr     r2,=EXCALIBUR_IOCR_SDRAM_INIT
        str     r2,[r1,#_IOCR_SDRAM]
        ldr     r2,=EXCALIBUR_IOCR_EBI_INIT
        str     r2,[r1,#_IOCR_EBI]
        ldr     r2,=EXCALIBUR_IOCR_UART_INIT
        str     r2,[r1,#_IOCR_UART]

        // Expansion bus init
        ldr     r1,=EXCALIBUR_EBI_CR
        ldr     r2,=EXCALIBUR_EBI_CR_INIT
        str     r2,[r1]

#ifdef DEBUG
        // Init UART for debug tracing
        ldr     r4,=EXCALIBUR_UART0_BASE
        ldr     r2,=(_UART_MC_8BIT | _UART_MC_1STOP | _UART_MC_PARITY_NONE)
        str     r2,[r4,#_UART_MC]
        ldr     r2,=CYG_DEVICE_SERIAL_BAUD_LSB
        str     r2,[r4,#_UART_DIV_LO]
        ldr     r2,=CYG_DEVICE_SERIAL_BAUD_MSB
        str     r2,[r4,#_UART_DIV_HI]
        ldr     r2,=(_UART_FCR_TC | _UART_FCR_RC | _UART_FCR_TX_THR_15 | _UART_FCR_RX_THR_1)
        str     r2,[r4,#_UART_FCR]
#endif

        // Setup the PLLs see the label PLL_ADDR below for the input
        // clock frequency and the desired output frequencies of PLL1
        // and PLL2 Load the value into K,M,N for PLL 1 and 2
        adr     r0,2f
        ldmia   r0,{r0-r11}
        str     r6,[r0]
        str     r7,[r1]
        str     r8,[r2]
        str     r9,[r3]
        str     r10,[r4]
        str     r11,[r5]
                
        // Turn on the PLLs
        ldr     r3,=EXCALIBUR_CLK_BASE
        ldr     r1,=(0x1035 | _CLK_PLL1_CTRL_P)
        str     r1,[r3, #_CLK_PLL1_CTRL]
        str     r1,[r3, #_CLK_PLL2_CTRL]

        // Ensure the PLLs are not in bypass
        ldr     r1,=(0x10 | _CLK_DERIVE_BP1 | _CLK_DERIVE_BP2)
        str     r1,[r3, #_CLK_DERIVE]
        orr     r1,r1,#0x300 /* Use PLL2 for AHB and for the SDRAM */
        str     r1,[r3, #_CLK_DERIVE]
        ldr     r2,=(_CLK_DERIVE_BP1 | _CLK_DERIVE_BP2)
        bic     r1,r1,r2
        str     r1,[r3, #_CLK_DERIVE]

        // Poll waiting for the PLL's to lock and the bits to not be
        // in bypass mode
        ldr     r2,=_CLK_STATUS_L2 /*_CLK_STATUS_L1 | _CLK_STATUS_L2*/
1:      ldr     r1, [r3, #_CLK_STATUS]
        and     r1, r1, r2
        cmp     r1, r2
        bne     1b

        // Clear the interrupt caused by the change in lock status
        ldr     r2, =(_CLK_STATUS_C1 | _CLK_STATUS_C2)
        str     r2, [r3, #_CLK_STATUS]
        
        b       3f

        // PLL Registers Addresses
2:      .long EXCALIBUR_CLK_BASE+_CLK_PLL1_KCNT
        .long EXCALIBUR_CLK_BASE+_CLK_PLL1_MCNT
        .long EXCALIBUR_CLK_BASE+_CLK_PLL1_NCNT
        .long EXCALIBUR_CLK_BASE+_CLK_PLL2_KCNT
        .long EXCALIBUR_CLK_BASE+_CLK_PLL2_MCNT
        .long EXCALIBUR_CLK_BASE+_CLK_PLL2_NCNT
        // PLL Registers Values ensure this follows on from the
        // addresses the code depends on it
        .long 0x40000 // CLK_PLL1_KCNT_VAL  = 1
        .long 0x20101 // CLK_PLL1_MCNT_VAL  = 2
        .long 0x40000 // CLK_PLL1_NCNT_VAL  = 1
        .long 0x40000 // CLK_PLL2_KCNT_VAL  = 1
        .long 0x20303 // CLK_PLL2_MCNT_VAL  = 6
        .long 0x40000 // CLK_PLL2_NCNT_VAL  = 1
3:

        RAW_LED_MACRO 0

        // Jump to ROM
        ldr     r1,=(EXCALIBUR_MMAP_BASE + _MMAP_EBI0)
        ldr     r2,=_MMAP_EBI0_INIT
        str     r2,[r1]
        ldr     r1,=CYGMEM_REGION_rom
        add     pc,pc,r1
        nop
        nop

        // Disable ROM mapping
        ldr     r1,=EXCALIBUR_BOOT_CR
        ldr     r2,=EXCALIBUR_BOOT_CR_BM
        str     r2,[r1]

        RAW_LED_MACRO 1

        // Disable and clear caches
        
        mrc  p15,0,r0,c1,c0,0
        bic  r0,r0,#0x1000              // disable ICache
        bic  r0,r0,#0x0007              // disable DCache,
                                        // MMU and alignment faults
        mcr  p15,0,r0,c1,c0,0
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        mov  r0,#0
        mcr  p15,0,r0,c7,c6,0           // clear data cache
        mcr  p15,0,r0,c7,c5,0           // clear instruction cache
        
#if 0
        mrc  p15,0,r0,c15,c1,0          // disable streaming
        orr  r0,r0,#0x80
        mcr  p15,0,r0,c15,c1,0
#endif

        RAW_LED_MACRO 2

        // Set memory mapping
        ldr     r1,=EXCALIBUR_MMAP_BASE
        ldr     r2,=_MMAP_REGISTERS_INIT
        str     r2,[r1,#_MMAP_REGISTERS]
        ldr     r2,=_MMAP_SRAM0_INIT
        str     r2,[r1,#_MMAP_SRAM0]
        ldr     r2,=_MMAP_SRAM1_INIT
        str     r2,[r1,#_MMAP_SRAM1]
        ldr     r2,=_MMAP_DPSRAM0_INIT
        str     r2,[r1,#_MMAP_DPSRAM0]
        ldr     r2,=_MMAP_DPSRAM1_INIT
        str     r2,[r1,#_MMAP_DPSRAM1]
        ldr     r2,=_MMAP_SDRAM0_INIT
        str     r2,[r1,#_MMAP_SDRAM0]
        ldr     r2,=_MMAP_SDRAM1_INIT
        str     r2,[r1,#_MMAP_SDRAM1]
        ldr     r2,=_MMAP_PLD0_INIT
        str     r2,[r1,#_MMAP_PLD0]
        ldr     r2,=_MMAP_PLD1_INIT
        str     r2,[r1,#_MMAP_PLD1]
        ldr     r2,=_MMAP_PLD2_INIT
        str     r2,[r1,#_MMAP_PLD2]
        ldr     r2,=_MMAP_PLD3_INIT
        str     r2,[r1,#_MMAP_PLD3]
        ldr     r2,=_MMAP_EBI1_INIT
        str     r2,[r1,#_MMAP_EBI1]
        ldr     r2,=_MMAP_EBI2_INIT
        str     r2,[r1,#_MMAP_EBI2]
        ldr     r2,=_MMAP_EBI3_INIT
        str     r2,[r1,#_MMAP_EBI3]

        RAW_LED_MACRO 3

        // FIXME: Disable MMAP registers?

        RAW_LED_MACRO 4

        // DPSRAM init
        ldr     r2,=EXCALIBUR_DPSRAM_BASE
        ldr     r1,=_DPSRAM0_LCR_INIT
        str     r1,[r2,#_DPSRAM0_LCR]
        ldr     r1,=_DPSRAM1_LCR_INIT
        str     r1,[r2,#_DPSRAM1_LCR]

        RAW_LED_MACRO 5

#ifdef _ALTERA_CACHEHACK
        mrc     p15,0,r0,c1,c0,0
        orr     r0,r0,#0x1000              // enable ICache
        mcr     p15,0,r0,c1,c0,0
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
#endif //  _ALTERA_CACHEHACK

        //  Setup the SDR

        // All the clock values below assume the SDR is running @ 100 MHz

        // 1. Wait for the PLL's to lock this was already done in
        // Setup_plls Then wait another 100uS, which given we're
        // running @ 150MHz is 15,000 clock cycles
        mov     r3,#0x3b00 // (15104)
1:      subs    r3,r3,#1
        bne     1b


        // 2. Set up all the SDRAM Controllers configuration settings
        // These are done using LDMIA's as it's more efficient, they're
        // done in the order they appear in the datasheet with the
        // exception of SDRAM_INIT which has to be written last

        // Load the address of the first seven SDRAM registers and the
        // contents
        adr     r0,SDRAM_REGS_ADDR
        ldmia   r0,{r0-r5}

        adr     r6,SDRAM_REGS_VALUE
        ldmia   r6,{r6-r11}
        str     r6,[r0]
        str     r7,[r1]
        str     r8,[r2]
        str     r9,[r3]
        str     r10,[r4]
        str     r11,[r5]

        ldr     r11,=EXCALIBUR_SDRAM_BASE
        ldr     r0,=EXCALIBUR_SDRAM_WIDTH
        ldr     r1,=(_SDRAM_WIDTH_W | _SDRAM_WIDTH_LK)  /* 32 bit wide */
        str     r1,[r0]

#ifdef _ALTERA_CACHEHACK
        // OK we have a minor "feature/bug" in the chip which requires
        // us to be a little clever now.  The code between the labels
        // Cache_Start and Cache_stop starts the SDRAM controller and
        // issues the pre_charge command
 
        // It must execute within one refresh period, so we must load
        // this code and the code it calls (Issue_SDRAM_Command) into
        // cache, as typical flashes have a cycle time of ~100nS If we
        // are doing byte loads that's ~400ns per instruction.  Depending
        // upon the size of the DDR the refresh period can be as low as
        // 7us (smallest we found).
        adr     r1,SDR_Cache_Start
        adr     r2,SDR_Cache_Stop       
        adr     r3,Issue_SDRAM_Command
        adr     r4,End_Issue_SDRAM_Command

SDR_Load_Code:  
        mcr     p15,0,r1,c7,c13,1
        add     r1,r1,#32
        cmp     r1,r2
        ble     SDR_Load_Code
        
SDR_Load_Code2:         
        mcr     p15,0,r3,c7,c13,1
        add     r3,r3,#32
        cmp     r3,r4
        ble     SDR_Load_Code2
#endif // _ALTERA_CACHEHACK

        // setup the timer for later
        ldr     r4,=EXCALIBUR_TIMER0_LIMIT
        mvn     r5, #1
        str     r5, [r4]

        ldr     r4,=_SDRAM_INIT_PR
        ldr     r5,=_SDRAM_INIT_RF
        ldr     r6,=_SDRAM_INIT_LM
        ldr     r9,=EXCALIBUR_TIMER0_READ
        ldr     r10,=EXCALIBUR_TIMER0_CR

        // Enable the controller by setting the EN bit in the
        // SDRAM_INIT register
        mov     r1,#_SDRAM_INIT_EN
SDR_Cache_Start:
        str     r1,[r11, #_SDRAM_INIT]

        // Short delay
        mov     r3, #EXCALIBUR_TIMER_CR_S
        str     r3, [r10]
        ldr     r3, [r9]
        add     r3,r3,#200 
short_delay:
        ldr     r8, [r9]
        cmp     r3, r8
        bgt     short_delay
        mov     r3, #0
        str     r3, [r10]

        // 4. Issue pre-charge all command
        mov     r1,r4
        bl      Issue_SDRAM_Command

        // 5. Issue two Refresh Commands
        mov     r1, r5
        mov     r4,#2
SDR_RF_Command:
        bl      Issue_SDRAM_Command
        subs    r4,r4,#1
        bne     SDR_RF_Command

        // 6. Issue a load mode command with the DLL being Reset
        mov     r1, r6
        bl      Issue_SDRAM_Command

SDR_Cache_Stop:
        b        10f


        //--------------------------------------------
        // Issue a command from the SDRAM controller
        // Assumes:
        // r1 is the command to be issued
        // r2 and r3 are trashed
Issue_SDRAM_Command:
        mov     r2,#_SDRAM_INIT_EN
        orr     r2,r2,r1
        str     r2,[r11, #_SDRAM_INIT]
 
        // OK chip bug, the busy bit does not work properly, so we need
        // to insert a delay of 50 SDRAM clock cycles here NB Obviously
        // this must change when either the SDRAM clock or the processor
        // clock change
        ldr     r2,=(EXCALIBUR_CLK_BASE+_CLK_AHB1_COUNT)
        ldr     r3, [r2]
        add     r3,r3,#200 // CPU 150 MHz SDRAM 75 MHz
clock_cycles:
        ldr     r8, [r2]
        cmp     r3, r8
        bgt     clock_cycles
        mov     pc, lr
End_Issue_SDRAM_Command:
        

        // SDRAM Register Addresses
SDRAM_REGS_ADDR: 
        .long EXCALIBUR_SDRAM_BASE+_SDRAM_TIMING1
        .long EXCALIBUR_SDRAM_BASE+_SDRAM_TIMING2
        .long EXCALIBUR_SDRAM_BASE+_SDRAM_CONFIG
        .long EXCALIBUR_SDRAM_BASE+_SDRAM_REFRESH
        .long EXCALIBUR_SDRAM_BASE+_SDRAM_ADDR
        .long EXCALIBUR_SDRAM_BASE+_SDRAM_MODE0
        .long EXCALIBUR_SDRAM_BASE+_SDRAM_MODE1

SDRAM_REGS_VALUE:
        .long 0x00004892 /* SDRAM_TIMING1_VAL */
        .long 0x000007b0 /* SDRAM_TIMING2_VAL */
        .long 0x00000000 /* DDR */
        .long 0x00000492 /* SDRAM_REFRESH_VAL */
        .long 0x0000Ca80 /* SDRAM_ADDR_VAL */
        .long 0x00000023 /* SDRAM_MODE0_VAL */
        .long 0x00000000 /* SDRAM_MODE1_VAL */
10:
#ifdef _ALTERA_CACHEHACK
        mrc     p15,0,r0,c1,c0,0
        bic     r0,r0,#0x1000              // disable ICache
        mcr     p15,0,r0,c1,c0,0
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
#endif // _ALTERA_CACHEHACK

        RAW_LED_MACRO 6

#if defined(CYG_HAL_STARTUP_ROMRAM) || defined(CYG_HAL_STARTUP_REDBOOT)
        ldr     r0,=__rom_vectors_lma   // Relocate FLASH/ROM to SDRAM
        ldr     r1,=__rom_vectors_vma   // ram base & length
        ldr     r2,=__ram_data_end
20:     ldr     r3,[r0],#4
        str     r3,[r1],#4
        cmp     r1,r2
        bne     20b
        ldr     r0,=30f
        nop
        mov     pc,r0
	nop
30:	nop
#endif

        RAW_LED_MACRO 7

        // Set up a stack [for calling C code]
        ldr     r1,=__startup_stack
        ldr     r2,=EXCALIBUR_SDRAM_PHYS_BASE
        orr     sp,r1,r2

        // Create MMU tables
        bl      hal_mmu_init

        RAW_LED_MACRO 8

        // Enable MMU
        ldr     r2,=10f
        ldr     r1,=MMU_Control_Init|MMU_Control_M
        mcr     MMU_CP,0,r1,MMU_Control,c0
        mov     pc,r2    // Change address spaces
        nop
        nop
        nop
10:

        RAW_LED_MACRO 9
        .endm
        
#else // defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM) || defined(CYG_HAL_STARTUP_REDBOOT)
#define PLATFORM_SETUP1
#endif

//-----------------------------------------------------------------------------
#ifdef DEBUG
#define CYGHWR_LED_MACRO                                \
        ldr     r0,=(EXCALIBUR_UART0_BASE+_UART_TD);    \
        mov     r1,#((\x) + 0x61);                      \
        str     r1,[r0];
#endif

//-----------------------------------------------------------------------------
// end of hal_platform_setup.h
#endif // CYGONCE_HAL_PLATFORM_SETUP_H
