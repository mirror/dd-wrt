/*!
 * \file infra.c
 *
 * \author FTDI
 * \date 20110317
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
 * Module: Infra
 *
 * Revision History:
 * 0.1 - initial version
 * 0.2 - 20110708 - exported Init_libMPSSE & Cleanup_libMPSSE for Microsoft toolchain support
 * 0.3 - 20111103 - commented & cleaned up
 */

/******************************************************************************/
/*								Include files					  			  */
/******************************************************************************/
#define FTDI_EXPORTS
#include "ftdi_infra.h"		/*portable infrastructure(datatypes, libraries, etc)*/


/******************************************************************************/
/*								Macro defines					  			  */
/******************************************************************************/

#ifndef _WIN32
	#define GET_FUNC(libHandle, symbol)	dlsym(libHandle, symbol)
	/* Macro to check if dlsym returned correctly */
	#define CHECK_SYMBOL(exp) {if (!exp)\
		fprintf(stderr, "dlsym failed: %s\n", dlerror());};
#else // _WIN32
	#define GET_FUNC(libHandle, symbol) GetProcAddress(libHandle, symbol)
	#define CHECK_SYMBOL(exp) {if (GetLastError())\
		fprintf(stderr, "GetProcAddress failed: 0x%lx\n", GetLastError());};
#endif // _WIN32

/******************************************************************************/
/*								Global variables							  */
/******************************************************************************/

#ifdef INFRA_DEBUG_ENABLE
	int currentDebugLevel = MSG_INFO;
	//int currentDebugLevel = MSG_DEBUG;
#endif

/* Handle to D2XX driver */
#ifdef _WIN32
	HANDLE hdll_d2xx;
#else // _WIN32
	void *hdll_d2xx;
#endif // _WIN32

InfraFunctionPtrLst varFunctionPtrLst;

/******************************************************************************/
/*								Local function declarations					  */
/******************************************************************************/
#if 0
#ifndef _WIN32
void __attribute__ ((constructor))my_init(void);/*called when lib is loaded*/
void __attribute__ ((destructor))my_exit(void);/*called when lib is unloaded*/
#endif // _WIN32
#endif

/******************************************************************************/
/*						Global function definitions						  */
/******************************************************************************/

/*!
 * \brief Print function return status
 *
 * All the functions return a status code. This function prints a text to the debug terminal
 * that provides the meaning of the status code.
 *
 * \param[in] status Status code returned by functions
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa
 * \note
 * \warning
 */
FT_STATUS Infra_DbgPrintStatus(FT_STATUS status)
{
	FN_ENTER;

	switch(status)
	{
		case FT_OK:
			DBG(MSG_ERR, "Status: FT_OK\n");
		break;

		case FT_INVALID_HANDLE:
			DBG(MSG_ERR, "Status: FT_INVALID_HANDLE\n");
		break;

		case FT_DEVICE_NOT_FOUND:
			DBG(MSG_ERR, "Status: FT_DEVICE_NOT_FOUND\n");
		break;

		case FT_DEVICE_NOT_OPENED:
			DBG(MSG_ERR, "Status: FT_DEVICE_NOT_OPENED\n");
		break;

		case FT_IO_ERROR:
			DBG(MSG_ERR, "Status: FT_IO_ERROR\n");
		break;

		case FT_INSUFFICIENT_RESOURCES:
			DBG(MSG_ERR, "Status: FT_INSUFFICIENT_RESOURCES\n");
		break;

		case FT_INVALID_PARAMETER:
			DBG(MSG_ERR, "Status: FT_INVALID_PARAMETER\n");
		break;

		case FT_INVALID_BAUD_RATE:
			DBG(MSG_ERR, "Status: FT_INVALID_BAUD_RATE\n");
		break;

		case FT_DEVICE_NOT_OPENED_FOR_ERASE:
			DBG(MSG_ERR, "Status: FT_DEVICE_NOT_OPENED_FOR_ERASE\n");
		break;

		case FT_DEVICE_NOT_OPENED_FOR_WRITE:
			DBG(MSG_ERR, "Status: FT_DEVICE_NOT_OPENED_FOR_WRITE\n");
		break;

		case FT_FAILED_TO_WRITE_DEVICE:
			DBG(MSG_ERR, "Status: FT_FAILED_TO_WRITE_DEVICE\n");
		break;

		case FT_EEPROM_READ_FAILED:
			DBG(MSG_ERR, "Status: FT_EEPROM_READ_FAILED\n");
		break;

		case FT_EEPROM_WRITE_FAILED:
			DBG(MSG_ERR, "Status: FT_EEPROM_WRITE_FAILED\n");
		break;

		case FT_EEPROM_ERASE_FAILED:
			DBG(MSG_ERR, "Status: FT_EEPROM_ERASE_FAILED\n");
		break;

		case FT_EEPROM_NOT_PRESENT:
			DBG(MSG_ERR, "Status: FT_EEPROM_NOT_PRESENT\n");
		break;

		case FT_EEPROM_NOT_PROGRAMMED:
			DBG(MSG_ERR, "Status: FT_EEPROM_NOT_PROGRAMMED\n");
		break;

		case FT_INVALID_ARGS:
			DBG(MSG_ERR, "Status: FT_INVALID_ARGS\n");
		break;

		case FT_NOT_SUPPORTED:
			DBG(MSG_ERR, "Status: FT_NOT_SUPPORTED\n");
		break;

		case FT_OTHER_ERROR:
			DBG(MSG_ERR, "Status: FT_OTHER_ERROR\n");
		break;

#ifndef __linux__
/* gives compilation error in linux - not defined in D2XX for linux */
		case FT_DEVICE_LIST_NOT_READY:
			DBG(MSG_ERR, "Status: FT_DEVICE_LIST_NOT_READY\n");
#endif
		break;
			DBG(MSG_ERR, "Status: Unknown Error!\n");
		default:

			;

	}
	FN_EXIT;
	return FT_OK;
}


