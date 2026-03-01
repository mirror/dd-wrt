/*!
 * \file ftdi_mid.c
 *
 * \author FTDI
 * \date 20110321
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
 * Module: Middle Layer
 *
 * Revision History:
 * 0.1 -  initial version
 * 0.2 -  20110524 - updated for SPI and cleaned up
 * 0.21- 20110708 - Added functions FT_ReadGPIO & FT_WriteGPIO
 * 0.3 -  20111103 - Added MPSSE_CMD_ENABLE_DRIVE_ONLY_ZERO
 */


/******************************************************************************/
/*								Include files					  			  */
/******************************************************************************/
#define FTDI_EXPORTS
#include "ftdi_infra.h"		/*Common portable infrastructure(datatypes, libraries, etc)*/
#include "ftdi_common.h"	/*Common across I2C, SPI, JTAG modules*/
#include "ftdi_mid.h"		/*Midlayer specific specific*/
#include "string.h"


/******************************************************************************/
/*								Macro defines					  			  */
/******************************************************************************/




/******************************************************************************/
/*								Local function declarations					  */
/******************************************************************************/




/******************************************************************************/
/*								Global variables							  */
/******************************************************************************/



/******************************************************************************/
/*						Public function definitions						  */
/******************************************************************************/

/*!
 * \brief Returns the number of MPSSE channels a available
 *
 * This function returns the number of MPSSE channels that are connected to the host system
 * Although all the MPSSE channels that are available in the current chips support SPI/I2C/JTAG,
 * future versions of MPSSE may have support for other legacy protocols too. The caller of this
 * function (i.e. I2C_GetNumChannel, SPI_GetNumChannel, etc) should check if all the channels
 * reported by this function support the legacy protocol that the caller needs to support.
 *
 * \param[in] Protocol Specifies the protocol type(I2C/SPI/JTAG)
 * \param[out] *numChans
 * \return status
 * \sa
 * \note This function doesn't return the number of FTDI chips connected to the host system
 * \note FT2232D has 1 MPSSE port
 * \note FT2232H has 2 MPSSE ports
 * \note FT4232H has 4 ports but only 2 of them have MPSSEs
 * so a call to this function will return 2 if a FT4232 is connected to it.
 * \warning
 */
FT_STATUS FT_GetNumChannels(FT_LegacyProtocol Protocol, DWORD *numChans)
{
	DWORD tempNumChannels;
	FT_DEVICE_LIST_INFO_NODE *pDeviceList;
	FT_DEVICE_LIST_INFO_NODE deviceList;

	FT_STATUS status;
	DWORD devLoop = MID_NO_CHANNEL_FOUND;

	FN_ENTER;
	/*initalize *numChansto 0 */
	*numChans = MID_NO_CHANNEL_FOUND;

	/*Get the number of devices connected to the system with
	  FT_CreateDeviceInfoList */
	status = varFunctionPtrLst.p_FT_GetNumChannel(&tempNumChannels);
	/*Check if the status is Ok */
	if (status == FT_OK)
	{

		/*Check if No of channel is greater than 0*/
		if (tempNumChannels > MID_NO_CHANNEL_FOUND)
		{
			/*Allocate space for getting the information about the device (based on
			number of devices)*/
			pDeviceList = INFRA_MALLOC(sizeof(FT_DEVICE_LIST_INFO_NODE) * tempNumChannels);
			if (NULL == pDeviceList)
			{
				return FT_INSUFFICIENT_RESOURCES;
			}
			/*get the devices information(FT_GetDeviceInfoList)*/
			status = varFunctionPtrLst.p_FT_GetDeviceInfoList(pDeviceList, &tempNumChannels);
			while(devLoop < tempNumChannels)
			{
				deviceList = pDeviceList[devLoop];
				/*check if device is I2C*/
				if (Mid_CheckMPSSEAvailable(deviceList))
				{
					/*increment *numChans*/
					*numChans = *numChans + 1;
				}
				devLoop++;
			}
			INFRA_FREE(pDeviceList);
		}
		else
		{
			*numChans = tempNumChannels;
        }
	}
	else
	{
		*numChans = tempNumChannels;
	}

	/*return status*/
	FN_EXIT;
	return status;
}

FT_STATUS FT_GetChannelInfo(FT_LegacyProtocol Protocol, DWORD index,
			FT_DEVICE_LIST_INFO_NODE *chanInfo)
{
	DWORD tempNumChannels, channelCount;
	FT_DEVICE_LIST_INFO_NODE *pDeviceList;
	FT_DEVICE_LIST_INFO_NODE deviceList;

	FT_STATUS status;
	DWORD devLoop = MID_NO_CHANNEL_FOUND;

	FN_ENTER;

	/*initalize *numChansto 0 */
	channelCount = MID_NO_CHANNEL_FOUND;

	/*Get the number of devices connected to the system with
	  FT_CreateDeviceInfoList */
	status = varFunctionPtrLst.p_FT_GetNumChannel(&tempNumChannels);
	CHECK_STATUS(status);

	/*Check if No of channel is greater than 0*/
	if (tempNumChannels >= index)
	{
		/*Allocate space for getting the information about the device (based on
		number of devices)*/
		pDeviceList = INFRA_MALLOC(sizeof(FT_DEVICE_LIST_INFO_NODE) * tempNumChannels);
		if (NULL == pDeviceList)
		{
			return FT_INSUFFICIENT_RESOURCES;
		}

		/*get the devices information with FT_GetDeviceInfoList */
		status = varFunctionPtrLst.p_FT_GetDeviceInfoList(pDeviceList, &tempNumChannels);
		CHECK_STATUS(status);

		while(devLoop < tempNumChannels)
		{
			deviceList = pDeviceList[devLoop];
			if (Mid_CheckMPSSEAvailable(deviceList))
			{
				/*increment *numChans*/
				channelCount++;
			}
			/*check if the index matches the I2C channel count*/
			if (channelCount == index)
			{
				INFRA_MEMCPY(chanInfo,&deviceList, sizeof(FT_DEVICE_LIST_INFO_NODE));
				break;
			}
			devLoop++;
		}
		INFRA_FREE(pDeviceList);
	}
	else
	{
		/* The index of the device is greater than the max number of devices available */
		return FT_INVALID_HANDLE;
	}

	/*return status*/
	FN_EXIT;
	return status;
}


