#include <Copyright.h>

/*******************************************************************************
* gtPIRL.c
*
* DESCRIPTION:
*       API definitions for PIRL Resources
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/

#include <msApi.h>
#include <gtSem.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>

/****************************************************************************/
/* STATS operation function declaration.                                    */
/****************************************************************************/
static GT_STATUS pirlOperationPerform
(
    IN   GT_QD_DEV            *dev,
    IN   GT_PIRL_OPERATION    pirlOp,
    INOUT GT_PIRL_OP_DATA     *opData
);

static GT_STATUS pirlInitialize
(
    IN  GT_QD_DEV  			*dev
);

static GT_STATUS pirlInitIRLUnit
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				irlUnit
);

static GT_STATUS pirlDataToResource
(
    IN  GT_QD_DEV  			*dev,
    IN  GT_PIRL_DATA		*pirlData,
    OUT GT_PIRL_RESOURCE	*res
);

static GT_STATUS pirlResourceToData
(
    IN  GT_QD_DEV  			*dev,
    IN  GT_PIRL_RESOURCE	*res,
    OUT GT_PIRL_DATA		*pirlData
);

static GT_STATUS pirlWriteResource
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				irlUnit,
    IN  GT_PIRL_RESOURCE	*res
);

static GT_STATUS pirlReadResource
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				irlUnit,
    OUT GT_PIRL_RESOURCE	*res
);

static GT_STATUS pirlSetPortVec
(
    IN  GT_QD_DEV	*dev,
	IN  GT_U32		irlUnit,
	IN  GT_U32		portVec
);

static GT_STATUS pirlGetPortVec
(
    IN  GT_QD_DEV	*dev,
	IN  GT_U32		irlUnit,
	OUT GT_U32		*portVec
);

static GT_STATUS pirlSetFcMode
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	IN  GT_PIRL_FC_DEASSERT		mode
);

