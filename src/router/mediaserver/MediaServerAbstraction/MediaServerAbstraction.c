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
 * $Workfile: MediaServerAbstraction.c
 *
 *
 *
 */

#ifdef _POSIX
#define stricmp strcasecmp
#define strcmpi strcasecmp
#define strnicmp strncasecmp
#endif

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#ifndef _WIN32_WCE
#include <crtdbg.h>
#endif
#else
#include <stdlib.h>
#endif


#if defined(WINSOCK2)
#include <winsock2.h>
#elif defined(WINSOCK1)
#include <winsock.h>
#endif

#include <stdio.h>
#include <string.h>
#include "ILibAsyncSocket.h"
#include "ILibWebServer.h"
#include "MediaServer_MicroStack.h"
#include "ILibParsers.h"
#include "MediaServerAbstraction.h"
#include "CdsDidlSerializer.h"
#include "CdsErrors.h"
#include "CdsStrings.h"
#include "FileIoAbstraction.h"


#ifdef WIN32
#ifndef UNDER_CE
#include "assert.h"
#include <crtdbg.h>
#endif
#endif

#ifdef _POSIX
#include "assert.h"
#include <semaphore.h>
#define strnicmp strncasecmp
#endif

#ifndef _WIN32_WCE
#define ASSERT(x)
#endif

struct MSA_InternalState
{
	/*
	 *	Lock for thread-safety.
	 */
	sem_t				Lock;

	/*
	 *	SystemUpdateID value of the media server.
	 */
	unsigned int		SystemUpdateID;

	/*
	 *	Linked list of container update IDs to print.
	 */
	struct MSA_ContainerUpdate *ContainerUpdateID;

	/*
	 *	Moderation flag for eventing.
	 */
	unsigned short				ModerationFlag;

	/*
	 *	Supported abilities for sorting browse/search results.
	 *	SortCapabilitiesString:		concatenation of strings in MSA_CONFIG_SORT_CAPABILITIES
	 */
	char	*SortCapabilitiesString;

	/*
	 *	Supported abilities for searching results.
	 *	SearchCapabilitiesString:	concatenation of strings in MSA_CONFIG_SEARCH_CAPABILITIES
	 */
	char	*SearchCapabilitiesString;

	/*
	 *	Contains the list of IP addresses for the media server.
	 */
	int		*IpAddrList;
	int		IpAddrListLen;

	/*
	 *	The list of protocolInfo for this device as a source.
	 */
	char	*SourceProtocolInfo;

	/*
	 *	The list of protocolInfo for this device as a sink.
	 */
	char	*SinkProtocolInfo;

	/*
	 *	Miscellaneous object that is provided in MSA_CreateMediaServer.
	 */
	void	*UserObject;

	/*
	 *	Statistic Information.
	 */
	struct	MSA_Stats						*Statistics;
};

struct MSA_ContainerUpdate
{
	char *ContainerID;
	unsigned int UpdateID;
	struct MSA_ContainerUpdate *Next;
};

/* Internal Structure for calling methods through the thread pool.          */
#ifdef WIN32
typedef unsigned __int64 METHOD_PARAM;
#else
typedef void *METHOD_PARAM;
#endif
typedef struct _contextSwitchCall
{
	MSA									MSA_Object;
	void*								UPnPSession;
	MSA_ActionHandlerContextSwitchId	ActionId;
	int									ParameterCount;
	METHOD_PARAM						Parameters[16];
} *ContextSwitchCall;

MSA_Error _ExecuteCallbackThroughThreadPool(MSA msa_obj, ContextSwitchCall context_switch);

int _AddMethodParameter(ContextSwitchCall context_switch, METHOD_PARAM parameter)
{
	if(context_switch->ParameterCount == 16)
    {
        return -1;
    }

	context_switch->Parameters[context_switch->ParameterCount++] = parameter;

    return 0;
}

ContextSwitchCall _CreateMethod(MSA_ActionHandlerContextSwitchId action_id, MSA msa_obj, void* upnp_session)
{
    ContextSwitchCall result =  malloc(sizeof(struct _contextSwitchCall));
	memset(result, 0, sizeof(struct _contextSwitchCall));
    if(result != NULL)
    {
		result->UPnPSession = upnp_session;
		result->ActionId = action_id;
		result->MSA_Object = msa_obj;
    }
    return result;
}


/*
 *	This method release the resources occupied
 *		msaObj					: The object returned from CreateMediaServer
 */
void MSA_DestroyMediaServer(void *msa_obj)
{
	MSA msa = (MSA) msa_obj;
	struct MSA_ContainerUpdate *cu, *nextCu;
	struct MSA_InternalState *state;

	state = (struct MSA_InternalState*) msa->InternalState;

	/* properly deallocate the objects for the container UpdateIDs */
	sem_wait(&(state->Lock));
	msa->ThreadPool = NULL;

	if(state->Statistics != NULL)
	{
		free(state->Statistics);
	}

	cu = state->ContainerUpdateID;
	while (cu != NULL)
	{
		nextCu = cu->Next;
		free(cu);
		cu = nextCu;
	}

	msa->OnStats = NULL;
	msa->OnBrowse = NULL;
	msa->OnSearch = NULL;
	msa->OnGetSystemUpdateID = NULL;
	msa->OnGetSearchCapabilities = NULL;
	msa->OnGetSortCapabilities = NULL;
	msa->OnGetProtocolInfo = NULL;
	msa->OnGetCurrentConnectionIDs = NULL;
	msa->OnGetCurrentConnectionInfo = NULL;

	#if defined(INCLUDE_FEATURE_UPLOAD)
	msa->OnCreateObject = NULL;
	msa->OnDestroyObject = NULL;
	#endif

	free(state->SinkProtocolInfo);
	free(state->SourceProtocolInfo);
	free(state->SearchCapabilitiesString);
	free(state->SortCapabilitiesString);
	free(state->IpAddrList);

	sem_post(&(state->Lock));
	sem_destroy(&(state->Lock));

	free(state);
	msa->InternalState = NULL;
}

void _MSA_Helper_PopulateIpInfo(MSA_State msa_state, void* upnp_session, struct MSA_CdsQuery *cdsQuery)
{
	int size, i, swapValue;
	struct MSA_InternalState* state = (struct MSA_InternalState*) msa_state;

	/*
	 *	Obtain the IP address and port that received this request
	 */
	cdsQuery->RequestedOnAddress = MediaServer_GetLocalInterfaceToHost(upnp_session);
	cdsQuery->RequestedOnPort = MediaServer_GetLocalPortNumber(upnp_session);

	/*
	 *	Obtain the list of active IP addresses for this machine.
 	 *	Microstack allows us to assume that the port number
	 *	will be the same for all IP addresses.
	 */
	sem_wait(&(state->Lock));
	cdsQuery->IpAddrListLen = state->IpAddrListLen;
	size = (int) (sizeof(int) * cdsQuery->IpAddrListLen);
	cdsQuery->IpAddrList = (int*) malloc(size);
	memcpy(cdsQuery->IpAddrList, state->IpAddrList, size);
	sem_post(&(state->Lock));

	/*
	 *	Reorder the list of active IP addresses so that the
	 *	IP address for the interface that received the request
	 *	is listed first.
	 */
	if (cdsQuery->IpAddrList[0] != cdsQuery->RequestedOnAddress)
	{
		swapValue = cdsQuery->IpAddrList[0];
		cdsQuery->IpAddrList[0] = cdsQuery->RequestedOnAddress;
		for (i=1; i < cdsQuery->IpAddrListLen; i++)
		{
			if (cdsQuery->IpAddrList[i] == cdsQuery->RequestedOnAddress)
			{
				cdsQuery->IpAddrList[i] = swapValue;
				break;
			}
		}
	}
}

/* number of bytes needed to represent a 32bit unsigned int as a string with a comma. */
#define MSA_MAX_BYTESIZE_UINT 13
void MSA_Helper_UpdateImmediate_ContainerUpdateID(MSA msa_obj)
{
	int size;
	struct MSA_ContainerUpdate *cu, *nextCu;
	char *sv, *var;
	int writecomma;
	struct MSA_InternalState *state;

	state = (struct MSA_InternalState*) msa_obj->InternalState;

	/* don't bother updating if there's nothing to report */
	if (state->ContainerUpdateID == NULL) return;

	size = 0;

	/* calculate size needed for state variable value */
	cu = state->ContainerUpdateID;
	while (cu != NULL)
	{
		size += (int) (MSA_MAX_BYTESIZE_UINT + strlen(cu->ContainerID));
	}
	size++;

	/*
	 *	Acquire the value of the state variable by writing it to 'var.
	 *
	 *	We progressively write 'var' by writing to 'sv'. The format
	 *	of the state variable is a comma-delimited list of
	 *	containerID/UpdateID pairs... of course, the spec authors
	 *	also made the delimiter between ContainerID and UpdateID
	 *	into a comma... how silly.
	 */
	cu = state->ContainerUpdateID;
	var = sv = (char*) malloc(size);
	writecomma = 0;
	while (cu != NULL)
	{
		nextCu = cu->Next;

		if (writecomma != 0) { sv += sprintf(sv, ","); }
		sv += sprintf(sv, "%s,%u", cu->ContainerID, cu->UpdateID);

		writecomma = 1;
		free(cu);
		cu = nextCu;
	}
	state->ContainerUpdateID = NULL;

	ASSERT(sv <= var+size);
	free(var);	
}

