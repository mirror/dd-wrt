#include <Copyright.h>

/********************************************************************************
* gtPhyCtrl.h
* 
* DESCRIPTION:
* API definitions for PHY control facility.
*
* DEPENDENCIES:
* None.
*
* FILE REVISION NUMBER:
* $Revision: 10 $
*******************************************************************************/

#include <msApi.h>
#include <gtHwCntl.h>
#include <gtDrvConfig.h>
#include <gtDrvSwRegs.h>
#include <gtVct.h>
#include <gtSem.h>
#include <linux/kernel.h>
/*
 * This routine set Auto-Negotiation Ad Register for Fast Ethernet Phy
*/
static 
GT_STATUS feSetAutoMode
(
	IN GT_QD_DEV *dev,
	IN GT_U8 	 hwPort,
	IN GT_PHY_INFO	 *phyInfo,
	IN GT_PHY_AUTO_MODE mode
)
{
    GT_U16 			u16Data;

    DBG_INFO(("feSetAutoMode Called.\n"));

    if(hwReadPhyReg(dev,hwPort,QD_PHY_AUTONEGO_AD_REG,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_AUTONEGO_AD_REG));
   	    return GT_FAIL;
	}

	/* Mask out all auto mode related bits. */
	u16Data &= ~QD_PHY_MODE_AUTO_AUTO;
	printk("feSetAutoMode: mode = %i\n",mode);
	switch(mode)
	{
		case SPEED_AUTO_DUPLEX_AUTO:
				u16Data |= QD_PHY_MODE_AUTO_AUTO;
				break;
		case SPEED_100_DUPLEX_AUTO:
				u16Data |= QD_PHY_MODE_100_AUTO;
				break;
		case SPEED_10_DUPLEX_AUTO:
				u16Data |= QD_PHY_MODE_10_AUTO;
				break;
		case SPEED_AUTO_DUPLEX_FULL:
				u16Data |= QD_PHY_MODE_AUTO_FULL;
				break;
		case SPEED_AUTO_DUPLEX_HALF:
				u16Data |= QD_PHY_MODE_AUTO_HALF;
				break;
		case SPEED_100_DUPLEX_FULL:
				u16Data |= QD_PHY_100_FULL;
				break;
		case SPEED_100_DUPLEX_HALF:
				u16Data |= QD_PHY_100_HALF;
				break;
		case SPEED_10_DUPLEX_FULL:
				u16Data |= QD_PHY_10_FULL;
				break;
		case SPEED_10_DUPLEX_HALF:
				u16Data |= QD_PHY_10_HALF;
				break;
/* set to 10/100 auto mode */
                case SPEED_10_100_DUPLEX_AUTO:
	                    u16Data |= QD_PHY_MODE_AUTO_AUTO;
			    printk(" case SPEED_10_100_DUPLEX_AUTO\n");
	                    break;

		default:
	 	        DBG_INFO(("Unknown Auto Mode (%d)\n",mode));
				return GT_BAD_PARAM;
	}

    /* Write to Phy AutoNegotiation Advertisement Register.  */
    if(hwWritePhyReg(dev,hwPort,QD_PHY_AUTONEGO_AD_REG,u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to write Phy Reg(port:%d,offset:%d,data:%#x).\n",hwPort,QD_PHY_AUTONEGO_AD_REG,u16Data));
   	    return GT_FAIL;
	}

	return GT_OK;
}

/*
 * This routine set Auto-Negotiation Ad Register for Copper
*/
static 
GT_STATUS gigCopperSetAutoMode
(
	IN GT_QD_DEV *dev,
	IN GT_U8 hwPort,
	IN GT_PHY_INFO	 *phyInfo,
	IN GT_PHY_AUTO_MODE mode
)
{
    GT_U16 			u16Data,u16Data1;

    DBG_INFO(("gigCopperSetAutoMode Called.\n"));

    if(hwReadPagedPhyReg(dev,hwPort,0,QD_PHY_AUTONEGO_AD_REG,phyInfo->anyPage,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_AUTONEGO_AD_REG));
   	    return GT_FAIL;
	}

	/* Mask out all auto mode related bits. */
	u16Data &= ~QD_PHY_MODE_AUTO_AUTO;

    if(hwReadPagedPhyReg(dev,hwPort,0,QD_PHY_AUTONEGO_1000AD_REG,phyInfo->anyPage,&u16Data1) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_AUTONEGO_AD_REG));
   	    return GT_FAIL;
	}

	/* Mask out all auto mode related bits. */
	u16Data1 &= ~(QD_GIGPHY_1000T_FULL|QD_GIGPHY_1000T_HALF);

	switch(mode)
	{
		case SPEED_AUTO_DUPLEX_AUTO:
				u16Data |= QD_PHY_MODE_AUTO_AUTO;
		case SPEED_1000_DUPLEX_AUTO:
				u16Data1 |= QD_GIGPHY_1000T_FULL|QD_GIGPHY_1000T_HALF;
				break;
		case SPEED_AUTO_DUPLEX_FULL:
				u16Data  |= QD_PHY_MODE_AUTO_FULL;
				u16Data1 |= QD_GIGPHY_1000T_FULL;
				break;
		case SPEED_1000_DUPLEX_FULL:
				u16Data1 |= QD_GIGPHY_1000T_FULL;
				break;
		case SPEED_1000_DUPLEX_HALF:
				u16Data1 |= QD_GIGPHY_1000T_HALF;
				break;
		case SPEED_AUTO_DUPLEX_HALF:
				u16Data  |= QD_PHY_MODE_AUTO_HALF;
				u16Data1 |= QD_GIGPHY_1000T_HALF;
				break;
		case SPEED_100_DUPLEX_AUTO:
				u16Data |= QD_PHY_MODE_100_AUTO;
				break;
		case SPEED_10_DUPLEX_AUTO:
				u16Data |= QD_PHY_MODE_10_AUTO;
				break;
		case SPEED_100_DUPLEX_FULL:
				u16Data |= QD_PHY_100_FULL;
				break;
		case SPEED_100_DUPLEX_HALF:
				u16Data |= QD_PHY_100_HALF;
				break;
		case SPEED_10_DUPLEX_FULL:
				u16Data |= QD_PHY_10_FULL;
				break;
		case SPEED_10_DUPLEX_HALF:
				u16Data |= QD_PHY_10_HALF;
				break;
/* set to 10/100 auto mode */
                case SPEED_10_100_DUPLEX_AUTO:
		   printk("caseSPEED_10_100_DUPLEX_AUTO\n");
                    u16Data |= QD_PHY_MODE_AUTO_AUTO;
                    break;
		default:
				DBG_INFO(("Unknown Auto Mode (%d)\n",mode));
				return GT_BAD_PARAM;
	}

    /* Write to Phy AutoNegotiation Advertisement Register.  */
    if(hwWritePagedPhyReg(dev,hwPort,0,QD_PHY_AUTONEGO_AD_REG,phyInfo->anyPage,u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to write Phy Reg(port:%d,offset:%d,data:%#x).\n",hwPort,QD_PHY_AUTONEGO_AD_REG,u16Data));
   	    return GT_FAIL;
	}

    /* Write to Phy AutoNegotiation 1000B Advertisement Register.  */
    if(hwWritePagedPhyReg(dev,hwPort,0,QD_PHY_AUTONEGO_1000AD_REG,phyInfo->anyPage,u16Data1) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_AUTONEGO_AD_REG));
   	    return GT_FAIL;
	}

	return GT_OK;
}

