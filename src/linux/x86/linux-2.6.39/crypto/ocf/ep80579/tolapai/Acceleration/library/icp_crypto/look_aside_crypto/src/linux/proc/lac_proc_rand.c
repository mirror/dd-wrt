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
 * @file lac_proc_rand.c random /proc functions
 *
 * @ingroup LacProc
 *
 * @description This file implements /proc filesystem functions for Random
 * number generation.
 *****************************************************************************/

/*
********************************************************************************
* Include public/global header files
********************************************************************************
*/

#include "lac_proc.h"
#include "cpa_cy_rand.h"


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

#define PROC_FILENAME "rand"

/*
********************************************************************************
* Define static function definitions
********************************************************************************
*/

static int
procfile_read(char *page, char **start, off_t off, int count,
                  int *eof, void *data)
{
    CpaStatus status = 0;
    int len = 0;
    CpaCyRandStats randStats = {0};
    
    status = cpaCyRandQueryStats(CPA_INSTANCE_HANDLE_SINGLE, &randStats);

    CHECK_STATUS_AND_OFFSET();

    /* Engine Info */
    len += snprintf(page+len, count-len,
       SEPARATOR
       BORDER " Random Stats                               "
       BORDER "\n"
       SEPARATOR);

    /* Random Info */    
    len += snprintf(page+len, count-len,
       BORDER " Random Requests:                %10u " BORDER "\n"
       BORDER " Random Completed:               %10u " BORDER "\n"
       BORDER " Random Requests  Errors:        %10u " BORDER "\n"
       BORDER " Random Completed Errors:        %10u " BORDER "\n"
       SEPARATOR,
       randStats.numRandNumRequests,
       randStats.numRandNumCompleted,
       randStats.numRandNumRequestErrors,
       randStats.numRandNumCompletedErrors);   

    len += snprintf(page+len, count-len,
       BORDER " Random Bit Requests:            %10u " BORDER "\n"
       BORDER " Random Bit Completed:           %10u " BORDER "\n"
       BORDER " Random Bit Requests  Errors:    %10u " BORDER "\n"
       BORDER " Random Bit Completed Errors:    %10u " BORDER "\n"
       SEPARATOR,
       randStats.numRandBitRequests,
       randStats.numRandBitCompleted,
       randStats.numRandBitRequestErrors,
       randStats.numRandBitCompletedErrors);

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

int lacProcRand_init()
{
    return lacProc_createFile(PROC_FILENAME, procfile_read);
}

void lacProcRand_cleanup()
{
    lacProc_deleteFile(PROC_FILENAME);
}



