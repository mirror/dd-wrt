#ifndef CYGONCE_HAL_INTEGRATOR_H
#define CYGONCE_HAL_INTEGRATOR_H

/*=============================================================================
//
//      hal_integrator.h
//
//      HAL Description of INTEGRATOR board
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
// Author(s):    Philippe Robin
// Contributors: 
// Date:         November 7, 2000
// Purpose:      
// Description:
// Usage:        #include <cyg/hal/hal_integrator.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/


#define INTEGRATOR_HDR_BASE             0x10000000
#define INTEGRATOR_HDR_CTRL_OFFSET      0x0C
#define INTEGRATOR_HDR_CTRL_REMAP       0x04

#define INTEGRATOR_BOOT_ROM_BASE	0x20000000
#define INTEGRATOR_HDR0_SDRAM_BASE      0x80000000

#define INTEGRATOR_DBG_ALPHA_OFFSET     0x00
#define INTEGRATOR_DBG_LEDS_OFFSET      0x04
#define INTEGRATOR_DBG_SWITCH_OFFSET    0x08

#define INTEGRATOR_DBG_BASE             0x1A000000
#define INTEGRATOR_DBG_ALPHA            (INTEGRATOR_DBG_BASE + INTEGRATOR_DBG_ALPHA_OFFSET)
#define INTEGRATOR_DBG_LEDS             (INTEGRATOR_DBG_BASE + INTEGRATOR_DBG_LEDS_OFFSET)
#define INTEGRATOR_DBG_SWITCH           (INTEGRATOR_DBG_BASE + INTEGRATOR_DBG_SWITCH_OFFSET)

#define INTEGRATOR_UART0_BASE                   0x16000000	 /*  UART 0 */
#define INTEGRATOR_UART1_BASE                   0x17000000	 /*  UART 1 */

#define INTEGRATOR_IRQCONT_BASE              	0x14000000

#define INTEGRATOR_IRQSTATUS			0x00
#define INTEGRATOR_IRQRAWSTATUS		 	0x04
#define INTEGRATOR_IRQENABLE			0x08
#define INTEGRATOR_IRQENABLESET	        	0x08
#define INTEGRATOR_IRQENABLECLEAR		0x0C

#define INTEGRATOR_IRQSOFT			0x10
#define INTEGRATOR_IRQSOFTSET		        0x10
#define INTEGRATOR_IRQSOFTCLEAR		        0x14

#define INTEGRATOR_FIQSTATUS			0x20
#define INTEGRATOR_FIQRAWSTATUS			0x24
#define INTEGRATOR_FIQENABLE			0x28
#define INTEGRATOR_FIQENABLESET			0x28
#define INTEGRATOR_FIQENABLECLEAR		0x2C

#define INTEGRATOR_IRQCONT_IRQSTATUS		(INTEGRATOR_IRQCONT_BASE + INTEGRATOR_IRQSTATUS)
#define INTEGRATOR_IRQCONT_IRQRAWSTATUS         (INTEGRATOR_IRQCONT_BASE + INTEGRATOR_IRQRAWSTATUS)
#define INTEGRATOR_IRQCONT_IRQENABLE            (INTEGRATOR_IRQCONT_BASE + INTEGRATOR_IRQENABLE)
#define INTEGRATOR_IRQCONT_IRQENABLESET         (INTEGRATOR_IRQCONT_BASE + INTEGRATOR_IRQENABLESET)
#define INTEGRATOR_IRQCONT_IRQENABLECLEAR       (INTEGRATOR_IRQCONT_BASE + INTEGRATOR_IRQENABLECLEAR)
#define INTEGRATOR_IRQCONT_IRQSOFT              (INTEGRATOR_IRQCONT_BASE + INTEGRATOR_IRQSOFT)
#define INTEGRATOR_IRQCONT_IRQSOFTSET           (INTEGRATOR_IRQCONT_BASE + INTEGRATOR_IRQSOFTSET)
#define INTEGRATOR_IRQCONT_IRQSOFTCLEAR         (INTEGRATOR_IRQCONT_BASE + INTEGRATOR_IRQSOFTCLEAR)
#define INTEGRATOR_IRQCONT_FIQSTATUS            (INTEGRATOR_IRQCONT_BASE + INTEGRATOR_FIQSTATUS)
#define INTEGRATOR_IRQCONT_FIQRAWSTATUS         (INTEGRATOR_IRQCONT_BASE + INTEGRATOR_FIQRAWSTATUS)
#define INTEGRATOR_IRQCONT_FIQENABLE            (INTEGRATOR_IRQCONT_BASE + INTEGRATOR_FIQENABLE)
#define INTEGRATOR_IRQCONT_FIQENABLESET         (INTEGRATOR_IRQCONT_BASE + INTEGRATOR_FIQENABLESET)
#define INTEGRATOR_IRQCONT_FIQENABLECLEAR       (INTEGRATOR_IRQCONT_BASE + INTEGRATOR_FIQENABLECLEAR)
#define INTEGRATOR_IRQCONT_FIQSOFT              (INTEGRATOR_IRQCONT_BASE + INTEGRATOR_FIQSOFT)

