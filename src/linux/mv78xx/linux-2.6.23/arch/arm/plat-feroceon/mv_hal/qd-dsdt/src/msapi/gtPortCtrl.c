#include <Copyright.h>

/********************************************************************************
* gtPortCtrl.c
*
* DESCRIPTION:
*       API implementation for switch port control.
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
* gprtSetForceFc
*
* DESCRIPTION:
*       This routine set the force flow control state.
*
* INPUTS:
*       port  - the logical port number.
*       force - GT_TRUE for force flow control  or GT_FALSE otherwise
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
GT_STATUS gprtSetForceFc
(
    IN GT_QD_DEV  *dev,
    IN GT_LPORT   port,
    IN GT_BOOL    force
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_PORT_STP_STATE  state;

    DBG_INFO(("gprtSetForceFc Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if device allows to force a flowcontrol disabled */
	if (IS_IN_DEV_GROUP(dev,DEV_FC_WITH_VALUE))
	{
		if(force)
			data = 3;
		else
			data = 0;

		retVal = hwSetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,6,2,data);
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

	/* Port should be disabled before Set Force Flow Control bit */
	retVal = gstpGetPortState(dev,port, &state);
    if(retVal != GT_OK)
	{
	    DBG_INFO(("gstpGetPortState failed.\n"));
		return retVal;
	}

	retVal = gstpSetPortState(dev,port, GT_PORT_DISABLE);
    if(retVal != GT_OK)
	{
	    DBG_INFO(("gstpSetPortState failed.\n"));
		return retVal;
	}

    /* translate BOOL to binary */
    BOOL_2_BIT(force, data);

    /* Set the force flow control bit.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,15,1,data);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

	/* Restore original stp state. */
	if(gstpSetPortState(dev,port, state) != GT_OK)
	{
	    DBG_INFO(("gstpSetPortState failed.\n"));
		return GT_FAIL;
	}

    return retVal;
}



/*******************************************************************************
* gprtGetForceFc
*
* DESCRIPTION:
*       This routine get the force flow control state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       force - GT_TRUE for force flow control  or GT_FALSE otherwise
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
GT_STATUS gprtGetForceFc
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    OUT GT_BOOL    *force
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetForceFc Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if device allows to force a flowcontrol disabled */
	if (IS_IN_DEV_GROUP(dev,DEV_FC_WITH_VALUE))
	{
		retVal = hwGetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,6,2,&data);
		if(retVal != GT_OK)
		{
			DBG_INFO(("Failed.\n"));
		}
		else
		{
			DBG_INFO(("OK.\n"));
		}

		if(data & 0x1)
			*force = GT_TRUE;
		else
			*force = GT_FALSE;

		return retVal;
	}

    /* Get the force flow control bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,15,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *force);
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
* gprtSetUseCoreTag
*
* DESCRIPTION:
*		This routine set the UseCoreTag bit in Port Control Register.
*		When this bit is cleared to a zero, ingressing frames are considered
*		Tagged if the 16-bits following the frame's Source Address is 0x8100.
*		When this bit is set to a one, ingressing frames are considered Tagged
*		if the 16-bits following the frame's Source Address is equal to the
*		CoreTag register value.
*
* INPUTS:
*       port  - the logical port number.
*       force - GT_TRUE for force flow control  or GT_FALSE otherwise
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
GT_STATUS gprtSetUseCoreTag
(
    IN GT_QD_DEV  *dev,
    IN GT_LPORT   port,
    IN GT_BOOL    force
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetUseCoreTag Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if device allows to force a flowcontrol disabled */
	if (!IS_IN_DEV_GROUP(dev,DEV_CORE_TAG))
	{
		DBG_INFO(("GT_NOT_SUPPORTED.\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate BOOL to binary */
    BOOL_2_BIT(force, data);

    /* Set the UseCoreTag bit.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,15,1,data);
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
* gprtGetUseCoreTag
*
* DESCRIPTION:
*       This routine get the Use Core Tag state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       force - GT_TRUE for using core tag register  or GT_FALSE otherwise
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
GT_STATUS gprtGetUseCoreTag
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    OUT GT_BOOL    *force
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetUseCoreTag Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_CORE_TAG))
	{
		DBG_INFO(("GT_NOT_SUPPORTED.\n"));
		return GT_NOT_SUPPORTED;
	}

    /* Get the UseCoreTag bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,15,1,&data);
    /* translate binary to BOOL  */
    BIT_2_BOOL(data, *force);
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
* gprtSetTrailerMode
*
* DESCRIPTION:
*       This routine set the egress trailer mode.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for add trailer or GT_FALSE otherwise
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
GT_STATUS gprtSetTrailerMode
(
    IN GT_QD_DEV  *dev,
    IN GT_LPORT   port,
    IN GT_BOOL    mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetTrailerMode Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_TRAILER|DEV_TRAILER_P5|DEV_TRAILER_P4P5))
	{
        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
        return GT_NOT_SUPPORTED;
	}

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if(hwPort < 4)
	{
	    /* check if device supports this feature for this port */
		if (IS_IN_DEV_GROUP(dev,DEV_TRAILER_P5|DEV_TRAILER_P4P5))
		{
	        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
    	    return GT_NOT_SUPPORTED;
		}
	}
	else if(hwPort == 4)
	{
	    /* check if device supports this feature for this port*/
		if (IS_IN_DEV_GROUP(dev,DEV_TRAILER_P5))
		{
	        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
    	    return GT_NOT_SUPPORTED;
		}
	}

    /* Set the trailer mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,14,1,data);

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
* gprtGetTrailerMode
*
* DESCRIPTION:
*       This routine get the egress trailer mode.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for add trailer or GT_FALSE otherwise
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
GT_STATUS gprtGetTrailerMode
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    OUT GT_BOOL    *mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetTrailerMode Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_TRAILER|DEV_TRAILER_P5|DEV_TRAILER_P4P5))
	{
        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
        return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if(hwPort < 4)
	{
	    /* check if device supports this feature for this port */
		if (IS_IN_DEV_GROUP(dev,DEV_TRAILER_P5|DEV_TRAILER_P4P5))
		{
	        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
    	    return GT_NOT_SUPPORTED;
		}
	}
	else if(hwPort == 4)
	{
	    /* check if device supports this feature for this port */
		if (IS_IN_DEV_GROUP(dev,DEV_TRAILER_P5))
		{
	        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
    	    return GT_NOT_SUPPORTED;
		}
	}

    /* Get the Trailer mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,14,1,&data);
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

    return retVal;
}




/*******************************************************************************
* gprtSetIngressMode
*
* DESCRIPTION:
*       This routine set the ingress mode.
*
* INPUTS:
*       port - the logical port number.
*       mode - the ingress mode.
*
* OUTPUTS:
*       None.
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
GT_STATUS gprtSetIngressMode
(
    IN  GT_QD_DEV      *dev,
    IN GT_LPORT        port,
    IN GT_INGRESS_MODE mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetIngressMode Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* Gigabit Switch does not support this status. */
	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* check if device supports this feature */
    switch (mode)
    {
        case (GT_UNMODIFY_INGRESS):
            break;

        case (GT_TRAILER_INGRESS):
		    if(!(IS_IN_DEV_GROUP(dev,DEV_TRAILER|DEV_TRAILER_P5|DEV_TRAILER_P4P5)))
			{
			    DBG_INFO(("Given ingress mode is not supported by this device\n"));
				return GT_NOT_SUPPORTED;
			}
            break;

        case (GT_UNTAGGED_INGRESS):
		    if(!(IS_IN_DEV_GROUP(dev,DEV_TAGGING)))
			{
			    DBG_INFO(("Given ingress mode is not supported by this device\n"));
				return GT_NOT_SUPPORTED;
			}
            break;

        case (GT_CPUPORT_INGRESS):
		    if(!(IS_IN_DEV_GROUP(dev,DEV_IGMP_SNOOPING)))
			{
			    DBG_INFO(("Given ingress mode is not supported by this device\n"));
				return GT_NOT_SUPPORTED;
			}

			if(hwPort != GT_LPORT_2_PORT(dev->cpuPortNum))
			{
			    DBG_INFO(("Given ingress mode is supported by CPU port only\n"));
				return GT_NOT_SUPPORTED;
			}

            break;

        default:
            DBG_INFO(("Failed.\n"));
            return GT_FAIL;
    }

    /* Set the Ingress Mode.        */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,8,2,(GT_U16)mode);
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
* gprtGetIngressMode
*
* DESCRIPTION:
*       This routine get the ingress mode.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - the ingress mode.
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
GT_STATUS gprtGetIngressMode
(
    IN  GT_QD_DEV      *dev,
    IN  GT_LPORT        port,
    OUT GT_INGRESS_MODE *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetIngressMode Called.\n"));

	/* Gigabit Switch does not support this status. */
	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    /* Get the Ingress Mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL, 8, 2,&data);
    *mode = data;
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
* gprtSetMcRateLimit
*
* DESCRIPTION:
*       This routine set the port multicast rate limit.
*
* INPUTS:
*       port - the logical port number.
*       rate - GT_TRUE to Enable, GT_FALSE for otherwise.
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
GT_STATUS gprtSetMcRateLimit
(
    IN  GT_QD_DEV   *dev,
    IN GT_LPORT     port,
    IN GT_MC_RATE   rate
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetMcRateLimit Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* this feature only exits in 6051, 6052, and 6012. It is replace with
     * Rate Cotrol Register in the future products, starting from clippership
     */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_MC_RATE_PERCENT)) != GT_OK)
        return retVal;

    /* Set the multicast rate limit.    */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,2,2,(GT_U16)rate);
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
* gprtGetMcRateLimit
*
* DESCRIPTION:
*       This routine Get the port multicast rate limit.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       rate - GT_TRUE to Enable, GT_FALSE for otherwise.
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
GT_STATUS gprtGetMcRateLimit
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT    port,
    OUT GT_MC_RATE  *rate
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read data        */

    DBG_INFO(("gprtGetMcRateLimit Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* this feature only exits in 6051, 6052, and 6012. It is replace with
     * Rate Cotrol Register in the future products, starting from clippership
     */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_MC_RATE_PERCENT)) != GT_OK)
        return retVal;

    /* Get the multicast rate limit.    */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL, 2, 2,&data);
    *rate = data;
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


/* the following two APIs are added to support fullsail and clippership */

