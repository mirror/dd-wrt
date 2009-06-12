//==========================================================================
//
//      hal_sa11x0.h
//
//      HAL misc board support definitions for StrongARM SA11x0
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
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
// Contributors: gthomas, dmoseley
// Date:         2000-04-04
// Purpose:      Platform register definitions
// Description:  
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#ifndef __HAL_SA11X0_H__
#define __HAL_SA11X0_H__ 1

#ifdef __ASSEMBLER__

#define REG8_VAL(a)  (a)
#define REG16_VAL(a) (a)
#define REG32_VAL(a) (a)

#define REG8_PTR(a)  (a)
#define REG16_PTR(a) (a)
#define REG32_PTR(a) (a)

#else /* __ASSEMBLER__ */

#define REG8_VAL(a)  ((unsigned char)(a))
#define REG16_VAL(a) ((unsigned short)(a))
#define REG32_VAL(a) ((unsigned int)(a))

#define REG8_PTR(a)  ((volatile unsigned char *)(a))
#define REG16_PTR(a) ((volatile unsigned long *)(a))
#define REG32_PTR(a) ((volatile unsigned long *)(a))

#endif /* __ASSEMBLER__ */

/*
 * SA11X0 Default Memory Layout Definitions
 */
// Typically ROM, FLASH
#define SA11X0_ROM_BANK0_BASE                    (0)
#define SA11X0_ROM_BANK1_BASE                    (SA11X0_ROM_BANK0_BASE + SZ_128M)
#define SA11X0_ROM_BANK2_BASE                    (SA11X0_ROM_BANK1_BASE + SZ_128M)
#define SA11X0_ROM_BANK3_BASE                    (SA11X0_ROM_BANK2_BASE + SZ_128M)

// May be ROM, FLASH, SRAM, etc
#define SA11X0_ROM_BANK4_BASE                    (0x40000000)
#define SA11X0_ROM_BANK5_BASE                    (SA11X0_ROM_BANK4_BASE + SZ_128M)

// Typically DRAM
#define SA11X0_RAM_BANK0_BASE                    (0xC0000000)
#define SA11X0_RAM_BANK1_BASE                    (SA11X0_RAM_BANK0_BASE + SZ_128M)
#define SA11X0_RAM_BANK2_BASE                    (SA11X0_RAM_BANK1_BASE + SZ_128M)
#define SA11X0_RAM_BANK3_BASE                    (SA11X0_RAM_BANK2_BASE + SZ_128M)

#define SA11X0_ZEROS_BANK_BASE                   (SA11X0_RAM_BANK3_BASE + SZ_128M)

/*
 * SA11X0 Register Definitions
 */
#define SA11X0_REGISTER_BASE                     0x80000000
#define SA11X0_REGISTER(x)                       REG32_PTR(SA11X0_REGISTER_BASE + (x))

/*
 * SA-1100 Cache and MMU Definitions
 */
#define SA11X0_ICACHE_SIZE                       0x4000  // 16K
#define SA11X0_DCACHE_SIZE                       0x2000  // 8K
#define SA11X0_ICACHE_LINESIZE_BYTES             32
#define SA11X0_DCACHE_LINESIZE_BYTES             32
#define SA11X0_ICACHE_LINESIZE_WORDS             8
#define SA11X0_DCACHE_LINESIZE_WORDS             8
#define SA11X0_ICACHE_WAYS                       32
#define SA11X0_DCACHE_WAYS                       32
#define SA11X0_ICACHE_LINE_BASE(p)               REG32_PTR((unsigned long)(p) & \
                                                           ~(SA11X0_ICACHE_LINESIZE_BYTES - 1))
#define SA11X0_DCACHE_LINE_BASE(p)               REG32_PTR((unsigned long)(p) & \
                                                           ~(SA11X0_DCACHE_LINESIZE_BYTES - 1))

/*
 * SA-1100 Coprocessor 15 Extensions Register Definitions
 */
#ifdef __ASSEMBLER__
#  define SA11X0_READ_PROCESS_ID_REGISTER        c13
#  define SA11X0_WRITE_PROCESS_ID_REGISTER       c13
#  define SA11X0_READ_BREAKPOINT_REGISTER        c14
#  define SA11X0_WRITE_BREAKPOINT_REGISTER       c14
#  define SA11X0_TEST_CLOCK_AND_IDLE_REGISTER    c15
#else /* __ASSEMBLER__ */
#  define SA11X0_READ_PROCESS_ID_REGISTER        "c13"
#  define SA11X0_WRITE_PROCESS_ID_REGISTER       "c13"
#  define SA11X0_READ_BREAKPOINT_REGISTER        "c14"
#  define SA11X0_WRITE_BREAKPOINT_REGISTER       "c14"
#  define SA11X0_TEST_CLOCK_AND_IDLE_REGISTER    "c15"
#endif /* __ASSEMBLER__ */

/*
 * SA-1100 Process ID Virtual Address Mapping Definitions
 */
#ifdef __ASSEMBLER__
#  define SA11X0_ACCESS_PROC_ID_REGISTER_OPCODE  0x0
#  define SA11X0_ACCESS_PROC_ID_REGISTER_RM      c0
#else /* __ASSEMBLER__ */
#  define SA11X0_ACCESS_PROC_ID_REGISTER_OPCODE  "0x0"
#  define SA11X0_ACCESS_PROC_ID_REGISTER_RM      "c0"
#endif /* __ASSEMBLER__ */

#define SA11X0_PROCESS_ID_PID_MASK               0x7E000000

/*
 * SA-1100 Debug Support Definitions
 */
