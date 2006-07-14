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
 * $Workfile: DlnaHttp.h
 * $Revision: #1.0.2201.28945
 * $Author:   Intel Corporation, Intel Device Builder
 * $Date:     Tuesday, January 10, 2006
 *
 *
 *
 */

#ifndef DLNAHTTP_H
#define DLNAHTTP_H

/*! \file DlnaHttp.h 
	\brief Common DLNA HTTP server/client functions.
*/

/*!	\warning 
	You need to define the following in your .c file
	before you include \i ILibParsers.h.

	#if defined(WINSOCK2)
		#include <winsock2.h>
		#include <ws2tcpip.h>
	#elif defined(WINSOCK1)
		#include <winsock.h>
		#include <wininet.h>
	#endif
*/
#include "ILibParsers.h"

/*! \defgroup DlnaHttp DLNA HTTP 
	\brief This module provides common functions for HTTP servers/clients.

	<b>&lt;Transfer Modes&gt;</b><BR>
	The DLNAv1.0 guidelines did not define transfer modes, 
	but the guidelines assume that all audio and audio/video files 
	are transmitted with Streaming rules and all images are transmitted 
	with Interactive. A key impact of this decision is that HTTP clients 
	that attempted to use TCP flow control ran the risk of causing buffer 
	overflows on the HTTP server. DLNA eventually approved v1.0 errata 
	that defined the <i>http-stalling</i> flag. This flag (indicated in the 4th 
	field of protocolInfo) indicates whether or not HTTP clients can use 
	TCP flow control without any risk.

	The DLNAv1.5 guidelines assume that all audio and audio/video files 
	must support the Streaming transfer mode to ensure backwards compatibility 
	with DLNAv1.0. Likewise, images must also support the Interactive transfer mode.
	
	A major change for DLNAv1.5 is that HTTP clients can issue requests so that the 
	transfer mode indicates a lower priority, where the priority model is 
	(<i>Streaming</i> &gt; <i>Interactive</i> &gt; <i>Background</i>). 
	HTTP clients can know if a particular transfer mode is supported by 
	parsing the <i>DLNA.ORG_FLAGS</i> parameter in the 
	4th field of protocolInfo (often acquired through a <b>HEAD</b> request that uses the 
	<b>getcontentFeatures.dlna.org</b>) and checking if the <i>tm-s</i>, <i>tm-i</i>, 
	and <i>tm-b</i> bits are set. For all transfer modes, the <i>http-stalling</i> flag 
	only applies to <i>Streaming</i> transfers, since the actual throughput for 
	<i>Interactive</i> or <i>Background</i> is never guaranteed.

	For more information about specific transfer modes, see \ref DH_TransferModes.
	<BR><b>&lt;/Transfer Modes&gt;</b>

	<b>&lt;DLNA HTTP Headers&gt;</b><BR>
	All of the method names that begin with <b>DH_AddHeader</b> are used to 
	add HTTP headers to an HTTP request or response.

	As a general rule, HTTP Servers and HTTP Clients conforming to v1.5 can use headers 
	defined to v1.5 without negative impact to endpoints conforming to v1.0. 
	The key thing is that a v1.5 that uses any features (including any HTTP header) 
	defined in v1.5 must also implement all other mandatory requirements for v1.5. 
	This means that you cannot build something that claims v1.0 conformance that uses 
	v1.5 features. 

	The 4th field of protocolInfo is used to indicate which HTTP headers are supported by an 
	HTTP server. Essentially, HTTP clients are supposed to issue HTTP HEAD requests with 
	the <b>getcontentFeatures.dlna.org</b> header and parse the 4th field before attempting 
	to use associated features. Likewise, HTTP servers must respond to those HTTP <b>HEAD</b>
	requests by including the <b>contentFeatures.dlna.org</b> header.
	<BR><b>&lt;/DLNA HTTP Headers&gt;</b>

	<b>&lt;Future Work&gt;</b><BR>
	- Define a mechanism to easily parse the 4th field for supported features.
		(e.g. easily determine if http-stalling, tm-i, tm-s, tm-b are true. When
		this is done, be sure to take into account that bits in the flags-param sometimes
		have inferred values of true|false|unknown when the flags-param is omitted.)

	<BR><b>&lt;/Future Work&gt;</b>
	\{
*/

/*!	\brief DLNA-defined maximum total size for HTTP header buffers.

	DLNA guidelines only require HTTP endpoints to understand the first 4K of HTTP headers.
	HTTP clients and servers can use larger header sizes, but the receiving endpoint is not
	required to interpret any of the HTTP headers that are past the 4K mark.
*/
#define DH_MAX_HTTP_HEADER_SIZE 4096


struct DH_TransferStatus_StateObject
{
	sem_t syncLock;
	long TotalBytesToBeSent;
	long ActualBytesSent;
	long TotalBytesToBeReceived;
	long ActualBytesReceived;
	void *RequestToken;
	void *ServerSession;
	int SessionFlag;
	int Reserved1;
	int Reserved2;
	void *Reserved3;
	int Reserved4;
};

/*! \brief A handle that allows the application to find out about the current
	state of the transfer.
*/
typedef struct DH_TransferStatus_StateObject* DH_TransferStatus;

DH_TransferStatus DH_CreateNewTransferStatus();
void DH_DestroyTransferStatus(DH_TransferStatus status);