/*******************************************************************************
* gprtSetIGMPSnoop
*
* DESCRIPTION:
* 		This routine set the IGMP Snoop. When set to one and this port receives
*		IGMP frame, the frame is switched to the CPU port, overriding all other
*		switching decisions, with exception for CPU's Trailer.
*		CPU port is determined by the Ingress Mode bits. A port is considered
*		the CPU port if its Ingress Mode are either GT_TRAILER_INGRESS or
*		GT_CPUPORT_INGRESS.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for IGMP Snoop or GT_FALSE otherwise
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetIGMPSnoop
(
    IN  GT_QD_DEV   *dev,
    IN GT_LPORT     port,
    IN GT_BOOL      mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetIGMPSnoop Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_IGMP_SNOOPING)) != GT_OK)
      return retVal;

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set the IGMP Snooping mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,10,1, data);

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
* gprtGetIGMPSnoop
*
* DESCRIPTION:
*       This routine get the IGMP Snoop mode.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE: IGMP Snoop enabled
*	       GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetIGMPSnoop
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT    port,
    OUT GT_BOOL     *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetIGMPSnoop Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_IGMP_SNOOPING)) != GT_OK)
      return retVal;

    /* Get the Ingress Mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL, 10, 1, &data);

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

    return retVal;
}

/* the following two APIs are added to support clippership */

/*******************************************************************************
* gprtSetHeaderMode
*
* DESCRIPTION:
*       This routine set ingress and egress header mode of a switch port.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for header mode  or GT_FALSE otherwise
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetHeaderMode
(
    IN  GT_QD_DEV   *dev,
    IN GT_LPORT     port,
    IN GT_BOOL      mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetHeaderMode Called.\n"));

    /* only devices beyond quarterdeck (6052) has this feature */
    /* Fullsail (DEV_QD_88E6502) is an exception, and does not support this feature */
    if(IS_VALID_API_CALL(dev,port, DEV_HEADER|DEV_HEADER_P5|DEV_HEADER_P4P5) != GT_OK)
	{
        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
        return GT_NOT_SUPPORTED;
	}

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if(hwPort < 4)
	{
		if (IS_IN_DEV_GROUP(dev,DEV_HEADER_P5|DEV_HEADER_P4P5))
		{
	        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
    	    return GT_NOT_SUPPORTED;
		}
	}
	else if(hwPort == 4)
	{
		if (IS_IN_DEV_GROUP(dev,DEV_HEADER_P5))
		{
	        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
    	    return GT_NOT_SUPPORTED;
		}
	}

    /* Set the header mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,11,1, data);

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
* gprtGetHeaderMode
*
* DESCRIPTION:
*       This routine gets ingress and egress header mode of a switch port.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE: header mode enabled
*	       GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetHeaderMode
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT    port,
    OUT GT_BOOL     *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetHeaderMode Called.\n"));

    /* only devices beyond quarterdeck (6052) has this feature */
    /* Fullsail (DEV_QD_88E602) is an exception, and does not support this feature */
    if(IS_VALID_API_CALL(dev,port, DEV_HEADER|DEV_HEADER_P5|DEV_HEADER_P4P5) != GT_OK)
	{
        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
        return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if(hwPort < 4)
	{
		if (IS_IN_DEV_GROUP(dev,DEV_HEADER_P5|DEV_HEADER_P4P5))
		{
	        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
    	    return GT_NOT_SUPPORTED;
		}
	}
	else if(hwPort == 4)
	{
		if (IS_IN_DEV_GROUP(dev,DEV_HEADER_P5))
		{
	        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
    	    return GT_NOT_SUPPORTED;
		}
	}

    /* Get the Header Mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL, 11, 1, &data);

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

    return retVal;
}

/* the following four APIs are added to support Octane */

/*******************************************************************************
* gprtSetProtectedMode
*
* DESCRIPTION:
*       This routine set protected mode of a switch port.
*		When this mode is set to GT_TRUE, frames are allowed to egress port
*		defined by the 802.1Q VLAN membership for the frame's VID 'AND'
*		by the port's VLANTable if 802.1Q is enabled on the port. Both must
*		allow the frame to Egress.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for protected mode or GT_FALSE otherwise
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetProtectedMode
(
    IN  GT_QD_DEV   *dev,
    IN GT_LPORT     port,
    IN GT_BOOL      mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetProtectedMode Called.\n"));

	/* Check if this feature is supported */
    if(IS_VALID_API_CALL(dev,port, DEV_PORT_SECURITY) != GT_OK)
	{
        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
        return GT_NOT_SUPPORTED;
	}

	if (IS_IN_DEV_GROUP(dev,DEV_CROSS_CHIP_VLAN))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Set the protected mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,3,1, data);

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
* gprtGetProtectedMode
*
* DESCRIPTION:
*       This routine gets protected mode of a switch port.
*		When this mode is set to GT_TRUE, frames are allowed to egress port
*		defined by the 802.1Q VLAN membership for the frame's VID 'AND'
*		by the port's VLANTable if 802.1Q is enabled on the port. Both must
*		allow the frame to Egress.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE: header mode enabled
*	       GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetProtectedMode
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT    port,
    OUT GT_BOOL     *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetProtectedMode Called.\n"));

    if(IS_VALID_API_CALL(dev,port, DEV_PORT_SECURITY) != GT_OK)
	{
        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
        return GT_NOT_SUPPORTED;
	}

	if (IS_IN_DEV_GROUP(dev,DEV_CROSS_CHIP_VLAN))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the protected Mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL, 3, 1, &data);

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

    return retVal;
}

/*******************************************************************************
* gprtSetForwardUnknown
*
* DESCRIPTION:
*       This routine set Forward Unknown mode of a switch port.
*		When this mode is set to GT_TRUE, normal switch operation occurs.
*		When this mode is set to GT_FALSE, unicast frame with unknown DA addresses
*		will not egress out this port.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for protected mode or GT_FALSE otherwise
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetForwardUnknown
(
    IN  GT_QD_DEV   *dev,
    IN GT_LPORT     port,
    IN GT_BOOL      mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetForwardUnknown Called.\n"));

    if(IS_VALID_API_CALL(dev,port, DEV_PORT_SECURITY|DEV_EGRESS_FLOOD) != GT_OK)
	{
        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
        return GT_NOT_SUPPORTED;
	}

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Set the forward unknown mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,2,1, data);

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
* gprtGetForwardUnknown
*
* DESCRIPTION:
*       This routine gets Forward Unknown mode of a switch port.
*		When this mode is set to GT_TRUE, normal switch operation occurs.
*		When this mode is set to GT_FALSE, unicast frame with unknown DA addresses
*		will not egress out this port.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE: header mode enabled
*	       GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetForwardUnknown
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT    port,
    OUT GT_BOOL     *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetForwardUnknown Called.\n"));

    if(IS_VALID_API_CALL(dev,port, DEV_PORT_SECURITY|DEV_EGRESS_FLOOD) != GT_OK)
	{
        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
        return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the forward unknown Mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL, 2, 1, &data);

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

    return retVal;
}


/*******************************************************************************
* gprtSetDropOnLock
*
* DESCRIPTION:
*		This routine set the Drop on Lock. When set to one, Ingress frames will
*		be discarded if their SA field is not in the ATU's address database.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for Unknown SA drop or GT_FALSE otherwise
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetDropOnLock
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL		mode
)
{
	GT_U16          data;
	GT_STATUS       retVal;         /* Functions return value.      */
	GT_U8           hwPort;         /* the physical port number     */

	DBG_INFO(("gprtSetDropOnLock Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_CASCADE_PORT))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	/* translate BOOL to binary */
	BOOL_2_BIT(mode, data);

	/* Set the DropOnLock mode.            */
	retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,14,1,data);

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
* gprtGetDropOnLock
*
* DESCRIPTION:
*		This routine gets DropOnLock mode.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE: DropOnLock enabled,
*				 GT_FALSE otherwise
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
GT_STATUS gprtGetDropOnLock
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
	GT_STATUS       retVal;         /* Functions return value.      */
	GT_U8           hwPort;         /* the physical port number     */
	GT_U16          data;           /* to keep the read valve       */

	DBG_INFO(("gprtGetDropOnLock Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_CASCADE_PORT))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	/* Get the DropOnLock Mode.            */
	retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL, 14, 1, &data);

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

	return retVal;
}

/*******************************************************************************
* gprtSetDoubleTag
*
* DESCRIPTION:
*		This routine set the Ingress Double Tag Mode. When set to one,
*		ingressing frames are examined to see if they contain an 802.3ac tag.
*		If they do, the tag is removed and then the frame is processed from
*		there (i.e., removed tag is ignored). Essentially, untagged frames
*		remain untagged, single tagged frames become untagged and double tagged
*		frames become single tagged.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for DoulbeTag mode or GT_FALSE otherwise
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetDoubleTag
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL		mode
)
{
	GT_U16          data;
	GT_STATUS       retVal;         /* Functions return value.      */
	GT_U8           hwPort;         /* the physical port number     */

	DBG_INFO(("gprtSetDoubleTag Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_INGRESS_DOUBLE_TAGGING))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	/* translate BOOL to binary */
	BOOL_2_BIT(mode, data);

	/* Set the DoubleTag mode.            */
	retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,9,1,data);

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
* gprtGetDoubleTag
*
* DESCRIPTION:
*		This routine gets DoubleTag mode.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE: DoubleTag enabled,
*				 GT_FALSE otherwise
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
GT_STATUS gprtGetDoubleTag
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
	GT_STATUS       retVal;         /* Functions return value.      */
	GT_U8           hwPort;         /* the physical port number     */
	GT_U16          data;           /* to keep the read valve       */

	DBG_INFO(("gprtGetDoubleTag Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_INGRESS_DOUBLE_TAGGING))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	/* Get the DoubleTag Mode.            */
	retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL, 9, 1, &data);

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

	return retVal;
}


/*******************************************************************************
* gprtSetInterswitchPort
*
* DESCRIPTION:
*		This routine set Interswitch Port. When set to one,
*		it indicates this port is a interswitch port used to communicated with
*		CPU or to cascade with another switch device.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for Interswitch port or GT_FALSE otherwise
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetInterswitchPort
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL		mode
)
{
	GT_U16          data;
	GT_STATUS       retVal;         /* Functions return value.      */
	GT_U8           hwPort;         /* the physical port number     */

	DBG_INFO(("gprtSetInterswitchPort Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_CASCADE_PORT))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	/* translate BOOL to binary */
	BOOL_2_BIT(mode, data);

	/* Set the InterswitchPort.            */
	retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,8,1,data);

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
* gprtGetInterswithPort
*
* DESCRIPTION:
*		This routine gets InterswitchPort.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE: This port is interswitch port,
*				 GT_FALSE otherwise
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
GT_STATUS gprtGetInterswitchPort
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
	GT_STATUS       retVal;         /* Functions return value.      */
	GT_U8           hwPort;         /* the physical port number     */
	GT_U16          data;           /* to keep the read valve       */

	DBG_INFO(("gprtGetInterswitchPort Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_CASCADE_PORT))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	/* Get the InterswitchPort Mode.            */
	retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL, 8, 1, &data);

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

	return retVal;
}

/*******************************************************************************
* gprtSetLearnDisable
*
* DESCRIPTION:
*		This routine enables/disables automatic learning of new source MAC
*		addresses on the given port ingress
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for disable or GT_FALSE otherwise
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetLearnDisable
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL		mode
)
{
	GT_U16          data;
	GT_STATUS       retVal;         /* Functions return value.      */
	GT_U8           hwPort;         /* the physical port number     */

	DBG_INFO(("gprtSetLearnDisable Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	/* translate BOOL to binary */
	BOOL_2_BIT(mode, data);

	/* Set the LearnDisable mode.            */
	retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_VLAN_MAP,11,1,data);

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
* gprtGetLearnDisable
*
* DESCRIPTION:
*		This routine gets LearnDisable setup
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE: Learning disabled on the given port ingress frames,
*				 GT_FALSE otherwise
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
GT_STATUS gprtGetLearnDisable
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
	GT_STATUS       retVal;         /* Functions return value.      */
	GT_U8           hwPort;         /* the physical port number     */
	GT_U16          data;           /* to keep the read valve       */

	DBG_INFO(("gprtGetLearnDisable Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	/* Get the LearnDisable Mode.            */
	retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_VLAN_MAP, 11, 1, &data);

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

	return retVal;
}

/*******************************************************************************
* gprtSetIgnoreFCS
*
* DESCRIPTION:
*		This routine sets FCS Ignore mode. When this bit is set to a one,
*		the last four bytes of frames received on this port are overwritten with
*		a good CRC and the frames will be accepted by the switch.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for ignore FCS or GT_FALSE otherwise
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetIgnoreFCS
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL 		mode
)
{
	GT_U16          data;
	GT_STATUS       retVal;         /* Functions return value.      */
	GT_U8           hwPort;         /* the physical port number     */

	DBG_INFO(("gprtSetIgnoreFCS Called.\n"));

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH|DEV_ENHANCED_FE_SWITCH))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	/* translate BOOL to binary */
	BOOL_2_BIT(mode, data);

	/* Set the IgnoreFCS mode.            */
	if (IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY|DEV_ENHANCED_FE_SWITCH))
	{
		retVal = hwSetPortRegField(dev,hwPort,QD_REG_PORT_CONTROL2,15,1,data );
	}
	else
	{
		retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_VLAN_MAP,10,1,data);
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
* gprtGetIgnoreFCS
*
* DESCRIPTION:
*		This routine gets Ignore FCS setup
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE: Ignore FCS on the given port's ingress frames,
*				 GT_FALSE otherwise
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
GT_STATUS gprtGetIgnoreFCS
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetIgnoreFCS Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH|DEV_ENHANCED_FE_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the IgnoreFCS Mode.            */
	if (IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY|DEV_ENHANCED_FE_SWITCH))
	{
		retVal = hwGetPortRegField(dev,hwPort,QD_REG_PORT_CONTROL2,15,1,&data );
	}
	else
	{
		retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_VLAN_MAP, 10, 1, &data);
	}

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

    return retVal;
}


