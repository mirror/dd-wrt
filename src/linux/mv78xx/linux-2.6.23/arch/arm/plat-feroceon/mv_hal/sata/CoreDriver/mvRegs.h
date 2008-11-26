/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/
/*******************************************************************************
* mvRegs.h - Header file that includes the regs addresses
*
* DESCRIPTION:
*       None.
*
* DEPENDENCIES:
*       None.
*
*******************************************************************************/

#ifndef __INCmvRegs
#define __INCmvRegs
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* General Definitions */
#define MV_BIT0                                 0x00000001
#define MV_BIT1                                 0x00000002
#define MV_BIT2                                 0x00000004
#define MV_BIT3                                 0x00000008
#define MV_BIT4                                 0x00000010
#define MV_BIT5                                 0x00000020
#define MV_BIT6                                 0x00000040
#define MV_BIT7                                 0x00000080
#define MV_BIT8                                 0x00000100
#define MV_BIT9                                 0x00000200
#define MV_BIT10                                0x00000400
#define MV_BIT11                                0x00000800
#define MV_BIT12                                0x00001000
#define MV_BIT13                                0x00002000
#define MV_BIT14                                0x00004000
#define MV_BIT15                                0x00008000
#define MV_BIT16                                0x00010000
#define MV_BIT17                                0x00020000
#define MV_BIT18                                0x00040000
#define MV_BIT19                                0x00080000
#define MV_BIT20                                0x00100000
#define MV_BIT21                                0x00200000
#define MV_BIT22                                0x00400000
#define MV_BIT23                                0x00800000
#define MV_BIT24                                0x01000000
#define MV_BIT25                                0x02000000
#define MV_BIT26                                0x04000000
#define MV_BIT27                                0x08000000
#define MV_BIT28                                0x10000000
#define MV_BIT29                                0x20000000
#define MV_BIT30                                0x40000000
#define MV_BIT31                                0x80000000


#define MV_PCI_REGS_OFFSET                      0x0

/* PCI registers */

#define MV_MAIN_INTERRUPT_CAUSE_REG_OFFSET      0x1d60

#define MV_MAIN_INTERRUPT_MASK_REG_OFFSET       0x1d64
#define MV_MAIN_INTERRUPT_MASK_REG_ALL_BITS     0x7ffff
#define MV_MAIN_INTERRUPT_MASK_REG_PCIERR_BIT   MV_BIT18
#define MV_MAIN_INTERRUPT_HOST_SELF_INT_BIT     MV_BIT23

#define MV_PCI_CONTROL_REG_OFFSET               0x1d68
#define MV_PCI_CONTROL_REG_H2H_INT_BIT          MV_BIT1

/* enable ports error, coalescing done and pci error*/
#define MV_MAIN_INTERRUPT_MASK_ENABLE_ALL       0x6ab55

#define MV_PCI_DLL_STATUS_CONTROL_REG_OFFSET    0x1d20
#define MV_PCI_COMMAND_REG_OFFSET               0xc00
#define MV_PCI_COMMAND_REG_DEFAULT              0x0107E371
#define MV_PCI_COMMAND_PCI_CONVENTIONAL_ONLY            \
    (MV_BIT5|MV_BIT6|MV_BIT7|MV_BIT8|MV_BIT9|MV_BIT31)
#define MV_PCI_MWRITE_COMBINE_BIT               MV_BIT4
#define MV_PCI_MREAD_COMBINE_BIT                MV_BIT5

#define MV_PCI_MODE_REG_OFFSET                  0xd00
#define MV_PCI_MODE_MASK                        0x30
#define MV_PCI_MODE_OFFSET                      4

#define MV_PCI_DISCARD_TIMER_REG_OFFSET         0xd04

#define MV_PCI_MSI_TRIGGER_REG_OFFSET           0xc38

#define MV_PCI_EXPANSION_ROM_CONTROL_REG_OFFSET 0xd2c

#define MV_PCI_XBAR_IF_TIMEOUT_REG_OFFSET       0x1d04

