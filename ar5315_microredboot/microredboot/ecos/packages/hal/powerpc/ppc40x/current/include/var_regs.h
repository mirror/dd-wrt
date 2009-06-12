#ifndef CYGONCE_HAL_VAR_REGS_H
#define CYGONCE_HAL_VAR_REGS_H

//==========================================================================
//
//      var_regs.h
//
//      PowerPC 40x variant CPU definitions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002, 2003 Gary Thomas
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
// Author(s):    jskov
// Contributors: jskov,gthomas
// Date:         2000-08-27
// Purpose:      Provide PPC40x register definitions
// Description:  Provide PPC40x register definitions
//               The short definitions (sans CYGARC_REG_) are exported only
//               if CYGARC_HAL_COMMON_EXPORT_CPU_MACROS is defined.
// Usage:        Included via the architecture register header:
//               #include <cyg/hal/ppc_regs.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/plf_regs.h>

//--------------------------------------------------------------------------
// Hardware control (usage depends on chip model)
#define CYGARC_REG_HID0   1008

#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#define HID0       CYGARC_REG_HID0
#endif // ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

//--------------------------------------------------------------------------
// MMU control.
#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

#define SPR_ESR         980            // Exception syndrome
#define SPR_EVPR        982            // Exception vector prefix
#define SPR_PID         945            // Process ID
#define SPR_CCR0        0x3B3          // Core configuration register

#define M_EPN_EPNMASK  0xfffff000      // effective page no mask
#define M_EPN_EV       0x00000040      // entry valid
#define M_EPN_SIZE(n)  (n<<7)          // entry size (0=1K, 1=4K, ... 7=16M)

#define M_RPN_RPNMASK  0xfffff000      // real page no mask
#define M_RPN_EX       0x00000200      // execute enable
#define M_RPN_WR       0x00000100      // write enable
#define M_RPN_W        0x00000008      // write-through (when cache enabled)
#define M_RPN_I        0x00000004      // cache inhibited
#define M_RPN_M        0x00000002      // memory coherent (not implemented)
#define M_RPN_G        0x00000001      // guarded

#define CYGARC_TLBWE(_id_, _hi_, _lo_) \
        asm volatile ("tlbwe %1,%0,0; tlbwe %2,%0,1" :: "r"(_id_), "r"(_hi_), "r"(_lo_));

#define CYGARC_TLBRE(_id_, _hi_, _lo_) \
        asm volatile ("tlbre %0,%2,0; tlbre %1,%2,1" : "=r"(_hi_), "=r"(_lo_) : "r"(_id_));

#endif // ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

//--------------------------------------------------------------------------
// Device control register access macros.
#define CYGARC_MTDCR(_dcr_, _v_) \
    asm volatile ("mtdcr %0, %1;" :: "I" (_dcr_), "r" (_v_));
#define CYGARC_MFDCR(_dcr_, _v_) \
    asm volatile ("mfdcr %0, %1;" : "=r" (_v_) : "I" (_dcr_));

#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

// Interrupt control (device) registers
#if defined(CYGHWR_HAL_POWERPC_PPC4XX_403)
#define DCR_EXIER  66
#define DCR_EXISR  64
#define DCR_IOCR   160
#endif

#if defined(CYGHWR_HAL_POWERPC_PPC4XX_405) || defined(CYGHWR_HAL_POWERPC_PPC4XX_405GP)
// Interrupt controller
#define DCR_UIC0_SR   0xC0  // Status register
#define DCR_UIC0_ER   0xC2  // Enable register
#define DCR_UIC0_CR   0xC3  // Critical [or not]
#define DCR_UIC0_PR   0xC4  // Polarity (high/low)
#define DCR_UIC0_TR   0xC5  // Trigger (level/edge)
#define DCR_UIC0_MSR  0xC6  // Masked status
#define DCR_UIC0_VR   0xC7  // Vector
#define DCR_UIC0_VCR  0xC8  // Vector configuration

// PPC 405GP control registers (in DCR space)
#define DCR_SDRAM0_CFGADDR   0x10
#define DCR_SDRAM0_CFGDATA   0x11
#define DCR_EBC0_CFGADDR     0x12
#define DCR_EBC0_CFGDATA     0x13
#define DCR_CPC0_CR0         0xB1
#define DCR_CPC0_CR1         0xB2
#define DCR_CPC0_ECR         0xAA
#define DCR_CPC0_ECID0       0xA8     // 64 bit unique chip serial number
#define DCR_CPC0_ECID1       0xA9     // 64 bit unique chip serial number

