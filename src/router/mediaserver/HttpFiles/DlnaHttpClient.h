/*
 * INTEL CONFIDENTIAL
 * Copyright (c) 2002 - 2005 Intel Corporation.  All rights reserved.
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
 * $Workfile: DlnaHttpClient.h
 * $Revision: #1.0.2201.28945
 * $Author:   Intel Corporation, Intel Device Builder
 * $Date:     Tuesday, January 10, 2006
 *
 *
 *
 */

#ifndef DLNAHTTPCLIENT_H
#define DLNAHTTPCLIENT_H

/*! \file DlnaHttpClient.h 
	\brief DLNA HTTP Client API
*/

#include "ILibWebClient.h"
#include "DlnaHttp.h"
#include "ILibThreadPool.h"

#define DHC_READ_BLOCK_SIZE 4096


/*! \defgroup DlnaHttpClient DLNA HTTP Client 
	\brief This module makes it easier to issue HTTP requests 
	and handle the response data.
	
	It is built on top of the \ref DlnaHttp and \a ILibWebClient modules. 

	<b>&lt;FUTURE WORK&gt;</b><BR>
	- Issue an HTTP HEAD request to acquire information about the 4th field protocolInfo in a struct.
	- Issue an HTTP HEAD request that acquires the available data ranges for use with RANGE and TimeSeekRange.dlna.org (under the Limited Random Access Data Availability model).
	- Issue an HTTP HEAD request that acquires the URI for IFO file, if available.

	<BR><b>&lt;/FUTURE WORK&gt;</b>

	\{
*/

/*! \brief Enumeration of errors that can be returned in result callbacks.
*/
enum DHC_Errors
{
	/*!	No error. 
	*/
	DHC_ERRORS_NONE = 0,

	/*!	The \a PeerManager or \a FriendlyName is not specified in the request.
	*/
	DHC_Errors_PeerFriendlyUnspecified, 

	DHC_ERRORS_CONNECTION_FAILED_OR_ABORTED =5,
	/*!	Received an HTTP error code.
	*/
	DHC_ERRORS_HTTP = 10
};


/*! \brief Method signature for handling a completed HTTP request/response transaction.
	\param[in] transfer_status_handle This is the \ref DH_TransferStatus that represents the transfer.
	\param[in] dhc_error_code This is the type of error that was encountered. Usually equal to \ref DHC_Errors.
	\param[in] http_error_code This is the HTTP error code that was returned, if \a dhc_error_code == \ref DHC_Errors.
	\param[in] error_message This contains details of error as a string. Usually the error message returned by the HTTP server.
	\param[in] user_obj The user object that was associated with the request.

	\warning After this callback executes and returns to the DlnaHttpClient module, the \a transfer_status handle will 
	no longer be valid.
*/
typedef void(*DHC_OnResponseDone)(DH_TransferStatus transfer_status_handle, enum DHC_Errors dhc_error_code, int http_error_code, const char* error_message, void *user_obj);

/*! \brief Issue an HTTP <b>GET</b> request with a transfer mode of \ref DH_TransferMode_Bulk
	and save the returned data to a local file path.

	\note Given the asynchronous nature of the call, the application is responsible for the 
	lifetime of the \a request and its associated data.
	The best time to free the \a request and (if necessary, its member fields) is the handler for
	the \a callback_response.

	\param[in] request_manager The \a ILibWebClient_RequestManager instance that was created through \a ILibWebClient_Create.
	\param[in] pool The ILibThreadPool object to use to read from the file.
	\param[in] file_path The file path where the data is saved.
	\param[in] append_flag 
	Zero value means that the data overwrites the existing file indicated by \a file_path.
	-1 value means that the data is appended to the indicated \a file_path
	Positive integer value means that the data is appended to the indicated \a file_path at the specified position
	by doing the following.
	- Before issuing the HTTP request, the method will examine the current
	file length of \a file_path.
	- The method will attempt to use the <b>RANGE</b> header to transfer the file
	from the next byte index.
	- The received data is appended to \a file_path.
	\param[in] target_uri The URI that is to be downloaded. 
	The URI is assumed to have proper HTTP-escaping for DLNA.
	\param[in] requestedTransferMode The transfer mode to request for the transfer. 
	\param[in] user_obj Associate the \a user_obj with the returned \ref DH_TransferStatus.
	\param[in] callback_response Upon completion or error, execute this callback.
	\returns The \ref DH_TransferStatus that allows the application to track the progress of the transfer.
*/
DH_TransferStatus DHC_IssueRequestAndSave(ILibWebClient_RequestManager request_manager, ILibThreadPool pool, const char *file_path, long append_flag, const char *target_uri, enum DH_TransferModes requestedTransferMode, void* user_obj, DHC_OnResponseDone callback_response);

/*!	\brief Issue an HTTP <b>POST</b> request to upload a local file.

	\param[in] request_manager The \a ILibWebClient_RequestManager instance that was created through \a ILibWebClient_Create.
	\param[in] pool The ILibThreadPool object to use to read from the file.
	\param[in] target_uri The URI that is to receive the uploaded file. 
	The URI is assumed to have proper HTTP-escaping for DLNA.
	\param[in] file_path The file path of the local file to upload.
	\param[in] resume_pos
	Negative value means that the HTTP client will upload with intent to overwrite 
	any existing content data that is saved at the specified URI.
	Non-negative value means that the HTTP client will attempt to transfer from
	this resume position (i.e. \a resume_pos should equal the res\@uploadedSize
	of the correpsonding res\@importUri).
	\warning The application is responsible for having properly created the
	&lt;res&gt; element with the res\@resumeUpload="1" and the DMS must also
	indicate support for the <i>resume content transfer</i> operation by
	having responded with metadata that preserved the res\@resumeUpload attribute.
	\param[in] user_obj Associate the \a user_obj with the returned \ref DH_TransferStatus.
	\param[in] callback_response Upon completion or error, execute this callback.
	\returns The \ref DH_TransferStatus that allows the application to track the progress of the transfer.
*/
DH_TransferStatus DHC_IssuePostRequestFromFile(ILibWebClient_RequestManager request_manager, ILibThreadPool pool, const char* target_uri, const char *file_path, int resume_pos, void* user_obj, DHC_OnResponseDone callback_response);

void DHC_Pause(DH_TransferStatus tstatus);
void DHC_Resume(DH_TransferStatus tstatus);

/*! \} */

#endif
