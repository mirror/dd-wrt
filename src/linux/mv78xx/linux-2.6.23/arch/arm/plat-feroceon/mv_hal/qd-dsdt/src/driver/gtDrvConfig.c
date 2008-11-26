#include <Copyright.h>
/********************************************************************************
* gtDrvConfig.h
*
* DESCRIPTION:
*       Includes driver level configuration and initialization function.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 8 $
*
*******************************************************************************/

#include <gtDrvSwRegs.h>
#include <gtDrvConfig.h>
#include <gtMiiSmiIf.h>
#include <gtHwCntl.h>
#include <gtVct.h>
#include <msApiDefs.h>


/*******************************************************************************
* lport2port
*
* DESCRIPTION:
*       This function converts logical port number to physical port number
*
* INPUTS:
*		portVec - physical port list in vector
*		port    - logical port number
* OUTPUTS:
*		None.
* RETURNS:
*       physical port number
*
* COMMENTS:
*
*******************************************************************************/
GT_U8 lport2port
(
    IN GT_U16    portVec,
	IN GT_LPORT	 port
)
{
    GT_U8	hwPort, tmpPort;

	tmpPort = hwPort = 0;

	while (portVec)
	{
		if(portVec & 0x1)
		{
			if((GT_LPORT)tmpPort == port)
				break;
			tmpPort++;
		}
		hwPort++;
		portVec >>= 1;
	}

	if (!portVec)
		hwPort = GT_INVALID_PORT;

	return hwPort;
}

/*******************************************************************************
* port2lport
*
* DESCRIPTION:
*       This function converts physical port number to logical port number
*
* INPUTS:
*		portVec - physical port list in vector
*		port    - logical port number
* OUTPUTS:
*		None.
* RETURNS:
*       physical port number
*
* COMMENTS:
*
*******************************************************************************/
GT_LPORT port2lport
(
    IN GT_U16    portVec,
	IN GT_U8	 hwPort
)
{
    GT_U8		tmpPort,port;

	port = 0;

	if (hwPort == GT_INVALID_PORT)
		return (GT_LPORT)hwPort;

	if (!GT_IS_PORT_SET(portVec, hwPort))
		return (GT_LPORT)GT_INVALID_PORT;

	for (tmpPort = 0; tmpPort <= hwPort; tmpPort++)
	{
		if(portVec & 0x1)
		{
			port++;
		}
		portVec >>= 1;
	}

	return (GT_LPORT)port-1;
}

/*******************************************************************************
* lportvec2portvec
*
* DESCRIPTION:
*       This function converts logical port vector to physical port vector
*
* INPUTS:
*		portVec - physical port list in vector
*		lVec 	- logical port vector
* OUTPUTS:
*		None.
* RETURNS:
*       physical port vector
*
* COMMENTS:
*
*******************************************************************************/
GT_U32 lportvec2portvec
(
    IN GT_U16    portVec,
	IN GT_U32	 lVec
)
{
    GT_U32	pVec, vec;

	pVec = 0;
	vec = 1;

	while (portVec)
	{
		if(portVec & 0x1)
		{
			if(lVec & 0x1)
			{
				pVec |= vec;
			}
			lVec >>= 1;
		}

		vec <<= 1;
		portVec >>= 1;
	}

	if(lVec)
		return GT_INVALID_PORT_VEC;

	return pVec;
}

/*******************************************************************************
* portvec2lportvec
*
* DESCRIPTION:
*       This function converts physical port vector to logical port vector
*
* INPUTS:
*		portVec - physical port list in vector
*		pVec 	- physical port vector
* OUTPUTS:
*		None.
* RETURNS:
*       logical port vector
*
* COMMENTS:
*
*******************************************************************************/
GT_U32 portvec2lportvec
(
    IN GT_U16    portVec,
	IN GT_U32	 pVec
)
{
    GT_U32	lVec, vec;

	lVec = 0;
	vec = 1;

	while (portVec)
	{
		if(portVec & 0x1)
		{
			if(pVec & 0x1)
			{
				lVec |= vec;
			}
			vec <<= 1;
		}

		pVec >>= 1;
		portVec >>= 1;
	}

	return lVec;
}


/*******************************************************************************
* driverConfig
*
* DESCRIPTION:
*       This function initializes the driver level of the quarterDeck software.
*
* INPUTS:
*		None.
* OUTPUTS:
*		None.
* RETURNS:
*       GT_OK               - on success, or
*       GT_OUT_OF_CPU_MEM   - if failed to allocate CPU memory,
*       GT_FAIL             - otherwise.
*
* COMMENTS:
*       1.  This function should perform the following:
*           -   Initialize the global switch configuration structure.
*           -   Initialize Mii Interface
*
*******************************************************************************/
GT_STATUS driverConfig
(
    IN GT_QD_DEV    *dev
)
{
    GT_U16          deviceId;
    GT_BOOL         highSmiDevAddr;

	if(dev->accessMode == SMI_AUTO_SCAN_MODE)
	{
	    /* Initialize the MII / SMI interface, search for the device */
    	if((deviceId = miiSmiIfInit(dev,&highSmiDevAddr)) == 0)
	    {
    	    return GT_FAIL;
	    }

		dev->baseRegAddr = (highSmiDevAddr)?0x10:0;
	}
	else
	{
    	if((deviceId = miiSmiManualIfInit(dev,(GT_U32)dev->baseRegAddr)) == 0)
	    {
    	    return GT_FAIL;
	    }
	}

    /* Init the device's config struct.             */
    dev->deviceId       = deviceId >> 4;
    dev->revision       = (GT_U8)deviceId & 0xF;

    return GT_OK;
}


