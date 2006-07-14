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
 * $Workfile: DmsIntegration.c
 *
 *
 *
 */

#ifdef _POSIX
#include <assert.h>
#define ASSERT assert
#define stricmp strcasecmp
#define strcmpi strcasecmp
#define strnicmp strncasecmp
#endif

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#ifndef _WIN32_WCE
#include <crtdbg.h>
	#include <assert.h>
	#define ASSERT(x) assert(x)
#endif
#else
#include <stdlib.h>
#endif


#if defined(WINSOCK2)
#include <winsock2.h>
#elif defined(WINSOCK1)
#include <winsock.h>
#endif

#define DMS_UDN_SIZE			80
#define DMS_NOTIFY_CYCLE		1800
#define DMS_USE_THREADPOOL		1
#define DMS_VIRTUAL_DIRNAME		"content"
#define DMS_VIRTUAL_DIRNAME_LEN	7
#define DMS_UPLOAD_VIRTUAL_DIRNAME "upload"
#define DMS_UPLOAD_VIRTUAL_DIRNAME_LEN 6
#define DMS_READTRANSMIT_SIZE	4096
#define REQUIRE_REGISTERED_PATH 0

#include "DmsIntegration.h"
#include "DlnaHttpServer.h"
#include "ILibWebServer.h"
#include "MediaServerAbstraction.h"
#include "MediaServer_MicroStack.h"
#include "FileIoAbstraction.h"
#include "MimeTypes.h"
#include "UTF8Utils.h"
#include "CdsStrings.h"
#include "CdsDidlSerializer.h"
#include "FileSystemEnumerator.h"
#include "DLNAProtocolInfo.h"



struct DMS_State
{
	/*
	 	The PreSelect, PostSelect, and Destroy fields must
	 	remain here as the object needs to be compatible with the
	 	thread-chaining framework.
	 */
	void (*PreSelect)(void* object,fd_set *readset, fd_set *writeset, fd_set *errorset, int* blocktime);
	void (*PostSelect)(void* object,int slct, fd_set *readset, fd_set *writeset, fd_set *errorset);
	void (*Destroy)(void* object);

	char*					State_SearchCapabilities;
	char*					State_SortCapabilities;
	char*					State_SinkProtocolInfo;
	char*					State_SourceProtocolInfo;

	sem_t					Lock;

	void*					Token_Ltm;
	void*					Token_ThreadChain;
	MediaServer_MicroStackToken		Token_Upnp;
	void*					Token_WebServer;
	unsigned short			WebServer_Port;
	MSA						Token_Msa;
	void*					Token_DmsUserObj;
	void*					Token_BackEnd;
	ILibThreadPool			Token_ThreadPool;

	sem_t IPAddressLock;		/* read/write lock for list of IP addresses */
	int IPAddressLength;		/* number of IP addresses for the machine */
	int *IPAddressList;		/* array of IP addresses for the machine */
#if defined(INCLUDE_FEATURE_UPLOAD)
	void									*UploadRequests;
#endif
};

#if defined(INCLUDE_FEATURE_UPLOAD)

struct DMS_UploadRequestObj
{
	char* UploadId;
	char* ImportURI;
	char* UploadLocalFilePath;
	int BytesReceived;
	int TotalBytesExpected;
	DMS_StateObj DmsState;
	DH_TransferStatus TransferStatus;
};

#endif

static void _DMS_RespondWithError(MSA msa_obj, void* upnp_session, enum Enum_CdsErrors error_code)
{
	const char* errormsg;

	switch (error_code)
	{
	case CDS_EC_OBJECT_ID_NO_EXIST:
		errormsg = CDS_EM_OBJECT_ID_NO_EXIST;
		break;

	case CDS_EC_NO_SUCH_CONTAINER:
		errormsg = CDS_EM_NO_SUCH_CONTAINER;
		break;

	case CDS_EC_INVALID_BROWSEFLAG:
		errormsg = CDS_EM_INVALID_BROWSEFLAG;
		break;

	case CDS_EC_INTERNAL_ERROR:
		errormsg = CDS_EM_INTERNAL_ERROR;
		break;

	case CMS_EC_CONNECTION_DOES_NOT_EXIST:
		errormsg = CMS_EM_CONNECTION_DOES_NOT_EXIST;
		break;

	default:
		error_code = CDS_EC_ACTION_FAILED;
		errormsg = CDS_EM_ACTION_FAILED;
		break;
	}
	
	MSA_ForResponse_RespondError(msa_obj, upnp_session, error_code, errormsg);
}

struct DLNAProtocolInfo *GetBaseProtocolInfo()
{
	struct DLNAProtocolInfo *p = (struct DLNAProtocolInfo*)malloc(sizeof(struct DLNAProtocolInfo));
	memset(p,0,sizeof(struct DLNAProtocolInfo));

	p->DLNA_Major_Version = 1;
	p->DLNA_Minor_Version = 5;
	return(p);
}

char *_DMS_SetImportUriValue(int RequestIPAddress, int RequestPort, char *UploadID)
{
	char *importUri = (char*)malloc((int)strlen(UploadID)+30);
	char *RetVal;
	int RetValLen;

	sprintf(importUri,"http://%s:%d/%s",ILibIPAddress_ToDottedQuad(RequestIPAddress),RequestPort,UploadID);
	RetValLen = ILibHTTPEscapeLength(importUri);
	RetVal = (char*)malloc(RetValLen+1);
	ILibHTTPEscape(RetVal,importUri);
	free(importUri);

	return(RetVal);
}

void AttachResourceToCdsObject(struct CdsObject *cdsObj, struct CdsResource *res)
{
	struct CdsResource *res2;

	if(cdsObj->Res==NULL)
	{
		cdsObj->Res = res;
	}
	else
	{
		res2 = cdsObj->Res;
		while(res2->Next!=NULL)
		{
			res2 = res2->Next;
		}
		res2->Next = res;
	}
}