#ifdef __ASSEMBLER__
#  define SA11X0_ACCESS_DBAR_OPCODE              0x0
#  define SA11X0_ACCESS_DBAR_RM                  c0
#  define SA11X0_ACCESS_DBVR_OPCODE              0x0
#  define SA11X0_ACCESS_DBVR_RM                  c1
#  define SA11X0_ACCESS_DBMR_OPCODE              0x0
#  define SA11X0_ACCESS_DBMR_RM                  c2
#  define SA11X0_LOAD_DBCR_OPCODE                0x0
#  define SA11X0_LOAD_DBCR_RM                    c3
#else /* __ASSEMBLER__ */
#  define SA11X0_ACCESS_DBAR_OPCODE              "0x0"
#  define SA11X0_ACCESS_DBAR_RM                  "c0"
#  define SA11X0_ACCESS_DBVR_OPCODE              "0x0"
#  define SA11X0_ACCESS_DBVR_RM                  "c1"
#  define SA11X0_ACCESS_DBMR_OPCODE              "0x0"
#  define SA11X0_ACCESS_DBMR_RM                  "c2"
#  define SA11X0_LOAD_DBCR_OPCODE                "0x0"
#  define SA11X0_LOAD_DBCR_RM                    "c3"
#endif /* __ASSEMBLER__ */

#define SA11X0_DBCR_LOAD_WATCH_DISABLED          0x00000000
#define SA11X0_DBCR_LOAD_WATCH_ENABLED           0x00000001
#define SA11X0_DBCR_LOAD_WATCH_MASK              0x00000001
#define SA11X0_DBCR_STORE_ADDRESS_WATCH_DISABLED 0x00000000
#define SA11X0_DBCR_STORE_ADDRESS_WATCH_ENABLED  0x00000002
#define SA11X0_DBCR_STORE_ADDRESS_WATCH_MASK     0x00000002
#define SA11X0_DBCR_STORE_DATA_WATCH_DISABLED    0x00000000
#define SA11X0_DBCR_STORE_DATA_WATCH_ENABLED     0x00000004
#define SA11X0_DBCR_STORE_DATA_WATCH_MASK        0x00000004

#define SA11X0_IBCR_INSTRUCTION_ADDRESS_MASK     0xFFFFFFFC
#define SA11X0_IBCR_BREAKPOINT_DISABLED          0x00000000
#define SA11X0_IBCR_BREAKPOINT_ENABLED           0x00000001
#define SA11X0_IBCR_BREAKPOINT_ENABLE_MASK       0x00000001

/*
 * SA-1100 Test, Clock and Idle Control Definition
 */
#ifdef __ASSEMBLER__
#  define SA11X0_ICACHE_ODD_WORD_LOADING_OPCODE  0x1
#  define SA11X0_ICACHE_ODD_WORD_LOADING_RM      c1
#  define SA11X0_ICACHE_EVEN_WORD_LOADING_OPCODE 0x1
#  define SA11X0_ICACHE_EVEN_WORD_LOADING_RM     c2
#  define SA11X0_ICACHE_CLEAR_LFSR_OPCODE        0x1
#  define SA11X0_ICACHE_CLEAR_LFSR_RM            c4
#  define SA11X0_MOVE_LFSR_TO_R14_ABORT_OPCODE   0x1
#  define SA11X0_MOVE_LFSR_TO_R14_ABORT_RM       c8
#  define SA11X0_ENABLE_CLOCK_SWITCHING_OPCODE   0x2
#  define SA11X0_ENABLE_CLOCK_SWITCHING_RM       c1
#  define SA11X0_DISABLE_CLOCK_SWITCHING_OPCODE  0x2
#  define SA11X0_DISABLE_CLOCK_SWITCHING_RM      c2
#  define SA11X0_WAIT_FOR_INTERRUPT_OPCODE       0x2
#  define SA11X0_WAIT_FOR_INTERRUPT_RM           c8
#else /* __ASSEMBLER__ */
#  define SA11X0_ICACHE_ODD_WORD_LOADING_OPCODE  "0x1"
#  define SA11X0_ICACHE_ODD_WORD_LOADING_RM      "c1"
#  define SA11X0_ICACHE_EVEN_WORD_LOADING_OPCODE "0x1"
#  define SA11X0_ICACHE_EVEN_WORD_LOADING_RM     "c2"
#  define SA11X0_ICACHE_CLEAR_LFSR_OPCODE        "0x1"
#  define SA11X0_ICACHE_CLEAR_LFSR_RM            "c4"
#  define SA11X0_MOVE_LFSR_TO_R14_ABORT_OPCODE   "0x1"
#  define SA11X0_MOVE_LFSR_TO_R14_ABORT_RM       "c8"
#  define SA11X0_ENABLE_CLOCK_SWITCHING_OPCODE   "0x2"
#  define SA11X0_ENABLE_CLOCK_SWITCHING_RM       "c1"
#  define SA11X0_DISABLE_CLOCK_SWITCHING_OPCODE  "0x2"
#  define SA11X0_DISABLE_CLOCK_SWITCHING_RM      "c2"
#  define SA11X0_WAIT_FOR_INTERRUPT_OPCODE       "0x2"
#  define SA11X0_WAIT_FOR_INTERRUPT_RM           "c8"
#endif /* __ASSEMBLER__ */


/*
 * SA11X0 IRQ Controller IRQ Numbers
 */
#define SA11X0_IRQ_MIN                           0
                                                 
