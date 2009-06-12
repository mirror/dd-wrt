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
// Contributors: dmoseley, jskov
// Date:         2001-03-20
// Purpose:      Malta platform IO support
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
#define HAL_REG8(x)             x
#define HAL_REG16(x)            x
#else
#define HAL_REG(x)              (volatile CYG_WORD *)(x)
#define HAL_REG8(x)             (volatile CYG_BYTE *)(x)
#define HAL_REG16(x)            (volatile CYG_WORD16 *)(x)
#endif

//-----------------------------------------------------------------------------

/* Malta Memory Definitions */
#define HAL_MALTA_RAM_BASE                      0x00000000
#define HAL_MALTA_PCI_MEM0_BASE                 0x08000000
#define HAL_MALTA_PCI_MEM0_SIZE                 0x08000000  // 128 MB
#define HAL_MALTA_PCI_MEM1_BASE                 0x10000000
#define HAL_MALTA_PCI_MEM1_SIZE                 0x08000000  // 128 MB

#define HAL_MALTA_PCI_IO_BASE                   0x18000000
#define HAL_MALTA_PCI_IO_SIZE                   0x03d00000  //  62 MB
#define HAL_MALTA_CONTROLLER_BASE               0x1BE00000
#define HAL_MALTA_CONTROLLER_BASE_ISD_CONFIG    (HAL_MALTA_CONTROLLER_BASE >> 21)
#define HAL_MALTA_FLASH_BASE                    0x1E000000
#define HAL_MALTA_FLASH_SIZE                    SZ_4M
#define HAL_MALTA_MAX_BANKSIZE                  SZ_128M

#define HAL_MALTA_NULL_DEVNUM                   0x0
#define HAL_MALTA_MEMERROR                      1

// PCI registers
#define _PIIX4_PCI_ID   10
#define _PIIX4_BRIDGE    0
#define _PIIX4_IDE       1
#define _PIIX4_USB       2
#define _PIIX4_POWER     3

#define CYG_PCI_CFG_PIIX4_PIRQR          0x60
#define CYG_PCI_CFG_PIIX4_SERIRQC        0x64
#define CYG_PCI_CFG_PIIX4_TOM            0x69
#define CYG_PCI_CFG_PIIX4_GENCFG         0xb0

#define CYG_PCI_CFG_PIIX4_IDETIM         0x40
#define CYG_PCI_CFG_PIIX4_IDETIM_IDE     0x8000
#define CYG_PCI_CFG_PIIX4_IDETIM_SITRE   0x4000
#define CYG_PCI_CFG_PIIX4_IDETIM_DTE1    0x0080
#define CYG_PCI_CFG_PIIX4_IDETIM_PPE1    0x0040
#define CYG_PCI_CFG_PIIX4_IDETIM_IE1     0x0020
#define CYG_PCI_CFG_PIIX4_IDETIM_TIME1   0x0010
#define CYG_PCI_CFG_PIIX4_IDETIM_DTE0    0x0008
#define CYG_PCI_CFG_PIIX4_IDETIM_PPE0    0x0004
#define CYG_PCI_CFG_PIIX4_IDETIM_IE0     0x0002
#define CYG_PCI_CFG_PIIX4_IDETIM_TIME0   0x0001

#define CYG_PCI_CFG_PIIX4_SERIRQC_ENABLE 0x80
#define CYG_PCI_CFG_PIIX4_SERIRQC_CONT   0x40

#define CYG_PCI_CFG_PIIX4_TOM_TOM_MASK 0xf0
#define CYG_PCI_CFG_PIIX4_TOM_TOM_16M  0xf0

#define CYG_PCI_CFG_PIIX4_GENCFG_ISA    0x00000001
#define CYG_PCI_CFG_PIIX4_GENCFG_SERIRQ 0x00010000



/* Malta Registers */
#define HAL_MALTA_REGISTER_BASE                 0xBF000000

