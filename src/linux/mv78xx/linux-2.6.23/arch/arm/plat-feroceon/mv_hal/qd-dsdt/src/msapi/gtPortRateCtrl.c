#include <Copyright.h>

/********************************************************************************
* gtPortRateCtrl.c
*
* DESCRIPTION:
*       API definitions to handle port rate control registers (0xA).
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 5 $
*******************************************************************************/

#include <msApi.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>

/*
 Convert given hw Rate Limit to sw defined Rate Limit.
 This routine is only for Gigabit Managed Switch Device.
 If the given device is not an accepted device, it'll simply copy the hw limit
 to sw limit.
*/
static GT_STATUS cRateLimit(GT_QD_DEV *dev, GT_U32 hwLimit, GT_U32* swLimit)
{
	GT_U32 sLimit, hLimit, startLimit, endLimit, i;

	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH|DEV_ENHANCED_FE_SWITCH))
	{
		*swLimit = hwLimit;
		return GT_OK;
	}

	if(hwLimit == 0)
	{
		*swLimit = GT_NO_LIMIT;
		return GT_OK;
	}

	sLimit = 1000;

	if (IS_IN_DEV_GROUP(dev,DEV_ENHANCED_FE_SWITCH))
		hLimit = GT_GET_RATE_LIMIT3(sLimit);
	else if (!IS_IN_DEV_GROUP(dev,DEV_88E6183_FAMILY))
		hLimit = GT_GET_RATE_LIMIT2(sLimit);
	else
		hLimit = GT_GET_RATE_LIMIT(sLimit);
	if(hLimit == hwLimit)
	{
		*swLimit = GT_1M;
		return GT_OK;
	}

	if(hLimit > hwLimit)
	{
		startLimit = 2000;
		endLimit = 256000;
		*swLimit = GT_2M;
	}
	else
	{
		startLimit = 128;
		endLimit = 512;
		*swLimit = GT_128K;
	}

	i = 0;
	for(sLimit=startLimit;sLimit<=endLimit;sLimit *= 2, i++)
	{
		if (IS_IN_DEV_GROUP(dev,DEV_ENHANCED_FE_SWITCH))
			hLimit = GT_GET_RATE_LIMIT3(sLimit);
		else if (!IS_IN_DEV_GROUP(dev,DEV_88E6183_FAMILY))
			hLimit = GT_GET_RATE_LIMIT2(sLimit);
		else
			hLimit = GT_GET_RATE_LIMIT(sLimit);

		if(hLimit == 0)
			hLimit = 1;

		if(hLimit == hwLimit)
		{
			*swLimit += i;
			return GT_OK;
		}

		if(hLimit < hwLimit)
			break;
	}

	*swLimit = hwLimit;
	return GT_OK;
}


/*
 Convert given sw defined Burst Rate to meaningful number.
*/
static GT_STATUS cBurstEnum2Number(GT_QD_DEV *dev, GT_BURST_RATE rate, GT_U32 *rLimit)
{
	GT_U32 rateLimit;

	switch(rate)
	{
		case GT_BURST_NO_LIMIT :
				rateLimit = 0; /* MAX_RATE_LIMIT; */
				break;
		case GT_BURST_64K :
				rateLimit = 64;
				break;
		case GT_BURST_128K :
				rateLimit = 128;
				break;
		case GT_BURST_256K :
				rateLimit = 256;
				break;
		case GT_BURST_384K :
				rateLimit = 384;
				break;
		case GT_BURST_512K :
				rateLimit = 512;
				break;
		case GT_BURST_640K :
				rateLimit = 640;
				break;
		case GT_BURST_768K :
				rateLimit = 768;
				break;
		case GT_BURST_896K :
				rateLimit = 896;
				break;
		case GT_BURST_1M :
				rateLimit = 1000;
				break;
		case GT_BURST_1500K :
				rateLimit = 1500;
				break;
		case GT_BURST_2M :
				rateLimit = 2000;
				break;
		case GT_BURST_4M :
				rateLimit = 4000;
				break;
		case GT_BURST_8M :
				rateLimit = 8000;
				break;
		case GT_BURST_16M :
				rateLimit = 16000;
				break;
		case GT_BURST_32M :
				rateLimit = 32000;
				break;
		case GT_BURST_64M :
				rateLimit = 64000;
				break;
		case GT_BURST_128M :
				rateLimit = 128000;
				break;
		case GT_BURST_256M :
				rateLimit = 256000;
				break;
		default :
				return GT_BAD_PARAM;
	}

	*rLimit = rateLimit;
	return GT_OK;
}


/*
 Convert given hw Burst Rate Limit to sw defined Burst Rate Limit.
*/
static GT_STATUS cBurstRateLimit(GT_QD_DEV *dev, GT_U32 burstSize, GT_U32 hwLimit, GT_BURST_RATE* swLimit)
{
	GT_BURST_RATE sLimit, startLimit, endLimit;
	GT_U32 rLimit, tmpLimit;
    GT_STATUS       retVal;         /* Functions return value.      */

	if(hwLimit == 0)
	{
		*swLimit = GT_BURST_NO_LIMIT;
		return GT_OK;
	}

	startLimit = GT_BURST_64K;
	endLimit = GT_BURST_256M;

	for(sLimit=startLimit;sLimit<=endLimit;sLimit++)
	{
		if((retVal = cBurstEnum2Number(dev, sLimit, &rLimit)) != GT_OK)
		{
        	DBG_INFO(("Failed.\n"));
	   	    return retVal;
		}

		tmpLimit = GT_GET_BURST_RATE_LIMIT(burstSize,rLimit);

		if(hwLimit == tmpLimit)
		{
			*swLimit = sLimit;
			return GT_OK;
		}
	}

	return GT_FAIL;
}


