/**
 * @file IxVersionId.h
 *
 * @date 22-Aug-2002
 *
 * @brief This file contains the IXP400 Software version identifier
 *
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

/**
 * @defgroup IxVersionId IXP400 Version ID (IxVersionId)
 *
 * @brief Version Identifiers
 *
 * @{
 */

#ifndef IXVERSIONID_H
#define IXVERSIONID_H

/**
 * @brief Version Identifier String
 *
 * This string will be updated with each customer release of the IXP400
 * Software.
 */
#define IX_VERSION_ID "2_0"

/**
 * This string will be updated with each customer release of the IXP400
 * ADSL driver package.
 */
#define IX_VERSION_ADSL_ID "1_12"


/**
 * This string will be updated with each customer release of the IXP400
 * USB Client driver package.
 */
#define IX_VERSION_USBRNDIS_ID "1_9"

/**
 * This string will be updated with each customer release of the IXP400
 * I2C Linux driver package.
 */
#define IX_VERSION_I2C_LINUX_ID "1_0"

/**
 * @brief Linux Ethernet Driver Patch Version Identifier String
 *
 * This string will be updated with each release of Linux Ethernet Patch
 */
#define LINUX_ETHERNET_DRIVER_PATCH_ID "1_4"

/**
 * @brief Linux Integration Patch Version Identifier String
 *
 * This String will be updated with each release of Linux Integration Patch
 */
#define LINUX_INTEGRATION_PATCH_ID "1_3"

/**
 * @brief Linux Ethernet Readme version Identifier String
 *
 * This string will be updated with each release of Linux Ethernet Readme
 */
#define LINUX_ETHERNET_README_ID "1_3"

/**
 * @brief Linux Integration Readme version Identifier String
 *
 * This string will be updated with each release of Linux Integration Readme
 */

#define LINUX_INTEGRATION_README_ID "1_3"

/**
 * @brief Linux I2C driver Readme version Identifier String
 *
 * This string will be updated with each release of Linux I2C Driver Readme
 */
#define LINUX_I2C_DRIVER_README_ID "1_0"

/**
 * @brief ixp425_eth_update_nf_bridge.patch version Identifier String
 *
 * This string will be updated with each release of ixp425_eth_update_nf_bridge.
patch
 *
 */

#define IXP425_ETH_UPDATE_NF_BRIDGE_ID "1_3"

/**
 * @brief Internal Release Identifier String
 *
 * This string will be updated with each internal release (SQA drop)
 * of the IXP400 Software.
 */
#define IX_VERSION_INTERNAL_ID "SQA3_5"

/**
 * @brief Compatible Tornado Version Identifier
 */
#define IX_VERSION_COMPATIBLE_TORNADO "Tornado2_2_1-PNE2_0"

/**
 * @brief Compatible Linux Version Identifier
 */
#define IX_VERSION_COMPATIBLE_LINUX "MVL3_1"


#endif /* IXVERSIONID_H */

/**
 * @} addtogroup IxVersionId
 */
