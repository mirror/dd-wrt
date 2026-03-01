/*!
 * \file libmpsse_i2c.h
 *
 * \author FTDI
 * \date 20110127
 *
 * Copyright � 2000-2014 Future Technology Devices International Limited
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
 * Module: I2C
 *
 * Revision History:
 * 0.1 - initial version
 * 0.2 - 20110708 - added macro I2C_DISABLE_3PHASE_CLOCKING
 * 0.3 - 20200428 - removed unnecessary files and directory structure
 *                  type of bool changed to unsigned int match WinTypes.h
 */

#ifndef FTDI_I2C_H
#define FTDI_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FTDIMPSSE_API
#ifdef _WIN32
// The following ifdef block is the standard way of creating macros
// which make exporting from a DLL simpler.  All files within this DLL
// are compiled with the FTDIMPSSE_EXPORTS symbol defined on the command line.
// This symbol should not be defined on any project that uses this DLL.
// This way any other project whose source files include this file see
// FTDIMPSSE_API functions as being imported from a DLL, whereas this DLL
// sees symbols defined with this macro as being exported.

#ifdef FTDIMPSSE_EXPORTS
#define FTDIMPSSE_API __declspec(dllexport)
#elif defined(FTDIMPSSE_STATIC)
// Avoid decorations when linking statically.
#define FTDIMPSSE_API
#else // FTDIMPSSE_EXPORTS
#define FTDIMPSSE_API __declspec(dllimport)
#endif // FTDIMPSSE_EXPORTS

#else // _WIN32

// Compiling on non-Windows platform.
#include "WinTypes.h"
// No decorations needed.
#define FTDIMPSSE_API

#endif // _WIN32
#endif // FTDIMPSSE_API

/******************************************************************************/
/*								Macro defines								  */
/******************************************************************************/

/** Options to I2C_DeviceWrite & I2C_DeviceRead */
/*Generate start condition before transmitting. */
#define	I2C_TRANSFER_OPTIONS_START_BIT		0x00000001

/*Generate stop condition before transmitting. */
#define I2C_TRANSFER_OPTIONS_STOP_BIT		0x00000002

/*Continue transmitting data in bulk without caring about Ack or nAck from device if this bit
is not set. If this bit is set then stop transferring the data in the buffer when the device
nACKs. */
#define I2C_TRANSFER_OPTIONS_BREAK_ON_NACK	0x00000004

/* libMPSSE-I2C generates an ACKs for every byte read. Some I2C slaves require the I2C
master to generate a nACK for the last data byte read. Setting this bit enables working with
such I2C slaves. */
#define I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE	0x00000008

/*Fast transfers prepare a buffer containing commands to generate START/STOP/ADDRESS
   conditions and commands to read/write data. This buffer is sent to the MPSSE in one shot,
   hence delays between different phases of the I2C transfer are eliminated. Fast transfers
   can have data length in terms of bits or bytes. The user application should call
   I2C_DeviceWrite or I2C_DeviceRead with either
   I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES or
   I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS bit set to perform a fast transfer.
   I2C_TRANSFER_OPTIONS_START_BIT and I2C_TRANSFER_OPTIONS_STOP_BIT have
   their usual meanings when used in fast transfers, however
   I2C_TRANSFER_OPTIONS_BREAK_ON_NACK and
   I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE are not applicable in fast transfers. */
#define I2C_TRANSFER_OPTIONS_FAST_TRANSFER		0x00000030/*not visible to user*/

/* When the user calls I2C_DeviceWrite or I2C_DeviceRead with this bit set then libMPSSE
	 packs commands to transfer sizeToTransfer number of bytes, and to read/write
	 sizeToTransfer number of ack bits. If data is written then the read ack bits are ignored, if
	 data is being read then an acknowledgement bit(SDA=LOW) is given to the I2C slave
	 after each byte read. */
#define I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES	0x00000010

/* When the user calls I2C_DeviceWrite or I2C_DeviceRead with this bit set then libMPSSE
	 packs commands to transfer sizeToTransfer number of bits. There is no ACK phase when
	 this bit is set. */