void MSA_Helper_ModerationSink_ContainerUpdateID(MSA msa_obj)
{
	struct MSA_InternalState *state;
	state = (struct MSA_InternalState*) msa_obj->InternalState;

	#ifdef _DEBUG
	printf("MSA_Helper_ModerationSink_ContainerUpdateID()\r\n");
	#endif

	sem_wait(&(state->Lock));

	MSA_Helper_UpdateImmediate_ContainerUpdateID(msa_obj);
	state->ModerationFlag = 0;

	sem_post(&(state->Lock));
}

/*
 *	Assumes caller has locked msa->Lock.
 */
void MSA_Helper_UpdateModerated_ContainerUpdateID(MSA msa_obj)
{
	/* don't bother updating if there's nothing to report */
	struct MSA_InternalState *state;
	state = (struct MSA_InternalState*) msa_obj->InternalState;

	if (state->ContainerUpdateID == NULL) return;

	if (state->ModerationFlag == 0)
	{
		state->ModerationFlag = 1;
		MSA_Helper_UpdateImmediate_ContainerUpdateID(msa_obj);
		ILibLifeTime_Add(msa_obj->LifeTimeMonitor,state,1,MSA_Helper_ModerationSink_ContainerUpdateID, NULL);
	}
}

/* END SECTION - helper functions */
/************************************************************************************/



/************************************************************************************/
/* START SECTION - Dispatch sinks generated in original main.c */
/* entry point from generated code */
void MediaServer_ContentDirectory_Browse(void* upnptoken,char* ObjectID,char* BrowseFlag,char* Filter,unsigned int StartingIndex,unsigned int RequestedCount,char* SortCriteria)
{
	struct MSA_CdsQuery *browseArgs;
	int errorCode;
	const char *errorMsg;
	enum MSA_Enum_QueryTypes browseChildren = 0;
    ContextSwitchCall method = NULL;

	struct MSA_InternalState* state;
	MSA msa = (MSA) MediaServer_GetTag(((struct ILibWebServer_Session*)upnptoken)->User);
	state = (struct MSA_InternalState*) msa->InternalState;


	#ifdef _DEBUG
	printf("UPnP Invoke: MediaServer_ContentDirectory_Browse();\r\n");
	if (msa->OnStats != NULL && state!=NULL)
	{
		state->Statistics->Count_Browse++;
		msa->OnStats(msa, state->Statistics);
	}
	#endif

	/*
	 *	Validate arguments.
	 */

	errorCode = 0;
	errorMsg = NULL;
	if (strcmpi(BrowseFlag, CDS_STRING_BROWSE_DIRECT_CHILDREN) == 0)
	{
		browseChildren = MSA_Query_BrowseDirectChildren;
	}
	else if (strcmpi(BrowseFlag, CDS_STRING_BROWSE_METADATA) == 0)
	{
		browseChildren = MSA_Query_BrowseMetadata;
	}
	else
	{
		fprintf(stderr, "WARNING: MediaServer_ContentDirectory_Browse(): Possible error with generated microstack. Encountered BrowseFlag='%s'\r\n", BrowseFlag);
		errorCode = CDS_EC_INVALID_BROWSEFLAG;
		errorMsg = CDS_EM_INVALID_BROWSEFLAG;
	}

	if ((errorCode != 0) || (errorMsg != NULL))
	{
		/* ensure that the error code and message map to something */
		if (errorCode == 0) { errorCode = CDS_EC_INTERNAL_ERROR; }
		if (errorMsg == NULL) {	errorMsg = CDS_EM_INTERNAL_ERROR; }

		MediaServer_Response_Error(upnptoken, errorCode, errorMsg);
	}
	else
	{
		/* deserialize request */

		/*
		 *	Input arguments valid at UPnP layer.
		 *	Create an MSA_CdsQuery object and execute
		 *	the browse callback so that application can return results.
		 */

		browseArgs = (struct MSA_CdsQuery*) malloc (sizeof(struct MSA_CdsQuery));
		memset(browseArgs, 0, sizeof(struct MSA_CdsQuery));

		browseArgs->QueryType = browseChildren;
		browseArgs->Filter = ILibString_Copy(Filter, -1);
		browseArgs->ObjectID = ILibString_Copy(ObjectID, -1);

		/* Be sure to unescape it first. */
		ILibInPlaceXmlUnEscape(browseArgs->ObjectID);

		browseArgs->RequestedCount = RequestedCount;
		browseArgs->SortCriteria = ILibString_Copy(SortCriteria, -1);

		browseArgs->StartingIndex = StartingIndex;
		browseArgs->IpAddrList = NULL;
		_MSA_Helper_PopulateIpInfo(msa->InternalState, upnptoken, browseArgs);

		method = _CreateMethod(MSA_CDS_BROWSE, msa, upnptoken);
		_AddMethodParameter(method, (METHOD_PARAM) browseArgs);
		if(_ExecuteCallbackThroughThreadPool(msa, method) != MSA_ERROR_NONE)
		{
			MediaServer_Response_Error(upnptoken, CDS_EC_INTERNAL_ERROR, CDS_EM_INTERNAL_ERROR);
		}
	}
}

void MediaServer_ContentDirectory_Search(void* upnptoken,char* ContainerID,char* SearchCriteria, char* Filter,unsigned int StartingIndex,unsigned int RequestedCount,char* SortCriteria)
{
	struct MSA_CdsQuery *searchArgs;
    ContextSwitchCall method = NULL;

	struct MSA_InternalState* state;
	MSA msa = (MSA) MediaServer_GetTag(((struct ILibWebServer_Session*)upnptoken)->User);
	state = (struct MSA_InternalState*) msa->InternalState;

	#ifdef _DEBUG
	printf("UPnP Invoke: MediaServer_ContentDirectory_Search();\r\n");
	if (msa->OnStats != NULL && state!=NULL)
	{
		state->Statistics->Count_Search++;
		msa->OnStats(msa, state->Statistics);
	}

	#endif

	/*
	 *	Validate arguments.
	 */

	/*
	 *	Input arguments valid at UPnP layer.
	 *	Create an MSA_CdsQuery object and execute
	 *	the browse callback so that application can return results.
	 */

	searchArgs = (struct MSA_CdsQuery*) malloc (sizeof(struct MSA_CdsQuery));
	memset(searchArgs, 0, sizeof(struct MSA_CdsQuery));

	searchArgs->QueryType = MSA_Query_Search;
	
	searchArgs->Filter = ILibString_Copy(Filter, -1);
	searchArgs->ObjectID = ILibString_Copy(ContainerID, -1);

	/* Be sure to unescape it first. */
	ILibInPlaceXmlUnEscape(searchArgs->ObjectID);

	searchArgs->SearchCriteria = ILibString_Copy(SearchCriteria, -1);

	searchArgs->RequestedCount = RequestedCount;
	searchArgs->SortCriteria = ILibString_Copy(SortCriteria, -1);

	searchArgs->StartingIndex = StartingIndex;
	searchArgs->IpAddrList = NULL;
	_MSA_Helper_PopulateIpInfo(msa->InternalState, upnptoken, searchArgs);

	method = _CreateMethod(MSA_CDS_SEARCH, msa, upnptoken);
	_AddMethodParameter(method, (METHOD_PARAM) searchArgs);
	if(_ExecuteCallbackThroughThreadPool(msa, method) != MSA_ERROR_NONE)
	{
		MediaServer_Response_Error(upnptoken, CDS_EC_INTERNAL_ERROR, CDS_EM_INTERNAL_ERROR);
	}
}

