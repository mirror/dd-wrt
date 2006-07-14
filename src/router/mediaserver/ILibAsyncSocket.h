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
 * $Workfile: ILibAsyncSocket.h
 * $Revision: #1.0.2384.30305
 * $Author:   Intel Corporation, Intel Device Builder
 * $Date:     Thursday, July 13, 2006
 *
 *
 *
 */

#ifndef ___ILibAsyncSocket___
#define ___ILibAsyncSocket___

/*! \file ILibAsyncSocket.h 
	\brief MicroStack APIs for TCP Client Functionality
*/

/*! \defgroup ILibAsyncSocket ILibAsyncSocket Module
	\{
*/

#if defined(WIN32) || defined(_WIN32_WCE)
#include <STDDEF.H>
#else
#include <malloc.h>
#endif

#if defined(_WIN32_WCE)
#ifndef ptrdiff_t
#define ptrdiff_t long
#endif
#endif

/*! \def MEMORYCHUNKSIZE
	\brief Incrementally grow the buffer by this amount of bytes
*/
#define MEMORYCHUNKSIZE 4096

enum ILibAsyncSocket_SendStatus
{
	ILibAsyncSocket_ALL_DATA_SENT					= 0, /*!< All of the data has already been sent */
	ILibAsyncSocket_NOT_ALL_DATA_SENT_YET			= 1, /*!< Not all of the data could be sent, but is queued to be sent as soon as possible */
	ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR		= -4 /*!< A send operation was attmepted on a closed socket */
};

/*! \enum ILibAsyncSocket_MemoryOwnership
	\brief Enumeration values for Memory Ownership of variables
*/
enum ILibAsyncSocket_MemoryOwnership
{
	ILibAsyncSocket_MemoryOwnership_CHAIN=0, /*!< The Microstack will own this memory, and free it when it is done with it */
	ILibAsyncSocket_MemoryOwnership_STATIC=1, /*!< This memory is static, so the Microstack will not free it, and assume it will not go away, so it won't copy it either */
	ILibAsyncSocket_MemoryOwnership_USER=2 /*!< The Microstack doesn't own this memory, so if necessary the memory will be copied */
};

/*! \typedef ILibAsyncSocket_SocketModule
	\brief The handle for an ILibAsyncSocket module
*/
typedef void* ILibAsyncSocket_SocketModule;
/*! \typedef ILibAsyncSocket_OnInterrupt
	\brief Handler for when a session was interrupted by a call to ILibStopChain
	\param socketModule The \a ILibAsyncSocket_SocketModule that was interrupted
	\param user The user object that was associated with this connection
*/
typedef void(*ILibAsyncSocket_OnInterrupt)(ILibAsyncSocket_SocketModule socketModule, void *user);
/*! \typedef ILibAsyncSocket_OnData
	\brief Handler for when data is received
	\par
	<B>Note on memory handling:</B>
	When you process the received buffer, you must advance \a p_beginPointer the number of bytes that you 
	have processed. If \a p_beginPointer does not equal \a endPointer when this method completes,
	the system will continue to reclaim any memory that has already been processed, and call this method again
	until no more memory has been processed. If no memory has been processed, and more data has been received
	on the network, the buffer will be automatically grown (according to a specific alogrythm), to accomodate any new data.
	\param socketModule The \a ILibAsyncSocket_SocketModule that received data
	\param buffer The data that was received
	\param[in,out] p_beginPointer The start index of the data that was received
	\param endPointer The end index of the data that was received
	\param[in,out] OnInterrupt Set this pointer to receive notification if this session is interrupted
	\param[in,out] user Set a custom user object
	\param[out] PAUSE Flag to indicate if the system should continue reading data off the network
*/
typedef void(*ILibAsyncSocket_OnData)(ILibAsyncSocket_SocketModule socketModule,char* buffer,int *p_beginPointer, int endPointer,ILibAsyncSocket_OnInterrupt* OnInterrupt, void **user, int *PAUSE);
/*! \typedef ILibAsyncSocket_OnConnect
	\brief Handler for when a connection is made
	\param socketModule The \a ILibAsyncSocket_SocketModule that was connected
	\param user The user object that was associated with this object
*/
typedef void(*ILibAsyncSocket_OnConnect)(ILibAsyncSocket_SocketModule socketModule, int Connected, void *user);
/*! \typedef ILibAsyncSocket_OnDisconnect
	\brief Handler for when a connection is terminated normally
	\param socketModule The \a ILibAsyncSocket_SocketModule that was disconnected
	\param user User object that was associated with this connection
*/
typedef void(*ILibAsyncSocket_OnDisconnect)(ILibAsyncSocket_SocketModule socketModule, void *user);
/*! \typedef ILibAsyncSocket_OnSendOK
	\brief Handler for when pending send operations have completed
	\par
	This handler will only be called if a call to \a ILibAsyncSocket_Send returned a value greater
	than 0, which indicates that not all of the data could be sent.
	\param socketModule The \a ILibAsyncSocket_SocketModule whos pending sends have completed
	\param user User object that was associated with this connection
*/
typedef void(*ILibAsyncSocket_OnSendOK)(ILibAsyncSocket_SocketModule socketModule, void *user);
/*! \typedef ILibAsyncSocket_OnBufferReAllocated
	\brief Handler for when the internal data buffer has been resized.
	\par
	<B>Note:</B> This is only useful if you are storing pointer values into the buffer supplied in \a ILibAsyncSocket_OnData. 
	\param AsyncSocketToken The \a ILibAsyncSocket_SocketModule whos buffer was resized
	\param user The user object that was associated with this connection
	\param newOffset The new offset differential. Simply add this value to your existing pointers, to obtain the correct pointer into the resized buffer.
*/
typedef void(*ILibAsyncSocket_OnBufferReAllocated)(ILibAsyncSocket_SocketModule AsyncSocketToken, void *user, ptrdiff_t newOffset);