#define SA11X0_IRQ_GPIO_0_EDGE_DETECT            0
#define SA11X0_IRQ_GPIO_1_EDGE_DETECT            1
#define SA11X0_IRQ_GPIO_2_EDGE_DETECT            2
#define SA11X0_IRQ_GPIO_3_EDGE_DETECT            3
#define SA11X0_IRQ_GPIO_4_EDGE_DETECT            4
#define SA11X0_IRQ_GPIO_5_EDGE_DETECT            5
#define SA11X0_IRQ_GPIO_6_EDGE_DETECT            6
#define SA11X0_IRQ_GPIO_7_EDGE_DETECT            7
#define SA11X0_IRQ_GPIO_8_EDGE_DETECT            8
#define SA11X0_IRQ_GPIO_9_EDGE_DETECT            9
#define SA11X0_IRQ_GPIO_10_EDGE_DETECT           10
#define SA11X0_IRQ_GPIO_ANY_EDGE_DETECT          11
#define SA11X0_IRQ_LCD_CONT_SERVICE_REQUEST      12
#define SA11X0_IRQ_USB_SERVICE_REQUEST           13
#define SA11X0_IRQ_SDLC_SERVICE_REQUEST          14
#define SA11X0_IRQ_UART1_SERVICE_REQUEST         15
#define SA11X0_IRQ_ICP_SERVICE_REQUEST           16
#define SA11X0_IRQ_UART3_SERVICE_REQUEST         17
#define SA11X0_IRQ_MCP_SERVICE_REQUEST           18
#define SA11X0_IRQ_SSP_SERVICE_REQUEST           19
#define SA11X0_IRQ_CHANNEL_0_SERVICE_REQUEST     20
#define SA11X0_IRQ_CHANNEL_1_SERVICE_REQUEST     21
#define SA11X0_IRQ_CHANNEL_2_SERVICE_REQUEST     22
#define SA11X0_IRQ_CHANNEL_3_SERVICE_REQUEST     23
#define SA11X0_IRQ_CHANNEL_4_SERVICE_REQUEST     24
#define SA11X0_IRQ_CHANNEL_5_SERVICE_REQUEST     25
#define SA11X0_IRQ_OS_TIMER_MATCH_REG_0          26
#define SA11X0_IRQ_OS_TIMER_MATCH_REG_1          27
#define SA11X0_IRQ_OS_TIMER_MATCH_REG_2          28
#define SA11X0_IRQ_OS_TIMER_MATCH_REG_3          29
#define SA11X0_IRQ_ONE_HZ_CLOCK_TIC              30
#define SA11X0_IRQ_RTC_EQUALS_ALARM              31
                                                 
#define SA11X0_IRQ_MAX                           31
#define NUM_SA11X0_INTERRUPTS                    SA11X0_IRQ_MAX - SA11X0_IRQ_MIN + 1
#define SA11X0_IRQ_INTSRC_MASK(irq_nr)           (1 << (irq_nr))

/*
 * SA11X0 UART 1 Registers
 */
#define SA11X0_UART1_BASE                       SA11X0_REGISTER(0x10000)
#define SA11X0_UART1_CONTROL0                   SA11X0_REGISTER(0x10000)
#define SA11X0_UART1_CONTROL1                   SA11X0_REGISTER(0x10004)
#define SA11X0_UART1_CONTROL2                   SA11X0_REGISTER(0x10008)
#define SA11X0_UART1_CONTROL3                   SA11X0_REGISTER(0x1000C)
#define SA11X0_UART1_DATA                       SA11X0_REGISTER(0x10014)
#define SA11X0_UART1_STATUS0                    SA11X0_REGISTER(0x1001C)
#define SA11X0_UART1_STATUS1                    SA11X0_REGISTER(0x10020)

/*
 * SA11X0 UART 3 Registers
 */
#define SA11X0_UART3_BASE                       SA11X0_REGISTER(0x50000)
#define SA11X0_UART3_CONTROL0                   SA11X0_REGISTER(0x50000)
#define SA11X0_UART3_CONTROL1                   SA11X0_REGISTER(0x50004)
#define SA11X0_UART3_CONTROL2                   SA11X0_REGISTER(0x50008)
#define SA11X0_UART3_CONTROL3                   SA11X0_REGISTER(0x5000C)
#define SA11X0_UART3_DATA                       SA11X0_REGISTER(0x50014)
#define SA11X0_UART3_STATUS0                    SA11X0_REGISTER(0x5001C)
#define SA11X0_UART3_STATUS1                    SA11X0_REGISTER(0x50020)

/*
 * SA11X0 UART Control Register 0 Bit Fields.
 */
#define SA11X0_UART_PARITY_DISABLED              0x00
#define SA11X0_UART_PARITY_ENABLED               0x01
#define SA11X0_UART_PARITY_ENABLE_MASK           0x01
#define SA11X0_UART_PARITY_ODD                   0x00
#define SA11X0_UART_PARITY_EVEN                  0x02
#define SA11X0_UART_PARITY_MODE_MASK             0x02
#define SA11X0_UART_STOP_BITS_1                  0x00
#define SA11X0_UART_STOP_BITS_2                  0x04
#define SA11X0_UART_STOP_BITS_MASK               0x04
#define SA11X0_UART_DATA_BITS_7                  0x00
#define SA11X0_UART_DATA_BITS_8                  0x08
#define SA11X0_UART_DATA_BITS_MASK               0x08
#define SA11X0_UART_SAMPLE_CLOCK_DISABLED        0x00
#define SA11X0_UART_SAMPLE_CLOCK_ENABLED         0x10
#define SA11X0_UART_SAMPLE_CLOCK_ENABLE_MASK     0x10
#define SA11X0_UART_RX_RISING_EDGE_SELECT        0x00
#define SA11X0_UART_RX_FALLING_EDGE_SELECT       0x20
#define SA11X0_UART_RX_EDGE_SELECT_MASK          0x20
#define SA11X0_UART_TX_RISING_EDGE_SELECT        0x00
#define SA11X0_UART_TX_FALLING_EDGE_SELECT       0x40
#define SA11X0_UART_TX_EDGE_SELECT_MASK          0x20