void MediaServer_ContentDirectory_GetSystemUpdateID(void* upnptoken)
{
	/*
	 *	Reports the known SystemUpdateID.
	 *	The SystemUpdateID is changed through MSA_IncrementSystemUpdateID().
	 */
	struct MSA_InternalState* state;
    ContextSwitchCall method = NULL;

	MSA msa = (MSA) MediaServer_GetTag(((struct ILibWebServer_Session*)upnptoken)->User);
	state = (struct MSA_InternalState*) msa->InternalState;

	#ifdef _DEBUG
	printf("UPnP Invoke: MediaServer_ContentDirectory_GetSystemUpdateID();\r\n");
	if (msa->OnStats != NULL && state!=NULL)
	{
		state->Statistics->Count_GetSystemUpdateID++;
		msa->OnStats(msa, state->Statistics);
	}
	#endif

	method = _CreateMethod(MSA_CDS_GETSYSTEMUPDATEID, msa, upnptoken);
	if(_ExecuteCallbackThroughThreadPool(msa, method) != MSA_ERROR_NONE)
	{
		MediaServer_Response_Error(upnptoken, CDS_EC_INTERNAL_ERROR, CDS_EM_INTERNAL_ERROR);
	}
}

void MediaServer_ContentDirectory_GetSearchCapabilities(void* upnptoken)
{
	/*
	 *	Reports the statically defined search capabilities of the MediaServer.
	 *	You can customize this value to the abilities of the backend database
	 *	by changing MSA_CONFIG_SEARCH_CAPABILITIES_xxx variables.
	 */
	struct MSA_InternalState* state;
    ContextSwitchCall method = NULL;

	MSA msa = (MSA) MediaServer_GetTag(((struct ILibWebServer_Session*)upnptoken)->User);
	state = (struct MSA_InternalState*) msa->InternalState;	

	#ifdef _DEBUG
	printf("UPnP Invoke: MediaServer_ContentDirectory_GetSearchCapabilities();\r\n");
	if (msa->OnStats != NULL && state!=NULL)
	{
		state->Statistics->Count_GetSearchCapabilities++;
		msa->OnStats(msa, state->Statistics);
	}
	#endif

	method = _CreateMethod(MSA_CDS_GETSEARCHCAPABILITIES, msa, upnptoken);
	if(_ExecuteCallbackThroughThreadPool(msa, method) != MSA_ERROR_NONE)
	{
		MediaServer_Response_Error(upnptoken, CDS_EC_INTERNAL_ERROR, CDS_EM_INTERNAL_ERROR);
	}
}

void MediaServer_ContentDirectory_GetSortCapabilities(void* upnptoken)
{
	/*
	 *	Reports the statically defined sort capabilities of the MediaServer.
	 *	You can customize this value to the abilities of the backend database
	 *	by changing MSA_CONFIG_SORT_CAPABILITIES_xxx variables.
	 */
	struct MSA_InternalState* state;
    ContextSwitchCall method = NULL;

	MSA msa = (MSA) MediaServer_GetTag(((struct ILibWebServer_Session*)upnptoken)->User);
	state = (struct MSA_InternalState*) msa->InternalState;

	#ifdef _DEBUG
	printf("UPnP Invoke: MediaServer_ContentDirectory_GetSortCapabilities();\r\n");
	if (msa->OnStats != NULL && state!=NULL)
	{
		state->Statistics->Count_GetSortCapabilities++;
		msa->OnStats(msa, state->Statistics);
	}
	#endif

	method = _CreateMethod(MSA_CDS_GETSORTCAPABILITIES, msa, upnptoken);
	if(_ExecuteCallbackThroughThreadPool(msa, method) != MSA_ERROR_NONE)
	{
		MediaServer_Response_Error(upnptoken, CDS_EC_INTERNAL_ERROR, CDS_EM_INTERNAL_ERROR);
	}
}

void MediaServer_ConnectionManager_GetProtocolInfo(void* upnptoken)
{
	struct MSA_InternalState* state;
    ContextSwitchCall method = NULL;

	MSA msa = (MSA) MediaServer_GetTag(((struct ILibWebServer_Session*)upnptoken)->User);
	state = (struct MSA_InternalState*) msa->InternalState;

	#ifdef _DEBUG
	printf("UPnP Invoke: MediaServer_ConnectionManager_GetProtocolInfo();\r\n");
	if (msa->OnStats != NULL && state!=NULL)
	{
		state->Statistics->Count_GetProtocolInfo++;
		msa->OnStats(msa, state->Statistics);
	}
	#endif

	method = _CreateMethod(MSA_CMS_GETPROTOCOLINFO, msa, upnptoken);
	if(_ExecuteCallbackThroughThreadPool(msa, method) != MSA_ERROR_NONE)
	{
		MediaServer_Response_Error(upnptoken, CDS_EC_INTERNAL_ERROR, CDS_EM_INTERNAL_ERROR);
	}
}

void MediaServer_ConnectionManager_GetCurrentConnectionIDs(void* upnptoken)
{
	/*
	 *	HTTP connections are stateless, from the perspective of UPnP AV.
	 *	This is largely because we can't really monitor connection lifetime
	 *	of HTTP traffic in the UPnP AV sense, without risking memory leaks.
	 *
	 *	TODO: Low priority - Add support for connection-lifetime aware protocols.
	 */
	struct MSA_InternalState* state;
    ContextSwitchCall method = NULL;

	MSA msa = (MSA) MediaServer_GetTag(((struct ILibWebServer_Session*)upnptoken)->User);
	state = (struct MSA_InternalState*) msa->InternalState;

	#ifdef _DEBUG
	printf("UPnP Invoke: MediaServer_ConnectionManager_GetCurrentConnectionIDs();\r\n");
	if (msa->OnStats != NULL && state!=NULL)
	{
		state->Statistics->Count_GetCurrentConnectionIDs++;
		msa->OnStats(msa, state->Statistics);
	}
	#endif

	method = _CreateMethod(MSA_CMS_GETCURRENTCONNECTIONIDS, msa, upnptoken);
	if(_ExecuteCallbackThroughThreadPool(msa, method) != MSA_ERROR_NONE)
	{
		MediaServer_Response_Error(upnptoken, CDS_EC_INTERNAL_ERROR, CDS_EM_INTERNAL_ERROR);
	}
}

void MediaServer_ConnectionManager_GetCurrentConnectionInfo(void* upnptoken,int ConnectionID)
{
	/*
	 *	HTTP connections are stateless, from the perspective of UPnP AV.
	 *	This is largely because we can't really monitor connection lifetime
	 *	of HTTP traffic in the UPnP AV sense, without risking memory leaks.
	 *
	 *	TODO: Low priority - Add support for connection-lifetime aware protocols.
	 */
	struct MSA_InternalState* state;
    ContextSwitchCall method = NULL;

	MSA msa = (MSA) MediaServer_GetTag(((struct ILibWebServer_Session*)upnptoken)->User);
	state = (struct MSA_InternalState*) msa->InternalState;

	#ifdef _DEBUG
	printf("UPnP Invoke: MediaServer_ConnectionManager_GetCurrentConnectionInfo(%u);\r\n",ConnectionID);
	if (msa->OnStats != NULL && state!=NULL)
	{
		state->Statistics->Count_GetCurrentConnectionInfo++;
		msa->OnStats(msa, state->Statistics);
	}
	#endif

	method = _CreateMethod(MSA_CMS_GETCURRENTCONNECTIONINFO, msa, upnptoken);
	_AddMethodParameter(method, (METHOD_PARAM) ConnectionID);
	if(_ExecuteCallbackThroughThreadPool(msa, method) != MSA_ERROR_NONE)
	{
		MediaServer_Response_Error(upnptoken, CDS_EC_INTERNAL_ERROR, CDS_EM_INTERNAL_ERROR);
	}
}

/* END SECTION - Dispatch sinks generated in original main.c */
/************************************************************************************/






/************************************************************************************/
/* START SECTION - public methods*/