/*! \brief Specifies the type (i.e. HTTP method) of the HTTP request.
*/
enum DH_RequestTypes
{
	/*!	For HTTP <b>GET</b> requests. Used to acquire/stream content. 
	*/
	DH_RequestType_GET = 0,

	/*!	For HTTP <b>HEAD</b> requests. Used to probe a URI for supported features.
		For example, \a <b>HEAD</b> requests allows a rendering engine to...
		- determine the 4th field of the res\@protocolInfo for the URI,
		- determine the available data ranges for content governed by DLNA's 
		<i>Limited Random Access Data Availability Model</i>,
		- determine the instance-length for a URI,
		Generally, it's best to issue a <b>HEAD</b> request to acquire information about
		a URI, and then follow up with the appropriate <b>GET</b> requests to transfer data.
	*/
	DH_RequestType_HEAD,

	/*!	For HTTP <b>POST</b> requests. Used to upload files to a DMS over HTTP.
	*/
	DH_RequestType_POST
};


/*! \brief Specifies the DLNA transfer mode.
*/
enum DH_TransferModes
{
	DH_TransferMode_Unspecified = 0x00,

	/*!	<i>Background</i> transfer mode.
		For use with upload and download transfers to and from the server. 
		The primary difference between \ref DH_TransferMode_Interactive and 
		\ref DH_TransferMode_Bulk is that the latter assumes that the user 
		is not relying on the transfer for immediately rendering the content 
		and there are no issues with causing a buffer overflow if the 
		receiver uses TCP flow control to reduce total throughput.
	*/
	DH_TransferMode_Bulk = 0x01,

	/*! <i>Interactive</i> transfer mode.
		For best effort transfer of images and non-real-time transfers. 
		URIs with image content usually support \ref DH_TransferMode_Bulk too. 
		The primary difference between \ref DH_TransferMode_Interactive and 
		\ref DH_TransferMode_Bulk is that the former assumes that the 
		transfer is intended for immediate rendering.
	*/
	DH_TransferMode_Interactive = 0x02,

	/*!	<i>Streaming</i> transfer mode.
		The server transmits at a throughput sufficient for real-time playback of 
		audio or video. URIs with audio or video often support the 
		\ref DH_TransferMode_Interactive and \ref DH_TransferMode_Bulk transfer modes. 
		The most well-known exception to this general claim is for live streams.
	*/
	DH_TransferMode_Streaming = 0x03
};
enum DH_TransferModes DH_GetRequestedTransferMode(struct packetheader *p);

/*!	\brief Adds an HTTP header to existing set of HTTP headers. Generally used to add vendor-extensible headers.

	\param[in] http_headers The set of HTTP headers, acquired from \a ILibCreateEmptyPacket().
	\param[in] header_name The name of the HTTP header to add.
	\param[in] header_name_len The length of the HTTP header name.
	If negative, then the method will assume that \a header_name is a null-terminated string.
	\param[in] header_value The value of the HTTP header for \a header_name.
	\param[in] header_value_len The length of the HTTP header value. 
	If negative, then the method will assume that \a header_value is a null-terminated string.
	\returns The following values.
		- Zero if the header was applied. 
		- Positive: The header was applied but required a resize. The returned value is the new allocated size.
		- Negative: The header was not applied because it would cause an overrun 
		beyond \ref DH_MAX_HTTP_HEADER_SIZE.
*/
int DH_AddHeader(struct packetheader* http_headers, const char* header_name, int header_name_len, const char* header_value, int header_value_len);

/*!	\brief Adds the <b>transferMode.dlna.org</b> HTTP header to a set of HTTP headers. 
	
	- HTTP clients and servers generally use this HTTP header to indicate the requested/used DLNA
		transfer mode for content requests/responses. See \ref DH_TransferModes for more information.
	- Can be used in HTTP <b>HEAD</b> requests to probe an HTTP server’s support for a transfer mode, but this is not recommended. 
	- <b>4th field note</b>: See <i>tm-s</i>, <i>tm-i</i>, and <i>tm-b</i> bits of the 
		<i>primary-flags</i> token in the <i>DLNA.ORG_FLAGS</i> parameter.
	- <b>DLNA version note</b>: Valid only for DLNA v1.5. Transfer modes are inferred for DLNAv1.0 based on the media class.
		- images infers <i>Interactive</i>
		- audio infers <i>Streaming</i>
		- audio/video infers <i>Streaming</i>

	\param[in] http_headers The set of HTTP headers, acquired from \a ILibCreateEmptyPacket().
	\param[in] transfer_mode The transfer mode to use for the HTTP request or response.
	\returns The following values.
		- Zero if the header was applied. 
		- Positive: The header was applied but required a resize. The returned value is the new allocated size.
		- Negative: The header was not applied because it would cause an overrun 
		beyond \ref DH_MAX_HTTP_HEADER_SIZE.
*/
int DH_AddHeader_transferMode(struct packetheader* http_headers, enum DH_TransferModes transfer_mode);

