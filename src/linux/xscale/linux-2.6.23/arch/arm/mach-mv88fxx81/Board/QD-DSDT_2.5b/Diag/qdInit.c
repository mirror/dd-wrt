/*******************************************************************************
*                Copyright 2002, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
*********************************************************************************/
/* 
 * FILENAME:    $Workfile: qdInit.c $ 
 * REVISION:    $Revision: 12 $ 
 * LAST UPDATE: $Modtime: 3/03/03 12:01p $ 
 * 
 * DESCRIPTION: QD initialization module
 *     
 */
#include "mv_qd.h"
#include "mv_debug.h"
#include "mv_os.h"
#include <linux/proc_fs.h>

#define MARVELL_GIGA_BOARD 1

void qdStatus(void);


/*
 * A system configuration structure
 * It used to configure the QD driver with configuration data
 * and with platform specific implementation functions 
 */
GT_SYS_CONFIG   	cfg;

/*
 * The QD device.
 * This struct is a logical representation of the QD switch HW device.
 */
GT_QD_DEV       	qddev[4] = {{0}};

/*
 * The QD device pointer.
 * A constant pointer to the one and only QD device.
 */
GT_QD_DEV       	*qd_dev = &qddev[0];
GT_QD_DEV       	*qd_ext = &qddev[1];


/*
 * read mii register - see qdFFmii.c
 */ 
extern GT_BOOL ffReadMii(GT_QD_DEV* dev, 
		      unsigned int portNumber , 
		      unsigned int MIIReg, unsigned int* value
		      );

/*
 * write mii register - see qdFFmii.c
 */ 
extern GT_BOOL ffWriteMii(GT_QD_DEV* dev, 
		       unsigned int portNumber , 
		       unsigned int MIIReg, 
		       unsigned int value
		       );

/*
 * A phy patch for deviceId == GT_88E6063
 */
static GT_STATUS phyPatch(GT_QD_DEV *dev)
{
	GT_U32 u32Data;
	/*
	 * Set Bit2 of Register 29 of any phy
	 */
    if(gsysReadMiiReg(dev, dev->baseRegAddr,29,&u32Data) != GT_OK)
	{		
		return GT_FAIL;
	}

    if(gsysWriteMiiReg(dev, (GT_U32)dev->baseRegAddr,29,(GT_U16)(u32Data|0x4)) != GT_OK)
	{		
		return GT_FAIL;
	}

	/*
	 * ReSet Bit6 of Register 30 of any phy
	 */
    if(gsysReadMiiReg(dev,dev->baseRegAddr,30,&u32Data) != GT_OK)
	{		
		return GT_FAIL;
	}

    if(gsysWriteMiiReg(dev, (GT_U32)dev->baseRegAddr,30,(GT_U16)(u32Data&(~0x40))) != GT_OK)
	{		
		return GT_FAIL;
	}
	return GT_OK;
}

/*
*  Initialize the QuarterDeck. This should be done in BSP driver init routine.
*	Since BSP is not combined with QuarterDeck driver, we are doing here.
*/
GT_STATUS qdStart(void) /* devId is used for simulator only */
{
	GT_STATUS status;
	/*
	 *  Register all the required functions to QuarterDeck Driver.
	 */
	cfg.BSPFunctions.readMii   = ffReadMii;
	cfg.BSPFunctions.writeMii  = ffWriteMii;
#ifdef USE_SEMAPHORE
	cfg.BSPFunctions.semCreate = osSemCreate;
	cfg.BSPFunctions.semDelete = osSemDelete;
	cfg.BSPFunctions.semTake   = osSemWait;
	cfg.BSPFunctions.semGive   = osSemSignal;
#else /* USE_SEMAPHORE */
	cfg.BSPFunctions.semCreate = NULL;
	cfg.BSPFunctions.semDelete = NULL;
	cfg.BSPFunctions.semTake   = NULL;
	cfg.BSPFunctions.semGive   = NULL;
#endif /* USE_SEMAPHORE */

	cfg.initPorts = GT_TRUE;	
	cfg.cpuPortNum = GT_CPU_SWITCH_PORT;	
	qd_dev->cpuPortNum = GT_CPU_SWITCH_PORT;	
	if((status = qdLoadDriver(&cfg, qd_dev)) != GT_OK) {		
	  gtOsPrintf("qdLoadDriver is failed: status = 0x%x\n", status);
	  return status;
	}
	
	/*
	*  start the QuarterDeck
	*/
	if (qd_dev->deviceId == GT_88E6063) {
	  phyPatch(qd_dev);
	}

	/* to which VID should we set the CPU_PORT? (1 is temporary)*/
	if((status = gvlnSetPortVid(qd_dev, GT_CPU_SWITCH_PORT, 5)) != GT_OK) {
	  gtOsPrintf("gprtSetPortVid returned fail for CPU port.\n");
	  return status;
	}

#ifdef QD_TRAILER_MODE
	/* set ingress trailer mode*/
	gprtSetIngressMode(qd_dev, GT_CPU_SWITCH_PORT, GT_TRAILER_INGRESS);	
	/* set egress trailer*/
	gprtSetTrailerMode(qd_dev, GT_CPU_SWITCH_PORT, GT_TRUE);
#endif

#ifdef QD_HEADER_MODE
	if((status = gprtSetHeaderMode(qd_dev, GT_CPU_SWITCH_PORT, GT_TRUE)) != GT_OK)
	{
	  gtOsPrintf("gprtSetHeaderMode return Failed\n");
	  return status;
	}   
#endif

	return GT_OK;    
}