/******************************************************************************/
/*						Local function definitions						  */
/*!
 * \brief Delay the execution of the thread
 *
 * Delay the execution of the thread
 *
 * \param[in] delay Value of the delay in milliseconds
 * \return Returns status code of type FT_STATUS(see D2XX Programmer's Guide)
 * \sa
 * \note The macro INFRA_SLEEP has a resolution of 1 second
 * \warning
 */
FT_STATUS Infra_Delay(uint64 delay)
{
	FT_STATUS status = FT_OTHER_ERROR;
	FN_ENTER;

#ifdef _WIN32
/*TBD*/
	/*status = FT_OK;*/
#else // _WIN32
/*TBD*/
	/*status = FT_OK;*/
#endif

	FN_EXIT;
	return status;
}

/******************************************************************************/
/*						Local function definitions						  */
/******************************************************************************/

FTDIMPSSE_API void Init_libMPSSE(void)
{
	FT_STATUS status;
	(void)status;
	FN_ENTER;

/* Load D2XX dynamic library */
#ifndef _WIN32
	hdll_d2xx = dlopen("libftd2xx.so", RTLD_LAZY);
	if (!hdll_d2xx)
	{
		fprintf(stderr, "dlopen failed: %s\n", dlerror());
	}
#else // _WIN32
	hdll_d2xx = LoadLibrary("ftd2xx.dll");
#endif // _WIN32

	CHECK_NULL(hdll_d2xx);

	varFunctionPtrLst.p_FT_GetLibraryVersion = (pfunc_FT_GetLibraryVersion)GET_FUNC(hdll_d2xx, "FT_GetLibraryVersion");
	CHECK_SYMBOL(varFunctionPtrLst.p_FT_GetLibraryVersion);
	/*FunctionPointer for FT_CreateDeviceInfoList*/
	varFunctionPtrLst.p_FT_GetNumChannel = (pfunc_FT_GetNumChannel)GET_FUNC(hdll_d2xx,"FT_CreateDeviceInfoList");
	CHECK_SYMBOL(varFunctionPtrLst.p_FT_GetNumChannel);
	/*function Pointer for FT_GetDeviceInfoList */
	varFunctionPtrLst.p_FT_GetDeviceInfoList = (pfunc_FT_GetDeviceInfoList)GET_FUNC(hdll_d2xx,"FT_GetDeviceInfoList");
	CHECK_SYMBOL(varFunctionPtrLst.p_FT_GetDeviceInfoList);
	/*open*/
	varFunctionPtrLst.p_FT_Open = (pfunc_FT_Open)GET_FUNC(hdll_d2xx,"FT_Open");
	CHECK_SYMBOL(varFunctionPtrLst.p_FT_Open);
	/*close*/
	varFunctionPtrLst.p_FT_Close = (pfunc_FT_Close)GET_FUNC(hdll_d2xx,"FT_Close");
	CHECK_SYMBOL(varFunctionPtrLst.p_FT_Close);
	/*Reset*/
	varFunctionPtrLst.p_FT_ResetDevice = (pfunc_FT_ResetDevice)GET_FUNC(hdll_d2xx, "FT_ResetDevice");
	CHECK_SYMBOL(varFunctionPtrLst.p_FT_ResetDevice);
	/*Purge*/
	varFunctionPtrLst.p_FT_Purge = (pfunc_FT_Purge)GET_FUNC(hdll_d2xx,"FT_Purge");
	CHECK_SYMBOL(varFunctionPtrLst.p_FT_Purge);
	/*SetUSBParameters*/
	varFunctionPtrLst.p_FT_SetUSBParameters = (pfunc_FT_SetUSBParameters)GET_FUNC(hdll_d2xx,"FT_SetUSBParameters");
	CHECK_SYMBOL(varFunctionPtrLst.p_FT_SetUSBParameters);
	/*SetChars*/
	varFunctionPtrLst.p_FT_SetChars = (pfunc_FT_SetChars)GET_FUNC(hdll_d2xx,"FT_SetChars");
	CHECK_SYMBOL(varFunctionPtrLst.p_FT_SetChars);
	/*SetTimeouts*/
	varFunctionPtrLst.p_FT_SetTimeouts = (pfunc_FT_SetTimeouts)GET_FUNC(hdll_d2xx,"FT_SetTimeouts");
	CHECK_SYMBOL(varFunctionPtrLst.p_FT_SetTimeouts);
	/*GetLatencyTimer*/
    varFunctionPtrLst.p_FT_GetLatencyTimer = (pfunc_FT_GetLatencyTimer)GET_FUNC(hdll_d2xx,"FT_GetLatencyTimer");
	CHECK_SYMBOL(varFunctionPtrLst.p_FT_GetLatencyTimer);
	/*SetLatencyTimer*/
    varFunctionPtrLst.p_FT_SetLatencyTimer = (pfunc_FT_SetLatencyTimer)GET_FUNC(hdll_d2xx,"FT_SetLatencyTimer");
	CHECK_SYMBOL(varFunctionPtrLst.p_FT_SetLatencyTimer);
	/*SetBitmode*/
	varFunctionPtrLst.p_FT_SetBitmode = (pfunc_FT_SetBitmode)GET_FUNC(hdll_d2xx,"FT_SetBitMode");
	CHECK_SYMBOL(varFunctionPtrLst.p_FT_SetBitmode);
	/*FT_GetQueueStatus*/
	varFunctionPtrLst.p_FT_GetQueueStatus = (pfunc_FT_GetQueueStatus)GET_FUNC(hdll_d2xx,"FT_GetQueueStatus");
	CHECK_SYMBOL(varFunctionPtrLst.p_FT_GetQueueStatus);
	/*FT_Read*/
	varFunctionPtrLst.p_FT_Read = (pfunc_FT_Read)GET_FUNC(hdll_d2xx,"FT_Read");
	CHECK_SYMBOL(varFunctionPtrLst.p_FT_Read);
	/*FT_Write*/
	varFunctionPtrLst.p_FT_Write = (pfunc_FT_Write)GET_FUNC(hdll_d2xx,"FT_Write");
	CHECK_SYMBOL(varFunctionPtrLst.p_FT_Write);
	/*FT_GetDeviceInfo*/
	varFunctionPtrLst.p_FT_GetDeviceInfo = (pfunc_FT_GetDeviceInfo)GET_FUNC(hdll_d2xx,"FT_GetDeviceInfo");
	CHECK_SYMBOL(varFunctionPtrLst.p_FT_GetDeviceInfo);

	/*Call module specific initialization functions from here(if at all they are required)
		Example:
			Top_Init();	//This may be a function in ftdi_common.c
						// Inside this function we may have macros (eg: #ifdef(_I2C))

			Mid_Init();
	*/

	FN_EXIT;
}

