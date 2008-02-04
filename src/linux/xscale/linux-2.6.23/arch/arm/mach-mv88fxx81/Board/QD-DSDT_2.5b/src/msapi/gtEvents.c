#include <Copyright.h>

/********************************************************************************
* gtEvents.c
*
* DESCRIPTION:
*       API definitions for system interrupt events handling.
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
* eventSetActive
*
* DESCRIPTION:
*       This routine enables/disables the receive of an hardware driven event.
*
* INPUTS:
*       eventType - the event type. any combination of the folowing: 
*       	GT_STATS_DONE, GT_VTU_PROB, GT_VTU_DONE, GT_ATU_FULL(or GT_ATU_PROB),
*       	GT_ATU_DONE, GT_PHY_INTERRUPT, and GT_EE_INTERRUPT
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
GT_STATUS eventSetActive
(
    IN GT_QD_DEV *dev,
    IN GT_U32 	 eventType
)
{
    GT_STATUS   retVal;   
    GT_U16 	data;
	GT_U16	intMask;

    DBG_INFO(("eventSetActive Called.\n"));

	data = (GT_U16) eventType;

	if (IS_IN_DEV_GROUP(dev,DEV_EXTERNAL_PHY_ONLY))
    {
		intMask = GT_NO_INTERNAL_PHY_INT_MASK;
    }
	else
	{
		intMask = GT_INT_MASK;
	}
	
	if(data & ~intMask)
	{
	    DBG_INFO(("Invalid event type.\n"));
		return GT_FAIL;
	}

    /* Set the IntEn bit.               */
    retVal = hwSetGlobalRegField(dev,QD_REG_GLOBAL_CONTROL,0,7,data);
    if(retVal != GT_OK)
    {
        DBG_INFO(("Failed.\n"));
        return retVal;
    }
    DBG_INFO(("OK.\n"));
    return GT_OK;
}


/*******************************************************************************
* eventGetIntStatus
*
* DESCRIPTION:
*       This routine reads an hardware driven event status.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       intCause -  It provides the source of interrupt of the following:
*       GT_STATS_DONE, GT_VTU_PROB, GT_VTU_DONE, GT_ATU_FULL,  
*       GT_ATU_DONE, GT_PHY_INTERRUPT, and GT_EE_INTERRUPT. 
*		For Gigabit Switch, GT_ATU_FULL is replaced with GT_ATU_PROB and 
*		if there is no internal phy, GT_PHY_INTERRUPT is not supported.
*
* RETURNS:
*       GT_TRUE - read success and there is a pending event.
*       GT_FAIL - otherwise
*
* COMMENTS:
*       None.
*
* GalTis:
*
*******************************************************************************/
GT_BOOL eventGetIntStatus
(
    IN GT_QD_DEV *dev,
    OUT GT_U16   *intCause
)
{
    GT_STATUS       retVal;         /* Function calls return value.     */

    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_STATUS,0,7,intCause);

    if(retVal != GT_OK)
        return GT_FALSE;

    return (*intCause)?GT_TRUE:GT_FALSE;
}


/*******************************************************************************
* gvtuGetIntStatus
*
* DESCRIPTION:
* Check to see if a specific type of VTU interrupt occured
*
* INPUTS:
*       	intType - the type of interrupt which causes an interrupt.
*			any combination of 
*			GT_MEMEBER_VIOLATION,
*			GT_MISS_VIOLATION,
*			GT_FULL_VIOLATION
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
* FULL_VIOLATION is only for Fast Ethernet Switch (not for Gigabit Switch).
*
*******************************************************************************/

