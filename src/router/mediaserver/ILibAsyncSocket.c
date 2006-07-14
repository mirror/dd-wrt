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
 * $Workfile: ILibAsyncSocket.c
 * $Revision: #1.0.2384.30305
 * $Author:   Intel Corporation, Intel Device Builder
 * $Date:     Thursday, July 13, 2006
 *
 *
 *
 */

#ifdef MEMORY_CHECK
	#include <assert.h>
	#define MEMCHECK(x) x
#else
	#define MEMCHECK(x)
#endif

#if defined(WIN32)
	#define _CRTDBG_MAP_ALLOC
#endif

#if defined(WINSOCK2)
	#include <winsock2.h>
	#include <ws2tcpip.h>
#elif defined(WINSOCK1)
	#include <winsock.h>
	#include <wininet.h>
#endif
#include "ILibParsers.h"
#include "ILibAsyncSocket.h"

#if defined(WIN32) && !defined(_WIN32_WCE)
	#include <crtdbg.h>
#endif


#define DEBUGSTATEMENT(x)


#ifdef SEMAPHORE_TRACKING
#define SEM_TRACK(x) x
void AsyncSocket_TrackLock(const char* MethodName, int Occurance, void *data)
{
	char v[100];

	sprintf(v,"  LOCK[%s, %d] (%x)\r\n",MethodName,Occurance,data);
#ifdef WIN32
	OutputDebugString(v);
#else
	printf(v);
#endif
}
void AsyncSocket_TrackUnLock(const char* MethodName, int Occurance, void *data)
{
	char v[100];

	sprintf(v,"UNLOCK[%s, %d] (%x)\r\n",MethodName,Occurance,data);
#ifdef WIN32
	OutputDebugString(v);
#else
	printf(v);
#endif
}
#else
#define SEM_TRACK(x)
#endif

struct ILibAsyncSocket_SendData
{
	char* buffer;
	int bufferSize;
	int bytesSent;

	int remoteAddress;
	unsigned short remotePort;

	int UserFree;
	struct ILibAsyncSocket_SendData *Next;
};

struct ILibAsyncSocketModule
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);
	void *Chain;

	unsigned int PendingBytesToSend;
	unsigned int TotalBytesSent;

	#if defined(_WIN32_WCE) || defined(WIN32)
		SOCKET internalSocket;
	#elif defined(_POSIX)
		int internalSocket;
	#endif

	int RemoteIPAddress;
	int RemotePort;
	int LocalIPAddress;
	int LocalIPAddress2;

	struct sockaddr_in addr;

	ILibAsyncSocket_OnData OnData;
	ILibAsyncSocket_OnConnect OnConnect;
	ILibAsyncSocket_OnDisconnect OnDisconnect;
	ILibAsyncSocket_OnSendOK OnSendOK;
	ILibAsyncSocket_OnInterrupt OnInterrupt;

	ILibAsyncSocket_OnBufferReAllocated OnBufferReAllocated;

	void *LifeTime;
	void *TimeoutTimer;

	void *user;
	int PAUSE;
	
	int FinConnect;
	int BeginPointer;
	int EndPointer;
	
	char* buffer;
	int MallocSize;
	int InitialSize;

	struct ILibAsyncSocket_SendData *PendingSend_Head;
	struct ILibAsyncSocket_SendData *PendingSend_Tail;
	sem_t SendLock;
};