/*******************************************************************************
* gpirlActivate
*
* DESCRIPTION:
*       This routine activates Ingress Rate Limiting for the given ports by 
*		initializing a resource bucket, assigning ports, and configuring
*		Bucket Parameters.
*
* INPUTS:
*		irlUnit  - bucket to be used (0 ~ 11).
*       portVec  - the list of ports that share the bucket.
*		pirlData - PIRL resource parameters.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpirlActivate
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32		irlUnit,
	IN  GT_U32		portVec,
	IN  GT_PIRL_DATA	*pirlData
)
{
	GT_STATUS       	retVal;
	GT_PORT_STP_STATE	pState[MAX_SWITCH_PORTS];
	GT_LPORT			port;
	GT_PIRL_OPERATION	op;
	GT_PIRL_OP_DATA		opData;
	GT_PIRL_RESOURCE	pirlRes;

	DBG_INFO(("gpirlActivate Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PIRL_RESOURCE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* check if the given bucket number is valid */
	if (!GT_IS_IRLUNIT_VALID(dev,irlUnit))
    {
        DBG_INFO(("GT_BAD_PARAM irlUnit\n"));
		return GT_BAD_PARAM;
    }

    /* check if the given portVec is valid */
	if ((!portVec) || (portVec >= (1<<dev->numOfPorts)))
    {
        DBG_INFO(("GT_BAD_PARAM portVec\n"));
		return GT_BAD_PARAM;
    }

	/* set or reset port's ingress resource bit based on the portVec */
	retVal = pirlSetPortVec(dev, irlUnit, portVec);
	if(retVal != GT_OK)
	{
        DBG_INFO(("Getting Port State failed\n"));
		return retVal;
	}

	/* Disable ports that share the bucket */
	for(port=0; port<dev->numOfPorts; port++)
	{
		if(!GT_IS_PORT_SET(portVec,port))
			continue;

		retVal = gstpGetPortState(dev, port, &pState[port]);
		if(retVal != GT_OK)
		{
	        DBG_INFO(("Getting Port State failed\n"));
			return retVal;
		}

		retVal = gstpSetPortState(dev, port, GT_PORT_DISABLE);
		if(retVal != GT_OK)
		{
	        DBG_INFO(("Getting Port State failed\n"));
			return retVal;
		}
	}

	/* Program Tuning register */
	op = PIRL_WRITE_RESOURCE;
	opData.irlUnit = irlUnit;
	opData.irlReg = 0xF;
	opData.irlData = 0x7;
	retVal = pirlOperationPerform(dev,op,&opData);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("Failed (statsOperationPerform returned GT_FAIL).\n"));
    	return retVal;
	}

	/* Program the Ingress Rate Resource Parameters */
	retVal = pirlDataToResource(dev,pirlData,&pirlRes);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("PIRL Data to PIRL Resource conversion failed.\n"));
    	return retVal;
	}

	retVal = pirlWriteResource(dev,irlUnit,&pirlRes);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("PIRL Write Resource failed.\n"));
    	return retVal;
	}

	/* Initialize internal counters */
	retVal = pirlInitIRLUnit(dev,irlUnit);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("PIRL Write Resource failed.\n"));
    	return retVal;
	}

	/* Program PirlFCMode for each port that shares Bucket */
	if (pirlRes.ebsLimitAction == ESB_LIMIT_ACTION_FC)
	{
		for(port=0; port<dev->numOfPorts; port++)
		{
			if(!GT_IS_PORT_SET(portVec,port))
				continue;

			retVal = pirlSetFcMode(dev,port,pirlData->fcDeassertMode[port]);
			if(retVal != GT_OK)
			{
			    DBG_INFO(("PIRL FC Mode set failed.\n"));
    			return retVal;
			}
		}
	}

	/* Set the ports in their original state */
	for(port=0; port<dev->numOfPorts; port++)
	{
		if(!GT_IS_PORT_SET(portVec,port))
			continue;

		retVal = gstpSetPortState(dev, port, pState[port]);
		if(retVal != GT_OK)
		{
	        DBG_INFO(("Getting Port State failed\n"));
			return retVal;
		}
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gpirlDeactivate
*
* DESCRIPTION:
*       This routine deactivates Ingress Rate Limiting for the given bucket.
*		It simply removes every ports from the Ingress Rate Resource.
*		It is assumed that gpirlActivate has been successfully called with
*		the irlUnit before this function is called.
*
* INPUTS:
*		irlUnit  - bucket to be used (0 ~ 11).
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpirlDeactivate
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32		irlUnit
)
{
	GT_STATUS       	retVal;

	DBG_INFO(("gpirlDectivate Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PIRL_RESOURCE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* check if the given bucket number is valid */
	if (!GT_IS_IRLUNIT_VALID(dev,irlUnit))
    {
        DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
    }

	/* reset port's ingress resource bit */
	retVal = pirlSetPortVec(dev, irlUnit, 0);
	if(retVal != GT_OK)
	{
        DBG_INFO(("Getting Port State failed\n"));
		return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;
}


/*******************************************************************************
* gpirlUpdateParam
*
* DESCRIPTION:
*       This routine updates IRL Parameter.
*		It is assumed that gpirlActivate has been successfully called with
*		the given irlUnit before this function is called.
*
* INPUTS:
*		irlUnit  - bucket to be used (0 ~ 11).
*		pirlData - PIRL resource parameters.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpirlUpdateParam
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32		irlUnit,
	IN  GT_PIRL_DATA	*pirlData
)
{
	GT_STATUS       	retVal;
	GT_PORT_STP_STATE	pState[MAX_SWITCH_PORTS];
	GT_LPORT			port;
	GT_PIRL_RESOURCE	pirlRes;
	GT_U32				portVec;

	DBG_INFO(("gpirlUpdateParam Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PIRL_RESOURCE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* check if the given bucket number is valid */
	if (!GT_IS_IRLUNIT_VALID(dev,irlUnit))
    {
        DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
    }

	/* get port list that share ingress resource */
	retVal = pirlGetPortVec(dev, irlUnit, &portVec);
	if(retVal != GT_OK)
	{
        DBG_INFO(("Getting Port State failed\n"));
		return retVal;
	}

    /* check if the given portVec is valid */
	if (!portVec)
    {
        DBG_INFO(("IRL Unit not Activated\n"));
		return GT_FAIL;
    }

	/* Disable ports that share the bucket */
	for(port=0; port<dev->numOfPorts; port++)
	{
		if(!GT_IS_PORT_SET(portVec,port))
			continue;

		retVal = gstpGetPortState(dev, port, &pState[port]);
		if(retVal != GT_OK)
		{
	        DBG_INFO(("Getting Port State failed\n"));
			return retVal;
		}

		retVal = gstpSetPortState(dev, port, GT_PORT_DISABLE);
		if(retVal != GT_OK)
		{
	        DBG_INFO(("Getting Port State failed\n"));
			return retVal;
		}
	}

	/* Program the Ingress Rate Resource Parameters */
	retVal = pirlDataToResource(dev,pirlData,&pirlRes);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("PIRL Data to PIRL Resource conversion failed.\n"));
    	return retVal;
	}

	retVal = pirlWriteResource(dev,irlUnit,&pirlRes);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("PIRL Write Resource failed.\n"));
    	return retVal;
	}

	/* Initialize internal counrters for the bucket */
	retVal = pirlInitIRLUnit(dev,irlUnit);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("PIRL Write Resource failed.\n"));
    	return retVal;
	}

	/* Program PirlFCMode for each port that shares Bucket */
	if (pirlRes.ebsLimitAction == ESB_LIMIT_ACTION_FC)
	{
		for(port=0; port<dev->numOfPorts; port++)
		{
			if(!GT_IS_PORT_SET(portVec,port))
				continue;

			retVal = pirlSetFcMode(dev,port,pirlData->fcDeassertMode[port]);
			if(retVal != GT_OK)
			{
			    DBG_INFO(("PIRL FC Mode set failed.\n"));
    			return retVal;
			}
		}
	}

	/* Set the ports in their original state */
	for(port=0; port<dev->numOfPorts; port++)
	{
		if(!GT_IS_PORT_SET(portVec,port))
			continue;

		retVal = gstpSetPortState(dev, port, pState[port]);
		if(retVal != GT_OK)
		{
	        DBG_INFO(("Getting Port State failed\n"));
			return retVal;
		}
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gpirlReadParam
*
* DESCRIPTION:
*       This routine retrieves IRL Parameter.
*		It is assumed that gpirlActivate has been successfully called with
*		the given irlUnit before this function is called.
*
* INPUTS:
*		irlUnit  - bucket to be used (0 ~ 11).
*
* OUTPUTS:
*		pirlData - PIRL resource parameters.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpirlReadParam
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32		irlUnit,
	OUT GT_PIRL_DATA	*pirlData
)
{
	GT_STATUS       	retVal;
	GT_LPORT			port;
	GT_PIRL_RESOURCE	pirlRes;
	GT_U32				portVec;

	DBG_INFO(("gpirlReadParam Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PIRL_RESOURCE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* check if the given bucket number is valid */
	if (!GT_IS_IRLUNIT_VALID(dev,irlUnit))
    {
        DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
    }

	/* get port list that share ingress resource */
	retVal = pirlGetPortVec(dev, irlUnit, &portVec);
	if(retVal != GT_OK)
	{
        DBG_INFO(("Getting Port State failed\n"));
		return retVal;
	}

    /* check if the given portVec is valid */
	if (!portVec)
    {
        DBG_INFO(("IRL Unit not Activated\n"));
		return GT_FAIL;
    }

	/* Read the Ingress Rate Resource Parameters */
	retVal = pirlReadResource(dev,irlUnit,&pirlRes);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("PIRL Read Resource failed.\n"));
    	return retVal;
	}

	retVal = pirlResourceToData(dev,&pirlRes,pirlData);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("PIRL Resource to PIRL Data conversion failed.\n"));
    	return retVal;
	}

	/* Program PirlFCMode for each port that shares Bucket */
	if (pirlRes.ebsLimitAction == ESB_LIMIT_ACTION_FC)
	{
		for(port=0; port<dev->numOfPorts; port++)
		{
			if(!GT_IS_PORT_SET(portVec,port))
				continue;
		
			retVal = grcGetPirlFcMode(dev,port,&pirlData->fcDeassertMode[port]);
			if(retVal != GT_OK)
			{
			    DBG_INFO(("PIRL FC Mode get failed.\n"));
    			return retVal;
			}
		}
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}



