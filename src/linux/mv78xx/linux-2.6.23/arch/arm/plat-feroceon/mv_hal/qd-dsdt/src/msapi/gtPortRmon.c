#include <Copyright.h>

/*******************************************************************************
* gtPortCounter.c
*
* DESCRIPTION:
*       API definitions for RMON counters
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
static GT_STATUS statsOperationPerform
(
    IN   GT_QD_DEV            *dev,
    IN   GT_STATS_OPERATION   statsOp,
    IN   GT_U8                port,
    IN   GT_STATS_COUNTERS    counter,
    OUT  GT_VOID              *statsData
);

static GT_STATUS statsCapture
(
    IN GT_QD_DEV  *dev,
    IN GT_U8      port
);

static GT_STATUS statsReadCounter
(
    IN   GT_QD_DEV		*dev,
    IN   GT_U32			counter,
    OUT  GT_U32			*statsData
);

static GT_STATUS statsReadRealtimeCounter
(
    IN   GT_QD_DEV      *dev,
    IN   GT_U8 		    port,
    IN   GT_U32			counter,
    OUT  GT_U32		    *statsData
);


/*******************************************************************************
* gstatsFlushAll
*
* DESCRIPTION:
*       Flush All counters for all ports.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*
* COMMENTS:
*       None
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsFlushAll
(
        IN GT_QD_DEV  *dev
)
{
	GT_STATUS       	retVal;

	DBG_INFO(("gstatsFlushAll Called.\n"));

    /* check if device supports this feature */
	if((retVal = IS_VALID_API_CALL(dev,1, DEV_RMON)) != GT_OK)
	{
		return retVal;
	}

	retVal = statsOperationPerform(dev,STATS_FLUSH_ALL,0,0,NULL);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("Failed (statsOperationPerform returned GT_FAIL).\n"));
    	return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gstatsFlushPort