#define HAL_MALTA_NMISTATUS_OFFSET              0x00000024
#define HAL_MALTA_NMIACK_OFFSET                 0x00000104
#define HAL_MALTA_SOFTRES_OFFSET                0x00000500
#define HAL_MALTA_BRKRES_OFFSET                 0x00000508
#define HAL_MALTA_REVISION_OFFSET               0x00C00010

#define HAL_MALTA_NMISTATUS                     HAL_REG(HAL_MALTA_REGISTER_BASE + HAL_MALTA_NMISTATUS_OFFSET)
#define HAL_MALTA_NMIACK                        HAL_REG(HAL_MALTA_REGISTER_BASE + HAL_MALTA_NMIACK_OFFSET)
#define HAL_MALTA_SOFTRES                       HAL_REG(HAL_MALTA_REGISTER_BASE + HAL_MALTA_SOFTRES_OFFSET)
#define HAL_MALTA_BRKRES                        HAL_REG(HAL_MALTA_REGISTER_BASE + HAL_MALTA_BRKRES_OFFSET)
#define HAL_MALTA_REVISION                      HAL_REG(HAL_MALTA_REGISTER_BASE + HAL_MALTA_REVISION_OFFSET)

/* Malta NMI controller fields */
#define HAL_MALTA_NMISTATUS_FLAG                0x00000001
#define HAL_MALTA_NMIACK_FLAG                   0x00000001

/* Malta softreset fields */
#define HAL_MALTA_GORESET                       0x42

/* Malta brkreset fields */
#define HAL_MALTA_BRKRES_DEFAULT_VALUE          0xA

// PIIX4 registers
#define HAL_PIIX4_REGISTER_BASE        0xb8000000

// PIIX4 interrupt controller stuff
#define HAL_PIIX4_MASTER_ICW1          HAL_REG(HAL_PIIX4_REGISTER_BASE + 0x0020)
#define HAL_PIIX4_MASTER_ICW2          HAL_REG(HAL_PIIX4_REGISTER_BASE + 0x0021)
#define HAL_PIIX4_MASTER_ICW3          HAL_REG(HAL_PIIX4_REGISTER_BASE + 0x0021)
#define HAL_PIIX4_MASTER_ICW4          HAL_REG(HAL_PIIX4_REGISTER_BASE + 0x0021)
#define HAL_PIIX4_MASTER_OCW3          HAL_REG(HAL_PIIX4_REGISTER_BASE + 0x0020)
#define HAL_PIIX4_MASTER_OCW1          HAL_REG(HAL_PIIX4_REGISTER_BASE + 0x0021)

#define HAL_PIIX4_SLAVE_ICW1           HAL_REG(HAL_PIIX4_REGISTER_BASE + 0x00a0)
#define HAL_PIIX4_SLAVE_OCW3           HAL_REG(HAL_PIIX4_REGISTER_BASE + 0x00a0)
#define HAL_PIIX4_SLAVE_ICW2           HAL_REG(HAL_PIIX4_REGISTER_BASE + 0x00a1)
#define HAL_PIIX4_SLAVE_ICW3           HAL_REG(HAL_PIIX4_REGISTER_BASE + 0x00a1)
#define HAL_PIIX4_SLAVE_ICW4           HAL_REG(HAL_PIIX4_REGISTER_BASE + 0x00a1)
#define HAL_PIIX4_SLAVE_OCW1           HAL_REG(HAL_PIIX4_REGISTER_BASE + 0x00a1)

#define HAL_PIIX4_MASTER_SLAVE_OFFSET  0x80

#define HAL_PIIX4_ELCR1                HAL_REG(HAL_PIIX4_REGISTER_BASE + 0x04d0)
#define HAL_PIIX4_ELCR2                HAL_REG(HAL_PIIX4_REGISTER_BASE + 0x04d1)

#define HAL_PIIX4_ICW1_SEL             0x10
#define HAL_PIIX4_ICW1_WR              0x01
#define HAL_PIIX4_ICW3_CASCADE         0x04
#define HAL_PIIX4_ICW3_SLAVE           0x02
#define HAL_PIIX4_ICW4_UPMODE          0x01

#define HAL_PIIX4_OCW3_ESSM            0x40
#define HAL_PIIX4_OCW3_SEL             0x08
#define HAL_PIIX4_OCW3_REQ             0x02
#define HAL_PIIX4_OCW3_IS              0x03

#define HAL_PIIX4_ELCR1_MASK           0xf8
#define HAL_PIIX4_ELCR2_MASK           0xde

// PIIX4 IDE interface
#define HAL_PIIX4_IDE_PRI_CMD          (HAL_PIIX4_REGISTER_BASE + 0x01f0)
#define HAL_PIIX4_IDE_PRI_CTL          (HAL_PIIX4_REGISTER_BASE + 0x03f4)
#define HAL_PIIX4_IDE_SEC_CMD          (HAL_PIIX4_REGISTER_BASE + 0x0170)
#define HAL_PIIX4_IDE_SEC_CTL          (HAL_PIIX4_REGISTER_BASE + 0x0374)

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
#define HAL_GALILEO_PCI0_CONFIG_IOEn            0x1
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
#define HAL_GALILEO_PCI_IO_REMAP                0x0f0

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


#define HAL_I2CFPGA_BASE                        0x1f000b00
#define HAL_I2CFPGA_INP                         0x00
#define HAL_I2CFPGA_OE                          0x08
#define HAL_I2CFPGA_OUT                         0x10
#define HAL_I2CFPGA_SEL                         0x18

#define HAL_I2CFPGA_SEL_FPGA                    0x00000001
#define HAL_I2CFPGA_SEL_SB                      0x00000000

#define HAL_I2CFPGA_OE_SCL_OUT                  0x00000002
#define HAL_I2CFPGA_OE_SCL_TRI                  0x00000000
#define HAL_I2CFPGA_OE_SDA_OUT                  0x00000001
#define HAL_I2CFPGA_OE_SDA_TRI                  0x00000000

#define HAL_I2CFPGA_OUT_SCL_HIGH                0x00000002
#define HAL_I2CFPGA_OUT_SCL_LOW                 0x00000000
#define HAL_I2CFPGA_OUT_SDA_HIGH                0x00000001
#define HAL_I2CFPGA_OUT_SDA_LOW                 0x00000000


#define HAL_I2CFPGA_IN_SDA_MASK                 0x00000001

#define HAL_I2CFPGA_OUT_SDA_ACK                 0x00000000
#define HAL_I2CFPGA_OUT_SDA_NACK                0x00000001
#define HAL_I2CFPGA_OUT_SDA_WAIT_ACK            0x00000001


#define HAL_I2C_WRITE                           0x00
#define HAL_I2C_READ                            0x01

#define HAL_I2C_SPD_ADDRESS                     0xa0

#define HAL_I2C_COUT_DOUT                       (HAL_I2CFPGA_OE_SCL_OUT|HAL_I2CFPGA_OE_SDA_OUT)
#define HAL_I2C_COUT_DIN                        (HAL_I2CFPGA_OE_SCL_OUT|HAL_I2CFPGA_OE_SDA_TRI)
#define HAL_I2C_CIN_DIN                         (HAL_I2CFPGA_OE_SCL_TRI|HAL_I2CFPGA_OE_SDA_TRI)
#define HAL_I2C_CHIGH_DHIGH                     (HAL_I2CFPGA_OUT_SCL_HIGH|HAL_I2CFPGA_OUT_SDA_HIGH)
#define HAL_I2C_CHIGH_DLOW                      (HAL_I2CFPGA_OUT_SCL_HIGH|HAL_I2CFPGA_OUT_SDA_LOW)
#define HAL_I2C_CLOW_DLOW                       (HAL_I2CFPGA_OUT_SCL_LOW|HAL_I2CFPGA_OUT_SDA_LOW)
#define HAL_I2C_CLOW_DHIGH                      (HAL_I2CFPGA_OUT_SCL_LOW|HAL_I2CFPGA_OUT_SDA_HIGH)


