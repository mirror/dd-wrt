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
 * $Workfile: ILibWebClient.c
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


#ifdef SEMAPHORE_TRACKING
#define SEM_TRACK(x) x
void WebClient_TrackLock(const char* MethodName, int Occurance, void *data)
{
	char v[100];

	sprintf(v,"  LOCK[%s, %d] (%x)\r\n",MethodName,Occurance,data);
#ifdef WIN32
	OutputDebugString(v);
#else
	printf(v);
#endif
}
void WebClient_TrackUnLock(const char* MethodName, int Occurance, void *data)
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


#include "ILibParsers.h"
#include "ILibWebClient.h"
#include "ILibWebServer.h"
#include "ILibAsyncSocket.h"


#if defined(WIN32) && !defined(_WIN32_WCE)
	#include <crtdbg.h>
#endif

#ifdef ILibWebClient_SESSION_TRACKING
void ILibWebClient_SessionTrack(void *RequestToken, void *Session, char *msg)
{
#if defined(WIN32) || defined(_WIN32_WCE)
	char tempMsg[4096];
	sprintf(tempMsg,"%s >> Request: %x , Session: %x\r\n",msg,RequestToken,Session);
	OutputDebugString(tempMsg);
#else
	printf("%s >> Request: %x , Session: %x\r\n",msg,RequestToken,Session);
#endif
}
#define SESSION_TRACK(RequestToken,Session,msg) ILibWebClient_SessionTrack(RequestToken,Session,msg)
#else
	#define SESSION_TRACK(RequestToken,Session,msg)
#endif



//
// We keep a table of all the connections. This is the maximum number allowed to be
// idle. Since we have in the constructor a pool size, this feature may be depracted.
// ToDo: Look into depracating this
//
#define MAX_IDLE_SESSIONS 20


//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
//
// This means the module doesn't know yet if the server supports persistent connections
//
#define PIPELINE_UNKNOWN 0
//
// The server does indeed support persistent connections
//
#define PIPELINE_YES 1
//
// The server does not support persistent connections
//
#define PIPELINE_NO 2
//
// Chunk processing flags
//
#define STARTCHUNK 0
#define ENDCHUNK 1
#define DATACHUNK 2
#define FOOTERCHUNK 3
//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}


struct ILibWebClient_StreamedRequestBuffer
{
	char *buffer;
	enum ILibAsyncSocket_MemoryOwnership MemoryOwnership;
	int length;
	int done;
};

struct ILibWebClient_StreamedRequestState
{
	ILibWebClient_OnSendOK OnSendOK;
	void *BufferQueue;
	int done;
	int canceled;
	int doNotSendRightAway;
};

struct ILibWebClientManager
{
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);

	void **socks;
	int socksLength;

	void *DataTable;
	void *idleTable;
	void *backlogQueue;

	int MaxConnectionsToSameServer;

	void *timer;
	int idleCount;

	void *Chain;
	sem_t QLock;

	void *user;
};
//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
struct ILibWebClient_ChunkData
{
	int Flag;
	char *buffer;
	int offset;
	int mallocSize;

	int read_BeginPointer;
	int read_EndPointer;

	int bytesLeft;
};
//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
struct ILibWebClientDataObject
{
	int PipelineFlag;
	int ActivityCounter;
	struct sockaddr_in remote;
	struct ILibWebClientManager *Parent;

	int PendingConnectionIndex;

	int DeferDestruction;
	int CancelRequest;
	int FinHeader;
	int Chunked;
//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
	struct ILibWebClient_ChunkData *chunk;
	int ConnectionCloseSpecified;
//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
	int BytesLeft;
	int WaitForClose;
	int Closing;
	int Server;
	int DisconnectSent;

	int HeaderLength;
	int ExponentialBackoff;


	struct packetheader *header;
	struct sockaddr_in source;

	int InitialRequestAnswered;
	void* RequestQueue;
	void *SOCK;
	int LocalIP;
	int PAUSE;
	int IndexNumber;
};

struct ILibWebClient_PipelineRequestToken
{
	struct ILibWebClientDataObject *wcdo;
	void* timer;
};

struct ILibWebRequest
{
	char **Buffer;
	int *BufferLength;
	int *UserFree;
	int NumberOfBuffers;

	struct sockaddr_in remote;
	void *user1,*user2;

	struct ILibWebClient_PipelineRequestToken *requestToken;
	struct ILibWebClient_StreamedRequestState *streamedState;

	int IsHeadRequest;

	ILibWebClient_OnResponse OnResponse;
};

//
// Internal method used to free resources associated with a WebRequest
//
// <param name="wr">The WebRequest to free</param>
void ILibWebClient_DestroyWebRequest(struct ILibWebRequest *wr)
{
	int i;
	struct ILibWebClient_StreamedRequestBuffer *b;

	if(wr==NULL){return;}

	if(wr->streamedState!=NULL)
	{
		while(ILibQueue_IsEmpty(wr->streamedState->BufferQueue)==0)
		{
			b = (struct ILibWebClient_StreamedRequestBuffer*)ILibQueue_DeQueue(wr->streamedState->BufferQueue);
			if(b->MemoryOwnership==ILibAsyncSocket_MemoryOwnership_CHAIN)
			{
				free(b->buffer);
			}
			free(b);
		}
		ILibQueue_Destroy(wr->streamedState->BufferQueue);
		free(wr->streamedState);
		wr->streamedState=NULL;
	}

	for(i=0;i<wr->NumberOfBuffers;++i)
	{
		//
		// If we own any of the buffers, we need to free them
		//
		if(wr->UserFree[i]==0) {free(wr->Buffer[i]);}
	}

	//
	// Free the other resources
	//
	free(wr->Buffer);
	free(wr->BufferLength);
	free(wr->UserFree);
	if(wr->requestToken!=NULL) 
	{
		ILibLifeTime_Remove(wr->requestToken->timer,wr->requestToken->wcdo);
		free(wr->requestToken);
		wr->requestToken = NULL;
	}
	free(wr);
	wr = NULL;
}

//
// Internal method used to free resources associated with a WebClientDataObject
//
// <param name="token">The WebClientDataObject to free</param>
void ILibWebClient_DestroyWebClientDataObject(ILibWebClient_StateObject token)
{
	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)token;
	struct ILibWebRequest *wr;
	int zero=0;

	if(wcdo==NULL) {return;}


	if(wcdo->Closing<0) 
	{
		//
		// This connection is already in the process of closing somewhere, so we can just exit
		//
		return;
	}


	if(wcdo->SOCK!=NULL && ILibAsyncSocket_IsFree(wcdo->SOCK)==0)
	{
		//
		// This connection needs to be disconnected first
		//
		wcdo->Closing = -1;
		ILibAsyncSocket_Disconnect(wcdo->SOCK);
	}

		if(wcdo->header!=NULL)
		{
			//
			// The header needs to be freed
			//
			ILibDestructPacket(wcdo->header);
			wcdo->header = NULL;
		}
//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
		if(wcdo->chunk!=NULL)
		{
			//
			// The resources associated with the Chunk Processing needs to be freed
			//
			if(wcdo->chunk->buffer!=NULL) {free(wcdo->chunk->buffer);}
			free(wcdo->chunk);
			wcdo->chunk = NULL;
		}
//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
		//
		// Iterate through all the pending requests
		//
		wr = ILibQueue_DeQueue(wcdo->RequestQueue);
		while(wr!=NULL)
		{
			if(wcdo->Server==0 && wr->OnResponse!=NULL)
			{		
				//
				// If this is a client request, then we need to signal
				// that this request is being aborted
				//
				wr->OnResponse(
						wcdo,
						WEBCLIENT_DESTROYED,
						NULL,
						NULL,
						NULL,
						0,
						-1,
						wr->user1,
						wr->user2,
						&zero);		
			}
			ILibWebClient_DestroyWebRequest(wr);
			wr = ILibQueue_DeQueue(wcdo->RequestQueue);
		}

	ILibQueue_Destroy(wcdo->RequestQueue);
	free(wcdo);
}

//
// Internal method to free resources associated with an ILibWebClient object
//
// <param name="object">The ILibWebClient to free</param>
void ILibDestroyWebClient(void *object)
{
	struct ILibWebClientManager *manager = (struct ILibWebClientManager*)object;
	void *en;
	void *wcdo;
	char *key;
	int keyLength;

	//
	// Iterate through all the WebClientDataObjects
	//
	en = ILibHashTree_GetEnumerator(manager->DataTable);
	while(ILibHashTree_MoveNext(en)==0)
	{
		//
		// Free the WebClientDataObject
		//
		ILibHashTree_GetValue(en,&key,&keyLength,&wcdo);
		ILibWebClient_DestroyWebClientDataObject(wcdo);
	}
	ILibHashTree_DestroyEnumerator(en);
	
	//
	// Free all the other associated resources
	//
	ILibQueue_Destroy(manager->backlogQueue);
	ILibDestroyHashTree(manager->idleTable);
	ILibDestroyHashTree(manager->DataTable);
	sem_destroy(&(manager->QLock));
	free(manager->socks);
}

void ILibWebClient_TimerInterruptSink(void *object)
{
}
void ILibWebClient_ResetWCDO(struct ILibWebClientDataObject *wcdo)
{
	ILibWebClient_RequestToken rt = NULL;
	if(wcdo==NULL) {return;}
	rt = ILibWebClient_GetRequestToken_FromStateObject(wcdo);	
	if( NULL != rt) 
	{
		struct ILibWebClient_PipelineRequestToken * plrt = ( struct ILibWebClient_PipelineRequestToken * ) rt;
		//check the cancel request in the timer list
		
		if( NULL != plrt->timer )
		{
			ILibLifeTime_Remove(plrt->timer , plrt );
		}


	}
	wcdo->PAUSE=0;
	wcdo->CancelRequest=0;
	wcdo->Chunked = 0;
	wcdo->FinHeader = 0;
	wcdo->WaitForClose = 0;
	wcdo->InitialRequestAnswered = 1;
	wcdo->DisconnectSent=0;
	wcdo->PendingConnectionIndex=-1;
//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
	wcdo->ConnectionCloseSpecified=0;
//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}

	if(wcdo->chunk!=NULL)
	{
		if(wcdo->chunk->buffer!=NULL) {free(wcdo->chunk->buffer);}
		free(wcdo->chunk);
		wcdo->chunk = NULL;
	}
	if(wcdo->header!=NULL)
	{
		ILibDestructPacket(wcdo->header);
		wcdo->header = NULL;
	}
}
//
// Internal method dispatched by the LifeTimeMonitor
//
//
// This timed callback is used to close idle sockets. A socket is considered idle
// if after a request is answered, another request isn't received 
// within the time specified by HTTP_SESSION_IDLE_TIMEOUT
//
// <param name="object">The WebClientDataObject</param>
void ILibWebClient_TimerSink(void *object)
{
	void *enumerator;
	char IPV4Address[25];
	int IPV4AddressLength;
	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)object;
	struct ILibWebClientDataObject *wcdo2;

	char *key;
	int keyLength;
	void *data;

	void *DisconnectSocket = NULL;
	SEM_TRACK(WebClient_TrackLock("ILibWebClient_TimerSink",1,wcdo->Parent);)
	sem_wait(&(wcdo->Parent->QLock));
	if(ILibQueue_IsEmpty(wcdo->RequestQueue)!=0)
	{
		//
		// This connection is idle, becuase there are no pending requests
		//
		if(wcdo->SOCK!=NULL && ILibAsyncSocket_IsFree(wcdo->SOCK)==0)
		{
			//
			// We need to close this socket
			//
			wcdo->Closing = 1;
			DisconnectSocket = wcdo->SOCK;
		}
		if(wcdo->Parent->idleCount>MAX_IDLE_SESSIONS)
		{
			//
			// We need to remove an entry from th idleTable, if there are too
			// many entries in it
			//
			--wcdo->Parent->idleCount;
			enumerator = ILibHashTree_GetEnumerator(wcdo->Parent->idleTable);
			ILibHashTree_MoveNext(enumerator);
			ILibHashTree_GetValue(enumerator,&key,&keyLength,&data);
			ILibHashTree_DestroyEnumerator(enumerator);
			wcdo2 = ILibGetEntry(wcdo->Parent->DataTable,key,keyLength);
			ILibDeleteEntry(wcdo->Parent->DataTable,key,keyLength);
			ILibDeleteEntry(wcdo->Parent->idleTable,key,keyLength);
			SEM_TRACK(WebClient_TrackUnLock("ILibWebClient_TimerSink",2,wcdo->Parent);)
			sem_post(&(wcdo->Parent->QLock));
			ILibWebClient_DestroyWebClientDataObject(wcdo2);
			return;
		}
		else
		{
			//
			// Add this DataObject into the idleTable for use later
			//
			IPV4AddressLength = sprintf(IPV4Address,"%s:%d:%d",
				inet_ntoa(wcdo->remote.sin_addr),
				ntohs(wcdo->remote.sin_port),
				wcdo->IndexNumber);
			MEMCHECK(assert(IPV4AddressLength<=21);)

			ILibAddEntry(wcdo->Parent->idleTable,IPV4Address,IPV4AddressLength,wcdo);
			++wcdo->Parent->idleCount;
			wcdo->SOCK = NULL;
			wcdo->DisconnectSent=0;
		}
	}
	SEM_TRACK(WebClient_TrackUnLock("ILibWebClient_TimerSink",3,wcdo->Parent);)
	sem_post(&(wcdo->Parent->QLock));
	//
	// Let the user know, the socket has been disconnected
	//
	if(DisconnectSocket!=NULL)
	{
		ILibAsyncSocket_Disconnect(DisconnectSocket);
	}

}
//
// Internal method called by ILibWebServer, when a response was completed
//
// <param name="_wcdo">The associated WebClientDataObject</param>
void ILibWebClient_FinishedResponse_Server(ILibWebClient_StateObject _wcdo)
{
	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)_wcdo;

	if(wcdo==NULL) {return;}