void qdClose(void) 
{
	if (qd_dev->devEnabled)
		qdUnloadDriver(qd_dev);
}

/* Gemtek - detect wan link status */
static int wanLink_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
       IN GT_BOOL link;

	if (mvBoardIdGet() == RD_88F5181L_VOIP_GE) {
//GT_PORT_2_LPORT
// 2 for WAN port , 1 , 0 for LAN ports
		//mvOsPrintf("Giga board.\n");
#if FOR_DUALBAND
		gprtGetPhyLinkStatus(qd_dev,5,&link);
#else
		gprtGetPhyLinkStatus(qd_dev,2,&link);
#endif
	}
	else {//MARVELL_10_100_BOARD
// 0 for WAN port, 1-4 for LAN ports
		//mvOsPrintf("FE board.\n");
		gprtGetPhyLinkStatus(qd_dev,0,&link);
	}

	if( link )	// wan port link state, phy 4, reg 1, bit 2
	{
		len = sprintf(buf,"1");
	}else{
		len = sprintf(buf,"0");
	}
	return len;
}
/* Gemtek - detect wan speed status */
static int wanSpeed_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
       IN GT_16 speed;

	if (mvBoardIdGet() == RD_88F5181L_VOIP_GE) {
//GT_PORT_2_LPORT
// 2 for WAN port , 1 , 0 for LAN ports
		//mvOsPrintf("Giga board.\n");
		gprtGetPhyStatus(qd_dev,2,NULL,&speed,NULL);
	}
	else {//MARVELL_10_100_BOARD
// 0 for WAN port, 1-4 for LAN ports
		//mvOsPrintf("FE board.\n");
		gprtGetPhyStatus(qd_dev,0,NULL,&speed,NULL);
	}
	len = sprintf(buf,"%d", speed);

	return len;
}
/* Gemtek - detect wan duplex status */
static int wanDuplex_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
       IN GT_BOOL duplex;

	if (mvBoardIdGet() == RD_88F5181L_VOIP_GE) {
//GT_PORT_2_LPORT
// 2 for WAN port , 1 , 0 for LAN ports
		//mvOsPrintf("Giga board.\n");
		gprtGetPhyStatus(qd_dev,2,NULL,NULL,&duplex);
	}
	else {//MARVELL_10_100_BOARD
// 0 for WAN port, 1-4 for LAN ports
		//mvOsPrintf("FE board.\n");
		gprtGetPhyStatus(qd_dev,0,NULL,NULL,&duplex);
	}

	if( duplex )
	{
		len = sprintf(buf,"1");
	}else{
		len = sprintf(buf,"0");
	}
	return len;
}


