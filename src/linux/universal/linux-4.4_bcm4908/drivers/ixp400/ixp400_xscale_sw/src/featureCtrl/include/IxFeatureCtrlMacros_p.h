/**
 * @file IxFeatureCtrlMacros_p.h
 *
 * @author Intel Corporation
 * @date 10 March 2006
 *
 * @brief This file contains the macros for the IxFeatureCtrl component.
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

/**
 * @defgroup IxFeatureCtrlMacros_p IxFeatureCtrlMacros_p
 *
 * @brief Macros for the IxFeatureCtrl component.
 * 
 * @{
 */

#ifndef IXFEATURECTRLMACROS_P_H
#define IXFEATURECTRLMACROS_P_H

/*
 * Put the user defined include files required.
 */
#if (CPU != XSCALE)
/* To support IxFeatureCtrl unit tests... */
#include <stdio.h>
#include "test/IxFeatureCtrlTest.h"
#else   
#include "IxOsal.h"
#endif

/* Macros to be used for IxFeatureCtrl unit tests - no registers exist in the simulator */
#if (CPU != XSCALE)

#define IX_FEATURE_CTRL_READ(address, result) \
do { \
 (result) = ixFeatureCtrlTestRegRead(address); \
} while (0)

#define IX_FEATURE_CTRL_WRITE(address, result) \
do { \
ixFeatureCtrlTestRegWrite (address, result); \
} while (0)

#else

/* Macro to read from the Feature Control Register */
#define IX_FEATURE_CTRL_READ(address, result) \
do { \
 (result) = IX_OSAL_READ_LONG((UINT32 *)address); \
} while (0)

/* Macro to write to the Feature Control Register */
#define IX_FEATURE_CTRL_WRITE(address, value) \
do { \
IX_OSAL_WRITE_LONG((UINT32 *)address, (value)); \
} while (0)
#endif
#endif
