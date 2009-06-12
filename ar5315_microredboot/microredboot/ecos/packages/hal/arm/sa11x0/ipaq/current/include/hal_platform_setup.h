#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H

/*=============================================================================
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
// Author(s):    gthomas
// Contributors: gthomas, richard.panton@3glab.com
// Date:         2001-02-24
// Purpose:      Intel SA1110/iPAQ platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/hal_platform_setup.h>
//     Only used by "vectors.S"         
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant (SA11x0) specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include <cyg/hal/hal_sa11x0.h>         // Variant specific hardware definitions
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#include <cyg/hal/ipaq.h>               // Platform specific hardware definitions

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_Compaq) || defined(CYG_HAL_STARTUP_WinCE)
#define PLATFORM_SETUP1 _platform_setup1
#define CYGHWR_HAL_ARM_HAS_MMU
#define CYGSEM_HAL_ROM_RESET_USES_JUMP
#if defined(CYG_HAL_STARTUP_WinCE)
#define UNMAPPED(x) (x)+SA11X0_RAM_BANK0_BASE
#endif
                
#if defined(CYG_HAL_STARTUP_ROM)
#if (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 59000)
#define SA11X0_PLL_CLOCK 0x0        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 73700)
#define SA11X0_PLL_CLOCK 0x1
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 88500)
#define SA11X0_PLL_CLOCK 0x2        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 103200) 
#define SA11X0_PLL_CLOCK 0x3        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 118000)
#define SA11X0_PLL_CLOCK 0x4        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 132700)
#define SA11X0_PLL_CLOCK 0x5        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 147500)
#define SA11X0_PLL_CLOCK 0x6        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 162200)
#define SA11X0_PLL_CLOCK 0x7        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 176900)
#define SA11X0_PLL_CLOCK 0x8        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 191700)
#define SA11X0_PLL_CLOCK 0x9        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 206400)
#define SA11X0_PLL_CLOCK 0xA        
#elif (CYGHWR_HAL_ARM_SA11X0_PROCESSOR_CLOCK == 221200)
#define SA11X0_PLL_CLOCK 0xB        
#else
#error Invalid processor clock speed
#endif
#endif

//#define DEBUG_INIT

// Special image header - required when run via Parrot loader
#ifdef CYGSEM_HAL_PARROT_BOOT
#define PLATFORM_PREAMBLE _platform_preamble
        .macro  _platform_preamble
        b       100f
        .org    0x40
        .long   0x43454345      // CECE
        .long   0x8C0B3000      // Unknown magic
        .org    0x1000
100:            
        .endm
#endif

        .macro  InitUART3
#define EGPIOBase 0x49000000
#define EGPIO_BITSY_RS232_ON     (1 << 7)   /* UART3 transceiver force on.  Active high. */
   
#define SA1100_UTCR0     0x00
#define SA1100_UTCR1     0x04
#define SA1100_UTCR2     0x08
#define SA1100_UTCR3     0x0C
#define SA1100_UTDR      0x14
#define SA1100_UTSR0     0x1c
#define SA1100_UTSR1     0x20

#define SA1100_UTCR0_PE         (1 << 0)   /* parity enable */
#define SA1100_UTCR0_OES        (1 << 1)   /* 1 for even parity */
#define SA1100_UTCR0_2STOP      (1 << 2)   /* 1 for 2 stop bits */
#define SA1100_UTCR0_8BIT       (1 << 3)   /* 1 for 8 bit data */
#define SA1100_UTCR0_SCE        (1 << 4)   /* sample clock enable */
#define SA1100_UTCR0_RCE        (1 << 5)   /* receive clock edge select */
#define SA1100_UTCR0_TCE        (1 << 6)   /* transmit clock edge select */

#define SA1100_UTCR1_BRDHIMASK  0xF
#define SA1100_UTCR2_BRDLoMASK  0xFF

#define SA1100_UTCR3_RXE        (1 << 0)        /* receiver enable */
#define SA1100_UTCR3_TXE        (1 << 1)        /* transmit enable */
#define SA1100_UTCR3_BRK        (1 << 2)        /* send a BRK */
#define SA1100_UTCR3_RIE        (1 << 3)        /* receive FIFO interrupt enable */
#define SA1100_UTCR3_TIE        (1 << 4)        /* transmit FIFO interrupt enable */
#define SA1100_UTCR3_LBM        (1 << 5)        /* loopback mode */

/* [1] 11.11.6 */
#define SA1100_UTDR_PRE         (1 << 8)        /* parity error */
#define SA1100_UTDR_FRE         (1 << 9)        /* framing error */
#define SA1100_UTDR_ROR         (1 << 10)       /* receiver overrun */

/* [1] 11.11.7 */
#define SA1100_UTSR0_TFS        (1 << 0)        /* transmit FIFO service request */

