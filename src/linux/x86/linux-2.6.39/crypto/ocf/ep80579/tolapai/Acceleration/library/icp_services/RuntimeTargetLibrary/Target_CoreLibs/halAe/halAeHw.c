/**
 **************************************************************************
 * @file halAeHw.c
 *
 * @description
 *      This file provides Implementation of Ucode AE Library
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
 *
 **************************************************************************/ 

#include "core_io.h"
#include "halAeApi.h"

void halAe_ExecuteCycles(unsigned int cycles);
void halAe_WaitUStoreAddrReady(unsigned char ae);
int waitNumSimCycles(unsigned char ae, 
                     unsigned int cycles, 
                     int chkInactive);
int getRelDataReg(unsigned char ae, unsigned char ctx, 
                  icp_RegType_T regType,
                  unsigned short regNum, unsigned int *data);
int getRelNNReg(unsigned char ae, 
                unsigned char ctx, 
                icp_RegType_T regType,
                unsigned short regNum,
                unsigned int *data);
int putRelDataReg(unsigned char ae, unsigned char ctx, 
                  icp_RegType_T regType,
                  unsigned short regNum, 
                  unsigned int data);
int putRelWrXfer(unsigned char ae, 
                 unsigned char ctx, 
                 icp_RegType_T regType,
                 unsigned short regNum, 
                 unsigned int data);
int putRelNN(unsigned char ae, 
             unsigned char ctx, 
             unsigned short nnNum,
             unsigned int value);
int getCtxIndrCsr(unsigned char ae, 
              unsigned char ctx, 
              unsigned int aeCsr, 
              unsigned int *csrVal);
int halAe_intrSupported(void);

int halAe_PutLM_Common(unsigned char ae, 
                       unsigned short lmAddr, 
                       unsigned int value);
int halAe_GetLM_Common(unsigned char ae, 
                       unsigned short lmAddr, 
                       unsigned int *value);
int halAe_PutSharedRam_Common(unsigned char ae, 
                              unsigned int addr, 
                              unsigned int value);
int halAe_GetSharedRam_Common(unsigned char ae, 
                              unsigned int addr, 
                              unsigned int *value);
int getRelDataReg_Common(unsigned char ae, unsigned char ctx, 
                         icp_RegType_T regType,
                         unsigned short regNum, unsigned int *data);
int getRelNNReg_Common(unsigned char ae, 
                       unsigned char ctx, 
                       icp_RegType_T regType,
                       unsigned short regNum,
                       unsigned int *data);
int putRelDataReg_Common(unsigned char ae, unsigned char ctx, 
                         icp_RegType_T regType,
                         unsigned short regNum, 
                         unsigned int data);
int putRelWrXfer_Common(unsigned char ae, 
                        unsigned char ctx, 
                        icp_RegType_T regType,
                        unsigned short regNum, 
                        unsigned int data);
int putRelNN_Common(unsigned char ae, 
                    unsigned char ctx, 
                    unsigned short nnNum,
                    unsigned int value);
int halAe_GetRelDataReg_Common(unsigned char ae, 
                               unsigned char ctx, 
                               icp_RegType_T regType,
                               unsigned short regNum, 
                               unsigned int *regData);
int halAe_PutRelDataReg_Common(unsigned char ae, 
                               unsigned char ctx, 
                               icp_RegType_T regType,
                               unsigned short regNum, 
                               unsigned int regData);
int halAe_GetAbsDataReg_Common(unsigned char ae, 
                               icp_RegType_T regType,
                               unsigned short absRegNum, 
                               unsigned int *regData);
int halAe_PutAbsDataReg_Common(unsigned char ae, 
                               icp_RegType_T regType,
                               unsigned short absRegNum, 
                               unsigned int regData);
int getCtxIndrCsr_Common(unsigned char ae, 
                         unsigned char ctx, 
                         unsigned int aeCsr, 
                         unsigned int *csrVal);