/*!
 * \brief Opens a channel and returns a handle to it
 *
 * This function opens the indexed channel and returns a handle to it
 *
 * \param[in] Protocol Specifies the protocol type(I2C/SPI/JTAG)
 * \param[in] index Index of the channel
 * \param[out] handle Pointer to the handle
 * \return status
 * \sa
 * \note Trying to open an already open channel will return an error code
 * \warning
 */
FT_STATUS FT_OpenChannel(FT_LegacyProtocol Protocol, DWORD index,
			FT_HANDLE *handle)
{
	/* Opens a channel and returns the pointer to its handle */
	DWORD tempNumChannels, channelCount;
	FT_DEVICE_LIST_INFO_NODE *pDeviceList;
	FT_DEVICE_LIST_INFO_NODE deviceList;

	FT_STATUS status;
	DWORD devLoop = MID_NO_CHANNEL_FOUND;
	FN_ENTER;

	/*initalize *numChansto 0 */
	channelCount = MID_NO_CHANNEL_FOUND;

	/*Get the number of devices connected to the system with
	  FT_CreateDeviceInfoList */
	status = varFunctionPtrLst.p_FT_GetNumChannel(&tempNumChannels);
	CHECK_STATUS(status);

	/*Check if No of channel is greater than 0*/
	if (tempNumChannels >= index)
	{
		/*Allocate space for getting the information about the device (based on
		number of devices)*/
		pDeviceList = INFRA_MALLOC(sizeof(FT_DEVICE_LIST_INFO_NODE) * tempNumChannels);
		if (NULL == pDeviceList)
		{
			return FT_INSUFFICIENT_RESOURCES;
		}
		/*get the devices information(FT_GetDeviceInfoList)*/
		status = varFunctionPtrLst.p_FT_GetDeviceInfoList(pDeviceList, &tempNumChannels);
		CHECK_STATUS(status);

		/*loop until No of devices */
		while(devLoop < tempNumChannels)
		{
			deviceList = pDeviceList[devLoop];
			/*check if device is I2C*/
			if (Mid_CheckMPSSEAvailable(deviceList))
			{
				/*increment *numChans*/
				channelCount++;
			}
			/*check if the index matches the I2C channel count*/
			if (channelCount == index)
			{
				/*call FT_Open*/
				status = varFunctionPtrLst.p_FT_Open(devLoop, handle);
                break;
            }
			devLoop++;
		}
		INFRA_FREE(pDeviceList);
	}
	else
	{
		/* The index of the device is greater than the max number of devices available */
		return FT_INVALID_HANDLE;
	}

	/*return status*/
	FN_EXIT;
	return status;
}

/*!
 * \brief Initializes a channel
 *
 * This function initializes the channel and the communication parameters associated with it. The
 * function takes variable number of parameters. For example, if channelType is I2C then the
 * number of variable parameters will be equal to the number of members in the structure
 * ChannelConfig. The variable parameters will be passed to this function in the same order as
 * they appear in the structure defination
 * The function performs the USB function specific initialization followed by MPSSE initialization.
 * Once that is done it will configure MPSSE with the legacy protocol specific initializations(SPI/
 * I2C/JTAG) with the help of that parameters that are passed by the caller
 *
 * \param[in] Protocol Specifies the protocol type(I2C/SPI/JTAG)
 * \param[in] handle Handle of the channel
 * \param[in] varArg1 Clock rate
 * \param[in] varArg2 Latency timer
 * \param[in] varArg3 Configuration options
 * \return status
 * \sa
 * \note
 * \warning
 */