/*
 Convert given sw defined Burst Rate to meaningful number.
*/
static GT_STATUS cTCPBurstRate(GT_QD_DEV *dev, GT_BURST_RATE rate, GT_U32 *data)
{
	switch(rate)
	{
		case GT_BURST_NO_LIMIT :
				*data = 0; /* MAX_RATE_LIMIT; */
				break;
		case GT_BURST_64K :
				*data = 0x1D00;
				break;
		case GT_BURST_128K :
				*data = 0x3FFF;
				break;
		case GT_BURST_256K :
				*data = 0x7FFF;
				break;
		case GT_BURST_384K :
				*data = 0x7DE0;
				break;
		case GT_BURST_512K :
				*data = 0x76F0;
				break;
		case GT_BURST_640K :
				*data = 0x7660;
				break;
		case GT_BURST_768K :
				*data = 0x7600;
				break;
		case GT_BURST_896K :
				*data = 0x74EF;
				break;
		case GT_BURST_1M :
				*data = 0x7340;
				break;
		case GT_BURST_1500K :
				*data = 0x7300;
				break;
		default :
				return GT_BAD_PARAM;
	}

	return GT_OK;
}

static GT_STATUS setEnhancedERate(GT_QD_DEV *dev, GT_LPORT port, GT_ERATE_TYPE *rateType)
{
    GT_STATUS	retVal;         /* Functions return value.      */
	GT_U16		data;
	GT_U32		rate, eDec;
	GT_PIRL_ELIMIT_MODE		mode;
    GT_U8		phyPort;        /* Physical port.               */

    phyPort = GT_LPORT_2_PORT(port);

	if((retVal = grcGetELimitMode(dev,port,&mode)) != GT_OK)
	{
		return retVal;
	}

	if (mode == GT_PIRL_ELIMIT_FRAME)
	{
		/* Count Per Frame */
		rate = rateType->fRate;

		if (rate == 0) /* disable egress rate limit */
		{
			eDec = 0;
			data = 0;
		}
		else if((rate < 7600)  || (rate > 1488000))
		{
			return GT_BAD_PARAM;
		}
		else
		{
			eDec = 1;
			data = GT_GET_RATE_LIMIT_PER_FRAME(rate,eDec);
		}
	}
	else
	{
		/* Count Per Byte */
		rate = rateType->kbRate;

		if(rate == 0)
		{
			eDec = 0;
		}
		else if(rate < 1000)	/* less than 1Mbps */
		{
			/* it should be divided by 64 */
			if(rate % 64)
				return GT_BAD_PARAM;
			eDec = rate/64;
		}
		else if(rate <= 100000)	/* less than or equal to 100Mbps */
		{
			/* it should be divided by 1000 */
			if(rate % 1000)
				return GT_BAD_PARAM;
			eDec = rate/1000;
		}
		else if(rate <= 1000000)	/* less than or equal to 1000Mbps */
		{
			/* it should be divided by 10000 */
			if(rate % 10000)
				return GT_BAD_PARAM;
			eDec = rate/10000;
		}
		else
			return GT_BAD_PARAM;

		if(rate == 0)
		{
			data = 0;
		}
		else
		{
			data = GT_GET_RATE_LIMIT_PER_BYTE(rate,eDec);
		}
	}

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_RATE_CTRL0,0,7,(GT_U16)eDec);
	if(retVal != GT_OK)
	{
    	DBG_INFO(("Failed.\n"));
    	return retVal;
	}

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_EGRESS_RATE_CTRL,0,12,(GT_U16)data );
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


static GT_STATUS getEnhancedERate(GT_QD_DEV *dev, GT_LPORT port, GT_ERATE_TYPE *rateType)
{
    GT_STATUS	retVal;         /* Functions return value.      */
	GT_U16		rate, eDec;
	GT_PIRL_ELIMIT_MODE		mode;
    GT_U8		phyPort;        /* Physical port.               */

    phyPort = GT_LPORT_2_PORT(port);

	if((retVal = grcGetELimitMode(dev,port,&mode)) != GT_OK)
	{
		return retVal;
	}

    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL0,0,7,&eDec);
	if(retVal != GT_OK)
	{
    	DBG_INFO(("Failed.\n"));
    	return retVal;
	}

    retVal = hwGetPortRegField(dev,phyPort,QD_REG_EGRESS_RATE_CTRL,0,12,&rate );
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }

	if (mode == GT_PIRL_ELIMIT_FRAME)
	{
		rateType->fRate = GT_GET_RATE_LIMIT_PER_FRAME(rate,eDec);
	}
	else
	{
		/* Count Per Byte */
		rateType->kbRate = GT_GET_RATE_LIMIT_PER_BYTE(rate,eDec);
	}

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* grcSetLimitMode
*
* DESCRIPTION:
*       This routine sets the port's rate control ingress limit mode.
*
* INPUTS:
*       port	- logical port number.
*       mode 	- rate control ingress limit mode.
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
GT_STATUS grcSetLimitMode
(
    IN GT_QD_DEV             *dev,
    IN GT_LPORT 	     port,
    IN GT_RATE_LIMIT_MODE    mode
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcSetLimitMode Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK )
      return retVal;

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH))
	{
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_EGRESS_RATE_CTRL,14,2,(GT_U16)mode );
	}
	else
	{
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,14,2,(GT_U16)mode );
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
* grcGetLimitMode
*
* DESCRIPTION:
*       This routine gets the port's rate control ingress limit mode.
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       mode 	- rate control ingress limit mode.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcGetLimitMode
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_RATE_LIMIT_MODE    *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcGetLimitMode Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK )
      return retVal;

    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH))
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_EGRESS_RATE_CTRL,14,2,&data );
	}
	else
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,14,2,&data );
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