//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
	if(wcdo->chunk!=NULL)
	{
		//
		// Free any resources associated with the Chunk Processor
		//
		free(wcdo->chunk->buffer);
		free(wcdo->chunk);
		wcdo->chunk = NULL;
	}
//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
	if(wcdo->header!=NULL)
	{
		//
		// Free the resources associated with the header
		//
		ILibDestructPacket(wcdo->header);
		wcdo->header = NULL;
	}
	//
	// Reset all the flags
	//
	ILibWebClient_ResetWCDO(wcdo);
}
//
// Internal method called when a WebClient has finished processing a Request/Response
//
// <param name="socketModule">The WebClient</param>
// <param name="wcdo">The associated WebClientDataObject</param>
void ILibWebClient_FinishedResponse(ILibAsyncSocket_SocketModule socketModule, struct ILibWebClientDataObject *wcdo)
{
	struct ILibWebRequest *wr;
	int i;
	if(wcdo==NULL) {return;}


	//
	// Only continue if this is a client calling this
	//
	if(wcdo->Server!=0) {return;}

	//
	// The current request was cancelled, so it can't really be finished, so we
	// need to skip this, because otherwise we will end up duplicating some calls.
	//
	if(wcdo->CancelRequest!=0) {return;}

	if(wcdo->header!=NULL && wcdo->header->StatusCode>=100 && wcdo->header->StatusCode<200)
	{
		//
		// This was an informational packet, and so it did not conclude this session
		//
		ILibWebClient_ResetWCDO(wcdo);
		wcdo->InitialRequestAnswered = 0;
		return;
	}

	//
	// If this socket isn't connected, it's because it was previously closed, 
	// so this finished response isn't valid anymore
	//
	if(wcdo->SOCK==NULL|| ILibAsyncSocket_IsFree(wcdo->SOCK))
	{
		SEM_TRACK(WebClient_TrackLock("ILibWebClient_FinishedResponse",1,wcdo->Parent);)
		sem_wait(&(wcdo->Parent->QLock));
		ILibWebClient_DestroyWebRequest(ILibQueue_DeQueue(wcdo->RequestQueue));
		SEM_TRACK(WebClient_TrackUnLock("ILibWebClient_FinishedResponse",2,wcdo->Parent);)
		sem_post(&(wcdo->Parent->QLock));
		return;
	}

//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
	if(wcdo->chunk!=NULL)
	{
		//
		// Free any resources associated with the Chunk Processor
		//
		free(wcdo->chunk->buffer);
		free(wcdo->chunk);
		wcdo->chunk = NULL;
	}
//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
	if(wcdo->header!=NULL)
	{
		//
		// Free any resources associated with the header
		//
		ILibDestructPacket(wcdo->header);
		wcdo->header = NULL;
	}

	//
	// Reset the flags
	//
	ILibWebClient_ResetWCDO(wcdo);

	SEM_TRACK(WebClient_TrackLock("ILibWebClient_FinishedResponse",1,wcdo->Parent);)
	sem_wait(&(wcdo->Parent->QLock));
	wr = ILibQueue_DeQueue(wcdo->RequestQueue);
	if(wr!=NULL)
	{
		//
		// Only execute this logic, if there was a pending request. If there wasn't one, that means
		// that this session was closed the last time the app as called with data, making this next step unnecessary.
		//
		ILibWebClient_DestroyWebRequest(wr);
		wr = ILibQueue_PeekQueue(wcdo->RequestQueue);
		if(wr==NULL)
		{
			//
			// Since the request queue is empty, that means this connection is now idle.
			// Set a timed callback, so we can free this resource if neccessary
			//
			if(ILibIsChainBeingDestroyed(wcdo->Parent->Chain)==0)
			{
				ILibLifeTime_Add(wcdo->Parent->timer,wcdo,HTTP_SESSION_IDLE_TIMEOUT,&ILibWebClient_TimerSink,&ILibWebClient_TimerInterruptSink);		
			}
		}
	}
	SEM_TRACK(WebClient_TrackUnLock("ILibWebClient_FinishedResponse",2,wcdo->Parent);)
	sem_post(&(wcdo->Parent->QLock));
//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
	if(wr!=NULL)
	{
		//
		// There are still pending requests in the queue, so try to send them
		//
		if(wcdo->PipelineFlag!=PIPELINE_NO)
		{
			//
			// If the connection is still open, and we didn't flag this as not supporting
			// persistent connections, than obviously it is supported
			//
			wcdo->PipelineFlag = PIPELINE_YES;

			for(i=0;i<wr->NumberOfBuffers;++i)
			{
				//
				// Try to send the request
				//
				ILibAsyncSocket_Send(wcdo->SOCK,wr->Buffer[i],wr->BufferLength[i],ILibAsyncSocket_MemoryOwnership_STATIC);
			}
		}
	}
	if(wcdo->PipelineFlag==PIPELINE_NO)
	{
//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
		/*	Pipelining is not supported, so we should just close the socket, instead
			of waiting for the other guy to close it, because if they forget to, it will
			screw us over if there are pending requests */

		//
		// It should also be noted, that when this closes, the module will realize there are
		// pending requests, in which case it will open a new connection for the requests.
		//
		if(ILibIsChainBeingDestroyed(wcdo->Parent->Chain)==0)
		{
			//
			// Only do this if the chain is still alive, otherwise things will get screwed
			// up, because modules may not be ready.
			//
			ILibAsyncSocket_Disconnect(wcdo->SOCK);
		}
//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
	}
//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
}