void AddHTTPResources(struct DMS_State *dms, unsigned int filter, struct MSA_CdsQuery * cds_query, struct CdsObject *cds_obj)
{
	struct CdsResource *r = NULL;
	struct DLNAProtocolInfo *pi;
	char *fileExtension;
	int i;
	char *temp;
	int tempLen;
	int IsTempFile = 0;

	if(cds_obj->MediaClass & CDS_CLASS_MASK_ITEM)
	{
		if(ILibString_EndsWith(cds_obj->Source,(int)strlen(cds_obj->Source),EXTENSION_UPLOAD_TEMP,EXTENSION_UPLOAD_TEMP_LEN)==0)
		{
			fileExtension = cds_obj->Source + ILibString_LastIndexOf(cds_obj->Source,(int)strlen(cds_obj->Source),".",1);
		}
		else
		{
			IsTempFile=1;
			fileExtension = cds_obj->Source + ILibString_LastIndexOf(cds_obj->Source,(int)strlen(cds_obj->Source)-EXTENSION_UPLOAD_TEMP_LEN,".",1);
			fileExtension = ILibString_Copy(fileExtension,(int)strlen(fileExtension)-EXTENSION_UPLOAD_TEMP_LEN);
		}

		for(i=0;((CdsFilter_DlnaAllIp & filter) && i < cds_query->IpAddrListLen) || (i==0);++i)
		{
			r = CDS_AllocateResource();
			pi = GetBaseProtocolInfo();
			pi->DLNA_Major_Version=1;
			pi->DLNA_Minor_Version=5;
			pi->HTTP_Stalling = 1;
			pi->SupportsByteBasedSeek = 1;
			pi->Protocol = DLNAProtocolInfo_ProtocolType_HTTP;

			pi->TM_B=1;

			if(FileExtensionToClassCode(fileExtension, 0) == CDS_MEDIACLASS_IMAGEITEM)
			{
				pi->TM_I=1;
			}
			if((FileExtensionToClassCode(fileExtension, 0) == CDS_MEDIACLASS_AUDIOITEM) ||
			   (FileExtensionToClassCode(fileExtension, 0) == CDS_MEDIACLASS_VIDEOITEM)
			)
			{
				pi->TM_S=1;
			}

			pi->MimeType = ILibString_Copy((char*)FileExtensionToMimeType(fileExtension,0),-1);
			pi->Profile = ILibString_Copy((char*) FileExtensionToDlnaProfile(fileExtension, 0),-1);

			r->ProtocolInfo = DLNAProtocolInfo_Serialize(pi);
			r->Allocated |= CDS_RES_ALLOC_ProtocolInfo;
			DLNAProtocolInfo_Destruct(pi);
			
			if(IsTempFile!=0)
			{
				//
				// This is a temp upload file, so set the import uri
				//
				r->ResumeUpload=1;
				r->UploadedSize = (int)ILibFileDir_GetFileSize(cds_obj->Source); //ToDo: This needs to be a long
				tempLen = FSE_LocalFilePathToRelativePath(dms->Token_BackEnd,cds_obj->Source,(int)strlen(cds_obj->Source),&temp);
				r->ImportUri = _DMS_SetImportUriValue(cds_query->IpAddrList[i],ILibWebServer_GetPortNumber(dms->Token_WebServer),temp);
				r->Allocated |= CDS_RES_ALLOC_ImportUri;
				free(temp);
			}
			else
			{
				//
				// Set the Value
				//
				tempLen = FSE_LocalFilePathToRelativePath(dms->Token_BackEnd,cds_obj->Source,(int)strlen(cds_obj->Source),&temp);
				r->Value = (char*)malloc(tempLen + 30 + DMS_VIRTUAL_DIRNAME_LEN + 1);	// http://255.255.255.255:65535/
				sprintf(r->Value,"http://%s:%u/%s/%s",ILibIPAddress_ToDottedQuad(cds_query->IpAddrList[i]),ILibWebServer_GetPortNumber(dms->Token_WebServer),DMS_VIRTUAL_DIRNAME,temp);
				free(temp);
				tempLen = ILibHTTPEscapeLength(r->Value);
				temp = (char*)malloc(tempLen+1);
				tempLen = ILibHTTPEscape(temp,r->Value);
				temp[tempLen]=0;
				free(r->Value);
				r->Value = temp;
				r->Allocated |= CDS_RES_ALLOC_Value;
			}
			AttachResourceToCdsObject(cds_obj,r);
		}
		if(IsTempFile!=0)
		{
			free(fileExtension);
		}
	}
}

#if defined (INCLUDE_FEATURE_PLAYSINGLE)
void AddPlaySingleResources(struct DMS_State *dms, unsigned int filter, struct MSA_CdsQuery * cds_query, struct CdsObject *cds_obj)
{
	/*
	*	SOLUTION_REFERENCE#3.6.3.11c
	*/

	struct CdsResource *r = NULL;
	struct DLNAProtocolInfo *pi;
	char *udn, *itemId;
	char *fileExtension;
	char *temp;
	int tempLen;

	if(cds_obj->MediaClass & CDS_CLASS_MASK_ITEM)
	{
		fileExtension = cds_obj->Source + ILibString_LastIndexOf(cds_obj->Source,(int)strlen(cds_obj->Source),".",1);

		r = CDS_AllocateResource();
		pi = GetBaseProtocolInfo();
		DH_UpdateProtocolInfoOptions(pi);

		pi->MimeType = ILibString_Copy((char*)FileExtensionToMimeType(fileExtension,0),-1);
		pi->Profile = ILibString_Copy((char*) FileExtensionToDlnaProfile(fileExtension, 0),-1);

		r->ProtocolInfo = DLNAProtocolInfo_Serialize(pi);
		r->Allocated |= CDS_RES_ALLOC_ProtocolInfo;
		DLNAProtocolInfo_Destruct(pi);
		
		udn = (char*) MediaServer_GetConfiguration()->UDN;

		/*
			*	Get the worst case length for this URI.
			*	Maximum length of a 32-bit integer is 21 bytes.
			*	Ensure room for a few extra / chars.
			*/
		itemId = cds_obj->ID+CDS_STRING_PLAYSINGLE_ITEM_PREFIX_LEN;

		tempLen = CDS_STRING_PLAYSINGLE_SCHEME_LEN + (int) strlen(udn) + CDS_STRING_PLAYSINGLE_SERVICEID_CDS_LEN +
			CDS_STRING_PLAYSINGLE_ITEMID_LEN + (int) strlen(itemId) + 1; // 1 for '?'

		r->Value = (char*) malloc(tempLen+1);

		sprintf(r->Value, "%s%s?%s%s%s", CDS_STRING_PLAYSINGLE_SCHEME, udn, CDS_STRING_PLAYSINGLE_SERVICEID_CDS,
			CDS_STRING_PLAYSINGLE_ITEMID, itemId);

		tempLen = ILibHTTPEscapeLength(r->Value);
		temp = (char*)malloc(tempLen+1);
		tempLen = ILibHTTPEscape(temp,r->Value);
		temp[tempLen]=0;
		free(r->Value);
		r->Value = temp;
		r->Allocated |= CDS_RES_ALLOC_Value;

		AttachResourceToCdsObject(cds_obj,r);
	}
}
#endif

static int _DMS_ProcessObject(MSA msa_obj, struct MSA_CdsQuery * cds_query, void *qs_obj, unsigned int filter, void *dms_state, /*INOUT*/ struct CdsObject *cds_obj)
{
	struct DMS_State *dms = (struct DMS_State*) dms_state;
	int val = FSE_HandleProcessObject(dms->Token_BackEnd, cds_query, qs_obj, filter, cds_obj);

	//
	// Adding the resources to the CdsObject
	//
	if(val != 0)
	{
		#if defined (INCLUDE_FEATURE_PLAYSINGLE)
		if(ILibString_StartsWith(cds_obj->ID, (int) strlen(cds_obj->ID), "playsingle-", 11)!=0)
		{
			//
			// PlaySingle Resources
			//
			AddPlaySingleResources(dms, filter, cds_query, cds_obj);
		}
		else
		{
		#endif

			//
			// HTTP Resources
			//
			AddHTTPResources(dms, filter, cds_query, cds_obj);

		#if defined (INCLUDE_FEATURE_PLAYSINGLE)
		}
		#endif
	}
	return(val);
}

