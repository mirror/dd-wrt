#include <Copyright.h>

/********************************************************************************
* gtBrgFdb.c
*
* DESCRIPTION:
*       API definitions for Multiple Forwarding Databases 
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 9 $
*******************************************************************************/

#include <msApi.h>
#include <gtSem.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>


/****************************************************************************/
/* Forward function declaration.                                            */
/****************************************************************************/
static GT_STATUS atuOperationPerform
(
    IN      GT_QD_DEV           *dev,
    IN      GT_ATU_OPERATION    atuOp,
	INOUT	GT_EXTRA_OP_DATA	*opData,
    INOUT 	GT_ATU_ENTRY    	*atuEntry
);

static GT_STATUS atuStateAppToDev
(
    IN  GT_QD_DEV	*dev,
	IN  GT_BOOL		unicast,
	IN  GT_U32		state,
	OUT GT_U32		*newOne
);

static GT_STATUS atuStateDevToApp
(
    IN  GT_QD_DEV	*dev,
	IN  GT_BOOL		unicast,
	IN  GT_U32		state,
	OUT GT_U32		*newOne
);

static GT_STATUS atuGetStats
(
    IN  GT_QD_DEV	*dev,
	IN  GT_ATU_STAT	*atuStat,
	OUT GT_U32		*count
);