/* Gemtek - detect lan status */
static int lanStatus_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	int i = 0;
       IN GT_BOOL duplex[4];
       IN GT_16 speed[4];
       IN GT_BOOL link[4];
       char tmp[64]="";
	memset(duplex,0,sizeof(duplex));
	memset(speed,0,sizeof(speed));
	memset(link,0,sizeof(link));
	if (mvBoardIdGet() == RD_88F5181L_VOIP_GE) {
#if FOR_DUALBAND
		gprtGetPhyStatus(qd_dev,1,&link[0],&speed[0],&duplex[0]);
		gprtGetPhyStatus(qd_dev,0,&link[1],&speed[1],&duplex[1]);
		gprtGetPhyStatus(qd_dev,2,&link[2],&speed[2],&duplex[2]);
		gprtGetPhyStatus(qd_dev,7,&link[3],&speed[3],&duplex[3]);
#else
//GT_PORT_2_LPORT
// 2 for WAN port , 1 , 0, 5, 7 for LAN ports
		//mvOsPrintf("Giga board.\n");
		gprtGetPhyStatus(qd_dev,1,&link[0],&speed[0],&duplex[0]);
		gprtGetPhyStatus(qd_dev,0,&link[1],&speed[1],&duplex[1]);
		gprtGetPhyStatus(qd_dev,5,&link[2],&speed[2],&duplex[2]);
		gprtGetPhyStatus(qd_dev,7,&link[3],&speed[3],&duplex[3]);
#endif
	}
	else {//MARVELL_10_100_BOARD
// 0 for WAN port, 1-4 for LAN ports
		//mvOsPrintf("FE board.\n");
		gprtGetPhyStatus(qd_dev,1,&link[0],&speed[0],&duplex[0]);
		gprtGetPhyStatus(qd_dev,2,&link[1],&speed[1],&duplex[1]);
		gprtGetPhyStatus(qd_dev,3,&link[2],&speed[2],&duplex[2]);
		gprtGetPhyStatus(qd_dev,4,&link[3],&speed[3],&duplex[3]);
	}

	strcpy(buf,"port link speed duplex (speed: 0:10mb, 1:100mb, 2:1000mb. duplex: 0:half, 1:full.)\n");
	len = strlen(buf);
	for (i=0;i<4;i++) {
		sprintf(tmp,"%4d %4d %5d %6d\n",i , link[i], speed[i], duplex[i]);
		strcat(buf,tmp);
		len += strlen(tmp);
	}

	return len;
}

/* Gemtek - detect collisions */
static int collision_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	int i = 0;
       char tmp[64]="";
       GT_STATS_COUNTER_SET2 counter_set[5];
	memset(counter_set,0,sizeof(counter_set));
	if (mvBoardIdGet() == RD_88F5181L_VOIP_GE) {
//GT_PORT_2_LPORT
// 2 for WAN port , 1 , 0, 5, 7 for LAN ports
		//mvOsPrintf("Giga board.\n");
		gstatsGetPortAllCounters2_GTK(qd_dev,2,&counter_set[0]);		
		gstatsGetPortAllCounters2_GTK(qd_dev,1,&counter_set[1]);		
		gstatsGetPortAllCounters2_GTK(qd_dev,0,&counter_set[2]);		
		gstatsGetPortAllCounters2_GTK(qd_dev,5,&counter_set[3]);		
		gstatsGetPortAllCounters2_GTK(qd_dev,7,&counter_set[4]);		
	}
	else {//MARVELL_10_100_BOARD
// 0 for WAN port, 1-4 for LAN ports
		//mvOsPrintf("FE board.\n");
		gstatsGetPortAllCounters_GTK(qd_dev,0,&counter_set[0]);		
		gstatsGetPortAllCounters_GTK(qd_dev,1,&counter_set[1]);		
		gstatsGetPortAllCounters_GTK(qd_dev,2,&counter_set[2]);		
		gstatsGetPortAllCounters_GTK(qd_dev,3,&counter_set[3]);		
		gstatsGetPortAllCounters_GTK(qd_dev,4,&counter_set[4]);		
	}

	strcpy(buf,"collision detect: wan lan1 lan2 lan3 lan4\n");
	len = strlen(buf);

	sprintf(tmp,"%d %d %d %d %d",counter_set[0].Collisions,counter_set[1].Collisions,counter_set[2].Collisions,counter_set[3].Collisions,counter_set[4].Collisions);
	strcat(buf,tmp);
	len += strlen(tmp);


	return len;
}


static int resetLanPhy_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int i;
	GT_STATUS ret;
	if (mvBoardIdGet() == RD_88F5181L_VOIP_GE) { /* Giga Board */
		ret = gprtPhyReset2(qd_dev,0);
		ret = gprtPhyReset2(qd_dev,1);
		ret = gprtPhyReset2(qd_dev,5);
		ret = gprtPhyReset2(qd_dev,7);
	}
	else {
		ret = gprtPhyReset2(qd_dev,1);
		ret = gprtPhyReset2(qd_dev,2);
		ret = gprtPhyReset2(qd_dev,3);
		ret = gprtPhyReset2(qd_dev,4);
	}
#if 0
	for (i = 0; i < qd_dev->numOfPorts; i++) {
		if( i == cfg.cpuPortNum )
			continue;
		ret = gprtPhyReset2(qd_dev,i);
		mvOsPrintf("reset phy [%d], result [%d]\n", i, ret);
	}
