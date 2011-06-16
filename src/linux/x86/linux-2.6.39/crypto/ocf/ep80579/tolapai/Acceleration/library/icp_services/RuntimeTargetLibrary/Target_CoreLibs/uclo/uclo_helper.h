/**
 **************************************************************************
 * @file uclo_helper.h
 *
 * @description
 *      This file provides helper functions for Ucode Object File Loader
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

#ifndef __UCLO_HELPER_H__
#define __UCLO_HELPER_H__

#include "uclo_platform.h"
#include "icptype.h"

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

#ifndef WORD_BIT
#define WORD_BIT ((unsigned int)(CHAR_BIT * sizeof(unsigned int)))
#endif

#define MASK (WORD_BIT-1)
#define IS_BIT_SET(wrd, bit) ((wrd) & (1 << ((bit) & MASK)))
#define isBitSet(wrd, bit) (IS_BIT_SET(wrd, bit))

#define CRC_POLY    0x1021              /* CRC-CCITT, ADCCP, SDLC/HDLC */

#define CRC_WIDTH 16                    /* CRC width */
#define CRC_BITMASK(X) (1L << (X))      /* CRC bit mask*/
#define CRC_WIDTHMASK(width) ((((1L<<(width-1))-1L)<<1)|1L) /* CRC width mask*/

#ifndef TRUE
#define TRUE 1         /* logical true */
#endif
#ifndef FALSE
#define FALSE 0        /* logical false */ 
#endif

#ifdef __cplusplus
extern "C"{
#endif

unsigned int UcLo_strChecksum(char *pChar, 
                                       int numChar);
uint64 UcLo_setField64(uint64 word, 
                       int startBit, 
                       int fieldLen, 
                       uint64 value);
int UcLo_parseNum(char *pStr, int *pNum);
char *UcLo_stripLeadBlanks(char *pStr);
int UcLo_strcmp(const char *pStr1, 
                const char *pStr2, 
                int sensitive);
char *UcLo_strtoken(char *pStr, 
                    char *pDlim, 
                    char **savPtr);

#ifdef __cplusplus
}
#endif

#endif /* __UCLO_HELPER_H__ */