/*******************************************************************************
* grcSetPri3Rate
*
* DESCRIPTION:
*       This routine sets the ingress data rate limit for priority 3 frames.
*       Priority 3 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       mode - the priority 3 frame rate limit mode
*              GT_FALSE: use the same rate as Pri2Rate
*              GT_TRUE:  use twice the rate as Pri2Rate
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
GT_STATUS grcSetPri3Rate
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcSetPri3Rate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK )
      return retVal;

    BOOL_2_BIT(mode,data);

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH))
	{
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,14,1,data );
	}
	else
	{
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,13,1,data);
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
* grcGetPri3Rate
*
* DESCRIPTION:
*       This routine gets the ingress data rate limit for priority 3 frames.
*       Priority 3 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - the priority 3 frame rate limit mode
*              GT_FALSE: use the same rate as Pri2Rate
*              GT_TRUE:  use twice the rate as Pri2Rate
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
GT_STATUS grcGetPri3Rate
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcGetPri3Rate Called.\n"));
    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK )
      return retVal;

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH))
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,14,1,&data );
	}
	else
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,13,1,&data);
	}
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
* grcSetPri2Rate
*
* DESCRIPTION:
*       This routine sets the ingress data rate limit for priority 2 frames.
*       Priority 2 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       mode - the priority 2 frame rate limit mode
*              GT_FALSE: use the same rate as Pri1Rate
*              GT_TRUE:  use twice the rate as Pri1Rate
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
GT_STATUS grcSetPri2Rate
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT port,
    IN GT_BOOL  mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcSetPri2Rate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK )
      return retVal;

    BOOL_2_BIT(mode,data);

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH))
	{
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,13,1,data );
	}
	else
	{
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,12,1,data);
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
* grcGetPri2Rate
*
* DESCRIPTION:
*       This routine gets the ingress data rate limit for priority 2 frames.
*       Priority 2 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - the priority 2 frame rate limit mode
*              GT_FALSE: use the same rate as Pri1Rate
*              GT_TRUE:  use twice the rate as Pri1Rate
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
GT_STATUS grcGetPri2Rate
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcGetPri2Rate Called.\n"));
    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK )
      return retVal;

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH))
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,13,1,&data );
	}
	else
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,12,1,&data);
	}
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
* grcSetPri1Rate
*
* DESCRIPTION:
*       This routine sets the ingress data rate limit for priority 1 frames.
*       Priority 1 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*       mode - the priority 1 frame rate limit mode
*              GT_FALSE: use the same rate as Pri0Rate
*              GT_TRUE:  use twice the rate as Pri0Rate
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
GT_STATUS grcSetPri1Rate
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

    DBG_INFO(("grcSetPri1Rate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);
    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK )
      return retVal;

    BOOL_2_BIT(mode,data);

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH))
	{
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,12,1,data );
	}
	else
	{
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,11,1,data);
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
* grcGetPri1Rate
*
* DESCRIPTION:
*       This routine gets the ingress data rate limit for priority 1 frames.
*       Priority 1 frames will be discarded after the ingress rate selection
*       is reached or exceeded.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       mode - the priority 1 frame rate limit mode
*              GT_FALSE: use the same rate as Pri0Rate
*              GT_TRUE:  use twice the rate as Pri0Rate
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
GT_STATUS grcGetPri1Rate
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_BOOL  *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcGetPri1Rate Called.\n"));
    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    phyPort = GT_LPORT_2_PORT(port);
    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS)) != GT_OK )
      return retVal;

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH))
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,12,1,&data );
	}
	else
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,11,1,&data);
	}
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
* grcSetPri0Rate
*
* DESCRIPTION:
*       This routine sets the port's ingress data limit for priority 0 frames.
*
* INPUTS:
*       port	- logical port number.
*       rate    - ingress data rate limit for priority 0 frames. These frames
*       	  will be discarded after the ingress rate selected is reached
*       	  or exceeded.
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
*			GT_16M, GT_32M, GT_64M, GT_128M, and GT_256M in GT_PRI0_RATE enum
*			are supported only by Gigabit Ethernet Switch.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcSetPri0Rate
(
    IN GT_QD_DEV       *dev,
    IN GT_LPORT        port,
    IN GT_PRI0_RATE    rate
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
	GT_U32			rateLimit, tmpLimit;

    DBG_INFO(("grcSetPri0Rate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS|DEV_UNMANAGED_SWITCH)) != GT_OK )
      return retVal;

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH))
	{
		switch(rate)
		{
			case GT_NO_LIMIT :
					rateLimit = 0; /* MAX_RATE_LIMIT; */
					break;
			case GT_128K :
					rateLimit = 128;
					break;
			case GT_256K :
					rateLimit = 256;
					break;
			case GT_512K :
					rateLimit = 512;
					break;
			case GT_1M :
					rateLimit = 1000;
					break;
			case GT_2M :
					rateLimit = 2000;
					break;
			case GT_4M :
					rateLimit = 4000;
					break;
			case GT_8M :
					rateLimit = 8000;
					break;
			case GT_16M :
					rateLimit = 16000;
					break;
			case GT_32M :
					rateLimit = 32000;
					break;
			case GT_64M :
					rateLimit = 64000;
					break;
			case GT_128M :
					rateLimit = 128000;
					break;
			case GT_256M :
					rateLimit = 256000;
					break;
			default :
					return GT_BAD_PARAM;
