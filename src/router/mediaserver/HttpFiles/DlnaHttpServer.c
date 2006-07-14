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
 * $Workfile: DlnaHttpServer.c
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
#elif defined(WINSOCK1)
	#include <winsock.h>
#endif

#include "ILibParsers.h"
#include "DlnaHttpServer.h"
#include "ILibWebClient.h"


struct DH_Data
{
	FILE *f;
	long BytesLeft;

	struct packetheader *header;
	struct ILibWebServer_Session *session;
	ILibThreadPool pool;
	DHS_OnResponseDone callback_response;

	DH_TransferStatus TransferStatus;
	int SendStatus;
	sem_t SendStatusLock;
	int Disconnect;

	void *OriginalReceivePtr;
	void *user_object;
};

void DH_Pool(ILibThreadPool,void*);

void DH_UpdateProtocolInfoOptions(struct DLNAProtocolInfo *inputInfo)
{
	inputInfo->DLNA_Major_Version=1;
	inputInfo->DLNA_Minor_Version=5;
	inputInfo->HTTP_Stalling = 1;
	inputInfo->SupportsByteBasedSeek = 1;
	inputInfo->Protocol = DLNAProtocolInfo_ProtocolType_HTTP;

	inputInfo->TM_B=1;
	inputInfo->TM_I=1;
	inputInfo->TM_S=1;
}
void DHS_OnDisconnect(struct ILibWebServer_Session *session)
{
	struct DH_Data *data = (struct DH_Data*)session->User3;
	if(session->User3!=NULL)
	{
		//
		// The session was aborted
		//
		if(data->callback_response!=NULL)
		{
			data->callback_response(session,data->TransferStatus,DHS_ERRORS_PEER_ABORTED_CONNECTION,data->user_object);
		}
		if(data->f!=NULL)
		{
			fclose(data->f);
			data->f = NULL;
		}
		DH_DestroyTransferStatus(data->TransferStatus);
		free(session->User3);
		session->User3=NULL;
	}
}
void DH_SendOK(struct ILibWebServer_Session *session)
{
	struct DH_Data *data = (struct DH_Data*)session->User3;

	session->OnSendOK = NULL;
	ILibThreadPool_QueueUserWorkItem(data->pool,data,&DH_Pool);
}
void DH_Disconnect(struct ILibWebServer_Session *session)
{
	struct DH_Data *data = (struct DH_Data*)session->User3;
	int SendStatus = 0;

	if(data!=NULL)
	{
		sem_wait(&(data->SendStatusLock));
		SendStatus = data->SendStatus;
		sem_post(&(data->SendStatusLock));
		if(SendStatus>0 || SendStatus<0)
		{
			if(data->f!=NULL)
			{
				if(data->callback_response!=NULL)
				{
					data->callback_response(data->session, data->TransferStatus, DHS_ERRORS_PEER_ABORTED_CONNECTION,data->user_object);
				}
				fclose(data->f);
				data->f = NULL;
			}
			DH_DestroyTransferStatus(data->TransferStatus);
			sem_destroy(&(data->SendStatusLock));
			free(data);
		}
		else
		{
			data->Disconnect=1;
		}
	}
}

