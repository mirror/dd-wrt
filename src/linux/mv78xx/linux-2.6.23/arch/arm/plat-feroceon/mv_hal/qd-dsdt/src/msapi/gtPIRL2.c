#include <Copyright.h>

/*******************************************************************************
* gtPIRL2.c
*
* DESCRIPTION:
*       API definitions for Port based PIRL Resources
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
/* PIRL operation function declaration.                                    */
/****************************************************************************/
static GT_STATUS pirl2OperationPerform
(
    IN   GT_QD_DEV            *dev,
    IN   GT_PIRL2_OPERATION    pirlOp,
    INOUT GT_PIRL2_OP_DATA     *opData
);

static GT_STATUS pirl2Initialize
(
    IN  GT_QD_DEV  			*dev
);

static GT_STATUS pirl2InitIRLResource
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				irlPort,
	IN	GT_U32				irlRes
);

static GT_STATUS pirl2DisableIRLResource
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				irlPort,
	IN	GT_U32				irlRes
);

static GT_STATUS pirl2DataToResource
(
    IN  GT_QD_DEV  			*dev,
    IN  GT_PIRL2_DATA		*pirlData,
    OUT GT_PIRL2_RESOURCE	*res
);

static GT_STATUS pirl2ResourceToData
(
    IN  GT_QD_DEV  			*dev,
    IN  GT_PIRL2_RESOURCE	*res,
    OUT GT_PIRL2_DATA		*pirlData
);

static GT_STATUS pirl2WriteResource
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				irlPort,
	IN	GT_U32				irlRes,
    IN  GT_PIRL2_RESOURCE	*res
);

static GT_STATUS pirl2ReadResource
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				irlPort,
	IN	GT_U32				irlRes,
    OUT GT_PIRL2_RESOURCE	*res
);


/*******************************************************************************
* gpirl2WriteResource
*
* DESCRIPTION:
*       This routine writes resource bucket parameters to the given resource
*		of the port.
*
* INPUTS:
*       port     - logical port number.
*		irlRes   - bucket to be used (0 ~ 4).
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
GT_STATUS gpirl2WriteResource
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_LPORT	port,
	IN  GT_U32		irlRes,
	IN  GT_PIRL2_DATA	*pirlData
)
{
	GT_STATUS       	retVal;
	GT_PIRL2_RESOURCE	pirlRes;
    GT_U32           	irlPort;         /* the physical port number     */
	GT_U32				maxRes;

	DBG_INFO(("gpirl2WriteResource Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PIRL2_RESOURCE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* check if the given bucket number is valid */
	if (IS_IN_DEV_GROUP(dev,DEV_RESTRICTED_PIRL2_RESOURCE))
	{
		maxRes = 2;
	}
	else
	{
		maxRes = 5;
	}

	if (irlRes >= maxRes)
    {
        DBG_INFO(("GT_BAD_PARAM irlRes\n"));
		return GT_BAD_PARAM;
    }

    irlPort = (GT_U32)GT_LPORT_2_PORT(port);
	if (irlPort == GT_INVALID_PORT)
	{
        DBG_INFO(("GT_BAD_PARAM port\n"));
		return GT_BAD_PARAM;
	}

	/* Initialize internal counters */
	retVal = pirl2InitIRLResource(dev,irlPort,irlRes);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("PIRL Write Resource failed.\n"));
    	return retVal;
	}

	/* Program the Ingress Rate Resource Parameters */
	retVal = pirl2DataToResource(dev,pirlData,&pirlRes);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("PIRL Data to PIRL Resource conversion failed.\n"));
    	return retVal;
	}

	retVal = pirl2WriteResource(dev,irlPort,irlRes,&pirlRes);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("PIRL Write Resource failed.\n"));
    	return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}