/*
 * This routine set Auto-Negotiation Ad Register for Fiber
*/
static 
GT_STATUS gigFiberSetAutoMode
(
	IN GT_QD_DEV *dev,
	IN GT_U8 hwPort,
	IN GT_PHY_INFO	 *phyInfo,
	IN GT_PHY_AUTO_MODE mode
)
{
    GT_U16 			u16Data;

    DBG_INFO(("gigPhySetAutoMode Called.\n"));

    if(hwReadPagedPhyReg(dev,hwPort,1,QD_PHY_AUTONEGO_AD_REG,phyInfo->anyPage,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_AUTONEGO_AD_REG));
   	    return GT_FAIL;
	}

	/* Mask out all auto mode related bits. */
	u16Data &= ~(QD_GIGPHY_1000X_FULL|QD_GIGPHY_1000X_HALF);

	switch(mode)
	{
		case SPEED_AUTO_DUPLEX_AUTO:
		case SPEED_1000_DUPLEX_AUTO:
				u16Data |= QD_GIGPHY_1000X_FULL|QD_GIGPHY_1000X_HALF;
				break;
		case SPEED_AUTO_DUPLEX_FULL:
		case SPEED_1000_DUPLEX_FULL:
				u16Data |= QD_GIGPHY_1000X_FULL;
				break;
		case SPEED_AUTO_DUPLEX_HALF:
		case SPEED_1000_DUPLEX_HALF:
				u16Data |= QD_GIGPHY_1000X_HALF;
				break;
		default:
	 	       	DBG_INFO(("Unknown Auto Mode (%d)\n",mode));
				return GT_BAD_PARAM;
	}

    /* Write to Phy AutoNegotiation Advertisement Register.  */
    if(hwWritePagedPhyReg(dev,hwPort,1,QD_PHY_AUTONEGO_AD_REG,phyInfo->anyPage,u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to write Phy Reg(port:%d,offset:%d,data:%#x).\n",hwPort,QD_PHY_AUTONEGO_AD_REG,u16Data));
   	    return GT_FAIL;
	}

	return GT_OK;
}

/*
 * This routine sets Auto Mode and Reset the phy
*/
static 
GT_STATUS phySetAutoMode
(
	IN GT_QD_DEV *dev,
	IN GT_U8 hwPort,
	IN GT_PHY_INFO *phyInfo,
	IN GT_PHY_AUTO_MODE mode
)
{
    GT_U16 		u16Data;
	GT_STATUS	status;
	GT_BOOL			autoOn;
	GT_U16			pageReg;

    DBG_INFO(("phySetAutoMode Called.\n"));

	if (!(phyInfo->flag & GT_PHY_GIGABIT))
	{
		if((status=feSetAutoMode(dev,hwPort,phyInfo,mode)) != GT_OK)
		{
   		    return status;
		}

		u16Data = QD_PHY_SPEED | QD_PHY_DUPLEX | QD_PHY_AUTONEGO;

    	DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
        	      hwPort,QD_PHY_CONTROL_REG,u16Data));

		/* soft reset */
		return hwPhyReset(dev,hwPort,u16Data);
	}

	if(driverPagedAccessStart(dev,hwPort,phyInfo->pageType,&autoOn,&pageReg) != GT_OK)
	{
		return GT_FAIL;
	}

	if(phyInfo->flag & GT_PHY_FIBER)
	{
		if((status=gigFiberSetAutoMode(dev,hwPort,phyInfo,mode)) != GT_OK)
		{
   		    return status;
		}
		u16Data = QD_PHY_AUTONEGO;

    	DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
        	      hwPort,QD_PHY_CONTROL_REG,u16Data));

	    /* Write to Phy Control Register.  */
	    if(hwWritePagedPhyReg(dev,hwPort,1,QD_PHY_CONTROL_REG,phyInfo->anyPage,u16Data) != GT_OK)
    		return GT_FAIL;
	}

	if(phyInfo->flag & GT_PHY_COPPER)
	{
		if((status=gigCopperSetAutoMode(dev,hwPort,phyInfo,mode)) != GT_OK)
		{
   		    return status;
		}

		u16Data = QD_PHY_AUTONEGO;

    	DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
        	      hwPort,QD_PHY_CONTROL_REG,u16Data));

	    /* Write to Phy Control Register.  */
	    if(hwWritePagedPhyReg(dev,hwPort,0,QD_PHY_CONTROL_REG,phyInfo->anyPage,u16Data) != GT_OK)
    		return GT_FAIL;
	}

	if(driverPagedAccessStop(dev,hwPort,phyInfo->pageType,autoOn,pageReg) != GT_OK)
	{
		return GT_FAIL;
	}

	return hwPhyReset(dev,hwPort,0xFF);
}


GT_STATUS gprtSetLed(	IN GT_U8  hwPort, GT_16 data1, GT_16 data2)
{
extern GT_QD_DEV* qd_dev;

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_PHY_INFO		phyInfo;

 	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(qd_dev,hwPort)) == 0)
	{
		return GT_NOT_SUPPORTED;
	}

	if(driverFindPhyInformation(qd_dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		return GT_FAIL;
	}

	//retVal = phySetAutoMode(qd_dev,hwPort,&phyInfo,SPEED_AUTO_DUPLEX_AUTO);
// Gemtek test
	printk("gprtSetLed: port=%d,data1=%x,data2=%x\n",hwPort,data1,data2);
//	printk("mv_gtw_main: before setup register 17 = %x,ret=%x\n",hwPort,data1); 	
	retVal = hwWritePagedPhyReg(qd_dev,hwPort,3,17,phyInfo.anyPage,data1);
	retVal = hwWritePagedPhyReg(qd_dev,hwPort,3,16,phyInfo.anyPage,data2);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}
    return retVal;
}


/*******************************************************************************
* gprtPhyReset
*
* DESCRIPTION:
*       This routine preforms PHY reset.
*		After reset, phy will be in Autonegotiation mode.
*
* INPUTS:
* port - The logical port number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
* COMMENTS:
* data sheet register 0.15 - Reset
* data sheet register 0.13 - Speed
* data sheet register 0.12 - Autonegotiation
* data sheet register 0.8  - Duplex Mode
*******************************************************************************/

GT_STATUS gprtPhyReset
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
	GT_PHY_INFO		phyInfo;

    DBG_INFO(("gprtPhyReset Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	/* set Auto Negotiation AD Register */
	retVal = phySetAutoMode(dev,hwPort,&phyInfo,SPEED_AUTO_DUPLEX_AUTO);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

	gtSemGive(dev,dev->phyRegsSem);

    return retVal;
}

GT_STATUS gprtPhyReset2
(
	IN GT_QD_DEV *dev,
	IN GT_U8  port
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
	GT_PHY_INFO		phyInfo;

    DBG_INFO(("gprtPhyReset Called.\n"));

    hwPort = (port);
	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	/* set Auto Negotiation AD Register */
	retVal = phySetAutoMode(dev,hwPort,&phyInfo,SPEED_AUTO_DUPLEX_AUTO);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}
		gtSemGive(dev,dev->phyRegsSem);
    return retVal;
}


/*******************************************************************************
* gprtSetPortLoopback
*
* DESCRIPTION:
* Enable/Disable Internal Port Loopback. 
* For 10/100 Fast Ethernet PHY, speed of Loopback is determined as follows:
*   If Auto-Negotiation is enabled, this routine disables Auto-Negotiation and 
*   forces speed to be 10Mbps.
*   If Auto-Negotiation is disabled, the forced speed is used.
*   Disabling Loopback simply clears bit 14 of control register(0.14). Therefore,
*   it is recommended to call gprtSetPortAutoMode for PHY configuration after 
*   Loopback test.
* For 10/100/1000 Gigagbit Ethernet PHY, speed of Loopback is determined as follows:
*   If Auto-Negotiation is enabled and Link is active, the current speed is used.
*   If Auto-Negotiation is disabled, the forced speed is used.
*   All other cases, default MAC Interface speed is used. Please refer to the data
*   sheet for the information of the default MAC Interface speed.
*   
*
* INPUTS:
* port - logical port number
* enable - If GT_TRUE, enable loopback mode
* If GT_FALSE, disable loopback mode
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.14 - Loop_back
*
*******************************************************************************/

GT_STATUS gprtSetPortLoopback
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port,
	IN GT_BOOL   enable
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;
	GT_PHY_INFO		phyInfo;

    DBG_INFO(("gprtSetPortLoopback Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

    if(hwReadPhyReg(dev,hwPort,QD_PHY_CONTROL_REG,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_CONTROL_REG));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
	}

	/* is this Fast Ethernet Phy? */
	if (!(phyInfo.flag & GT_PHY_GIGABIT))
	{
		if(enable)
		{
			if(u16Data & QD_PHY_AUTONEGO)
			{
				/* disable Auto-Neg and force speed to be 10Mbps */
				u16Data = u16Data & QD_PHY_DUPLEX;

				if((retVal=hwPhyReset(dev,hwPort,u16Data)) != GT_OK)
				{
					DBG_INFO(("Softreset failed.\n"));
					gtSemGive(dev,dev->phyRegsSem);
					return retVal;
				}
			}
		}
	}

	BOOL_2_BIT(enable,u16Data);

    /* Write to Phy Control Register.  */
    retVal = hwSetPhyRegField(dev,hwPort,QD_PHY_CONTROL_REG,14,1,u16Data);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}
	gtSemGive(dev,dev->phyRegsSem);
    return retVal;
}