// External bus controller (indirect via EBC0_CFGADDR/EBC0_CFGDATA)
#define DCR_EBC0_B0CR        0x00
#define DCR_EBC0_B1CR        0x01
#define DCR_EBC0_B2CR        0x02
#define DCR_EBC0_B3CR        0x03
#define DCR_EBC0_B4CR        0x04
#define DCR_EBC0_B5CR        0x05
#define DCR_EBC0_B6CR        0x06
#define DCR_EBC0_B7CR        0x07
#define DCR_EBC0_B0AP        0x10
#define DCR_EBC0_B1AP        0x11
#define DCR_EBC0_B2AP        0x12
#define DCR_EBC0_B3AP        0x13
#define DCR_EBC0_B4AP        0x14
#define DCR_EBC0_B5AP        0x15
#define DCR_EBC0_B6AP        0x16
#define DCR_EBC0_B7AP        0x17
#define DCR_EBC0_BEAR        0x20
#define DCR_EBC0_BESR0       0x21
#define DCR_EBC0_BESR1       0x22
#define DCR_EBC0_CFG         0x23

// SDRAM controller
#define DCR_SDRAM0_CFG       0x20     // Memory controller options
#define DCR_SDRAM0_STATUS    0x24     // Controller status
#define DCR_SDRAM0_RTR       0x30     // Refresh timer
#define DCR_SDRAM0_PMIT      0x34     // Power management idle timer
#define DCR_SDRAM0_B0CR      0x40     // Bank 0 configuration
#define DCR_SDRAM0_B1CR      0x44     // Bank 1 configuration
#define DCR_SDRAM0_TR        0x80     // Timing

// On-chip memory
#define DCR_OCM0_ISARC       0x18     // Instruction side address compare
#define DCR_OCM0_ISCNTL      0x19     // Instruction side control
#define DCR_OCM0_DSARC       0x1A     // Data side address compare
#define DCR_OCM0_DSCNTL      0x1B     // Data side control

// I2C controller
#define IIC0_MDBUF           ((_PPC405GP_IIC0)+0x00)  // Master data buffer
#define IIC0_SDBUF           ((_PPC405GP_IIC0)+0x02)  // Slave data buffer
#define IIC0_LMADR           ((_PPC405GP_IIC0)+0x04)  // Master address (low)
#define IIC0_HMADR           ((_PPC405GP_IIC0)+0x05)  // Master address (high)
#define IIC0_CNTL            ((_PPC405GP_IIC0)+0x06)  // Control
#define IIC0_CNTL_HMT           0x80   // Halt master transfer
#define IIC0_CNTL_AMD           0x40   // Address mode (0=7 bit, 1=10 bit)
#define IIC0_CNTL_TCT           0x30   // Transfer size (1-4 bytes)
#define IIC0_CNTL_TCT_SHIFT         4
#define IIC0_CNTL_TCT_1             0x00  
#define IIC0_CNTL_TCT_2             0x10
#define IIC0_CNTL_TCT_3             0x20
#define IIC0_CNTL_TCT_4             0x30
#define IIC0_CNTL_RPST          0x08   // Repeated start
#define IIC0_CNTL_CHT           0x04   // Chain transfer
#define IIC0_CNTL_RW            0x02   // Read/write
#define IIC0_CNTL_RW_READ           0x02
#define IIC0_CNTL_RW_WRITE          0x00
#define IIC0_CNTL_PT            0x01   // Pending transfer
#define IIC0_MDCNTL          ((_PPC405GP_IIC0)+0x07)  // Mode control
#define IIC0_MDCNTL_FSDB        0x80   // Flush slave data buffer
#define IIC0_MDCNTL_FMDB        0x40   // Flush master data buffer
#define IIC0_MDCNTL_FSM         0x10   // Fast/standard mode (1=fast)
#define IIC0_MDCNTL_ESM         0x08   // Enable slave mode
#define IIC0_MDCNTL_EINT        0x04   // Enable interrupt
#define IIC0_MDCNTL_EUBS        0x02   // Exit unknown bus state
#define IIC0_MDCNTL_HSCL        0x01   // Hold serial clock low (slave only)
#define IIC0_STS             ((_PPC405GP_IIC0)+0x08)  // Status
#define IIC0_STS_SSS            0x80   // Slave operation in progress
#define IIC0_STS_SLPR           0x40   // Sleep request
#define IIC0_STS_MDBS           0x20   // Master data buffer status (0=empty)
#define IIC0_STS_MDBF           0x10   // Master data buffer status (1=full)
#define IIC0_STS_SCMP           0x08   // Stop complete
#define IIC0_STS_ERR            0x04   // Error
#define IIC0_STS_IRQA           0x02   // Interrupt active
#define IIC0_STS_PT             0x01   // Transfer pending
#define IIC0_EXTSTS          ((_PPC405GP_IIC0)+0x09)  // Extended status
#define IIC0_EXTSTS_IRQP        0x80   // Interrupt pending
#define IIC0_EXTSTS_BCS         0x70   // Bus status
#define IIC0_EXTSTS_BCS_ss          0x10   // Slave selected
#define IIC0_EXTSTS_BCS_st          0x20   // Slave transfer
#define IIC0_EXTSTS_BCS_mt          0x30   // Master transfer
#define IIC0_EXTSTS_BCS_free        0x40   // Bus free
#define IIC0_EXTSTS_BCS_busy        0x50   // Bus busy
#define IIC0_EXTSTS_IRQD        0x08   // IRQ on deck 
#define IIC0_EXTSTS_LA          0x04   // Lost arbitration
#define IIC0_EXTSTS_ICT         0x02   // Incomplete transfer
#define IIC0_EXTSTS_XFRA        0x01   // Transfer aborted
#define IIC0_LSADR           ((_PPC405GP_IIC0)+0x0A)  // Slave address (low)
#define IIC0_HSADR           ((_PPC405GP_IIC0)+0x0B)  // Slave address (high)
#define IIC0_CLKDIV          ((_PPC405GP_IIC0)+0x0C)  // Clock divide
#define IIC0_INTRMSK         ((_PPC405GP_IIC0)+0x0D)  // Interrupt mask
#define IIC0_XFRCNT          ((_PPC405GP_IIC0)+0x0E)  // Transfer count
#define IIC0_XFRCNT_STC         0x70   // Slave transfer count
#define IIC0_XFRCNT_STC_SHIFT   4
#define IIC0_XFRCNT_MTC         0x07   // Master transfer count
#define IIC0_XFRCNT_MTC_SHIFT   0         
#define IIC0_XTCNTLSS        ((_PPC405GP_IIC0)+0x0F)  // Extended control & slave status
#define IIC0_DIRECT          ((_PPC405GP_IIC0)+0x10)  // Direct control over I/O lines

