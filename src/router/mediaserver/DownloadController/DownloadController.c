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
 * $Workfile: DownloadController.c
 *
 *
 *
 */

#include "DownloadController.h"
#include "DlnaHttpClient.h"	
#include "DLNAProtocolInfo.h"
#include "FileIoAbstraction.h"

#ifndef _WIN32_WCE
	#include <assert.h>
	#define ASSERT(x) assert(x)
#endif

ILibWebClient_RequestManager	DMD_HttpClient = NULL;
#define HTTP_CLIENT_POOL_SIZE	5
#define DMD_MAX_CONNECTIONS_PER_SERVER 2

static void *ILib_Pool;

struct DMD_DownloadSession
{
	int RefCount;

	ILibWebClient_RequestManager HttpClient;

	DMD_OnDownloadResponse OnResponse;

	int IsResumeSupported;								//incdicate if the server supports byte-based seek

	int PausedFlag;										//indicate if this transfer is paused by disconnecting

	int PausedByFlowControlFlag;						//indicate if this transfer is paused using TCP flow control
			
	int ResumeFlag;										//indicate if this is a resumed transfer after aborting

	int BytesReceived;									//indicate bytes received on this transfer

	int TotalBytesExpected;								//indicate total bytes expected to transfer

	DH_TransferStatus TransferStatus;					//transfer status object

	char* TargetUri;									//target url of the downloading file

	char* DownloadToPath;								//file path of the downloading file

	void* UserObj;										//user object specified in application
};

void DMD_AddRef(DMD_Session session)
{
	struct DMD_DownloadSession* download_session;
	if(session!=NULL)
	{
		download_session = (struct DMD_DownloadSession*) session;
		download_session->RefCount++;
	}
}

void DMD_Release(DMD_Session session)
{
	struct DMD_DownloadSession* download_session;
	if(session!=NULL)
	{
		download_session = (struct DMD_DownloadSession*) session;
		download_session->RefCount--;
		if(download_session->RefCount<0)
		{
			download_session->HttpClient = NULL;
			download_session->OnResponse = NULL;
			download_session->TransferStatus = NULL;
			free(download_session->TargetUri);
			free(download_session->DownloadToPath);
			free(download_session);
		}
	}
}

void OnHttpGetResponseDone(DH_TransferStatus transfer_status_handle, enum DHC_Errors dhc_error_code, int http_error_code, const char* error_message, void *user_obj)
{
	struct DMD_DownloadSession* session;
	enum DMD_Errors errorCode = DMD_ERROR_Unknown;
	long actualBytesReceived = -1;

	if(user_obj!=NULL)
	{
		session = (struct DMD_DownloadSession*) user_obj;

		/* update transfer status */

		if(session->TransferStatus!=NULL)
		{
			DH_QueryTransferStatus(session->TransferStatus, NULL, NULL, &actualBytesReceived, NULL);	
			if(actualBytesReceived > 0)
			{

				if(session->ResumeFlag != 0)
				{
					session->BytesReceived += actualBytesReceived;
					/* clear TransferStatus */
					session->TransferStatus = NULL;
				}
				else
				{
					session->BytesReceived = actualBytesReceived;
				}
			}
		}

		switch(dhc_error_code)
		{
			case DHC_ERRORS_NONE:
				errorCode = DMD_ERROR_None;
				printf("Http download finished.");
				break;
			case DHC_ERRORS_CONNECTION_FAILED_OR_ABORTED:
				errorCode = DMD_ERROR_FailedOrAborted;	
				break;
			case DHC_ERRORS_HTTP:
				errorCode = DMD_ERROR_HttpError;	
				break;
			default:
				errorCode = DMD_ERROR_Unknown;
		}

		if(errorCode == DMD_ERROR_FailedOrAborted &&  session->PausedFlag!=0)
		{
			/* if this session is aborted because using Pause, then it is okay to resume */
			session->ResumeFlag = 0;
		}

		session->OnResponse(session, errorCode, http_error_code, session->UserObj);
		DMD_Release(session);
	}

}