/*******************************************************************************
* gpirlUpdatePortVec
*
* DESCRIPTION:
*       This routine updates port list that share the bucket.
*		It is assumed that gpirlActivate has been successfully called with
*		the given irlUnit before this function is called.
*
* INPUTS:
*		irlUnit  - bucket to be used (0 ~ 11).
*       portVec  - the list of ports that share the bucket.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpirlUpdatePortVec
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32		irlUnit,
	IN  GT_U32		portVec
)
{
	GT_STATUS       retVal;
	GT_U32			tmpVec;

	DBG_INFO(("gpirlActivate Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PIRL_RESOURCE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* check if the given bucket number is valid */
	if (!GT_IS_IRLUNIT_VALID(dev,irlUnit))
    {
        DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
    }

    /* check if the given portVec is valid */
	if ((!portVec) || (portVec > (1<<dev->numOfPorts)))
    {
        DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
    }

	/* get port list that share ingress resource */
	retVal = pirlGetPortVec(dev, irlUnit, &tmpVec);
	if(retVal != GT_OK)
	{
        DBG_INFO(("Getting Port State failed\n"));
		return retVal;
	}

    /* check if the given portVec is valid */
	if (!tmpVec)
    {
        DBG_INFO(("IRL Unit not Activated\n"));
		return GT_FAIL;
    }

	/* set or reset port's ingress resource bit based on the portVec */
	retVal = pirlSetPortVec(dev, irlUnit, portVec);
	if(retVal != GT_OK)
	{
        DBG_INFO(("Getting Port State failed\n"));
		return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gpirlReadPortVec
*
* DESCRIPTION:
*       This routine retrieves port list that share the bucket.
*		It is assumed that gpirlActivate has been successfully called with
*		the given irlUnit before this function is called.
*
* INPUTS:
*		irlUnit  - bucket to be used (0 ~ 11).
*
* OUTPUTS:
*       portVec  - the list of ports that share the bucket.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*		GT_BAD_PARAM - if invalid parameter is given
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpirlReadPortVec
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_U32		irlUnit,
	OUT GT_U32		*portVec
)
{
	GT_STATUS       retVal;

	DBG_INFO(("gpirlReadPortVec Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PIRL_RESOURCE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* check if the given bucket number is valid */
	if (!GT_IS_IRLUNIT_VALID(dev,irlUnit))
    {
        DBG_INFO(("GT_BAD_PARAM\n"));
		return GT_BAD_PARAM;
    }

	/* get port list that share ingress resource */
	retVal = pirlGetPortVec(dev, irlUnit, portVec);
	if(retVal != GT_OK)
	{
        DBG_INFO(("Getting Port State failed\n"));
		return retVal;
	}

    /* check if the given portVec is valid */
	if (!*portVec)
    {
        DBG_INFO(("IRL Unit not Activated\n"));
		return GT_FAIL;
    }

	DBG_INFO(("OK.\n"));
	return GT_OK;

}



/*******************************************************************************
* grcGetPirlFcMode
*
* DESCRIPTION:
*       This routine gets Port Ingress Rate Limit Flow Control mode.
*		When EBSLimitAction is programmed to generate a flow control message, 
*		the deassertion of flow control is controlled by this mode.
*			GT_PIRL_FC_DEASSERT_EMPTY:
*				De-assert when the ingress rate resource has become empty
*			GT_PIRL_FC_DEASSERT_CBS_LIMIT
*				De-assert when the ingress rate resource has enough room as
*				specified by the CBSLimit.
*		Please refer to GT_PIRL_RESOURCE structure for EBSLimitAction and
*		CBSLimit.
*
* INPUTS:
*       port - logical port number
*
* OUTPUTS:
*		mode - GT_PIRL_FC_DEASSERT enum type
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS: 
*
*******************************************************************************/
GT_STATUS grcGetPirlFcMode
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	OUT GT_PIRL_FC_DEASSERT		*mode
)
{
    GT_U16          data;           
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;        /* Physical port.               */

    DBG_INFO(("grcSetDaNrlEn Called.\n"));

    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PIRL_RESOURCE))
	{
   	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* Get the PirlFcMode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_INGRESS_RATE_CTRL,12,1,&data);
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }

    *mode = (GT_PIRL_FC_DEASSERT)data;
    DBG_INFO(("OK.\n"));

    return GT_OK;
}

