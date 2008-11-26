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

#ifndef __INCmvUsbRegsh
#define __INCmvUsbRegsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************/
/* USB ARC Core Registers                  */
/*******************************************/
#define MV_USB_CORE_ID_REG(dev)                 (USB_REG_BASE(dev) + 0x00)
#define MV_USB_CORE_GENERAL_REG(dev)            (USB_REG_BASE(dev) + 0x04)
#define MV_USB_CORE_HOST_REG(dev)               (USB_REG_BASE(dev) + 0x08)
#define MV_USB_CORE_DEVICE_REG(dev)             (USB_REG_BASE(dev) + 0x0C)
#define MV_USB_CORE_TX_BUF_REG(dev)             (USB_REG_BASE(dev) + 0x10)
#define MV_USB_CORE_RX_BUF_REG(dev)             (USB_REG_BASE(dev) + 0x14)
#define MV_USB_CORE_TTTX_BUF_REG(dev)           (USB_REG_BASE(dev) + 0x18)
#define MV_USB_CORE_TTRX_BUF_REG(dev)           (USB_REG_BASE(dev) + 0x1C)

#define MV_USB_CORE_CAP_LENGTH_REG(dev)         (USB_REG_BASE(dev) + 0x100)
#define MV_USB_CORE_CAP_HCS_PARAMS_REG(dev)     (USB_REG_BASE(dev) + 0x104)
#define MV_USB_CORE_CAP_HCC_PARAMS_REG(dev)     (USB_REG_BASE(dev) + 0x108)

#define MV_USB_CORE_CAP_DCI_VERSION_REG(dev)    (USB_REG_BASE(dev) + 0x120)
#define MV_USB_CORE_CAP_DCC_PARAMS_REG(dev)     (USB_REG_BASE(dev) + 0x124)

#define MV_USB_CORE_CMD_REG(dev)                (USB_REG_BASE(dev) + 0x140)

#define MV_USB_CORE_CMD_RUN_BIT             0
#define MV_USB_CORE_CMD_RUN_MASK            (1 << MV_USB_CORE_CMD_RUN_BIT)

#define MV_USB_CORE_CMD_RESET_BIT           1
#define MV_USB_CORE_CMD_RESET_MASK          (1 << MV_USB_CORE_CMD_RESET_BIT)

#define MV_USB_CORE_STATUS_REG(dev)             (USB_REG_BASE(dev) + 0x144)
#define MV_USB_CORE_INTR_REG(dev)               (USB_REG_BASE(dev) + 0x148)
#define MV_USB_CORE_FRAME_INDEX_REG(dev)        (USB_REG_BASE(dev) + 0x14C)

#define MV_USB_CORE_PERIODIC_LIST_BASE_REG(dev) (USB_REG_BASE(dev) + 0x154)
#define MV_USB_CORE_DEV_ADDR_REG(dev)           (USB_REG_BASE(dev) + 0x154)

#define MV_USB_CORE_ASYNC_LIST_ADDR_REG(dev)    (USB_REG_BASE(dev) + 0x158)
#define MV_USB_CORE_ENDPOINT_LIST_ADDR_REG(dev) (USB_REG_BASE(dev) + 0x158)

#define MV_USB_CORE_TT_CTRL_REG(dev)            (USB_REG_BASE(dev) + 0x15C)
#define MV_USB_CORE_BURST_SIZE_REG(dev)         (USB_REG_BASE(dev) + 0x160)
#define MV_USB_CORE_TX_FILL_TUNING_REG(dev)     (USB_REG_BASE(dev) + 0x164)
#define MV_USB_CORE_TX_TT_FILL_TUNING_REG(dev)  (USB_REG_BASE(dev) + 0x168)
#define MV_USB_CORE_CONFIG_FLAG_REG(dev)        (USB_REG_BASE(dev) + 0x180)
#define MV_USB_CORE_PORTSC_REG(dev)             (USB_REG_BASE(dev) + 0x184)
#define MV_USB_CORE_OTGSC_REG(dev)              (USB_REG_BASE(dev) + 0x1A4)

#define MV_USB_CORE_MODE_REG(dev)               (USB_REG_BASE(dev) + 0x1A8)

#define MV_USB_CORE_MODE_OFFSET                 0
#define MV_USB_CORE_MODE_MASK                   (3 << MV_USB_CORE_MODE_OFFSET)
#define MV_USB_CORE_MODE_HOST                   (3 << MV_USB_CORE_MODE_OFFSET)
#define MV_USB_CORE_MODE_DEVICE                 (2 << MV_USB_CORE_MODE_OFFSET)