FTDIMPSSE_API void Cleanup_libMPSSE(void)
{
	FT_STATUS status = FT_OK;
	(void)status;
	FN_ENTER;
#ifdef _WIN32
	if (NULL != hdll_d2xx)
	{
		if (FreeLibrary(hdll_d2xx)>0)
		{
			DBG(MSG_DEBUG, "D2XX unloaded\n");
		}
		else
		{
			DBG(MSG_DEBUG, "failed unloading D2XX\n");
		}
	}
	else
	{
		DBG(MSG_INFO, "handle to D2XX is NULL\n");
	}
#endif // _WIN32

	FN_EXIT;
}


#ifdef _WIN32

/*!
 * \brief Module entry point for Windows DLL
 *
 * This function is called by Windows OS when an application loads/unloads libMPSSE as a DLL
 *
 * \param[in] hModule			Handle
 * \param[in] reason_for_call	Reason for being called
 * \param[in] lpReserved		Reserved
 * \return none
 * \sa
 * \note
 * \warning
 */
BOOL APIENTRY DllMain(HANDLE hModule, DWORD reason_for_call, LPVOID lpReserved)
{
	FN_ENTER;

    switch (reason_for_call)
  	{
		case DLL_PROCESS_ATTACH:
			DBG(MSG_DEBUG,"reason_for_call = DLL_PROCESS_ATTACH\n");
			Init_libMPSSE();
		break;
		case DLL_THREAD_ATTACH:
			DBG(MSG_DEBUG,"reason_for_call = DLL_THREAD_ATTACH\n");

      	break;

		case DLL_THREAD_DETACH:
			DBG(MSG_DEBUG,"reason_for_call = DLL_THREAD_DETACH\n");

		break;
		case DLL_PROCESS_DETACH:
			DBG(MSG_DEBUG,"reason_for_call = DLL_PROCESS_DETACH\n");
			Cleanup_libMPSSE();
		break;

		default:
			DBG(MSG_WARN,"DllMain was called with an unknown reason\n");
    }

	FN_EXIT;
    return TRUE;
}
#endif /*_WIN32*/