/*******************************************************************************
* driverEnable
*
* DESCRIPTION:
*       This function enables the switch for full operation, after the driver
*       Config function was called.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL othrwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS driverEnable
(
	IN GT_QD_DEV    *dev
)
{
    return GT_OK;
}


GT_STATUS driverGetSerdesPort(GT_QD_DEV *dev, GT_U8* hwPort)
{
	switch(dev->devName)
	{
		case DEV_88E6108:
			if ((*hwPort<4) || (*hwPort>7))
			{
				*hwPort = GT_INVALID_PORT;
			}
			else
			{
				*hwPort += 8;
			}
			break;
		default:
			*hwPort = GT_INVALID_PORT;
			break;
	}
	return GT_OK;
}

/*******************************************************************************
* driverFindPhyID
*
* DESCRIPTION:
*       This function get Phy ID from Phy register 2 and 3.
*
* INPUTS:
*       hwPort	- port number where the Phy is connected
*
* OUTPUTS:
*       phyId	- Phy ID
*
* RETURNS:
*       GT_OK 	- if found Marvell Phy,
*       GT_FAIL - othrwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS driverFindPhyID
(
	IN  GT_QD_DEV    *dev,
	IN	GT_U8		 hwPort,
	OUT	GT_U32		 *phyID
)
{
	GT_U16 ouiMsb, ouiLsb;
	GT_STATUS status;

	if((status= hwReadPhyReg(dev,hwPort,2,&ouiMsb)) != GT_OK)
	{
	    DBG_INFO(("Not able to read Phy Register.\n"));
		return status;
	}

	if((status= hwReadPhyReg(dev,hwPort,3,&ouiLsb)) != GT_OK)
	{
	    DBG_INFO(("Not able to read Phy Register.\n"));
		return status;
	}

	if(ouiMsb != MARVELL_OUI_MSb)
		return GT_FAIL;

	*phyID = (GT_U32)ouiLsb;

    return GT_OK;
}


/*******************************************************************************
* driverIsPhyAttached
*
* DESCRIPTION:
*       This function reads and returns Phy ID (register 3) of Marvell Phy.
*
* INPUTS:
*       hwPort	- port number where the Phy is connected
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       phyId - if Marvell Phy exists
*		0	  - otherwise
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_U32 driverIsPhyAttached
(
	IN  GT_QD_DEV    *dev,
	IN	GT_U8		 hwPort
)
{
	GT_U32		 phyId;

	if(hwPort > dev->maxPorts)
		return 0;

	if(driverFindPhyID(dev,hwPort,&phyId) != GT_OK)
	{
	    DBG_INFO(("cannot find Marvell Phy.\n"));
		return 0;
	}

	return phyId;
}

/*******************************************************************************
* driverPagedAccessStart
*
* DESCRIPTION:
*       This function stores page register and Auto Reg Selection mode if needed.
*
* INPUTS:
*       hwPort	 - port number where the Phy is connected
*		pageType - type of the page registers
*
* OUTPUTS:
*       autoOn	- GT_TRUE if Auto Reg Selection enabled, GT_FALSE otherwise.
*		pageReg - Page Register Data
*
* RETURNS:
*       GT_OK 	- if success
*       GT_FAIL - othrwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS driverPagedAccessStart
(
	IN  GT_QD_DEV    *dev,
	IN	GT_U8		 hwPort,
	IN	GT_U8		 pageType,
	OUT	GT_BOOL		 *autoOn,
	OUT	GT_U16		 *pageReg
)
{
	GT_U16 data;
	GT_STATUS status;

	switch(pageType)
	{
		case GT_PHY_PAGE_WRITE_BACK:
			break;
		case GT_PHY_PAGE_DIS_AUTO1:	/* 88E1111 Type */
			if((status= hwGetPhyRegField(dev,hwPort,27,9,1,&data)) != GT_OK)
			{
	    		DBG_INFO(("Not able to read Phy Register.\n"));
				return status;
			}

			data ^= 0x1;	/* toggle bit 0 */
		    BIT_2_BOOL(data, *autoOn);

			if (*autoOn) /* Auto On */
			{
				if((status= hwSetPhyRegField(dev,hwPort,27,9,1,data)) != GT_OK)
				{
	    			DBG_INFO(("Not able to write Phy Register.\n"));
					return status;
				}
			}

			break;
		case GT_PHY_PAGE_DIS_AUTO2:	/* 88E1112 Type */
			if((status= hwGetPhyRegField(dev,hwPort,22,15,1,&data)) != GT_OK)
			{
	    		DBG_INFO(("Not able to read Phy Register.\n"));
				return status;
			}

		    BIT_2_BOOL(data, *autoOn);
			data ^= 0x1;	/* toggle bit 0 */

			if (*autoOn) /* Auto On */
			{
				if((status= hwSetPhyRegField(dev,hwPort,22,15,1,data)) != GT_OK)
				{
	    			DBG_INFO(("Not able to write Phy Register.\n"));
					return status;
				}
			}

			break;

		case GT_PHY_NO_PAGE:
		default:
			/* Nothing to do */
			return GT_OK;
	}


	if((status= hwGetPhyRegField(dev,hwPort,22,0,8,pageReg)) != GT_OK)
	{
	    DBG_INFO(("Not able to read Phy Register.\n"));
		return status;
	}

    return GT_OK;
}


/*******************************************************************************
* driverPagedAccessStop
*
* DESCRIPTION:
*       This function restores page register and Auto Reg Selection mode if needed.
*
* INPUTS:
*       hwPort	 - port number where the Phy is connected
*		pageType - type of the page registers
*       autoOn	 - GT_TRUE if Auto Reg Selection enabled, GT_FALSE otherwise.
*		pageReg  - Page Register Data
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       GT_OK 	- if success
*       GT_FAIL - othrwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS driverPagedAccessStop
(
	IN  GT_QD_DEV    *dev,
	IN	GT_U8		 hwPort,
	IN	GT_U8		 pageType,
	IN	GT_BOOL		 autoOn,
	IN	GT_U16		 pageReg
)
{
	GT_U16 data;
	GT_STATUS status;

	switch(pageType)
	{
		case GT_PHY_PAGE_WRITE_BACK:
			break;
		case GT_PHY_PAGE_DIS_AUTO1:	/* 88E1111 Type */
			if (autoOn) /* Auto On */
			{
				data = 0;
				if((status= hwSetPhyRegField(dev,hwPort,27,9,1,data)) != GT_OK)
				{
	    			DBG_INFO(("Not able to write Phy Register.\n"));
					return status;
				}
			}

			break;
		case GT_PHY_PAGE_DIS_AUTO2:	/* 88E1112 Type */
			if (autoOn) /* Auto On */
			{
				data = 1;
				if((status= hwSetPhyRegField(dev,hwPort,22,15,1,data)) != GT_OK)
				{
	    			DBG_INFO(("Not able to write Phy Register.\n"));
					return status;
				}
			}

			break;

		case GT_PHY_NO_PAGE:
		default:
			/* Nothing to do */
			return GT_OK;
	}


	if((status= hwSetPhyRegField(dev,hwPort,22,0,8,pageReg)) != GT_OK)
	{
	    DBG_INFO(("Not able to write Phy Register.\n"));
		return status;
	}

    return GT_OK;
}