/*******************************************************************************
* gprtSetPortSpeed
*
* DESCRIPTION:
* 		Sets speed for a specific logical port. This function will keep the duplex 
*		mode and loopback mode to the previous value, but disable others, such as 
*		Autonegotiation.
*
* INPUTS:
* 		port  - logical port number
* 		speed - port speed.
*				PHY_SPEED_10_MBPS for 10Mbps
*				PHY_SPEED_100_MBPS for 100Mbps
*				PHY_SPEED_1000_MBPS for 1000Mbps
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* data sheet register 0.13 - Speed Selection (LSB)
* data sheet register 0.6  - Speed Selection (MSB)
*
*******************************************************************************/

GT_STATUS gprtSetPortSpeed
(
IN GT_QD_DEV *dev,
IN GT_LPORT  port,
IN GT_PHY_SPEED speed
)
{
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;
	GT_PHY_INFO		phyInfo;
	GT_STATUS		retVal;

    DBG_INFO(("gprtSetPortSpeed Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

    if(hwReadPhyReg(dev,hwPort,QD_PHY_CONTROL_REG,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_CONTROL_REG));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
	}

	switch(speed)
	{
		case PHY_SPEED_10_MBPS:
			u16Data = u16Data & (QD_PHY_LOOPBACK | QD_PHY_DUPLEX);
			break;
		case PHY_SPEED_100_MBPS:
			u16Data = (u16Data & (QD_PHY_LOOPBACK | QD_PHY_DUPLEX)) | QD_PHY_SPEED;
			break;
		case PHY_SPEED_1000_MBPS:
			if (!(phyInfo.flag & GT_PHY_GIGABIT))
			{
				gtSemGive(dev,dev->phyRegsSem);
				return GT_BAD_PARAM;
			}
			u16Data = (u16Data & (QD_PHY_LOOPBACK | QD_PHY_DUPLEX)) | QD_PHY_SPEED_MSB;
			break;
		default:
			gtSemGive(dev,dev->phyRegsSem);
			return GT_FAIL;
	}

    DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
              hwPort,QD_PHY_CONTROL_REG,u16Data));

	retVal = hwPhyReset(dev,hwPort,u16Data);
  	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}


/*******************************************************************************
* gprtPortAutoNegEnable
*
* DESCRIPTION:
* 		Enable/disable an Auto-Negotiation.
*		This routine simply sets Auto Negotiation bit (bit 12) of Control 
*		Register and reset the phy.
*		For Speed and Duplex selection, please use gprtSetPortAutoMode.
*
* INPUTS:
* 		port - logical port number
* 		state - GT_TRUE for enable Auto-Negotiation,
*				GT_FALSE otherwise
*
* OUTPUTS:
* 		None.
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL 	- on error
*
* COMMENTS:
* 		data sheet register 0.12 - Auto-Negotiation Enable
* 		data sheet register 4.8, 4.7, 4.6, 4.5 - Auto-Negotiation Advertisement
*
*******************************************************************************/
GT_STATUS gprtPortAutoNegEnable
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port,
	IN GT_BOOL   state
)
{
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;
	GT_STATUS		retVal;

    DBG_INFO(("gprtPortAutoNegEnable Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

    if(hwReadPhyReg(dev,hwPort,QD_PHY_CONTROL_REG,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_CONTROL_REG));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
	}

	if(state)
	{
		u16Data = (u16Data & (QD_PHY_SPEED | QD_PHY_DUPLEX)) | QD_PHY_AUTONEGO;
	}
	else
	{
		u16Data = u16Data & (QD_PHY_SPEED | QD_PHY_DUPLEX);
	}


    DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
              hwPort,QD_PHY_CONTROL_REG,u16Data));

	retVal = hwPhyReset(dev,hwPort,u16Data);
	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}

/*******************************************************************************
* gprtPortPowerDown
*
* DESCRIPTION:
* 		Enable/disable (power down) on specific logical port.
*		Phy configuration remains unchanged after Power down.
*
* INPUTS:
* 		port	- logical port number
* 		state	-  GT_TRUE: power down
* 					GT_FALSE: normal operation
*
* OUTPUTS:
* 		None.
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL 	- on error
*
* COMMENTS:
* 		data sheet register 0.11 - Power Down
*
*******************************************************************************/

GT_STATUS gprtPortPowerDown
(
IN GT_QD_DEV *dev,
IN GT_LPORT  port,
IN GT_BOOL   state
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;

    DBG_INFO(("gprtPortPowerDown Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	BOOL_2_BIT(state,u16Data);

	if((retVal=hwSetPhyRegField(dev,hwPort,QD_PHY_CONTROL_REG,11,1,u16Data)) != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return retVal;
	}

	gtSemGive(dev,dev->phyRegsSem);
	return GT_OK;
}

/*******************************************************************************
* gprtPortRestartAutoNeg
*
* DESCRIPTION:
* 		Restart AutoNegotiation. If AutoNegotiation is not enabled, it'll enable 
*		it. Loopback and Power Down will be disabled by this routine.
*
* INPUTS:
* 		port - logical port number
*
* OUTPUTS:
* 		None.
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL 	- on error
*
* COMMENTS:
* 		data sheet register 0.9 - Restart Auto-Negotiation
*
*******************************************************************************/

GT_STATUS gprtPortRestartAutoNeg
( 
IN GT_QD_DEV *dev,
IN GT_LPORT  port
)
{
    GT_STATUS       retVal;      
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;

    DBG_INFO(("gprtPortRestartAutoNeg Called.\n"));
  
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

    if(hwReadPhyReg(dev,hwPort,QD_PHY_CONTROL_REG,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_CONTROL_REG));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
	}

	u16Data &= (QD_PHY_DUPLEX | QD_PHY_SPEED);
	u16Data |= (QD_PHY_RESTART_AUTONEGO | QD_PHY_AUTONEGO);

    DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
              hwPort,QD_PHY_CONTROL_REG,u16Data));

    /* Write to Phy Control Register.  */
    retVal = hwWritePhyReg(dev,hwPort,QD_PHY_CONTROL_REG,u16Data);
	gtSemGive(dev,dev->phyRegsSem);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}
    return retVal;
}

/*******************************************************************************
* gprtSetPortDuplexMode
*
* DESCRIPTION:
* 		Sets duplex mode for a specific logical port. This function will keep 
*		the speed and loopback mode to the previous value, but disable others,
*		such as Autonegotiation.
*
* INPUTS:
* 		port 	- logical port number
* 		dMode	- dulpex mode
*
* OUTPUTS:
* 		None.
*
* RETURNS:
* 		GT_OK 	- on success
* 		GT_FAIL 	- on error
*
* COMMENTS:
* 		data sheet register 0.8 - Duplex Mode
*
*******************************************************************************/
GT_STATUS gprtSetPortDuplexMode
(
IN GT_QD_DEV *dev,
IN GT_LPORT  port,
IN GT_BOOL   dMode
)
{
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;
	GT_STATUS		retVal;

    DBG_INFO(("gprtSetPortDuplexMode Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if(!IS_CONFIGURABLE_PHY(dev,hwPort))
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

    if(hwReadPhyReg(dev,hwPort,QD_PHY_CONTROL_REG,&u16Data) != GT_OK)
	{
        DBG_INFO(("Not able to read Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_CONTROL_REG));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
	}

	if(dMode)
	{
		u16Data = (u16Data & (QD_PHY_LOOPBACK | QD_PHY_SPEED | QD_PHY_SPEED_MSB)) | QD_PHY_DUPLEX;
	}
	else
	{
		u16Data = u16Data & (QD_PHY_LOOPBACK | QD_PHY_SPEED | QD_PHY_SPEED_MSB);
	}


    DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
              hwPort,QD_PHY_CONTROL_REG,u16Data));

    /* Write to Phy Control Register.  */
	retVal = hwPhyReset(dev,hwPort,u16Data);
	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}


/*******************************************************************************
* gprtSetPortAutoMode
*
* DESCRIPTION:
* 		This routine sets up the port with given Auto Mode.
*		Supported mode is as follows:
*		- Auto for both speed and duplex.
*		- Auto for speed only and Full duplex.
*		- Auto for speed only and Half duplex.
*		- Auto for duplex only and speed 1000Mbps.
*		- Auto for duplex only and speed 100Mbps.
*		- Auto for duplex only and speed 10Mbps.
*		- Speed 1000Mbps and Full duplex.
*		- Speed 1000Mbps and Half duplex.
*		- Speed 100Mbps and Full duplex.
*		- Speed 100Mbps and Half duplex.
*		- Speed 10Mbps and Full duplex.
*		- Speed 10Mbps and Half duplex.
*		
*
* INPUTS:
* 		port - The logical port number
* 		mode - Auto Mode to be written
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - on device without copper
*
* COMMENTS:
* 		data sheet register 4.8, 4.7, 4.6, and 4.5 Autonegotiation Advertisement
* 		data sheet register 4.6, 4.5 Autonegotiation Advertisement for 1000BX
* 		data sheet register 9.9, 9.8 Autonegotiation Advertisement for 1000BT
*******************************************************************************/

GT_STATUS gprtSetPortAutoMode
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port,
	IN GT_PHY_AUTO_MODE mode
)
{

	GT_STATUS       retVal;         /* Functions return value.      */
	GT_U8           hwPort;         /* the physical port number     */
	GT_U16 			u16Data;
	GT_PHY_INFO		phyInfo;
	GT_BOOL			autoOn;
	GT_U16			pageReg;

	DBG_INFO(("gprtSetPortAutoMode Called.\n"));
    
	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	retVal = GT_NOT_SUPPORTED;

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if (!(phyInfo.flag & GT_PHY_GIGABIT))
	{
		if((retVal=feSetAutoMode(dev,hwPort,&phyInfo,mode)) != GT_OK)
		{
			gtSemGive(dev,dev->phyRegsSem);
   		    return retVal;
		}

		u16Data = QD_PHY_SPEED | QD_PHY_DUPLEX | QD_PHY_AUTONEGO;

    	DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
        	      hwPort,QD_PHY_CONTROL_REG,u16Data));

	    /* Write to Phy Control Register.  */
		retVal = hwPhyReset(dev,hwPort,u16Data);
		gtSemGive(dev,dev->phyRegsSem);
		return retVal;
	}
	

	if(phyInfo.flag & GT_PHY_COPPER)
	{
		if(driverPagedAccessStart(dev,hwPort,phyInfo.pageType,&autoOn,&pageReg) != GT_OK)
		{
			gtSemGive(dev,dev->phyRegsSem);
			return GT_FAIL;
		}

		if((retVal=gigCopperSetAutoMode(dev,hwPort,&phyInfo,mode)) != GT_OK)
		{
			gtSemGive(dev,dev->phyRegsSem);
   		    return retVal;
		}

		u16Data = QD_PHY_AUTONEGO;

    	DBG_INFO(("Write to phy(%d) register: regAddr 0x%x, data %#x",
        	      hwPort,QD_PHY_CONTROL_REG,u16Data));

	    /* Write to Phy Control Register.  */
	    if(hwWritePagedPhyReg(dev,hwPort,0,QD_PHY_CONTROL_REG,phyInfo.anyPage,u16Data) != GT_OK)
		{
			gtSemGive(dev,dev->phyRegsSem);
    		return GT_FAIL;
		}

		if(driverPagedAccessStop(dev,hwPort,phyInfo.pageType,autoOn,pageReg) != GT_OK)
		{
			gtSemGive(dev,dev->phyRegsSem);
			return GT_FAIL;
		}

		retVal = hwPhyReset(dev,hwPort,0xFF);
	}

	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}


/*******************************************************************************
* gprtSetPause
*
* DESCRIPTION:
*       This routine will set the pause bit in Autonegotiation Advertisement
*		Register. And restart the autonegotiation.
*
* INPUTS:
* port - The logical port number
* state - GT_PHY_PAUSE_MODE enum value.
*			GT_PHY_NO_PAUSE		- disable pause
* 			GT_PHY_PAUSE		- support pause
*			GT_PHY_ASYMMETRIC_PAUSE	- support asymmetric pause
*			GT_PHY_BOTH_PAUSE	- support both pause and asymmetric pause
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
* COMMENTS:
* data sheet register 4.10 Autonegotiation Advertisement Register
*******************************************************************************/

GT_STATUS gprtSetPause
(
IN GT_QD_DEV *dev,
IN GT_LPORT  port,
IN GT_PHY_PAUSE_MODE state
)
{
	GT_U8           hwPort;         /* the physical port number     */
	GT_U16 			u16Data;
	GT_STATUS		retVal = GT_OK;
	GT_PHY_INFO		phyInfo;

	DBG_INFO(("phySetPause Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if(state & GT_PHY_ASYMMETRIC_PAUSE)
	{
		if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
		{
	    	DBG_INFO(("Unknown PHY device.\n"));
			gtSemGive(dev,dev->phyRegsSem);
			return GT_FAIL;
		}

		if (!(phyInfo.flag & GT_PHY_GIGABIT))
		{
			DBG_INFO(("Not Supported\n"));
			gtSemGive(dev,dev->phyRegsSem);
			return GT_BAD_PARAM;
		}
	}

	u16Data = state;

	/* Write to Phy AutoNegotiation Advertisement Register.  */
	if((retVal=hwSetPhyRegField(dev,hwPort,QD_PHY_AUTONEGO_AD_REG,10,2,u16Data)) != GT_OK)
	{
		DBG_INFO(("Not able to write Phy Reg(port:%d,offset:%d).\n",hwPort,QD_PHY_AUTONEGO_AD_REG));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	/* Restart Auto Negotiation */
	if((retVal=hwSetPhyRegField(dev,hwPort,QD_PHY_CONTROL_REG,9,1,1)) != GT_OK)
	{
		DBG_INFO(("Not able to write Phy Reg(port:%d,offset:%d,data:%#x).\n",hwPort,QD_PHY_AUTONEGO_AD_REG,u16Data));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}


static
GT_STATUS dteWorkAround_Phy100M
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8            hwPort
)
{
	GT_STATUS status = GT_OK;
	GT_U32 threshold[] = {0x000B,0x0000,0x8780,0x0000,0x8F80,0x0000,
						  0x9780,0x0000,0x9F80,0x0000,0xA780,0x0000,
						  0xAF80,0x0000,0xB780,0x0000,0xBF80,0x0000,
						  0xC780,0x0000,0xCF80,0x0000,0xD780,0x0000,
						  0xDF80,0x0000,0xE780,0x0000,0xEF80,0x0000,
						  0xF780,0x0000,0xFF80,0x0000};
	int i, thresholdSize;

	/* force r125 clock */
	if((status= hwWritePhyReg(dev,hwPort,0x1D,0x0003)) != GT_OK)
	{
		return status;
	}
	if((status= hwWritePhyReg(dev,hwPort,0x1E,0x807f)) != GT_OK)
	{
		return status;
	}

	/* write thresholds */
	if((status= hwWritePhyReg(dev,hwPort,0x1D,0x000B)) != GT_OK)
	{
		return status;
	}

	thresholdSize = sizeof(threshold)/sizeof(GT_U32);

	for(i=0; i<thresholdSize; i++)
	{
		if((status= hwWritePhyReg(dev,hwPort,0x1E,(GT_U16)threshold[i])) != GT_OK)
		{
			return status;
		}
	}

	/* setting adc Masking */
	if((status= hwWritePhyReg(dev,hwPort,0x1D,0x0001)) != GT_OK)
	{
		return status;
	}
	if((status= hwWritePhyReg(dev,hwPort,0x1E,0x4000)) != GT_OK)
	{
		return status;
	}

	/* setting noise level */
	if((status= hwWritePhyReg(dev,hwPort,0x1D,0x0005)) != GT_OK)
	{
		return status;
	}
	if((status= hwWritePhyReg(dev,hwPort,0x1E,0xA000)) != GT_OK)
	{
		return status;
	}

	/* 
		offseting cable length measurement by 6.72m(2*4*0.84m)
		set 30_10.14:11 to 0x1001 for cable length measure.
	*/ 
	if((status= hwWritePhyReg(dev,hwPort,0x1D,0x000a)) != GT_OK)
	{
		return status;
	}
	if((status= hwWritePhyReg(dev,hwPort,0x1E,0x4840)) != GT_OK)
	{
		return status;
	}

	/* release force r125 clock */
	if((status= hwWritePhyReg(dev,hwPort,0x1D,0x0003)) != GT_OK)
	{
		return status;
	}
	if((status= hwWritePhyReg(dev,hwPort,0x1E,0x0000)) != GT_OK)
	{
		return status;
	}


	return status;
}

static
GT_STATUS dteWorkAround_Phy1000M
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8            hwPort
)
{
	GT_STATUS status = GT_OK;
	GT_U32 threshold[] = {0x0000,0x8780,0x0000,0x8F80,0x0000,0x9780,
						  0x0000,0x9F80,0x0000,0xA780,0x0000,0xAF80,
						  0x0000,0xB780,0x0000,0xBF80,0x0000,0xC780,
						  0x0000,0xCF80,0x0000,0xD780,0x0000,0xDF80,
						  0x0000,0xE780,0x0000,0xEF80,0x0000,0xF780,
						  0x0000,0xFF80,0x0000};
	int i, thresholdSize;

	/*  */
	if((status= hwWritePhyReg(dev,hwPort,0x1D,0x001B)) != GT_OK)
	{
		return status;
	}
	if((status= hwWritePhyReg(dev,hwPort,0x1E,0x43FF)) != GT_OK)
	{
		return status;
	}

	/*  */
	if((status= hwWritePhyReg(dev,hwPort,0x1D,0x001C)) != GT_OK)
	{
		return status;
	}
	if((status= hwWritePhyReg(dev,hwPort,0x1E,0x9999)) != GT_OK)
	{
		return status;
	}

	/*  */
	if((status= hwWritePhyReg(dev,hwPort,0x1D,0x001F)) != GT_OK)
	{
		return status;
	}
	if((status= hwWritePhyReg(dev,hwPort,0x1E,0xE00C)) != GT_OK)
	{
		return status;
	}

	/*  */
	if((status= hwWritePhyReg(dev,hwPort,0x1D,0x0018)) != GT_OK)
	{
		return status;
	}
	if((status= hwWritePhyReg(dev,hwPort,0x1E,0xFFA1)) != GT_OK)
	{
		return status;
	}

	/* write thresholds */
	if((status= hwWritePhyReg(dev,hwPort,0x1D,0x0010)) != GT_OK)
	{
		return status;
	}

	thresholdSize = sizeof(threshold)/sizeof(GT_U32);

	for(i=0; i<thresholdSize; i++)
	{
		if((status= hwWritePhyReg(dev,hwPort,0x1E,(GT_U16)threshold[i])) != GT_OK)
		{
			return status;
		}
	}

	return status;
}

static
GT_STATUS feSetDTE
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8     hwPort,
	IN  GT_BOOL   state
)
{
	GT_U16 			u16Data;
	GT_STATUS		retVal = GT_OK;

	if((retVal = hwReadPhyReg(dev,hwPort,0x10,&u16Data)) != GT_OK)
	{
		return retVal;
	}

	u16Data = state?(u16Data|0x8000):(u16Data&(~0x8000));

	if((retVal = hwWritePhyReg(dev,hwPort,0x10,u16Data)) != GT_OK)
	{
		return retVal;
	}

	/* soft reset */
	if((retVal = hwPhyReset(dev,hwPort,0xFF)) != GT_OK)
	{
		return retVal;
	}

	return retVal;
}

static
GT_STATUS gigSetDTE
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8     hwPort,
	IN  GT_BOOL   state
)
{
	GT_U16 			u16Data;
	GT_STATUS		retVal = GT_OK;

	if((retVal = hwReadPhyReg(dev,hwPort,20,&u16Data)) != GT_OK)
	{
		return retVal;
	}

	u16Data = state?(u16Data|0x4):(u16Data&(~0x4));

	if((retVal = hwWritePhyReg(dev,hwPort,20,u16Data)) != GT_OK)
	{
		return retVal;
	}

	/* soft reset */
	if((retVal = hwPhyReset(dev,hwPort,0xFF)) != GT_OK)
	{
		return retVal;
	}

	return retVal;
}

