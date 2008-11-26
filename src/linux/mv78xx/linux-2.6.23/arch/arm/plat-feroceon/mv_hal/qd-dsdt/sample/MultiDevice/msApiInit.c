#include <Copyright.h>
/********************************************************************************
* msApiInit.c
*
* DESCRIPTION:
*		MS API initialization routine for devices supporting Multi Address Mode,
*		such as 88E6183. Following setup will be used for this sample code.
*
*	  	------------------
*		|CPU Ethernet Dev|
*		------------------
*		 |		
*		 |		
*		 |   8--------------9	 8--------------9	  8--------------
*		 |----| QD Device 0|------| QD Device 1|-----| QD Device 2|
*	         --------------		  --------------		--------------
*		       0 1 2 ... 7			 0 1 2 ... 7		  0 1 2 ... 7
*
*
*		Ethernet port of CPU is connected to port 8 of Device 0,
*		port 9 of Device 0 is connected to port 8 of Device 1, and
*		port 9 of Device 1 is connected to port 8 of Device 2.
*
*		Device 0 uses Phy Address 1, 
*		Device 1 uses Phy Address 2, and
*		Device 2 uses Phy Address 3
*		Notes: Phy Address 0 cannot be used in a Multi Chip Address Mode.
*
*		Each Switch Device has to be configured to Multi Chip Address Mode.
*		For detailed information for Multi Chip Address Mode configuration,
*		please refer to your device's Datasheet.
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

#define MULTI_ADDR_MODE
#define N_OF_QD_DEVICES		3	/* number of 88E6183 devices connected */

#define DEVICE0_ID		10
#define DEVICE1_ID		DEVICE0_ID + 1
#define DEVICE2_ID		DEVICE0_ID + 2

#define DEVICE0_PHY_ADDR	1
#define DEVICE1_PHY_ADDR	DEVICE0_PHY_ADDR + 1
#define DEVICE2_PHY_ADDR	DEVICE0_PHY_ADDR + 2

#define S_CPU_DEVICE		DEVICE0_ID

#define DEVICE0_CPU_PORT		8
#define DEVICE0_CASCADE_PORT	9
#define DEVICE1_CPU_PORT		8
#define DEVICE1_CASCADE_PORT	9
#define DEVICE2_CPU_PORT		8
#define DEVICE2_CASCADE_PORT	9

GT_QD_DEV       *qdMultiDev[N_OF_QD_DEVICES] = {0,};

/*
 * Initialize each Switch Devices. This should be done in BSP driver init routine.
 *	Since BSP is not combined with QuarterDeck driver, we are doing here.
 * This routine will setup Switch Devices according to the above description.
*/

