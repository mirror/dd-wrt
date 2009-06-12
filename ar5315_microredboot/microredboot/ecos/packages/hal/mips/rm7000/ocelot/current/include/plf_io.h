#ifndef CYGONCE_PLF_IO_H
#define CYGONCE_PLF_IO_H

//=============================================================================
//
//      plf_io.h
//
//      Platform specific IO support
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
// Author(s):    hmt, jskov, nickg
// Contributors: hmt, jskov, nickg
// Date:         1999-08-09
// Purpose:      Ocelot/Galileo GT-64120A PCI IO support macros
// Description: 
// Usage:        #include <cyg/hal/plf_io.h>
//
// Note:         Based on information in 
//               "Galileo GT 64120A System Controller For 
//                RC4650/4700/5000 and RM526X/527X/7000 CPUs"
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#include <cyg/hal/hal_arch.h>           // address macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_intr.h>           // Interrupt vectors

//-----------------------------------------------------------------------------
// PCI access registers

//#define HAL_PCI_ADDRESS_WINDOW_1        0xAF000014
//#define HAL_PCI_ADDRESS_WINDOW_2        0xAF000018
//#define HAL_PCI_IO_WINDOW               0xAF000024
#define HAL_PCI_CONFIG_SPACE_DATA       0xb4000cfc
#define HAL_PCI_CONFIG_SPACE_ADDR       0xb4000cf8
//#define HAL_PCI_ENABLE_REG              0xAF000074

//-----------------------------------------------------------------------------
// Mappings for PCI memory and IO spaces

// These are the offsets programmed into the Galileo for setting up the
// CPU->PCI space mapping. These are put high to allow for 256MB RAM in
// kseg0/1 - but we'll probably have to put the RAM in kuseg anyway to get
// it all mapped (512MB). These mappings leave enough space for the
// PCI devices on the Ocelot regardless though.
//
// Note that the CPU addresses are going directly to the PCI bus, so
// the IO/MEM bases are the matching CPU address space locations, not
// zero.

#define HAL_OCELOT_PCI_IO_BASE                  0x10000000
#define HAL_OCELOT_PCI_IO_SIZE                  0x01000000  // 16 MB
#define HAL_OCELOT_PCI_MEM0_BASE                0x12000000
#define HAL_OCELOT_PCI_MEM0_SIZE                0x01000000  // 16 MB
#define HAL_OCELOT_PCI_MEM1_BASE                0x13000000
#define HAL_OCELOT_PCI_MEM1_SIZE                0x01000000  // 16 MB

// This is where the PCI spaces are mapped in the CPU's (virtual)
// address space. These are the uncached addresses.
#define HAL_PCI_PHYSICAL_MEMORY_BASE            CYGARC_UNCACHED_ADDRESS(0)
#define HAL_PCI_PHYSICAL_IO_BASE                CYGARC_UNCACHED_ADDRESS(0)

// Map PCI device resources starting from these addresses in PCI space.
#define HAL_PCI_ALLOC_BASE_MEMORY               HAL_OCELOT_PCI_MEM0_BASE
#define HAL_PCI_ALLOC_BASE_IO                   HAL_OCELOT_PCI_IO_BASE


// Uncached controller base
#define HAL_GALILEO_CONTROLLER_BASE             0xb4000000

//-----------------------------------------------------------------------------

#define HAL_GALILEO_PUTREG(r,d) \
    HAL_WRITE_UINT32((HAL_GALILEO_CONTROLLER_BASE + (r)), \
                     ((((d) & 0xff) << 24) | (((d) & 0xff00) << 8) | (((d) & 0xff0000) >> 8) | (((d) >> 24) & 0xff)))
#define HAL_GALILEO_GETREG(r)   \
    ({ cyg_uint32 d; HAL_READ_UINT32((HAL_GALILEO_CONTROLLER_BASE + (r)), d);\
       (((d & 0xff) << 24) | ((d & 0xff00) << 8) | ((d & 0xff0000) >> 8) | ((d >> 24) & 0xff)); })