/*
 * SA-1100 UART Baud Control Register bit masks
 */
#define SA11X0_UART_H_BAUD_RATE_DIVISOR_MASK     0x0000000F
#define SA11X0_UART_L_BAUD_RATE_DIVISOR_MASK     0x000000FF
#define SA11X0_UART_BAUD_RATE_DIVISOR(x)         ((3686400/(16*(x)))-1)

/*
 * SA-1100 UART Control Register 3 Bit Fields.
 */
#define SA11X0_UART_RX_DISABLED                  0x00
#define SA11X0_UART_RX_ENABLED                   0x01
#define SA11X0_UART_RX_ENABLE_MASK               0x01
#define SA11X0_UART_TX_DISABLED                  0x00
#define SA11X0_UART_TX_ENABLED                   0x02
#define SA11X0_UART_TX_ENABLE_MASK               0x02
#define SA11X0_UART_BREAK_DISABLED               0x00
#define SA11X0_UART_BREAK_ENABLED                0x04
#define SA11X0_UART_BREAK_MASK                   0x04
#define SA11X0_UART_RX_FIFO_INT_DISABLED         0x00
#define SA11X0_UART_RX_FIFO_INT_ENABLED          0x08
#define SA11X0_UART_RX_FIFO_INT_ENABLE_MASK      0x08
#define SA11X0_UART_TX_FIFO_INT_DISABLED         0x00
#define SA11X0_UART_TX_FIFO_INT_ENABLED          0x10
#define SA11X0_UART_TX_FIFO_INT_ENABLE_MASK      0x10
#define SA11X0_UART_NORMAL_OPERATION             0x00
#define SA11X0_UART_LOOPBACK_MODE                0x20

/*
 * SA-1100 UART Data Register bit masks
 */
#define SA11X0_UART_DATA_MASK                    0x000000FF

/*
 * SA-1100 UART Status Register 0 Bit Fields.
 */
#define SA11X0_UART_TX_SERVICE_REQUEST           0x01
#define SA11X0_UART_RX_SERVICE_REQUEST           0x02
#define SA11X0_UART_RX_IDLE                      0x04
#define SA11X0_UART_RX_BEGIN_OF_BREAK            0x08
#define SA11X0_UART_RX_END_OF_BREAK              0x10
#define SA11X0_UART_ERROR_IN_FIFO                0x20

/*
 * SA-1100 UART Status Register 1 Bit Fields.
 */
#define SA11X0_UART_TX_BUSY                      0x01
#define SA11X0_UART_RX_FIFO_NOT_EMPTY            0x02
#define SA11X0_UART_TX_FIFO_NOT_FULL             0x04
#define SA11X0_UART_PARITY_ERROR                 0x08
#define SA11X0_UART_FRAMING_ERROR                0x10
#define SA11X0_UART_RX_FIFO_OVERRUN              0x20

#define UART_BASE_0                              SA11X0_UART_CONTROL_0
#define UART_BASE_1                              SA11X0_UART_1_CONTROL_0

/*
 * SA11X0 IRQ Controller Register Definitions.
 */
#define SA11X0_ICIP                              SA11X0_REGISTER(0x10050000)
#define SA11X0_ICMR                              SA11X0_REGISTER(0x10050004)
#define SA11X0_ICLR                              SA11X0_REGISTER(0x10050008)
#define SA11X0_ICCR                              SA11X0_REGISTER(0x1005000C)
#define SA11X0_ICFP                              SA11X0_REGISTER(0x10050010)
#define SA11X0_ICPR                              SA11X0_REGISTER(0x10050020)

/*
 * SA11X0 IRQ Controller Control Register Bit Fields.
 */
#define SA11X0_ICCR_DISABLE_IDLE_MASK_ALL        0x0
#define SA11X0_ICCR_DISABLE_IDLE_MASK_ENABLED    0x1

/*
 * SA11X0 Timer/counter registers
 */
#define SA11X0_OSMR0                             SA11X0_REGISTER(0x10000000)
#define SA11X0_OSMR1                             SA11X0_REGISTER(0x10000004)
#define SA11X0_OSMR2                             SA11X0_REGISTER(0x10000008)
#define SA11X0_OSMR3                             SA11X0_REGISTER(0x1000000C)
#define SA11X0_OSCR                              SA11X0_REGISTER(0x10000010)
#define SA11X0_OSSR                              SA11X0_REGISTER(0x10000014)
#define SA11X0_OWER                              SA11X0_REGISTER(0x10000018)
#define SA11X0_OIER                              SA11X0_REGISTER(0x1000001C)
#define SA11X0_RCNR                              SA11X0_REGISTER(0x10010004)
#define SA11X0_RTTR                              SA11X0_REGISTER(0x10010008)
#define SA11X0_RTSR                              SA11X0_REGISTER(0x10010010)

// Timer status register
#define SA11X0_OSSR_TIMER0  (1<<0)   // Timer match register #0
#define SA11X0_OSSR_TIMER1  (1<<1)
#define SA11X0_OSSR_TIMER2  (1<<2)
#define SA11X0_OSSR_TIMER3  (1<<3)

// Timer interrupt enable register
#define SA11X0_OIER_TIMER0  (1<<0)
#define SA11X0_OIER_TIMER1  (1<<1)
#define SA11X0_OIER_TIMER2  (1<<2)
#define SA11X0_OIER_TIMER3  (1<<3)

// OS Timer Watchdog Match Enable Register
#define SA11X0_OWER_ENABLE  (1<<0) // write-once!

/*
 * SA-1100 Reset Controller Register Definition
 */
