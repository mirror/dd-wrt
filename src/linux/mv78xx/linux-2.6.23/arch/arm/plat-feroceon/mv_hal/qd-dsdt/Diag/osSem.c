#include <Copyright.h>
/********************************************************************************
* osSem.c
*
* DESCRIPTION:
*       Semaphore related routines
*
* DEPENDENCIES:
*       OS Dependent.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#ifdef _VXWORKS
#include "vxWorks.h"
#include "semLib.h"
#include "errnoLib.h"
#include "objLib.h"
int sysClkRateGet(void);

#elif defined(WIN32)
#include "windows.h"
/* #include "wdm.h" */
#elif defined(LINUX)
#include "/usr/include/semaphore.h"
typedef    sem_t          semaphore ;
#endif

#include <msApi.h>

GT_SEM osSemCreate( GT_SEM_BEGIN_STATE state);
GT_STATUS osSemDelete(GT_SEM smid);
GT_STATUS osSemWait(  GT_SEM smid, GT_U32 timeOut);
GT_STATUS osSemSignal(GT_SEM smid);

/*******************************************************************************
* osSemCreate
*
* DESCRIPTION:
*       Create semaphore.
*
* INPUTS:
*       name   - semaphore Name
*       init   - init value of semaphore counter
*       count  - max counter (must be >= 1)
*
* OUTPUTS:
*       smid - semaphore Id
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_SEM osSemCreate(GT_SEM_BEGIN_STATE state)
{
#ifdef _VXWORKS
#if 0
	return (GT_SEM)semBCreate(SEM_Q_FIFO, state);
#else
	GT_SEM semid;
	semid =(GT_SEM)semBCreate(SEM_Q_FIFO, state);
	return semid;
#endif

#elif defined(WIN32)
	return (GT_SEM)CreateSemaphore(NULL, state, 1, NULL);
#elif defined(LINUX)
	semaphore lxSem;

	sem_init(&lxSem, state, 1);
	return lxSem;
#else
	return 1;
#endif
	return GT_OK;
}

/*******************************************************************************
* osSemDelete
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
GT_STATUS osSemDelete(GT_SEM smid)
{
#ifdef _VXWORKS
	STATUS rc;

	rc = semDelete((SEM_ID) smid);
	if (rc != OK)
		return GT_FAIL;

#elif defined(WIN32)
	if (CloseHandle((HANDLE)smid) == 0)
		return GT_FAIL;

#elif defined(LINUX)
	sem_destroy((semaphore*) smid);
#else
	return GT_OK;
#endif

	return GT_OK;
}

/*******************************************************************************
* osSemWait
*
* DESCRIPTION:
*       Wait on semaphore.
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
GT_STATUS osSemWait(GT_SEM smid, GT_U32 timeOut)
{
#ifdef _VXWORKS
	STATUS rc;

	if (timeOut == 0)
	rc = semTake ((SEM_ID) smid, WAIT_FOREVER);
	else
	{
		int num, delay;

		num = sysClkRateGet();
		delay = (num * timeOut) / 1000;
		if (delay < 1)
			rc = semTake ((SEM_ID) smid, 1);
		else
			rc = semTake ((SEM_ID) smid, delay);
	}

	if (rc != OK)
	{
		if (errno == S_objLib_OBJ_TIMEOUT)
			return GT_TIMEOUT;
		else
			return GT_FAIL;
	}

#elif defined(WIN32)
	DWORD rc;

	rc = WaitForSingleObject((HANDLE)smid, timeOut);

	if (rc == WAIT_ABANDONED)
		return GT_FAIL;
	if (rc == WAIT_TIMEOUT)
		return GT_TIMEOUT;

#elif defined(LINUX)
	sem_wait((semaphore*) smid) ; 
#else
	return GT_OK;

#endif

	return GT_OK;
}

/*******************************************************************************
* osSemSignal
*
* DESCRIPTION:
*       Signal a semaphore.
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
GT_STATUS osSemSignal(GT_SEM smid)
{
#ifdef _VXWORKS
	STATUS rc;
	rc = semGive ((SEM_ID) smid);
	if (rc != OK)
		return GT_FAIL;

#elif defined(WIN32)
	if(ReleaseSemaphore((HANDLE) smid, 1, NULL) == 0)
		return GT_FAIL;

#elif defined(LINUX)
	sem_post((semaphore*) smid) ; 
#else
	return GT_OK;
#endif
	return GT_OK;
}