// Interrupt controller registers
#define CYG_DEVICE_ICTL_BASE            INTEGRATOR_IRQCONT_BASE
#define CYG_DEVICE_IRQ_Status \
    ((volatile cyg_uint32 *) (CYG_DEVICE_ICTL_BASE + INTEGRATOR_IRQSTATUS))
    // Current status, read only
#define CYG_DEVICE_IRQ_Enable \
    ((volatile cyg_uint32 *) (CYG_DEVICE_ICTL_BASE + INTEGRATOR_IRQENABLE))
    // Enable status, read only
#define CYG_DEVICE_IRQ_EnableSet \
    ((volatile cyg_uint32 *) (CYG_DEVICE_ICTL_BASE + INTEGRATOR_IRQENABLESET))
    // Enable (1's only), write only
#define CYG_DEVICE_IRQ_EnableClear \
    ((volatile cyg_uint32 *) (CYG_DEVICE_ICTL_BASE + INTEGRATOR_IRQENABLECLEAR))
    // Disable (1's only), write only

// Timer registers
#define INTEGRATOR_CT_BASE              0x13000000	 /*  Counter/Timers */

#define INTEGRATOR_TIMER0_BASE          INTEGRATOR_CT_BASE
#define INTEGRATOR_TIMER1_BASE          (INTEGRATOR_CT_BASE + 0x100)
#define INTEGRATOR_TIMER2_BASE          (INTEGRATOR_CT_BASE + 0x200)

#define CYG_DEVICE_TIMER_BASE           INTEGRATOR_TIMER2_BASE

#define CYG_DEVICE_TIMER_LOAD \
    ((volatile cyg_uint32 *) (CYG_DEVICE_TIMER_BASE + 0x00))
    // Load value, read/write
#define CYG_DEVICE_TIMER_CURRENT \
    ((volatile cyg_uint32 *) (CYG_DEVICE_TIMER_BASE + 0x04))
    // Current value, read
#define CYG_DEVICE_TIMER_CONTROL \
    ((volatile cyg_uint32 *) (CYG_DEVICE_TIMER_BASE + 0x08))
    // Control register, read/write
#define CYG_DEVICE_TIMER_CLEAR \
    ((volatile cyg_uint32 *) (CYG_DEVICE_TIMER_BASE + 0x0C))
    // Clears interrrupt, write only

// Clock/timer control register
#define CTL_ENABLE      0x80            // Bit   7: 1 - counter enabled
#define CTL_DISABLE     0x00            //          0 - counter disabled
#define CTL_FREERUN     0x00            // Bit   6: 0 - free running counter
#define CTL_PERIODIC    0x40            //          1 - periodic timer mode
#define CTL_SCALE_1     0x00            // Bits 32: 00 - Scale clock by 1
#define CTL_SCALE_16    0x04            //          01 - Scale by 16
#define CTL_SCALE_256   0x08            //          10 - Scale by 256
                                        //               12.8us/tick
/*-------------------------------------------------------------------------------
 *  From AMBA UART (PL010) Block Specification (ARM-0001-CUST-DSPC-A03)
 * -------------------------------------------------------------------------------
 *  UART Register Offsets.
 */
#define AMBA_UARTDR                     0x00	 /*  Data read or written from the interface. */
#define AMBA_UARTRSR                    0x04	 /*  Receive status register (Read). */
#define AMBA_UARTECR                    0x04	 /*  Error clear register (Write). */
#define AMBA_UARTLCR_H                  0x08	 /*  Line control register, high byte. */
#define AMBA_UARTLCR_M                  0x0C	 /*  Line control register, middle byte. */
#define AMBA_UARTLCR_L                  0x10	 /*  Line control register, low byte. */
#define AMBA_UARTCR                     0x14	 /*  Control register. */
#define AMBA_UARTFR                     0x18	 /*  Flag register (Read only). */
#define AMBA_UARTIIR                    0x1C	 /*  Interrupt indentification register (Read). */
#define AMBA_UARTICR                    0x1C	 /*  Interrupt clear register (Write). */
#define AMBA_UARTILPR                   0x20	 /*  IrDA low power counter register. */