/*******************************************************************************
* gpirlGetIngressRateResource
*
* DESCRIPTION:
*       This routine gets Ingress Rate Limiting Resources assigned to the port.
*		This vector is used to attach specific counter resources to the physical
*		port. And the same counter resource can be attached to more than one port.
*
* INPUTS:
*       port   - logical port number
*
* OUTPUTS:
*		resVec - resource vector (bit 11:0, since there is 12 resources)
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS: 
*
*******************************************************************************/
GT_STATUS gpirlGetIngressRateResource
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	OUT GT_U32		*resVec
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;        /* Physical port.               */
	GT_U16			data;

    DBG_INFO(("grcGetIngressRateResource Called.\n"));

    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PIRL_RESOURCE))
	{
   	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* Get the resource vector.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_INGRESS_RATE_CTRL,0,12,&data);
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }

	*resVec = (GT_U32)data;

    DBG_INFO(("OK.\n"));

    return GT_OK;
}


/****************************************************************************/
/* Internal functions.                                                  */
/****************************************************************************/

/*******************************************************************************
* gpirlInitialize
*
* DESCRIPTION:
*       This routine initializes PIRL Resources.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gpirlInitialize
(
    IN  GT_QD_DEV  			*dev
)
{
	GT_STATUS       	retVal;
	GT_LPORT		port;
    GT_U8           hwPort;        /* Physical port.               */

	/* reset port's ingress resource bit */
	for(port=0; port<dev->numOfPorts; port++)
	{
	    hwPort = GT_LPORT_2_PORT(port);

	    /* Set the resource vector.            */
    	retVal = hwSetPortRegField(dev,hwPort, QD_REG_INGRESS_RATE_CTRL,0,12,0);
	    if(retVal != GT_OK)
   		{
	        DBG_INFO(("Failed.\n"));
   		    return retVal;
	    }
	}

	retVal = pirlInitialize(dev);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	    return retVal;
    }

	return GT_OK;
}