void ILibAsyncSocket_PostSelect(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
void ILibAsyncSocket_PreSelect(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);

//
// An internal method called by Chain as Destroy, to cleanup AsyncSocket
//
// <param name="socketModule">The AsyncSocketModule</param>
void ILibAsyncSocket_Destroy(void *socketModule)
{
	struct ILibAsyncSocketModule* module = (struct ILibAsyncSocketModule*)socketModule;
	struct ILibAsyncSocket_SendData *temp,*current;


	//
	// Call the interrupt event if necessary
	//
	if(!ILibAsyncSocket_IsFree(module))
	{
		if(module->OnInterrupt!=NULL)
		{
			module->OnInterrupt(module,module->user);
		}
	}

	//
	// Close socket if necessary
	//
	if(module->internalSocket!=~0)
	{
		#if defined(_WIN32_WCE) || defined(WIN32)
			#if defined(WINSOCK2)
				shutdown(module->internalSocket,SD_BOTH);
			#endif
			closesocket(module->internalSocket);
		#elif defined(_POSIX)
			shutdown(module->internalSocket,SHUT_RDWR);
			close(module->internalSocket);
		#endif
	}
	


	//
	// Free the buffer if necessary
	//
	if(module->buffer!=NULL)
	{
		free(module->buffer);
		module->buffer = NULL;
		module->MallocSize = 0;
	}
	
	//
	// Clear all the data that is pending to be sent
	//
	temp=current=module->PendingSend_Head;
	while(current!=NULL)
	{
		temp = current->Next;
		if(current->UserFree==0)
		{
			free(current->buffer);
		}
		free(current);
		current = temp;
	}
	
	sem_destroy(&(module->SendLock));
}
/*! \fn ILibAsyncSocket_SetReAllocateNotificationCallback(ILibAsyncSocket_SocketModule AsyncSocketToken, ILibAsyncSocket_OnBufferReAllocated Callback)
	\brief Set the callback handler for when the internal data buffer has been resized
	\param AsyncSocketToken The specific connection to set the callback with
	\param Callback The callback handler to set
*/
void ILibAsyncSocket_SetReAllocateNotificationCallback(ILibAsyncSocket_SocketModule AsyncSocketToken, ILibAsyncSocket_OnBufferReAllocated Callback)
{
	if(AsyncSocketToken!=NULL)
	{
		((struct ILibAsyncSocketModule*)AsyncSocketToken)->OnBufferReAllocated = Callback;
	}
}

/*! \fn ILibCreateAsyncSocketModule(void *Chain, int initialBufferSize, ILibAsyncSocket_OnData OnData, ILibAsyncSocket_OnConnect OnConnect, ILibAsyncSocket_OnDisconnect OnDisconnect,ILibAsyncSocket_OnSendOK OnSendOK)
	\brief Creates a new AsyncSocketModule
	\param Chain The chain to add this module to. (Chain must <B>not</B> be running)
	\param initialBufferSize The initial size of the receive buffer
	\param OnData Function Pointer that triggers when Data is received
	\param OnConnect Function Pointer that triggers upon successfull connection establishment
	\param OnDisconnect Function Pointer that triggers upon disconnect
	\param OnSendOK Function Pointer that triggers when pending sends are complete
	\returns An ILibAsyncSocket token
*/
ILibAsyncSocket_SocketModule ILibCreateAsyncSocketModule(void *Chain, int initialBufferSize, ILibAsyncSocket_OnData OnData, ILibAsyncSocket_OnConnect OnConnect, ILibAsyncSocket_OnDisconnect OnDisconnect,ILibAsyncSocket_OnSendOK OnSendOK)
{
	struct ILibAsyncSocketModule *RetVal = (struct ILibAsyncSocketModule*)malloc(sizeof(struct ILibAsyncSocketModule));
	memset(RetVal,0,sizeof(struct ILibAsyncSocketModule));
	RetVal->PreSelect = &ILibAsyncSocket_PreSelect;
	RetVal->PostSelect = &ILibAsyncSocket_PostSelect;
	RetVal->Destroy = &ILibAsyncSocket_Destroy;
	
	RetVal->internalSocket = -1;
	RetVal->OnData = OnData;
	RetVal->OnConnect = OnConnect;
	RetVal->OnDisconnect = OnDisconnect;
	RetVal->OnSendOK = OnSendOK;
	RetVal->buffer = (char*)malloc(initialBufferSize);
	RetVal->InitialSize = initialBufferSize;
	RetVal->MallocSize = initialBufferSize;

	RetVal->LifeTime = ILibCreateLifeTime(Chain);
	RetVal->TimeoutTimer = ILibCreateLifeTime(Chain);

	sem_init(&(RetVal->SendLock),0,1);
	
	RetVal->Chain = Chain;
	ILibAddToChain(Chain,RetVal);

	return((void*)RetVal);
}

/*! \fn ILibAsyncSocket_ClearPendingSend(ILibAsyncSocket_SocketModule socketModule)
	\brief Clears all the pending data to be sent for an AsyncSocket
	\param socketModule The ILibAsyncSocket to clear
*/
void ILibAsyncSocket_ClearPendingSend(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	struct ILibAsyncSocket_SendData *data,*temp;
	
	data = module->PendingSend_Head;
	module->PendingSend_Tail = NULL;
	while(data!=NULL)
	{
		temp = data->Next;
		if(data->UserFree==0)
		{
			//
			// We only need to free this if we have ownership of this memory
			//
			free(data->buffer);
		}
		free(data);
		data = temp;
	}
	module->PendingSend_Head = NULL;
	module->PendingBytesToSend=0;
}

/*! \fn ILibAsyncSocket_SendTo(ILibAsyncSocket_SocketModule socketModule, char* buffer, int length, int remoteAddress, unsigned short remotePort, enum ILibAsyncSocket_MemoryOwnership UserFree)
	\brief Sends data on an AsyncSocket module to a specific destination. (Valid only for <B>UDP</B>)
	\param socketModule The ILibAsyncSocket module to send data on
	\param buffer The buffer to send
	\param length The length of the buffer to send
	\param remoteAddress The IPAddress of the destination 
	\param remotePort The Port number of the destination
	\param UserFree Flag indicating memory ownership. 
	\returns \a ILibAsyncSocket_SendStatus indicating the send status
*/
enum ILibAsyncSocket_SendStatus ILibAsyncSocket_SendTo(ILibAsyncSocket_SocketModule socketModule, char* buffer, int length, int remoteAddress, unsigned short remotePort, enum ILibAsyncSocket_MemoryOwnership UserFree)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	struct ILibAsyncSocket_SendData *data = (struct ILibAsyncSocket_SendData*)malloc(sizeof(struct ILibAsyncSocket_SendData));
	int unblock=0;
	int bytesSent;

	struct sockaddr_in dest;
	int destlen = sizeof(dest);

	memset(data,0,sizeof(struct ILibAsyncSocket_SendData));

	data->buffer = buffer;
	data->bufferSize = length;
	data->bytesSent = 0;
	data->UserFree = UserFree;
	data->remoteAddress = remoteAddress;
	data->remotePort = remotePort;
	data->Next = NULL;

	SEM_TRACK(AsyncSocket_TrackLock("ILibAsyncSocket_Send",1,module);)
	sem_wait(&(module->SendLock));
	if(module->internalSocket==~0)
	{
		// Too Bad, the socket closed
		if(UserFree==0){free(buffer);}
		free(data);
		SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_Send",2,module);)
		sem_post(&(module->SendLock));
		return(ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR);
	}

	module->PendingBytesToSend += length;
	if(module->PendingSend_Tail!=NULL)
	{
		//
		// There are still bytes that are pending to be sent, so we need to queue this up
		//
		module->PendingSend_Tail->Next = data;
		module->PendingSend_Tail = data;
		unblock=1;
		if(UserFree==ILibAsyncSocket_MemoryOwnership_USER)
		{
			//
			// If we don't own this memory, we need to copy the buffer,
			// because the user may free this memory before we have a chance
			// to send it
			//
			data->buffer = (char*)malloc(data->bufferSize);
			memcpy(data->buffer,buffer,length);
			MEMCHECK(assert(length <= data->bufferSize);)

			data->UserFree = ILibAsyncSocket_MemoryOwnership_CHAIN;
		}
	}
	else
	{
		//
		// There is no data pending to be sent, so lets go ahead and try to send it
		//
		module->PendingSend_Tail = data;
		module->PendingSend_Head = data;
		
		if(module->PendingSend_Head->remoteAddress==0 && module->PendingSend_Head->remotePort==0)
		{
#if defined(MSG_NOSIGNAL)
			bytesSent = send(module->internalSocket,module->PendingSend_Head->buffer+module->PendingSend_Head->bytesSent,module->PendingSend_Head->bufferSize-module->PendingSend_Head->bytesSent,MSG_NOSIGNAL);
#else
			bytesSent = send(module->internalSocket,module->PendingSend_Head->buffer+module->PendingSend_Head->bytesSent,module->PendingSend_Head->bufferSize-module->PendingSend_Head->bytesSent,0);
#endif
		}
		else
		{
			dest.sin_addr.s_addr = module->PendingSend_Head->remoteAddress;
			dest.sin_port = htons(module->PendingSend_Head->remotePort);
			dest.sin_family = AF_INET;
#if defined(MSG_NOSIGNAL)
			bytesSent = sendto(module->internalSocket,module->PendingSend_Head->buffer+module->PendingSend_Head->bytesSent,module->PendingSend_Head->bufferSize-module->PendingSend_Head->bytesSent,MSG_NOSIGNAL,(struct sockaddr*)&dest,destlen);
#else
			bytesSent = sendto(module->internalSocket,module->PendingSend_Head->buffer+module->PendingSend_Head->bytesSent,module->PendingSend_Head->bufferSize-module->PendingSend_Head->bytesSent,0,(struct sockaddr*)&dest,destlen);
#endif
		}
		if(bytesSent>0)
		{
			//
			// We were able to send something, so lets increment the counters
			//
			module->PendingSend_Head->bytesSent+=bytesSent;
			module->PendingBytesToSend -= bytesSent;
			module->TotalBytesSent += bytesSent;
		}
		if(bytesSent==-1)
		{
			// 
			// Send returned an error, so lets figure out what it was,
			// as it could be normal
			//
#if defined(_WIN32_WCE) || defined(WIN32)
			bytesSent = WSAGetLastError();
			if(bytesSent!=WSAEWOULDBLOCK)
#else
			if(errno!=EWOULDBLOCK)
#endif
			{
				//
				// Most likely the socket closed while we tried to send
				//
				if(UserFree==0){free(buffer);}
				module->PendingSend_Head = module->PendingSend_Tail = NULL;
				free(data);
				SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_Send",3,module);)
				sem_post(&(module->SendLock));
				
				//
				//Ensure Calling On_Disconnect with MicroStackThread
				//
				ILibLifeTime_Add(module->LifeTime,socketModule,0,&ILibAsyncSocket_Disconnect,NULL);
				
				return(ILibAsyncSocket_SEND_ON_CLOSED_SOCKET_ERROR);
			}
		}
		if(module->PendingSend_Head->bytesSent==module->PendingSend_Head->bufferSize)
		{
			//
			// All of the data has been sent
			//
			if(UserFree==0){free(module->PendingSend_Head->buffer);}
			module->PendingSend_Tail = NULL;
			free(module->PendingSend_Head);
			module->PendingSend_Head = NULL;
		}
		else
		{
			//
			// All of the data wasn't sent, so we need to copy the buffer
			// if we don't own the memory, because the user may free the
			// memory, before we have a chance to complete sending it.
			//
			if(UserFree==ILibAsyncSocket_MemoryOwnership_USER)
			{
				data->buffer = (char*)malloc(data->bufferSize);
				memcpy(data->buffer,buffer,length);
				MEMCHECK(assert(length <= data->bufferSize);)

				data->UserFree = ILibAsyncSocket_MemoryOwnership_CHAIN;
			}
			unblock = 1;
		}

	}
	SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_Send",4,module);)
	sem_post(&(module->SendLock));
	if(unblock!=0) {ILibForceUnBlockChain(module->Chain);}
	return(unblock);
}