/*-----------------------------------------------------------------------------
   Function:    halAe_ExecuteCycles
   Description: Execute cycles
   Returns:     
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
void halAe_ExecuteCycles(unsigned int cycles)
{
}

/*-----------------------------------------------------------------------------
   Function:    halAe_WaitUStoreAddrReady
   Description: Wait until ustore address is propagated
   Returns:     
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
void halAe_WaitUStoreAddrReady(unsigned char ae)
{
    waitNumSimCycles(ae, 8, 0);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutLM
   Description: Write a long-word value to the AE's LM location specified by 
                the lmAddr word-address. It's unsafe to call this function while 
                the AE is enabled.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutLM(unsigned char ae, 
            unsigned short lmAddr, 
            unsigned int value)
{
    return halAe_PutLM_Common(ae, lmAddr, value);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetLM
   Description: Write a long-word value to the AE's LM location specified by
                the lmAddr word-address. 
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_AEACTIVE
   Uses:        
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetLM(unsigned char ae, 
            unsigned short lmAddr, 
            unsigned int *value)
{
    return halAe_GetLM_Common(ae, lmAddr, value);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutSharedRam
   Description: Write a long-word value to the SSU Share Ram location specified
                by the addr. It's unsafe to call this function while the AE 
                is enabled.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutSharedRam(unsigned char ae, 
                   unsigned int addr, 
                   unsigned int value)
{
    return halAe_PutSharedRam_Common(ae, addr, value);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetSharedRam
   Description: Read a long-word value from the SSU Share Ram location 
                specified by the addr. It's unsafe to call this function while 
                the AE is enabled.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetSharedRam(unsigned char ae, 
                   unsigned int addr, 
                   unsigned int *value)
{
    return halAe_GetSharedRam_Common(ae, addr, value);
}

/*-----------------------------------------------------------------------------
   Function:    getRelDataReg
   Description: Read a long-word from a GPR, read-xfer, and LM index registers.
                It's unsafe to called this function while the AE is enabled.
                Type is one of: ICP_GPA_REL, ICP_GPB_REL, ICP_SR_REL, 
                                ICP_SR_RD_REL, ICP_DR_REL, ICP_DR_RD_REL, 
                                ICP_LMEM0, ICP_LMEM1.
                Ctx = 0-7/0-6 (even).

   Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_BADLIB, HALAE_RESET
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
getRelDataReg(unsigned char ae, 
              unsigned char ctx, 
              icp_RegType_T regType,
              unsigned short regNum, 
              unsigned int *data)
{
    return getRelDataReg_Common(ae, ctx, regType, regNum, data);
}

/*-----------------------------------------------------------------------------
   Function:    getRelNNReg
   Description: Read a long-word from a next-neigh registers.
                It's unsafe to called this function while the AE is enabled.
                Type is ICP_NEIGH_REL.
                Ctx = 0-7/0-6 (even).

   Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_BADLIB, HALAE_RESET
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
getRelNNReg(unsigned char ae, 
            unsigned char ctx, 
            icp_RegType_T regType,
            unsigned short regNum, 
            unsigned int *data)
{
    return getRelNNReg_Common(ae, ctx, regType, regNum, data);
}

/*-----------------------------------------------------------------------------
   Function:    putRelDataReg
   Description: Write a long-word to a GPR or LM index register. This logic 
                cannot work for neighbor or transfer registers.
                The AE must be disabled prior to calling this function.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_ENABLED, 
                HALAE_FAIL, HALAE_RESET
                Type is one of: ICP_GPA_REL, ICP_GPB_REL, ICP_LMEM0, ICP_LMEM1.
                Ctx = 0-7/0, 2, 4, 6.
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
putRelDataReg(unsigned char ae, 
              unsigned char ctx, 
              icp_RegType_T regType, 
              unsigned short regNum, 
              unsigned int data)
{
    return putRelDataReg_Common(ae, ctx, regType, regNum, data);
}

/*-----------------------------------------------------------------------------
   Function:    putRelWrXfer
   Description: Write a long-word to a relative write-xfer reg.  This logic
                also supports neighbor registers.
                The AE must be disabled prior to calling this function.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG, HALAE_ENABLED, HALAE_FAIL,
                HALAE_RESET
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
putRelWrXfer(unsigned char ae, 
             unsigned char ctx, 
             icp_RegType_T regType,
             unsigned short regNum, 
             unsigned int data)
{
    return putRelWrXfer_Common(ae, ctx, regType, regNum, data);
}

/*-----------------------------------------------------------------------------
   Function:    putRelNN
   Description: Write a long-word value to the AE's NN register. The AE must be
                disabled prior to calling this function.
   Returns:     HALAE_SUCCESS, HALAE_BADLIB, HALAE_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
putRelNN(unsigned char ae, 
         unsigned char ctx, 
         unsigned short nnNum,
         unsigned int value)
{
    return putRelNN_Common(ae, ctx, nnNum, value);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetRelDataReg
   Description: Read long-words from the specified register type:
                ICP_GPA_REL, ICP_GPB_REL, ICP_DR_RD_REL, ICP_SR_RD_REL,
                ICP_DR_WR_REL, ICP_SR_WR_REL, ICP_NEIGH_REL.
                The AE must be disabled prior to calling this function.

   Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_ENABLED, HALAE_FAIL,
                HALAE_RESET, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetRelDataReg(unsigned char ae, 
                    unsigned char ctx, 
                    icp_RegType_T regType,
                    unsigned short regNum, 
                    unsigned int *regData)
{
    return halAe_GetRelDataReg_Common(ae, ctx, regType, regNum, regData);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutRelDataReg
   Description: Write long-words to the specified register type:
                ICP_GPA_REL, ICP_GPB_REL, ICP_DR_RD_REL, ICP_SR_RD_REL,
                ICP_DR_WR_REL, ICP_SR_WR_REL, ICP_NEIGH_REL
                The AE must be disabled prior to calling this function.

   Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_ENABLED, HALAE_FAIL,
                HALAE_RESET, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutRelDataReg(unsigned char ae, 
                    unsigned char ctx, 
                    icp_RegType_T regType,
                    unsigned short regNum, 
                    unsigned int regData)
{
    return halAe_PutRelDataReg_Common(ae, ctx, regType, regNum, regData);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_GetAbsDataReg
   Description: Read long-words from the specified register type:
                ICP_GPA_ABS, ICP_GPB_ABS, ICP_DR_RD_ABS, ICP_SR_RD_ABS,
                ICP_DR_WR_ABS, ICP_SR_WR_ABS, ICP_NEIGH_ABS
                The AE must be disabled prior to calling this function.

   Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_ENABLED, HALAE_FAIL,
                HALAE_RESET, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_GetAbsDataReg(unsigned char ae, 
                    icp_RegType_T regType,
                    unsigned short absRegNum, 
                    unsigned int *regData)
{
    return halAe_GetAbsDataReg_Common(ae, regType, absRegNum, regData);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_PutAbsDataReg
   Description: Write long-words to the specified register type:
                ICP_GPA_ABS, ICP_GPB_ABS, ICP_DR_RD_ABS, ICP_SR_RD_ABS,
                ICP_DR_WR_ABS, ICP_SR_WR_ABS, ICP_NEIGH_ABS
                The AE must be disabled prior to calling this function.

   Returns:     HALAE_SUCCESS, HALAE_BADARG, HALAE_ENABLED, HALAE_FAIL,
                HALAE_RESET, HALAE_AEACTIVE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_PutAbsDataReg(unsigned char ae, 
                    icp_RegType_T regType,
                    unsigned short absRegNum, 
                    unsigned int regData)
{
    return halAe_PutAbsDataReg_Common(ae, regType, absRegNum, regData);
}

/*-----------------------------------------------------------------------------
   Function:    getCtxIndrCsr
   Description: Read a context indirect-csr.
   Returns:     HALAE_SUCCESS, HALAE_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
getCtxIndrCsr(unsigned char ae, 
              unsigned char ctx, 
              unsigned int aeCsr, 
              unsigned int *csrVal)
{
    return getCtxIndrCsr_Common(ae, ctx, aeCsr, csrVal);
}

/*-----------------------------------------------------------------------------
   Function:    halAe_intrSupported
   Description: Return if interrupt is supported.
   Returns:     0, 1
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
halAe_intrSupported(void)
{
    return 1;
}