FT_STATUS FT_InitChannel(FT_LegacyProtocol Protocol, FT_HANDLE handle,...)
{
	va_list argumentList;
	uint32	clockRate, latencyTimer, configOptions;
	FT_STATUS status;
	FT_DEVICE ftDevice;

	FN_ENTER;

	/*initialise the argument list*/
	va_start(argumentList, handle);
	/*Get the value for Clockrate*/
	clockRate = va_arg(argumentList, uint32);
	/*latencyTimer*/
	latencyTimer = va_arg(argumentList, uint32);
	/* The options parameter passed in I2C_Init, SPI_Init */
	configOptions = va_arg(argumentList, uint32);
	/*Check parameters*/
	if ((clockRate < MIN_CLOCK_RATE)
		|| (clockRate > MAX_CLOCK_RATE)
		|| (latencyTimer < MIN_LATENCY_TIMER)
		|| (latencyTimer > MAX_LATENCY_TIMER))
				return FT_INVALID_PARAMETER;

	/*Get the device type*/
	status = Mid_GetFtDeviceType(handle, &ftDevice);
	CHECK_STATUS(status);
	/*reset the device*/
	status = Mid_ResetDevice(handle);
	CHECK_STATUS(status);
	/*Purge*/
	status = Mid_PurgeDevice(handle);
	CHECK_STATUS(status);
	/*set USB buffer size*/
	status = Mid_SetUSBParameters(handle,
		USB_INPUT_BUFFER_SIZE, USB_OUTPUT_BUFFER_SIZE);
	CHECK_STATUS(status);
	/*sets the special characters for the device,
	disable event and error characters*/
	status = Mid_SetDeviceSpecialChar(handle, FALSE, DISABLE_EVENT, FALSE, DISABLE_CHAR);
	CHECK_STATUS(status);
	/*SetTimeOut*/
	status = Mid_SetDeviceTimeOut(handle, DEVICE_READ_TIMEOUT, DEVICE_WRITE_TIMEOUT);
	CHECK_STATUS(status);
	/*SetLatencyTimer*/
	status = Mid_SetLatencyTimer(handle,(UCHAR)latencyTimer);
	CHECK_STATUS(status);
	/*ResetMPSSE*/
	status = Mid_ResetMPSSE(handle);
	CHECK_STATUS(status);
	/*EnableMPSSEInterface*/
	status = Mid_EnableMPSSEIn(handle);
	CHECK_STATUS(status);
	/*20110608 - enabling loopback before sync*/
	status = Mid_SetDeviceLoopbackState(handle, MID_LOOPBACK_TRUE);
	CHECK_STATUS(status);
	/*Sync MPSSE */
	status = Mid_SyncMPSSE(handle);
	CHECK_STATUS(status);
	/*wait for USB*/
	INFRA_SLEEP(50);
	/*set Clock frequency*/
	status = Mid_SetClock(handle, ftDevice, clockRate);
	CHECK_STATUS(status);
	DBG(MSG_INFO, "Mid_SetClock Status Ok return 0x%x\n",(unsigned)status);
	INFRA_SLEEP(20);
	/*Stop Loop back*/
	status = Mid_SetDeviceLoopbackState(handle, MID_LOOPBACK_FALSE);
	CHECK_STATUS(status);
	DBG(MSG_INFO, "Mid_SetDeviceLoopbackState Status Ok return 0x%x\n", (unsigned)status);
	status = Mid_EmptyDeviceInputBuff(handle);
	CHECK_STATUS(status);
	DBG(MSG_INFO, "Mid_EmptyDeviceInputBuff Status Ok return 0x%x\n", (unsigned) status);

	switch(Protocol)
	{
		case I2C:
		{
			/*Set i/o pin states*/
			status = Mid_SetGPIOLow(handle, MID_SET_LOW_BYTE_DATA_BITS_DATA,
				MID_SET_LOW_BYTE_DATA_BITS_DATA);
			CHECK_STATUS(status);

			/* The I2C master should actually drive the SDA line only when the output is LOW.
			It should tristate the SDA line when the output should be high. This tristating
			the SDA line during output HIGH is supported only in FT232H chip*/
			if ((ftDevice == FT_DEVICE_232H) && (configOptions & I2C_ENABLE_DRIVE_ONLY_ZERO))
			{
				uint8 buffer[3];//3
				DWORD noOfBytesToTransfer;
				DWORD noOfBytesTransferred;
				DBG(MSG_DEBUG,"Enabling DRIVE_ONLY_ZERO\n");
				noOfBytesToTransfer = 3;
				noOfBytesTransferred = 0;
				buffer[0] = MPSSE_CMD_ENABLE_DRIVE_ONLY_ZERO;/* MPSSE command */
				buffer[1] = 0x03; /* LowByte */
				buffer[2] = 0x00; /* HighByte */
				status = FT_Channel_Write(I2C, handle, noOfBytesToTransfer,
					buffer,&noOfBytesTransferred);
				CHECK_STATUS(status);
			}
		}
		break;

		case SPI:
		break;

		case JTAG:

		break;

		default:
			DBG(MSG_WARN, "undefined protocol value(%u)\n",(unsigned)Protocol);
	}
	FN_EXIT;
	return FT_OK;

}

/*!
 * \brief Closes a channel
 *
 * Closes a channel and frees all resources that were used by it
 *
 * \param[in] Protocol Specifies the protocol type(I2C/SPI/JTAG)
 * \param[in] handle Handle of the channel
 * \param[out] none
 * \return status
 * \sa
 * \note
 * \warning
 */
FT_STATUS FT_CloseChannel(FT_LegacyProtocol Protocol, FT_HANDLE handle)
{
	FT_STATUS status;
	FN_ENTER;
	status = varFunctionPtrLst.p_FT_Close(handle);
	FN_EXIT;
	return status;
}

/*!
 * \brief Reads data from channel
 *
 * This function reads the specified number of bytes from channel.
 *
 * \param[in] Protocol Specifies the protocol type(I2C/SPI/JTAG)
 * \param[in] handle Handle of the channel
 * \param[in] noOfBytes Number of bytes to be read
 * \param[out] buffer Pointer to the buffer where data is to be read
 * \param[out] noOfBytesTransferred The actual number of bytes transfered
 * \return status
 * \sa
 * \note
 * \warning
 */



