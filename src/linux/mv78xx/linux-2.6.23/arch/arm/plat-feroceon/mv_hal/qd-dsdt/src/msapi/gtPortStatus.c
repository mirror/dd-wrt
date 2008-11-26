#include <Copyright.h>

/********************************************************************************
* gtPortCtrl.c
*
* DESCRIPTION:
*       API implementation for switch port status.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*******************************************************************************/

#include <msApi.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>
#include <gtDrvConfig.h>

typedef struct _GT_Px_MODE
{
	GT_BOOL miiEn;
	GT_BOOL portMode;
	GT_BOOL phyMode;
	GT_PORT_SPEED_MODE speed;
	GT_BOOL duplex;
} GT_Px_MODE;

/*******************************************************************************
* procPx_Mode
*
* DESCRIPTION:
*       This routine retrieves Px_MODE and analize it.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - Px_MODE structure
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS procPx_Mode
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_Px_MODE   *mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("procPx_Mode Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,7,5,&data);
	if (retVal != GT_OK)
		return retVal;

	if(data & 0x1)
	{
		/* MII Interface Enabled. */
		mode->miiEn = GT_TRUE;		/* Mii Interface Enabled */

		switch(data >> 1)
		{
			case 0:
			case 1:
			case 4:
			case 5:
				mode->speed = PORT_SPEED_10_MBPS;
				mode->duplex = GT_FALSE;	/* half duplex */
				mode->portMode = GT_FALSE;	/* not standard Mii, either SNI or 200 Mii */
				mode->phyMode = GT_TRUE;	/* PHY Mode */
				break;
			case 2:
			case 6:
				mode->speed = PORT_SPEED_10_MBPS;
				mode->duplex = GT_TRUE;		/* full duplex */
				mode->portMode = GT_FALSE;	/* not standard Mii, either SNI or 200 Mii */
				mode->phyMode = GT_TRUE;	/* PHY Mode */
				break;
			case 3:
				mode->speed = PORT_SPEED_200_MBPS;
				mode->duplex = GT_TRUE;		/* full duplex */
				mode->portMode = GT_FALSE;	/* not standard Mii, either SNI or 200 Mii */
				mode->phyMode = GT_FALSE;	/* MAC Mode */
				break;
			case 7:
				mode->speed = PORT_SPEED_200_MBPS;
				mode->duplex = GT_TRUE;		/* full duplex */
				mode->portMode = GT_FALSE;	/* not standard Mii, either SNI or 200 Mii */
				mode->phyMode = GT_TRUE;	/* PHY Mode */
				break;
			case 8:
				mode->speed = PORT_SPEED_UNKNOWN;
				mode->duplex = GT_FALSE;	/* half duplex */
				mode->portMode = GT_TRUE;	/* Mii Mode */
				mode->phyMode = GT_FALSE;	/* MAC Mode */
				break;
			case 9:
				mode->speed = PORT_SPEED_UNKNOWN;
				mode->duplex = GT_FALSE;	/* half duplex */
				mode->portMode = GT_TRUE;	/* RMii Mode */
				mode->phyMode = GT_TRUE;	/* PHY Mode */
				break;
			case 10:
				mode->speed = PORT_SPEED_UNKNOWN;
				mode->duplex = GT_TRUE;		/* full duplex */
				mode->portMode = GT_TRUE;	/* Mii Mode */
				mode->phyMode = GT_FALSE;	/* MAC Mode */
				break;
			case 11:
				mode->speed = PORT_SPEED_UNKNOWN;
				mode->duplex = GT_TRUE;		/* full duplex */
				mode->portMode = GT_TRUE;	/* RMii Mode */
				mode->phyMode = GT_TRUE;	/* PHY Mode */
				break;
			case 12:
				mode->speed = PORT_SPEED_10_MBPS;
				mode->duplex = GT_FALSE;	/* half duplex */
				mode->portMode = GT_TRUE;	/* Mii Mode */
				mode->phyMode = GT_TRUE;	/* PHY Mode */
				break;
			case 13:
				mode->speed = PORT_SPEED_100_MBPS;
				mode->duplex = GT_FALSE;	/* half duplex */
				mode->portMode = GT_TRUE;	/* Mii Mode */
				mode->phyMode = GT_TRUE;	/* PHY Mode */
				break;
			case 14:
				mode->speed = PORT_SPEED_10_MBPS;
				mode->duplex = GT_TRUE;		/* full duplex */
				mode->portMode = GT_TRUE;	/* Mii Mode */
				mode->phyMode = GT_TRUE;	/* PHY Mode */
				break;
			case 15:
				mode->speed = PORT_SPEED_100_MBPS;
				mode->duplex = GT_TRUE;		/* full duplex */
				mode->portMode = GT_TRUE;	/* Mii Mode */
				mode->phyMode = GT_TRUE;	/* PHY Mode */
				break;
			default:
				return GT_FAIL;
		}
	}
	else
	{
		/* MII Interface Disabled. */
		mode->miiEn = GT_FALSE;

		switch((data >> 1) & 0x3)
		{
			case 0:
				mode->speed = PORT_SPEED_10_MBPS;
				mode->duplex = GT_FALSE;	/* half duplex */
				mode->portMode = GT_TRUE;	/* MII Mode */
				mode->phyMode = GT_TRUE;	/* PHY Mode */
				break;
			case 1:
				mode->speed = PORT_SPEED_100_MBPS;
				mode->duplex = GT_FALSE;	/* half duplex */
				mode->portMode = GT_TRUE;	/* MII Mode */
				mode->phyMode = GT_TRUE;	/* PHY Mode */
				break;
			case 2:
				mode->speed = PORT_SPEED_10_MBPS;
				mode->duplex = GT_TRUE;		/* full duplex */
				mode->portMode = GT_TRUE;	/* MII Mode */
				mode->phyMode = GT_TRUE;	/* PHY Mode */
				break;
			case 3:
				mode->speed = PORT_SPEED_100_MBPS;
				mode->duplex = GT_TRUE;		/* full duplex */
				mode->portMode = GT_TRUE;	/* MII Mode */
				mode->phyMode = GT_TRUE;	/* PHY Mode */
				break;
			default:
				return GT_FAIL;
		}
	}

    /* return */
    return GT_OK;
}


/*******************************************************************************
* gprtGetPartnerLinkPause
*
* DESCRIPTION:
*       This routine retrives the link partner pause state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - GT_TRUE for enable  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPartnerLinkPause
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetPartnerLinkPause Called.\n"));

	/* Gigabit Switch does not support this status. gprtGetPauseEn is supported instead. */
	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,15,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}
    /* return */
    return retVal;
}

/*******************************************************************************
* gprtGetPauseEn
*
* DESCRIPTION:
*		This routine retrives the link pause state.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE for enable or GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		If set MAC Pause (for Full Duplex flow control) is implemented in the
*		link partner and in MyPause
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPauseEn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetPauseEn Called.\n"));

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,15,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}
    /* return */
    return retVal;
}

/*******************************************************************************
* gprtGetSelfLinkPause
*
* DESCRIPTION:
*       This routine retrives the link pause state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - GT_TRUE for enable  or GT_FALSE otherwise
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
GT_STATUS gprtGetSelfLinkPause
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetSelfLinkPause Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,14,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}
    /* return */
    return retVal;
}

/*******************************************************************************
* gprtGetResolve
*
* DESCRIPTION:
*       This routine retrives the resolve state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - GT_TRUE for Done  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetResolve
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetResolve Called.\n"));

	/* Gigabit Switch does not support this status. */
	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,13,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}

/*******************************************************************************
* gprtGetHdFlow
*
* DESCRIPTION:
*		This routine retrives the half duplex flow control value.
*		If set, Half Duplex back pressure will be used on this port if this port
*		is in a half duplex mode.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE for enable or GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetHdFlow
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL 	*state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetHdFlow Called.\n"));

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,13,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}
    /* return */
    return retVal;
}

/*******************************************************************************
* gprtGetPHYDetect
*
* DESCRIPTION:
*		This routine retrives the information regarding PHY detection.
*		If set, An 802.3 PHY is attached to this port.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if connected or GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPHYDetect
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL 	*state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetPHYDetect Called.\n"));

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,12,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}
    /* return */
    return retVal;
}