DMD_Session DMD_DownloadObject(const struct CdsResource* download_this, int resume_flag, char* save_as_file_name, DMD_OnDownloadResponse callback_response, void* user_obj)
{
	struct DMD_DownloadSession* session;
	struct DLNAProtocolInfo* protocolInfo;
	int fileSize = 0;

	if(DMD_HttpClient==NULL)
	{
		return NULL;
	}

	if(download_this != NULL && download_this->Value != NULL && download_this->ProtocolInfo != NULL)
	{
		session = (struct DMD_DownloadSession*) malloc(sizeof(struct DMD_DownloadSession));
		memset(session,0,sizeof(struct DMD_DownloadSession));
		protocolInfo = DLNAProtocolInfo_Parse(download_this->ProtocolInfo, (int) strlen(download_this->ProtocolInfo));
		if(protocolInfo->SupportsByteBasedSeek == 1)
		{
			session->IsResumeSupported = 1;
		}

		session->OnResponse = callback_response;
		session->HttpClient = DMD_HttpClient;
		session->DownloadToPath = (char*) malloc(strlen(save_as_file_name) + 1);
		strcpy(session->DownloadToPath, save_as_file_name);
		session->TargetUri = (char*) malloc(strlen(download_this->Value) + 1);
		strcpy(session->TargetUri, download_this->Value);
		if(resume_flag == 0)
		{
			session->TransferStatus = DHC_IssueRequestAndSave(DMD_HttpClient, ILib_Pool, session->DownloadToPath, 0, session->TargetUri, DH_TransferMode_Unspecified, session, OnHttpGetResponseDone); 
		}
		else
		{
			fileSize = ILibFileDir_GetFileSize(session->DownloadToPath);
			session->TransferStatus = DHC_IssueRequestAndSave(DMD_HttpClient, ILib_Pool, session->DownloadToPath, fileSize, session->TargetUri, DH_TransferMode_Unspecified, session, OnHttpGetResponseDone); 
		}
		session->UserObj = user_obj;

		DLNAProtocolInfo_Destruct(protocolInfo);
		return session;
	}

	return NULL;
}


void DMD_Initialize(void* chain, void* thread_pool)
{
	ASSERT(chain != NULL && thread_pool!=NULL);

	ILib_Pool = thread_pool;

	//Create a web client to do HTTP posts
	DMD_HttpClient = ILibCreateWebClient(HTTP_CLIENT_POOL_SIZE,chain);
	ILibWebClient_SetMaxConcurrentSessionsToServer(DMD_HttpClient, DMD_MAX_CONNECTIONS_PER_SERVER);
}

int DMD_IsResumeSupported(DMD_Session session)
{
	struct DMD_DownloadSession* download_session = (struct DMD_DownloadSession*) session;

	/* need to examine the ProtocolInfo 4th field to see if the server supports byte-based seek */
	return download_session->IsResumeSupported;
}

int DMD_GetBytesReceived(DMD_Session session)
{
	struct DMD_DownloadSession* download_session = (struct DMD_DownloadSession*) session;
	long bytesReceived = 0;
	long actualBytesReceived = -1;

	if(download_session != NULL)
	{
		if(download_session->ResumeFlag != 0 || download_session->PausedFlag !=0)
		{
			/* this is a resumed session */
			bytesReceived = download_session->BytesReceived;
		}

		if(download_session->TransferStatus != NULL)
		{

			DH_QueryTransferStatus(download_session->TransferStatus, NULL, NULL, &actualBytesReceived, NULL);
			if(actualBytesReceived > 0)
			{	
				bytesReceived += actualBytesReceived;
			}
		}
	}
	return bytesReceived;
}

int DMD_GetTotalBytesExpected(DMD_Session session)
{
	struct DMD_DownloadSession* download_session = (struct DMD_DownloadSession*) session;
	long totalBytes = -1;
	long totalBytesExpected = -1;

	if(download_session != NULL)
	{
		if(download_session->ResumeFlag != 0 || download_session->PausedFlag !=0)
		{
			/* this is a resumed session */
			totalBytes = download_session->TotalBytesExpected;
		}
		else if(download_session->TransferStatus != NULL)
		{
			DH_QueryTransferStatus(download_session->TransferStatus, NULL, NULL, NULL, &totalBytesExpected);
			if(totalBytesExpected>0)
			{
				totalBytes = totalBytesExpected;
			}
		}
	}

	return totalBytes;
}

int DMD_IsPaused(DMD_Session session)
{
	struct DMD_DownloadSession* download_session = (struct DMD_DownloadSession*) session;
	int retVal = 0;

	if(download_session != NULL)
	{
		retVal = download_session->PausedFlag;
	}

	return retVal;
}

