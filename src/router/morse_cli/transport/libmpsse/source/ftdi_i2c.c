/*!
 * \file ftdi_i2c.c
 *
 * \author FTDI
 * \date 20110321
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
 * Module: I2C
 *
 * Revision History:
 * 0.1 - initial version
 * 0.2 - 20110708 - Added 3-phase-clocking functionality in I2C_InitChannel
 * 0.3 - 20111103 - Added I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE
 *				  Added I2C_TRANSFER_OPTIONS_FAST_TRANSFER(bit/byte/noAddress modes)
 * 				  Returns FT_DEVICE_NOT_FOUND if addressed slave doesn't respond
 *				  Adjustment to clock rate if 3-phase-clocking is enabled
  * 0.4 - 20200428 - removed unnecessary files and directory structure
*/

/******************************************************************************/
/*								Include files					  			  */
/******************************************************************************/
#define FTDI_EXPORTS
#include "ftdi_infra.h"		/*Common portable infrastructure(datatypes, libraries, etc)*/
#include "ftdi_common.h"	/*Common across I2C, SPI, JTAG modules*/
#include "ftdi_mid.h"		/*Middle layer*/
#include "libmpsse_i2c.h"	/*I2C specific*/


/******************************************************************************/
/*								Macro and type defines					  		  */
/******************************************************************************/

/* Enabling the macro will lead to checking of all parameters that are passed to the library from
    the user application*/
#define ENABLE_PARAMETER_CHECKING

/* This macro will enable all code that is needed to support I2C_GETDEVICEID feature */
#undef I2C_CMD_GETDEVICEID_SUPPORTED

/* This macro will enable the code to read acknowledgements from slaves in I2C_RawWrite */
#define FASTWRITE_READ_ACK

#define START_DURATION_1	10
#define START_DURATION_2	20

#define STOP_DURATION_1	10
#define STOP_DURATION_2	10
#define STOP_DURATION_3	10

#define SEND_ACK			0x00
#define SEND_NACK			0x80

#define I2C_ADDRESS_READ_MASK	0x01	/*LSB 1 = Read*/
#define I2C_ADDRESS_WRITE_MASK	0xFE	/*LSB 0 = Write*/

#ifdef I2C_CMD_GETDEVICEID_SUPPORTED

/* This enum lists the supported I2C modes*/
typedef enum I2C_Modes_t{
	I2C_STANDARD_MODE = 0,
	I2C_FAST_MODE,
	I2C_FAST_MODE_PLUS,
	I2C_HIGH_SPEED_MODE,
	I2C_MAXIMUM_SUPPORTED_MODES
} I2C_Modes;

/* This enum lists the various I2C bus condition*/
typedef enum I2C_Bus_Condition_t{
	I2C_CONDITION_PRESTART,
	I2C_CONDITION_START,
	I2C_CONDITION_POSTSTART,
	I2C_CONDITION_PRESTOP,
	I2C_CONDITION_STOP,
	I2C_CONDITION_POSTSTOP,
	I2C_MAXIMUM_SUPPORTED_CONDITIONS
} I2C_Bus_Condition;

#endif // I2C_CMD_GETDEVICEID_SUPPORTED

/******************************************************************************/
/*								Local function declarations					  */
/******************************************************************************/

#ifdef I2C_CMD_GETDEVICEID_SUPPORTED
/*!
 * \brief Generate I2C bus restart condition
 *
 * This function generates the restart condition in the I2C bus
 *
 * \param[in] handle Handle of the channel
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa
 * \note
 * \warning
 */
static FT_STATUS I2C_Restart(FT_HANDLE handle);
#endif // I2C_CMD_GETDEVICEID_SUPPORTED

/*!
 * \brief Writes 8 bits and gets the ack bit
 *
 * This function writes 8 bits of data to the I2C bus and gets the ack bit from the device
 *
 * \param[in] handle Handle of the channel
 * \param[in] data The 8bits of data that are to be written to the I2C bus
 * \param[out] ack The acknowledgment bit returned by the I2C device
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa
 * \note
 * \warning
 */
static FT_STATUS I2C_Write8bitsAndGetAck(FT_HANDLE handle, uint8 data, bool *ack);

/*!
 * \brief Reads 8 bits of data and sends ack bit
 *
 * This function reads 8 bits of data from the I2C bus and then writes an ack bit to the bus
 *
 * \param[in] handle Handle of the channel
 * \param[in] *data Pointer to the buffer where the 8bits would be read to
 * \param[in] ack Gives ack to device if set, otherwise gives nAck
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa
 * \note
 * \warning
 */
static FT_STATUS I2C_Read8bitsAndGiveAck(FT_HANDLE handle, uint8 *data, bool ack);

/*!
 * \brief Write I2C device address
 *
 * This function writes the direction and address bits to the I2C bus, and then gets the ACK
 *
 * \param[in] handle Handle of the channel
 * \param[in] deviceAddress Address of the I2C device
 * \param[in] direction 0 = Write; 1 = Read
 * \param[in] AddLen10Bit Setting this bit specifies 10bit addressing, otherwise 7bit
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa
 * \note
 * \warning
 */
static FT_STATUS I2C_WriteDeviceAddress(FT_HANDLE handle, UCHAR deviceAddress,
			bool direction, bool AddLen10Bit, bool *ack);

/*!
 * \brief Saves the channel's configuration data
 *
 * This function saves the channel's configuration data
 *
 * \param[in] handle Handle of the channel
 * \param[in] config Pointer to ChannelConfig structure(memory to be allocated by caller)
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa
 * \note
 * \warning
 */
static FT_STATUS I2C_SaveChannelConfig(FT_HANDLE handle, ChannelConfig *config);

#ifdef I2C_CMD_GETDEVICEID_SUPPORTED
/*!
 * \brief Retrieves channel's configuration data
 *
 * This function retrieves the channel's configuration data that was previously saved
 *
 * \param[in] handle Handle of the channel
 * \param[in] config Pointer to ChannelConfig structure(memory to be allocated by caller)
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa
 * \note
 * \warning
 */
static FT_STATUS I2C_GetChannelConfig(FT_HANDLE handle, ChannelConfig *config);
#endif // I2C_CMD_GETDEVICEID_SUPPORTED

/*!
 * \brief Generates the I2C Start condition
 *
 * This function generates the I2C start condition on the bus.
 *
 * \param[in] handle Handle of the channel
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa
 * \note
 * \warning
 */
static FT_STATUS I2C_Start(FT_HANDLE handle);

/*!
 * \brief Generates the I2C Stop condition
 *
 * This function generates the I2C stop condition on the bus.
 *
 * \param[in] handle Handle of the channel
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa
 * \note
 * \warning
 */
static FT_STATUS I2C_Stop(FT_HANDLE handle);

/*!
 * \brief This function generates the START, ADDRESS, DATA(write) & STOP phases in the I2C
 *		bus without having delays between these phases
 *
 * This function allocates memory locally, makes MPSSE command frames to write each data
 * byte/bit, makes MPSSE command frames to read the acknowledgement bits, and then writes
 * all these to the MPSSE in one shot. This function is useful where delays between START, DATA
 * and STOP phases are not prefered.
 *
 * \param[in] handle Handle of the channel
 * \param[in] deviceAddress Address of the I2C Slave. This parameter is ignored if flag
 *			I2C_TRANSFER_OPTIONS_NO_ADDRESS is set in the options parameter
 * \param[in] sizeToTransfer Number of bytes or bits to be written, depending on if
 *			I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS or
 *			I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES is set in options parameter
 * \param[in] *buffer Pointer to the buffer from where data is to be written. The user application
 * 			is expected to send a byte array of length sizeToTransfer. However if
 *			I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS is set then the length of the byte
 *			array should be sizeToTransfer/8.
 * \param[out] *ack Pointer to the buffer to where the ack bits are to be stored. The ack bits are
 *			 ignored if NULL is passed
 * \param[out] sizeTransferred Pointer to variable containing the number of bytes/bits written
 * \param[in] options This parameter specifies data transfer options. Namely if a start/stop
 *			conditions are required, if size is in bytes or bits, if address is provided.
 * \return 	Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa		Definitions of I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES,
 *			I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS,
 *			I2C_TRANSFER_OPTIONS_NO_ADDRESS,
 *			I2C_TRANSFER_OPTIONS_START_BIT,
 *			I2C_TRANSFER_OPTIONS_STOP_BIT
 * \note The I2C_TRANSFER_OPTIONS_BREAK_ON_NACK bit in the options parameter is not
 *          applicable for this function.
 * \warning
 */