/*******************************************************************************
* gprtSetPHYDetect
*
* DESCRIPTION:
*		This routine sets PHYDetect bit which make PPU change its polling.
*		PPU's pool routine uses these bits to determine which port's to poll
*		PHYs on for Link, Duplex, Speed, and Flow Control.
*
* INPUTS:
*		port - the logical port number.
*		state - GT_TRUE or GT_FALSE
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
*		This function should not be called if gsysGetPPUState returns
*		PPU_STATE_ACTIVE.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetPHYDetect
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	IN  GT_BOOL  	state
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetPHYDetect Called.\n"));

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Set the PHY Detect bit.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,12,1,(GT_U16)state);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}



/*******************************************************************************
* gprtGetLinkState
*
* DESCRIPTION:
*       This routine retrives the link state.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       state - GT_TRUE for Up  or GT_FALSE otherwise
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
GT_STATUS gprtGetLinkState
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
	GT_U8			bitNumber;

    DBG_INFO(("gprtGetLinkState Called.\n"));

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
		bitNumber = 11;
	}
	else
	{
		bitNumber = 12;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,bitNumber,1,&data);

    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}


/*******************************************************************************
* gprtGetPortMode
*
* DESCRIPTION:
*       This routine retrives the port mode.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for MII 10/100 or RMII 100,
*			   GT_FALSE for SNI 10 or MII 200
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPortMode
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
	GT_Px_MODE		pxMode;

    DBG_INFO(("gprtGetPortMode Called.\n"));

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	if (IS_IN_DEV_GROUP(dev,DEV_Px_MODE))
	{
		retVal = procPx_Mode(dev,port,&pxMode);
		if (retVal != GT_OK)
		{
	        DBG_INFO(("procPx_Mode return Fail\n"));
			return retVal;
		}
		*mode = pxMode.portMode;
		return GT_OK;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,11,1,&data);

    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *mode);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}



/*******************************************************************************
* gprtGetPhyMode
*
* DESCRIPTION:
*       This routine retrives the PHY mode.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for MII PHY Mode,
*			   GT_FALSE for MII MAC Mode
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPhyMode
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
	GT_Px_MODE		pxMode;

    DBG_INFO(("gprtGetPhyMode Called.\n"));

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	if (IS_IN_DEV_GROUP(dev,DEV_Px_MODE))
	{
		retVal = procPx_Mode(dev,port,&pxMode);
		if (retVal != GT_OK)
		{
	        DBG_INFO(("procPx_Mode return Fail\n"));
			return retVal;
		}
		*mode = pxMode.phyMode;
		return GT_OK;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,10,1,&data);

    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *mode);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}



/*******************************************************************************
* gprtGetDuplex
*
* DESCRIPTION:
*       This routine retrives the port duplex mode.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for Full  or GT_FALSE otherwise
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
GT_STATUS gprtGetDuplex
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
	GT_U8			bitNumber;
	GT_Px_MODE		pxMode;

    DBG_INFO(("gprtGetDuplex Called.\n"));

	if (IS_IN_DEV_GROUP(dev,DEV_Px_MODE))
	{
		retVal = procPx_Mode(dev,port,&pxMode);
		if (retVal != GT_OK)
		{
	        DBG_INFO(("procPx_Mode return Fail\n"));
			return retVal;
		}
		*mode = pxMode.duplex;
		return GT_OK;
	}

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
		bitNumber = 10;
	}
	else
	{
		bitNumber = 9;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,bitNumber,1,&data);

    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *mode);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}



/*******************************************************************************
* gprtGetSpeed
*
* DESCRIPTION:
*       This routine retrives the port speed.
*
* INPUTS:
*       speed - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for 100Mb/s  or GT_FALSE otherwise
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
GT_STATUS gprtGetSpeed
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *speed
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
	GT_Px_MODE		pxMode;

    DBG_INFO(("gprtGetSpeed Called.\n"));

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	if (IS_IN_DEV_GROUP(dev,DEV_Px_MODE))
	{
		retVal = procPx_Mode(dev,port,&pxMode);
		if (retVal != GT_OK)
		{
	        DBG_INFO(("procPx_Mode return Fail\n"));
			return retVal;
		}
		*speed = (pxMode.speed==PORT_SPEED_100_MBPS)?GT_TRUE:GT_FALSE;
		return GT_OK;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,8,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *speed);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}


/*******************************************************************************
* gprtGetSpeedMode
*
* DESCRIPTION:
*       This routine retrives the port speed.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_PORT_SPEED_MODE type.
*				(PORT_SPEED_1000_MBPS,PORT_SPEED_100_MBPS, PORT_SPEED_10_MBPS,
*				etc.)
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
GT_STATUS gprtGetSpeedMode
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_PORT_SPEED_MODE   *speed
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
	GT_Px_MODE		pxMode;

    DBG_INFO(("gprtGetSpeed Called.\n"));

	if (IS_IN_DEV_GROUP(dev,DEV_Px_MODE))
	{
		retVal = procPx_Mode(dev,port,&pxMode);
		if (retVal != GT_OK)
		{
	        DBG_INFO(("procPx_Mode return Fail\n"));
			return retVal;
		}
		*speed = pxMode.speed;
		return GT_OK;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
	    /* Get the force flow control bit.  */
    	retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,8,2,&data);
	}
	else
	{
    	retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,8,1,&data);
	}

	*speed = (GT_PORT_SPEED_MODE)data;

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}



