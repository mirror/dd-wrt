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
 * $Workfile: DlnaHttpServer.h
 * $Revision: #1.0.2201.28945
 * $Author:   Intel Corporation, Intel Device Builder
 * $Date:     Tuesday, January 10, 2006
 *
 *
 *
 */

#ifndef DLNAHTTPSERVER_H
#define DLNAHTTPSERVER_H

/*! \file DlnaHttpServer.h 
	\brief DLNA HTTP Server API
*/

#include "ILibWebServer.h"
#include "DlnaHttp.h"
#include "ILibThreadPool.h"
#include "DLNAProtocolInfo.h"

#define DHS_READ_BLOCK_SIZE 8144
#define DHS_DEFAULT_MIMETYPE "application/octet-stream"


enum DHS_Errors
{
	DHS_ERRORS_NONE=0,
	DHS_ERRORS_PEER_ABORTED_CONNECTION=20
};

/*! \defgroup DlnaHttpServer DLNA HTTP Server 
	\brief This module makes it easier to respond to HTTP requests.
	
	It is built on top of the \ref DlnaHttp and \a ILibWebServer modules. 

	<b>&lt;FUTURE WORK&gt;</b><BR>
	- Use a struct to represent 4th field protocolInfo and properly serialize the 4th field parameter as a string.

	<BR><b>&lt;/FUTURE WORK&gt;</b>
	\{
*/

/*! \brief Method signature for handling a completed HTTP request/response transaction.
	\param[in] session The HTTP webserver session, obtained from the \a ILibWebServer_Session_OnSession callback that was specified in the \a ILibWebServer_Create() method.
	\param[in] transfer_status_handle This is the \ref DH_TransferStatus that represents the transfer.
	\param[in] dhs_error_code This is the type of error that was encountered. 
	\param[in] user_obj The User object that was associated with this response.

	\warning After this callback executes and returns to the DlnaHttpServer module, the \a transfer_status handle will 
	no longer be valid.
*/
typedef void(*DHS_OnResponseDone)(struct ILibWebServer_Session *session, DH_TransferStatus transfer_status_handle, enum DHS_Errors dhs_error_code, void *user_obj);

/*!	\brief Allows the application to respond to an HTTP <b>GET</b> or <b>HEAD</b> request by specifying a local file for response.
	
	The application is responsible for doing the following.
	- examining \a header->DirectiveObj to determine the requested URI
	- providing the path for the local file that the URI corresponds to the requested URI
	- providing the mime-type and protocolInfo 4th field information (i.e. contentFeatures.dlna.org value) for the URI
	- providing a URI for an IFO file (if necessary)
	- providing a callback for handling the response

	This method will handle the following HTTP headers in the following ways.
	- <b>RANGE</b> : If the request specified a RANGE header, then the response will include a CONTENT-RANGE header.
		Furthermore, the DlnaHttpServer module will appropriately send the entity-body bytes that correspond to
		the data in the CONTENT-RANGE header.
	- <b>transferMode.dlna.org</b> : If the request omits the transferMode.dlna.org HTTP header, then the response
		will be sent with the \a default_transfer_mode, specified by the application. (The application is responsible
		for specifying the proper transfer mode based on DLNA requirements.) If the request includes the
		transferMode.dlna.org, then the DlnaHttpServer will respond with the specified transfer mode if the
		mode is supported. Otherwise, the DlnaHttpServer will respond with a 406 (Not Acceptable).
	- <b>getcontentFeatures.dlna.org</b> : If the request includes the "getcontentFeatures.dlna.org:1" HTTP header,
		then the response will include the contentFeatures.dlna.org HTTP header with a value of 
		\a protocol_info_fourth_field. If the request includes the getcontentFeatures.dlna.org with a value
		other than <i>1</i>, the response is 400 (Bad Request).
	- <b>pragma: getifoFileURI</b> : If the "pragma" HTTP header includes the "getifoFileURI.dlna.org" token,
		then the response will include the ifoFileURI.dlna.org HTTP header with the \a ifo_uri specified
		as the value.

	\note Generally, the \a min_transfer_mode for all content is \ref DH_TransferMode_Bulk, with live content
	being the notable exception. The \a default_transfer_mode for most content is as follows.
	- \ref DH_TransferMode_Streaming : for immediate rendering of audio or video content
	- \ref DH_TransferMode_Interactive : for immediate rendering of images or playlist files
	- \ref DH_TransferMode_Bulk : for upload or download of all content that is not paced by the server

	\param[in] session The HTTP webserver session, obtained from the \a ILibWebServer_Session_OnSession callback
	that was specified in the \a ILibWebServer_Create method.
	\param[in] pool The ILibThreadPool object to use to read from the file.
	\param[in] header The HTTP headers that were specified in the request, obtained from the \a ILibWebServer_Session_OnSession callback
	that was specified in the \a ILibWebServer_Create method.
	\param[in] buffer_size The buffer size to use when reading from the disk and sending on the wire.
	\param[in] file_name The local file to use for the response.
	\param[in] min_transfer_mode The minimum transfer mode that the request may specify in the transferMode.dlna.org HTTP header.
	\param[in] default_transfer_mode The default transfer mode that the response will specify if the request omitted the transferMode.dlna.org HTTP header.
	\param[in] mime_type The mime-type for file, as specified in the DLNA specification.
	\param[in] protocol_info_fourth_field The 4th field of the protocolInfo for the URI.
	\param[in] ifo_uri The URI to the IFO file. Required only when the \a file_name is for MPEG2-PS content with SCR/PTS discontinuities.
	\param[in] user_obj Application-level user object to associate with the transfer.
	\param[in] callback_response The callback to execute when an error or completion occurs.
	\returns The \ref DH_TransferStatus that allows the application to track the progress of the transfer.
*/
DH_TransferStatus DHS_RespondWithLocalFile(struct ILibWebServer_Session *session, ILibThreadPool pool, struct packetheader *header, size_t buffer_size, const char *file_name, enum DH_TransferModes min_transfer_mode, enum DH_TransferModes default_transfer_mode, const char *mime_type, const char *protocol_info_fourth_field, const char* ifo_uri, void *user_obj, DHS_OnResponseDone callback_response);