static FT_STATUS I2C_FastWrite(FT_HANDLE handle, UCHAR deviceAddress,
			DWORD bitsToTransfer, UCHAR *buffer, UCHAR *ack, LPDWORD bytesTransferred,
			uint32 options);

/*!
 * \brief This function generates the START, ADDRESS, DATA(read) & STOP phases in the I2C
 *		bus without having delays between these phases
 *
 * This function allocates memory locally, makes MPSSE command frames to read each data
 * byte/bit, makes MPSSE command frames to write the acknowledgement bits, and then writes
 * all these to the MPSSE in one shot. This function is useful where delays between START, DATA
 * and STOP phases are not prefered.
 *
 * \param[in] handle Handle of the channel
 * \param[in] deviceAddress Address of the I2C Slave. This parameter is ignored if flag
 *			I2C_TRANSFER_OPTIONS_NO_ADDRESS is set in the options parameter
 * \param[in] sizeToTransfer Number of bytes or bits to be written, depending on if
 *			I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS or
 *			I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES is set in options parameter
 * \param[in] *buffer Pointer to the buffer to where data is to be read. The user application
 * 			is expected to send a byte array of length sizeToTransfer. However if
 *			I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS is set then the length of the byte
 *			array should be sizeToTransfer/8
 * \param[out] *ack Reserved (place holder for buffer in which user can provide ack/nAck bits)
 * \param[out] sizeTransferred Pointer to variable containing the number of bytes/bits written
 * \param[in] options This parameter specifies data transfer options. Namely if a start/stop
 *			conditions are required, if size is in bytes or bits, if address is provided.
 * \return 	Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa		Definitions of I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES,
 *			I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS,
 *			I2C_TRANSFER_OPTIONS_NO_ADDRESS,
 *			I2C_TRANSFER_OPTIONS_START_BIT,
 *			I2C_TRANSFER_OPTIONS_STOP_BIT
 * \note The I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE bit in the options parameter is not
 *          applicable for this function.
 * \warning
 */
static FT_STATUS I2C_FastRead(FT_HANDLE handle, UCHAR deviceAddress,
			DWORD bitsToTransfer, UCHAR *buffer, UCHAR *ack, LPDWORD bytesTransferred,
			uint32 options);


/******************************************************************************/
/*								Global variables							  */
/******************************************************************************/

#ifdef I2C_CMD_GETDEVICEID_SUPPORTED
/*!
 * \brief I2C bus condition timings table
 *
 * This table contains the minimim time that the I2C bus needs to be held in, in order to register
 * a condition such as start, stop restart, etc. The table has one row each for the different bus
 * speeds.
 *
 * \sa
 * \note
 * \warning
 */
const uint64 I2C_Timings[I2C_MAXIMUM_SUPPORTED_MODES]
[I2C_MAXIMUM_SUPPORTED_CONDITIONS] = {
	/*Durations for conditions are as follows:
	pre-start, start, post-start, pre-stop, stop, post-stop */
	{0, 0,0, 0,0, 0,}, /* I2C_CLOCK_STANDARD_MODE */
	{0, 0,0, 0,0, 0,}, /*I2C_CLOCK_FAST_MODE */
	{0, 0,0, 0,0, 0,}, /* I2C_CLOCK_FAST_MODE_PLUS */
	{0, 0,0, 0,0, 0}	/* I2C_CLOCK_HIGH_SPEED_MODE */
};
#endif // I2C_CMD_GETDEVICEID_SUPPORTED


/******************************************************************************/
/*						Public function definitions						  */
/******************************************************************************/

FTDIMPSSE_API FT_STATUS I2C_GetNumChannels(DWORD *numChannels)
{
	FT_STATUS status;

	FN_ENTER;
	
#ifdef ENABLE_PARAMETER_CHECKING
	CHECK_NULL_RET(numChannels);
#endif // ENABLE_PARAMETER_CHECKING
	status = FT_GetNumChannels(I2C, numChannels);
	CHECK_STATUS(status);
	FN_EXIT;
	return status;
}

FTDIMPSSE_API FT_STATUS I2C_GetChannelInfo(DWORD index,
					FT_DEVICE_LIST_INFO_NODE *chanInfo)
{
	FT_STATUS status;
	
	FN_ENTER;
#ifdef ENABLE_PARAMETER_CHECKING
	CHECK_NULL_RET(chanInfo);
#endif // ENABLE_PARAMETER_CHECKING
	status = FT_GetChannelInfo(I2C, index+1, chanInfo);
	CHECK_STATUS(status);
	FN_EXIT;
	return status;
}

FTDIMPSSE_API FT_STATUS I2C_OpenChannel(DWORD index, FT_HANDLE *handle)
{
	FT_STATUS status;

	FN_ENTER;
#ifdef ENABLE_PARAMETER_CHECKING
	CHECK_NULL_RET(handle);
#endif // ENABLE_PARAMETER_CHECKING
	/* Opens a channel and returns the pointer to its handle */
	status = FT_OpenChannel(I2C, index+1, handle);
	DBG(MSG_DEBUG,"index=%u handle=%u\n",(unsigned)index,(unsigned)*handle);
	CHECK_STATUS(status);
	FN_EXIT;
	return status;
}

FTDIMPSSE_API FT_STATUS I2C_InitChannel(FT_HANDLE handle, ChannelConfig *config)
{
	FT_STATUS status;
	uint8 buffer[3];//3
	uint32 noOfBytesToTransfer;
	DWORD noOfBytesTransferred;
	FN_ENTER;
#ifdef ENABLE_PARAMETER_CHECKING
	CHECK_NULL_RET(config);
	CHECK_NULL_RET(handle);
#endif // ENABLE_PARAMETER_CHECKING
	if (!(config->Options & I2C_DISABLE_3PHASE_CLOCKING))
	{/* Adjust clock rate if 3phase clocking should be enabled */
		config->ClockRate = (config->ClockRate * 3)/2;
	}
	DBG(MSG_DEBUG,"handle = 0x%x ClockRate=%u LatencyTimer=%u Options = 0x%x\n",
		(unsigned)handle, (unsigned)config->ClockRate,
		(unsigned)config->LatencyTimer,(unsigned)config->Options);
	status = FT_InitChannel(I2C, handle, (uint32)config->ClockRate,
		(uint32)config->LatencyTimer,(uint32)config->Options);
	CHECK_STATUS(status);

	if (!(config->Options & I2C_DISABLE_3PHASE_CLOCKING))
	{
		DBG(MSG_DEBUG,"Enabling 3 phase clocking\n");
		noOfBytesToTransfer = 1;
		noOfBytesTransferred = 0;
		buffer[0] = MPSSE_CMD_ENABLE_3PHASE_CLOCKING;/* MPSSE command */
		status = FT_Channel_Write(I2C, handle, noOfBytesToTransfer,
			buffer,&noOfBytesTransferred);
		CHECK_STATUS(status);
	}

	/*Save the channel's config data for later use*/
	status = I2C_SaveChannelConfig(handle, config);
	CHECK_STATUS(status);
	FN_EXIT;
	return status;
}