/*******************************************************************************
* gprtSetVTUPriOverride
*
* DESCRIPTION:
*		VTU Priority Override. The following modes are supported:
*			PRI_OVERRIDE_NONE -
*				Normal frame priority processing occurs.
*			PRI_OVERRIDE_FRAME_QUEUE -
*				Both frame and queue overrides take place on the frame.
*			PRI_OVERRIDE_FRAME -
*				Priority assigned to the frame's VID (in the VTU table) is used
*				to overwite the frame's FPri (frame priority).
*				If the frame egresses tagged, the priority in the frame will be
*				this new priority value.
*			PRI_OVERRIDE_QUEUE -
*				Priority assigned to the frame's VID (in the VTU table) is used
*				to overwite the frame's QPri (queue priority).
*				QPri is used internally to map the frame to one of the egress
*				queues inside the switch.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_PRI_OVERRIDE type
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if mode is invalid
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		PRI_OVERRIDE_FRAME and PRI_OVERRIDE_QUEUE modes are supported only on
*		certain switch device. Please refer to the device datasheet.
*
*******************************************************************************/
GT_STATUS gprtSetVTUPriOverride
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_PRI_OVERRIDE		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetVTUPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PRIORITY_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	switch (mode)
	{
		case PRI_OVERRIDE_NONE:
			data = 0;
			break;
		case PRI_OVERRIDE_FRAME_QUEUE:
			if (IS_IN_DEV_GROUP(dev,DEV_EXT_PRIORITY_OVERRIDE))
			{
				data = 3;
			}
			else
			{
				data = 1;
			}
			break;
		case PRI_OVERRIDE_FRAME:
			if (!IS_IN_DEV_GROUP(dev,DEV_EXT_PRIORITY_OVERRIDE))
			{
		        DBG_INFO(("Bad Parameter\n"));
				return GT_BAD_PARAM;
			}
			data = 1;
			break;
		case PRI_OVERRIDE_QUEUE:
			if (!IS_IN_DEV_GROUP(dev,DEV_EXT_PRIORITY_OVERRIDE))
			{
		        DBG_INFO(("Bad Parameter\n"));
				return GT_BAD_PARAM;
			}
			data = 2;
			break;
		default:
	        DBG_INFO(("Bad Parameter\n"));
			return GT_BAD_PARAM;
	}

    /* Set the VTUPri Override mode.            */
	if (IS_IN_DEV_GROUP(dev,DEV_EXT_PRIORITY_OVERRIDE))
	{
	    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PRI_OVERRIDE,10,2,data);
	}
	else
	{
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,14,1,data);
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
* gprtGetVTUPriOverride
*
* DESCRIPTION:
*		VTU Priority Override. The following modes are supported:
*			PRI_OVERRIDE_NONE -
*				Normal frame priority processing occurs.
*			PRI_OVERRIDE_FRAME_QUEUE -
*				Both frame and queue overrides take place on the frame.
*			PRI_OVERRIDE_FRAME -
*				Priority assigned to the frame's VID (in the VTU table) is used
*				to overwite the frame's FPri (frame priority).
*				If the frame egresses tagged, the priority in the frame will be
*				this new priority value.
*			PRI_OVERRIDE_QUEUE -
*				Priority assigned to the frame's VID (in the VTU table) is used
*				to overwite the frame's QPri (queue priority).
*				QPri is used internally to map the frame to one of the egress
*				queues inside the switch.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_PRI_OVERRIDE type
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		PRI_OVERRIDE_FRAME and PRI_OVERRIDE_QUEUE modes are supported only on
*		certain switch device. Please refer to the device datasheet.
*
*******************************************************************************/
GT_STATUS gprtGetVTUPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_PRI_OVERRIDE		*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetVTUPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PRIORITY_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the VTUPriOverride Mode.            */
	if (IS_IN_DEV_GROUP(dev,DEV_EXT_PRIORITY_OVERRIDE))
	{
	    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PRI_OVERRIDE,10,2,&data);
	}
	else
	{
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2, 14, 1, &data);
		if(data == 1)
			data = 3;
	}

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
		return retVal;
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

	switch (data)
	{
		case 0:
			*mode = PRI_OVERRIDE_NONE;
			break;
		case 3:
			*mode = PRI_OVERRIDE_FRAME_QUEUE;
			break;
		case 1:
			*mode = PRI_OVERRIDE_FRAME;
			break;
		case 2:
			*mode = PRI_OVERRIDE_QUEUE;
			break;
		default:
	        DBG_INFO(("Bad Parameter\n"));
			return GT_BAD_PARAM;
	}

    return GT_OK;
}


/*******************************************************************************
* gprtSetSAPriOverride
*
* DESCRIPTION:
*		SA Priority Override. The following mode is supported:
*			PRI_OVERRIDE_NONE -
*				Normal frame priority processing occurs.
*			PRI_OVERRIDE_FRAME_QUEUE -
*				Both frame and queue overrides take place on the frame.
*			PRI_OVERRIDE_FRAME -
*				Priority assigned to the frame's SA (in the ATU table) is used
*				to overwite the frame's FPri (frame priority).
*				If the frame egresses tagged, the priority in the frame will be
*				this new priority value.
*			PRI_OVERRIDE_QUEUE -
*				Priority assigned to the frame's SA (in the ATU table) is used
*				to overwite the frame's QPri (queue priority).
*				QPri is used internally to map the frame to one of the egress
*				queues inside the switch.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_PRI_OVERRIDE type
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if mode is invalid
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		PRI_OVERRIDE_FRAME and PRI_OVERRIDE_QUEUE modes are supported only on
*		certain switch device. Please refer to the device datasheet.
*
*******************************************************************************/
GT_STATUS gprtSetSAPriOverride
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetSAPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PRIORITY_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	switch (mode)
	{
		case PRI_OVERRIDE_NONE:
			data = 0;
			break;
		case PRI_OVERRIDE_FRAME_QUEUE:
			if (IS_IN_DEV_GROUP(dev,DEV_EXT_PRIORITY_OVERRIDE))
			{
				data = 3;
			}
			else
			{
				data = 1;
			}
			break;
		case PRI_OVERRIDE_FRAME:
			if (!IS_IN_DEV_GROUP(dev,DEV_EXT_PRIORITY_OVERRIDE))
			{
		        DBG_INFO(("Bad Parameter\n"));
				return GT_BAD_PARAM;
			}
			data = 1;
			break;
		case PRI_OVERRIDE_QUEUE:
			if (!IS_IN_DEV_GROUP(dev,DEV_EXT_PRIORITY_OVERRIDE))
			{
		        DBG_INFO(("Bad Parameter\n"));
				return GT_BAD_PARAM;
			}
			data = 2;
			break;
		default:
	        DBG_INFO(("Bad Parameter\n"));
			return GT_BAD_PARAM;
	}

    /* Set the SAPriOverride mode.            */
	if (IS_IN_DEV_GROUP(dev,DEV_EXT_PRIORITY_OVERRIDE))
	{
	    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PRI_OVERRIDE,12,2,data);
	}
	else
	{
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,13,1,data);
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
* gprtGetSAPriOverride
*
* DESCRIPTION:
*		SA Priority Override. The following mode is supported:
*			PRI_OVERRIDE_NONE -
*				Normal frame priority processing occurs.
*			PRI_OVERRIDE_FRAME_QUEUE -
*				Both frame and queue overrides take place on the frame.
*			PRI_OVERRIDE_FRAME -
*				Priority assigned to the frame's SA (in the ATU table) is used
*				to overwite the frame's FPri (frame priority).
*				If the frame egresses tagged, the priority in the frame will be
*				this new priority value.
*			PRI_OVERRIDE_QUEUE -
*				Priority assigned to the frame's SA (in the ATU table) is used
*				to overwite the frame's QPri (queue priority).
*				QPri is used internally to map the frame to one of the egress
*				queues inside the switch.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_PRI_OVERRIDE type
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		PRI_OVERRIDE_FRAME and PRI_OVERRIDE_QUEUE modes are supported only on
*		certain switch device. Please refer to the device datasheet.
*
*******************************************************************************/
GT_STATUS gprtGetSAPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_PRI_OVERRIDE		*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetSAPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PRIORITY_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the SAPriOverride Mode.            */
	if (IS_IN_DEV_GROUP(dev,DEV_EXT_PRIORITY_OVERRIDE))
	{
	    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PRI_OVERRIDE,12,2,&data);
	}
	else
	{
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2, 13, 1, &data);
		if(data == 1)
			data = 3;
	}

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
		return retVal;
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

	switch (data)
	{
		case 0:
			*mode = PRI_OVERRIDE_NONE;
			break;
		case 3:
			*mode = PRI_OVERRIDE_FRAME_QUEUE;
			break;
		case 1:
			*mode = PRI_OVERRIDE_FRAME;
			break;
		case 2:
			*mode = PRI_OVERRIDE_QUEUE;
			break;
		default:
	        DBG_INFO(("Bad Parameter\n"));
			return GT_BAD_PARAM;
	}

    return GT_OK;
}