/*!	\brief Adds the <b>RANGE</b> HTTP header to a set of HTTP headers.
	
	- Used for HTTP <b>GET</b> requests to indicate the requested byte-range 
		that should be sent by the HTTP server in the response.
	- Can be used in HTTP <b>HEAD</b> requests to probe an HTTP server’s support for the <b>RANGE</b> feature, 
		but this is not recommended.
	- <b>4th field note:</b> See the <i>b-val</i> token of the of the <i>DLNA.ORG_OP</i> parameter and the <i>lop-bytes</i> bit 
		of the <i>primary-flags</i> token in the <i>DLNA.ORG_FLAGS</i> parameter.
	- <b>DLNA version note</b>: Valid for DLNA v1.0 and v1.5. 

	An HTTP client application is expected to probe a URI with an HTTP <b>HEAD</b> request that
	specifies the <b>getcontentFeatures.dlna.org</b> HTTP header. The response's
	<b>contentFeatures.dlna.org</b> HTTP header will indicate if the 
	<b>RANGE</b> header is supported through the DLNA.ORG_OP parameter,
	which indicates support through the \a b-val token.

	An HTTP server application is not supposed to use this HTTP header in a response.
	However, HTTP servers that support the <b>RANGE</b> header in conjunction with
	either <b>TimeSeekRange.dlna.org</b> or <b>PlaySpeed.dlna.org</b> must treat
	the <b>RANGE</b> header with the highest precedence. Specifically, the 
	presence of the <b>RANGE</b> header automatically causes the HTTP server to
	ignore the <b>TimeSeekRange.dlna.org</b> and <b>PlaySpeed.dlna.org</b> headers
	when used with <b>RANGE</b>.

	\param[in] http_headers The set of HTTP headers, acquired from \a ILibCreateEmptyPacket().
	\param[in] first_byte_pos The value for the <i>first-byte-pos</i> token. 
	If the value is negative, then the token is omitted from the header value.
	\param[in] last_byte_pos The value for the <i>last-byte-pos</i> token. 
	If the value is negative, then the token is omitted from the header value.
	\returns The following values.
		- Zero if the header was applied. 
		- Positive: The header was applied but required a resize. The returned value is the new allocated size.
		- Negative: The header was not applied because it would cause an overrun 
		beyond \ref DH_MAX_HTTP_HEADER_SIZE.
*/
int DH_AddHeader_Range(struct packetheader* http_headers, int first_byte_pos, int last_byte_pos);

/*!	\brief Adds the <b>Content-Range</b> HTTP header to a set of HTTP headers.
	
	- Used for HTTP <b>GET</b> responses to indicate the byte range that is being sent to the HTTP client. 
		The header is used in response to an HTTP request that used the <b>RANGE</b> header.
	- Also used in HTTP <b>HEAD</b> responses. This behavior is required when an HTTP client issues a <b>HEAD</b> request with 
		the HTTP header and the HTTP server supports the header.
	- <b>4th field note:</b> See the <i>b-val</i> token of the of the <i>DLNA.ORG_OP</i> parameter and 
		<i>lop-bytes</i> bit of the <i>primary-flags</i> token in the <i>DLNA.ORG_FLAGS</i> parameter.
	- <b>DLNA version note</b>: Valid for DLNA v1.0 and v1.5. 

	An HTTP server application that uses this HTTP header in a response indicates
	that the returned content data maps to the zero-based byte index from
	\a first_byte_pos to \a last_byte_pos.
	
	\note: <b>POST transactions</b>
	An HTTP client application is expected to use this only when issuing
	an HTTP <b>POST</b> transaction for the purpose of uploading content.
	Before attempting to use the <b>Content-Range</b> header, the HTTP client
	application (associated with a DLNA Upload Controller) must know that the
	content supports the <i>resume content transfer</i> operation. (e.g.
	The CDS:CreateObject request honored the dlna:res\@resumeUpload=1 portion
	of the SOAP action.) Using this header for <b>POST</b> requests is
	valid only for DLNAv1.5.

	\param[in] http_headers The set of HTTP headers, acquired from \a ILibCreateEmptyPacket().
	\param[in] first_byte_pos The value for the <i>first-byte-pos</i> token. 
	\param[in] last_byte_pos The value for the <i>last-byte-pos</i> token. 
	For HTTP <b>POST</b> transactions, the value must be \a instance_length-1.
	\param[in] instance_length <b>Must be non-negative for HTTP client's POST request and must not be INT_MAX.</b>
	Indicates the value of the <i>instance-length</i> token (e.g. length of the file).
	If the value is negative, then the token is omitted from the header value.
	If <b>INT_MAX</b> is specified (as defined in limits.h), then the asterik value 
	(<b>*</b>) will be used as the instance-length value. 
	\returns The following values.
		- Zero if the header was applied. 
		- Positive: The header was applied but required a resize. The returned value is the new allocated size.
		- Negative: The header was not applied because it would cause an overrun 
		beyond \ref DH_MAX_HTTP_HEADER_SIZE.
*/
int DH_AddHeader_ContentRange(struct packetheader* http_headers, size_t first_byte_pos, size_t last_byte_pos, long instance_length);

