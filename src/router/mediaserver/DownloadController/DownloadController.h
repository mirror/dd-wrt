/*
 * INTEL CONFIDENTIAL
 * Copyright (c) 2002 - 2006 Intel Corporation.  All rights reserved.
 * 
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel
 * Corporation or its suppliers or licensors.  Title to the
 * Material remains with Intel Corporation or its suppliers and
 * licensors.  The Material contains trade secrets and proprietary
 * and confidential information of Intel or its suppliers and
 * licensors. The Material is protected by worldwide copyright and
 * trade secret laws and treaty provisions.  No part of the Material
 * may be used, copied, reproduced, modified, published, uploaded,
 * posted, transmitted, distributed, or disclosed in any way without
 * Intel's prior express written permission.
 
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you
 * by disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise. Any license
 * under such intellectual property rights must be express and
 * approved by Intel in writing.
 * 
 * $Workfile: DownloadController.h
 *
 *
 *
 */

#ifndef __DOWNLOAD_CONTROLLER_H__
#define __DOWNLOAD_CONTROLLER_H__

#include "CdsObject.h"
#include "DlnaHttpClient.h"
/*! \file DownloadController.h 
	\brief This is the DMD component for supporting the download capabilities on CDS servers and
	saves the streamed content to a local file system.
*/

/*! \defgroup DownloadController DLNA - Download Controller
	\brief 
	This module provides the functionality for downloading the 
	\ref CdsResource from the DMS to local file system. 

	\{
*/

/*!	\brief The session object created from \ref DMD_DownloadObject.
 */
typedef void* DMD_Session;

/*!	\brief Enumeration of common DMD error codes.
 */

enum DMD_Errors
{
	/*!	\brief No errors.
	*/
	DMD_ERROR_None				= 0,
	/*!	\brief Transfer aborted or failed.
	*/
	DMD_ERROR_FailedOrAborted	= 1,
	/*!	\brief File does not exist on server.
	*/
	DMD_ERROR_FileNotExist		= 2,
	/*!	\brief Http errors.
	*/
	DMD_ERROR_HttpError			= 3,
	/*!	\brief Unknown error.
	*/
	DMD_ERROR_Unknown			= 4
};

/*! \brief Method signature for handling finsihed HTTP downloads from the DMS.
	\param[in] DMU_Errors error_code Error returned for the download request indicated in \ref DMD_Errors.
	\param[in] http_error_code This is http error code that represents the response of the HTTP GET request
	a sucessful upload would return the 200 http reponse.
	\param[in] user_obj	The user object specified in \ref DMD_DownloadObject method.
*/

typedef void (*DMD_OnDownloadResponse)(DMD_Session session, enum DMD_Errors error_code, int http_error_code, void* user_obj);

/*!	\brief Initialize the DMD on the UPnP chain. 

	\param[in] chain The UPnP chain of the microstack.
	\param[in] thread_pool The ThreadPool for doing disk I/O processing.

*/
void DMD_Initialize(void* chain, void* thread_pool);

/*!	\brief Allows the application to check whether the server supports resume using RANGE requests.
	\param[in] session The download session initiated from\ref DMD_DownloadObject.

	\returns The 1 if resume is supported, 0 otherwise.
*/
int DMD_IsResumeSupported(DMD_Session session);

/*!	\brief Allows the application to download a CDS resource from an CDS object on the DMS to the local file system.

	\param[in] download_this CDS res element of the CDS object that would liked to be downloaded.
	\param[in] resume_flag This flag indicates whether you want to retry/resume or just overwrite the file.
	Zero value means retrying by overwriting the files, non-zero values mean resume by appending to the file.
	\param[in] save_as_file_name The local file path where the Res element is saved to.
	\param[in] callback_response The callback to execute when the download completes or an error occurs.
	\param[in] user_obj	The user object specified in the application which is passed back on the \ref DMD_OnDownloadResponse. 

	returns The \ref DMD_Session object.
*/
DMD_Session DMD_DownloadObject(const struct CdsResource* download_this, int resume_flag, char* save_as_file_name, DMD_OnDownloadResponse callback_response, void* user_obj);