/*! \fn ILibAsyncSocket_Disconnect(ILibAsyncSocket_SocketModule socketModule)
	\brief Disconnects an ILibAsyncSocket
	\param socketModule The ILibAsyncSocket to disconnect
*/
void ILibAsyncSocket_Disconnect(ILibAsyncSocket_SocketModule socketModule)
{
	#if defined(_WIN32_WCE) || defined(WIN32)
		SOCKET s;
	#elif defined(_POSIX)
		int s;
	#endif

	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	if(module==NULL){return;}


	if(!ILibIsChainBeingDestroyed(module->Chain))
	{
		ILibLifeTime_Remove(module->TimeoutTimer,module);
	}

	SEM_TRACK(AsyncSocket_TrackLock("ILibAsyncSocket_Disconnect",1,module);)
	sem_wait(&(module->SendLock));
	
	if(module->internalSocket!=~0)
	{
		//
		// There is an associated socket that is still valid, so we need to close it
		//
		module->PAUSE = 1;
		s = module->internalSocket;
		module->internalSocket = ~0;
		if(s!=-1)
		{
			#if defined(_WIN32_WCE) || defined(WIN32)
					#if defined(WINSOCK2)
						shutdown(s,SD_BOTH);
					#endif
					closesocket(s);
			#elif defined(_POSIX)
					shutdown(s,SHUT_RDWR);
					close(s);
			#endif
		}

		//
		// Since the socket is closing, we need to clear the data that is pending to be sent
		//
		ILibAsyncSocket_ClearPendingSend(socketModule);
		SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_Disconnect",2,module);)
		sem_post(&(module->SendLock));
		if(module->OnDisconnect!=NULL)
		{
			//
			// Trigger the OnDissconnect event if necessary
			//
			module->OnDisconnect(module,module->user);
		}
	}
	else
	{
		SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_Disconnect",3,module);)
		sem_post(&(module->SendLock));
	}
}

void ILibAsyncSocket_ConnectTimeout(void *socketModule)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	

	sem_wait(&(module->SendLock));
	/* Connection Timeout */
	#if defined(_WIN32_WCE) || defined(WIN32)
		#if defined(WINSOCK2)
			shutdown(module->internalSocket,SD_BOTH);
		#endif
		closesocket(module->internalSocket);
	#elif defined(_POSIX)
		shutdown(module->internalSocket,SHUT_RDWR);
		close(module->internalSocket);
	#endif
	module->internalSocket = ~0;
	sem_post(&(module->SendLock));

	if(module->OnConnect!=NULL)
	{
		module->OnConnect(module,0,module->user);
	}	
}

