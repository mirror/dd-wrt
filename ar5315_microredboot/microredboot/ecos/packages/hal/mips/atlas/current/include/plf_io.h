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
// Author(s):    dmoseley
// Contributors: dmoseley, michael anburaj <michaelanburaj@hotmail.com>
// Date:         2000-06-06
// Purpose:      Atlas platform IO support
// Description: 
// Usage:        #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_misc.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/plf_intr.h>

#ifdef __ASSEMBLER__
#define HAL_REG(x)              x
#else
#define HAL_REG(x)              (volatile CYG_WORD *)(x)
#endif

//-----------------------------------------------------------------------------

/* Atlas Memory Definitions */
#define HAL_ATLAS_RAM_BASE                      0x00000000
#define HAL_ATLAS_PCI_MEM0_BASE                 0x08000000
#define HAL_ATLAS_PCI_MEM0_SIZE                 0x08000000  // 128 MB
#define HAL_ATLAS_PCI_MEM1_BASE                 0x10000000
#define HAL_ATLAS_PCI_MEM1_SIZE                 0x08000000  // 128 MB
#define HAL_ATLAS_PCI_IO_BASE                   0x18000000
#define HAL_ATLAS_PCI_IO_SIZE                   0x03E00000  //  62 MB
#define HAL_ATLAS_CONTROLLER_BASE               0x1BE00000
#define HAL_ATLAS_CONTROLLER_BASE_ISD_CONFIG    (HAL_ATLAS_CONTROLLER_BASE >> 21)
#define HAL_ATLAS_FLASH_BASE                    0x1C000000
#define HAL_ATLAS_FLASH_SIZE                    SZ_32M
#define HAL_ATLAS_MAX_BANKSIZE                  SZ_512M

#define HAL_ATLAS_NULL_DEVNUM                   0x0
#define HAL_ATLAS_SAA9730_DEVNUM                0x9800
#define HAL_ATLAS_MEMERROR                      1

/* Atlas Registers */
#define HAL_ATLAS_REGISTER_BASE                 0xBF000000

#define HAL_ATLAS_INTBASE                       HAL_ATLAS_REGISTER_BASE

#define HAL_ATLAS_INTRAW_OFFSET                 0x00000000
#define HAL_ATLAS_INTSETEN_OFFSET               0x00000008
#define HAL_ATLAS_INTRSTEN_OFFSET               0x00000010
#define HAL_ATLAS_INTENABLE_OFFSET              0x00000018
#define HAL_ATLAS_INTSTATUS_OFFSET              0x00000020

#define HAL_ATLAS_INTRAW                        HAL_REG(HAL_ATLAS_INTBASE + HAL_ATLAS_INTRAW_OFFSET)
#define HAL_ATLAS_INTSETEN                      HAL_REG(HAL_ATLAS_INTBASE + HAL_ATLAS_INTSETEN_OFFSET)
#define HAL_ATLAS_INTRSTEN                      HAL_REG(HAL_ATLAS_INTBASE + HAL_ATLAS_INTRSTEN_OFFSET)
#define HAL_ATLAS_INTENABLE                     HAL_REG(HAL_ATLAS_INTBASE + HAL_ATLAS_INTENABLE_OFFSET)
#define HAL_ATLAS_INTSTATUS                     HAL_REG(HAL_ATLAS_INTBASE + HAL_ATLAS_INTSTATUS_OFFSET)


#define HAL_ATLAS_NMISTATUS_OFFSET              0x00000024
#define HAL_ATLAS_NMIACK_OFFSET                 0x00000104
#define HAL_ATLAS_SOFTRES_OFFSET                0x00000500
#define HAL_ATLAS_BRKRES_OFFSET                 0x00000508
#define HAL_ATLAS_REVISION_OFFSET               0x00C00010

#define HAL_ATLAS_NMISTATUS                     HAL_REG(HAL_ATLAS_REGISTER_BASE + HAL_ATLAS_NMISTATUS_OFFSET)
#define HAL_ATLAS_NMIACK                        HAL_REG(HAL_ATLAS_REGISTER_BASE + HAL_ATLAS_NMIACK_OFFSET)
#define HAL_ATLAS_SOFTRES                       HAL_REG(HAL_ATLAS_REGISTER_BASE + HAL_ATLAS_SOFTRES_OFFSET)
#define HAL_ATLAS_BRKRES                        HAL_REG(HAL_ATLAS_REGISTER_BASE + HAL_ATLAS_BRKRES_OFFSET)
#define HAL_ATLAS_REVISION                      HAL_REG(HAL_ATLAS_REGISTER_BASE + HAL_ATLAS_REVISION_OFFSET)