#define HAL_SPD_GET_NUM_ROW_BITS                3
#define HAL_SPD_GET_NUM_COL_BITS                4
#define HAL_SPD_GET_NUM_MODULE_BANKS            5
#define HAL_SPD_GET_SDRAM_WIDTH                 6
#define HAL_SPD_GET_CONFIG_TYPE                 11
#define HAL_SPD_GET_REFRESH_RATE                12
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

/* Malta Display Registers */
#define HAL_DISPLAY_BASE                        (HAL_MALTA_REGISTER_BASE + 0x400)

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
    (*((volatile unsigned *)(CYGARC_UNCACHED_ADDRESS(HAL_MALTA_CONTROLLER_BASE) + (x))) = (y))
#define HAL_GALILEO_GETREG(x)   \
    (*((volatile unsigned *)(CYGARC_UNCACHED_ADDRESS(HAL_MALTA_CONTROLLER_BASE) + (x))))


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

// leave gap at start of IO and mem for southbridge which is beyond standards
// and not only ignores writes to the BAR, but also does not advertise use of
// any IO/memory space. That is, southbridge is hardwired at 0x18000000

// Map PCI device resources starting from these addresses in PCI space.
#define HAL_PCI_ALLOC_BASE_MEMORY    (HAL_MALTA_PCI_MEM0_BASE + 0x20000)
#define HAL_PCI_ALLOC_BASE_IO        0x10000

// This is where the PCI spaces are mapped in the CPU's address space.
// 
#define HAL_PCI_PHYSICAL_MEMORY_BASE CYGARC_UNCACHED_ADDRESS(0)
#define HAL_PCI_PHYSICAL_IO_BASE     CYGARC_UNCACHED_ADDRESS(HAL_MALTA_PCI_IO_BASE)

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
#define HAL_PCI_TRANSLATE_INTERRUPT( __bus, __devfn, __vec, __valid)            \
    CYG_MACRO_START                                                             \
    cyg_uint8 __req;                                                            \
    HAL_PCI_CFG_READ_UINT8(__bus, __devfn, CYG_PCI_CFG_INT_PIN, __req);         \
    if (0 != __req) {                                                           \
        /* Interrupt assignment as Galileo sees them.                     */    \
        /* (From Malta User's Manual, 6.1 PCI Bus)                        */    \
        CYG_ADDRWORD __translation[4] = {                                       \
            CYGNUM_HAL_INTERRUPT_PCI_AB,  /* INTB# */                           \
            CYGNUM_HAL_INTERRUPT_PCI_CD,  /* INTC# */                           \
            CYGNUM_HAL_INTERRUPT_PCI_CD,  /* INTD# */                           \
            CYGNUM_HAL_INTERRUPT_PCI_AB}; /* INTA# */                           \
                                                                                \
        /* The PCI lines from the different slots are wired like this  */       \
        /* on the PCI backplane:                                       */       \
        /*                PCI_AB    PCI_AB   PCI_CD  PCI_CD            */       \
        /* AMD PCnet                INTA#                              */       \
        /* I/O Slot 1     INTA#     INTB#    INTC#   INTD#             */       \
        /* I/O Slot 2     INTD#     INTA#    INTB#   INTC#             */       \
        /* I/O Slot 3     INTC#     INTD#    INTA#   INTB#             */       \
        /* I/O Slot 4     INTB#     INTC#    INTD#   INTA#             */       \
        /*                                                             */       \
        /* Devsel signals are wired to, resulting in device IDs:       */       \
        /* AMD PCnet      AD21 / dev 11      [(11+1)&3 = 0]            */       \
        /* I/O Slot 1     AD28 / dev 18      [(18+1)&3 = 3]            */       \
        /* I/O Slot 2     AD29 / dev 19      [(19+1)&3 = 0]            */       \
        /* I/O Slot 3     AD30 / dev 20      [(20+1)&3 = 1]            */       \
        /* I/O Slot 4     AD31 / dev 21      [(21+1)&3 = 2]            */       \
                                                                                \
        /* For some reason the Ethernet device comes in on interrupt   */       \
        /* 11 rather than interrupt 16.                                */       \
        if( (__bus)==0 && (__devfn)==0x58 )                                     \
            __vec = CYGNUM_HAL_INTERRUPT_11;                                    \
        else __vec = __translation[((__req+CYG_PCI_DEV_GET_DEV(__devfn))&3)];   \
        __valid = true;                                                         \
    } else {                                                                    \
        /* Device will not generate interrupt requests. */                      \
        __valid = false;                                                        \
    }                                                                           \
    CYG_MACRO_END

