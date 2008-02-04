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
*		 |   8--------------9    8--------------9   8--------------
*		 |----| QD Device 0|------| QD Device 1|-----| QD Device 2|
*             --------------      --------------     --------------
*               0 1 2 ... 7         0 1 2 ... 7        0 1 2 ... 7
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
#define N_OF_QD_DEVICES		2	/* number of 88E6183 devices connected */

#define DEVICE0_ID		1
#define DEVICE1_ID		DEVICE0_ID + 1
#define DEVICE2_ID		DEVICE0_ID + 2

#define DEVICE0_PHY_ADDR	0x11
#define DEVICE1_PHY_ADDR	DEVICE0_PHY_ADDR + 1
#define DEVICE2_PHY_ADDR	DEVICE0_PHY_ADDR + 2

#define S_CPU_DEVICE		DEVICE0_ID

#define DEVICE0_CPU_PORT		7
#define DEVICE0_CASCADE_PORT	6
#define DEVICE1_CPU_PORT		7
#define DEVICE1_CASCADE_PORT	6
#define DEVICE2_CPU_PORT		7
#define DEVICE2_CASCADE_PORT	6

extern GT_QD_DEV       	qddev[4];

GT_QD_DEV       *qdMultiDev[N_OF_QD_DEVICES] = {0,};
GT_QD_DEV       *qdDev0 = &qddev[1];
GT_QD_DEV       *qdDev1 = NULL;
GT_QD_DEV       *qdDev2 = NULL;

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

GT_STATUS RubyStart(int phyAddr, GT_QD_DEV* d)
{
	GT_STATUS status = GT_FAIL;
	GT_SYS_CONFIG   cfg;

	memset((char*)&cfg,0,sizeof(GT_SYS_CONFIG));
	MSG_PRINT(("Size of GT_QD_DEV %i\n",sizeof(GT_QD_DEV)));

	if(d == NULL)
	{
		MSG_PRINT(("Device Structure is NULL.\n"));
		return GT_FAIL;
	}

	memset((char*)d,0,sizeof(GT_QD_DEV));

	cfg.BSPFunctions.readMii   = ffReadMii;
	cfg.BSPFunctions.writeMii  = ffWriteMii;
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
	cfg.cpuPortNum = 10;

	cfg.mode.scanMode = SMI_MULTI_ADDR_MODE;	
	cfg.mode.baseAddr = phyAddr;	/* valid value in this case is either 0 or 0x10 */

	if((status=qdLoadDriver(&cfg, d)) != GT_OK)
	{
		MSG_PRINT(("qdLoadDriver return Failed\n"));
		return status;
	}

	MSG_PRINT(("Device ID     : 0x%x\n",d->deviceId));
	MSG_PRINT(("PHY Addr      : 0x%x\n",d->phyAddr));
	MSG_PRINT(("Base Addr     : 0x%x\n",d->baseRegAddr));
	MSG_PRINT(("CPU Ports     : %d\n",d->cpuPortNum));
	MSG_PRINT(("N Ports       : %d\n",d->numOfPorts));
	MSG_PRINT(("Device Group  : 0x%x\n",d->devName));
	MSG_PRINT(("QDDev         : %#x\n",(unsigned long)&d));

	/*
	 *  start the QuarterDeck
	*/
	if((status=sysEnable(d)) != GT_OK)
	{
		MSG_PRINT(("sysConfig return Failed\n"));
		return status;
	}

	return GT_OK;
}

GT_STATUS SwStart(int phyAddr, GT_QD_DEV* d)
{
	return RubyStart(phyAddr,d);
}

