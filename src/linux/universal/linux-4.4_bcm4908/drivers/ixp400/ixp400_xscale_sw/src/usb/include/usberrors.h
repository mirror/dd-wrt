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

/** device is not received SET_CONFIGURATION message **/
#define IX_USB_DEVICE_NOT_CONFIGURED    (IX_USB_ERROR_BASE + 13) 

#endif /* usberrors_H */

/**
 * @} addtogroup IxUsbAPI 
 */
