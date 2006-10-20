/*
 * Copyright (c) 1996, 2003 VIA Networking Technologies, Inc.
 * All rights reserved.
 *
 * This software is copyrighted by and is the sole property of
 * VIA Networking Technologies, Inc. This software may only be used
 * in accordance with the corresponding license agreement. Any unauthorized
 * use, duplication, transmission, distribution, or disclosure of this
 * software is expressly forbidden.
 *
 * This software is provided by VIA Networking Technologies, Inc. "as is"
 * and any express or implied warranties, including, but not limited to, the
 * implied warranties of merchantability and fitness for a particular purpose
 * are disclaimed. In no event shall VIA Networking Technologies, Inc.
 * be liable for any direct, indirect, incidental, special, exemplary, or
 * consequential damages.
 *
 *
 * File: rhine_hw.c
 *
 * Purpose: shared function for accessing and configurating Rhine series MAC.
 *
 * Author: Chuang Liang-Shing, AJ Jiang, Guard Kuo
 *
 * Date: Feb 1, 2005
 *
 * Functions:
 *      List all the functions this module provides.
 *      (This section is omitted in the header of ".h" files)
 *
 * Revision History:
 *      03-17-2005 Guard Kuo : First released.
 *      06-07-2005 Guard Kuo : 1.Fixed the bug of NWAY-force mode. If user forces the duplex mode to 
 *                               full and the partner device is legacy device, the driver reports
 *                               the current duplex mode is half. It makes the duplex of MAC/PHY
 *                               dismatched.
 *                             2.Removed mii_status on rhine_hw structure.
 *                             3.Removed rhine_get_opt_media_mode()
 *      06-15-2006 Guard Kuo : 1.If the current ANAR is same as the setting value, the Auto/Reauto isn't
 *                               executed. rhine_set_media_mode()
 *                               
 *
 */

#include "rhine_hw.h"

/************************************************************************
* MII access , media link mode setting functions
************************************************************************/
void rhine_disable_mii_auto_poll(struct rhine_hw *hw)
{
    WORD        ww;

    /* turn off MAUTO */
    CSR_WRITE_1(hw, 0, MAC_REG_MIICR);

    /* for VT86C100A only */
    if (hw->byRevId < REV_ID_VT3071_A) {
        /* turn off MSRCEN
        // NOTE.... address of MII should be 0x01,
        //          otherwise SRCI will invoked */
        CSR_WRITE_1(hw, 1, MAC_REG_MIIAD);

        mdelay(1);

        /* turn on MAUTO */
        CSR_WRITE_1(hw, MIICR_MAUTO, MAC_REG_MIICR);

        /* W_MAX_TIMEOUT is the timeout period */
        for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
            udelay(5);
            if(BYTE_REG_BITS_IS_ON(hw, MIIAD_MDONE, MAC_REG_MIIAD))
                break;
        }

        /* as soon as MDONE is on,
        // this is the right time to turn off MAUTO */
        CSR_WRITE_1(hw, 0, MAC_REG_MIICR);
    }
    else {
        /* as soon as MIDLE is on, MAUTO is really stoped */
        for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
            udelay(5);
            if (BYTE_REG_BITS_IS_ON(hw, MIIAD_MIDLE, MAC_REG_MIIAD))
                break;
        }
    }
}

void rhine_enable_mii_auto_poll(struct rhine_hw *hw) {
    int ii;
    CSR_WRITE_1(hw, 0, MAC_REG_MIICR);
    CSR_WRITE_1(hw, MIIAD_MSRCEN|0x01, MAC_REG_MIIAD);
    CSR_WRITE_1(hw, MIICR_MAUTO, MAC_REG_MIICR);

    for (ii=0;ii<W_MAX_TIMEOUT; ii++)
        if (BYTE_REG_BITS_IS_ON(hw, MIIAD_MDONE, MAC_REG_MIIAD))
            break;
    BYTE_REG_BITS_ON(hw, MIIAD_MSRCEN,MAC_REG_MIIAD);
}

BOOL rhine_mii_read(struct rhine_hw *hw, U8 byIdx, PU16 pdata)
{
    WORD        ww;

    /* disable MIICR_MAUTO, so that mii addr can be set normally */
    rhine_disable_mii_auto_poll(hw);

    CSR_WRITE_1(hw, byIdx, MAC_REG_MIIAD);

    BYTE_REG_BITS_ON(hw, MIICR_RCMD, MAC_REG_MIICR);

    for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
        if(!BYTE_REG_BITS_IS_ON(hw, MIICR_RCMD, MAC_REG_MIICR))
            break;
    }

    *pdata=CSR_READ_2(hw, MAC_REG_MIIDATA);

    /* for VT3043 only */
    if (hw->byRevId < REV_ID_VT6102_A)
        mdelay(1);

    rhine_enable_mii_auto_poll(hw);

    if (ww == W_MAX_TIMEOUT) {
        return FALSE;
    }
    return TRUE;
}