//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
//
// Internal method called to decode chunked transfers
//
// <param name="socketModule">The ILibWebClient token</param>
// <param name="wcdo">The associated WebClientDataObject</param>
// <param name="buffer">The receive buffer</param>
// <param name="p_beginPointer">The buffer start pointer</param>
// <param name="endPointer">The length of the buffer</param>
void ILibWebClient_ProcessChunk(ILibAsyncSocket_SocketModule socketModule, struct ILibWebClientDataObject *wcdo, char *buffer, int *p_beginPointer, int endPointer)
{
	char *hex;
	int i;
	struct parser_result *pr;
	struct ILibWebRequest *wr;
	int bp;
	int resize;
	int passthru=0;

	if(wcdo==NULL) {return;}

	//
	// ToDo: Document what a Parent is
	//
	if(wcdo->Parent!=NULL) 
	{
		SEM_TRACK(WebClient_TrackLock("ILibWebClient_ProcessChunk",1,wcdo->Parent);)
		sem_wait(&(wcdo->Parent->QLock));
	}
	wr = ILibQueue_PeekQueue(wcdo->RequestQueue);
	if(wcdo->Parent!=NULL) 
	{
		SEM_TRACK(WebClient_TrackUnLock("ILibWebClient_ProcessChunk",2,wcdo->Parent);)
		sem_post(&(wcdo->Parent->QLock));
	}

	if(wcdo->chunk==NULL)
	{
		//
		// Create a state object for the Chunk Processor
		//
		wcdo->chunk = (struct ILibWebClient_ChunkData*)malloc(sizeof(struct ILibWebClient_ChunkData));
		memset(wcdo->chunk,0,sizeof(struct ILibWebClient_ChunkData));

		wcdo->chunk->buffer = (char*)malloc(INITIAL_BUFFER_SIZE);
		wcdo->chunk->mallocSize = INITIAL_BUFFER_SIZE;
	}

	switch(wcdo->chunk->Flag)
	{
		//
		// Based on the Chunk Flag, we can figure out how to parse this thing
		//
		case STARTCHUNK:
			// Reading Chunk Header
			if(endPointer<3){return;}
			for(i=2;i<endPointer;++i)
			{
				if(buffer[i-2]=='\r' && buffer[i-1]=='\n')
				{
					//
					// The chunk header is terminated with a CRLF. The part before the ';'
					// is the hex number representing the length of the chunk
					//
					pr = ILibParseString(buffer,0,i-2,";",1);
					pr->FirstResult->data[pr->FirstResult->datalength] = '\0';
					wcdo->chunk->bytesLeft  = (int)strtol(pr->FirstResult->data,&hex,16);
					*p_beginPointer = i;
					wcdo->chunk->Flag=wcdo->chunk->bytesLeft==0?FOOTERCHUNK:DATACHUNK;
					ILibDestructParserResults(pr);
					break;
				}
			}
			break;
		case ENDCHUNK:
			if(endPointer>=2)
			{
				//
				// There is more chunks to come
				//
				*p_beginPointer = 2;
				wcdo->chunk->Flag = STARTCHUNK;
			}
			break;
		case DATACHUNK:
			if(endPointer>=wcdo->chunk->bytesLeft)
			{
				//
				// Only consume what we need
				//
				i = wcdo->chunk->bytesLeft;
			}
			else
			{
				//
				// Consume all of the data
				//
				i=endPointer;
			}

			if(wcdo->chunk->read_BeginPointer!= wcdo->chunk->read_EndPointer)
			{
				//
				// The user didn't consume all of the data, so we need to copy the data
				// into this buffer, before passing it to the user
				//
				// But before we do that, we need to check to see if our buffer is big enough
				//
				if(wcdo->chunk->offset+endPointer>wcdo->chunk->mallocSize)
				{
					//
					// The buffer is too small, we need to make it bigger
					// ToDo: Add code to enforce a max buffer size if specified
					// ToDo: Does the above layer need to know when buffers were realloced?
					//
					resize = wcdo->chunk->offset+endPointer-wcdo->chunk->mallocSize>INITIAL_BUFFER_SIZE?wcdo->chunk->offset+endPointer-wcdo->chunk->mallocSize:INITIAL_BUFFER_SIZE;
					wcdo->chunk->buffer = (char*)realloc(wcdo->chunk->buffer,wcdo->chunk->mallocSize+resize);
					wcdo->chunk->mallocSize += resize;
				}
				//
				// Write the decoded chunk blob into the buffer
				//
				memcpy(wcdo->chunk->buffer+wcdo->chunk->offset,buffer,i);
				MEMCHECK(assert(wcdo->chunk->offset+i<=wcdo->chunk->mallocSize);)
				//
				// Adjust our counters
				//
				wcdo->chunk->offset+=i;
			}
			else
			{
				//
				// We aren't growing this buffer for the user, so we can start out
				// by trying to just pass the underlying buffer, instead of copying it
				//
				passthru=1;
			}

			bp=0;
			if(wr!=NULL && wr->OnResponse!=NULL)
			{
				//
				// Let the user know we got some data
				//

				if(passthru)
				{
					wr->OnResponse(
						wcdo,
						0,
						wcdo->header,
						buffer,
						&bp,
						i,
						0,
						wr->user1,
						wr->user2,
						&(wcdo->PAUSE));	

					if(bp==0)
					{
						//
						// User didn't consume anything, so probably
						// wants to grow the buffer
						//
						wcdo->chunk->read_BeginPointer = 0;
						wcdo->chunk->read_EndPointer = i;

						//
						// We need to copy this data, so the stack can continue processing
						//
						if(wcdo->chunk->offset+endPointer>wcdo->chunk->mallocSize)
						{
							//
							// The buffer is too small, we need to make it bigger
							// ToDo: Add code to enforce a max buffer size if specified
							// ToDo: Does the above layer need to know when buffers were realloced?
							//
							resize = wcdo->chunk->offset+endPointer-wcdo->chunk->mallocSize>INITIAL_BUFFER_SIZE?wcdo->chunk->offset+endPointer-wcdo->chunk->mallocSize:INITIAL_BUFFER_SIZE;
							wcdo->chunk->buffer = (char*)realloc(wcdo->chunk->buffer,wcdo->chunk->mallocSize+resize);
							wcdo->chunk->mallocSize += resize;
						}
						memcpy(wcdo->chunk->buffer+wcdo->chunk->offset,buffer,i);
						wcdo->chunk->offset+=i;
						bp = i;
					}
					else 
					{
						//
						// User consumed something at least. We don't need to
						// copy anything just yet, because maybe they'll consume
						// everything on the next pass
						//
						if(bp!=i)
						{
							wcdo->chunk->read_BeginPointer = bp;
							wcdo->chunk->read_EndPointer = i;
						}
						else
						{
							//
							// Everything was consumed, so we can
							// set things up, so on the next pass we just
							// allow the buffer to passthru
							//
							wcdo->chunk->read_BeginPointer = 0;
							wcdo->chunk->read_EndPointer = 0;
						}
					}
				}
				else
				{
					wr->OnResponse(
						wcdo,
						0,
						wcdo->header,
						wcdo->chunk->buffer,
						&bp,
						wcdo->chunk->offset,
						0,
						wr->user1,
						wr->user2,
						&(wcdo->PAUSE));	
					if(bp==wcdo->chunk->offset)
					{
						//
						// Everything was consumed, so we can
						// set things up, so on the next pass we just
						// allow the buffer to passthru
						//
						wcdo->chunk->read_BeginPointer = 0;
						wcdo->chunk->read_EndPointer = 0;
					}
					else
					{
						//
						// Not everything was consumed
						//
						wcdo->chunk->read_BeginPointer = bp;
						wcdo->chunk->read_EndPointer = wcdo->chunk->offset;
					}
					bp=i;
				}
			}
			
			
			wcdo->chunk->bytesLeft-=bp;
			*p_beginPointer = bp;
			if(wcdo->chunk->bytesLeft==0)
			{
				wcdo->chunk->Flag = ENDCHUNK;
			}
			break;
		case FOOTERCHUNK:
			if(endPointer>=2)
			{
				for(i=2;i<=endPointer;++i)
				{
					if(buffer[i-2]=='\r' && buffer[i-1]=='\n')
					{
						//
						// An empty line means the chunk is finished
						//
						if(i==2)
						{
							// FINISHED
							if(wr != NULL && wr->OnResponse!=NULL)
							{
								bp = wcdo->chunk->read_BeginPointer;
								wr->OnResponse(
									wcdo,
									0,
									wcdo->header,
									wcdo->chunk->buffer,
									&bp,
									wcdo->chunk->read_EndPointer,
									-1,
									wr->user1,
									wr->user2,
									&(wcdo->PAUSE));			
							}
							if(ILibAsyncSocket_IsFree(socketModule)==0)
							{
								//
								// Free the resources associated with this chunk
								//
								if(wcdo->chunk!=NULL)
								{
									if(wcdo->chunk->buffer!=NULL) {free(wcdo->chunk->buffer);}
									free(wcdo->chunk);
									wcdo->chunk = NULL;
								}
								ILibWebClient_FinishedResponse(wcdo->SOCK,wcdo);
							}
						}
						else
						{
							//
							// ToDo: This is where to add code to add support trailers
							//
						}
						*p_beginPointer = i;
						break;
					}
				}
			}
			break;
	}
}
//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}



