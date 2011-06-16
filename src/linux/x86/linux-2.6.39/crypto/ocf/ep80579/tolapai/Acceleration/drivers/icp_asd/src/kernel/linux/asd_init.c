/*****************************************************************************
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
*****************************************************************************/
/**
 *****************************************************************************
 * @ingroup ASD
 * @file  asd_init.c 
 * @description
 *        This file contains the subcomponent module initialisation code for the
 *        Acceleration System Driver.
 *****************************************************************************/
/*****************************************************************************
 *
 * Contents:
 *
 *        Definitions of following local functions:
 *            asd_SubsystemInit 
 *            asd_SubsystemShutdown
 *
 *****************************************************************************/
#include "cpa.h"
#include "icp.h"
#include "icp_asd_cfg.h"
#include "icp_accel_handle.h"
#include "icp_hal_cfg.h"
#include "icp_lac_cfg.h"
#include "icp_qatal_cfg.h"
#include "icp_dcc_cfg.h"
#include "icp_dcc_al.h"
#include "asd_uclo_ldr.h"
#include "asd_init.h"
#include "asd_isr.h"
#include "asd_drv.h"
#include "asd_cfg.h"


/*
 * The following global variable to represent the status of the subcomponents in
 * the system.
 */
static Cpa32U asdSubsystemStatus = 0x0;

/*
 * asdModuleId provided to the DCC component at ASD Version Registration.
 * The module id is initialised to VERSION_INFO_UNREGISTERED, this value implies
 * that ASD has not registerd with the DCC component.
 * The DCC component will set the module id to a non-zero value when ASD 
 * registers it's version inforamtion.
 */
static Cpa32U asdModuleId = VERSION_INFO_UNREGISTERED;

/*****************************************************************************
 * asd_register_version_info
 *
 * This function registers the ASD version information with the DCC Component
 ******************************************************************************/
static CpaStatus asdRegisterVersionInfo(void)
{
    icp_dcc_ver_info_t asd_version_info;

     /* 
      * Populate the versioning structure 
      */
    strncpy(asd_version_info.name, asd_driver_name, 
            (sizeof(asd_version_info.name) - 1));
    asd_version_info.name[sizeof(asd_version_info.name) - 1] = '\0';
    asd_version_info.majorVersion = ASD_MAJOR_VERSION;
    asd_version_info.minorVersion = ASD_MINOR_VERSION;
    asd_version_info.patchVersion = ASD_PATCH_VERSION;

    /*
     * Register the version information with DCC
     */
    if (ICP_STATUS_SUCCESS != icp_DccComponentVersionRegister(&asd_version_info,
                                                              &asdModuleId))
    {
        return CPA_STATUS_FAIL;
    }
    return CPA_STATUS_SUCCESS;
}



/*****************************************************************************
 * Abstract:
 *    This function initiates the initialisation of all sub-component modules.
 *    Sub-component initialisation is divided into two stages: init and start.
 *    This is to facilitate any ordering dependencies between sub-components 
 *    prior to starting any of the AccelEngines. 
 *    The init stage is used to execute code to initialize a subcomponent prior
 *    to starting any AE. It is used for tasks such as allocating system resources 
 *    (eg memory, locks), patching microcode, doing ring allocation etc. 
 *    The start stage, which is mandatory but primarily useful for sub-components
 *    that manage an AE, is used to complete initialisation and start the required
 *    AccelEngine(s). All init functions are called first by the ASD, followed by 
 *    their start functions.
 *
 * Side Effects:
 *
 * Assumptions:
 *
 *****************************************************************************/

CpaStatus                             /* OUT: status of the initialisation*/
asd_SubsystemInit(
               icp_accel_dev_t *accel_dev) /* IN:pointer to icp_accel_dev_t */

