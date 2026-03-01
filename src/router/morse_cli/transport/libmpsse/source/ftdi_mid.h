/*
 * \file ftdi_mid.h
 *
 * \author FTDI
 * \date 20110323
 *
 * Copyright  2000-2014 Future Technology Devices International Limited
 *
 *
 * THIS SOFTWARE IS PROVIDED BY FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Project: libMPSSE
 * Module: MiddleLayer
 *
 * Revision History:
 * 0.1 - initial version
 * 0.2 - 20110708	Added functions FT_ReadGPIO & FT_WriteGPIO
 * 0.3 - 20111102	Added function Mid_GetFtDeviceType
 *				Modified function Mid_SetClock
 */

#ifndef FTDI_MID_H
#define FTDI_MID_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ftdi_common.h"	/*Common across I2C, SPI, JTAG modules*/
#include "ftdi_infra.h"		/*Common prrtable infrastructure(datatypes, libraries, etc)*/

#define MID_COMMENTED_AFTER_REVIEW_COMMENT  			0
#define MID_COMMENTED_AFTER_REVIEW_COMMENT_ENTRY_EXIT 	0

#define MID_NO_CHANNEL_FOUND	0

#define MID_NO_MPSSE					0
#define MID_MPSSE_AVAILABLE				1

#define MID_MAX_IN_BUF_SIZE         	4096

#define MID_ECHO_COMMAND_ONCE			0
#define MID_ECHO_COMMAND_CONTINUOUSLY   1
#define MID_ECHO_CMD_1					0xAA
#define MID_ECHO_CMD_2					0xAB
#define MID_BAD_COMMAND_RESPONSE        0xFA
#define MID_CMD_NOT_ECHOED				0
#define MID_CMD_ECHOED					1

/*clock*/
#define MID_SET_LOW_BYTE_DATA_BITS_CMD	0x80
#define MID_GET_LOW_BYTE_DATA_BITS_CMD	0x81
#define MID_SET_HIGH_BYTE_DATA_BITS_CMD	0x82
#define MID_GET_HIGH_BYTE_DATA_BITS_CMD	0x83
#define MID_SET_CLOCK_FREQUENCY_CMD		0x86
#define MID_SET_LOW_BYTE_DATA_BITS_DATA 0x13
#define MID_SET_HIGH_BYTE_DATA_BITS_DATA 0x0F

#define MID_6MHZ						6000000
#define MID_30MHZ						30000000

#define DISABLE_CLOCK_DIVIDE			0x8A
#define ENABLE_CLOCK_DIVIDE				0x8B

#define MID_LOOPBACK_FALSE				0
#define MID_LOOPBACK_TRUE				1
#define MID_TURN_ON_LOOPBACK_CMD		0x84
#define MID_TURN_OFF_LOOPBACK_CMD		0x85

#define MID_LEN_MAX_ERROR_STRING		500

#define MID_CHK_IN_BUF_OK(size)	{if (size > MID_MAX_IN_BUF_SIZE) \
	{ return FT_INSUFFICIENT_RESOURCES;}}

FT_STATUS FT_GetNumChannels(FT_LegacyProtocol Protocol, DWORD *numChans);
FT_STATUS FT_GetChannelInfo(FT_LegacyProtocol Protocol, DWORD index,
			FT_DEVICE_LIST_INFO_NODE *chanInfo);
FT_STATUS FT_OpenChannel(FT_LegacyProtocol Protocol, DWORD index,
			FT_HANDLE *handle);
FT_STATUS FT_InitChannel(FT_LegacyProtocol Protocol, FT_HANDLE handle,...);
FT_STATUS FT_CloseChannel(FT_LegacyProtocol Protocol, FT_HANDLE handle);
FT_STATUS FT_Channel_Read(FT_LegacyProtocol Protocol, FT_HANDLE handle,
				DWORD noOfBytes, uint8* buffer, LPDWORD noOfBytesTransferred);
FT_STATUS FT_Channel_Write(FT_LegacyProtocol Protocol, FT_HANDLE handle,
			DWORD noOfBytes, uint8* buffer, LPDWORD noOfBytesTransferred);
extern bool Mid_CheckMPSSEAvailable(FT_DEVICE_LIST_INFO_NODE);

extern FT_STATUS Mid_ResetDevice(FT_HANDLE handle);
extern FT_STATUS Mid_PurgeDevice (FT_HANDLE handle);
extern FT_STATUS Mid_SetUSBParameters(FT_HANDLE handle, DWORD inputBufSize,
										DWORD outputBufSize);
extern FT_STATUS Mid_SetDeviceSpecialChar(FT_HANDLE handle,
			UCHAR eventCh, UCHAR eventStatus,
			UCHAR errorCh, UCHAR errorStatus);
extern FT_STATUS Mid_SetDeviceTimeOut(FT_HANDLE handle,
										DWORD rdTimeOut, DWORD wrTimeOut);
extern FT_STATUS Mid_SetLatencyTimer (FT_HANDLE handle, UCHAR milliSecond);
extern FT_STATUS Mid_ResetMPSSE(FT_HANDLE handle);
extern FT_STATUS Mid_EnableMPSSEIn(FT_HANDLE handle);
extern FT_STATUS Mid_SyncMPSSE(FT_HANDLE handle);
extern FT_STATUS Mid_SendReceiveCmdFromMPSSE(FT_HANDLE handle, 
			UCHAR echoCmdFlag, UCHAR ecoCmd, UCHAR *cmdEchoed);
extern FT_STATUS Mid_SetGPIOLow(FT_HANDLE handle, uint8 value, uint8 direction);
extern FT_STATUS Mid_SetClock(FT_HANDLE handle, FT_DEVICE ftDevice, uint32 clock);
extern FT_STATUS Mid_GetFtDeviceType(FT_HANDLE handle, FT_DEVICE *ftDevice);
extern FT_STATUS Mid_SetDeviceLoopbackState(FT_HANDLE handle, uint8 loopBackFlag);
extern FT_STATUS Mid_EmptyDeviceInputBuff(FT_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif /* FTDI_MID_H */