BOOL rhine_mii_write (struct rhine_hw *hw, BYTE byMiiAddr, WORD wData)
{
    WORD        ww;

    /* disable MIICR_MAUTO, so that mii addr can be set normally */
    rhine_disable_mii_auto_poll(hw);

    /* MII reg offset */
    CSR_WRITE_1(hw, byMiiAddr, MAC_REG_MIIAD);
    /* set MII data */
    CSR_WRITE_2(hw, wData, MAC_REG_MIIDATA);

    /* turn on MIICR_WCMD */
    BYTE_REG_BITS_ON(hw, MIICR_WCMD, MAC_REG_MIICR);

    /* W_MAX_TIMEOUT is the timeout period */
    for (ww = 0; ww < W_MAX_TIMEOUT; ww++) {
        udelay(5);
        if(!BYTE_REG_BITS_IS_ON(hw, MIICR_WCMD, MAC_REG_MIICR))
            break;
    }

    /* for VT3043 only */
    if (hw->byRevId < REV_ID_VT6102_A)
        mdelay(1);

    rhine_enable_mii_auto_poll(hw);

    if (ww == W_MAX_TIMEOUT) {
        return FALSE;
    }
    return TRUE;
}

void rhine_get_phy_id(struct rhine_hw *hw)
{
    rhine_mii_read(hw, MII_REG_PHYID2, (PU16) &hw->dwPHYId);
    rhine_mii_read(hw, MII_REG_PHYID1, ((PU16) &(hw->dwPHYId) + 1));
}


void rhine_disable_int(struct rhine_hw *hw) {
    CSR_WRITE_2(hw, 0, MAC_REG_IMR);
    if(hw->byRevId > REV_ID_VT6102_A)
        CSR_WRITE_1(hw, 0, MAC_REG_MIMR);
}

void rhine_enable_int(struct rhine_hw *hw, U32 IsrStatus) {
    BOOL        bDeferRx = FALSE;
    BOOL        bDeferTx = FALSE;

    /* [4.39] For adaptive interrupt */
    if (hw->byRevId > REV_ID_VT6102_A) {
        if (IsrStatus & ISR_PRX)
            bDeferRx = TRUE;
        if (IsrStatus & ISR_PTX)
            bDeferTx = TRUE;

        if (bDeferRx && bDeferTx) {
            /* Defer PTXM, PRXM */
            CSR_WRITE_2(hw, (WORD)IMR_MASK_EXCEPT_PTXPRX, MAC_REG_IMR);
        }
        else if (bDeferRx && !bDeferTx) {
            /* Only defer PRXM */
            CSR_WRITE_2(hw, (WORD)IMR_MASK_EXCEPT_PRX, MAC_REG_IMR);
        }
        else if (!bDeferRx && bDeferTx) {
            /* Only defer PTXM */
            CSR_WRITE_2(hw, (WORD)IMR_MASK_EXCEPT_PTX, MAC_REG_IMR);
        }
        else {
            /* No defer */
            CSR_WRITE_2(hw, (WORD)IMR_MASK_VALUE, MAC_REG_IMR);
        }
        CSR_WRITE_1(hw, 0x0B, MAC_REG_MIMR);

        /* Set Timer0 */
        if (bDeferTx || bDeferRx) {
            if (hw->byRevId >= REV_ID_VT6105_A0) {
                /* [VT6105 above use micro-second timer]
                // VT6105 only, set Timer0 in mini-second resolution */
                BYTE_REG_BITS_ON(hw, MISC_CR0_TM0US, MAC_REG_MISC_CR0);
                /* set delay time to udelay, unit is us */
                CSR_WRITE_2(hw, 1000, MAC_REG_SOFT_TIMER0);
                /* set time0 */
                BYTE_REG_BITS_ON(hw, MISC_CR0_TIMER0_EN, MAC_REG_MISC_CR0);
                BYTE_REG_BITS_OFF(hw, MISC_CR0_TIMER0_SUSPEND, MAC_REG_MISC_CR0);
            }
            else {
                /* [VT6102 below use mili-second timer]
                // VT6105 only, set Timer0 in mini-second resolution */
                BYTE_REG_BITS_OFF(hw, MISC_CR0_TM0US, MAC_REG_MISC_CR0);
                /* set delay time to udelay, unit is ms */
                CSR_WRITE_1(hw, 1, MAC_REG_SOFT_TIMER0);
                /* set timer0 */
                BYTE_REG_BITS_ON(hw, MISC_CR0_TIMER0_EN, MAC_REG_MISC_CR0);
                BYTE_REG_BITS_OFF(hw, MISC_CR0_TIMER0_SUSPEND, MAC_REG_MISC_CR0);
            }
        }
    }
    else {
        /* Don't defer IMR */
        CSR_WRITE_2(hw, (WORD)IMR_MASK_VALUE, MAC_REG_IMR);
        CSR_WRITE_1(hw, 0x0B, MAC_REG_MIMR);
    }
}

