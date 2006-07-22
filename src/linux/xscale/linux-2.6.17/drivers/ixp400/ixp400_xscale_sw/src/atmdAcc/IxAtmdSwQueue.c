
/**
* @file IxAtmdSwQueue.c
*
 * @author Intel Corporation
* @date 17 March 2002
*
* @brief IxAtmdAcc misc functions
*
* This file contains the functions required to initialialize
* the sw queues. Sw queue size needs to be a power of 2 and
* this function in this file rounds up the requested queue size to
* the nearest larger power of 2.
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

/*
* Put the user defined include files required.
*/

#include "IxAtmdAssert_p.h"
#include "IxAtmdDefines_p.h"
#include "IxAtmdSwQueue_p.h"

/*------------------------------------------------------------------------
*  Find the nearest power of two
*/
unsigned int
ixAtmdAccUtilNearestPowerOfTwoGet(unsigned int val)
{
    unsigned int count = 0;
    unsigned int adjustedVal = val;
    BOOL powerOf2 = TRUE;

    /* check if a power of 2 and count the number of bits needed */
    while (adjustedVal > 1)
    {
        if (adjustedVal & 1)
        {
            /* the input number is not a power of 2 */
            powerOf2 = FALSE;
        };
        count++;
        adjustedVal >>= 1;
    } /* end of while(adjustedVal) */

    /* if the input number is not a power of 2, the resulting number has
     * to be greater than the input number
     */
    if (!powerOf2)
    {
        count++;
    }

    /* compute the power of 2 */
    adjustedVal = 1 << count;

    /* ensure the result is consistent with the input values */
    IX_ATMDACC_ENSURE((adjustedVal >= val) && (adjustedVal < (val * 2)),
        "IxAtmdAccUtilNearestPowerOfTwo failed");

    return adjustedVal;
}

/*------------------------------------------------------------------------
*  Convert the requested queue size to a sw queue size
*/
unsigned int
ixAtmdAccUtilQueueSizeConvert(unsigned int swQueueSize)
{
    /* check input parameter */
    IX_ATMDACC_ENSURE(swQueueSize <= IX_ATMDACC_SWQ_MAX_ENTRIES, "Queue Size too large");

    /* convert the queue size */
    swQueueSize = ixAtmdAccUtilNearestPowerOfTwoGet(swQueueSize);

    /* check output value */
    IX_ATMDACC_ENSURE(swQueueSize <= IX_ATMDACC_SWQ_MAX_ENTRIES, "Queue Size too large");

    return swQueueSize;
}