int DMD_IsPausedByFlowControl(DMD_Session session)
{
	struct DMD_DownloadSession* download_session = (struct DMD_DownloadSession*) session;
	int retVal = 0;

	if(download_session != NULL)
	{
		retVal = download_session->PausedByFlowControlFlag;
	}

	return retVal;
}

int	DMD_PauseTransfer(DMD_Session session)
{
	struct DMD_DownloadSession* download_session = (struct DMD_DownloadSession*) session;
	int retVal = -1;

	if((download_session != NULL) && (download_session->PausedFlag == 0) && download_session->PausedByFlowControlFlag==0)
	{
		retVal = DMD_AbortTransfer(session);
		
		if(retVal == 0)
		{
			/* aborted successfully */

			download_session->PausedFlag = 1;
		}
	}
	return retVal;
}


int DMD_PauseTransferByFlowControl(DMD_Session session)
{
	struct DMD_DownloadSession* download_session = (struct DMD_DownloadSession*) session;
	long actualBytesReceived = -1;

	if((download_session != NULL) && (download_session->PausedFlag == 0) && (download_session->PausedByFlowControlFlag==0))
	{
		/* this session was not paused before or aborted */

		if(download_session->TransferStatus!=NULL)
		{
			DH_QueryTransferStatus(download_session->TransferStatus, NULL, NULL, &actualBytesReceived, NULL);

			if(actualBytesReceived > 0)
			{
				/* pause using TCP flow control */
				DHC_Pause(download_session->TransferStatus);
				download_session->PausedByFlowControlFlag = 1;
				return 0;
			}
		}

	}
	return -1;
}

int DMD_ResumeTransfer(DMD_Session session)
{
	struct DMD_DownloadSession* download_session = (struct DMD_DownloadSession*) session;

	/* make sure connection is not paused using TCP flow control */
	if((download_session != NULL) && (download_session->PausedByFlowControlFlag==0))
	{

		/*
		 * this session was aborted, add ref to the session because it will be released by the callback
		 */
		DMD_AddRef(session);

		download_session->ResumeFlag = 1;
		download_session->PausedFlag = 0;

		download_session->TransferStatus = DHC_IssueRequestAndSave(
			download_session->HttpClient,
			ILib_Pool,
			download_session->DownloadToPath,
			-1,
			download_session->TargetUri,
			DH_TransferMode_Unspecified,
			download_session,
			OnHttpGetResponseDone);

		return 0;
	}
	return -1;
}

int DMD_ResumeTransferByFlowControl(DMD_Session session)
{
	struct DMD_DownloadSession* download_session = (struct DMD_DownloadSession*) session;
	long actualBytesReceived = -1;

	if((download_session != NULL) && (download_session->PausedFlag == 0) && (download_session->PausedByFlowControlFlag==1))
	{
		/* this session was paused using TCP flow control */

		if(download_session->TransferStatus!=NULL)
		{
			DH_QueryTransferStatus(download_session->TransferStatus, NULL, NULL, &actualBytesReceived, NULL);

			if(actualBytesReceived > 0)
			{
				/* pause using TCP flow control */
				DHC_Resume(download_session->TransferStatus);
				download_session->PausedByFlowControlFlag = 0;
				return 0;
			}
		}

	}
	return -1;
}

int DMD_AbortTransfer(DMD_Session session)
{
	struct DMD_DownloadSession* download_session = (struct DMD_DownloadSession*) session;
	long actualBytesReceived = -1;

	if(download_session != NULL)
	{

		if(download_session->PausedFlag!=0)
		{
			download_session->PausedFlag = 0;
			/* this transfer was paused by disconnecting, therefore the connection is already closed */
			download_session->OnResponse(download_session, DMD_ERROR_FailedOrAborted, 0, download_session->UserObj);
			DMD_Release(session);
			return 0;
		}
		else
		{
			if(download_session->TransferStatus==NULL)
			{
				return -1;
			}
				DH_QueryTransferStatus(download_session->TransferStatus, NULL, NULL, &actualBytesReceived, NULL);

			if(actualBytesReceived > 0)
			{
				DH_AbortTransfer(download_session->TransferStatus);
				return 0;
			}
		}
	}
	return -1;
}