// GPIO (General Purpose I/O)
#define GPIO_OR              ((_PPC405GP_GPIO)+0x00)  // Output register
#define GPIO_TCR             ((_PPC405GP_GPIO)+0x04)  // Tri-state control
#define GPIO_OSRH            ((_PPC405GP_GPIO)+0x08)  // Output select (high)
#define GPIO_OSRL            ((_PPC405GP_GPIO)+0x0C)  // Output select (low)
#define GPIO_TSRH            ((_PPC405GP_GPIO)+0x10)  // Tri-state select (high)
#define GPIO_TSRL            ((_PPC405GP_GPIO)+0x14)  // Tri-state select (low)
#define GPIO_ODR             ((_PPC405GP_GPIO)+0x18)  // Open drain
#define GPIO_IR              ((_PPC405GP_GPIO)+0x1C)  // Input register
#define GPIO_RR              ((_PPC405GP_GPIO)+0x20)  // Receive register
#define GPIO_ISRH            ((_PPC405GP_GPIO)+0x30)  // Input select (high)
#define GPIO_ISRL            ((_PPC405GP_GPIO)+0x34)  // Input select (low)

// PCI

// PCI Bridge
#define PCIL0_PMM0LA         ((_PPC405GP_PCI_BRIDGE)+0x00)     // PMM 0 Local address
#define PCIL0_PMM0MA         ((_PPC405GP_PCI_BRIDGE)+0x04)     // PMM 0 Mask/Attributes
#define PCIL0_PMM0PCILA      ((_PPC405GP_PCI_BRIDGE)+0x08)     // PMM 0 PCI Low Address
#define PCIL0_PMM0PCIHA      ((_PPC405GP_PCI_BRIDGE)+0x0C)     // PMM 0 PCI High Address
#define PCIL0_PMM1LA         ((_PPC405GP_PCI_BRIDGE)+0x10)     // PMM 1 Local address
#define PCIL0_PMM1MA         ((_PPC405GP_PCI_BRIDGE)+0x14)     // PMM 1 Mask/Attributes
#define PCIL0_PMM1PCILA      ((_PPC405GP_PCI_BRIDGE)+0x18)     // PMM 1 PCI Low Address
#define PCIL0_PMM1PDIHA      ((_PPC405GP_PCI_BRIDGE)+0x1C)     // PMM 1 PCI High Address
#define PCIL0_PMM2LA         ((_PPC405GP_PCI_BRIDGE)+0x20)     // PMM 2 Local address
#define PCIL0_PMM2MA         ((_PPC405GP_PCI_BRIDGE)+0x24)     // PMM 2 Mask/Attributes
#define PCIL0_PMM2PCILA      ((_PPC405GP_PCI_BRIDGE)+0x28)     // PMM 2 PCI Low Address
#define PCIL0_PMM2PCIHA      ((_PPC405GP_PCI_BRIDGE)+0x2C)     // PMM 2 PCI High Address
#define PCIL0_PTM1MS         ((_PPC405GP_PCI_BRIDGE)+0x30)     // PTM 1 Memory Size/Attribute
#define PCIL0_PTM1LA         ((_PPC405GP_PCI_BRIDGE)+0x34)     // PTM 1 Local address
#define PCIL0_PTM2MS         ((_PPC405GP_PCI_BRIDGE)+0x38)     // PTM 2 Memory Size/Attribute
#define PCIL0_PTM2LA         ((_PPC405GP_PCI_BRIDGE)+0x3C)     // PTM 2 Local address

