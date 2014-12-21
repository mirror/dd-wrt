
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
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
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