// PCI config reads are special: all devices but the Galileo itself
// are in big-endian mode.  Fiddling the endian configs did not seem
// to make a difference.
#define HAL_GALILEO_PUTPCI(bus, devfn, r, data)                         \
    CYG_MACRO_START                                                     \
    if (0 == bus && 0 == devfn)                                         \
        HAL_GALILEO_PUTREG(r, data);                                    \
    else                                                                \
        HAL_WRITE_UINT32((HAL_GALILEO_CONTROLLER_BASE + (r)), data);    \
    CYG_MACRO_END

#define HAL_GALILEO_GETPCI(bus, devfn, r, data)                         \
    CYG_MACRO_START                                                     \
    if (0 == bus && 0 == devfn)                                         \
        data = HAL_GALILEO_GETREG(r);                                   \
    else                                                                \
        HAL_READ_UINT32((HAL_GALILEO_CONTROLLER_BASE + (r)), data);     \
    CYG_MACRO_END


extern cyg_uint32 cyg_hal_plf_pci_cfg_read_dword (cyg_uint32 bus,
						  cyg_uint32 devfn,
						  cyg_uint32 offset);
extern cyg_uint16 cyg_hal_plf_pci_cfg_read_word  (cyg_uint32 bus,
						  cyg_uint32 devfn,
						  cyg_uint32 offset);
extern cyg_uint8 cyg_hal_plf_pci_cfg_read_byte   (cyg_uint32 bus,
						  cyg_uint32 devfn,
						  cyg_uint32 offset);
extern void cyg_hal_plf_pci_cfg_write_dword (cyg_uint32 bus,
					     cyg_uint32 devfn,
					     cyg_uint32 offset,
					     cyg_uint32 val);
extern void cyg_hal_plf_pci_cfg_write_word  (cyg_uint32 bus,
					     cyg_uint32 devfn,
					     cyg_uint32 offset,
					     cyg_uint16 val);
extern void cyg_hal_plf_pci_cfg_write_byte   (cyg_uint32 bus,
					      cyg_uint32 devfn,
					      cyg_uint32 offset,
					      cyg_uint8 val);

//-----------------------------------------------------------------------------

// Initialize the PCI bus.
externC void cyg_hal_plf_pci_init(void);
#define HAL_PCI_INIT() cyg_hal_plf_pci_init()

// Read a value from the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and
// offset.
#define HAL_PCI_CFG_READ_UINT8( __bus, __devfn, __offset, __val )  \
    __val = cyg_hal_plf_pci_cfg_read_byte((__bus),  (__devfn), (__offset))
    
#define HAL_PCI_CFG_READ_UINT16( __bus, __devfn, __offset, __val ) \
    __val = cyg_hal_plf_pci_cfg_read_word((__bus),  (__devfn), (__offset))

#define HAL_PCI_CFG_READ_UINT32( __bus, __devfn, __offset, __val ) \
    __val = cyg_hal_plf_pci_cfg_read_dword((__bus),  (__devfn), (__offset))

// Write a value to the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
#define HAL_PCI_CFG_WRITE_UINT8( __bus, __devfn, __offset, __val )  \
    cyg_hal_plf_pci_cfg_write_byte((__bus),  (__devfn), (__offset), (__val))

#define HAL_PCI_CFG_WRITE_UINT16( __bus, __devfn, __offset, __val ) \
    cyg_hal_plf_pci_cfg_write_word((__bus),  (__devfn), (__offset), (__val))

#define HAL_PCI_CFG_WRITE_UINT32( __bus, __devfn, __offset, __val ) \
    cyg_hal_plf_pci_cfg_write_dword((__bus),  (__devfn), (__offset), (__val))


//-----------------------------------------------------------------------------
// Resources

// Translate the PCI interrupt requested by the device (INTA#, INTB#,
// INTC# or INTD#) to the associated CPU interrupt (i.e., HAL vector).
// We don't actually know what the mappings are at present for this
// board. The following is therefore just a temporary guess until
// we can find out.

