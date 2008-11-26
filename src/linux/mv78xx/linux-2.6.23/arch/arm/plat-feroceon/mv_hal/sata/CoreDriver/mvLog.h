/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer.

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    *   Neither the name of Marvell nor the names of its contributors may be
        used to endorse or promote products derived from this software without
        specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/
/*******************************************************************************
* mvLog.h - Header File for CORE driver logger
*
* DESCRIPTION:
*       None.
*
* DEPENDENCIES:
*
*******************************************************************************/
#ifndef __INCmvLogh
#define __INCmvLogh

#ifdef __cplusplus
extern "C" /*{*/
#endif /* __cplusplus */

#include "mvOsS.h"

/*-------------H file-----------------------------*/
#define MV_DEBUG_FATAL_ERROR                    0x01
#define MV_DEBUG_ERROR                          0x02
#define MV_DEBUG_INIT                           0x04
#define MV_DEBUG_INTERRUPTS                     0x08
#define MV_DEBUG_SATA_LINK                      0x10
#define MV_DEBUG_UDMA_COMMAND                   0x20
#define MV_DEBUG_NON_UDMA_COMMAND               0x40
#define MV_DEBUG_PM                             0x80
#define MV_DEBUG                                0x100
#define MV_DEBUG_INFO                           0x200

#define MV_DEBUG_ENABLE_ALL			0x3FF

#define MV_MAX_LOG_MODULES         16
#define MV_MAX_MESSAGE_TYPE        10
#define MV_RAW_MSG_ID              0xF

typedef struct
{
    MV_BOOLEAN      used;
    MV_U32          filterMask;
    const char      *name;
    char            *filters;
} MV_LOG_FILTER_HEADER;


#if defined (MV_LOG_DEBUG) || defined (MV_LOG_ERROR)
    #define MV_LOGGER       1
    #if defined (WIN32)
ULONG
_cdecl
DbgPrint(
        PCH Format,
        ...
        );
        #define MV_LOG_PRINT    DbgPrint
    #elif defined (LINUX)
        #define MV_LOG_PRINT    printk
    #elif defined __DOS__
        #define MV_LOG_PRINT    printf
    #elif defined __UBOOT__
        #define MV_LOG_PRINT    printf
    #else
        #define MV_LOG_PRINT	printf
    #endif


MV_BOOLEAN mvLogRegisterModule(MV_U8 moduleId, MV_U32 filterMask, const char* name);
MV_BOOLEAN mvLogSetModuleFilter(MV_U8 moduleId, MV_U32 filterMask);
MV_U32 mvLogGetModuleFilter(MV_U8 moduleId);
void mvLogMsg(MV_U8 moduleId, MV_U32 type, const char* format, ...);

#else /*defined (MV_LOG_DEBUG) || defined (MV_LOG_ERROR)*/

    #undef MV_LOGGER

    #if defined (WIN32)
        #define MV_LOG_PRINT
        #define mvLogRegisterModule
        #define mvLogGetModuleFilter
        #define mvLogRegisterAllModules
        #define mvLogMsg

    #elif defined (MV_LINUX)
        #define MV_LOG_PRINT(x...)
        #define mvLogRegisterModule(x...)
        #define mvLogSetModuleFilter(x...)
        #define mvLogGetModuleFilter(x...)
        #define mvLogRegisterAllModules(x...)
        #define mvLogMsg(x...)

    #elif defined (MV_UBOOT)
        #define MV_LOG_PRINT(x...)
        #define mvLogRegisterModule(x...)
        #define mvLogSetModuleFilter(x...)
        #define mvLogGetModuleFilter(x...)
        #define mvLogRegisterAllModules(x...)
        #define mvLogMsg(x...)

    #elif defined (__DOS__)
        #define MV_LOG_PRINT()
        #define mvLogRegisterModule()
        #define mvLogSetModuleFilter()
        #define mvLogGetModuleFilter()
        #define mvLogRegisterAllModules()
        #define mvLogMsg()

    #else
        #define MV_LOG_PRINT
        #define mvLogRegisterModule
        #define mvLogSetModuleFilter
        #define mvLogGetModuleFilter
        #define mvLogRegisterAllModules
        #define mvLogMsg
    #endif

#endif /*!defined (MV_LOG_DEBUG) && !defined (MV_LOG_ERROR)*/

#ifdef __cplusplus

/*}*/
#endif /* __cplusplus */

#endif