/*!	\brief Adds the <b>TimeSeekRange.dlna.org</b> HTTP header to a set of HTTP headers.
	
	- Used for HTTP <b>GET</b> request to indicate the time range that the HTTP 
		server should send in the response.
	- Used for HTTP <b>GET</b> responses to indicate the time range that the HTTP server 
		is sending in the response.
	- Can be used in HTTP <b>HEAD</b> requests to probe an HTTP server’s support for 
		<b>TimeSeekRange.dlna.org</b>, but this is not recommended.
	- <b>4th field note:</b> See the <i>a-val</i> token of the of the
		<i>DLNA.ORG_OP</i> parameter and <i>lop-npt</i> bit of the 
		<i>primary-flags</i> token in the <i>DLNA.ORG_FLAGS</i> parameter.
	- <b>DLNA version note</b>: Valid for DLNA v1.0 and v1.5. 

	An HTTP client application is expected to probe a URI with an HTTP <b>HEAD</b> request that
	specifies the <b>getcontentFeatures.dlna.org</b> HTTP header. The response's
	<b>contentFeatures.dlna.org</b> HTTP header will indicate if the 
	<b>TimeSeekRange.dlna.org</b> header is supported through the DLNA.ORG_OP parameter,
	which indicates support through the \a a-val token.

	An HTTP server application that supports the <b>TimeSeekRange.dlna.org</b> header
	is expected to indicate the npt-range that is actually returned for an HTTP request that
	specified a particular npt-range in the <b>TimeSeekRange.dlna.org</b> header.
	In addition to specifying the npt-range, the HTTP server application is also expected
	to provide the bytes-range token, which is comprised of a <i>first-byte-pos</i>, last-byte-post,
	and instance-length tokens.

	If an HTTP server application supports both <b>TimeSeekRange.dlna.org</b> and
	<b>PlaySpeed.dlna.org</b> and it receives an HTTP request that specifies both
	headers, then the HTTP server must interpret the request as specifying
	a server-side play-speed transaction from \a first_npt_time to \a last_npt_time.

	\param[in] http_headers The set of HTTP headers, acquired from \a ILibCreateEmptyPacket().
	\param[in] first_npt_sec The value for the first <i>npt-time</i> token in the npt-sec syntax.
	(i.e. 1*DIGIT [ "." 1*3DIGIT ]). The method will automatically truncate decimal digits
	beyond the third decimal place.
	If the value is negative, then the <i>npt-time</i> token is omitted from the header value.
	\param[in] last_npt_sec The value for the last <i>npt-time</i> token in the npt-sec syntax.
	(i.e. 1*DIGIT [ "." 1*3DIGIT ]). The method will automatically truncate decimal digits
	beyond the third decimal place.
	If the value is negative, then the <i>npt-time</i> token is omitted from the header value.
	\param[in] first_byte_pos <b>Non-negative value is mandatory for HTTP servers. 
	Non-negative values are prohibited for HTTP clients.</b> 
	Specifies the <i>first-byte-pos</i> token, which represents the zero-based index of the 
	first byte of content data that is returned.
	If the \a first_byte_pos is negative then \a last_byte_pos and \a instance-length must also be negative.
	This will cause the bytes-range token to be ommitted from the header value.
	\param[in] last_byte_pos <b>Non-negative value is mandatory for HTTP servers. 
	Non-negative values are prohibited for HTTP clients.</b> 
	Specifies the <i>last-byte-pos</i> token, which represents the zero-based index of the 
	last byte of content data that is returned.
	\param[in] instance_length <b>Non-negative value is mandatory for HTTP servers. 
	Non-negative values are prohibited for HTTP clients.</b> 
	Specifies the total length of a content binary. If <b>INT_MAX</b> is specified
	(as defined in limits.h), then the asterik value (<b>*</b>) will be used
	as the instance-length value.
	\returns The following values.
		- Zero if the header was applied. 
		- Positive: The header was applied but required a resize. The returned value is the new allocated size.
		- Negative: The header was not applied because it would cause an overrun 
		beyond \ref DH_MAX_HTTP_HEADER_SIZE.
*/
int DH_AddHeader_TimeSeekRange(struct packetheader* http_headers, float first_npt_sec, float last_npt_sec, int first_byte_pos, int last_byte_pos, int instance_length);

/*!	\brief Adds the <b>getAvailableSeekRange.dlna.org</b> HTTP header to a set of HTTP headers.
	
	- Used for HTTP <b>HEAD</b> requests to request the server's supported time and byte ranges 
		for HTTP <b>GET</b> requests that use <b>TimeSeekRange.dlna.org</b> and <b>RANGE</b>.
	- <b>4th field note:</b> See the <i>lop-npt</i> and <i>lop-bytes</i> bits of the 
		<i>primary-flags</i> token in the <i>DLNA.ORG_FLAGS</i> parameter.
	- <b>DLNA version note:</b> Valid only for DLNA v1.5.

	An HTTP client application must use this HTTP header (usually with
	a <b>HEAD</b> request) before attempting to use the <b>RANGE</b>
	or <b>TimeSeekRange.dlna.org</b> HTTP headers with a URI that indicates
	the <i>Limited Random Access Data Availability</i> model. This comes into play
	primarily when attempting to use those headers with live content
	and certain cases of converted/transcoded content.

	HTTP server applications do not use this HTTP header in responses.
	However, HTTP server applications that indicate the 
	<i>Limited Random Access Data Availability</i> model for a URI
	(via the 4th field of protocolInfo) must respond with the
	<b>availableSeekRange.dlna.org</b> HTTP header when a request
	uses the <b>getAvailableSeekRange.dlna.org</b>.

	\param[in] http_headers The set of HTTP headers, acquired from \a ILibCreateEmptyPacket().
	\returns The following values.
		- Zero if the header was applied. 
		- Positive: The header was applied but required a resize. The returned value is the new allocated size.
		- Negative: The header was not applied because it would cause an overrun 
		beyond \ref DH_MAX_HTTP_HEADER_SIZE.
*/
int DH_AddHeader_getAvailableSeekRange(struct packetheader* http_headers);

