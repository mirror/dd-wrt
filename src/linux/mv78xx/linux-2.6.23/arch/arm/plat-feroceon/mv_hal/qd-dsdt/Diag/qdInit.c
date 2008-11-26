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


GT_STATUS qdInit(void)
{
	GT_STATUS	 status = GT_OK;	
	unsigned int i;

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
