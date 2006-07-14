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
 * $Workfile: DlnaHttp.c
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

#include <stdlib.h>
#include <string.h>

#if defined(WINSOCK2)
#include <winsock2.h>
#elif defined(WINSOCK1)
#include <winsock.h>
#endif
#include "DlnaHttp.h"
#include "ILibWebClient.h"
#include "ILibWebServer.h"

int DH_AddHeader(struct packetheader* http_headers, const char* header_name, int header_name_len, const char* header_value, int header_value_len)
{
	/* TODO: DH_AddHeader */

	int retval = 0;

	return retval;
}

int DH_AddHeader_transferMode(struct packetheader* http_headers, enum DH_TransferModes transfer_mode)
{
	/* TODO: DH_AddHeader_transferMode */

	int retval = 0;

	switch(transfer_mode)
	{
	case DH_TransferMode_Bulk:
		ILibAddHeaderLine(http_headers,"transferMode.dlna.org",21,"Background",10);
		break;
	case DH_TransferMode_Interactive:
		ILibAddHeaderLine(http_headers,"transferMode.dlna.org",21,"Interactive",11);
		break;
	case DH_TransferMode_Streaming:
		ILibAddHeaderLine(http_headers,"transferMode.dlna.org",21,"Streaming",9);
		break;
	default:
		retval = 1;
	}

	return retval;
}

int DH_AddHeader_Range(struct packetheader* http_headers, int first_byte_pos, int last_byte_pos)
{
	char h[255];
	int hLen=0;
	int retval = 0;
	
	if(first_byte_pos>=0 && last_byte_pos>=0)
	{
		hLen = sprintf(h,"bytes=%d-%d",first_byte_pos,last_byte_pos);
	}
	else if(first_byte_pos>=0)
	{
		hLen = sprintf(h,"bytes=%d-",first_byte_pos);
	}
	else if(last_byte_pos>=0)
	{
		hLen = sprintf(h,"bytes=-%d",last_byte_pos);
	}

	ILibAddHeaderLine(http_headers,"Range",5,h,hLen);
	return retval;
}

int DH_AddHeader_ContentRange(struct packetheader* http_headers, size_t first_byte_pos, size_t last_byte_pos, long instance_length)
{
	char h[255];
	int hLen = sprintf(h,"bytes %ld-%ld/%ld",(long)first_byte_pos,(long)last_byte_pos,instance_length);

	ILibAddHeaderLine(http_headers,"Content-Range",13,h,hLen);

	return(0);
}

int DH_AddHeader_TimeSeekRange(struct packetheader* http_headers, float first_npt_sec, float last_npt_sec, int first_byte_pos, int last_byte_pos, int instance_length)
{
	/* TODO: DH_AddHeader_TimeSeekRange */

	int retval = 0;

	return retval;
}

int DH_AddHeader_getAvailableSeekRange(struct packetheader* http_headers)
{
	/* TODO: DH_AddHeader_getAvailableSeekRange */

	int retval = 0;

	return retval;
}

int DH_AddHeader_availableSeekRange(struct packetheader* http_headers, int mode_flag, int first_byte_pos, int last_byte_pos, float first_npt_sec, float last_npt_sec)
{
	/* TODO: DH_AddHeader_availableSeekRange */

	int retval = 0;

	return retval;
}

int DH_AddHeader_PlaySpeed(struct packetheader* http_headers, const char* transport_play_speed, int transport_play_speed_len)
{
	/* TODO: DH_AddHeader_PlaySpeed */

	int retval = 0;

	return retval;
}
int DH_AddHeader_scid(struct packetheader* http_headers, unsigned int connection_id)
{
	/* TODO: DH_AddHeader_scid */

	int retval = 0;

	return retval;
}

int DH_AddHeader_FriendlyName(struct packetheader* http_headers, const char* friendly_name, int friendly_name_len)
{
	/* TODO: DH_AddHeader_FriendlyName */

	int retval = 0;

	return retval;
}

int DH_AddHeader_PeerManager(struct packetheader* http_headers, const char* friendly_name, int friendly_name_len)
{
	/* TODO: DH_AddHeader_PeerManager */

	int retval = 0;

	return retval;
}

