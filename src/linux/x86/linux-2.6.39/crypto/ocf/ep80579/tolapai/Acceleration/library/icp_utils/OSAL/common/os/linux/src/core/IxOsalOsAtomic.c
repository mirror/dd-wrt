/**
 * @file IxOsalOsAtomic.c (linux)
 *
 * @brief OS-specific Atomic API's implementation.
 * 
 * 
 * @par
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 */
#include "IxOsal.h"
#include <asm/atomic.h>



/**
 * @ingroup IxOsal
 *
 * @brief Atomically read the value of atomic variable
 *
 * @param  atomicVar (in)   - atomic variable
 *
 * Atomically reads the value of atomicVar to the outValue 
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return  atomicVar value
 */

PUBLIC UINT32 
ixOsalAtomicGet(IxOsalAtomic *atomicVar)
{
    return (( UINT32 )atomic_read( atomicVar ));
}

/**
 * @ingroup IxOsal
 *
 * @brief Atomically set the value of atomic variable
 *
 * @param  inValue (in)   -  atomic variable to be set equal to inValue
 *
 * @param  atomicVar (out)   - atomic variable
 *
 * Atomically sets the value of IxOsalAtomicVar to the value given
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return none
 */

PUBLIC void 
ixOsalAtomicSet(UINT32 inValue, IxOsalAtomic *atomicVar)
{
    atomic_set(atomicVar,inValue);
}

/**
 * @ingroup IxOsal
 *
 * @brief add the value to atomic variable
 *
 * @param  inValue (in)   -  value to be added to the atomic variable
 *
 * @param  atomicVar (in & out)   - atomic variable
 *
 * Atomically adds the value of inValue to the IxOsalAtomicVar 
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return none
 */

PUBLIC void 
ixOsalAtomicAdd(UINT32 inValue, IxOsalAtomic *atomicVar)
{
    atomic_add((int)inValue, atomicVar);
}

/**
 * @ingroup IxOsal
 *
 * @brief subtract the value from atomic variable
 *
 * @param  inValue (in)   -  atomic variable value to be subtracted by value
 *
 * @param  atomicVar (in & out)   - atomic variable
 *
 * Atomically subtracts the value of IxOsalAtomicVar by inValue
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return none
 */

PUBLIC void 
ixOsalAtomicSub(UINT32 inValue, IxOsalAtomic *atomicVar)
{
    atomic_sub((int)inValue, atomicVar);
}

/**
 * @ingroup IxOsal
 *
 * @brief subtract the value from atomic variable and test result
 *
 * @param  inValue (in)   -  value to be subtracted from the atomic variable
 *
 * @param  atomicVar (in & out)   - atomic variable
 *
 * Atomically subtracts the IxOsalAtomicVar value by inValue and
 * test the result.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return TRUE if the result is zero or FALSE for other cases.
 */

PUBLIC IX_STATUS
ixOsalAtomicSubAndTest(UINT32 inValue, IxOsalAtomic *atomicVar)
{
    return (IX_STATUS)(atomic_sub_and_test((int)inValue, atomicVar));
}

/**
 * @ingroup IxOsal
 *
 * @brief increment value of atomic variable by 1
 *
 * @param  atomicVar (in & out)   - atomic variable
 *
 * Atomically increments the value of IxOsalAtomicVar by 1.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return none
 */

PUBLIC void
ixOsalAtomicInc(IxOsalAtomic *atomicVar)
{
    atomic_inc(atomicVar);
}

/**
 * @ingroup IxOsal
 *
 * @brief decrement value of atomic variable by 1 
 *
 * @param  atomicVar (out)   - atomic variable
 *
 * Atomically decrements the value of IxOsalAtomicVar by 1.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return none
 */

PUBLIC void 
ixOsalAtomicDec(IxOsalAtomic *atomicVar)
{
    atomic_dec(atomicVar);
}

/**
 * @ingroup IxOsal
 *
 * @brief decrement atomic variable value by 1 and test result
 *
 * @param  atomicVar (in & out)   - atomic variable
 *
 * Atomically decrements the value of IxOsalAtomicVar by 1 and test 
 * result for zero.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return TRUE if the result is zero or FALSE otherwise
 */

PUBLIC IX_STATUS
ixOsalAtomicDecAndTest(IxOsalAtomic *atomicVar)
{
    return (IX_STATUS)(atomic_dec_and_test(atomicVar));
}

/**
 * @ingroup IxOsal
 *
 * @brief increment atomic variable by 1 and test result
 *
 * @param  atomicVar (in & out)   - atomic variable
 *
 * Atomically increments the value of IxOsalAtomicVar by 1 and test 
 * result for zero.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return TRUE if the result is zero or FALSE otherwise
 */

PUBLIC IX_STATUS 
ixOsalAtomicIncAndTest(IxOsalAtomic *atomicVar)
{
    return (IX_STATUS)(atomic_inc_and_test(atomicVar));
}

