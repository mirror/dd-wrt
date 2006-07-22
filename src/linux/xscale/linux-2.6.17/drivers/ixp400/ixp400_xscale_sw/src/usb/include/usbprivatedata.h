/**
 * @file usbprivatedata.h
 *
 * @author Intel Corporation
 * @date 5-AUG-2003

 * @brief This file contains the public API of the IXP400 USB Driver
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


#ifndef usbprivatedata_H
#define usbprivatedata_H

/* Endpoint descriptions */
static int EPDescriptorTable[] =
{
    /* EP_0 */  USB_CONTROL     | USB_IN_OUT,
    /* EP_1 */  USB_BULK        | USB_IN,
    /* EP_2 */  USB_BULK        | USB_OUT,
    /* EP_3 */  USB_ISOCHRONOUS | USB_IN,
    /* EP_4 */  USB_ISOCHRONOUS | USB_OUT,
    /* EP_5 */  USB_INTERRUPT   | USB_IN,
    /* EP_6 */  USB_BULK        | USB_IN,
    /* EP_7 */  USB_BULK        | USB_OUT,
    /* EP_8 */  USB_ISOCHRONOUS | USB_IN,
    /* EP_9 */  USB_ISOCHRONOUS | USB_OUT,
    /* EP_10 */ USB_INTERRUPT   | USB_IN,
    /* EP_11 */ USB_BULK        | USB_IN,
    /* EP_12 */ USB_BULK        | USB_OUT,
    /* EP_13 */ USB_ISOCHRONOUS | USB_IN,
    /* EP_14 */ USB_ISOCHRONOUS | USB_OUT,
    /* EP_15 */ USB_INTERRUPT   | USB_IN
};

#endif  /* usbprivatedata_H */