/*******************************************************************************
* gpirl2ReadResource
*
* DESCRIPTION:
*       This routine retrieves IRL Parameter.
*
* INPUTS:
*       port     - logical port number.
*		irlRes   - bucket to be used (0 ~ 4).
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
GT_STATUS gpirl2ReadResource
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_LPORT	port,
	IN  GT_U32		irlRes,
	OUT GT_PIRL2_DATA	*pirlData
)
{
	GT_STATUS       	retVal;
	GT_U32				irlPort;
	GT_PIRL2_RESOURCE	pirlRes;
	GT_U32				maxRes;

	DBG_INFO(("gpirl2ReadResource Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PIRL2_RESOURCE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* check if the given bucket number is valid */
	if (IS_IN_DEV_GROUP(dev,DEV_RESTRICTED_PIRL2_RESOURCE))
	{
		maxRes = 2;
	}
	else
	{
		maxRes = 5;
	}

	if (irlRes >= maxRes)
    {
        DBG_INFO(("GT_BAD_PARAM irlRes\n"));
		return GT_BAD_PARAM;
    }

    irlPort = (GT_U32)GT_LPORT_2_PORT(port);
	if (irlPort == GT_INVALID_PORT)
	{
        DBG_INFO(("GT_BAD_PARAM port\n"));
		return GT_BAD_PARAM;
	}

	/* Read the Ingress Rate Resource Parameters */
	retVal = pirl2ReadResource(dev,irlPort,irlRes,&pirlRes);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("PIRL Read Resource failed.\n"));
    	return retVal;
	}

	retVal = pirl2ResourceToData(dev,&pirlRes,pirlData);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("PIRL Resource to PIRL Data conversion failed.\n"));
    	return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gpirl2DisableResource
*
* DESCRIPTION:
*       This routine disables Ingress Rate Limiting for the given bucket.
*
* INPUTS:
*       port     - logical port number.
*		irlRes   - bucket to be used (0 ~ 4).
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
GT_STATUS gpirl2DisableResource
(
	IN  GT_QD_DEV 	*dev,
	IN  GT_LPORT	port,
	IN  GT_U32		irlRes
)
{
	GT_STATUS       	retVal;
	GT_U32				irlPort;
	GT_U32				maxRes;

	DBG_INFO(("gpirl2Dectivate Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PIRL2_RESOURCE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* check if the given bucket number is valid */
	if (IS_IN_DEV_GROUP(dev,DEV_RESTRICTED_PIRL2_RESOURCE))
	{
		maxRes = 2;
	}
	else
	{
		maxRes = 5;
	}

	if (irlRes >= maxRes)
    {
        DBG_INFO(("GT_BAD_PARAM irlRes\n"));
		return GT_BAD_PARAM;
    }

    irlPort = (GT_U32)GT_LPORT_2_PORT(port);
	if (irlPort == GT_INVALID_PORT)
	{
        DBG_INFO(("GT_BAD_PARAM port\n"));
		return GT_BAD_PARAM;
	}

	/* disable irl resource */
	retVal = pirl2DisableIRLResource(dev, irlPort, irlRes);
	if(retVal != GT_OK)
	{
        DBG_INFO(("Getting Port State failed\n"));
		return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;
}

/*******************************************************************************
* gpirl2SetCurTimeUpInt
*
* DESCRIPTION:
*       This function sets the current time update interval.
*		Please contact FAE for detailed information.
*
* INPUTS:
*       upInt - updata interval (0 ~ 7)
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
*
*******************************************************************************/
GT_STATUS gpirl2SetCurTimeUpInt
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				upInt
)
{
    GT_STATUS       retVal;		/* Functions return value */
	GT_PIRL2_OPERATION	op;
	GT_PIRL2_OP_DATA	opData;

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PIRL2_RESOURCE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	if (upInt > 0x7)
		return GT_BAD_PARAM;

	op = PIRL_READ_RESOURCE;

	opData.irlPort = 0xF;
	opData.irlRes = 0;
	opData.irlReg = 1;
	opData.irlData = 0;

	retVal = pirl2OperationPerform(dev, op, &opData);
	if (retVal != GT_OK)
	{
   	    DBG_INFO(("PIRL OP Failed.\n"));
       	return retVal;
	}

	op = PIRL_WRITE_RESOURCE;
	opData.irlData = (opData.irlData & 0xFFF8) | (GT_U16)upInt;

	retVal = pirl2OperationPerform(dev, op, &opData);
	if (retVal != GT_OK)
	{
   	    DBG_INFO(("PIRL OP Failed.\n"));
       	return retVal;
	}

	return GT_OK;	
}


/****************************************************************************/
/* Internal functions.                                                  */
/****************************************************************************/

/*******************************************************************************
* gpirl2Initialize
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
GT_STATUS gpirl2Initialize
(
    IN  GT_QD_DEV  			*dev
)
{
	GT_STATUS       	retVal;

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PIRL2_RESOURCE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	retVal = pirl2Initialize(dev);
    if(retVal != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
	    return retVal;
    }

	return GT_OK;
}


/*******************************************************************************
* pirl2OperationPerform
*
* DESCRIPTION:
*       This function accesses Ingress Rate Command Register and Data Register.
*
* INPUTS:
*       pirlOp     - The stats operation bits to be written into the stats
*                    operation register.
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
static GT_STATUS pirl2OperationPerform
(
    IN    GT_QD_DEV 			*dev,
    IN    GT_PIRL2_OPERATION	pirlOp,
    INOUT GT_PIRL2_OP_DATA		*opData
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
					(opData->irlPort << 8) |
					(opData->irlRes << 5);
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
					(opData->irlPort << 8)	|
					(opData->irlRes << 5)	|
					(opData->irlReg & 0xF);
			retVal = hwWriteGlobal2Reg(dev,QD_REG_INGRESS_RATE_COMMAND,data);
	        if(retVal != GT_OK)
    	    {
        	    gtSemGive(dev,dev->pirlRegsSem);
            	return retVal;
	        }
			break;

		case PIRL_READ_RESOURCE:
			data = (1 << 15) | (PIRL_READ_RESOURCE << 12) | 
					(opData->irlPort << 8)	|
					(opData->irlRes << 5)	|
					(opData->irlReg & 0xF);
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
static GT_STATUS pirl2Initialize
(
    IN  GT_QD_DEV  			*dev
)
{
    GT_STATUS       retVal;	/* Functions return value */
	GT_PIRL2_OPERATION	op;

	op = PIRL_INIT_ALL_RESOURCE;

	retVal = pirl2OperationPerform(dev, op, NULL);
	if (retVal != GT_OK)
	{
   	    DBG_INFO(("PIRL OP Failed.\n"));
       	return retVal;
	}

	retVal = gpirl2SetCurTimeUpInt(dev,4);
	if (retVal != GT_OK)
	{
   	    DBG_INFO(("PIRL OP Failed.\n"));
	}

	return retVal;
}

/*
 * Initialize the selected PIRL resource to the inital state.
 * This function initializes only the BSM structure for the IRL Unit.
*/
static GT_STATUS pirl2InitIRLResource
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				irlPort,
	IN	GT_U32				irlRes
)
{
    GT_STATUS       retVal;	/* Functions return value */
	GT_PIRL2_OPERATION	op;
	GT_PIRL2_OP_DATA 	opData;

	op = PIRL_INIT_RESOURCE;
	opData.irlPort = irlPort;
	opData.irlRes = irlRes;

	retVal = pirl2OperationPerform(dev, op, &opData);
	if (retVal != GT_OK)
	{
   	    DBG_INFO(("PIRL OP Failed.\n"));
       	return retVal;
	}

	return retVal;
}

