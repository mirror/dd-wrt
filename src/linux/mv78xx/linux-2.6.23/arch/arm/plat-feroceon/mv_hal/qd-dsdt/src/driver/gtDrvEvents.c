#include <Copyright.h>
/********************************************************************************
* gtDrvEvents.c
*
* DESCRIPTION:
*       This file includes function declarations for QuarterDeck interrupts
*       configuration and handling.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <gtDrvSwRegs.h>
#include <gtHwCntl.h>
#include <gtDrvEvents.h>

/*******************************************************************************
* drvEventsInit
*
* DESCRIPTION:
*       This function initializes the driver's interrupt handling mechanism.
*
* INPUTS:
*       intVecNum   - The interrupt vector the switch is connected to.
*       isrFunc     - A pointer to the Interrupt Service Routine to be
*                     connected to the given interrupt vector.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_STATUS drvEventsInit
(
    IN  GT_QD_DEV       *dev,
    IN GT_U32           intVecNum,
    IN GT_VOIDFUNCPTR   isrFunc
)
{
#if 0
    return osInterruptConnect(intVecNum,isrFunc,0);
#endif
	return GT_OK;
}



/*******************************************************************************
* eventQdSr
*
* DESCRIPTION:
*       QuarterDeck interrupt service routine.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_BOOL eventQdSr
(
	IN  GT_QD_DEV* dev,
	OUT GT_U16* intCause
)
{
    GT_STATUS       retVal;         /* Function calls return value.     */

    retVal = hwGetGlobalRegField(dev,QD_REG_GLOBAL_STATUS,0,4,intCause);

    if(retVal != GT_OK)
        return GT_FALSE;

    return (*intCause)?GT_TRUE:GT_FALSE;
}

