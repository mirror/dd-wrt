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

#ifndef ___ILibAsyncUDPSocket___
#define ___ILibAsyncUDPSocket___

/*! \file ILibAsyncUDPSocket.h 
	\brief MicroStack APIs for UDP Functionality
*/

/*! \defgroup ILibAsyncUDPSocket ILibAsyncUDPSocket Module
	\{
*/

#if defined(WIN32) || defined(_WIN32_WCE)
#include <STDDEF.H>
#else
#include <malloc.h>
#endif

#include "ILibAsyncSocket.h"

enum ILibAsyncUDPSocket_Reuse
{
	ILibAsyncUDPSocket_Reuse_EXCLUSIVE=0,	/*!< A socket is to be bound for exclusive access */
	ILibAsyncUDPSocket_Reuse_SHARED=1		/*!< A socket is to be bound for shared access */
};

/*! \typedef ILibAsyncUDPSocket_SocketModule
	\brief The handle for an ILibAsyncUDPSocket module
*/
typedef void* ILibAsyncUDPSocket_SocketModule;
/*! \typedef ILibAsyncUDPSocket_OnData
	\brief The handler that is called when data is received
	\param socketModule The \a ILibAsyncUDPSocket_SocketModule handle that received data
	\param buffer The buffer that contains the read data
	\param bufferLength The amount of data that was read
	\param remoteInterface The IP address of the source, in network order
	\param remotePort The port number of the source, in host order
	\param user	User object associated with this module
	\param user2 User2 object associated with this module
	\param[out] PAUSE Set this flag to non-zero, to prevent more data from being read
*/
typedef void(*ILibAsyncUDPSocket_OnData)(ILibAsyncUDPSocket_SocketModule socketModule,char* buffer, int bufferLength, int remoteInterface, unsigned short remotePort, void *user, void *user2, int *PAUSE);
/*! \typedef ILibAsyncUDPSocket_OnSendOK
	\brief Handler for when pending send operations have completed
	\par
	This handler will only be called if a call to \a ILibAsyncUDPSocket_SendTo returned a value greater
	than 0, which indicates that not all of the data could be sent.
	<P><B>Note:</B> On most systems, UDP data that cannot be sent will be dropped, which means that this handler
	may actually never be called.
	\param socketModule The \a ILibAsyncUDPSocket_SocketModule whos pending sends have completed
	\param user1 User object that was associated with this connection
	\param user2 User2 object that was associated with this connection
*/
typedef void(*ILibAsyncUDPSocket_OnSendOK)(ILibAsyncUDPSocket_SocketModule socketModule, void *user1, void *user2);

ILibAsyncUDPSocket_SocketModule ILibAsyncUDPSocket_CreateEx(void *Chain, int BufferSize, int localInterface, unsigned short localPortStartRange, unsigned short localPortEndRange, enum ILibAsyncUDPSocket_Reuse reuse, ILibAsyncUDPSocket_OnData OnData, ILibAsyncUDPSocket_OnSendOK OnSendOK, void *user);
/*! \def ILibAsyncUDPSocket_Create
	\brief Creates a new instance of an ILibAsyncUDPSocket module.
	\param Chain The chain to add this object to. (Chain must <B>not</B> not be running)
	\param BufferSize The size of the buffer to use
	\param localInterface The IP address to bind this socket to, in network order
	\param localPort The port to bind this socket to, in host order. (0 = Random IANA specified generic port)
	\param reuse Reuse type
	\param OnData The handler to receive data
	\param OnSendOK The handler to receive notification that pending sends have completed
	\param user User object to associate with this object
	\returns The ILibAsyncUDPSocket_SocketModule handle that was created
*/
#define ILibAsyncUDPSocket_Create(Chain, BufferSize, localInterface, localPort, reuse, OnData , OnSendOK, user)localPort==0?ILibAsyncUDPSocket_CreateEx(Chain, BufferSize, localInterface, 50000, 65500, reuse, OnData, OnSendOK, user):ILibAsyncUDPSocket_CreateEx(Chain, BufferSize, localInterface, localPort, localPort, reuse, OnData, OnSendOK, user)

int ILibAsyncUDPSocket_JoinMulticastGroup(ILibAsyncUDPSocket_SocketModule module, int localInterface, int remoteInterface);
int ILibAsyncUDPSocket_SetMulticastInterface(ILibAsyncUDPSocket_SocketModule module, int localInterface);
int ILibAsyncUDPSocket_SetMulticastTTL(ILibAsyncUDPSocket_SocketModule module, unsigned char TTL);

/*! \def ILibAsyncUDPSocket_GetPendingBytesToSend
	\brief Returns the number of bytes that are pending to be sent
	\param socketModule The ILibAsyncUDPSocket_SocketModule handle to query
	\returns Number of pending bytes
*/
#define ILibAsyncUDPSocket_GetPendingBytesToSend(socketModule) ILibAsyncSocket_GetPendingBytesToSend(socketModule)
/*! \def ILibAsyncUDPSocket_GetTotalBytesSent
	\brief Returns the total number of bytes that have been sent, since the last reset
	\param socketModule The ILibAsyncUDPSocket_SocketModule handle to query
	\returns Number of bytes sent
*/
#define ILibAsyncUDPSocket_GetTotalBytesSent(socketModule) ILibAsyncSocket_GetTotalBytesSent(socketModule)
/*! \def ILibAsyncUDPSocket_ResetTotalBytesSent
	\brief Resets the total bytes sent counter
	\param socketModule The ILibAsyncUDPSocket_SocketModule handle to reset
*/
#define ILibAsyncUDPSocket_ResetTotalBytesSent(socketModule) ILibAsyncSocket_ResetTotalBytesSent(socketModule)


/*! \def ILibAsyncUDPSocket_SendTo
	\brief Sends a UDP packet
	\param socketModule The ILibAsyncUDPSocket_SocketModule handle to send a packet on
	\param remoteInterface The IP address in network order, to send the packet to
	\param remotePort The port numer in host order to send the packet to
	\param buffer The buffer to send
	\param length The length of \a buffer
	\param UserFree The ILibAsyncSocket_MemoryOwnership flag indicating how the memory in \a buffer is to be handled
	\returns The ILibAsyncSocket_SendStatus status of the packet that was sent
*/
#define ILibAsyncUDPSocket_SendTo(socketModule, remoteInterface, remotePort, buffer, length, UserFree) ILibAsyncSocket_SendTo(socketModule, buffer, length, remoteInterface, remotePort, UserFree)
/*! \def ILibAsyncUDPSocket_GetLocalInterface
	\brief Get's the bounded IP address in network order
	\param socketModule The ILibAsyncUDPSocket_SocketModule to query
	\returns The local bounded IP address in network order
*/
#define ILibAsyncUDPSocket_GetLocalInterface(socketModule) ILibAsyncSocket_GetLocalInterface(socketModule)
/*! \def ILibAsyncUDPSocket_GetLocalPort
	\brief Get's the bounded port in host order
	\param socketModule The ILibAsyncUDPSocket_SocketModule to query
	\returns The local bounded port in host order
*/
#define ILibAsyncUDPSocket_GetLocalPort(socketModule) ILibAsyncSocket_GetLocalPort(socketModule)

#define ILibAsyncUDPSocket_Resume(socketModule) ILibAsyncSocket_Resume(socketModule)  
/*! \} */
#endif