#if 0
/*!
 * \brief Module entry point for Windows Static Library and Linux
 * Dynamic & Static Libraries
 *
 * This function is the entry point for the module when it is loaded as a
 * static library(libMPSSE.lib or libMPSSE.a) in windows, and when it is loaded
 * as either a static library(libMPSSE.a) or a dynamic library(libMPSSE.so) in
 * linux
 *
 * \param[in] none
 * \param[out] none
 * \return none
 * \sa
 * \note
 * \warning
 */
void my_init(void)
{
	FT_STATUS status = FT_OK;
	(void)status;

	FN_ENTER;

	Init_libMPSSE();

	FN_EXIT;
}

/*!
 * \brief Module exit point for Windows Static Library and Linux
 * Dynamic & Static Libraries
 *
 * This function is the exit point for the module when it is loaded as a
 * static library(libMPSSE.lib or libMPSSE.a) in windows, and when it is loaded
 * as either a static library(libMPSSE.a) or a dynamic library(libMPSSE.so) in
 * linux
 *
 * \param[in] none
 * \param[out] none
 * \return none
 * \sa
 * \note
 * \warning
 */
void my_exit(void)
{
	FT_STATUS status = FT_OK;
	(void)status;

	FN_ENTER;

	Cleanup_libMPSSE();

	FN_EXIT;
}
#endif

/******************************************************************************/
/*						Public function definitions						  */
/******************************************************************************/

/**
 * Construct a DWORD with major, minor and build version numbers.
 *
 * The format matches that published in the D2XX Programmer's Guide,
 * where each individual byte contains a value which, when displayed
 * as hexadecimal, represents the version number.  For example, "15"
 * is stored as 21.
 *
 * @return Version numbers as a single DWORD.
 */
static DWORD versionNumberToHex(void)
{
#if !defined(FT_VER_MAJOR) || !defined(FT_VER_MINOR) || !defined(FT_VER_BUILD)
    #error "The Makefile must define major, minor and build version numbers."
    return (DWORD)0;
#else
    char  buf[7]; /* enough for '123456' and terminator */
    char *endPtr = NULL;
    long int versionNumber;

#define COMPILE_TIME_ASSERT(pred) switch(0){case 0:case pred:;}
    COMPILE_TIME_ASSERT(FT_VER_MAJOR < 99);
    COMPILE_TIME_ASSERT(FT_VER_MINOR < 99);
    COMPILE_TIME_ASSERT(FT_VER_BUILD < 99);

#ifdef _WIN32
    sprintf_s(buf, 7, "%02d%02d%02d", FT_VER_MAJOR, FT_VER_MINOR, FT_VER_BUILD);
#else
    sprintf(buf, "%02d%02d%02d", FT_VER_MAJOR, FT_VER_MINOR, FT_VER_BUILD);
#endif
    versionNumber = strtol(buf, &endPtr, 16);

    return (DWORD)versionNumber;

#endif /* !defined(FT_VER_MAJOR) || !defined(FT_VER_MINOR) || !defined(FT_VER_BUILD) */
}

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
FTDIMPSSE_API FT_STATUS Ver_libMPSSE(LPDWORD libmpsse, LPDWORD libftd2xx)
{
	FT_STATUS status = FT_INVALID_PARAMETER;

	FN_ENTER;

    if ((libmpsse) && (libftd2xx))
    {
        *libmpsse = versionNumberToHex();

		CHECK_NULL_RET(varFunctionPtrLst.p_FT_GetLibraryVersion);
		status = varFunctionPtrLst.p_FT_GetLibraryVersion(libftd2xx);
	}

	FN_EXIT;
	return status;
}