void rhine_set_duplex(struct rhine_hw *hw, BOOL bFlag) {

    if (bFlag)
        BYTE_REG_BITS_ON(hw, CR1_FDX, MAC_REG_CR1);
    else
        BYTE_REG_BITS_OFF(hw, CR1_FDX, MAC_REG_CR1);

    if (hw->byRevId>=REV_ID_VT6102_A)
    {
        if(bFlag)
            CSR_WRITE_1(hw, WOLCFG_SFDX, MAC_REG_WOLCG_SET);
        else
            CSR_WRITE_1(hw, WOLCFG_SFDX, MAC_REG_WOLCG_CLR);
    }
}

/*
// Get the media mode stored in EEPROM or module options
*/

void mii_set_auto_on(struct rhine_hw *hw)
{

    if ((hw->byRevId >= REV_ID_VT6105_A0) || IS_PHY_VT6103(hw))
        MII_REG_BITS_OFF(1, MII_REG_MODCFG, hw);

    MII_REG_BITS_ON((BMCR_AUTO | BMCR_REAUTO), MII_REG_BMCR, hw);
}


void mii_set_auto_off(struct rhine_hw *hw)
{

    if ((hw->byRevId>=REV_ID_VT6105_A0) ||
        ((hw->byRevId>=REV_ID_VT6102_A) && IS_PHY_VT6103(hw)))
        MII_REG_BITS_ON(1, MII_REG_MODCFG, hw);

    MII_REG_BITS_OFF(BMCR_AUTO, MII_REG_BMCR, hw);
}
/*
// This function check if MAC operation at Full duplex mode
*/
BOOL rhine_is_full_duplex (struct rhine_hw *hw)
{
    U8  byRevId=hw->byRevId;
    /* if in AUTO-NEGO mode */
    if (byRevId>=REV_ID_VT6105_A0)
        return BYTE_REG_BITS_IS_ON(hw, MIISR_N_FDX, MAC_REG_MIISR);

    if (MII_REG_BITS_IS_ON(BMCR_AUTO, MII_REG_BMCR, hw)) {

        /* if my TX_FD on, check both TX_FD */
        if (MII_REG_BITS_IS_ON(ANAR_TXFD, MII_REG_ANAR, hw)) {
            /* partner's TX_FD */
            if (MII_REG_BITS_IS_ON(ANLPAR_TXFD, MII_REG_ANLPAR, hw))
                return TRUE;
        }

        /* if my T4 on, check both T4 */
        if (MII_REG_BITS_IS_ON(ANAR_T4, MII_REG_ANAR, hw)) {
            /* partner's T4 */
            if (MII_REG_BITS_IS_ON(ANLPAR_T4, MII_REG_ANLPAR, hw))
                return FALSE;
        }

        /* if my TX_HD on, check both TX_HD */
        if (MII_REG_BITS_IS_ON(ANAR_TX, MII_REG_ANAR, hw)) {
            /* partner's TX_HD */
            if (MII_REG_BITS_IS_ON(ANLPAR_TX, MII_REG_ANLPAR, hw))
                return FALSE;
        }

        /* if my 10_FD on, check both 10_FD */
        if (MII_REG_BITS_IS_ON(ANAR_10FD, MII_REG_ANAR, hw)) {
            /* partner's 10_FD */
            if (MII_REG_BITS_IS_ON(ANLPAR_10FD, MII_REG_ANLPAR, hw))
                return TRUE;
        }

        /* if all above is not, then it would be 10_HD or no link
        // both case will be half duplex */
        return FALSE;
    }
    /* if in force mode */
    else {

        if (!MII_REG_BITS_IS_ON(BMCR_FDX, MII_REG_BMCR, hw))
            return FALSE;
    }

    return TRUE;
}