#define SA11X0_RESET_SOFTWARE_RESET              SA11X0_REGISTER(0x10030000)
#define SA11X0_RESET_STATUS                      SA11X0_REGISTER(0x10030004)
#define SA11X0_TUCR                              SA11X0_REGISTER(0x10030008)

#define SA11X0_TUCR_EXTERNAL_MEMORY_MASTER       (1<<10)
#define SA11X0_TUCR_RESERVED_BITS                0x1FFFF9FF

/*
 * SA-1100 Reset Controller Bit Field Definitions
 */
#define SA11X0_INVOKE_SOFTWARE_RESET             0x1

#define SA11X0_HARDWARE_RESET                    0x1
#define SA11X0_SOFTWARE_RESET                    0x2
#define SA11X0_WATCHDOG_RESET                    0x4
#define SA11X0_SLEEP_MODE_RESET                  0x8

/*
 * SA-1100 Power Manager Registers
 */
#define SA11X0_PWR_MGR_CONTROL                   SA11X0_REGISTER(0x10020000)
#define SA11X0_PWR_MGR_SLEEP_STATUS              SA11X0_REGISTER(0x10020004)
#define SA11X0_PWR_MGR_SCRATCHPAD                SA11X0_REGISTER(0x10020008)
#define SA11X0_PWR_MGR_WAKEUP_ENABLE             SA11X0_REGISTER(0x1002000C)
#define SA11X0_PWR_MGR_GENERAL_CONFIG            SA11X0_REGISTER(0x10020010)
#define SA11X0_PWR_MGR_PLL_CONFIG                SA11X0_REGISTER(0x10020014)
#define SA11X0_PWR_MGR_GPIO_SLEEP_STATE          SA11X0_REGISTER(0x10020018)
#define SA11X0_PWR_MGR_OSC_STATUS                SA11X0_REGISTER(0x1002001C)

/*
 * SA-1100 Control Register Bit Field Definitions
 */
#define SA11X0_NO_FORCE_SLEEP_MODE               0x00000000
#define SA11X0_FORCE_SLEEP_MODE                  0x00000001
#define SA11X0_SLEEP_MODE_MASK                   0x00000001

/*
 * SA-1100 Power Management Configuration Register Bit Field Definitions
 */
#define SA11X0_NO_STOP_OSC_DURING_SLEEP          0x00000000
#define SA11X0_STOP_OSC_DURING_SLEEP             0x00000001
#define SA11X0_OSC_DURING_SLEEP_MASK             0x00000001
#define SA11X0_DRIVE_PCMCIA_DURING_SLEEP         0x00000000
#define SA11X0_FLOAT_PCMCIA_DURING_SLEEP         0x00000002
#define SA11X0_PCMCIA_DURING_SLEEP_MASK          0x00000002
#define SA11X0_DRIVE_CHIPSEL_DURING_SLEEP        0x00000000
#define SA11X0_FLOAT_CHIPSEL_DURING_SLEEP        0x00000004
#define SA11X0_CHIPSEL_DURING_SLEEP_MASK         0x00000004
#define SA11X0_WAIT_OSC_STABLE                   0x00000000
#define SA11X0_FORCE_OSC_ENABLE_ON               0x00000008
#define SA11X0_OSC_STABLE_MASK                   0x00000008

/*
 * SA-1100 PLL Configuration Register Bit Field Definitions
 */
#define SA11X0_CLOCK_SPEED_59_0_MHz              0x00000000
#define SA11X0_CLOCK_SPEED_73_7_MHz              0x00000001
#define SA11X0_CLOCK_SPEED_88_5_MHz              0x00000002
#define SA11X0_CLOCK_SPEED_103_2_MHz             0x00000003
#define SA11X0_CLOCK_SPEED_118_0_MHz             0x00000004
#define SA11X0_CLOCK_SPEED_132_7_MHz             0x00000005
#define SA11X0_CLOCK_SPEED_147_5_MHz             0x00000006
#define SA11X0_CLOCK_SPEED_162_2_MHz             0x00000007
#define SA11X0_CLOCK_SPEED_176_9_MHz             0x00000008
#define SA11X0_CLOCK_SPEED_191_7_MHz             0x00000009
#define SA11X0_CLOCK_SPEED_206_4_MHz             0x0000000A
#define SA11X0_CLOCK_SPEED_221_2_MHz             0x0000000B

/*
 * SA-1100 Power Manager Wakeup Register Bit Field Definitions
 */
#define SA11X0_WAKEUP_ENABLE(x)                  ((x) & 0x8FFFFFFF)

/*
 * SA-1100 Power Manager Sleep Status Bit Field Definitions
 */
#define SA11X0_SOFTWARE_SLEEP_STATUS             0x00000001
#define SA11X0_BATTERY_FAULT_STATUS              0x00000002
#define SA11X0_VDD_FAULT_STATUS                  0x00000004
#define SA11X0_DRAM_CONTROL_HOLD                 0x00000008
#define SA11X0_PERIPHERAL_CONTROL_HOLD           0x00000010

/*
 * SA-1100 Power Manager Oscillator Status Register Bit Field Definitions
 */
#define SA11X0_OSCILLATOR_STATUS                 0x00000001

/*
 * SA-1110 GPCLK Register Definitions
 */
#define SA1110_GPCLK_CONTROL_0                   SA11X0_REGISTER(0x00020060)
#define SA1110_GPCLK_CONTROL_1                   SA11X0_REGISTER(0x0002006C)
#define SA1110_GPCLK_CONTROL_2                   SA11X0_REGISTER(0x00020070)

/* GPCLK Control Register 0 */
#define SA1110_GPCLK_SUS_GPCLK   0
#define SA1110_GPCLK_SUS_UART    1
#define SA1110_GPCLK_SCE         2
#define SA1110_GPCLK_SCD_IN      0
#define SA1110_GPCLK_SCD_OUT     4