/*
 * Disable the selected PIRL resource.
*/
static GT_STATUS pirl2DisableIRLResource
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				irlPort,
	IN	GT_U32				irlRes
)
{
    GT_STATUS       retVal;			/* Functions return value */
	GT_PIRL2_OPERATION	op;
	GT_PIRL2_OP_DATA	opData;
	int				i;

	op = PIRL_WRITE_RESOURCE;

	for(i=0; i<8; i++)
	{
		opData.irlPort = irlPort;
		opData.irlRes = irlRes;
		opData.irlReg = i;
		opData.irlData = 0;

		retVal = pirl2OperationPerform(dev, op, &opData);
		if (retVal != GT_OK)
		{
    	    DBG_INFO(("PIRL OP Failed.\n"));
        	return retVal;
		}
	}

	return GT_OK;	
}

/*
 * convert PIRL Data structure to PIRL Resource structure.
 * if PIRL Data is not valid, return GT_BAD_PARARM;
*/
static GT_STATUS pirl2DataToResource
(
    IN  GT_QD_DEV  			*dev,
    IN  GT_PIRL2_DATA		*pirlData,
    OUT GT_PIRL2_RESOURCE	*res
)
{
	GT_U32 typeMask;
	GT_U32 data;

	gtMemSet((void*)res,0,sizeof(GT_PIRL2_RESOURCE));

	data = (GT_U32)(pirlData->accountQConf|pirlData->accountFiltered|
					pirlData->mgmtNrlEn|pirlData->saNrlEn|pirlData->daNrlEn|
					pirlData->samplingMode);

	if (data > 1)
	{
		DBG_INFO(("GT_BAD_PARAM (Boolean)\n"));
		return GT_BAD_PARAM;
	}

	if (IS_IN_DEV_GROUP(dev,DEV_RESTRICTED_PIRL2_RESOURCE))
	{
		if (pirlData->samplingMode != GT_FALSE)
		{
			DBG_INFO(("GT_BAD_PARAM (sampling mode)\n"));
			return GT_BAD_PARAM;
		}
	}

	res->accountQConf = pirlData->accountQConf;
	res->accountFiltered = pirlData->accountFiltered;
	res->mgmtNrlEn = pirlData->mgmtNrlEn;
	res->saNrlEn = pirlData->saNrlEn;
	res->daNrlEn = pirlData->daNrlEn;
	res->samplingMode = pirlData->samplingMode;

	switch(pirlData->actionMode)
	{
		case PIRL_ACTION_ACCEPT:
		case PIRL_ACTION_USE_LIMIT_ACTION:
			res->actionMode = pirlData->actionMode;
			break;
		default:
			DBG_INFO(("GT_BAD_PARAM actionMode\n"));
			return GT_BAD_PARAM;
	}

	switch(pirlData->ebsLimitAction)
	{
		case ESB_LIMIT_ACTION_DROP:
		case ESB_LIMIT_ACTION_FC:
			res->ebsLimitAction = pirlData->ebsLimitAction;
			break;
		default:
			DBG_INFO(("GT_BAD_PARAM ebsLimitAction\n"));
			return GT_BAD_PARAM;
	}

	switch(pirlData->fcDeassertMode)
	{
		case GT_PIRL_FC_DEASSERT_EMPTY:
		case GT_PIRL_FC_DEASSERT_CBS_LIMIT:
			res->fcDeassertMode = pirlData->fcDeassertMode;
			break;
		default:
			if(res->ebsLimitAction != ESB_LIMIT_ACTION_FC)
			{
				res->fcDeassertMode	= GT_PIRL_FC_DEASSERT_EMPTY;
				break;
			}
			DBG_INFO(("GT_BAD_PARAM fcDeassertMode\n"));
			return GT_BAD_PARAM;
	}

	if(pirlData->customSetup.isValid == GT_TRUE)
	{
		res->ebsLimit = pirlData->customSetup.ebsLimit;
		res->cbsLimit = pirlData->customSetup.cbsLimit;
		res->bktIncrement = pirlData->customSetup.bktIncrement;
		res->bktRateFactor = pirlData->customSetup.bktRateFactor;
	}
	else
	{
		if(pirlData->ingressRate == 0)
		{
			DBG_INFO(("GT_BAD_PARAM ingressRate(%i)\n",pirlData->ingressRate));
			return GT_BAD_PARAM;
		}

		if(pirlData->ingressRate < 1000)	/* less than 1Mbps */
		{
			/* it should be divided by 64 */
			if(pirlData->ingressRate % 64)
			{
				DBG_INFO(("GT_BAD_PARAM ingressRate(%i)\n",pirlData->ingressRate));
				return GT_BAD_PARAM;
			}
			res->bktRateFactor = pirlData->ingressRate/64;
		}
		else if(pirlData->ingressRate < 10000)	/* less than or equal to 10Mbps */
		{
			/* it should be divided by 1000 */
			if(pirlData->ingressRate % 1000)
			{
				DBG_INFO(("GT_BAD_PARAM ingressRate(%i)\n",pirlData->ingressRate));
				return GT_BAD_PARAM;
			}
			res->bktRateFactor = pirlData->ingressRate/128 + ((pirlData->ingressRate % 128)?1:0);
		}
		else if(pirlData->ingressRate < 100000)	/* less than or equal to 100Mbps */
		{
			/* it should be divided by 1000 */
			if(pirlData->ingressRate % 1000)
			{
				DBG_INFO(("GT_BAD_PARAM ingressRate(%i)\n",pirlData->ingressRate));
				return GT_BAD_PARAM;
			}
			res->bktRateFactor = pirlData->ingressRate/1000;
		}
		else if(pirlData->ingressRate <= 200000)	/* less than or equal to 200Mbps */
		{
			/* it should be divided by 10000 */
			if(pirlData->ingressRate % 10000)
			{
				DBG_INFO(("GT_BAD_PARAM ingressRate(%i)\n",pirlData->ingressRate));
				return GT_BAD_PARAM;
			}
			res->bktRateFactor = pirlData->ingressRate/1000;
		}
		else
		{
			DBG_INFO(("GT_BAD_PARAM ingressRate(%i)\n",pirlData->ingressRate));
			return GT_BAD_PARAM;
		}

		res->ebsLimit = RECOMMENDED_ESB_LIMIT(dev, pirlData->ingressRate);
		res->cbsLimit = RECOMMENDED_CBS_LIMIT(dev, pirlData->ingressRate);
		res->bktIncrement = RECOMMENDED_BUCKET_INCREMENT(dev, pirlData->ingressRate);
	}

	switch(pirlData->bktRateType)
	{
		case BUCKET_TYPE_TRAFFIC_BASED:
			res->bktRateType = pirlData->bktRateType;

			typeMask = 0x7FFF;

			if (pirlData->bktTypeMask > typeMask)
			{
				DBG_INFO(("GT_BAD_PARAM bktTypeMask(%#x)\n",pirlData->bktTypeMask));
				return GT_BAD_PARAM;
			}

		   	res->bktTypeMask = pirlData->bktTypeMask;

			if (pirlData->bktTypeMask & BUCKET_TRAFFIC_ARP)
			{
				res->bktTypeMask &= ~BUCKET_TRAFFIC_ARP;
				res->bktTypeMask |= 0x80;
			}
			
			if (pirlData->priORpt > 1)
			{
				DBG_INFO(("GT_BAD_PARAM rpiORpt\n"));
				return GT_BAD_PARAM;
			}

			res->priORpt = pirlData->priORpt;

			if (pirlData->priMask >= (1 << 4))
			{
				DBG_INFO(("GT_BAD_PARAM priMask(%#x)\n",pirlData->priMask));
				return GT_BAD_PARAM;
			}

			res->priMask = pirlData->priMask;

			break;

		case BUCKET_TYPE_RATE_BASED:
			res->bktRateType = pirlData->bktRateType;
		   	res->bktTypeMask = pirlData->bktTypeMask;
			res->priORpt = pirlData->priORpt;
			res->priMask = pirlData->priMask;
			break;

		default:
			DBG_INFO(("GT_BAD_PARAM bktRateType(%#x)\n",pirlData->bktRateType));
			return GT_BAD_PARAM;
	}

	switch(pirlData->byteTobeCounted)
	{
		case GT_PIRL2_COUNT_FRAME:
		case GT_PIRL2_COUNT_ALL_LAYER1:
		case GT_PIRL2_COUNT_ALL_LAYER2:
		case GT_PIRL2_COUNT_ALL_LAYER3:
			res->byteTobeCounted = pirlData->byteTobeCounted;
			break;
		default:
			DBG_INFO(("GT_BAD_PARAM byteTobeCounted(%#x)\n",pirlData->byteTobeCounted));
			return GT_BAD_PARAM;
	}

	return GT_OK;			
}