FT_STATUS FT_Channel_Read(FT_LegacyProtocol Protocol, FT_HANDLE handle,
				DWORD noOfBytes, uint8* buffer, LPDWORD noOfBytesTransferred)
{
	FT_STATUS status;
	FN_ENTER;

	status = varFunctionPtrLst.p_FT_Read(handle, buffer, noOfBytes, noOfBytesTransferred);

#ifdef INFRA_DEBUG_ENABLE
	{
		DWORD i;
		printf("FT_Channel_Read called with noOfBytes=%u\n", (unsigned)noOfBytes);
		for (i = 0; i < noOfBytes; i++)
		{
			printf(" 0x%x", buffer[i]);
		}
		printf("\n");
	}
#endif

	FN_EXIT;
    return status;


}

/*!
 * \brief Writes data to the channel
 *
 * This function writes the specified number of bytes to the channel
 *
 * \param[in] Protocol Specifies the protocol type(I2C/SPI/JTAG)
 * \param[in] handle Handle of the channel
 * \param[in] noOfBytes Number of bytes to be written
 * \param[in] buffer Pointer to the buffer from where data is to be written
 * \param[out] noOfBytesTransferred The actual number of bytes transfered
 * \return status
 * \sa
 * \note
 * \warning
 */
FT_STATUS FT_Channel_Write(FT_LegacyProtocol Protocol, FT_HANDLE handle,
			DWORD noOfBytes, uint8* buffer, LPDWORD noOfBytesTransferred)
{
	FT_STATUS status;
	FN_ENTER;

#ifdef INFRA_DEBUG_ENABLE
	{
		DWORD i;
		printf("FT_Channel_Write called with noOfBytes=%u\n",(unsigned)noOfBytes);
		for (i = 0; i < noOfBytes; i++)
		{
			printf(" 0x%x", buffer[i]);
		}
		printf("\n");
	}
#endif

	status = varFunctionPtrLst.p_FT_Write(handle, buffer, noOfBytes, noOfBytesTransferred);

	FN_EXIT;
  	return status;
}


/*
*\brief Check if the device has MPSSE
* This function looks for the device type if it matches then look for the Location Id to determine
* the availability of MPSSE if  matches then returns 1 otherwise returns 0
*\Param[in] FT_DEVICE_LIST_INFO_NODE device info node wich contains the information about the device
*\return bool return a boolean value
*/
bool Mid_CheckMPSSEAvailable(FT_DEVICE_LIST_INFO_NODE devList)
{
	FT_STATUS status = FT_OK;
	(void)status;
	bool isMPSSEAvailable = MID_NO_MPSSE;
	FN_ENTER;

	size_t  los = strlen(devList.Description);

	/*check TYPE field*/
	switch(devList.Type)
	{
		case FT_DEVICE_2232C:
			if (devList.Description[los-1] == 0x41)   //Last character = 0x41 = ASCII "A"
			{
				isMPSSEAvailable =	MID_MPSSE_AVAILABLE;
			}
			break;
		case FT_DEVICE_2232H:
		case FT_DEVICE_2233HP:
		case FT_DEVICE_2232HP:
		case FT_DEVICE_2232HA:
			if ((devList.Description[los - 1] == 0x41) || (devList.Description[los - 1] == 0x42))  //Last character = 0x41 = ASCII "A", 0x42 = ASCII "B"
			{
				isMPSSEAvailable =	MID_MPSSE_AVAILABLE;
			}
			break;
		case FT_DEVICE_4232H:
		case FT_DEVICE_4233HP:
		case FT_DEVICE_4232HP:
		case FT_DEVICE_4232HA:
			if ((devList.Description[los - 1] == 0x41) || (devList.Description[los - 1] == 0x42))  //Last character = 0x41 = ASCII "A", 0x42 = ASCII "B"
			{
				isMPSSEAvailable =	MID_MPSSE_AVAILABLE;
			}
			break;
		case FT_DEVICE_232H:
		case FT_DEVICE_232HP:
		case FT_DEVICE_233HP:
				isMPSSEAvailable =	MID_MPSSE_AVAILABLE;
			break;
		default:
			break;
	};

	FN_EXIT;
	return isMPSSEAvailable;
}


/*!
 * \brief Reset the device
 *
 * This function calles the FT_Reset to reset the device
 * \param[in] handle of the device
 * \return status
 * \sa
 * \note
 * \warning
 */
FT_STATUS Mid_ResetDevice(FT_HANDLE handle)
{
	FT_STATUS status;
	FN_ENTER;
	status = varFunctionPtrLst.p_FT_ResetDevice(handle);
	FN_EXIT;
	return status;

}

/*!
 * \brief Purge the device
 *
 * This function clears the Input and Output buffer of the device
 * \param[in] handle Handle of the channel
 * \return status
 * \sa
 * \note
 * \warning
 */
FT_STATUS Mid_PurgeDevice (FT_HANDLE handle)
{
	FT_STATUS status;
	FN_ENTER;
	status = varFunctionPtrLst.p_FT_Purge(handle, FT_PURGE_RX | FT_PURGE_TX);
	FN_EXIT;
	return status;
}


/*!
 * \brief Sets the Input and Output buffer size
 *
 * This function sets the Input and Output buffer size as requested in the parameter.
 *
 * \param[in] handle Handle of the channel
 * \param[in] inputBufSize Size of the Input buffer
 * \param[in] outputBufSize Size of the Output buffer
 * \return status
 * \sa
 * \note
 * \warning
 */
