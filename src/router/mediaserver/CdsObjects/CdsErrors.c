/*
 * INTEL CONFIDENTIAL
 * Copyright (c) 2002 - 2006 Intel Corporation.  All rights reserved.
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
 * $Workfile: CdsErrors.c
 *
 *
 *
 */

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#ifndef _WIN32_WCE
#include <crtdbg.h>
#endif
#else
#include <stdlib.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include "CdsErrors.h"

const char *CDS_ErrorStrings[] = 
{
	"",
	CDS_EM_ACTION_FAILED,
	CDS_EM_OBJECT_ID_NO_EXIST,
	CDS_EM_NO_SUCH_CONTAINER
};

const int CDS_ErrorCodes[] = 
{
	0,
	CDS_EC_ACTION_FAILED,
	CDS_EC_OBJECT_ID_NO_EXIST,
	CDS_EC_NO_SUCH_CONTAINER
};