#define MV_PCI_SERR_MASK_REG_OFFSET             0xc28
#define MV_PCI_SERR_MASK_REG_ENABLE_ALL         0xd77fe6

#define MV_PCI_INTERRUPT_CAUSE_REG_OFFSET       0x1d58

#define MV_PCI_INTERRUPT_MASK_REG_OFFSET        0x1d5c

#define MV_PCI_INTERRUPT_MASK_REG_ENABLE_ALL    MV_PCI_SERR_MASK_REG_ENABLE_ALL

#define MV_PCI_ERROR_LOW_ADDRESS_REG_OFFSET     0x1d40

#define MV_PCI_ERROR_HIGH_ADDRESS_REG_OFFSET    0x1d44

#define MV_PCI_ERROR_ATTRIBUTE_REG_OFFSET       0x1d48

#define MV_PCI_ERROR_COMMAND_REG_OFFSET         0x1d50

#define MV_PCI_MAIN_COMMAND_STATUS_REG_OFFSET   0xd30 /*Relevant for 60x1 only*/
#define MV_PCI_MAIN_COMMAND_DIS_CORE_CLK_MASK   MV_BIT0
#define MV_PCI_MAIN_COMMAND_DIS_SHC1_CLK_MASK   MV_BIT0
#define MV_PCI_MAIN_COMMAND_STOP_MASTER_MASK    MV_BIT2
#define MV_PCI_MAIN_COMMAND_MASTER_EMPTY_MASK   MV_BIT3
#define MV_PCI_MAIN_COMMAND_GLOBAL_RESET_MASK   MV_BIT4

/*PEX (PCI-Express registers*/
#define MV_PCI_E_INTERRUPT_CAUSE_REG_OFFSET     0x1900
#define MV_PCI_E_INTERRUPT_MASK_REG_OFFSET      0x1910
#define MV_PCI_E_ERROR_MASK_VALUE               (MV_BIT10|MV_BIT9|MV_BIT8|MV_BIT3|MV_BIT1)
/* Flash interface registers */
#define MV_FLASH_PARAMS_REG_OFFSET              0x1046c

#define MV_FLASH_GPIO_PORT_CONTROL_OFFSET       0x104f0

#define MV_RESET_CONFIG_REG_OFFSET              0x180d8
#define MV_RESET_CONFIG_TWSI_INIT_MASK          MV_BIT0
#define MV_SATA_II_ALL_PORTS_INT_COAL_CMND_THR_REG_OFFSET 0x180cc

#define MV_SATA_II_ALL_PORTS_INT_COAL_TIME_THR_REG_OFFSET 0x180d0

#define MV_SATA_II_ALL_PORTS_INT_CAUSE_REG_OFFSET 0x18008

/* SATAHC registers*/
#define MV_SATAHC_0_REGS_BASE_OFFSET            0x20000 /* 128KByte offset */

#define MV_SATAHC_1_REGS_BASE_OFFSET            0x30000 /* 192KByte offset */

#define MV_SATAHC_REGS_BASE_OFFSET(unit)        ((unit)?(MV_SATAHC_1_REGS_BASE_OFFSET):(MV_SATAHC_0_REGS_BASE_OFFSET))

#define MV_SATAHC_XBAR_CONF_REG_OFFSET          0x000

#define MV_SATAHC_RESPONSE_Q_IN_POINTER_OFFSET  0x008

#define MV_SATAHC_INT_COAL_THRE_REG_OFFSET      0x00c

#define MV_SATAHC_INT_TIME_THRE_REG_OFFSET      0x010

#define MV_SATAHC_INTERRUPT_CAUSE_REG_OFFSET    0x014

#define MV_SATA_I_HC_BRIDGES_TEST_CONTROL_REG_OFFSET    0x018
#define MV_SATA_I_TEST_CONTROL_PATTERN_MASK         0x7
#define MV_SATA_I_TEST_CONTROL_PATTERN_OFFSET           10
#define MV_SATA_I_TEST_CONTROL_PORT_MASK                0x3
#define MV_SATA_I_TEST_CONTROL_PORT_OFFSET              13
#define MV_SATA_I_TEST_CONTROL_PHY_SHUTDOWN_MASK(port)  ((MV_U32)1 << (24 + (port)))

