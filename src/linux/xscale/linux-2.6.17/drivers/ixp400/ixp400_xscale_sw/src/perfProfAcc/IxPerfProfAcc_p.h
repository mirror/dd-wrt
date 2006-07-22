/**
 * @file IxPerfProfAcc_p.h
 *
 * @date 22 May 2003
 *
 * @brief Private header file containing lock and unlock function prototypes
 *
 * Design Notes:
 *    
 * 
 * @par
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
 */

#ifndef IXPERFPROFACC_P_H
#define IXPERFPROFACC_P_H

#include "IxPerfProfAcc.h"


/**
 * ixPerfProfAccLock (void)
 *
 * This will determine if any other utility is running; if not, this function 
 * will get the lock so that no other utilities can be called.  The function
 * returns :
 *	- IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS if another utility is
 *	  already running
 *  - IX_PERFPROF_ACC_STATUS_SUCCESS if no other utility is running and the lock
 *    is obtained	
 *
 */
IxPerfProfAccStatus 
ixPerfProfAccLock (void);

/**
 * ixPerfProfAccUnlock (void)
 *
 * This will return the lock so that another utility may begin.
 *
 */
void 
ixPerfProfAccUnlock (void);

#endif /*#ifndef IXPERFPROFACC_P_H*/