FT_STATUS Mid_SetUSBParameters(FT_HANDLE handle, DWORD inputBufSize, DWORD outputBufSize)
{
	FT_STATUS status;

	FN_ENTER;

	status = varFunctionPtrLst.p_FT_SetUSBParameters(handle, inputBufSize, outputBufSize);

	FN_EXIT;
	return status;
}

/*!
 * \brief Sets  the events and error characters
 *
 * This function sets  the events and special characters as requested in the parameter.
 * \param[in] handle Handle of the channel
 * \param[in] eventCh event char to set
 * \param[in] eventStatus 0 if event character disabled, non-zero otherwise
 * \param[in] errorCh error char to set
 * \param[in] errorStatus 0 if error character disabled, non-zero otherwise
 * \return status
 * \sa
 * \note
 * \warning
 */
FT_STATUS Mid_SetDeviceSpecialChar(FT_HANDLE handle, UCHAR eventCh,
						UCHAR eventStatus, UCHAR errorCh, UCHAR errorStatus)
{
	FT_STATUS status;

	FN_ENTER;

	status = varFunctionPtrLst.p_FT_SetChars(handle, eventCh, eventStatus, errorCh, errorStatus);

	FN_EXIT;
	return status;
}


/*!
 * \brief sets read and write timeout
 *
 * This function set the read and write timeout as requested in the parameter
 *
 * \param[in] handle Handle of the channel
 * \param[in] rdTimeOut timeout valu for read
 * \param[in] wrTimeOut timeout valu for write
 * \return status
 * \sa
 * \note
 * \warning
 */
FT_STATUS Mid_SetDeviceTimeOut(FT_HANDLE handle, DWORD rdTimeOut, DWORD wrTimeOut)
{
	FT_STATUS status;

	FN_ENTER;

	status = varFunctionPtrLst.p_FT_SetTimeouts(handle, rdTimeOut, wrTimeOut);

	FN_EXIT;
	return status;
}

 /*!
  * \brief sets the latency timer
  *
  * This function set the latency timer for the device as requested in the parameter

  * \param[in] handle Handle of the channel
  * \param[in] milliSecond time in millisecond to be set for latencytimer
  * \return status
  * \sa
  * \note
  * \warning
  */
FT_STATUS Mid_SetLatencyTimer (FT_HANDLE handle, UCHAR milliSecond)
{
	FT_STATUS status;

	FN_ENTER;

	status = varFunctionPtrLst.p_FT_SetLatencyTimer(handle, milliSecond);

	FN_EXIT;
	return status;
 }

/*!
 * \brief Set the bit mode to zero
 *
 * This function sets the bit mode to 0
 * \param[in] handle Handle of the channel
 * \return status
 * \sa
 * \note
 * \warning
 */
FT_STATUS Mid_ResetMPSSE(FT_HANDLE handle)
{
	FT_STATUS status;

	FN_ENTER;

	status = varFunctionPtrLst.p_FT_SetBitmode(handle, INTERFACE_MASK_IN, RESET_INTERFACE);

	FN_EXIT;
	return status;
}

/*!
 * \brief enable MPSSE
 *
 * This function set the bit 2 to enable the MPSSE
 * \param[in] handle Handle of the channel
 * \return status
 * \sa
 * \note
 * \warning
 */
FT_STATUS Mid_EnableMPSSEIn(FT_HANDLE handle)
{
	FT_STATUS status;

	FN_ENTER;

	status = varFunctionPtrLst.p_FT_SetBitmode(handle, INTERFACE_MASK_IN, ENABLE_MPSSE);

	FN_EXIT;
	return status;
}

/*!
 * \brief syncronize MPSSE Channel
 *
 * This function syncronice the MPSSE by continiously sending 0XAA until it is it is readback
 *and then sends 0XAB once and read until we get it back
 * \param[in] handle Handle of the channel
 * \return status
 * \sa
 * \note
 * \warning
 */
FT_STATUS Mid_SyncMPSSE(FT_HANDLE handle)
{
	FT_STATUS status;

	UCHAR cmdEchoed;

	FN_ENTER;

	status = Mid_EmptyDeviceInputBuff(handle);
	CHECK_STATUS(status);
	/*send and receive command*/
	status = Mid_SendReceiveCmdFromMPSSE(handle, MID_ECHO_COMMAND_CONTINUOUSLY,	MID_ECHO_CMD_1, &cmdEchoed);
	CHECK_STATUS(status);

	if (cmdEchoed == MID_CMD_ECHOED)
	{
        status = Mid_SendReceiveCmdFromMPSSE(handle, MID_ECHO_COMMAND_ONCE,	MID_ECHO_CMD_2, &cmdEchoed);
		CHECK_STATUS(status);
		if (cmdEchoed != MID_CMD_ECHOED)
		{
			status = FT_OTHER_ERROR;
		}
	}
	else
	{
		status = FT_OTHER_ERROR;
    }

	FN_EXIT;
	return status;
}



/*!
 * \brief sends the data and read it back from the device
 *
 * This function sends the ecoCmd based on the ecoCmdFlag(continiously/Once) and reads it back
 * \param[in] handle Handle of the channel
 * \param[in] echoCmdFlag specifies whether to sen char continiously or once
 * \param[in] ecoCmd char to be sent
 * \param[out] cmdEchoed 1 if char echoed from device otherwise 0
 * \return status
 * \sa
 * \note
 * \warning
 */



