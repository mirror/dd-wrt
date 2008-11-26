#include <Copyright.h>

/*******************************************************************************
* gtSysStatus.c
*
* DESCRIPTION:
*       API definitions for system global status.
* 	Added for fullsail
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
* gsysGetPPUState
*
* DESCRIPTION:
*		This routine get the PPU State. These two bits return
*		the current value of the PPU.
*
* INPUTS:
*		None.
*
* OUTPUTS:
*		mode - GT_PPU_STATE
*
* RETURNS:
*		GT_OK           - on success
*		GT_BAD_PARAM    - on bad parameter
*		GT_FAIL         - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*		None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetPPUState
(
	IN  GT_QD_DEV   	*dev,
	OUT GT_PPU_STATE	*mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetPPUState Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
        DBG_INFO(("Not Supported.\n"));
		return GT_NOT_SUPPORTED;
	}

    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* get the bits from hardware */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_STATUS,14,2,&data);
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
* gsysGetSW_Mode
*
* DESCRIPTION:
*       This routine get the Switch mode. These two bits returen
*       the current value of the SW_MODE[1:0] pins.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE Discard is enabled, GT_FALSE otherwise.
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
GT_STATUS gsysGetSW_Mode
(
    IN  GT_QD_DEV  *dev,
    OUT GT_SW_MODE *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetSW_Mode Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_QD_PLUS|DEV_ENHANCED_FE_SWITCH))
	{
        DBG_INFO(("Not Supported.\n"));
		return GT_NOT_SUPPORTED;
	}

    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* get the bits from hardware */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_STATUS,12,2,&data);
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
* gsysGetInitReady
*
* DESCRIPTION:
*       This routine get the InitReady bit. This bit is set to a one when the ATU,
*       the Queue Controller and the Statistics Controller are done with their
*       initialization and are ready to accept frames.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       mode - GT_TRUE: switch is ready, GT_FALSE otherwise.
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
GT_STATUS gsysGetInitReady
(
    IN  GT_QD_DEV  *dev,
    OUT GT_BOOL    *mode
)
{
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U16          data;           /* The register's read data.    */

    DBG_INFO(("gsysGetInitReady Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_QD_PLUS|DEV_ENHANCED_FE_SWITCH))
	{
        DBG_INFO(("Not Supported.\n"));
		return GT_NOT_SUPPORTED;
	}

    if(mode == NULL)
    {
        DBG_INFO(("Failed.\n"));
        return GT_BAD_PARAM;
    }

    /* get the bits from hardware */
    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_STATUS,11,1,&data);
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
* gsysGetFreeQSize
*
* DESCRIPTION:
*       This routine gets Free Queue Counter. This counter reflects the
*		current number of unalllocated buffers available for all the ports.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       count - Free Queue Counter
*
* RETURNS:
*       GT_OK            - on success
*       GT_FAIL          - on error
*		GT_NOT_SUPPORTED - if current device does not support this feature.
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_STATUS gsysGetFreeQSize
(
    IN  GT_QD_DEV	*dev,
    OUT GT_U16 		*count
)
{
    GT_STATUS       retVal;         /* Functions return value.      */

    DBG_INFO(("gsysGetFreeQSize Called.\n"));

    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_OUT_Q_SIZE))
	{
        DBG_INFO(("Not Supported.\n"));
		return GT_NOT_SUPPORTED;
	}

    /* get the counter */
    retVal = hwGetGlobalRegField(dev,QD_REG_TOTAL_FREE_COUNTER,0,9,count);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }

    DBG_INFO(("OK.\n"));
    return GT_OK;
}