/* Atlas NMI controller fields */
#define HAL_ATLAS_NMISTATUS_FLAG                0x00000001
#define HAL_ATLAS_NMIACK_FLAG                   0x00000001

/* Atlas softreset fields */
#define HAL_ATLAS_GORESET                       0x42

/* Atlas brkreset fields */
#define HAL_ATLAS_BRKRES_DEFAULT_VALUE          0xA

/* Galileo Registers */
#define HAL_GALILEO_REGISTER_BASE               0xB4000000
#define HAL_GALILEO_PCI0_MEM0_BASE              0xB2000000

#define HAL_GALILEO_CPU_INTERFACE_CONFIG_OFFSET 0x0
#define HAL_GALILEO_INT_SPACE_DECODE_OFFSET     0x68
#define HAL_GALILEO_CS3_HIGH_DECODE_OFFSET      0x43c
#define HAL_GALILEO_CSBOOT_LOW_DECODE_OFFSET    0x440
#define HAL_GALILEO_CSBOOT_HIGH_DECODE_OFFSET   0x444

/* Galileo CPU Interface config fields */
#define HAL_GALILEO_BYTE_SWAP                   (BIT16 | BIT0)

#define HAL_GALILEO_CACHEOPMAP_MASK             0x000001FF
#define HAL_GALILEO_CACHEPRES_MASK              0x00000200
#define HAL_GALILEO_WRITEMODE_MASK              0x00000800
#define HAL_GALILEO_ENDIAN_MASK                 0x00001000
#define HAL_GALILEO_R5KL2_MASK                  0x00004000
#define HAL_GALILEO_EXT_HIT_DELAY_MASK          0x00008000
#define HAL_GALILEO_CPU_WRITERATE_MASK          0x00010000
#define HAL_GALILEO_STOP_RETRY_MASK             0x00020000
#define HAL_GALILEO_MULTI_GT_MASK               0x00040000
#define HAL_GALILEO_SYSADCVALID_MASK            0x00080000

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

// GALILEO PCI Internal
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

#define HAL_SAA9730_I2CSC_I2CCC_6400            0x500
#define HAL_SAA9730_I2CTFR_ATTR0_START          0xC
#define HAL_SAA9730_I2CTFR_ATTR2_START          0xC0
#define HAL_SAA9730_I2CTFR_ATTR2_STOP           0x40
#define HAL_SAA9730_I2CTFR_ATTR1_CONT           0x20
#define HAL_SAA9730_I2CTFR_OFFSET               0x2400
#define HAL_SAA9730_I2CSC_OFFSET                0x2404
#define HAL_SAA9730_SYSRESET_OFFSET             0x4020
#define   HAL_SAA9730_SYSRESET_ALL 0xdeae

#define HAL_SPD_GET_NUM_ROW_BITS                3
#define HAL_SPD_GET_NUM_COL_BITS                4
#define HAL_SPD_GET_NUM_MODULE_BANKS            5
#define HAL_SPD_GET_CONFIG_TYPE                 11
#define HAL_SPD_GET_REFRESH_RATE                12
#define HAL_SPD_GET_SDRAM_WIDTH                 13
#define HAL_SPD_GET_ERROR_CHECK_WIDTH           14
#define HAL_SPD_GET_BURST_LENGTH                16
#define HAL_SPD_GET_NUM_DEVICE_BANKS            17
#define HAL_SPD_GET_CAS_LAT                     18
#define HAL_SPD_GET_ROW_DENSITY                 31
#define HAL_SPD_CONFIG_TYPE_PARITY              BIT0
#define HAL_SPD_CONFIG_TYPE_ECC                 BIT1
#define HAL_SPD_REFRESH_RATE_125                5
#define HAL_SPD_REFRESH_RATE_62_5               4
#define HAL_SPD_REFRESH_RATE_31_3               3
#define HAL_SPD_REFRESH_RATE_15_625             0
#define HAL_SPD_REFRESH_RATE_7_8                2
#define HAL_SPD_REFRESH_RATE_3_9                1

#define HAL_SPD_REFRESH_COUNTER_125             (125*2)
#define HAL_SPD_REFRESH_COUNTER_62_5            (62*2)
#define HAL_SPD_REFRESH_COUNTER_31_3            (31*2)
#define HAL_SPD_REFRESH_COUNTER_15_625          (15*2)
#define HAL_SPD_REFRESH_COUNTER_7_8             (7*2)
#define HAL_SPD_REFRESH_COUNTER_3_9             (3*2)