FT_STATUS Mid_SendReceiveCmdFromMPSSE(FT_HANDLE handle, UCHAR echoCmdFlag, UCHAR ecoCmd, UCHAR *cmdEchoed)
{
	FT_STATUS status;
	DWORD bytesInInputBuf = 0;
	DWORD numOfBytesRead = 0;
	DWORD bytesWritten;
	DWORD byteCounter;
	UCHAR cmdResponse = MID_CMD_NOT_ECHOED;
	int loopCounter = 0;
	UCHAR *readBuffer = NULL;

	FN_ENTER;

	readBuffer = (UCHAR*)INFRA_MALLOC(MID_MAX_IN_BUF_SIZE);
	if (NULL == readBuffer)
	{
		return FT_INSUFFICIENT_RESOURCES;
	}
	/*initialize cmdEchoed to MID_CMD_NOT_ECHOED*/
	*cmdEchoed = MID_CMD_NOT_ECHOED;
	/* check whether command has to be sent only once*/
	if (echoCmdFlag == MID_ECHO_COMMAND_ONCE)
	{
		status = varFunctionPtrLst.p_FT_Write(handle,&ecoCmd, 1,&bytesWritten);
		CHECK_STATUS(status);
	}

	do
	{
		DBG(MSG_DEBUG,"In Loop Mid_SendReceiveCmdFromMPSSE loopCounter = %d \n", loopCounter);
		/*check whether command has to be sent every time in the loop*/
		if (echoCmdFlag == MID_ECHO_COMMAND_CONTINUOUSLY)
		{
		  status = varFunctionPtrLst.p_FT_Write(handle,&ecoCmd, 1,&bytesWritten);
		 CHECK_STATUS(status);
		}
		/*read the no of bytes available in Receive buffer*/
		status = varFunctionPtrLst.p_FT_GetQueueStatus(handle,&bytesInInputBuf);
		CHECK_STATUS(status);
		INFRA_SLEEP(1);
		DBG(MSG_DEBUG,"bytesInInputBuf size =  %d\n", bytesInInputBuf);
		if (bytesInInputBuf >0)
		{
			MID_CHK_IN_BUF_OK(bytesInInputBuf);
			status = varFunctionPtrLst.p_FT_Read(handle, readBuffer, bytesInInputBuf,&numOfBytesRead);
			CHECK_STATUS(status);
			if (numOfBytesRead >0)
			{
				byteCounter = 0;
				do
				{
					if (byteCounter <= (numOfBytesRead-1))
                    {
						if (readBuffer[byteCounter]==MID_BAD_COMMAND_RESPONSE)
                        {
							cmdResponse = MID_BAD_COMMAND_RESPONSE;
						}
						else
						{
							if (cmdResponse == MID_BAD_COMMAND_RESPONSE)
							{
								if (readBuffer[byteCounter]==ecoCmd)
								{
                                    *cmdEchoed = MID_CMD_ECHOED;
                                }
							}
							cmdResponse = MID_CMD_NOT_ECHOED;
                        }
					}
					byteCounter++;
				}while((byteCounter < bytesInInputBuf)&&(*cmdEchoed == MID_CMD_NOT_ECHOED));
			}
		}

		/*for breaking the loop */
		loopCounter++;
		if (loopCounter > MID_MAX_IN_BUF_SIZE)
		{
			DBG(MSG_DEBUG,"Loop breaked after executing %u times\n",(unsigned)MID_MAX_IN_BUF_SIZE);
			status = FT_OTHER_ERROR;
			break;
		}
	}while((*cmdEchoed == MID_CMD_NOT_ECHOED) && (status == FT_OK));
	if (NULL != readBuffer)
	{
		INFRA_FREE(readBuffer);
	}

	FN_EXIT;
    return status;
}


/*!
 * \brief sets the pin state
 *
 * This function  sets the pin state to 0x13 for low bits data and 0x0f for high bit data
 * \param[in] handle Handle of the channel
 * \return status
 * \sa
 * \note
 * \warning
 */
FT_STATUS Mid_SetGPIOLow(FT_HANDLE handle, uint8 value, uint8 direction)
{
	FT_STATUS status = FT_OK;
	(void)status;
	UCHAR inputBuffer[10];
	DWORD bytesWritten = 0;
	DWORD bufIdx = 0;

	FN_ENTER;

	inputBuffer[bufIdx++] = MID_SET_LOW_BYTE_DATA_BITS_CMD;
	inputBuffer[bufIdx++] = value;//0x13;
	inputBuffer[bufIdx++] = direction;//0x13;

	FN_EXIT;

	return varFunctionPtrLst.p_FT_Write(handle, inputBuffer, bufIdx,&bytesWritten);

}

