/*
 * Copyright (c) 1996, 2003 VIA Networking Technologies, Inc.
 * All rights reserved.
 *
 * This software may be redistributed and/or modified under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 *
 * File: rhine_wol.c
 *
 * Purpose: Functions to set WOL.
 *
 * Author: Chuang Liang-Shing, AJ Jiang
 *
 * Date: Aug 15, 2003
 *
 */

#include <linux/if_arp.h>

#include "rhine.h"
#include "rhine_wol.h"

static U32  adwMaskPattern[2][4]={
                {0x00203000, 0x000003C0, 0x00000000, 0x0000000}, //ARP
                {0xfffff000, 0xffffffff, 0xffffffff, 0x000ffff} //Magic Packet
};

static unsigned const ethernet_polynomial_le = 0xedb88320U;
static inline unsigned ether_crc_le(int length, unsigned char *data,U32 seed)
{
    unsigned int crc = seed;    /* Initial value. */
    while(--length >= 0) {
        unsigned char current_octet = *data++;
        int bit;
        for (bit = 8; --bit >= 0; current_octet >>= 1) {
            if ((crc ^ current_octet) & 1) {
                crc >>= 1;
                crc ^= ethernet_polynomial_le;
            } else
                crc >>= 1;
        }
    }
    return crc;
}

U32 WOLCalCRC (int size, PBYTE pbyPattern, PU8 abyMaskPattern)
{
    U32   dwCrc = 0xFFFFFFFFUL;
    U8    byMask;
    int     i, j;

    for (i = 0; i <size; i++) {
         byMask = abyMaskPattern[i];

     // skip this loop if the mask equals to zero
         if (byMask == 0x00) continue;

         for (j = 0; j < 8; j++) {
              if ((byMask & 0x01) == 0) {
                  byMask >>= 1;
                  continue;
                 }

              byMask >>= 1;
              dwCrc = ether_crc_le(1, &(pbyPattern[i * 8 + j]), dwCrc);
             }
        }

    // finally, invert the result once to get the correct data
    dwCrc = ~dwCrc;

    return dwCrc;
}

BOOL rhine_set_wol(PRHINE_INFO pInfo) {
    //PMAC_REGS   pMacRegs = pInfo->pMacRegs;
    static BYTE abyBuf[256];
    int         i;
    CSR_WRITE_1(&pInfo->hw, 0xFF, MAC_REG_WOLCR_CLR);
    if (pInfo->hw.byRevId > REV_ID_VT6105_B0)
        CSR_WRITE_1(&pInfo->hw, 0x03, MAC_REG_TSTREG_CLR);

    CSR_WRITE_1(&pInfo->hw, WOLCFG_SAB|WOLCFG_SAM, MAC_REG_WOLCG_SET);
    CSR_WRITE_1(&pInfo->hw, WOLCR_MAGPKT_EN, MAC_REG_WOLCR_SET);

    if (pInfo->hw.byRevId >= REV_ID_VT6105_B0)
        CSR_WRITE_1(&pInfo->hw, WOLCFG_PTNPAG, MAC_REG_WOLCG_CLR);

    if (pInfo->wol_opts & RHINE_WOL_PHY)
        CSR_WRITE_1(&pInfo->hw, (WOLCR_LNKON_EN|WOLCR_LNKOFF_EN), MAC_REG_WOLCR_SET);

    if (pInfo->wol_opts & RHINE_WOL_UCAST)
        CSR_WRITE_1(&pInfo->hw, WOLCR_UNICAST_EN, MAC_REG_WOLCR_SET);

    if (pInfo->wol_opts & RHINE_WOL_ARP) {
        PARP_PACKET pPacket=(PARP_PACKET) abyBuf;
        U32             dwCRC;
        memset(abyBuf, 0, sizeof(ARP_PACKET)+7);

        for (i=0;i<4;i++)
            CSR_WRITE_4(&pInfo->hw, adwMaskPattern[0][i], MAC_REG_BYTEMSK0_0+(i*4));

        pPacket->wType = htons(ETH_P_ARP);
        pPacket->ar_op = htons(1);

        memcpy(pPacket->ar_tip, pInfo->abyIPAddr, 4);
        printk("ip=%d.%d.%d.%d\n",
            pInfo->abyIPAddr[0],pInfo->abyIPAddr[1],
            pInfo->abyIPAddr[2],pInfo->abyIPAddr[3]);
        dwCRC=WOLCalCRC((sizeof(ARP_PACKET)+7)/8,
            abyBuf,(PU8)&adwMaskPattern[0][0]);
        CSR_WRITE_4(&pInfo->hw, htonl(dwCRC),MAC_REG_PATRN_CRC0);
        CSR_WRITE_1(&pInfo->hw, WOLCR_ARP_EN, MAC_REG_WOLCR_SET);
    }

    CSR_WRITE_1(&pInfo->hw, 0xFF, MAC_REG_PWRCSR_CLR);
    if (pInfo->hw.byRevId>REV_ID_VT6105_B0)
        CSR_WRITE_1(&pInfo->hw, 0x03, MAC_REG_PWRCSR1_CLR);

    // turn on Legacy WOL enable in stickhw
    BYTE_REG_BITS_ON(&pInfo->hw, STICKHW_LEGACY_WOLEN, MAC_REG_STICKHW);
    // turn on Legacy WOL enable
    CSR_WRITE_1(&pInfo->hw, PWCFG_LEGACY_WOLEN, MAC_REG_PWCFG_SET);

    //Go to bed .....
    BYTE_REG_BITS_ON(&pInfo->hw, (STICKHW_DS1_SHADOW|STICKHW_DS0_SHADOW),
            MAC_REG_STICKHW);

    return FALSE;
}

