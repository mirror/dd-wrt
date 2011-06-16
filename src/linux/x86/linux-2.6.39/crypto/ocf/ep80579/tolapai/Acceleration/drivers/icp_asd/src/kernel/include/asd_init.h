/**************************************************************************
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
 **************************************************************************/
/**
 *****************************************************************************
 * @file asd_init.h
 * 
 * @defgroup icp_Asd Acceleration System Driver Initialisation
 * 
 * @ingroup icp
 *
 * @description
 *      This header file that contains the prototypes and definitions required
 *      ASD initialisation.
 *
 *****************************************************************************/
#ifndef ASD_INIT_H
#define ASD_INIT_H 1


#include "icp_accel_handle.h"

/*
 * Version numbers for ASD Version Registration
 *
 * Major Version Number is updated upon significant feature change(s)
 * Significant POR changes
 *
 * Minor Version Number is updated upon less significant feature change(s)
 *
 * Patch Version Number is updated upon bug fixes, release candidate updates
 *
 */
#define ASD_MAJOR_VERSION 1
#define ASD_MINOR_VERSION 1
#define ASD_PATCH_VERSION 4

/*
 * Initialisation value for the asdModuleId, a value of 0 implies ASD has
 * not registered the version information with the DCC component.
 */
#define VERSION_INFO_UNREGISTERED 0 


/*
 * The following macro definitions are used to represent the status of the
 * subcomponents in the system.
 * Each subcomponent(HAL, LAC, BTR, QATAL, DCC, TLPFP) has a bit in the subsystem status 
 * bitfield that denotes whether the subcomponent is Initialised and/or Started.
 * Additionally, a bit is used to denote it the AccelEngine code is mapped and finally a bit is used to
 * denote if the AccelEngines are loaded.
 * The primary reason for using this is to faciliate the "clean" shutdown of the system i.e.
 * ensure we stop subcomponents that have been started, shutdown subcomponents that have been
 * initialised and deallocate resources that have been allocated.
 */

/* A bit to represent if HAL is initialised */
#define HAL_INITIALISED          0        
/* A bit to represent if LAC is initialised */
#define LAC_INITIALISED          1 
/* A bit to represent if BTR is initialised */        
#define BTR_INITIALISED          2        
/* A bit to represent if QATAL is initialised */
#define QATAL_INITIALISED        3        
/* A bit to represent if TLPFP is initialised */
#define TLPFP_INITIALISED        4    
/* A bit to represent if UCLO loader is initialised */
#define UCLO_INITIALISED         5        
/* A bit to represent if DCC is initialised */
#define DCC_INITIALISED          6 
/* A bit to represent if HAL is started */
#define HAL_STARTED              7  
/* A bit to represent if LAC is started */
#define LAC_STARTED              8
/* A bit to represent if BTR is started */
#define BTR_STARTED              9   
/* A bit to represent if QATAL is started */      
#define QATAL_STARTED            10   
/* A bit to represent if TLPFP is started */      
#define TLPFP_STARTED            11 
/* A bit to represent if DCC is started */       
#define DCC_STARTED              13 
/* A bit to represent if the ISR Service resources are allocated */ 
#define ISR_RESOURCES_ALLOCATED  14  
/* A bit to represent if AccelEngine microcode is mapped */      
#define AE_UCODE_MAPPED        15       
/* A bit to represent if AccelEngines are loaded */
#define AE_LOADED              16       

/*
 * This macro sets the specified bit in status to 1
 * i.e. set the bit
 */
#define SET_STATUS_BIT( status, bit )   status |= (1 << bit)

/*
 * This macro sets the specified bit in status to 0
 * i.e. clears the bit
 */
#define CLEAR_STATUS_BIT( status, bit )  status &= ~(1 << bit)

/*
 * This macro checks if the specified bit in status is set or not.
 */
#define BIT_IS_SET( status, bit )       ( status & (1<<bit) )


/**
 *****************************************************************************
 * @ingroup asd
 * 
 * @description
 *      This function will initialise the subcomponents, map the firmware, load 
 *      the AEs and start the subcomponent
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param accel_dev           IN  This input parameter is a pointer to the 
 *                                icp_accel_dev_t structure.
 *
 * @retval CPA_STATUS_SUCCESS   Function executed successfully.
 * @retval CPA_STATUS_FAIL      Function failed.
 * 
 * 
 *****************************************************************************/


extern CpaStatus asd_SubsystemInit( icp_accel_dev_t *accel_dev); 

/**
 *****************************************************************************
 * @ingroup asd
 * 
 * @description
 *      This function will stop and shutdown the subcomponents in the system,
 *      and free resources for ISR and firmware loading that have been allocated
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @blocking
 *      No
 * @reentrant
 *      No
 * @threadSafe
 *      Yes
 * 
 * @param accel_dev           IN  This input parameter is a pointer to the 
 *                                icp_accel_dev_t structure.
 *
 * @retval CPA_STATUS_SUCCESS   Function executed successfully.
 * @retval CPA_STATUS_FAIL      Function failed.
 * 
 * 
 *****************************************************************************/

extern CpaStatus asd_SubsystemShutdown( icp_accel_dev_t *accel_dev);


#endif /* ASD_INIT_H */