/*****************************************************************************/
/* UPLOAD CAPABILITY														 */
/*****************************************************************************/
#if defined(INCLUDE_FEATURE_UPLOAD)
void _MSA_Helper_PopulateIpInfoEx(MSA_State msa_state, void* upnp_session, struct MSA_CdsCreateObj *create_obj_arg)
{
	int size, i, swapValue;
	struct MSA_InternalState* state = (struct MSA_InternalState*) msa_state;

	/*
	 *	Obtain the IP address and port that received this request
	 */
	create_obj_arg->RequestedOnAddress = MediaServer_GetLocalInterfaceToHost(upnp_session);
	create_obj_arg->RequestedOnPort = MediaServer_GetLocalPortNumber(upnp_session);

	/*
	 *	Obtain the list of active IP addresses for this machine.
 	 *	Microstack allows us to assume that the port number
	 *	will be the same for all IP addresses.
	 */
	sem_wait(&(state->Lock));
	create_obj_arg->IpAddrListLen = state->IpAddrListLen;
	size = (int) (sizeof(int) * create_obj_arg->IpAddrListLen);
	create_obj_arg->IpAddrList = (int*) malloc(size);
	memcpy(create_obj_arg->IpAddrList, state->IpAddrList, size);
	sem_post(&(state->Lock));

	/*
	 *	Reorder the list of active IP addresses so that the
	 *	IP address for the interface that received the request
	 *	is listed first.
	 */
	if (create_obj_arg->IpAddrList[0] != create_obj_arg->RequestedOnAddress)
	{
		swapValue = create_obj_arg->IpAddrList[0];
		create_obj_arg->IpAddrList[0] = create_obj_arg->RequestedOnAddress;
		for (i=1; i < create_obj_arg->IpAddrListLen; i++)
		{
			if (create_obj_arg->IpAddrList[i] == create_obj_arg->RequestedOnAddress)
			{
				create_obj_arg->IpAddrList[i] = swapValue;
				break;
			}
		}
	}
}

void MediaServer_ContentDirectory_CreateObject(void* upnptoken, char* id, char* Elements)
{
	struct MSA_CdsCreateObj *createObjArg;
	struct ILibXMLNode* nodeList;
	struct ILibXMLNode* node;
	struct ILibXMLAttribute *attribs;
	
	int resultLen;
	int parsePeerResult = 0;
	char *lastResultPos;

	int isItemFlag = 0;
	struct CdsObject *newObj;

	struct MSA_InternalState* state;
    ContextSwitchCall method = NULL;

	MSA msa = (MSA) MediaServer_GetTag(((struct ILibWebServer_Session*)upnptoken)->User);
	state = (struct MSA_InternalState*) msa->InternalState;

	#ifdef _DEBUG
	printf("UPnP Invoke: MediaServer_ContentDirectory_CreateObject();\r\n");
	#endif

	if((id != NULL)&& (Elements != NULL))
	{
		/* parse the didl Result into a CdsObject, */
		newObj = NULL;
		resultLen = (int) strlen(Elements);

		lastResultPos = Elements + resultLen;
		nodeList = ILibParseXML(Elements, 0, resultLen);
		ILibXML_BuildNamespaceLookupTable(nodeList);
		parsePeerResult = ILibProcessXMLNodeList(nodeList);

		if (parsePeerResult != 0 || resultLen == 0)
		{
			MediaServer_Response_Error(upnptoken, 712, "Bad Metadata - Cannot parse DIDL-Lite.");
			ILibDestructXMLNodeList(nodeList);
			return;
		}
		else
		{
			node = nodeList;
			while (node != NULL)
			{
				if (node->StartTag != 0)
				{
					/*[DONOTREPARSE] null terminate string */
					attribs = ILibGetXMLAttributes(node);
					node->Name[node->NameLength] = '\0';

					newObj = NULL;
					if (strcmp(node->Name, CDS_TAG_CONTAINER) == 0)
					{
						newObj = CDS_DeserializeDidlToObject(node, attribs, 0, Elements, lastResultPos);
					}
					else if (strcmp(node->Name, CDS_TAG_ITEM) == 0)
					{
						newObj = CDS_DeserializeDidlToObject(node, attribs, 1, Elements, lastResultPos);
						isItemFlag = 1;
					}
					else if (strcmp(node->Name, CDS_TAG_DIDL) == 0)
					{
						/* this is didl-lite root node, go to first child */
						node = node->Next;
					}
					else
					{
						/* this node is not supported, go to next sibling/peer */
						if (node->Peer != NULL)
						{
							node = node->Peer;
						}
						else if(node->Parent!=NULL)
						{
							node = node->Parent->Peer;
						}
						else
						{
							node = NULL;
						}
					}

					/* free attribute mappings */
					ILibDestructXMLAttributeList(attribs);
					if(newObj != NULL)
					{
						break;
					}
				}
				else
				{
					node = node->Next;
				}
			}
		}

		if(newObj != NULL)
		{
			createObjArg = (struct MSA_CdsCreateObj*) malloc (sizeof(struct MSA_CdsCreateObj));
			memset(createObjArg, 0, sizeof(struct MSA_CdsCreateObj));

			createObjArg->UserObject = NULL;
			createObjArg->IpAddrList = NULL;
			_MSA_Helper_PopulateIpInfoEx(msa->InternalState, upnptoken, createObjArg);

			method = _CreateMethod(MSA_CDS_CREATEOBJECT, msa, upnptoken);
			_AddMethodParameter(method, (METHOD_PARAM) createObjArg);
			_AddMethodParameter(method, (METHOD_PARAM) newObj);
			_ExecuteCallbackThroughThreadPool(msa, method);
		}
		else
		{
			MediaServer_Response_Error(upnptoken, 712, "Bad Metadata - Cannot parse DIDL-Lite.");
		}

		/* free resources from XML parsing */
		ILibDestructXMLNodeList(nodeList);
	}
	else
	{
		MediaServer_Response_Error(upnptoken, 402, "Missing value for the 'Elements' argument in the CreateObject");
	}

}

void MediaServer_ContentDirectory_DestroyObject(void* upnptoken, char* ObjectID)
{
	struct MSA_CdsDestroyObj *destroyObjArg;
	struct MSA_InternalState* state;
    ContextSwitchCall method = NULL;

	MSA msa = (MSA) MediaServer_GetTag(((struct ILibWebServer_Session*)upnptoken)->User);
	state = (struct MSA_InternalState*) msa->InternalState;

	#ifdef _DEBUG
	printf("UPnP Invoke: MediaServer_ContentDirectory_DestroyObject();\r\n");
	#endif

	if(ObjectID == NULL || strlen(ObjectID) == 0)
	{
		MediaServer_Response_Error(upnptoken, 402, "Missing value for the 'ObjectID' argument in the DestroyObject");
		return;
	}

	destroyObjArg = (struct MSA_CdsDestroyObj*) malloc (sizeof(struct MSA_CdsDestroyObj));
	memset(destroyObjArg, 0, sizeof(struct MSA_CdsDestroyObj));
	destroyObjArg->ObjectID = ILibString_Copy(ObjectID, -1);
	destroyObjArg->UserObject = NULL;

	method = _CreateMethod(MSA_CDS_DESTROYOBJECT, msa, upnptoken);
	_AddMethodParameter(method, (METHOD_PARAM) destroyObjArg);
	_ExecuteCallbackThroughThreadPool(msa, method);
}

int MSA_ForCreateObjectResponse_Accept(MSA msa_obj, void* upnp_session, struct MSA_CdsCreateObj* create_obj_arg, struct CdsObject* cds_obj)
{
	char *didl;
	int didlLen;
	unsigned int filter = 0;

	/* ToDo:  check if uploadFileName is set to NULL,
		if so, the Application has rejected the upload */

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

	/* we want to print all Res elements and DLNA-specific attributes */
	filter |= CdsFilter_ResAllAttribs;
	filter |= CdsFilter_DlnaContainerType;
	filter |= CdsFilter_DlnaManaged;

	/* convert CDS object to DIDL-Lite */
	didl = CDS_SerializeObjectToDidl(cds_obj, METADATA_IS_XML_ESCAPED, filter, 1, &didlLen);

	if(strlen(didl) == 0)
	{
		/*
		*	print an error message if we couldn't get the DIDL.
		*/
		fprintf(stderr, " failed to serialize object %s. Reason=%d.\r\n", cds_obj->ID, didlLen);
		/*
		*	an error occurred during the response
		*/
		MediaServer_Response_Error(upnp_session, 700, "Error - Deserializing response to DIDL-Lite format.");
	}
	else
	{
		MediaServer_AsyncResponse_START(upnp_session, CDS_STRING_CREATE_OBJECT, CDS_STRING_URN_CDS);
		MediaServer_AsyncResponse_OUT(upnp_session, "ObjectID", cds_obj->ID, (int)strlen(cds_obj->ID), ILibAsyncSocket_MemoryOwnership_USER, 1, 1);
		MediaServer_AsyncResponse_OUT(upnp_session, "Result", didl, (int)strlen(didl), ILibAsyncSocket_MemoryOwnership_USER, 1, 1);
		MediaServer_AsyncResponse_DONE(upnp_session, CDS_STRING_CREATE_OBJECT);
	}

	/* Free resources */

	ILibWebServer_Release(upnp_session);
	free(didl);
	free(create_obj_arg->IpAddrList);
	free(create_obj_arg);

	return 0;
}

