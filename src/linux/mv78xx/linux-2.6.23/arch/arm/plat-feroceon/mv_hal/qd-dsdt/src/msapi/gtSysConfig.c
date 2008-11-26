#include <Copyright.h>

/********************************************************************************
* gtSysConfig.c
*
* DESCRIPTION:
*       API definitions for system configuration, and enabling.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 4 $
*
*******************************************************************************/

#include <msApi.h>
#include <gtDrvConfig.h>
#include <gtSem.h>
#include <platformDeps.h>

static GT_BOOL gtRegister(GT_QD_DEV *dev, BSP_FUNCTIONS* pBSPFunctions);

/*******************************************************************************
* qdLoadDriver
*
* DESCRIPTION:
*       QuarterDeck Driver Initialization Routine.
*       This is the first routine that needs be called by system software.
*       It takes *cfg from system software, and retures a pointer (*dev)
*       to a data structure which includes infomation related to this QuarterDeck
*       device. This pointer (*dev) is then used for all the API functions.
*
* INPUTS:
*       cfg  - Holds device configuration parameters provided by system software.
*
* OUTPUTS:
*       dev  - Holds device information to be used for each API call.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_ALREADY_EXIST    - if device already started
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
* 	qdUnloadDriver is also provided to do driver cleanup.
*
*******************************************************************************/
GT_STATUS qdLoadDriver
(
    IN  GT_SYS_CONFIG   *cfg,
    OUT GT_QD_DEV	*dev
)
{
    GT_STATUS   retVal;
	GT_LPORT	port;

    DBG_INFO(("qdLoadDriver Called.\n"));

    /* Check for parameters validity        */
    if(dev == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* Check for parameters validity        */
    if(cfg == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* The initialization was already done. */
    if(dev->devEnabled)
    {
        DBG_INFO(("QuarterDeck already started.\n"));
        return GT_ALREADY_EXIST;
    }

    if(gtRegister(dev,&(cfg->BSPFunctions)) != GT_TRUE)
    {
       DBG_INFO(("gtRegister Failed.\n"));
       return GT_FAIL;
    }
	dev->accessMode = cfg->mode.scanMode;
	if (dev->accessMode == SMI_MULTI_ADDR_MODE)
	{
		dev->baseRegAddr = 0;
		dev->phyAddr = cfg->mode.baseAddr;
	}
	else
	{
		dev->baseRegAddr = cfg->mode.baseAddr;
		dev->phyAddr = 0;
	}

    /* Initialize the driver    */
    retVal = driverConfig(dev);
    if(retVal != GT_OK)
    {
        DBG_INFO(("driverConfig Failed.\n"));
        return retVal;
    }

    /* Initialize dev fields.         */
    dev->cpuPortNum = cfg->cpuPortNum;
    dev->maxPhyNum = 5;
	dev->devGroup = 0;

    /* Assign Device Name */
    switch(dev->deviceId)
    {
		case GT_88E6021:
				dev->numOfPorts = 3;
				dev->maxPorts = 3;
				dev->maxPhyNum = 2;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6021;
				break;

		case GT_88E6051:
				dev->numOfPorts = 5;
				dev->maxPorts = 5;
				dev->maxPhyNum = 5;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6051;
				break;

		case GT_88E6052:
				dev->numOfPorts = 7;
				dev->maxPorts = 7;
				dev->maxPhyNum = 5;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6052;
				break;

		case GT_88E6060:
				if((dev->cpuPortNum != 4)&&(dev->cpuPortNum != 5))
				{
					return GT_FAIL;
				}
				dev->numOfPorts = 6;
				dev->maxPorts = 6;
				dev->maxPhyNum = 5;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6060;
				break;

		case GT_88E6031:
				dev->numOfPorts = 3;
				dev->maxPorts = 6;
				dev->maxPhyNum = 3;
				dev->validPortVec = 0x31;	/* port 0, 4, and 5 */
				dev->validPhyVec = 0x31;	/* port 0, 4, and 5 */
				dev->devName = DEV_88E6061;
				break;

		case GT_88E6061:
				dev->numOfPorts = 6;
				dev->maxPorts = 6;
				dev->maxPhyNum = 6;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6061;
				break;

		case GT_88E6035:
				dev->numOfPorts = 3;
				dev->maxPorts = 6;
				dev->maxPhyNum = 3;
				dev->validPortVec = 0x31;	/* port 0, 4, and 5 */
				dev->validPhyVec = 0x31;	/* port 0, 4, and 5 */
				dev->devName = DEV_88E6065;
				break;

		case GT_88E6055:
				dev->numOfPorts = 5;
				dev->maxPorts = 6;
				dev->maxPhyNum = 5;
				dev->validPortVec = 0x2F;	/* port 0,1,2,3, and 5 */
				dev->validPhyVec = 0x2F;	/* port 0,1,2,3, and 5 */
				dev->devName = DEV_88E6065;
				break;

		case GT_88E6065:
				dev->numOfPorts = 6;
				dev->maxPorts = 6;
				dev->maxPhyNum = 6;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6065;
				break;

		case GT_88E6063:
				dev->numOfPorts = 7;
				dev->maxPorts = 7;
				dev->maxPhyNum = 5;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6063;
				break;

		case GT_FH_VPN:
				dev->numOfPorts = 7;
				dev->maxPorts = 7;
				dev->maxPhyNum = 5;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_FH_VPN;
				break;

		case GT_FF_EG:
				if(dev->cpuPortNum != 5)
				{
					return GT_FAIL;
				}
				dev->numOfPorts = 6;
				dev->maxPorts = 6;
				dev->maxPhyNum = 5;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_FF_EG;
				break;

		case GT_FF_HG:
				dev->numOfPorts = 7;
				dev->maxPorts = 7;
				dev->maxPhyNum = 5;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_FF_HG;
				break;

		case GT_88E6083:
				dev->numOfPorts = 10;
				dev->maxPorts = 10;
				dev->maxPhyNum = 8;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6083;
				break;

		case GT_88E6153:
				dev->numOfPorts = 6;
				dev->maxPorts = 6;
				dev->maxPhyNum = 6;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6183;
				break;

		case GT_88E6181:
				dev->numOfPorts = 8;
				dev->maxPorts = 8;
				dev->maxPhyNum = 8;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6181;
				break;

		case GT_88E6183:
				dev->numOfPorts = 10;
				dev->maxPorts = 10;
				dev->maxPhyNum = 10;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6183;
				break;

		case GT_88E6093:
				dev->numOfPorts = 11;
				dev->maxPorts = 11;
				dev->maxPhyNum = 11;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6093;
				break;

		case GT_88E6092:
				dev->numOfPorts = 11;
				dev->maxPorts = 11;
				dev->maxPhyNum = 11;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6092;
				break;

		case GT_88E6095:
				dev->numOfPorts = 11;
				dev->maxPorts = 11;
				dev->maxPhyNum = 11;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6095;
				break;

		case GT_88E6045:
				dev->numOfPorts = 6;
				dev->maxPorts = 11;
				dev->maxPhyNum = 11;
				dev->validPortVec = 0x60F;
				dev->validPhyVec = 0x60F;
				dev->devName = DEV_88E6095;
				break;

		case GT_88E6097:
				dev->numOfPorts = 11;
				dev->maxPorts = 11;
				dev->maxPhyNum = 11;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6097;
				break;

		case GT_88E6096:
				dev->numOfPorts = 11;
				dev->maxPorts = 11;
				dev->maxPhyNum = 11;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6096;
				break;

		case GT_88E6047:
				dev->numOfPorts = 6;
				dev->maxPorts = 11;
				dev->maxPhyNum = 11;
				dev->validPortVec = 0x60F;
				dev->validPhyVec = 0x60F;
				dev->devName = DEV_88E6097;
				break;

		case GT_88E6046:
				dev->numOfPorts = 6;
				dev->maxPorts = 11;
				dev->maxPhyNum = 11;
				dev->validPortVec = 0x60F;
				dev->validPhyVec = 0x60F;
				dev->devName = DEV_88E6096;
				break;

		case GT_88E6085:
				dev->numOfPorts = 10;
				dev->maxPorts = 11;
				dev->maxPhyNum = 11;
				dev->validPortVec = 0x6FF;
				dev->validPhyVec = 0x6FF;
				dev->devName = DEV_88E6096;
				break;

		case GT_88E6152:
				dev->numOfPorts = 6;
				dev->maxPorts = 6;
				dev->maxPhyNum = 6;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6182;
				break;

		case GT_88E6155:
				dev->numOfPorts = 6;
				dev->maxPorts = 6;
				dev->maxPhyNum = 6;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6185;
				break;

		case GT_88E6182:
				dev->numOfPorts = 10;
				dev->maxPorts = 10;
				dev->maxPhyNum = 10;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6182;
				break;

		case GT_88E6185:
				dev->numOfPorts = 10;
				dev->maxPorts = 10;
				dev->maxPhyNum = 10;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6185;
				break;

		case GT_88E6121:
				dev->numOfPorts = 3;
				dev->maxPorts = 8;
				dev->maxPhyNum = 3;
				dev->validPortVec = 0xE;	/* port 1, 2, and 3 */
				dev->validPhyVec = 0xE;		/* port 1, 2, and 3 */
				dev->devName = DEV_88E6108;
				break;

		case GT_88E6122:
				dev->numOfPorts = 6;
				dev->maxPorts = 8;
				dev->maxPhyNum = 16;
				dev->validPortVec = 0x7E;	/* port 1 ~ 6 */
				dev->validPhyVec = 0xF07E;	/* port 1 ~ 6, 12 ~ 15 (serdes) */
				dev->devName = DEV_88E6108;
				break;

		case GT_88E6131:
		case GT_88E6108:
				dev->numOfPorts = 8;
				dev->maxPorts = 8;
				dev->maxPhyNum = 16;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = (1 << dev->maxPhyNum) - 1;
				dev->devName = DEV_88E6108;
				break;

		case GT_88E6123:
				dev->numOfPorts = 3;
				dev->maxPorts = 6;
				dev->maxPhyNum = 14;
				dev->validPortVec = 0x23;
				dev->validPhyVec = 0x303F;
				dev->devName = DEV_88E6161;
				break;

		case GT_88E6125:
				dev->numOfPorts = 3;
				dev->maxPorts = 6;
				dev->maxPhyNum = 14;
				dev->validPortVec = 0x23;
				dev->validPhyVec = 0x303F;
				dev->devName = DEV_88E6165;
				break;

		case GT_88E6140:
				dev->numOfPorts = 6;
				dev->maxPorts = 6;
				dev->maxPhyNum = 14;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = 0x303F;
				dev->devName = DEV_88E6165;
				break;

		case GT_88E6161:
				dev->numOfPorts = 6;
				dev->maxPorts = 6;
				dev->maxPhyNum = 14;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = 0x303F;
				dev->devName = DEV_88E6161;
				break;

		case GT_88E6165:
				dev->numOfPorts = 6;
				dev->maxPorts = 6;
				dev->maxPhyNum = 14;
				dev->validPortVec = (1 << dev->numOfPorts) - 1;
				dev->validPhyVec = 0x303F;
				dev->devName = DEV_88E6165;
				break;

		default:
				DBG_INFO(("Unknown Device. Initialization failed\n"));
				return GT_FAIL;
    }

    /* Initialize the MultiAddress Register Access semaphore.    */
    if((dev->multiAddrSem = gtSemCreate(dev,GT_SEM_FULL)) == 0)
    {
        DBG_INFO(("semCreate Failed.\n"));
        qdUnloadDriver(dev);
        return GT_FAIL;
    }

    /* Initialize the ATU semaphore.    */
    if((dev->atuRegsSem = gtSemCreate(dev,GT_SEM_FULL)) == 0)
    {
        DBG_INFO(("semCreate Failed.\n"));
        qdUnloadDriver(dev);
        return GT_FAIL;
    }

    /* Initialize the VTU semaphore.    */
    if((dev->vtuRegsSem = gtSemCreate(dev,GT_SEM_FULL)) == 0)
    {
        DBG_INFO(("semCreate Failed.\n"));
		qdUnloadDriver(dev);
        return GT_FAIL;
    }

    /* Initialize the STATS semaphore.    */
    if((dev->statsRegsSem = gtSemCreate(dev,GT_SEM_FULL)) == 0)
    {
        DBG_INFO(("semCreate Failed.\n"));
		qdUnloadDriver(dev);
        return GT_FAIL;
    }

    /* Initialize the PIRL semaphore.    */
    if((dev->pirlRegsSem = gtSemCreate(dev,GT_SEM_FULL)) == 0)
    {
        DBG_INFO(("semCreate Failed.\n"));
		qdUnloadDriver(dev);
        return GT_FAIL;
    }

    /* Initialize the PTP semaphore.    */
    if((dev->ptpRegsSem = gtSemCreate(dev,GT_SEM_FULL)) == 0)
    {
        DBG_INFO(("semCreate Failed.\n"));
		qdUnloadDriver(dev);
        return GT_FAIL;
    }

    /* Initialize the Table semaphore.    */
    if((dev->tblRegsSem = gtSemCreate(dev,GT_SEM_FULL)) == 0)
    {
        DBG_INFO(("semCreate Failed.\n"));
		qdUnloadDriver(dev);
        return GT_FAIL;
    }

    /* Initialize the PHY Device Register Access semaphore.    */
    if((dev->phyRegsSem = gtSemCreate(dev,GT_SEM_FULL)) == 0)
    {
        DBG_INFO(("semCreate Failed.\n"));
        qdUnloadDriver(dev);
        return GT_FAIL;
    }

    /* Initialize the ports states to forwarding mode. */
    if(cfg->initPorts == GT_TRUE)
    {
		for (port=0; port<dev->numOfPorts; port++)
		{
			if((retVal = gstpSetPortState(dev,port,GT_PORT_FORWARDING)) != GT_OK)
   			{
	    	    DBG_INFO(("Failed.\n"));
				qdUnloadDriver(dev);
   		    	return retVal;
	    	}
		}
    }

    if(cfg->skipInitSetup == GT_SKIP_INIT_SETUP)
	{
	    dev->devEnabled = 1;
    	dev->devNum = cfg->devNum;

	    DBG_INFO(("OK.\n"));
    	return GT_OK;
	}

	if(IS_IN_DEV_GROUP(dev,DEV_ENHANCED_CPU_PORT))
	{
		if((retVal = gsysSetRsvd2CpuEnables(dev,0)) != GT_OK)
		{
	        DBG_INFO(("gsysGetRsvd2CpuEnables failed.\n"));
			qdUnloadDriver(dev);
			return retVal;
		}

		if((retVal = gsysSetRsvd2Cpu(dev,GT_FALSE)) != GT_OK)
		{
	        DBG_INFO(("gsysSetRsvd2Cpu failed.\n"));
			qdUnloadDriver(dev);
			return retVal;
		}
	}

	if (IS_IN_DEV_GROUP(dev,DEV_CPU_DEST_PER_PORT))
	{
		for (port=0; port<dev->numOfPorts; port++)
		{
			retVal = gprtSetCPUPort(dev,port,dev->cpuPortNum);
		    if(retVal != GT_OK)
    		{
	    	    DBG_INFO(("Failed.\n"));
				qdUnloadDriver(dev);
	   	    	return retVal;
		    }
		}
	}

	if(IS_IN_DEV_GROUP(dev,DEV_CPU_PORT))
	{
		retVal = gsysSetCPUPort(dev,dev->cpuPortNum);
	    if(retVal != GT_OK)
   		{
    	    DBG_INFO(("Failed.\n"));
			qdUnloadDriver(dev);
   	    	return retVal;
	    }
	}

	if(IS_IN_DEV_GROUP(dev,DEV_CPU_DEST))
	{
		retVal = gsysSetCPUDest(dev,dev->cpuPortNum);
	    if(retVal != GT_OK)
   		{
    	    DBG_INFO(("Failed.\n"));
			qdUnloadDriver(dev);
   	    	return retVal;
	    }
	}

	if(IS_IN_DEV_GROUP(dev,DEV_MULTICAST))
	{
		if((retVal = gsysSetRsvd2Cpu(dev,GT_FALSE)) != GT_OK)
		{
	        DBG_INFO(("gsysSetRsvd2Cpu failed.\n"));
			qdUnloadDriver(dev);
			return retVal;
		}
	}

	if (IS_IN_DEV_GROUP(dev,DEV_PIRL_RESOURCE))
	{
		retVal = gpirlInitialize(dev);
	    if(retVal != GT_OK)
   		{
    	    DBG_INFO(("Failed.\n"));
			qdUnloadDriver(dev);
   	    	return retVal;
	    }
	}

	if (IS_IN_DEV_GROUP(dev,DEV_PIRL2_RESOURCE))
	{
		retVal = gpirl2Initialize(dev);
	    if(retVal != GT_OK)
   		{
    	    DBG_INFO(("Failed.\n"));
			qdUnloadDriver(dev);
   	    	return retVal;
	    }
	}

    dev->devEnabled = 1;
    dev->devNum = cfg->devNum;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* sysEnable
*
* DESCRIPTION:
*       This function enables the system for full operation.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       1.  This function should be called only after successful execution of
*           qdLoadDriver().
*
*******************************************************************************/
GT_STATUS sysEnable( GT_QD_DEV *dev)
{
    DBG_INFO(("sysEnable Called.\n"));
    DBG_INFO(("OK.\n"));
    return driverEnable(dev);
}


/*******************************************************************************
* qdUnloadDriver
*
* DESCRIPTION:
*       This function unloads the QuaterDeck Driver.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       1.  This function should be called only after successful execution of
*           qdLoadDriver().
*
*******************************************************************************/
GT_STATUS qdUnloadDriver
(
    IN GT_QD_DEV* dev
)
{
    DBG_INFO(("qdUnloadDriver Called.\n"));

    /* Delete the MultiAddress mode reagister access semaphore.    */
    if(gtSemDelete(dev,dev->multiAddrSem) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

    /* Delete the ATU semaphore.    */
    if(gtSemDelete(dev,dev->atuRegsSem) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

    /* Delete the VTU semaphore.    */
    if(gtSemDelete(dev,dev->vtuRegsSem) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

    /* Delete the STATS semaphore.    */
    if(gtSemDelete(dev,dev->statsRegsSem) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

    /* Delete the PIRL semaphore.    */
    if(gtSemDelete(dev,dev->pirlRegsSem) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

    /* Delete the PTP semaphore.    */
    if(gtSemDelete(dev,dev->ptpRegsSem) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

    /* Delete the Table semaphore.    */
    if(gtSemDelete(dev,dev->tblRegsSem) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

    /* Delete the PHY Device semaphore.    */
    if(gtSemDelete(dev,dev->phyRegsSem) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

	gtMemSet(dev,0,sizeof(GT_QD_DEV));
	return GT_OK;
}


/*******************************************************************************
* gtRegister
*
* DESCRIPTION:
*       BSP should register the following functions:
*		1) MII Read - (Input, must provide)
*			allows QuarterDeck driver to read QuarterDeck device registers.
*		2) MII Write - (Input, must provice)
*			allows QuarterDeck driver to write QuarterDeck device registers.
*		3) Semaphore Create - (Input, optional)
*			OS specific Semaphore Creat function.
*		4) Semaphore Delete - (Input, optional)
*			OS specific Semaphore Delete function.
*		5) Semaphore Take - (Input, optional)
*			OS specific Semaphore Take function.
*		6) Semaphore Give - (Input, optional)
*			OS specific Semaphore Give function.
*		Notes: 3) ~ 6) should be provided all or should not be provided at all.
*
* INPUTS:
*		pBSPFunctions - pointer to the structure for above functions.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       GT_TRUE, if input is valid. GT_FALSE, otherwise.
*
* COMMENTS:
*       This function should be called only once.
*
*******************************************************************************/
static GT_BOOL gtRegister(GT_QD_DEV *dev, BSP_FUNCTIONS* pBSPFunctions)
{
	dev->fgtReadMii =  pBSPFunctions->readMii;
	dev->fgtWriteMii = pBSPFunctions->writeMii;

	dev->semCreate = pBSPFunctions->semCreate;
	dev->semDelete = pBSPFunctions->semDelete;
	dev->semTake   = pBSPFunctions->semTake  ;
	dev->semGive   = pBSPFunctions->semGive  ;

	return GT_TRUE;
}


