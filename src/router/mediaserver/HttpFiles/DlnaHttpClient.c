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
 * $Workfile: DlnaHttpClient.c
 * $Revision: #1.0.2201.28945
 * $Author:   Intel Corporation, Intel Device Builder
 * $Date:     Tuesday, January 10, 2006
 *
 *
 *
 */

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
#include "DlnaHttpClient.h"
#include "MimeTypes.h"
#include "DLNAProtocolInfo.h"

struct DHC_Data
{
	ILibThreadPool pool;
	ILibWebClient_RequestToken token;
	ILibWebClient_RequestManager manager;
	ILibWebClient_StateObject webState;
	ILibReaderWriterLock rwLock;

	FILE *f;
	DHC_OnResponseDone Callback;
	void *user;
	int GotContinue;

	char *buffer;
	int bufferLength;

	void *ActualPacket;
	long StartPosition;

	DH_TransferStatus TransferStatus;
};

void DHC_Pool(ILibThreadPool sender, void *var)
{
	struct DHC_Data *data = (struct DHC_Data*)var;
	int bytesRead = -1;
	long currentPosition;

	if(data->TransferStatus->TotalBytesToBeSent==-1)
	{
		sem_wait(&(data->TransferStatus->syncLock));
		currentPosition = ftell(data->f);
		fseek(data->f,0,SEEK_END);
		data->TransferStatus->TotalBytesToBeSent = ftell(data->f) - currentPosition;
		fseek(data->f,currentPosition,SEEK_SET);
		sem_post(&(data->TransferStatus->syncLock));
	}

	if(data->f!=NULL && feof(data->f))
	{
		fclose(data->f);
		data->f = NULL;
		printf("C");
		ILibWebClient_StreamRequestBody(data->token,NULL,0,ILibAsyncSocket_MemoryOwnership_STATIC,1);
	}
	else
	{
		bytesRead = (int)fread(data->buffer,sizeof(char),data->bufferLength,data->f);

		if(bytesRead>0)
		{
			ILibWebClient_StreamRequestBody(data->token,data->buffer,bytesRead,ILibAsyncSocket_MemoryOwnership_USER,0);
			
			sem_wait(&(data->TransferStatus->syncLock));
			data->TransferStatus->ActualBytesSent += (long)bytesRead;
			sem_post(&(data->TransferStatus->syncLock));
		}
		else
		{
			//
			// Read Error
			//
			if(feof(data->f))
			{
				fclose(data->f);
				data->f = NULL;
				ILibWebClient_StreamRequestBody(data->token,NULL,0,ILibAsyncSocket_MemoryOwnership_STATIC,1);
			}
		}
	}



}
void DHC_OnSendOK(ILibWebClient_StateObject sender, void *user1, void *user2)
{
	struct DHC_Data *data = (struct DHC_Data*)user2;
	int ok = 0;
	void *Abort = NULL;

	sem_wait(&(data->TransferStatus->syncLock));
	if(data->TransferStatus->Reserved1!=0)
	{
		data->TransferStatus->Reserved2 = 1;
		data->TransferStatus->Reserved3 = data;
	}
	else if(data->TransferStatus->Reserved4==0)
	{
		ok = 1;
	}
	if(data->TransferStatus->Reserved4!=0 && data->TransferStatus->RequestToken!=NULL)
	{
		//
		// Abort
		//
		Abort = data->TransferStatus->RequestToken;
		data->TransferStatus->RequestToken = NULL;
	}
	sem_post(&(data->TransferStatus->syncLock));

	if(Abort==NULL && ok && data->f!=NULL && data->GotContinue!=0)
	{
		ILibThreadPool_QueueUserWorkItem(data->pool,data,&DHC_Pool);
	}
	else if(Abort!=NULL)
	{
		ILibWebClient_CancelRequest(Abort);
	}
}
void DHC_OnResponse(ILibWebClient_StateObject WebStateObject,int InterruptFlag,struct packetheader *header,char *bodyBuffer,int *beginPointer,int endPointer,int done,void *user1,void *user2,int *PAUSE)
{
	struct DHC_Data *data = (struct DHC_Data*)user2;
	
	if(done)
	{
		if(data->f!=NULL && header!=NULL && header->StatusCode==100)
		{
			data->GotContinue=1;
			ILibThreadPool_QueueUserWorkItem(data->pool,data,&DHC_Pool);
			return;
		}
	}
	
	if(!done)
	{
		return;
	}

	//
	// Completed Sending the File
	//
	if(data->Callback!=NULL)
	{
		if(header!=NULL && InterruptFlag==0)
		{
			header->StatusData[header->StatusDataLength]=0;
			if(header->StatusCode==200)
			{
				data->Callback(data->TransferStatus, DHC_ERRORS_NONE, header->StatusCode, header->StatusData,data->user);
			}
			else
			{
				data->Callback(data->TransferStatus, DHC_ERRORS_HTTP, header->StatusCode, header->StatusData,data->user);
			}
		}
		else
		{
			data->Callback(data->token, DHC_ERRORS_CONNECTION_FAILED_OR_ABORTED, 0, NULL,data->user);
		}
	}

	if(data->f!=NULL)
	{
		fclose(data->f);
		data->f = NULL;
	}
	DH_DestroyTransferStatus(data->TransferStatus);
	free(data->buffer);
	free(data);
}