/* Bit[2] (ES) - don't care */

#define MV_USB_CORE_SETUP_LOCK_DISABLE_BIT      3
#define MV_USB_CORE_SETUP_LOCK_DISABLE_MASK     (1 << MV_USB_CORE_SETUP_LOCK_DISABLE_BIT)

#define MV_USB_CORE_STREAM_DISABLE_BIT          4
#define MV_USB_CORE_STREAM_DISABLE_MASK         (1 << MV_USB_CORE_STREAM_DISABLE_BIT)


#define MV_USB_CORE_ENDPT_SETUP_STAT_REG(dev)    (USB_REG_BASE(dev) + 0x1AC)
#define MV_USB_CORE_ENDPT_PRIME_REG(dev)         (USB_REG_BASE(dev) + 0x1B0)
#define MV_USB_CORE_ENDPT_FLUSH_REG(dev)         (USB_REG_BASE(dev) + 0x1B4)
#define MV_USB_CORE_ENDPT_STATUS_REG(dev)        (USB_REG_BASE(dev) + 0x1B8)
#define MV_USB_CORE_ENDPT_COMPLETE_REG(dev)      (USB_REG_BASE(dev) + 0x1BC)
#define MV_USB_CORE_ENDPT_CTRL_REG(dev, ep)      (USB_REG_BASE(dev) + 0x1C0 + (ep*4))

/*******************************************/
/* Interrupt Controller Registers          */
/*******************************************/
#define USB_CAUSE_REG(dev)                  (USB_REG_BASE(dev) + 0x310)
#define USB_MASK_REG(dev)                   (USB_REG_BASE(dev) + 0x314)
#define USB_ERROR_ADDR_REG(dev)             (USB_REG_BASE(dev) + 0x31c)

#define MV_USB_BRIDGE_INTR_CAUSE_REG(dev)        (USB_REG_BASE(dev) + 0x310)
#define MV_USB_BRIDGE_INTR_MASK_REG(dev)         (USB_REG_BASE(dev) + 0x314)

/*******************************************/
/* USB Bridge Registers                    */
/*******************************************/

/* BITs in Bridge Interrupt Cause and Mask registers */
#define MV_USB_ADDR_DECODE_ERROR_BIT        0
#define MV_USB_ADDR_DECODE_ERROR_MASK       (1<<MV_USB_ADDR_DECODE_ERROR_BIT)

#define MV_USB_BRIDGE_ERROR_ADDR_REG(dev)        (USB_REG_BASE(dev) + 0x31C)

#define MV_USB_BRIDGE_CTRL_REG(dev)              (USB_REG_BASE(dev) + 0x300)
#define MV_USB_BRIDGE_CORE_BYTE_SWAP_OFFSET 4
#define MV_USB_BRIDGE_CORE_BYTE_SWAP_MASK   (1 << MV_USB_BRIDGE_CORE_BYTE_SWAP_OFFSET)
#define MV_USB_BRIDGE_CORE_BYTE_SWAP_EN     (0 << MV_USB_BRIDGE_CORE_BYTE_SWAP_OFFSET)

#define MV_USB_BRIDGE_IPG_REG(dev)          (USB_REG_BASE(dev) + 0x360)

/*******************************************/
/* USB PHY Registers                       */
/*******************************************/

#define MV_USB_PHY_POWER_CTRL_REG(dev)          (USB_REG_BASE(dev) + 0x400)

#define MV_USB_PHY_POWER_UP_BIT                 0
#define MV_USB_PHY_POWER_UP_MASK                (1<<MV_USB_PHY_POWER_UP_BIT)

#define MV_USB_PHY_PLL_POWER_UP_BIT             1
#define MV_USB_PHY_PLL_POWER_UP_MASK            (1<<MV_USB_PHY_PLL_POWER_UP_BIT)

#define MV_USB_PHY_PLL_CTRL_REG(dev)            (USB_REG_BASE(dev) + 0x410)
#define MV_USB_PHY_TX_CTRL_REG(dev)             (USB_REG_BASE(dev) + 0x420) 
#define MV_USB_PHY_RX_CTRL_REG(dev)             (USB_REG_BASE(dev) + 0x430) 
#define MV_USB_PHY_IVREF_CTRL_REG(dev)          (USB_REG_BASE(dev) + 0x440) 
#define MV_USB_PHY_TEST_GROUP_CTRL_REG_0(dev)   (USB_REG_BASE(dev) + 0x450) 
#define MV_USB_PHY_TEST_GROUP_CTRL_REG_1(dev)   (USB_REG_BASE(dev) + 0x454) 

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCmvUsbRegsh */
