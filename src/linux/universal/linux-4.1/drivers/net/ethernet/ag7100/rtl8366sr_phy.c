/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright © 2003 Atheros Communications, Inc.,  All Rights Reserved.
 */

/*
 * Manage the atheros ethernet PHY.
 *
 * All definitions in this file are operating system independent!
 */

#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include "ag7100_phy.h"
#include "ag7100.h"
#include "rtl8366_smi.h"


#define RTL8366S_LED_BLINK_REG						0x420
#define RTL8366S_LED_BLINKRATE_BIT				0
#define RTL8366S_LED_BLINKRATE_MSK				0x0007
#define RTL8366S_LED_INDICATED_CONF_REG			0x421

enum RTL8366S_LEDCONF{

	LEDCONF_LEDOFF=0, 		
	LEDCONF_DUPCOL,		
	LEDCONF_LINK_ACT,		
	LEDCONF_SPD1000,		
	LEDCONF_SPD100,		
	LEDCONF_SPD10,			
	LEDCONF_SPD1000ACT,	
	LEDCONF_SPD100ACT,	
	LEDCONF_SPD10ACT,		
	LEDCONF_SPD10010ACT,  
	LEDCONF_FIBER,			
	LEDCONF_FAULT,			
	LEDCONF_LINKRX,		
	LEDCONF_LINKTX,		
	LEDCONF_MASTER,		
	LEDCONF_LEDFORCE,		
};


int32 rtl8366s_setAsicLedIndicateInfoConfig(uint32 ledNo, enum RTL8366S_LEDCONF config);
int32 rtl8366s_getAsicReg(uint32 reg, uint32 *val);
int32 rtl8366s_setAsicReg(uint32 reg, uint32 value);

int32 rtl8366s_initChip(void);
int32 rtl8366s_getPHYLinkStatus(uint32 phy, uint32 *linkStatus);
int32 rtl8366s_setMac5ForceLink(enum PORTLINKSPEED speed,enum PORTLINKDUPLEXMODE duplex,uint32 link,uint32 txPause,uint32 rxPause);
int32 rtl8366s_getMac5ForceLink(enum PORTLINKSPEED* speed,enum PORTLINKDUPLEXMODE* duplex,uint32* link,uint32* txPause,uint32* rxPause);
int32 rtl8366s_setEthernetPHY(uint32 phy, phyAbility_t ability);

enum RTL8366S_MIBCOUNTER{

	IfInOctets = 0,
	EtherStatsOctets,
	EtherStatsUnderSizePkts,
	EtherFregament,
	EtherStatsPkts64Octets,
	EtherStatsPkts65to127Octets,
	EtherStatsPkts128to255Octets,
	EtherStatsPkts256to511Octets,
	EtherStatsPkts512to1023Octets,
	EtherStatsPkts1024to1518Octets,
	EtherOversizeStats,
	EtherStatsJabbers,
	IfInUcastPkts,
	EtherStatsMulticastPkts,
	EtherStatsBroadcastPkts,
	EtherStatsDropEvents,
	Dot3StatsFCSErrors,
	Dot3StatsSymbolErrors,
	Dot3InPauseFrames,
	Dot3ControlInUnknownOpcodes,
	IfOutOctets,
	Dot3StatsSingleCollisionFrames,
	Dot3StatMultipleCollisionFrames,
	Dot3sDeferredTransmissions,
	Dot3StatsLateCollisions,
	EtherStatsCollisions,
	Dot3StatsExcessiveCollisions,
	Dot3OutPauseFrames,
	Dot1dBasePortDelayExceededDiscards,
	Dot1dTpPortInDiscards,
	IfOutUcastPkts,
	IfOutMulticastPkts,
	IfOutBroadcastPkts,
	/*Device only */	
	Dot1dTpLearnEntryDiscardFlag,
	RTL8366S_MIBS_NUMBER,	
};	

//#define DEBUG_MSG(arg)
#define DEBUG_MSG(arg) printk arg


int autoNeg = 0; // Appended by jinno @ NTT-DATA-ITEC 2008/09/17

extern char *wan_speed;
/* PHY selections and access functions */
extern int autoNeg;
/*
 * Track per-PHY port information.
 */
typedef struct {    
	BOOL   isEnetPort;       /* normal enet port */
    BOOL   isPhyAlive;       /* last known state of link */
    ag7100_phy_speed_t isSpeed;  /* last known state of speed */
    int    isFdx;            /* last known state of fdx */
    int    ethUnit;          /* MAC associated with this phy port */    
} rtlPhyInfo_t;

/*
 * Per-PHY information, indexed by PHY unit number.
 */
static rtlPhyInfo_t rtlPhyInfo[] = {
    {TRUE,
     FALSE,   /* phy port 0 -- LAN port 0 */ 
     AG7100_PHY_SPEED_10T,
     FALSE,
     0//LAN
    },
    {TRUE,
     FALSE,   /* phy port 1 -- LAN port 1 */     
     AG7100_PHY_SPEED_10T,
     FALSE,
     0
    },
    {TRUE,
     FALSE,   /* phy port 2 -- LAN port 2 */
     AG7100_PHY_SPEED_10T,
     FALSE,
     0
    },
    {TRUE,
     FALSE,   /* phy port 3 -- LAN port 3 */
     AG7100_PHY_SPEED_10T,
     FALSE,
     0
    },
    {TRUE,
     FALSE,   /* phy port 4 -- WAN port or LAN port 4 */
     AG7100_PHY_SPEED_10T,
     FALSE,
     1//wan     
    },    
    { FALSE,
      FALSE, /* phy port 5 -- CPU port (no RJ45 connector) */     
     AG7100_PHY_SPEED_10T,
     FALSE,
     2//cpu
    },
};

/* Convenience macros to access myPhyInfo */
#define RTL_IS_ENET_PORT(phyUnit) (rtlPhyInfo[phyUnit].isEnetPort)
#define RTL_IS_PHY_ALIVE(phyUnit) (rtlPhyInfo[phyUnit].isPhyAlive)
#define RTL_ETHUNIT(phyUnit) (rtlPhyInfo[phyUnit].ethUnit)

#define RTL_IS_ETHUNIT(phyUnit, ethUnit) \
            (RTL_IS_ENET_PORT(phyUnit) &&        \
            RTL_ETHUNIT(phyUnit) == (ethUnit))

#if 1 /* 2008/11/28 BUFFALO: fix WAN portlink down issue */

#define MAX_PHY_RETRY 3 
#define DELAY_PHY_RETRY 30 
uint32_t rtl8366s_getAsicPHYRegs( uint32_t phyNo, uint32_t page, uint32_t addr, uint32_t *data);

uint32_t rtl8366s_getAsicPHYRegsRetry( uint32_t phyNo, uint32_t page, uint32_t addr, uint32_t *data){
	int i;
	uint32_t status;
	for(i = 0; i < MAX_PHY_RETRY; i++){
		status = rtl8366s_getAsicPHYRegs(phyNo, page, addr, data);
		if( status == SUCCESS){
			break;
		}else{
			//DEBUG_MSG(("%s: Retry count = %d \n",__FUNCTION__, i));
		}
		udelay(DELAY_PHY_RETRY);
	}
	return status;
}

#endif


/******************************************************************************
*
* rtl8366sr_phy_is_link_alive - test to see if the specified link is alive
*
* RETURNS:
*    TRUE  --> link is alive
*    FALSE --> link is down
*/

BOOL rtl8366sr_phy_is_link_alive(int phyUnit)
{
    uint32_t regData=1;
    
	//DEBUG_MSG(("rtl8366sr_phy_is_link_alive %d\n",phyUnit));
 	rtl8366s_getPHYLinkStatus(phyUnit,&regData);
    	
    if(regData)
    	return TRUE;
    else
    	return FALSE;
}


/******************************************************************************
*
* rtl8366sr_phy_setup - reset and setup the PHY associated with
* the specified MAC unit number.
*
* Resets the associated PHY port.
*
* RETURNS:
*    TRUE  --> associated PHY is alive
*    FALSE --> no LINKs on this ethernet unit
*/

/******************************************************************************
*
* athrs26_phy_setup - reset and setup the PHY associated with
* the specified MAC unit number.
*
* Resets the associated PHY port.
*
* RETURNS:
*    TRUE  --> associated PHY is alive
*    FALSE --> no LINKs on this ethernet unit
*/
static int init_first = 0;
BOOL
rtl8366sr_phy_setup(int ethUnit)
{
    int     phyUnit;
    uint32_t  phyHwStatus, checkvalue;
//    uint16_t  timeout;
    int     liveLinks = 0;    
//    BOOL    foundPhy = FALSE;
    rtlPhyInfo_t *lastStatus;
    enum PORTLINKSPEED speed=0;
    enum PORTLINKDUPLEXMODE duplex=0;
    uint32_t link=0,txPause=0,rxPause=0; // ,autoNegotiation=0; 
//    phyAbility_t pability;
    
    /* See if there's any configuration data for this enet */
    /* start auto negogiation on each phy */
    printk("rtl8366sr_phy_setup  ethUnit=%x\n", ethUnit);     
    

#if 1//swith init    
    if(!init_first){
    	//smi_reset();
    	smi_init();
    	switch_reg_read(0x6,&checkvalue);
    	printk(KERN_EMERG "rtl8366sr register 0x6 state = %08X\n",checkvalue);
#ifdef CONFIG_DIR825
    	switch_reg_write(0x6,0x108);
#endif
    
		switch_reg_read(RTL8366S_CHIP_ID_REG,&phyHwStatus);
		switch_reg_read(RTL8366S_CHIP_ID_REG,&phyHwStatus);
		DEBUG_MSG(("Realtek 8366SR switch ID 0x%x\n", phyHwStatus));//0x8366
#if 1		
		rtl8366s_initChip();	
		mdelay(4000);
#endif
	
#ifdef CPU_PORT	
	/* Set port 5 noTag and don't dropUnda */	
		rtl8366s_setCPUPort(PORT5, 1, 0);	
#ifdef GET_INFO
		rtl8366s_getCPUPort(&speed,&duplex,&link);
#endif		
#endif		
	/* Enable CPU port - forced speed, full duplex and flow control */
#ifdef CFG_SP1000
		DEBUG_MSG(("SP1000\n"));	
		rtl8366s_setMac5ForceLink(SPD_1000M,FULL_DUPLEX,1,1,1);	
#ifdef GET_INFO
		rtl8366s_getMac5ForceLink(&speed,&duplex,&link,&txPause,&rxPause);	
#endif	
#else //100
		DEBUG_MSG(("SP100\n"));	
		rtl8366s_setMac5ForceLink(SPD_100M,FULL_DUPLEX,1,1,1);
#ifdef GET_INFO
		rtl8366s_getMac5ForceLink(&speed,&duplex,&link,&txPause,&rxPause);	
#endif	
#endif	
#ifndef CONFIG_BUFFALO  // { append by BUFFALO 2008.09.19
#endif //CONFIG_BUFFALO // } append by BUFFALO 2008.09.19
		init_first =1;
	}
#endif

#if 0	
    for (phyUnit=0; phyUnit < REALTEK_MAX_PORT_ID; phyUnit++) {
        if (!RTL_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        } 
        
        foundPhy = TRUE;       
       
 		if (phyUnit != 4){//lan 
 			DEBUG_MSG(("rtl8366sr_phy_setup: phyUnit=%x auto ability is all\n",phyUnit));      
#if 1
#if 0//lan			
 			pability.AutoNegotiation=1;
			pability.Half_10=1;		
			pability.Full_10=1;	
			pability.Half_100=1;		
			pability.Full_100=1;		
			pability.Full_1000=1;	
			pability.FC=1;			
			pability.AsyFC=1;
			//DEBUG_MSG(("rtl8366sr_phy_setup: pability auto=%x,h10=%x,f10=%x,h100=%x,\n",pability.AutoNegotiation,pability.Half_10,pability.Full_10,pability.Half_100));
			//DEBUG_MSG(("rtl8366sr_phy_setup: pability f100=%x,f1000=%x,fc=%x,asfc=%x\n",pability.Full_100,pability.Full_1000,pability.FC,pability.AsyFC)); 	
			/* ability all */	
       		rtl8366s_setEthernetPHY(phyUnit,pability);
#else
			rtl8366s_setAsicPHYRegs(phyUnit,0, MII_CONTROL_REG, (1<<MII_CONTROL_RENEG)|(1<<MII_CONTROL_AUTONEG));
        	//rtl8366s_getAsicPHYRegs(phyUnit,0,MII_CONTROL_REG,&phyHwStatus);       
        	//while ( (phyHwStatus & (1<<MII_CONTROL_RESET)) || (!(phyHwStatus & (1<<5))) ) {}	   		     
#endif 
#endif      		 	
    	}else{
        	if(wan_speed == NULL  || !strcmp(wan_speed, "auto")){
	      		DEBUG_MSG(("rtl8366sr_phy_setup: phyUnit=%x auto ability is all\n",phyUnit));
#if 1
#if 0//wan
        		pability.AutoNegotiation=1;
				pability.Half_10=1;		
				pability.Full_10=1;	
				pability.Half_100=1;		
				pability.Full_100=1;		
				pability.Full_1000=1;	
				pability.FC=1;			
				pability.AsyFC=1;
				//DEBUG_MSG(("rtl8366sr_phy_setup: pability auto=%x,h10=%x,f10=%x,h100=%x,\n",pability.AutoNegotiation,pability.Half_10,pability.Full_10,pability.Half_100));
				//DEBUG_MSG(("rtl8366sr_phy_setup: pability f100=%x,f1000=%x,fc=%x,asfc=%x\n",pability.Full_100,pability.Full_1000,pability.FC,pability.AsyFC)); 		
				/* ability all */	
       			rtl8366s_setEthernetPHY(phyUnit,pability); 
#else       			
       			rtl8366s_setAsicPHYRegs(phyUnit,0, MII_CONTROL_REG, (1<<MII_CONTROL_RENEG)|(1<<MII_CONTROL_AUTONEG));
        		//rtl8366s_getAsicPHYRegs(phyUnit,0,MII_CONTROL_REG,&phyHwStatus);       
        		//while ( (phyHwStatus & (1<<MII_CONTROL_RESET)) || (!(phyHwStatus & (1<<5))) ) {}	   		
#endif       			
#endif
       		}else if(!strcmp(wan_speed, "giga")){
       			DEBUG_MSG(("rtl8366sr_phy_setup: phyUnit=%x auto ability is 1000 full\n",phyUnit));
        	    pability.AutoNegotiation=1;
				pability.Half_10=0;		
				pability.Full_10=0;	
				pability.Half_100=0;		
				pability.Full_100=0;		
				pability.Full_1000=1;	
				pability.FC=1;			
				pability.AsyFC=1;
				//DEBUG_MSG(("rtl8366sr_phy_setup: pability auto=%x,h10=%x,f10=%x,h100=%x,\n",pability.AutoNegotiation,pability.Half_10,pability.Full_10,pability.Half_100));
				//DEBUG_MSG(("rtl8366sr_phy_setup: pability f100=%x,f1000=%x,fc=%x,asfc=%x\n",pability.Full_100,pability.Full_1000,pability.FC,pability.AsyFC)); 						
       			rtl8366s_setEthernetPHY(phyUnit,pability);  	       		
       		}else if(!strcmp(wan_speed, "100full"))	{
       			DEBUG_MSG(("rtl8366sr_phy_setup: phyUnit=%x auto ability is 100 full\n",phyUnit));
        	    pability.AutoNegotiation=1;
				pability.Half_10=0;		
				pability.Full_10=0;	
				pability.Half_100=0;		
				pability.Full_100=1;		
				pability.Full_1000=0;	
				pability.FC=1;			
				pability.AsyFC=1;	
				//DEBUG_MSG(("rtl8366sr_phy_setup: pability auto=%x,h10=%x,f10=%x,h100=%x,\n",pability.AutoNegotiation,pability.Half_10,pability.Full_10,pability.Half_100));
				//DEBUG_MSG(("rtl8366sr_phy_setup: pability f100=%x,f1000=%x,fc=%x,asfc=%x\n",pability.Full_100,pability.Full_1000,pability.FC,pability.AsyFC)); 					
       			rtl8366s_setEthernetPHY(phyUnit,pability);   
        	}else if(!strcmp(wan_speed, "10full")) {
        		DEBUG_MSG(("rtl8366sr_phy_setup: phyUnit=%x auto ability is 10 full\n",phyUnit));
       	   	   	pability.AutoNegotiation=1;
				pability.Half_10=0;		
				pability.Full_10=1;	
				pability.Half_100=0;		
				pability.Full_100=0;		
				pability.Full_1000=0;	
				pability.FC=1;			
				pability.AsyFC=1;
				//DEBUG_MSG(("rtl8366sr_phy_setup: pability auto=%x,h10=%x,f10=%x,h100=%x,\n",pability.AutoNegotiation,pability.Half_10,pability.Full_10,pability.Half_100);
				//DEBUG_MSG(("rtl8366sr_phy_setup: pability f100=%x,f1000=%x,fc=%x,asfc=%x\n",pability.Full_100,pability.Full_1000,pability.FC,pability.AsyFC)); 						
       			rtl8366s_setEthernetPHY(phyUnit,pability);   
       	   	}
		}		
        /* Reset PHYs*/
		//rtl8366s_setAsicPHYRegs(phyUnit,0, MII_CONTROL_REG, (1<<MII_CONTROL_RESET)|(1<<MII_CONTROL_AUTONEG));
        //rtl8366s_getAsicPHYRegs(phyUnit,0,MII_CONTROL_REG,&phyHwStatus);       
        //while ( (phyHwStatus & (1<<MII_CONTROL_RESET)) || (!(phyHwStatus & (1<<5))) ) {}
    }//end for

    if (!foundPhy) {
    	DEBUG_MSG(("rtl8366sr_phy_setup: no fhy find\n"));
        return FALSE; /* No PHY's configured for this ethUnit */
    }
   
     /*
     * After the phy is reset, it takes a little while before
     * it can respond properly.
     */
    mdelay(3000); 
//#endif    
    
    /*
     * Wait up to 3 seconds for ALL associated PHYs to finish
     * autonegotiation.  The only way we get out of here sooner is
     * if ALL PHYs are connected AND finish autonegotiation.
     */
//#if 0     
    for (phyUnit=0; phyUnit < REALTEK_MAX_PORT_ID; phyUnit++) {
        if (!RTL_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }        
 	
        timeout=20;
        for (;;) {
            rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_CONTROL_REG,&phyHwStatus);
            if (phyHwStatus & (1<<5) ) {
                DEBUG_MSG(("rtl8366sr_phy_setup: phyUnit=%x auto Success\n",phyUnit));                
                break;
            }
            if (timeout == 0) {
                DEBUG_MSG(("rtl8366sr_phy_setup: phyUnit=%x auto timeout\n",phyUnit));                         
                break;
            }
            if (--timeout == 0) {
                DEBUG_MSG(("rtl8366sr_phy_setup: phyUnit=%x auto --timeout\n",phyUnit)); 
                break;
            }
            mdelay(150);
        }
    }
    
	mdelay(2000);
#endif	
    /*
     * All PHYs have had adequate time to autonegotiate.
     * Now initialize software status.
     *
     * It's possible that some ports may take a bit longer
     * to autonegotiate; but we can't wait forever.  They'll
     * get noticed by mv_phyCheckStatusChange during regular
     * polling activities.
     */
    for (phyUnit=0; phyUnit < REALTEK_MAX_PORT_ID; phyUnit++) {
        if (!RTL_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        } 
        
		lastStatus = &rtlPhyInfo[phyUnit];
		
        if (rtl8366sr_phy_is_link_alive(phyUnit)) {
            liveLinks++;
            DEBUG_MSG(("phyUnit=%x is link\n", phyUnit));
            lastStatus->isPhyAlive = TRUE;
        } else {
        	DEBUG_MSG(("phyUnit=%x is lostlink\n", phyUnit));
            lastStatus->isPhyAlive = FALSE;
        }
#if 0        
		rtl8366s_getAsicPortLinkState(phyUnit,&speed,&duplex,&link,&txPause,&rxPause,&autoNegotiation);
        DEBUG_MSG(("PHY port[%d] init state:", phyUnit));
        rtl8366s_getAsicPHYRegs(phyUnit,0,MII_CONTROL_REG,&phyHwStatus);
        DEBUG_MSG(("\nBasic Control: REG[0] = 0x%hx",phyHwStatus));
        rtl8366s_getAsicPHYRegs(phyUnit,0,MII_STATUS_REG,&phyHwStatus);
        DEBUG_MSG(("\nBasic Status:  REG[1] = 0x%hx",phyHwStatus));
        rtl8366s_getAsicPHYRegs(phyUnit,0,MII_PHY_ID0,&phyHwStatus);
        DEBUG_MSG(("\nPHY ID  :      REG[2] = 0x%hx",phyHwStatus));
        rtl8366s_getAsicPHYRegs(phyUnit,0,MII_PHY_ID1,&phyHwStatus);
        DEBUG_MSG(("\n               REG[3] = 0x%hx",phyHwStatus));
        rtl8366s_getAsicPHYRegs(phyUnit,0,MII_LOCAL_CAP,&phyHwStatus);
        DEBUG_MSG(("\nAutoNeg Local: REG[4] = 0x%hx",phyHwStatus));
        rtl8366s_getAsicPHYRegs(phyUnit,0,MII_REMOTE_CAP,&phyHwStatus);
        DEBUG_MSG(("\n       Remote: REG[5] = 0x%hx",phyHwStatus));
        rtl8366s_getAsicPHYRegs(phyUnit,0,MII_GIGA_CONTROL,&phyHwStatus);
        DEBUG_MSG(("\nGiga Control:  REG[9] = 0x%hx",phyHwStatus));
        rtl8366s_getAsicPHYRegs(phyUnit,0,MII_GIGA_STATUS,&phyHwStatus);
        DEBUG_MSG(("\n     Status:   REG[A] = 0x%hx\n",phyHwStatus));   
#endif
    }
	
#if 0//GET_INFO
    switch_reg_read(0x60,&phyHwStatus);
    DEBUG_MSG(("0x60=0x%x\n", phyHwStatus));
    switch_reg_read(0x61,&phyHwStatus);
    DEBUG_MSG(("0x61=0x%x\n", phyHwStatus));
    switch_reg_read(0x62,&phyHwStatus);
    DEBUG_MSG(("0x62=0x%x\n", phyHwStatus));	
#endif 	
#ifdef CONFIG_WNDR3700
printk(KERN_EMERG "init led registers (special hack for wndr3700)\n");
u32 data;
rtl8366s_getAsicReg(RTL8366S_LED_BLINK_REG,&data);
data &= ~7;
data |= 2;
rtl8366s_setAsicReg(RTL8366S_LED_BLINK_REG,data);
rtl8366s_setAsicLedIndicateInfoConfig(1,LEDCONF_SPD10010ACT);
rtl8366s_setAsicLedIndicateInfoConfig(2,LEDCONF_SPD1000ACT);
rtl8366s_getAsicReg(RTL8366S_LED_BLINK_REG,&data);
data = (data & (~(0xF << 4))) | (6 << 4);
rtl8366s_setAsicReg(RTL8366S_LED_BLINK_REG,data);
#endif
    return (liveLinks > 0);
}

void  rtl8366sr_phy_set(int phyUnit, uint16_t status)	
{
//	uint32_t phyHwStatus;
	phyAbility_t pability;
		
	DEBUG_MSG(("\n\nrtl8366sr_phy_set ethUnit=%x,status=%x\n\n", phyUnit, status));
	switch(status)
	{   	   
	   case 0x10: //VCTSETPORTSPEED_AUTO
	   		DEBUG_MSG(("rtl8366sr_phy_set: phyUnit=%x auto ability is auto\n",phyUnit));
#if 1
	   		pability.AutoNegotiation=1;
			pability.Half_10=1;		
			pability.Full_10=1;	
			pability.Half_100=1;		
			pability.Full_100=1;		
			pability.Full_1000=1;	
			pability.FC=1;			
			pability.AsyFC=1;
			//DEBUG_MSG(("rtl8366sr_phy_setup: pability auto=%x,h10=%x,f10=%x,h100=%x,\n",pability.AutoNegotiation,pability.Half_10,pability.Full_10,pability.Half_100));
			//DEBUG_MSG(("rtl8366sr_phy_setup: pability f100=%x,f1000=%x,fc=%x,asfc=%x\n",pability.Full_100,pability.Full_1000,pability.FC,pability.AsyFC)); 	
			/* ability all */	
       		rtl8366s_setEthernetPHY(phyUnit,pability);
#else
	   		rtl8366s_setAsicPHYRegs(phyUnit,0, MII_CONTROL_REG, (1<<MII_CONTROL_RENEG)|(1<<MII_CONTROL_AUTONEG));
        	//rtl8366s_getAsicPHYRegs(phyUnit,0,MII_CONTROL_REG,&phyHwStatus);       
        	//while ( (phyHwStatus & (1<<MII_CONTROL_RESET)) || (!(phyHwStatus & (1<<5))) ) {}	   		
#endif
	   break;
	   case 0x80: //VCTSETPORTSPEED_10HALF 
			DEBUG_MSG(("rtl8366sr_phy_set: phyUnit=%x auto ability is 10 half\n",phyUnit));
       	   	pability.AutoNegotiation=1;
			pability.Half_10=1;		
			pability.Full_10=0;	
			pability.Half_100=0;		
			pability.Full_100=0;		
			pability.Full_1000=0;	
			pability.FC=1;			
			pability.AsyFC=1;					
       		rtl8366s_setEthernetPHY(phyUnit,pability);   			
	   break;
	   case 0x81: //VCTSETPORTSPEED_10FULL
			DEBUG_MSG(("rtl8366sr_phy_set: phyUnit=%x auto ability is 10 full\n",phyUnit));
       	   	pability.AutoNegotiation=1;
			pability.Half_10=0;		
			pability.Full_10=1;	
			pability.Half_100=0;		
			pability.Full_100=0;		
			pability.Full_1000=0;	
			pability.FC=1;			
			pability.AsyFC=1;					
       		rtl8366s_setEthernetPHY(phyUnit,pability); 
       		//rtl8366s_setAsicPHYRegs(phyUnit,0, MII_CONTROL_REG, (1<<MII_CONTROL_RESET)|(1<<MII_CONTROL_AUTONEG));
        	//rtl8366s_getAsicPHYRegs(phyUnit,0,MII_CONTROL_REG,&phyHwStatus);       
        	//while ( (phyHwStatus & (1<<MII_CONTROL_RESET)) || (!(phyHwStatus & (1<<5))) ) {}  
	   break;
	   case 0xa0: //VCTSETPORTSPEED_100HALF
			DEBUG_MSG(("rtl8366sr_phy_set: phyUnit=%x auto ability is 100 half\n",phyUnit));
       	   	pability.AutoNegotiation=1;
			pability.Half_10=0;		
			pability.Full_10=0;	
			pability.Half_100=1;		
			pability.Full_100=0;		
			pability.Full_1000=0;	
			pability.FC=1;			
			pability.AsyFC=1;					
       		rtl8366s_setEthernetPHY(phyUnit,pability);   	
	   break;
	   case 0xa1: //VCTSETPORTSPEED_100FULL
        	DEBUG_MSG(("rtl8366sr_phy_set: phyUnit=%x auto ability is 100 full\n",phyUnit));
       	   	pability.AutoNegotiation=1;
			pability.Half_10=0;		
			pability.Full_10=0;	
			pability.Half_100=0;		
			pability.Full_100=1;		
			pability.Full_1000=0;	
			pability.FC=1;			
			pability.AsyFC=1;					
       		rtl8366s_setEthernetPHY(phyUnit,pability); 
       		//rtl8366s_setAsicPHYRegs(phyUnit,0, MII_CONTROL_REG, (1<<MII_CONTROL_RESET)|(1<<MII_CONTROL_AUTONEG));
        	//rtl8366s_getAsicPHYRegs(phyUnit,0,MII_CONTROL_REG,&phyHwStatus);       
        	//while ( (phyHwStatus & (1<<MII_CONTROL_RESET)) || (!(phyHwStatus & (1<<5))) ) {}  
	   break;
	   case 6: //VCTSETPORTSPEED_1000FULL
        	DEBUG_MSG(("rtl8366sr_phy_set: phyUnit=%x auto ability is 100 full\n",phyUnit));
       	   	pability.AutoNegotiation=1;
			pability.Half_10=0;		
			pability.Full_10=0;	
			pability.Half_100=0;		
			pability.Full_100=0;		
			pability.Full_1000=1;	
			pability.FC=1;			
			pability.AsyFC=1;					
       		rtl8366s_setEthernetPHY(phyUnit,pability); 
       		//rtl8366s_setAsicPHYRegs(phyUnit,0, MII_CONTROL_REG, (1<<MII_CONTROL_RESET)|(1<<MII_CONTROL_AUTONEG));
        	//rtl8366s_getAsicPHYRegs(phyUnit,0,MII_CONTROL_REG,&phyHwStatus);       
        	//while ( (phyHwStatus & (1<<MII_CONTROL_RESET)) || (!(phyHwStatus & (1<<5))) ) {}  
	   break;
	   default:
	   	DEBUG_MSG(("************************************wrong val =0x%x",status));
	   break;		   	   	   	
	}
    mdelay(3000);
}


/******************************************************************************
*
* rtl8366sr_phy_is_fdx - Determines whether the phy ports associated with the
* specified device are FULL or HALF duplex.
*
* RETURNS:
*    1  --> FULL
*    0 --> HALF
*/
int
rtl8366sr_phy_is_fdx(int ethUnit)
{
    int	phyUnit;  	    
#if 0
return 1;    
#else
    //DEBUG_MSG(("rtl8366sr_phy_is_fdx ethUnit=%d\n",ethUnit));      
#ifdef CFG_SP1000
	if(!ethUnit)
		return TRUE;
		
	for (phyUnit=0; phyUnit < REALTEK_MAX_PORT_ID; phyUnit++) {
        if (!RTL_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }  
        //DEBUG_MSG(("rtl8366sr_phy_is_fdx phyUnit=%x\n",phyUnit));       
        if (rtl8366sr_phy_is_link_alive(phyUnit)) {
        	//DEBUG_MSG(("phyUnit=%x is link\n",phyUnit));
			uint32_t link_status_giga;
			uint32_t local_cap,remote_cap,common_cap;
			
#if 1 /* 2008/11/28 BUFFALO: fix WAN portlink down issue */
			rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_GIGA_STATUS,&link_status_giga);
			rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_LOCAL_CAP,&local_cap);
			rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_REMOTE_CAP,&remote_cap);
#else
			rtl8366s_getAsicPHYRegs(phyUnit,0,MII_GIGA_STATUS,&link_status_giga);
			rtl8366s_getAsicPHYRegs(phyUnit,0,MII_LOCAL_CAP,&local_cap);
			rtl8366s_getAsicPHYRegs(phyUnit,0,MII_REMOTE_CAP,&remote_cap);
#endif
        	common_cap = (local_cap & remote_cap);
        	
        	if (link_status_giga & (1 << MII_GIGA_STATUS_FULL)) {//
            	return TRUE;
            	//link_speed = 1000;
        	} else if (link_status_giga & (1 << MII_GIGA_STATUS_HALF)) {
            	return FALSE;
            	//link_speed = 1000;
        	} else {
            	if (common_cap & (1 << MII_CAP_100BASE_TX_FULL)) {
                	return TRUE;
                	//link_speed = 100;
            	} else if (common_cap & (1 << MII_CAP_100BASE_TX)) {
                	return FALSE;
                	//link_speed = 100;
            	} else if (common_cap & (1 << MII_CAP_10BASE_TX_FULL)) {
                	return TRUE;
                	//link_speed = 10;
           	 	} else  {
                	return FALSE;
                	//link_speed = 10;
           		}
        	}
           
        }
        //else{
        	//DEBUG_MSG(("phyUnit=%x is lostlink\n",phyUnit));
        //}	
    }
#else  	
   	if(!ethUnit)
		return TRUE;
    
    for (phyUnit=0; phyUnit < REALTEK_MAX_PORT_ID; phyUnit++) {
        if (!RTL_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }  
               
        if (rtl8366sr_phy_is_link_alive(phyUnit)) {
        	//DEBUG_MSG(("phyUnit=%x is link\n",phyUnit));
			uint32_t link_status_giga;
			uint32_t local_cap,remote_cap,common_cap;
			
			rtl8366s_getAsicPHYRegs(phyUnit,0,MII_GIGA_STATUS,&link_status_giga);
			rtl8366s_getAsicPHYRegs(phyUnit,0,MII_LOCAL_CAP,&local_cap);
			rtl8366s_getAsicPHYRegs(phyUnit,0,MII_REMOTE_CAP,&remote_cap);
        	common_cap = (local_cap & remote_cap);
        	
        	if (link_status_giga & (1 << MII_GIGA_STATUS_FULL)) {//
            	return TRUE;
            	//link_speed = 1000;
        	} else if (link_status_giga & (1 << MII_GIGA_STATUS_HALF)) {
            	return FALSE;
            	//link_speed = 1000;
        	} else {
            	if (common_cap & (1 << MII_CAP_100BASE_TX_FULL)) {
                	return TRUE;
                	//link_speed = 100;
            	} else if (common_cap & (1 << MII_CAP_100BASE_TX)) {
                	return FALSE;
                	//link_speed = 100;
            	} else if (common_cap & (1 << MII_CAP_10BASE_TX_FULL)) {
                	return TRUE;
                	//link_speed = 10;
           	 	} else  {
                	return FALSE;
                	//link_speed = 10;
           		}
        	}
           
        }
        //else{
        	//DEBUG_MSG(("phyUnit=%x is lostlink\n",phyUnit));
        //}	
    }
#endif    
    return FALSE;
#endif
}

/******************************************************************************
*
* rtl8366sr_phy_is_fdx_ext - Determines whether the phy ports associated with the
* specified device are FULL or HALF duplex.
*
* RETURNS:
*    1  --> FULL
*    0 --> HALF
*/
int
rtl8366sr_phy_is_fdx_ext(int phyUnit)
{      	
#if 0 
return 1;
#else    
    DEBUG_MSG(("rtl8366sr_phy_is_fdx_ext phyUnit=%d\n",phyUnit));  	
    
    if (rtl8366sr_phy_is_link_alive(phyUnit)) {
    	DEBUG_MSG(("phyUnit=%x is link\n",phyUnit));
		uint32_t link_status_giga;
		uint32_t local_cap,remote_cap,common_cap;
		
#if 1 /* 2008/11/28 BUFFALO: fix WAN portlink down issue */
		rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_GIGA_STATUS,&link_status_giga);
		rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_LOCAL_CAP,&local_cap);
		rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_REMOTE_CAP,&remote_cap);
#else
		rtl8366s_getAsicPHYRegs(phyUnit,0,MII_GIGA_STATUS,&link_status_giga);
		rtl8366s_getAsicPHYRegs(phyUnit,0,MII_LOCAL_CAP,&local_cap);
		rtl8366s_getAsicPHYRegs(phyUnit,0,MII_REMOTE_CAP,&remote_cap);