FTDIMPSSE_API FT_STATUS I2C_CloseChannel(FT_HANDLE handle)
{
	FT_STATUS status;
	FN_ENTER;
#ifdef ENABLE_PARAMETER_CHECKING
		CHECK_NULL_RET(handle);
#endif // ENABLE_PARAMETER_CHECKING
	status = FT_CloseChannel(I2C, handle);
	CHECK_STATUS(status);
	FN_EXIT;
	return status;
}

FTDIMPSSE_API FT_STATUS I2C_DeviceRead(FT_HANDLE handle, UCHAR deviceAddress,
	DWORD sizeToTransfer, UCHAR *buffer, LPDWORD sizeTransferred, DWORD options)
{
	FT_STATUS status = FT_OK;
	bool ack = TRUE;
	uint32 i;

	FN_ENTER;
	
#ifdef ENABLE_PARAMETER_CHECKING
	CHECK_NULL_RET(handle);
	CHECK_NULL_RET(buffer);
	CHECK_NULL_RET(sizeTransferred);
	
	if (deviceAddress > 127)
	{
		DBG(MSG_ERR,"deviceAddress(0x%x) is greater than 127\n", (unsigned)deviceAddress);
		return FT_INVALID_PARAMETER;
	}
#endif // ENABLE_PARAMETER_CHECKING

	LOCK_CHANNEL(handle);
	if (options & I2C_TRANSFER_OPTIONS_FAST_TRANSFER)
	{
		status = I2C_FastRead (handle, deviceAddress, sizeToTransfer, 
			buffer, NULL, sizeTransferred, options);
	}
	else
	{
		/* Write START bit */
		if (options & I2C_TRANSFER_OPTIONS_START_BIT)
		{
			status = I2C_Start(handle);
			CHECK_STATUS(status);
		}

		if (!(options & I2C_TRANSFER_OPTIONS_NO_ADDRESS))
		{
		/* Write device address (with LSB = 1 => READ)  & Get ACK */
		status = I2C_WriteDeviceAddress(handle, deviceAddress, TRUE, FALSE,&ack);
		CHECK_STATUS(status);
		}
		else
		{
			ack = 0;
		}

		if (!ack) /* check acknowledgement of device address write */
		{
			for (i = 0; (i < sizeToTransfer) && (status == FT_OK); i++)
			{
				/* Read byte to buffer & give ACK (or nACK if it is last byte and
				I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE is set)*/
				status = I2C_Read8bitsAndGiveAck(handle, &(buffer[i]),
					(i<(sizeToTransfer-1))?TRUE:
				((options & I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE)?FALSE:TRUE));
			}

			*sizeTransferred = i;
			if (*sizeTransferred != sizeToTransfer)
			{
				DBG(MSG_ERR," sizeToTransfer=%u sizeTransferred=%u\n",
					(unsigned)sizeToTransfer, (unsigned)*sizeTransferred);
				status = FT_IO_ERROR;
			}
			else
			{
				/* Write STOP bit */
				if (options & I2C_TRANSFER_OPTIONS_STOP_BIT)
				{
					status = I2C_Stop(handle);
					CHECK_STATUS(status);
				}
			}
		}
		else
		{
			DBG(MSG_ERR,"I2C device with address 0x%x didn't ack when addressed\n",
				(unsigned)deviceAddress);
			/* Write STOP bit */
			if (options & I2C_TRANSFER_OPTIONS_STOP_BIT)
			{
				status = I2C_Stop(handle);
				CHECK_STATUS(status);
			}
			/*20111102 : FT_IO_ERROR was returned when a device doesn't respond to the
	 		master when it is addressed, as well as when a data transfer fails. To distinguish
	 		between these to errors, FT_DEVICE_NOT_FOUND is now returned after a device
	 		doesn't respond when its addressed*/
			/* old code: status = FT_IO_ERROR; */
			status = FT_DEVICE_NOT_FOUND;
		}
	}
	UNLOCK_CHANNEL(handle);
	FN_EXIT;
	return status;
}

FTDIMPSSE_API FT_STATUS I2C_DeviceWrite(FT_HANDLE handle, UCHAR deviceAddress,
	DWORD sizeToTransfer, UCHAR *buffer, LPDWORD sizeTransferred, DWORD options)
{
	FT_STATUS status = FT_OK;
	bool ack = FALSE;
	uint32 i;
	FN_ENTER;
#ifdef ENABLE_PARAMETER_CHECKING
	CHECK_NULL_RET(handle);
	CHECK_NULL_RET(buffer);
	CHECK_NULL_RET(sizeTransferred);
	if ((deviceAddress > 127) && (!(options & I2C_TRANSFER_OPTIONS_NO_ADDRESS)))
	{
		DBG(MSG_WARN,"deviceAddress(0x%x) is greater than 127\n", 
			(unsigned)deviceAddress);
	}
#endif // ENABLE_PARAMETER_CHECKING
	DBG(MSG_DEBUG,"handle = 0x%x deviceAddress = 0x%x sizeToTransfer=%u options= 0x%x\n",
		(unsigned)handle, (unsigned)deviceAddress, 
		(unsigned)sizeToTransfer, (unsigned)options);

	LOCK_CHANNEL(handle);
	Mid_PurgeDevice(handle);

	if (options & I2C_TRANSFER_OPTIONS_FAST_TRANSFER)
	{
		status = I2C_FastWrite(handle, deviceAddress, sizeToTransfer, buffer,
			NULL, sizeTransferred, options);
	}
	else
	{
		/* Write START bit */
		if (options & I2C_TRANSFER_OPTIONS_START_BIT)
		{
			status = I2C_Start(handle);
			CHECK_STATUS(status);
		}

		if (!(options & I2C_TRANSFER_OPTIONS_NO_ADDRESS))
		{
		/* Write device address (with LSB = 0 => Write) & Get ACK*/
		status = I2C_WriteDeviceAddress(handle, deviceAddress, FALSE, FALSE,&ack);
		CHECK_STATUS(status);
		}
		else
		{
			ack = 0;
		}

		if (!ack)/*ack bit set actually means device nAcked*/
		{
			/* LOOP until sizeToTransfer */
			for (i = 0; ((i < sizeToTransfer) && (status == FT_OK)); i++)
			{
				/* Write byte to buffer & Get ACK */
				ack = 0;
				status = I2C_Write8bitsAndGetAck(handle, buffer[i],&ack);
				DBG(MSG_DEBUG,"handle = 0x%x buffer[%u] = 0x%x ack = 0x%x \n", 
					(unsigned)handle, (unsigned)i, (unsigned)buffer[i],
					(unsigned)(1&ack));
				if (ack)
				{
					DBG(MSG_WARN,"I2C device(address 0x%x) nAcked while writing	byte no %d(i.e. 0x%x\n",
						(unsigned)deviceAddress, (int)i, (unsigned)buffer[i]);
					/* add bit in options to return with error if device nAcked
					sizeTransferred = number of correctly transfered bytes */
					if (options & I2C_TRANSFER_OPTIONS_BREAK_ON_NACK)
					{
						/*status = FT_FAILED_TO_WRITE_DEVICE;
						break;*/
						DBG(MSG_WARN,"returning FT_FAILED_TO_WRITE_DEVICE options = 0x%x ack = 0x%x\n", 
							options, ack);
						
						/* Write STOP bit */
						if (options & I2C_TRANSFER_OPTIONS_STOP_BIT)
						{
							status = I2C_Stop(handle);
							CHECK_STATUS(status);
						}
						return FT_FAILED_TO_WRITE_DEVICE;
					}
				}
			}
			*sizeTransferred = i;
			if (*sizeTransferred != sizeToTransfer)
			{
				DBG(MSG_ERR," sizeToTransfer=%u sizeTransferred=%u\n",
					(unsigned)sizeToTransfer, (unsigned)*sizeTransferred);
				status = FT_IO_ERROR;
			}
			else
			{
				/* Write STOP bit */
				if (options & I2C_TRANSFER_OPTIONS_STOP_BIT)
				{
					status = I2C_Stop(handle);
					CHECK_STATUS(status);
				}
			}
		}
		else
		{
			DBG(MSG_ERR,"I2C device with address 0x%x didn't ack when addressed\n",
				(unsigned)deviceAddress);

			/* Write STOP bit */
			if (options & I2C_TRANSFER_OPTIONS_STOP_BIT)
			{
				status = I2C_Stop(handle);
				CHECK_STATUS(status);
			}
			/*20111102 : libMPSSE v0.2 returned FT_IO_ERROR both when a device doesn't
			respond to the master when it is addressed, and when a data transfer fails. To
			distinguish between these to errors, FT_DEVICE_NOT_FOUND is now returned after
			a device doesn't respond when its addressed*/
			status = FT_DEVICE_NOT_FOUND;
			/* old code: status = FT_IO_ERROR; */
		}
	}
	UNLOCK_CHANNEL(handle);
	FN_EXIT;
	return status;
}