/* Atlas Display Registers */
#define HAL_DISPLAY_BASE                        (HAL_ATLAS_REGISTER_BASE + 0x400)

#define HAL_DISPLAY_LEDGREEN_OFFSET             0x00
#define HAL_DISPLAY_LEDBAR_OFFSET               0x08
#define HAL_DISPLAY_ASCIIWORD_OFFSET            0x10
#define HAL_DISPLAY_ASCIIPOS0_OFFSET            0x18
#define HAL_DISPLAY_ASCIIPOS1_OFFSET            0x20
#define HAL_DISPLAY_ASCIIPOS2_OFFSET            0x28
#define HAL_DISPLAY_ASCIIPOS3_OFFSET            0x30
#define HAL_DISPLAY_ASCIIPOS4_OFFSET            0x38
#define HAL_DISPLAY_ASCIIPOS5_OFFSET            0x40
#define HAL_DISPLAY_ASCIIPOS6_OFFSET            0x48
#define HAL_DISPLAY_ASCIIPOS7_OFFSET            0x50

#define HAL_DISPLAY_LEDGREEN                    HAL_REG(HAL_DISPLAY_BASE + HAL_DISPLAY_LEDGREEN_OFFSET)
#define HAL_DISPLAY_LEDBAR                      HAL_REG(HAL_DISPLAY_BASE + HAL_DISPLAY_LEDBAR_OFFSET)
#define HAL_DISPLAY_ASCIIWORD                   HAL_REG(HAL_DISPLAY_BASE + HAL_DISPLAY_ASCIIWORD_OFFSET)
#define HAL_DISPLAY_ASCIIPOS0                   HAL_REG(HAL_DISPLAY_BASE + HAL_DISPLAY_ASCIIPOS0_OFFSET)
#define HAL_DISPLAY_ASCIIPOS1                   HAL_REG(HAL_DISPLAY_BASE + HAL_DISPLAY_ASCIIPOS1_OFFSET)
#define HAL_DISPLAY_ASCIIPOS2                   HAL_REG(HAL_DISPLAY_BASE + HAL_DISPLAY_ASCIIPOS2_OFFSET)
#define HAL_DISPLAY_ASCIIPOS3                   HAL_REG(HAL_DISPLAY_BASE + HAL_DISPLAY_ASCIIPOS3_OFFSET)
#define HAL_DISPLAY_ASCIIPOS4                   HAL_REG(HAL_DISPLAY_BASE + HAL_DISPLAY_ASCIIPOS4_OFFSET)
#define HAL_DISPLAY_ASCIIPOS5                   HAL_REG(HAL_DISPLAY_BASE + HAL_DISPLAY_ASCIIPOS5_OFFSET)
#define HAL_DISPLAY_ASCIIPOS6                   HAL_REG(HAL_DISPLAY_BASE + HAL_DISPLAY_ASCIIPOS6_OFFSET)
#define HAL_DISPLAY_ASCIIPOS7                   HAL_REG(HAL_DISPLAY_BASE + HAL_DISPLAY_ASCIIPOS7_OFFSET)

#ifdef __ASSEMBLER__

#  define DEBUG_ASCII_DISPLAY(register, character)             \
	li	k0, CYGARC_UNCACHED_ADDRESS(register);                 \
	li	k1, character;                                         \
	sw	k1, 0(k0);                                             \
    nop;                                                       \
    nop;                                                       \
    nop

#  define DEBUG_LED_IMM(val)                                   \
    li k0, HAL_DISPLAY_LEDBAR;                                 \
    li k1, val;                                                \
    sw k1, 0(k0)

#  define DEBUG_LED_REG(reg)                                   \
    li k0, HAL_DISPLAY_LEDBAR;                                 \
    sw reg, 0(k0)

#  define DEBUG_HEX_DISPLAY_IMM(val)                           \
    li k0, HAL_DISPLAY_ASCIIWORD;                              \
    li k1, val;                                                \
    sw k1, 0(k0)

#  define DEBUG_HEX_DISPLAY_REG(reg)                           \
    li k0, HAL_DISPLAY_ASCIIWORD;                              \
    sw reg, 0(k0)

#  define DEBUG_DELAY()                                        \
     li	k0, 0x20000;                                           \
0:	 sub k0, k0, 1;                                            \
     bnez k0, 0b;                                              \
     nop

#else

#  define DEBUG_ASCII_DISPLAY(register, character)             \
    *(register) = character

#  define DEBUG_LED_IMM(val)                                   \
    *HAL_DISPLAY_LEDBAR = val

#  define DEBUG_HEX_DISPLAY_IMM(val)                           \
    *HAL_DISPLAY_ASCIIWORD = val