#endif
    	common_cap = (local_cap & remote_cap);
    	
    	if (link_status_giga & (1 << MII_GIGA_STATUS_FULL)) {//
        	return TRUE;
        	//link_speed = 1000;
    	} else if (link_status_giga & (1 << MII_GIGA_STATUS_HALF)) {
        	return FALSE;
        	//link_speed = 1000;
    	} else {
        	if (common_cap & (1 << MII_CAP_100BASE_TX_FULL)) {
            	return TRUE;
            	//link_speed = 100;
        	} else if (common_cap & (1 << MII_CAP_100BASE_TX)) {
            	return FALSE;
            	//link_speed = 100;
        	} else if (common_cap & (1 << MII_CAP_10BASE_TX_FULL)) {
            	return TRUE;
            	//link_speed = 10;
       	 	} else  {
            	return FALSE;
            	//link_speed = 10;
       		}
    	}
       
    }
    //else{
    	//DEBUG_MSG(("phyUnit=%x is lostlink\n",phyUnit));
    //}	

    return FALSE;
#endif
}

/******************************************************************************
*
* rtl8366sr_phy_speed - Determines the speed of phy ports associated with the
* specified device.
*
* RETURNS:
*               AG7100_PHY_SPEED_10T, AG7100_PHY_SPEED_100TX;
*               AG7100_PHY_SPEED_1000T;
*/

int
rtl8366sr_phy_speed(int ethUnit)
{
#if 0
return AG7100_PHY_SPEED_1000T;
#else
   int	phyUnit;
 
   //DEBUG_MSG(("rtl8366sr_phy_speed ethUnit=%d\n",ethUnit));  	
#ifdef CFG_SP1000
	if(!ethUnit)
		return AG7100_PHY_SPEED_1000T;
		
	for (phyUnit=0; phyUnit < REALTEK_MAX_PORT_ID; phyUnit++) {
        if (!RTL_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }  
        //DEBUG_MSG(("rtl8366sr_phy_speed phyUnit=%x\n",phyUnit));      
        if (rtl8366sr_phy_is_link_alive(phyUnit)) {
        	//DEBUG_MSG(("phyUnit=%x is link\n",phyUnit));
			uint32_t link_status_giga;			
			uint32_t local_cap,remote_cap,common_cap;
#if 0
			int link_control_giga,link_extstat_giga;
#endif			
#if 1 /* 2008/11/28 BUFFALO: fix WAN portlink down issue */
			rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_GIGA_STATUS,&link_status_giga);
			rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_LOCAL_CAP,&local_cap);
			rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_REMOTE_CAP,&remote_cap);
#else
			rtl8366s_getAsicPHYRegs(phyUnit,0,MII_GIGA_STATUS,&link_status_giga);
			rtl8366s_getAsicPHYRegs(phyUnit,0,MII_LOCAL_CAP,&local_cap);
			rtl8366s_getAsicPHYRegs(phyUnit,0,MII_REMOTE_CAP,&remote_cap);
#endif
#if 0			
			rtl8366s_getAsicPHYRegs(phyUnit,0,MII_GIGA_CONTROL,&link_control_giga);
			rtl8366s_getAsicPHYRegs(phyUnit,0,MII_EXT_STATUS_REG,&link_extstat_giga);
			//DEBUG_MSG(("c=%x,g=%x,e=%x\n",link_control_giga,link_status_giga,link_extstat_giga));
#endif
        	common_cap = (local_cap & remote_cap);
        	
        	if (link_status_giga & (1 << MII_GIGA_STATUS_FULL)) {//
            	//full_duplex = TRUE;
            	return AG7100_PHY_SPEED_1000T;
        	} else if (link_status_giga & (1 << MII_GIGA_STATUS_HALF)) {
            	//full_duplex = FALSE;
            	return AG7100_PHY_SPEED_1000T;
        	} else {
            	if (common_cap & (1 << MII_CAP_100BASE_TX_FULL)) {
                	//full_duplex = TRUE;
                	return AG7100_PHY_SPEED_100TX;
            	} else if (common_cap & (1 << MII_CAP_100BASE_TX)) {
                	//full_duplex = FALSE;
                	return AG7100_PHY_SPEED_100TX;
            	} else if (common_cap & (1 << MII_CAP_10BASE_TX_FULL)) {
                	//full_duplex = TRUE;
                	return AG7100_PHY_SPEED_10T;
           	 	} else  {
                	//full_duplex = FALSE;
                	return AG7100_PHY_SPEED_10T;
           		}
        	}
           
        }
        //else{
        	//DEBUG_MSG(("phyUnit=%x is lostlink\n",phyUnit));
        //}	
    }    
#else  	

   if(!ethUnit)
		return AG7100_PHY_SPEED_100TX;
		
   for (phyUnit=0; phyUnit < REALTEK_MAX_PORT_ID; phyUnit++) {
        if (!RTL_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }  
              
        if (rtl8366sr_phy_is_link_alive(phyUnit)) {
        	//DEBUG_MSG(("phyUnit=%x is link\n",phyUnit));
			uint32_t link_status_giga;
			uint32_t local_cap,remote_cap,common_cap;
			
			rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_GIGA_STATUS,&link_status_giga);
			rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_LOCAL_CAP,&local_cap);
			rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_REMOTE_CAP,&remote_cap);
        	common_cap = (local_cap & remote_cap);
        	
        	if (link_status_giga & (1 << MII_GIGA_STATUS_FULL)) {//
            	//full_duplex = TRUE;
            	return AG7100_PHY_SPEED_1000T;
        	} else if (link_status_giga & (1 << MII_GIGA_STATUS_HALF)) {
            	//full_duplex = FALSE;
            	return AG7100_PHY_SPEED_1000T;
        	} else {
            	if (common_cap & (1 << MII_CAP_100BASE_TX_FULL)) {
                	//full_duplex = TRUE;
                	return AG7100_PHY_SPEED_100TX;
            	} else if (common_cap & (1 << MII_CAP_100BASE_TX)) {
                	//full_duplex = FALSE;
                	return AG7100_PHY_SPEED_100TX;
            	} else if (common_cap & (1 << MII_CAP_10BASE_TX_FULL)) {
                	//full_duplex = TRUE;
                	return AG7100_PHY_SPEED_10T;
           	 	} else  {
                	//full_duplex = FALSE;
                	return AG7100_PHY_SPEED_10T;
           		}
        	}
           
        }
        //else{
        	//DEBUG_MSG(("phyUnit=%x is lostlink\n",phyUnit));
        //}	
    }    
#endif  
    return AG7100_PHY_SPEED_10T;
#endif
}

/******************************************************************************
*
* rtl8366sr_phy_speed_ext - Determines the speed of phy ports associated with the
* specified device.
*
* RETURNS:
*               AG7100_PHY_SPEED_10T, AG7100_PHY_SPEED_100TX;
*               AG7100_PHY_SPEED_1000T;
*/

int
rtl8366sr_phy_speed_ext(int phyUnit)
{
#if 0
return AG7100_PHY_SPEED_1000T;
#else
   DEBUG_MSG(("rtl8366sr_phy_speed ext phyUnit=%d\n",phyUnit));  	
  	
    if (rtl8366sr_phy_is_link_alive(phyUnit)) {
    	DEBUG_MSG(("phyUnit=%x is link\n",phyUnit));
		uint32_t link_status_giga;
		uint32_t local_cap,remote_cap,common_cap;
		
#if 1 /* 2008/11/28 BUFFALO: fix WAN portlink down issue */
		rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_GIGA_STATUS,&link_status_giga);
		rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_LOCAL_CAP,&local_cap);
		rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_REMOTE_CAP,&remote_cap);