/*!	\brief Adds the <b>availableSeekRange.dlna.org</b> HTTP header to a set of HTTP headers.
	
	- Generally used for HTTP <b>HEAD</b> responses to indicate the supported time and byte ranges 
		that can be used with HTTP <b>GET</b> requests that use <b>TimeSeekRange.dlna.org</b> and <b>RANGE</b>.
	- Also used in HTTP <b>GET</b> responses. HTTP servers are required to respond to
		<b>GET</b> requests if they are able to respond similarly to a <b>HEAD</b> request.
	- <b>4th field note:</b> See the <i>lop-npt</i> and <i>lop-bytes</i> bits of the 
		<i>primary-flags</i> token in the <i>DLNA.ORG_FLAGS</i> parameter.
	- <b>DLNA version note:</b> Valid only for DLNA v1.5.

	HTTP server applications use this HTTP header in responses
	to indicate the available data ranges for requests that use
	<b>RANGE</b> and/or <b>TimeSeekRange.dlna.org</b>.

	In addition to reporting the available data ranges, 
	the <b>availableSeekRange.dlna.org</b> also specifies a set of 
	assumptions for two types of operating modes (\a mode_flag).

	If \a mode_flag ==0, then the following assumptions must apply.
	- The range-specifier (i.e. npt-range and/or bytes-range) maps to [s0, sN].
	(i.e. [r0, rN] must equal [s0, sN])
	- Clients are expected to specify a valid value for first-byte-pos and/or 
	the starting npt-time. (i.e. Specifying a request that explicitly indicates 
	"0" for npt or a byte position is not guaranteed to work.)
	- Servers that receive a <b>GET</b> request that omits both <b>RANGE</b> 
	and <b>TimeSeekRange.dlna.org</b> will respond from a "live position".
	This live position has the following attributes:
		- equal to sN
		- shifts in real-time with sN

	If \a mode_flag ==1, then the following assumptions must apply.
	- If present, the npt-range must map to [r0, rN], 
	which must be equal to [s0, sN].
	- If present, the bytes-range maps to a subset of [r0, rN] 
	([r0, rN] must be equal to [s0, sN]).
	- The content data mapped by range-specifier is (usually) 
	available immediately for transmission.
	- The content data for [s0, sN] is available for transmission.
	- The s0 data position represents a fixed (non-changing), 
	absolute beginning for the content.
	- Servers that receive a GET request that omits both 
	<b>RANGE</b> and <b>TimeSeekRange.dlna.org</b> will respond 
	from the absolute beginning (i.e. s0) of the content.

	HTTP clients do not use this HTTP header.

	\param[in] http_headers The set of HTTP headers, acquired from \a ILibCreateEmptyPacket().
	\param[in] mode_flag The mode-flag, which must be <i>0</i> or <i>1</i>,
	to indicate one of the operating modes described above.
	\param[in] first_byte_pos The value for the <i>first-byte-pos</i> token. 
	If the value is negative, then the token is omitted from the header value.
	\param[in] last_byte_pos The value for the <i>last-byte-pos</i> token. 
	If the value is negative, then the token is omitted from the header value.
	\param[in] first_npt_sec The value for the first <i>npt-time</i> token in the npt-sec syntax.
	(i.e. 1*DIGIT [ "." 1*3DIGIT ]). The method will automatically truncate decimal digits
	beyond the third decimal place.
	If the value is negative, then the <i>npt-time</i> token is omitted from the header value.
	\param[in] last_npt_sec The value for the last <i>npt-time</i> token in the npt-sec syntax.
	(i.e. 1*DIGIT [ "." 1*3DIGIT ]). The method will automatically truncate decimal digits
	beyond the third decimal place.
	If the value is negative, then the <i>npt-time</i> token is omitted from the header value.
	\returns The following values.
		- Zero if the header was applied. 
		- Positive: The header was applied but required a resize. The returned value is the new allocated size.
		- Negative: The header was not applied because it would cause an overrun 
		beyond \ref DH_MAX_HTTP_HEADER_SIZE.
*/
int DH_AddHeader_availableSeekRange(struct packetheader* http_headers, int mode_flag, int first_byte_pos, int last_byte_pos, float first_npt_sec, float last_npt_sec);