*
* DESCRIPTION:
*       Flush All counters for a given port.
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*
* COMMENTS:
*       None
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsFlushPort
(
    IN GT_QD_DEV  *dev,
    IN GT_LPORT	  port
)
{
	GT_STATUS	retVal;
    GT_U8		hwPort;         /* physical port number         */

	DBG_INFO(("gstatsFlushPort Called.\n"));

    /* translate logical port to physical port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_RMON)) != GT_OK)
	{
		return retVal;
	}

	retVal = statsOperationPerform(dev,STATS_FLUSH_PORT,hwPort,0,NULL);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("Failed (statsOperationPerform returned GT_FAIL).\n"));
    	return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}

/*******************************************************************************
* gstatsGetPortCounter
*
* DESCRIPTION:
*		This routine gets a specific counter of the given port
*
* INPUTS:
*		port - the logical port number.
*		counter - the counter which will be read
*
* OUTPUTS:
*		statsData - points to 32bit data storage for the MIB counter
*
* RETURNS:
*		GT_OK      - on success
*		GT_FAIL    - on error
*
* COMMENTS:
*		None
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsGetPortCounter
(
	IN  GT_QD_DEV		*dev,
	IN  GT_LPORT		port,
	IN  GT_STATS_COUNTERS	counter,
	OUT GT_U32			*statsData
)
{
    GT_STATUS	retVal;
    GT_U8		hwPort;         /* physical port number         */

	DBG_INFO(("gstatsFlushPort Called.\n"));

    /* translate logical port to physical port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_RMON)) != GT_OK)
	{
		return retVal;
	}

	/* Gigabit Switch does not support this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_RMON_TYPE_1))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	retVal = statsOperationPerform(dev,STATS_READ_COUNTER,hwPort,counter,(GT_VOID*)statsData);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("Failed (statsOperationPerform returned GT_FAIL).\n"));
    	return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gstatsGetPortAllCounters
*
* DESCRIPTION:
*       This routine gets all counters of the given port
*
* INPUTS:
*       port - the logical port number.
*
* OUTPUTS:
*       statsCounterSet - points to GT_STATS_COUNTER_SET for the MIB counters
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*
* COMMENTS:
*       None
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gstatsGetPortAllCounters
(
    IN  GT_QD_DEV               *dev,
    IN  GT_LPORT		port,
    OUT GT_STATS_COUNTER_SET	*statsCounterSet
)
{
	GT_STATUS	retVal;
    GT_U8		hwPort;         /* physical port number         */

	DBG_INFO(("gstatsFlushPort Called.\n"));

    /* translate logical port to physical port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_RMON)) != GT_OK)
	{
		return retVal;
	}

	/* Gigabit Switch does not support this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_RMON_TYPE_1))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	retVal = statsOperationPerform(dev,STATS_READ_ALL,hwPort,0,(GT_VOID*)statsCounterSet);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("Failed (statsOperationPerform returned GT_FAIL).\n"));
    	return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}

/*******************************************************************************
* gstatsGetPortCounter2
*
* DESCRIPTION:
*		This routine gets a specific counter of the given port
*
* INPUTS:
*		port - the logical port number.
*		counter - the counter which will be read
*
* OUTPUTS:
*		statsData - points to 32bit data storage for the MIB counter
*
* RETURNS:
*		GT_OK      - on success
*		GT_FAIL    - on error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gstatsGetPortCounter2
(
	IN  GT_QD_DEV		*dev,
	IN  GT_LPORT		port,
	IN  GT_STATS_COUNTERS2	counter,
	OUT GT_U32			*statsData
)
{
    GT_STATUS	retVal;
    GT_U8		hwPort;         /* physical port number         */

	DBG_INFO(("gstatsGetPortCounters2 Called.\n"));

    /* translate logical port to physical port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_RMON)) != GT_OK)
	{
		return retVal;
	}

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_RMON_TYPE_2))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	retVal = statsOperationPerform(dev,STATS_READ_COUNTER,hwPort,counter,(GT_VOID*)statsData);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("Failed (statsOperationPerform returned GT_FAIL).\n"));
    	return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gstatsGetPortAllCounters2
*
* DESCRIPTION:
*		This routine gets all counters of the given port
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		statsCounterSet - points to GT_STATS_COUNTER_SET for the MIB counters
*
* RETURNS:
*		GT_OK      - on success
*		GT_FAIL    - on error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gstatsGetPortAllCounters2
(
	IN  GT_QD_DEV		*dev,
	IN  GT_LPORT		port,
	OUT GT_STATS_COUNTER_SET2	*statsCounterSet
)
{
	GT_STATUS	retVal;
    GT_U8		hwPort;         /* physical port number         */

	DBG_INFO(("gstatsGetPortAllCounters2 Called.\n"));

    /* translate logical port to physical port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_RMON)) != GT_OK)
	{
		return retVal;
	}

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_RMON_TYPE_2))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	retVal = statsOperationPerform(dev,STATS_READ_ALL,hwPort,0,(GT_VOID*)statsCounterSet);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("Failed (statsOperationPerform returned GT_FAIL).\n"));
    	return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}

/*******************************************************************************
* gstatsGetPortCounter3
*
* DESCRIPTION:
*		This routine gets a specific counter of the given port
*
* INPUTS:
*		port - the logical port number.
*		counter - the counter which will be read
*
* OUTPUTS:
*		statsData - points to 32bit data storage for the MIB counter
*
* RETURNS:
*		GT_OK      - on success
*		GT_FAIL    - on error
*
* COMMENTS:
*		This function supports Gigabit Switch and Spinnaker family
*
*******************************************************************************/
GT_STATUS gstatsGetPortCounter3
(
	IN  GT_QD_DEV		*dev,
	IN  GT_LPORT		port,
	IN  GT_STATS_COUNTERS3	counter,
	OUT GT_U32			*statsData
)
{
    GT_STATUS	retVal;
    GT_U8		hwPort;         /* physical port number         */

	DBG_INFO(("gstatsGetPortCounters3 Called.\n"));

    /* translate logical port to physical port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_RMON)) != GT_OK)
	{
		return retVal;
	}

	/* Only 88E6093 Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_RMON_TYPE_3))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	retVal = statsOperationPerform(dev,STATS_READ_COUNTER,hwPort,counter,(GT_VOID*)statsData);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("Failed (statsOperationPerform returned GT_FAIL).\n"));
    	return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/*******************************************************************************
* gstatsGetPortAllCounters3
*
* DESCRIPTION:
*		This routine gets all counters of the given port
*
* INPUTS:
*		port - the logical port number.
*
* OUTPUTS:
*		statsCounterSet - points to GT_STATS_COUNTER_SET for the MIB counters
*
* RETURNS:
*		GT_OK      - on success
*		GT_FAIL    - on error
*
* COMMENTS:
*		This function supports Gigabit Switch and Spinnaker family
*
*******************************************************************************/
GT_STATUS gstatsGetPortAllCounters3
(
	IN  GT_QD_DEV		*dev,
	IN  GT_LPORT		port,
	OUT GT_STATS_COUNTER_SET3	*statsCounterSet
)
{
	GT_STATUS	retVal;
    GT_U8		hwPort;         /* physical port number         */

	DBG_INFO(("gstatsGetPortAllCounters3 Called.\n"));

    /* translate logical port to physical port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_RMON)) != GT_OK)
	{
		return retVal;
	}

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_RMON_TYPE_3))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	retVal = statsOperationPerform(dev,STATS_READ_ALL,hwPort,0,(GT_VOID*)statsCounterSet);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("Failed (statsOperationPerform returned GT_FAIL).\n"));
    	return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}

/*******************************************************************************
* gstatsGetHistogramMode
*
* DESCRIPTION:
*		This routine gets the Histogram Counters Mode.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		mode - Histogram Mode (GT_COUNT_RX_ONLY, GT_COUNT_TX_ONLY,
*					and GT_COUNT_RX_TX)
*
* RETURNS:
*		GT_OK           - on success
*		GT_BAD_PARAM    - on bad parameter
*		GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gstatsGetHistogramMode
(
	IN  GT_QD_DEV				*dev,
	OUT GT_HISTOGRAM_MODE	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gstatsGetHistogramMode Called.\n"));
	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH|DEV_RMON_REALTIME_SUPPORT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* Get the Histogram mode bit.                */
    retVal = hwGetGlobalRegField(dev,QD_REG_STATS_OPERATION,10,2,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	*mode = data - 1; /* Software definition starts from 0 ~ 2,
						while hardware supports the values from 1 to 3 */

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gstatsSetHistogramMode
*
* DESCRIPTION:
*		This routine sets the Histogram Counters Mode.
*
* INPUTS:
*		mode - Histogram Mode (GT_COUNT_RX_ONLY, GT_COUNT_TX_ONLY,
*					and GT_COUNT_RX_TX)
*
* OUTPUTS:
*		None.
*
* RETURNS:
*		GT_OK           - on success
*		GT_BAD_PARAM    - on bad parameter
*		GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gstatsSetHistogramMode
(
	IN GT_QD_DEV 				*dev,
	IN GT_HISTOGRAM_MODE		mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gstatsSetHistogramMode Called.\n"));
	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH|DEV_RMON_REALTIME_SUPPORT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	switch (mode)
	{
		case GT_COUNT_RX_ONLY:
		case GT_COUNT_TX_ONLY:
		case GT_COUNT_RX_TX:
			break;
		default:
	        DBG_INFO(("Failed.\n"));
    	    return GT_BAD_PARAM;
    }

	data = (GT_U16)mode + 1;

    /* Set the Histogram mode bit.                */
    retVal = hwSetGlobalRegField(dev,QD_REG_STATS_OPERATION,10,2,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gstatsGetRealtimePortCounter
*
* DESCRIPTION:
*		This routine gets a specific realtime counter of the given port
*
* INPUTS:
*		port - the logical port number.
*		counter - the counter which will be read
*
* OUTPUTS:
*		statsData - points to 32bit data storage for the MIB counter
*
* RETURNS:
*		GT_OK      - on success
*		GT_FAIL    - on error
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gstatsGetRealtimePortCounter
(
	IN  GT_QD_DEV		*dev,
	IN  GT_LPORT		port,
	IN  GT_STATS_COUNTERS3	counter,
	OUT GT_U32			*statsData
)
{
    GT_STATUS	retVal;
    GT_U8		hwPort;         /* physical port number         */

	DBG_INFO(("gstatsGetRealtimePortCounter Called.\n"));

    /* translate logical port to physical port */
    hwPort = GT_LPORT_2_PORT(port);

    /* check if device supports this feature */
	if((retVal = IS_VALID_API_CALL(dev,hwPort, DEV_RMON)) != GT_OK)
	{
		return retVal;
	}

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_RMON_REALTIME_SUPPORT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	retVal = statsOperationPerform(dev,STATS_READ_REALTIME_COUNTER,hwPort,counter,(GT_VOID*)statsData);
	if(retVal != GT_OK)
	{
	    DBG_INFO(("Failed (statsOperationPerform returned GT_FAIL).\n"));
    	return retVal;
	}

	DBG_INFO(("OK.\n"));
	return GT_OK;

}


/****************************************************************************/
/* Internal use functions.                                                  */
/****************************************************************************/

/*******************************************************************************
* statsOperationPerform
*
* DESCRIPTION:
*       This function is used by all stats control functions, and is responsible
*       to write the required operation into the stats registers.
*
* INPUTS:
*       statsOp       - The stats operation bits to be written into the stats
*                     operation register.
*       port        - port number
*       counter     - counter to be read if it's read operation
*
* OUTPUTS:
*       statsData   - points to the data storage where the MIB counter will be saved.
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL otherwise.
*
* COMMENTS:
*
*******************************************************************************/

static GT_STATUS statsOperationPerform
(
    IN   GT_QD_DEV            *dev,
    IN   GT_STATS_OPERATION   statsOp,
    IN   GT_U8                port,
    IN   GT_STATS_COUNTERS    counter,
    OUT  GT_VOID              *statsData
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data,histoData; /* Data to be set into the      */
                                    /* register.                    */
	GT_U32 statsCounter;
	GT_U32 lastCounter;
	GT_U16			portNum;

    gtSemTake(dev,dev->statsRegsSem,OS_WAIT_FOREVER);

	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH|DEV_RMON_REALTIME_SUPPORT))
    {
		lastCounter = (GT_U32)STATS_OutDiscards;
    }
	else
	{
		lastCounter = (GT_U32)STATS2_Late;
	}

	if (IS_IN_DEV_GROUP(dev,DEV_RMON_PORT_BITS))
	{
		portNum = (port + 1) << 5;
	}
	else
	{
		portNum = (GT_U16)port;
	}

    /* Wait until the stats in ready. */
    data = 1;
    while(data == 1)
    {
        retVal = hwGetGlobalRegField(dev,QD_REG_STATS_OPERATION,15,1,&data);
        if(retVal != GT_OK)
        {
            gtSemGive(dev,dev->statsRegsSem);
            return retVal;
        }
    }

    /* Get the Histogram mode bit.                */
	retVal = hwReadGlobalReg(dev,QD_REG_STATS_OPERATION,&histoData);
    if(retVal != GT_OK)
    {
        gtSemGive(dev,dev->statsRegsSem);
        return retVal;
    }

	histoData &= 0xC00;

    /* Set the STAT Operation register */
	switch (statsOp)
	{
		case STATS_FLUSH_ALL:
			data = (1 << 15) | (GT_STATS_FLUSH_ALL << 12) | histoData;
			retVal = hwWriteGlobalReg(dev,QD_REG_STATS_OPERATION,data);
			gtSemGive(dev,dev->statsRegsSem);
			return retVal;

		case STATS_FLUSH_PORT:
			data = (1 << 15) | (GT_STATS_FLUSH_PORT << 12) | portNum | histoData;
			retVal = hwWriteGlobalReg(dev,QD_REG_STATS_OPERATION,data);
			gtSemGive(dev,dev->statsRegsSem);
			return retVal;

		case STATS_READ_COUNTER:
			retVal = statsCapture(dev,port);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->statsRegsSem);
				return retVal;
			}

			retVal = statsReadCounter(dev,counter,(GT_U32*)statsData);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->statsRegsSem);
				return retVal;
			}
			break;

		case STATS_READ_REALTIME_COUNTER:
			retVal = statsReadRealtimeCounter(dev,port,counter,(GT_U32*)statsData);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->statsRegsSem);
				return retVal;
			}

			break;

		case STATS_READ_ALL:
			retVal = statsCapture(dev,port);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->statsRegsSem);
				return retVal;
			}

			for(statsCounter=0; statsCounter<=lastCounter; statsCounter++)
			{
				retVal = statsReadCounter(dev,statsCounter,((GT_U32*)statsData + statsCounter));
				if(retVal != GT_OK)
				{
					gtSemGive(dev,dev->statsRegsSem);
					return retVal;
				}
			}
			break;

		default:

			gtSemGive(dev,dev->statsRegsSem);
			return GT_FAIL;
	}

    gtSemGive(dev,dev->statsRegsSem);
    return GT_OK;
}


