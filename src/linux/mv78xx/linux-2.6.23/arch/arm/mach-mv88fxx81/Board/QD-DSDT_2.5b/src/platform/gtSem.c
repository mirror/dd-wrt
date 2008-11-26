#include <Copyright.h>
/********************************************************************************
* gtOs.c
*
* DESCRIPTION:
*       Semaphore related routines
*
* DEPENDENCIES:
*       OS Dependent.
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*******************************************************************************/

#include <msApi.h>
#include <gtSem.h>


/*******************************************************************************
* gtSemCreate
*
* DESCRIPTION:
*       Create semaphore.
*
* INPUTS:
*		state - beginning state of the semaphore, either GT_SEM_EMPTY or GT_SEM_FULL
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_SEM if success. Otherwise, NULL
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_SEM gtSemCreate
(
	IN GT_QD_DEV    *dev,
	IN GT_SEM_BEGIN_STATE state
)
{
	if(dev->semCreate)
		return dev->semCreate(state);

	return 1; /* should return any value other than 0 to let it keep going */
}

/*******************************************************************************
* gtSemDelete
*
* DESCRIPTION:
*       Delete semaphore.
*
* INPUTS:
*       smid - semaphore Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gtSemDelete
(
	IN GT_QD_DEV    *dev,
	IN GT_SEM smid
)
{
	if((dev->semDelete) && (smid))
		return dev->semDelete(smid);

	return GT_OK;
}


/*******************************************************************************
* gtSemTake
*
* DESCRIPTION:
*       Wait for semaphore.
*
* INPUTS:
*       smid    - semaphore Id
*       timeOut - time out in miliseconds or 0 to wait forever
*
* OUTPUTS:
*       None
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*       OS_TIMEOUT - on time out
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gtSemTake
(
	IN GT_QD_DEV    *dev,
	IN GT_SEM smid, 
	IN GT_U32 timeOut
)
{
	if(dev->semTake)
		return dev->semTake(smid, timeOut);

	return GT_OK;

}

/*******************************************************************************
* gtSemGive
*
* DESCRIPTION:
*       release the semaphore which was taken previously.
*
* INPUTS:
*       smid    - semaphore Id
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_STATUS gtSemGive
(
	IN GT_QD_DEV    *dev,
	IN GT_SEM       smid
)
{
	if(dev->semGive)
		return dev->semGive(smid);

	return GT_OK;
}