#define AMBA_UARTRSR_OE                 0x08
#define AMBA_UARTRSR_BE                 0x04
#define AMBA_UARTRSR_PE                 0x02
#define AMBA_UARTRSR_FE                 0x01

#define AMBA_UARTFR_TXFF                0x20
#define AMBA_UARTFR_RXFE                0x10
#define AMBA_UARTFR_BUSY                0x08
#define AMBA_UARTFR_TMSK                (AMBA_UARTFR_TXFF + AMBA_UARTFR_BUSY)
 
#define AMBA_UARTCR_RTIE                0x40
#define AMBA_UARTCR_TIE                 0x20
#define AMBA_UARTCR_RIE                 0x10
#define AMBA_UARTCR_MSIE                0x08
#define AMBA_UARTCR_IIRLP               0x04
#define AMBA_UARTCR_SIREN               0x02
#define AMBA_UARTCR_UARTEN              0x01
 
#define AMBA_UARTLCR_H_WLEN_8           0x60
#define AMBA_UARTLCR_H_WLEN_7           0x40
#define AMBA_UARTLCR_H_WLEN_6           0x20
#define AMBA_UARTLCR_H_WLEN_5           0x00
#define AMBA_UARTLCR_H_FEN              0x10
#define AMBA_UARTLCR_H_STP2             0x08
#define AMBA_UARTLCR_H_EPS              0x04
#define AMBA_UARTLCR_H_PEN              0x02
#define AMBA_UARTLCR_H_BRK              0x01

#define AMBA_UARTIIR_RTIS               0x08
#define AMBA_UARTIIR_TIS                0x04
#define AMBA_UARTIIR_RIS                0x02
#define AMBA_UARTIIR_MIS                0x01

#define ARM_BAUD_460800                 1
#define ARM_BAUD_230400                 3
#define ARM_BAUD_115200                 7
#define ARM_BAUD_57600                  15
#define ARM_BAUD_38400                  23
#define ARM_BAUD_19200                  47
#define ARM_BAUD_14400                  63
#define ARM_BAUD_9600                   95
#define ARM_BAUD_4800                   191
#define ARM_BAUD_2400                   383
#define ARM_BAUD_1200                   767

// PCI Base area
#define INTEGRATOR_PCI_BASE		0x40000000
#define INTEGRATOR_PCI_SIZE		0x3FFFFFFF

// memory map as seen by the CPU on the local bus
#define CPU_PCI_IO_ADRS		0x60000000 	// PCI I/O space base
#define CPU_PCI_IO_SIZE		0x10000	

#define CPU_PCI_CNFG_ADRS	0x61000000	// PCI config space
#define CPU_PCI_CNFG_SIZE	0x1000000

#define PCI_MEM_BASE            0x40000000   // 512M to xxx
//  unused 256M from A0000000-AFFFFFFF might be used for I2O ???
#define PCI_IO_BASE             0x60000000   // 16M to xxx
//  unused (128-16)M from B1000000-B7FFFFFF
#define PCI_CONFIG_BASE         0x61000000   // 16M to xxx
//  unused ((128-16)M - 64K) from XXX

#define PCI_V3_BASE             0x62000000

// V3 PCI bridge controller
#define V3_BASE			0x62000000    // V360EPC registers

