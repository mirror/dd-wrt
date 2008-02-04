#include <Copyright.h>
/********************************************************************************
* msApiInit.c
*
* DESCRIPTION:
*       MS API initialization routine
*
* DEPENDENCIES:   Platform
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include "msSample.h"
/*
#define MULTI_ADDR_MODE
#define MANUAL_MODE
*/

GT_SYS_CONFIG   cfg;
GT_QD_DEV       diagDev;
GT_QD_DEV       *dev=&diagDev;


/*
 *  Initialize the QuarterDeck. This should be done in BSP driver init routine.
 *	Since BSP is not combined with QuarterDeck driver, we are doing here.
*/

GT_STATUS qdStart(int cpuPort, int useQdSim, int devId) /* devId is used for simulator only */
{
GT_STATUS status;

	/*
	 *  Register all the required functions to QuarterDeck Driver.
	*/
	memset((char*)&cfg,0,sizeof(GT_SYS_CONFIG));
	memset((char*)&diagDev,0,sizeof(GT_QD_DEV));

	if(useQdSim == 0) /* use EV-96122 */
	{
		cfg.BSPFunctions.readMii   = gtBspReadMii;
		cfg.BSPFunctions.writeMii  = gtBspWriteMii;
#ifdef USE_SEMAPHORE
		cfg.BSPFunctions.semCreate = osSemCreate;
		cfg.BSPFunctions.semDelete = osSemDelete;
		cfg.BSPFunctions.semTake   = osSemWait;
		cfg.BSPFunctions.semGive   = osSemSignal;
#else
		cfg.BSPFunctions.semCreate = NULL;
		cfg.BSPFunctions.semDelete = NULL;
		cfg.BSPFunctions.semTake   = NULL;
		cfg.BSPFunctions.semGive   = NULL;
#endif
		gtBspMiiInit(dev);
	}
	else	/* use QuaterDeck Simulator (No QD Device Required.) */
	{
		cfg.BSPFunctions.readMii   = qdSimRead;
		cfg.BSPFunctions.writeMii  = qdSimWrite;
#ifdef USE_SEMAPHORE
		cfg.BSPFunctions.semCreate = osSemCreate;
		cfg.BSPFunctions.semDelete = osSemDelete;
		cfg.BSPFunctions.semTake   = osSemWait;
		cfg.BSPFunctions.semGive   = osSemSignal;
#else
		cfg.BSPFunctions.semCreate = NULL;
		cfg.BSPFunctions.semDelete = NULL;
		cfg.BSPFunctions.semTake   = NULL;
		cfg.BSPFunctions.semGive   = NULL;
#endif

		qdSimInit(devId,0);
	}

	cfg.initPorts = GT_TRUE;	/* Set switch ports to Forwarding mode. If GT_FALSE, use Default Setting. */
	cfg.cpuPortNum = cpuPort;
#ifdef MANUAL_MODE	/* not defined. this is only for sample */
	/* user may want to use this mode when there are two QD switchs on the same MII bus. */
	cfg.mode.scanMode = SMI_MANUAL_MODE;	/* Use QD located at manually defined base addr */
	cfg.mode.baseAddr = 0x10;	/* valid value in this case is either 0 or 0x10 */
#else
#ifdef MULTI_ADDR_MODE
	cfg.mode.scanMode = SMI_MULTI_ADDR_MODE;	/* find a QD in indirect access mode */
	cfg.mode.baseAddr = 1;		/* this is the phyAddr used by QD family device. 
								Valid value are 1 ~ 31.*/
#else
	cfg.mode.scanMode = SMI_AUTO_SCAN_MODE;	/* Scan 0 or 0x10 base address to find the QD */
	cfg.mode.baseAddr = 0;
#endif
#endif
	if((status=qdLoadDriver(&cfg, dev)) != GT_OK)
	{
		MSG_PRINT(("qdLoadDriver return Failed\n"));
		return status;
	}

	MSG_PRINT(("Device ID     : 0x%x\n",dev->deviceId));
	MSG_PRINT(("Base Reg Addr : 0x%x\n",dev->baseRegAddr));
	MSG_PRINT(("No of Ports   : %d\n",dev->numOfPorts));
	MSG_PRINT(("CPU Ports     : %d\n",dev->cpuPortNum));

	/*
	 *  start the QuarterDeck
	*/
	if((status=sysEnable(dev)) != GT_OK)
	{
		MSG_PRINT(("sysConfig return Failed\n"));
		return status;
	}

	MSG_PRINT(("QuarterDeck has been started.\n"));

	return GT_OK;
}

