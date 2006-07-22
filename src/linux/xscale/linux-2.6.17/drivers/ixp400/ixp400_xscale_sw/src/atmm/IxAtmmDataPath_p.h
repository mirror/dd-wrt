/**
 * @file    IxAtmmDataPath_p.h
 *
 * @author Intel Corporation
 * @date    01-MAR-2002
 *
 * @brief   This file contains the internal functions and data types for AtmmDataPath
 *
 * 
 * @par
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
 */

#ifndef IXATMMDATAPATH_P_H
#define IXATMMDATAPATH_P_H
/*
 * User defined header files
 */
#include "IxOsalTypes.h"
#include "IxAtmm.h"

/*
 * #defines and macros used in this file.
 */

/*
 * Typedefs
 */

/**
 * @typedef Prototype of the function to query a ports details
 * 
 * @brief Callback prototype for querying a ports details
 *
 * @param IxLogicalPort port (in)  port to query
 * @param unsigned vpi (in) vpi to query on the port
 * @param unsigned vci (in) vci to query on the port
 * @param IxAtmmVcDirection direction (in) direction of the VC
 * @param IxAtmSchedulerVcId *vcId (out) vcId of the port found
 * @param IxAtmmVc *vcDesc (out) vcDesc of the port found
 */
typedef IX_STATUS (*IxAtmmVcQueryCallback) (IxAtmLogicalPort port, 
					    unsigned vpi, 
					    unsigned vci, 
					    IxAtmmVcDirection direction, 
					    IxAtmSchedulerVcId *vcId, 
					    IxAtmmVc *vcDesc);

/* 
 * Function prototypes 
 */

/**
 * @fn IX_STATUS ixAtmmDataPathSetup (IxAtmLogicalPort port)
 *
 * @brief This function configures receive and transmit control for
 *        a port.
 *
 * @param IxAtmLogicalPort port (in) - The port to setup
 *
 * @return IX_STATUS
 *
 */
IX_STATUS
ixAtmmDataPathSetup (IxAtmLogicalPort port);


/**
 * @fn IX_STATUS ixAtmmDataPathUninit (IxAtmLogicalPort port)
 *
 * @brief This function uninitialises the port specified.
 *
 * @param IxAtmLogicalPort port (in) - The port to uninitialise
 *
 * @return IX_STATUS
 *
 */
IX_STATUS
ixAtmmDataPathUninit (IxAtmLogicalPort port);


/**
 * @fn void ixAtmmVcQueryCallbackRegister ( IxAtmmVcQueryCallback callback)
 *
 * @brief This function registers a callback function that is used to query a VC
 *
 * @param IxAtmmVcQueryCallback callback (in) callback function to register
 *
 * @return void
 *
 */
void
ixAtmmVcQueryCallbackRegister ( IxAtmmVcQueryCallback callback);



/**
 * @fn void ixAtmmVcQueryCallbackUnregister ( void)
 *
 * @brief This function unregister the callback function
 *
 * @return void
 *
 */
void
ixAtmmVcQueryCallbackUnregister(void);



#endif /* IXATMMDATAPATH_P_H */