/*
					rateLimit = (GT_U32)rate;
					break;
*/
		}

		if (!IS_IN_DEV_GROUP(dev,DEV_88E6183_FAMILY))
			tmpLimit = GT_GET_RATE_LIMIT2(rateLimit);
		else
			tmpLimit = GT_GET_RATE_LIMIT(rateLimit);

		if((tmpLimit == 0) && (rateLimit != 0))
			rateLimit = 1;
		else
			rateLimit = tmpLimit;

	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,0,12,(GT_U16)rateLimit );
	    if(retVal != GT_OK)
    	{
	        DBG_INFO(("Failed.\n"));
    	    return retVal;
	    }
	}
	else
	{
		switch(rate)
		{
			case GT_NO_LIMIT :
			case GT_128K :
			case GT_256K :
			case GT_512K :
			case GT_1M :
			case GT_2M :
			case GT_4M :
			case GT_8M :
					break;
			default :
					return GT_BAD_PARAM;
		}
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,8,3,(GT_U16)rate );
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
* grcGetPri0Rate
*
* DESCRIPTION:
*       This routine gets the port's ingress data limit for priority 0 frames.
*
* INPUTS:
*       port	- logical port number to set.
*
* OUTPUTS:
*       rate    - ingress data rate limit for priority 0 frames. These frames
*       	  will be discarded after the ingress rate selected is reached
*       	  or exceeded.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*			GT_16M, GT_32M, GT_64M, GT_128M, and GT_256M in GT_PRI0_RATE enum
*			are supported only by Gigabit Ethernet Switch.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcGetPri0Rate
(
    IN  GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_PRI0_RATE    *rate
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */
	GT_U32			tmpLimit;

    DBG_INFO(("grcGetPri0Rate Called.\n"));

    if(rate == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS|DEV_UNMANAGED_SWITCH)) != GT_OK )
      return retVal;

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,0,12,&data);
		tmpLimit = (GT_U32)data;

	    if(retVal != GT_OK)
    	{
	        DBG_INFO(("Failed.\n"));
    	    return retVal;
	    }

		cRateLimit(dev, tmpLimit, (GT_U32*)rate);
	}
	else
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,8,3,&data );
	    if(retVal != GT_OK)
    	{
	        DBG_INFO(("Failed.\n"));
    	    return retVal;
	    }
	    *rate = data;
	}

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* grcSetBytesCount
*
* DESCRIPTION:
*       This routine sets the byets to count for limiting needs to be determined
*
* INPUTS:
*       port	  - logical port number to set.
*    	limitMGMT - GT_TRUE: To limit and count MGMT frame bytes
*    		    GT_FALSE: otherwise
*    	countIFG  - GT_TRUE: To count IFG bytes
*    		    GT_FALSE: otherwise
*    	countPre  - GT_TRUE: To count Preamble bytes
*    		    GT_FALSE: otherwise
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
GT_STATUS grcSetBytesCount
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT  port,
    IN GT_BOOL 	 limitMGMT,
    IN GT_BOOL 	 countIFG,
    IN GT_BOOL 	 countPre
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
    GT_U16          data;           /* data for bytes count         */

    DBG_INFO(("grcSetBytesCount Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS|DEV_UNMANAGED_SWITCH)) != GT_OK )
      return retVal;

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
	    BOOL_2_BIT(limitMGMT,data);
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,15,1,data );
		if (retVal != GT_OK)
			return retVal;

		data = 0;
		if( countIFG == GT_TRUE ) data |= 2;
		if( countPre == GT_TRUE ) data |= 1;

	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_EGRESS_RATE_CTRL,12,2,data );
	}
	else
	{
		data = 0;
	    if(	limitMGMT == GT_TRUE ) data |=4;
    	if(	 countIFG == GT_TRUE ) data |=2;
	    if(	 countPre == GT_TRUE ) data |=1;

	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,4,3,data );
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
* grcGetBytesCount
*
* DESCRIPTION:
*       This routine gets the byets to count for limiting needs to be determined
*
* INPUTS:
*       port	- logical port number
*
* OUTPUTS:
*    	limitMGMT - GT_TRUE: To limit and count MGMT frame bytes
*    		    GT_FALSE: otherwise
*    	countIFG  - GT_TRUE: To count IFG bytes
*    		    GT_FALSE: otherwise
*    	countPre  - GT_TRUE: To count Preamble bytes
*    		    GT_FALSE: otherwise
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcGetBytesCount
(
    IN GT_QD_DEV *dev,
    IN GT_LPORT  port,
    IN GT_BOOL 	 *limitMGMT,
    IN GT_BOOL 	 *countIFG,
    IN GT_BOOL 	 *countPre
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcGetBytesCount Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_INGRESS_RATE_KBPS|DEV_UNMANAGED_SWITCH)) != GT_OK )
      return retVal;

    if (limitMGMT == NULL || countIFG == NULL || countPre == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }
   	*limitMGMT = *countIFG = *countPre = GT_FALSE;

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,15,1,&data );
		if (retVal != GT_OK)
		{
    	    DBG_INFO(("Failed.\n"));
			return retVal;
		}

	    BIT_2_BOOL(data,*limitMGMT);
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_EGRESS_RATE_CTRL,12,2,&data );
		if (retVal != GT_OK)
		{
    	    DBG_INFO(("Failed.\n"));
			return retVal;
		}

		if( data & 0x2 ) *countIFG = GT_TRUE;
		if( data & 0x1 ) *countPre = GT_TRUE;

	}
	else
	{

	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,4,3,&data );
    	if(retVal != GT_OK)
	    {
    	    DBG_INFO(("Failed.\n"));
        	return retVal;
	    }

	    if ( data & 4 ) *limitMGMT = GT_TRUE;
    	if ( data & 2 ) *countIFG  = GT_TRUE;
	    if ( data & 1 ) *countPre  = GT_TRUE;

	}

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* grcSetEgressRate
*
* DESCRIPTION:
*       This routine sets the port's egress data limit.
*
*
* INPUTS:
*       port	- logical port number.
*       rateType  - egress data rate limit (GT_ERATE_TYPE union type).
*					union type is used to support multiple devices with the
*					different formats of egress rate.
*					GT_ERATE_TYPE has the following fields:
*						definedRate - GT_EGRESS_RATE enum type should used for the
*							following devices:
*							88E6218, 88E6318, 88E6063, 88E6083, 88E6181, 88E6183,
*							88E6093, 88E6095, 88E6185, 88E6108, 88E6065, 88E6061,
*							and their variations
*						kbRate - rate in kbps that should used for the following
*							devices:
*							88E6097, 88E6096 with the GT_PIRL_ELIMIT_MODE of
*								GT_PIRL_ELIMIT_LAYER1,
*								GT_PIRL_ELIMIT_LAYER2, or
*								GT_PIRL_ELIMIT_LAYER3 (see grcSetELimitMode)
*							64kbps ~ 1Mbps    : increments of 64kbps,
*							1Mbps ~ 100Mbps   : increments of 1Mbps, and
*							100Mbps ~ 1000Mbps: increments of 10Mbps
*							Therefore, the valid values are:
*								64, 128, 192, 256, 320, 384,..., 960,
*								1000, 2000, 3000, 4000, ..., 100000,
*								110000, 120000, 130000, ..., 1000000.
*						fRate - frame per second that should used for the following
*							devices:
*							88E6097, 88E6096 with GT_PIRL_ELIMIT_MODE of
*								GT_PIRL_ELIMIT_FRAME
*							Valid values are between 7600 and 1488000
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
*			GT_16M, GT_32M, GT_64M, GT_128M, and GT_256M in GT_EGRESS_RATE enum
*			are supported only by Gigabit Ethernet Switch.
*
*******************************************************************************/
GT_STATUS grcSetEgressRate
(
    IN GT_QD_DEV       *dev,
    IN GT_LPORT        port,
    IN GT_ERATE_TYPE   *rateType
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
	GT_U32			rateLimit, tmpLimit;
    GT_EGRESS_RATE  rate;

    DBG_INFO(("grcSetEgressRate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_EGRESS_RATE_KBPS|DEV_UNMANAGED_SWITCH)) != GT_OK )
      return retVal;

	if (IS_IN_DEV_GROUP(dev,DEV_ELIMIT_FRAME_BASED))
	{
		return setEnhancedERate(dev,port,rateType);
	}

	rate = rateType->definedRate;

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH|DEV_ENHANCED_FE_SWITCH))
	{
		switch(rate)
		{
			case GT_NO_LIMIT :
					rateLimit = 0; /* MAX_RATE_LIMIT; */
					break;
			case GT_128K :
					rateLimit = 128;
					break;
			case GT_256K :
					rateLimit = 256;
					break;
			case GT_512K :
					rateLimit = 512;
					break;
			case GT_1M :
					rateLimit = 1000;
					break;
			case GT_2M :
					rateLimit = 2000;
					break;
			case GT_4M :
					rateLimit = 4000;
					break;
			case GT_8M :
					rateLimit = 8000;
					break;
			case GT_16M :
					rateLimit = 16000;
					break;
			case GT_32M :
					rateLimit = 32000;
					break;
			case GT_64M :
					rateLimit = 64000;
					break;
			case GT_128M :
					rateLimit = 128000;
					break;
			case GT_256M :
					rateLimit = 256000;
					break;
			default :
					return GT_BAD_PARAM;
/*
					rateLimit = (GT_U32)rate;
					break;
*/
		}

		if (IS_IN_DEV_GROUP(dev,DEV_ENHANCED_FE_SWITCH))
			tmpLimit = GT_GET_RATE_LIMIT3(rateLimit);
		else if (!IS_IN_DEV_GROUP(dev,DEV_88E6183_FAMILY))
			tmpLimit = GT_GET_RATE_LIMIT2(rateLimit);
		else
			tmpLimit = GT_GET_RATE_LIMIT(rateLimit);

		if((tmpLimit == 0) && (rateLimit != 0))
			rateLimit = 1;
		else
			rateLimit = tmpLimit;

	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_EGRESS_RATE_CTRL,0,12,(GT_U16)rateLimit );
	    if(retVal != GT_OK)
    	{
	        DBG_INFO(("Failed.\n"));
    	    return retVal;
	    }
	}
	else
	{
		switch(rate)
		{
			case GT_NO_LIMIT :
			case GT_128K :
			case GT_256K :
			case GT_512K :
			case GT_1M :
			case GT_2M :
			case GT_4M :
			case GT_8M :
					break;
			default :
					return GT_BAD_PARAM;
		}
	    retVal = hwSetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,0,3,(GT_U16)rate );
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
* grcGetEgressRate
*
* DESCRIPTION:
*       This routine gets the port's egress data limit.
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       rateType  - egress data rate limit (GT_ERATE_TYPE union type).
*					union type is used to support multiple devices with the
*					different formats of egress rate.
*					GT_ERATE_TYPE has the following fields:
*						definedRate - GT_EGRESS_RATE enum type should used for the
*							following devices:
*							88E6218, 88E6318, 88E6063, 88E6083, 88E6181, 88E6183,
*							88E6093, 88E6095, 88E6185, 88E6108, 88E6065, 88E6061,
*							and their variations
*						kbRate - rate in kbps that should used for the following
*							devices:
*							88E6097, 88E6096 with the GT_PIRL_ELIMIT_MODE of
*								GT_PIRL_ELIMIT_LAYER1,
*								GT_PIRL_ELIMIT_LAYER2, or
*								GT_PIRL_ELIMIT_LAYER3 (see grcSetELimitMode)
*							64kbps ~ 1Mbps    : increments of 64kbps,
*							1Mbps ~ 100Mbps   : increments of 1Mbps, and
*							100Mbps ~ 1000Mbps: increments of 10Mbps
*							Therefore, the valid values are:
*								64, 128, 192, 256, 320, 384,..., 960,
*								1000, 2000, 3000, 4000, ..., 100000,
*								110000, 120000, 130000, ..., 1000000.
*						fRate - frame per second that should used for the following
*							devices:
*							88E6097, 88E6096 with GT_PIRL_ELIMIT_MODE of
*								GT_PIRL_ELIMIT_FRAME
*							Valid values are between 7600 and 1488000
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*
* COMMENTS:
*			GT_16M, GT_32M, GT_64M, GT_128M, and GT_256M in GT_EGRESS_RATE enum
*			are supported only by Gigabit Ethernet Switch.
*
*******************************************************************************/
GT_STATUS grcGetEgressRate
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_ERATE_TYPE  *rateType
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */
	GT_U32			tmpLimit,tmpRate;
    GT_EGRESS_RATE  rate;

    DBG_INFO(("grcGetEgressRate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
    if((retVal = IS_VALID_API_CALL(dev,phyPort, DEV_EGRESS_RATE_KBPS|DEV_UNMANAGED_SWITCH)) != GT_OK )
      return retVal;

    if(rateType == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

	if (IS_IN_DEV_GROUP(dev,DEV_ELIMIT_FRAME_BASED))
	{
		return getEnhancedERate(dev,port,rateType);
	}

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH|DEV_ENHANCED_FE_SWITCH))
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_EGRESS_RATE_CTRL,0,12,&data );
		tmpLimit = (GT_U32)data;
	    if(retVal != GT_OK)
    	{
	        DBG_INFO(("Failed.\n"));
    	    return retVal;
	    }

		cRateLimit(dev, tmpLimit, &tmpRate);
	    rate = (GT_EGRESS_RATE)tmpRate;
	}
	else
	{
	    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL,0,3,&data );
	    if(retVal != GT_OK)
    	{
	        DBG_INFO(("Failed.\n"));
    	    return retVal;
	    }

	    rate = (GT_EGRESS_RATE)data;
	}

	rateType->definedRate = rate;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* grcSetBurstRate