/*******************************************************************************
* gprtSetDAPriOverride
*
* DESCRIPTION:
*		DA Priority Override. The following mode is supported:
*			PRI_OVERRIDE_NONE -
*				Normal frame priority processing occurs.
*			PRI_OVERRIDE_FRAME -
*				Priority assigned to the frame's DA (in the ATU table) is used
*				to overwite the frame's FPri (frame priority).
*				If the frame egresses tagged, the priority in the frame will be
*				this new priority value.
*			PRI_OVERRIDE_QUEUE -
*				Priority assigned to the frame's DA (in the ATU table) is used
*				to overwite the frame's QPri (queue priority).
*				QPri is used internally to map the frame to one of the egress
*				queues inside the switch.
*			PRI_OVERRIDE_FRAME_QUEUE -
*				Both frame and queue overrides take place on the frame.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_PRI_OVERRIDE type
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if mode is invalid
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		PRI_OVERRIDE_FRAME and PRI_OVERRIDE_QUEUE modes are supported only on
*		certain switch device. Please refer to the device datasheet.
*
*******************************************************************************/
GT_STATUS gprtSetDAPriOverride
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_PRI_OVERRIDE		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetDAPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PRIORITY_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	switch (mode)
	{
		case PRI_OVERRIDE_NONE:
			data = 0;
			break;
		case PRI_OVERRIDE_FRAME_QUEUE:
			if (IS_IN_DEV_GROUP(dev,DEV_EXT_PRIORITY_OVERRIDE))
			{
				data = 3;
			}
			else
			{
				data = 1;
			}
			break;
		case PRI_OVERRIDE_FRAME:
			if (!IS_IN_DEV_GROUP(dev,DEV_EXT_PRIORITY_OVERRIDE))
			{
		        DBG_INFO(("Bad Parameter\n"));
				return GT_BAD_PARAM;
			}
			data = 1;
			break;
		case PRI_OVERRIDE_QUEUE:
			if (!IS_IN_DEV_GROUP(dev,DEV_EXT_PRIORITY_OVERRIDE))
			{
		        DBG_INFO(("Bad Parameter\n"));
				return GT_BAD_PARAM;
			}
			data = 2;
			break;
		default:
	        DBG_INFO(("Bad Parameter\n"));
			return GT_BAD_PARAM;
	}

    /* Set the DAPriOverride mode.            */
	if (IS_IN_DEV_GROUP(dev,DEV_EXT_PRIORITY_OVERRIDE))
	{
	    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PRI_OVERRIDE,14,2,data);
	}
	else
	{
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,12,1,data);
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
* gprtGetDAPriOverride
*
* DESCRIPTION:
*		DA Priority Override. The following mode is supported:
*			PRI_OVERRIDE_NONE -
*				Normal frame priority processing occurs.
*			PRI_OVERRIDE_FRAME -
*				Priority assigned to the frame's DA (in the ATU table) is used
*				to overwite the frame's FPri (frame priority).
*				If the frame egresses tagged, the priority in the frame will be
*				this new priority value.
*			PRI_OVERRIDE_QUEUE -
*				Priority assigned to the frame's DA (in the ATU table) is used
*				to overwite the frame's QPri (queue priority).
*				QPri is used internally to map the frame to one of the egress
*				queues inside the switch.
*			PRI_OVERRIDE_FRAME_QUEUE -
*				Both frame and queue overrides take place on the frame.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_PRI_OVERRIDE type
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		PRI_OVERRIDE_FRAME and PRI_OVERRIDE_QUEUE modes are supported only on
*		certain switch device. Please refer to the device datasheet.
*
*******************************************************************************/
GT_STATUS gprtGetDAPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_PRI_OVERRIDE		*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetDAPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PRIORITY_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the DAPriOverride Mode.            */
	if (IS_IN_DEV_GROUP(dev,DEV_EXT_PRIORITY_OVERRIDE))
	{
	    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PRI_OVERRIDE,14,2,&data);
	}
	else
	{
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2, 12, 1, &data);
		if(data == 1)
			data = 3;
	}

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
		return retVal;
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

	switch (data)
	{
		case 0:
			*mode = PRI_OVERRIDE_NONE;
			break;
		case 3:
			*mode = PRI_OVERRIDE_FRAME_QUEUE;
			break;
		case 1:
			*mode = PRI_OVERRIDE_FRAME;
			break;
		case 2:
			*mode = PRI_OVERRIDE_QUEUE;
			break;
		default:
	        DBG_INFO(("Bad Parameter\n"));
			return GT_BAD_PARAM;
	}

    return GT_OK;
}