/*******************************************************************************
* statsCapture
*
* DESCRIPTION:
*       This function is used to capture all counters of a port.
*
* INPUTS:
*       port        - port number
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL otherwise.
*
* COMMENTS:
*		If Semaphore is used, Semaphore should be acquired before this function call.
*******************************************************************************/
static GT_STATUS statsCapture
(
    IN GT_QD_DEV            *dev,
    IN GT_U8 		    port
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data, histoData;/* Data to be set into the      */
                                    /* register.                    */
	GT_U16			portNum;

	if (IS_IN_DEV_GROUP(dev,DEV_RMON_PORT_BITS))
	{
		portNum = (port + 1) << 5;
	}
	else
	{
		portNum = (GT_U16)port;
	}

    /* Get the Histogram mode bit.                */
	retVal = hwReadGlobalReg(dev,QD_REG_STATS_OPERATION,&histoData);
    if(retVal != GT_OK)
    {
        gtSemGive(dev,dev->statsRegsSem);
        return retVal;
    }

	histoData &= 0xC00;

    data = 1;
   	while(data == 1)
    {
        retVal = hwGetGlobalRegField(dev,QD_REG_STATS_OPERATION,15,1,&data);
        if(retVal != GT_OK)
   	    {
           	return retVal;
        }
   	}

	data = (1 << 15) | (GT_STATS_CAPTURE_PORT << 12) | portNum | histoData;
	retVal = hwWriteGlobalReg(dev,QD_REG_STATS_OPERATION,data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	return GT_OK;

}


/*******************************************************************************
* statsReadCounter
*
* DESCRIPTION:
*       This function is used to read a captured counter.
*
* INPUTS:
*       counter     - counter to be read if it's read operation
*
* OUTPUTS:
*       statsData   - points to the data storage where the MIB counter will be saved.
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL otherwise.
*
* COMMENTS:
*		If Semaphore is used, Semaphore should be acquired before this function call.
*******************************************************************************/
static GT_STATUS statsReadCounter
(
    IN   GT_QD_DEV      *dev,
    IN   GT_U32			counter,
    OUT  GT_U32		    *statsData
)
{
    GT_STATUS   retVal;         /* Functions return value.            */
    GT_U16      data, histoData;/* Data to be set into the  register. */
    GT_U16	counter3_2;     /* Counter Register Bytes 3 & 2       */
    GT_U16	counter1_0;     /* Counter Register Bytes 1 & 0       */

    /* Get the Histogram mode bit.                */
	retVal = hwReadGlobalReg(dev,QD_REG_STATS_OPERATION,&histoData);
    if(retVal != GT_OK)
    {
        gtSemGive(dev,dev->statsRegsSem);
        return retVal;
    }

	histoData &= 0xC00;

    data = 1;
   	while(data == 1)
    {
        retVal = hwGetGlobalRegField(dev,QD_REG_STATS_OPERATION,15,1,&data);
        if(retVal != GT_OK)
   	    {
           	return retVal;
        }
   	}

	data = (1 << 15) | (GT_STATS_READ_COUNTER << 12) | counter | histoData;
	retVal = hwWriteGlobalReg(dev,QD_REG_STATS_OPERATION,data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

    data = 1;
   	while(data == 1)
    {
   	 retVal = hwGetGlobalRegField(dev,QD_REG_STATS_OPERATION,15,1,&data);
        if(retVal != GT_OK)
   	    {
           	return retVal;
        }
   	}

	retVal = hwReadGlobalReg(dev,QD_REG_STATS_COUNTER3_2,&counter3_2);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	retVal = hwReadGlobalReg(dev,QD_REG_STATS_COUNTER1_0,&counter1_0);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	*statsData = (counter3_2 << 16) | counter1_0;

	return GT_OK;

}


/*******************************************************************************
* statsReadRealtimeCounter
*
* DESCRIPTION:
*       This function is used to read a realtime counter.
*
* INPUTS:
*       port     - port to be accessed
*       counter  - counter to be read if it's read operation
*
* OUTPUTS:
*       statsData   - points to the data storage where the MIB counter will be saved.
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL otherwise.
*
* COMMENTS:
*		If Semaphore is used, Semaphore should be acquired before this function call.
*******************************************************************************/
static GT_STATUS statsReadRealtimeCounter
(
    IN   GT_QD_DEV      *dev,
    IN   GT_U8 		    port,
    IN   GT_U32			counter,
    OUT  GT_U32		    *statsData
)
{
    GT_STATUS   retVal;         /* Functions return value.            */
    GT_U16      data, histoData;/* Data to be set into the  register. */
    GT_U16	counter3_2;     /* Counter Register Bytes 3 & 2       */
    GT_U16	counter1_0;     /* Counter Register Bytes 1 & 0       */

    /* Get the Histogram mode bit.                */
	retVal = hwReadGlobalReg(dev,QD_REG_STATS_OPERATION,&histoData);
    if(retVal != GT_OK)
    {
        gtSemGive(dev,dev->statsRegsSem);
        return retVal;
    }

	histoData &= 0xC00;

    data = 1;
   	while(data == 1)
    {
        retVal = hwGetGlobalRegField(dev,QD_REG_STATS_OPERATION,15,1,&data);
        if(retVal != GT_OK)
   	    {
           	return retVal;
        }
   	}

	data = (1 << 15) | (GT_STATS_READ_COUNTER << 12) | ((port+1) << 5) | counter | histoData;
	retVal = hwWriteGlobalReg(dev,QD_REG_STATS_OPERATION,data);
	if(retVal != GT_OK)
	{
		return retVal;
	}

    data = 1;
   	while(data == 1)
    {
   	 retVal = hwGetGlobalRegField(dev,QD_REG_STATS_OPERATION,15,1,&data);
        if(retVal != GT_OK)
   	    {
           	return retVal;
        }
   	}

	retVal = hwReadGlobalReg(dev,QD_REG_STATS_COUNTER3_2,&counter3_2);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	retVal = hwReadGlobalReg(dev,QD_REG_STATS_COUNTER1_0,&counter1_0);
	if(retVal != GT_OK)
	{
		return retVal;
	}

	*statsData = (counter3_2 << 16) | counter1_0;

	return GT_OK;

}