GT_QD_DEV* loadDev(GT_QD_DEV* dev, int mode, int phyAddr, int cpuPort, unsigned int cfgMode)
{
	GT_QD_DEV* d = dev;
	GT_STATUS status = GT_FAIL;
	GT_SYS_CONFIG   cfg;

	if((int)dev == -1)
		goto printUse;

	memset((char*)&cfg,0,sizeof(GT_SYS_CONFIG));

	if(d == NULL)
	{
		d = (GT_QD_DEV*)malloc(sizeof(GT_QD_DEV));
		
		if(d == NULL)
		{
			MSG_PRINT(("Failed to allocate Device Structure\n"));
			return NULL;
		}
	}

	memset((char*)d,0,sizeof(GT_QD_DEV));

	cfg.BSPFunctions.readMii   = ffReadMii;
	cfg.BSPFunctions.writeMii  = ffWriteMii;
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
	cfg.cpuPortNum = cpuPort;
	cfg.skipInitSetup = (GT_U32)cfgMode;

	switch(mode)
	{
		case SMI_MANUAL_MODE:		/* Use QD located at manually defined base addr */
		case SMI_MULTI_ADDR_MODE:	/* Use QD in multi chip address mode */
			cfg.mode.scanMode = mode;	
			cfg.mode.baseAddr = phyAddr;	/* valid value in this case is either 0 or 0x10 */
			break;
		case SMI_AUTO_SCAN_MODE:	/* Scan 0 or 0x10 base address to find the QD */
			cfg.mode.scanMode = mode;
			cfg.mode.baseAddr = 0;
			break;
		default:
			MSG_PRINT(("Unknown Mode %i\n",mode));
			goto printUse;
	}

	if((status=qdLoadDriver(&cfg, d)) != GT_OK)
	{
		MSG_PRINT(("qdLoadDriver return Failed\n"));
		goto loadErr;
	}

	MSG_PRINT(("Device ID     : 0x%x\n",d->deviceId));
	MSG_PRINT(("PHY Addr      : 0x%x\n",d->phyAddr));
	MSG_PRINT(("Base Addr     : 0x%x\n",d->baseRegAddr));
	MSG_PRINT(("CPU Ports     : %d\n",d->cpuPortNum));

	/*
	 *  start the QuarterDeck
	*/
	if((status=sysEnable(d)) != GT_OK)
	{
		MSG_PRINT(("sysConfig return Failed\n"));
		goto loadErr;
	}

	return d;

printUse:
	MSG_PRINT(("Usage: loadDev(Dev,mode,phyAddr,cpuPort)\n",SMI_AUTO_SCAN_MODE));
	MSG_PRINT(("\tSMI_AUTO_SCAN_MODE :  %i\n",SMI_AUTO_SCAN_MODE));
	MSG_PRINT(("\tSMI_MANUAL_MODE :     %i\n",SMI_MANUAL_MODE));
	MSG_PRINT(("\tSMI_MULTI_ADDR_MODE : %i\n",SMI_MULTI_ADDR_MODE));
	MSG_PRINT(("Example: loadDev(0,1,0x10,5)\n"));
	MSG_PRINT(("for Manual mode, phy base address 0x10, and cpu port 5\n"));
	
loadErr:
	
	if(dev)
		return NULL;

	if(d)
		free(d);

	return NULL;
}

