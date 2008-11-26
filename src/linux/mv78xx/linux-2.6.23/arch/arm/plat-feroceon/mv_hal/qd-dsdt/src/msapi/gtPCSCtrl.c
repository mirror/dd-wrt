#include <Copyright.h>

/********************************************************************************
* gtPCSCtrl.c
*
* DESCRIPTION:
*       API implementation for 1000BASE-X PCS block register access.
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
* gpcsGetCommaDet
*
* DESCRIPTION:
*		This routine retrieves Comma Detection status in PCS
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE for Comma Detected or GT_FALSE otherwise
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
GT_STATUS gpcsGetCommaDet
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsGetCommaDet Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PCS))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	if (IS_IN_DEV_GROUP(dev,DEV_PCS_LINK))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the CommaDet bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,15,1,&data);

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
* gpcsGetPCSLink
*
* DESCRIPTION:
*		This routine retrieves Link up status in PCS
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE for Comma Detected or GT_FALSE otherwise
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
GT_STATUS gpcsGetPCSLink
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsGetPCSLink Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PCS))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PCS_LINK))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!DOES_DEVPORT_SUPPORT_PCS(dev,hwPort))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the PCS Link bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,15,1,&data);

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
* gpcsGetSyncOK
*
* DESCRIPTION:
*		This routine retrieves SynOK bit. It is set to a one when the PCS has
*		detected a few comma patterns and is synchronized with its peer PCS
*		layer.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if synchronized or GT_FALSE otherwise
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
GT_STATUS gpcsGetSyncOK
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsGetSyncOK Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PCS))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given port supports PCS */
	if (!DOES_DEVPORT_SUPPORT_PCS(dev,hwPort))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the SyncOK bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,14,1,&data);

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
* gpcsGetSyncFail
*
* DESCRIPTION:
*		This routine retrieves SynFail bit.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if synchronizaion failed or GT_FALSE otherwise
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
GT_STATUS gpcsGetSyncFail
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsGetSyncFail Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PCS))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given port supports PCS */
	if (!DOES_DEVPORT_SUPPORT_PCS(dev,hwPort))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the SyncFail bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,13,1,&data);

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
* gpcsGetAnBypassed
*
* DESCRIPTION:
*		This routine retrieves Inband Auto-Negotiation bypass status.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if AN is bypassed or GT_FALSE otherwise
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
GT_STATUS gpcsGetAnBypassed
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsGetAnBypassed Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PCS))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given port supports PCS */
	if (!DOES_DEVPORT_SUPPORT_PCS(dev,hwPort))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the AnBypassed bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,12,1,&data);

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
* gpcsGetAnBypassMode
*
* DESCRIPTION:
*		This routine retrieves Enable mode of Inband Auto-Negotiation bypass.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE if AN bypass is enabled or GT_FALSE otherwise
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
GT_STATUS gpcsGetAnBypassMode
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsGetAnBypassMode Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PCS))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given port supports PCS */
	if (!DOES_DEVPORT_SUPPORT_PCS(dev,hwPort))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the AnBypass bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,11,1,&data);

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
* gpcsSetAnBypassMode
*
* DESCRIPTION:
*		This routine retrieves Enable mode of Inband Auto-Negotiation bypass.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to enable AN bypass mode or GT_FALSE otherwise
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
GT_STATUS gpcsSetAnBypassMode
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL  	mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsSetAnBypassMode Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PCS))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given port supports PCS */
	if (!DOES_DEVPORT_SUPPORT_PCS(dev,hwPort))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the AnBypass bit.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,11,1,data);

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
* gpcsGetPCSAnEn
*
* DESCRIPTION:
*		This routine retrieves Enable mode of PCS Inband Auto-Negotiation.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE if PCS AN is enabled or GT_FALSE otherwise
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
GT_STATUS gpcsGetPCSAnEn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsGetPCSAnEn Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PCS))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given port supports PCS */
	if (!DOES_DEVPORT_SUPPORT_PCS(dev,hwPort))
    {
        if (!IS_IN_DEV_GROUP(dev, DEV_INTERNAL_GPHY))
        {
            DBG_INFO(("GT_NOT_SUPPORTED\n"));
	    	return GT_NOT_SUPPORTED;
        }

        if ((hwPort < 4) || (hwPort > 7))
        {
            DBG_INFO(("GT_NOT_SUPPORTED\n"));
	    	return GT_NOT_SUPPORTED;
        }
    }

    /* Get the PCSAnEn bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,10,1,&data);

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
* gpcsSetPCSAnEn
*
* DESCRIPTION:
*		This routine sets Enable mode of PCS Inband Auto-Negotiation.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE to enable PCS AN mode or GT_FALSE otherwise
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
GT_STATUS gpcsSetPCSAnEn
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_BOOL  	mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsSetPCSAnEn Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PCS))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given port supports PCS */
	if (!DOES_DEVPORT_SUPPORT_PCS(dev,hwPort))
    {
        if (!IS_IN_DEV_GROUP(dev, DEV_INTERNAL_GPHY))
        {
            DBG_INFO(("GT_NOT_SUPPORTED\n"));
	    	return GT_NOT_SUPPORTED;
        }

        if ((hwPort < 4) || (hwPort > 7))
        {
            DBG_INFO(("GT_NOT_SUPPORTED\n"));
	    	return GT_NOT_SUPPORTED;
        }
    }

    /* Get the PCSAnEn bit.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,10,1,data);

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
* gpcsSetRestartPCSAn
*
* DESCRIPTION:
*		This routine restarts PCS Inband Auto-Negotiation.
*
* INPUTS:
*		port - the logical port number.
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
GT_STATUS gpcsSetRestartPCSAn
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsSetRestartPCSAn Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PCS))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    data = 1;	/* to set RestartPCSAn bit */

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given port supports PCS */
	if (!DOES_DEVPORT_SUPPORT_PCS(dev,hwPort))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the RestartPCSAn bit.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,9,1,data);

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
* gpcsGetPCSAnDone
*
* DESCRIPTION:
*		This routine retrieves completion information of PCS Auto-Negotiation.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE if PCS AN is done or never done
*			    GT_FALSE otherwise
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
GT_STATUS gpcsGetPCSAnDone
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsGetPCSAnDone Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PCS))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given port supports PCS */
	if (!DOES_DEVPORT_SUPPORT_PCS(dev,hwPort))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the PCSAnDone bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,8,1,&data);

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
* gpcsSetFCValue
*
* DESCRIPTION:
*		This routine sets Flow Control's force value
*
* INPUTS:
*		port - the logical port number.
*		state - GT_TRUE to force flow control enabled, GT_FALSE otherwise
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
GT_STATUS gpcsSetFCValue
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN	GT_BOOL		state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsSetFCValue Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_FC_WITH_VALUE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(state, data);

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Set the FCValue bit.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,7,1,data);

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
* gpcsGetFCValue
*
* DESCRIPTION:
*		This routine retrieves Flow Control Value which will be used for Forcing
*		Flow Control enabled or disabled.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if FC Force value is one (flow control enabled)
*			     GT_FALSE otherwise (flow control disabled)
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
GT_STATUS gpcsGetFCValue
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsGetFCValue Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_FC_WITH_VALUE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the FCValue bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,7,1,&data);

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
* gpcsSetForcedFC
*
* DESCRIPTION:
*		This routine forces Flow Control. If FCValue is set to one, calling this
*		routine with GT_TRUE will force Flow Control to be enabled.
*
* INPUTS:
*		port - the logical port number.
*		state - GT_TRUE to force flow control (enable or disable), GT_FALSE otherwise
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
GT_STATUS gpcsSetForcedFC
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN	GT_BOOL		state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsSetForcedFC Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_FC_WITH_VALUE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(state, data);

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the ForcedFC bit.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,6,1,data);

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
* gpcsGetForcedFC
*
* DESCRIPTION:
*		This routine retrieves Forced Flow Control bit
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if ForcedFC bit is one,
*			     GT_FALSE otherwise
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
GT_STATUS gpcsGetForcedFC
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsGetForcedLink Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_FC_WITH_VALUE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the ForcedLink bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,6,1,&data);

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
* gpcsSetLinkValue
*
* DESCRIPTION:
*		This routine sets Link's force value
*
* INPUTS:
*		port - the logical port number.
*		state - GT_TRUE to force link up, GT_FALSE otherwise
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
GT_STATUS gpcsSetLinkValue
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN	GT_BOOL		state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsSetLinkValue Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_FORCE_WITH_VALUE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(state, data);

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the LinkValue bit.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,5,1,data);

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
* gpcsGetLinkValue
*
* DESCRIPTION:
*		This routine retrieves Link Value which will be used for Forcing Link
*		up or down.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if Link Force value is one (link up)
*			     GT_FALSE otherwise (link down)
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
GT_STATUS gpcsGetLinkValue
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsGetLinkValue Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_FORCE_WITH_VALUE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the LinkValue bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,5,1,&data);

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
* gpcsSetForcedLink
*
* DESCRIPTION:
*		This routine forces Link. If LinkValue is set to one, calling this
*		routine with GT_TRUE will force Link to be up.
*
* INPUTS:
*		port - the logical port number.
*		state - GT_TRUE to force link (up or down), GT_FALSE otherwise
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
GT_STATUS gpcsSetForcedLink
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN	GT_BOOL		state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsSetForcedLink Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_FORCE_WITH_VALUE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(state, data);

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the ForcedLink bit.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,4,1,data);

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
* gpcsGetForcedLink
*
* DESCRIPTION:
*		This routine retrieves Forced Link bit
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if ForcedLink bit is one,
*			     GT_FALSE otherwise
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
GT_STATUS gpcsGetForcedLink
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsGetForcedLink Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_FORCE_WITH_VALUE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the ForcedLink bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,4,1,&data);

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
* gpcsSetDpxValue
*
* DESCRIPTION:
*		This routine sets Duplex's Forced value. This function needs to be
*		called prior to gpcsSetForcedDpx.
*
* INPUTS:
*		port - the logical port number.
*		state - GT_TRUE to force full duplex, GT_FALSE otherwise
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
GT_STATUS gpcsSetDpxValue
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN	GT_BOOL		state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsSetDpxValue Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_FORCE_WITH_VALUE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(state, data);

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the DpxValue bit.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,3,1,data);

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
* gpcsGetDpxValue
*
* DESCRIPTION:
*		This routine retrieves Duplex's Forced value
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if Duplex's Forced value is set to Full duplex,
*			     GT_FALSE otherwise
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
GT_STATUS gpcsGetDpxValue
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsGetForcedLink Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_FORCE_WITH_VALUE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the DpxValue bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,3,1,&data);

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
* gpcsSetForcedDpx
*
* DESCRIPTION:
*		This routine forces duplex mode. If DpxValue is set to one, calling this
*		routine with GT_TRUE will force duplex mode to be full duplex.
*
* INPUTS:
*		port - the logical port number.
*		state - GT_TRUE to force duplex mode, GT_FALSE otherwise
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
GT_STATUS gpcsSetForcedDpx
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN	GT_BOOL		state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsSetForcedDpx Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_FORCE_WITH_VALUE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(state, data);

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the ForcedDpx bit.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,2,1,data);

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
* gpcsGetForcedDpx
*
* DESCRIPTION:
*		This routine retrieves Forced Duplex.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_TRUE if ForcedDpx bit is one,
*			     GT_FALSE otherwise
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
GT_STATUS gpcsGetForcedDpx
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_BOOL  	*state
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsGetForcedDpx Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_FORCE_WITH_VALUE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the ForcedDpx bit.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,2,1,&data);

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
* gpcsSetForceSpeed
*
* DESCRIPTION:
*		This routine forces Speed.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_PORT_FORCED_SPEED_MODE (10, 100, 1000, or no force speed)
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
GT_STATUS gpcsSetForceSpeed
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_PORT_FORCED_SPEED_MODE  mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsSetForceSpeed Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_FORCE_WITH_VALUE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Set the Force Speed bits.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,0,2,mode);

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
* gpcsGetForceSpeed
*
* DESCRIPTION:
*		This routine retrieves Force Speed value
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		state - GT_PORT_FORCED_SPEED_MODE (10, 100, 1000, or no force speed)
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
GT_STATUS gpcsGetForceSpeed
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	OUT GT_PORT_FORCED_SPEED_MODE   *mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gpcsGetForceSpeed Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_FORCE_WITH_VALUE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* Get the ForceSpeed bits.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PCS_CONTROL,0,2,&data);

    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	}
    else
	{
        DBG_INFO(("OK.\n"));
	}

	*mode = data;

    /* return */
    return retVal;
}