static void _DMS_ProcessQuery(MSA msa_obj, void* upnp_session, struct MSA_CdsQuery* cds_query, void* qs_obj, unsigned int filter, void* dms_state)
{
	struct DMS_State *dms = (struct DMS_State*) dms_state;
	struct CdsObject *cdsObj = NULL;

	char *didl;
	int didlLen;
	unsigned int numberReturned = 0, totalMatched = 0, updateID = 0, status = 0;

	/*
	*	SOLUTION_REFERENCE#3.6.3.2c
	*/


	if(msa_obj!=NULL)
	{
		/* ask app layer to start incremental process for the CDS query */
		status = MSA_ForQueryResponse_Start(msa_obj, upnp_session, cds_query, 1);
	}


	/* allocate an object */
	cdsObj = CDS_AllocateObject();


	/*
	*	SOLUTION_REFERENCE#3.6.3.5b
	*/

	/* inner while loop: acquire and send metadata for each CDS object */
	while ((status >= 0) && (_DMS_ProcessObject(msa_obj, cds_query, qs_obj, filter, dms_state, cdsObj) != 0))
	{
		totalMatched++;
		/* repetitively ask app layer to get metadata for 1 <res> element */

		if (
			((totalMatched-1) >= cds_query->StartingIndex)
			&& ((cds_query->RequestedCount == 0) || (numberReturned < cds_query->RequestedCount))
			)
		{
			/*
				*	Get the XML form and send it.
				*/

			#ifndef METADATA_IS_XML_ESCAPED
			/*
				*	TODO: Optimize by defining METADATA_IS_XML_ESCAPED
				*	and implementing a metadata source that writes
				*	XML-escaped data.
				*/
			#define METADATA_IS_XML_ESCAPED 0
			#endif

			/* convert CDS object to DIDL-Lite */
			didl = CDS_SerializeObjectToDidl(cdsObj, METADATA_IS_XML_ESCAPED, filter, 0, &didlLen);
			if (didlLen > 0)
			{
				if ((msa_obj != NULL) && (status >= 0))
				{
					/*
						*	TODO: Optimize by defining storing all Metadata in UTF8 format (such as a database)
						*	and therefore skipping the need to encode from multibyte to UTF8-encoding.
						*/
					
					char* utf8didl;
					utf8didl = EncodeToUTF8(didl);

					/* respond with DIDL-Lite for 1 CDS object*/
					status = MSA_ForQueryResponse_ResultArgumentRaw(msa_obj, upnp_session, cds_query, utf8didl, (int) strlen(utf8didl));
					free(utf8didl);
				}
				numberReturned++;
			}
			else
			{
				/*
					*	print an error message if we couldn't get the DIDL.
					*	If this happens, there will likely be an inconsistency between
					*	the number of objects returned and the TotalMatched value.
					*/
				fprintf(stderr, "_MSL_ProcessQueue() failed to serialize object %s. Reason=%d.\r\n", cdsObj->ID, didlLen);
			}

			/*
`				 *	Free and create a new cdsobj.
				*/
			free (didl);
		}

		CDS_ObjRef_Release(cdsObj);
		cdsObj = CDS_AllocateObject();
	}	/* end inner while */

	/* free the CDS object */


	CDS_ObjRef_Release(cdsObj);
	cdsObj = NULL;

	/*
		*	Get the updateID value.
		*/
	updateID = FSE_HandleOnAcquireUpdateIDAndCleanup(dms->Token_BackEnd, cds_query, qs_obj);
	
	/*
		*	Finish up response
		*/
	if (msa_obj != NULL)
	{
		if (status >= 0)
		{
			/* IDF#8c: send rest of response */
			/* ignore the return code because it will call MSA_ForQueryResponse_Cancelled for us */
			MSA_ForQueryResponse_FinishResponse(msa_obj, upnp_session, cds_query, 1, numberReturned, totalMatched, updateID);
		}
		else 
		{
			/*
				*	an error occurred during the response, so tell the microstack that
				*	we're cancelling the response and that we have no intention to
				*	call other MSA_ForQueryResponse_xxx methods.
				*/
			MSA_ForQueryResponse_Cancelled(msa_obj, upnp_session, cds_query);
		}
	}
}

static void _DMS_MsaHandler_OnStats(MSA msa_obj, struct MSA_Stats* stats)
{
	/* TODO: Application does something like update a UI. */

	/* do not hold a reference to stats after execution returns to MSA */
}

static void _DMS_MsaHandler_OnBrowse(MSA msa_obj, void* upnp_session, struct MSA_CdsQuery* cds_query)
{
	struct DMS_State *dms = NULL;
	char* utf8Id;

	/*
	*	SOLUTION_REFERENCE#3.6.3.4
	*/

	/* 
		TODO: Your application layer should replace this implementation
		with whatever mechanism is needed to respond to respond to the query.

		For systems that want a synchronous approach, please set DMS_USE_THREADPOOL==1.
		This will cause the microstack to do a context switch for the callback,
		allowing this method to respond without doing any harm to the microstack
		thread-chain.

		For systems that want an asynchronous approach, usually it's best to set
		DMS_USE_THREADPOOL==0. Generally, you will want to 
			- translate cds_query into a query that your back-end can handle
			- issue the query to your back-end
			- allow the thread to return to the MediaServerAbstraction
			- convert the query results from your back-end on a separate thread
				into a series of calls to MSA_ForQueryResponse_xxx. Usually this
				process happens the thread that your back-end used to notify your
				app of the results.
	*/

	unsigned int filter;
	enum Enum_CdsErrors errorcode = CdsError_None;
	void* fsequery;

	/*
	*	SOLUTION_REFERENCE#3.6.3.2a
	*/

	if(msa_obj == NULL) return;

	dms = (struct DMS_State*) MSA_GetUserObject(msa_obj);

	filter = CDS_ConvertCsvStringToBitString(cds_query->Filter);

	/*
	*	SOLUTION_REFERENCE#3.6.3.2b and SOLUTION_REFERENCE#3.6.3.5a
	*/

	utf8Id = DecodeFromUTF8(cds_query->ObjectID);
	free(cds_query->ObjectID);
	cds_query->ObjectID = utf8Id;

	fsequery = FSE_HandleQuery(dms->Token_BackEnd, cds_query, filter, &errorcode);
	if (errorcode == CdsError_None)
	{
		/* the back-end will accept this query */
		
		/*
		*	SOLUTION_REFERENCE#3.6.3.2a
		*/

		/* BrowseDirectChildren and Search */
		_DMS_ProcessQuery(msa_obj, upnp_session, cds_query, fsequery, filter, dms);
	}
	else
	{
		/* An error has occured, so send an error response. */
		MSA_ForResponse_RespondError(msa_obj, upnp_session, errorcode, "Error");
	}

	/* we have responded so free the query */
	MSA_DeallocateCdsQuery(cds_query);


	/*
		This comment branch shows how to respond if your back-end is guaranteed
		to execute a callback asynchronously on a separate thread.
		This branch is just a pseudo-code example.

		-	_DMS_MsaHandler_OnQuery(...) {
		-		sql_query = BE_TranslateQueryToSql();
		-		BE_DoSqlQuery(sql_query, Callback_SqlQueryDone, msa_obj, cds_query, msa_user_obj);
		-		return;
		-	}

		The implementation of Callback_SqlQueryDone()
		should be like this:

		-	Callback_SqlQueryDone(sql_query, sql_query_response, msa_obj, cds_query, msa_user_obj) {
		-		MSA_ForQueryResponse_Start(...);
		-		foreach (cds_object in sql_query_response) {
		-			MSA_ForQueryResponse_ResultArgumentStruct(cds_object, ...);
		-		}
		-		MSA_ForQueryResponse_FinishResponse(...);	
		-		return;
		-	}
	*/
}