#else
		rtl8366s_getAsicPHYRegs(phyUnit,0,MII_GIGA_STATUS,&link_status_giga);
		rtl8366s_getAsicPHYRegs(phyUnit,0,MII_LOCAL_CAP,&local_cap);
		rtl8366s_getAsicPHYRegs(phyUnit,0,MII_REMOTE_CAP,&remote_cap);
#endif
    	common_cap = (local_cap & remote_cap);
    	
    	if (link_status_giga & (1 << MII_GIGA_STATUS_FULL)) {//
        	//full_duplex = TRUE;
        	return AG7100_PHY_SPEED_1000T;
    	} else if (link_status_giga & (1 << MII_GIGA_STATUS_HALF)) {
        	//full_duplex = FALSE;
        	return AG7100_PHY_SPEED_1000T;
    	} else {
        	if (common_cap & (1 << MII_CAP_100BASE_TX_FULL)) {
            	//full_duplex = TRUE;
            	return AG7100_PHY_SPEED_100TX;
        	} else if (common_cap & (1 << MII_CAP_100BASE_TX)) {
            	//full_duplex = FALSE;
            	return AG7100_PHY_SPEED_100TX;
        	} else if (common_cap & (1 << MII_CAP_10BASE_TX_FULL)) {
            	//full_duplex = TRUE;
            	return AG7100_PHY_SPEED_10T;
       	 	} else  {
            	//full_duplex = FALSE;
            	return AG7100_PHY_SPEED_10T;
       		}
    	}
       
    }
    //else{
    	//DEBUG_MSG(("phyUnit=%x is lostlink\n",phyUnit));
    //} 

    return AG7100_PHY_SPEED_10T;
#endif
}

/*****************************************************************************
*
* rtl8366sr_phy_is_up -- checks for significant changes in PHY state.
*
* A "significant change" is:
*     dropped link (e.g. ethernet cable unplugged) OR
*     autonegotiation completed + link (e.g. ethernet cable plugged in)
*
* When a PHY is plugged in, phyLinkGained is called.
* When a PHY is unplugged, phyLinkLost is called.
*/

int
rtl8366sr_phy_is_up(int ethUnit)
{
#if 0
return 1;
#else
    int           phyUnit;
    uint32_t      phyHwStatus, phyHwControl;
    rtlPhyInfo_t *lastStatus;
    int           linkCount   = 0;
    int           lostLinks   = 0;
    //int           gainedLinks = 0;    

	//DEBUG_MSG(("rtl8366sr_phy_is_up  ethUnit=%x\n", ethUnit)); 
#ifdef CFG_SP1000	
		
	for (phyUnit=0; phyUnit < REALTEK_MAX_PORT_ID; phyUnit++) {
        if (!RTL_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }  
        //DEBUG_MSG(("rtl8366sr_phy_is_up phyUnit=%x\n",phyUnit)); 
        lastStatus = &rtlPhyInfo[phyUnit];

        if (lastStatus->isPhyAlive) { /* last known link status was ALIVE */
        	//DEBUG_MSG(("rtl8366sr_phy_is_up: phyUnit=%x is alive\n", phyUnit));
#if 1 /* 2008/11/28 BUFFALO: fix WAN portlink down issue */
        	rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_STATUS_REG,&phyHwStatus);
#else
        	rtl8366s_getAsicPHYRegs(phyUnit,0,MII_STATUS_REG,&phyHwStatus);
#endif
            /* See if we've lost link */
            if (phyHwStatus & (1<<2)) {
                linkCount++;
                //DEBUG_MSG(("phyUnit=%x is link\n", phyUnit));
            } else {
                lostLinks++;
                //DEBUG_MSG(("phyUnit=%x is lostlink\n", phyUnit)); 
                lastStatus->isPhyAlive = FALSE;
            }
        } else { /* last known link status was DEAD */
            //DEBUG_MSG(("rtl8366sr_phy_is_up: phyUnit=%x is dead\n", phyUnit));
            /* Check for reset complete */
#if 1 /* 2008/11/28 BUFFALO: fix WAN portlink down issue */
            rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_CONTROL_REG,&phyHwControl);         
#else
            rtl8366s_getAsicPHYRegs(phyUnit,0,MII_CONTROL_REG,&phyHwControl);         
#endif
            if (phyHwControl & (1<<MII_CONTROL_RESET))
                continue;
#if 1 /* 2008/11/28 BUFFALO: fix WAN portlink down issue */
            rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_STATUS_REG,&phyHwStatus);
#else
            rtl8366s_getAsicPHYRegs(phyUnit,0,MII_STATUS_REG,&phyHwStatus);
#endif
//#if 1   
	  		if(autoNeg){
	  			//DEBUG_MSG(("phyUnit=%x is auto\n", phyUnit));
	  			udelay(50);         
            	/* Check for AutoNegotiation complete */            
            	if (/* !(phyHwControl & (1<<12)) || */ (phyHwStatus & (1<<5)) ) {                	
                	rtl8366s_getAsicPHYRegs(phyUnit,0,MII_STATUS_REG,&phyHwStatus);
                	
                	if (phyHwStatus & (1<<2)) {
                		//gainedLinks++;
                		linkCount++;
                		//DEBUG_MSG(("phyUnit=%x is link\n", phyUnit));
                		lastStatus->isPhyAlive = TRUE;
            		}
        		}
//#else
	   		}else{
	   			//DEBUG_MSG(("phyUnit=%x is notauto\n", phyUnit));
#if 1 /* 2008/11/28 BUFFALO: fix WAN portlink down issue */
                rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_STATUS_REG,&phyHwStatus);
#else
                rtl8366s_getAsicPHYRegs(phyUnit,0,MII_STATUS_REG,&phyHwStatus);
#endif
                if (phyHwStatus & (1<<2)) {
                	//gainedLinks++;
                	linkCount++;
                	//DEBUG_MSG(("phyUnit=%x is link\n", phyUnit));                                               
                	lastStatus->isPhyAlive = TRUE;
            	}
	   		}//end autoNeg
//#endif        
    	}//lastStatus->isPhyAlive
    }//end for	