/*******************************************************************************
* statsOperationPerform
*
* DESCRIPTION:
*       This function accesses Ingress Rate Command Register and Data Register.
*
* INPUTS:
*       pirlOp       - The stats operation bits to be written into the stats
*                     operation register.
*       port        - port number
*       counter     - counter to be read if it's read operation
*
* OUTPUTS:
*       pirlData   - points to the data storage where the MIB counter will be saved.
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL otherwise.
*
* COMMENTS:
*
*******************************************************************************/
static GT_STATUS pirlOperationPerform
(
    IN    GT_QD_DEV           *dev,
    IN    GT_PIRL_OPERATION   pirlOp,
    INOUT GT_PIRL_OP_DATA     *opData
)
{
    GT_STATUS       retVal;	/* Functions return value */
    GT_U16          data; 	/* temporary Data storage */

    gtSemTake(dev,dev->pirlRegsSem,OS_WAIT_FOREVER);

    /* Wait until the pirl in ready. */
    data = 1;
    while(data == 1)
    {
        retVal = hwGetGlobal2RegField(dev,QD_REG_INGRESS_RATE_COMMAND,15,1,&data);
        if(retVal != GT_OK)
        {
            gtSemGive(dev,dev->pirlRegsSem);
            return retVal;
        }
    }

    /* Set the PIRL Operation register */
	switch (pirlOp)
	{
		case PIRL_INIT_ALL_RESOURCE:
			data = (1 << 15) | (PIRL_INIT_ALL_RESOURCE << 12);
			retVal = hwWriteGlobal2Reg(dev,QD_REG_INGRESS_RATE_COMMAND,data);
	        if(retVal != GT_OK)
    	    {
        	    gtSemGive(dev,dev->pirlRegsSem);
            	return retVal;
	        }
			break;
		case PIRL_INIT_RESOURCE:
			data = (1 << 15) | (PIRL_INIT_RESOURCE << 12) | 
					((opData->irlUnit&0xF)<< 4);
			retVal = hwWriteGlobal2Reg(dev,QD_REG_INGRESS_RATE_COMMAND,data);
	        if(retVal != GT_OK)
    	    {
        	    gtSemGive(dev,dev->pirlRegsSem);
            	return retVal;
	        }
			break;

		case PIRL_WRITE_RESOURCE:
			data = (GT_U16)opData->irlData;
			retVal = hwWriteGlobal2Reg(dev,QD_REG_INGRESS_RATE_DATA,data);
	        if(retVal != GT_OK)
    	    {
        	    gtSemGive(dev,dev->pirlRegsSem);
            	return retVal;
	        }

			data = (1 << 15) | (PIRL_WRITE_RESOURCE << 12) | 
					((opData->irlUnit&0xF) << 4) | (opData->irlReg & 0xF);
			retVal = hwWriteGlobal2Reg(dev,QD_REG_INGRESS_RATE_COMMAND,data);
	        if(retVal != GT_OK)
    	    {
        	    gtSemGive(dev,dev->pirlRegsSem);
            	return retVal;
	        }
			break;

		case PIRL_READ_RESOURCE:
			data = (1 << 15) | (PIRL_READ_RESOURCE << 12) | 
					((opData->irlUnit&0xF) << 4) | (opData->irlReg & 0xF);
			retVal = hwWriteGlobal2Reg(dev,QD_REG_INGRESS_RATE_COMMAND,data);
	        if(retVal != GT_OK)
    	    {
        	    gtSemGive(dev,dev->pirlRegsSem);
            	return retVal;
	        }

		    data = 1;
		    while(data == 1)
		    {
		        retVal = hwGetGlobal2RegField(dev,QD_REG_INGRESS_RATE_COMMAND,15,1,&data);
		        if(retVal != GT_OK)
		        {
		            gtSemGive(dev,dev->pirlRegsSem);
		            return retVal;
        		}
		    }

			retVal = hwReadGlobal2Reg(dev,QD_REG_INGRESS_RATE_DATA,&data);
			opData->irlData = (GT_U32)data;
	        if(retVal != GT_OK)
    	    {
        	    gtSemGive(dev,dev->pirlRegsSem);
            	return retVal;
	        }
		    gtSemGive(dev,dev->pirlRegsSem);
		    return retVal;

		default:
			
			gtSemGive(dev,dev->pirlRegsSem);
			return GT_FAIL;
	}

    /* Wait until the pirl in ready. */
    data = 1;
    while(data == 1)
    {
        retVal = hwGetGlobal2RegField(dev,QD_REG_INGRESS_RATE_COMMAND,15,1,&data);
        if(retVal != GT_OK)
        {
            gtSemGive(dev,dev->pirlRegsSem);
            return retVal;
        }
    }

    gtSemGive(dev,dev->pirlRegsSem);
    return retVal;
}

/*
 * Initialize all PIRL resources to the inital state.
*/
static GT_STATUS pirlInitialize
(
    IN  GT_QD_DEV  			*dev
)
{
    GT_STATUS       retVal;	/* Functions return value */
	GT_PIRL_OPERATION	op;

	op = PIRL_INIT_ALL_RESOURCE;

	retVal = pirlOperationPerform(dev, op, NULL);
	if (retVal != GT_OK)
	{
   	    DBG_INFO(("PIRL OP Failed.\n"));
       	return retVal;
	}

	return retVal;
}

