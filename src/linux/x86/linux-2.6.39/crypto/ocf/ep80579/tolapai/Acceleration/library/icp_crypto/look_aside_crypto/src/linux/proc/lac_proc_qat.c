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
 *****************************************************************************
 *
 * @file lac_proc_qat.c qat /proc functions
 *
 * @ingroup LacProc
 *
 * @description This file implements /proc filesystem functions for qat.
 *****************************************************************************/

/*
********************************************************************************
* Include public/global header files
********************************************************************************
*/

#include "lac_proc.h"
#include "qat_comms.h"


/*
********************************************************************************
* Include private header files
********************************************************************************
*/

#include "lac_proc_common_p.h"

/*
********************************************************************************
* Static Variables
********************************************************************************
*/

#define PROC_FILENAME "qat"

/*
********************************************************************************
* Define static function definitions
********************************************************************************
*/

static int
procfile_read(char *page, char **start, off_t off, int count,
                  int *eof, void *data)
{
    int len = 0;
    
    Cpa32U lacNumSent = 0;
    Cpa32U lacNumReceived = 0;   
    Cpa32U lacNumRetrys = 0;
    Cpa32U pkeNumSent = 0;
    Cpa32U pkeNumReceived = 0;
    Cpa32U pkeNumRetrys = 0;
    Cpa32U fwNumSent = 0;
    Cpa32U fwNumReceived = 0;
        
    /* We output data in one part, so we reject request for further data */
    if(off > 0)
    {
        *eof = 1;
        return 0;
    }
    
    len += snprintf(page+len, count-len,
            SEPARATOR
            BORDER
            " Qat Comms LA Stats                         "
            BORDER"\n"
            SEPARATOR);

    QatComms_MsgCountGet(ICP_ARCH_IF_REQ_QAT_FW_LA,
                     &lacNumSent, &lacNumReceived, &lacNumRetrys);

    len += snprintf(page+len, count-len,
            BORDER " Requests:                       %10u " BORDER "\n"
            BORDER " Responses:                      %10u " BORDER "\n"
            BORDER " Retrys:                         %10u " BORDER "\n"
            SEPARATOR,
            lacNumSent,
            lacNumReceived, 
            lacNumRetrys);

    len += snprintf(page+len, count-len,
            SEPARATOR
            BORDER
            " Qat Comms PKE Stats                        "
            BORDER"\n"
            SEPARATOR);

    QatComms_MsgCountGet(ICP_ARCH_IF_REQ_QAT_FW_PKE,
                     &pkeNumSent, &pkeNumReceived, &pkeNumRetrys);

    len += snprintf(page+len, count-len,
            BORDER " Requests:                       %10u " BORDER "\n"
            BORDER " Responses:                      %10u " BORDER "\n"
            BORDER " Retrys:                         %10u " BORDER "\n"
            SEPARATOR,
            pkeNumSent,
            pkeNumReceived,
            pkeNumRetrys);
    
    len += snprintf(page+len, count-len,
            SEPARATOR
            BORDER
            " Qat Firmware                               "
            BORDER"\n"
            SEPARATOR);
    
    Qatal_FWCountGet( &fwNumSent, &fwNumReceived);

    len += snprintf(page+len, count-len,
            BORDER " Firmware Requests:              %10u " BORDER "\n"
            BORDER " Firmware Responses:             %10u " BORDER "\n"
            SEPARATOR,
            fwNumReceived,
            fwNumSent);
    
    SET_EOF();
    
    return len;
}


/*
********************************************************************************
* Global Variables
********************************************************************************
*/

/*
********************************************************************************
* Define public/global function definitions
********************************************************************************
*/

int lacProcQat_init()
{
    return lacProc_createFile(PROC_FILENAME, procfile_read);
}

void lacProcQat_cleanup()
{
    lacProc_deleteFile(PROC_FILENAME);
}

