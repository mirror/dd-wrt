/*
 *  linux/include/asm-arm/hardware/pci_v3.h
 *
 *  Internal header file PCI V3 chip
 *
 *  Copyright (C) ARM Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef ASM_ARM_HARDWARE_PCI_V3_H
#define ASM_ARM_HARDWARE_PCI_V3_H

/* -------------------------------------------------------------------------------
 *  V3 Local Bus to PCI Bridge definitions
 * -------------------------------------------------------------------------------
 *  Registers (these are taken from page 129 of the EPC User's Manual Rev 1.04
 *  All V3 register names are prefaced by V3_ to avoid clashing with any other
 *  PCI definitions.  Their names match the user's manual.
 * 
 *  I'm assuming that I20 is disabled.
 * 
 */
#define V3_PCI_VENDOR                   0x00000000
#define V3_PCI_DEVICE                   0x00000002
#define V3_PCI_CMD                      0x00000004
#define V3_PCI_STAT                     0x00000006
#define V3_PCI_CC_REV                   0x00000008
#define V3_PCI_HDR_CFG                  0x0000000C
#define V3_PCI_IO_BASE                  0x00000010
#define V3_PCI_BASE0                    0x00000014
#define V3_PCI_BASE1                    0x00000018
#define V3_PCI_SUB_VENDOR               0x0000002C
#define V3_PCI_SUB_ID                   0x0000002E
#define V3_PCI_ROM                      0x00000030
#define V3_PCI_BPARAM                   0x0000003C
#define V3_PCI_MAP0                     0x00000040
#define V3_PCI_MAP1                     0x00000044
#define V3_PCI_INT_STAT                 0x00000048
#define V3_PCI_INT_CFG                  0x0000004C 
#define V3_LB_BASE0                     0x00000054
#define V3_LB_BASE1                     0x00000058
#define V3_LB_MAP0                      0x0000005E
#define V3_LB_MAP1                      0x00000062
#define V3_LB_BASE2                     0x00000064
#define V3_LB_MAP2                      0x00000066
#define V3_LB_SIZE                      0x00000068
#define V3_LB_IO_BASE                   0x0000006E
#define V3_FIFO_CFG                     0x00000070
#define V3_FIFO_PRIORITY                0x00000072
#define V3_FIFO_STAT                    0x00000074
#define V3_LB_ISTAT                     0x00000076
#define V3_LB_IMASK                     0x00000077
#define V3_SYSTEM                       0x00000078
#define V3_LB_CFG                       0x0000007A
#define V3_PCI_CFG                      0x0000007C
#define V3_DMA_PCI_ADR0                 0x00000080
#define V3_DMA_PCI_ADR1                 0x00000090
#define V3_DMA_LOCAL_ADR0               0x00000084
#define V3_DMA_LOCAL_ADR1               0x00000094
#define V3_DMA_LENGTH0                  0x00000088
#define V3_DMA_LENGTH1                  0x00000098
#define V3_DMA_CSR0                     0x0000008B
#define V3_DMA_CSR1                     0x0000009B
#define V3_DMA_CTLB_ADR0                0x0000008C
#define V3_DMA_CTLB_ADR1                0x0000009C
#define V3_DMA_DELAY                    0x000000E0
#define V3_MAIL_DATA                    0x000000C0
#define V3_PCI_MAIL_IEWR                0x000000D0
#define V3_PCI_MAIL_IERD                0x000000D2
#define V3_LB_MAIL_IEWR                 0x000000D4
#define V3_LB_MAIL_IERD                 0x000000D6
#define V3_MAIL_WR_STAT                 0x000000D8
#define V3_MAIL_RD_STAT                 0x000000DA
#define V3_QBA_MAP                      0x000000DC

/*  PCI COMMAND REGISTER bits
 */
#define V3_COMMAND_M_FBB_EN             (1 << 9)
#define V3_COMMAND_M_SERR_EN            (1 << 8)
#define V3_COMMAND_M_PAR_EN             (1 << 6)
#define V3_COMMAND_M_MASTER_EN          (1 << 2)
#define V3_COMMAND_M_MEM_EN             (1 << 1)
#define V3_COMMAND_M_IO_EN              (1 << 0)

/*  SYSTEM REGISTER bits
 */
#define V3_SYSTEM_M_RST_OUT             (1 << 15)
#define V3_SYSTEM_M_LOCK                (1 << 14)

/*  PCI_CFG bits
 */
#define V3_PCI_CFG_M_RETRY_EN           (1 << 10)
#define V3_PCI_CFG_M_AD_LOW1            (1 << 9)
#define V3_PCI_CFG_M_AD_LOW0            (1 << 8)

/*  PCI_BASE register bits (PCI -> Local Bus)
 */
#define V3_PCI_BASE_M_ADR_BASE          0xFFF00000
#define V3_PCI_BASE_M_ADR_BASEL         0x000FFF00
#define V3_PCI_BASE_M_PREFETCH          (1 << 3)
#define V3_PCI_BASE_M_TYPE              (3 << 1)
#define V3_PCI_BASE_M_IO                (1 << 0)

/*  PCI MAP register bits (PCI -> Local bus)
 */
#define V3_PCI_MAP_M_MAP_ADR            0xFFF00000
#define V3_PCI_MAP_M_RD_POST_INH        (1 << 15)
#define V3_PCI_MAP_M_ROM_SIZE           (3 << 10)
#define V3_PCI_MAP_M_SWAP               (3 << 8)
#define V3_PCI_MAP_M_ADR_SIZE           0x000000F0
#define V3_PCI_MAP_M_REG_EN             (1 << 1)
#define V3_PCI_MAP_M_ENABLE             (1 << 0)

/*  9 => 512M window size
 */
#define V3_PCI_MAP_M_ADR_SIZE_512M      0x00000090
/*  A => 1024M window size
 */
#define V3_PCI_MAP_M_ADR_SIZE_1024M     0x000000A0      

/*  LB_BASE register bits (Local bus -> PCI)
 */
#define V3_LB_BASE_M_MAP_ADR            0xFFF00000
#define V3_LB_BASE_M_SWAP               (3 << 8)
#define V3_LB_BASE_M_ADR_SIZE           0x000000F0
#define V3_LB_BASE_M_PREFETCH           (1 << 3)
#define V3_LB_BASE_M_ENABLE             (1 << 0)

/*  LB_MAP register bits (Local bus -> PCI)
 */
#define V3_LB_MAP_M_MAP_ADR             0xFFF0
#define V3_LB_MAP_M_TYPE                0x000E
#define V3_LB_MAP_M_AD_LOW_EN           (1 << 0)

#endif