/*
 * convert PIRL Resource structure to PIRL Data structure.
*/
static GT_STATUS pirl2ResourceToData
(
    IN  GT_QD_DEV  			*dev,
    IN  GT_PIRL2_RESOURCE	*res,
    OUT GT_PIRL2_DATA		*pirlData
)
{
	GT_U32	rate;
	GT_U32	factor;

	pirlData->accountQConf = res->accountQConf;
	pirlData->accountFiltered = res->accountFiltered;
	pirlData->mgmtNrlEn = res->mgmtNrlEn;
	pirlData->saNrlEn = res->saNrlEn;
	pirlData->daNrlEn = res->daNrlEn;
	pirlData->samplingMode = res->samplingMode;
	pirlData->ebsLimitAction = res->ebsLimitAction;
	pirlData->actionMode = res->actionMode;
	pirlData->fcDeassertMode = res->fcDeassertMode;

	pirlData->customSetup.isValid = GT_FALSE;

	FACTOR_FROM_BUCKET_INCREMENT(dev,res->bktIncrement,factor);

	rate = res->bktRateFactor * factor;
	if(factor == 128)
	{
		pirlData->ingressRate = rate - (rate % 1000);
	}
	else if (factor == 0)
	{
		pirlData->ingressRate = 0;
		pirlData->customSetup.isValid = GT_TRUE;
		pirlData->customSetup.ebsLimit = res->ebsLimit;
		pirlData->customSetup.cbsLimit = res->cbsLimit;
		pirlData->customSetup.bktIncrement = res->bktIncrement;
		pirlData->customSetup.bktRateFactor = res->bktRateFactor;
	}
	else
	{
		pirlData->ingressRate = rate;
	}

	pirlData->bktRateType = res->bktRateType;
	pirlData->bktTypeMask = res->bktTypeMask;

	if (pirlData->bktTypeMask & 0x80)
	{
		res->bktTypeMask &= ~0x80;
		res->bktTypeMask |= BUCKET_TRAFFIC_ARP;
	}
			
	pirlData->priORpt = res->priORpt;
	pirlData->priMask = res->priMask;

	pirlData->byteTobeCounted = res->byteTobeCounted;

	return GT_OK;			
}