{
 
    CpaStatus returnCode = 0; 
    CpaInstanceHandle instanceHandle = 0;
    instanceHandle = (CpaInstanceHandle)((Cpa32U)accel_dev);

    /*
     * Sub-component initialisation is divided into two stages: init and start. This is
     * to facilitate any ordering dependencies between sub-components prior to starting 
     * any of the AccelEngines.                                                         
     * The init sequence for the sub-components is as follows:                          
     *                 - Init HAL                                              
     *                 - Init UCLO   
     *                 - Init DCC                                            
     *                 - Initialise the ASD ISR Services                       
     *                 - Map the AE Mirco code                 
     *                 - Init QATAL
     *                 - Init LAC                                              
     *                 - Load the AE Micro code                 
     */

    returnCode = icp_AsdCfgHalInit(instanceHandle, icp_AsdCfgParamGet);
    if (CPA_STATUS_SUCCESS != returnCode)
    {
        ASD_ERROR("Failed to initialise HAL\n");
        return CPA_STATUS_FAIL;
    }
    else
    {
        SET_STATUS_BIT(asdSubsystemStatus, HAL_INITIALISED);
    }

    returnCode = asd_uclo_ldr_init( accel_dev );
    if (CPA_STATUS_SUCCESS != returnCode)
    {
        ASD_ERROR("Failed to initialise uclo loader\n");
        return CPA_STATUS_FAIL;
    }
    else
    {
        SET_STATUS_BIT(asdSubsystemStatus, UCLO_INITIALISED);
    }

    returnCode = icp_AsdCfgDccInit(instanceHandle, icp_AsdCfgParamGet);
    if (CPA_STATUS_SUCCESS != returnCode)
    {
        ASD_ERROR("Failed to initialise DCC\n");
        return CPA_STATUS_FAIL;
    }
    else
    {
        SET_STATUS_BIT(asdSubsystemStatus, DCC_INITIALISED);
    }

    if (icp_asd_reg_isr) 
    {
        int result = 0;
        result = asd_isr_resource_alloc(accel_dev, &accel_dev->ringCtrlr);

        if (result)
        {
            ASD_ERROR("Failed to initialise ISR Services\n");
            return CPA_STATUS_FAIL;
        }
        else
        {
            SET_STATUS_BIT(asdSubsystemStatus, ISR_RESOURCES_ALLOCATED);
        }
    }

    if (icp_asd_load_fw)
    {
        returnCode = asd_uclo_ae_map_ucode( accel_dev );
    
        if (CPA_STATUS_SUCCESS != returnCode)
        {
            ASD_ERROR("Failed to map the Microcode\n");
            return CPA_STATUS_FAIL;
        }
        else
        {
             SET_STATUS_BIT(asdSubsystemStatus, AE_UCODE_MAPPED);
        }
    
    }

    returnCode = icp_AsdCfgQatalInit(instanceHandle, icp_AsdCfgParamGet);
    if (CPA_STATUS_SUCCESS != returnCode)
    {
        ASD_ERROR("Failed to initialise QATAL\n");
        return CPA_STATUS_FAIL;
    }
    else
    {
        SET_STATUS_BIT(asdSubsystemStatus, QATAL_INITIALISED);
    }

    returnCode = icp_AsdCfgLacInit(instanceHandle, icp_AsdCfgParamGet);
    if (CPA_STATUS_SUCCESS != returnCode)
    {
        ASD_ERROR("Failed to initialise LAC\n");
        return CPA_STATUS_FAIL;
    }
    else
    {
        SET_STATUS_BIT(asdSubsystemStatus, LAC_INITIALISED);
    }


    if(icp_asd_load_fw)
    {
        returnCode = asd_uclo_ae_load( accel_dev );
        if (CPA_STATUS_SUCCESS != returnCode)
        {
            ASD_ERROR("Failed to load the Microcode\n");
            return CPA_STATUS_FAIL;
        }
        else
        {
            SET_STATUS_BIT(asdSubsystemStatus, AE_LOADED);
        }
    }

    /*
     * Start the sub-components. The start sequence is as follows:                 
     *                 - Start HAL                                          
     *                 - Start DCC                                            
     *                 - Start QATAL
     *                 - Start LAC                                             
     */

    returnCode = icp_AsdCfgHalStart(instanceHandle);
    if (CPA_STATUS_SUCCESS != returnCode)
    {
        ASD_ERROR("Failed to start HAL\n");
        return CPA_STATUS_FAIL;
    }
    else
    {
        SET_STATUS_BIT(asdSubsystemStatus, HAL_STARTED);
    }

    returnCode = icp_AsdCfgDccStart(instanceHandle);
    if (CPA_STATUS_SUCCESS != returnCode)
    {
        ASD_ERROR("Failed to start DCC\n");
        return CPA_STATUS_FAIL;
    }
    else
    {
        SET_STATUS_BIT(asdSubsystemStatus, DCC_STARTED);
    }

    returnCode = icp_AsdCfgQatalStart(instanceHandle);
    if (CPA_STATUS_SUCCESS != returnCode)
    {
        ASD_ERROR("Failed to start QATAL\n");
        return CPA_STATUS_FAIL;
    }
    else
    {
        SET_STATUS_BIT(asdSubsystemStatus, QATAL_STARTED);
    }


    returnCode = icp_AsdCfgLacStart(instanceHandle);
    if (CPA_STATUS_SUCCESS != returnCode)
    {
        ASD_ERROR("Failed to start LAC\n");
        return CPA_STATUS_FAIL;
    }
    else
    {
        SET_STATUS_BIT(asdSubsystemStatus, LAC_STARTED);
    }

    /*
     * Register the version information with DCC if we have not already
     * registered. An asdModuleId != 0, implies we have registered
     */
    if (VERSION_INFO_UNREGISTERED == asdModuleId)
    {
        returnCode = asdRegisterVersionInfo();
        if (CPA_STATUS_SUCCESS != returnCode)
        {
            ASD_ERROR("Failed to register version information with DCC\n");
        }
    }

    return CPA_STATUS_SUCCESS;
}


