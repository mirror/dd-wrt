/**
 * @file IxFeatureCtrl_sp.h
 *
 * @date 26-Oct-2005
 *
 * @brief This file contains the semi-private API of the Feature Control component.
 *
 * 
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
*/

/**
 * @defgroup IxFeatureCtrlPrivateAPI Intel(R) IXP Software Feature Control (featureCtrl) Private API
 *
 * @brief The Semi-private API for the IXP Feature Control.
 * 
 */

#ifndef IXFEATURECTRL_P_H
#define IXFEATURECTRL_P_H

/*
 * User defined include files
 */
#include "IxOsal.h"
#include "IxNpeDl.h"
#include "IxFeatureCtrl.h"

/*
 * #defines and macros
 */
 
/*
 * Prototypes for interface functions
 */

/**
 * @ingroup IxFeatureCtrlPrivateAPI 
 *
 * @fn IxFeatureCtrlReg ixFeatureCtrlRead (void)
 */ 

/* @brief This function reads out the CURRENT value of Feature Control Register.
 *        The current value may not be the same as that of the hardware component 
 *        availability.    
 * 
 * The bit location of each hardware component is defined above. 
 * A value of '1' in bit means the hardware component is not available.  A value of '0'   
 * means the hardware component is available.
 */
 
/*
 * @return 
 *      - IxFeatureCtrlReg - the enabled/disabled status of components
 *	
 * @internal
 *
 */ 
IxFeatureCtrlReg
ixFeatureCtrlRead (void);

/**
 * @ingroup IxFeatureCtrlPrivateAPI 
 *
 * @fn void ixFeatureCtrlWrite (IxFeatureCtrlReg expUnitReg)
 */ 
/*
 * @brief This function write the value stored in IxFeatureCtrlReg expUnitReg  
 *        to the Feature Control Register. 
 * 
 * The bit location of each hardware component is defined above.
 * The write is only effective on available hardware components. Writing '1' in a  
 * bit will software disable the respective hardware component. A '0' will mean that  
 * the hardware component will remain to be operable.
 */

/*
 * @param expUnitReg @ref IxFeatureCtrlReg [in] - The value to be written to GCU Software
 *                                                Fuse register.
 *
 * @return none
 *	
 * @internal
 *
 */ 
void
ixFeatureCtrlWrite (IxFeatureCtrlReg expUnitReg);

#endif  /* IXFEATURECTRL_SP_H */
 