/*
   Only check current connection type and does not modify any internal register.
*/
U32 rhine_check_media_mode(struct rhine_hw *hw)
{
    U32                 status=0;

    if (BYTE_REG_BITS_IS_ON(hw, MIISR_LNKFL, MAC_REG_MIISR)) {
        status|=RHINE_LINK_FAIL;
    }
    else {
        if(hw->sOpts.spd_dpx == SPD_DPX_AUTO)
        {
            if (BYTE_REG_BITS_IS_ON(hw, MIISR_SPEED, MAC_REG_MIISR))
                status|=RHINE_SPEED_10;
            else
                status|=RHINE_SPEED_100;

            /* check duplex mode*/
            /* if VT6105, check N_FDX bit in MII Status Register directly */
            if (rhine_is_full_duplex(hw)) {
                status|=RHINE_DUPLEX_FULL;
                rhine_set_duplex(hw, TRUE);
            }
            else {
                rhine_set_duplex(hw, FALSE);
            }
            status |= RHINE_AUTONEG_ENABLE;
        }
        else
        {
            switch(hw->sOpts.spd_dpx)
            {
                case SPD_DPX_100_HALF:
                    status |= RHINE_SPEED_100;
                    break;
                case SPD_DPX_100_FULL:
                    status |= (RHINE_SPEED_100|RHINE_DUPLEX_FULL);
                    break;
                case SPD_DPX_10_FULL:
                    status |= RHINE_DUPLEX_FULL;
                    break;
                case SPD_DPX_10_HALF:
                default:
                    break;
            }
        }
    }

    return status;
}


void rhine_clearISR(struct rhine_hw *hw) {
    CSR_WRITE_2(hw, 0xFFFF, MAC_REG_ISR);
    CSR_WRITE_1(hw, 0xDF, MAC_REG_MISR);
}

void rhine_enable_mmio(struct rhine_hw *hw) {
    int n;

    if (hw->byRevId < REV_ID_VT6102_A) {
        n = _INB(hw, MAC_REG_CFGA) | 0x20;
        _OUTB(hw, n, MAC_REG_CFGA);
    }
    else {
        n = _INB(hw, MAC_REG_CFGD) | CFGD_GPIOEN;
        _OUTB(hw, n, MAC_REG_CFGD);
    }
}

void rhine_reload_eeprom(struct rhine_hw *hw) {
    int i;

    _OUTB(hw, _INB(hw, MAC_REG_EECSR)|EECSR_AUTOLD, MAC_REG_EECSR);

    /* Typically 2 cycles to reload. */
        for (i = 0; i < 150; i++)
            if(!(_INB(hw, MAC_REG_EECSR) & EECSR_AUTOLD))
                break;

    rhine_enable_mmio(hw);
}

void rhine_init_flow_control_register(struct rhine_hw *hw, U16 RxDescs) {

    if (hw->byRevId >= REV_ID_VT6105_A0) {
    /* Set {XHITH1, XHITH0, XLTH1, XLTH0} in FlowCR1 to {1, 0, 1, 1}
    // depend on RD=64, and Turn on XNOEN in FlowCR1 */
        BYTE_REG_BITS_SET( hw,
            (FLOWCR1_XONEN|FLOWCR1_XHITH1|FLOWCR1_XLTH1|FLOWCR1_XLTH0),
            0xFF,MAC_REG_FLOWCR1);

        /* Set TxPauseTimer to 0xFFFF */
        CSR_WRITE_2(hw, 0xFFFF, MAC_REG_PAUSE_TIMER);

        /* Initialize RBRDU to Rx buffer count */
        CSR_WRITE_1(hw, RxDescs, MAC_REG_FLOWCR0);
    }
}

void
rhine_set_tx_thresh(struct rhine_hw *hw, int tx_thresh) {
    BYTE_REG_BITS_SET(hw, tx_thresh <<3,
        (BCR1_CTSF|BCR1_CTFT1|BCR1_CTFT0), MAC_REG_BCR1);

    BYTE_REG_BITS_SET(hw, tx_thresh <<5,
        (TCR_RTSF|TCR_RTFT1|TCR_RTFT0), MAC_REG_TCR);
}

void
rhine_set_rx_thresh(struct rhine_hw *hw, int rx_thresh) {
    BYTE_REG_BITS_SET(hw, rx_thresh <<3,
        (BCR0_CRFT2|BCR0_CRFT1|BCR0_CRFT0), MAC_REG_BCR0);

    BYTE_REG_BITS_SET(hw, rx_thresh <<5,
        (RCR_RRFT2|RCR_RRFT1|RCR_RRFT0), MAC_REG_RCR);
}
void
rhine_set_DMA_length(struct rhine_hw *hw, int DMA_length) {
    BYTE_REG_BITS_SET(hw, DMA_length,
        (BCR0_DMAL2|BCR0_DMAL1|BCR0_DMAL0),MAC_REG_BCR0);
}

U32 rhine_ReadISR(struct rhine_hw *hw) {
    U32 status=0;
    status=CSR_READ_2(hw, MAC_REG_ISR);
    if (hw->byRevId>REV_ID_VT6102_A)
        status |= CSR_READ_2(hw, MAC_REG_MISR) << 16;
    return status;
}