//
// Internal method dispatched by the OnData event of the underlying ILibAsyncSocket
//
// <param name="socketModule">The underlying ILibAsyncSocket</param>
// <param name="buffer">The receive buffer</param>
// <param name="p_beginPointer">start pointer in the buffer</param>
// <param name="endPointer">The length of the buffer</param>
// <param name="InterruptPtr">Function Pointer that triggers when a connection is interrupted</param>
// <param name="user">User data that can be set/received</param>
// <param name="PAUSE">Flag to tell the underlying socket to pause reading data</param>
void ILibWebClient_OnData(ILibAsyncSocket_SocketModule  socketModule,char* buffer,int *p_beginPointer, int endPointer,void (**InterruptPtr)(void *socketModule, void *user), void **user, int *PAUSE)
{
	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)(*user);
	struct ILibWebRequest *wr;
	struct packetheader *tph;
	struct packetheader_field_node *phfn;
	int i=0;
	int zero = 0;
	int Fini;

	char *tempIP;
	char *tempPath;
	unsigned short tempPort;

	if(wcdo==NULL) {return;}

	if(wcdo->Server==0)
	{
		SEM_TRACK(WebClient_TrackLock("ILibWebClient_OnData",1,wcdo->Parent);)
		sem_wait(&(wcdo->Parent->QLock));
	}
	wr = (struct ILibWebRequest*)ILibQueue_PeekQueue(wcdo->RequestQueue);
	if(wcdo->Server==0)
	{
		SEM_TRACK(WebClient_TrackUnLock("ILibWebClient_OnData",2,wcdo->Parent);)
		sem_post(&(wcdo->Parent->QLock));
	}
	if(wr==NULL)
	{
		//
		// There are no pending requests, so we have no idea what we are supposed to do with
		// this data, other than just recycling the receive buffer, so we don't leak memory.
		// If this code executes, this usually signifies a processing error of some sort. Most
		// of the time, it means the remote endpoint is sending invalid packets.
		//
		*p_beginPointer = endPointer;
		return;
	}
	if(wcdo->FinHeader==0)
	{
		//Still Reading Headers
		if(endPointer - (*p_beginPointer)>=4)
		{
			while(i <= (endPointer - (*p_beginPointer))-4)
			{
				if(buffer[*p_beginPointer+i]=='\r' &&
					buffer[*p_beginPointer+i+1]=='\n' &&
					buffer[*p_beginPointer+i+2]=='\r' &&
					buffer[*p_beginPointer+i+3]=='\n')
				{
					//
					// Headers are delineated with a CRLF, and terminated with an empty line
					//
					memset(&(wcdo->source),0,sizeof(struct sockaddr_in));
					wcdo->source.sin_family = AF_INET;
					wcdo->source.sin_addr.s_addr = ILibAsyncSocket_GetRemoteInterface(socketModule);
					wcdo->source.sin_port = htons(ILibAsyncSocket_GetRemotePort(socketModule));

					wcdo->HeaderLength = i+4;
					wcdo->WaitForClose=1;
					wcdo->BytesLeft=-1;
					wcdo->FinHeader=1;
					wcdo->header = ILibParsePacketHeader(buffer,*p_beginPointer,endPointer-(*p_beginPointer));
					if(wcdo->header!=NULL)
					{
						//
						// Check to see if this has an absolute path. Might be able to convert it
						// for easier processing
						//
						if(wcdo->Server!=0 && wcdo->header->DirectiveObjLength>7 && memcmp(wcdo->header->DirectiveObj,"http://",7)==0)
						{
							wcdo->header->DirectiveObj[wcdo->header->DirectiveObjLength]=0;
							ILibParseUri(wcdo->header->DirectiveObj,&tempIP,&tempPort,&tempPath);
							if(strcmp(tempIP,ILibIPAddress_ToDottedQuad(ILibWebServer_GetLocalInterface(wr->user2)))==0 &&
								tempPort==ILibWebServer_GetPortNumber(wr->user1))
							{
								//
								// We can simplify this, becuase it's an absolute path pointing to this server
								//
								memcpy(wcdo->header->DirectiveObj,tempPath,(int)strlen(tempPath));
								wcdo->header->DirectiveObjLength = (int)strlen(tempPath);
								wcdo->header->DirectiveObj[wcdo->header->DirectiveObjLength]=0;
							}
							free(tempIP);
							free(tempPath);
						}
						
						wcdo->header->Source = &(wcdo->source);
						wcdo->header->ReceivingAddress = wcdo->LocalIP;
						//
						// Introspect Request, to see what to do next
						//
						phfn = wcdo->header->FirstField;
						while(phfn!=NULL)
						{
//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
							if(phfn->FieldLength==17 && strncasecmp(phfn->Field,"transfer-encoding",17)==0)
							{
								if(phfn->FieldDataLength==7 && strncasecmp(phfn->FieldData,"chunked",7)==0)
								{
									//
									// This packet was chunk encoded
									//
									wcdo->WaitForClose=0;
									wcdo->Chunked = 1;
								}
							}
							if(phfn->FieldLength==10 && strncasecmp(phfn->Field,"connection",10)==0)
							{
								if(phfn->FieldDataLength==5 && strncasecmp(phfn->FieldData,"close",5)==0)
								{
									//
									// This packet specified connection: close token
									//
									wcdo->ConnectionCloseSpecified=1;
								}
							}
//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
							if(phfn->FieldLength==14 && strncasecmp(phfn->Field,"content-length",14)==0)
							{
								//
								// This packet has a Content-Length
								//
								wcdo->WaitForClose=0;
								phfn->FieldData[phfn->FieldDataLength] = '\0';
								wcdo->BytesLeft = atoi(phfn->FieldData);
							}
							phfn = phfn->NextField;
						}
//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
						if(atof(wcdo->header->Version)>1.0)
						{
							//
							// This is HTTP/1.1 or better, so we need to check to see
							// 1. Transfer-Encoding is not chunked
							// 2. Connection: close is NOT specified
							// 3. Content-Length is not specified
							// because than the packet MUST NOT have a body.
							//
							if(wcdo->Chunked==0 && wcdo->ConnectionCloseSpecified==0 && wcdo->WaitForClose!=0)
							{
								wcdo->WaitForClose=0;
								wcdo->BytesLeft=0;
							}
						}
						if(wcdo->header->StatusCode>=100 && wcdo->header->StatusCode<=199)
						{
							if(wr->streamedState==NULL)
							{
								//
								// HTTP Informational, ignore packet
								// because this request was not a streaming request
								// so we know there is no reason to bother the upper layers
								// with this piece of information
								//
								wcdo->FinHeader=0;
								*p_beginPointer += wcdo->HeaderLength;
								ILibDestructPacket(wcdo->header);
								wcdo->header = NULL;
								break;
							}
						}
						else if(wcdo->header->StatusCode==204 || wcdo->header->StatusCode==304)
						{
							// No Body
							wcdo->WaitForClose=0;
							wcdo->BytesLeft = 0;
						}
//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
						if(wcdo->Server!=0 && wcdo->BytesLeft==-1 && wcdo->Chunked==0)
						{
							//
							// This request has no body
							//
							wcdo->BytesLeft=0;	
						}
						if(wcdo->RequestQueue!=NULL && ILibQueue_PeekQueue(wcdo->RequestQueue)!=NULL)
						{
							if(((struct ILibWebRequest*)ILibQueue_PeekQueue(wcdo->RequestQueue))->IsHeadRequest!=0)
							{
								wcdo->BytesLeft = 0;
								wcdo->WaitForClose = 0;
								wcdo->Chunked = 0;
							}
						}
						if(wcdo->BytesLeft==0)
						{
							//
							// We already have the complete Response Packet
							// 

							//
							// We need to set this, because we want to prevent the
							// layer above us from calling disconnect, and throwing this
							// object away prematurely
							//
							wcdo->DeferDestruction=1;

							if(wr->OnResponse!=NULL)
							{
								wr->OnResponse(
									wcdo,
									0,
									wcdo->header,
									NULL,
									&zero,
									0,
									-1,
									wr->user1,
									wr->user2,
									&(wcdo->PAUSE));
							}
							*p_beginPointer = *p_beginPointer + i + 4;
							wcdo->DeferDestruction=0;
							ILibWebClient_FinishedResponse(socketModule,wcdo);
						}
						else
						{
							//
							// There is still data we need to read. Lets see if any of the 
							// body arrived yet
							//
							if(wcdo->Chunked==0)
							{
								//
								// This isn't chunked, so we can process normally
								// 
								if(wcdo->BytesLeft!=-1 && (endPointer-(*p_beginPointer)) - (i+4) >= wcdo->BytesLeft)
								{
									//
									// We have the entire body now, so we have the entire packet
									// 

									//
									// We need to set this, because we want to prevent the
									// layer above us from calling disconnect, and throwing this
									// object away prematurely
									//
									wcdo->DeferDestruction=1;
									if(wr->OnResponse!=NULL)
									{
										wr->OnResponse(
											wcdo,
											0,
											wcdo->header,
											buffer+i+4,
											&zero,
											wcdo->BytesLeft,
											-1,
											wr->user1,
											wr->user2,
											&(wcdo->PAUSE));
									}
									*p_beginPointer = *p_beginPointer + i + 4 + (zero==0?wcdo->BytesLeft:zero);
									wcdo->DeferDestruction = 0;
									ILibWebClient_FinishedResponse(socketModule,wcdo);
								}
								else
								{
									//
									// We read some of the body, but not all of it yet
									//
									if(wr->OnResponse!=NULL)
									{
										wr->OnResponse(
											wcdo,
											0,
											wcdo->header,
											buffer+i+4,
											&zero,
											(endPointer - (*p_beginPointer) - (i+4)),
											0,
											wr->user1,
											wr->user2,
											&(wcdo->PAUSE));
									}
									wcdo->HeaderLength = 0;
									*p_beginPointer = i+4+zero;
									wcdo->BytesLeft -= zero;
									//
									// We are consuming the header portion of the buffer
									// so we need to copy it out, so we can recycle the buffer
									// for the body
									//
									tph = ILibClonePacket(wcdo->header);
									ILibDestructPacket(wcdo->header);
									wcdo->header = tph;
								}
							}
//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
							else
							{
								//
								// Before we run this through our chunk processor, let's
								// make the callback, if the Chunk state is not set. 
								// (If it's not set, it means we haven't looked at the body yet)
								// We need to do this, because the header could contain information
								// such as an Expect: 100-Continue, that the above layer will need
								// to look at and send a 100 Continue, before the other end will
								// send the rest of the request.
								//
								// There is nothing to lose anyways by calling this. Worse comes to 
								// worse, they'll just ignore it, and life will go on.
								//
								if(wcdo->chunk==NULL && wr->OnResponse!=NULL)
								{
									wr->OnResponse(
										wcdo,
										0,
										wcdo->header,
										buffer+i+4,
										&zero,
										0,
										0,
										wr->user1,
										wr->user2,
										&(wcdo->PAUSE));
									if(ILibAsyncSocket_IsFree(socketModule)!=0)
									{
										//
										// The user closed the socket, so just return
										//
										return;
									}
									else
									{
										if(wcdo->FinHeader==0)
										{
											//
											// The user sent a response already, so advance the
											// beginPointer and return.
											//
											// If the user sent a response, and it wasn't in relation to a 100 Continue, then the 
											// user is in ERROR, because the done flag is not set here, which means that
											// the entire request did not get received yet. Simply answering the request
											// without receiving the entire request, without closing the socket is in
											// VIOLATION of the http specification. 
											//
											*p_beginPointer = (endPointer - (*p_beginPointer) - (i+4));
											return;
										}
									}
								}
								
								
								//
								// This packet is chunk encoded, so we need to run it
								// through our Chunk Processor
								//
								ILibWebClient_ProcessChunk(socketModule,wcdo,buffer+i+4,&zero,(endPointer - (*p_beginPointer) - (i+4)));
								*p_beginPointer = i+4+zero;
								tph = ILibClonePacket(wcdo->header);
								ILibDestructPacket(wcdo->header);
								wcdo->header = tph;
							}
//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
						}
					}
					else
					{
						//
						// ToDo: There was an error parsing the headers. What should we do about it?
						// Right now, we don't care
						//
					}
					break;
				}

				++i;
			}
		}
	}
	else
	{
		//
		// We already processed the headers, so we are only expecting the body now
		//
		if(wcdo->Chunked==0)
		{
			//
			// This isn't chunk encoded
			//
			if(wcdo->WaitForClose==0)
			{
				//
				// This is to determine if we have everything
				//
				Fini = ((endPointer - (*p_beginPointer))>=wcdo->BytesLeft)?-1:0;
			}
			else
			{
				//
				// We need to read until the socket closes
				//
				Fini = 0;
			}

			if(wr->OnResponse!=NULL)
			{
				wr->OnResponse(
					wcdo,
					0,
					wcdo->header,
					buffer,
					p_beginPointer,
					wcdo->WaitForClose==0?(((endPointer - (*p_beginPointer))>=wcdo->BytesLeft)?wcdo->BytesLeft:(endPointer - (*p_beginPointer))):(endPointer-(*p_beginPointer)),
					Fini,
					wr->user1,
					wr->user2,
					&(wcdo->PAUSE));
			}
			if(ILibAsyncSocket_IsFree(socketModule)==0)
			{
				if(wcdo->WaitForClose==0)
				{
					//
					// Decrement our counters
					//
					wcdo->BytesLeft -= *p_beginPointer;
					if(Fini!=0)
					{
						//
						// We finished processing this, so signal it
						//
						*p_beginPointer = *p_beginPointer + wcdo->BytesLeft;
						ILibWebClient_FinishedResponse(socketModule,wcdo);
					}
				}
			}
		}
//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
		else
		{
			//
			// This is chunk encoded, so run it through our Chunk Processor
			//
			ILibWebClient_ProcessChunk(socketModule,wcdo,buffer,p_beginPointer,endPointer);
		}
//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
	}
	if(ILibAsyncSocket_IsFree(socketModule)==0)
	{
		//
		// If the user said to pause this connection, do so
		//
		*PAUSE = wcdo->PAUSE;
	}
}

//
// Internal method dispatched by the LifeTimeMonitor, to retry refused connections
//
//
// This module does an exponential backoff, when retrying connections. The number
// of retries is determined by the value of HTTP_CONNECT_RETRY_COUNT
//
// <param name="object">The associated WebClientDataObject</param>
void ILibWebClient_RetrySink(ILibWebClient_StateObject object)
{
	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)object;
	struct ILibWebClientManager *wcm = wcdo->Parent;
	
	SEM_TRACK(WebClient_TrackLock("ILibWebClient_RetrySink",1,wcm);)
	sem_wait(&(wcm->QLock));
	wcdo->ExponentialBackoff = wcdo->ExponentialBackoff==0?1:wcdo->ExponentialBackoff * 2;
	if(wcdo->ExponentialBackoff>0)
	{
		//
		// Lets retry again
		//
		ILibQueue_EnQueue(wcdo->Parent->backlogQueue,wcdo);
	}
	SEM_TRACK(WebClient_TrackUnLock("ILibWebClient_RetrySink",3,wcm);)
	sem_post(&(wcm->QLock));

	if(!ILibIsChainBeingDestroyed(wcm->Chain))
	{
		ILibForceUnBlockChain(wcm->Chain);
	}
}


//
// Internal method dispatched by the OnSendOK event of the underlying ILibAsyncSocket
//
// <param name="socketModule">The underlying ILibAsyncSocket</param>
// <param name="user">The associated WebClientDataObject</param>
void ILibWebClient_OnSendOKSink(ILibAsyncSocket_SocketModule socketModule, void *user)
{
	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)user;
	struct ILibWebRequest *wr = (struct ILibWebRequest*)ILibQueue_PeekQueue(wcdo->RequestQueue);

	if(wr!=NULL)
	{
		wr->streamedState->OnSendOK(wcdo,wr->user1,wr->user2);
	}
}

