#include <Copyright.h>

/********************************************************************************
* gtQosMap.c
*
* DESCRIPTION:
*       API implementation for qos mapping.
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
* gcosSetPortDefaultTc
*
* DESCRIPTION:
*       Sets the default traffic class for a specific port.
*
* INPUTS:
*       port      - logical port number
*       trafClass - default traffic class of a port.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       Fast Ethernet switch family supports 2 bits (0 ~ 3) while Gigabit Switch
*		family supports 3 bits (0 ~ 7)
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gcosSetPortDefaultTc
(
    IN  GT_QD_DEV *dev,
    IN GT_LPORT   port,
    IN GT_U8      trafClass
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gcosSetPortDefaultTc Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_QoS)) != GT_OK )
      return retVal;

	/* Only Gigabit Switch supports this status. */
	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH|DEV_ENHANCED_FE_SWITCH))
    {
	    /* Set the default port pri.  */
    	retVal = hwSetPortRegField(dev,hwPort,QD_REG_PVID,13,3,trafClass);
    }
	else
	{
	    /* Set the default port pri.  */
    	retVal = hwSetPortRegField(dev,hwPort,QD_REG_PVID,14,2,trafClass);
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
* gcosGetPortDefaultTc
*
* DESCRIPTION:
*       Gets the default traffic class for a specific port.
*
* INPUTS:
*       port      - logical port number
*
* OUTPUTS:
*       trafClass - default traffic class of a port.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       Fast Ethernet switch family supports 2 bits (0 ~ 3) while Gigabit Switch
*		family supports 3 bits (0 ~ 7)
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gcosGetPortDefaultTc
(
    IN  GT_QD_DEV *dev,
    IN GT_LPORT   port,
    OUT GT_U8     *trafClass
)
{
	GT_U16			data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gcosSetPortDefaultTc Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_QoS)) != GT_OK )
      return retVal;

	/* Only Gigabit Switch supports this status. */
	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH|DEV_ENHANCED_FE_SWITCH))
    {
	    /* Get the default port pri.  */
	    retVal = hwGetPortRegField(dev,hwPort,QD_REG_PVID,13,3,&data);
    }
	else
	{
	    /* Get the default port pri.  */
	    retVal = hwGetPortRegField(dev,hwPort,QD_REG_PVID,14,2,&data);
	}

	*trafClass = (GT_U8)data;

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
* gqosSetPrioMapRule
*
* DESCRIPTION:
*       This routine sets priority mapping rule.
*		If the current frame is both IEEE 802.3ac tagged and an IPv4 or IPv6,
*		and UserPrioMap (for IEEE 802.3ac) and IPPrioMap (for IP frame) are
*		enabled, then priority selection is made based on this setup.
*		If PrioMapRule is set to GT_TRUE, UserPrioMap is used.
*		If PrioMapRule is reset to GT_FALSE, IPPrioMap is used.
*
* INPUTS:
*       port - the logical port number.
*       mode - GT_TRUE for user prio rule, GT_FALSE for otherwise.
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
GT_STATUS gqosSetPrioMapRule
(
    IN  GT_QD_DEV *dev,
    IN GT_LPORT   port,
    IN GT_BOOL    mode
)
{
    GT_U16          data;           /* temporary data buffer */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosSetPrioMapRule Called.\n"));
    /* translate bool to binary */
    BOOL_2_BIT(mode, data);
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_QoS)) != GT_OK )
      return retVal;

    /* Set the TagIfBoth.  */
    retVal = hwSetPortRegField(dev,hwPort,QD_REG_PORT_CONTROL,6,1,data);
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
* gqosGetPrioMapRule
*
* DESCRIPTION:
*       This routine gets priority mapping rule.
*		If the current frame is both IEEE 802.3ac tagged and an IPv4 or IPv6,
*		and UserPrioMap (for IEEE 802.3ac) and IPPrioMap (for IP frame) are
*		enabled, then priority selection is made based on this setup.
*		If PrioMapRule is set to GT_TRUE, UserPrioMap is used.
*		If PrioMapRule is reset to GT_FALSE, IPPrioMap is used.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       mode - GT_TRUE for user prio rule, GT_FALSE for otherwise.
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
GT_STATUS gqosGetPrioMapRule
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosGetPrioMapRule Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_QoS)) != GT_OK )
      return retVal;

    /* get the TagIfBoth.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,6,1,&data);
    /* translate bool to binary */
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
* gqosIpPrioMapEn
*
* DESCRIPTION:
*       This routine enables the IP priority mapping.
*
* INPUTS:
*       port - the logical port number.
*       en   - GT_TRUE to Enable, GT_FALSE for otherwise.
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
GT_STATUS gqosIpPrioMapEn
(
    IN  GT_QD_DEV *dev,
    IN GT_LPORT   port,
    IN GT_BOOL    en
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosIpPrioMapEn Called.\n"));
    /* translate bool to binary */
    BOOL_2_BIT(en, data);
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_QoS)) != GT_OK )
      return retVal;

    /* Set the useIp.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,5,1,data);
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
* gqosGetIpPrioMapEn
*
* DESCRIPTION:
*       This routine return the IP priority mapping state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       en    - GT_TRUE for user prio rule, GT_FALSE for otherwise.
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
GT_STATUS gqosGetIpPrioMapEn
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *en
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosGetIpPrioMapEn Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_QoS)) != GT_OK )
      return retVal;

    /* Get the UseIp.  */
    retVal = hwGetPortRegField(dev,hwPort,QD_REG_PORT_CONTROL,5,1,&data);
    /* translate bool to binary */
    BIT_2_BOOL(data, *en);
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
* gqosUserPrioMapEn
*
* DESCRIPTION:
*       This routine enables the user priority mapping.
*
* INPUTS:
*       port - the logical port number.
*       en   - GT_TRUE to Enable, GT_FALSE for otherwise.
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
GT_STATUS gqosUserPrioMapEn
(
    IN  GT_QD_DEV *dev,
    IN GT_LPORT   port,
    IN GT_BOOL    en
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosUserPrioMapEn Called.\n"));
    /* translate bool to binary */
    BOOL_2_BIT(en, data);
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_QoS)) != GT_OK )
      return retVal;

    /* Set the useTag.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL,4,1,data);
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
* gqosGetUserPrioMapEn
*
* DESCRIPTION:
*       This routine return the user priority mapping state.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       en    - GT_TRUE for user prio rule, GT_FALSE for otherwise.
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
GT_STATUS gqosGetUserPrioMapEn
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT  port,
    OUT GT_BOOL   *en
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosGetUserPrioMapEn Called.\n"));
    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_QoS)) != GT_OK )
      return retVal;

    /* Get the UseTag.  */
    retVal = hwGetPortRegField(dev,hwPort,QD_REG_PORT_CONTROL,4,1,&data);
    /* translate bool to binary */
    BIT_2_BOOL(data, *en);
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
* gcosGetUserPrio2Tc
*
* DESCRIPTION:
*       Gets the traffic class number for a specific 802.1p user priority.
*
* INPUTS:
*       userPrior - user priority
*
* OUTPUTS:
*       trClass - The Traffic Class the received frame is assigned.
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
GT_STATUS gcosGetUserPrio2Tc
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8     userPrior,
    OUT GT_U8     *trClass
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           bitOffset;      /* the bit offset in the reg    */
    GT_U16          data;           /* store the read data          */

    DBG_INFO(("gcosGetUserPrio2Tc Called.\n"));

    /* check if device supports this feature */
    if(!IS_IN_DEV_GROUP(dev,DEV_QoS))
		return GT_NOT_SUPPORTED;

    /* calc the bit offset */
    bitOffset = ((userPrior & 0x7) * 2);
    /* Get the traffic class for the VPT.  */
    retVal = hwGetGlobalRegField(dev,QD_REG_IEEE_PRI,bitOffset,2,&data);
    *trClass = (GT_U8)data;
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
* gcosSetUserPrio2Tc
*
* DESCRIPTION:
*       Sets the traffic class number for a specific 802.1p user priority.
*
* INPUTS:
*       userPrior - user priority of a port.
*       trClass   - the Traffic Class the received frame is assigned.
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
GT_STATUS gcosSetUserPrio2Tc
(
    IN  GT_QD_DEV *dev,
    IN GT_U8      userPrior,
    IN GT_U8      trClass
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           bitOffset;      /* the bit offset in the reg    */

    DBG_INFO(("gcosSetUserPrio2Tc Called.\n"));
    /* check if device supports this feature */
    if(!IS_IN_DEV_GROUP(dev,DEV_QoS))
		return GT_NOT_SUPPORTED;

    /* calc the bit offset */
    bitOffset = ((userPrior & 0x7) * 2);
    /* Set the traffic class for the VPT.  */
    retVal = hwSetGlobalRegField(dev,QD_REG_IEEE_PRI, bitOffset,2,trClass);
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
* gcosGetDscp2Tc
*
* DESCRIPTION:
*       This routine retrieves the traffic class assigned for a specific
*       IPv4 Dscp.
*
* INPUTS:
*       dscp    - the IPv4 frame dscp to query.
*
* OUTPUTS:
*       trClass - The Traffic Class the received frame is assigned.
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
GT_STATUS gcosGetDscp2Tc
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8     dscp,
    OUT GT_U8     *trClass
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           bitOffset;      /* the bit offset in the reg    */
    GT_U8           regOffset;      /* the reg offset in the IP tbl */
    GT_U16          data;           /* store the read data          */

    DBG_INFO(("gcosGetDscp2Tc Called.\n"));
    /* check if device supports this feature */
    if(!IS_IN_DEV_GROUP(dev,DEV_QoS))
		return GT_NOT_SUPPORTED;

    /* calc the bit offset */
    bitOffset = (((dscp & 0x3f) % 8) * 2);
    regOffset = ((dscp & 0x3f) / 8);
    /* Get the traffic class for the IP dscp.  */
    retVal = hwGetGlobalRegField(dev,(GT_U8)(QD_REG_IP_PRI_BASE+regOffset),
                                 bitOffset, 2, &data);
    *trClass = (GT_U8)data;
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
* gcosSetDscp2Tc
*
* DESCRIPTION:
*       This routine sets the traffic class assigned for a specific
*       IPv4 Dscp.
*
* INPUTS:
*       dscp    - the IPv4 frame dscp to map.
*       trClass - the Traffic Class the received frame is assigned.
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
GT_STATUS gcosSetDscp2Tc
(
    IN  GT_QD_DEV *dev,
    IN GT_U8      dscp,
    IN GT_U8      trClass
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           bitOffset;      /* the bit offset in the reg    */
    GT_U8           regOffset;      /* the reg offset in the IP tbl */

    DBG_INFO(("gcosSetDscp2Tc Called.\n"));
    /* check if device supports this feature */
    if(!IS_IN_DEV_GROUP(dev,DEV_QoS))
		return GT_NOT_SUPPORTED;

    /* calc the bit offset */
    bitOffset = (((dscp & 0x3f) % 8) * 2);
    regOffset = ((dscp & 0x3f) / 8);
    /* Set the traffic class for the IP dscp.  */
    retVal = hwSetGlobalRegField(dev,(GT_U8)(QD_REG_IP_PRI_BASE+regOffset),
                                 bitOffset, 2, trClass);
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
* gqosGetTagRemap
*
* DESCRIPTION:
*		Gets the remapped priority value for a specific 802.1p priority on a
*		given port.
*
* INPUTS:
*		port  - the logical port number.
*		pri   - 802.1p priority
*
* OUTPUTS:
*		remappedPri - remapped Priority
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosGetTagRemap
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT 	port,
	IN  GT_U8    	pri,
	OUT GT_U8   	*remappedPri
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* store the read data          */
    GT_U8           phyPort;        /* Physical port.               */
    GT_U8           regAddr;        /* register address.            */
    GT_U8           bitOffset;      /* the bit offset in the reg    */

    DBG_INFO(("gqosGetTagRemap Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PRIORITY_REMAPPING))
	{
		return GT_NOT_SUPPORTED;
	}

    phyPort = GT_LPORT_2_PORT(port);

	if (pri <= 3)
	{
		regAddr = QD_REG_IEEE_PRI_REMAP_3_0;
	}
	else
	{
		regAddr = QD_REG_IEEE_PRI_REMAP_7_4;
	}

    /* calc the bit offset */
    bitOffset = 4 * (pri % 4);

    retVal = hwGetPortRegField(dev,phyPort,regAddr,bitOffset,3,&data );

    *remappedPri = (GT_U8)data;

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
* gqosSetTagRemap
*
* DESCRIPTION:
*		Sets the remapped priority value for a specific 802.1p priority on a
*		given port.
*
* INPUTS:
*		port  - the logical port number.
*		pri   - 802.1p priority
*		remappedPri - remapped Priority
*
* OUTPUTS:
*		None
*
* RETURNS:
*		GT_OK   - on success
*		GT_FAIL - on error
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetTagRemap
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT 	port,
	IN GT_U8    	pri,
	IN GT_U8    	remappedPri
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
    GT_U8           regAddr;        /* register address.            */
    GT_U8           bitOffset;      /* the bit offset in the reg    */

    DBG_INFO(("gqosSetTagRemap Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PRIORITY_REMAPPING))
	{
		return GT_NOT_SUPPORTED;
	}

    phyPort = GT_LPORT_2_PORT(port);

	if (pri <= 3)
	{
		regAddr = QD_REG_IEEE_PRI_REMAP_3_0;
	}
	else
	{
		regAddr = QD_REG_IEEE_PRI_REMAP_7_4;
	}

    /* calc the bit offset */
    bitOffset = 4 * (pri % 4);

    retVal = hwSetPortRegField(dev,phyPort,regAddr,bitOffset,3,remappedPri);

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
* gqosSetQPriValue
*
* DESCRIPTION:
*       This routine sets Queue priority value to used when forced.
*		When ForceQPri is enabled (gqosSetForceQPri), all frames entering this port
*		are mapped to the priority queue defined in this value, unless a VTU, SA,
*		DA or ARP priority override occurs. The Frame's priority (FPri) is not
*		effected by this value.
*
* INPUTS:
*       port - the logical port number.
*       pri  - Queue priority value
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_BAD_PARAM - if pri > 3
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gqosSetQPriValue
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    IN  GT_U8      pri
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosSetQPriValue Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_QoS_FPRI_QPRI))
	{
		return GT_NOT_SUPPORTED;
	}

	if (pri > 3)
	{
		return GT_BAD_PARAM;
	}

    /* Set the QPriValue.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_VLAN_MAP, 10, 2, (GT_U16)pri);
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
* gqosGetQPriValue
*
* DESCRIPTION:
*       This routine gets Queue priority value to used when forced.
*		When ForceQPri is enabled (gqosSetForceQPri), all frames entering this port
*		are mapped to the priority queue defined in this value, unless a VTU, SA,
*		DA or ARP priority override occurs. The Frame's priority (FPri) is not
*		effected by this value.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       pri  - Queue priority value
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gqosGetQPriValue
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    OUT GT_U8      *pri
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosGetQPriValue Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_QoS_FPRI_QPRI))
	{
		return GT_NOT_SUPPORTED;
	}

    /* Get the QPriValue.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_VLAN_MAP, 10, 2, (GT_U16*)pri);
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
* gqosSetForceQPri
*
* DESCRIPTION:
*       This routine enables/disables forcing Queue priority.
*		When ForceQPri is disabled, normal priority queue mapping is used on all
*		ingressing frames entering this port. When it's enabled, all frames
*		entering this port are mapped to the QPriValue (gqosSetQPriValue), unless
*		a VTU, SA, DA or ARP priority override occurs. The frame's priorty (FPri)
*		is not effected by this feature.
*
* INPUTS:
*       port - the logical port number.
*       en   - GT_TRUE, to force Queue Priority,
*			   GT_FALSE, otherwise.
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
*******************************************************************************/
GT_STATUS gqosSetForceQPri
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    IN  GT_BOOL    en
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosSetQPriValue Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* translate BOOL to binary */
    BOOL_2_BIT(en, data);

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_QoS_FPRI_QPRI))
	{
		return GT_NOT_SUPPORTED;
	}

    /* Set the ForceQPri.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_VLAN_MAP, 9, 1, data);
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
* gqosGetForceQPri
*
* DESCRIPTION:
*       This routine checks if forcing Queue priority is enabled.
*		When ForceQPri is disabled, normal priority queue mapping is used on all
*		ingressing frames entering this port. When it's enabled, all frames
*		entering this port are mapped to the QPriValue (gqosSetQPriValue), unless
*		a VTU, SA, DA or ARP priority override occurs. The frame's priorty (FPri)
*		is not effected by this feature.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       en   - GT_TRUE, to force Queue Priority,
*			   GT_FALSE, otherwise.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gqosGetForceQPri
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    OUT GT_BOOL    *en
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosGetQPriValue Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_QoS_FPRI_QPRI))
	{
		return GT_NOT_SUPPORTED;
	}

    /* Get the ForceQPri.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_VLAN_MAP, 9, 1, &data);
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
* gqosSetDefFPri
*
* DESCRIPTION:
*       This routine sets the default frame priority (0 ~ 7).
*		This priority is used as the default frame priority (FPri) to use when
*		no other priority information is available.
*
* INPUTS:
*       port - the logical port number
*       pri  - default frame priority
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_BAD_PARAM - if pri > 7
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gqosSetDefFPri
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    IN  GT_U8      pri
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosSetDefFPri Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_QoS_FPRI_QPRI))
	{
		return GT_NOT_SUPPORTED;
	}

	if (pri > 7)
	{
		return GT_BAD_PARAM;
	}

    /* Set the DefFPri.  */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PVID, 13, 3, (GT_U16)pri);
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
* gqosGetDefFPri
*
* DESCRIPTION:
*       This routine gets the default frame priority (0 ~ 7).
*		This priority is used as the default frame priority (FPri) to use when
*		no other priority information is available.
*
* INPUTS:
*       port - the logical port number
*
* OUTPUTS:
*       pri  - default frame priority
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gqosGetDefFPri
(
    IN  GT_QD_DEV  *dev,
    IN  GT_LPORT   port,
    OUT GT_U8      *pri
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosGetDefFPri Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_QoS_FPRI_QPRI))
	{
		return GT_NOT_SUPPORTED;
	}

    /* Get the DefFPri.  */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PVID, 13, 3, (GT_U16*)pri);
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
* gqosSetVIDFPriOverride
*
* DESCRIPTION:
*		This routine sets VID Frame Priority Override. When this feature is enabled,
*		VID Frame priority overrides can occur on this port.
*		VID Frame priority override occurs when the determined VID of a frame
*		results in a VTU entry whose useVIDFPri override field is set to GT_TRUE.
*		When this occurs the VIDFPri value assigned to the frame's VID (in the
*		VTU Table) is used to overwrite the frame's previously determined frame
*		priority. If the frame egresses tagged the priority in the frame will be
*		this new VIDFPri value. This function does not affect the egress queue
*		priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for VID Frame Priority Override,
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetVIDFPriOverride
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosSetVIDFPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FQPRI_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set the VIDFPriOverride mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,14,1,data);

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
* gqosGetVIDFPriOverride
*
* DESCRIPTION:
*		This routine gets VID Frame Priority Override. When this feature is enabled,
*		VID Frame priority overrides can occur on this port.
*		VID Frame priority override occurs when the determined VID of a frame
*		results in a VTU entry whose useVIDFPri override field is set to GT_TRUE.
*		When this occurs the VIDFPri value assigned to the frame's VID (in the
*		VTU Table) is used to overwrite the frame's previously determined frame
*		priority. If the frame egresses tagged the priority in the frame will be
*		this new VIDFPri value. This function does not affect the egress queue
*		priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE for VID Frame Priority Override,
*			   GT_FALSE otherwise
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
GT_STATUS gqosGetVIDFPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosGetVIDFPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FQPRI_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the VIDFPriOverride mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,14,1,&data);

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
* gqosSetSAFPriOverride
*
* DESCRIPTION:
*		This routine sets Source Address(SA) Frame Priority Override.
*		When this feature is enabled, SA Frame priority overrides can occur on
*		this port.
*		SA ATU Frame priority override occurs when the determined source address
*		of a frame results in an ATU hit where the SA's MAC address entry contains
*		the useATUFPri field set to GT_TRUE.
*		When this occurs the ATUFPri value assigned to the frame's SA (in the
*		ATU Table) is used to overwrite the frame's previously determined frame
*		priority. If the frame egresses tagged the priority in the frame will be
*		this new ATUFPri value. This function does not affect the egress queue
*		priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for SA Frame Priority Override,
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetSAFPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosSetSAFPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FQPRI_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set the SAFPriOverride mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,13,1,data);

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
* gqosGetSAFPriOverride
*
* DESCRIPTION:
*		This routine gets Source Address(SA) Frame Priority Override.
*		When this feature is enabled, SA Frame priority overrides can occur on
*		this port.
*		SA ATU Frame priority override occurs when the determined source address
*		of a frame results in an ATU hit where the SA's MAC address entry contains
*		the useATUFPri field set to GT_TRUE.
*		When this occurs the ATUFPri value assigned to the frame's SA (in the
*		ATU Table) is used to overwrite the frame's previously determined frame
*		priority. If the frame egresses tagged the priority in the frame will be
*		this new ATUFPri value. This function does not affect the egress queue
*		priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE for SA Frame Priority Override,
*			   GT_FALSE otherwise
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
GT_STATUS gqosGetSAFPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosGetSAFPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FQPRI_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the SAFPriOverride mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,13,1,&data);

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
* gqosSetDAFPriOverride
*
* DESCRIPTION:
*		This routine sets Destination Address(DA) Frame Priority Override.
*		When this feature is enabled, DA Frame priority overrides can occur on
*		this port.
*		DA ATU Frame priority override occurs when the determined destination address
*		of a frame results in an ATU hit where the DA's MAC address entry contains
*		the useATUFPri field set to GT_TRUE.
*		When this occurs the ATUFPri value assigned to the frame's DA (in the
*		ATU Table) is used to overwrite the frame's previously determined frame
*		priority. If the frame egresses tagged the priority in the frame will be
*		this new ATUFPri value. This function does not affect the egress queue
*		priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for DA Frame Priority Override,
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetDAFPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosSetDAFPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FQPRI_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set the DAFPriOverride mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,12,1,data);

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
* gqosGetDAFPriOverride
*
* DESCRIPTION:
*		This routine gets Destination Address(DA) Frame Priority Override.
*		When this feature is enabled, DA Frame priority overrides can occur on
*		this port.
*		DA ATU Frame priority override occurs when the determined destination address
*		of a frame results in an ATU hit where the DA's MAC address entry contains
*		the useATUFPri field set to GT_TRUE.
*		When this occurs the ATUFPri value assigned to the frame's DA (in the
*		ATU Table) is used to overwrite the frame's previously determined frame
*		priority. If the frame egresses tagged the priority in the frame will be
*		this new ATUFPri value. This function does not affect the egress queue
*		priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE for DA Frame Priority Override,
*			   GT_FALSE otherwise
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
GT_STATUS gqosGetDAFPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosGetDAFPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FQPRI_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the DAFPriOverride mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,12,1,&data);

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
* gqosSetVIDQPriOverride
*
* DESCRIPTION:
*		This routine sets VID Queue Priority Override. When this feature is enabled,
*		VID Queue priority overrides can occur on this port.
*		VID Queue priority override occurs when the determined VID of a frame
*		results in a VTU entry whose useVIDQPri override field is set to GT_TRUE.
*		When this occurs the VIDQPri value assigned to the frame's VID (in the
*		VTU Table) is used to overwrite the frame's previously determined queue
*		priority. If the frame egresses tagged the priority in the frame will not
*		be modified by this new VIDQPri value. This function affects the egress
*		queue priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for VID Queue Priority Override,
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetVIDQPriOverride
(
	IN GT_QD_DEV	*dev,
	IN GT_LPORT		port,
	IN GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosSetVIDQPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FQPRI_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set the VIDQPriOverride mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,3,1,data);

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
* gqosGetVIDQPriOverride
*
* DESCRIPTION:
*		This routine gets VID Queue Priority Override. When this feature is enabled,
*		VID Queue priority overrides can occur on this port.
*		VID Queue priority override occurs when the determined VID of a frame
*		results in a VTU entry whose useVIDQPri override field is set to GT_TRUE.
*		When this occurs the VIDQPri value assigned to the frame's VID (in the
*		VTU Table) is used to overwrite the frame's previously determined queue
*		priority. If the frame egresses tagged the priority in the frame will not
*		be modified by this new VIDQPri value. This function affects the egress
*		queue priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE for VID Queue Priority Override,
*			   GT_FALSE otherwise
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
GT_STATUS gqosGetVIDQPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosGetVIDQPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FQPRI_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the VIDQPriOverride mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,3,1,&data);

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
* gqosSetSAQPriOverride
*
* DESCRIPTION:
*		This routine sets Source Address(SA) Queue Priority Override.
*		When this feature is enabled, SA Queue priority overrides can occur on
*		this port.
*		SA ATU Queue priority override occurs when the determined source address
*		of a frame results in an ATU hit where the SA's MAC address entry contains
*		the useATUQPri field set to GT_TRUE.
*		When this occurs the ATUQPri value assigned to the frame's SA (in the
*		ATU Table) is used to overwrite the frame's previously determined queue
*		priority. If the frame egresses tagged the priority in the frame will not
*		be modified by this new ATUQPri value. This function affects the egress
*		queue priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for SA Queue Priority Override,
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetSAQPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosSetSAQPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FQPRI_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set the SAQPriOverride mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,2,1,data);

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
* gqosGetSAQPriOverride
*
* DESCRIPTION:
*		This routine gets Source Address(SA) Queue Priority Override.
*		When this feature is enabled, SA Queue priority overrides can occur on
*		this port.
*		SA ATU Queue priority override occurs when the determined source address
*		of a frame results in an ATU hit where the SA's MAC address entry contains
*		the useATUQPri field set to GT_TRUE.
*		When this occurs the ATUQPri value assigned to the frame's SA (in the
*		ATU Table) is used to overwrite the frame's previously determined queue
*		priority. If the frame egresses tagged the priority in the frame will not
*		be modified by this new ATUQPri value. This function affects the egress
*		queue priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE for SA Queue Priority Override,
*			   GT_FALSE otherwise
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
GT_STATUS gqosGetSAQPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosGetSAQPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FQPRI_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the SAQPriOverride mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,2,1,&data);

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
* gqosSetDAQPriOverride
*
* DESCRIPTION:
*		This routine sets Destination Address(DA) Queue Priority Override.
*		When this feature is enabled, DA Queue priority overrides can occur on
*		this port.
*		DA ATU Queue priority override occurs when the determined destination address
*		of a frame results in an ATU hit where the DA's MAC address entry contains
*		the useATUQPri field set to GT_TRUE.
*		When this occurs the ATUQPri value assigned to the frame's DA (in the
*		ATU Table) is used to overwrite the frame's previously determined queue
*		priority. If the frame egresses tagged the priority in the frame will not
*		be modified by this new ATUQPri value. This function affects the egress
*		queue priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for DA Queue Priority Override,
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetDAQPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosSetDAQPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FQPRI_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set the DAQPriOverride mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,1,1,data);

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
* gqosGetDAQPriOverride
*
* DESCRIPTION:
*		This routine sets Destination Address(DA) Queue Priority Override.
*		When this feature is enabled, DA Queue priority overrides can occur on
*		this port.
*		DA ATU Queue priority override occurs when the determined destination address
*		of a frame results in an ATU hit where the DA's MAC address entry contains
*		the useATUQPri field set to GT_TRUE.
*		When this occurs the ATUQPri value assigned to the frame's DA (in the
*		ATU Table) is used to overwrite the frame's previously determined queue
*		priority. If the frame egresses tagged the priority in the frame will not
*		be modified by this new ATUQPri value. This function affects the egress
*		queue priority (QPri) the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE for DA Queue Priority Override,
*			   GT_FALSE otherwise
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
GT_STATUS gqosGetDAQPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosGetDAQPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FQPRI_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the DAQPriOverride mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,1,1,&data);

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
* gqosSetARPQPriOverride
*
* DESCRIPTION:
*		This routine sets ARP Queue Priority Override.
*		When this feature is enabled, ARP Queue priority overrides can occur on
*		this port.
*		ARP Queue priority override occurs for all ARP frames.
*		When this occurs, the frame's previously determined egress queue priority
*		will be overwritten with ArpQPri.
*		If the frame egresses tagged the priority in the frame will not
*		be modified. When used, the two bits of the ArpQPri priority determine the
*		egress queue the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*		mode - GT_TRUE for ARP Queue Priority Override,
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
* GalTis:
*
*******************************************************************************/
GT_STATUS gqosSetARPQPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosSetARPQPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FQPRI_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set the ARPQPriOverride mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,0,1,data);

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
* gqosGetARPQPriOverride
*
* DESCRIPTION:
*		This routine sets ARP Queue Priority Override.
*		When this feature is enabled, ARP Queue priority overrides can occur on
*		this port.
*		ARP Queue priority override occurs for all ARP frames.
*		When this occurs, the frame's previously determined egress queue priority
*		will be overwritten with ArpQPri.
*		If the frame egresses tagged the priority in the frame will not
*		be modified. When used, the two bits of the ArpQPri priority determine the
*		egress queue the frame is switched into.
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE for ARP Queue Priority Override,
*			   GT_FALSE otherwise
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
GT_STATUS gqosGetARPQPriOverride
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gqosGetARPQPriOverride Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_FQPRI_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the ARPQPriOverride mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_CONTROL2,0,1,&data);

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
* gqosSetArpQPri
*
* DESCRIPTION:
*       This routine sets ARP queue Priority to use for ARP QPri Overridden
*		frames. When a ARP frame is received on a por tthat has its ARP
*		QPriOVerride is enabled, the QPri assigned to the frame comes from
*		this value
*
* INPUTS:
*       pri - ARP Queue Priority (0 ~ 3)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_BAD_PARAM - if pri > 3
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS gqosSetArpQPri
(
    IN  GT_QD_DEV *dev,
    IN  GT_U8     pri
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("gqosSetArpQPri Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_FQPRI_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	if (pri > 3)
	{
		DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
	}

	data = (GT_U16)pri;

    /* Set the ArpQPri bit.            */
    retVal = hwSetGlobalRegField(dev,QD_REG_MANGEMENT_CONTROL,6,2,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gqosGetArpQPri
*
* DESCRIPTION:
*       This routine gets ARP queue Priority to use for ARP QPri Overridden
*		frames. When a ARP frame is received on a por tthat has its ARP
*		QPriOVerride is enabled, the QPri assigned to the frame comes from
*		this value
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       pri - ARP Queue Priority (0 ~ 3)
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
GT_STATUS gqosGetArpQPri
(
    IN  GT_QD_DEV *dev,
    OUT GT_U8     *pri
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("gqosGetArpQPri Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_FQPRI_OVERRIDE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the ArpQPri bit.            */
    retVal = hwGetGlobalRegField(dev,QD_REG_MANGEMENT_CONTROL,6,2,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	*pri = (GT_U8)data;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


