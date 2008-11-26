#include <Copyright.h>

/********************************************************************************
* gtPolicy.c
*
* DESCRIPTION:
*       API definitions to handle Policy Mapping
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
* gprtSetPolicy
*
* DESCRIPTION:
*       This routine sets the Policy for ports.
*		Supported Policies are defined as GT_FRAME_POLICY as follows:
*			FRAME_POLICY_NONE    - normal frame switching
*			FRAME_POLICY_MIRROR  - mirror (copy) frame to MirrorDest port
*			FRAME_POLICY_TRAP    - trap(re-direct) frame to the CPUDest port
*			FRAME_POLICY_DISCARD - discard(filter) the frame
*		Supported Policy types are defined as GT_POLICY_TYPE:
*			POLICY_TYPE_DA - DA Policy Mapping
*				DA Policy Mapping occurs when the DA of a frame is contained in 
*				the ATU address database with an Entry State that indicates Policy.
*			POLICY_TYPE_SA - SA Policy Mapping
*				SA Policy Mapping occurs when the SA of a frame is contained in 
*				the ATU address database with an Entry State that indicates Policy.
*			POLICY_TYPE_VTU - VTU Policy Mapping
*				VTU Policy Mapping occurs when the VID of a frame is contained in
*				the VTU database with the VidPolicy is enabled.
*			POLICY_TYPE_ETYPE - EtherType Policy Mapping
*				EType Policy Mapping occurs when the EtherType of a frame matches
*				the PortEType (see gprtSetPortEType API)
*			POLICY_TYPE_PPPoE - PPPoE Policy Mapping
*				PPPoE Policy Mapping occurs when the EtherType of a frame matches 0x8863
*			POLICY_TYPE_VBAS - VBAS Policy Mapping
*				VBAS Policy Mapping occurs when the EtherType of a frame matches 0x8200
*			POLICY_TYPE_OPT82 - DHCP Option 82 Policy Mapping
*				DHCP Option 82 Policy Mapping occurs when the ingressing frame is an
*				IPv4 UDP with a UDP Destination port = 0x0043 or 0x0044, or an
*				IPv6 UDP with a UDP Destination port = 0x0223 or 0x0222
*			POLICY_TYPE_UDP - UDP Policy Mapping
*				UDP Policy Mapping occurs when the ingressing frame is
*				a Broadcast IPv4 UDP or a Multicast IPv6 UDP.
*
* INPUTS:
*       port	- logical port number.
*       type 	- policy type (GT_POLICY_TYPE)
*       policy 	- policy (GT_FRAME_POLICY)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK            - on success
*       GT_FAIL          - on error
*       GT_BAD_PARAM     - on bad parameters
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS: 
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtSetPolicy
(
    IN  GT_QD_DEV 	*dev,
    IN  GT_LPORT 	port,
    IN  GT_POLICY_TYPE	type,
	IN	GT_FRAME_POLICY	policy
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* Physical port.               */
    GT_U8	    	offset;

    DBG_INFO(("gprtSetPolicy Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_POLICY))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	switch (policy)
	{
		case FRAME_POLICY_NONE:
		case FRAME_POLICY_MIRROR:
		case FRAME_POLICY_TRAP:
		case FRAME_POLICY_DISCARD:
			break;
		default:
	        DBG_INFO(("Bad Policy\n"));
			return GT_BAD_PARAM;
	}
		
	switch (type)
	{
		case POLICY_TYPE_DA:
			offset = 14;
			break;
		case POLICY_TYPE_SA:
			offset = 12;
			break;
		case POLICY_TYPE_VTU:
			offset = 10;
			break;
		case POLICY_TYPE_ETYPE:
			offset = 8;
			break;
		case POLICY_TYPE_PPPoE:
			offset = 6;
			break;
		case POLICY_TYPE_VBAS:
			offset = 4;
			break;
		case POLICY_TYPE_OPT82:
			offset = 2;
			break;
		case POLICY_TYPE_UDP:
			offset = 0;
			break;
		default:
	        DBG_INFO(("Bad Parameter\n"));
			return GT_BAD_PARAM;
	}

    retVal = hwSetPortRegField(dev,hwPort, QD_REG_POLICY_CONTROL, offset, 2, (GT_U16)policy);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gprtGetPolicy
*
* DESCRIPTION:
*       This routine gets the Policy of the given policy type.
*		Supported Policies are defined as GT_FRAME_POLICY as follows:
*			FRAME_POLICY_NONE    - normal frame switching
*			FRAME_POLICY_MIRROR  - mirror (copy) frame to MirrorDest port
*			FRAME_POLICY_TRAP    - trap(re-direct) frame to the CPUDest port
*			FRAME_POLICY_DISCARD - discard(filter) the frame
*		Supported Policy types are defined as GT_POLICY_TYPE:
*			POLICY_TYPE_DA - DA Policy Mapping
*				DA Policy Mapping occurs when the DA of a frame is contained in 
*				the ATU address database with an Entry State that indicates Policy.
*			POLICY_TYPE_SA - SA Policy Mapping
*				SA Policy Mapping occurs when the SA of a frame is contained in 
*				the ATU address database with an Entry State that indicates Policy.
*			POLICY_TYPE_VTU - VTU Policy Mapping
*				VTU Policy Mapping occurs when the VID of a frame is contained in
*				the VTU database with the VidPolicy is enabled.
*			POLICY_TYPE_ETYPE - EtherType Policy Mapping
*				EType Policy Mapping occurs when the EtherType of a frame matches
*				the PortEType (see gprtSetPortEType API)
*			POLICY_TYPE_PPPoE - PPPoE Policy Mapping
*				PPPoE Policy Mapping occurs when the EtherType of a frame matches 0x8863
*			POLICY_TYPE_VBAS - VBAS Policy Mapping
*				VBAS Policy Mapping occurs when the EtherType of a frame matches 0x8200
*			POLICY_TYPE_OPT82 - DHCP Option 82 Policy Mapping
*				DHCP Option 82 Policy Mapping occurs when the ingressing frame is an
*				IPv4 UDP with a UDP Destination port = 0x0043 or 0x0044, or an
*				IPv6 UDP with a UDP Destination port = 0x0223 or 0x0222
*			POLICY_TYPE_UDP - UDP Policy Mapping
*				UDP Policy Mapping occurs when the ingressing frame is
*				a Broadcast IPv4 UDP or a Multicast IPv6 UDP.
*
* INPUTS:
*       port	- logical port number.
*       type 	- policy type (GT_POLICY_TYPE)
*
* OUTPUTS:
*       policy 	- policy (GT_FRAME_POLICY)
*
* RETURNS:
*       GT_OK            - on success
*       GT_FAIL          - on error
*       GT_BAD_PARAM     - on bad parameters
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS: 
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gprtGetPolicy
(
    IN  GT_QD_DEV 	*dev,
    IN  GT_LPORT 	port,
    IN  GT_POLICY_TYPE	type,
	OUT GT_FRAME_POLICY	*policy
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* Physical port.               */
    GT_U8	    	offset;
	GT_U16			data;

    DBG_INFO(("gprtGetPolicy Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);
    
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_POLICY))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}
	
	switch (type)
	{
		case POLICY_TYPE_DA:
			offset = 14;
			break;
		case POLICY_TYPE_SA:
			offset = 12;
			break;
		case POLICY_TYPE_VTU:
			offset = 10;
			break;
		case POLICY_TYPE_ETYPE:
			offset = 8;
			break;
		case POLICY_TYPE_PPPoE:
			offset = 6;
			break;
		case POLICY_TYPE_VBAS:
			offset = 4;
			break;
		case POLICY_TYPE_OPT82:
			offset = 2;
			break;
		case POLICY_TYPE_UDP:
			offset = 0;
			break;
		default:
	        DBG_INFO(("Bad Parameter\n"));
			return GT_BAD_PARAM;
	}

    retVal = hwGetPortRegField(dev,hwPort, QD_REG_POLICY_CONTROL, offset, 2, &data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	*policy = (GT_FRAME_POLICY)data;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}




