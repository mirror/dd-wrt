/**
 **************************************************************************
 * @file dbgAeInfo.h
 *
 * @description
 *      This is the header file for AE debugger
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

#ifndef __DBGAEINFO_H
#define __DBGAEINFO_H

#define DBGAEINFO_BKPT_NOT_ALLOWED     0x00 /* breakpoint not allowed */
#define DBGAEINFO_BKPT_ALLOWED         0x01 /* breakpoint allowed */
#define DBGAEINFO_UNKNOWN              0x02 /* unknown debug information */
#define DBGAEINFO_SOFTBKPT             0x02 /* software breakpoint */
#define DBGAEINFO_CTX_ARB_KILL         0x08 /* context arb kill */
#define DBGAEINFO_BR_PAGE              0x10 /* branch page instrction */
#define DBGAEINFO_RTN_PAGE             0x20 /* return from page instruction */

#define DBGAEINFO_SBRK_HALT     0           /* soft breakpoint halt */
#define DBGAEINFO_SBRK_KILL     1           /* soft breakpoint kill */
#define DBGAEINFO_SBRK_BR_PAGE  2           /* soft breakpoint branch page */
#define DBGAEINFO_SBRK_RTN_PAGE 3           /* soft breakpoint return from page */

#define MAX_NUM_OF_AE 0x18

typedef struct{
    unsigned int            uAddr;           /**< Virtual address of this uword */
    unsigned int            brkPtAllowed;    /**< 0=not allowed, 1=allowed, 2=soft breakpoint */
    unsigned char           ctxArbKill;      /**< whether is an ctx_arb[kill] instruction */
    unsigned short          deferCount;      /**< this instruction's defer count */
    int                     brAddr;          /**< branch address; -1=none */
    int                     regAddr;         /**< register address; -1=none */
    unsigned short          regType;         /**< register type */
    unsigned short          reserved1;       /**< reserved for future use */
    char                   *text;            /**< Pointer to associated text */
}dbgAeInfo_SrcLine_T;

typedef struct {
    char                   *name;             /**< Pointer to name of label */
    unsigned int            addr;             /**< Virtual address of label */
} dbgAeInfo_Label_T;

typedef struct{
    unsigned int            aeAssigned;       /**< bit-flags of the assigned AEs */
    unsigned int            ctxAssigned;      /**< bit-flags of the assigned Ctx */
    unsigned short          numLabels;        /**< num of labels in array below */
    unsigned short          numSrcLine;       /**< num lines pointed to by srcLineInfo */
    dbgAeInfo_SrcLine_T     *srcLine;         /**< ptr to srcLine */
    dbgAeInfo_Label_T       *label;           /**< ptr to array of labels */
}dbgAeInfo_SrcImage_T;

typedef struct{
    unsigned int            numImage;        /**< num srcImageInfo ptr in array srcImageInfo */
    dbgAeInfo_SrcImage_T    srcImage[MAX_NUM_OF_AE]; /**< image information */
}dbgAeInfo_Image_T;

#endif            /* __DBGAEINFO_H */