*
* DESCRIPTION:
*       This routine sets the port's ingress data limit based on burst size.
*
* INPUTS:
*       port	- logical port number.
*       bsize	- burst size.
*       rate    - ingress data rate limit. These frames will be discarded after
*				the ingress rate selected is reached or exceeded.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*								Minimum rate for Burst Size 24K byte is 128Kbps
*								Minimum rate for Burst Size 48K byte is 256Kbps
*								Minimum rate for Burst Size 96K byte is 512Kbps
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*		If the device supports both priority based Rate Limiting and burst size
*		based Rate limiting, user has to manually change the mode to burst size
*		based Rate limiting by calling gsysSetRateLimitMode.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcSetBurstRate
(
    IN GT_QD_DEV       *dev,
    IN GT_LPORT        port,
    IN GT_BURST_SIZE   bsize,
    IN GT_BURST_RATE   rate
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
	GT_U32			rateLimit;
	GT_U32			burstSize =0;

    DBG_INFO(("grcSetBurstRate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

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

	switch (bsize)
	{
		case GT_BURST_SIZE_12K:
			burstSize = 0;
			break;
		case GT_BURST_SIZE_24K:
			if ((rate < GT_BURST_128K) && (rate != GT_BURST_NO_LIMIT))
				return GT_BAD_PARAM;
			burstSize = 1;
			break;
		case GT_BURST_SIZE_48K:
			if ((rate < GT_BURST_256K) && (rate != GT_BURST_NO_LIMIT))
				return GT_BAD_PARAM;
			burstSize = 3;
			break;
		case GT_BURST_SIZE_96K:
			if ((rate < GT_BURST_512K) && (rate != GT_BURST_NO_LIMIT))
				return GT_BAD_PARAM;
			burstSize = 7;
			break;
		default:
			return GT_BAD_PARAM;
	}

	if((retVal = cBurstEnum2Number(dev, rate, &rateLimit)) != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
	}

	rateLimit = GT_GET_BURST_RATE_LIMIT(burstSize,rateLimit);

	rateLimit |= (GT_U32)(burstSize << 12);

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,0,15,(GT_U16)rateLimit );
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* grcGetBurstRate
*
* DESCRIPTION:
*       This routine retrieves the port's ingress data limit based on burst size.
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       bsize	- burst size.
*       rate    - ingress data rate limit. These frames will be discarded after
*				the ingress rate selected is reached or exceeded.
*
* RETURNS:
*       GT_OK            - on success
*       GT_FAIL          - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcGetBurstRate
(
    IN  GT_QD_DEV       *dev,
    IN  GT_LPORT        port,
    OUT GT_BURST_SIZE   *bsize,
    OUT GT_BURST_RATE   *rate
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
	GT_U32			rateLimit, burstSize;
	GT_U16			data;

    DBG_INFO(("grcGetBurstRate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

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

    retVal = hwGetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,0,15,&data);
	rateLimit = (GT_U32)data;
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }

	burstSize = rateLimit >> 12;
	rateLimit &= 0x0FFF;

	retVal = cBurstRateLimit(dev, burstSize, rateLimit, rate);
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }

	switch (burstSize)
	{
		case 0:
			*bsize = GT_BURST_SIZE_12K;
			break;
		case 1:
			*bsize = GT_BURST_SIZE_24K;
			break;
		case 3:
			*bsize = GT_BURST_SIZE_48K;
			break;
		case 7:
			*bsize = GT_BURST_SIZE_96K;
			break;
		default:
			return GT_BAD_VALUE;
	}

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* grcSetTCPBurstRate
*
* DESCRIPTION:
*       This routine sets the port's TCP/IP ingress data limit based on burst size.
*
* INPUTS:
*       port	- logical port number.
*       rate    - ingress data rate limit for TCP/IP packets. These frames will
*				be discarded after the ingress rate selected is reached or exceeded.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*								Valid rate is GT_BURST_NO_LIMIT, or between
*								64Kbps and 1500Kbps.
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*		If the device supports both priority based Rate Limiting and burst size
*		based Rate limiting, user has to manually change the mode to burst size
*		based Rate limiting by calling gsysSetRateLimitMode.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcSetTCPBurstRate
(
    IN GT_QD_DEV       *dev,
    IN GT_LPORT        port,
    IN GT_BURST_RATE   rate
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
	GT_U32			rateLimit;

    DBG_INFO(("grcSetTCPBurstRate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

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

	if((retVal = cTCPBurstRate(dev, rate, &rateLimit)) != GT_OK)
	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
	}

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,0,15,(GT_U16)rateLimit );
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* grcGetTCPBurstRate
*
* DESCRIPTION:
*       This routine sets the port's TCP/IP ingress data limit based on burst size.
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       rate    - ingress data rate limit for TCP/IP packets. These frames will
*				be discarded after the ingress rate selected is reached or exceeded.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_VALUE        - register value is not known
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*		If the device supports both priority based Rate Limiting and burst size
*		based Rate limiting, user has to manually change the mode to burst size
*		based Rate limiting by calling gsysSetRateLimitMode.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS grcGetTCPBurstRate
(
    IN  GT_QD_DEV       *dev,
    IN  GT_LPORT        port,
    OUT GT_BURST_RATE   *rate
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */
	GT_U32			rateLimit;
	GT_U32			data;
	GT_U16			u16Data;
	GT_BURST_RATE sLimit, startLimit, endLimit;

    DBG_INFO(("grcGetTCPBurstRate Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

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

    retVal = hwGetPortRegField(dev,phyPort,QD_REG_INGRESS_RATE_CTRL,0,15,&u16Data);
	data = (GT_U32)u16Data;
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }

	if ((data & 0xFFF) == 0)
	{
		*rate = GT_BURST_NO_LIMIT;
		return GT_OK;
	}

	startLimit = GT_BURST_64K;
	endLimit = GT_BURST_1500K;

	for(sLimit=startLimit;sLimit<=endLimit;sLimit++)
	{
		if((retVal = cTCPBurstRate(dev, sLimit, &rateLimit)) != GT_OK)
		{
        	break;
		}

		if(rateLimit == data)
		{
			*rate = sLimit;
			return GT_OK;
		}
	}

    DBG_INFO(("Fail to find TCP Rate.\n"));
    return GT_BAD_VALUE;
}


/*******************************************************************************
* grcSetVidNrlEn
*
* DESCRIPTION:
*       This routine enables/disables VID None Rate Limit (NRL).
*		When VID NRL is enabled and the determined VID of a frame results in a VID
*		whose VIDNonRateLimit in the VTU Table is set to GT_TURE, then the frame
*		will not be ingress nor egress rate limited.
*
* INPUTS:
*       port - logical port number.
*		mode - GT_TRUE to enable VID None Rate Limit
*			   GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS grcSetVidNrlEn
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	IN  GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;        /* Physical port.               */

    DBG_INFO(("grcSetVidNrlEn Called.\n"));

    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_NONE_RATE_LIMIT))
	{
   	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set the VidNrlEn mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_INGRESS_RATE_CTRL,15,1,data);
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }

    DBG_INFO(("OK.\n"));

    return GT_OK;
}