int MSA_ForCreateObjectResponse_AcceptUpload(MSA msa_obj, void* upnp_session, struct MSA_CdsCreateObj* create_obj_arg, struct CdsObject* cds_obj)
{
	char *didl;
	int didlLen;
	unsigned int filter = 0;


	/* ToDo:  check if uploadFileName is set to NULL,
		if so, the Application has rejected the upload */

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


	/* we want to print all Res elements and DLNA-specific attributes */
	filter |= CdsFilter_ResAllAttribs;
	filter |= CdsFilter_DlnaContainerType;
	filter |= CdsFilter_DlnaManaged;

	/* convert CDS object to DIDL-Lite */
	didl = CDS_SerializeObjectToDidl(cds_obj, METADATA_IS_XML_ESCAPED, filter, 1, &didlLen);

	if(strlen(didl) == 0)
	{
		/*
		*	print an error message if we couldn't get the DIDL.
		*/
		fprintf(stderr, " failed to serialize object %s. Reason=%d.\r\n", cds_obj->ID, didlLen);
		/*
		*	an error occurred during the response
		*/
		MediaServer_Response_Error(upnp_session, 700, "Error - Deserializing response to DIDL-Lite format.");
	}
	else
	{
		MediaServer_AsyncResponse_START(upnp_session, CDS_STRING_CREATE_OBJECT, CDS_STRING_URN_CDS);
		MediaServer_AsyncResponse_OUT(upnp_session, "ObjectID", cds_obj->ID, (int)strlen(cds_obj->ID), ILibAsyncSocket_MemoryOwnership_USER, 1, 1);
		MediaServer_AsyncResponse_OUT(upnp_session, "Result", didl, (int)strlen(didl), ILibAsyncSocket_MemoryOwnership_USER, 1, 1);
		MediaServer_AsyncResponse_DONE(upnp_session, CDS_STRING_CREATE_OBJECT);
	}

	/* Free resources */

	ILibWebServer_Release(upnp_session);
	free(didl);
	free(create_obj_arg->IpAddrList);
	free(create_obj_arg);

	return 0;
}

int MSA_ForCreateObjectResponse_Reject(MSA msa_obj, void* upnp_session, struct MSA_CdsCreateObj* create_obj_arg, struct CdsObject* cds_obj, int error_code, char* error_msg)
{

	/*
	*	an error occurred during the response
	*/
	MediaServer_Response_Error(upnp_session, error_code, error_msg);

	/* Free resources */
	ILibWebServer_Release(upnp_session);
	free(create_obj_arg->IpAddrList);
	free(create_obj_arg);

	return 0;
}

#endif


/* see header file */
MSA MSA_CreateMediaServer(void* chain, MediaServer_MicroStackToken upnp_stack, void* lifetime_monitor,  unsigned int system_update_id, const char* sink_protocol_info, const char* source_protocol_info, const char* sortable_fields, const char* searchable_fields, ILibThreadPool thread_pool, void* msa_user_obj)
{

    MSA msa = NULL;
	struct MSA_InternalState* internalState = NULL;

    /* Allocate the MSA and internal state structures */
    msa = (MSA) malloc(sizeof(struct MSA_Instance));
	if(msa == NULL)
	{
		return NULL;
	}
	memset(msa, 0, sizeof(struct MSA_Instance));
	internalState = (struct MSA_InternalState* )malloc(sizeof(struct MSA_InternalState));
	if(internalState == NULL)
	{
		return NULL;
	}
	memset(internalState, 0, sizeof(struct MSA_InternalState));	

	internalState->Statistics = (struct MSA_Stats*) malloc(sizeof(struct MSA_Stats));
	memset(internalState->Statistics, 0, sizeof(struct MSA_Stats*));

	internalState->UserObject = msa_user_obj;
	sem_init(&(internalState->Lock), 0, 1);

	MediaServer_SetState_ContentDirectory_SystemUpdateID(upnp_stack, internalState->SystemUpdateID);
	MediaServer_SetState_ContentDirectory_ContainerUpdateIDs(upnp_stack, "");

	/* set initial source_protocol_info */
	internalState->SourceProtocolInfo = (char*) malloc(strlen(source_protocol_info) + 1);
	strcpy(internalState->SourceProtocolInfo, source_protocol_info);
	MediaServer_SetState_ConnectionManager_SourceProtocolInfo(upnp_stack, internalState->SourceProtocolInfo);

	/* set initial sink_protocol_info */
	internalState->SinkProtocolInfo = (char*) malloc(strlen(sink_protocol_info) + 1);
	strcpy(internalState->SinkProtocolInfo, sink_protocol_info);

	MediaServer_SetState_ConnectionManager_SinkProtocolInfo(upnp_stack,internalState->SinkProtocolInfo);

	/* no connections */
	MediaServer_SetState_ConnectionManager_CurrentConnectionIDs(upnp_stack, "");

	/* set sort capabilities */
	internalState->SortCapabilitiesString = (char*) malloc(strlen(sortable_fields) + 1);
	strcpy(internalState->SortCapabilitiesString, sortable_fields);

	/* set search cabilities */
	internalState->SearchCapabilitiesString = (char*) malloc(strlen(searchable_fields) + 1);
	strcpy(internalState->SearchCapabilitiesString, searchable_fields);

	msa->DeviceMicroStack = upnp_stack;
	msa->InternalState = internalState;
	msa->ThreadPool = thread_pool;
	msa->LifeTimeMonitor = lifetime_monitor;
	msa->Destroy = MSA_DestroyMediaServer;

	MediaServer_SetTag(upnp_stack, (void*)msa);
	ILibAddToChain(chain, msa);


	//
	// Initialize All the Handlers:
	//

	//
	// Connection Manager
	//
	MediaServer_FP_ConnectionManager_GetCurrentConnectionIDs = (MediaServer__ActionHandler_ConnectionManager_GetCurrentConnectionIDs)&MediaServer_ConnectionManager_GetCurrentConnectionIDs;
	MediaServer_FP_ConnectionManager_GetCurrentConnectionInfo = (MediaServer__ActionHandler_ConnectionManager_GetCurrentConnectionInfo)&MediaServer_ConnectionManager_GetCurrentConnectionInfo;
	MediaServer_FP_ConnectionManager_GetProtocolInfo = (MediaServer__ActionHandler_ConnectionManager_GetProtocolInfo)&MediaServer_ConnectionManager_GetProtocolInfo;

	//
	// Content Directory
	//
	MediaServer_FP_ContentDirectory_Browse = (MediaServer__ActionHandler_ContentDirectory_Browse)&MediaServer_ContentDirectory_Browse;
	MediaServer_FP_ContentDirectory_GetSearchCapabilities = (MediaServer__ActionHandler_ContentDirectory_GetSearchCapabilities)&MediaServer_ContentDirectory_GetSearchCapabilities;
	MediaServer_FP_ContentDirectory_GetSortCapabilities = (MediaServer__ActionHandler_ContentDirectory_GetSortCapabilities)&MediaServer_ContentDirectory_GetSortCapabilities;
	MediaServer_FP_ContentDirectory_GetSystemUpdateID = (MediaServer__ActionHandler_ContentDirectory_GetSystemUpdateID)&MediaServer_ContentDirectory_GetSystemUpdateID;
	MediaServer_FP_ContentDirectory_Search = (MediaServer__ActionHandler_ContentDirectory_Search)&MediaServer_ContentDirectory_Search;
#if defined(INCLUDE_FEATURE_UPLOAD)
	MediaServer_FP_ContentDirectory_CreateObject = (MediaServer__ActionHandler_ContentDirectory_CreateObject)&MediaServer_ContentDirectory_CreateObject;
	MediaServer_FP_ContentDirectory_DestroyObject = (MediaServer__ActionHandler_ContentDirectory_DestroyObject)&MediaServer_ContentDirectory_DestroyObject;
#else
	MediaServer_GetConfiguration()->ContentDirectory->CreateObject = NULL;
	MediaServer_GetConfiguration()->ContentDirectory->DestroyObject = NULL;
#endif

	return msa;
}



/* see header file */
void MSA_DeallocateCdsQuery(struct MSA_CdsQuery *cdsQuery)
{
	if (cdsQuery->Filter != NULL) free (cdsQuery->Filter);
	if (cdsQuery->ObjectID != NULL) free (cdsQuery->ObjectID);
	if (cdsQuery->SortCriteria != NULL) free (cdsQuery->SortCriteria);
	if (cdsQuery->SearchCriteria != NULL) free (cdsQuery->SearchCriteria);
	if (cdsQuery->IpAddrList != NULL) free (cdsQuery->IpAddrList);
	free (cdsQuery);
}

