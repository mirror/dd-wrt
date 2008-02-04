#include <Copyright.h>

/********************************************************************************
* gtDrvEvents.h
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

#ifndef __gtDrvEventsh
#define __gtDrvEventsh

#include <msApi.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* drvEventInit
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
    IN  GT_QD_DEV     *dev,
    IN GT_U32         intVecNum,
    IN GT_VOIDFUNCPTR isrFunc
);



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
	IN  GT_QD_DEV  *dev,
	OUT GT_U16*    intCause
);

#ifdef __cplusplus
}
#endif

#endif /* __gtDrvEventsh */