void DH_Pool_RequestResponse(ILibThreadPool sender, void *obj)
{
	struct DHC_Data *data = (struct DHC_Data*)obj;
	int okToFree = 0;

	if(data->bufferLength>0)
	{
		fwrite(data->buffer,sizeof(char),data->bufferLength,data->f);
	}

	sem_wait(&(data->TransferStatus->syncLock));
	data->TransferStatus->ActualBytesReceived += data->bufferLength;
	sem_post(&(data->TransferStatus->syncLock));

	if(data->GotContinue)
	{
		fclose(data->f);
		
		if(data->Callback!=NULL)
		{
			data->Callback(data->TransferStatus, DHC_ERRORS_NONE, 200, "OK",data->user);
		}
		okToFree = 1;
	}

	if(!okToFree)
	{
		sem_wait(&(data->TransferStatus->syncLock));
		if(data->TransferStatus->Reserved1!=0)
		{
			data->TransferStatus->Reserved2 = 1;
		}
		else
		{
			ILibWebClient_Resume(data->webState);
		}
		sem_post(&(data->TransferStatus->syncLock));
	}
	else
	{
		ILibWebClient_Resume(data->webState);
		DH_DestroyTransferStatus(data->TransferStatus);
		free(data);
	}
}
void DH_RequestResponse(ILibWebClient_StateObject WebStateObject,int InterruptFlag,struct packetheader *header,char *bodyBuffer,int *beginPointer,int endPointer,int done,void *user1,void *user2,int *PAUSE)
{
	struct DHC_Data *data = (struct DHC_Data*)user1;
	enum DHC_Errors status;
	void *Abort = NULL;
	char *rangeResult;
	struct parser_result *pr;
	long total;
	char *contentFeatures;
	struct DLNAProtocolInfo *pi;

	ILibReaderWriterLock rwLock = NULL;
	struct packetheader *GetRequest = NULL;
	struct sockaddr_in dest;

	if((header==NULL && done!=0) || (header!=NULL && header->StatusCode!=200 && header->StatusCode!=206 && done!=0))
	{
		fclose(data->f);
		if(header==NULL)
		{
			status = DHC_ERRORS_CONNECTION_FAILED_OR_ABORTED;
		}
		else
		{
			status = DHC_ERRORS_HTTP;
		}

		if(data->Callback!=NULL)
		{
			if(header!=NULL)
			{
				header->StatusData[header->StatusDataLength]=0;
			}
			data->Callback(data->TransferStatus, status, header==NULL?0:header->StatusCode, header==NULL?NULL:header->StatusData,data->user);
		}
		DH_DestroyTransferStatus(data->TransferStatus);
		if(data->ActualPacket!=NULL)
		{
			ILibDestructPacket(data->ActualPacket);
		}
		free(data);
	}
	else if(header!=NULL && (header->StatusCode==200 || header->StatusCode==206))
	{
		if(data->ActualPacket!=NULL)
		{
			if(data->TransferStatus->Reserved4!=0 && data->TransferStatus->RequestToken!=NULL)
			{
				//
				// Abort... Nothing really to do, because at this point we already finished.
				// Only thing to do, is to not actually issue the request.
				//
				data->TransferStatus->RequestToken = NULL;
			}
			else
			{
				rangeResult = ILibGetHeaderLine(header,"content-range",13);
				if(rangeResult!=NULL)
				{
					pr = ILibParseString(rangeResult,0,(int)strlen(rangeResult),"/",1);
					total = atol(pr->LastResult->data);
					ILibDestructParserResults(pr);
					total -= data->StartPosition;
					sem_wait(&(data->TransferStatus->syncLock));
					data->TransferStatus->TotalBytesToBeReceived = total;
					sem_post(&(data->TransferStatus->syncLock));
				}
				GetRequest = data->ActualPacket;
				data->ActualPacket = NULL;

				if(ILibGetHeaderLine(GetRequest,"transferMode.dlna.org",21)==NULL)
				{
					//
					// Transfer Mode wasn't specified
					//
					contentFeatures = ILibGetHeaderLine(header,"contentFeatures.dlna.org",24);
					if(contentFeatures!=NULL)
					{
						pi = DLNAProtocolInfo_Parse(contentFeatures,(int)strlen(contentFeatures));
						if(pi!=NULL)
						{
							if(pi->TM_B!=0)
							{
								DH_AddHeader_transferMode(GetRequest,DH_TransferMode_Bulk);
							}
							else if(pi->TM_I!=0)
							{
								DH_AddHeader_transferMode(GetRequest,DH_TransferMode_Interactive);
							}
							else if(pi->TM_S!=0)
							{
								DH_AddHeader_transferMode(GetRequest,DH_TransferMode_Streaming);
							}
							DLNAProtocolInfo_Destruct(pi);
						}
					}
				}
				
				rwLock = (ILibReaderWriterLock)ILibWebClient_GetUser(data->manager);
				memset(&dest,0,sizeof(struct sockaddr_in));
	
				dest.sin_family = AF_INET;
				dest.sin_addr.s_addr = header->Source->sin_addr.s_addr;
				dest.sin_port = header->Source->sin_port;

				ILibReaderWriterLock_WriteLock(rwLock);
				data->TransferStatus->RequestToken = data->token = ILibWebClient_PipelineRequest(data->manager,&dest,GetRequest,&DH_RequestResponse,data,NULL);
				ILibReaderWriterLock_WriteUnLock(rwLock);
			}
			*beginPointer = endPointer; // Don't really need to do this, because it should always be zero
			return;
		}


		data->buffer = bodyBuffer;
		data->bufferLength = endPointer;
		*beginPointer = endPointer;
		data->GotContinue = done;
		if(endPointer!=0 || done)
		{
			*PAUSE = 1;
		}
		data->webState = ILibWebClient_GetStateObjectFromRequestToken(data->token);
		if(data->TransferStatus->Reserved4!=0 && data->TransferStatus->RequestToken!=NULL)
		{
			//
			// Abort
			//
			Abort = data->TransferStatus->RequestToken;
			data->TransferStatus->RequestToken = NULL;
			ILibWebClient_CancelRequest(Abort);
		}
		else if(data->TransferStatus->RequestToken!=NULL)
		{
			if(endPointer>0 || done)
			{
				ILibThreadPool_QueueUserWorkItem(data->pool,data,&DH_Pool_RequestResponse);
			}
		}
	}
}