/*******************************************************************************
* gprtSetCPUPort
*
* DESCRIPTION:
*		This routine sets CPU Port number. When Snooping is enabled on this port
*		or when this port is configured as an Interswitch Port and it receives a
*		To_CPU frame, the switch needs to know what port on this device the frame
*		should egress.
*
* INPUTS:
*		port - the logical port number.
*		cpuPort - CPU Port number or interswitch port where CPU Port is connected
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetCPUPort
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_LPORT 	cpuPort
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetCPUPort Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    data = (GT_U16)GT_LPORT_2_PORT(cpuPort);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_CPU_DEST_PER_PORT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Set the CPU Port.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,0,4,data);

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
* gprtGetCPUPort
*
* DESCRIPTION:
*		This routine gets CPU Logical Port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		cpuPort - CPU Port's logical number
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
GT_STATUS gprtGetCPUPort
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_LPORT 	*cpuLPort
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetCPUPort Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_CPU_DEST_PER_PORT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the CPUPort.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2, 0, 4, &data);

    *cpuLPort = GT_PORT_2_LPORT((GT_U8)data);

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
* gprtSetLockedPort
*
* DESCRIPTION:
*		This routine sets LockedPort. When it's set to one, CPU directed
*		learning for 802.1x MAC authentication is enabled on this port. In this
*		mode, an ATU Miss Violation interrupt will occur when a new SA address
*		is received in a frame on this port. Automatically SA learning and
*		refreshing is disabled in this mode.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for Locked Port, GT_FALSE otherwise
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetLockedPort
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetLockedPort Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH|DEV_ENHANCED_FE_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set Locked Port.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_ASSOCIATION, 13, 1, data);

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
* gprtGetLockedPort
*
* DESCRIPTION:
*		This routine gets Locked Port mode for the given port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode  - GT_TRUE if LockedPort, GT_FALSE otherwise
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
GT_STATUS gprtGetLockedPort
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL  	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetLockedPort Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH|DEV_ENHANCED_FE_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the LockedPort. */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_ASSOCIATION, 13, 1, &data);

    BIT_2_BOOL(data, *mode);

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
* gprtSetIgnoreWrongData
*
* DESCRIPTION:
*		This routine sets Ignore Wrong Data. If the frame's SA address is found
*		in the database and if the entry is 'static' or if the port is 'locked'
*		the source port's bit is checked to insure the SA has been assigned to
*		this port. If the SA is NOT assigned to this port, it is considered an
*		ATU Member Violation. If the IgnoreWrongData is set to GT_FALSE, an ATU
*		Member Violation interrupt will be generated. If it's set to GT_TRUE,
*		the ATU Member Violation error will be masked and ignored.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for IgnoreWrongData, GT_FALSE otherwise
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetIgnoreWrongData
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetIgnoreWrongData Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH|DEV_ENHANCED_FE_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set IgnoreWrongData.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_ASSOCIATION, 12, 1, data);

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
* gprtGetIgnoreWrongData
*
* DESCRIPTION:
*		This routine gets Ignore Wrong Data mode for the given port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode  - GT_TRUE if IgnoreWrongData, GT_FALSE otherwise
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
GT_STATUS gprtGetIgnoreWrongData
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetIgnoreWrongData Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH|DEV_ENHANCED_FE_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the IgnoreWrongData. */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_ASSOCIATION, 12, 1, &data);

    BIT_2_BOOL(data, *mode);

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
* gprtSetDiscardTagged
*
* DESCRIPTION:
*		When this bit is set to a one, all non-MGMT frames that are processed as
*		Tagged will be discarded as they enter this switch port. Priority only
*		tagged frames (with a VID of 0x000) are considered tagged.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to discard tagged frame, GT_FALSE otherwise
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetDiscardTagged
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetDiscardTagged Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY|DEV_ENHANCED_FE_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set DiscardTagged. */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2, 9, 1, data);

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
* gprtGetDiscardTagged
*
* DESCRIPTION:
*		This routine gets DiscardTagged bit for the given port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode  - GT_TRUE if DiscardTagged bit is set, GT_FALSE otherwise
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
GT_STATUS gprtGetDiscardTagged
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetDiscardTagged Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY|DEV_ENHANCED_FE_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the DiscardTagged. */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2, 9, 1, &data);

    BIT_2_BOOL(data, *mode);

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
* gprtSetDiscardUntagged
*
* DESCRIPTION:
*		When this bit is set to a one, all non-MGMT frames that are processed as
*		Untagged will be discarded as they enter this switch port. Priority only
*		tagged frames (with a VID of 0x000) are considered tagged.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to discard untagged frame, GT_FALSE otherwise
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetDiscardUntagged
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetDiscardUntagged Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY|DEV_ENHANCED_FE_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set DiscardUnTagged. */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2, 8, 1, data);

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
* gprtGetDiscardUntagged
*
* DESCRIPTION:
*		This routine gets DiscardUntagged bit for the given port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode  - GT_TRUE if DiscardUntagged bit is set, GT_FALSE otherwise
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
GT_STATUS gprtGetDiscardUntagged
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetDiscardUnTagged Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY|DEV_ENHANCED_FE_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the DiscardUnTagged. */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2, 8, 1, &data);

    BIT_2_BOOL(data, *mode);

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
* gprtSetMapDA
*
* DESCRIPTION:
*		When this bit is set to a one, normal switch operation will occur where a
*		frame's DA address is used to direct the frame out the correct port.
*		When this be is cleared to a zero, the frame will be sent out the port(s)
*		defined by ForwardUnknown bits or the DefaultForward bits even if the DA
*		is ound in the address database.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to use MapDA, GT_FALSE otherwise
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetMapDA
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetMapDA Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY|DEV_ENHANCED_FE_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set MapDA. */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2, 7, 1, data);

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
* gprtGetMapDA
*
* DESCRIPTION:
*		This routine gets MapDA bit for the given port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode  - GT_TRUE if MapDA bit is set, GT_FALSE otherwise
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
GT_STATUS gprtGetMapDA
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetMapDA Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY|DEV_ENHANCED_FE_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the MapDA. */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2, 7, 1, &data);

    BIT_2_BOOL(data, *mode);

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
* gprtSetDefaultForward
*
* DESCRIPTION:
*		When this bit is set to a one, normal switch operation will occurs and
*		multicast frames with unknown DA addresses are allowed to egress out this
*		port (assuming the VLAN settings allow the frame to egress this port too).
*		When this bit is cleared to a zero, multicast frames with unknown DA
*		addresses will not egress out this port.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to use DefaultForward, GT_FALSE otherwise
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
GT_STATUS gprtSetDefaultForward
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetDefaultForward Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set DefaultForward. */
	if (!IS_IN_DEV_GROUP(dev,DEV_EGRESS_FLOOD))
	{
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2, 6, 1, data);
	}
	else
	{
	    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL, 3, 1, data);
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
* gprtGetDefaultForward
*
* DESCRIPTION:
*		This routine gets DefaultForward bit for the given port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode  - GT_TRUE if DefaultForward bit is set, GT_FALSE otherwise
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
GT_STATUS gprtGetDefaultForward
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetDefaultForward Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the DefaultForward. */
	if (!IS_IN_DEV_GROUP(dev,DEV_EGRESS_FLOOD))
	{
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2, 6, 1, &data);
	}
	else
	{
	    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL, 3, 1, &data);
	}

    BIT_2_BOOL(data, *mode);

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
* gprtSetEgressMonitorSource
*
* DESCRIPTION:
*		When this be is cleared to a zero, normal network switching occurs.
*		When this bit is set to a one, any frame that egresses out this port will
*		also be sent to the EgressMonitorDest Port
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to set EgressMonitorSource, GT_FALSE otherwise
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetEgressMonitorSource
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetEgressMonitorSource Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set EgressMonitorSource. */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2, 5, 1, data);

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
* gprtGetEgressMonitorSource
*
* DESCRIPTION:
*		This routine gets EgressMonitorSource bit for the given port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode  - GT_TRUE if EgressMonitorSource bit is set, GT_FALSE otherwise
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
GT_STATUS gprtGetEgressMonitorSource
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetEgressMonitorSource Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the EgressMonitorSource. */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2, 5, 1, &data);

    BIT_2_BOOL(data, *mode);

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
* gprtSetIngressMonitorSource
*
* DESCRIPTION:
*		When this be is cleared to a zero, normal network switching occurs.
*		When this bit is set to a one, any frame that egresses out this port will
*		also be sent to the EgressMonitorDest Port
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to set EgressMonitorSource, GT_FALSE otherwise
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetIngressMonitorSource
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetIngressMonitorSource Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set IngressMonitorSource. */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2, 4, 1, data);

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
* gprtGetIngressMonitorSource
*
* DESCRIPTION:
*		This routine gets IngressMonitorSource bit for the given port
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode  - GT_TRUE if IngressMonitorSource bit is set, GT_FALSE otherwise
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
GT_STATUS gprtGetIngressMonitorSource
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetIngressMonitorSource Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the IngressMonitorSource. */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2, 4, 1, &data);

    BIT_2_BOOL(data, *mode);

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
* gprtSetMessagePort
*
* DESCRIPTION:
*		When the Learn2All bit is set to one, learning message frames are
*		generated. These frames will be sent out all ports whose Message Port is
*		set to one.
* 		If this feature is used, it is recommended that all Marvell Tag ports,
*		except for the CPU's port, have their MessagePort bit set to one.
*		Ports that are not Marvell Tag ports should not have their Message Port
*		bit set to one.
*
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to make this port a Message Port. GT_FALSE, otherwise.
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
GT_STATUS gprtSetMessagePort
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetMessagePort Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_TRUNK))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set IngressMonitorSource. */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL1, 15, 1, data);

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
* gprtGetMessagePort
*
* DESCRIPTION:
*		When the Learn2All bit is set to one, learning message frames are
*		generated. These frames will be sent out all ports whose Message Port is
*		set to one.
* 		If this feature is used, it is recommended that all Marvell Tag ports,
*		except for the CPU's port, have their MessagePort bit set to one.
*		Ports that are not Marvell Tag ports should not have their Message Port
*		bit set to one.
*
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE to make this port a Message Port. GT_FALSE, otherwise.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetMessagePort
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL 	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetMessagePort Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_TRUNK))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the IngressMonitorSource. */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL1, 15, 1, &data);

    BIT_2_BOOL(data, *mode);

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
* gprtSetTrunkPort
*
* DESCRIPTION:
*		This function enables/disables and sets the trunk ID.
*
* INPUTS:
*		port - the logical port number.
*		en - GT_TRUE to make the port be a member of a trunk with the given trunkId.
*			 GT_FALSE, otherwise.
*		trunkId - valid ID is 0 ~ 15.
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if trunkId is neither valid nor INVALID_TRUNK_ID
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtSetTrunkPort
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL 		en,
	IN GT_U32		trunkId
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetTrunkPort Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_TRUNK))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(en, data);

	if(en == GT_TRUE)
	{
		/* need to enable trunk. so check the trunkId */
		if (!IS_TRUNK_ID_VALID(dev, trunkId))
		{
	        DBG_INFO(("GT_BAD_PARAM\n"));
			return GT_BAD_PARAM;
		}

	    /* Set TrunkId. */
		if (IS_IN_DEV_GROUP(dev,DEV_TRUNK_NEW_ID_LOCATION))
		{
	    	retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL1, 8, 4, (GT_U16)trunkId);
		}
		else
		{
    	retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL1, 4, 4, (GT_U16)trunkId);
		}

	    if(retVal != GT_OK)
		{
	       	DBG_INFO(("Failed.\n"));
			return retVal;
		}

	}
	else
	{
		/*
		   Need to reset trunkId for 88E6095 rev0.
		*/
		if (IS_IN_DEV_GROUP(dev,DEV_NEW_FEATURE_IN_REV) &&
			((GT_DEVICE_REV)dev->revision < GT_REV_1))
		{
			trunkId = 0;

	    	/* Set TrunkId. */
	    	retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL1, 4, 4, (GT_U16)trunkId);
		    if(retVal != GT_OK)
			{
		       	DBG_INFO(("Failed.\n"));
				return retVal;
			}
		}
	}

    /* Set TrunkPort bit. */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL1, 14, 1, data);

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
* gprtGetTrunkPort
*
* DESCRIPTION:
*		This function returns trunk state of the port.
*		When trunk is disabled, trunkId field won't have valid value.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		en - GT_TRUE, if the port is a member of a trunk,
*			 GT_FALSE, otherwise.
*		trunkId - 0 ~ 15, valid only if en is GT_TRUE
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetTrunkPort
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	OUT GT_BOOL 	*en,
	OUT GT_U32		*trunkId
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetTrunkPort Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_TRUNK))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	data = 0;

    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL1, 14, 1, &data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
		return retVal;
	}

    BIT_2_BOOL(data, *en);

	if (IS_IN_DEV_GROUP(dev,DEV_TRUNK_NEW_ID_LOCATION))
	{
	    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL1, 8, 4, &data);
	}
	else
	{
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL1, 4, 4, &data);
	}

	*trunkId = (GT_U32)data;

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
* gprtGetDiscardBCastMode
*
* DESCRIPTION:
*       This routine gets the Discard Broadcast Mode. If the mode is enabled,
*		all the broadcast frames to the given port will be discarded.
*
* INPUTS:
*       port - logical port number
*
* OUTPUTS:
*		en - GT_TRUE, if enabled,
*			 GT_FALSE, otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetDiscardBCastMode
(
	IN  GT_QD_DEV    *dev,
	IN  GT_LPORT     port,
	OUT GT_BOOL 	 *en
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetDiscardBCastMode Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_DROP_BCAST))
	{
		if (!IS_IN_DEV_GROUP(dev,DEV_NEW_FEATURE_IN_REV) ||
			((GT_DEVICE_REV)dev->revision < GT_REV_1))
	    {
    	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
			return GT_NOT_SUPPORTED;
	    }
	}

	data = 0;

    retVal = hwGetPortRegField(dev,hwPort, 0x15, 6, 1, &data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
		return retVal;
	}

    BIT_2_BOOL(data, *en);

    return GT_OK;
}


/*******************************************************************************
* gprtSetDiscardBCastMode
*
* DESCRIPTION:
*       This routine sets the Discard Broadcast mode.
*		If the mode is enabled, all the broadcast frames to the given port will
*		be discarded.
*
* INPUTS:
*       port - logical port number
*		en - GT_TRUE, to enable the mode,
*			 GT_FALSE, otherwise.
*
* OUTPUTS:
*		None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetDiscardBCastMode
(
	IN  GT_QD_DEV    *dev,
	IN  GT_LPORT     port,
	IN  GT_BOOL 	 en
)
{
    GT_U16          data;           /* Used to poll the data */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetDiscardBCastMode Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_DROP_BCAST))
	{
		if (!IS_IN_DEV_GROUP(dev,DEV_NEW_FEATURE_IN_REV) ||
			((GT_DEVICE_REV)dev->revision < GT_REV_1))
	    {
    	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
			return GT_NOT_SUPPORTED;
	    }
	}

    /* translate BOOL to binary */
    BOOL_2_BIT(en, data);

    retVal = hwSetPortRegField(dev,hwPort, 0x15, 6, 1, data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
		return retVal;
	}

    return GT_OK;
}