void rhine_WriteISR(U32 status,struct rhine_hw *hw)    {
    CSR_WRITE_2(hw, (status & 0xFFFF), MAC_REG_ISR);
    if (hw->byRevId>REV_ID_VT6102_A)
        CSR_WRITE_2(hw, (status>>16), MAC_REG_MISR);
}

void rhine_wol_reset(struct rhine_hw *hw) {
    if (hw->byRevId >= REV_ID_VT6102_A) {
        /* clear sticky bits */
        BYTE_REG_BITS_OFF(hw, (STICKHW_DS1_SHADOW|STICKHW_DS0_SHADOW),
            MAC_REG_STICKHW);
        /* disable force PME-enable */
        CSR_WRITE_1(hw, WOLCFG_PME_OVR, MAC_REG_WOLCG_CLR);
        /* disable power-event config bit */
        CSR_WRITE_1(hw, 0xFF, MAC_REG_WOLCR_CLR);
        if (hw->byRevId>REV_ID_VT6105_B0)
            CSR_WRITE_1(hw, 0x03, MAC_REG_TSTREG_CLR);
        /* clear power status */
        CSR_WRITE_1(hw, 0xFF, MAC_REG_PWRCSR_CLR);
        if (hw->byRevId>REV_ID_VT6105_B0)
            CSR_WRITE_1(hw, 0x03, MAC_REG_PWRCSR1_CLR);
    }
}

BOOL rhine_soft_reset(struct rhine_hw *hw) {
    int i=0;

    BYTE_REG_BITS_ON(hw, CR1_SFRST, MAC_REG_CR1);

    /* VT86C100A may need long delay after reset (dlink) */
    if (hw->byRevId < REV_ID_VT6102_A)
        udelay(100);

    for (i=0; i<W_MAX_TIMEOUT; i++) {
        udelay(5);
        if ( !BYTE_REG_BITS_IS_ON(hw, CR1_SFRST, MAC_REG_CR1))
            break;
    }

    if (i == W_MAX_TIMEOUT) {
        if (hw->byRevId < REV_ID_VT6102_A) {
            return FALSE;
        }

        BYTE_REG_BITS_ON(hw, MISC_CR1_FORSRST, MAC_REG_MISC_CR1);
        /* delay 2ms */
        mdelay(2);
    }
    return TRUE;
}

void rhine_get_cam_mask(struct rhine_hw* hw, PU32 pMask, RHINE_CAM_TYPE cam_type)
{
    /* enable CAMEN */
    if (cam_type == RHINE_VLAN_ID_CAM)
        CSR_WRITE_1(hw, CAMC_CAMEN | CAMC_VCAMSL, MAC_REG_CAMC);
    else
        CSR_WRITE_1(hw, CAMC_CAMEN,MAC_REG_CAMC);
    wmb();
    /* read mask */
    *pMask = CSR_READ_4(hw, MAC_REG_CAMMSK);

    /* disable CAMEN */
    CSR_WRITE_1(hw, 0, MAC_REG_CAMC);

}

void rhine_set_cam_mask(struct rhine_hw *hw, U32 mask, RHINE_CAM_TYPE cam_type)
{

    if (cam_type == RHINE_VLAN_ID_CAM)
        CSR_WRITE_1(hw, CAMC_CAMEN | CAMC_VCAMSL, MAC_REG_CAMC);
    else
        CSR_WRITE_1(hw, CAMC_CAMEN, MAC_REG_CAMC);
    wmb();
    /* write mask */
    CSR_WRITE_4(hw, mask, MAC_REG_CAMMSK);

    /* disable CAMEN */
    CSR_WRITE_1(hw, 0, MAC_REG_CAMC);

}

void rhine_get_cam(struct rhine_hw *hw, int idx, PU8 addr, RHINE_CAM_TYPE cam_type)
{

    int i;

    if (cam_type==RHINE_VLAN_ID_CAM)
        CSR_WRITE_1(hw, CAMC_CAMEN | CAMC_VCAMSL, MAC_REG_CAMC);
    else
        CSR_WRITE_1(hw, CAMC_CAMEN,MAC_REG_CAMC);

    wmb();
    CSR_WRITE_1(hw, (U8)(idx & 0x1F), MAC_REG_CAMADD);
    wmb();

    CSR_WRITE_1(hw, CAMC_CAMRD|CAMC_CAMEN,MAC_REG_CAMC);

    wmb();
    udelay(10);

    if (cam_type==RHINE_VLAN_ID_CAM)
        *((PU16) addr)=CSR_READ_2(hw, MAC_REG_VCAM);
    else
        for (i=0;i<6;i++, addr++)
            *((PU8)addr)=CSR_READ_1(hw, MAC_REG_MCAM);

    CSR_WRITE_1(hw, 0, MAC_REG_CAMC);
}

void rhine_set_cam(struct rhine_hw *hw, int idx, PU8 addr, RHINE_CAM_TYPE cam_type)
{
    int i;

    if (cam_type==RHINE_VLAN_ID_CAM)
        CSR_WRITE_1(hw, CAMC_CAMEN|CAMC_VCAMSL, MAC_REG_CAMC);
    else
        CSR_WRITE_1(hw, CAMC_CAMEN, MAC_REG_CAMC);

    wmb();
    CSR_WRITE_1(hw, (U8)(idx & idx), MAC_REG_CAMADD);

    if (cam_type==RHINE_VLAN_ID_CAM) {
        CSR_WRITE_1(hw, *addr, MAC_REG_MAR+6);
        CSR_WRITE_1(hw, *(addr+1), MAC_REG_MAR+7);
    }
    else
        for (i=0;i<6;i++, addr++)
            CSR_WRITE_1(hw, *addr, MAC_REG_MAR+i);
    udelay(10);
    wmb();
    CSR_WRITE_1(hw, CAMC_CAMWR|CAMC_CAMEN, MAC_REG_CAMC);
    udelay(10);

    CSR_WRITE_1(hw, 0, MAC_REG_CAMC);
}

void enable_flow_control_ability(struct rhine_hw *hw) {

    U16         wANAR,wANLPAR;
    U8          byFlowCR;
    U32         status;
    
    status = rhine_check_media_mode(hw);

    rhine_mii_read(hw,MII_REG_ANAR,&wANAR);
    rhine_mii_read(hw,MII_REG_ANLPAR,&wANLPAR);

    if (hw->byRevId<REV_ID_VT6105_A0)
        byFlowCR=CSR_READ_1(hw, MAC_REG_MISC_CR0);
    else
        byFlowCR=CSR_READ_1(hw, MAC_REG_FLOWCR1);

    if (!(status & RHINE_DUPLEX_FULL)) {
        /* Half duplex mode */
        if (hw->byRevId<REV_ID_VT6105_A0) {
            byFlowCR&=~MISC_CR0_FDXRFEN;
        }
        else {
            byFlowCR&=~FLOWCR1_FDXTFCEN;
        }
    }
    else {
        /*Full duplxe mode */
        if (((wANAR & (ANAR_ASMDIR|ANAR_PAUSE))==ANAR_ASMDIR)
            && ((wANLPAR & (ANLPAR_ASMDIR|ANLPAR_PAUSE))
            ==(ANLPAR_ASMDIR|ANLPAR_PAUSE))) {
            if (hw->byRevId<REV_ID_VT6105_A0) {
                byFlowCR&=~MISC_CR0_FDXRFEN;
            }
            else {
                byFlowCR|=FLOWCR1_FDXTFCEN;
                byFlowCR&=~FLOWCR1_FDXRFCEN;
            }
        }
        else if ((wANAR & ANAR_PAUSE) && (wANLPAR & ANLPAR_PAUSE)) {
            if (hw->byRevId<REV_ID_VT6105_A0)
                byFlowCR|=MISC_CR0_FDXRFEN;
            else
                byFlowCR|=(FLOWCR1_FDXTFCEN|FLOWCR1_FDXRFCEN);
        }
        else if (((wANAR & (ANAR_ASMDIR|ANAR_PAUSE))==(ANAR_ASMDIR|ANAR_PAUSE))
            && ((wANLPAR & (ANLPAR_ASMDIR|ANLPAR_PAUSE)) ==ANLPAR_ASMDIR)) {
            if (hw->byRevId<REV_ID_VT6105_A0)
                byFlowCR|=MISC_CR0_FDXRFEN;
            else {
                byFlowCR&=~FLOWCR1_FDXTFCEN;
                byFlowCR|=FLOWCR1_FDXRFCEN;
            }
        }
        else {
            if (hw->byRevId<REV_ID_VT6105_A0)
                byFlowCR&=~MISC_CR0_FDXRFEN;
            else
                byFlowCR&=~(FLOWCR1_FDXTFCEN|FLOWCR1_FDXRFCEN);
        }

    }

    if (hw->byRevId< REV_ID_VT6105_A0) {
        byFlowCR &=~(MISC_CR0_HDXFEN|MISC_CR0_FDXTFEN);
        CSR_WRITE_1(hw, byFlowCR, MAC_REG_MISC_CR0);
    }
    else {
        byFlowCR &=~FLOWCR1_HDXFCEN;
        CSR_WRITE_1(hw, byFlowCR, MAC_REG_FLOWCR1);
    }

}

