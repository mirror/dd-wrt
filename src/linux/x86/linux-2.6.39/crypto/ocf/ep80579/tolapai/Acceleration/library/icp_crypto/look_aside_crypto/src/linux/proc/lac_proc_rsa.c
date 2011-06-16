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
 * @file lac_proc_rsa.c RSA /proc functions
 *
 * @ingroup LacProc
 *
 * @description This file implements /proc filesystem functions for RSA.
 *****************************************************************************/

/*
********************************************************************************
* Include public/global header files
********************************************************************************
*/

#include "lac_proc.h"
#include "cpa_cy_rsa.h"


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

#define PROC_FILENAME "rsa"

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
    CpaCyRsaStats rsaStats = {0};
    
    status = cpaCyRsaQueryStats(CPA_INSTANCE_HANDLE_SINGLE, &rsaStats);
    
    CHECK_STATUS_AND_OFFSET();

    /* Engine Info */
    len += snprintf(page+len, count-len,
        SEPARATOR           
        BORDER " RSA Stats                                  "
        BORDER "\n"
        SEPARATOR);

    /*Perform Info*/
    len += snprintf(page+len, count-len,
        BORDER " RSA Key Gen Requests:           %10u " BORDER "\n"
        BORDER " RSA Key Gen Request Errors      %10u " BORDER "\n"
        BORDER " RSA Key Gen Completed:          %10u " BORDER "\n"
        BORDER " RSA Key Gen Completed Errors:   %10u " BORDER "\n"
        SEPARATOR,
        rsaStats.numRsaKeyGenRequests,
        rsaStats.numRsaKeyGenRequestErrors,
        rsaStats.numRsaKeyGenCompleted,
        rsaStats.numRsaKeyGenCompletedErrors);        
    
    /*Perform Info*/
    len += snprintf(page+len, count-len,
        BORDER " RSA Encrypt Requests:           %10u " BORDER "\n"
        BORDER " RSA Encrypt Request Errors:     %10u " BORDER "\n"
        BORDER " RSA Encrypt Completed:          %10u " BORDER "\n"
        BORDER " RSA Encrypt Completed Errors:   %10u " BORDER "\n"
        SEPARATOR,
        rsaStats.numRsaEncryptRequests,
        rsaStats.numRsaEncryptRequestErrors,
        rsaStats.numRsaEncryptCompleted,
        rsaStats.numRsaEncryptCompletedErrors);

    /*Perform Info*/
    len += snprintf(page+len, count-len,
        BORDER " RSA Decrypt Requests:           %10u " BORDER "\n"
        BORDER " RSA Decrypt Request Errors:     %10u " BORDER "\n"
        BORDER " RSA Decrypt Completed:          %10u " BORDER "\n"
        BORDER " RSA Decrypt Completed Errors:   %10u " BORDER "\n"
        SEPARATOR,
        rsaStats.numRsaDecryptRequests,
        rsaStats.numRsaDecryptRequestErrors,
        rsaStats.numRsaDecryptCompleted,
        rsaStats.numRsaDecryptCompletedErrors);

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

int lacProcRsa_init()
{
    return lacProc_createFile(PROC_FILENAME, procfile_read);
}

void lacProcRsa_cleanup()
{
    lacProc_deleteFile(PROC_FILENAME);
}


