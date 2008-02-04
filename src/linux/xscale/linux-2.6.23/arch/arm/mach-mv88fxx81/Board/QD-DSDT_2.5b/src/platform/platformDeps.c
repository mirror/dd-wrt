#include <Copyright.h>
/********************************************************************************
* platformDeps.c
*
* DESCRIPTION:
*       platform dependent functions
*
* DEPENDENCIES:   Platform.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#include <msApi.h>
#include <gtDrvEvents.h>
#include <gtHwCntl.h>
#include <platformDeps.h>

#if 0
/*******************************************************************************
* gtAssignFunctions
*
* DESCRIPTION:
*       Exchange MII access functions and QuarterDeck Int Handler.
*		MII access functions will be called by QuarterDeck driver and
*		QD Int Handler should be called by BSP when BSP sees an interrupt which is related to
*		QD (This interrupt has to be initialized by BSP, since QD has no idea which
*		interrupt is assigned to QD)
*
* INPUTS:
*       fReadMii 	- function to read MII registers
*		fWriteMii	- functino to write MII registers
*
* OUTPUTS:
*		None.
*
* RETURNS:
*       GT_TRUE, if input is valid. GT_FALSE, otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_BOOL gtAssignFunctions
(
   GT_QD_DEV      *dev,
   FGT_READ_MII   fReadMii,
   FGT_WRITE_MII fWriteMii
)
{
	if((fReadMii == NULL) || (fWriteMii == NULL))
		return GT_FALSE;

	dev->fgtReadMii = fReadMii;
	dev->fgtWriteMii = fWriteMii;
	
	return GT_TRUE;
}

#endif

GT_BOOL defaultMiiRead (unsigned int portNumber , unsigned int miiReg, unsigned int* value)
{
	if (portNumber > GLOBAL_REGS_START_ADDR)
		portNumber -= GLOBAL_REGS_START_ADDR;

	if (portNumber > GLOBAL_REGS_START_ADDR)
		return GT_FALSE;

	*value = 0;

	return GT_TRUE;
}

GT_BOOL defaultMiiWrite (unsigned int portNumber , unsigned int miiReg, unsigned int value)
{
	if (portNumber > GLOBAL_REGS_START_ADDR)
		portNumber -= GLOBAL_REGS_START_ADDR;

	if (portNumber > GLOBAL_REGS_START_ADDR)
		return GT_FALSE;

	value = value;

	return GT_TRUE;
}