static void _DMS_MsaHandler_OnSearch(MSA msa_obj, struct MSA_CdsQuery* cds_query, void* upnp_session)
{
	/* search is currently not implemented, and it will just be handled like a browse request */
	_DMS_MsaHandler_OnBrowse(msa_obj, cds_query, upnp_session);
}

static void _DMS_MsaHandler_OnGetSystemUpdateID(MSA msa_obj, void* upnp_session)
{
	if(msa_obj == NULL) return;
	
	MSA_RespondGetSystemUpdateID(msa_obj, upnp_session, MSA_GetSystemUpdateID(msa_obj));
}

static void _DMS_MsaHandler_OnGetSearchCapabilities(MSA msa_obj, void* upnp_session)
{
	struct DMS_State *dms = NULL;

	if(msa_obj == NULL) return;
	dms = (struct DMS_State*) MSA_GetUserObject(msa_obj);

	MSA_RespondGetSearchCapabilities(msa_obj, upnp_session, dms->State_SearchCapabilities);
}

static void _DMS_MsaHandler_OnGetSortCapabilities(MSA msa_obj, void* upnp_session)
{
	struct DMS_State *dms = NULL;

	if(msa_obj == NULL) return;
	dms = (struct DMS_State*) MSA_GetUserObject(msa_obj);

	MSA_RespondGetSortCapabilities(msa_obj, upnp_session, dms->State_SortCapabilities);
}

static void _DMS_MsaHandler_OnGetProtocolInfo(MSA msa_obj, void* upnp_session)
{
	struct DMS_State *dms = NULL;

	if(msa_obj == NULL) return;
	dms = (struct DMS_State*) MSA_GetUserObject(msa_obj);

	MSA_RespondGetProtocolInfo(msa_obj, upnp_session, dms->State_SourceProtocolInfo, dms->State_SinkProtocolInfo);
}

static void _DMS_MsaHandler_OnGetCurrentConnectionIDs(MSA msa_obj, void* upnp_session)
{
	/* not implemented, always return 0 */
	MSA_RespondGetCurrentConnectionIDs(msa_obj, upnp_session, "0");
}

static void _DMS_MsaHandler_OnGetCurrentConnectionInfo(MSA msa_obj, void* upnp_session, int connection_id)
{
	if(connection_id != 0)
	{
		MSA_ForResponse_RespondError(msa_obj, upnp_session, (int) CMS_EC_CONNECTION_DOES_NOT_EXIST, CMS_EM_CONNECTION_DOES_NOT_EXIST);	
	}
	else
	{
		MSA_RespondGetCurrentConnectionInfo(msa_obj, upnp_session, 0, 0, "", "", -1, "Output", "Unknown");
	}
}

static void _DMS_WebServerHandler_OnContentRequestDone(struct ILibWebServer_Session *session, DH_TransferStatus transfer_status_handle, enum DHS_Errors dhs_error_code, void *user_obj)
{
	/* 
		TODO: Update the app's UI or something.

		Do not use transfer_status_handle after this method exits as the DlnaHttpServer
		will destroy the memory.
	*/
	if(dhs_error_code == DHS_ERRORS_NONE)
	{
		printf("Finished responding HTTP-GET request.\r\n");
	}
	else if(dhs_error_code == DHS_ERRORS_PEER_ABORTED_CONNECTION)
	{
		printf("HTTP-GET request aborted by client endpoint.\r\n");
	}
}

/*
	Handle Incoming HTTP Requests
*/

static void _DMS_WebServerHandler_OnContentRequest(struct ILibWebServer_Session *session, struct packetheader *header, char *bodyBuffer, int *beginPointer, int endPointer, int done, void *user)
{
	struct DMS_State *dms = (struct DMS_State*) user;
	/* TODO: _DMS_WebServerHandler_OnContentRequest */

	struct DLNAProtocolInfo *protocolInfo;
	struct parser_result *pr;
	char localfile[MAX_FILENAME_SIZE];
	char* fileExtension;
	char* fourthField;
	char* mimeType;
	char* profile;
	char* protocolStr;
	char* directiveObj;
	int len = 0;

	int pdl = header->DirectiveLength;
	if(pdl != 3 && pdl != 4)
	{
		// TODO:  Add error condition here...
		printf("Webrequest (invalid header field): \r\n");
		return;
	}

	printf("received request\r\n");

	/* 
		TODO: Your application layer should replace this implementation
		with whatever mechanism is needed to respond to respond to the query.
	*/

	/*
	*	SOLUTION_REFERENCE#3.6.3.5d and SOLUTION_REFERENCE#3.6.3.6a
	*/
	header->DirectiveObj[header->DirectiveObjLength]=0;
	header->DirectiveObjLength = ILibInPlaceHTTPUnEscape(header->DirectiveObj);
	header->DirectiveObj[header->DirectiveObjLength]=0;
	
	directiveObj = DecodeFromUTF8(header->DirectiveObj);
	len = FSE_RelativePathToLocalFilePath(
		dms->Token_BackEnd,
		directiveObj,
		(int) strlen(directiveObj), 
		(char*)&localfile);

	// HTTP-GET or HTTP-HEAD requests
	if(strncasecmp(header->Directive, "GET", pdl) == 0 || strncasecmp(header->Directive, "HEAD", pdl) == 0)
	{
		char* range = (char*)ILibGetHeaderLine(header,"Range",5);
		if(strncasecmp(header->Directive, "GET", pdl) == 0)
		{
			printf("Webrequest (http-get) (GET): %s\r\n", localfile);
			if(range != NULL)
			{
				printf("\tRange Requested: %s\n", range);
			}
		}
		else
		{
			printf("Webrequest (http-get) (HEAD): %s\r\n", localfile);
			if(range != NULL)
			{
				printf("\tRange Requested: %s\n", range);
			}
		}

		fileExtension = localfile + ILibString_LastIndexOf(localfile,(int)strlen(localfile),".",1);
		mimeType = (char*) FileExtensionToMimeType(fileExtension, 0);
		profile = (char*) FileExtensionToDlnaProfile(fileExtension, 0);

		protocolInfo = (struct DLNAProtocolInfo*) malloc(sizeof(struct DLNAProtocolInfo));
		memset(protocolInfo, 0, sizeof(struct DLNAProtocolInfo));
		protocolInfo->MimeType = ILibString_Copy(mimeType,(int)strlen(mimeType));
		protocolInfo->Profile = ILibString_Copy(profile,(int)strlen(profile));

		protocolInfo->DLNA_Major_Version=1;
		protocolInfo->DLNA_Minor_Version=5;
		protocolInfo->HTTP_Stalling = 1;
		protocolInfo->SupportsByteBasedSeek = 1;
		protocolInfo->Protocol = DLNAProtocolInfo_ProtocolType_HTTP;

		protocolInfo->TM_B=1;

		if(FileExtensionToClassCode(fileExtension, 0) == CDS_MEDIACLASS_IMAGEITEM)
		{
			protocolInfo->TM_I=1;
		}
		if((FileExtensionToClassCode(fileExtension, 0) == CDS_MEDIACLASS_AUDIOITEM) ||
           (FileExtensionToClassCode(fileExtension, 0) == CDS_MEDIACLASS_VIDEOITEM)
		   )
		{
			protocolInfo->TM_S=1;
		}

		protocolStr = DLNAProtocolInfo_Serialize(protocolInfo);
		DLNAProtocolInfo_Destruct(protocolInfo);

		pr = ILibParseString(protocolStr,0,(int) strlen(protocolStr),":",1);

		fourthField = pr->LastResult->data;

		DHS_RespondWithLocalFile(
			session,
			dms->Token_ThreadPool,
			header, 
			DMS_READTRANSMIT_SIZE, 
			localfile,
			DH_TransferMode_Bulk,
			DH_TransferMode_Streaming,
			mimeType,
			fourthField,
			NULL,
			NULL,
			_DMS_WebServerHandler_OnContentRequestDone);

		ILibDestructParserResults(pr);
		free(protocolStr);
	}
	// HTTP-POST requests
	else if(strncasecmp(header->Directive, "POST", pdl) == 0)
	{
		printf("Invalid Webrequest: \r\n");

		ILibWebServer_Send_Raw(session,"HTTP/1.1 400\r\n\r\n",16,ILibAsyncSocket_MemoryOwnership_STATIC,0);	

	}
	free(directiveObj);
}