void DH_Pool(ILibThreadPool sender, void *var)
{
	struct DH_Data *data = (struct DH_Data*)var;
	char *buffer = (char*)malloc(DHS_READ_BLOCK_SIZE*sizeof(char));
	int bytesRead = 0;
	int paused=0;
	int SendStatus=0;

	sem_wait(&(data->TransferStatus->syncLock));
	if(data->TransferStatus->Reserved1!=0)
	{
		data->TransferStatus->Reserved2 = 2;
		data->TransferStatus->Reserved3 = data;
		paused=1;
		free(buffer);
	}
	sem_post(&(data->TransferStatus->syncLock));

	if(paused)
	{
		printf("PAUSED!\r\n");
		return;
	}

	if(data->TransferStatus->TotalBytesToBeSent==-1)
	{
		sem_wait(&(data->TransferStatus->syncLock));
		fseek(data->f,0,SEEK_END);
		data->TransferStatus->TotalBytesToBeSent = ftell(data->f);
		fseek(data->f,0,SEEK_SET);
		sem_post(&(data->TransferStatus->syncLock));
	}

	bytesRead = (int)fread(buffer,sizeof(char),data->BytesLeft>DHS_READ_BLOCK_SIZE?DHS_READ_BLOCK_SIZE:data->BytesLeft,data->f);

	if(bytesRead>0)
	{
		data->BytesLeft -= bytesRead;
		data->session->User3 = data;
		data->session->OnSendOK = &DH_SendOK;
		data->session->OnDisconnect = &DH_Disconnect;

		sem_wait(&(data->SendStatusLock));
		SendStatus = data->SendStatus = ILibWebServer_StreamBody(data->session,buffer,bytesRead,ILibAsyncSocket_MemoryOwnership_CHAIN,0);
		sem_post(&(data->SendStatusLock));

		if(SendStatus>=0)
		{
			sem_wait(&(data->TransferStatus->syncLock));
			data->TransferStatus->ActualBytesSent += (long)bytesRead;
			sem_post(&(data->TransferStatus->syncLock));
		}
	}
	else
	{
		//
		// Read Error
		//
		free(buffer);
		printf("Read Error!\r\n");
	}

	if(SendStatus>=0)
	{
		if(feof(data->f) || data->BytesLeft==0)
		{
			//
			// Read all there is to read
			//
			data->session->OnSendOK = NULL;
			data->session->OnDisconnect = NULL;
			data->session->User3 = NULL;
			ILibWebServer_StreamBody(data->session,NULL,0,ILibAsyncSocket_MemoryOwnership_STATIC,1);
			//
			// Call the OnDone method
			//
			if(data->callback_response!=NULL)
			{
				data->callback_response(data->session, data->TransferStatus, DHS_ERRORS_NONE,data->user_object);
			}
			fclose(data->f);
			DH_DestroyTransferStatus(data->TransferStatus);
			sem_destroy(&(data->SendStatusLock));
			free(data);
		}
		else
		{
			//
			// More to read
			//
			
			if(data->SendStatus==0 && data->Disconnect==0)
			{
				//
				// Everything was sent, just need to read more
				//
				ILibThreadPool_QueueUserWorkItem(data->pool,data,&DH_Pool);
			}
			else if(data->SendStatus==0 && data->Disconnect!=0)
			{
				//
				// Clean up everything, because the session was disconnected
				//
				if(data->f!=NULL)
				{
					if(data->callback_response!=NULL)
					{
						data->callback_response(data->session, data->TransferStatus, DHS_ERRORS_PEER_ABORTED_CONNECTION,data->user_object);
					}
					fclose(data->f);
					data->f = NULL;
				}
				DH_DestroyTransferStatus(data->TransferStatus);
				sem_destroy(&(data->SendStatusLock));
				free(data);
			}

			//
			// if sendStatus > 0 that means we couldn't send everything,
			// and must wait for the SendOK event to continue.
			//
		}	
	}
}