#define MV_SATA_I_HC_BRIDGES_TEST_STATUS_REG_OFFSET 0x01c
#define MV_SATA_I_TEST_STATUS_LOOPBACK_PASS_BIT     MV_BIT8

#define MV_SATA_I_HC_BRIDGES_PINS_CONFIG_REG_OFFSET 0x020

#define MV_SATA_I_HC_R00_STATUS_BRIDGE_PORT_OFFSET(port)        \
    (0x100 + (port)*0x100)

#define MV_SATA_I_HC_R00_STATUS_BRIDGE_DET_OFFSET  0x0
#define MV_SATA_I_HC_R00_STATUS_BRIDGE_DET_MASK 0xf

#define MV_SATA_I_HC_R02_STATUS_BRIDGE_PORT_OFFSET(port)    \
    (0x108 + (port)*0x100)

#define MV_SATA_I_HC_PHY_CONTROL_BRIDGE_PORT_OFFSET(port)   \
    (0x10C + (port)*0x100)

#define MV_SATA_I_HC_R04_STATUS_BRIDGE_PORT_OFFSET(port)    \
    (0x110 + (port)*0x100)
#define MV_SATA_I_HC_R05_STATUS_BRIDGE_PORT_OFFSET(port)    \
    (0x114 + (port)*0x100)
#define MV_SATA_I_HC_R06_STATUS_BRIDGE_PORT_OFFSET(port)    \
    (0x118 + (port)*0x100)
#define MV_SATA_I_HC_R0F_STATUS_BRIDGE_PORT_OFFSET(port)    \
    (0x13c + (port)*0x100)
#define MV_SATA_I_HC_LT_MODES_PORT_REG_OFFSET(port)         \
    (0x130 + ((port) * 0x100))
#define MV_SATA_I_HC_PHY_MODE_BRIDGE_PORT_REG_OFFSET(port)  \
    (0x174 + ((port) * 0x100))
#define MV_SATA_I_PHY_MODE_AMP_MASK             0xe0
#define MV_SATA_I_PHY_MODE_AMP_OFFSET               5
#define MV_SATA_I_PHY_MODE_PRE_MASK             0x1800
#define MV_SATA_I_PHY_MODE_PRE_OFFSET               11

#define MV_SATA_II_PHY_MODE_1_REG_OFFSET        0x32c

#define MV_SATA_II_PHY_MODE_2_REG_OFFSET        0x330
#define MV_SATA_II_PHY_MODE_2_AMP_MASK          0x700
#define MV_SATA_II_PHY_MODE_2_AMP_OFFSET        8
#define MV_SATA_II_PHY_MODE_2_PRE_MASK          0xe0
#define MV_SATA_II_PHY_MODE_2_PRE_OFFSET        5

#define MV_SATA_II_PHY_MODE_3_REG_OFFSET        0x310

#define MV_SATA_II_PHY_MODE_4_REG_OFFSET        0x314

#define MV_SATA_II_IF_CONTROL_REG_OFFSET        0x344
#define MV_SATA_II_IF_CONTROL_PMTX_OFFSET       0
#define MV_SATA_II_IF_CONTROL_PMTX_MASK         0xf

#define MV_SATA_II_IF_TEST_CTRL_REG_OFFSET		0x348

#define MV_SATA_II_IF_STATUS_REG_OFFSET         0x34c
#define MV_SATA_II_IF_STATUS_VUQ_DONE_MASK      MV_BIT12
#define MV_SATA_II_IF_STATUS_VUQ_ERR_MASK       MV_BIT13
#define MV_SATA_II_IF_STATUS_FSM_STATUS_MASK    0x3f000000


#define MV_SATA_II_VENDOR_UQ_REG_OFFSET         0x35c

#define MV_SATA_II_S_STATUS_REG_OFFSET          0x300

#define MV_SATA_II_S_ERROR_REG_OFFSET           0x304

#define MV_SATA_II_S_CONTROL_REG_OFFSET         0x308

