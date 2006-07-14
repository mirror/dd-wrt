/*
 * INTEL CONFIDENTIAL
 * Copyright (c) 2002 - 2005 Intel Corporation.  All rights reserved.
 * 
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel
 * Corporation or its suppliers or licensors.  Title to the
 * Material remains with Intel Corporation or its suppliers and
 * licensors.  The Material contains trade secrets and proprietary
 * and confidential information of Intel or its suppliers and
 * licensors. The Material is protected by worldwide copyright and
 * trade secret laws and treaty provisions.  No part of the Material
 * may be used, copied, reproduced, modified, published, uploaded,
 * posted, transmitted, distributed, or disclosed in any way without
 * Intel's prior express written permission.
 
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you
 * by disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise. Any license
 * under such intellectual property rights must be express and
 * approved by Intel in writing.
 * 
 * $Workfile: ILibThreadPool.c
 * $Revision: #1.0.2384.30305
 * $Author:   Intel Corporation, Intel Device Builder
 * $Date:     Thursday, July 13, 2006
 *
 *
 *
 */


#include "ILibThreadPool.h"

#if defined(WINSOCK2)
#include <winsock2.h>
#elif defined(WINSOCK1)
#include <winsock.h>
#endif

#include "ILibParsers.h"

struct ILibThreadPool_WorkItem
{
	ILibThreadPool_Handler Callback;
	void *var;
};
struct ILibThreadPool_ThreadState
{
	int NumThreads;
	int Terminate;
	void *WorkItemQueue;
	sem_t SyncHandle;
	sem_t AbortHandle;
};

/*! \fn ILibThreadPool ILibThreadPool_Create()
	\brief Instantiate a new ILibThreadPool handle
	\returns Handle to a new ILibThreadPool module
*/
ILibThreadPool ILibThreadPool_Create()
{
	struct ILibThreadPool_ThreadState *ts = (struct ILibThreadPool_ThreadState*)malloc(sizeof(struct ILibThreadPool_ThreadState));
	memset(ts,0,sizeof(struct ILibThreadPool_ThreadState));

	ts->WorkItemQueue = ILibQueue_Create();
	sem_init(&(ts->SyncHandle),0,0);
	sem_init(&(ts->AbortHandle),0,0);
	return(ts);
}
int ILibThreadPool_GetThreadCount(ILibThreadPool pool)
{
	struct ILibThreadPool_ThreadState *ts = (struct ILibThreadPool_ThreadState*)pool;
	return(ts->NumThreads);
}
/*! \fn void ILibThreadPool_Destroy(ILibThreadPool pool)
	\brief Free the resources associated with an ILibThreadPool module
	\param pool Handle to free
*/
void ILibThreadPool_Destroy(ILibThreadPool pool)
{
	struct ILibThreadPool_ThreadState *ts = (struct ILibThreadPool_ThreadState*)pool;
	int ok = 0;
	int count = 0;

	ILibQueue_Lock(ts->WorkItemQueue);
	ts->Terminate=1;
	ok = count = ts->NumThreads;
	ILibQueue_UnLock(ts->WorkItemQueue);


	while(count!=0)
	{
		sem_post(&(ts->SyncHandle));
		--count;
	}

	if(ok!=0)
	{
		sem_wait(&(ts->AbortHandle));
	}

	sem_destroy(&(ts->SyncHandle));
	sem_destroy(&(ts->AbortHandle));
	ILibQueue_Destroy(ts->WorkItemQueue);

	free(pool);
}

/*! \fn void ILibThreadPool_AddThread(ILibThreadPool pool)
	\brief Gives ownership of the current thread to the pool.
	\par
	This method will not return until the ThreadPool is destroyed
	\param pool Handle to the ILibThreadPool module to add a thread to
*/
void ILibThreadPool_AddThread(ILibThreadPool pool)
{
	struct ILibThreadPool_ThreadState *ts = (struct ILibThreadPool_ThreadState*)pool;
	struct ILibThreadPool_WorkItem *wi = NULL;
	int abort = 0;
	int id;
	int ok=0;

	ILibQueue_Lock(ts->WorkItemQueue);
	++ts->NumThreads;
	id = ts->NumThreads;
	ILibQueue_UnLock(ts->WorkItemQueue);

	do
	{
		sem_wait(&(ts->SyncHandle));

		ILibQueue_Lock(ts->WorkItemQueue);
		wi = (struct ILibThreadPool_WorkItem*)ILibQueue_DeQueue(ts->WorkItemQueue);	
		abort = ts->Terminate;
		ILibQueue_UnLock(ts->WorkItemQueue);

		if(wi!=NULL && !abort)
		{
			wi->Callback(pool,wi->var);
			free(wi);
		}
		else if(wi!=NULL)
		{
			free(wi);
		}

		ILibQueue_Lock(ts->WorkItemQueue);
		abort = ts->Terminate;
		ILibQueue_UnLock(ts->WorkItemQueue);
		
	}while(!abort);

	ILibQueue_Lock(ts->WorkItemQueue);
	--ts->NumThreads;
	ok = ts->NumThreads;
	ILibQueue_UnLock(ts->WorkItemQueue);

	if(ok==0)
	{
		sem_post(&(ts->AbortHandle));
	}
}

/*! \fn void ILibThreadPool_QueueUserWorkItem(ILibThreadPool pool, void *var, ILibThreadPool_Handler callback)
	\brief Queues a new work item to the thread pool
	\param pool The ILibThreadPool handle to queue the work item to
	\param var State object to pass to the handler
	\param callback The handler to be called when an available thread is ready to process the work item
*/
void ILibThreadPool_QueueUserWorkItem(ILibThreadPool pool, void *var, ILibThreadPool_Handler callback)
{
	struct ILibThreadPool_WorkItem *wi = NULL;
	struct ILibThreadPool_ThreadState *ts = (struct ILibThreadPool_ThreadState*)pool;
	int NumThreads=0;

	wi = (struct ILibThreadPool_WorkItem*)malloc(sizeof(struct ILibThreadPool_WorkItem));
	memset(wi,0,sizeof(struct ILibThreadPool_WorkItem));
	wi->var = var;
	wi->Callback = callback;
	
	ILibQueue_Lock(ts->WorkItemQueue);
		
	if(ts->NumThreads!=0)
	{
		NumThreads = ts->NumThreads;
		ILibQueue_EnQueue(ts->WorkItemQueue,wi);
		sem_post(&(ts->SyncHandle));
	}
	ILibQueue_UnLock(ts->WorkItemQueue);

	if(NumThreads==0)
	{
		//
		// There are no threads in the Pool, so call this thing from here
		//
		wi->Callback(pool,wi->var);
		free(wi);
	}
}
