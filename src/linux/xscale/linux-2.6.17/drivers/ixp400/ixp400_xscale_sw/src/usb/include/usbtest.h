/**
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
#ifndef usbtest_H

#define usbtest_H

typedef enum /* USBVendorCommands */
{
    /* 
     * Sets the device configuration 
     *
     * wValue[31:16] - mode
     * wValue[15:0]  - flags
     * wIndex[31:16] - in endpoint
     * wIndex[15:0]  - out endpoint
     */
    IXP425_CONFIG      = 0x0001,

    /* 
     * Requests the device to send data
     *
     * wValue - number of bytes to send
     */
    IXP425_SEND_DATA   = 0x0010,

    /*
     * Requests the device to report
     * the last number of received bytes
     *
     * WLength = 4
     * Data stage: requested value (4 bytes)
     */
    IXP425_SEND_REPORT = 0x0020,

    /*
     * Control no_data test
     *
     */
    IXP425_CTRL_NO_DATA = 0x0030,

    /*
     * Control Write test
     *
     * Data stage: sample data
     */
    IXP425_CTRL_WRITE = 0x0031,

    /*
     * Control read test
     *
     * Data stage: sample data
     *
     */
    IXP425_CTRL_READ = 0x0032,
    
    /*
     * Configures the loopback mode
     *
     * wValue - 1 to enable and 0 to disable loopback
     */
    IXP425_CONFIG_LOOPBACK = 0x0040
} USBVendorCommands;

typedef enum /* ControlPayloadType */
{
    UNKNOWN    = 0x00,
    DESCRIPTOR = 0x01,
    DATA       = 0x02
} ControlPayloadType;

typedef struct /* USBConfigData */
{
    UINT16 deviceAddress;
    UINT16 configurationIndex;
    UINT16 interfaceIndex;
    UINT16 alternateInterfaceIndex;
    BOOL remoteWakeupEnabled;
    UINT16 currentSetupCommand;

    UCHAR *usbConfigurationDescriptor;
    UINT8 inEndpoint;
    UINT8 outEndpoint;
    UINT8 currentMode;

#ifndef __HWEMU__
    sem_t isocFrameLock; 
#endif /* __HWEMU__ */

    UINT32 lastRxTransactionSize;

    UINT8 expectedControlPayload;

    BOOL loopbackEnabled;
    IX_USB_MBLK *responseBuffer;
} USBConfigData;

#endif /* usbtest_H */