#define V3_PCI_VENDOR           0x00000000
#define V3_PCI_DEVICE           0x00000002
#define V3_PCI_CMD              0x00000004
#define V3_PCI_STAT             0x00000006
#define V3_PCI_CC_REV           0x00000008
#define V3_PCI_HDR_CF           0x0000000C
#define V3_PCI_IO_BASE          0x00000010
#define V3_PCI_BASE0            0x00000014
#define V3_PCI_BASE1            0x00000018
#define V3_PCI_SUB_VENDOR       0x0000002C
#define V3_PCI_SUB_ID           0x0000002E
#define V3_PCI_ROM              0x00000030
#define V3_PCI_BPARAM           0x0000003C
#define V3_PCI_MAP0             0x00000040
#define V3_PCI_MAP1             0x00000044
#define V3_PCI_INT_STAT         0x00000048
#define V3_PCI_INT_CFG          0x0000004C
#define V3_LB_BASE0             0x00000054
#define V3_LB_BASE1             0x00000058
#define V3_LB_MAP0              0x0000005E
#define V3_LB_MAP1              0x00000062
#define V3_LB_BASE2             0x00000064
#define V3_LB_MAP2              0x00000066
#define V3_LB_SIZE              0x00000068
#define V3_LB_IO_BASE           0x0000006E
#define V3_FIFO_CFG             0x00000070
#define V3_FIFO_PRIORITY        0x00000072
#define V3_FIFO_STAT            0x00000074
#define V3_LB_ISTAT             0x00000076
#define V3_LB_IMASK             0x00000077
#define V3_SYSTEM               0x00000078
#define V3_LB_CFG               0x0000007A
#define V3_PCI_CFG              0x0000007C
#define V3_DMA_PCI_ADR0         0x00000080
#define V3_DMA_PCI_ADR1         0x00000090
#define V3_DMA_LOCAL_ADR0       0x00000084
#define V3_DMA_LOCAL_ADR1       0x00000094
#define V3_DMA_LENGTH0          0x00000088
#define V3_DMA_LENGTH1          0x00000098
#define V3_DMA_CSR0             0x0000008B
#define V3_DMA_CSR1             0x0000009B
#define V3_DMA_CTLB_ADR0        0x0000008C
#define V3_DMA_CTLB_ADR1        0x0000009C
#define V3_DMA_DELAY            0x000000E0
#define V3_MAIL_DATA            0x000000C0
#define V3_PCI_MAIL_IEWR        0x000000D0
#define V3_PCI_MAIL_IERD        0x000000D2
#define V3_LB_MAIL_IEWR         0x000000D4
#define V3_LB_MAIL_IERD         0x000000D6
#define V3_MAIL_WR_STAT         0x000000D8
#define V3_MAIL_RD_STAT         0x000000DA
#define V3_QBA_MAP              0x000000DC

// SYSTEM register bits
#define V3_SYSTEM_M_RST_OUT             (1 << 15)
#define V3_SYSTEM_M_LOCK                (1 << 14)

//  PCI_CFG bits
#define V3_PCI_CFG_M_RETRY_EN           (1 << 10)
#define V3_PCI_CFG_M_AD_LOW1            (1 << 9)
#define V3_PCI_CFG_M_AD_LOW0            (1 << 8)

// PCI MAP register bits (PCI -> Local bus)
#define V3_PCI_MAP_M_MAP_ADR            0xFFF00000
#define V3_PCI_MAP_M_RD_POST_INH        (1 << 15)
#define V3_PCI_MAP_M_ROM_SIZE           (1 << 11 | 1 << 10)
#define V3_PCI_MAP_M_SWAP               (1 << 9 | 1 << 8)
#define V3_PCI_MAP_M_ADR_SIZE           0x000000F0
#define V3_PCI_MAP_M_REG_EN             (1 << 1)
#define V3_PCI_MAP_M_ENABLE             (1 << 0)

// 9 => 512M window size
#define V3_PCI_MAP_M_ADR_SIZE_512M      0x00000090

// A => 1024M window size
#define V3_PCI_MAP_M_ADR_SIZE_1024M     0x000000A0

// LB_BASE register bits (Local bus -> PCI)
#define V3_LB_BASE_M_MAP_ADR            0xFFF00000
#define V3_LB_BASE_M_SWAP               (1 << 8 | 1 << 9)
#define V3_LB_BASE_M_ADR_SIZE           0x000000F0
#define V3_LB_BASE_M_PREFETCH           (1 << 3)
#define V3_LB_BASE_M_ENABLE             (1 << 0)

// PCI COMMAND REGISTER bits
#define V3_COMMAND_M_FBB_EN             (1 << 9)
#define V3_COMMAND_M_SERR_EN            (1 << 8)
#define V3_COMMAND_M_PAR_EN             (1 << 6)
#define V3_COMMAND_M_MASTER_EN          (1 << 2)
#define V3_COMMAND_M_MEM_EN             (1 << 1)
#define V3_COMMAND_M_IO_EN              (1 << 0)

#define INTEGRATOR_SC_BASE		0x11000000
#define INTEGRATOR_SC_PCIENABLE_OFFSET	0x18
#define INTEGRATOR_SC_PCIENABLE \
			(INTEGRATOR_SC_BASE + INTEGRATOR_SC_PCIENABLE_OFFSET)



#define SZ_256M                         0x10000000

// Integrator EBI register definitions

#define INTEGRATOR_EBI_BASE 0x12000000

