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
 *******************************************************************************
 * @file lac_proc_common_p.h
 * 
 * @ingroup LacProc
 * 
 * Constants and helper macros for LAC proc stats components
 *  
 ******************************************************************************/

#ifndef LAC_PROC_COMMON_P_H
#define LAC_PROC_COMMON_P_H

/* 
********************************************************************************
* Include public/global header files 
********************************************************************************
*/ 

#include <linux/proc_fs.h>

/* 
********************************************************************************
* Include private header files 
********************************************************************************
*/

/*
********************************************************************************
* Helper Macros and settings
********************************************************************************
*/

#define SEPARATOR "+--------------------------------------------+\n"
/**< @ingroup LacProc
 * separator used for printing stats in the /proc files */

#define BORDER "|"
/**< @ingroup LacProc
 * border used for printing stats in the /proc files */


#define CHECK_STATUS_AND_OFFSET()                                              \
    do {                                                                       \
        if(CPA_STATUS_SUCCESS != status)                                       \
        {                                                                      \
            return 0;                                                          \
        }                                                                      \
                                                                               \
        /* We output data in one part, so we reject request for further data */\
        if(off > 0)                                                            \
        {                                                                      \
            *eof = 1;                                                          \
            return 0;                                                          \
        }                                                                      \
    } while(0)
/**< @ingroup LacProc
 * Macro called at the beginning of every proc_read, it checks for the
 * correctness of status and verify the page offset is not set */



#define SET_EOF()                                                              \
    do {                                                                       \
        if(count - len > 0)                                                    \
        {                                                                      \
            *eof = 1;                                                          \
        }                                                                      \
    } while(0)
/**< @ingroup LacProc
 * At the end of the proc_read call, we set the eof flag to 1 */


/**
*******************************************************************************
 * @ingroup LacProc
 *      Create a file in the /proc subdirectory
 *
 * @description
 *      This function checks the state of LAC to see if it has being
 *      initialised
 * 
 * @param[in] procFileName          The name of the file to create
 * @param[in] procFileRead          Pointer to the component proc_read function
 *
 * @retval 0            No errors
 * @retval -ENOMEM      Errors at file creation
 *
 *****************************************************************************/
int lacProc_createFile(const char *procFileName, read_proc_t *procFileRead);

/**
*******************************************************************************
 * @ingroup LacProc
 *      Check to see if the LAC component has been initialised
 *
 * @description
 *      This function checks the state of LAC to see if it has being
 *      initialised
 * 
 * @param[in] procFileName          The name of the file to delete
 *
 *****************************************************************************/
void lacProc_deleteFile(const char *procFileName);

/**
*******************************************************************************
 * @ingroup LacProc
 *      Create the subdirectory in the /proc folder
 *
 * @retval 0            No errors
 * @retval -ENOMEM      Errors at file creation
 *
 *****************************************************************************/
int lacProc_createDirectory(void);

/**
*******************************************************************************
 * @ingroup LacProc
 *      Delete the subdirectory in the /proc folder
 *
 *****************************************************************************/
void lacProc_deleteDirectory(void);

#endif /* LAC_PROC_COMMON_P_H */