/*****************************************************************************
 * Abstract:
 *    This function initiates the cleanup of all sub-component modules.
 *    Sub-component cleanup is also divided into two stages: stop and shutdown.
 *    It works in the reverse order to the initialisation sequence. The stop 
 *    stage, which is mandatory but primarily useful for sub-components that 
 *    manage an AE, is used to begin the cleanup sequence and stop the required 
 *    AccelEngine(s). The shutdown stage, is used to execute code to release any
 *    any of the sub-components' system resources.
 *    All stop functions are called first by the ASD, followed by their shutdown
 *    functions.
 *
 * Side Effects:
 *
 * Assumptions: 
 *    This is a best-effort shutdown. If a sub-component fails to shutdown the
 *    function will not abort but continue to attempt to shutdown remaining 
 *    sub-components
 *
 *****************************************************************************/

CpaStatus                             /* OUT: status of the shutdown   */
asd_SubsystemShutdown(
                   icp_accel_dev_t *accel_dev)   /* IN: opaque device handle */
{
    CpaStatus returnCode = 0; 
    CpaStatus status = CPA_STATUS_SUCCESS;
    CpaInstanceHandle instanceHandle = 0;

    instanceHandle = (CpaInstanceHandle)((Cpa32U)accel_dev);

    /*
     * The stop stage is used to begin the cleanup sequence and stop the required
     * AccelEngines. The stop sequence for the sub-components is as follows: 
     *                          - Stop LAC
     *                          - Stop QATAL
     *                          - Stop HAL 
     *                          - Stop DCC                                  
     */


    if (BIT_IS_SET(asdSubsystemStatus, LAC_STARTED))
    {
        returnCode = icp_AsdCfgLacStop(instanceHandle);
        if (CPA_STATUS_SUCCESS != returnCode)
        {
            ASD_ERROR("Failed to stop LAC\n");
            status = returnCode;
        }
        else
        {
            CLEAR_STATUS_BIT(asdSubsystemStatus, LAC_STARTED);
        }
    }

    if (BIT_IS_SET(asdSubsystemStatus, QATAL_STARTED))
    {
        returnCode = icp_AsdCfgQatalStop(instanceHandle);
        if (CPA_STATUS_SUCCESS != returnCode)
        {
            ASD_ERROR("Failed to stop QATAL\n");
            status = returnCode;
        }
        else
        {
            CLEAR_STATUS_BIT(asdSubsystemStatus, QATAL_STARTED);
        }
    }

    if (BIT_IS_SET(asdSubsystemStatus, HAL_STARTED))
    {
        returnCode = icp_AsdCfgHalStop(instanceHandle);
        if (CPA_STATUS_SUCCESS != returnCode)
        {
            ASD_ERROR("Failed to stop HAL\n");
            status = returnCode;
        }
        else
        {
            CLEAR_STATUS_BIT(asdSubsystemStatus, HAL_STARTED);
        }
    }

    if (BIT_IS_SET(asdSubsystemStatus, DCC_STARTED))
    {
        returnCode = icp_AsdCfgDccStop(instanceHandle);
        if (CPA_STATUS_SUCCESS != returnCode)
        {
            ASD_ERROR("Failed to stop DCC\n");
            status = returnCode;
        }
        else
        {
            CLEAR_STATUS_BIT(asdSubsystemStatus, DCC_STARTED);
        }
    }

    /* 
     * The shutdown stage, is used to execute code to release any of the sub-components 
     * system resources.                                                                
     * The shutdown sequence for the sub-components is as follows:                      
     *                          - Shutdown LAC
     *                          - Shutdown QATAL
     *                          - Shutdown UCLO                                          
     *                          - Shutdown HAL
     *                          - Shutdown DCC
     */

    if (BIT_IS_SET(asdSubsystemStatus,ISR_RESOURCES_ALLOCATED))
    {
        asd_isr_resource_free(accel_dev, &accel_dev->ringCtrlr);
        CLEAR_STATUS_BIT(asdSubsystemStatus, ISR_RESOURCES_ALLOCATED);
        status = CPA_STATUS_SUCCESS;
    }

    if (BIT_IS_SET(asdSubsystemStatus, LAC_INITIALISED))
    {
        returnCode = icp_AsdCfgLacShutdown(instanceHandle);
        if (CPA_STATUS_SUCCESS != returnCode)
        {
            ASD_ERROR("Failed to shutdown LAC\n");
            status = returnCode;
        }
        else
        {
            CLEAR_STATUS_BIT(asdSubsystemStatus, LAC_INITIALISED);
        }
    }

    if (BIT_IS_SET(asdSubsystemStatus, QATAL_INITIALISED))
    {
        returnCode = icp_AsdCfgQatalShutdown(instanceHandle);
        if (CPA_STATUS_SUCCESS != returnCode)
        {
            ASD_ERROR("Failed to shutdown QATAL\n");
            status = returnCode;
        }
        else
        {
            CLEAR_STATUS_BIT(asdSubsystemStatus, QATAL_INITIALISED);
        }
    }
   
    if (BIT_IS_SET(asdSubsystemStatus, UCLO_INITIALISED))
    {
        returnCode = asd_uclo_ldr_stop( accel_dev );

        if (CPA_STATUS_SUCCESS != returnCode)
        {
            ASD_ERROR("Failed to shutdown UCLO loader\n");
            status = returnCode;
        }
        else
        {
            CLEAR_STATUS_BIT(asdSubsystemStatus, UCLO_INITIALISED);
        }
    }

    if (BIT_IS_SET(asdSubsystemStatus, HAL_INITIALISED))
    {
        returnCode = icp_AsdCfgHalShutdown(instanceHandle);
        if (CPA_STATUS_SUCCESS != returnCode)
        {
            ASD_ERROR("Failed to shutdown HAL\n");
            status = returnCode;
        }
        else
        {
            CLEAR_STATUS_BIT(asdSubsystemStatus, HAL_INITIALISED);
        }
    }

    if (BIT_IS_SET(asdSubsystemStatus, DCC_INITIALISED))
    {
        returnCode = icp_AsdCfgDccShutdown(instanceHandle);
        if (CPA_STATUS_SUCCESS != returnCode)
        {
            ASD_ERROR("Failed to shutdown DCC\n");
            status = returnCode;
        }
        else
        {
            CLEAR_STATUS_BIT(asdSubsystemStatus, DCC_INITIALISED);
        }
    }
    return status;
}