/*!	\brief Allows the application to save an HTTP <b>POST</b> request to a local file.

	The application is responsible for doing the following.
	- examining \a header->DirectiveObj to determine the requested URI
	- determine whether the local file can append the received data 
	(i.e. Did the HTTP server detect a failed upload attempt for
	the URI on a previous <b>POST</b> transaction for a &lt;res&gt;
	element that was created with the res\@resumeUpload attribute?)
	- providing the path for the local file that should receive the content data
	- providing the mime-type and protocolInfo 4th field information (i.e. contentFeatures.dlna.org value) for the URI
	- providing a callback for handling the response

	\param[in] session The HTTP webserver session, obtained from the \a ILibWebServer_Session_OnSession callback
	that was specified in the \a ILibWebServer_Create method.
	\param[in] pool The ILibThreadPool object to use to write to the file.
	\param[in] header The HTTP headers that were specified in the request, obtained from the \a ILibWebServer_Session_OnSession callback
	that was specified in the \a ILibWebServer_Create method.
	\param[in] file_name The local file that should receive the uploaded content.
	\param[in] append_flag Zero value indicates \a file_name will be overwritten. 
	Non-zero value indicates that the uploaded data can be appended to the
	existing data of \a file_name. Generally,
	- If \a append_flag !=0 and the \a header 
	indicates that <b>CONTENT-RANGE</b> is being used to upload partial data,
	then the method will append the received data to \a file_name.
	- If \a append_flag ==0 and the request specifies <b>CONTENT-RANGE</b>,
	then the method will respond with a 406 (Not Acceptable) error.
	\param[in] user_obj Application-level user object to associate with the transfer.
	\param[in] callback_response The callback to execute when an error or completion occurs.
	\returns The \ref DH_TransferStatus that allows the application to track the progress of the transfer.
*/
DH_TransferStatus DHS_SavePostToLocalFile(struct ILibWebServer_Session *session, ILibThreadPool pool, struct packetheader *header, const char *file_name, int append_flag, void *user_obj, DHS_OnResponseDone callback_response);
void DH_UpdateProtocolInfoOptions(struct DLNAProtocolInfo *inputInfo);

void DH_Pause(DH_TransferStatus tstatus);
void DH_Resume(DH_TransferStatus tstatus);

/*! \} */

#endif