/*!	\brief Adds the <b>PlaySpeed.dlna.org</b> HTTP header to a set of HTTP headers.
	
	- Generally used in HTTP <b>GET</b> requests to indicate the request the HTTP server 
		to time-scale the content (i.e. server does trick-mode computations so 
		that the client doesn’t have to do it)
	- Generally used in HTTP <b>GET</b> responses to indicate that the response 
		data has been time-scaled.
	- Can be used in HTTP <b>HEAD</b> requests, but not recommended. HTTP servers are required
		to respond to <b>HEAD</b> requests if they are able to respond to similarly to <b>GET</b> requests.
	- <b>4th field note:</b> See the <i>DLNA.ORG_PS</i> parameter.
	- <b>DLNA version note:</b> Valid for DLNA v1.0 and v1.5.

	An HTTP client application is expected to probe a URI with an HTTP <b>HEAD</b> request that
	specifies the <b>getcontentFeatures.dlna.org</b> HTTP header. The response's
	<b>contentFeatures.dlna.org</b> HTTP header will indicate if the 
	<b>PlaySpeed.dlna.org</b> header is supported through the DLNA.ORG_PS parameter,
	which lists a comma-delimited list of server-side transport play-speeds.

	An HTTP server application that supports the <b>PlaySpeed.dlna.org</b> header
	is expected to use this header when it honors an HTTP client's request
	for server-side play speed content at the indicated <i>TransportPlaySpeed</i>.

	If an HTTP server application supports both <b>TimeSeekRange.dlna.org</b> and
	<b>PlaySpeed.dlna.org</b> and it receives an HTTP request that specifies both
	headers, then the HTTP server must interpret the request as specifying
	a server-side play-speed transaction from \a first_npt_time to \a last_npt_time.

	\param[in] http_headers The set of HTTP headers, acquired from \a ILibCreateEmptyPacket().
	\param[in] transport_play_speed The value of the requested server-side play speed.
	\param[in] transport_play_speed_len The byte length of \a transport_play_speed. 
	If \a transport_play_speed_len is negative, then the method assumes \a transport_play_speed
	is null-terminated.
	\returns The following values.
		- Zero if the header was applied. 
		- Positive: The header was applied but required a resize. The returned value is the new allocated size.
		- Negative: The header was not applied because it would cause an overrun 
		beyond \ref DH_MAX_HTTP_HEADER_SIZE.
*/
int DH_AddHeader_PlaySpeed(struct packetheader* http_headers, const char* transport_play_speed, int transport_play_speed_len);

/*!	\brief Adds the <b>scid.dlna.org</b> HTTP header to a set of HTTP headers.
	
	- Used in HTTP <b>GET</b> responses to indicate the associated <i>UPnP AV Connection ID</i> 
		assigned by the UPnP AV MediaServer. This is used when the MediaServer implements DLNA's 
		<i>basic connection management</i> feature.
	- Also used in HTTP <b>GET</b> requests to indicate the request is associated with the same user-session 
		(i.e. <i>UPnP AV Connection</i>) of a previous request.
	- Also used in HTTP <b>HEAD</b> requests to extend the validity of the UPnP AV Connection ID when idling a UPnP AV connection.
	- <b>4th field note:</b> Not related to the 4th field. 
		See the DLNA guidelines related to <i>basic connection management</i> (i.e. search for “BCM”).
	- <b>DLNA version note:</b> Valid only for DLNA v1.5.

	An HTTP client application is expected to provide the server's connection-id value
	by using the value provided from the <b>scid.dlna.org</b> header from a 
	previous HTTP response for the same target URI and user playback session.

	An HTTP server application uses this header when it serving for a DMS
	or M-DMS with the <i>basic connection management</i> feature. A DMS or M-DMS
	that supports <i>basic connection management</i> is expected to do
	the following.
	- A request without the <b>scid.dlna.org</b> header causes the DMS/M-DMS
	to create a new <i>UPnP AV Connection ID</i>. The value of this connection-id
	is specified in the HTTP response as the value of the <b>scid.dlna.org</b> header.
	- A request with the <b>scid.dlna.org</b> header causes the DMS/M-DMS
	to interpret the request as being a continuation of HTTP transport layer
	activity for the indicated connection-id.

	\note Whenever possible, HTTP clients should specify this field when a
	subsequent HTTP request is for the same URI as a previous request and the 
	subsequent HTTP request serves to provide a continuation of a user experience
	provided by the earlier HTTP request. For example, if an HTTP client issues
	multiple HTTP <b>GET</b> requests using the <b>RANGE</b> HTTP header in a series to produce
	a trick-mode playback effect, then the HTTP client should
	- acquire the value of the <b>scid.dlna.org</b> HTTP header from the first HTTP response 
	- and specify the same value for the <b>scid.dlna.org</b> header in the subsequent HTTP requests.

	\param[in] http_headers The set of HTTP headers, acquired from \a ILibCreateEmptyPacket().
	\param[in] connection_id The value of the connection-id, acquired from a previous HTTP response.
	\returns The following values.
		- Zero if the header was applied. 
		- Positive: The header was applied but required a resize. The returned value is the new allocated size.
		- Negative: The header was not applied because it would cause an overrun 
		beyond \ref DH_MAX_HTTP_HEADER_SIZE.
*/
int DH_AddHeader_scid(struct packetheader* http_headers, unsigned int connection_id);