/* see header file */
void MSA_ForResponse_RespondError(MSA msa_obj, void* upnp_session, int error_code, const char* error_msg)
{
	if(upnp_session == NULL) return;

	MediaServer_Response_Error(upnp_session, error_code, error_msg);
	ILibWebServer_Release(upnp_session);
}

/* see header file */
int MSA_ForQueryResponse_Start(MSA msa_obj, void* upnp_session, struct MSA_CdsQuery* cds_query, int send_didl_header_flag)
{
	int status = 0;
	ASSERT(cds_query != NULL);

	/* don't bother sending data if the socket disconnected */
	if (upnp_session == NULL) return -1;

	if (cds_query->QueryType == MSA_Query_Search)
	{
		status = MediaServer_AsyncResponse_START(upnp_session, CDS_STRING_SEARCH, CDS_STRING_URN_CDS);
	}
	else
	{
		status = MediaServer_AsyncResponse_START(upnp_session, CDS_STRING_BROWSE, CDS_STRING_URN_CDS);
	}

	if ((send_didl_header_flag != 0) && (status >= 0))
	{
		status = MediaServer_AsyncResponse_OUT(upnp_session, CDS_STRING_RESULT, CDS_DIDL_HEADER_ESCAPED, CDS_DIDL_HEADER_ESCAPED_LEN, ILibAsyncSocket_MemoryOwnership_STATIC, 1, 0);
	}
	else
	{
		status = MediaServer_AsyncResponse_OUT(upnp_session, CDS_STRING_RESULT, "", 0, ILibAsyncSocket_MemoryOwnership_STATIC, 1, 0);
	}

	return status;
}

/* see header file */
int MSA_ForQueryResponse_ResultArgumentRaw(MSA msa_obj, void* upnp_session, struct MSA_CdsQuery* cds_query, const char* xml_escaped_utf8_didl, int didl_size)
{
	int status = 0;
	ASSERT(cds_query != NULL);

	/* don't bother sending data if the socket disconnected */
	if (upnp_session == NULL) return -1;

	status = MediaServer_AsyncResponse_OUT(upnp_session, CDS_STRING_RESULT, xml_escaped_utf8_didl, didl_size, ILibAsyncSocket_MemoryOwnership_USER, 0, 0);

	return status;
}

/* see header file */
int MSA_ForQueryResponse_FinishResponse(MSA msa_obj, void* upnp_session, struct MSA_CdsQuery* cds_query, int send_did_footer_flag, unsigned int number_returned, unsigned int total_matches, unsigned int update_id)
{
	char numResult[30];
	int status = 0;

	ASSERT(cds_query != NULL);

	/* don't bother sending data if the socket disconnected */
	if (upnp_session == NULL) return -1;

	if (send_did_footer_flag != 0)
	{
		status = MediaServer_AsyncResponse_OUT(upnp_session, CDS_STRING_RESULT, CDS_DIDL_FOOTER_ESCAPED, CDS_DIDL_FOOTER_ESCAPED_LEN, ILibAsyncSocket_MemoryOwnership_STATIC, 0, 1);
	}
	else
	{
		status = MediaServer_AsyncResponse_OUT(upnp_session, CDS_STRING_RESULT, "", 0, ILibAsyncSocket_MemoryOwnership_STATIC, 0, 1);
	}

	if (status >= 0)
	{
		/*
		 *	Instruct the generated microstack to send the data for the last
		 *	three out-arguments of the Browse request.
		 */

		sprintf(numResult, "%u", number_returned);
		status = MediaServer_AsyncResponse_OUT(upnp_session, CDS_STRING_NUMBER_RETURNED, numResult, (int) strlen(numResult), ILibAsyncSocket_MemoryOwnership_USER, 1,1);

		if (status >= 0)
		{
			sprintf(numResult, "%u", total_matches);
			status = MediaServer_AsyncResponse_OUT(upnp_session, CDS_STRING_TOTAL_MATCHES, numResult, (int) strlen(numResult), ILibAsyncSocket_MemoryOwnership_USER, 1,1);

			if (status >= 0)
			{
				sprintf(numResult, "%u", update_id);
				status = MediaServer_AsyncResponse_OUT(upnp_session, CDS_STRING_UPDATE_ID, numResult, (int) strlen(numResult), ILibAsyncSocket_MemoryOwnership_USER, 1,1);

				if (status >= 0)
				{
					if (cds_query->QueryType == MSA_Query_Search)
					{
						status = MediaServer_AsyncResponse_DONE(upnp_session, CDS_STRING_SEARCH);
					}
					else
					{
						status = MediaServer_AsyncResponse_DONE(upnp_session, CDS_STRING_BROWSE);
					}
				}
			}
		}
	}

	/* we are not going to call any more MSA_ForQueryResponse_xxx methods for this UPnP response */
	ILibWebServer_Release(upnp_session);

	return status;
}

void MSA_ForQueryResponse_Cancelled(MSA msa_obj, void* upnp_session, struct MSA_CdsQuery *cdsQuery)
{
	ASSERT(cdsQuery != NULL);
	if (upnp_session == NULL) return;
	
	/* we are not going to call any more MSA_ForQueryResponse_xxx methods for this UPnP response */
	ILibWebServer_Release(upnp_session);
}

/* see header file */
void MSA_IncrementSystemUpdateID(MSA msa_obj)
{
	struct MSA_InternalState *state;
	state = (struct MSA_InternalState*) msa_obj->InternalState;

	state->SystemUpdateID++;
	MediaServer_SetState_ContentDirectory_SystemUpdateID(msa_obj->DeviceMicroStack, state->SystemUpdateID);
}

/* see header file */
void MSA_UpdateContainerID(MSA msa_obj, const char *container_id, unsigned int container_update_id)
{
	struct MSA_InternalState *state;
	struct MSA_ContainerUpdate *cu;
	struct MSA_ContainerUpdate *fcu;
	struct MSA_ContainerUpdate *lcu;
	int size;

	state = (struct MSA_InternalState*) msa_obj->InternalState;


	/* lock state */
	sem_wait(&(state->Lock));

	/*
	 *	Attempt to find an existing ContainerUpdate
	 *	object for the specified containerID.
	 */
	cu = state->ContainerUpdateID;
	lcu = fcu = NULL;
	while ((cu != NULL) && (fcu == NULL))
	{
		if (strcmp(cu->ContainerID, container_id) == 0)
		{
			fcu = cu;
		}

		lcu = cu;
		cu = cu->Next;
	}

	if (fcu == NULL)
	{
		/*
		 *	If fcu is NULL, then we need to add
		 *	a new MSA_ContainerUpdate to the object.
		 */
		if(lcu){
			fcu = lcu->Next = (struct MSA_ContainerUpdate*) malloc(sizeof(struct MSA_ContainerUpdate));
		}
		else{
			fcu = state->ContainerUpdateID = (struct MSA_ContainerUpdate*) malloc(sizeof(struct MSA_ContainerUpdate));
			fcu->Next = NULL;
		}
		
		size = (int) strlen(container_id)+1;
		fcu->ContainerID = (char*) malloc(size);
		memcpy(fcu->ContainerID, container_id, size);
		fcu->Next = NULL;
	}

	ASSERT(fcu != NULL);

	/*
	 *	Assign a new UpdateID for the specified containerID.
	 */
	fcu->UpdateID = container_update_id;

	MSA_Helper_UpdateModerated_ContainerUpdateID(msa_obj);

	/* unlock */
	sem_post(&(state->Lock));
}

void MSA_UpdateIpInfo(MSA msa_obj, int *ip_addr_list, int ip_addr_list_len)
{
	int size;
	struct MSA_InternalState *state;
	state = (struct MSA_InternalState*) msa_obj->InternalState;

	/* copy the ip addresses to the msa object */

	sem_wait(&(state->Lock));

	if (state->IpAddrList != NULL)
	{
		free(state->IpAddrList);
	}
	size = (int) (ip_addr_list_len * sizeof(int));
	state->IpAddrList = (int*) malloc(size);
	memcpy(state->IpAddrList, ip_addr_list, size);
	state->IpAddrListLen = ip_addr_list_len;

	sem_post(&(state->Lock));
}


unsigned int MSA_GetSystemUpdateID(MSA msa_obj)
{
	struct MSA_InternalState *state;

	if(msa_obj!=NULL)
	{
		state = (struct MSA_InternalState*) msa_obj->InternalState;
		return state->SystemUpdateID;
	}
	return -1;
}

void MSA_SetSystemUpdateID(MSA msa_obj, unsigned int system_update_id)
{
	struct MSA_InternalState *state;

	if(msa_obj!=NULL)
	{
		state = (struct MSA_InternalState*) msa_obj->InternalState;

		state->SystemUpdateID = system_update_id;

		/* set the UPnP state to send the event out to subscribed control points. */
		MediaServer_SetState_ContentDirectory_SystemUpdateID(msa_obj->DeviceMicroStack, state->SystemUpdateID);
	}
}