#define MV_SATA_II_SATA_CONFIG_REG_OFFSET       0x50

#define MV_SATA_II_PHY_MODE_9_GEN1_REG_OFFSET   0x39c
#define MV_SATA_II_PHY_MODE_9_GEN2_REG_OFFSET   0x398
/* EDMA registers */

#define MV_EDMA_PORT_BASE_OFFSET(port)          ( 0x2000UL * ((port) + 1))

#define MV_EDMA_CONFIG_REG_OFFSET               0x000
#define MV_EDMA_CONFIG_Q_DEPTH_MASK             0x1f
#define MV_EDMA_CONFIG_NATIVE_QUEUING_MASK      MV_BIT5 /* Relevant for 60x1 */
#define MV_EDMA_CONFIG_BURST_SIZE_MASK          MV_BIT8
#define MV_EDMA_CONFIG_EQUEUE_ENABLED_MASK      MV_BIT9
#define MV_EDMA_CONFIG_STOP_ON_ERROR_MASK       MV_BIT10
#define MV_EDMA_CONFIG_BURST_SIZE_EXT_MASK      MV_BIT11
#define MV_EDMA_CONFIG_CONONDEVERR_MASK			MV_BIT14

#define MV_EDMA_TIMER_REG_OFFSET                0x004

#define MV_EDMA_INTERRUPT_ERROR_CAUSE_REG_OFFSET 0x008

#define MV_EDMA_INTERRUPT_ERROR_MASK_REG_OFFSET  0x00c

/* unrecoverable EDMA errors*/
#define MV_EDMA_GEN_I_UNRECOVERABLE_EDMA_ERROR		0x00001E6B
#define MV_EDMA_GEN_I_RECOVERABLE_EDMA_ERROR		0x00000000
#define MV_EDMA_GEN_I_ERROR_MASK					0x00001118

#define MV_EDMA_GEN_II_UNRECOVERABLE_EDMA_ERROR		0xFC1E9E0B
#define MV_EDMA_GEN_II_RECOVERABLE_EDMA_ERROR		0x03E16020
#define MV_EDMA_GEN_II_NORMAL_RCVRBL_EDMA_ERROR		0x02200000

/*don't enable rcvrbl errors that occur too often*/
#define MV_EDMA_GEN_II_ERROR_MASK					0xFDDFF198


#define MV_EDMA_REQUEST_Q_BAH_REG_OFFSET        0x010

#define MV_EDMA_REQUEST_Q_INP_REG_OFFSET        0x014
#define MV_EDMA_REQUEST_Q_INP_MASK              0x000003e0
#define MV_EDMA_REQUEST_Q_INP_OFFSET            5
#define MV_EDMA_REQUEST_Q_BA_MASK               0xfffffc00

#define MV_EDMA_GEN2E_REQUEST_Q_INP_MASK        0x00000fe0
#define MV_EDMA_GEN2E_REQUEST_Q_BA_MASK         0xfffff000

#define MV_EDMA_REQUEST_Q_OUTP_REG_OFFSET       0x018
#define MV_EDMA_REQUEST_Q_OUTP_MASK             0x000003e0

#define MV_EDMA_RESPONSE_Q_BAH_REG_OFFSET       0x01c

#define MV_EDMA_RESPONSE_Q_INP_REG_OFFSET       0x020
#define MV_EDMA_RESPONSE_Q_INP_MASK             0x000000f8

#define MV_EDMA_RESPONSE_Q_OUTP_REG_OFFSET      0x024
#define MV_EDMA_RESPONSE_Q_OUTP_MASK            0x000000f8
#define MV_EDMA_RESPONSE_Q_OUTP_OFFSET          3

#define MV_EDMA_RESPONSE_Q_BA_MASK              0xffffff00


#define MV_EDMA_COMMAND_REG_OFFSET              0x028
#define MV_EDMA_COMMAND_ENABLE_MASK             MV_BIT0
#define MV_EDMA_COMMAND_DISABLE_MASK            MV_BIT1
#define MV_EDMA_COMMAND_HARD_RST_MASK           MV_BIT2

