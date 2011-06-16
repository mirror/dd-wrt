/**
 * @file IxOsalOsOemSys.h
 *
 * @brief linux and EP805XX specific defines
 *
 * Design Notes:
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
 */

#ifndef IxOsalOsOemSys_H
#define IxOsalOsOemSys_H

/*
 * Important Note: The current #defines in this 
 * file are just place-holders and should be 
 * replaced and filled once the actual values 
 * are available. This file has portions that 
 * has glue-code/logic/stubs and should cease 
 * to exist in the final code.
 */


#ifndef IxOsalOsOem_H
#error "Error: IxOsalOsOemSys.h cannot be included directly before IxOsalOsOem.h"
#endif

/* Memory Base Address */

/* Memory Mapping size */

/* Expansion Bus */

/* Time Stamp Resolution */
#define IX_OSAL_EP805XX_TIME_STAMP_RESOLUTION    	(846596000) /**< 846.596 MHz	*/

/*********************
 *	Memory map
 ********************/

/* Note: - dynamic maps will be mapped using ioremap() with the base addresses and sizes declared in this array (matched to include the requested zones, 
           but not using the actual IxOsalMemoryMap requests) 
         - static maps have to be also declared 
         - the user-friendly name will be made available in /proc for dynamically allocated maps 
         - the declared order of the maps is important if the maps overlap - when a common zone is requested only the
           first usable map will be always chosen */


/* Global memmap only visible to IO MEM module */


#ifdef IXOSALIOMEM_C
IxOsalMemoryMap ixOsalGlobalMemoryMap[] = {
    /*
     * Dummy - Place holder.
     */
    {
     IX_OSAL_STATIC_MAP,			/* type            */
     IX_OSAL_EP805XX_DUMMY_PHYS_BASE,	/* physicalAddress */
     IX_OSAL_EP805XX_DUMMY_MAP_SIZE,		/* size            */
     IX_OSAL_EP805XX_DUMMY_VIRT_BASE,	/* virtualAddress  */
     0,						/* mapFunction     */
     0,						/* unmapFunction   */
     0,						/* refCount        */
     IX_OSAL_LE,				/* endianType      */   
     "Dummy"					/* name            */
     }
};

#endif /* IXOSALIOMEM_C */

#endif /* IxOsalOsOemSys_H */