static
GT_STATUS gigMPSetDTE
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8     hwPort,
	IN  GT_BOOL   state
)
{
	GT_U16 			u16Data;
	GT_STATUS		retVal = GT_OK;

	if((retVal = hwReadPagedPhyReg(dev,hwPort,0,26,0,&u16Data)) != GT_OK)
	{
		return retVal;
	}

	u16Data = state?(u16Data|0x100):(u16Data&(~0x100));

	if((retVal = hwWritePagedPhyReg(dev,hwPort,0,26,0,u16Data)) != GT_OK)
	{
		return retVal;
	}

	/* soft reset */
	if((retVal = hwPhyReset(dev,hwPort,0xFF)) != GT_OK)
	{
		return retVal;
	}

	return retVal;
}

/*******************************************************************************
* gprtSetDTEDetect
*
* DESCRIPTION:
*       This routine enables/disables DTE.
*
* INPUTS:
* 		port - The logical port number
* 		mode - either GT_TRUE(for enable) or GT_FALSE(for disable)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gprtSetDTEDetect
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port,
	IN GT_BOOL   state
)
{
	GT_U8           hwPort;         /* the physical port number     */
	GT_STATUS		retVal = GT_OK;
	GT_PHY_INFO	phyInfo;
	GT_BOOL			autoOn;
	GT_U16			pageReg;

	DBG_INFO(("phySetDTE Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	/* check if the port supports DTE */
	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if (!(phyInfo.flag & GT_PHY_DTE_CAPABLE))
	{
		DBG_INFO(("Not Supported\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	switch(phyInfo.dteType)
	{
		case GT_PHY_DTE_TYPE1:
			/* FE Phy needs work-around */
			if((retVal = feSetDTE(dev,hwPort,state)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}

			if(state == GT_FALSE)
				break;

			if((retVal = dteWorkAround_Phy100M(dev,hwPort)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}
			break;
		case GT_PHY_DTE_TYPE3:
			/* Gigabit Phy with work-around required */
			if((retVal = gigSetDTE(dev,hwPort,state)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}

			if(state == GT_FALSE)
				break;

			if((retVal = dteWorkAround_Phy1000M(dev,hwPort)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}
			break;

		case GT_PHY_DTE_TYPE2:
			/* no workaround required */
			if((retVal = gigSetDTE(dev,hwPort,state)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}

			break;
		case GT_PHY_DTE_TYPE4:
			/* no workaround required */
			if(driverPagedAccessStart(dev,hwPort,phyInfo.pageType,&autoOn,&pageReg) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return GT_FAIL;
			}

			if((retVal = gigMPSetDTE(dev,hwPort,state)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}

			if(driverPagedAccessStop(dev,hwPort,phyInfo.pageType,autoOn,pageReg) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return GT_FAIL;
			}
			break;
		default:
			gtSemGive(dev,dev->phyRegsSem);
			return GT_NOT_SUPPORTED;
	}

	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}


/*******************************************************************************
* gprtGetDTEDetectStatus
*
* DESCRIPTION:
*       This routine gets DTE status.
*
* INPUTS:
* 		port - The logical port number
*
* OUTPUTS:
*       status - GT_TRUE, if link partner needs DTE power.
*				 GT_FALSE, otherwise.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gprtGetDTEDetectStatus
(
	IN  GT_QD_DEV *dev,
	IN  GT_LPORT  port,
	OUT GT_BOOL   *state
)
{
	GT_U8           hwPort;         /* the physical port number     */
	GT_U16 			u16Data,pageReg;
	GT_STATUS		retVal = GT_OK;
	GT_PHY_INFO	phyInfo;
	GT_BOOL			autoOn;

	DBG_INFO(("gprtGetDTEStatus Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	/* check if the port supports DTE */
	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if (!(phyInfo.flag & GT_PHY_DTE_CAPABLE))
	{
		DBG_INFO(("Not Supported\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	switch(phyInfo.dteType)
	{
		case GT_PHY_DTE_TYPE1:
			/* FE Phy needs work-around */
			if((retVal = hwReadPhyReg(dev,hwPort,17,&u16Data)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}
			*state = (u16Data & 0x8000)?GT_TRUE:GT_FALSE;

			break;
		case GT_PHY_DTE_TYPE2:
		case GT_PHY_DTE_TYPE3:
			if((retVal = hwReadPhyReg(dev,hwPort,27,&u16Data)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}
			*state = (u16Data & 0x10)?GT_TRUE:GT_FALSE;
			
			break;
		case GT_PHY_DTE_TYPE4:
			if(driverPagedAccessStart(dev,hwPort,phyInfo.pageType,&autoOn,&pageReg) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return GT_FAIL;
			}
				
			if((retVal = hwReadPagedPhyReg(dev,hwPort,0,17,phyInfo.anyPage,&u16Data)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}
			*state = (u16Data & 0x4)?GT_TRUE:GT_FALSE;

			if(driverPagedAccessStop(dev,hwPort,phyInfo.pageType,autoOn,pageReg) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return GT_FAIL;
			}

			break;
		default:
			gtSemGive(dev,dev->phyRegsSem);
			return GT_NOT_SUPPORTED;
	}

	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}


/*******************************************************************************
* gprtSetDTEDetectDropWait
*
* DESCRIPTION:
*       Once the PHY no longer detects that the link partner filter, the PHY
*		will wait a period of time before clearing the power over Ethernet 
*		detection status bit. The wait time is 5 seconds multiplied by the 
*		given value.
*
* INPUTS:
* 		port - The logical port number
*       waitTime - 0 ~ 15 (unit of 4 sec.)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gprtSetDTEDetectDropWait
(
	IN  GT_QD_DEV *dev,
	IN  GT_LPORT  port,
	IN  GT_U16    waitTime
)
{
	GT_U8           hwPort;         /* the physical port number     */
	GT_U16 			u16Data;
	GT_STATUS		retVal = GT_OK;
	GT_PHY_INFO	phyInfo;
	GT_BOOL			autoOn;
	GT_U16			pageReg;

	DBG_INFO(("gprtSetDTEDropWait Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	/* check if the port supports DTE */
	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if (!(phyInfo.flag & GT_PHY_DTE_CAPABLE))
	{
		DBG_INFO(("Not Supported\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	switch(phyInfo.dteType)
	{
		case GT_PHY_DTE_TYPE1:
			if((retVal = hwReadPhyReg(dev,hwPort,22,&u16Data)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}
			u16Data = (u16Data & ~(0xF<<12)) | ((waitTime & 0xF) << 12);

			if((retVal = hwWritePhyReg(dev,hwPort,22,u16Data)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}
			break;
		case GT_PHY_DTE_TYPE2:
		case GT_PHY_DTE_TYPE3:
			if((retVal = hwReadPhyReg(dev,hwPort,27,&u16Data)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}
			u16Data = (u16Data & ~(0xF<<5)) | ((waitTime & 0xF) << 5);

			if((retVal = hwWritePhyReg(dev,hwPort,27,u16Data)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}
			
			break;
		case GT_PHY_DTE_TYPE4:
			if(driverPagedAccessStart(dev,hwPort,phyInfo.pageType,&autoOn,&pageReg) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return GT_FAIL;
			}

			if((retVal = hwReadPagedPhyReg(dev,hwPort,0,26,phyInfo.anyPage,&u16Data)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}
			u16Data = (u16Data & ~(0xF<<4)) | ((waitTime & 0xF) << 4);
			if((retVal = hwWritePagedPhyReg(dev,hwPort,0,26,phyInfo.anyPage,u16Data)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}

			if(driverPagedAccessStop(dev,hwPort,phyInfo.pageType,autoOn,pageReg) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return GT_FAIL;
			}

			break;
		default:
			gtSemGive(dev,dev->phyRegsSem);
			return GT_NOT_SUPPORTED;
	}

	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}


/*******************************************************************************
* gprtGetDTEDetectDropWait
*
* DESCRIPTION:
*       Once the PHY no longer detects that the link partner filter, the PHY
*		will wait a period of time before clearing the power over Ethernet 
*		detection status bit. The wait time is 5 seconds multiplied by the 
*		returned value.
*
* INPUTS:
* 		port - The logical port number
*
* OUTPUTS:
*       waitTime - 0 ~ 15 (unit of 4 sec.)
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gprtGetDTEDetectDropWait
(
	IN  GT_QD_DEV *dev,
	IN  GT_LPORT  port,
	OUT GT_U16    *waitTime
)
{
	GT_U8           hwPort;         /* the physical port number     */
	GT_U16 			u16Data;
	GT_STATUS		retVal = GT_OK;
	GT_PHY_INFO	phyInfo;
	GT_BOOL			autoOn;
	GT_U16			pageReg;

	DBG_INFO(("gprtSetDTEDropWait Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if (!(phyInfo.flag & GT_PHY_DTE_CAPABLE))
	{
		DBG_INFO(("Not Supported\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	switch(phyInfo.dteType)
	{
		case GT_PHY_DTE_TYPE1:
			if((retVal = hwReadPhyReg(dev,hwPort,22,&u16Data)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}
			u16Data = (u16Data >> 12) & 0xF;

			break;
		case GT_PHY_DTE_TYPE2:
		case GT_PHY_DTE_TYPE3:
			if((retVal = hwReadPhyReg(dev,hwPort,27,&u16Data)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}
			u16Data = (u16Data >> 5) & 0xF;

			break;
		case GT_PHY_DTE_TYPE4:
			if(driverPagedAccessStart(dev,hwPort,phyInfo.pageType,&autoOn,&pageReg) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return GT_FAIL;
			}

			if((retVal = hwReadPagedPhyReg(dev,hwPort,0,26,phyInfo.anyPage,&u16Data)) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return retVal;
			}
			u16Data = (u16Data >> 4) & 0xF;

			if(driverPagedAccessStop(dev,hwPort,phyInfo.pageType,autoOn,pageReg) != GT_OK)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return GT_FAIL;
			}
			break;
		default:
			gtSemGive(dev,dev->phyRegsSem);
			return GT_NOT_SUPPORTED;
	}

	*waitTime = u16Data;

	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}


/*******************************************************************************
* gprtSet1000TMasterMode
*
* DESCRIPTION:
*       This routine sets the ports 1000Base-T Master mode and restart the Auto
*		negotiation.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_1000T_MASTER_SLAVE structure
*				autoConfig   - GT_TRUE for auto, GT_FALSE for manual setup.
*				masterPrefer - GT_TRUE if Master configuration is preferred.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSet1000TMasterMode
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT     port,
    IN  GT_1000T_MASTER_SLAVE   *mode
)
{
	GT_STATUS	retVal;         /* Functions return value.      */
	GT_U8			hwPort;         /* the physical port number     */
	GT_U16		data;
	GT_PHY_INFO	phyInfo;
	GT_BOOL			autoOn;
	GT_U16			pageReg;

	DBG_INFO(("gprtSet1000TMasterMode Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
		return GT_NOT_SUPPORTED;
	}

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if (!(phyInfo.flag & GT_PHY_GIGABIT) || !(phyInfo.flag & GT_PHY_COPPER))
	{
		DBG_INFO(("Not Supported\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if(mode->autoConfig == GT_TRUE)
	{
		if(mode->masterPrefer == GT_TRUE)
		{
			data = 0x1;
		}
		else
		{
			data = 0x0;
		}
	}
	else
	{
		if(mode->masterPrefer == GT_TRUE)
		{
			data = 0x6;
		}
		else
		{
			data = 0x4;
		}
	}

	if(driverPagedAccessStart(dev,hwPort,phyInfo.pageType,&autoOn,&pageReg) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	/* Set the Master Mode.    */
	retVal = hwSetPagedPhyRegField(dev,hwPort,0,9,10,3,phyInfo.anyPage,data);
	if(retVal != GT_OK)
	{
		DBG_INFO(("Failed.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return retVal;
	}
    else
	{
		DBG_INFO(("OK.\n"));
	}

	/* Restart Auto Negotiation */
	if((retVal=hwSetPhyRegField(dev,hwPort,QD_PHY_CONTROL_REG,9,1,1)) != GT_OK)
	{
		DBG_INFO(("Not able to write Phy Reg(port:%d,offset:%d,data:%#x).\n",hwPort,QD_PHY_CONTROL_REG,1));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if(driverPagedAccessStop(dev,hwPort,phyInfo.pageType,autoOn,pageReg) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}


/*******************************************************************************
* gprtGet1000TMasterMode
*
* DESCRIPTION:
*       This routine retrieves 1000Base-T Master Mode
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_1000T_MASTER_SLAVE structure
*				autoConfig   - GT_TRUE for auto, GT_FALSE for manual setup.
*				masterPrefer - GT_TRUE if Master configuration is preferred.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGet1000TMasterMode
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT     port,
    OUT GT_1000T_MASTER_SLAVE   *mode
)
{
	GT_STATUS	retVal;         /* Functions return value.      */
	GT_U8			hwPort;         /* the physical port number     */
	GT_U16		data;
	GT_PHY_INFO	phyInfo;
	GT_BOOL			autoOn;
	GT_U16			pageReg;

	DBG_INFO(("gprtGet1000TMasterMode Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
		return GT_NOT_SUPPORTED;
	}

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if (!(phyInfo.flag & GT_PHY_GIGABIT) || !(phyInfo.flag & GT_PHY_COPPER))
	{
		DBG_INFO(("Not Supported\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if(driverPagedAccessStart(dev,hwPort,phyInfo.pageType,&autoOn,&pageReg) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	/* Set the Master Mode.    */
	retVal = hwGetPagedPhyRegField(dev,hwPort,0,9,10,3,phyInfo.anyPage,&data);
	if(retVal != GT_OK)
	{
		DBG_INFO(("Failed.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return retVal;
	}
	else
	{
		DBG_INFO(("OK.\n"));
	}

	if(driverPagedAccessStop(dev,hwPort,phyInfo.pageType,autoOn,pageReg) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if(data & 0x4)	/* Manual Mode */
	{
		mode->autoConfig = GT_FALSE;

		if(data & 0x2)
		{
			mode->masterPrefer = GT_TRUE;
		}
		else
		{
			mode->masterPrefer = GT_FALSE;
		}
	}
	else	/* Auto Mode */
	{
		mode->autoConfig = GT_TRUE;

		if(data & 0x1)
		{
			mode->masterPrefer = GT_TRUE;
		}
		else
		{
			mode->masterPrefer = GT_FALSE;
		}
	}

	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}

/*******************************************************************************
* gprtGetPhyLinkStatus
*
* DESCRIPTION:
*       This routine retrieves the Link status.
*
* INPUTS:
* 		port 	- The logical port number
*
* OUTPUTS:
*       linkStatus - GT_FALSE if link is not established,
*				     GT_TRUE if link is established.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		
*
* COMMENTS:
*
*******************************************************************************/
#include <linux/kernel.h>
GT_STATUS gprtGetPhyLinkStatus
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port,
    IN GT_BOOL 	 *linkStatus
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;
	GT_PHY_INFO		phyInfo;

    DBG_INFO(("gprtGetPhyLinkStatus Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		if(IS_IN_DEV_GROUP(dev,DEV_SERDES_CORE))
		{	
			GT_GET_SERDES_PORT(dev,&hwPort);
			if(hwPort > dev->maxPhyNum)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return GT_NOT_SUPPORTED;
			}
		}
		else
		{
			gtSemGive(dev,dev->phyRegsSem);
		 	return GT_NOT_SUPPORTED;
		}
	}

	if((retVal=hwGetPhyRegField(dev,hwPort,17,10,1,&u16Data)) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return retVal;
	}

	BIT_2_BOOL(u16Data,*linkStatus);

	gtSemGive(dev,dev->phyRegsSem);
    return retVal;
}
#if 1
GT_STATUS gprtGetPhyStatus
(
	IN GT_QD_DEV *dev,
	IN GT_U8  port,
    IN GT_BOOL 	 *linkStatus,
    IN GT_U16 	 *speed,
    IN GT_BOOL 	 *duplex
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			u16Data;
	GT_PHY_INFO		phyInfo;

    DBG_INFO(("gprtGetPhyLinkStatus Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort =GT_LPORT_2_PORT(port);
//printk(KERN_EMERG"enter\n");			
//printk(KERN_DEBUG"enter2\n");			

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);


	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		if(IS_IN_DEV_GROUP(dev,DEV_SERDES_CORE))
		{	
			GT_GET_SERDES_PORT(dev,&hwPort);
			if(hwPort > dev->maxPorts)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return GT_NOT_SUPPORTED;
			}
		}
		else
		{
			gtSemGive(dev,dev->phyRegsSem);
		 	return GT_NOT_SUPPORTED;
		}
	}
#if 0	
	if (hwGetPhyRegField(dev,hwPort,17,11,1,&u16Data)==GT_OK)
		printk(KERN_EMERG" register 17_0.1 is [%d]\n", u16Data);
	else
		printk(KERN_EMERG" get register 17_0.1 failed\n");
#endif	
	if (linkStatus!=NULL) {
		if((retVal=hwGetPhyRegField(dev,hwPort,17,10,1,&u16Data)) != GT_OK){
			gtSemGive(dev,dev->phyRegsSem);
printk(KERN_EMERG"can not get link status\n");			
printk(KERN_DEBUG"can not get link status2\n");			
			return retVal;
		}
		BIT_2_BOOL(u16Data,*linkStatus);
	}
	if (duplex != NULL) {
		if((retVal=hwGetPhyRegField(dev,hwPort,17,13,1,duplex)) != GT_OK){
			gtSemGive(dev,dev->phyRegsSem);
printk(KERN_EMERG"can not get duplex\n");			
printk(KERN_DEBUG"can not get duplex2\n");			
			return retVal;
		}
		//BIT_2_BOOL(u16Data,*duplex);
	}
	if (speed != NULL) {
		if((retVal=hwGetPhyRegField(dev,hwPort,17,14,2,speed)) != GT_OK){
			gtSemGive(dev,dev->phyRegsSem);
printk(KERN_EMERG"can not get speed\n");			
printk(KERN_DEBUG"can not get speed2\n");			
			return retVal;
		}
		//*speed = u16Data;
	}

	gtSemGive(dev,dev->phyRegsSem);
    return retVal;
}
#endif

/*******************************************************************************
* gprtSetPktGenEnable
*
* DESCRIPTION:
*       This routine enables or disables Packet Generator.
*       Link should be established first prior to enabling the packet generator,
*       and generator will generate packets at the speed of the established link.
*		When enables packet generator, the following information should be 
*       provided:
*           Payload Type:  either Random or 5AA55AA5
*           Packet Length: either 64 or 1514 bytes
*           Error Packet:  either Error packet or normal packet
*
* INPUTS:
* 		port 	- The logical port number
*       en      - GT_TRUE to enable, GT_FALSE to disable
*       pktInfo - packet information(GT_PG structure pointer), if en is GT_TRUE.
*                 ignored, if en is GT_FALSE
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtSetPktGenEnable
(
	IN GT_QD_DEV *dev,
	IN GT_LPORT  port,
    IN GT_BOOL   en,
    IN GT_PG     *pktInfo
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16 			data;
	GT_BOOL			link;
	GT_PHY_INFO		phyInfo;
	GT_U8			page,reg, offset, len;
	GT_BOOL			autoOn;
	GT_U16			pageReg;

    DBG_INFO(("gprtSetPktGenEnable Called.\n"));
    
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if(!(phyInfo.flag & GT_PHY_PKT_GENERATOR))
	{
		if(IS_IN_DEV_GROUP(dev,DEV_SERDES_CORE))
		{	
			GT_GET_SERDES_PORT(dev,&hwPort);
			if(hwPort > dev->maxPhyNum)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return GT_NOT_SUPPORTED;
			}

			/* check if the port is configurable */
			if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
			{
				gtSemGive(dev,dev->phyRegsSem);
				return GT_NOT_SUPPORTED;
			}

			if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
			{
			    DBG_INFO(("Unknown PHY device.\n"));
				gtSemGive(dev,dev->phyRegsSem);
				return GT_FAIL;
			}

			if(!(phyInfo.flag & GT_PHY_PKT_GENERATOR))
			{
			    DBG_INFO(("Not Supported.\n"));
				gtSemGive(dev,dev->phyRegsSem);
				return GT_NOT_SUPPORTED;
			}
		}
		else
		{
		    DBG_INFO(("Not Supported.\n"));
			gtSemGive(dev,dev->phyRegsSem);
			return GT_NOT_SUPPORTED;
		}
	}

	switch (phyInfo.pktGenType)
	{
		case GT_PHY_PKTGEN_TYPE1:	/* 30_18.5:2 */
				page = 18;
				reg = 30;
				offset = 2;
				break;
		case GT_PHY_PKTGEN_TYPE2:	/* 16_6.3:0 */
				page = 6;
				reg = 16;
				offset = 0;
				break;
		case GT_PHY_PKTGEN_TYPE3:	/* 25.3:0 */
				page = 0;
				reg = 25;
				offset = 0;
				break;
		default:
			    DBG_INFO(("Unknown PKTGEN Type.\n"));
				gtSemGive(dev,dev->phyRegsSem);
				return GT_FAIL;
	}

	if (en)
	{
		if((retVal = gprtGetPhyLinkStatus(dev,port,&link)) != GT_OK)
		{
			gtSemGive(dev,dev->phyRegsSem);
			return GT_FAIL;
		}
	
		if (link == GT_FALSE)
		{
		    DBG_INFO(("Link should be on to run Packet Generator.\n"));
			gtSemGive(dev,dev->phyRegsSem);
			return GT_FAIL;
		}

		data = 0x8;

        if (pktInfo->payload == GT_PG_PAYLOAD_5AA5)
            data |= 0x4;

        if (pktInfo->length == GT_PG_LENGTH_1514)
            data |= 0x2;

        if (pktInfo->tx == GT_PG_TX_ERROR)
            data |= 0x1;

        len = 4;
	}
	else
	{
		data = 0;
		len = 1;
		offset += 3;
	}

	if(driverPagedAccessStart(dev,hwPort,phyInfo.pageType,&autoOn,&pageReg) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if((retVal=hwSetPagedPhyRegField(dev,hwPort,
				page,reg,offset,len,phyInfo.anyPage,data)) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return retVal;
	}

	if(driverPagedAccessStop(dev,hwPort,phyInfo.pageType,autoOn,pageReg) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	gtSemGive(dev,dev->phyRegsSem);
    return retVal;
}


/*******************************************************************************
* gprtGetSerdesMode
*
* DESCRIPTION:
*       This routine reads Serdes Interface Mode.
*
* INPUTS:
*       port    - logical port number
*
* OUTPUTS:
*       mode    - Serdes Interface Mode
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetSerdesMode
(
    IN  GT_QD_DEV    *dev,
    IN  GT_LPORT     port,
	IN  GT_SERDES_MODE *mode
)
{
    GT_U16          u16Data;           /* The register's read data.    */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetSerdesMode Called.\n"));

    hwPort = GT_LPORT_2_PORT(port);

	if(!IS_IN_DEV_GROUP(dev,DEV_SERDES_CORE))
	{
		return GT_NOT_SUPPORTED;
	}
	
	GT_GET_SERDES_PORT(dev,&hwPort);

	if(hwPort > dev->maxPhyNum)
	{
		return GT_NOT_SUPPORTED;
	}

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

    /* Get Phy Register. */
    if(hwGetPhyRegField(dev,hwPort,16,0,2,&u16Data) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
    }

	*mode = u16Data;

	gtSemGive(dev,dev->phyRegsSem);
    return GT_OK;
}


/*******************************************************************************
* gprtSetSerdesMode
*
* DESCRIPTION:
*       This routine sets Serdes Interface Mode.
*
* INPUTS:
*       port    - logical port number
*       mode    - Serdes Interface Mode
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetSerdesMode
(
    IN  GT_QD_DEV    *dev,
    IN  GT_LPORT     port,
	IN  GT_SERDES_MODE mode
)
{
    GT_U16          u16Data;           /* The register's read data.    */
    GT_U8           hwPort;         /* the physical port number     */
	GT_STATUS		retVal;

    DBG_INFO(("gprtSetSerdesMode Called.\n"));

    hwPort = GT_LPORT_2_PORT(port);

	if(!IS_IN_DEV_GROUP(dev,DEV_SERDES_CORE))
	{
		return GT_NOT_SUPPORTED;
	}
	
	GT_GET_SERDES_PORT(dev,&hwPort);

	if(hwPort > dev->maxPhyNum)
	{
		return GT_NOT_SUPPORTED;
	}

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	u16Data = mode;

    /* Get Phy Register. */
    if(hwSetPhyRegField(dev,hwPort,16,0,2,u16Data) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
    }

	retVal = hwPhyReset(dev,hwPort,0xFF);
	gtSemGive(dev,dev->phyRegsSem);
	return retVal;
}


/*******************************************************************************
* gprtGetPhyReg
*
* DESCRIPTION:
*       This routine reads Phy Registers.
*
* INPUTS:
*       port    - logical port number
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPhyReg
(
    IN  GT_QD_DEV    *dev,
    IN  GT_LPORT     port,
    IN  GT_U32	     regAddr,
    OUT GT_U16	     *data
)
{
    GT_U16          u16Data;           /* The register's read data.    */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetPhyReg Called.\n"));

    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

    /* Get Phy Register. */
    if(hwReadPhyReg(dev,hwPort,(GT_U8)regAddr,&u16Data) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
    }

	*data = u16Data;

	gtSemGive(dev,dev->phyRegsSem);
    return GT_OK;
}

/*******************************************************************************
* gprtSetPhyReg
*
* DESCRIPTION:
*       This routine writes Phy Registers.
*
* INPUTS:
*       port    - logical port number
*       regAddr - The register's address.
*
* OUTPUTS:
*       data    - The read register's data.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetPhyReg
(
    IN  GT_QD_DEV		*dev,
    IN  GT_LPORT		port,
    IN  GT_U32			regAddr,
    IN  GT_U16			data
)
{
    GT_U8           hwPort;         /* the physical port number     */
    
    DBG_INFO(("gprtSetPhyReg Called.\n"));

    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

    /* Write to Phy Register */
    if(hwWritePhyReg(dev,hwPort,(GT_U8)regAddr,data) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
		gtSemGive(dev,dev->phyRegsSem);
        return GT_FAIL;
    }

	gtSemGive(dev,dev->phyRegsSem);
	return GT_OK;
}



/*******************************************************************************
* gprtGetPagedPhyReg
*
* DESCRIPTION:
*       This routine reads phy register of the given page
*
* INPUTS:
*		port 	- logical port to be read
*		regAddr	- register offset to be read
*		page	- page number to be read
*
* OUTPUTS:
*		data	- value of the read register
*
* RETURNS:
*       GT_OK   			- if read successed
*       GT_FAIL   			- if read failed
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gprtGetPagedPhyReg
(
    IN  GT_QD_DEV *dev,
    IN  GT_U32  port,
	IN	GT_U32  regAddr,
	IN	GT_U32  page,
    OUT GT_U16* data
)
{
	GT_PHY_INFO		phyInfo;
	GT_BOOL			autoOn;
	GT_U16			pageReg;
	GT_U8			hwPort;

    hwPort = GT_LPORT_2_PORT(port);
	
	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if(driverPagedAccessStart(dev,hwPort,phyInfo.pageType,&autoOn,&pageReg) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if(hwReadPagedPhyReg(dev,hwPort,(GT_U8)page,
								(GT_U8)regAddr,0,data) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if(driverPagedAccessStop(dev,hwPort,phyInfo.pageType,autoOn,pageReg) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	gtSemGive(dev,dev->phyRegsSem);
	return GT_OK;
}

/*******************************************************************************
* gprtSetPagedPhyReg
*
* DESCRIPTION:
*       This routine writes a value to phy register of the given page
*
* INPUTS:
*		port 	- logical port to be read
*		regAddr	- register offset to be read
*		page	- page number to be read
*		data	- value of the read register
*
* OUTPUTS:
*		None
*
* RETURNS:
*       GT_OK   			- if read successed
*       GT_FAIL   			- if read failed
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gprtSetPagedPhyReg
(
    IN  GT_QD_DEV *dev,
    IN  GT_U32 port,
	IN	GT_U32 regAddr,
	IN	GT_U32 page,
    IN  GT_U16 data
)
{
	GT_PHY_INFO		phyInfo;
	GT_BOOL			autoOn;
	GT_U16			pageReg;
	GT_U8			hwPort;

    hwPort = GT_LPORT_2_PORT(port);

	gtSemTake(dev,dev->phyRegsSem,OS_WAIT_FOREVER);

	/* check if the port is configurable */
	if((phyInfo.phyId=IS_CONFIGURABLE_PHY(dev,hwPort)) == 0)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_NOT_SUPPORTED;
	}

	if(driverFindPhyInformation(dev,hwPort,&phyInfo) != GT_OK)
	{
	    DBG_INFO(("Unknown PHY device.\n"));
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if(driverPagedAccessStart(dev,hwPort,phyInfo.pageType,&autoOn,&pageReg) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if(hwWritePagedPhyReg(dev,hwPort,(GT_U8)page,
								(GT_U8)regAddr,0,data) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	if(driverPagedAccessStop(dev,hwPort,phyInfo.pageType,autoOn,pageReg) != GT_OK)
	{
		gtSemGive(dev,dev->phyRegsSem);
		return GT_FAIL;
	}

	gtSemGive(dev,dev->phyRegsSem);
	return GT_OK;
}