#define MV_EDMA_TEST_CONTROL_REG_OFFSET         0x02c
#define MV_EDMA_LOOPBACK_MODE_BIT               MV_BIT1

#define MV_EDMA_STATUS_REG_OFFSET               0x030
#define MV_EDMA_STATUS_TAG_MASK                 0x1f
#define MV_EDMA_STATUS_TAG_OFFSET               0
#define MV_EDMA_STATUS_ECACHE_EMPTY_BIT			MV_BIT6
#define MV_EDMA_STATUS_EDMA_IDLE_BIT			MV_BIT7


#define MV_EDMA_IORDY_TIMEOUT_REG_OFFSET        0x034

#define MV_EDMA_ARBITER_CNFG_REG_OFFSET         0x038
#define MV_EDMA_NO_SNOOP_BIT                    MV_BIT6

#define MV_EDMA_CMD_DELAY_THRE_REG_OFFSET       0x040

#define MV_EDMA_HALT_CONDITIONS_REG_OFFSET      0x060

#define MV_EDMA_TCQ_STATUS_REG_OFFSET           0x08c

#define MV_EDMA_NCQTCQ0_OUTSTANDING_REG_OFFSET	0x094
#define MV_EDMA_NCQTCQ1_OUTSTANDING_REG_OFFSET	0x098
#define MV_EDMA_NCQTCQ2_OUTSTANDING_REG_OFFSET	0x09c
#define MV_EDMA_NCQTCQ3_OUTSTANDING_REG_OFFSET	0x0a0

#define MV_EDMA_FIS_CONFIGURATION_REG_OFFSET	0x360
#define MV_EDMA_FIS_INTERRUPT_CAUSE_REG_OFFSET	0x364
#define MV_EDMA_FIS_INTERRUPT_MASK_REG_OFFSET	0x368
/* C2C */
/* BM DMA */
#define MV_BMDMA_COMMAND_OFFSET                 0x224
#define MV_BMDMA_STATUS_OFFSET                  0x228
#define MV_BMDMA_PRD_TABLE_LOW_ADDRESS_OFFSET   0x22c
#define MV_BMDMA_PRD_TABLE_HIGH_ADDRESS_OFFSET  0x230

/* storage device registers*/

#define MV_ATA_DEVICE_PIO_DATA_REG_OFFSET       0x100
#define MV_ATA_DEVICE_FEATURES_REG_OFFSET       0x104
#define MV_ATA_DEVICE_ERROR_REG_OFFSET          MV_ATA_DEVICE_FEATURES_REG_OFFSET
#define MV_ATA_DEVICE_SECTOR_COUNT_REG_OFFSET   0x108
#define MV_ATA_DEVICE_LBA_LOW_REG_OFFSET        0x10c
#define MV_ATA_DEVICE_LBA_MID_REG_OFFSET        0x110
#define MV_ATA_DEVICE_LBA_HIGH_REG_OFFSET       0x114
#define MV_ATA_DEVICE_HEAD_REG_OFFSET           0x118
#define MV_ATA_DEVICE_COMMAND_REG_OFFSET        0x11c
#define MV_ATA_DEVICE_STATUS_REG_OFFSET         MV_ATA_DEVICE_COMMAND_REG_OFFSET
#define MV_ATA_DEVICE_CONTROL_REG_OFFSET        0x120
#define MV_ATA_DEVICE_ALTERNATE_REG_OFFSET      MV_ATA_DEVICE_CONTROL_REG_OFFSET

#define MV_IOG_TRANS_LOW_BIT      (MV_BIT19)
#define MV_IOG_TRANS_HIGH_BIT     (MV_BIT20)
#define MV_IOG_TRANS_INT_MASK     (MV_IOG_TRANS_LOW_BIT | MV_IOG_TRANS_HIGH_BIT)
#define MV_IOG_TRANS_LOW_REG_OFFSET  0x18088
#define MV_IOG_TRANS_HIGH_REG_OFFSET 0x1808C
#define MV_IOG_TRANS_CTRL_REG_OFFSET 0x18048


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCmvRegs */
