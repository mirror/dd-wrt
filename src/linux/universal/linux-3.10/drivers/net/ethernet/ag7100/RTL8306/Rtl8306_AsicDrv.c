    /*
    * Copyright c                  Realtek Semiconductor Corporation, 2005 
    * All rights reserved.
    * 
    * Program : RTL8306 switch low-level API
    * Abstract : 
    * Author : Robin Zheng-bei Xing(robin_xing@realsil.com.cn)                
    *  $Id: Rtl8306_AsicDrv.c,v 1.2 2007/08/03 03:59:52 michael Exp $
    */


#include "Rtl8306_types.h"
#include "mdcmdio.h"
#include "Rtl8306_AsicDrv.h"
#include "Rtl8306_Driver_s.h"


#ifdef RTL8306_TBLBAK
    extern rtl8306_ConfigBakPara_t rtl8306_TblBak;
#endif 

#ifdef RTL8306_TBLDRV_TEST
    /*Virtual PHY Registers in cpu memory, only for test*/
uint32	VirtualPhyReg[RTL8306_REG_NUMBER]; 	     
#endif

#ifndef RTL8306_TBLDRV_TEST
int32 rtl8306_setAsicPhyReg(uint32 phyad, uint32 regad, uint32 npage, uint32 value) {
	uint32 rdata; 

	if ((phyad >= RTL8306_PHY_NUMBER) || (regad >= RTL8306_REGSPERPAGE) ||
		(npage >= RTL8306_PAGE_NUMBER))	
		return FAILED;
	/* Select PHY Register Page through configuring PHY 0 Register 16 [bit1 bit15] */
	value = value & 0xFFFF;
	smiRead(0, 16, &rdata); 
	switch (npage) {
	case RTL8306_REGPAGE0:
		smiWrite(0, 16, (rdata & 0x7FFF) | 0x0002);
		break;
	case RTL8306_REGPAGE1:
		smiWrite(0, 16, rdata | 0x8002 );
		break;
	case RTL8306_REGPAGE2:
		smiWrite(0, 16, rdata & 0x7FFD);
		break;
	case RTL8306_REGPAGE3:
		smiWrite(0, 16, (rdata & 0xFFFD) | 0x8000);
		break;
	default:
		return FAILED;
	}
	
	smiWrite(phyad, regad, value);
	return SUCCESS;
}


int32 rtl8306_getAsicPhyReg(uint32 phyad, uint32 regad, uint32 npage, uint32 *pvalue) {
	uint32 rdata;

	if ((phyad >= RTL8306_PHY_NUMBER) || (regad >= RTL8306_REGSPERPAGE) ||
		(npage >= RTL8306_PAGE_NUMBER))	
		return FAILED;

	/* Select PHY Register Page through configuring PHY 0 Register 16 [bit1 bit15] */
	smiRead(0, 16, &rdata); 
	switch (npage) {
	case RTL8306_REGPAGE0:
		smiWrite(0, 16, (rdata & 0x7FFF) | 0x0002);
		break;
	case RTL8306_REGPAGE1:
		smiWrite(0, 16, rdata | 0x8002 );
		break;
	case RTL8306_REGPAGE2:
		smiWrite(0, 16, rdata & 0x7FFD);
		break;
	case RTL8306_REGPAGE3:
		smiWrite(0, 16, (rdata & 0xFFFD) | 0x8000);
		break;
	default:
		return FAILED;
	}

	smiRead(phyad, regad, pvalue);
	*pvalue = *pvalue & 0xFFFF;
	return SUCCESS;
	
}

#else /* vitual PHY register access*/

int32 rtl8306_setAsicPhyReg(uint32 phyad, uint32 regad, uint32 npage, uint32 value) {

	if ((phyad >= RTL8306_PHY_NUMBER) || (regad >= RTL8306_REGSPERPAGE) ||
		(npage >= RTL8306_PAGE_NUMBER))	
		return FAILED;
	value = value & 0xFFFF;
	switch (npage) {
	case RTL8306_REGPAGE0:
		VirtualPhyReg[phyad * RTL8306_REGSPERPHY + regad] = value;
		break;
	case RTL8306_REGPAGE1:
		if (regad < 26)
			VirtualPhyReg[phyad * RTL8306_REGSPERPHY + regad ] = value;
		else 
			VirtualPhyReg[phyad * RTL8306_REGSPERPHY + 32 + (regad - 26)] = value;
		break;
	case RTL8306_REGPAGE2:
		if (regad < 17)
			VirtualPhyReg[phyad * RTL8306_REGSPERPHY + regad] = value;
		else
			VirtualPhyReg[phyad * RTL8306_REGSPERPHY + 38 + (regad - 17)] = value;
		break;
	case RTL8306_REGPAGE3:
		if (regad < 17)
			VirtualPhyReg[phyad * RTL8306_REGSPERPHY + regad] = value;
		else 
			VirtualPhyReg[phyad * RTL8306_REGSPERPHY + 53+ (regad - 17)] = value;
		break;
	default:
		return FAILED;	
	}	
	
	return	SUCCESS; 
}

int32 rtl8306_getAsicPhyReg(uint32 phyad, uint32 regad, uint32 npage, uint32 *pvalue) {

	if ((phyad >= RTL8306_PHY_NUMBER) || (regad >= RTL8306_REGSPERPAGE) ||
		(npage >= RTL8306_PAGE_NUMBER) || (pvalue == NULL))
		return FAILED;
	switch (npage) {
	case RTL8306_REGPAGE0:
		*pvalue = VirtualPhyReg[phyad * RTL8306_REGSPERPHY + regad];
		break;
	case RTL8306_REGPAGE1:
		if (regad < 26)
			*pvalue = VirtualPhyReg[phyad * RTL8306_REGSPERPHY + regad ] ;
		else 
			*pvalue = VirtualPhyReg[phyad * RTL8306_REGSPERPHY + 32 + (regad - 26)] ;
		break;
	case RTL8306_REGPAGE2:
		if (regad < 17)
			*pvalue = VirtualPhyReg[phyad * RTL8306_REGSPERPHY + regad];
		else
			*pvalue = VirtualPhyReg[phyad * RTL8306_REGSPERPHY + 38 + (regad - 17)];
		break;
	case RTL8306_REGPAGE3:
		if (regad < 17)
			*pvalue = VirtualPhyReg[phyad * RTL8306_REGSPERPHY + regad];
		else  
			*pvalue = VirtualPhyReg[phyad * RTL8306_REGSPERPHY + 53 + (regad - 17)];
		break;
	default:
		return FAILED;
	}
	*pvalue = *pvalue & 0xFFFF;
	return	SUCCESS; 
		
}

#endif /*end ifndef RTL8306_TBLDRV_TEST */




int32 rtl8306_setAsicPhyRegBit(uint32 phyad, uint32 regad, uint32 bit, uint32 npage,  uint32 value) {
	uint32 rdata;
	if ((phyad >= RTL8306_PHY_NUMBER) || (regad >= RTL8306_REGSPERPAGE) || 
		(npage >= RTL8306_PAGE_NUMBER) || (bit > 15) || (value >1))
		return FAILED;
	rtl8306_getAsicPhyReg(phyad, regad,  npage, &rdata);
	if (value) 
		rtl8306_setAsicPhyReg(phyad, regad, npage, rdata | (1 << bit));
	else
		rtl8306_setAsicPhyReg(phyad, regad, npage, rdata & (~(1 << bit)));
	return SUCCESS;
}


int32 rtl8306_getAsicPhyRegBit(uint32 phyad, uint32 regad, uint32 bit, uint32 npage,  uint32 * pvalue) {
	uint32 rdata;

	if ((phyad >= RTL8306_PHY_NUMBER) || (regad >= RTL8306_REGSPERPAGE) ||
		(npage >= RTL8306_PAGE_NUMBER) || (bit > 15) || (pvalue == NULL))
		return FAILED;	
	rtl8306_getAsicPhyReg(phyad, regad, npage, &rdata);
	if (rdata & (1 << bit))
		*pvalue =1;
	else 
		*pvalue =0;
		
	return SUCCESS;
}


int32 rtl8306_setAsicEthernetPHY(uint32 phy, uint32 autoNegotiation, uint32 advCapability, uint32 speed, uint32 fullDuplex) {
	uint32 ctrlReg;

	if(phy >= RTL8306_PHY_NUMBER || 
		advCapability < RTL8306_ETHER_AUTO_100FULL 
		|| advCapability > RTL8306_ETHER_AUTO_10HALF 
		||(speed != 100 && speed != 10))
		return FAILED;

	if(advCapability == RTL8306_ETHER_AUTO_100FULL)		
		rtl8306_setAsicPhyReg(phy, 4, 0, RTL8306_CAPABLE_PAUSE | RTL8306_CAPABLE_100BASE_TX_FD 
								| RTL8306_CAPABLE_100BASE_TX_HD | RTL8306_CAPABLE_10BASE_TX_FD 
								| RTL8306_CAPABLE_10BASE_TX_HD | 0x1);
	else if(advCapability == RTL8306_ETHER_AUTO_100HALF)
		rtl8306_setAsicPhyReg(phy, 4, 0, RTL8306_CAPABLE_PAUSE | RTL8306_CAPABLE_100BASE_TX_HD
								| RTL8306_CAPABLE_10BASE_TX_FD | RTL8306_CAPABLE_10BASE_TX_HD | 0x1);
	else	if(advCapability == RTL8306_ETHER_AUTO_10FULL)
		rtl8306_setAsicPhyReg(phy, 4, 0, RTL8306_CAPABLE_PAUSE | RTL8306_CAPABLE_10BASE_TX_FD 
								| RTL8306_CAPABLE_10BASE_TX_HD | 0x1);
	else if(advCapability == RTL8306_ETHER_AUTO_10HALF)
		rtl8306_setAsicPhyReg(phy, 4, 0, RTL8306_CAPABLE_PAUSE | RTL8306_CAPABLE_10BASE_TX_HD | 0x1);
	
	/* Each time the link ability of the RTL8306 is reconfigured, 
		the auto-negotiation process should be executed to allow the configuration to take effect. */
	if(autoNegotiation == TRUE) 
		ctrlReg = RTL8306_ENABLE_AUTONEGO | RTL8306_RESTART_AUTONEGO; 
	else
		ctrlReg = 0;
	if(speed == 100) // 100Mbps, default assume 10Mbps
		ctrlReg |= RTL8306_SPEED_SELECT_100M;
	if(fullDuplex == TRUE)
		ctrlReg |= RTL8306_SELECT_FULL_DUPLEX;
	rtl8306_setAsicPhyReg(phy, 0, RTL8306_REGPAGE0, ctrlReg);
	
	return SUCCESS;
}

int32 rtl8306_getAsicEthernetPHY(uint32 phy, uint32 *autoNegotiation, uint32 *advCapability, uint32 *speed, uint32 *fullDuplex) {
	uint32 regData;
	
	if((phy >= RTL8306_PHY_NUMBER) || (autoNegotiation == NULL) || (advCapability == NULL)
		|| (speed == NULL) || (fullDuplex == NULL))
		return FAILED;
		
	rtl8306_getAsicPhyReg(phy, 0, RTL8306_REGPAGE0, &regData);
	*autoNegotiation = (regData & RTL8306_ENABLE_AUTONEGO)? TRUE: FALSE;
	*speed = (regData & RTL8306_SPEED_SELECT_100M)? 100: 10;
	*fullDuplex = (regData & RTL8306_SELECT_FULL_DUPLEX)? TRUE: FALSE;

	rtl8306_getAsicPhyReg(phy, 4, RTL8306_REGPAGE0, &regData);
	if(regData & RTL8306_CAPABLE_100BASE_TX_FD)
		*advCapability = RTL8306_ETHER_AUTO_100FULL;
	else if(regData & RTL8306_CAPABLE_100BASE_TX_HD)
		*advCapability = RTL8306_ETHER_AUTO_100HALF;
	else if(regData & RTL8306_CAPABLE_10BASE_TX_FD)
		*advCapability = RTL8306_ETHER_AUTO_10FULL;
	else if(regData & RTL8306_CAPABLE_10BASE_TX_HD)
		*advCapability = RTL8306_ETHER_AUTO_10HALF;
	
	return SUCCESS;
}

int32 rtl8306_setAsicPort5LinkStatus(uint32 enabled) {

      uint32 duplex, speed, nway;

     /*save phy 6 reg 0.13, 0.12, 0.8*/
      rtl8306_getAsicPhyRegBit(6, 0, 13, 0, &speed);
      rtl8306_getAsicPhyRegBit(6, 0, 12, 0, &nway);
      rtl8306_getAsicPhyRegBit(6, 0, 8, 0, &duplex);
      
	rtl8306_setAsicPhyRegBit(6, 22, 15, 0, enabled == TRUE ? 1:0);
	
     /*restore phy 6 reg 0.13, 0.12, 0.8*/
       rtl8306_setAsicPhyRegBit(6, 0, 13, 0, speed);
       rtl8306_setAsicPhyRegBit(6, 0, 12, 0, nway);
       rtl8306_setAsicPhyRegBit(6, 0, 8, 0, duplex);

       return SUCCESS;       
}

int32 rtl8306_getAsicPort5LinkStatus(uint32 *enabled) {
	uint32 bitValue;
	
	if (enabled == NULL) 
		return FAILED;
	rtl8306_getAsicPhyRegBit(6, 22, 15, 0, &bitValue);
	*enabled = (bitValue ==1 ? TRUE:FALSE);
	return SUCCESS;
}


#if 1
int32 rtl8306_setAsicTurboMIIEnable(uint32 enabled)
{
     uint32 regval;

      rtl8306_getVendorID(&regval);
      /*TMII is only for RTL8306SD/SDM*/      
      if (RTL8306S(regval))
        return FAILED;
      
      if (enabled)
      {
            /*set MAC5 link up*/
            rtl8306_setAsicPort5LinkStatus(TRUE);
      }      
      /*enable TMII*/
      rtl8306_setAsicPhyRegBit(0, 16, 14, 0, enabled ? 1:0);        
      return SUCCESS;
}

int32 rtl8306_getAsicPHYLinkStatus(uint32 phy, uint32 *linkUp) {
	uint32 bitValue;
	
	if (linkUp == NULL)
		return FAILED;
	rtl8306_getAsicPhyRegBit(phy, 1, 2, RTL8306_REGPAGE0, &bitValue);
	*linkUp = (bitValue == 1? TRUE: FALSE);

	return SUCCESS;
}

int32 rtl8306_getAsicPHYAutoNegotiationDone(uint32 phy, uint32 *done) {
	uint32 bitValue;

	if (done == NULL)
		return FAILED;
	rtl8306_getAsicPhyRegBit(phy, 1, 5, 0, &bitValue);
	*done = (bitValue == 1? TRUE: FALSE);
	return SUCCESS;
}

int32 rtl8306_setAsicPHYLoopback(uint32 phy, uint32 enabled) {

	rtl8306_setAsicPhyRegBit(phy, 0, 14, 0, enabled==TRUE?1:0);	
	return SUCCESS;
}

int32 rtl8306_getAsicPHYLoopback(uint32 phy, uint32 *enabled) {
	uint32 bitValue;

	if (enabled == NULL)
		return FAILED;
	rtl8306_getAsicPhyRegBit(phy, 0, 14, 0, &bitValue);
	*enabled = (bitValue == 1? TRUE: FALSE);
	return SUCCESS;
}



int32 rtl8306_setAsic25MClockOutput(uint32 enabled) {
    uint32 vendorID;
    
    rtl8306_getVendorID(&vendorID);
    if (RTL8306SD(vendorID) || RTL8306SDM(vendorID)) {
        rtl8306_setAsicPhyRegBit(2, 22,14, 3,enabled ==TRUE ? 0:1 );        
    }    
    return SUCCESS;
}

int32 rtl8306_setAsicVlanEnable(uint32 enabled) {
	

        rtl8306_setAsicPhyRegBit(0, 18, 8, 0, enabled==FALSE?1:0);

    #ifdef RTL8306_TBLBAK
        rtl8306_TblBak.vlanConfig.enVlan = (uint8) enabled; 
    #endif
    
	/*disable trunk*/
	rtl8306_setAsicPhyRegBit(0, 19, 11, 0, 1);
	return SUCCESS;
}

int32 rtl8306_getAsicVlanEnable(uint32 *enabled) {
	uint32 bitValue;	

	if (enabled == NULL)
		return FAILED;
	rtl8306_getAsicPhyRegBit(0, 18, 8, 0, &bitValue);
	*enabled = (bitValue == 0 ? TRUE: FALSE);		
	return SUCCESS;
}

int32 rtl8306_setAsicVlanTagAware(uint32 enabled) {
	rtl8306_setAsicPhyRegBit(0, 16, 10, 0, enabled==FALSE?1:0 );
    #ifdef RTL8306_TBLBAK
        rtl8306_TblBak.vlanConfig.enTagAware = (uint8) enabled;
    #endif
    
	return SUCCESS;
}

int32 rtl8306_getAsicVlanTagAware(uint32 *enabled) {
	uint32 bitValue;

	if (enabled == NULL)
		return FAILED;
	rtl8306_getAsicPhyRegBit(0, 16, 10, 0, &bitValue);
	*enabled = (bitValue == 0? TRUE: FALSE);
	return SUCCESS;
}

int32 rtl8306_setAsicVlanIngressFilter(uint32 enabled) {
	rtl8306_setAsicPhyRegBit(0, 16, 9, 0, enabled==FALSE?1:0);

    #ifdef RTL8306_TBLBAK
        rtl8306_TblBak.vlanConfig.enIngress = (uint8) enabled;
    #endif
    
	return SUCCESS;
}

int32 rtl8306_getAsicVlanIngressFilter(uint32 *enabled) {
	uint32 bitValue;

	if (enabled == NULL)
		return FAILED;	
	rtl8306_getAsicPhyRegBit(0, 16, 9, 0, &bitValue);
	*enabled = (bitValue == 0? TRUE: FALSE);		
	return SUCCESS;
}

int32 rtl8306_setAsicVlanTaggedOnly(uint32 enabled) {
	rtl8306_setAsicPhyRegBit(0, 16, 8, 0, enabled==FALSE?1:0 );
    #ifdef RTL8306_TBLBAK
      rtl8306_TblBak.vlanConfig.enVlanTagOnly = (uint8) enabled;
    #endif
    
	return SUCCESS;
}

int32 rtl8306_getAsicVlanTaggedOnly(uint32 *enabled) {
	uint32 bitValue;

	if(enabled==NULL)
		return FAILED;
	rtl8306_getAsicPhyRegBit(0, 16, 8, 0, &bitValue);
	*enabled = (bitValue == 0? TRUE: FALSE);		
	return SUCCESS;
}