//
// Internal method dispatched by the OnConnect event of the underlying ILibAsyncSocket
//
// <param name="socketModule">The underlying ILibAsyncSocket</param>
// <param name="Connected">Flag indicating connect status: Nonzero indicates success</param>
// <param name="user">Associated WebClientDataObject</param>
void ILibWebClient_OnConnect(ILibAsyncSocket_SocketModule socketModule, int Connected, void *user)
{
	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)user;
	struct ILibWebRequest *r;
	int i;

	int keyLength;
	char key[25];

	wcdo->SOCK = socketModule;
	wcdo->InitialRequestAnswered=0;
	wcdo->DisconnectSent=0;


	wcdo->PendingConnectionIndex=0;

	if(Connected!=0 && wcdo->DisconnectSent==0)
	{
		//Success: Send First Request
		wcdo->LocalIP = ILibAsyncSocket_GetLocalInterface(socketModule);
		wcdo->ExponentialBackoff=1;
		
		SEM_TRACK(WebClient_TrackLock("ILibWebClient_OnConnect",1,wcdo->Parent);)
		sem_wait(&(wcdo->Parent->QLock));
		r = ILibQueue_PeekQueue(wcdo->RequestQueue);
		SEM_TRACK(WebClient_TrackUnLock("ILibWebClient_OnConnect",2,wcdo->Parent);)
		sem_post(&(wcdo->Parent->QLock));
		if(r!=NULL)
		{
			for(i=0;i<r->NumberOfBuffers;++i)
			{
				ILibAsyncSocket_Send(socketModule,r->Buffer[i],r->BufferLength[i],-1);
			}
		
			if(r->streamedState!=NULL)
			{
				ILibWebClient_OnSendOKSink(socketModule,wcdo);
			}
		}
	}
	else
	{
		//
		// The connection failed, so lets set a timed callback, and try again
		//
		if(wcdo->DisconnectSent==0)
		{
			wcdo->Closing=2; //This is required, so we don't notify the user yet
			ILibAsyncSocket_Disconnect(socketModule);
			wcdo->Closing=0;
			//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
			wcdo->PipelineFlag = PIPELINE_UNKNOWN;
			//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
		}

		if(wcdo->ExponentialBackoff==(int)pow((double)2,(double)HTTP_CONNECT_RETRY_COUNT))
		{

			//
			// Retried enough times, give up
			//
			sem_wait(&(wcdo->Parent->QLock));
			keyLength = sprintf(key,"%s:%d:%d",inet_ntoa(wcdo->remote.sin_addr),(int)ntohs(wcdo->remote.sin_port),wcdo->IndexNumber);		
			MEMCHECK(assert(keyLength<=25);) 

			ILibDeleteEntry(wcdo->Parent->DataTable,key,keyLength);
			ILibDeleteEntry(wcdo->Parent->idleTable,key,keyLength);
			
			SEM_TRACK(WebClient_TrackUnLock("ILibWebClient_RetrySink",2,wcm);)
			sem_post(&(wcdo->Parent->QLock));
			ILibWebClient_DestroyWebClientDataObject(wcdo);
		}
		else
		{
			ILibLifeTime_Add(wcdo->Parent->timer,wcdo,wcdo->ExponentialBackoff,&ILibWebClient_RetrySink,NULL);
		}
	}
}
//
// Internal method dispatched by the OnDisconnect event of the underlying ILibAsyncSocket
// 
// <param name="socketModule">The underlying ILibAsyncSocket</param>
// <param name="user">The associated WebClientDataObject</param>
void ILibWebClient_OnDisconnectSink(ILibAsyncSocket_SocketModule socketModule, void *user)
{
	struct packetheader *h;
	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)user;
	struct ILibWebRequest *wr;

	char *buffer;
	int BeginPointer,EndPointer;

	if(wcdo==NULL) {return;}
	if(wcdo->DeferDestruction && wcdo->CancelRequest==0) 
	{
		return;
	}

	if(wcdo->DisconnectSent!=0)
	{
		//
		// We probably closed the socket on purpose, and don't want to tell
		// anyone about it yet
		//
		return;
	}
	else if(ILibQueue_PeekQueue(wcdo->RequestQueue)!=NULL && wcdo->WaitForClose!=0 && wcdo->CancelRequest==0)
	{
		//
		// There are still pending requests, so we probably already
		// send the disconnect event up
		//
		wcdo->DisconnectSent = 1;
	}

	wcdo->SOCK = NULL;

	//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
	if(wcdo->Closing==0) 
	{
		if(ILibQueue_PeekQueue(wcdo->RequestQueue)!=NULL && wcdo->CancelRequest==0)
		{
			//
			// If there are still pending requests, than obviously this server
			// doesn't do persistent connections
			//
			wcdo->PipelineFlag = PIPELINE_NO;
		}
	}
	//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}

	if(wcdo->WaitForClose!=0 && wcdo->CancelRequest==0)
	{
		//
		// Since we had to read until the socket closes, we finally have
		// all the data we need
		//
		ILibAsyncSocket_GetBuffer(socketModule,&buffer,&BeginPointer,&EndPointer);
		
		SEM_TRACK(WebClient_TrackLock("ILibWebClient_OnDisconnect",1,wcdo->Parent);)
		sem_wait(&(wcdo->Parent->QLock));
		wr = ILibQueue_DeQueue(wcdo->RequestQueue);
		SEM_TRACK(WebClient_TrackUnLock("ILibWebClient_OnDisconnect",2,wcdo->Parent);)
		sem_post(&(wcdo->Parent->QLock));
		wcdo->InitialRequestAnswered=1;
		//{{{ REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT--> }}}
		wcdo->PipelineFlag = PIPELINE_NO;
		//{{{ <--REMOVE_THIS_FOR_HTTP/1.0_ONLY_SUPPORT }}}
		wcdo->FinHeader=0;
		h = wcdo->header;
		wcdo->header = NULL;
		if(wr!=NULL && wr->OnResponse!=NULL)
		{				
			wr->OnResponse(
				wcdo,
				0,
				h,
				buffer,
				&BeginPointer,
				EndPointer,
				-1,
				wr->user1,
				wr->user2,
				&(wcdo->PAUSE));
		}
		ILibWebClient_ResetWCDO(wcdo);
		if(wcdo->DisconnectSent==1)
		{
			wcdo->DisconnectSent=0;
		}
		if(wr!=NULL)
		{
			ILibWebClient_DestroyWebRequest(wr);
		}
		if(h!=NULL)
		{
			ILibDestructPacket(h);
		}
	}
	
	if(wcdo->Closing!=0){return;}

	SEM_TRACK(WebClient_TrackLock("ILibWebClient_OnDisconnect",3,wcdo->Parent);)
	sem_wait(&(wcdo->Parent->QLock));
	wr = ILibQueue_PeekQueue(wcdo->RequestQueue);
	SEM_TRACK(WebClient_TrackUnLock("ILibWebClient_OnDisconnect",4,wcdo->Parent);)
	sem_post(&(wcdo->Parent->QLock));

	if(wr!=NULL)
	{
		// Still Requests to be made
		if(wcdo->InitialRequestAnswered==0 && wcdo->CancelRequest==0)
		{
			//Error
			wr->OnResponse(
				wcdo,
				0,
				NULL,
				NULL,
				NULL,
				0,
				-1,
				wr->user1,
				wr->user2,
				&(wcdo->PAUSE));
			ILibWebClient_FinishedResponse(socketModule,wcdo);	

			SEM_TRACK(WebClient_TrackLock("ILibWebClient_OnDisconnect",5,wcdo->Parent);)
			sem_wait(&(wcdo->Parent->QLock));
			wr = ILibQueue_PeekQueue(wcdo->RequestQueue);
			SEM_TRACK(WebClient_TrackUnLock("ILibWebClient_OnDisconnect",6,wcdo->Parent);)
			sem_post(&(wcdo->Parent->QLock));
			if(wr==NULL){return;}
		}


		// Make Another Connection and Continue
		wcdo->Closing = 0;
		ILibQueue_EnQueue(wcdo->Parent->backlogQueue,wcdo);
	}
	wcdo->CancelRequest=0;
}



//
// Chain PreSelect handler
//
// <param name="WebClientModule">The WebClient token</param>
// <param name="readset"></param>
// <param name="writeset"></param>
// <param name="errorset"></param>
// <param name="blocktime"></param>
void ILibWebClient_PreProcess(void* WebClientModule,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime)
{
	struct ILibWebClientManager *wcm = (struct ILibWebClientManager*)WebClientModule;
	struct ILibWebClientDataObject *wcdo;
	int i;
	int OK=0;

	//
	// Try and satisfy as many things as we can. If we have resources
	// grab a socket and go
	//
	SEM_TRACK(WebClient_TrackLock("ILibWebClient_PreProcess",1,wcm);)
	sem_wait(&(wcm->QLock));
	while(OK==0 && ILibQueue_IsEmpty(wcm->backlogQueue)==0)
	{
		OK=1;
		for(i=0;i<wcm->socksLength;++i)
		{
			if(ILibAsyncSocket_IsFree(wcm->socks[i])!=0)
			{
				OK=0;
				wcdo = ILibQueue_DeQueue(wcm->backlogQueue);
				if(wcdo!=NULL)
				{
					wcdo->Closing = 0;
					wcdo->PendingConnectionIndex = i;
					ILibAsyncSocket_ConnectTo(
						wcm->socks[i], 
						INADDR_ANY, 
						wcdo->remote.sin_addr.s_addr, 
						(int)ntohs(wcdo->remote.sin_port),
						NULL,
						wcdo);
				}
			}
			if(ILibQueue_IsEmpty(wcm->backlogQueue)!=0) {break;}
		}
	}
	SEM_TRACK(WebClient_TrackUnLock("ILibWebClient_PreProcess",2,wcm);)
	sem_post(&(wcm->QLock));
}

//
// Internal method dispatched by either ILibWebServer or ILibWebClient, to recheck the headers
//
// In certain cases, when the underlying buffer has been reallocated, the pointers in the 
// header structure will be invalid.
//
// <param name="token">The sender</param>
// <param name="user">The WCDO object</param>
// <param name="offSet">The offSet to the new buffer location</param>
void ILibWebClient_OnBufferReAllocate(ILibAsyncSocket_SocketModule token, void *user, ptrdiff_t offSet)
{
	struct packetheader_field_node *n;
	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)user;
	if(wcdo!=NULL && wcdo->header!=NULL)
	{
		//
		// Sometimes, the header was copied, in which case this realloc doesn't affect us
		//
		if(wcdo->header->ClonedPacket==0)
		{
			//
			// Sometimes the user instantiated the string, so again
			// this may not concern us
			//
			if(wcdo->header->UserAllocStrings==0)
			{
				if(wcdo->header->StatusCode==-1)
				{
					// Request Packet
					wcdo->header->Directive  += offSet;
					wcdo->header->DirectiveObj += offSet;
				}
				else
				{
					// Response Packet
					wcdo->header->StatusData += offSet;
				}
			}
			//
			// Sometimes the user instantiated the string, so again
			// this may not concern us
			//
			if(wcdo->header->UserAllocVersion==0)
			{
				wcdo->header->Version += offSet;
			}
			n = wcdo->header->FirstField;
			while(n!=NULL)
			{
				//
				// Sometimes the user instantiated the string, so again
				// this may not concern us
				//
				if(n->UserAllocStrings==0)
				{
					n->Field += offSet;
					n->FieldData += offSet;
				}
				n = n->NextField;
			}
		}
	}
}



//
// Internal method called by the ILibWebServer module to create a WebClientDataObject
//
// <param name="OnResponse">Function pointer to handle data reception</param>
// <param name="socketModule">The underlying ILibAsyncSocket</param>
// <param name="user1"></param>
// <param name="user2"></param>
// <returns>The WebClientDataObject</returns>
ILibWebClient_StateObject ILibCreateWebClientEx(ILibWebClient_OnResponse OnResponse, ILibAsyncSocket_SocketModule socketModule, void *user1, void *user2)
{
	struct ILibWebClientDataObject *wcdo;
	struct ILibWebRequest *wr;

	wcdo = (struct ILibWebClientDataObject*)malloc(sizeof(struct ILibWebClientDataObject));
	memset(wcdo,0,sizeof(struct ILibWebClientDataObject));
	wcdo->Parent = NULL;
	wcdo->RequestQueue = ILibQueue_Create();
	wcdo->Server = 1;
	wcdo->SOCK = socketModule;
	wcdo->PendingConnectionIndex=-1;

	wr = (struct ILibWebRequest*)malloc(sizeof(struct ILibWebRequest));
	memset(wr,0,sizeof(struct ILibWebRequest));
	wr->OnResponse = OnResponse;
	ILibQueue_EnQueue(wcdo->RequestQueue,wr);
	wr->user1 = user1;
	wr->user2 = user2;
	return(wcdo);
}

