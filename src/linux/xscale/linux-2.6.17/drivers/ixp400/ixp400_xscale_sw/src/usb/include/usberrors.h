/**
 * @file usberrors.h
 *
 * @author Intel Corporation
 * @date 30-OCT-2001
 *
 * @brief This file containes USB driver error codes
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

/**
 * @addtogroup IxUsbAPI
 *
 * @brief Definition of extended USB error codes
 *
 * @{
 */

#ifndef usberrors_H

#ifndef __doxygen_HIDE

#define usberrors_H

#endif /* __doxygen_HIDE */

#ifndef IX_USB_ERROR_BASE
/** USB error base */ 
#define IX_USB_ERROR_BASE 4096   
#endif /* IX_USB_ERROR_BASE */

/** error due to unknown reasons */ 
#define IX_USB_ERROR		            (IX_USB_ERROR_BASE + 0)

/** invalid USBDevice structure passed as parameter or no device present */
#define IX_USB_INVALID_DEVICE 	        (IX_USB_ERROR_BASE + 1)

/** no permission for attempted operation */
#define IX_USB_NO_PERMISSION            (IX_USB_ERROR_BASE + 2)

/** redundant operation */
#define IX_USB_REDUNDANT                (IX_USB_ERROR_BASE + 3)

/** send queue full */
#define IX_USB_SEND_QUEUE_FULL          (IX_USB_ERROR_BASE + 4)

/** invalid endpoint */
#define IX_USB_NO_ENDPOINT              (IX_USB_ERROR_BASE + 5)

/** no IN capability on endpoint */
#define IX_USB_NO_IN_CAPABILITY	        (IX_USB_ERROR_BASE + 6)

/** no OUT capability on endpoint */
#define IX_USB_NO_OUT_CAPABILITY        (IX_USB_ERROR_BASE + 7)

/** transfer type incompatible with endpoint */
#define IX_USB_NO_TRANSFER_CAPABILITY   (IX_USB_ERROR_BASE + 8)

/** endpoint stalled */
#define IX_USB_ENDPOINT_STALLED         (IX_USB_ERROR_BASE + 9)

/** invalid parameter(s) */
#define IX_USB_INVALID_PARMS            (IX_USB_ERROR_BASE + 10)

/** device is disabled */
#define IX_USB_DEVICE_DISABLED          (IX_USB_ERROR_BASE + 11)

/** no STALL capability */
#define IX_USB_NO_STALL_CAPABILITY      (IX_USB_ERROR_BASE + 12)

#endif /* usberrors_H */

/**
 * @} addtogroup IxUsbAPI 
 */