/*
 * Initialize the selected PIRL resource to the inital state.
 * This function initializes only the BSM structure for the IRL Unit.
*/
static GT_STATUS pirlInitIRLUnit
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				irlUnit
)
{
    GT_STATUS       retVal;	/* Functions return value */
	GT_PIRL_OPERATION	op;
	GT_PIRL_OP_DATA		opData;

	op = PIRL_INIT_RESOURCE;
	opData.irlUnit = irlUnit;

	retVal = pirlOperationPerform(dev, op, &opData);
	if (retVal != GT_OK)
	{
   	    DBG_INFO(("PIRL OP Failed.\n"));
       	return retVal;
	}

	return retVal;
}

/*
 * convert PIRL Data structure to PIRL Resource structure.
 * if PIRL Data is not valid, return GT_BAD_PARARM;
*/
static GT_STATUS pirlDataToResource
(
    IN  GT_QD_DEV  			*dev,
    IN  GT_PIRL_DATA		*pirlData,
    OUT GT_PIRL_RESOURCE	*res
)
{
	GT_U16 typeMask;

	switch(pirlData->accountQConf)
	{
		case GT_FALSE:
		case GT_TRUE:
			res->accountQConf = pirlData->accountQConf;
			break;
		default:
			return GT_BAD_PARAM;
	}

	switch(pirlData->accountFiltered)
	{
		case GT_FALSE:
		case GT_TRUE:
			res->accountFiltered = pirlData->accountFiltered;
			break;
		default:
			return GT_BAD_PARAM;
	}

	switch(pirlData->ebsLimitAction)
	{
		case ESB_LIMIT_ACTION_DROP:
		case ESB_LIMIT_ACTION_FC:
			res->ebsLimitAction = pirlData->ebsLimitAction;
			break;
		default:
			return GT_BAD_PARAM;
	}

	if(pirlData->ingressRate == 0)
		return GT_BAD_PARAM;

	if(pirlData->ingressRate < 1000)	/* less than 1Mbps */
	{
		/* it should be divided by 64 */
		if(pirlData->ingressRate % 64)
			return GT_BAD_PARAM;
		res->bktRateFactor = pirlData->ingressRate/64;
	}
	else if(pirlData->ingressRate <= 100000)	/* less than or equal to 100Mbps */
	{
		/* it should be divided by 1000 */
		if(pirlData->ingressRate % 1000)
			return GT_BAD_PARAM;
		res->bktRateFactor = pirlData->ingressRate/64 + ((pirlData->ingressRate % 64)?1:0);
	}
	else if(pirlData->ingressRate <= 200000)	/* less than or equal to 200Mbps */
	{
		/* it should be divided by 10000 */
		if(pirlData->ingressRate % 10000)
			return GT_BAD_PARAM;
		res->bktRateFactor = pirlData->ingressRate/64 + ((pirlData->ingressRate % 64)?1:0);
	}
	else
		return GT_BAD_PARAM;

	res->ebsLimit = RECOMMENDED_ESB_LIMIT(dev);
	res->cbsLimit = RECOMMENDED_CBS_LIMIT(dev);
	res->bktIncrement = RECOMMENDED_BUCKET_INCREMENT(dev);

	switch(pirlData->bktRateType)
	{
		case BUCKET_TYPE_TRAFFIC_BASED:
			res->bktRateType = pirlData->bktRateType;

			if (IS_IN_DEV_GROUP(dev,DEV_RESTRICTED_PIRL_RESOURCE))
			{
				typeMask = 0xF;
			}
			else
			{
				typeMask = 0x7F;
			}

			if (pirlData->bktTypeMask > typeMask)
			{
				return GT_BAD_PARAM;
			}
			else
			{
				res->bktTypeMask = pirlData->bktTypeMask;
			}

			break;

		case BUCKET_TYPE_RATE_BASED:
			if (IS_IN_DEV_GROUP(dev,DEV_RESTRICTED_PIRL_RESOURCE))
				return GT_BAD_PARAM;
			res->bktRateType = pirlData->bktRateType;
			res->bktTypeMask = 0;
			break;

		default:
			return GT_BAD_PARAM;
	}

	switch(pirlData->byteTobeCounted)
	{
		case GT_PIRL_COUNT_ALL_LAYER1:
			res->byteTobeCounted = 1;
			break;
		case GT_PIRL_COUNT_ALL_LAYER2:
			res->byteTobeCounted = 2;
			break;
		case GT_PIRL_COUNT_ALL_LAYER3:
			res->byteTobeCounted = 6;
			break;
		default:
			return GT_BAD_PARAM;
	}

	return GT_OK;			
}