void
rhine_save_mac_context(PRHINE_INFO pInfo, PRHINE_CONTEXT pContext) {
    U16 i;

    for (i=MAC_REG_PAR;i<MAC_REG_CUR_RD_ADDR;i++)
        *(pContext->abyMacRegs+i)=CSR_READ_1(&pInfo->hw, i);

    for (i=MAC_REG_CUR_RD_ADDR;i<MAC_REG_CUR_TD_ADDR+4;i+=4)
        *((PU32)(pContext->abyMacRegs+i))=CSR_READ_4(&pInfo->hw, i);

    for (i=MAC_REG_MPHY;i<MAC_REG_CNTR_MPA;i++)
        *(pContext->abyMacRegs+i)=CSR_READ_1(&pInfo->hw, i);

    if (pInfo->hw.byRevId>=REV_ID_VT6105M_A0) {
        for (i=MAC_REG_CUR_TD_ADDR+4;i<MAC_REG_CUR_TD_ADDR+4*8;i+=4)
            *((PU32)(pContext->abyMacRegs+i))=CSR_READ_4(&pInfo->hw, i);

        rhine_get_cam_mask(&pInfo->hw,&pContext->dwCammask[RHINE_MULTICAST_CAM],
            RHINE_MULTICAST_CAM);
        rhine_get_cam_mask(&pInfo->hw,&pContext->dwCammask[RHINE_VLAN_ID_CAM],
            RHINE_VLAN_ID_CAM);
    }

    if (pInfo->hw.byRevId>=REV_ID_VT6105_A0) {
        for (i=MAC_REG_FLOWCR0;i<MAC_REG_SOFT_TIMER0;i++)
            *(pContext->abyMacRegs+i)=CSR_READ_1(&pInfo->hw, i);
    }

    if (pInfo->hw.byRevId>=REV_ID_VT6105_B0)
        CSR_WRITE_1(&pInfo->hw, WOLCFG_PTNPAG,MAC_REG_WOLCG_CLR);

    if (pInfo->hw.byRevId>=REV_ID_VT6102_A) {
        for (i=MAC_REG_WOLCR_SET;i<MAC_REG_WOLCR_CLR;i++) {
            // No need to restore WOLCR/WOLCR1 after power mode wake-up to disable power event in case that
            // after wake-up packet re-triggers the PME status.
            if ((i == MAC_REG_WOLCR_SET) || (i == MAC_REG_TSTREG_SET)) {
                continue;
            }
            else {
                *(pContext->abyMacRegs+i)=CSR_READ_1(&pInfo->hw, i);
            }
        }
        for (i=MAC_REG_PATRN_CRC0;i<MAC_REG_BYTEMSK3_3+4;i+=4)
            *((PU32)(pContext->abyMacRegs+i))=CSR_READ_4(&pInfo->hw, i);
    }

    if (pInfo->hw.byRevId>=REV_ID_VT6105_A0) {

        if (pInfo->hw.byRevId>=REV_ID_VT6105_B0) {
            int j=0;
            CSR_WRITE_1(&pInfo->hw, WOLCFG_PTNPAG,MAC_REG_WOLCG_SET);
            for (i=MAC_REG_PATRN_CRC0,j=0;i<MAC_REG_PATRN_CRC1+4;i+=4,j++)
                pContext->dwPatCRC[j]=CSR_READ_4(&pInfo->hw, i);
            for (i=MAC_REG_BYTEMSK0_0,j=0;i<MAC_REG_BYTEMSK1_3+4;i+=4,j++)
                pContext->dwPattern[j]=CSR_READ_4(&pInfo->hw, i);
        }

        if (pInfo->hw.byRevId>=REV_ID_VT6105M_B1) {
            for (i=0;i<MCAM_SIZE;i++)
                rhine_get_cam(&pInfo->hw,i,pContext->aMcamAddr[i],
                    RHINE_MULTICAST_CAM);

            for (i=0;i<VCAM_SIZE;i++)
                rhine_get_cam(&pInfo->hw,i,(PU8) &(pContext->awVcam[i]),
                    RHINE_VLAN_ID_CAM);
        }

    }

}