/*
 * SA11X0 Peripheral Port Controller Register Definitions
 */
#define SA11X0_PPC_PIN_DIRECTION                 SA11X0_REGISTER(0x10060000)
#define SA11X0_PPC_PIN_STATE                     SA11X0_REGISTER(0x10060004)
#define SA11X0_PPC_PIN_ASSIGNMENT                SA11X0_REGISTER(0x10060008)
#define SA11X0_PPC_PIN_SLEEP_MODE_DIR            SA11X0_REGISTER(0x1006000C)
#define SA11X0_PPC_PIN_FLAG                      SA11X0_REGISTER(0x10060010)

/*
 * SA11X0 PPC Bit Field Definitions
 */
#define SA11X0_PPC_LCD_PIN_0_DIR_INPUT           0x00000000
#define SA11X0_PPC_LCD_PIN_0_DIR_OUTPUT          0x00000001
#define SA11X0_PPC_LCD_PIN_0_DIR_MASK            0x00000001
#define SA11X0_PPC_LCD_PIN_1_DIR_INPUT           0x00000000
#define SA11X0_PPC_LCD_PIN_1_DIR_OUTPUT          0x00000002
#define SA11X0_PPC_LCD_PIN_1_DIR_MASK            0x00000002
#define SA11X0_PPC_LCD_PIN_2_DIR_INPUT           0x00000000
#define SA11X0_PPC_LCD_PIN_2_DIR_OUTPUT          0x00000004
#define SA11X0_PPC_LCD_PIN_2_DIR_MASK            0x00000004
#define SA11X0_PPC_LCD_PIN_3_DIR_INPUT           0x00000000
#define SA11X0_PPC_LCD_PIN_3_DIR_OUTPUT          0x00000008
#define SA11X0_PPC_LCD_PIN_3_DIR_MASK            0x00000008
#define SA11X0_PPC_LCD_PIN_4_DIR_INPUT           0x00000000
#define SA11X0_PPC_LCD_PIN_4_DIR_OUTPUT          0x00000010
#define SA11X0_PPC_LCD_PIN_4_DIR_MASK            0x00000010
#define SA11X0_PPC_LCD_PIN_5_DIR_INPUT           0x00000000
#define SA11X0_PPC_LCD_PIN_5_DIR_OUTPUT          0x00000020
#define SA11X0_PPC_LCD_PIN_5_DIR_MASK            0x00000020
#define SA11X0_PPC_LCD_PIN_6_DIR_INPUT           0x00000000
#define SA11X0_PPC_LCD_PIN_6_DIR_OUTPUT          0x00000040
#define SA11X0_PPC_LCD_PIN_6_DIR_MASK            0x00000040
#define SA11X0_PPC_LCD_PIN_7_DIR_INPUT           0x00000000
#define SA11X0_PPC_LCD_PIN_7_DIR_OUTPUT          0x00000080
#define SA11X0_PPC_LCD_PIN_7_DIR_MASK            0x00000080
#define SA11X0_PPC_LCD_PIXCLK_DIR_INPUT          0x00000000
#define SA11X0_PPC_LCD_PIXCLK_DIR_OUTPUT         0x00000100
#define SA11X0_PPC_LCD_PIXCLK_DIR_MASK           0x00000100
#define SA11X0_PPC_LCD_LINECLK_DIR_INPUT         0x00000000
#define SA11X0_PPC_LCD_LINECLK_DIR_OUTPUT        0x00000200
#define SA11X0_PPC_LCD_LINECLK_DIR_MASK          0x00000200
#define SA11X0_PPC_LCD_FRAMECLK_DIR_INPUT        0x00000000
#define SA11X0_PPC_LCD_FRAMECLK_DIR_OUTPUT       0x00000400
#define SA11X0_PPC_LCD_FRAMECLK_DIR_MASK         0x00000400
#define SA11X0_PPC_LCD_AC_BIAS_DIR_INPUT         0x00000000
#define SA11X0_PPC_LCD_AC_BIAS_DIR_OUTPUT        0x00000800
#define SA11X0_PPC_LCD_AC_BIAS_DIR_MASK          0x00000800
#define SA11X0_PPC_SERIAL_PORT_1_TX_DIR_INPUT    0x00000000
#define SA11X0_PPC_SERIAL_PORT_1_TX_DIR_OUTPUT   0x00001000
#define SA11X0_PPC_SERIAL_PORT_1_TX_DIR_MASK     0x00001000
#define SA11X0_PPC_SERIAL_PORT_1_RX_DIR_INPUT    0x00000000
#define SA11X0_PPC_SERIAL_PORT_1_RX_DIR_OUTPUT   0x00002000
#define SA11X0_PPC_SERIAL_PORT_1_RX_DIR_MASK     0x00002000
#define SA11X0_PPC_SERIAL_PORT_2_TX_DIR_INPUT    0x00000000
#define SA11X0_PPC_SERIAL_PORT_2_TX_DIR_OUTPUT   0x00004000
#define SA11X0_PPC_SERIAL_PORT_2_TX_DIR_MASK     0x00004000
#define SA11X0_PPC_SERIAL_PORT_2_RX_DIR_INPUT    0x00000000
#define SA11X0_PPC_SERIAL_PORT_2_RX_DIR_OUTPUT   0x00008000
#define SA11X0_PPC_SERIAL_PORT_2_RX_DIR_MASK     0x00008000
#define SA11X0_PPC_SERIAL_PORT_3_TX_DIR_INPUT    0x00000000
#define SA11X0_PPC_SERIAL_PORT_3_TX_DIR_OUTPUT   0x00010000
#define SA11X0_PPC_SERIAL_PORT_3_TX_DIR_MASK     0x00010000
#define SA11X0_PPC_SERIAL_PORT_3_RX_DIR_INPUT    0x00000000
#define SA11X0_PPC_SERIAL_PORT_3_RX_DIR_OUTPUT   0x00020000
#define SA11X0_PPC_SERIAL_PORT_3_RX_DIR_MASK     0x00020000
#define SA11X0_PPC_SERIAL_PORT_4_TX_DIR_INPUT    0x00000000
#define SA11X0_PPC_SERIAL_PORT_4_TX_DIR_OUTPUT   0x00040000
#define SA11X0_PPC_SERIAL_PORT_4_TX_DIR_MASK     0x00040000
#define SA11X0_PPC_SERIAL_PORT_4_RX_DIR_INPUT    0x00000000
#define SA11X0_PPC_SERIAL_PORT_4_RX_DIR_OUTPUT   0x00080000
#define SA11X0_PPC_SERIAL_PORT_4_RX_DIR_MASK     0x00080000
#define SA11X0_PPC_SERIAL_PORT_4_SERCLK_INPUT    0x00000000
#define SA11X0_PPC_SERIAL_PORT_4_SERCLK_OUTPUT   0x00100000
#define SA11X0_PPC_SERIAL_PORT_4_SERCLK_MASK     0x00100000
#define SA11X0_PPC_SERIAL_PORT_4_SERFRM_INPUT    0x00000000
#define SA11X0_PPC_SERIAL_PORT_4_SERFRM_OUTPUT   0x00200000
#define SA11X0_PPC_SERIAL_PORT_4_SERFRM_MASK     0x00200000

