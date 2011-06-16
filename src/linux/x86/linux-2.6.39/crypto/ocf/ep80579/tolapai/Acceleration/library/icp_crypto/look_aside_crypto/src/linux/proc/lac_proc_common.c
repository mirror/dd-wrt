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
 * @file lac_proc_common.c /proc helper functions
 *
 * @ingroup LacProc
 *
 * @description This file implements helpers for /proc file definitions.
 *****************************************************************************/

/*
********************************************************************************
* Include public/global header files
********************************************************************************
*/

#include "lac_proc_common_p.h"

#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/version.h>

/*
********************************************************************************
* Include private header files
********************************************************************************
*/


/*
********************************************************************************
* Static Variables
********************************************************************************
*/
#define PROC_DIR_NAME   "icp-crypto"
#define PROC_DIR_PATH   "/proc/" PROC_DIR_NAME "/"

static struct proc_dir_entry *lacProcDir = 0;


/*
********************************************************************************
* Define static function definitions
********************************************************************************
*/

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

int
lacProc_createFile(const char *procFileName, read_proc_t *procFileRead)
{
    struct proc_dir_entry *procFile = create_proc_read_entry( 
        procFileName, 0, lacProcDir, procFileRead, NULL);
        /* name, mode, parent, read function, client data */
            
    /* File creation failed, we print an error message
     * and return an error code */
    if (NULL == procFile)
    {
        remove_proc_entry(procFileName, lacProcDir);
        printk(KERN_ERR "Could not initialize " PROC_DIR_PATH "%s\n", 
                procFileName);
        return -ENOMEM;
    }

    /* Everything is Ok */
    printk(KERN_DEBUG PROC_DIR_PATH "%s file created\n", procFileName);    
    return 0;   
}

void
lacProc_deleteFile(const char *procFileName)
{
    /* Cleanup proc file, function can't fail */
    remove_proc_entry(procFileName, lacProcDir);
    printk(KERN_DEBUG PROC_DIR_PATH "%s file removed\n", procFileName);       
}

int
lacProc_createDirectory()
{
    /* Create the LAC folder in /proc */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
    lacProcDir = proc_mkdir(PROC_DIR_NAME, &proc_root);
#else
    lacProcDir = proc_mkdir(PROC_DIR_NAME, NULL);
#endif
    /* Check for error during the folder creation */
    if(NULL == lacProcDir)
    {
        printk(KERN_ERR "Could not initialize directory " PROC_DIR_PATH "\n");
        return -ENOMEM;
    }

    /* Everything is Ok */
    printk(KERN_DEBUG PROC_DIR_PATH " directory created\n");
    return 0;
}

void
lacProc_deleteDirectory()
{
    /* Delete the LAC folder in /proc, function can't fail */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
    remove_proc_entry(PROC_DIR_NAME, &proc_root);
#else
    remove_proc_entry(PROC_DIR_NAME, NULL);
#endif
    printk(KERN_DEBUG PROC_DIR_PATH " directory removed\n");   
}