// Access to local/bridge PCI configuration registers
#define PCIC0_CFGADDR        ((_PPC405GP_PCI_IO)+0x00C00000)   // Indirect address pointer
#define PCIC0_CFGDATA        ((_PPC405GP_PCI_IO)+0x00C00004)   // Indirect data

// Debug control registers (SPR)
#define SPR_DBCR0       0x3F2

#endif // PPC4XX_405 || PPC4XX_405GP

// Timer control (special) registers
#define SPR_PIT    987
#define SPR_TCR    986
#define SPR_TSR    984

// Interval and watchdog timer control
#define TCR_WP          0xC0000000  // Watchdog timer period
#define TCR_WP_17       0x00000000  //   2^17 clocks
#define TCR_WP_21       0x40000000  //   2^21 clocks
#define TCR_WP_25       0x80000000  //   2^25 clocks
#define TCR_WP_29       0xC0000000  //   2^29 clocks
#define TCR_WRC         0x30000000  // Reset control
#define TCR_WRC_None    0x00000000  //   No reset on timeout
#define TCR_WRC_Core    0x10000000  //   Reset core on timeout
#define TCR_WRC_Chip    0x20000000  //   Reset chip on timeout
#define TCR_WRC_System  0x30000000  //   Reset system on timeout
#define TCR_WIE         0x08000000  // Watchdog interrupt enable
#define TCR_PIE         0x04000000  // Programmable timer interrupt
#define TCR_FP          0x03000000  // Fixed timer interval
#define TCR_FP_9        0x00000000  //   2^9 clocks
#define TCR_FP_13       0x01000000  //   2^13 clocks
#define TCR_FP_17       0x02000000  //   2^17 clocks
#define TCR_FP_21       0x03000000  //   2^21 clocks
#define TCR_FIE         0x00800000  // Fixed timer interrupt
#define TCR_ARE         0x00400000  // Auto-reload enable

// Interval and watchdog status
#define TSR_ENW         0x80000000  // Enable next watchdog
#define TSR_WIS         0x40000000  // Watchdog interrupt pending
#define TSR_WRS         0x30000000  // Watchdog reset state
#define TSR_WRS_None    0x00000000  //   No watchdog reset
#define TSR_WRS_Core    0x10000000  //   Core reset by watchdog
#define TSR_WRS_Chip    0x20000000  //   Chip reset by watchdog
#define TSR_WRS_System  0x30000000  //   System reset by watchdog
#define TSR_PIS         0x08000000  // Programmable timer interrupt
#define TSR_FIS         0x04000000  // Fixed timer interrupt

// Debug registers
#define SPR_DBSR        1008
#define SPR_DBCR        1010

#define DBCR_IDM        0x40000000  // Internal debug enable
#define DBCR_IC         0x08000000  // Instruction completion

#endif //  CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

#if defined(CYGHWR_HAL_POWERPC_PPC4XX_405) || defined(CYGHWR_HAL_POWERPC_PPC4XX_405GP)
//
// Memory mapped peripherals
//
#define _PPC405GP_UART0            0xEF600300 
#define _PPC405GP_UART1            0xEF600400 
#define _PPC405GP_IIC0             0xEF600500 
#define _PPC405GP_OPB_ARBITER      0xEF600600 
#define _PPC405GP_GPIO             0xEF600700 
#define _PPC405GP_ENET             0xEF600800 
#define _PPC405GP_PCI_BRIDGE       0xEF400000
#define _PPC405GP_PCI_IO           0xEE000000
//
// Window to PCI memory space (256MB, must be configured by target)
#define _PPC405GP_PCI_MEM          0xA0000000
//
// On-chip memory (4K only, must be configured by target platform)
//
#define _PPC405GP_OCM              0xD0000000
#endif // 405GP

//-----------------------------------------------------------------------------
#endif // ifdef CYGONCE_HAL_VAR_REGS_H
// End of var_regs.h