/*
 * convert PIRL Resource structure to PIRL Data structure.
*/
static GT_STATUS pirlResourceToData
(
    IN  GT_QD_DEV  			*dev,
    IN  GT_PIRL_RESOURCE	*res,
    OUT GT_PIRL_DATA		*pirlData
)
{
	GT_U32	rate;

	pirlData->accountQConf = res->accountQConf;
	pirlData->accountFiltered = res->accountFiltered;
	pirlData->ebsLimitAction = res->ebsLimitAction;

	rate = res->bktRateFactor * 64;
	if(rate < 1000)
	{
		pirlData->ingressRate = rate;
	}
	else if(rate < 100000)
	{
		pirlData->ingressRate = rate - (rate % 1000);
	}
	else
	{
		pirlData->ingressRate = rate - (rate % 10000);
	}

	pirlData->bktRateType = res->bktRateType;
	pirlData->bktTypeMask = res->bktTypeMask;

	switch(res->byteTobeCounted)
	{
		case 1:
			pirlData->byteTobeCounted = GT_PIRL_COUNT_ALL_LAYER1;
			break;
		case 2:
			pirlData->byteTobeCounted = GT_PIRL_COUNT_ALL_LAYER2;
			break;
		case 6:
			pirlData->byteTobeCounted = GT_PIRL_COUNT_ALL_LAYER3;
			break;
		default:
			return GT_BAD_PARAM;
	}

	return GT_OK;			
}

/*******************************************************************************
* pirlWriteResource
*
* DESCRIPTION:
*       This function writes IRL Resource to BCM (Bucket Configuration Memory)
*
* INPUTS:
*		irlUnit - resource unit to be accessed
*       res 	- IRL Resource data
*
* OUTPUTS:
*       Nont.
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL otherwise.
*
* COMMENTS:
*
*******************************************************************************/
static GT_STATUS pirlWriteResource
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				irlUnit,
    IN  GT_PIRL_RESOURCE	*res
)
{
    GT_STATUS       retVal;			/* Functions return value */
    GT_U16          data[8]; 	/* temporary Data storage */
	GT_PIRL_OPERATION	op;
	GT_PIRL_OP_DATA		opData;
	int				i;

	op = PIRL_WRITE_RESOURCE;

	/* reg0 data */
	data[0] = (GT_U16)((res->bktRateType << 15) |	/* Bit[15] : Bucket Rate Type */
			  		(res->bktTypeMask << 4 ) |		/* Bit[14:4] : Traffic Type   */
			  		res->byteTobeCounted );			/* Bit[3:0] : Bytes to be counted */

	/* reg1 data */
	data[1] = (GT_U16)res->bktIncrement;	/* Bit[11:0] : Bucket Increment */

	/* reg2 data */
	data[2] = (GT_U16)res->bktRateFactor;	/* Bit[15:0] : Bucket Rate Factor */

	/* reg3 data */
	data[3] = (GT_U16)(res->cbsLimit & 0xFFF) << 4;	/* Bit[15:4] : CBS Limit[11:0] */

	/* reg4 data */
	data[4] = (GT_U16)(res->cbsLimit >> 12);		/* Bit[11:0] : CBS Limit[23:12] */

	/* reg5 data */
	data[5] = (GT_U16)(res->ebsLimit & 0xFFFF);		/* Bit[15:0] : EBS Limit[15:0] */

	/* reg6 data */
	data[6] = (GT_U16)((res->ebsLimit >> 16)	|	/* Bit[7:0] : EBS Limit[23:16] */
					(res->ebsLimitAction << 12)	|	/* Bit[12] : EBS Limit Action */
					(res->accountFiltered << 14)|	/* Bit[14] : Account Filtered */
					(res->accountQConf << 15));		/* Bit[15] : Account QConf */
	/* reg7 data */
	data[7] = 0;	/* Reserved */

	for(i=0; i<8; i++)
	{
		opData.irlUnit = irlUnit;
		opData.irlReg = i;
		opData.irlData = data[i];

		retVal = pirlOperationPerform(dev, op, &opData);
		if (retVal != GT_OK)
		{
    	    DBG_INFO(("PIRL OP Failed.\n"));
        	return retVal;
		}
	}

	return GT_OK;	
}