GT_STATUS gvtuGetIntStatus
(
    IN GT_QD_DEV          *dev,
    OUT GT_VTU_INT_STATUS *vtuIntStatus
)
{
    GT_STATUS       retVal;      

    DBG_INFO(("gvtuGetIntStatus Called.\n"));
   
    /* check if device supports this feature */
    if((IS_VALID_API_CALL(dev,1, DEV_802_1Q)) != GT_OK ) 
      return GT_FAIL; 

	if (IS_IN_DEV_GROUP(dev,DEV_GIGABIT_SWITCH))
	{
	    retVal = gvtuGetViolation2(dev,vtuIntStatus);
	}
	else if (IS_IN_DEV_GROUP(dev,DEV_ENHANCED_FE_SWITCH))
	{
	    retVal = gvtuGetViolation3(dev,vtuIntStatus);
	}
	else
	{
	    retVal = gvtuGetViolation(dev,vtuIntStatus);
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
* gatuGetIntStatus
*
* DESCRIPTION:
* Check to see if a specific type of ATU interrupt occured
*
* INPUTS:
*       	intType - the type of interrupt which causes an interrupt.
*			any combination of 
*			GT_MEMEBER_VIOLATION,
*			GT_MISS_VIOLATION,
*			GT_FULL_VIOLATION 
*
* OUTPUTS:
* None.
*
* RETURNS:
* GT_OK - on success
* GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/

GT_STATUS gatuGetIntStatus
(
    IN GT_QD_DEV          *dev,
    OUT GT_ATU_INT_STATUS *atuIntStatus
)
{
    GT_STATUS       retVal;      

    DBG_INFO(("gatuGetIntStatus Called.\n"));
   
    /* check if device supports this feature */
	if (!IS_IN_DEV_GROUP(dev,DEV_GIGABIT_MANAGED_SWITCH|DEV_ENHANCED_FE_SWITCH))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    retVal = gatuGetViolation(dev,atuIntStatus);
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
* geventSetAgeIntEn
*
* DESCRIPTION:
*		This routine enables/disables Age Interrupt for a port.
*		When it's enabled, ATU Age Violation interrupts from this port are enabled.
*		An Age Violation will occur anytime a port is Locked(gprtSetLockedPort) 
*		and the ingressing frame's SA is contained in the ATU as a non-Static 
*		entry with a EntryState less than 0x4.
*
* INPUTS:
*		port - the logical port number
*		mode - GT_TRUE to enable Age Interrupt,
*			   GT_FALUSE to disable
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
*******************************************************************************/
GT_STATUS geventSetAgeIntEn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	IN  GT_BOOL		mode
)
{
    GT_U16          data;           
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("geventSetAgeIntEn Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PORT_BASED_AGE_INT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* translate BOOL to binary */
    BOOL_2_BIT(mode, data);

    /* Set Age Interrupt Enable Mode.            */
    retVal = hwSetPortRegField(dev,hwPort, QD_REG_PORT_ASSOCIATION,11,1,data);

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
* geventGetAgeIntEn
*
* DESCRIPTION:
*		This routine gets Age Interrupt Enable for the port.
*		When it's enabled, ATU Age Violation interrupts from this port are enabled.
*		An Age Violation will occur anytime a port is Locked(gprtSetLockedPort) 
*		and the ingressing frame's SA is contained in the ATU as a non-Static 
*		entry with a EntryState less than 0x4.
*
* INPUTS:
*		port - the logical port number
*		mode - GT_TRUE to enable Age Interrupt,
*			   GT_FALUSE to disable
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
*******************************************************************************/
GT_STATUS geventGetAgeIntEn
(
	IN  GT_QD_DEV	*dev,
	IN  GT_LPORT	port,
	OUT GT_BOOL		*mode
)
{
    GT_U16          data;           
    GT_STATUS       retVal;         /* Functions return value.      */
    GT_U8           hwPort;         /* the physical port number     */

    DBG_INFO(("geventGetAgeIntEn Called.\n"));

    /* translate LPORT to hardware port */
    hwPort = GT_LPORT_2_PORT(port);

	if (!IS_IN_DEV_GROUP(dev,DEV_PORT_BASED_AGE_INT))
    {
        DBG_INFO(("GT_NOT_SUPPORTED\n"));
		return GT_NOT_SUPPORTED;
    }

    /* Get Age Interrupt Enable Mode.            */
    retVal = hwGetPortRegField(dev,hwPort, QD_REG_PORT_ASSOCIATION,11,1,&data);

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