#if defined(INCLUDE_FEATURE_UPLOAD)
/*
	Handle Incoming HTTP Post Requests
*/
static void _DMS_WebServerHandler_OnContentUploadDone(struct ILibWebServer_Session *session, DH_TransferStatus transfer_status_handle, enum DHS_Errors dhs_error_code, void *user_obj)
{
	/* 
		TODO: this will execute the callback to update the app's UI or something.

		Do not use request object after this method exits as the MSA
		will destroy the memory.
	*/
	struct DMS_State* state;
	struct DMS_UploadRequestObj *request;
	char* key = NULL;
	int savedFileLen;

	request = (struct DMS_UploadRequestObj*) user_obj;

	/* upload completed, remove request from table */
	if(request != NULL)
	{
		state = (struct DMS_State*) request->DmsState;

		/* upload finished, delete request from the table */
		if(state!=NULL && state->UploadRequests!=NULL)
		{

			ILibHashTree_Lock(state->UploadRequests);
			request = (struct DMS_UploadRequestObj*) ILibGetEntry(state->UploadRequests, request->UploadId, (int) strlen(request->UploadId));
			
			if(request!=NULL)
			{
				key = ILibString_Copy(request->UploadId, -1);
				if (request != NULL)
				{
					request->BytesReceived = DMS_GetBytesReceived(request);

					if(dhs_error_code == DHS_ERRORS_NONE)
					{

						savedFileLen = (int) strlen(request->UploadLocalFilePath);

						if(ILibString_EndsWith(request->UploadLocalFilePath, savedFileLen, EXTENSION_UPLOAD_TEMP, EXTENSION_UPLOAD_TEMP_LEN)!=0)
						{
							char* renameToFile;
							/* it is a temp file, rename it to valid media file */

							renameToFile = (char*) malloc(savedFileLen-3);
							memcpy(renameToFile, request->UploadLocalFilePath, savedFileLen-4);
							renameToFile[savedFileLen-4] = '\0';
							if(ILibFileDir_MoveFile(request->UploadLocalFilePath, renameToFile)!=0)
							{
								ILibFileDir_DeleteFile(request->UploadLocalFilePath);
							}

							free(renameToFile);
						}

						printf("Finished process HTTP-POST request.\r\n");
						request->TransferStatus = NULL;
						free(request->UploadId);
						free(request->ImportURI);
						free(request->UploadLocalFilePath);
						free(request);
						ILibDeleteEntry(state->UploadRequests, key, (int) strlen(key));
					}
					else if(dhs_error_code == DHS_ERRORS_PEER_ABORTED_CONNECTION)
					{
						printf("HTTP-POST request aborted by client endpoint.\r\n");
					}
				}

				free(key);
			}

			ILibHashTree_UnLock(state->UploadRequests);
		}
	}
}


static void _DMS_WebServerHandler_OnContentUpload(struct ILibWebServer_Session *session, struct packetheader *header, char *bodyBuffer, int *beginPointer, int endPointer, int done, void *user)
{
	struct DMS_State* state;
	struct DMS_UploadRequestObj *request;
	char* uploadId;
	char* directiveObj;
	int unescapedLen;
	int pdl = 0;

	state = (struct DMS_State*) user;
	ASSERT(state->Token_ThreadPool);
	MSA_LockState(state->Token_Msa);

	pdl = header->DirectiveLength;
	if(pdl != 3 && pdl != 4)
	{
		// TODO:  Add error condition here...

		printf("Webrequest (invalid header field): \r\n");
		return;
	}

	// HTTP-GET or HTTP-HEAD requests
	if(strncasecmp(header->Directive, "GET", pdl) == 0 || strncasecmp(header->Directive, "HEAD", pdl) == 0)
	{
		printf("Invalid Webrequest: \r\n");

			ILibWebServer_Send_Raw(session,"HTTP/1.1 500 Invalid Request\r\n\r\n",32,ILibAsyncSocket_MemoryOwnership_STATIC,1);

	}
	// HTTP-POST requests
	else if(strncasecmp(header->Directive, "POST", pdl) == 0)
	{
		/* check to see if this upload was accepted
           upload requests are stored using directObj as the key
		*/
		directiveObj = DecodeFromUTF8(header->DirectiveObj);

		uploadId = ILibString_Cat(DMS_UPLOAD_VIRTUAL_DIRNAME,DMS_UPLOAD_VIRTUAL_DIRNAME_LEN,header->DirectiveObj,header->DirectiveObjLength);



		/* make sure unescape HTTP string */

		unescapedLen = ILibInPlaceHTTPUnEscape(uploadId);

		uploadId[unescapedLen] = '\0';

		ILibHashTree_Lock(state->UploadRequests);

		request = (struct DMS_UploadRequestObj*) ILibGetEntry(state->UploadRequests, uploadId, unescapedLen);
		
		if(request==NULL)
		{
			/* request was not accepted, invalid upload request */
			ILibWebServer_Send_Raw(session,"HTTP/1.1 500 Upload Not Approved\r\n\r\n",36,ILibAsyncSocket_MemoryOwnership_STATIC,0);
		}
		else
		{

			if(ILibFileDir_GetFileSize(request->UploadLocalFilePath) > 0)
			{
				request->TransferStatus = DHS_SavePostToLocalFile(
											session,
											state->Token_ThreadPool,
											header,
											request->UploadLocalFilePath,
											1,
											request,
											_DMS_WebServerHandler_OnContentUploadDone);
			}
			else
			{
				request->TransferStatus = DHS_SavePostToLocalFile(
										session,
										state->Token_ThreadPool,
										header,
										request->UploadLocalFilePath,
										0,
										request,
										_DMS_WebServerHandler_OnContentUploadDone);
			}
		}

		free(directiveObj);
		free(uploadId);
		ILibHashTree_UnLock(state->UploadRequests);	
	}
	MSA_UnLockState(state->Token_Msa);
}