int rhine_set_media_mode(struct rhine_hw *hw, POPTIONS option) {
    U8          byRevId = hw->byRevId;
    U8          byFlowCR;
    U16         wOrigANAR;
    U16         wANAR=0;
    U16	        wANARMask;
    U32         status=RHINE_LINK_UNCHANGE;

    /* get original ANAR */
    rhine_mii_read(hw, MII_REG_ANAR, &wOrigANAR);
    wANARMask = wOrigANAR;
    wOrigANAR &= (ANAR_10|ANAR_10FD|ANAR_TX|ANAR_TXFD|ANAR_PAUSE|ANAR_ASMDIR);
    wANARMask &= ~(ANAR_10|ANAR_10FD|ANAR_TX|ANAR_TXFD|ANAR_PAUSE|ANAR_ASMDIR);

    if (byRevId >= REV_ID_VT6102_A) {
        /* read original flow control setting from MAC */
        if (byRevId < REV_ID_VT6105_A0) {
            byFlowCR = CSR_READ_1(hw, MAC_REG_MIICR);
        }
        else {
            byFlowCR = CSR_READ_1(hw, MAC_REG_FLOWCR1);
        }

        /* Disable PAUSE in ANAR, disable TX/RX flow control in MAC */
        if (option->flow_cntl == 2) {

            if (byRevId < REV_ID_VT6105_A0) {
                byFlowCR &= ~MISC_CR0_FDXRFEN;
            }
            else {
                byFlowCR &= ~FLOWCR1_FDXTFCEN;
                byFlowCR &= ~FLOWCR1_FDXRFCEN;
            }
        }
        else if (option->flow_cntl == 3) {
            /* Enable PAUSE in ANAR, enable TX/RX flow control in MAC */
            wANAR |= ANAR_PAUSE;

            if (byRevId < REV_ID_VT6105_A0) {
                byFlowCR |= MISC_CR0_FDXRFEN;
            }
            else {
                byFlowCR |= FLOWCR1_FDXTFCEN;
                byFlowCR |= FLOWCR1_FDXRFCEN;
            }
        }

        /* read new flow control setting from MAC */
        if (byRevId < REV_ID_VT6105_A0) {
            byFlowCR &= ~(MISC_CR0_HDXFEN|MISC_CR0_FDXTFEN);
            CSR_WRITE_1(hw, byFlowCR, MAC_REG_MISC_CR0);
        }
        else {
            byFlowCR &= ~FLOWCR1_HDXFCEN;
            CSR_WRITE_1(hw, byFlowCR, MAC_REG_FLOWCR1);
        }

    } /* (Rev > VT6102A) */

    /* if connection type is AUTO */
    if (option->spd_dpx == SPD_DPX_AUTO) {

        wANAR |= (ANAR_TXFD|ANAR_TX|ANAR_10FD|ANAR_10);
        
        if(wANAR != wOrigANAR){
            wANAR |= wANARMask;
            rhine_mii_write(hw, MII_REG_ANAR, wANAR);
            /* enable AUTO-NEGO mode */
            mii_set_auto_on(hw);
            status = RHINE_LINK_CHANGE;
        }

        /* set duplex mode of MAC according to duplex mode of MII */
        
        if (rhine_is_full_duplex(hw)) {
            rhine_set_duplex(hw, TRUE);
        }
        else {
            rhine_set_duplex(hw, FALSE);
        }
        

    }
    else if (byRevId < REV_ID_VT6102_A) {
        U16 wBMCR;

        /* if not Auto, then For VT3043
        // 1). turn off AUTO-NEGO
        // 2). set USER-FORCED speed (10/100) & duplex (half/full) mode of MII and MAC */

        /* turn off AUTO-NEGO */
        mii_set_auto_off(hw);

        /* set loopback in MII to un-link in 100M mode,
        // in 10M mode set this bit cannot make it un-link 
        // but it doesn't matter*/
        MII_REG_BITS_ON(BMCR_LBK, MII_REG_BMCR, hw);

        /* read the original value of BMCR register */
        rhine_mii_read(hw,MII_REG_BMCR, &wBMCR);

        /* mask off AUTO-NEGO */
        wBMCR &= ~(BMCR_AUTO);

        if (option->spd_dpx == SPD_DPX_100_HALF ||
            option->spd_dpx == SPD_DPX_10_HALF) {
            wBMCR &=(~BMCR_FDX);
            rhine_set_duplex(hw, FALSE);
        }else {
            wBMCR |=BMCR_FDX;
            rhine_set_duplex(hw, TRUE);
        }
        
        if (option->spd_dpx == SPD_DPX_100_HALF ||
            option->spd_dpx == SPD_DPX_100_FULL) {
            wBMCR |= BMCR_SPEED;
        }else {
            wBMCR &= ~(BMCR_SPEED);
        }
        
        /* write the setting to BMCR register */
        rhine_mii_write(hw,  MII_REG_BMCR, wBMCR);

        /* delay to avoid link down from force-10M to force-100M */
        mdelay(200);

        /* unset MII loopback to re-link */
        MII_REG_BITS_OFF(BMCR_LBK, MII_REG_BMCR, hw);
        status = RHINE_LINK_CHANGE;
    }
    else {
        U16     wBMCR;


        /* read the original value of BMCR register */
        rhine_mii_read(hw, MII_REG_BMCR, &wBMCR);

        switch(option->spd_dpx)
        {
            case SPD_DPX_100_FULL:
                wBMCR |= (BMCR_SPEED|BMCR_FDX);
                wANAR |= ANAR_TXFD;
                rhine_set_duplex(hw, TRUE);
                break;
            case SPD_DPX_100_HALF:
                wBMCR |= BMCR_SPEED;
                wANAR |= ANAR_TX;
                rhine_set_duplex(hw, FALSE);
                break;
            case SPD_DPX_10_FULL:
                wBMCR |= BMCR_FDX;
                wANAR |= ANAR_10FD;
                rhine_set_duplex(hw, TRUE);
                break;
            case SPD_DPX_10_HALF:
                wANAR |= ANAR_10;
                rhine_set_duplex(hw, FALSE);
                break;
            default:
                break;
        }

        /* write the setting to BMCR register */
        rhine_mii_write(hw, MII_REG_BMCR, wBMCR);

        if(wANAR != wOrigANAR){
            wANAR |= wANARMask;
            rhine_mii_write(hw, MII_REG_ANAR, wANAR);
            MII_REG_BITS_ON((BMCR_AUTO | BMCR_REAUTO), MII_REG_BMCR, hw);
            status = RHINE_LINK_CHANGE;
        }
    }
    
    return status;
}