GT_STATUS qdMultiDevStart()
{
	GT_STATUS status = GT_FAIL;
	GT_SYS_CONFIG   cfg;
	int cpuPort;
	int cascadePort;
	int i,j;

	memset((char*)&cfg,0,sizeof(GT_SYS_CONFIG));
	
	/* 
	 *	Create QD Device Structure for each device.
	 */
	for(i=0; i<N_OF_QD_DEVICES; i++)
	{
		qdMultiDev[i] = (GT_QD_DEV*)malloc(sizeof(GT_QD_DEV));

		if(qdMultiDev[i] == NULL)
		{
			while(i--)
				free(qdMultiDev[i]);
			return GT_FAIL;
		}

		memset((char*)qdMultiDev[i],0,sizeof(GT_QD_DEV));
	}
	
	/*
	 *  Register all the required functions to QuarterDeck Driver for each device.
	*/
	for(i=0; i<N_OF_QD_DEVICES; i++)
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

		cfg.initPorts = GT_TRUE;	/* Set switch ports to Forwarding mode. If GT_FALSE, use Default Setting. */
		switch (i)
		{
			case 0: /* if we are registering device 0 */
				cfg.cpuPortNum = DEVICE0_CPU_PORT;
				break;
			case 1: /* if we are registering device 1 */
				cfg.cpuPortNum = DEVICE1_CPU_PORT;	/* where device 0 is connected */
				break;
			case 2: /* if we are registering device 2 */
				cfg.cpuPortNum = DEVICE2_CPU_PORT;	/* where device 1 is connected */
				break;
			default: /* we don't have any more device. it shouldn't happen in our sample setup. */
				goto errorExit;
		}

#ifdef MANUAL_MODE	/* not defined. this is only for sample */
		/* user may want to use this mode when there are two QD switchs on the same MII bus. */
		cfg.mode.scanMode = SMI_MANUAL_MODE;	/* Use QD located at manually defined base addr */
		cfg.mode.baseAddr = 0x10;	/* valid value in this case is either 0 or 0x10 */
#else
#ifdef MULTI_ADDR_MODE	/* It should have been defined for this sample code */
		cfg.mode.scanMode = SMI_MULTI_ADDR_MODE;	/* find a QD in indirect access mode */
		cfg.mode.baseAddr = DEVICE0_PHY_ADDR + i;		/* this is the phyAddr used by QD family device. 
																		Valid values are 1 ~ 31.*/
#else
		cfg.mode.scanMode = SMI_AUTO_SCAN_MODE;	/* Scan 0 or 0x10 base address to find the QD */
		cfg.mode.baseAddr = 0;
#endif
#endif

		if((status=qdLoadDriver(&cfg, qdMultiDev[i])) != GT_OK)
		{
			MSG_PRINT(("qdLoadDriver return Failed\n"));
			goto errorExit;
		}

		MSG_PRINT(("Device ID     : 0x%x\n",qdMultiDev[i]->deviceId));
		MSG_PRINT(("Base Reg Addr : 0x%x\n",qdMultiDev[i]->baseRegAddr));
		MSG_PRINT(("No of Ports   : %d\n",qdMultiDev[i]->numOfPorts));
		MSG_PRINT(("CPU Ports     : %d\n",qdMultiDev[i]->cpuPortNum));

		/*
		 *  start the QuarterDeck
		*/
		if((status=sysEnable(qdMultiDev[i])) != GT_OK)
		{
			MSG_PRINT(("sysConfig return Failed\n"));
			goto errorExit;
		}
	}

	/* 
		Now, we need to configure Cascading information for each devices.
		1. Set Interswitch port mode for port 8 and 9 for device 0,1,and 2,
			so that switch device can expect Marvell Tag from frames 
			ingressing/egressing this port.
		2. Set CPU Port information (for To_CPU frame) for each port of device.
		3. Set Cascading Port information (for From_CPU fram) for each device.
		4. Set Device ID (if required)
			Note: DeviceID is hardware configurable.
	*/
	for(i=0; i<N_OF_QD_DEVICES; i++)
	{
		switch (i)
		{
			case 0: /* if we are registering device 0 */
				cpuPort = DEVICE0_CPU_PORT; 		/* where CPU Enet port is connected */
				cascadePort = DEVICE0_CASCADE_PORT;	/* where device 1 is connected */
				break;
			case 1: /* if we are registering device 1 */
				cpuPort = DEVICE1_CPU_PORT; 		/* where device 0 is connected */
				cascadePort = DEVICE1_CASCADE_PORT;	/* where device 2 is connected */
				break;
			case 2: /* if we are registering device 2 */
				cpuPort = DEVICE2_CPU_PORT; 		/* where device 1 is connected */
				cascadePort = DEVICE2_CASCADE_PORT;	/* no need to setup for the given sample setup */
				break;
			default: /* we don't have any more device. it shouldn't happen in our sample setup. */
				goto errorExit;
		}

		/*
			1. Set Interswitch port mode for port 8 and 9 for device 0,1,and 2,
				so that switch device can expect Marvell Tag from frames 
				ingressing/egressing this port.
			2. Set CPU Port information (for To_CPU frame) for each port of device.
		*/			
		for(j=0; j<qdMultiDev[i]->numOfPorts; j++)
		{
			if((i == cpuPort) || (i == cascadePort))
			{
				if((status=gprtSetInterswitchPort(qdMultiDev[i],j,GT_TRUE)) != GT_OK)
				{
					MSG_PRINT(("gprtSetInterswitchPort returned %i (port %i, mode TRUE)\n",status,j));
					goto errorExit;
				}
			}
			else
			{
				if((status=gprtSetInterswitchPort(qdMultiDev[i],j,GT_FALSE)) != GT_OK)
				{
					MSG_PRINT(("gprtSetInterswitchPort returned %i (port %i, mode FALSE)\n",status,j));
					goto errorExit;
				}
			}

			if((status=gprtSetCPUPort(qdMultiDev[i],j,cpuPort)) != GT_OK)
			{
				MSG_PRINT(("gprtSetCPUPort returned %i\n",status));
				goto errorExit;
			}
		}

		/*
			3. Set Cascading Port information (for From_CPU fram) for each device.
		*/	 	
		if((status=gsysSetCascadePort(qdMultiDev[i],cascadePort)) != GT_OK)
		{
			MSG_PRINT(("gsysSetCascadePort returned %i\n",status));
			goto errorExit;
		}

		/*
			4. Set Device ID (if required)
		*/	 	
		if((status=gsysSetDeviceNumber(qdMultiDev[i],DEVICE0_ID+i)) != GT_OK)
		{
			MSG_PRINT(("gsysSetDeviceNumber returned %i\n",status));
			goto errorExit;
		}

	}	

	MSG_PRINT(("QuarterDeck has been started.\n"));

	return GT_OK;

errorExit:

	for(i=0; i<N_OF_QD_DEVICES; i++)
	{
		if(qdMultiDev[i] != NULL)
		{
			qdUnloadDriver(qdMultiDev[i]);
	  		free(qdMultiDev[i]);
		}
	}	

	MSG_PRINT(("QuarterDeck initialization failed.\n"));

	return status;
}