int32 rtl8306_setAsicVlan(uint32 vlanIndex, uint32 vid, uint32 memberPortMask) {
	uint32 regValue;

	switch(vlanIndex) {
	case 0:/* VLAN[A] */
		rtl8306_getAsicPhyReg(0, 25, 0, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		rtl8306_setAsicPhyReg(0, 25, 0, regValue);
		rtl8306_getAsicPhyReg(0, 24, 0, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		rtl8306_setAsicPhyReg(0, 24, 0, regValue);
		break;
		
	case 1:/* VLAN[B] */
		rtl8306_getAsicPhyReg(1, 25, 0, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		rtl8306_setAsicPhyReg(1, 25, 0, regValue);
		rtl8306_getAsicPhyReg(1, 24, 0, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		rtl8306_setAsicPhyReg(1, 24, 0, regValue);
		break;
		
	case 2:/* VLAN[C] */
		rtl8306_getAsicPhyReg(2, 25, 0, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		rtl8306_setAsicPhyReg(2, 25, 0, regValue);
		rtl8306_getAsicPhyReg(2, 24, 0, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		rtl8306_setAsicPhyReg(2, 24, 0, regValue);
		break;
		
	case 3:/* VLAN[D] */
		rtl8306_getAsicPhyReg(3, 25, 0, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		rtl8306_setAsicPhyReg(3, 25, 0, regValue);
		rtl8306_getAsicPhyReg(3, 24, 0, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		rtl8306_setAsicPhyReg(3, 24, 0, regValue);
		break;
		
	case 4:/* VLAN[E] */
		rtl8306_getAsicPhyReg(4, 25, 0, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		rtl8306_setAsicPhyReg(4, 25, 0, regValue);
		rtl8306_getAsicPhyReg(4, 24, 0, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		rtl8306_setAsicPhyReg(4, 24, 0, regValue);
		break;
		
	case 5:/* VLAN[F] */
		rtl8306_getAsicPhyReg(0, 27, 1, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		rtl8306_setAsicPhyReg(0, 27, 1, regValue);
		rtl8306_getAsicPhyReg(0, 26, 1, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		rtl8306_setAsicPhyReg(0, 26, 1, regValue);
		break;
		
	case 6:/* VLAN[G] */
		rtl8306_getAsicPhyReg(1, 27, 1, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		rtl8306_setAsicPhyReg(1, 27, 1, regValue);
		rtl8306_getAsicPhyReg(1, 26, 1, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		rtl8306_setAsicPhyReg(1, 26, 1, regValue);
		break;
		
	case 7:/* VLAN[H] */
		rtl8306_getAsicPhyReg(2, 27, 1, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		rtl8306_setAsicPhyReg(2, 27, 1, regValue);
		rtl8306_getAsicPhyReg(2, 26, 1, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		rtl8306_setAsicPhyReg(2, 26, 1, regValue);
		break;
		
	case 8:/* VLAN[I] */
		rtl8306_getAsicPhyReg(3, 27, 1, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		rtl8306_setAsicPhyReg(3, 27, 1, regValue);
		rtl8306_getAsicPhyReg(3, 26, 1, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		rtl8306_setAsicPhyReg(3, 26, 1, regValue);
		break;
		
	case 9:/* VLAN[J] */
		rtl8306_getAsicPhyReg(4, 27, 1, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		rtl8306_setAsicPhyReg(4, 27, 1, regValue);
		rtl8306_getAsicPhyReg(4, 26, 1, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		rtl8306_setAsicPhyReg(4, 26, 1, regValue);
		break;
	
	case 10:/* VLAN[K] */
		rtl8306_getAsicPhyReg(0, 29, 1, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		rtl8306_setAsicPhyReg(0, 29, 1, regValue);
		rtl8306_getAsicPhyReg(0, 28, 1, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		rtl8306_setAsicPhyReg(0, 28, 1, regValue);
		break;
		
	case 11:/* VLAN[L] */
		rtl8306_getAsicPhyReg(1, 29, 1, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		rtl8306_setAsicPhyReg(1, 29, 1, regValue);
		rtl8306_getAsicPhyReg(1, 28, 1, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		rtl8306_setAsicPhyReg(1, 28, 1, regValue);
		break;
		
	case 12:/* VLAN[M] */
		rtl8306_getAsicPhyReg(2, 29, 1, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		rtl8306_setAsicPhyReg(2, 29, 1, regValue);
		rtl8306_getAsicPhyReg(2, 28, 1, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		rtl8306_setAsicPhyReg(2, 28, 1, regValue);
		break;
		
	case 13:/* VLAN[N] */
		rtl8306_getAsicPhyReg(3, 29, 1, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		rtl8306_setAsicPhyReg(3, 29, 1, regValue);
		rtl8306_getAsicPhyReg(3, 28, 1, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		rtl8306_setAsicPhyReg(3, 28, 1, regValue);
		break;

	case 14:/* VLAN[O] */
		rtl8306_getAsicPhyReg(4, 29, 1, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		rtl8306_setAsicPhyReg(4, 29, 1, regValue);
		rtl8306_getAsicPhyReg(4, 28, 1, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		rtl8306_setAsicPhyReg(4, 28, 1, regValue);
		break;
		
	case 15:/* VLAN[P] */
		rtl8306_getAsicPhyReg(0, 31, 1, &regValue);
		regValue = (regValue & 0xF000) | (vid & 0xFFF);
		rtl8306_setAsicPhyReg(0, 31, 1, regValue);
		rtl8306_getAsicPhyReg(0, 30, 1, &regValue);
		regValue = (regValue & 0xFFC0) | (memberPortMask & 0x3F);
		rtl8306_setAsicPhyReg(0, 30, 1, regValue);
		break;

	default:
		return FAILED;
		
	}

#ifdef RTL8306_TBLBAK
       rtl8306_TblBak.vlanTable[vlanIndex].vid = (uint16) vid;
       rtl8306_TblBak.vlanTable[vlanIndex].memberPortMask = (uint8) memberPortMask;
#endif

	return	SUCCESS;
}

int32 rtl8306_getAsicVlan(uint32 vlanIndex, uint32 *vid, uint32 *memberPortMask) {

	if(vid == NULL || memberPortMask == NULL)
		return FAILED;
	switch(vlanIndex) {
	case 0: /*VLAN[A]*/
		rtl8306_getAsicPhyReg(0, 25, 0, vid);
		rtl8306_getAsicPhyReg(0, 24, 0, memberPortMask);
		break;
	case 1: /*VLAN[B]*/
		rtl8306_getAsicPhyReg(1, 25, 0, vid);
		rtl8306_getAsicPhyReg(1, 24, 0, memberPortMask);		
		break;
	case 2: /*VLAN[C]*/
		rtl8306_getAsicPhyReg(2, 25, 0, vid);
		rtl8306_getAsicPhyReg(2, 24, 0, memberPortMask);		
		break;
	case 3: /*VLAN[D]*/
		rtl8306_getAsicPhyReg(3, 25, 0, vid);
		rtl8306_getAsicPhyReg(3, 24, 0, memberPortMask);		
		break;		
	case 4: /*VLAN[E]*/
		rtl8306_getAsicPhyReg(4, 25, 0, vid);
		rtl8306_getAsicPhyReg(4, 24, 0, memberPortMask);		
		break;
	case 5: /*VLAN[F]*/
		rtl8306_getAsicPhyReg(0, 27, 1, vid);
		rtl8306_getAsicPhyReg(0, 26, 1, memberPortMask);		
		break;
	case 6: /*VLAN[G]*/
		rtl8306_getAsicPhyReg(1, 27, 1, vid);
		rtl8306_getAsicPhyReg(1, 26, 1, memberPortMask);		
		break;
	case 7: /*VLAN[H]*/
		rtl8306_getAsicPhyReg(2, 27, 1, vid);
		rtl8306_getAsicPhyReg(2, 26, 1, memberPortMask);		
		break;		
	case 8: /*VLAN[I]*/
		rtl8306_getAsicPhyReg(3, 27, 1, vid);
		rtl8306_getAsicPhyReg(3, 26, 1, memberPortMask);		
		break;
	case 9: /*VLAN[J]*/
		rtl8306_getAsicPhyReg(4, 27, 1, vid);
		rtl8306_getAsicPhyReg(4, 26, 1, memberPortMask);		
		break;
	case 10: /*VLAN[K]*/
		rtl8306_getAsicPhyReg(0, 29, 1, vid);
		rtl8306_getAsicPhyReg(0, 28, 1, memberPortMask);		
		break;
	case 11: /*VLAN[L]*/
		rtl8306_getAsicPhyReg(1, 29, 1, vid);
		rtl8306_getAsicPhyReg(1, 28, 1, memberPortMask);		
		break;
	case 12: /*VLAN[M]*/
		rtl8306_getAsicPhyReg(2, 29, 1, vid);
		rtl8306_getAsicPhyReg(2, 28, 1, memberPortMask);		
		break;
	case 13: /*VLAN[N]*/
		rtl8306_getAsicPhyReg(3, 29, 1, vid);
		rtl8306_getAsicPhyReg(3, 28, 1, memberPortMask);		
		break;
	case 14: /*VLAN[O]*/
		rtl8306_getAsicPhyReg(4, 29, 1, vid);
		rtl8306_getAsicPhyReg(4, 28, 1, memberPortMask);		
		break;
	case 15: /*VLAN[P]*/
		rtl8306_getAsicPhyReg(0, 31, 1, vid);
		rtl8306_getAsicPhyReg(0, 30, 1, memberPortMask);		
		break;		
	default:
		return 	FAILED;
	}	
	*vid = (*vid) & 0xFFF;
	*memberPortMask = (*memberPortMask) & 0x3F;
	return	SUCCESS;
		
}	

int32 rtl8306_setAsicPortVlanIndex(uint32 port, uint32 vlanIndex) {
	uint32 regValue;

	if(port > RTL8306_PORT5)
		return FAILED;
	if (port < RTL8306_PORT5) {
		rtl8306_getAsicPhyReg(port, 24, 0, &regValue);
		regValue = (regValue & 0xFFF) | (vlanIndex<<12);
		rtl8306_setAsicPhyReg(port, 24, 0, regValue);
	} else {
		rtl8306_getAsicPhyReg(0, 26, 1, &regValue);	
		regValue = (regValue & 0xFFF) | (vlanIndex<<12);		
		rtl8306_setAsicPhyReg(0, 26, 1, regValue);		
	}	

       #ifdef RTL8306_TBLBAK
            rtl8306_TblBak.vlanPvidIdx[port]= (uint8) vlanIndex;
       #endif
       
	return SUCCESS;
}


int32 rtl8306_getAsicPortVlanIndex(uint32 port, uint32 *vlanIndex) {
	if((port > RTL8306_PORT5) || vlanIndex == NULL)
		return FAILED;
	if (port < RTL8306_PORT5) 
		rtl8306_getAsicPhyReg(port, 24, 0, vlanIndex);
	else 
		rtl8306_getAsicPhyReg(0, 26, 1, vlanIndex);
	*vlanIndex = (*vlanIndex>>12) & 0xF;
	return SUCCESS;
}

int32 rtl8306_setAsicLeakyVlan(uint32 enabled) {

	rtl8306_setAsicPhyRegBit(0, 18, 11, 0, enabled==FALSE?1:0);	
    #ifdef RTL8306_TBLBAK
       rtl8306_TblBak.vlanConfig.enLeakVlan = (uint8) enabled;
    #endif

	return SUCCESS;
}


int32 rtl8306_getAsicLeakyVlan(uint32 *enabled) {
	uint32 bitValue;

	if(enabled==NULL)
		return FAILED;
	rtl8306_getAsicPhyRegBit(0, 18, 11, 0, &bitValue);
	*enabled = (bitValue == 0? TRUE: FALSE);
		
	return SUCCESS;
}


int32 rtl8306_setAsicArpVlan(uint32 enabled) {

	rtl8306_setAsicPhyRegBit(0, 18, 10, 0, enabled==FALSE?1:0);

    #ifdef RTL8306_TBLBAK
        rtl8306_TblBak.vlanConfig.enArpVlan = (uint8) enabled;
    #endif
	return SUCCESS;
}

int32 rtl8306_getAsicArpVlan(uint32 *enabled) {
	uint32 bitValue;

	if(enabled==NULL)
		return FAILED;
	rtl8306_getAsicPhyRegBit(0, 18, 10, 0, &bitValue);
	*enabled = (bitValue == 0? TRUE: FALSE);		
	return SUCCESS;
}


int32 rtl8306_setAsicMulticastVlan(uint32 enabled) {
	rtl8306_setAsicPhyRegBit(2, 23, 7, 3, enabled == FALSE ? 1:0);
    #ifdef RTL8306_TBLBAK
        rtl8306_TblBak.vlanConfig.enIPMleaky = (uint8) enabled;
    #endif
    
	return SUCCESS;
}


int32 rtl8306_getAsicMulticastVlan(uint32 * enabled) {
	uint32 bitValue;
	
	if (enabled == NULL) 
		return FAILED;
	rtl8306_getAsicPhyRegBit(2, 23, 7, 3, &bitValue);
	*enabled = (bitValue== 0 ? TRUE:FALSE);
	return SUCCESS;
}

int32 rtl8306_setAsicMirrorVlan(uint32 enabled) {
	rtl8306_setAsicPhyRegBit(2, 23, 6, 3, enabled == FALSE ? 1:0);
    #ifdef RTL8306_TBLBAK
        rtl8306_TblBak.vlanConfig.enMirLeaky = (uint8) enabled;
    #endif
    
	return SUCCESS;
}

int32 rtl8306_getAsicMirrorVlan(uint32 * enabled) {
	uint32 bitValue;
	
	if (enabled == NULL) 
		return FAILED;
	rtl8306_getAsicPhyRegBit(2, 23, 6, 3, &bitValue);
	*enabled = (bitValue== 0 ? TRUE:FALSE);
	return SUCCESS;
}


int32 rtl8306_setAsicNullVidReplaceVlan(uint32 port, uint32 enabled) {
      uint32 speed, duplex, nway;

	if (port > RTL8306_PORT5)
		return FAILED;
       /*save mac 4 or port status when operate reg.22*/
       if (port == 4) {
            rtl8306_getAsicPhyRegBit(5, 0, 13, 0, &speed);
            rtl8306_getAsicPhyRegBit(5, 0, 12, 0, &nway);
            rtl8306_getAsicPhyRegBit(5, 0, 8, 0, &duplex);            
       } else if (port == 5) {
            rtl8306_getAsicPhyRegBit(6, 0, 13, 0, &speed);
            rtl8306_getAsicPhyRegBit(6, 0, 12, 0, &nway);
            rtl8306_getAsicPhyRegBit(6, 0, 8, 0, &duplex);            
       }

	/*Port 5 corresponding PHY6*/
	if (port == RTL8306_PORT5 )
		port ++ ; 
	rtl8306_setAsicPhyRegBit(port, 22, 12, 0, enabled == TRUE ? 1:0);

       /*restore mac 4 or port status when operate reg.22*/
      if (port == 4) {
            rtl8306_setAsicPhyRegBit(5, 0, 13, 0, speed);
            rtl8306_setAsicPhyRegBit(5, 0, 12, 0, nway);
            rtl8306_setAsicPhyRegBit(5, 0, 8, 0, duplex);
            
      } else if (port == 6)  {  /*for port++ when port 5*/
           rtl8306_setAsicPhyRegBit(6, 0, 13, 0, speed);
           rtl8306_setAsicPhyRegBit(6, 0, 12, 0, nway);
           rtl8306_setAsicPhyRegBit(6, 0, 8, 0, duplex);
      }      

       #ifdef RTL8306_TBLBAK
            rtl8306_TblBak.vlanConfig_perport[port].enNulPvidRep = (uint8) enabled;
       #endif
	return SUCCESS;
}


int32 rtl8306_getAsicNullVidReplaceVlan(uint32 port, uint32* enabled) {
	uint32 bitValue;
	
	if ( (port > RTL8306_PORT5) || (enabled == NULL))
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  
	rtl8306_getAsicPhyRegBit(port, 22, 12, 0, &bitValue);
	*enabled = (bitValue == 1 ? TRUE:FALSE);	
	return SUCCESS;
}


int32 rtl8306_setAsicVlanTagInsertRemove(uint32 port, uint32 option) {
	uint32 regValue;
	 uint32 speed, duplex, nway;
     
	if ( (port > RTL8306_PORT5) || (option > 0x3) ) 
		return FAILED;
       /* 
	//when enable inserting CPU tag, VLAN tag could not be added 
	rtl8306_getAsicPhyReg(2, 21, 3, &regValue);
	if ((option == RTL8306_VLAN_ITAG) && ((regValue & 0x9000) == 0x1000))
		return FAILED;
        */
       /*save mac 4 or port status when operate reg.22*/
       if (port == 4) {
            rtl8306_getAsicPhyRegBit(5, 0, 13, 0, &speed);
            rtl8306_getAsicPhyRegBit(5, 0, 12, 0, &nway);
            rtl8306_getAsicPhyRegBit(5, 0, 8, 0, &duplex);            
       } else if (port == 5) {
            rtl8306_getAsicPhyRegBit(6, 0, 13, 0, &speed);
            rtl8306_getAsicPhyRegBit(6, 0, 12, 0, &nway);
            rtl8306_getAsicPhyRegBit(6, 0, 8, 0, &duplex);            
       }
    
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  
	rtl8306_getAsicPhyReg(port, 22, 0, &regValue);
	rtl8306_setAsicPhyReg(port, 22, 0, (regValue & 0xFFFC) | option);
       /*restore mac 4 or port status when operate reg.22*/
      if (port == 4) {
            rtl8306_setAsicPhyRegBit(5, 0, 13, 0, speed);
            rtl8306_setAsicPhyRegBit(5, 0, 12, 0, nway);
            rtl8306_setAsicPhyRegBit(5, 0, 8, 0, duplex);
            
      } else if (port == 6)  {  /*for port++ when port 5*/
           rtl8306_setAsicPhyRegBit(6, 0, 13, 0, speed);
           rtl8306_setAsicPhyRegBit(6, 0, 12, 0, nway);
           rtl8306_setAsicPhyRegBit(6, 0, 8, 0, duplex);
      }      

      #ifdef RTL8306_TBLBAK
            rtl8306_TblBak.vlanConfig_perport[port].vlantagInserRm = (uint8) option;
      #endif
	return SUCCESS;
}


int32 rtl8306_getAsicVlanTagInsertRemove(uint32 port, uint32 *option) {
	uint32 regValue;
	
	if (port > RTL8306_PORT5 || (option == NULL))
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  
	rtl8306_getAsicPhyReg(port, 22, 0, &regValue);	
	*option = regValue & 0x3;
       /*
	//When enable inserting CPU tag, VLAN tag could not be added 
	rtl8306_getAsicPhyReg(2, 21, 3, &regValue);
	if ((*option == RTL8306_VLAN_ITAG) && ((regValue & 0x9000) == 0x1000))
		*option = RTL8306_VLAN_UNDOTAG; 	
	*/
	return SUCCESS;
}


int32 rtl8306_setAsicVlanTrapToCPU(uint32 enabled) {

	if (enabled == FALSE) {
		rtl8306_setAsicPhyRegBit(2, 22, 6, 3, 1);		
	} else {
		/*Enable CPU function*/
		rtl8306_setAsicPhyRegBit(2, 21, 15, 3, 0);
		/*Enable inserting CPU Tag*/
		rtl8306_setAsicPhyRegBit(2, 21, 12, 3, 1);
		/*Enable removing CPU Tag*/
		rtl8306_setAsicPhyRegBit(2, 21, 11, 3, 1);
		/*Set DisVLANtrap2CPU*/
		rtl8306_setAsicPhyRegBit(2, 22, 6, 3, 0);
	}		
	return SUCCESS;
}

int32 rtl8306_getAsicVlanTrapToCPU(uint32 *enabled) {
	uint32 cpufun, bitValue;

	if (enabled == NULL)
		return FAILED;
	rtl8306_getAsicPhyRegBit(2, 21, 15, 3, &cpufun);
	rtl8306_getAsicPhyRegBit(2, 22, 6, 3, &bitValue);
	*enabled = (!cpufun) && (!bitValue );
	return SUCCESS;
}

int32 rtl8306_setAsic1pRemarkingVlan(uint32 port, uint32 enabled) {

	if (port > RTL8306_PORT5)
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  	
	rtl8306_setAsicPhyRegBit(port, 17, 0, 2, enabled== TRUE ? 1:0);
       #ifdef RTL8306_TBLBAK
           rtl8306_TblBak.vlanConfig_perport[port].en1PRemark= (uint8) enabled;
       #endif
    
	return SUCCESS;
}


int32 rtl8306_getAsic1pRemarkingVlan(uint32 port, uint32 *enabled) {
	uint32 bitValue;
	
	if (port > RTL8306_PORT5 || (enabled == NULL))
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  	
	rtl8306_getAsicPhyRegBit(port, 17, 0, 2, &bitValue);	
	*enabled = (bitValue == 1 ? TRUE:FALSE);
	return SUCCESS;

}

int32 rtl8306_setAsic1pRemarkingPriority(uint32 priority, uint32 priority1p) {
	uint32 regValue;
	
	if ( (priority > 3) || (priority1p > 0x7) ) 
		return FAILED;
	rtl8306_getAsicPhyReg(0, 24, 3, &regValue);
	switch(priority) {
	case 0:		
		regValue = (regValue & 0xFFF8) | priority1p;
		break;
	case 1:
		regValue = (regValue & 0xFFC7) | (priority1p << 3);
		break;
	case 2:
		regValue = (regValue & 0xFE3F) | (priority1p <<6);
		break;
	case 3:
		regValue = (regValue & 0xF1FF) | (priority1p <<9);
		break;
	default:
		return FAILED;
	}
	rtl8306_setAsicPhyReg(0, 24, 3, regValue);	
      #ifdef RTL8306_TBLBAK
        rtl8306_TblBak.dot1PremarkCtl[priority] = (uint8) priority1p;
      #endif
	return SUCCESS;
}


int32 rtl8306_getAsic1pRemarkingPriority(uint32 priority, uint32 *priority1p) {
	uint32 regValue;

	if ( (priority > 3) || (priority1p == NULL) ) 
		return FAILED;
	rtl8306_getAsicPhyReg(0, 24, 3, &regValue);
	switch(priority) {
	case 0:
		*priority1p = (regValue & 0x7);
		break;
	case 1:
		*priority1p = (regValue & 0x0038) >> 3;
		break;
	case 2:
		*priority1p = (regValue & 0x01C0) >> 6;
		break;
	case 3:
		*priority1p = (regValue & 0x0E00) >> 9;
		break;
	default:
		return FAILED;
	}
	
	return SUCCESS;
}


int32 rtl8306_setAsicCPUPort(uint32 port, uint32 enTag) {
	uint32 regValue;
	
	if (port > RTL8306_NOCPUPORT)
		return FAILED;

      if (port < RTL8306_PORT_NUMBER) {
            /*Enable CPU port Function */
        	rtl8306_setAsicPhyRegBit(2, 21, 15, 3, 0);	
        	/*Whether enable inserting CPU tag*/
        	rtl8306_setAsicPhyRegBit(2, 21, 12, 3, enTag == TRUE ? 1 : 0);
        	/*Enable the ability to check cpu tag*/
        	//rtl8306_setAsicPhyRegBit(4, 21, 7, 0, enTag == TRUE ? 1 : 0);
        	rtl8306_setAsicPhyRegBit(4, 21, 7, 0, 1);
        	/*Enable removing CPU tag*/
        	rtl8306_setAsicPhyRegBit(2, 21, 11, 3, 1);
        	rtl8306_getAsicPhyReg(4, 21, 0, &regValue);
        	regValue = (regValue & 0xFFF8) | port;
        	rtl8306_setAsicPhyReg(4, 21, 0, regValue);
        	/*Disable IEEE802.1x function of CPU Port*/	
        	if (port < RTL8306_PORT5) {
	        	rtl8306_setAsicPhyRegBit(port, 17, 9, 2, 0);
        		rtl8306_setAsicPhyRegBit(port, 17, 8, 2, 0);
        	} else {
	        	rtl8306_setAsicPhyRegBit(6, 17, 9, 2, 0);
        		rtl8306_setAsicPhyRegBit(6, 17, 8, 2, 0);
        	}
        	/*Port 5 should be enabled especially*/
        	if (port == RTL8306_PORT5)
	        	rtl8306_setAsicPhyRegBit(6, 22, 15, 0, TRUE);
        }
        else {
            /*Disable CPU port Function */
        	rtl8306_setAsicPhyRegBit(2, 21, 15, 3, 1);	
        	rtl8306_getAsicPhyReg(4, 21, 0, &regValue);
        	regValue = (regValue & 0xFFF8) | port;
        	rtl8306_setAsicPhyReg(4, 21, 0, regValue);
        }   
		
	return SUCCESS;	
}


int32 rtl8306_getAsicCPUPort(uint32 *port, uint32 *enTag) {
	uint32 regValue;
	uint32 bitValue, bitVal, cpufun;
		
	if ((port == NULL) || (enTag == NULL))
		return FAILED;
	rtl8306_getAsicPhyRegBit(2, 21, 12, 3, &bitValue);
	rtl8306_getAsicPhyRegBit(2, 21, 15, 3, &cpufun);
	rtl8306_getAsicPhyRegBit(4, 21, 7, 0, &bitVal);
	if ((!cpufun) && bitValue && bitVal)
		*enTag = TRUE;
	else 
		*enTag = FALSE;
	rtl8306_getAsicPhyReg(4, 21, 0, &regValue);
	*port = regValue & 0x7;
	return SUCCESS;
}



int32 rtl8306_setAsicWANPort(uint32 port, uint8 *ispmac, uint32 enISPmac) {
        uint32 regval;

        rtl8306_getAsicPhyReg(4, 21, 0, &regval);
        regval = (regval & 0xFF80) | ((port & 0x7) << 4);
        rtl8306_setAsicPhyRegBit(0, 16, 0, 0, enISPmac == TRUE ? 1:0);
        regval = (ispmac[1] << 8) | ispmac[0];
        rtl8306_setAsicPhyReg(3, 19, 0, regval);
        regval = (ispmac[3] << 8) | ispmac[2];
        rtl8306_setAsicPhyReg(3, 20, 0, regval);
        regval = (ispmac[5] << 8) | ispmac[4];
        rtl8306_setAsicPhyReg(3, 21, 0, regval);
        
        return SUCCESS;
}

int32 rtl8306_setAsicCPUTaggedPktCRCCheck(uint32 enabled) {

    
       rtl8306_setAsicPhyRegBit(4, 21, 3, 0, enabled == TRUE ? 0:1);
       return SUCCESS;    

}

int32 rtl8306_getAsicCPUTaggedPktCRCCheck(uint32 *enabled) {
        uint32 regValue;
        
        if (enabled == NULL)
            return FAILED;
        
        rtl8306_getAsicPhyRegBit(4, 21, 3, 0, &regValue);
        *enabled = (regValue == 0 ? TRUE : FALSE);        
        return SUCCESS;
}


int32 rtl8306_setAsicIGMPMLDSnooping(uint32 protocol, uint32 enabled) {
	uint32 regValue;

	rtl8306_getAsicPhyReg(2, 21, 3, &regValue);
	switch(protocol) {
	case RTL8306_IGMP:
		if (enabled == TRUE) 
			/*IGMP snooping, CPU function, insert tag, remove tag should all be enalbed*/
			rtl8306_setAsicPhyReg(2, 21, 3, (regValue & 0x7FFF) | 0x5800 );
		else
			rtl8306_setAsicPhyReg(2, 21, 3, regValue & 0xBFFF);		
		break;
	case RTL8306_MLD:
		if (enabled == TRUE) 
			/*MLD snooping, CPU function, insert tag, remove tag should all be enalbed*/
			rtl8306_setAsicPhyReg(2, 21, 3, (regValue & 0x7FFF) | 0x3800 );
		else
			rtl8306_setAsicPhyReg(2, 21, 3, regValue & 0xDFFF);				
		break;
	default:
		return	FAILED;
	}
	return SUCCESS;
}


int32 rtl8306_getAsicIGMPMLDSnooping(uint32 protocol, uint32 *enabled) {
	uint32 regValue;
	uint32 igmp,mld, cputag, cpufun;

	if (enabled == NULL) 
		return FAILED;
	rtl8306_getAsicPhyReg(2, 21, 3, &regValue);
	cpufun = (regValue & 0x8FFF) >> 15;
	igmp = (regValue & 0x4FFF) >> 14;
	mld  = (regValue & 0x2FFF) >>13;
	cputag = (regValue & 0x1FFF) >> 12;
	
	switch(protocol) {
	case RTL8306_IGMP:
		/*After IGMP snooping, CPU function, insert tag are all be enalbed, IGMP does work*/
		*enabled = (!cpufun && igmp && cputag) ? TRUE:FALSE;
		break;
	case RTL8306_MLD:
		/*After MLD snooping, CPU function, insert tag are all be enalbed, MLD does work*/
		*enabled = (!cpufun && mld && cputag) ? TRUE:FALSE;
		break;
	default:
		return FAILED;
	}
	return SUCCESS;	
}

int32 rtl8306_setAsicTrapPPPoEPkt(uint32 enabled) {

        rtl8306_setAsicPhyRegBit(2, 22, 4, 3, enabled == TRUE ? 0 : 1);

        #ifdef rtl8306_TblBak
        rtl8306_TblBak.enTrapPPPOE = (uint8) enabled;
        #endif
        return SUCCESS;

}


int32 rtl8306_getAsicTrapPPPoEPkt(uint32 *enabled) {
        uint32 regValue;

        if (enabled == NULL)
            return FAILED;
        rtl8306_getAsicPhyRegBit(2, 22, 4, 3, &regValue);
        *enabled = (regValue == 0 ? TRUE : FALSE);
        return SUCCESS;

}
int32 rtl8306_setAsicQosPortQueueNum(uint32 num) {
	uint32 regValue;
	
	if ((num ==0) ||(num > 4) )
		return FAILED;
	rtl8306_getAsicPhyReg(2, 22, 3, &regValue);		
	rtl8306_setAsicPhyReg(2, 22, 3, (regValue & 0xFFF3) | ((num-1) << 2));	
	/*A soft-reset is required after configuring queue num*/
	 rtl8306_asicSoftReset( );	
	return SUCCESS;	
}

int32 rtl8306_getAsicQosPortQueueNum(uint32 *num) {
	uint32 regValue;
	
	if (num == NULL) 
		return FAILED;
	rtl8306_getAsicPhyReg(2, 22, 3, &regValue);
	*num =    ((regValue & 0xC) >> 2) + 1;
	return SUCCESS;

}

int32 rtl8306_setAsicQosTxQueueWeight(uint32 queue, uint32 weight, uint32 set ) {
	uint32 regValue;
	
	if ((queue > 3) || (weight > 0x7F) || (set > 1))
		return FAILED;
	switch(queue) {
	case RTL8306_QUEUE0:
		if (set == 0) { 
			rtl8306_getAsicPhyReg(5, 20, 3, &regValue);
			regValue = (regValue & 0xFF80) | weight;	
			rtl8306_setAsicPhyReg(5, 20, 3, regValue);			
		} else { 
			rtl8306_getAsicPhyReg(5, 25, 3, &regValue);
			regValue = (regValue & 0xFF80) | weight;
			rtl8306_setAsicPhyReg(5, 25, 3, regValue);						
		}	
		break;
	case RTL8306_QUEUE1:
		if (set == 0)  {
			rtl8306_getAsicPhyReg(5, 20, 3, &regValue);
			regValue = (regValue & 0x80FF) | (weight << 8);
			rtl8306_setAsicPhyReg(5, 20, 3, regValue);			
		} else {
			rtl8306_getAsicPhyReg(5, 25, 3, &regValue);
			regValue = (regValue & 0x80FF) | (weight << 8);
			rtl8306_setAsicPhyReg(5, 25, 3, regValue);			
		}
		break;
	case RTL8306_QUEUE2:
		if (set == 0) {
			rtl8306_getAsicPhyReg(5, 21, 3, &regValue);
			regValue = (regValue & 0xFF80) | weight;	
			rtl8306_setAsicPhyReg(5, 21, 3, regValue);			
		} else {
			rtl8306_getAsicPhyReg(5, 26, 3, &regValue);
			regValue = (regValue & 0xFF80) | weight;
			rtl8306_setAsicPhyReg(5, 26, 3, regValue);			
		}
		break;
	case RTL8306_QUEUE3:
		if (set == 0) {
			rtl8306_getAsicPhyReg(5, 21, 3, &regValue);
			regValue = (regValue & 0x80FF) | (weight << 8);
			rtl8306_setAsicPhyReg(5, 21, 3, regValue);			
		} else {
			rtl8306_getAsicPhyReg(5, 26, 3, &regValue);
			regValue = (regValue & 0x80FF) | (weight << 8);
			rtl8306_setAsicPhyReg(5, 26, 3, regValue);			
		}
		break;
	default:
		return FAILED;
	}

	return SUCCESS;
}

int32 rtl8306_getAsicQosTxQueueWeight(uint32 queue, uint32 *weight, uint32 set) {
	uint32 regValue;
	
	if ((queue > 3) || (set > 1) || (weight == NULL))
		return FAILED;

	switch(queue) {
	case RTL8306_QUEUE0:
		if (set == 0) 
			rtl8306_getAsicPhyReg(5, 20, 3, &regValue);
		else 
			rtl8306_getAsicPhyReg(5, 25, 3, &regValue);
		*weight = regValue & 0x7F;
		break;
	case RTL8306_QUEUE1:
		if (set == 0) 
			rtl8306_getAsicPhyReg(5, 20, 3, &regValue);
		else
			rtl8306_getAsicPhyReg(5, 25, 3, &regValue);
		*weight = (regValue & 0x7F00) >> 8;
		break;
	case RTL8306_QUEUE2:
		if (set == 0)
			rtl8306_getAsicPhyReg(5, 21, 3, &regValue);
		else
			rtl8306_getAsicPhyReg(5, 26, 3, &regValue);
		*weight = regValue & 0x7F;
		break;
	case RTL8306_QUEUE3:
		if (set == 0)
			rtl8306_getAsicPhyReg(5, 21, 3, &regValue);
		else
			rtl8306_getAsicPhyReg(5, 26, 3, &regValue);
		*weight = (regValue & 0x7F00) >> 8;
		break;
	default:
		return FAILED;
	}
	return SUCCESS;
}

int32 rtl8306_setAsicQosTxQueueStrictPriority(uint32 queue, uint32 set, uint32 enabled) {
	
	if ((queue < RTL8306_QUEUE2)  || (set > 1)) 
		return FAILED;
	
	switch(queue) {
	case RTL8306_QUEUE2:
		if (set == 0)
			rtl8306_setAsicPhyRegBit(5, 21, 7, 3, enabled == TRUE ? 1:0);
		else
			rtl8306_setAsicPhyRegBit(5, 26, 7, 3, enabled == TRUE ? 1:0);		
		break;
	case RTL8306_QUEUE3:
		if (set == 0)
			rtl8306_setAsicPhyRegBit(5, 21, 15, 3, enabled == TRUE ? 1:0);
		else
			rtl8306_setAsicPhyRegBit(5, 26, 15, 3, enabled == TRUE ? 1:0);
		break;
	default:
		return FAILED;		
	}			
			
	return SUCCESS;
}

int32 rtl8306_getAsicQosTxQueueStrictPriority(uint32 queue, uint32 set, uint32 *enabled) {
	uint32 bitValue;

	if ((queue < RTL8306_QUEUE2) || (set > 1) || (enabled == NULL)) 
		return FAILED;
	switch(queue) {
	case RTL8306_QUEUE2:
		if (set == 0)
			rtl8306_getAsicPhyRegBit(5, 21, 7, 3, &bitValue);
		else
			rtl8306_getAsicPhyRegBit(5, 26, 7, 3, &bitValue);
		break;
	case RTL8306_QUEUE3:
		if (set == 0)
			rtl8306_getAsicPhyRegBit(5, 21, 15, 3, &bitValue);
		else
			rtl8306_getAsicPhyRegBit(5, 26, 15, 3, &bitValue);
		break;
	default:
		return FAILED;		
	}
	*enabled = (bitValue == 1 ? TRUE:FALSE);
	
	return SUCCESS;		
}

int32 rtl8306_setAsicQosTxQueueLeakyBucket(uint32 queue, uint32 set, uint32 burstsize, uint32 rate) {
	uint32 regValue;

	if ((queue < RTL8306_QUEUE2) || (set > 1) || (burstsize > 0x30) ||(rate > 0x5F6 ))
		return FAILED;
	switch(queue) {
	case RTL8306_QUEUE2:
		if(set == 0) {
			rtl8306_getAsicPhyReg(5, 17, 3, &regValue);
			regValue = (regValue & 0xC0FF) | (burstsize << 8);
			rtl8306_setAsicPhyReg(5, 17, 3, regValue);
			rtl8306_getAsicPhyReg(5, 18, 3, &regValue);
			regValue = (regValue & 0xF800) | (rate & 0x7FF );
			rtl8306_setAsicPhyReg(5, 18, 3, regValue);
		} else {
			rtl8306_getAsicPhyReg(5, 22, 3, &regValue);
			regValue = (regValue & 0xC0FF) | (burstsize << 8);
			rtl8306_setAsicPhyReg(5, 22, 3, regValue);
			rtl8306_getAsicPhyReg(5, 23, 3, &regValue);
			regValue = (regValue & 0xF800) | (rate & 0x7FF);
			rtl8306_setAsicPhyReg(5, 23, 3, regValue);
		}
		break;
	case RTL8306_QUEUE3:
		if(set == 0) {
			rtl8306_getAsicPhyReg(5, 17, 3, &regValue);
			regValue = (regValue & 0xFFC0) | burstsize;
			rtl8306_setAsicPhyReg(5, 17, 3, regValue);
			rtl8306_getAsicPhyReg(5, 19, 3, &regValue);
			regValue = (regValue & 0xF800) | (rate & 0x7FF);
			rtl8306_setAsicPhyReg(5, 19, 3, regValue);
			
		} else {
			rtl8306_getAsicPhyReg(5, 22, 3, &regValue);
			regValue = (regValue & 0xFFC0) | burstsize;
			rtl8306_setAsicPhyReg(5, 22, 3, regValue);
			rtl8306_getAsicPhyReg(5, 24, 3, &regValue);
			regValue = (regValue & 0xF800) | (rate & 0x7FF);
			rtl8306_setAsicPhyReg(5, 24, 3, regValue);
		}
		break;
	default:
		return FAILED;
	}
	/*A soft reset is required after confiugre LB size*/
	 rtl8306_asicSoftReset( );		
	
	return SUCCESS;
}


int32 rtl8306_getAsicQosTxQueueLeakyBucket(uint32 queue, uint32 set, uint32 *burstsize, uint32 *rate) {
	uint32 regValue;

	if ((queue < RTL8306_QUEUE2) || (set > 1) || 
		(burstsize == NULL) || (rate ==NULL))
		return FAILED;
	switch(queue) {
	case RTL8306_QUEUE2 :
		if (set == 0)  {
			rtl8306_getAsicPhyReg(5, 17, 3, &regValue);
			*burstsize = (regValue & 0x3F00) >>8;
			rtl8306_getAsicPhyReg(5, 18, 3, &regValue);
			*rate = regValue &  0x7FF;
		} else {
			rtl8306_getAsicPhyReg(5, 22, 3, &regValue);
			*burstsize = (regValue & 0x3F00) >>8;
			rtl8306_getAsicPhyReg(5, 23, 3, &regValue);
			*rate = regValue & 0x7FF;
		} 
		break;
	case RTL8306_QUEUE3:
		if (set == 0) {
			rtl8306_getAsicPhyReg(5, 17, 3, &regValue);
			*burstsize = regValue & 0x3F;
			rtl8306_getAsicPhyReg(5, 19, 3, &regValue);
			*rate = regValue & 0x7FF;
		
		} else {
			rtl8306_getAsicPhyReg(5, 22, 3, &regValue);
			*burstsize = regValue & 0x3F;
			rtl8306_getAsicPhyReg(5, 24, 3, &regValue);
			*rate = regValue & 0x7FF;
		}
		break;
	default:
		return FAILED;
	}	
	
	return SUCCESS;
} 

int32 rtl8306_setAsicQosPortScheduleMode(uint32 port, uint32 set, uint32 quemask) {
	uint32 regValue;
	
	if ((port > RTL8306_PORT5) ||(set > 1))
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ; 
	quemask = ((quemask & 0x8) >> 3 ) | ((quemask & 0x4) >> 1);
	rtl8306_getAsicPhyReg(port, 18, 2, &regValue);
	regValue = (regValue & 0x97FF) | (quemask << 13) | (set & 0x1) << 11;
	rtl8306_setAsicPhyReg(port, 18, 2, regValue);
	return SUCCESS;
}

int32 rtl8306_getAsicQosPortScheduleMode(uint32 port, uint32 *set, uint32 *quemask) {

	uint32 regValue;
	
	if ((port > RTL8306_PORT5) ||(set == NULL) || (quemask == NULL))
		return FAILED;	
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  
	//rtl8306_getAsicPhyRegBit(port, 18, 11, 2, set);	
	rtl8306_getAsicPhyReg(port, 18, 2, &regValue );
	*set = (regValue >> 11) & 0x1;
	*quemask = (regValue >> 13) & 0x3;
	*quemask = ((*quemask & 0x1) << 3) | ((*quemask & 0x2) << 1);
	return SUCCESS;
}

int32 rtl8306_setAsicQosPortRate(uint32 port, uint32 rate, uint32 direction, uint32 enabled) {
	uint32 regValue;
	
	if ((port > RTL8306_PORT5) || (rate > 0x5F6) || (direction > 1))
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  		
	if (direction == RTL8306_PORT_RX) {  /*configure port Rx rate*/
		if (enabled == FALSE) {
			rtl8306_setAsicPhyRegBit(0, 21, 15, 3, 1);			
		} else {
			rtl8306_setAsicPhyRegBit(0, 21, 15, 3, 0);			
			rtl8306_getAsicPhyReg(port, 21, 2, &regValue);
			regValue = (regValue & 0xF800) | (rate & 0x7FF);
			rtl8306_setAsicPhyReg(port, 21, 2, regValue);
		}		
	} else { 	  /*configure port Tx rate*/
		if (enabled == FALSE) {
			rtl8306_setAsicPhyRegBit(port, 18, 15, 2, 0);			
		} else {
			rtl8306_setAsicPhyRegBit(port, 18, 15, 2, 1);
			rtl8306_getAsicPhyReg(port, 18, 2, &regValue);
			regValue = (regValue & 0xF800) | (rate & 0x7FF);
			rtl8306_setAsicPhyReg(port, 18, 2, regValue);
		}
	}
	return SUCCESS;
}

int32 rtl8306_getAsicQosPortRate(uint32 port, uint32 *rate, uint32 direction, uint32 *enabled) {
	uint32 regValue;

	if ((port > RTL8306_PORT5) || (rate == NULL) || (direction > RTL8306_PORT_TX) || (enabled == NULL))
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  
#if 0	
	rtl8306_getAsicPhyRegBit(port, 18, 15, 2, enabled);
	if (direction == RTL8306_PORT_RX) {	/*Get port Rx rate*/
		rtl8306_getAsicPhyReg(port, 21, 2, &regValue);
		*rate = regValue & 0x7FF;				
	} else { 			/*Get port Tx rate*/
		rtl8306_getAsicPhyReg(port, 18, 2, &regValue);
		*rate = regValue & 0x7FF;
	}
#endif
	if (direction == RTL8306_PORT_RX) {	/*Get port Rx rate*/
		rtl8306_getAsicPhyRegBit(0, 21, 15, 3, &regValue);
		*enabled = (regValue == 1 ? FALSE:TRUE);
		rtl8306_getAsicPhyReg(port, 21, 2, &regValue);
		*rate = regValue & 0x7FF;				
	} else { 			/*Get port Tx rate*/
		rtl8306_getAsicPhyRegBit(port, 18, 15, 2, enabled);		
		rtl8306_getAsicPhyReg(port, 18, 2, &regValue);
		*rate = regValue & 0x7FF;
	}
	return SUCCESS;
}

int32 rtl8306_setAsicQosRxRateGlobalControl(uint32 hisize, uint32 losize, uint32 preamble) {
	uint32 regValue;

	
	if ((hisize > 0x3F ) || (losize > 0x3F) || (preamble >1))
		return FAILED;

	rtl8306_getAsicPhyReg(0, 21, 3, &regValue);
	regValue = (regValue & 0x80C0) | ((preamble == TRUE ? 1:0) << 14) |(hisize <<8) | (losize);
	rtl8306_setAsicPhyReg(0, 21, 3, regValue);
	return SUCCESS;
} 

int32 rtl8306_getAsicQosRxRateGlobalControl(uint32 *hisize, uint32 *losize, uint32 *preamble) {
	uint32 regValue;

	if ((hisize == NULL) || (losize == NULL) || (preamble == NULL))
		return FAILED;
	rtl8306_getAsicPhyReg(0, 21, 3, &regValue);	
	*preamble = (regValue & 0x4000) ? TRUE:FALSE;
	*hisize = (regValue & 0x3F00) >> 8;
	*losize = regValue & 0x3F;

	return SUCCESS;
}

int32 rtl8306_setAsicQosPktPriorityAssign(uint32 type, uint32 level) {
	uint32 regValue;

	if ((type > 3) || (level > 4))
		return FAILED;
	rtl8306_getAsicPhyReg(1, 21, 3, &regValue);
	switch(type) {
	case RTL8306_ACL_PRIO:
		if (level == 0)
			regValue = regValue & 0x0FFF;
		else 
			regValue = (regValue & 0x0FFF) | (1 << (level - 1 + 12));
		break;
	case  RTL8306_DSCP_PRIO:
		if (level == 0)
			regValue = regValue & 0xF0FF;
		else 
			regValue = (regValue & 0xF0FF) | (1 << (level- 1 + 8 ));
		break;
	case RTL8306_1QBP_PRIO:
		if (level == 0)
			regValue = regValue & 0xFF0F;
		else
			regValue = (regValue & 0xFF0F) | (1 << (level -1 + 4 ));
		break;
	case RTL8306_PBP_PRIO:
		if (level == 0)
			regValue = regValue & 0xFFF0;
		else 
			regValue = (regValue & 0xFFF0) | (1 << (level -1));
		break;
	default:
		return FAILED;
	}	
	rtl8306_setAsicPhyReg(1, 21, 3, regValue);
	return SUCCESS;
}


int32 rtl8306_getAsicQosPktPriorityAssign(uint32 type, uint32 *level) {
	uint32 regValue;

	if ((type > 3) ||(level == NULL))
		return FAILED;
	rtl8306_getAsicPhyReg(1, 21, 3, &regValue);
	switch(type) {
	case RTL8306_ACL_PRIO :
		regValue = (regValue & 0xF000) >> 12;								
		break;
	case RTL8306_DSCP_PRIO:
		regValue = (regValue & 0x0F00) >> 8;
		break;
	case RTL8306_1QBP_PRIO:
		regValue = (regValue & 0x00F0) >> 4;
		break;
	case RTL8306_PBP_PRIO:
		regValue = (regValue & 0x000F);
		break;
	default :
		return FAILED;
	}
	
	switch(regValue) {
	case 0x0:
		*level = 0;
		break;
	case 0x1:
		*level = 1;
		break;
	case 0x2:
		*level = 2;
		break;
	case 0x4:
		*level = 3;
		break;
	case 0x8:
		*level =4;
		break;
	default:
		return FAILED;
	}
	
	return SUCCESS;
}

int32 rtl8306_setAsicQosPrioritytoQIDMapping(uint32 priority, uint32 qid) {
	uint32 regValue;

	if ((qid >3) || (priority > 3)) 
		return FAILED;
	rtl8306_getAsicPhyReg(1, 22, 3, &regValue);
	switch(priority) {
	case 0:
		regValue = (regValue & 0xFFFC) | qid;	
		break;
	case 1:
		regValue = (regValue & 0xFFF3) | (qid << 2);
		break;
	case 2:
		regValue = (regValue & 0xFFCF) | (qid << 4);
		break;
	case 3:
		regValue = (regValue & 0xFF3F) | (qid << 6);
		break;
	default:
		return FAILED;
	}
	rtl8306_setAsicPhyReg(1, 22, 3, regValue);	
	return SUCCESS;
}

int32 rtl8306_getAsicQosPrioritytoQIDMapping(uint32 priority, uint32 *qid) {
	uint32 regValue;
	
	if ((priority > 3) || (qid == NULL))
		return FAILED;
	rtl8306_getAsicPhyReg(1, 22, 3, &regValue);
	switch(priority) {
	case 0:
		*qid = regValue & 0x3;
		break;
	case 1:
		*qid = (regValue & 0xC) >>2;
		break;
	case 2:
		*qid = (regValue & 0x30) >> 4;
		break;
	case 3:
		*qid = (regValue & 0xC0) >> 6;
		break;
	default:
		return FAILED;
	}	
	return SUCCESS;	
	
}

int32 rtl8306_setAsicQosPortBasedPriority(uint32 port, uint32 priority) {
	uint32 regValue;

	if ((port > RTL8306_PORT5) ||(priority > 3))
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  
	rtl8306_getAsicPhyReg(port, 17, 2, &regValue);
	regValue = (regValue & 0xE7FF) | (priority << 11);
	rtl8306_setAsicPhyReg(port, 17, 2, regValue);
	return SUCCESS;
}

int32 rtl8306_getAsicQosPortBasedPriority(uint32 port, uint32 *priority) {
	uint32 regValue;

	if ((port > RTL8306_PORT5) ||(priority == NULL))
		return FAILED;
	if (port < RTL8306_PORT5) 
		rtl8306_getAsicPhyReg(port, 17, 2, &regValue);
	else
		rtl8306_getAsicPhyReg(6, 17, 2, &regValue);
	*priority = (regValue & 0x1800) >> 11;
	
	return SUCCESS;
}

int32 rtl8306_setAsicQos1QBasedPriority(uint32 port, uint32 priority) {
	uint32 regValue;

	if ((port > RTL8306_PORT5) || (priority > 3) )
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  
	rtl8306_getAsicPhyReg(port, 17, 2, &regValue);
	regValue = (regValue & 0x9FFF) | (priority << 13);
	rtl8306_setAsicPhyReg(port, 17, 2, regValue);
	return SUCCESS;
}

int32 rtl8306_getAsicQos1QBasedPriority(uint32 port, uint32 *priority) {
	uint32 regValue;

	if ((port > RTL8306_PORT5) || (priority == NULL))
		return FAILED;
	if (port < RTL8306_PORT5) 
		rtl8306_getAsicPhyReg(port, 17, 2, &regValue);
	else
		rtl8306_getAsicPhyReg(6, 17, 2, &regValue);
	*priority = (regValue & 0x6000) >> 13;
	
	return SUCCESS;
}

int32 rtl8306_setAsicQos1QtagPriorityto2bitPriority(uint32 tagprio, uint32 prio) {	
	uint32 regValue;

	if ((tagprio > RTL8306_1QTAG_PRIO7) || (prio > RTL8306_PRIO3 ))
		return FAILED;
	rtl8306_getAsicPhyReg(2, 24, 3, &regValue);
	switch(tagprio) {
	case RTL8306_1QTAG_PRIO0:	
		regValue = (regValue & 0xFFFC) | prio;
		break;
	case RTL8306_1QTAG_PRIO1:
		regValue = (regValue & 0xFFF3) |(prio << 2);
		break;
	case RTL8306_1QTAG_PRIO2:
		regValue = (regValue & 0xFFCF) | (prio << 4);
		break;
	case RTL8306_1QTAG_PRIO3:
		regValue = (regValue & 0xFF3F) | (prio << 6);
		break;
	case RTL8306_1QTAG_PRIO4:
		regValue = (regValue & 0xFCFF) | (prio << 8);
		break;
	case RTL8306_1QTAG_PRIO5:
		regValue = (regValue & 0xF3FF) | (prio << 10);
		break;
	case RTL8306_1QTAG_PRIO6:
		regValue = (regValue & 0xCFFF) | (prio << 12);
		break;
	case RTL8306_1QTAG_PRIO7:
		regValue = (regValue & 0x3FFF) | (prio << 14);
		break;
	default:
		return FAILED;
	}
	rtl8306_setAsicPhyReg(2, 24, 3, regValue);
	return SUCCESS;
}

int32 rtl8306_getAsicQos1QtagPriorityto2bitPriority(uint32 tagprio, uint32 *prio) {
	uint32 regValue;
	
	if ((tagprio > RTL8306_1QTAG_PRIO7) || (prio == NULL ))
		return FAILED;
	rtl8306_getAsicPhyReg(2, 24, 3, &regValue);
	switch(tagprio) {
	case RTL8306_1QTAG_PRIO0:	
		*prio = regValue & 0x3;
		break;
	case RTL8306_1QTAG_PRIO1:
		*prio = (regValue & 0xC) >> 2;
		break;
	case RTL8306_1QTAG_PRIO2:
		*prio = (regValue & 0x30) >> 4;
		break;
	case RTL8306_1QTAG_PRIO3:
		*prio = (regValue & 0xC0) >> 6;
		break;
	case RTL8306_1QTAG_PRIO4:
		*prio = (regValue & 0x300) >> 8;
		break;
	case RTL8306_1QTAG_PRIO5:
		*prio = (regValue & 0xC00) >> 10;
		break;
	case RTL8306_1QTAG_PRIO6:
		*prio = (regValue & 0x3000) >> 12;
		break;
	case RTL8306_1QTAG_PRIO7:
		*prio = (regValue & 0xC000) >> 14;
		break;
	default:
		return FAILED;
	}

	return SUCCESS;	
}

int32 rtl8306_setAsicQosDSCPBasedPriority(uint32 type, uint32 priority) {
	uint32 regValue1, regValue2;

	if ((type > RTL8306_DSCP_BF) ||(priority > RTL8306_PRIO3))
		return FAILED;
	
	rtl8306_getAsicPhyReg(1, 23, 3, &regValue1);
	rtl8306_getAsicPhyReg(1, 24, 3, &regValue2);
	switch(type) {
	case RTL8306_DSCP_EF:
		regValue1 = (regValue1 & 0xFFFC) | priority;
		rtl8306_setAsicPhyReg(1, 23, 3, regValue1);
		break;
	case RTL8306_DSCP_AFL1:
		regValue1 = (regValue1 & 0xFFF3) | (priority << 2);
		rtl8306_setAsicPhyReg(1, 23, 3, regValue1);
		break;
	case RTL8306_DSCP_AFM1:
		regValue1 = (regValue1 & 0xFFCF) | (priority << 4);
		rtl8306_setAsicPhyReg(1, 23, 3, regValue1);		
		break;
	case RTL8306_DSCP_AFH1:
		regValue1 = (regValue1 & 0xFF3F) | (priority << 6);
		rtl8306_setAsicPhyReg(1, 23, 3, regValue1);		
		break;
	case RTL8306_DSCP_AFL2:
		regValue1 = (regValue1 & 0xFCFF) | (priority << 8);
		rtl8306_setAsicPhyReg(1, 23, 3, regValue1);		
		break;
	case RTL8306_DSCP_AFM2:
		regValue1 = (regValue1 & 0xF3FF) | (priority << 10);
		rtl8306_setAsicPhyReg(1, 23, 3, regValue1);		
		break;
	case RTL8306_DSCP_AFH2:
		regValue1 = (regValue1 & 0xCFFF) |(priority << 12);
		rtl8306_setAsicPhyReg(1, 23, 3, regValue1);		
		break;
	case RTL8306_DSCP_AFL3:
		regValue1 = (regValue1 & 0x3FFF) | (priority << 14);
		rtl8306_setAsicPhyReg(1, 23, 3, regValue1);				
		break;
	case RTL8306_DSCP_AFM3:		
		regValue2 = (regValue2 & 0xFFFC) | priority;
		rtl8306_setAsicPhyReg(1, 24, 3, regValue2);		
		break;
	case RTL8306_DSCP_AFH3:
		regValue2 = (regValue2 & 0xFFF3) | (priority <<2);
		rtl8306_setAsicPhyReg(1, 24, 3, regValue2);				
		break;
	case RTL8306_DSCP_AFL4:
		regValue2 = (regValue2 & 0xFFCF) | (priority <<4);
		rtl8306_setAsicPhyReg(1, 24, 3, regValue2);						
		break;
	case RTL8306_DSCP_AFM4:
		regValue2 = (regValue2 & 0xFF3F) | (priority << 6);
		rtl8306_setAsicPhyReg(1, 24, 3, regValue2);						
		break;
	case RTL8306_DSCP_AFH4:
		regValue2 = (regValue2 & 0xFCFF) | (priority << 8);
		rtl8306_setAsicPhyReg(1, 24, 3, regValue2);						
		break;
	case RTL8306_DSCP_NC:
		regValue2 = (regValue2 & 0xF3FF) | (priority << 10);
		rtl8306_setAsicPhyReg(1, 24, 3, regValue2);						
		break;
	case RTL8306_DSCP_REG_PRI:
		regValue2 = (regValue2 & 0xCFFF) | (priority << 12);
		rtl8306_setAsicPhyReg(1, 24, 3, regValue2);						
		break;
	case RTL8306_DSCP_BF:
		regValue2 = (regValue2 & 0x3FFF) | (priority << 14);
		rtl8306_setAsicPhyReg(1, 24, 3, regValue2);						
		break;
	default:
		return FAILED;
	}
	return SUCCESS;
}

int32 rtl8306_getAsicQosDSCPBasedPriority(uint32 type, uint32 *priority) {
	uint32	regValue1, regValue2;

	if ((type > RTL8306_DSCP_BF) || (priority == NULL))
		return FAILED;
	
	rtl8306_getAsicPhyReg(1, 23, 3, &regValue1);
	rtl8306_getAsicPhyReg(1, 24, 3, &regValue2);
	switch(type) {
	case RTL8306_DSCP_EF:
		*priority = regValue1 & 0x3;
		break;
	case RTL8306_DSCP_AFL1:
		*priority = (regValue1 & 0xC) >> 2;
		break;
	case RTL8306_DSCP_AFM1:
		*priority = (regValue1 & 0x30) >> 4;
		break;
	case RTL8306_DSCP_AFH1:
		*priority = (regValue1 & 0xC0) >> 6;
		break;
	case RTL8306_DSCP_AFL2:
		*priority = (regValue1 & 0x300) >> 8;
		break;
	case RTL8306_DSCP_AFM2:
		*priority = (regValue1 & 0xC00) >> 10;
		break;
	case RTL8306_DSCP_AFH2:
		*priority = (regValue1 & 0x3000) >> 12;
		break;
	case RTL8306_DSCP_AFL3:
		*priority = (regValue1 & 0xC000) >> 14;
		break;
	case RTL8306_DSCP_AFM3:		
		*priority = regValue2 & 0x3;
		break;
	case RTL8306_DSCP_AFH3:
		*priority = (regValue2 & 0xC) >> 2;
		break;
	case RTL8306_DSCP_AFL4:
		*priority = (regValue2 & 0x30) >> 4;
		break;
	case RTL8306_DSCP_AFM4:
		*priority = (regValue2 & 0xC0) >> 6;
		break;
	case RTL8306_DSCP_AFH4:
		*priority = (regValue2 & 0x300) >> 8;
		break;
	case RTL8306_DSCP_NC:
		*priority = (regValue2 & 0xC00) >>10;
		break;
	case RTL8306_DSCP_REG_PRI:
		*priority = (regValue2 & 0x3000) >> 12;
		break;
	case RTL8306_DSCP_BF:
		*priority = (regValue2 & 0xC000) >> 14;
		break;
	default:
		return FAILED;
	}
		
	return SUCCESS;
} 

int32 rtl8306_setAsicQosDSCPUserAssignPriority(uint32 entry, uint32 priority, uint32 enabled) {
	uint32 regValue;
	
	if ((entry > 1) || (priority > 0x3F ))
		return FAILED;
	rtl8306_getAsicPhyReg(0, 17, 0, &regValue);
	switch (entry) {
	case RTL8306_DSCP_USERA:
		if (enabled == TRUE)
			regValue = (regValue & 0x40FF) | (priority << 8) | (1 << 15);
		else
			regValue = (regValue & 0x40FF) | (priority << 8);
		break;
	case RTL8306_DSCP_USERB:
		if (enabled == TRUE)
			regValue = (regValue & 0xFF40) | priority | (1 << 7);
		else
			regValue = (regValue & 0xFF40) | priority;
		break;
	default:
		return FAILED;
	}
	rtl8306_setAsicPhyReg(0, 17, 0, regValue);
	return SUCCESS;
}

int32 rtl8306_getAsicQosDSCPUserAssignPriority(uint32 entry, uint32 *priority, uint32 *enabled) {
	uint32 regValue;

	if ((entry > 1) || (priority == NULL))
		return FAILED;
	rtl8306_getAsicPhyReg(0, 17, 0, &regValue);
	switch (entry) {
	case RTL8306_DSCP_USERA:
		*priority = (regValue & 0x3F00) >> 8;
		*enabled = (regValue & 0x8000 ? TRUE: FALSE); 
		break;
	case RTL8306_DSCP_USERB:
		*priority = regValue & 0x3F;
		*enabled = (regValue & 0x80 ? TRUE : FALSE);
		break;
	default:
		return FAILED;
	}
	return SUCCESS;
}

int32 rtl8306_setAsicQosIPAddressPriority(uint32 priority) {
	uint32 regValue;

	if (priority > 3)
		return FAILED;
	rtl8306_getAsicPhyReg(2, 22, 3, &regValue);
	rtl8306_setAsicPhyReg(2, 22, 3, (regValue & 0xFFFC) |priority);
	return SUCCESS;
}

int32 rtl8306_getAsicQosIPAddressPriority(uint32 *priority) {
	uint32 regValue;

	if (priority == NULL)
		return FAILED;
	rtl8306_getAsicPhyReg(2, 22, 3, &regValue);
	*priority =  regValue & 0x3;
	return SUCCESS;
}

int32 rtl8306_setAsicQosIPAddress(uint32 entry, uint32 ip, uint32 mask, uint32 enabled) {
	uint32 regValue;
	
	if (entry > 1) 
		return FAILED;
	switch(entry) {
	case RTL8306_IPADD_A:
#if 0		
		regValue = ip & 0xFFFF;
		rtl8306_setAsicPhyReg(1, 17, 0, regValue);
		regValue = (ip & 0xFFFF0000) >> 16;
		rtl8306_setAsicPhyReg(1, 16, 0, regValue);
		regValue = mask & 0xFFFF;
		rtl8306_setAsicPhyReg(2, 17, 0, regValue);
		regValue = (mask & 0xFFFF0000) >> 16;
		rtl8306_setAsicPhyReg(2, 16, 0, regValue);
#endif		
		if (enabled == TRUE) {
			rtl8306_setAsicPhyRegBit(0, 17, 14, 0, 1);
			regValue = ip & 0xFFFF;
			rtl8306_setAsicPhyReg(1, 17, 0, regValue);
			regValue = (ip & 0xFFFF0000) >> 16;
			rtl8306_setAsicPhyReg(1, 16, 0, regValue);
			regValue = mask & 0xFFFF;
			rtl8306_setAsicPhyReg(2, 17, 0, regValue);
			regValue = (mask & 0xFFFF0000) >> 16;
			rtl8306_setAsicPhyReg(2, 16, 0, regValue);
		}	
		else 
			rtl8306_setAsicPhyRegBit(0, 17, 14, 0, 0);
		break;
	case RTL8306_IPADD_B:
#if 0		
		regValue = ip & 0xFFFF;
		rtl8306_setAsicPhyReg(1, 19, 0, regValue);
		regValue = (ip & 0xFFFF0000) >> 16;
		rtl8306_setAsicPhyReg(1, 18, 0, regValue);
		regValue = mask & 0xFFFF;
		rtl8306_setAsicPhyReg(2, 19, 0, regValue);
		regValue = (mask & 0xFFFF0000) >> 16;
		rtl8306_setAsicPhyReg(2, 18, 0, regValue);
#endif		
		if (enabled == TRUE) {
			rtl8306_setAsicPhyRegBit(0, 17, 6, 0, 1);
			regValue = ip & 0xFFFF;
			rtl8306_setAsicPhyReg(1, 19, 0, regValue);
			regValue = (ip & 0xFFFF0000) >> 16;
			rtl8306_setAsicPhyReg(1, 18, 0, regValue);
			regValue = mask & 0xFFFF;
			rtl8306_setAsicPhyReg(2, 19, 0, regValue);
			regValue = (mask & 0xFFFF0000) >> 16;
			rtl8306_setAsicPhyReg(2, 18, 0, regValue);
		}
		else 
			rtl8306_setAsicPhyRegBit(0, 17, 6, 0, 0);		
		break;
	default:
		return FAILED;
	}	
	return SUCCESS;
}

int32 rtl8306_getAsicQosIPAddress(uint32 entry, uint32 *ip, uint32 *mask , uint32 *enabled) {
	uint32 hi, lo;
	uint32 bitValue;

	if ((entry > 1) || (ip == NULL) || (mask == NULL) || (enabled == NULL))
		return FAILED;
	switch (entry) {
	case RTL8306_IPADD_A :
		rtl8306_getAsicPhyReg(1, 17, 0, &lo);
		rtl8306_getAsicPhyReg(1, 16, 0, &hi);
		*ip = lo + (hi << 16);
		rtl8306_getAsicPhyReg(2, 17, 0, &lo);
		rtl8306_getAsicPhyReg(2, 16, 0, &hi);
		*mask = lo + (hi << 16);
		rtl8306_getAsicPhyRegBit(0, 17, 14, 0, &bitValue);
		*enabled = (bitValue == 1 ? TRUE : FALSE);
		break;
	case RTL8306_IPADD_B :
		rtl8306_getAsicPhyReg(1, 19, 0, &lo);
		rtl8306_getAsicPhyReg(1, 18, 0, &hi);
		*ip = lo + (hi << 16);
		rtl8306_getAsicPhyReg(2, 19, 0, &lo);
		rtl8306_getAsicPhyReg(2, 18, 0, &hi);
		*mask = lo + (hi << 16); 
		rtl8306_getAsicPhyRegBit(0, 17, 6, 0, &bitValue);
		*enabled = (bitValue == 1 ? TRUE : FALSE);		
		break;
	default :
		return FAILED;		
	}	
	return SUCCESS;
}

int32 rtl8306_setAsicQosPriorityEnable(uint32 port, uint32 type, uint32 enabled) {
      uint32 duplex, speed, nway;

      
	if (port > RTL8306_PORT5)
		return FAILED;
       /*save mac 4 or port status when operate reg.22*/    
       if (port == 4) {
            rtl8306_getAsicPhyRegBit(5, 0, 13, 0, &speed);
            rtl8306_getAsicPhyRegBit(5, 0, 12, 0, &nway);
            rtl8306_getAsicPhyRegBit(5, 0, 8, 0, &duplex);            
       } else if (port == 5) {
            rtl8306_getAsicPhyRegBit(6, 0, 13, 0, &speed);
            rtl8306_getAsicPhyRegBit(6, 0, 12, 0, &nway);
            rtl8306_getAsicPhyRegBit(6, 0, 8, 0, &duplex);            
       }
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  
	switch(type) {
	case RTL8306_DSCP_PRIO:
		rtl8306_setAsicPhyRegBit(port, 22, 9, 0, enabled == FALSE ? 1:0 );
		break;
	case RTL8306_1QBP_PRIO:
		rtl8306_setAsicPhyRegBit(port, 22, 10, 0, enabled == FALSE ? 1:0 );
		break;
	case RTL8306_PBP_PRIO:
		rtl8306_setAsicPhyRegBit(port, 22, 8, 0, enabled == FALSE ? 1:0 );
		break;
	case RTL8306_CPUTAG_PRIO:
		rtl8306_setAsicPhyRegBit(port, 17, 1, 2, enabled == TRUE ? 1:0);
		break;
	default:
		return FAILED;
	}	
       /*restore mac 4 or port status when operate reg.22*/    
      if (port == 4) {
            rtl8306_setAsicPhyRegBit(5, 0, 13, 0, speed);
            rtl8306_setAsicPhyRegBit(5, 0, 12, 0, nway);
            rtl8306_setAsicPhyRegBit(5, 0, 8, 0, duplex);
            
      } else if (port == 6)  {  /*for port++ when port 5*/
           rtl8306_setAsicPhyRegBit(6, 0, 13, 0, speed);
           rtl8306_setAsicPhyRegBit(6, 0, 12, 0, nway);
           rtl8306_setAsicPhyRegBit(6, 0, 8, 0, duplex);
      }      
	return SUCCESS;
}

int32 rtl8306_getAsicQosPriorityEnable(uint32 port, uint32 type, uint32 *enabled) {
	uint32 bitValue;
	
	if ((port > RTL8306_PORT5) || (enabled == NULL))
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  	
	switch(type) {
	case RTL8306_DSCP_PRIO:
		rtl8306_getAsicPhyRegBit(port, 22, 9, 0, &bitValue);
		*enabled = (bitValue ==1 ? FALSE : TRUE);
		break;
	case RTL8306_1QBP_PRIO:
		rtl8306_getAsicPhyRegBit(port, 22, 10, 0, &bitValue);
		*enabled = (bitValue ==1 ? FALSE : TRUE);
		break;
	case RTL8306_PBP_PRIO:
		rtl8306_getAsicPhyRegBit(port, 22, 8, 0, &bitValue);
		*enabled = (bitValue ==1 ? FALSE : TRUE);
		break;
	case RTL8306_CPUTAG_PRIO:
		rtl8306_getAsicPhyRegBit(port, 17, 1, 2, &bitValue);
		*enabled = (bitValue ==1 ? TRUE : FALSE);
		break;
	default:
		return FAILED;
	}
		
	return SUCCESS;
}


int32 rtl8306_setAsicQosSystemRxFlowControl(uint32 enabled) {

	rtl8306_setAsicPhyRegBit(0, 22, 9, 3, enabled == TRUE ? 1:0);
	return SUCCESS;
} 

int32 rtl8306_getAsicQosSystemRxFlowControl(uint32 *enabled) {
	uint32 bitValue;

	if (enabled == NULL)
		return FAILED;
	rtl8306_getAsicPhyRegBit(0, 22, 9, 3, &bitValue);
	*enabled = (bitValue == 1 ? TRUE:FALSE);
	return SUCCESS;
}


int32 rtl8306_setAsicAclEntry(uint32 entryadd, uint32 phyport, uint32 action, uint32 protocol, uint32 data, uint32 priority) {
	uint32 regValue, value;
	uint32 pollcnt  ;
	uint32 bitValue;
	
	if ((entryadd >= RTL8306_ACL_ENTRYNUM) || (phyport > RTL8306_ACL_ANYPORT) || 
		(action > RTL8306_ACT_MIRROR) ||(protocol > RTL8306_ACL_TCPUDP) 
		||(priority > RTL8306_PRIO3))
			return FAILED;

	/*Enable CPU port function, Enable inserting CPU TAG, Enable removing CPU TAG */
	rtl8306_getAsicPhyReg(2, 21, 3, &regValue);
	regValue = (regValue & 0x7FFF) | (1 << 11) | (1<<12);	
	rtl8306_setAsicPhyReg(2, 21, 3, regValue);	
	/*set EtherType or TCP/UDP Ports, ACL entry access register 0*/
	rtl8306_setAsicPhyReg(3, 21, 3, data);
	/*set ACL entry access register 1*/
	rtl8306_getAsicPhyReg(3, 22, 3, &regValue);
	value = (1 << 14) |(entryadd << 9)  | (priority << 7) | (action << 5) | (phyport << 2) | protocol ;
	regValue = (regValue & 0x8000) | value  ;
	rtl8306_setAsicPhyReg(3, 22, 3, regValue);
	/*Polling whether the command is done*/
	for (pollcnt = 0; pollcnt < RTL8306_IDLE_TIMEOUT ; pollcnt++) {
		rtl8306_getAsicPhyRegBit(3, 22, 14, 3, &bitValue);
		if (!bitValue)
			break;
	}
	if (pollcnt == RTL8306_IDLE_TIMEOUT)
		return FAILED;

      #ifdef RTL8306_TBLBAK
            rtl8306_TblBak.aclTbl[entryadd].phy_port = phyport;
            rtl8306_TblBak.aclTbl[entryadd].action = action;
            rtl8306_TblBak.aclTbl[entryadd].proto = protocol;
            rtl8306_TblBak.aclTbl[entryadd].data = data;
            rtl8306_TblBak.aclTbl[entryadd].pri = priority;
      #endif  
	return SUCCESS;
}


int32 rtl8306_getAsicAclEntry(uint32 entryadd, uint32 *phyport, uint32 *action, uint32 *protocol, uint32  *data, uint32 *priority) {
	uint32 regValue;
	uint32 pollcnt  ;
	uint32 bitValue;

	if ((entryadd >= RTL8306_ACL_ENTRYNUM) || (phyport == NULL) || (action == NULL) || 
		(protocol == NULL) || (data == NULL) || (priority == NULL))
		return FAILED;
	/*trigger a command to read ACL entry*/
	rtl8306_getAsicPhyReg(3, 22, 3, &regValue);
	regValue = (regValue & 0x81FF) | (0x3 << 13) | (entryadd << 9);
	rtl8306_setAsicPhyReg(3, 22, 3, regValue);
	/*Polling whether the command is done*/
	for (pollcnt = 0; pollcnt < 100 ; pollcnt++) {
		rtl8306_getAsicPhyRegBit(3, 22, 14, 3, &bitValue);
		if (!bitValue)
			break;
	}
	if (pollcnt > 50)
		return FAILED;
	rtl8306_getAsicPhyReg(3, 21, 3, &regValue);
	*data = regValue;
	rtl8306_getAsicPhyReg(3, 22, 3, &regValue);
	*priority = (regValue  >> 7) & 0x3;
	*action  = (regValue  >> 5) & 0x3;
	*phyport = (regValue >> 2) & 0x7;
	*protocol = regValue & 0x3;
	
	return SUCCESS;
}

int32 rtl8306_getAsicMibCounter(uint32 port, uint32 counter, uint32 *value) {
	uint32 regValue1, regValue2;

	if ((port > RTL8306_PORT5) || (counter > RTL8306_MIB_CNT5) || (value == NULL))
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  
	
	switch(counter) {
	case RTL8306_MIB_CNT1:
		/*Must read low 16 bit first, then hight 16 bit*/
		rtl8306_getAsicPhyReg(port, 22, 2, &regValue1);
		rtl8306_getAsicPhyReg(port, 23, 2, &regValue2);
		*value = (regValue2 << 16) + regValue1;
		break;
	case RTL8306_MIB_CNT2:
		/*Must read low 16 bit first, then hight 16 bit*/
		rtl8306_getAsicPhyReg(port, 24, 2, &regValue1);
		rtl8306_getAsicPhyReg(port, 25, 2, &regValue2);
		*value = (regValue2 << 16) + regValue1;		
		break;
	case RTL8306_MIB_CNT3:
		/*Must read low 16 bit first, then hight 16 bit*/
		rtl8306_getAsicPhyReg(port, 26, 2, &regValue1);
		rtl8306_getAsicPhyReg(port, 27, 2, &regValue2);
		*value = (regValue2 << 16) + regValue1;		
		break;
	case RTL8306_MIB_CNT4:
		/*Must read low 16 bit first, then hight 16 bit*/
		rtl8306_getAsicPhyReg(port, 28, 2, &regValue1);
		rtl8306_getAsicPhyReg(port, 29, 2, &regValue2);
		*value = (regValue2 << 16) + regValue1;		
		break;
	case RTL8306_MIB_CNT5:
		/*Must read low 16 bit first, then hight 16 bit*/
		rtl8306_getAsicPhyReg(port, 30, 2, &regValue1);
		rtl8306_getAsicPhyReg(port, 31, 2, &regValue2);
		*value = (regValue2 << 16) + regValue1;		
		break;
	default:
		return FAILED;		
	}	
	return SUCCESS;
}

int32 rtl8306_setAsicMibCounterReset(uint32 port, uint32 operation) {

	if ((port > RTL8306_PORT5) || (operation >1 ))
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  	
	switch(operation) {
	case RTL8306_MIB_RESET:
		rtl8306_setAsicPhyRegBit(port, 17, 2, 2, 1);
		break;
	case RTL8306_MIB_START:
		rtl8306_setAsicPhyRegBit(port, 17, 2, 2, 0);
		break;		
	default :
		return FAILED;
	}	
	return SUCCESS;
}


int32 rtl8306_setAsicMibCounterUnit(uint32 port, uint32 counter, uint32 unit) {

	if ((port > RTL8306_PORT5) ||(unit > RTL8306_MIB_PKT))
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  		
	switch(counter) {
	case RTL8306_MIB_CNT1:
		rtl8306_setAsicPhyRegBit(port, 17, 3, 2, unit);		
		break;
	case RTL8306_MIB_CNT2:
		rtl8306_setAsicPhyRegBit(port, 17, 4, 2, unit);
		break;		
	default :
		return FAILED;
	}	
	return SUCCESS;	
}

int32 rtl8306_getAsicMibCounterUnit(uint32 port, uint32 counter, uint32 *unit) { 
	uint32 bitValue;
	
	if ((port > RTL8306_PORT5) ||(unit == NULL))
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  			
	switch(counter) {
	case RTL8306_MIB_CNT1:
		rtl8306_getAsicPhyRegBit(port, 17, 3, 2, &bitValue);
		break;
	case RTL8306_MIB_CNT2:
		rtl8306_getAsicPhyRegBit(port, 17, 4, 2, &bitValue);
		break;
	default:
		return FAILED;
	}
	*unit = (bitValue == 1 ? RTL8306_MIB_PKT : RTL8306_MIB_BYTE);
	return SUCCESS;
}
int32 rtl8306_setAsicMirrorPort(uint32 mirport, uint32 rxport, uint32 txport, uint32 enFilter) {
	uint32 regValue;
	
	if ((mirport > 7) ||(rxport > 0x3F) || (txport > 0x3F) )
		return FAILED;

	/*Set Mirror Port*/
	rtl8306_getAsicPhyReg(2, 22, 3, &regValue);
	regValue = (regValue & 0xC7FF) | (mirport << 11);
	rtl8306_setAsicPhyReg(2, 22, 3, regValue);
	/*Whether enable mirror port to filter the mirrored packet sent from itself */
	rtl8306_setAsicPhyRegBit(6, 21, 7, 3, enFilter == TRUE  ? 1:0);
	/*Set Ports Whose RX Data are Mirrored */
	rtl8306_getAsicPhyReg(6, 21, 3, &regValue);
	regValue = (regValue & 0xFFC0) | rxport ;
	rtl8306_setAsicPhyReg(6, 21, 3, regValue);	
	/*Set Ports Whose TX Data are Mirrored */
	rtl8306_getAsicPhyReg(6, 21, 3, &regValue);
	regValue = (regValue & 0xC0FF) | (txport << 8);
	rtl8306_setAsicPhyReg(6, 21, 3, regValue);
       #ifdef RTL8306_TBLBAK
           rtl8306_TblBak.mir.mirPort = (uint8)mirport;
           rtl8306_TblBak.mir.mirRxPortMask = (uint8)rxport;
           rtl8306_TblBak.mir.mirTxPortMask = (uint8)txport;
           rtl8306_TblBak.mir.enMirself = (uint8)enFilter;
       #endif
	return SUCCESS;
}


int32 rtl8306_getAsicMirrorPort(uint32 *mirport, uint32 *rxport, uint32 *txport, uint32 *enFilter) {
	uint32 regValue;
	uint32 bitValue;
	
	if ((mirport == NULL) ||(rxport == NULL) || (txport == NULL)) 
		return FAILED;
	/*Get Mirror Port*/
	rtl8306_getAsicPhyReg(2, 22, 3, &regValue);
	*mirport = (regValue & 0x3800) >> 11;
	/*Whether enable mirror port to filter the mirrored packet sent from itself */
	rtl8306_getAsicPhyRegBit(6, 21, 7, 3, &bitValue);
	*enFilter = (bitValue == 1 ? TRUE:FALSE);
	/*Get Ports Whose RX Data are Mirrored*/	
	rtl8306_getAsicPhyReg(6, 21, 3, &regValue);	
	*rxport = regValue & 0x3F;	
	/*Get Ports Whose TX Data are Mirrored */	
	rtl8306_getAsicPhyReg(6, 21, 3, &regValue);	
	*txport = (regValue & 0x3F00) >> 8;		
	return SUCCESS;
}

int32 rtl8306_setAsicMirrorMacAddress(uint8 *macAddr, uint32 enabled) {
	
	if (macAddr == NULL)
		return FAILED;
	if (enabled == FALSE) {
		rtl8306_setAsicPhyRegBit(6, 21, 14, 3, 0);
	} else {
		rtl8306_setAsicPhyRegBit(6, 21, 14, 3, 1);
		rtl8306_setAsicPhyReg(6, 22, 3, (macAddr[1] << 8) | macAddr[0]);
		rtl8306_setAsicPhyReg(6, 23, 3, (macAddr[3] << 8) | macAddr[2]);
		rtl8306_setAsicPhyReg(6, 24, 3, (macAddr[5] << 8) | macAddr[4]);
	}
       #ifdef RTL8306_TBLBAK
            rtl8306_TblBak.mir.enMirMac = (uint8) enabled;
            rtl8306_TblBak.mir.mir_mac[0] = macAddr[0];
            rtl8306_TblBak.mir.mir_mac[1] = macAddr[1];
            rtl8306_TblBak.mir.mir_mac[2] = macAddr[2];
            rtl8306_TblBak.mir.mir_mac[3] = macAddr[3];
            rtl8306_TblBak.mir.mir_mac[4] = macAddr[4];
            rtl8306_TblBak.mir.mir_mac[5] = macAddr[5];            
            
       #endif
	return SUCCESS;
}

int32 rtl8306_getAsicMirrorMacAddress(uint8 *macAddr, uint32 *enabled) {
	uint32 regValue;
	uint32 bitValue;
	
	if (macAddr == NULL)
		return FAILED;
	rtl8306_getAsicPhyRegBit(6, 21, 14, 3, &bitValue);
	*enabled = (bitValue == 1 ? TRUE : FALSE);
	rtl8306_getAsicPhyReg(6, 22, 3, &regValue);
	macAddr[0] = regValue & 0xFF;
	macAddr[1] = (regValue & 0xFF00) >> 8;
	rtl8306_getAsicPhyReg(6, 23, 3, &regValue);
	macAddr[2] = regValue & 0xFF;
	macAddr[3] = (regValue & 0xFF00) >> 8;
	rtl8306_getAsicPhyReg(6, 24, 3, &regValue);
	macAddr[4] = regValue & 0xFF;
	macAddr[5] = (regValue & 0xFF00) >> 8;
	return SUCCESS;
}

int32 rtl8306_setAsicLUTUnicastEntry(uint8 *macAddress, uint32 entry, uint32 age, uint32 isStatic, uint32 isAuth, uint32 port) {
	uint32 regValue, index, pollcnt;
	uint32 bitValue;
	
	if ((macAddress == NULL) || (entry > RTL8306_LUT_ENTRY3) || (age > RTL8306_LUT_AGE300) ||
		(port > RTL8306_PORT5))
		return FAILED;	
	/*For unicast entry, MAC[47] is 0  */
	if (macAddress[0] & 0x1)
		return FAILED;
	/*Enable lookup table access*/
	rtl8306_setAsicPhyRegBit(0, 16, 13, 0, 1);
	/*Write Data[55:48]*/
	if (age == RTL8306_LUT_AGE300) 
		age = 0x2;
	else if (age == RTL8306_LUT_AGE200)
		age = 0x3;
	else if (age == RTL8306_LUT_AGE100)
		age = 0x1;
	else 
		age = 0;

	regValue = ((isAuth == TRUE ? 1:0 ) << 7) | ((isStatic == TRUE ? 1:0) << 6) | (age << 4) | port;
	rtl8306_setAsicPhyReg(4, 17, 0, regValue & 0xFF);
	/*write Data[47:32]*/
	rtl8306_setAsicPhyReg(4, 18, 0, macAddress[5] << 8 | macAddress [4]);
	/*wrtie Data[31:16]*/
	rtl8306_setAsicPhyReg(4, 19, 0, macAddress[3] << 8 | macAddress [2]);
	/*wrtie Data[15:0]*/	
	rtl8306_setAsicPhyReg(4, 20, 0, macAddress[1] << 8 | macAddress [0]);	
	/*LUT index : MAC[13:15] + MAC[0:5]*/
	index = ((macAddress[4] & 0x7) << 6) | ((macAddress[5] & 0xFC) >> 2);
	/*Write Command, 2-bit indicating four-way lies in highest bit of Entry_Addr[10:0]*/
	regValue = (entry << 13) | (index << 4) | 0x2;	
	rtl8306_setAsicPhyReg(4, 16, 0, regValue);
	/*Waiting for write command done and prevent polling dead loop*/
	for (pollcnt = 0; pollcnt < RTL8306_IDLE_TIMEOUT; pollcnt ++) {
		rtl8306_getAsicPhyRegBit(4, 16, 1, 0, &bitValue);
		if (!bitValue)
			break;
	}
	if (pollcnt == RTL8306_IDLE_TIMEOUT)
		return FAILED;
	/*Disable lookup table access*/
	rtl8306_setAsicPhyRegBit(0, 16, 13, 0, 0);
	return SUCCESS;
}

int32 rtl8306_getAsicLUTUnicastEntry(uint8 *macAddress, uint32 entryAddr, uint32 *age, uint32 *isStatic, uint32 *isAuth, uint32 *port) {
	uint32 entryAddrHd;
	uint32 regValue, pollcnt;
	uint32 bitValue;
	
	if ((macAddress == NULL) || (entryAddr > 0x7FF) || (age == NULL) || (isStatic == NULL) ||
		(isAuth == NULL) || (port == NULL))
		return FAILED;
	/*Hardware data format, four-way info is the highest 2 bits of 11-bit entry info*/
	entryAddrHd = (entryAddr >> 2) | ((entryAddr & 0x3) << 9);	
	/*Enable lookup table access*/
	rtl8306_setAsicPhyRegBit(0, 16, 13, 0, 1);
	/*Read Command*/
	regValue = (entryAddrHd << 4) | 0x3;
	rtl8306_setAsicPhyReg(4, 16, 0, regValue);
	/*Waiting for Read command done and prevent polling dead loop*/
	for (pollcnt = 0; pollcnt < RTL8306_IDLE_TIMEOUT; pollcnt ++) {
		rtl8306_getAsicPhyRegBit(4, 16, 1, 0, &bitValue);
		if (!bitValue)
			break;
	}
	if (pollcnt == RTL8306_IDLE_TIMEOUT)
		return FAILED;
	/*Read Data[55:48]*/
	rtl8306_getAsicPhyReg(4, 17, 0, &regValue);
	*isAuth = (regValue & 0x80) ? TRUE: FALSE;
	*isStatic = (regValue & 0x40) ? TRUE:FALSE;
	*age = (regValue & 0x30) >> 4;
	if (*age == 0x2) 
		*age = RTL8306_LUT_AGE300;
	else if (*age == 0x3)
		*age = RTL8306_LUT_AGE200;
	else if (*age == 0x1 )
		*age = RTL8306_LUT_AGE100;
	else 
		*age = RTL8306_LUT_AGEOUT;	
	*port = regValue & 0x7;
	/*Read Data[47:32]*/
	rtl8306_getAsicPhyReg(4, 18, 0, &regValue);
	macAddress[5] = ((regValue & 0x300) >> 8) | (entryAddr & 0xFC);
	macAddress[4] = (regValue & 0xF8) | ((entryAddr >> 8) & 0x7);
	/*Read Data[31:16]*/
	rtl8306_getAsicPhyReg(4, 19, 0, &regValue);
	macAddress[3] = (regValue & 0xFF00) >> 8;
	macAddress[2] = regValue & 0xFF;
	/*Read Data[15:0]*/	
	rtl8306_getAsicPhyReg(4, 20, 0, &regValue);
	macAddress[1] = (regValue & 0xFF00) >> 8;
	macAddress[0] = regValue & 0xFF;
	
	/*Disable lookup table access*/
	rtl8306_setAsicPhyRegBit(0, 16, 13, 0, 0);
	return SUCCESS;
}

int32 rtl8306_setAsicLUTMulticastEntry(uint8 *macAddress, uint32 entry, uint32 isAuth, uint32 portMask) {
	uint32 regValue, index, pollcnt;
	uint32 bitValue;

	if ((macAddress == NULL) || (entry > RTL8306_LUT_ENTRY3) || (portMask > 0x3F ))
		return FAILED;
	/*For Muticast entry, MAC[47] is 1  */
	if (!(macAddress[0] & 0x1))
		return FAILED;
	/*Enalbe Lookup table access*/
	rtl8306_setAsicPhyRegBit(0, 16, 13, 0, 1);
	/*Write Data[55:48]*/
	/*Multicast entry portmask bits is Data[54:52], Data[50:48]*/
	regValue = ((isAuth == TRUE ? 1: 0) << 7) | (portMask & 0x38) << 1 | (portMask & 0x7); 
	rtl8306_setAsicPhyReg(4, 17, 0, regValue);
	/*Write Data[47:32]*/
	rtl8306_setAsicPhyReg(4, 18, 0, (macAddress[5] << 8) |macAddress[4]);
	/*Write Data[31:16]*/
	rtl8306_setAsicPhyReg(4, 19, 0, (macAddress[3] << 8) |macAddress[2]);
	/*Write Data[15:0]*/
	rtl8306_setAsicPhyReg(4, 20, 0, (macAddress[1] << 8) |macAddress[0]);	
	/*LUT index : MAC[13:15] + MAC[0:5]*/	
	index = ((macAddress[4] & 0x7) <<6)  |  ((macAddress[5] & 0xFC) >> 2) ;
	/*Write Command, 2-bit indicating four-way lies in highest bit of Entry_Addr[10:0]*/
	regValue = (entry << 13) | (index << 4) | 0x2;		
	rtl8306_setAsicPhyReg(4, 16, 0, regValue);
	/*Waiting for write command done and prevent polling dead loop*/
	for (pollcnt = 0; pollcnt < RTL8306_IDLE_TIMEOUT; pollcnt ++) {
		rtl8306_getAsicPhyRegBit(4, 16, 1, 0, &bitValue);
		if (!bitValue)
			break;
	}
	if (pollcnt == RTL8306_IDLE_TIMEOUT)
		return FAILED;	
	/*Disable Lookup table access*/
	rtl8306_setAsicPhyRegBit(0, 16, 13, 0, 0);				
	return SUCCESS;
}

int32 rtl8306_getAsicLUTMulticastEntry(uint8 *macAddress, uint32 entryAddr, uint32 *isAuth, uint32 *portMask) {
	uint32 entryAddrHd;
	uint32 regValue, pollcnt;	
	uint32 bitValue;
	
	if ((macAddress == NULL) || (entryAddr > 0x7FF) ||(isAuth == NULL) || (portMask == NULL))
		return FAILED;		
	/*Hardware data format, four-way info is the highest 2 bits of 11-bit entry info*/
	//entryAddrHd = (entryAddr >> 9) | ((entryAddr & 0x3) << 9);
	entryAddrHd = (entryAddr >> 2) | ((entryAddr & 0x3) << 9);	
	/*Enalbe Lookup table access*/
	rtl8306_setAsicPhyRegBit(0, 16, 13, 0, 1);
	/*Write Command*/
	regValue = (entryAddrHd << 4) | 0x3; 	
	rtl8306_setAsicPhyReg(4, 16, 0, regValue);
	/*Waiting for Read command done and prevent polling dead loop*/
	for (pollcnt = 0; pollcnt < RTL8306_IDLE_TIMEOUT; pollcnt ++) {
		rtl8306_getAsicPhyRegBit(4, 16, 1, 0, &bitValue);
		if (!bitValue)
			break;
	}
	if (pollcnt == RTL8306_IDLE_TIMEOUT)
		return FAILED;
	/*Read Data[55:48]*/		
	rtl8306_getAsicPhyReg(4, 17, 0, &regValue);
	*isAuth = (regValue & 0x80 ? 1:0);
	/*Multicast entry portmask bits is Data[54:52], Data[50:48]*/
	*portMask = ((regValue & 0x70) >> 4) << 3 | (regValue & 0x7);
	/*Read Data[47:32]*/
	rtl8306_getAsicPhyReg(4, 18, 0, &regValue);
	macAddress[5] = ((regValue & 0x300) >> 8) | (entryAddr & 0xFC);
	macAddress[4] = (regValue & 0xF8) | ((entryAddr >> 8) & 0x7);
	/*Read Data[31:16]*/
	rtl8306_getAsicPhyReg(4, 19, 0, &regValue);
	macAddress[3] = (regValue & 0xFF00) >> 8;
	macAddress[2] = regValue & 0xFF;
	/*Read Data[15:0]*/
	rtl8306_getAsicPhyReg(4, 20, 0, &regValue);
	macAddress[1] = (regValue & 0xFF00) >> 8;
	macAddress[0] = regValue & 0xFF;
	return SUCCESS;	
	
}

int32 rtl8306_setAsicLUTLRU(uint32 enabled) {
	
	rtl8306_setAsicPhyRegBit(2, 22, 8, 3, enabled == TRUE ? 1 : 0);
	return SUCCESS;
}

int32 rtl8306_getAsicLUTLRU(uint32 *enabled) {

	if (enabled == NULL)
		return FAILED;
	rtl8306_getAsicPhyRegBit(2, 22, 8, 3, enabled);
	return SUCCESS;
}

int32  rtl8306_addMuticastMacAddress(uint8 *macAddr,uint32 isAuth, uint32 portMask, uint32 *entryaddr) {
#if 0
	uint32  i, port;
	uint8 macAddr1[6];
	uint32 index, entryaddr1, age;
	uint32 isStatic, isAuth1;
	uint32 isFull = TRUE;


	if ((macAddr == NULL) || (!(macAddr[0] & 0x1)) || (entryaddr == NULL))
		return FAILED;
	/*Get index of MAC in lookup table*/
	index = ((macAddr[4] & 0x7) << 6) |((macAddr[5] & 0xFC) >>2) ;	
	/*
	 First scan four-ways, if the multicast entry has existed, only update the entry, that could 
	 prevent two same Mac in four-ways; if the mac was not written into entry before, then scan 
	 four-ways again, to Find an dynamic & unauthorized unicast entry which is auto learned, then  
	 replace it with the multicast Mac addr. scanning sequence is from entry 3 to entry 0, because priority
	   of four way is entry 3 > entry 2 > entry 1 > entry 0

	*/
	for (i = 4; i >= 1; i--) {
		entryaddr1 = (index << 2) | (i - 1);
		if (rtl8306_getAsicLUTUnicastEntry(macAddr1, entryaddr1, &age, &isStatic, &isAuth1, &port) != SUCCESS) {					
			return FAILED;
		}	
		else if ((macAddr[0] == macAddr1[0]) && (macAddr[1] == macAddr1[1]) && 
			 (macAddr[2] == macAddr1[2]) && (macAddr[3] == macAddr1[3]) &&
			 (macAddr[4] == macAddr1[4]) && (macAddr[5] == macAddr1[5])) {			
			rtl8306_setAsicLUTMulticastEntry(macAddr, i - 1, isAuth, portMask);
			*entryaddr = entryaddr1;
			isFull = FALSE;
			return SUCCESS;
		}
	}
	for (i = 4; i >= 1; i--) {
		entryaddr1 = (index << 2) | (i - 1);
		if (rtl8306_getAsicLUTUnicastEntry(macAddr1, entryaddr1, &age, &isStatic, &isAuth1, &port) != SUCCESS) {					
			return FAILED;
		}	
		else if (((macAddr1[0] & 0x1) == 0) && (isStatic == FALSE) && (isAuth1 == FALSE))  {			
			rtl8306_setAsicLUTMulticastEntry(macAddr, i - 1, isAuth, portMask);
			*entryaddr = entryaddr1;
			isFull = FALSE;
			break;
		}
	}

	
	/*If four way are all full, return RTL8306_LUT_FULL*/
	if (isFull) {
		*entryaddr = 10000;
		return RTL8306_LUT_FULL;
	}	
	return SUCCESS;
#else
	int32  i;
	uint8 macAddress[4][6];
	uint32 index, entryAddress[4], age[4];
	uint32 isStatic[4], isAuthority[4];
	uint32 port[4];
	uint32 isFull = TRUE;


	if ((macAddr == NULL) || (!(macAddr[0] & 0x1)) || (entryaddr == NULL))
		return FAILED;
	/*Get index of MAC in lookup table*/
	index = ((macAddr[4] & 0x7) << 6) |((macAddr[5] & 0xFC) >>2) ;	
	/*
	 First scan four-ways, if the multicast entry has existed, only update the entry, that could 
	 prevent two same Mac in four-ways; if the mac was not written into entry before, then scan 
	 four-ways again, to Find an dynamic & unauthorized unicast entry which is auto learned, then  
	 replace it with the multicast Mac addr. scanning sequence is from entry 3 to entry 0, because priority
	   of four way is entry 3 > entry 2 > entry 1 > entry 0

	*/
	for (i = 3; i >= 0; i--) {
		entryAddress[i] = (index << 2) | i;
		if (rtl8306_getAsicLUTUnicastEntry(macAddress[i], entryAddress[i], &age[i], &isStatic[i], &isAuthority[i], &port[i]) != SUCCESS) {					
			return FAILED;
		}	
		else if ((macAddr[0] == macAddress[i][0]) && (macAddr[1] == macAddress[i][1]) && 
			 (macAddr[2] == macAddress[i][2]) && (macAddr[3] == macAddress[i][3]) &&
			 (macAddr[4] == macAddress[i][4]) && (macAddr[5] == macAddress[i][5])) {			
			rtl8306_setAsicLUTMulticastEntry(macAddr, i, isAuth, portMask);
			*entryaddr = entryAddress[i];
			isFull = FALSE;
			return SUCCESS;
		}
	}
	for (i = 3; i >= 0; i--) {
		
		 if (((macAddress[i][0] & 0x1) == 0) && (isStatic[i] == FALSE) && (isAuthority[i] == FALSE))  {			
			rtl8306_setAsicLUTMulticastEntry(macAddr, i , isAuth, portMask);
			*entryaddr = entryAddress[i];
			isFull = FALSE;
			break;
		}
	}

	
	/*If four way are all full, return RTL8306_LUT_FULL*/
	if (isFull) {
		*entryaddr = 10000;
		return RTL8306_LUT_FULL;
	}	
	return SUCCESS;
#endif
}



int32 rtl8306_deleteMacAddress(uint8 *macAddr, uint32 *entryaddr) {
	uint32  i, port, portmask;
	uint8 macAddr1[6];
	uint32 index, entryaddr1, age;
	uint32 isStatic, isAuth;
	uint32 isHit = FALSE;
	
	if ((macAddr == NULL) || (entryaddr == NULL))
		return FAILED;
	index = ((macAddr[4] & 0x7) << 6) |((macAddr[5] & 0xFC) >>2) ;
	if (!(macAddr[0] & 0x1)) {  		/*Delete an unicast entry*/				
		for (i = 4; i >= 1; i --) {
			entryaddr1 = (index << 2) | (i - 1);
			if(rtl8306_getAsicLUTUnicastEntry(macAddr1, entryaddr1, &age, &isStatic, &isAuth, &port) == FAILED) 					
				return FAILED;
			else if (macAddr[0] ==macAddr1[0] && macAddr[1] ==macAddr1[1] 
					&& macAddr[2] ==macAddr1[2] && macAddr[3] ==macAddr1[3]
					&& macAddr[4] ==macAddr1[4] && macAddr[5] ==macAddr1[5]) {  
					rtl8306_setAsicLUTUnicastEntry(macAddr, i -1, 0, FALSE, FALSE, port);
					*entryaddr = entryaddr1;
					isHit = TRUE;
					break;			
			} 
			
		}					
	} else {							/*Delet an multicast entry*/
		for (i = 4; i >= 1; i --) {
			entryaddr1 = (index << 2) | (i - 1);
			if(rtl8306_getAsicLUTMulticastEntry(macAddr1, entryaddr1, &isAuth, &portmask) == FAILED) 					
				return FAILED;
			else if (macAddr[0] ==macAddr1[0] && macAddr[1] ==macAddr1[1] 
					&& macAddr[2] ==macAddr1[2] && macAddr[3] ==macAddr1[3]
					&& macAddr[4] ==macAddr1[4] && macAddr[5] ==macAddr1[5]) {  
					/*Turn multicast address to unicast address MAC[47] = 0*/
					macAddr[0] = macAddr[0] & 0xFE; 
					rtl8306_setAsicLUTUnicastEntry(macAddr, i -1, 0, FALSE, FALSE, 0);
					*entryaddr = entryaddr1;
					isHit = TRUE;
					break;			
			} 													
		}					
	
	}
	if (!isHit) {
		*entryaddr = 10000;
		return RTL8306_LUT_NOTEXIST;
	}
	else 
		return SUCCESS;
	
}

int32 rtl8306_setAsic1dPortState(uint32 port, uint32 state) {
	uint32 regValue;
	
	if ((port > RTL8306_PORT5) || (state > RTL8306_SPAN_FORWARD))
		return FAILED;
	/*Enable BPDU to trap to cpu, BPDU could not be flooded to all port*/
	rtl8306_setAsicPhyRegBit(2, 21, 6, 3, 1);		
	rtl8306_getAsicPhyReg(4, 21, 3, &regValue);
	regValue = (regValue & ~(0x3 << (2*port))) | (state << (2*port));
	rtl8306_setAsicPhyReg(4, 21, 3, regValue);

       #ifdef RTL8306_TBLBAK
            rtl8306_TblBak.dot1DportCtl[port] = (uint8) state;
       #endif
	return SUCCESS;
}

int32 rtl8306_getAsic1dPortState(uint32 port, uint32 *state) {
	uint32 regValue;

	if ((port > RTL8306_PORT5) || (state == NULL))
		return FAILED;
	rtl8306_getAsicPhyReg(4, 21, 3, &regValue);
	*state = (regValue & (0x3 << 2*port)) >> (2*port);	
	return SUCCESS;
}

int32 rtl8306_setAsicReservedAddressForward(uint32 addr, uint32 action) {

	if ((addr > RTL8306_RESADDR01) || (action > RTL8306_ACT_FLOOD)) 
		return FAILED;
	
	switch (addr) {
		
	case RTL8306_RESADDR21:
	case RTL8306_RESADDR20:
	case RTL8306_RESADDR10:
	case RTL8306_RESADDR03:
	case RTL8306_RESADDR00:  /*Above cases have same action*/	
		 if (action == RTL8306_ACT_FLOOD) 
			rtl8306_setAsicPhyRegBit(2, 21, addr, 3, 0);		 
		 else if (action == RTL8306_ACT_TRAPCPU)
		 	rtl8306_setAsicPhyRegBit(2, 21, addr, 3, 1);
		 else 
		 	return FAILED;
		 break;				 
	case RTL8306_RESADDR02:
		if (action == RTL8306_ACT_FLOOD) {
			rtl8306_setAsicPhyRegBit(1, 23, 4, 0, 0); 	/*Dis02FWD = 0*/
			rtl8306_setAsicPhyRegBit(2, 21, addr, 3, 0);
		} else if (action == RTL8306_ACT_TRAPCPU) {
			rtl8306_setAsicPhyRegBit(1, 23, 4, 0, 0); 	/*Dis02FWD = 0*/
			rtl8306_setAsicPhyRegBit(2, 21, addr, 3, 1);			
		} else if (action == RTL8306_ACT_DROP) 
			rtl8306_setAsicPhyRegBit(1, 23, 4, 0, 1);	/*Dis02FWD = 1*/
		else 
			return FAILED;
		 break;
	case RTL8306_RESADDRXX:
		if (action == RTL8306_ACT_FLOOD) {
			rtl8306_setAsicPhyRegBit(0, 18, 12, 0, 1); /*ENFORWARD = 1*/
			rtl8306_setAsicPhyRegBit(2, 21, addr, 3, 0);			
		} else if (action == RTL8306_ACT_TRAPCPU) {
			rtl8306_setAsicPhyRegBit(0, 18, 12, 0, 1); /*ENFORWARD = 1*/	
			rtl8306_setAsicPhyRegBit(2, 21, addr, 3, 1);								
		} else if (action == RTL8306_ACT_DROP) 
			rtl8306_setAsicPhyRegBit(0, 18, 12, 0, 0); /*ENFORWARD = 0*/	
		else 
			return FAILED;
		 break;
	case RTL8306_RESADDR01:
		if (action == RTL8306_ACT_FLOOD) 
			rtl8306_setAsicPhyRegBit(0, 22, 14, 0, 1); /*EnNECPause = 1*/
		else if (action == RTL8306_ACT_DROP)
			rtl8306_setAsicPhyRegBit(0, 22, 14, 0, 0); /*EnNECPause = 0*/
		else 
			return FAILED;							
		break;
	default :
		return FAILED;

	}	
	return SUCCESS;
}

int32 rtl8306_getAsicReservedAddressForward(uint32 addr, uint32 *action) {
	uint32 bitValue1, bitValue2;	

	if ((addr > RTL8306_RESADDR01) || (action == NULL)) 
		return FAILED;
	
	switch (addr) {		
	case RTL8306_RESADDR21:
	case RTL8306_RESADDR20:
	case RTL8306_RESADDR10:
	case RTL8306_RESADDR03:
	case RTL8306_RESADDR00:  /*Above cases have same action*/	
		rtl8306_getAsicPhyRegBit(2, 21, addr, 3, &bitValue1);
		if (bitValue1) 
			*action = RTL8306_ACT_TRAPCPU; 
		else
			*action = RTL8306_ACT_FLOOD; 
		break;				 
	case RTL8306_RESADDR02:
		rtl8306_getAsicPhyRegBit(1, 23, 4, 0, &bitValue1); 		/*Dis02FWD*/
		rtl8306_getAsicPhyRegBit(2, 21, addr, 3, &bitValue2);	
		if (bitValue1)
			*action = RTL8306_ACT_DROP;
		else if (bitValue2)
			*action = RTL8306_ACT_TRAPCPU;
		else
			*action = RTL8306_ACT_FLOOD;
		 break;
	case RTL8306_RESADDRXX:
		rtl8306_getAsicPhyRegBit(0, 18, 12, 0, &bitValue1);  /*ENFORWARD*/
		rtl8306_getAsicPhyRegBit(2, 21, addr, 3, &bitValue2);
		if (!bitValue1)
			*action = RTL8306_ACT_DROP;
		else if (bitValue2)
			*action = RTL8306_ACT_TRAPCPU;
		else
			*action = RTL8306_ACT_FLOOD;			
		 break;
	case RTL8306_RESADDR01:
		rtl8306_getAsicPhyRegBit(0, 22, 14, 0, &bitValue1); /*EnNECPause*/
		if (bitValue1)
			*action = RTL8306_ACT_FLOOD;
		else
			*action = RTL8306_ACT_DROP;
		break;
	default :
		return FAILED;
	}			
	return SUCCESS;
}

int32 rtl8306_setAsicLedStatus(uint32 port, uint32 group, uint32 status, uint32 enCPUCtl) {

	if ((port > RTL8306_PORT4) || (group > RTL8306_LED_GROUPD))
		return FAILED;
	switch(group) {
	case RTL8306_LED_GROUPA:
		if (enCPUCtl != TRUE)			
			rtl8306_setAsicPhyRegBit(2, 21, 10, 3, 0);
		else {
			rtl8306_setAsicPhyRegBit(2, 21, 10, 3, 1);
			rtl8306_setAsicPhyRegBit(3, 23, port, 3, status == TRUE ? 1:0);
		}		
		break;
	case RTL8306_LED_GROUPB:
		if (enCPUCtl != TRUE)
			rtl8306_setAsicPhyRegBit(2, 21, 9, 3, 0);
		else {
			rtl8306_setAsicPhyRegBit(2, 21, 9, 3, 1);
			rtl8306_setAsicPhyRegBit(3, 23, (5+port), 3, status == TRUE ? 1:0);		
		}		
		break;
	case RTL8306_LED_GROUPC:
		if (enCPUCtl != TRUE)
			rtl8306_setAsicPhyRegBit(2, 21, 8, 3, 0);
		else {
			rtl8306_setAsicPhyRegBit(2, 21, 8, 3, 1);			
			rtl8306_setAsicPhyRegBit(3, 24, port, 3, status == TRUE ? 1:0);							
		}
		break;
	case RTL8306_LED_GROUPD:
		if (enCPUCtl != TRUE) 			
			rtl8306_setAsicPhyRegBit(2, 21, 7, 3, 0);
		else {
			rtl8306_setAsicPhyRegBit(2, 21, 7, 3, 1);			
			rtl8306_setAsicPhyRegBit(3, 24, (5+port), 3, status == TRUE ? 1:0);						
		}
		break;
	default:
		return FAILED;			
	} 
	return SUCCESS;
}


int32 rtl8306_getAsicLedStatus(uint32 port, uint32 group, uint32 *status, uint32 *enCPUCtl) {
	uint32 bitValue1, bitValue2 ;
	
	if ((port > RTL8306_PORT4) || (group > RTL8306_LED_GROUPD) ||(status == NULL) ||
		(enCPUCtl == NULL))
		return FAILED;
	switch(group) {
	case RTL8306_LED_GROUPA:
		rtl8306_getAsicPhyRegBit(2, 21, 10, 3, &bitValue1);
		rtl8306_getAsicPhyRegBit(3, 23, port, 3, &bitValue2);
		break;
	case RTL8306_LED_GROUPB:
		rtl8306_getAsicPhyRegBit(2, 21, 9, 3, &bitValue1);	
		rtl8306_getAsicPhyRegBit(3, 23, (5+port), 3, &bitValue2);		
		break;
	case RTL8306_LED_GROUPC:
		rtl8306_getAsicPhyRegBit(2, 21, 8, 3, &bitValue1);
		rtl8306_getAsicPhyRegBit(3, 24, port, 3, &bitValue2);				
		break;
	case RTL8306_LED_GROUPD:
		rtl8306_getAsicPhyRegBit(2, 21, 7, 3, &bitValue1);	
		rtl8306_getAsicPhyRegBit(3, 24, (5+port), 3, &bitValue2);						
		break;
	default:
		return FAILED;			
	} 
	if (!bitValue1) {
		*enCPUCtl = RTL8306_LED_ASICCTL;
		*status = RTL8306_LED_OFF;
	} else  {
		*enCPUCtl = RTL8306_LED_CPUCTL;
		*status = (bitValue2 == 1 ? RTL8306_LED_ON : RTL8306_LED_OFF);		
	}	
	return SUCCESS;
}




int32 rtl8306_setAsicBicolorLedStatus(uint32 port, uint32 greenStatus, uint32 yellowStatus, uint32 enCPUCtl) {

	if (port > RTL8306_PORT4)
		return FAILED;	

	if (enCPUCtl != TRUE)		
		rtl8306_setAsicPhyRegBit(2, 21, 7, 3, 0);
	else {
		/*Enable CPU control*/
		rtl8306_setAsicPhyRegBit(2, 21, 7, 3, 1);
		if ((greenStatus == FALSE) && (yellowStatus == FALSE)) {
			rtl8306_setAsicPhyRegBit(3, 23, (5 + port), 3, 0);
			rtl8306_setAsicPhyRegBit(3, 24, (5 + port), 3, 0);
		} else if ((greenStatus == TRUE) && (yellowStatus == FALSE)) {
			rtl8306_setAsicPhyRegBit(3, 23, (5 + port), 3, 1);
			rtl8306_setAsicPhyRegBit(3, 24, (5 + port), 3, 0);						
		} else if ((greenStatus == FALSE) && (yellowStatus == TRUE)) {
			rtl8306_setAsicPhyRegBit(3, 23, (5 + port), 3, 0);
			rtl8306_setAsicPhyRegBit(3, 24, (5 + port), 3, 1);						
		} else 
			return FAILED;		
	}
	return SUCCESS;
}


int32 rtl8306_getAsicBicolorLedStatus(uint32 port, uint32 *greenStatus, uint32 *yellowStatus, uint32 *enCPUCtl) {
	uint32 bitValue1, bitValue2;
	
	if (port > RTL8306_PORT4) 
		return FAILED;
	/*Whether LED is under CPU control*/
	rtl8306_getAsicPhyRegBit(2, 21, 7, 3, &bitValue1);
	if (!bitValue1) {
		*enCPUCtl = RTL8306_LED_ASICCTL;
		*greenStatus = RTL8306_LED_OFF;
		*yellowStatus = RTL8306_LED_OFF;
		return SUCCESS;
	}
	*enCPUCtl = RTL8306_LED_CPUCTL;
	rtl8306_getAsicPhyRegBit(3, 23, (5 + port), 3, &bitValue1);
	rtl8306_getAsicPhyRegBit(3, 24, (5 + port), 3, &bitValue2);
	if (bitValue1 == bitValue2) {
		*greenStatus = RTL8306_LED_OFF;
		*yellowStatus = RTL8306_LED_OFF;		
	} else {
		*greenStatus = (bitValue1 == 1 ? RTL8306_LED_ON : RTL8306_LED_OFF);
		*yellowStatus = (bitValue2 == 1 ? RTL8306_LED_ON : RTL8306_LED_OFF); 
	}			
	return SUCCESS;
}

int32 rtl8306_setAsic1xPortBased(uint32 port, uint32 enabled, uint32 isAuth, uint32 direction) {

	if (port > RTL8306_PORT5)
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  
	if (enabled == FALSE) {
		rtl8306_setAsicPhyRegBit(port, 17, 8, 2, 0);
		return SUCCESS;
	} else {
		rtl8306_setAsicPhyRegBit(port, 17, 8, 2, 1);
		rtl8306_setAsicPhyRegBit(port, 17, 6, 2, isAuth == RTL8306_PORT_AUTH ? 1:0);
		rtl8306_setAsicPhyRegBit(port, 17, 7, 2, direction == RTL8306_PORT_BOTHDIR ? 0:1);
	}
	
	return SUCCESS;
}

int32 rtl8306_getAsic1xPortBased(uint32 port, uint32 *enabled, uint32 *isAuth, uint32 *direction) {
	uint32 bitValue;
	
	if ((port > RTL8306_PORT5) || (enabled == NULL) || (isAuth == NULL) || (direction == NULL)) 
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  
	rtl8306_getAsicPhyRegBit(port, 17, 8, 2, &bitValue);
	*enabled = (bitValue == 1 ? TRUE : FALSE);
	rtl8306_getAsicPhyRegBit(port, 17, 6, 2, &bitValue);
	*isAuth = (bitValue == 1 ? RTL8306_PORT_AUTH : RTL8306_PORT_UNAUTH);
	rtl8306_getAsicPhyRegBit(port, 17, 7, 2, &bitValue);
	*direction = (bitValue == 0 ? RTL8306_PORT_BOTHDIR : RTL8306_PORT_INDIR);	
	return SUCCESS;	
} 



int32 rtl8306_setAsic1xMacBased(uint32 port, uint32 enabled, uint32 direction) {

	if (port > RTL8306_PORT5)
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  
	if (enabled == FALSE) {
		rtl8306_setAsicPhyRegBit(port, 17, 9, 2, 0);
		return SUCCESS;
	} else {
		rtl8306_setAsicPhyRegBit(port, 17, 9, 2, 1);
		rtl8306_setAsicPhyRegBit(2, 22, 7, 3, direction == RTL8306_MAC_BOTHDIR ?  0 : 1 );
	}
	return SUCCESS;
}


int32 rtl8306_getAsic1xMacBased(uint32 port, uint32 *enabled, uint32 *direction) {
	uint32 bitValue;
	
	if ((port > RTL8306_PORT5) || (enabled == NULL) || (direction == NULL))
		return FAILED;
	
	if (port  < RTL8306_PORT5 ) 
		rtl8306_getAsicPhyRegBit(port, 17, 9, 2, &bitValue);
	else 
		rtl8306_getAsicPhyRegBit(6, 17, 9, 2, &bitValue);
	*enabled = (bitValue == 1 ? TRUE : FALSE) ; 			
	rtl8306_getAsicPhyRegBit(2, 22, 7, 3, &bitValue);
	*direction = (bitValue == 0 ? RTL8306_PORT_BOTHDIR : RTL8306_PORT_INDIR);
	return SUCCESS;
}



int32 rtl8306_setAsic1xUnauthPktAct(uint32 action) {

	if (action == RTL8306_ACT_DROP)
		rtl8306_setAsicPhyRegBit(2, 22, 5, 3, 0);
	else if (action == RTL8306_ACT_TRAPCPU)
		rtl8306_setAsicPhyRegBit(2, 22, 5, 3, 1);
	else 
		return FAILED;
	return SUCCESS;
}

int32 rtl8306_getAsic1xUnauthPktAct(uint32 *action) {
	uint32 bitValue;

	if (action == NULL)
		return FAILED;
	rtl8306_getAsicPhyRegBit(2, 22, 5, 3, &bitValue);
	*action = (bitValue == 1 ? RTL8306_ACT_TRAPCPU : RTL8306_ACT_DROP);   
	return SUCCESS;
}

int32 rtl8306_asicSoftReset(void) {
	int i;
	uint32 bitLink = 0;
	uint32 bitSPD = 0;
	uint32 bitUDP = 0;
	
	/*read port5 link status*/
	rtl8306_getAsicPhyRegBit( 6, 1, 2, 0, &bitLink);
	/*read port5 speed and duplex*/
	rtl8306_getAsicPhyRegBit(6, 0, 13, 0, &bitSPD);
	rtl8306_getAsicPhyRegBit(6, 0, 8, 0, &bitUDP);

	/*disable all mac Tx & Rx*/
	for(i = 0; i < 5 ;i++){
		rtl8306_setAsicPhyRegBit(i, 24, 10, 0, 0);
		rtl8306_setAsicPhyRegBit(i, 24, 11, 0, 0);
	}
	/*trunk port3 & port4*/
	rtl8306_setAsicPhyRegBit(0, 16, 6, 0, 0);
	rtl8306_setAsicPhyRegBit(0, 19, 11, 0, 0);

	/*disable port5*/
	rtl8306_setAsicPhyRegBit(6, 22, 15, 0, 0);	
	/*software reset*/
	rtl8306_setAsicPhyRegBit(0, 16, 12, 0, 1);
	/*disable port3 & port4 trunk*/
	rtl8306_setAsicPhyRegBit(0, 16, 6, 0, 1);
	rtl8306_setAsicPhyRegBit(0, 19, 11, 0, 1);
	/*enable all mac Tx & Rx*/
	for(i = 0; i < 5 ;i++){
		rtl8306_setAsicPhyRegBit(i, 24, 10, 0, 1);
		rtl8306_setAsicPhyRegBit(i, 24, 11, 0, 1);
	}
	/*restore port5 speed & duplex & link status*/
	rtl8306_setAsicPhyRegBit(6, 22, 15, 0, bitLink);
	rtl8306_setAsicPhyRegBit(6, 0, 13, 0, bitSPD);
	rtl8306_setAsicPhyRegBit(6, 0, 8, 0, bitUDP);
	return SUCCESS;
}

int32 rtl8306_setAsicStormFilterEnable(uint32 type, uint32 enabled) {

	switch(type) {
	case RTL8306_BROADCASTPKT:
		rtl8306_setAsicPhyRegBit(0, 18, 2, 0, enabled == TRUE ? 0:1);
		break;
	case RTL8306_MULTICASTPKT:
		rtl8306_setAsicPhyRegBit(2, 23, 9, 3, enabled == TRUE ? 0:1);		
		break;
	case RTL8306_UDAPKT:
		rtl8306_setAsicPhyRegBit(2, 23, 8, 3, enabled == TRUE ? 0:1);				
		break;
	default:
		return FAILED;		
	}		
	return SUCCESS;
}

int32 rtl8306_getAsicStormFilterEnable(uint32 type, uint32 *enabled) {
	uint32 bitValue;
	
	if (enabled == NULL)
		return FAILED;
	switch(type) {
	case RTL8306_BROADCASTPKT:
		rtl8306_getAsicPhyRegBit(0, 18, 2, 0, &bitValue);
		break;
	case RTL8306_MULTICASTPKT:
		rtl8306_getAsicPhyRegBit(2, 23, 9, 3, &bitValue);		
		break;
	case RTL8306_UDAPKT:
		rtl8306_getAsicPhyRegBit(2, 23, 8, 3, &bitValue);				
		break;
	default:
		return FAILED;		

	}	
	*enabled = (bitValue == 0 ? TRUE : FALSE);
	return SUCCESS;
}

int32 rtl8306_setAsicStormFilter(uint32 trigNum, uint32 filTime, uint32 enOtherClr, uint32 enStmInt) {
	uint32 regValue;
	
	if ( (trigNum > 3) || (filTime > 3) )
		return FAILED;
	rtl8306_getAsicPhyReg(6, 25, 0, &regValue);
	regValue &= 0xFC1F;
	regValue |= (trigNum << 8) | (filTime << 5) | ((enOtherClr == TRUE ? 0 : 1) << 7);
	rtl8306_setAsicPhyReg(6, 25, 0 , regValue);
	/*Set whether storm filter interrupt cpu*/
	rtl8306_setAsicPhyRegBit(4, 23, 7, 3, enStmInt == TRUE ? 1:0);
    	/*CPU interrupt enable when enable storm filter interrupt*/
	rtl8306_setAsicPhyRegBit(2, 22, 15, 3, enStmInt == TRUE ? 0:1);

	return SUCCESS;
}

int32 rtl8306_getAsicStormFilter(uint32* trigNum, uint32* filTime, uint32* enOtherClr, uint32* enStmInt) {
	uint32 regValue;
      uint32 regVal;

	if ((trigNum == NULL) || (filTime == NULL) || (enOtherClr == NULL) || (enStmInt == NULL))
		return FAILED;
	rtl8306_getAsicPhyReg(6, 25, 0, &regValue);
	*trigNum = (regValue >> 8) & 0x3;
	*filTime = (regValue >> 5) & 0x3;
	*enOtherClr = ((regValue >> 7) & 0x1)? FALSE:TRUE;
	rtl8306_getAsicPhyRegBit(4, 23, 7, 3, &regValue);
      rtl8306_getAsicPhyRegBit(2, 22, 15, 3, &regVal);
      *enStmInt = regValue & (~regVal);
	return SUCCESS;
}

int32 rtl8306_setAsicInputOutputDrop(uint32 type) {

	switch(type) {
	case RTL8306_INPUTDROP:
		/*Enable EnBrdDrp*/		
		rtl8306_setAsicPhyRegBit(0, 18, 13, 0, 1); 
		break;
	case RTL8306_OUTPUTDROP:
		/*Disable EnBrdDrp*/
		rtl8306_setAsicPhyRegBit(0, 18, 13, 0, 0); 		
		break;
	case RTL8306_BRO_INPUTDROP:
		/*Disable EnBrdDrp*/				
		rtl8306_setAsicPhyRegBit(0, 18, 13, 0, 0);
		/*Select Broadcast packet input drop*/
		rtl8306_setAsicPhyRegBit(2, 23, 13, 3, 1);				
		break;
	case RTL8306_BRO_OUTDROP:
		/*Disable EnBrdDrp*/						
		rtl8306_setAsicPhyRegBit(0, 18, 13, 0, 0);
		/*Select Broadcast packet output drop*/		
		rtl8306_setAsicPhyRegBit(2, 23, 13, 3, 0);						
		break;
	case RTL8306_MUL_INPUTDROP:
		/*Disable EnBrdDrp*/								
		rtl8306_setAsicPhyRegBit(0, 18, 13, 0, 0);
		/*Select Multcast packet input drop*/		
		rtl8306_setAsicPhyRegBit(2, 23, 12, 3, 1);						
		break;
	case RTL8306_MUL_OUTDROP:
		/*Disable EnBrdDrp*/										
		rtl8306_setAsicPhyRegBit(0, 18, 13, 0, 0);
		/*Select Multcast packet input drop*/				
		rtl8306_setAsicPhyRegBit(2, 23, 12, 3, 0);								
		break;
	case RTL8306_UDA_INPUTDROP:
		/*Disable EnBrdDrp*/												
		rtl8306_setAsicPhyRegBit(0, 18, 13, 0, 0);
		/*Enable drop unknown DA unicast*/		
		rtl8306_setAsicPhyRegBit(2, 23, 15, 3, 0);  
		/*Select unkown DA unicast input drop*/
		rtl8306_setAsicPhyRegBit(2, 23, 11, 3, 1);	
		break;
	case RTL8306_UDA_OUTPUTDROP:
		/*Disable EnBrdDrp*/														
		rtl8306_setAsicPhyRegBit(0, 18, 13, 0, 0);
		/*Enable drop unknown DA unicast*/				
		rtl8306_setAsicPhyRegBit(2, 23, 15, 3, 0); 
		/*Select unkown DA unicast output drop*/		
		rtl8306_setAsicPhyRegBit(2, 23, 11, 3, 0);	
		break;
	case RTL8306_UDA_DISABLEDROP:
		/*Disable drop unknown DA unicast*/		
		rtl8306_setAsicPhyRegBit(2, 23, 15, 3, 1); 
		break;
	default:
		return FAILED;		
	}			
	return SUCCESS;
}

int32 rtl8306_getAsicInputOutputDrop(uint32 pkttype, uint32 *drptype) {
	uint32 bitValue1, bitValue2, bitValue3;

	if (drptype == NULL)
		return FAILED;
	switch(pkttype) {
	case RTL8306_UNICASTPKT:
		rtl8306_getAsicPhyRegBit(0, 18, 13, 0, &bitValue1);
		*drptype = (bitValue1 == 1 ? RTL8306_INPUTDROP: RTL8306_OUTPUTDROP);
		break;
	case RTL8306_BROADCASTPKT:
		rtl8306_getAsicPhyRegBit(0, 18, 13, 0, &bitValue1);
		rtl8306_getAsicPhyRegBit(2, 23, 13, 3, &bitValue2);
		if (bitValue1 == TRUE)
			*drptype = RTL8306_INPUTDROP;
		else if (bitValue2 == TRUE)
			*drptype = RTL8306_BRO_INPUTDROP;
		else 
			*drptype = RTL8306_BRO_OUTDROP;		
		break;
	case RTL8306_MULTICASTPKT:
		rtl8306_getAsicPhyRegBit(0, 18, 13, 0, &bitValue1);
		rtl8306_getAsicPhyRegBit(2, 23, 12, 3, &bitValue2);
		if (bitValue1 == TRUE)
			*drptype = RTL8306_INPUTDROP;
		else if (bitValue2 == TRUE)
			*drptype = RTL8306_MUL_INPUTDROP;
		else 
			*drptype = RTL8306_MUL_OUTDROP;				
		break;
	case RTL8306_UDAPKT:		
		rtl8306_getAsicPhyRegBit(0, 18, 13, 0, &bitValue1);
		rtl8306_getAsicPhyRegBit(2, 23, 15, 3, &bitValue3);
		rtl8306_getAsicPhyRegBit(2, 23, 11, 3, &bitValue2);	
		if (bitValue3 == TRUE) 
			*drptype = RTL8306_UDA_DISABLEDROP;
		else if (bitValue1 == TRUE)
			*drptype = RTL8306_INPUTDROP;
		else if (bitValue2 == TRUE)
			*drptype = RTL8306_UDA_INPUTDROP;
		else
			*drptype = RTL8306_UDA_OUTPUTDROP;			
		break;
	default:
		return FAILED;		
	}			
	return SUCCESS;
}


int32 rtl8306_setAsicInterrupt(uint32 enInt, uint32 intmask) {
	uint32 regValue;
	
	if (intmask > 0xFF)
		return FAILED;

	

	if (enInt != TRUE) {
		/*CPU interrupt disable, do not change interrupt port mask*/
		rtl8306_setAsicPhyRegBit(2, 22, 15, 3, 1);
		return SUCCESS;
	} 	
	/*CPU interrupt enable*/
	rtl8306_setAsicPhyRegBit(2, 22, 15, 3, 0);
	/*Set link change interrupt mask*/
	rtl8306_getAsicPhyReg(4, 23, 3, &regValue);
	regValue = (regValue & 0xFF00) | (intmask & 0xFF);
	rtl8306_setAsicPhyReg(4, 23, 3, regValue);
	return SUCCESS;
	
}


int32 rtl8306_getAsicInterrupt(uint32 *enInt, uint32 *intmask) {
	uint32 regValue;
	uint32 bitValue;

	if ((enInt == NULL) || (intmask == NULL))
		return FAILED;
	rtl8306_getAsicPhyRegBit(2, 22, 15, 3, &bitValue);
	*enInt = (bitValue == 1 ? FALSE : TRUE);
	rtl8306_getAsicPhyReg(4, 23, 3, &regValue);
	*intmask = regValue & 0xFF;
	return SUCCESS;
}

int32 rtl8306_getAsicInterruptFlag(uint32 *intmask) {
	uint32 regValue;

	if (intmask == NULL)
		return FAILED;
	rtl8306_getAsicPhyReg(4, 22, 3, &regValue);
	*intmask = regValue & 0xFF;
	return SUCCESS;
}


int32 rtl8306_setAsicQosQueueFlowControlThr(uint32 queue, uint32 type, uint32 onoff, uint32 set, uint32 value, uint32 enabled) {
	uint32 regValue, mask;
	uint32 selection;
	uint32 reg, shift;
	
	if ((queue > RTL8306_QUEUE3) || (type > RTL8306_FCO_QLEN) || (onoff > RTL8306_FCON) || (set > RTL8306_FCO_SET1))
		return FAILED;
	selection = (set << 2) | (onoff <<1) |type;
	switch (selection) {
	case 0 : 		/*set 0, turn off, DSC*/
		if (queue == RTL8306_QUEUE0) {
			reg = 17;
			mask = 0xFFF0;
			shift = 0;
		} else if  (queue == RTL8306_QUEUE1 ) {
			reg = 17;
			mask = 0xF0FF;
			shift = 8;
		} else if (queue == RTL8306_QUEUE2 ) {
			reg = 20;
			mask = 0xFFF0;
			shift = 0;
		} else  {
			reg = 20;
			mask = 0xF0FF;
			shift = 8;
		}
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		regValue = (regValue & mask) | (value << shift);
		rtl8306_setAsicPhyReg(5, reg, 2, regValue);						
		break;
	case 1 :		/*set 0, turn off, QLEN*/
		if (queue == RTL8306_QUEUE0) {
			reg = 17;
			mask = 0xFF0F;
			shift = 4;
		} else if  (queue == RTL8306_QUEUE1 ) {
			reg = 17;
			mask = 0x0FFF;
			shift = 12;
		} else if (queue == RTL8306_QUEUE2 ) {
			reg = 20;
			mask = 0xFF0F;
			shift = 4;
		} else  {
			reg = 20;
			mask = 0x0FFF;
			shift = 12;
		}
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		regValue = (regValue & mask) | (value << shift);
		rtl8306_setAsicPhyReg(5, reg, 2, regValue);						
		break;
	case 2 :		/*set 0, turn on, DSC*/
		if (queue == RTL8306_QUEUE0) 
			reg = 18;
		else if  (queue == RTL8306_QUEUE1 ) 
			reg = 19;
		else if (queue == RTL8306_QUEUE2 )
			reg = 21;
		else  
			reg = 22;
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		regValue = (regValue & 0xFFC0) | value;
		rtl8306_setAsicPhyReg(5, reg, 2, regValue);					
		break;
	case 3:		/*set 0, turn  on, QLEN*/
		if (queue == RTL8306_QUEUE0) 
			reg = 18;
		 else if  (queue == RTL8306_QUEUE1 ) 
			reg = 19;
		 else if (queue == RTL8306_QUEUE2 ) 
			reg = 21;
		 else  
			reg = 22;	
		if (queue != RTL8306_QUEUE3)  {
			rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
			regValue = (regValue & 0xC0FF) | (value << 8);
			rtl8306_setAsicPhyReg(5, reg, 2, regValue);
		}  else {
			rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
			regValue = (regValue & 0x3FF) | (value << 10);
			rtl8306_setAsicPhyReg(5, reg, 2, regValue);			
		}	
		break;
	case 4:		/*set 1, turn off, DSC*/
		if (queue == RTL8306_QUEUE0) {
			reg = 23;
			mask = 0xFFF0;
			shift =0;
		} else if  (queue == RTL8306_QUEUE1 ) {
			reg = 23;
			mask = 0xF0FF;
			shift =8;
		} else if (queue == RTL8306_QUEUE2 ) {
			reg = 26;
			mask = 0xFFF0;
			shift =0;
		} else  {
			reg = 26;
			mask = 0xF0FF;
			shift =8;		
		}
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		regValue = (regValue & mask) | (value << shift);
		rtl8306_setAsicPhyReg(5, reg, 2, regValue);						
		break;
	case 5:		/*set 1, turn off, QLEN*/
		if (queue == RTL8306_QUEUE0) {
			reg = 23;
			mask = 0xFF0F;
			shift = 4;
		} else if  (queue == RTL8306_QUEUE1 ) {
			reg = 23;
			mask = 0x0FFF;
			shift = 12;
		} else if (queue == RTL8306_QUEUE2 ) {
			reg = 26;
			mask = 0xFF0F;
			shift = 4;
		} else  {
			reg = 26;
			mask = 0x0FFF;
			shift = 12;		
		}
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		regValue = (regValue & mask) | (value << shift);
		rtl8306_setAsicPhyReg(5, reg, 2, regValue);								
		break;
	case 6:		/*set 1, turn on, DSC*/
		if (queue == RTL8306_QUEUE0) 
			reg = 24;
		else if  (queue == RTL8306_QUEUE1 ) 
			reg =25;
		else if (queue == RTL8306_QUEUE2 ) 
			reg = 27;
		else  
			reg = 28;		
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		regValue = (regValue & 0xFFC0) | value;
		rtl8306_setAsicPhyReg(5, reg, 2, regValue);						
		break;
	case 7:		/*set 1, turn  on, QLEN*/
		if (queue == RTL8306_QUEUE0) 
			reg = 24;
		else if  (queue == RTL8306_QUEUE1 ) 
			reg =25;
		else if (queue == RTL8306_QUEUE2 ) 
			reg = 27;
		else  
			reg = 28;		
		if (queue != RTL8306_QUEUE3)  {
			rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
			regValue = (regValue & 0xC0FF) | (value << 8);
			rtl8306_setAsicPhyReg(5, reg, 2, regValue);
		}  else {
			rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
			regValue = (regValue & 0x3FF) | (value << 10);
			rtl8306_setAsicPhyReg(5, reg, 2, regValue);			
		}	
		break;
		
	default:
		return FAILED;
	}

	/*Enable/Disable Flow control of the specified queue*/
	switch (queue) {
	case RTL8306_QUEUE0:
		if (set == RTL8306_FCO_SET0)
			rtl8306_setAsicPhyRegBit(5, 22, 6, 2, enabled == FALSE ? 1:0);
		else 
			rtl8306_setAsicPhyRegBit(5, 28, 6, 2, enabled == FALSE ? 1:0);			
		break;
	case RTL8306_QUEUE1:
		if (set == RTL8306_FCO_SET0)
			rtl8306_setAsicPhyRegBit(5, 22, 7, 2, enabled == FALSE ? 1:0);
		else 
			rtl8306_setAsicPhyRegBit(5, 28, 7, 2, enabled == FALSE ? 1:0);					
		break;
	case RTL8306_QUEUE2:
		if (set == RTL8306_FCO_SET0)
			rtl8306_setAsicPhyRegBit(5, 22, 8, 2, enabled == FALSE ? 1:0);
		else 
			rtl8306_setAsicPhyRegBit(5, 28, 8, 2, enabled == FALSE ? 1:0);					
		break;
	case RTL8306_QUEUE3:
		if (set == RTL8306_FCO_SET0)
			rtl8306_setAsicPhyRegBit(5, 22, 9, 2, enabled == FALSE ? 1:0);
		else 
			rtl8306_setAsicPhyRegBit(5, 28, 9, 2, enabled == FALSE ? 1:0);					
		break;
	default:
		return FAILED;
	}			
	return SUCCESS;
}


int32 rtl8306_getAsicQosQueueFlowControlThr(uint32 queue, uint32 type, uint32 onoff, uint32 set, uint32* value, uint32* enabled) {
	uint32 regValue, mask;
	uint32 selection;
	uint32 reg, shift = 0;
	uint32 bitValue;
	
	if ((queue > RTL8306_QUEUE3) || (type > RTL8306_FCO_QLEN) || (onoff > RTL8306_FCON) 
		|| (set > RTL8306_FCO_SET1) || (value == NULL) || (enabled == NULL))
		return FAILED;
	selection = (set << 2) | (onoff <<1) |type;
	switch (selection) {
	case 0 : 		/*set 0, turn off, DSC*/
		if (queue == RTL8306_QUEUE0) {
			reg = 17;
			mask = 0xF;
			shift = 0;
		} else if  (queue == RTL8306_QUEUE1 ) {
			reg = 17;
			mask = 0x0F00;
			shift = 8;
		} else if (queue == RTL8306_QUEUE2 ) {
			reg = 20;
			mask = 0xF;
			shift = 0;
		} else  {
			reg = 20;
			mask = 0x0F00;
			shift = 8;
		}
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		*value = (regValue & mask) >> shift;		
		break;
	case 1 :		/*set 0, turn off, QLEN*/
		if (queue == RTL8306_QUEUE0) {
			reg = 17;
			mask = 0x00F0;
			shift = 4;
		} else if  (queue == RTL8306_QUEUE1 ) {
			reg = 17;
			mask = 0xF000;
			shift = 12;
		} else if (queue == RTL8306_QUEUE2 ) {
			reg = 20;
			mask = 0x00F0;
			shift = 4;
		} else  {
			reg = 20;
			mask = 0xF000;
			shift = 12;
		}
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		*value = (regValue & mask) >> shift;				
		break;
	case 2 :		/*set 0, turn on, DSC*/
		if (queue == RTL8306_QUEUE0) 
			reg = 18;
		else if  (queue == RTL8306_QUEUE1 ) 
			reg = 19;
		else if (queue == RTL8306_QUEUE2 )
			reg = 21;
		else  
			reg = 22;
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		*value = regValue & 0x3F;		
		break;
	case 3:		/*set 0, turn  on, QLEN*/
		if (queue == RTL8306_QUEUE0) 
			reg = 18;
		 else if  (queue == RTL8306_QUEUE1 ) 
			reg = 19;
		 else if (queue == RTL8306_QUEUE2 ) 
			reg = 21;
		 else  
			reg = 22;	
		 if (queue != RTL8306_QUEUE3) {
			rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
			*value = (regValue & 0x3F00) >> 8 ;	
		 } else {
			rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
			*value = (regValue & 0xFC00) >> 10;
		 }
		break;
	case 4:		/*set 1, turn off, DSC*/
		if (queue == RTL8306_QUEUE0) {
			reg = 23;
			mask = 0x000F;
			shift =0;
		} else if  (queue == RTL8306_QUEUE1 ) {
			reg = 23;
			mask = 0x0F00;
			shift =8;
		} else if (queue == RTL8306_QUEUE2 ) {
			reg = 26;
			mask = 0x000F;
			shift =0;
		} else  {
			reg = 26;
			mask = 0x0F00;
			shift =8;		
		}
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		*value = (regValue & mask) >> shift;
		break;
	case 5:		/*set 1, turn off, QLEN*/
		if (queue == RTL8306_QUEUE0) {
			reg = 23;
			mask = 0xF0;
			shift = 4;
		} else if  (queue == RTL8306_QUEUE1 ) {
			reg = 23;
			mask = 0xF000;
			shift = 12;
		} else if (queue == RTL8306_QUEUE2 ) {
			reg = 26;
			mask = 0xF0;
			shift = 4;
		} else  {
			reg = 26;
			mask = 0xF000;
			shift = 12;		
		}
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		*value = (regValue & mask) >> shift;
		break;
	case 6:		/*set 1, turn on, DSC*/
		if (queue == RTL8306_QUEUE0) 
			reg = 24;
		else if  (queue == RTL8306_QUEUE1 ) 
			reg =25;
		else if (queue == RTL8306_QUEUE2 ) 
			reg = 27;
		else  
			reg = 28;		
		rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
		*value = (regValue & 0x3F) >> shift;
		break;
	case 7:		/*set 1, turn  on, QLEN*/
		if (queue == RTL8306_QUEUE0) 
			reg = 24;
		else if  (queue == RTL8306_QUEUE1 ) 
			reg =25;
		else if (queue == RTL8306_QUEUE2 ) 
			reg = 27;
		else  
			reg = 28;		
		 if (queue != RTL8306_QUEUE3) {
			rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
			*value = (regValue & 0x3F00) >> 8 ;	
		 } else {
			rtl8306_getAsicPhyReg(5, reg, 2, &regValue);
			*value = (regValue & 0xFC00) >> 10;
		 }
		
	default:
		return FAILED;
	}
	switch (queue) {
	case RTL8306_QUEUE0:
		if (set == RTL8306_FCO_SET0)
			rtl8306_getAsicPhyRegBit(5, 22, 6, 2, &bitValue);
		else 
			rtl8306_getAsicPhyRegBit(5, 28, 6, 2, &bitValue);			
		break;
	case RTL8306_QUEUE1:
		if (set == RTL8306_FCO_SET0)
			rtl8306_getAsicPhyRegBit(5, 22, 7, 2, &bitValue);
		else 
			rtl8306_getAsicPhyRegBit(5, 28, 7, 2, &bitValue);					
		break;
	case RTL8306_QUEUE2:
		if (set == RTL8306_FCO_SET0)
			rtl8306_getAsicPhyRegBit(5, 22, 8, 2, &bitValue);
		else 
			rtl8306_getAsicPhyRegBit(5, 28, 8, 2, &bitValue);					
		break;
	case RTL8306_QUEUE3:
		if (set == RTL8306_FCO_SET0)
			rtl8306_getAsicPhyRegBit(5, 22, 9, 2, &bitValue);
		else 
			rtl8306_getAsicPhyRegBit(5, 28, 9, 2, &bitValue);					
		break;
	default:
		return FAILED;
		
	*enabled = (bitValue == 0 ? TRUE:FALSE	);
	}			
	
	return SUCCESS;	
}


int32 rtl8306_setAsicQosPortFlowControlThr(uint32 port, uint32 onthr, uint32 offthr, uint32 direction ) {
	uint32 regValue;

	if ((port > RTL8306_PORT5) || (direction > 1))
		return FAILED;
	regValue = (offthr << 8) + onthr;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  	
	if (direction == RTL8306_PORT_TX) 
		rtl8306_setAsicPhyReg(port, 20, 2, regValue);
	else 
		rtl8306_setAsicPhyReg(port, 19, 3, regValue);
	return SUCCESS;
}

int32 rtl8306_getAsicQosPortFLowControlThr(uint32 port, uint32 *onthr, uint32 *offthr, uint32 direction) {
	uint32 regValue;

	if ((port > RTL8306_PORT5) || (onthr == NULL) || (offthr == NULL) || (direction > 1))
		return FAILED;
	/*Port 5 corresponding PHY6*/	
	if (port == RTL8306_PORT5 )  
		port ++ ;  		
	if (direction == RTL8306_PORT_TX) 
		rtl8306_getAsicPhyReg(port, 20, 2, &regValue);	
	else 
		rtl8306_getAsicPhyReg(port, 19, 3, &regValue);	
	*onthr = regValue & 0xFF;
	*offthr = (regValue & 0xFF00) >> 8;
	return SUCCESS;
}

 int32 rtl8306_setAsicQosPortFlowControlMode(uint32 port, uint32 set) {

	if ((port > RTL8306_PORT5) || (set > RTL8306_FCO_SET1))
		return FAILED;
	if (port < RTL8306_PORT5) 
		rtl8306_setAsicPhyRegBit(port, 18, 12, 2, set);
	else 
		rtl8306_setAsicPhyRegBit(6, 18, 12, 2, set);
	return SUCCESS;
}


int32 rtl8306_getAsicQosPortFlowControlMode(uint32 port , uint32 *set) {
	
	if ((port > RTL8306_PORT5) || (set == NULL))
		return FAILED;
	if (port < RTL8306_PORT5)
		rtl8306_getAsicPhyRegBit(port, 18, 12, 2, set);
	else
		rtl8306_getAsicPhyRegBit(6, 18, 12, 2, set);
	return SUCCESS;
}

int32 rtl8306_getVendorID(uint32 *id)
{
	if (id == NULL)
		return FAILED;
	rtl8306_getAsicPhyReg(4, 31, 0, id);
	/* bit[8:9]*/
	*id = (*id & 0x300) >> 8;
	return SUCCESS;
}


int32 rtl8306_setAsicPortLearningAbility(uint32 port, uint32 enabled) {


       if (port > RTL8306_PORT5)
            return FAILED;
       if (port == RTL8306_PORT5 )
            port++;

       rtl8306_setAsicPhyRegBit(port, 24, 9, 0, enabled == TRUE ? 1:0);

       return SUCCESS;
}



int32 rtl8306_getAsicVersionInfo(asicVersionPara_t *pAsicVer)
{
    uint32 regval;

    /*get chip id*/
    rtl8306_getAsicPhyReg(4, 30, 0, &regval );
    pAsicVer->chipid = (uint16)regval;
    /*get version number*/
    rtl8306_getAsicPhyReg(4, 31, 0, &regval);
    pAsicVer->vernum = (uint8)(regval & 0xFF);
    /* bit[8:9]*/
    rtl8306_getAsicPhyReg(4, 31, 0, &regval);
    regval = (regval & 0x300) >> 8;
    if (regval  == 0 || regval ==2)
        pAsicVer->series = RTL8306_S;
    else if (regval == 1)
        pAsicVer->series = RTL8306_SD;
    else if (regval == 3)
        pAsicVer->series = RTL8306_SDM;
    else 
        pAsicVer->series = 0xFF;
        
    rtl8306_setAsicPhyRegBit(0, 16, 11, 0, 1);
    rtl8306_getAsicPhyReg(4, 26, 0, &regval);
    pAsicVer->revision = (regval & 0xE000) >> 13;
    rtl8306_setAsicPhyRegBit(0, 16, 11, 0, 0);

    return SUCCESS;
}
#endif
