#include <Copyright.h>

/********************************************************************************
* gtSysCtrl.c
*
* DESCRIPTION:
*       API definitions for system global control.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*******************************************************************************/

#include <msApi.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>
#include <gtSem.h>

/*******************************************************************************
* gsysSwReset
*
* DESCRIPTION:
*       This routine preforms switch software reset.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSwReset
(
    IN  GT_QD_DEV *dev
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U8			regOffset;

    DBG_INFO(("gsysSwReset Called.\n"));

    /* Set the Software reset bit.                  */
	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
		regOffset = QD_REG_GLOBAL_CONTROL;
	}
	else
	{
		regOffset = QD_REG_ATU_CONTROL;
	}

    retVal = hwSetGlobalRegField(dev,regOffset,15,1,1);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    /* Make sure the reset operation is completed.  */
    data = 1;
    while(data != 0)
    {
   	    retVal = hwGetGlobalRegField(dev,regOffset,15,1,&data);

        if(retVal != GT_OK)
        {
            DBG_INFO(("Failed.\n"));
            return retVal;
        }
    }
	
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysSetPPUEn
*
* DESCRIPTION:
*		This routine enables/disables Phy Polling Unit.
*
* INPUTS:
*		en - GT_TRUE to enable PPU, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetPPUEn
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL 		en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetPPUEn Called.\n"));
	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(en,data);

    /* Set the PPUEn bit.                */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,14,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetPPUEn
*
* DESCRIPTION:
*		This routine get the PPU state.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE if PPU is enabled, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK           - on success
*		GT_BAD_PARAM    - on bad parameter
*		GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetPPUEn
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetPPUEn Called.\n"));
	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    if(en == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* Get the GetPPUEn bit.                */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,14,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysSetDiscardExcessive
*
* DESCRIPTION:
*       This routine set the Discard Excessive state.
*
* INPUTS:
*       en - GT_TRUE Discard is enabled, GT_FALSE otherwise.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetDiscardExcessive
(
    IN  GT_QD_DEV *dev,
    IN GT_BOOL en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetDiscardExcessive Called.\n"));
    BOOL_2_BIT(en,data);

    /* Set the Discard Exissive bit.                */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,13,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysGetDiscardExcessive
*
* DESCRIPTION:
*       This routine get the Discard Excessive state.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE Discard is enabled, GT_FALSE otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetDiscardExcessive
(
    IN  GT_QD_DEV *dev,
    IN GT_BOOL    *en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetDiscardExcessive Called.\n"));
    if(en == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* Get the Discard Exissive bit.                */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,13,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysSetSchedulingMode
*
* DESCRIPTION:
*       This routine set the Scheduling Mode.
*
* INPUTS:
*       mode - GT_TRUE wrr, GT_FALSE strict.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetSchedulingMode
(
    IN  GT_QD_DEV *dev,
    IN GT_BOOL    mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetSchedulingMode Called.\n"));
    BOOL_2_BIT(mode,data);
    data = 1 - data;

    /* Set the Schecduling bit.             */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,11,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysGetSchedulingMode
*
* DESCRIPTION:
*       This routine get the Scheduling Mode.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE wrr, GT_FALSE strict.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetSchedulingMode
(
    IN  GT_QD_DEV *dev,
    OUT GT_BOOL   *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetSchedulingMode Called.\n"));
    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }
    /* Get the Scheduling bit.              */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,11,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(1 - data,*mode);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysSetMaxFrameSize
*
* DESCRIPTION:
*       This routine Set the max frame size allowed.
*
* INPUTS:
*       mode - GT_TRUE max size 1522, 
*			   GT_FALSE max size 1535, 1632, or 2048.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*		Please refer to the device spec. to get the max frame size.
*       88E6095 device supports upto 1632. 
*       88E6065/88E6061 devices support upto 2048. 
*
*******************************************************************************/
GT_STATUS gsysSetMaxFrameSize
(
    IN  GT_QD_DEV *dev,
    IN  GT_BOOL   mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetMaxFrameSize Called.\n"));
    BOOL_2_BIT(mode,data);
    data = 1 - data;

    /* Set the Max Fram Size bit.               */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,10,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysGetMaxFrameSize
*
* DESCRIPTION:
*       This routine Get the max frame size allowed.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE max size 1522, 
*			   GT_FALSE max size 1535, 1632, or 2048.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*		Please refer to the device spec. to get the max frame size.
*       88E6095 device supports upto 1632.
*       88E6065/88E6061 devices support upto 2048. 
*
*******************************************************************************/
GT_STATUS gsysGetMaxFrameSize
(
    IN  GT_QD_DEV *dev,
    OUT GT_BOOL   *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetMaxFrameSize Called.\n"));
    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }
    /* Get the Max Frame Size bit.          */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,10,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(1 - data,*mode);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysReLoad
*
* DESCRIPTION:
*       This routine cause to the switch to reload the EEPROM.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysReLoad
(
    IN  GT_QD_DEV *dev
)
{
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gsysReLoad Called.\n"));
    /* Set the Reload bit.                  */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,9,1,1);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    /* Should a check for reload completion. */

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysSetWatchDog
*
* DESCRIPTION:
*       This routine Set the the watch dog mode.
*
* INPUTS:
*       en - GT_TRUE enables, GT_FALSE disable.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetWatchDog
(
    IN  GT_QD_DEV *dev,
    IN  GT_BOOL   en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetWatchDog Called.\n"));

	/* Check if Switch supports this feature. */
	if (IS_IN_DEV_GROUP(dev,DEV_ENHANCED_FE_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(en,data);

    /* Set the WatchDog bit.            */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,7,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysGetWatchDog
*
* DESCRIPTION:
*       This routine Get the the watch dog mode.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE enables, GT_FALSE disable.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetWatchDog
(
    IN  GT_QD_DEV *dev,
    OUT GT_BOOL   *en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetWatchDog Called.\n"));

	/* Check if Switch supports this feature. */
	if (IS_IN_DEV_GROUP(dev,DEV_ENHANCED_FE_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    if(en == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* Get the WatchDog bit.            */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,7,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetDuplexPauseMac
*
* DESCRIPTION:
*       This routine sets the full duplex pause src Mac Address.
*
* INPUTS:
*       mac - The Mac address to be set.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetDuplexPauseMac
(
    IN GT_QD_DEV    *dev,
    IN GT_ETHERADDR *mac
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetDuplexPauseMac Called.\n"));
    if(mac == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* Set the first Mac register with diffAddr bit reset.  */
    data = (((*mac).arEther[0] & 0xFE) << 8) | (*mac).arEther[1];
    retVal = hwWriteGlobalReg(dev,QD_REG_MACADDR_01,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    /* Set the Mac23 address register.   */
    data = ((*mac).arEther[2] << 8) | (*mac).arEther[3];
    retVal = hwWriteGlobalReg(dev,QD_REG_MACADDR_23,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    /* Set the Mac45 address register.   */
    data = ((*mac).arEther[4] << 8) | (*mac).arEther[5];
    retVal = hwWriteGlobalReg(dev,QD_REG_MACADDR_45,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysGetDuplexPauseMac
*
* DESCRIPTION:
*       This routine Gets the full duplex pause src Mac Address.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mac - the Mac address.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetDuplexPauseMac
(
    IN  GT_QD_DEV    *dev,
    OUT GT_ETHERADDR *mac
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to read from register.  */

    DBG_INFO(("gsysGetDuplexPauseMac Called.\n"));
    if(mac == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* Get the Mac01 register.      */
    retVal = hwReadGlobalReg(dev,QD_REG_MACADDR_01,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
#if 0	/* This should be always a unicast. */
    /* The mac is allwasy a multicast mac   */
    (*mac).arEther[0] = (data >> 8) | 0x01;
    (*mac).arEther[1] = data & 0xFF;
#else
    (*mac).arEther[0] = (data >> 8) & ~0x01;
    (*mac).arEther[1] = data & 0xFF;
#endif
    /* Get the Mac23 register.      */
    retVal = hwReadGlobalReg(dev,QD_REG_MACADDR_23,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    (*mac).arEther[2] = data >> 8;
    (*mac).arEther[3] = data & 0xFF;

    /* Get the Mac45 register.      */
    retVal = hwReadGlobalReg(dev,QD_REG_MACADDR_45,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    (*mac).arEther[4] = data >> 8;
    (*mac).arEther[5] = data & 0xFF;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysSetPerPortDuplexPauseMac
*
* DESCRIPTION:
*       This routine sets whether the full duplex pause src Mac Address is per
*       port or per device.
*
* INPUTS:
*       en - GT_TURE per port mac, GT_FALSE global mac.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetPerPortDuplexPauseMac
(
    IN GT_QD_DEV    *dev,
    IN GT_BOOL      en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetPerPortDuplexPauseMac Called.\n"));
    BOOL_2_BIT(en,data);

    /* Set the WatchDog bit.            */
    retVal = hwSetGlobalRegField(dev,QD_REG_MACADDR_01,8,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysGetPerPortDuplexPauseMac
*
* DESCRIPTION:
*       This routine Gets whether the full duplex pause src Mac Address is per
*       port or per device.
*
* INPUTS:
*       en - GT_TURE per port mac, GT_FALSE global mac.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetPerPortDuplexPauseMac
(
    IN  GT_QD_DEV    *dev,
    OUT GT_BOOL      *en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetPerPortDuplexPauseMac Called.\n"));
    if(en == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* Get the Scheduling bit.              */
    retVal = hwGetGlobalRegField(dev,QD_REG_MACADDR_01,8,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysReadMiiReg
*
* DESCRIPTION:
*       This routine reads QuarterDeck Registers. Since this routine is only for
*		Diagnostic Purpose, no error checking will be performed.
*		User has to know which phy address(0 ~ 0x1F) will be read.		
*
* INPUTS:
*       phyAddr - Phy Address to read the register for.( 0 ~ 0x1F )
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
GT_STATUS gsysReadMiiReg
(
    IN  GT_QD_DEV    *dev,
    IN  GT_U32	     phyAddr,
    IN  GT_U32	     regAddr,
    OUT GT_U32	     *data
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          u16Data;           /* The register's read data.    */

    DBG_INFO(("gsysReadMiiRegister Called.\n"));

    /* Get the Scheduling bit.              */
    retVal = hwReadMiiReg(dev,(GT_U8)phyAddr,(GT_U8)regAddr,&u16Data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	*data = (GT_U32)u16Data;

    return GT_OK;
}

/*******************************************************************************
* gsysWriteMiiReg
*
* DESCRIPTION:
*       This routine writes QuarterDeck Registers. Since this routine is only for
*		Diagnostic Purpose, no error checking will be performed.
*		User has to know which phy address(0 ~ 0x1F) will be read.		
*
* INPUTS:
*       phyAddr - Phy Address to read the register for.( 0 ~ 0x1F )
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
GT_STATUS gsysWriteMiiReg
(
    IN  GT_QD_DEV    *dev,
    IN  GT_U32	     phyAddr,
    IN  GT_U32	     regAddr,
    IN  GT_U16	     data
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    
    DBG_INFO(("gsysWriteMiiRegister Called.\n"));

    /* Get the Scheduling bit.              */
    retVal = hwWriteMiiReg(dev,(GT_U8)phyAddr,(GT_U8)regAddr,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	return GT_OK;
}


/*******************************************************************************
* gsysSetRetransmitMode
*
* DESCRIPTION:
*       This routine set the Retransmit Mode.
*
* INPUTS:
*       en - GT_TRUE Retransimt Mode is enabled, GT_FALSE otherwise.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetRetransmitMode
(
    IN  GT_QD_DEV    *dev,
    IN  GT_BOOL      en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetRetransmitMode Called.\n"));
	/* Only Gigabit Switch supports this status. */
	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}
    BOOL_2_BIT(en,data);

    /* Set the Retransmit Mode bit.                */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,15,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysGetRetransmitMode
*
* DESCRIPTION:
*       This routine get the Retransmit Mode.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE Retransmit Mode is enabled, GT_FALSE otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetRetransmitMode
(
    IN  GT_QD_DEV    *dev,
    OUT GT_BOOL      *en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetRetransmitMode Called.\n"));
    if(en == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

	/* Only Gigabit Switch supports this status. */
	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}
    /* Get the bit.                */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,15,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetLimitBackoff
*
* DESCRIPTION:
*       This routine set the Limit Backoff bit.
*
* INPUTS:
*       en - GT_TRUE:  uses QoS half duplex backoff operation  
*            GT_FALSE: uses normal half duplex backoff operation
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetLimitBackoff
(
    IN  GT_QD_DEV    *dev,
    IN  GT_BOOL      en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetLimitBackoff Called.\n"));
	/* Only Gigabit Switch supports this status. */
	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}
    BOOL_2_BIT(en,data);

    /* Set the bit.                */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,14,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysGetLimitBackoff
*
* DESCRIPTION:
*       This routine set the Limit Backoff bit.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE:  uses QoS half duplex backoff operation  
*            GT_FALSE: uses normal half duplex backoff operation
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetLimitBackoff
(
    IN  GT_QD_DEV    *dev,
    OUT GT_BOOL      *en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetLimitBackoff Called.\n"));
    if(en == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }
	/* Only Gigabit Switch supports this status. */
	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* Get the bit.                */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,14,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetRsvRegPri
*
* DESCRIPTION:
*       This routine set the Reserved Queue's Requesting Priority 
*
* INPUTS:
*       en - GT_TRUE: use the last received frome's priority
*            GT_FALSE:use the last switched frame's priority 
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetRsvReqPri
(
    IN  GT_QD_DEV    *dev,
    IN  GT_BOOL      en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetRsvReqPri Called.\n"));
	/* Only Gigabit Switch supports this status. */
	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}
    BOOL_2_BIT(en,data);

    /* Set the bit.                */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,12,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gsysGetRsvReqPri
*
* DESCRIPTION:
*       This routine get the Reserved Queue's Requesting Priority 
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE: use the last received frome's priority
*            GT_FALSE:use the last switched frame's priority 
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetRsvReqPri
(
    IN  GT_QD_DEV    *dev,
    OUT GT_BOOL      *en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetRsvReqPri Called.\n"));
    if(en == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }
	/* Only Gigabit Switch supports this status. */
	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* Get the bit.                */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,12,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysSetCascadePort
*
* DESCRIPTION:
*		This routine sets Cascade Port number.
*		In multichip systems frames coming from a CPU need to know when they
*		have reached their destination chip.
*
*		Use Cascade Port = 0xE to indicate this chip has no Cascade port.
*		Use Cascade Port = 0xF to use Routing table (gsysGetDevRoutingTable).
*
* INPUTS:
*		port - Cascade Port
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetCascadePort
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysSetCascadePort Called.\n"));
	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
	if((port == 0xE) || (port == 0xF))
		data = (GT_U16)port;
	else
	{
	    data = (GT_U16)(GT_LPORT_2_PORT(port));
		if (data == GT_INVALID_PORT)
			return GT_BAD_PARAM;
	}

    /* Set the Cascade port.                */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL2,12,4,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetCascadePort
*
* DESCRIPTION:
*		This routine gets Cascade Port number.
*		In multichip systems frames coming from a CPU need to know when they
*		have reached their destination chip.
*
*		Use Cascade Port = 0xE to indicate this chip has no Cascade port.
*		Use Cascade Port = 0xF to use Routing table (gsysGetDevRoutingTable).
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		port - Cascade Port
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetCascadePort
(
	IN  GT_QD_DEV	*dev,
	OUT GT_LPORT 	*port
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysSetCascadePort Called.\n"));
	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the Cascade port.                */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL2,12,4,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	if((data == 0xE) || (data == 0xF))
	{
		*port = (GT_LPORT)data;
	}
	else
	{
	    *port = GT_PORT_2_LPORT(data);
	}
    
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysSetDeviceNumber
*
* DESCRIPTION:
*		This routine sets Device Number.
*		In multichip systems frames coming from a CPU need to know when they
*		have reached their destination chip. From CPU frames whose Dev_Num
*		fieldmatches these bits have reachedtheir destination chip and are sent
*		out this chip using the port number indicated in the frame's Trg_Port 
*		field.
*
* INPUTS:
*		devNum - Device Number (0 ~ 31)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetDeviceNumber
(
	IN GT_QD_DEV	*dev,
	IN GT_U32  		devNum
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysSetDeviceNumber Called.\n"));
	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    data = ((GT_U16)devNum) & 0x1F; /* only 5 bits are valid */

    /* Set the Device Number.                */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL2,0,5,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetDeviceNumber
*
* DESCRIPTION:
*		This routine gets Device Number.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		devNum - Device Number (0 ~ 31)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetDeviceNumber
(
	IN  GT_QD_DEV	*dev,
	OUT GT_U32  	*devNum
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysGetDeviceNumber Called.\n"));
	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the Device Number.                */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL2,0,5,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *devNum = (GT_U32)data;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysSetCoreTagType
*
* DESCRIPTION:
*		This routine sets Ether Core Tag Type.
*		This Ether Type is added to frames that egress the switch as Double Tagged 
*		frames. It is also the Ether Type expected during Ingress to determine if 
*		a frame is Tagged or not on ports configured as UseCoreTag mode.
*
* INPUTS:
*		etherType - Core Tag Type (2 bytes)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetCoreTagType
(
	IN GT_QD_DEV	*dev,
	IN GT_U16  		etherType
)
{
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gsysSetCoreTagType Called.\n"));

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Set the Ether Type */
    retVal = hwWriteGlobalReg(dev,QD_REG_CORETAG_TYPE,etherType);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetCoreTagType
*
* DESCRIPTION:
*		This routine gets CoreTagType
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		etherType - Core Tag Type (2 bytes)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetCoreTagType
(
	IN  GT_QD_DEV	*dev,
	OUT GT_U16  	*etherType
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysGetCoreTagType Called.\n"));

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the Device Number.                */
    retVal = hwReadGlobalReg(dev,QD_REG_CORETAG_TYPE,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *etherType = data;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysSetIngressMonitorDest
*
* DESCRIPTION:
*		This routine sets Ingress Monitor Destination Port. Frames that are 
*		targeted toward an Ingress Monitor Destination go out the port number 
*		indicated in these bits. This includes frames received on a Marvell Tag port
*		with the Ingress Monitor type, and frames received on a Network port that 
*		is enabled to be the Ingress Monitor Source Port.
*		If the Ingress Monitor Destination Port resides in this device these bits 
*		should point to the Network port where these frames are to egress. If the 
*		Ingress Monitor Destination Port resides in another device these bits 
*		should point to the Marvell Tag port in this device that is used to get 
*		to the device that contains the Ingress Monitor Destination Port.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetIngressMonitorDest
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gsysSetIngressMonitorDest Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
	if(port == 0xF)
		hwPort = (GT_U16)port;
	else
	{
	    hwPort = (GT_U16)(GT_LPORT_2_PORT(port));
		if (hwPort == GT_INVALID_PORT)
			return GT_BAD_PARAM;
	}

    /* Set the Ether Type */
    retVal = hwSetGlobalRegField(dev,QD_REG_MONITOR_CONTROL, 12, 4, (GT_U16)hwPort);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetIngressMonitorDest
*
* DESCRIPTION:
*		This routine gets Ingress Monitor Destination Port.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		port  - the logical port number.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetIngressMonitorDest
(
	IN  GT_QD_DEV	*dev,
	OUT GT_LPORT  	*port
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysGetIngressMonitorDest Called.\n"));

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the IngressMonitorDest. */
    retVal = hwGetGlobalRegField(dev,QD_REG_MONITOR_CONTROL, 12, 4, &data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	if(data == 0xF)
	{
		*port = (GT_LPORT)data;
	}
	else
	{
	    *port = GT_PORT_2_LPORT(data);
	}
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysSetEgressMonitorDest
*
* DESCRIPTION:
*		This routine sets Egress Monitor Destination Port. Frames that are 
*		targeted toward an Egress Monitor Destination go out the port number 
*		indicated in these bits. This includes frames received on a Marvell Tag port
*		with the Egress Monitor type, and frames transmitted on a Network port that 
*		is enabled to be the Egress Monitor Source Port.
*		If the Egress Monitor Destination Port resides in this device these bits 
*		should point to the Network port where these frames are to egress. If the 
*		Egress Monitor Destination Port resides in another device these bits 
*		should point to the Marvell Tag port in this device that is used to get 
*		to the device that contains the Egress Monitor Destination Port.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetEgressMonitorDest
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gsysSetEgressMonitorDest Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	if(port == 0xF)
		hwPort = (GT_U16)port;
	else
	{
	    hwPort = (GT_U16)(GT_LPORT_2_PORT(port));
		if (hwPort == GT_INVALID_PORT)
			return GT_BAD_PARAM;
	}

    /* Set EgressMonitorDest */
    retVal = hwSetGlobalRegField(dev,QD_REG_MONITOR_CONTROL, 8, 4, (GT_U16)hwPort);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetEgressMonitorDest
*
* DESCRIPTION:
*		This routine gets Egress Monitor Destination Port.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		port  - the logical port number.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetEgressMonitorDest
(
	IN  GT_QD_DEV	*dev,
	OUT GT_LPORT  	*port
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysGetEgressMonitorDest Called.\n"));

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the EgressMonitorDest. */
    retVal = hwGetGlobalRegField(dev,QD_REG_MONITOR_CONTROL, 8, 4, &data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	if(data == 0xF)
	{
		*port = (GT_LPORT)data;
	}
	else
	{
	    *port = GT_PORT_2_LPORT(data);
	}
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetARPDest
*
* DESCRIPTION:
*		This routine sets ARP Monitor Destination Port. Tagged or untagged 
*		frames ingress Network ports that have the Broadcast Destination Address 
*		with an Ethertype of 0x0806 are mirrored to this port. The ARPDest 
*		should point to the port that directs these frames to the switch's CPU 
*		that will process ARPs. This target port should be a Marvell Tag port so 
*		that frames will egress with a To CPU Marvell Tag with a CPU Code of ARP.
*		To CPU Marvell Tag frames with a CPU Code off ARP that ingress a Marvell 
*		Tag port will be sent to the port number defineded in ARPDest.
*
*		If ARPDest =  0xF, ARP Monitoring is disabled and ingressing To CPU ARP 
*		frames will be discarded.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetARPDest
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gsysSetARPDest Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* Check if Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ARP_SUPPORT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
	if(port == 0xF)
		hwPort = (GT_U16)port;
	else
	{
	    hwPort = (GT_U16)(GT_LPORT_2_PORT(port));
		if (hwPort == GT_INVALID_PORT)
			return GT_BAD_PARAM;
	}

    /* Set related bit */
    retVal = hwSetGlobalRegField(dev,QD_REG_MONITOR_CONTROL, 4, 4, (GT_U16)hwPort);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetARPDest
*
* DESCRIPTION:
*		This routine gets ARP Monitor Destination Port. Tagged or untagged 
*		frames ingress Network ports that have the Broadcast Destination Address 
*		with an Ethertype of 0x0806 are mirrored to this port. The ARPDest 
*		should point to the port that directs these frames to the switch's CPU 
*		that will process ARPs. This target port should be a Marvell Tag port so 
*		that frames will egress with a To CPU Marvell Tag with a CPU Code of ARP.
*		To CPU Marvell Tag frames with a CPU Code off ARP that ingress a Marvell 
*		Tag port will be sent to the port number defineded in ARPDest.
*
*		If ARPDest =  0xF, ARP Monitoring is disabled and ingressing To CPU ARP 
*		frames will be discarded.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		port  - the logical port number.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetARPDest
(
	IN  GT_QD_DEV	*dev,
	OUT GT_LPORT  	*port
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysGetARPDest Called.\n"));

	/* Check if Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ARP_SUPPORT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get related bit */
    retVal = hwGetGlobalRegField(dev,QD_REG_MONITOR_CONTROL, 4, 4, &data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	if(data == 0xF)
	{
		*port = (GT_LPORT)data;
	}
	else
	{
	    *port = GT_PORT_2_LPORT(data);
	}
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetRsvd2CpuEnables
*
* DESCRIPTION:
*		Reserved DA Enables. When the Rsvd2Cpu(gsysSetRsvd2Cpu) is set to a one, 
*		the 16 reserved multicast DA addresses, whose bit in this register are 
*		also set to a one, are treadted as MGMT frames. All the reserved DA's 
*		take the form 01:80:C2:00:00:0x. When x = 0x0, bit 0 of this register is 
*		tested. When x = 0x2, bit 2 of this field is tested and so on.
*		If the tested bit in this register is cleared to a zero, the frame will 
*		be treated as a normal (non-MGMT) frame.
*
* INPUTS:
*		enBits - bit vector of enabled Reserved Multicast.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetRsvd2CpuEnables
(
	IN GT_QD_DEV	*dev,
	IN GT_U16		enBits
)
{
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gsysSetRsvd2CpuEnables Called.\n"));

	/* Check if Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ENHANCED_MULTICAST))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Set related register */
    retVal = hwWriteGlobal2Reg(dev,QD_REG_MGMT_ENABLE, (GT_U16)enBits);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetRsvd2CpuEnables
*
* DESCRIPTION:
*		Reserved DA Enables. When the Rsvd2Cpu(gsysSetRsvd2Cpu) is set to a one, 
*		the 16 reserved multicast DA addresses, whose bit in this register are 
*		also set to a one, are treadted as MGMT frames. All the reserved DA's 
*		take the form 01:80:C2:00:00:0x. When x = 0x0, bit 0 of this register is 
*		tested. When x = 0x2, bit 2 of this field is tested and so on.
*		If the tested bit in this register is cleared to a zero, the frame will 
*		be treated as a normal (non-MGMT) frame.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		enBits - bit vector of enabled Reserved Multicast.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetRsvd2CpuEnables
(
	IN  GT_QD_DEV	*dev,
	OUT GT_U16  	*enBits
)
{
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gsysGetRsvd2CpuEnables Called.\n"));

	/* Check if Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ENHANCED_MULTICAST))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get related register */
    retVal = hwReadGlobal2Reg(dev, QD_REG_MGMT_ENABLE, enBits);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetRsvd2Cpu
*
* DESCRIPTION:
*		When the Rsvd2Cpu is set to a one(GT_TRUE), frames with a Destination 
*		Address in the range 01:80:C2:00:00:0x, regardless of their VLAN 
*		membership, will be considered MGMT frames and sent to the CPU Port.
*		If device supports Rsvd2CpuEnable (gsysSetRsvd2CpuEnable function),
*		the frame will be considered MGMT frame when the associated Rsvd2CpuEnable
*		bit for the frames's DA is also set to a one.
*
* INPUTS:
*		en - GT_TRUE if Rsvd2Cpu is set. GT_FALSE, otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetRsvd2Cpu
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("gsysSetRsvd2Cpu Called.\n"));

	/* Check if Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ENHANCED_MULTICAST|DEV_MULTICAST))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(en,data);

    /* Set related bit */
	if (IS_IN_DEV_GROUP(dev,DEV_MULTICAST))
	{
	    retVal = hwSetGlobalRegField(dev,QD_REG_MANGEMENT_CONTROL,3,1, data);
	}
	else
	{
	    retVal = hwSetGlobal2RegField(dev,QD_REG_MANAGEMENT, 3, 1, data);
	}

    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetRsvd2Cpu
*
* DESCRIPTION:
*		When the Rsvd2Cpu is set to a one(GT_TRUE), frames with a Destination 
*		Address in the range 01:80:C2:00:00:0x, regardless of their VLAN 
*		membership, will be considered MGMT frames and sent to the CPU Port.
*		If device supports Rsvd2CpuEnable (gsysSetRsvd2CpuEnable function),
*		the frame will be considered MGMT frame when the associated Rsvd2CpuEnable
*		bit for the frames's DA is also set to a one.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE if Rsvd2Cpu is set. GT_FALSE, otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetRsvd2Cpu
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysGetRsvd2Cpu Called.\n"));

	/* Check if Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ENHANCED_MULTICAST|DEV_MULTICAST))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get related bit */
	if (IS_IN_DEV_GROUP(dev,DEV_MULTICAST))
	{
	    retVal = hwGetGlobalRegField(dev,QD_REG_MANGEMENT_CONTROL,3,1,&data);
	}
	else
	{
	    retVal = hwGetGlobal2RegField(dev,QD_REG_MANAGEMENT,3,1,&data);
	}

    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysSetMGMTPri
*
* DESCRIPTION:
*		These bits are used as the PRI[2:0] bits on Rsvd2CPU frames.
*
* INPUTS:
*		pri - PRI[2:0] bits (should be less than 8)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - If pri is not less than 8.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetMGMTPri
(
	IN GT_QD_DEV	*dev,
	IN GT_U16		pri
)
{
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gsysSetMGMTPri Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ENHANCED_MULTICAST))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	if (pri > 0x7)
	{
        DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
	}

    /* Set related bit */
    retVal = hwSetGlobal2RegField(dev,QD_REG_MANAGEMENT, 0, 3, pri);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetMGMTPri
*
* DESCRIPTION:
*		These bits are used as the PRI[2:0] bits on Rsvd2CPU frames.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		pri - PRI[2:0] bits (should be less than 8)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetMGMTPri
(
	IN  GT_QD_DEV	*dev,
	OUT GT_U16  	*pri
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    DBG_INFO(("gsysGetMGMTPri Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ENHANCED_MULTICAST))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get related bit */
    retVal = hwGetGlobal2RegField(dev,QD_REG_MANAGEMENT,0,3,pri);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetUseDoubleTagData
*
* DESCRIPTION:
*		This bit is used to determine if Double Tag data that is removed from a 
*		Double Tag frame is used or ignored when making switching decisions on 
*		the frame.
*
* INPUTS:
*		en - GT_TRUE to use removed tag data, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetUseDoubleTagData
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("gsysSetUseDoubleTagData Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_MARVELL_TAG_LOOP_BLOCK))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(en,data);

    /* Set related bit */
    retVal = hwSetGlobal2RegField(dev,QD_REG_MANAGEMENT, 15, 1, data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetUseDoubleTagData
*
* DESCRIPTION:
*		This bit is used to determine if Double Tag data that is removed from a 
*		Double Tag frame is used or ignored when making switching decisions on 
*		the frame.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE if removed tag data is used, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetUseDoubleTagData
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysGetUseDoubleTagData Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_MARVELL_TAG_LOOP_BLOCK))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get related bit */
    retVal = hwGetGlobal2RegField(dev,QD_REG_MANAGEMENT,15,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetPreventLoops
*
* DESCRIPTION:
*		When a Marvell Tag port receives a Forward Marvell Tag whose Src_Dev 
*		field equals this device's Device Number, the following action will be 
*		taken depending upon the value of this bit.
*		GT_TRUE (1) - The frame will be discarded.
*		GT_FALSE(0) - The frame will be prevented from going out its original 
*						source port as defined by the frame's Src_Port field.
*
* INPUTS:
*		en - GT_TRUE to discard the frame as described above, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetPreventLoops
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("gsysSetPreventLoops Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_MARVELL_TAG_LOOP_BLOCK))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(en,data);

    /* Set related bit */
    retVal = hwSetGlobal2RegField(dev,QD_REG_MANAGEMENT, 14, 1, data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetPreventLoops
*
* DESCRIPTION:
*		When a Marvell Tag port receives a Forward Marvell Tag whose Src_Dev 
*		field equals this device's Device Number, the following action will be 
*		taken depending upon the value of this bit.
*		GT_TRUE (1) - The frame will be discarded.
*		GT_FALSE(0) - The frame will be prevented from going out its original 
*						source port as defined by the frame's Src_Port field.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE to discard the frame as described above, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetPreventLoops
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysGetPreventLoops Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_MARVELL_TAG_LOOP_BLOCK))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get related bit */
    retVal = hwGetGlobal2RegField(dev,QD_REG_MANAGEMENT,14,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysSetFlowControlMessage
*
* DESCRIPTION:
*		When this bit is set to one, Marvell Tag Flow Control messages will be 
*		generated when an output queue becomes congested and received Marvell Tag 
*		Flow Control messages will pause MACs inside this device. When this bit 
*		is cleared to a zero Marvell Tag Flow Control messages will not be 
*		generated and any received will be ignored at the target MAC.
*
* INPUTS:
*		en - GT_TRUE to use Marvell Tag Flow Control message, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetFlowControlMessage
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("gsysSetFlowControlMessage Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_MARVELL_TAG_FLOW_CTRL))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(en,data);

    /* Set related bit */
    retVal = hwSetGlobal2RegField(dev,QD_REG_MANAGEMENT, 13, 1, data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetFlowControlMessage
*
* DESCRIPTION:
*		When this bit is set to one, Marvell Tag Flow Control messages will be 
*		generated when an output queue becomes congested and received Marvell Tag 
*		Flow Control messages will pause MACs inside this device. When this bit 
*		is cleared to a zero Marvell Tag Flow Control messages will not be 
*		generated and any received will be ignored at the target MAC.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE to use Marvell Tag Flow Control message, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetFlowControlMessage
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysGetFlowControlMessage Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_MARVELL_TAG_FLOW_CTRL))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get related bit */
    retVal = hwGetGlobal2RegField(dev,QD_REG_MANAGEMENT,13,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysSetForceFlowControlPri
*
* DESCRIPTION:
*		When this bit is set to a one the PRI[2:0] bits of generated Marvell Tag 
*		Flow Control frames will be set to the value of the FC Pri bits (set by 
*		gsysSetFCPri function call). When this bit is cleared to a zero generated 
*		Marvell Tag Flow Control frames will retain the PRI[2:0] bits from the 
*		frames that caused the congestion. This bit will have no effect if the 
*		FlowControlMessage bit(gsysSetFlowControlMessage function call) is 
*		cleared to a zero.
*
* INPUTS:
*		en - GT_TRUE to use defined PRI bits, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetForceFlowControlPri
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("gsysSetForceFlowControlPri Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_MARVELL_TAG_FLOW_CTRL))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(en,data);

    /* Set related bit */
    retVal = hwSetGlobal2RegField(dev,QD_REG_MANAGEMENT, 7, 1, data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetForceFlowControlPri
*
* DESCRIPTION:
*		When this bit is set to a one the PRI[2:0] bits of generated Marvell Tag 
*		Flow Control frames will be set to the value of the FC Pri bits (set by 
*		gsysSetFCPri function call). When this bit is cleared to a zero generated 
*		Marvell Tag Flow Control frames will retain the PRI[2:0] bits from the 
*		frames that caused the congestion. This bit will have no effect if the 
*		FlowControlMessage bit(gsysSetFlowControlMessage function call) is 
*		cleared to a zero.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE to use defined PRI bits, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetForceFlowControlPri
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysGetForceFlowControlPri Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_MARVELL_TAG_FLOW_CTRL))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get related bit */
    retVal = hwGetGlobal2RegField(dev,QD_REG_MANAGEMENT,7,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysSetFCPri
*
* DESCRIPTION:
*		These bits are used as the PRI[2:0] bits on generated Marvell Tag Flow 
*		Control frames if the ForceFlowControlPri bit(gsysSetForceFlowControlPri)
*		is set to a one.
*
* INPUTS:
*		pri - PRI[2:0] bits (should be less than 8)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - If pri is not less than 8.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetFCPri
(
	IN GT_QD_DEV	*dev,
	IN GT_U16		pri
)
{
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gsysSetFCPri Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_MARVELL_TAG_FLOW_CTRL))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	if (pri > 0x7)
	{
        DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
	}

    /* Set related bit */
    retVal = hwSetGlobal2RegField(dev,QD_REG_MANAGEMENT, 4, 3, pri);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetFCPri
*
* DESCRIPTION:
*		These bits are used as the PRI[2:0] bits on generated Marvell Tag Flow 
*		Control frames if the ForceFlowControlPri bit(gsysSetForceFlowControlPri)
*		is set to a one.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		pri - PRI[2:0] bits (should be less than 8)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetFCPri
(
	IN  GT_QD_DEV	*dev,
	OUT GT_U16  	*pri
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    DBG_INFO(("gsysGetFCPri Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_MARVELL_TAG_FLOW_CTRL))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get related bit */
    retVal = hwGetGlobal2RegField(dev,QD_REG_MANAGEMENT,4,3,pri);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetFlowCtrlDelay
*
* DESCRIPTION:
*		This function sets Flow control delay time for 10Mbps, 100Mbps, and 
*		1000Mbps. 
*
* INPUTS:
*		sp - PORT_SPEED_10_MBPS, PORT_SPEED_100_MBPS, or PORT_SPEED_1000_MBPS
*		delayTime - actual delay time will be (this value x 2.048uS).
*					the value cannot exceed 0x1FFF (or 8191 in decimal).
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if sp is not valid or delayTime is > 0x1FFF.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetFlowCtrlDelay
(
	IN GT_QD_DEV			*dev,
	IN GT_PORT_SPEED_MODE	sp,
	IN GT_U32				delayTime
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("gsysSetFlowCtrlDelay Called.\n"));

	/* Check if Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_FLOW_CTRL_DELAY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	gtSemTake(dev,dev->tblRegsSem,OS_WAIT_FOREVER);

	/* Check if the register can be accessed. */
	do
	{
	    retVal = hwReadGlobal2Reg(dev,QD_REG_FLOWCTRL_DELAY,&data);
    	if(retVal != GT_OK)
	    {
    	    DBG_INFO(("Failed.\n"));
			gtSemGive(dev,dev->tblRegsSem);
        	return retVal;
	    }
	} while (data & 0x8000);

	switch(sp)
	{
		case PORT_SPEED_10_MBPS:
				data = 0;
				break;
		case PORT_SPEED_100_MBPS:
				data = 1 << 13;
				break;
		case PORT_SPEED_1000_MBPS:
				data = 2 << 13;
				break;
		default:
		        DBG_INFO(("GT_BAD_PARAM (sp)\n"));
				gtSemGive(dev,dev->tblRegsSem);
				return GT_BAD_PARAM;
	}

	if (delayTime > 0x1FFF)
	{
		DBG_INFO(("GT_BAD_PARAM (delayTime)\n"));
		gtSemGive(dev,dev->tblRegsSem);
		return GT_BAD_PARAM;
	}

	data |= (GT_U16)(0x8000 | delayTime);

    /* Set related register */
    retVal = hwWriteGlobal2Reg(dev,QD_REG_FLOWCTRL_DELAY,data);

	gtSemGive(dev,dev->tblRegsSem);

    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetFlowCtrlDelay
*
* DESCRIPTION:
*		This function retrieves Flow control delay time for 10Mbps, 100Mbps, and
*		1000Mbps. 
*
* INPUTS:
*		sp - PORT_SPEED_10_MBPS, PORT_SPEED_100_MBPS, or PORT_SPEED_1000_MBPS
*
* OUTPUTS:
*		delayTime - actual delay time will be (this value x 2.048uS).
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if sp is not valid or delayTime is > 0x1FFF.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetFlowCtrlDelay
(
	IN  GT_QD_DEV	*dev,
	IN  GT_PORT_SPEED_MODE	sp,
	OUT GT_U32		*delayTime
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysGetFlowCtrlDelay Called.\n"));

	/* Check if Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_FLOW_CTRL_DELAY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	gtSemTake(dev,dev->tblRegsSem,OS_WAIT_FOREVER);

	/* Check if the register can be accessed. */
	do
	{
	    retVal = hwReadGlobal2Reg(dev,QD_REG_FLOWCTRL_DELAY,&data);
    	if(retVal != GT_OK)
	    {
    	    DBG_INFO(("Failed.\n"));
			gtSemGive(dev,dev->tblRegsSem);
        	return retVal;
	    }
	} while (data & 0x8000);

	switch(sp)
	{
		case PORT_SPEED_10_MBPS:
				data = 0;
				break;
		case PORT_SPEED_100_MBPS:
				data = 1 << 13;
				break;
		case PORT_SPEED_1000_MBPS:
				data = 2 << 13;
				break;
		default:
		        DBG_INFO(("GT_BAD_PARAM (sp)\n"));
				gtSemGive(dev,dev->tblRegsSem);
				return GT_BAD_PARAM;
	}

    retVal = hwWriteGlobal2Reg(dev,QD_REG_FLOWCTRL_DELAY,data);
   	if(retVal != GT_OK)
    {
   	    DBG_INFO(("Failed.\n"));
		gtSemGive(dev,dev->tblRegsSem);
       	return retVal;
    }

    retVal = hwReadGlobal2Reg(dev,QD_REG_FLOWCTRL_DELAY,&data);

	gtSemGive(dev,dev->tblRegsSem);

   	if(retVal != GT_OK)
    {
   	    DBG_INFO(("Failed.\n"));
       	return retVal;
    }

	*delayTime = (GT_U32)(data & 0x1FFF);
		
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetDevRoutingTable
*
* DESCRIPTION:
*		This function sets Device to Port mapping (which device is connected to 
*		which port of this device). 
*
* INPUTS:
*		devNum - target device number.
*		portNum - the logical port number.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if sp is not valid or delayTime is > 0x1FFF.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetDevRoutingTable
(
	IN GT_QD_DEV	*dev,
	IN GT_U32  		devNum,
	IN GT_LPORT 	port
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysSetDevRoutingTable Called.\n"));

	/* Check if Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_STACKING))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	gtSemTake(dev,dev->tblRegsSem,OS_WAIT_FOREVER);

	/* Check if the register can be accessed. */
	do
	{
	    retVal = hwReadGlobal2Reg(dev,QD_REG_ROUTING_TBL,&data);
    	if(retVal != GT_OK)
	    {
    	    DBG_INFO(("Failed.\n"));
			gtSemGive(dev,dev->tblRegsSem);
        	return retVal;
	    }
	} while (data & 0x8000);

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if(devNum > 0x1F)
	{
		DBG_INFO(("GT_BAD_PARAM (devNum)\n"));
		gtSemGive(dev,dev->tblRegsSem);
		return GT_BAD_PARAM;
	}

	if(port >= dev->numOfPorts)
	{
		hwPort = 0xF;
	}

	data = 0x8000 | (devNum << 8) | hwPort;

    /* Set related register */
    retVal = hwWriteGlobal2Reg(dev,QD_REG_ROUTING_TBL,data);

	gtSemGive(dev,dev->tblRegsSem);

    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetDevRoutingTable
*
* DESCRIPTION:
*		This function gets Device to Port mapping (which device is connected to 
*		which port of this device). 
*
* INPUTS:
*		devNum - target device number.
*
* OUTPUTS:
*		portNum - the logical port number.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if sp is not valid or delayTime is > 0x1FFF.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetDevRoutingTable
(
	IN  GT_QD_DEV	*dev,
	IN  GT_U32 		devNum,
	OUT GT_LPORT 	*port
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetDevRoutingTable Called.\n"));

	/* Check if Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_STACKING))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	gtSemTake(dev,dev->tblRegsSem,OS_WAIT_FOREVER);

	/* Check if the register can be accessed. */
	do
	{
	    retVal = hwReadGlobal2Reg(dev,QD_REG_ROUTING_TBL,&data);
    	if(retVal != GT_OK)
	    {
    	    DBG_INFO(("Failed.\n"));
			gtSemGive(dev,dev->tblRegsSem);
        	return retVal;
	    }
	} while (data & 0x8000);

	if(devNum > 0x1F)
	{
		DBG_INFO(("GT_BAD_PARAM (devNum)\n"));
		gtSemGive(dev,dev->tblRegsSem);
		return GT_BAD_PARAM;
	}

	data = devNum << 8;

    retVal = hwWriteGlobal2Reg(dev,QD_REG_ROUTING_TBL,data);
   	if(retVal != GT_OK)
    {
   	    DBG_INFO(("Failed.\n"));
		gtSemGive(dev,dev->tblRegsSem);
       	return retVal;
    }

    retVal = hwReadGlobal2Reg(dev,QD_REG_ROUTING_TBL,&data);

	gtSemGive(dev,dev->tblRegsSem);

   	if(retVal != GT_OK)
    {
   	    DBG_INFO(("Failed.\n"));
       	return retVal;
    }

	*port = GT_PORT_2_LPORT(data & 0xF);
	if(*port == GT_INVALID_PORT)
	{
		*port = 0xF;
	}	
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysSetTrunkMaskTable
*
* DESCRIPTION:
*		This function sets Trunk Mask for the given Trunk Number.
*
* INPUTS:
*		trunkNum - Trunk Number.
*		trunkMask - Trunk mask bits. Bit 0 controls trunk masking for port 0,
*					bit 1 for port 1 , etc.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if trunkNum > 0x7 for 88E6095 and 88E6183 family and
*					   if trunkNum > 0x3 for 88E6065 family.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetTrunkMaskTable
(
	IN GT_QD_DEV	*dev,
	IN GT_U32  		trunkNum,
	IN GT_U32		trunkMask
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
	GT_U32			mask;

    DBG_INFO(("gsysSetTrunkMaskTable Called.\n"));

	/* Check if Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_TRUNK|DEV_REDUCED_TRUNK))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	gtSemTake(dev,dev->tblRegsSem,OS_WAIT_FOREVER);

	/* Check if the register can be accessed. */
	do
	{
	    retVal = hwReadGlobal2Reg(dev,QD_REG_TRUNK_MASK_TBL,&data);
    	if(retVal != GT_OK)
	    {
    	    DBG_INFO(("Failed.\n"));
			gtSemGive(dev,dev->tblRegsSem);
        	return retVal;
	    }
	} while (data & 0x8000);

	data &= 0x0800;

	if(trunkNum > 0x7)
	{
		DBG_INFO(("GT_BAD_PARAM (trunkNum)\n"));
		gtSemGive(dev,dev->tblRegsSem);
		return GT_BAD_PARAM;
	}

	if((trunkNum > 0x3) && IS_IN_DEV_GROUP(dev,DEV_REDUCED_TRUNK))
	{
		DBG_INFO(("GT_BAD_PARAM (trunkNum)\n"));
		gtSemGive(dev,dev->tblRegsSem);
		return GT_BAD_PARAM;
	}

	mask = (1 << dev->numOfPorts) - 1;

	if(trunkMask > mask)
	{
		DBG_INFO(("GT_BAD_PARAM (trunkMask)\n"));
		gtSemGive(dev,dev->tblRegsSem);
		return GT_BAD_PARAM;
	}

	mask = GT_LPORTVEC_2_PORTVEC(trunkMask);

	data = 0x8000 | data | (trunkNum << 12) | mask;

    /* Set related register */
    retVal = hwWriteGlobal2Reg(dev,QD_REG_TRUNK_MASK_TBL,data);

	gtSemGive(dev,dev->tblRegsSem);

    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetTrunkMaskTable
*
* DESCRIPTION:
*		This function gets Trunk Mask for the given Trunk Number.
*
* INPUTS:
*		trunkNum - Trunk Number.
*
* OUTPUTS:
*		trunkMask - Trunk mask bits. Bit 0 controls trunk masking for port 0,
*					bit 1 for port 1 , etc.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if trunkNum > 0x7 for 88E6095 and 88E6183 family and
*					   if trunkNum > 0x3 for 88E6065 family.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetTrunkMaskTable
(
	IN  GT_QD_DEV	*dev,
	IN  GT_U32 		trunkNum,
	OUT GT_U32		*trunkMask
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
	GT_U32			mask;

    DBG_INFO(("gsysGetTrunkMaskTable Called.\n"));

	/* Check if Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_TRUNK|DEV_REDUCED_TRUNK))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	gtSemTake(dev,dev->tblRegsSem,OS_WAIT_FOREVER);

	/* Check if the register can be accessed. */
	do
	{
	    retVal = hwReadGlobal2Reg(dev,QD_REG_TRUNK_MASK_TBL,&data);
    	if(retVal != GT_OK)
	    {
    	    DBG_INFO(("Failed.\n"));
			gtSemGive(dev,dev->tblRegsSem);
        	return retVal;
	    }
	} while (data & 0x8000);

	data &= 0x0800;

	if(trunkNum > 0x7)
	{
		DBG_INFO(("GT_BAD_PARAM (trunkId)\n"));
		gtSemGive(dev,dev->tblRegsSem);
		return GT_BAD_PARAM;
	}

	if((trunkNum > 0x3) && IS_IN_DEV_GROUP(dev,DEV_REDUCED_TRUNK))
	{
		DBG_INFO(("GT_BAD_PARAM (trunkNum)\n"));
		gtSemGive(dev,dev->tblRegsSem);
		return GT_BAD_PARAM;
	}

	data = data | (trunkNum << 12);

    retVal = hwWriteGlobal2Reg(dev,QD_REG_TRUNK_MASK_TBL,data);
   	if(retVal != GT_OK)
    {
   	    DBG_INFO(("Failed.\n"));
		gtSemGive(dev,dev->tblRegsSem);
       	return retVal;
    }

    retVal = hwReadGlobal2Reg(dev,QD_REG_TRUNK_MASK_TBL,&data);

	gtSemGive(dev,dev->tblRegsSem);

   	if(retVal != GT_OK)
    {
   	    DBG_INFO(("Failed.\n"));
       	return retVal;
    }

	mask = (1 << dev->maxPorts) - 1;

	*trunkMask = GT_PORTVEC_2_LPORTVEC(data & mask);
			
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysSetHashTrunk
*
* DESCRIPTION:
*		Hash DA & SA for TrunkMask selection. Trunk load balancing is accomplished 
*		by using the frame's DA and SA fields to access one of eight Trunk Masks. 
*		When this bit is set to a one the hashed computed for address table 
*		lookups is used for the TrunkMask selection. When this bit is cleared to 
*		a zero the lower 3 bits of the frame's DA and SA are XOR'ed together to 
*		select the TrunkMask to use.
*
* INPUTS:
*		en - GT_TRUE to use lookup table, GT_FALSE to use XOR.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetHashTrunk
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("gsysSetHashTrunk Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_TRUNK|DEV_REDUCED_TRUNK))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(en,data);

    /* Set related bit */
    retVal = hwSetGlobal2RegField(dev,QD_REG_TRUNK_MASK_TBL, 11, 1, data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetHashTrunk
*
* DESCRIPTION:
*		Hash DA & SA for TrunkMask selection. Trunk load balancing is accomplished 
*		by using the frame's DA and SA fields to access one of eight Trunk Masks. 
*		When this bit is set to a one the hashed computed for address table 
*		lookups is used for the TrunkMask selection. When this bit is cleared to 
*		a zero the lower 3 bits of the frame's DA and SA are XOR'ed together to 
*		select the TrunkMask to use.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE to use lookup table, GT_FALSE to use XOR.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetHashTrunk
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysGetHashTrunk Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_TRUNK|DEV_REDUCED_TRUNK))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get related bit */
    retVal = hwGetGlobal2RegField(dev,QD_REG_TRUNK_MASK_TBL,11,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysSetTrunkRouting
*
* DESCRIPTION:
*		This function sets routing information for the given Trunk ID.
*
* INPUTS:
*		trunkId - Trunk ID.
*		trunkRoute - Trunk route bits. Bit 0 controls trunk routing for port 0,
*					bit 1 for port 1 , etc.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if trunkId > 0xF.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysSetTrunkRouting
(
	IN GT_QD_DEV	*dev,
	IN GT_U32  		trunkId,
	IN GT_U32		trunkRoute
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
	GT_U32			mask;
	GT_U32			maxTrunk;

    DBG_INFO(("gsysSetTrunkRouting Called.\n"));

	/* Check if Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_TRUNK))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	gtSemTake(dev,dev->tblRegsSem,OS_WAIT_FOREVER);

	/* Check if the register can be accessed. */
	do
	{
	    retVal = hwReadGlobal2Reg(dev,QD_REG_TRUNK_ROUTING,&data);
    	if(retVal != GT_OK)
	    {
    	    DBG_INFO(("Failed.\n"));
			gtSemGive(dev,dev->tblRegsSem);
        	return retVal;
	    }
	} while (data & 0x8000);

	if (IS_IN_DEV_GROUP(dev,DEV_8_TRUNKING))
		maxTrunk = 8;
	else
		maxTrunk = 16;

	if(trunkId >= maxTrunk)
	{
		DBG_INFO(("GT_BAD_PARAM (trunkId)\n"));
		gtSemGive(dev,dev->tblRegsSem);
		return GT_BAD_PARAM;
	}

	mask = (1 << dev->numOfPorts) - 1;

	if(trunkRoute > mask)
	{
		DBG_INFO(("GT_BAD_PARAM (trunkRoute)\n"));
		gtSemGive(dev,dev->tblRegsSem);
		return GT_BAD_PARAM;
	}

	mask = GT_LPORTVEC_2_PORTVEC(trunkRoute);

	data = 0x8000 | (trunkId << 11) | mask;

    /* Set related register */
    retVal = hwWriteGlobal2Reg(dev,QD_REG_TRUNK_ROUTING,data);

	gtSemGive(dev,dev->tblRegsSem);

    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetTrunkRouting
*
* DESCRIPTION:
*		This function retrieves routing information for the given Trunk ID.
*
* INPUTS:
*		trunkId - Trunk ID.
*
* OUTPUTS:
*		trunkRoute - Trunk route bits. Bit 0 controls trunk routing for port 0,
*					bit 1 for port 1 , etc.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if trunkId > 0xF.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetTrunkRouting
(
	IN  GT_QD_DEV	*dev,
	IN  GT_U32 		trunkId,
	OUT GT_U32		*trunkRoute
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
	GT_U32			mask;
	GT_U32			maxTrunk;

    DBG_INFO(("gsysGetTrunkRouting Called.\n"));

	/* Check if Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_TRUNK))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	gtSemTake(dev,dev->tblRegsSem,OS_WAIT_FOREVER);

	/* Check if the register can be accessed. */
	do
	{
	    retVal = hwReadGlobal2Reg(dev,QD_REG_TRUNK_ROUTING,&data);
    	if(retVal != GT_OK)
	    {
    	    DBG_INFO(("Failed.\n"));
			gtSemGive(dev,dev->tblRegsSem);
        	return retVal;
	    }
	} while (data & 0x8000);

	if (IS_IN_DEV_GROUP(dev,DEV_8_TRUNKING))
		maxTrunk = 8;
	else
		maxTrunk = 16;

	if(trunkId >= maxTrunk)
	{
		DBG_INFO(("GT_BAD_PARAM (trunkId)\n"));
		gtSemGive(dev,dev->tblRegsSem);
		return GT_BAD_PARAM;
	}

	data = trunkId << 11;

    retVal = hwWriteGlobal2Reg(dev,QD_REG_TRUNK_ROUTING,data);
   	if(retVal != GT_OK)
    {
   	    DBG_INFO(("Failed.\n"));
		gtSemGive(dev,dev->tblRegsSem);
       	return retVal;
    }

    retVal = hwReadGlobal2Reg(dev,QD_REG_TRUNK_ROUTING,&data);
	gtSemGive(dev,dev->tblRegsSem);
   	if(retVal != GT_OK)
    {
   	    DBG_INFO(("Failed.\n"));
       	return retVal;
    }

	mask = (1 << dev->maxPorts) - 1;

	*trunkRoute = GT_PORTVEC_2_LPORTVEC(data & mask);
			
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetRateLimitMode
*
* DESCRIPTION:
*		Ingress Rate Limiting can be either Priority based or Burst Size based.
*		This routine sets which mode to use.
*
* INPUTS:
*		mode - either GT_RATE_PRI_BASE or GT_RATE_BURST_BASE
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if invalid mode is used.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetRateLimitMode
(
	IN GT_QD_DEV	*dev,
	IN GT_INGRESS_RATE_MODE mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gsysSetRateLimitMode Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_BURST_RATE))
	{
		if (!IS_IN_DEV_GROUP(dev,DEV_NEW_FEATURE_IN_REV) || 
			((GT_DEVICE_REV)dev->revision < GT_REV_2))
	    {
    	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
			return GT_NOT_SUPPORTED;
	    }
	}

	switch (mode)
	{
		case GT_RATE_PRI_BASE:
			data = 0;
			break;
		case GT_RATE_BURST_BASE:
			data = 1;
			break;
		default:
	        DBG_INFO(("Not supported mode %i\n",mode));
			return GT_BAD_PARAM;
	}

    hwPort = 7;

    /* Set related bit */
    retVal = hwSetPortRegField(dev,hwPort, 0x1A, 15, 1, data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetRateLimitMode
*
* DESCRIPTION:
*		Ingress Rate Limiting can be either Priority based or Burst Size based.
*		This routine gets which mode is being used.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		mode - either GT_RATE_PRI_BASE or GT_RATE_BURST_BASE
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetRateLimitMode
(
	IN  GT_QD_DEV	*dev,
	OUT GT_INGRESS_RATE_MODE *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gsysGetRateLimitMode Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_BURST_RATE))
	{
		if (!IS_IN_DEV_GROUP(dev,DEV_NEW_FEATURE_IN_REV) || 
			((GT_DEVICE_REV)dev->revision < GT_REV_2))
	    {
    	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
			return GT_NOT_SUPPORTED;
	    }
	}

    hwPort = 7;
	data = 0;

    /* Get related bit */
    retVal = hwGetPortRegField(dev,hwPort, 0x1A, 15, 1, &data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	if (data == 0)
		*mode = GT_RATE_PRI_BASE;
	else
		*mode = GT_RATE_BURST_BASE;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetAgeInt
*
* DESCRIPTION:
*		Enable/Disable Age Refresh Interrupt. If CPU Directed Learning is being
*		used (gprtSetLockedPort), it may be desirable to know when an address is
*		still being used before it totally ages out of the switch. This can be 
*		accomplished by enabling Age Refresh Interrupt (or ATU Age Violation Int).
*		An ATU Age Violation looks identical to and reported the same as an ATU 
*		Miss Violation. The only difference is when this reported. Normal ATU Miss
*		Violation only occur if a new SA arrives at a LockedPort. The Age version 
*		of the ATU Miss Violation occurs if an SA arrives at a LockedPort, where
*		the address is contained in the ATU's database, but where its EntryState 
*		is less than 0x4 (i.e., it has aged more than 1/2 way).
*		GT_ATU_PROB Interrupt should be enabled for this interrupt to occur.
*		Refer to eventSetActive routine to enable GT_ATU_PROB.
*		
*
* INPUTS:
*		en - GT_TRUE, to enable,
*			 GT_FALSE, otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetAgeInt
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gsysSetAgeInt Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_AGE_INTERRUPT))
	{
		if (!IS_IN_DEV_GROUP(dev,DEV_NEW_FEATURE_IN_REV) || 
			((GT_DEVICE_REV)dev->revision < GT_REV_2))
	    {
    	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
			return GT_NOT_SUPPORTED;
	    }
	}

    BOOL_2_BIT(en, data);

    hwPort = 7;

    /* Set related bit */
    retVal = hwSetPortRegField(dev,hwPort, 0x1A, 14, 1, data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetAgeInt
*
* DESCRIPTION:
*		Get state of Age Refresh Interrupt mode. If CPU Directed Learning is being
*		used (gprtSetLockedPort), it may be desirable to know when an address is
*		still being used before it totally ages out of the switch. This can be 
*		accomplished by enabling Age Refresh Interrupt (or ATU Age Violation Int).
*		An ATU Age Violation looks identical to and reported the same as an ATU 
*		Miss Violation. The only difference is when this reported. Normal ATU Miss
*		Violation only occur if a new SA arrives at a LockedPort. The Age version 
*		of the ATU Miss Violation occurs if an SA arrives at a LockedPort, where
*		the address is contained in the ATU's database, but where its EntryState 
*		is less than 0x4 (i.e., it has aged more than 1/2 way).
*		GT_ATU_PROB Interrupt should be enabled for this interrupt to occur.
*		Refer to eventSetActive routine to enable GT_ATU_PROB.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE, if enabled,
*			 GT_FALSE, otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysGetAgeInt
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL		*en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gsysGetAgeInt Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_AGE_INTERRUPT))
	{
		if (!IS_IN_DEV_GROUP(dev,DEV_NEW_FEATURE_IN_REV) || 
			((GT_DEVICE_REV)dev->revision < GT_REV_2))
	    {
    	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
			return GT_NOT_SUPPORTED;
	    }
	}

    hwPort = 7;
	data = 0;

    /* Get related bit */
    retVal = hwGetPortRegField(dev,hwPort, 0x1A, 14, 1, &data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data, *en);

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetForceSnoopPri
*
* DESCRIPTION:
*		Force Snooping Priority. The priority on IGMP or MLD Snoop frames are
*		set to the SnoopPri value (gsysSetSnoopPri API) when Force Snooping
*       Priority is enabled. When it's disabled, the priority on these frames
*		is not modified.
*
* INPUTS:
*		en - GT_TRUE to use defined PRI bits, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetForceSnoopPri
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("gsysSetForceSnoopPri Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_SNOOP_PRI))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(en,data);

    /* Set related bit */
    retVal = hwSetGlobal2RegField(dev,QD_REG_PRIORITY_OVERRIDE, 7, 1, data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetForceSnoopPri
*
* DESCRIPTION:
*		Force Snooping Priority. The priority on IGMP or MLD Snoop frames are
*		set to the SnoopPri value (gsysSetSnoopPri API) when Force Snooping
*       Priority is enabled. When it's disabled, the priority on these frames
*		is not modified.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE to use defined PRI bits, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetForceSnoopPri
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysGetForceSnoopPri Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_SNOOP_PRI))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get related bit */
    retVal = hwGetGlobal2RegField(dev,QD_REG_PRIORITY_OVERRIDE,7,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysSetSnoopPri
*
* DESCRIPTION:
*		Snoop Priority. When ForceSnoopPri (gsysSetForceSnoopPri API) is enabled,
*       this priority is used as the egressing frame's PRI[2:0] bits on generated
*       Marvell Tag To_CPU Snoop frames and higher 2 bits of the priority are
*       used as the internal Queue Priority to use on IGMP/MLD snoop frames.
*
* INPUTS:
*		pri - PRI[2:0] bits (should be less than 8)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - If pri is not less than 8.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetSnoopPri
(
	IN GT_QD_DEV	*dev,
	IN GT_U16		pri
)
{
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gsysSetSnoopPri Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_SNOOP_PRI))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	if (pri > 0x7)
	{
        DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
	}

    /* Set related bit */
    retVal = hwSetGlobal2RegField(dev,QD_REG_PRIORITY_OVERRIDE, 4, 3, pri);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetSnoopPri
*
* DESCRIPTION:
*		Snoop Priority. When ForceSnoopPri (gsysSetForceSnoopPri API) is enabled,
*       this priority is used as the egressing frame's PRI[2:0] bits on generated
*       Marvell Tag To_CPU Snoop frames and higher 2 bits of the priority are
*       used as the internal Queue Priority to use on IGMP/MLD snoop frames.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		pri - PRI[2:0] bits (should be less than 8)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetSnoopPri
(
	IN  GT_QD_DEV	*dev,
	OUT GT_U16  	*pri
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    DBG_INFO(("gsysGetSnoopPri Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_SNOOP_PRI))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get related bit */
    retVal = hwGetGlobal2RegField(dev,QD_REG_PRIORITY_OVERRIDE,4,3,pri);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetForceARPPri
*
* DESCRIPTION:
*		Force ARP Priority. The priority on ARP frames are set to the ARPPri 
*       value (gsysSetARPPri API) when Force ARP Priority is enabled. When it's 
*       disabled, the priority on these frames is not modified.
*
* INPUTS:
*		en - GT_TRUE to use defined PRI bits, GT_FALSE otherwise.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetForceARPPri
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("gsysSetForceARPPri Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ARP_PRI))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(en,data);

    /* Set related bit */
    retVal = hwSetGlobal2RegField(dev,QD_REG_PRIORITY_OVERRIDE, 3, 1, data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetForceARPPri
*
* DESCRIPTION:
*		Force ARP Priority. The priority on ARP frames are set to the ARPPri 
*       value (gsysSetARPPri API) when Force ARP Priority is enabled. When it's 
*       disabled, the priority on these frames is not modified.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en - GT_TRUE to use defined PRI bits, GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetForceARPPri
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL  	*en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    DBG_INFO(("gsysGetForceARPPri Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ARP_PRI))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get related bit */
    retVal = hwGetGlobal2RegField(dev,QD_REG_PRIORITY_OVERRIDE,3,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysSetARPPri
*
* DESCRIPTION:
*		ARP Priority. When ForceARPPri (gsysSetForceARPPri API) is enabled,
*       this priority is used as the egressing frame's PRI[2:0] bits on generated
*       Marvell Tag To_CPU ARP frames and higher 2 bits of the priority are
*       used as the internal Queue Priority to use on ARP frames.
*
* INPUTS:
*		pri - PRI[2:0] bits (should be less than 8)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - If pri is not less than 8.
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
*******************************************************************************/
GT_STATUS gsysSetARPPri
(
	IN GT_QD_DEV	*dev,
	IN GT_U16		pri
)
{
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gsysSetARPPri Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ARP_PRI))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	if (pri > 0x7)
	{
        DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
	}

    /* Set related bit */
    retVal = hwSetGlobal2RegField(dev,QD_REG_PRIORITY_OVERRIDE, 0, 3, pri);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetARPPri
*
* DESCRIPTION:
*		ARP Priority. When ForceARPPri (gsysSetForceARPPri API) is enabled,
*       this priority is used as the egressing frame's PRI[2:0] bits on generated
*       Marvell Tag To_CPU ARP frames and higher 2 bits of the priority are
*       used as the internal Queue Priority to use on ARP frames.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		pri - PRI[2:0] bits (should be less than 8)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetARPPri
(
	IN  GT_QD_DEV	*dev,
	OUT GT_U16  	*pri
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    DBG_INFO(("gsysGetARPPri Called.\n"));

	/* Check if Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ARP_PRI))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get related bit */
    retVal = hwGetGlobal2RegField(dev,QD_REG_PRIORITY_OVERRIDE,0,3,pri);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetUsePortSchedule
*
* DESCRIPTION:
*       This routine sets per port scheduling mode
*
* INPUTS:
*       en - GT_TRUE enables per port scheduling, 
*			 GT_FALSE disable.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gsysSetUsePortSchedule
(
    IN  GT_QD_DEV *dev,
    IN  GT_BOOL   en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetWatchDog Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_PORT_SCHEDULE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(en,data);

    /* Set the UsePortSchedule bit.            */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,12,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysGetUsePortSchedule
*
* DESCRIPTION:
*       This routine gets per port scheduling mode
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE enables per port scheduling, 
*			 GT_FALSE disable.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gsysGetUsePortSchedule
(
    IN  GT_QD_DEV *dev,
    OUT GT_BOOL   *en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gsysSetWatchDog Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_PORT_SCHEDULE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the UsePortSchedule bit.            */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,12,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data, *en);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetOldHader
*
* DESCRIPTION:
*       This routine sets Egress Old Header.
*		When this feature is enabled and frames are egressed with a Marvell Header, 
*		the format of the Header is slightly modified to be backwards compatible 
*		with previous devices that used the original Header. Specifically, bit 3
*		of the Header's 2nd octet is cleared to a zero such that only FPri[2:1]
*		is available in the Header.
*
* INPUTS:
*       en - GT_TRUE to enable Old Header Mode,
*			 GT_FALSE to disable
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gsysSetOldHader
(
    IN  GT_QD_DEV *dev,
    IN  GT_BOOL   en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("gsysSetArpQPri Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_OLD_HEADER))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(en,data);

    /* Set the OldHader bit.            */
    retVal = hwSetGlobalRegField(dev,QD_REG_MANGEMENT_CONTROL,5,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysGetOldHader
*
* DESCRIPTION:
*       This routine gets Egress Old Header.
*		When this feature is enabled and frames are egressed with a Marvell Header, 
*		the format of the Header is slightly modified to be backwards compatible 
*		with previous devices that used the original Header. Specifically, bit 3
*		of the Header's 2nd octet is cleared to a zero such that only FPri[2:1]
*		is available in the Header.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE to enable Old Header Mode,
*			 GT_FALSE to disable
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gsysGetOldHader
(
    IN  GT_QD_DEV *dev,
    OUT GT_BOOL   *en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("gsysGetArpQPri Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_OLD_HEADER))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the OldHader bit.            */
    retVal = hwGetGlobalRegField(dev,QD_REG_MANGEMENT_CONTROL,5,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data, *en);

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetRecursiveStrippingDisable
*
* DESCRIPTION:
*       This routine determines if recursive tag stripping feature needs to be
*		disabled.
*
* INPUTS:
*       en - GT_TRUE to disable Recursive Tag Stripping,
*			 GT_FALSE to enable
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gsysSetRecursiveStrippingDisable
(
    IN  GT_QD_DEV *dev,
    IN  GT_BOOL   en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("gsysSetRecursiveStrippingDisable Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_RECURSIVE_TAG_STRIP))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(en,data);

    /* Set the RecursiveStrippingDisable bit.            */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL2,15,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysGetRecursiveStrippingDisable
*
* DESCRIPTION:
*       This routine checks if recursive tag stripping feature is disabled.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       en - GT_TRUE, if Recursive Tag Stripping is disabled,
*			 GT_FALSE, otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gsysGetRecursiveStrippingDisable
(
    IN  GT_QD_DEV *dev,
    OUT GT_BOOL   *en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("gsysGetRecursiveStrippingDisable Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_RECURSIVE_TAG_STRIP))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the RecursiveStrippingDisable bit.            */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL2,15,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data, *en);

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gsysSetCPUPort
*
* DESCRIPTION:
*       This routine sets CPU Port where Rsvd2Cpu frames and IGMP/MLD Snooped 
*		frames are destined.
*
* INPUTS:
*       cpuPort - CPU Port
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gsysSetCPUPort
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  cpuPort
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gsysSetCPUPort Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(cpuPort);

	if (!IS_IN_DEV_GROUP(dev,DEV_CPU_PORT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	if (cpuPort >= dev->numOfPorts)
	{
		return GT_BAD_PARAM;
	}

    /* Set the CPU Port.            */
    retVal = hwSetGlobalRegField(dev,QD_REG_MANGEMENT_CONTROL,0,3,(GT_U16)hwPort);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gsysGetCPUPort
*
* DESCRIPTION:
*       This routine gets CPU Port where Rsvd2Cpu frames and IGMP/MLD Snooped 
*		frames are destined.
*
* INPUTS:
*       cpuPort - CPU Port
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gsysGetCPUPort
(
    IN  GT_QD_DEV *dev,
    OUT GT_LPORT  *cpuPort
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          hwPort;         /* the physical port number     */

    DBG_INFO(("gsysGetCPUPort Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_CPU_PORT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the CPU Port.            */
    retVal = hwGetGlobalRegField(dev,QD_REG_MANGEMENT_CONTROL,0,3,&hwPort);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    /* translate hardware port to LPORT */
    *cpuPort = (GT_LPORT)GT_PORT_2_LPORT(hwPort);

    DBG_INFO(("OK.\n"));
    return GT_OK;
}