void MSA_LockState(MSA msa_obj)
{
	struct MSA_InternalState *state;

	if(msa_obj!=NULL)
	{
		state = (struct MSA_InternalState*) msa_obj->InternalState;
		sem_wait(&(state->Lock));
	}
}

void MSA_UnLockState(MSA msa_obj)
{
	struct MSA_InternalState *state;

	if(msa_obj!=NULL)
	{
		state = (struct MSA_InternalState*) msa_obj->InternalState;
		sem_post(&(state->Lock));
	}
}

void* MSA_GetUserObject(MSA msa_obj)
{
	struct MSA_InternalState *state;

	if(msa_obj!=NULL)
	{
		state = (struct MSA_InternalState*) msa_obj->InternalState;
		return state->UserObject;
	}
	return NULL;
}

void MSA_SetSourceProtocolInfo(MSA msa_obj, const char* source_protocol_info)
{
	struct MSA_InternalState *state;

	if(msa_obj!=NULL)
	{
		state = (struct MSA_InternalState*) msa_obj->InternalState;
		if(state->SourceProtocolInfo != NULL)
		{
			free(state->SourceProtocolInfo);
		}

		state->SourceProtocolInfo = (char*) malloc(strlen(source_protocol_info) + 1);
		strcpy(state->SourceProtocolInfo, source_protocol_info);

		/* set the UPnP state to send the event out to subscribed control points. */
		MediaServer_SetState_ConnectionManager_SourceProtocolInfo(msa_obj->DeviceMicroStack, state->SourceProtocolInfo);
	}
}

const char* MSA_GetSourceProtocolInfo(MSA msa_obj)
{
	struct MSA_InternalState *state;

	if(msa_obj!=NULL)
	{
		state = (struct MSA_InternalState*) msa_obj->InternalState;
		return state->SourceProtocolInfo;
	}
	return NULL;
}

void MSA_SetSinkProtocolInfo(MSA msa_obj, const char* sink_protocol_info)
{
	struct MSA_InternalState *state;

	if(msa_obj!=NULL)
	{
		state = (struct MSA_InternalState*) msa_obj->InternalState;
		if(state->SinkProtocolInfo != NULL)
		{
			free(state->SinkProtocolInfo);
		}

		state->SinkProtocolInfo = (char*) malloc(strlen(sink_protocol_info) + 1);
		strcpy(state->SinkProtocolInfo, sink_protocol_info);

		/* set the UPnP state to send the event out to subscribed control points. */
		MediaServer_SetState_ConnectionManager_SourceProtocolInfo(msa_obj->DeviceMicroStack, state->SinkProtocolInfo);
	}
}

const char* MSA_GetSinkProtocolInfo(MSA msa_obj)
{
	struct MSA_InternalState *state;

	if(msa_obj!=NULL)
	{
		state = (struct MSA_InternalState*) msa_obj->InternalState;
		return state->SinkProtocolInfo;
	}
	return NULL;
}

void MSA_SetSortableProperties(MSA msa_obj, const char* sortable_fields)
{
	struct MSA_InternalState *state;

	if(msa_obj!=NULL)
	{
		state = (struct MSA_InternalState*) msa_obj->InternalState;
		if(state->SortCapabilitiesString != NULL)
		{
			free(state->SortCapabilitiesString);
		}

		state->SortCapabilitiesString = (char*) malloc(strlen(sortable_fields) + 1);
		strcpy(state->SortCapabilitiesString, sortable_fields);

		/* set the UPnP state to send the event out to subscribed control points. */
		MediaServer_SetState_ConnectionManager_SourceProtocolInfo(msa_obj->DeviceMicroStack, state->SortCapabilitiesString);
	}
}

const char* MSA_GetSortableProperties(MSA msa_obj)
{
	struct MSA_InternalState *state;

	if(msa_obj!=NULL)
	{
		state = (struct MSA_InternalState*) msa_obj->InternalState;
		return state->SortCapabilitiesString;
	}
	return NULL;
}

void MSA_SetSearchableProperties(MSA msa_obj, const char* searchable_fields)
{
	struct MSA_InternalState *state;

	if(msa_obj!=NULL)
	{
		state = (struct MSA_InternalState*) msa_obj->InternalState;
		if(state->SearchCapabilitiesString != NULL)
		{
			free(state->SearchCapabilitiesString);
		}

		state->SearchCapabilitiesString = (char*) malloc(strlen(searchable_fields) + 1);
		strcpy(state->SearchCapabilitiesString, searchable_fields);

		/* set the UPnP state to send the event out to subscribed control points. */
		MediaServer_SetState_ConnectionManager_SourceProtocolInfo(msa_obj->DeviceMicroStack, state->SearchCapabilitiesString);
	}
}

const char* MSA_GetSearchableProperties(MSA msa_obj)
{
	struct MSA_InternalState *state;

	if(msa_obj!=NULL)
	{
		state = (struct MSA_InternalState*) msa_obj->InternalState;
		return state->SearchCapabilitiesString;
	}
	return NULL;
}

/** Generic respond methods for all supported UPnP action **/
void MSA_RespondBrowse(MSA msa_obj, void* upnp_session, const char* result, const unsigned int number_returned, const unsigned int total_matches, const unsigned int update_id)
{
	if(upnp_session == NULL) return;

	MediaServer_Response_ContentDirectory_Browse(upnp_session, result, number_returned, total_matches, update_id);
	ILibWebServer_Release(upnp_session);
}

void MSA_RespondSearch(MSA msa_obj, void* upnp_session, const char* result, const unsigned int number_returned, const unsigned int total_matches, const unsigned int update_id)
{
	if(upnp_session == NULL) return;

	MediaServer_Response_ContentDirectory_Search(upnp_session, result, number_returned, total_matches, update_id);
	ILibWebServer_Release(upnp_session);
}

void MSA_RespondGetSystemUpdateID(MSA msa_obj, void* upnp_session, const unsigned int id)
{
	if(upnp_session == NULL) return;

	MediaServer_Response_ContentDirectory_GetSystemUpdateID(upnp_session, id);
	ILibWebServer_Release(upnp_session);
}

void MSA_RespondGetSearchCapabilities(MSA msa_obj, void* upnp_session, const char* search_caps)
{
	if(upnp_session == NULL) return;

	MediaServer_Response_ContentDirectory_GetSearchCapabilities(upnp_session, search_caps);
	ILibWebServer_Release(upnp_session);
}

void MSA_RespondGetSortCapabilities(MSA msa_obj, void* upnp_session, const char* sort_caps)
{
	if(upnp_session == NULL) return;

	MediaServer_Response_ContentDirectory_GetSortCapabilities(upnp_session, sort_caps);
	ILibWebServer_Release(upnp_session);
}

void MSA_RespondGetProtocolInfo(MSA msa_obj, void* upnp_session, const char* source, const char* sink)
{
	if(upnp_session == NULL) return;

	MediaServer_Response_ConnectionManager_GetProtocolInfo(upnp_session, source, sink);
	ILibWebServer_Release(upnp_session);
}

void MSA_RespondGetCurrentConnectionIDs(MSA msa_obj, void* upnp_session, const char* connection_ids)
{
	if(upnp_session == NULL) return;

	MediaServer_Response_ConnectionManager_GetCurrentConnectionIDs(upnp_session, connection_ids);
	ILibWebServer_Release(upnp_session);
}

void MSA_RespondGetCurrentConnectionInfo(MSA msa_obj, void* upnp_session, const int rcs_ic, const int av_transport_Id, const char* protocol_info, const char* peer_connection_manager, const int peer_connection_id, const char* direction, const char* status)
{
	if(upnp_session == NULL) return;

	MediaServer_Response_ConnectionManager_GetCurrentConnectionInfo(upnp_session, rcs_ic, av_transport_Id, protocol_info, peer_connection_manager, peer_connection_id, direction, status);
	ILibWebServer_Release(upnp_session);
}

#if defined(INCLUDE_FEATURE_UPLOAD)
void MSA_RespondCreateObject(MSA msa_obj, void* upnp_session, const char* object_id, const char* result)
{
	if(upnp_session == NULL) return;

	MediaServer_Response_ContentDirectory_CreateObject(upnp_session, object_id, result);
	ILibWebServer_Release(upnp_session);
}

void MSA_RespondDestroyObject(MSA msa_obj, void* upnp_session)
{
	if(upnp_session == NULL) return;

	MediaServer_Response_ContentDirectory_DestroyObject(upnp_session);
	ILibWebServer_Release(upnp_session);
}
#endif