/*******************************************************************************
* pirl2WriteResource
*
* DESCRIPTION:
*       This function writes IRL Resource to BCM (Bucket Configuration Memory)
*
* INPUTS:
*       irlPort - physical port number.
*		irlRes  - bucket to be used (0 ~ 4).
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
static GT_STATUS pirl2WriteResource
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				irlPort,
	IN	GT_U32				irlRes,
    IN  GT_PIRL2_RESOURCE	*res
)
{
    GT_STATUS       retVal;			/* Functions return value */
    GT_U16          data[8]; 	/* temporary Data storage */
	GT_PIRL2_OPERATION	op;
	GT_PIRL2_OP_DATA 	opData;
	int				i;

	op = PIRL_WRITE_RESOURCE;

	/* reg0 data */
	data[0] = (GT_U16)((res->bktRateType << 15) |	/* Bit[15] : Bucket Rate Type */
			  		(res->bktTypeMask << 0 )); 		/* Bit[14:0] : Traffic Type   */

	/* reg1 data */
	data[1] = (GT_U16)res->bktIncrement;	/* Bit[11:0] : Bucket Increment */

	/* reg2 data */
	data[2] = (GT_U16)res->bktRateFactor;	/* Bit[15:0] : Bucket Rate Factor */

	/* reg3 data */
	data[3] = (GT_U16)((res->cbsLimit & 0xFFF) << 4)|	/* Bit[15:4] : CBS Limit[11:0] */
					(res->byteTobeCounted << 2);		/* Bit[3:0] : Bytes to be counted */

	/* reg4 data */
	data[4] = (GT_U16)(res->cbsLimit >> 12);		/* Bit[11:0] : CBS Limit[23:12] */

	/* reg5 data */
	data[5] = (GT_U16)(res->ebsLimit & 0xFFFF);		/* Bit[15:0] : EBS Limit[15:0] */

	/* reg6 data */
	data[6] = (GT_U16)((res->ebsLimit >> 16)	|	/* Bit[7:0] : EBS Limit[23:16] */
					(res->samplingMode << 11)	|	/* Bit[11] : Sampling Mode */
					(res->ebsLimitAction << 12)	|	/* Bit[12] : EBS Limit Action */
					(res->actionMode << 13)		|	/* Bit[13] : Action Mode */
					(res->fcDeassertMode << 14));	/* Bit[14] : Flow control mode */

	/* reg7 data */
	data[7] = (GT_U16)((res->daNrlEn)			|	/* Bit[0]  : DA Nrl En */
					(res->saNrlEn << 1)			|	/* Bit[1]  : SA Nrl En */
					(res->mgmtNrlEn << 2) 		|	/* Bit[2]  : MGMT Nrl En */
					(res->priMask << 8) 		|	/* Bit[11:8] : Priority Queue Mask */
					(res->priORpt << 12) 		|	/* Bit[12] : Priority OR PacketType */
					(res->accountFiltered << 14)|	/* Bit[14] : Account Filtered */
					(res->accountQConf << 15));		/* Bit[15] : Account QConf */

	for(i=0; i<8; i++)
	{
		opData.irlPort = irlPort;
		opData.irlRes = irlRes;
		opData.irlReg = i;
		opData.irlData = data[i];

		retVal = pirl2OperationPerform(dev, op, &opData);
		if (retVal != GT_OK)
		{
    	    DBG_INFO(("PIRL OP Failed.\n"));
        	return retVal;
		}
	}

	return GT_OK;	
}