#define I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS	0x00000020

/* The address parameter is ignored in transfers if this bit is set. This would mean that
	 the address is either a part of the data or this is a special I2C frame that doesn't require
	 an address. However if this bit is not set then 7bit address and 1bit direction will be
	 written to the I2C bus each time I2C_DeviceWrite or I2C_DeviceRead is called and a
	 1bit acknowledgement will be read after that. */
#define I2C_TRANSFER_OPTIONS_NO_ADDRESS		0x00000040

#define I2C_CMD_GETDEVICEID_RD	0xF9
#define I2C_CMD_GETDEVICEID_WR	0xF8

#define I2C_GIVE_ACK	1
#define I2C_GIVE_NACK	0

/* 3-phase clocking is enabled by default. Setting this bit in ConfigOptions will disable it */
#define I2C_DISABLE_3PHASE_CLOCKING	0x0001

/******************************************************************************/
/*								Type defines								  */
/******************************************************************************/

/**
Valid range for clock divisor is 0 to 65535
Highest clock freq is 6MHz represented by 0
The next highest is   3MHz represented by 1
Lowest is            91Hz  represented by 65535

User can pass either pass I2C_DataRate_100K, I2C_DataRate_400K or
I2C_DataRate_3_4M for the standard clock rates
or a clock divisor value may be passed
*/
typedef enum I2C_ClockRate_t{
	I2C_CLOCK_STANDARD_MODE = 100000,		/* 100kb/sec */
	I2C_CLOCK_FAST_MODE = 400000,			/* 400kb/sec */
	I2C_CLOCK_FAST_MODE_PLUS = 1000000,		/* 1000kb/sec */
	I2C_CLOCK_HIGH_SPEED_MODE = 3400000		/* 3.4Mb/sec */
} I2C_CLOCKRATE;

/**/
typedef struct ChannelConfig_t
{
	I2C_CLOCKRATE	ClockRate; 
	/** There were 2 functions I2C_TurnOn/OffDivideByFive
	ClockinghiSpeedDevice (FTC_HANDLE fthandle) in the old DLL. This function turns on the
	divide by five for the MPSSE clock to allow the hi-speed devices FT2232H and FT4232H to
	clock at the same rate as the FT2232D device. This allows for backward compatibility
	NOTE: This feature is probably a per chip feature and not per device*/

	UCHAR			LatencyTimer; 
	/** Required value, in milliseconds, of latency timer.
	Valid range is 2 � 255
	In the FT8U232AM and FT8U245AM devices, the receive buffer timeout that is used to flush
	remaining data from the receive buffer was fixed at 16 ms. In all other FTDI devices, this
	timeout is programmable and can be set at 1 ms intervals between 2ms and 255 ms.  This
	allows the device to be better optimized for protocols requiring faster response times from
	short data packets
	NOTE: This feature is probably a per chip feature and not per device*/

	DWORD			Options;	
	/** This member provides a way to enable/disable features
	specific to the protocol that are implemented in the chip
	BIT0		: 3PhaseDataClocking - Setting this bit will turn on 3 phase data clocking for a
			FT2232H dual hi-speed device or FT4232H quad hi-speed device. Three phase
			data clocking, ensures the data is valid on both edges of a clock
	BIT1		: Loopback
	BIT2		: Clock stretching
	BIT3 -BIT31		: Reserved
	*/
} ChannelConfig;

/******************************************************************************/
/*								External variables							  */
/******************************************************************************/

/******************************************************************************/
/*								Function declarations						  */
/******************************************************************************/

/*!
 * \brief This function initializes libMPSSE
 *
 * This function is called once when the library is loaded. It initializes all the modules in the
 * library. This function initializes all the variables and data structures that are required to be
 * initialized once only during loading.
 *
 * \param[in] none
 * \param[out] none
 * \return none
 * \sa
 * \note May individually call Ftdi_I2C_Module_Init, Ftdi_SPI_Module_Init, Ftdi_Mid_Module_Init,
 * Ftdi_Common_Module_Init, etc if required. This function should be called by the OS specific
 * function(eg: DllMain for windows) that is called by the OS automatically during startup.
 * \warning
 */