/*******************************************************************************
* gprtSetDuplex
*
* DESCRIPTION:
*       This routine sets the duplex mode of MII/SNI/RMII ports.
*
* INPUTS:
*       port - 	the logical port number.
*				(for FullSail, it will be port 2, and for ClipperShip,
*				it could be either port 5 or port 6.)
*       mode -  GT_TRUE for Full Duplex,
*				GT_FALSE for Half Duplex.
*
* OUTPUTS: None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetDuplex
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    IN  GT_BOOL   mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetDuplex Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_MII_DUPLEX_CONFIG)) != GT_OK)
	{
		return retVal;
	}

	/* check if phy is not configurable. */
	if(IS_CONFIGURABLE_PHY(dev, hwPort))
	{
		/*
		 * phy is configurable. this function is not for the port where phy
		 * can be configured.
		 */
		return GT_NOT_SUPPORTED;
	}

    /* Set the duplex mode. */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,9,1,(GT_U16)mode);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}


/*******************************************************************************
* gprtGetHighErrorRate
*
* DESCRIPTION:
*		This routine retrives the PCS High Error Rate.
*		This routine returns GT_TRUE if the rate of invalid code groups seen by
*		PCS has exceeded 10 to the power of -11.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE or GT_FALSE
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetHighErrorRate
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetHighErrorRate Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	if (IS_IN_DEV_GROUP(dev,DEV_MGMII_STATUS))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the high error rate bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,6,1,&data);

    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}

/*******************************************************************************
* gprtGetMGMII
*
* DESCRIPTION:
*		SERDES Interface mode. When this bit is cleared to a zero and a PHY is
*		detected connected to this port, the SERDES interface between this port
*		and the PHY will be SGMII.  When this bit is set toa one and a PHY is
*		detected connected to this port, the SERDES interface between this port
*		and the PHY will be MGMII. When no PHY is detected on this port and the
*		SERDES interface is being used, it will be configured in 1000Base-X mode.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE or GT_FALSE
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetMGMII
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetMGMII Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_MGMII_STATUS))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the high error rate bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,6,1,&data);

    /* translate binary to BOOL  */
	if (IS_IN_DEV_GROUP(dev,DEV_MGMII_REVERSE_STATUS))
	{
    	BIT_2_BOOL_R(data, *state);
	}
	else
	{
    BIT_2_BOOL(data, *state);
	}

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}


/*******************************************************************************
* gprtSetMGMII
*
* DESCRIPTION:
*		SERDES Interface mode. When this bit is cleared to a zero and a PHY is
*		detected connected to this port, the SERDES interface between this port
*		and the PHY will be SGMII.  When this bit is set toa one and a PHY is
*		detected connected to this port, the SERDES interface between this port
*		and the PHY will be MGMII. When no PHY is detected on this port and the
*		SERDES interface is being used, it will be configured in 1000Base-X mode.
*
* INPUTS:
*		port - the logical port number.
*		state - GT_TRUE or GT_FALSE
*
* OUTPUTS:
*		None
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetMGMII
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	IN  GT_BOOL  	state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetMGMII Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_MGMII_STATUS))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (IS_IN_DEV_GROUP(dev,DEV_MGMII_REVERSE_STATUS))
	{
	    BOOL_2_BIT_R(state,data);
	}
	else
	{
    BOOL_2_BIT(state,data);
	}

    /* Get the high error rate bit.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,6,1,data);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}



/*******************************************************************************
* gprtGetTxPaused
*
* DESCRIPTION:
*		This routine retrives Transmit Pause state.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if Rx MAC receives a PAUSE frame with none-zero Puase Time
*				  GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetTxPaused
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetTxPaused Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_FC_STATUS))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the TxPaused bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,5,1,&data);

    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}


/*******************************************************************************
* gprtGetFlowCtrl
*
* DESCRIPTION:
*		This routine retrives Flow control state.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if Rx MAC determines that no more data should be
*					entering this port.
*				  GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetFlowCtrl
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetFlowCtrl Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_FC_STATUS))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the FlowCtrl bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,4,1,&data);

    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}


/*******************************************************************************
* gprtGetFdFlowDis
*
* DESCRIPTION:
*		This routine retrives the read time value of the Full Duplex Flow Disable.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if Full Duplex Flow Disable.
*	   		    GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetFdFlowDis
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetFdFlowDis Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_FC_DIS_STATUS))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the FdFlowDis bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,3,1,&data);

    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}

/*******************************************************************************
* gprtGetHdFlowDis
*
* DESCRIPTION:
*		This routine retrives the read time value of the Half Duplex Flow Disable.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if Half Duplex Flow Disable.
*	   		    GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetHdFlowDis
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetHdFlowDis Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_FC_DIS_STATUS))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the HdFlowDis bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,2,1,&data);

    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}

/*******************************************************************************
* gprtGetPxMode
*
* DESCRIPTION:
*		This routine retrives 4 bits of Px_MODE Configuration value.
*		If speed and duplex modes are forced, the returned mode value would be
*		different from the configuration pin values.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - Px_MODE configuration value
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPxMode
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_U32  	*mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetPxMode Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_Px_MODE))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the Px_Mode bits.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,8,4,&data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

	*mode = (GT_U32) data;

    /* return */
    return retVal;
}