#ifdef UPNP_DEBUG
char *ILibWebClient_QueryWCDO(ILibWebClient_RequestManager wcm, char *query)
{
	struct ILibWebClientManager *wc = (struct ILibWebClientManager*)wcm;
	struct ILibWebClientDataObject *wcdo;
	struct ILibWebRequest *wr;
	void *en;
	char *key;
	int keyLen;
	void *data;

	char *RetVal = (char*)malloc(32768);
	int RetValLen = 32768;
	int RetValX=0;
	char temp[24];

	sem_wait(&(wc->QLock));
	if(stricmp(query,"*")==0)
	{
		en = ILibHashTree_GetEnumerator(wc->DataTable);
		while(ILibHashTree_MoveNext(en)==0)
		{
			ILibHashTree_GetValue(en,&key,&keyLen,&data);
			temp[keyLen]=0;
			memcpy(temp,key,keyLen);
			RetValX += sprintf(RetVal+RetValX,"WCDOKey: %s<br>",temp);
			if(data!=NULL)
			{
				wcdo = (struct ILibWebClientDataObject*)data;
				wr = ILibQueue_PeekQueue(wcdo->RequestQueue);
				if(wr!=NULL)
				{
					RetValX += sprintf(RetVal+RetValX,"Pending Requests: %d [%d]<br>%s<br>",ILibLinkedList_GetCount(wcdo->RequestQueue),wcdo->ExponentialBackoff,wr->Buffer[0]);
				}
				else
				{
					RetValX += sprintf(RetVal+RetValX,"NO PENDING REQUESTS<br>");
				}
			}
			RetValX += sprintf(RetVal+RetValX,"<br><br>");
		}
		ILibHashTree_DestroyEnumerator(en);
	}
	else
	{
		if(memcmp(query,"K",1)==0)
		{
			ILibDeleteEntry(wc->DataTable,query+1,(int)strlen(query+1));
			sprintf(RetVal,"DELETED<br>");
		}
		else
		{
			wcdo = (struct ILibWebClientDataObject*)ILibGetEntry(wc->DataTable,query,(int)strlen(query));
			if(wcdo!=NULL)
			{
				wr = ILibQueue_PeekQueue(wcdo->RequestQueue);
				if(wr!=NULL)
				{
					RetValX += sprintf(RetVal+RetValX,"%s<br>",wr->Buffer[0]);
				}
				else
				{
					RetValX += sprintf(RetVal+RetValX,"NO PENDING REQUESTS<br>");
				}
			}
			else
			{
				RetValX += sprintf(RetVal+RetValX,"NOT FOUND<br>");
			}
		}
	}
	sem_post(&(wc->QLock));
	return(RetVal);
}
#endif
/*! \fn ILibCreateWebClient(int PoolSize,void *Chain)
	\brief Constructor to create a new ILibWebClient
	\param PoolSize The max number of ILibAsyncSockets to have in the pool
	\param Chain The chain to add this module to. (Chain must <B>not</B> be running)
	\returns An ILibWebClient
*/
ILibWebClient_RequestManager ILibCreateWebClient(int PoolSize,void *Chain)
{
	int i;
	struct ILibWebClientManager *RetVal;
	
	if(Chain==NULL){return(NULL);}

	RetVal = (struct ILibWebClientManager*)malloc(sizeof(struct ILibWebClientManager));
	memset(RetVal,0,sizeof(struct ILibWebClientManager));
	RetVal->MaxConnectionsToSameServer = 1;
	
	RetVal->Destroy = &ILibDestroyWebClient;
	RetVal->PreSelect = &ILibWebClient_PreProcess;
	//RetVal->PostSelect = &ILibWebClient_PreProcess;

	RetVal->socksLength = PoolSize;
	RetVal->socks = (void**)malloc(PoolSize*sizeof(void*));
	sem_init(&(RetVal->QLock),0,1);
	RetVal->Chain = Chain;

	RetVal->backlogQueue = ILibQueue_Create();
	RetVal->DataTable = ILibInitHashTree();
	RetVal->idleTable = ILibInitHashTree();

	
	ILibAddToChain(Chain,RetVal);
	RetVal->timer = ILibCreateLifeTime(Chain);
	//
	// Create our pool of sockets
	//
	for(i=0;i<PoolSize;++i)
	{
		RetVal->socks[i] = ILibCreateAsyncSocketModule(
			Chain,
			INITIAL_BUFFER_SIZE,
			&ILibWebClient_OnData,
			&ILibWebClient_OnConnect,
			&ILibWebClient_OnDisconnectSink,
			&ILibWebClient_OnSendOKSink);
		//
		// We want to know about any buffer reallocations, because we may need to fix some things
		//
		ILibAsyncSocket_SetReAllocateNotificationCallback(RetVal->socks[i],&ILibWebClient_OnBufferReAllocate);
	}
	return((void*)RetVal);
}

void ILibWebClient_SetMaxConcurrentSessionsToServer(ILibWebClient_RequestManager WebClient, int maxConnections)
{
	struct ILibWebClientManager *wcm = (struct ILibWebClientManager*)WebClient;
	wcm->MaxConnectionsToSameServer = maxConnections;
}

/*! \fn ILibWebClient_PipelineRequest(ILibWebClient_RequestManager WebClient, struct sockaddr_in *RemoteEndpoint, struct packetheader *packet, ILibWebClient_OnResponse OnResponse, void *user1, void *user2)
	\brief Queues a new web request
	\param WebClient The ILibWebClient to queue the requests to
	\param RemoteEndpoint The destination
	\param packet The packet to send
	\param OnResponse Response Handler
	\param user1 User object
	\param user2 User object
	\returns Request Token
*/
ILibWebClient_RequestToken ILibWebClient_PipelineRequest(
								ILibWebClient_RequestManager WebClient, 
								struct sockaddr_in *RemoteEndpoint, 
								struct packetheader *packet,
								ILibWebClient_OnResponse OnResponse,
								void *user1,
								void *user2)
{
	int bufferLength;
	char *buffer;

	bufferLength = ILibGetRawPacket(packet,&buffer);
	ILibDestructPacket(packet);
	return(ILibWebClient_PipelineRequestEx(WebClient,RemoteEndpoint,buffer,bufferLength,ILibAsyncSocket_MemoryOwnership_CHAIN,NULL,0,0,OnResponse,user1,user2));
}

ILibWebClient_RequestToken ILibWebClient_PipelineRequestEx2(
	ILibWebClient_RequestManager WebClient, 
	struct sockaddr_in *RemoteEndpoint, 
	char *headerBuffer,
	int headerBufferLength,
	int headerBuffer_FREE,
	char *bodyBuffer,
	int bodyBufferLength,
	int bodyBuffer_FREE,
	ILibWebClient_OnResponse OnResponse,
	struct ILibWebClient_StreamedRequestState *state,
	void *user1,
	void *user2)
{
	int ForceUnBlock=0;
	char IPV4Address[25];
	int IPV4AddressLength=0;
	struct ILibWebClientManager *wcm = (struct ILibWebClientManager*)WebClient;
	struct ILibWebClientDataObject *wcdo;
	struct ILibWebRequest *request = (struct ILibWebRequest*)malloc(sizeof(struct ILibWebRequest));
	int i;

	int indexWithLeast;
	int numberOfItems;

	memset(request,0,sizeof(struct ILibWebRequest));
	request->NumberOfBuffers = bodyBuffer!=NULL?2:1;
	request->Buffer = (char**)malloc(request->NumberOfBuffers*sizeof(char*));
	request->BufferLength = (int*)malloc(request->NumberOfBuffers*sizeof(int));
	request->UserFree = (int*)malloc(request->NumberOfBuffers*sizeof(int));

	request->Buffer[0] = headerBuffer;
	request->BufferLength[0] = headerBufferLength;
	request->UserFree[0] = headerBuffer_FREE;
	request->requestToken = (struct ILibWebClient_PipelineRequestToken*)malloc(sizeof(struct ILibWebClient_PipelineRequestToken));
	memset(request->requestToken,0,sizeof(struct ILibWebClient_PipelineRequestToken));
	request->requestToken->timer = wcm->timer;

	if(headerBufferLength>5 && strncasecmp("HEAD ",headerBuffer,5)==0)
	{
		request->IsHeadRequest = 1;
	}

	if(bodyBuffer!=NULL)
	{
		request->Buffer[1] = bodyBuffer;
		request->BufferLength[1] = bodyBufferLength;
		request->UserFree[1] = bodyBuffer_FREE;
	}

	if(state!=NULL)
	{
		// We were called from ILibWebClient_PipelineStreamedRequest
		request->streamedState = state;
	}

	request->OnResponse = OnResponse;
	request->user1 = user1;
	request->user2 = user2;
	
	request->remote.sin_port = RemoteEndpoint->sin_port;
	request->remote.sin_addr.s_addr = RemoteEndpoint->sin_addr.s_addr;

	//
	// Does the client already have a connection to the server?
	//
	SEM_TRACK(WebClient_TrackLock("ILibWebClient_PipelineRequestEx",1,wcm);)
	sem_wait(&(wcm->QLock));

	if(wcm->MaxConnectionsToSameServer>1)
	{
		for(i=0;i<wcm->MaxConnectionsToSameServer;++i)
		{
			IPV4AddressLength = sprintf(IPV4Address,"%s:%d:%d",
				inet_ntoa(RemoteEndpoint->sin_addr),
				ntohs(RemoteEndpoint->sin_port),
				i);
			MEMCHECK(assert(IPV4AddressLength<=25);)
			if(ILibHasEntry(wcm->DataTable,IPV4Address,IPV4AddressLength)==0)
			{
				//
				// A free slot!
				//
				break;
			}
		}
		if(i==wcm->MaxConnectionsToSameServer)
		{
			//
			// There were not any free slots, so we need to find the one with the
			// fewest number of requests.
			//
			indexWithLeast = -1;
			numberOfItems = -1;
			for(i=0;i<wcm->MaxConnectionsToSameServer;++i)
			{
				IPV4AddressLength = sprintf(IPV4Address,"%s:%d:%d",
					inet_ntoa(RemoteEndpoint->sin_addr),
					ntohs(RemoteEndpoint->sin_port),
					i);
				MEMCHECK(assert(IPV4AddressLength<=25);)
				wcdo = (struct ILibWebClientDataObject*)ILibGetEntry(wcm->DataTable,IPV4Address,IPV4AddressLength);
				
				if(numberOfItems==-1)
				{
					numberOfItems = ILibLinkedList_GetCount(wcdo->RequestQueue);
					indexWithLeast = i;
				}
				else
				{
					if(ILibLinkedList_GetCount(wcdo->RequestQueue)<numberOfItems)
					{
						numberOfItems = ILibLinkedList_GetCount(wcdo->RequestQueue);
						indexWithLeast = i;
					}
				}
			}
			IPV4AddressLength = sprintf(IPV4Address,"%s:%d:%d",
				inet_ntoa(RemoteEndpoint->sin_addr),
				ntohs(RemoteEndpoint->sin_port),
				indexWithLeast);
			MEMCHECK(assert(IPV4AddressLength<=25);)
		}
	}
	else
	{
		i = 0;
		IPV4AddressLength = sprintf(IPV4Address,"%s:%d:%d",
			inet_ntoa(RemoteEndpoint->sin_addr),
			ntohs(RemoteEndpoint->sin_port),
			0);
		MEMCHECK(assert(IPV4AddressLength<=25);)
	}
	if(ILibHasEntry(wcm->DataTable,IPV4Address,IPV4AddressLength)!=0)
	{
		//
		// Yes it does!
		//
		wcdo = (struct ILibWebClientDataObject*)ILibGetEntry(wcm->DataTable,IPV4Address,IPV4AddressLength);
		request->requestToken->wcdo = wcdo;
		if(ILibQueue_IsEmpty(wcdo->RequestQueue)!=0)
		{
			//
			// There are no pending requests however, so we can try to send this right away!
			//
			ILibQueue_EnQueue(wcdo->RequestQueue,request);

			// Take out of Idle State
			wcm->idleCount = wcm->idleCount==0?0:wcm->idleCount-1;
			ILibDeleteEntry(wcm->idleTable,IPV4Address,IPV4AddressLength);
			ILibLifeTime_Remove(wcm->timer,wcdo);
			if(wcdo->DisconnectSent==0 && (wcdo->SOCK==NULL || ILibAsyncSocket_IsFree(wcdo->SOCK)))
			{
				//
				// If this was in our idleTable, then most likely the select doesn't know about
				// it, so we need to force it to unblock
				//
				ILibQueue_EnQueue(wcm->backlogQueue,wcdo);	
				ForceUnBlock=1;
			}
			else if(wcdo->SOCK!=NULL)
			{
				//
				// Socket is still there
				//
				if(wcdo->WaitForClose==0)
				{
					for(i=0;i<request->NumberOfBuffers;++i)
					{
						ILibAsyncSocket_Send(wcdo->SOCK,request->Buffer[i],request->BufferLength[i],1);
					}
					if(request->streamedState!=NULL)
					{
						ILibWebClient_OnSendOKSink(wcdo->SOCK,wcdo);
					}
				}
			}
		}
		else
		{
			//
			// There are still pending requests, so lets just queue this up
			//
			ILibQueue_EnQueue(wcdo->RequestQueue,request);
		}
	}
	else
	{
		// 
		// There is no previous connection, so we need to set it up
		//
		wcdo = (struct ILibWebClientDataObject*)malloc(sizeof(struct ILibWebClientDataObject));
		request->requestToken->wcdo = wcdo;
		memset(wcdo,0,sizeof(struct ILibWebClientDataObject));
		wcdo->Parent = wcm;
		wcdo->PendingConnectionIndex=-1;
		wcdo->RequestQueue = ILibQueue_Create();
		wcdo->remote.sin_port = RemoteEndpoint->sin_port;
		wcdo->remote.sin_addr.s_addr = RemoteEndpoint->sin_addr.s_addr;
		wcdo->IndexNumber = i;

		ILibQueue_EnQueue(wcdo->RequestQueue,request);
		ILibAddEntry(wcm->DataTable,IPV4Address,IPV4AddressLength,wcdo);
		if(wcdo->DisconnectSent==0)
		{
			//
			// Queue it up in our Backlog, because we don't want to burden ourselves, so we
			// need to see if we have the resources for it. The Pool will grab one when it can.
			// The chain doesn't know about us, so we need to force it to unblock, to process this.
			//
			ILibQueue_EnQueue(wcm->backlogQueue,wcdo);		
			ForceUnBlock=1;
		}
	}
	SEM_TRACK(WebClient_TrackUnLock("ILibWebClient_PipelineRequestEx",2,wcm);)
	sem_post(&(wcm->QLock));
	if(ForceUnBlock!=0)
	{
		ILibForceUnBlockChain(wcm->Chain);
	}
	SESSION_TRACK(request->requestToken,NULL,"PipelinedRequestEx");
	return(request->requestToken);
}

