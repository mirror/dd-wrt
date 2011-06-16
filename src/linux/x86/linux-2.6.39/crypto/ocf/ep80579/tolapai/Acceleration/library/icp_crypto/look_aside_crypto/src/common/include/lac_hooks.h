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
 * @file lac_hooks.h    
 * 
 * @defgroup LacHooks   Hooks
 * 
 * @ingroup LacCommon
 * 
 * Component Init/Shutdown functions. These are: 
 *  - an init function which is called during the intialisation sequence,
 *  - a shutdown function which is called by the overall shutdown function,
 *  
 ******************************************************************************/

#ifndef LAC_HOOKS_H
#define LAC_HOOKS_H

/* 
********************************************************************************
* Include public/global header files 
********************************************************************************
*/ 

#include "cpa.h"

/* 
********************************************************************************
* Include private header files 
********************************************************************************
*/ 


/******************************************************************************/

/**
 *******************************************************************************
 * @ingroup LacHooks
 *      This function initialises the Large Number (ModExp and ModInv) module
 *
 * @description
 *      This function clears the Large Number statistics and registers any
 *      internal callbacks needed by the module.
 *
 * @retval CPA_STATUS_SUCCESS       No error
 * @retval CPA_STATUS_FAIL          Function failed.
 * @retval CPA_STATUS_RESOURCE      Error related to system resources.
 *
 ******************************************************************************/

CpaStatus
LacLn_Init(void);
                                                           
/**
 *******************************************************************************
 * @ingroup LacHooks
 *      This function shuts down the Large Number (ModExp and ModInv) module
 *
 * @description
 *      This function shuts down the Large Number module. It destroys the
 *      Large Number callback table.
 *
 * @retval CPA_STATUS_SUCCESS       No error
 * @retval CPA_STATUS_FAIL          Function failed.
 * @retval CPA_STATUS_RESOURCE      Error related to system resources.
 *
 ******************************************************************************/

CpaStatus
LacLn_Shutdown(void);

/**
 *******************************************************************************
 * @ingroup LacHooks
 *      This function initialises the Prime module
 *
 * @description
 *      This function clears the Prime statistics and registers any
 *      internal callbacks needed by the module.
 *
 * @retval CPA_STATUS_SUCCESS       No error
 * @retval CPA_STATUS_FAIL          Function failed.
 * @retval CPA_STATUS_RESOURCE      Error related to system resources.
 *
 ******************************************************************************/
                                                                    
CpaStatus
LacPrime_Init(Cpa64U numAsymConcurrentReq);
                                                                      
/**
 *******************************************************************************
 * @ingroup LacHooks
 *      This function shuts down the Prime module
 *
 * @description
 *      This function shuts down the Prime module. It destroys the
 *      Prime callback table.
 *
 * @retval CPA_STATUS_SUCCESS       No error
 * @retval CPA_STATUS_FAIL          Function failed.
 * @retval CPA_STATUS_RESOURCE      Error related to system resources.
 *
 ******************************************************************************/
                                                                    
CpaStatus
LacPrime_Shutdown(void);

/**
 *******************************************************************************
 * @ingroup LacHooks
 *      This function initialises the DSA module
 * 
 * @description
 *      This function clears the DSA statistics and registers any
 *      internal callbacks needed by the module.
 * 
 * @retval CPA_STATUS_SUCCESS       No error
 * @retval CPA_STATUS_FAIL          Function failed.
 * @retval CPA_STATUS_RESOURCE      Error related to system resources.
 * 
 ******************************************************************************/

CpaStatus 
LacDsa_Init(void);

/**
 *******************************************************************************
 * @ingroup LacHooks
 *      This function shuts down the DSA module
 * 
 * @description
 *      This function shuts down the DSA module. It destroys the
 *      DSA callback table.
 * 
 * @retval CPA_STATUS_SUCCESS       No error
 * @retval CPA_STATUS_FAIL          Function failed.
 * @retval CPA_STATUS_RESOURCE      Error related to system resources.
 * 
 ******************************************************************************/

CpaStatus 
LacDsa_Shutdown(void);

/******************************************************************************/

/**
 *******************************************************************************
 * @ingroup LacHooks
 *      This function initialises the Diffie Hellmann module
 * 
 * @description
 *      This function clears the Diffie Hellman statistics and registers any
 *      internal callbacks needed by the module.
 * 
 * @retval CPA_STATUS_SUCCESS       No error
 * @retval CPA_STATUS_FAIL          Function failed.
 * @retval CPA_STATUS_RESOURCE      Error related to system resources.
 * 
 ******************************************************************************/