/*
 * Initialize each Switch Devices. This should be done in BSP driver init routine.
 *	Since BSP is not combined with QuarterDeck driver, we are doing here.
 * This routine will setup Switch Devices according to the above description.
*/
GT_STATUS qdMultiDevStart()
{
	GT_STATUS status = GT_FAIL;
	int cpuPort;
	int cascadePort;
	int i,j;

	/* 
	 *	Create QD Device Structure for each device.
	 */
	for(i=0; i<N_OF_QD_DEVICES; i++)
	{
		if(qdMultiDev[i] == NULL)
		{
			qdMultiDev[i] = (GT_QD_DEV*)malloc(sizeof(GT_QD_DEV));
		
			if(qdMultiDev[i] == NULL)
			{
				while(i--)
				{
					free(qdMultiDev[i]);
					qdMultiDev[i] = NULL;
				}
				return GT_FAIL;
			}
		}

		memset((char*)qdMultiDev[i],0,sizeof(GT_QD_DEV));
	}
	
	/*
	 *  Register all the required functions to QuarterDeck Driver for each device.
	*/
	for(i=0; i<N_OF_QD_DEVICES; i++)
	{
		switch (i)
		{
			case 0: /* if we are registering device 0 */
				cpuPort = DEVICE0_CPU_PORT;
				break;
			case 1: /* if we are registering device 1 */
				cpuPort = DEVICE1_CPU_PORT;	/* where device 0 is connected */
				break;
			case 2: /* if we are registering device 2 */
				cpuPort = DEVICE2_CPU_PORT;	/* where device 1 is connected */
				break;
			default: /* we don't have any more device. it shouldn't happen in our sample setup. */
				goto errorExit;
		}

		MSG_PRINT(("Initializing QD Device %i...\n",i));
		if(loadDev(qdMultiDev[i],SMI_MULTI_ADDR_MODE,DEVICE0_PHY_ADDR + i,cpuPort,0) == NULL)
		{
			MSG_PRINT(("QD Device %i is not initialized.\n",i));
			free(qdMultiDev[i]);
			qdMultiDev[i] = NULL;
			continue;
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
		switch (i)
		{
			case 0: /* if we are registering device 0 */
				cpuPort = DEVICE0_CPU_PORT; 		/* where CPU Enet port is connected */
				cascadePort = DEVICE0_CASCADE_PORT;	/* where device 1 is connected */
				qdDev0 = qdMultiDev[i];
				MSG_PRINT(("Use qdDev0 to access Device 0.\n"));
				break;
			case 1: /* if we are registering device 1 */
				cpuPort = DEVICE1_CPU_PORT; 		/* where device 0 is connected */
				cascadePort = DEVICE1_CASCADE_PORT;	/* where device 2 is connected */
				qdDev1 = qdMultiDev[i];
				MSG_PRINT(("Use qdDev1 to access Device 1.\n"));
				break;
			case 2: /* if we are registering device 2 */
				cpuPort = DEVICE2_CPU_PORT; 		/* where device 1 is connected */
				cascadePort = DEVICE2_CASCADE_PORT;	/* no need to setup for the given sample setup */
				qdDev2 = qdMultiDev[i];
				MSG_PRINT(("Use qdDev2 to access Device 2.\n"));
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
		MSG_PRINT(("Setting InterSwitch Port and CPU Port...\n"));
		for(j=0; j<qdMultiDev[i]->numOfPorts; j++)
		{
			if((j == cpuPort) || (j == cascadePort))
			{
				if((status=gprtSetInterswitchPort(qdMultiDev[i],j,GT_TRUE)) != GT_OK)
				{
					MSG_PRINT(("gprtSetInterswitchPort returned %i (port %i, mode TRUE)\n",status,j));
					break;
				}
			}
			else
			{
				if((status=gprtSetInterswitchPort(qdMultiDev[i],j,GT_FALSE)) != GT_OK)
				{
					MSG_PRINT(("gprtSetInterswitchPort returned %i (port %i, mode FALSE)\n",status,j));
					break;
				}
			}

			if((status=gprtSetCPUPort(qdMultiDev[i],j,cpuPort)) != GT_OK)
			{
				MSG_PRINT(("gprtSetCPUPort returned %i\n",status));
					break;
			}
		}

		/*
			3. Set Cascading Port information (for From_CPU fram) for each device.
		*/	 	
		MSG_PRINT(("Setting Cascade Port...\n"));
		if((status=gsysSetCascadePort(qdMultiDev[i],cascadePort)) != GT_OK)
		{
			MSG_PRINT(("gsysSetCascadePort returned %i\n",status));
			continue;
		}

		/*
			4. Set Device ID (if required)
		*/	 	
		MSG_PRINT(("Setting Device ID (%i)...\n",DEVICE0_ID+i));
		if((status=gsysSetDeviceNumber(qdMultiDev[i],DEVICE0_ID+i)) != GT_OK)
		{
			MSG_PRINT(("gsysSetDeviceNumber returned %i\n",status));
			continue;
		}

	}	

	return GT_OK;

errorExit:

	/* code will be reached here only if N_OF_QD_DEVICES > 3 */
	for(i=0; i<N_OF_QD_DEVICES; i++)
	{
		if(qdMultiDev[i] != NULL)
		{
	  		free(qdMultiDev[i]);
			qdMultiDev[i] = NULL;
		}
	}	

	return GT_FAIL;
}