/*****************************************************************************/
/* UPLOAD CAPABILITY														 */
/*****************************************************************************/
char* _DMS_MakeUploadId(const char* obj_id)
{
	char* uploadId;
	char* tempPtr;
	int len = (int) strlen(obj_id);

	/* uploads request IDs are HTTP Post directive object in the /upload virtual directory
	   this literal will be used to to map to POST directive object on upload requests.
	*/

	uploadId = (char*) malloc(len + 2);
	strcpy(uploadId, "/");
	strcat(uploadId, obj_id);
	tempPtr = uploadId;
	uploadId = ILibString_Replace(tempPtr, (int) strlen(uploadId), "\\", 1, "/", 1);
	free(tempPtr);

	return uploadId;
}

int DMS_GetBytesReceived(DMS_UploadRequest request)
{
	struct DMS_UploadRequestObj* upload_request = (struct DMS_UploadRequestObj*) request;
	long bytesReceived = 0;
	long actualBytesReceived = -1;

	if(upload_request != NULL)
	{
		bytesReceived = upload_request->BytesReceived;

		if(upload_request->TransferStatus != NULL)
		{
			DH_QueryTransferStatus(upload_request->TransferStatus, NULL, NULL, &actualBytesReceived, NULL);
			if(actualBytesReceived > 0)
			{	
				bytesReceived += actualBytesReceived;
			}
		}
	}
	return bytesReceived;
}

int DMS_GetTotalBytesExpected(DMS_UploadRequest request)
{
	struct DMS_UploadRequestObj* upload_request = (struct DMS_UploadRequestObj*) request;
	long totalBytes = -1;
	long totalBytesExpected = -1;

	if(upload_request != NULL)
	{
		if(upload_request->TransferStatus != NULL)
		{
			DH_QueryTransferStatus(upload_request->TransferStatus, NULL, NULL, NULL, &totalBytesExpected);
			if(totalBytesExpected>0)
			{
				totalBytes = totalBytesExpected;
			}
		}
	}

	return totalBytesExpected;
}

static void _DMS_MsaHandler_OnCreateObjectAndUpload(MSA msa_obj,  void* upnp_session, void* create_obj_arg, struct CdsObject* cds_obj)
{
	struct DMS_UploadRequestObj* request;
	struct DMS_State *dms = NULL;
	struct MSA_CdsCreateObj *createObjArg = (struct MSA_CdsCreateObj*) create_obj_arg;
	struct DLNAProtocolInfo *protocolInfo;
	struct DLNAProtocolInfo *sourceProtocolInfo;
	struct CdsCreateClass* createClass;
	char* supportedProtocolInfo;
	char* uploadId;
	char* id;
	int reject = 0;
	int error = 0;
	struct parser_result *pr;
	struct parser_result_field *prf;

	/* TODO: _DMS_MsaHandler_OnUpload */

	/*
	*	SOLUTION_REFERENCE#3.5.3.8
	*/

	if(msa_obj == NULL || cds_obj == NULL) return;

	dms = (struct DMS_State*) MSA_GetUserObject(msa_obj);

	/* ToDo: servers should validate with the backend to make sure that this upload is supported. 
	 * First check the mediaclass of the created CDS item is supported on the container it intends to create.
	 * However, since we use a file system, all containers will support the all
	 * image, audio and video items, anything else will be rejected. Also, all OCM operations are accepted,
	 * therefore, we can just ignore the dlnaManaged flag.
	 */
	if(((cds_obj->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE) == CDS_CLASS_MASK_ITEM) && cds_obj->ID!=NULL)
	{

		/* This is an item, it has to have one and only one <res> element, then
		 * check whether the protocolInfo is supported on the server
		 */

		if(
			(cds_obj->Res != NULL) && 
			(cds_obj->Res->ProtocolInfo != NULL) &&
			(cds_obj->Res->Next == NULL)
			)
		{
			protocolInfo = DLNAProtocolInfo_Parse(cds_obj->Res->ProtocolInfo, (int) strlen(cds_obj->Res->ProtocolInfo));
			supportedProtocolInfo = dms->State_SourceProtocolInfo;

			/* validate the protocolInfo, we only support the list of DLNA profiles listed in the ProtocolInfo string */
			pr = ILibParseString(supportedProtocolInfo,0,(int)strlen(supportedProtocolInfo),",",1);
			prf = pr->FirstResult;
			reject=1;
			while(prf!=NULL)
			{
				sourceProtocolInfo = DLNAProtocolInfo_Parse(prf->data,prf->datalength);
				if(DLNAProtocolInfo_IsMatch(protocolInfo,sourceProtocolInfo)!=0)
				{
					reject=0;
					DLNAProtocolInfo_Destruct(sourceProtocolInfo);
					break;
				}
				else
				{
					DLNAProtocolInfo_Destruct(sourceProtocolInfo);
				}
				prf = prf->NextResult;
			}
			ILibDestructParserResults(pr);

			if(protocolInfo->Profile==NULL)
			{
				/* missing DLNA.ORG_PN */
				reject = 1;
				error = 702;
			}
			
			if(reject==0 && FSE_HandleCreateObject(dms->Token_BackEnd, createObjArg, protocolInfo, cds_obj)!=0)
			{
				/* file system doesn't approve this upload, reject */
				printf("Rejecting upload - File system couldn't create the object\r\n");
				reject = 1;	
			}
			else if(reject==0)
			{
				/* successfully verified with backend, send CreateObject response */
				id = DecodeFromUTF8(cds_obj->ID);

				/*
					Application has the responsiblity to check for the uploadedSize field to see whether the control point
					is requesting upload with resume capabilities, if DMS supports resume capability, it must respond with uploadedSize
					value of 0.  Setting the resumeUpload value to 0 means the DMS does not support resume.
				*/
				if(cds_obj->Res->ResumeUpload!=0)
				{

					/* if server supports resume */
					cds_obj->Res->ResumeUpload = 1;
					cds_obj->Res->UploadedSize = 0;

					/* if server does not support resume, then you do the following */
					/* cds_obj->Res->UploadedSize = 0; */
				}

				/*
					Save to a temp file with different extension first,
					When upload finishes, import the file into CDS by changing its extension
				*/
				request = (struct DMS_UploadRequestObj*) malloc(sizeof(struct DMS_UploadRequestObj));
				memset(request, 0, sizeof(struct DMS_UploadRequestObj));

				request->DmsState = dms;


				FSE_LocalFilePathToRelativePath(dms->Token_BackEnd,cds_obj->Source,(int)strlen(cds_obj->Source),&uploadId);
				request->UploadId = (char*)malloc((int)strlen(uploadId)+DMS_UPLOAD_VIRTUAL_DIRNAME_LEN+2);
				sprintf(request->UploadId,"%s/%s",DMS_UPLOAD_VIRTUAL_DIRNAME,uploadId);
				request->UploadLocalFilePath = ILibString_Copy(cds_obj->Source,-1);
				free(uploadId);

				/* set the importUri value to the upload POST request directive */
				cds_obj->Res->ImportUri = _DMS_SetImportUriValue(((struct MSA_CdsCreateObj*)create_obj_arg)->RequestedOnAddress,((struct MSA_CdsCreateObj*)create_obj_arg)->RequestedOnPort,request->UploadId);
				cds_obj->Res->Allocated |= CDS_RES_ALLOC_ImportUri;
				request->ImportURI = ILibString_Copy(cds_obj->Res->ImportUri,-1);

				if(MSA_ForCreateObjectResponse_AcceptUpload(msa_obj, upnp_session, createObjArg, cds_obj)==0)
				{
					printf("Accepted upload request\r\n");

					/* lock the request table */
					ILibHashTree_Lock(dms->UploadRequests);	

					/*
					*	Add the request to our list of accepted upload requests, use UploadId as key 
					*/
					ILibAddEntry(dms->UploadRequests, request->UploadId, (int) strlen(request->UploadId), request);		

					/* unlock the request table */
					ILibHashTree_UnLock(dms->UploadRequests);
					MSA_IncrementSystemUpdateID(msa_obj);
				}
				else
				{
					/* dealloate memory */
					free(request->UploadId);
					free(request->ImportURI);
					free(request->UploadLocalFilePath);
					free(request);
					CDS_ObjRef_Release(cds_obj);
				}

				free(id);
				MSA_IncrementSystemUpdateID(msa_obj);		
			}

			DLNAProtocolInfo_Destruct(protocolInfo);
		}
		else
		{
			/* Missing protocolInfo vlaue or multiple <res> elements, reject */
			reject = 1;
			error = 702;
		}
	}
	else if(((cds_obj->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE) == CDS_CLASS_MASK_CONTAINER) && cds_obj->ID!=NULL)
	{
		/* This is a container, check whether the <CreateClass> is valid */
		createClass = cds_obj->TypeObject.Container.CreateClass;

		while(createClass != NULL)
		{
			if(
				((createClass->MediaClass & CDS_CLASS_MASK_OBJECT_TYPE) == CDS_CLASS_MASK_CONTAINER) ||
				((createClass->MediaClass & CDS_CLASS_MASK_MAJOR) == CDS_CLASS_MASK_MAJOR_IMAGEITEM) ||
				((createClass->MediaClass & CDS_CLASS_MASK_MAJOR) == CDS_CLASS_MASK_MAJOR_AUDIOITEM) ||
				((createClass->MediaClass & CDS_CLASS_MASK_MAJOR) == CDS_CLASS_MASK_MAJOR_VIDEOITEM)
				)
			{
				createClass = createClass->Next;
			}
			else
			{
				reject = 1;
				createClass = NULL;
			}
		}
		if(!reject)
		{
			/* all CreateClass are valid */

			/* validate upload with file system */
			if(FSE_HandleCreateObject(dms->Token_BackEnd, createObjArg, NULL, cds_obj)==0)
			{
				/* successfully verified with backend, send CreateObject response */
				MSA_ForCreateObjectResponse_Accept(msa_obj, upnp_session, createObjArg, cds_obj);
				MSA_IncrementSystemUpdateID(msa_obj);
			}
			else
			{
				reject = 1;
			}
		}
	}

	if(reject)
	{
		if(error != 0)
		{
			MSA_ForCreateObjectResponse_Reject(msa_obj, upnp_session, createObjArg, cds_obj, error, "CreateObject Failed");
			printf("CreateObject Failed - Error: %d\r\n", error);
		}
		else
		{
			MSA_ForCreateObjectResponse_Reject(msa_obj, upnp_session, createObjArg, cds_obj, 700, "CreateObject Failed");
			printf("CreateObject Failed - Error: 700\r\n");
		}
		CDS_ObjRef_Release(cds_obj);
	}
}