/*******************************************************************************
* grcGetVidNrlEn
*
* DESCRIPTION:
*       This routine gets VID None Rate Limit (NRL) mode.
*		When VID NRL is enabled and the determined VID of a frame results in a VID
*		whose VIDNonRateLimit in the VTU Table is set to GT_TURE, then the frame
*		will not be ingress nor egress rate limited.
*
* INPUTS:
*       port - logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE to enable VID None Rate Limit
*			   GT_FALSE otherwise
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS grcGetVidNrlEn
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;        /* Physical port.               */

    DBG_INFO(("grcGetVidNrlEn Called.\n"));

    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_NONE_RATE_LIMIT))
	{
   	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* Get the VidNrlEn mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_INGRESS_RATE_CTRL,15,1,&data);
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
* grcSetSaNrlEn
*
* DESCRIPTION:
*       This routine enables/disables SA None Rate Limit (NRL).
*		When SA NRL is enabled and the source address of a frame results in a ATU
*		hit where the SA's MAC address returns an EntryState that indicates Non
*		Rate Limited, then the frame will not be ingress nor egress rate limited.
*
* INPUTS:
*       port - logical port number.
*		mode - GT_TRUE to enable SA None Rate Limit
*			   GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS grcSetSaNrlEn
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	IN  GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;        /* Physical port.               */

    DBG_INFO(("grcSetSaNrlEn Called.\n"));

    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_NONE_RATE_LIMIT))
	{
   	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set the SaNrlEn mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_INGRESS_RATE_CTRL,14,1,data);
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }

    DBG_INFO(("OK.\n"));

    return GT_OK;
}

