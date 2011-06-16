/******************************************************************************
 *
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
 *
 *****************************************************************************/

/**
 ***************************************************************************
 * @file lac_sym_partial.h
 * 
 * @defgroup LacSymPartial  Partial Packets
 *
 * @ingroup LacSymCommon
 * 
 * Partial packet handling code
 * 
 * @lld_start
 *
 * <b>Partials In Flight</b>\n
 * The API states that for partial packets the client should not submit 
 * the next partial request until the callback for the current partial has
 * been called. We have chosen to enforce this rather than letting the user 
 * proceed where they would get an incorrect digest, cipher result. 
 * 
 * Maintain a SpinLock for partials in flight per session. Try and acquire this
 * SpinLock. If it cant be acquired return an error straight away to the client 
 * as there is already a partial in flight. There is no blocking in the data
 * path for this. 
 * 
 * By preventing any other partials from coming in while a partial is in flight
 * we can check and change the state of the session without having to lock 
 * round it (dont want to have to lock and block in the data path). The state
 * of the session indicates the previous packet type that a request was 
 * successfully completed for. The last packet type is only updated for partial
 * packets. This state determines the packet types that can be accepted. 
 * e.g a last partial will not be accepted unless the previous packet was a 
 * partial. By only allowing one partial packet to be in flight, there is no 
 * need to lock around the update of the previous packet type for the session.
 * 
 * The ECB Cipher mode, ciphers each block separately. No state is maintained
 * between blocks. There is no need to wait for the callback for previous 
 * partial in ECB mode as the result of the previous partial has no impact on 
 * it. The API and our implementation only allows 1 partial packet to be in 
 * flight per session, therefore a partial packet request for ECB mode must 
 * be fully completed (ie. callback called) before the next partial request 
 * can be issued.  
 *    
 * <b>Partial Ordering</b>\n
 * The ordering that the user submits partial packets will be checked.  
 * (we could have let the user proceed where they will get an incorrect 
 * digest/cipher result but chose against this).
 *
 *  -# Maintain the last packet type of a partial operation for the session. If 
 *     there have been no previous partials, we will accept only first partials
 *  -# The state must be set to partial before we will accept a final partial.
 *     i.e. a partial request must have already completed. 
 *
 * The last packet type is updated in the callback for partial packets as this
 * is the only place we can guarantee that a partial packet operation has been
 * completed. When a partial completes the state can be updated from FULL to 
 * PARTIAL. The SpinLock for partial packets in flight for the session can be 
 * unlocked at this point. On a final Partial request the last packet type is 
 * reset back to FULL. NOTE: This is not done at the same time as the check in
 * the perform as if an error occurs we would have to roll back the state 
 *
 * For Hash mode it is possible to interleave full and a single partial
 * packet stream in a session as the hash state buffer is updated for partial
 * packets. It is not touched by full packets. For cipher mode, as the client
 * manages the state, they can interleave full and a single partial packets.
 * For ARC4, the state is managed internally and the packet type will always
 * be set to partial internally.
 *  
 * @lld_end 
 *
 ***************************************************************************/

/***************************************************************************/

#ifndef LAC_SYM_PARTIAL_H
#define LAC_SYM_PARTIAL_H


#include "cpa.h"
#include "cpa_cy_sym.h"

/***************************************************************************/

/**
*******************************************************************************
 * @ingroup LacSymPartial
 *      check if partial packet request is valid for a session
 *
 * @description
 *      This function checks to see if there is a partial packet request in 
 *      flight and then if the partial state is correct
 *
 * @param[in]  packetType               Partial packet request
 * @param[in]  partialState             Partial state of session
 * 
 * @retval CPA_STATUS_SUCCESS           Normal Operation
 * @retval CPA_STATUS_INVALID_PARAM     Invalid Parameter
 *
 *****************************************************************************/
CpaStatus
LacSym_PartialPacketStateCheck(CpaCySymPacketType packetType,
                               CpaCySymPacketType partialState);


/**
*******************************************************************************
 * @ingroup LacSymPartial
 *      update the state of the partial packet in a session
 *
 * @description
 *      This function is called in callback operation. It updates the state
 *      of a partial packet in a session and indicates that there is no
 *      longer a partial packet in flight for the session 
 *
 * @param[in]  packetType           Partial packet request
 * @param[out] pPartialState        Pointer to partial state of session
 * 
 *****************************************************************************/
void
LacSym_PartialPacketStateUpdate(CpaCySymPacketType packetType,
                                CpaCySymPacketType *pPartialState);


#endif /* LAC_SYM_PARTIAL_H */
