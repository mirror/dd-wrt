#include <Copyright.h>

/********************************************************************************
* gtPortStat.c
*
* DESCRIPTION:
*       API implementation for switch port rx/tx counters.
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/

#include <msApi.h>
#include <gtHwCntl.h>
#include <gtDrvSwRegs.h>


/*******************************************************************************
* gprtSetCtrMode
*
* DESCRIPTION:
*       This routine sets the port rx/tx counters mode of operation.
*
* INPUTS:
*       mode  - the counter mode.
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
GT_STATUS gprtSetCtrMode
(
    IN GT_QD_DEV    *dev,
    IN GT_CTR_MODE  mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gprtSetCtrMode Called.\n"));

	if (IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,8,1,(GT_U16)mode);
    if(retVal != GT_OK)
    {
        DBG_INFO(("gprtSetCtrMode Failed .\n"));
        return GT_FAIL;
    }

    DBG_INFO(("gprtSetCtrMode OK .\n"));
    return retVal;
}



/*******************************************************************************
* gprtClearAllCtr
*
* DESCRIPTION:
*       This routine clears all port rx/tx counters.
*
* INPUTS:
*       None.
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
GT_STATUS gprtClearAllCtr
(
    IN GT_QD_DEV    *dev
)
{
    IN GT_STATUS     retVal;         /* Functions return value.      */
    IN GT_U16        mode;           /* hold counters current mode   */

    DBG_INFO(("gprtClearAllCtr Called.\n"));

	if (IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    /* get counter current mode  */
    if(hwGetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,8,1,&mode) != GT_OK)
    {
        DBG_INFO(("Failed (Get field).\n"));
        return GT_FAIL;
    }
    /* write opposite value to reset counter */
    if(hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,8,1,(GT_U16)(1 - mode)) != GT_OK)
    {
        DBG_INFO(("Failed (Get field).\n"));
        return GT_FAIL;
    }
    /* restore counters mode */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,8,1,mode);

    DBG_INFO(("OK.\n"));
    return retVal;
}



/*******************************************************************************
* gprtGetPortCtr
*
* DESCRIPTION:
*       This routine gets the port rx/tx counters.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       ctr - the counters value.
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
GT_STATUS gprtGetPortCtr
(
    IN  GT_QD_DEV       *dev,
    IN  GT_LPORT        port,
    OUT GT_PORT_STAT    *ctr
)
{
    GT_U16          count;          /* counters current value       */
    GT_U8           hwPort;         /* physical port number         */

    DBG_INFO(("gprtGetPortCtr Called.\n"));

	if (IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    if(ctr  == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }
    /* translate logical port to physical port */
    hwPort = GT_LPORT_2_PORT(port);

    /* get rx counter value  */
    if(hwReadPortReg(dev,hwPort, QD_REG_RX_COUNTER, &count) != GT_OK)
    {
        DBG_INFO(("Failed (Read Rx).\n"));
        return GT_FAIL;
    }
    ctr->rxCtr = count;
    /* get tx counter value  */
    if(hwReadPortReg(dev,hwPort, QD_REG_TX_COUNTER, &count) != GT_OK)
    {
        DBG_INFO(("Failed (Read Tx).\n"));
        return GT_FAIL;
    }
    ctr->txCtr = count;

	if (IS_IN_DEV_GROUP(dev,DEV_ENHANCED_FE_SWITCH))
	{
	    /* get dropped counter value  */
    	if(hwReadPortReg(dev,hwPort, QD_REG_DROPPED_COUNTER, &count) != GT_OK)
	    {
    	    DBG_INFO(("Failed (Read Tx).\n"));
        	return GT_FAIL;
	    }
		ctr->dropped = count;
	}
	else
		ctr->dropped = 0;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* gprtGetPortCtr2
*
* DESCRIPTION:
*       This routine gets the port InDiscards, InFiltered, and OutFiltered counters.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       ctr - the counters value.
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
GT_STATUS gprtGetPortCtr2
(
    IN  GT_QD_DEV       *dev,
    IN  GT_LPORT        port,
    OUT GT_PORT_STAT2   *ctr
)
{
    GT_U16          count;          /* counters current value       */
    GT_U8           hwPort;         /* physical port number         */

    DBG_INFO(("gprtGetPortCtr2 Called.\n"));

	if (!IS_IN_DEV_GROUP(dev,DEV_88E6093_FAMILY))
	{
		DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
	}

    if(ctr  == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* translate logical port to physical port */
    hwPort = GT_LPORT_2_PORT(port);

    /* get InDiscard Low counter value  */
    if(hwReadPortReg(dev,hwPort, QD_REG_INDISCARD_LO_COUNTER, &count) != GT_OK)
    {
        DBG_INFO(("Failed (Read inDiscardLo).\n"));
        return GT_FAIL;
    }
    ctr->inDiscardLo = count;
    /* get InDiscard High counter value  */
    if(hwReadPortReg(dev,hwPort, QD_REG_INDISCARD_HI_COUNTER, &count) != GT_OK)
    {
        DBG_INFO(("Failed (Read inDiscardHi).\n"));
        return GT_FAIL;
    }
    ctr->inDiscardHi = count;

    /* get InFiltered counter value  */
    if(hwReadPortReg(dev,hwPort, QD_REG_INFILTERED_COUNTER, &count) != GT_OK)
    {
        DBG_INFO(("Failed (Read inFiltered).\n"));
        return GT_FAIL;
    }
    ctr->inFiltered = count;

    /* get OutFiltered counter value  */
    if(hwReadPortReg(dev,hwPort, QD_REG_OUTFILTERED_COUNTER, &count) != GT_OK)
    {
        DBG_INFO(("Failed (Read outFiltered).\n"));
        return GT_FAIL;
    }
    ctr->outFiltered = count;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


#ifdef DEBUG_FEATURE /* this is a debug feature*/
/*******************************************************************************
* gprtGetPortQueueCtr
*
* DESCRIPTION:
*       This routine gets the port queue counters.
*
* INPUTS:
*       port  - the logical port number.
*
* OUTPUTS:
*       ctr - the counters value.
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
GT_STATUS gprtGetPortQueueCtr
(
    IN  GT_QD_DEV       *dev,
    IN  GT_LPORT        port,
    OUT GT_PORT_Q_STAT  *ctr
)
{
    GT_U16          count;          /* counters current value       */
    GT_U8           hwPort;         /* physical port number         */

    DBG_INFO(("gprtGetPortQueueCtr Called.\n"));

    if(ctr  == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* translate logical port to physical port */
    hwPort = GT_LPORT_2_PORT(port);

    /* get queue counter value  */
    if(hwReadPortReg(dev,hwPort, QD_REG_Q_COUNTER, &count) != GT_OK)
    {
        DBG_INFO(("Failed (Read Rx).\n"));
        return GT_FAIL;
    }

    /* the fist 5 bits(4:0) are OutQ_Size */
    ctr->OutQ_Size = count & 0x1F;

    /* the Rsv_Size are bits 15:8 */
    ctr->Rsv_Size  = count >> 8;

    DBG_INFO(("OK.\n"));
    return GT_OK;
}
#endif 