/*!	\brief Adds the <b>FriendlyName.dlna.org</b> HTTP header to a set of HTTP headers.

	- Used in HTTP <b>GET</b>/<b>HEAD</b> requests to indicate the friendly name of the device. 
		This is mutually exclusive with <b>peerManager.dlna.org</b>.
	- <b>4th field note:</b> Not related to the 4th field. See the DLNA guidelines related to 
		<i>basic connection management</i> (i.e. search for “BCM”).
	- <b>DLNA version note:</b> Valid only for DLNA v1.5.

	An HTTP client application that issues HTTP requests on behalf of
	a non-DMR device (includes DMP, M-DMP, Download Controller, M-DMD, DMPr)
	should set <b>FriendlyName.dlna.org</b> to indicate the human-readable/user-friendly
	name of the device that is issuing the HTTP request.

	HTTP server applications do not respond with this HTTP header. However,
	a DMS or M-DMS that supports <i>basic connection management</i> must use
	this information in the <i>PeerConnectionManager</i> output argument of
	a CMS:GetConnectionInfo request.

	\note HTTP clients issuing requests on behalf of DMR devices should
	use the <b>PeerManager.dlna.org</b> HTTP header.

	\param[in] http_headers The set of HTTP headers, acquired from \a ILibCreateEmptyPacket().
	\param[in] friendly_name The friendly name of the device, not to exceed 64-bytes.
	\param[in] friendly_name_len The byte length of \a friendly_name. 
	If \a friendly_name_len is negative, then the method assumes \a friendly_name
	is null-terminated.
	\returns The following values.
		- Zero if the header was applied. 
		- Positive: The header was applied but required a resize. The returned value is the new allocated size.
		- Negative: The header was not applied because it would cause an overrun 
		beyond \ref DH_MAX_HTTP_HEADER_SIZE.
*/
int DH_AddHeader_FriendlyName(struct packetheader* http_headers, const char* friendly_name, int friendly_name_len);

/*!	\brief Adds the <b>PeerManager.dlna.org</b> HTTP header to a set of HTTP headers.

	- Used in HTTP <b>GET</b>/<b>HEAD</b> requests to indicate the UPnP AV MediaRenderer’s UDN and ServiceID. 
		This is mutually exclusive with <b>friendlyName.dlna.org</b>.
	- <b>4th field note:</b> Not related to the 4th field. See the DLNA guidelines related to 
		<i>basic connection management</i> (i.e. search for “BCM”).
	- <b>DLNA version note:</b> Valid only for DLNA v1.5.

	An HTTP client application that issues HTTP requests on behalf of
	a DMR device should set <b>PeerManager.dlna.org</b> to indicate the UDN/Service-ID
	of the UPnP device and ConnectionManager service that is issuing the HTTP request.

	HTTP server applications do not respond with this HTTP header. However,
	a DMS or M-DMS that supports <i>basic connection management</i> must use
	this information in the <i>PeerConnectionManager</i> output argument of
	a CMS:GetConnectionInfo request.

	\note HTTP clients issuing requests on behalf of non-DMR devices should
	use the <b>FriendlyName.dlna.org</b> HTTP header.

	\param[in] http_headers The set of HTTP headers, acquired from \a ILibCreateEmptyPacket().
	\param[in] friendly_name The friendly name of the endpoint.
	\param[in] friendly_name_len The byte length of \a friendly_name. 
	If the value is negative, then the method assumes \a friendly_name is null-terminated.
	\returns The following values.
		- Zero if the header was applied. 
		- Positive: The header was applied but required a resize. The returned value is the new allocated size.
		- Negative: The header was not applied because it would cause an overrun 
		beyond \ref DH_MAX_HTTP_HEADER_SIZE.
*/
int DH_AddHeader_PeerManager(struct packetheader* http_headers, const char* friendly_name, int friendly_name_len);

/*!	\brief Adds the <b>getcontentFeatures.dlna.org</b> HTTP header to a set of HTTP headers.
	
	- Used in HTTP <b>GET</b>/<b>HEAD</b> requests to acquire the 4th field protocolInfo value for the URI.
	- <b>4th field note:</b> Always supported by HTTP servers for content files. No dependency on 4th field of protocolInfo.
	- <b>DLNA version note:</b> Valid for DLNA v1.0 and v1.5.

	HTTP client applications use this HTTP header (often with HTTP <b>HEAD</b>
	requests) to probe a URI for supported transport layer features.

	HTTP server applications do not use this HTTP header, but they are expected
	to respond with the <b>contentFeatures.dlna.org</b> HTTP header when
	a request specifies the <b>getcontentFeatures.dlna.org</b> header.

	\param[in] http_headers The set of HTTP headers, acquired from \a ILibCreateEmptyPacket().
	\returns The following values.
		- Zero if the header was applied. 
		- Positive: The header was applied but required a resize. The returned value is the new allocated size.
		- Negative: The header was not applied because it would cause an overrun 
		beyond \ref DH_MAX_HTTP_HEADER_SIZE.
*/
int DH_AddHeader_getcontentFeatures(struct packetheader* http_headers);

/*!	\brief Adds the <b>contentFeatures.dlna.org</b> HTTP header to a set of HTTP headers.

	- Used in HTTP <b>GET</b>/<b>HEAD</b> responses to report the 4th field protocolInfo value for the URI.
	- <b>4th field note:</b> Always supported by HTTP servers for content files. No dependency on 4th field of protocolInfo.
	- <b>DLNA version note:</b> Valid for DLNA v1.0 and v1.5.

	HTTP server applications use this HTTP header when responding to a request
	that specifies the <b>getcontentFeatures.dlna.org</b> header. The value of the
	<b>contentFeatures.dlna.org</b> HTTP header is the 4th field of the protocolInfo
	for the URI.

	HTTP client applications do not use this HTTP header.

	\param[in] http_headers The set of HTTP headers, acquired from \a ILibCreateEmptyPacket().
	\param[in] fourth_field The 4th field of the protocolInfo for the URI.
	\param[in] fourth_field_len The byte length of \a fourth_field. 
	If the value is negative, then the method assumes \a fourth_field is null-terminated.
	\returns The following values.
		- Zero if the header was applied. 
		- Positive: The header was applied but required a resize. The returned value is the new allocated size.
		- Negative: The header was not applied because it would cause an overrun 
		beyond \ref DH_MAX_HTTP_HEADER_SIZE.
*/
int DH_AddHeader_contentFeatures(struct packetheader* http_headers, const char* fourth_field, int fourth_field_len);

