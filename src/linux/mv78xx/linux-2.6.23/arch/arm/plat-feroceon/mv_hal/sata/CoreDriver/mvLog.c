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
* mvLog - C File for implementation of the core driver logger
*
* DESCRIPTION:
*       None.
*
* DEPENDENCIES:
*
*******************************************************************************/
#include "mvOsS.h"


#if defined (MV_LOG_DEBUG) || defined (MV_LOG_ERROR)

const char* mvLogMsgType[MV_MAX_MESSAGE_TYPE] = {
    " (FATAL_ERROR) ",
    " (ERROR) ",
    " (DEBUG INIT) ",
    " (DEBUG INTERRUPTS) ",
    " (DEBUG SATA LINK) ",
    " (DEBUG UDMA COMMAND) ",
    " (DEBUG NON UDMA COMMAND) ",
    " (DEBUG PM) ",
    " (DEBUG) ",
    " (INFO) ",
};

static MV_LOG_FILTER_HEADER mvLogInstance[MV_MAX_LOG_MODULES] =
{
    {MV_FALSE, 0, NULL},
    {MV_FALSE, 0, NULL},
    {MV_FALSE, 0, NULL},
    {MV_FALSE, 0, NULL},
    {MV_FALSE, 0, NULL},
    {MV_FALSE, 0, NULL},
    {MV_FALSE, 0, NULL},
    {MV_FALSE, 0, NULL},
};

static char szMessageBuffer[1024];


MV_BOOLEAN mvLogRegisterModule(MV_U8 moduleId, MV_U32 filterMask, const char* name)
{
    if (moduleId >= MV_MAX_LOG_MODULES)
    {
        return MV_FALSE;
    }
    if (mvLogInstance[moduleId].used == MV_TRUE)
    {
        return MV_FALSE;
    }
    if (name == NULL)
    {
        return MV_FALSE;
    }
    mvLogInstance[moduleId].filterMask = filterMask;
    mvLogInstance[moduleId].name = name;
    mvLogInstance[moduleId].used = MV_TRUE;
    return MV_TRUE;
}

MV_BOOLEAN mvLogSetModuleFilter(MV_U8 moduleId, MV_U32 filterMask)
{
    if (moduleId >= MV_MAX_LOG_MODULES)
    {
        return MV_FALSE;
    }
    if (mvLogInstance[moduleId].used == MV_FALSE)
    {
        return MV_FALSE;
    }
    mvLogInstance[moduleId].filterMask = filterMask;
    return MV_TRUE;
}


MV_U32 mvLogGetModuleFilter(MV_U8 moduleId)
{
    if (moduleId >= MV_MAX_LOG_MODULES)
    {
        return 0;
    }
    if (mvLogInstance[moduleId].used == MV_FALSE)
    {
        return 0;
    }
    return mvLogInstance[moduleId].filterMask;
}

void mvLogMsg(MV_U8 moduleId, MV_U32 type, const char* format, ...)
{
    int len;
    va_list args;

    if (moduleId >= MV_MAX_LOG_MODULES)
    {
        return;
    }
    if ((moduleId != MV_RAW_MSG_ID) &&
        ((mvLogInstance[moduleId].used == MV_FALSE) ||
         ((mvLogInstance[moduleId].filterMask & type) == 0)))
    {
        return;
    }
    if ((moduleId != MV_RAW_MSG_ID) && (type & 0x1ff))
    {
        MV_U8 msgType = 0;
        /* find least significant 1*/
        while (msgType < MV_MAX_MESSAGE_TYPE)
        {
            if (type & ( 1 << msgType))
            {
                break;
            }
            msgType++;
        }
        len = sprintf(szMessageBuffer, "%s%s",
                      mvLogInstance[moduleId].name, mvLogMsgType[msgType]);
    }
    else
    {
        len = 0;
    }
    va_start(args, format);
    vsprintf(&szMessageBuffer[len], format, args);
    va_end(args);
    MV_LOG_PRINT("%s", szMessageBuffer);
}
#endif