/*******************************************************************************
* pirlReadResource
*
* DESCRIPTION:
*       This function reads IRL Resource from BCM (Bucket Configuration Memory)
*
* INPUTS:
*		irlUnit - resource unit to be accessed
*
* OUTPUTS:
*       res - IRL Resource data
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL otherwise.
*
* COMMENTS:
*
*******************************************************************************/
static GT_STATUS pirlReadResource
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				irlUnit,
    OUT GT_PIRL_RESOURCE	*res
)
{
    GT_STATUS       retVal;			/* Functions return value */
    GT_U16          data[8]; 	/* temporary Data storage */
	GT_PIRL_OPERATION	op;
	GT_PIRL_OP_DATA		opData;
	int				i;

	op = PIRL_READ_RESOURCE;

	for(i=0; i<8; i++)
	{
		opData.irlUnit = irlUnit;
		opData.irlReg = i;
		opData.irlData = 0;

		retVal = pirlOperationPerform(dev, op, &opData);
		if (retVal != GT_OK)
		{
    	    DBG_INFO(("PIRL OP Failed.\n"));
        	return retVal;
		}

		data[i] = opData.irlData;
	}
	

	/* reg0 data */
	res->bktRateType = (data[0] >> 15) & 0x1;
	res->bktTypeMask = (data[0] >> 4) & 0x7F;

	res->byteTobeCounted = data[0] & 0xF;

	/* reg1 data */
	res->bktIncrement = data[1] & 0xFFF;

	/* reg2 data */
	res->bktRateFactor = data[2] & 0xFFFF;

	/* reg3,4 data */
	res->cbsLimit = ((data[3] >> 4) & 0xFFF) | ((data[4] & 0xFFF) << 12);

	/* reg5,6 data */
	res->ebsLimit = data[5] | ((data[6] & 0xFF) << 16);
													   
	/* reg6 data */
	res->ebsLimitAction = (data[6] >> 12) & 0x1;
	res->accountFiltered = (data[6] >> 14) & 0x1;
	res->accountQConf = (data[6] >> 15) & 0x1;

	return GT_OK;
}

/*******************************************************************************
* pirlSetPortVec
*
* DESCRIPTION:
*       This routine sets port list that share the bucket and resets ports that
*		do not share the bucket.
*
* INPUTS:
*		irlUnit  - bucket to be used.
*       portVec  - the list of ports that share the bucket.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*
* COMMENTS: 
*
*******************************************************************************/
static GT_STATUS pirlSetPortVec
(
    IN  GT_QD_DEV	*dev,
	IN  GT_U32		irlUnit,
	IN  GT_U32		portVec
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_LPORT		port;
    GT_U8           hwPort;        /* Physical port.               */
	GT_U16			data;

	for(port=0; port<dev->numOfPorts; port++)
	{
		if(GT_IS_PORT_SET(portVec,port))
			data = 1;
		else
			data = 0;

	    hwPort = GT_LPORT_2_PORT(port);

	    /* Set the resource vector.            */
    	retVal = hwSetPortRegField(dev,hwPort, QD_REG_INGRESS_RATE_CTRL,irlUnit,1,data);
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
* pirlGetPortVec
*
* DESCRIPTION:
*       This routine gets port list that share the bucket.
*
* INPUTS:
*		irlUnit  - bucket to be used.
*
* OUTPUTS:
*       portVec  - the list of ports that share the bucket.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*
* COMMENTS: 
*
*******************************************************************************/
static GT_STATUS pirlGetPortVec
(
    IN  GT_QD_DEV	*dev,
	IN  GT_U32		irlUnit,
	OUT GT_U32		*portVec
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_LPORT		port;
    GT_U8           hwPort;        /* Physical port.               */
	GT_U16			data;

	*portVec = 0;

	for(port=0; port<dev->numOfPorts; port++)
	{
	    hwPort = GT_LPORT_2_PORT(port);

	    /* Set the resource vector.            */
    	retVal = hwGetPortRegField(dev,hwPort, QD_REG_INGRESS_RATE_CTRL,irlUnit,1,&data);
	    if(retVal != GT_OK)
   		{
	        DBG_INFO(("Failed.\n"));
   		    return retVal;
	    }

		if(data == 1)
			*portVec |= (1 << port);
	}

    DBG_INFO(("OK.\n"));

    return GT_OK;
}


/*******************************************************************************
* pirlSetFcMode
*
* DESCRIPTION:
*       This routine gets Port Ingress Rate Limit Flow Control mode.
*		When EBSLimitAction is programmed to generate a flow control message, 
*		the deassertion of flow control is controlled by this mode.
*			GT_PIRL_FC_DEASSERT_EMPTY:
*				De-assert when the ingress rate resource has become empty
*			GT_PIRL_FC_DEASSERT_CBS_LIMIT
*				De-assert when the ingress rate resource has enough room as
*				specified by the CBSLimit.
*		Please refer to GT_PIRL_RESOURCE structure for EBSLimitAction and
*		CBSLimit.
*
* INPUTS:
*       port - logical port number
*		mode - GT_PIRL_FC_DEASSERT enum type
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS: 
*
*******************************************************************************/
static GT_STATUS pirlSetFcMode
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	IN  GT_PIRL_FC_DEASSERT		mode
)
{
    GT_U16          data;           
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;        /* Physical port.               */

    DBG_INFO(("pirlSetFcMode Called.\n"));

    hwPort = GT_LPORT_2_PORT(port);

	data = (GT_U16) mode;

    /* Set the PirlFcMode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_INGRESS_RATE_CTRL,12,1,data);
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }

    DBG_INFO(("OK.\n"));

    return GT_OK;
}

