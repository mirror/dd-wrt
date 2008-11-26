#include <Copyright.h>

/********************************************************************************
* gtEvents.c
*
* DESCRIPTION:
*       API definitions for system interrupt events handling.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*******************************************************************************/

#include <msApi.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>

/*******************************************************************************
* eventSetActive
*
* DESCRIPTION:
*       This routine enables/disables the receive of an hardware driven event.
*
* INPUTS:
*       eventType - the event type. any combination of the folowing:
*       	GT_STATS_DONE, GT_VTU_PROB, GT_VTU_DONE, GT_ATU_FULL(or GT_ATU_PROB),
*       	GT_ATU_DONE, GT_PHY_INTERRUPT, GT_EE_INTERRUPT, GT_DEVICE_INT,
*			and GT_AVB_INTERRUPT
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       Each switch device has its own set of event Types. Please refer to the
*		device datasheet for the list of event types that the device supports.
*
*******************************************************************************/
GT_STATUS eventSetActive
(
    IN GT_QD_DEV *dev,
    IN GT_U32 	 eventType
)
{
    GT_STATUS   retVal;
    GT_U16 	data, len;
	GT_U16	intMask;

    DBG_INFO(("eventSetActive Called.\n"));

	data = (GT_U16) eventType;
	len = 9;

	if ((IS_IN_DEV_GROUP(dev,DEV_EXTERNAL_PHY_ONLY)) ||
		(IS_IN_DEV_GROUP(dev,DEV_DEV_PHY_INTERRUPT)))
    {
		intMask = GT_NO_INTERNAL_PHY_INT_MASK;
    }
	else
	{
		intMask = GT_INT_MASK;
	}

	if (!IS_IN_DEV_GROUP(dev,DEV_AVB_INTERRUPT))
	{
		intMask &= ~GT_AVB_INT;
		len = 8;
	}

	if (!IS_IN_DEV_GROUP(dev,DEV_DEVICE_INTERRUPT))
	{
		intMask &= ~GT_DEVICE_INT;
		len = 7;
	}


	if(data & ~intMask)
	{
	    DBG_INFO(("Invalid event type.\n"));
		return GT_FAIL;
	}

    /* Set the IntEn bit.               */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,0,len,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* eventGetIntStatus
*
* DESCRIPTION:
*       This routine reads an hardware driven event status.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       intCause -  It provides the source of interrupt of the following:
*       GT_STATS_DONE, GT_VTU_PROB, GT_VTU_DONE, GT_ATU_FULL,
*       		GT_ATU_DONE, GT_PHY_INTERRUPT, GT_EE_INTERRUPT, GT_DEVICE_INT,
*				and GT_AVB_INTERRUPT
*		For Gigabit Switch, GT_ATU_FULL is replaced with GT_ATU_PROB and
*		if there is no internal phy, GT_PHY_INTERRUPT is not supported.
*
* RETURNS:
*       GT_OK   - read success.
*       GT_FAIL - otherwise
*
* COMMENTS:
*       Each switch device has its own set of event Types. Please refer to the
*		device datasheet for the list of event types that the device supports.
*
*******************************************************************************/
GT_STATUS eventGetIntStatus
(
    IN GT_QD_DEV *dev,
    OUT GT_U16   *intCause
)
{
    GT_STATUS       retVal;         /* Function calls return value.     */
    GT_U16 		len;

	if (IS_IN_DEV_GROUP(dev,DEV_AVB_INTERRUPT))
		len = 9;
	else if (IS_IN_DEV_GROUP(dev,DEV_DEVICE_INTERRUPT))
		len = 8;
	else
		len = 7;

    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_STATUS,0,len,intCause);

    return retVal;
}