// Galileo GT64120 on MIPS MALTA requires special processing.
// First, it will hang when accessing device 31 on the local bus.
// Second, we need to ignore the GT64120 so we can set it up
// outside the generic PCI library.
#define HAL_PCI_IGNORE_DEVICE(__bus, __dev, __fn) \
    ((__bus) == 0 && ((__dev) == 0 || (__dev) == 31))

// Bus address translation macros
#define HAL_PCI_CPU_TO_BUS(__cpu_addr, __bus_addr)        \
    CYG_MACRO_START                                       \
    (__bus_addr) = CYGARC_PHYSICAL_ADDRESS((cyg_uint32)__cpu_addr);   \
    CYG_MACRO_END

#define HAL_PCI_BUS_TO_CPU(__bus_addr, __cpu_addr)        \
    CYG_MACRO_START                                       \
    (__cpu_addr) = CYGARC_UNCACHED_ADDRESS(__bus_addr);   \
    CYG_MACRO_END


// IDE interface macros
//
#define HAL_IDE_NUM_CONTROLLERS 2

// Initialize the IDE controller(s).
externC int cyg_hal_plf_ide_init(void);
#define HAL_IDE_INIT() cyg_hal_plf_ide_init()

#define HAL_IDE_READ_UINT8( __ctlr, __regno, __val )  \
    __val = *HAL_REG8(((__ctlr) ? HAL_PIIX4_IDE_SEC_CMD : HAL_PIIX4_IDE_PRI_CMD) + (__regno))
#define HAL_IDE_READ_UINT16( __ctlr, __regno, __val )  \
    __val = *HAL_REG16(((__ctlr) ? HAL_PIIX4_IDE_SEC_CMD : HAL_PIIX4_IDE_PRI_CMD) + (__regno))
#define HAL_IDE_READ_ALTSTATUS( __ctlr, __val )  \
    __val = *HAL_REG16(((__ctlr) ? HAL_PIIX4_IDE_SEC_CTL : HAL_PIIX4_IDE_PRI_CTL) + 2)

#define HAL_IDE_WRITE_UINT8( __ctlr, __regno, __val )  \
    *HAL_REG8( ((__ctlr) ? HAL_PIIX4_IDE_SEC_CMD : HAL_PIIX4_IDE_PRI_CMD) + (__regno)) = (__val)
#define HAL_IDE_WRITE_UINT16( __ctlr, __regno, __val )  \
    *HAL_REG16( ((__ctlr) ? HAL_PIIX4_IDE_SEC_CMD : HAL_PIIX4_IDE_PRI_CMD) + (__regno)) = (__val)
#define HAL_IDE_WRITE_CONTROL( __ctlr, __val )  \
    *HAL_REG8( ((__ctlr) ? HAL_PIIX4_IDE_SEC_CTL : HAL_PIIX4_IDE_PRI_CTL) + 2) = (__val)

#endif

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