/*******************************************************************************
* pirl2ReadResource
*
* DESCRIPTION:
*       This function reads IRL Resource from BCM (Bucket Configuration Memory)
*
* INPUTS:
*       irlPort  - physical port number.
*		irlRes   - bucket to be used (0 ~ 4).
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
static GT_STATUS pirl2ReadResource
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				irlPort,
	IN	GT_U32				irlRes,
    OUT GT_PIRL2_RESOURCE	*res
)
{
    GT_STATUS       retVal;		/* Functions return value */
    GT_U16          data[8]; 	/* temporary Data storage */
	GT_PIRL2_OPERATION	op;
	GT_PIRL2_OP_DATA	opData;
	int				i;

	op = PIRL_READ_RESOURCE;

	for(i=0; i<8; i++)
	{
		opData.irlPort = irlPort;
		opData.irlRes = irlRes;
		opData.irlReg = i;
		opData.irlData = 0;

		retVal = pirl2OperationPerform(dev, op, &opData);
		if (retVal != GT_OK)
		{
    	    DBG_INFO(("PIRL OP Failed.\n"));
        	return retVal;
		}

		data[i] = opData.irlData;
	}
	

	/* reg0 data */
	res->bktRateType = (data[0] >> 15) & 0x1;
	res->bktTypeMask = (data[0] >> 0) & 0x7FFF;

	/* reg1 data */
	res->bktIncrement = data[1] & 0xFFF;

	/* reg2 data */
	res->bktRateFactor = data[2] & 0xFFFF;

	/* reg3,4 data */
	res->byteTobeCounted = (data[3] >> 2) & 0x3;
	res->cbsLimit = ((data[3] >> 4) & 0xFFF) | ((data[4] & 0xFFF) << 12);

	/* reg5,6 data */
	res->ebsLimit = data[5] | ((data[6] & 0xFF) << 16);
													   
	/* reg6 data */
	res->samplingMode = (data[6] >> 11) & 0x1;
	res->ebsLimitAction = (data[6] >> 12) & 0x1;
	res->actionMode = (data[6] >> 13) & 0x1;
	res->fcDeassertMode = (data[6] >> 14) & 0x1;

	/* reg7 data */
	res->daNrlEn = (data[7] >> 0) & 0x1;
	res->saNrlEn = (data[7] >> 1) & 0x1;
	res->mgmtNrlEn = (data[7] >> 2) & 0x1;
	res->priMask = (data[7] >> 8) & 0xF;
	res->priORpt = (data[7] >> 12) & 0x1;
	res->accountFiltered = (data[7] >> 14) & 0x1;
	res->accountQConf = (data[7] >> 15) & 0x1;

	return GT_OK;
}

