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
 * $Workfile: ILibThreadPool.h
 * $Revision: #1.0.2384.30305
 * $Author:   Intel Corporation, Intel Device Builder
 * $Date:     Thursday, July 13, 2006
 *
 *
 *
 */

#ifndef __ILIBTHREADPOOL__
#define __ILIBTHREADPOOL__

/*! \file ILibThreadPool.h 
	\brief MicroStack APIs for platform independent threadpooling capabilities
*/

/*! \defgroup ILibThreadPool ILibThreadPool Module
	\{
*/

/*! \typedef ILibThreadPool
	\brief Handle to an ILibThreadPool module
*/
typedef void* ILibThreadPool;
/*! \typedef ILibThreadPool_Handler
	\brief Handler for a thread pool work item
	\param sender The ILibThreadPool handle
	\param var State object
*/
typedef void(*ILibThreadPool_Handler)(ILibThreadPool sender, void *var);

ILibThreadPool ILibThreadPool_Create();
void ILibThreadPool_AddThread(ILibThreadPool pool);
void ILibThreadPool_QueueUserWorkItem(ILibThreadPool pool, void *var, ILibThreadPool_Handler callback);
void ILibThreadPool_Destroy(ILibThreadPool pool);
int ILibThreadPool_GetThreadCount(ILibThreadPool pool);


/*! \} */
#endif