/* [1] 11.11.8 */
#define SA1100_UTSR1_TBY        (1 << 0)        /* transmit FIFO busy */
#define SA1100_UTSR1_RNE        (1 << 1)        /* receive FIFO not empty */
#define SA1100_UTSR1_TNF        (1 << 2)        /* transmit FIFO not full */
#define SA1100_UTSR1_PRE        (1 << 3)        /* parity error */          
#define SA1100_UTSR1_FRE        (1 << 4)        /* framing error */         
#define SA1100_UTSR1_ROR        (1 << 5)        /* receiver overrun */      

#define SA1100_UTSR1_ERROR_MASK 0x38

#define SA1100_UART3BASE        0x80050000 
        
       /*
        ;; ********************************************************************
        ;; InitUART3 - Initialize Serial Communications
        ;; ********************************************************************
        ;; Following reset, the UART is disabled. So, we do the following:
        */

        ldr     r1, =SA1100_UART3BASE
        /* disable the UART */
        mov     r2, #0x00
        str     r2, [r1, #SA1100_UTCR3]         /* UART1 Control Reg. 3        */
        /* Now clear all 'sticky' bits in serial I registers, cf. [1] 11.11 */
        mov     r2, #0xFF
        str     r2, [r1, #SA1100_UTSR0]         /* UART1 Status Reg. 0        */
        
        /* Set the serial port to sensible defaults: no break, no interrupts, */
        /* no parity, 8 databits, 1 stopbit. */
        mov     r2, #SA1100_UTCR0_8BIT
        str     r2, [r1, #SA1100_UTCR0]         /* UART1 Control Reg. 0        */

        /* Set BRD to 1, for a baudrate of 115K2 ([1] 11.11.4.1) */
        /* Set BRD to 3, for a baudrate of 57k6 ([1] 11.11.4.1) */
        /* Set BRD to 5, for a baudrate of 38k4 ([1] 11.11.4.1) */
        /* Set BRD to 23, for a baudrate of 9k6 ([1] 11.11.4.1) */
        mov     r2, #0x00
        str     r2, [r1, #SA1100_UTCR1]
        mov     r2, #SA11X0_UART_BAUD_RATE_DIVISOR(CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD)
        str     r2, [r1, #SA1100_UTCR2]
        /* enable the UART TX and RX */
//InitUart3Enable:        
        mov     r2, #(SA1100_UTCR3_RXE|SA1100_UTCR3_TXE)
        str     r2, [r1, #SA1100_UTCR3]

        ldr     r3, =EGPIOBase
        mov     r2, #EGPIO_BITSY_RS232_ON
        str     r2, [r3, #0]
        ldr     r2, [r3, #0]

        // Give a little pause to let the thing settle
        ldr     r1,=1000000
10:     sub     r1,r1,#1
        cmp     r1,#0
        bne     10b        
        
        .endm

        .macro  PutC c
        ldr     r8,=\c
        PutCh   r8
        .endm

        .macro  PutCh c
        ldr     r9,=SA1100_UART3BASE
        str     \c,[r9,#SA1100_UTDR]
77:     ldr     \c,[r9,#SA1100_UTSR1]
        tst     \c,#SA1100_UTSR1_TNF        // Tx FIFO not full
        beq     77b
        .endm

        .macro  PutNibble n
        and     \n,\n,#0x0F
        cmp     \n,#0x0A
        blt     78f
        add     \n,\n,#'A'-'0'-0x0A
78:     add     \n,\n,#'0'
        PutCh   \n
        .endm
        
        .macro  PutHex1 h
        mov     r0,\h,LSR #4
        PutNibble r0
        PutNibble \h
        .endm

        .macro  PutHex2 h
        mov     r1,\h,LSR #8
        PutHex1 r1
        PutHex1 \h
        .endm

        .macro  PutHex4 h
        PutC    '0'
        PutC    'x'
        mov     r2,\h,LSR #16
        PutHex2 r2
        PutHex2 \h
        .endm
        
// This macro represents the initial startup code for the platform        
        .macro  _platform_setup1

#ifdef DEBUG_INIT
        b       54f
// Dump hex memory
//  R5 - base address
//  R4 - length
//  R0..R6 destroyed
dump:           
        PutC    '\r'
        PutC    '\n'
50:     mov     r6,r5
        PutHex4 r6
        PutC    ' '
        ldr     r3,=8
52:     PutC    ' '        
        ldr     r6,[r5],#4
        PutHex4 r6
        sub     r3,r3,#1
        cmp     r3,#0
        bne     52b
        PutC    '\n'
        PutC    '\r'
        sub     r4,r4,#1
        cmp     r4,#0
        bne     50b
        mov     pc,lr

hexR6:  
        PutHex4 r6
        PutC    ' '
        mov     pc,lr        
54:             
#endif // DEBUG_INIT

        // Disable all interrupts
        ldr     r1,=SA11X0_ICMR
        mov     r0,#0
        str     r0,[r1]

        // Make sure MMU is OFF
	mov r0,#0xE0000000	// Force cache writeback by reloading
	add r2,r0,#0x4000	// cache from the zeros bank
123:    ldr r1,[r0],#32
	cmp r0, r2
	bne 123b
	mov r0,#0
	mov r1,#0x0070		// MMU Control System bit
	mcr p15,0,r0,c7,c7,0	// Flush data and instruction cache
	mcr p15,0,r0,c8,c7,0	// Flush ID TLBs
	mcr p15,0,r0,c9,c0,0	// Flush Read-Buffer
	mcr p15,0,r0,c7,c10,4	// Drain write buffer
	mcr p15,0,r0,c13,c0,0	// Disable virtual ID mapping
	mcr p15,0,r1,c1,c0,0	// Write MMU control register
	nop; nop; nop; nop

        InitUART3
#ifdef DEBUG_INIT        
        mov     r7,#5
05:     PutC    '\n'
        PutC    '\r'
        sub     r7,r7,#1
        cmp     r7,#0
        bne     05b

        mrs     r6,cpsr
        bl      hexR6
#endif // DEBUG_INIT
        
#if 0 // This made no difference        
        ldr     r1,=10f
        ldr     r2,=SA11X0_RAM_BANK0_BASE
        add     r1,r1,r2
        mov     r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
        msr     cpsr,r0
        msr     spsr,r0
        movs    pc,r1
10:             
#else
                        
        mov     r0,#(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
        msr     cpsr,r0
#endif
#ifdef DEBUG_INIT
        mrs     r6,cpsr
        bl      hexR6

        ldr     r5,=0x00000000
        ldr     r4,=0x100/32
        bl      dump
#endif // DEBUG_INIT

        // Initialise extended GPIO
        ldr     r0,=SA1110_EGPIO
        ldr     r4,=SA1110_EIO_MIN

	// Initialize pin directions
	ldr	r1,=SA11X0_GPIO_PIN_DIRECTION
	ldr	r2,=0x0401F3FC
	str	r2,[r1]

	ldr	r2,[r1,#4]
	tst	r2,#0x08000000	// Look for expansion pack
	orreq	r4,r4,#0x01B0	// Power it up if there
	str	r4,[r0]

#if defined(CYG_HAL_STARTUP_ROM)
        // Disable clock switching
        mcr     p15,0,r0,\
                SA11X0_TEST_CLOCK_AND_IDLE_REGISTER,\
                SA11X0_DISABLE_CLOCK_SWITCHING_RM,\
                SA11X0_DISABLE_CLOCK_SWITCHING_OPCODE

        // Set up processor clock
        ldr     r1,=SA11X0_PWR_MGR_PLL_CONFIG
        ldr     r2,=SA11X0_PLL_CLOCK
        str     r2,[r1]

        // Turn clock switching back on
        mcr     p15,0,r0,\
                SA11X0_TEST_CLOCK_AND_IDLE_REGISTER,\
                SA11X0_ENABLE_CLOCK_SWITCHING_RM,\
                SA11X0_ENABLE_CLOCK_SWITCHING_OPCODE
        nop
        nop
#endif
        
        // Pause
        ldr     r1,=100000
10:     sub     r1,r1,#1
        cmp     r1,#0
        bne     10b        

#ifdef CYGBLD_HAL_STARTUP_ROM_INIT_DRAM
        
        // Initialize DRAM controller
        bl      19f
// DRAM controller initialization        
dram_table:
        .word   SA11X0_DRAM0_CAS_0,           0xAAAAAAA7
        .word   SA11X0_DRAM0_CAS_1,           0xAAAAAAAA
        .word   SA11X0_DRAM0_CAS_2,           0xAAAAAAAA
//        .word   SA11X0_STATIC_CONTROL_0,      0x4B384B38
//        .word   SA11X0_STATIC_CONTROL_1,      0x22212419
        .word   SA11X0_EXP_BUS_CONFIGURATION, 0x994A994A  // 0x90E790E7
        .word   SA11X0_REFRESH_CONFIGURATION, 0x00302001
        .word   SA11X0_DRAM2_CAS_0,           0xAAAAAAA7
        .word   SA11X0_DRAM2_CAS_1,           0xAAAAAAAA
        .word   SA11X0_DRAM2_CAS_2,           0xAAAAAAAA
//        .word   SA11X0_STATIC_CONTROL_2,      0x42194449
        .word   SA11X0_SMROM_CONFIGURATION,   0x00000000  // 0xAFCCAFCC
        .word   SA11X0_DRAM_CONFIGURATION,    0x0000F354  // 0x72547254        // Disabled
        .word   0, 0

19:     mov     r1,lr                           // Points to 'dram_table'        
        ldr     r2,[r1],#4                      // First control register
20:     ldr     r3,[r1],#4
        str     r3,[r2]
        ldr     r2,[r1],#4                      // Next control register
        cmp     r2,#0
        bne     20b

        // Enable UART
        ldr     r1,=SA1110_GPCLK_CONTROL_0
        ldr     r2,=SA1110_GPCLK_SUS_UART
        str     r2,[r1]

        // Release DRAM hold (set by RESET)
        ldr     r1,=SA11X0_PWR_MGR_SLEEP_STATUS
        ldr     r2,=SA11X0_DRAM_CONTROL_HOLD
        str     r2,[r1]

        // Perform 8 reads from unmapped/unenabled DRAM
        ldr     r1,=SA11X0_RAM_BANK0_BASE
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]
        ldr     r2,[r1]

        // Enable DRAM controller
        ldr     r1,=SA11X0_DRAM_CONFIGURATION
        ldr     r2,=0x0000F355   // 0x72547255
        str     r2,[r1]

#endif // CYGBLD_HAL_STARTUP_ROM_INIT_DRAM
        
        // Release peripheral hold (set by RESET)
        ldr     r1,=SA11X0_PWR_MGR_SLEEP_STATUS
        ldr     r2,=SA11X0_PERIPHERAL_CONTROL_HOLD
        str     r2,[r1]

        // Wakeup (via power/resume button)
        ldr     r1,=SA11X0_RESET_STATUS
        ldr     r2,[r1]
        cmp     r2,#SA11X0_SLEEP_MODE_RESET
        bne     45f
        ldr     r1,=SA11X0_PWR_MGR_SCRATCHPAD
        ldr     r1,[r1]
        mov     pc,r1
        nop
45:     nop        

        // Set up a stack [for calling C code]
        ldr     r1,=__startup_stack
        ldr     r2,=SA11X0_RAM_BANK0_BASE
        orr     sp,r1,r2

#ifdef DEBUG_INIT
        mrc     p15,0,r6,c1,c0,0
        bl      hexR6
        mrc     p15,0,r6,c2,c0,0
        bl      hexR6
        mrc     p15,0,r6,c3,c0,0
        bl      hexR6

        mrc     p15,0,r5,c2,c0,0
        ldr     r6,=0xFFFFC000
        and     r5,r5,r6
        ldr     r4,=0x100/32
        bl      dump
#endif // DEBUG_INIT
        
        // Create MMU tables
        bl      hal_mmu_init

#ifdef DEBUG_INIT
        ldr     r5,=0x00000000
        ldr     r4,=0x100/32
        bl      dump

        ldr     r5,=0xC0020000
        ldr     r4,=0x100/32
        bl      dump

        mrc     p15,0,r5,c2,c0,0
        ldr     r6,=0xFFFFC000
        and     r5,r5,r6
//        ldr     r4,=0x4000/32
        ldr     r4,=0x100/32
        bl      dump
#endif // DEBUG_INIT
        
        // Enable MMU
        ldr     r2,=10f
       	ldr	r1,=MMU_Control_Init|MMU_Control_M
	mcr	MMU_CP,0,r1,MMU_Control,c0
        mov     pc,r2
        mcr     MMU_CP,0,r0,MMU_InvalidateCache,c7,0	// Flush data and instruction cache
	mcr     MMU_CP,0,r0,MMU_TLB,c7,0        	// Flush ID TLBs
10:     
        nop
        nop
        nop

#ifdef DEBUG_INIT
        ldr     r5,=0x00022000
        ldr     r4,=0xDEADDEAD
        str     r4,[r5],#4
        str     r4,[r5],#4
        str     r4,[r5],#4
        str     r4,[r5],#4

        ldr     r5,=0x00022000
        ldr     r4,=0x100/32
        bl      dump
#endif // DEBUG_INIT
        
        // Save shadow copy of BCR
        ldr     r1,=_ipaq_EGPIO
#ifdef DEGUG_INIT
        ldr     r4,=SA1110_EIO_MIN
#endif // DEBUG_INIT
        str     r4,[r1]
        .endm

#if defined(CYG_HAL_STARTUP_Compaq)
#define CYG_HAL_STARTUP_ROM
#define CYG_HAL_ROM_RESET_USES_JUMP
#endif

#else // defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1
#endif

#define PLATFORM_VECTORS         _platform_vectors
        .macro  _platform_vectors
        .globl  _ipaq_EGPIO
_ipaq_EGPIO:    .long   0       // Extended GPIO shadow

        .globl  _ipaq_LCD_params
_ipaq_LCD_params:       
        .short  0,0,0,0         // Coordinates used by virtual keyboard
        .short  0,0,0,0
        .short  0               // Checksum of above        
        .endm                                        

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
