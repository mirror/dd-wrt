/**
 * @file IxOsalOsIxp400CustomizedMapping.h 
 *
 * @brief Set LE coherency modes for components. 
 *        The default setting is IX_OSAL_NO_MAPPING for LE.
 * 
 *
 *		  By default IX_OSAL_STATIC_MEMORY_MAP is defined for all the components.
 *		  If any component uses a dynamic memory map it must define
 *		  IX_OSAL_DYNAMIC_MEMORY_MAP in its corresponding section.
 *        
 *
 * @par
 * 
 * @par
 * IXP400 SW Release version 1.5
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2004 Intel Corporation All Rights Reserved.
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

#ifndef IxOsalOsIxp400CustomizedMapping_H
#define IxOsalOsIxp400CustomizedMapping_H

/*
 * only include this file in Little Endian
 */

#if defined (IX_OSAL_ECOS_BE)
#error Only include IxOsalOsIxp400CustomizedMapping.h in Little Endian
#endif

 /*
  * Components don't have to be in this list if
  * the default mapping is OK.
  */
#define ix_osal                1
#define ix_dmaAcc              2
#define ix_atmdAcc             3

#define ix_atmsch              5
#define ix_ethAcc              6
#define ix_npeMh               7
#define ix_qmgr                8
#define ix_npeDl               9
#define ix_atmm                10
#define ix_hssAcc              11
#define ix_ethDB               12
#define ix_ethMii              13
#define ix_timerCtrl           14
#define ix_adsl                15
#define ix_usb                 16
#define ix_uartAcc             17
#define ix_featureCtrl         18
#define ix_cryptoAcc           19
#define ix_unloadAcc           33
#define ix_perfProfAcc         34

#if (IX_COMPONENT_NAME == ix_qmgr)
#define IX_OSAL_LE_DC_MAPPING
#else
#define IX_OSAL_LE_AC_MAPPING
#endif

#endif /* IxOsalOsIxp400CustomizedMapping_H */