#define INTEGRATOR_EBI_CSR0_OFFSET      0x00
#define INTEGRATOR_EBI_CSR1_OFFSET      0x04
#define INTEGRATOR_EBI_CSR2_OFFSET      0x08
#define INTEGRATOR_EBI_CSR3_OFFSET      0x0C
#define INTEGRATOR_EBI_LOCK_OFFSET      0x20

#define INTEGRATOR_EBI_CSR0 (INTEGRATOR_EBI_BASE + INTEGRATOR_EBI_CSR0_OFFSET)
#define INTEGRATOR_EBI_CSR1 (INTEGRATOR_EBI_BASE + INTEGRATOR_EBI_CSR1_OFFSET)
#define INTEGRATOR_EBI_CSR2 (INTEGRATOR_EBI_BASE + INTEGRATOR_EBI_CSR2_OFFSET)
#define INTEGRATOR_EBI_CSR3 (INTEGRATOR_EBI_BASE + INTEGRATOR_EBI_CSR3_OFFSET)
#define INTEGRATOR_EBI_LOCK (INTEGRATOR_EBI_BASE + INTEGRATOR_EBI_LOCK_OFFSET)

#define INTEGRATOR_EBI_8_BIT            0x00
#define INTEGRATOR_EBI_16_BIT           0x01
#define INTEGRATOR_EBI_32_BIT           0x02
#define INTEGRATOR_EBI_WRITE_ENABLE     0x04
#define INTEGRATOR_EBI_SYNC             0x08
#define INTEGRATOR_EBI_WS_2             0x00
#define INTEGRATOR_EBI_WS_3             0x10
#define INTEGRATOR_EBI_WS_4             0x20
#define INTEGRATOR_EBI_WS_5             0x30
#define INTEGRATOR_EBI_WS_6             0x40
#define INTEGRATOR_EBI_WS_7             0x50
#define INTEGRATOR_EBI_WS_8             0x60
#define INTEGRATOR_EBI_WS_9             0x70
#define INTEGRATOR_EBI_WS_10            0x80
#define INTEGRATOR_EBI_WS_11            0x90
#define INTEGRATOR_EBI_WS_12            0xA0
#define INTEGRATOR_EBI_WS_13            0xB0
#define INTEGRATOR_EBI_WS_14            0xC0
#define INTEGRATOR_EBI_WS_15            0xD0
#define INTEGRATOR_EBI_WS_16            0xE0
#define INTEGRATOR_EBI_WS_17            0xF0

#define FL_SC_CONTROL			0x06	// Enable Flash Write and Vpp

/* 
 *  System Controller
 * 
 */
#define INTEGRATOR_SC_ID_OFFSET         0x00
#define INTEGRATOR_SC_OSC_OFFSET        0x04
#define INTEGRATOR_SC_CTRLS_OFFSET      0x08
#define INTEGRATOR_SC_CTRLC_OFFSET      0x0C
#define INTEGRATOR_SC_DEC_OFFSET        0x10
#define INTEGRATOR_SC_ARB_OFFSET        0x14
#define INTEGRATOR_SC_PCIENABLE_OFFSET  0x18
#define INTEGRATOR_SC_LOCK_OFFSET       0x1C

#define INTEGRATOR_SC_BASE              0x11000000
#define INTEGRATOR_SC_ID                (INTEGRATOR_SC_BASE + INTEGRATOR_SC_ID_OFFSET)
#define INTEGRATOR_SC_OSC               (INTEGRATOR_SC_BASE + INTEGRATOR_SC_OSC_OFFSET)
#define INTEGRATOR_SC_CTRLS             (INTEGRATOR_SC_BASE + INTEGRATOR_SC_CTRLS_OFFSET)
#define INTEGRATOR_SC_CTRLC             (INTEGRATOR_SC_BASE + INTEGRATOR_SC_CTRLC_OFFSET)
#define INTEGRATOR_SC_DEC               (INTEGRATOR_SC_BASE + INTEGRATOR_SC_DEC_OFFSET)
#define INTEGRATOR_SC_ARB               (INTEGRATOR_SC_BASE + INTEGRATOR_SC_ARB_OFFSET)
#define INTEGRATOR_SC_PCIENABLE         (INTEGRATOR_SC_BASE + INTEGRATOR_SC_PCIENABLE_OFFSET)
#define INTEGRATOR_SC_LOCK              (INTEGRATOR_SC_BASE + INTEGRATOR_SC_LOCK_OFFSET)

#endif //CYGONCE_HAL_INTEGRATOR_H