FTDIMPSSE_API void Init_libMPSSE(void);

/*!
 * \brief Cleans up the module before unloading
 *
 * This function frees all the resources that were allocated during initialization. It should be called
 * by the OS to ensure that the module exits gracefully
 *
 * \param[in] none
 * \param[out] none
 * \return none
 * \sa
 * \note
 * \warning
 */
FTDIMPSSE_API void Cleanup_libMPSSE(void);

/*!
 * \brief Gets the number of I2C channels connected to the host
 *
 * This function gets the number of I2C channels that are connected to the host system
 * The number of ports available in each of these chips are different.
 *
 * \param[out] *numChannels Pointer to variable in which the no of channels will be returned
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa
 * \note This function doesn't return the number of FTDI chips connected to the host system
 * \note FT2232D has 1 MPSSE port
 * \note FT2232H has 2 MPSSE ports
 * \note FT4232H has 4 ports but only 2 of them have MPSSEs
 * so call to this function will return 2 if a FT4232 is connected to it.
 * \warning
 */
FTDIMPSSE_API FT_STATUS I2C_GetNumChannels(DWORD *numChannels);

/*!
 * \brief Provides information about channel
 *
 * This function takes a channel index (valid values are from 1 to the value returned by
 * I2C_GetNumChannels) and provides information about the channel in the form of a populated
 * ChannelInfo structure.
 *
 * \param[in] index Index of the channel
 * \param[out] chanInfo Pointer to FT_DEVICE_LIST_INFO_NODE structure(see D2XX \
 *			  Programmer's Guide)
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \note  The channel ID can be determined by the user from the last digit of the location ID
 * \sa
 * \warning
 */
FTDIMPSSE_API FT_STATUS I2C_GetChannelInfo(DWORD index,
	FT_DEVICE_LIST_INFO_NODE *chanInfo);

/*!
 * \brief Opens a channel and returns a handle to it
 *
 * This function opens the indexed channel and returns a handle to it
 *
 * \param[in] index Index of the channel
 * \param[out] handle Pointer to the handle of the opened channel
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa
 * \note Trying to open an already open channel will return an error code
 * \warning
 */
FTDIMPSSE_API FT_STATUS I2C_OpenChannel(DWORD index, FT_HANDLE *handle);

/*!
 * \brief Initializes a channel
 *
 * This function initializes the channel and the communication parameters associated with it
 *
 * \param[in] handle Handle of the channel
 * \param[out] config Pointer to ChannelConfig structure(memory to be allocated by caller)
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa
 * \note
 * \warning
 */
FTDIMPSSE_API FT_STATUS I2C_InitChannel(FT_HANDLE handle, ChannelConfig *config);

/*!
 * \brief Closes a channel
 *
 * Closes a channel and frees all resources that were used by it
 *
 * \param[in] handle Handle of the channel
 * \param[out] none
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa
 * \note
 * \warning
 */
FTDIMPSSE_API FT_STATUS I2C_CloseChannel(FT_HANDLE handle);

/*!
 * \brief Reads data from I2C slave
 *
 * This function reads the specified number of bytes from an addressed I2C slave
 *
 * \param[in] handle Handle of the channel
 * \param[in] deviceAddress Address of the I2C slave
 * \param[in] sizeToTransfer Number of bytes to be read
 * \param[out] buffer Pointer to the buffer where data is to be read
 * \param[out] sizeTransferred Pointer to variable containing the number of bytes read
 * \param[in] options This parameter specifies data transfer options. Namely, if a start/stop bits
 *			are required, if the transfer should continue or stop if device nAcks, etc
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa	Definitions of macros I2C_TRANSFER_OPTIONS_START_BIT,
 *		I2C_TRANSFER_OPTIONS_STOP_BIT, I2C_TRANSFER_OPTIONS_BREAK_ON_NACK,
 *		I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES,
 *		I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS &
 *		I2C_TRANSFER_OPTIONS_NO_ADDRESS
 * \note
 * \warning
 */