void ILibAsyncSocket_SetReAllocateNotificationCallback(ILibAsyncSocket_SocketModule AsyncSocketToken, ILibAsyncSocket_OnBufferReAllocated Callback);
void * ILibAsyncSocket_GetUser(ILibAsyncSocket_SocketModule socketModule);

ILibAsyncSocket_SocketModule ILibCreateAsyncSocketModule(void *Chain, int initialBufferSize, ILibAsyncSocket_OnData , ILibAsyncSocket_OnConnect OnConnect ,ILibAsyncSocket_OnDisconnect OnDisconnect,ILibAsyncSocket_OnSendOK OnSendOK);

void* ILibAsyncSocket_GetSocket(ILibAsyncSocket_SocketModule module);

unsigned int ILibAsyncSocket_GetPendingBytesToSend(ILibAsyncSocket_SocketModule socketModule);
unsigned int ILibAsyncSocket_GetTotalBytesSent(ILibAsyncSocket_SocketModule socketModule);
void ILibAsyncSocket_ResetTotalBytesSent(ILibAsyncSocket_SocketModule socketModule);

void ILibAsyncSocket_ConnectTo(ILibAsyncSocket_SocketModule socketModule, int localInterface, int remoteInterface, int remotePortNumber,ILibAsyncSocket_OnInterrupt InterruptPtr, void *user);
enum ILibAsyncSocket_SendStatus ILibAsyncSocket_SendTo(ILibAsyncSocket_SocketModule socketModule, char* buffer, int length, int remoteAddress, unsigned short remotePort, enum ILibAsyncSocket_MemoryOwnership UserFree);
/*! \def ILibAsyncSocket_Send
	\brief Sends data onto the TCP stream
	\param socketModule The \a ILibAsyncSocket_SocketModule to send data on
	\param buffer The data to be sent
	\param length The length of \a buffer
	\param UserFree The \a ILibAsyncSocket_MemoryOwnership enumeration, that identifies how the memory pointer to by \a buffer is to be handled
	\returns \a ILibAsyncSocket_SendStatus indicating the send status
*/
#define ILibAsyncSocket_Send(socketModule, buffer, length, UserFree) ILibAsyncSocket_SendTo(socketModule, buffer, length, 0, 0, UserFree)
void ILibAsyncSocket_Disconnect(ILibAsyncSocket_SocketModule socketModule);
void ILibAsyncSocket_GetBuffer(ILibAsyncSocket_SocketModule socketModule, char **buffer, int *BeginPointer, int *EndPointer);

void ILibAsyncSocket_UseThisSocket(ILibAsyncSocket_SocketModule socketModule,void* TheSocket,ILibAsyncSocket_OnInterrupt InterruptPtr,void *user);
void ILibAsyncSocket_SetRemoteAddress(ILibAsyncSocket_SocketModule socketModule,int RemoteAddress);
void ILibAsyncSocket_SetLocalInterface2(ILibAsyncSocket_SocketModule module, int localInterface2);

int ILibAsyncSocket_IsFree(ILibAsyncSocket_SocketModule socketModule);
int ILibAsyncSocket_GetLocalInterface(ILibAsyncSocket_SocketModule socketModule);
unsigned short ILibAsyncSocket_GetLocalPort(ILibAsyncSocket_SocketModule socketModule);
int ILibAsyncSocket_GetRemoteInterface(ILibAsyncSocket_SocketModule socketModule);
unsigned short ILibAsyncSocket_GetRemotePort(ILibAsyncSocket_SocketModule socketModule);

void ILibAsyncSocket_Resume(ILibAsyncSocket_SocketModule socketModule);

/*! \} */
#endif