#define HAL_PCI_TRANSLATE_INTERRUPT( __bus, __devfn, __vec, __valid)          \
    CYG_MACRO_START                                                           \
    cyg_uint8 __req;                                                          \
    HAL_PCI_CFG_READ_UINT8(__bus, __devfn, CYG_PCI_CFG_INT_PIN, __req);       \
    if (0 != __req) {                                                         \
        CYG_ADDRWORD __translation[4] = {                                     \
            CYGNUM_HAL_INTERRUPT_PCI_INTA,  /* INTA# */                       \
            CYGNUM_HAL_INTERRUPT_PCI_INTB,  /* INTB# */                       \
            CYGNUM_HAL_INTERRUPT_PCI_INTC,  /* INTC# */                       \
            CYGNUM_HAL_INTERRUPT_PCI_INTD };/* INTD# */                       \
                                                                              \
        __vec = __translation[(((__req-1)+CYG_PCI_DEV_GET_DEV(__devfn))&3)];  \
                                                                              \
        __valid = true;                                                       \
    } else {                                                                  \
        /* Device will not generate interrupt requests. */                    \
        __valid = false;                                                      \
    }                                                                         \
    CYG_MACRO_END

// Galileo GT64120 on MIPS Ocelot requires special processing.
// First, it will hang when accessing device 31 on the local bus.
// Second, we need to ignore the GT64120 so we can set it up
// outside the generic PCI library.
#define HAL_PCI_IGNORE_DEVICE(__bus, __dev, __fn) \
    ((__bus) == 0 && ((__dev) == 0 || (__dev) == 31))


//-----------------------------------------------------------------------------
// PCI / Galileo register definitions
#define HAL_GALILEO_PCI0_CONFIG_ADDR_ConfigEn   BIT31
#define HAL_GALILEO_PCI0_STATUS_COMMAND_REGNUM  0x04
#define HAL_GALILEO_PCI0_BIST_REGNUM            0x0C
#define HAL_GALILEO_PCI0_SCS32_BASE_REGNUM      0x14
#define HAL_GALILEO_PCI0_CONFIG_MEMEn           0x2
#define HAL_GALILEO_PCI0_CONFIG_MasEn           0x4
#define HAL_GALILEO_PCI0_CONFIG_SErrEn          0x100
#define HAL_GALILEO_PCI0_LAT_TIMER_VAL          0x800
#define HAL_GALILEO_PCI0_TIMEOUT_RETRY_VALUE    0x00ffffff

#define HAL_GALILEO_PCI_INTERNAL_COMMAND_OFFSET 0xC00
#define HAL_GALILEO_PCI0_TIMEOUT_RETRY_OFFSET   0xc04
#define HAL_GALILEO_PCI0_SCS10_SIZE_OFFSET      0xc08
#define HAL_GALILEO_PCI0_SCS32_SIZE_OFFSET      0xc0c
#define HAL_GALILEO_PCI0_SCS20_SIZE_OFFSET      0xc10
#define HAL_GALILEO_PCI0_CS3_SIZE_OFFSET        0xc14
#define HAL_GALILEO_BAR_ENA_OFFSET		0xc3c
#  define HAL_GALILEO_BAR_ENA_SWCS3  (1 << 0)
#  define HAL_GALILEO_BAR_ENA_SWCS32 (1 << 1)
#  define HAL_GALILEO_BAR_ENA_SWCS10 (1 << 2)
#  define HAL_GALILEO_BAR_ENA_IO     (1 << 3)
#  define HAL_GALILEO_BAR_ENA_MEM    (1 << 4)
#  define HAL_GALILEO_BAR_ENA_CS3    (1 << 5)
#  define HAL_GALILEO_BAR_ENA_CS20   (1 << 6)
#  define HAL_GALILEO_BAR_ENA_SCS32  (1 << 7)
#  define HAL_GALILEO_BAR_ENA_SCS10  (1 << 8)
#define HAL_GALILEO_PCI0_CONFIG_ADDR_OFFSET     0xcf8
#  define HAL_GALILEO_PCI0_CONFIG_ADDR_ENABLE (1 << 31)
#define HAL_GALILEO_PCI0_CONFIG_DATA_OFFSET     0xcfc