DH_TransferStatus DHC_IssueRequestAndSave(ILibWebClient_RequestManager request_manager, ILibThreadPool pool, const char *file_path, long append_flag, const char *target_uri, enum DH_TransferModes requestedTransferMode, void* user_obj, DHC_OnResponseDone callback_response)
{
	DH_TransferStatus RetVal = NULL;
	char *IP, *Path;
	unsigned short Port;
	struct sockaddr_in dest;
	char *host;
	int hostLen;

	struct packetheader *req;
	FILE *f;
	struct DHC_Data *data;
	ILibReaderWriterLock rwLock = (ILibReaderWriterLock)ILibWebClient_GetUser(request_manager);

	if(rwLock==NULL)
	{
		rwLock = ILibReaderWriterLock_CreateEx(ILibWebClient_GetChain(request_manager));
		ILibWebClient_SetUser(request_manager,rwLock);
	}
	
	memset(&dest,0,sizeof(struct sockaddr_in));

	if(append_flag==0)
	{
		f = fopen(file_path,"wb");
	}
	else 
	{
		f = fopen(file_path,"r+b");
	}
	
	if(f!=NULL)
	{
		data = (struct DHC_Data*)malloc(sizeof(struct DHC_Data));
		memset(data,0,sizeof(struct DHC_Data));

		data->f = f;
		data->manager = request_manager;
		data->Callback = callback_response;
		data->pool = pool;
		data->user = user_obj;
		data->TransferStatus = DH_CreateNewTransferStatus();
		RetVal = data->TransferStatus;
		data->rwLock = rwLock;

		req = ILibCreateEmptyPacket();
		ILibSetVersion(req,"1.1",3);

		ILibParseUri((char*)target_uri,&IP,&Port,&Path);
		dest.sin_addr.s_addr = inet_addr(IP);
		dest.sin_port = htons(Port);

		host = (char*)malloc((int)strlen(IP)+10);
		hostLen = sprintf(host,"%s:%u",IP,Port);

		ILibAddHeaderLine(req,"Host",4,host,hostLen);
		ILibSetDirective(req,"GET",3,Path,(int)strlen(Path));

		//
		// Look at the append_flag
		//
		if(append_flag==-1)
		{
			//
			// Move to the end of the file
			//
			fseek(f,0,SEEK_END);
			append_flag = ftell(f);
		}
		if(append_flag>0)
		{
			if(fseek(f,append_flag,SEEK_SET)!=0)
			{
				fseek(f,0,SEEK_END);
				append_flag = ftell(f);
			}
			
			DH_AddHeader_Range(req,append_flag,-1);
			data->StartPosition = append_flag;
		}
		if(requestedTransferMode!=DH_TransferMode_Unspecified)
		{
			DH_AddHeader_transferMode(req,requestedTransferMode);
		}

		data->ActualPacket = req;
		req = ILibCreateEmptyPacket();
		ILibSetVersion(req,"1.1",3);
		ILibAddHeaderLine(req,"Host",4,host,hostLen);
		ILibSetDirective(req,"HEAD",4,Path,(int)strlen(Path));
		DH_AddHeader_Range(req,0,-1);

		if(requestedTransferMode==DH_TransferMode_Unspecified)
		{
			//
			// Choose a transfer mode that is supported
			//
			ILibAddHeaderLine(req,"getcontentFeatures.dlna.org",27,"1",1);
		}

		ILibReaderWriterLock_WriteLock(rwLock);
		data->TransferStatus->RequestToken = data->token = ILibWebClient_PipelineRequest(request_manager,&dest,req,&DH_RequestResponse,data,NULL);
		ILibReaderWriterLock_WriteUnLock(rwLock);

		free(host);
		free(IP);
		free(Path);
	}
	return(RetVal);
}