/*******************************************************************************
* gfdbSetPortAtuLearnLimit
*
* DESCRIPTION:
*       Port's auto learning limit. When the limit is non-zero value, the number
*		of MAC addresses that can be learned on this port are limited to the value
*		specified in this API. When the learn limit has been reached any frame 
*		that ingresses this port with a source MAC address not already in the 
*		address database that is associated with this port will be discarded. 
*		Normal auto-learning will resume on the port as soon as the number of 
*		active unicast MAC addresses associated to this port is less than the 
*		learn limit.
*		CPU directed ATU Load, Purge, or Move will not have any effect on the 
*		learn limit.
*		This feature is disabled when the limit is zero.
*		The following care is needed when enabling this feature:
*			1) dsable learning on the ports
*			2) flush all non-static addresses in the ATU
*			3) define the desired limit for the ports
*			4) re-enable learing on the ports
*
* INPUTS:
*       port  - logical port number
*       limit - auto learning limit ( 0 ~ 255 )
*											  
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_BAD_PARAM - if limit > 0xFF
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbSetPortAtuLearnLimit
(
    IN  GT_QD_DEV 	*dev,
    IN  GT_LPORT  	port,
    IN  GT_U32   	limit
)
{
    GT_U16          data, mask;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gfdbSetPortAtuLearnLimit Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* Check device if it has fixed ATU Size. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ATU_LIMIT))
    {
		return GT_NOT_SUPPORTED;
    }

	if (limit > 0xFF)
	{
        DBG_INFO(("Bad Parameter\n"));
		return GT_BAD_PARAM;
	}

	mask = 0x80FF;
	data = (GT_U16) limit;

    /* Set the learn limit bits.                  */
    retVal = hwSetPortRegBits(dev,hwPort, QD_REG_PORT_ATU_CONTROL, mask, data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbGetPortAtuLearnCnt
*
* DESCRIPTION:
*       Read the current number of active unicast MAC addresses associated with 
*		the given port. This counter (LearnCnt) is held at zero if learn limit
*		(gfdbSetPortAtuLearnLimit API) is set to zero.
*
* INPUTS:
*       port  - logical port number
*											  
* OUTPUTS:
*       count - current auto learning count
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbGetPortAtuLearnCnt
(
    IN  GT_QD_DEV 	*dev,
    IN  GT_LPORT  	port,
    IN  GT_U32   	*count
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("gfdbGetPortAtuLearnCnt Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	/* Check device if this feature is supported. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ATU_LIMIT))
    {
		return GT_NOT_SUPPORTED;
    }

    /* Get the ReadLearnCnt bit. */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_ATU_CONTROL, 15, 1, &data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	if(data == 0)
	{
	    /* Set the ReadLearnCnt bit. */
    	retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_ATU_CONTROL, 15, 1, 1);
	    if(retVal != GT_OK)
    	{
        	DBG_INFO(("Failed.\n"));
	        return retVal;
    	}
	}

    /* Get the LearnCnt bits. */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_ATU_CONTROL, 0, 8, &data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	*count = (GT_U32)data;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbGetAtuAllCount
*
* DESCRIPTION:
*       Counts all entries in the Address Translation Unit.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       count - number of valid entries.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gfdbGetAtuAllCount
(
    IN  GT_QD_DEV 	*dev,
    OUT GT_U32 		*count
)
{
    GT_U32          dbNum, maxDbNum, numOfEntries;
    GT_STATUS       retVal;
    GT_ATU_ENTRY    entry;
	GT_ATU_STAT		atuStat;

    DBG_INFO(("gfdbGetAtuAllCount Called.\n"));

	if(IS_IN_DEV_GROUP(dev,DEV_ATU_STATS))
	{
		atuStat.op = GT_ATU_STATS_ALL;
		return atuGetStats(dev,&atuStat,count);
	}

    numOfEntries = 0;
	
	if (IS_IN_DEV_GROUP(dev,DEV_DBNUM_FULL))
		maxDbNum = 16;
	else if(IS_IN_DEV_GROUP(dev,DEV_DBNUM_64))
		maxDbNum = 64;
	else if(IS_IN_DEV_GROUP(dev,DEV_DBNUM_256))
		maxDbNum = 256;
	else if(IS_IN_DEV_GROUP(dev,DEV_DBNUM_4096))
		maxDbNum = 4096;
	else
		maxDbNum = 1;

	for(dbNum=0; dbNum<maxDbNum; dbNum++)
	{
		entry.DBNum = (GT_U16)dbNum;

		if(IS_IN_DEV_GROUP(dev,DEV_BROADCAST_INVALID))
		    gtMemSet(entry.macAddr.arEther,0,sizeof(GT_ETHERADDR));
		else
    		gtMemSet(entry.macAddr.arEther,0xFF,sizeof(GT_ETHERADDR));

	    while(1)
    	{
	        retVal = atuOperationPerform(dev,GET_NEXT_ENTRY,NULL,&entry);
        	if(retVal != GT_OK)
	        {
    	        DBG_INFO(("Failed.\n"));
        	    return retVal;
	        }

    	    if(IS_BROADCAST_MAC(entry.macAddr))
			{
				if(IS_IN_DEV_GROUP(dev,DEV_BROADCAST_INVALID))
					break;
				else if(entry.entryState.ucEntryState == 0)
					break;
	        	numOfEntries++;
				break;
			}

        	numOfEntries++;
	    }
	}

    *count = numOfEntries;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbGetAtuAllCountInDBNum
*
* DESCRIPTION:
*       Counts all entries in the defined FID (or DBNum).
*
* INPUTS:
*       dbNum - 
*
* OUTPUTS:
*       count - number of valid entries in FID (or DBNum).
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gfdbGetAtuAllCountInDBNum
(
    IN  GT_QD_DEV 	*dev,
    IN  GT_U32 		dbNum,
    OUT GT_U32 		*count
)
{
    GT_U32          numOfEntries;
    GT_STATUS       retVal;
    GT_ATU_ENTRY    entry;
	GT_ATU_STAT		atuStat;

    DBG_INFO(("gfdbGetAtuAllCountInDBNum Called.\n"));

	if(IS_IN_DEV_GROUP(dev,DEV_ATU_STATS))
	{
		atuStat.op = GT_ATU_STATS_ALL_FID;
		atuStat.DBNum = dbNum;
		return atuGetStats(dev,&atuStat,count);
	}

    numOfEntries = 0;
	
	entry.DBNum = (GT_U16)dbNum;

	if(IS_IN_DEV_GROUP(dev,DEV_BROADCAST_INVALID))
	    gtMemSet(entry.macAddr.arEther,0,sizeof(GT_ETHERADDR));
	else
    	gtMemSet(entry.macAddr.arEther,0xFF,sizeof(GT_ETHERADDR));

	while(1)
    {
	    retVal = atuOperationPerform(dev,GET_NEXT_ENTRY,NULL,&entry);
    	if(retVal != GT_OK)
	    {
            DBG_INFO(("Failed.\n"));
    	    return retVal;
	    }

        if(IS_BROADCAST_MAC(entry.macAddr))
		{
			if(IS_IN_DEV_GROUP(dev,DEV_BROADCAST_INVALID))
				break;
			else if(entry.entryState.ucEntryState == 0)
				break;
	    	numOfEntries++;
			break;
		}

    	numOfEntries++;
	}

    *count = numOfEntries;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbGetAtuDynamicCountInDBNum
*
* DESCRIPTION:
*       Counts all non-static entries in the defined FID (or DBNum).
*
* INPUTS:
*       dbNum - 
*
* OUTPUTS:
*       count - number of valid non-static entries in FID (or DBNum).
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gfdbGetAtuDynamicCountInDBNum
(
    IN  GT_QD_DEV 	*dev,
    IN  GT_U32 		dbNum,
    OUT GT_U32 		*count
)
{
    GT_U32          numOfEntries, tmpState;
    GT_STATUS       retVal;
    GT_ATU_ENTRY    entry;
	GT_ATU_UC_STATE	state;
	GT_ATU_STAT		atuStat;

    DBG_INFO(("gfdbGetAtuDynamicCountInDBNum Called.\n"));

	if(IS_IN_DEV_GROUP(dev,DEV_ATU_STATS))
	{
		atuStat.op = GT_ATU_STATS_NON_STATIC_FID;
		atuStat.DBNum = dbNum;
		return atuGetStats(dev,&atuStat,count);
	}

    numOfEntries = 0;
	
	entry.DBNum = (GT_U16)dbNum;

	if(IS_IN_DEV_GROUP(dev,DEV_BROADCAST_INVALID))
	    gtMemSet(entry.macAddr.arEther,0,sizeof(GT_ETHERADDR));
	else
    	gtMemSet(entry.macAddr.arEther,0xFF,sizeof(GT_ETHERADDR));

	while(1)
    {
	    retVal = atuOperationPerform(dev,GET_NEXT_ENTRY,NULL,&entry);
        if(retVal != GT_OK)
	    {
    	    DBG_INFO(("Failed.\n"));
            return retVal;
	    }

    	if(IS_BROADCAST_MAC(entry.macAddr))
            break;

	    if(IS_MULTICAST_MAC(entry.macAddr))
	    {
	        continue;
    	}

		atuStateDevToApp(dev,GT_TRUE,entry.entryState.ucEntryState,&tmpState);
		state = (GT_ATU_UC_STATE)tmpState;
		if (state == GT_UC_DYNAMIC)
		{
	    	numOfEntries++;
		}
	}
	
    *count = numOfEntries;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gfdbSetAtuSize
*
* DESCRIPTION:
*       Sets the Mac address table size.
*
* INPUTS:
*       size    - Mac address table size.
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
GT_STATUS gfdbSetAtuSize
(
    IN GT_QD_DEV    *dev,
    IN ATU_SIZE     size
)
{
    GT_U16          data;
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gfdbSetAtuSize Called.\n"));

	switch(size)
	{
		case ATU_SIZE_256:
			if (IS_IN_DEV_GROUP(dev,DEV_ATU_256_2048))
				data = 0;
			else
				return GT_NOT_SUPPORTED;
			break;
    	case ATU_SIZE_512:
    	case ATU_SIZE_1024:
    	case ATU_SIZE_2048:
			if (IS_IN_DEV_GROUP(dev,DEV_ATU_256_2048))
				data = (GT_U16)size;
			else
				data = (GT_U16)size - 1;
			break;

    	case ATU_SIZE_4096:
			if ((IS_IN_DEV_GROUP(dev,DEV_ATU_256_2048))||(IS_IN_DEV_GROUP(dev,DEV_ATU_562_2048)))
				return GT_NOT_SUPPORTED;
			else
				data = 3;
			break;
		default:
			return GT_NOT_SUPPORTED;
	}
	
	/* Check device if it has fixed ATU Size. */
	if (IS_IN_DEV_GROUP(dev,DEV_ATU_SIZE_FIXED))
    {
		return GT_NOT_SUPPORTED;
    }

    /* Set the Software reset bit.                  */
    retVal = hwSetGlobalRegField(dev,QD_REG_ATU_CONTROL,12,2,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    /* Make sure the reset operation is completed.  */
    data = 0;
    while(data == 0)
    {
        retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_STATUS,11,1,&data);
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
* gfdbGetAgingTimeRange
*
* DESCRIPTION:
*       Gets the maximal and minimum age times that the hardware can support.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       maxTimeout - max aging time in secounds.
*       minTimeout - min aging time in secounds.
*
* RETURNS:
*       GT_OK           - on success
*       GT_BAD_PARAM    - on bad parameter
*       GT_FAIL         - on error
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbGetAgingTimeRange
(
    IN GT_QD_DEV    *dev,
    OUT GT_U32 *maxTimeout,
    OUT GT_U32 *minTimeout
)
{
    DBG_INFO(("gfdbGetAgingTimeRange Called.\n"));
    if((maxTimeout == NULL) || (minTimeout == NULL))
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

	if (IS_IN_DEV_GROUP(dev,DEV_ATU_15SEC_AGING))
	{
		*minTimeout = 15;
		*maxTimeout = 3825;
	}
	else
	{
		*minTimeout = 16;
		*maxTimeout = 4080;
	}
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbGetAgingTimeout
*
* DESCRIPTION:
*       Gets the timeout period in seconds for aging out dynamically learned
*       forwarding information. The returned value may not be the same as the value
*		programmed with <gfdbSetAgingTimeout>. Please refer to the description of
*		<gfdbSetAgingTimeout>.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       timeout - aging time in seconds.
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
GT_STATUS gfdbGetAgingTimeout
(
    IN  GT_QD_DEV    *dev,
    OUT GT_U32       *timeout
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
	GT_U16			timeBase;

    DBG_INFO(("gfdbGetAgingTimeout Called.\n"));
 
	if (IS_IN_DEV_GROUP(dev,DEV_ATU_15SEC_AGING))
		timeBase = 15;
	else
		timeBase = 16;

    /* Get the Time Out value.              */
    retVal = hwGetGlobalRegField(dev,QD_REG_ATU_CONTROL,4,8,&data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

	*timeout = data*timeBase;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbSetAgingTimeout
*
* DESCRIPTION:
*       Sets the timeout period in seconds for aging out dynamically learned
*       forwarding information. The standard recommends 300 sec.
*		Supported aging timeout values are multiple of time-base, where time-base
*		is either 15 or 16 seconds, depending on the Switch device. For example,
*		88E6063 uses time-base 16, and so supported aging timeouts are 0,16,32,
*		48,..., and 4080. If unsupported timeout value (bigger than 16) is used, 
*		the value will be rounded to the nearest supported value smaller than the 
*		given timeout. If the given timeout is less than 16, minimum timeout value
*		16 will be used instead. E.g.) 35 becomes 32 and 5 becomes 16.
*		<gfdbGetAgingTimeRange> function can be used to find the time-base.
*
* INPUTS:
*       timeout - aging time in seconds.
*
* OUTPUTS:
*       None.
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
GT_STATUS gfdbSetAgingTimeout
(
    IN GT_QD_DEV    *dev,
    IN GT_U32 timeout
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */
	GT_U16			timeBase;

    DBG_INFO(("gfdbSetAgingTimeout Called.\n"));
 
	if (IS_IN_DEV_GROUP(dev,DEV_ATU_15SEC_AGING))
		timeBase = 15;
	else
		timeBase = 16;

	if((timeout < timeBase) && (timeout != 0))
	{	
 	   data = 1;
	}
	else
	{
 	   data = (GT_U16)(timeout/timeBase);
	   if (data & 0xFF00)
			data = 0xFF;
	}

    /* Set the Time Out value.              */
    retVal = hwSetGlobalRegField(dev,QD_REG_ATU_CONTROL,4,8,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbGetLearn2All
*
* DESCRIPTION:
*		When more than one Marvell device is used to form a single 'switch', it
*		may be desirable for all devices in the 'switch' to learn any address this 
*		device learns. When this bit is set to a one all other devices in the 
*		'switch' learn the same addresses this device learns. When this bit is 
*		cleared to a zero, only the devices that actually receive frames will learn
*		from those frames. This mode typically supports more active MAC addresses 
*		at one time as each device in the switch does not need to learn addresses 
*		it may nerver use.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		mode  - GT_TRUE if Learn2All is enabled, GT_FALSE otherwise
*
* RETURNS:
*		GT_OK           - on success
*		GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbGetLearn2All
(
	IN  GT_QD_DEV    *dev,
	OUT GT_BOOL 	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* to keep the read valve       */

    DBG_INFO(("gprtGetLearn2All Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get the Learn2All. */
    retVal = hwGetGlobalRegField(dev,QD_REG_ATU_CONTROL, 3, 1, &data);

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
* gfdbSetLearn2All
*
* DESCRIPTION:
*		Enable or disable Learn2All mode.
*
* INPUTS:
*		mode - GT_TRUE to set Learn2All, GT_FALSE otherwise
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
GT_STATUS gfdbSetLearn2All
(
	IN GT_QD_DEV	*dev,
	IN GT_BOOL		mode
)
{
    GT_U16          data;           /* Used to poll the SWReset bit */
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gprtSetLearn2All Called.\n"));

	/* check if the given Switch supports this feature. */
	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set Learn2All. */
    retVal = hwSetGlobalRegField(dev,QD_REG_ATU_CONTROL, 3, 1, data);

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
* gfdbGetAtuDynamicCount
*
* DESCRIPTION:
*       Gets the current number of dynamic unicast (non-static) entries in this
*       Filtering Database.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       numDynEntries - number of dynamic entries.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NO_SUCH - vlan does not exist.
*
* COMMENTS:
*       None
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbGetAtuDynamicCount
(
    IN  GT_QD_DEV 	*dev,
    OUT GT_U32 		*numDynEntries
)
{
    GT_U32          dbNum, maxDbNum, numOfEntries, tmpState;
    GT_STATUS       retVal;
    GT_ATU_ENTRY    entry;
	GT_ATU_UC_STATE	state;
	GT_ATU_STAT		atuStat;

    DBG_INFO(("gfdbGetAtuDynamicCount Called.\n"));

	if(IS_IN_DEV_GROUP(dev,DEV_ATU_STATS))
	{
		atuStat.op = GT_ATU_STATS_NON_STATIC;
		return atuGetStats(dev,&atuStat,numDynEntries);
	}

    numOfEntries = 0;
	
	if (IS_IN_DEV_GROUP(dev,DEV_DBNUM_FULL))
		maxDbNum = 16;
	else if(IS_IN_DEV_GROUP(dev,DEV_DBNUM_64))
		maxDbNum = 64;
	else if(IS_IN_DEV_GROUP(dev,DEV_DBNUM_256))
		maxDbNum = 256;
	else if(IS_IN_DEV_GROUP(dev,DEV_DBNUM_4096))
		maxDbNum = 4096;
	else
		maxDbNum = 1;

	for(dbNum=0; dbNum<maxDbNum; dbNum++)
	{
		entry.DBNum = (GT_U16)dbNum;

		if(IS_IN_DEV_GROUP(dev,DEV_BROADCAST_INVALID))
		    gtMemSet(entry.macAddr.arEther,0,sizeof(GT_ETHERADDR));
		else
    		gtMemSet(entry.macAddr.arEther,0xFF,sizeof(GT_ETHERADDR));

	    while(1)
    	{
	        retVal = atuOperationPerform(dev,GET_NEXT_ENTRY,NULL,&entry);
        	if(retVal != GT_OK)
	        {
    	        DBG_INFO(("Failed.\n"));
        	    return retVal;
	        }

    	    if(IS_BROADCAST_MAC(entry.macAddr))
        	    break;

	        if(IS_MULTICAST_MAC(entry.macAddr))
	        {
	            continue;
    	    }

			atuStateDevToApp(dev,GT_TRUE,(GT_U32)entry.entryState.ucEntryState,&tmpState);
			state = (GT_ATU_UC_STATE)tmpState;
			if (state == GT_UC_DYNAMIC)
			{
	        	numOfEntries++;
			}
	    }
	}

    *numDynEntries = numOfEntries;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbGetAtuEntryFirst
*
* DESCRIPTION:
*       Gets first lexicographic MAC address entry from the ATU.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       atuEntry - match Address translate unit entry.
*
* RETURNS:
*       GT_OK      - on success
*       GT_FAIL    - on error
*       GT_NO_SUCH - table is empty.
*
* COMMENTS:
*       Search starts from Mac[00:00:00:00:00:00]
*
*		DBNum in atuEntry - 
*			ATU MAC Address Database number. If multiple address 
*			databases are not being used, DBNum should be zero.
*			If multiple address databases are being used, this value
*			should be set to the desired address database number.
*
*******************************************************************************/
GT_STATUS gfdbGetAtuEntryFirst
(
    IN GT_QD_DEV    *dev,
    OUT GT_ATU_ENTRY    *atuEntry
)
{
    GT_STATUS       retVal;
    GT_ATU_ENTRY    entry;

    DBG_INFO(("gfdbGetAtuEntryFirst Called.\n"));

	if(IS_IN_DEV_GROUP(dev,DEV_BROADCAST_INVALID))
	    gtMemSet(entry.macAddr.arEther,0,sizeof(GT_ETHERADDR));
	else
    	gtMemSet(entry.macAddr.arEther,0xFF,sizeof(GT_ETHERADDR));

	entry.DBNum = atuEntry->DBNum;

    DBG_INFO(("DBNum : %i\n",entry.DBNum));

    retVal = atuOperationPerform(dev,GET_NEXT_ENTRY,NULL,&entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (atuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    if(IS_BROADCAST_MAC(entry.macAddr))
    {
		if(IS_IN_DEV_GROUP(dev,DEV_BROADCAST_INVALID))
		{
	        DBG_INFO(("Failed (Invalid Mac).\n"));
    	    return GT_NO_SUCH;
		}
		else if(entry.entryState.ucEntryState == 0)
		{
	        DBG_INFO(("Failed (Invalid Mac).\n"));
    	    return GT_NO_SUCH;
		}
    }

    gtMemCpy(atuEntry->macAddr.arEther,entry.macAddr.arEther,6);
    atuEntry->portVec   = GT_PORTVEC_2_LPORTVEC(entry.portVec);
    atuEntry->prio      = entry.prio;
    atuEntry->trunkMember = entry.trunkMember;
	atuEntry->exPrio.useMacFPri = entry.exPrio.useMacFPri;
	atuEntry->exPrio.macFPri = entry.exPrio.macFPri;
	atuEntry->exPrio.macQPri = entry.exPrio.macQPri;

    if(IS_MULTICAST_MAC(entry.macAddr))
    {
        if(dev->deviceId == GT_88E6051)
        {
            DBG_INFO(("Failed.\n"));
            return GT_FAIL;
        }

		atuStateDevToApp(dev,GT_FALSE,(GT_U32)entry.entryState.ucEntryState,
						(GT_U32*)&atuEntry->entryState.mcEntryState);
    }
    else
    {
		atuStateDevToApp(dev,GT_TRUE,(GT_U32)entry.entryState.ucEntryState,
						(GT_U32*)&atuEntry->entryState.ucEntryState);
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gfdbGetAtuEntryNext
*
* DESCRIPTION:
*       Gets next lexicographic MAC address from the specified Mac Addr.
*
* INPUTS:
*       atuEntry - the Mac Address to start the search.
*
* OUTPUTS:
*       atuEntry - match Address translate unit entry.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - on error or entry does not exist.
*       GT_NO_SUCH - no more entries.
*
* COMMENTS:
*       Search starts from atu.macAddr[xx:xx:xx:xx:xx:xx] specified by the
*       user.
*
*		DBNum in atuEntry - 
*			ATU MAC Address Database number. If multiple address 
*			databases are not being used, DBNum should be zero.
*			If multiple address databases are being used, this value
*			should be set to the desired address database number.
*
*******************************************************************************/
GT_STATUS gfdbGetAtuEntryNext
(
    IN GT_QD_DEV    *dev,
    INOUT GT_ATU_ENTRY  *atuEntry
)
{
    GT_STATUS       retVal;
    GT_ATU_ENTRY    entry;

    DBG_INFO(("gfdbGetAtuEntryNext Called.\n"));

    if(IS_BROADCAST_MAC(atuEntry->macAddr))
    {
   	    return GT_NO_SUCH;
    }

    gtMemCpy(entry.macAddr.arEther,atuEntry->macAddr.arEther,6);

	entry.DBNum = atuEntry->DBNum;
    DBG_INFO(("DBNum : %i\n",entry.DBNum));

    retVal = atuOperationPerform(dev,GET_NEXT_ENTRY,NULL,&entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (atuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    if(IS_BROADCAST_MAC(entry.macAddr))
    {
		if(IS_IN_DEV_GROUP(dev,DEV_BROADCAST_INVALID))
		{
	        DBG_INFO(("Failed (Invalid Mac).\n"));
    	    return GT_NO_SUCH;
		}
		else if(entry.entryState.ucEntryState == 0)
		{
	        DBG_INFO(("Failed (Invalid Mac).\n"));
    	    return GT_NO_SUCH;
		}
    }

    gtMemCpy(atuEntry->macAddr.arEther,entry.macAddr.arEther,6);
    atuEntry->portVec   = GT_PORTVEC_2_LPORTVEC(entry.portVec);
    atuEntry->prio      = entry.prio;
    atuEntry->trunkMember = entry.trunkMember;
	atuEntry->exPrio.useMacFPri = entry.exPrio.useMacFPri;
	atuEntry->exPrio.macFPri = entry.exPrio.macFPri;
	atuEntry->exPrio.macQPri = entry.exPrio.macQPri;

    if(IS_MULTICAST_MAC(entry.macAddr))
    {
        if(dev->deviceId == GT_88E6051)
        {
            DBG_INFO(("Failed.\n"));
            return GT_FAIL;
        }

		atuStateDevToApp(dev,GT_FALSE,(GT_U32)entry.entryState.ucEntryState,
						(GT_U32*)&atuEntry->entryState.mcEntryState);
    }
    else
    {
		atuStateDevToApp(dev,GT_TRUE,(GT_U32)entry.entryState.ucEntryState,
						(GT_U32*)&atuEntry->entryState.ucEntryState);
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gfdbFindAtuMacEntry
*
* DESCRIPTION:
*       Find FDB entry for specific MAC address from the ATU.
*
* INPUTS:
*       atuEntry - the Mac address to search.
*
* OUTPUTS:
*       found    - GT_TRUE, if the appropriate entry exists.
*       atuEntry - the entry parameters.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - on error or entry does not exist.
*       GT_NO_SUCH - no more entries.
*       GT_BAD_PARAM    - on bad parameter
*
* COMMENTS:
*		DBNum in atuEntry - 
*			ATU MAC Address Database number. If multiple address 
*			databases are not being used, DBNum should be zero.
*			If multiple address databases are being used, this value
*			should be set to the desired address database number.
*
*******************************************************************************/
GT_STATUS gfdbFindAtuMacEntry
(
    IN GT_QD_DEV    *dev,
    INOUT GT_ATU_ENTRY  *atuEntry,
    OUT GT_BOOL         *found
)
{
    GT_STATUS       retVal;
    GT_ATU_ENTRY    entry;
    int           i;

    DBG_INFO(("gfdbFindAtuMacEntry Called.\n"));
    *found = GT_FALSE;
    gtMemCpy(entry.macAddr.arEther,atuEntry->macAddr.arEther,6);
	entry.DBNum = atuEntry->DBNum;

    /* Decrement 1 from mac address.    */
    for(i=5; i >= 0; i--)
    {
        if(entry.macAddr.arEther[i] != 0)
        {
            entry.macAddr.arEther[i] -= 1;
            break;
        }
		else
            entry.macAddr.arEther[i] = 0xFF;
    }

    /* Check if the given mac equals zero   */
    if((i == -1) && IS_IN_DEV_GROUP(dev,DEV_BROADCAST_INVALID))
    {
        DBG_INFO(("Address should not be all zeros.\n"));
        return GT_BAD_PARAM;
    }

    retVal = atuOperationPerform(dev,GET_NEXT_ENTRY,NULL,&entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    if(IS_BROADCAST_MAC(entry.macAddr))
    {
		if(IS_IN_DEV_GROUP(dev,DEV_BROADCAST_INVALID))
		{
	        DBG_INFO(("Failed (Broadcast addr is not valid).\n"));
    	    return GT_NO_SUCH;
		}
		else if(entry.entryState.ucEntryState == 0)
		{
	        DBG_INFO(("Failed (Invalid Mac).\n"));
    	    return GT_NO_SUCH;
		}
    }

	if(gtMemCmp(atuEntry->macAddr.arEther,entry.macAddr.arEther,ETHERNET_HEADER_SIZE))
	{
        DBG_INFO(("Failed.\n"));
        return GT_NO_SUCH;
	}

    atuEntry->portVec   = GT_PORTVEC_2_LPORTVEC(entry.portVec);
    atuEntry->prio      = entry.prio;
    atuEntry->trunkMember = entry.trunkMember;
	atuEntry->exPrio.useMacFPri = entry.exPrio.useMacFPri;
	atuEntry->exPrio.macFPri = entry.exPrio.macFPri;
	atuEntry->exPrio.macQPri = entry.exPrio.macQPri;

    if(IS_MULTICAST_MAC(entry.macAddr))
    {
        if(dev->deviceId == GT_88E6051)
        {
            DBG_INFO(("Failed.\n"));
            return GT_FAIL;
        }

		atuStateDevToApp(dev,GT_FALSE,(GT_U32)entry.entryState.ucEntryState,
						(GT_U32*)&atuEntry->entryState.mcEntryState);
    }
    else
    {
		atuStateDevToApp(dev,GT_TRUE,(GT_U32)entry.entryState.ucEntryState,
						(GT_U32*)&atuEntry->entryState.ucEntryState);
    }

    *found = GT_TRUE;
    DBG_INFO(("OK.\n"));
    return GT_OK;
}



/*******************************************************************************
* gfdbFlush
*
* DESCRIPTION:
*       This routine flush all or unblocked addresses from the MAC Address
*       Table.
*
* INPUTS:
*       flushCmd - the flush operation type.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NO_RESOURCE  - failed to allocate a t2c struct
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbFlush
(
    IN GT_QD_DEV    *dev,
    IN GT_FLUSH_CMD flushCmd
)
{
    GT_STATUS       retVal;
    GT_ATU_ENTRY    entry;

    DBG_INFO(("gfdbFlush Called.\n"));
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_STATIC_ADDR))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	entry.DBNum = 0;
	entry.entryState.ucEntryState = 0;

    if(flushCmd == GT_FLUSH_ALL)
        retVal = atuOperationPerform(dev,FLUSH_ALL,NULL,&entry);
    else
        retVal = atuOperationPerform(dev,FLUSH_UNLOCKED,NULL,&entry);

    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbFlushInDB
*
* DESCRIPTION:
*       This routine flush all or unblocked addresses from the particular
*       ATU Database (DBNum). If multiple address databases are being used, this
*		API can be used to flush entries in a particular DBNum database.
*
* INPUTS:
*       flushCmd - the flush operation type.
*		DBNum	 - ATU MAC Address Database Number. 
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORTED- if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbFlushInDB
(
    IN GT_QD_DEV    *dev,
    IN GT_FLUSH_CMD flushCmd,
	IN GT_U32 DBNum
)
{
    GT_STATUS       retVal;
    GT_ATU_ENTRY    entry;

    DBG_INFO(("gfdbFlush Called.\n"));
    DBG_INFO(("gfdbFush: dev=%x, dev->atuRegsSem=%d \n",dev, dev->atuRegsSem));

    /* check if device supports this feature */
	if ((!IS_IN_DEV_GROUP(dev,DEV_DBNUM_FULL)) && 
		(!IS_IN_DEV_GROUP(dev,DEV_DBNUM_64)) && 
		(!IS_IN_DEV_GROUP(dev,DEV_DBNUM_4096)) && 
		(!IS_IN_DEV_GROUP(dev,DEV_DBNUM_256)))
	{
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

	entry.DBNum = (GT_U16)DBNum;
	entry.entryState.ucEntryState = 0;

    if(flushCmd == GT_FLUSH_ALL)
        retVal = atuOperationPerform(dev,FLUSH_ALL_IN_DB,NULL,&entry);
    else
        retVal = atuOperationPerform(dev,FLUSH_UNLOCKED_IN_DB,NULL,&entry);

    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbMove
*
* DESCRIPTION:
*       This routine moves all or unblocked addresses from a port to another.
*
* INPUTS:
* 		moveCmd  - the move operation type.
*		moveFrom - port where moving from
*		moveTo   - port where moving to
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbMove
(
	IN GT_QD_DEV    *dev,
	IN GT_MOVE_CMD  moveCmd,
	IN GT_LPORT		moveFrom,
	IN GT_LPORT		moveTo
)
{
    GT_STATUS       retVal;
    GT_ATU_ENTRY    entry;
	GT_EXTRA_OP_DATA	opData;

    DBG_INFO(("gfdbMove Called.\n"));

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_802_1W))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	entry.DBNum = 0;
	entry.entryState.ucEntryState = 0xF;
	if (moveTo == 0xF)
		opData.moveTo = moveTo;
	else
		opData.moveTo = (GT_U32)GT_LPORT_2_PORT(moveTo);
	opData.moveFrom = (GT_U32)GT_LPORT_2_PORT(moveFrom);

	if((opData.moveTo == 0xFF) || (opData.moveFrom == 0xFF))
		return GT_BAD_PARAM;

    if(moveCmd == GT_MOVE_ALL)
        retVal = atuOperationPerform(dev,FLUSH_ALL,&opData,&entry);
    else
        retVal = atuOperationPerform(dev,FLUSH_UNLOCKED,&opData,&entry);

    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbMoveInDB
*
* DESCRIPTION:
*       This routine move all or unblocked addresses which are in the particular
*       ATU Database (DBNum) from a port to another.
*
* INPUTS:
*       moveCmd  - the move operation type.
*		DBNum	 - ATU MAC Address Database Number.
*		moveFrom - port where moving from
*		moveTo   - port where moving to
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORTED- if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbMoveInDB
(
    IN GT_QD_DEV    *dev,
    IN GT_MOVE_CMD 	moveCmd,
	IN GT_U32 		DBNum,
	IN GT_LPORT		moveFrom,
	IN GT_LPORT		moveTo
)
{
    GT_STATUS       retVal;
    GT_ATU_ENTRY    entry;
	GT_EXTRA_OP_DATA	opData;

    DBG_INFO(("gfdbMoveInDB Called.\n"));

	/* Only Gigabit Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_802_1W))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	entry.DBNum = (GT_U16)DBNum;
	entry.entryState.ucEntryState = 0xF;

	if (moveTo == 0xF)
		opData.moveTo = moveTo;
	else
		opData.moveTo = (GT_U32)GT_LPORT_2_PORT(moveTo);
	opData.moveFrom = (GT_U32)GT_LPORT_2_PORT(moveFrom);

	if((opData.moveTo == 0xFF) || (opData.moveFrom == 0xFF))
		return GT_BAD_PARAM;

    if(moveCmd == GT_MOVE_ALL)
        retVal = atuOperationPerform(dev,FLUSH_ALL_IN_DB,&opData,&entry);
    else
        retVal = atuOperationPerform(dev,FLUSH_UNLOCKED_IN_DB,&opData,&entry);

    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbRemovePort
*
* DESCRIPTION:
*       This routine deassociages all or unblocked addresses from a port.
*
* INPUTS:
*       moveCmd - the move operation type.
*       port - the logical port number.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbRemovePort
(
	IN GT_QD_DEV    *dev,
    IN GT_MOVE_CMD 	moveCmd,
    IN GT_LPORT		port
)
{
    DBG_INFO(("gfdbRemovePort Called.\n"));

	/* Only 88E6093 Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ATU_RM_PORTS))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

	return gfdbMove(dev,moveCmd,port,(GT_LPORT)0xF);
}


/*******************************************************************************
* gfdbRemovePortInDB
*
* DESCRIPTION:
*       This routine deassociages all or unblocked addresses from a port in the
*       particular ATU Database (DBNum).
*
* INPUTS:
*       moveCmd  - the move operation type.
*       port - the logical port number.
*		DBNum	 - ATU MAC Address Database Number.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORTED- if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbRemovePortInDB
(
	IN GT_QD_DEV    *dev,
    IN GT_MOVE_CMD 	moveCmd,
    IN GT_LPORT		port,
	IN GT_U32 		DBNum
)
{
    DBG_INFO(("gfdbRemovePortInDB Called.\n"));

	/* Only 88E6093 Switch supports this status. */
	if (!IS_IN_DEV_GROUP(dev,DEV_ATU_RM_PORTS))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }
	return gfdbMoveInDB(dev,moveCmd,DBNum,port,(GT_LPORT)0xF);
}


/*******************************************************************************
* gfdbAddMacEntry
*
* DESCRIPTION:
*       Creates the new entry in MAC address table.
*
* INPUTS:
*       macEntry    - mac address entry to insert to the ATU.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK          - on success
*       GT_FAIL        - on error
*       GT_BAD_PARAM   - on invalid port vector
*
* COMMENTS:
*		DBNum in atuEntry - 
*			ATU MAC Address Database number. If multiple address 
*			databases are not being used, DBNum should be zero.
*			If multiple address databases are being used, this value
*			should be set to the desired address database number.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbAddMacEntry
(
    IN GT_QD_DEV    *dev,
    IN GT_ATU_ENTRY *macEntry
)
{
    GT_STATUS       retVal;
    GT_ATU_ENTRY    entry;

    DBG_INFO(("gfdbAddMacEntry Called.\n"));
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_STATIC_ADDR))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    gtMemCpy(entry.macAddr.arEther,macEntry->macAddr.arEther,6);
	entry.DBNum		= macEntry->DBNum;
    entry.portVec     = GT_LPORTVEC_2_PORTVEC(macEntry->portVec);
	if(entry.portVec == GT_INVALID_PORT_VEC)
	{
		return GT_BAD_PARAM;
	}

	if(IS_IN_DEV_GROUP(dev,DEV_ATU_EXT_PRI))
	{
		if(IS_IN_DEV_GROUP(dev,DEV_FQPRI_IN_TABLE))
		{
			entry.exPrio.useMacFPri = macEntry->exPrio.useMacFPri;
			entry.exPrio.macFPri = macEntry->exPrio.macFPri;
			entry.exPrio.macQPri = macEntry->exPrio.macQPri;
		}
		else
		{
			entry.exPrio.useMacFPri = 0;
			entry.exPrio.macFPri = 0;
			entry.exPrio.macQPri = macEntry->exPrio.macQPri;
		}
    	entry.prio	    = 0;
	}
	else
	{
		entry.exPrio.useMacFPri = 0;
		entry.exPrio.macFPri = 0;
		entry.exPrio.macQPri = 0;
    	entry.prio	    = macEntry->prio;
	}

	if (IS_IN_DEV_GROUP(dev,DEV_TRUNK))
	{
	    entry.trunkMember = macEntry->trunkMember;
	}
	else
	{
	    entry.trunkMember = GT_FALSE;
	}

    if(IS_MULTICAST_MAC(entry.macAddr))
    {
		atuStateAppToDev(dev,GT_FALSE,(GT_U32)macEntry->entryState.mcEntryState,
							(GT_U32*)&entry.entryState.ucEntryState);
    }
    else
	{
		atuStateAppToDev(dev,GT_TRUE,(GT_U32)macEntry->entryState.ucEntryState,
							(GT_U32*)&entry.entryState.ucEntryState);
	}

	if (entry.entryState.ucEntryState == 0)
	{
        DBG_INFO(("Entry State should not be ZERO.\n"));
		return GT_BAD_PARAM;
	}

    retVal = atuOperationPerform(dev,LOAD_PURGE_ENTRY,NULL,&entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gfdbDelMacEntry
*
* DESCRIPTION:
*       Deletes MAC address entry. If DBNum or FID is used, gfdbDelAtuEntry API
*		would be the better choice to delete an entry in ATU.
*
* INPUTS:
*       macAddress - mac address.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NO_RESOURCE  - failed to allocate a t2c struct
*       GT_NO_SUCH      - if specified address entry does not exist
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS gfdbDelMacEntry
(
    IN GT_QD_DEV    *dev,
    IN GT_ETHERADDR  *macAddress
)
{
    GT_STATUS retVal;
    GT_ATU_ENTRY    entry;

    DBG_INFO(("gfdbDelMacEntry Called.\n"));
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_STATIC_ADDR))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    gtMemCpy(entry.macAddr.arEther,macAddress->arEther,6);
	entry.DBNum = 0;
	entry.prio = 0;
	entry.portVec = 0;
	entry.entryState.ucEntryState = 0;
	entry.trunkMember = GT_FALSE;
	entry.exPrio.useMacFPri = GT_FALSE;
	entry.exPrio.macFPri = 0;
	entry.exPrio.macQPri = 0;

    retVal = atuOperationPerform(dev,LOAD_PURGE_ENTRY,NULL,&entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gfdbDelAtuEntry
*
* DESCRIPTION:
*       Deletes ATU entry.
*
* INPUTS:
*       atuEntry - the ATU entry to be deleted.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NO_RESOURCE  - failed to allocate a t2c struct
*       GT_NO_SUCH      - if specified address entry does not exist
*
* COMMENTS:
*		DBNum in atuEntry - 
*			ATU MAC Address Database number. If multiple address 
*			databases are not being used, DBNum should be zero.
*			If multiple address databases are being used, this value
*			should be set to the desired address database number.
*
*******************************************************************************/
GT_STATUS gfdbDelAtuEntry
(
    IN GT_QD_DEV    *dev,
    IN GT_ATU_ENTRY  *atuEntry
)
{
    GT_ATU_ENTRY    entry;
    GT_STATUS retVal;

    DBG_INFO(("gfdbDelMacEntry Called.\n"));
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_STATIC_ADDR))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    gtMemCpy(entry.macAddr.arEther,atuEntry->macAddr.arEther,6);
	entry.DBNum = atuEntry->DBNum;
	entry.prio = 0;
	entry.portVec = 0;
    entry.entryState.ucEntryState = 0;
    entry.trunkMember = GT_FALSE;
	entry.exPrio.useMacFPri = GT_FALSE;
	entry.exPrio.macFPri = 0;
	entry.exPrio.macQPri = 0;

    retVal = atuOperationPerform(dev,LOAD_PURGE_ENTRY,NULL,&entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/*******************************************************************************
* gfdbLearnEnable
*
* DESCRIPTION:
*       Enable/disable automatic learning of new source MAC addresses on port
*       ingress.
*
* INPUTS:
*       en - GT_TRUE for enable  or GT_FALSE otherwise
*
* OUTPUTS:
*       None
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
GT_STATUS gfdbLearnEnable
(
    IN GT_QD_DEV    *dev,
    IN GT_BOOL  en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
	GT_LPORT	port;
	GT_BOOL		mode;

    DBG_INFO(("gfdbLearnEnable Called.\n"));
    BOOL_2_BIT(en,data);
    data = 1 - data;

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
		mode = (en)?GT_FALSE:GT_TRUE;

		for (port=0; port<dev->numOfPorts; port++)
		{
			retVal = gprtSetLearnDisable(dev,port,mode);
		    if(retVal != GT_OK)
    		{
	    	    DBG_INFO(("Failed.\n"));
    	    	return retVal;
		    }
		}
	}
	else
	{
	    /* Set the Learn Enable bit.            */
    	retVal = hwSetGlobalRegField(dev,QD_REG_ATU_CONTROL,14,1,data);
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
* gfdbGetLearnEnable
*
* DESCRIPTION:
*       Get automatic learning status of new source MAC addresses on port ingress.
*
* INPUTS:
*       None
*
* OUTPUTS:
*       en - GT_TRUE if enabled  or GT_FALSE otherwise
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gfdbGetLearnEnable
(
    IN GT_QD_DEV    *dev,
    OUT GT_BOOL  *en
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    DBG_INFO(("gfdbGetLearnEnable Called.\n"));

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
		return GT_NOT_SUPPORTED;
	}
	else
	{
	    /* Get the Learn Enable bit.            */
    	retVal = hwGetGlobalRegField(dev,QD_REG_ATU_CONTROL,14,1,&data);
	    if(retVal != GT_OK)
    	{
	        DBG_INFO(("Failed.\n"));
    	    return retVal;
	    }
	}	

    data = 1 - data;
    BOOL_2_BIT(data, *en);

    DBG_INFO(("OK.\n"));
    return GT_OK;
}

/****************************************************************************/
/* Internal use functions.                                                  */
/****************************************************************************/

/*******************************************************************************
* gatuGetViolation
*
* DESCRIPTION:
*       Get ATU Violation data
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       atuIntStatus - interrupt cause, source portID, and vid.
*
* RETURNS:
*       GT_OK           - on success
*       GT_FAIL         - on error
*       GT_NOT_SUPPORT  - if current device does not support this feature.
*
* COMMENTS:
*		This is an internal function. No user should call this function.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gatuGetViolation
(
    IN  GT_QD_DEV         *dev,
    OUT GT_ATU_INT_STATUS *atuIntStatus
)
{
    GT_U16              intCause;
    GT_STATUS       	retVal;
    GT_ATU_ENTRY    	entry;
	GT_EXTRA_OP_DATA	opData;
	GT_BOOL				found, ageInt;

    DBG_INFO(("gatuGetViolation Called.\n"));

	/* check which Violation occurred */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_STATUS,3,1,&intCause);
    if(retVal != GT_OK)
    {
	    DBG_INFO(("ERROR to read ATU OPERATION Register.\n"));
        return retVal;
    }

	if (!intCause)
	{
		/* No Violation occurred. */
		atuIntStatus->atuIntCause = 0;
		return GT_OK;
	}

    retVal = atuOperationPerform(dev,SERVICE_VIOLATIONS,&opData,&entry);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed (atuOperationPerform returned GT_FAIL).\n"));
        return retVal;
    }

    gtMemCpy(atuIntStatus->macAddr.arEther,entry.macAddr.arEther,6);

	atuIntStatus->atuIntCause = opData.intCause;
	atuIntStatus->spid = entry.entryState.ucEntryState;

	if(atuIntStatus->spid != 0xF)
		atuIntStatus->spid = (GT_U8)GT_PORT_2_LPORT(atuIntStatus->spid);
			
	if (IS_IN_DEV_GROUP(dev,DEV_AGE_OUT_INT))
	{
		if (opData.intCause == GT_AGE_VIOLATION)
		{
			atuIntStatus->atuIntCause = GT_AGE_OUT_VIOLATION;
		}
		else if (opData.intCause == GT_MISS_VIOLATION)
		{
			/* check if it's AGE Violation */
			if((retVal = gsysGetAgeInt(dev, &ageInt)) != GT_OK)
				return retVal;

			if(ageInt)
			{
				gfdbFindAtuMacEntry(dev, &entry, &found);
				if ((found) && (entry.entryState.ucEntryState <= 4))
					atuIntStatus->atuIntCause = GT_AGE_VIOLATION;
			}
			
		}
	}

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* atuOperationPerform
*
* DESCRIPTION:
*       This function is used by all ATU control functions, and is responsible
*       to write the required operation into the ATU registers.
*
* INPUTS:
*       atuOp       - The ATU operation bits to be written into the ATU
*                     operation register.
*       DBNum       - ATU Database Number for CPU accesses
*       entryPri    - The EntryPri field in the ATU Data register.
*       portVec     - The portVec field in the ATU Data register.
*       entryState  - The EntryState field in the ATU Data register.
*       atuMac      - The Mac address to be written to the ATU Mac registers.
*
* OUTPUTS:
*       entryPri    - The EntryPri field in case the atuOp is GetNext.
*       portVec     - The portVec field in case the atuOp is GetNext.
*       entryState  - The EntryState field in case the atuOp is GetNext.
*       atuMac      - The returned Mac address in case the atuOp is GetNext.
*
* RETURNS:
*       GT_OK on success,
*       GT_FAIL otherwise.
*
* COMMENTS:
*       1.  if atuMac == NULL, nothing needs to be written to ATU Mac registers.
*
*******************************************************************************/
static GT_STATUS atuOperationPerform
(
    IN      GT_QD_DEV           *dev,
    IN      GT_ATU_OPERATION    atuOp,
	INOUT	GT_EXTRA_OP_DATA	*opData,
    INOUT 	GT_ATU_ENTRY    	*entry
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* Data to be set into the      */
                                    /* register.                    */
    GT_U16          opcodeData;           /* Data to be set into the      */
                                    /* register.                    */
    GT_U8           i;
    GT_U16			portMask;

    gtSemTake(dev,dev->atuRegsSem,OS_WAIT_FOREVER);

	portMask = (1 << dev->maxPorts) - 1;

    /* Wait until the ATU in ready. */
    data = 1;
    while(data == 1)
    {
        retVal = hwGetGlobalRegField(dev,QD_REG_ATU_OPERATION,15,1,&data);
        if(retVal != GT_OK)
        {
            gtSemGive(dev,dev->atuRegsSem);
            return retVal;
        }
    }

	opcodeData = 0;

	switch (atuOp)
	{
		case LOAD_PURGE_ENTRY:
				if (IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY) || 
					IS_IN_DEV_GROUP(dev,DEV_88E6097_FAMILY))
				{
					if (IS_IN_DEV_GROUP(dev,DEV_TRUNK) && entry->trunkMember)
					{
						/* portVec represents trunk ID */
				        data = ( 0x8000 | (((entry->portVec) & 0xF) << 4) |
        			         (((entry->entryState.ucEntryState) & 0xF)) );
					}
					else
					{
				        data = ( (((entry->portVec) & portMask) << 4) |
        			         (((entry->entryState.ucEntryState) & 0xF)) );
					}
					opcodeData |= (entry->prio & 0x7) << 8;
				}
				else if(IS_IN_DEV_GROUP(dev,DEV_ATU_EXT_PRI))
				{
			        data = ( (((entry->portVec) & portMask) << 4) |
        			         (((entry->entryState.ucEntryState) & 0xF)) |
        			         (((entry->exPrio.macQPri) & 0x3) << 14) );
					if(entry->exPrio.useMacFPri == GT_TRUE)
						data |= ((1 << 13) | ((entry->exPrio.macFPri & 0x7) << 10));
				}
				else
				{
			        data = ( (((entry->prio) & 0x3) << 14) | 
			        		(((entry->portVec) & portMask) << 4) |
        					(((entry->entryState.ucEntryState) & 0xF)) );
				}
		        retVal = hwWriteGlobalReg(dev,QD_REG_ATU_DATA_REG,data);
        		if(retVal != GT_OK)
		        {
        		    gtSemGive(dev,dev->atuRegsSem);
		            return retVal;
        		}
				/* pass thru */

		case GET_NEXT_ENTRY:
		        for(i = 0; i < 3; i++)
        		{
		            data=(entry->macAddr.arEther[2*i] << 8)|(entry->macAddr.arEther[1 + 2*i]);
        		    retVal = hwWriteGlobalReg(dev,(GT_U8)(QD_REG_ATU_MAC_BASE+i),data);
		            if(retVal != GT_OK)
        		    {
		                gtSemGive(dev,dev->atuRegsSem);
        		        return retVal;
		            }
        		}
				break;

		case FLUSH_ALL:
		case FLUSH_UNLOCKED:
		case FLUSH_ALL_IN_DB:
		case FLUSH_UNLOCKED_IN_DB:
				if (entry->entryState.ucEntryState == 0xF)
				{
			        data = 0xF | ((opData->moveFrom & 0xF) << 4) | ((opData->moveTo & 0xF) << 8);
				}
				else
				{
			        data = 0;
				}
		        retVal = hwWriteGlobalReg(dev,QD_REG_ATU_DATA_REG,data);
       			if(retVal != GT_OK)
	        	{
       		    	gtSemGive(dev,dev->atuRegsSem);
		            return retVal;
   	    		}
				break;

		case SERVICE_VIOLATIONS:

				break;

		default :
				return GT_FAIL;
	}

    /* Set DBNum */
	if(IS_IN_DEV_GROUP(dev,DEV_FID_REG))
	{
	    retVal = hwSetGlobalRegField(dev,QD_REG_ATU_FID_REG,0,12,entry->DBNum & 0xFFF);
    	if(retVal != GT_OK)
	    {
    	    gtSemGive(dev,dev->atuRegsSem);
        	return retVal;
	    }
	}
	else if (IS_IN_DEV_GROUP(dev,DEV_DBNUM_256))
	{
	    retVal = hwSetGlobalRegField(dev,QD_REG_ATU_CONTROL,12,4,(entry->DBNum & 0xF0) >> 4);
    	if(retVal != GT_OK)
	    {
    	    gtSemGive(dev,dev->atuRegsSem);
        	return retVal;
	    }
	}
	else if (IS_IN_DEV_GROUP(dev,DEV_DBNUM_64))
	{
	    opcodeData |= ((entry->DBNum & 0x30) << 4);	/* Op Reg bit 9:8 */
	}

    /* Set the ATU Operation register in addtion to DBNum setup  */

	if(IS_IN_DEV_GROUP(dev,DEV_FID_REG))
	    opcodeData |= ((1 << 15) | (atuOp << 12));
	else
	    opcodeData |= ((1 << 15) | (atuOp << 12) | (entry->DBNum & 0xF));

    retVal = hwWriteGlobalReg(dev,QD_REG_ATU_OPERATION,opcodeData);
    if(retVal != GT_OK)
    {
        gtSemGive(dev,dev->atuRegsSem);
        return retVal;
    }

	/* If the operation is to service violation operation wait for the response   */
	if(atuOp == SERVICE_VIOLATIONS)
	{
		/* Wait until the VTU in ready. */
		data = 1;
		while(data == 1)
		{
			retVal = hwGetGlobalRegField(dev,QD_REG_ATU_OPERATION,15,1,&data);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->atuRegsSem);
				return retVal;
			}
		}

		/* get the Interrupt Cause */
		retVal = hwGetGlobalRegField(dev,QD_REG_ATU_OPERATION,4,3,&data);
		if(retVal != GT_OK)
		{
			gtSemGive(dev,dev->atuRegsSem);
			return retVal;
		}
	
		switch (data)
		{
			case 8:	/* Age Interrupt */
				opData->intCause = GT_AGE_VIOLATION;
				break;
			case 4:	/* Member Violation */
				opData->intCause = GT_MEMBER_VIOLATION;
				break;
			case 2:	/* Miss Violation */
				opData->intCause = GT_MISS_VIOLATION;
				break;
			case 1:	/* Full Violation */
				opData->intCause = GT_FULL_VIOLATION;
				break;
			default:
				opData->intCause = 0;
				gtSemGive(dev,dev->atuRegsSem);
				return GT_OK;
		}

		/* get the DBNum that was involved in the violation */

		entry->DBNum = 0;

		if(IS_IN_DEV_GROUP(dev,DEV_FID_REG))
		{
		    retVal = hwGetGlobalRegField(dev,QD_REG_ATU_FID_REG,0,12,&data);
    		if(retVal != GT_OK)
	    	{
	    	    gtSemGive(dev,dev->atuRegsSem);
    	    	return retVal;
	    	}
			entry->DBNum = (GT_U16)data;
		}
		else if (IS_IN_DEV_GROUP(dev,DEV_DBNUM_256))
		{
		    retVal = hwGetGlobalRegField(dev,QD_REG_ATU_CONTROL,12,4,&data);
    		if(retVal != GT_OK)
	    	{
	    	    gtSemGive(dev,dev->atuRegsSem);
    	    	return retVal;
	    	}
			entry->DBNum = (GT_U16)data << 4;
		}
		else if (IS_IN_DEV_GROUP(dev,DEV_DBNUM_64))
		{
			retVal = hwGetGlobalRegField(dev,QD_REG_ATU_OPERATION,8,2,&data);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->atuRegsSem);
				return retVal;
			}
			entry->DBNum = (GT_U16)data << 4;
		}

		if(!IS_IN_DEV_GROUP(dev,DEV_FID_REG))
		{
			retVal = hwGetGlobalRegField(dev,QD_REG_ATU_OPERATION,0,4,&data);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->atuRegsSem);
				return retVal;
			}

			entry->DBNum |= (GT_U8)(data & 0xF);
		}

		/* get the Source Port ID that was involved in the violation */

		retVal = hwReadGlobalReg(dev,QD_REG_ATU_DATA_REG,&data);
		if(retVal != GT_OK)
		{
			gtSemGive(dev,dev->atuRegsSem);
			return retVal;
		}

		entry->entryState.ucEntryState = data & 0xF;

        /* Get the Mac address  */
        for(i = 0; i < 3; i++)
        {
            retVal = hwReadGlobalReg(dev,(GT_U8)(QD_REG_ATU_MAC_BASE+i),&data);
            if(retVal != GT_OK)
            {
                gtSemGive(dev,dev->atuRegsSem);
                return retVal;
            }
            entry->macAddr.arEther[2*i] = data >> 8;
            entry->macAddr.arEther[1 + 2*i] = data & 0xFF;
        }


	} /* end of service violations */
    /* If the operation is a gen next operation wait for the response   */
    if(atuOp == GET_NEXT_ENTRY)
    {
		entry->trunkMember = GT_FALSE;
		entry->exPrio.useMacFPri = GT_FALSE;
		entry->exPrio.macFPri = 0;
        entry->exPrio.macQPri = 0;

        /* Wait until the ATU in ready. */
        data = 1;
        while(data == 1)
        {
            retVal = hwGetGlobalRegField(dev,QD_REG_ATU_OPERATION,15,1,&data);
            if(retVal != GT_OK)
            {
                gtSemGive(dev,dev->atuRegsSem);
                return retVal;
            }
        }

        /* Get the Mac address  */
        for(i = 0; i < 3; i++)
        {
            retVal = hwReadGlobalReg(dev,(GT_U8)(QD_REG_ATU_MAC_BASE+i),&data);
            if(retVal != GT_OK)
            {
                gtSemGive(dev,dev->atuRegsSem);
                return retVal;
            }
            entry->macAddr.arEther[2*i] = data >> 8;
            entry->macAddr.arEther[1 + 2*i] = data & 0xFF;
        }

        retVal = hwReadGlobalReg(dev,QD_REG_ATU_DATA_REG,&data);
        if(retVal != GT_OK)
        {
            gtSemGive(dev,dev->atuRegsSem);
            return retVal;
        }

        /* Get the Atu data register fields */
		if(IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY|DEV_88E6097_FAMILY))
		{
			if (IS_IN_DEV_GROUP(dev,DEV_TRUNK))
			{
				entry->trunkMember = (data & 0x8000)?GT_TRUE:GT_FALSE;
			}

			entry->portVec = (data >> 4) & portMask;
			entry->entryState.ucEntryState = data & 0xF;
			retVal = hwGetGlobalRegField(dev,QD_REG_ATU_OPERATION,8,3,&data);
			if(retVal != GT_OK)
			{
				gtSemGive(dev,dev->atuRegsSem);
				return retVal;
			}
			entry->prio = data;
		}
		else if(IS_IN_DEV_GROUP(dev,DEV_ATU_EXT_PRI))
		{
	        entry->prio = 0;
    	    entry->portVec = (data >> 4) & portMask;
        	entry->entryState.ucEntryState = data & 0xF;
			entry->exPrio.useMacFPri = (data & 0x2000)?GT_TRUE:GT_FALSE;
			entry->exPrio.macFPri = (data >> 10) & 0x7;
	        entry->exPrio.macQPri = data >> 14;
		}
		else
		{
	        entry->prio = data >> 14;
    	    entry->portVec = (data >> 4) & portMask;
        	entry->entryState.ucEntryState = data & 0xF;
		}
    }

    gtSemGive(dev,dev->atuRegsSem);
    return GT_OK;
}

static GT_STATUS atuStateAppToDev
(
    IN  GT_QD_DEV	*dev,
	IN  GT_BOOL		unicast,
	IN  GT_U32		state,
	OUT GT_U32		*newOne
)
{
	GT_U32	newState;
	GT_STATUS	retVal = GT_OK;

	if(unicast)
	{
		switch ((GT_ATU_UC_STATE)state)
		{
			case GT_UC_INVALID:
				newState = state;
				break;

			case GT_UC_DYNAMIC:
				if (IS_IN_DEV_GROUP(dev,DEV_UC_7_DYNAMIC))
				{
					newState = 7;
				}
				else
				{
					newState = 0xE;
				}
				break;

			case GT_UC_NO_PRI_TO_CPU_STATIC_NRL:
				if (IS_IN_DEV_GROUP(dev,DEV_UC_NO_PRI_TO_CPU_STATIC_NRL))
				{
					newState = state;
				}
				else
				{
					newState = (GT_U32)GT_UC_STATIC;
					retVal = GT_BAD_PARAM;
				}
				break;

			case GT_UC_TO_CPU_STATIC_NRL:
				if (IS_IN_DEV_GROUP(dev,DEV_UC_TO_CPU_STATIC_NRL))
				{
					newState = state;
				}
				else
				{
					newState = (GT_U32)GT_UC_STATIC;
					retVal = GT_BAD_PARAM;
				}
				break;

			case GT_UC_NO_PRI_STATIC_NRL:
				if (IS_IN_DEV_GROUP(dev,DEV_UC_NO_PRI_STATIC_NRL))
				{
					newState = state;
				}
				else
				{
					newState = (GT_U32)GT_UC_STATIC;
					retVal = GT_BAD_PARAM;
				}
				break;

			case GT_UC_STATIC_NRL:
				if (IS_IN_DEV_GROUP(dev,DEV_UC_STATIC_NRL))
				{
					newState = state;
				}
				else
				{
					newState = (GT_U32)GT_UC_STATIC;
					retVal = GT_BAD_PARAM;
				}
				break;

			case GT_UC_NO_PRI_TO_CPU_STATIC:
				if (IS_IN_DEV_GROUP(dev,DEV_UC_NO_PRI_TO_CPU_STATIC))
				{
					newState = state;
				}
				else
				{
					newState = (GT_U32)GT_UC_STATIC;
					retVal = GT_BAD_PARAM;
				}
				break;

			case GT_UC_TO_CPU_STATIC:
				if (IS_IN_DEV_GROUP(dev,DEV_UC_TO_CPU_STATIC))
				{
					newState = state;
				}
				else
				{
					newState = (GT_U32)GT_UC_STATIC;
					retVal = GT_BAD_PARAM;
				}
				break;

			case GT_UC_NO_PRI_STATIC:
				if (IS_IN_DEV_GROUP(dev,DEV_UC_NO_PRI_STATIC))
				{
					newState = state;
				}
				else
				{
					newState = (GT_U32)GT_UC_STATIC;
					retVal = GT_BAD_PARAM;
				}
				break;

			case GT_UC_STATIC:
				if (IS_IN_DEV_GROUP(dev,DEV_UC_STATIC))
				{
					newState = state;
				}
				else
				{
					newState = (GT_U32)GT_UC_STATIC;
					retVal = GT_BAD_PARAM;
				}
				break;

			default:
				if (IS_IN_DEV_GROUP(dev,DEV_UC_7_DYNAMIC))
				{
					newState = 7;
				}
				else
				{
					newState = 0xE;
				}
				retVal = GT_BAD_PARAM;
				break;

		}
	}
	else
	{
		switch ((GT_ATU_UC_STATE)state)
		{
			case GT_MC_INVALID:
				newState = state;
				break;

			case GT_MC_MGM_STATIC_UNLIMITED_RATE:
				if (IS_IN_DEV_GROUP(dev,DEV_MC_MGM_STATIC_UNLIMITED_RATE))
				{
					newState = state;
				}
				else
				{
					newState = (GT_U32)GT_MC_STATIC;
					retVal = GT_BAD_PARAM;
				}
				break;

			case GT_MC_STATIC_UNLIMITED_RATE:
				if (IS_IN_DEV_GROUP(dev,DEV_MC_STATIC_UNLIMITED_RATE))
				{
					newState = state;
				}
				else
				{
					newState = (GT_U32)GT_MC_STATIC;
					retVal = GT_BAD_PARAM;
				}
				break;

			case GT_MC_MGM_STATIC:
				if (IS_IN_DEV_GROUP(dev,DEV_MC_MGM_STATIC))
				{
					newState = state;
				}
				else
				{
					newState = (GT_U32)GT_MC_STATIC;
					retVal = GT_BAD_PARAM;
				}
				break;

			case GT_MC_STATIC:
				if (IS_IN_DEV_GROUP(dev,DEV_MC_STATIC))
				{
					newState = state;
				}
				else
				{
					newState = (GT_U32)GT_MC_STATIC;
					retVal = GT_BAD_PARAM;
				}
				break;

			case GT_MC_PRIO_MGM_STATIC_UNLIMITED_RATE:
				if (IS_IN_DEV_GROUP(dev,DEV_MC_PRIO_MGM_STATIC_UNLIMITED_RATE))
				{
					newState = state;
				}
				else
				{
					newState = (GT_U32)GT_MC_STATIC;
					retVal = GT_BAD_PARAM;
				}
				break;

			case GT_MC_PRIO_STATIC_UNLIMITED_RATE:
				if (IS_IN_DEV_GROUP(dev,DEV_MC_PRIO_STATIC_UNLIMITED_RATE))
				{
					newState = state;
				}
				else
				{
					newState = (GT_U32)GT_MC_STATIC;
					retVal = GT_BAD_PARAM;
				}
				break;

			case GT_MC_PRIO_MGM_STATIC:
				if (IS_IN_DEV_GROUP(dev,DEV_MC_PRIO_MGM_STATIC))
				{
					newState = state;
				}
				else
				{
					newState = (GT_U32)GT_MC_STATIC;
					retVal = GT_BAD_PARAM;
				}
				break;

			case GT_MC_PRIO_STATIC:
				if (IS_IN_DEV_GROUP(dev,DEV_MC_PRIO_STATIC))
				{
					newState = state;
				}
				else
				{
					newState = (GT_U32)GT_MC_STATIC;
					retVal = GT_BAD_PARAM;
				}
				break;

			default:
				newState = (GT_U32)GT_MC_STATIC;
				retVal = GT_BAD_PARAM;
				break;

		}
	}
	
	*newOne = newState;
	return retVal;
}

static GT_STATUS atuStateDevToApp
(
    IN  GT_QD_DEV	*dev,
	IN  GT_BOOL		unicast,
	IN  GT_U32		state,
	OUT GT_U32		*newOne
)
{
	GT_U32	newState;
	GT_STATUS	retVal = GT_OK;

	if(unicast)
	{
		if (state == 0)
		{
			newState = (GT_U32)GT_UC_INVALID;
		}
		else if (state <= 7)
		{
			newState = (GT_U32)GT_UC_DYNAMIC;
		}
		else if ((state <= 0xE) && (!IS_IN_DEV_GROUP(dev,DEV_UC_7_DYNAMIC)))
		{
			newState = (GT_U32)GT_UC_DYNAMIC;
		}
		else
		{
			newState = state;
		}
	}
	else
	{
		newState = state;
	}

	*newOne = newState;
	return retVal;
}


static GT_STATUS atuGetStats
(
    IN  GT_QD_DEV	*dev,
	IN  GT_ATU_STAT	*atuStat,
	OUT GT_U32		*count
)
{
    GT_U32          numOfEntries, dbNum;
    GT_ATU_ENTRY    entry;
	GT_U16			data,mode,bin;
    GT_STATUS       retVal;

    DBG_INFO(("atuGetStats Called.\n"));

	switch (atuStat->op)
	{
		case GT_ATU_STATS_ALL:
		case GT_ATU_STATS_NON_STATIC:
			dbNum = 0;
			break;
		case GT_ATU_STATS_ALL_FID:
		case GT_ATU_STATS_NON_STATIC_FID:
			dbNum = atuStat->DBNum;
			break;
		default:
			return GT_FALSE;
	}

    numOfEntries = 0;
	mode = atuStat->op;

	for(bin=0; bin<4; bin++)
	{	
		data = (bin << 14) | (mode << 12);

	    retVal = hwWriteGlobal2Reg(dev, QD_REG_ATU_STATS, data);
       	if(retVal != GT_OK)
        {
   	        DBG_INFO(("Failed.\n"));
       	    return retVal;
        }
		
		entry.DBNum = (GT_U16)dbNum;
	    gtMemSet(entry.macAddr.arEther,0,sizeof(GT_ETHERADDR));

        retVal = atuOperationPerform(dev,GET_NEXT_ENTRY,NULL,&entry);
       	if(retVal == GT_FAIL)
        {
   	        DBG_INFO(("Failed.\n"));
       	    return retVal;
        }

	    retVal = hwReadGlobal2Reg(dev, QD_REG_ATU_STATS, &data);
       	if(retVal != GT_OK)
        {
   	        DBG_INFO(("Failed.\n"));
       	    return retVal;
        }
		
		numOfEntries += (data & 0xFFF);
	}

	*count = numOfEntries;

	return GT_OK;
}