void
rhine_restore_mac_context(PRHINE_INFO pInfo, PRHINE_CONTEXT pContext) {
    U16 i;

    for (i=MAC_REG_PAR; i<MAC_REG_CUR_RD_ADDR; i++) {
        if (i != MAC_REG_CR0)
            CSR_WRITE_1(&pInfo->hw, *(pContext->abyMacRegs+i), i);
    }

    for (i=MAC_REG_CUR_RD_ADDR; i<MAC_REG_CUR_TD_ADDR+4; i+=4)
        CSR_WRITE_4(&pInfo->hw, *((PU32)(pContext->abyMacRegs+i)), i);

    for (i=MAC_REG_MPHY; i<MAC_REG_CNTR_MPA; i++)
        CSR_WRITE_1(&pInfo->hw, *(pContext->abyMacRegs+i), i);

    if (pInfo->hw.byRevId >= REV_ID_VT6105M_A0) {
        for (i=MAC_REG_CUR_TD_ADDR+4; i<MAC_REG_CUR_TD_ADDR+4*8; i+=4)
            CSR_WRITE_4(&pInfo->hw, *((PU32)(pContext->abyMacRegs+i)), i);

        rhine_set_cam_mask(&pInfo->hw, pContext->dwCammask[RHINE_MULTICAST_CAM],
            RHINE_MULTICAST_CAM);
        rhine_set_cam_mask(&pInfo->hw, pContext->dwCammask[RHINE_VLAN_ID_CAM],
            RHINE_VLAN_ID_CAM);
    }

    if (pInfo->hw.byRevId >= REV_ID_VT6105_A0) {
        for (i=MAC_REG_FLOWCR0; i<MAC_REG_SOFT_TIMER0; i++)
            CSR_WRITE_1(&pInfo->hw, *(pContext->abyMacRegs+i), i);
    }

    if (pInfo->hw.byRevId >= REV_ID_VT6105_B0)
        CSR_WRITE_1(&pInfo->hw, WOLCFG_PTNPAG, MAC_REG_WOLCG_CLR);

    if (pInfo->hw.byRevId >= REV_ID_VT6102_A) {
        for (i=MAC_REG_WOLCR_SET; i<MAC_REG_WOLCR_CLR; i++) {
            // No need to restore WOLCR/WOLCR1 after power mode wake-up to disable power event in case that
            // after wake-up packet re-triggers the PME status.
            if ((i == MAC_REG_WOLCR_SET) || (i == MAC_REG_TSTREG_SET)) {
                continue;
            }
            else {
                CSR_WRITE_1(&pInfo->hw, *(pContext->abyMacRegs+i), i);
            }
        }
        for (i=MAC_REG_PATRN_CRC0; i<MAC_REG_BYTEMSK3_3+4; i+=4)
            CSR_WRITE_4(&pInfo->hw, *((PU32)(pContext->abyMacRegs+i)), i);
    }

    if (pInfo->hw.byRevId >= REV_ID_VT6105_A0) {
        if (pInfo->hw.byRevId >= REV_ID_VT6105_B0) {
            int j=0;
            CSR_WRITE_1(&pInfo->hw, WOLCFG_PTNPAG, MAC_REG_WOLCG_SET);
            for (i=MAC_REG_PATRN_CRC0,j=0; i<MAC_REG_PATRN_CRC1+4; i+=4,j++)
                CSR_WRITE_4(&pInfo->hw, pContext->dwPatCRC[j], i);
            for (i=MAC_REG_BYTEMSK0_0,j=0; i<MAC_REG_BYTEMSK1_3+4; i+=4,j++)
                CSR_WRITE_4(&pInfo->hw, pContext->dwPattern[j], i);
        }

        if (pInfo->hw.byRevId >= REV_ID_VT6105M_B1) {
            for (i=0; i<MCAM_SIZE; i++)
                rhine_set_cam(&pInfo->hw, i, pContext->aMcamAddr[i],
                    RHINE_MULTICAST_CAM);

            for (i=0; i<VCAM_SIZE; i++)
                rhine_set_cam(&pInfo->hw,i,(PU8) &(pContext->awVcam[i]),
                    RHINE_VLAN_ID_CAM);
        }
    }
}

void 
rhine_save_pci_context(PRHINE_INFO pInfo, PU32 pContext) {
    // save MODE0 ~ MODE3 registers
    pci_read_config_dword(pInfo->pcid, PCI_REG_MODE0, pContext);
}

void 
rhine_restore_pci_context(PRHINE_INFO pInfo, U32 Context) {
    // restore MODE0 ~ MODE3 registers
    pci_write_config_dword(pInfo->pcid, PCI_REG_MODE0, Context);
}