#endif
	return ret;
}
static int resetWanPhy_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int i;
	GT_STATUS ret;

	if (mvBoardIdGet() == RD_88F5181L_VOIP_GE) { /* Giga Board */
		ret = gprtPhyReset2(qd_dev,2);
	}
	else {
		ret = gprtPhyReset2(qd_dev,0);
	}
	return ret;
}

GT_STATUS qdInit(void)
{
	GT_STATUS	 status = GT_OK;	
	unsigned int i;

sfenjifnweifj
	status = qdStart();
	if (GT_OK != status)
	{
		gtOsPrintf("qdStart is failed: status = 0x%x\n", status);
		return status;
	}

#ifdef DB_6093_88E6218
	/* start 88E6090 device, assumes SMI Address 0x11 and CPU Port 10 */

	if(loadDev(qd_ext, SMI_MULTI_ADDR_MODE, 0x11, 10) == NULL)
	{
		gtOsPrintf("Failed to start External Device. Please check the SMI Address 0x11!\n");
	}

	/* allow larger than 1522 bytes of frame (header + marvell tag) */
	gsysSetMaxFrameSize(qd_dev,GT_FALSE);

#endif

	for (i=0; i<qd_dev->numOfPorts; i++) 
	{
		/* default port prio to three */
		gcosSetPortDefaultTc(qd_dev, i, 3);       
		/* disable IP TOS Prio */
		gqosIpPrioMapEn(qd_dev, i, GT_FALSE);  
		/* disable QOS Prio */
		gqosUserPrioMapEn(qd_dev, i, GT_FALSE);
		/* Force flow control for all ports */
		gprtSetForceFc(qd_dev, i, GT_FALSE);
	}

	/* Enable port #6 */
	status = gstpSetPortState(qd_dev, 6, GT_PORT_FORWARDING);

	if((status = gprtClearAllCtr(qd_dev)) != GT_OK)
	{		
		return status;
	}	
	for (i=0; i<GT_CPU_SWITCH_PORT; i++)
	{
		gprtSetMcRateLimit(qd_dev, i, GT_MC_100_PERCENT_RL);
	}

#ifdef QD_DEBUG
	for (i=0; i<qd_dev->numOfPorts; i++) 
	{
		short sdata;
	  
	  	hwReadPortReg(qd_dev, i, 0x4, &sdata);
	  	gtOsPrintf("Control reg for port[%d] is: %x\n",i,sdata);
	
	  	hwReadPortReg(qd_dev, i, 0x0, &sdata);
	  	gtOsPrintf("Status reg for port[%d] is: %x\n",i,sdata);
	
	}
	qdStatus();
#endif /* QD_DEBUG */
	/* Gemtek - add /proc/wanLink entry to let user space to detect wan link status */
	create_proc_read_entry("wanLink",0,NULL,wanLink_read_proc,NULL);
	create_proc_read_entry("wanSpeed",0,NULL,wanSpeed_read_proc,NULL);
	create_proc_read_entry("wanDuplex",0,NULL,wanDuplex_read_proc,NULL);
	create_proc_read_entry("lanStatus",0,NULL,lanStatus_read_proc,NULL);
	create_proc_read_entry("resetLanPhy",0,NULL,resetLanPhy_read_proc,NULL);
	create_proc_read_entry("resetWanPhy",0,NULL,resetWanPhy_read_proc,NULL);
	create_proc_read_entry("collisionStatus",0,NULL,collision_read_proc,NULL);
	if (mvBoardIdGet() == RD_88F5181L_VOIP_GE) { /* Giga Board */
#if LED_DUAL_COLOR
	/*
	 // first 200 pieces of NETGEAR WNR854T
	gprtSetLed(0,0x15,0x1000);
	gprtSetLed(1,0x15,0x1000);
	gprtSetLed(2,0x15,0x1000);
	//gprtSetLed(5,0x15,0x1000);
	//gprtSetLed(7,0x15,0x1000);
	*/
	gprtSetLed(0,0x15,0x1777);
	gprtSetLed(1,0x15,0x1777);
	gprtSetLed(2,0x15,0x1777);
	gprtSetLed(5,0x15,0x1777);
	gprtSetLed(7,0x15,0x1777);
#else /* Single color LED */
	gprtSetLed(0,0x40,0x1000);
	gprtSetLed(1,0x40,0x1000);
	gprtSetLed(2,0x40,0x1000);
	gprtSetLed(5,0x40,0x1000);
	gprtSetLed(7,0x40,0x1000);
#endif
	}
	else { /* Fast Ethernet Board */
#if LED_DUAL_COLOR
/*
		for (i = 0; i < qd_dev->numOfPorts; i++) {
			if( i == cfg.cpuPortNum )
				continue;
			gprtSetLed(i,0x15,0x1777);
			mvOsPrintf("set [%d] to 0x15,0x1777\n");
		}
*/
#else
		for (i = 0; i < qd_dev->numOfPorts; i++) {
			if( i == cfg.cpuPortNum )
				continue;
			gprtSetLed(i,0x40,0x1000);
		}
#endif

	}

#if 0
	#if MARVELL_GIGA_BOARD /* Board for Netgear */
#if 0 // first 200 pieces of NETGEAR WNR854T
	gprtSetLed(0,0x15,0x1000);
	gprtSetLed(1,0x15,0x1000);
	gprtSetLed(2,0x15,0x1000);
	//gprtSetLed(5,0x15,0x1000);
	//gprtSetLed(7,0x15,0x1000);
#else // Final Hardware for NETGEAR WNR854T
	gprtSetLed(0,0x15,0x1777);
	gprtSetLed(1,0x15,0x1777);
	gprtSetLed(2,0x15,0x1777);
	gprtSetLed(5,0x15,0x1777);
	gprtSetLed(7,0x15,0x1777);
/*
	for (i = 0; i < qd_dev->numOfPorts; i++) {
		if( i == cfg.cpuPortNum )
			continue;
		gprtSetLed(i,0x15,0x1777);
	}
*/
#endif
	//gprtSetLed(5,0x15,0x1000);
	//gprtSetLed(7,0x15,0x1000);
#else /* Board for White Label */

#if 0
	gprtSetLed(0,0x55,0x1000);
	gprtSetLed(1,0x55,0x1000);
	gprtSetLed(2,0x55,0x1000);
	gprtSetLed(5,0x55,0x1000);
	gprtSetLed(7,0x55,0x1000);
#else /* for 10/100 board */
	for (i = 0; i < qd_dev->numOfPorts; i++) {
		if( i == cfg.cpuPortNum )
			continue;
		gprtSetLed(i,0x40,0x1000);
	}
#endif

#endif
#endif

#if 1 //Fix the issue of 2 lan ports can not work.
	if (mvBoardIdGet() == RD_88F5181L_VOIP_GE)
	{
		mvOsPrintf("----set PPU En\n");
		gsysSetPPUEn(qd_dev, GT_TRUE);
#if 1 //set to auto mode
		for (i = 0; i < qd_dev->numOfPorts; i++) {
		    if( i == cfg.cpuPortNum )
			continue;
		    if(gprtSetPortAutoMode(qd_dev, i, SPEED_10_100_DUPLEX_AUTO ) != GT_OK)
			mvOsPrintf("####### gprtSetPortAutoMode failed on port %d\n",i);
		}
#endif
	}
#endif
	gtOsPrintf("QD initiated\n");
	return status;    
}