/*******************************************************************************
* driverFindPhyInformation
*
* DESCRIPTION:
*       This function gets information of Phy connected to the given port.
*		PhyInfo structure should have valid Phy ID.
*
* INPUTS:
*       hwPort	- port number where the Phy is connected
*
* OUTPUTS:
*       phyId	- Phy ID
*
* RETURNS:
*       GT_OK 	- if found Marvell Phy,
*       GT_FAIL - othrwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS driverFindPhyInformation
(
	IN  GT_QD_DEV    *dev,
	IN	GT_U8		 hwPort,
	OUT	GT_PHY_INFO	 *phyInfo
)
{
	GT_U32 phyId;
	GT_U16 data;

	phyId = phyInfo->phyId;

	switch (phyId & PHY_MODEL_MASK)
	{
		case DEV_E3082:
				phyInfo->anyPage = 0xFFFFFFFF;
				phyInfo->flag = GT_PHY_VCT_CAPABLE|GT_PHY_DTE_CAPABLE|
								GT_PHY_MAC_IF_LOOP|GT_PHY_EXTERNAL_LOOP|
								GT_PHY_COPPER;
				phyInfo->vctType = GT_PHY_VCT_TYPE1;
				if ((phyId & PHY_REV_MASK) < 9)
					phyInfo->dteType = GT_PHY_DTE_TYPE1;	/* need workaround */
				else
					phyInfo->dteType = GT_PHY_DTE_TYPE5;

				phyInfo->pktGenType = 0;
				phyInfo->macIfLoopType = GT_PHY_LOOPBACK_TYPE1;
				phyInfo->lineLoopType = 0;
				phyInfo->exLoopType = GT_PHY_EX_LB_TYPE0;
				phyInfo->pageType = GT_PHY_NO_PAGE;
				break;

	    case DEV_E104X:
				phyInfo->anyPage = 0xFFFFFFFF;
				phyInfo->flag = GT_PHY_VCT_CAPABLE|GT_PHY_GIGABIT|
								GT_PHY_MAC_IF_LOOP|GT_PHY_EXTERNAL_LOOP;

				phyInfo->dteType = 0;
				if ((phyId & PHY_REV_MASK) < 3)
					phyInfo->flag &= ~GT_PHY_VCT_CAPABLE; /* VCT is not supported */
				else if ((phyId & PHY_REV_MASK) == 3)
					phyInfo->vctType = GT_PHY_VCT_TYPE3;	/* Need workaround */
				else
					phyInfo->vctType = GT_PHY_VCT_TYPE2;

				phyInfo->pktGenType = 0;
				phyInfo->macIfLoopType = GT_PHY_LOOPBACK_TYPE1;
				phyInfo->lineLoopType = 0;
				phyInfo->exLoopType = GT_PHY_EX_LB_TYPE0;
				phyInfo->pageType = GT_PHY_NO_PAGE;

				break;

		case DEV_E1111:
				phyInfo->anyPage = 0xFFF1FE0C;
				phyInfo->flag = GT_PHY_VCT_CAPABLE|GT_PHY_DTE_CAPABLE|
								GT_PHY_EX_CABLE_STATUS|
								GT_PHY_MAC_IF_LOOP|GT_PHY_LINE_LOOP|GT_PHY_EXTERNAL_LOOP|
								GT_PHY_GIGABIT|GT_PHY_RESTRICTED_PAGE;

				phyInfo->vctType = GT_PHY_VCT_TYPE2;
				if ((phyId & PHY_REV_MASK) < 2)
					phyInfo->dteType = GT_PHY_DTE_TYPE3;	/* Need workaround */
				else
					phyInfo->dteType = GT_PHY_DTE_TYPE2;

				phyInfo->pktGenType = GT_PHY_PKTGEN_TYPE1;
				phyInfo->macIfLoopType = GT_PHY_LOOPBACK_TYPE1;
				phyInfo->lineLoopType = 0;
				phyInfo->exLoopType = GT_PHY_EX_LB_TYPE0;
				phyInfo->pageType = GT_PHY_PAGE_DIS_AUTO1;
				break;

		case DEV_E1112:
				phyInfo->anyPage = 0xFBC0780C;
				phyInfo->flag = GT_PHY_VCT_CAPABLE|GT_PHY_DTE_CAPABLE|
								GT_PHY_EX_CABLE_STATUS|
								GT_PHY_GIGABIT|GT_PHY_RESTRICTED_PAGE|
								GT_PHY_MAC_IF_LOOP|GT_PHY_LINE_LOOP|GT_PHY_EXTERNAL_LOOP|
								GT_PHY_PKT_GENERATOR;

				phyInfo->vctType = GT_PHY_VCT_TYPE4;
				phyInfo->dteType = GT_PHY_DTE_TYPE4;

				phyInfo->pktGenType = GT_PHY_PKTGEN_TYPE2;
				phyInfo->macIfLoopType = GT_PHY_LOOPBACK_TYPE1;
				phyInfo->lineLoopType = 0;
				phyInfo->exLoopType = GT_PHY_EX_LB_TYPE0;
				phyInfo->pageType = GT_PHY_PAGE_DIS_AUTO2;
				break;

		case DEV_E114X:
				phyInfo->anyPage = 0xFFF1FE0C;
				phyInfo->flag = GT_PHY_VCT_CAPABLE|GT_PHY_DTE_CAPABLE|
								GT_PHY_EX_CABLE_STATUS|
								GT_PHY_MAC_IF_LOOP|GT_PHY_LINE_LOOP|GT_PHY_EXTERNAL_LOOP|
								GT_PHY_GIGABIT|GT_PHY_RESTRICTED_PAGE;

				phyInfo->vctType = GT_PHY_VCT_TYPE2;
				if ((phyId & PHY_REV_MASK) < 4)
					phyInfo->dteType = GT_PHY_DTE_TYPE3;	/* Need workaround */
				else
					phyInfo->dteType = GT_PHY_DTE_TYPE2;

				phyInfo->pktGenType = GT_PHY_PKTGEN_TYPE1;
				phyInfo->macIfLoopType = GT_PHY_LOOPBACK_TYPE1;
				phyInfo->lineLoopType = 0;
				phyInfo->exLoopType = GT_PHY_EX_LB_TYPE0;
				phyInfo->pageType = GT_PHY_PAGE_DIS_AUTO1;

				break;

		case DEV_E1149:
				phyInfo->anyPage = 0x2040FFFF;
				phyInfo->flag = GT_PHY_VCT_CAPABLE|GT_PHY_DTE_CAPABLE|
								GT_PHY_EX_CABLE_STATUS|
								GT_PHY_GIGABIT|
								GT_PHY_MAC_IF_LOOP|GT_PHY_LINE_LOOP|GT_PHY_EXTERNAL_LOOP|
								GT_PHY_PKT_GENERATOR;
				phyInfo->vctType = GT_PHY_VCT_TYPE4;
				phyInfo->dteType = GT_PHY_DTE_TYPE4;
				phyInfo->pktGenType = GT_PHY_PKTGEN_TYPE2;
				phyInfo->macIfLoopType = GT_PHY_LOOPBACK_TYPE1;
				phyInfo->lineLoopType = 0;
				phyInfo->exLoopType = GT_PHY_EX_LB_TYPE0;
				phyInfo->pageType = GT_PHY_PAGE_WRITE_BACK;
				break;

		case DEV_G15LV:
				phyInfo->anyPage = 0x2040FFFF;
				phyInfo->flag = GT_PHY_VCT_CAPABLE|GT_PHY_DTE_CAPABLE|
								GT_PHY_EX_CABLE_STATUS|
								GT_PHY_GIGABIT|
								GT_PHY_MAC_IF_LOOP|GT_PHY_LINE_LOOP|GT_PHY_EXTERNAL_LOOP|
								GT_PHY_PKT_GENERATOR;
				phyInfo->vctType = GT_PHY_VCT_TYPE4;
				phyInfo->dteType = GT_PHY_DTE_TYPE4;
				phyInfo->pktGenType = GT_PHY_PKTGEN_TYPE2;
				phyInfo->macIfLoopType = GT_PHY_LOOPBACK_TYPE1;
				phyInfo->lineLoopType = 0;
				phyInfo->exLoopType = GT_PHY_EX_LB_TYPE0;
				phyInfo->pageType = GT_PHY_PAGE_WRITE_BACK;
				break;

		case DEV_EC010:
				phyInfo->anyPage = 0x2040780C;
				phyInfo->flag = GT_PHY_VCT_CAPABLE|GT_PHY_DTE_CAPABLE|
								GT_PHY_EX_CABLE_STATUS|
								GT_PHY_GIGABIT|GT_PHY_RESTRICTED_PAGE|
								GT_PHY_MAC_IF_LOOP|GT_PHY_LINE_LOOP|GT_PHY_EXTERNAL_LOOP;
				phyInfo->vctType = GT_PHY_VCT_TYPE2;
				phyInfo->dteType = GT_PHY_DTE_TYPE3;	/* Need workaround */
				phyInfo->pktGenType = 0;
				phyInfo->macIfLoopType = GT_PHY_LOOPBACK_TYPE1;
				phyInfo->lineLoopType = 0;
				phyInfo->exLoopType = GT_PHY_EX_LB_TYPE0;
				phyInfo->pageType = GT_PHY_PAGE_WRITE_BACK;
				break;

		case DEV_S15LV:
				phyInfo->anyPage = 0xFFFFFFFF;
				phyInfo->flag = GT_PHY_SERDES_CORE|
								GT_PHY_MAC_IF_LOOP|GT_PHY_LINE_LOOP|GT_PHY_EXTERNAL_LOOP|
								GT_PHY_PKT_GENERATOR;
				phyInfo->vctType = 0;
				phyInfo->dteType = 0;
				phyInfo->pktGenType = GT_PHY_PKTGEN_TYPE3;
				phyInfo->macIfLoopType = GT_PHY_LOOPBACK_TYPE3;
				phyInfo->lineLoopType = GT_PHY_LINE_LB_TYPE4;
				phyInfo->exLoopType = 0;
				phyInfo->pageType = GT_PHY_NO_PAGE;
				break;

		default:
			return GT_FAIL;
	}

	if (phyInfo->flag & GT_PHY_GIGABIT)
	{
	    if(hwGetPhyRegField(dev,hwPort,15,12,4,&data) != GT_OK)
		{
        	DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,15));
	   	    return GT_FAIL;
		}

		if(data & QD_GIGPHY_1000X_CAP)
			phyInfo->flag |= GT_PHY_FIBER;

		if(data & QD_GIGPHY_1000T_CAP)
		{
			phyInfo->flag |= GT_PHY_COPPER;
		}
		else
		{
			phyInfo->flag &= ~(GT_PHY_VCT_CAPABLE|GT_PHY_EX_CABLE_STATUS|GT_PHY_DTE_CAPABLE);
		}
	}

    return GT_OK;
}