DH_TransferStatus DHC_IssuePostRequestFromFile(ILibWebClient_RequestManager request_manager, ILibThreadPool pool, const char* target_uri, const char *file_path, int resume_pos, void* user_obj, DHC_OnResponseDone callback_response)
{
	DH_TransferStatus RetVal;
	struct packetheader *h;
	struct sockaddr_in addr;
	char host[22];
	struct DHC_Data *data = NULL;
	FILE *f;
	long fileLength;

	char *Ip;
	char *Path;
	unsigned short Port;
	ILibReaderWriterLock rwLock = (ILibReaderWriterLock)ILibWebClient_GetUser(request_manager);
	int extensionIndex;
	char *mimeType;

	if(rwLock==NULL)
	{
		rwLock = ILibReaderWriterLock_CreateEx(ILibWebClient_GetChain(request_manager));
		ILibWebClient_SetUser(request_manager,rwLock);
	}

	f = fopen(file_path,"rb");
	if(f==NULL)
	{
		//
		// Couldn't open file
		//
		return(NULL);
	}

	data = (struct DHC_Data*)malloc(sizeof(struct DHC_Data));
	memset(data,0,sizeof(struct DHC_Data));
	data->f = f;
	data->user = user_obj;
	data->Callback = callback_response;
	data->pool = pool;

	data->buffer = (char*)malloc(DHC_READ_BLOCK_SIZE);
	data->bufferLength = DHC_READ_BLOCK_SIZE;

	RetVal = data->TransferStatus = DH_CreateNewTransferStatus();
	data->rwLock = rwLock;

	ILibParseUri((char*)target_uri,&Ip,&Port,&Path);
	memset(&addr,0,sizeof(struct sockaddr_in));
	addr.sin_addr.s_addr = inet_addr(Ip);
	addr.sin_port = htons(Port);

	sprintf(host,"%s:%u",Ip,Port);

	h = ILibCreateEmptyPacket();
	ILibSetVersion(h,"1.1",3);
	ILibSetDirective(h,"POST",4,Path,(int)strlen(Path));
	ILibAddHeaderLine(h,"Host",4,host,(int)strlen(host));
	ILibAddHeaderLine(h,"Expect",6,"100-Continue",12);
	extensionIndex = ILibString_LastIndexOf(file_path,(int)strlen(file_path),".",1);
	if(extensionIndex>0)
	{
		mimeType = (char*)FileExtensionToMimeType(file_path+extensionIndex,0);
		ILibAddHeaderLine(h,"Content-Type",12,mimeType,(int)strlen(mimeType));
	}
	if(resume_pos>0)
	{
		fseek(f,0,SEEK_END);
		fileLength = ftell(f);
		fseek(f,(long)resume_pos,SEEK_SET);
		DH_AddHeader_ContentRange(h,resume_pos,fileLength,fileLength);
	}

	ILibReaderWriterLock_WriteLock(rwLock);
	data->token = data->TransferStatus->RequestToken = ILibWebClient_PipelineStreamedRequest(request_manager,&addr,h,&DHC_OnResponse,&DHC_OnSendOK,NULL,data);
	ILibReaderWriterLock_WriteUnLock(rwLock);

	free(Ip);
	free(Path);
	return(RetVal);
}
void DHC_Pause(DH_TransferStatus tstatus)
{
	sem_wait(&(tstatus->syncLock));
	tstatus->Reserved1=1;
	sem_post(&(tstatus->syncLock));
}
void DHC_Resume(DH_TransferStatus tstatus)
{
	int paused = 0;
	int ok = 0;
	struct DHC_Data* data = NULL;
	void *token = NULL;
	ILibWebClient_StateObject WebState=NULL;

	sem_wait(&(tstatus->syncLock));
	paused = tstatus->Reserved2;
	tstatus->Reserved2 = 0;
	tstatus->Reserved1 = 0;
	token = tstatus->RequestToken;
	data = (struct DHC_Data*)tstatus->Reserved3;

	if(data!=NULL && data->f!=NULL && data->GotContinue!=0)
	{
		ok = 1;
	}
	if(data==NULL)
	{
		ok = 1;
	}
	sem_post(&(tstatus->syncLock));

	if(paused)
	{
		if(ok && data!=NULL)
		{
			ILibThreadPool_QueueUserWorkItem(data->pool,data,&DHC_Pool);
		}
		else if(ok && data==NULL)
		{
			WebState = ILibWebClient_GetStateObjectFromRequestToken(token);
			ILibWebClient_Resume(WebState);
		}
	}
}