/*! \fn ILibWebClient_PipelineRequestEx(
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
	void *user2)
	\brief Queues a new web request
	\par
	This method differs from ILibWebClient_PiplineRequest, in that this method
	allows you to directly specify the buffers, rather than a packet structure
	\param WebClient The ILibWebClient to queue the requests to
	\param RemoteEndpoint The destination
	\param headerBuffer The buffer containing the headers
	\param headerBufferLength The length of the headers
	\param headerBuffer_FREE Flag indicating memory ownership of buffer
	\param bodyBuffer The buffer containing the HTTP body
	\param bodyBufferLength The length of the buffer
	\param bodyBuffer_FREE Flag indicating memory ownership of buffer
	\param OnResponse Data reception handler
	\param user1 User object
	\param user2 User object
	\returns Request Token
*/
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
	void *user2)
{
	return(ILibWebClient_PipelineRequestEx2(WebClient,RemoteEndpoint,headerBuffer,headerBufferLength,headerBuffer_FREE,bodyBuffer,bodyBufferLength,bodyBuffer_FREE,OnResponse,NULL,user1,user2));
}
ILibWebClient_RequestToken ILibWebClient_PipelineRequest2(
								ILibWebClient_RequestManager WebClient, 
								struct sockaddr_in *RemoteEndpoint, 
								struct packetheader *packet,
								ILibWebClient_OnResponse OnResponse,
								struct ILibWebClient_StreamedRequestState *state,
								void *user1,
								void *user2)
{
	int bufferLength;
	char *buffer;

	bufferLength = ILibGetRawPacket(packet,&buffer);
	ILibDestructPacket(packet);
	return(ILibWebClient_PipelineRequestEx2(WebClient,RemoteEndpoint,buffer,bufferLength,ILibAsyncSocket_MemoryOwnership_CHAIN,NULL,0,0,OnResponse,state,user1,user2));
}

//
// Returns the headers from a given WebClientDataObject
//
// <param name="token">The WebClientDataObject to query</param>
// <returns>The headers</returns>
struct packetheader *ILibWebClient_GetHeaderFromDataObject(ILibWebClient_StateObject token)
{
	return(token==NULL?NULL:((struct ILibWebClientDataObject*)token)->header);
}

/*! \fn ILibWebClient_DeleteRequests(ILibWebClient_RequestManager WebClientToken,char *IP,int Port)
	\brief Deletes all pending requests to a specific IP/Port combination
	\param WebClientToken The ILibWebClient to purge
	\param IP The IP address of the destination
	\param Port The destination port
*/
void ILibWebClient_DeleteRequests(ILibWebClient_RequestManager WebClientToken,char *IP,int Port)
{
	struct ILibWebClientManager *wcm = (struct ILibWebClientManager*)WebClientToken;
	char IPV4Address[25];
	struct ILibWebClientDataObject *wcdo=NULL;
	int IPV4AddressLength;
	struct ILibWebRequest *wr;
	int zero = 0;
	int i;

	void *RemoveQ;
	
	if(wcm==NULL) {return;}

	RemoveQ = ILibQueue_Create();


	for(i=0;i<wcm->MaxConnectionsToSameServer;++i)
	{
		IPV4AddressLength = sprintf(IPV4Address,"%s:%d:%d",IP,Port,i);
		MEMCHECK(assert(IPV4AddressLength<=24);) 

		//
		// Are there any pending requests to this IP/Port combo?
		//
		SEM_TRACK(WebClient_TrackLock("ILibWebClient_DeleteRequests",1,wcm);)
		sem_wait(&(wcm->QLock));
		if(ILibHasEntry(wcm->DataTable,IPV4Address,IPV4AddressLength)!=0)
		{
			//
			// Yes there is!. Lets iterate through them
			//
			wcdo = (struct ILibWebClientDataObject*)ILibGetEntry(wcm->DataTable,IPV4Address,IPV4AddressLength);
			if(wcdo!=NULL)
			{
				//
				// Remove ourselves from the backlog queue, because a previous connection attempt may be in progress.
				//
				ILibLinkedList_Remove_ByData(wcm->backlogQueue,wcdo);
				while(ILibQueue_IsEmpty(wcdo->RequestQueue)==0)
				{
					//
					// Put all the pending requests into this queue, so we can trigger them outside of this lock
					//
					wr = (struct ILibWebRequest*)ILibQueue_DeQueue(wcdo->RequestQueue);
					ILibQueue_EnQueue(RemoveQ,wr);
				}
			
				ILibLifeTime_Remove(wcdo->Parent->timer,wcdo);
			}
		}
		SEM_TRACK(WebClient_TrackUnLock("ILibWebClient_DeleteRequests",2,wcm);)
		sem_post(&(wcm->QLock));
	}


	//
	// Lets iterate through all the requests that we need to get rid of
	//
	while(ILibQueue_IsEmpty(RemoveQ)==0)
	{
		//
		// Tell the user, we are aborting these requests
		//
		wr = (struct ILibWebRequest*)ILibQueue_DeQueue(RemoveQ);
					if(wr->OnResponse!=NULL)
		wr->OnResponse(				
			NULL,
			WEBCLIENT_DELETED,
			NULL,
			NULL,
			NULL,
			0,
			-1,
			wr->user1,
			wr->user2,
			&zero);
		
		ILibWebClient_DestroyWebRequest(wr);
	}
	if(wcdo!=NULL && wcdo->Parent!=NULL)
	{
		if(wcdo->PendingConnectionIndex!=-1)
		{
			ILibAsyncSocket_Disconnect(wcm->socks[wcdo->PendingConnectionIndex]);
			wcdo->PendingConnectionIndex=-1;
		}
	}
	if(wcdo!=NULL && wcdo->SOCK!=NULL)
	{
		ILibAsyncSocket_Disconnect(wcdo->SOCK);
	}
	ILibQueue_Destroy(RemoveQ);
}

/*! \fn ILibWebClient_Resume(ILibWebClient_StateObject wcdo)
	\brief Resumes a paused connection
	\par
	If the client has set the PAUSED flag, the underlying socket will no longer
	read data from the NIC. This method, resumes the socket.
	\param wcdo The associated WebClientDataObject
*/
void ILibWebClient_Resume(ILibWebClient_StateObject wcdo)
{
	struct ILibWebClientDataObject *d = (struct ILibWebClientDataObject*)wcdo;
	
	if(d!=NULL)
	{
		d->PAUSE = 0;
		ILibAsyncSocket_Resume(d->SOCK);
	}
}
void ILibWebClient_Pause(ILibWebClient_StateObject wcdo)
{
	struct ILibWebClientDataObject *d = (struct ILibWebClientDataObject*)wcdo;
	if(d!=NULL)
	{
		d->PAUSE = 1;
	}
}

/*! \fn ILibWebClient_Disconnect(ILibWebClient_StateObject wcdo)
	\brief Disconnects the underlying socket, of a client object.
	\par
	<b>Note</b>: This is <b>not</b> to be used to close an HTTP Session, as ILibWebClient does not
	keep separate states for client sessions. The HTTP behavior is abstracted, such that the user <b>must</b> not
	make any assumptions about the connection, because multiple requests could be multiplexed into a single connection.
	<br><br>
	If the user desires to cancel their client session, they need to cancel the requests that they had made. That can
	be accomplished by calling \a ILibWebClient_CancelRequest, with the RequestToken obtained from \a ILibWebClient_PipelineRequest.
	\param wcdo WebClient State Object, obtained from \a ILibCreateWebClientEx.
*/
void ILibWebClient_Disconnect(ILibWebClient_StateObject wcdo)
{
	struct ILibWebClientDataObject *d = (struct ILibWebClientDataObject*)wcdo;
	if(d!=NULL)
	{
		ILibAsyncSocket_Disconnect(d->SOCK);
	}
}

void ILibWebClient_CancelRequestEx2(ILibWebClient_StateObject wcdo, void *userRequest)
{
	void *node,*nextnode;
	struct ILibWebRequest *wr;
	struct ILibWebClientDataObject *_wcdo = (struct ILibWebClientDataObject*)wcdo;
	
	int HeadDeleted = 0;
	void *head;

	int BeginPointer=0;
	int EndPointer=0;

	void *PendingRequestQ=NULL;

	if(wcdo!=NULL)
	{
		PendingRequestQ = ILibQueue_Create();

		SEM_TRACK(WebClient_TrackLock("ILibWebClient_CancelRequest",1,wcdo->Parent);)
		sem_wait(&(_wcdo->Parent->QLock));

		head = node = ILibLinkedList_GetNode_Head(_wcdo->RequestQueue);
		while(node!=NULL)
		{
			nextnode = ILibLinkedList_GetNextNode(node);

			wr = (struct ILibWebRequest*)ILibLinkedList_GetDataFromNode(node);
			if(wr->requestToken==userRequest)
			{
				if(node==head)
				{
					SESSION_TRACK(wr->requestToken,NULL,"Cancelling Current Request");
					HeadDeleted=1;
				}
				else
				{
					SESSION_TRACK(wr->requestToken,NULL,"Cancelling Request");
				}
				
				ILibQueue_EnQueue(PendingRequestQ,wr);
				ILibLinkedList_Remove(node);
			}
			node = nextnode;
		}

		// If this connection was unsuccessful, remove it from the retry
		// logic, because we will have to manually re-connect anyways, in order
		// for the rest of the pending requests to go through, because we have to manually
		// disconnect the socket, in order to interrupt this session.
		ILibLifeTime_Remove(_wcdo->Parent->timer,_wcdo);
		if(HeadDeleted!=0 && _wcdo->SOCK!=NULL)
		{
			//
			// Since the request we removed was the currently active request, we
			// must close the socket, because that is the only way to interrupt the server
			//
			ILibWebClient_ResetWCDO(wcdo);
			_wcdo->Closing = 2;
			_wcdo->CancelRequest=1;
			ILibWebClient_Disconnect(_wcdo);
			_wcdo->Closing =0;
			_wcdo->CancelRequest=0;

			//
			// Now that we disconnected the session, re-connect it, if there are more requests
			// to be handled
			//
			if(ILibQueue_PeekQueue(_wcdo->RequestQueue)!=NULL)
			{
				//
				// Queue the session to be reconnected
				//
				ILibQueue_EnQueue(_wcdo->Parent->backlogQueue,_wcdo);				
			}
		}
		
		SEM_TRACK(WebClient_TrackUnLock("ILibWebClient_CancelRequest",2,wcdo->Parent);)
		sem_post(&(_wcdo->Parent->QLock));

		wr = (struct ILibWebRequest*)ILibQueue_DeQueue(PendingRequestQ);
		while(wr!=NULL)
		{			
			if(wr->OnResponse!=NULL)
			{				
				wr->OnResponse(
					_wcdo,
					0,
					NULL,
					NULL,          // No Buffer to pass in
					&BeginPointer, // This is zero
					EndPointer,    // This is zero
					-1,
					wr->user1,
					wr->user2,
					&(_wcdo->PAUSE)); // Pause is also zero
			}

			ILibWebClient_DestroyWebRequest(wr);
			wr = (struct ILibWebRequest*)ILibQueue_DeQueue(PendingRequestQ);
		}
		ILibQueue_Destroy(PendingRequestQ);
	}
}