void _ExecuteCallback(ILibThreadPool threadPool, void* context_switch)
{
    MSA msa = NULL;
	struct MSA_InternalState* internal_state = NULL;
    ContextSwitchCall contextSwitch = (ContextSwitchCall) context_switch;
    if(contextSwitch == NULL)
    {
        return;
    }
	msa = contextSwitch->MSA_Object;
	internal_state = (struct MSA_InternalState*) msa->InternalState;
	switch(contextSwitch->ActionId)
    {
        case MSA_CDS_BROWSE:
            {
				if(msa->OnBrowse != NULL && contextSwitch->ParameterCount == 1)
                {
                    struct MSA_CdsQuery* query = (struct MSA_CdsQuery*) contextSwitch->Parameters[0];
					msa->OnBrowse(msa, contextSwitch->UPnPSession, query);
                }
				else if(msa->OnBrowse == NULL)
                {
					MSA_ForResponse_RespondError(msa,  contextSwitch->UPnPSession, 501, "Action Failed: Action Not Implemented");
                }
                else
                {
					MSA_ForResponse_RespondError(msa, contextSwitch->UPnPSession, 501, "Action Failed: Program Error");
                }
            }
            break;
		case MSA_CDS_SEARCH:
            {
				if(msa->OnBrowse != NULL && contextSwitch->ParameterCount == 1)
                {
                    struct MSA_CdsQuery* query = (struct MSA_CdsQuery*) contextSwitch->Parameters[0];
					msa->OnBrowse(msa, contextSwitch->UPnPSession, query);
                }
				else if(msa->OnBrowse == NULL)
                {
					MSA_ForResponse_RespondError(msa,  contextSwitch->UPnPSession, 501, "Action Failed: Action Not Implemented");
                }
                else
                {
					MSA_ForResponse_RespondError(msa, contextSwitch->UPnPSession, 501, "Action Failed: Program Error");
                }
            }
			break;
		case MSA_CDS_GETSYSTEMUPDATEID:
            {
				if(msa->OnGetSystemUpdateID != NULL && contextSwitch->ParameterCount == 0)
                {
					msa->OnGetSystemUpdateID(msa, contextSwitch->UPnPSession);
                }
				else if(msa->OnGetSystemUpdateID == NULL)
                {
					MSA_ForResponse_RespondError(msa,  contextSwitch->UPnPSession, 501, "Action Failed: Action Not Implemented");
                }
                else
                {
					MSA_ForResponse_RespondError(msa, contextSwitch->UPnPSession, 501, "Action Failed: Program Error");
                }
            }
			break;
		case MSA_CDS_GETSEARCHCAPABILITIES:
            {
				if(msa->OnGetSearchCapabilities != NULL && contextSwitch->ParameterCount == 0)
                {
					msa->OnGetSearchCapabilities(msa, contextSwitch->UPnPSession);
                }
				else if(msa->OnGetSearchCapabilities == NULL)
                {
					MSA_ForResponse_RespondError(msa,  contextSwitch->UPnPSession, 501, "Action Failed: Action Not Implemented");
                }
                else
                {
					MSA_ForResponse_RespondError(msa, contextSwitch->UPnPSession, 501, "Action Failed: Program Error");
                }
            }
			break;
		case MSA_CDS_GETSORTCAPABILITIES:
            {
				if(msa->OnGetSortCapabilities != NULL && contextSwitch->ParameterCount == 0)
                {
					msa->OnGetSortCapabilities(msa, contextSwitch->UPnPSession);
                }
				else if(msa->OnGetSortCapabilities == NULL)
                {
					MSA_ForResponse_RespondError(msa,  contextSwitch->UPnPSession, 501, "Action Failed: Action Not Implemented");
                }
                else
                {
					MSA_ForResponse_RespondError(msa, contextSwitch->UPnPSession, 501, "Action Failed: Program Error");
                }
            }
			break;
		case MSA_CMS_GETPROTOCOLINFO:
            {
				if(msa->OnGetProtocolInfo != NULL && contextSwitch->ParameterCount == 0)
                {
					msa->OnGetProtocolInfo(msa, contextSwitch->UPnPSession);
                }
				else if(msa->OnGetProtocolInfo == NULL)
                {
					MSA_ForResponse_RespondError(msa,  contextSwitch->UPnPSession, 501, "Action Failed: Action Not Implemented");
                }
                else
                {
					MSA_ForResponse_RespondError(msa, contextSwitch->UPnPSession, 501, "Action Failed: Program Error");
                }
            }
			break;
		case MSA_CMS_GETCURRENTCONNECTIONIDS:
            {
				if(msa->OnGetCurrentConnectionIDs != NULL && contextSwitch->ParameterCount == 0)
                {
					msa->OnGetCurrentConnectionIDs(msa, contextSwitch->UPnPSession);
                }
				else if(msa->OnGetCurrentConnectionIDs == NULL)
                {
					MSA_ForResponse_RespondError(msa,  contextSwitch->UPnPSession, 501, "Action Failed: Action Not Implemented");
                }
                else
                {
					MSA_ForResponse_RespondError(msa, contextSwitch->UPnPSession, 501, "Action Failed: Program Error");
                }
            }
			break;
		case MSA_CMS_GETCURRENTCONNECTIONINFO:
            {
				if(msa->OnGetCurrentConnectionInfo != NULL && contextSwitch->ParameterCount == 1)
                {
					msa->OnGetCurrentConnectionInfo(msa, contextSwitch->UPnPSession, (int) contextSwitch->Parameters[0]);
                }
				else if(msa->OnGetCurrentConnectionInfo == NULL)
                {
					MSA_ForResponse_RespondError(msa,  contextSwitch->UPnPSession, 501, "Action Failed: Action Not Implemented");
                }
                else
                {
					MSA_ForResponse_RespondError(msa, contextSwitch->UPnPSession, 501, "Action Failed: Program Error");
                }
            }
			break;
#if defined(INCLUDE_FEATURE_UPLOAD)
		case MSA_CDS_CREATEOBJECT:
			{
				if(msa->OnCreateObject != NULL && contextSwitch->ParameterCount == 2)
                {
                    struct MSA_CdsCreateObj* createObjArg = (struct MSA_CdsCreateObj*) contextSwitch->Parameters[0];
					struct CdsObject* newObj = (struct CdsObject*) contextSwitch->Parameters[1];
					msa->OnCreateObject(msa, contextSwitch->UPnPSession, createObjArg, newObj);
                }
				else if(msa->OnCreateObject == NULL)
                {
					MSA_ForResponse_RespondError(msa,  contextSwitch->UPnPSession, 501, "Action Failed: Action Not Implemented");
                }
                else
                {
					MSA_ForResponse_RespondError(msa, contextSwitch->UPnPSession, 501, "Action Failed: Program Error");
                }
			}
			break;
		case MSA_CDS_DESTROYOBJECT:
			{
				if(msa->OnDestroyObject != NULL && contextSwitch->ParameterCount == 1)
                {
					struct MSA_CdsDestroyObj* destroyObjArg = (struct MSA_CdsDestroyObj*) contextSwitch->Parameters[0];
					msa->OnDestroyObject(msa, contextSwitch->UPnPSession, destroyObjArg);
                }
				else if(msa->OnDestroyObject == NULL)
                {
					MSA_ForResponse_RespondError(msa,  contextSwitch->UPnPSession, 501, "Action Failed: Action Not Implemented");
                }
                else
                {
					MSA_ForResponse_RespondError(msa, contextSwitch->UPnPSession, 501, "Action Failed: Program Error");
                }
			}
			break;
#endif
        default:
            return;
    }
    free(contextSwitch);
}

MSA_Error _ExecuteCallbackThroughThreadPool(MSA msa_obj, ContextSwitchCall context_switch)
{
	if(msa_obj == NULL)
	{
		return MSA_ERROR_INTERNALERROR;
	}

	/* add ref to UPnP Session, because the callback could be executing on a different thread */
	ILibWebServer_AddRef(context_switch->UPnPSession);

	if(msa_obj->ContextSwitchBitMask & context_switch->ActionId)
	{
		/* bit mask indicates that this action handler will be called using context switch through ThreadPool */
        ILibThreadPool_QueueUserWorkItem(msa_obj->ThreadPool, (void*)context_switch, &_ExecuteCallback);
    }
    else
    {
		/* no context switch, just execute the callback using the same thread */
        _ExecuteCallback(NULL, (void*)context_switch);
    }

    return MSA_ERROR_NONE;
}

/* END SECTION - public methods */
/************************************************************************************/