#  define DEBUG_DELAY()                                        \
   {                                                           \
     volatile int i = 0x20000;                                 \
     while (--i) ;                                             \
   }

#  define DEBUG_DISPLAY(str)                                   \
   {                                                           \
     DEBUG_ASCII_DISPLAY(HAL_DISPLAY_ASCIIPOS0, str[0]);       \
     DEBUG_ASCII_DISPLAY(HAL_DISPLAY_ASCIIPOS1, str[1]);       \
     DEBUG_ASCII_DISPLAY(HAL_DISPLAY_ASCIIPOS2, str[2]);       \
     DEBUG_ASCII_DISPLAY(HAL_DISPLAY_ASCIIPOS3, str[3]);       \
     DEBUG_ASCII_DISPLAY(HAL_DISPLAY_ASCIIPOS4, str[4]);       \
     DEBUG_ASCII_DISPLAY(HAL_DISPLAY_ASCIIPOS5, str[5]);       \
     DEBUG_ASCII_DISPLAY(HAL_DISPLAY_ASCIIPOS6, str[6]);       \
     DEBUG_ASCII_DISPLAY(HAL_DISPLAY_ASCIIPOS7, str[7]);       \
   }


#define HAL_GALILEO_PUTREG(x,y) \
    (*((volatile unsigned *)(CYGARC_UNCACHED_ADDRESS(HAL_ATLAS_CONTROLLER_BASE) + (x))) = (y))
#define HAL_GALILEO_GETREG(x)   \
    (*((volatile unsigned *)(CYGARC_UNCACHED_ADDRESS(HAL_ATLAS_CONTROLLER_BASE) + (x))))


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

// Initialize the PCI bus.
externC void cyg_hal_plf_pci_init(void);
#define HAL_PCI_INIT() cyg_hal_plf_pci_init()

// Map PCI device resources starting from these addresses in PCI space.
#define HAL_PCI_ALLOC_BASE_MEMORY 0x08000000
#define HAL_PCI_ALLOC_BASE_IO     0x18000000

// This is where the PCI spaces are mapped in the CPU's address space.
// 
#define HAL_PCI_PHYSICAL_MEMORY_BASE 0xA0000000
#define HAL_PCI_PHYSICAL_IO_BASE     0xA0000000

// Read a value from the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
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


// Translate the PCI interrupt requested by the device (INTA#, INTB#,
// INTC# or INTD#) to the associated CPU interrupt (i.e., HAL vector).
#define HAL_PCI_TRANSLATE_INTERRUPT( __bus, __devfn, __vec, __valid)          \
    CYG_MACRO_START                                                           \
    cyg_uint8 __req;                                                          \
    HAL_PCI_CFG_READ_UINT8(__bus, __devfn, CYG_PCI_CFG_INT_PIN, __req);       \
    if (0 != __req) {                                                         \
        CYG_ADDRWORD __translation[4] = {                                     \
            CYGNUM_HAL_INTERRUPT_INTA,                                        \
            CYGNUM_HAL_INTERRUPT_INTB,                                        \
            CYGNUM_HAL_INTERRUPT_INTC,                                        \
            CYGNUM_HAL_INTERRUPT_INTD};                                       \
                                                                              \
        /* The PCI #INT lines for different device numbers are wired like this: */     \
        /*                                                             */     \
        /* Int. Ctrl.     PCI_A     PCI_B    PCI_C   PCI_D             */     \
        /* Device no.                                                  */     \
        /* 0, 4, ..28     INTC#     INTD#    INTA#   INTB#             */     \
        /* 1, 5, ..29     INTD#     INTA#    INTB#   INTC#             */     \
        /* 2, 6, ..30     INTA#     INTB#    INTC#   INTD#             */     \
        /* 3, 7, ..31     INTB#     INTC#    INTD#   INTA#             */     \
                                                                              \
        __vec = __translation[(((__req+1)+CYG_PCI_DEV_GET_DEV(__devfn))&3)];  \
        __valid = true;                                                       \
    } else {                                                                  \
        /* Device will not generate interrupt requests. */                    \
        __valid = false;                                                      \
    }                                                                         \
    CYG_MACRO_END


// Galileo GT64120 on MIPS ATLAS requires special processing.
// First, it will hang when accessing device 31 on the local bus.
// Second, we need to ignore the GT64120 so we can set it up
// outside the generic PCI library.
#define HAL_PCI_IGNORE_DEVICE(__bus, __dev, __fn) \
    ((__bus) == 0 && ((__dev) == 0 || (__dev) == 31))

#endif


//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