/*******************************************************************************
* gprtGetFCOnRateLimitMode
*
* DESCRIPTION:
*       This routine returns mode that tells if ingress rate limiting uses Flow
*		Control. When this mode is enabled and the port receives frames over the
*		limit, Ingress Rate Limiting will be performed by stalling the
*		link partner using flow control, instead of discarding frames.
*
* INPUTS:
*       port - logical port number
*
* OUTPUTS:
*		en - GT_TRUE, if the mode is enabled,
*			 GT_FALSE, otherwise.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		In order for this mode to work, Flow Control and Rate Limiting
*		should be configured properly.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetFCOnRateLimitMode
(
	IN  GT_QD_DEV    *dev,
	IN  GT_LPORT     port,
	OUT GT_BOOL 	 *en
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetFCOnRateLimitMode Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_BURST_RATE))
	{
		if (!IS_IN_DEV_GROUP(dev,DEV_NEW_FEATURE_IN_REV) ||
			((GT_DEVICE_REV)dev->revision < GT_REV_1))
	    {
    	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
			return GT_NOT_SUPPORTED;
	    }
	}

	data = 0;

    retVal = hwGetPortRegField(dev,hwPort, 0x15, 4, 2, &data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
		return retVal;
	}

	if (data == 0x3)
		*en = GT_TRUE;
	else
		*en = GT_FALSE;

    return GT_OK;
}


/*******************************************************************************
* gprtSetFCOnRateLimitMode
*
* DESCRIPTION:
*       This routine sets the mode that tells if ingress rate limiting uses Flow
*		Control. When this mode is enabled and the port receives frames over the
*		limit, Ingress Rate Limiting will be performed by stalling the
*		link partner using flow control, instead of discarding frames.
*
* INPUTS:
*       port - logical port number
*		en - GT_TRUE, to enable the mode,
*			 GT_FALSE, otherwise.
*
* OUTPUTS:
*		None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       This routine won't configure Flow Control or Rate Limiting.
*		In order for this mode to work, Flow Control and Rate Limiting
*		should be configured properly.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetFCOnRateLimitMode
(
	IN  GT_QD_DEV    *dev,
	IN  GT_LPORT     port,
	IN  GT_BOOL 	 en
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetFCOnRateLimitMode Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_BURST_RATE))
	{
		if (!IS_IN_DEV_GROUP(dev,DEV_NEW_FEATURE_IN_REV) ||
			((GT_DEVICE_REV)dev->revision < GT_REV_1))
	    {
    	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
			return GT_NOT_SUPPORTED;
	    }
	}

    /* translate BOOL to binary */
	if (en)
		data = 0x3;
	else
		data = 0;

    retVal = hwSetPortRegField(dev,hwPort, 0x15, 4, 2, data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
		return retVal;
	}

    return GT_OK;
}