#define HAL_OCELOT_NULL_DEVNUM                   0x0

// GALILEO Interrupts
#define HAL_GALILEO_IRQ_CAUSE_OFFSET		0xc18
#  define HAL_GALILEO_IRQCAUSE_INTSUM	(1 << 0)
#  define HAL_GALILEO_IRQCAUSE_MEMOUT	(1 << 1)
#  define HAL_GALILEO_IRQCAUSE_DMAOUT	(1 << 2)
#  define HAL_GALILEO_IRQCAUSE_CPUOUT	(1 << 3)
#  define HAL_GALILEO_IRQCAUSE_DMA0	(1 << 4)
#  define HAL_GALILEO_IRQCAUSE_DMA1	(1 << 5)
#  define HAL_GALILEO_IRQCAUSE_DMA2	(1 << 6)
#  define HAL_GALILEO_IRQCAUSE_DMA3	(1 << 7)
#  define HAL_GALILEO_IRQCAUSE_T0	(1 << 8)
#  define HAL_GALILEO_IRQCAUSE_T1	(1 << 9)
#  define HAL_GALILEO_IRQCAUSE_T2	(1 << 10)
#  define HAL_GALILEO_IRQCAUSE_T3	(1 << 11)
#  define HAL_GALILEO_IRQCAUSE_MASRD	(1 << 12)
#  define HAL_GALILEO_IRQCAUSE_SLVWR	(1 << 13)
#  define HAL_GALILEO_IRQCAUSE_MASWR	(1 << 14)
#  define HAL_GALILEO_IRQCAUSE_SLVRD	(1 << 15)
#  define HAL_GALILEO_IRQCAUSE_AERR	(1 << 16)
#  define HAL_GALILEO_IRQCAUSE_MERR	(1 << 17)
#  define HAL_GALILEO_IRQCAUSE_MASABT	(1 << 18)
#  define HAL_GALILEO_IRQCAUSE_TARABT	(1 << 19)
#  define HAL_GALILEO_IRQCAUSE_RETRY	(1 << 20)
#  define HAL_GALILEO_IRQCAUSE_CPUSUM	(1 << 30)
#  define HAL_GALILEO_IRQCAUSE_PCISUM	(1 << 31)
#define HAL_GALILEO_HIRQ_CAUSE_OFFSET		0xc98
#define HAL_GALILEO_CPUIRQ_MASK_OFFSET          0xc1c
#define HAL_GALILEO_CPUHIRQ_MASK_OFFSET         0xc9c

/* Galileo Memory Controller registers */
#define HAL_GALILEO_SDRAM_DUPLICATE_BANK_ADDR   BIT20
#define HAL_GALILEO_SDRAM_BANK_INTERLEAVE_DIS   BIT14
#define HAL_GALILEO_CPU_DECODE_SHIFT            21
#define HAL_GALILEO_DEV_DECODE_SHIFT            20
#define HAL_GALILEO_SDRAM_SRAS_TO_SCAS_DELAY_3C BIT10
#define HAL_GALILEO_SDRAM_WIDTH_64BIT           BIT6
#define HAL_GALILEO_SDRAM_SRAS_PRECHARGE_3C     BIT3
#define HAL_GALILEO_SDRAM_BANK0_CASLAT_2        BIT0
#define HAL_GALILEO_SDRAM_BANK0_SZ_64M          BIT11
#define HAL_GALILEO_SDRAM_NUM_BANKS_4           BIT5
#define HAL_GALILEO_SDRAM_BANK0_PARITY          BIT8
#define HAL_GALILEO_SDRAM_CFG_RAM_WIDTH         BIT15
#define HAL_GALILEO_PCI0_CONFIG_ADDR_ConfigEn   BIT31
#define HAL_GALILEO_PCI0_STATUS_COMMAND_REGNUM  0x04
#define HAL_GALILEO_PCI0_BIST_REGNUM            0x0C
#define HAL_GALILEO_PCI0_SCS32_BASE_REGNUM      0x14
#define HAL_GALILEO_PCI0_CONFIG_MEMEn           0x2
#define HAL_GALILEO_PCI0_CONFIG_MasEn           0x4
#define HAL_GALILEO_PCI0_CONFIG_SErrEn          0x100
#define HAL_GALILEO_PCI0_LAT_TIMER_VAL          0x800
#define HAL_GALILEO_PCI0_TIMEOUT_RETRY_VALUE    0x00ffffff