/*! \fn ILibAsyncSocket_ConnectTo(ILibAsyncSocket_SocketModule socketModule, int localInterface, int remoteInterface, int remotePortNumber, ILibAsyncSocket_OnInterrupt InterruptPtr,void *user)
	\brief Attempts to establish a TCP connection
	\param socketModule The ILibAsyncSocket to initiate the connection
	\param localInterface The interface to use to establish the connection
	\param remoteInterface The remote interface to connect to
	\param remotePortNumber The remote port to connect to
	\param InterruptPtr Function Pointer that triggers if connection attempt is interrupted
	\param user User object that will be passed to the \a OnConnect method
*/
void ILibAsyncSocket_ConnectTo(void* socketModule, int localInterface, int remoteInterface, int remotePortNumber, ILibAsyncSocket_OnInterrupt InterruptPtr,void *user)
{
	int flags;
	struct sockaddr_in addr;
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	

	if(module==NULL){return;}

	module->RemoteIPAddress = remoteInterface;
	module->RemotePort = remotePortNumber;
	module->PendingBytesToSend = 0;
	module->TotalBytesSent = 0;
	module->PAUSE = 0;
	module->user = user;
	module->OnInterrupt = InterruptPtr;
	module->buffer = (char*)realloc(module->buffer,module->InitialSize);
	module->MallocSize = module->InitialSize;
	memset((char *)&addr, 0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = remoteInterface;

	addr.sin_port = htons((unsigned short)remotePortNumber);

	
	//
	// If there isn't a socket already allocated, we need to allocate one
	//
	if(module->internalSocket==-1)
	{
		#if defined(WIN32) || defined(_WIN32_WCE)
			ILibGetStreamSocket(localInterface,0,(HANDLE*)&(module->internalSocket));
		#else
			ILibGetStreamSocket(localInterface,0,&(module->internalSocket));
		#endif
	}
	
	//
	// Initialise the buffer pointers, since no data is in them yet.
	//
	module->FinConnect = 0;
	module->BeginPointer = 0;
	module->EndPointer = 0;
	
	//
	// Set the socket to non-blocking mode, because we need to play nice
	// and share the MicroStack thread
	//
	#if defined(_WIN32_WCE) || defined(WIN32)
		flags = 1;
		ioctlsocket(module->internalSocket,FIONBIO,&flags);
	#elif defined(_POSIX)
		flags = fcntl(module->internalSocket,F_GETFL,0);
		fcntl(module->internalSocket,F_SETFL,O_NONBLOCK|flags);
	#endif

	//
	// Turn on keep-alives for the socket
	//
	flags = 1;
	setsockopt(module->internalSocket,SOL_SOCKET,SO_KEEPALIVE,(char*)&flags,sizeof(flags));

	//
	// Connect the socket, and force the chain to unblock, since the select statement
	// doesn't have us in the fdset yet.
	//
	connect(module->internalSocket,(struct sockaddr*)&addr,sizeof(addr));

	//
	// Sometimes a Connection attempt can fail, without triggering the FD_SET. We will force
	// a failure after 30 seconds.
	//
	ILibLifeTime_Add(module->TimeoutTimer,module,30,&ILibAsyncSocket_ConnectTimeout,NULL);
	ILibForceUnBlockChain(module->Chain);
}

//
// Internal method called when data is ready to be processed on an ILibAsyncSocket
//
// <param name="Reader">The ILibAsyncSocket with pending data</param>
void ILibProcessAsyncSocket(struct ILibAsyncSocketModule *Reader, int pendingRead)
{
	int bytesReceived;
	char *temp;
	int addrlen = sizeof(Reader->addr);

	int iBeginPointer=0;
	int iEndPointer=0;
	int iPointer=0;

#if defined(_POSIX) && defined(IP_PKTINFO)
	int opt;
	struct msghdr msgh;
	struct cmsghdr *cmsg;
	struct iovec iov;
	char cbuf[1024];
	int toaddr;
#endif

	//
	// If the thing isn't paused, and the user set the pointers such that we still have data
	// in our buffers, we need to call the user back with that data, before we attempt to read
	// more data off the network
	//
	if(!pendingRead)
	{
		if(Reader->internalSocket!=~0 && Reader->PAUSE<=0 && Reader->BeginPointer!=Reader->EndPointer)
		{
			iBeginPointer = Reader->BeginPointer;
			iEndPointer = Reader->EndPointer;
			iPointer = 0;
			
			while(Reader->internalSocket!=~0 && Reader->PAUSE<=0 && Reader->BeginPointer!=Reader->EndPointer && Reader->EndPointer!=0)
			{				
				Reader->EndPointer = Reader->EndPointer-Reader->BeginPointer;
				Reader->BeginPointer = 0;
				if(Reader->OnData!=NULL)
				{
					Reader->OnData(Reader,Reader->buffer + iBeginPointer,&(iPointer),Reader->EndPointer,&(Reader->OnInterrupt),&(Reader->user),&(Reader->PAUSE));
				}
				iBeginPointer += iPointer;
				Reader->EndPointer -= iPointer;
				if(iPointer==0)
				{
					break;
				}
				iPointer = 0;
			}
			Reader->BeginPointer = iBeginPointer;
			Reader->EndPointer = iEndPointer;
		}
	}

	/* Reading Body Only */
	if(Reader->BeginPointer == Reader->EndPointer)
	{
		Reader->BeginPointer = 0;
		Reader->EndPointer = 0;
	}
	if(!pendingRead || Reader->PAUSE>0)
	{
		return;
	}
	


	//
	// If we need to grow the buffer, do it now
	//
	if(Reader->MallocSize - Reader->EndPointer <1024)
	{
		//
		// This memory reallocation sometimes causes Insure++
		// to incorrectly report a READ_DANGLING (usually in 
		// a call to ILibWebServer_StreamHeader_Raw.)
		// 
		// We verified that the problem is with Insure++ by
		// noting the value of 'temp' (0x008fa8e8), 
		// 'Reader->buffer' (0x00c55e80), and
		// 'MEMORYCHUNKSIZE' (0x00001800).
		//
		// When Insure++ reported the error, it (incorrectly) 
		// claimed that a pointer to memory address 0x00c55ea4
		// was invalid, while (correctly) citing the old memory
		// (0x008fa8e8-0x008fb0e7) as freed memory.
		// Normally Insure++ reports that the invalid pointer 
		// is pointing to someplace in the deallocated block,
		// but that wasn't the case.
		//
		Reader->MallocSize += MEMORYCHUNKSIZE;
		temp = Reader->buffer;
		Reader->buffer = (char*)realloc(Reader->buffer,Reader->MallocSize);
		//
		// If this realloc moved the buffer somewhere, we need to inform people of it
		//
		if(Reader->buffer!=temp && Reader->OnBufferReAllocated!=NULL)
		{
			Reader->OnBufferReAllocated(Reader,Reader->user,Reader->buffer-temp);
		}
	}
	else if(Reader->BeginPointer!=0)
	{
		//
		// We can save some cycles by moving the data back to the top
		// of the buffer, instead of just allocating more memory.
		//
		temp = Reader->buffer + Reader->BeginPointer;;
		memmove(Reader->buffer,temp,Reader->EndPointer-Reader->BeginPointer);
		Reader->EndPointer -= Reader->BeginPointer;
		Reader->BeginPointer = 0;
		
		//
		// Even though we didn't allocate new memory, we still moved data in the buffer, 
		// so we need to inform people of that, because it might be important
		//
		if(Reader->OnBufferReAllocated!=NULL)
		{
			Reader->OnBufferReAllocated(Reader,Reader->user,temp-Reader->buffer);
		}
	}





	sem_wait(&(Reader->SendLock));

#if defined(_POSIX) && defined(IP_PKTINFO)
if(Reader->LocalIPAddress2!=0)
{
	opt = 1;
	toaddr = Reader->LocalIPAddress2;
	setsockopt(Reader->internalSocket, SOL_IP, IP_PKTINFO, &opt, sizeof(opt));


	iov.iov_base = Reader->buffer+Reader->EndPointer;
	iov.iov_len = Reader->MallocSize-Reader->EndPointer;
	msgh.msg_control = cbuf;
	msgh.msg_controllen = sizeof(cbuf);
	msgh.msg_name = &(Reader->addr);
	msgh.msg_namelen = addrlen;
	msgh.msg_iov = &iov;
	msgh.msg_iovlen = 1;

	bytesReceived = recvmsg(Reader->internalSocket,&msgh,MSG_PEEK);
	
	for(cmsg = CMSG_FIRSTHDR(&msgh);
		cmsg != NULL && cmsg->cmsg_len >= sizeof(*cmsg);
		cmsg = CMSG_NXTHDR(&msgh,cmsg))
	{
		if(cmsg->cmsg_level == SOL_IP &&
			cmsg->cmsg_type == IP_PKTINFO)
		{
			toaddr = ((struct in_pktinfo*)CMSG_DATA(cmsg))->ipi_spec_dst.s_addr;
			break;
		}
	}
	if(Reader->LocalIPAddress2!=toaddr)
	{
		bytesReceived = recvmsg(Reader->internalSocket,&msgh,0);
		sem_post(&(Reader->SendLock));
		return;
	}
}
#endif

	bytesReceived = recvfrom(Reader->internalSocket,Reader->buffer+Reader->EndPointer,Reader->MallocSize-Reader->EndPointer,0,(struct sockaddr *)&(Reader->addr),&addrlen);

	if(Reader->addr.sin_addr.s_addr!=0)
	{
		Reader->RemoteIPAddress = Reader->addr.sin_addr.s_addr;
		Reader->RemotePort = (int)ntohs(Reader->addr.sin_port);
#if defined(WIN32) || defined(_WIN32_WCE)
		if(bytesReceived<=0 && WSAGetLastError()==WSAEMSGSIZE)
		{
			bytesReceived = Reader->MallocSize;
		}
#endif
	}

	if(bytesReceived<=0)
	{
		//
		// This means the socket was gracefully closed by the remote endpoint
		//
		SEM_TRACK(AsyncSocket_TrackLock("ILibProcessAsyncSocket",1,Reader);)
		ILibAsyncSocket_ClearPendingSend(Reader);
		SEM_TRACK(AsyncSocket_TrackUnLock("ILibProcessAsyncSocket",2,Reader);)

		#if defined(_WIN32_WCE) || defined(WIN32)
			#if defined(WINSOCK2)
				shutdown(Reader->internalSocket,SD_BOTH);
			#endif
			closesocket(Reader->internalSocket);
		#elif defined(_POSIX)
			shutdown(Reader->internalSocket,SHUT_RDWR);
			close(Reader->internalSocket);
		#endif

		Reader->internalSocket = ~0;
		sem_post(&(Reader->SendLock));

		//
		// Inform the user the socket has closed
		//
		if(Reader->OnDisconnect!=NULL)
		{
			Reader->OnDisconnect(Reader,Reader->user);
		}

		//
		// If we need to free the buffer, do so
		//
		if(Reader->buffer!=NULL)
		{
			free(Reader->buffer);
			Reader->buffer = NULL;
			Reader->MallocSize = 0;
		}
	}
	else
	{
		sem_post(&(Reader->SendLock));

		//
		// Data was read, so increment our counters
		//
		Reader->EndPointer += bytesReceived;

		//
		// Tell the user we have some data
		//
		if(Reader->OnData!=NULL)
		{
			iBeginPointer = Reader->BeginPointer;
			iPointer = 0;
			Reader->OnData(Reader,Reader->buffer + Reader->BeginPointer,&(iPointer),Reader->EndPointer - Reader->BeginPointer,&(Reader->OnInterrupt),&(Reader->user),&(Reader->PAUSE));
			Reader->BeginPointer += iPointer;
		}
		//
		// If the user set the pointers, and we still have data, call them back with the data
		//
		if(Reader->internalSocket!=~0 && Reader->PAUSE<=0 && Reader->BeginPointer!=Reader->EndPointer && Reader->BeginPointer!=0)
		{
			iBeginPointer = Reader->BeginPointer;
			iEndPointer = Reader->EndPointer;
			iPointer = 0;
			
			while(Reader->internalSocket!=~0 && Reader->PAUSE<=0 && Reader->BeginPointer!=Reader->EndPointer && Reader->EndPointer!=0)
			{				
				Reader->EndPointer = Reader->EndPointer-Reader->BeginPointer;
				Reader->BeginPointer = 0;
				if(Reader->OnData!=NULL)
				{
					Reader->OnData(Reader,Reader->buffer + iBeginPointer,&(iPointer),Reader->EndPointer,&(Reader->OnInterrupt),&(Reader->user),&(Reader->PAUSE));
				}
				iBeginPointer += iPointer;
				Reader->EndPointer -= iPointer;
				if(iPointer==0)
				{
					break;
				}
				iPointer = 0;
			}
			Reader->BeginPointer = iBeginPointer;
			Reader->EndPointer = iEndPointer;
		}
		
		//
		// If the user consumed all of the buffer, we can recycle it
		//
		if(Reader->BeginPointer==Reader->EndPointer)
		{
			Reader->BeginPointer = 0;
			Reader->EndPointer = 0;
		}
	}
}

/*! \fn ILibAsyncSocket_GetUser(ILibAsyncSocket_SocketModule socketModule)
	\brief Returns the user object
	\param socketModule The ILibAsyncSocket token to fetch the user object from
	\returns The user object
*/
void * ILibAsyncSocket_GetUser(ILibAsyncSocket_SocketModule socketModule)
{
	return(socketModule==NULL?NULL:((struct ILibAsyncSocketModule*)socketModule)->user);
}
//
// Chained PreSelect handler for ILibAsyncSocket
//
// <param name="readset"></param>
// <param name="writeset"></param>
// <param name="errorset"></param>
// <param name="blocktime"></param>
void ILibAsyncSocket_PreSelect(void* socketModule,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;

	SEM_TRACK(AsyncSocket_TrackLock("ILibAsyncSocket_PreSelect",1,module);)
	sem_wait(&(module->SendLock));

	if(module->internalSocket!=-1)
	{
		if(module->PAUSE<0)
		{
			*blocktime = 0;
		}
		if(module->FinConnect==0)
		{
			/* Not Connected Yet */
			FD_SET(module->internalSocket,writeset);
			FD_SET(module->internalSocket,errorset);
		}
		else
		{
			if(module->PAUSE==0) // Only if this is zero. <0 is resume, so we want to process first
			{
				/* Already Connected, just needs reading */
				FD_SET(module->internalSocket,readset);
				FD_SET(module->internalSocket,errorset);
			}
		}
	}

	if(module->PendingSend_Head!=NULL)
	{
		//
		// If there is pending data to be sent, then we need to check when the socket is writable
		//
		FD_SET(module->internalSocket,writeset);
	}
	SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_PreSelect",2,module);)
	sem_post(&(module->SendLock));
}
//
// Chained PostSelect handler for ILibAsyncSocket
//
// <param name="socketModule"></param>
// <param name="slct"></param>
// <param name="readset"></param>
// <param name="writeset"></param>
// <param name="errorset"></param>
void ILibAsyncSocket_PostSelect(void* socketModule,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset)
{
	int TriggerSendOK = 0;
	struct ILibAsyncSocket_SendData *temp;
	int bytesSent=0;
	int flags;
	struct sockaddr_in receivingAddress;
	int receivingAddressLength = sizeof(struct sockaddr_in);
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	int TRY_TO_SEND = 1;

	int triggerReadSet=0;
	int triggerResume=0;
	int triggerWriteSet=0;
	int triggerErrorSet=0;

	struct sockaddr_in dest;
	int destlen = sizeof(dest);

	SEM_TRACK(AsyncSocket_TrackLock("ILibAsyncSocket_PostSelect",1,module);)
	sem_wait(&(module->SendLock));

	// Write Handling
	if(module->FinConnect!=0 && module->internalSocket!=~0 && FD_ISSET(module->internalSocket,writeset)!=0)
	{
		//
		// The socket is writable, and data needs to be sent
		//

		//
		// Keep trying to send data, until we are told we can't
		//
		while(TRY_TO_SEND!=0)
		{

			if(module->PendingSend_Head->remoteAddress==0 && module->PendingSend_Head->remotePort==0)
			{
#if defined(MSG_NOSIGNAL)
				bytesSent = send(module->internalSocket,module->PendingSend_Head->buffer+module->PendingSend_Head->bytesSent,module->PendingSend_Head->bufferSize-module->PendingSend_Head->bytesSent,MSG_NOSIGNAL);
#else
				bytesSent = send(module->internalSocket,module->PendingSend_Head->buffer+module->PendingSend_Head->bytesSent,module->PendingSend_Head->bufferSize-module->PendingSend_Head->bytesSent,0);
#endif
			}
			else
			{
				dest.sin_addr.s_addr = module->PendingSend_Head->remoteAddress;
				dest.sin_port = htons(module->PendingSend_Head->remotePort);
				dest.sin_family = AF_INET;
#if defined(MSG_NOSIGNAL)
				bytesSent = sendto(module->internalSocket,module->PendingSend_Head->buffer+module->PendingSend_Head->bytesSent,module->PendingSend_Head->bufferSize-module->PendingSend_Head->bytesSent,MSG_NOSIGNAL,(struct sockaddr*)&dest,destlen);
#else
				bytesSent = sendto(module->internalSocket,module->PendingSend_Head->buffer+module->PendingSend_Head->bytesSent,module->PendingSend_Head->bufferSize-module->PendingSend_Head->bytesSent,0,(struct sockaddr*)&dest,destlen);
#endif
			}
			if(bytesSent>0)
			{
				module->PendingBytesToSend -= bytesSent;
				module->TotalBytesSent += bytesSent;
				module->PendingSend_Head->bytesSent+=bytesSent;
				if(module->PendingSend_Head->bytesSent==module->PendingSend_Head->bufferSize)
				{
					// Finished Sending this block
					if(module->PendingSend_Head==module->PendingSend_Tail)
					{
						module->PendingSend_Tail = NULL;
					}
					if(module->PendingSend_Head->UserFree==0)
					{
						free(module->PendingSend_Head->buffer);
					}
					temp = module->PendingSend_Head->Next;
					free(module->PendingSend_Head);
					module->PendingSend_Head = temp;
					if(module->PendingSend_Head==NULL) {TRY_TO_SEND=0;}
				}
				else
				{
					//
					// We sent data, but not everything that needs to get sent was sent, try again
					//
					TRY_TO_SEND = 1;
				}
			}
			if(bytesSent==-1)
			{
				// Error, clean up everything
				TRY_TO_SEND = 0;
				#if defined(_WIN32_WCE) || defined(WIN32)
					bytesSent = WSAGetLastError();
					if(bytesSent!=WSAEWOULDBLOCK)
				#else
					if(errno!=EWOULDBLOCK)
				#endif
				{
					//
					// There was an error sending
					//
					ILibAsyncSocket_ClearPendingSend(socketModule);
					ILibLifeTime_Add(module->LifeTime,socketModule,0,&ILibAsyncSocket_Disconnect,NULL);
				}
			}
		}
		//
		// This triggers OnSendOK, if all the pending data has been sent.
		//
		if(module->PendingSend_Head==NULL && bytesSent!=-1) {TriggerSendOK=1;}
		SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_PostSelect",2,module);)
		sem_post(&(module->SendLock));
		if(TriggerSendOK!=0)
		{
			module->OnSendOK(module,module->user);
		}
	}
	else
	{
		SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_PostSelect",2,module);)
		sem_post(&(module->SendLock));
	}


	SEM_TRACK(AsyncSocket_TrackLock("ILibAsyncSocket_PostSelect",1,module);)
	sem_wait(&(module->SendLock));

	//
	// Connection Handling / Read Handling
	//
	if(module->internalSocket!=~0)
	{
		if(module->FinConnect==0)
		{
			/* Not Connected Yet */
			if(FD_ISSET(module->internalSocket,writeset)!=0)
			{
				//
				// Connection was a success, so remove the timeout timer
				//
				ILibLifeTime_Remove(module->TimeoutTimer,module);

				/* Connected */
				getsockname(module->internalSocket,(struct sockaddr*)&receivingAddress,&receivingAddressLength);
				module->LocalIPAddress = receivingAddress.sin_addr.s_addr;
				module->FinConnect = 1;
				module->PAUSE = 0;
				
				//
				// Set the socket to non-blocking mode, so we can play nice and share the thread
				//
				#if defined(_WIN32_WCE) || defined(WIN32)
					flags = 1;
					ioctlsocket(module->internalSocket,FIONBIO,&flags);
				#elif defined(_POSIX)
					flags = fcntl(module->internalSocket,F_GETFL,0);
					fcntl(module->internalSocket,F_SETFL,O_NONBLOCK|flags);
				#endif

				/* Connection Complete */
				triggerWriteSet=1;
			}
			if(FD_ISSET(module->internalSocket,errorset)!=0)
			{
				//
				// Connection was a failure, so remove the timeout timer
				//
				ILibLifeTime_Remove(module->TimeoutTimer,module);

				/* Connection Failed */
				#if defined(_WIN32_WCE) || defined(WIN32)
					#if defined(WINSOCK2)	
						shutdown(module->internalSocket,SD_BOTH);
					#endif
					closesocket(module->internalSocket);
				#elif defined(_POSIX)
					shutdown(module->internalSocket,SHUT_RDWR);
					close(module->internalSocket);
				#endif
				module->internalSocket = ~0;
				triggerErrorSet=1;
			}

			SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_PostSelect",4,module);)
			sem_post(&(module->SendLock));

			if(triggerWriteSet!=0 && module->OnConnect!=NULL)
			{
				module->OnConnect(module,-1,module->user);
			}
			if(triggerErrorSet!=0 && module->OnConnect!=NULL)
			{
				module->OnConnect(module,0,module->user);
			}
		}
		else
		{
			/* Check if PeerReset */
			if(FD_ISSET(module->internalSocket,errorset)!=0)
			{
				/* Socket Closed */
				#if defined(_WIN32_WCE) || defined(WIN32)
					#if defined(WINSOCK2)
						shutdown(module->internalSocket,SD_BOTH);
					#endif
					closesocket(module->internalSocket);
				#elif defined(_POSIX)
					shutdown(module->internalSocket,SHUT_RDWR);
					close(module->internalSocket);
				#endif
				module->internalSocket = ~0;
				module->PAUSE = 1;

				ILibAsyncSocket_ClearPendingSend(socketModule);

				triggerErrorSet=1;
			}

			/* Already Connected, just needs reading */
			if(FD_ISSET(module->internalSocket,readset)!=0)
			{
				/* Data Available */
				triggerReadSet=1;
			}
			else if(module->PAUSE<0)
			{
				//
				// Someone resumed a paused connection, but the FD_SET was not triggered
				// because there is no new data on the socket.
				//
				triggerResume = 1;
				++module->PAUSE;
			}

			SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_PostSelect",4,module);)
			sem_post(&(module->SendLock));

			if(triggerErrorSet!=0 && module->OnDisconnect!=NULL)
			{
				module->OnDisconnect(module,module->user);
			}
			if(triggerReadSet!=0 || triggerResume!=0)
			{
				ILibProcessAsyncSocket(module, triggerReadSet);
			}
		}
	}
	else
	{
		SEM_TRACK(AsyncSocket_TrackUnLock("ILibAsyncSocket_PostSelect",4,module);)
		sem_post(&(module->SendLock));
	}
}

/*! \fn ILibAsyncSocket_IsFree(ILibAsyncSocket_SocketModule socketModule)
	\brief Determines if an ILibAsyncSocket is in use
	\param socketModule The ILibAsyncSocket to query
	\returns 0 if in use, nonzero otherwise
*/
int ILibAsyncSocket_IsFree(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	return(module->internalSocket==~0?1:0);
}

/*! \fn ILibAsyncSocket_GetPendingBytesToSend(ILibAsyncSocket_SocketModule socketModule)
	\brief Returns the number of bytes that are pending to be sent
	\param socketModule The ILibAsyncSocket to query
	\returns Number of pending bytes
*/
unsigned int ILibAsyncSocket_GetPendingBytesToSend(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	return(module->PendingBytesToSend);
}

/*! \fn ILibAsyncSocket_GetTotalBytesSent(ILibAsyncSocket_SocketModule socketModule)
	\brief Returns the total number of bytes that have been sent, since the last reset
	\param socketModule The ILibAsyncSocket to query
	\returns Number of bytes sent
*/
unsigned int ILibAsyncSocket_GetTotalBytesSent(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
    return(module->TotalBytesSent);
}

/*! \fn ILibAsyncSocket_ResetTotalBytesSent(ILibAsyncSocket_SocketModule socketModule)
	\brief Resets the total bytes sent counter
	\param socketModule The ILibAsyncSocket to reset
*/
void ILibAsyncSocket_ResetTotalBytesSent(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	module->TotalBytesSent = 0;
}

/*! \fn ILibAsyncSocket_GetBuffer(ILibAsyncSocket_SocketModule socketModule, char **buffer, int *BeginPointer, int *EndPointer)
	\brief Returns the buffer associated with an ILibAsyncSocket
	\param socketModule The ILibAsyncSocket to obtain the buffer from
	\param[out] buffer The buffer
	\param[out] BeginPointer Stating offset of the buffer
	\param[out] EndPointer Length of buffer
*/
void ILibAsyncSocket_GetBuffer(ILibAsyncSocket_SocketModule socketModule, char **buffer, int *BeginPointer, int *EndPointer)
{
	struct ILibAsyncSocketModule* module = (struct ILibAsyncSocketModule*)socketModule;

	if(module==NULL){return;}
	*buffer = module->buffer;
	*BeginPointer = module->BeginPointer;
	*EndPointer = module->EndPointer;
}

//
// Sets the remote address field
//
// This is utilized by the ILibAsyncServerSocket module
// <param name="socketModule">The ILibAsyncSocket to modify</param>
// <param name="RemoteAddress">The remote interface</param>
void ILibAsyncSocket_SetRemoteAddress(ILibAsyncSocket_SocketModule socketModule,int RemoteAddress)
{
	struct ILibAsyncSocketModule* module = (struct ILibAsyncSocketModule*)socketModule;
	if(module!=NULL)
	{
		module->RemoteIPAddress = RemoteAddress;
	}
}

/*! \fn ILibAsyncSocket_UseThisSocket(ILibAsyncSocket_SocketModule socketModule,void* UseThisSocket,ILibAsyncSocket_OnInterrupt InterruptPtr,void *user)
	\brief Associates an actual socket with ILibAsyncSocket
	\par
	Instead of calling \a ConnectTo, you can call this method to associate with an already
	connected socket.
	\param socketModule The ILibAsyncSocket to associate
	\param UseThisSocket The socket to associate
	\param InterruptPtr Function Pointer that triggers when the TCP connection is interrupted
	\param user User object to associate with this session
*/
void ILibAsyncSocket_UseThisSocket(ILibAsyncSocket_SocketModule socketModule,void* UseThisSocket,ILibAsyncSocket_OnInterrupt InterruptPtr,void *user)
{
	#if defined(_WIN32_WCE) || defined(WIN32)
		SOCKET TheSocket = *((SOCKET*)UseThisSocket);
	#elif defined(_POSIX)
		int TheSocket = *((int*)UseThisSocket);
	#endif
	int flags;
	struct ILibAsyncSocketModule* module = (struct ILibAsyncSocketModule*)socketModule;

	if(module==NULL){return;}

	module->PendingBytesToSend = 0;
	module->TotalBytesSent = 0;
	module->internalSocket = TheSocket;
	module->OnInterrupt = InterruptPtr;
	module->user = user;
	module->FinConnect = 1;
	module->PAUSE = 0;

	//
	// If the buffer is too small/big, we need to realloc it to the minimum specified size
	//
	module->buffer = (char*)realloc(module->buffer,module->InitialSize);
	module->MallocSize = module->InitialSize;
	module->FinConnect = 1;
	module->BeginPointer = 0;
	module->EndPointer = 0;

	//
	// Make sure the socket is non-blocking, so we can play nice and share the thread
	//
	#if defined(_WIN32_WCE) || defined(WIN32)
		flags = 1;
		ioctlsocket(module->internalSocket,FIONBIO,&flags);
	#elif defined(_POSIX)
		flags = fcntl(module->internalSocket,F_GETFL,0);
		fcntl(module->internalSocket,F_SETFL,O_NONBLOCK|flags);
	#endif
}

/*! \fn ILibAsyncSocket_GetRemoteInterface(ILibAsyncSocket_SocketModule socketModule)
	\brief Returns the Remote Interface of a connected session
	\param socketModule The ILibAsyncSocket to query
	\returns The remote interface
*/
int ILibAsyncSocket_GetRemoteInterface(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	return(module->RemoteIPAddress);
}
/*! \fn ILibAsyncSocket_GetRemotePort(ILibAsyncSocket_SocketModule socketModule)
	\brief Returns the Port number of the origin of the last received packet
	\param socketModule The ILibAsyncSocket to query
	\returns The remote port
*/
unsigned short ILibAsyncSocket_GetRemotePort(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	return(module->RemotePort);
}

/*! \fn ILibAsyncSocket_GetLocalInterface(ILibAsyncSocket_SocketModule socketModule)
	\brief Returns the Local Interface of a connected session, in network order
	\param socketModule The ILibAsyncSocket to query
	\returns The local interface
*/
int ILibAsyncSocket_GetLocalInterface(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	struct sockaddr_in receivingAddress;
	int receivingAddressLength = sizeof(struct sockaddr_in);

	if(module->LocalIPAddress2!=0)
	{
		return(module->LocalIPAddress2);
	}
	else
	{
		getsockname(module->internalSocket,(struct sockaddr*)&receivingAddress,&receivingAddressLength);
		return(receivingAddress.sin_addr.s_addr);
	}
}
/*! \fn unsigned short ILibAsyncSocket_GetLocalPort(ILibAsyncSocket_SocketModule socketModule)
	\brief Returns the Local Port of a connected session, in host order
	\param socketModule The ILibAsyncSocket to query
	\returns The local port number
*/
unsigned short ILibAsyncSocket_GetLocalPort(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *module = (struct ILibAsyncSocketModule*)socketModule;
	struct sockaddr_in receivingAddress;
	int receivingAddressLength = sizeof(struct sockaddr_in);

	getsockname(module->internalSocket,(struct sockaddr*)&receivingAddress,&receivingAddressLength);
	return(ntohs(receivingAddress.sin_port));
}

/*! \fn ILibAsyncSocket_Resume(ILibAsyncSocket_SocketModule socketModule)
	\brief Resumes a paused session
	\par
	Sessions can be paused, such that further data is not read from the socket until resumed
	\param socketModule The ILibAsyncSocket to resume
*/
void ILibAsyncSocket_Resume(ILibAsyncSocket_SocketModule socketModule)
{
	struct ILibAsyncSocketModule *sm = (struct ILibAsyncSocketModule*)socketModule;
	if(sm!=NULL)
	{
		sm->PAUSE = -1;
		ILibForceUnBlockChain(sm->Chain);
	}

}
/*! \fn ILibAsyncSocket_GetSocket(ILibAsyncSocket_SocketModule module)
	\brief Obtain the underlying raw socket
	\param module The ILibAsyncSocket to query
	\returns The raw socket
*/
void* ILibAsyncSocket_GetSocket(ILibAsyncSocket_SocketModule module)
{
	struct ILibAsyncSocketModule *sm = (struct ILibAsyncSocketModule*)module;
	return(&(sm->internalSocket));
}
void ILibAsyncSocket_SetLocalInterface2(ILibAsyncSocket_SocketModule module, int localInterface2)
{
	struct ILibAsyncSocketModule *sm = (struct ILibAsyncSocketModule*)module;
	sm->LocalIPAddress2 = localInterface2;
}