#define SA11X0_PPC_UART_PIN_NOT_REASSIGNED       0x00000000
#define SA11X0_PPC_UART_PIN_REASSIGNED           0x00001000
#define SA11X0_PPC_UART_PIN_REASSIGNMENT_MASK    0x00001000
#define SA11X0_PPC_SSP_PIN_NOT_REASSIGNED        0x00000000
#define SA11X0_PPC_SSP_PIN_REASSIGNED            0x00040000
#define SA11X0_PPC_SSP_PIN_REASSIGNMENT_MASK     0x00040000

/*
 * SA-1100 MCP Registers
 */
#define SA11X0_MCP_CONTROL_0                     SA11X0_REGISTER(0x00060000)
#define SA11X0_MCP_DATA_0                        SA11X0_REGISTER(0x00060008)
#define SA11X0_MCP_DATA_1                        SA11X0_REGISTER(0x0006000C)
#define SA11X0_MCP_DATA_2                        SA11X0_REGISTER(0x00060010)
#define SA11X0_MCP_STATUS                        SA11X0_REGISTER(0x00060018)
#define SA11X0_MCP_CONTROL_1                     SA11X0_REGISTER(0x00060030)

/*
 * SA-1100 Memory Configuration Registers
 */
#define SA11X0_DRAM_CONFIGURATION                SA11X0_REGISTER(0x20000000)
#define SA11X0_DRAM0_CAS_0                       SA11X0_REGISTER(0x20000004)
#define SA11X0_DRAM0_CAS_1                       SA11X0_REGISTER(0x20000008)
#define SA11X0_DRAM0_CAS_2                       SA11X0_REGISTER(0x2000000C)
#define SA11X0_STATIC_CONTROL_0                  SA11X0_REGISTER(0x20000010)
#define SA11X0_STATIC_CONTROL_1                  SA11X0_REGISTER(0x20000014)
#define SA11X0_EXP_BUS_CONFIGURATION             SA11X0_REGISTER(0x20000018)
#define SA11X0_REFRESH_CONFIGURATION             SA11X0_REGISTER(0x2000001C)
#define SA11X0_DRAM2_CAS_0                       SA11X0_REGISTER(0x20000020)
#define SA11X0_DRAM2_CAS_1                       SA11X0_REGISTER(0x20000024)
#define SA11X0_DRAM2_CAS_2                       SA11X0_REGISTER(0x20000028)
#define SA11X0_STATIC_CONTROL_2                  SA11X0_REGISTER(0x2000002C)
#define SA11X0_SMROM_CONFIGURATION               SA11X0_REGISTER(0x20000030)

/*
 * SA-1100 DRAM Configuration Bit Field Definitions
 */
#define SA11X0_DRAM_BANK_0_DISABLED              0x00000000
#define SA11X0_DRAM_BANK_0_ENABLED               0x00000001
#define SA11X0_DRAM_BANK_0_ENABLE_MASK           0x00000001
#define SA11X0_DRAM_BANK_1_DISABLED              0x00000000
#define SA11X0_DRAM_BANK_1_ENABLED               0x00000002
#define SA11X0_DRAM_BANK_1_ENABLE_MASK           0x00000002
#define SA11X0_DRAM_BANK_2_DISABLED              0x00000000
#define SA11X0_DRAM_BANK_2_ENABLED               0x00000004
#define SA11X0_DRAM_BANK_2_ENABLE_MASK           0x00000004
#define SA11X0_DRAM_BANK_3_DISABLED              0x00000000
#define SA11X0_DRAM_BANK_3_ENABLED               0x00000008
#define SA11X0_DRAM_BANK_3_ENABLE_MASK           0x00000008
#define SA11X0_DRAM_ROW_ADDRESS_BITS_9           0x00000000
#define SA11X0_DRAM_ROW_ADDRESS_BITS_10          0x00000010
#define SA11X0_DRAM_ROW_ADDRESS_BITS_11          0x00000020
#define SA11X0_DRAM_ROW_ADDRESS_BITS_12          0x00000030
#define SA11X0_DRAM_ROW_ADDRESS_BITS_MASK        0x00000030
#define SA11X0_DRAM_CLOCK_CPU_CLOCK              0x00000000
#define SA11X0_DRAM_CLOCK_CPU_CLOCK_DIV_2        0x00000040
#define SA11X0_DRAM_CLOCK_CPU_CLOCK_MASK         0x00000040
#define SA11X0_DRAM_RAS_PRECHARGE(x)             (((x) & 0xF) << 7)
#define SA11X0_DRAM_CAS_BEFORE_RAS(x)            (((x) & 0xF) << 11)
#define SA11X0_DATA_INPUT_LATCH_WITH_CAS         0x00000000
#define SA11X0_DATA_INPUT_LATCH_CAS_PLUS_ONE     0x00008000
#define SA11X0_DATA_INPUT_LATCH_CAS_PLUS_TWO     0x00010000
#define SA11X0_DATA_INPUT_LATCH_CAS_PLUS_THREE   0x00018000
#define SA11X0_DRAM_REFRESH_INTERVAL(x)          (((x) & 0x7FFF) << 17)