#define PIRL2_DEBUG
#ifdef PIRL2_DEBUG
/*******************************************************************************
* pirl2DumpResource
*
* DESCRIPTION:
*       This function dumps IRL Resource register values.
*
* INPUTS:
*       irlPort  - physical port number.
*		irlRes   - bucket to be used (0 ~ 4).
*		dataLen  - data size.
*
* OUTPUTS:
*       data - IRL Resource data
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL otherwise.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS pirl2DumpResource
(
    IN  GT_QD_DEV  			*dev,
	IN	GT_U32				irlPort,
	IN	GT_U32				irlRes,
	IN	GT_U32				dataLen,
    OUT GT_U16				*data
)
{
    GT_STATUS       retVal;		/* Functions return value */
	GT_PIRL2_OPERATION	op;
	GT_PIRL2_OP_DATA	opData;
	int				i;

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_PIRL2_RESOURCE))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	op = PIRL_READ_RESOURCE;

	for(i=0; i<dataLen; i++)
	{
		opData.irlPort = irlPort;
		opData.irlRes = irlRes;
		opData.irlReg = i;
		opData.irlData = 0;

		retVal = pirl2OperationPerform(dev, op, &opData);
		if (retVal != GT_OK)
		{
    	    DBG_INFO(("PIRL OP Failed.\n"));
        	return retVal;
		}

		data[i] = opData.irlData;
	}

	return GT_OK;	
}
#endif /* PIRL2_DEBUG */