int DH_AddHeader_getcontentFeatures(struct packetheader* http_headers)
{
	/* TODO: DH_AddHeader_getcontentFeatures */

	int retval = 0;

	return retval;
}
int DH_AddHeader_contentFeatures(struct packetheader* http_headers, const char* fourth_field, int fourth_field_len)
{
	/* TODO: DH_AddHeader_contentFeatures */

	int retval = 0;

	return retval;
}

int DH_AddHeader_Pragma(struct packetheader* http_headers, int get_ifo_flag, const char* ifo_file_uri, int ifo_file_uri_len, const char* custom_pragma, int custom_pragma_len)
{
	/* TODO: DH_AddHeader_Pragma */

	int retval = 0;

	return retval;
}

DH_TransferStatus DH_SetUserObject(DH_TransferStatus transfer_status_handle, void* user_object)
{
	/*
		TODO: Implement DH_SetUserObject.
	*/

	return transfer_status_handle;
}

void* DH_GetUserObject(DH_TransferStatus transfer_status_handle)
{
	/*
		TODO: Implement DH_GetUserObject.
	*/

	return NULL;
}

void DH_QueryTransferStatus(DH_TransferStatus transfer_status_handle, long* send_total, long* send_expected, long* receive_total, long* receive_expected)
{
	sem_wait(&(transfer_status_handle->syncLock));
	if(send_total!=NULL){*send_total = transfer_status_handle->ActualBytesSent;}
	if(send_expected!=NULL){*send_expected = transfer_status_handle->TotalBytesToBeSent;}
	if(receive_total!=NULL){*receive_total = transfer_status_handle->ActualBytesReceived;}
	if(receive_expected!=NULL){*receive_expected = transfer_status_handle->TotalBytesToBeReceived;}
	sem_post(&(transfer_status_handle->syncLock));
}
DH_TransferStatus DH_CreateNewTransferStatus()
{
	DH_TransferStatus retval = (struct DH_TransferStatus_StateObject*)malloc(sizeof(struct DH_TransferStatus_StateObject));
	memset(retval,0,sizeof(struct DH_TransferStatus_StateObject));
	sem_init(&(retval->syncLock),0,1);
	retval->TotalBytesToBeSent = -1;
	retval->TotalBytesToBeReceived = -1;
	return(retval);
}
void DH_DestroyTransferStatus(DH_TransferStatus status)
{
	sem_destroy(&(status->syncLock));
	free(status);
}
void DH_AbortTransfer(DH_TransferStatus transfer_status_handle)
{
	sem_wait(&(transfer_status_handle->syncLock));
	if(transfer_status_handle->RequestToken != NULL)
	{
		transfer_status_handle->Reserved4 = 1;
	}
	sem_post(&(transfer_status_handle->syncLock));
	if(transfer_status_handle->ServerSession!=NULL)
	{
		if(transfer_status_handle->SessionFlag!=0)
		{
			ILibWebServer_Send_Raw(transfer_status_handle->ServerSession,"HTTP/1.1 500 Server Abort\r\n\r\n",29,ILibAsyncSocket_MemoryOwnership_STATIC,1);
		}
		ILibWebServer_DisconnectSession(transfer_status_handle->ServerSession);
	}
}
enum DH_TransferModes DH_GetRequestedTransferMode(struct packetheader *p)
{
	char *transferModeValue = ILibGetHeaderLine(p,"transferMode.dlna.org",21);
	enum DH_TransferModes RetVal = DH_TransferMode_Unspecified;

	if(transferModeValue!=NULL)
	{
		if(strcmp(transferModeValue,"Streaming")==0)
		{
			RetVal = DH_TransferMode_Streaming;
		}
		else if(strcmp(transferModeValue,"Interactive")==0)
		{
			RetVal = DH_TransferMode_Interactive;
		}
		else if(strcmp(transferModeValue,"Background")==0)
		{
			RetVal = DH_TransferMode_Bulk;
		}
	}
	return(RetVal);
}