FTDIMPSSE_API FT_STATUS I2C_GetDeviceID(FT_HANDLE handle, uint8 deviceAddress,
	uint8* deviceID)
{
	FT_STATUS status = FT_OTHER_ERROR;

#ifdef I2C_CMD_GETDEVICEID_SUPPORTED
	bool ack;
	FN_ENTER;

#ifdef ENABLE_PARAMETER_CHECKING
	CHECK_NULL_RET(handle);
	CHECK_NULL_RET(deviceID);
	
	if (deviceAddress > 127)
	{
		DBG(MSG_WARN,"deviceAddress(0x%x) is greater than 127\n", 
			(unsigned)deviceAddress);
		status = FT_INVALID_PARAMETER;
		return status;
	}
#endif // ENABLE_PARAMETER_CHECKING

	status = I2C_Start(handle);
	CHECK_STATUS(status);
	status = I2C_Write8bitsAndGetAck(handle,(uint8)I2C_CMD_GETDEVICEID_RD,&ack);
	CHECK_STATUS(status);
	status = I2C_Write8bitsAndGetAck(handle, deviceAddress, &ack);
	CHECK_STATUS(status);
	status = I2C_Restart(handle);
	CHECK_STATUS(status);
	status = I2C_Write8bitsAndGetAck(handle,(uint8)I2C_CMD_GETDEVICEID_WR,&ack);
	CHECK_STATUS(status);
	status = I2C_Read8bitsAndGiveAck(handle,&(deviceID[0]),I2C_GIVE_ACK);
	CHECK_STATUS(status);
	status = I2C_Read8bitsAndGiveAck(handle,&(deviceID[1]),I2C_GIVE_ACK);
	CHECK_STATUS(status);
	/*NACK 3rd byte*/
	status = I2C_Read8bitsAndGiveAck(handle,&(deviceID[2]),I2C_GIVE_NACK);
	CHECK_STATUS(status);
	FN_EXIT;

#else // I2C_CMD_GETDEVICEID_SUPPORTED
	
	FN_ENTER;
	status = FT_NOT_SUPPORTED;
	FN_EXIT;

#endif // I2C_CMD_GETDEVICEID_SUPPORTED

	return status;
}

/******************************************************************************/
/*						Local function definitions						  */
/******************************************************************************/
#ifdef I2C_CMD_GETDEVICEID_SUPPORTED
static FT_STATUS I2C_Restart(FT_HANDLE handle)
{
	FT_STATUS status;
	uint8 buffer[3];
	uint32 noOfBytesToTransfer;
	DWORD noOfBytesTransferred;
	I2C_Modes mode;

	FN_ENTER;

#if 0 /* Not necessary */
	status = I2C_GetChannelConfig(handle, config);
	CHECK_STATUS(status);
	CHECK_NULL_RET(config);
	switch(config->ClockRate)
	{
		case I2C_CLOCK_STANDARD_MODE:
			mode = I2C_STANDARD_MODE;
		break;

		case I2C_CLOCK_FAST_MODE:
			mode = I2C_FAST_MODE;
		break;

		case I2C_CLOCK_FAST_MODE_PLUS:
			mode = I2C_FAST_MODE_PLUS;
		break;

		case I2C_CLOCK_HIGH_SPEED_MODE:
			mode = I2C_HIGH_SPEED_MODE;
		break;

		default:
			mode = I2C_MAXIMUM_SUPPORTED_MODES;
	}
#else
	mode = I2C_STANDARD_MODE;
#endif

	/*Restart condition SDA, SCL: 0, 1->1, 1->1, 0->1, 1->0, 1*/

	/*I2C_CONDITION_RESTART_1*/
	noOfBytesToTransfer = 3;
	noOfBytesTransferred = 0;
	buffer[0] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
	buffer[1] = VALUE_SCLHIGH_SDALOW; /* Value */
	buffer[2] = DIRECTION_SCLOUT_SDAOUT; /* Direction */
	status = FT_Channel_Write(I2C, handle, noOfBytesToTransfer,
		buffer,&noOfBytesTransferred);
	if ( (FT_OK != status) && (noOfBytesToTransfer != noOfBytesTransferred) )
	{
		Infra_DbgPrintStatus(status);
		DBG(MSG_ERR,"noOfBytesToTransfer=%d noOfBytesTransferred=%d\n",
			(int)noOfBytesToTransfer,(int)noOfBytesTransferred);
	}
	Infra_Delay(I2C_Timings[mode][I2C_CONDITION_PRESTART]);

	/*I2C_CONDITION_RESTART_2*/
	noOfBytesToTransfer = 3;
	noOfBytesTransferred = 0;
	buffer[0] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
	buffer[1] = VALUE_SCLHIGH_SDAHIGH; /* Value */
	buffer[2] = DIRECTION_SCLOUT_SDAOUT; /* Direction */
	status = FT_Channel_Write(I2C, handle, noOfBytesToTransfer,
		buffer,&noOfBytesTransferred);
	if ( (FT_OK != status) && (noOfBytesToTransfer != noOfBytesTransferred) )
	{
		Infra_DbgPrintStatus(status);
		DBG(MSG_ERR,"noOfBytesToTransfer=%d noOfBytesTransferred=%d\n",
			(int)noOfBytesToTransfer,(int)noOfBytesTransferred);
	}
	Infra_Delay(I2C_Timings[mode][I2C_CONDITION_PRESTART]);

	/*I2C_CONDITION_RESTART_3*/
	noOfBytesToTransfer = 3;
	noOfBytesTransferred = 0;
	buffer[0] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
	buffer[1] = VALUE_SCLLOW_SDAHIGH; /* Value */
	buffer[2] = DIRECTION_SCLOUT_SDAOUT; /* Direction */
	status = FT_Channel_Write(I2C, handle, noOfBytesToTransfer,
		buffer,&noOfBytesTransferred);
	if ( (FT_OK != status) && (noOfBytesToTransfer != noOfBytesTransferred) )
	{
		Infra_DbgPrintStatus(status);
		DBG(MSG_ERR,"noOfBytesToTransfer=%d noOfBytesTransferred=%d\n",
			(int)noOfBytesToTransfer,(int)noOfBytesTransferred);
	}
	Infra_Delay(I2C_Timings[mode][I2C_CONDITION_PRESTART]);

	/*I2C_CONDITION_RESTART_4*/
	noOfBytesToTransfer = 3;
	noOfBytesTransferred = 0;
	buffer[0] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
	buffer[1] = VALUE_SCLHIGH_SDAHIGH; /* Value */
	buffer[2] = DIRECTION_SCLOUT_SDAOUT; /* Direction */
	status = FT_Channel_Write(I2C, handle, noOfBytesToTransfer,
		buffer,&noOfBytesTransferred);
	if ( (FT_OK != status) && (noOfBytesToTransfer != noOfBytesTransferred) )
	{
		Infra_DbgPrintStatus(status);
		DBG(MSG_ERR,"noOfBytesToTransfer=%d noOfBytesTransferred=%d\n",
			(int)noOfBytesToTransfer,(int)noOfBytesTransferred);
	}
	Infra_Delay(I2C_Timings[mode][I2C_CONDITION_PRESTART]);

	/*I2C_CONDITION_RESTART_5*/
	noOfBytesToTransfer = 3;
	noOfBytesTransferred = 0;
	buffer[0] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
	buffer[1] = VALUE_SCLLOW_SDAHIGH; /* Value */
	buffer[2] = DIRECTION_SCLOUT_SDAOUT; /* Direction */
	status = FT_Channel_Write(I2C, handle, noOfBytesToTransfer,
		buffer,&noOfBytesTransferred);
	if ( (FT_OK != status) && (noOfBytesToTransfer != noOfBytesTransferred) )
	{
		Infra_DbgPrintStatus(status);
		DBG(MSG_ERR,"noOfBytesToTransfer=%d noOfBytesTransferred=%d\n",
			(int)noOfBytesToTransfer,(int)noOfBytesTransferred);
	}
	Infra_Delay(I2C_Timings[mode][I2C_CONDITION_PRESTART]);

	/*I2C_CONDITION_RESTART -tristate SCL & SDA */
	noOfBytesToTransfer = 3;
	noOfBytesTransferred = 0;
	buffer[0] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
	buffer[1] = VALUE_SCLLOW_SDALOW; /* Value(0x00 = SCL low, SDA low) */
	buffer[2] = DIRECTION_SCLIN_SDAIN; /* Direction */
	status = FT_Channel_Write(I2C, handle, noOfBytesToTransfer,
		buffer,&noOfBytesTransferred);
	if ( (FT_OK != status) && (noOfBytesToTransfer != noOfBytesTransferred) )
	{
		Infra_DbgPrintStatus(status);
		DBG(MSG_ERR,"noOfBytesToTransfer=%d noOfBytesTransferred=%d\n",
			(int)noOfBytesToTransfer,(int)noOfBytesTransferred);
	}
	Infra_Delay(I2C_Timings[mode][I2C_CONDITION_POSTSTOP]);

	FN_EXIT;
	return status;
}
#endif // I2C_CMD_GETDEVICEID_SUPPORTED