/*******************************************************************************
* gprtSetSAFiltering
*
* DESCRIPTION:
*		This routine set the Source Address(SA) fitering method.
*			GT_SA_FILTERING_DISABLE :
*				no frame will be filtered.
*			GT_SA_DROP_ON_LOCK :
*				discard if SA field is not in the ATU's address database.
*			GT_SA_DROP_ON_UNLOCK :
*				discard if SA field is in the ATU's address database as Static
*				entry with a PortVec of all zeros.
*			GT_SA_DROP_TO_CPU :
*				Ingressing frames will be mapped to the CPU Port if their SA
*				field is in the ATU's address database as Static entry with a
*				PortVec of all zeros. Otherwise, the frames will be discarded
*				if their SA field is not in the ATU's address database or if this
*				port's bit is not set in the PortVec bits for the frame's SA.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_SA_FILTERING structure
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
*******************************************************************************/
GT_STATUS gprtSetSAFiltering
(
    IN GT_QD_DEV  *dev,
    IN GT_LPORT   port,
    IN GT_SA_FILTERING    mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
	GT_U16			data;

    DBG_INFO(("gprtSetSAFiltering Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if device allows to force a flowcontrol disabled */
	if (!IS_IN_DEV_GROUP(dev,DEV_SA_FILTERING))
	{
		DBG_INFO(("GT_NOT_SUPPORTED.\n"));
		return GT_NOT_SUPPORTED;
	}

	data = (GT_U16) mode;

    /* Set the SA Filtering bits.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,14,2,data);
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
* gprtGetSAFiltering
*
* DESCRIPTION:
*		This routine gets the Source Address(SA) fitering method.
*			GT_SA_FILTERING_DISABLE :
*				no frame will be filtered.
*			GT_SA_DROP_ON_LOCK :
*				discard if SA field is not in the ATU's address database.
*			GT_SA_DROP_ON_UNLOCK :
*				discard if SA field is in the ATU's address database as Static
*				entry with a PortVec of all zeros.
*			GT_SA_DROP_TO_CPU :
*				Ingressing frames will be mapped to the CPU Port if their SA
*				field is in the ATU's address database as Static entry with a
*				PortVec of all zeros. Otherwise, the frames will be discarded
*				if their SA field is not in the ATU's address database or if this
*				port's bit is not set in the PortVec bits for the frame's SA.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_SA_FILTERING structure
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS gprtGetSAFiltering
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    OUT GT_SA_FILTERING    *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
	GT_U16			data;

    DBG_INFO(("gprtSetSAFiltering Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if device allows to force a flowcontrol disabled */
	if (!IS_IN_DEV_GROUP(dev,DEV_SA_FILTERING))
	{
		DBG_INFO(("GT_NOT_SUPPORTED.\n"));
		return GT_NOT_SUPPORTED;
	}

    /* Get the SA Filtering bits.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,14,2,&data);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

	*mode = (GT_SA_FILTERING)data;

    return retVal;
}


/*******************************************************************************
* gprtSetARPtoCPU
*
* DESCRIPTION:
*		When ARPtoCPU (or ARP Mirror) is set to GT_TRUE, ARP frames are mirrored
*		to the CPU port.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE, to map ARP frames to CPU Port,
*			   GT_FALSE, otherwise.
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
*******************************************************************************/
GT_STATUS gprtSetARPtoCPU
(
    IN GT_QD_DEV  *dev,
    IN GT_LPORT   port,
    IN GT_BOOL    mode
)
{
	GT_U16			data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetARPtoCPU Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if device allows to force a flowcontrol disabled */
	if (!IS_IN_DEV_GROUP(dev,DEV_ARP_TO_CPU))
	{
		DBG_INFO(("GT_NOT_SUPPORTED.\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set the ARPtoCPU bits.  */
	if (IS_IN_DEV_GROUP(dev,DEV_FASTETH_SWITCH))
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,8,1,data);
	else
	    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,6,1,data);
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
* gprtGetARPtoCPU
*
* DESCRIPTION:
*		When ARPtoCPU (or ARP Mirror) is set to GT_TRUE, ARP frames are mirrored
*		to the CPU port.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE, to map ARP frames to CPU Port,
*			   GT_FALSE, otherwise.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS gprtGetARPtoCPU
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    OUT GT_BOOL    *mode
)
{
	GT_U16			data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetARPtoCPU Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if device supports the feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_ARP_TO_CPU))
	{
		DBG_INFO(("GT_NOT_SUPPORTED.\n"));
		return GT_NOT_SUPPORTED;
	}

    /* Get the ARPtoCPU bits.  */
	if (IS_IN_DEV_GROUP(dev,DEV_FASTETH_SWITCH))
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,8,1,&data);
	else
	    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,6,1,&data);
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
* gprtSetEgressFlood
*
* DESCRIPTION:
*       This routine set Egress Flooding Mode.
*		Frames with unknown DA (Destination Address that is not in ATU database)
*		generally flood out all the ports. This mode can be used to prevent
*		those frames from egressing this port as follows:
*			GT_BLOCK_EGRESS_UNKNOWN
*				do not egress frame with unknown DA (both unicast and multicast)
*			GT_BLOCK_EGRESS_UNKNOWN_MULTICAST
*				do not egress frame with unknown multicast DA
*			GT_BLOCK_EGRESS_UNKNOWN_UNICAST
*				do not egress frame with unknown unicast DA
*			GT_BLOCK_EGRESS_NONE
*				egress all frames with unknown DA
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_EGRESS_FLOOD type
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
GT_STATUS gprtSetEgressFlood
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT     port,
    IN  GT_EGRESS_FLOOD      mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
	GT_U16			data;

    DBG_INFO(("gprtSetEgressFlood Called.\n"));

	/* check if device supports the feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_EGRESS_FLOOD))
	{
        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
        return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	data = (GT_U16) mode;

    /* Set the Egress Flood mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,2,2,data);

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
* gprtGetEgressFlood
*
* DESCRIPTION:
*       This routine gets Egress Flooding Mode.
*		Frames with unknown DA (Destination Address that is not in ATU database)
*		generally flood out all the ports. This mode can be used to prevent
*		those frames from egressing this port as follows:
*			GT_BLOCK_EGRESS_UNKNOWN
*				do not egress frame with unknown DA (both unicast and multicast)
*			GT_BLOCK_EGRESS_UNKNOWN_MULTICAST
*				do not egress frame with unknown multicast DA
*			GT_BLOCK_EGRESS_UNKNOWN_UNICAST
*				do not egress frame with unknown unicast DA
*			GT_BLOCK_EGRESS_NONE
*				egress all frames with unknown DA
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - GT_EGRESS_FLOOD type
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetEgressFlood
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT     port,
    OUT GT_EGRESS_FLOOD      *mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetEgressFlood Called.\n"));

	/* check if device supports the feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_EGRESS_FLOOD))
	{
        DBG_INFO(("GT_NOT_SUPPORTED.\n"));
        return GT_NOT_SUPPORTED;
	}

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* get the Egress Flood mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,2,2,&data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

	*mode = (GT_EGRESS_FLOOD) data;

    return retVal;
}


/*******************************************************************************
* gprtSetPortSched
*
* DESCRIPTION:
*		This routine sets Port Scheduling Mode.
*		When usePortSched is enablied, this mode is used to select the Queue
*		controller's scheduling on the port as follows:
*			GT_PORT_SCHED_WEIGHTED_RRB - use 8,4,2,1 weighted fair scheduling
*			GT_PORT_SCHED_STRICT_PRI - use a strict priority scheme
*
* INPUTS:
*		port - the logical port number
*		mode - GT_PORT_SCHED_MODE enum type
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
GT_STATUS gprtSetPortSched
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_PORT_SCHED_MODE		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetPortSched Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PORT_SCHEDULE|DEV_PORT_MIXED_SCHEDULE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	if (IS_IN_DEV_GROUP(dev,DEV_PORT_MIXED_SCHEDULE))
    {
		switch(mode)
		{
			case GT_PORT_SCHED_WEIGHTED_RRB:
				data = 0;
				break;
			case GT_PORT_SCHED_STRICT_PRI:
				data = 3;
				break;
			case GT_PORT_SCHED_STRICT_PRI3:
				data = 1;
				break;
			case GT_PORT_SCHED_STRICT_PRI2_3:
				data = 2;
				break;
			default:
				return GT_BAD_PARAM;
		}

	    retVal = hwSetPortRegField(dev,hwPort, QD_REG_RATE_CTRL, 12,2,data);

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


	data = mode;

    /* Set the gprtSetPortSched mode.            */
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
* gprtGetPortSched
*
* DESCRIPTION:
*		This routine gets Port Scheduling Mode.
*		When usePortSched is enablied, this mode is used to select the Queue
*		controller's scheduling on the port as follows:
*			GT_PORT_SCHED_WEIGHTED_RRB - use 8,4,2,1 weighted fair scheduling
*			GT_PORT_SCHED_STRICT_PRI - use a strict priority scheme
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		mode - GT_PORT_SCHED_MODE enum type
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetPortSched
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_PORT_SCHED_MODE		*mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetPortSched Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PORT_SCHEDULE|DEV_PORT_MIXED_SCHEDULE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	if (IS_IN_DEV_GROUP(dev,DEV_PORT_MIXED_SCHEDULE))
    {
	    retVal = hwGetPortRegField(dev,hwPort, QD_REG_RATE_CTRL, 12,2,&data);
	    if(retVal != GT_OK)
		{
        	DBG_INFO(("Failed.\n"));
	    	return retVal;
		}

		switch(data)
		{
			case 0:
				*mode = GT_PORT_SCHED_WEIGHTED_RRB;
				break;
			case 1:
				*mode = GT_PORT_SCHED_STRICT_PRI3;
				break;
			case 2:
				*mode = GT_PORT_SCHED_STRICT_PRI2_3;
				break;
			case 3:
				*mode = GT_PORT_SCHED_STRICT_PRI;
				break;
			default:
				return GT_BAD_PARAM;
		}

    	return GT_OK;

    }

    /* Get the gprtGetPortSched mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_ASSOCIATION,14,1,&data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

	*mode = (GT_PORT_SCHED_MODE)data;

    return retVal;
}


/*******************************************************************************
* gprtSetProviderTag
*
* DESCRIPTION:
*		This routine sets Provider Tag which indicates the provider tag (Ether
*		Type) value that needs to be matched to in ingress to determine if a
*		frame is Provider tagged or not.
*
* INPUTS:
*		port - the logical port number
*		tag  - Provider Tag (Ether Type)
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
GT_STATUS gprtSetProviderTag
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_U16		tag
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetProviderTag Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PROVIDER_TAG))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Set Provider Tag.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PROVIDER_TAG, 0, 16, tag);

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
* gprtGetProviderTag
*
* DESCRIPTION:
*		This routine gets Provider Tag which indicates the provider tag (Ether
*		Type) value that needs to be matched to in ingress to determine if a
*		frame is Provider tagged or not.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		tag  - Provider Tag (Ether Type)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetProviderTag
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_U16		*tag
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetProviderTag Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PROVIDER_TAG))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get Provider Tag.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PROVIDER_TAG, 0, 16, tag);

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
* gprtSetPauseLimitOut
*
* DESCRIPTION:
*		Limit the number of continuous Pause refresh frames that can be transmitted
*		from this port. When full duplex Flow Control is enabled on this port,
*		these bits are used to limit the number of Pause refresh frames that can
*		be generated from this port to keep this port's link partner from sending
*		any data.
*		Setting this value to 0 will allow continuous Pause frame refreshes to
*		egress this port as long as this port remains congested.
*		Setting this value to 1 will allow 1 Pause frame to egress from this port
*		for each congestion situation.
*		Setting this value to 2 will allow 2 Pause frames to egress from this port
*		for each congestion situation, etc.
*
* INPUTS:
*		port - the logical port number
*		limit - the max number of Pause refresh frames for each congestion situation
*				( 0 ~ 0xFF)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if limit > 0xFF
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtSetPauseLimitOut
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_U16		limit
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetPauseLimitOut Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PAUSE_LIMIT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	if (limit > 0xFF)
	{
        DBG_INFO(("Bad Parameter\n"));
		return GT_BAD_PARAM;
	}

    /* Set Pause Limit.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_LIMIT_PAUSE_CONTROL, 8, 8, limit);

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
* gprtGetPauseLimitOut
*
* DESCRIPTION:
*		Limit the number of continuous Pause refresh frames that can be transmitted
*		from this port. When full duplex Flow Control is enabled on this port,
*		these bits are used to limit the number of Pause refresh frames that can
*		be generated from this port to keep this port's link partner from sending
*		any data.
*		Setting this value to 0 will allow continuous Pause frame refreshes to
*		egress this port as long as this port remains congested.
*		Setting this value to 1 will allow 1 Pause frame to egress from this port
*		for each congestion situation.
*		Setting this value to 2 will allow 2 Pause frames to egress from this port
*		for each congestion situation, etc.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		limit - the max number of Pause refresh frames for each congestion situation
*				( 0 ~ 0xFF)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetPauseLimitOut
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_U16		*limit
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetPauseLimitOut Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PAUSE_LIMIT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get Pause Limit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_LIMIT_PAUSE_CONTROL, 8, 8, limit);

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
* gprtSetPauseLimitIn
*
* DESCRIPTION:
*		Limit the number of continuous Pause refresh frames that can be received
*		on this port. When a port has flow Control enabled, this value can be
*		used to limit how long this port can be Paused off to prevent a port stall
*		through jamming.
*		When this value is in the range of 0x01 to 0xFF, and a frame is ready to
*		be transmitted out this port, but it cannot be transmitted due to the port
*		being jammed, this limit mechanism starts. The limit mechanism starts
*		counting new Pause refresh frames or counts of 16 consecutive collisions.
*		If the counter reaches the value set through this API, the following event
*		will occur:
*			1) Port's ForceFC is enabled,
*			2) Port's FCValue is cleared to a zero, and
*			3) Jam Limit Interrupt is asserted.
*		This effectively disables Flow Control on the port once the Pause timer
*		expires. If a frame gets transmitted out this port before the counter
*		reaches this limit, then this limit mechanism counter resets back to zero.
*
*		Setting this value to 0 will allow continuous jamming to be received on
*		this port without the Port's ForceFC and FCValue getting modified.
*
*		The modification of Port's ForceFC and FCValue is the only indication that
*		the limit was reached on this port.
*
* INPUTS:
*		port - the logical port number
*		limit - the max number of continuous Pause refresh frames for each trasmition
*				( 0 ~ 0xFF)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if limit > 0xFF
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtSetPauseLimitIn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_U16		limit
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetPauseLimitIn Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PAUSE_LIMIT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	if (limit > 0xFF)
	{
        DBG_INFO(("Bad Parameter\n"));
		return GT_BAD_PARAM;
	}

    /* Set Pause Limit.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_LIMIT_PAUSE_CONTROL, 0, 8, limit);

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
* gprtGetPauseLimitIn
*
* DESCRIPTION:
*		Limit the number of continuous Pause refresh frames that can be received
*		on this port. When a port has flow Control enabled, this value can be
*		used to limit how long this port can be Paused off to prevent a port stall
*		through jamming.
*		When this value is in the range of 0x01 to 0xFF, and a frame is ready to
*		be transmitted out this port, but it cannot be transmitted due to the port
*		being jammed, this limit mechanism starts. The limit mechanism starts
*		counting new Pause refresh frames or counts of 16 consecutive collisions.
*		If the counter reaches the value set through this API, the following event
*		will occur:
*			1) Port's ForceFC is enabled,
*			2) Port's FCValue is cleared to a zero, and
*			3) Jam Limit Interrupt is asserted.
*		This effectively disables Flow Control on the port once the Pause timer
*		expires. If a frame gets transmitted out this port before the counter
*		reaches this limit, then this limit mechanism counter resets back to zero.
*
*		Setting this value to 0 will allow continuous jamming to be received on
*		this port without the Port's ForceFC and FCValue getting modified.
*
*		The modification of Port's ForceFC and FCValue is the only indication that
*		the limit was reached on this port.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		limit - the max number of continuous Pause refresh frames for each trasmition
*				( 0 ~ 0xFF)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetPauseLimitIn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_U16		*limit
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetPauseLimitIn Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PAUSE_LIMIT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get Pause Limit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_LIMIT_PAUSE_CONTROL, 0, 8, limit);

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
* gprtSetFrameMode
*
* DESCRIPTION:
*		Frmae Mode is used to define the expected Ingress and the generated Egress
*		tagging frame format for this port as follows:
*			GT_FRAME_MODE_NORMAL -
*				Normal Network mode uses industry standard IEEE 802.3ac Tagged or
*				Untagged frames. Tagged frames use an Ether Type of 0x8100.
*			GT_FRAME_MODE_DSA -
*				DSA mode uses a Marvell defined tagged frame format for
*				Chip-to-Chip and Chip-to-CPU connections.
*			GT_FRAME_MODE_PROVIDER -
*				Provider mode uses user definable Ether Types per port
*				(see gprtSetPortEType/gprtGetPortEType API).
*			GT_FRAME_MODE_ETHER_TYPE_DSA -
*				Ether Type DSA mode uses standard Marvell DSA Tagged frame info
*				flowing a user definable Ether Type. This mode allows the mixture
*				of Normal Network frames with DSA Tagged frames and is useful to
*				be used on ports that connect to a CPU.
*
* INPUTS:
*		port - the logical port number
*		mode - GT_FRAME_MODE type
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_BAD_PARAM - if mode is unknown
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtSetFrameMode
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_FRAME_MODE	mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetFrameMode Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FRAME_MODE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    switch (mode)
    {
		case GT_FRAME_MODE_NORMAL:
		case GT_FRAME_MODE_DSA:
		case GT_FRAME_MODE_PROVIDER:
		case GT_FRAME_MODE_ETHER_TYPE_DSA:
			break;
		default:
	        DBG_INFO(("Bad Parameter\n"));
			return GT_BAD_PARAM;
	}

    /* Set Frame Mode.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL, 8, 2, (GT_U16)mode);

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
* gprtGetFrameMode
*
* DESCRIPTION:
*		Frmae Mode is used to define the expected Ingress and the generated Egress
*		tagging frame format for this port as follows:
*			GT_FRAME_MODE_NORMAL -
*				Normal Network mode uses industry standard IEEE 802.3ac Tagged or
*				Untagged frames. Tagged frames use an Ether Type of 0x8100.
*			GT_FRAME_MODE_DSA -
*				DSA mode uses a Marvell defined tagged frame format for
*				Chip-to-Chip and Chip-to-CPU connections.
*			GT_FRAME_MODE_PROVIDER -
*				Provider mode uses user definable Ether Types per port
*				(see gprtSetPortEType/gprtGetPortEType API).
*			GT_FRAME_MODE_ETHER_TYPE_DSA -
*				Ether Type DSA mode uses standard Marvell DSA Tagged frame info
*				flowing a user definable Ether Type. This mode allows the mixture
*				of Normal Network frames with DSA Tagged frames and is useful to
*				be used on ports that connect to a CPU.
*
* INPUTS:
*		port - the logical port number
*
* OUTPUTS:
*		mode - GT_FRAME_MODE type
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetFrameMode
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_FRAME_MODE	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetFrameMode Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FRAME_MODE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get Pause Limit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL, 8, 2, &data);
    *mode = data;
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
* gprtSetHoldAt1
*
* DESCRIPTION:
*		Hold Aging ATU Entries at an Entry State value of 1. When this feature
*		is set to GT_TRUE, ATU entries associated with this port will age down
*		to an Entry State of 0x1, but will not go to 0x0 (0x0 would purge the
*		entry)
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to hold aging ATU entry with Entry State of 1,
*			   GT_FALSE otherwise
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
GT_STATUS gprtSetHoldAt1
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetHoldAt1 Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if device allows to force a flowcontrol disabled */
	if (!IS_IN_DEV_GROUP(dev,DEV_AGE_HOLD))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set HoldAt1 */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_ASSOCIATION, 15, 1, data);

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
* gprtGetHoldAt1
*
* DESCRIPTION:
*		Hold Aging ATU Entries at an Entry State value of 1. When this feature
*		is set to GT_TRUE, ATU entries associated with this port will age down
*		to an Entry State of 0x1, but will not go to 0x0 (0x0 would purge the
*		entry)
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE to hold aging ATU entry with Entry State of 1,
*			   GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetHoldAt1
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL  	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetHoldAt1 Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if device allows to force a flowcontrol disabled */
	if (!IS_IN_DEV_GROUP(dev,DEV_AGE_HOLD))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get HoldAt1 */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_ASSOCIATION, 15, 1, &data);

    BIT_2_BOOL(data, *mode);

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
* gprtSetIntOnAgeOut
*
* DESCRIPTION:
*		Interrupt on Age Out. When aging is enabled, all non-static address
*		entries in the ATU's address database are periodically aged.
*		When this feature is set to GT_TRUE and an entry associated with this
*		port is aged out, an AgeOutViolation will be captured for that entry.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to enable AgeOutViloation interrupt
*			   GT_FALSE otherwise
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
GT_STATUS gprtSetIntOnAgeOut
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetIntOnAgeOut Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if device allows to force a flowcontrol disabled */
	if (!IS_IN_DEV_GROUP(dev,DEV_AGE_OUT_INT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set Age Out Interrupt Enable Mode. */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_ASSOCIATION, 14, 1, data);

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
* gprtGetIntOnAgeOut
*
* DESCRIPTION:
*		Interrupt on Age Out. When aging is enabled, all non-static address
*		entries in the ATU's address database are periodically aged.
*		When this feature is set to GT_TRUE and an entry associated with this
*		port is aged out, an AgeOutViolation will be captured for that entry.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE to enable AgeOutViloation interrupt
*			   GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetIntOnAgeOut
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL  	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetIntOnAgeOut Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if device allows to force a flowcontrol disabled */
	if (!IS_IN_DEV_GROUP(dev,DEV_AGE_OUT_INT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get IntOnAgeOut */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_ASSOCIATION, 14, 1, &data);

    BIT_2_BOOL(data, *mode);

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
* gprtSetRefreshLocked
*
* DESCRIPTION:
*		Auto Refresh known addresses when port is Locked. Already known addresses
*		will be auto refreshed when this feature is enabled. When this feature
*		is disabled, auto refreshing will not occur on Locked ports.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to enable Auto Refresh known addresses on locked port
*			   GT_FALSE otherwise
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
GT_STATUS gprtSetRefreshLocked
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetRefreshLocked Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if device allows to force a flowcontrol disabled */
	if (!IS_IN_DEV_GROUP(dev,DEV_AUTO_REFRESH_LOCKED))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set RefreshLocked */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_ASSOCIATION, 11, 1, data);

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
* gprtGetRefreshLocked
*
* DESCRIPTION:
*		Auto Refresh known addresses when port is Locked. Already known addresses
*		will be auto refreshed when this feature is enabled. When this feature
*		is disabled, auto refreshing will not occur on Locked ports.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE to enable Auto Refresh known addresses on locked port
*			   GT_FALSE otherwise
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetRefreshLocked
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL  	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetRefreshLocked Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if device allows to force a flowcontrol disabled */
	if (!IS_IN_DEV_GROUP(dev,DEV_AUTO_REFRESH_LOCKED))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get RefreshLocked */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_ASSOCIATION, 11, 1, &data);

    BIT_2_BOOL(data, *mode);

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
* gprtSetPortEType
*
* DESCRIPTION:
*		This routine sets the port's special Ether Type. This Ether Type is used
*		for Policy (see gprtSetPolicy API) and FrameMode (see gprtSetFrameMode API).
*
* INPUTS:
*		port  - the logical port number
*		etype - port's special ether type
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
GT_STATUS gprtSetPortEType
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_ETYPE		etype
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetPortEType Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PORT_ETYPE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    data = (GT_U16)etype;

    /* Set the EtherType.            */
    retVal = hwWritePortReg(dev,hwPort, QD_REG_PORT_ETH_TYPE,data);

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
* gprtGetPortEType
*
* DESCRIPTION:
*		This routine retrieves the port's special Ether Type. This Ether Type is used
*		for Policy (see gprtSetPolicy API) and FrameMode (see gprtSetFrameMode API).
*
* INPUTS:
*		port  - the logical port number
*
* OUTPUTS:
*		etype - port's special ether type
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gprtGetPortEType
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_ETYPE	*etype
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetPortEType Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PORT_ETYPE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the EtherType.            */
    retVal = hwReadPortReg(dev,hwPort, QD_REG_PORT_ETH_TYPE,&data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

	*etype = (GT_ETYPE) data;

    return retVal;
}