CpaStatus 
LacDh_Init(void);

/**
 *******************************************************************************
 * @ingroup LacHooks
 *      This function shuts down the Diffie Hellmann module
 * 
 * @description
 *      This function shuts down the Diffie Hellman module. It destroys the
 *      Diffie Hellman callback table.
 * 
 * @retval CPA_STATUS_SUCCESS       No error
 * @retval CPA_STATUS_FAIL          Function failed.
 * @retval CPA_STATUS_RESOURCE      Error related to system resources.
 * 
 ******************************************************************************/
CpaStatus 
LacDh_Shutdown(void);


/**
*******************************************************************************
 * @ingroup LacHooks
 *      The function initialises the symmetric component
 * 
 * @description
 *      This function calls the init functions of all the components and 
 *      sub-components that the symmetric component depends on. 
 *
 * @param[in] numSymConcurrentReq   Number of outstanding symmetric requests.   
 * 
 * @retval CPA_STATUS_SUCCESS       Normal Operation
 * @retval CPA_STATUS_RESOURCE      Error related to system resources.
 * @retval CPA_STATUS_FAIL          Cannot be initialised
 *
 *****************************************************************************/
CpaStatus
LacSym_Init(Cpa64U numSymConcurrentReq);

/**
*******************************************************************************
 * @ingroup LacHooks
 *      The function shuts down the symmetric component
 * 
 * @description
 *      This function calls the shut down functions of all the components and 
 *      sub-components that the symmetric component depends on. 
 * 
 * @retval CPA_STATUS_SUCCESS       Normal Operation
 * @retval CPA_STATUS_RESOURCE      Error related to system resources.
 * @retval CPA_STATUS_FAIL          Cannot be shut down
 *
 *****************************************************************************/
CpaStatus
LacSym_Shutdown(void);


/**
 ******************************************************************************
 * @ingroup LacSymKey
 *      This function registers the callback handlers to SSL/TLS and MGF and 
 *      allocates resources that are needed for the component.
 *
 * @retval CPA_STATUS_SUCCESS   Status Success
 * @retval CPA_STATUS_FAIL      General failure
 * @retval CPA_STATUS_RESOURCE  Resource allocation failure
 * 
 *****************************************************************************/
CpaStatus
LacSymKey_Init(void);


/**
 ******************************************************************************
 * @ingroup LacSymKey
 *      This function frees up resources obtained by the key gen component
 *
 * @retval CPA_STATUS_SUCCESS   Status Success
 * @retval CPA_STATUS_FAIL      General failure
 * 
 *****************************************************************************/
CpaStatus
LacSymKey_Shutdown(void);


/**
*******************************************************************************
 * @ingroup LAC_RAND
 *      Initialise the Random module
 * 
 * @description
 *      Initialise the Random module
 *
 * @retval CPA_STATUS_SUCCESS       Normal operation
 * @retval CPA_STATUS_FAIL          Failed operation
 *
 *****************************************************************************/
CpaStatus 
LacRand_Init(void);

/**
 *******************************************************************************
 * @ingroup LAC_RAND
 *      Shutdown the Random module
 * 
 * @description
 *      Shutdown the Random module. It destroys the
 *      preallocated random data buffer.
 * 
 * @retval CPA_STATUS_SUCCESS       Normal operation
 * @retval CPA_STATUS_FAIL          Failed operation
 * 
 ******************************************************************************/
CpaStatus 
LacRand_Shutdown(void);

/**
 *******************************************************************************
 * @ingroup LacHooks
 *      This function initialises the RSA module
 * 
 * @description
 *      This function clears the RSA statistics and registers any
 *      internal callbacks needed by the module.
 * 
 * @retval CPA_STATUS_SUCCESS       Normal operation
 * @retval CPA_STATUS_FAIL          Function failed.
 * @retval CPA_STATUS_RESOURCE      Error related to system resources.
 * 
 ******************************************************************************/
CpaStatus 
LacRsa_Init(void);

/**
 *******************************************************************************
 * @ingroup LacHooks
 *      This function shuts down the RSA module
 * 
 * @description
 *      This function shuts down the RSA module. It destroys the
 *      RSA callback table.
 * 
 * @retval CPA_STATUS_SUCCESS       No error
 * @retval CPA_STATUS_FAIL          Function failed.
 * @retval CPA_STATUS_RESOURCE      Error related to system resources.
 * 
 ******************************************************************************/
CpaStatus 
LacRsa_Shutdown(void);

#endif /* LAC_HOOKS_H */