FT_STATUS Mid_GetFtDeviceType(FT_HANDLE handle, FT_DEVICE *ftDevice)
{
	FT_STATUS status = FT_OTHER_ERROR;
	DWORD deviceID;
	CHAR pSerialNumber[32]; // Maximum size 16
	CHAR pDescription[128]; // Maximum size 64
	VOID *Dummy = NULL;

	FN_ENTER;
	status = varFunctionPtrLst.p_FT_GetDeviceInfo(handle, ftDevice, &deviceID, \
		(PCHAR)pSerialNumber, (PCHAR)pDescription, Dummy);

#ifdef INFRA_DEBUG_ENABLE
	switch(*ftDevice)
	{
		case FT_DEVICE_BM:
			DBG(MSG_DEBUG,"Detected device is a BM chip\n");
			break;
		case FT_DEVICE_AM:
			DBG(MSG_DEBUG,"Detected device is an AM chip\n");
			break;
		case FT_DEVICE_100AX:
			DBG(MSG_DEBUG,"Detected device is a 110AX chip\n");
			break;
		case FT_DEVICE_2232C:
			DBG(MSG_DEBUG,"Detected device is a FT2232C chip\n");
			break;
		case FT_DEVICE_232R:
			DBG(MSG_DEBUG,"Detected device is a FT232R chip\n");
			break;
		case FT_DEVICE_2232H:
			DBG(MSG_DEBUG,"Detected device is a FT2232H chip\n");
			break;
		case FT_DEVICE_4232H:
			DBG(MSG_DEBUG,"Detected device is a FT4232H chip\n");
			break;
		case FT_DEVICE_232H:
			DBG(MSG_DEBUG,"Detected device is a FT232H chip\n");
			break;
		case FT_DEVICE_UNKNOWN:
		default:
		/*If the value of ftDevice is 3 then this may indicate some problem in the driver or the
		setup. If the value of ftDevice is greater than 8 then this may indicate a newer chip*/
			DBG(MSG_DEBUG,"Detected device is an unknown(0x%x) chip\n",\
				*ftDevice);
	}
#endif
	FN_EXIT;
	return status;
}

/*!
 * \brief sets the clock
 *
 * This function fids the device type. based on the device type calculate the value for the clock
 * requested in the parameter and sets it.
 * \param[in] handle Handle of the channel
 * \param[in] clock Clock value to be set
 * \return status
 * \sa
 * \note
 * \warning
 */
FT_STATUS Mid_SetClock(FT_HANDLE handle, FT_DEVICE ftDevice, uint32 clock)

{
    UCHAR inputBuffer[10];
	DWORD bytesWritten = 0;
	DWORD bufIdx = 0;
	uint8 valueH, valueL;
	uint32 value;
	FT_STATUS status;

	FN_ENTER;
	switch(ftDevice)
	{
		case FT_DEVICE_2232C:/* This is actually FT2232D but defined is FT_DEVICE_2232C
			in D2XX. Also, it is the only FS device that supports MPSSE */
			value = (MID_6MHZ/clock) - 1;
			break;

		default:/* Assuming all new chips will he HS MPSSE devices */
			DBG(MSG_DEBUG,"Unknown device type(0x%x). Assuming high speed device",(unsigned)ftDevice);
			/* Fall through */
		case FT_DEVICE_2232H:
		case FT_DEVICE_4232H:
		case FT_DEVICE_232H:
			DBG(MSG_DEBUG,"handle = 0x%x value = 0x%x DISABLE_CLOCK_DIVIDE\n", (unsigned)handle,(unsigned)value);
			value = DISABLE_CLOCK_DIVIDE;
			status = varFunctionPtrLst.p_FT_Write(handle,&value, 1, &bytesWritten);
			CHECK_STATUS(status);
			value = (MID_30MHZ/clock) - 1;
			break;
	}
	/*calculate valueH and ValueL*/
	valueL = (uint8)value;
	valueH = (uint8)(value>>8);
	/*set the clock*/
    inputBuffer[bufIdx++] = MID_SET_CLOCK_FREQUENCY_CMD;
	DBG(MSG_DEBUG,"valueL = 0x%x valueH = 0x%x \n", valueL, valueH);
	inputBuffer[bufIdx++] = valueL;
	inputBuffer[bufIdx++] = valueH;

	FN_EXIT;
	return varFunctionPtrLst.p_FT_Write(handle, inputBuffer, bufIdx,&bytesWritten);
}

/*!
 * \brief enable or disable the loopback
 *
 * This function enables or disables the loopback based on the loopBackFlag
 * \param[in] handle Handle of the channel
 * \param[in] loopBackFlag flag to specify to turn off/on loopback
 * \return status
 * \sa
 * \note
 * \warning
 */
FT_STATUS Mid_SetDeviceLoopbackState(FT_HANDLE handle, uint8 loopBackFlag)
{
	FT_STATUS status = FT_OK;
	(void)status;
    UCHAR inputBuffer[10];
	DWORD bytesWritten = 0;
	DWORD bufIdx = 0;

	FN_ENTER;

	if (loopBackFlag == MID_LOOPBACK_FALSE)
	{
		inputBuffer[bufIdx++] = MID_TURN_OFF_LOOPBACK_CMD;
	}
	else
	{
		inputBuffer[bufIdx++] = MID_TURN_ON_LOOPBACK_CMD;
    }

	FN_EXIT;
   return varFunctionPtrLst.p_FT_Write(handle, inputBuffer, bufIdx,&bytesWritten);
}

/*!
 * \brief empty the device read buffer
 *
 * This function checks if any data available in the Rx buffer if true read all the data available in
 * the buffer
 * \param[in] handle Handle of the channel
 * \return status
 * \sa
 * \note
 * \warning
 */