/*!	\brief Allows the application to get the current download progress in the number of bytes received.
	\param[in] session The download session initiated from\ref DMD_DownloadObject.

	\returns The number of bytes downloaded.
*/
int DMD_GetBytesReceived(DMD_Session session);

/*!	\brief Allows the application to get the total number of bytes needed to be downloaded.
	\param[in] session The upload session initiated from\ref DMD_DownloadObject.

	\returns The number of total bytes uploaded, which is the file size. If size is unknown
	it will be -1.
*/
int DMD_GetTotalBytesExpected(DMD_Session session);

/*!	\brief Find out if the download session is paused by disconnecting.  Which means the client will disconnect
	from the server and have to resume only using \ref DMD_ResumeTransfer function.
	\param[in] session The upload session initiated from\ref DMD_DownloadObject.

	\returns 1 if session is paused by disconnecting, 0 otherwise.
*/
int DMD_IsPaused(DMD_Session session);

/*!	\brief Find out if the download session is paused using TCP flow control, which it will keep the connection
	to the server but will not receive any bytes by stalling.  The client can only resume using
	\ref DMD_ResumeTransferByFlowControl function.
	\param[in] session The upload session initiated from\ref DMD_DownloadObject.

	\returns 1 if session is paused using TCP flow control, 0 otherwise.
*/
int DMD_IsPausedByFlowControl(DMD_Session session);

/*!	\brief Pause download session by disconnecting and resume using RANGE.  Which means the client will disconnect
	from the server and have to resume only using \ref DMU_ResumeTransfer function.
	\warning Do not use \ref DMD_ResumeTransferByFlowControl to resume the connection. Only use
	DMD_ResumeTransfer which will issue an HTTP-GET request using CONTENT-RANGE on last byte position.
	\warning This function will only work if the server supports RANGE capabiliity.

	\param[in] session The upload session initiated from\ref DMU_CreateObjectAndUpload.

	\returns A non-zero value if successful.
*/
int	DMD_PauseTransfer(DMD_Session session);

/*!	\brief Pause upload session using TCP flow control.  which it will keep the connection
	to the server but will not receive any bytes.
	\warning Must use DMD_ResumeTransferByFlowControl and resume within 5 mins, or the Server could terminiate
	the connection.

	\param[in] session The upload session initiated from\ref DMD_DownloadObject.

	\returns A non-zero value if successful.
*/
int	DMD_PauseTransferByFlowControl(DMD_Session session);

/*!	\brief Allows the application to resume the download session from the last downloaded byte position.  It will
	create a new HTTP-GET request to the server using RANGE from the last byte position.
	\warning If the connection is paused using \ref DMD_PauseTransferByFlowControl, then do not use method to resume,
	resume.

	\param[in] session The upload session initiated from\ref DMD_DownloadObject.

	\returns A non-zero value if successful.
*/

int DMD_ResumeTransfer(DMD_Session session);

/*!	\brief Allows the application to resume the download session using TCP flow control. which it will 
	resume the current connection to the server.

	\param[in] session The upload session initiated from\ref DMD_DownloadObject.

	\returns A non-zero value if successful.
*/
int DMD_ResumeTransferByFlowControl(DMD_Session session);

/*!	\brief Allows the application to abort the transfer progress of the download session.  This will close
	the connection on the server.  The \ref DMD_OnDownloadResponse callback will be called to notify
	the application that the transfer aborted.
	\warning If the session is paused using \ref DMD_PauseTransfer, you still need to call this function
	to abort the transfer, although the connection is already aborted during pause, this will make sure
	that session object is destroyed and the callback is executed.

	\param[in] session The upload session initiated from\ref DMD_DownloadObject.

	\returns A non-zero value if successful.
*/
int DMD_AbortTransfer(DMD_Session session);

/*! \} */

#endif

