/**
 * @file IxOsServicesComponents.h (Replaced by OSAL)
 *
 * @brief Header file for memory access
 *
 * @par
 * 
 * @par
 * IXP400 SW Release version  2.0
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2005 Intel Corporation All Rights Reserved.
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

#ifndef IxOsServicesComponents_H
#define IxOsServicesComponents_H

#include "IxOsalBackward.h"
 * codelets_parityENAcc
 * timeSyncAcc
 * parityENAcc
 * sspAcc
 * i2c
 * integration_sspAcc
 * integration_i2c
#define ix_timeSyncAcc         36
#define ix_parityENAcc         37
#define ix_codelets_parityENAcc     38
#define ix_sspAcc              39
#define ix_i2c                 40
#define ix_integration_sspAcc  41
#define ix_integration_i2c     42
#define ix_osal		       43
#define ix_integration_parityENAcc  44
#define ix_integration_timeSyncAcc  45

/***************************
 * timeSyncAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_timeSyncAcc)

#if defined (IX_OSSERV_VXWORKS_LE)

#define CSR_LE_DATA_COHERENT_MAPPING

#endif /* IX_OSSERV_VXWORKS_LE */

#endif /* timeSyncAcc */

/***************************
 * parityENAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_parityENAcc)

#if defined (IX_OSSERV_VXWORKS_LE)

#define CSR_LE_DATA_COHERENT_MAPPING

#endif /* IX_OSSERV_VXWORKS_LE */

#endif /* parityENAcc */

/***************************
 * codelets_parityENAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_codelets_parityENAcc)

#if defined (IX_OSSERV_VXWORKS_LE)

#define CSR_LE_DATA_COHERENT_MAPPING

#endif /* IX_OSSERV_VXWORKS_LE */

#endif /* codelets_parityENAcc */

#endif /* IxOsServicesComponents_H */

/***************************
 * integration_timeSyncAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_integration_timeSyncAcc)

#if defined (IX_OSSERV_VXWORKS_LE)

#define CSR_LE_DATA_COHERENT_MAPPING

#endif /* IX_OSSERV_VXWORKS_LE */

#endif /* integration_timeSyncAcc */

/***************************
 * integration_parityENAcc
 ***************************/
#if (IX_COMPONENT_NAME == ix_integration_parityENAcc)

#if defined (IX_OSSERV_VXWORKS_LE)

#define CSR_LE_DATA_COHERENT_MAPPING

#endif /* IX_OSSERV_VXWORKS_LE */

#endif /* integration_parityENAcc */