FT_STATUS Mid_EmptyDeviceInputBuff(FT_HANDLE handle)
{
	FT_STATUS status;
	UCHAR *readBuffer;
	DWORD bytesInInputBuf = 0;
	DWORD numOfBytesRead = 0;

	FN_ENTER;
	readBuffer = (UCHAR*)INFRA_MALLOC(MID_MAX_IN_BUF_SIZE);
	if (NULL == readBuffer)
	{
		return FT_INSUFFICIENT_RESOURCES;
	}
	status = varFunctionPtrLst.p_FT_GetQueueStatus(handle,&bytesInInputBuf);
	CHECK_STATUS(status);
	if (bytesInInputBuf > 0)
	{
		do
		{
			if (bytesInInputBuf >MID_MAX_IN_BUF_SIZE)
			{
				status = varFunctionPtrLst.p_FT_Read(handle, readBuffer,
					MID_MAX_IN_BUF_SIZE,&numOfBytesRead);
				CHECK_STATUS(status);
				bytesInInputBuf = bytesInInputBuf - numOfBytesRead;
			}
			else
			{
				status = varFunctionPtrLst.p_FT_Read(handle, readBuffer,
					bytesInInputBuf,&numOfBytesRead);
				CHECK_STATUS(status);
				bytesInInputBuf = bytesInInputBuf - numOfBytesRead;
			}
		}while((status == FT_OK)&&(bytesInInputBuf!=0));
	}
	if (NULL != readBuffer)
	{
		INFRA_FREE(readBuffer);//Bug12
	}
	FN_EXIT;
	return status;

}


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
FTDIMPSSE_API FT_STATUS FT_WriteGPIO(FT_HANDLE handle, uint8 dir, uint8 value)
{
	FT_STATUS status;
	uint8 buffer[3];
	DWORD bytesWritten = 0;
	DWORD bufIdx = 0;

	FN_ENTER;

	buffer[bufIdx++] = MPSSE_CMD_SET_DATA_BITS_HIGHBYTE;
	buffer[bufIdx++] = value;
	buffer[bufIdx++] = dir;
	status = varFunctionPtrLst.p_FT_Write(handle, buffer, bufIdx,&bytesWritten);

	FN_EXIT;
	return status;
}

/*
 * FTDI libMPSSE is dumb and doesn't have a function for writing the lower GPIO bits which the
 * FT4232H only has. So I'm adding this.
 */
FTDIMPSSE_API FT_STATUS FT_WriteGPIOL(FT_HANDLE handle, uint8 dir, uint8 value)
{
	FT_STATUS status;
	uint8 buffer[3];
	DWORD bytesWritten = 0;
	DWORD bufIdx = 0;

	FN_ENTER;

	buffer[bufIdx++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
	buffer[bufIdx++] = value;
	buffer[bufIdx++] = dir;
	status = varFunctionPtrLst.p_FT_Write(handle, buffer, bufIdx,&bytesWritten);

	FN_EXIT;
	return status;
}

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
FTDIMPSSE_API FT_STATUS FT_ReadGPIO(FT_HANDLE handle, uint8 *value)
{
	FT_STATUS status;
	uint8 buffer[2];
	DWORD bytesTransfered = 0;
	DWORD bytesToTransfer = 0;
	UCHAR readBuffer[10];

	FN_ENTER;

	buffer[bytesToTransfer++] = MPSSE_CMD_GET_DATA_BITS_HIGHBYTE;
	buffer[bytesToTransfer++] = MPSSE_CMD_SEND_IMMEDIATE;
	status = varFunctionPtrLst.p_FT_Write(handle, buffer, bytesToTransfer, &bytesTransfered);
	CHECK_STATUS(status);
	DBG(MSG_DEBUG,"bytesToTransfer = 0x%x bytesTransfered = 0x%x\n", (unsigned)bytesToTransfer, (unsigned)bytesTransfered);
	bytesToTransfer = 1;
	bytesTransfered = 0;
	status = varFunctionPtrLst.p_FT_Read(handle, readBuffer, bytesToTransfer, &bytesTransfered);
	CHECK_STATUS(status);
	DBG(MSG_DEBUG,"bytesToTransfer = 0x%x bytesTransfered = 0x%x\n", (unsigned)bytesToTransfer, (unsigned)bytesTransfered);
	if (bytesToTransfer != bytesTransfered)
		status = FT_IO_ERROR;
	*value = readBuffer[0];

	FN_EXIT;
	return status;
}

FTDIMPSSE_API FT_STATUS FT_ReadGPIOL(FT_HANDLE handle, uint8 *value)
{
	FT_STATUS status;
	uint8 buffer[2];
	DWORD bytesTransfered = 0;
	DWORD bytesToTransfer = 0;
	UCHAR readBuffer[10];

	FN_ENTER;

	buffer[bytesToTransfer++] = MPSSE_CMD_GET_DATA_BITS_LOWBYTE;
	buffer[bytesToTransfer++] = MPSSE_CMD_SEND_IMMEDIATE;
	status = varFunctionPtrLst.p_FT_Write(handle, buffer, bytesToTransfer, &bytesTransfered);
	CHECK_STATUS(status);
	DBG(MSG_DEBUG,"bytesToTransfer = 0x%x bytesTransfered = 0x%x\n", (unsigned)bytesToTransfer, (unsigned)bytesTransfered);
	bytesToTransfer = 1;
	bytesTransfered = 0;
	status = varFunctionPtrLst.p_FT_Read(handle, readBuffer, bytesToTransfer, &bytesTransfered);
	CHECK_STATUS(status);
	DBG(MSG_DEBUG,"bytesToTransfer = 0x%x bytesTransfered = 0x%x\n", (unsigned)bytesToTransfer, (unsigned)bytesTransfered);
	if (bytesToTransfer != bytesTransfered)
		status = FT_IO_ERROR;
	*value = readBuffer[0];

	FN_EXIT;
	return status;
}

