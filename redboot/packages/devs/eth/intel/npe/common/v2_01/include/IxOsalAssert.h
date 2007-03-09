/*
 * @file        IxOsalAssert.h 
 * @author 	Intel Corporation
 * @date        25-08-2004
 *
 * @brief       description goes here
 *
 * @par
 * IXP400 SW Release version 1.5
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2004 Intel Corporation All Rights Reserved.
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

#ifndef IX_OSAL_ASSERT_H
#define IX_OSAL_ASSERT_H

/*
 * Put the system defined include files required
 * @par
 * <TAGGED>
 */

#include "IxOsalOsAssert.h"

/**
 * @brief Assert macro, assert the condition is true. This
 *        will not be compiled out.
 *        N.B. will result in a system crash if it is false.
 */
#define IX_OSAL_ASSERT(c) IX_OSAL_OS_ASSERT(c)


/**
 * @brief Ensure macro, ensure the condition is true.
 *        This will be conditionally compiled out and
 *        may be used for test purposes.
 */
#ifdef IX_OSAL_ENSURE_ON
#define IX_OSAL_ENSURE(c, str) do { \
if (!(c)) ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT, str, \
0, 0, 0, 0, 0, 0); } while (0)

#else
#define IX_OSAL_ENSURE(c, str)
#endif


#endif /* IX_OSAL_ASSERT_H */
