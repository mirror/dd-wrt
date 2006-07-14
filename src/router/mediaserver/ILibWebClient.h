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
 * $Workfile: ILibWebClient.h
 * $Revision: #1.0.2384.30305
 * $Author:   Intel Corporation, Intel Device Builder
 * $Date:     Thursday, July 13, 2006
 *
 *
 *
 */

#ifndef __ILibWebClient__
#define __ILibWebClient__

#include "ILibParsers.h"
#include "ILibAsyncSocket.h"

/*! \file ILibWebClient.h 
	\brief MicroStack APIs for HTTP Client functionality
*/

/*! \defgroup ILibWebClient ILibWebClient Module
	\{
*/

/*! \def WEBCLIENT_DESTROYED
	\brief The ILibWebClient object was destroyed
*/
#define WEBCLIENT_DESTROYED 5
/*! \def WEBCLIENT_DELETED
	\brief The ILibWebClient object was deleted with a call to ILibWebClient_DeleteRequests
*/
#define WEBCLIENT_DELETED 6

#if defined(WIN32) || defined(_WIN32_WCE)
#include <STDDEF.h>
#else
#include <malloc.h>
#endif

enum ILibWebClient_Range_Result
{
	ILibWebClient_Range_Result_OK = 0,
	ILibWebClient_Range_Result_INVALID_RANGE = 1,
	ILibWebClient_Range_Result_BAD_REQUEST = 2,
};
/*! \typedef ILibWebClient_RequestToken
	\brief The handle for a request, obtained from a call to \a ILibWebClient_PipelineRequest
*/
typedef void* ILibWebClient_RequestToken;

/*! \typedef ILibWebClient_RequestManager
	\brief An object that manages HTTP Client requests. Obtained from \a ILibCreateWebClient
*/
typedef void* ILibWebClient_RequestManager;

/*! \typedef ILibWebClient_StateObject
	\brief Handle for an HTTP Client Connection.
*/
typedef void* ILibWebClient_StateObject;

/*! \typedef ILibWebClient_OnResponse
	\brief Function Callback Pointer, dispatched to process received data
	\param WebStateObject The ILibWebClient that received a response
	\param InterruptFlag A flag indicating if this request was interrupted with a call to ILibStopChain
	\param header The packetheader object containing the HTTP headers.
	\param bodyBuffer The buffer containing the body
	\param[in,out] beginPointer The start index of the buffer referenced by \a bodyBuffer
	\param endPointer The end index of the buffer referenced by \a bodyBuffer
	\param done Flag indicating if all of the response has been received. (0 = More data to be received, NonZero = Done)
	\param user1 
	\param user2
	\param[out] PAUSE Set to nonzero value if you do not want the system to read any more data from the network
*/
typedef void(*ILibWebClient_OnResponse)(ILibWebClient_StateObject WebStateObject,int InterruptFlag,struct packetheader *header,char *bodyBuffer,int *beginPointer,int endPointer,int done,void *user1,void *user2,int *PAUSE);
/*! \typedef ILibWebClient_OnSendOK
	\brief Handler for when pending send operations have completed
	\par
	<B>Note:</B> This handler will only be called after all data from any call(s) to \a ILibWebClient_StreamRequestBody
	have completed.
	\param sender The \a ILibWebClient_StateObject whos pending sends have completed
	\param user1 User1 object that was associated with this connection
	\param user2 User2 object that was associated with this connection
*/
typedef void(*ILibWebClient_OnSendOK)(ILibWebClient_StateObject sender, void *user1, void *user2);
/*! \typedef ILibWebClient_OnDisconnect
	\brief Handler for when the session has disconnected
	\param sender The \a ILibWebClient_StateObject that has been disconnected
	\param request The ILibWebClient_RequestToken that represents the request that was in progress when the disconnect happened.
*/
typedef void(*ILibWebClient_OnDisconnect)(ILibWebClient_StateObject sender, ILibWebClient_RequestToken request);

//
// This is the number of seconds that a connection must be idle for, before it will
// be automatically closed. Idle means there are no pending requests
//
/*! \def HTTP_SESSION_IDLE_TIMEOUT
	\brief This is the number of seconds that a connection must be idle for, before it will be automatically closed.
	\par
	Idle means there are no pending requests
*/
#define HTTP_SESSION_IDLE_TIMEOUT 3