/*******************************************************************************
* grcGetSaNrlEn
*
* DESCRIPTION:
*       This routine gets SA None Rate Limit (NRL) mode.
*		When SA NRL is enabled and the source address of a frame results in a ATU
*		hit where the SA's MAC address returns an EntryState that indicates Non
*		Rate Limited, then the frame will not be ingress nor egress rate limited.
*
* INPUTS:
*       port - logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE to enable SA None Rate Limit
*			   GT_FALSE otherwise
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS grcGetSaNrlEn
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;        /* Physical port.               */

    DBG_INFO(("grcGetSaNrlEn Called.\n"));

    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_NONE_RATE_LIMIT))
	{
   	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* Get the SaNrlEn mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_INGRESS_RATE_CTRL,14,1,&data);
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
* grcSetDaNrlEn
*
* DESCRIPTION:
*       This routine enables/disables DA None Rate Limit (NRL).
*		When DA NRL is enabled and the destination address of a frame results in
*		a ATU hit where the DA's MAC address returns an EntryState that indicates
*		Non Rate Limited, then the frame will not be ingress nor egress rate
*		limited.
*
* INPUTS:
*       port - logical port number.
*		mode - GT_TRUE to enable DA None Rate Limit
*			   GT_FALSE otherwise
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS grcSetDaNrlEn
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	IN  GT_BOOL		mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;        /* Physical port.               */

    DBG_INFO(("grcSetDaNrlEn Called.\n"));

    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_NONE_RATE_LIMIT))
	{
   	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set the DaNrlEn mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_INGRESS_RATE_CTRL,13,1,data);
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }

    DBG_INFO(("OK.\n"));

    return GT_OK;
}

/*******************************************************************************
* grcGetDaNrlEn
*
* DESCRIPTION:
*       This routine gets SA None Rate Limit (NRL) mode.
*		When DA NRL is enabled and the destination address of a frame results in
*		a ATU hit where the DA's MAC address returns an EntryState that indicates
*		Non Rate Limited, then the frame will not be ingress nor egress rate
*		limited.
*
* INPUTS:
*       port - logical port number.
*
* OUTPUTS:
*		mode - GT_TRUE to enable DA None Rate Limit
*			   GT_FALSE otherwise
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS grcGetDaNrlEn
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;        /* Physical port.               */

    DBG_INFO(("grcGetDaNrlEn Called.\n"));

    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_NONE_RATE_LIMIT))
	{
   	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* Get the DaNrlEn mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_INGRESS_RATE_CTRL,13,1,&data);
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
* grcSetELimitMode
*
* DESCRIPTION:
*       This routine sets Egress Rate Limit counting mode.
*		The supported modes are as follows:
*			GT_PIRL_ELIMIT_FRAME -
*				Count the number of frames
*			GT_PIRL_ELIMIT_LAYER1 -
*				Count all Layer 1 bytes:
*				Preamble (8bytes) + Frame's DA to CRC + IFG (12bytes)
*			GT_PIRL_ELIMIT_LAYER2 -
*				Count all Layer 2 bytes: Frame's DA to CRC
*			GT_PIRL_ELIMIT_LAYER1 -
*				Count all Layer 1 bytes:
*				Frame's DA to CRC - 18 - 4 (if frame is tagged)
*
* INPUTS:
*       port - logical port number
*		mode - GT_PIRL_ELIMIT_MODE enum type
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS grcSetELimitMode
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	IN  GT_PIRL_ELIMIT_MODE		mode
)
{
	GT_U16			data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;        /* Physical port.               */

    DBG_INFO(("grcSetELimitMode Called.\n"));

    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PIRL_RESOURCE|DEV_ELIMIT_FRAME_BASED))
	{
   	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	if (!IS_IN_DEV_GROUP(dev,DEV_ELIMIT_FRAME_BASED))
	{
		if(mode == GT_PIRL_ELIMIT_FRAME)
			return GT_NOT_SUPPORTED;
	}

	data = (GT_U16)mode & 0x3;

    /* Set the Elimit mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_EGRESS_RATE_CTRL,14,2,data);
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }

    DBG_INFO(("OK.\n"));

    return GT_OK;
}


/*******************************************************************************
* grcGetELimitMode
*
* DESCRIPTION:
*       This routine gets Egress Rate Limit counting mode.
*		The supported modes are as follows:
*			GT_PIRL_ELIMIT_FRAME -
*				Count the number of frames
*			GT_PIRL_ELIMIT_LAYER1 -
*				Count all Layer 1 bytes:
*				Preamble (8bytes) + Frame's DA to CRC + IFG (12bytes)
*			GT_PIRL_ELIMIT_LAYER2 -
*				Count all Layer 2 bytes: Frame's DA to CRC
*			GT_PIRL_ELIMIT_LAYER1 -
*				Count all Layer 1 bytes:
*				Frame's DA to CRC - 18 - 4 (if frame is tagged)
*
* INPUTS:
*       port - logical port number
*
* OUTPUTS:
*		mode - GT_PIRL_ELIMIT_MODE enum type
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*		GT_NOT_SUPPORTED    - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS grcGetELimitMode
(
    IN  GT_QD_DEV	*dev,
    IN  GT_LPORT	port,
	OUT GT_PIRL_ELIMIT_MODE		*mode
)
{
	GT_U16			data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;        /* Physical port.               */

    DBG_INFO(("grcGetELimitMode Called.\n"));

    hwPort = GT_LPORT_2_PORT(port);

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_PIRL_RESOURCE|DEV_ELIMIT_FRAME_BASED))
	{
   	    DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* Get the Elimit mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_EGRESS_RATE_CTRL,14,2,&data);
    if(retVal != GT_OK)
   	{
        DBG_INFO(("Failed.\n"));
   	    return retVal;
    }

	*mode = data;

    DBG_INFO(("OK.\n"));

    return GT_OK;
}