/*!	\brief Adds the <b>PRAGMA</b> HTTP header to a set of HTTP headers.
	
	- Used for <B>GET</b>/<b>HEAD</b> requests and responses to request and report the IFO file (if present) for the associated content.
	- HTTP client and server applications also use this HTTP header for a variety of custom extensions.
	- <b>4th field note:</b> No dependency on 4th field of protocolInfo. HTTP servers must always handle this if the content has an IFO file.
	- <b>DLNA version note:</b> Valid for DLNA v1.0 and v1.5.

	For some DLNA HTTP clients (specifically rendering endpoints that cannot
	tolerate MPEG-2 SCR/PTS discontinuities), this HTTP header is used to signal
	the HTTP server to respond with a URI (in the <b>PRAGMA</b> header)
	where an IFO file can be acquired for the MPEG-2 video file (indicated
	by the target URI of the HTTP request).

	\param[in] http_headers The set of HTTP headers, acquired from \a ILibCreateEmptyPacket().
	\param[in] get_ifo_flag <b>Must be zero for HTTP servers.</b> 
	A nonzero value instructs the HTTP header to include the 
	<i>getIfoFileURI.dlna.org</i> string in the <b>PRAGMA</b> header.
	\param[in] ifo_file_uri <b>Must be NULL for HTTP clients.</b> 
	A non-NULL value instructs the HTTP header to include the 
	<i>ifoFileURI.dlna.org=</i> \a ifo_file_uri string in the <b>PRAGMA</b> header.
	\param[in] ifo_file_uri_len The byte length of \a ifo_file_uri. 
	If the value is negative, then the method assumes \a ifo_file_uri is null-terminated.
	\param[in] custom_pragma 
	A non-NULL value instructs the HTTP header to append \a custom_pragma to
	the exisitng <b>PRAGMA</b> header value. 
	<b>Applications must properly delimit the pragma-directive tokens.</b>
	\param[in] custom_pragma_len The byte length of \a custom_pragma. 
	If the value is negative, then the method assumes \a custom_pragma is null-terminated.
	\returns The following values.
		- Zero if the header was applied. 
		- Positive: The header was applied but required a resize. The returned value is the new allocated size.
		- Negative: The header was not applied because it would cause an overrun 
		beyond \ref DH_MAX_HTTP_HEADER_SIZE.
*/
int DH_AddHeader_Pragma(struct packetheader* http_headers, int get_ifo_flag, const char* ifo_file_uri, int ifo_file_uri_len, const char* custom_pragma, int custom_pragma_len);

/*!	\brief Allows applications to set a miscellaneous value of a transfer status instance.

	Generally, applications use this method to associate application-level state with
	a \ref DH_TransferStatus instance.

	\param[in] transfer_status_handle The \ref DH_TransferStatus that should receive \a user_object.
	\param[in] user_object Assign this value with \a transfer_status_handle.
	\returns \a transfer_status_handle
*/
DH_TransferStatus DH_SetUserObject(DH_TransferStatus transfer_status_handle, void* user_object);

/*!	\brief Allows applications to get the miscellaneous value of a transfer status instance.

	Applications use this method to retrieve the application-level state that was associated with
	the \ref DH_TransferStatus instance in a previous call to \ref DH_SetUserObject().

	\param[in] transfer_status_handle The \ref DH_TransferStatus instance.
	\returns The value that was last specified in a previous call to \ref DH_SetUserObject().
*/
void* DH_GetUserObject(DH_TransferStatus transfer_status_handle);

/*!	\brief Allows applications to query the progress of the HTTP transaction.

	\note
	- For HTTP clients, \a send_total and \a send_expected apply only in the case of <b>POST</b> transactions.
	- For HTTP clients, \a receive_total and \a receive_expected apply in <b>GET</b> transactions.
	- For HTTP servers, \a send_total and \a send_expected apply only to <b>GET</b> transactions.
	- For HTTP servers, \a receive_total and \a receive_expected apply in <b>POST</b> transactions.
	
	\param[in] transfer_status_handle The \ref DH_TransferStatus that represents the state of the transfer.
	\param[in,out] send_total The number of entity-body bytes that have been sent.
	\param[in,out] send_expected The total number of entity-body bytes that expect to be sent.
	\param[in,out] receive_total The number of entity-body bytes that have been received.
	\param[in,out] receive_expected The total number of entity-body bytes that expect to be received.
*/
void DH_QueryTransferStatus(DH_TransferStatus transfer_status_handle, long* send_total, long* send_expected, long* receive_total, long* receive_expected);
void DH_AbortTransfer(DH_TransferStatus transfer_status_handle);

/*! \} */

#endif