static void _DMS_MsaHandler_OnDestroyObject(MSA msa_obj, void* upnp_session, void* destroy_obj_arg)
{
	struct DMS_State *dms = NULL;
	struct MSA_CdsDestroyObj *destroyObjArg = (struct MSA_CdsDestroyObj*) destroy_obj_arg;
	struct CdsObject* destroyObj = NULL;

	unsigned int status = 0;


	/* TODO: _DMS_MsaHandler_OnUpload */


	/*
	*	SOLUTION_REFERENCE#3.6.3.5c
	*/
	if(msa_obj == NULL) return;

	dms = (struct DMS_State*) MSA_GetUserObject(msa_obj);

	destroyObj = CDS_AllocateObject();

	destroyObj->ID = destroyObjArg->ObjectID;

	status = FSE_HandleDestroyObject(dms->Token_BackEnd, destroyObj);

	if (status == 0)
	{
		MediaServer_AsyncResponse_START(upnp_session, CDS_STRING_DESTROY_OBJECT, CDS_STRING_URN_CDS);
		MediaServer_AsyncResponse_DONE(upnp_session, CDS_STRING_DESTROY_OBJECT);
		MSA_IncrementSystemUpdateID(msa_obj);
	}
	else 
	{
		/*
			*	an error occurred during the response, so tell the microstack that
			*	we're cancelling the response and that we have no intention to
			*	call other MSA_ForQueryResponse_xxx methods.
			*/
			MediaServer_Response_Error(upnp_session, 1000, "Error");
			ILibWebServer_Release(upnp_session);
	}

	free(destroyObjArg->ObjectID);
	free(destroyObjArg);
	CDS_ObjRef_Release(destroyObj);

}

#endif

/*
	This function is passed to the Chain, and will be called when the UPnP chain is destroyed.
*/
void DMS_DestroyMediaServer(DMS_StateObj dms_obj)
{
	struct DMS_State* dms = (struct DMS_State*)dms_obj;
	#if defined(INCLUDE_FEATURE_UPLOAD)
	struct DMS_UploadRequestObj* request;
	void *en;
	char *key;
	int keyLen;
	void *val;
	#endif

	DMS_Lock(dms);

	#if defined(INCLUDE_FEATURE_UPLOAD)
	if(dms->UploadRequests != NULL) 
	{
		ILibHashTree_Lock(dms->UploadRequests);
		en = ILibHashTree_GetEnumerator(dms->UploadRequests);
		while(ILibHashTree_MoveNext(en)==0)
		{
			ILibHashTree_GetValue(en,&key,&keyLen,&val);
			request = (struct DMS_UploadRequestObj*) val;
			/* clean up any temp files that didn't finish uploading */
			ILibFileDir_DeleteFile(request->UploadLocalFilePath);

			/* dealloate memory */
			free(request->UploadId);
			free(request->ImportURI);
			free(request->UploadLocalFilePath);
			free(request);
		}
		ILibHashTree_DestroyEnumerator(en);
		ILibHashTree_UnLock(dms->UploadRequests);
	}

	ILibDestroyHashTree(dms->UploadRequests);
	#endif

	free(dms->State_SearchCapabilities);
	free(dms->State_SortCapabilities);
	free(dms->State_SinkProtocolInfo);
	free(dms->State_SourceProtocolInfo);
	free(dms->IPAddressList);
	DMS_UnLock(dms);
}