/*! \def HTTP_CONNECT_RETRY_COUNT
	\brief This is the number of times, an HTTP connection will be attempted, before it fails.
	\par
	This module utilizes an exponential backoff algorithm. That is, it will retry immediately, then it will retry after 1 second, then 2, then 4, etc.
*/
#define HTTP_CONNECT_RETRY_COUNT 4

/*! \def INITIAL_BUFFER_SIZE
	\brief This initial size of the receive buffer
*/
#define INITIAL_BUFFER_SIZE 65535


ILibWebClient_RequestManager ILibCreateWebClient(int PoolSize,void *Chain);
ILibWebClient_StateObject ILibCreateWebClientEx(ILibWebClient_OnResponse OnResponse, ILibAsyncSocket_SocketModule socketModule, void *user1, void *user2);

void ILibWebClient_OnBufferReAllocate(ILibAsyncSocket_SocketModule token, void *user, ptrdiff_t offSet);
void ILibWebClient_OnData(ILibAsyncSocket_SocketModule socketModule,char* buffer,int *p_beginPointer, int endPointer,ILibAsyncSocket_OnInterrupt *InterruptPtr, void **user, int *PAUSE);
void ILibDestroyWebClient(void *object);

void ILibWebClient_DestroyWebClientDataObject(ILibWebClient_StateObject token);
struct packetheader *ILibWebClient_GetHeaderFromDataObject(ILibWebClient_StateObject token);

ILibWebClient_RequestToken ILibWebClient_PipelineRequestEx(
	ILibWebClient_RequestManager WebClient, 
	struct sockaddr_in *RemoteEndpoint, 
	char *headerBuffer,
	int headerBufferLength,
	int headerBuffer_FREE,
	char *bodyBuffer,
	int bodyBufferLength,
	int bodyBuffer_FREE,
	ILibWebClient_OnResponse OnResponse,
	void *user1,
	void *user2);

ILibWebClient_RequestToken ILibWebClient_PipelineRequest(
	ILibWebClient_RequestManager WebClient, 
	struct sockaddr_in *RemoteEndpoint, 
	struct packetheader *packet,
	ILibWebClient_OnResponse OnResponse,
	void *user1,
	void *user2);

int ILibWebClient_StreamRequestBody(
									 ILibWebClient_RequestToken token, 
									 char *body,
									 int bodyLength, 
									 enum ILibAsyncSocket_MemoryOwnership MemoryOwnership,
									 int done
									 );
ILibWebClient_RequestToken ILibWebClient_PipelineStreamedRequest(
									ILibWebClient_RequestManager WebClient,
									struct sockaddr_in *RemoteEndpoint,
									struct packetheader *packet,
									ILibWebClient_OnResponse OnResponse,
									ILibWebClient_OnSendOK OnSendOK,
									void *user1,
									void *user2);



void ILibWebClient_FinishedResponse_Server(ILibWebClient_StateObject wcdo);
void ILibWebClient_DeleteRequests(ILibWebClient_RequestManager WebClientToken,char *IP,int Port);
void ILibWebClient_Resume(ILibWebClient_StateObject wcdo);
void ILibWebClient_Pause(ILibWebClient_StateObject wcdo);
void ILibWebClient_Disconnect(ILibWebClient_StateObject wcdo);
void ILibWebClient_CancelRequest(ILibWebClient_RequestToken RequestToken);
ILibWebClient_RequestToken ILibWebClient_GetRequestToken_FromStateObject(ILibWebClient_StateObject WebStateObject);
ILibWebClient_StateObject ILibWebClient_GetStateObjectFromRequestToken(ILibWebClient_RequestToken token);

void ILibWebClient_Parse_ContentRange(char *contentRange, int *Start, int *End, int *TotalLength);
enum ILibWebClient_Range_Result ILibWebClient_Parse_Range(char *Range, long *Start, long *Length, long TotalLength);

void ILibWebClient_SetMaxConcurrentSessionsToServer(ILibWebClient_RequestManager WebClient, int maxConnections);
void ILibWebClient_SetUser(ILibWebClient_RequestManager manager, void *user);
void* ILibWebClient_GetUser(ILibWebClient_RequestManager manager);
void* ILibWebClient_GetChain(ILibWebClient_RequestManager manager);

#ifdef UPNP_DEBUG
char *ILibWebClient_QueryWCDO(ILibWebClient_RequestManager wcm, char *query);
#endif
/*! \} */

#endif