DH_TransferStatus DHS_RespondWithLocalFile(struct ILibWebServer_Session *session, ILibThreadPool pool, struct packetheader *header, size_t buffer_size, const char *file_name, enum DH_TransferModes min_transfer_mode, enum DH_TransferModes max_transfer_mode, const char *mime_type, const char *protocol_info_fourth_field, const char* ifo_uri, void *user_obj, DHS_OnResponseDone callback_response)
{
	DH_TransferStatus retval = NULL;
	FILE *f;
	struct DH_Data *data = NULL;
	struct packetheader *resp = NULL;
	char *ifo = NULL;
	long RangeStart,RangeLength,FileLength;
	char len[255];
	enum ILibWebClient_Range_Result RangeResult = 0;
	enum DH_TransferModes transferMode;

	f = fopen(file_name,"rb");
	if(f!=NULL)
	{
		data = (struct DH_Data*)malloc(sizeof(struct DH_Data));
		memset(data,0,sizeof(struct DH_Data));

		resp = ILibCreateEmptyPacket();
		ILibSetVersion(resp,"1.1",3);
		
		fseek(f,0,SEEK_END);
		FileLength = ftell(f);
		fseek(f,0,SEEK_SET);

		if(ILibGetHeaderLine(header,"Range",5)!=NULL)
		{
			RangeResult = ILibWebClient_Parse_Range(ILibGetHeaderLine(header,"Range",5),&RangeStart,&RangeLength,FileLength);
			switch(RangeResult)
			{
				case ILibWebClient_Range_Result_OK:
					fseek(f,RangeStart,SEEK_SET);
					data->BytesLeft = RangeLength;
					ILibSetStatusCode(resp,206,"Partial Content",15);
					DH_AddHeader_ContentRange(resp,RangeStart,(RangeStart+RangeLength)-1,FileLength);
					break;
				case ILibWebClient_Range_Result_INVALID_RANGE:								 
					ILibWebServer_Send_Raw(session,"HTTP/1.1 416 Invalid Range\r\n\r\n",30,ILibAsyncSocket_MemoryOwnership_STATIC,1);
					fclose(f);
					free(data);
					ILibDestructPacket(resp);
					return(NULL);
					break;
				case ILibWebClient_Range_Result_BAD_REQUEST:
					ILibWebServer_Send_Raw(session,"HTTP/1.1 400 Bad Request\r\n\r\n",28,ILibAsyncSocket_MemoryOwnership_STATIC,1);
					fclose(f);
					free(data);
					ILibDestructPacket(resp);
					return(NULL);
					break;
			}
		}
		else if(ILibGetHeaderLine(header,"timeSeekRange.dlna.org",22)!=NULL)
		{
			ILibWebServer_Send_Raw(session,"HTTP/1.1 406 Time-based seek not supported\r\n\r\n",46,ILibAsyncSocket_MemoryOwnership_STATIC,1);
			fclose(f);
			free(data);
			ILibDestructPacket(resp);
			return(NULL);
		}
		else
		{
			ILibSetStatusCode(resp,200,"OK",2);
			data->BytesLeft = FileLength;
			sprintf(len,"%ld",data->BytesLeft);
			ILibAddHeaderLine(resp,"Content-Length",14,len,(int)strlen(len));
		}

		transferMode = DH_GetRequestedTransferMode(header);
		if(transferMode != DH_TransferMode_Unspecified && transferMode < min_transfer_mode)
		{
			//
			// Specified TransferMode is too low
			//
			ILibWebServer_Send_Raw(session,"HTTP/1.1 406 Not Acceptable\r\n\r\n",31,ILibAsyncSocket_MemoryOwnership_STATIC,1);
			fclose(f);
			free(data);
			ILibDestructPacket(resp);
			return(NULL);
		}
		else if(transferMode!= DH_TransferMode_Unspecified && transferMode > max_transfer_mode)
		{
			ILibWebServer_Send_Raw(session,"HTTP/1.1 406 Not Acceptable\r\n\r\n",31,ILibAsyncSocket_MemoryOwnership_STATIC,1);
			fclose(f);
			free(data);
			ILibDestructPacket(resp);
			return(NULL);
		}
		else if(transferMode != DH_TransferMode_Unspecified)
		{
			DH_AddHeader_transferMode(resp,transferMode);	
		}

		data->callback_response = callback_response;
		data->f = f;
		data->header = header;
		data->session = session;
		data->pool = pool;
		data->TransferStatus = retval = DH_CreateNewTransferStatus();
		data->TransferStatus->TotalBytesToBeSent = data->BytesLeft;
		data->user_object = user_obj;
		retval->ServerSession = session;
		session->OnDisconnect = &DH_Disconnect;
		
		if(protocol_info_fourth_field!=NULL)
		{
			ILibAddHeaderLine(resp,"contentFeatures.dlna.org",24,(char*) protocol_info_fourth_field,(int)strlen(protocol_info_fourth_field));
		}
		if(mime_type!=NULL)
		{
			ILibAddHeaderLine(resp,"Content-Type",12,(char*)mime_type,(int)strlen(mime_type));
		}
		else
		{
			ILibAddHeaderLine(resp,"Content-Type",12,(char*)DHS_DEFAULT_MIMETYPE,(int)strlen(DHS_DEFAULT_MIMETYPE));
		}
		if(ifo_uri!=NULL)
		{
			ifo = (char*)malloc((int)strlen(ifo_uri)+25);
			sprintf(ifo,"ifoFileURI.dlna.org=\"%s\"",ifo_uri);
			ILibAddHeaderLine(resp,"PRAGMA",6,ifo,(int)strlen(ifo));
			free(ifo);
		}

		ILibWebServer_StreamHeader(session,resp);

		if(header->DirectiveLength==4 && strncasecmp(header->Directive,"HEAD",4)==0)
		{
			ILibWebServer_StreamBody(session,NULL,0,ILibAsyncSocket_MemoryOwnership_STATIC,1);
			fclose(data->f);
			data->f = NULL;
			DH_DestroyTransferStatus(data->TransferStatus);
			free(data);
			session->OnDisconnect = NULL;
			return(NULL);
		}
		else
		{
			sem_init(&(data->SendStatusLock),0,1);
			ILibThreadPool_QueueUserWorkItem(pool,data,&DH_Pool);
		}
	}
	else
	{
		//
		// Error opening file
		//
		ILibWebServer_Send_Raw(session,"HTTP/1.1 404 File Not Found\r\n\r\n",31,ILibAsyncSocket_MemoryOwnership_STATIC,1);
	}
	return retval;
}
void DH_Pool_SavePostToLocalFile(ILibThreadPool pool, void *var)
{
	struct DH_Data *data = (struct DH_Data*)var;
	struct packetheader *resp;
	struct ILibWebServer_Session *session = data->session;
	int paused=0;

	sem_wait(&(data->TransferStatus->syncLock));
	if(data->TransferStatus->Reserved1!=0)
	{
		data->TransferStatus->Reserved2 = 1;
		data->TransferStatus->Reserved3 = data;
		paused=1;
	}
	sem_post(&(data->TransferStatus->syncLock));

	if(paused)
	{
		ILibWebServer_Release(session);
		return;
	}

	if(data->session->bufferLength!=0)
	{
		fwrite(data->session->buffer,sizeof(char),data->session->bufferLength,data->f);
	}

	sem_wait(&(data->TransferStatus->syncLock));
	data->TransferStatus->ActualBytesReceived += data->session->bufferLength;
	sem_post(&(data->TransferStatus->syncLock));

	if(data->session->done!=0)
	{
		fclose(data->f);
		data->f = NULL;
		//
		// Finished Saving file
		//
		resp = ILibCreateEmptyPacket();
		ILibSetVersion(resp,"1.1",3);
		ILibSetStatusCode(resp,200,"OK",2);

		ILibWebServer_Send(data->session,resp);
		data->session->User3 = NULL;
		data->session->Reserved7 = NULL;
		data->session->OnReceive = data->OriginalReceivePtr;

		if(data->callback_response!=NULL)
		{
			data->callback_response(data->session,data->TransferStatus,DHS_ERRORS_NONE,data->user_object);
		}
		DH_DestroyTransferStatus(data->TransferStatus);
		free(data);
	}

	ILibWebServer_Resume(session);
	ILibWebServer_Release(session);
}
void DHS_OnReceive_SavePostToLocalFile(struct ILibWebServer_Session *sender, int InterruptFlag, struct packetheader *header, char *bodyBuffer, int *beginPointer, int endPointer, int done)
{
	struct DH_Data *data = (struct DH_Data*)sender->User3;
	ILibWebServer_Pause(sender);

	*beginPointer = endPointer;
	ILibWebServer_AddRef(sender);
	ILibThreadPool_QueueUserWorkItem(data->pool,data,&DH_Pool_SavePostToLocalFile);
}
DH_TransferStatus DHS_SavePostToLocalFile(struct ILibWebServer_Session *session, ILibThreadPool pool, struct packetheader *header, const char *file_name, int append_flag, void *user_obj, DHS_OnResponseDone callback_response)
{
	DH_TransferStatus RetVal = NULL;
	char *hdr_field = NULL;
	int temp = 0;
	FILE *f;
	struct DH_Data *data = NULL;

	int Start,End,TotalLength;

	//
	// Check the append flag
	//
	if(append_flag==0 && ILibGetHeaderLine(header,"content-range",13)!=NULL)
	{
		//
		// append_flag indicates overwrite, but the header's indicate partial content
		// this is not allowed
		//
		ILibWebServer_Send_Raw(session,"HTTP/1.1 406 Not Acceptable\r\n\r\n",31,ILibAsyncSocket_MemoryOwnership_STATIC,1);
		ILibWebServer_DisconnectSession(session);
		return(NULL);
	}

	if(append_flag==0)
	{
		f = fopen(file_name,"wb");
	}
	else 
	{
		f = fopen(file_name,"r+b");
	}
	
	if(f==NULL)
	{
		//
		// Error
		//											 
		ILibWebServer_Send_Raw(session,"HTTP/1.1 404 File not found\r\n\r\n",31,ILibAsyncSocket_MemoryOwnership_STATIC,1);
		ILibWebServer_DisconnectSession(session);
		return(NULL);
	}

	data = (struct DH_Data*)malloc(sizeof(struct DH_Data));
	memset(data,0,sizeof(struct DH_Data));

	data->callback_response = callback_response;
	data->f = f;
	data->header = header;
	data->session = session;
	data->pool = pool;
	data->OriginalReceivePtr = session->OnReceive;
	data->user_object = user_obj;
	RetVal = data->TransferStatus = DH_CreateNewTransferStatus();
	RetVal->ServerSession = session;
	RetVal->SessionFlag = 1;

	ILibWebServer_OverrideReceiveHandler(session,&DHS_OnReceive_SavePostToLocalFile);
	session->User3 = data;
	session->OnDisconnect = &DHS_OnDisconnect;
	
	//
	// Check to see if we need to advance the file pointer
	//
	if(append_flag!=0)
	{
		hdr_field = ILibGetHeaderLine(header,"content-range",13);
		if(hdr_field==NULL)
		{
			//
			// Just append to the end of the file
			//
			fseek(f,0,SEEK_END);
		}
		else
		{
			//
			// Advance the file pointer the specified amount
			//
			ILibWebClient_Parse_ContentRange(hdr_field,&Start,&End,&TotalLength);
			fseek(f,Start,SEEK_SET);
		}
	}


	if(session->done!=0)
	{
		//
		// All the data is here, so OnReceive won't be called anymore
		//
		DHS_OnReceive_SavePostToLocalFile(session,0,header,session->buffer,&temp,session->bufferLength,session->done);
	}
	else
	{
		hdr_field = ILibGetHeaderLine(header,"Content-Length",14);
		if(hdr_field!=NULL)
		{
			data->TransferStatus->TotalBytesToBeReceived = atol(hdr_field);
		}
		//
		// Check to see if we are supposed to send a 100 Continue response
		//
		hdr_field = ILibGetHeaderLine(header,"expect",6);
		if(hdr_field != NULL && strcasecmp(hdr_field,"100-continue")==0)
		{
			//
			// Send 100-Continue
			//
			ILibWebServer_Send_Raw(session,"HTTP/1.1 100 Continue\r\n\r\n",25,ILibAsyncSocket_MemoryOwnership_STATIC,0);
		}
		else if(hdr_field != NULL)
		{
			//
			// We don't recognize the expectation
			//
			ILibWebServer_Send_Raw(session,"HTTP/1.1 417 Expectation Failed\r\n\r\n",35,ILibAsyncSocket_MemoryOwnership_STATIC,1);
			ILibWebServer_DisconnectSession(session);
			RetVal = NULL;
		}
	}
	return(RetVal);
}
void DH_Pause(DH_TransferStatus tstatus)
{
	sem_wait(&(tstatus->syncLock));
	tstatus->Reserved1=1;
	sem_post(&(tstatus->syncLock));
}
void DH_Resume(DH_TransferStatus tstatus)
{
	int WhatToDo = 0;
	struct DH_Data* data = NULL;

	sem_wait(&(tstatus->syncLock));
	WhatToDo = tstatus->Reserved2;
	data = (struct DH_Data*)tstatus->Reserved3;
	data->TransferStatus->Reserved1=0;
	data->TransferStatus->Reserved2=0;
	data->TransferStatus->Reserved3=NULL;
	sem_post(&(tstatus->syncLock));

	switch(WhatToDo)
	{
		case 1:
			ILibThreadPool_QueueUserWorkItem(data->pool,data,&DH_Pool_SavePostToLocalFile);
			break;
		case 2:
			ILibThreadPool_QueueUserWorkItem(data->pool,data,&DH_Pool);
			break;
	}
}