#define HAL_GALILEO_SDRAM_BANK0_OFFSET          0x44c
#define HAL_GALILEO_SDRAM_BANK2_OFFSET          0x454
#define HAL_GALILEO_SDRAM_CONFIG_OFFSET         0x448

#define HAL_GALILEO_SCS10_LD_OFFSET             0x008
#define HAL_GALILEO_SCS10_HD_OFFSET             0x010
#define HAL_GALILEO_SCS32_LD_OFFSET             0x018
#define HAL_GALILEO_SCS32_HD_OFFSET             0x020
#define HAL_GALILEO_CS20_LD_OFFSET              0x028
#define HAL_GALILEO_CS20_HD_OFFSET              0x030
#define HAL_GALILEO_PCIIO_LD_OFFSET             0x048
#define HAL_GALILEO_PCIIO_HD_OFFSET             0x050
#define HAL_GALILEO_PCIMEM0_LD_OFFSET           0x058
#define HAL_GALILEO_PCIMEM0_HD_OFFSET           0x060
#define HAL_GALILEO_PCIMEM1_LD_OFFSET           0x080
#define HAL_GALILEO_PCIMEM1_HD_OFFSET           0x088
#define HAL_GALILEO_PCI1IO_LD_OFFSET            0x090
#define HAL_GALILEO_PCI1IO_HD_OFFSET            0x098
#define HAL_GALILEO_PCI1MEM0_LD_OFFSET          0x0a0
#define HAL_GALILEO_PCI1MEM0_HD_OFFSET          0x0a8
#define HAL_GALILEO_PCI1MEM1_LD_OFFSET          0x0b0
#define HAL_GALILEO_PCI1MEM1_HD_OFFSET          0x0b8
#define HAL_GALILEO_PCIIO_REMAP_OFFSET          0x0f0
#define HAL_GALILEO_PCIMEM0_REMAP_OFFSET        0x0f8
#define HAL_GALILEO_PCIMEM1_REMAP_OFFSET        0x100
#define HAL_GALILEO_SCS0_LD_OFFSET              0x400
#define HAL_GALILEO_SCS0_HD_OFFSET              0x404
#define HAL_GALILEO_SCS1_LD_OFFSET              0x408
#define HAL_GALILEO_SCS1_HD_OFFSET              0x40c
#define HAL_GALILEO_SCS2_LD_OFFSET              0x410
#define HAL_GALILEO_SCS2_HD_OFFSET              0x414
#define HAL_GALILEO_SCS3_LD_OFFSET              0x418
#define HAL_GALILEO_SCS3_HD_OFFSET              0x41c
#define HAL_GALILEO_CS0_LD_OFFSET               0x420
#define HAL_GALILEO_CS0_HD_OFFSET               0x424
#define HAL_GALILEO_CS1_LD_OFFSET               0x428
#define HAL_GALILEO_CS1_HD_OFFSET               0x42c
#define HAL_GALILEO_CS2_LD_OFFSET               0x430
#define HAL_GALILEO_CS2_HD_OFFSET               0x434
#define HAL_GALILEO_CPU_DECODE_SHIFT            21

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
