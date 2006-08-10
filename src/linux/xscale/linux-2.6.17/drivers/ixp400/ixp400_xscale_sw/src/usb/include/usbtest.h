/**
 * @par
 * IXP400 SW Release Crypto version 2.3
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2005, Intel Corporation.
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