static const char* qdPortStpStates[] = 
	{"DISABLE",
     "BLOCKING",
     "LEARNING",
     "FORWARDING"};	

static char* qdPortListToStr(GT_LPORT* portList, int portListNum,
							char* portListStr)
{
	int	port, idx, strIdx=0;
	
	for(idx=0; idx<portListNum; idx++)
	{
		port = portList[idx];
		sprintf(&portListStr[strIdx], "%d,", port);
		strIdx = strlen(portListStr);
	}
	portListStr[strIdx] = '\0';
	return portListStr;
}

void qdStatus(void)
{
	int 				port;
	GT_BOOL				linkState;
	GT_PORT_STP_STATE 	stpState;
	GT_PORT_STAT    	counters;
	GT_U16				pvid;
	GT_LPORT 			portList[GT_NUM_OF_SWITCH_PORTS];
    GT_U8    			portNum;
	char				portListStr[100];

	gtOsPrintf("Port  Link   PVID    Group       State       RxCntr      TxCntr\n\n");

    for (port=0; port<GT_NUM_OF_SWITCH_PORTS; port++) 
	{
		gprtGetLinkState(qd_dev, port, &linkState);
		gstpGetPortState(qd_dev, port, &stpState);
		gprtGetPortCtr(qd_dev,port, &counters);
		gstpGetPortState(qd_dev, port, &stpState);
		gvlnGetPortVid(qd_dev, port, &pvid);
		gvlnGetPortVlanPorts(qd_dev, port, portList, &portNum);
		qdPortListToStr(portList, portNum, portListStr);

		gtOsPrintf(" %d.   %4s    %d     %-10s  %-10s   0x%-8x  0x%-8x\n",
					port, (linkState==GT_TRUE) ? "UP" : "DOWN",
					pvid, portListStr, qdPortStpStates[stpState],
					counters.rxCtr, counters.txCtr);
	}
}