void rhine_print_link_status(U32 status) {

    if (status & RHINE_LINK_FAIL) {
        RHINE_HW_PRT("failed to detect cable link\n");
    }
    else {
        if (status & RHINE_AUTONEG_ENABLE)
            RHINE_HW_PRT("Link autonegation");
        else
            RHINE_HW_PRT("Link forced");

        if (status & RHINE_SPEED_100)
            RHINE_HW_PRT(" speed 100M bps");
        else
            RHINE_HW_PRT(" speed 10M bps");

        if (status & RHINE_DUPLEX_FULL)
            RHINE_HW_PRT(" full duplex\n");
        else
            RHINE_HW_PRT(" half duplex\n");
    }
}

void rhine_set_promiscuous(struct rhine_hw* hw)
{
    CSR_WRITE_4(hw, 0xffffffff, MAC_REG_MAR);
    CSR_WRITE_4(hw, 0xffffffff, MAC_REG_MAR+4);
    BYTE_REG_BITS_ON(hw, RCR_AM|RCR_AB|RCR_PROM, MAC_REG_RCR);
}

void rhine_set_all_multicast(struct rhine_hw* hw)
{
    CSR_WRITE_4(hw, 0xffffffff, MAC_REG_MAR);
    CSR_WRITE_4(hw, 0xffffffff, MAC_REG_MAR+4);
    BYTE_REG_BITS_ON(hw, RCR_AM|RCR_AB, MAC_REG_RCR);
}




BOOL rhine_td_own_bit_on(PTX_DESC pTD)
{
    return (pTD->tdesc0 & cpu_to_le32(TSR_OWN));
}


void rhine_set_td_own(PTX_DESC pTD)
{
    pTD->tdesc0 |= cpu_to_le32(TSR_OWN);
}

void rhine_set_tx_buf_sz(PTX_DESC pTD, U16 size)
{
    pTD->tdesc1 &= cpu_to_le32(~FET_TXCTL_BUFLEN);
    pTD->tdesc1 |= cpu_to_le32(size);
}

U16 rhine_get_tx_buf_sz(PTX_DESC pTD)
{
    U16 size;
    U32 desc1;
    desc1 = cpu_to_le32(pTD->tdesc1);
    desc1 &= FET_TXCTL_BUFLEN;
    size = (U16)desc1;
    return size;
}


/* RD operation helper function. */
BOOL rhine_rd_own_bit_on( PRX_DESC pRD )
{
    return (pRD->rdesc0 & cpu_to_le32(RSR_OWN));
}
void rhine_set_rd_own( PRX_DESC pRD)
{
    pRD->rdesc0 |= cpu_to_le32(RSR_OWN);
}

U16 rhine_get_rx_frame_length( PRX_DESC pRD)
{
    U16 length;
    length = ((cpu_to_le32(pRD->rdesc0) & FET_RXSTAT_RXLEN) >> 16);
    return length;
}

void rhine_set_rx_buf_sz(PRX_DESC pRD, U16 size)
{
    pRD->rdesc1 &= cpu_to_le32(~FET_RXCTL_BUFLEN);
    pRD->rdesc1 |= cpu_to_le32(size);
}