static FT_STATUS I2C_Write8bitsAndGetAck(FT_HANDLE handle, uint8 data, bool *ack)
{
	FT_STATUS status = FT_OTHER_ERROR;
	uint8 buffer[20]={0};
	uint8 inBuffer[3]={0};
	uint32 noOfBytes = 0;
	DWORD noOfBytesTransferred;

	FN_ENTER;

	/*set direction*/
	buffer[noOfBytes++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
	buffer[noOfBytes++] = VALUE_SCLLOW_SDAHIGH; /*Value*/
	buffer[noOfBytes++] = DIRECTION_SCLOUT_SDAOUT; /*Direction*/

	/* Command to write 8bits */
	buffer[noOfBytes++]= MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE;/* MPSSE command */
	buffer[noOfBytes++]= DATA_SIZE_8BITS;
	buffer[noOfBytes++] = data;

	/* Set SDA to input mode before reading ACK bit */
	buffer[noOfBytes++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
	buffer[noOfBytes++] = VALUE_SCLLOW_SDALOW; /*Value*/
	buffer[noOfBytes++] = DIRECTION_SCLOUT_SDAIN; /*Direction*/

	/* Command to get ACK bit */
	buffer[noOfBytes++] = MPSSE_CMD_DATA_IN_BITS_POS_EDGE;/* MPSSE command */
	buffer[noOfBytes++] = DATA_SIZE_1BIT; /*Read only one bit */

	/*Command MPSSE to send data to PC immediately */
	buffer[noOfBytes++] = MPSSE_CMD_SEND_IMMEDIATE;
	status = FT_Channel_Write(I2C, handle, noOfBytes, buffer,
		&noOfBytesTransferred);
	DBG(MSG_DEBUG, "FT_Channel_Write returned: noOfBytes=%u, noOfBytesTransferred=%u \n", 
		(unsigned)noOfBytes, (unsigned)noOfBytesTransferred);
	if (FT_OK != status)
	{
		DBG(MSG_DEBUG, "FT_OK != status \n");
		Infra_DbgPrintStatus(status);
	}
	else if (noOfBytes != noOfBytesTransferred)
	{
		DBG(MSG_ERR, "Requested to send %u bytes, no. of bytes sent is %u bytes",
			(unsigned)noOfBytes, (unsigned)noOfBytesTransferred);
		status = FT_IO_ERROR;
	}
	else
	{/*Get ack*/
		noOfBytes = 1;
		noOfBytesTransferred = 0;
		INFRA_SLEEP(1);
		status = FT_Channel_Read(I2C, handle, noOfBytes, inBuffer, 
			&noOfBytesTransferred);
		DBG(MSG_DEBUG, "FT_Channel_Read returned: noOfBytes=%u, noOfBytesTransferred=%u\n",
			(unsigned)noOfBytes, (unsigned)noOfBytesTransferred);
		if (FT_OK != status)
			Infra_DbgPrintStatus(status);
		else if (noOfBytes != noOfBytesTransferred)
		{
			DBG(MSG_ERR, "Requested to send %u bytes, no. of bytes sent is %u bytes",
				(unsigned)noOfBytes,(unsigned)noOfBytesTransferred);
			status = FT_IO_ERROR;
		}
		else
		{
			*ack = (bool)(inBuffer[0] & 0x01);
			DBG(MSG_DEBUG,"success ACK= 0x%x; noOfBytes=%d noOfBytesTransferred=%d\n",
				(unsigned)*ack, (unsigned)noOfBytes, (unsigned)noOfBytesTransferred);
		}
	}
	
	FN_EXIT;
	return status;
}

static FT_STATUS I2C_Read8bitsAndGiveAck(FT_HANDLE handle, uint8 *data, bool ack)
{
	FT_STATUS status = FT_OTHER_ERROR;
	uint8 buffer[20], inBuffer[5];
	uint32 noOfBytes = 0;
	DWORD noOfBytesTransferred;

	FN_ENTER;
	
	/*set direction*/
	buffer[noOfBytes++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
	buffer[noOfBytes++] = VALUE_SCLLOW_SDALOW; /*Value*/
	buffer[noOfBytes++] = DIRECTION_SCLOUT_SDAIN; /*Direction*/

	/*Command to read 8 bits*/
	buffer[noOfBytes++] = MPSSE_CMD_DATA_IN_BITS_POS_EDGE;
	buffer[noOfBytes++] = DATA_SIZE_8BITS;/*0x00 = 1bit; 0x07 = 8bits*/

	/*Command MPSSE to send data to PC immediately */
	buffer[noOfBytes++] = MPSSE_CMD_SEND_IMMEDIATE;

	/* Fix introduced to solve a glitch issue */
	buffer[noOfBytes++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;  
	buffer[noOfBytes++] = VALUE_SCLLOW_SDALOW ; 
	buffer[noOfBytes++] = DIRECTION_SCLOUT_SDAIN;                                                                                                      
	
	/* Burn off one I2C bit time */
	buffer[noOfBytes++] = MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE;
	buffer[noOfBytes++] = 0; /*0x00 = 1bit; 0x07 = 8bits*/  
        buffer[noOfBytes++] = ack ? SEND_ACK : SEND_NACK;/*Only MSB is sent*/

	status = FT_Channel_Write(I2C, handle, noOfBytes, buffer, &noOfBytesTransferred);
	if (FT_OK != status)
		Infra_DbgPrintStatus(status);
	else if (noOfBytes != noOfBytesTransferred)
	{
		DBG(MSG_ERR, "Requested to send %u bytes, no. of bytes sent is %u bytes",
			(unsigned)noOfBytes, (unsigned)noOfBytesTransferred);
		status = FT_IO_ERROR;
	}
	else
	{
		noOfBytes = 1;
		status = FT_Channel_Read(I2C, handle, noOfBytes, inBuffer, &noOfBytesTransferred);
		if (FT_OK != status)
		{
			Infra_DbgPrintStatus(status);
		}
		else if (noOfBytes != noOfBytesTransferred)
		{
			DBG(MSG_ERR, "Requested to read %u bytes, no. of bytes read is %u bytes",
				(unsigned)noOfBytes,(unsigned)noOfBytesTransferred);
			status = FT_IO_ERROR;
		}
		else
		{
			*data = inBuffer[0];
			DBG(MSG_DEBUG,"	*data = 0x%x\n", (unsigned)*data);
		}
	}

	FN_EXIT;
	return status;
}

static FT_STATUS I2C_FastWrite(FT_HANDLE handle, UCHAR deviceAddress,
	DWORD sizeToTransfer, UCHAR *buffer, UCHAR *ack, LPDWORD sizeTransferred,
	uint32 options)
{
	FT_STATUS status = FT_OK;
	uint32 i = 0; /* index of cmdBuffer that is filled */
	uint32 j = 0; /* scratch register */
	uint32 sizeTotal;
	uint32 sizeOverhead;
	uint8* outBuffer;
	uint8* inBuffer;
	uint8  bitsInThisTransfer;
	DWORD bytesRead;
	uint32 bytesToTransfer;
	uint8 tempAddress;
	uint32 bitsToTransfer;
#if defined(FASTWRITE_READ_ACK) && defined(INFRA_DEBUG_ENABLE)
	uint32 bitsToRead = 0;
#endif

	FN_ENTER;

	if (options & I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS)
	{/* size is in bits */
		bitsToTransfer = sizeToTransfer;
	}
	else
	{/* size is in bytes */
	    bitsToTransfer = sizeToTransfer * 8;
	}
	bytesToTransfer = (bitsToTransfer > 0)?(((bitsToTransfer/8)==0)?1:((bitsToTransfer/8)+1)):(0);

	/* Calculate size of required buffer */
	sizeTotal = (bytesToTransfer*(6
#ifdef FASTWRITE_READ_ACK
	+5
#endif
	)) /*the size of data itself */
	+ ((!(options & I2C_TRANSFER_OPTIONS_NO_ADDRESS))?(11):(0))/*for address byte*/
		+ ((options & I2C_TRANSFER_OPTIONS_START_BIT)?((START_DURATION_1 +	
			START_DURATION_2+1)*3):0) /* size required for START */
		+ ((options & I2C_TRANSFER_OPTIONS_STOP_BIT)?((STOP_DURATION_1 +		
			STOP_DURATION_2+STOP_DURATION_3+1)*3):0); /* size for STOP */

	sizeOverhead = sizeTotal - bytesToTransfer;
	(void)sizeOverhead; /* NB Not used */
	
	/* Allocate buffers */
	outBuffer = (uint8*) INFRA_MALLOC(sizeTotal);
	if (NULL == outBuffer)
	{
		return FT_INSUFFICIENT_RESOURCES;
	}

	/* Write START bit */
	if (options & I2C_TRANSFER_OPTIONS_START_BIT)
	{
		DBG(MSG_DEBUG,"adding START condition\n");
		/* SCL high, SDA high */
		for (j = 0; j < START_DURATION_1; j++)
		{
			outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
			outBuffer[i++] = VALUE_SCLHIGH_SDAHIGH;
			outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
		}
		/* SCL high, SDA low */
		for (j = 0; j < START_DURATION_2; j++)
		{
			outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
			outBuffer[i++] = VALUE_SCLHIGH_SDALOW;
			outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
		}
		/*SCL low, SDA low */
		outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
		outBuffer[i++] = VALUE_SCLLOW_SDALOW;
		outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
	}

	if (!(options & I2C_TRANSFER_OPTIONS_NO_ADDRESS))
	{
		tempAddress = (uint8)deviceAddress;
		tempAddress = (tempAddress << 1);
		tempAddress = (tempAddress & I2C_ADDRESS_WRITE_MASK);
		DBG(MSG_DEBUG,"7bit I2C address plus direction bit = 0x%x\n", tempAddress);

		/*set direction*/
		outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
		outBuffer[i++] = VALUE_SCLLOW_SDAHIGH; /*Value*/
		outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT; /*Direction*/

		/* write address + direction bit */
		outBuffer[i++]= MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE;/* MPSSE command */
		outBuffer[i++]= DATA_SIZE_8BITS;
		outBuffer[i++] = tempAddress;

#ifdef FASTWRITE_READ_ACK
		/* Set SDA to input mode before reading ACK bit */
		outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
		outBuffer[i++] = VALUE_SCLLOW_SDALOW; /*Value*/
		outBuffer[i++] = DIRECTION_SCLOUT_SDAIN; /*Direction*/

		/* Command to get ACK bit */
		outBuffer[i++] = MPSSE_CMD_DATA_IN_BITS_POS_EDGE;/* MPSSE command */
		outBuffer[i++] = DATA_SIZE_1BIT; /* Read only one bit */
#ifdef INFRA_DEBUG_ENABLE
		bitsToRead++;
#endif
#endif
	}

	/* add commands & data to buffer */
	j = 0;
	while(j < bitsToTransfer)
	{
		bitsInThisTransfer = ((bitsToTransfer-j)>8)?8:(bitsToTransfer-j);
		/*set direction*/
		outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
		outBuffer[i++] = VALUE_SCLLOW_SDAHIGH; /*Value*/
		outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT; /*Direction*/

		/* Command to write 8bits */
		bitsInThisTransfer = ((bitsToTransfer-j)>8)?8:(bitsToTransfer-j);
		outBuffer[i++]= MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE;/* MPSSE command */
		outBuffer[i++]= bitsInThisTransfer - 1;
		outBuffer[i++] = buffer[j/8];

#ifdef FASTWRITE_READ_ACK
		/* Read 1bit ack after each 8bits written - only in byte mode */
		if (options & I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES)
		{
			/* Set SDA to input mode before reading ACK bit */
			outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
			outBuffer[i++] = VALUE_SCLLOW_SDALOW; /*Value*/
			outBuffer[i++] = DIRECTION_SCLOUT_SDAIN; /*Direction*/

			/* Command to get ACK bit */
			outBuffer[i++] = MPSSE_CMD_DATA_IN_BITS_POS_EDGE;/* MPSSE command */
			outBuffer[i++] = DATA_SIZE_1BIT; /* Read only one bit */
#ifdef INFRA_DEBUG_ENABLE
			bitsToRead++;
#endif
		}
#endif
		j+= bitsInThisTransfer;
		DBG(MSG_DEBUG,"i=%u bitsToTransfer=%u bitsInThisTransfer=%u\n",
			(unsigned)i,(unsigned)bitsToTransfer,(unsigned)bitsInThisTransfer);
	}

	/* Write STOP bit */
	if (options & I2C_TRANSFER_OPTIONS_STOP_BIT)
	{
		/* SCL low, SDA low */
		for (j = 0; j < STOP_DURATION_1; j++)
		{
			outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
			outBuffer[i++] = VALUE_SCLLOW_SDALOW;
			outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
		}
		/* SCL high, SDA low */
		for (j = 0; j < STOP_DURATION_2; j++)
		{
			outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
			outBuffer[i++] = VALUE_SCLHIGH_SDALOW;
			outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
		}
		/* SCL high, SDA high */
		for (j = 0; j < STOP_DURATION_3; j++)
		{
			outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
			outBuffer[i++] = VALUE_SCLHIGH_SDAHIGH;
			outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
		}
		outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
		outBuffer[i++] = VALUE_SCLHIGH_SDAHIGH;
		outBuffer[i++] = DIRECTION_SCLIN_SDAIN; /* Tristate the SCL & SDA pins */
	}

	/* write buffer */
	DBG(MSG_DEBUG,"i=%u bitsToTransfer=%u bitsInThisTransfer=%u\n", (unsigned)i,
	(unsigned)bitsToTransfer,(unsigned)bitsInThisTransfer);
	status = FT_Channel_Write(I2C, handle, i,outBuffer,&bytesRead);
	*sizeTransferred = sizeToTransfer;
	CHECK_STATUS(status);

#ifdef FASTWRITE_READ_ACK
	/* Read ack of address */
	if (!(options & I2C_TRANSFER_OPTIONS_NO_ADDRESS))
	{
		uint8 addAck;
		status = FT_Channel_Read(I2C, handle, 1, &addAck, &bytesRead);
		DBG(MSG_DEBUG,"addAck = %u bitsToRead=%u bytesToRead=%u\n", (unsigned)(addAck & 1),
			(unsigned)bitsToRead, (unsigned)bitsToRead / 8);
	}

	/* if byte mode: read 1bit ack after each 8bits written */
	if (options & I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES)
	{
		inBuffer = INFRA_MALLOC(bytesToTransfer);
		if (NULL != inBuffer)
		{
			status = FT_Channel_Read(I2C, handle, sizeToTransfer, inBuffer, 
				&bytesRead);
			CHECK_STATUS(status);
			if (ack)
			{/* Copy the ack bits into the ack buffer if provided */
				INFRA_MEMCPY(ack, inBuffer, bytesRead);
			}
			INFRA_FREE(inBuffer);
		}
	}
#endif
	INFRA_FREE(outBuffer);
	FN_EXIT;
	return status;
}

static FT_STATUS I2C_FastRead(FT_HANDLE handle, UCHAR deviceAddress,
	DWORD sizeToTransfer, UCHAR *buffer, UCHAR *ack, DWORD *sizeTransferred,
	uint32 options)
{
	FT_STATUS status = FT_OK;
	uint32 i = 0; /* index of cmdBuffer that is filled */
	uint32 j = 0; /* scratch register */
	uint32 sizeTotal;
	uint32 sizeOverhead;
	uint8* outBuffer;
	uint8  bitsInThisTransfer;
	DWORD bytesRead;
	uint8  addressAck;
	uint32 bytesToTransfer;
	uint8 tempAddress;
	uint32 bitsToTransfer;

	FN_ENTER;

	if (options & I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS)
	{/* size is in bits */
		bitsToTransfer = sizeToTransfer;
	}
	else
	{/* size is in bytes */
	    bitsToTransfer = sizeToTransfer * 8;
	}
	bytesToTransfer = (bitsToTransfer > 0) ? (((bitsToTransfer/8)==0)?1:(bitsToTransfer/8)): (0);

	/* Calculate size of required buffer */
	sizeTotal = (bytesToTransfer*12) /* the size of data itself */
	+ ((!(options & I2C_TRANSFER_OPTIONS_NO_ADDRESS))?(11):(0))/* for address byte*/
	+ ((options & I2C_TRANSFER_OPTIONS_START_BIT)?((START_DURATION_1 +	
			START_DURATION_2+1)*3):0) /* size required for START */
	+ ((options & I2C_TRANSFER_OPTIONS_STOP_BIT)?((STOP_DURATION_1 +		
			STOP_DURATION_2+STOP_DURATION_3+1)*3):0); /* size for STOP */

	sizeOverhead = sizeTotal - bytesToTransfer;
	(void)sizeOverhead; /* NB Not used */
	
	/* Allocate buffers */
	outBuffer = (uint8*) INFRA_MALLOC(sizeTotal);
	if (NULL == outBuffer)
	{
		return FT_INSUFFICIENT_RESOURCES;
	}

	/* Write START bit */
	if (options & I2C_TRANSFER_OPTIONS_START_BIT)
	{
		/* SCL high, SDA high */
		for (j = 0; j < START_DURATION_1; j++)
		{
			outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
			outBuffer[i++] = VALUE_SCLHIGH_SDAHIGH;
			outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
		}
		/* SCL high, SDA low */
		for (j = 0; j < START_DURATION_2; j++)
		{
			outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
			outBuffer[i++] = VALUE_SCLHIGH_SDALOW;
			outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
		}
		/*SCL low, SDA low */
		outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
		outBuffer[i++] = VALUE_SCLLOW_SDALOW;
		outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
	}

	if (!(options & I2C_TRANSFER_OPTIONS_NO_ADDRESS))
	{
		tempAddress = (uint8)deviceAddress;
		tempAddress = (tempAddress << 1);
		tempAddress = (tempAddress | I2C_ADDRESS_READ_MASK);
		DBG(MSG_DEBUG,"7bit I2C address plus direction bit = 0x%x\n", tempAddress);

		/*set direction*/
		outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
		outBuffer[i++] = VALUE_SCLLOW_SDAHIGH; /*Value*/
		outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT; /*Direction*/

		/* write address + direction bit */
		outBuffer[i++]= MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE;/* MPSSE command */
		outBuffer[i++]= DATA_SIZE_8BITS;
		outBuffer[i++] = tempAddress;

#ifdef FASTWRITE_READ_ACK
		/* Set SDA to input mode before reading ACK bit */
		outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
		outBuffer[i++] = VALUE_SCLLOW_SDALOW; /*Value*/
		outBuffer[i++] = DIRECTION_SCLOUT_SDAIN; /*Direction*/

		/* Command to get ACK bit */
		outBuffer[i++] = MPSSE_CMD_DATA_IN_BITS_POS_EDGE;/* MPSSE command */
		outBuffer[i++] = DATA_SIZE_1BIT; /* Read only one bit */
#endif
	}

	/* add commands & data to buffer */
	j = 0;
	while(j < bitsToTransfer)
	{
		bitsInThisTransfer = ((bitsToTransfer-j)>8)?8:(bitsToTransfer-j);

		/*set direction*/
		outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
		outBuffer[i++] = VALUE_SCLLOW_SDALOW; /*Value*/
		outBuffer[i++] = DIRECTION_SCLOUT_SDAIN; /*Direction*/

		/*Command to read 8 bits*/
		outBuffer[i++] = MPSSE_CMD_DATA_IN_BITS_POS_EDGE;
		outBuffer[i++] = bitsInThisTransfer - 1;

		/*Command MPSSE to send data to PC immediately */
		/*buffer[i++] = MPSSE_CMD_SEND_IMMEDIATE;*/

		/* Write 1bit ack after each 8bits read - only in byte mode */
		if (options & I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES)
		{
			outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
			outBuffer[i++] = VALUE_SCLLOW_SDALOW ;  
			outBuffer[i++] = DIRECTION_SCLOUT_SDAIN;

        // Burn off one I2C bit time
       outBuffer[i++] = MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE;                                                                      //
			outBuffer[i++] = 0; /*0x00 = 1bit; 0x07 = 8bits*/ 
        outBuffer[i++] = (j<(bitsToTransfer-1))?(SEND_ACK):	\
			((options & I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE)?SEND_NACK:SEND_ACK);
		}
		j+= bitsInThisTransfer;
	}
	*sizeTransferred = j;

	/* Write STOP bit */
	if (options & I2C_TRANSFER_OPTIONS_STOP_BIT)
	{
		/* SCL low, SDA low */
		for (j = 0; j < STOP_DURATION_1; j++)
		{
			outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
			outBuffer[i++] = VALUE_SCLLOW_SDALOW;
			outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
		}
		/* SCL high, SDA low */
		for (j = 0; j < STOP_DURATION_2; j++)
		{
			outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
			outBuffer[i++] = VALUE_SCLHIGH_SDALOW;
			outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
		}
		/* SCL high, SDA high */
		for (j = 0; j < STOP_DURATION_3; j++)
		{
			outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
			outBuffer[i++] = VALUE_SCLHIGH_SDAHIGH;
			outBuffer[i++] = DIRECTION_SCLOUT_SDAOUT;
		}
		outBuffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
		outBuffer[i++] = VALUE_SCLHIGH_SDAHIGH;
		outBuffer[i++] = DIRECTION_SCLIN_SDAIN; /* Tristate the SCL & SDA pins */
	}

	/* write buffer */
	DBG(MSG_DEBUG,"i=%u bitsToTransfer=%u bitsInThisTransfer=%u\n",
		(unsigned)i, (unsigned)bitsToTransfer, (unsigned)bitsInThisTransfer);
	status = FT_Channel_Write(I2C, handle, i,outBuffer,&bytesRead);
	CHECK_STATUS(status);

	/*read the address ack bit */
	if (!(options & I2C_TRANSFER_OPTIONS_NO_ADDRESS))
	{
		status = FT_Channel_Read(I2C, handle, 1, &addressAck, &bytesRead);
		CHECK_STATUS(status);
	}

	/* read the actual data from the MPSSE-chip into the host system */
	if (options & I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES)
		status = FT_Channel_Read(I2C, handle, bytesToTransfer, buffer, &bytesRead);
	else
		status = FT_Channel_Read(I2C, handle, bytesToTransfer+1, buffer, &bytesRead);
	CHECK_STATUS(status);

	INFRA_FREE(outBuffer);
	
	FN_EXIT;
	return status;
}

static FT_STATUS I2C_WriteDeviceAddress(FT_HANDLE handle, UCHAR deviceAddress,
			bool direction, bool AddLen10Bit, bool *ack)
{
	FT_STATUS status = FT_OTHER_ERROR;
	uint8 tempAddress;
	
	FN_ENTER;
	
	if (!AddLen10Bit)
	{/* 7bit addressing */
		tempAddress = (uint8)deviceAddress;
		DBG(MSG_DEBUG,"7bit I2C address = 0x%x\n", tempAddress);
		tempAddress = (tempAddress << 1);
		if (direction)
			tempAddress = (tempAddress | I2C_ADDRESS_READ_MASK);
		else
			tempAddress = (tempAddress & I2C_ADDRESS_WRITE_MASK);
		DBG(MSG_DEBUG,"7bit I2C address plus direction bit = 0x%x\n", tempAddress);
		status = I2C_Write8bitsAndGetAck(handle, tempAddress, ack);
		if (FT_OK != status)
			Infra_DbgPrintStatus(status);
		if ((*ack))
		{
			DBG(MSG_ERR,"Didn't receieve an ACK from the addressed device\n");
		}
	}
	else
	{/* 10bit addressing */
		/* TODO: Add support for 10bit addressing */
		DBG(MSG_ERR, "10 bit addressing yet to be supported\n");
		status = FT_NOT_SUPPORTED;
	}
	
	FN_EXIT;
	return status;
}

static FT_STATUS I2C_SaveChannelConfig(FT_HANDLE handle, ChannelConfig *config)
{
	FT_STATUS status = FT_OTHER_ERROR;
	FN_ENTER;

	status = FT_OK;
	FN_EXIT;
	return status;
}

#ifdef I2C_CMD_GETDEVICEID_SUPPORTED
static FT_STATUS I2C_GetChannelConfig(FT_HANDLE handle, ChannelConfig *config)
{
	FT_STATUS status = FT_OTHER_ERROR;
	
	FN_ENTER;

	status = FT_OK;
	
	FN_EXIT;
	return status;
}
#endif // I2C_CMD_GETDEVICEID_SUPPORTED

static FT_STATUS I2C_Start(FT_HANDLE handle)
{
	FT_STATUS status;
	uint8 buffer[(START_DURATION_1+START_DURATION_2+1)*3];
	uint32 i = 0, j = 0;
	DWORD noOfBytesTransferred;
	
	FN_ENTER;

	/* SCL high, SDA high */
	for (j = 0; j < START_DURATION_1; j++)
	{
		buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
		buffer[i++] = VALUE_SCLHIGH_SDAHIGH;
		buffer[i++] = DIRECTION_SCLOUT_SDAOUT;
	}
	/* SCL high, SDA low */
	for (j = 0; j < START_DURATION_2; j++)
	{
		buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
		buffer[i++] = VALUE_SCLHIGH_SDALOW;
		buffer[i++] = DIRECTION_SCLOUT_SDAOUT;
	}
	/*SCL low, SDA low */
	buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
	buffer[i++] = VALUE_SCLLOW_SDALOW;
	buffer[i++] = DIRECTION_SCLOUT_SDAOUT;

	status = FT_Channel_Write(I2C, handle, i,buffer,&noOfBytesTransferred);

	FN_EXIT;
	return status;
}

static FT_STATUS I2C_Stop(FT_HANDLE handle)
{
	FT_STATUS status;
	uint8 buffer[(STOP_DURATION_1+STOP_DURATION_2+STOP_DURATION_3+1)*3];
	uint32 i = 0, j = 0;
	DWORD noOfBytesTransferred;

	FN_ENTER;
	
	/* SCL low, SDA low */
	for (j = 0; j < STOP_DURATION_1; j++)
	{
		buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
		buffer[i++] = VALUE_SCLLOW_SDALOW;
		buffer[i++] = DIRECTION_SCLOUT_SDAOUT;
	}
	/* SCL high, SDA low */
	for (j = 0; j < STOP_DURATION_2; j++)
	{
		buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
		buffer[i++] = VALUE_SCLHIGH_SDALOW;
		buffer[i++] = DIRECTION_SCLOUT_SDAOUT;
	}
	/* SCL high, SDA high */
	for (j = 0; j < STOP_DURATION_3; j++)
	{
		buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
		buffer[i++] = VALUE_SCLHIGH_SDAHIGH;
		buffer[i++] = DIRECTION_SCLOUT_SDAOUT;
	}
	buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
	buffer[i++] = VALUE_SCLHIGH_SDAHIGH;
	buffer[i++] = DIRECTION_SCLIN_SDAIN; /* Tristate the SCL & SDA pins */

	status = FT_Channel_Write(I2C, handle, i,buffer,&noOfBytesTransferred);

	FN_EXIT;
	return status;
}