/*******************************************************************************
* gprtGetMiiInterface
*
* DESCRIPTION:
*		This routine retrives Mii Interface Mode.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if Mii Interface is enabled,
*				  GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetMiiInterface
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetMiiInterface Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_Px_MODE))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the Mii bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,7,1,&data);

    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}

/*******************************************************************************
* gprtGetOutQSize
*
* DESCRIPTION:
*		This routine gets egress queue size counter value.
*		This counter reflects the current number of Egress buffers switched to
*		this port. This is the total number of buffers across all four priority
*		queues.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		count - egress queue size counter value
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetOutQSize
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_U16		*count
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetOutQSize Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_OUT_Q_SIZE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get OutQ_Size.            */
	if (IS_IN_DEV_GROUP(dev,DEV_OUT_Q_512))
	{
	    retVal = hwGetPortRegField(dev,hwPort, QD_REG_Q_COUNTER,7,9,count);
	}
	else
	{
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_Q_COUNTER,8,8,count);
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
* gprtGetBufHigh
*
* DESCRIPTION:
*		Output from QC telling the MAC that it should perform Flow Control.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		bufHigh - GT_TRUE, if Flow control required
*				  GT_FALSE, otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetBufHigh
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*bufHigh
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
	GT_U16			data;

    DBG_INFO(("gprtGetBufHigh Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FULL_Q_COUNTER))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get BufHigh.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_Q_COUNTER,6,1,&data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *bufHigh);

    return retVal;
}

/*******************************************************************************
* gprtGetFcEn
*
* DESCRIPTION:
*		Input into the QC telling it that Flow Control is enabled on this port.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		fcEn - GT_TRUE, if Flow control is enabled
*			   GT_FALSE, otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetFcEn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*fcEn
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
	GT_U16			data;

    DBG_INFO(("gprtGetFcEn Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FULL_Q_COUNTER))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get FcEn.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_Q_COUNTER,5,1,&data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *fcEn);

    return retVal;
}

/*******************************************************************************
* gprtGetRsvSize
*
* DESCRIPTION:
*		This routine gets Ingress reserved queue size counter.
*		This counter reflects the current number of reserved ingress buffers
*		assigned to this port.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		count - reserved ingress queue size counter value
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetRsvSize
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_U16		*count
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetRsvSize Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_OUT_Q_SIZE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get Rsv_Size.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_Q_COUNTER,0,5,count);

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
* gprtGetC_Duplex
*
* DESCRIPTION:
*		This routine retrives Port 9's duplex configuration mode determined
*		at reset.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if configured as Full duplex operation
*				  GT_FALSE otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		Return value is valid only if the given port is 9.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetC_Duplex
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetC_Duplex Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the C_Duplex bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,3,1,&data);

    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *state);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}

/*******************************************************************************
* gprtGetC_Mode
*
* DESCRIPTION:
*		This routine retrives port's interface type configuration mode
*		determined at reset.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - one of value in GT_PORT_CONFIG_MODE enum type
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		Return value is valid only if the given port is 9.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetC_Mode
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_PORT_CONFIG_MODE   *state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetC_Mode Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the C_Mode bits.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_STATUS,0,3,&data);

    /* translate binary to BOOL  */
    *state = (GT_PORT_CONFIG_MODE)data;

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

    /* return */
    return retVal;
}