void ILibWebClient_CancelRequestEx(void *RequestToken)
{
	ILibWebClient_CancelRequestEx2(((struct ILibWebClient_PipelineRequestToken*)RequestToken)->wcdo,RequestToken);
}

/*! \fn ILibWebClient_CancelRequest(ILibWebClient_RequestToken RequestToken)
	\brief Cancels a pending request via the RequestToken received when making the request.
	\param RequestToken The identifier obtained from calls to \a ILibWebClient_PipelineRequest.
*/
void ILibWebClient_CancelRequest(ILibWebClient_RequestToken RequestToken)
{
	if(RequestToken!=NULL)
	{
		ILibLifeTime_Add(((struct ILibWebClient_PipelineRequestToken*)RequestToken)->timer,RequestToken,0,ILibWebClient_CancelRequestEx,NULL);
	}
}
/*! \fn ILibWebClient_GetRequestToken_FromStateObject(ILibWebClient_StateObject WebStateObject)
	\brief Obtains the Request Token associated with the specified WebReader (response) token.
	\param WebStateObject The response token obtained from the response handler passed into \a ILibWebClient_PipelineRequest.
	\returns The request identifier of the request this response was for
*/
ILibWebClient_RequestToken ILibWebClient_GetRequestToken_FromStateObject(ILibWebClient_StateObject WebStateObject)
{
	struct ILibWebClientDataObject *wcdo = (struct ILibWebClientDataObject*)WebStateObject;
	struct ILibWebRequest *wr;

	if(wcdo==NULL){return(NULL);}

	wr = (struct ILibWebRequest*)ILibQueue_PeekQueue(wcdo->RequestQueue);

	if(wr!=NULL)
	{
		return(wr->requestToken);
	}
	else
	{
		return(NULL);
	}
}

/*! \fn ILibWebClient_StateObject ILibWebClient_GetStateObjectFromRequestToken(ILibWebClient_RequestToken token)
	\brief Obtain the user object that was associated with a request, from the request token
	\param token The token that was obtained from a call to \a ILibWebClient_PipelineRequest or \a ILibWebClient_PipelineRequestEx
*/
ILibWebClient_StateObject ILibWebClient_GetStateObjectFromRequestToken(ILibWebClient_RequestToken token)
{
	return(((struct ILibWebClient_PipelineRequestToken*)token)->wcdo);
}





/*! \fn ILibWebClient_RequestToken ILibWebClient_PipelineStreamedRequest(ILibWebClient_RequestManager WebClient,struct sockaddr_in *RemoteEndpoint,struct packetheader *packet,ILibWebClient_OnResponse OnResponse,ILibWebClient_OnSendOK OnSendOK,void *user1,void *user2)
	\brief Queues a web request, but allows for streaming of the request body
	\param WebClient The client to queue to request onto
	\param RemoteEndpoint The server to make the request to
	\param packet The HTTP headers to send to the server
	\param OnResponse Function pointer that will get triggered apon receipt of a response
	\param OnSendOK Function pointer that will trigger when a connection has been established and again when all calls to ILibWebClient_StreamRequestBody() have completed
	\param user1 User state object
	\param user2 User state object
*/
ILibWebClient_RequestToken ILibWebClient_PipelineStreamedRequest(ILibWebClient_RequestManager WebClient,struct sockaddr_in *RemoteEndpoint,struct packetheader *packet,ILibWebClient_OnResponse OnResponse,ILibWebClient_OnSendOK OnSendOK,void *user1,void *user2)
{
	struct ILibWebClient_StreamedRequestState *state = (struct ILibWebClient_StreamedRequestState*)malloc(sizeof(struct ILibWebClient_StreamedRequestState));
	memset(state,0,sizeof(struct ILibWebClient_StreamedRequestState));

	state->BufferQueue = ILibQueue_Create();
	state->OnSendOK = OnSendOK;
	state->doNotSendRightAway=1;

	ILibAddHeaderLine(packet,"Transfer-Encoding",17,"chunked",7);
	
	return(ILibWebClient_PipelineRequest2(WebClient,RemoteEndpoint,packet,OnResponse,state,user1,user2));
}

/*! \fn void ILibWebClient_StreamRequestBody(ILibWebClient_RequestToken token, char *body,int bodyLength, enum ILibAsyncSocket_MemoryOwnership MemoryOwnership,int done)
	\brief Streams part of the request body
	\param token The RequestToken received from a call to ILibWebClient_PipelineStreamedRequest()
	\param body The buffer to send
	\param bodyLength Size of the buffer to send
	\param MemoryOwnership Memory ownership flag for the buffer
	\param done Non-zero if all of the body has been submitted
*/
int ILibWebClient_StreamRequestBody(
									 ILibWebClient_RequestToken token, 
									 char *body,
									 int bodyLength, 
									 enum ILibAsyncSocket_MemoryOwnership MemoryOwnership,
									 int done
									 )
{
	struct ILibWebClient_PipelineRequestToken *t = (struct ILibWebClient_PipelineRequestToken*)token;
	struct ILibWebRequest *wr;

	char hex[16];
	int hexLen;
	int result = 0;
	
	if(t!=NULL && t->wcdo!=NULL)
	{
		wr = (struct ILibWebRequest*)ILibQueue_PeekQueue(t->wcdo->RequestQueue);
		if(body!=NULL && bodyLength>0)
		{
			hexLen = sprintf(hex,"%X\r\n",bodyLength);
			result = ILibAsyncSocket_Send(t->wcdo->SOCK,hex,hexLen,ILibAsyncSocket_MemoryOwnership_USER);
			if(result>=0)
			{
				result = ILibAsyncSocket_Send(t->wcdo->SOCK,body,bodyLength,MemoryOwnership);
				if(result>=0)
				{
					result = ILibAsyncSocket_Send(t->wcdo->SOCK,"\r\n",2,ILibAsyncSocket_MemoryOwnership_STATIC);
				}
			}
			else if(MemoryOwnership==ILibAsyncSocket_MemoryOwnership_CHAIN)
			{
				free(body);
			}
		}
		if(result>=0 && done!=0)
		{
			result = ILibAsyncSocket_Send(t->wcdo->SOCK,"0\r\n\r\n",5,ILibAsyncSocket_MemoryOwnership_STATIC);
		}
		
		if(result==0 && wr->streamedState->OnSendOK != NULL && done==0)
		{
			//
			// All the data was sent, so call OnSendOK
			//

			wr->streamedState->OnSendOK(t->wcdo,wr->user1,wr->user2);
		}
	}
	return(result);
}
/*! \fn void ILibWebClient_Parse_ContentRange(char *contentRange, int *Start, int *End, int *TotalLength)
	\brief Parses an HTTP/206 Partial Content, Content-Range header, to obtain the range that was returned.
	\param contentRange The Content-Range header to parse. This can be obtained with a call to \a ILibGetHeaderLine
	\param[out] Start Pointer to the int value where the Start byte position will be stored
	\param[out] End Pointer to the int value where the End byte position will be stored
	\param[out] TotalLength Pointer to the int value where the total available length will be stored
*/
void ILibWebClient_Parse_ContentRange(char *contentRange, int *Start, int *End, int *TotalLength)
{
	struct parser_result *pr,*pr2,*pr3;
	*Start = 0;
	*End = 0;
	*TotalLength = 0;
	
	pr = ILibParseString(contentRange,0,(int)strlen(contentRange)," ",1);
	//
	// bytes x-y/z
	//
	if(pr->NumResults==2)
	{
		pr2 = ILibParseString(pr->LastResult->data,0,pr->LastResult->datalength,"/",1);
		if(pr2->NumResults==2)
		{
			*TotalLength = atoi(pr2->LastResult->data);
			pr3 = ILibParseString(pr2->FirstResult->data,0,pr2->FirstResult->datalength,"-",1);
			if(pr3->NumResults==2)
			{
				pr3->FirstResult->data[pr3->FirstResult->datalength]=0;
				pr3->LastResult->data[pr3->LastResult->datalength]=0;
				*Start = atoi(pr3->FirstResult->data);
				*End = atoi(pr3->LastResult->data);
			}
			ILibDestructParserResults(pr3);
		}
		ILibDestructParserResults(pr2);
	}
	ILibDestructParserResults(pr);
}
/*! \fn int ILibWebClient_Parse_Range(char *Range, long *Start, long *Length, long TotalLength)
	\brief Parses the Range request header, to obtain the requested range
	\param Range The Range header to parse. This can be obtained with a call to \a ILibGetHeaderLine
	\param[out] Start Pointer to the long value where the Start byte position will be stored
	\param[out] Length Pointer to the long value where the desired length will be stored.
	\param TotalLength The total length of available content.
	\returns 0 = Success, 1 = Failure
*/
enum ILibWebClient_Range_Result ILibWebClient_Parse_Range(char *Range, long *Start, long *Length, long TotalLength)
{
	struct parser_result *pr,*pr2;
	long x=-1;
	long y=-1;
	enum ILibWebClient_Range_Result RetVal = ILibWebClient_Range_Result_OK;

	*Start = 0;
	*Length = 0;

	pr = ILibParseString(Range,0,(int)strlen(Range),"=",1);
	if(pr->NumResults==2 && pr->FirstResult->datalength==5 && memcmp(pr->FirstResult->data,"bytes",5)==0)
	{
		pr2 = ILibParseString(pr->LastResult->data,0,pr->LastResult->datalength,"-",1);
		if(pr2->NumResults==2)
		{
			if(pr2->FirstResult->datalength == 0)
			{
				x = -1;
			}
			else
			{
				pr2->FirstResult->data[pr2->FirstResult->datalength]=0;
				x = atol(pr2->FirstResult->data);
			}
			if(pr2->LastResult->datalength == 0)
			{
				if(x!=-1)
				{
					y = TotalLength - x;
				}
			}
			else
			{
				pr2->LastResult->data[pr2->LastResult->datalength]=0;
				if(x!=-1)
				{
					y = 1+atol(pr2->LastResult->data)-x;
				}
				else
				{
					x = TotalLength - atol(pr2->LastResult->data);
					y = TotalLength - x;
				}
			}
		}
		else
		{
			RetVal = ILibWebClient_Range_Result_BAD_REQUEST;
		}
		ILibDestructParserResults(pr2);
	}
	else
	{
		RetVal = ILibWebClient_Range_Result_BAD_REQUEST;
	}
	ILibDestructParserResults(pr);

	if(x>=0 && y>=0 && x<=TotalLength)
	{
		*Start = x;
		*Length = y;
		RetVal = ILibWebClient_Range_Result_OK;
	}
	else if(RetVal==ILibWebClient_Range_Result_OK)
	{
		RetVal = ILibWebClient_Range_Result_INVALID_RANGE;
	}
	return(RetVal);
}
void ILibWebClient_SetUser(ILibWebClient_RequestManager manager, void *user)
{
	((struct ILibWebClientManager*)manager)->user = user;
}
void* ILibWebClient_GetUser(ILibWebClient_RequestManager manager)
{
	return(((struct ILibWebClientManager*)manager)->user);
}
void* ILibWebClient_GetChain(ILibWebClient_RequestManager manager)
{
	return(((struct ILibWebClientManager*)manager)->Chain);
}