/*******************************************************************************
* gvtuGetIntStatus
*
* DESCRIPTION:
* Check to see if a specific type of VTU interrupt occured
*
* INPUTS:
*       	intType - the type of interrupt which causes an interrupt.
*			any combination of
*			GT_MEMEBER_VIOLATION,
*			GT_MISS_VIOLATION,
*			GT_FULL_VIOLATION
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* 	FULL_VIOLATION is not supported by all switch devices.
*	Please refer to the device datasheet.
*
*******************************************************************************/
GT_STATUS gvtuGetIntStatus
(
    IN GT_QD_DEV          *dev,
    OUT GT_VTU_INT_STATUS *vtuIntStatus
)
{
    GT_STATUS       retVal;

    DBG_INFO(("gvtuGetIntStatus Called.\n"));

    /* check if device supports this feature */
    if((IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK )
      return GT_FAIL;

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
	    retVal = gvtuGetViolation2(dev,vtuIntStatus);
	}
	else if (IS_IN_DEV_GROUP(dev,DEV_ENHANCED_FE_SWITCH))
	{
	    retVal = gvtuGetViolation3(dev,vtuIntStatus);
	}
	else
	{
	    retVal = gvtuGetViolation(dev,vtuIntStatus);
	}

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
* gatuGetIntStatus
*
* DESCRIPTION:
* Check to see if a specific type of ATU interrupt occured
*
* INPUTS:
*       	intType - the type of interrupt which causes an interrupt.
*			any combination of
*			GT_AGE_OUT_VIOLATION,
*			GT_AGE_VIOLATION,
*			GT_MEMEBER_VIOLATION,
*			GT_MISS_VIOLATION,
*			GT_FULL_VIOLATION
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gatuGetIntStatus
(
    IN GT_QD_DEV          *dev,
    OUT GT_ATU_INT_STATUS *atuIntStatus
)
{
    GT_STATUS       retVal;

    DBG_INFO(("gatuGetIntStatus Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH|DEV_ENHANCED_FE_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    retVal = gatuGetViolation(dev,atuIntStatus);
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
* geventGetDevIntStatus
*
* DESCRIPTION:
* 		Check to see which device interrupts (WatchDog, JamLimit, Duplex Mismatch,
*		SERDES Link Int, and Phy Int) have occurred.
*
* INPUTS:
*       intType - the type of interrupt which causes an interrupt.
*				  any combination of
*					GT_DEV_INT_WATCHDOG,
*					GT_DEV_INT_JAMLIMIT,
*					GT_DEV_INT_DUPLEX_MISMATCH,
*					GT_DEV_INT_SERDES_LINK
*					GT_DEV_INT_PHY
*		port	- logical port where GT_DEV_INT_DUPLEX_MISMATCH occurred.
*				  valid only if GT_DEV_INT_DUPLEX_MISMATCH is set in intType.
*		linkInt - SERDES port list where GT_DEV_INT_SERDES_LINK interrupt is
*				  asserted. It's in vector format, Bit 10 is for port 10,
*				  Bit 9 is for port 9, etc.
*				  valid only if GT_DEV_INT_SERDES_LINK bit is set in intType.
*				  These bits are only valid of the port that is in 1000Base-X mode.
*		phyInt  - port list where GT_DEV_INT_PHY interrupt is asserted.
*				  It's in vector format, Bit 0 is for port 0, Bit 1 is for port 1, etc.
*				  valid only if GT_DEV_INT_PHY bit is set in intType.
*
* OUTPUTS:
* 		None.
*
* RETURNS:
* 		GT_OK - on success
* 		GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS geventGetDevIntStatus
(
    IN  GT_QD_DEV 			*dev,
    OUT GT_DEV_INT_STATUS	*devIntStatus
)
{
    GT_STATUS       retVal;
	GT_U16			data, hwPort;

    DBG_INFO(("geventGetDevIntStatus Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_DEVICE_INTERRUPT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	devIntStatus->devIntCause = 0;

    retVal = hwReadGlobal2Reg(dev,QD_REG_DEVINT_SOURCE,&data);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
		return retVal;
	}

	/* check SERDES Link Int and Phy Int, if applicable */
	if (IS_IN_DEV_GROUP(dev,DEV_DEVICE_INT_TYPE1))
	{
		/* check SERDES Link Int */
		if (data & (0x7 << 8))
		{
			devIntStatus->devIntCause |= GT_DEV_INT_SERDES_LINK;
			devIntStatus->linkInt = GT_PORTVEC_2_LPORTVEC((data & (7<<8)));
		}
	}
	else	/* DEV_DEVICE_INT_TYPE2 */
	{
		if (data & (0x3 << 11))
		{
			devIntStatus->devIntCause |= GT_DEV_INT_SERDES_LINK;
			devIntStatus->linkInt = GT_PORTVEC_2_LPORTVEC((data & (0x3 << 11)) >> 7);
		}

		if (data & 0x1F)
		{
			devIntStatus->devIntCause |= GT_DEV_INT_PHY;
			devIntStatus->phyInt = GT_PORTVEC_2_LPORTVEC((data & 0x1F));
		}
	}

	if (data & QD_DEV_INT_DUPLEX_MISMATCH)
	{
		devIntStatus->devIntCause |= GT_DEV_INT_DUPLEX_MISMATCH;

		/* read port that causes the interrupt */
	    retVal = hwGetGlobal2RegField(dev, QD_REG_WD_CONTROL, 12, 4, &hwPort);
	    if(retVal != GT_OK)
		{
	        DBG_INFO(("Failed.\n"));
			return retVal;
		}

		/* re-arm the interrupt event */
	    retVal = hwSetGlobal2RegField(dev, QD_REG_WD_CONTROL, 12, 4, 0xF);
	    if(retVal != GT_OK)
		{
	        DBG_INFO(("Failed.\n"));
			return retVal;
		}

		devIntStatus->port = GT_PORT_2_LPORT(hwPort);
	}

	if (data & QD_DEV_INT_WATCHDOG)
	{
		devIntStatus->devIntCause |= GT_DEV_INT_WATCHDOG;
	}

	if (data & QD_DEV_INT_JAMLIMIT)
	{
		devIntStatus->devIntCause |= GT_DEV_INT_JAMLIMIT;
	}

    return retVal;
}


/*******************************************************************************
* geventSetAgeIntEn
*
* DESCRIPTION:
*		This routine enables/disables Age Interrupt for a port.
*		When it's enabled, ATU Age Violation interrupts from this port are enabled.
*		An Age Violation will occur anytime a port is Locked(gprtSetLockedPort)
*		and the ingressing frame's SA is contained in the ATU as a non-Static
*		entry with a EntryState less than 0x4.
*
* INPUTS:
*		port - the logical port number
*		mode - GT_TRUE to enable Age Interrupt,
*			   GT_FALUSE to disable
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
*
*******************************************************************************/
GT_STATUS geventSetAgeIntEn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("geventSetAgeIntEn Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PORT_BASED_AGE_INT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set Age Interrupt Enable Mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_ASSOCIATION,11,1,data);

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
* geventGetAgeIntEn
*
* DESCRIPTION:
*		This routine gets Age Interrupt Enable for the port.
*		When it's enabled, ATU Age Violation interrupts from this port are enabled.
*		An Age Violation will occur anytime a port is Locked(gprtSetLockedPort)
*		and the ingressing frame's SA is contained in the ATU as a non-Static
*		entry with a EntryState less than 0x4.
*
* INPUTS:
*		port - the logical port number
*		mode - GT_TRUE to enable Age Interrupt,
*			   GT_FALUSE to disable
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
*
*******************************************************************************/
GT_STATUS geventGetAgeIntEn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("geventGetAgeIntEn Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PORT_BASED_AGE_INT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get Age Interrupt Enable Mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_ASSOCIATION,11,1,&data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    BIT_2_BOOL(data, *mode);

    return retVal;
}


/*******************************************************************************
* geventSetAgeOutIntEn
*
* DESCRIPTION:
*		Interrupt on Age Out. When aging is enabled, all non-static address
*		entries in the ATU's address database are periodically aged.
*		When this feature is set to GT_TRUE and an entry associated with this
*		port is aged out, an AgeOutViolation will be captured for that entry.
*
* INPUTS:
*		port - the logical port number
*		mode - GT_TRUE to enable Age Out Interrupt,
*			   GT_FALUSE to disable
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
*
*******************************************************************************/
GT_STATUS geventSetAgeOutIntEn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("geventSetAgeOutIntEn Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_AGE_OUT_INT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set Age Out Interrupt Enable Mode. */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_ASSOCIATION,14,1,data);

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
* geventGetAgeOutIntEn
*
* DESCRIPTION:
*		Interrupt on Age Out. When aging is enabled, all non-static address
*		entries in the ATU's address database are periodically aged.
*		When this feature is set to GT_TRUE and an entry associated with this
*		port is aged out, an AgeOutViolation will be captured for that entry.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		mode - GT_TRUE, if Age Out Interrupt is enabled
*			   GT_FALUSE, otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS geventGetAgeOutIntEn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("geventGetAgeOutIntEn Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_AGE_OUT_INT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get Age Out Interrupt Enable Mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_ASSOCIATION,14,1,&data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    BIT_2_BOOL(data, *mode);

    return retVal;
}


/*******************************************************************************
* geventSetOverLimitInt
*
* DESCRIPTION:
*		This routine enables/disables Over Limit Interrupt for a port.
*		If it's enabled, an ATU Miss violation will be generated when port auto
*		learn reached the limit(refer to gfdbGetPortAtuLimitReached API).
*
* INPUTS:
*		port - the logical port number
*		mode - GT_TRUE to enable Over Limit Interrupt,
*			   GT_FALUSE to disable
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
*
*******************************************************************************/
GT_STATUS geventSetOverLimitInt
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("geventSetOverLimitInt Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_ATU_LIMIT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set Over Limit Interrupt Enable Mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_ATU_CONTROL, 13, 1, data);

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
* geventGetOverLimitInt
*
* DESCRIPTION:
*		This routine enables/disables Over Limit Interrupt for a port.
*		If it's enabled, an ATU Miss violation will be generated when port auto
*		learn reached the limit(refer to gfdbSetPortAtuLearnLimit API).
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		mode - GT_TRUE to enable Over Limit Interrupt,
*			   GT_FALUSE to disable
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS geventGetOverLimitInt
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("geventGetOverLimitInt Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_ATU_LIMIT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Set Over Limit Interrupt Enable Mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_ATU_CONTROL, 13, 1, &data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    BIT_2_BOOL(data, *mode);

    return retVal;
}

/*******************************************************************************
* geventGetPortAtuLimitReached
*
* DESCRIPTION:
*       This routine checks if learn limit has been reached.
*		When it reached, the port can no longer auto learn any more MAC addresses
*		because the address learn limit set on this port has been reached.
*
* INPUTS:
*       port  - logical port number
*
* OUTPUTS:
*       limit - GT_TRUE, if limit has been reached
*			    GT_FALSE, otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*
*******************************************************************************/
GT_STATUS geventGetPortAtuLimitReached
(
    IN  GT_QD_DEV 	*dev,
    IN  GT_LPORT  	port,
    IN  GT_BOOL   	*limit
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("geventGetPortAtuLimitReached Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* Check device if this feature is supported. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ATU_LIMIT))
    {
		return GT_NOT_SUPPORTED;
    }

    /* Get the LimitReached bit. */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_ATU_CONTROL, 14, 1, &data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data, *limit);

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* eventSetDevInt
*
* DESCRIPTION:
*		Device Interrupt.
*		The following device interrupts are supported:
*			GT_DEV_INT_WATCHDOG	-
*				WatchDog event interrupt (WatchDog event can be configured with
*				gwdSetEvent API)
*			GT_DEV_INT_JAMLIMIT	-
*				any of the ports detect an Ingress Jam Limit violation
*				(see gprtSetPauseLimitIn API)
*			GT_DEV_INT_DUPLEX_MISMATCH -
*				any of the ports detect a duplex mismatch (i.e., the local port is
*				in half duplex mode while the link partner is in full duplex mode)
*			GT_DEV_INT_SERDES_LINK -
*				SERDES link change interrupt.
*				An interrupt occurs when a SERDES port changes link status
*				(link up or link down)
*			GT_DEV_INT_PHY - Phy interrupt.
*
*		If any of the above events is enabled, GT_DEVICE_INT interrupt will
*		be asserted by the enabled event when GT_DEV_INT is enabled with
*		eventSetActive API.
*
* INPUTS:
*		devInt - GT_DEV_INT
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
*
*******************************************************************************/
GT_STATUS eventSetDevInt
(
	IN  GT_QD_DEV	*dev,
	IN  GT_DEV_EVENT    *devInt
)
{
	GT_U16          data, event;
	GT_U16			serdesMask, phyMask, mask;
	GT_U32			pList;
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("eventSetDevInt Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_DEVICE_INTERRUPT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	event = devInt->event;

	if (IS_IN_DEV_GROUP(dev,DEV_DEVICE_INT_TYPE1))
	{
		serdesMask = mask = 7 << 8;	/* SERDES Port List */
		phyMask = 0;
	}
	else
	{
		serdesMask = mask = 3 << 11;	/* SERDES Port List */
		mask |= 0x1F;	/* Phy list */
		phyMask = 0x1F;
	}
	mask |= QD_DEV_INT_WATCHDOG | QD_DEV_INT_JAMLIMIT | QD_DEV_INT_DUPLEX_MISMATCH;

	data = 0;

 	if (event & GT_DEV_INT_SERDES_LINK)
 	{
		/* check for valid SERDES Port List */
		if (IS_IN_DEV_GROUP(dev,DEV_DEVICE_INT_TYPE1))
		{
			pList = GT_LPORTVEC_2_PORTVEC(devInt->portList);
			if ((GT_U16)pList & (~serdesMask))
			{
		        DBG_INFO(("GT_BAD_PARAM portList\n"));
				return GT_BAD_PARAM;
			}
			data = (GT_U16)pList;
		}
		else
		{
			pList = GT_LPORTVEC_2_PORTVEC(devInt->portList);
			pList <<= 7;
			if ((GT_U16)pList & (~serdesMask))
			{
		        DBG_INFO(("GT_BAD_PARAM portList\n"));
				return GT_BAD_PARAM;
			}
			data = (GT_U16)pList;
		}
	}

	if (event & GT_DEV_INT_PHY)
	{
		/* check for valid Phy List */
		if (IS_IN_DEV_GROUP(dev,DEV_DEVICE_INT_TYPE1))
 		{
	        DBG_INFO(("GT_BAD_PARAM: PHY Int not supported.\n"));
 			return GT_BAD_PARAM;
 		}
		else
		{
			pList = GT_LPORTVEC_2_PORTVEC(devInt->phyList);
			if ((GT_U16)pList & (~phyMask))
			{
		        DBG_INFO(("GT_BAD_PARAM phyList\n"));
				return GT_BAD_PARAM;
			}

			data |= (GT_U16)pList;
		}
 	}

	if (event & GT_DEV_INT_WATCHDOG)
	{
		data |= QD_DEV_INT_WATCHDOG;
	}

	if (event & GT_DEV_INT_JAMLIMIT)
	{
		data |= QD_DEV_INT_JAMLIMIT;
	}

	if (event & GT_DEV_INT_DUPLEX_MISMATCH)
	{
		data |= QD_DEV_INT_DUPLEX_MISMATCH;
	}

	if (data & (~mask))
	{
        DBG_INFO(("GT_BAD_PARAM portList\n"));
		return GT_BAD_PARAM;
	}

	if (data & GT_DEV_INT_DUPLEX_MISMATCH)
	{
	    retVal = hwSetGlobal2RegField(dev, QD_REG_WD_CONTROL, 12, 4, 0xF);
	    if(retVal != GT_OK)
		{
	        DBG_INFO(("Failed.\n"));
			return retVal;
		}
	}

    /* Set the related bit. */
    retVal = hwSetGlobal2RegBits(dev,QD_REG_DEVINT_MASK, mask, data);

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
* gwdSetEvent
*
* DESCRIPTION:
*		Watch Dog Event.
*		The following Watch Dog events are supported:
*			GT_WD_QC  - Queue Controller Watch Dog enable.
*						When enabled, the QC's watch dog circuit checks for link
*						list errors and any errors found in the QC.
*			GT_WD_EGRESS - Egress Watch Dog enable.
*						When enabled, each port's egress circuit checks for problems
*						between the port and the Queue Controller.
*			GT_WD_FORCE - Force a Watch Dog event.
*
*		If any of the above events is enabled, GT_DEVICE_INT interrupt will
*		be asserted by the enabled WatchDog event when GT_DEV_INT_WATCHDOG is
*		enabled with eventSetDevActive API and GT_DEV_INT is enabled with
*		eventSetActive API.
*
* INPUTS:
*		wdEvent - Watch Dog Events
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
*
*******************************************************************************/
GT_STATUS gwdSetEvent
(
	IN  GT_QD_DEV	*dev,
	IN  GT_U32	    wdEvent
)
{
    GT_U16          data, mask;
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gwdSetEvent Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_WATCHDOG_EVENT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	mask = (1 << 5) | (1 << 3) | (1 << 2);
	data = 0;

	if (wdEvent & GT_WD_QC)
	{
		data |= (1 << 5);
	}

	if (wdEvent & GT_WD_EGRESS)
	{
		data |= (1 << 3);
	}

	if (wdEvent & GT_WD_FORCE)
	{
		data |= (1 << 2);
	}

    /* Set the related bit. */
    retVal = hwSetGlobal2RegBits(dev,QD_REG_WD_CONTROL, mask, data);

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
* gwdSetSWResetOnWD
*
* DESCRIPTION:
*		SWReset on Watch Dog Event.
*		When this feature is enabled, any enabled watch dog event (gwdSetEvent API)
*		will automatically reset the switch core's datapath just as if gsysSwReset
*		API is called.
*
*		The Watch Dog History (gwdGetHistory API) won't be cleared by this
*		automatic SWReset. This allows the user to know if any watch dog event
*		ever occurred even if the swich is configured to automatically recover
*		from a watch dog.
*
*		When this feature is disabled, enabled watch dog events will not cause a
*		SWReset.
*
* INPUTS:
*		en   - GT_TRUE to enable SWReset on WD
*			   GT_FALUSE to disable
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
*
*******************************************************************************/
GT_STATUS gwdSetSWResetOnWD
(
	IN  GT_QD_DEV	*dev,
	IN  GT_BOOL	    en
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gwdSetSWResetOnWD Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_WATCHDOG_EVENT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(en,data);

    /* Set the related bit. */
    retVal = hwSetGlobal2RegField(dev,QD_REG_WD_CONTROL, 0, 1, data);

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
* gwdGetSWResetOnWD
*
* DESCRIPTION:
*		SWReset on Watch Dog Event.
*		When this feature is enabled, any enabled watch dog event (gwdSetEvent API)
*		will automatically reset the switch core's datapath just as if gsysSwReset
*		API is called.
*
*		The Watch Dog History (gwdGetHistory API) won't be cleared by this
*		automatic SWReset. This allows the user to know if any watch dog event
*		ever occurred even if the swich is configured to automatically recover
*		from a watch dog.
*
*		When this feature is disabled, enabled watch dog events will not cause a
*		SWReset.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en   - GT_TRUE, if SWReset on WD is enabled
*			   GT_FALUSE, otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gwdGetSWResetOnWD
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL	    *en
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gwdSetSWResetOnWD Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_WATCHDOG_EVENT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the related bit. */
    retVal = hwGetGlobal2RegField(dev,QD_REG_WD_CONTROL, 0, 1, &data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    BIT_2_BOOL(data, *en);

    return retVal;
}


/*******************************************************************************
* gwdGetHistory
*
* DESCRIPTION:
*		This routine retrieves Watch Dog history. They are
*
*		wdEvent -
*			When it's set to GT_TRUE, some enabled Watch Dog event occurred.
*			The following events are possible:
*				QC WatchDog Event (GT_WD_QC)
*				Egress WatchDog Event (GT_WD_EGRESS)
*				Forced WatchDog Event (GT_WD_FORCE)
*		egressEvent -
*			If any port's egress logic detects an egress watch dog issue,
*			this field is set to GT_TRUE, regardless of the enabling GT_WD_EGRESS
*			event.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		history - GT_WD_EVENT_HISTORY structure
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gwdGetHistory
(
	IN  GT_QD_DEV			*dev,
	OUT GT_WD_EVENT_HISTORY	*history
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gwdSetSWResetOnWD Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_WATCHDOG_EVENT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the related bit. */
    retVal = hwReadGlobal2Reg(dev,QD_REG_WD_CONTROL,&data);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	    return retVal;
	}

	if (data & (1 << 4))
	{
		history->egressEvent = GT_TRUE;
	}
	else
	{
		history->egressEvent = GT_FALSE;
	}

	if (data & (1 << 1))
	{
		history->wdEvent = GT_TRUE;
	}
	else
	{
		history->wdEvent = GT_FALSE;
	}

	DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gwdSetRMUTimeOut
*
* DESCRIPTION:
*		Remote Management Timeout. When this bit is set to a one the Remote
*		Management Unit(RMU) will timeout on Wait on Bit commands. If the bit that
*		is being tested has not gone to the specified value after 1 sec. has elapsed
*		the Wait on Bit command will be terminated and the Response frame will be
*		sent without any further processing.
*
*		When this bit is cleared to a zero the Wait on Bit command will wait
*		until the bit that is being tested has changed to the specified value.
*
* INPUTS:
*		en   - GT_TRUE to enable RMU Timeout
*			   GT_FALUSE to disable
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
*
*******************************************************************************/
GT_STATUS gwdSetRMUTimeOut
(
	IN  GT_QD_DEV	*dev,
	IN  GT_BOOL	    en
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gwdSetRMUTimeOut Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_WATCHDOG_EVENT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(en,data);

    /* Set the related bit. */
    retVal = hwSetGlobal2RegField(dev,QD_REG_WD_CONTROL, 6, 1, data);

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
* gwdGetRMUTimeOut
*
* DESCRIPTION:
*		Remote Management Timeout. When this bit is set to a one the Remote
*		Management Unit(RMU) will timeout on Wait on Bit commands. If the bit that
*		is being tested has not gone to the specified value after 1 sec. has elapsed
*		the Wait on Bit command will be terminated and the Response frame will be
*		sent without any further processing.
*
*		When this bit is cleared to a zero the Wait on Bit command will wait
*		until the bit that is being tested has changed to the specified value.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		en   - GT_TRUE to enable RMU Timeout
*			   GT_FALUSE, otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gwdGetRMUTimeOut
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL	    *en
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gwdGetRMUTimeOut Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_WATCHDOG_EVENT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the related bit. */
    retVal = hwGetGlobal2RegField(dev,QD_REG_WD_CONTROL, 6, 1, &data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    BIT_2_BOOL(data, *en);

    return retVal;
}


/*******************************************************************************
* gwdGetEgressWDEvent
*
* DESCRIPTION:
*		If any port's egress logic detects an egress watch dog issue, this bit
*		will be set to a one, regardless of the setting of the GT_WD_EGRESS in
*		gwdSetEvent function.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		event - GT_TRUE, if egress logic has detected any egress watch dog issue
*			    GT_FALUSE, otherwise
*
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gwdGetEgressWDEvent
(
	IN  GT_QD_DEV		*dev,
	OUT GT_BOOL			*event
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gwdGetEgressWDEvent Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_WATCHDOG_EVENT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the related bit. */
    retVal = hwGetGlobal2RegField(dev,QD_REG_WD_CONTROL, 7, 1, &data);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    BIT_2_BOOL(data, *event);

    return retVal;
}