/*
	Initializes the DMS server
*/
char* _IPAddressListToString(int* list, int length)
{
	char* result = (char*)malloc((length * 18) + 1);
	int i;

	result[0] = 0;
	for(i = 0; i < length; i++)
	{
		struct in_addr ipaddr;
		if(i != 0)
		{
			strcat(result, ", ");
		}
		ipaddr.s_addr = list[i];
		strcat(result, inet_ntoa(ipaddr));
	}

	return result;
}
DMS_StateObj DMS_Create(void* thread_chain,
						ILibThreadPool thread_pool,
						unsigned short port,
						const char* friendly_name, 
						const char* udn_uuid,
						const char* serial_num, 
						unsigned int sortable_fields, 
						unsigned int searchable_fields,
						const char* sink_protocol_info, 
						const char* source_protocol_info, 
						struct DMS_BackEndInit* back_end_init,
						void* dms_user_obj)
{
	struct DMS_State* retval = (struct DMS_State*) malloc(sizeof(struct DMS_State));
	int searchLen;
	int	sortLen;

	memset(retval, 0, sizeof(struct DMS_State));

	/* set initial state */

	/* convert the sortable and searchable properties into strings, create the MSA, and free */
	searchLen = CDS_GetCsvLengthFromBitString(searchable_fields);
	sortLen = CDS_GetCsvLengthFromBitString(sortable_fields);
	retval->State_SearchCapabilities = CDS_ConvertBitStringToCsvString(malloc(searchLen+1), searchable_fields);
	retval->State_SortCapabilities = CDS_ConvertBitStringToCsvString(malloc(sortLen+1), sortable_fields);
	retval->State_SinkProtocolInfo = ILibString_Copy(sink_protocol_info, -1);
	retval->State_SourceProtocolInfo = ILibString_Copy(source_protocol_info, -1);

	/*
		Assign all of the various tokens on DMS state object 
	 */ 
	retval->Token_ThreadChain = thread_chain;
	retval->Token_ThreadPool = thread_pool;
	retval->WebServer_Port = port;
	retval->Token_Ltm = ILibCreateLifeTime(retval->Token_ThreadChain);
	retval->Token_DmsUserObj = dms_user_obj;

	/* create the upnp device */
	retval->Token_Upnp = MediaServer_CreateMicroStack(
		thread_chain,
		friendly_name,
		udn_uuid,
		serial_num,
		DMS_NOTIFY_CYCLE,
		port);

	retval->Token_Msa = MSA_CreateMediaServer(
		retval->Token_ThreadChain, 
		retval->Token_Upnp,
		retval->Token_Ltm,
		0,
		retval->State_SinkProtocolInfo,
		retval->State_SourceProtocolInfo,
		retval->State_SortCapabilities,
		retval->State_SearchCapabilities,
		#if (DMS_USE_THREADPOOL==0)
		NULL,
		#else
		retval->Token_ThreadPool,
		#endif
		retval);

	/* register a virtual directory for our http server */
	retval->Token_WebServer = MediaServer_GetWebServerToken(retval->Token_Upnp);

	/*
	*	SOLUTION_REFERENCE#3.6.3.6b
	*/

	ILibWebServer_RegisterVirtualDirectory(
	retval->Token_WebServer,
	DMS_VIRTUAL_DIRNAME,
	DMS_VIRTUAL_DIRNAME_LEN,
	_DMS_WebServerHandler_OnContentRequest,
	retval);

	retval->Token_Msa->OnStats = _DMS_MsaHandler_OnStats;
	retval->Token_Msa->OnBrowse = _DMS_MsaHandler_OnBrowse;
	retval->Token_Msa->OnSearch = _DMS_MsaHandler_OnSearch;
	retval->Token_Msa->OnGetSystemUpdateID = _DMS_MsaHandler_OnGetSystemUpdateID;
	retval->Token_Msa->OnGetSearchCapabilities = _DMS_MsaHandler_OnGetSearchCapabilities;
	retval->Token_Msa->OnGetSortCapabilities = _DMS_MsaHandler_OnGetSortCapabilities;
	retval->Token_Msa->OnGetProtocolInfo = _DMS_MsaHandler_OnGetProtocolInfo;
	retval->Token_Msa->OnGetCurrentConnectionIDs = _DMS_MsaHandler_OnGetCurrentConnectionIDs;
	retval->Token_Msa->OnGetCurrentConnectionInfo = _DMS_MsaHandler_OnGetCurrentConnectionInfo;

	#if defined(INCLUDE_FEATURE_UPLOAD)

	retval->Token_Msa->OnCreateObject = _DMS_MsaHandler_OnCreateObjectAndUpload;
	retval->Token_Msa->OnDestroyObject = _DMS_MsaHandler_OnDestroyObject;

	/* only callbacks that have more entensive work needs a context switch on the ThreadPool */
	retval->Token_Msa->ContextSwitchBitMask |= (MSA_CDS_BROWSE | MSA_CDS_SEARCH | MSA_CDS_CREATEOBJECT | MSA_CDS_DESTROYOBJECT);

	/* register a virtual directory specific for upload */

	ILibWebServer_RegisterVirtualDirectory(
	retval->Token_WebServer,
	DMS_UPLOAD_VIRTUAL_DIRNAME,
	DMS_UPLOAD_VIRTUAL_DIRNAME_LEN,
	_DMS_WebServerHandler_OnContentUpload,
	retval);

	/* initialize upload request table keyed by the upload directive */
	retval->UploadRequests = ILibInitHashTree();
	#endif
	

	/*
	 *	Obtain initial set of IP addresses and update MSA
	 */
	sem_init(&(retval->IPAddressLock), 0, 1);
	sem_wait(&(retval->IPAddressLock));
	retval->IPAddressLength = ILibGetLocalIPAddressList(&(retval->IPAddressList));
	MSA_UpdateIpInfo(retval->Token_Msa, retval->IPAddressList, retval->IPAddressLength);
	sem_post(&(retval->IPAddressLock));

	{
		char* addrList = _IPAddressListToString(retval->IPAddressList, retval->IPAddressLength);
		printf("IP Address List: %s\n", addrList);
		free(addrList);
	}


	/* add the semaphore */
	sem_init(&(retval->Lock), 0, 1);

	/* TODO: initialize your back-end */
	retval->Token_BackEnd = FSE_InitFSEState(retval->Token_ThreadChain, back_end_init->Path, DMS_VIRTUAL_DIRNAME);

	retval->Destroy = DMS_DestroyMediaServer;

	/* Add retval to the chain so it can be destroyed */
	ILibAddToChain(thread_chain, retval);

	return retval;
}

void DMS_SetUserObj(DMS_StateObj dms_obj, void* dms_user_obj)
{
	if(dms_obj != NULL)
	{
		((struct DMS_State*)dms_obj)->Token_DmsUserObj = dms_user_obj;
	}
}

void* DMS_GetUserObj(DMS_StateObj dms_obj)
{
	if(dms_obj != NULL)
	{
		return ((struct DMS_State*)dms_obj)->Token_DmsUserObj;
	}
	return NULL;
}

void DMS_Lock(DMS_StateObj dms_obj)
{
	if(dms_obj != NULL)
	{
		sem_wait(&(((struct DMS_State*)dms_obj)->Lock));
	}
}

void DMS_UnLock(DMS_StateObj dms_obj)
{
	if(dms_obj != NULL)
	{
		sem_post(&(((struct DMS_State*)dms_obj)->Lock));
	}
}

void DMS_NotifyIPAddressChange(DMS_StateObj dms_obj)
{
	if(dms_obj != NULL)
	{
		struct DMS_State* dms = (struct DMS_State*)dms_obj;
		MediaServer_IPAddressListChanged(dms->Token_Upnp);
	}
}
