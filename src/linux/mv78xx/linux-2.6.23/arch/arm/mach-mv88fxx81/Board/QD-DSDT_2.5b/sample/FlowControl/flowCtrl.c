#include <Copyright.h>
/********************************************************************************
* flowCtrl.c
*
* DESCRIPTION:
*       Sample program which will show how to Enable or Disable Flow Control of 
*		the given Port of the QuaterDeck.
*		
*
* DEPENDENCIES:   None.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#include "msSample.h"

/*
 *	Enable or Disable Flow Control of the given port.
 *	Input - port : port to be programmed.
 *			enalble : either Enable or Disable.
*/
GT_STATUS sampleSetFlowControl(GT_QD_DEV *dev, GT_LPORT port, GT_BOOL enable)
{
	GT_STATUS status;

	/* 
	 *	Program Phy's Pause bit in AutoNegotiation Advertisement Register.
	 */
	if((status = gprtSetPause(dev,port,enable)) != GT_OK)
	{
		MSG_PRINT(("gprtSetForceFC return Failed\n"));
		return status;
	}

	/* 
	 *	Restart AutoNegotiation of the given Port's phy
	 */
	if((status = gprtPortRestartAutoNeg(dev,port)) != GT_OK)
	{
		MSG_PRINT(("gprtSetForceFC return Failed\n"));
		return status;
	}

	/* 
	 *	Program Port's Flow Control.
	 */
	if((status = gprtSetForceFc(dev,port,enable)) != GT_OK)
	{
		MSG_PRINT(("gprtSetForceFC return Failed\n"));
		return status;
	}

	return GT_OK;
}
