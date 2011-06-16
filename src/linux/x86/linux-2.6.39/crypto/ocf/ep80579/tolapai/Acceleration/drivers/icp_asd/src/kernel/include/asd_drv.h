/**
 **************************************************************************
 * @file asd_drv.h
 *
 * @description
 *      This is the header file for the main ASD source file.
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

#ifndef _ASDDRV_H_
#define _ASDDRV_H_

#if (defined (__linux__) && defined (__KERNEL__))
#include <linux/kernel.h>
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAIL
#define FAIL 1
#endif

#ifndef ASD_BYTE
#define ASD_BYTE 1
#endif

#ifndef ASD_WORD
#define ASD_WORD 2
#endif

/* ASD module parameters */

extern int icp_asd_debug; /* When 1: print debug messages */ 
extern int icp_asd_load_fw; /* When 1: load firmware files  */
extern int icp_asd_auto_init; /* When 1: init acceleration subsystem straight away */
extern int icp_asd_reg_isr; /* When 1: register an ISR for the Ring Controller */

extern int icp_asd_ncdram_base; /* Provide base address of NCDRAM region, got from BIOS */
extern int icp_asd_ncdram_size; /* Size of NCDRAM region */
extern int icp_asd_cdram_base; /* Provide base address of CDRAM region, got from BIOS */
extern int icp_asd_cdram_size; /* Size of CDRAM region */

/* OS Driver name */
extern char asd_driver_name[];

/* 
 * ASD Print Macros
 */
#if defined (__linux__)
#define xprintk(level,level_str,fmt,args...) \
        printk(level "%s %s: %s: " fmt , asd_driver_name, level_str, (__func__), \
                ## args)

#ifdef ASD_SHOW_DEBUG
#define ASD_DEBUG(fmt,args...) \
        if (icp_asd_debug) xprintk(KERN_DEBUG, "debug", fmt, ## args)
#else
#define ASD_DEBUG(fmt,args...)
#endif

#define ASD_ERROR(fmt,args...) \
        xprintk(KERN_ERR, "error", fmt, ## args)

#define ASD_WARN(fmt,args...) \
        xprintk(KERN_WARNING, "warning", fmt, ## args)

#define ASD_INFO(fmt,args...) \
        xprintk(KERN_INFO, "info", fmt, ## args)

#elif defined (__freebsd)

#define xprintf(level_str,args...) \
{\
  printf("%s %s: %s: ", asd_driver_name, level_str, (__func__));\
  printf(args);\
}

#ifdef ASD_SHOW_DEBUG
#define ASD_DEBUG(args...) \
        if (icp_asd_debug) xprintf("debug", args)
#else
#define ASD_DEBUG(args...)
#endif

#define ASD_ERROR(args...) \
  xprintf("error", args)

#define ASD_WARN(args...) \
  xprintf("warning", args)

#define ASD_INFO(args...) \
  xprintf("info", args)

#else
#error Unsupported operating system
#endif /* OS */

/*
 * asd_init_devices
 * Initialize the Acceleration Subsystem for all accelerators found in the
 * system
 * Returns: 0 Success, 1 Failure
 */
extern int asd_init_devices(void);

/*
 * asddrv_release
 * 'Exported' asd_detach function
 */
#if defined(__freebsd)
extern int _asd_detach(device_t dev);
#endif


#endif /* _ASDDRV_H_ */