FTDIMPSSE_API FT_STATUS I2C_DeviceRead(FT_HANDLE handle, UCHAR deviceAddress,
	DWORD sizeToTransfer, UCHAR *buffer, LPDWORD sizeTransfered, DWORD options);

/*!
 * \brief Writes data from I2C slave
 *
 * This function writes the specified number of bytes from an addressed I2C slave
 *
 * \param[in] handle Handle of the channel
 * \param[in] deviceAddress Address of the I2C slave
 * \param[in] sizeToTransfer Number of bytes to be written
 * \param[out] buffer Pointer to the buffer from where data is to be written
 * \param[out] sizeTransferred Pointer to variable containing the number of bytes written
 * \param[in] options This parameter specifies data transfer options. Namely if a start/stop bits
 *			are required, if the transfer should continue or stop if device nAcks, etc
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa	Definitions of macros I2C_TRANSFER_OPTIONS_START_BIT,
 *		I2C_TRANSFER_OPTIONS_STOP_BIT, I2C_TRANSFER_OPTIONS_BREAK_ON_NACK,
 *		I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES,
 *		I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS &
 *		I2C_TRANSFER_OPTIONS_NO_ADDRESS
 * \note
 * \warning
 */
FTDIMPSSE_API FT_STATUS I2C_DeviceWrite(FT_HANDLE handle, UCHAR deviceAddress,
	DWORD sizeToTransfer, UCHAR *buffer, LPDWORD sizeTransfered, DWORD options);

/*!
 * \brief Get the I2C device ID
 *
 * This function retrieves the I2C device ID. It may not be enabled in the library
 * depending on build configuration. If it is not enabled then it will return
 * FT_NOT_SUPPORTED.
 *
 * \param[in] handle Handle of the channel
 * \param[in] deviceAddress Address of the I2C slave
 * \param[out] deviceID Address of memory where the 3byte I2C device ID will be stored
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa
 * \note
 * \warning
 */
FTDIMPSSE_API FT_STATUS I2C_GetDeviceID(FT_HANDLE handle, UCHAR deviceAddress,
	UCHAR *deviceID);

/*!
 * \brief Writes to the 8 GPIO lines
 *
 * Writes to the 8 GPIO lines associated with the high byte of the MPSSE channel
 *
 * \param[in] handle Handle of the channel
 * \param[in] dir The direction of the 8 lines. 0 for in and 1 for out
 * \param[in] value Output state of the 8 GPIO lines
 * \return status
 * \sa
 * \note
 * \warning
 */
FTDIMPSSE_API FT_STATUS FT_WriteGPIO(FT_HANDLE handle, UCHAR dir, UCHAR value);

/*!
 * \brief Reads from the 8 GPIO lines
 *
 * This function reads the GPIO lines associated with the high byte of the MPSSE channel
 *
 * \param[in] handle Handle of the channel
 * \param[out] *value Input state of the 8 GPIO lines(1 = high)
 * \return status
 * \sa
 * \note The directions of the GPIO pins have to be first set to input mode using FT_WriteGPIO
 * \warning
 */
FTDIMPSSE_API FT_STATUS FT_ReadGPIO(FT_HANDLE handle, UCHAR *value);

/******************************************************************************/

/*!
 * \brief Version Number Function
 *
 * Returns libMPSSE and libFTD2XX version number
 *
 * \param[out]  *libmpsse	MPSSE version number is returned
 * \param[out]  *libftd2xx	D2XX version number is returned
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa
 * \warning
 */
FTDIMPSSE_API FT_STATUS Ver_libMPSSE(LPDWORD libmpsse, LPDWORD libftd2xx);

#ifdef __cplusplus
}
#endif

#endif	/*FTDI_I2C_H*/