/*******************************************************************************
* gsysSetJumboMode
*
* DESCRIPTION:
*       This routine Set the max frame size allowed to be received and transmitted
*		from or to a given port.
*
* INPUTS:
*		port - the logical port number
*       mode - GT_JUMBO_MODE (1522, 2048, or 10240)
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
*
*******************************************************************************/
GT_STATUS gsysSetJumboMode
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_JUMBO_MODE   mode
)
{
	GT_STATUS       retVal;         /* Functions return value.      */
	GT_U8           hwPort;         /* the physical port number     */

	DBG_INFO(("gsysSetJumboMode Called.\n"));

	if (mode > GT_JUMBO_MODE_10240)
	{
		DBG_INFO(("Bad Parameter\n"));
		return GT_BAD_PARAM;
	}

	if (!IS_IN_DEV_GROUP(dev,DEV_JUMBO_MODE))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	/* Set the Jumbo Fram Size bit.               */
	retVal = hwSetPortRegField(dev,hwPort,QD_REG_PORT_CONTROL2,12,2,mode );
	if(retVal != GT_OK)
	{
		DBG_INFO(("Failed.\n"));
		return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;
}

/*******************************************************************************
* gsysGetJumboMode
*
* DESCRIPTION:
*       This routine gets the max frame size allowed to be received and transmitted
*		from or to a given port.
*
* INPUTS:
*		port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_JUMBO_MODE (1522, 2048, or 10240)
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gsysGetJumboMode
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_JUMBO_MODE   *mode
)
{
	GT_STATUS       retVal;         /* Functions return value.      */
	GT_U8           hwPort;         /* the physical port number     */
	GT_U16          data;           /* to keep the read valve       */

	DBG_INFO(("gsysGetJumboMode Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_JUMBO_MODE))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	/* translate LPORT to hardware port */
	hwPort = GT_LPORT_2_PORT(port);

	/* Get Jumbo Frame Mode.            */
	retVal = hwGetPortRegField(dev,hwPort,QD_REG_PORT_CONTROL2,12,2,&data );

	*mode = (GT_JUMBO_MODE)data;

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
* gprtGetSwitchReg
*
* DESCRIPTION:
*       This routine reads Switch Port Registers.
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
GT_STATUS gprtGetSwitchReg
(
    IN  GT_QD_DEV    *dev,
    IN  GT_LPORT     port,
    IN  GT_U32	     regAddr,
    OUT GT_U16	     *data
)
{
    GT_U16          u16Data;           /* The register's read data.    */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtGetSwitchReg Called.\n"));

    hwPort = GT_LPORT_2_PORT(port);

    /* Get Phy Register. */
    if(hwReadPortReg(dev,hwPort,(GT_U8)regAddr,&u16Data) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

	*data = u16Data;

    return GT_OK;
}

/*******************************************************************************
* gprtSetSwitchReg
*
* DESCRIPTION:
*       This routine writes Switch Port Registers.
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
GT_STATUS gprtSetSwitchReg
(
    IN  GT_QD_DEV		*dev,
    IN  GT_LPORT		port,
    IN  GT_U32			regAddr,
    IN  GT_U16			data
)
{
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gprtSetSwitchReg Called.\n"));

    hwPort = GT_LPORT_2_PORT(port);

    /* Get the Scheduling bit.              */
    if(hwWritePortReg(dev,hwPort,(GT_U8)regAddr,data) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

	return GT_OK;
}


/*******************************************************************************
* gprtGetGlobalReg
*
* DESCRIPTION:
*       This routine reads Switch Global Registers.
*
* INPUTS:
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
GT_STATUS gprtGetGlobalReg
(
    IN  GT_QD_DEV    *dev,
    IN  GT_U32	     regAddr,
    OUT GT_U16	     *data
)
{
    GT_U16          u16Data;           /* The register's read data.    */

    DBG_INFO(("gprtGetGlobalReg Called.\n"));

    /* Get Phy Register. */
    if(hwReadGlobalReg(dev,(GT_U8)regAddr,&u16Data) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

	*data = u16Data;

    return GT_OK;
}

/*******************************************************************************
* gprtSetGlobalReg
*
* DESCRIPTION:
*       This routine writes Switch Global Registers.
*
* INPUTS:
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
GT_STATUS gprtSetGlobalReg
(
    IN  GT_QD_DEV		*dev,
    IN  GT_U32			regAddr,
    IN  GT_U16			data
)
{
    DBG_INFO(("gprtSetGlobalReg Called.\n"));

    /* Get the Scheduling bit.              */
    if(hwWriteGlobalReg(dev,(GT_U8)regAddr,data) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

	return GT_OK;
}

/*******************************************************************************
* gprtGetGlobal2Reg
*
* DESCRIPTION:
*       This routine reads Switch Global 2 Registers.
*
* INPUTS:
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
GT_STATUS gprtGetGlobal2Reg
(
    IN  GT_QD_DEV    *dev,
    IN  GT_U32	     regAddr,
    OUT GT_U16	     *data
)
{
    GT_U16          u16Data;           /* The register's read data.    */

    DBG_INFO(("gprtGetGlobal2Reg Called.\n"));

    /* Get Phy Register. */
    if(hwReadGlobal2Reg(dev,(GT_U8)regAddr,&u16Data) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

	*data = u16Data;

    return GT_OK;
}

/*******************************************************************************
* gprtSetGlobal2Reg
*
* DESCRIPTION:
*       This routine writes Switch Global2 Registers.
*
* INPUTS:
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
GT_STATUS gprtSetGlobal2Reg
(
    IN  GT_QD_DEV		*dev,
    IN  GT_U32			regAddr,
    IN  GT_U16			data
)
{
    DBG_INFO(("gprtSetGlobal2Reg Called.\n"));

    /* Get the Scheduling bit.              */
    if(hwWriteGlobal2Reg(dev,(GT_U8)regAddr,data) != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return GT_FAIL;
    }

	return GT_OK;
}
