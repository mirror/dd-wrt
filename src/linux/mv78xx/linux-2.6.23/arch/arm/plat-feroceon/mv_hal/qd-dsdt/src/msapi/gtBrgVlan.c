#include <Copyright.h>

/********************************************************************************
* gtBrgVlan.c
*
* DESCRIPTION:
*       API definitions to handle port-based vlan configuration.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*******************************************************************************/

#include <msApi.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>

/*******************************************************************************
* gprtSetEgressMode
*
* DESCRIPTION:
*       This routine set the egress mode.
*
* INPUTS:
*       port - the logical port number.
*       mode - the egress mode.
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
GT_STATUS gprtSetEgressMode
(
    IN GT_QD_DEV       *dev,
    IN GT_LPORT        port,
    IN GT_EGRESS_MODE  mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("gprtSetEgressMode Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_TAGGING)) != GT_OK )
      return retVal;

    switch (mode)
    {
        case (GT_UNMODIFY_EGRESS):
            data = 0;
            break;

        case (GT_TAGGED_EGRESS):
            data = 2;
            break;

        case (GT_UNTAGGED_EGRESS):
            data = 1;
            break;

        case (GT_ADD_TAG):
			if(!IS_IN_DEV_GROUP(dev,DEV_EGRESS_DOUBLE_TAGGING))
			{
		        DBG_INFO(("GT_NOT_SUPPORTED\n"));
				return GT_NOT_SUPPORTED;
			}
            data = 3;
            break;
        default:
            DBG_INFO(("Failed.\n"));
            return GT_FAIL;
    }

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_PORT_CONTROL,12,2,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gprtGetEgressMode
*
* DESCRIPTION:
*       This routine get the egress mode.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - the egress mode.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetEgressMode
(
    IN GT_QD_DEV       *dev,
    IN  GT_LPORT        port,
    OUT GT_EGRESS_MODE  *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    if(mode == NULL)
        return GT_BAD_PARAM;

    DBG_INFO(("gprtGetEgressMode Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    retVal = hwGetPortRegField(dev,phyPort,QD_REG_PORT_CONTROL,12,2,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    switch (data)
    {
        case (0):
            *mode = GT_UNMODIFY_EGRESS;
            break;

        case (2):
            *mode = GT_TAGGED_EGRESS;
            break;

        case (1):
            *mode = GT_UNTAGGED_EGRESS;
            break;

        case (3):
            *mode = GT_ADD_TAG;
            break;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gprtSetVlanTunnel
*
* DESCRIPTION:
*       This routine sets the vlan tunnel mode.
*
* INPUTS:
*       port - the logical port number.
*       mode - the vlan tunnel mode.
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
GT_STATUS gprtSetVlanTunnel
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT  port,
    IN GT_BOOL   mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("gprtSetVlanTunnel Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);
    BOOL_2_BIT(mode,data);

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_PORT_CONTROL,7,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gprtGetVlanTunnel
*
* DESCRIPTION:
*       This routine get the vlan tunnel mode.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - the vlan tunnel mode..
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetVlanTunnel
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("gprtGetVlanTunnel Called.\n"));
    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    phyPort = GT_LPORT_2_PORT(port);

    retVal = hwGetPortRegField(dev,phyPort,QD_REG_PORT_CONTROL,7,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*mode);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gvlnSetPortVlanPorts
*
* DESCRIPTION:
*       This routine sets the port VLAN group port membership list.
*
* INPUTS:
*       port        - logical port number to set.
*       memPorts    - array of logical ports in the same vlan.
*       memPortsLen - number of members in memPorts array
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortVlanPorts
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT  port,
    IN GT_LPORT  memPorts[],
    IN GT_U8     memPortsLen
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    GT_U8           phyPort;        /* Physical port.               */
    GT_U8           i;

    DBG_INFO(("gvlnSetPortVlanPorts Called.\n"));
    if(memPorts == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    phyPort = GT_LPORT_2_PORT(port);
    data = 0;

	if(memPortsLen > dev->numOfPorts)
    {
        DBG_INFO(("Failed (PortsLen Too Big).\n"));
        return GT_BAD_PARAM;
    }

    for(i = 0; i < memPortsLen; i++)
        data |= (1 << GT_LPORT_2_PORT(memPorts[i]));

    /* numOfPorts = 3 for fullsail, = 10 for octane, = 7 for others */
    retVal = hwSetPortRegField(dev,phyPort,QD_REG_PORT_VLAN_MAP,0,dev->maxPorts,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gvlnGetPortVlanPorts
*
* DESCRIPTION:
*       This routine gets the port VLAN group port membership list.
*
* INPUTS:
*       port        - logical port number to set.
*
* OUTPUTS:
*       memPorts    - array of logical ports in the same vlan.
*       memPortsLen - number of members in memPorts array
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortVlanPorts
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_LPORT memPorts[],
    OUT GT_U8    *memPortsLen
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */
    GT_U8           i;

    DBG_INFO(("gvlnGetPortVlanPorts Called.\n"));
    if((memPorts == NULL) || (memPortsLen == NULL))
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    phyPort = GT_LPORT_2_PORT(port);

    /* memPortsLen = 3 for fullsail, =7 for others */
    retVal = hwGetPortRegField(dev,phyPort,QD_REG_PORT_VLAN_MAP,0,dev->maxPorts,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    i = 0;
    for(phyPort = 0; phyPort < dev->maxPorts; phyPort++)
    {
		if(!GT_IS_PORT_SET(dev->validPortVec, phyPort))
			continue;

        if(((1 << phyPort) & data) != 0)
        {
            memPorts[i] = GT_PORT_2_LPORT(phyPort);
            i++;
        }
    }
    *memPortsLen = i;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gvlnSetPortUserPriLsb
*
* DESCRIPTION:
*       This routine Set the user priority (VPT) LSB bit, to be added to the
*       user priority on the egress.
*
* INPUTS:
*       port       - logical port number to set.
*       userPriLsb - GT_TRUE for 1, GT_FALSE for 0.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortUserPriLsb
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT  port,
    IN GT_BOOL   userPriLsb
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("gvlnSetPortUserPriLsb Called.\n"));

	/* Gigabit Switch does not support this status. */
	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH|DEV_ENHANCED_FE_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    phyPort = GT_LPORT_2_PORT(port);
    BOOL_2_BIT(userPriLsb,data);

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_PVID,13,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gvlnGetPortUserPriLsb
*
* DESCRIPTION:
*       This routine gets the user priority (VPT) LSB bit.
*
* INPUTS:
*       port       - logical port number to set.
*
* OUTPUTS:
*       userPriLsb - GT_TRUE for 1, GT_FALSE for 0.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortUserPriLsb
(
    IN GT_QD_DEV    *dev,
    IN  GT_LPORT    port,
    OUT GT_BOOL     *userPriLsb
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("gvlnGetPortUserPriLsb Called.\n"));

	/* Gigabit Switch does not support this status. */
	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH|DEV_ENHANCED_FE_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    if(userPriLsb == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    phyPort = GT_LPORT_2_PORT(port);

    retVal = hwGetPortRegField(dev,phyPort,QD_REG_PVID,13,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*userPriLsb);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gvlnSetPortVid
*
* DESCRIPTION:
*       This routine Set the port default vlan id.
*
* INPUTS:
*       port - logical port number to set.
*       vid  - the port vlan id.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortVid
(
    IN GT_QD_DEV    *dev,
    IN GT_LPORT     port,
    IN GT_U16       vid
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("gvlnSetPortVid Called.\n"));
    phyPort = GT_LPORT_2_PORT(port);

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_PVID,0,12, vid);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gvlnGetPortVid
*
* DESCRIPTION:
*       This routine Get the port default vlan id.
*
* INPUTS:
*       port - logical port number to set.
*
* OUTPUTS:
*       vid  - the port vlan id.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortVid
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_U16   *vid
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("gvlnGetPortVid Called.\n"));
    if(vid == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    phyPort = GT_LPORT_2_PORT(port);

    retVal = hwGetPortRegField(dev,phyPort,QD_REG_PVID,0,12, &data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *vid = data;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gvlnSetPortVlanDBNum
*
* DESCRIPTION:
*       This routine sets the port's default VLAN database number (DBNum or
*		FID, Forwarding Information Database).
*
* INPUTS:
*       port	- logical port number to set.
*       DBNum 	- database number for this port (or FID)
*
* OUTPUTS:
*       None.
*
* RETURNS:IN GT_INGRESS_MODE mode
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortVlanDBNum
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT  port,
    IN GT_U32    DBNum
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("gvlnSetPortVlanDBNum Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

	if(IS_IN_DEV_GROUP(dev,DEV_DBNUM_4096))
	{
		if(DBNum > 4095)
		{
			return GT_BAD_PARAM;
		}
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_PORT_CONTROL1,0,8,(GT_U16)((DBNum & 0xFF0) >> 4));
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_PORT_VLAN_MAP,12,4,(GT_U16)(DBNum & 0x000F));
	}
	else if(IS_IN_DEV_GROUP(dev,DEV_DBNUM_256))
	{
		if(DBNum > 255)
		{
			return GT_BAD_PARAM;
		}
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_PORT_CONTROL1,0,4,(GT_U16)((DBNum & 0xF0) >> 4));
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_PORT_VLAN_MAP,12,4,(GT_U16)(DBNum & 0x0F));
	}
	else if(IS_IN_DEV_GROUP(dev,DEV_DBNUM_64))
	{
		if(DBNum > 63)
		{
			return GT_BAD_PARAM;
		}
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_PORT_VLAN_MAP,6,2,(GT_U16)((DBNum & 0x30) >> 4));
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_PORT_VLAN_MAP,12,4,(GT_U16)(DBNum & 0x0F));
	}
	else
	{
		if(DBNum > 15)
		{
			return GT_BAD_PARAM;
		}
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_PORT_VLAN_MAP,12,4,(GT_U16)(DBNum & 0x0F));
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
* gvlnGetPortVlanDBNum
*
* DESCRIPTION:
*       This routine gets the port's default VLAN database number (DBNum or
*		FID, Forwarding Information Database).
*
* INPUTS:
*       port 	- logical port number to get.
*
* OUTPUTS:
*       DBNum 	- database number for this port (or FID)
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortVlanDBNum
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_U32   *DBNum
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data,dataH;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("gvlnGetPortVlanDBNum Called.\n"));

    if(DBNum == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    phyPort = GT_LPORT_2_PORT(port);

	if(IS_IN_DEV_GROUP(dev,DEV_DBNUM_4096))
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_PORT_CONTROL1,0,8, &dataH);
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_PORT_VLAN_MAP,12,4, &data);
	}
	else if(IS_IN_DEV_GROUP(dev,DEV_DBNUM_256))
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_PORT_CONTROL1,0,4, &dataH);
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_PORT_VLAN_MAP,12,4, &data);
	}
	else if(IS_IN_DEV_GROUP(dev,DEV_DBNUM_64))
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_PORT_VLAN_MAP,6,2, &dataH);
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_PORT_VLAN_MAP,12,4, &data);
	}
	else
	{
		dataH = 0;
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_PORT_VLAN_MAP,12,4, &data);
	}

    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *DBNum = (GT_U32)(data | (dataH << 4));
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/********************************************************************
* gvlnSetPortVlanDot1qMode
*
* DESCRIPTION:
*       This routine sets the IEEE 802.1q mode for this port (11:10)
*
* INPUTS:
*       port	- logical port number to set.
*       mode 	- 802.1q mode for this port
*
* OUTPUTS:
*       None.
*
* RETURNS:IN GT_INGRESS_MODE mode
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortVlanDot1qMode
(
    IN GT_QD_DEV        *dev,
    IN GT_LPORT 	port,
    IN GT_DOT1Q_MODE	mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("gvlnSetPortVlanDot1qMode Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_802_1Q)) != GT_OK )
      return retVal;

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH|DEV_ENHANCED_FE_SWITCH))
	{
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_PORT_CONTROL2,10,2,(GT_U16)mode );
	}
	else
	{
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_PORT_VLAN_MAP,10,2,(GT_U16)mode );
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
* gvlnGetPortVlanDot1qMode
*
* DESCRIPTION:
*       This routine gets the IEEE 802.1q mode for this (bit 11:10).
*
* INPUTS:
*       port 	- logical port number to get.
*
* OUTPUTS:
*       mode 	- 802.1q mode for this port
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortVlanDot1qMode
(
    IN GT_QD_DEV        *dev,
    IN  GT_LPORT        port,
    OUT GT_DOT1Q_MODE   *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("gvlnGetPortVlanDot1qMode Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_802_1Q)) != GT_OK )
      return retVal;

    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH|DEV_ENHANCED_FE_SWITCH))
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_PORT_CONTROL2,10,2, &data);
	}
	else
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_PORT_VLAN_MAP,10,2, &data);
	}

    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *mode = data;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/********************************************************************
* gvlnSetPortVlanForceDefaultVID
*
* DESCRIPTION:
*       This routine sets the mode for forcing to use default VID
*
* INPUTS:
*       port    - logical port number to set.
*       mode    - GT_TRUE, force to use default VID
*                 GT_FAULSE, otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnSetPortVlanForceDefaultVID
(
    IN GT_QD_DEV        *dev,
    IN GT_LPORT 	port,
    IN GT_BOOL  	mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
    GT_U16          data;           /* Data to be set into the      */

    DBG_INFO(("gvlnSetPortForceDefaultVID Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_802_1Q)) != GT_OK )
      return retVal;

    BOOL_2_BIT(mode,data);

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_PVID,12,1,data );
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gvlnGetPortVlanForceDefaultVID
*
* DESCRIPTION:
*       This routine gets the port mode for ForceDefaultVID (bit 12).
*
* INPUTS:
*       port 	- logical port number to get.
*
* OUTPUTS:
*       mode 	- ForceDefaultVID mode for this port
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gvlnGetPortVlanForceDefaultVID
(
    IN GT_QD_DEV        *dev,
    IN  GT_LPORT 	port,
    OUT GT_BOOL    	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */


    DBG_INFO(("gvlnGetPortVlanDot1qMode Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_802_1Q)) != GT_OK )
      return retVal;

    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    retVal = hwGetPortRegField(dev,phyPort,QD_REG_PVID,12,1, &data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*mode);
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/********************************************************************
* gvlnSetForceMap
*
* DESCRIPTION:
*       This routine enables/disables Force Map feature.
*		When Force Map feature is enabled, all received frames will be
*		considered MGMT and they are mapped to the port or ports defined
*		in the VLAN Table overriding the mapping from the address database.
*
* INPUTS:
*       port    - logical port number to set.
*       mode    - GT_TRUE, to enable force map feature
*                 GT_FAULSE, otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gvlnSetForceMap
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT 	port,
    IN  GT_BOOL  	mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
    GT_U16          data;           /* Data to be set into the reg  */

    DBG_INFO(("gvlnSetForceMap Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_FORCE_MAP))
	{
		return GT_NOT_SUPPORTED;
	}

    BOOL_2_BIT(mode,data);

    retVal = hwSetPortRegField(dev,phyPort, QD_REG_PORT_VLAN_MAP, 8, 1, data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));

    return GT_OK;
}


/********************************************************************
* gvlnGetForceMap
*
* DESCRIPTION:
*       This routine checks if Force Map feature is enabled.
*		When Force Map feature is enabled, all received frames will be
*		considered MGMT and they are mapped to the port or ports defined
*		in the VLAN Table overriding the mapping from the address database.
*
* INPUTS:
*       port    - logical port number to set.
*
* OUTPUTS:
*       mode    - GT_TRUE, to enable force map feature
*                 GT_FAULSE, otherwise
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gvlnGetForceMap
(
    IN  GT_QD_DEV   *dev,
    IN  GT_LPORT 	port,
    OUT GT_BOOL  	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
    GT_U16          data;           /* Data to be set into the reg  */

    DBG_INFO(("gvlnGetForceMap Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_FORCE_MAP))
	{
		return GT_NOT_SUPPORTED;
	}

    retVal = hwGetPortRegField(dev,phyPort, QD_REG_PORT_VLAN_MAP, 8, 1, &data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data, *mode);

    DBG_INFO(("OK.\n"));

    return GT_OK;
}


/*******************************************************************************
* gvlnSetNoEgrPolicy
*
* DESCRIPTION:
*		No Egress Policy. When this bit is set to a one Egress 802.1Q Secure and
*		Check discards are not performed. This mode allowsa non-802.1Q enabled
*		port to send a frame to an 802.1Q enabled port that is configured in the
*		Secure or Check 802.1Q mode. In this situation the frames will egress
*		even if the VID assigned to the frame is not found in the VTU.
*
* INPUTS:
*		mode - no egress policy mode
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
GT_STATUS gvlnSetNoEgrPolicy
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */

    DBG_INFO(("gvlnSetNoEgrPolicy Called.\n"));

	/* Check if Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_NO_EGRESS_POLICY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(mode,data);

    /* Set related register */
    retVal = hwSetGlobal2RegField(dev,QD_REG_SDET_POLARITY,13,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gvlnGetNoEgrPolicy
*
* DESCRIPTION:
*		No Egress Policy. When this bit is set to a one Egress 802.1Q Secure and
*		Check discards are not performed. This mode allowsa non-802.1Q enabled
*		port to send a frame to an 802.1Q enabled port that is configured in the
*		Secure or Check 802.1Q mode. In this situation the frames will egress
*		even if the VID assigned to the frame is not found in the VTU.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		mode - no egress policy mode
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
GT_STATUS gvlnGetNoEgrPolicy
(
	IN  GT_QD_DEV	*dev,
	OUT GT_BOOL		*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gvlnGetNoEgrPolicy Called.\n"));

	/* Check if Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_NO_EGRESS_POLICY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get related register */
    retVal = hwGetGlobal2RegField(dev,QD_REG_SDET_POLARITY,13,1,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    BIT_2_BOOL(data,*mode);

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

