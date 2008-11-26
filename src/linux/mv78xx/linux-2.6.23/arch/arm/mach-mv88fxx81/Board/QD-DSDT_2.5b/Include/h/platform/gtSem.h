#include <Copyright.h>

/********************************************************************************
* gtOs.h
*
* DESCRIPTION:
*       Operating System wrapper
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*******************************************************************************/

#ifndef __gtSemh
#define __gtSemh

#include <msApi.h>

#ifdef __cplusplus
extern "C" {
#endif

/***** Defines  ********************************************************/

#define OS_WAIT_FOREVER             0

#define OS_MAX_TASKS                30
#define OS_MAX_TASK_NAME_LENGTH     10

#define OS_MAX_QUEUES               30
#define OS_MAX_QUEUE_NAME_LENGTH    10

#define OS_MAX_EVENTS               10

#define OS_MAX_SEMAPHORES           50

#define OS_EOF                      (-1)


/*******************************************************************************
* gtSemCreate
*
* DESCRIPTION:
*       Create semaphore.
*
* INPUTS:
*		state - beginning state of the semaphore, either SEM_EMPTY or SEM_FULL
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
);

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
    IN GT_SEM       smid
);

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
*
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
    IN GT_SEM       smid,
    IN GT_U32       timeOut
);

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
);

#ifdef __cplusplus
}
#endif

#endif  /* __gtSemh */
/* Do Not Add Anything Below This Line */