#else			
    
    for (phyUnit=0; phyUnit < REALTEK_MAX_PORT_ID; phyUnit++) {
        if (!RTL_IS_ETHUNIT(phyUnit, ethUnit)) {
            continue;
        }  
        
        lastStatus = &rtlPhyInfo[phyUnit];

        if (lastStatus->isPhyAlive) { /* last known link status was ALIVE */
        	//DEBUG_MSG(("rtl8366sr_phy_is_up: phyUnit=%x is alive\n", phyUnit));
        	rtl8366s_getAsicPHYRegs(phyUnit,0,MII_STATUS_REG,&phyHwStatus);
			
            /* See if we've lost link */
            if (phyHwStatus & (1<<2)) {
                linkCount++;
                //DEBUG_MSG(("phyUnit=%x is link\n", phyUnit));
            } else {
                lostLinks++;
                //DEBUG_MSG(("phyUnit=%x is lostlink\n", phyUnit)); 
                lastStatus->isPhyAlive = FALSE;
            }
        } else { /* last known link status was DEAD */
            //DEBUG_MSG(("rtl8366sr_phy_is_up: phyUnit=%x is dead\n", phyUnit));
            /* Check for reset complete */
            rtl8366s_getAsicPHYRegs(phyUnit,0,MII_CONTROL_REG,&phyHwControl);         
            if (phyHwControl & (1<<MII_CONTROL_RESET))
                continue;
            rtl8366s_getAsicPHYRegs(phyUnit,0,MII_STATUS_REG,&phyHwStatus);
//#if 1   
	  		if(autoNeg){
	  			//DEBUG_MSG(("phyUnit=%x is auto\n", phyUnit));
	  			udelay(50);         
            	/* Check for AutoNegotiation complete */            
            	if (/* !(phyHwControl & (1<<12)) || */ (phyHwStatus & (1<<5)) ) {                	
                	rtl8366s_getAsicPHYRegs(phyUnit,0,MII_STATUS_REG,&phyHwStatus);
                	
                	if (phyHwStatus & (1<<2)) {
                		//gainedLinks++;
                		linkCount++;
                		//DEBUG_MSG(("phyUnit=%x is link\n", phyUnit));
                		lastStatus->isPhyAlive = TRUE;
            		}
        		}
//#else
	   		}else{
	   			//DEBUG_MSG(("phyUnit=%x is notauto\n", phyUnit));
                rtl8366s_getAsicPHYRegs(phyUnit,0,MII_STATUS_REG,&phyHwStatus);

                if (phyHwStatus & (1<<2)) {
                	//gainedLinks++;
                	linkCount++;
                	//DEBUG_MSG(("phyUnit=%x is link\n", phyUnit));                                               
                	lastStatus->isPhyAlive = TRUE;
            	}
	   		}//end autoNeg
//#endif        
    	}//lastStatus->isPhyAlive
    }//end for
#endif
    return (linkCount);
#endif
}

int
rtl_ioctl(uint32_t *args, int cmd)
{
	printk("rtl_ioctl:\n");
#ifdef FULL_FEATURE
    if (sw_ioctl(args, cmd))
        return -EOPNOTSUPP;

    return 0;
#else	
    return -EOPNOTSUPP;
#endif
}

/* 2008/11/28 BUFFALO: fix WAN portlink down issue */
int rtl8366sr_get_link_status(int unit, int *link, int *fdx, ag7100_phy_speed_t *speed)
{
	int					phyUnit;
	uint32_t			phyHwStatus, phyHwControl;
	rtlPhyInfo_t		*lastStatus = NULL;
	uint32_t			status;
	int					checkNewStatus = 0;
	int					linkStatus = 0;
	ag7100_phy_speed_t	speedStatus = AG7100_PHY_SPEED_10T;
	int					fdxStatus = FALSE;

	/* Check LinkStatus */
	for (phyUnit=0; phyUnit < REALTEK_MAX_PORT_ID; phyUnit++) {
		if (!RTL_IS_ETHUNIT(phyUnit, unit)) {
			continue;
		}
		lastStatus = &rtlPhyInfo[phyUnit];
		if(lastStatus->isPhyAlive == FALSE){
			/* Check for reset complete */
			status = rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_CONTROL_REG,&phyHwControl);
			if(status != SUCCESS){
				DEBUG_MSG(("%s: fail rtl8366s_getAsicPHYRegs(): unit=%d, port=%d: not reset complete \n",__FUNCTION__, unit, phyUnit));
				checkNewStatus = 0;
				continue;
			}
		}
		status = rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_STATUS_REG,&phyHwStatus);
		if(status != SUCCESS){
			DEBUG_MSG(("%s: fail rtl8366s_getAsicPHYRegs(): unit=%d, port=%d: ignore this port\n",__FUNCTION__, unit, phyUnit));
			checkNewStatus = 0;
			continue;
		}
		/* See if we've lost link */
		if (phyHwStatus & (1<<2)) {
			linkStatus = 1;
			checkNewStatus = 1;
			//DEBUG_MSG(("phyUnit=%x is link\n", phyUnit));
			break;
		} else {
			// linkStatus = 0;
			checkNewStatus = 1;
			//DEBUG_MSG(("phyUnit=%x is lostlink\n", phyUnit)); 
		}
	}

	if(checkNewStatus){
		if(unit == 1){
			/* case unit == 1 */
			if(linkStatus){
				/* Check from Chip register */
				uint32_t link_status_giga;			
				uint32_t local_cap,remote_cap,common_cap;
				status = rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_GIGA_STATUS,&link_status_giga);
				if(status != SUCCESS){
					checkNewStatus = 0;
				}
				status = rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_LOCAL_CAP,&local_cap);
				if(status != SUCCESS){
					checkNewStatus = 0;
				}
				status = rtl8366s_getAsicPHYRegsRetry(phyUnit,0,MII_REMOTE_CAP,&remote_cap);
				if(status != SUCCESS){
					checkNewStatus = 0;
				}
				if(checkNewStatus){
					common_cap = (local_cap & remote_cap);
					if (link_status_giga & (1 << MII_GIGA_STATUS_FULL)) {//
						fdxStatus = TRUE;
						speedStatus = AG7100_PHY_SPEED_1000T;
					} else if (link_status_giga & (1 << MII_GIGA_STATUS_HALF)) {
						fdxStatus = FALSE;
						speedStatus = AG7100_PHY_SPEED_1000T;
					} else {
						if (common_cap & (1 << MII_CAP_100BASE_TX_FULL)) {
							fdxStatus = TRUE;
							speedStatus =  AG7100_PHY_SPEED_100TX;
						} else if (common_cap & (1 << MII_CAP_100BASE_TX)) {
							fdxStatus = FALSE;
							speedStatus = AG7100_PHY_SPEED_100TX;
						} else if (common_cap & (1 << MII_CAP_10BASE_TX_FULL)) {
							fdxStatus = TRUE;
							speedStatus = AG7100_PHY_SPEED_10T;
						} else  {
							fdxStatus = FALSE;
							speedStatus = AG7100_PHY_SPEED_10T;
						}
					}
				}else{
					/* use previous status */
					fdxStatus = lastStatus->isFdx;
					speedStatus = lastStatus->isSpeed;
				}
				
			}else{
				speedStatus = AG7100_PHY_SPEED_10T;
				fdxStatus = FALSE;
			}
		}else{
			/* case unit == 0 */
			fdxStatus = TRUE;
			speedStatus = AG7100_PHY_SPEED_1000T;
		}
	} else {
		/* use previous status */
		linkStatus = lastStatus->isPhyAlive;
		fdxStatus = lastStatus->isFdx;
		speedStatus = lastStatus->isSpeed;
		DEBUG_MSG(("%s: fail rtl8366s_getAsicPHYRegs(): unit=%d use previous status(linkStatus=%d) \n",__FUNCTION__, unit, linkStatus));
	}

	for (phyUnit=0; phyUnit < REALTEK_MAX_PORT_ID; phyUnit++) {
		if (!RTL_IS_ETHUNIT(phyUnit, unit)) {
			continue;
		}
		lastStatus = &rtlPhyInfo[phyUnit];
		lastStatus->isPhyAlive = linkStatus;
		lastStatus->isFdx = fdxStatus;
		lastStatus->isSpeed = speedStatus;
	}

	*link = linkStatus;
	*fdx = fdxStatus;
	*speed = speedStatus;

    return 0;
}