/*
 * SA-1100 Static Memory Control Register Bit Field Definitions
 */
#define SA11X0_STATIC_ROM_TYPE_FLASH             0x00000000
#define SA11X0_STATIC_ROM_TYPE_SRAM              0x00000001
#define SA11X0_STATIC_ROM_TYPE_BURST_OF_4_ROM    0x00000002
#define SA11X0_STATIC_ROM_TYPE_BURST_OF_8_ROM    0x00000003
#define SA11X0_STATIC_ROM_TYPE_MASK              0x00000003
#define SA11X0_STATIC_ROM_BUS_WIDTH_32_BITS      0x00000000
#define SA11X0_STATIC_ROM_BUS_WIDTH_16_BITS      0x00000004
#define SA11X0_STATIC_ROM_BUS_WIDTH_MASK         0x00000004
#define SA11X0_STATIC_ROM_DELAY_FIRST_ACCESS(x)  (((x) & 0x1F) << 3)
#define SA11X0_STATIC_ROM_DELAY_NEXT_ACCESS(x)   (((x) & 0x1F) << 8)
#define SA11X0_STATIC_ROM_RECOVERY(x)            (((x) & 0x7) << 13)

#define SA11X0_STATIC_ROM_BANK_0(x)              (((x) & 0xFFFF) <<  0)
#define SA11X0_STATIC_ROM_BANK_1(x)              (((x) & 0xFFFF) << 16)
#define SA11X0_STATIC_ROM_BANK_2(x)              (((x) & 0xFFFF) <<  0)
#define SA11X0_STATIC_ROM_BANK_3(x)              (((x) & 0xFFFF) << 16)

/*
 * SA-1100 GPIO Register Definitions
 */
#define SA11X0_GPIO_PIN_0                        (1 << 0)
#define SA11X0_GPIO_PIN_1                        (1 << 1)
#define SA11X0_GPIO_PIN_2                        (1 << 2)
#define SA11X0_GPIO_PIN_3                        (1 << 3)
#define SA11X0_GPIO_PIN_4                        (1 << 4)
#define SA11X0_GPIO_PIN_5                        (1 << 5)
#define SA11X0_GPIO_PIN_6                        (1 << 6)
#define SA11X0_GPIO_PIN_7                        (1 << 7)
#define SA11X0_GPIO_PIN_8                        (1 << 8)
#define SA11X0_GPIO_PIN_9                        (1 << 9)
#define SA11X0_GPIO_PIN_10                       (1 << 10)
#define SA11X0_GPIO_PIN_11                       (1 << 11)
#define SA11X0_GPIO_PIN_12                       (1 << 12)
#define SA11X0_GPIO_PIN_13                       (1 << 13)
#define SA11X0_GPIO_PIN_14                       (1 << 14)
#define SA11X0_GPIO_PIN_15                       (1 << 15)
#define SA11X0_GPIO_PIN_16                       (1 << 16)
#define SA11X0_GPIO_PIN_17                       (1 << 17)
#define SA11X0_GPIO_PIN_18                       (1 << 18)
#define SA11X0_GPIO_PIN_19                       (1 << 19)
#define SA11X0_GPIO_PIN_20                       (1 << 20)
#define SA11X0_GPIO_PIN_21                       (1 << 21)
#define SA11X0_GPIO_PIN_22                       (1 << 22)
#define SA11X0_GPIO_PIN_23                       (1 << 23)
#define SA11X0_GPIO_PIN_24                       (1 << 24)
#define SA11X0_GPIO_PIN_25                       (1 << 25)
#define SA11X0_GPIO_PIN_26                       (1 << 26)
#define SA11X0_GPIO_PIN_27                       (1 << 27)

#define SA11X0_GPIO_PIN_LEVEL                    SA11X0_REGISTER(0x10040000)
#define SA11X0_GPIO_PIN_DIRECTION                SA11X0_REGISTER(0x10040004)
#define SA11X0_GPIO_PIN_OUTPUT_SET               SA11X0_REGISTER(0x10040008)
#define SA11X0_GPIO_PIN_OUTPUT_CLEAR             SA11X0_REGISTER(0x1004000C)
#define SA11X0_GPIO_RISING_EDGE_DETECT           SA11X0_REGISTER(0x10040010)
#define SA11X0_GPIO_FALLING_EDGE_DETECT          SA11X0_REGISTER(0x10040014)
#define SA11X0_GPIO_EDGE_DETECT_STATUS           SA11X0_REGISTER(0x10040018)
#define SA11X0_GPIO_ALTERNATE_FUNCTION           SA11X0_REGISTER(0x1004001C)

#endif /* __HAL_SA11X0_H__ */