/*******************************************************************************
* grcSetRsvdNrlEn
*
* DESCRIPTION:
*       This routine sets Reserved Non Rate Limit.
*		When this feature is enabled, frames that match the requirements of the
*		Rsvd2Cpu bit below will also be considered to be ingress and egress non
*		rate limited.
*
* INPUTS:
*       en - GT_TRUE to enable Reserved Non Rate Limit,
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
GT_STATUS grcSetRsvdNrlEn
(
    IN  GT_QD_DEV *dev,
    IN  GT_BOOL   en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("grcSetRsvdNrlEn Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_NONE_RATE_LIMIT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    BOOL_2_BIT(en,data);

    /* Set the RsvdNrl bit.            */
    retVal = hwSetGlobalRegField(dev,QD_REG_MANGEMENT_CONTROL,4,1,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* grcGetRsvdNrlEn
*
* DESCRIPTION:
*       This routine gets Reserved Non Rate Limit.
*		When this feature is enabled, frames that match the requirements of the
*		Rsvd2Cpu bit below will also be considered to be ingress and egress non
*		rate limited.
*
* INPUTS:
*       en - GT_TRUE to enable Reserved Non Rate Limit,
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
GT_STATUS grcGetRsvdNrlEn
(
    IN  GT_QD_DEV *dev,
    OUT GT_BOOL   *en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
	GT_U16			data;

    DBG_INFO(("grcGetRsvdNrlEn Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_NONE_RATE_LIMIT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the RsvdNrl bit.            */
    retVal = hwGetGlobalRegField(dev,QD_REG_MANGEMENT_CONTROL,4,1,&data);
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
* grcSetFrameOverhead
*
* DESCRIPTION:
*       Egress rate frame overhead adjustment.
*		This field is used to adjust the number of bytes that need to be added to a
*		frame's IFG on a per frame basis.
*
*		The egress rate limiter multiplies the value programmed in this field by four
*		for computing the frame byte offset adjustment value (i.e., the amount the
*		IPG is increased for every frame). This adjustment, if enabled, is made to
*		every egressing frame's IPG and it is made in addition to any other IPG
*		adjustments due to other Egress Rate Control settings.
*
*		The egress overhead adjustment can add the following number of byte times
*		to each frame's IPG: 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52,
*		56 and 60.
*
*		Example:
*		If FrameOverhead = 11, the egress rate limiter would increase the IPG
*		between every frame by an additional 44 bytes.
*
*		Note: When the Count Mode (port offset 0x0A) is in Frame based egress rate
*		shaping mode, these Frame Overhead bits must be 0x0.
*
* INPUTS:
*       port	 - logical port number.
*       overhead - Frame overhead (0 ~ 15)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*       GT_BAD_PARAM        - on bad parameters
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS grcSetFrameOverhead
(
    IN GT_QD_DEV		*dev,
    IN GT_LPORT			port,
    IN GT_32			overhead
)
{

    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcSetFrameOverhead Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_ELIMIT_FRAME_BASED))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	if (overhead > 15)
	{
        DBG_INFO(("GT_BAD_PARAM \n"));
		return GT_BAD_PARAM;
	}

    retVal = hwSetPortRegField(dev,phyPort,QD_REG_RATE_CTRL0,8,4,(GT_U16)overhead );
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* grcGetFrameOverhead
*
* DESCRIPTION:
*       Egress rate frame overhead adjustment.
*		This field is used to adjust the number of bytes that need to be added to a
*		frame's IFG on a per frame basis.
*
*		The egress rate limiter multiplies the value programmed in this field by four
*		for computing the frame byte offset adjustment value (i.e., the amount the
*		IPG is increased for every frame). This adjustment, if enabled, is made to
*		every egressing frame's IPG and it is made in addition to any other IPG
*		adjustments due to other Egress Rate Control settings.
*
*		The egress overhead adjustment can add the following number of byte times
*		to each frame's IPG: 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52,
*		56 and 60.
*
*		Example:
*		If FrameOverhead = 11, the egress rate limiter would increase the IPG
*		between every frame by an additional 44 bytes.
*
*		Note: When the Count Mode (port offset 0x0A) is in Frame based egress rate
*		shaping mode, these Frame Overhead bits must be 0x0.
*
* INPUTS:
*       port	- logical port number.
*
* OUTPUTS:
*       overhead - Frame overhead (0 ~ 15)
*
* RETURNS:
*       GT_OK            - on success
*       GT_FAIL          - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
*******************************************************************************/
GT_STATUS grcGetFrameOverhead
(
    IN GT_QD_DEV *dev,
    IN  GT_LPORT port,
    OUT GT_32    *overhead
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
    GT_U8           phyPort;        /* Physical port.               */

    DBG_INFO(("grcGetFrameOverhead Called.\n"));

    phyPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_ELIMIT_FRAME_BASED))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    retVal = hwGetPortRegField(dev,phyPort,QD_REG_RATE_CTRL0,8,4,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    *overhead = (GT_U32)data;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

